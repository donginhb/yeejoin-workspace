/*
 * master_app.h
 *
 *  Created on: 2013-12-2
 *      Author: David, zhaoshaowei@yeejoin.com
 */

#ifndef MASTER_API_H_
#define MASTER_API_H_

#include <compiler_defs.h>

#include <ms_common.h>

#include <sink_info.h>


#define MASTER_ECHO_STATISTICS 1

#if MASTER_ECHO_STATISTICS
extern ubase_t echo_rx_succ, echo_rx_fail;
extern ubase_t echo_tx_succ, echo_tx_fail;
#endif


extern volatile ubase_t master_node_cmd_vector;

extern int send_cmd_to_4432(enum sinkinfo_cmd_e cmd, enum sinkinfo_dev_type_e dev_type);
extern enum sinkinfo_error_e get_si4432_sinkinfo(enum sinkinfo_cmd_e cmd, struct sink_info_msg *send_rep);

extern ms_data_proc_err_t do_master_proc_push_cmd_data(struct master_slave_data_frame_head_s *h, uint8_t *recv_data,
		struct sid_did_pairs_t *sdid);
extern int do_master_proc_push_cmd_rep_data(struct master_slave_data_frame_head_s *h, uint8_t *rx_data,
		uint8_t *send_mac_data, ubase_t *send_mac_data_len, ms_data_proc_err_t result);

#endif /* MASTER_API_H_ */
