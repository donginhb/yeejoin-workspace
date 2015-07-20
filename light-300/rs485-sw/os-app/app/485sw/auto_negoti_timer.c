/*
 ******************************************************************************
 * auto_negoti_timer.c
 *
 *  Created on: 2015-01-31
 *      Author: David, zhaoshaowei@yeejoin.com
 *
 * COPYRIGHT (C) 2015, YEEJOIN (Beijing) Technology Development Co., Ltd.
 ******************************************************************************
 */

#include <board.h>



/*
 * -------------------------------------------------------------
 * timer
 *
 * tim_period:
 * 	当AUTO_NEGOTI_TIMER_PRESCALER是72时: 单位是us
 * 	当AUTO_NEGOTI_TIMER_PRESCALER是60000时: (72000000 / 60000 = 1200)单位是 s/1200
 * -------------------------------------------------------------
 * */
void init_auto_negoti_timer(uint16_t tim_period)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	TIM_DeInit(AUTO_NEGOTI_TIMER);

	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode	= TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period	= tim_period - 1;	/* 定时时间为 (TIM_Period+1) * Tclk */

	 /* The counter clock frequency CK_CNT is equal to f CK_PSC / (PSC[15:0] + 1)
	  * 1us/clk
	  *  */
	TIM_TimeBaseStructure.TIM_Prescaler	= AUTO_NEGOTI_TIMER_PRESCALER - 1;
	TIM_TimeBaseInit(AUTO_NEGOTI_TIMER, &TIM_TimeBaseStructure);

	TIM_ARRPreloadConfig(AUTO_NEGOTI_TIMER, DISABLE);

	TIM_ClearITPendingBit(AUTO_NEGOTI_TIMER, TIM_IT_Update);
//  	TIM_ITConfig(AUTO_NEGOTI_TIMER, TIM_IT_Update, ENABLE);

	disable_timerx(AUTO_NEGOTI_TIMER);

  	return;
}


void start_auto_negoti_timer(void)
{
	set_timx_int_enable_bit(AUTO_NEGOTI_TIMER, TIM_IT_Update, DISABLE);
	AUTO_NEGOTI_TIMER->EGR = TIM_PSCReloadMode_Immediate;
	TIM_ClearITPendingBit(AUTO_NEGOTI_TIMER, TIM_IT_Update);
	set_timx_int_enable_bit(AUTO_NEGOTI_TIMER, TIM_IT_Update, ENABLE);

	enable_timerx(AUTO_NEGOTI_TIMER);
}


void stop_auto_negoti_timer(void)
{
	disable_timerx(AUTO_NEGOTI_TIMER);
}

void reset_auto_negoti_timer(void)
{
//	disable_timerx(AUTO_NEGOTI_TIMER);

	if ((AUTO_NEGOTI_TIMER)->CR1 & TIM_CR1_CEN) {
		set_timx_int_enable_bit(AUTO_NEGOTI_TIMER, TIM_IT_Update, DISABLE);
		AUTO_NEGOTI_TIMER->EGR = TIM_PSCReloadMode_Immediate;
		TIM_ClearITPendingBit(AUTO_NEGOTI_TIMER, TIM_IT_Update);
		set_timx_int_enable_bit(AUTO_NEGOTI_TIMER, TIM_IT_Update, ENABLE);
	}

//	enable_timerx(AUTO_NEGOTI_TIMER);
}

/*
 * -------------------------------------------------------------
 * timer中断
 * -------------------------------------------------------------
 * */
void cfg_auto_negoti_timer_nvic(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = AUTO_NEGOTI_TIMER_IRQ_VECTER;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = AUTO_NEGOTI_TIMER_PREEMPTION_PRI;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = AUTO_NEGOTI_TIMER_SUB_PRI;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	return;
}

void enable_auto_negoti_timer_int(void)
{
	set_timx_int_enable_bit(AUTO_NEGOTI_TIMER, TIM_IT_Update, ENABLE);

	return;
}

FunctionalState disable_auto_negoti_timer_int(void)
{
	return set_timx_int_enable_bit(AUTO_NEGOTI_TIMER, TIM_IT_Update, DISABLE);
}

void restore_auto_negoti_timer_int(FunctionalState new_state)
{
	set_timx_int_enable_bit(AUTO_NEGOTI_TIMER, TIM_IT_Update, new_state);

	return;
}

void clr_auto_negoti_timer_int(void)
{
	TIM_ClearITPendingBit(AUTO_NEGOTI_TIMER, TIM_IT_Update);

	return;
}



#if 0
#include <finsh.h>

#define auto_negoti_test(x) 	printf_syn x

#define TIMEOUT_MS	(2000)

volatile unsigned tim2_cnt;

void an_test(int cmd, int timeout)
{
	switch (cmd) {
	case 1:
		auto_negoti_test(("init timer %d ms(%d ticks)\n", timeout, get_auto_negoti_tikcs_of_ms(timeout)));
		init_auto_negoti_timer(get_auto_negoti_tikcs_of_ms(timeout));
		cfg_auto_negoti_timer_nvic();
		enable_auto_negoti_timer_int();
		printf_syn("os ticks:%u\n", rt_tick_get());
		break;

	case 2:
		start_auto_negoti_timer();
		auto_negoti_test(("start_auto_negoti_timer, os ticks:%u\n", rt_tick_get()));
		break;

	case 3:
		stop_auto_negoti_timer();
		auto_negoti_test(("stop_auto_negoti_timer, os ticks:%u\n", rt_tick_get()));
		break;

	case 4:
		reset_auto_negoti_timer();
		auto_negoti_test(("reset_auto_negoti_timer, os ticks:%u\n", rt_tick_get()));
		break;

	case 5:
		tim2_cnt = 0;
		printf_syn("clr tim2_cnt, os ticks:%u\n", rt_tick_get());
		break;
	case 6:
		printf_syn("tim2_cnt:%u, os ticks:%u\n", tim2_cnt,  rt_tick_get());
		break;

	default:
		auto_negoti_test(("param err"));
		break;
	}
}
FINSH_FUNCTION_EXPORT(an_test, "auto negoti timer test");
#endif
