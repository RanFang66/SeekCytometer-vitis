
#include "xil_printf.h"
#include "xparameters.h"
#include "lwipopts.h"
#include "log.h"

#include "lwip/init.h"
#include "lwip/err.h"

#include "netif/xadapter.h"
#include "udp_server.h"
#include <arch/cc.h>
#include <xstatus.h>

#include "cytometer.h"
#include "timers.h"
#include "axi_dma.h"
#include "event_data.h"






/* Is connected with HMI */
int is_connected = 0;
int comm_lost_timer = 0;

static unsigned char mac_ethernet_address[] = {0x00, 0x0a, 0x35, 0x00, 0x01, 0x02 };
static struct netif netif_inst;
static struct udp_pcb *server_pcb;
static u16_t  sequence_val = 0;
static u16_t  sequence_last = 0;
static u16_t  sequence_recv = 0;
static u16_t  sequence_recv_last = 0;
static u8_t   hmi_state = 0;

ip_addr_t       target_ip;
u16_t           target_port;
static u16_t    server_port = 5001;

u8_t waveform_data_buffer[MAX_DMA_LEN];


static u8 wait_for_dma_finished = 0;


extern u8 send_udp_frame_flag;
extern u8 dma_s2mm_flag;
extern u8 send_event_data_flag;






static void print_ip(const char *msg, const ip_addr_t *ip)
{
	print(msg);
	xil_printf("%d.%d.%d.%d\r\n", ip4_addr1(ip), ip4_addr2(ip),
			ip4_addr3(ip), ip4_addr4(ip));
}

static void print_ip_settings(const struct netif *netif)
{
    print_ip("Board IP:       ", &(netif->ip_addr));
	print_ip("Netmask :       ", &(netif->netmask));
	print_ip("Gateway :       ", &(netif->gw));
}

XStatus init_udp_server(struct netif *netif, ip_addr_t *ipaddr, ip_addr_t *netmask, ip_addr_t *gateway, UINTPTR mac_baseaddr)
{
    if (!netif) {
        LOG_ERROR("The input netif instance is NULL!");
        return XST_FAILURE;
    }

    lwip_init();

    if (!xemac_add(netif, ipaddr, netmask, gateway, mac_ethernet_address, mac_baseaddr)) {
        LOG_ERROR("Add network interface failed!");
        return XST_FAILURE;
    }

    netif_set_default(netif);

    netif_set_up(netif);
    if (netif_is_up(netif)) {
        LOG_INFO("Network interface is up!");
    } else {
        LOG_ERROR("Cannot up network interface!");
        return XST_FAILURE;
    }

    print_ip_settings(netif);

    LOG_INFO("Initialize network interface finished!");

    return XST_SUCCESS;
}


XStatus start_udp_server(struct udp_pcb *server_pcb, const ip_addr_t *ip_addr, u16_t port, udp_recv_fn udp_recv_handler, void *recv_arg)
{
    err_t  err;

    err = udp_bind(server_pcb, ip_addr, port);
    if (err != ERR_OK) {
        LOG_ERROR("Failed to bind UDP server to port %d, err_no = %d.", port, err);
        udp_remove(server_pcb);
        return XST_FAILURE;
    }
    LOG_INFO("bind UDP server to port %d ok!", port);

    udp_recv(server_pcb, udp_recv_handler, recv_arg);

    return XST_SUCCESS;
}

u32_t big_endian_to_little_endian(const u8_t *bytes)
{
	return ((u32_t)bytes[0] << 24) |
			((u32_t)bytes[1] << 16) |
			((u32_t)bytes[2] << 8) |
			((u32_t)bytes[3]);
}


static u16_t  checksum(const u8_t *pdata, int len)
{
	u16_t sum = 0;
	for (int i = 0; i < len; i++) {
		sum += (u8_t)pdata[i];
	}
	return sum;
}

static XStatus parse_frame(const u8_t *pdata, int len)
{
    if (len < 9) {
    	LOG_WARN("UDP Server Error: Frame size is less than 9");
		return XST_FAILURE;
	}

	u8_t frame_head = (u8_t)pdata[0];
	if (frame_head != FRAME_HEADER_HMI_TO_SOC) {
		LOG_WARN("UDP Server Error: Frame header error!");
		return XST_FAILURE;
	}

	u16_t chk_sum = checksum(pdata, len - 2);
	if (chk_sum !=  (((u16_t)pdata[len-2] << 8) | (u16_t)pdata[len-1])) {
		LOG_WARN("UDP Server Error: Frame check sum error!");
		return XST_FAILURE;
	}

    sequence_recv = ((u16_t)pdata[1] << 8) | (u16_t)pdata[2];
	u16_t cmd_type = ((u16_t)pdata[3] << 8) | (u16_t)pdata[4];
	u16_t data_len = ((u16_t)pdata[5] << 8) | (u16_t)pdata[6];

    if (data_len != len - 9) {
    	LOG_WARN("UDP Server Error: Frame size is not match with data size!");
		return XST_FAILURE;
    }

    const u8_t *data_start = pdata+7;
	switch (cmd_type) {
	case CMD_HAND_SHAKE:
	{
        u8_t state = (u8_t)data_start[0];
        if (state != hmi_state) {
            LOG_INFO("HMI state changed from %d to %d.", hmi_state, state);
            hmi_state = state;
        }
		break;
    }
	case CMD_DETECTOR_SETTINGS:
    {
		u8_t enabled_channels = 0;
    	for (int i = 0; i < data_len / 9; i++) {
			u8_t id = (u8_t)data_start[9*i];
			u8_t enable_height = (u8_t)data_start[9*i + 1];
			u8_t enable_width = (u8_t)data_start[9*i + 2];
			u8_t enable_area = (u8_t)data_start[9*i + 3];
			u8_t enable_thresh = (u8_t)data_start[9*i + 4];
			int threshold_val = big_endian_to_little_endian(data_start + 9*i+5);

			if (id <= AD_CHANNEL_NUM && id > 0) {
				update_detector_settings(id, enable_height, enable_width, enable_area, enable_thresh, threshold_val);
			}

			enabled_channels |= ((u8_t)0x01 << (id-1));
		}
    	update_enabled_channels(enabled_channels);
        LOG_INFO("Detector settings changed.");
		break;
    }
    case CMD_DISABLE_DETECTOR:
    {
        u8_t id = (u8_t)data_start[0];
        reset_detector(id);
        break;
    }

	case CMD_ACQUIRE_SETTINGS:
	break;
	case CMD_ACQUIRE_START:
    {
        cytometer_start_analyze();
//        reset_event_data();
        LOG_INFO("Start analyzing.");
        break;
    }

	case CMD_ACQUIRE_STOP:
    {
        cytometer_stop_analyze();
        LOG_INFO("Stop analyzing.");

        break;
    }

	case CMD_DRIVE_SETTINGS:
	{
		u8_t type = (u8_t)(data_start[0]);
		u32_t drive_delay = big_endian_to_little_endian(data_start + 1);
		u32_t drive_width = big_endian_to_little_endian(data_start + 5);
		u32_t cooling_time = big_endian_to_little_endian(data_start + 9);
		u32_t coe = big_endian_to_little_endian(data_start + 13);
		cytometer_set_drive_settings(type, drive_delay, drive_width, cooling_time, coe);
		LOG_INFO("Drive settings changed. %d, %d, %d, %d, %d", type, drive_delay, drive_width, cooling_time, coe);
		break;
	}

	case CMD_GATE_SETTINGS:
	{
		struct GateData gate;
		gate.gate_type = (data_start[0]);
		gate.ch_x = (data_start[1]);
		gate.ch_y = (data_start[2]);
		gate.type_x = (data_start[3]);
		gate.type_y = (data_start[4]);
		gate.x_min = big_endian_to_little_endian(data_start + 5);
		gate.y_max = big_endian_to_little_endian(data_start + 9);



		if (data_len >= 21) {
			gate.x_max = big_endian_to_little_endian(data_start + 13);
			gate.y_min = big_endian_to_little_endian(data_start + 17);
		}
		cytometer_set_gate(gate);
		LOG_INFO("Gate Changed, type: %d, ChX = %d-%d, ChY = %d-%d, X(%d-%d), y(%d-%d)",
				gate.gate_type, gate.ch_x, gate.type_x, gate.ch_y, gate.type_y, gate.x_min, gate.x_max, gate.y_min, gate.y_max);
		break;
	}

	case CMD_SPEED_MEASURE_SETTINGS:
		u8_t preCh = data_start[0];
		u8_t postCh = data_start[1];
		int preThresh = big_endian_to_little_endian(data_start+2);
		int dist = big_endian_to_little_endian(data_start+6);
		int max_diff_time = big_endian_to_little_endian(data_start+10);

		cytometer_set_speed_measure(preCh, postCh, preThresh, dist, max_diff_time);
		LOG_INFO("Speed Measure Setting Changed, PreCh = %d, PostCh = %d, PreThresh = %d, max_diff_time = %d.", preCh, postCh, preThresh, max_diff_time);
		break;

	case CMD_SORTING_START:
    {
        cytometer_start_sort();
        LOG_INFO("Start sorting.");

        break;
    }

	case CMD_SORTING_STOP:
    {
        cytometer_stop_sort();
        LOG_INFO("Stop sorting.");

    	break;
    }

	case CMD_EVENT_DATA:
	break;

	case CMD_WAVEFORM_DATA:
    {
		u8_t waveform_en = (u8_t)(data_start[0]);
        u8_t waveform_channels = (u8_t)(data_start[1]);
        u8_t sample_interval = (u8_t)(data_start[2]);

        update_waveform_setting(waveform_en, waveform_channels, sample_interval);
        LOG_INFO("Waveform setting changed: enabled: %d, channels: %d, interval: %d.", waveform_en, waveform_channels, sample_interval);

        break;
    }

	default:
		LOG_ERROR("UDP Server Error: Undefined command type!");
		return XST_FAILURE;
		break;
	}

	return XST_SUCCESS;
}


static void udp_recv_handler(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{

    if (!p) {
    	LOG_WARN("UDP server received NULL pbuf!");
        return;
    }

    if (p->len < 9) {
    	LOG_WARN("UDP server received frame, but frame length is %d less then 9!", p->len);
        pbuf_free(p);
        return;
    }

    u8_t *pdata;
    pdata = (u8_t*)(p->payload);
    parse_frame(pdata, p->len);

    pbuf_free(p);

}



XStatus setup_udp_server()
{
    XStatus status;
    ip_addr_t ipaddr;
    ip_addr_t netmask;
    ip_addr_t gateway;
    IP_ADDR4(&ipaddr, 192, 168, 8, 10);
    IP_ADDR4(&netmask, 255, 255, 255, 0);
    IP_ADDR4(&gateway, 192, 168, 8, 1);

    IP4_ADDR(&target_ip, 192, 168, 8, 35);
    target_port = 8080;

    status = init_udp_server(&netif_inst, &ipaddr, &netmask, &gateway, NETIF_EMAC_BASEADDR);
    if (status != XST_SUCCESS) {
        LOG_ERROR("Platform initialize UDP server failed!");
        return status;
    }

    server_pcb = udp_new();
    if (!server_pcb) {
        LOG_ERROR("Error creating UDP PCB.");
        return XST_FAILURE;
    }
    status = start_udp_server(server_pcb, IP_ADDR_ANY, server_port, udp_recv_handler, server_pcb);
    if (status != XST_SUCCESS) {
        LOG_ERROR("Platform startup UDP server failed!");
        return status;
    }
    return XST_SUCCESS;
}


XStatus send_udp_frame(u16_t cmd_type, u16_t data_len, const u8_t *data)
{
    struct pbuf *p;
    err_t ret;

    p = pbuf_alloc(PBUF_TRANSPORT, data_len + 9, PBUF_POOL);
    if (!p) {
        LOG_ERROR("Allocate pbuf failed!");
        return XST_FAILURE;
    }

    ((u8_t *) p->payload)[0] = FRAME_HEADER_SOC_TO_HMI;
    ((u8_t *) p->payload)[1] = ((u8_t)(sequence_val >> 8));
	((u8_t *) p->payload)[2] = ((u8_t)(sequence_val));
	((u8_t *) p->payload)[3] = ((u8_t)(cmd_type >> 8));
	((u8_t *) p->payload)[4] = ((u8_t)(cmd_type));
	((u8_t *) p->payload)[5] = ((u8_t)(data_len >> 8));
	((u8_t *) p->payload)[6] = ((u8_t)(data_len));
    if (data_len > 0 && data) {
	    memcpy(((u8*)p->payload + 7), data, data_len);
    }
    u16_t chksum = checksum((u8_t*)p->payload, data_len + 7);
    ((u8_t *) p->payload)[data_len+7] = ((u8_t)(chksum >> 8));
    ((u8_t *) p->payload)[data_len+8] = ((u8_t)(chksum));
    p->len = p->tot_len = 9+data_len;
    ret = udp_sendto(server_pcb, p, &target_ip, target_port);
    pbuf_free(p);
    sequence_val++;
    if (ret != ERR_OK) {
        LOG_ERROR("Send UDP frame error! err_no = %d", ret);
        return XST_FAILURE;
    }
    return XST_SUCCESS;
}

XStatus send_handshake()
{
    u8_t send_data[1];
    send_data[0] = (u8_t)(get_current_state());
    return send_udp_frame(CMD_HAND_SHAKE, 1, send_data);
}


XStatus send_waveform()
{
	u16 udp_len = 0;
	u8 enabled_channels = waveform_enabled_channels();

	int data_len_bytes = update_waveform_data(waveform_data_buffer, enabled_channels, waveform_sample_interval());


	for (int i = 0; i < data_len_bytes; i+= UDP_FRAME_LENGTH) {
		if (i + UDP_FRAME_LENGTH > data_len_bytes) {
			udp_len = data_len_bytes - i;
		} else {
			udp_len = UDP_FRAME_LENGTH;
		}
		send_udp_frame(CMD_WAVEFORM_DATA, udp_len, (const u8*)waveform_data_buffer + i);
	}

	return XST_SUCCESS;
}

XStatus send_event_data(u32 data_bytes, const u8 *data_buffer)
{
	u16 udp_len = 0;

	u8 channel_num = analyze_enabled_channels_num();
	u16 event_bytes = channel_num * 3 + 5;
	u16 frame_len = (UDP_FRAME_LENGTH / event_bytes) * event_bytes;
	u16 count = 0;
	for (int i = 0; i < data_bytes; i += frame_len) {
		if (i + frame_len > data_bytes) {
			udp_len = data_bytes - i;
		} else {
			udp_len = frame_len;
		}
		send_udp_frame(CMD_EVENT_DATA, udp_len, data_buffer + i);
		count++;
	}

//	LOG_INFO("Send %d Frames(%d bytes) data, total: %d bytes", count, frame_len, data_bytes);


	return XST_SUCCESS;
}



void lwip_loop_func()
{
    int ret = 0;
    xemacif_input(&netif_inst);


    if (send_event_data_flag == 1) {
    	u32 ch_num = analyze_enabled_channels_num();
    	send_events_udp(ch_num);
 //    	send_event_data_flag = 0;
//    	u32 data_bytes = process_events();
//    	if (data_bytes > 0) {
//    		send_event_data(data_bytes, get_event_data_buffer_udp());
//    	}
    }

    if (send_udp_frame_flag == 1) {
        send_udp_frame_flag = 0;
        if (sequence_last == sequence_val) {
            ret = send_handshake();
        }
        sequence_last = sequence_val;

        if (is_connected) {
            if (sequence_recv_last == sequence_recv) {
                comm_lost_timer += 2;
                if (comm_lost_timer > 10) {
                    is_connected = 0;
                    comm_lost_timer = 0;
                    LOG_WARN("Communication with HMI is lost!");
                }
            } else {
                comm_lost_timer = 0;
            }
        } else {
            comm_lost_timer = 0;
            if (sequence_recv_last != sequence_recv) {
                is_connected = 1;
                LOG_INFO("Communication with HMI connected!");
            }
        }
        sequence_recv_last = sequence_recv;
    }

    if (dma_s2mm_flag == 1) {
    	send_waveform();
    	dma_s2mm_flag = 0;
    	wait_for_dma_finished = 0;
    }

    if (waveform_is_enabled() && waveform_enabled_channels() > 0 && wait_for_dma_finished == 0) {
    	axi_dma_start();
    	wait_for_dma_finished = 1;
    }
}

