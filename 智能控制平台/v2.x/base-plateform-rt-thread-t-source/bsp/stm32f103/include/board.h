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

#include <sys_app_cfg.h>
#include <rtdef.h>
#include <stm32f10x.h>


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

void rt_hw_board_init(void);

#if STM32_CONSOLE_USART == 0
#define CONSOLE_DEVICE "no"
#elif STM32_CONSOLE_USART == 1
#define CONSOLE_DEVICE "uart1"
#elif STM32_CONSOLE_USART == 2
#define CONSOLE_DEVICE "uart2"
#elif STM32_CONSOLE_USART == 3
#define CONSOLE_DEVICE "uart3"
#elif STM32_CONSOLE_USART == 4
#define CONSOLE_DEVICE "uart4"
#elif STM32_CONSOLE_USART == 5
#define CONSOLE_DEVICE "uart5"
#endif

/*
 * independent watchdog
 */
#define USE_STM32_IWDG 1

/*
 * window watchdog
 *
 * tWWDG = tPCLK1 * 4096 * 2^WDGTB * (t[5:0]+1)  (ms)
 *
 * PCLK1 -- 36MHz max
 * tWWDG = 1/36 * 10^-3 * 4096 * 2^3 * (t[5:0]+1)
 *       = 1/36 * 32768 * 10^-3 * (t[5:0]+1)
 *	 = 910.22 * 10^-3  * (t[5:0]+1) (ms) 
 *
 * if t[5:0]=0x3f then tWWDG = 58.25ms
 * reset system when t[6] change from 1 to 0
 */
#define USE_STM32_WWDG 1
#define WWDG_RELOAD_VALUE (0X7f)

/*
 * 
 */
#define STM32_USE_FSMC_NOR_MUX 1

/*
 * I/O pin map
 */
#define NEED_CTRL_LED 1
#if NEED_CTRL_LED
#define LED_PORTX   GPIOC
#define LED1_PIN    GPIO_Pin_6
#define LED2_PIN    GPIO_Pin_7
#define LED3_PIN    GPIO_Pin_8

#define EPON_STATE_LED	LED1_PIN
#define FAULT_LED	LED2_PIN
#define WARNING_LED	LED3_PIN
#endif

#define GPIO_PIN_SCL    GPIO_Pin_6
#define GPIO_PIN_SDA    GPIO_Pin_7
#define GPIO_PORT_I2C	GPIOB

#define TCA8418_INT_RCC 	RCC_APB2Periph_GPIOB
#define TCA8418_INT_PORT 	GPIOB
#define TCA8418_INT_PIN  	GPIO_Pin_8
#define TCA8418_INT_PORT_SOURCE GPIO_PortSourceGPIOB
#define TCA8418_INT_PIN_SOURCE 	GPIO_PinSource8
#define TCA8418_INT_LINE 	EXTI_Line8
#define TCA8418_INT_NUM		EXTI9_5_IRQn

#define ELOCK_RCC  RCC_APB2Periph_GPIOB
#define ELOCK_PORT GPIOB
#define ELOCK_PIN  GPIO_Pin_8

#define ELOCK_SWITCH_PORT GPIOB
#define ELOCK_SWITCH_PIN  GPIO_Pin_8


#define BEEP_PORT GPIOB
#define BEEP_PIN  GPIO_Pin_8


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

/* temperature */
#define TMP_Pin      GPIO_Pin_3
#define TMP_PORT     GPIOD
#define TMP_CLK      RCC_APB2Periph_GPIOD

/*
 * ENC28J60, stm32--SPI2
 */
#define ENC28J60_CS_PORT GPIOB
#define ENC28J60_CS_PIN  GPIO_Pin_12

#define ENC28J60_SPI_RCC_PERIPH	RCC_APB1Periph_SPI2
#define ENC28J60_SPI_PORT 	GPIOB
#define ENC28J60_SPI_SCK	GPIO_Pin_13
#define ENC28J60_SPI_MISO	GPIO_Pin_14
#define ENC28J60_SPI_MOSI	GPIO_Pin_15
#define ENC28J60_SPI_NSS	GPIO_Pin_12

#define ENC28J60_RST_PORT	GPIOB
#define ENC28J60_RST_PIN	GPIO_Pin_11

#if 1==USE_TO_9INCH_LCD
/* PB10, 9´çÆÁ ¸¨°å */
#define ENC28J60_INT_PORT	GPIOB
#define ENC28J60_INT_PIN	GPIO_Pin_10
#define ENC28J60_INT_NUM 	EXTI15_10_IRQn
#define ENC28J60_INT_PORT_SOURCE	GPIO_PortSourceGPIOB
#define ENC28J60_INT_PIN_SOURCE		GPIO_PinSource10
#define ENC28J60_INT_EXT_LINE		EXTI_Line10
#elif 1==USE_TO_7INCH_LCD
/* PC6, 7´çÆÁ Ö÷°å */
#define ENC28J60_INT_PORT		GPIOC
#define ENC28J60_INT_PIN		GPIO_Pin_6
#define ENC28J60_INT_NUM 		EXTI9_5_IRQn
#define ENC28J60_INT_PORT_SOURCE	GPIO_PortSourceGPIOC
#define ENC28J60_INT_PIN_SOURCE		GPIO_PinSource6
#define ENC28J60_INT_EXT_LINE		EXTI_Line6
#endif


/*
 * RA8875 /INT
 */
#define RA8875_INT_PORT		GPIOD
#define RA8875_INT_PIN		GPIO_Pin_12
#define RA8875_INT_NUM 		EXTI15_10_IRQn
#define RA8875_INT_PORT_SOURCE	GPIO_PortSourceGPIOD
#define RA8875_INT_PIN_SOURCE		GPIO_PinSource12
#define RA8875_INT_EXT_LINE		EXTI_Line12


/*
 * SPI NAND Flash
 * 
 * SPI1
 * PA4 -- NSS
 * PA5 -- SCK
 * PA6 -- MISO
 * PA7 -- MOSI
 */
#define SPIFLASH_SPI_RCC_PERIPH	RCC_APB2Periph_SPI1
#define SPIFLASH_SPI_PORT 	GPIOA
#define SPIFLASH_SPI_SCK	GPIO_Pin_5
#define SPIFLASH_SPI_MISO	GPIO_Pin_6
#define SPIFLASH_SPI_MOSI	GPIO_Pin_7
#define SPIFLASH_SPI_NSS	GPIO_Pin_4

/* PD.2 EM7164SU16.ZZ */
#define PSRAM_EM7164_ZZ_PORT 	GPIOD
#define PSRAM_EM7164_ZZ_PIN 	GPIO_Pin_2

/* PSRAM mem size */
#define PSRAM_START_ADDR	((unsigned char *)0X60000000UL)
#define PSRAM_MAX_LEN		(2 * 1024 * 1024UL)


/*
 * operate i/o pin
 */
#define clr_port_pin(port, pin)  (port)->BRR  = (pin)
#define set_port_pin(port, pin)  (port)->BSRR = (pin)
#define pin_in_is_set(port, pin)    ((port)->IDR & (pin))
#define pin_out_is_set(port, pin)    ((port)->ODR & (pin))

/* bit[31:16]--BRy(bit reset), bit[15:0]--BSy(bit set) */
#define gpio_bits_reset(port, bits) ((port)->BSRR = (bits << 16))
#define gpio_bits_set(port, bits)   ((port)->BSRR = bits)

#define beep_on  clr_port_pin
#define beep_off set_port_pin
#if 0
#define elock_open  set_port_pin
#define elock_close clr_port_pin
#else
#define elock_open(dummy)   set_port_pin(ELOCK_PORT, ELOCK_PIN)
#define elock_close(dummy)  clr_port_pin(ELOCK_PORT, ELOCK_PIN)
#endif
#define is_elock_open pin_out_is_set
#define is_door_open  pin_in_is_set

#define led_on  set_port_pin
#define led_off clr_port_pin


//#define MII_MODE          /* MII mode for STM3210C-EVAL Board (MB784) (check jumpers setting) */
#define RMII_MODE       /* RMII mode for STM3210C-EVAL Board (MB784) (check jumpers setting) */


/*
 * Function prototypes
 */

void rt_hw_usart_init(void);
void rt_hw_msd_init(void); /* SD Card init function */
extern void rt_hw_board_init();
extern void nvic_cfg_app(void);
extern void reset_whole_system(void);

#if 1
extern uint8_t spix_send_byte(SPI_TypeDef* SPIx, unsigned char writedat);
extern uint8_t spix_rec_byte(SPI_TypeDef* SPIx);
extern void spix_send_data(SPI_TypeDef* SPIx, const uint8_t* data, uint16_t data_len);
extern void spix_rec_data(SPI_TypeDef* SPIx, uint8_t* buffer, uint16_t buffer_len);
#endif

#endif

// <<< Use Configuration Wizard in Context Menu >>>
