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
	DMN_SINKINFO = 0,
	DMN_ADE7880,		/* 用于7880模块 */
	DMN_SI4432,		/* 用于4432模块 */
	DMN_METER_PROTO,	/* 用于电表通信协议模块, meter communication protocol */
	DMN_PRIV_MIB,		/* 用于私有mib模块 */
	DMN_TRANS_PTCT_DATA,	/* 用于传输ptc、ctc采集数据的模块 */

	DMN_NUM	/* must at the end */
};

/* 用于sinkinfo模块的子模块/打印级别 */
enum dsw_sinkinfo_module_no_e {
	DSMN_SI_BODY,	/* sinkinfo的主体, 协商超时以及波特率设置, 脉冲中断打印 */
	DSMN_SI_DATA,	/* sinkinfo处理缓冲区数据 */

	DSMN_SI_NUM /* must at the end */

};

/* 用于7880模块的子模块/打印级别 */
enum dsw_ade7880_module_no_e {
	DSMN_ADE7880_HW,	/* ade7880寄存器调试打印信息 */
	DSMN_ADE7880_API,	/* ade7880信号获取调试打印信息 */

	DSMN_ADE7880_NUM /* must at the end */
};

/* 用于4432模块的子模块/打印级别 */
enum dsw_si4432_module_no_e {
	DSMN_SI4432_MAC,		/* 4432的mac代码使用 */
	DSMN_SI4432_APP_COMM,	/* app的common代码使用 */
	DSMN_SI4432_DOT2MDOT,	/* 点对多点代码使用 */

	DSMN_SI4432_NUM /* must at the end */
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

/* 用于私有mib模块的子模块/打印级别 */
enum dsw_priv_mib_module_no_e {
	DSMN_PM_LWIP,	/* LWIP信息打印 */
	DSMN_PM_MIB,	/* MIB接口信息打印 */

	DSMN_PM_NUM /* must at the end */
};

/* 用于传输ptc、ctc采集数据的模块的子模块/打印级别 */
enum dsw_trans_ptct_data_module_no_e {
	DSMN_TPD_485BUS,	/* 用于基于485总线通信的代码 */
	DSMN_TPD_TCP,		/* 用于基于tcp的数据分发代码 */

	DSMN_TPD_NUM /* must at the end */
};


#define debug_info(m, sm, x)	do { if ((m) && (sm)) { printf_syn x; } } while(0)
#define is_sub_m_on(subm_vect, no) ((subm_vect) & (1<<no))


/*
 * 以下是用于sinkinfo模块的打印宏定义
 * */
#define sinki_debug_body(x)	debug_info(dsw_module[DMN_SINKINFO], \
		is_sub_m_on(dsw_sub_module[DMN_SINKINFO], DSMN_SI_BODY), x)

#define sinki_debug_data(x)	debug_info(dsw_module[DMN_SINKINFO], \
		is_sub_m_on(dsw_sub_module[DMN_SINKINFO], DSMN_SI_DATA), x)


/*
 * 以下是用于 ADE7880 模块的打印宏定义
 * */
#define ade7880_debug_hw(x)	debug_info(dsw_module[DMN_ADE7880], \
		is_sub_m_on(dsw_sub_module[DMN_ADE7880], DSMN_ADE7880_HW), x)
#define ade7880_debug_api(x)	debug_info(dsw_module[DMN_ADE7880], \
		is_sub_m_on(dsw_sub_module[DMN_ADE7880], DSMN_ADE7880_API), x)

/*
 * 以下是用于 SI4432 模块的打印宏定义
 * */
#define si4432_debug_mac(x)		debug_info(dsw_module[DMN_SI4432], \
		is_sub_m_on(dsw_sub_module[DMN_SI4432], DSMN_SI4432_MAC), x)
#define si4432_debug_app_comm(x)	debug_info(dsw_module[DMN_SI4432], \
		is_sub_m_on(dsw_sub_module[DMN_SI4432], DSMN_SI4432_APP_COMM), x)
#define si4432_debug_dot2mdot(x)	debug_info(dsw_module[DMN_SI4432], \
		is_sub_m_on(dsw_sub_module[DMN_SI4432], DSMN_SI4432_DOT2MDOT), x)


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
 * 以下是用于 私有mib 模块的打印宏定义
 * */
#define priv_mib_debug_lwip(x)	debug_info(dsw_module[DMN_PRIV_MIB], \
		is_sub_m_on(dsw_sub_module[DMN_PRIV_MIB], DSMN_PM_LWIP), x)
#define priv_mib_debug_mib(x)	debug_info(dsw_module[DMN_PRIV_MIB], \
		is_sub_m_on(dsw_sub_module[DMN_PRIV_MIB], DSMN_PM_MIB), x)


/*
 * 以下是用于 传输ptc、ctc采集数据 模块的打印宏定义
 * */
#define trans_ptct_debug_485(x)	debug_info(dsw_module[DMN_TRANS_PTCT_DATA], \
		is_sub_m_on(dsw_sub_module[DMN_TRANS_PTCT_DATA], DSMN_TPD_485BUS), x)
#define trans_ptct_debug_tcp(x)	debug_info(dsw_module[DMN_TRANS_PTCT_DATA], \
		is_sub_m_on(dsw_sub_module[DMN_TRANS_PTCT_DATA], DSMN_TPD_TCP), x)


/*
 * 全局变量声明
 * */
extern unsigned char dsw_module[DMN_NUM];
extern unsigned long dsw_sub_module[DMN_NUM];

extern void dsw_set(enum dsw_module_no_e m, int sm, int on_off);

#endif /* DEBUG_SW_H_ */
