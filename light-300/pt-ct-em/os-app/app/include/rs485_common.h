#ifndef RS485_COMMON_
#define RS485_COMMON_

#include "rtdef.h"




/*** ���ܱ����ô��� ***/
enum frame_uart_485 {
	FRAME_485_UART1 = 1,
	FRAME_485_UART2,
	FRAME_485_UART3,	
};
#define READ_MAX_LEN  220 /* ��վ�쳣Ӧ��֡�̶�����*/
#define WRITE_MAX_LEN 50

#define FRAME_485_LEN 250 
/********* ����645 *********/
enum frame485_e {
	FRAME485_EOK   =	0x00,   /* ִ����ȷ������0 */
	FRAME485_ERROR,         /* һ���Դ��� */
	FRAME485_REVE_TIMEOUT,	/* û�н��յ�485֡����,���ճ�ʱ */
	FRAME485_FRAG,     	/* ��֡�Ǹ�Ƭ�� */
 	FRAME485_CS_ERR,        /* У����� */
 	FRAME485_DATA_ERR,      /* ���ݴ��� */
 	FRAME485_FLAG_ERROR,    /* ����ʶ������ݱ�ʶ */
 	FRAME485_ADDR_MISMATCH,	/* ����֡�����뷢�������ַ��ƥ�� */
 	FRAME485_FLAG_MISMATCH,	/* ����֡�����뷢���������ݱ�ʶ��ƥ�� */
};




#endif
