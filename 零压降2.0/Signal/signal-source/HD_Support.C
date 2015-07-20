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

#include "..\Source\FWLib\stm32f10x_exti.h"

/********************
*   系 统 宏 定 义  *
********************/



/*------------------*
*   常 数 宏 定 义  *
*------------------*/



/*------------------*
*   动 作 宏 定 义  *
*------------------*/

/********************
*  模块结构体定义区 *
********************/

/********************
*   函 数 声 明 区  *
********************/
void Device_Init(void);

/********************
*   模块函数声明区  *
********************/

static void RCC_Configuration(void);
static void PORT_Init(void);
static void USART1_Configuration(void); 
static void NVIC_Configuration(void);
static void TIM2_Configuration(void);
 	   void TIM3_Configuration(void);

void DMA_Configuration_SPI1_TX( void );
void SPI1_Configuration_withDMA( void );

//static  void IWDG_Configuration(void);
//	   void ADC1_Configuration(void);
	//   void DMA1_Configuration(void);

/********************
*   模块变量声明区  *
********************/

/********************
*   全局变量声明区  *
********************/
RCC_ClocksTypeDef RCC_ClockFreq;   //全局设备时钟提供商 ^_^


extern void get_sig_cfg_param(int is_init);
/***********************************************************
*   函数说明：系统硬件初始化函数                           *
*   输入：    无                                           *
*   输出：    无                                           *
*   调用函数：Port_Init()  RCC_Configuration()             *
***********************************************************/
void Device_Init(void)
{
    EXTI_InitTypeDef EXTI_InitStructure;

    RCC_Configuration();
    get_sig_cfg_param(1);

    PORT_Init();
 	USART1_Configuration();
	SPI1_Configuration_withDMA();
	DMA_Configuration_SPI1_TX();

 	TIM2_Configuration( );
    TIM3_Configuration( );

//	USART1_DMA_Configuration();
//	IWDG_Configuration();

#if 1 /* exti_2, David */
    /*
     * 将外部中断线与IO引脚连接起来
     */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource2);
    /*配置EXTI线0上出现下降沿，则产生中断*/
    EXTI_InitStructure.EXTI_Line    = EXTI_Line2;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //下降沿触发
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;   //中断线使能
    EXTI_Init(&EXTI_InitStructure);             //初始化中断
    //EXTI_GenerateSWInterrupt(EXTI_Line2);       //EXTI_Line2中断允许
#endif
    EXTI_ClearITPendingBit(EXTI_Line2);
	NVIC_Configuration(); 
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

	/**************************************************
	获取RCC的信息,调试用
	请参考RCC_ClocksTypeDef结构体的内容,当时钟配置完成后,
	里面变量的值就直接反映了器件各个部分的运行频率
	***************************************************/
	RCC_GetClocksFreq(&RCC_ClockFreq);
	
	/* 这个配置可使外部晶振停振的时候,产生一个NMI中断,不需要用的可屏蔽掉*/
	//RCC_ClockSecuritySystemCmd(ENABLE);

/*	
	//SYSTICK分频--1ms的系统时钟中断  
	//相当于配置预装载值 这个时钟是以72M为输入的
	// 72/(72/1000) = 1usX1000 = 1ms
*/

	if (SysTick_Config(SystemFrequency/1000/15 ))	//现在配置的是的中断周期1ms 用做定时启动ADC1
  	{ 
  	  	/* Capture error */ 
    	while (1);
  	}


	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);	//将复用功能 在最前面打开 牛X
} 	  

/***********************************************************
*   函数说明：独立看门狗初始化函数						   *
*   输入：    无                                           *
*   输出：    无                                           *
*   调用函数：                                             *
***********************************************************/
 
 
 

/***********************************************************
*   函数说明：定时器2初始化函数                *
*   输入：    无                                           *
*   输出：    无                                           *
*   调用函数：Port_Init()  RCC_Configuration()             *
***********************************************************/
static  void TIM2_Configuration(void)
{

	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	/* TIM3 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	/* ---------------------------------------------------------------
	TIM2CLK 即PCLK1=36MHz
	--------------------------------------------------------------- */
	
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Prescaler =6-1; 					//设置用来作为TIMx时钟频率除数的预分频值  不分频
	TIM_TimeBaseStructure.TIM_Period = 256-1; 						//设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 80K
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; 					//设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  	//TIM向上计数模式
	//根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); 
	


 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; 				//选择定时器模式:TIM脉冲宽度调制模式2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; 	//比较输出使能
	TIM_OCInitStructure.TIM_Pulse = 0; 							//设置待装入捕获比较寄存器的脉冲值
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; 		//输出极性:TIM输出比较极性低

	TIM_OC1Init(TIM2, &TIM_OCInitStructure);  						//根据TIM_OCInitStruct中指定的参数初始化外设TIMx
	TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);  				//使能TIMx在CCR2上的预装载寄存器

	TIM_ARRPreloadConfig(TIM2, ENABLE); 							//使能TIMx在ARR上的预装载寄存器

	/* TIM2 enable counter */
	TIM_Cmd(TIM2, ENABLE);  										//使能TIMx外设

	TIM_CtrlPWMOutputs(TIM2,ENABLE);
}


#if 0
//static void TIM2_Configuration(void)
//{
//	 //TIM2 的时间 作为差值的时间间隔	触发DMA 输出数据到DAC
//	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
//
//	/* TIM2 clock enable */
//  	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
//
//  	/* Time base configuration */
//  	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); 
//  	TIM_TimeBaseStructure.TIM_Period = 72*4; // 默认使用4us插值一次相当于         
//  	TIM_TimeBaseStructure.TIM_Prescaler = 0x0;       
//  	TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;    
//  	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
//  	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
//
//  	/* TIM2 TRGO selection */
//  	TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);
//
////	TIM_Cmd(TIM2, ENABLE);
//
// 
//}
#endif

static void TIM3_Configuration(void)
{
	TIM_TimeBaseInitTypeDef 	 TIM_TimeBaseStructure;
 
	
	/* TIM3 clock source enable 
	通用定时器3时钟使能*/					
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	/* Timer configuration in Encoder mode */
	TIM_DeInit(TIM3);		//将TIM3恢复到默认设置状态 
	
	/* Time Base configuration */
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); 
	TIM_TimeBaseStructure.TIM_Prescaler = 7200-1;    //0.1ms 分频 
	TIM_TimeBaseStructure.TIM_Period = 17000-1;    //定时1.7s       
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;  //配置 滤波用  
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
	
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	
	TIM_ARRPreloadConfig(TIM3, ENABLE);
 
  	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);	  //使能中断 
 
  	//TIM_Cmd(TIM3, ENABLE);  
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
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	

	/* DMA1 CHANNEL3 中断, SPI1_TX使用 */			
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel3_IRQn;		 
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#if 1 /* exti_2, David */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn; //中断通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;//强占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; //次优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //通道中断使能
    NVIC_Init(&NVIC_InitStructure);
#endif
    /* 打开 TIM3 Update 中断   */
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/*USART1*/
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}


/***********************************************************
*   函数说明：DMA 通道3 的SPI1的初始化函数                 *
*   输入：    无                                           *
*   输出：    无                                           *
*   调用函数：无                                           *
*   [说明]                                                 *
***********************************************************/
 //
void DMA_Configuration_SPI1_TX( void )
{
	 /*
	 说明：
	 		SPI1发送通道 DMA使用的是DMA1 ch3
			采用非循环模式进行
			24bit 的数据长度 8bit的单帧长度 bufferSize采用3
									即：    3X8 = 24；
	 */
	DMA_InitTypeDef  DMA_InitStructure;

  /* DMA1 channel3 --- SPI1-TX configuration */
   RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    DMA_DeInit(DMA1_Channel3);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&(SPI1->DR));
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)(&(SPI_DMA_Table[0]));
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = SIP_DMA_TBL_SIZE; /* David */
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;	//  	  	
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel3, &DMA_InitStructure);

    DMA_ITConfig(DMA1_Channel3, DMA_IT_TC, ENABLE);	//   DMA_IT_HT

  	/*DMA ENABLE*/
  //	DMA_Cmd(DMA1_Channel3,ENABLE);	 //DMA通道号 使能
 
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

 
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);


    /* SPI1 configuration */
	SPI_Cmd(SPI1, DISABLE); 	  //必须先禁能,才能改变MODE												
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;		//两线全双工
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;							//主
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;						//8位
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;								//CPOL=1 时钟悬空高	 
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;							//CPHA=0 数据捕获第一个↓
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;								//软件NSS  
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;		//2分频	   SPI_BaudRatePrescaler_2
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;						//高位在前
    SPI_InitStructure.SPI_CRCPolynomial = 7;								//CRC7
	SPI_Init(SPI1, &SPI_InitStructure);

	 /*Enable SPI1.NSS as a GPIO*/
	SPI_SSOutputCmd(SPI1, ENABLE);
 
	/*使能SPI 的发送DMA*/
    SPI_I2S_DMACmd(SPI1,SPI_I2S_DMAReq_Tx,ENABLE);

	 /*使能SPI*/
    SPI_Cmd(SPI1, ENABLE); 
}


 
/***********************************************************
*   函数说明：串口参数配置函数                             *
*   输入：    无                                           *
*   输出：    无                                           *
*   调用函数：INSERT_UART_ISR_CODE                         *
***********************************************************/
static void USART1_Configuration(void)
{
    USART_InitTypeDef 		USART_InitStructure;
	USART_ClockInitTypeDef 	USART_ClockInitStructure;
	
	//使能串口1，PA，AFIO总线
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO | RCC_APB2Periph_USART1, ENABLE);

	/* 串口参数配置  */
    USART_InitStructure.USART_BaudRate = 115200;	  					//波特率	   1024000*3
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;		//8位数据长度
	USART_InitStructure.USART_StopBits = USART_StopBits_1;			//一位停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;				//无校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//不使用硬件数据流
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//串口模式选择
	
	USART_ClockInitStructure.USART_Clock = USART_Clock_Disable;
	USART_ClockInitStructure.USART_CPOL = USART_CPOL_Low;
	USART_ClockInitStructure.USART_CPHA = USART_CPHA_2Edge;
	USART_ClockInitStructure.USART_LastBit = USART_LastBit_Disable;

	USART_ClockInit(USART1, &USART_ClockInitStructure);
    USART_Init(USART1, &USART_InitStructure);

    /* 串口1使能 */
    USART_Cmd(USART1, ENABLE);
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE); //允许串口1接收中断
	//USART_ITConfig(USART1,USART_IT_TXE,ENABLE);	 //允许串口1发送中断


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
		
		//PORTE 初始化
		RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOE|
								RCC_APB2Periph_GPIOA|
								RCC_APB2Periph_GPIOC|
								RCC_APB2Periph_GPIOB|
								RCC_APB2Periph_AFIO , ENABLE);
		
		//============LED指示灯管脚配置======================
		/* PE5    LED */
		GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6|GPIO_Pin_5 ;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOE, &GPIO_InitStructure);
 
		//===================================================================================
//UASRT1 	
		GPIO_StructInit(&GPIO_InitStructure);
		/*USART1管脚配置*/
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;		  		/* PA9    USART1_Tx */
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		//推挽输出-TX
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		
		//=====================================================
		GPIO_StructInit(&GPIO_InitStructure);
		//=====================================================
		GPIO_StructInit(&GPIO_InitStructure);    				/* PA10   USART1_Rx  */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//浮空输入-RX
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		//====================================================
#if 0		
//USART2 
		/*USART2管脚配置*/
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;		  		/* PA2    USART1_Tx */
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		//推挽输出-TX
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		
		//=====================================================
		GPIO_StructInit(&GPIO_InitStructure);
		//=====================================================
		GPIO_StructInit(&GPIO_InitStructure);    				/* PA3   USART1_Rx  */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入-RX
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		//====================================================
 
//===========ADC1管脚配置=============================  
		
		/* PC0-CH10 PC1-CH11 PC2-CH12 PC3-CH13     */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
		GPIO_Init(GPIOC, &GPIO_InitStructure);  
#endif		
		//===================================================
//SPI2
		/* PB15-MOSI2,PB13-SCK2*/
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 |GPIO_Pin_14 | GPIO_Pin_15;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; //NSS 手动设置
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

//SPI1
		/* PA7-MOSI1,PA5-SCK1,PA6-MISO1 */
	    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7| GPIO_Pin_4;
	    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	    GPIO_Init(GPIOA, &GPIO_InitStructure);

		//NSS1使用
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6| GPIO_Pin_4; //NSS 手动设置
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		
		//功能灯 指示灯 
		RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE);
		GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8|GPIO_Pin_9;
	  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	  	GPIO_Init(GPIOB, &GPIO_InitStructure);


		/* PA0设置为功能脚(PWM) */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
	
		/*PA1 控制继电器 选择加热OR制冷模式*/
		GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_1;
	  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	  	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
		GPIO_ResetBits(GPIOA, GPIO_Pin_1);  //初始化为低电平  PA1
	  	//GPIO_SetBits(GPIOA, GPIO_Pin_1);

#if 1 /* exti_2, David */
		/* PA2 过载检测, 正常负载高电平, 过载时为低电平 */
		GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_2;
	  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	  	GPIO_Init(GPIOA, &GPIO_InitStructure);
#endif
}





#if 0
//=================ADC+DMA的美丽开始========================
/***********************************************************
*   函数声明：ADC初始化函数                                *
*   输入：    无                                           *
*   输出：    无                                           *
*   调用函数：ST F.W. Ver3.0                               *
***********************************************************/   
static ADC_InitTypeDef 	ADC_InitStructure;
void ADC1_Configuration(void)
{
  	/* 允许ADC */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);

	//ADC1独立模式
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;		
	//扫描模式开启
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;		
	//连续扫描
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;		
	//采样的通道数
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	//软件启动转换
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;		
	//数据右对齐
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
						
	ADC_Init(ADC1, &ADC_InitStructure);

	
 ///////ADC1使能没有打开  等待上位机的命令 上位机触发的形式进行的
 	ADC_RegularChannelConfig(ADC1, ADC_Channel_16  , 1, ADC_SampleTime_41Cycles5);//一次2.16us   13/6	ADC_SampleTime_7Cycles5

    ADC_TempSensorVrefintCmd(ENABLE);  //测温度的 使能
 
	/* 允许ADC1的DMA模式 */
 	ADC_DMACmd(ADC1, ENABLE);
	/* 允许ADC1*/
 	ADC_Cmd(ADC1, ENABLE);
		
	/*重置校准寄存器 */   
	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1));
	
	/*开始校准状态*/
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));
	   
	/* 人工打开ADC转换.*/ 
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}


               
void ADC1_SampleTimeConfig(u8 SampleTime)
{	
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0 , 1, SampleTime);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1 , 2, SampleTime);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_2 , 3, SampleTime);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_3 , 4, SampleTime);
	
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10 , 5, SampleTime);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11 , 6, SampleTime);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12 , 7, SampleTime);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_13 , 8, SampleTime);
	
}


/***********************************************************
*   函数声明：DMA初始化函数                                *
*   输入：    无                                           *
*   输出：    无                                           *
*   调用函数：无                                           *
***********************************************************/
//DMA的配置	
static DMA_InitTypeDef DMA_InitStructure;

void DMA1_Configuration(void)
{
	//DMA_InitTypeDef DMA_InitStructure;
	/* 允许 DMA1 */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	/* DMA通道1*/
	DMA_DeInit(DMA1_Channel1);
	DMA_InitStructure.DMA_PeripheralBaseAddr =(u32)( &(ADC1->DR));			//ADC1数据寄存器
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)ADC_Val;					//获取ADC的数组
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;						//片内外设作源头

	DMA_InitStructure.DMA_BufferSize = 1;								//每次DMA中断数据数目

	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		//外设地址不增加
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					//内存地址增加
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	//半字
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;			//半字

//	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;							//普通模式
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;							//设置DMA的传输模式：连续不断的循环模式

	DMA_InitStructure.DMA_Priority = DMA_Priority_High;						//高优先级
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;							//非内存到内存

	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	
	DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);			        		//DMA通道1传输完成中断

	DMA_Cmd(DMA1_Channel1, ENABLE);	   /*使能DMA1的channel1 */
}

/***********************************************************
*   函数说明：DMA2初始化函数                               *
*   输入：    无                                           *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ：DMA2是用来传输DAC的数据的                       *
***********************************************************/
  
////=================ADC+DMA的美丽开始========================

#endif
