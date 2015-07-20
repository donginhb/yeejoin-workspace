/*
 * tctrl.c
 *
 * 2013-05-13,  creat by David, zhaoshaowei@yeejoin.com
 */
#include "..\Source\FWLib\stm32f10x.h"
#include "..\Source\EX_Support.h"
#include "tctrl.h"
#include "sys_comm.h"

extern volatile unsigned int tim3_8ms_cnt;
extern volatile int  cpu_temper;

#if 0
#define USE4COLLECT_SIGNAL_END
#else
#define USE4RESTORE_SIGNAL_END
#endif

#define USE_GRADUALLY_APPROACHING 1



#ifdef USE4COLLECT_SIGNAL_END
/* 微调时间间隔 */
#define MICRO_ADJ_TIME8MS_GAP_S		(6*125)
#define MICRO_ADJ_TIME8MS_GAP_L		(30*125)

#define MICRO_ADJ_TIME8MS_GAP_INIT	(12*125)

/* 微调步长 */
#define MICRO_ADJ_PWM_DUTY_RATIO	(10)

/* 该值需要大于等于3 */
#define LARGE_SMALL_TIME_PAIR_MAX	(3)

/* 长时间处于某一状态 */
#define LONG_TIME_STATE_TIME8MS 	(40*125)
/* 穿过期望温度后延迟微调时间 */
#define DELAY_MICRO_ADJ_AFTER_THROUGH	(3*125)


/* 15.016s更新一次delta值 */
#define DELTA_ADC_PER_TIME_UNIT	(15*125 + 2)
#define DELTA_ADC_MAX 		(10)

#define HEAT_POWER_WHEN_THROUGH_MAX (3)
#endif

#ifdef USE4RESTORE_SIGNAL_END
/* 微调时间间隔 */
#define MICRO_ADJ_TIME8MS_GAP_S		(6*125)
#define MICRO_ADJ_TIME8MS_GAP_L		(30*125)

#define MICRO_ADJ_TIME8MS_GAP_INIT	(12*125)

/* 微调步长 */
#define MICRO_ADJ_PWM_DUTY_RATIO	(25)

/* 该值需要大于等于3 */
#define LARGE_SMALL_TIME_PAIR_MAX	(3)

/* 长时间处于某一状态 */
#define LONG_TIME_STATE_TIME8MS 	(40*125)
/* 穿过期望温度后延迟微调时间 */
#define DELAY_MICRO_ADJ_AFTER_THROUGH	(3*125)


/* 15.016s更新一次delta值 */
#define DELTA_ADC_PER_TIME_UNIT	(15*125 + 2)
#define DELTA_ADC_MAX 		(10)

#define HEAT_POWER_WHEN_THROUGH_MAX (3)
#endif

#ifdef USE4COLLECT_SIGNAL_END
/* 75摄氏度 */
#define GOAL_OF_TEMPERATURE_ADC_VALUE	526
#define GOAL_OF_TEMPERATURE_VALUE 	75

#define TEMPERATURE_ADC_VALUE_OF_70	606
#define TEMPERATURE_ADC_VALUE_OF_80	457

/*
 * VCC	R1	R2	T	V0		adc	
 * 3.3	10	1.740 	70	0.489097 	606.93 
 * 3.3	10	1.476 	75	0.424434 	526.68 
 * 3.3	10	1.257 	80	0.368491 	457.26 
 *
 *			71	
 *			72	
 *			73	
 * 			74	
 *			75	
 * 			76	
 *			77	
 *			78	
 *			79	
 *
 * 0.489097 - 0.424434 = 0.064663, /5 = 0.0129326
 * 606.93 - 526.68 = 80.25, /5 = 16.05 
 * 1.740 - 1.476 = 0.264, /5 = 0.0528  
 *
 * 0.424434 - 0.368491 = 0.055943, /5 = 0.0111886
 * 526.68 - 457.26 = 69.42, /5 = 13.884
 * 1.476 - 1.257 = 0.219, /5 = 0.0438 
 */
#define PRE_INCREASE_HEAT_POWER_TADC (GOAL_OF_TEMPERATURE_ADC_VALUE - 3*69/5)
#define PRE_DECREASE_HEAT_POWER_TADC (GOAL_OF_TEMPERATURE_ADC_VALUE + 3*80/5)


extern volatile unsigned int box_temper_send;
/* 1s更新一次box_temper_send */
#define get_boxtmper_adc_value() ((int)box_temper_send)
#endif

#ifdef USE4RESTORE_SIGNAL_END
/* 75摄氏度 */
#define GOAL_OF_TEMPERATURE_ADC_VALUE	798
#define GOAL_OF_TEMPERATURE_VALUE 	75

#define TEMPERATURE_ADC_VALUE_OF_70	919
#define TEMPERATURE_ADC_VALUE_OF_80	692

/*
 * VCC	R1	R2	T	V0	adc	
 * 5	10	1.740 	70	0.741056 	919.58
 * 5	10	1.476 	75	0.643081 	798.01 	121.58 
 * 5	10	1.257 	80	0.558319 	692.82 	105.18 
 *
 *			71	0.721461
 *			72	0.701866
 *			73	0.682271
 * 			74	0.662676
 *			75	0.643081
 * 			76	0.626129
 *			77	0.609177
 *			78	0.592225
 *			79	0.575273
 *
 * 0.741056 - 0.643081 = 0.097975, /5 = 0.019595
 * 0.643081 - 0.558319 = 0.084762, /5 = 0.016952
 */
#define PRE_INCREASE_HEAT_POWER_TADC (GOAL_OF_TEMPERATURE_ADC_VALUE - 3*105/5)
#define PRE_DECREASE_HEAT_POWER_TADC (GOAL_OF_TEMPERATURE_ADC_VALUE + 3*121/5)


extern unsigned int box_temper_adc4tctrl;
/* 1s更新一次box_temper_prev_send */
#define get_boxtmper_adc_value() ((int)box_temper_adc4tctrl)
#endif

#define get_cputmper_val()	cpu_temper
#define get_t8ms_cnt()		tim3_8ms_cnt


#define FULL_POWER_HEATING		(1000)
#define STOP_HEATING			(0)

#define PWM_COUNT_CYC  (1000-1)
#define CR1_CEN_Set                 ((uint16_t)0x0001)
#define CR1_CEN_Reset               ((uint16_t)0x03FE)

#ifndef NULL
#define NULL (0x00)
#endif


#if 0
enum tmper_rise_fall_state {
	TRFS_IDLE	= 0,
	TRFS_RISE,
	TRFS_FALL,
};

struct tmper_rise_fall_time_st {
	unsigned int rise_tcnt; /* 温度上升到期望温度所花费的时间, 8ms的倍数 */
	unsigned int fall_tcnt;
};

#else
enum tmper_large_small_state {
	TLSS_IDLE	= 0,
	TLSS_LARGE_START,
	TLSS_SMALL_START,
	TLSS_LARGE,
	TLSS_SMALL,
};

struct tmper_large_small_time_st {
	unsigned int large_tcnt; /* 温度高于期望温度所持续的时间, 8ms的倍数 */
	unsigned int l_duty_ratio; /* 温度高于期望温度时, 对应的pwm占空比 */
	unsigned int small_tcnt;
	unsigned int s_duty_ratio; /*  */
};

struct large_small_time_ctrl_st {
	unsigned int large_time_cnt;
	unsigned int small_time_cnt;
	struct tmper_large_small_time_st ls_pair[LARGE_SMALL_TIME_PAIR_MAX];
};

struct delta_dac_per_unit_st {
	unsigned int delta_adc_cnt;
	int delta_adc_per_unit[DELTA_ADC_MAX];
};

struct heat_power_val_st {
	unsigned cnt;
	int hp_val[HEAT_POWER_WHEN_THROUGH_MAX];
};

static struct large_small_time_ctrl_st ls_time_ctrl;
static struct delta_dac_per_unit_st delta_dac_per_unit;

#endif

#if 0==USE_GRADUALLY_APPROACHING
static struct heat_power_val_st hp_val_rise;
static struct heat_power_val_st hp_val_fall;

static int is_through_heating_power; /* 是否刚穿过期望温度 */
static unsigned long large_small_time_cnt4long;
#endif

static int duty_ratio_from_avg_ls; /* 根据全功率/关闭计算的占空比, 该值只是接近理想值 */
static unsigned long large_small_time_cnt;

static int micro_adj_t8ms_gap;
static int micro_adj_pwm_dutyratio;

int micro_adj_t8ms_gap_s;
int micro_adj_t8ms_gap_l;

#if 0
#define is_not_need_calc() (ls_time_ctrl.large_time_cnt<LARGE_SMALL_TIME_PAIR_MAX || ls_time_ctrl.small_time_cnt<LARGE_SMALL_TIME_PAIR_MAX)
#define is_get_init_val() (ls_time_ctrl.large_time_cnt>=LARGE_SMALL_TIME_PAIR_MAX && ls_time_ctrl.small_time_cnt>=LARGE_SMALL_TIME_PAIR_MAX)
#else
#define is_not_need_calc() (1)
#define is_get_init_val() (0)
#endif

#define limit_heating_power(hp) do {\
	if (hp < STOP_HEATING)\
		hp = STOP_HEATING;\
	else if (hp > FULL_POWER_HEATING)\
		hp = FULL_POWER_HEATING;\
}while (0)

#define debug_info(x)		send_data_to_pc x
#define debug_log(x)		send_data_to_pc x

#define debug_error_line()	send_data_to_pc(0x6b01 << 16 | __LINE__, 4)
#define debug_info_line()	//send_data_to_pc(0x5a00 << 16 | __LINE__, 4)

static int calc_ls_time_sum(unsigned int *plsum, unsigned int *pssum);
static int calc_heating_power_init(enum tmper_large_small_state ls_state, int cur_duty_ratio);

#if 0==USE_GRADUALLY_APPROACHING
static int calc_heating_power_when_through(enum tmper_large_small_state ls_state, int cur_duty_ratio);
static int calc_heating_power_when_lstate(enum tmper_large_small_state ls_state, int cur_duty_ratio);
static int micro_adj_heating_power(enum tmper_large_small_state ls_state, int cur_duty_ratio);
static int set_heating_power_after_through(enum tmper_large_small_state ls_state, int cur_duty_ratio);

static int is_delta_continue_negative(void);
static int is_delta_continue_positive(void);
static int get_avg_hp_val_rise_fall(const struct heat_power_val_st *hpv);
#endif

static void update_tmper_large_small_time(int is_large_time, unsigned int time, unsigned duty_ratio);
static int set_heating_power_param(int duty_ratio);
static void get_delta_adc_per_unit(void);
static void update_delta_adc_per_unit(int delta_adc);
static void update_int_cycle_buf(unsigned int *cnt, int *ic_buf, int buf_len, int data);

void box_tmper_ctrl_init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;

	micro_adj_t8ms_gap	= MICRO_ADJ_TIME8MS_GAP_S;
	micro_adj_pwm_dutyratio = MICRO_ADJ_PWM_DUTY_RATIO;

	/*定时器2-通道2-PB3控制输出pwm信号*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	TIM_DeInit(TIM2);

	TIM_TimeBaseStructure.TIM_Period	= PWM_COUNT_CYC;
	TIM_TimeBaseStructure.TIM_Prescaler	= 6;
	TIM_TimeBaseStructure.TIM_ClockDivision	= 0;
	TIM_TimeBaseStructure.TIM_CounterMode	= TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	TIM_OCInitStructure.TIM_OCMode		= TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState	= TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OCPolarity	= TIM_OCPolarity_High;
	
	/*CH2 output pwm*/
	TIM_OCInitStructure.TIM_Pulse	= 0;
	TIM_OC2Init(TIM2, &TIM_OCInitStructure);
	TIM_OC2PreloadConfig(TIM2,TIM_OCPreload_Enable);

	TIM_ARRPreloadConfig(TIM2,ENABLE);

	TIM_Cmd(TIM2,ENABLE);
}

/*
 */
static int set_heating_power_param(int duty_ratio)
{
	limit_heating_power(duty_ratio);

	TIM2->CR1  &= CR1_CEN_Reset;	/* Disable the TIM Counter */
	TIM2->CCR2 = duty_ratio;	/* Set the Capture Compare Register value */		
	TIM2->CR1  |= CR1_CEN_Set;	/* Enable the TIM Counter */

	return 0;
}

#if 0!=USE_GRADUALLY_APPROACHING
void box_tmper_ctrl(void)
{
	static int large_small_state;
	static int cur_duty_ratio;

	if (get_t8ms_cnt() < 5*125)
		return;

	get_delta_adc_per_unit();
	
	switch (large_small_state) {
	case TLSS_IDLE:
		if (get_boxtmper_adc_value() <= GOAL_OF_TEMPERATURE_ADC_VALUE) {
			large_small_state	= TLSS_LARGE_START;
			cur_duty_ratio		= STOP_HEATING;
			set_heating_power_param(cur_duty_ratio);
			debug_info_line();
		} else {
			large_small_state 	= TLSS_SMALL_START;
			cur_duty_ratio 		= FULL_POWER_HEATING;
			set_heating_power_param(cur_duty_ratio);
			debug_info_line();
		}
		break;

	case TLSS_LARGE_START:
		if (get_boxtmper_adc_value() > GOAL_OF_TEMPERATURE_ADC_VALUE) {
			large_small_time_cnt 	  = get_t8ms_cnt();
			large_small_state 	  = TLSS_SMALL;
			cur_duty_ratio 		  = FULL_POWER_HEATING;
			set_heating_power_param(cur_duty_ratio);
			debug_info_line();
		}
		break;

	case TLSS_SMALL_START:
		if (get_boxtmper_adc_value() <= GOAL_OF_TEMPERATURE_ADC_VALUE) {
			large_small_time_cnt 	  = get_t8ms_cnt();
			large_small_state 	  = TLSS_LARGE;
			cur_duty_ratio 		  = STOP_HEATING;
			set_heating_power_param(cur_duty_ratio);
			debug_info_line();
		}
		break;

	case TLSS_LARGE:
		if (get_boxtmper_adc_value() > GOAL_OF_TEMPERATURE_ADC_VALUE) {
			if (!is_get_init_val()) {
				update_tmper_large_small_time(1, get_t8ms_cnt()-large_small_time_cnt, cur_duty_ratio);
				cur_duty_ratio = calc_heating_power_init(TLSS_SMALL, cur_duty_ratio);
				if (STOP_HEATING!=cur_duty_ratio && FULL_POWER_HEATING!=cur_duty_ratio) {
					duty_ratio_from_avg_ls = cur_duty_ratio;
					micro_adj_t8ms_gap = MICRO_ADJ_TIME8MS_GAP_INIT;
				}
			} else {
				cur_duty_ratio += micro_adj_pwm_dutyratio;
			}

			set_heating_power_param(cur_duty_ratio);
			large_small_time_cnt 	  = get_t8ms_cnt();
			large_small_state 	  = TLSS_SMALL;
			debug_info_line();
		} else if (0!=duty_ratio_from_avg_ls && (get_t8ms_cnt() - large_small_time_cnt >= micro_adj_t8ms_gap)) {
			large_small_time_cnt 	  = get_t8ms_cnt();

			if (get_boxtmper_adc_value() > PRE_INCREASE_HEAT_POWER_TADC) {
				cur_duty_ratio -= 1;
				micro_adj_t8ms_gap = MICRO_ADJ_TIME8MS_GAP_L;
			} else {
				cur_duty_ratio -= micro_adj_pwm_dutyratio;
				micro_adj_t8ms_gap = MICRO_ADJ_TIME8MS_GAP_S;
			}

			limit_heating_power(cur_duty_ratio);
			set_heating_power_param(cur_duty_ratio);
		}
		break;

	case TLSS_SMALL:
		if (get_boxtmper_adc_value() <= GOAL_OF_TEMPERATURE_ADC_VALUE) {
			if (!is_get_init_val()) {
				update_tmper_large_small_time(0, get_t8ms_cnt()-large_small_time_cnt, cur_duty_ratio);
				cur_duty_ratio = calc_heating_power_init(TLSS_LARGE, cur_duty_ratio);
				if (STOP_HEATING!=cur_duty_ratio && FULL_POWER_HEATING!=cur_duty_ratio) {
					duty_ratio_from_avg_ls = cur_duty_ratio;
					micro_adj_t8ms_gap = MICRO_ADJ_TIME8MS_GAP_INIT;
				}
			} else {
				cur_duty_ratio -= micro_adj_pwm_dutyratio;
			}

			set_heating_power_param(cur_duty_ratio);
			large_small_time_cnt	  = get_t8ms_cnt();
			large_small_state 	  = TLSS_LARGE;
			debug_info_line();
		} else if (0!=duty_ratio_from_avg_ls && (get_t8ms_cnt() - large_small_time_cnt >= micro_adj_t8ms_gap)) {
			large_small_time_cnt 	  = get_t8ms_cnt();

			if (get_boxtmper_adc_value() < PRE_DECREASE_HEAT_POWER_TADC) {
				cur_duty_ratio += 1;
				micro_adj_t8ms_gap = MICRO_ADJ_TIME8MS_GAP_L;
			} else {
				cur_duty_ratio += micro_adj_pwm_dutyratio;
				micro_adj_t8ms_gap = MICRO_ADJ_TIME8MS_GAP_S;
			}

			limit_heating_power(cur_duty_ratio);
			set_heating_power_param(cur_duty_ratio);
		}
		break;

	default:
		debug_error_line();
		break;
	}

	return;
}
#else
void box_tmper_ctrl(void)
{
	static int large_small_state;
	static int cur_duty_ratio;

	if (get_t8ms_cnt() < 5*125)
		return;

	get_delta_adc_per_unit();
	
	switch (large_small_state) {
	case TLSS_IDLE:
		if (get_boxtmper_adc_value() <= GOAL_OF_TEMPERATURE_ADC_VALUE) {
			large_small_state	= TLSS_LARGE_START;
			cur_duty_ratio		= STOP_HEATING;
			set_heating_power_param(cur_duty_ratio);
			debug_info_line();
		} else {
			large_small_state 	= TLSS_SMALL_START;
			cur_duty_ratio 		= FULL_POWER_HEATING;
			set_heating_power_param(cur_duty_ratio);
			debug_info_line();
		}
		break;

	case TLSS_LARGE_START:
		if (get_boxtmper_adc_value() > GOAL_OF_TEMPERATURE_ADC_VALUE) {
			large_small_time_cnt 	  = get_t8ms_cnt();
			large_small_time_cnt4long = large_small_time_cnt;
			large_small_state 	  = TLSS_SMALL;
			cur_duty_ratio 		  = FULL_POWER_HEATING;
			set_heating_power_param(cur_duty_ratio);
			debug_info_line();
		}
		break;

	case TLSS_SMALL_START:
		if (get_boxtmper_adc_value() <= GOAL_OF_TEMPERATURE_ADC_VALUE) {
			large_small_time_cnt 	  = get_t8ms_cnt();
			large_small_time_cnt4long = large_small_time_cnt;
			large_small_state 	  = TLSS_LARGE;
			cur_duty_ratio 		  = STOP_HEATING;
			set_heating_power_param(cur_duty_ratio);
			debug_info_line();
		}
		break;

	case TLSS_LARGE:
		if (get_boxtmper_adc_value() > GOAL_OF_TEMPERATURE_ADC_VALUE) {
			if (is_get_init_val()) {
				cur_duty_ratio = calc_heating_power_when_through(TLSS_SMALL, cur_duty_ratio);
			} else {
				update_tmper_large_small_time(1, get_t8ms_cnt()-large_small_time_cnt, cur_duty_ratio);
				cur_duty_ratio = calc_heating_power_init(TLSS_SMALL, cur_duty_ratio);
				if (STOP_HEATING!=cur_duty_ratio && FULL_POWER_HEATING!=cur_duty_ratio)
					duty_ratio_from_avg_ls = cur_duty_ratio;
			}
			set_heating_power_param(cur_duty_ratio);

			large_small_time_cnt 	  = get_t8ms_cnt();
			large_small_time_cnt4long = large_small_time_cnt;
			large_small_state 	  = TLSS_SMALL;
			debug_info_line();
		} else if (get_t8ms_cnt() - large_small_time_cnt4long >= LONG_TIME_STATE_TIME8MS) {
			cur_duty_ratio = calc_heating_power_when_lstate(TLSS_LARGE, cur_duty_ratio);
			set_heating_power_param(cur_duty_ratio);

			large_small_time_cnt 	  = get_t8ms_cnt();
			large_small_time_cnt4long = large_small_time_cnt;
			debug_info_line();
		} else if (is_through_heating_power) {
			cur_duty_ratio = set_heating_power_after_through(TLSS_LARGE, cur_duty_ratio);
		} else if (is_get_init_val() && (get_t8ms_cnt() - large_small_time_cnt >= micro_adj_t8ms_gap)) {
			large_small_time_cnt 	  = get_t8ms_cnt();
			large_small_time_cnt4long = large_small_time_cnt;
			cur_duty_ratio = micro_adj_heating_power(TLSS_LARGE, cur_duty_ratio);
		}
		break;

	case TLSS_SMALL:
		if (get_boxtmper_adc_value() <= GOAL_OF_TEMPERATURE_ADC_VALUE) {
			if (is_get_init_val()) {
				cur_duty_ratio = calc_heating_power_when_through(TLSS_LARGE, cur_duty_ratio);
			} else {
				update_tmper_large_small_time(0, get_t8ms_cnt()-large_small_time_cnt, cur_duty_ratio);
				cur_duty_ratio = calc_heating_power_init(TLSS_LARGE, cur_duty_ratio);
				if (STOP_HEATING!=cur_duty_ratio && FULL_POWER_HEATING!=cur_duty_ratio)
					duty_ratio_from_avg_ls = cur_duty_ratio;
			}
			set_heating_power_param(cur_duty_ratio);

			large_small_time_cnt	  = get_t8ms_cnt();
			large_small_time_cnt4long = large_small_time_cnt;
			large_small_state 	  = TLSS_LARGE;
			debug_info_line();
		} else if (get_t8ms_cnt() - large_small_time_cnt4long >= LONG_TIME_STATE_TIME8MS) {
			cur_duty_ratio = calc_heating_power_when_lstate(TLSS_SMALL, cur_duty_ratio);
			set_heating_power_param(cur_duty_ratio);

			large_small_time_cnt 	  = get_t8ms_cnt();
			large_small_time_cnt4long = large_small_time_cnt;
			debug_info_line();
		} else if (is_through_heating_power) {
			cur_duty_ratio = set_heating_power_after_through(TLSS_SMALL, cur_duty_ratio);
		} else if (is_get_init_val() && (get_t8ms_cnt() - large_small_time_cnt >= micro_adj_t8ms_gap)) {
			large_small_time_cnt 	  = get_t8ms_cnt();
			large_small_time_cnt4long = large_small_time_cnt;
			cur_duty_ratio = micro_adj_heating_power(TLSS_SMALL, cur_duty_ratio);
		}
		break;

	default:
		debug_error_line();
		break;
	}

	return;
}
#endif

#if 0==USE_GRADUALLY_APPROACHING
static int is_cur_delta_negative(void)
{
	int index;
	int *arry;
	int ret = 0;

	if (delta_dac_per_unit.delta_adc_cnt < 1)
		return 0;

	index = delta_dac_per_unit.delta_adc_cnt % DELTA_ADC_MAX;
	arry  = delta_dac_per_unit.delta_adc_per_unit;

	if (index >= 3) {
		if (arry[index-1]<0)
			ret = 1;
	} else if (2==index) {
		if (arry[index-1]<0)
			ret = 1;
	} else if (1==index) {
		if (arry[index-1]<0)
			ret = 1;
	} else if (0==index) {
		if (arry[DELTA_ADC_MAX-1]<0)
			ret = 1;
	}

	return ret;
}

static int is_cur_delta_positive(void)
{
	int index;
	int *arry;
	int ret = 0;

	if (delta_dac_per_unit.delta_adc_cnt < 1)
		return 0;

	index = delta_dac_per_unit.delta_adc_cnt % DELTA_ADC_MAX;
	arry  = delta_dac_per_unit.delta_adc_per_unit;

	if (index >= 3) {
		if (arry[index-1]>0)
			ret = 1;
	} else if (2==index) {
		if (arry[index-1]>0)
			ret = 1;
	} else if (1==index) {
		if (arry[index-1]>0)
			ret = 1;
	} else if (0==index) {
		if (arry[DELTA_ADC_MAX-1]>0)
			ret = 1;
	}

	return ret;
}


static int is_delta_continue_negative(void)
{
	int index;
	int *arry;
	int ret = 0;

	if (delta_dac_per_unit.delta_adc_cnt < 3)
		return 0;

	index = delta_dac_per_unit.delta_adc_cnt % DELTA_ADC_MAX;
	arry  = delta_dac_per_unit.delta_adc_per_unit;

#if 0
	if (index >= 3) {
		if (arry[index-1]+arry[index-2]+arry[index-3] < 0)
			ret = 1;
	} else if (2==index) {
		if (arry[index-1]+arry[index-2]+arry[DELTA_ADC_MAX-1] < 0)
			ret = 1;
	} else if (1==index) {
		if (arry[index-1]+arry[DELTA_ADC_MAX-1]+arry[DELTA_ADC_MAX-2] < 0)
			ret = 1;
	} else if (0==index) {
		if (arry[DELTA_ADC_MAX-1]+arry[DELTA_ADC_MAX-2]+arry[DELTA_ADC_MAX-3] < 0)
			ret = 1;
	}

#else
	if (index >= 3) {
		if (arry[index-1]<0 && arry[index-2]<0 && arry[index-3]<0)
			ret = 1;
	} else if (2==index) {
		if (arry[index-1]<0 && arry[index-2]<0 && arry[DELTA_ADC_MAX-1]<0)
			ret = 1;
	} else if (1==index) {
		if (arry[index-1]<0 && arry[DELTA_ADC_MAX-1]<0 && arry[DELTA_ADC_MAX-2]<0)
			ret = 1;
	} else if (0==index) {
		if (arry[DELTA_ADC_MAX-1]<0 && arry[DELTA_ADC_MAX-2]<0 && arry[DELTA_ADC_MAX-3]<0)
			ret = 1;
	}
#endif
	return ret;
}

static int is_delta_continue_positive(void)
{
	int index;
	int *arry;
	int ret = 0;

	if (delta_dac_per_unit.delta_adc_cnt < 3)
		return 0;

	index = delta_dac_per_unit.delta_adc_cnt % DELTA_ADC_MAX;
	arry  = delta_dac_per_unit.delta_adc_per_unit;
#if 0
	if (index >= 3) {
		if (arry[index-1]+arry[index-2]+arry[index-3] > 0)
			ret = 1;
	} else if (2==index) {
		if (arry[index-1]+arry[index-2]+arry[DELTA_ADC_MAX-1] > 0)
			ret = 1;
	} else if (1==index) {
		if (arry[index-1]+arry[DELTA_ADC_MAX-1]+arry[DELTA_ADC_MAX-2] > 0)
			ret = 1;
	} else if (0==index) {
		if (arry[DELTA_ADC_MAX-1]+arry[DELTA_ADC_MAX-2]+arry[DELTA_ADC_MAX-3] > 0)
			ret = 1;
	}
#else
	if (index >= 3) {
		if (arry[index-1]>0 && arry[index-2]>0 && arry[index-3]>0)
			ret = 1;
	} else if (2==index) {
		if (arry[index-1]>0 && arry[index-2]>0 && arry[DELTA_ADC_MAX-1]>0)
			ret = 1;
	} else if (1==index) {
		if (arry[index-1]>0 && arry[DELTA_ADC_MAX-1]>0 && arry[DELTA_ADC_MAX-2]>0)
			ret = 1;
	} else if (0==index) {
		if (arry[DELTA_ADC_MAX-1]>0 && arry[DELTA_ADC_MAX-2]>0 && arry[DELTA_ADC_MAX-3]>0)
			ret = 1;
	}

#endif
	return ret;
}


static int get_avg_hp_val_rise_fall(const struct heat_power_val_st *hpv)
{
	int i, avg, cnt;
	const int *buf;

	if (NULL == hpv)
		return 0;

	avg = 0;
	cnt = hpv->cnt;
	buf = hpv->hp_val;
	
	if (0 == cnt) {
		avg = duty_ratio_from_avg_ls;
	} else if (cnt < HEAT_POWER_WHEN_THROUGH_MAX){
		for (i=0; i<cnt; ++i)
			avg += buf[i];
		avg /= cnt;
	} else {
		for (i=0; i<HEAT_POWER_WHEN_THROUGH_MAX; ++i)
			avg += buf[i];
		avg /= HEAT_POWER_WHEN_THROUGH_MAX;
	}


	return avg;
}

/*
 * ls_state -- 当前所处状态
 */
static int set_heating_power_after_through(enum tmper_large_small_state ls_state, int cur_duty_ratio)
{
	int heatp;

	if (get_t8ms_cnt() - large_small_time_cnt >= DELAY_MICRO_ADJ_AFTER_THROUGH) {
		is_through_heating_power  = 0;

		large_small_time_cnt 	  = get_t8ms_cnt();
		large_small_time_cnt4long = large_small_time_cnt;
	} else {
		return cur_duty_ratio;
	}
	
	switch (ls_state) {
	case TLSS_LARGE:
		heatp = get_avg_hp_val_rise_fall(&hp_val_fall);
		break;
	case TLSS_SMALL:
		heatp = get_avg_hp_val_rise_fall(&hp_val_rise);
		break;

	default:
		heatp = STOP_HEATING;
		debug_error_line();
		break;
	}

	set_heating_power_param(heatp);

	return heatp;
}

/*
 * 每隔MICRO_ADJ_TIME8MS_GAP_S微调一次
 *
 * ls_state -- 调用该函数时, 所处的状态
 *
 * 返回占空比的 千分数, FULL_POWER_HEATING
 */
static int micro_adj_heating_power(enum tmper_large_small_state ls_state, int cur_duty_ratio)
{
	int ret;

	switch (ls_state) {
	case TLSS_LARGE:
		if (get_boxtmper_adc_value()>PRE_INCREASE_HEAT_POWER_TADC) {
			if (is_cur_delta_positive())
				cur_duty_ratio += micro_adj_pwm_dutyratio;
			else if (is_delta_continue_positive())
				cur_duty_ratio += 2 * micro_adj_pwm_dutyratio;
			else
				; /* nothing */
		} else {
			if (is_delta_continue_negative())
				cur_duty_ratio = STOP_HEATING;
			else
				cur_duty_ratio -= 3 * micro_adj_pwm_dutyratio;
		}

		ret = cur_duty_ratio;
		break;

	case TLSS_SMALL:
		if (get_boxtmper_adc_value()<PRE_DECREASE_HEAT_POWER_TADC) {
			if (is_cur_delta_negative())
				cur_duty_ratio -= micro_adj_pwm_dutyratio;
			else if (is_delta_continue_negative())
				cur_duty_ratio -= 2 * micro_adj_pwm_dutyratio;
			else
				; /* nothing */
		} else {
			if (is_delta_continue_positive())
				cur_duty_ratio = FULL_POWER_HEATING;
			else
				cur_duty_ratio += 3 * micro_adj_pwm_dutyratio;
		}

		ret = cur_duty_ratio;
		break;

	default:
		ret = STOP_HEATING;
		debug_error_line();
		break;
	}

	set_heating_power_param(ret);

	debug_info_line();
	debug_info(((unsigned)0xaa51<<16 | ret, 4));

	return ret;
}

/*
 * 当温度穿过期望值时
 *
 * ls_state -- 调用该函数时, 所处的状态
 *
 * 返回占空比的 千分数, FULL_POWER_HEATING
 */
static int calc_heating_power_when_through(enum tmper_large_small_state ls_state, int cur_duty_ratio)
{
	unsigned int ret;

	switch (ls_state) {
	case TLSS_LARGE:
		update_int_cycle_buf(&hp_val_rise.cnt, hp_val_rise.hp_val, HEAT_POWER_WHEN_THROUGH_MAX, cur_duty_ratio);
		ret = STOP_HEATING;
		is_through_heating_power = 1;
		break;

	case TLSS_SMALL:
		update_int_cycle_buf(&hp_val_fall.cnt, hp_val_fall.hp_val, HEAT_POWER_WHEN_THROUGH_MAX, cur_duty_ratio);
		ret = FULL_POWER_HEATING;
		is_through_heating_power = 1;
		break;

	default:
		ret = STOP_HEATING;
		debug_error_line();
		break;
	}

	return ret;
}


/*
 * 当温度长时间达不到过期望值时
 *
 * ls_state -- 调用该函数时, 所处的状态
 *
 * 返回占空比的 千分数, FULL_POWER_HEATING
 */
static int calc_heating_power_when_lstate(enum tmper_large_small_state ls_state, int cur_duty_ratio)
{
	int ret;

	switch (ls_state) {
	case TLSS_LARGE:
		ret = cur_duty_ratio - cur_duty_ratio/2;
		break;

	case TLSS_SMALL:
		ret = cur_duty_ratio + cur_duty_ratio/2;
		break;

	default:
		ret = STOP_HEATING;
		debug_error_line();
		break;
	}

	debug_info_line();
	debug_info(((unsigned)0xaa51<<16 | ret, 4));

	return ret;
}
#endif

/*
 * 当温度过期望值时
 *
 * ls_state -- 调用该函数时, 所处的状态
 *
 * 返回占空比的 千分数, FULL_POWER_HEATING
 */
static int calc_heating_power_init(enum tmper_large_small_state ls_state, int cur_duty_ratio)
{
	int ret;
	unsigned int lsum, ssum;

	switch (ls_state) {
	case TLSS_LARGE:
		if (is_not_need_calc()) {
			ret = STOP_HEATING;
		} else {
			calc_ls_time_sum(&lsum, &ssum);
			ret = ssum * FULL_POWER_HEATING / (lsum + ssum);
		}
		break;

	case TLSS_SMALL:
		if (is_not_need_calc()) {
			ret = FULL_POWER_HEATING;
		} else {
			calc_ls_time_sum(&lsum, &ssum);
			ret = ssum * FULL_POWER_HEATING / (lsum + ssum);
		}
		break;

	default:
		ret = STOP_HEATING;
		debug_error_line();
		break;
	}
#if 0
	if (STOP_HEATING!=ret && FULL_POWER_HEATING!=ret)
		ret = ret * 80 / 100;
#endif
	debug_info_line();
	debug_info(((unsigned)0xaa51<<16 | ret, 4));

	return ret;
}



static void update_tmper_large_small_time(int is_large_time, unsigned int time, unsigned duty_ratio)
{
	int index;

	if (is_large_time) {
		index = ls_time_ctrl.large_time_cnt % LARGE_SMALL_TIME_PAIR_MAX;
		ls_time_ctrl.ls_pair[index].large_tcnt	 = time;
		ls_time_ctrl.ls_pair[index].l_duty_ratio = duty_ratio;
		ls_time_ctrl.large_time_cnt 		 += 1;
	} else {
		index = ls_time_ctrl.small_time_cnt % LARGE_SMALL_TIME_PAIR_MAX;
		ls_time_ctrl.ls_pair[index].small_tcnt	 = time;
		ls_time_ctrl.ls_pair[index].s_duty_ratio = duty_ratio;
		ls_time_ctrl.small_time_cnt 		 += 1;
	}

	return;
}

static int calc_ls_time_sum(unsigned int *plsum, unsigned int *pssum)
{
	int i;
	unsigned int lsum, ssum;

	lsum = 0;
	ssum = 0;
	for (i=1; i<LARGE_SMALL_TIME_PAIR_MAX; ++i) {
		lsum += ls_time_ctrl.ls_pair[i].large_tcnt;
		ssum += ls_time_ctrl.ls_pair[i].small_tcnt;
	}

	*plsum = lsum;
	*pssum = ssum;

	debug_info_line();
	debug_info(((unsigned)0xaa51<<16 | lsum, 4));
	debug_info(((unsigned)0xaa51<<16 | ssum, 4));
	
	return 0;
}

static void get_delta_adc_per_unit(void)
{
	static unsigned long get_adc_time_prev;
	static int get_adc_prev;

	if (0 != get_adc_time_prev) {
		if (get_t8ms_cnt() - get_adc_time_prev >= DELTA_ADC_PER_TIME_UNIT) {
			update_delta_adc_per_unit(get_boxtmper_adc_value() - get_adc_prev);
			get_adc_time_prev = get_t8ms_cnt();
			get_adc_prev	  = get_boxtmper_adc_value();
		}
	} else {
		get_adc_time_prev = get_t8ms_cnt();
		get_adc_prev	  = get_boxtmper_adc_value();
	}

	return;
}

static void update_delta_adc_per_unit(int delta_adc)
{
	update_int_cycle_buf(&delta_dac_per_unit.delta_adc_cnt, delta_dac_per_unit.delta_adc_per_unit,
				DELTA_ADC_MAX, delta_adc);
	return;
}

static void update_int_cycle_buf(unsigned int *cnt, int *ic_buf, int buf_len, int data)
{
	int index;

	if (NULL==cnt || NULL==ic_buf || buf_len<=0)
		return;

	if (STOP_HEATING==data || FULL_POWER_HEATING==data)
		return;

	index = *cnt % buf_len;

	ic_buf[index] = data;
	*cnt += 1;

	return;	
}


#if 1
void print_delta_buf_info(void)
{
	int i;
	unsigned int temp;

	send_data_to_pc(delta_dac_per_unit.delta_adc_cnt, 4);

	for (i=0; i<DELTA_ADC_MAX; ++i) {
		if (delta_dac_per_unit.delta_adc_per_unit[i] < 0) {
			temp = -delta_dac_per_unit.delta_adc_per_unit[i];
			temp |= 1U<<31;
		} else {
			temp = delta_dac_per_unit.delta_adc_per_unit[i];
		}

		send_data_to_pc(delta_dac_per_unit.delta_adc_per_unit[i], 4);
	}

	return;
}

void set_get_micro_param(int id, int data)
{
	if (data <= 0)
		debug_error_line();

	switch (id) {
	case 1:
		if (data<300*125)
			micro_adj_t8ms_gap_s	= data;
		else
			debug_error_line();
		break;

	case 2:
		if (data<800)
			micro_adj_pwm_dutyratio = data;
		else
			debug_error_line();
		break;

	case 3:
		debug_info((micro_adj_t8ms_gap, 4));
		debug_info((micro_adj_pwm_dutyratio, 4));
		break;

	case 4:
		if (data<300*125)
			micro_adj_t8ms_gap_l	= data;
		else
			debug_error_line();
		break;

	default:
		debug_error_line();
		break;	
	}
}
#endif

