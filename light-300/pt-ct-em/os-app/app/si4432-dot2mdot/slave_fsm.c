/*
 * slave_fsm.c
 *
 *  Created on: 2013-11-27
 *      Author: David, zhaoshaowei@yeejoin.com
 */


#include <stack.h>
#include <EZMacPro.h>

#include <si4432_hardif.h>
#include <slave.h>
#include <slave_fsm.h>
#include <slave_app.h>
#include <ms_common.h>

#include <syscfgdata.h>
#include <sys_cfg_api.h>
#include <ade7880_api.h>
#include <board.h>

#define NODE_TYPE_NAME_STR	"slave node"

#define SLAVE_FSM_DEBUG_CODE	1

#define slave_fsm_log(x)	printf_syn x
#define slave_fsm_debug(x) 	printf_syn x
#define slave_fsm_info(x) 	printf_syn x


/* 应用层状态机的状态变量 */
static master_slave_cmd_t slave_cmd;

uint8_t *slave_tx_buf;
uint8_t *slave_rx_buf;
uint8_t slave_master_echo_cnt;

static int slave_ezmacpro_init(void);

#if 1

//static ms_result_state_t slave_recv_echo(uint8_t *rx_buf, struct slave_app_fsm_info_st *sfsm_info);
//static ms_result_state_t slave_send_echo_rep(uint8_t *tx_buf, struct slave_app_fsm_info_st *sfsm_info);
static base_t init_slave_node_fsm(struct slave_fsm_state_st *state);

static ms_result_state_t slave_recv_request_before_have_valid_id(uint8_t *rx_buf,
		struct slave_app_fsm_info_st *sfsm_info);
static ms_result_state_t slave_send_response_before_have_valid_id(uint8_t *tx_buf,
		struct slave_app_fsm_info_st *sfsm_info);
static base_t record_wireless_cfgdata(struct msfd_distribute_cfgdata_st *p);

static ms_result_state_t slave_recv_collect_rep(uint8_t *rx_buf, struct slave_app_fsm_info_st *sfsm_info);/* mark by zp */
static ms_result_state_t slave_send_collect_data(uint8_t *tx_buf, struct slave_app_fsm_info_st *sfsm_info);
static void proc_cfg_data_for_slave(struct msfd_distribute_cfgdata_st *p);
static void set_slave_node_rfparam(struct msfd_distribute_cfgdata_st *p);

extern rt_timer_t sscd_timer;
extern rt_sem_t sscd_sem;

rt_uint8_t is_stop_echo = 0;

struct msfd_distribute_cfgdata_st dist_cfg;

static void reset_si4432(void)
{
	while (SUCC != slave_ezmacpro_init()) {
		slave_fsm_log(("reset si4432 fail\n"));
	}
	_init_si4432_transport_fsm();
}

extern void reset_system(void);
static void _reset_system(void)
{
	reset_system();
}

int slave_node_app_fsm(struct slave_app_fsm_info_st *sfsm_info)
{
	sfsm_state_t slave_node_fsm_state;
	slave_next_state_t slave_next_state;
	uint8_t *tx_pch, *rx_pch;
	ms_result_state_t ms_result;

	if (NULL==sfsm_info || NULL==sfsm_info->tx_buf || NULL==sfsm_info->rx_buf) {
		slave_fsm_info(("func:%s(), param error\n", __FUNCTION__));
		return FAIL;
	}

	tx_pch = sfsm_info->tx_buf;
	rx_pch = sfsm_info->rx_buf;

	slave_node_fsm_state	= sfsm_info->s_state.slave_node_fsm_state;
	slave_next_state	= sfsm_info->s_state.slave_next_state;

	switch (slave_node_fsm_state) {
	case SFS_NONE              :
		break;

	case SFS_IDLE              :
		switch (slave_next_state) {
		case SNS_IDLE:
			break;

		case SNS_SLEEP:
			break;
#if 0
		case SNS_ECHO_MODE:
			if (SUCC != ms_prepare_into_ezmacpro_recv_state()) {/* mark by zp */
				slave_fsm_info(("func:%s(), line:%d, state change fail\n", __FUNCTION__, __LINE__));
				break;
			}
			slave_node_fsm_state = SFS_WAIT_ECHO_REQUEST;
			break;
#endif

		case SNS_RECV_FILE_MODE:
			break;

		case SNS_PUSH_CMD_MODE:
			slave_node_fsm_state = SFS_SEND_PUSH_CMD;
			break;

		default:
			break;
		}
		break;

	case SFS_INTO_IDLE_STATE   :
		if (SUCC == set_ezmac_into_idle_state()) {
			slave_node_fsm_state = SFS_IDLE;
		} else {
			slave_fsm_log(("func:%s(), line:%d, into idle-state fail\n", __FUNCTION__, __LINE__));
		}
		break;

	case SFS_WAIT_PROBE_SLAVE_NODE:	/* 等待探测可链接从节点命令 */
	case SFS_WAIT_CFGDATA2SLAVE_NODE:	/* 等待配置数据给从节点命令 */
		ms_result = slave_recv_request_before_have_valid_id(rx_pch, sfsm_info);
		if (MRS_RESULT_DETECT == ms_result) {
			slave_node_fsm_state = SFS_SEND_PROBE_SLAVE_NODE_REP;
			if (SUCC != set_ezmac_into_idle_state()) {/* mark by zp */
				slave_fsm_log(("func:%s(),line:%d,into idle fail.\n",__FUNCTION__,__LINE__));
			}
		} else if (MRS_RESULT_DISTRIBUTE == ms_result) {
			slave_node_fsm_state = SFS_SEND_CFGDATA2SLAVE_NODE_REP;
			if (SUCC != set_ezmac_into_idle_state()) {/* mark by zp */
				slave_fsm_log(("func:%s(),line:%d,into idle fail.\n",__FUNCTION__,__LINE__));
			}
		}
		break;

	case SFS_SEND_PROBE_SLAVE_NODE_REP:	/* 发送探测可链接从节点命令的响应 */
		rt_thread_delay(get_rand_num4delay() + 5);/* mark by zp */
	case SFS_SEND_CFGDATA2SLAVE_NODE_REP:	/* 发送配置数据命令响应 */
		rt_thread_delay(5);/* mark by zp */
		ms_result = slave_send_response_before_have_valid_id(tx_pch, sfsm_info);
		/* 无论是否成功, 均要切换状态 */
		if (SFS_SEND_CFGDATA2SLAVE_NODE_REP == slave_node_fsm_state) {
			if (MRS_RESULT_SUCC == ms_result) {
				/* 以新参数重新设置无线 */

				/* ....... */

				slave_fsm_info(("func:%s,line:%d rep succ\n",__FUNCTION__,__LINE__));
				//slave_node_fsm_state	= SFS_IDLE;
				//slave_next_state	= SNS_IDLE;
				
				set_slave_node_rfparam(&dist_cfg);
				
				slave_node_fsm_state	= SFS_IDLE;
				slave_next_state	= SNS_PUSH_CMD_MODE;
				rt_timer_start(sscd_timer);
			} else {
				/* 如果失败需要重新发送, 这里将不修改状态 */
			}
		} else {
			if (MRS_RESULT_SUCC == ms_result) {
				slave_node_fsm_state = SFS_WAIT_PROBE_SLAVE_NODE;/* mark by zp */
			} else {
				slave_fsm_info(("func:%s,line:%d send probe rep fail\n",__FUNCTION__,__LINE__));
			}
		}
#if 0 != SLAVE_FSM_DEBUG_CODE /* debug */
		if (MRS_RESULT_SUCC != ms_result)
			printf_syn("func:%s(), line:%d, recv response fail(%d)\n", __FUNCTION__, __LINE__, ms_result);
#endif
		break;

	case SFS_SEND_PUSH_CMD:/* mark by zp */
		if (RT_EOK == rt_sem_take(sscd_sem, RT_WAITING_FOREVER)) {
			rt_thread_delay(get_rand_num4delay());/* mark by zp */			
			ms_result = slave_send_collect_data(tx_pch, sfsm_info);
			if (MRS_RESULT_SUCC == ms_result) {
				//EZMacPRO_Reg_Write(PFCR, 0xA0);/* enable address filter */
				slave_node_fsm_state = SFS_WAIT_PUSH_CMD_REP;
				slave_fsm_info(("send collect data succ\n"));
			} else {
				slave_fsm_info(("func:%s,line:%d,send collect data error\n",__FUNCTION__,__LINE__));
				if (SUCC != set_ezmac_into_idle_state()) {
					slave_fsm_info(("func:%s,line:%d,goto idle fail\n",__FUNCTION__,__LINE__));
				}
				rt_timer_start(sscd_timer);
			}
		} else {
			slave_fsm_info(("func:%s,line:%d take sem error\n",__FUNCTION__,__LINE__));
		}
		break;

	case SFS_WAIT_PUSH_CMD_REP:/* mark by zp */
		ms_result = slave_recv_collect_rep(rx_pch,sfsm_info);
		if (MRS_RESULT_SUCC == ms_result) {
			slave_node_fsm_state = SFS_IDLE;
			slave_fsm_info(("recv collect data rep succ\n"));
			rt_timer_start(sscd_timer);
#if 0
		} else if (MRS_RESULT_START_ECHO == ms_result) {
			slave_node_fsm_state = SFS_IDLE;
			slave_next_state = SNS_ECHO_MODE;
#endif
		} else if (MRS_RESULT_FAIL == ms_result) {
			/* change state */
			slave_node_fsm_state = SFS_IDLE;
			slave_fsm_info(("func:%s,line:%d recv collect rep error\n",__FUNCTION__,__LINE__));
			rt_timer_start(sscd_timer);
		}
		break;	
#if 0
	case SFS_WAIT_ECHO_REQUEST:
		ms_result = slave_recv_echo(rx_pch, sfsm_info);
		if (MRS_RESULT_SUCC == ms_result) {
			is_stop_echo = 0;
			slave_node_fsm_state = SFS_SEND_ECHO_RESPONSE;
		} else if (MRS_RESULT_STOP_ECHO == ms_result) {
			is_stop_echo = 1;
			slave_node_fsm_state = SFS_SEND_ECHO_RESPONSE;
		} else {
			is_stop_echo = 0;
			slave_node_fsm_state = SFS_IDLE;
			/* break; */
		}
		break; /* not falls through */

	case SFS_SEND_ECHO_RESPONSE:
		rt_thread_delay(5);/* mark by zp */
		slave_send_echo_rep(tx_pch, sfsm_info);
		if (is_stop_echo) {
			is_stop_echo = 0;
			slave_node_fsm_state = SFS_IDLE;
			slave_next_state = SNS_PUSH_CMD_MODE;
			rt_timer_start(sscd_timer);
		} else {
			slave_node_fsm_state = SFS_IDLE;
		}
		break;
#endif
	default:
		break;
	}

	sfsm_info->s_state.slave_node_fsm_state = slave_node_fsm_state;
	sfsm_info->s_state.slave_next_state	= slave_next_state;

	return SUCC;
}

/*
 * NOTE: 特例,在函数中会修改状态变量
 * */
static ms_result_state_t slave_recv_request_before_have_valid_id(uint8_t *rx_buf,
		struct slave_app_fsm_info_st *sfsm_info)
{
	ms_result_state_t ret = MRS_RESULT_SUCC;
	struct si4432_trans_data_info data_info;
	struct master_slave_data_frame_head_s	h;
	si4432_transport_err_t trans_err;

	if (NULL==rx_buf || NULL==sfsm_info) {
		slave_fsm_info(("func:%s(), param error\n", __FUNCTION__));
		return MRS_RESULT_FAIL;
	}

	init_si4432_trans_data_info_st(&data_info);
	data_info.databuf.rx_buf = rx_buf;
	data_info.buflen	 = RF_RX_BUFFER_SIZE;
	trans_err = si4432_transport_data(STDC_WAIT_REQUEST, &data_info);
	if (STE_OK == trans_err) {
		unpackage_master_slave_data(data_info.databuf.rx_buf, data_info.buflen, &h,
				sfsm_info->buf.unpkt_buf, sizeof(sfsm_info->buf.unpkt_buf));

		if ((MNA_DETECT_SLAVE_NODE == h.cmd)) {
			sfsm_info->s_state.slave_node_fsm_state = SFS_WAIT_PROBE_SLAVE_NODE;

			slave_fsm_info(("+*+--> recv probe-cmd from master node:"));
			print_dev_sn(sfsm_info->buf.unpkt_buf, h.len);
			ret = MRS_RESULT_DETECT;
		} else if (MNA_DISTRIBUTE_CFG_DATA == h.cmd) {
			sfsm_info->s_state.slave_node_fsm_state = SFS_WAIT_CFGDATA2SLAVE_NODE;

			if (TRUE == is_sn_same_as_self_sn(sfsm_info->buf.unpkt_buf)) {
				record_wireless_cfgdata((struct msfd_distribute_cfgdata_st *)sfsm_info->buf.unpkt_buf);
				/* mark by zp recoard data */
				proc_cfg_data_for_slave((struct msfd_distribute_cfgdata_st *)sfsm_info->buf.unpkt_buf);
				ret = MRS_RESULT_DISTRIBUTE;
			} else {
				ret = MRS_RESULT_FAIL;
			}
		} else {
			slave_fsm_log(("func:%s(), line:%d, cmd:%d\n", __FUNCTION__, __LINE__, h.cmd));
			ret = MRS_RESULT_FAIL;
		}
	} else if (STE_TX_ERROR_STATE == trans_err) {
		reset_si4432();
		ret = MRS_RESULT_FAIL;
	} else if (STE_WAIT_REP_TIMEOUT == trans_err) {
		ret = MRS_RESULT_FAIL;
		slave_fsm_debug(("func:%s(), line:%d, si4432_transport_data() fail(%d)\n",
				__FUNCTION__, __LINE__, trans_err));
	} else if (STE_RX_ERROR_STATE == trans_err) {
		reset_si4432();
		ret = MRS_RESULT_PENDIND;
		slave_fsm_debug(("func:%s(), line:%d, mac state not match si4432 state fail(%d)\n",
				__FUNCTION__, __LINE__, trans_err));
	} else if (STE_CHECK_RECV_STATUS == trans_err) {
		ret = MRS_RESULT_PENDIND;
	}

	return ret;
}


static ms_result_state_t slave_send_response_before_have_valid_id(uint8_t *tx_buf,
		struct slave_app_fsm_info_st *sfsm_info)
{
	ms_result_state_t ret = MRS_RESULT_SUCC;

	struct si4432_trans_data_info 		data_info;
	struct master_slave_data_frame_head_s	h;
	si4432_transport_err_t 			trans_err;

	uint8_t sn[DEV_SN_MODE_LEN];

	if (NULL==tx_buf || NULL==sfsm_info) {
		slave_fsm_info(("func:%s(), param error\n", __FUNCTION__));
		return MRS_RESULT_FAIL;
	}

	init_master_slave_frame_head(&h);

	if (SFS_SEND_PROBE_SLAVE_NODE_REP == sfsm_info->s_state.slave_node_fsm_state) {
		get_self_sn(sn, sizeof(sn));
		h.cmd	= MNA_DETECT_SLAVE_NODE_REP;
		h.len	= sizeof(sn);
		package_master_slave_data(tx_buf, RF_TX_BUFFER_SIZE, &h, sn);
	} else if (SFS_SEND_CFGDATA2SLAVE_NODE_REP == sfsm_info->s_state.slave_node_fsm_state) {
		h.cmd	= MNA_DISTRIBUTE_CFG_DATA_REP;
		h.len	= 0;
		package_master_slave_data(tx_buf, RF_TX_BUFFER_SIZE, &h, NULL);
	} else {
		slave_fsm_info(("func:%s(), line:%d, state(%d) error\n", __FUNCTION__, __LINE__,
				sfsm_info->s_state.slave_node_fsm_state));
	}

	init_si4432_trans_data_info_st(&data_info);
	data_info.ms_id.did	 = LIGHT300_MASTER_SFID;
	data_info.databuf.tx_buf = tx_buf;
	data_info.buflen	 = RF_TX_BUFFER_SIZE;
	data_info.datalen	 = h.len + MASTER_SLAVE_DATA_REP_FREAM_HEAD_LEN;
	trans_err = si4432_transport_data(STDC_SEND_REQUEST_REP, &data_info);
	if (STE_TX_ERROR_STATE == trans_err) {
		reset_si4432();
		ret = MRS_RESULT_FAIL;
	} else if (STE_OK != trans_err) {
		slave_fsm_info(("send %3d to master node fail\n", sfsm_info->echo_cnt));
		ret = MRS_RESULT_FAIL;
	}

	return ret;
}

/* mark by zp */
static ms_result_state_t slave_send_collect_data(uint8_t *tx_buf, struct slave_app_fsm_info_st *sfsm_info)
{
	ms_result_state_t ret = MRS_RESULT_SUCC;
	struct si4432_trans_data_info data_info;
	struct master_slave_data_frame_head_s h;
	si4432_transport_err_t trans_err;
	struct msfd_report_collection_data_sn_st collect_data_sn;
	
	if (NULL == tx_buf || NULL == sfsm_info) {
		slave_fsm_info(("func:%s(),param error\n",__FUNCTION__));
		return MRS_RESULT_FAIL;
	}
	
	init_master_slave_frame_head(&h);
	if (SFS_SEND_PUSH_CMD == sfsm_info->s_state.slave_node_fsm_state) {
		/* get 7880 data */
		get_self_sn(collect_data_sn.sn, sizeof(collect_data_sn.sn));
				
		px_virtual_mode_voltage(PHASE_A, &collect_data_sn.coll_data.pt_ct_v[0]);
		px_virtual_mode_voltage(PHASE_B, &collect_data_sn.coll_data.pt_ct_v[1]);
		px_virtual_mode_voltage(PHASE_C, &collect_data_sn.coll_data.pt_ct_v[2]);
		
		px_virtual_mode_current(PHASE_A, &collect_data_sn.coll_data.pt_ct_i[0]);
		px_virtual_mode_current(PHASE_B, &collect_data_sn.coll_data.pt_ct_i[1]);
		px_virtual_mode_current(PHASE_C, &collect_data_sn.coll_data.pt_ct_i[2]);

		collect_data_sn.coll_data.pt_ct_app_p[0] = px_apparent_mode_power(PHASE_A);
		collect_data_sn.coll_data.pt_ct_app_p[1] = px_apparent_mode_power(PHASE_B);
		collect_data_sn.coll_data.pt_ct_app_p[2] = px_apparent_mode_power(PHASE_C);

		collect_data_sn.coll_data.pt_ct_ap_p[0] = px_active_mode_power(PHASE_A);
		collect_data_sn.coll_data.pt_ct_ap_p[1] = px_active_mode_power(PHASE_B);
		collect_data_sn.coll_data.pt_ct_ap_p[2] = px_active_mode_power(PHASE_C);

#if 1
		slave_fsm_debug(("\nvoltage:%10d, %10d, %10d\n", collect_data_sn.coll_data.pt_ct_v[0],
			collect_data_sn.coll_data.pt_ct_v[1], collect_data_sn.coll_data.pt_ct_v[2]));

		slave_fsm_debug(("current:%10d, %10d, %10d\n", collect_data_sn.coll_data.pt_ct_i[0],
			collect_data_sn.coll_data.pt_ct_i[1], collect_data_sn.coll_data.pt_ct_i[2]));
		
		slave_fsm_debug(("  app-p:%10d, %10d, %10d\n", collect_data_sn.coll_data.pt_ct_app_p[0],
			collect_data_sn.coll_data.pt_ct_app_p[1], collect_data_sn.coll_data.pt_ct_app_p[2]));
		
		slave_fsm_debug(("  act-p:%10d, %10d, %10d\n\n", collect_data_sn.coll_data.pt_ct_ap_p[0],
			collect_data_sn.coll_data.pt_ct_ap_p[1], collect_data_sn.coll_data.pt_ct_ap_p[2]));
#endif
		h.cmd = MNP_PUSH_COLLECTION_DATA;
		h.len = sizeof(struct msfd_report_collection_data_sn_st) - 4;
		package_master_slave_data(tx_buf,RF_TX_BUFFER_SIZE,&h, &collect_data_sn);
	} else {
		slave_fsm_info(("func:%s(),line:%d,state(%d) error\n",__FUNCTION__,__LINE__,
			sfsm_info->s_state.slave_node_fsm_state));
	}
	
	init_si4432_trans_data_info_st(&data_info);
	data_info.ms_id.did = LIGHT300_MASTER_SFID;
	data_info.databuf.tx_buf = tx_buf;
	data_info.buflen = RF_TX_BUFFER_SIZE;
	data_info.datalen = h.len + MASTER_SLAVE_DATA_FREAM_HEAD_LEN;
	trans_err = si4432_transport_data(STDC_SEND_REQUEST, &data_info);

	if (STE_TX_ERROR_STATE == trans_err) {
		reset_si4432();
		ret = MRS_RESULT_FAIL;
	} else if (STE_OK != trans_err) {
		slave_fsm_info(("send collect data fail\n"));
		ret = MRS_RESULT_FAIL;
	}
	
	return ret;
}

/* mark by zp */
static ms_result_state_t slave_recv_collect_rep(uint8_t *rx_buf, struct slave_app_fsm_info_st *sfsm_info)
{
	ms_result_state_t ret = MRS_RESULT_SUCC;
	struct si4432_trans_data_info data_info;
	struct master_slave_data_frame_head_s h;
	si4432_transport_err_t trans_err;
	if (NULL == rx_buf || NULL == sfsm_info) {
		slave_fsm_info(("func:%s(),param error\n",__FUNCTION__));
		return MRS_RESULT_FAIL;
	}

	init_si4432_trans_data_info_st(&data_info);
	data_info.databuf.rx_buf = rx_buf;
	data_info.buflen = RF_RX_BUFFER_SIZE;
	trans_err = si4432_transport_data(STDC_WAIT_REQUEST_REP, &data_info);
	if (STE_OK == trans_err) {
		unpackage_master_slave_data(data_info.databuf.rx_buf, data_info.datalen, &h, 
					sfsm_info->buf.unpkt_buf, sizeof(sfsm_info->buf.unpkt_buf));
		if (MNA_COLLECTION_DATA_REP == h.cmd) {
			ret = MRS_RESULT_SUCC;
#if 0
		} else if (MNA_ECHO == h.cmd) {
			ret = MRS_RESULT_START_ECHO;
#endif
		} else {
			ret = MRS_RESULT_PENDIND;
			//slave_fsm_info(("func:%s,line:%d,cmd(%d) error\n", __FUNCTION__,__LINE__,h.cmd));
		}
	} else if (STE_TX_ERROR_STATE == trans_err) {
		reset_si4432();
		ret = MRS_RESULT_FAIL;
	} else if (STE_WAIT_REP_TIMEOUT == trans_err) {
		ret = MRS_RESULT_FAIL;
		slave_fsm_info(("func:%s,line:%d recv collect rep fail\n",__FUNCTION__,__LINE__));
		set_ezmac_into_idle_state();
	} else if (STE_RX_ERROR_STATE == trans_err) {
		slave_fsm_debug(("func:%s(), line:%d, mac state not match si4432 state fail(%d)\n",
				__FUNCTION__, __LINE__, trans_err));
		_reset_system();
		ret = MRS_RESULT_PENDIND;
	} else if (STE_CHECK_RECV_STATUS == trans_err) {
		ret = MRS_RESULT_PENDIND;;
	}

	return ret;
}
#if 0
static ms_result_state_t slave_recv_echo(uint8_t *rx_buf, struct slave_app_fsm_info_st *sfsm_info)
{
	ms_result_state_t ret = MRS_RESULT_SUCC;
	struct si4432_trans_data_info data_info;
	struct master_slave_data_frame_head_s	h;
	si4432_transport_err_t trans_err;

	if (NULL==rx_buf || NULL==sfsm_info) {
		slave_fsm_info(("func:%s(), param error\n", __FUNCTION__));
		return MRS_RESULT_FAIL;
	}

	init_si4432_trans_data_info_st(&data_info);
	data_info.databuf.rx_buf = rx_buf;
	data_info.buflen	 = RF_RX_BUFFER_SIZE;
	trans_err = si4432_transport_data(STDC_WAIT_REQUEST, &data_info);
	if (STE_OK == trans_err) {
		unpackage_master_slave_data(data_info.databuf.rx_buf, data_info.buflen, &h,
				sfsm_info->buf.unpkt_buf, sizeof(sfsm_info->buf.unpkt_buf));

		if ((MNA_ECHO==h.cmd)) {// && (get_slave_self_mac_id() == data_info.ms_id.did)
			sfsm_info->echo_cnt = sfsm_info->buf.unpkt_buf[0];
			slave_fsm_info(("recv %3d from master node succ\n", sfsm_info->echo_cnt));
#if SLAVE_ECHO_STATISTICS
			++echo_rx_succ;
#endif
		} else if ((MNA_STOP_ECHO==h.cmd)) {
			ret = MRS_RESULT_STOP_ECHO;
			slave_fsm_info(("stop echo\n"));
		} else {
			slave_fsm_info(("func:%s(), line:%d, cmd:%d, sid:(%d)\n",
					__FUNCTION__, __LINE__, h.cmd, data_info.ms_id.did));
			slave_fsm_info(("recv %3d from master node fail\n", slave_master_echo_cnt+1));
#if SLAVE_ECHO_STATISTICS
			++echo_rx_fail;
#endif
			ret = MRS_RESULT_FAIL;
		}
	} else if (STE_TX_ERROR_STATE == trans_err) {
		reset_si4432();
		ret = MRS_RESULT_FAIL;
	} else if (STE_WAIT_REP_TIMEOUT == trans_err) {
		ret = MRS_RESULT_FAIL;
		slave_fsm_info(("func:%s,line:%d recv collect rep fail\n",__FUNCTION__,__LINE__));
	} else if (STE_RX_ERROR_STATE == trans_err) {
		reset_si4432();
		ret = MRS_RESULT_FAIL;
		slave_fsm_debug(("func:%s(), line:%d, mac state not match si4432 state fail(%d)\n",
				__FUNCTION__, __LINE__, trans_err));
	} else if (STE_CHECK_RECV_STATUS == trans_err) {
		ret = MRS_RESULT_FAIL;
	}

	return ret;
}

static ms_result_state_t slave_send_echo_rep(uint8_t *tx_buf, struct slave_app_fsm_info_st *sfsm_info)
{
	ms_result_state_t ret = MRS_RESULT_SUCC;

	struct si4432_trans_data_info 		data_info;
	struct master_slave_data_frame_head_s	h;
	si4432_transport_err_t 			trans_err;

	if (NULL==tx_buf || NULL==sfsm_info) {
		slave_fsm_info(("func:%s(), param error\n", __FUNCTION__));
		return MRS_RESULT_FAIL;
	}

	init_master_slave_frame_head(&h);
	if (1 == is_stop_echo) {
		h.cmd	= MNA_STOP_ECHO_REP;
		h.len	= 1;
	} else {
		h.cmd	= MNA_ECHO_REP;
		h.len	= 1;
	}	
	package_master_slave_data(tx_buf, RF_TX_BUFFER_SIZE, &h, &sfsm_info->echo_cnt);

	init_si4432_trans_data_info_st(&data_info);
	data_info.ms_id.did	 = LIGHT300_MASTER_SFID;
	data_info.databuf.tx_buf = tx_buf;
	data_info.buflen	 = RF_TX_BUFFER_SIZE;
	data_info.datalen	 = h.len + MASTER_SLAVE_DATA_REP_FREAM_HEAD_LEN;
	trans_err = si4432_transport_data(STDC_SEND_REQUEST_REP, &data_info);
	if (STE_TX_ERROR_STATE == trans_err) {
		reset_si4432();
		ret = MRS_RESULT_FAIL;
	} else if (STE_OK != trans_err) {
		slave_fsm_info(("send %3d to master node fail\n", sfsm_info->echo_cnt));
#if SLAVE_ECHO_STATISTICS
		++echo_tx_fail;
#endif
		ret = MRS_RESULT_FAIL;
	} else {
		slave_fsm_info(("send %3d to master node succ\n", sfsm_info->echo_cnt));
#if SLAVE_ECHO_STATISTICS
		++echo_tx_succ;
#endif
	}

	return ret;
}
#endif

/*
 * 根据，来决定初始状态
 * */
static base_t init_slave_node_fsm(struct slave_fsm_state_st *state)
{
	struct msfd_distribute_cfgdata_st cfgdata;

	if (NULL == state) {
		slave_fsm_info(("func:%s(), param error\n", __FUNCTION__));
		return FAIL;
	}

	if (SUCC == get_wireless_cfgdata(&cfgdata)) {
		if (is_slave_id_valid(cfgdata.slave_id)) {
			state->slave_node_fsm_state	= SFS_IDLE;
			state->slave_next_state		= SNS_PUSH_CMD_MODE;
			proc_cfg_data_for_slave(&cfgdata);
			set_slave_node_rfparam(&cfgdata);
			rt_timer_start(sscd_timer);
		} else {
			state->slave_node_fsm_state	= SFS_WAIT_PROBE_SLAVE_NODE;
			state->slave_next_state		= SNS_IDLE;
		}
		return SUCC;
	} else {
		state->slave_node_fsm_state	= SFS_WAIT_PROBE_SLAVE_NODE;
		state->slave_next_state		= SNS_IDLE;
		slave_fsm_info(("func:%s(), get slave cfg-data fail!\n", __FUNCTION__));
		return FAIL;
	}
}

base_t init_slave_app_fsm_info_st(struct slave_app_fsm_info_st *p)
{
	if (NULL == p) {
		slave_fsm_info(("func:%s(), param error\n", __FUNCTION__));
		return FAIL;
	}

	/* 全部设置为0 */
	rt_memset(p, 0, sizeof(*p));

	/* 只需要设置不为0的成员即可 */
	init_slave_node_fsm(&p->s_state);

	return SUCC;
}


static base_t record_wireless_cfgdata(struct msfd_distribute_cfgdata_st *p)
{
	struct tinywireless_if_info_st	 tw_info;

	if (NULL == p) {
		slave_fsm_info(("func:%s(), param error\n", __FUNCTION__));
		return FAIL;
	}

	slave_fsm_info(("func:%s(), sn:", __FUNCTION__));
	print_dev_sn(p->sn, sizeof(p->sn));
	slave_fsm_info(("channel:%d, cid:%d, slave_id:%d, syn_word:0x%x%x\n",/* mark by zp */
			p->channel_no, p->cid, p->slave_id, p->syn_word[0], p->syn_word[1]));
	if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_TW_INFO, 0, &tw_info)) {
		printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
		return FAIL;
	}

	tw_info.channel_no	= p->channel_no;
	tw_info.cid		= p->cid;
	tw_info.slave_id	= p->slave_id;
	tw_info.sync_word 	= ((p->syn_word[0] << 8) | p->syn_word[1]);

	if (RT_EOK != write_syscfgdata_tbl(SYSCFGDATA_TBL_TW_INFO, 0, &tw_info)) {
		printf_syn("%s(), write syscfg data tbl fail\n", __FUNCTION__);
		return FAIL;
	}
	syscfgdata_syn_proc();

	return SUCC;
}

/* mark by zp */
static void proc_cfg_data_for_slave(struct msfd_distribute_cfgdata_st *p)
{
	if (SUCC == set_ezmac_into_idle_state()) {
		write_ezmacpro_reg_check_if_succ(SFID, p->slave_id);

		rt_memcpy(&dist_cfg, p, sizeof(dist_cfg));
	} else {
		slave_fsm_info(("goto idle state fail,dont set rf param\n"));
	}
}

static void set_slave_node_rfparam(struct msfd_distribute_cfgdata_st *p)
{
	if (SUCC == set_ezmac_into_idle_state()) {
		write_ezmacpro_reg_check_if_succ(SCID, p->cid);
		write_ezmacpro_reg_check_if_succ(FSR,  p->channel_no);
		//Set the SYNC WORD
		macSpiWriteReg(SI4432_SYNC_WORD_3, p->syn_word[0]);
		macSpiWriteReg(SI4432_SYNC_WORD_2, p->syn_word[1]);	
	} else {
		slave_fsm_info(("goto idle state fail,dont set rf param\n"));
	}
}


#endif


int slave_init(void)
{
	unsigned int temp1, temp2;

	/* 以下用于配置spi, 使能在nvic中的timer中断, 配置外部中断并使能在nvic中的外部中断 */
	init_si4432_spi_pin();
	cfg_si4432_spi_param();

	rt_thread_delay(get_ticks_of_ms(400));
	shutdown_si443x_or_not(0);
	rt_thread_delay(get_ticks_of_ms(500));

	temp1 = macSpiReadReg(SI4432_DEVICE_TYPE);
	temp2 = macSpiReadReg(SI4432_DEVICE_VERSION);
	if (0x08!=temp1 || 0x06!=temp2) {
		slave_fsm_log(("si4432 dev-type:0x%x(shoule 0x08), dev-ver:0x%x(should 0x06)\n", temp1, temp2));
		goto err_ret;
	}

	slave_fsm_debug(("line:%d, func:%s(), dev type is right\n", __LINE__, __FUNCTION__));

	cfg_si4432_mac_timer_nvic();

	init_si4432_exti();
	cfg_si4432_exti_nvic();

	/* 至此, EZmacPRO的硬件资源已初始化完成, 下面进行EZmacPRO的初始化 */
	if (SUCC != slave_ezmacpro_init()) {
		slave_fsm_log(("%s node ezmacpro initial fail.\n", NODE_TYPE_NAME_STR));
		goto err_ret;
	}

	slave_fsm_info(("%s node ezmacpro initial succ.\n", NODE_TYPE_NAME_STR));

	/* EZmacPRO已初始化完成, 开始fsm的初始化 */
	slave_cmd		= MNP_PUSH_COLLECTION_DATA;

	init_si4432_transport_fsm();

	return SUCC;

err_ret:
	return FAIL;
}

int set_slave_cmd(master_slave_cmd_t scmd)
{
	slave_cmd = scmd;

	return SUCC;
}

master_slave_cmd_t get_slave_cmd(void)
{
	return slave_cmd;
}

int set_slave_next_state(struct slave_app_fsm_info_st *sfsm_info, slave_next_state_t s)
{
	if (NULL == sfsm_info) {
		slave_fsm_info(("func:%s(), param error\n", __FUNCTION__));
		return FAIL;
	}

	sfsm_info->s_state.slave_next_state  = s;

	return SUCC;
}

ubase_t get_slave_self_mac_id(void)
{
	struct tinywireless_if_info_st	 tw_info;
	ubase_t id;

	if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_TW_INFO, 0, &tw_info)) {
		printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
		id = INVALID_WIRELESS_MAC_ID;
	} else {
		id = tw_info.slave_id;
	}

	return id;
}

static int slave_ezmacpro_init(void)
{
	int temp;
	int ret = SUCC;
	struct msfd_distribute_cfgdata_st wl_cfg_data;

	temp = EZMacPRO_Init();
	if (MAC_OK != temp) {
		slave_fsm_log(("EZMacPRO_Init fail(%d)\n", temp));
		ret = FAIL;
		goto ret_entry;
	}

//	slave_fsm_debug(("line:%d, func:%s(), temp:%d\n", __LINE__, __FUNCTION__, temp));

#if 1 /* wakeup si4432 */
	if (WEIS_OK != wait_emmacpro_into_state(EZMACS_WAKEUP, WAIT_EMMACPRO_INTO_STATE_FOREVER)) {
		slave_fsm_log(("line:%d, func:%s(), wait into wakeup fail\n", __LINE__, __FUNCTION__));
		ret = FAIL;
		goto ret_entry;
	}
	clr_ezmac_state_trans_flag(fEZMacPRO_StateWakeUpEntered);
	clr_ezmac_state_trans_flag(fEZMacPRO_StateSleepEntered);

//	slave_fsm_debug(("line:%d, func:%s(), temp:%d\n", __LINE__, __FUNCTION__, temp));

	temp = 0;
#if 0
	/* Configure and start 2sec timeout for Silabs splash screen. */
	temp += write_ezmacpro_reg_check_if_succ(LFTMR0, LFTMR0_TIMEOUT_SEC(STARTUP_TIMEOUT_S));
	temp += write_ezmacpro_reg_check_if_succ(LFTMR1, LFTMR1_TIMEOUT_SEC(STARTUP_TIMEOUT_S));
	temp += write_ezmacpro_reg_check_if_succ(LFTMR2, LFTMR2_WAKEUP_TIMER_ENABLED_BIT | LFTMR2_32KHZ_OSCILLATOR_BIT
			| LFTMR2_TIMEOUT_SEC(STARTUP_TIMEOUT_S));

	if (WEIS_OK != wait_emmacpro_into_state(EZMACS_LFTIMER_EXPIRED, WAIT_EMMACPRO_INTO_STATE_FOREVER)) {
		slave_fsm_log(("line:%d, func:%s(), wait into lft-timer expired fail\n", __LINE__, __FUNCTION__));
		ret = FAIL;
		goto ret_entry;
	}
#endif

//	slave_fsm_debug(("line:%d, func:%s(), temp:%d\n", __LINE__, __FUNCTION__, temp));

	/* Disable LFT. */
	temp += write_ezmacpro_reg_check_if_succ(LFTMR2, ~LFTMR2_WAKEUP_TIMER_ENABLED_BIT
			& (LFTMR2_32KHZ_OSCILLATOR_BIT | LFTMR2_TIMEOUT_SEC(STARTUP_TIMEOUT_S)));
#endif
	/* Star demo Master node initialisation. */
	temp += write_ezmacpro_reg_check_if_succ(MCR,  0xAC);	// CIDE=1, DR=9.6kbps, RAD=1, DNPL=1, NFR=0

#if 0
	/* State after receive is RX state and state after transmit is Idle state */
	temp += write_ezmacpro_reg_check_if_succ(SECR, 0x60);
#else
	/* State after Receive is Idle; state after Transmit is Idle */
	temp += write_ezmacpro_reg_check_if_succ(SECR, 0x50);
#endif
	temp += write_ezmacpro_reg_check_if_succ(RCR,  0x00);	// Search disable
	temp += write_ezmacpro_reg_check_if_succ(FR0,  0);	// Set the used frequency channel
	/* LBT enabled/disabled, Output power: +20 dBm, ACK disable, AFC disable */
	temp += write_ezmacpro_reg_check_if_succ(TCR,  lbt_switch_on(0x70));
	temp += write_ezmacpro_reg_check_if_succ(LBTLR, 0x78);	// RSSI threshold -60 dB
	temp += write_ezmacpro_reg_check_if_succ(LBTIR, 0x8A);	// Time interval
	temp += write_ezmacpro_reg_check_if_succ(LBDR,  0x80);	// Enable Low Battery Detect

//	temp += write_ezmacpro_reg_check_if_succ(PFCR,  0x28);	// Destination address filter is enabled
	temp += write_ezmacpro_reg_check_if_succ(PFCR, 0x91);  // CID filter and Mcast addr. filter (MCA mode) are enabled

	if (SUCC == get_wireless_cfgdata(&wl_cfg_data)) {
//		wl_cfg_data.channel_no
//		wl_cfg_data.syn_word[0], wl_cfg_data.syn_word[1]
		temp += write_ezmacpro_reg_check_if_succ(SCID,  wl_cfg_data.cid);	// Set customer ID
//		temp += write_ezmacpro_reg_check_if_succ(SFID,  get_slave_self_mac_id());	// Set self ID
		temp += write_ezmacpro_reg_check_if_succ(SFID,  0XFF==wl_cfg_data.slave_id ? 1 : wl_cfg_data.slave_id);
	} else {
		slave_fsm_log(("func:%s(), get wireless cfg data fail\n", __FUNCTION__));
		ret = FAIL;
		goto ret_entry;
	}

	if (0 != temp) {
		slave_fsm_log(("write ezmacpro reg fail(cnt:%d), when init ezmacpro\n", temp));
		ret = FAIL;
		goto ret_entry;
	}

	slave_fsm_debug(("%s startup done.\n", NODE_TYPE_NAME_STR));

	rt_thread_delay(get_ticks_of_ms(20)); /* NOTE:must delay! mark by David */

	if (MAC_OK != EZMacPRO_Wake_Up()) {	/* Wake up from Sleep mode. */
		slave_fsm_log(("EZMacPRO_Wake_Up fail\n"));
		ret = FAIL;
		goto ret_entry;
	}

//	slave_fsm_debug(("line:%d, func:%s()\n", __LINE__, __FUNCTION__));

	if (WEIS_OK != wait_emmacpro_into_state(EZMACS_IDLE, WAIT_EMMACPRO_INTO_STATE_FOREVER)) {
		slave_fsm_log(("line:%d, func:%s(), wait into idle fail\n", __LINE__, __FUNCTION__));
		ret = FAIL;
		goto ret_entry;
	}
	clr_ezmac_state_trans_flag(fEZMacPRO_StateWakeUpEntered);
	clr_ezmac_state_trans_flag(fEZMacPRO_StateIdleEntered);

ret_entry:
	return ret;
}




#if 0
#include <finsh.h>
#include <si4432_v2.h>
#include <spi4ezmacpro.h>

#define slave_fsm_test(x) 	printf_syn x

unsigned char recv_buf[64];

void sfsm_test(int cmd)
{
	unsigned int temp1, temp2;
	struct ezmacpro_pkt_header_t h;
	unsigned char ch;
	ubase_t payload_len;

	(void)temp1;
	(void)temp2;

	switch (cmd) {
	case 0:
		slave_init();
		break;

	case 1:
		recv_pkt_from_ezmacpro_prepare();
		if (WEIS_OK != wait_emmacpro_into_state(EZMACS_PKT_RECEIVED, WAIT_EMMACPRO_INTO_STATE_FOREVER)) {
			slave_fsm_test(("func:%s(), line:%d\n", __FUNCTION__, __LINE__));
			print_callback_var();
		}

		recv_pkt_from_ezmacpro(recv_buf, sizeof(recv_buf), &payload_len, &rssi);
		slave_fsm_test(("func:%s(), line:%d, recv 1st byte:0x%x, recv len:%d\n", __FUNCTION__, __LINE__,
				recv_buf[0], payload_len));

//		rt_thread_delay(get_ticks_of_ms(2000));

		h.dst_id = LIGHT300_MASTER_SFID;
		ch = recv_buf[0] + 1;
		sent_pkt_to_ezmacpro(&h, &ch, 1, SPEFB_FORCE_SEND);
		slave_fsm_test(("func:%s(), line:%d, send data:0x%x\n", __FUNCTION__, __LINE__, ch));

		if (WEIS_OK != wait_emmacpro_into_state(EZMACS_PKT_SENT, 2000)) {
			slave_fsm_test(("func:%s(), line:%d\n", __FUNCTION__, __LINE__));
			get_ezmacproc_fsm_state();
			print_callback_var();
		}

		break;

	case 2:
		slave_fsm_info(("TIMEOUT_XTAL_START:%d, TIMEOUT_LBTI_ETSI:%d, TimeoutLBTI:%d, TimeoutChannelSearch:%d\n"
				"TimeoutSyncWord:%d, TimeoutTX_Packet:%d, TimeoutRX_Packet:%d, TimeoutACK:%d\n"
				"mpl:%d\n",
				TIMEOUT_XTAL_START, TIMEOUT_LBTI_ETSI, TimeoutLBTI, TimeoutChannelSearch,
				TimeoutSyncWord, TimeoutTX_Packet, TimeoutRX_Packet, TimeoutACK,
				EZMacProReg.name.MPL));
		break;

	case 3:
		slave_next_state = SNS_ECHO_MODE;
		break;

	default:
		break;
	}

	return;
}
FINSH_FUNCTION_EXPORT(sfsm_test, "slave fsm test");
#endif
