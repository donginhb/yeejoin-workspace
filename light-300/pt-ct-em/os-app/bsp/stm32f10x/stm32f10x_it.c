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
#include <debug_sw.h>

#if RT_USING_ADE7880
#include <ade7880_api.h>
#include <ade7880_hw.h>
#endif

#if RT_USING_SI4432_MAC
/* use by si4432 */
#define USE_NEW_SI4432_MAC 1

#if !USE_NEW_SI4432_MAC
#include "si4432_def.h"
#include "EZMacPro.h"
#include "hardware_defs.h"
#include "EZMacPro_CallBacks.h"
#include <Timer.h>
#include <SI4432.h>
#include <EXTI.h>
#else
#include <ezmacpro_common.h>
#endif
#endif

#include <rtdef.h>
#include <stm32_cpu_comm.h>

/** @addtogroup Template_Project
  * @{
  */


#if RT_USING_RTC
/* rtc 1s中断一次, 该计数也是系统上电后，连续运行的秒数 */
volatile unsigned long rtc_1s_int_cnt;
#endif
volatile u32 hsdc_transcomp_flag;    

#if RT_USING_ADE7880 || RT_USING_SI4432_MAC
struct rt_event isr_event_set;
volatile unsigned isr_event_var_flag;
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
	Write_16bitReg_ADE7880(CONFIG_Register_Address, CONFIG_Register_Data);	

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
	/* enter interrupt */
	rt_interrupt_enter();

	if (is_exti_it_set(ENC28J60_INT_EXT_LINE)) {
		clear_exti_it_pending_bit(ENC28J60_INT_EXT_LINE);
		/* enc28j60 /int */
		isr_proc_enc28j60_int();
	}

	/* leave interrupt */
	rt_interrupt_leave();
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

#if RT_USING_SI4432_MAC
/* si4432 */
void TIM3_IRQHandler(void)
{
	/* enter interrupt */
	rt_interrupt_enter();

	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
#if !USE_NEW_SI4432_MAC
		if (time3_EZMacProTimerMSB == 0) {
			TIM_ITConfig(TIM3,TIM_IT_Update,DISABLE);
			TIM_Cmd(TIM3, DISABLE);
			TimeOutReqUpdate = 1;
		} else {
			TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
			time3_EZMacProTimerMSB--;
			TIM_Cmd(TIM3, DISABLE);
			TIM_TimeBaseStructure.TIM_Prescaler = 36000 - 1;
			TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
			TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;
			TIM_TimeBaseStructure.TIM_Period = 0x10000 - 1;
			TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);
			TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
			TIM_Cmd(TIM3, ENABLE);
		}
#endif
	}

	/* leave interrupt */
	rt_interrupt_leave();
}

/* si4432 */
void TIM2_IRQHandler(void)
{
	/* enter interrupt */
	rt_interrupt_enter();

	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

#if 1
		rt_event_send(&isr_event_set, EVENT_BIT_NEED_RUN_TIM2_IRQ);
		set_bit(isr_event_var_flag, EVENT_BIT_NEED_RUN_TIM2_IRQ);
#else
//		led_blink(led1_gpio, led1_pin);
//		clr_port_pin(led1_gpio, led1_pin);
		ezmacpro_timer_isr();
#endif
	}

	/* leave interrupt */
	rt_interrupt_leave();
}


void tim2_isr(void)
{
#if !USE_NEW_SI4432_MAC
	uint8_t state;

	if (EZMacProTimerMSB == 0) {
		DISABLE_MAC_TIMER_INTERRUPT();
		STOP_MAC_TIMER();
		state = EZMacProReg.name.MSR & 0x0F;
		if (EZMacProReg.name.MSR == EZMAC_PRO_WAKE_UP) {
			//if the MAC is in Wake up state call the WakeUp function
			timerIntWakeUp();
		} else if ((EZMacProReg.name.MSR & TX_STATE_BIT) == TX_STATE_BIT) {
			//if the MAC is in transmit state then call the transmit state machine
			timerIntTX_StateMachine(state);
		} else if ((EZMacProReg.name.MSR & RX_STATE_BIT) == RX_STATE_BIT) {
			//if the MAC is in receive state then call the receiver
			timerIntRX_StateMachine(state);
		}
	} else {
		TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
		EZMacProTimerMSB--;
		TIM_Cmd(TIM2, DISABLE);
		TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;
		TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
		TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;
		TIM_TimeBaseStructure.TIM_Period = 0x10000 - 1;
		TIM_TimeBaseInit(TIM2,&TIM_TimeBaseStructure);
		TIM_ClearFlag(TIM2, TIM_FLAG_Update);
		TIM_Cmd(TIM2, ENABLE);
	}
#else
//		led_blink(led1_gpio, led1_pin);
		ezmacpro_timer_isr();
#endif
}


void si4432_mac_exti_isr(void)
{
#if !USE_NEW_SI4432_MAC
	uint8_t state;

	ItStatus1 = SI4432_ReadReg(SI4432_INTERRUPT_STATUS_1);
	ItStatus2 = SI4432_ReadReg(SI4432_INTERRUPT_STATUS_2);
	if((ItStatus1 != 0) || ((ItStatus2 & 0xF3) != 0)) {
		state = EZMacProReg.name.MSR & 0x0F;
		if(EZMacProReg.name.MSR == EZMAC_PRO_WAKE_UP) {
			extIntWakeUp(ItStatus2); //if the MAC is in Wake up state call the WakeUp function
		} else if ((EZMacProReg.name.MSR & TX_STATE_BIT) == TX_STATE_BIT) {
			//if the MAC is in transmit state then call the transmit state machine
			extIntTX_StateMachine(state, ItStatus1, ItStatus2);
		} else if ((EZMacProReg.name.MSR & RX_STATE_BIT) == RX_STATE_BIT) {
			//if the MAC is in receive state then call the receiver
			extIntRX_StateMachine(state, ItStatus1, ItStatus2);
		} else {
			extIntDisableInterrupts();
		}
	}
	//Low Frequency Timer
	//if Wake up timer interrupt is occurred the proper callback function will be called
	if((ItStatus2 & 0x08) == 0x08) {
		EZMacPRO_LFTimerExpired();
		ENABLE_MAC_EXT_INTERRUPT();
	}
	//Low Battery Detect
	//if low battery detect interrupt is occurred the proper callback function will be called
	if((ItStatus2 & 0x04) == 0x04) {
		EZMacPRO_LowBattery();
		ENABLE_MAC_EXT_INTERRUPT();
	}
#else
	ezmacpro_exti_isr();
#endif
}

#endif /* #if RT_USING_SI4432_MAC */

#if EM_ALL_TYPE_BASE
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
#endif

#if RT_USING_ADE7880
#if 0==ADE7880_USE_I2C_HSDC

/* ade7880 */
void TIM4_IRQHandler(void)
{
	/* enter interrupt */
	rt_interrupt_enter();

	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET) {
		rt_event_send(&isr_event_set, EVENT_BIT_NEED_RUN_TIM4_IRQ);
		set_bit(isr_event_var_flag, EVENT_BIT_NEED_RUN_TIM4_IRQ);
		TIM4->SR = ~TIM_FLAG_Update; /* Clear the flags */
	}

	/* leave interrupt */
	rt_interrupt_leave();

	return;
}
#endif
/* ade7880 */
void TIM5_IRQHandler(void)
{
	/* enter interrupt */
	rt_interrupt_enter();

//	if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET) {
	if (is_timer_update_it_set(TIM5)) {
		++check_eenergy_timer_clk_period_cnt;
//		led_blink(led3_gpio, led3_pin);
		TIM5->SR = ~TIM_FLAG_Update; /* Clear the flags */
	}

	/* leave interrupt */
	rt_interrupt_leave();

	return;
}


#if 0==ADE7880_USE_I2C_HSDC

void tim4_isr(void)
{
	static int  ade7880_sample_cnt = 0;

	/*
	sample_value=1 VA;sample_value=2 VB;sample_value=3 VC;
	sample_value=4 IA;sample_value=5 IB;sample_value=6 IC;
	*/

	switch (ade7880_sample_cmd) {
	case ASSC_SAMPLE_AVI:
		clr_ade7880_statusx(STATUS0_Register_Address);
		wait_dsp_complete(BIT(17));

		pa_ade7880_sample_data[0][ade7880_sample_cnt] = Read_32bitReg_ADE7880(VAWV_Register_Address);
		pa_ade7880_sample_data[1][ade7880_sample_cnt] = Read_32bitReg_ADE7880(IAWV_Register_Address);
		ade7880_sample_cnt++;
		break;

	case ASSC_SAMPLE_BVI:
		clr_ade7880_statusx(STATUS0_Register_Address);
		wait_dsp_complete(BIT(17));

		pb_ade7880_sample_data[0][ade7880_sample_cnt] = Read_32bitReg_ADE7880(VBWV_Register_Address);
		pb_ade7880_sample_data[1][ade7880_sample_cnt] = Read_32bitReg_ADE7880(IBWV_Register_Address);
		ade7880_sample_cnt++;
		break;

	case ASSC_SAMPLE_CVI:
		clr_ade7880_statusx(STATUS0_Register_Address);
		wait_dsp_complete(BIT(17));

		pc_ade7880_sample_data[0][ade7880_sample_cnt] = Read_32bitReg_ADE7880(VCWV_Register_Address);
		pc_ade7880_sample_data[1][ade7880_sample_cnt] = Read_32bitReg_ADE7880(ICWV_Register_Address);
		ade7880_sample_cnt++;
		break;

	default:
		ade7880_sample_cnt 	= 0;
		ade7880_sample_cmd	= ASSC_SAMPLE_NONE;
		TIM4->CR1 &= CR1_CEN_Reset; /* Disable the TIM Counter */
		break;
	}

	if(SINK_INFO_PX_SAMPLE_DOT_NUM == ade7880_sample_cnt){
		ade7880_sample_cnt 	= 0;
		ade7880_sample_cmd	= ASSC_SAMPLE_NONE;
		TIM4->CR1 &= CR1_CEN_Reset; /* Disable the TIM Counter */
	}

	return;
}

#endif
#endif


void EXTI9_5_IRQHandler(void)
{
#if EM_ALL_TYPE_BASE
	static unsigned exti9_5_cnt;
	EXTI_InitTypeDef EXTI_InitStructure;
	unsigned long num_of_period, num_of_clks;
#endif
	/* enter interrupt */
	rt_interrupt_enter();
#if EM_ALL_TYPE_BASE
	if (is_exti_it_set(EM_ACTIVE_ENERGY_INT_EXT_LINE) || is_exti_it_set(EM_REACTIVE_ENERGY_INT_EXT_LINE)) {
		num_of_clks	= get_timer_cnt(CHECK_E_ENERGY_TIMER);
		num_of_period	= check_eenergy_timer_clk_period_cnt;

		++em_energy_int_cnt;
		if (1 == em_energy_int_cnt) {
			if (ECS_IDLE == check_eenergy_state) {
				enable_timerx(CHECK_E_ENERGY_TIMER);
				check_eenergy_state = ECS_TIMER_RUNNING;
			}

			em_energy_timer_clks_cnt_start = 60000*num_of_period + num_of_clks;
		} else if (2 == em_energy_int_cnt) {
			em_energy_timer_clks_cnt_end   = 60000*num_of_period + num_of_clks;

			if (ade7880_energy_int_cnt >= 2) {
				disable_timerx(CHECK_E_ENERGY_TIMER);
				check_eenergy_state = ECS_CHECK_OVER;
			}
//		} else if (1 != em_energy_int_cnt) {
			EXTI_InitStructure.EXTI_Line 	= EM_ACTIVE_ENERGY_INT_EXT_LINE;
			EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;
			EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
			EXTI_InitStructure.EXTI_LineCmd = DISABLE;
			EXTI_Init(&EXTI_InitStructure);

			EXTI_InitStructure.EXTI_Line 	= EM_REACTIVE_ENERGY_INT_EXT_LINE;
			EXTI_Init(&EXTI_InitStructure);
		}

		clear_exti_it_pending_bit(EM_ACTIVE_ENERGY_INT_EXT_LINE);
		clear_exti_it_pending_bit(EM_REACTIVE_ENERGY_INT_EXT_LINE);
//		rt_kprintf("exti 9-5, cnt:%d, %d\n", em_energy_int_cnt, ade7880_energy_int_cnt);
	}
	if (is_sub_m_on(dsw_sub_module[DMN_SINKINFO], DSMN_SI_BODY)) {
		rt_kprintf("exti 9-5, cnt:%d, %d, %d\n", em_energy_int_cnt,
				ade7880_energy_int_cnt, ++exti9_5_cnt);
	}
#endif

#if RT_USING_SI4432_MAC
	/* si4432 mac */
	if (EXTI_GetITStatus(SI4432_INT_EXT_LINE) != RESET) {
		EXTI_ClearITPendingBit(SI4432_INT_EXT_LINE);
#if 1
		rt_event_send(&isr_event_set, EVENT_BIT_NEED_RUN_SI4432_MAC_EXTI_IRQ);
		set_bit(isr_event_var_flag, EVENT_BIT_NEED_RUN_SI4432_MAC_EXTI_IRQ);
#else
		ezmacpro_exti_isr();
#endif
	}

#endif
	/* leave interrupt */
	rt_interrupt_leave();

	return;
}

void EXTI15_10_IRQHandler(void)
{
#if EM_ALL_TYPE_BASE
	static unsigned exti15_10_cnt;
	EXTI_InitTypeDef EXTI_InitStructure;
	unsigned long num_of_period, num_of_clks;
#endif
	/* enter interrupt */
	rt_interrupt_enter();
#if EM_ALL_TYPE_BASE
	if (is_exti_it_set(ADE7880_ACTIVE_ENERGY_INT_EXT_LINE) || is_exti_it_set(ADE7880_REACTIVE_ENERGY_INT_EXT_LINE)) {
		num_of_clks	= get_timer_cnt(CHECK_E_ENERGY_TIMER);
		num_of_period	= check_eenergy_timer_clk_period_cnt;

		++ade7880_energy_int_cnt;
		if (1 == ade7880_energy_int_cnt) {
			if (ECS_IDLE == check_eenergy_state) {
				enable_timerx(CHECK_E_ENERGY_TIMER);
				check_eenergy_state = ECS_TIMER_RUNNING;
			}

			ade7880_energy_timer_clks_cnt_start = 60000*num_of_period + num_of_clks;
		} else if (2 == ade7880_energy_int_cnt) {
			ade7880_energy_timer_clks_cnt_end   = 60000*num_of_period + num_of_clks;

			if (em_energy_int_cnt >= 2) {
				disable_timerx(CHECK_E_ENERGY_TIMER);
				check_eenergy_state = ECS_CHECK_OVER;
			}
//		} else if (1 != ade7880_energy_int_cnt) {
			EXTI_InitStructure.EXTI_Line 	= ADE7880_ACTIVE_ENERGY_INT_EXT_LINE;
			EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;
			EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
			EXTI_InitStructure.EXTI_LineCmd = DISABLE;
			EXTI_Init(&EXTI_InitStructure);

			EXTI_InitStructure.EXTI_Line 	= ADE7880_REACTIVE_ENERGY_INT_EXT_LINE;
			EXTI_InitStructure.EXTI_LineCmd = DISABLE;
			EXTI_Init(&EXTI_InitStructure);
		}

		clear_exti_it_pending_bit(ADE7880_ACTIVE_ENERGY_INT_EXT_LINE);
		clear_exti_it_pending_bit(ADE7880_REACTIVE_ENERGY_INT_EXT_LINE);
	}

	if (is_sub_m_on(dsw_sub_module[DMN_SINKINFO], DSMN_SI_BODY)) {
		rt_kprintf("exti 15-10, cnt:%d, %d, %d, 0x%x, 0x%x\n", em_energy_int_cnt,
				ade7880_energy_int_cnt, ++exti15_10_cnt, EXTI->IMR, EXTI->PR);
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
