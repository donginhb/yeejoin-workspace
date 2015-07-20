#ifndef BOARD_H__
#define BOARD_H__

#include "sys_comm.h"

#define TEST_TX_POWEROFF 0
#define USE_PVD_CHECK_POWEROFF 1

#define led1_rcc    RCC_APB2Periph_GPIOA
#define led1_gpio   GPIOA
#define led1_pin    (GPIO_Pin_15)

#define led2_rcc    RCC_APB2Periph_GPIOB
#define led2_gpio   GPIOB
#define led2_pin    (GPIO_Pin_3)

#define led3_rcc    RCC_APB2Periph_GPIOB
#define led3_gpio   GPIOB
#define led3_pin    (GPIO_Pin_4)

#define led4_rcc    RCC_APB2Periph_GPIOB
#define led4_gpio   GPIOB
#define led4_pin    (GPIO_Pin_5)

#define led5_rcc    RCC_APB2Periph_GPIOB
#define led5_gpio   GPIOB
#define led5_pin    (GPIO_Pin_6)

#define ledall_rcc   (led1_rcc | led2_rcc | led2_rcc | led3_rcc | led5_rcc)

#define led_on(port, pin)     set_port_pin(port, pin)
#define led_off(port, pin)    clr_port_pin(port, pin)
#define is_led_off(port, pin) !pin_out_is_set(port, pin)
#define led_blink(port, pin)  rolling_over_port_pin(port, pin)

#define blink_running_led()	led_blink(led1_gpio, led1_pin)
#define on_running_led()	led_on(led1_gpio, led1_pin)
#define off_running_led()	led_off(led1_gpio, led1_pin)

#if TEST_TX_POWEROFF
#define blink_send_data_led()
#define on_send_data_led()
#define off_send_data_led()

#define int_led_blink() led_blink(led2_gpio, led2_pin)
#define int_led_on()    led_on(led2_gpio, led2_pin)
#define int_led_off()   led_off(led2_gpio, led2_pin)

#else

#define blink_send_data_led()	led_blink(led2_gpio, led2_pin)
#define on_send_data_led()	led_on(led2_gpio, led2_pin)
#define off_send_data_led()	led_off(led2_gpio, led2_pin)

#endif

#define blink_pa_losting_phase_led() led_blink(led3_gpio, led3_pin)
#define blink_pb_losting_phase_led() led_blink(led4_gpio, led4_pin)
#define blink_pc_losting_phase_led() led_blink(led5_gpio, led5_pin)

#define on_pa_losting_phase_led() led_on(led3_gpio, led3_pin)
#define on_pb_losting_phase_led() led_on(led4_gpio, led4_pin)
#define on_pc_losting_phase_led() led_on(led5_gpio, led5_pin)

#define off_pa_losting_phase_led() led_off(led3_gpio, led3_pin)
#define off_pb_losting_phase_led() led_off(led4_gpio, led4_pin)
#define off_pc_losting_phase_led() led_off(led5_gpio, led5_pin)

/*
 * 1s中对电压平均值以及温度对应的电压, 进行125个采样
 * 一个有效采样是125个adc值的平均值(需要去掉最大值与最小值)
 *
 * tim2-7, tim12-14的clk max = 36 MHz
 * TIM3时钟360分频后为0.1MHz, 100 tick = 1 ms, David 
 */
#define UPDATE_DATA_CNT_PER_SEC (1)
#define ADC_CNT_PER_SAMPLE	(125)
#define TIM3_PERIOD_MS 		((1000/(UPDATE_DATA_CNT_PER_SEC*ADC_CNT_PER_SAMPLE)))
#define TIM3_PERIOD_MS_TICK_CNT (TIM3_PERIOD_MS * 100)

#define TIM3_INT_CNT_PER1S	(1000/TIM3_PERIOD_MS)


/*
 * ADS8329, stm32--SPI1
 *
 * SPI1
 * PA4 -- NSS, PA5 -- SCK, PA6 -- MISO, PA7 -- MOSI
 */
#define ADS8329_SPI_RCC_PERIPH	RCC_APB2Periph_SPI1
#define ADS8329_SPI_PORT 	GPIOA
#define ADS8329_SPI_SCK		GPIO_Pin_5
#define ADS8329_SPI_MISO	GPIO_Pin_6
#define ADS8329_SPI_MOSI	GPIO_Pin_7
#define ADS8329_SPI_NSS		GPIO_Pin_4

#define ADS8329_CS_PORT 	GPIOB
#define ADS8329_CS_PIN  	GPIO_Pin_0

/* ads8329 input pin, Freezes sample and hold, starts conversion with next rising edge of internal clock */
#define ADS8329_NCONVST_PORT 	GPIOB
#define ADS8329_NCONVST_PIN  	GPIO_Pin_2

/* ads8329 output pin, If programmed as EOC, this pin is low (default) when a conversion is in progress */
#define ADS8329_EOC_PORT 	GPIOB
#define ADS8329_EOC_PIN      	GPIO_Pin_1

#define SELECT_PA_CHANNEL_PORT	GPIOB
#define SELECT_PA_CHANNEL_PIN	GPIO_Pin_11
#define SELECT_PB_CHANNEL_PORT	GPIOB
#define SELECT_PB_CHANNEL_PIN	GPIO_Pin_10
#define SELECT_PC_CHANNEL_PORT	GPIOA
#define SELECT_PC_CHANNEL_PIN	GPIO_Pin_4

#define PA_ADS8329_CH_PIN (GPIO_Pin_11)
#define PB_ADS8329_CH_PIN (GPIO_Pin_10)
#define PC_ADS8329_CH_PIN (GPIO_Pin_4)

/* ABC三相通道模拟开关, 0 -- open, 1 -- close */
#define open_pa_switch()  clr_port_pin(SELECT_PA_CHANNEL_PORT, SELECT_PA_CHANNEL_PIN)
#define close_pa_switch() set_port_pin(SELECT_PA_CHANNEL_PORT, SELECT_PA_CHANNEL_PIN)

#define open_pb_switch()  clr_port_pin(SELECT_PB_CHANNEL_PORT, SELECT_PB_CHANNEL_PIN)
#define close_pb_switch() set_port_pin(SELECT_PB_CHANNEL_PORT, SELECT_PB_CHANNEL_PIN)

#define open_pc_switch()  clr_port_pin(SELECT_PC_CHANNEL_PORT, SELECT_PC_CHANNEL_PIN)
#define close_pc_switch() set_port_pin(SELECT_PC_CHANNEL_PORT, SELECT_PC_CHANNEL_PIN)

#endif

