#ifndef __485SW_H__
#define __485SW_H__

#include <rtdef.h>

enum rs485_uart {
	UART_UNKNOWN,
	EM_UART,
	AMMETER_UART,
	TL16_UART1,
	TL16_UART2,
	TL16_UART3,
	TL16_UART4,
	TL16_UART5,
	TL16_UART6,
	TL16_UART7,
	TL16_UART8,
};

enum ammeter_protocol_e {
	AP_PROTOCOL_UNKNOWN,
	AP_PROTOCOL_645_1997,
	AP_PROTOCOL_645_2007,
	AP_PROTOCOL_DLMS,
	AP_PROTOCOL_WS,
	AP_PROTOCOL_IEC1107,
	AP_PROTOCOL_ACTARIS,
	AP_PROTOCOL_EDMI,
	AP_PROTOCOL_SIMENS,

	AP_PROTOCOL_NOTHING
};

#define DEBUG_485SW
#ifdef DEBUG_485SW
#define ASSERT_485SW(x)   if (!(x)) {printf_syn("Assertion: (%s) assert failed at %s:%d \n", \
                            #x, __FUNCTION__, __LINE__); while (1);}
#else
#define ASSERT_485SW(x)
#endif

#define ERROR_485SW
#ifdef ERROR_485SW

#define ERROR_485SW_PRINTF(x) do {printf_syn("ERROR: %s failed at line %d in %s()\n", \
                                     x, __LINE__, __FUNCTION__);} while(0)
#else
#define ERROR_485SW_PRINTF(x)
#endif



extern void rt_485sw_em_init(void);
extern void uart_485_tl16_1_dev_init(void);
extern void uart_485_tl16_2_dev_init(void);
extern void uart_485_tl16_3_dev_init(void);
extern void uart_485_tl16_4_dev_init(void);
extern void uart_485_tl16_5_dev_init(void);
extern void uart_485_tl16_6_dev_init(void);
extern void uart_485_tl16_7_dev_init(void);
extern void uart_485_tl16_8_dev_init(void);
#endif



