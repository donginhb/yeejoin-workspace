/*
 ******************************************************************************
 * ms_common.c
 *
 *  Created on: 2013-12-18
 *      Author: David, zhaoshaowei@yeejoin.com
 *
 * 2014-03-05：
 * 	David: 增加简单的si4432传输层的抽象
 *
 * COPYRIGHT (C) 2013, YEEJOIN (Beijing) Technology Development Co., Ltd.
 ******************************************************************************
 */

#include <rtdef.h>
#include <rtthread.h>
#include <sys_cfg_api.h>

#include <ms_common.h>


#define ms_common_debug(x)	printf_syn x
#define ms_common_log(x)	printf_syn x
#define ms_common_info(x)	printf_syn x

enum si4432_trans_timer_user_id_e{
	STTUID_WAIT_RECV_REQUEST,
	STTUID_WAIT_RECV_REQUEST_REP,

	STTUID_BUTT
};
#if C51_SYNTAX_
typedef base_t trans_timer_user_id_t;
#else
typedef enum si4432_trans_timer_user_id_e trans_timer_user_id_t;
#endif

#define CHECK_WIRELESS_RECV_STATUS_PERIOD get_ticks_of_ms(30)


/*
 * send_data -- 要发送的应用数据
 * 将h以及send_data的数据打包到pkt
 *
 * 这里只是简单拷贝，没有处理字节序
 * */
int package_master_slave_data(uint8_t *pkt, uint8_t pkt_len,
		struct master_slave_data_frame_head_s *h, void *send_data)
{
	int len, i;
	char *pch;

	if (NULL==pkt || NULL==h || (0!=h->len && NULL==send_data)) {
		ms_common_log(("%s() param error\n", __FUNCTION__));
		return FAIL;
	}

	len = 0;
	*pkt++ = h->cmd;
	*pkt++ = h->len;

	switch (h->cmd) {
	/* 主节点主动发起 */
	case MNA_ECHO			:
	case MNA_STOP_ECHO				:
	case MNA_DETECT_SLAVE_NODE			:	/* 探测可链接的从节点 */
	case MNA_DISTRIBUTE_CFG_DATA			:	/* 无线主节点给从节点分发配置数据 */
	case MNA_COLLECTION_DATA                   	:
	case MNA_GET_STATE_INFO                      :
	case MNA_GET_ID_INFO                         :
	/* 从节点主动发起 */
	case MNP_PUSH_COLLECTION_DATA                :	/* 将采集数据定时上报给master节点 */
	case MNP_PUSH_STATE_INFO                     :	/* 在需要时主动上报状态信息, 比如，电池欠压 */
		len = MIN(pkt_len-MASTER_SLAVE_DATA_FREAM_HEAD_LEN, h->len);
		break;
	/* ---------------- 以下为响应命令 --------------- */
	case MNA_ECHO_REP			 :
	case MNA_STOP_ECHO_REP				:
	case MNA_DETECT_SLAVE_NODE_REP		:
	case MNA_DISTRIBUTE_CFG_DATA_REP		:
	case MNA_COLLECTION_DATA_REP               	:
	case MNA_GET_STATE_INFO_REP                  :
	case MNA_GET_ID_INFO_REP                     :
	case MNP_PUSH_COLLECTION_DATA_REP            :
	case MNP_PUSH_STATE_INFO_REP                 :
		*pkt++ = h->result;
		len = MIN(pkt_len-MASTER_SLAVE_DATA_REP_FREAM_HEAD_LEN, h->len);
		break;

	default:
		ms_common_info(("%s() cmd error\n", __FUNCTION__));
		return FAIL;	/* break; */
	}

	pch = send_data;
	for (i=0; i<len; ++i)
		*pkt++ = *pch++;

	return SUCC;
}


/*
 * recv_data -- 接收到的应用数据
 * 将pkt的数据解包到h及recv_data中
 *
 * 这里只是简单拷贝，没有处理字节序
 * */
int unpackage_master_slave_data(uint8_t *pkt, uint8_t pkt_len,
		struct master_slave_data_frame_head_s *h, void *recv_data, const uint8_t recv_len)
{
	int i, len;
	uint8_t *pch;

	if (NULL==pkt || NULL==h || NULL==recv_data) {
		ms_common_log(("%s() param error\n", __FUNCTION__));
		return FAIL;
	}

	h->cmd = *pkt++;
	h->len = *pkt++; /* *recvdata_len = pkt_len-2; */

	switch (h->cmd) {
	/* 主节点主动发起 */
	case MNA_ECHO			:
	case MNA_STOP_ECHO				:
	case MNA_DETECT_SLAVE_NODE			:	/* 探测可链接的从节点 */
	case MNA_DISTRIBUTE_CFG_DATA			:	/* 无线主节点给从节点分发配置数据 */
	case MNA_COLLECTION_DATA                   	:
	case MNA_GET_STATE_INFO                      :
	case MNA_GET_ID_INFO                         :
	/* 从节点主动发起 */
	case MNP_PUSH_COLLECTION_DATA                :	/* 将采集数据定时上报给master节点 */
	case MNP_PUSH_STATE_INFO                     :	/* 在需要时主动上报状态信息, 比如，电池欠压 */
		len = recv_len - MASTER_SLAVE_DATA_FREAM_HEAD_LEN;
		break;
	/* ---------------- 以下为响应命令 --------------- */
	case MNA_ECHO_REP			 :
	case MNA_DETECT_SLAVE_NODE_REP		:
	case MNA_DISTRIBUTE_CFG_DATA_REP		:
	case MNA_COLLECTION_DATA_REP               	:
	case MNA_GET_STATE_INFO_REP                  :
	case MNA_GET_ID_INFO_REP                     :
	case MNP_PUSH_COLLECTION_DATA_REP            :
	case MNP_PUSH_STATE_INFO_REP                 :
	case MNA_STOP_ECHO_REP				:
		h->result = *pkt++;
		len = recv_len - MASTER_SLAVE_DATA_REP_FREAM_HEAD_LEN;
		break;

	default:
		ms_common_info(("%s() cmd(0x%x) error\n", __FUNCTION__, h->cmd));
		return FAIL;	/* break; */
	}

	if (len < h->len)
		goto buf_too_small;
	else
		len = h->len;

	pch = recv_data;
	for (i=0; i<len; ++i)
		*pch++ = *pkt++;

	return SUCC;

buf_too_small:
	ms_common_info(("%s() recv buf too samll, cmd(%d), recv len %d\n", __FUNCTION__, h->cmd, recv_len));
	return FAIL;
}

int ms_prepare_into_ezmacpro_recv_state(void)
{
#if 1	/* mark by zp */
	if (is_ezmac_in_recv_state())
		return SUCC;
	
	if ((!is_ezmac_in_idle_state()) && (WEIS_OK!=wait_emmacpro_into_state(EZMACS_IDLE, 100))) {
		ms_common_log(("func:%s(), line:%d, wait into idle fail\n", __FUNCTION__, __LINE__));
		return FAIL;
	}
	
	return recv_pkt_from_ezmacpro_prepare();
#else
	set_ezmac_into_idle_state();
	
	if ((!is_ezmac_in_idle_state()) && (WEIS_OK!=wait_emmacpro_into_state(EZMACS_IDLE, 100))) {
		ms_common_log(("func:%s(), line:%d, wait into idle fail\n", __FUNCTION__, __LINE__));
		return FAIL;
	}
	
	return recv_pkt_from_ezmacpro_prepare();
#endif
}


#if !defined(RT_USING_LWIP) && (BYTE_ORDER == LITTLE_ENDIAN)

/**
 * Convert an u16_t from host- to network byte order.
 *
 * @param n u16_t in host byte order
 * @return n in network byte order
 */
u16_t
ms_htons(u16_t n)
{
	return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}

/**
 * Convert an u16_t from network- to host byte order.
 *
 * @param n u16_t in network byte order
 * @return n in host byte order
 */
u16_t
ms_ntohs(u16_t n)
{
	return ms_htons(n);
}

/**
 * Convert an u32_t from host- to network byte order.
 *
 * @param n u32_t in host byte order
 * @return n in network byte order
 */
u32_t
ms_htonl(u32_t n)
{
	return ((n & 0xff) << 24) |
		   ((n & 0xff00) << 8) |
		   ((n & 0xff0000UL) >> 8) |
		   ((n & 0xff000000UL) >> 24);
}

/**
 * Convert an u32_t from network- to host byte order.
 *
 * @param n u32_t in network byte order
 * @return n in host byte order
 */
u32_t
ms_ntohl(u32_t n)
{
	return ms_htonl(n);
}

#endif /* (LWIP_PLATFORM_BYTESWAP == 0) && (BYTE_ORDER == LITTLE_ENDIAN) */




/*
 * 2014-03-05, zhaoshaowei@yeejoin.com
 *
 * 由于编写si4432代码之初，没有设想si4432会有复杂的应用，light300项目进行到现在，显现出了si4432应用的复杂性，因此，下面将对基于si4432的
 * 通信层进行抽象
 *
 *
 * */
static si4432_transport_fsm_t si4432_trans_fsm_state;
static struct rt_semaphore si4432_fsm_sem;
static struct rt_timer si4432_trans_timer;
static volatile base_t is_si4432_trans_timeout;
rt_mailbox_t si4432_recv_mb;/* mark by zp */

#define SI4432_WAIT_MULT_REQUEST_REP_TIMEOUT    (5 * 1000)/* mark by zp */
#define SI4432_WAIT_REQUEST_REP_TIMEOUT_MS_MAX	(1 * 1000)
#define SI4432_WAIT_REQUEST_TIMEOUT_MS_MAX	(2 * 1000)

static void si4432_trans_timeout(void* parameter);
static rt_err_t start_si4432_trans_timer(ubase_t timeout_ms, trans_timer_user_id_t user_id);
static si4432_transport_err_t si4432_trans_senddata(struct si4432_trans_data_info *data_info);


si4432_transport_err_t si4432_transport_data(si4432_transport_data_cmd_t cmd, struct si4432_trans_data_info *data_info)
{
	rt_err_t err;
	si4432_transport_err_t ret = STE_OK;
	rt_uint32_t temp;
	static rt_uint8_t is_timer_start = 0;
	int ret_err;

	if (NULL==data_info || NULL==data_info->databuf.tx_buf) {
		ms_common_info(("func:%s() param error\n", __FUNCTION__));
		return STE_PTR_NULL;
	}

	if (RT_EOK != (err=rt_sem_take(&si4432_fsm_sem, RT_WAITING_FOREVER))) {
		ms_common_log(("take si4432-fsm-sem error(%d)\n", err));

		/* error process ???? */
		return STE_FAIL;
	}

	if (STF_IDLE == si4432_trans_fsm_state) {
		switch (cmd) {
		case STDC_SEND_REQUEST:	/* 发送请求 */
			si4432_trans_fsm_state = STF_SEND_REQUEST;
			break;

		case STDC_WAIT_REQUEST_REP:	/* 等待请求响应 */
		case STDC_WAIT_MULTI_REQUEST_REP:	/* 等待多个请求响应 */
			si4432_trans_fsm_state = STF_WAIT_REQUEST_REP;
			break;

		case STDC_WAIT_REQUEST:	/* 等待请求 */
			si4432_trans_fsm_state = STF_WAIT_REQUEST;
			break;

		case STDC_SEND_REQUEST_REP:	/* 发送请求响应 */
			si4432_trans_fsm_state = STF_SEND_REQUEST_REP;
			break;

		default:
			ms_common_log(("func:%s(), line:%d, cmd(%d) error\n", __FUNCTION__, __LINE__, cmd));
			goto cmd_state_not_match;
		}
	} else {
		switch (si4432_trans_fsm_state) {
		case STF_SEND_REQUEST:	/* 该无线节点主动向外发送请求 */
			if (STDC_SEND_REQUEST != cmd) {
				ms_common_log(("func:%s(), line:%d, cmd(%d) error\n", __FUNCTION__, __LINE__, cmd));
				goto cmd_state_not_match;
			}
			break;

		case STF_WAIT_REQUEST_REP:	/* 该无线节点等待某无线节点对自己请求的响应 */
			if (STDC_WAIT_REQUEST_REP!=cmd && STDC_WAIT_MULTI_REQUEST_REP!=cmd) {
				ms_common_log(("func:%s(), line:%d, cmd(%d) error\n", __FUNCTION__, __LINE__, cmd));
				goto cmd_state_not_match;
			}
			break;

		case STF_WAIT_REQUEST:	/* 该无线节点等待某无线节点发送的请求 */
			if (STDC_WAIT_REQUEST != cmd) {
				ms_common_log(("func:%s(), line:%d, cmd(%d) error\n", __FUNCTION__, __LINE__, cmd));
				goto cmd_state_not_match;
			}
			break;

		case STF_SEND_REQUEST_REP:	/* 该无线节点发送请求的响应 */
			if (STDC_SEND_REQUEST_REP != cmd) {
				ms_common_log(("func:%s(), line:%d, cmd(%d) error\n", __FUNCTION__, __LINE__, cmd));
				goto cmd_state_not_match;
			}
			break;

		default:
			ms_common_log(("func:%s(), line:%d, cmd(%d) error\n", __FUNCTION__, __LINE__, cmd));
			goto cmd_state_not_match;
		}
	}

	switch (si4432_trans_fsm_state) {
	case STF_SEND_REQUEST:	/* 该无线节点主动向外发送请求 */
		ret = si4432_trans_senddata(data_info);
		if (STE_OK != ret)
			goto err_ret;

		recv_pkt_from_ezmacpro_prepare();
		si4432_trans_fsm_state = STF_WAIT_REQUEST_REP;
		break;		/* not falls through */

	case STF_WAIT_REQUEST_REP:/* mark by zp *//* 该无线节点等待某无线节点对自己请求的响应 */
		ret_err = is_ezmac_in_recv_state();
		if (-1 == ret_err) {
			ret = STE_RX_ERROR_STATE;
			goto err_ret;
		} else if (0 == ret_err) {
			recv_pkt_from_ezmacpro_prepare();
		}

		if (cmd == STDC_WAIT_MULTI_REQUEST_REP) {
			if (!is_timer_start) {
				is_timer_start = 1;
				start_si4432_trans_timer(SI4432_WAIT_MULT_REQUEST_REP_TIMEOUT, STTUID_WAIT_RECV_REQUEST_REP);
			}
		} else if (cmd == STDC_WAIT_REQUEST_REP) {
			if (!is_timer_start) {
				is_timer_start = 1;
				start_si4432_trans_timer(SI4432_WAIT_REQUEST_REP_TIMEOUT_MS_MAX, STTUID_WAIT_RECV_REQUEST_REP);
			}
		}

		err = rt_mb_recv(si4432_recv_mb, &temp, CHECK_WIRELESS_RECV_STATUS_PERIOD);
		if (RT_EOK == err) {
			if (SRTE_RECEIVED == temp) {
				if (TRUE == is_ezmac_had_recv_pkt()) {
					data_info->ms_id.sid = get_pkt_sid();
					data_info->ms_id.did = get_pkt_did();
					recv_pkt_from_ezmacpro(data_info->databuf.rx_buf, data_info->buflen,
								&data_info->datalen, &data_info->rssi);
				} else {
					printf_syn("%s(), line:%d, ezmac not recv pkt, but mb is valid\n",
							__FUNCTION__, __LINE__);
					ret = STE_FAIL;
				}

				if (cmd == STDC_WAIT_MULTI_REQUEST_REP) {
					ret = STE_PENDING;
					goto release_ret;
				} else {
					rt_timer_stop(&si4432_trans_timer);
					si4432_trans_fsm_state = STF_IDLE;
					is_timer_start = 0;
					break;
				}

			} else if (SRTE_TIMEOUT == temp) {
				ret = STE_WAIT_REP_TIMEOUT;
				is_timer_start = 0;
				goto err_ret;
			}
		} else if (-RT_ETIMEOUT == err) {
			ret = STE_CHECK_RECV_STATUS;
			goto release_ret;
		} else {
			ret = STE_FAIL;
			ms_common_info(("func:%s,line:%d recv si4432_recv_mb error\n"));
			is_timer_start = 0;
			goto err_ret;
		}

		break;

	case STF_WAIT_REQUEST:	/* mark by zp *//* 该无线节点等待某无线节点发送的请求 */
		ret_err = is_ezmac_in_recv_state();
		if (-1 == ret_err) {
			ret = STE_RX_ERROR_STATE;
			goto err_ret;
		} else if (0 == ret_err) {
			recv_pkt_from_ezmacpro_prepare();
		}

		//start_si4432_trans_timer(SI4432_WAIT_REQUEST_TIMEOUT_MS_MAX, STTUID_WAIT_RECV_REQUEST);
		err = rt_mb_recv(si4432_recv_mb, &temp, CHECK_WIRELESS_RECV_STATUS_PERIOD);
		if (RT_EOK == err) {
			if (SRTE_RECEIVED == temp) {
				if (TRUE == is_ezmac_had_recv_pkt()) {
					rt_timer_stop(&si4432_trans_timer);
					data_info->ms_id.sid = get_pkt_sid();
					data_info->ms_id.did = get_pkt_did();
					recv_pkt_from_ezmacpro(data_info->databuf.rx_buf, data_info->buflen,
							&data_info->datalen, &data_info->rssi);
					si4432_trans_fsm_state = STF_SEND_REQUEST_REP;//si4432_trans_fsm_state = STF_IDLE;
				} else {
					printf_syn("%s(), line:%d, ezmac not recv pkt, but mb is valid\n",
							__FUNCTION__, __LINE__);
					ret = STE_FAIL;
					si4432_trans_fsm_state = STF_IDLE;
				}
			} else if (SRTE_TIMEOUT == temp) {
				ret = STE_WAIT_REQUEST_TIMEOUT;
				si4432_trans_fsm_state = STF_IDLE;
			}
		} else if (-RT_ETIMEOUT == err) {
			ret = STE_CHECK_RECV_STATUS;
			goto release_ret;
		} else {
			ret = STE_FAIL;
			ms_common_info(("func:%s,line:%d recv si4432_recv_mb error\n"));
			goto err_ret;
		}
		break;

	case STF_SEND_REQUEST_REP:	/* 该无线节点发送请求的响应 */
		ret = si4432_trans_senddata(data_info);
		if (STE_OK != ret)
			goto err_ret;

		recv_pkt_from_ezmacpro_prepare();
		si4432_trans_fsm_state = STF_IDLE;
		break;

	default:
		ms_common_log(("func:%s(), line:%d, state(%d)-cmd(%d) error\n",
				__FUNCTION__, __LINE__, si4432_trans_fsm_state, cmd));
		ret = STE_STATE_CMD_NOT_MATCH;
		break;
	}

	goto release_ret;

//set_trans_idle_and_realease:
//	if (!is_ezmac_in_recv_state())/* mark by zp */
//		recv_pkt_from_ezmacpro_prepare();
err_ret:
	si4432_trans_fsm_state = STF_IDLE;
	goto release_ret;

cmd_state_not_match:
	si4432_trans_fsm_state = STF_IDLE;/* mark by zp */
	ret = STE_STATE_CMD_NOT_MATCH;

release_ret:
	rt_sem_release(&si4432_fsm_sem);

	return ret;
}


base_t init_si4432_transport_fsm(void)
{
	si4432_trans_fsm_state = STF_IDLE;
	rt_sem_init(&si4432_fsm_sem, "4432fsm", 1, RT_IPC_FLAG_PRIO);

	return MS_COMMON_SUCC;
}

void _init_si4432_transport_fsm(void)
{
	si4432_trans_fsm_state = STF_IDLE;
}


/* 定时器2超时函数 */
static void si4432_trans_timeout(void* parameter)
{
	trans_timer_user_id_t *p;
	rt_err_t err;

	is_si4432_trans_timeout = TRUE;
	
	p = parameter;
	switch (*p) {
	case STTUID_WAIT_RECV_REQUEST:
		break;

	case STTUID_WAIT_RECV_REQUEST_REP:
		ms_common_info(("func:%s(), wait request response timeout\n", __FUNCTION__));
		break;

	default:
		ms_common_log(("func:%s(), timeout param(%d) error\n", __FUNCTION__, *p));
		break;
	}

	err = rt_mb_send(si4432_recv_mb, SRTE_TIMEOUT);
	if (RT_EOK != err) {/* mark by zp */
		printf_syn("send si4432_recv_mb timeout-msg fail(%d)\n", err);
	}
	
	return;
}


static rt_err_t start_si4432_trans_timer(ubase_t timeout_ms, trans_timer_user_id_t user_id)
{
	static trans_timer_user_id_t trans_timer_user_id;

	is_si4432_trans_timeout = FALSE;

	trans_timer_user_id = user_id;

	rt_timer_init(&si4432_trans_timer, "4432tran",   /* 定时器名字 */
			si4432_trans_timeout, /* 超时时回调的处理函数 */
				  &trans_timer_user_id, /* 超时函数的入口参数 */
				  get_ticks_of_ms(timeout_ms), /* 定时长度为x个OS Tick */
				  RT_TIMER_FLAG_ONE_SHOT); /* 单次定时器 */

	return rt_timer_start(&si4432_trans_timer);
}

#if 0
static void do_something_when_wait_trans_timeout(void)
{
	rt_thread_delay(get_ticks_of_ms(20));

	return;
}
#endif


static si4432_transport_err_t si4432_trans_senddata(struct si4432_trans_data_info *data_info)
{
	struct ezmacpro_pkt_header_t mac_pkt_h;
	si4432_transport_err_t ret = STE_OK;
	wait_emmacpro_into_state_err_t wait_ret;

	/* 等待可以避免信道忙时, 请求发送 */
	if ((!is_ezmac_in_idle_state()) && (WEIS_OK!=wait_emmacpro_into_state(EZMACS_IDLE, 500)) ) {
		ms_common_info(("func:%s(), line:%d, wait into idle fail\n", __FUNCTION__, __LINE__));
		if (SUCC != set_ezmac_into_idle_state()) {
			ms_common_info(("func:%s(), line:%d, set ezmac into idle fail\n", __FUNCTION__, __LINE__));
			ret = STE_EZMAC_BUSY;
			goto ret_entry;
		}
	}

	mac_pkt_h.dst_id = data_info->ms_id.did;
	/* SPEFB_FLAG_NONE, SPEFB_FORCE_SEND */
	sent_pkt_to_ezmacpro(&mac_pkt_h, data_info->databuf.tx_buf, data_info->datalen, SPEFB_FORCE_SEND);

	wait_ret = wait_emmacpro_into_state(EZMACS_PKT_SENT, 800);
	if (WEIS_OK != wait_ret) {
		ms_common_info(("func:%s(), line:%d, wait pkt sent over fail(%d)\n",
				__FUNCTION__, __LINE__, wait_ret));
		if (WEIS_TX_ERROR_STATE == wait_ret) {
			ret = STE_TX_ERROR_STATE;
		} else if (WEIS_TX_CHANNEL_BUSY == wait_ret) {
			ret = STE_TX_CHANNEL_BUSY;
		} else {
			ret = STE_SEND_FAIL;
		}
		goto ret_entry;
	}

ret_entry:
	return ret;
}

int init_si4432_trans_data_info_st(struct si4432_trans_data_info *data_info)
{
	if (NULL == data_info) {
		ms_common_info(("func:%s() param error\n", __FUNCTION__));
		return FAIL;
	}

	rt_memset(data_info, 0, sizeof(*data_info));

	return SUCC;

}

int init_master_slave_frame_head(struct master_slave_data_frame_head_s *h)
{
	if (NULL == h) {
		ms_common_info(("func:%s() param error\n", __FUNCTION__));
		return FAIL;
	}

	rt_memset(h, 0, sizeof(*h));

	return SUCC;

}

int get_self_sn(uint8_t *sn, base_t len)
{
	if (NULL == sn) {
		ms_common_info(("func:%s() param error\n", __FUNCTION__));
		return FAIL;
	}

	return get_devsn((char *)sn, len);
}

base_t is_sn_same_as_self_sn(uint8_t *sn)
{
	uint8_t self_sn[DEV_SN_MODE_LEN];
	base_t i;

	if (NULL == sn) {
		ms_common_info(("func:%s() param error\n", __FUNCTION__));
		return FALSE;
	}

	get_self_sn(self_sn, sizeof(self_sn));

	for (i=0; i<DEV_SN_MODE_LEN; ++i)
		if (self_sn[i] != *sn++)
			break;

	if (i < DEV_SN_MODE_LEN)
		return FALSE;
	else
		return TRUE;
}


base_t get_wireless_cfgdata(struct msfd_distribute_cfgdata_st *p)
{
	uint8_t self_sn[DEV_SN_MODE_LEN];
	struct tinywireless_if_info_st	 tw_info;

	if (NULL == p) {
		ms_common_info(("func:%s() param error\n", __FUNCTION__));
		return FAIL;
	}

	if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_TW_INFO, 0, &tw_info)) {
		printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
		return FAIL;
	}

	p->channel_no	= tw_info.channel_no;
	p->cid		= tw_info.cid;
	p->slave_id	= tw_info.slave_id;
	p->syn_word[0]	= (tw_info.sync_word>>8) & 0xff;
	p->syn_word[1]	= (tw_info.sync_word) & 0xff;

	get_self_sn(self_sn, sizeof(self_sn));
	rt_memcpy(p->sn, self_sn, sizeof(p->sn));

	return SUCC;
}

#if ENABLE_RAND /* mark by zp */
extern u16 Get_ADCValue(void);
extern int rand_r(unsigned int* seed);
unsigned char get_rand_num4delay(void)
{
	unsigned int temp;

	temp = Get_ADCValue() + rt_tick_get();
	return (unsigned char)rand_r(&temp);
}

#ifdef RT_USING_FINSH
#include <finsh.h>
void get_adc_value(void)
{
	ms_common_debug(("adc value is %d.\n",get_rand_num4delay()));
}
FINSH_FUNCTION_EXPORT(get_adc_value, "get adc value")
#endif
#endif
