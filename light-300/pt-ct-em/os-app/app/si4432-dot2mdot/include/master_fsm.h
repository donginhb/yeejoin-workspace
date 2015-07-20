/*
 * master_fsm.h
 *
 *  Created on: 2013-12-6
 *      Author: David, zhaoshaowei@yeejoin.com
 */

#ifndef MASTER_FSM_H_
#define MASTER_FSM_H_

#include <EZMacPro_cfg.h>
#include <compiler_defs.h>

#include <ms_common.h>

#include <rtthread.h>
#include <syscfgdata.h>


#define MASTER_SLAVE_PAIRS_MAX	(1)

#if 0
enum master_fsm_state_e {
	MFS_NONE = 0,
	MFS_NONE	,
	MFS_LISTEN_MODE,

	MFS_SEND_ECHO_REQUEST	,
	MFS_WAIT_ECHO_RESPONSE,

	MFS_WAIT_PUSH_CMD,	/* 用于等待无线从节点主动发起的无线通信过程 */
	MFS_SEND_PUSH_CMD_REP,

//	MFS_SEND_GET_CMD,	/* 用于无线主节点主动发起的无线通信过程 */
//	MFS_WAIT_GET_CMD_REP,

	MFS_SEND_PROBE_SLAVE_NODE,	/* 发送探测可链接从节点命令 */
	MFS_WAIT_PROBE_SLAVE_NODE_REP,	/* 等待探测可链接从节点命令的响应 */

	MFS_SEND_CFGDATA2SLAVE_NODE,	/* 下发配置数据给从节点 */
	MFS_WAIT_CFGDATA2SLAVE_NODE_REP,	/* 等待下发配置数据命令响应 */
};
#else
enum master_fsm_state_e {
	MFS_NONE	= 0,

	MFS_WAIT_PROBE_SLAVE_CMD_FROM_485BUS,
	MFS_WAIT_PROBE_SLAVE_CMD_REP,

	MFS_WAIT_WIRELESS_NETCFG_FROM_485BUS,
	MFS_DISTRIBUTE_SLAVE_NODE_CFG_DATA,
	MFS_WAIT_DISTRIBUTE_SLAVE_NODE_CFG_DATA_REP,

	MFS_LISTEN_MODE,

	MFS_COLLECT_DATA_REP,/* mark by zp */

	MFS_SEND_ECHO_REQUEST,
	MFS_RECV_ECHO_REQUEST_REP,

	MFS_BUTT
};
#endif
#if C51_SYNTAX_
typedef base_t mfsm_state_t;
#else
typedef enum master_fsm_state_e mfsm_state_t;
#endif

struct wireless_cfgdata_use4tx_st {
	uint8_t sn[DEV_SN_MODE_LEN];
	uint8_t slave_id;		/* 用于主节点响应注册请求时, 返回节点id */
	uint8_t channel_no;
	uint8_t syn_word[2];
	uint8_t cid;
};

struct master_app_fsm_info_st {
	mfsm_state_t master_fsm_state;		/* 主调函数只需初始化 */
	enum si4432_transport_data_cmd_e cmd;
	rt_tick_t multi_rep_start_time;
	uint8_t *tx_buf;
	uint8_t *rx_buf;
	union {
		uint8_t unpkt_buf[RF_RX_BUFFER_SIZE];
		struct wireless_cfgdata_use4tx_st wl_cfgdata;
	} buf;

	struct {
		uint8_t tx_cnt;	/**/
		uint8_t rx_cnt;
	} tx_rx_cnt;

	uint8_t echo_cnt;
	uint8_t echo_did;
};

struct register_slave_node_info_st {
	uint8_t sn[DEV_SN_MODE_LEN];
	uint8_t rssi;
	uint8_t pad[3];
	rt_tick_t last_update_time;
};

struct register_slave_node_original_info_st {/* mark by zp */
	uint8_t sn[DEV_SN_MODE_LEN];
	uint8_t rssi;
	uint8_t pad[3];
};

#define WAIT_PROBE_REP_TIME_S_MAX	(10)
#define WAIT_PROBE_REP_TIME_TICKS_MAX	(get_ticks_of_ms((WAIT_PROBE_REP_TIME_S_MAX)*1000))

#define DISTRIBUTE_WIRELESS_CFGDATA_CNT_MAX	(2)
#define DISTRIBUTE_WIRELESS_CFGDATA_CNT_REP_MAX (2)

extern rt_timer_t  ms_echo_timer;


extern volatile uint8_t master_slave_doing_echo;
extern volatile uint8_t master_slave_echo_timeout_cnt;

extern struct register_slave_node_info_st register_slave_node_info[WIRELESS_SLAVE_NODE_MAX];

extern int master_init(void);

extern base_t init_master_app_fsm_info_st(struct master_app_fsm_info_st *p);
extern int master_node_app_fsm(struct master_app_fsm_info_st *mfsm_info);

#endif /* MASTER_FSM_H_ */
