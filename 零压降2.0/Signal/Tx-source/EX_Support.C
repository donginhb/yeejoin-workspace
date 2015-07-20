/***********************************************************
*   函数库说明：底层硬件初始化驱动函数库                   *
*   版本：    	Ver1.0                                     *
*   作者：    	zzjjhh250/ZZJJHH250 @ (CopyRight)          *
*   创建日期：	08/31/2010                                 *
* -------------------------------------------------------- *
*  [硬件说明]                                              *
*   处理器：    STM32F103VBT6                              *
*   系统时钟：  外部8M/PLL = 72M                           *
* -------------------------------------------------------- *
*  [支 持 库]                                              *
*   支持库名称：PF_Config.h                                *
*   需要版本：  -----                                      *
*   声明库说明：硬件平台配置声明库                         *
*                                                          *
*   支持库名称：HD_Support.h                               *
*   需要版本：  -----                                      *
*   声明库说明：底层硬件初始化声明库                       *
*                                                          *
*   支持库名称：EX_Support.h                               *
*   需要版本：  -----                                      *
*   声明库说明：底层硬件初始化驱动声明库                   *
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
#include "..\Source\PF_Config.h"
#include "..\Source\EX_Support.h"
#include  "..\Source\LIB_Config.h"


/********************
*   系 统 宏 定 义  *
********************/

/*------------------*
*   常 数 宏 定 义  *
*------------------*/
#define USART1_RX_BUF_SIZE	64	/*必须是2的幂*/
#define USART1_TX_BUF_SIZE	64	/*必须是2的幂 2k*/ /* David */

#define USART1_RX_BUF_SIZE_B	64	/*必须是2的幂*/
#define USART1_TX_BUF_SIZE_B	64	/*必须是2的幂 2k*/


#define USART2_RX_BUF_SIZE	128	/*必须是2的幂*/
#define USART2_TX_BUF_SIZE	128	/*必须是2的幂 2k*/


/*------------------*
*   动 作 宏 定 义  *
*------------------*/
#define SPI_CS_RESET	GPIOE->BRR=GPIO_Pin_2
#define SPI_CS_SET		GPIOE->BSRR=GPIO_Pin_2

#define SPI_DI_SET		GPIOE->BSRR=GPIO_Pin_3
#define SPI_DI_RESET	GPIOE->BRR=GPIO_Pin_3

#define SPI_CLK_SET		GPIOE->BSRR=GPIO_Pin_4
#define SPI_CLK_RESET	GPIOE->BRR=GPIO_Pin_4
/********************
*  模块结构体定义区 *
********************/

/********************
*   函 数 声 明 区  *
********************/
void 	Delay_20ms(u32 speed);


void Driver_Init(void);

/********************
*   模块函数声明区  *
********************/
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void 	USART1_RxBufferInit(void);
u8 		USART1_PutDatatoRxBuffer(u8 dat);
u16 	USART1_GetRxBufferLeftLength(void);
u8 		USART1_GetRxBufferData( void );
u16 	USART1_GetRxBufferCurrentSize( void );
void    USART1_FlushRxBuffer( void );
void 	USART1_GetBytesFromRxFIFO( u8 *pdat ,u16 length);

////////////////////////////////////////////////////////
void 	USART1_RxBufferInit_B(void);
u8 		USART1_PutDatatoRxBuffer_B(u8 dat);
u16 	USART1_GetRxBufferLeftLength_B(void);
u8 		USART1_GetRxBufferData_B( void );
u16 	USART1_GetRxBufferCurrentSize_B( void );
void    USART1_FlushRxBuffer_B( void );
void 	USART1_GetBytesFromRxFIFO_B( u8 *pdat ,u16 length);
///////////////////////////////////////////////////////

void 	USART2_RxBufferInit(void);
u8 		USART2_PutDatatoRxBuffer(u8 dat);
u16 	USART2_GetRxBufferLeftLength(void);
u8 		USART2_GetRxBufferData( void );
u16 	USART2_GetRxBufferCurrentSize( void );
void    USART2_FlushRxBuffer( void );
void 	USART2_GetBytesFromRxFIFO( u8 *pdat ,u16 length);

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void 	USART1_TxBufferInit(void);
u8 		USART1_PutDatatoTxBuffer(u8 dat);
u16 	USART1_GetTxBufferLeftLength(void);
u8 		USART1_GetTxBufferData( void );
u16 	USART1_GetTxBufferCurrentSize( void );
void    USART1_FlushTxBuffer( void );
void 	USART1_PutBytesToTxFIFO( u8 *pdat ,u16 length);

void 	USART1_QueryPutMultiChar( u8 *pdat ,u16 length);
void 	USART1_QueryPutChar( u8 dat );

/*
///////////////////////////////////////////////////////
void 	USART1_TxBufferInit_B(void);
u8 		USART1_PutDatatoTxBuffer_B(u8 dat);
u16 	USART1_GetTxBufferLeftLength_B(void);
u8 		USART1_GetTxBufferData_B( void );
u16 	USART1_GetTxBufferCurrentSize_B( void );
void    USART1_FlushTxBuffer_B( void );
void 	USART1_PutBytesToTxFIFO_B( u8 *pdat ,u16 length);

void 	USART1_QueryPutMultiChar_B( u8 *pdat ,u16 length);
void 	USART1_QueryPutChar_B( u8 dat );
///////////////////////////////////////////////////////
*/

void 	USART2_TxBufferInit(void);
u8 		USART2_PutDatatoTxBuffer(u8 dat);
u16 	USART2_GetTxBufferLeftLength(void);
u8 		USART2_GetTxBufferData( void );
u16 	USART2_GetTxBufferCurrentSize( void );
void    USART2_FlushTxBuffer( void );

void 	USART2_PutBytesToTxFIFO( u8 *pdat ,u16 length);


void 	USART2_QueryPutMultiChar( u8 *pdat ,u16 length);
void 	USART2_QueryPutChar( u8 dat );

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void 	UASRT1_BeginSend(void);
void 	UASRT1_StopSend(void);

void 	UASRT2_BeginSend(void);
void 	UASRT2_StopSend(void);

void 	USART1_StopRx(void);
void 	USART1_BeginRx(void);

void 	USART2_StopRx(void);
void 	USART2_BeginRx(void);
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

/********************
*   模块变量声明区  *
********************/

static  u8 USART1_RXBuffer[USART1_RX_BUF_SIZE]; 	 //USART1 发送缓冲区FIFO
static  volatile cBuffer  USART1_tRXBufferMana;			 //管理结构体变量 David, 中断中可能会改变变量的值

static  u8 USART1_TXBuffer[USART1_TX_BUF_SIZE]; 	 //USART1 发送缓冲区FIFO
static  volatile cBuffer  USART1_tTXBufferMana;	 		 //管理结构体变量 David, 中断中可能会改变变量的值


#if 0
static u8 USART1_RXBuffer_B[USART1_RX_BUF_SIZE_B]; 	 //USART1 发送缓冲区FIFO
static cBuffer  USART1_tRXBufferMana_B;			 //管理结构体变量

static u8 USART1_TXBuffer_B[USART1_TX_BUF_SIZE_B]; 	 //USART1 发送缓冲区FIFO
static cBuffer  USART1_tTXBufferMana_B;	 		 //管理结构体变量
#endif

static u8 USART2_RXBuffer[USART2_RX_BUF_SIZE]; 	 //USART2 发送缓冲区FIFO
static cBuffer  USART2_tRXBufferMana;			 //管理结构体变量

static u8 USART2_TXBuffer[USART2_TX_BUF_SIZE]; 	 //USART2 发送缓冲区FIFO
static cBuffer  USART2_tTXBufferMana;	 		 //管理结构体变量

/********************
*   全局变量声明区  *
********************/
volatile bool  UARTBufferFlag = FALSE ;
volatile bool  UART_TXFlag = FALSE ;
volatile bool  UARTNewConfigFlag = FALSE ;

u8 UART1_SendBuff1[8];
u8 UART1_SendBuff2[8];

volatile u8 uart1_dma_buf[UART1_DMA_BUF_LEN];

volatile u16 adc_buf4temper[2];
volatile u32 is_had_enter_adjust_value_mode;
volatile u32  pa_avg_val_max;
volatile u32  pa_avg_val_min;
volatile u32  pa_lost_avg_val;

volatile u32  pb_avg_val_max;
volatile u32  pb_avg_val_min;
volatile u32  pb_lost_avg_val;

volatile u32  pc_avg_val_max;
volatile u32  pc_avg_val_min;
volatile u32  pc_lost_avg_val;



/***********************************************************
*   函数说明：数字电位器A   SPI写函数                      *
*   输入：    输入的数据-0-255 ；      *
*   输出：    无                                           *
*   调用函数：无                                           *
***********************************************************/
void WritePOT_A(u8 dat )
{
	UINT16 data = (UINT16)(dat)&0xfeff;
	UINT8 temp;
	SPI_CS_RESET;
	__nop();
	for(temp=0; temp<9; temp++) {
		if((data & 0x0100)) { //先写数据
			SPI_DI_SET;		 //写1
			__nop();
		} else {
			SPI_DI_RESET;  //写0
			__nop();
		}

		SPI_CLK_SET;  //再写CLK
		__nop();
		SPI_CLK_RESET;
		__nop();

		data <<= 1;
		__nop();
	}
	SPI_CS_SET;
	__nop();

}


/***********************************************************
*   函数说明：数字电位器B   SPI写函数                      *
*   输入：    输入的数据-0-255 ；  选择的内阻0-A ; 1-B     *
*   输出：    无                                           *
*   调用函数：无                                           *
***********************************************************/
void WritePOT_B(u8 dat )
{
	UINT16 data = (UINT16)(0x0001<<8 | dat);
	UINT8 temp;
	SPI_CS_RESET;
	__nop();
	for(temp=0; temp<9; temp++) {
		if((data & 0x0100)) { //先写数据
			SPI_DI_SET;		 //写1
			__nop();
		} else {
			SPI_DI_RESET;  //写0
			__nop();
		}

		SPI_CLK_SET;  //再写CLK
		__nop();
		SPI_CLK_RESET;
		__nop();

		data <<= 1;
		__nop();
	}
	SPI_CS_SET;
	__nop();

}


/***********************************************************
*   函数说明：数字电位器SPI初始化函数                      *
*   输入：    无                                           *
*   输出：    无                                           *
*   调用函数：无                                           *
***********************************************************/
void DigitalPOT_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;


	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);


	/* PE2-CS PE3-DI PE4-CLK*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 |GPIO_Pin_3 | GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	//信号预置
	SPI_CS_SET;
	SPI_CLK_RESET;

}


//设置捕获寄存器1
void SetT1Pwm1(u16 pulse ,u8 num)
{
	switch(num) {
	case 1:
		TIM1->CCR1=pulse;
		break;
	case 2:
		TIM1->CCR2=pulse;
		break;
	case 3:
		TIM1->CCR3=pulse;
		break;
	case 4:
		TIM1->CCR4=pulse;
		break;
	default :
		;
	}
}


void SpeedTIM2_Enable(void)
{
	TIM_Cmd(TIM2, ENABLE);
}

void SpeedTIM2_Disable(void)
{
	TIM_Cmd(TIM2, DISABLE);
}


void ENC_Disable(void)
{
	TIM_Cmd(TIM4, DISABLE);
	TIM_Cmd(TIM3, DISABLE);
	TIM_Cmd(TIM1, DISABLE);
}

void ENC_Enable(void)
{
	TIM_Cmd(TIM4, ENABLE);
	TIM_Cmd(TIM3, ENABLE);
	TIM_Cmd(TIM1, ENABLE);
}

void TIM3_ADC1Trigger_Reconfig(void)
{
	TIM_TimeBaseInitTypeDef 	 TIM_TimeBaseStructure;

	/* TIM3 clock source enable
	通用定时器3时钟使能*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	/* Timer configuration in Encoder mode */
	TIM_DeInit(TIM3);		//将TIM3恢复到默认设置状态

	/* Time Base configuration */
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Period = 36*2*3;    //定时 用的 以3us
	TIM_TimeBaseStructure.TIM_Prescaler = 0;    //0分频
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;  //配置 滤波用
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	TIM_ARRPreloadConfig(TIM3, ENABLE);

	// Clear all pending interrupts
	TIM_ClearFlag(TIM3, TIM_FLAG_Update);


	//作为普通定时器使用 用来触发 ADC1
//	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

	/* 选择TIM3溢出的输出事件   */
	TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);

//	TIM_Cmd(TIM3, ENABLE);
}


/***********************************************************
*   函数说明：数字滤波函数                                 *
*   输入：    1.半字长度数据指针 2.要处理数据的长度        *
*   输出：    无                                           *
*   调用函数：无										   *
*   说明:     取NO的2/5作为头尾忽略值,注意N要大于5,否则	   *
			  不会去头尾                                   *
***********************************************************/
u16 DigitFilter(u16* buf,u8 no)
{
	u8 i,j;
	u16 tmp;
	u8 cut_no=0;
	//排序
	for(i=0; i<no; i++) {
		for(j=0; j<no-i-1; j++) {

			if(buf[j]>buf[j+1]) {
				tmp=buf[j];
				buf[j]=buf[j+1];
				buf[j+1]=tmp;
			}
		}
	}

	if(no>5) {
		cut_no=no/5;
	}

	//去头去尾取平均
	tmp=0;
	for(i=cut_no; i<no-cut_no; i++)	//只取中间n-2*cut_no个求平均
		tmp+=buf[i];
	return(tmp/(no-2*cut_no));
}

/***********************************************************
*   函数说明：软件驱动初始化函数                           *
*   输入：    无                                           *
*   输出：    无                                           *
*   调用函数：无                                           *
***********************************************************/

void Driver_Init(void)
{
	USART1_RxBufferInit();
	USART1_TxBufferInit();

	USART2_RxBufferInit();
	USART2_TxBufferInit();
	// DigitalPOT_Config();//初始化 外设 数字电位器
}


/***********************************************************
*   函数说明：系统延时函数                                 *
*   输入：    20MS延时的基准输入                           *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ：20ms                                            *
***********************************************************/
void Delay_20ms(u32 speed)
{
	u16 i;
	while(speed!=0) {
		speed--;
		for(i=0; i<400; i++);
	}
}


/* ==========================================================
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// *********************************************************
//以下是将通用FIFO缓冲区管理的函数具体配置到USART中		   *
//总共有两个静态的缓冲区，分别用来接收和发送       		   *
// *********************************************************
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
===========================================================*/

/***********************************************************
*   函数名：USART1_RxBufferInit                            *
*   输入：    无                         				   *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ：                                                *
***********************************************************/
void USART1_RxBufferInit(void)
{
	bufferInit(&USART1_tRXBufferMana, USART1_RXBuffer, USART1_RX_BUF_SIZE);
}

/***********************************************************
*   函数名：USART1_PutDatatoRxBuffer                       *
*   输入：    要存入FIFO的数据（注意是一个Byte大小）	   *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ：                                                *
***********************************************************/
u8 USART1_PutDatatoRxBuffer(u8 dat)
{
	return ( bufferAddToEnd(&USART1_tRXBufferMana,dat ));
}

/*************************************************
**函数名:USART1GetRxBufLen
**功能:获取缓冲中有效数据的长度
**注意事项:获取的值必然为最小值,因为真实长度会不断变化.由于是32位的ARM系统,获取32位数据不存在临界问题,所以可以不考虑关中断
**************************************************/
u16 USART1_GetRxBufferLeftLength(void)
{
	return(bufferIsNotFull(&USART1_tRXBufferMana));
}

/**************************************************
**函数名:USART1_GetRxBufferData
**功能:从队列中获取数据
**注意事项:
**************************************************/
u8 USART1_GetRxBufferData( void )
{
	return (bufferGetFromFront(&USART1_tRXBufferMana));
}

/***********************************************************
*   函数名：USART1_FlushRxBuffer                           *
*   输入：                                          	   *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ： 接收缓冲区清空                                 *
***********************************************************/
void USART1_FlushRxBuffer( void )
{
	bufferFlush(&USART1_tRXBufferMana) ;
}

/***********************************************************
*   函数名：USART1_GetRxBufferCurrentSize                  *
*   输入：    无                                      	   *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ： 获得当前数据区的大小                           *
***********************************************************/
u16 USART1_GetRxBufferCurrentSize( void )
{
	return (bufferGetSize(&USART1_tRXBufferMana)) ;
}

/***********************************************************
*   函数名：  USART1_GetBytesFromRxFIFO                    *
*   输入：    1. 从当前的接收缓冲区获取的数据的存放指针	   *
*			  2. 获得的数据的长度                     	   *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ：                                                *
***********************************************************/
void USART1_GetBytesFromRxFIFO( u8 *pdat ,u16 length)
{
	u16 num = 0;
	while(num < length) {
		*(pdat++) = USART1_GetRxBufferData();
		num++;
	}
}


//////////////////////////////////////////
//	以下函数为发送相关
//////////////////////////////////////////
/**************************************************************
接收区缓冲区初始化函数
***************************************************************/
void USART1_TxBufferInit(void)
{
	bufferInit(&USART1_tTXBufferMana, USART1_TXBuffer, USART1_TX_BUF_SIZE);
}

/**************************************************************
** 函数名:USART1PutDatatoTxBuf
** 功能:把数据放进发送队列中,
** 注意事项:用户需要有数据发的时候使用
***************************************************************/
u8 USART1_PutDatatoTxBuffer(u8 dat)
{
	return ( bufferAddToEnd(&USART1_tTXBufferMana,dat ));
}

/*************************************************
**函数名:USART1GetTxBufLen
**功能:获取缓冲中有效数据的长度
**注意事项:获取的值必然为最小值,因为真实长度会不断变化.由于是32位的ARM系统,获取32位数据不存在临界问题,所以可以不考虑关中断
**			所谓有效,是指剩余的可用长度
**************************************************/
u16 USART1_GetTxBufferLeftLength(void)
{
	return(bufferIsNotFull(&USART1_tTXBufferMana));
}

/**************************************************
**函数名:USART1GetTxBufDat
**功能:从队列中获取数据
**注意事项:调用此函数前请先确保队列中有数据!!
**************************************************/
u8 USART1_GetTxBufferData( void )
{
	return (bufferGetFromFront(&USART1_tTXBufferMana));
}

/***********************************************************
*   函数名：USART1_FlushTxBuffer                           *
*   输入：   无 	                                       *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ：                                                *
***********************************************************/
void USART1_FlushTxBuffer( void )
{
	bufferFlush(&USART1_tTXBufferMana) ;
}

/***********************************************************
*   函数名：USART1_QueryPutChar                            *
*   输入：  要发送的数据（注意是一个Byte大小）	           *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ： 此串口操作时以查询的方式发送数据               *
***********************************************************/
u16 USART1_GetTxBufferCurrentSize( void )
{
	return (bufferGetSize(&USART1_tTXBufferMana)) ;
}

/********************************************************
**函数名:USART1_BeginSend
**功能:启动发送
**注意事项:这里使用空中断方式发送,只要发送寄存器为空,则会进入发送空中断,系统再在中断中进行发送
********************************************************/
void UASRT1_BeginSend(void)
{
	USART_ITConfig(USART1,USART_IT_TXE,ENABLE);
}

/*******************************************************
**函数名:USART1_StopSend
**功能:启动发送
**注意事项:这里使用空中断方式发送,只要发送寄存器为空,则会进入发送空中断,系统再在中断中进行发送
********************************************************/
void UASRT1_StopSend(void)
{
	USART_ITConfig(USART1,USART_IT_TXE,DISABLE);
}

/***********************************************************
*   函数名：USART1_QueryPutChar                            *
*   输入：  要发送的数据（注意是一个Byte大小）	           *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ： 此串口操作时以查询的方式发送数据               *
***********************************************************/
void USART1_QueryPutChar( u8 dat )
{

	USART_SendData(USART1 , (u8)(dat));
	while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);

}

/***********************************************************
*   函数名：USART1_QueryPutMultiChar                       *
*   输入：  1.要发送的数据的指针 2.要发送数据的长度        *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ： 此串口操作时以查询的方式发送数据               *
***********************************************************/
void USART1_QueryPutMultiChar( u8 *pdat ,u16 length)
{
	u16 num = 0;
	while(num++ < length) {
		USART1_QueryPutChar((u8)(*(pdat++)));
	}
}

/***********************************************************
*   函数名：USART1_PutBytesToTxFIFO                        *
*   输入：  1.要发送的数据的指针 2.要发送数据的长度        *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ： 把数据串的注入到发送FIFO                       *
***********************************************************/
void USART1_PutBytesToTxFIFO( u8 *pdat ,u16 length)
{
	u16 num = 0;
	while(num++ < length) {
		USART1_PutDatatoRxBuffer((u8)(*(pdat++)));
	}
}















#if 0
/////////////////////////UART1_B////////////////////////////////////////////////////
/***********************************************************
*   函数名：USART1_RxBufferInit                            *
*   输入：    无                         				   *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ：                                                *
***********************************************************/
void USART1_RxBufferInit_B(void)
{
	bufferInit(&USART1_tRXBufferMana_B, USART1_RXBuffer_B, USART1_RX_BUF_SIZE_B);
}

/***********************************************************
*   函数名：USART1_PutDatatoRxBuffer                       *
*   输入：    要存入FIFO的数据（注意是一个Byte大小）	   *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ：                                                *
***********************************************************/
u8 USART1_PutDatatoRxBuffer_B(u8 dat)
{
	return ( bufferAddToEnd(&USART1_tRXBufferMana_B,dat ));
}

/*************************************************
**函数名:USART1GetRxBufLen
**功能:获取缓冲中有效数据的长度
**注意事项:获取的值必然为最小值,因为真实长度会不断变化.由于是32位的ARM系统,获取32位数据不存在临界问题,所以可以不考虑关中断
**************************************************/
u16 USART1_GetRxBufferLeftLength_B(void)
{
	return(bufferIsNotFull(&USART1_tRXBufferMana_B));
}

/**************************************************
**函数名:USART1_GetRxBufferData
**功能:从队列中获取数据
**注意事项:
**************************************************/
u8 USART1_GetRxBufferData_B( void )
{
	return (bufferGetFromFront(&USART1_tRXBufferMana_B));
}

/***********************************************************
*   函数名：USART1_FlushRxBuffer                           *
*   输入：                                          	   *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ： 接收缓冲区清空                                 *
***********************************************************/
void USART1_FlushRxBuffer_B( void )
{
	bufferFlush(&USART1_tRXBufferMana_B) ;
}

/***********************************************************
*   函数名：USART1_GetRxBufferCurrentSize                  *
*   输入：    无                                      	   *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ： 获得当前数据区的大小                           *
***********************************************************/
u16 USART1_GetRxBufferCurrentSize_B( void )
{
	return (bufferGetSize(&USART1_tRXBufferMana_B)) ;
}

/***********************************************************
*   函数名：  USART1_GetBytesFromRxFIFO                    *
*   输入：    1. 从当前的接收缓冲区获取的数据的存放指针	   *
*			  2. 获得的数据的长度                     	   *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ：                                                *
***********************************************************/
void USART1_GetBytesFromRxFIFO_B( u8 *pdat ,u16 length)
{
	u16 num = 0;
	while(num < length) {
		*(pdat++) = USART1_GetRxBufferData_B();
		num++;
	}
}


//////////////////////////////////////////
//	以下函数为发送相关
//////////////////////////////////////////
/**************************************************************
接收区缓冲区初始化函数
***************************************************************/
void USART1_TxBufferInit_B(void)
{
	bufferInit(&USART1_tTXBufferMana_B, USART1_TXBuffer_B, USART1_TX_BUF_SIZE_B);
}

/**************************************************************
** 函数名:USART1PutDatatoTxBuf
** 功能:把数据放进发送队列中,
** 注意事项:用户需要有数据发的时候使用
***************************************************************/
u8 USART1_PutDatatoTxBuffer_B(u8 dat)
{
	return ( bufferAddToEnd(&USART1_tTXBufferMana_B,dat ));
}

/*************************************************
**函数名:USART1GetTxBufLen
**功能:获取缓冲中有效数据的长度
**注意事项:获取的值必然为最小值,因为真实长度会不断变化.由于是32位的ARM系统,获取32位数据不存在临界问题,所以可以不考虑关中断
**			所谓有效,是指剩余的可用长度
**************************************************/
u16 USART1_GetTxBufferLeftLength_B(void)
{
	return(bufferIsNotFull(&USART1_tTXBufferMana_B));
}

/**************************************************
**函数名:USART1GetTxBufDat
**功能:从队列中获取数据
**注意事项:调用此函数前请先确保队列中有数据!!
**************************************************/
u8 USART1_GetTxBufferData_B( void )
{
	return (bufferGetFromFront(&USART1_tTXBufferMana_B));
}

/***********************************************************
*   函数名：USART1_FlushTxBuffer                           *
*   输入：   无 	                                       *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ：                                                *
***********************************************************/
void USART1_FlushTxBuffer_B( void )
{
	bufferFlush(&USART1_tTXBufferMana_B) ;
}

/***********************************************************
*   函数名：USART1_QueryPutChar                            *
*   输入：  要发送的数据（注意是一个Byte大小）	           *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ： 此串口操作时以查询的方式发送数据               *
***********************************************************/
u16 USART1_GetTxBufferCurrentSize_B( void )
{
	return (bufferGetSize(&USART1_tTXBufferMana_B)) ;
}


/***********************************************************
*   函数名：USART1_QueryPutChar                            *
*   输入：  要发送的数据（注意是一个Byte大小）	           *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ： 此串口操作时以查询的方式发送数据               *
***********************************************************/
void USART1_QueryPutChar_B( u8 dat )
{

	USART_SendData(USART1 , (u8)(dat));
	while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);

}

/***********************************************************
*   函数名：USART1_QueryPutMultiChar                       *
*   输入：  1.要发送的数据的指针 2.要发送数据的长度        *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ： 此串口操作时以查询的方式发送数据               *
***********************************************************/
void USART1_QueryPutMultiChar_B( u8 *pdat ,u16 length)
{
	u16 num = 0;
	while(num++ < length) {
		USART1_QueryPutChar_B((u8)(*(pdat++)));
	}
}

/***********************************************************
*   函数名：USART1_PutBytesToTxFIFO                        *
*   输入：  1.要发送的数据的指针 2.要发送数据的长度        *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ： 把数据串的注入到发送FIFO                       *
***********************************************************/
void USART1_PutBytesToTxFIFO_B( u8 *pdat ,u16 length)
{
	u16 num = 0;
	while(num++ < length) {
		USART1_PutDatatoRxBuffer_B((u8)(*(pdat++)));
	}
}
/////////////////////////UART1_B////////////////////////////////////////////////////
#endif

/***********************************************************
*   函数说明：串口停止接收函数                             *
*   输入：    无                                           *
*   输出：    无                                           *
*   调用函数：ST.F.W.3.0                                   *
***********************************************************/
void USART2_StopRx(void)
{
	USART_ITConfig(USART2,USART_IT_RXNE,DISABLE);
}

/***********************************************************
*   函数说明：串口开始接收使能函数                         *
*   输入：    无                                           *
*   输出：    无                                           *
*   调用函数：ST.F.W.3.0                                   *
***********************************************************/
void USART2_BeginRx(void)
{
	USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);
}




//======================USART2管理函数开始====================
/***********************************************************
*   函数名：USART1_RxBufferInit                            *
*   输入：    无                         				   *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ：                                                *
***********************************************************/
void USART2_RxBufferInit(void)
{
	bufferInit(&USART2_tRXBufferMana, USART2_RXBuffer, USART2_RX_BUF_SIZE);
}

/***********************************************************
*   函数名：USART1_PutDatatoRxBuffer                       *
*   输入：    要存入FIFO的数据（注意是一个Byte大小）	   *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ：                                                *
***********************************************************/
u8 USART2_PutDatatoRxBuffer(u8 dat)
{
	return ( bufferAddToEnd(&USART2_tRXBufferMana,dat ));
}

/*************************************************
**函数名:USART1GetRxBufLen
**功能:获取缓冲中有效数据的长度
**注意事项:获取的值必然为最小值,因为真实长度会不断变化.由于是32位的ARM系统,获取32位数据不存在临界问题,所以可以不考虑关中断
**************************************************/
u16 USART2_GetRxBufferLeftLength(void)
{
	return(bufferIsNotFull(&USART2_tRXBufferMana));
}

/**************************************************
**函数名:USART1_GetRxBufferData
**功能:从队列中获取数据
**注意事项:
**************************************************/
u8 USART2_GetRxBufferData( void )
{
	return (bufferGetFromFront(&USART2_tRXBufferMana));
}

/***********************************************************
*   函数名：USART1_FlushRxBuffer                           *
*   输入：                                          	   *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ： 接收缓冲区清空                                 *
***********************************************************/
void USART2_FlushRxBuffer( void )
{
	bufferFlush(&USART2_tRXBufferMana) ;
}

/***********************************************************
*   函数名：USART1_GetRxBufferCurrentSize                  *
*   输入：    无                                      	   *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ： 获得当前数据区的大小                           *
***********************************************************/
u16 USART2_GetRxBufferCurrentSize( void )
{
	return (bufferGetSize(&USART2_tRXBufferMana)) ;
}

/***********************************************************
*   函数名：  USART1_GetBytesFromRxFIFO                    *
*   输入：    1. 从当前的接收缓冲区获取的数据的存放指针	   *
*			  2. 获得的数据的长度                     	   *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ：                                                *
***********************************************************/
void USART2_GetBytesFromRxFIFO( u8 *pdat ,u16 length)
{
	u16 num = 0;
	while(num < length) {
		*(pdat++) = USART2_GetRxBufferData();
		num++;
	}
}


//////////////////////////////////////////
//	以下函数为发送相关
//////////////////////////////////////////
/**************************************************************
接收区缓冲区初始化函数
***************************************************************/
void USART2_TxBufferInit(void)
{
	bufferInit(&USART2_tTXBufferMana, USART2_TXBuffer, USART2_TX_BUF_SIZE);
}

/**************************************************************
** 函数名:USART1PutDatatoTxBuf
** 功能:把数据放进发送队列中,
** 注意事项:用户需要有数据发的时候使用
***************************************************************/
u8 USART2_PutDatatoTxBuffer(u8 dat)
{
	return ( bufferAddToEnd(&USART2_tTXBufferMana,dat ));
}

/*************************************************
**函数名:USART1GetTxBufLen
**功能:获取缓冲中有效数据的长度
**注意事项:获取的值必然为最小值,因为真实长度会不断变化.由于是32位的ARM系统,获取32位数据不存在临界问题,所以可以不考虑关中断
**			所谓有效,是指剩余的可用长度
**************************************************/
u16 USART2_GetTxBufferLeftLength(void)
{
	return(bufferIsNotFull(&USART2_tTXBufferMana));
}

/**************************************************
**函数名:USART1GetTxBufDat
**功能:从队列中获取数据
**注意事项:调用此函数前请先确保队列中有数据!!
**************************************************/
u8 USART2_GetTxBufferData( void )
{
	return (bufferGetFromFront(&USART2_tTXBufferMana));
}

/***********************************************************
*   函数名：USART1_FlushTxBuffer                           *
*   输入：   无 	                                       *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ：                                                *
***********************************************************/
void USART2_FlushTxBuffer( void )
{
	bufferFlush(&USART2_tTXBufferMana) ;
}

/***********************************************************
*   函数名：USART1_QueryPutChar                            *
*   输入：  要发送的数据（注意是一个Byte大小）	           *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ： 此串口操作时以查询的方式发送数据               *
***********************************************************/
u16 USART2_GetTxBufferCurrentSize( void )
{
	return (bufferGetSize(&USART2_tTXBufferMana)) ;
}

/********************************************************
**函数名:USART1_BeginSend
**功能:启动发送
**注意事项:这里使用空中断方式发送,只要发送寄存器为空,则会进入发送空中断,系统再在中断中进行发送
********************************************************/
void UASRT2_BeginSend(void)
{
	USART_ITConfig(USART2,USART_IT_TXE,ENABLE);
}

/*******************************************************
**函数名:USART1_StopSend
**功能:启动发送
**注意事项:这里使用空中断方式发送,只要发送寄存器为空,则会进入发送空中断,系统再在中断中进行发送
********************************************************/
void UASRT2_StopSend(void)
{
	USART_ITConfig(USART2,USART_IT_TXE,DISABLE);
}

/***********************************************************
*   函数名：USART1_QueryPutChar                            *
*   输入：  要发送的数据（注意是一个Byte大小）	           *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ： 此串口操作时以查询的方式发送数据               *
***********************************************************/
void USART2_QueryPutChar( u8 dat )
{

	USART_SendData(USART2 , (u8)(dat));
	while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);

}

/***********************************************************
*   函数名：USART1_QueryPutMultiChar                       *
*   输入：  1.要发送的数据的指针 2.要发送数据的长度        *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ： 此串口操作时以查询的方式发送数据               *
***********************************************************/
void USART2_QueryPutMultiChar( u8 *pdat ,u16 length)
{
	u16 num = 0;
	while(num++ < length) {
		USART2_QueryPutChar((u8)(*(pdat++)));
	}
}

/***********************************************************
*   函数名：USART1_PutBytesToTxFIFO                        *
*   输入：  1.要发送的数据的指针 2.要发送数据的长度        *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ： 把数据串的注入到发送FIFO                       *
***********************************************************/
void USART2_PutBytesToTxFIFO( u8 *pdat ,u16 length)
{
	u16 num = 0;
	while(num++ < length) {
		USART2_PutDatatoRxBuffer((u8)(*(pdat++)));
	}
}








