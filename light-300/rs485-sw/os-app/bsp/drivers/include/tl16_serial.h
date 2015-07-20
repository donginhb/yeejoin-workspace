/*
 ******************************************************************************
 * tl16_serial.h
 *
 *  Created on: 2015-1-4
 *      Author: David, zhaoshaowei@yeejoin.com
 *
 * COPYRIGHT (C) 2015, YEEJOIN (Beijing) Technology Development Co., Ltd.
 ******************************************************************************
 */


#ifndef TL16_SERIAL_H_
#define TL16_SERIAL_H_

#include <rthw.h>
#include <rtthread.h>

/* STM32F10x library definitions */
#include <stm32f10x.h>

#include <tl16c554_hal.h>
#include <tl16_uart.h>



#define TL16_UART_RX_BUFFER_SIZE		(128)


struct tl16_serial_int_rx {
	rt_uint8_t  *rx_buffer;
	rt_uint32_t buf_size;
	rt_uint32_t read_index, save_index;
};

struct tl16_serial_device {
	struct tl16_uart_st *uart_device;

	/* rx structure */
	struct tl16_serial_int_rx *int_rx;
};


extern rt_err_t rt_hw_tl16_serial_register(rt_device_t device, const char* name,
		rt_uint32_t flag, struct tl16_serial_device *serial);
extern void rt_hw_tl16_serial_isr(rt_device_t device);

#endif /* TL16_SERIAL_H_ */
