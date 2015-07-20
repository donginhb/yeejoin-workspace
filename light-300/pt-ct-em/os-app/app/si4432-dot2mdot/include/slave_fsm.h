/*
 * slave_fsm.h
 *
 *  Created on: 2013-12-9
 *      Author: David, zhaoshaowei@yeejoin.com
 */

#ifndef SLAVE_FSM_H_
#define SLAVE_FSM_H_

#include <EZMacPro_cfg.h>

#include <ms_common.h>

enum slave_fsm_state_e {
	SFS_NONE = 0,
	SFS_IDLE	,
	SFS_INTO_IDLE_STATE,

	SFS_WAIT_PROBE_SLAVE_NODE,	/* 等待探测可链接从节点命令 */
	SFS_SEND_PROBE_SLAVE_NODE_REP,	/* 发送探测可链接从节点命令的响应 */

	SFS_WAIT_CFGDATA2SLAVE_NODE,	/* 等待配置数据给从节点命令 */
	SFS_SEND_CFGDATA2SLAVE_NODE_REP,	/* 发送配置数据命令响应 */

	SFS_SEND_PUSH_CMD,
	SFS_WAIT_PUSH_CMD_REP,

	SFS_WAIT_ECHO_REQUEST	,
	SFS_SEND_ECHO_RESPONSE,

//	SFS_WAIT_GET_CMD,
//	SFS_SEND_GET_CMD_REP,

	SFS_BUTT
};

#if C51_SYNTAX_
typedef base_t sfsm_state_t;
#else
typedef enum slave_fsm_state_e sfsm_state_t;
#endif

struct slave_fsm_state_st {
	sfsm_state_t slave_node_fsm_state;
	slave_next_state_t slave_next_state;
};

struct slave_app_fsm_info_st {
	struct slave_fsm_state_st s_state;
	uint8_t echo_cnt;

	uint8_t *tx_buf;
	uint8_t *rx_buf;

	union {
		uint8_t unpkt_buf[RF_RX_BUFFER_SIZE];
	} buf;
};

extern base_t slave_master_doing_echo;

extern uint8_t *slave_tx_buf;
extern uint8_t *slave_rx_buf;
extern uint8_t slave_master_echo_cnt;


extern int slave_init(void);
extern int slave_node_app_fsm(struct slave_app_fsm_info_st *sfsm_info);

extern int set_slave_cmd(master_slave_cmd_t scmd);
extern master_slave_cmd_t get_slave_cmd(void);

extern int set_slave_next_state(struct slave_app_fsm_info_st *sfsm_info, slave_next_state_t s);

extern ubase_t get_slave_self_mac_id(void);

extern base_t init_slave_app_fsm_info_st(struct slave_app_fsm_info_st *p);


#endif /* SLAVE_FSM_H_ */
