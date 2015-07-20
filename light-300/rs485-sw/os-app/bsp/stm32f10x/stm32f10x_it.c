/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "serial.h"
#include <board.h>
#include <rtthread.h>
#include <rtdef.h>
#include <stm32_cpu_comm.h>

/** @addtogroup Template_Project
  * @{
  */

struct rt_event isr_event_set;


#if RT_USING_RTC
/* rtc 1s中断一次, 该计数也是系统上电后，连续运行的秒数 */
volatile unsigned long rtc_1s_int_cnt;
#endif



/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
#if 0
/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
	/* Go to infinite loop when Memory Manage exception occurs */
	while (1) {
	}
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
	/* Go to infinite loop when Bus Fault exception occurs */
	while (1) {
	}
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
	/* Go to infinite loop when Usage Fault exception occurs */
	while (1) {
	}
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}
#endif

void SysTick_Handler(void)
{
	extern void rt_hw_timer_handler(void);

	rt_hw_timer_handler();
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

#if 0
/*******************************************************************************
* Function Name  : DMA1_Channel2_IRQHandler
* Description    : This function handles DMA1 Channel 2 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel2_IRQHandler(void)
{
#if RT_UART3_USE_DMA
	extern struct rt_device uart3_device;
	extern void rt_hw_serial_dma_tx_isr(struct rt_device *device);

	/* enter interrupt */
	rt_interrupt_enter();

	if (DMA_GetITStatus(DMA1_IT_TC2)) {
		/* transmission complete, invoke serial dma tx isr */
		rt_hw_serial_dma_tx_isr(&uart3_device);
	}

	/* clear DMA flag */
	DMA_ClearFlag(DMA1_FLAG_TC2 | DMA1_FLAG_TE2);

	/* leave interrupt */
	rt_interrupt_leave();
#else
	while (1);
#endif
}
#endif
#if RT_USING_ADE7880
void DMA1_Channel2_IRQHandler(void)
{ 
	/* enter interrupt */
	rt_interrupt_enter();   
   
	DMA_ClearFlag(DMA1_FLAG_TC2 | DMA1_FLAG_TE2);
 

	/* leave interrupt */
	rt_interrupt_leave();
}
#endif

/*******************************************************************************
* Function Name  : USART1_IRQHandler
* Description    : This function handles USART1 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USART1_IRQHandler(void)
{
#ifdef RT_USING_UART1
	extern struct rt_device uart1_device;

	/* enter interrupt */
	rt_interrupt_enter();

	rt_hw_serial_isr(&uart1_device);

	/* leave interrupt */
	rt_interrupt_leave();
#else
	while (1);
#endif
}

/*******************************************************************************
* Function Name  : USART2_IRQHandler
* Description    : This function handles USART2 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USART2_IRQHandler(void)
{
#ifdef RT_USING_UART2
	extern struct rt_device uart2_device;

	/* enter interrupt */
	rt_interrupt_enter();

	rt_hw_serial_isr(&uart2_device);

	/* leave interrupt */
	rt_interrupt_leave();
#else
	while (1);
#endif
}

/*******************************************************************************
* Function Name  : USART3_IRQHandler
* Description    : This function handles USART3 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USART3_IRQHandler(void)
{
#ifdef RT_USING_UART3
	extern struct rt_device uart3_device;

	/* enter interrupt */
	rt_interrupt_enter();

	rt_hw_serial_isr(&uart3_device);

	/* leave interrupt */
	rt_interrupt_leave();
#else
	while (1);
#endif
}


void UART4_IRQHandler(void)
{
#ifdef RT_USING_UART4
	extern struct rt_device uart4_device;

	/* enter interrupt */
	rt_interrupt_enter();

	rt_hw_serial_isr(&uart4_device);

	/* leave interrupt */
	rt_interrupt_leave();
#else
	while (1);
#endif
}

void UART5_IRQHandler(void)
{
#ifdef RT_USING_UART5
	extern struct rt_device uart5_device;

	/* enter interrupt */
	rt_interrupt_enter();

	rt_hw_serial_isr(&uart5_device);

	/* leave interrupt */
	rt_interrupt_leave();
#else
	while (1);
#endif
}

#if RT_USING_RTC
void RTC_IRQHandler(void)
{
	/* enter interrupt */
	rt_interrupt_enter();

	++rtc_1s_int_cnt;

	RTC_ClearITPendingBit(RTC_IT_ALR | RTC_IT_SEC);

	/* leave interrupt */
	rt_interrupt_leave();

	return;
}
#endif


#if defined(RT_USING_DFS) && STM32_USE_SDIO
/*******************************************************************************
* Function Name  : SDIO_IRQHandler
* Description    : This function handles SDIO global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SDIO_IRQHandler(void)
{
	extern int SD_ProcessIRQSrc(void);

	/* enter interrupt */
	rt_interrupt_enter();

	/* Process All SDIO Interrupt Sources */
	SD_ProcessIRQSrc();

	/* leave interrupt */
	rt_interrupt_leave();
}
#endif

#ifdef RT_USING_LWIP

#ifdef STM32F10X_CL
/*******************************************************************************
* Function Name  : ETH_IRQHandler
* Description    : This function handles ETH interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ETH_IRQHandler(void)
{
	extern void rt_hw_stm32_eth_isr(void);

	/* enter interrupt */
	rt_interrupt_enter();

	rt_hw_stm32_eth_isr();

	/* leave interrupt */
	rt_interrupt_leave();
}
#else
#if (STM32_ETH_IF == 0)

extern void isr_proc_enc28j60_int(void);

void EXTI1_IRQHandler(void)
{
	if (is_exti_it_set(ENC28J60_INT_EXT_LINE)) {
		clear_exti_it_pending_bit(ENC28J60_INT_EXT_LINE);
		/* enc28j60 /int */
		isr_proc_enc28j60_int();
	}

}


#endif

#if (STM32_ETH_IF == 1)
/*******************************************************************************
* Function Name  : EXTI4_IRQHandler
* Description    : This function handles External lines 9 to 5 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI4_IRQHandler(void)
{
	extern void rt_dm9000_isr(void);

	/* enter interrupt */
	rt_interrupt_enter();

	/* Clear the DM9000A EXTI line pending bit */
	EXTI_ClearITPendingBit(EXTI_Line4);

	rt_dm9000_isr();

	/* leave interrupt */
	rt_interrupt_leave();
}
#endif
#endif
#endif /* end of RT_USING_LWIP */


//volatile unsigned tim2_cnt;
extern void emc_state_timeout(void *p);

void TIM2_IRQHandler(void)
{
	/* enter interrupt */
	rt_interrupt_enter();

	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
//		rt_kprintf("%s() os ticks:%u\n", __func__, rt_tick_get());
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		disable_timerx(AUTO_NEGOTI_TIMER);
		emc_state_timeout(NULL);

//		running_led_blink();
//		tim2_cnt++;


	}

	/* leave interrupt */
	rt_interrupt_leave();
}



#include <tl16c554_hal.h>
#include <tl16_uart.h>
#include <tl16_serial.h>

/*
 *   INTERRUPT
 * IDENTIFICATION			INTERRUPT SET AND RESET FUNCTIONS
 *   REGISTER
 * BIT3-0 	PRIORITY LEVEL 	INTERRUPT TYPE 			INTERRUPT SOURCE 		INTERRUPT RESET CONTROL
 * 0 0 0 1 	— 		None 				None 				—
 * 0 1 1 0 	First		Receiver line status 		OE, PE, FE, or BI 		LSR read
 * 0 1 0 0 	Second 		Received data available 	Receiver data available  	RBR read until FIFOdrops
 * 								or trigger level reached	below the trigger level
 * 1 1 0 0 	Second 		Character time-out indicator 	No characters have been		RBR read
 * 								removed from or input to the
 * 								receiver FIFO during the last
 * 								four character times, and there
 * 								is at least one character in it
 * 								during this time.
 * 0 0 1 0 	Third 		THRE 				THRE 				IIR read if THRE is the
 * 												interrupt source or THR write
 * 0 0 0 0 	Fourth		 Modem status 			/CTS, /DSR, /RI, or /DCD
 *
 * -------------------
 *
 * Bit0: IIR0 indicates whether an interrupt is pending. When IIR0 is cleared, an interrupt is pending.
 * Bits1-2: IIR1 and IIR2 identify the highest priority interrupt pending as indicated in Table 5.
 * Bit3: IIR3 is always cleared when in the TL16C450 mode. This bit is set along with bit 2 when in the FIFO mode and a trigger change level interrupt is pending.
 * Bits4-5: IIR4 and IIR5 are always cleared.
 * Bits6-7: IIR6 and IIR7 are set when FCR0 = 1.
 * */
int do_tl16_irq(enum tl16_x_csx_addr_e x_csx, int *had_proc)
{
	unsigned int iir_val, lsr_val;
	rt_device_t dev;
	struct tl16_serial_device *uart;

	dev = get_tl16dev_from_x_csx(x_csx);
	if (NULL == dev) {
		rt_kprintf("\n%s(), param invalid\n", __func__);
		return FAIL;
	}

	uart = get_tl16uart_from_x_csx(x_csx);

	iir_val = tl16_read_iir(x_csx) & 0xf;

//	rt_kprintf("\n%s(), x_csx:%d, iir_val:0x%x\n", __func__, x_csx, iir_val);

	switch (iir_val) {
	case 0x1:
	case 0:
		/* nothing */
		break;
	case 0x6: /* 0 1 1 0 First		Receiver line status 		OE, PE, FE, or BI */
		/*
		 * LSR BITS						1		0
		 * LSR0 data ready (DR)					Ready 		Not ready
		 * LSR1 overrun error (OE) 				Error 		No error
		 * LSR2 parity error (PE) 				Error 		No error
		 * LSR3 framing error (FE) 				Error 		No error
		 * LSR4 break interrupt (BI) 				Break 		No break
		 * LSR5 transmitter holding register empty (THRE) 	Empty 		Not empty
		 * LSR6 transmitter register empty (TEMT)		Empty 		Not empty
		 * LSR7 receiver FIFO error				Error in FIFO 	No error in FIFO
		 * */
		lsr_val = tl16_read_lsr(x_csx);

		if (lsr_val & 0x1) {
			rt_hw_tl16_serial_isr(dev);
			*had_proc += 1;
		}

		if (lsr_val & 0x1<<1) {
			rt_hw_tl16_serial_isr(dev);
			*had_proc += 1;
			uart->uart_device->overrun_err_cnt++;
		}

		if (lsr_val & 0x1<<2) {
			uart->uart_device->parity_err_cnt++;
		}

		if (lsr_val & 0x1<<3) {
			uart->uart_device->framing_err_cnt++;
		}

		if (lsr_val & 0x1<<4) {
			uart->uart_device->break_int_bit_cnt++;
		}
#if 0
		if (lsr_val & 0x1<<5) {
		}

		if (lsr_val & 0x1<<6) {
		}
#endif
		if (lsr_val & 0x1<<7) {
			uart->uart_device->fifo_err_cnt++;
		}

		break;

	case 0x4: /* 0 1 0 0 Second 		Received data available */
	case 0xc: /* 1 1 0 0 Second 		Character time-out indicator */

		rt_hw_tl16_serial_isr(dev);
//		rt_kprintf("\n%s(), x_csx-%d, recv data\n", __func__, x_csx);
		*had_proc += 1;
		break;

	default:
		rt_kprintf("\n%s(), x_csx-%d not expect iir-val:0x%x\n", __func__, x_csx, iir_val);
		return FAIL;
	}

	return SUCC;
}

/*
 * tl16c554_1_intx
 * GPIO_Pin_4
 * */
void EXTI4_IRQHandler(void)
{
	/* enter interrupt */
	rt_interrupt_enter();

//	rt_kprintf("%s()\n", __func__);

	if (is_exti_it_set(TL16C554_1_INT_EXT_LINE)) {
#if 0
		enum tl16_x_csx_addr_e x_csx;

		for (x_csx=TL16_1_CSA_ADDR; x_csx<=TL16_1_CSD_ADDR; ++x_csx) {
			do_tl16_irq(x_csx);
		}
#else
		rt_event_send(&isr_event_set, EVENT_BIT_NEED_RUN_TL16_1_IRQ);
#endif
		clear_exti_it_pending_bit(TL16C554_1_INT_EXT_LINE);
	}

	/* leave interrupt */
	rt_interrupt_leave();

	return;
}


/*
 * tl16c554_2_intx
 * GPIO_Pin_6
 * */
void EXTI9_5_IRQHandler(void)
{
	/* enter interrupt */
	rt_interrupt_enter();

//	rt_kprintf("%s()\n", __func__);

	if (is_exti_it_set(TL16C554_2_INT_EXT_LINE)) {
#if 0
		for (x_csx=TL16_2_CSA_ADDR; x_csx<=TL16_2_CSD_ADDR; ++x_csx) {
			do_tl16_irq(x_csx);
		}
#else
		rt_event_send(&isr_event_set, EVENT_BIT_NEED_RUN_TL16_2_IRQ);
#endif
		clear_exti_it_pending_bit(TL16C554_2_INT_EXT_LINE);
	}

	/* leave interrupt */
	rt_interrupt_leave();

	return;
}

void EXTI15_10_IRQHandler(void)
{
	/* enter interrupt */
	rt_interrupt_enter();

	rt_kprintf("%s()\n", __func__);

	/* leave interrupt */
	rt_interrupt_leave();

	return;
}

/**
  * @}
  */



/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
