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
	YTI_ETH_LINK_STATUS_CHANGE = 1,	/* ��̫����״̬�ı� */
	YTI_485_LINK_STATUS_CHANGE,		/* 485 �ӿ�״̬�ı�*/
	YTI_DEV_TW_STATUS_CHANGE,	/* �ɼ������߽ӿ�ͨ��״̬�ı�*/
	YTI_PT_TW_STATUS_CHANGE,	/* PT�����߽ӿ�ͨ��״̬�ı�*/
	YTI_CT_TW_STATUS_CHANGE,	/* CT�����߽ӿ�ͨ��״̬�ı� */
	YTI_TEMP_RH_OUTOF_GAUGE,	      /* �¶Ȼ�ʪ�ȳ���, M3��Ҫʵ�� */
	YTI_LINE_DATA_READ_STATUS_CHANGE,	/* ��·�����澯״̬ */
	YTI_METER_DATA_READ_STATUS_CHANGE,	/* ���ɼ��澯״̬*/
	YTI_PT_DATA_READ_STATUS_CHANGE,	   /* PT�����ݲɼ��澯״̬*/
	YTI_CT_DATA_READ_STATUS_CHANGE,   	/* CT�����ݸ澯״̬ */
	YTI_PHASEA_THDEXC_ALARM_STATUS,		/* A���������޸澯 */
	YTI_PHASEB_THDEXC_ALARM_STATUS,		/* B���������޸澯 */
	YTI_PHASEC_THDEXC_ALARM_STATUS,		/* C���������޸澯 */
	YTI_TOTAL_THDEXC_ALARM_STATUS,		/* �ܵ����������޸澯 */
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
