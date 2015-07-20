/**
 * @file
 * Exports Private lwIP MIB
 */

#ifndef __LWIP_PRIVATE_MIB_H__
#define __LWIP_PRIVATE_MIB_H__

//#include "arch/cc.h"
#include "lwip/opt.h"
#include "lwip/snmp_structs.h"

#define lt300_SYS_DESCR_INFO_MAX_LEN 32
#define lt300_SYS_DEV_INFO_MAX_LEN   10
#define PHASEA	1
#define PHASEB	2
#define PHASEC	3

#if LWIP_SNMP

#if 1
enum YEEJOIN_TRAP_ID_E {
	YTI_ETH_LINK_STATUS_CHANGE = 1,	/* 以太网口状态改变 */
	YTI_485_LINK_STATUS_CHANGE,		/* 485 接口状态改变*/
	YTI_DEV_TW_STATUS_CHANGE,	/* 采集器无线接口通信状态改变*/
	YTI_PT_TW_STATUS_CHANGE,	/* PT侧无线接口通信状态改变*/
	YTI_CT_TW_STATUS_CHANGE,	/* CT侧无线接口通信状态改变 */
	YTI_TEMP_RH_OUTOF_GAUGE,	      /* 温度或湿度超限, M3需要实现 */
	YTI_LINE_DATA_READ_STATUS_CHANGE,	/* 线路计量告警状态 */
	YTI_METER_DATA_READ_STATUS_CHANGE,	/* 电表采集告警状态*/
	YTI_PT_DATA_READ_STATUS_CHANGE,	   /* PT侧数据采集告警状态*/
	YTI_CT_DATA_READ_STATUS_CHANGE,   	/* CT侧数据告警状态 */
	YTI_PHASEA_THDEXC_ALARM_STATUS,		/* A相数据误差超限告警 */
	YTI_PHASEB_THDEXC_ALARM_STATUS,		/* B相数据误差超限告警 */
	YTI_PHASEC_THDEXC_ALARM_STATUS,		/* C相数据误差超限告警 */
	YTI_TOTAL_THDEXC_ALARM_STATUS,		/* 总电能数据误差超限告警 */
};
#endif

extern const struct mib_array_node mib_private;

extern void lwip_privmib_init(void);
extern void snmp_yeejoin_trap(enum YEEJOIN_TRAP_ID_E trapid);
extern void rs485_open_trap(void);
extern void rs485_close_trap(void);
extern void ocstrncpy(u8_t *dst, u8_t *src, u16_t n);

#define SNMP_PRIVATE_MIB_INIT() lwip_privmib_init()


#endif

#endif
