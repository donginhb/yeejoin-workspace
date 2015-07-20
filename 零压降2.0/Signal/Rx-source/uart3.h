#ifndef __UART3_H
#define __UART3_H

#define get_uart3_buf_data_fn USART3_GetRxBufferData
#define put_data2uart3_buf_fn USART3_PutDatatoTxBuffer
#define get_uart3_buf_len_fn  USART3_GetRxBufferCurrentSize
#define flush_uart3_buf_fn    USART3_FlushRxBuffer


enum uart3_recv_state {
	UART3_RECV_NULL     = 0,
	UART3_RECV_1_BYTE   = 1,
	UART3_RECV_2_BYTE   = 2,
};


void USART3_RxBufferInit(void);
u8 USART3_PutDatatoRxBuffer(u8 dat);
u16 USART3_GetRxBufferLeftLength(void);
u8 USART3_GetRxBufferData(void);
void USART3_FlushRxBuffer(void);
u16 USART3_GetRxBufferCurrentSize(void);
void USART3_GetBytesFromRxFIFO(u8 *pdat ,u16 length);
void proc_data_uart3(void);







#endif


