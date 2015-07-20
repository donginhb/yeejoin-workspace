#ifndef RS485_COMMON_
#define RS485_COMMON_

#include "rtdef.h"




/*** 电能表所用串口 ***/
enum frame_uart_485 {
	FRAME_485_UART1 = 1,
	FRAME_485_UART2,
	FRAME_485_UART3,	
};
#define READ_MAX_LEN  220 /* 从站异常应答帧固定长度*/
#define WRITE_MAX_LEN 50

#define FRAME_485_LEN 250 
/********* 解析645 *********/
enum frame485_e {
	FRAME485_EOK   =	0x00,   /* 执行正确，返回0 */
	FRAME485_ERROR,         /* 一般性错误 */
	FRAME485_REVE_TIMEOUT,	/* 没有接收到485帧数据,接收超时 */
	FRAME485_FRAG,     	/* 此帧是个片段 */
 	FRAME485_CS_ERR,        /* 校验错误 */
 	FRAME485_DATA_ERR,      /* 数据错误 */
 	FRAME485_FLAG_ERROR,    /* 不能识别此数据标识 */
 	FRAME485_ADDR_MISMATCH,	/* 接收帧数据与发送命令地址不匹配 */
 	FRAME485_FLAG_MISMATCH,	/* 接收帧数据与发送命令数据标识不匹配 */
};




#endif
