/*
 * File      : usart.h
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

#ifndef __USART_H__
#define __USART_H__

#include <rthw.h>
#include <rtthread.h>

#define USART1_DR_Base  0x40013804
#define USART2_DR_Base  0x40004404
#define USART3_DR_Base  0x40004804

/*
 * usart 1-3, uart4 	-- 可以使用dma
 * usart 5   		-- 不能使用dma
 */

/* USART1_REMAP = 0 */
#define UART1_GPIO_TX		GPIO_Pin_9
#define UART1_GPIO_RX		GPIO_Pin_10
#define UART1_GPIO_PORT		GPIOA
#define RCC_APBPeriph_UART1	RCC_APB2Periph_USART1
#define UART1_TX_DMA		DMA1_Channel4
#define UART1_RX_DMA		DMA1_Channel5

#if defined(STM32F10X_LD) || defined(STM32F10X_MD) || defined(STM32F10X_CL)
#define UART2_GPIO_TX	    GPIO_Pin_5
#define UART2_GPIO_RX	    GPIO_Pin_6
#define UART2_GPIO_PORT	    	GPIOD
#define RCC_APBPeriph_UART2	RCC_APB1Periph_USART2
#else /* for STM32F10X_HD */
/* USART2_REMAP = 0 */
#define UART2_GPIO_TX		GPIO_Pin_2
#define UART2_GPIO_RX		GPIO_Pin_3
#define UART2_GPIO_PORT		GPIOA
#define RCC_APBPeriph_UART2	RCC_APB1Periph_USART2
#define UART2_TX_DMA		DMA1_Channel7
#define UART2_RX_DMA		DMA1_Channel6
#endif

/* USART3_REMAP[1:0] = 00 */
#define UART3_GPIO_RX		GPIO_Pin_11
#define UART3_GPIO_TX		GPIO_Pin_10
#define UART3_GPIO_PORT			GPIOB
#define RCC_APBPeriph_UART3	RCC_APB1Periph_USART3
#define UART3_TX_DMA		DMA1_Channel2
#define UART3_RX_DMA		DMA1_Channel3

/* USART4_REMAP = 0 */
#define UART4_GPIO_TX		GPIO_Pin_10
#define UART4_GPIO_RX		GPIO_Pin_11
#define UART4_GPIO_PORT		GPIOC
#define RCC_APBPeriph_UART4	RCC_APB1Periph_UART4
#define UART4_TX_DMA		DMA2_Channel5
#define UART4_RX_DMA		DMA2_Channel3

/* USART5_REMAP = 0 */
#define UART5_GPIO_TX		GPIO_Pin_12
#define UART5_GPIO_RX		GPIO_Pin_2
#define UART5_GPIO_PORT_T	GPIOC
#define UART5_GPIO_PORT_R	GPIOD
#define RCC_APBPeriph_UART5	RCC_APB1Periph_UART5
 

void rt_hw_usart_init(void);

#endif
