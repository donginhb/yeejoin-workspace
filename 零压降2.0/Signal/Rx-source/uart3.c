#include "..\Source\LIB_Config.h"
#include "..\Source\PF_Config.H"
#include ".\fwlib\stm32f10x.h"

#include "stm32f10x.h"
#include "uart3.h"
#include "Universal_Buffer.h"
#include "sys_comm.h"
#include "board.h"
#include "info_tran.h"


#define USART3_RX_BUF_SIZE	8	/*必须是2的幂*/
#define USART3_TX_BUF_SIZE	8	/*必须是2的幂 2k*/

static u8 USART3_RXBuffer[USART3_RX_BUF_SIZE]; 	 //USART2 发送缓冲区FIFO
static cBuffer  USART3_tRXBufferMana;			 //管理结构体变量

void USART3_RxBufferInit(void)
{
	bufferInit(&USART3_tRXBufferMana, USART3_RXBuffer, USART3_RX_BUF_SIZE);
}

u8 USART3_PutDatatoRxBuffer(u8 dat)
{
	return ( bufferAddToEnd(&USART3_tRXBufferMana,dat ));
}

u16 USART3_GetRxBufferLeftLength(void)
{
	return(bufferIsNotFull(&USART3_tRXBufferMana));
}

u8 USART3_GetRxBufferData( void )
{
	return (bufferGetFromFront(&USART3_tRXBufferMana));
}

void USART3_FlushRxBuffer( void )
{
	bufferFlush(&USART3_tRXBufferMana) ;
}

u16 USART3_GetRxBufferCurrentSize( void )
{
	return (bufferGetSize(&USART3_tRXBufferMana)) ;
}

void USART3_GetBytesFromRxFIFO( u8 *pdat ,u16 length)
{
	u16 num = 0;
	while(num < length) {
		*(pdat++) = USART3_GetRxBufferData();
		num++;
	}
}

extern void write_byte2lcd_cpu(u8 DATA);
void proc_data_uart3(void)
{
	static int cmd, param;
	static int recv_state = UART3_RECV_NULL;
	
	disable_usartx_recv_int(USART3);
	switch (recv_state) {
	case UART3_RECV_NULL:
		if(0 != get_uart3_buf_len_fn()) {
			cmd = get_uart3_buf_data_fn();
			recv_state = UART3_RECV_1_BYTE;
		} else {
			break;
		}

		if(0 != get_uart3_buf_len_fn()) {
			param = get_uart3_buf_data_fn();
			recv_state = UART3_RECV_2_BYTE;
		} else {
			break;
		}
		break;
	
	case UART3_RECV_1_BYTE:
		if(0 != get_uart3_buf_len_fn()) {
			param = get_uart3_buf_data_fn();
			recv_state = UART3_RECV_2_BYTE;
		} else {
			break;
		}
		break;

	default:
		break;
	}		
	if (recv_state == UART3_RECV_2_BYTE) {
		recv_state = UART3_RECV_NULL;
		if ((cmd == CMD_HEAD)) {
			switch (param) {
			case SWITCH2PT:
				/* 切换到PT */
				force_output_switch2pt(UIUS_SWITCH_CMD_FROM_LCD_CPU);

				write_byte2lcd_cpu(creat_info_id(ITID_RXE_ACK,TRX_SWITCH2PT));
				write_byte2lcd_cpu(0x00);
				break;

			case SWITCH_FROM_PT:
				cancel_force_output_switch2pt(UIUS_SWITCH_CMD_FROM_LCD_CPU);
				break;

			default:
				break;
			}
		} else {
			flush_uart3_buf_fn();
		}		
	}
	enable_usartx_recv_int(USART3);
}

/*
void USART3_TxBufferInit(void)
{
	bufferInit(&USART3_tTXBufferMana, USART3_TXBuffer, USART3_TX_BUF_SIZE);
}
*/

/*
u8 USART3_PutDatatoTxBuffer(u8 dat)
{
	return ( bufferAddToEnd(&USART3_tTXBufferMana,dat ));
}
*/

/*
u16 USART3_GetTxBufferLeftLength(void)
{
	return(bufferIsNotFull(&USART3_tTXBufferMana));
}
*/

/*
u8 USART3_GetTxBufferData( void )
{
	return (bufferGetFromFront(&USART3_tTXBufferMana));
}
*/

/*
void USART3_FlushTxBuffer( void )
{
	bufferFlush(&USART3_tTXBufferMana) ;
}
*/

/*
u16 USART3_GetTxBufferCurrentSize( void )
{
	return (bufferGetSize(&USART3_tTXBufferMana)) ;
}
*/

/*
void UASRT3_BeginSend(void)
{
	USART_ITConfig(USART3,USART_IT_TXE,ENABLE);
}

void UASRT3_StopSend(void)
{
	USART_ITConfig(USART3,USART_IT_TXE,DISABLE);
}

void USART3_QueryPutChar( u8 dat )
{

	USART_SendData(USART3 , (u8)(dat));
	while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);

}

void USART3_QueryPutMultiChar( u8 *pdat ,u16 length)
{
	u16 num = 0;
	while(num++ < length) {
		USART3_QueryPutChar((u8)(*(pdat++)));
	}
}

void USART3_PutBytesToTxFIFO( u8 *pdat ,u16 length)
{
	u16 num = 0;
	while(num++ < length) {
		USART3_PutDatatoRxBuffer((u8)(*(pdat++)));
	}
}
*/

