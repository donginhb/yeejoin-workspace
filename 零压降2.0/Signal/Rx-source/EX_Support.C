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
# include "..\Source\PF_Config.h"
# include "..\Source\EX_Support.h"
#include  "..\Source\LIB_Config.h"


/********************
*   系 统 宏 定 义  *
********************/

/*------------------*
*   常 数 宏 定 义  *
*------------------*/
#define USART1_RX_BUF_SIZE	64	/*必须是2的幂*/
#define USART1_TX_BUF_SIZE	64	/*必须是2的幂 2k*/


#define USART2_RX_BUF_SIZE	128	/*必须是2的幂*/
#define USART2_TX_BUF_SIZE	128	/*必须是2的幂 2k*/

/*------------------*
*   动 作 宏 定 义  *
*------------------*/

/********************
*  模块结构体定义区 *
********************/

/********************
*   函 数 声 明 区  *
********************/
void 	Delay_20ms(u32 speed);


//void Driver_Init(void);

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
void USART3_RxBufferInit(void);
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

/********************
*   模块变量声明区  *
********************/

static u8 USART1_RXBuffer[USART1_RX_BUF_SIZE]; 	 //USART1 发送缓冲区FIFO
static volatile cBuffer  USART1_tRXBufferMana;	 //管理结构体变量 David, 中断中可能会改变变量的值

static u8 USART1_TXBuffer[USART1_TX_BUF_SIZE]; 	 //USART1 发送缓冲区FIFO
static volatile cBuffer  USART1_tTXBufferMana;	 //管理结构体变量 David, 中断中可能会改变变量的值


static u8 USART2_RXBuffer[USART2_RX_BUF_SIZE]; 	 //USART2 发送缓冲区FIFO
static cBuffer  USART2_tRXBufferMana;			 //管理结构体变量

static u8 USART2_TXBuffer[USART2_TX_BUF_SIZE]; 	 //USART2 发送缓冲区FIFO
static cBuffer  USART2_tTXBufferMana;	 		 //管理结构体变量



/********************
*   全局变量声明区  *
********************/

#if 0
u16 g_wSysTime1 = 0;   //系统定时
u16 g_wSysTime2 = 0;
u16 g_wSysTime3 = 0;
u16 g_wSysTime4 = 0;

u16 counter1=0;
u16 counter2=1;
u16 counter3=2;
u16 counter4=3;

uc16 Sine12bit[32] = {
	0x7FF,0x98E,0xB0F,0xC71,0xDA7,0xEA5,
	0xF63,0xFD7,0xFFE,0xFD7,0xF63,0xEA5,
	0xDA7,0xC71,0xB0F,0x98E ,0x7FF,0x670,
	0x4EF,0x38D,0x257,0x159,0x09B,0x027,
	0x000,0x027,0x09B,0x159,0x257,0x38D,
	0x4EF,0x670
}	;

union32_type wInArrayTab[128];  //固定25个点 串口2传输下来的时间 需要*25 相当于
//默认的情况下向后推移了25*4 = 100us
//DMA固定传输25个数据点 这样的话 时间不能长
//推出了 下一个点 意义就不大了
u8 USART1_DMA_Buf1[8] = {0};
u8 cSineWaveIndex = 0;
#endif

volatile u8 SPI_DMA_Table_serial_in[6];
volatile u8 SPI_DMA_Table_serial_in_zero[6]= {0x7f,0xff,0x7f,0xff,0x7f,0xff};
volatile u16 adc_buf[4]= {0 , 0 , 0, 0};
volatile u8 adc_formtx_tolcd_data_buf[8]= {0 , 0 , 0, 0, 0, 0, 0, 0};


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

	USART3_RxBufferInit();
}


#if 0
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
*   函数说明：SPI2发送一个字节的数据                       *
*   输入：    无                                           *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明：	  切记不可忽略SPI的硬件接收,因为读SPI_DR才能清除RXEN
***********************************************************/
u8 SPI1SendByte(u8 byte)
{
	/*等待发送寄存器空*/
	while((SPI1->SR & SPI_I2S_FLAG_TXE)==RESET);
	/*发送一个字节*/
	SPI2->DR = byte;
	/* 等待接收寄存器有效*/
	while( (SPI1->SR & SPI_I2S_FLAG_RXNE)==RESET );
	return(SPI1->DR);
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


/***********************************************************
*   函数说明：定时器2(普通定时器)重新初始化函数            *
*   输入：    无                                           *
*   输出：    无                                           *
*   调用函数：Port_Init()  RCC_Configuration()             *
***********************************************************/
void TIM2_ReConfiguration(u8 cInTime)
{
	//TIM2 的时间 作为差值的时间间隔	触发DMA 输出数据到DAC
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	/* TIM2 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	/* Time base configuration */
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Period = 72*cInTime; // 上位机传输的命令时间单位是US
	TIM_TimeBaseStructure.TIM_Prescaler = 0x0;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	/* TIM2 TRGO selection */
	TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);

//	TIM_Cmd(TIM2, ENABLE);


}



void DMA2_ReConfiguration( u8 cSampleNum)
{
	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);

	/* DMA2 channel4 configuration */
	DMA_DeInit(DMA2_Channel4);
	DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40007420;
	DMA_InitStructure.DMA_MemoryBaseAddr = 	(u32)wInArrayTab;


//	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)Sine12bit;

	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_BufferSize = cSampleNum+1;

	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;

	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;

//	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;

	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;

	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA2_Channel4, &DMA_InitStructure);


	DMA_ITConfig(DMA2_Channel4, DMA_IT_TC, ENABLE);

	/* Enable DMA2 Channel4 */
	DMA_Cmd(DMA2_Channel4, ENABLE);


}
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



/***********************************************************
*   函数说明：串口停止接收函数                             *
*   输入：    无                                           *
*   输出：    无                                           *
*   调用函数：ST.F.W.3.0                                   *
***********************************************************/
void USART1_StopRx(void)
{
	USART_ITConfig(USART1,USART_IT_RXNE,DISABLE);
}

/***********************************************************
*   函数说明：串口开始接收使能函数                         *
*   输入：    无                                           *
*   输出：    无                                           *
*   调用函数：ST.F.W.3.0                                   *
***********************************************************/
void USART1_BeginRx(void)
{
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
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

