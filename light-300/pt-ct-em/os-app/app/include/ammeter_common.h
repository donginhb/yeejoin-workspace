/*
 ******************************************************************************
 * ammeter_common.h
 *
 *  Created on: 2015-4-7
 *      Author: David, zhaoshaowei@yeejoin.com
 *
 * COPYRIGHT (C) 2015, YEEJOIN (Beijing) Technology Development Co., Ltd.
 ******************************************************************************
 */


#ifndef AMMETER_COMMON_H_
#define AMMETER_COMMON_H_


/*** 电能表所用串口 ***/
enum ammeter_uart_e {
	AMMETER_UART1 = 1,
	AMMETER_UART2,
	AMMETER_UART3,
};


enum frame_error_e {
	FRAME_E_OK = 0x00,   	/* 执行正确，返回0 */
	FRAME_E_ERROR,         	/* 一般性错误 */
	FRAME_E_REVE_TIMEOUT,	/* 没有接收到645帧数据,接收超时 */
	FRAME_E_FRAG,     		/* 此帧是个片段 */
 	FRAME_E_CS_ERR,        	/* 校验错误 */
 	FRAME_E_DATA_ERR,      	/* 数据错误 */
 	FRAME_E_FLAG_ERROR,    	/* 不能识别此数据标识 */
 	FRAME_E_ADDR_MISMATCH,	/* 接收帧数据与发送命令地址不匹配 */
 	FRAME_E_FLAG_MISMATCH,	/* 接收帧数据与发送命令数据标识不匹配 */
};


/********************各种规约协议******************/
enum ammeter_protocal_e {
	AP_PROTOCOL_UNKNOWN,
	AP_PROTOCOL_645_1997,
	AP_PROTOCOL_645_2007,
	AP_PROTOCOL_DLMS,
	AP_PROTOCOL_WS,
	AP_PROTOCOL_IEC1107,
	AP_PROTOCOL_ACTARIS,
	AP_PROTOCOL_EDMI,
	AP_PROTOCOL_SIMENS,
	AP_PROTOCOL_NOTHING
};



struct ammeter_time
{
	rt_uint16_t year;
	rt_uint8_t month;
	rt_uint8_t day;
	rt_uint8_t hour;
	rt_uint8_t minite;
	rt_uint8_t seconds;
	rt_uint8_t resv;
};

#endif /* AMMETER_COMMON_H_ */
