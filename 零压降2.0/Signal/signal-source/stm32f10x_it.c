/*************STMicroelectronics (C) COPYRIGHT 2008*********/
/***********************************************************
*   函数库说明：硬件平台中断函数库                         *
*   版本：    Ver1.0                                       *
*   作者：    zzjjhh250/ZZJJHH250 @ (CopyRight)            *
*   创建日期：08/31/2010                                   *
* -------------------------------------------------------- *
*  [硬件说明]                                              *
*   处理器：    STM32F103VBT6                              *
*   系统时钟：  外部8M/PLL = 72M                           *
* -------------------------------------------------------- *
*  [支 持 库]                                              *
*   库名称：    PF_Config.h                                *
*   需要版本：  -----                                      *
*   支持库说明：硬件平台配置声明库                         *
* -------------------------------------------------------- *
*  [版本更新]                                              *
*   修改：                                                 *
*   修改日期：                                             *
*   版本：                                                 *
* -------------------------------------------------------- *
*  [版本历史]                                              *
* -------------------------------------------------------- *
*  [使用说明]：											   *
*				1.可调用EX_Support.c模块中各自对应的处理函 *
*			      数处理中断。                             *
*				2.所有中断响应函数，一般如果中断比较简易， *
*				  则直接在此编写，否则可以在EX_Support.c   *
***********************************************************/

/********************
* 头 文 件 配 置 区 *
********************/ 
/* Includes ------------------------------------------------------------------*/
#include "..\Source\LIB_Config.h"
#include "..\Source\PF_Config.H"

#include ".\fwlib\stm32f10x.h"
#include ".\FWLib\stm32f10x_exti.h"
#include "sys_comm.h"

/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define countof(a)   (sizeof(a) / sizeof(*(a)))

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : NMIException
* Description    : This function handles NMI exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void NMIException(void)
{
}

/*******************************************************************************
* Function Name  : HardFaultException
* Description    : This function handles Hard Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void HardFaultException(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/*******************************************************************************
* Function Name  : MemManageException
* Description    : This function handles Memory Manage exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MemManageException(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/*******************************************************************************
* Function Name  : BusFaultException
* Description    : This function handles Bus Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void BusFaultException(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/*******************************************************************************
* Function Name  : UsageFaultException
* Description    : This function handles Usage Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UsageFaultException(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/*******************************************************************************
* Function Name  : DebugMonitor
* Description    : This function handles Debug Monitor exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DebugMonitor(void)
{
}

/*******************************************************************************
* Function Name  : SVCHandler
* Description    : This function handles SVCall exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SVCHandler(void)
{
}

/*******************************************************************************
* Function Name  : PendSVC
* Description    : This function handles PendSVC exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PendSVC(void)
{
}

 

 
/*******************************************************************************
* Function Name  : SysTickHandler
* Description    :系统时钟，一般1MS中断一次
*******************************************************************************/
 
extern volatile s32 pa_org_value;   /* 保存查表数据 */
extern volatile s32 pb_org_value;
extern volatile s32 pc_org_value;

extern volatile s32 pa_zero_position_value;
extern volatile s32 pb_zero_position_value;
extern volatile s32 pc_zero_position_value;

extern u32 pa_zero_pos_mic_adj_cnt; /* 用于0位微调, 调整占空比 */
extern u32 pb_zero_pos_mic_adj_cnt;
extern u32 pc_zero_pos_mic_adj_cnt;

/* 方波的逻辑电平对应的ad值 */
extern u16 pa_zero_pos_square_lgc_0; 
extern u16 pa_zero_pos_square_lgc_1;

extern u16 pb_zero_pos_square_lgc_0; 
extern u16 pb_zero_pos_square_lgc_1;

extern u16 pc_zero_pos_square_lgc_0; 
extern u16 pc_zero_pos_square_lgc_1;

extern volatile int is_need_send_new_data2dac;

volatile int is_over_load = 0;

unsigned long sys_ticks_cnt; 

/* 获取三相数据, 调整0位 */
void SysTick_Handler(void)
{
    static int cIndex = 0;
    static int pa_zero_position_cnt = 0;
    static int pb_zero_position_cnt = 0;
    static int pc_zero_position_cnt = 0;

    ++sys_ticks_cnt;

    is_need_send_new_data2dac = 1;
	pa_org_value = Sine12bit[cIndex];
	#if 0
 	pb_org_value = Sine12bit[(cIndex+100)%301];
 	pc_org_value = Sine12bit[(cIndex+200)%301];
 	#else
 	pb_org_value = Sine12bit[(cIndex+200)%301];
 	pc_org_value = Sine12bit[(cIndex+100)%301];
 	#endif

#if 1
    if (pa_zero_position_cnt < pa_zero_pos_mic_adj_cnt)
        pa_zero_position_value = pa_zero_pos_square_lgc_1;
    else
        pa_zero_position_value = pa_zero_pos_square_lgc_0;

    if (pb_zero_position_cnt < pb_zero_pos_mic_adj_cnt)
        pb_zero_position_value = pb_zero_pos_square_lgc_1;
    else
        pb_zero_position_value = pb_zero_pos_square_lgc_0;

    if (pc_zero_position_cnt < pc_zero_pos_mic_adj_cnt)
        pc_zero_position_value = pc_zero_pos_square_lgc_1;
    else
        pc_zero_position_value = pc_zero_pos_square_lgc_0;
    
    
    if (++pa_zero_position_cnt >= NUM_OF_PX_ZERO_POS_MIC_ADJ_CNT)
        pa_zero_position_cnt = 0;

    if (++pb_zero_position_cnt >= NUM_OF_PX_ZERO_POS_MIC_ADJ_CNT)
        pb_zero_position_cnt = 0;

    if (++pc_zero_position_cnt >= NUM_OF_PX_ZERO_POS_MIC_ADJ_CNT)
        pc_zero_position_cnt = 0;
#endif 
/* =================== */
	++cIndex;
	if(cIndex == 300)
	    cIndex = 0;

	return;
}

/*******************************************************************************
* Function Name  : WWDG_IRQHandler
* Description    : This function handles WWDG interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void WWDG_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : PVD_IRQHandler
* Description    : This function handles PVD interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PVD_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TAMPER_IRQHandler
* Description    : This function handles Tamper interrupt request. 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TAMPER_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : RTC_IRQHandler    RTC中断
* Description    : RTC中断中，处理好Real_Time的值
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RTC_IRQHandler(void)
{

}

/*******************************************************************************
* Function Name  : FLASH_IRQHandler
* Description    : This function handles Flash interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void FLASH_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : RCC_IRQHandler
* Description    : This function handles RCC interrupt request. 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RCC_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : EXTI0_IRQHandler
* Description    : This function handles External interrupt Line 0 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI0_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : EXTI1_IRQHandler
* Description    : This function handles External interrupt Line 1 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI1_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : EXTI2_IRQHandler
* Description    : This function handles External interrupt Line 2 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
volatile int exti2_cnt;
void EXTI2_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line2)!= RESET){
        EXTI_ClearITPendingBit(EXTI_Line2);
        /* mark by David */
        /* PA2 过载检测, 正常负载高电平, 过载时为低电平 */
        CS1_H;
    	SPI_DMA_Table[0] = DAC_CLR_CMD;
    	DMA_Configuration_SPI1_TX();
    	CS1_L;
    	//DMA_Cmd(DMA1_Channel3,ENABLE);	 //DMA通道号 使能
    	enable_dmax_y(DMA1_Channel3);

    	is_over_load = 1;
    	++exti2_cnt;
	}
}

/*******************************************************************************
* Function Name  : EXTI3_IRQHandler
* Description    : This function handles External interrupt Line 3 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI3_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : EXTI4_IRQHandler
* Description    : This function handles External interrupt Line 4 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI4_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : DMAChannel1_IRQHandler
* Description    : This function handles DMA Stream 1 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel1_IRQHandler(void)
{
#if 0
  	if(DMA_GetITStatus(DMA1_IT_TC1))
 	{
	//	ADC_Cmd(ADC1, DISABLE);	   //在DMA之后将ADC1关闭 等待下次上位机命令的触发
	//	DMA_ClearITPendingBit(DMA1_IT_GL1);	//清除全部中断标志
	//	ADC_OnceFlag=TRUE;
	}
#endif
}

/*******************************************************************************
* Function Name  : DMAChannel2_IRQHandler
* Description    : This function handles DMA Stream 2 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel2_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : DMAChannel3_IRQHandler
* Description    : This function handles DMA Stream 3 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel3_IRQHandler(void)
{
//	u16 dwCounter = 0xffff;
    if(DMA_GetITStatus(DMA1_IT_TC3)) {  	
		DMA_ClearITPendingBit(DMA1_IT_GL3);	//清除全部中断标志
#if 1
		//while(SPI_I2S_GetFlagStatus(SPI1 , SPI_I2S_FLAG_BSY) && dwCounter-- )
		while(SPI_I2S_GetFlagStatus(SPI1 , SPI_I2S_FLAG_BSY))
			;//如果是高电平 则表示忙

		CS1_H;
		CS_H;
#else

#endif
	}
}

/*******************************************************************************
* Function Name  : DMAChannel4_IRQHandler
* Description    : This function handles DMA Stream 4 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel4_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : DMAChannel5_IRQHandler
* Description    : This function handles DMA Stream 5 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel5_IRQHandler(void)
{
#if 0 /* David */
	if(DMA_GetITStatus(DMA1_IT_TC5))
 	{
	    DMA_ClearITPendingBit(DMA1_IT_GL5);	//清除全部中断标志


 
	//!Only For Test Using
  	   //GPIOE->ODR ^= GPIO_Pin_13;
	   //GPIOE->ODR ^= GPIO_Pin_14;
 	}
#endif
}

/*******************************************************************************
* Function Name  : DMAChannel6_IRQHandler
* Description    : This function handles DMA Stream 6 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel6_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : DMAChannel7_IRQHandler
* Description    : This function handles DMA Stream 7 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel7_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : ADC_IRQHandler
* Description    : This function handles ADC global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ADC1_2_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : USB_HP_CAN_TX_IRQHandler
* Description    : This function handles USB High Priority or CAN TX interrupts 
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USB_HP_CAN1_TX_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : USB_LP_CAN_RX0_IRQHandler
* Description    : This function handles USB Low Priority or CAN RX0 interrupts 
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USB_LP_CAN1_RX0_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : CAN_RX1_IRQHandler
* Description    : This function handles CAN RX1 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CAN1_RX1_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : CAN_SCE_IRQHandler
* Description    : This function handles CAN SCE interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CAN1_SCE_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : EXTI9_5_IRQHandler
* Description    : This function handles External lines 9 to 5 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI9_5_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM1_BRK_IRQHandler
* Description    : This function handles TIM1 Break interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM1_BRK_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM1_UP_IRQHandler
* Description    : This function handles TIM1 overflow and update interrupt 
*                  request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM1_UP_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM1_TRG_COM_IRQHandler
* Description    : This function handles TIM1 Trigger and commutation interrupts 
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM1_TRG_COM_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM1_CC_IRQHandler
* Description    : This function handles TIM1 capture compare interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM1_CC_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM2_IRQHandler TIM2中断
* Description    : This function handles TIM2 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

void TIM2_IRQHandler(void)
{
#if 0
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);
#endif
}

/*******************************************************************************
* Function Name  : TIM3_IRQHandler
* Description    : This function handles TIM3 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
 volatile s32  dwTemper,dwPWMKan20,dwPWMKan21,test;
 volatile s32  dwPWMKan20 = 60;
 volatile s32  dwPWMKan21 = 60;
 volatile s32  dwTemperContrl = 128;
 volatile s32  dwTMP,dwToPWM;
 volatile u8   cTemperCnt;
 volatile bool bFlag_TemperDir = FALSE; //继电器默认方向是制冷 说明是开始制冷 TO_COOL_DIR FALSE制冷
// volatile bool bFlag_ToCoolDir = FALSE;

void TIM3_IRQHandler(void)	 //	 1.7s
{
  /* Clear the interrupt pending flag */
  TIM_ClearFlag(TIM3, TIM_FLAG_Update);

  //GPIOB->ODR ^= GPIO_Pin_8;//运行指示灯

  cTemperCnt++;
  dwTMP +=  ADC_Val[0].union16;

  if(cTemperCnt == 4)
  {	
  	cTemperCnt = 0;
  	dwTMP >>= 2;
#if 0
 	dwTemper = (1.42 -   ((dwTMP*2.50)/4096))*1000/4.35 + 25;
   	//dwTemper = (1.38 -   ((dwTMP*2.50)/4096))*1000/4.35 + 25;
#else /* David */
   	dwTemper = (142 -   ((dwTMP*250)/4096)) * 1000/435 + 25;
   	//dwTemper = (138 -   ((dwTMP*250)/4096)) * 1000/435 + 25;
#endif
	dwTMP = 0;
 
 	if(dwTemper < 28)	   //小于28°
	  {
		   dwTemperContrl--;
  	  }
	else if(dwTemper > 28)		//大于28°
	  {
		   dwTemperContrl++;
	  }
	else
      {
		   dwTemperContrl = dwTemperContrl;
	  }



	 if(dwTemperContrl < 3)
	 	dwTemperContrl =2;
	 if(dwTemperContrl >254)
	 	dwTemperContrl =254;	//限幅



	if(dwTemperContrl > 128)	 	
	 {
		GPIOA->BRR = GPIO_Pin_1;//	置低  开始制冷
		bFlag_TemperDir = FALSE;

		dwToPWM = dwTemperContrl - 128;
	 }
	 else
		 if(dwTemperContrl < 128)
		 {

	        GPIOA->BSRR = GPIO_Pin_1;  //开始制热	置高
	        bFlag_TemperDir = TRUE;

			dwToPWM = 128 - dwTemperContrl;
		 }
		 else
		 {
		 	 dwToPWM = 0;	
		 }
 


	  if(bFlag_TemperDir == FALSE)	//制冷
	    {
		  if(dwToPWM >= 82)
		  {
		    TIM2->CCR1 = 252;
			dwToPWM = 82;//防止回来时 需要很长时间
		  }
		  else
            TIM2->CCR1 = dwToPWM*3 ;
		}
	  else
	    {
		  if(dwToPWM >= 125)
		  {
		    TIM2->CCR1 = 250;
			dwToPWM = 125;//防止回来时 需要很长时间
		  }
		  else
            TIM2->CCR1 = dwToPWM*2 ;
		} 
			  //test= test + 5;
			  //if(test > 254)	test = 0;		
		  	//  TIM2->CCR1 =test;
  }
//	GPIOE->ODR ^= GPIO_Pin_5;
}

/*******************************************************************************
* Function Name  : TIM4_IRQHandler
* Description    : This function handles TIM4 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM4_IRQHandler(void)
{
#if 0
	TIM_ClearFlag(TIM4, TIM_FLAG_Update);
 #endif
}
/*******************************************************************************
* Function Name  : I2C1_EV_IRQHandler
* Description    : This function handles I2C1 Event interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C1_EV_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : I2C1_ER_IRQHandler
* Description    : This function handles I2C1 Error interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C1_ER_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : I2C2_EV_IRQHandler
* Description    : This function handles I2C2 Event interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C2_EV_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : I2C2_ER_IRQHandler
* Description    : This function handles I2C2 Error interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C2_ER_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : SPI1_IRQHandler
* Description    : This function handles SPI1 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI1_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : SPI2_IRQHandler
* Description    : This function handles SPI2 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI2_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : USART1_IRQHandler
* Description    : This function handles USART1 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USART1_IRQHandler(void)
{
	if (is_usartx_rxd_not_empty(USART1)) {  
		/* 读数据时硬件会清楚USART_IT_RXNE位 */
		if (0 == USART1_GetRxBufferLeftLength())
		    USART1_GetRxBufferData(); /* 接收缓冲区满, 丢弃一个老数据 */
	    USART1_PutDatatoRxBuffer(get_usartx_data(USART1));
	}

	if (is_usartx_txd_empty(USART1)) {
    	if(USART1_GetTxBufferCurrentSize()!= 0) {
    		put_usartx_data(USART1, USART1_GetTxBufferData());
    	} else {
    		disable_usartx_send_int(USART1); //UASRT1_StopSend();				 
    	}
	}

 	if(is_usartx_overrun_err(USART1)) {
		USART_ClearFlag(USART1,USART_FLAG_ORE);	//清除ORE
		USART_ReceiveData(USART1);				//读DR
	}
}


/*******************************************************************************
* Function Name  : USART2_IRQHandler
* Description    : This function handles USART2 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
#if 0
volatile  bool bIDFlag = FALSE;
volatile  u8  cRXLastVal = 0;
#endif
void USART2_IRQHandler(void)
{
#if 0
	u8 cRXValTemp = 0;
	u8 CIDValTemp = 0;
	u16 WTemp  = 0;

	FIBER_STATE_ON; //光纤示灯

	g_bDatReceCnt = 0; //	 防止 烤肉	 主中++

 //由于F高速的请求 这样 见程序直接在中断中处理 即通过SPI DAC输出
 //	if(USART_GetITStatus(USART2,USART_IT_RXNE)==SET)
	{
		USART_ClearITPendingBit(USART2,USART_IT_RXNE); //清除标志位

	//	USART2_PutDatatoRxBuffer(USART_ReceiveData(USART2));
	
	
	 cRXValTemp  =(u8)(USART2->DR);	 //接收的值暂存

	 CIDValTemp	= cRXValTemp>>6;  //ID号暂存

 /////////////////////////////////////////////////////////

  	if(CIDValTemp)//是高字节 即;ID是非零
	  {
	  	 bIDFlag = TRUE ; //设置标志位

		 cRXLastVal = cRXValTemp;//将高字节数据	暂存
	  	 switch (CIDValTemp)
		   {
			case 1 :  
					SPI_DMA_Table[0]= CMD_DAC0;

					break;
			case 2 :  
					SPI_DMA_Table[0]= CMD_DAC1;
					break;
			case 3 :  
					SPI_DMA_Table[0]= CMD_DAC2;
					break;
			default : ;
	        }
	  	
	  }
	else
	  {
		if(bIDFlag)	 //如果上个Byte 是高字节
		  {	
		  	WTemp =	cRXLastVal;
			WTemp <<= 8;   //放到高字节 ID存在
		  	WTemp += (u8)(cRXValTemp<<2);	  //去除ID（低字节）放到低字节
			WTemp <<= 2;

			SPI_DMA_Table[2] = (u8)WTemp;  //低字节 放到 数组最后
			SPI_DMA_Table[1] = (u8)(WTemp>>8);

			DMA_Configuration_SPI1_TX();
   			CS1_L;
	  		DMA_Cmd(DMA1_Channel3,ENABLE);	 //DMA通道号 使能
		  }
 
		  
		  bIDFlag = FALSE;//复位标志位
	  }
   }

	//溢出处理-如果发生溢出需要先清除ORE,再读DR寄存器 则可清除不断入中断的问题
  if(USART_GetFlagStatus(USART2,USART_FLAG_ORE)==SET)
	{
		USART_ClearFlag(USART2,USART_FLAG_ORE);	//清除ORE
		USART_ReceiveData(USART2);				//读DR
	}


////发送空中断 将TX FIFO中的数据发送 直到其中的数据完成 就停止发送
//	if(USART_GetITStatus(USART2,USART_IT_TXE)==SET)
//	{	   
//		if(USART2_GetTxBufferCurrentSize()!= 0)	//一直判断发送FIFO中 是否有数据 只要有数据 就不停止发送
//		{
//			USART_SendData(USART2, USART2_GetTxBufferData());
//		}
//		else
//		{
//			UASRT2_StopSend();					 //TX发送FIFO中的数据发送完毕之后 停止发送
//		}
//	}
#endif
}

/*******************************************************************************
* Function Name  : USART3_IRQHandler
* Description    : This function handles USART3 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USART3_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : EXTI15_10_IRQHandler
* Description    : This function handles External lines 15 to 10 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI15_10_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : RTCAlarm_IRQHandler
* Description    : This function handles RTC Alarm interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RTCAlarm_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : USBWakeUp_IRQHandler
* Description    : This function handles USB WakeUp interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USBWakeUp_IRQHandler(void)
{
}













/*****************************************************************
*		以下是HD系列才有的中断资源		V  D   E
******************************************************************/

void TIM8_BRK_IRQHandler(void)
{

}

void TIM8_UP_IRQHandler(void)
{

}

void TIM8_TRG_COM_IRQHandler(void)
{

}

void TIM8_CC_IRQHandler(void)
{

}

void ADC3_IRQHandler(void)
{

}
        
void FSMC_IRQHandler(void)
{

}

void SDIO_IRQHandler(void)
{

}

void TIM5_IRQHandler(void)
{

}

void SPI3_IRQHandler(void)
{

}

void UART4_IRQHandler(void)
{

}

void UART5_IRQHandler(void)
{

}

void TIM6_IRQHandler(void)
{

}

void TIM7_IRQHandler(void)
{

}

void DMA2_Channel1_IRQHandler(void)
{

}

void DMA2_Channel2_IRQHandler(void)
{

}

void DMA2_Channel3_IRQHandler(void)
{

}

void DMA2_Channel4_5_IRQHandler(void)
{
#if 0
	if(DMA_GetITStatus(DMA2_IT_TC4))//	 ||DMA_GetITStatus(DMA2_IT_TC5)
 	{
		TIM2_DISABLE;	   //在DMA之后将TIM2关闭 等待下次传输过来的数据触发
		DMA_ClearITPendingBit(DMA2_IT_GL4);	//清除全部中断标志
	//	DMA_ClearITPendingBit(DMA2_IT_GL5);
	//	GPIOE->ODR ^= GPIO_Pin_5;
		ADC_OnceFlag=TRUE;
	}
#endif
}

 







/****** (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/
