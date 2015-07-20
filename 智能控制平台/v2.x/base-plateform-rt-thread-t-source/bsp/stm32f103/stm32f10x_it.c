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
#include <board.h>
#include <rtthread.h>


extern void rt_hw_serial_isr(struct rt_device *device);

/** @addtogroup Template_Project
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

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
  while (1)
  {
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
  while (1)
  {
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
  while (1)
  {
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

extern volatile int is_need_fed_wwdg;
extern void snmp_inc_sysuptime(void);

void SysTick_Handler(void)
{
    extern void rt_hw_timer_handler(void);
#if USE_STM32_IWDG
	/* Reloads IWDG counter with value defined in the reload register */
	/* #define KR_KEY_Reload    ((uint16_t)0xAAAA) */
	IWDG->KR = 0xAAAA;
#endif

#if USE_STM32_WWDG
	if (0 != is_need_fed_wwdg)
		WWDG->CR = WWDG_RELOAD_VALUE; /* Î¹¹· */
#endif
    rt_hw_timer_handler();
    snmp_inc_sysuptime();
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

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
#endif
}

extern struct rt_semaphore keyscan_chip_had_int;
extern void isr_proc_enc28j60_int(void);
extern void isr_proc_ra8875_int(void);

void EXTI9_5_IRQHandler(void)
{
	/* enter interrupt */
	rt_interrupt_enter();
#if 1

#if 1==USE_TO_7INCH_LCD
	if (RESET != EXTI_GetITStatus(EXTI_Line6)) {
		EXTI_ClearITPendingBit(EXTI_Line6);
		/* enc28j60 /int */
		isr_proc_enc28j60_int();
	}
#endif

#else
	if (RESET != EXTI_GetITStatus(EXTI_Line5)) {
		EXTI_ClearITPendingBit(EXTI_Line5);
	}

	if (RESET != EXTI_GetITStatus(EXTI_Line6)) {
		EXTI_ClearITPendingBit(EXTI_Line6);
	}

	if (RESET != EXTI_GetITStatus(EXTI_Line7)) {
		EXTI_ClearITPendingBit(EXTI_Line7);
		//rt_sem_release(&keyscan_chip_had_int);
	}

	if (RESET != EXTI_GetITStatus(EXTI_Line8)) {
		EXTI_ClearITPendingBit(EXTI_Line8);
	}

	if (RESET != EXTI_GetITStatus(EXTI_Line9)) {
		EXTI_ClearITPendingBit(EXTI_Line9);
	}
#endif
	/* leave interrupt */
	rt_interrupt_leave();

	return;
}


void EXTI15_10_IRQHandler(void)
{
	/* enter interrupt */
	rt_interrupt_enter();
#if 1

#if 1==USE_TO_9INCH_LCD
	if (RESET != EXTI_GetITStatus(EXTI_Line10)) {
		EXTI_ClearITPendingBit(EXTI_Line10);
		/* enc28j60 /int */
		isr_proc_enc28j60_int();
	}
#endif

#if 1==USE_TO_7INCH_LCD
	if (RESET != EXTI_GetITStatus(EXTI_Line12)) {
		EXTI_ClearITPendingBit(EXTI_Line12);
		isr_proc_ra8875_int();
		/* ra8875 /int */
		rt_sem_release(&keyscan_chip_had_int);;
	}
#endif

#else
	if (RESET != EXTI_GetITStatus(EXTI_Line10)) {
		EXTI_ClearITPendingBit(EXTI_Line10);
		/* enc28j60 /int */
		isr_proc_enc28j60_int();
	}

	if (RESET != EXTI_GetITStatus(EXTI_Line11)) {
		EXTI_ClearITPendingBit(EXTI_Line11);
	}

	if (RESET != EXTI_GetITStatus(EXTI_Line12)) {
		EXTI_ClearITPendingBit(EXTI_Line12);
		isr_proc_ra8875_int();
		/* ra8875 /int */
		rt_sem_release(&keyscan_chip_had_int);;
	}

	if (RESET != EXTI_GetITStatus(EXTI_Line13)) {
		EXTI_ClearITPendingBit(EXTI_Line13);
	}

	if (RESET != EXTI_GetITStatus(EXTI_Line14)) {
		EXTI_ClearITPendingBit(EXTI_Line14);
	}

	if (RESET != EXTI_GetITStatus(EXTI_Line15)) {
		EXTI_ClearITPendingBit(EXTI_Line15);
	}
#endif
	/* leave interrupt */
	rt_interrupt_leave();

	return;
}

/**
  * @}
  */ 

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
