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
	YTI_ELOCK_STATUS_CHANGE = 1,	/* ������״̬�ı�, M3��Ҫʵ�� */
	YTI_DOOR_STATUS_CHANGE,		/* ��״̬�ı�, M3��Ҫʵ�� */
	YTI_ETH_LINK_STATUS_CHANGE,	/* ��̫����״̬�ı� */
	YTI_TEMP_RH_OUTOF_GAUGE,	/* �¶Ȼ�ʪ�ȳ���, M3��Ҫʵ�� */
	YTI_UPDATE_INFO_STATUS,		/* ����֪ͨ��Ϣ */
	YTI_ILLEGAL_OPEN_ELOCK,		/* �Ƿ�����, M3��Ҫʵ�� */
};


extern const struct mib_array_node mib_private;

extern void lwip_privmib_init(void);
extern void snmp_yeejoin_trap(enum YEEJOIN_TRAP_ID_E trapid);
extern void elock_open_trap(void);
extern void elock_close_trap(void);


#define SNMP_PRIVATE_MIB_INIT() lwip_privmib_init()

#endif

#endif
