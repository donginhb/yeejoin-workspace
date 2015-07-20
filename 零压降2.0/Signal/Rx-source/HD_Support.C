/***********************************************************
*   函数库说明：底层硬件初始化函数库                       *
*   版本：    Ver1.0                                       *
*   作者：    zzjjhh250/ZZJJHH250 @ (CopyRight)            *
*   创建日期：11/17/2010                                   *
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
#include "sys_comm.h"
#include "board.h"
#include "tctrl.h"
#include "common.h"

/********************
*   模块函数声明区  *
********************/
static void RCC_Configuration(void);
static void PORT_Init(void);
static void USART1_Configuration(void);
static void NVIC_Configuration(void);
static void TIM3_Configuration(void);
static void adc1_dma_cfg(void);
static void ADC1_Configuration(void);
static void Exti_Init(void);
static void USART3_Configuration(void);


#if (!IS_USART2_USE_DMA)
static void USART2_Configuration(void);
#else
static void init_usart2_cfg_with_dma_rx(void);
static void usart2_cfg_dma1_6_rx(void);
#endif
void DMA_Configuration_SPI1_TX( void );
void SPI1_Configuration_withDMA( void );


/********************
*   模块变量声明区  *
********************/

#define IS_USART2_USE_DMA 0

/********************
*   全局变量声明区  *
********************/
RCC_ClocksTypeDef RCC_ClockFreq;

volatile u8 uart2_rx_buf[6];
DMA_InitTypeDef     DMA_InitStructure_USART2;

#define 	USART2_DR_ADDR  	(USART2_BASE + 0x04)

extern void get_sig_cfg_param(int is_init);

#if USE_PVD
static void Exti_Pvd_Init(void);
#endif

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
	PORT_Init();

#if (0 != IS_USART2_USE_DMA)
	init_usart2_cfg_with_dma_rx();
	usart2_cfg_dma1_6_rx();
#else
	USART2_Configuration(); /* 用于与pc机通信 */
#endif
	USART1_Configuration(); /* 使用usart1接收光纤数据 */

	/* 使用dma方式, 用spi将数据发送给片外dac */
	SPI1_Configuration_withDMA();
	DMA_Configuration_SPI1_TX();
	//Exti_Init();

	/* 用于与lcd的cpu通信 */
	USART3_Configuration();
	//usart3_cfg_dma1_2_tx();
	//box_tmper_ctrl_init();

#if 0
	/*
	* David, sys_tick -- 10ms
	*/
	if (SysTick_Config(SystemFrequency/100 )) {
		while (1);
	}
#endif

	adc1_dma_cfg();
	ADC1_Configuration();

	TIM3_Configuration();

#if USE_PVD
	Exti_Pvd_Init();
#endif
	NVIC_Configuration();

	clr_dac();

	enable_usartx(USART1);

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

	/*USART3*/
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/*USART2*/
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/*USART1*/
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/*2013/3/14日加入 malooei */
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#if 0
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif

#if USE_PVD
	NVIC_InitStructure.NVIC_IRQChannel = PVD_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif
}

/*
 * tim2-7, tim12-14的clk max = 36 MHz, 经测试时钟到了72MHz
 * TIM3时钟360分频后为0.1MHz, 100 tick = 1 ms, David
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
*   函数说明：DMA 通道3 的SPI1的初始化函数                 *
*   输入：    无                                           *
*   输出：    无                                           *
*   调用函数：无                                           *
*   [说明]                                                 *
***********************************************************/
void DMA_Configuration_SPI1_TX( void )
{
	DMA_InitTypeDef  DMA_InitStructure;

	DMA_DeInit(DMA1_Channel3);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&(SPI1->DR));
	DMA_InitStructure.DMA_MemoryBaseAddr     = (u32)(&(SPI_DMA_Table_serial_in[0]));
	DMA_InitStructure.DMA_DIR                = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_BufferSize         = 6;
	DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode               = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority           = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_M2M                = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel3, &DMA_InitStructure);
}


/***********************************************************
*   函数说明：SPI1初始化 使用DMA模式传输函数               *
*   输入：    无                                           *
*   输出：    无                                           *
*   调用函数：无                                           *
*   [说明]                                                 *
***********************************************************/
void SPI1_Configuration_withDMA( void )
{
	SPI_InitTypeDef  SPI_InitStructure;

	SPI_Cmd(SPI1, DISABLE);//必须先禁能,才能改变MODE
	SPI_InitStructure.SPI_Direction         = SPI_Direction_2Lines_FullDuplex; //SPI_Direction_1Line_Tx, SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode              = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize          = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL              = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA              = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS               = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
	SPI_InitStructure.SPI_FirstBit          = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial     = 7;
	SPI_Init(SPI1, &SPI_InitStructure);

	SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
	SPI_Cmd(SPI1, ENABLE);
}

static void USART1_Configuration(void)
{
	USART_InitTypeDef 		USART_InitStructure;
	USART_ClockInitTypeDef 	USART_ClockInitStructure;
	/* 串口参数配置, 4500000 */
	USART_InitStructure.USART_BaudRate = 4500 * 1000;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_2;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx;
	USART_ClockInitStructure.USART_Clock = USART_Clock_Disable;
	USART_ClockInitStructure.USART_CPOL = USART_CPOL_Low;
	USART_ClockInitStructure.USART_CPHA = USART_CPHA_2Edge;
	USART_ClockInitStructure.USART_LastBit = USART_LastBit_Disable;
	USART_ClockInit(USART1, &USART_ClockInitStructure);
	USART_Init(USART1, &USART_InitStructure);

	/* 串口1使能, 延迟使能usart1 */
	//USART_Cmd(USART1, ENABLE);
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE); //允许串口1接收中断

	return;
}



#if 0
static void USART2_Configuration_withDMATx(void)
{
	USART_InitTypeDef 	USART_InitStructure;
	USART_ClockInitTypeDef 	USART_ClockInitStructure;
	/* 串口参数配置  */
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_2;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_ClockInitStructure.USART_Clock = USART_Clock_Disable;
	USART_ClockInitStructure.USART_CPOL = USART_CPOL_Low;
	USART_ClockInitStructure.USART_CPHA = USART_CPHA_2Edge;
	USART_ClockInitStructure.USART_LastBit = USART_LastBit_Disable;
	USART_ClockInit(USART2, &USART_ClockInitStructure);
	USART_Init(USART2, &USART_InitStructure);
	/* 串口1使能 */
	USART_Cmd(USART2, ENABLE);
	USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);
	USART_ITConfig(USART2,USART_IT_TXE,ENABLE);
	USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
}
static void usart2_cfg_dma1_7_tx(void)
{
	DMA_InitTypeDef     DMA_InitStructure_USART2;
	DMA_DeInit(DMA1_Channel7);
	DMA_InitStructure_USART2.DMA_PeripheralBaseAddr = (u32)(&(USART2->DR));//0x40013804;
	DMA_InitStructure_USART2.DMA_MemoryBaseAddr = (u32)(adc_formtx_tolcd_data_buf);
	DMA_InitStructure_USART2.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure_USART2.DMA_BufferSize = 8;
	DMA_InitStructure_USART2.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure_USART2.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure_USART2.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure_USART2.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure_USART2.DMA_Mode = DMA_Mode_Normal ; //DMA_Mode_Circular
	DMA_InitStructure_USART2.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStructure_USART2.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel7, &DMA_InitStructure_USART2);
	DMA_ITConfig(DMA1_Channel7, DMA_IT_TC, ENABLE);	//DMA通道7传输完成中断
}
#endif


static void USART3_Configuration(void)
{
	USART_InitTypeDef 	USART_InitStructure;
	USART_ClockInitTypeDef 	USART_ClockInitStructure;

	/* 串口参数配置  */
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_ClockInitStructure.USART_Clock = USART_Clock_Disable;
	USART_ClockInitStructure.USART_CPOL = USART_CPOL_Low;
	USART_ClockInitStructure.USART_CPHA = USART_CPHA_2Edge;
	USART_ClockInitStructure.USART_LastBit = USART_LastBit_Disable;
	USART_ClockInit(USART3, &USART_ClockInitStructure);
	USART_Init(USART3, &USART_InitStructure);
	/* 串口3使能 */
	USART_Cmd(USART3, ENABLE);
	USART_ITConfig(USART3,USART_IT_RXNE,ENABLE);
#if 0
	USART_ITConfig(USART3,USART_IT_TXE,ENABLE);
#endif
	//USART_DMACmd(USART3, USART_DMAReq_Tx, ENABLE);
}


#if 0
static void usart3_cfg_dma1_2_tx(void)
{
	DMA_InitTypeDef     DMA_InitStructure_USART3;
	DMA_DeInit(DMA1_Channel2);
	DMA_InitStructure_USART3.DMA_PeripheralBaseAddr = (u32)(&(USART3->DR));//0x40013804;
	DMA_InitStructure_USART3.DMA_MemoryBaseAddr = (u32)(adc_formtx_tolcd_data_buf);
	DMA_InitStructure_USART3.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure_USART3.DMA_BufferSize = 8;
	DMA_InitStructure_USART3.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure_USART3.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure_USART3.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure_USART3.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure_USART3.DMA_Mode = DMA_Mode_Normal ; //DMA_Mode_Circular
	DMA_InitStructure_USART3.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStructure_USART3.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel2, &DMA_InitStructure_USART3);
	DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, ENABLE);	//DMA通道7传输完成中断
}
#endif


#if (!IS_USART2_USE_DMA)
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

	USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);
	USART_ITConfig(USART2,USART_IT_TXE,ENABLE);

	USART_Cmd(USART2, ENABLE);

}

#else
/***********************************************************
*   函数说明：串口参数配置函数                             *
*   输入：    无                                           *
*   输出：    无                                           *
*   调用函数：INSERT_UART_ISR_CODE                         *
***********************************************************/
static void init_usart2_cfg_with_dma_rx(void)
{
	USART_InitTypeDef 		USART_InitStructure;
	USART_ClockInitTypeDef 	USART_ClockInitStructure;
	USART_DeInit(USART2);
	//使能串口1，PA，AFIO总线
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	/* 串口参数配置  */
	USART_InitStructure.USART_BaudRate              = 2048000;
	USART_InitStructure.USART_WordLength            = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits              = USART_StopBits_1;
	USART_InitStructure.USART_Parity                = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl   = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode                  = USART_Mode_Rx | USART_Mode_Tx;
	USART_ClockInitStructure.USART_Clock    = USART_Clock_Disable;
	USART_ClockInitStructure.USART_CPOL     = USART_CPOL_Low;
	USART_ClockInitStructure.USART_CPHA     = USART_CPHA_2Edge;
	USART_ClockInitStructure.USART_LastBit  = USART_LastBit_Disable;
	USART_ClockInit(USART2, &USART_ClockInitStructure);
	USART_Init(USART2, &USART_InitStructure);
#if 0
	/* 串口2使能 */
	USART_Cmd(USART2, ENABLE);
	USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);
	USART_ITConfig(USART2,USART_IT_TXE,ENABLE);
#else
	/* 串口2使能 */
	//USART_Cmd(USART2, ENABLE);
	//将USART2模块设置成DMA方式工作	   接收设置为DMA方式
	USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
#endif
}

/***********************************************************
*   函数说明：串口1的DMA通道配置函数                       *
*   输入：    无                                           *
*   输出：    无                                           *
*   调用函数： 无                                          *
***********************************************************/
/*
   DMA,Channel6为串口2接收数据的DMA方式
*/


static void usart2_cfg_dma1_6_rx(void)
{
	DMA_DeInit(DMA1_Channel6);
	DMA_InitStructure_USART2.DMA_PeripheralBaseAddr = USART2_DR_ADDR;
	DMA_InitStructure_USART2.DMA_MemoryBaseAddr     = (u32)(uart2_rx_buf);
	DMA_InitStructure_USART2.DMA_DIR                = DMA_DIR_PeripheralSRC;
	DMA_InitStructure_USART2.DMA_BufferSize         = 2; //6;
	DMA_InitStructure_USART2.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
	DMA_InitStructure_USART2.DMA_MemoryInc          = DMA_MemoryInc_Enable;
	DMA_InitStructure_USART2.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure_USART2.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
	DMA_InitStructure_USART2.DMA_Mode     = DMA_Mode_Circular;//DMA_Mode_Circular; //DMA_Mode_Normal; /* mark by David */
	DMA_InitStructure_USART2.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure_USART2.DMA_M2M      = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel6, &DMA_InitStructure_USART2);
	DMA_ITConfig(DMA1_Channel6, DMA_IT_TC, ENABLE);			        		//DMA通道1传输完成中断
	//DMA_Cmd(DMA1_Channel6, ENABLE);	   /*使能DMA1的channel  6 */
}
#endif

static void Exti_Init(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource14);

	EXTI_InitStructure.EXTI_Line 	= EXTI_Line14;
	EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;

	EXTI_Init(&EXTI_InitStructure);

	//EXTI_GenerateSWInterrupt(EXTI_Line14);

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource14);

}


/***********************************************************
*   函数说明：系统端口初始化函数                           *
*   输入：    无                                           *
*   输出：    无                                           *
*   调用函数：无                                           *
***********************************************************/
static void PORT_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB
						   | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO , ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); /* DMA1 channel3 -- SPI1-TX configuration */

	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

	/* 零压降v2.0 接收端菊花链式DA控制管脚
	 * 以下为对DAC控制引脚       CSB(PA4),LDACB(PB12)
	 */
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/*USART1管脚配置*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; 		/* PA9    USART1_Tx */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		//推挽输出-TX
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//浮空输入-RX mark by David
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/*USART2管脚配置*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;		/* PA2    USART1_Tx */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		//推挽输出-TX
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//浮空输入-RX, mark by David
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/*USART3管脚配置*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;		  		/* PA2    USART1_Tx */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		//推挽输出-TX
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//浮空输入-RX, mark by David
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/*SPI1 PA7-MOSI1,PA5-SCK1,PA6-MISO1 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6| GPIO_Pin_4; //NSS 手动设置
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* pc13-控制是否给放大电路供电
	 * pc13：0--闭合， 1--断开
	 * PC13, PC14 and PC15 are supplied through the power switch. Since the switch only sinks a limited amount of
	 * current (3 mA), the use of GPIOs PC13 to PC15 in output mode is limited: the speed should not exceed 2 MHz
	 * with a maximum load of 30 pF and these IOs must not be used as a current source (e.g. to drive an LED)
	 */
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	turnoff_110v_power();

	/* 恒温盒内温度采集及ABC三相的均值
	 * ADC1, ADC_Channel_0->PA0->恒温盒内温度
	 * ADC1, ADC_Channel_1->PA1->C相平均值
	 * ADC1, ADC_Channel_8->PB0->B相平均值
	 * ADC1, ADC_Channel_9->PB1->A相平均值
	 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0| GPIO_Pin_1;  //PB0
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* 输出过限检测引脚 */
	/* !!NOTE: pa_too_heigh_gpio, pa_too_low_gpio, pb_too_heigh_gpio, pb_too_low_gpio,
	 * pc_too_heigh_gpio, pc_too_low_gpio都是GPIOB  */
	GPIO_InitStructure.GPIO_Pin = pa_too_heigh_pin | pa_too_low_pin | pb_too_heigh_pin | pb_too_low_pin
								  | pc_too_heigh_pin | pc_too_low_pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(px_too_lh_gpio, &GPIO_InitStructure);

	/* 当某相过高/过低时，该引脚输出0, 使继电器切换至pt侧 */
	GPIO_InitStructure.GPIO_Pin = px_falt_sw_pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(px_falt_sw_gpio, &GPIO_InitStructure);
	/* cancel_force_output_switch2pt(); */
	set_port_pin(px_falt_sw_gpio, px_falt_sw_pin);


	/* 已切换至pt侧电缆
	 *
	 * 未切换时, 持续为低
	 * 切换后, 50Hz正弦波的半波整流
	 * David
	 */
	GPIO_InitStructure.GPIO_Pin = switch2pt_indication_pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(switch2pt_indication_gpio, &GPIO_InitStructure);

#if 1  /* led */
	GPIO_InitStructure.GPIO_Pin = led1_pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(led1_gpio, &GPIO_InitStructure);
#else
	GPIO_InitStructure.GPIO_Pin = led2_pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(led3_gpio, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = led3_pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(led3_gpio, &GPIO_InitStructure);
#endif
	/* 检测过流 110电源-做过流保护 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);


#if 1
	/* 作为pwm信号输出管脚-对应timer2通道2  PB3 需进行Remap设置 */
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO , ENABLE);
//	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	GPIO_PinRemapConfig(GPIO_PartialRemap1_TIM2 , ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
#endif

#if USE_OPTICX_200S_VERSION
	GPIO_InitStructure.GPIO_Pin	= use_channel_1_indication_pin;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_2MHz;
	GPIO_Init(use_channel_1_indication_gpio, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin	= use_channel_2_indication_pin;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_2MHz;
	GPIO_Init(use_channel_2_indication_gpio, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin	= use_channel_x_ctrl_pin;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_2MHz;
	GPIO_Init(use_channel_x_ctrl_gpio, &GPIO_InitStructure);
	set_to_use_optical_fiber1(); /* 默认使用1#光纤头 */
#endif
	return;
}

#if 0
/*
 * 继电器切换到pt侧, 是常态, 该状态耗电很小
 */
void enable_protect_electric_relay_pin(unsigned int pin)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* 作为输出, 输出低电平时继电器切换到pt侧电缆 */
	GPIO_InitStructure.GPIO_Pin = pin ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(px_too_lh_gpio, &GPIO_InitStructure);

	return;
}

void disable_protect_electric_relay_pin(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;


	/* 输出过限检测引脚 */
	/* !!NOTE: pa_too_heigh_gpio, pa_too_low_gpio, pb_too_heigh_gpio, pb_too_low_gpio,
	 * pc_too_heigh_gpio, pc_too_low_gpio都是GPIOB  */
	GPIO_InitStructure.GPIO_Pin = pa_too_heigh_pin | pa_too_low_pin | pb_too_heigh_pin | pb_too_low_pin
								  | pc_too_heigh_pin | pc_too_low_pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(px_too_lh_gpio, &GPIO_InitStructure);

	return;
}
#endif

//=================ADC1+DMA1_Channel1的美丽开始========================
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
	ADC_InitStructure.ADC_NbrOfChannel = 4;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_Init(ADC1, &ADC_InitStructure);

	/*
	ADC1, ADC_Channel_0->PA0->恒温盒内温度
	ADC1, ADC_Channel_1->PA1->C相平均值
	ADC1, ADC_Channel_8->PB0->B相平均值
	ADC1, ADC_Channel_9->PB1->A相平均值

	ADC_SampleTime_71Cycles5
	       */
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0,  1, ADC_SampleTime_239Cycles5);  //恒温盒内温度
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1,  2, ADC_SampleTime_239Cycles5);  //PA1->C相平均值
	ADC_RegularChannelConfig(ADC1, ADC_Channel_8,  3, ADC_SampleTime_239Cycles5);  //PB0->B相平均值
	ADC_RegularChannelConfig(ADC1, ADC_Channel_9,  4, ADC_SampleTime_239Cycles5);  //PB1->A相平均值
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
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)adc_buf;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = 4;
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
	SystemInit();
	RCC_GetClocksFreq(&RCC_ClockFreq);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
}


/*
 * cmd: ENABLE, DISABLE
 */
void overload_exti_cfg(FunctionalState cmd)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = cmd;

	EXTI_ClearITPendingBit(EXTI_Line14);
	//EXTI->PR = EXTI_Line14;

	NVIC_Init(&NVIC_InitStructure);

	return;
}

void overload_exti_init(void)
{
	Exti_Init();
	overload_exti_cfg(ENABLE);

	return;
}

#if USE_PVD
static void Exti_Pvd_Init(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR,ENABLE);
	PWR_PVDLevelConfig(PWR_PVDLevel_2V9);
	PWR_PVDCmd(ENABLE);

	EXTI_InitStructure.EXTI_Line 	= EXTI_Line16;
	EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;
	/* pvd信号与电压信号相反 */
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	//EXTI_ClearITPendingBit(EXTI_Line16);
	EXTI->PR = EXTI_Line16;

	return;
}
#endif

