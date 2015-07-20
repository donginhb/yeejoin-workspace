#ifndef _EX_SUPPORT_H_
#define _EX_SUPPORT_H_
/***********************************************************
*   声明库说明：底层硬件初始化驱动声明库                   *
*   版本：                                                 *
*   作者：                                                 *
*   创建日期：                                             *
* -------------------------------------------------------- *
*  [支 持 库]                                              *
*   支持库名称：                                           *
*   需要版本：                                             *
*   声明库说明：                                           *
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

/********************
*   系 统 宏 定 义  *
********************/

/*------------------*
*   常 数 宏 定 义  *
*------------------*/


/*------------------*
*   动 作 宏 定 义  *
*------------------*/
#define TIM2_ENABLE 	TIM_Cmd(TIM2, ENABLE)
#define TIM2_DISABLE 	TIM_Cmd(TIM2, DISABLE)

#define C1_PE13_RESET	GPIOE->BRR=GPIO_Pin_13
#define C1_PE13_SET		GPIOE->BSRR=GPIO_Pin_13

#define C2_PE14_RESET	GPIOE->BRR=GPIO_Pin_14
#define C2_PE14_SET		GPIOE->BSRR=GPIO_Pin_14

#define CS_H		GPIOB->BSRR=GPIO_Pin_12
#define CS_L		GPIOB->BRR=GPIO_Pin_12

#if 0
#define CS1_H		GPIOA->BSRR=GPIO_Pin_6
#define CS1_L		GPIOA->BRR=GPIO_Pin_6
#else
#define CS1_H		do {GPIOA->BSRR = GPIO_Pin_6;} while (0)
#define CS1_L		do {GPIOA->BRR  = GPIO_Pin_6;} while (0)
#endif
#define RUN_STATE_ON 	GPIOB->BRR = GPIO_Pin_8;  //灯亮 置0
#define RUN_STATE_OFF 	GPIOB->BSRR = GPIO_Pin_8; //灯灭 置1

#define FIBER_STATE_ON 		do {GPIOB->BRR = GPIO_Pin_9;} while (0)  //灯亮 置0
#define FIBER_STATE_OFF 	do {GPIOB->BSRR = GPIO_Pin_9;} while (0) //灯灭 置1

/********************
*    结构体定义区   *
********************/
typedef union union_temp16 {
        u16 union16;
        u8  unionarray[2];
} union16_type;



typedef union union_temp32 {
        u32 union16;
        u16  unionarray[2];
} union32_type;


/********************
*   函 数 引 用 区  *
********************/
//=====================================
extern void 	Driver_Init(void);
extern void 	Delay_20ms(u32 speed);
//=====================================
extern u8 		USART1_PutDatatoRxBuffer(u8 dat);
extern u16 		USART1_GetRxBufferLeftLength(void);
extern u8 		USART1_GetRxBufferData( void );
extern u16 		USART1_GetRxBufferCurrentSize( void );
extern void    	USART1_FlushRxBuffer( void );


extern u8	 	USART1_PutDatatoTxBuffer(u8 dat);
extern u16 		USART1_GetTxBufferLeftLength(void);
extern u8 		USART1_GetTxBufferData( void );
extern u16 		USART1_GetTxBufferCurrentSize( void );
extern void    	USART1_FlushTxBuffer( void );
extern void 	USART1_QueryPutChar( u8 dat );
extern void 	USART1_QueryPutMultiChar( u8 *pdat ,u16 length);

extern void 	USART1_PutBytesToTxFIFO( u8 *pdat ,u16 length);
extern void 	USART1_GetBytesFromRxFIFO( u8 *pdat ,u16 length);
/////////////////////////////////////////////////////////////////////
extern u8 		USART2_PutDatatoRxBuffer(u8 dat);
extern u16 		USART2_GetRxBufferLeftLength(void);
extern u8 		USART2_GetRxBufferData( void );
extern u16 		USART2_GetRxBufferCurrentSize( void );
extern void    	USART2_FlushRxBuffer( void );


extern u8	 	USART2_PutDatatoTxBuffer(u8 dat);
extern u16 		USART2_GetTxBufferLeftLength(void);
extern u8 		USART2_GetTxBufferData( void );
extern u16 		USART2_GetTxBufferCurrentSize( void );
extern void    	USART2_FlushTxBuffer( void );
extern void 	USART2_QueryPutChar( u8 dat );
extern void 	USART2_QueryPutMultiChar( u8 *pdat ,u16 length);

extern void 	USART2_PutBytesToTxFIFO( u8 *pdat ,u16 length);
extern void 	USART2_GetBytesFromRxFIFO( u8 *pdat ,u16 length);




//////////////////////////////////////////////////////////////

extern void 	UASRT1_BeginSend(void);
extern void 	UASRT1_StopSend(void);

extern void 	UASRT2_BeginSend(void);
extern void 	UASRT2_StopSend(void);

extern void 	USART1_StopRx(void);
extern void 	USART1_BeginRx(void);

extern void 	USART2_StopRx(void);
extern void 	USART2_BeginRx(void);

//===================DMA ADC=====================

extern u16 DigitFilter(u16* buf,u8 no);



//===============================================
extern void DMA2_ReConfiguration( u8 cSampleNum);
extern void TIM2_ReConfiguration(u8 cInTime);



extern void DMA_Configuration_SPI1_TX( void );


/********************
*   全局变量引用区  *
********************/
extern u16 g_wSysTime1;
extern u16 g_wSysTime2;
extern u16 g_wSysTime3;
extern u16 g_wSysTime4;

extern  u16 counter1;
extern 	u16 counter2;
extern 	u16 counter3;  
extern	u16 counter4;


extern volatile bool	 ADC_OnceFlag;
extern union16_type		 ADC_Val[6] ;

extern u8 USART1_DMA_Buf1[8];

extern uc16 			 Sine12bit[32] ;
extern u8  cSineWaveIndex ;

extern union32_type 	 wInArrayTab[128];


extern  u8 SPI1SendByte(u8 byte);

extern  volatile u8 SPI_DMA_Table_serial_in[6];
extern  volatile u8 SPI_DMA_Table_serial_in_zero[6];
extern volatile u16 adc_buf[4];

extern volatile u8 adc_formtx_tolcd_data_buf[8];
extern volatile s32 boxtemple_sample_30ms;
#endif

