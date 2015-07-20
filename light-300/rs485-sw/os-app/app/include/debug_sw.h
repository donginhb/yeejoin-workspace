/*
 ******************************************************************************
 * debug_sw.h
 *
 *  Created on: 2014-10-16
 *      Author: David, zhaoshaowei@yeejoin.com
 *
 * COPYRIGHT (C) 2014, YEEJOIN (Beijing) Technology Development Co., Ltd.
 ******************************************************************************
 */


#ifndef DEBUG_SW_H_
#define DEBUG_SW_H_

#include <rtthread.h>


/*
 * 主模块编号从0开始，最大值与内存有关
 * 从模块编号从0开始，最大到31
 * */
enum dsw_module_no_e {
	DMN_485SW = 0,
	DMN_METER_PROTO,	/* 用于电表通信协议模块, meter communication protocol */

	DMN_NUM	/* must at the end */
};

/* 用于485sw模块的子模块/打印级别 */
enum dsw_485sw_module_no_e {
	DSMN_485SW_BODY,	/* 485sw的主体 */
	DSMN_485SW_NEGOTIATION_CMD_DATA,	/* 打印485sw协商数据 */
	DSMN_485SW_SENDCMD2EM_DATA,		/* 打印485sw发送给em的数据 */
	DSMN_485SW_ACK_FRAME_DATA,		/* 打印485sw转发的ack数据 */

	DSMN_485SW_ALL_SUM_MOD,
	DSMN_485SW_NUM		/* must at the end */
};


/* 用于电表通信协议模块的子模块/打印级别 */
enum dsw_meter_proto_module_no_e {
	DSMN_MP_MAIN,		/* 主体函数接收数据打印 */
	DSMN_MP_USART_RECV,	/* 串口接收数据打印 */
	DSMN_MP_645_1997, 	/* 645_1997协议发送数据打印 */
	DSMN_MP_645_2007, 	/* 645_2007协议发送数据打印  */
	DSMN_MP_DLMS, 		/* DLMS协议发送数据打印  */
	DSMN_MP_WS, 		/* WS协议发送数据打印  */
	DSMN_MP_IEC1107, 	/* IEC1107协议发送数据打印  */
	DSMN_MP_ACTARIS, 	/* ACTARIS协议发送数据打印  */
	DSMN_MP_EDMI, 		/* EDMI协议发送数据打印  */
	DSMN_MP_SIMENS, 	/* SIMENS协议发送数据打印 */

	DSMN_MP_NUM /* must at the end */
};



#define debug_info(m, sm, x)	do { if ((m) && (sm)) { printf_syn x; } } while(0)
#define is_sub_m_on(subm_vect, no) ((subm_vect) & (1<<no))


/*
 * 以下是用于485sw模块的打印宏定义
 * */
#define sw485_debug_body(x)	debug_info(dsw_module[DMN_485SW], \
		is_sub_m_on(dsw_sub_module[DMN_485SW], DSMN_485SW_BODY), x)

#define sw485_debug_negoti(x)	debug_info(dsw_module[DMN_485SW], \
		is_sub_m_on(dsw_sub_module[DMN_485SW], DSMN_485SW_NEGOTIATION_CMD_DATA), x)

#define sw485_debug_sendcmd(x)	debug_info(dsw_module[DMN_485SW], \
		is_sub_m_on(dsw_sub_module[DMN_485SW], DSMN_485SW_SENDCMD2EM_DATA), x)

#define sw485_debug_ack(x)	debug_info(dsw_module[DMN_485SW], \
		is_sub_m_on(dsw_sub_module[DMN_485SW], DSMN_485SW_ACK_FRAME_DATA), x)


/*
 * 以下是用于 电表通信协议 模块的打印宏定义
 * */
#define meterp_debug_main(x)	debug_info(dsw_module[DMN_METER_PROTO], \
		is_sub_m_on(dsw_sub_module[DMN_METER_PROTO], DSMN_MP_MAIN), x)
#define meterp_debug_uart(x)	debug_info(dsw_module[DMN_METER_PROTO], \
		is_sub_m_on(dsw_sub_module[DMN_METER_PROTO], DSMN_MP_USART_RECV), x)
#define meterp_debug_97(x)	debug_info(dsw_module[DMN_METER_PROTO], \
		is_sub_m_on(dsw_sub_module[DMN_METER_PROTO], DSMN_MP_645_1997), x)
#define meterp_debug_07(x)	debug_info(dsw_module[DMN_METER_PROTO], \
		is_sub_m_on(dsw_sub_module[DMN_METER_PROTO], DSMN_MP_645_2007), x)
#define meterp_debug_dlms(x)	debug_info(dsw_module[DMN_METER_PROTO], \
		is_sub_m_on(dsw_sub_module[DMN_METER_PROTO], DSMN_MP_DLMS), x)
#define meterp_debug_ws(x)	debug_info(dsw_module[DMN_METER_PROTO], \
		is_sub_m_on(dsw_sub_module[DMN_METER_PROTO], DSMN_MP_WS), x)
#define meterp_debug_iec1107(x)	debug_info(dsw_module[DMN_METER_PROTO], \
		is_sub_m_on(dsw_sub_module[DMN_METER_PROTO], DSMN_MP_IEC1107), x)
#define meterp_debug_actaris(x)	debug_info(dsw_module[DMN_METER_PROTO], \
		is_sub_m_on(dsw_sub_module[DMN_METER_PROTO], DSMN_MP_ACTARIS), x)
#define meterp_debug_edmi(x)	debug_info(dsw_module[DMN_METER_PROTO], \
		is_sub_m_on(dsw_sub_module[DMN_METER_PROTO], DSMN_MP_EDMI), x)
#define meterp_debug_simens(x)	debug_info(dsw_module[DMN_METER_PROTO], \
		is_sub_m_on(dsw_sub_module[DMN_METER_PROTO], DSMN_MP_SIMENS), x)



/*
 * 全局变量声明
 * */
extern volatile unsigned char dsw_module[DMN_NUM];
extern volatile unsigned long dsw_sub_module[DMN_NUM];

#endif /* DEBUG_SW_H_ */
