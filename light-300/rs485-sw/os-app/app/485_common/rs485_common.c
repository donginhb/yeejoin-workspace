#include "rtdef.h"
#include <rs485_common.h>
#include <rtthread.h>
#include <rs485.h>
#include <board.h>

#if 0
/******获取信号量*******/
static rt_err_t uart485_sem_take(enum frame_uart_485 port, rt_int32_t time)
{
        rt_err_t ret = RT_EOK;
	
        switch (port) {
        case FRAME_485_UART1:
                ret = rt_sem_take(&uart485_1_rx_byte_sem, time);
                break;
		
        case FRAME_485_UART2:
                ret = rt_sem_take(&uart485_2_rx_byte_sem, time);
                break;
		
        default:
                rt_kprintf("ERROR: uart error!\n");
                return RT_ERROR;
        }
	
        return ret;
}
#endif

#if 1
/*********************电能表串口格式转换**************************/
USART_TypeDef *get_485_port_ptr_from_uartno(enum frame_uart_485 port)
{
	USART_TypeDef *ptr;

	switch (port) {
	case FRAME_485_UART1:
		ptr = UART_485_1_DEV_PTR;
		break;

	case FRAME_485_UART2:
		ptr = UART_485_2_DEV_PTR;
		break;
	case FRAME_485_UART3:
		ptr = NULL;
		break;
	default:
		ptr = NULL;
		break;
	}

	return ptr;
}
#endif


