#ifndef  __UDP_SERVER_HEADER__
#define  __UDP_SERVER_HEADER__

#ifdef __cplusplus
extern "C" {
#endif
#include <xil_types.h>
#include <xstatus.h>
#include "lwip/netif.h"
#include "lwip/ip_addr.h"
#include "lwip/udp.h"




#define NETIF_EMAC_BASEADDR         XPAR_XEMACPS_0_BASEADDR
#define FRAME_HEADER_HMI_TO_SOC     0xA5
#define FRAME_HEADER_SOC_TO_HMI     0x5A

#define CMD_HAND_SHAKE 				0x0001
#define CMD_DETECTOR_SETTINGS 		0x0002
#define CMD_DISABLE_DETECTOR        0x0003
#define CMD_ACQUIRE_SETTINGS		0x0004
#define CMD_ACQUIRE_START			0x0005
#define CMD_ACQUIRE_STOP			0x0006
#define CMD_DRIVE_SETTINGS 			0x0007
#define CMD_SORTING_START			0x0008
#define CMD_SORTING_STOP			0x0009

#define CMD_GATE_SETTINGS			0x000A
#define CMD_SPEED_MEASURE_SETTINGS  0x000B
#define CMD_DELAY_REFER_SETTINGS    0x000C

#define CMD_EVENT_DATA				0x0010
#define CMD_WAVEFORM_DATA			0x0011



#define UDP_FRAME_LENGTH  		1024

XStatus init_udp_server(struct netif *netif, ip_addr_t *ipaddr, ip_addr_t *netmask, ip_addr_t *gateway, UINTPTR mac_baseaddr);

XStatus start_udp_server(struct udp_pcb *server_pcb, const ip_addr_t *ip_addr, u16_t port, udp_recv_fn udp_recv_handler, void *recv_arg);

XStatus setup_udp_server();

void lwip_loop_func();


XStatus send_udp_frame(u16_t cmd_type, u16_t data_len, const u8_t *data);


#ifdef __cplusplus
}
#endif

#endif
