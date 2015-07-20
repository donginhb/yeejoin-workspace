/*
 * netutils: ping implementation
 */

#include "lwip/opt.h"

#include "lwip/mem.h"
#include "lwip/icmp.h"
#include "lwip/netif.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "lwip/inet_chksum.h"
#include "lwip/ip.h"

#include <rtthread.h>
/**
 * PING_DEBUG: Enable debugging for PING.
 */
#ifndef PING_DEBUG
#define PING_DEBUG     LWIP_DBG_ON
#endif

#define PING_TARGET   (netif_default?netif_default->gw:ip_addr_any)
/** ping receive timeout - in milliseconds */
#define PING_RCV_TIMEO 1000
/** ping delay - in milliseconds */
#define PING_DELAY     1000

/** ping identifier - must fit on a u16_t */
#ifndef PING_ID
#define PING_ID        0xAFAF
#endif

/** ping additional data size to include in the packet */
#ifndef PING_DATA_SIZE
#define PING_DATA_SIZE 32
#endif

/** ping result action - no default action */
#ifndef PING_RESULT
#define PING_RESULT(ping_ok)
#endif

#if PING_DEBUG == LWIP_DBG_ON
#define print_ping(x) printf_syn x
#else
#define print_ping(x)
#endif



/* ping variables */
static u16_t ping_seq_num;
static u32_t ping_time;

struct _ip_addr
{
	rt_uint8_t addr0, addr1, addr2, addr3;
};

/** Prepare a echo ICMP request */
static void ping_prepare_echo( struct icmp_echo_hdr *iecho, u16_t len)
{
	size_t i;
	size_t data_len = len - sizeof(struct icmp_echo_hdr);

	ICMPH_TYPE_SET(iecho, ICMP_ECHO);
	ICMPH_CODE_SET(iecho, 0);
	iecho->chksum = 0;
	iecho->id     = PING_ID;
	iecho->seqno  = htons(++ping_seq_num);

	/* fill the additional data buffer with some data */
	for(i = 0; i < data_len; i++)
	{
		((char*)iecho)[sizeof(struct icmp_echo_hdr) + i] = (char)i;
	}

	iecho->chksum = inet_chksum(iecho, len);
}

/* Ping using the socket ip */
static err_t ping_send(int s, struct ip_addr *addr)
{
	int err;
	struct icmp_echo_hdr *iecho;
	struct sockaddr_in to;
	size_t ping_size = sizeof(struct icmp_echo_hdr) + PING_DATA_SIZE;
	if (!(ping_size <= 0xffff)) {
		print_ping(("ping_size is too big, 0x%x!",  ping_size));
		return ERR_VAL;
	}

	iecho = (struct icmp_echo_hdr *)mem_malloc((mem_size_t)ping_size);
	if (!iecho) {
		return ERR_MEM;
	}

	ping_prepare_echo(iecho, (u16_t)ping_size);

	to.sin_len = sizeof(to);
	to.sin_family = AF_INET;
	inet_addr_from_ipaddr(&to.sin_addr, addr);

	err = lwip_sendto(s, iecho, ping_size, MSG_DONTWAIT, (struct sockaddr*)&to, sizeof(to));
	mem_free(iecho);

	return (err ? ERR_OK : ERR_VAL);
}

static void ping_recv(int s)
{
	char buf[64];
	int fromlen, len;
	struct sockaddr_in from;
	struct ip_hdr *iphdr;
	struct icmp_echo_hdr *iecho;
	//struct _ip_addr *addr;
#if 1
	while((len = lwip_recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr*)&from, (socklen_t*)&fromlen)) > 0)
#else
	len = lwip_recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr*)&from, (socklen_t*)&fromlen);
	print_ping(("lwip_recvfrom len:%d \n", len));
#endif
	{
		if (len >= (sizeof(struct ip_hdr)+sizeof(struct icmp_echo_hdr)))
		{
            ip_addr_t fromaddr;
            inet_addr_to_ipaddr(&fromaddr, &from.sin_addr);
            print_ping(("ping: recv %u.%u.%u.%u  %u ms\n", ip4_addr1_16(&fromaddr), ip4_addr2_16(&fromaddr),\
                      ip4_addr3_16(&fromaddr), ip4_addr4_16(&fromaddr), (sys_now() - ping_time)));

            /*ip_addr_debug_print(PING_DEBUG, &fromaddr);
            print_ping(("%u.%u.%u.%u", ip4_addr1_16(&fromaddr), ip4_addr2_16(&fromaddr),\
                      ip4_addr3_16(&fromaddr), ip4_addr4_16(&fromaddr)));
            print_ping((" %u ms\n", (sys_now() - ping_time)));
            */
/*
			addr = (struct _ip_addr *)&(from.sin_addr);
			print_ping(("ping: recv %d.%d.%d.%d\n", addr->addr0, addr->addr1, addr->addr2, addr->addr3));
*/
			iphdr = (struct ip_hdr *)buf;
			iecho = (struct icmp_echo_hdr *)(buf+(IPH_HL(iphdr) * 4));
			if ((iecho->id == PING_ID) && (iecho->seqno == htons(ping_seq_num)))
			{
                /* do some ping result processing */
                PING_RESULT((ICMPH_TYPE(iecho) == ICMP_ER));
				return;
			}
			else
			{
				print_ping(("ping: drop\n"));
			}
		}
	}

	if (len <= 0)
	{
        print_ping( ("ping: recv - %u ms - timeout\n", (sys_now()-ping_time)));
		print_ping(("ping: timeout\n"));
	}
}

rt_err_t ping(char* target, int time)
{
	int s;
	int timeout = PING_RCV_TIMEO;
	int send_time;
	struct _ip_addr
	{
		rt_uint8_t addr0, addr1, addr2, addr3;
	} *addr;

	struct ip_addr ping_target;


	if (inet_aton(target, (struct in_addr*)&ping_target) == 0) return -RT_ERROR;
	addr = (struct _ip_addr*)&ping_target;

	if ((s = lwip_socket(AF_INET, SOCK_RAW, IP_PROTO_ICMP)) < 0)
	{
	    print_ping(("create socket failled\n"));
		return -RT_ERROR;
	}

	lwip_setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

	send_time = 0;
	if (time <= 0 || time>10)
		time = 4;

	while (send_time < time) {
		send_time ++;

		if (ping_send(s, &ping_target) == ERR_OK) {
			print_ping(("ping(%d/%d): send %d.%d.%d.%d\n", send_time, time, 
					addr->addr0, addr->addr1, addr->addr2, addr->addr3));
			ping_time = sys_now();
			ping_recv(s);
		} else {
			print_ping(("ping: send %d.%d.%d.%d - error\n", addr->addr0, addr->addr1, addr->addr2, addr->addr3));
		}

		rt_thread_delay(get_ticks_of_ms(PING_DELAY)); /* take a delay */
	}

	lwip_close(s);
	
	return RT_EOK;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(ping, ping network host);
#endif

