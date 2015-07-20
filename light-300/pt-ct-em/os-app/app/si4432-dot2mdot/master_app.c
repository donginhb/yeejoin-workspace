/*
 * master_app.c
 *
 *  Created on: 2013-12-2
 *      Author: David, zhaoshaowei@yeejoin.com
 */


#include <rtdef.h>
#include <app-thread.h>

#include <master_fsm.h>
#include <master_app.h>
#include <ms_common.h>

#include <sys_cfg_api.h>

#include <rs485.h>/* mark by zp */

#define master_app_debug(x)	printf_syn x
#define master_app_log(x)	printf_syn x
#define master_app_info(x)	printf_syn x

#if MASTER_ECHO_STATISTICS
ubase_t echo_rx_succ, echo_rx_fail;
ubase_t echo_tx_succ, echo_tx_fail;
#endif

struct rt_semaphore si4432_had_recv_data_sem;

rt_sem_t    probe_timeout_sem;


static uint8_t *master_tx_buf[1];
static uint8_t *master_rx_buf[1];
static struct master_app_fsm_info_st mfsm_info;


static void rt_4432master_entry(void* parameter);
static void ms_echo_timeout(void *param);

int creat_4432master_th(void)
{
	rt_thread_t thread_h;
	int cnt = 0;

	master_tx_buf[0] = rt_malloc(RF_TX_BUFFER_SIZE);
	if (NULL == master_tx_buf[0]) {
		master_app_log(("rf master alloc tx buf fail\n"));
		return FAIL;
	}

	master_rx_buf[0] = rt_malloc(RF_RX_BUFFER_SIZE);
	if (NULL == master_rx_buf[0]) {
		master_app_log(("rf master alloc rx buf fail\n"));
		rt_free(master_tx_buf[0]);
		return FAIL;
	}

	while (cnt < 3 && SUCC!=master_init()) {
		master_app_log(("rf master init fail.\n"));
		++cnt;
		//goto err_ret;
	}

	if (cnt >= 3)
		goto err_ret;

	ms_echo_timer = rt_timer_create("ms-echo", ms_echo_timeout, NULL, get_ticks_of_ms(1000), RT_TIMER_FLAG_PERIODIC);
	if (NULL == ms_echo_timer) {
		master_app_log(("rf creat echo timer fail.\n"));
		goto err_ret;
	}

	si4432_recv_mb = rt_mb_create("si_recv", 1, RT_IPC_FLAG_FIFO);/* mark by zp */
	if (NULL == si4432_recv_mb) {
		master_app_log(("rf master alloc si4432_recv_mb fail\n"));
		rt_timer_delete(ms_echo_timer);
		goto err_ret;
	}

	probe_timeout_sem = rt_sem_create("pto_sem", 0, RT_IPC_FLAG_FIFO);
	if (NULL == probe_timeout_sem) {
		master_app_log(("rf master create probe_timeout_sem fail\n"));
		rt_timer_delete(ms_echo_timer);
		rt_mb_delete(si4432_recv_mb);
		goto err_ret;

	}
	
	thread_h = rt_thread_create("rfmaster", rt_4432master_entry, RT_NULL, 2048, THREAD_PRIORITY_4432MASTER, 10);
	if (thread_h != RT_NULL) {
		rt_thread_startup(thread_h);
	} else {
		master_app_log(("create rt_4432master_entry fail\n"));
		rt_timer_delete(ms_echo_timer);
		rt_mb_delete(si4432_recv_mb);
		rt_sem_delete(probe_timeout_sem);
		master_app_log(("create rt_4432master_entry fail\n"));
		goto err_ret;
	}

	return SUCC;

err_ret:
	rt_free(master_tx_buf[0]);
	rt_free(master_rx_buf[0]);
	return FAIL;
}


int send_cmd_to_4432(enum sinkinfo_cmd_e cmd, enum sinkinfo_dev_type_e dev_type)
{
	master_app_debug(("func:%s, line:%d, cmd:%d, dev_type:%d\n", __FUNCTION__, __LINE__, cmd, dev_type));

	//demo_su_req_tx(SU_DT,SU_CMD); /* mark by David */
	return 0;
}

enum sinkinfo_error_e get_si4432_sinkinfo(enum sinkinfo_cmd_e cmd, struct sink_info_msg *send_rep)
{
	enum sinkinfo_error_e err_code = SIE_OK;

	master_app_debug(("func:%s, line:%d\n", __FUNCTION__, __LINE__));


	return err_code;
}

#if 0
ms_data_proc_err_t do_master_proc_push_cmd_data(struct master_slave_data_frame_head_s *h, uint8_t *recv_data,
		struct sid_did_pairs_t *sdid)
{
	void *pdata;

	if (NULL==h || NULL==recv_data || NULL==sdid) {
		master_app_log(("func:%s, line:%d, param error\n", __FUNCTION__, __LINE__));
		return MSDPE_ERROR;
	}

	master_app_debug(("func:%s, cmd:%d, did:%d, sid:%d, len:%d\n", __FUNCTION__, h->cmd, sdid->did, sdid->sid, h->len));

	master_app_debug(("func:%s, 0x%x, 0x%x, 0x%x, 0x%x\n", __FUNCTION__, recv_data[0], recv_data[1],
			recv_data[2], recv_data[3]));

	pdata = recv_data;

	switch (h->cmd) {
	case MNP_PUSH_COLLECTION_DATA:
		master_app_debug(("func:%s, line:%d, app:0x%x, 0x%x, 0x%x, 0x%x\n", __FUNCTION__, __LINE__,
				ms_ntohl(convert_vptr(pdata, struct msfd_report_collection_data_st*)->pt_ct_app_p),
				ms_ntohl(convert_vptr(pdata, struct msfd_report_collection_data_st*)->vi.pt_v[0]),
				ms_ntohl(convert_vptr(pdata, struct msfd_report_collection_data_st*)->vi.pt_v[1]),
				ms_ntohl(convert_vptr(pdata, struct msfd_report_collection_data_st*)->vi.pt_v[2])
				                                                             ));
		break;

	case MNP_PUSH_STATE_INFO     :
		break;


	default:
		break;
	}

	return MSDPE_OK;

}
#endif


static void rt_4432master_entry(void* parameter)
{
	say_thread_start();

	init_master_app_fsm_info_st(&mfsm_info);
	mfsm_info.tx_buf = master_tx_buf[0];
	mfsm_info.rx_buf = master_rx_buf[0];

	while (1) {
		master_node_app_fsm(&mfsm_info);
		//rt_thread_delay(get_ticks_of_ms(30));/* mark by zp */
	} /* while(1) */

}


static void ms_echo_timeout(void *param)
{
	(void)param;

	++master_slave_echo_timeout_cnt;
	return;
}


#if 1
#include <finsh.h>
#include <si4432_v2.h>
#include <spi4ezmacpro.h>

#include <compiler_defs.h>
#include <board.h>

#define master_app_test(x) 	printf_syn x


#if 1
extern volatile base_t is_recv_probe_from_485;
extern volatile base_t is_recv_wlnet_cfg_from_485_over;
#endif


unsigned char recv_buf[64];

extern void print_si4432_state(void);

void mapp_test(int cmd, int sub)
{
	uint8_t sn[DEV_SN_MODE_LEN];
	struct msfd_distribute_cfgdata_st wl_cfgdata;
	int i;

	switch (cmd) {
	case 0:
		master_slave_doing_echo = MASTER_SLAVE_ECHO_START;
		mfsm_info.echo_did 	= convert_usr_scope_id_to_ezmac_id(1);
		break;

	case 1:
		switch (sub) {
		case 1:
			is_recv_probe_from_485 = 1;
			rt_mb_send(uart485_mb, ESTP_PROBE_SLAVE_CMD);/* mark by zp */
			break;

		case 2:
			register_slave_node_info[1].sn[0] = 0x33;
			register_slave_node_info[1].sn[1] = 0x33;
			register_slave_node_info[1].sn[2] = 0x33;
			register_slave_node_info[1].sn[3] = 0x33;
			register_slave_node_info[1].sn[4] = 0x33;
			register_slave_node_info[1].sn[5] = 0x33;
			is_recv_wlnet_cfg_from_485_over = 2;
			rt_mb_send(uart485_mb, ESTP_NETCFG_CMD);/* mark by zp */
			break;

		case 3:
			get_wireless_cfgdata(&wl_cfgdata);

			printf_syn("====wireless cfg data====\nSN:");
			print_dev_sn(wl_cfgdata.sn, sizeof(wl_cfgdata.sn));
			printf_syn("ch-no:%d, cid:%d, syn-word:0x%2x%2x\n",wl_cfgdata.channel_no,
					wl_cfgdata.cid, wl_cfgdata.syn_word[0], wl_cfgdata.syn_word[1]);
			break;

		case 4:
			for (i=0; i<sizeof(register_slave_node_info)/sizeof(register_slave_node_info[0]); ++i) {
				if (is_dev_sn_valid(register_slave_node_info[i].sn, sizeof(register_slave_node_info[0].sn)))
					printf_syn("[index %2d]: rssi:%3d, sn:%s\n",
							i, register_slave_node_info[i].rssi,
							register_slave_node_info[i].sn);
			}

			break;

		default:
			printf_syn("func:%s, line:%d, param error, cmd:%d, sub:%d\n", __FUNCTION__, __LINE__, cmd, sub);
			break;
		}

		break;

	case 2:
		print_callback_var();
		print_si4432_state();
		printf_syn("state:%d, cmd:%d\n", mfsm_info.master_fsm_state, mfsm_info.cmd);
		break;

	case 4:
#if MASTER_ECHO_STATISTICS
		master_app_info(("echo_rx_succ:%d, echo_rx_fail:%d, echo_tx_succ:%d, echo_tx_fail:%d\n",
				echo_rx_succ, echo_rx_fail, echo_tx_succ, echo_tx_fail));
#endif
		break;

	case 5:
		printf_syn("master fsm is %d\n", mfsm_info.master_fsm_state);
		break;

	case 6:
		get_self_sn(sn, sizeof(sn));
		printf_syn("self sn:");
		print_dev_sn(sn, sizeof(sn));
		break;

	default:
		break;
	}

	return;
}
FINSH_FUNCTION_EXPORT(mapp_test, "master app test");
#endif
