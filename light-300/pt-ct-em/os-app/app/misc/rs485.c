/*
 ******************************************************************************
 * rs485.c
 *
 * 2013-10-15,  creat by David, zhaoshaowei@yeejoin.com
 ******************************************************************************
 */

#include <rtdef.h>
#include <board.h>
#include <rs485.h>
#include <rs485_common.h>
#include <sys_cfg_api.h>
#include <misc_lib.h>
#if WIRELESS_MASTER_NODE
#include <syscfgdata.h>
#endif

#if 1
#define WAIT_RX_MSG_TIMEOUT (get_ticks_of_ms(1500))
#else
#define WAIT_RX_MSG_TIMEOUT RT_WAITING_FOREVER
#endif

#if EM_MASTER_DEV || EM_MULTI_MASTER_DEV
#include <frame_em.h>

extern rt_uint8_t echo_id_m;
extern struct msfd_report_collection_data_sn_st *pc_collect_data_m;
extern struct master_slave_original_pairs_table_st *ms_original_pairs_table;
extern rt_sem_t     recv_wireless_master_probe_rep_sem;
extern rt_sem_t     recv_wireless_master_netcfg_rep_sem;
extern rt_sem_t     recv_wireless_master_stop_probe_rep_sem;
extern rt_sem_t     recv_wireless_master_data_rep_sem;
extern rt_sem_t     tcp_send_sem;

enum tcp_transport_err_e em_distribute_data_by_em_distrib_table(struct msfd_report_collection_data_sn_st *data);
void updata_ade7880_data_to_ram(struct msfd_report_collection_data_sn_st *data);
void rs485_send_read_master_data_cmd(rt_uint8_t *tx_buf, rt_uint8_t *dest_addr, struct rs485_frame_format *format, enum rs485_frame_ctrl ctrl);

void rs485_send_probe_cmd(rt_uint8_t *tx_buf, rt_uint8_t *dest_addr, struct rs485_frame_format *format, enum rs485_frame_ctrl ctrl);
extern rt_uint8_t dest_sn[DEV_SN_MODE_LEN];

#elif WIRELESS_MASTER_NODE
#include <master_fsm.h>
//extern rt_uint8_t is_stop_echo;
//extern rt_uint8_t is_start_echo;
//extern rt_uint8_t echo_id_s;
void rs485_send_rep_probe_cmd(rt_uint8_t *tx_buf, rt_uint8_t *src_addr, rt_uint8_t *data, rt_uint8_t data_len,
				struct rs485_frame_format *format, enum rs485_frame_ctrl ctrl);
void rs485_send_rep_netcfg_cmd(rt_uint8_t *tx_buf, rt_uint8_t *src_addr, rt_uint8_t *data, rt_uint8_t data_len,
				struct rs485_frame_format *format);
void rs485_send_rep_read_master_data_cmd(rt_uint8_t *tx_buf, rt_uint8_t *src_addr, rt_uint8_t *data, rt_uint8_t data_len,
					struct rs485_frame_format *format, enum rs485_frame_ctrl ctrl);
void rs485_send_rep_echo_cmd(rt_uint8_t *tx_buf, rt_uint8_t *src_addr, rt_uint8_t *echo, struct rs485_frame_format *format);
void rs485_send_rep_stop_echo_cmd(rt_uint8_t *tx_buf, rt_uint8_t *src_addr, struct rs485_frame_format *format);
void rs485_send_rep_stop_probe_cmd(rt_uint8_t *tx_buf, rt_uint8_t *src_addr, struct rs485_frame_format *format);
void updata_self_data(struct msfd_report_collection_data_sn_st *msfd_collect_data);
extern struct msfd_report_collection_data_sn_st *pc_collect_data_s;
//extern struct wireless_register_white_st *wireless_white_register;
extern rt_sem_t    probe_timeout_sem;
extern struct register_slave_node_info_st register_slave_node_info[WIRELESS_SLAVE_NODE_MAX];
extern 	struct register_slave_node_original_info_st *original_info;
#endif

struct anylasis_wl_485_frame_t {
	enum rs485_recv_analysis_state recv_state; /* 分析数据的状态机状态 */
	int data_len;	/* 记录该帧的数据域长度 */
	int frame_ind;	/* 记录分析的数据域字节索引 */
};

struct anylasis_wl_485_frame_t  anylasis_wl_485_frame;

extern int init_anylasis_wl_485_frame_t(struct anylasis_wl_485_frame_t *p);

struct rt_semaphore uart485_1_rx_byte_sem;
struct rt_semaphore uart485_2_rx_byte_sem;
struct rt_semaphore uart485_3_rx_byte_sem;
rt_mailbox_t uart485_mb;/* mark by zp */

static int set_485_tx_rx_state(USART_TypeDef *dev_485, int is_tx_state);
static rt_err_t uart485_1_rx_ind(rt_device_t dev, rt_size_t size);
static rt_err_t uart485_2_rx_ind(rt_device_t dev, rt_size_t size);
static rt_err_t uart485_3_rx_ind(rt_device_t dev, rt_size_t size);
static void wireless_rs485_rx_data(rt_device_t dev, rt_uint32_t select);
static void wireless_rs485_data_proc(rt_uint8_t *pkt_data);

static rt_device_t dev_485_1;
static rt_device_t dev_485_2;
static rt_device_t dev_485_3;

rt_uint8_t *rs485_send_buf_s;
rt_uint8_t *rs485_send_buf_m;

rt_uint8_t *rs485_recv_buf;

extern struct master_slave_original_pairs_table_st *ms_original_pairs_table;
extern struct master_slave_proc_pairs_table_st *ms_proc_pairs_table;

#define rs485_info(x) 		printf_syn x
#define PRINT_DETAIL_MSG	1

void init_sys_485(void)
{

	rt_sem_init(&uart485_1_rx_byte_sem, "u485-1", 0, RT_IPC_FLAG_PRIO);
	rt_sem_init(&uart485_2_rx_byte_sem, "u485-2", 0, RT_IPC_FLAG_PRIO);
	rt_sem_init(&uart485_3_rx_byte_sem, "u485-3", 0, RT_IPC_FLAG_PRIO);
	uart485_mb = rt_mb_create("485_mb", 1, RT_IPC_FLAG_FIFO);/* mark by zp */
	if (RT_NULL == uart485_mb) {
		rs485_info(("func:%s(), line:%d malloc fail\n",__FUNCTION__, __LINE__));
		return;
	}

	rs485_recv_buf= (rt_uint8_t *)rt_malloc(FRAME_485_LEN);
	if(rs485_recv_buf == RT_NULL) {
		rt_mb_delete(uart485_mb);
		rs485_info(("func:%s(), line:%d malloc fail\n",__FUNCTION__, __LINE__));
		return;		
	}
	rt_memset(rs485_recv_buf, 0, FRAME_485_LEN);

	dev_485_1 = rt_device_find(UART_485_1_DEV);
	if (dev_485_1 != RT_NULL && rt_device_open(dev_485_1, RT_DEVICE_OFLAG_RDWR) == RT_EOK) {
		rs485_info(("open device:%s succ\n", UART_485_1_DEV));
		rt_device_set_rx_indicate(dev_485_1, uart485_1_rx_ind);
	} else {
		rs485_info(("can not find device:%s\n", UART_485_1_DEV));
		goto err_ret;
	}

	dev_485_2 = rt_device_find(UART_485_2_DEV);
	if (dev_485_2 != RT_NULL && rt_device_open(dev_485_2, RT_DEVICE_OFLAG_RDWR) == RT_EOK) {
		rs485_info(("open device:%s succ\n", UART_485_2_DEV));
		rt_device_set_rx_indicate(dev_485_2, uart485_2_rx_ind);
	} else {
		rs485_info(("can not find device:%s\n", UART_485_2_DEV));
		goto err_ret;
	}

	dev_485_3 = rt_device_find(UART_485_3_DEV);
	if (dev_485_3 != RT_NULL && rt_device_open(dev_485_3, RT_DEVICE_OFLAG_RDWR) == RT_EOK) {
		rs485_info(("open device:%s succ\n", UART_485_3_DEV));
		rt_device_set_rx_indicate(dev_485_3, uart485_3_rx_ind);
	} else {
		rs485_info(("can not find device:%s\n", UART_485_3_DEV));
		goto err_ret;
	}

	return;

err_ret:
	rt_mb_delete(uart485_mb);
	rt_free(rs485_recv_buf);
	return;
}



rt_size_t send_data_by_485(USART_TypeDef *dev_485, void *data, rt_size_t len)
{
	rt_size_t size;

	if (NULL==dev_485 || NULL==data) {
		rs485_info(("send_data_by_485() param invalid\n"));
		return 0;
	}

	set_485_tx_rx_state(dev_485, 1);
	//rt_thread_delay(2);

	if (UART_485_1_DEV_PTR == dev_485) {
		size = dev_485_1->write(dev_485_1, 0, data, len);
	} else if (UART_485_2_DEV_PTR == dev_485) {
		size = dev_485_2->write(dev_485_2, 0, data, len);
	} else if (UART_485_3_DEV_PTR == dev_485) {
		size = dev_485_3->write(dev_485_3, 0, data, len);
	} else {
		size = 0;
		rs485_info(("revc invalid 485 dev param(0x%x)\n", dev_485));
	}
	//rt_thread_delay(1);
	wait_usartx_send_over(dev_485);

	set_485_tx_rx_state(dev_485, 0);

	return size;
}

rt_size_t recv_data_by_485(USART_TypeDef *dev_485, void *buf, rt_size_t len)
{
	rt_size_t size;

	if (NULL==dev_485 || NULL==buf) {
		rs485_info(("recv_data_by_485() param invalid\n"));
		return 0;
	}

	if (UART_485_1_DEV_PTR == dev_485) {
		size = dev_485_1->read(dev_485_1, 0, buf, len);
	} else if (UART_485_2_DEV_PTR == dev_485) {
		size = dev_485_2->read(dev_485_2, 0, buf, len);
	} else if (UART_485_3_DEV_PTR == dev_485) {
		size = dev_485_3->read(dev_485_3, 0, buf, len);
	} else {
		size = 0;
		rs485_info(("*revc invalid 485 dev param(0x%x)\n", dev_485));
	}

	return size;
}

void clr_rx_buf_recv_data_by_485(USART_TypeDef *dev_485)
{
	if (NULL==dev_485) {
		rs485_info(("send_data_by_485() param invalid\n"));
		return;
	}

	if (UART_485_1_DEV_PTR == dev_485) {
		dev_485_1->control(dev_485_1, RT_DEVICE_CTRL_CLR_RXBUF, NULL);
	} else if (UART_485_2_DEV_PTR == dev_485) {
		dev_485_2->control(dev_485_1, RT_DEVICE_CTRL_CLR_RXBUF, NULL);
	} else if (UART_485_3_DEV_PTR == dev_485) {
		dev_485_3->control(dev_485_1, RT_DEVICE_CTRL_CLR_RXBUF, NULL);
	} else {
		rs485_info(("%s()  revc invalid 485 dev param(0x%x)\n", __func__, dev_485));
	}

	return;
}


static int set_485_tx_rx_state(USART_TypeDef *dev_485, int is_tx_state)
{
	if (UART_485_1_DEV_PTR == dev_485) {
		if (0 != is_tx_state)
			tx_en_rev_disable_485_1();
		else
			tx_disable_rev_en_485_1();
	} else if (UART_485_2_DEV_PTR == dev_485) {
		if (0 != is_tx_state)
			tx_en_rev_disable_485_2();
		else
			tx_disable_rev_en_485_2();
	} else if (UART_485_3_DEV_PTR == dev_485) {
		if (0 != is_tx_state)
			tx_en_rev_disable_485_3();
		else
			tx_disable_rev_en_485_3();
	} else {
		rs485_info(("**revc invalid 485 dev param(0x%x)\n", dev_485));
	}

	return 0;
}

static rt_err_t uart485_1_rx_ind(rt_device_t dev, rt_size_t size)
{
	rt_sem_release(&uart485_1_rx_byte_sem);
	return RT_EOK;
}

static rt_err_t uart485_2_rx_ind(rt_device_t dev, rt_size_t size)
{
	rt_sem_release(&uart485_2_rx_byte_sem);
	return RT_EOK;
}

static rt_err_t uart485_3_rx_ind(rt_device_t dev, rt_size_t size)
{
	rt_sem_release(&uart485_3_rx_byte_sem);
	return RT_EOK;
}


static rt_uint8_t rs485_frame_checksum(rt_uint8_t *pkt_data, rt_uint8_t len)
{
	rt_uint8_t *data;
	rt_uint8_t sum = 0;
	int i;
	
	data = pkt_data;
	for (i=0; i<len; i++) {
		sum += *data++;
	}

	return sum;
}

void rs485_create_frame_head(rt_uint8_t data_len, rt_uint8_t *sn, enum rs485_frame_ctrl ctrl, 
				struct rs485_frame_format *format)
{
	format->start1 = 0x79;
	format->addr = sn;
	format->start2 = 0x79;
	format->ctrl = ctrl;
	format->data_len = data_len;
}

void rs485_package_data(rt_uint8_t *pkt_data, rt_uint8_t pkt_len, struct rs485_frame_format *format,
				rt_uint8_t *data)
{
	rt_uint8_t *tx_pch;
	
	if ((RT_NULL == pkt_data) || (RT_NULL == format)) {
		rs485_info(("%s(),%d, param error\n",__FUNCTION__,__LINE__));
	}

	tx_pch = pkt_data;
	
	*tx_pch++ = format->start1;
	rt_memcpy(tx_pch, format->addr, DEV_SN_MODE_LEN);
	tx_pch += DEV_SN_MODE_LEN;
	*tx_pch++ = format->start2;
	*tx_pch++ = format->ctrl;
	*tx_pch++ = format->data_len;
	rt_memcpy(tx_pch, data, format->data_len);
	tx_pch += format->data_len;
	*tx_pch++ = rs485_frame_checksum(pkt_data, format->data_len + RS485_CHECK_SUM_FIX_LEN);/* check sum */
	*tx_pch++ = 0x27;
}

/* 用于485线收集无线采集数据时, 字节之间的超时定时器 */
rt_timer_t  frame_of_collect_wl_data_timer;

static void frame_of_collect_wl_data_timeout(void *param)
{
	(void)param;

	printf_syn("%s()\n", __FUNCTION__);
	init_anylasis_wl_485_frame_t(&anylasis_wl_485_frame);

	return;
}

void  wireless_rs485_recv_entry(void *parameter)
{
	rt_device_t dev_rs485;
	rt_err_t ret;
	rt_uint32_t param;

	say_thread_start();

	param = (rt_uint32_t)parameter;

#if 0 /* init_sys_485()中已设置 */
	dev_rs485 = rt_device_find(RS485_DEV_USED_BY_WIRELESS);
	if ((dev_rs485 != RT_NULL) && (RT_EOK == rt_device_open(dev_rs485, RT_DEVICE_FLAG_RDWR))) {
		rt_device_set_rx_indicate(dev_rs485, RS485_RX_IND_USED_BY_WIRELESS);
	} else {
		rs485_info(("open device RS485_DEV_USED_BY_WIRELESS fail\n"));
	}
#else
	dev_rs485 = rt_device_find(RS485_DEV_USED_BY_WIRELESS);
	if (NULL == dev_rs485) {
		printf_syn("find rs485-dev for collect wl-data fail\n");
		return;
	}
#endif

	frame_of_collect_wl_data_timer = rt_timer_create("cwldata", frame_of_collect_wl_data_timeout, NULL,
			get_ticks_of_ms(1000), RT_TIMER_FLAG_ONE_SHOT);
	if (NULL == frame_of_collect_wl_data_timer) {
		printf_syn("%s(), create frame_of_collect_wl_data_timer fail\n", __FUNCTION__);
		return;
	}

	init_anylasis_wl_485_frame_t(&anylasis_wl_485_frame);

        while (1) {
		ret = rt_sem_take(&RS485_RX_SEM_USED_BY_WIRELESS, RT_WAITING_FOREVER);
		if (RT_EOK == ret) {
			wireless_rs485_rx_data(dev_rs485, param);
		} else {
			printf_syn("%s(), wait sem fail(%d)", __FUNCTION__, ret);
		}
        }

        return;
}

static void wireless_rs485_data_proc(rt_uint8_t *pkt_data)
{
#if EM_MASTER_DEV || EM_MULTI_MASTER_DEV
	int i = 0;
	int index = 0;
	struct rs485_frame_format format;
	struct record_netcfg_fail_data_st *ird_info;
	enum tcp_transport_err_e tte_err;
#elif WIRELESS_MASTER_NODE
	#define MAX_COUNT_OF_DATA	3
	char src_sn[DEV_SN_MODE_LEN];
	struct rs485_frame_format format;
	struct wireless_register_white_st *white_register;
	static int count = 0;
	static int index = 0;
	int i;
	int ret;
#endif

	switch (pkt_data[RS485_CTL_OFFSET]) {
#if EM_MASTER_DEV || EM_MULTI_MASTER_DEV
	case SD_SEND_READ_MASTER_DATA_CMD_REP:
	case SD_SEND_READ_MASTER_DATA_SUB_CMD_REP:
		/* recv data send to sub EM by tcp */
		if (pkt_data[RS485_DATA_LEN_OFFSET] > 0) {
			for (i=0; i<SLAVE_NODE_INFO_MAX_LEN; i++) {
				if (!is_dev_sn_valid(pc_collect_data_m[i].sn, DEV_SN_MODE_LEN)) {
					break;
				}
			}
			if (i < SLAVE_NODE_INFO_MAX_LEN) {
				rt_memcpy(pc_collect_data_m + i, pkt_data+RS485_DATA_OFFSET, *(pkt_data+RS485_DATA_LEN_OFFSET));
			}
		}
		if (pkt_data[RS485_CTL_OFFSET] == SD_SEND_READ_MASTER_DATA_CMD_REP) {
			rs485_info(("recv read master data rep cmd\n"));
			if (RT_EOK != rt_sem_take(tcp_send_sem, RT_WAITING_FOREVER)) {
				rs485_info(("func:%s,line:%d,take tcp_send_sem fail\n", __FUNCTION__, __LINE__));
				break;
			}
			rt_sem_release(recv_wireless_master_data_rep_sem);
			for (i=0;i<SLAVE_NODE_INFO_MAX_LEN;i++) {
				/* mark by David */
				if (is_dev_sn_valid(pc_collect_data_m[i].sn, sizeof(pc_collect_data_m[0].sn))) {
#if PRINT_DETAIL_MSG
					rs485_info(("sn is:"));
					print_dev_sn(pc_collect_data_m[i].sn, sizeof(pc_collect_data_m[0].sn));
					rs485_info(("voltage:%10d, %10d, %10d\n", pc_collect_data_m[i].coll_data.pt_ct_v[0],
						pc_collect_data_m[i].coll_data.pt_ct_v[1], pc_collect_data_m[i].coll_data.pt_ct_v[2]));

					rs485_info(("current:%10d, %10d, %10d\n", pc_collect_data_m[i].coll_data.pt_ct_i[0],
						pc_collect_data_m[i].coll_data.pt_ct_i[1], pc_collect_data_m[i].coll_data.pt_ct_i[2]));
					
					rs485_info(("  app-p:%10d, %10d, %10d\n", pc_collect_data_m[i].coll_data.pt_ct_app_p[0],
						pc_collect_data_m[i].coll_data.pt_ct_app_p[1], pc_collect_data_m[i].coll_data.pt_ct_app_p[2]));
					
					rs485_info(("  act-p:%10d, %10d, %10d\n\n", pc_collect_data_m[i].coll_data.pt_ct_ap_p[0],
						pc_collect_data_m[i].coll_data.pt_ct_ap_p[1], pc_collect_data_m[i].coll_data.pt_ct_ap_p[2]));
#endif
					updata_ade7880_data_to_ram(&pc_collect_data_m[i]);
#if 1 /* 注释掉tcp分发代码, mark by David */
					tte_err = em_distribute_data_by_em_distrib_table(pc_collect_data_m+i);
					if (TTE_EOK == tte_err) {
						rs485_info(("distribute the package succ!\n"));
					} else {
						rs485_info(("distribute the package fail!\n"));
					}
#endif
				} else {
					break;
				}
			}
			rt_sem_release(tcp_send_sem);
		} else {
			rs485_info(("recv read master data sub rep cmd\n"));
			rs485_send_read_master_data_cmd(rs485_send_buf_m, dest_sn, &format, MD_SEND_READ_MASTER_DATA_SUB_CMD);
		}
		break;

	case SD_SEND_NETCFG_CMD_REP:
		ird_info = rt_malloc(sizeof(struct record_netcfg_fail_data_st)*WIRELESS_SLAVE_NODE_MAX);
		if (ird_info == RT_NULL) {
			rs485_info(("func:%s(), line:%d malloc fail\n",__FUNCTION__, __LINE__));
			return;
		}
		rt_memset(ird_info, 0, sizeof(struct record_netcfg_fail_data_st)*WIRELESS_SLAVE_NODE_MAX);
		rs485_info(("recv netcfg rep cmd\n"));
		if (pkt_data[RS485_DATA_LEN_OFFSET] > 0) {
			rt_memcpy(ird_info, pkt_data+RS485_DATA_OFFSET, *(pkt_data+RS485_DATA_LEN_OFFSET));
			rs485_info(("*******************\n"));
			rs485_info(("netcfg fail sn:\n"));
			for (i=0; i<WIRELESS_SLAVE_NODE_MAX; i++) {
				if (is_dev_sn_valid(ird_info[i].sn, sizeof(ird_info[0].sn))) {
					print_dev_sn(ird_info[i].sn, sizeof(ird_info[0].sn));
				} else {
					break;
				}
			}
			rs485_info(("*******************\n"));
		} else {
			rt_sem_release(recv_wireless_master_netcfg_rep_sem);
		}
		rt_free(ird_info);
		break;

	case SD_SEND_PROBE_CMD_REP:
	case SD_SEND_PROBE_SUB_CMD_REP:
		/* analysis to get wireless write */
		if (pkt_data[RS485_DATA_LEN_OFFSET] > 0) {
			for(i=0;i<SLAVE_485_MAX_NUM;i++) {/* 找到PT的位置 */
				if (0 == rt_memcmp(ms_original_pairs_table[i].src_sn, pkt_data+1, sizeof(ms_original_pairs_table[0].src_sn))) {
					int j;/* 写入无线从节点数据到对应的PT */

					index = i;
					for (j=0; j<SLAVE_NODE_INFO_MAX_LEN; j++) {
						if (!is_dev_sn_valid(ms_original_pairs_table[i].slave_node_info[j].sn, DEV_SN_MODE_LEN)) {
							break;
						}
					}
					if (j < SLAVE_NODE_INFO_MAX_LEN) {
						rt_memcpy(&ms_original_pairs_table[i].slave_node_info[j], 
							(pkt_data+RS485_DATA_OFFSET), *(pkt_data+RS485_DATA_LEN_OFFSET));
					}
					
					break;
				}
			}
			if (i > SLAVE_485_MAX_NUM) {
				rs485_info(("recv data src_sn dont exist in ms_original_pairs_table\n\n"));
			}
		}
		if (pkt_data[RS485_CTL_OFFSET] == SD_SEND_PROBE_CMD_REP) {
			int j;

			rs485_info(("recv probe rep cmd\n"));
			for (j=0; j<SLAVE_NODE_INFO_MAX_LEN; j++) {
				if (TRUE == is_dev_sn_valid(ms_original_pairs_table[index].slave_node_info[j].sn, DEV_SN_MODE_LEN)) {
					print_dev_sn((uint8_t *)ms_original_pairs_table[index].slave_node_info[j].sn, DEV_SN_MODE_LEN);
				} else {
					break;
				}
			}
			rt_sem_release(recv_wireless_master_probe_rep_sem);
		} else {
			rs485_info(("recv probe sub rep cmd\n"));
			rs485_send_probe_cmd(rs485_send_buf_m, dest_sn, &format, MD_SEND_PROBE_SUB_CMD);
		}
		break;

	case SD_SEND_ECHO_CMD_REP:
		rs485_info(("recv succ from NO.%d slave node succ\n",*(pkt_data+RS485_DATA_OFFSET)));
		break;

	case SD_SEND_STOP_PROBE_CMD_REP:
		rs485_info(("rs485 slave is stoping probe\n"));
		rt_sem_release(recv_wireless_master_stop_probe_rep_sem);
		break;
		
#elif WIRELESS_MASTER_NODE
	case MD_SEND_READ_MASTER_DATA_CMD:
		rs485_info(("recv read master data cmd\n"));
		index = 0;
		updata_self_data(pc_collect_data_s);
		for (i=0;i<SLAVE_NODE_INFO_MAX_LEN;i++) {
			if (!is_dev_sn_valid(pc_collect_data_s[i].sn, sizeof(pc_collect_data_s[0].sn))) {
				break;
			}
		}
		if (i > 0) {
			count = i;
			get_devsn(src_sn, DEV_SN_MODE_LEN);
			if ((index + MAX_COUNT_OF_DATA) < count) {
				rs485_send_rep_read_master_data_cmd(rs485_send_buf_s, (rt_uint8_t *)src_sn, 
				(rt_uint8_t *)(pc_collect_data_s + index), sizeof(struct msfd_report_collection_data_sn_st)*MAX_COUNT_OF_DATA, 
				&format, SD_SEND_READ_MASTER_DATA_SUB_CMD_REP);
				index += MAX_COUNT_OF_DATA;
			} else {
				rs485_send_rep_read_master_data_cmd(rs485_send_buf_s, (rt_uint8_t *)src_sn, 
				(rt_uint8_t *)(pc_collect_data_s + index), sizeof(struct msfd_report_collection_data_sn_st)*count, 
				&format, SD_SEND_READ_MASTER_DATA_CMD_REP);
				index = 0;
			}
		}
		break;

	case MD_SEND_READ_MASTER_DATA_SUB_CMD:
		rs485_info(("recv read master data sub cmd\n"));
		get_devsn(src_sn, DEV_SN_MODE_LEN);
		if ((index + MAX_COUNT_OF_DATA) < count) {
			rs485_send_rep_read_master_data_cmd(rs485_send_buf_s, (rt_uint8_t *)src_sn, 
			(rt_uint8_t *)(pc_collect_data_s + index), sizeof(struct msfd_report_collection_data_sn_st)*MAX_COUNT_OF_DATA, 
			&format, SD_SEND_READ_MASTER_DATA_SUB_CMD_REP);
			index += MAX_COUNT_OF_DATA;
		} else {
			rs485_send_rep_read_master_data_cmd(rs485_send_buf_s, (rt_uint8_t *)src_sn, 
			(rt_uint8_t *)(pc_collect_data_s + index), sizeof(struct msfd_report_collection_data_sn_st)*(count-index), 
			&format, SD_SEND_READ_MASTER_DATA_CMD_REP);
			index = 0;
		}
		break;

	case MD_SEND_PROBE_CMD:
		index = 0;
		rs485_info(("recv probe cmd\n"));
		ret = rt_mb_send(uart485_mb, ESTP_PROBE_SLAVE_CMD);
		if (RT_EOK != ret) {
			rs485_info(("send uart485_mb fail\n"));
			break;
		}
		rt_memset(original_info, 0, sizeof(struct register_slave_node_original_info_st)*WIRELESS_SLAVE_NODE_MAX);
		if (RT_EOK == rt_sem_take(probe_timeout_sem, 600)) {
			for (i=0; i<sizeof(register_slave_node_info)/sizeof(register_slave_node_info[0]); i++) {
				if (!is_dev_sn_valid(register_slave_node_info[i].sn, sizeof(register_slave_node_info[0].sn))) {
					break;
				} else {
					rt_memcpy(&original_info[i], &register_slave_node_info[i], sizeof(original_info[0]));
				}
			}

			if (i >= 0) {
				count = i;
				get_devsn(src_sn, DEV_SN_MODE_LEN);
				if ((index + MAX_COUNT_OF_DATA) < count) {
					rs485_send_rep_probe_cmd(rs485_send_buf_s, (rt_uint8_t *)src_sn, 
						(rt_uint8_t *)(original_info + index), 
						sizeof(struct register_slave_node_original_info_st)*MAX_COUNT_OF_DATA,
						&format, SD_SEND_PROBE_SUB_CMD_REP);	
					index += MAX_COUNT_OF_DATA;
					rs485_info(("probe data sub rep\n"));
				} else {
					rs485_send_rep_probe_cmd(rs485_send_buf_s, (rt_uint8_t *)src_sn, 
						(rt_uint8_t *)(original_info + index), 
						sizeof(struct register_slave_node_original_info_st)*count,
						&format, SD_SEND_PROBE_CMD_REP);
					index = 0;
					rs485_info(("probe data rep\n"));
				}
			}
		}
		break;

	case MD_SEND_PROBE_SUB_CMD:
		rs485_info(("recv probe sub cmd\n"));
		get_devsn(src_sn, DEV_SN_MODE_LEN);
		print_dev_sn((uint8_t *)src_sn, DEV_SN_MODE_LEN);
		if ((index + MAX_COUNT_OF_DATA) < count) {
			rs485_send_rep_probe_cmd(rs485_send_buf_s, (rt_uint8_t *)src_sn, 
			(rt_uint8_t *)(original_info + index),
			sizeof(struct register_slave_node_original_info_st)*MAX_COUNT_OF_DATA, 
			&format, SD_SEND_PROBE_SUB_CMD_REP);
			index += MAX_COUNT_OF_DATA;
		} else {
			rs485_send_rep_probe_cmd(rs485_send_buf_s, (rt_uint8_t *)src_sn, 
			(rt_uint8_t *)(original_info + index),
			sizeof(struct register_slave_node_original_info_st)*(count-index), 
			&format, SD_SEND_PROBE_CMD_REP);
			index = 0;
		}
		break;

	case MD_SEND_STOP_PROBE_CMD:
		rs485_info(("recv stop probe cmd\n"));
		ret = rt_mb_send(uart485_mb, ESTP_STOP_PROBE_SLAVE_CMD);
		if (RT_EOK != ret) {
			rs485_info(("send uart485_mb fail\n"));
			break;
		}
		break;

	case MD_SEND_NETCFG_CMD:
		rs485_info(("recv netcfg cmd\n"));
#if 1
		white_register = rt_malloc(sizeof(struct wireless_register_white_st)*SLAVE_NODE_INFO_MAX_LEN);
		if (RT_NULL == white_register) {
			rs485_info(("func:%s(), out of memory\n", __FUNCTION__));
			return;
		}
		rt_memset(white_register, 0, sizeof(struct wireless_register_white_st)*SLAVE_NODE_INFO_MAX_LEN);
		rt_memcpy(white_register, pkt_data+RS485_DATA_OFFSET, *(pkt_data+RS485_DATA_LEN_OFFSET));
		write_syscfgdata_tbl(SYSCFGDATA_TBL_REGISTER_WHITER, 0, white_register);
		syscfgdata_syn_proc();
		ret = rt_mb_send(uart485_mb, ESTP_NETCFG_CMD);
		if (RT_EOK != ret) {
			rs485_info(("send uart485_mb fail\n"));
			break;
		}
#else
		rt_memset(wireless_white_register, 0, sizeof(struct wireless_register_white_st)*SLAVE_NODE_INFO_MAX_LEN);
		rt_memcpy(wireless_white_register, pkt_data+RS485_DATA_OFFSET, *(pkt_data+RS485_DATA_LEN_OFFSET));
		ret = rt_mb_send(uart485_mb, ESTP_NETCFG_CMD);
		if (RT_EOK != ret) {
			rs485_info(("send uart485_mb fail\n"));
			break;
		}
#endif
		break;
#if 0
	case MD_SEND_ECHO_CMD:
		echo_id_s = *(pkt_data+RS485_DATA_OFFSET);
		//rt_mb_send(uart485_mb, ESTP_ECHO_CMD);
		is_start_echo = 1;
		break;

	case MD_SEND_STOP_ECHO_CMD:
		is_stop_echo = 1;
		break;
#endif
#endif
	default:
		break;
	}
}


int init_anylasis_wl_485_frame_t(struct anylasis_wl_485_frame_t *p)
{
	if (NULL == p) {
		printf_syn("%s(), param is NULL\n", __FUNCTION__);
		return FAIL;
	}

	p->recv_state	= RRAS_IDLE;
	p->data_len	= 0;
	p->frame_ind	= 0;

	return SUCC;
}

static void wireless_rs485_rx_data(rt_device_t dev, rt_uint32_t select)
{
	int cnt, byte_data, index;
	rt_uint8_t *data_buf;

#define DATA_BUF_SIZE	256

	data_buf = rt_calloc(DATA_BUF_SIZE, 1);
	if (NULL == data_buf) {
		printf_syn("%s(), out of mem\n", __FUNCTION__);
		return;
	}

	index = 0;
	cnt = dev->read(dev, 0, data_buf, DATA_BUF_SIZE);
	while (0 != cnt--) {
		byte_data = data_buf[index++];
		rs485_recv_buf[anylasis_wl_485_frame.frame_ind++] = byte_data;

		switch (anylasis_wl_485_frame.recv_state) {
		case RRAS_IDLE:
			if ((RS485_HEAD_1TH + 1) == anylasis_wl_485_frame.frame_ind) {
				if (0x79 == byte_data) {
					rt_timer_start(frame_of_collect_wl_data_timer);
					continue;	
				} else {
					anylasis_wl_485_frame.data_len = 0;
					anylasis_wl_485_frame.frame_ind = 0;
					break;
				}
			} else if ((RS485_HEAD_2TH + 1) == anylasis_wl_485_frame.frame_ind) {
				rt_timer_stop(frame_of_collect_wl_data_timer);
				if (0x79 == byte_data) {
					anylasis_wl_485_frame.recv_state = RRAS_WAIT_DATA;
					rt_timer_start(frame_of_collect_wl_data_timer);
				} else {
					anylasis_wl_485_frame.data_len = 0;
					anylasis_wl_485_frame.frame_ind = 0;
				}
			} else {
				rt_timer_stop(frame_of_collect_wl_data_timer);
				rt_timer_start(frame_of_collect_wl_data_timer);
			}
			break;
			
		case RRAS_WAIT_DATA:
			rt_timer_stop(frame_of_collect_wl_data_timer);
			if ((RS485_DATA_LEN_OFFSET + 1) == anylasis_wl_485_frame.frame_ind) {
				anylasis_wl_485_frame.data_len = byte_data;
				rt_timer_start(frame_of_collect_wl_data_timer);
			} else if ((anylasis_wl_485_frame.frame_ind
					== (anylasis_wl_485_frame.data_len + RS485_FIX_LEN))) { /* 长度已达到一帧数据的完整长度 */
				byte_data = rs485_frame_checksum(rs485_recv_buf,
		                		   anylasis_wl_485_frame.data_len + RS485_CHECK_SUM_FIX_LEN);
				if (byte_data == rs485_recv_buf[anylasis_wl_485_frame.data_len + RS485_FIX_LEN - 2]) {
					if (select == 1) {
						wireless_rs485_data_proc(rs485_recv_buf);
					} else if (select == 2) {
						char sn[DEV_SN_MODE_LEN];

						get_devsn(sn, sizeof(sn));
						if (0 == rt_memcmp((rs485_recv_buf+ 1), sn, sizeof(sn))) {
							wireless_rs485_data_proc(rs485_recv_buf);
						}
					}
				} else {
					printf_syn(":%s(), recv frame check-sum error\n", __FUNCTION__);
				}

				anylasis_wl_485_frame.recv_state = RRAS_IDLE;
				anylasis_wl_485_frame.data_len = 0;
				anylasis_wl_485_frame.frame_ind = 0;
			} else {
				rt_timer_start(frame_of_collect_wl_data_timer);
			}
			break;
			
		default:
			printf_syn(":%s(), fsm-state error(%d)\n", __FUNCTION__, anylasis_wl_485_frame.recv_state);
			break;
		}
	}

	rt_free(data_buf);

	return;
}
