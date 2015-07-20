/***********************************************************
*   函数库说明：底层硬件初始化函数库                       *
*   版本：    Ver1.0                                       *
*   作者：    zzjjhh250/ZZJJHH250 @ (CopyRight)            *
*   创建日期：11/13/2010                                   *
* -------------------------------------------------------- *
*  [硬件说明]                                              *
*   处理器：    STM32F103VBT6                              *
*   系统时钟：  外部8M/PLL 72M                             *
* -------------------------------------------------------- *
*  [支 持 库]                                              *
*   支持库名称：PF_Config.h                                *
*   需要版本：  -----                                      *
*   声明库说明：硬件平台配置声明库                         *
*                                                          *
*   支持库名称：HD_Support.h                               *
*   需要版本：  -----                                      *
*   声明库说明：底层硬件初始化声明库                       *
* -------------------------------------------------------- *
*  [版本更新]                                              *
*   修改：                                                 *
*   修改日期：                                             *
*   版本：                                                 *
* -------------------------------------------------------- *
*  [版本历史]                                              *
* -------------------------------------------------------- *
*  [使用说明]                                              *
***********************************************************/

/********************
* 头 文 件 配 置 区 *
********************/
#include "..\Source\FWLib\stm32f10x.h"
#include "..\Source\HD_Support.h"
#include "..\Source\LIB_Config.h"
#include ".\fwlib\stm32f10x_exti.h"
#include ".\fwlib\stm32f10x_pwr.h"
#include "ads8329.h"
#include "board.h"
#include "tctrl.h"


RCC_ClocksTypeDef RCC_ClockFreq;   //全局设备时钟提供商 ^_^


void Device_Init(void);

static void RCC_Configuration(void);
static void PORT_Init(void);
static void USART1_Configuration(void);
static void USART2_Configuration(void);

static void TIM3_Configuration(void);
static void adc1_dma_cfg(void);
static void ADC1_Configuration(void);

#if !USE_PVD_CHECK_POWEROFF
static void Exti_Init(void);
#else
static void Exti_Pvd_Init(void);
#endif
static void NVIC_Configuration(void);
static void DMA_USART1_Configuration(void);
extern void get_sig_cfg_param(int is_init);

void delay_us(int xus);

/***********************************************************
*   函数说明：系统硬件初始化函数                           *
*   输入：    无                                           *
*   输出：    无                                           *
*   调用函数：Port_Init()  RCC_Configuration()             *
***********************************************************/
void Device_Init(void)
{
	RCC_Configuration();
	get_sig_cfg_param(1);

	PORT_Init();/*此处需要修改  malooei  2012/11/14*/

	USART2_Configuration(); /* 用于连接pc */

	USART1_Configuration(); /* 用于向光纤传输数据 */
	DMA_USART1_Configuration( );

	adc1_dma_cfg();
	ADC1_Configuration();

	//box_tmper_ctrl_init();

#if !USE_PVD_CHECK_POWEROFF
	Exti_Init();
#else
	Exti_Pvd_Init();
#endif

	spi1_cfg4ads8329();
	delay_us(500*1000); /* 240ms, cpu最低工作电压为2.0V, ads8329最低工作电压为2.7V, David */
	init_ads8329();

	TIM3_Configuration();

	NVIC_Configuration();

	return;
}

/***********************************************************
*   函数说明：嵌套分组向量配置函数                         *
*   输入：    无                                           *
*   输出：    无                                           *
*   调用函数：无                                           *
*   [说明]                                                 *
***********************************************************/
static void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Configure one bit for preemption priority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);

#if 1
	/*USART2*/
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif

	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

#if !USE_PVD_CHECK_POWEROFF
	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#else
	NVIC_InitStructure.NVIC_IRQChannel = PVD_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif


#if 1
	/* 用于通过usart1给光纤发送数据 */
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif
	/* 45000, 40000
	 * min (SystemFrequency/5)
	 */
	if (SysTick_Config(SystemFrequency/SYS_TICK_DIV_CNT))
		while (1);

	/* 有4bits用于设置优先级, 抢占优先级,响应优先级的bit数, 由NVIC_PriorityGroupConfig决定 */
	NVIC_SetPriority(SysTick_IRQn, 1);

}

/*
 * tim2-7, tim12-14的clk max = 36 MHz, 经测试时钟到了72MHz
 * TIM3时钟720分频后为0.1MHz, 100 tick = 1 ms, David
 */
void TIM3_Configuration(void)
{
	TIM_TimeBaseInitTypeDef 	 TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	TIM_DeInit(TIM3);

	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Prescaler = 720 - 1; /* mark by David */
	TIM_TimeBaseStructure.TIM_Period    = (TIM3_PERIOD_MS_TICK_CNT - 1);
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;
	TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	TIM_ARRPreloadConfig(TIM3, ENABLE);
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);  //使能中断
	TIM_Cmd(TIM3, ENABLE);
}


/***********************************************************
*   函数说明：串口参数配置函数                             *
*   输入：    无                                           *
*   输出：    无                                           *
*   调用函数：INSERT_UART_ISR_CODE                         *
***********************************************************/
static void USART1_Configuration(void)
{
	USART_InitTypeDef 	USART_InitStructure;
	USART_ClockInitTypeDef 	USART_ClockInitStructure;

	/* 串口参数配置, 4500000 */
	USART_InitStructure.USART_BaudRate = 4500 * 1000;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_2;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_ClockInitStructure.USART_Clock = USART_Clock_Disable;
	USART_ClockInitStructure.USART_CPOL = USART_CPOL_Low;
	USART_ClockInitStructure.USART_CPHA = USART_CPHA_2Edge;
	USART_ClockInitStructure.USART_LastBit = USART_LastBit_Disable;
	USART_ClockInit(USART1, &USART_ClockInitStructure);

	USART_Init(USART1, &USART_InitStructure);

	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
}



/***********************************************************
*   函数说明：串口参数配置函数                             *
*   输入：    无                                           *
*   输出：    无                                           *
*   调用函数：INSERT_UART_ISR_CODE                         *
***********************************************************/
static void USART2_Configuration(void)
{
	USART_InitTypeDef 	USART_InitStructure;
	USART_ClockInitTypeDef 	USART_ClockInitStructure;

	/* 串口参数配置  */
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_ClockInitStructure.USART_Clock = USART_Clock_Disable;
	USART_ClockInitStructure.USART_CPOL = USART_CPOL_Low;
	USART_ClockInitStructure.USART_CPHA = USART_CPHA_2Edge;
	USART_ClockInitStructure.USART_LastBit = USART_LastBit_Disable;

	USART_ClockInit(USART2, &USART_ClockInitStructure);
	USART_Init(USART2, &USART_InitStructure);

	/* 串口2使能 */
	USART_Cmd(USART2, ENABLE);
#if 1
	USART_ITConfig(USART2, USART_IT_RXNE,ENABLE);
	USART_ITConfig(USART2, USART_IT_TXE,ENABLE);
#endif
}


/***********************************************************
*   函数说明：串口1的DMA通道配置函数                       *
*   输入：    无                                           *
*   输出：    无                                           *
*   调用函数： 无                                          *
***********************************************************/
//#define 	USART1_DR_Base  	0x40013804

static void DMA_USART1_Configuration(void)
{
	DMA_InitTypeDef     DMA_InitStructure_USART1;

	DMA_DeInit(DMA1_Channel4);
	DMA_InitStructure_USART1.DMA_PeripheralBaseAddr = 0x40013804;//(u32)(&(USART2->DR));
	DMA_InitStructure_USART1.DMA_MemoryBaseAddr = (u32)(uart1_dma_buf);
	DMA_InitStructure_USART1.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure_USART1.DMA_BufferSize = UART1_DMA_BUF_CNT;
	DMA_InitStructure_USART1.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure_USART1.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure_USART1.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure_USART1.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;

	DMA_InitStructure_USART1.DMA_Mode = DMA_Mode_Normal; //DMA_Mode_Circular

	DMA_InitStructure_USART1.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStructure_USART1.DMA_M2M = DMA_M2M_Disable;

	DMA_Init(DMA1_Channel4, &DMA_InitStructure_USART1);

	DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);	//DMA通道1传输完成中断
}


/*
不包括控制线的配置，只配置SPI1接口！
ADS8329连接STM32的SPI1接口！

SPI1与ADS8329连接的管脚说明（结合ADS8329数据手册提供的与CPU STM32的通信方式）：

STM32的SPI1端口配置，使能端口时钟分别连接Nss SCK MOSI MISO
根据STM32复用端口的配置表，PA4为NSS(实际为片选连接AD的片选)，PA5是时钟信号（连接ADS8329SCLK管脚），
   PA6为MISO（主输入从输出，连接ADS8329的SDOUT），PA7为MOSI(是否作为AD的命令输入)
相关控制线的配置管脚：ADS8329的EOC输出接至CPU一个IO口作为状态检测
                       ADS8329的CS输入接至CPU一个IO口作为控制线
					   ADS8329的CNVST输入接至CPU一个IO口作为控制线
					   另外：INT与EOC共用一引脚需要注意其处理方式？
*/

/***********************************************************
*   函数说明：系统端口初始化函数                           *
*   输入：    无                                           *
*   输出：    无                                           *
*   调用函数：无                                           *
***********************************************************/
static void PORT_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	//PORTE 初始化
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC
							| RCC_APB2Periph_AFIO | ledall_rcc, ENABLE);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_SPI1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

	/* USART1管脚配置 */
	/* PA9    USART1_Tx */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		//推挽输出-TX
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* PA10   USART1_Rx  */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //GPIO_Mode_IN_FLOATING;//浮空输入-RX  GPIO_Mode_IPU
	GPIO_Init(GPIOA, &GPIO_InitStructure);
#if 1
	/* USART2管脚配置 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;		  		/* PA2    USART1_Tx */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		//推挽输出-TX
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //GPIO_Mode_IN_FLOATING;//浮空输入-RX  GPIO_Mode_IPU
	GPIO_Init(GPIOA, &GPIO_InitStructure);
#endif

#if 1 /* ads8329 */
	/* Configure SPI1 pins: SCK, MISO and MOSI (ADS8329 using) */
	GPIO_InitStructure.GPIO_Pin 	= ADS8329_SPI_SCK | ADS8329_SPI_MISO | ADS8329_SPI_MOSI;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF_PP;
	GPIO_Init(ADS8329_SPI_PORT, &GPIO_InitStructure);

	/* (ADS8329_CS_PORT, ADS8329_CS_PIN), Configure SPI1 pin: NSS */
	GPIO_InitStructure.GPIO_Pin 	= ADS8329_CS_PIN;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;
	GPIO_Init(ADS8329_CS_PORT, &GPIO_InitStructure);
	GPIO_SetBits(ADS8329_CS_PORT, ADS8329_CS_PIN);

	GPIO_InitStructure.GPIO_Pin   = ADS8329_NCONVST_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_Init(ADS8329_NCONVST_PORT, &GPIO_InitStructure);
	GPIO_SetBits(ADS8329_NCONVST_PORT, ADS8329_NCONVST_PIN);

	GPIO_InitStructure.GPIO_Pin   = ADS8329_EOC_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
	GPIO_Init(ADS8329_EOC_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin   = SELECT_PA_CHANNEL_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_Init(SELECT_PA_CHANNEL_PORT, &GPIO_InitStructure);
	close_pa_switch();

	GPIO_InitStructure.GPIO_Pin   = SELECT_PB_CHANNEL_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_Init(SELECT_PB_CHANNEL_PORT, &GPIO_InitStructure);
	close_pb_switch();

	GPIO_InitStructure.GPIO_Pin   = SELECT_PC_CHANNEL_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_Init(SELECT_PC_CHANNEL_PORT, &GPIO_InitStructure);
	close_pc_switch();

	open_pa_switch();
#endif
	/* 恒温盒内温度采集及ABC三相的均值, ADC1, ADC_Channel_0->PA0->恒温盒内温度 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/*
	 * led
	 */
	GPIO_InitStructure.GPIO_Pin = led1_pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(led1_gpio, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = led2_pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(led2_gpio, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = led3_pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(led3_gpio, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = led4_pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(led4_gpio, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = led5_pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(led5_gpio, &GPIO_InitStructure);

#if 1
	/* 作为pwm信号输出管脚-对应timer2通道2  PB3 需进行Remap设置 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
#endif


	return;
}

/***********************************************************
*   函数声明：ADC初始化函数                                *
*   function：采集片内温度，保温盒温度，三相电有效值       *
*   输出    ：    无                                       *
*   调用函数：ST F.W. Ver3.0                               *
***********************************************************/
void ADC1_Configuration(void)
{
	ADC_InitTypeDef ADC_InitStructure;

	/* 12MHz的时钟 */
	RCC_ADCCLKConfig(RCC_PCLK2_Div6); /* David */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_Init(ADC1, &ADC_InitStructure);

	/*
	ADC1, ADC_Channel_0->PA0->恒温盒内温度

	ADC_SampleTime_71Cycles5
	*/
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0,  1, ADC_SampleTime_239Cycles5);  //恒温盒内温度

#if 1
	ADC_InjectedSequencerLengthConfig(ADC1, 1);
	ADC_InjectedChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_239Cycles5);//ADC_SampleTime_55Cycles5
	ADC_TempSensorVrefintCmd(ENABLE);  //测温度的 使能
	ADC_ExternalTrigInjectedConvConfig(ADC1, ADC_ExternalTrigInjecConv_None);	// ADC_ExternalTrigInjecConv_T2_TRGO
	ADC_AutoInjectedConvCmd(ADC1, ENABLE);
#endif
	ADC_DMACmd(ADC1, ENABLE);
	ADC_Cmd(ADC1, ENABLE);

	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

#if !USE_PVD_CHECK_POWEROFF
static void Exti_Init(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);

	EXTI_InitStructure.EXTI_Line 	= EXTI_Line1;
	EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	//EXTI_GenerateSWInterrupt(EXTI_Line1);

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);

}
#else
static void Exti_Pvd_Init(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;

	EXTI_InitStructure.EXTI_Line 	= EXTI_Line16;
	EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;
	/* pvd信号与电压信号相反 */
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	//EXTI_GenerateSWInterrupt(EXTI_Line16);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR,ENABLE);
	PWR_PVDLevelConfig(PWR_PVDLevel_2V9);
	PWR_PVDCmd(ENABLE);

	return;
}
#endif

/***********************************************************
*   函数声明：DMA初始化函数                                *
*   输入：    无                                           *
*   输出：    无                                           *
*   调用函数：无                                           *
***********************************************************/
void adc1_dma_cfg(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	/* 允许 DMA1 */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	/* DMA通道1*/
	DMA_DeInit(DMA1_Channel1);
	DMA_InitStructure.DMA_PeripheralBaseAddr =(u32)( &(ADC1->DR));
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)adc_buf4temper;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = 1;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;	 //DMA_Mode_Circular
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	//DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE); //David
	DMA_Cmd(DMA1_Channel1, ENABLE);
}

/***********************************************************
*   函数说明：系统时钟初始化函数                           *
*   输入：    无                                           *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ：HSE作为时钟 PLL的锁频输出72M（MAX值）           *
***********************************************************/
static void RCC_Configuration(void)
{
	SystemInit();//源自system_stm32f10x.c文件,只需要调用此函数,则可完成RCC的配置.
	RCC_GetClocksFreq(&RCC_ClockFreq);
}

void delay_us(int xus)
{
	int i;

	xus *= 6;
	for (i=0; i<xus; i++)
		;

	return;
}

