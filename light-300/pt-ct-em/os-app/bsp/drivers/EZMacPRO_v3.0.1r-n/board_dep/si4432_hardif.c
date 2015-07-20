/*
 * si4432_hardif.c
 *
 *  Created on: 2013-11-27
 *      Author: David, zhaoshaowei@yeejoin.com
 */


#include <stm32f10x.h>
#include <stm32_cpu_comm.h>

#include <board.h>
#include <si4432_hardif.h>

/*
 * RCC统一在board.c中开启
 *
 * */


/*
 * -------------------------------------------------------------
 * SPI
 * -------------------------------------------------------------
 * */
void init_si4432_spi_pin(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* 与ade7880共用spi3 */
	GPIO_InitStructure.GPIO_Pin 	= SI4432_SPI_SCK | SI4432_SPI_MISO | SI4432_SPI_MOSI;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF_PP;
	GPIO_Init(SI4432_SPI_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin 	= SI4432_CS_PIN;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;
	GPIO_Init(SI4432_CS_PORT, &GPIO_InitStructure);
	set_port_pin(SI4432_CS_PORT, SI4432_CS_PIN);

	return;
}


void cfg_si4432_spi_param(void)
{
	SPI_InitTypeDef SPI_InitStruct;

	SPI_Cmd(SI4432_SPIX, DISABLE);

	SPI_InitStruct.SPI_Direction		= SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_Mode			= SPI_Mode_Master;
	SPI_InitStruct.SPI_DataSize		= SPI_DataSize_8b;
	SPI_InitStruct.SPI_CPOL			= SPI_CPOL_Low;
	SPI_InitStruct.SPI_CPHA			= SPI_CPHA_1Edge;
	SPI_InitStruct.SPI_NSS			= SPI_NSS_Soft;
	SPI_InitStruct.SPI_BaudRatePrescaler	= SPI_BaudRatePrescaler_4;
	SPI_InitStruct.SPI_FirstBit		= SPI_FirstBit_MSB;
	SPI_InitStruct.SPI_CRCPolynomial	= 7;
	SPI_Init(SI4432_SPIX, &SPI_InitStruct);

	SPI_Cmd(SI4432_SPIX, ENABLE);

	return;
}


/*
 * -------------------------------------------------------------
 * timer
 *
 * tim_period: 单位是us(当SI4432_MAC_TIMER_PRESCALER是72时)
 * -------------------------------------------------------------
 * */

void init_si4432_mac_timer(uint16_t tim_period)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	TIM_DeInit(SI4432_MAC_TIMER);

	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode	= TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period	= tim_period - 1;	/* 定时时间为 (TIM_Period+1) * Tclk */

	 /* The counter clock frequency CK_CNT is equal to f CK_PSC / (PSC[15:0] + 1)
	  * 1us/clk
	  *  */
	TIM_TimeBaseStructure.TIM_Prescaler	= SI4432_MAC_TIMER_PRESCALER - 1;
	TIM_TimeBaseInit(SI4432_MAC_TIMER, &TIM_TimeBaseStructure);

	TIM_ARRPreloadConfig(SI4432_MAC_TIMER, ENABLE);

	TIM_ClearITPendingBit(SI4432_MAC_TIMER, TIM_IT_Update);
//  	TIM_ITConfig(SI4432_MAC_TIMER, TIM_IT_Update, ENABLE);

  	disable_timerx(SI4432_MAC_TIMER);

  	return;
}


void start_si4432_mac_timer(void)
{
	enable_timerx(SI4432_MAC_TIMER);
}


void stop_si4432_mac_timer(void)
{
	disable_timerx(SI4432_MAC_TIMER);
}

/*
 * -------------------------------------------------------------
 * timer中断
 * -------------------------------------------------------------
 * */
void cfg_si4432_mac_timer_nvic(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = SI4432_MAC_TIMER_IRQ_VECTER;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = SI4432_MAC_TIMER_PREEMPTION_PRI;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = SI4432_MAC_TIMER_SUB_PRI;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	return;
}

void enable_si4432_mac_timer_int(void)
{
	set_timx_int_enable_bit(SI4432_MAC_TIMER, TIM_IT_Update, ENABLE);

	return;
}

FunctionalState disable_si4432_mac_timer_int(void)
{
	return set_timx_int_enable_bit(SI4432_MAC_TIMER, TIM_IT_Update, DISABLE);
}

void restore_si4432_mac_timer_int(FunctionalState new_state)
{
	set_timx_int_enable_bit(SI4432_MAC_TIMER, TIM_IT_Update, new_state);

	return;
}

void clr_si4432_mac_timer_int(void)
{
	TIM_ClearITPendingBit(SI4432_MAC_TIMER, TIM_IT_Update);

	return;
}

/*
 * -------------------------------------------------------------
 * exit中断
 * -------------------------------------------------------------
 * */
void init_si4432_exti(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

	GPIO_InitStructure.GPIO_Pin 	= SI4432_INT_PIN;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_Init(SI4432_INT_PORT, &GPIO_InitStructure);

	EXTI_ClearITPendingBit(SI4432_INT_EXT_LINE);
	GPIO_EXTILineConfig(SI4432_INT_PORT_SOURCE, SI4432_INT_PIN_SOURCE);
	EXTI_InitStructure.EXTI_Line 	= SI4432_INT_EXT_LINE;
	EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	return;
}

void cfg_si4432_exti_nvic(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = SI4432_INT_NUM;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = SI4432_PREEMPTION_PRI;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = SI4432_SUB_PRI;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	return;
}

void enable_si4432_exti(void)
{
	set_exti_enable_bit(SI4432_INT_EXT_LINE, ENABLE);

	return;
}

FunctionalState disable_si4432_exti(void)
{
	return set_exti_enable_bit(SI4432_INT_EXT_LINE, DISABLE);
}

void restore_si4432_exti(FunctionalState new_state)
{
	set_exti_enable_bit(SI4432_INT_EXT_LINE, new_state);

	return;
}

void clr_si4432_exti(void)
{
	EXTI_ClearITPendingBit(SI4432_INT_EXT_LINE);

	return;
}


/*
 * -------------------------------------------------------------
 * 4432 mac的所有中断
 * -------------------------------------------------------------
 * */
/* 在cpu中允许si4432的所有中断 */
void enable_si4432_mac_int(void)
{
	enable_si4432_exti();
	enable_si4432_mac_timer_int();

	return;
}

/* 在cpu中禁止si4432的所有中断 */
ubase_t disable_si4432_mac_int(void)
{
	ubase_t ret;

	ret = 0;

	if (ENABLE == disable_si4432_exti())
		ret |= 1;

	if (ENABLE == disable_si4432_mac_timer_int())
		ret |= 0x02;

	return ret;
}

/* 在cpu中恢复si4432的所有中断 */
void restore_si4432_mac_int(ubase_t flag)
{
	if (0 != (flag & 1))
		enable_si4432_exti();
	else
		disable_si4432_exti();

	if (0 != (flag & 0x2))
		enable_si4432_mac_timer_int();
	else
		disable_si4432_mac_timer_int();

	return;
}



#if 0
#include <finsh.h>
#include <si4432_v2.h>
#include <spi4ezmacpro.h>

#define si4432_hardif_test(x) 	printf_syn x


void hardif_test(int cmd)
{
	unsigned int temp1, temp2;

	switch (cmd) {
	case 1:
		init_si4432_spi_pin();
		cfg_si4432_spi_param();
		temp1 = macSpiReadReg(SI4432_DEVICE_TYPE);
		temp2 = macSpiReadReg(SI4432_DEVICE_VERSION);

		si4432_hardif_test(("dev-type:0x%x(shoule 0x08), dev-ver:0x%x(should 0x06)\n", temp1, temp2));
		break;

	case 2:
		init_si4432_mac_timer(1000);
		cfg_si4432_mac_timer_nvic();
		enable_si4432_mac_timer_int();

		start_si4432_mac_timer();
		break;

	case 3:
		stop_si4432_mac_timer();
		break;

	case 4:
		temp1 = disable_si4432_mac_timer_int();
		si4432_hardif_test(("ret value:%d(enable:1, disable:0)\n", temp1));
		break;

	case 5:
		enable_si4432_mac_timer_int();
		break;

	default:
		break;
	}
}
FINSH_FUNCTION_EXPORT(hardif_test, "hardif test");
#endif
