/*
 ******************************************************************************
 * ms_common.h
 *
 *  Created on: 2013-12-18
 *      Author: David, zhaoshaowei@yeejoin.com
 *
 * 2014-03-05：
 * 	David: 增加简单的si4432传输层的抽象
 *
 * COPYRIGHT (C) 2013, YEEJOIN (Beijing) Technology Development Co., Ltd.
 ******************************************************************************
 */

#ifndef MS_COMMON_H__
#define MS_COMMON_H__


#include <compiler_defs.h>
#include <ezmacpro_common.h>

#include <syscfgdata-common.h>


#define MS_COMMON_SUCC	0
#define MS_COMMON_WAIT	1
#define MS_COMMON_FAIL	2
enum ms_result_state_e {
	MRS_RESULT_SUCC	= 0,
	MRS_RESULT_PENDIND,
	MRS_RESULT_FAIL,
	MRS_RESULT_TIMEOUT,/* mark by zp */
	MRS_RESULT_STOP_ECHO,/* mark by zp */
	MRS_RESULT_START_ECHO,/* mark by zp */
	MRS_RESULT_DETECT,
	MRS_RESULT_DISTRIBUTE,
};
#if C51_SYNTAX_
typedef ubase_t ms_result_state_t;
#else
typedef enum ms_result_state_e ms_result_state_t;
#endif


/*
 * MNA_*由主节点主动发起, MNP_*由从节点主动发起, 每个命令都要有响应
 * MNA - master node active
 * MNP - master node passive
 *
 * */
enum master_slave_cmd_e {
	/* 主节点主动发起 */
	MNA_ECHO			= 0x00,
	MNA_STOP_ECHO				,
	MNA_DETECT_SLAVE_NODE			,	/* 探测可链接的从节点 */
	MNA_DISTRIBUTE_CFG_DATA			,	/* 无线主节点给从节点分发配置数据 */

	MNA_COLLECTION_DATA                   	,
	MNA_GET_STATE_INFO                      ,
	MNA_GET_ID_INFO                         ,

	/* 从节点主动发起 */
	MNP_PUSH_COLLECTION_DATA                ,	/* 将采集数据定时上报给master节点 */
	MNP_PUSH_STATE_INFO                     ,	/* 在需要时主动上报状态信息, 比如，电池欠压 */

	/* ---------------- 以下为响应命令 ---------------- */
	MNA_ECHO_REP			= 0X80,
	MNA_STOP_ECHO_REP			,
	MNA_DETECT_SLAVE_NODE_REP		,
	MNA_DISTRIBUTE_CFG_DATA_REP		,

	MNA_COLLECTION_DATA_REP               	,
	MNA_GET_STATE_INFO_REP                  ,
	MNA_GET_ID_INFO_REP                     ,

	MNP_PUSH_COLLECTION_DATA_REP            ,
	MNP_PUSH_STATE_INFO_REP                 ,
};

#if C51_SYNTAX_
typedef ubase_t master_slave_cmd_t;
#else
typedef enum master_slave_cmd_e master_slave_cmd_t;
#endif

enum master_slave_cmd_result_e {
	MSCR_OK,
	MSCR_ERROR,
};


enum master_slave_data_proc_err_e {
	MSDPE_OK,
	MSDPE_ERROR,

};
#if C51_SYNTAX_
typedef ubase_t ms_data_proc_err_t;
#else
typedef enum master_slave_data_proc_err_e ms_data_proc_err_t;
#endif

struct sid_did_pairs_t {
	ubase_t sid;
	ubase_t did;
};


enum slave_next_state_e {
	SNS_IDLE,
	SNS_SLEEP,
	SNS_ECHO_MODE,
	SNS_RECV_FILE_MODE,
	SNS_PUSH_CMD_MODE,
};
#if C51_SYNTAX_
typedef ubase_t slave_next_state_t;
#else
typedef enum slave_next_state_e slave_next_state_t;
#endif


/*
 * 应用数据包/帧格式如下:	帧头 + 帧数据
 *
 * 帧头结构定义见struct master_slave_data_frame_head_s, struct master_slave_rep_data_frame_head_s
 *
 * 帧数据：根据帧头中的cmd类型做不同的解释
 *
 * NOTE:网络序与本机序的转换!
 * */

/* 用户看到的无线节点ID与si4432实际使用的ID的差值 */
#define RF_TX_BUFFER_SIZE	(64)
#define RF_RX_BUFFER_SIZE	(64)

#define LIGHT300_SLAVE_NODE_ID_OFFSET	(10)
#define convert_usr_scope_id_to_ezmac_id(usr_scope_id)	((usr_scope_id) + LIGHT300_SLAVE_NODE_ID_OFFSET)
#define convert_ezmac_id_to_usr_scope_id(ezmac_id)	((ezmac_id) - LIGHT300_SLAVE_NODE_ID_OFFSET)


#pragma pack(1)

struct master_slave_data_frame_head_s {
	uint8_t cmd;	/* enum master_slave_cmd_e */
	uint8_t	len;	/* the length of the frame data */
	uint8_t result;	/* 该成员只对响应有效 */
};
#define MASTER_SLAVE_DATA_FREAM_HEAD_LEN	(sizeof(struct master_slave_data_frame_head_s)-1)
#define MASTER_SLAVE_DATA_REP_FREAM_HEAD_LEN	(sizeof(struct master_slave_data_frame_head_s))

/*
 * msfd -- master-slave frame data
 * */
/* -------- 1. 探测从节点 */
/* --- request */
/* --- response */
struct msfd_probe_st {
	uint8_t sn[DEV_SN_MODE_LEN];	/* request--主节点sn, response--从节点sn */
};

/* --- indication -- 发送收到响应指示 */
struct msfd_probe_rep_indication_st {
	uint8_t sn[DEV_SN_MODE_LEN];	/* 从节点sn */
};


/* -------- 2. 下发配置数据 */
/* --- request */
struct msfd_distribute_cfgdata_st {
	uint8_t sn[DEV_SN_MODE_LEN];	/* 用于从节点发送注册请求时, 存储从节点sn */
	uint8_t slave_id;		/* 用于主节点响应注册请求时, 返回节点id */
	uint8_t channel_no;
	uint8_t syn_word[2];
	uint8_t cid;
};

/* --- response, 此时从节点已有ID, 所以, 无需携带数据 */


/* -------- 3. 从节点上报采集数据 */
/* --- request */
struct msfd_report_collection_data_st {
	s32_t pt_ct_v[3];	/* abc三相电压, s24 -> 32zp */
	s32_t pt_ct_i[3];	/* abc三相电流, s24 -> 32zp */
	s32_t pt_ct_app_p[3];	/* 视在功率(apparent power), s24 -> 32se */ /* mark by zp */
	s32_t pt_ct_ap_p[3];	/* 有功功率 */
};

struct msfd_report_collection_data_sn_st {
	uint8_t sn[DEV_SN_MODE_LEN];
	struct msfd_report_collection_data_st coll_data;
	rt_tick_t last_update_time;
};

/* --- response */
struct msfd_report_collection_data_rep_st {
	uint8_t timegap_sec[2];	/* 上报采集数据的时间间隔, 单位是秒, 2字节整数, 网络序 */
	/* 还可以携带从节点下一个状态 */
};


/* -------- 4. 从节点上报状态信息 */
/* --- request */
struct msfd_report_slave_node_state_st {
	uint8_t state;
};

struct record_netcfg_fail_data_st {
	uint8_t sn[DEV_SN_MODE_LEN];
};

/* --- response,  只需应答命令, 无数据 */


/* -------- 5. echo */
/* --- request, 携带一个字节的计数值 */
/* --- response, 携带接收到的计数值 */


#pragma pack()


#define LIGHT300_MASTER_SLAVE_CID	(0x3a)
#define LIGHT300_MASTER_SLAVE_MCAST	(0xFE)
#define LIGHT300_MASTER_SLAVE_BROADCAST	(EZMAC_BROADCAST_ADDR)
#define LIGHT300_MASTER_SLAVE_DEFAULT_SYN_WORD	(0X2DD4)

#define LIGHT300_MASTER_SFID                (0x00)

#define DEMO_SLAVE_SFID                     (LIGHT300_SLAVE_NODE_ID_OFFSET + 0)

#define MASTER_SLAVE_ECHO_START	1
#define MASTER_SLAVE_ECHO_WORK	2
#define MASTER_SLAVE_ECHO_STOP	3
#define MASTER_SLAVE_ECHO_IDLE	0


extern int package_master_slave_data(uint8_t *pkt, uint8_t pkt_len,
		struct master_slave_data_frame_head_s *h, void *send_data);

extern int unpackage_master_slave_data(uint8_t *pkt, uint8_t pkt_len,
		struct master_slave_data_frame_head_s *h, void *recv_data, const uint8_t recv_len);

extern int ms_prepare_into_ezmacpro_recv_state(void);

extern unsigned char get_rand_num4delay(void);/* mark by zp */

extern rt_mailbox_t si4432_recv_mb;/* mark by zp */

#if BYTE_ORDER == LITTLE_ENDIAN

#if defined(RT_USING_LWIP)
#include <lwip/def.h>

#define ms_htons(n)	lwip_htons(n)
#define ms_ntohs(n)	lwip_ntohs(n)
#define ms_htonl(n)	lwip_htonl(n)
#define ms_ntohl(n)	lwip_ntohl(n)
#else /* !defined(RT_USING_LWIP) */
u16_t ms_htons(u16_t n);
u16_t ms_ntohs(u16_t n);
u32_t ms_htonl(u32_t n);
u32_t ms_ntohl(u32_t n);
#endif /* !defined(RT_USING_LWIP) */

#else /* BYTE_ORDER == LITTLE_ENDIAN */

#define ms_htons(n)	(n)
#define ms_ntohs(n)	(n)
#define ms_htonl(n)	(n)
#define ms_ntohl(n)	(n)
#endif /* BYTE_ORDER == LITTLE_ENDIAN */


#define ref_vptr(vptr, ptr_type)	(*(ptr_type)vptr)
#define convert_vptr(vptr, ptr_type)	((ptr_type)vptr)


/*
 * si4432不能进行全双工通信，该状态机是简单传输层的抽象(并发度不高，只有进行完一次会话后才能进行下一次会话)
 **/
enum si4432_transport_err_e {
	STE_OK	= 0,
	STE_WAIT_REQUEST_TIMEOUT,

	STE_FAIL,
	STE_PTR_NULL,
	STE_STATE_CMD_NOT_MATCH,
	STE_EZMAC_BUSY,
	STE_SEND_FAIL,
	STE_WAIT_REP_TIMEOUT,
	STE_PENDING,
	STE_CHECK_RECV_STATUS,

	STE_TX_CHANNEL_BUSY,
	STE_TX_ERROR_STATE,
	STE_RX_ERROR_STATE,

	STE_BUTT

};
#if C51_SYNTAX_
typedef ubase_t si4432_transport_err_t;
#else
typedef enum si4432_transport_err_e si4432_transport_err_t;
#endif
enum si4432_transport_fsm_e {
	STF_IDLE	= 0,

	STF_SEND_REQUEST,	/* 该无线节点主动向外发送请求 */
	STF_WAIT_REQUEST_REP,	/* 该无线节点等待某无线节点对自己请求的响应 */

	STF_WAIT_REQUEST,	/* 该无线节点等待某无线节点发送的请求 */
	STF_SEND_REQUEST_REP,	/* 该无线节点发送请求的响应 */

	STF_BUTT
};
#if C51_SYNTAX_
typedef ubase_t si4432_transport_fsm_t;
#else
typedef enum si4432_transport_fsm_e si4432_transport_fsm_t;
#endif

enum si4432_transport_data_cmd_e {
	STDC_SEND_REQUEST,	/* 发送请求 */
	STDC_WAIT_REQUEST_REP,	/* 等待请求响应 */
	STDC_WAIT_MULTI_REQUEST_REP,	/* 等待多个请求响应 */

	STDC_WAIT_REQUEST,	/* 等待请求 */
	STDC_SEND_REQUEST_REP,	/* 发送请求响应 */

	STDC_BUTT
};
#if C51_SYNTAX_
typedef ubase_t si4432_transport_data_cmd_t;
#else
typedef enum si4432_transport_data_cmd_e si4432_transport_data_cmd_t;
#endif


struct si4432_trans_data_info{
	struct sid_did_pairs_t  ms_id;
	ubase_t 	buflen;		/* 缓冲区长度 */
	ubase_t 	datalen;	/* 帧头 + 帧数据 的总长度 */
	union {
		unsigned char 	*tx_buf;
		unsigned char 	*rx_buf;
	} databuf;

	ubase_t  	rssi;	/* 无线信号强度 */
	ubase_t 	misc_info;
};

enum si4432_recv_timeout_e {
	SRTE_RECEIVED = 0,
	SRTE_TIMEOUT,
};

#define STDI_WAIT_MULTI_REQUEST_REQ_BIT	0X01
#define STDI_MULTI_FRAME_PKT_BIT	0X02

#define INVALID_RSSI_VALUE	(0x0)
#define INVALID_WIRELESS_MAC_ID	(0XFF)


/* datainfo是结构体变量 */
#define is_stdi_datainfo_bit_set(datainfo, member, bit_mask)	((datainfo).(member) & (bit_mask))
#define set_stdi_datainfo_bit(datainfo, member, bit_mask)	((datainfo).(member) |= (bit_mask))
#define clr_stdi_datainfo_bit(datainfo, member, bit_mask)	((datainfo).(member) &= ~(bit_mask))

#define is_wait_multi_request_req(datainfo) ((datainfo).misc_info & STDI_WAIT_MULTI_REQUEST_REQ_BIT)
#define set_wait_multi_request_req_bit(datainfo) ((datainfo).misc_info |= STDI_WAIT_MULTI_REQUEST_REQ_BIT)
#define clr_wait_multi_request_req_bit(datainfo) ((datainfo).misc_info &= ~STDI_WAIT_MULTI_REQUEST_REQ_BIT)

#define is_pkt_have_multi_frame(datainfo)	is_stdi_datainfo_bit_set(datainfo, misc_info, STDI_MULTI_FRAME_PKT_BIT)
#define set_pkt_have_multi_frame_bit(datainfo)	set_stdi_datainfo_bit(datainfo, misc_info, STDI_MULTI_FRAME_PKT_BIT)
#define clr_pkt_have_multi_frame_bit(datainfo)	clr_stdi_datainfo_bit(datainfo, misc_info, STDI_MULTI_FRAME_PKT_BIT)

#define get_dev_sn_hex_list(sn)		(sn)[0],(sn)[1],(sn)[2],(sn)[3],(sn)[4],(sn)[5]

#define is_slave_id_valid(id) ((id) != INVALID_WIRELESS_MAC_ID)

extern si4432_transport_err_t si4432_transport_data(si4432_transport_data_cmd_t cmd, struct si4432_trans_data_info *data_info);
extern base_t init_si4432_transport_fsm(void);
extern int init_si4432_trans_data_info_st(struct si4432_trans_data_info *data_info);
extern int init_master_slave_frame_head(struct master_slave_data_frame_head_s *h);
extern int get_self_sn(uint8_t *sn, base_t len);
extern base_t is_sn_same_as_self_sn(uint8_t *sn);
extern base_t get_wireless_cfgdata(struct msfd_distribute_cfgdata_st *p);
extern void _init_si4432_transport_fsm(void);

#endif /* MS_COMMON_H_ */
