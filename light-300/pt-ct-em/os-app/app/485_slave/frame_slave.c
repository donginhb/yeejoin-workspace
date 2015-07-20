/**********************************************************************************
* Filename   : frame_485.c
* Discription : define the fucntions that read the ammeters
* Begin time  : 2014-2-27
* Finish time : 
* Engineer    : 
* Version      : V1.0
*************************************************************************************/
/*
 ******************************************************************************
 * frame_485.c
 *
 * 2014-2-27,  creat by fanfuchao, fanfuchao@yeejoin.com
 ******************************************************************************
 */
#include <rtthread.h>
#include <board.h>
#include <rs485.h>
#include <sys_cfg_api.h>
#include <frame_slave.h>
#include <rs485_common.h>
#include <master_fsm.h>

#ifdef RT_USING_FINSH
#include <finsh.h>
#endif

extern rt_uint8_t *rs485_send_buf_s;
extern rt_uint8_t *rs485_recv_buf;

#define frame_slave_info(x) printf_syn x

struct msfd_report_collection_data_sn_st *pc_collect_data_s;

//struct wireless_register_white_st *wireless_white_register;

struct register_slave_node_original_info_st *original_info;


//rt_uint8_t echo_id_s = 0;


int rs485_slave_init(void)
{
        rt_thread_t thread_h;

	rs485_send_buf_s = (rt_uint8_t *)rt_malloc(FRAME_485_LEN);
	if (rs485_send_buf_s == RT_NULL) {
		frame_slave_info(("func:%s(), line:%d malloc fail\n",__FUNCTION__, __LINE__));
		return FAIL;	
	}
	rt_memset(rs485_send_buf_s, 0, FRAME_485_LEN);

	pc_collect_data_s = rt_malloc(sizeof(struct msfd_report_collection_data_sn_st)*SLAVE_NODE_INFO_MAX_LEN);
	if (NULL == pc_collect_data_s) {
		rt_free(rs485_send_buf_s);
		frame_slave_info(("func:%s(), line:%d malloc fail\n",__FUNCTION__, __LINE__));
		return FAIL;
	}
	rt_memset(pc_collect_data_s, 0, sizeof(struct msfd_report_collection_data_sn_st)*SLAVE_NODE_INFO_MAX_LEN);
#if 0
	wireless_white_register = rt_malloc(sizeof(struct wireless_register_white_st)*SLAVE_NODE_INFO_MAX_LEN);
	if (NULL == wireless_white_register) {
		rt_free(rs485_send_buf_s);
		rt_free(pc_collect_data_s);
		frame_slave_info(("func:%s(), line:%d malloc fail\n",__FUNCTION__, __LINE__));
		return FAIL;
	}
	rt_memset(wireless_white_register, 0, sizeof(struct wireless_register_white_st)*SLAVE_NODE_INFO_MAX_LEN);
#endif

	original_info = rt_malloc(sizeof(struct register_slave_node_original_info_st)*WIRELESS_SLAVE_NODE_MAX);
	if (original_info == RT_NULL) {
		rt_free(rs485_send_buf_s);
		rt_free(pc_collect_data_s);
		//rt_free(wireless_white_register);
		frame_slave_info(("func:%s(), line:%d malloc fail\n",__FUNCTION__, __LINE__));
	}
	rt_memset(original_info, 0, sizeof(struct register_slave_node_original_info_st)*WIRELESS_SLAVE_NODE_MAX);
 	
        thread_h = rt_thread_create("rs485_s",wireless_rs485_recv_entry, (void *)2, 1024, 24, 10);/* rs485 slave recv thread */
	
        if (thread_h != RT_NULL) {
		rt_thread_startup(thread_h);
	} else {
		rt_free(rs485_send_buf_s);
		rt_free(pc_collect_data_s);
		//rt_free(wireless_white_register);
		rt_free(original_info);
		frame_slave_info(("func:%s(), line:%d create thread fail\n",__FUNCTION__, __LINE__));
	}

        return 0;
}

void rs485_send_rep_probe_cmd(rt_uint8_t *tx_buf, rt_uint8_t *src_addr, rt_uint8_t *data, rt_uint8_t data_len,
				struct rs485_frame_format *format, enum rs485_frame_ctrl ctrl)
{
	rs485_create_frame_head(data_len, src_addr, ctrl, format);
	rs485_package_data(tx_buf, FRAME_485_LEN, format, data);
	send_data_by_485(RS485_PORT_USED_BY_WIRELESS, tx_buf, (RS485_FIX_LEN + format->data_len));
}

void rs485_send_rep_netcfg_cmd(rt_uint8_t *tx_buf, rt_uint8_t *src_addr, rt_uint8_t *data, rt_uint8_t data_len,
				struct rs485_frame_format *format)
{
	rs485_create_frame_head(data_len, src_addr, SD_SEND_NETCFG_CMD_REP, format);
	rs485_package_data(tx_buf, FRAME_485_LEN, format, data);
	send_data_by_485(RS485_PORT_USED_BY_WIRELESS, tx_buf, (RS485_FIX_LEN + format->data_len));
}

void rs485_send_rep_read_master_data_cmd(rt_uint8_t *tx_buf, rt_uint8_t *src_addr, rt_uint8_t *data, rt_uint8_t data_len,
					struct rs485_frame_format *format, enum rs485_frame_ctrl ctrl)
{
	rs485_create_frame_head(data_len, src_addr, ctrl, format);
	rs485_package_data(tx_buf, FRAME_485_LEN, format, data);
	send_data_by_485(RS485_PORT_USED_BY_WIRELESS, tx_buf, (RS485_FIX_LEN + format->data_len));
}

void rs485_send_rep_echo_cmd(rt_uint8_t *tx_buf, rt_uint8_t *src_addr, rt_uint8_t *echo, struct rs485_frame_format *format)
{
	rs485_create_frame_head(sizeof(rt_uint8_t), src_addr, SD_SEND_ECHO_CMD_REP, format);
	rs485_package_data(tx_buf, FRAME_485_LEN, format, echo);
	send_data_by_485(RS485_PORT_USED_BY_WIRELESS, tx_buf, (RS485_FIX_LEN + format->data_len));
}

void rs485_send_rep_stop_echo_cmd(rt_uint8_t *tx_buf, rt_uint8_t *src_addr, struct rs485_frame_format *format)
{
	rs485_create_frame_head(0, src_addr, SD_SEND_ECHO_STOP_CMD_REP, format);
	rs485_package_data(tx_buf, FRAME_485_LEN, format, RT_NULL);
	send_data_by_485(RS485_PORT_USED_BY_WIRELESS, tx_buf, (RS485_FIX_LEN + format->data_len));
}

void rs485_send_rep_stop_probe_cmd(rt_uint8_t *tx_buf, rt_uint8_t *src_addr, struct rs485_frame_format *format)
{
	rs485_create_frame_head(0, src_addr, SD_SEND_STOP_PROBE_CMD_REP, format);
	rs485_package_data(tx_buf, FRAME_485_LEN, format, RT_NULL);
	send_data_by_485(RS485_PORT_USED_BY_WIRELESS, tx_buf, (RS485_FIX_LEN + format->data_len));
}
