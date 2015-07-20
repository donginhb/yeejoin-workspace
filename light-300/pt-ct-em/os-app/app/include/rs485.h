/*
 ******************************************************************************
 * rs485.h
 *
 * 2013-10-15,  creat by David, zhaoshaowei@yeejoin.com
 ******************************************************************************
 */

#ifndef RS485_H_
#define RS485_H_

#include "stm32f10x.h"/* mark by zp */
#include <ms_common.h>
#include <syscfgdata-common.h>

#define RS485_FIX_LEN 	      (DEV_SN_MODE_LEN + 6)

#define RS485_CHECK_SUM_FIX_LEN   16
#define RS485_CTL_OFFSET          14
#define RS485_DATA_LEN_OFFSET     15
#define RS485_DATA_OFFSET         16
#define RS485_HEAD_1TH	          0
#define RS485_HEAD_2TH            13

#define SLAVE_485_MAX_NUM 4

#define SLAVE_NODE_INFO_MAX_LEN  50
struct recv_original_slave_node_info_st {
	uint8_t sn[DEV_SN_MODE_LEN];
	uint8_t rssi;
	uint8_t pad[3];
};

struct master_slave_original_pairs_table_st {
	rt_uint8_t src_sn[DEV_SN_MODE_LEN];
	struct recv_original_slave_node_info_st slave_node_info[SLAVE_NODE_INFO_MAX_LEN];
};

struct send_proc_slave_node_info_st {
	uint8_t sn[DEV_SN_MODE_LEN];
};

struct master_slave_proc_pairs_table_st {
	rt_uint8_t src_sn[DEV_SN_MODE_LEN];
	struct send_proc_slave_node_info_st slave_node_info[SLAVE_NODE_INFO_MAX_LEN];
};

/*** 485 帧格式控制码 ***/
enum rs485_frame_ctrl {
	/* 主站点主动发起 */
	MD_SEND_READ_MASTER_DATA_CMD  		= 0x01,/* 00001：主站发"读无线主节点采集到的数据 "命令帧*/
	MD_SEND_NETCFG_CMD			= 0x02,/* 00010：配置无线从节点上报数据时间间隔*/
	MD_SEND_PROBE_CMD 		        = 0x03,/* 00011：探测功能*/
	MD_SEND_ECHO_CMD			= 0x04,/* echo cmd */
	MD_SEND_STOP_ECHO_CMD			= 0x05,
	MD_SEND_STOP_PROBE_CMD			= 0x06,/* stop probe and goto netcfg */
	MD_SEND_READ_MASTER_DATA_SUB_CMD	= 0x07,
	MD_SEND_PROBE_SUB_CMD			= 0x08,

	/* 从站点主动发起 */
	SD_SEND_READ_MASTER_DATA_CMD_REP        = 0x81,/*10000001:  从站答"读无线主节点采集到的数据 "命令*/
	SD_SEND_NETCFG_CMD_REP                  = 0x82,
	SD_SEND_PROBE_CMD_REP			= 0x83,/*1100 0011:探测功能回复*/
	SD_SEND_ECHO_CMD_REP			= 0x84,
	SD_SEND_ECHO_STOP_CMD_REP		= 0x85,
	SD_SEND_READ_MASTER_DATA_SUB_CMD_REP    = 0x86,/*10100001:  从站答"读无线主节点采集到的数据 "命令后续帧*/
	SD_SEND_STOP_PROBE_CMD_REP		= 0x87,
	SD_SEND_PROBE_SUB_CMD_REP		= 0x88,


};

enum em_send_to_pt_cmd_e {/* mark by zp */
	ESTP_PROBE_SLAVE_CMD = 0,
	ESTP_STOP_PROBE_SLAVE_CMD,
	ESTP_NETCFG_CMD,
	ESTP_ECHO_CMD,
	ESTP_RECV_COLLECT_DATA_CMD,
};

struct rs485_frame_format
{
	rt_uint8_t start1;
	rt_uint8_t start2;
        rt_uint8_t ctrl;	/* 控制码 */
	rt_uint8_t data_len;	/* 发送/接收的数据长度, 不是缓冲区的长度 */
        rt_uint8_t *addr;            	/* 地址 */
};

enum rs485_recv_analysis_state {
	RRAS_IDLE = 0,
	RRAS_WAIT_DATA,
	RRAS_RECV_DATA_OVER,
};

extern struct rt_semaphore uart485_1_rx_byte_sem;
extern struct rt_semaphore uart485_2_rx_byte_sem;
extern struct rt_semaphore uart485_3_rx_byte_sem;
extern rt_mailbox_t uart485_mb;/* mark by zp */

extern void init_sys_485(void);
extern rt_size_t send_data_by_485(USART_TypeDef *dev_485, void *data, rt_size_t len);
extern rt_size_t recv_data_by_485(USART_TypeDef *dev_485, void *buf, rt_size_t len);
extern void  wireless_rs485_recv_entry(void *parameter);
extern void rs485_create_frame_head(rt_uint8_t data_len, rt_uint8_t *sn, enum rs485_frame_ctrl ctrl, 
				struct rs485_frame_format *format);
extern void rs485_package_data(rt_uint8_t *pkt_data, rt_uint8_t pkt_len, struct rs485_frame_format *format,
				rt_uint8_t *data);

extern void clr_rx_buf_recv_data_by_485(USART_TypeDef *dev_485);
#endif
