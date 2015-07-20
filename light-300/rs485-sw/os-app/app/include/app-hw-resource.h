/*
 * app-hw-resource.h
 *
 *  Created on: 2013-11-4
 *      Author: David, zhaoshaowei@yeejoin.com
 */

#ifndef APP_HW_RESOURCE_H_
#define APP_HW_RESOURCE_H_

#include <rtconfig.h>

/*
 * ---- 中断优先级 ----
 * 	NVIC_IRQChannelPreemptionPriority
 *	NVIC_IRQChannelSubPriority
 *
 * */

/* Configure one bit for preemption priority */
#define NVIC_PRIORITY_GRP	NVIC_PriorityGroup_3

//#define ENC28J60_PREEMPTION_PRI		1
//#define ENC28J60_SUB_PRI		0

#define USART1_PREEMPTION_PRI		7
#define USART1_SUB_PRI			0

#define USART2_PREEMPTION_PRI		2
#define USART2_SUB_PRI			1

#define USART3_PREEMPTION_PRI		4
#define USART3_SUB_PRI			0

#define UART4_PREEMPTION_PRI		2
#define UART4_SUB_PRI			0

#define UART5_PREEMPTION_PRI		2
#define UART5_SUB_PRI			0

#define RTC_PREEMPTION_PRI		6
#define RTC_SUB_PRI			0

#define TL16_1_PREEMPTION_PRI		1
#define TL16_1_SUB_PRI			0

#define TL16_2_PREEMPTION_PRI		1
#define TL16_2_SUB_PRI			1

#define TIM2_PREEMPTION_PRI		5
#define TIM2_SUB_PRI			1


#if LOWPOWER_MODLE
#define RTC_ALARM_PREEMPTION_PRI	1
#define RTC_ALARM_SUB_PRI		1
#endif


/*
 * ---- timer ----
 *
 * si4432
 * 	tim2 -- 发送/接收数据超时
 * 	tim3 -- 关联/数据请求时，状态机的状态切换
 *
 * ade7880
 * 	tim4 -- 用于电压、电流采样值
 * 	tim5 -- 用于计量电能脉冲时间
 * */
#if 0
#define SI4432_MAC_TIMER		TIM2
#define SI4432_MAC_TIMER_RCC		RCC_APB1Periph_TIM2
#define SI4432_MAC_TIMER_PRESCALER	(72)

#define SI4432_MAC_TIMER_IRQ_VECTER	TIM2_IRQn
#define SI4432_MAC_TIMER_PREEMPTION_PRI	TIM2_PREEMPTION_PRI
#define SI4432_MAC_TIMER_SUB_PRI	TIM2_SUB_PRI
#else
#define AUTO_NEGOTI_TIMER		TIM2
#define AUTO_NEGOTI_TIMER_RCC		RCC_APB1Periph_TIM2
#define AUTO_NEGOTI_TIMER_PRESCALER	(60000)

#define AUTO_NEGOTI_TIMER_IRQ_VECTER	TIM2_IRQn
#define AUTO_NEGOTI_TIMER_PREEMPTION_PRI	TIM2_PREEMPTION_PRI
#define AUTO_NEGOTI_TIMER_SUB_PRI	TIM2_SUB_PRI

#define get_auto_negoti_tikcs_of_ms(xms) ((xms) * 72000/(AUTO_NEGOTI_TIMER_PRESCALER))
#endif

#define CHECK_E_ENERGY_TIMER		TIM5
#define CHECK_E_ENERGY_TIMER_RCC	RCC_APB1Periph_TIM5



/*
 * ---- ext it ----
 *
 * ext5 -- enc28j60
 * ext3 -- si4432
 *
 * */
#define ENC28J60_INT_PORT		GPIOB
#define ENC28J60_INT_PIN		GPIO_Pin_1
#define ENC28J60_INT_NUM 		EXTI1_IRQn
#define ENC28J60_INT_PORT_SOURCE	GPIO_PortSourceGPIOB
#define ENC28J60_INT_PIN_SOURCE		GPIO_PinSource1
#define ENC28J60_INT_EXT_LINE		EXTI_Line1


/* 当前没有使用ade7880的IRQ0和IRQ1 */
#define ADE7880_INT0_PORT		GPIOB
#define ADE7880_INT0_PIN		GPIO_Pin_0
#define ADE7880_INT0_NUM 		EXTI0_IRQn
#define ADE7880_INT0_PORT_SOURCE	GPIO_PortSourceGPIOB
#define ADE7880_INT0_PIN_SOURCE		GPIO_PinSource0
#define ADE7880_INT0_EXT_LINE		EXTI_Line0

#define ADE7880_INT1_PORT		GPIOB
#define ADE7880_INT1_PIN		GPIO_Pin_6
#define ADE7880_INT1_NUM 		EXTI9_5_IRQn
#define ADE7880_INT1_PORT_SOURCE	GPIO_PortSourceGPIOB
#define ADE7880_INT1_PIN_SOURCE		GPIO_PinSource6
#define ADE7880_INT1_EXT_LINE		EXTI_Line6

/*
 * 当前新硬件(20140311)使用pc7
 *
 * 需要修改为pb2, 飞线 --- 不用飞线了，em测脉冲宽度，但是，不使用si4432
 * */
#define SI4432_INT_PORT			GPIOC
#define SI4432_INT_PIN			GPIO_Pin_7
#define SI4432_INT_NUM 			EXTI9_5_IRQn
#define SI4432_INT_PORT_SOURCE		GPIO_PortSourceGPIOC
#define SI4432_INT_PIN_SOURCE		GPIO_PinSource7
#define SI4432_INT_EXT_LINE		EXTI_Line7

/*
 * 电表电能脉冲中断
 * */
#define EM_ACTIVE_ENERGY_INT_PORT		GPIOB
#define EM_ACTIVE_ENERGY_INT_PIN		GPIO_Pin_8
#define EM_ACTIVE_ENERGY_INT_NUM 		EXTI9_5_IRQn
#define EM_ACTIVE_ENERGY_INT_PORT_SOURCE	GPIO_PortSourceGPIOB
#define EM_ACTIVE_ENERGY_INT_PIN_SOURCE		GPIO_PinSource8
#define EM_ACTIVE_ENERGY_INT_EXT_LINE		EXTI_Line8

#define EM_REACTIVE_ENERGY_INT_PORT		GPIOB
#define EM_REACTIVE_ENERGY_INT_PIN		GPIO_Pin_9
#define EM_REACTIVE_ENERGY_INT_NUM 		EXTI9_5_IRQn
#define EM_REACTIVE_ENERGY_INT_PORT_SOURCE	GPIO_PortSourceGPIOB
#define EM_REACTIVE_ENERGY_INT_PIN_SOURCE	GPIO_PinSource9
#define EM_REACTIVE_ENERGY_INT_EXT_LINE		EXTI_Line9

/*
 * ADE7880电能脉冲中断
 * */
#define ADE7880_ACTIVE_ENERGY_INT_PORT		GPIOA
#define ADE7880_ACTIVE_ENERGY_INT_PIN		GPIO_Pin_11
#define ADE7880_ACTIVE_ENERGY_INT_NUM 		EXTI15_10_IRQn
#define ADE7880_ACTIVE_ENERGY_INT_PORT_SOURCE	GPIO_PortSourceGPIOA
#define ADE7880_ACTIVE_ENERGY_INT_PIN_SOURCE	GPIO_PinSource11
#define ADE7880_ACTIVE_ENERGY_INT_EXT_LINE	EXTI_Line11

#define ADE7880_REACTIVE_ENERGY_INT_PORT	GPIOA
#define ADE7880_REACTIVE_ENERGY_INT_PIN		GPIO_Pin_12
#define ADE7880_REACTIVE_ENERGY_INT_NUM 	EXTI15_10_IRQn
#define ADE7880_REACTIVE_ENERGY_INT_PORT_SOURCE	GPIO_PortSourceGPIOA
#define ADE7880_REACTIVE_ENERGY_INT_PIN_SOURCE	GPIO_PinSource12
#define ADE7880_REACTIVE_ENERGY_INT_EXT_LINE	EXTI_Line12

/*
 *
 * */
#define tl16c554_1_intx_gpio		GPIOA
#define tl16c554_1_intx_pin		GPIO_Pin_4
#define TL16C554_1_INT_NUM 		EXTI4_IRQn
#define TL16C554_1_INT_PORT_SOURCE	GPIO_PortSourceGPIOA
#define TL16C554_1_INT_PIN_SOURCE	GPIO_PinSource4
#define TL16C554_1_INT_EXT_LINE		EXTI_Line4

#define tl16c554_2_intx_gpio		GPIOF
#define tl16c554_2_intx_pin		GPIO_Pin_6
#define TL16C554_2_INT_NUM 		EXTI9_5_IRQn
#define TL16C554_2_INT_PORT_SOURCE	GPIO_PortSourceGPIOF
#define TL16C554_2_INT_PIN_SOURCE	GPIO_PinSource6
#define TL16C554_2_INT_EXT_LINE		EXTI_Line6


/*
 **************************************************************************************************
 * spi cfg
 **************************************************************************************************
 */

/*
 * SPI1
 * PA4 -- NSS, PA5 -- SCK, PA6 -- MISO, PA7 -- MOSI
 */
#define SPI1_RCC_PERIPH		RCC_APB2Periph_SPI1
#define SPI1_PORT 		GPIOA
#define SPI1_NSS_PORT 		GPIOA
#define SPI1_SCK		GPIO_Pin_5
#define SPI1_MISO		GPIO_Pin_6
#define SPI1_MOSI		GPIO_Pin_7
#define SPI1_NSS		GPIO_Pin_4

/*
 * SPI2
 * PB12 -- NSS, PB13 -- SCK, PB14 -- MISO, PB15 -- MOSI
 */
#define SPI2_RCC_PERIPH		RCC_APB1Periph_SPI2
#define SPI2_PORT 		GPIOB
#define SPI2_NSS_PORT 		GPIOB
#define SPI2_SCK		GPIO_Pin_13
#define SPI2_MISO		GPIO_Pin_14
#define SPI2_MOSI		GPIO_Pin_15
#define SPI2_NSS		GPIO_Pin_12

/*
 * SPI3
 * PA15 -- NSS, PB3 -- SCK, PB4 -- MISO, PB5 -- MOSI
 */
#define SPI3_RCC_PERIPH		RCC_APB1Periph_SPI3
#define SPI3_PORT 		GPIOB
#define SPI3_NSS_PORT 		GPIOA
#define SPI3_SCK		GPIO_Pin_3
#define SPI3_MISO		GPIO_Pin_4
#define SPI3_MOSI		GPIO_Pin_5
#define SPI3_NSS		GPIO_Pin_15

/*
 * SPI NAND Flash
 */
#define SF_SPIX			SPI1
#define SF_SPI_RCC_PERIPH	SPI1_RCC_PERIPH
#define SF_SPI_PORT 		SPI1_PORT
#define SF_SPI_NSS_PORT		SPI1_NSS_PORT
#define SF_SPI_SCK		SPI1_SCK
#define SF_SPI_MISO		SPI1_MISO
#define SF_SPI_MOSI		SPI1_MOSI
#define SF_SPI_NSS		SPI1_NSS

#define SF_CS_PORT 		SF_SPI_NSS_PORT
#define SF_CS_PIN 		SF_SPI_NSS

/*
 * ENC28J60
 */
#define ENC28J60_SPIX		SPI2
#define ENC28J60_SPI_RCC_PERIPH	SPI2_RCC_PERIPH
#define ENC28J60_SPI_PORT 	SPI2_PORT
#define ENC28J60_SPI_NSS_PORT 	SPI2_NSS_PORT
#define ENC28J60_SPI_SCK	SPI2_SCK
#define ENC28J60_SPI_MISO	SPI2_MISO
#define ENC28J60_SPI_MOSI	SPI2_MOSI
#define ENC28J60_SPI_NSS	SPI2_NSS

#define ENC28J60_CS_PORT 	ENC28J60_SPI_NSS_PORT
#define ENC28J60_CS_PIN  	ENC28J60_SPI_NSS


/*
 * ade7880
 */

/* spi mode */
#define ADE7880_SPIX		SPI1
#define ADE7880_SPI_RCC_PERIPH	SPI1_RCC_PERIPH
#define ADE7880_SPI_PORT 	SPI1_PORT
#define ADE7880_SPI_NSS_PORT	SPI1_NSS_PORT
#define ADE7880_SPI_SCK		SPI1_SCK
#define ADE7880_SPI_MISO	SPI1_MISO
#define ADE7880_SPI_MOSI	SPI1_MOSI 
#define ADE7880_SPI_NSS		SPI1_NSS

/* i2c + HSDC  mode */
#define ADE7880_HSDC_RCC_PERIPH	SPI1_RCC_PERIPH
#define ADE7880_HSDC_PORT 	SPI1_PORT

#define ADE7880_HSDC_SCK	SPI1_SCK
#define ADE7880_HSDC_MISO	SPI1_MISO

#define ADE7880_CS_PORT 	GPIOC
#define ADE7880_CS_PIN 		GPIO_Pin_3


/*
 * si4432
 */
#define SI4432_SPIX		SPI3
#define SI4432_SPI_RCC_PERIPH	SPI3_RCC_PERIPH
#define SI4432_SPI_PORT 	SPI3_PORT
#define SI4432_SPI_NSS_PORT	SPI3_NSS_PORT
#define SI4432_SPI_SCK		SPI3_SCK
#define SI4432_SPI_MISO		SPI3_MISO
#define SI4432_SPI_MOSI		SPI3_MOSI
#define SI4432_SPI_NSS		SPI3_NSS

#define SI4432_CS_PORT 	GPIOA
#define SI4432_CS_PIN 	GPIO_Pin_15


/*
 **************************************************************************************************
 * I2C cfg
 **************************************************************************************************
 */
/*
 * I2C2
 * PB10 -- SCL, PB11 -- SDA,
 */
#define I2C2_SCL		GPIO_Pin_10
#define I2C2_SDA		GPIO_Pin_11
#define I2C2_PORT		GPIOB
#define I2C2_RCC_PERIPH		RCC_APB1Periph_I2C2

/* I2C */
#define ADE7880_I2CX		I2C2
#define ADE7880_I2C_SCL		I2C2_SCL
#define ADE7880_I2C_SDA		I2C2_SDA



/*
 **************************************************************************************************
 * misc cfg
 **************************************************************************************************
 */

/*
 * ADE7880
 */
#if EM_MULTI_BASE
#define ADE7880_RESET_RCC	RCC_APB2Periph_GPIOG
#define ADE7880_RESET_GPIO	GPIOG
#define ADE7880_RESET_PIN	GPIO_Pin_6
#else
#define ADE7880_RESET_RCC	RCC_APB2Periph_GPIOC
#define ADE7880_RESET_GPIO	GPIOC
#define ADE7880_RESET_PIN	GPIO_Pin_12
#endif

#define ADE7880_PM0_RCC		RCC_APB2Periph_GPIOD
#define ADE7880_PM0_GPIO        GPIOD
#define ADE7880_PM0_PIN         GPIO_Pin_3

#if EM_MULTI_BASE
#define ADE7880_PM1_RCC		RCC_APB2Periph_GPIOF
#define ADE7880_PM1_GPIO        GPIOF
#define ADE7880_PM1_PIN         GPIO_Pin_10
#else
#define ADE7880_PM1_RCC		RCC_APB2Periph_GPIOD
#define ADE7880_PM1_GPIO        GPIOD
#define ADE7880_PM1_PIN         GPIO_Pin_2
#endif

#define ADE7880_IRQ0_RCC	RCC_APB2Periph_GPIOB
#define ADE7880_IRQ0_GPIO       GPIOB
#define ADE7880_IRQ0_PIN        GPIO_Pin_7

#define ADE7880_IRQ1_RCC	RCC_APB2Periph_GPIOB
#define ADE7880_IRQ1_GPIO       GPIOB
#define ADE7880_IRQ1_PIN        GPIO_Pin_6

/* ADE7880电能脉冲输入接口 */
#define ADE7880_ACTIVE_ENERGY_RCC		RCC_APB2Periph_GPIOA
#define ADE7880_ACTIVE_ENERGY_GPIO       	GPIOA
#define ADE7880_ACTIVE_ENERGY_PIN        	GPIO_Pin_11

#define ADE7880_REACTIVE_ENERGY_RCC		RCC_APB2Periph_GPIOA
#define ADE7880_REACTIVE_ENERGY_GPIO       	GPIOA
#define ADE7880_REACTIVE_ENERGY_PIN        	GPIO_Pin_12

/*
 * 电表电能脉冲输入接口
 * */
#define EM_ACTIVE_ENERGY_RCC		RCC_APB2Periph_GPIOB
#define EM_ACTIVE_ENERGY_GPIO       	GPIOB
#define EM_ACTIVE_ENERGY_PIN        	GPIO_Pin_8

#define EM_REACTIVE_ENERGY_RCC		RCC_APB2Periph_GPIOB
#define EM_REACTIVE_ENERGY_GPIO       	GPIOB
#define EM_REACTIVE_ENERGY_PIN        	GPIO_Pin_9


/*
 * SI4432
 */
#define SI4432_GPIO_0_RCC	RCC_APB2Periph_GPIOA
#define SI4432_GPIO_0_GPIO	GPIOA
#define SI4432_GPIO_0_PIN	GPIO_Pin_8

#define SI4432_GPIO_1_RCC	RCC_APB2Periph_GPIOC
#define SI4432_GPIO_1_GPIO      GPIOC
#define SI4432_GPIO_1_PIN       GPIO_Pin_9

#define SI4432_GPIO_2_RCC	RCC_APB2Periph_GPIOC
#define SI4432_GPIO_2_GPIO      GPIOC
#define SI4432_GPIO_2_PIN       GPIO_Pin_8

#define SI4432_NIRQ_RCC		RCC_APB2Periph_GPIOC
#define SI4432_NIRQ_GPIO       	GPIOC
#define SI4432_NIRQ_PIN        	GPIO_Pin_7

#define SI4432_SDN_RCC		RCC_APB2Periph_GPIOC
#define SI4432_SDN_GPIO       	GPIOC
#define SI4432_SDN_PIN        	GPIO_Pin_6


/*
 * SPI NAND Flash
 */
#define SF_HOLD_PORT 	GPIOA
#define SF_HOLD_PIN  	GPIO_Pin_0

#define SF_WP_PORT 	GPIOA
#define SF_WP_PIN  	GPIO_Pin_1


/*
 * ENC28J60
 */

#define ENC28J60_RST_PORT	GPIOC
#define ENC28J60_RST_PIN	GPIO_Pin_4


#endif /* APP_HW_RESOURCE_H_ */
