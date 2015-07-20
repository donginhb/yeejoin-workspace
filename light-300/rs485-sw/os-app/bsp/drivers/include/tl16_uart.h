/*
 ******************************************************************************
 * tl16_uart.h
 *
 *  Created on: 2015-1-4
 *      Author: David, zhaoshaowei@yeejoin.com
 *
 * COPYRIGHT (C) 2015, YEEJOIN (Beijing) Technology Development Co., Ltd.
 ******************************************************************************
 */


#ifndef TL16_UART_H_
#define TL16_UART_H_

#include <rthw.h>
#include <rtdef.h>

#include <tl16c554_hal.h>

extern struct rt_device tl16_uart1_device;
extern struct rt_device tl16_uart2_device;
extern struct rt_device tl16_uart3_device;
extern struct rt_device tl16_uart4_device;
extern struct rt_device tl16_uart5_device;
extern struct rt_device tl16_uart6_device;
extern struct rt_device tl16_uart7_device;
extern struct rt_device tl16_uart8_device;

extern struct rt_mutex tl16_x_lock;

struct tl16_uart_st {
	enum tl16_x_csx_addr_e x_csx;
	unsigned int overrun_err_cnt;
	unsigned int parity_err_cnt;
	unsigned int framing_err_cnt;
	unsigned int break_int_bit_cnt;
	unsigned int fifo_err_cnt;
};

extern void rt_hw_tl16_usart_init(void);
extern rt_device_t get_tl16dev_from_x_csx(enum tl16_x_csx_addr_e x_csx);
extern struct tl16_serial_device *get_tl16uart_from_x_csx(enum tl16_x_csx_addr_e x_csx);
extern void print_tl16_uart_err_info(void);
extern int tl16_485_tx_ctrl(rt_device_t device, int is_enable);
extern int tl16_485_rx_ctrl(rt_device_t device, int is_enable);
extern int is_tl16_485_tx_over(rt_device_t device);

#endif /* TL16_UART_H_ */
