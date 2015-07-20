#ifndef _LWIP_USR_LIB_H_
#define _LWIP_USR_LIB_H_

#include "lwip/netbuf.h"
#include "lwip/sys.h"
#include "lwip/ip_addr.h"

/* IP Address Acquisition Modes */
#define IPADDR_USE_STATIC  0 /* will force static IP addressing to be used */
#define IPADDR_USE_DHCP    1 /* will force DHCP with fallback to Link Local (Auto IP) */
#define IPADDR_USE_AUTOIP  2 /* will force Link Local only */

#define NET_INTERFACE_NUMBER 1


struct ip_param {
    int ipmode;
    struct ip_addr ipaddr;
    struct ip_addr gw;
    struct ip_addr netmask;
};

#define IS_IP_ADDR_VALID(x) ((x)!=0 && (x)!=0xffffffff)

extern void lwip_usr_init(const unsigned char *pucMAC, struct ip_param *ipcfg, int netifno);
extern void lwip_usr_modify_net_cfg(struct ip_param *ipcfg, int netifno);
extern unsigned long lwip_usr_get_ipaddr(int netifno);
extern unsigned long lwip_usr_get_netmask(int netifno);
extern unsigned long lwip_usr_get_gw(int netifno);
extern void lwip_usr_get_mac(int netifno, unsigned char *mac);
extern int lwip_usr_get_ipmode(int netifno);

extern struct netif *lwip_usr_get_netif(int netifno);
extern int lwip_usr_get_netif_no(struct netif *netif);

extern void lwip_usr_get_ipcfg4init(struct ip_param *ipcfg);



#endif
