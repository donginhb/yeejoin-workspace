/*
 * File      : board.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-09-22     Bernard      add board.h to this bsp
 */

// <<< Use Configuration Wizard in Context Menu >>>
#ifndef __BOARD_H__
#define __BOARD_H__

#include <rtdef.h>
#include <stm32f10x.h>
#include <stm32f10x_bkp.h>
#include <syscfgdata.h>


#define USE_STM32_IWDG 1

/* board configuration */
// <o> SDCard Driver <1=>SDIO sdcard <0=>SPI MMC card
// 	<i>Default: 1
#define STM32_USE_SDIO			0

/* whether use board external SRAM memory */
// <e>Use external SRAM memory on the board
// 	<i>Enable External SRAM memory
#define STM32_EXT_SRAM          0
//	<o>Begin Address of External SRAM
//		<i>Default: 0x68000000
#define STM32_EXT_SRAM_BEGIN    0x68000000 /* the begining address of external SRAM */
//	<o>End Address of External SRAM
//		<i>Default: 0x68080000
#define STM32_EXT_SRAM_END      0x68080000 /* the end address of external SRAM */
// </e>

// <o> Internal SRAM memory size[Kbytes] <8-64>
//	<i>Default: 64
#define STM32_SRAM_SIZE         64
#define STM32_SRAM_END          (0x20000000 + STM32_SRAM_SIZE * 1024)

// <o> Console on USART: <0=> no console <1=>USART 1 <2=>USART 2 <3=> USART 3
// 	<i>Default: 1
#define STM32_CONSOLE_USART		1

// <o> Ethernet Interface: <0=> Microchip ENC28J60 <1=> Davicom DM9000A
// 	<i>Default: 0
#define STM32_ETH_IF			1

void rt_hw_board_led_on(int n);
void rt_hw_board_led_off(int n);
void rt_hw_board_init(void);

#if STM32_CONSOLE_USART == 0
#define CONSOLE_DEVICE "no"
#elif STM32_CONSOLE_USART == 1
#define CONSOLE_DEVICE "uart1"
#elif STM32_CONSOLE_USART == 2
#define CONSOLE_DEVICE "uart2"
#elif STM32_CONSOLE_USART == 3
#define CONSOLE_DEVICE "uart3"
#endif

#define RECV_REX_DEVICE "uart3"

/* USART1_REMAP = 0 */
#if 0==RT_USE_UART1_REMAP
#define UART1_GPIO_TX		GPIO_Pin_9
#define UART1_GPIO_RX		GPIO_Pin_10
#define UART1_GPIO		GPIOA
#define UART1_GPIO_RCC  	RCC_APB2Periph_GPIOA
#else
#define UART1_GPIO_TX		GPIO_Pin_6
#define UART1_GPIO_RX		GPIO_Pin_7
#define UART1_GPIO		GPIOB
#define UART1_GPIO_RCC  	RCC_APB2Periph_GPIOB
#endif
#define RCC_APBPeriph_UART1	RCC_APB2Periph_USART1
#define UART1_TX_DMA		DMA1_Channel4
#define UART1_RX_DMA		DMA1_Channel5

/* USART2_REMAP = 0 */
#if 0==RT_USE_UART2_REMAP
#define UART2_GPIO_TX		GPIO_Pin_2
#define UART2_GPIO_RX		GPIO_Pin_3
#define UART2_GPIO			GPIOA
#define UART2_GPIO_RCC  	RCC_APB2Periph_GPIOA
#else
#define UART2_GPIO_TX		GPIO_Pin_5
#define UART2_GPIO_RX		GPIO_Pin_6
#define UART2_GPIO		GPIOD
#define UART2_GPIO_RCC  	RCC_APB2Periph_GPIOD
#endif
#define RCC_APBPeriph_UART2	RCC_APB1Periph_USART2
#define UART2_TX_DMA		DMA1_Channel7
#define UART2_RX_DMA		DMA1_Channel6


/* USART3_REMAP[1:0] = 00 */
#define UART3_GPIO_RX		GPIO_Pin_11
#define UART3_GPIO_TX		GPIO_Pin_10
#define UART3_GPIO		GPIOB
#define UART3_GPIO_RCC  	RCC_APB2Periph_GPIOB
#define RCC_APBPeriph_UART3	RCC_APB1Periph_USART3
#define UART3_TX_DMA		DMA1_Channel2
#define UART3_RX_DMA		DMA1_Channel3


#define STM32_USE_FSMC_NOR_MUX2LCD 1
#define LCD_X_WIDTH 320
#define LCD_Y_HEIGHT 240

#define lcd_bl_en_rcc	RCC_APB2Periph_GPIOC
#define lcd_bl_en_gpio	GPIOC
#define lcd_bl_en_pin	(GPIO_Pin_9)

#define lcd_reset_rcc	RCC_APB2Periph_GPIOE
#define lcd_reset_gpio	GPIOE
#define lcd_reset_pin	(GPIO_Pin_2)

#if 1 /* LCD */
/* LCD Control pins */
#define LCD_Pin_WR      GPIO_Pin_14
#define LCD_PORT_WR     GPIOB
#define LCD_CLK_WR      RCC_APB2Periph_GPIOB

#define LCD_Pin_CS      GPIO_Pin_8
#define LCD_PORT_CS     GPIOC
#define LCD_CLK_CS      RCC_APB2Periph_GPIOC

#define LCD_Pin_RS      GPIO_Pin_13
#define LCD_PORT_RS     GPIOD
#define LCD_CLK_RS      RCC_APB2Periph_GPIOD

#define LCD_Pin_RD      GPIO_Pin_15
#define LCD_PORT_RD     GPIOD
#define LCD_CLK_RD      RCC_APB2Periph_GPIOD

#define SetCs  GPIO_SetBits(LCD_PORT_CS, LCD_Pin_CS);
#define ClrCs  GPIO_ResetBits(LCD_PORT_CS, LCD_Pin_CS);
 

#define SetWr  GPIO_SetBits(LCD_PORT_WR, LCD_Pin_WR);
#define ClrWr  GPIO_ResetBits(LCD_PORT_WR, LCD_Pin_WR);

#define SetRs  GPIO_SetBits(LCD_PORT_RS, LCD_Pin_RS);
#define ClrRs  GPIO_ResetBits(LCD_PORT_RS, LCD_Pin_RS);

#define SetRd  GPIO_SetBits(LCD_PORT_RD, LCD_Pin_RD);
#define ClrRd  GPIO_ResetBits(LCD_PORT_RD, LCD_Pin_RD);

#if 0
#define LCD_Write(LCD_DATA)  GPIO_Write(GPIOE, LCD_DATA)
#define LCD_Read()  GPIO_ReadInputData(GPIOE)
#else
#define LCD_Write(data)  do{GPIOE->ODR = data;}while(0)
#define LCD_Read()  	GPIOE->IDR
#endif
#endif

#if 1 /* touch */
#define TOUCH_USE_PHONY_SPI 0

#define TOUCH_CS_RCC 	RCC_APB2Periph_GPIOB
#define TOUCH_CS_PORT	GPIOB
#define TOUCH_CS_PIN	GPIO_Pin_12

#if 0==TOUCH_USE_PHONY_SPI
/*
 * RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO | RCC_APB2Periph_SPI1
 */
#define TOUCH_USE_SPIX	SPI2
#define TOUCH_SPI_RCC_PERIPH	RCC_APB1Periph_SPI2 | RCC_APB2Periph_GPIOB
#define TOUCH_SPI_PORT 	GPIOB
#define TOUCH_SPI_SCK	GPIO_Pin_13
#define TOUCH_SPI_MISO	GPIO_Pin_14
#define TOUCH_SPI_MOSI	GPIO_Pin_15
#define TOUCH_SPI_NSS	GPIO_Pin_12
#else
#define TOUCH_SPI_RCC_PERIPH	RCC_APB2Periph_GPIOC
#define TOUCH_SPI_PORT 	GPIOC
#define TOUCH_SPI_SCK	GPIO_Pin_10
#define TOUCH_SPI_MISO	GPIO_Pin_11
#define TOUCH_SPI_MOSI	GPIO_Pin_12
#endif


#define TOUCH_INT_RCC 	RCC_APB2Periph_GPIOC
#define TOUCH_INT_PORT	GPIOC
#define TOUCH_INT_PIN	GPIO_Pin_7

#define TOUCH_BUSY_PORT	GPIOC
#define TOUCH_BUSY_PIN	GPIO_Pin_8

#define TOUCH_EXTI_LINE                 EXTI_Line7
#define TOUCH_EXTI_PORT_SOURCE          GPIO_PortSourceGPIOC
#define TOUCH_EXTI_PIN_SOURCE           GPIO_PinSource7
#define TOUCH_EXTI_IRQn                 EXTI9_5_IRQn 


#endif

#if 0!=TOUCH_USE_PHONY_SPI
#define set_spip_sck()    set_port_pin(TOUCH_SPI_PORT, TOUCH_SPI_SCK)
#define clr_spip_sck()    clr_port_pin(TOUCH_SPI_PORT, TOUCH_SPI_SCK)

#define set_spip_mosi()   set_port_pin(TOUCH_SPI_PORT, TOUCH_SPI_MOSI)
#define clr_spip_mosi()   clr_port_pin(TOUCH_SPI_PORT, TOUCH_SPI_MOSI)

#define spip_miso_read()  (!!pin_in_is_set(TOUCH_SPI_PORT, TOUCH_SPI_MISO))
#endif



/*
 * SPI NAND Flash
 * SST25, stm32--SPI1
 *
 * SPI1
 * PA4 -- NSS
 * PA5 -- SCK
 * PA6 -- MISO
 * PA7 -- MOSI
 */
#define SST25_CS_PORT GPIOA
#define SST25_CS_PIN  GPIO_Pin_4

#define SST25_HOLD_PORT GPIOA
#define SST25_HOLD_PIN  GPIO_Pin_3

#define SST25_WP_PORT GPIOC
#define SST25_WP_PIN  GPIO_Pin_4

#define SST25_SPI_RCC_PERIPH	RCC_APB2Periph_SPI1
#define SST25_SPI_PORT 	GPIOA
#define SST25_SPI_SCK	GPIO_Pin_5
#define SST25_SPI_MISO	GPIO_Pin_6
#define SST25_SPI_MOSI	GPIO_Pin_7
#define SST25_SPI_NSS	GPIO_Pin_4



// led define
#ifdef STM32_SIMULATOR
#define led1_rcc                    RCC_APB2Periph_GPIOA
#define led1_gpio                   GPIOA
#define led1_pin                    (GPIO_Pin_5)

#define led2_rcc                    RCC_APB2Periph_GPIOA
#define led2_gpio                   GPIOA
#define led2_pin                    (GPIO_Pin_6)

#else

#define led1_rcc                    RCC_APB2Periph_GPIOC
#define led1_gpio                   GPIOC
#define led1_pin                    (GPIO_Pin_10)

#define led2_rcc                    RCC_APB2Periph_GPIOC
#define led2_gpio                   GPIOC
#define led2_pin                    (GPIO_Pin_11)

#define led3_rcc                    RCC_APB2Periph_GPIOC
#define led3_gpio                   GPIOC
#define led3_pin                    (GPIO_Pin_12)

#define led4_rcc                    RCC_APB2Periph_GPIOD
#define led4_gpio                   GPIOD
#define led4_pin                    (GPIO_Pin_2)
#endif // led define #ifdef STM32_SIMULATOR

#define buzzer_rcc                    RCC_APB2Periph_GPIOC
#define buzzer_gpio                   GPIOC
#define buzzer_pin                    (GPIO_Pin_5)


#ifndef NULL
#define NULL ((void *)0)
#endif


/*
 * operate i/o pin
 */
#define clr_port_pin(port, pin)          (port)->BRR  = (pin)
#define set_port_pin(port, pin)          (port)->BSRR = (pin)
#define rolling_over_port_pin(port, pin) (port)->ODR ^= (pin)
#define pin_in_is_set(port, pin)         ((port)->IDR & (pin))
#define pin_out_is_set(port, pin)        ((port)->ODR & (pin))

/* bit[31:16]--BRy(bit reset), bit[15:0]--BSy(bit set) */
#define gpio_bits_reset(port, bits) ((port)->BSRR = (bits << 16))
#define gpio_bits_set(port, bits)   ((port)->BSRR = bits)

#define buzzer_on(port, pin)	clr_port_pin(port, pin)
#define buzzer_off(port, pin)	set_port_pin(port, pin)
#define buzzer_rolling_over(port, pin) rolling_over_port_pin(port, pin)

/* led */
#define led_on(port, pin)     set_port_pin(port, pin)
#define led_off(port, pin)    clr_port_pin(port, pin)
#define is_led_off(port, pin) !pin_out_is_set(port, pin)
#define led_blink(port, pin)  rolling_over_port_pin(port, pin)

#define running_led_blink() led_blink(led4_gpio, led4_pin)
#define data_led_blink()    led_blink(led3_gpio, led3_pin)
#define data_led_off()    	led_off(led3_gpio, led3_pin)

/* 故障包括:发射端某相超限/缺相, 接收端某相超限, 切换至pt侧, 无光纤数据 */
#define fault_led_blink()    led_blink(led1_gpio, led1_pin)
#define fault_led_on()   led_on(led1_gpio, led1_pin)
#define fault_led_off()  led_off(led1_gpio, led1_pin)

/* lcd背光灯 */
#define lcd_bl_led_on()   set_port_pin(lcd_bl_en_gpio, lcd_bl_en_pin)
#define lcd_bl_led_off()  clr_port_pin(lcd_bl_en_gpio, lcd_bl_en_pin)


/*
 * usart
 */
#define disable_usartx_send_int(usartx)   do {(usartx)->CR1 &= ~(1<<7);} while (0)
#define enable_usartx_send_int(usartx)    do {(usartx)->CR1 |=  (1<<7);} while (0)
#define disable_usartx_recv_int(usartx)   do {(usartx)->CR1 &= ~(1<<5);} while (0)
#define enable_usartx_recv_int(usartx)    do {(usartx)->CR1 |=  (1<<5);} while (0)
/* 1 -- enable_usartx_send_int */
#define get_usartx_send_int_state(usartx) (!!((usartx)->CR1 & (1<<7)))

#define is_usartx_overrun_err(usartx)   ((usartx)->SR & (1<<3))
#define is_usartx_rxd_not_empty(usartx) ((usartx)->SR & (1<<5))
#define is_usartx_txd_empty(usartx)     ((usartx)->SR & (1<<7))

#define clear_usartx_rxne_flag(usartx)  do{((usartx)->SR &= ~(1<<5));}while(0)
#define clear_usartx_tc_flag(usartx)  do{((usartx)->SR &= ~(1<<6));}while(0)

#define get_usartx_data(usartx)    ((usartx)->DR & 0x01ff)
#define put_usartx_data(usartx, data)    do { (usartx)->DR = (data) & 0x01ff;} while (0)



void rt_hw_usart_init(void);

/* external memory dev init function */
void rt_hw_sdcard_init(void);
void rt_hw_msd_init(void);
extern void rt_hw_spiflash_init();

/* ETH interface init function */
void rt_hw_enc28j60_init(void);
void rt_hw_dm9000_init(void);

extern void rt_hw_board_init();
extern void nvic_cfg_app(void);
extern void reset_whole_system(void);


#if 1
extern uint8_t spix_send_byte(SPI_TypeDef* SPIx, unsigned char writedat);
extern uint8_t spix_rec_byte(SPI_TypeDef* SPIx, unsigned dummy);
extern void spix_send_data(SPI_TypeDef* SPIx, const uint8_t* data, uint32_t data_len);
extern void spix_rec_data(SPI_TypeDef* SPIx, uint8_t* buffer, uint32_t buffer_len, unsigned dummy);
#endif


#define RTC_SET_FLAG_BKP16BITS			BKP_DR1
#define POWEROFF_INFO_VALID_FLAG_BKP16BITS	BKP_DR12

#define TX1_POWEROFF_CNT_BKP16BITS	BKP_DR2
#define TX1_POWEROFF_0_BKP16BITS_H	BKP_DR3
#define TX1_POWEROFF_0_BKP16BITS_L	BKP_DR4
#define TX1_POWEROFF_1_BKP16BITS_H	BKP_DR5
#define TX1_POWEROFF_1_BKP16BITS_L	BKP_DR6
#define TX1_POWEROFF_2_BKP16BITS_H	BKP_DR7
#define TX1_POWEROFF_2_BKP16BITS_L	BKP_DR8

#define RX_POWEROFF_CNT_BKP16BITS	BKP_DR9
#define RX_POWEROFF_N_BKP16BITS_H	BKP_DR10
#define RX_POWEROFF_N_BKP16BITS_L	BKP_DR11

#define TX2_POWEROFF_CNT_BKP16BITS	BKP_DR13
#define TX2_POWEROFF_0_BKP16BITS_H	BKP_DR14
#define TX2_POWEROFF_0_BKP16BITS_L	BKP_DR15
#define TX2_POWEROFF_1_BKP16BITS_H	BKP_DR16
#define TX2_POWEROFF_1_BKP16BITS_L	BKP_DR17
#define TX2_POWEROFF_2_BKP16BITS_H	BKP_DR18
#define TX2_POWEROFF_2_BKP16BITS_L	BKP_DR19


#define POWEROFF_INFO_VALID_MAGIC 0xa530

extern void check_poweroff_info_and_save2flash(void);
extern unsigned long read_poweroff_time_from_bkp_r(unsigned hreg, unsigned lreg);

extern struct poweroff_info_st poweroff_info_data;

#endif

// <<< Use Configuration Wizard in Context Menu >>>

