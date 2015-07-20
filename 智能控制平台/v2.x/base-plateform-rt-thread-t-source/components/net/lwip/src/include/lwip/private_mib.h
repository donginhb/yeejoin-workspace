/**
 * @file
 * Exports Private lwIP MIB 
 */

#ifndef __LWIP_PRIVATE_MIB_H__
#define __LWIP_PRIVATE_MIB_H__

//#include "arch/cc.h"
#include "lwip/opt.h"

#if LWIP_SNMP
#include "lwip/snmp_structs.h"

enum YEEJOIN_TRAP_ID_E {
	YTI_ELOCK_STATUS_CHANGE = 1,	/* 电子锁状态改变, M3需要实现 */
	YTI_DOOR_STATUS_CHANGE,		/* 门状态改变, M3需要实现 */
	YTI_ETH_LINK_STATUS_CHANGE,	/* 以太网口状态改变 */
	YTI_TEMP_RH_OUTOF_GAUGE,	/* 温度或湿度超限, M3需要实现 */
	YTI_UPDATE_INFO_STATUS,		/* 更新通知消息 */
	YTI_ILLEGAL_OPEN_ELOCK,		/* 非法开锁, M3需要实现 */
};


extern const struct mib_array_node mib_private;

extern void lwip_privmib_init(void);
extern void snmp_yeejoin_trap(enum YEEJOIN_TRAP_ID_E trapid);
extern void elock_open_trap(void);
extern void elock_close_trap(void);


#define SNMP_PRIVATE_MIB_INIT() lwip_privmib_init()

#endif

#endif
