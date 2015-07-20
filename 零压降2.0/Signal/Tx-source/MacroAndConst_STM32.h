#ifndef _MACRO_AND_CONST_STM32_H_
#define _MACRO_AND_CONST_STM32_H_

/*****************************************
*    ������˵����ϵͳ���ú궨��������    *
*    �汾��      Ver1.0                  *
*    ���ߣ�      ZZJJHH250/zzjjhh250     *
*    ����ʱ�䣺  08/11/2010              *
*========================================*
*��֧�ֿ⡿                              *
*    ֧�ֿ����ƣ�                        *
*    ��Ӧ�汾  ��                        *
*    ��Ӧ˵��  ��                        *
*========================================*
*��ʹ��˵����
*    1����ͷ�ļ����ڻ����ļ���
*****************************************/

/****************************************
*         �����궨��                    *
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
*         �û��������Ͷ���              *
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
*         �����궨��                    *
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
