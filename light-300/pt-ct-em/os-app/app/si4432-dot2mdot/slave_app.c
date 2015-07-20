/*
 ******************************************************************************
 * slave_app.c
 *
 *  Created on: 2013-12-19
 *      Author: David, zhaoshaowei@yeejoin.com
 *
 * COPYRIGHT (C) 2013, YEEJOIN (Beijing) Technology Development Co., Ltd.
 ******************************************************************************
 */


#include <rtdef.h>
#include <rtthread.h>
#include <app-thread.h>

#include <ms_common.h>
#include <slave_fsm.h>
#include <slave_app.h>

#include <sys_cfg_api.h>

#define slave_app_debug(x)	printf_syn x
#define slave_app_log(x)	printf_syn x
#define slave_app_info(x)	printf_syn x

//#define SEND_INTERVAL		get_ticks_of_ms(2000)
#define SEND_INTERVAL		6000

#if SLAVE_ECHO_STATISTICS
ubase_t echo_rx_succ, echo_rx_fail;
ubase_t echo_tx_succ, echo_tx_fail;
#endif

rt_timer_t sscd_timer;/* send slave collect data */
rt_sem_t sscd_sem;

static void rt_4432slave_entry(void* parameter);
void sscd_timeout(void *param);


int creat_4432slave_th(void)
{
	rt_thread_t thread_h;
	int cnt = 0;

	slave_tx_buf = rt_malloc(RF_TX_BUFFER_SIZE);
	if (NULL == slave_tx_buf) {
		slave_app_log(("rf master alloc tx buf fail\n"));
		return FAIL;
	}

	slave_rx_buf = rt_malloc(RF_RX_BUFFER_SIZE);
	if (NULL == slave_rx_buf) {
		slave_app_log(("rf master alloc rx buf fail\n"));
		rt_free(slave_tx_buf);
		return FAIL;
	}

	while (cnt<3 && SUCC != slave_init()) {
		slave_app_log(("rf slave init fail.\n"));
		++cnt;
		//goto err_ret;
	}

	if (cnt >= 3)
		goto err_ret;

	si4432_recv_mb = rt_mb_create("si_recv", 1, RT_IPC_FLAG_FIFO);/* mark by zp */
	if (NULL == si4432_recv_mb) {
		slave_app_log(("rf slave alloc si4432_recv_mb fail.\n"));
		goto err_ret;
	}

	sscd_timer = rt_timer_create("sscd_t", sscd_timeout, RT_NULL, SEND_INTERVAL, RT_TIMER_FLAG_ONE_SHOT);
	if (NULL == sscd_timer) {
		slave_app_log(("rf slave alloc sscd_timer fail.\n"));
		rt_mb_delete(si4432_recv_mb);
		goto err_ret;
	}
	
	sscd_sem = rt_sem_create("sscd_s", 0, RT_IPC_FLAG_PRIO);
	if (NULL == sscd_sem) {
		slave_app_log(("rf slave alloc sscd_sem fail.\n"));
		rt_mb_delete(si4432_recv_mb);
		rt_timer_delete(sscd_timer);
		goto err_ret;
	}

	thread_h = rt_thread_create("rfslave", rt_4432slave_entry, RT_NULL, 1024, THREAD_PRIORITY_4432SLAVE, 10);
	if (thread_h != RT_NULL) {
		rt_thread_startup(thread_h);
	} else {
		slave_app_log(("create rt_4432slave_entry fail\n"));
		rt_mb_delete(si4432_recv_mb);
		rt_timer_delete(sscd_timer);
		rt_sem_delete(sscd_sem);
		goto err_ret;
	}

	return SUCC;
err_ret:
	rt_free(slave_tx_buf);
	rt_free(slave_rx_buf);
	return FAIL;
}

void sscd_timeout(void *param)
{
	(void)param;
	rt_sem_release(sscd_sem);
}

#if 0

/*
 * 根据不同状态准备数据, 并打包
 * */

int do_slave_proc_push_cmd_data(struct master_slave_data_frame_head_s *h, uint8_t *send_mac_data,
		ubase_t *send_mac_data_len)
{
	base_t tx_len;
	uint8_t temp_buf[20];
	void *pdata;

	if (NULL==h || NULL==send_mac_data || NULL==send_mac_data_len) {
		slave_app_log(("func:%s, line:%d, param error\n", __FUNCTION__, __LINE__));
		return FAIL;
	}

	tx_len = 0;
	pdata = temp_buf;
	switch (get_slave_cmd()) {
	case MNP_PUSH_COLLECTION_DATA:
		h->cmd = MNP_PUSH_COLLECTION_DATA;

		convert_vptr(pdata, struct msfd_report_collection_data_st*)->pt_ct_app_p = (s32_t)ms_htonl(0x12345678);
		convert_vptr(pdata, struct msfd_report_collection_data_st*)->vi.pt_v[0] = (s32_t)ms_htonl(0x56781234);
		convert_vptr(pdata, struct msfd_report_collection_data_st*)->vi.pt_v[1] = (s32_t)ms_htonl(0x66781234);
		convert_vptr(pdata, struct msfd_report_collection_data_st*)->vi.pt_v[2] = (s32_t)ms_htonl(0x76781234);
		tx_len += sizeof(struct msfd_report_collection_data_st);
		break;

	case MNP_PUSH_STATE_INFO     :
		h->cmd = MNP_PUSH_STATE_INFO;
		break;

	default:
		break;
	}


	h->len = tx_len;
	*send_mac_data_len = tx_len + MASTER_SLAVE_DATA_FREAM_HEAD_LEN;

	package_master_slave_data(send_mac_data, *send_mac_data_len, h, temp_buf, tx_len);

	slave_app_debug(("func:%s, line:%d, h->cmd:%d, h->len:%d\n", __FUNCTION__, __LINE__, h->cmd, h->len));

	return SUCC;
}
#endif

static struct slave_app_fsm_info_st sfsm_info;

static void rt_4432slave_entry(void* parameter)
{
	say_thread_start();

	init_slave_app_fsm_info_st(&sfsm_info);
	sfsm_info.tx_buf = slave_tx_buf;
	sfsm_info.rx_buf = slave_rx_buf;

	while (1) {
		slave_node_app_fsm(&sfsm_info);

		//rt_thread_delay(get_ticks_of_ms(30));/* mark by zp */
	} /* while(1) */

}


#if 1
#include <finsh.h>
#include <si4432_v2.h>
#include <spi4ezmacpro.h>

#include <compiler_defs.h>
#include <board.h>

#define slave_app_test(x) 	printf_syn x


//unsigned char recv_buf[64];
extern void print_si4432_state(void);


void sapp_test(int cmd, int sub)
{
	uint8_t sn[DEV_SN_MODE_LEN];
	struct msfd_distribute_cfgdata_st wl_cfgdata;

	switch (cmd) {
	case 0:
		set_slave_cmd(SFS_IDLE);
		set_slave_next_state(&sfsm_info, SNS_ECHO_MODE);
		break;

	case 1:
		switch (sub) {
		case 1:
			printf_syn("sfsm--- state:%d, next state:%d\n", sfsm_info.s_state.slave_node_fsm_state,
					sfsm_info.s_state.slave_next_state);
			break;

		case 2:/* mark by zp */
			set_slave_next_state(&sfsm_info, SNS_PUSH_CMD_MODE);
			break;

		case 3:
			get_wireless_cfgdata(&wl_cfgdata);

			printf_syn("====wireless cfg data====\nSN:");
			print_dev_sn(wl_cfgdata.sn, sizeof(wl_cfgdata.sn));
			printf_syn("ch-no:%d, cid:%d, syn-word:0x%2x%2x\n",wl_cfgdata.channel_no,
					wl_cfgdata.cid, wl_cfgdata.syn_word[0], wl_cfgdata.syn_word[1]);
			break;

		default:
			break;
		}
		break;

	case 2:
		print_callback_var();
		print_si4432_state();
		printf_syn("state:%d, next-state:%d\n",
				sfsm_info.s_state.slave_node_fsm_state, sfsm_info.s_state.slave_next_state);
		break;

	case 3:
		switch (sub) {
		case 0:
			slave_app_info(("into recv %s\n", SUCC==recv_pkt_from_ezmacpro_prepare() ? "succ" : "fail"));
			break;

		case 1:
			set_slave_cmd(MNP_PUSH_COLLECTION_DATA);
			set_slave_next_state(&sfsm_info, SNS_PUSH_CMD_MODE);
			break;

		case 2:
			set_ezmac_into_idle_state();
			break;

		default:
			break;
		}
		break;

	case 4:
#if SLAVE_ECHO_STATISTICS
		slave_app_info(("echo_rx_succ:%d, echo_rx_fail:%d, echo_tx_succ:%d, echo_tx_fail:%d\n",
				echo_rx_succ, echo_rx_fail, echo_tx_succ, echo_tx_fail));
#endif
		break;

	case 5:
		printf_syn("slave fsm is %d,%d\n",sfsm_info.s_state.slave_node_fsm_state, sfsm_info.s_state.slave_next_state);
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
FINSH_FUNCTION_EXPORT(sapp_test, "slave app test");
#endif
