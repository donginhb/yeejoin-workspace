/*
 ******************************************************************************
 * tl16_serial.c
 *
 *  Created on: 2015-1-4
 *      Author: David, zhaoshaowei@yeejoin.com
 *
 * COPYRIGHT (C) 2015, YEEJOIN (Beijing) Technology Development Co., Ltd.
 ******************************************************************************
 */



#include <tl16_serial.h>
#include "board.h"



/* RT-Thread Device Interface */
static rt_err_t rt_tl16_serial_init (rt_device_t dev)
{
	struct tl16_serial_device* uart = (struct tl16_serial_device*) dev->user_data;

	if (!(dev->flag & RT_DEVICE_FLAG_ACTIVATED)) {
		uart->int_rx->read_index = 0;
		uart->int_rx->save_index = 0;

		/* Enable USART */
//		USART_Cmd(uart->uart_device->.x_csx, ENABLE);
		dev->flag |= RT_DEVICE_FLAG_ACTIVATED;
	}

	return RT_EOK;
}

static rt_err_t rt_tl16_serial_open(rt_device_t dev, rt_uint16_t oflag)
{
	return RT_EOK;
}

static rt_err_t rt_tl16_serial_close(rt_device_t dev)
{
	return RT_EOK;
}

static rt_size_t rt_tl16_serial_read (rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
	rt_uint8_t* ptr;
	rt_err_t err_code;
	struct tl16_serial_device* uart = (struct tl16_serial_device*)dev->user_data;
	rt_uint32_t buf_size = uart->int_rx->buf_size;

	ptr = buffer;
	err_code = RT_EOK;

	if (dev->flag & RT_DEVICE_FLAG_INT_RX) {
		if (RT_EOK != rt_mutex_take(&tl16_x_lock, RT_WAITING_FOREVER)) {
			err_code = -RT_ERROR;
			goto ret_entry;
		}

		/* interrupt mode Rx */
		while (size) {
//			rt_base_t level;

			/* disable interrupt */
//			level = rt_hw_interrupt_disable();
			if (uart->int_rx->read_index != uart->int_rx->save_index) {
				/* read a character */
				*ptr++ = uart->int_rx->rx_buffer[uart->int_rx->read_index];
				size--;

				/* move to next position */
				uart->int_rx->read_index ++;
				if (uart->int_rx->read_index >= buf_size)
					uart->int_rx->read_index = 0;
			} else {
				/* set error code */
				err_code = -RT_EEMPTY;

				/* enable interrupt */
//				rt_hw_interrupt_enable(level);
				break;
			}

			/* enable interrupt */
//			rt_hw_interrupt_enable(level);
		}

		rt_mutex_release(&tl16_x_lock);
	} else {
		/* polling mode not support*/
		RT_ASSERT(0);
		err_code = RT_EIO;
	}

ret_entry:
	/* set error code */
	rt_set_errno(err_code);
	return (rt_uint32_t)ptr - (rt_uint32_t)buffer;
}


static rt_size_t rt_tl16_serial_write (rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
	rt_uint8_t* ptr;
	rt_err_t err_code;
	struct tl16_serial_device* uart;

	err_code = RT_EOK;
	ptr = (rt_uint8_t*)buffer;
	uart = (struct tl16_serial_device*)dev->user_data;

	if (dev->flag & RT_DEVICE_FLAG_INT_TX) {
		/* interrupt mode Tx, does not support */
		RT_ASSERT(0);
	} else {
		/* polling mode */
		/* write data directly */
		while (size) {
			while (!if_can_send2tl16_uart(uart->uart_device->x_csx))
				;
			if (RT_EOK != rt_mutex_take(&tl16_x_lock, RT_WAITING_FOREVER)) {
				err_code = -RT_ERROR;
				goto ret_entry;
			}

			tl16_write_reg(create_tl16_reg_addr(uart->uart_device->x_csx, TL16_THR_REG), (*ptr & 0xFF));
			++ptr;
			--size;

			rt_mutex_release(&tl16_x_lock);

#if USE_STM32_IWDG
			/* Reloads IWDG counter with value defined in the reload register */
			/* #define KR_KEY_Reload    ((uint16_t)0xAAAA) */
			IWDG->KR = 0xAAAA;
#endif
		}
	}

ret_entry:
	/* set error code */
	rt_set_errno(err_code);

	return (rt_uint32_t)ptr - (rt_uint32_t)buffer;
}

static rt_err_t rt_tl16_serial_control (rt_device_t dev, rt_uint8_t cmd, void *args)
{
	struct tl16_serial_device* uart;
	rt_err_t err_code = RT_EOK;

	RT_ASSERT(dev != RT_NULL);

	uart = (struct tl16_serial_device*)dev->user_data;

	if (RT_EOK != rt_mutex_take(&tl16_x_lock, RT_WAITING_FOREVER)) {
		err_code = -RT_ERROR;
		goto ret_entry;
	}

	switch (cmd) {
	case RT_DEVICE_CTRL_SUSPEND:
		/* suspend device */
		dev->flag |= RT_DEVICE_FLAG_SUSPENDED;
//		USART_Cmd(uart->uart_device->x_csx, DISABLE);
		tl16_set_ier(uart->uart_device->x_csx, TL16C554_RX_LINE_STATUS, 0);
//		tl16_set_ier(uart->uart_device->x_csx, TL16C554_TX_HOLD_REG_EMPTY, 0);
		tl16_set_ier(uart->uart_device->x_csx, TL16C554_RX_DATA_AVAILABLE, 0);

		break;

	case RT_DEVICE_CTRL_RESUME:
		/* resume device */
		dev->flag &= ~RT_DEVICE_FLAG_SUSPENDED;
//		USART_Cmd(uart->uart_device->x_csx, ENABLE);
		tl16_set_ier(uart->uart_device->x_csx, TL16C554_RX_LINE_STATUS, 1);
//		tl16_set_ier(uart->uart_device->x_csx, TL16C554_TX_HOLD_REG_EMPTY, 1);
		tl16_set_ier(uart->uart_device->x_csx, TL16C554_RX_DATA_AVAILABLE, 1);

		break;

	case RT_DEVICE_CTRL_CLR_RXBUF:
		uart->int_rx->read_index = 0;
		uart->int_rx->save_index = 0;
		break;

	}
	rt_mutex_release(&tl16_x_lock);

ret_entry:
	return err_code;
}

/*  */
rt_err_t rt_hw_tl16_serial_register(rt_device_t device, const char* name, rt_uint32_t flag, struct tl16_serial_device *serial)
{
	RT_ASSERT(device != RT_NULL);

	if ((flag & RT_DEVICE_FLAG_DMA_RX) ||
		(flag & RT_DEVICE_FLAG_INT_TX)) {
		RT_ASSERT(0);
	}

	device->type 		= RT_Device_Class_Char;
	device->rx_indicate = RT_NULL;
	device->tx_complete = RT_NULL;
	device->init 		= rt_tl16_serial_init;
	device->open		= rt_tl16_serial_open;
	device->close		= rt_tl16_serial_close;
	device->read 		= rt_tl16_serial_read;
	device->write 		= rt_tl16_serial_write;
	device->control 	= rt_tl16_serial_control;
	device->user_data	= serial;

	/* register a character device */
	return rt_device_register(device, name, RT_DEVICE_FLAG_RDWR | flag);
}

#define MY_TEST_UART 0
/* ISR for serial interrupt */
void rt_hw_tl16_serial_isr(rt_device_t device)
{
	struct tl16_serial_device* uart = (struct tl16_serial_device*) device->user_data;
	rt_uint32_t buf_size = uart->int_rx->buf_size;

	rt_uint32_t *read_index, *save_index;
	enum tl16_x_csx_addr_e x_csx = uart->uart_device->x_csx;
	rt_uint8_t  *rx_buffer = uart->int_rx->rx_buffer;

#if MY_TEST_UART
	char str[RT_NAME_MAX+4];
#endif

	read_index = &uart->int_rx->read_index;
	save_index = &uart->int_rx->save_index;
	/* save on rx buffer */
	while (if_tl16_uart_not_empty(x_csx)) {
//		rt_base_t level;

//		rt_kprintf("%s(), x_csx:%d", __func__, x_csx);

		/* disable interrupt */
//		level = rt_hw_interrupt_disable();
#if 0 //USE_STM32_IWDG
		/* Reloads IWDG counter with value defined in the reload register */
		/* #define KR_KEY_Reload    ((uint16_t)0xAAAA) */
		IWDG->KR = 0xAAAA;
#endif

		/* save character */
		rx_buffer[*save_index] = tl16_read_reg(create_tl16_reg_addr(x_csx, TL16_RBR_REG));
		++*save_index;
		if (*save_index >= buf_size)
			*save_index = 0;

		/* if the next position is read index, discard this 'read char' */
		if (*save_index == *read_index) {
			++*read_index;
			if (*read_index >= buf_size)
				*read_index = 0;
		}

		/* enable interrupt */
//		rt_hw_interrupt_enable(level);
	}

#if MY_TEST_UART

	rt_memcpy(str, device->parent.name, RT_NAME_MAX);
	str[RT_NAME_MAX] = '\0';
	rt_kprintf("**%s(), %d, dev-name:%s\n", __FUNCTION__, x_csx, str);
#endif
	/* invoke callback */
	if (device->rx_indicate != RT_NULL) {
		rt_size_t rx_length;

		/* get rx length */
		rx_length = *read_index > *save_index ?
					buf_size - *read_index + *save_index :
					*save_index - *read_index;

		device->rx_indicate(device, rx_length);
	} else {
		printf_syn("warning: x_csx-%d have not rx_indicate\n", x_csx);
	}


}
