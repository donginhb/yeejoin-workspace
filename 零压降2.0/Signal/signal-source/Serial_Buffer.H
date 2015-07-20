#ifndef _SERIAL_BUFF_H_
#define _SERIAL_BUFF_H_
/***********************************************************
*   声明库说明：串行通讯缓冲区处理声明库                   *
*   版本：      v1.10                                      *
*   作者：      王卓然                                     *
*   创建日期：  2006年4月24日                              *
* -------------------------------------------------------- *
*  [支 持 库]                                              *
*   支持库名称：LIB_Config.h                               *
*   需要版本：  ----                                       *
*   声明库说明：库函数配置声明库                           *
* -------------------------------------------------------- *
*  [版本更新]                                              *
* -------------------------------------------------------- *
*  [版本历史]                                              *
*       v0.30  该版本提供了最基本的环形队列缓冲区操作函数。*
*       v0.31  增加了调用该函数时候的宏说明，可以选择需要  *
*              提供的缓冲区功能。                          *
*       v0.40  增加了串口发送进程。增加了原子操作。        *
*       v1.00  针对新规范进行了修改，增加自动收发进程。    *
*       v1.10  将数据的发送改为中断自动进行，也就是说，每  *
*              次只需要超级循环中的进程触发第一次发送，直  *
*              到发送缓冲中的数据都被发送完成之前，不需要  *
*              主循环进行干预。                            *
* -------------------------------------------------------- *
*  [使用说明]                                              *
*           1、通过SERIAL_RX_BUFF_SIZE来设定接收缓冲区的大 *
*              小；通过SERIAL_TX_BUFF_SIZE来设定发送缓冲区 *
*              的大小。                                    *
*           2、通过定义_USE_SERIAL_RX_BUFF来选择添加一个接 *
*              收缓冲区；通过定义_USE_SERIAL_TX_BUFF来添加 *
*              一个发送缓冲区。                            *
*           3、在取数的时候，有时候要考虑屏蔽中断，否则会  *
*              出现不可预知的后果。                        *
*           4、可以通过宏_USE_SERIAL_TX_TRANSMITTER_PROC在 *
*              使用发送缓冲时，使用自动发送进程。并需要定  *
*              义宏UDR_PORT来指定发送端口、也需要将宏      *
*              SERIAL_INSERT_USART_TX_ISR_CODE插入到发送完 *
*              成中断处理程序中。                          *
*           5、使用宏SERIAL_ENABLE_WRITE定义额外的发送使能 *
*              代码。例如485总线的发送使能等。             *
*           6、使用宏SERIAL_DISABLE_WRITE定义额外的发送使  *
*              能关闭代码。例如485总线发送使能的关闭。     *
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
#ifdef _USE_SERIAL_RX_BUFF
# define SERIAL_IN(x)              UARTgetDataFromRxBuff(&x)
#endif
#ifdef _USE_SERIAL_TX_BUFF                
# define SERIAL_OUT(x)             UARTaddDataToTxBuff(x);
#endif


/********************
*   全局变量引用区  *
********************/

#ifdef _USE_SERIAL_TX_BUFF
    #ifdef _USE_SERIAL_TX_TRANSMITTER_PROC
extern BOOL g_bIfSerialTransmitCompleted;
    #endif
#endif

/********************
*   函 数 引 用 区  *
********************/
#ifdef _SERIAL_RX_BUFF
extern BOOL UARTaddDataToRxBuff(uint8 Data);
extern BOOL UARTgetDataFromRxBuff(uint8 *Data);
extern BOOL UARTPeekDataFromRxBuff(uint8 *pData);
#endif


#ifdef _SERIAL_TX_BUFF
extern BOOL UARTaddDataToTxBuff(uint8 Data);
extern BOOL UARTgetDataFromTxBuff(uint8 *Data);
extern BOOL UARTPeekDataFromTxBuff(uint8 *pData);
#endif



#endif 
