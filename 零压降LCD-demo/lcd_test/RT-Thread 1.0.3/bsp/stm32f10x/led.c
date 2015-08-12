/*
 * File      : led.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 */
#include <rtthread.h>
#include <stm32f10x.h>
#include "board.h"

void rt_hw_led_on(rt_uint32_t n)
{
	switch (n) {
	case 0:
		GPIO_SetBits(led1_gpio, led1_pin);
		break;
	case 1:
		GPIO_SetBits(led2_gpio, led2_pin);
		break;
	case 2:
		GPIO_SetBits(led3_gpio, led3_pin);
		break;
	case 3:
		GPIO_SetBits(led4_gpio, led4_pin);
		break;
	default:
		break;
	}
}

void rt_hw_led_off(rt_uint32_t n)
{
	switch (n) {
	case 0:
		GPIO_ResetBits(led1_gpio, led1_pin);
		break;
	case 1:
		GPIO_ResetBits(led2_gpio, led2_pin);
		break;
	case 2:
		GPIO_ResetBits(led3_gpio, led3_pin);
		break;
	case 3:
		GPIO_ResetBits(led4_gpio, led4_pin);
		break;
	default:
		break;
	}
}

#if 0 //def RT_USING_FINSH
#include <finsh.h>
static rt_uint8_t led_inited = 0;
void led(rt_uint32_t led, rt_uint32_t value)
{
	/* init led configuration if it's not inited. */
	if (!led_inited) {
		led_inited = 1;
	}

	if ( led == 0 ) {
		/* set led status */
		switch (value) {
		case 0:
			rt_hw_led_off(0);
			break;
		case 1:
			rt_hw_led_on(0);
			break;
		default:
			break;
		}
	}

	if ( led == 1 ) {
		/* set led status */
		switch (value) {
		case 0:
			rt_hw_led_off(1);
			break;
		case 1:
			rt_hw_led_on(1);
			break;
		default:
			break;
		}
	}
}
FINSH_FUNCTION_EXPORT(led, set led[0 - 1] on[1] or off[0].)
#endif

