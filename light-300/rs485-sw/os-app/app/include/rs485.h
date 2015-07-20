/*
 ******************************************************************************
 * rs485.h
 *
 * 2013-10-15,  creat by David, zhaoshaowei@yeejoin.com
 ******************************************************************************
 */

#ifndef RS485_H_
#define RS485_H_

#include "stm32f10x.h"/* mark by zp */
#include <syscfgdata-common.h>

#define EVENT_BIT_TL16_1_RECV_DATA	0X01
#define EVENT_BIT_TL16_2_RECV_DATA	0X02
#define EVENT_BIT_TL16_3_RECV_DATA	0X04
#define EVENT_BIT_TL16_4_RECV_DATA	0X08
#define EVENT_BIT_TL16_5_RECV_DATA	0X10
#define EVENT_BIT_TL16_6_RECV_DATA	0X20
#define EVENT_BIT_TL16_7_RECV_DATA	0X40
#define EVENT_BIT_TL16_8_RECV_DATA	0X80

#define EVENT_BIT_UART485_1_RECV_DATA	0X0100
#define EVENT_BIT_UART485_2_RECV_DATA	0X0200
#define EVENT_BIT_UART485_3_RECV_DATA	0X0400

enum lt_485if_e {
	LT_485_TL16_U1,	/* must start 0 */
	LT_485_TL16_U2,
	LT_485_TL16_U3,
	LT_485_TL16_U4,
	LT_485_TL16_U5,
	LT_485_TL16_U6,
	LT_485_TL16_U7,
	LT_485_TL16_U8,
	LT_485_EMC_UART,

	LT_485_EM_UART_1,
	LT_485_EM_UART_2,

	LT_485_BUTT,

	LT_485_PHONY	/* 用于要使用,但是，还不能确定使用那个接口 */
};

extern rt_device_t dev_485_tl16_1;
extern rt_device_t dev_485_tl16_2;
extern rt_device_t dev_485_tl16_3;
extern rt_device_t dev_485_tl16_4;
extern rt_device_t dev_485_tl16_5;
extern rt_device_t dev_485_tl16_6;
extern rt_device_t dev_485_tl16_7;
extern rt_device_t dev_485_tl16_8;

//extern struct rt_semaphore uart485_1_rx_byte_sem;
//extern struct rt_semaphore uart485_2_rx_byte_sem;
//extern struct rt_semaphore uart485_3_rx_byte_sem;

extern struct rt_event em_protocol_data_event_set;

extern void init_sys_485(void);
extern rt_size_t send_data_by_485(USART_TypeDef *dev_485, void *data, rt_size_t len);
extern rt_size_t recv_data_by_485(USART_TypeDef *dev_485, void *buf, rt_size_t len);
extern rt_size_t send_data_by_tl16_485(rt_device_t dev_485, void *data, rt_size_t len);
extern rt_size_t recv_data_by_tl16_485(rt_device_t dev_485, void *buf, rt_size_t len);

extern void *get_us485_dev_ptr(enum lt_485if_e rs485_if, int is_stm32_uart_st);
#endif
