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
#include "tx_rx_comm.h"
#include "common.h"

#define TIM3_ENABLE			TIM_Cmd(TIM3, ENABLE);

#define TIM3_DISABLE	 	TIM_Cmd(TIM3, DISABLE);

#define ADC1_ENABLE	 		ADC_Cmd(ADC1, ENABLE);

#define ADC1_DISABLE		ADC_Cmd(ADC1, DISABLE);

/*

  malooei 2013/1/31
  定义新版V2.0发送端LED指示，硬件为共阴，即BSRR等亮，BRR灯灭
  按照文档<LCD界面简要描述>中关于指示灯的说明，列表如下:
  LED1->PA15->运行灯（绿色）闪烁
  LED2-> PB3->接收数据（绿色）--正常接受数据亮，无数据传输灭
  LED3-> PB4->故障灯-（红色）-有告警灯亮，无告警灯灭
  LED4-> PB5->切换灯（黄色）--正常的时候灭，输出切换到PT时亮
  LED5-> PB6->待定


*/

#define SIGNAL_IN_STATE_ON 	do {GPIOB->BRR = GPIO_Pin_8;} while (0)    //灯亮 置0
#define SIGNAL_IN_STATE_OFF do {GPIOB->BSRR = GPIO_Pin_8;} while (0)   //灯灭 置1

#define FIBER_STATE_ON 		do {GPIOB->BRR = GPIO_Pin_9;} while (0)  //灯亮 置0
#define FIBER_STATE_OFF 	do {GPIOB->BSRR = GPIO_Pin_9;} while (0) //灯灭 置1







/********************
*    结构体定义区   *
********************/
typedef union union_temp16 {
        u16 union16;
        u8  unionarray[2];
} union16_type;
/********************
*   函 数 引 用 区  *
********************/

extern void 	Driver_Init(void);
extern void 	Delay_20ms(u32 speed);

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



extern void 	USART1_TxBufferInit_B(void);
extern u8 		USART1_PutDatatoTxBuffer_B(u8 dat);
extern u16 	USART1_GetTxBufferLeftLength_B(void);
extern u8 		USART1_GetTxBufferData_B( void );
extern u16 	USART1_GetTxBufferCurrentSize_B( void );
extern void    USART1_FlushTxBuffer_B( void );
extern void 	USART1_PutBytesToTxFIFO_B( u8 *pdat ,u16 length);

extern void 	USART1_QueryPutMultiChar_B( u8 *pdat ,u16 length);
extern void 	USART1_QueryPutChar_B( u8 dat );


extern void 	USART1_RxBufferInit_B(void);
extern u8 		USART1_PutDatatoRxBuffer_B(u8 dat);
extern u16 	USART1_GetRxBufferLeftLength_B(void);
extern u8 		USART1_GetRxBufferData_B( void );
extern u16 	USART1_GetRxBufferCurrentSize_B( void );
extern void    USART1_FlushRxBuffer_B( void );
extern void 	USART1_GetBytesFromRxFIFO_B( u8 *pdat ,u16 length);











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

//===================DMA ADC=====================

extern u16 DigitFilter(u16* buf,u8 no);

extern void ENC_Enable(void);
extern void ENC_Disable(void);

extern void SpeedTIM2_Enable(void);
extern void SpeedTIM2_Disable(void);

extern void WritePOT_A(u8 dat) ;
extern void WritePOT_B(u8 dat) ;



extern void DMA_ReConfiguration(void);

extern void TIM3_ADC1Trigger_Reconfig(void);
/********************
*   全局变量引用区  *
********************/
extern u16 g_wSysTime1;
extern u16 g_wSysTime2;
extern u16 g_wSysTime3;
extern u16 g_wSysTime4;



extern volatile bool  UARTBufferFlag;
extern volatile bool  UART_TXFlag;
extern void SetT1Pwm1(u16 pulse ,u8 num);


typedef enum {BUF_NO1=0,BUF_NO2=1} BUF_NO_TYPE;


extern u8 UART1_SendBuff1[8];
extern u8 UART1_SendBuff2[8];

extern volatile u8 uart1_dma_buf[UART1_DMA_BUF_LEN];
extern volatile u16 adc_buf4temper[2];
extern volatile u32 is_had_enter_adjust_value_mode;

extern volatile u32  pa_avg_val_max;
extern volatile u32  pa_avg_val_min;
extern volatile u32  pa_lost_avg_val;

extern volatile u32  pb_avg_val_max;
extern volatile u32  pb_avg_val_min;
extern volatile u32  pb_lost_avg_val;

extern volatile u32  pc_avg_val_max;
extern volatile u32  pc_avg_val_min;
extern volatile u32  pc_lost_avg_val;


/* 方波一个周期对应的systick中断次数 */
#define NUM_OF_PX_ZERO_POS_MIC_ADJ_CNT  (100)



#endif



