/*
 * master_fsm.c
 *
 *  Created on: 2013-11-27
 *      Author: David, zhaoshaowei@yeejoin.com
 *
 *  2014-01-10: 使主节点可以同时处理多个不同状态的从节点数据
 *  	发送完数据后, si4432必须尽快切换至接收状态
 */


#include <stack.h>
#include <EZMacPro.h>

#include <si4432_hardif.h>
#include <master.h>
#include <master_fsm.h>

#include <master_app.h>

#include <sys_cfg_api.h>

#include <ade7880_api.h>/* mark by zp */

#include <rs485.h>/* mark by zp */

#include <board.h>

#define NODE_TYPE_NAME_STR	"master node"


#define master_fsm_log(x)	printf_syn x
#define master_fsm_debug(x) 	printf_syn x
#define master_fsm_info(x) 	printf_syn x


rt_timer_t  ms_echo_timer;

volatile uint8_t master_slave_doing_echo;
volatile uint8_t master_slave_echo_timeout_cnt;

volatile base_t is_recv_probe_from_485;
volatile base_t is_recv_wlnet_cfg_from_485_over;

struct register_slave_node_info_st register_slave_node_info[WIRELESS_SLAVE_NODE_MAX];

static int master_ezmacpro_init(void);
static base_t change_master_node_fsm_state(mfsm_state_t *master_node_fsm_state, mfsm_state_t new_state,
		struct master_app_fsm_info_st *mfsm_info);
static base_t init_master_node_fsm(mfsm_state_t *state);
static base_t get_next_wireless_cfgdata(struct wireless_cfgdata_use4tx_st *cfgdata, int *index);
static base_t reset_master_tx_rx_cnt(struct master_app_fsm_info_st *p);
static ms_result_state_t master_send_probe_cmd(uint8_t *tx_buf, struct master_app_fsm_info_st *mfsm_info);
static ms_result_state_t master_recv_probe_cmd_rep(uint8_t *rx_buf, struct master_app_fsm_info_st *mfsm_info);
static ms_result_state_t master_send_cfg_data(uint8_t *tx_buf, struct master_app_fsm_info_st *mfsm_info,
		struct wireless_cfgdata_use4tx_st *cfgdata);
static ms_result_state_t master_recv_cfg_data_rep(uint8_t *rx_buf, struct master_app_fsm_info_st *mfsm_info,
		struct wireless_cfgdata_use4tx_st *cfgdata);
//static ms_result_state_t master_send_echo(uint8_t *tx_buf, struct master_app_fsm_info_st *mfsm_info);
//static ms_result_state_t master_recv_echo_rep(uint8_t *rx_buf, struct master_app_fsm_info_st *mfsm_info);

static int updata_slave_node_rssi(uint8_t slave_usr_id, uint8_t rssi);
static base_t proc_probe_slave_node_rep_msg(struct si4432_trans_data_info *data_info,
		struct master_slave_data_frame_head_s *h, uint8_t *data, uint8_t len);
static base_t init_register_slave_node_info_use_white_list(struct register_slave_node_info_st *p);

static ms_result_state_t master_recv_collect_data(uint8_t *rx_buf, struct master_app_fsm_info_st *mfsm_info);/* mark by zp */
static ms_result_state_t master_send_collect_rep(uint8_t *tx_buf, struct master_app_fsm_info_st *mfsm_info);
void rs485_send_rep_echo_cmd(rt_uint8_t *tx_buf, rt_uint8_t *src_addr, rt_uint8_t *echo, struct rs485_frame_format *format);

void rs485_send_rep_netcfg_cmd(rt_uint8_t *tx_buf, rt_uint8_t *src_addr, rt_uint8_t *data, rt_uint8_t data_len, struct rs485_frame_format *format);

void rs485_send_rep_stop_probe_cmd(rt_uint8_t *tx_buf, rt_uint8_t *src_addr, struct rs485_frame_format *format);

static int updata_slave_node_data(struct msfd_report_collection_data_sn_st *msfd_collect_data, struct msfd_report_collection_data_sn_st *data);

static void proc_cfg_data_for_master(struct msfd_distribute_cfgdata_st *p);

extern rt_uint8_t *rs485_send_buf_s;

//extern rt_uint8_t echo_id_s;

extern struct msfd_report_collection_data_sn_st *pc_collect_data_s;
 
//rt_uint8_t is_stop_echo = 0;
//rt_uint8_t is_start_echo = 0;

//extern struct wireless_register_white_st *wireless_white_register;

extern rt_sem_t    probe_timeout_sem;

int master_init(void)
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
		master_fsm_log(("si4432 dev-type:0x%x(shoule 0x08), dev-ver:0x%x(should 0x06)\n", temp1, temp2));
		goto err_ret;
	}

	master_fsm_debug(("func:%s(), dev type is right\n", __FUNCTION__));

	cfg_si4432_mac_timer_nvic();

	init_si4432_exti();
	cfg_si4432_exti_nvic();

	/* 至此, EZmacPRO的硬件资源已初始化完成, 下面进行EZmacPRO的初始化 */
	if (SUCC != master_ezmacpro_init()) {
		master_fsm_log(("%s node ezmacpro initial fail.\n", NODE_TYPE_NAME_STR));
		goto err_ret;
	}

	master_fsm_info(("%s node ezmacpro initial succ.\n", NODE_TYPE_NAME_STR));

	/* EZmacPRO已初始化完成, 开始fsm的初始化 */
	init_si4432_transport_fsm(); /* 抽象的简单传输层的初始化 */

	return SUCC;

err_ret:
	return FAIL;
}

static void reset_si4432(void)
{
	while (SUCC != master_ezmacpro_init()) {
		master_fsm_log(("reset si4432 fail"));
	}
	_init_si4432_transport_fsm();
}

extern void reset_system(void);
static void _reset_system(void)
{
	reset_system();
}

base_t init_master_app_fsm_info_st(struct master_app_fsm_info_st *p)
{
	if (NULL == p) {
		master_fsm_info(("func:%s(), param error\n", __FUNCTION__));
		return FAIL;
	}

	/* 全部设置为0 */
	rt_memset(p, 0, sizeof(*p));

	/* 只需要设置不为0的成员即可 */
	init_master_node_fsm(&p->master_fsm_state);
	p->cmd			= STDC_BUTT;

	return SUCC;
}

static int get_netcfg_fail_data(struct register_slave_node_info_st *info, struct record_netcfg_fail_data_st *to_info)
{
	int num = 0, i;

	for (i=0; i<SLAVE_NODE_INFO_MAX_LEN; i++) {
		if (info[i].rssi == 0xFF) {
			rt_memcpy(to_info[num++].sn, info[i].sn, sizeof(info[0].sn));
		}
	}

	return num;
}

int master_node_app_fsm(struct master_app_fsm_info_st *mfsm_info)
{
	mfsm_state_t master_node_fsm_state;
	uint8_t *tx_pch, *rx_pch;
	ms_result_state_t ms_result = MRS_RESULT_SUCC;
	rt_err_t err;
	rt_uint32_t temp;
	int index;

	if (NULL==mfsm_info || NULL==mfsm_info->tx_buf || NULL==mfsm_info->rx_buf) {
		master_fsm_info(("func:%s(), param error\n", __FUNCTION__));
		return FAIL;
	}

	tx_pch = mfsm_info->tx_buf;
	rx_pch = mfsm_info->rx_buf;
	master_node_fsm_state = mfsm_info->master_fsm_state;

	switch (master_node_fsm_state) {
	case MFS_WAIT_PROBE_SLAVE_CMD_FROM_485BUS:/* mark by zp */
		err = rt_mb_recv(uart485_mb, &temp, RT_WAITING_FOREVER);
		if (RT_EOK == err) {
			if (ESTP_PROBE_SLAVE_CMD == temp) {
				rt_memset(register_slave_node_info, 0, sizeof(register_slave_node_info));
				if ( MRS_RESULT_SUCC == master_send_probe_cmd(tx_pch, mfsm_info)) {
					change_master_node_fsm_state(&master_node_fsm_state,
							MFS_WAIT_PROBE_SLAVE_CMD_REP, mfsm_info);
				} else {
					master_fsm_info(("func:%s(), line:%d, send probe cmd error\n", __FUNCTION__, __LINE__));
				}
			} else if (ESTP_STOP_PROBE_SLAVE_CMD == temp) {
				char src_sn[DEV_SN_MODE_LEN];
				struct rs485_frame_format format;
				
				master_fsm_info(("goto netcfg state\n"));
				get_devsn(src_sn, DEV_SN_MODE_LEN);		
				rs485_send_rep_stop_probe_cmd(rs485_send_buf_s, (rt_uint8_t *)src_sn, &format);
				change_master_node_fsm_state(&master_node_fsm_state,
						MFS_WAIT_WIRELESS_NETCFG_FROM_485BUS, mfsm_info);
			}
		} else {
			master_fsm_info(("func:%s,line:%d recv uart485_mb error\n",__FUNCTION__,__LINE__));
		}

		break;

	case MFS_WAIT_PROBE_SLAVE_CMD_REP:/* mark by zp */
		ms_result = master_recv_probe_cmd_rep(rx_pch, mfsm_info);

		if (MRS_RESULT_TIMEOUT == ms_result) {
			change_master_node_fsm_state(&master_node_fsm_state,
						MFS_WAIT_PROBE_SLAVE_CMD_FROM_485BUS, mfsm_info);
				/* 将收集到的无线数据发送给em主设备 */
			//send_detect_slave_node2em_dev_by_485();
			rt_sem_release(probe_timeout_sem);
			set_ezmac_into_idle_state();
		}

		break;

	case MFS_WAIT_WIRELESS_NETCFG_FROM_485BUS:/* mark by zp sem */
		err = rt_mb_recv(uart485_mb, &temp, RT_WAITING_FOREVER);
		if (RT_EOK == err) {
			if (ESTP_NETCFG_CMD == temp) {
				//int i;
				rt_memset(register_slave_node_info, 0, sizeof(register_slave_node_info));
				init_register_slave_node_info_use_white_list(register_slave_node_info);
#if 0
				for (i=0; i<SLAVE_NODE_INFO_MAX_LEN; i++) {
					rt_memcpy(register_slave_node_info[i].sn, wireless_white_register[i].sn_bytes, sizeof(wireless_white_register[0].sn_bytes));
				}
#endif
				if (SUCC == get_next_wireless_cfgdata(&mfsm_info->buf.wl_cfgdata, &index)) {
					change_master_node_fsm_state(&master_node_fsm_state, MFS_DISTRIBUTE_SLAVE_NODE_CFG_DATA, mfsm_info);
				} else {
					change_master_node_fsm_state(&master_node_fsm_state, MFS_WAIT_PROBE_SLAVE_CMD_FROM_485BUS, mfsm_info);
				}
			}
		} else {
			master_fsm_info(("func:%s,line:%d recv uart485_mb error\n", __FUNCTION__, __LINE__));
		}

		break;

	case MFS_DISTRIBUTE_SLAVE_NODE_CFG_DATA:
		if (mfsm_info->tx_rx_cnt.tx_cnt > DISTRIBUTE_WIRELESS_CFGDATA_CNT_MAX) {
			master_fsm_log(("func:%s(),line:%d, distribute wireless cfg data error, sn:",
				__FUNCTION__, __LINE__));
			print_dev_sn(mfsm_info->buf.wl_cfgdata.sn, DEV_SN_MODE_LEN);
			if (index >= 0) {
				register_slave_node_info[index].rssi = 0xFF;
			}
			reset_master_tx_rx_cnt(mfsm_info);
			if (FAIL == get_next_wireless_cfgdata(&mfsm_info->buf.wl_cfgdata, &index)) {/* mark by zp */
				char src_sn[DEV_SN_MODE_LEN];
				struct rs485_frame_format format;
				struct msfd_distribute_cfgdata_st wl_cfgdata;/* change rf param */
				int num;
				struct record_netcfg_fail_data_st ird_info[WIRELESS_SLAVE_NODE_MAX];

				num = get_netcfg_fail_data(register_slave_node_info, ird_info);
				get_devsn(src_sn, DEV_SN_MODE_LEN);
				rs485_send_rep_netcfg_cmd(rs485_send_buf_s, (rt_uint8_t *)src_sn, (rt_uint8_t *)ird_info,
					sizeof(struct record_netcfg_fail_data_st)*num, &format);
				get_wireless_cfgdata(&wl_cfgdata);
				proc_cfg_data_for_master(&wl_cfgdata);/* 改变无线主节点的参数 */
				change_master_node_fsm_state(&master_node_fsm_state, MFS_LISTEN_MODE, mfsm_info);
				master_fsm_info(("net cfg end\n"));
			}
		} else {
			/* send cfg data */
			ms_result = master_send_cfg_data(tx_pch, mfsm_info, &mfsm_info->buf.wl_cfgdata);
			if (MRS_RESULT_SUCC == ms_result) {
				master_node_fsm_state = MFS_WAIT_DISTRIBUTE_SLAVE_NODE_CFG_DATA_REP;
			} else {
				master_fsm_info(("func:%s(), line:%d, send cfg-data error\n", __FUNCTION__, __LINE__));
			}
		}
		break;

	case MFS_WAIT_DISTRIBUTE_SLAVE_NODE_CFG_DATA_REP:
		ms_result = master_recv_cfg_data_rep(rx_pch, mfsm_info, &mfsm_info->buf.wl_cfgdata);
		if (MRS_RESULT_PENDIND != ms_result) {
			if (MRS_RESULT_SUCC == ms_result) {
				set_ezmac_into_idle_state();
				if (SUCC != get_next_wireless_cfgdata(&mfsm_info->buf.wl_cfgdata, &index)) {
					char src_sn[DEV_SN_MODE_LEN];
					struct rs485_frame_format format;
					struct msfd_distribute_cfgdata_st wl_cfgdata;/* change rf param */
					int num;
					struct record_netcfg_fail_data_st ird_info[WIRELESS_SLAVE_NODE_MAX];

					num = get_netcfg_fail_data(register_slave_node_info, ird_info);
					get_devsn(src_sn, DEV_SN_MODE_LEN);
					rs485_send_rep_netcfg_cmd(rs485_send_buf_s, (rt_uint8_t *)src_sn, (rt_uint8_t *)ird_info,
						sizeof(struct record_netcfg_fail_data_st)*num, &format);
					get_wireless_cfgdata(&wl_cfgdata);
					proc_cfg_data_for_master(&wl_cfgdata);
					change_master_node_fsm_state(&master_node_fsm_state,
									MFS_LISTEN_MODE, mfsm_info);
					master_fsm_info(("net cfg end\n"));
					master_fsm_info(("func:%s,line:%d into listen mode\n",__FUNCTION__,__LINE__));
				} else {
					change_master_node_fsm_state(&master_node_fsm_state,
							MFS_DISTRIBUTE_SLAVE_NODE_CFG_DATA, mfsm_info);
					rt_thread_delay(20);
				}
			} else if (MRS_RESULT_FAIL == ms_result) {
				master_node_fsm_state = MFS_DISTRIBUTE_SLAVE_NODE_CFG_DATA;
				rt_thread_delay(20);
			}
		}
		break;

	case MFS_LISTEN_MODE:
#if 0
		if (1 == is_stop_echo) {
			is_stop_echo = 0;
			master_slave_doing_echo = MASTER_SLAVE_ECHO_STOP;
			master_fsm_info(("goto stop echo state\n"));
		}
		if (MASTER_SLAVE_ECHO_START == master_slave_doing_echo) {
			master_slave_doing_echo = MASTER_SLAVE_ECHO_WORK;
			master_node_fsm_state = MFS_SEND_ECHO_REQUEST;
			rt_timer_start(ms_echo_timer);
			mfsm_info->echo_cnt = 0;
		} else if (MASTER_SLAVE_ECHO_WORK == master_slave_doing_echo) {
			if (0 != master_slave_echo_timeout_cnt)  {
				--master_slave_echo_timeout_cnt;
				master_node_fsm_state = MFS_SEND_ECHO_REQUEST;
			}
		} else if (MASTER_SLAVE_ECHO_STOP == master_slave_doing_echo) {
			rt_timer_stop(ms_echo_timer);
			master_slave_echo_timeout_cnt = 0;
			master_slave_doing_echo	= MASTER_SLAVE_ECHO_IDLE;
			master_node_fsm_state = MFS_SEND_ECHO_REQUEST;
			master_fsm_info(("stop echo cmd\n"));
		} else { /* mark by zp */
#endif		
			ms_result = master_recv_collect_data(rx_pch, mfsm_info);
			if (ms_result == MRS_RESULT_SUCC) {
				/* save data to em by 485 */
				change_master_node_fsm_state(&master_node_fsm_state, 
						MFS_COLLECT_DATA_REP, mfsm_info);
				/* mark by zp */
				/* goto idle state */
				set_ezmac_into_idle_state();
			} else {
				//master_fsm_info(("func:%s,line:%d recv erro\n",__FUNCTION__,__LINE__));
			}
			break;
#if 0
		}
		break;/* mark by zp */
#endif

#if 0
	case MFS_SEND_ECHO_REQUEST:
		ms_result = master_send_echo(tx_pch, mfsm_info);
		if (MRS_RESULT_SUCC == ms_result) {
			change_master_node_fsm_state(&master_node_fsm_state,
					MFS_RECV_ECHO_REQUEST_REP, mfsm_info);
		} else {
			change_master_node_fsm_state(&master_node_fsm_state,
					MFS_LISTEN_MODE, mfsm_info);
			break;
		}
		//break; /* falls through *//* mark by zp */

	case MFS_RECV_ECHO_REQUEST_REP:
		master_recv_echo_rep(rx_pch, mfsm_info);
		if (MRS_RESULT_STOP_ECHO == ms_result) {
			change_master_node_fsm_state(&master_node_fsm_state, MFS_LISTEN_MODE, mfsm_info);
		} else {
			change_master_node_fsm_state(&master_node_fsm_state, MFS_LISTEN_MODE, mfsm_info);		
		}
		break;
#endif
	case MFS_COLLECT_DATA_REP:/* mark by zp */
		//rt_thread_delay(10);
		ms_result = master_send_collect_rep(tx_pch, mfsm_info);
		if (ms_result == MRS_RESULT_SUCC) {
#if 0
			if (is_start_echo) {
				is_start_echo = 0;
				master_slave_doing_echo = MASTER_SLAVE_ECHO_START;
				mfsm_info->echo_did 	= convert_usr_scope_id_to_ezmac_id(echo_id_s);
			}
#endif
			change_master_node_fsm_state(&master_node_fsm_state, 
						MFS_LISTEN_MODE, mfsm_info);
		} else {
			/* goto idle state */
			set_ezmac_into_idle_state();
			change_master_node_fsm_state(&master_node_fsm_state, 
						MFS_LISTEN_MODE, mfsm_info);
			master_fsm_info(("func:%s,line:%d collect data rep error\n",__FUNCTION__,__LINE__));
		}
		break;

	default:
		break;
	}

	mfsm_info->master_fsm_state = master_node_fsm_state;

	return SUCC;
}

static void proc_cfg_data_for_master(struct msfd_distribute_cfgdata_st *p)
{
	if (SUCC == set_ezmac_into_idle_state()) {
		write_ezmacpro_reg_check_if_succ(SCID, p->cid);
		write_ezmacpro_reg_check_if_succ(FSR, p->channel_no);
		//Set the SYNC WORD
		macSpiWriteReg(SI4432_SYNC_WORD_3, p->syn_word[0]);
		macSpiWriteReg(SI4432_SYNC_WORD_2, p->syn_word[1]);
	} else {
		master_fsm_info(("goto idle state fail,dont set rf param\n"));
	}
}

#if 0
static int send_detect_slave_node2em_dev_by_485(void)
{
	struct rs485_frame_format format;
	char dev_sn[DEV_SN_MODE_LEN];
	int j;
	struct register_slave_node_original_info_st *original_info;

	original_info = rt_malloc(sizeof(struct register_slave_node_original_info_st)*WIRELESS_SLAVE_NODE_MAX);
	if (original_info == RT_NULL) {
		master_fsm_info(("malloc fail\n"));
		return FAIL;
	}
	rt_memset(original_info, 0, sizeof(struct register_slave_node_original_info_st)*WIRELESS_SLAVE_NODE_MAX);
#if 0!=INVALID_RSSI_VALUE
	int i;
#endif
	/* 将register_slave_node_info中的有效内容通过485总线发送给em主设备 */
	for (j=0; j<sizeof(register_slave_node_info)/sizeof(register_slave_node_info[0]); ++j) {
		if (!is_dev_sn_valid(register_slave_node_info[j].sn, sizeof(register_slave_node_info[0].sn))) {
			break;			
		} else {
			rt_memcpy(&original_info[j], &register_slave_node_info[j], sizeof(original_info[0]));
		}
	}

	if (j >= 0) {
		get_devsn(dev_sn, DEV_SN_MODE_LEN);
		rs485_send_rep_probe_cmd(rs485_send_buf_s, dev_sn, (rt_uint8_t *)original_info, sizeof(original_info[0])*j, &format);	
	}
	rt_free(original_info);
	rt_memset(&register_slave_node_info, 0, sizeof(register_slave_node_info));
#if 0!=INVALID_RSSI_VALUE
	for (i=0; i<sizeof(register_slave_node_info)/sizeof(register_slave_node_info[0]); ++i)
		register_slave_node_info[i].rssi = INVALID_RSSI_VALUE;
#endif
	return SUCC;
}
#endif

static int updata_slave_node_rssi(uint8_t slave_usr_id, uint8_t rssi)
{
	if (slave_usr_id < WIRELESS_SLAVE_NODE_MAX) {
		register_slave_node_info[slave_usr_id].rssi = rssi;
		return SUCC;
	} else {
		printf_syn("func:%s(), slave_usr_id is too large (%d, %d)!\n", __FUNCTION__,
				slave_usr_id, WIRELESS_SLAVE_NODE_MAX);
		return FAIL;
	}
}

void updata_self_data(struct msfd_report_collection_data_sn_st *msfd_collect_data)
{
	int i;
	struct msfd_report_collection_data_st self_data;
	rt_uint8_t self_sn[DEV_SN_MODE_LEN];
	
	get_self_sn(self_sn, sizeof(self_sn));
	px_virtual_mode_voltage(PHASE_A, &self_data.pt_ct_v[0]);
	px_virtual_mode_voltage(PHASE_B, &self_data.pt_ct_v[1]);
	px_virtual_mode_voltage(PHASE_C, &self_data.pt_ct_v[2]);
	
	px_virtual_mode_current(PHASE_A, &self_data.pt_ct_i[0]);
	px_virtual_mode_current(PHASE_B, &self_data.pt_ct_i[1]);
	px_virtual_mode_current(PHASE_C, &self_data.pt_ct_i[2]);

	self_data.pt_ct_app_p[0] = px_apparent_mode_power(PHASE_A);
	self_data.pt_ct_app_p[1] = px_apparent_mode_power(PHASE_B);
	self_data.pt_ct_app_p[2] = px_apparent_mode_power(PHASE_C);

	self_data.pt_ct_ap_p[0] = px_active_mode_power(PHASE_A);
	self_data.pt_ct_ap_p[1] = px_active_mode_power(PHASE_B);
	self_data.pt_ct_ap_p[2] = px_active_mode_power(PHASE_C);
	
	for (i=0;i<WIRELESS_SLAVE_NODE_MAX;i++) {
		if (0 == rt_memcmp(self_sn, msfd_collect_data[i].sn, sizeof(self_sn))) {
			rt_memcpy(&msfd_collect_data[i].coll_data, &self_data, sizeof(struct msfd_report_collection_data_st));
			msfd_collect_data[i].last_update_time = rt_tick_get();
			break;
		}
	}
	if (i >= WIRELESS_SLAVE_NODE_MAX) {
		for (i=0;i<WIRELESS_SLAVE_NODE_MAX;i++) {
			if (!is_dev_sn_valid(msfd_collect_data[i].sn, sizeof(msfd_collect_data[0].sn))) {
				rt_memcpy(msfd_collect_data[i].sn, self_sn, sizeof(msfd_collect_data[0].sn));
				rt_memcpy(&msfd_collect_data[i].coll_data, &self_data, sizeof(self_data));
				msfd_collect_data[i].last_update_time = rt_tick_get();
				break;
			}
		}
	}
#if 1
	if (i < WIRELESS_SLAVE_NODE_MAX) {
		master_fsm_info(("self sn:"));
		print_dev_sn(msfd_collect_data[i].sn, sizeof(msfd_collect_data[0].sn));
		master_fsm_info(("self data:\n"));
		master_fsm_info(("voltage:%10d, %10d, %10d\n", msfd_collect_data[i].coll_data.pt_ct_v[0],
				msfd_collect_data[i].coll_data.pt_ct_v[1], msfd_collect_data[i].coll_data.pt_ct_v[2]));
		master_fsm_info(("current:%10d, %10d, %10d\n", msfd_collect_data[i].coll_data.pt_ct_i[0],
				msfd_collect_data[i].coll_data.pt_ct_i[1], msfd_collect_data[i].coll_data.pt_ct_i[2]));
		master_fsm_info(("  app-p:%10d, %10d, %10d\n", msfd_collect_data[i].coll_data.pt_ct_app_p[0],
				msfd_collect_data[i].coll_data.pt_ct_app_p[1], msfd_collect_data[i].coll_data.pt_ct_app_p[2]));
		master_fsm_info(("  act-p:%10d, %10d, %10d\n\n", msfd_collect_data[i].coll_data.pt_ct_ap_p[0],
				msfd_collect_data[i].coll_data.pt_ct_ap_p[1], msfd_collect_data[i].coll_data.pt_ct_ap_p[2]));
	}
#endif
}

static int updata_slave_node_data(struct msfd_report_collection_data_sn_st *msfd_collect_data, struct msfd_report_collection_data_sn_st *data)
{
	int i;
	
	for (i=0;i<WIRELESS_SLAVE_NODE_MAX;i++) {
		if (0 == rt_memcmp(data->sn, msfd_collect_data[i].sn, sizeof(msfd_collect_data[0].sn))) {
			rt_memcpy(&msfd_collect_data[i].coll_data, &data->coll_data, sizeof(struct msfd_report_collection_data_st));
			msfd_collect_data[i].last_update_time = rt_tick_get();
			break;
		}
	}
	if (i >= WIRELESS_SLAVE_NODE_MAX) {
		for (i=0;i<WIRELESS_SLAVE_NODE_MAX;i++) {
			if (!is_dev_sn_valid(msfd_collect_data[i].sn, sizeof(msfd_collect_data[0].sn))) {
				rt_memcpy(msfd_collect_data[i].sn, data->sn, sizeof(msfd_collect_data[0].sn));
				rt_memcpy(&msfd_collect_data[i].coll_data, &data->coll_data, sizeof(struct msfd_report_collection_data_st));
				msfd_collect_data[i].last_update_time = rt_tick_get();
				break;
			}
		}
	}
#if 1
	if (i < WIRELESS_SLAVE_NODE_MAX) {
		master_fsm_info(("recv collect data sn:"));
		print_dev_sn(msfd_collect_data[i].sn, sizeof(msfd_collect_data[0].sn));
		master_fsm_info(("voltage:%10d, %10d, %10d\n", msfd_collect_data[i].coll_data.pt_ct_v[0],
				msfd_collect_data[i].coll_data.pt_ct_v[1], msfd_collect_data[i].coll_data.pt_ct_v[2]));
		master_fsm_info(("current:%10d, %10d, %10d\n", msfd_collect_data[i].coll_data.pt_ct_i[0],
				msfd_collect_data[i].coll_data.pt_ct_i[1], msfd_collect_data[i].coll_data.pt_ct_i[2]));
		master_fsm_info(("  app-p:%10d, %10d, %10d\n", msfd_collect_data[i].coll_data.pt_ct_app_p[0],
				msfd_collect_data[i].coll_data.pt_ct_app_p[1], msfd_collect_data[i].coll_data.pt_ct_app_p[2]));
		master_fsm_info(("  act-p:%10d, %10d, %10d\n\n", msfd_collect_data[i].coll_data.pt_ct_ap_p[0],
				msfd_collect_data[i].coll_data.pt_ct_ap_p[1], msfd_collect_data[i].coll_data.pt_ct_ap_p[2]));
	}
#endif
	return i;
}

/* mark by zp */
static ms_result_state_t master_recv_collect_data(uint8_t *rx_buf, struct master_app_fsm_info_st *mfsm_info)
{
	ms_result_state_t ret = MRS_RESULT_SUCC;
	struct si4432_trans_data_info data_info;
	struct master_slave_data_frame_head_s h;
	si4432_transport_err_t trans_err;

	if (NULL == rx_buf || NULL == mfsm_info) {
		master_fsm_info(("func:%s(),param error\n", __FUNCTION__));
		return MRS_RESULT_FAIL;
	}

	init_si4432_trans_data_info_st(&data_info);
	data_info.databuf.rx_buf = rx_buf;
	data_info.buflen = RF_RX_BUFFER_SIZE;
	trans_err = si4432_transport_data(STDC_WAIT_REQUEST, &data_info);
	
	if (STE_OK == trans_err) {
		unpackage_master_slave_data(data_info.databuf.rx_buf, data_info.datalen, &h,
			mfsm_info->buf.unpkt_buf, sizeof(mfsm_info->buf.unpkt_buf));
		if (MNP_PUSH_COLLECTION_DATA == h.cmd) {
			updata_slave_node_rssi(convert_ezmac_id_to_usr_scope_id(data_info.ms_id.sid), data_info.rssi);
			updata_slave_node_data(pc_collect_data_s, (struct msfd_report_collection_data_sn_st *)mfsm_info->buf.unpkt_buf);
			//master_fsm_info(("recv slave data succ\n"));
			ret = MRS_RESULT_SUCC;
		} else {
			master_fsm_info(("func:%s(),line:%d,cmd:%d cmd error\n",__FUNCTION__,__LINE__,h.cmd));
			ret = MRS_RESULT_FAIL;
		}
	} else if (STE_TX_ERROR_STATE == trans_err) {
		reset_si4432();
		ret = MRS_RESULT_FAIL;
	} else if (STE_RX_ERROR_STATE == trans_err) {
		master_fsm_debug(("func:%s(), line:%d, mac state not match si4432 state fail(%d)\n",
				__FUNCTION__, __LINE__, trans_err));
		_reset_system();
		ret = MRS_RESULT_PENDIND;
	} else if (STE_CHECK_RECV_STATUS == trans_err) {
		ret = MRS_RESULT_PENDIND;
	} else {
		master_fsm_info(("func:%s,line:%d,si4432_transport_data fail:%d\n",__FUNCTION__,__LINE__,trans_err));
		ret = MRS_RESULT_FAIL;
	}

	return ret;
	
}

/* mark by zp */
static ms_result_state_t master_send_collect_rep(uint8_t *tx_buf, struct master_app_fsm_info_st *mfsm_info)
{
	ms_result_state_t ret = MRS_RESULT_SUCC;
	struct si4432_trans_data_info data_info;
	struct master_slave_data_frame_head_s h;
	si4432_transport_err_t trans_err;

	if (NULL == tx_buf || NULL == mfsm_info) {
		master_fsm_info(("func:%s,param error\n",__FUNCTION__));
		return MRS_RESULT_FAIL;
	}

	init_master_slave_frame_head(&h);
#if 0
	if (is_start_echo) {
		h.cmd = MNA_ECHO;
		h.len = 0;
	} else {
#endif
		h.cmd = MNA_COLLECTION_DATA_REP;
		h.len = 0;
#if 0
	}
#endif	
	
	package_master_slave_data(tx_buf,RF_RX_BUFFER_SIZE, &h, NULL);

	init_si4432_trans_data_info_st(&data_info);
	data_info.ms_id.did = get_pkt_sid();
	data_info.databuf.tx_buf = tx_buf;
	data_info.buflen = RF_TX_BUFFER_SIZE;
	data_info.datalen = h.len + MASTER_SLAVE_DATA_REP_FREAM_HEAD_LEN;
	trans_err = si4432_transport_data(STDC_SEND_REQUEST_REP,&data_info);
	if (STE_TX_ERROR_STATE == trans_err) {
		reset_si4432();
		ret = MRS_RESULT_FAIL;
	} else if (STE_OK != trans_err) {
		master_fsm_info(("send collect rep fail\n"));
		ret = MRS_RESULT_FAIL;
	} else {
		//master_fsm_info(("send collect rep succ\n"));
	}

	return ret;
}

#if 0
static ms_result_state_t master_send_echo(uint8_t *tx_buf, struct master_app_fsm_info_st *mfsm_info)
{
	ms_result_state_t ret = MRS_RESULT_SUCC;

	struct si4432_trans_data_info 		data_info;
	struct master_slave_data_frame_head_s	h;
	si4432_transport_err_t 			trans_err;

	if (NULL==tx_buf || NULL==mfsm_info) {
		master_fsm_info(("func:%s(), param error\n", __FUNCTION__));
		return MRS_RESULT_FAIL;
	}

	init_master_slave_frame_head(&h);
	if (master_slave_doing_echo == MASTER_SLAVE_ECHO_IDLE) {
		h.cmd	= MNA_STOP_ECHO;
		h.len	= 1;
	} else {
		h.cmd	= MNA_ECHO;
		h.len	= 1;	
	}
	package_master_slave_data(tx_buf, RF_TX_BUFFER_SIZE, &h, &mfsm_info->echo_cnt);

	init_si4432_trans_data_info_st(&data_info);
	data_info.ms_id.did	 = mfsm_info->echo_did;
	data_info.databuf.tx_buf = tx_buf;
	data_info.buflen	 = RF_TX_BUFFER_SIZE;
	data_info.datalen	 = h.len + MASTER_SLAVE_DATA_FREAM_HEAD_LEN;
	trans_err = si4432_transport_data(STDC_SEND_REQUEST, &data_info);
	if (STE_TX_ERROR_STATE == trans_err) {
		reset_si4432();
		ret = MRS_RESULT_FAIL;
	} else if (STE_OK != trans_err) {
		master_fsm_info(("send %3d to NO.%d slave node fail\n", mfsm_info->echo_cnt, mfsm_info->echo_did));
#if MASTER_ECHO_STATISTICS
		++echo_tx_fail;
#endif
		ret = MRS_RESULT_FAIL;
	} else {
		//master_fsm_info(("send %3d to NO.%d slave node succ\n", mfsm_info->echo_cnt, mfsm_info->echo_did));
#if MASTER_ECHO_STATISTICS
		++echo_tx_succ;
#endif
	}

	++mfsm_info->echo_cnt;

	return ret;
} 

static ms_result_state_t master_recv_echo_rep(uint8_t *rx_buf, struct master_app_fsm_info_st *mfsm_info)
{
	ms_result_state_t ret = MRS_RESULT_SUCC;
	struct si4432_trans_data_info data_info;
	struct master_slave_data_frame_head_s	h;
	si4432_transport_err_t trans_err;
	char dev_sn[DEV_SN_MODE_LEN];
	struct rs485_frame_format format;

	if (NULL==rx_buf || NULL==mfsm_info) {
		master_fsm_info(("func:%s(), param error\n", __FUNCTION__));
		return MRS_RESULT_FAIL;
	}

	init_si4432_trans_data_info_st(&data_info);
	data_info.databuf.rx_buf = rx_buf;
	data_info.buflen	 = RF_RX_BUFFER_SIZE;
	trans_err = si4432_transport_data(STDC_WAIT_REQUEST_REP, &data_info);
	if (STE_OK == trans_err) {
		unpackage_master_slave_data(data_info.databuf.rx_buf, data_info.buflen, &h,
				mfsm_info->buf.unpkt_buf, sizeof(mfsm_info->buf.unpkt_buf));

		if ((MNA_ECHO_REP==h.cmd) && (mfsm_info->echo_did == data_info.ms_id.sid)) {
			master_fsm_info(("recv %3d from NO.%d slave node succ\n", mfsm_info->buf.unpkt_buf[0],
					mfsm_info->echo_did));
			updata_slave_node_rssi(convert_ezmac_id_to_usr_scope_id(data_info.ms_id.sid), data_info.rssi);
			get_devsn(dev_sn, DEV_SN_MODE_LEN);
			rs485_send_rep_echo_cmd(rs485_send_buf_s, (rt_uint8_t *)dev_sn, &mfsm_info->echo_did, &format);
#if MASTER_ECHO_STATISTICS
			++echo_rx_succ;
#endif
		} else if (MNA_STOP_ECHO_REP==h.cmd) {
			master_fsm_info(("be going to stop echo\n", mfsm_info->buf.unpkt_buf[0],
					mfsm_info->echo_did));
			updata_slave_node_rssi(convert_ezmac_id_to_usr_scope_id(data_info.ms_id.sid), data_info.rssi);
			ret = MRS_RESULT_STOP_ECHO;
		} else {
			master_fsm_info(("func:%s(), line:%d, cmd:%d, sid:%d)\n",
					__FUNCTION__, __LINE__, h.cmd, data_info.ms_id.sid));
			goto recv_echo_fail;
		}
	} else if (STE_TX_ERROR_STATE == trans_err) {
		reset_si4432();
		ret = MRS_RESULT_FAIL;
	} else if (STE_WAIT_REP_TIMEOUT == trans_err) {
		ret = MRS_RESULT_FAIL;
	} else if (STE_RX_ERROR_STATE == trans_err) {
		reset_si4432();
		ret = MRS_RESULT_PENDIND;
		master_fsm_debug(("func:%s(), line:%d, mac state not match si4432 state fail(%d)\n",
				__FUNCTION__, __LINE__, trans_err));
	} else if (STE_CHECK_RECV_STATUS != trans_err) {
		master_fsm_debug(("func:%s(), line:%d, si4432_transport_data() fail(%d)\n",
				__FUNCTION__, __LINE__, trans_err));
		goto recv_echo_fail;
	}

	return ret;

recv_echo_fail:
	ret = MRS_RESULT_FAIL;
	master_fsm_info(("recv %3d from NO.%d slave node fail\n", (mfsm_info->echo_cnt-1), mfsm_info->echo_did));
#if MASTER_ECHO_STATISTICS
	++echo_rx_fail;
#endif

	return ret;
}
#endif

static base_t change_master_node_fsm_state(mfsm_state_t *master_node_fsm_state, mfsm_state_t new_state,
		struct master_app_fsm_info_st *mfsm_info)
{
	if (NULL==master_node_fsm_state || NULL==mfsm_info) {
		master_fsm_info(("func:%s(), param error\n", __FUNCTION__));
		return FAIL;
	}

//	master_fsm_debug(("func:%s(), %d->%d\n", __FUNCTION__, *master_node_fsm_state, new_state));

	*master_node_fsm_state = new_state;

	mfsm_info->tx_rx_cnt.rx_cnt = 0;
	mfsm_info->tx_rx_cnt.tx_cnt = 0;

	return SUCC;
}

static void set_master_node_rfparam(void)
{
	struct tinywireless_if_info_st	 tw_info;

	if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_TW_INFO, 0, &tw_info)) {
		printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
		return;
	}

	if (SUCC == set_ezmac_into_idle_state()) {
		write_ezmacpro_reg_check_if_succ(SCID, tw_info.cid);
		write_ezmacpro_reg_check_if_succ(FSR, tw_info.channel_no);
		//Set the SYNC WORD
		macSpiWriteReg(SI4432_SYNC_WORD_3, (tw_info.sync_word>>8)&0xff);
		macSpiWriteReg(SI4432_SYNC_WORD_2, tw_info.sync_word&0xff);	
	} else {
		master_fsm_info(("goto idle state fail,dont set rf param\n"));
	}
}

/*
 * 根据注册白名单是否为空以及配置数据是否下发成功，来决定初始状态
 * */
static base_t init_master_node_fsm(mfsm_state_t *state)
{
	if (NULL == state) {
		master_fsm_info(("func:%s(), param error\n", __FUNCTION__));
		return FAIL;
	}

	if (TRUE == is_wl_white_list_empty() ) {
		*state = MFS_WAIT_PROBE_SLAVE_CMD_FROM_485BUS;
	} else {
		*state = MFS_LISTEN_MODE;
		set_master_node_rfparam();
		init_register_slave_node_info_use_white_list(register_slave_node_info);
	}

	return SUCC;
}

static base_t init_register_slave_node_info_use_white_list(struct register_slave_node_info_st *p)
{
	struct wireless_register_white_st *white_register;
	int i, j;

	if (NULL == p) {
		master_fsm_info(("func:%s(), param error\n", __FUNCTION__));
		return FAIL;
	}

	/* 申请动态内存 */
	white_register = rt_malloc(WIRELESS_SLAVE_NODE_MAX * sizeof(struct wireless_register_white_st));
	if (NULL == white_register) {
		printf_syn("func:%s(), out of memory\n", __FUNCTION__);
		return FAIL;
	}

	read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_REGISTER_WHITER, 0, white_register); /* 读数据 */

	/* 修改内容 */
	j = 0;
	for (i=0; i<WIRELESS_SLAVE_NODE_MAX; ++i) {
		rt_memcpy((p+j)->sn, (white_register+i)->sn_bytes, sizeof(p->sn));
		++j;
	}


	rt_free(white_register);

	return SUCC;
}

#if 0
static base_t is_recv_probe_cmd_from_485(void)
{
	if (is_recv_probe_from_485) {
		is_recv_probe_from_485 = 0;

		/* 将register_slave_node_info中的内容写入注册白名单中 */

		/* ...... */

		return TRUE;
	} else {
		return FALSE;
	}
}
#endif

#if 0
static base_t is_recv_wl_netcfg_from_485_over(void)
{
	if (is_recv_wlnet_cfg_from_485_over) {
		is_recv_wlnet_cfg_from_485_over = 0;
		return TRUE;
	} else {
		return FALSE;
	}
}
#endif

static base_t get_next_wireless_cfgdata(struct wireless_cfgdata_use4tx_st *cfgdata, int *index)
{
	int i;
	struct msfd_distribute_cfgdata_st wl_cfgdata;

	*index = -1;
	if (NULL == cfgdata) {
		master_fsm_info(("func:%s(), param error\n", __FUNCTION__));
		return FAIL;
	}

	for (i=0; i<WIRELESS_SLAVE_NODE_MAX; ++i) {
		if (is_dev_sn_valid(register_slave_node_info[i].sn, sizeof(register_slave_node_info[i].sn))
				&& INVALID_RSSI_VALUE==register_slave_node_info[i].rssi)
			break;
	}

	if (i < WIRELESS_SLAVE_NODE_MAX) {
		cfgdata->slave_id	= convert_usr_scope_id_to_ezmac_id(i);
		rt_memcpy(cfgdata->sn, register_slave_node_info[i].sn, sizeof(cfgdata->sn));

		get_wireless_cfgdata(&wl_cfgdata);
		cfgdata->channel_no	= wl_cfgdata.channel_no;
		cfgdata->cid		= wl_cfgdata.cid;
		cfgdata->syn_word[0]	= wl_cfgdata.syn_word[0];
		cfgdata->syn_word[1]	= wl_cfgdata.syn_word[1];

		*index = i;
		
		return SUCC;
	}

	return FAIL;
}


static base_t reset_master_tx_rx_cnt(struct master_app_fsm_info_st *p)
{
	if (NULL == p) {
		master_fsm_info(("func:%s(), param error\n", __FUNCTION__));
		return FAIL;
	}

	p->tx_rx_cnt.rx_cnt = 0;
	p->tx_rx_cnt.tx_cnt = 0;

	return SUCC;
}


/*
 *
 * */
static ms_result_state_t master_send_probe_cmd(uint8_t *tx_buf, struct master_app_fsm_info_st *mfsm_info)
{
	ms_result_state_t ret = MRS_RESULT_SUCC;

	struct si4432_trans_data_info data_info;
	struct master_slave_data_frame_head_s	h;
	si4432_transport_err_t trans_err;

	uint8_t sn[DEV_SN_MODE_LEN];

	if (NULL==tx_buf || NULL==mfsm_info) {
		master_fsm_info(("func:%s(), param error\n", __FUNCTION__));
		return MRS_RESULT_FAIL;
	}

	get_self_sn(sn, sizeof(sn));

	init_master_slave_frame_head(&h);
	h.cmd	= MNA_DETECT_SLAVE_NODE;
	h.len	= sizeof(sn);
	package_master_slave_data(tx_buf, RF_TX_BUFFER_SIZE, &h, sn);

	init_si4432_trans_data_info_st(&data_info);
	data_info.ms_id.did	 = EZMAC_BROADCAST_ADDR;
	data_info.databuf.tx_buf = tx_buf;
	data_info.buflen	 = RF_TX_BUFFER_SIZE;
	data_info.datalen	 = h.len + MASTER_SLAVE_DATA_FREAM_HEAD_LEN;
	trans_err = si4432_transport_data(STDC_SEND_REQUEST, &data_info);
	if (STE_TX_ERROR_STATE == trans_err) {
		reset_si4432();
		ret = MRS_RESULT_FAIL;
	} else if (STE_OK != trans_err) {
		master_fsm_info(("func:%s(), line:%d, si4432_transport_data() fail(%d)\n",
				__FUNCTION__, __LINE__, trans_err));
		ret = MRS_RESULT_FAIL;
	}

	return ret;
}

static ms_result_state_t master_recv_probe_cmd_rep(uint8_t *rx_buf, struct master_app_fsm_info_st *mfsm_info)
{
	ms_result_state_t ret = MRS_RESULT_SUCC;
	struct si4432_trans_data_info data_info;
	struct master_slave_data_frame_head_s	h;
	si4432_transport_err_t trans_err;

	if (NULL==rx_buf || NULL==mfsm_info) {
		master_fsm_info(("func:%s(), param error\n", __FUNCTION__));
		return MRS_RESULT_FAIL;
	}

	init_si4432_trans_data_info_st(&data_info);
	data_info.databuf.rx_buf = rx_buf;
	data_info.buflen	 = RF_RX_BUFFER_SIZE;
	trans_err = si4432_transport_data(STDC_WAIT_MULTI_REQUEST_REP, &data_info);/* mark by zp */

	switch (trans_err) {
	case STE_PENDING:/* mark by zp */
		unpackage_master_slave_data(data_info.databuf.rx_buf, data_info.buflen, &h,
				mfsm_info->buf.unpkt_buf, sizeof(mfsm_info->buf.unpkt_buf));

		if (MNA_DETECT_SLAVE_NODE_REP == h.cmd) {
			/* send_recv_probe_rep_indication(); */ /* 发送指示, 有可能会错失接收探测响应 */
			proc_probe_slave_node_rep_msg(&data_info, &h, mfsm_info->buf.unpkt_buf, h.len);
		} else {
			master_fsm_info(("func:%s,line:%d cmd error\n",__FUNCTION__,__LINE__));
		}
		ret = MRS_RESULT_PENDIND;
		break; /* not falls through */

	case STE_WAIT_REP_TIMEOUT:/* mark by zp */
		ret = MRS_RESULT_TIMEOUT;
		break;
		
	case STE_WAIT_REQUEST_TIMEOUT:
	case STE_FAIL:
	case STE_PTR_NULL:
	case STE_STATE_CMD_NOT_MATCH:
	case STE_EZMAC_BUSY:
	case STE_SEND_FAIL:
		ret = MRS_RESULT_FAIL;
		master_fsm_debug(("func:%s(), line:%d, si4432_transport_data() fail(%d)\n",
				__FUNCTION__, __LINE__, trans_err));
		break;
		
	case STE_CHECK_RECV_STATUS:
		ret = MRS_RESULT_PENDIND;
		break;

	case STE_TX_ERROR_STATE:
		reset_si4432();
		ret = MRS_RESULT_FAIL;
		break;

	case STE_RX_ERROR_STATE:
		reset_si4432();
		ret = MRS_RESULT_PENDIND;
		master_fsm_debug(("func:%s(), line:%d, mac state not match si4432 state fail(%d)\n",
				__FUNCTION__, __LINE__, trans_err));
		break;

	default:
		ret = MRS_RESULT_FAIL;
		master_fsm_debug(("func:%s(), line:%d, si4432_transport_data() fail(%d)\n",
				__FUNCTION__, __LINE__, trans_err));
		break;
	}

	return ret;
}


static ms_result_state_t master_send_cfg_data(uint8_t *tx_buf, struct master_app_fsm_info_st *mfsm_info,
		struct wireless_cfgdata_use4tx_st *cfgdata)
{
	ms_result_state_t ret = MRS_RESULT_SUCC;

	struct msfd_distribute_cfgdata_st distri_cfgdata;
	struct si4432_trans_data_info data_info;
	struct master_slave_data_frame_head_s	h;
	si4432_transport_err_t trans_err;

	if (NULL==tx_buf || NULL==mfsm_info || NULL==cfgdata) {
		master_fsm_info(("func:%s(), param error\n", __FUNCTION__));
		return MRS_RESULT_FAIL;
	}

	distri_cfgdata.channel_no	= cfgdata->channel_no;
	distri_cfgdata.cid		= cfgdata->cid;
	distri_cfgdata.slave_id		= cfgdata->slave_id;
	rt_memcpy(distri_cfgdata.sn, cfgdata->sn, sizeof(distri_cfgdata.sn));
	distri_cfgdata.syn_word[0]	= cfgdata->syn_word[0];
	distri_cfgdata.syn_word[1]	= cfgdata->syn_word[1];

	init_master_slave_frame_head(&h);
	h.cmd	= MNA_DISTRIBUTE_CFG_DATA;
	h.len	= sizeof(struct msfd_distribute_cfgdata_st);
	package_master_slave_data(tx_buf, RF_TX_BUFFER_SIZE, &h, &distri_cfgdata);

	init_si4432_trans_data_info_st(&data_info);
	data_info.ms_id.did	 = EZMAC_BROADCAST_ADDR;
	data_info.databuf.tx_buf = tx_buf;
	data_info.buflen	 = RF_TX_BUFFER_SIZE;
	data_info.datalen	 = h.len + MASTER_SLAVE_DATA_FREAM_HEAD_LEN;
	trans_err = si4432_transport_data(STDC_SEND_REQUEST, &data_info);
	if (STE_TX_ERROR_STATE == trans_err) {
		reset_si4432();
		ret = MRS_RESULT_FAIL;
	} else if (STE_OK != trans_err) {
		master_fsm_info(("func:%s(), line:%d, si4432_transport_data() fail(%d)\n",
				__FUNCTION__, __LINE__, trans_err));
		ret = MRS_RESULT_FAIL;
	}

	++mfsm_info->tx_rx_cnt.tx_cnt;

	return ret;

}


static ms_result_state_t master_recv_cfg_data_rep(uint8_t *rx_buf, struct master_app_fsm_info_st *mfsm_info,
		struct wireless_cfgdata_use4tx_st *cfgdata)
{
	ms_result_state_t ret = MRS_RESULT_SUCC;
	struct si4432_trans_data_info data_info;
	struct master_slave_data_frame_head_s	h;
	si4432_transport_err_t trans_err;

	if (NULL==rx_buf || NULL==mfsm_info || NULL==cfgdata) {
		master_fsm_info(("func:%s(), param error\n", __FUNCTION__));
		return MRS_RESULT_FAIL;
	}

	init_si4432_trans_data_info_st(&data_info);
	data_info.databuf.rx_buf = rx_buf;
	data_info.buflen	 = RF_RX_BUFFER_SIZE;
	trans_err = si4432_transport_data(STDC_WAIT_REQUEST_REP, &data_info);

	switch (trans_err) {
	case STE_OK	:
		unpackage_master_slave_data(data_info.databuf.rx_buf, data_info.buflen, &h,
				mfsm_info->buf.unpkt_buf, sizeof(mfsm_info->buf.unpkt_buf));

		/* --- response, 此时从节点已有ID, 所以, 无需携带数据 */
		if (MNA_DISTRIBUTE_CFG_DATA_REP==h.cmd) {
			/* sid与无线配置的slave_id不同 */
			if (cfgdata->slave_id != data_info.ms_id.sid) {
				ret = MRS_RESULT_PENDIND;
				master_fsm_info(("func:%s,line:%d slave_id error\n",__FUNCTION__,__LINE__));
			} else {
				updata_slave_node_rssi(convert_ezmac_id_to_usr_scope_id(data_info.ms_id.sid),
						data_info.rssi);
				master_fsm_info(("master recv netcfg rep succ\n"));
				master_fsm_info(("rep netcfg sn:"));
				print_dev_sn(register_slave_node_info[cfgdata->slave_id - LIGHT300_SLAVE_NODE_ID_OFFSET].sn, DEV_SN_MODE_LEN);
			}
		} else {
			/* 接收到非期望消息, 丢弃 */
			ret = MRS_RESULT_PENDIND;
			master_fsm_info(("func:%s,line:%d cmd error(%d)\n",__FUNCTION__,__LINE__, h.cmd));
		}

		break; /* not falls through */

	case STE_WAIT_REP_TIMEOUT:
	case STE_WAIT_REQUEST_TIMEOUT:
	case STE_FAIL:
	case STE_PTR_NULL:
	case STE_STATE_CMD_NOT_MATCH:
	case STE_EZMAC_BUSY:
	case STE_SEND_FAIL:
		ret = MRS_RESULT_FAIL;
		//master_fsm_debug(("func:%s(), line:%d, si4432_transport_data() fail(%d)\n",
		//		__FUNCTION__, __LINE__, trans_err));
		break;

	case STE_TX_ERROR_STATE:
		reset_si4432();
		ret = MRS_RESULT_FAIL;
		break;

	case STE_CHECK_RECV_STATUS:
		ret = MRS_RESULT_PENDIND;
		break;

	case STE_RX_ERROR_STATE:
		reset_si4432();
		ret = MRS_RESULT_PENDIND;
		master_fsm_debug(("func:%s(), line:%d, mac state not match si4432 state fail(%d)\n",
				__FUNCTION__, __LINE__, trans_err));
		break;

	default:
		ret = MRS_RESULT_FAIL;
		//master_fsm_debug(("func:%s(), line:%d, si4432_transport_data() fail(%d)\n",
		//		__FUNCTION__, __LINE__, trans_err));
		break;
	}

	++mfsm_info->tx_rx_cnt.rx_cnt;

	return ret;
}

static base_t proc_probe_slave_node_rep_msg(struct si4432_trans_data_info *data_info,
		struct master_slave_data_frame_head_s *h, uint8_t *data, uint8_t len)
{
	int i, ret = SUCC;

	if (NULL==data_info || NULL==h || NULL==data) {
		master_fsm_info(("func:%s(), param error\n", __FUNCTION__));
		return FAIL;
	}

	if (sizeof(register_slave_node_info[0].sn) != len) {
		master_fsm_info(("func:%s(), sn len is error(%d, %d)\n", __FUNCTION__,
				len, sizeof(register_slave_node_info[0].sn)));
		return FAIL;
	}

	master_fsm_debug(("func:%s(), line:%d, recv slave sn:", __FUNCTION__, __LINE__));
	print_dev_sn(data, len);
	master_fsm_debug(("recv slave rssi:%d.\n",data_info->rssi));/* mark by zp */

	/* 查找是否已有记录 */
	for (i=0; i<sizeof(register_slave_node_info)/sizeof(register_slave_node_info[0]); ++i) {
		if (0 == rt_memcmp(register_slave_node_info[i].sn, data, sizeof(register_slave_node_info[0].sn)))
			break;
	}

	if (i < sizeof(register_slave_node_info)/sizeof(register_slave_node_info[0])) {
		register_slave_node_info[i].rssi = data_info->rssi;
	} else {
		/* 寻找一个空位置 */
		for (i=0; i<sizeof(register_slave_node_info)/sizeof(register_slave_node_info[0]); ++i) {
			if (!is_dev_sn_valid(register_slave_node_info[i].sn, sizeof(register_slave_node_info[0].sn)))
				break;
		}

		if (i < sizeof(register_slave_node_info)/sizeof(register_slave_node_info[0])) {
			register_slave_node_info[i].rssi = data_info->rssi;
			rt_memcpy(register_slave_node_info[i].sn, data, sizeof(register_slave_node_info[0].sn));
		} else {
			master_fsm_info(("func:%s(), register_slave_node_info buf is full\n", __FUNCTION__));
			ret = FAIL;
		}
	}

	return ret;
}



static int master_ezmacpro_init(void)
{
	int temp;
	int ret = SUCC;
	struct msfd_distribute_cfgdata_st wl_cfg_data;

	temp = EZMacPRO_Init();
	if (MAC_OK != temp) {
		master_fsm_log(("EZMacPRO_Init fail(%d)\n", temp));
		ret = FAIL;
		goto ret_entry;
	}

//	master_fsm_debug(("line:%d, func:%s(), temp:%d\n", __LINE__, __FUNCTION__, temp));

	if (WEIS_OK != wait_emmacpro_into_state(EZMACS_WAKEUP, WAIT_EMMACPRO_INTO_STATE_FOREVER)) {
		master_fsm_log(("line:%d, func:%s(), wait into wakeup fail\n", __LINE__, __FUNCTION__));
		ret = FAIL;
		goto ret_entry;
	}
	clr_ezmac_state_trans_flag(fEZMacPRO_StateWakeUpEntered);
	clr_ezmac_state_trans_flag(fEZMacPRO_StateSleepEntered);

//	master_fsm_debug(("line:%d, func:%s(), temp:%d\n", __LINE__, __FUNCTION__, temp));

	temp = 0;
#if 0
	/* Configure and start 2sec timeout for Silabs splash screen. */
	temp += write_ezmacpro_reg_check_if_succ(LFTMR0, LFTMR0_TIMEOUT_SEC(STARTUP_TIMEOUT_S));
	temp += write_ezmacpro_reg_check_if_succ(LFTMR1, LFTMR1_TIMEOUT_SEC(STARTUP_TIMEOUT_S));
	temp += write_ezmacpro_reg_check_if_succ(LFTMR2, LFTMR2_WAKEUP_TIMER_ENABLED_BIT | LFTMR2_32KHZ_OSCILLATOR_BIT
			| LFTMR2_TIMEOUT_SEC(STARTUP_TIMEOUT_S));

	if (WEIS_OK != wait_emmacpro_into_state(EZMACS_LFTIMER_EXPIRED, WAIT_EMMACPRO_INTO_STATE_FOREVER)) {
		master_fsm_log(("line:%d, func:%s(), wait into lft-timer expired fail\n", __LINE__, __FUNCTION__));
		ret = FAIL;
		goto ret_entry;
	}
	clr_ezmac_state_trans_flag(fEZMacPRO_LFTimerExpired);
#endif

//	master_fsm_debug(("line:%d, func:%s(), temp:%d\n", __LINE__, __FUNCTION__, temp));

	/* Disable LFT. */
	temp += write_ezmacpro_reg_check_if_succ(LFTMR2, ~LFTMR2_WAKEUP_TIMER_ENABLED_BIT
			& (LFTMR2_32KHZ_OSCILLATOR_BIT | LFTMR2_TIMEOUT_SEC(STARTUP_TIMEOUT_S)));

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

	temp += write_ezmacpro_reg_check_if_succ(PFCR,  0x28);	// Destination address filter is enabled

	if (SUCC == get_wireless_cfgdata(&wl_cfg_data)) {
//		wl_cfg_data.channel_no
//		wl_cfg_data.syn_word[0], wl_cfg_data.syn_word[1]
//		wl_cfg_data.slave_id
		temp += write_ezmacpro_reg_check_if_succ(SCID,  wl_cfg_data.cid);	// Set customer ID
		temp += write_ezmacpro_reg_check_if_succ(SFID,  LIGHT300_MASTER_SFID);	// Set self ID
	} else {
		master_fsm_log(("func:%s(), get wireless cfg data fail\n", __FUNCTION__));
		ret = FAIL;
		goto ret_entry;
	}



	if (0 != temp) {
		master_fsm_log(("write ezmacpro reg fail(cnt:%d), when init ezmacpro\n", temp));
		ret = FAIL;
		goto ret_entry;
	}

	master_fsm_log(("%s setup done.\n", NODE_TYPE_NAME_STR));

	rt_thread_delay(get_ticks_of_ms(20)); /* NOTE:must delay! mark by David */

	if (MAC_OK != EZMacPRO_Wake_Up()) {	/* Wake up from Sleep mode. */
		master_fsm_log(("EZMacPRO_Wake_Up fail\n"));
		ret = FAIL;
		goto ret_entry;
	}

//	master_fsm_debug(("line:%d, func:%s()\n", __LINE__, __FUNCTION__));

	if (WEIS_OK != wait_emmacpro_into_state(EZMACS_IDLE, WAIT_EMMACPRO_INTO_STATE_FOREVER)) {
		master_fsm_log(("line:%d, func:%s(), wait into idle fail\n", __LINE__, __FUNCTION__));
		ret = FAIL;
		goto ret_entry;
	}
	clr_ezmac_state_trans_flag(fEZMacPRO_StateWakeUpEntered);

ret_entry:
	return ret;
}



#if 0
#include <finsh.h>
#include <si4432_v2.h>
#include <spi4ezmacpro.h>

#include <compiler_defs.h>
#include <board.h>

#define master_fsm_test(x) 	printf_syn x

unsigned char recv_buf[64];

void mfsm_test(int cmd)
{
	unsigned int temp1, temp2;
	struct ezmacpro_pkt_header_t h;
	unsigned char ch;
	ubase_t payload_len;
	ubase_t rssi;


	(void)temp1;
	(void)temp2;

	switch (cmd) {
	case 0:
		master_init();
		break;

	case 1:
		h.dst_id = LIGHT300_MASTER_SLAVE_MCAST;
		ch = 0x5a;
		sent_pkt_to_ezmacpro(&h, &ch, 1, SPEFB_SET_MAC_TCR | SPEFB_FORCE_SEND);
//		master_fsm_test(("func:%s(), line:%d\n", __FUNCTION__, __LINE__));

		if (WEIS_OK != wait_emmacpro_into_state(EZMACS_PKT_SENT, 800)) {
			master_fsm_test(("func:%s(), line:%d\n", __FUNCTION__, __LINE__));
			get_ezmacproc_fsm_state();
			print_callback_var();
		}

		master_fsm_test(("func:%s(), line:%d, send pkt over\n", __FUNCTION__, __LINE__));

		recv_pkt_from_ezmacpro_prepare();
		if (WEIS_OK != wait_emmacpro_into_state(EZMACS_PKT_RECEIVED, 6000)) {
			master_fsm_test(("func:%s(), line:%d\n", __FUNCTION__, __LINE__));
			get_ezmacproc_fsm_state();
		}

//		master_fsm_test(("func:%s(), line:%d\n", __FUNCTION__, __LINE__));

		recv_pkt_from_ezmacpro(recv_buf, sizeof(recv_buf), &payload_len, &rssi);
		master_fsm_test(("func:%s(), line:%d, recv 1st byte:0x%x, recv len:%d\n", __FUNCTION__, __LINE__,
				recv_buf[0], payload_len));
		break;

	case 2:
		master_fsm_info(("TIMEOUT_XTAL_START:%d, TIMEOUT_LBTI_ETSI:%d, TimeoutLBTI:%d, TimeoutChannelSearch:%d\n"
				"TimeoutSyncWord:%d, TimeoutTX_Packet:%d, TimeoutRX_Packet:%d, TimeoutACK:%d\n"
				"mpl:%d\n",
				TIMEOUT_XTAL_START, TIMEOUT_LBTI_ETSI, TimeoutLBTI, TimeoutChannelSearch,
				TimeoutSyncWord, TimeoutTX_Packet, TimeoutRX_Packet, TimeoutACK,
				EZMacProReg.name.MPL));
		break;

	case 3:
		temp1 = macSpiReadReg(SI4432_TX_DATA_RATE_1);
		temp1 <<= 8;

		temp1 |= macSpiReadReg(SI4432_TX_DATA_RATE_0);

		temp2 = macSpiReadReg(SI4432_MODULATION_MODE_CONTROL_1);

		master_fsm_test(("data rate:%d(0x%x), temp2:0x%x\n", temp1, temp1, temp2));

		rolling_over_port_pin(led1_gpio, led1_pin);
		rolling_over_port_pin(led2_gpio, led2_pin);
		rolling_over_port_pin(led3_gpio, led3_pin);
		rolling_over_port_pin(led4_gpio, led4_pin);
		rolling_over_port_pin(led5_gpio, led5_pin);

		break;

	case 4:
	{
		UU32 time;
		unsigned long t = 0x12345678;

		time.U32 = 70805;
		master_fsm_test(("msb:0x%x, lsb:0x%x, 0x%x\n", time.U16[MSB], time.U16[LSB], *((unsigned char *)&t)));


		macTimeout(70805);
		disable_si4432_exti();
		enable_si4432_mac_timer_int();

		rolling_over_port_pin(led3_gpio, led3_pin);

	}
		break;

	default:
		break;
	}

	return;
}
FINSH_FUNCTION_EXPORT(mfsm_test, "master fsm test");
#endif
