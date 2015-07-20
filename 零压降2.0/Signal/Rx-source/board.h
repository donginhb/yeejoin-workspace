#ifndef BOARD_H__
#define BOARD_H__

#include "sys_comm.h"

#define led1_rcc    RCC_APB2Periph_GPIOB
#define led1_gpio   GPIOB
#define led1_pin    (GPIO_Pin_13)

#define led2_rcc    RCC_APB2Periph_GPIOB
#define led2_gpio   GPIOB
#define led2_pin    (GPIO_Pin_11)
#if 0
#define led3_rcc    RCC_APB2Periph_GPIOB
#define led3_gpio   GPIOB
#define led3_pin    (GPIO_Pin_15)
#endif
#define ledall_rcc   (led1_rcc | led2_rcc | led3_rcc)

#define led_on(port, pin)     clr_port_pin(port, pin)
#define led_off(port, pin)    set_port_pin(port, pin)
#define is_led_off(port, pin) pin_out_is_set(port, pin)
#define led_blink(port, pin)  rolling_over_port_pin(port, pin)

#define blink_running_led()	led_blink(led1_gpio, led1_pin)
#define on_running_led()	led_on(led1_gpio, led1_pin)
#define off_running_led()	led_off(led1_gpio, led1_pin)

/*
 * px����ʱ, ��Ӧ����Ϊ�͵�ƽ
 */
#define pa_too_heigh_rcc    	RCC_APB2Periph_GPIOB
#define pa_too_heigh_gpio   	GPIOB
#define pa_too_heigh_pin    	(GPIO_Pin_9)

#define pa_too_low_rcc		RCC_APB2Periph_GPIOB
#define pa_too_low_gpio		GPIOB
#define pa_too_low_pin		(GPIO_Pin_8)

#define pb_too_heigh_rcc	RCC_APB2Periph_GPIOB
#define pb_too_heigh_gpio   	GPIOB
#define pb_too_heigh_pin    	(GPIO_Pin_7)

#define pb_too_low_rcc    	RCC_APB2Periph_GPIOB
#define pb_too_low_gpio   	GPIOB
#define pb_too_low_pin    	(GPIO_Pin_6)

#define pc_too_heigh_rcc    	RCC_APB2Periph_GPIOB
#define pc_too_heigh_gpio   	GPIOB
#define pc_too_heigh_pin    	(GPIO_Pin_5)

#define pc_too_low_rcc    	RCC_APB2Periph_GPIOB
#define pc_too_low_gpio  	GPIOB
#define pc_too_low_pin    	(GPIO_Pin_4)

#define px_too_lh_gpio  	GPIOB

#define is_px_too_heigh(port, pin)	(!pin_in_is_set(port, pin))
#define is_px_too_low(port, pin) 	(!pin_in_is_set(port, pin))

/* �����л����� */
#define px_falt_sw_rcc    	RCC_APB2Periph_GPIOA
#define px_falt_sw_gpio  	GPIOA
#define px_falt_sw_pin    	(GPIO_Pin_12)

#if 0
/*
 * ���������3�������ǿ���л���pt:
 * 1. ��⵽������ߡ�����ʱ
 * 2. ���յ�lcd-cpu��ǿ���л�����
 * 3. ͨ��ָʾ�ź��쳣
 * */

#define force_output_switch2pt()	clr_port_pin(px_falt_sw_gpio, px_falt_sw_pin)
#define cancel_force_output_switch2pt()	set_port_pin(px_falt_sw_gpio, px_falt_sw_pin)
#else
enum user_id_of_use_switch2pt_e {
	UIUS_OUTPUT_TOO_LOW_OR_HEIGH	= 0X01,
	UIUS_CHANNEL_INDICATION		= 0X02,
	UIUS_SWITCH_CMD_FROM_LCD_CPU	= 0X04,
};

extern void force_output_switch2pt(enum user_id_of_use_switch2pt_e uid);
extern void cancel_force_output_switch2pt(enum user_id_of_use_switch2pt_e uid);
#endif

/*
 * ���ڱ����̵���
 */
#define protect_electric_relay_rcc    	pa_too_heigh_rcc
#define protect_electric_relay_gpio   	pa_too_heigh_gpio
#define protect_electric_relay_pin    	pa_too_heigh_pin


#define switch2pt_indication_rcc    RCC_APB2Periph_GPIOA
#define switch2pt_indication_gpio   GPIOA
#define switch2pt_indication_pin    (GPIO_Pin_15)


#define is_had_force_output_switch2pt() (!pin_out_is_set(px_falt_sw_gpio, px_falt_sw_pin))

#if USE_OPTICX_200S_VERSION
/*
 * ********************************************************************
 * ����200s�豸��˫ͨ��ѡ��
 * ********************************************************************
 * */
/*
 * ʹ��ͨ��ָʾ�ź�
 *
 * ����̵�����������뿪©���ṹ��������ЧʱΪ�͵�ƽ
 * ���ǵ�Ӳ���Ѹ��ź�ͨ�������������ӵ�cpu��Ҳ����˵cpu���Ŷ���0����ʾδʹ��
 * */
/* ʹ��ͨ����ָʾ�źŵ���Чֵ */
#define CHANNEL_INDICATION_VALID_VALUE 1

#define use_channel_1_indication_rcc    RCC_APB2Periph_GPIOB
#define use_channel_1_indication_gpio   GPIOB
#define use_channel_1_indication_pin    (GPIO_Pin_14)

#define use_channel_2_indication_rcc    RCC_APB2Periph_GPIOB
#define use_channel_2_indication_gpio   GPIOB
#define use_channel_2_indication_pin    (GPIO_Pin_15)


/*
 * ͨ���л������ź�
 *
 * 74LVC1G3157
 * QY:
 * 	0 -- B0(channel-0), pin3 -- 2#����ͷ�ź�
 * 	1 -- B1(channel-1), pin1 -- 1#����ͷ�ź�
 * */
#define use_channel_x_ctrl_rcc    RCC_APB2Periph_GPIOA
#define use_channel_x_ctrl_gpio   GPIOA
#define use_channel_x_ctrl_pin    (GPIO_Pin_8)

/*
 * 0 -- invalid value
 * 1 -- use_channel_1_indication
 * 2 -- use_channel_2_indication
 * 3 -- invalid value
 * */
#define get_channel_indication_value() (((GPIOB->IDR)>>14) & 0x03)

#if 0
#define set_to_use_optical_fiber1() do {\
	set_port_pin(use_channel_x_ctrl_gpio, use_channel_x_ctrl_pin);\
	cur_fiber_channel_no = 0;\
	real_pa_zero_position_value = signal_cfg_param.px_adj_param[0].pa_zeropos_real_val;\
	real_pb_zero_position_value = signal_cfg_param.px_adj_param[0].pb_zeropos_real_val;\
	real_pc_zero_position_value = signal_cfg_param.px_adj_param[0].pc_zeropos_real_val;\
} while (0)
#define set_to_use_optical_fiber2() do {\
	clr_port_pin(use_channel_x_ctrl_gpio, use_channel_x_ctrl_pin);\
	cur_fiber_channel_no = 1;\
	real_pa_zero_position_value = signal_cfg_param.px_adj_param[1].pa_zeropos_real_val;\
	real_pb_zero_position_value = signal_cfg_param.px_adj_param[1].pb_zeropos_real_val;\
	real_pc_zero_position_value = signal_cfg_param.px_adj_param[1].pc_zeropos_real_val;\
} while (0)
#else
extern void set_to_use_optical_fiber1(void);
extern void set_to_use_optical_fiber2(void);
#endif
#endif

/*
 * 1s�жԵ�ѹƽ��ֵ�Լ��¶ȶ�Ӧ�ĵ�ѹ, ����125������
 * һ����Ч������125��adcֵ��ƽ��ֵ(��Ҫȥ�����ֵ����Сֵ)
 *
 * tim2-7, tim12-14��clk max = 36 MHz
 * TIM3ʱ��360��Ƶ��Ϊ0.1MHz, 100 tick = 1 ms, David
 */
#define UPDATE_DATA_CNT_PER_SEC (1)
#define ADC_CNT_PER_SAMPLE	(125)
#define TIM3_PERIOD_MS 		((1000/(UPDATE_DATA_CNT_PER_SEC*ADC_CNT_PER_SAMPLE)))
#define TIM3_PERIOD_MS_TICK_CNT (TIM3_PERIOD_MS * 100)

#define TIM3_INT_CNT_PER1S	(1000/TIM3_PERIOD_MS)

#if 1 /* new hw */
#define turnon_110v_power()  set_port_pin(GPIOC, GPIO_Pin_13)
#define turnoff_110v_power() clr_port_pin(GPIOC, GPIO_Pin_13)
#else /* old hw */
#define turnon_110v_power()  clr_port_pin(GPIOC, GPIO_Pin_13)
#define turnoff_110v_power() set_port_pin(GPIOC, GPIO_Pin_13)
#endif

extern void clr_dac(void);



#define USE_PVD 0
#endif

