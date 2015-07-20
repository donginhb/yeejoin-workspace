/**********************************************************************************
* Filename   	: ammeter_usart.h
* Description 	: define the functions that ammeters Communication port
* Begin time  	: 2014-01-07
* Finish time 	: 
* Engineer		: zhanghonglei
*************************************************************************************/
#ifndef __AMMETER_USART__
#define __AMMETER_USART__

#include <rs485.h>
#include <board.h>
#include <rtthread.h>
#include <ammeter.h>
#include <ammeter_common.h>

#define USING_RS485SW_USART

#define EMC_AUTO_ENTER_NEGOTIATION_UART	USART2


extern void ammeter_port_mutex_init(void);
extern rt_err_t ammeter_mutex_take(enum ammeter_uart_e port, rt_int32_t time);
extern rt_err_t ammeter_mutex_release(enum ammeter_uart_e port);
extern USART_TypeDef *get_485_port_from_ammeter_uart(enum ammeter_uart_e port);
extern rt_err_t recv_data_from_485(enum ammeter_uart_e port, rt_uint8_t recv_buf[256],
		rt_uint32_t *recv_len, rt_int32_t format_time_out, rt_int32_t byte_time_out);
extern rt_err_t set_485sw_and_em_usart(struct uart_485_param data, enum ammeter_uart_e port_485,
		enum ammeter_protocal_e amm_protocal);

extern void init_auto_negotiation_timer(void);
extern rt_size_t em_protoco_send_data_by_485(USART_TypeDef *dev_485, void *data, rt_size_t len);

extern int reset_auto_negotiation_timer(void);
extern int is_emc_in_negotiation_state_f(void);
#endif
