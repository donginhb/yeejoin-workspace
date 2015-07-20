/**********************************************************************************
* Filename   	: ammeter_usart.c
* Description 	: define the functions that ammeters Communication port
* Begin time  	: 2014-01-07
* Finish time 	:
* Engineer		: zhanghonglei
*************************************************************************************/
#include "485sw.h"
#include <rs485.h>
#include <rtdef.h>
#include <board.h>
#include <sys_cfg_api.h>

#define UART_485_ANALYSIS_ENABLE 1
#define UART_485_ANALYSIS_DISABLE 0

#define UART_485_ANALYSIS_START 0
#define UART_485_ANALYSIS_OK 1
#define UART_485_ANALYSIS_UART_FRAME 2
#define UART_485_ANALYSIS_NO 3

rt_uint8_t UART_485_ANALYSIS_control = UART_485_ANALYSIS_ENABLE;

rt_uint8_t sn_record[NUM_OF_COLLECT_EM_MAX][6]; /* 电表SN号记录 */
enum ammeter_protocal protocal_record[NUM_OF_COLLECT_EM_MAX];

struct uart_param tl16_485_data_record[TL16_UART_NUM];
enum ammeter_protocal tl16_485_protocal_record[TL16_UART_NUM];

struct uart_param em_rs485_param_record;
struct uart_param amm_rs485_param_record;
struct uart_param em_rs485_param_default;

static struct rt_semaphore uart485_ammeter_sem;

rt_uint8_t em_take_sem_record = 0;
rt_uint8_t tl16_take_sem_record = 0;

static void rt_em_thread_entry(void* parameter);
static void rt_uart485_tl16_1_thread_entry(void* parameter);
static void rt_uart485_tl16_2_thread_entry(void* parameter);
static void rt_uart485_tl16_3_thread_entry(void* parameter);
static void rt_uart485_tl16_4_thread_entry(void* parameter);
static void rt_uart485_tl16_5_thread_entry(void* parameter);
static void rt_uart485_tl16_6_thread_entry(void* parameter);
static void rt_uart485_tl16_7_thread_entry(void* parameter);
static void rt_uart485_tl16_8_thread_entry(void* parameter);

static rt_err_t do_recv_send_em_data(enum rs485_uart em_port, enum rs485_uart amm_port);
static rt_err_t do_recv_send_tl16_485_data(enum rs485_uart tl16_port, enum rs485_uart amm_port);

static rt_err_t recv_data_from_em_send_to_amm_port(enum rs485_uart recv_from_em_port, enum rs485_uart send_to_amm_port, rt_int32_t recv_format_time_out, rt_int32_t recv_byte_time_out);
static rt_err_t recv_data_from_amm_send_to_em_port(enum rs485_uart recv_from_amm_port, enum rs485_uart send_to_em_port, rt_int32_t recv_format_time_out, rt_int32_t recv_byte_time_out);
static rt_err_t recv_data_from_tl16_485_send_to_amm(enum rs485_uart recv_from_tl16_port, enum rs485_uart send_to_amm_port, rt_int32_t recv_format_time_out, rt_int32_t recv_byte_time_out);
static rt_err_t recv_data_from_amm_send_to_tl16_port(enum rs485_uart recv_from_amm_port, enum rs485_uart send_to_tl16_port, rt_int32_t recv_format_time_out, rt_int32_t recv_byte_time_out);

static rt_err_t uart485_wait_em_amm_revc_data_event(enum rs485_uart port, rt_int32_t time);
static USART_TypeDef *get_port_from_rs485_uart(enum rs485_uart port);
static rt_device_t get_port_from_tl16_485(enum rs485_uart port);
static rt_err_t event_take_tl16_485(enum rs485_uart port, rt_int32_t time);
//static rt_err_t create_frame_em(rt_uint8_t *send_frame_buf, rt_uint32_t *send_frame_len, struct uart_param data);
static rt_err_t analysis_frame_em(rt_uint8_t *recv_frame_buf, rt_uint32_t recv_frame_len, struct uart_param *data);
static rt_err_t analysis_frame_645(rt_uint8_t *recv_frame_buf, rt_uint32_t recv_frame_length);
static rt_err_t analysis_frame_edmi(rt_uint8_t *recv_frame_buf, rt_uint32_t recv_frame_length);
static rt_err_t analysis_frame_tl16(rt_uint8_t *recv_frame_buf, rt_uint32_t recv_frame_len, struct uart_param *data, enum rs485_uart port);

static rt_err_t uart_485_if_set(struct uart_param *data, enum rs485_uart port);
static rt_err_t str_sn_change_to_bcd_sn(rt_uint8_t *input_str_sn, rt_uint8_t *output_bcd_sn);
static rt_err_t get_flash_amm_addr(void);
rt_err_t memcmp_and_set_uart485_baud_data(struct uart_param *cs, struct uart_param *ct, enum rs485_uart port);

void rt_485sw_em_init(void)
{
	rt_thread_t thread_h;

	thread_h = rt_thread_create("485_em", rt_em_thread_entry, RT_NULL, 2048, 25, 10);

	if (thread_h != RT_NULL)
		rt_thread_startup(thread_h);

	return;
}

void uart_485_tl16_1_dev_init(void)
{
	rt_thread_t thread_h;

	thread_h = rt_thread_create("tl16_1", rt_uart485_tl16_1_thread_entry, RT_NULL, 2048, 20, 10);

	if (thread_h != RT_NULL)
		rt_thread_startup(thread_h);

	return;
}

void uart_485_tl16_2_dev_init(void)
{
	rt_thread_t thread_h;

	thread_h = rt_thread_create("tl16_2", rt_uart485_tl16_2_thread_entry, RT_NULL, 2048, 20, 10);

	if (thread_h != RT_NULL)
		rt_thread_startup(thread_h);

	return;
}

void uart_485_tl16_3_dev_init(void)
{
	rt_thread_t thread_h;

	thread_h = rt_thread_create("tl16_3", rt_uart485_tl16_3_thread_entry, RT_NULL, 2048, 20, 10);

	if (thread_h != RT_NULL)
		rt_thread_startup(thread_h);

	return;
}

void uart_485_tl16_4_dev_init(void)
{
	rt_thread_t thread_h;

	thread_h = rt_thread_create("tl16_4", rt_uart485_tl16_4_thread_entry, RT_NULL, 2048, 20, 10);

	if (thread_h != RT_NULL)
		rt_thread_startup(thread_h);

	return;
}

void uart_485_tl16_5_dev_init(void)
{
	rt_thread_t thread_h;

	thread_h = rt_thread_create("tl16_5", rt_uart485_tl16_5_thread_entry, RT_NULL, 2048, 20, 10);

	if (thread_h != RT_NULL)
		rt_thread_startup(thread_h);

	return;
}

void uart_485_tl16_6_dev_init(void)
{
	rt_thread_t thread_h;

	thread_h = rt_thread_create("tl16_6", rt_uart485_tl16_6_thread_entry, RT_NULL, 2048, 20, 10);

	if (thread_h != RT_NULL)
		rt_thread_startup(thread_h);

	return;
}

void uart_485_tl16_7_dev_init(void)
{
	rt_thread_t thread_h;

	thread_h = rt_thread_create("tl16_7", rt_uart485_tl16_7_thread_entry, RT_NULL, 2048, 20, 10);

	if (thread_h != RT_NULL)
		rt_thread_startup(thread_h);

	return;
}

void uart_485_tl16_8_dev_init(void)
{
	rt_thread_t thread_h;

	thread_h = rt_thread_create("tl16_8", rt_uart485_tl16_8_thread_entry, RT_NULL, 2048, 20, 10);

	if (thread_h != RT_NULL)
		rt_thread_startup(thread_h);

	return;
}

static void rt_em_thread_entry(void* parameter)
{
	rt_sem_init(&uart485_ammeter_sem, "u485_ammeter_used", 1, RT_IPC_FLAG_PRIO);

	rt_memset(&em_rs485_param_record, 0, sizeof(struct uart_param));
	rt_memset(&amm_rs485_param_record, 0, sizeof(struct uart_param));
	rt_memset(&em_rs485_param_default, 0, sizeof(struct uart_param));

	em_rs485_param_record.baudrate = 2400;
	em_rs485_param_record.databits = 9;
	em_rs485_param_record.paritybit = 1;
	em_rs485_param_record.stopbits = 1;

	rt_memcpy(&amm_rs485_param_record, &em_rs485_param_record, sizeof(struct uart_param));
	rt_memcpy(&em_rs485_param_default, &em_rs485_param_record, sizeof(struct uart_param));

	rt_thread_delay(20);

	while (1) {
		do_recv_send_em_data(EM_UART, AMMETER_UART);
	}

	return;
}

static void rt_uart485_tl16_1_thread_entry(void* parameter)
{
	rt_thread_delay(20);

	if (RT_EOK != get_flash_amm_addr()) {
		ERROR_485SW_PRINTF(" ");
		return;
	}

	while (1) {
		do_recv_send_tl16_485_data(TL16_UART1, AMMETER_UART);
	}

	return;
}

static void rt_uart485_tl16_2_thread_entry(void* parameter)
{
	rt_thread_delay(20);

	while (1) {
		do_recv_send_tl16_485_data(TL16_UART2, AMMETER_UART);
	}

	return;
}
static void rt_uart485_tl16_3_thread_entry(void* parameter)
{
	rt_thread_delay(20);

	while (1) {
		do_recv_send_tl16_485_data(TL16_UART3, AMMETER_UART);
	}

	return;
}
static void rt_uart485_tl16_4_thread_entry(void* parameter)
{
	rt_thread_delay(20);

	while (1) {
		do_recv_send_tl16_485_data(TL16_UART4, AMMETER_UART);
	}

	return;
}
static void rt_uart485_tl16_5_thread_entry(void* parameter)
{
	rt_thread_delay(20);

	while (1) {
		do_recv_send_tl16_485_data(TL16_UART5, AMMETER_UART);
	}

	return;
}
static void rt_uart485_tl16_6_thread_entry(void* parameter)
{
	rt_thread_delay(20);

	while (1) {
		do_recv_send_tl16_485_data(TL16_UART6, AMMETER_UART);
	}

	return;
}
static void rt_uart485_tl16_7_thread_entry(void* parameter)
{
	rt_thread_delay(20);

	while (1) {
		do_recv_send_tl16_485_data(TL16_UART7, AMMETER_UART);
	}

	return;
}
static void rt_uart485_tl16_8_thread_entry(void* parameter)
{
	rt_thread_delay(20);

	while (1) {
		do_recv_send_tl16_485_data(TL16_UART8, AMMETER_UART);
	}

	return;
}

static rt_err_t do_recv_send_em_data(enum rs485_uart em_port, enum rs485_uart amm_port)
{
	rt_err_t ret = RT_ERROR;

	ret = recv_data_from_em_send_to_amm_port(em_port, amm_port, 60000, 100);
	if(ret == RT_EOK) {
		if (RT_EOK != recv_data_from_amm_send_to_em_port(amm_port, em_port, 300, 100)) {
			ERROR_485SW_PRINTF(" ");
	//		return RT_ERROR;
		}

	} else if (ret == RT_ETIMEOUT){
		if (RT_EOK != memcmp_and_set_uart485_baud_data(&em_rs485_param_record, &em_rs485_param_default, em_port)) {
			ERROR_485SW_PRINTF(" ");
		//	return RT_ERROR;
		}
	}

	if (em_take_sem_record == 1) {
		if (RT_EOK != rt_sem_release(&uart485_ammeter_sem)) {
			ERROR_485SW_PRINTF(" ");
			return RT_ERROR;
		}
		em_take_sem_record = 0;
	}

	return RT_EOK;
}

static rt_err_t recv_data_from_em_send_to_amm_port(enum rs485_uart recv_from_em_port, enum rs485_uart send_to_amm_port, rt_int32_t recv_format_time_out, rt_int32_t recv_byte_time_out)
{
	rt_uint8_t flag = UART_485_ANALYSIS_START;
	rt_uint32_t len = 0;
	rt_uint32_t tmp = 0;
	rt_uint8_t buf[256] = {"\0"};
	rt_uint8_t buffer_ok[] = {0x57, 0xFF, 0xFF, 0xFF, 0x57, 0x81, 0x00, 0x2C, 0x75}; /*485sw正常回答em的请求， OK */
	struct uart_param data;

	rt_memset(&data, 0, sizeof(struct uart_param));

	if (RT_EOK == uart485_wait_em_amm_revc_data_event(recv_from_em_port, recv_format_time_out)) {
		tmp = recv_data_by_485(get_port_from_rs485_uart(recv_from_em_port), buf, sizeof(buf));
		len = tmp;
#if 0
	rt_uint32_t k = 0;
	printf_syn("recv_data_by_485 start (EM_UART): ");
	for (k= 0; k < len; k++) {
		printf_syn("%02x ", buf[k]);
    }
	printf_syn("\n");
#endif
		while (1) {
			if (len >= 5 && flag == UART_485_ANALYSIS_START) {
				if (*buf != 0x57 || *(buf + 4) != 0x57) {
					if (RT_EOK == rt_sem_take(&uart485_ammeter_sem, 0)) {
						em_take_sem_record = 1;
						if (RT_EOK != memcmp_and_set_uart485_baud_data(&amm_rs485_param_record, &em_rs485_param_record, send_to_amm_port)) {
							ERROR_485SW_PRINTF(" ");
							return RT_ERROR;
						}

						send_data_by_485(get_port_from_rs485_uart(send_to_amm_port), buf, len);
#if 0
	rt_uint32_t k = 0;
	printf_syn("send_data_by_485 start (AMM_UART): ");
	for (k= 0; k < len; k++) {
		printf_syn("%02x ", buf[k]);
    }
	printf_syn("\n");
#endif

						if (RT_EOK != rt_sem_release(&uart485_ammeter_sem)) {
							ERROR_485SW_PRINTF(" ");
							return RT_ERROR;
						}

						em_take_sem_record = 0;

						flag = UART_485_ANALYSIS_OK;

					} else {
						flag = UART_485_ANALYSIS_NO;
						ERROR_485SW_PRINTF(" ");
	//					return RT_ERROR;
					}
				} else {
					flag = UART_485_ANALYSIS_UART_FRAME;
				}
			} else if (flag == UART_485_ANALYSIS_OK) {
				if (RT_EOK == rt_sem_take(&uart485_ammeter_sem, 0)) {
					em_take_sem_record = 1;
					send_data_by_485(get_port_from_rs485_uart(send_to_amm_port), buf+len, tmp);
#if 0
	rt_uint32_t k = 0;
	printf_syn("send_data_by_485 second (AMM_UART): ");
	for (k= 0; k < tmp; k++) {
		printf_syn("%02x ", buf[k+len]);
    }
	printf_syn("\n");
#endif
				} else {
					flag = UART_485_ANALYSIS_NO;
					ERROR_485SW_PRINTF(" ");
		//			return RT_ERROR;
				}
			}

			if (RT_EOK == uart485_wait_em_amm_revc_data_event(recv_from_em_port, recv_byte_time_out)) {
				tmp = recv_data_by_485(get_port_from_rs485_uart(recv_from_em_port), buf + len, sizeof(buf)-len);
#if 0
	rt_uint32_t k = 0;
	printf_syn("recv_data_by_485 second (EM_UART): ");
	for (k= 0; k < len; k++) {
		printf_syn("%02x ", buf[k]);
    }
	printf_syn("\n");
#endif
				if (flag == UART_485_ANALYSIS_UART_FRAME || len < 5 || flag == UART_485_ANALYSIS_NO) {
					len += tmp;
				}

				if (em_take_sem_record == 1) {
					if (RT_EOK != rt_sem_release(&uart485_ammeter_sem)) {
						ERROR_485SW_PRINTF(" ");
						return RT_ERROR;
					}

					em_take_sem_record = 0;
				}
			} else {
				break;
			}

			if (len >= sizeof(buf))
				break;
		}

		if (flag == UART_485_ANALYSIS_UART_FRAME) {
			if (RT_EOK == analysis_frame_em(buf, len, &data)) {

				send_data_by_485(get_port_from_rs485_uart(recv_from_em_port), buffer_ok, sizeof(buffer_ok));

				if (RT_EOK != memcmp_and_set_uart485_baud_data(&em_rs485_param_record, &data, recv_from_em_port)) {
					ERROR_485SW_PRINTF(" ");
					return RT_ERROR;
				}
				return RT_EIO;
			}
		}

		return RT_EOK;
	} else {
		ERROR_485SW_PRINTF(" ");
		return RT_ETIMEOUT;
	}

	return RT_ERROR;
}

static rt_err_t recv_data_from_amm_send_to_em_port(enum rs485_uart recv_from_amm_port, enum rs485_uart send_to_em_port, rt_int32_t recv_format_time_out, rt_int32_t recv_byte_time_out)
{
	rt_uint32_t tmp = 0;
	rt_uint32_t len = 0;
	rt_uint8_t buf[256] = {'\0'};

	if (RT_EOK == uart485_wait_em_amm_revc_data_event(recv_from_amm_port, recv_format_time_out)) {
		tmp = recv_data_by_485(get_port_from_rs485_uart(recv_from_amm_port), buf, sizeof(buf));
#if 0
	rt_uint32_t k = 0;
	printf_syn("recv_data_by_485 start (AMM_UART): ");
	for (k= 0; k < tmp; k++) {
		printf_syn("%02x ", buf[k]);
    }
	printf_syn("\n");
#endif
		if (tmp > 0) {
			send_data_by_485(get_port_from_rs485_uart(send_to_em_port), buf, tmp);
			len = tmp;
#if 0
	rt_uint32_t k = 0;
	printf_syn("send_data_by_485 start (EM_UART): ");
	for (k= 0; k < tmp; k++) {
		printf_syn("%02x ", buf[k]);
    }
	printf_syn("\n");
#endif
		}

		while (1) {
			rt_memset(buf, 0, sizeof(buf));
			if (RT_EOK == uart485_wait_em_amm_revc_data_event(recv_from_amm_port, recv_byte_time_out)) {
				tmp = recv_data_by_485(get_port_from_rs485_uart(recv_from_amm_port), buf, sizeof(buf));
#if 0
	rt_uint32_t k = 0;
	printf_syn("recv_data_by_485 second (AMM_UART): ");
	for (k= 0; k < tmp; k++) {
		printf_syn("%02x ", buf[k]);
    }
	printf_syn("\n");
#endif
				if (tmp > 0) {
					send_data_by_485(get_port_from_rs485_uart(send_to_em_port), buf, tmp);
					len += tmp;
#if 0
	rt_uint32_t k = 0;
	printf_syn("send_data_by_485 start (EM_UART): ");
	for (k= 0; k < tmp; k++) {
		printf_syn("%02x ", buf[k]);
    }
	printf_syn("\n");
#endif
				}

			} else {
				break;
			}

			if (len >= sizeof(buf))
				break;
		}
	} else {
		ERROR_485SW_PRINTF(" ");
		return RT_ERROR;
	}

	return RT_EOK;
}




static rt_err_t do_recv_send_tl16_485_data(enum rs485_uart tl16_port, enum rs485_uart amm_port)
{
	UART_485_ANALYSIS_control = UART_485_ANALYSIS_ENABLE;

	if(RT_EOK == recv_data_from_tl16_485_send_to_amm(tl16_port, amm_port, RT_WAITING_FOREVER, 100)) {
		if (RT_EOK != recv_data_from_amm_send_to_tl16_port(amm_port, tl16_port, 300, 100)) {
			ERROR_485SW_PRINTF(" ");
//			return RT_ERROR;
		}

	} else {
//		ERROR_485SW_PRINTF(" ");
		printf_syn("ERROR: port(%d) failed at line %d in %s()\n", tl16_port - 2, __LINE__, __FUNCTION__);
//		return RT_ERROR;
	}

	if (tl16_take_sem_record == 1) {
		if (RT_EOK != rt_sem_release(&uart485_ammeter_sem)) {
			ERROR_485SW_PRINTF(" ");
			return RT_ERROR;
		}
		tl16_take_sem_record = 0;
	}

	return RT_EOK;
}

static rt_err_t recv_data_from_tl16_485_send_to_amm(enum rs485_uart recv_from_tl16_port, enum rs485_uart send_to_amm_port, rt_int32_t recv_format_time_out, rt_int32_t recv_byte_time_out)
{
	rt_uint8_t flag = UART_485_ANALYSIS_START;
	rt_uint32_t len = 0;
	rt_uint32_t tmp = 0;
	rt_uint8_t buf[256] = {"\0"};

	struct uart_param data;

	rt_memset(&data, 0, sizeof(struct uart_param));

	if (RT_EOK == event_take_tl16_485(recv_from_tl16_port, recv_format_time_out)) {
		tmp = recv_data_by_tl16_485(get_port_from_tl16_485(recv_from_tl16_port), buf, sizeof(buf));
		len = tmp;

		while (1) {
			if (len >= 8 && flag == UART_485_ANALYSIS_START && UART_485_ANALYSIS_control == UART_485_ANALYSIS_ENABLE) {
				if (RT_EOK == analysis_frame_tl16(buf, len, &data, recv_from_tl16_port)) {
					UART_485_ANALYSIS_control = UART_485_ANALYSIS_DISABLE;
#if 0
	rt_uint32_t k = 0;
	printf_syn("recv_data_by_tl16_485(%d) start: ", recv_from_tl16_port - 2);
	for (k= 0; k < len;k++) {
		printf_syn("%02x ", buf[k]);
    }
	printf_syn("\n");
#endif

					if (RT_EOK == rt_sem_take(&uart485_ammeter_sem, get_ticks_of_ms(400))) {
						tl16_take_sem_record = 1;

						if (RT_EOK != memcmp_and_set_uart485_baud_data(&amm_rs485_param_record, &data, send_to_amm_port)) {
							ERROR_485SW_PRINTF(" ");
							return RT_ERROR;
						}

						send_data_by_485(get_port_from_rs485_uart(send_to_amm_port), buf, len);
						flag = UART_485_ANALYSIS_OK;
#if 0
	rt_uint32_t k = 0;
	printf_syn("send_data_by_485(%d) start: ", recv_from_tl16_port - 2);
	for (k= 0; k < len;k++) {
		printf_syn("%02x ", buf[k]);
	}
	printf_syn("\n");
#endif

					} else {
						ERROR_485SW_PRINTF(" ");
						return RT_ERROR;
					}
				}
			} else if (flag == UART_485_ANALYSIS_OK) {
#if 0
	rt_uint32_t k = 0;
	printf_syn("recv_data_by_tl16_485(%d) second: ", recv_from_tl16_port - 2);
	for (k= 0; k < tmp; k++) {
		printf_syn("%02x ", buf[k+len]);
    }
	printf_syn("\n");
#endif
				send_data_by_485(get_port_from_rs485_uart(send_to_amm_port), buf+len, tmp);
#if 0
//	rt_uint32_t k = 0;
	printf_syn("send_data_by_485 start(%d) second: ", recv_from_tl16_port - 2);
	for (k= 0; k < tmp;k++) {
		printf_syn("%02x ", buf[k+len]);
	}
	printf_syn("\n");
#endif
			}

			if (RT_EOK == event_take_tl16_485(recv_from_tl16_port, recv_byte_time_out)) {
				tmp = recv_data_by_tl16_485(get_port_from_tl16_485(recv_from_tl16_port), buf + len, sizeof(buf)-len);
				if (flag == UART_485_ANALYSIS_START) {
					len += tmp;
				}
			} else {
				break;
			}

			if (len >= sizeof(buf))
				break;
		}

		if (flag == UART_485_ANALYSIS_OK) {
			return RT_EOK;
		} else {
			return RT_ERROR;
		}
	} else {
		ERROR_485SW_PRINTF(" ");
		return RT_ETIMEOUT;
	}

	return RT_ERROR;
}

static rt_err_t recv_data_from_amm_send_to_tl16_port(enum rs485_uart recv_from_amm_port, enum rs485_uart send_to_tl16_port, rt_int32_t recv_format_time_out, rt_int32_t recv_byte_time_out)
{
	rt_uint32_t tmp = 0;
	rt_uint32_t len = 0;
	rt_uint8_t buf[256] = {'\0'};

	if (RT_EOK == uart485_wait_em_amm_revc_data_event(recv_from_amm_port, recv_format_time_out)) {
		tmp = recv_data_by_485(get_port_from_rs485_uart(recv_from_amm_port), buf, sizeof(buf));
		if (tmp > 0) {
#if 0
	rt_uint32_t k = 0;
	printf_syn("recv_data_by_485 (AMMETER_UART) start_1: ");
	for (k= 0; k < tmp;k++) {
		printf_syn("%02x ", buf[k]);
    }
	printf_syn("\n");
#endif
			send_data_by_tl16_485(get_port_from_tl16_485(send_to_tl16_port), buf, tmp);
			len = tmp;
#if 0
//	rt_uint32_t k = 0;
	printf_syn("send_data_by_tl16_485 (%d) start_1: ", send_to_tl16_port - 2);
	for (k= 0; k < tmp;k++) {
		printf_syn("%02x ", buf[k]);
    }
	printf_syn("\n");
#endif
		}

		while (1) {
			rt_memset(buf, 0, sizeof(buf));
			if (RT_EOK == uart485_wait_em_amm_revc_data_event(recv_from_amm_port, recv_byte_time_out)) {
				tmp = recv_data_by_485(get_port_from_rs485_uart(recv_from_amm_port), buf, sizeof(buf));

				if (tmp > 0) {
#if 0
	rt_uint32_t k = 0;
	printf_syn("recv_data_by_485 (AMMETER_UART) second_2: ");
	for (k= 0; k < tmp;k++) {
		printf_syn("%02x ", buf[k]);
    }
	printf_syn("\n");
#endif
					send_data_by_tl16_485(get_port_from_tl16_485(send_to_tl16_port), buf, tmp);
					len += tmp;
#if 0
//	rt_uint32_t k = 0;
	printf_syn("send_data_by_tl16_485 (%d) second_2: ", send_to_tl16_port - 2);
	for (k= 0; k < tmp;k++) {
		printf_syn("%02x ", buf[k]);
    }
	printf_syn("\n");
#endif
				}
			} else {
				break;
			}

			if (len >= sizeof(buf))
				break;
		}
	} else {
		ERROR_485SW_PRINTF(" ");
		return RT_ERROR;
	}

	return RT_EOK;
}

static rt_err_t uart485_wait_em_amm_revc_data_event(enum rs485_uart port, rt_int32_t time)
{
	rt_err_t ret = RT_EOK;
	rt_int32_t tmp = 0;
	rt_uint32_t e = 0;

	if (time != RT_WAITING_FOREVER) {
		tmp = get_ticks_of_ms(time);
	} else {
		tmp = time;
	}

	switch (port) {
	case EM_UART:
		ret = rt_event_recv(&em_protocol_data_event_set, EVENT_BIT_UART485_2_RECV_DATA, RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, tmp, &e);
		break;

	case AMMETER_UART:
		ret = rt_event_recv(&em_protocol_data_event_set, EVENT_BIT_UART485_1_RECV_DATA, RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, tmp, &e);
		break;

	default:
		ERROR_485SW_PRINTF(" ");
		return RT_ERROR;
	}

	return ret;
}

static rt_err_t event_take_tl16_485(enum rs485_uart port, rt_int32_t time)
{
	rt_err_t ret = RT_EOK;
	rt_int32_t tmp = 0;
	rt_uint32_t e = 0;

	if (time != RT_WAITING_FOREVER) {
		tmp = get_ticks_of_ms(time);
	} else {
		tmp = time;
	}

	switch (port) {
	case TL16_UART1:
		ret = rt_event_recv(&em_protocol_data_event_set, EVENT_BIT_TL16_1_RECV_DATA, RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, tmp, &e);
		break;

	case TL16_UART2:
		ret = rt_event_recv(&em_protocol_data_event_set, EVENT_BIT_TL16_2_RECV_DATA, RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, tmp, &e);
		break;

	case TL16_UART3:
		ret = rt_event_recv(&em_protocol_data_event_set, EVENT_BIT_TL16_3_RECV_DATA, RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, tmp, &e);
		break;

	case TL16_UART4:
		ret = rt_event_recv(&em_protocol_data_event_set, EVENT_BIT_TL16_4_RECV_DATA, RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, tmp, &e);
		break;

	case TL16_UART5:
		ret = rt_event_recv(&em_protocol_data_event_set, EVENT_BIT_TL16_5_RECV_DATA, RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, tmp, &e);
		break;

	case TL16_UART6:
		ret = rt_event_recv(&em_protocol_data_event_set, EVENT_BIT_TL16_6_RECV_DATA, RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, tmp, &e);
		break;

	case TL16_UART7:
		ret = rt_event_recv(&em_protocol_data_event_set, EVENT_BIT_TL16_7_RECV_DATA, RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, tmp, &e);
		break;

	case TL16_UART8:
		ret = rt_event_recv(&em_protocol_data_event_set, EVENT_BIT_TL16_8_RECV_DATA, RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, tmp, &e);
		break;

	default:
		ERROR_485SW_PRINTF(" ");

		return RT_ERROR;
	}

	return ret;
}

static USART_TypeDef *get_port_from_rs485_uart(enum rs485_uart port)
{
	switch (port) {
	case EM_UART:
		return UART_485_2_DEV_PTR;

	case AMMETER_UART:
		return UART_485_1_DEV_PTR;

	default:
		return NULL;
	}
}

static rt_device_t get_port_from_tl16_485(enum rs485_uart port)
{
	switch (port) {
	case TL16_UART1:
		return dev_485_tl16_1;

	case TL16_UART2:
		return dev_485_tl16_2;

	case TL16_UART3:
		return dev_485_tl16_3;

	case TL16_UART4:
		return dev_485_tl16_4;

	case TL16_UART5:
		return dev_485_tl16_5;

	case TL16_UART6:
		return dev_485_tl16_6;

	case TL16_UART7:
		return dev_485_tl16_7;

	case TL16_UART8:
		return dev_485_tl16_8;

	default:
		return NULL;
	}
}

#if 0
static rt_err_t create_frame_em(rt_uint8_t *send_frame_buf, rt_uint32_t *send_frame_len, struct uart_param data)
{
	rt_uint8_t i = 0, check = 0;

	ASSERT_485SW(RT_NULL != send_frame_buf || RT_NULL != send_frame_len);

	/* 帧起始符 */
	*send_frame_buf       = 0x57;
	/* 波特率 */
	rt_memcpy(send_frame_buf+1, &(data.baudrate), 3);

	*(send_frame_buf + 4) = 0x57;
	/* 控制码 */
	*(send_frame_buf + 5) = 0x81;
	/* 数据长度 */
	*(send_frame_buf + 6) = 0x1;
	/* 数据位 */
	*(send_frame_buf + 7) = data.databits << 4;
	/* 校验位 */
	*(send_frame_buf + 7) += data.paritybit << 2;
	/* 停止位 */
	*(send_frame_buf + 7) += data.stopbits;
	/* 帧校验码 */
	for (i = 0; i < 8; i++) {
		check += *(send_frame_buf+i);
	}
	*(send_frame_buf + 8) = check;
	/* 结束符 */
	*(send_frame_buf + 9) = 0x75;

	*send_frame_len = 10;

	return RT_EOK;
}
#endif

static rt_err_t analysis_frame_em(rt_uint8_t *recv_frame_buf, rt_uint32_t recv_frame_len, struct uart_param *data)
{
	rt_uint8_t i = 0, check = 0;

	ASSERT_485SW(RT_NULL != recv_frame_buf || RT_NULL != data);

	/* 帧起始符 */

	if (*recv_frame_buf != 0x57 || *(recv_frame_buf + 4) != 0x57 || *(recv_frame_buf + 9) != 0x75) {
		ERROR_485SW_PRINTF(" ");
		return RT_ERROR;
	}

	/* 帧校验码 */
	for (i = 0; i < 8; i++) {
		check += *(recv_frame_buf+i);
	}
	if (*(recv_frame_buf + 8) != check) {
		ERROR_485SW_PRINTF(" ");
		return RT_ERROR;
	}

	rt_memcpy(&(data->baudrate), recv_frame_buf + 1, 3);	/* 波特率 */
	data->databits = *(recv_frame_buf + 7) >> 4;   			/* 数据位 */
	data->paritybit = (*(recv_frame_buf + 7)>>2)&0x3; 		/* 检验位 */
	data->stopbits = *(recv_frame_buf + 7)&0x3;				/* 停止位 */

	return RT_EOK;
}

static rt_err_t analysis_frame_645(rt_uint8_t *recv_frame_buf, rt_uint32_t recv_frame_length)
{
	rt_uint8_t i = 0, j = 0;

	ASSERT_485SW(RT_NULL != recv_frame_buf);

	for (i = 0; i < recv_frame_length; i++) {
		if (*(recv_frame_buf+i) == 0x68 && *(recv_frame_buf+i+7) == 0x68) { /*search the 68H*/
			for (j = 0; j < NUM_OF_COLLECT_EM_MAX; j++) {
				if (protocal_record[j] == PROTOCAL_645) {
					if (rt_memcmp(recv_frame_buf+i+1, sn_record[j], 6) == 0) {
						return RT_EOK;
					}
				}
			}
			break;
		}
	}

	return RT_ERROR;
}

static rt_err_t analysis_frame_edmi(rt_uint8_t *recv_frame_buf, rt_uint32_t recv_frame_length)
{
	rt_uint8_t i = 0, j = 0;

	ASSERT_485SW(RT_NULL != recv_frame_buf);

	for (i = 0; i < recv_frame_length; i++) {
		if (*(recv_frame_buf+i) == 0x02 && *(recv_frame_buf+i+1) == 0x45) { /*search the 68H*/
			for (j = 0; j < NUM_OF_COLLECT_EM_MAX; j++) {
				if (protocal_record[j] == PROTOCAL_EDMI) {
					if (rt_memcmp(recv_frame_buf+i+2, sn_record[j], 4) == 0) {
						return RT_EOK;
					}
				}
			}
			break;
		}
	}

	return RT_ERROR;
}

#if 0
static rt_err_t analysis_frame_tl16(rt_uint8_t *recv_frame_buf, rt_uint32_t recv_frame_len, struct uart_param *data, enum rs485_uart port)
{
	ASSERT_485SW(RT_NULL != recv_frame_buf);

	switch (port) {
	case TL16_UART1:
		data->baudrate = 1200;
		data->databits = 8;
		data->paritybit = 0;
		data->stopbits = 1;
		return analysis_frame_edmi(recv_frame_buf, recv_frame_len);

	case TL16_UART2:
		data->baudrate = 2400;
		data->databits = 9;
		data->paritybit = 1;
		data->stopbits = 1;
		return analysis_frame_645(recv_frame_buf, recv_frame_len);

	case TL16_UART3:
		data->baudrate = 1200;
		data->databits = 9;
		data->paritybit = 1;
		data->stopbits = 1;
		return analysis_frame_645(recv_frame_buf, recv_frame_len);

	case TL16_UART4:
		data->baudrate = 9600;
		data->databits = 8;
		data->paritybit = 0;
		data->stopbits = 1;
		return analysis_frame_edmi(recv_frame_buf, recv_frame_len);

	case TL16_UART5:
		return RT_ERROR;
		break;

	case TL16_UART6:
		return RT_ERROR;
		break;

	case TL16_UART7:
		return RT_ERROR;
		break;

	case TL16_UART8:
		return RT_ERROR;
		break;

	default:
		ERROR_485SW_PRINTF(" ");
		break;
	}

	return RT_ERROR;
}
#endif

static rt_err_t analysis_frame_tl16(rt_uint8_t *recv_frame_buf, rt_uint32_t recv_frame_len, struct uart_param *data, enum rs485_uart port)
{
	rt_uint8_t num = 0;

	ASSERT_485SW(RT_NULL != recv_frame_buf && RT_NULL != data);

	switch (port) {
	case TL16_UART1:
		num = 0;
		break;

	case TL16_UART2:
		num = 1;
		break;

	case TL16_UART3:
		num = 2;
		break;

	case TL16_UART4:
		num = 3;
		break;

	case TL16_UART5:
		num = 4;
		break;

	case TL16_UART6:
		num = 5;
		break;

	case TL16_UART7:
		num = 6;
		break;

	case TL16_UART8:
		num = 7;
		break;

	default:
		ERROR_485SW_PRINTF(" ");
		break;
	}

	rt_memcpy(data, &(tl16_485_data_record[num]), sizeof(struct uart_param));
	data->databits += data->paritybit;

	switch (tl16_485_protocal_record[num]) {
	case PROTOCAL_645:
		return analysis_frame_645(recv_frame_buf, recv_frame_len);

	case PROTOCAL_EDMI:
		return analysis_frame_edmi(recv_frame_buf, recv_frame_len);

	default:
		ERROR_485SW_PRINTF(" ");
		break;
	}


	return RT_ERROR;
}


static rt_err_t uart_485_if_set(struct uart_param *data, enum rs485_uart port)
{
	rt_uint8_t uart_sw;

	if (data->baudrate <= 0 || data->databits < 8 || data->databits > 9 || data->paritybit < 0 || data->paritybit > 2 || data->stopbits < 1 || data->stopbits > 2) {
		ERROR_485SW_PRINTF(" ");
		return RT_ERROR;
	}

	switch (port) {
	case EM_UART:
		uart_sw = 4;
		break;

	case AMMETER_UART:
		uart_sw = 2;
		break;

	default:
		return RT_ERROR;
	}

	if (RT_EOK != UART_if_Set(data->baudrate, data->databits, data->paritybit, data->stopbits, uart_sw)) {
		ERROR_485SW_PRINTF(" ");
		return RT_ERROR;
	}

	printf_syn("UART_if_Set(port: %d ):baudrate = %d\n", uart_sw, data->baudrate);
	printf_syn("UART_if_Set(port: %d ):databits = %d\n", uart_sw, data->databits);
	printf_syn("UART_if_Set(port: %d ):paritybit = %d\n", uart_sw, data->paritybit);
	printf_syn("UART_if_Set(port: %d ):stopbits = %d\n", uart_sw, data->stopbits);

//	rt_thread_delay(40);

	return RT_EOK;
}

static rt_err_t str_sn_change_to_bcd_sn(rt_uint8_t *input_str_sn, rt_uint8_t *output_bcd_sn)
{
	rt_uint32_t i = 0, count = 0;
	rt_uint8_t str[2] = {'\0'};
	rt_uint8_t addr_str[12] = {'\0'};

	if (input_str_sn == NULL || output_bcd_sn == NULL) {
		ERROR_485SW_PRINTF(" ");
		return RT_ERROR;
	}

	count = rt_strlen((char *)input_str_sn);
	if (count < 12) {
		rt_memcpy(addr_str + 12 - count, input_str_sn, count);
	} else {
		rt_memcpy(addr_str, input_str_sn, 12);
	}

	for (i = 0; i < 6; i++) {
		str[0] = addr_str[i*2];
		str[1] = addr_str[i*2+1];

		if (str[0] >= 0x30)
			str[0] = str[0] - 0x30;
		if (str[1] >= 0x30)
			str[1] = str[1] - 0x30;

		output_bcd_sn[i] = str[0]<<4 | str[1];
	}

	return RT_EOK;
}

static rt_err_t get_flash_amm_addr(void)
{
	rt_uint32_t i = 0, k = 0;
	rt_uint32_t sn = 0;
	rt_uint8_t addr[6] = {'\0'};
	rt_uint8_t tmp_buf[6] = {'\0'};
	struct uart_param uart_485_data[4];
	struct electric_meter_reg_info_st amm_sns;

	rt_memset(&amm_sns, 0, sizeof(struct electric_meter_reg_info_st));

	if (SUCC == get_em_reg_info(&amm_sns)) {
		for (k = 0; k < NUM_OF_COLLECT_EM_MAX; k++) {
			if (amm_sns.protocal != PROTOCAL_UNKNOWN) {
				if (RT_EOK == str_sn_change_to_bcd_sn((rt_uint8_t *)(amm_sns.em_sn[k]), addr)) {
					if (amm_sns.protocal[k] == PROTOCAL_645) {
						for (i = 0; i < 6; i++) {
							sn_record[k][i] = addr[5-i];
						}
						protocal_record[k] = PROTOCAL_645;
					} else if (amm_sns.protocal[k] == PROTOCAL_EDMI){
						for (i = 0; i < 6; i++) {
							tmp_buf[i] = (*(addr + i) & 0x0f) + ((*(addr + i) >> 4) * 10);
						}
						sn = tmp_buf[0]*10000000000 + tmp_buf[1]*100000000 + tmp_buf[2]*1000000 + tmp_buf[3]*10000 + tmp_buf[4]*100 + tmp_buf[5];

						for (i = 0; i < 4; i++) {
							sn_record[k][3 - i] = (sn >> i * 8) & 0xff;
						}
						protocal_record[k] = PROTOCAL_EDMI;
					} else {
						rt_memcpy(sn_record[k], addr, 6);
						protocal_record[k] = PROTOCAL_UNKNOWN;
					}
				}
			}
		}
	} else {
		ERROR_485SW_PRINTF(" ");
		return RT_ERROR;
	}
	rt_memset(&uart_485_data, 0, sizeof(uart_485_data));

	uart_485_data[0].baudrate = 1200;
	uart_485_data[0].databits = 8;
	uart_485_data[0].paritybit = 0;
	uart_485_data[0].stopbits = 1;

	uart_485_data[1].baudrate = 1200;
	uart_485_data[1].databits = 8;
	uart_485_data[1].paritybit = 1;
	uart_485_data[1].stopbits = 1;

	uart_485_data[2].baudrate = 2400;
	uart_485_data[2].databits = 8;
	uart_485_data[2].paritybit = 1;
	uart_485_data[2].stopbits = 1;

	uart_485_data[3].baudrate = 9600;
	uart_485_data[3].databits = 8;
	uart_485_data[3].paritybit = 0;
	uart_485_data[3].stopbits = 1;

	for (i = 0; i < TL16_UART_NUM; i++) {
		if (SUCC != get_tl16_uart_param(i+1, &(tl16_485_data_record[i]))) {
			ERROR_485SW_PRINTF(" ");
			return RT_ERROR;
		}

		if (rt_memcmp(&(tl16_485_data_record[i]), &(uart_485_data[0]), sizeof(struct uart_param)) == 0) {
			tl16_485_protocal_record[i] = PROTOCAL_EDMI;
		} else if (rt_memcmp(&(tl16_485_data_record[i]), &(uart_485_data[1]), sizeof(struct uart_param)) == 0) {
			tl16_485_protocal_record[i] = PROTOCAL_645;
		} else if (rt_memcmp(&(tl16_485_data_record[i]), &(uart_485_data[2]), sizeof(struct uart_param)) == 0) {
			tl16_485_protocal_record[i] = PROTOCAL_645;
		} else if (rt_memcmp(&(tl16_485_data_record[i]), &(uart_485_data[3]), sizeof(struct uart_param)) == 0) {
			tl16_485_protocal_record[i] = PROTOCAL_EDMI;
		} else {
			tl16_485_protocal_record[i] = PROTOCAL_UNKNOWN;
		}
	}

	return RT_EOK;
}


rt_err_t memcmp_and_set_uart485_baud_data(struct uart_param *cs, struct uart_param *ct, enum rs485_uart port)
{
	if (rt_memcmp(cs, ct, sizeof(struct uart_param)) != 0) {
		if (RT_EOK != uart_485_if_set(ct, port)) {
			ERROR_485SW_PRINTF(" ");
			return RT_ERROR;
		}
		rt_memcpy(cs, ct, sizeof(struct uart_param));
	}

	return RT_EOK;
}
