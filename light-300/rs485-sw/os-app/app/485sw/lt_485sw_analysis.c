/*
 ******************************************************************************
 * lt_485sw_analysis.c
 *
 *  Created on: 2015-01-26
 *      Author: David, zhaoshaowei@yeejoin.com
 *
 * COPYRIGHT (C) 2015, YEEJOIN (Beijing) Technology Development Co., Ltd.
 ******************************************************************************
 */


#include <rtdef.h>
#include <ringbuf.h>
#include <misc_lib.h>
#include <syscfgdata.h>
#include <sys_cfg_api.h>
#include <debug_sw.h>
#include <rthw.h>

#include <lt_485sw.h>
#include "lt_485sw_analysis.h"

#define lt485_debug(x) 	//printf_syn x
#define lt485_info(x) 	printf_syn x
#define lt485_log(x) 	printf_syn x

#define PRINT_ACK_FRAME_DATA 1

#if 1
#define LT485SW_ANALYSIS_FRAME_TIMEOUT_MS	(LT_485SW_FRAME_WAIT_TIMEOUT_MS+100)
#else
#define LT485SW_ANALYSIS_FRAME_TIMEOUT_MS	(3*1000)
#endif

/*
 *
 * */
#define EM_IF_USE_ORDER_BY_TL16		0X01	/* tl16预约使用em if */

volatile unsigned em_if_use_state_vector;

#define set_bit_em_if_use_state_vector(v, mask) do {\
		rt_base_t _level;\
		_level = rt_hw_interrupt_disable();\
		set_bit((v), (mask));\
		rt_hw_interrupt_enable(_level);\
	} while(0)
#define clr_bit_em_if_use_state_vector(v, mask) do {\
		rt_base_t _level;\
		_level = rt_hw_interrupt_disable();\
		clr_bit((v), (mask));\
		rt_hw_interrupt_enable(_level);\
	} while(0)

struct protoc_analy_info_st proto_analy_info[PROTOCL_ANALYS_INFO_MAX];
struct rt_timer protoc_inter_byte_timer[PROTOCL_ANALYS_INFO_MAX];
enum lt_485if_e protoc_inter_byte_timer_param[PROTOCL_ANALYS_INFO_MAX];


static unsigned char rx_tx_ringbuf[PROTOCL_ANALYS_INFO_MAX][PROTOC_RX_TX_BUF_SIZE];

static enum ammeter_protocol_e get_tl16_protoc_type(enum lt_485if_e rs485_if);

static enum result_protoc_proc_e do_protoc_cmd_frame_analys(
		struct protoc_analy_info_st *analys_info, enum lt_485if_e rs485_if, int *had_proc);
static enum result_protoc_proc_e do_protoc_ack_frame_forward(struct protoc_analy_info_st *analys_info,
		enum lt_485if_e rs485_if, enum lt_485if_e lt_if_use_by_em);

static int send_protoc_cmd_from_tl16(struct protoc_analy_info_st *analys_info,
		enum result_protoc_proc_e*state_ret, int need_send_mb);
static int send_protoc_cmd_from_emc(struct protoc_analy_info_st *analys_info,
		enum result_protoc_proc_e*state_ret, int need_send_mb);
static int do_protoc_cmd_analys_end(struct protoc_analy_info_st *analys_info,
		enum result_protoc_proc_e state_ret, int is_tl16_if);
static void do_send_ack_frame_forward(enum lt_485if_e rs485_if, enum lt_485if_e lt_if_use_by_em);
static int do_protoc_ack_frame_end(struct protoc_analy_info_st *analys_info,
		enum result_protoc_proc_e state_ret);
static int convert_proto_analys_state_to_result(struct protoc_analy_info_st *analys_info,
		enum protocol_analysis_proc_e analysis_state,
		enum result_protoc_proc_e *result_state);
static enum protocol_analysis_proc_e _do_protoc_cmd_frame_analysis(
		struct protoc_analy_info_st *analys_info, char data);
static int is_send_cmd2local_em(struct protoc_analy_info_st *analys_info);

static int start_protoc_inter_byte_timer(enum lt_485if_e lt_if);
static int stop_protoc_inter_byte_timer(enum lt_485if_e lt_if);
static int reset_protoc_inter_byte_timer(enum lt_485if_e lt_if);
static void _reset_protoc_analy_info(struct protoc_analy_info_st *p, enum lt_485if_e rs485_if);

/* 只在初始化时调用 */
void init_protoc_analy_rb_sys(struct protoc_analy_info_st *p, char *name, enum lt_485if_e rs485_if)
{
	ringbuf_init_sys_init(&p->rb, name, rx_tx_ringbuf[rs485_if], sizeof(rx_tx_ringbuf[0]));
	return;
}

/* 只在初始化时调用 */
void init_protoc_analy_info(struct protoc_analy_info_st *p, enum lt_485if_e rs485_if)
{
	char str[] = {"emc1if"};

	if (NULL == p) {
		lt485_log(("%s(), param invalid\n", __func__));
		return;
	}

	if (rs485_if<LT_485_TL16_U1 || rs485_if>LT_485_EMC_UART) {
		lt485_log(("!!!NOTE:%s(), rs485_if-%d invalid\n", __func__, rs485_if));
		return;
	}

	p->lt_if = rs485_if;
	/* ringbuf_clr(&p->rb); */ /* 在init_protoc_analy_rb_sys中初始化 */
	p->lt_if_use_by_em = LT_485_BUTT;

	if (rs485_if>=LT_485_TL16_U1 && rs485_if<=LT_485_TL16_U8)
		p->protocol = get_tl16_protoc_type(rs485_if);
	else if (rs485_if == LT_485_EMC_UART)
		p->protocol = protocol_use_by_emc;
	else
		p->protocol = AP_PROTOCOL_UNKNOWN;

	p->state = PAP_IDLE;
	p->rstate = RPP_NOTHING;
	p->em_addr_offset = 0;
	p->if_had_get_lu_sem = 0;
	p->data_zone_len = 0;
	p->had_recv_data_cnt = 0;
	p->frame_pos	= 0;

	lt485_debug(("%s() rs485_if-%d rb size %d\n", __func__, rs485_if, sizeof(rx_tx_ringbuf[0])));

	if (p->protocol == AP_PROTOCOL_UNKNOWN)
		lt485_info(("info: %s() rs485_if-%d protoc is unknown\n", __func__, rs485_if));

	str[3] = '0' + rs485_if;
	rt_mutex_init(&p->emcx_if_mutex, str, RT_IPC_FLAG_PRIO);

	return;
}

static void _reset_protoc_analy_info(struct protoc_analy_info_st *p, enum lt_485if_e rs485_if)
{
	enum lt_485if_e lt_if_use_by_em;
	int tmp;

	if (NULL == p) {
		lt485_log(("%s(), param invalid\n", __func__));
		return;
	}

	if (rs485_if<LT_485_TL16_U1 || rs485_if>LT_485_EMC_UART || p->lt_if!=rs485_if) {
		lt485_log(("!!!NOTE:%s(), rs485_if-(%d,%d) invalid\n", __func__, rs485_if, p->lt_if));
		return;
	}

	lt_if_use_by_em = p->lt_if_use_by_em;
	tmp = p->if_had_get_lu_sem;
	switch (tmp) {
	case 0:
		/* nothing */
		break;
	case 1:
		rt_sem_release(&(em_grp1_if_lu_sem));
		if (LT_485_EM_UART_1 != lt_if_use_by_em)
			lt485_log(("line:%d, %s() if_lu_exception(%d, %d)\n", __LINE__, __func__,
								lt_if_use_by_em, tmp));
		break;
	case 2:
		rt_sem_release(&(em_grp2_if_lu_sem));
		if (LT_485_EM_UART_2 != lt_if_use_by_em)
			lt485_log(("line:%d, %s() if_lu_exception(%d, %d)\n", __LINE__, __func__,
								lt_if_use_by_em, tmp));
		break;
	default:
		lt485_log(("line:%d, %s() if_lu_exception(%d, %d)\n", __LINE__, __func__,
										lt_if_use_by_em, tmp));
		break;
	}

	p->if_had_get_lu_sem = 0;
	p->lt_if_use_by_em = LT_485_BUTT;

	ringbuf_reset(&p->rb);

	if (rs485_if>=LT_485_TL16_U1 && rs485_if<=LT_485_TL16_U8)
		p->protocol = get_tl16_protoc_type(rs485_if);
	else if (rs485_if == LT_485_EMC_UART)
		p->protocol = protocol_use_by_emc;
	else
		p->protocol = AP_PROTOCOL_UNKNOWN;

	p->state = PAP_IDLE;
	p->rstate = RPP_NOTHING;
	p->em_addr_offset = 0;
	p->data_zone_len = 0;
	p->had_recv_data_cnt = 0;
	p->frame_pos	= 0;

	lt485_debug(("%s() rs485_if-%d rb size %d\n", __func__, rs485_if, sizeof(rx_tx_ringbuf[0])));

	if (p->protocol == AP_PROTOCOL_UNKNOWN)
		lt485_info(("info: %s() rs485_if-%d protoc is unknown\n", __func__, rs485_if));

	return;
}

static enum ammeter_protocol_e get_tl16_protoc_type(enum lt_485if_e rs485_if)
{
	struct uart_param uartcfg;
	enum ammeter_protocol_e protoc = AP_PROTOCOL_UNKNOWN;

	if (SUCC == get_tl16_uart_param(rs485_if+1, &uartcfg)) {
		protoc = uartcfg.protoc;
	} else {
		printf_syn("%s(), get_tl16_uart_param fail\n", __func__, rs485_if);
	}

	return protoc;
}


/*
 * 该函数分析接收到的命令帧，转发完给本地em的完整命令帧后，要转发ack帧
 * */
enum result_protoc_proc_e proc_frame(struct protoc_analy_info_st *analys_info,
		enum lt_485if_e rs485_if, int *had_proc)
{
	enum result_protoc_proc_e proc_state;

	proc_state = do_protoc_cmd_frame_analys(analys_info, rs485_if, had_proc);

	switch (proc_state) {
	case RPP_ANALYSIS_OK:
		sw485_debug_body(("%s() wait send cmd over and ack\n", __func__));

		proc_state = do_protoc_ack_frame_forward(analys_info, rs485_if,
				analys_info->lt_if_use_by_em);
		if (RPP_ACK_FORWARD_OVER != proc_state) {
			lt485_info(("%s() rs485_if-%d do_protoc_ack_frame_forward error(%d)\n",
					__func__, rs485_if, proc_state));
		}
		_reset_protoc_analy_info(analys_info, rs485_if);

		break;

	case RPP_ANALYSIS_UNKNOW_PROTOC:
	case RPP_ACK_FORWARD_WAITING:
	case RPP_ACK_FORWARD_TIMEOUT:
	case RPP_ACK_FORWARD_OVER:
	case RPP_ACK_UNKNOW_ERR:
	case RPP_ANALYSIS_TIMEOUT:
	case RPP_ANALYSIS_ERR:
	case RPP_TL16_LONGTIME_USE_EMIF:
	case RPP_TL16_ORDER_USE_EMIF:
	case RPP_EMC_WAIT_IF_LU_SEM_TIMEOUT:
	case RPP_EMC_UNKNOW_ERR:
	case RPP_TL16_WAIT_IF_LU_SEM_TIMEOUT:
	case RPP_EMC_LONGTIME_USE_EMIF:
	case RPP_TL16_UNKNOW_ERR:
	case RPP_TL16_SEND_MAILB_ERR:
	case RPP_EMC_SEND_MAILB_ERR:
	case RPP_EMC_SEND_MAILB_ERR_HAD_SEM:
	case RPP_TL16_INVALID_IF_GRP_NO_OF_EM:
	case RPP_EMC_INVALID_IF_GRP_NO_OF_EM:
		_reset_protoc_analy_info(analys_info, rs485_if);
		break;

	default:
		break;
	}

	return proc_state;
}

/* 处理完接收到的数据返回, 如果分析出现异常, 复位analys_info */
static enum result_protoc_proc_e do_protoc_cmd_frame_analys(
		struct protoc_analy_info_st *analys_info, enum lt_485if_e rs485_if, int *had_proc)
{
	char buf[128];
	int i, recv_data_len;
	void *ptr;
	rt_device_t dev;
	rt_err_t err;

	enum result_protoc_proc_e *state_ret = &analys_info->rstate;

	if (LT_485_EMC_UART == rs485_if) {
		i = 1;
	} else {
		i = 0;
	}

	ptr = get_us485_dev_ptr(rs485_if, i);
	dev = get_us485_dev_ptr(rs485_if, 0);

	do {
		if (rs485_if>=LT_485_TL16_U1 && rs485_if<=LT_485_TL16_U8) {
			err = rt_mutex_take(&analys_info->emcx_if_mutex, 0);
			if (RT_EOK != err) {
				printf_syn("%s() get mutex fail(%d)\n", __func__, err);
				break;
			}
			recv_data_len = recv_data_by_tl16_485(ptr, buf, sizeof(buf));
			rt_mutex_release(&analys_info->emcx_if_mutex);

			if (0 == recv_data_len)
				break;

			sw485_debug_body(("%s() read %d bytes from rs485_if-%d, state:%d\n", __func__,
					recv_data_len, rs485_if, analys_info->state));

			++*had_proc;
			/* 分析字节流 */
			for (i=0; i<recv_data_len; ++i) {
				_do_protoc_cmd_frame_analysis(analys_info, buf[i]);
				if (FAIL==send_protoc_cmd_from_tl16(analys_info, state_ret, !i)) {
					/* 485sw工作时由于冲突肯定会出现这种失败, 这里不打印信息 */
					if (NULL != dev)
						dev->control(dev, RT_DEVICE_CTRL_CLR_RXBUF, NULL);

					goto ret_entry;
				}
			}
		} else if (LT_485_EMC_UART == rs485_if) {
//			reset_auto_negotiation_timer();

			err = rt_mutex_take(&analys_info->emcx_if_mutex, 0);
			if (RT_EOK != err) {
				printf_syn("%s() get mutex fail(%d)\n", __func__, err);
				break;
			}
			recv_data_len = recv_data_by_485(ptr, buf, sizeof(buf));
			rt_mutex_release(&analys_info->emcx_if_mutex);

			if (0 == recv_data_len)
				break;

//			reset_auto_negotiation_timer();

			sw485_debug_body(("%s() read %d bytes from rs485_if-%d, state:%d\n", __func__,
					recv_data_len, rs485_if, analys_info->state));

			++*had_proc;
			/* 分析字节流 */
			for (i=0; i<recv_data_len; ++i) {
				_do_protoc_cmd_frame_analysis(analys_info, buf[i]);
				if (FAIL==send_protoc_cmd_from_emc(analys_info, state_ret, !i)) {
					/* 485sw工作时由于冲突肯定会出现这种失败, 这里不打印信息 */
					if (NULL != dev)
						dev->control(dev, RT_DEVICE_CTRL_CLR_RXBUF, NULL);

					goto ret_entry;
				}
			}
		} else {
			recv_data_len = 0;
			lt485_log(("%s(), rs485_if-%d param error\n", __func__, rs485_if));
		}
	} while (recv_data_len);

ret_entry:
	return *state_ret;
}

/* 等待并转发em的ack数据,直到转发完所有ack数据或者出错，再返回 */
static enum result_protoc_proc_e do_protoc_ack_frame_forward(struct protoc_analy_info_st *analys_info,
		enum lt_485if_e rs485_if, enum lt_485if_e lt_if_use_by_em)
{
	enum result_protoc_proc_e proc_state;
	unsigned eve_bit;
	rt_uint32_t e;
	rt_err_t err;

	proc_state = RPP_ANALYSIS_OK;
	eve_bit    = get_em_protocol_data_event_bit(lt_if_use_by_em);

	while (1) {
		switch (proc_state) {
		case RPP_ANALYSIS_OK:
			sw485_debug_body(("%s() line:%d, rs485_if-%d wait ack\n",
					__func__, __LINE__, rs485_if));
			/* 等待命令帧发送完, em返回ack帧 */
			err = rt_event_recv(&em_protocol_data_event_set, eve_bit,
					RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
					get_ticks_of_ms(LT485SW_ANALYSIS_FRAME_TIMEOUT_MS), &e);

//			sw485_debug_body(("%s() line:%d, err:%d\n", __func__, __LINE__, err));
			if (RT_EOK == err) {
				proc_state = RPP_ACK_FORWARD_WAITING;
				do_send_ack_frame_forward(rs485_if, lt_if_use_by_em);
			} else if (-RT_ETIMEOUT == err) {
				proc_state = RPP_ACK_FORWARD_TIMEOUT;
				goto ret_entry;
			} else {
				lt485_log(("%s() line(%d) recv event error(%d)\n", __func__, __LINE__, err));
				proc_state = RPP_ACK_UNKNOW_ERR;
				goto ret_entry;
			}
			break;

		case RPP_ACK_FORWARD_WAITING:
			err = rt_event_recv(&em_protocol_data_event_set, eve_bit,
					RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
					get_ticks_of_ms(LT_485SW_BYTE_WAIT_TIMEOUT_MS), &e);
			if (RT_EOK == err) {
				do_send_ack_frame_forward(rs485_if, lt_if_use_by_em);
			} else if (-RT_ETIMEOUT == err) {
				/* 以超时无数据作为ack帧的结束判断条件 */
				proc_state = RPP_ACK_FORWARD_OVER;
				goto ret_entry;
			} else {
				lt485_log(("%s() line(%d) recv event error(%d)\n", __func__, __LINE__, err));
				proc_state = RPP_ACK_UNKNOW_ERR;
				goto ret_entry;
			}
			break;

		default:
			lt485_log(("%s important state(%d) err\n", __func__, proc_state));
			proc_state = RPP_ACK_UNKNOW_ERR;
			goto ret_entry;
		}
	}

ret_entry:
	do_protoc_ack_frame_end(analys_info, proc_state);
	return proc_state;
}

static int do_send_mb_to_em(struct protoc_analy_info_st *analys_info, rt_err_t *err)
{
	int ret = SUCC;
	enum lt_485if_e lt_if_use_by_em;

	lt_if_use_by_em = analys_info->lt_if_use_by_em;

	if (LT_485_EM_UART_1 == lt_if_use_by_em) {
		*err = rt_mb_send(&send_cmd2em_grp1_mb, (rt_uint32_t)analys_info);
	} else if (LT_485_EM_UART_2 == lt_if_use_by_em) {
		*err = rt_mb_send(&send_cmd2em_grp2_mb, (rt_uint32_t)analys_info);
	} else {
		ret = FAIL;
	}

	return ret;
}


static int send_protoc_cmd_from_tl16(struct protoc_analy_info_st *analys_info,
		enum result_protoc_proc_e *state_ret, int need_send_mb)
{
	rt_err_t err;
	int ret;
	enum result_protoc_proc_e state;
	enum lt_485if_e lt_if_use_by_em;

	if (SUCC == convert_proto_analys_state_to_result(analys_info, analys_info->state, state_ret)) {
		state = *state_ret;
		lt_if_use_by_em = analys_info->lt_if_use_by_em;

		switch (state) {
		case RPP_ANALYSIS_PENDING:
			set_bit_em_if_use_state_vector(em_if_use_state_vector, EM_IF_USE_ORDER_BY_TL16);
			break;

		case RPP_ANALYSIS_START_FORWARD:
			if (LT_485_EM_UART_1 == lt_if_use_by_em) {
				err = rt_sem_take(&em_grp1_if_lu_sem, get_ticks_of_ms(LT_485SW_BYTE_WAIT_TIMEOUT_MS));
			} else if (LT_485_EM_UART_2 == lt_if_use_by_em) {
				err = rt_sem_take(&em_grp2_if_lu_sem, get_ticks_of_ms(LT_485SW_BYTE_WAIT_TIMEOUT_MS));
			} else {
				state = RPP_TL16_INVALID_IF_GRP_NO_OF_EM;
				lt485_log(("ERR(line:%d):%s() lt_if-%d(%d) if use by em invalid(%d)\n",
							__LINE__, __func__,
							analys_info->lt_if, state, lt_if_use_by_em));
				break;
			}

			lt485_debug(("th:%s, take grpx if lu sem(%d), lt_if_use_by_em:%d\n",
					rt_thread_self()->name, err, lt_if_use_by_em));

			if (RT_EOK == err) {
				if (LT_485_EM_UART_1 == lt_if_use_by_em) {
					analys_info->if_had_get_lu_sem = 1;
				} else if (LT_485_EM_UART_2 == lt_if_use_by_em) {
					analys_info->if_had_get_lu_sem = 2;
				} else {
					state = RPP_TL16_INVALID_IF_GRP_NO_OF_EM;
					lt485_log(("ERR(line:%d):%s() lt_if-%d(%d) if use by em invalid(%d)\n",
								__LINE__, __func__,
								analys_info->lt_if, state, lt_if_use_by_em));
					break;
				}


				clr_bit_em_if_use_state_vector(em_if_use_state_vector, EM_IF_USE_ORDER_BY_TL16);
				analys_info->frame_pos = 1;

				if (SUCC != do_send_mb_to_em(analys_info, &err)) {
					lt485_log(("ERR:%s() lt_if-%d(%d) if use by em invalid(%d)\n",
						__func__, analys_info->lt_if, state, lt_if_use_by_em));
					state = RPP_TL16_INVALID_IF_GRP_NO_OF_EM;
					break;
				}

				sw485_debug_body(("%s line-%d lt_if-%d send mb\n", __func__, __LINE__,
						analys_info->lt_if));

				if (RT_EOK != err) {
					lt485_log(("ERR:%s() lt_if-%d(%d) send mb fail(%d)\n",
							__func__, analys_info->lt_if, state, err));
					state = RPP_TL16_SEND_MAILB_ERR;
				}
			} else if (-RT_ETIMEOUT == err) {
				state = RPP_TL16_WAIT_IF_LU_SEM_TIMEOUT;
			} else {
				state = RPP_TL16_UNKNOW_ERR;
				lt485_log(("!!NOTE:%s() lt-if-%d take sem fail(%d)", __func__,
						analys_info->lt_if, err));
			}
			break;

		case RPP_ANALYSIS_FORWARDING:
		case RPP_ANALYSIS_OK:
			if (need_send_mb || RPP_ANALYSIS_OK==state) {
				if (RPP_ANALYSIS_OK==state) {
					analys_info->frame_pos = 2;
				}

				if (SUCC != do_send_mb_to_em(analys_info, &err)) {
					lt485_log(("ERR:%s() lt_if-%d(%d) if use by em invalid(%d)\n",
						__func__, analys_info->lt_if, state, lt_if_use_by_em));
					state = RPP_TL16_INVALID_IF_GRP_NO_OF_EM;
					break;
				}

				sw485_debug_body(("%s line-%d send mb\n", __func__, __LINE__));

				if (RT_EOK != err) {
					lt485_log(("ERR:%s() lt_if-%d(%d) send mb fail(%d)\n",
							__func__, analys_info->lt_if, state, err));
					state = RPP_TL16_SEND_MAILB_ERR;
				}
			}
			break;

		case RPP_ANALYSIS_START_CONSUME:
		case RPP_ANALYSIS_CONSUMING:
			clr_bit_em_if_use_state_vector(em_if_use_state_vector, EM_IF_USE_ORDER_BY_TL16);
			ringbuf_get(&analys_info->rb);
			break;

		case RPP_ANALYSIS_CONSUME_OVER:
			clr_bit_em_if_use_state_vector(em_if_use_state_vector, EM_IF_USE_ORDER_BY_TL16);
			ringbuf_clr(&analys_info->rb);
			_reset_protoc_analy_info(analys_info, analys_info->lt_if);
			break;

		case RPP_ANALYSIS_OK_CONSUME:
			clr_bit_em_if_use_state_vector(em_if_use_state_vector, EM_IF_USE_ORDER_BY_TL16);
			ringbuf_get(&analys_info->rb);
			_reset_protoc_analy_info(analys_info, analys_info->lt_if);
			break;

		case RPP_NOTHING:
			/* nothing */
			break;

		default:
			lt485_log(("ERR:%s() state(%d) error\n", __func__, state));
			break;
		}
	} else {
		state = *state_ret;
	}

	ret = do_protoc_cmd_analys_end(analys_info, state, 1);

	*state_ret = state;

	return ret;
}

static int send_protoc_cmd_from_emc(struct protoc_analy_info_st *analys_info,
		enum result_protoc_proc_e *state_ret, int need_send_mb)
{
	rt_err_t err;
	int ret;
	enum result_protoc_proc_e state;
	enum lt_485if_e lt_if_use_by_em;
	rt_base_t level;

	if (SUCC == convert_proto_analys_state_to_result(analys_info, analys_info->state, state_ret)) {
		state = *state_ret;
		lt_if_use_by_em = analys_info->lt_if_use_by_em;

		level = rt_hw_interrupt_disable();
		if (is_bit_set(em_if_use_state_vector, EM_IF_USE_ORDER_BY_TL16)) {
			state = RPP_TL16_ORDER_USE_EMIF;
			rt_hw_interrupt_enable(level);
			goto ret_entry;
		}
		rt_hw_interrupt_enable(level);

		switch (state) {
		case RPP_ANALYSIS_START_FORWARD:
			if (LT_485_EM_UART_1 == lt_if_use_by_em) {
				err = rt_sem_take(&em_grp1_if_lu_sem, get_ticks_of_ms(LT_485SW_BYTE_WAIT_TIMEOUT_MS));
			} else if (LT_485_EM_UART_2 == lt_if_use_by_em) {
				err = rt_sem_take(&em_grp2_if_lu_sem, get_ticks_of_ms(LT_485SW_BYTE_WAIT_TIMEOUT_MS));
			} else {
				state = RPP_EMC_INVALID_IF_GRP_NO_OF_EM;
				lt485_log(("ERR(line:%d):%s() lt_if-%d(%d) if use by em invalid(%d)\n",
							__LINE__, __func__,
							analys_info->lt_if, state, lt_if_use_by_em));
				break;
			}

			lt485_debug(("th:%s, take grpx if lu sem(%d), lt_if_use_by_em:%d\n",
					rt_thread_self()->name, err, lt_if_use_by_em));

			if (RT_EOK == err) {
				if (LT_485_EM_UART_1 == lt_if_use_by_em) {
					analys_info->if_had_get_lu_sem = 1;
				} else if (LT_485_EM_UART_2 == lt_if_use_by_em) {
					analys_info->if_had_get_lu_sem = 2;
				} else {
					state = RPP_EMC_INVALID_IF_GRP_NO_OF_EM;
					lt485_log(("ERR(line:%d):%s() lt_if-%d(%d) if use by em invalid(%d)\n",
								__LINE__, __func__,
								analys_info->lt_if, state, lt_if_use_by_em));
					break;
				}

				if (SUCC != do_send_mb_to_em(analys_info, &err)) {
					lt485_log(("ERR:%s() lt_if-%d(%d) if use by em invalid(%d)\n",
						__func__, analys_info->lt_if, state, lt_if_use_by_em));
					state = RPP_EMC_INVALID_IF_GRP_NO_OF_EM;
					break;
				}

				sw485_debug_body(("%s() lt_if-%d send mb, cur_tick:%u\n", __func__,
						analys_info->lt_if, rt_tick_get()));

				if (RT_EOK != err) {
					lt485_log(("ERR:%s() lt_if-%d(state) send mb fail(%d)\n",
							__func__, analys_info->lt_if, state, err));
					state = RPP_EMC_SEND_MAILB_ERR;
				}
			} else if (-RT_ETIMEOUT == err) {
				state = RPP_EMC_WAIT_IF_LU_SEM_TIMEOUT;
			} else {
				state = RPP_EMC_UNKNOW_ERR;
				lt485_log(("!!NOTE:%s() lt-if-%d(%d) take sem fail(%d)", __func__,
						analys_info->lt_if, state, err));
			}
			break;

		case RPP_ANALYSIS_FORWARDING:
			if (need_send_mb) {
				if (SUCC != do_send_mb_to_em(analys_info, &err)) {
					lt485_log(("ERR:%s() lt_if-%d(%d) if use by em invalid(%d)\n",
						__func__, analys_info->lt_if, state, lt_if_use_by_em));
					state = RPP_EMC_INVALID_IF_GRP_NO_OF_EM;
					break;
				}

				if (RT_EOK != err) {
					lt485_log(("ERR:%s() lt-if-%d(%d) send mb fail(%d)\n", __func__,
							analys_info->lt_if, state, err));
					state = RPP_EMC_SEND_MAILB_ERR;
				}
			}
			break;

		case RPP_ANALYSIS_OK:
			if (need_send_mb  || RPP_ANALYSIS_OK==state) {
				if (SUCC != do_send_mb_to_em(analys_info, &err)) {
					lt485_log(("ERR:%s() lt_if-%d(%d) if use by em invalid(%d)\n",
						__func__, analys_info->lt_if, state, lt_if_use_by_em));
					state = RPP_EMC_INVALID_IF_GRP_NO_OF_EM;
					break;
				}

				if (RT_EOK != err) {
					lt485_log(("ERR:%s() lt_if-%d(%d) send mb fail(%d)\n",
							__func__, analys_info->lt_if, state, err));
					state = RPP_EMC_SEND_MAILB_ERR_HAD_SEM;
				}
			}
			break;

		case RPP_ANALYSIS_START_CONSUME:
		case RPP_ANALYSIS_CONSUMING:
			ringbuf_get(&analys_info->rb);
			sw485_debug_body(("warning: %s() lt_if-%d recv non-local em cmd...\n", __func__,
					analys_info->lt_if));
			break;
		case RPP_ANALYSIS_OK_CONSUME:
		case RPP_ANALYSIS_CONSUME_OVER:
			sw485_debug_body(("!warning: %s() lt_if-%d recv non-local em cmd...\n", __func__,
					analys_info->lt_if));
			_reset_protoc_analy_info(analys_info, analys_info->lt_if);
			break;

		case RPP_NOTHING:
		case RPP_ANALYSIS_PENDING:
			/* nothing */
			break;

		default:
			lt485_log(("!!NOTE:%s() lt_if-%d state(%d) error\n", __func__,
					analys_info->lt_if, state));
			break;
		}
	} else {
		state = *state_ret;
	}


ret_entry:
	ret = do_protoc_cmd_analys_end(analys_info, state, 0);

	*state_ret = state;

	return ret;
}

#define clean_em_grpx_if_lu_sem(s, mask)		do {\
	rt_base_t level;\
	level = rt_hw_interrupt_disable();\
	if (is_bit_set(em_if_use_state_vector, (mask))) {\
		clr_bit(em_if_use_state_vector, (mask));\
		rt_sem_release(&(s));\
	}\
	rt_hw_interrupt_enable(level);\
} while (0)

/* send_protoc_cmd_from_*()处理正常状态，do_protoc_cmd_analys_end()处理异常状态 */
static int do_protoc_cmd_analys_end(struct protoc_analy_info_st *analys_info,
		enum result_protoc_proc_e state_ret, int is_tl16_if)
{
	int ret = FAIL;
	enum lt_485if_e lt_if_use_by_em;

	lt_if_use_by_em = analys_info->lt_if_use_by_em;

	switch (state_ret) {
	case RPP_NOTHING:
	case RPP_ANALYSIS_PENDING:
	case RPP_ANALYSIS_START_FORWARD:
	case RPP_ANALYSIS_FORWARDING:
	case RPP_ANALYSIS_START_CONSUME:
	case RPP_ANALYSIS_CONSUMING:

	case RPP_ANALYSIS_OK:
	case RPP_ANALYSIS_OK_CONSUME:
	case RPP_ANALYSIS_FORWARD_OVER: /* send_protoc_cmd_from_* 没有处理 */
	case RPP_ANALYSIS_CONSUME_OVER:
		/* nothing */
		ret = SUCC;
		break;

	case RPP_ANALYSIS_UNKNOW_PROTOC:
	case RPP_ACK_FORWARD_WAITING:
	case RPP_ACK_FORWARD_TIMEOUT:
	case RPP_ACK_FORWARD_OVER:
	case RPP_ACK_UNKNOW_ERR:
		lt485_info(("INFO:%s() lt-if:%d shuold not proc state(%d)\n", __func__,
				analys_info->lt_if, state_ret));
		break;

	case RPP_ANALYSIS_TIMEOUT:
	case RPP_ANALYSIS_ERR:
	case RPP_TL16_LONGTIME_USE_EMIF:
	case RPP_TL16_ORDER_USE_EMIF:
	case RPP_EMC_WAIT_IF_LU_SEM_TIMEOUT:
	case RPP_EMC_UNKNOW_ERR:

	case RPP_TL16_WAIT_IF_LU_SEM_TIMEOUT:
	case RPP_EMC_LONGTIME_USE_EMIF:
	case RPP_TL16_UNKNOW_ERR:
		if (is_tl16_interface(analys_info->lt_if)) {
			clr_bit_em_if_use_state_vector(em_if_use_state_vector, EM_IF_USE_ORDER_BY_TL16);
		}

		if (1 == analys_info->if_had_get_lu_sem) {
			rt_sem_release(&(em_grp1_if_lu_sem));
		} else if (2 == analys_info->if_had_get_lu_sem) {
			rt_sem_release(&(em_grp2_if_lu_sem));
		} else {
			/* nothing */
		}

		analys_info->if_had_get_lu_sem = 0;
		break;

	case RPP_TL16_SEND_MAILB_ERR:
		if (1 == analys_info->if_had_get_lu_sem) {
			rt_sem_release(&(em_grp1_if_lu_sem));
		} else if (2 == analys_info->if_had_get_lu_sem) {
			rt_sem_release(&(em_grp2_if_lu_sem));
		} else {
			lt485_log(("ERR(line:%d):%s() lt_if-%d if use by em invalid(%d)\n",
						__LINE__, __func__,
						analys_info->lt_if, lt_if_use_by_em));
		}

		analys_info->if_had_get_lu_sem = 0;
		break;

	case RPP_EMC_SEND_MAILB_ERR:
	case RPP_EMC_SEND_MAILB_ERR_HAD_SEM:
		if (1 == analys_info->if_had_get_lu_sem) {
			rt_sem_release(&(em_grp1_if_lu_sem));
		} else if (2 == analys_info->if_had_get_lu_sem) {
			rt_sem_release(&(em_grp2_if_lu_sem));
		} else {
			lt485_log(("ERR(line:%d):%s() lt_if-%d if use by em invalid(%d)\n",
						__LINE__, __func__,
						analys_info->lt_if, lt_if_use_by_em));
		}

		analys_info->if_had_get_lu_sem = 0;
		break;

	case RPP_TL16_INVALID_IF_GRP_NO_OF_EM:
	case RPP_EMC_INVALID_IF_GRP_NO_OF_EM:
		if (1 == analys_info->if_had_get_lu_sem) {
			rt_sem_release(&(em_grp1_if_lu_sem));
		} else if (2 == analys_info->if_had_get_lu_sem) {
			rt_sem_release(&(em_grp2_if_lu_sem));
		} else {
			/* nothing */
		}

		analys_info->if_had_get_lu_sem = 0;
		break;

	default:
		lt485_log(("!!!NOTE:%s() lt-if-%d state(%d) error\n", __func__,
				analys_info->lt_if, state_ret));
		break;
	}

	if (FAIL == ret) {
		stop_protoc_inter_byte_timer(analys_info->lt_if);
		_reset_protoc_analy_info(analys_info, analys_info->lt_if);
	}

	return ret;
}

/* 将em的ack数据转发给rs485_if */
static void do_send_ack_frame_forward(enum lt_485if_e rs485_if, enum lt_485if_e lt_if_use_by_em)
{
	char buf[64];
	int recv_data_len, send_data_len, i;
	void *dst_ptr;
	USART_TypeDef  *src_ptr;
	struct protoc_analy_info_st *analys_info;
	rt_mutex_t mutex;
	rt_err_t err;

	if (LT_485_EMC_UART == rs485_if)
		send_data_len = 1;
	else
		send_data_len = 0;

	dst_ptr = get_us485_dev_ptr(rs485_if, send_data_len);
	src_ptr = get_us485_dev_ptr(lt_if_use_by_em, 1);

	if (NULL==dst_ptr || NULL==src_ptr) {
		return;
	}

	/* 会抢占emc使用连接em的485口 */
	if (RT_EOK != lt_485_em_grpx_mutex_take(rs485_if, lt_if_use_by_em)) {
		return;
	}

	sw485_debug_body(("%s() line-%d lt_if-%d, lt_if_use_by_em-%d\n",
			__func__, __LINE__, rs485_if, lt_if_use_by_em));

	analys_info = get_analys_info_ptr(rs485_if);
	if (NULL == analys_info) {
		printf_syn("%s(), rs485_if-%d invalid\n", __func__, rs485_if);
		goto ret_entry;
	}

	mutex = &analys_info->emcx_if_mutex;
	do {
		recv_data_len = recv_data_by_485(src_ptr, buf, sizeof(buf));
#if PRINT_ACK_FRAME_DATA
		if (is_sub_m_on(dsw_sub_module[DMN_485SW], DSMN_485SW_ACK_FRAME_DATA)) {
			for (i=0; i<recv_data_len; ++i) {
				printf_syn("ack2if-%d:%x\n", rs485_if, buf[i]);
			}
		}
#endif
		if (rs485_if>=LT_485_TL16_U1 && rs485_if<=LT_485_TL16_U8) {
			err = rt_mutex_take(mutex, 0);
			if (RT_EOK != err) {
				printf_syn("%s() get mutex fail(%d), rs485_if-%d\n", __func__, err, rs485_if);
				break;
			}
			send_data_len = send_data_by_tl16_485(dst_ptr, buf, recv_data_len);
			rt_mutex_release(mutex);

			if (send_data_len != recv_data_len) {
				lt485_log(("%s() rs485_if-%d error (rx/tx len: %d/%d)\n", __func__,
						rs485_if, recv_data_len, send_data_len));
				break;
			}
		} else if (LT_485_EMC_UART == rs485_if) {
//			reset_auto_negotiation_timer();

			err = rt_mutex_take(mutex, 0);
			if (RT_EOK != err) {
				printf_syn("%s() get mutex fail(%d), rs485_if-%d\n", __func__, err, rs485_if);
				break;
			}
			send_data_len = send_data_by_485(dst_ptr, buf, recv_data_len);
			rt_mutex_release(mutex);

//			reset_auto_negotiation_timer();

			if (send_data_len != recv_data_len) {
				lt485_log(("%s() rs485_if-%d error (rx/tx len: %d/%d)\n", __func__,
						rs485_if, recv_data_len, send_data_len));
				break;
			}
		} else {
			lt485_log(("%s(), rs485_if-%d param error\n", __func__, rs485_if));
			break;
		}
	} while (recv_data_len);

ret_entry:
	lt_485_em_grpx_mutex_release(lt_if_use_by_em);

	return;
}

static int do_protoc_ack_frame_end(struct protoc_analy_info_st *analys_info,
		enum result_protoc_proc_e state_ret)
{
	int ret = SUCC, release = 0;
	enum lt_485if_e lt_if_use_by_em;

	lt_if_use_by_em = analys_info->lt_if_use_by_em;

	switch (state_ret) {
	case RPP_NOTHING:
		/* nothing */
		break;

	case RPP_ANALYSIS_UNKNOW_PROTOC:
	case RPP_ANALYSIS_PENDING:
	case RPP_ANALYSIS_START_FORWARD:
	case RPP_ANALYSIS_FORWARDING:
	case RPP_ANALYSIS_START_CONSUME:
	case RPP_ANALYSIS_CONSUMING:
	case RPP_ANALYSIS_OK:
	case RPP_ANALYSIS_OK_CONSUME:
	case RPP_ANALYSIS_FORWARD_OVER:
	case RPP_ANALYSIS_CONSUME_OVER:
	case RPP_ANALYSIS_TIMEOUT:
	case RPP_ANALYSIS_ERR:
	case RPP_TL16_SEND_MAILB_ERR:
	case RPP_EMC_SEND_MAILB_ERR_HAD_SEM:
	case RPP_EMC_SEND_MAILB_ERR:
	case RPP_TL16_LONGTIME_USE_EMIF:
	case RPP_TL16_ORDER_USE_EMIF:
	case RPP_EMC_WAIT_IF_LU_SEM_TIMEOUT:
	case RPP_EMC_UNKNOW_ERR:
	case RPP_TL16_WAIT_IF_LU_SEM_TIMEOUT:
	case RPP_EMC_LONGTIME_USE_EMIF:
	case RPP_TL16_UNKNOW_ERR:
		lt485_info(("%s() shuold not proc state(%d)\n", __func__, state_ret));
		ret = FAIL;
		break;

	case RPP_ACK_FORWARD_WAITING:
		/* nothing */
		break;

	case RPP_ACK_FORWARD_TIMEOUT:
	case RPP_ACK_UNKNOW_ERR:
		ret = FAIL;
		break;

	case RPP_ACK_FORWARD_OVER:
		release = 1;
		break;

	default:
		ret = FAIL;
		lt485_log(("!!NOTE:%s() state(%d) error\n", __func__, state_ret));
		break;
	}

	if (SUCC!=ret || release) {
		if (1 == analys_info->if_had_get_lu_sem) {
			rt_sem_release(&(em_grp1_if_lu_sem));
		} else if (2 == analys_info->if_had_get_lu_sem) {
			rt_sem_release(&(em_grp2_if_lu_sem));
		} else {
			lt485_log(("ERR(line:%d):%s() lt_if-%d if use by em invalid(%d)\n",
						__LINE__, __func__,
						analys_info->lt_if, lt_if_use_by_em));
		}

		analys_info->if_had_get_lu_sem = 0;

		if (is_tl16_interface(analys_info->lt_if)) {
			clr_bit_em_if_use_state_vector(em_if_use_state_vector, EM_IF_USE_ORDER_BY_TL16);
		}

		_reset_protoc_analy_info(analys_info, analys_info->lt_if);
	}

	return ret;
}

/* 将协议分析的状态转换为协议处理结果的状态 */
static int convert_proto_analys_state_to_result(struct protoc_analy_info_st *analys_info,
		enum protocol_analysis_proc_e analysis_state,
		enum result_protoc_proc_e *result_state)
{
	enum result_protoc_proc_e old_rstate, new_rstate;;
	int ret = SUCC;

	lt485_debug(("%s() line:%d, lt_if-%d before, state:%d, rstate:%d\n", __func__, __LINE__,
			analys_info->lt_if, analysis_state, *result_state));

	old_rstate = new_rstate = *result_state;
	switch (analysis_state) {
	case PAP_FRAME_START_OK:	/* 帧起始字节正确 */
	case PAP_RECV_ADDR:		/* 正在接收地址域数据 */
		new_rstate = RPP_ANALYSIS_PENDING;
		break;

	case PAP_RECV_ADDR_OK:	/* 地址域后的帧起始符正确 */
		if (is_send_cmd2local_em(analys_info))
			new_rstate = RPP_ANALYSIS_START_FORWARD;
		else
			new_rstate = RPP_ANALYSIS_START_CONSUME;
		break;

	case PAP_RECV_CTRL_WORD:	/* 接收到控制字 */
	case PAP_RECV_DATA_LEN:	/* 接收到数据域长度 */
	case PAP_RECV_OTHER_DATA:	/* 正在接收其他数据 */
		if (old_rstate == RPP_ANALYSIS_START_FORWARD) {
			new_rstate = RPP_ANALYSIS_FORWARDING;
		} else {
			if (RPP_ANALYSIS_FORWARDING!=old_rstate
					&& RPP_ANALYSIS_CONSUMING!=old_rstate)
				new_rstate = RPP_ANALYSIS_CONSUMING;
		}
		break;

	case PAP_FRAME_END_OK:	/* 接收完一个完整的数据帧 */
		if (old_rstate == RPP_ANALYSIS_FORWARDING)
			new_rstate = RPP_ANALYSIS_OK;
		else
			new_rstate = RPP_ANALYSIS_OK_CONSUME;
		break;

	case PAP_FRAME_TIMEOUT:	/* 接收cmd时超时 */
		ret = FAIL;
		new_rstate = RPP_ANALYSIS_TIMEOUT;
		break;
	case PAP_FRAME_ERR:		/* 接收cmd时发生错误 */
		ret = FAIL;
		new_rstate = RPP_ANALYSIS_ERR;
		break;

	case PAP_IDLE:
		new_rstate = RPP_NOTHING;
		break;

	default:
		ret = FAIL;
		new_rstate = RPP_ANALYSIS_ERR;
		printf_syn("ERR:%s(), param error(%d)\n", __func__, analysis_state);
		break;
#if 0
	case PAP_WAIT_ACK:		/* 等待ack数据 */
	case PAP_FORWARD_ACK_START:	/* 开始转发ack数据 */
	case PAP_FORWARD_TIMEOUT:	/* 转发ack数据时，等待超时 */
	case PAP_FORWARD_OVER:	/* 转发ack完成 */
#endif
	}

	*result_state = new_rstate;

	lt485_debug(("%s() line:%d, lt_if-%d after, state:%d, rstate:%d\n", __func__, __LINE__,
			analys_info->lt_if, analysis_state, *result_state));

	return ret;
}

#if 1
#define protoco_debug(x) do {\
	if (x == state_ret)\
		sw485_debug_body(("%s() line-%d lt_if-%d change protoc analys state\n", \
		__func__, __LINE__, lt_if));\
	} while (0)
#else
#define protoco_debug(x)
#endif

/*
 * 根据协议类型逐个字节分析cmd帧, 数据记录到rb, 切换分析状态
 *
 *
 * 	645-97 read em sn: fe fe fe fe 68 47 54 00 12 00 00 68 01 02 65 f3 d8 16
 * 	edmi enter cmd-mode: 02 45 0c 86 7f ed 00 00 00 01 00 14 37 d7 03
 * */
static enum protocol_analysis_proc_e _do_protoc_cmd_frame_analysis(
		struct protoc_analy_info_st *analys_info, char data)
{
	char ch;
	enum protocol_analysis_proc_e state_ret = analys_info->state;
	enum ammeter_protocol_e protocol = analys_info->protocol;
	enum lt_485if_e lt_if = analys_info->lt_if;
	struct ringbuf *r = &analys_info->rb;

	if (AP_PROTOCOL_UNKNOWN==protocol) {
		lt485_info(("%s() lt_if-%d protocol unknown\n", __func__, lt_if));
		return state_ret;
	}

	reset_protoc_inter_byte_timer(lt_if);

	lt485_debug(("if-%d put %x to rb, protoc:%d\n", lt_if, data, protocol));

	switch (state_ret) {
	case PAP_IDLE:
		ringbuf_put(r, data);
		if (protocol == AP_PROTOCOL_645_1997 || protocol == AP_PROTOCOL_645_2007) {
			if (data == 0xFE) {
				state_ret = PAP_IDLE;
			} else if (data == 0x68) {
				state_ret = PAP_FRAME_START_OK;
//				ringbuf_clr(r);
//				ringbuf_put(r, data);
			} else {
				state_ret = PAP_FRAME_ERR;
			}
		} else if (protocol == AP_PROTOCOL_EDMI) {
			if (data == 0xFF) {
				state_ret = PAP_IDLE;
			} else if (data == 0x02) {
				state_ret = PAP_FRAME_START_OK;
//				ringbuf_clr(r);
//				ringbuf_put(r, data);
			} else {
				state_ret = PAP_FRAME_ERR;
			}
		} else {
			state_ret = PAP_FRAME_ERR;
			lt485_log(("%s(), line-%d, lt_if-%d protoco err %d\n", __func__, __LINE__,
					lt_if, protocol));
		}

		if (state_ret == PAP_FRAME_START_OK) {
			start_protoc_inter_byte_timer(lt_if);
			analys_info->lt_if_use_by_em = LT_485_PHONY;
			lt485_debug(("%s() line-%d lt_if-%d change protoc analys state\n",
					__func__, __LINE__, lt_if));
		}
		break;

	case PAP_FRAME_START_OK:
		ringbuf_put(r, data);
		if (protocol == AP_PROTOCOL_645_1997 || protocol == AP_PROTOCOL_645_2007) {
			state_ret = PAP_RECV_ADDR;
		} else if (protocol == AP_PROTOCOL_EDMI) {
			state_ret = PAP_RECV_ADDR;
		} else {
			state_ret = PAP_FRAME_ERR;
			lt485_log(("%s(), line-%d, lt_if-%d protoco err %d\n", __func__, __LINE__,
					lt_if, protocol));
		}
		protoco_debug(PAP_RECV_ADDR);
		break;

	case PAP_RECV_ADDR:
		ringbuf_put(r, data);
		ringbuf_get_slice(r, 0, &ch, 1);

//		sw485_debug_body(("%s(), line:%d, elements:%d, ch:0x%x\n", __func__, __LINE__,
//				ringbuf_elements(r), ch));

		if (protocol == AP_PROTOCOL_645_1997 || protocol == AP_PROTOCOL_645_2007) {
			if (0xfe == ch)
				ch = 4+1;
			else
				ch = 1;
			if (ringbuf_elements(r) >= 6+ch+1) {
				if (data == 0x68) {
					state_ret = PAP_RECV_ADDR_OK;	/* mark */
					analys_info->em_addr_offset = ch;
				} else {
					state_ret = PAP_FRAME_ERR;
					lt485_log(("%s() line:%d, lt_if-%d 645-protoco after addr error 0x%x\n",
							__func__, __LINE__, analys_info->lt_if, data));
				}
			}
		} else if (protocol == AP_PROTOCOL_EDMI) {
			if (0xff==ch)
				ch = 1+1+1;	/* 0xff, 0x02, 0x45 */
			else
				ch = 1+1;	/* 0x02, 0x45 */
			if (ringbuf_elements(r) >= 8+ch) {	/* 4Bytes目的地址, 4Bytes源地址, 网络序 */
				state_ret = PAP_RECV_ADDR_OK;
				analys_info->em_addr_offset = ch;
			}
		} else {
			state_ret = PAP_FRAME_ERR;
			lt485_log(("%s(), line-%d, lt_if-%d protoco err %d\n", __func__, __LINE__,
					lt_if, protocol));
		}
		protoco_debug(PAP_RECV_ADDR_OK);
		break;

	case PAP_RECV_ADDR_OK:
		ringbuf_put(r, data);
		if (protocol == AP_PROTOCOL_645_1997 || protocol == AP_PROTOCOL_645_2007) {
			state_ret = PAP_RECV_CTRL_WORD;
		} else if (protocol == AP_PROTOCOL_EDMI) {
			state_ret = PAP_RECV_CTRL_WORD;	/* seq-1 */
		} else {
			state_ret = PAP_FRAME_ERR;
			lt485_log(("%s(), line-%d, lt_if-%d protoco err %d\n", __func__, __LINE__,
					lt_if, protocol));
		}
		protoco_debug(PAP_RECV_CTRL_WORD);
		break;

	case PAP_RECV_CTRL_WORD:
		ringbuf_put(r, data);
		if (protocol == AP_PROTOCOL_645_1997) {
			if (1==data || 2==data || 4==data || 6==data || 8==data) {
				analys_info->data_zone_len = data;
				state_ret = PAP_RECV_DATA_LEN;
			} else {
				state_ret = PAP_FRAME_ERR;
				lt485_log(("%s(), line-%d, lt_if-%d 645-data-zone-len-err:%d\n",
						__func__, __LINE__, lt_if, data));
			}
		} else if (protocol == AP_PROTOCOL_645_2007) {
			if (data < 32) {
				analys_info->data_zone_len = data;
				state_ret = PAP_RECV_DATA_LEN;
			} else {
				state_ret = PAP_FRAME_ERR;
				lt485_log(("%s(), line-%d, lt_if-%d 645-data-zone-len-err:%d\n",
						__func__, __LINE__, lt_if, data));
			}
		} else if (protocol == AP_PROTOCOL_EDMI) {
			state_ret = PAP_RECV_DATA_LEN;	/* seq-2 */
		} else {
			state_ret = PAP_FRAME_ERR;
			lt485_log(("%s(), line-%d, lt_if-%d protoco err %d\n", __func__, __LINE__,
					lt_if, protocol));
		}
		protoco_debug(PAP_RECV_DATA_LEN);
		break;

	case PAP_RECV_DATA_LEN:
		ringbuf_put(r, data);
		if (protocol == AP_PROTOCOL_645_1997 || protocol == AP_PROTOCOL_645_2007) {
			state_ret = PAP_RECV_OTHER_DATA;	/* data zone first byte */
			analys_info->had_recv_data_cnt = 1;
		} else if (protocol == AP_PROTOCOL_EDMI) {
			state_ret = PAP_RECV_OTHER_DATA;	/* ctrl byte */
//			analys_info->had_recv_data_cnt = 0;
		} else {
			state_ret = PAP_FRAME_ERR;
			lt485_log(("%s(), line-%d, lt_if-%d protoco err %d\n", __func__, __LINE__,
					lt_if, protocol));
		}
		protoco_debug(PAP_RECV_OTHER_DATA);
		break;

	case PAP_RECV_OTHER_DATA:
		ringbuf_put(r, data);
		if (protocol == AP_PROTOCOL_645_1997 || protocol == AP_PROTOCOL_645_2007) {
			++analys_info->had_recv_data_cnt;
//			printf_syn("&&&&%s(), %d, %d\n", __func__, analys_info->data_zone_len,
//					analys_info->had_recv_data_cnt);
			if (analys_info->had_recv_data_cnt >= analys_info->data_zone_len+2) {
				if (0x16 == data) {
				state_ret = PAP_FRAME_END_OK;
				} else {
					state_ret = PAP_FRAME_ERR;
					lt485_log(("%s(), line-%d, lt_if-%d protoco err %d\n", __func__, __LINE__,
										lt_if, protocol));
				}
			}
		} else if (protocol == AP_PROTOCOL_EDMI) {
//			++analys_info->had_recv_data_cnt;
			if (0x03 == data)
				state_ret = PAP_FRAME_END_OK;
		} else {
			state_ret = PAP_FRAME_ERR;
			lt485_log(("%s(), line-%d, lt_if-%d protoco err %d\n", __func__, __LINE__,
					lt_if, protocol));
		}

//		sw485_debug_body(("%s(), line:%d, data:0x%x, zone:%d, had-r:%d, state:%d, rstate:%d\n",
//				__func__, __LINE__, data,
//				analys_info->data_zone_len, analys_info->had_recv_data_cnt,
//				state_ret, analys_info->rstate));


		if (state_ret == PAP_FRAME_END_OK) {
			stop_protoc_inter_byte_timer(lt_if);
		}
		protoco_debug(PAP_FRAME_END_OK);
		break;

	default:
		lt485_log(("%s(), line-%d, lt_if-%d state_ret(%d) err\n", __func__, __LINE__,
				lt_if, state_ret));
		state_ret = PAP_FRAME_ERR;
		break;
	}

	analys_info->state = state_ret;

	if (state_ret == PAP_FRAME_ERR) {
		stop_protoc_inter_byte_timer(lt_if);
	}

	return state_ret;
}

/*
 * 数据帧中的地址格式：
 * 	645规约	-- BCD码, A0 A1 A2 A3 A4 A5
 * 	edmi	-- 以网络序存储的4字节整数
 * */
static int get_em_sn_str_from_frame(struct protoc_analy_info_st *analys_info)
{
	enum ammeter_protocol_e protocol = analys_info->protocol;
	enum ammeter_protocol_e proto;
	char grp_no;
	struct ringbuf *r = &analys_info->rb;
	char sn_buf[DEV_SN_BUF_STRING_WITH_NUL_LEN_MAX];
	char frame_sn_buf[8];
	unsigned int tmp;
	int ret = SUCC;

	if (AP_PROTOCOL_645_1997 == protocol || AP_PROTOCOL_645_2007 == protocol) {
		ringbuf_get_slice(r, analys_info->em_addr_offset, frame_sn_buf, 6);
		byte_reverse(frame_sn_buf, 6);
		if (SUCC == convert_bcd2str(frame_sn_buf, 6, sn_buf, sizeof(sn_buf))) {
//			str_reverse(sn_buf);
			del_multi_zero_in_str(sn_buf, 12);

			goto find_sn;
		} else {
			printf_syn("%s, convert_bcd2str fail\n", __func__);
			ret = FAIL;
		}
	} else if (AP_PROTOCOL_EDMI == protocol) {
		ringbuf_get_slice(r, analys_info->em_addr_offset, frame_sn_buf, 4);
		tmp = frame_sn_buf[0]<<24 | frame_sn_buf[1]<<16 | frame_sn_buf[2]<<8 | frame_sn_buf[3];
		ui2str(sn_buf, tmp);

		goto find_sn;
	} else {
		printf_syn("%s(), protoc invalid(%d)\n", __func__, protocol);
		ret = FAIL;
	}

	return ret;

find_sn:
	lt485_debug(("find %s in local reg-em tbl\n", sn_buf));
	if (SUCC == get_reg_em_grpno_proto(sn_buf, &proto, &grp_no)) {
		lt485_debug(("find %s(proto:%d, grp_no:%d) in local reg-em tbl succ\n",
				sn_buf, proto, grp_no));
		if (1 == grp_no) {
			analys_info->lt_if_use_by_em = LT_485_EM_UART_1;
		} else if (2 == grp_no) {
			analys_info->lt_if_use_by_em = LT_485_EM_UART_2;
		} else {
			printf_syn("%s, em's(%s) grp_no invalid(%d)\n", __func__,
					sn_buf, grp_no);
		}
	} else {
		/* 即使收到不是本地的sn号,也是正常情况 */
		ret = FAIL;
	}

	return ret;
}

static int is_send_cmd2local_em(struct protoc_analy_info_st *analys_info)
{
	if (SUCC == get_em_sn_str_from_frame(analys_info)) {
		sw485_debug_body(("%s(), line:%d, lt_if-%d, lt_if_use_by_em:%d\n", __func__, __LINE__,
				analys_info->lt_if, analys_info->lt_if_use_by_em));
		return 1;
	} else {
		lt485_debug(("%s(), line:%d, lt_if-%d, lt_if_use_by_em:%d\n", __func__, __LINE__,
				analys_info->lt_if, analys_info->lt_if_use_by_em));
		return 0;
	}
}

#define DEBUG_IPC_LT485SW 0

#if DEBUG_IPC_LT485SW
#define rt_list_entry(node, type, member) \
    ((type *)((char *)(node) - (unsigned long)(&((type *)0)->member)))

extern rt_list_t rt_thread_priority_table[RT_THREAD_PRIORITY_MAX];
extern rt_uint32_t rt_thread_ready_priority_group;

extern long list_mutex(void);

unsigned int rt_print_list_name(const rt_list_t *l)
{
	struct rt_thread *th;
	unsigned int len;
	const rt_list_t *p;

	len = 0;
	p = l;
	while( p->next != l ) {
		p = p->next;

		th = rt_list_entry(p, struct rt_thread, tlist);
		rt_kprintf("+%d:%10s, %d, %d, %d, %d, %d\n", len, th->name, th->stat,
				th->error, th->init_priority, th->current_priority, th->remaining_tick);

		len++;
	}

	return len;
}

void print_k_info(const rt_list_t *sup_list)
{
	register rt_base_t temp;
	int i;
	rt_thread_t th;

	/* disable interrupt */
	temp = rt_hw_interrupt_disable();

	list_mutex();

	th = rt_thread_self();

	rt_kprintf("\ncur_th info:%10s, %d, %d, %d, %d, %d\n", th->name, th->stat,
			th->error, th->init_priority, th->current_priority, th->remaining_tick);

	rt_kprintf("\nsup_list:\n");
	rt_print_list_name(sup_list);

	rt_kprintf("\nr_grp:%#x\n", rt_thread_ready_priority_group);
	for (i=0; i<32; ++i) {
		if (1<<i & rt_thread_ready_priority_group) {
			rt_kprintf("++%d, read-list:\n", i);
			rt_print_list_name(&rt_thread_priority_table[i]);
		}
	}

	/* enable interrupt */
	rt_hw_interrupt_enable(temp);
}
#endif

/* 必须时软定时器 */
void protoc_inter_byte_timeout(void *param)
{
	enum lt_485if_e rs485_if;
	enum lt_485if_e lt_if_use_by_em;
	char if_had_get_lu_sem;
	struct protoc_analy_info_st *analys_info;
	rt_device_t dev1, dev2;
	rt_err_t err;

	rs485_if = *(enum lt_485if_e *)param;

	analys_info = get_analys_info_ptr(rs485_if);
	if (NULL != analys_info) {
		lt_if_use_by_em = analys_info->lt_if_use_by_em;
		if_had_get_lu_sem = analys_info->if_had_get_lu_sem;

		if (LT_485_EM_UART_1 == lt_if_use_by_em) {
			dev1 = get_us485_dev_ptr(lt_if_use_by_em, 0);
			if (1 == if_had_get_lu_sem) {
				rt_sem_release(&(em_grp1_if_lu_sem));
			} else if (0 == if_had_get_lu_sem) {
				/* nothing */
			} else {
				rt_kprintf("%s(), lt_if_use_by_em(%d, %d) unusual\n", __func__,
						lt_if_use_by_em, if_had_get_lu_sem);
			}

			err = rt_mutex_take(&em_grp1_if_mutex, 0);
			if (RT_EOK != err) {
				rt_kprintf("%s() get mutex fail(%d)\n", __func__, err);
#if DEBUG_IPC_LT485SW
				print_k_info(&em_grp1_if_mutex.parent.suspend_thread);
#endif
			} else {
#if DEBUG_IPC_LT485SW
				if (rt_thread_self() != em_grp1_if_mutex.owner) {
					rt_kprintf("PANIC:%d, %s(), self:%s(%d,%d,%d,%d), owner:%s(%d, %d, %d,%d),%d,%d,%d,err:%d\n",
							 __LINE__, __func__, rt_thread_self()->name,
							 rt_thread_self()->init_priority,
							 rt_thread_self()->current_priority,
							 rt_thread_self()->error,
							 rt_thread_self()->stat,
							em_grp1_if_mutex.owner->name,
							em_grp1_if_mutex.owner->init_priority,
							em_grp1_if_mutex.owner->current_priority,
							em_grp1_if_mutex.owner->error,
							em_grp1_if_mutex.owner->stat,
							em_grp1_if_mutex.hold, em_grp1_if_mutex.original_priority,
							em_grp1_if_mutex.value, err);

					print_k_info(&em_grp1_if_mutex.parent.suspend_thread);
				}
#endif
				dev1->control(dev1, RT_DEVICE_CTRL_CLR_RXBUF, NULL);
				err = rt_mutex_release(&em_grp1_if_mutex);
				if (RT_EOK != err) {
					rt_kprintf("th:%s, line:%d, %s() release mutex fail(%d), owner:%s\n",
							rt_thread_self()->name, __LINE__, __func__, err,
							em_grp1_if_mutex.owner->name);
				}
			}
		} else if (LT_485_EM_UART_2 == lt_if_use_by_em) {
			dev1 = get_us485_dev_ptr(lt_if_use_by_em, 0);
			if (2 == if_had_get_lu_sem) {
				rt_sem_release(&(em_grp2_if_lu_sem));
			} else if (0 == if_had_get_lu_sem) {
				/* nothing */
			} else {
				rt_kprintf("%s(), lt_if_use_by_em(%d, %d) unusual\n", __func__,
						lt_if_use_by_em, if_had_get_lu_sem);
			}

			err = rt_mutex_take(&em_grp2_if_mutex, 0);
			if (RT_EOK != err) {
				rt_kprintf("%s() get mutex fail(%d)\n", __func__, err);
#if DEBUG_IPC_LT485SW
				print_k_info(&em_grp1_if_mutex.parent.suspend_thread);
#endif
			} else {
#if DEBUG_IPC_LT485SW
				if (rt_thread_self() != em_grp2_if_mutex.owner) {
					rt_kprintf("PANIC:%d, %s(), self:%s(%d,%d,%d,%d), owner:%s(%d,%d,%d,%d),%d,%d,%d, err:%d\n",
							 __LINE__, __func__, rt_thread_self()->name,
							 rt_thread_self()->init_priority,
							 rt_thread_self()->current_priority,
							 rt_thread_self()->error,
							 rt_thread_self()->stat,
							em_grp2_if_mutex.owner->name,
							em_grp2_if_mutex.owner->init_priority,
							em_grp2_if_mutex.owner->current_priority,
							em_grp2_if_mutex.owner->error,
							em_grp2_if_mutex.owner->stat,
							em_grp2_if_mutex.hold, em_grp2_if_mutex.original_priority,
							em_grp2_if_mutex.value, err);

					print_k_info(&em_grp2_if_mutex.parent.suspend_thread);
				}
#endif
				dev1->control(dev1, RT_DEVICE_CTRL_CLR_RXBUF, NULL);
				err = rt_mutex_release(&em_grp2_if_mutex);
				if (RT_EOK != err) {
					rt_kprintf("th:%s, line:%d, %s() release mutex fail(%d), owner:%s\n",
							rt_thread_self()->name, __LINE__, __func__, err,
							em_grp2_if_mutex.owner->name);
				}
			}
		} else if (LT_485_PHONY == lt_if_use_by_em) {
			/* 在没有确定使用那个口时timeout */
			/* nothing */
		} else {
			rt_kprintf("ERR(line:%d):%s() lt_if-%d if use by em invalid(%d)\n",
						__LINE__, __func__,
						analys_info->lt_if, lt_if_use_by_em);
		}

		analys_info->if_had_get_lu_sem = 0;

		if (is_tl16_interface(analys_info->lt_if)) {
			clr_bit_em_if_use_state_vector(em_if_use_state_vector, EM_IF_USE_ORDER_BY_TL16);
		}

		rt_kprintf("NOTE:%s() lt_if-%d, %d timeout\n", __func__,
				analys_info->lt_if, lt_if_use_by_em);

		err = rt_mutex_take(&analys_info->emcx_if_mutex, 0);
		if (RT_EOK != err) {
			rt_kprintf("%s() get mutex fail(%d)\n", __func__, err);
		} else {
			dev2 = get_us485_dev_ptr(analys_info->lt_if, 0);
			dev2->control(dev2, RT_DEVICE_CTRL_CLR_RXBUF, NULL);
			err = rt_mutex_release(&analys_info->emcx_if_mutex);
			if (RT_EOK != err) {
				rt_kprintf("th:%s, line:%d, %s() release mutex fail(%d), owner:%s\n",
						rt_thread_self()->name, __LINE__, __func__, err,
						analys_info->emcx_if_mutex.owner->name);
			}
		}

		_reset_protoc_analy_info(analys_info, analys_info->lt_if);
	} else {
		rt_kprintf("NOTE:%s() param error %d\n", __func__, rs485_if);
	}
}

#define lt485_debug_t(x) 	//printf_syn x
#include <rthw.h>

static int start_protoc_inter_byte_timer(enum lt_485if_e lt_if)
{
	if (lt_if<LT_485_TL16_U1 || lt_if>LT_485_EMC_UART) {
		lt485_log(("%s() lt-if(%d) is invalid\n", __func__, lt_if));
		return FAIL;
	}

	/* 只有已启动时，才会失败 */
	if (RT_EOK == rt_timer_start(&protoc_inter_byte_timer[lt_if])) {
		lt485_debug_t(("%s() lt-if-%d, init_tick:%u, timeout_tick:%u, cur_tick:%u\n",
				__func__, lt_if,
				protoc_inter_byte_timer[lt_if].init_tick,
				protoc_inter_byte_timer[lt_if].timeout_tick,
				rt_tick_get()));
	} else {
//		lt485_debug_t(("warning:%s() lt-if-%d\n", __func__, lt_if));
	}

	return SUCC;
}

static int stop_protoc_inter_byte_timer(enum lt_485if_e lt_if)
{
	if (lt_if<LT_485_TL16_U1 || lt_if>LT_485_EMC_UART) {
		lt485_log(("%s() lt-if(%d) is invalid\n", __func__, lt_if));
		return FAIL;
	}

	/* 只有未启动时，才会失败 */
	if (RT_EOK == rt_timer_stop(&protoc_inter_byte_timer[lt_if])) {
		lt485_debug_t(("%s() lt-if-%d, init_tick:%u, timeout_tick:%u, cur_tick:%u\n",
				__func__, lt_if,
				protoc_inter_byte_timer[lt_if].init_tick,
				protoc_inter_byte_timer[lt_if].timeout_tick,
				rt_tick_get()));
	} else {
//		lt485_debug_t(("warning:%s() lt-if-%d\n", __func__, lt_if));
	}

	return SUCC;
}

static int reset_protoc_inter_byte_timer(enum lt_485if_e lt_if)
{
	if (lt_if<LT_485_TL16_U1 || lt_if>LT_485_EMC_UART) {
		lt485_log(("%s() lt-if(%d) is invalid\n", __func__, lt_if));
		return FAIL;
	}

	if(is_bit_set(protoc_inter_byte_timer[lt_if].parent.flag, RT_TIMER_FLAG_ACTIVATED)) {
		rt_timer_stop(&protoc_inter_byte_timer[lt_if]);
		rt_timer_start(&protoc_inter_byte_timer[lt_if]);

		lt485_debug_t(("%s() lt-if-%d, init_tick:%u, timeout_tick:%u, cur_tick:%u\n",
				__func__, lt_if,
				protoc_inter_byte_timer[lt_if].init_tick,
				protoc_inter_byte_timer[lt_if].timeout_tick,
				rt_tick_get()));
	} else {
//		lt485_debug_t(("warning:%s() lt-if-%d\n", __func__, lt_if));
	}

	return SUCC;
}

