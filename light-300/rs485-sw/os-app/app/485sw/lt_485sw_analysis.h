/*
 ******************************************************************************
 * lt_485sw_analysis.h
 *
 *  Created on: 2015-01-26
 *      Author: David, zhaoshaowei@yeejoin.com
 *
 * COPYRIGHT (C) 2015, YEEJOIN (Beijing) Technology Development Co., Ltd.
 ******************************************************************************
 */

#ifndef OS_APP_APP_INCLUDE_LT_485SW_ANALYSIS_H_
#define OS_APP_APP_INCLUDE_LT_485SW_ANALYSIS_H_

#include <ringbuf.h>

enum protocol_analysis_proc_e {
	PAP_IDLE,

	PAP_FRAME_START_OK,	/* 帧起始字节正确 */
	PAP_RECV_ADDR,		/* 正在接收地址域数据 */
	PAP_RECV_ADDR_OK,	/* 地址域后的帧起始符正确 */
	PAP_RECV_CTRL_WORD,	/* 接收到控制字 */
	PAP_RECV_DATA_LEN,	/* 接收到数据域长度 */
	PAP_RECV_OTHER_DATA,	/* 正在接收其他数据 */
	PAP_FRAME_END_OK,	/* 接收完一个完整的数据帧 */

	PAP_FRAME_TIMEOUT,	/* 接收cmd时超时 */
	PAP_FRAME_ERR,		/* 接收cmd时发生错误 */

	/* 对ack不做分析 */
//	PAP_WAIT_ACK,		/* 等待ack数据 */
//	PAP_FORWARD_ACK_START,	/* 开始转发ack数据 */
//	PAP_FORWARD_TIMEOUT,	/* 转发ack数据时，等待超时 */
//	PAP_FORWARD_OVER	/* 转发ack完成 */
};

enum result_protoc_proc_e {
	RPP_NOTHING,
	RPP_ANALYSIS_UNKNOW_PROTOC,

	RPP_ANALYSIS_PENDING,		/* 2, 符合协议数据格式 */
	RPP_ANALYSIS_START_FORWARD,	/* 3, 是发送给本地em的cmd帧, 开始转发 */
	RPP_ANALYSIS_START_CONSUME,	/* 4, 格式正确，但不是是发送给本地em的cmd帧, 开始消费这些数据 */
	RPP_ANALYSIS_FORWARDING,	/* 5, 正在转发 */
	RPP_ANALYSIS_CONSUMING,		/* 6, 正在消费cmd数据 */
	RPP_ANALYSIS_OK,		/* 7, 命令帧分析完成 */
	RPP_ANALYSIS_OK_CONSUME,	/* 8, 命令帧分析完成consume */
	RPP_ANALYSIS_FORWARD_OVER,	/* 9, 命令帧已转发完成 */
	RPP_ANALYSIS_CONSUME_OVER,	/* 10, 命令帧已消费完成 */

	RPP_ANALYSIS_TIMEOUT,
	RPP_ANALYSIS_ERR,

	RPP_ACK_FORWARD_WAITING,
	RPP_ACK_FORWARD_TIMEOUT,	/* 14 */
	RPP_ACK_FORWARD_OVER,		/* 15 */

	RPP_TL16_WAIT_IF_LU_SEM_TIMEOUT,
	RPP_EMC_WAIT_IF_LU_SEM_TIMEOUT,
	RPP_TL16_SEND_MAILB_ERR,
	RPP_EMC_SEND_MAILB_ERR,
	RPP_EMC_SEND_MAILB_ERR_HAD_SEM,	/* 20 */
	RPP_TL16_LONGTIME_USE_EMIF,
	RPP_TL16_ORDER_USE_EMIF,
	RPP_EMC_LONGTIME_USE_EMIF,
	RPP_TL16_INVALID_IF_GRP_NO_OF_EM,
	RPP_EMC_INVALID_IF_GRP_NO_OF_EM,	/* 25 */
	RPP_TL16_UNKNOW_ERR,
	RPP_EMC_UNKNOW_ERR,
	RPP_ACK_UNKNOW_ERR
};


#define LT_485SW_FRAME_WAIT_TIMEOUT_MS		(510)
#define LT_485SW_BYTE_WAIT_TIMEOUT_MS		(510)

#define PROTOC_RX_TX_BUF_SIZE (128)
struct protoc_analy_info_st {
	enum lt_485if_e lt_if;	/* 上行485口 */
	enum lt_485if_e lt_if_use_by_em;	/* 下行485口 */
	enum ammeter_protocol_e protocol;
	enum protocol_analysis_proc_e state;
	enum result_protoc_proc_e rstate;
	char em_addr_offset;	/* em地址起始字节在帧中的偏移 */
	char if_had_get_lu_sem;	/* 1 -- use em_grp1_if_lu_sem, 2 -- use em_grp2_if_lu_sem */
	int data_zone_len;	/* 记录cmd帧的数据域长度, 用于判断帧的结束 */
	int had_recv_data_cnt;
	int frame_pos;		/* 0-invalid, 1--start forward, 2--end */
	struct rt_mutex emcx_if_mutex; /* 用于保护连接采集器/EMC的串口 */

	struct ringbuf rb;
};




#define PROTOCL_ANALYS_INFO_MAX	(LT_485_EMC_UART+1)

extern struct protoc_analy_info_st proto_analy_info[PROTOCL_ANALYS_INFO_MAX];
extern struct rt_timer protoc_inter_byte_timer[PROTOCL_ANALYS_INFO_MAX];

extern enum lt_485if_e protoc_inter_byte_timer_param[PROTOCL_ANALYS_INFO_MAX];

extern void init_protoc_analy_rb_sys(struct protoc_analy_info_st *p, char *name,
		enum lt_485if_e rs485_if);
extern void init_protoc_analy_info(struct protoc_analy_info_st *p, enum lt_485if_e rs485_if);

extern enum result_protoc_proc_e proc_frame(struct protoc_analy_info_st *analys_info,
		enum lt_485if_e rs485_if, int *had_proc);
extern void protoc_inter_byte_timeout(void *param);

#endif /* OS_APP_APP_INCLUDE_LT_485SW_ANALYSIS_H_ */
