/**********************************************************************************
* Filename   : frame_485.c
* Discription : define the fucntions that read the 485 and send the frame to ems
* Begin time  : 2014-2-27
* Finish time : 
* Engineer    :  creat by fanfuchao, fanfuchao@yeejoin.com
* Version      : V1.0
*************************************************************************************/
#include <rtthread.h>
#include <board.h>

#include "rtc.h"
#include <misc_lib.h>
#include <rs485.h>
#include <frame_em.h>
#include <sys_cfg_api.h>

#ifdef RT_USING_FINSH
#include <finsh.h>
#endif

#define frame_master_info(x) printf_syn x


#define RS485_CMD_MB_MAX_LEN  1

rt_mailbox_t wl_master_data_over485_mb;
rt_sem_t     recv_wireless_master_probe_rep_sem;
rt_sem_t     recv_wireless_master_netcfg_rep_sem;
rt_sem_t     recv_wireless_master_stop_probe_rep_sem;
rt_sem_t     recv_wireless_master_data_rep_sem;
rt_sem_t     tcp_send_sem;

rt_uint8_t dest_sn[DEV_SN_MODE_LEN];
rt_uint8_t echo_id_m = 0;
rt_uint32_t probe_node_num;

struct master_slave_original_pairs_table_st *ms_original_pairs_table;
struct master_slave_proc_pairs_table_st *ms_proc_pairs_table;
struct msfd_report_collection_data_sn_st *pc_collect_data_m;

extern rt_uint8_t *rs485_send_buf_m;
extern rt_uint8_t *rs485_recv_buf;


extern USART_TypeDef *get_485_port_ptr_from_uartno(enum frame_uart_485 port);
extern void si_set_is_had_finish_wl_netcfg_flag(int is_had_finish);

void rs485_send_probe_cmd(rt_uint8_t *tx_buf, rt_uint8_t *dest_addr, struct rs485_frame_format *format, enum rs485_frame_ctrl ctrl);
static void rs485_send_netcfg_cmd(rt_uint8_t *tx_buf, rt_uint8_t *dest_addr, rt_uint8_t *data, rt_uint8_t data_len,
				struct rs485_frame_format *format);
void rs485_send_read_master_data_cmd(rt_uint8_t *tx_buf, rt_uint8_t *dest_addr, struct rs485_frame_format *format, enum rs485_frame_ctrl ctrl);

static void rs485_send_echo_cmd(rt_uint8_t *tx_buf, rt_uint8_t *dest_addr, rt_uint8_t *echo, struct rs485_frame_format *format);

static int check_sn_index(const void *cs, const void *ct, int len, int times);

static void rs485_send_stop_echo_cmd(rt_uint8_t *tx_buf, rt_uint8_t *dest_addr, struct rs485_frame_format *format);

static void rs485_send_stop_probe_cmd(rt_uint8_t *tx_buf, rt_uint8_t *dest_addr, struct rs485_frame_format *format);

static int is_em_pt_table_empty(struct wl_master_485_slave_info_tbl *table);

static int is_ms_proc_pairs_table_empty(struct master_slave_proc_pairs_table_st *table);

static int get_ms_proc_pairs_table_num(struct master_slave_proc_pairs_table_st *table);
static int init_wl_master_485_slave_info_tbl(void);
static void auto_distribute_net_by_rssi(struct master_slave_original_pairs_table_st *original,
					struct master_slave_proc_pairs_table_st *proc);

void send_read(void)
{
	int ret;

	ret = rt_mb_send(wl_master_data_over485_mb, MD_SEND_READ_MASTER_DATA_CMD);
	if (RT_EOK != ret) {
		printf_syn("send rs485_cmd_mb fail\n");
	}
}
#ifdef RT_USING_FINSH
FINSH_FUNCTION_EXPORT(send_read,"send read cmd");
#endif

void auto_cmd(int sum)
{
	int ret;
	
	probe_node_num = sum;
	ret = rt_mb_send(wl_master_data_over485_mb, MD_SEND_PROBE_CMD);
	if (RT_EOK != ret) {
		printf_syn("send rs485_cmd_mb fail\n");
	}
}
#ifdef RT_USING_FINSH
FINSH_FUNCTION_EXPORT(auto_cmd,"auto networking cmd");
#endif

static int proc_send_probe(struct wl_master_485_slave_info_tbl *table, struct rs485_frame_format *format)
{
	rt_uint8_t temp;
	int i;

	if (TRUE == is_em_pt_table_empty(table)) {
		frame_master_info(("em_pt_table is empty!\n"));
		return FAIL;
	}
	for (i=0; i<WIRELESS_MASTER_NODE_MAX; i++) {
		if (is_dev_sn_valid(table->pt_sn[i], sizeof(table->pt_sn[0]))) {
			rt_memcpy(dest_sn, table->pt_sn[i], DEV_SN_MODE_LEN);
			rt_memset(ms_original_pairs_table[i].slave_node_info, 0, 
						sizeof(ms_original_pairs_table[i].slave_node_info));
			rs485_send_probe_cmd(rs485_send_buf_m, table->pt_sn[i], format, MD_SEND_PROBE_CMD);
			if (RT_EOK != rt_sem_take(recv_wireless_master_probe_rep_sem, 1000)) {
				frame_master_info(("\n*************************\n"));
				frame_master_info(("probe fail, rs485 slave sn:"));
				print_dev_sn(table->pt_sn[i], sizeof(table->pt_sn[0]));
				frame_master_info(("*************************\n"));
				return FAIL;
			}
		}
	}
	rt_memset(ms_proc_pairs_table, 0, sizeof(struct master_slave_proc_pairs_table_st)*SLAVE_485_MAX_NUM);
	/* ms_original_pairs_table to ms_proc_pairs_table */
	auto_distribute_net_by_rssi(ms_original_pairs_table, ms_proc_pairs_table);
	temp = get_ms_proc_pairs_table_num(ms_proc_pairs_table);
	if ((temp > 0) && (probe_node_num <= get_ms_proc_pairs_table_num(ms_proc_pairs_table))) {
		frame_master_info(("*************************\n"));
		frame_master_info(("probe slave node complete\n"));
		frame_master_info(("*************************\n"));
		return SUCC;
	} else {
		frame_master_info(("****************************\n"));
		frame_master_info(("probe slave node uncomplete\n"));
		frame_master_info(("****************************\n"));
		return FAIL;
	}
}

static int proc_send_netcfg(struct wl_master_485_slave_info_tbl *table, struct rs485_frame_format *format)
{
	rt_uint8_t is_complete = 1;
	int i;

	if (TRUE == is_em_pt_table_empty(table)) {
		frame_master_info(("em_pt_table is empty!\n"));
		return FAIL;
	}
	
	for (i=0;i<WIRELESS_MASTER_NODE_MAX;i++) {
		int j;
		
		if (TRUE == is_ms_proc_pairs_table_empty(ms_proc_pairs_table + i)) {
			frame_master_info(("ms_proc_pairs_table[%d] is empty!\n", i));
			continue;
		}

		for (j=0;j<SLAVE_NODE_INFO_MAX_LEN;j++) {
			if(!is_dev_sn_valid(ms_proc_pairs_table[i].slave_node_info[j].sn, DEV_SN_MODE_LEN))
				break;
		}
		if (j > 0) {
			rs485_send_netcfg_cmd(rs485_send_buf_m, ms_proc_pairs_table[i].src_sn, (rt_uint8_t *)ms_proc_pairs_table[i].slave_node_info, 
					sizeof(ms_proc_pairs_table[0].slave_node_info[0])*j, format);
			frame_master_info(("send netcfg cmd\n"));

			if (RT_EOK != rt_sem_take(recv_wireless_master_netcfg_rep_sem, 500)) {
				is_complete = 0;
				frame_master_info(("@@@@@@@@@@@@@@@@@@@@@@@@@@\n"));
				frame_master_info(("netcfg fail, rs485 slave sn:"));
				print_dev_sn(ms_proc_pairs_table[i].src_sn, sizeof(ms_proc_pairs_table[i].src_sn));
				frame_master_info(("@@@@@@@@@@@@@@@@@@@@@@@@@@\n"));
				//return FAIL;
			}
		}
	}
	if (is_complete) {
		frame_master_info(("***************\n"));
		frame_master_info(("netcfg complete\n"));
		frame_master_info(("***************\n"));

		set_wl_netcfg_finish_flag(TRUE);
		si_set_is_had_finish_wl_netcfg_flag(TRUE);
		return SUCC;
	} else {
		is_complete = 1;
		frame_master_info(("***************\n"));
		frame_master_info(("netcfg uncomplete\n"));
		frame_master_info(("***************\n"));
		return FAIL;
	}
}

static int proc_send_read_data(struct wl_master_485_slave_info_tbl *table, struct rs485_frame_format *format)
{
	rt_uint8_t is_complete = 1;
	int i;

	if (TRUE == is_em_pt_table_empty(table)) {
		frame_master_info(("em_pt_table is empty!\n"));
		return FAIL;
	}
	for (i=0; i<WIRELESS_MASTER_NODE_MAX; i++) {
		if (is_dev_sn_valid(table->pt_sn[i], sizeof(table->pt_sn[0]))) {
			rt_memcpy(dest_sn, table->pt_sn[i], DEV_SN_MODE_LEN);
			rt_memset(pc_collect_data_m, 0, sizeof(struct msfd_report_collection_data_sn_st)*SLAVE_NODE_INFO_MAX_LEN);
			rs485_send_read_master_data_cmd(rs485_send_buf_m, table->pt_sn[i], format, MD_SEND_READ_MASTER_DATA_CMD);
			if (RT_EOK != rt_sem_take(recv_wireless_master_data_rep_sem, 1000)) {
				is_complete = 0;
				frame_master_info(("\n*************************\n"));
				frame_master_info(("read data fail, rs485 slave sn:"));
				print_dev_sn(table->pt_sn[i], sizeof(table->pt_sn[0]));
				frame_master_info(("*************************\n"));
				//return FAIL;
			}
			if (RT_EOK != rt_sem_take(tcp_send_sem, RT_WAITING_FOREVER)) {
				frame_master_info(("func:%s,line:%d,take tcp_send_sem fail\n", __FUNCTION__, __LINE__));
				goto ret_err;
			}
			rt_sem_release(tcp_send_sem);
		}
	}

ret_err:
	if (is_complete) {
		frame_master_info(("***************\n"));
		frame_master_info(("read data complete\n"));
		frame_master_info(("***************\n"));
		return SUCC;
	} else {
		is_complete = 1;
		frame_master_info(("***************\n"));
		frame_master_info(("read data uncomplete\n"));
		frame_master_info(("***************\n"));
		return FAIL;
	}
}

static int proc_send_stop_probe(struct wl_master_485_slave_info_tbl *table, struct rs485_frame_format *format)
{
	rt_uint8_t is_complete = 1;
	int i;

	if (TRUE == is_em_pt_table_empty(table)) {
		frame_master_info(("em_pt_table is empty!\n"));
		return FAIL;
	}

	for (i=0;i<WIRELESS_MASTER_NODE_MAX;i++) {
		if (is_dev_sn_valid(table->pt_sn[i], sizeof(table->pt_sn[0]))) {
			rs485_send_stop_probe_cmd(rs485_send_buf_m, table->pt_sn[i], format);
			if (RT_EOK != rt_sem_take(recv_wireless_master_stop_probe_rep_sem, 300)) {
				is_complete = 0;
				frame_master_info(("stop probe fail, rs485 slave sn:"));
				print_dev_sn(table->pt_sn[i], sizeof(table->pt_sn[0]));
				return FAIL;
			}
		}
	}
	if (is_complete) {
		frame_master_info(("*******************\n"));
		frame_master_info(("stop probe complete\n"));
		frame_master_info(("*******************\n"));
		return SUCC;
	} else {
		is_complete = 1;
		frame_master_info(("*******************\n"));
		frame_master_info(("stop probe uncomplete\n"));
		frame_master_info(("*******************\n"));
		return FAIL;
	}
}

static int auto_networking(struct wl_master_485_slave_info_tbl *table, struct rs485_frame_format *format)
{
	if (SUCC != proc_send_probe(table, format)) {
		frame_master_info(("((((((((((((((((((((\n"));
		frame_master_info(("proc send probe fail\n"));
		frame_master_info(("))))))))))))))))))))\n"));		
		return FAIL;
	}

	if (SUCC != proc_send_stop_probe(table, format)) {
		frame_master_info(("((((((((((((((((((((\n"));
		frame_master_info(("proc send stop probe fail\n"));
		frame_master_info(("))))))))))))))))))))\n"));	
		return FAIL;
	}

	if (SUCC != proc_send_netcfg(table, format)) {
		frame_master_info(("((((((((((((((((((((\n"));
		frame_master_info(("proc send netcfg fail\n"));
		frame_master_info(("))))))))))))))))))))\n"));	
		return FAIL;
	}

	frame_master_info(("((((((((((((((((((((\n"));
	frame_master_info(("auto networking succ\n"));
	frame_master_info(("))))))))))))))))))))\n"));
	return SUCC;
}

void  rs485_master_thread_entry(void *parameter)
{
	rt_err_t err;
	rt_uint32_t rs485_cmd;
	struct rs485_frame_format format;
	struct wl_master_485_slave_info_tbl *em_pt_table;

	say_thread_start();

	init_wl_master_485_slave_info_tbl();
		
	while (1) {
		err = rt_mb_recv(wl_master_data_over485_mb, &rs485_cmd, RT_WAITING_FOREVER);
		if (RT_EOK == err) {
			em_pt_table = rt_malloc(sizeof(struct wl_master_485_slave_info_tbl));
			if (RT_NULL == em_pt_table) {
				frame_master_info(("func:%s(), out of memory\n", __FUNCTION__));
				return;
			}
			read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_WLM_485S_INFO, 0, em_pt_table);
			switch (rs485_cmd) {
			case MD_SEND_PROBE_CMD://发送探测广播帧
				auto_networking(em_pt_table, &format);
				break;
				
			case MD_SEND_READ_MASTER_DATA_CMD:
				proc_send_read_data(em_pt_table, &format);
				break;

			case MD_SEND_ECHO_CMD:
				if (check_sn_index(em_pt_table->pt_sn, dest_sn, sizeof(dest_sn), WIRELESS_MASTER_NODE_MAX) < WIRELESS_MASTER_NODE_MAX) {
					rs485_send_echo_cmd(rs485_send_buf_m, dest_sn, &echo_id_m, &format);
					frame_master_info(("send echo cmd\n"));
				} else {
					frame_master_info(("dont exist master node\n"));
				}
				break;

			case MD_SEND_STOP_ECHO_CMD:
				if (check_sn_index(em_pt_table->pt_sn, dest_sn, sizeof(dest_sn), WIRELESS_MASTER_NODE_MAX) < WIRELESS_MASTER_NODE_MAX) {
					rs485_send_stop_echo_cmd(rs485_send_buf_m, dest_sn, &format);
					frame_master_info(("send stop echo cmd\n"));
				} else {
					frame_master_info(("dont exist master node\n"));
				}
				break;
				
			default:
				break;
			
			}
			rt_free(em_pt_table);
		}
        }
	
        return;
}

int rs485_master_init(void)
{
        rt_thread_t thread_h1, thread_h2;

	rs485_send_buf_m = rt_malloc(FRAME_485_LEN);
	if(rs485_send_buf_m == RT_NULL)
		return FAIL;
	rt_memset(rs485_send_buf_m, 0, FRAME_485_LEN);

	ms_original_pairs_table = rt_malloc(sizeof(struct master_slave_original_pairs_table_st)*SLAVE_485_MAX_NUM);
	if (ms_original_pairs_table == RT_NULL) {
		rt_free(rs485_send_buf_m);
		frame_master_info(("func:%s(), line:%d malloc fail\n",__FUNCTION__, __LINE__));
		return FAIL;
	}
	rt_memset(ms_original_pairs_table, 0, sizeof(struct master_slave_original_pairs_table_st)*SLAVE_485_MAX_NUM);

	ms_proc_pairs_table = rt_malloc(sizeof(struct master_slave_proc_pairs_table_st)*SLAVE_485_MAX_NUM);
	if (ms_proc_pairs_table == RT_NULL) {
		rt_free(rs485_send_buf_m);
		rt_free(ms_original_pairs_table);
		frame_master_info(("func:%s(), line:%d malloc fail\n",__FUNCTION__, __LINE__));
		return FAIL;
	}
	rt_memset(ms_proc_pairs_table, 0, sizeof(struct master_slave_proc_pairs_table_st)*SLAVE_485_MAX_NUM);

	pc_collect_data_m = rt_malloc(sizeof(struct msfd_report_collection_data_sn_st)*SLAVE_NODE_INFO_MAX_LEN);
	if (pc_collect_data_m == RT_NULL) {
		rt_free(rs485_send_buf_m);
		rt_free(ms_original_pairs_table);
		rt_free(ms_proc_pairs_table);
		frame_master_info(("func:%s(), line:%d malloc fail\n",__FUNCTION__, __LINE__));
		return FAIL;
	}
	rt_memset(pc_collect_data_m, 0, sizeof(struct msfd_report_collection_data_sn_st)*SLAVE_NODE_INFO_MAX_LEN);

	wl_master_data_over485_mb = rt_mb_create("rsc_mb", RS485_CMD_MB_MAX_LEN, RT_IPC_FLAG_FIFO);
	if (RT_NULL == wl_master_data_over485_mb) {
		rt_free(rs485_send_buf_m);
		rt_free(ms_original_pairs_table);
		rt_free(ms_proc_pairs_table);
		rt_free(pc_collect_data_m);
		frame_master_info(("func:%s(), line:%d malloc fail\n",__FUNCTION__, __LINE__));
		return FAIL;
	}

	recv_wireless_master_probe_rep_sem = rt_sem_create("rsp_sem", 0, RT_IPC_FLAG_FIFO);
	if (recv_wireless_master_probe_rep_sem == RT_NULL) {
		rt_free(rs485_send_buf_m);
		rt_free(ms_original_pairs_table);
		rt_free(ms_proc_pairs_table);
		rt_free(pc_collect_data_m);
		rt_free(wl_master_data_over485_mb);
		frame_master_info(("func:%s(), line:%d malloc fail\n",__FUNCTION__, __LINE__));
		return FAIL;
	}

	recv_wireless_master_netcfg_rep_sem = rt_sem_create("rsn_sem", 0, RT_IPC_FLAG_FIFO);
	if (recv_wireless_master_netcfg_rep_sem == RT_NULL) {
		rt_free(rs485_send_buf_m);
		rt_free(ms_original_pairs_table);
		rt_free(ms_proc_pairs_table);
		rt_free(pc_collect_data_m);
		rt_free(wl_master_data_over485_mb);
		rt_free(recv_wireless_master_probe_rep_sem);
		frame_master_info(("func:%s(), line:%d malloc fail\n",__FUNCTION__, __LINE__));
		return FAIL;
	}

	recv_wireless_master_stop_probe_rep_sem = rt_sem_create("rssp_sem", 0, RT_IPC_FLAG_FIFO);
	if (recv_wireless_master_stop_probe_rep_sem == RT_NULL) {
		rt_free(rs485_send_buf_m);
		rt_free(ms_original_pairs_table);
		rt_free(ms_proc_pairs_table);
		rt_free(pc_collect_data_m);
		rt_free(wl_master_data_over485_mb);
		rt_free(recv_wireless_master_probe_rep_sem);
		rt_free(recv_wireless_master_netcfg_rep_sem);
		frame_master_info(("func:%s(), line:%d malloc fail\n",__FUNCTION__, __LINE__));
		return FAIL;
	}
	recv_wireless_master_data_rep_sem = rt_sem_create("rsrd_sem", 0, RT_IPC_FLAG_FIFO);
	if (recv_wireless_master_data_rep_sem == RT_NULL) {
		rt_free(rs485_send_buf_m);
		rt_free(ms_original_pairs_table);
		rt_free(ms_proc_pairs_table);
		rt_free(pc_collect_data_m);
		rt_free(wl_master_data_over485_mb);
		rt_free(recv_wireless_master_probe_rep_sem);
		rt_free(recv_wireless_master_netcfg_rep_sem);
		rt_free(recv_wireless_master_stop_probe_rep_sem);
		frame_master_info(("func:%s(), line:%d malloc fail\n",__FUNCTION__, __LINE__));
		return FAIL;
	}
	tcp_send_sem = rt_sem_create("tss_sem", 1, RT_IPC_FLAG_FIFO);
	if (tcp_send_sem == RT_NULL) {
		rt_free(rs485_send_buf_m);
		rt_free(ms_original_pairs_table);
		rt_free(ms_proc_pairs_table);
		rt_free(pc_collect_data_m);
		rt_free(wl_master_data_over485_mb);
		rt_free(recv_wireless_master_probe_rep_sem);
		rt_free(recv_wireless_master_netcfg_rep_sem);
		rt_free(recv_wireless_master_stop_probe_rep_sem);
		rt_free(recv_wireless_master_data_rep_sem);
		frame_master_info(("func:%s(), line:%d malloc fail\n",__FUNCTION__, __LINE__));
		return FAIL;
	}
	
	/* rs485 cmd thread */
        thread_h1= rt_thread_create("485_cmd",rs485_master_thread_entry, RT_NULL,512, 24, 10);
        if (thread_h1 != RT_NULL) {
		rt_thread_startup(thread_h1);
	} else {
		rt_free(rs485_send_buf_m);
		rt_free(ms_original_pairs_table);
		rt_free(ms_proc_pairs_table);
		rt_free(pc_collect_data_m);
		rt_free(wl_master_data_over485_mb);
		rt_free(recv_wireless_master_probe_rep_sem);
		rt_free(recv_wireless_master_netcfg_rep_sem);
		rt_free(recv_wireless_master_stop_probe_rep_sem);
		rt_free(recv_wireless_master_data_rep_sem);
		frame_master_info(("func:%s(), line:%d create thread fail\n",__FUNCTION__, __LINE__));
		return FAIL;
	}

        /* rs485 master recv thread */
	thread_h2 = rt_thread_create("485mrecv",wireless_rs485_recv_entry, (void *)1, 1024, 24, 10);
        if (thread_h2 != RT_NULL) {
		rt_thread_startup(thread_h2);
	} else {
		rt_free(rs485_send_buf_m);
		rt_free(ms_original_pairs_table);
		rt_free(ms_proc_pairs_table);
		rt_free(pc_collect_data_m);
		rt_free(wl_master_data_over485_mb);
		rt_free(recv_wireless_master_probe_rep_sem);
		rt_free(recv_wireless_master_netcfg_rep_sem);
		rt_free(recv_wireless_master_stop_probe_rep_sem);
		rt_free(recv_wireless_master_data_rep_sem);
		rt_thread_delete(thread_h1);
		frame_master_info(("func:%s(), line:%d create thread fail\n",__FUNCTION__, __LINE__));
		return FAIL;
	}

	
        return SUCC;
}

static int check_sn_index(const void *cs, const void *ct, int len, int times)
{
	int i;
	if (cs == RT_NULL || ct == RT_NULL) {
		frame_master_info(("func:%s,line:%d param error\n",__FUNCTION__, __LINE__));
		return -1;
	}
	for (i=0;i<times;i++) {
		if (0 == rt_memcmp(cs+i*len, ct, len)) {
			break;
		}
	}
	return i;
}

static int is_em_pt_table_empty(struct wl_master_485_slave_info_tbl *table)
{
	int i;

	for (i=0; i<sizeof(table->pt_sn)/sizeof(table->pt_sn[0]); i++) {
		if (is_dev_sn_valid(table->pt_sn[i], sizeof(table->pt_sn[0]))) {
			return FALSE;
		}
	}
	return TRUE;
}

static int is_ms_proc_pairs_table_empty(struct master_slave_proc_pairs_table_st *table)
{
	int i;

	for (i=0; i<sizeof(table->slave_node_info)/sizeof(table->slave_node_info[0]); i++) {
		if (is_dev_sn_valid(table->slave_node_info[i].sn, sizeof(table->slave_node_info[0].sn))) {
			return FALSE;
		}
	}
	return TRUE;
}

static int get_ms_proc_pairs_table_num(struct master_slave_proc_pairs_table_st *table)
{
	int num = 0, i, j;

	for (i=0; i<WIRELESS_MASTER_NODE_MAX; i++) {
		if (is_dev_sn_valid(table[i].src_sn, sizeof(table[0].src_sn))) {
			for (j=0; j<SLAVE_NODE_INFO_MAX_LEN; j++) {
				if (is_dev_sn_valid(table[i].slave_node_info[j].sn, sizeof(table[0].slave_node_info[0].sn))) {
					num++;
				} else {
					break;
				}
			}
		}
	}
	return num;
}

void rs485_send_probe_cmd(rt_uint8_t *tx_buf, rt_uint8_t *dest_addr, struct rs485_frame_format *format, enum rs485_frame_ctrl ctrl)
{
	rs485_create_frame_head(0, dest_addr, ctrl, format);
	rs485_package_data(tx_buf, FRAME_485_LEN, format, RT_NULL);
	send_data_by_485(RS485_PORT_USED_BY_WIRELESS, tx_buf, (RS485_FIX_LEN + format->data_len));
}

static void rs485_send_netcfg_cmd(rt_uint8_t *tx_buf, rt_uint8_t *dest_addr, rt_uint8_t *data, rt_uint8_t data_len,
					struct rs485_frame_format *format)
{
	rs485_create_frame_head(data_len, dest_addr, MD_SEND_NETCFG_CMD, format);
	rs485_package_data(tx_buf, FRAME_485_LEN, format, data);
	send_data_by_485(RS485_PORT_USED_BY_WIRELESS, tx_buf, (RS485_FIX_LEN + format->data_len));
}

void rs485_send_read_master_data_cmd(rt_uint8_t *tx_buf, rt_uint8_t *dest_addr, struct rs485_frame_format *format, enum rs485_frame_ctrl ctrl)
{
	rs485_create_frame_head(0, dest_addr, ctrl, format);
	rs485_package_data(tx_buf, FRAME_485_LEN, format, RT_NULL);
	send_data_by_485(RS485_PORT_USED_BY_WIRELESS, tx_buf, (RS485_FIX_LEN + format->data_len));
}

static void rs485_send_echo_cmd(rt_uint8_t *tx_buf, rt_uint8_t *dest_addr, rt_uint8_t *echo, struct rs485_frame_format *format)
{
	rs485_create_frame_head(sizeof(rt_uint8_t), dest_addr, MD_SEND_ECHO_CMD, format);
	rs485_package_data(tx_buf, FRAME_485_LEN, format, echo);
	send_data_by_485(RS485_PORT_USED_BY_WIRELESS, tx_buf, (RS485_FIX_LEN + format->data_len));
	*echo = 0;
}

static void rs485_send_stop_echo_cmd(rt_uint8_t *tx_buf, rt_uint8_t *dest_addr, struct rs485_frame_format *format)
{
	rs485_create_frame_head(0, dest_addr, MD_SEND_STOP_ECHO_CMD, format);
	rs485_package_data(tx_buf, FRAME_485_LEN, format, RT_NULL);
	send_data_by_485(RS485_PORT_USED_BY_WIRELESS, tx_buf, (RS485_FIX_LEN + format->data_len));
}

static void rs485_send_stop_probe_cmd(rt_uint8_t *tx_buf, rt_uint8_t *dest_addr, struct rs485_frame_format *format)
{
	rs485_create_frame_head(0, dest_addr, MD_SEND_STOP_PROBE_CMD, format);
	rs485_package_data(tx_buf, FRAME_485_LEN, format, RT_NULL);
	send_data_by_485(RS485_PORT_USED_BY_WIRELESS, tx_buf, (RS485_FIX_LEN + format->data_len));
}

static int init_wl_master_485_slave_info_tbl(void)
{
	struct wl_master_485_slave_info_tbl *em_pt_table;
	int i;
	rt_uint8_t index = 0;
	
	em_pt_table = rt_malloc(sizeof(struct wl_master_485_slave_info_tbl));
	if (RT_NULL == em_pt_table) {
		frame_master_info(("func:%s(), out of memory\n", __FUNCTION__));
		return FAIL;
	}
	read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_WLM_485S_INFO, 0, em_pt_table);
	rt_memset(ms_original_pairs_table, 0, sizeof(struct master_slave_original_pairs_table_st)*SLAVE_485_MAX_NUM);

	for (i=0; i<SLAVE_485_MAX_NUM; i++) {
		if (is_dev_sn_valid(em_pt_table->pt_sn[i], DEV_SN_MODE_LEN)) {
			break;
		}
	}
	if (i >= SLAVE_485_MAX_NUM) {
		frame_master_info(("em_pt_table is empty\n"));
		return FAIL;
	}
	
	for (i=0;i<WIRELESS_MASTER_NODE_MAX;i++) {
		/* 将wl_master_485_slave_info_tbl中有效的SN拷贝到ms_original_pairs_table */
		if (is_dev_sn_valid(em_pt_table->pt_sn[i], DEV_SN_MODE_LEN)) {
			rt_memcpy(ms_original_pairs_table[index++].src_sn, em_pt_table->pt_sn[i], sizeof(ms_original_pairs_table[0].src_sn));
		}
	}

	rt_free(em_pt_table);
	return SUCC;
}

/*
 * 将无线节点收集的ade7880数据缓存到ram中
 * */
void updata_ade7880_data_to_ram(struct msfd_report_collection_data_sn_st *data)
{
	enum sink_data_dev_type_e type;
	struct sinkinfo_wl_data_item_st wl_data;

	rt_memset(&wl_data, 0, sizeof(wl_data));

	rt_memcpy(wl_data.pt_ct_sn, data->sn, DEV_SN_MODE_LEN);
	type = si_get_dev_type(wl_data.pt_ct_sn);
	if (type == SDDT_PT) {
		wl_data.item.ptc_data.pt_pa.vx = data->coll_data.pt_ct_v[0];
		wl_data.item.ptc_data.pt_pa.appx = data->coll_data.pt_ct_app_p[0];
		wl_data.item.ptc_data.pt_pa.apx = data->coll_data.pt_ct_ap_p[0];
		wl_data.item.ptc_data.pt_pa.ix = data->coll_data.pt_ct_i[0];
		if (data->coll_data.pt_ct_app_p[0] != 0) {
			wl_data.item.ptc_data.pt_pa.pfx = data->coll_data.pt_ct_ap_p[0]/data->coll_data.pt_ct_app_p[0];
		} else {
			frame_master_info(("app_p_A is zero\n"));
		}
		//wl_data.item.ptc_data.pt_pa.admx = RT_INT32_MAX; /*由网管计算得到*/
		
		wl_data.item.ptc_data.pt_pb.vx = data->coll_data.pt_ct_v[1];
		wl_data.item.ptc_data.pt_pb.appx = data->coll_data.pt_ct_app_p[1];
		wl_data.item.ptc_data.pt_pb.apx = data->coll_data.pt_ct_ap_p[1];
		wl_data.item.ptc_data.pt_pb.ix = data->coll_data.pt_ct_i[1];
		if (data->coll_data.pt_ct_app_p[1] != 0) {
			wl_data.item.ptc_data.pt_pb.pfx = data->coll_data.pt_ct_ap_p[1]/data->coll_data.pt_ct_app_p[1];
		} else {
			frame_master_info(("app_p_B is zero\n"));
		}
		//wl_data.item.ptc_data.pt_pb.admx = RT_INT32_MAX; 
		
		wl_data.item.ptc_data.pt_pc.vx = data->coll_data.pt_ct_v[2];
		wl_data.item.ptc_data.pt_pc.appx = data->coll_data.pt_ct_app_p[2];
		wl_data.item.ptc_data.pt_pc.apx = data->coll_data.pt_ct_ap_p[2];
		wl_data.item.ptc_data.pt_pc.ix = data->coll_data.pt_ct_i[2];
		if (data->coll_data.pt_ct_app_p[2] != 0) {
			wl_data.item.ptc_data.pt_pc.pfx = data->coll_data.pt_ct_ap_p[2]/data->coll_data.pt_ct_app_p[2];
		} else {
			frame_master_info(("app_p_C is zero\n"));
		}
		//wl_data.item.ptc_data.pt_pc.admx = RT_INT32_MAX; 
		si_update_wl_collect_data(wl_data.pt_ct_sn, &wl_data.item, data->last_update_time);
	} else if (type == SDDT_CT) {
		wl_data.item.ctc_data.ct_pa.vx = data->coll_data.pt_ct_v[0];
		wl_data.item.ctc_data.ct_pa.appx = data->coll_data.pt_ct_app_p[0];
		wl_data.item.ctc_data.ct_pa.apx = data->coll_data.pt_ct_ap_p[0];
		wl_data.item.ctc_data.ct_pa.ix = data->coll_data.pt_ct_i[0];
		if (data->coll_data.pt_ct_app_p[0] != 0) {
			wl_data.item.ctc_data.ct_pa.pfx = data->coll_data.pt_ct_ap_p[0]/data->coll_data.pt_ct_app_p[0];
		} else {
			frame_master_info(("app_p_A is zero\n"));
		}
		//wl_data.item.ctc_data.ct_pa.admx = RT_INT32_MAX;
		
		wl_data.item.ctc_data.ct_pb.vx = data->coll_data.pt_ct_v[1];
		wl_data.item.ctc_data.ct_pb.appx = data->coll_data.pt_ct_app_p[1];
		wl_data.item.ctc_data.ct_pb.apx = data->coll_data.pt_ct_ap_p[1];
		wl_data.item.ctc_data.ct_pb.ix = data->coll_data.pt_ct_i[1];
		if (data->coll_data.pt_ct_app_p[1] != 0) {
			wl_data.item.ctc_data.ct_pb.pfx = data->coll_data.pt_ct_ap_p[1]/data->coll_data.pt_ct_app_p[1];
		} else {
			frame_master_info(("app_p_B is zero\n"));
		}
		//wl_data.item.ctc_data.ct_pb.admx = RT_INT32_MAX;
		
		wl_data.item.ctc_data.ct_pc.vx = data->coll_data.pt_ct_v[2];
		wl_data.item.ctc_data.ct_pc.appx = data->coll_data.pt_ct_app_p[2];
		wl_data.item.ctc_data.ct_pc.apx = data->coll_data.pt_ct_ap_p[2];
		wl_data.item.ctc_data.ct_pc.ix = data->coll_data.pt_ct_i[2];
		if (data->coll_data.pt_ct_app_p[2] != 0) {
			wl_data.item.ctc_data.ct_pc.pfx = data->coll_data.pt_ct_ap_p[2]/data->coll_data.pt_ct_app_p[2];
		} else {
			frame_master_info(("app_p_C is zero\n"));
		}
		//wl_data.item.ctc_data.ct_pc.admx = RT_INT32_MAX;
		si_update_wl_collect_data(wl_data.pt_ct_sn, &wl_data.item, data->last_update_time);
	} else {
		printf_syn("func:%s(), get invalid dev-type:%d\n", __FUNCTION__, type);
	}
}

/* 从485获取4432半自动组网的原始信息 */
static void auto_distribute_net_by_rssi(struct master_slave_original_pairs_table_st *original, 
					struct master_slave_proc_pairs_table_st *proc)
{
	rt_uint8_t i;
	rt_uint8_t temp_sn[DEV_SN_MODE_LEN];
	rt_uint8_t temp_rssi;
	rt_uint8_t valid_num;

	for (i=0; i<SLAVE_485_MAX_NUM; i++) {
		if (is_dev_sn_valid(original[i].src_sn, DEV_SN_MODE_LEN)) {
			rt_uint8_t n;
			frame_master_info(("&&&&&&&&&&&&&&&&\n"));
			frame_master_info(("master sn:"));
			print_dev_sn(original[i].src_sn, DEV_SN_MODE_LEN);
			for (n=0; n<WIRELESS_SLAVE_NODE_MAX; n++) {
				if (TRUE == is_dev_sn_valid(original[i].slave_node_info[n].sn, DEV_SN_MODE_LEN)) {
					frame_master_info(("rssi:%3d--", original[i].slave_node_info[n].rssi));
					frame_master_info(("slave sn:"));
					print_dev_sn(original[i].slave_node_info[n].sn, DEV_SN_MODE_LEN);
				} else {
					break;
				}
			}
			frame_master_info(("&&&&&&&&&&&&&&&&\n\n"));
		}
	}

	for (i=0; i<SLAVE_485_MAX_NUM; i++) {/* 找出有多少个485从设备 */
		if (!is_dev_sn_valid(original[i].src_sn, DEV_SN_MODE_LEN))
			break;
	}

	if (i > 0) {
		rt_uint8_t j;

		for (j=0; j<i; j++) {/* 循环每个从设备 */
			rt_uint8_t k;
			
			for (k=0; k<SLAVE_NODE_INFO_MAX_LEN; k++) {
				rt_uint8_t x;

				rt_memcpy(temp_sn, original[j].slave_node_info[k].sn, DEV_SN_MODE_LEN);
				temp_rssi = original[j].slave_node_info[k].rssi;
				for (x=j+1; x<i; x++) {
					rt_uint8_t y;
					
					for (y=0; y<SLAVE_NODE_INFO_MAX_LEN; y++) {
						if (0 == rt_memcmp(original[x].slave_node_info[y].sn, 
							temp_sn, DEV_SN_MODE_LEN)) {
							if (original[x].slave_node_info[y].rssi > temp_rssi) {
								rt_memset(original[j].slave_node_info[k].sn,
									0, DEV_SN_MODE_LEN);
							} else {
								rt_memset(original[x].slave_node_info[y].sn, 
									0, DEV_SN_MODE_LEN);
							}
							break;
						}
					}
				}
			}
		}

		for (j=0; j<i; j++) {
			rt_uint8_t z;

			valid_num = 0;
			rt_memcpy(proc[j].src_sn, original[j].src_sn, DEV_SN_MODE_LEN);
			frame_master_info(("\n\n"));
			frame_master_info(("###################\n"));
			frame_master_info(("wireless master SN:"));
			print_dev_sn(proc[j].src_sn, DEV_SN_MODE_LEN);
			for (z=0; z<SLAVE_NODE_INFO_MAX_LEN; z++) {
				if (is_dev_sn_valid(original[j].slave_node_info[z].sn, DEV_SN_MODE_LEN)) {
					rt_memcpy(proc[j].slave_node_info[valid_num++].sn, original[j].slave_node_info[z].sn, DEV_SN_MODE_LEN);
					frame_master_info(("%d slave SN:", z));
					print_dev_sn(original[j].slave_node_info[z].sn, DEV_SN_MODE_LEN);
				}
			}
			frame_master_info(("###################\n\n\n"));
		}
	}
}
