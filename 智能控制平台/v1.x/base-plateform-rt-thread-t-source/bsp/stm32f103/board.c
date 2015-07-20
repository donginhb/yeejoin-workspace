/*
 * File      : board.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009 RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      first implementation
 */

#include <rthw.h>
#include <rtthread.h>

#include "stm32f10x.h"
#include "stm32f10x_fsmc.h"
#include "board.h"
#include "usart.h"
#include "stm32_eth.h"
#include "rtconfig.h"
#include <stm32f10x_rcc.h>
#include <sys_app_cfg.h>
#include <stm32f10x_iwdg.h>
#include <core_cm3.h>

static void RCC_Configuration(void);
static void GPIO_Configuration(void);
static void NVIC_Configuration(void);
static void SysTick_Configuration(void);
static void exti_config(void);
static void SPI_config(void);

#if USE_STM32_IWDG
static void iwdg_init(void);
#endif
void FSMC_LCD_Init(void);

#define TP_CS()  GPIO_ResetBits(GPIOB,GPIO_Pin_7)
#define TP_DCS() GPIO_SetBits(GPIOB,GPIO_Pin_7)

/**
 * @addtogroup STM32
 */

/*@{*/


/**
 * This function will initial STM32 board.
 */
void rt_hw_board_init()
{
	RCC_Configuration();
	GPIO_Configuration();
	exti_config(); /* mark by David */
	NVIC_Configuration();

	/* Configure the SysTick */
	SysTick_Configuration();

#if STM32_EXT_SRAM
	EXT_SRAM_Configuration();
#endif

	rt_hw_usart_init();
	rt_console_set_device(CONSOLE_DEVICE);

	SPI_config();
#if 1==USE_TO_7INCH_LCD
	FSMC_LCD_Init();
#endif

#if USE_STM32_IWDG
	iwdg_init();
#endif
}

/**
 * This is the timer interrupt service routine.
 *
 */
void rt_hw_timer_handler(void)
{
	/* enter interrupt */
	rt_interrupt_enter();

	rt_tick_increase();

	/* leave interrupt */
	rt_interrupt_leave();
}


static void RCC_Configuration(void)
{
	uint32_t gpio_rcc = 0;

	gpio_rcc = RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC |
		RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE;
	/* Enable GPIOs clocks */
	RCC_APB2PeriphClockCmd(gpio_rcc, ENABLE); 	
#if 1==USE_TO_7INCH_LCD
	/* Enable the FSMC Clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
#endif
	/* Enable AFIO clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	/*
	* USART
	*/
#ifdef RT_USING_UART1
	/* Enable USART1 and GPIOA clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
#endif


#ifdef RT_USING_UART2
	/* Enable AFIO clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
#if (defined(STM32F10X_LD) || defined(STM32F10X_MD) || defined(STM32F10X_CL))
	/* Enable the USART2 Pins Software Remapping */
	GPIO_PinRemapConfig(GPIO_Remap_USART2, ENABLE);
#endif
	/* Enable USART2 clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
#endif


#ifdef RT_USING_UART3
	/* Enable USART3 clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	/* DMA clock enable */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
#endif


#if 0 //def RT_USING_I2C
	/* I2C */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);  /* Enable I2C1 clock */
#endif

	/*
	 * enc28j60
	 */
#if 0
#if RCC_APB1Periph_SPI2 == ENC28J60_SPI_RCC_PERIPH
	/* Enable SPI2 GPIOB clocks */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
#elif RCC_APB2Periph_SPI1 == ENC28J60_SPI_RCC_PERIPH
	/* Enable SPI1 GPIOB clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
#endif
#else
	/* Enable SPI2 GPIOB clocks */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
#endif

}


/*
 *  PERIPHERALS	MODES	REMAP	FUNCTIONS	PINS
 *  SPI1	Mode	0	SPI1_MISO	PA6
 *  SPI1	Mode	0	SPI1_MOSI	PA7
 *  SPI1	Mode	0	SPI1_SCK	PA5
 *  USART1	Mode	0	USART1_RX	PA10
 *  USART1	Mode	0	USART1_TX	PA9
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_A16	PD11
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_A17	PD12
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_A18	PD13
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_A19	PE3
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_A20	PE4
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_A21	PE5
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_A22	PE6
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_A23	PE2
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_CLK	PD3
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_DA0	PD14
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_DA1	PD15
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_DA10	PE13
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_DA11	PE14
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_DA12	PE15
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_DA13	PD8
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_DA14	PD9
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_DA15	PD10
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_DA2	PD0
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_DA3	PD1
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_DA4	PE7
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_DA5	PE8
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_DA6	PE9
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_DA7	PE10
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_DA8	PE11
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_DA9	PE12
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_NADV	PB7
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_NBL0	PE0
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_NBL1	PE1
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_NE1	PD7
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_NOE	PD4
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_NWAIT	PD6
 * FSMC_NOR_mux	Mode	0	FSMC_NOR_mux_NWE	PD5
 */
/*
 * ====ADS7843触摸屏芯片====
 * 引脚号	管脚		注释	引脚号		注释	备注
 * 92	PB6			11		PENIRQ	
 * 31	PA6			12		DOUT	
 * 32	PA7			14		DIN	
 * 93	PB7			15		CS	
 * 30	PA5			16		DCLK	
 */
/*
 * CPU中所对应SST25VF016B接口	SST25VF016B接口
 * 引脚号	管脚		注释	引脚号		注释	备注
 * 29	PA4		SPI1-NSS	1		CS	
 * 31	PA6		SPI1-MISO	2		SO	
 * 30	PA5		SPI1-SCK	6		SCK	
 * 91	PB5		SPI1-MOSI	5		SI	
 */
/*
 * CPU中所对应SD卡接口	SD卡接口
 * 引脚号	管脚		注释	引脚号		注释	备注
 * 78	PC10		SDIO-D2	1		PC10-SDIO-D2	
 * 79	PC11		SDIO-D3	2		PC11-SDIO-D3	
 * 83	PD2		SDIO-CMD	3		PD2-SDIO-CMD	
 * 80	PC12		SDIO-CK	5		PC12-SDIO-CK	
 * 65	PC8		SDIO-D0	7		PC8-SDIO-D0	
 * 66	PC9		SDIO-D1	8		PC9-SDIO-D1	
 */
static void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
#if 0
	/* LED1, led1(v2)	-- PB5, 高电平亮 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);					 

	/* PEN--IRQ */
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Configure SPI1 pins: SCK, MISO and MOSI */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure PB.7 as Output push-pull, used as Flash Chip select */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
#endif
#if 1==USE_TO_7INCH_LCD
	/*RA8875判断忙管脚 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* RA8875 reset pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	GPIO_SetBits(GPIOE, GPIO_Pin_1);

	/* RA8875 interrupt pin */
	GPIO_InitStructure.GPIO_Pin = RA8875_INT_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(RA8875_INT_PORT, &GPIO_InitStructure);

	/*
	 * FSMC
	 * PB7, PD0, PD1, PD[3:15], PE[0:15]
	 *
	 * 启用FSMC复用功能， 定义FSMC D0---D15及nWE, nOE对应的引脚
	 */
	/* PB.7 used as touch Chip select */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* NE1 configuration GPIO_Pin_7 */
	/* RS GPIO_Pin_11 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_7
		| GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | 
		GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOE, &GPIO_InitStructure); 

	GPIO_SetBits(GPIOD, GPIO_Pin_7);			//CS=1 
	GPIO_SetBits(GPIOD, GPIO_Pin_14| GPIO_Pin_15 |GPIO_Pin_0 | GPIO_Pin_1);  	 
	GPIO_SetBits(GPIOE, GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10);   
	GPIO_SetBits(GPIOE, GPIO_Pin_1);		//RESET=1
	GPIO_SetBits(GPIOD, GPIO_Pin_4);		    //RD=1
	GPIO_SetBits(GPIOD, GPIO_Pin_5);			//WR=1
#endif

#ifdef RT_USING_UART1
	/* Configure USART1 Rx (PA.10) as input floating */
	GPIO_InitStructure.GPIO_Pin = UART1_GPIO_RX;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; /* input pull up */
	GPIO_Init(UART1_GPIO_PORT, &GPIO_InitStructure);

	/* Configure USART1 Tx (PA.09) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = UART1_GPIO_TX;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(UART1_GPIO_PORT, &GPIO_InitStructure);
#endif

	GPIO_InitStructure.GPIO_Pin   = TMP_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
	GPIO_Init(TMP_PORT, &GPIO_InitStructure);

#if 1 /* ENC28J60 */
	/*
	 * stm32f103ve使用SPI2与ENC28J60连接
	 * NSS	-- PB12
	 * SCK	-- PB13
	 * MISO	-- PB14
	 * MOSI	-- PB15
	 *
	 * ENC28J60的/int引脚连接至stm32f103ve的PA8
	 * ENC28J60的/RST引脚连接至stm32f103ve的PC6
	 *
	 * ENC28J60的/int引脚连接至stm32f103ve的PB10
	 * ENC28J60的/RST引脚连接至stm32f103ve的PB11
	 */

	/* Configure SPI pins: SCK, MISO and MOSI */
	GPIO_InitStructure.GPIO_Pin 	= ENC28J60_SPI_SCK | ENC28J60_SPI_MISO | ENC28J60_SPI_MOSI;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF_PP;
	GPIO_Init(ENC28J60_SPI_PORT, &GPIO_InitStructure);

	/* (ENC28J60_CS_PORT, ENC28J60_CS_PIN), Configure SPI2 pin: NSS */
	GPIO_InitStructure.GPIO_Pin 	= ENC28J60_SPI_NSS;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;
	GPIO_Init(ENC28J60_SPI_PORT, &GPIO_InitStructure);
	GPIO_SetBits(ENC28J60_SPI_PORT, ENC28J60_SPI_NSS);   

	/* /int */
	GPIO_InitStructure.GPIO_Pin 	= ENC28J60_INT_PIN;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IN_FLOATING;
	GPIO_Init(ENC28J60_INT_PORT, &GPIO_InitStructure);

	/* /rst */
	GPIO_InitStructure.GPIO_Pin 	= ENC28J60_RST_PIN;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;
	GPIO_Init(ENC28J60_RST_PORT, &GPIO_InitStructure);
	GPIO_SetBits(ENC28J60_RST_PORT, ENC28J60_RST_PIN);

#endif
	/* elock */
	GPIO_InitStructure.GPIO_Pin 	= ELOCK_PIN;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;
	GPIO_Init(ELOCK_PORT, &GPIO_InitStructure);
	GPIO_ResetBits(ELOCK_PORT, ELOCK_PIN);

	GPIO_InitStructure.GPIO_Pin 	= ELOCK_SWITCH_PIN;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IN_FLOATING;
	GPIO_Init(ELOCK_SWITCH_PORT, &GPIO_InitStructure);

#if USE_LED_TEST
	GPIO_InitStructure.GPIO_Pin 	= LED1_PIN | LED2_PIN | LED3_PIN;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;
	GPIO_Init(LED_PORTX, &GPIO_InitStructure);
	GPIO_ResetBits(LED_PORTX, LED1_PIN | LED2_PIN | LED3_PIN);
#endif
}

/*******************************************************************************
* Function Name  : NVIC_Configuration
* Description    : Configures Vector Table base location.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

#ifdef  VECT_TAB_RAM
	/* Set the Vector Table base location at 0x20000000 */
	NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else  /* VECT_TAB_FLASH  */
	/* Set the Vector Table base location at 0x08000000 */
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
#endif

#ifdef RT_USING_UART1
	/* Enable the USART1 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif
	//nvic_cfg_app();
}



/*******************************************************************************
* Function Name  : NVIC_Configuration
* Description    : Configures Vector Table base location.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void nvic_cfg_app(void)
{
#if 1==USE_TO_7INCH_LCD
	NVIC_InitTypeDef NVIC_InitStructure;

	/* 允许RA8875中断 */
	NVIC_InitStructure.NVIC_IRQChannel = RA8875_INT_NUM;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);      
#endif
#if 0
	/* 允许ENC28J60中断 */
	NVIC_InitStructure.NVIC_IRQChannel = ENC28J60_INT_NUM;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);      
#endif
}


/*******************************************************************************
 * Function Name  : SysTick_Configuration
 * Description    : Configures the SysTick for OS tick.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
static void  SysTick_Configuration(void)
{
	RCC_ClocksTypeDef  rcc_clocks;
	rt_uint32_t         cnts;

	RCC_GetClocksFreq(&rcc_clocks);

	cnts = (rt_uint32_t)rcc_clocks.HCLK_Frequency / RT_TICK_PER_SECOND;

	SysTick_Config(cnts);
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
}

#if USE_STM32_IWDG
/*
 * These timings are given for a 40 kHz clock but the microcontroller’s internal RC frequency can vary 
 * from 30 to 60 kHz. 
 * t0 = 1/f40 * cnt / pre_div
 * t1 = 1/f30 * cnt / pre_div = 4/3 * t0
 * t2 = 1/f60 * cnt / pre_div = 2/3 * t0
 *
 * Prescaler divider 	PR[2:0] bits 	Min timeout (ms) RL[11:0]=0x000 	Max timeout (ms) RL[11:0]=0xFFF
 * /4 			0 		0.1 					409.6
 */

/*
 * t = (reload_val + 1) * 1/f40 / pre_div
 *
 * t0(ms)	t1(ms)	t2(ms)
 * 0.1		0.13	0.067
 * 409.6	546.13	273.07
 * 100(0x3e7)	133.3   66.67
 *
 * 30(0x12b)	40	20
 * 90(0x383)	120	60
 *
 * IWDG_RELOAD_VALUE -- max 0xfff
 */
#define IWDG_RELOAD_VALUE (0x383)

static void iwdg_init(void)
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

	IWDG_SetPrescaler(IWDG_Prescaler_4);
	IWDG_SetReload(IWDG_RELOAD_VALUE);
#if 0
	rt_kprintf("IWDG->SR:0x%x\n", IWDG->SR);
	/* 在没有iwdg没有enable时, 状态标志可能不会更新, ysh>>IWDG->SR:0x3 */
	while (SET==IWDG_GetFlagStatus(IWDG_FLAG_PVU)) 
		; /* nothing, 等待寄存器更新完成 */
	while (SET==IWDG_GetFlagStatus(IWDG_FLAG_RVU))
		; /* nothing, 等待寄存器更新完成 */
#endif		
	/* 禁止更新寄存器应该只是禁止软件来更新, 不会停止正在进行的更新(由硬件正在进行的更新) */
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Disable);

	IWDG_Enable();
	return;
}
#endif

void reset_whole_system(void)
{
	printf_syn("app will reset cm3!\n");

	NVIC_SystemReset();

	return;	
}

#if STM32_EXT_SRAM
static void EXT_SRAM_Configuration(void)
{
	FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
	FSMC_NORSRAMTimingInitTypeDef  p;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOG | RCC_APB2Periph_GPIOE |
						   RCC_APB2Periph_GPIOF, ENABLE);

	/*-- GPIO Configuration ------------------------------------------------------*/
	/* SRAM Data lines configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_8 | GPIO_Pin_9 |
								  GPIO_Pin_10 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 |
								  GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 |
								  GPIO_Pin_15;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* SRAM Address lines configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 |
								  GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_12 | GPIO_Pin_13 |
								  GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_Init(GPIOF, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 |
								  GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_Init(GPIOG, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* NOE and NWE configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 |GPIO_Pin_5;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* NE3 NE4 configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_12;
	GPIO_Init(GPIOG, &GPIO_InitStructure);

	/* NBL0, NBL1 configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/*-- FSMC Configuration ------------------------------------------------------*/
	p.FSMC_AddressSetupTime = 0;
	p.FSMC_AddressHoldTime = 0;
	p.FSMC_DataSetupTime = 2;
	p.FSMC_BusTurnAroundDuration = 0;
	p.FSMC_CLKDivision = 0;
	p.FSMC_DataLatency = 0;
	p.FSMC_AccessMode = FSMC_AccessMode_A;

	FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM3;
	FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
	FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;
	FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
	FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
	FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &p;
	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &p;

	FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);

	/* Enable FSMC Bank1_SRAM Bank */
	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM3, ENABLE);
}
#endif


/*
 *****************************************************************************
 * 函数名       EXTI_Configuration
 * 描述           配置EXTI线
 *****************************************************************************
 */
static void exti_config(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
#if 0	
	/*将EXTI线7连接到PD7*/
	GPIO_EXTILineConfig(TCA8418_INT_PORT_SOURCE, TCA8418_INT_PIN_SOURCE);

	/*配置EXTI线0上出现下降沿，则产生中断*/
	EXTI_InitStructure.EXTI_Line 	= TCA8418_INT_LINE;
	EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	EXTI_GenerateSWInterrupt(TCA8418_INT_LINE);
#endif

	/* 将EXTI线连接到enc28j60'/int */
	GPIO_EXTILineConfig(ENC28J60_INT_PORT_SOURCE, ENC28J60_INT_PIN_SOURCE);
	/*配置EXTI线上出现下降沿，则产生中断*/
	EXTI_InitStructure.EXTI_Line 	= ENC28J60_INT_EXT_LINE;
	EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	EXTI_GenerateSWInterrupt(ENC28J60_INT_EXT_LINE);
#if 1==USE_TO_7INCH_LCD
	/* 将EXTI线连接到ra8875'/int */
	GPIO_EXTILineConfig(RA8875_INT_PORT_SOURCE, RA8875_INT_PIN_SOURCE);
	/*配置EXTI线上出现下降沿，则产生中断*/
	EXTI_InitStructure.EXTI_Line 	= RA8875_INT_EXT_LINE;
	EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	EXTI_GenerateSWInterrupt(RA8875_INT_EXT_LINE);
#endif

}


static void SPI_config(void)
{
	SPI_InitTypeDef   SPI_InitStructure; 
#if 0
	/* SPI1 configuration */ 
	SPI_InitStructure.SPI_Direction 	= SPI_Direction_2Lines_FullDuplex; 
	SPI_InitStructure.SPI_Mode 		= SPI_Mode_Master; 
	SPI_InitStructure.SPI_DataSize 		= SPI_DataSize_8b; 
	SPI_InitStructure.SPI_CPOL 		= SPI_CPOL_Low; 
	SPI_InitStructure.SPI_CPHA 		= SPI_CPHA_1Edge; 
	SPI_InitStructure.SPI_NSS 		= SPI_NSS_Soft;   //SPI_NSS_Hard
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64; 
	SPI_InitStructure.SPI_FirstBit 		= SPI_FirstBit_MSB; 
	SPI_InitStructure.SPI_CRCPolynomial 	= 7; 
	SPI_Init(SPI1,&SPI_InitStructure); 

	SPI_Cmd(SPI1,ENABLE);  
#endif
	/* SPI2 configuration */
	/*
	 * SPI2 clk -- APB1, fCLK1(max = 36MHz)
	 * enc28j60's clk(max) = 10MHz
	 * DS80349C.errata #1:When the SPI clock from the host microcontroller is run at frequencies of less than 8 MHz,
	 * reading or writing to the MAC registers may be unreliable. 
	 */
	SPI_InitStructure.SPI_Direction 	= SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode 		= SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize 		= SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL 		= SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA 		= SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS 		= SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
	SPI_InitStructure.SPI_FirstBit 		= SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial 	= 7;
	SPI_Init(SPI2, &SPI_InitStructure);
	
	SPI_Cmd(SPI2, ENABLE); 

	return;
}


unsigned char SPI_ReadWrite(SPI_TypeDef* SPIx, unsigned char writedat) 
{ 
	unsigned char Data = 0; 

	//Wait until the transmit buffer is empty 
	while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE)==RESET); 
	// Send the byte  
	SPI_I2S_SendData(SPIx, writedat); 

	//Wait until a data is received 
	while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE)==RESET); 
	// Get the received data 
	Data = SPI_I2S_ReceiveData(SPIx); 

	// Return the shifted data 
	return Data; 
}  


void SpiDelay(unsigned int DelayCnt)
{
 unsigned int i;
 for(i=0;i<DelayCnt;i++);
}

u16 TPReadX(SPI_TypeDef* SPIx)
{ 
   u16 x=0;
   TP_CS();
   SpiDelay(10);
   SPI_ReadWrite(SPIx, 0x90);
   
 //  SPI_ReadWrite(0xd0);
   SpiDelay(10);
   x=SPI_ReadWrite(SPIx, 0x00);
   x<<=8;
   x+=SPI_ReadWrite(SPIx, 0x00);
   SpiDelay(10);
   TP_DCS(); 
   x = x>>3;
   return (x);
}

u16 TPReadY(SPI_TypeDef* SPIx)
{
 u16 y=0;
  TP_CS();
  SpiDelay(10);
  SPI_ReadWrite(SPIx, 0xD0);
  
 // SPI_ReadWrite(0x90);
  SpiDelay(10);
  y=SPI_ReadWrite(SPIx, 0x00);
  y<<=8;
  y+=SPI_ReadWrite(SPIx, 0x00);
  SpiDelay(10);
  TP_DCS();
  y = y>>3; 
  return (y);
}




/**
  * @brief  Configures the FSMC and GPIOs to interface with the SRAM memory.
  *         This function must be called before any write/read operation
  *         on the SRAM.
  * @param  None 
  * @retval : None
  */
void FSMC_LCD_Init(void)
{
	FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
	FSMC_NORSRAMTimingInitTypeDef  p;

	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct 	= &p;
	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct 	= &p;	  
	FSMC_NORSRAMStructInit(&FSMC_NORSRAMInitStructure);

	p.FSMC_AddressSetupTime = 0x02;
	p.FSMC_AddressHoldTime = 0x00;
	p.FSMC_DataSetupTime = 0x05;
	p.FSMC_BusTurnAroundDuration = 0x00;
	p.FSMC_CLKDivision = 0x00;
	p.FSMC_DataLatency = 0x00;
	p.FSMC_AccessMode = FSMC_AccessMode_B;

	FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM1;
	FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
	FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_NOR;
	FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
	FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
	FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;

	FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure); 

	/* Enable FSMC Bank1_SRAM Bank */
	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);  
}


#if 1
#include <enc28j60_config.h>

#define is_spix_sr_flag_set(spix, flag)	(0 != ((spix)->SR & (flag)))

#define spi_i2s_send_byte(spix, data)	((spix)->DR = (unsigned char)data)
#define spi_i2s_recv_byte(spix)		((spix)->DR)

#define DUMMY_DATA4ENC28J60 (0xc0)
//#define DUMMY_DATA4ENC28J60 (0xff)

/**
 * Sends a byte over the SPI bus.
 *
 * \param[in] b The byte to send.
 */
uint8_t spix_send_byte(SPI_TypeDef* SPIx, unsigned char writedat)
{
	//Wait until the transmit buffer is empty 
	while(!is_spix_sr_flag_set(SPIx, SPI_I2S_FLAG_TXE)); 
	spi_i2s_send_byte(SPIx, writedat); 

	//Wait until a data is received 
	while(!is_spix_sr_flag_set(SPIx, SPI_I2S_FLAG_RXNE)); 

	return spi_i2s_recv_byte(SPIx); 
}

/**
 * Receives a byte from the SPI bus.
 *
 * \returns The received byte.
 */
uint8_t spix_rec_byte(SPI_TypeDef* SPIx)
{
	while(!is_spix_sr_flag_set(SPIx, SPI_I2S_FLAG_TXE)); 
	spi_i2s_send_byte(SPIx, DUMMY_DATA4ENC28J60); /* 为了使clock能够使用 */

	//Wait until a data is received 
	while(!is_spix_sr_flag_set(SPIx, SPI_I2S_FLAG_RXNE)); 

	return spi_i2s_recv_byte(SPIx); 
}

/**
 * Sends data contained in a buffer over the SPI bus.
 *
 * \param[in] data A pointer to the buffer which contains the data to send.
 * \param[in] data_len The number of bytes to send.
 */
void spix_send_data(SPI_TypeDef* SPIx, const uint8_t* data, uint16_t data_len)
{
	uint8_t b;
	while (data_len--) {
		b = *data++;

		while(!is_spix_sr_flag_set(SPIx, SPI_I2S_FLAG_TXE)); 
		spi_i2s_send_byte(SPIx, b); 
		//Wait until a data is received 
		while(!is_spix_sr_flag_set(SPIx, SPI_I2S_FLAG_RXNE)); 
		spi_i2s_recv_byte(SPIx); 
	}

	return;
}

/**
 * Receives multiple bytes from the SPI bus and writes them to a buffer.
 *
 * \param[out] buffer A pointer to the buffer into which the data gets written.
 * \param[in] buffer_len The number of bytes to read.
 */
void spix_rec_data(SPI_TypeDef* SPIx, uint8_t* buffer, uint16_t buffer_len)
{
	while (buffer_len--) {
		while(!is_spix_sr_flag_set(SPIx, SPI_I2S_FLAG_TXE)); 
		spi_i2s_send_byte(SPIx, DUMMY_DATA4ENC28J60); /* 为了使clock能够使用 */

		while(!is_spix_sr_flag_set(SPIx, SPI_I2S_FLAG_RXNE)); 
		*buffer++ = spi_i2s_recv_byte(SPIx); 
	}

	return;
}

#endif

/*@}*/
