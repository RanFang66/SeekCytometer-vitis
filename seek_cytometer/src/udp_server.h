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

XStatus init_udp_server(struct netif *netif, ip_addr_t *ipaddr, ip_addr_t *netmask, ip_addr_t *gateway, UINTPTR mac_baseaddr);

XStatus start_udp_server(struct udp_pcb *server_pcb, const ip_addr_t *ip_addr, u16_t port, udp_recv_fn udp_recv_handler, void *recv_arg);

XStatus setup_udp_server();

void lwip_loop_func();



#ifdef __cplusplus
}
#endif

#endif
