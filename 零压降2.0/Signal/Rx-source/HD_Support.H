#ifndef  _HD_SUPPORT_H_
#define  _HD_SUPPORT_H_
/***********************************************************
*   声明库说明：底层硬件初始化声明库                       *
*   版本：      Ver1.0                                     *
*   作者：      zzjjhh250/ZZJJHH250 @ (CopyRight)          *
*   创建日期：  08/11/2010                                 *
* -------------------------------------------------------- *
*  [硬件说明]                                              *
*   处理器：    STC12C5616AD                               *
*   系统时钟：  外部16M                                    *
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
#define ADCSample9                      ((uint8_t)0x00)
#define ADCSample13                     ((uint8_t)0x01)
#define ADCSample17                     ((uint8_t)0x02)
#define ADCSample27                     ((uint8_t)0x03)
#define ADCSample36                     ((uint8_t)0x04)
#define ADCSample45                     ((uint8_t)0x05)
#define ADCSample56                     ((uint8_t)0x06)
#define ADCSample168                    ((uint8_t)0x07)
/*------------------*
*   动 作 宏 定 义  *
*------------------*/
/*


# define KEY_OUT_Y_P1   _PD2
# define KEY_OUT_Y_P2   _PD3
# define KEY_OUT_Y_P3   _PD4
# define KEY_OUT_Y_P4   _PD5
# define KEY_OUT_Y_P5   _PD6

# define SET_KEY_OUT_ALL_PORT_HIGH  PORTD |= BIT(PD2)|BIT(PD3)|BIT(PD4)|BIT(PD5)|BIT(PD6);
   */
/********************
*    结构体定义区   *
********************/


/********************
*   函 数 引 用 区  *
********************/
extern void Device_Init(void);



////以下是不合格的 非法调用的函数【不符合封装规范】
//  ADCSample9   -   9us            
//  ADCSample13  -   13us              
//  ADCSample17  -   17us               
//  ADCSample27  -   27us              
//  ADCSample36  -   36us              
//  ADCSample45  -   45us               
//  ADCSample56  -   56us               
//  ADCSample168 -   168us 
//extern void ADC1_SampleTimeConfig(u8 SampleTime);

extern void DMA_ReConfig(void);

extern void ADC1_Configuration(void);
extern void DMA_Configuration(void);

/********************
*   全局变量引用区  *
********************/
extern RCC_ClocksTypeDef RCC_ClockFreq;   //全局设备时钟提供商 ^_^



extern volatile u8 uart2_rx_buf[6];
//extern volatile u8 spi_dma_tbl_grp[3];


//#define IS_USART2_USE_DMA  (1)



#endif
