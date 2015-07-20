		 /***********************************************************
*   函数库说明：串行通讯缓冲区处理函数库                   *
*   版本：      v1.00                                      *
*   作者：      ZZJJHH250/zzjjhh250                        *
*   创建日期：  2010年8月18日                              *
* -------------------------------------------------------- *
*  [支 持 库]                                              *
*   支持库名称：MacroAndConst.h                            *
*   需要版本：  v1.0                                       *
*   函数库说明：系统常用宏定义库                           *
*                                                          *
*   支持库名称：UseSerialBuff.h                            *
*   需要版本：  -----                                      *
*   声明库说明：串行通讯缓冲区处理声明库                   *
* -------------------------------------------------------- *
*  [版本更新]                                              *
*   修改：                                                 *
*   修改日期：                                             *
*   版本：                                                 *

* -------------------------------------------------------- *
*  [版本历史]                                              *
*       v0.30  该版本提供了最基本的环形队列缓冲区操作函数。*
*       v0.31  增加了调用该函数时候的宏说明，可以选择需要  *
*              提供的缓冲区功能。                          *
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
***********************************************************/

/********************
* 头 文 件 配 置 区 *
********************/
# include "LIB_Config.h"
//# include "MacroAndConst.h"
# include "Serial_Buffer.h"

/********************
*   系 统 宏 定 义  *
********************/

/*------------------*
*   常 数 宏 定 义  *
*------------------*/
#ifndef _SERIAL_RX_BUFF_SIZE			   //如果前面没有宏定义 缓冲区大小 则这32BYTE作为默认大小
    # define SERIAL_RX_BUFF_SIZE 10
#endif

#ifndef _SERIAL_TX_BUFF_SIZE
    # define SERIAL_TX_BUFF_SIZE 10
#endif

/********************
*   函 数 声 明 区  *
********************/
#ifdef _SERIAL_RX_BUFF
BOOL UARTaddDataToRxBuff(uint8 Data);
BOOL UARTgetDataFromRxBuff(uint8 *Data);
#endif


#ifdef _SERIAL_TX_BUFF
BOOL UARTaddDataToTxBuff(uint8 Data);
BOOL UARTgetDataFromTxBuff(uint8 *Data);
#endif

/********************
*   模块变量定义区  *
********************/
#ifdef _SERIAL_RX_BUFF
static uint8  UARTRxBuff[SERIAL_RX_BUFF_SIZE]; //定义接受缓冲数组
static unsigned int  UARTRxBuffHead = 0;
static unsigned int  UARTRxBuffTail = 0;
static unsigned int  UARTRxBuffCounter = 0;
#endif

#ifdef _SERIAL_TX_BUFF
static uint8  UARTTxBuff[SERIAL_TX_BUFF_SIZE];
static unsigned int  UARTTxBuffHead = 0;
static unsigned int  UARTTxBuffTail = 0;
static unsigned int  UARTTxBuffCounter = 0;
#endif
 
/********************
*   全局变量定义区  *
********************/




/***********************************************************
*   函数说明：  接收缓冲区取值函数                         *
*   输入：      存储取出数据地址的指针                     *
*   输出：      取值是否成功                               *
*   调用函数：  无                                         *
***********************************************************/
#ifdef _SERIAL_RX_BUFF
BOOL UARTgetDataFromRxBuff(uint8  *Data)
{
    SAFE_CODE
    (
        if ((UARTRxBuffHead == UARTRxBuffTail) 
         && (UARTRxBuffCounter == 0))
        {
            SETI;
            return FALSE;
        }
        (*Data) = UARTRxBuff[UARTRxBuffHead++];
    
        UARTRxBuffCounter--;
    
        if (UARTRxBuffHead == SERIAL_RX_BUFF_SIZE)
        {
            UARTRxBuffHead = 0;
        }
    )		
    return TRUE;
}

/***********************************************************
*   函数说明：  接收缓冲区首数据察看函数                   *
*   输入：      存储取出数据地址的指针                     *
*   输出：      取值是否成功                               *
*   调用函数：  无                                         *
***********************************************************/
BOOL UARTPeekDataFromRxBuff(uint8 *pData)
{
    SAFE_CODE
    (
        if ((UARTRxBuffHead == UARTRxBuffTail) 
         && (UARTRxBuffCounter == 0))
        {
            SETI;
            return FALSE;
        }
        (*pData) = UARTRxBuff[UARTRxBuffHead];
    )		
    return TRUE;
}

/***********************************************************
*  函数说明：   向通讯输入缓冲区添加数据函数               *
*  输入：       收到的数据                                 *
*  输出：       无                                         *
*  调用函数：   无                                         *
***********************************************************/
BOOL UARTaddDataToRxBuff(uint8  Data)
{
    SAFE_CODE
    (
        if ((UARTRxBuffHead == UARTRxBuffTail) 
         && (UARTRxBuffCounter == SERIAL_RX_BUFF_SIZE))
        {
            SETI;
            return FALSE;
        }
        UARTRxBuff[UARTRxBuffTail++] = Data;
    
        UARTRxBuffCounter++;
    
        if (UARTRxBuffTail == SERIAL_RX_BUFF_SIZE)
        {
            UARTRxBuffTail = 0;
        }
	)	
    return TRUE;
}
#endif



//======================================================================================================


#ifdef _SERIAL_TX_BUFF											 //系统编译以下代码的 宏编译开关
/***********************************************************
*  函数说明：   发送中断取值函数                           *
*  输入：       存储取出数据地址的指针                     *
*  输出：       取值是否成功                               *
*  调用函数：   无                                         *
***********************************************************/
BOOL UARTgetDataFromTxBuff(uint8 *Data)
{



        if ((UARTTxBuffHead == UARTTxBuffTail) 
         && (UARTTxBuffCounter == 0))
        {

            return FALSE;
        }
        (*Data) = UARTTxBuff[UARTTxBuffHead++];
    
        UARTTxBuffCounter--;
    
        if (UARTTxBuffHead == SERIAL_TX_BUFF_SIZE)
        {
            UARTTxBuffHead = 0;
        }
		
    return TRUE;
}

/***********************************************************
*   函数说明：  首数据察看函数                             *
*   输入：      存储数据地址的指针                         *
*   输出：      取值是否成功                               *
*   调用函数：  无                                         *
***********************************************************/
BOOL UARTPeekDataFromTxBuff(uint8 *pData)
{

        if ((UARTTxBuffHead == UARTTxBuffTail) 
         && (UARTTxBuffCounter == 0))
        {

            return FALSE;
        }
        (*pData) = UARTTxBuff[UARTTxBuffHead];
	
    return TRUE;
}

/***********************************************************
*  函数说明：   向通讯输出缓冲区添加数据函数               *
*  输入：       需要发送的数据                             *
*  输出：       无                                         *
*  调用函数：   无                                         *
***********************************************************/
BOOL UARTaddDataToTxBuff(uint8 Data)
{
    
  if ((UARTTxBuffHead == UARTTxBuffTail) 
             && (UARTTxBuffCounter == SERIAL_TX_BUFF_SIZE))	  //查看数组是否已满，头尾相接有两种可能，1.满2.空 再加上COUNTER即可
        {

            return FALSE;
        }
        UARTTxBuff[UARTTxBuffTail++] = Data;	 //将数据加入串口发送缓冲区
    
        UARTTxBuffCounter ++;					//将缓冲区下标加1
    
        if (UARTTxBuffTail == SERIAL_TX_BUFF_SIZE)	  //当缓冲区满了之后 回到缓冲区开始处
        {
            UARTTxBuffTail = 0;
        }

    return TRUE;
} 
#endif
