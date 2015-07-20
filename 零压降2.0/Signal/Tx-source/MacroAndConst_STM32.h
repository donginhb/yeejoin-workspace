#ifndef _MACRO_AND_CONST_STM32_H_
#define _MACRO_AND_CONST_STM32_H_

/*****************************************
*    声明库说明：系统常用宏定义声明库    *
*    版本：      Ver1.0                  *
*    作者：      ZZJJHH250/zzjjhh250     *
*    创建时间：  08/11/2010              *
*========================================*
*【支持库】                              *
*    支持库名称：                        *
*    对应版本  ：                        *
*    对应说明  ：                        *
*========================================*
*【使用说明】
*    1、该头文件属于基础文件库
*****************************************/

/****************************************
*         参数宏定义                    *
*****************************************/



#ifndef HIGH
#define HIGH (0x01)
#endif

#ifndef LOW
#define LOW (0x00)
#endif

#ifndef PI
#define PI (3.1415926535897932f)
#endif
/****************************************
*         用户变量类型定义              *
*****************************************/
typedef    unsigned int    uint16;
typedef    unsigned int    UINT16;
typedef    unsigned int    word;
typedef    unsigned int    WORD;
typedef    int             int16;
typedef    int             INT16;
/*======================================*/
typedef    unsigned long    uint32;
typedef    unsigned long    UINT32;
typedef    unsigned long    dword;
typedef    unsigned long    DWORD;
typedef    long             int32;
typedef    long             INT32;
/*======================================*/
typedef    unsigned char    uint8;
typedef    unsigned char    UINT8;
typedef    unsigned char    byte;
typedef    unsigned char    BYTE;
typedef    char             int8;
typedef    char             INT8;

typedef    unsigned char    BOOL;
/*======================================*/

/****************************************
*         动作宏定义                    *
*****************************************/

#define BIN(a,b,c,d,e,f,g,h)   ((a << 7)|(b << 6)|(c << 5)|(d << 4)|(e << 3)|(f << 2)|(g << 1)|(h << 0))
#define bin BIN

#ifndef NULL
#define NULL (0x00)
#endif


#define SET(reg , n)    reg |= BIT(n)
#define CLR(reg , n)    reg &= ~BIT(n)

#define MAX(a , b)    (((a)>(b))?(a) : (b))
#define MIN(a , b)    (((a)<(b))?(a) : (b))


#endif
