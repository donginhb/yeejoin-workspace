#ifndef _USE_LIB_CONFIG_H_
#define _USE_LIB_CONFIG_H_
/***********************************************************
*   声明库说明：库函数配置声明库                           *
*   版本：                                                 *
*   作者：                                                 *
*   创建日期：                                             *
* -------------------------------------------------------- *
*  [硬件说明]                                              *
*   处理器：                                               *
*   系统时钟：                                             *
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
//# include "STC12C5616AD.h"
# include "MacroAndConst_STM32.h"	
# include "PF_Config.h"
# include "Universal_Buffer.h"





//串行缓冲区模块

//# define _SERIAL_TX_BUFF		//打开编译开关 编译发送函数
//# define SERIAL_TX_BUFF_SIZE    20

//# define _SERIAL_RX_BUFF			//打开编译开关 编译接收函数
//# define SERIAL_RX_BUFF_SIZE    32

//# include "Serial_Buffer.h"	


			  


//LCD1602显示模块
/*
# define LCD_RS         _PB0
# define LCD_RW         _PB1
# define LCD_E          _PB2
# define LCD_SET_WRITE_DATA DDRA = 0xff;
# define LCD_SET_READ_DATA  DDRA = 0x00;PORTA = 0xff;
# define LCD_BF         PIN_PA7
# define LCD_DATA_PORT  PORTA

# define LCD_RUN_STRING_SPEED   300
# define LCD_FLASH_TIME_OUT     0x03ff
# define _LCD1602_FULL_DATA_MODEL
# define _USE_LCD_ONLY_WRITE

# include "RD_LCD1602B.h"				    /////////////////////////////////////////////////////////////

*/
/*
//键盘处理模块
# define KEY_NULL       0xFF
# define _USE_KEY_LONG_PRESS_REPEAT
# define _USE_KEY_LONG_PRESS_DELAY_TIME         1024
# define _USE_KEY_LONG_KEY_FASTEST_SPEED_LEVEL  3
# define _USE_KEY_PRESS_SCAN_FUNC_INTERFACE Key_Press_Scan

# include "RD_UseKey.h"
*/
//595按键显示模块
/*
# define _USE_SPI
# define _USE_165_TO_GET_KEY_CODE
# define P_MISO     _PB6
# define P_SS       _PB4
# define P_SPDR     SPDR
# define SYS_TIMER_MS_ADD_UNTILL_OVF    g_wSystemTimer 
# define _USE_KEY_MAP

# include "RD_DispBoard_595.h"			 
*/

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
*    结构体定义区   *
********************/

/********************
*   函 数 引 用 区  *
********************/

/********************
*   全局变量引用区  *
********************/

#endif
