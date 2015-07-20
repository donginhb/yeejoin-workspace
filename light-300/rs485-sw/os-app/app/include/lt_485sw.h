/*
 ******************************************************************************
 * lt_485sw.h
 *
 *  Created on: 2015-01-26
 *      Author: David, zhaoshaowei@yeejoin.com
 *
 * COPYRIGHT (C) 2015, YEEJOIN (Beijing) Technology Development Co., Ltd.
 ******************************************************************************
 */

#ifndef OS_APP_APP_INCLUDE_LT_485SW_H_
#define OS_APP_APP_INCLUDE_LT_485SW_H_

#include <rs485.h>


#define EVENT_BIT_EMC_485_RECV_DATA	EVENT_BIT_UART485_2_RECV_DATA
#define EVENT_BIT_EM_485_RECV_DATA	EVENT_BIT_UART485_1_RECV_DATA
#define EMC_AUTO_ENTER_NEGOTIATION_UART	UART4


extern struct rt_mutex em_grp1_if_mutex;
extern struct rt_mutex em_grp2_if_mutex;

extern struct rt_semaphore em_grp1_if_lu_sem;
extern struct rt_semaphore em_grp2_if_lu_sem;

extern struct rt_mailbox send_cmd2em_grp1_mb;
extern struct rt_mailbox send_cmd2em_grp2_mb;
//extern struct rt_timer emc_auto_enter_negotiation_state_timer;

//extern unsigned protoc_inter_byte_toparam;
extern enum ammeter_protocol_e protocol_use_by_emc;


extern void lt_485sw_init(void);
extern unsigned int get_em_protocol_data_event_bit(enum lt_485if_e rs485_if);
extern int is_tl16_interface(enum lt_485if_e rs485_if);
extern struct protoc_analy_info_st *get_analys_info_ptr(enum lt_485if_e rs485_if);
extern int tl16_set_uartparam_use_by_em(enum lt_485if_e rs485_if, enum lt_485if_e lt_if_use_by_em);
extern int emc_set_uartparam_use_by_em(enum lt_485if_e lt_if_use_by_em);

extern int reset_auto_negotiation_timer(void);

extern rt_err_t lt_485_em_grpx_mutex_take(enum lt_485if_e rs485_if, enum lt_485if_e lt_if_use_by_em);
extern rt_err_t lt_485_em_grpx_mutex_release(enum lt_485if_e lt_if_use_by_em);

#endif /* OS_APP_APP_INCLUDE_LT_485SW_H_ */
