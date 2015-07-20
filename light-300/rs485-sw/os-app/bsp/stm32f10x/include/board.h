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

#ifndef __BOARD_H__
#define __BOARD_H__

#include <rtdef.h>
#include <stm32f10x.h>
#include <stm32f10x_bkp.h>
#include <syscfgdata.h>
#include <stm32_cpu_comm.h>
#include <app-hw-resource.h>

#define USE_STM32_IWDG 0

/* board configuration */
// <o> SDCard Driver <1=>SDIO sdcard <0=>SPI MMC card
// 	<i>Default: 1
#define STM32_USE_SDIO			0

/* whether use board external SRAM memory */
// <e>Use external SRAM memory on the board
// 	<i>Enable External SRAM memory
#if RT_USING_EXT_SRAM
#define STM32_EXT_SRAM          1
#else
#define STM32_EXT_SRAM          0
#endif
/*
 *  Bank 1(0x6000 0000 -- 0x6fff ffff) used to address up to 4 NOR Flash or PSRAM memory devices.
 *  This bank is  split into 4 NOR/PSRAM regions with 4 dedicated Chip Select.
 * region 0 - [0x6000 0000, 0x6400 0000)
 * region 1 - [0x6400 0000, 0x6800 0000)
 * region 2 - [0x6800 0000, 0x6c00 0000)
 * region 3 - [0x6c00 0000, 0x7000 0000)
 *
 *  For the LQFP100 and BGA100 packages, only FSMC Bank1 and Bank2 are available. Bank1 can only
 * support a multiplexed NOR/PSRAM memory using the NE1 Chip Select. Bank2 can only support a 16- or
 * 8-bit NAND Flash memory using the NCE2 Chip Select. The interrupt line cannot be used since Port G is
 * not available in this package.
 *
 * IS61WV25616 -- 256K*16bit = 4 Mbits = 512 KiB (512K == 0x80000)
 */
//	<o>Begin Address of External SRAM
//		<i>Default: 0x68000000
#define STM32_EXT_SRAM_BEGIN    0x60000000 /* the begining address of external SRAM */
//	<o>End Address of External SRAM
//		<i>Default: 0x68080000
#define STM32_EXT_SRAM_END      0x60080000 /* the end address of external SRAM */

#define STM32_EXT_SRAM_START_ADDR	((unsigned char *)0X60000000UL)
#define STM32_EXT_SRAM_MAX_LEN		(512 * 1024UL)


// </e>

// <o> Internal SRAM memory size[Kbytes] <8-64>
//	<i>Default: 64
#if defined(STM32F10X_HD)
#define STM32_SRAM_SIZE         64
#elif defined(STM32F10X_XL)
#define STM32_SRAM_SIZE         96
#endif
#define STM32_SRAM_END          (0x20000000 + STM32_SRAM_SIZE * 1024)

// <o> Console on USART: <0=> no console <1=>USART 1 <2=>USART 2 <3=> USART 3
// 	<i>Default: 1
#define STM32_CONSOLE_USART		1

// <o> Ethernet Interface: <0=> Microchip ENC28J60 <1=> Davicom DM9000A
// 	<i>Default: 0
#define STM32_ETH_IF			0

void rt_hw_board_init(void);


/*
 **************************************************************************************************
 * usart cfg
 **************************************************************************************************
 */
#if STM32_CONSOLE_USART == 0
#define CONSOLE_DEVICE "no"
#elif STM32_CONSOLE_USART == 1
#define CONSOLE_DEVICE "uart1"
#elif STM32_CONSOLE_USART == 2
#define CONSOLE_DEVICE "uart2"
#elif STM32_CONSOLE_USART == 3
#define CONSOLE_DEVICE "uart3"
#endif


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

/* uart4 */
#define UART4_GPIO_RX		GPIO_Pin_11
#define UART4_GPIO_TX		GPIO_Pin_10
#define UART4_GPIO		GPIOC
#define UART4_GPIO_RCC  	RCC_APB2Periph_GPIOC
#define RCC_APBPeriph_UAR4	RCC_APB1Periph_UART4
#define UART4_TX_DMA		DMA2_Channel5
#define UART4_RX_DMA		DMA2_Channel3

/* uart5, pc12-tx, pd2-rx */
#define UART5_GPIO_RX		GPIO_Pin_2
#define UART5_GPIO_TX		GPIO_Pin_12
#define UART5_GPIO_RX_PORT	GPIOD
#define UART5_GPIO_TX_PORT	GPIOC
#define UART5_GPIO_RCC  	(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD)
#define RCC_APBPeriph_UAR5	RCC_APB1Periph_UART5

/*
 * 485
 *
 * case LT_485_EMC_UART:  ptr = UART_485_2_DEV_PTR;
 * case LT_485_EM_UART_1: ptr = UART_485_3_DEV_PTR;
 * case LT_485_EM_UART_2: ptr = UART_485_1_DEV_PTR;
 *
 * [ttl-serial]    [jlink(swd)]    [485-AB,rx-pc11,tx-pc10]        [485AB, rx-pd2,tx-pc12]         [tl16_485AB]    [485AB, rx-pa3,tx-pa2]  [电源, G N L]
 *
 * #define UART2_GPIO_TX	GPIO_Pin_2
 * #define UART2_GPIO_RX	GPIO_Pin_3
 * #define UART2_GPIO		GPIOA
 *
 * #define UART4_GPIO_RX	GPIO_Pin_11
 * #define UART4_GPIO_TX	GPIO_Pin_10
 * #define UART4_GPIO		GPIOC
 *
 * #define UART5_GPIO_RX	GPIO_Pin_2
 * #define UART5_GPIO_TX	GPIO_Pin_12
 * #define UART5_GPIO_RX_PORT	GPIOD
 * #define UART5_GPIO_TX_PORT	GPIOC
 *
 * [485-AB,rx-pc11,tx-pc10]     [485AB, rx-pd2,tx-pc12]         [tl16_485AB]    [485AB, rx-pa3,tx-pa2]
 * [emc, uart4(pc10,pc11)]  [em-1, uart5(pd2,pc12)]  [em-2, uart2(pa3,pa2)]
 */
#define UART_485_1_DEV "uart2"
#define UART_485_2_DEV "uart4"
#define UART_485_3_DEV "uart5"

#define UART_485_TL16_1_DEV "tl16-u1"
#define UART_485_TL16_2_DEV "tl16-u2"
#define UART_485_TL16_3_DEV "tl16-u3"
#define UART_485_TL16_4_DEV "tl16-u4"
#define UART_485_TL16_5_DEV "tl16-u5"
#define UART_485_TL16_6_DEV "tl16-u6"
#define UART_485_TL16_7_DEV "tl16-u7"
#define UART_485_TL16_8_DEV "tl16-u8"


#define UART_485_1_DEV_PTR USART2
#define UART_485_2_DEV_PTR UART4
#define UART_485_3_DEV_PTR UART5

#define driver_oen_485_1_rcc	RCC_APB2Periph_GPIOC
#define driver_oen_485_1_gpio	GPIOC
#define driver_oen_485_1_pin	(GPIO_Pin_0)

#define driver_oen_485_2_rcc	RCC_APB2Periph_GPIOC
#define driver_oen_485_2_gpio	GPIOC
#define driver_oen_485_2_pin	(GPIO_Pin_2)

#define driver_oen_485_3_rcc	RCC_APB2Periph_GPIOC
#define driver_oen_485_3_gpio	GPIOC
#define driver_oen_485_3_pin	(GPIO_Pin_1)


/* 1--driver output enable, 0--receiver output enable, 光偶已反转电平 */
#if 0
/* 光偶 */
#define tx_en_rev_disable_485_1() clr_port_pin(driver_oen_485_1_gpio, driver_oen_485_1_pin)
#define tx_disable_rev_en_485_1() set_port_pin(driver_oen_485_1_gpio, driver_oen_485_1_pin)

#define tx_en_rev_disable_485_2() clr_port_pin(driver_oen_485_2_gpio, driver_oen_485_2_pin)
#define tx_disable_rev_en_485_2() set_port_pin(driver_oen_485_2_gpio, driver_oen_485_2_pin)

#define tx_en_rev_disable_485_3() clr_port_pin(driver_oen_485_3_gpio, driver_oen_485_3_pin)
#define tx_disable_rev_en_485_3() set_port_pin(driver_oen_485_3_gpio, driver_oen_485_3_pin)
#else
/* 直连 */
#define tx_en_rev_disable_485_1() set_port_pin(driver_oen_485_1_gpio, driver_oen_485_1_pin)
#define tx_disable_rev_en_485_1() clr_port_pin(driver_oen_485_1_gpio, driver_oen_485_1_pin)

#define tx_en_rev_disable_485_2() set_port_pin(driver_oen_485_2_gpio, driver_oen_485_2_pin)
#define tx_disable_rev_en_485_2() clr_port_pin(driver_oen_485_2_gpio, driver_oen_485_2_pin)

#define tx_en_rev_disable_485_3() set_port_pin(driver_oen_485_3_gpio, driver_oen_485_3_pin)
#define tx_disable_rev_en_485_3() clr_port_pin(driver_oen_485_3_gpio, driver_oen_485_3_pin)

#endif

#define RS485_PORT_USED_BY_645			AMMETER_UART1
#define RS485_PORT_USED_BY_WIRELESS		UART_485_2_DEV_PTR
#define RS485_DEV_USED_BY_WIRELESS		UART_485_2_DEV
#define RS485_RX_IND_USED_BY_WIRELESS		uart485_2_rx_ind
#define RS485_RX_SEM_USED_BY_WIRELESS		uart485_2_rx_byte_sem


/*
 **************************************************************************************************
 * led/lcd cfg
 **************************************************************************************************
 */
#if EM_MULTI_BASE
#define led1_rcc         RCC_APB2Periph_GPIOB
#define led1_gpio        GPIOB
#define led1_pin         GPIO_Pin_2

#define led2_rcc         RCC_APB2Periph_GPIOD
#define led2_gpio        GPIOD
#define led2_pin         GPIO_Pin_13

#define s2p_74hc164_data_rcc         RCC_APB2Periph_GPIOC
#define s2p_74hc164_data_gpio        GPIOC
#define s2p_74hc164_data_pin         GPIO_Pin_5

#define s2p_74hc164_clk_rcc         RCC_APB2Periph_GPIOF
#define s2p_74hc164_clk_gpio        GPIOF
#define s2p_74hc164_clk_pin         GPIO_Pin_11

#else
#define led1_rcc         RCC_APB2Periph_GPIOF
#define led1_gpio        GPIOF
#define led1_pin         GPIO_Pin_5

#define led2_rcc         RCC_APB2Periph_GPIOF
#define led2_gpio        GPIOF
#define led2_pin         GPIO_Pin_4

#define led3_rcc         RCC_APB2Periph_GPIOF
#define led3_gpio        GPIOF
#define led3_pin         GPIO_Pin_3

#define led4_rcc         RCC_APB2Periph_GPIOF
#define led4_gpio        GPIOF
#define led4_pin         GPIO_Pin_8

#define led5_rcc         RCC_APB2Periph_GPIOF
#define led5_gpio        GPIOF
#define led5_pin         GPIO_Pin_9
#endif

#if 0
#define buzzer_rcc       RCC_APB2Periph_GPIOC
#define buzzer_gpio      GPIOC
#define buzzer_pin       GPIO_Pin_5
#endif

#define is_led_off(port, pin)	!pin_out_is_set(port, pin)

#define led_on(port, pin)     	clr_port_pin(port, pin)
#define led_off(port, pin)    	set_port_pin(port, pin)
#define led_blink(port, pin)	rolling_over_port_pin(port, pin)

#define led_on_164(vector, bit)     	set_bit(vector, bit)
#define led_off_164(vector, bit)    	clr_bit(vector, bit)
#define led_blink_164(vector, bit)	reverse_bit(vector, bit)

#define running_led_blink() 	led_blink(led1_gpio, led1_pin)

#define led_port_sys_fault	led2_gpio
#define led_pin_sys_fault	led2_pin

#if !EM_MULTI_BASE
#define led_port_lose_pa	led3_gpio
#define led_pin_lose_pa		led3_pin

#define led_port_lose_pb	led4_gpio
#define led_pin_lose_pb		led4_pin

#define led_port_lose_pc	led5_gpio
#define led_pin_lose_pc		led5_pin
#endif

/*
 * lcd cfg
 */
#define STM32_USE_FSMC_NOR_MUX2LCD 0
#define LCD_X_WIDTH 320
#define LCD_Y_HEIGHT 240

#define lcd_bl_en_rcc	RCC_APB2Periph_GPIOC
#define lcd_bl_en_gpio	GPIOC
#define lcd_bl_en_pin	(GPIO_Pin_9)

#define lcd_reset_rcc	RCC_APB2Periph_GPIOE
#define lcd_reset_gpio	GPIOE
#define lcd_reset_pin	(GPIO_Pin_2)



/*
 **************************************************************************************************
 * other
 **************************************************************************************************
 */

#ifndef NULL
#define NULL ((void *)0)
#endif


#define RTC_SET_FLAG_BKP16BITS 			BKP_DR1
#define POWEROFF_INFO_VALID_FLAG_BKP16BITS 	BKP_DR12

#define TX_POWEROFF_CNT_BKP16BITS 	BKP_DR2
#define TX_POWEROFF_0_BKP16BITS_H 	BKP_DR3
#define TX_POWEROFF_0_BKP16BITS_L 	BKP_DR4
#define TX_POWEROFF_1_BKP16BITS_H 	BKP_DR5
#define TX_POWEROFF_1_BKP16BITS_L 	BKP_DR6
#define TX_POWEROFF_2_BKP16BITS_H 	BKP_DR7
#define TX_POWEROFF_2_BKP16BITS_L 	BKP_DR8

#define RX_POWEROFF_CNT_BKP16BITS 	BKP_DR9
#define RX_POWEROFF_N_BKP16BITS_H 	BKP_DR10
#define RX_POWEROFF_N_BKP16BITS_L 	BKP_DR11


#define EVENT_BIT_NEED_RUN_TL16_1_IRQ	(1 << 0)
#define EVENT_BIT_NEED_RUN_TL16_2_IRQ	(1 << 1)
extern struct rt_event isr_event_set;
extern volatile unsigned isr_event_var_flag;

void rt_hw_usart_init(void);

/* external memory dev init function */
void rt_hw_sdcard_init(void);
void rt_hw_msd_init(void);

/* ETH interface init function */
void rt_hw_enc28j60_init(void);
void rt_hw_dm9000_init(void);

extern void rt_hw_board_init();
extern void nvic_cfg_app(void);
extern void reset_whole_system(void);

#if EM_MULTI_BASE
enum port_type_use164led_e {
	PTU164_V,
	PTU164_I,

	PTU164_BUTT
};

/*
 * 目前用4片164，外扩32个I/O口
 * */
#define led_on_164_vector(bit)     	led_on_164(serial_to_parallel_bit_vector, bit)
#define led_off_164_vector(bit)    	led_off_164(serial_to_parallel_bit_vector, bit)
#define led_blink_164_vector(bit)	set_bit(serial_to_parallel_bit_vector_blink, bit)
#define led_no_blink_164_vector(bit)	clr_bit(serial_to_parallel_bit_vector_blink, bit)


#define VOLTAGE1_RUN_MASK_BIT	(1UL<<0)
#define VOLTAGE1_FAULT_MASK_BIT	(1UL<<1)
#define VOLTAGE2_RUN_MASK_BIT	(1UL<<2)
#define VOLTAGE2_FAULT_MASK_BIT	(1UL<<3)
#define VOLTAGE3_RUN_MASK_BIT	(1UL<<4)
#define VOLTAGE3_FAULT_MASK_BIT	(1UL<<5)
#define VOLTAGE4_RUN_MASK_BIT	(1UL<<6)
#define VOLTAGE4_FAULT_MASK_BIT	(1UL<<7)

#define CURRENT1_RUN_MASK_BIT		(1UL<<8 )
#define CURRENT1_FAULT_MASK_BIT		(1UL<<9 )
#define CURRENT2_RUN_MASK_BIT		(1UL<<10)
#define CURRENT2_FAULT_MASK_BIT		(1UL<<11)
#define CURRENT3_RUN_MASK_BIT		(1UL<<12)
#define CURRENT3_FAULT_MASK_BIT		(1UL<<13)
#define CURRENT4_RUN_MASK_BIT		(1UL<<14)
#define CURRENT4_FAULT_MASK_BIT		(1UL<<15)
#define CURRENT5_RUN_MASK_BIT		(1UL<<16)
#define CURRENT5_FAULT_MASK_BIT		(1UL<<17)
#define CURRENT6_RUN_MASK_BIT		(1UL<<18)
#define CURRENT6_FAULT_MASK_BIT		(1UL<<19)
#define CURRENT7_RUN_MASK_BIT		(1UL<<20)
#define CURRENT7_FAULT_MASK_BIT		(1UL<<21)
#define CURRENT8_RUN_MASK_BIT		(1UL<<22)
#define CURRENT8_FAULT_MASK_BIT		(1UL<<23)
#define CURRENT9_RUN_MASK_BIT		(1UL<<24)
#define CURRENT9_FAULT_MASK_BIT		(1UL<<25)
#define CURRENT10_RUN_MASK_BIT		(1UL<<26)
#define CURRENT10_FAULT_MASK_BIT	(1UL<<27)
#define CURRENT11_RUN_MASK_BIT		(1UL<<28)
#define CURRENT11_FAULT_MASK_BIT	(1UL<<29)
#define CURRENT12_RUN_MASK_BIT		(1UL<<30)
#define CURRENT12_FAULT_MASK_BIT	(1UL<<31)


#define decoder_3to8_a0_rcc	RCC_APB2Periph_GPIOG
#define decoder_3to8_a0_gpio	GPIOG
#define decoder_3to8_a0_pin	(GPIO_Pin_9)

#define decoder_3to8_a1_rcc	RCC_APB2Periph_GPIOG
#define decoder_3to8_a1_gpio	GPIOG
#define decoder_3to8_a1_pin	(GPIO_Pin_10)

#define decoder_3to8_a2_rcc	RCC_APB2Periph_GPIOG
#define decoder_3to8_a2_gpio	GPIOG
#define decoder_3to8_a2_pin	(GPIO_Pin_11)

#define decoder_4t16_a0_rcc	RCC_APB2Periph_GPIOE
#define decoder_4t16_a0_gpio	GPIOE
#define decoder_4t16_a0_pin	(GPIO_Pin_2)

#define decoder_4t16_a1_rcc	RCC_APB2Periph_GPIOE
#define decoder_4t16_a1_gpio	GPIOE
#define decoder_4t16_a1_pin	(GPIO_Pin_3)

#define decoder_4t16_a2_rcc	RCC_APB2Periph_GPIOE
#define decoder_4t16_a2_gpio	GPIOE
#define decoder_4t16_a2_pin	(GPIO_Pin_4)

#define decoder_4t16_a3_rcc	RCC_APB2Periph_GPIOE
#define decoder_4t16_a3_gpio	GPIOE
#define decoder_4t16_a3_pin	(GPIO_Pin_5)

#define DECODER_3TO8_PIN_OFFSET		9
#define DECODER_4TO16_PIN_OFFSET	2

/* ade7880 量程切换控制引脚 */
#define ade7880_range_switch_rcc	RCC_APB2Periph_GPIOB
#define ade7880_range_switch_gpio	GPIOB
#define ade7880_range_switch_pin	(GPIO_Pin_7)

#define set_7880_to_lagre_scale()	set_port_pin(ade7880_range_switch_gpio, ade7880_range_switch_pin)
#define set_7880_to_small_scale()	clr_port_pin(ade7880_range_switch_gpio, ade7880_range_switch_pin)

/* 12路双母线指示信号 */
#define double_mbus_indication_1_rcc	RCC_APB2Periph_GPIOE
#define double_mbus_indication_1_gpio	GPIOE
#define double_mbus_indication_1_pin	(GPIO_Pin_6)

#define double_mbus_indication_2_rcc	RCC_APB2Periph_GPIOG
#define double_mbus_indication_2_gpio	GPIOG
#define double_mbus_indication_2_pin	(GPIO_Pin_15)

#define double_mbus_indication_3_rcc	RCC_APB2Periph_GPIOG
#define double_mbus_indication_3_gpio	GPIOG
#define double_mbus_indication_3_pin	(GPIO_Pin_14)

#define double_mbus_indication_4_rcc	RCC_APB2Periph_GPIOG
#define double_mbus_indication_4_gpio	GPIOG
#define double_mbus_indication_4_pin	(GPIO_Pin_13)

#define double_mbus_indication_5_rcc	RCC_APB2Periph_GPIOG
#define double_mbus_indication_5_gpio	GPIOG
#define double_mbus_indication_5_pin	(GPIO_Pin_12)

#define double_mbus_indication_6_rcc	RCC_APB2Periph_GPIOD
#define double_mbus_indication_6_gpio	GPIOD
#define double_mbus_indication_6_pin	(GPIO_Pin_6)

#define double_mbus_indication_7_rcc	RCC_APB2Periph_GPIOF
#define double_mbus_indication_7_gpio	GPIOF
#define double_mbus_indication_7_pin	(GPIO_Pin_6)

#define double_mbus_indication_8_rcc	RCC_APB2Periph_GPIOF
#define double_mbus_indication_8_gpio	GPIOF
#define double_mbus_indication_8_pin	(GPIO_Pin_7)

#define double_mbus_indication_9_rcc	RCC_APB2Periph_GPIOF
#define double_mbus_indication_9_gpio	GPIOF
#define double_mbus_indication_9_pin	(GPIO_Pin_8)

#define double_mbus_indication_10_rcc	RCC_APB2Periph_GPIOG
#define double_mbus_indication_10_gpio	GPIOG
#define double_mbus_indication_10_pin	(GPIO_Pin_7)

#define double_mbus_indication_11_rcc	RCC_APB2Periph_GPIOG
#define double_mbus_indication_11_gpio	GPIOG
#define double_mbus_indication_11_pin	(GPIO_Pin_8)

#define double_mbus_indication_12_rcc	RCC_APB2Periph_GPIOF
#define double_mbus_indication_12_gpio	GPIOF
#define double_mbus_indication_12_pin	(GPIO_Pin_9)

extern unsigned long serial_to_parallel_bit_vector;
extern unsigned long serial_to_parallel_bit_vector_prev;
extern unsigned long serial_to_parallel_bit_vector_blink;

extern void write_32bit_to_74hc164(unsigned long data);
extern void set_decoder_3to8_data(unsigned data);
extern void set_decoder_4to16_data(unsigned data);

extern unsigned long get_run164led_bit_mask(enum port_type_use164led_e vi_type, int port_no);
extern unsigned long get_fault164led_bit_mask(enum port_type_use164led_e vi_type, int port_no);
#endif /* EM_MULTI_BASE */

#if RT_USING_SI4432_MAC
void shutdown_si443x_or_not(int shutdown);
#endif

#if RT_USING_TL16C554

/* ---- tl16c554 ---- */
#define tl16c554_1_csa_gpio	GPIOB
#define tl16c554_1_csa_pin	GPIO_Pin_12
#define tl16c554_1_csb_gpio	GPIOB
#define tl16c554_1_csb_pin	GPIO_Pin_13
#define tl16c554_1_csc_gpio	GPIOB
#define tl16c554_1_csc_pin	GPIO_Pin_14
#define tl16c554_1_csd_gpio	GPIOB
#define tl16c554_1_csd_pin	GPIO_Pin_15

#define tl16c554_1_reset_gpio	GPIOF
#define tl16c554_1_reset_pin	GPIO_Pin_7

/* -------- */
#define tl16c554_2_csa_gpio	GPIOC
#define tl16c554_2_csa_pin	GPIO_Pin_5
#define tl16c554_2_csb_gpio	GPIOB
#define tl16c554_2_csb_pin	GPIO_Pin_0
#define tl16c554_2_csc_gpio	GPIOB
#define tl16c554_2_csc_pin	GPIO_Pin_1
#define tl16c554_2_csd_gpio	GPIOB
#define tl16c554_2_csd_pin	GPIO_Pin_2

#define tl16c554_2_reset_gpio	GPIOF
#define tl16c554_2_reset_pin	GPIO_Pin_1


/* ---- tl16c554 rs485 ---- */
#define tl16c554_x_chx_rx_en_gpio	GPIOC
#define tl16c554_x_chx_rx_en_pin	GPIO_Pin_3

#define tl16c554_1_cha_tx_en_gpio	GPIOE
#define tl16c554_1_cha_tx_en_pin	GPIO_Pin_0
#define tl16c554_1_chb_tx_en_gpio	GPIOB
#define tl16c554_1_chb_tx_en_pin	GPIO_Pin_9
#define tl16c554_1_chc_tx_en_gpio	GPIOB
#define tl16c554_1_chc_tx_en_pin	GPIO_Pin_8
#define tl16c554_1_chd_tx_en_gpio	GPIOB
#define tl16c554_1_chd_tx_en_pin	GPIO_Pin_7

#define tl16c554_2_cha_tx_en_gpio	GPIOE
#define tl16c554_2_cha_tx_en_pin	GPIO_Pin_5
#define tl16c554_2_chb_tx_en_gpio	GPIOB
#define tl16c554_2_chb_tx_en_pin	GPIO_Pin_3
#define tl16c554_2_chc_tx_en_gpio	GPIOG
#define tl16c554_2_chc_tx_en_pin	GPIO_Pin_12
#define tl16c554_2_chd_tx_en_gpio	GPIOG
#define tl16c554_2_chd_tx_en_pin	GPIO_Pin_11

/* -- cs, reset, tx/rx-en op -- */
#define tl16c554_cs_act(port, pin)	clr_port_pin(port, pin)
#define tl16c554_cs_deact(port, pin)	set_port_pin(port, pin)

#define tl16c554_reset_act(port, pin)	set_port_pin(port, pin)
#define tl16c554_reset_deact(port, pin)	clr_port_pin(port, pin)

#define tl16c554_485rx_enable(port, pin)	set_port_pin(port, pin)
#define tl16c554_485rx_disable(port, pin)	clr_port_pin(port, pin)

#define tl16c554_485tx_enable(port, pin)	set_port_pin(port, pin)
#define tl16c554_485tx_disable(port, pin)	clr_port_pin(port, pin)

/* -- cs, reset -- */
#define tl16c554_1_csa_act()	tl16c554_cs_act(tl16c554_1_csa_gpio, tl16c554_1_csa_pin)
#define tl16c554_1_csa_deact()	tl16c554_cs_deact(tl16c554_1_csa_gpio, tl16c554_1_csa_pin)
#define tl16c554_1_csb_act()	tl16c554_cs_act(tl16c554_1_csb_gpio, tl16c554_1_csb_pin)
#define tl16c554_1_csb_deact()	tl16c554_cs_deact(tl16c554_1_csb_gpio, tl16c554_1_csb_pin)
#define tl16c554_1_csc_act()	tl16c554_cs_act(tl16c554_1_csc_gpio, tl16c554_1_csc_pin)
#define tl16c554_1_csc_deact()	tl16c554_cs_deact(tl16c554_1_csc_gpio, tl16c554_1_csc_pin)
#define tl16c554_1_csd_act()	tl16c554_cs_act(tl16c554_1_csd_gpio, tl16c554_1_csd_pin)
#define tl16c554_1_csd_deact()	tl16c554_cs_deact(tl16c554_1_csd_gpio, tl16c554_1_csd_pin)

#define tl16c554_2_csa_act()	tl16c554_cs_act(tl16c554_2_csa_gpio, tl16c554_2_csa_pin)
#define tl16c554_2_csa_deact()	tl16c554_cs_deact(tl16c554_2_csa_gpio, tl16c554_2_csa_pin)
#define tl16c554_2_csb_act()	tl16c554_cs_act(tl16c554_2_csb_gpio, tl16c554_2_csb_pin)
#define tl16c554_2_csb_deact()	tl16c554_cs_deact(tl16c554_2_csb_gpio, tl16c554_2_csb_pin)
#define tl16c554_2_csc_act()	tl16c554_cs_act(tl16c554_2_csc_gpio, tl16c554_2_csc_pin)
#define tl16c554_2_csc_deact()	tl16c554_cs_deact(tl16c554_2_csc_gpio, tl16c554_2_csc_pin)
#define tl16c554_2_csd_act()	tl16c554_cs_act(tl16c554_2_csd_gpio, tl16c554_2_csd_pin)
#define tl16c554_2_csd_deact()	tl16c554_cs_deact(tl16c554_2_csd_gpio, tl16c554_2_csd_pin)

#define tl16c554_1_reset_act()		tl16c554_reset_act(tl16c554_1_reset_gpio, tl16c554_1_reset_pin)
#define tl16c554_1_reset_deact()	tl16c554_reset_deact(tl16c554_1_reset_gpio, tl16c554_1_reset_pin)
#define tl16c554_2_reset_act()		tl16c554_reset_act(tl16c554_2_reset_gpio, tl16c554_2_reset_pin)
#define tl16c554_2_reset_deact()	tl16c554_reset_deact(tl16c554_2_reset_gpio, tl16c554_2_reset_pin)

/* -- tx/rx-en-- */
#define tl16c554_x_485rx_enable()	tl16c554_485rx_enable(tl16c554_x_chx_rx_en_gpio, tl16c554_x_chx_rx_en_pin)
#define tl16c554_x_485rx_disable()	tl16c554_485rx_disable(tl16c554_x_chx_rx_en_gpio, tl16c554_x_chx_rx_en_pin)

#define tl16c554_1_cha_485tx_enable()	tl16c554_485tx_enable(tl16c554_1_cha_tx_en_gpio, tl16c554_1_cha_tx_en_pin)
#define tl16c554_1_cha_485tx_disable()	tl16c554_485tx_disable(tl16c554_1_cha_tx_en_gpio, tl16c554_1_cha_tx_en_pin)
#define tl16c554_1_chb_485tx_enable()	tl16c554_485tx_enable(tl16c554_1_chb_tx_en_gpio, tl16c554_1_chb_tx_en_pin)
#define tl16c554_1_chb_485tx_disable()	tl16c554_485tx_disable(tl16c554_1_chb_tx_en_gpio, tl16c554_1_chb_tx_en_pin)
#define tl16c554_1_chc_485tx_enable()	tl16c554_485tx_enable(tl16c554_1_chc_tx_en_gpio, tl16c554_1_chc_tx_en_pin)
#define tl16c554_1_chc_485tx_disable()	tl16c554_485tx_disable(tl16c554_1_chc_tx_en_gpio, tl16c554_1_chc_tx_en_pin)
#define tl16c554_1_chd_485tx_enable()	tl16c554_485tx_enable(tl16c554_1_chd_tx_en_gpio, tl16c554_1_chd_tx_en_pin)
#define tl16c554_1_chd_485tx_disable()	tl16c554_485tx_disable(tl16c554_1_chd_tx_en_gpio, tl16c554_1_chd_tx_en_pin)

#define tl16c554_2_cha_485tx_enable()	tl16c554_485tx_enable(tl16c554_2_cha_tx_en_gpio, tl16c554_2_cha_tx_en_pin)
#define tl16c554_2_cha_485tx_disable()	tl16c554_485tx_disable(tl16c554_2_cha_tx_en_gpio, tl16c554_2_cha_tx_en_pin)
#define tl16c554_2_chb_485tx_enable()	tl16c554_485tx_enable(tl16c554_2_chb_tx_en_gpio, tl16c554_2_chb_tx_en_pin)
#define tl16c554_2_chb_485tx_disable()	tl16c554_485tx_disable(tl16c554_2_chb_tx_en_gpio, tl16c554_2_chb_tx_en_pin)
#define tl16c554_2_chc_485tx_enable()	tl16c554_485tx_enable(tl16c554_2_chc_tx_en_gpio, tl16c554_2_chc_tx_en_pin)
#define tl16c554_2_chc_485tx_disable()	tl16c554_485tx_disable(tl16c554_2_chc_tx_en_gpio, tl16c554_2_chc_tx_en_pin)
#define tl16c554_2_chd_485tx_enable()	tl16c554_485tx_enable(tl16c554_2_chd_tx_en_gpio, tl16c554_2_chd_tx_en_pin)
#define tl16c554_2_chd_485tx_disable()	tl16c554_485tx_disable(tl16c554_2_chd_tx_en_gpio, tl16c554_2_chd_tx_en_pin)

#endif


#endif /* __BOARD_H__ */
