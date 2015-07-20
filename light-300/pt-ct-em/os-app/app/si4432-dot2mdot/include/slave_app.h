/*
 ******************************************************************************
 * slave_app.h
 *
 *  Created on: 2013-12-19
 *      Author: David, zhaoshaowei@yeejoin.com
 *
 * COPYRIGHT (C) 2013, YEEJOIN (Beijing) Technology Development Co., Ltd.
 ******************************************************************************
 */

#ifndef SLAVE_APP_H_
#define SLAVE_APP_H_

#define SLAVE_ECHO_STATISTICS 1

#if SLAVE_ECHO_STATISTICS
extern ubase_t echo_rx_succ, echo_rx_fail;
extern ubase_t echo_tx_succ, echo_tx_fail;
#endif


extern ms_data_proc_err_t do_slave_proc_push_cmd_rep_data(struct master_slave_data_frame_head_s *h, uint8_t *recv_data,
		struct sid_did_pairs_t *sdid);

extern int do_slave_proc_push_cmd_data(struct master_slave_data_frame_head_s *h, uint8_t *send_mac_data,
		ubase_t *send_mac_data_len);

#endif /* SLAVE_APP_H_ */
