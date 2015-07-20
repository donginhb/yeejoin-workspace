/*
 * File      : usart.c
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
 * 2010-03-29     Bernard      remove interrupt Tx and DMA Rx mode
 */

#include "usart.h"
#include <serial.h>
#include <stm32f10x_dma.h>
#include "rtconfig.h"
#include "board.h"

/*
 * Use UART1 as console output and finsh input
 * interrupt Rx and poll Tx (stream mode)
 *
 * Use UART2 with interrupt Rx and poll Tx
 * Use UART3 with DMA Tx and interrupt Rx -- DMA channel 2
 *
 * USART DMA setting on STM32
 * USART1 Tx --> DMA Channel 4
 * USART1 Rx --> DMA Channel 5
 * USART2 Tx --> DMA Channel 7
 * USART2 Rx --> DMA Channel 6
 * USART3 Tx --> DMA Channel 2
 * USART3 Rx --> DMA Channel 3
 */

#ifdef RT_USING_UART1
rt_uint8_t  uart1_rx_buffer[UART_MRX_BUFFER_SIZE];
struct stm32_serial_int_rx uart1_int_rx = {uart1_rx_buffer, sizeof(uart1_rx_buffer), 0, 0};
struct stm32_serial_device uart1 = {
	USART1,
	&uart1_int_rx,
	RT_NULL
};
struct rt_device uart1_device;
#endif

#ifdef RT_USING_UART2
rt_uint8_t  uart2_rx_buffer[UART_SRX_BUFFER_SIZE];
struct stm32_serial_int_rx uart2_int_rx = {uart2_rx_buffer, sizeof(uart2_rx_buffer), 0, 0};
struct stm32_serial_device uart2 = {
	USART2,
	&uart2_int_rx,
	RT_NULL
};
struct rt_device uart2_device;
#endif

#ifdef RT_USING_UART3
rt_uint8_t  uart3_rx_buffer[UART_SRX_BUFFER_SIZE];
struct stm32_serial_int_rx uart3_int_rx = {uart3_rx_buffer, sizeof(uart3_rx_buffer), 0, 0};
#if RT_UART3_USE_DMA
struct stm32_serial_dma_tx uart3_dma_tx;
#endif
struct stm32_serial_device uart3 = {
	USART3,
	&uart3_int_rx,
#if RT_UART3_USE_DMA
	&uart3_dma_tx
#else
	RT_NULL
#endif
};
struct rt_device uart3_device;
#endif

struct rt_semaphore uart3_tx_sem;
struct rt_timer uart3_ack_timer;

#define USART1_DR_Base  0x40013804
#define USART2_DR_Base  0x40004404
#define USART3_DR_Base  0x40004804

static void DMA_Configuration_uart(void)
{
#if RT_UART3_USE_DMA
	DMA_InitTypeDef DMA_InitStructure;

	/* fill init structure */
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

	/* DMA1 Channel5 (triggered by USART3 Tx event) Config */
	DMA_DeInit(UART3_TX_DMA);
	DMA_InitStructure.DMA_PeripheralBaseAddr = USART3_DR_Base;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	/* As we will set them before DMA actually enabled, the DMA_MemoryBaseAddr
	 * and DMA_BufferSize are meaningless. So just set them to proper values
	 * which could make DMA_Init happy.
	 */
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)0;
	DMA_InitStructure.DMA_BufferSize = 1;
	DMA_Init(UART3_TX_DMA, &DMA_InitStructure);
	DMA_ITConfig(UART3_TX_DMA, DMA_IT_TC | DMA_IT_TE, ENABLE);
	DMA_ClearFlag(DMA1_FLAG_TC2);
#endif
}

/*
 * Init all related hardware in here
 * rt_hw_serial_init() will register all supported USART device
 */
void rt_hw_usart_init()
{
	USART_InitTypeDef USART_InitStructure;
	USART_ClockInitTypeDef USART_ClockInitStructure;

	DMA_Configuration_uart();

	/* uart init */
#ifdef RT_USING_UART1
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_ClockInitStructure.USART_Clock = USART_Clock_Disable;
	USART_ClockInitStructure.USART_CPOL = USART_CPOL_Low;
	USART_ClockInitStructure.USART_CPHA = USART_CPHA_2Edge;
	USART_ClockInitStructure.USART_LastBit = USART_LastBit_Disable;
	USART_Init(USART1, &USART_InitStructure);
	USART_ClockInit(USART1, &USART_ClockInitStructure);

	/* register uart1 */
	rt_hw_serial_register(&uart1_device, "uart1",
						  RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_STREAM,
						  &uart1);

	/* enable interrupt */
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
#endif

#ifdef RT_USING_UART2
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_ClockInitStructure.USART_Clock = USART_Clock_Disable;
	USART_ClockInitStructure.USART_CPOL = USART_CPOL_Low;
	USART_ClockInitStructure.USART_CPHA = USART_CPHA_2Edge;
	USART_ClockInitStructure.USART_LastBit = USART_LastBit_Disable;
	USART_Init(USART2, &USART_InitStructure);
	USART_ClockInit(USART2, &USART_ClockInitStructure);

	/* register uart2 */
	rt_hw_serial_register(&uart2_device, "uart2",
						  RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_STREAM,
						  &uart2);

	/* Enable USART2 Rx request */
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
#endif

#ifdef RT_USING_UART3
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_ClockInitStructure.USART_Clock = USART_Clock_Disable;
	USART_ClockInitStructure.USART_CPOL = USART_CPOL_Low;
	USART_ClockInitStructure.USART_CPHA = USART_CPHA_2Edge;
	USART_ClockInitStructure.USART_LastBit = USART_LastBit_Disable;
	USART_Init(USART3, &USART_InitStructure);
	USART_ClockInit(USART3, &USART_ClockInitStructure);

	/* register uart3 */
	rt_hw_serial_register(&uart3_device, "uart3",
						  RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX
#if RT_UART3_USE_DMA
						  | RT_DEVICE_FLAG_DMA_TX
#endif
						  ,
						  &uart3);

#if RT_UART3_USE_DMA
	uart3_dma_tx.dma_channel= UART3_TX_DMA;
	/* Enable USART3 DMA Tx request */
	USART_DMACmd(USART3, USART_DMAReq_Tx , ENABLE);
#endif

	/* enable interrupt */
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
#endif
}