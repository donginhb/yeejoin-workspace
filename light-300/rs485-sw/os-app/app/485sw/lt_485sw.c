/*
 ******************************************************************************
 * lt_485sw.c
 *
 *  Created on: 2015-01-25
 *      Author: David, zhaoshaowei@yeejoin.com
 *
 * COPYRIGHT (C) 2015, YEEJOIN (Beijing) Technology Development Co., Ltd.
 ******************************************************************************
 */


#include <rtdef.h>
#include <board.h>
#include <misc_lib.h>
#include <sys_cfg_api.h>

#include <stm32f10x_usart.h>
#include <debug_sw.h>

#include <lt_485sw.h>
#include "lt_485sw_analysis.h"

#include <auto_negoti_timer.h>

#define lt485_debug(x) 	//printf_syn x
#define lt485_info(x) 	printf_syn x
#define lt485_log(x) 	printf_syn x

#define PRINT_NEGOTIATION_CMD_DATA	1
#define PRINT_SENDCMD2EM_DATA		1

#if 1
#define EMC_AUTO_ENTER_NEGOTIATION_STATE_TIME_MS (890)
#else
#define EMC_AUTO_ENTER_NEGOTIATION_STATE_TIME_MS (890 + 10*1000)
#endif

volatile int had_start_auto_negotiation_timer;

struct rt_mutex em_grp1_if_mutex;
struct rt_mutex em_grp2_if_mutex;

struct rt_semaphore em_grp1_if_lu_sem;
struct rt_semaphore em_grp2_if_lu_sem;

struct rt_mailbox send_cmd2em_grp1_mb;
struct rt_mailbox send_cmd2em_grp2_mb;

//struct rt_timer emc_auto_enter_negotiation_state_timer;

//unsigned protoc_inter_byte_toparam;
enum ammeter_protocol_e protocol_use_by_emc;

struct uart_param emc_serial_param;

static char send_cmd2em_grp1_mb_pool[4*10];
static char send_cmd2em_grp2_mb_pool[4*10];

static void rt_tl16_proxy_entry(void* parameter);
static void rt_emc_proxy_entry(void* parameter);
static void rt_send_cmd2em_grpx_entry(void* parameter);

static int do_send_tl16_cmd2em(struct protoc_analy_info_st *analys_info, enum lt_485if_e lt_if_use_by_em);
static int do_send_emc_cmd2em(struct protoc_analy_info_st *analys_info,
		struct protoc_analy_info_st **new, enum lt_485if_e lt_if_use_by_em);
void emc_state_timeout(void *p);
static void proc_emc_negotiation_cmd(struct protoc_analy_info_st *analys_info,
		enum lt_485if_e rs485_if, int *had_proc);
static int start_auto_negotiation_timer(void);
//static int stop_auto_negotiation_timer(void);


#define TL16C554_PROXY_STACK_SIZE	(2048)
#define EMC_PROXY_STACK_SIZE 		(2048)

void lt_485sw_init(void)
{
	rt_thread_t thread_h;
	int i;
	static enum lt_485if_e tl16_entry_rs485_if[8];

	char str[] = {"ibyte-0"};
	char str_pai[] = {"pai-0"};

	for (i=LT_485_TL16_U1; i<=LT_485_EMC_UART; i++) {
		init_protoc_analy_info(&proto_analy_info[i], i);
		protoc_inter_byte_timer_param[i] = i;
		str[6] = '0' + i;
		rt_timer_init(&protoc_inter_byte_timer[i], str,   /* 定时器名字 */
				protoc_inter_byte_timeout, /* 超时时回调的处理函数 */
				&protoc_inter_byte_timer_param[i], /* 超时函数的入口参数 */
				get_ticks_of_ms(LT_485SW_BYTE_WAIT_TIMEOUT_MS),
				RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER); /* 单次定时器 */
		str_pai[4] = 'A' + i;
		init_protoc_analy_rb_sys(&proto_analy_info[i], str_pai, proto_analy_info[i].lt_if);
	}

	rt_mutex_init(&em_grp1_if_mutex, "emg1if", RT_IPC_FLAG_PRIO);
	rt_mutex_init(&em_grp2_if_mutex, "emg2if", RT_IPC_FLAG_PRIO);
	rt_sem_init(&em_grp1_if_lu_sem, "emg1if", 1, RT_IPC_FLAG_PRIO);
	rt_sem_init(&em_grp2_if_lu_sem, "emg2if", 1, RT_IPC_FLAG_PRIO);

	rt_mb_init(&send_cmd2em_grp1_mb, "sem1_cmd", send_cmd2em_grp1_mb_pool,
			sizeof(send_cmd2em_grp1_mb_pool)/4, RT_IPC_FLAG_PRIO);
	rt_mb_init(&send_cmd2em_grp2_mb, "sem2_cmd", send_cmd2em_grp2_mb_pool,
			sizeof(send_cmd2em_grp2_mb_pool)/4, RT_IPC_FLAG_PRIO);
#if 0
	rt_timer_init(&emc_auto_enter_negotiation_state_timer, "emcstat",   /* 定时器名字 */
			emc_state_timeout, /* 超时时回调的处理函数 */
			NULL, /* 超时函数的入口参数 */
			get_ticks_of_ms(EMC_AUTO_ENTER_NEGOTIATION_STATE_TIME_MS),
			RT_TIMER_FLAG_ONE_SHOT); /* 单次定时器 */
#else
	init_auto_negoti_timer(get_auto_negoti_tikcs_of_ms(EMC_AUTO_ENTER_NEGOTIATION_STATE_TIME_MS));
	cfg_auto_negoti_timer_nvic();
	enable_auto_negoti_timer_int();
#endif
	emc_serial_param.baudrate = 2400;
	emc_serial_param.databits = 8;
	emc_serial_param.paritybit = 1;
	emc_serial_param.stopbits =1;

	tl16_entry_rs485_if[0] = LT_485_TL16_U1;
	thread_h = rt_thread_create("554prox1", rt_tl16_proxy_entry,
			&tl16_entry_rs485_if[0], TL16C554_PROXY_STACK_SIZE, 21, 20);
	if (thread_h != RT_NULL)
		rt_thread_startup(thread_h);
	else
		printf_syn("create 554_prox1 thread fail\n");

	tl16_entry_rs485_if[1] = LT_485_TL16_U2;
	thread_h = rt_thread_create("554prox2", rt_tl16_proxy_entry,
			&tl16_entry_rs485_if[1], TL16C554_PROXY_STACK_SIZE, 21, 20);
	if (thread_h != RT_NULL)
		rt_thread_startup(thread_h);
	else
		printf_syn("create 554_prox2 thread fail\n");

	tl16_entry_rs485_if[2] = LT_485_TL16_U3;
	thread_h = rt_thread_create("554prox3", rt_tl16_proxy_entry,
			&tl16_entry_rs485_if[2], TL16C554_PROXY_STACK_SIZE, 21, 20);
	if (thread_h != RT_NULL)
		rt_thread_startup(thread_h);
	else
		printf_syn("create 554_prox3 thread fail\n");

	tl16_entry_rs485_if[3] = LT_485_TL16_U4;
	thread_h = rt_thread_create("554prox4", rt_tl16_proxy_entry,
			&tl16_entry_rs485_if[3], TL16C554_PROXY_STACK_SIZE, 21, 20);
	if (thread_h != RT_NULL)
		rt_thread_startup(thread_h);
	else
		printf_syn("create 554_prox4 thread fail\n");

	tl16_entry_rs485_if[4] = LT_485_TL16_U5;
	thread_h = rt_thread_create("554prox5", rt_tl16_proxy_entry,
			&tl16_entry_rs485_if[4], TL16C554_PROXY_STACK_SIZE, 21, 20);
	if (thread_h != RT_NULL)
		rt_thread_startup(thread_h);
	else
		printf_syn("create 554_prox5 thread fail\n");

	tl16_entry_rs485_if[5] = LT_485_TL16_U6;
	thread_h = rt_thread_create("554prox6", rt_tl16_proxy_entry,
			&tl16_entry_rs485_if[5], TL16C554_PROXY_STACK_SIZE, 21, 20);
	if (thread_h != RT_NULL)
		rt_thread_startup(thread_h);
	else
		printf_syn("create 554_prox6 thread fail\n");

	tl16_entry_rs485_if[6] = LT_485_TL16_U7;
	thread_h = rt_thread_create("554prox7", rt_tl16_proxy_entry,
			&tl16_entry_rs485_if[6], TL16C554_PROXY_STACK_SIZE, 21, 20);
	if (thread_h != RT_NULL)
		rt_thread_startup(thread_h);
	else
		printf_syn("create 554_prox7 thread fail\n");

	tl16_entry_rs485_if[7] = LT_485_TL16_U8;
	thread_h = rt_thread_create("554prox8", rt_tl16_proxy_entry,
			&tl16_entry_rs485_if[7], TL16C554_PROXY_STACK_SIZE, 21, 20);
	if (thread_h != RT_NULL)
		rt_thread_startup(thread_h);
	else
		printf_syn("create 554_prox8 thread fail\n");

	thread_h = rt_thread_create("emc_prox", rt_emc_proxy_entry, RT_NULL, EMC_PROXY_STACK_SIZE, 21, 20);
	if (thread_h != RT_NULL)
		rt_thread_startup(thread_h);
	else
		printf_syn("create emc_prox thread fail\n");

	thread_h = rt_thread_create("scmd2em1", rt_send_cmd2em_grpx_entry, &send_cmd2em_grp1_mb, 1024+128, 20, 60);
	if (thread_h != RT_NULL)
		rt_thread_startup(thread_h);
	else
		printf_syn("create scmd2em1 thread fail\n");

	thread_h = rt_thread_create("scmd2em2", rt_send_cmd2em_grpx_entry, &send_cmd2em_grp2_mb, 1024+128, 20, 60);
	if (thread_h != RT_NULL)
		rt_thread_startup(thread_h);
	else
		printf_syn("create scmd2em2 thread fail\n");

	return;
}



/*
 * 做为连接到tl16上的集中器代理
 * */
static void rt_tl16_proxy_entry(void* parameter)
{
	rt_err_t err;
	rt_uint32_t e;
	int had_proc;
	struct protoc_analy_info_st *p;
	enum lt_485if_e rs485_if;
	unsigned eve_bit;
	rt_thread_t th;

	th = rt_thread_self();
	rs485_if = *(enum lt_485if_e *)parameter;
	if (LT_485_TL16_U1>rs485_if || LT_485_TL16_U8<rs485_if) {
		printf_syn("th:%s, %s(), param invalid(%d)\n", th->name, __func__, rs485_if);
		return;
	}

	eve_bit = get_em_protocol_data_event_bit(rs485_if);
	p = &proto_analy_info[rs485_if];

//	lt485_debug(("th:%s, rs485_if:%d, eve_bit:0x%x\n", th->name, rs485_if, eve_bit));

	while (1) {
		err = rt_event_recv(&em_protocol_data_event_set, eve_bit,
				RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &e);

		lt485_debug(("th:%s, %s() recv event #%x, err:%d\n", th->name, __func__, e, err));

		if (RT_EOK != err) {
			lt485_log(("th:%s, %s(), recv event error(%d)\n", th->name, __func__, err));
			continue;
		}


		do {
			had_proc = 0;
			proc_frame(p, rs485_if, &had_proc);
		} while (had_proc);
	}

}

/* 处于等待emc协商命令的状态 */
static volatile int wait_emc_negotiation_cmd = 1;

/*
 * 做为emc的代理
 * */
static void rt_emc_proxy_entry(void* parameter)
{
	rt_err_t err;
	rt_uint32_t e;
	int had_proc;

	while (1) {
		err = rt_event_recv(&em_protocol_data_event_set, EVENT_BIT_EMC_485_RECV_DATA,
				RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &e);
		if (RT_EOK == err) {
//			reset_auto_negotiation_timer();

//			sw485_debug_body(("%s() recv event #%x\n", __func__, e));

			if (wait_emc_negotiation_cmd) {
				do {
					had_proc = 0;
					proc_emc_negotiation_cmd(&proto_analy_info[LT_485_EMC_UART],
							LT_485_EMC_UART, &had_proc);
//					reset_auto_negotiation_timer();
				} while (had_proc);
			} else {
				do {
					had_proc = 0;
					proc_frame(&proto_analy_info[LT_485_EMC_UART],
							LT_485_EMC_UART, &had_proc);
//					reset_auto_negotiation_timer();
				} while (had_proc);
			}
		} else {
			lt485_log(("%s(), recv event error(%d)\n", __func__, err));
		}
	}
}


static void set_emc_uart_to_def_param(void)
{
	USART_InitTypeDef USART_InitStructure;

	USART_StructInit(&USART_InitStructure);

	USART_InitStructure.USART_BaudRate = 2400;
	USART_InitStructure.USART_WordLength = USART_WordLength_9b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_Even;

	USART_Cmd(UART4, DISABLE);
	USART_Init(UART4, &USART_InitStructure);
	USART_Cmd(UART4, ENABLE);

	return;
}

void emc_state_timeout(void *p)
{
	if (is_sub_m_on(dsw_sub_module[DMN_485SW], DSMN_485SW_NEGOTIATION_CMD_DATA)) {
		rt_kprintf("\n%s(), cur_tick:%u\n\n", __func__, rt_tick_get());
	}

	wait_emc_negotiation_cmd = 1;
	had_start_auto_negotiation_timer = 0;
	set_emc_uart_to_def_param();
}

static rt_err_t analysis_negoti_frame_frome_em(rt_uint8_t *frame_buf, rt_uint32_t frame_len,
		struct uart_param *data, enum ammeter_protocol_e *protocol)
{
	rt_uint8_t i, tmp = 0;
	rt_uint8_t data_len = 0;
	rt_uint8_t *pch;

	*protocol = AP_PROTOCOL_UNKNOWN;

	/* 帧起始符 */
	if (!(frame_len >= 11))
		return RT_ERROR;

	frame_len -= 10;
	pch = frame_buf;
	for (i=0; i<frame_len; i++) {
		if (*pch==0x57 && *(pch+4)==0x57)
			break;
		++pch;
	}

	if (i>=frame_len) {
		lt485_log(("%s(), frame format error\n", __func__));
		return RT_ERROR;
	}

	data_len = *(pch+6);

	/* 帧校验码 */
	for (i=0; i<7+data_len; i++)
		tmp += *(pch+i);

	if (*(pch+7+data_len) != tmp) {
		lt485_log(("%s(), check sum error\n", __func__));
		return RT_ERROR;
	}

	if (*(pch+8+data_len) != 0x75) {
		lt485_log(("%s(), frame end error\n", __func__));
		return RT_ERROR;
	}

	rt_memcpy(&(data->baudrate), pch + 1, 3);	/* 波特率 */
	tmp = *(pch + 7);
	data->databits  = tmp >> 4;   	/* 数据位 */
	data->paritybit = (tmp>>2) & 0x3; 	/* 检验位 */
	data->stopbits  = tmp & 0x3;		/* 停止位 */

	if (data_len > 1) {
		*protocol = *(pch + 8);
	}

	return RT_EOK;
}

static void proc_emc_negotiation_cmd(struct protoc_analy_info_st *analys_info,
		enum lt_485if_e rs485_if, int *had_proc)
{
	static rt_uint8_t buf[64];
	static int recv_data_len = 0;

	struct uart_param uart485_data;
	int tmp, i;
	USART_TypeDef  *ptr;
	enum ammeter_protocol_e protoc;

	/*485sw正常回答em的请求， OK */
	rt_uint8_t buffer_ok[] = {0x57, 0xFF, 0xFF, 0xFF, 0x57, 0x81, 0x00, 0x2C, 0x75};

	ptr = get_us485_dev_ptr(rs485_if, 1);

	do {
		tmp = recv_data_by_485(ptr, buf+recv_data_len, sizeof(buf)-recv_data_len);
		recv_data_len += tmp;
		if (recv_data_len>=20)
			break;
		if (0 != tmp)
			++*had_proc;
	} while (tmp);

//	reset_auto_negotiation_timer();

	lt485_debug(("%s() recv_data_len:%d\n", __func__, recv_data_len));

	if (recv_data_len >= 11) {
		if (is_sub_m_on(dsw_sub_module[DMN_485SW], DSMN_485SW_NEGOTIATION_CMD_DATA)) {
			printf_syn("%s() recv data(%d):\n", __func__, recv_data_len);
			for (i=0; i<recv_data_len; ++i)
				printf_syn("nego-d:%x\n", buf[i]);
		}

		rt_memset(&uart485_data, 0, sizeof(struct uart_param));
		if (RT_EOK == analysis_negoti_frame_frome_em(buf, recv_data_len, &uart485_data, &protoc)) {
//			reset_auto_negotiation_timer();

			send_data_by_485(ptr, buffer_ok, sizeof(buffer_ok));

//			reset_auto_negotiation_timer();

			if (RT_EOK != UART_if_Set(uart485_data.baudrate, uart485_data.databits,
					uart485_data.paritybit, uart485_data.stopbits, 4)) {
				lt485_log(("%s() modify uart param fail\n", __func__));
			} else {
				start_auto_negotiation_timer();

				sw485_debug_body(("%s() ok: protoc-%d, %d,%d,%d,%d, cur_tick:%u\n", __func__,
						protoc, uart485_data.baudrate, uart485_data.databits,
						uart485_data.paritybit, uart485_data.stopbits,
						rt_tick_get()));

				/* 成功进入接收emc电表规约命令状态后start timer */

				wait_emc_negotiation_cmd = 0;

				emc_serial_param.baudrate = uart485_data.baudrate;
				emc_serial_param.databits = uart485_data.databits;
				emc_serial_param.paritybit = uart485_data.paritybit;
				emc_serial_param.stopbits = uart485_data.stopbits;

				protocol_use_by_emc = protoc;
				analys_info->protocol = protocol_use_by_emc;

				sw485_debug_body(("%s() lt_if-%d protocol_use_by_emc:%d\n", __func__,
						analys_info->lt_if ,protocol_use_by_emc));

//				rt_timer_start(&emc_auto_enter_negotiation_state_timer);
			}
		} else {
			/* clear  buf */
#if 0
			do {
				tmp = recv_data_by_485(ptr, buf, sizeof(buf));
			} while (tmp);
#else
			{
				rt_device_t dev;
				dev = get_us485_dev_ptr(rs485_if, 0);
				if (NULL != dev) {
					dev->control(dev, RT_DEVICE_CTRL_CLR_RXBUF, NULL);
				} else {
					lt485_log(("%s() get us485 dev fail\n", __func__));
				}
			}
#endif
			lt485_log(("%s() analysis_negoti_frame_frome_em fail\n", __func__));
		}
		recv_data_len = 0;
	}

	return;
}

static int do_send_cmd2em_grpx(struct protoc_analy_info_st *analys_info, rt_mailbox_t mb)
{
	struct protoc_analy_info_st *new;
	enum lt_485if_e lt_if_use_by_em;
	enum lt_485if_e lt_if;

//	sw485_debug_body(("%s() recv mb #%x\n", __func__, value));

	lt_if_use_by_em = analys_info->lt_if_use_by_em;
	lt_if = analys_info->lt_if;

//	if (had_start_auto_negotiation_timer && 2==analys_info->frame_pos) {
//		stop_auto_negotiation_timer();
//	}

	sw485_debug_body(("%s() msg num is %d, rb-elements:%d, lt_if-%d, lt_if_use_by_em:%d, cur_tick:%u\n",
			__func__, mb->entry, ringbuf_elements(&analys_info->rb), lt_if,
			lt_if_use_by_em, rt_tick_get()));

	if (is_tl16_interface(lt_if)) {
		if (SUCC != do_send_tl16_cmd2em(analys_info, lt_if_use_by_em)) {
			ringbuf_clr(&analys_info->rb);
		}
	} else if (LT_485_EMC_UART == lt_if) {
		if (SUCC != do_send_emc_cmd2em(analys_info, &new, lt_if_use_by_em)) {
			ringbuf_clr(&analys_info->rb);
		}
	} else {
		lt485_log(("%s(), recv invalid lt_if-%d from mb\n", __func__, lt_if));
	}

//	if (had_start_auto_negotiation_timer && 2==analys_info->frame_pos) {
//		ringbuf_clr(&analys_info->rb);
//		start_auto_negotiation_timer();
//	}

	return 0;
}


/*
 * 连接电表的485口，会被三个线程使用：
 * 	rt_tl16_proxy_entry， rt_emc_proxy_entry直接从该接口读取数据
 * 	rt_send_cmd2em_grpx_entry用于该接口发送数据
 * */
static void rt_send_cmd2em_grpx_entry(void* parameter)
{
	rt_err_t err;
	rt_uint32_t value;
	struct rt_mailbox *p;
	rt_thread_t th;

	th = rt_thread_self();
	p = (struct rt_mailbox *)parameter;

	while (1) {
		err = rt_mb_recv(p, &value, RT_WAITING_FOREVER);
		if (RT_EOK == err) {
			do_send_cmd2em_grpx((struct protoc_analy_info_st *)value, p);
		} else {
			lt485_log(("th:%s, %s(), recv mb error(%d)\n", th->name, __func__, err));
		}
	}
}


static int do_send_tl16_cmd2em(struct protoc_analy_info_st *analys_info, enum lt_485if_e lt_if_use_by_em)
{
	rt_err_t ret = SUCC;
	int read_len, size;
	char buf[64];
	struct ringbuf *r;
	USART_TypeDef  *dst_ptr;

	if (RT_EOK != lt_485_em_grpx_mutex_take(analys_info->lt_if, lt_if_use_by_em)) {
		return FAIL;
	}

	dst_ptr = get_us485_dev_ptr(lt_if_use_by_em, 1);
	r = &analys_info->rb;
	tl16_set_uartparam_use_by_em(analys_info->lt_if, lt_if_use_by_em); /* mark by David */

	do {
		size = ringbuf_elements(r);
		read_len = size>sizeof(buf) ? sizeof(buf) : size;

		if (!read_len) break;

		for (size=0; size<read_len; ++size) {
			buf[size] = ringbuf_get(r);
#if PRINT_SENDCMD2EM_DATA
			sw485_debug_sendcmd(("#t#%x\n", buf[size]));
#endif
		}

		size = send_data_by_485(dst_ptr, buf, read_len);

		if (size != read_len) {
			lt485_log(("%s() send data error(%d, %d)\n", __func__, read_len, size));
			ret = FAIL;
			break;
		}

//		rt_thread_delay(get_ticks_of_ms(50));
	} while (read_len);

	lt_485_em_grpx_mutex_release(lt_if_use_by_em);

	return ret;
}


static int do_send_emc_cmd2em(struct protoc_analy_info_st *analys_info,
		struct protoc_analy_info_st **new, enum lt_485if_e lt_if_use_by_em)
{
	rt_err_t ret = SUCC;
	int read_len, size;
	struct ringbuf *r;
	USART_TypeDef  *dst_ptr;
	char buf[64];

	if (RT_EOK != lt_485_em_grpx_mutex_take(analys_info->lt_if, lt_if_use_by_em)) {
		return FAIL;
	}

	dst_ptr = get_us485_dev_ptr(lt_if_use_by_em, 1);
	r = &analys_info->rb;
	*new = NULL;

	emc_set_uartparam_use_by_em(lt_if_use_by_em);

	do {
		size = ringbuf_elements(r);
		read_len = size>sizeof(buf) ? sizeof(buf) : size;

		if (!read_len) {
			break;
		} else if (read_len>40) {
			lt485_info(("NOTE:%s() emc send too data(will ignore)!!\n", __func__));
			ringbuf_clr(r);
			continue;
		}

		for (size=0; size<read_len; ++size) {
			buf[size] = ringbuf_get(r);
#if PRINT_SENDCMD2EM_DATA
			sw485_debug_sendcmd(("#e#%x\n", buf[size]));
#endif
		}

		size = send_data_by_485(dst_ptr, buf, read_len);

		if (size != read_len) {
			lt485_log(("%s() send data error(%d, %d)\n", __func__, read_len, size));
			ret = FAIL;
			break;
		}
	} while (read_len);

	lt_485_em_grpx_mutex_release(lt_if_use_by_em);

	return ret;
}


unsigned int get_em_protocol_data_event_bit(enum lt_485if_e rs485_if)
{
	unsigned eve_bit;

	switch (rs485_if) {
	case LT_485_TL16_U1:	eve_bit = EVENT_BIT_TL16_1_RECV_DATA;	break;
	case LT_485_TL16_U2:	eve_bit = EVENT_BIT_TL16_2_RECV_DATA;	break;
	case LT_485_TL16_U3:	eve_bit = EVENT_BIT_TL16_3_RECV_DATA;	break;
	case LT_485_TL16_U4:	eve_bit = EVENT_BIT_TL16_4_RECV_DATA;	break;
	case LT_485_TL16_U5:	eve_bit = EVENT_BIT_TL16_5_RECV_DATA;	break;
	case LT_485_TL16_U6:	eve_bit = EVENT_BIT_TL16_6_RECV_DATA;	break;
	case LT_485_TL16_U7:	eve_bit = EVENT_BIT_TL16_7_RECV_DATA;	break;
	case LT_485_TL16_U8:	eve_bit = EVENT_BIT_TL16_8_RECV_DATA;	break;

	case LT_485_EMC_UART:	eve_bit = EVENT_BIT_UART485_2_RECV_DATA;	break;
	case LT_485_EM_UART_1:	eve_bit = EVENT_BIT_UART485_3_RECV_DATA;	break;
	case LT_485_EM_UART_2:	eve_bit = EVENT_BIT_UART485_1_RECV_DATA;	break;

	default:
		eve_bit = 0;
		lt485_log(("%s() param invalid(%d)\n", __func__, rs485_if));
		break;
	}

	return eve_bit;
}


int is_tl16_interface(enum lt_485if_e rs485_if)
{
	switch (rs485_if) {
	case LT_485_TL16_U1:
	case LT_485_TL16_U2:
	case LT_485_TL16_U3:
	case LT_485_TL16_U4:
	case LT_485_TL16_U5:
	case LT_485_TL16_U6:
	case LT_485_TL16_U7:
	case LT_485_TL16_U8:
		return 1;

	default:
		return 0;
	}
}

struct protoc_analy_info_st *get_analys_info_ptr(enum lt_485if_e rs485_if)
{
	if (rs485_if>=LT_485_TL16_U1 && rs485_if<=LT_485_EMC_UART)
		return &proto_analy_info[rs485_if];
	else
		return NULL;
}


int tl16_set_uartparam_use_by_em(enum lt_485if_e rs485_if, enum lt_485if_e lt_if_use_by_em)
{
	struct uart_param uart485_data;
	int ret = SUCC;
	rt_uint8_t port;

	if (SUCC == get_tl16_uart_param(rs485_if+1, &uart485_data)) {
		if (LT_485_EM_UART_1 == lt_if_use_by_em) {
			port = 5;
		} else if (LT_485_EM_UART_2 == lt_if_use_by_em) {
			port = 2;
		} else {
			port = 5;
			printf_syn("%s(), line:%d, lt_if_use_by_em(%d) invalid\n", __func__, __LINE__,
					lt_if_use_by_em);
		}

		sw485_debug_body(("%s() line:%d, lt_if_use_by_em:%d -- %d, %d, %d, %d\n",
				__func__, __LINE__, lt_if_use_by_em,
				uart485_data.baudrate, uart485_data.databits,
				uart485_data.paritybit, uart485_data.stopbits));

		if (RT_EOK != UART_if_Set(uart485_data.baudrate, uart485_data.databits,
				uart485_data.paritybit, uart485_data.stopbits, port)) {
			lt485_log(("%s() modify uart param fail\n", __func__));
			ret = FAIL;
		}
	} else {
		lt485_log(("%s() param(%d, %d) error\n", __func__, rs485_if, lt_if_use_by_em));
		ret = FAIL;
	}

	return ret;
}

int emc_set_uartparam_use_by_em(enum lt_485if_e lt_if_use_by_em)
{
	int ret = SUCC;
	int port;

	sw485_debug_body(("%s() line:%d, lt_if_use_by_em(%d) -- %d, %d, %d, %d\n",
			__func__, __LINE__,  lt_if_use_by_em,
			emc_serial_param.baudrate, emc_serial_param.databits,
			emc_serial_param.paritybit, emc_serial_param.stopbits));

	if (LT_485_EM_UART_1 == lt_if_use_by_em) {
		port = 5;
	} else if (LT_485_EM_UART_2 == lt_if_use_by_em) {
		port = 2;
	} else {
		port = 5;
		printf_syn("%s(), line:%d, lt_if_use_by_em(%d) invalid\n", __func__, __LINE__,
				lt_if_use_by_em);
	}

	if (RT_EOK != UART_if_Set(emc_serial_param.baudrate, emc_serial_param.databits,
			emc_serial_param.paritybit, emc_serial_param.stopbits, port)) {
		lt485_log(("%s() modify uart param fail\n", __func__));
		ret = FAIL;
	}

	return ret;
}


#if 1
#define lt485_debug_t(x) 	//printf_syn x

static int start_auto_negotiation_timer(void)
{
#if 0
	if (is_bit_set(emc_auto_enter_negotiation_state_timer.parent.flag, RT_TIMER_FLAG_ACTIVATED))
		return SUCC;

	/* 只有已启动时，才会失败 */
	if (RT_EOK == rt_timer_start(&emc_auto_enter_negotiation_state_timer)) {
		lt485_debug_t(("%s() init_tick:%u, timeout_tick:%u, cur_tick:%u\n",
				__func__,
				emc_auto_enter_negotiation_state_timer.init_tick,
				emc_auto_enter_negotiation_state_timer.timeout_tick,
				rt_tick_get()));
	} else {
//		lt485_debug_t(("warning:%s() lt-if-%d\n", __func__, lt_if));
		return FAIL;
	}
#else
	wait_emc_negotiation_cmd = 0;
	start_auto_negoti_timer();
	had_start_auto_negotiation_timer = 1;
	sw485_debug_body(("%s() cur os tick:%u\n", __func__, rt_tick_get()));

#endif
	return SUCC;
}

#if 0
static int stop_auto_negotiation_timer(void)
{
#if 0
	/* 只有未启动时，才会失败 */
	if (RT_EOK == rt_timer_stop(&emc_auto_enter_negotiation_state_timer)) {
		lt485_debug_t(("%s() init_tick:%u, timeout_tick:%u, cur_tick:%u\n",
				__func__,
				emc_auto_enter_negotiation_state_timer.init_tick,
				emc_auto_enter_negotiation_state_timer.timeout_tick,
				rt_tick_get()));

		set_emc_uart_to_def_param();
	} else {
//		lt485_debug_t(("warning:%s() lt-if-%d\n", __func__, lt_if));
		return FAIL;
	}
#else
	stop_auto_negoti_timer();
	sw485_debug_body(("%s() cur os tick:%u\n", __func__, rt_tick_get()));

#endif
	return SUCC;
}
#endif

int reset_auto_negotiation_timer(void)
{
#if 0
	if(is_bit_set(emc_auto_enter_negotiation_state_timer.parent.flag, RT_TIMER_FLAG_ACTIVATED)) {
		rt_timer_stop(&emc_auto_enter_negotiation_state_timer);
		rt_timer_start(&emc_auto_enter_negotiation_state_timer);

		lt485_debug_t(("%s() init_tick:%u, timeout_tick:%u, cur_tick:%u\n",
				__func__,
				emc_auto_enter_negotiation_state_timer.init_tick,
				emc_auto_enter_negotiation_state_timer.timeout_tick,
				rt_tick_get()));
	} else {
//		lt485_debug_t(("warning:%s() lt-if-%d\n", __func__, lt_if));
	}
#else
	reset_auto_negoti_timer();
	lt485_debug_t(("%s() cur os tick:%u\n", __func__, rt_tick_get()));
#endif
	return SUCC;
}



#endif

rt_err_t lt_485_em_grpx_mutex_take(enum lt_485if_e rs485_if, enum lt_485if_e lt_if_use_by_em)
{
	rt_err_t err;
	struct rt_thread *thread;

	if (LT_485_EM_UART_1 == lt_if_use_by_em) {
		err = rt_mutex_take(&em_grp1_if_mutex, 0);
		thread = em_grp1_if_mutex.owner;
	} else if (LT_485_EM_UART_2 == lt_if_use_by_em) {
		err = rt_mutex_take(&em_grp2_if_mutex, 0);
		thread = em_grp2_if_mutex.owner;
	} else {
		printf_syn("%s(), lt_if_use_by_em(%d) invalid\n", __func__, lt_if_use_by_em);
		return RT_ERROR;
	}

	if (RT_EOK != err) {
		lt485_log(("th:%s, lt_if-%d take em_grpx_if_mutex fail(%d, %d), owner:%s\n",
				rt_thread_self()->name,
				rs485_if, lt_if_use_by_em, err, thread->name));
	}

	return err;
}

rt_err_t lt_485_em_grpx_mutex_release(enum lt_485if_e lt_if_use_by_em)
{
	rt_err_t err;
	struct rt_thread *thread;

	if (LT_485_EM_UART_1 == lt_if_use_by_em) {
		err = rt_mutex_release(&em_grp1_if_mutex);
		thread = em_grp1_if_mutex.owner;
	} else if (LT_485_EM_UART_2 == lt_if_use_by_em) {
		err = rt_mutex_release(&em_grp2_if_mutex);
		thread = em_grp2_if_mutex.owner;
	} else {
		printf_syn("%s(), lt_if_use_by_em(%d) invalid\n", __func__, lt_if_use_by_em);
		return RT_ERROR;
	}

	if (RT_EOK != err) {
		lt485_log(("th:%s, %s() release em_grpx_if_mutex fail(%d, %d), owner:%s\n",
				rt_thread_self()->name, __func__, lt_if_use_by_em, err, thread->name));
	}

	return err;
}
