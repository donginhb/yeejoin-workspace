/**********************************************************************************
* Filename   : frame_maser.h
* Discription : define the fucntions that read the ammeters
* Begin time  : 2014-2-27
* Finish time : 
* Engineer    : 
* Version      : V1.0
*************************************************************************************/


#ifndef FRAME_EM_H
#define FRAME_EM_H


#include "rtdef.h"
#include <rs485_common.h>
#include <syscfgdata.h>
#if RT_USING_TCPIP_STACK
#include <lwip/inet.h>

#define RT_CTRL_RTGHT      2             /*IP帧分发包后收到的回复帧确认控制字*/
#define RT_CTRL_WRONG      3            /*IP帧分发包后收到的回复帧否认控制字*/

#define SEV_PORT 5000
#define EM_FRAME_LEN 71
#define TRY_TIMES	1

#define NET_DATA_LEN           64
#define NET_PKT_LEN            71
#define NET_NORMAL_REP	       0
#define NET_ERROR_REP	       1
#define NET_PKT_LEN_REP	       6
#define NET_PKT_SN_OFFSET      7
#define NET_PKT_APP_OFFSET_A   (43)
#define NET_PKT_APP_OFFSET_B   (47)
#define NET_PKT_APP_OFFSET_C   (51)
#define NET_PKT_AP_OFFSET_A    (55)
#define NET_PKT_AP_OFFSET_B    (59)
#define NET_PKT_AP_OFFSET_C    (63)
#define NET_PKT_V_OFFSET_A     (19)
#define NET_PKT_V_OFFSET_B     (23)
#define NET_PKT_V_OFFSET_C     (27)
#define NET_PKT_I_OFFSET_A     (31)
#define NET_PKT_I_OFFSET_B     (35)
#define NET_PKT_I_OFFSET_C     (39)

/********* 解析tcp *********/
enum tcp_transport_err_e {
	TTE_EOK   		= 0x00,
	TTE_CREATE_SOCKET_FAIL,
	TTE_CONNECT_FAIL,
	TTE_SEND_FAIL,
	TTE_RECV_REP_FAIL,
	TTE_DONT_WRITE,
	TTE_DONT_READ,
	TTE_MALLOC_FAIL,
};

/*** 帧格式控制码 ***/
enum frame_ctrl_em {
	FIPCMD_SEND_DATA   		= 0x01, /* 00001：主电表采集器分发PT、CT数据给相应的电表采集器所用控制字*/
	FIPCMD_RESPOND_DATA             = 0x81, /*10000001:  电表采集器接收主电表采集器数据后的回复帧控制字*/
};

/*** 帧格式各参数 ***/
struct frame_format_em {
	rt_uint8_t head1;
	rt_uint8_t head2;
	rt_uint8_t head3;
	rt_uint8_t head4;
        enum frame_ctrl_em ctrl;	/* 控制码 */
        rt_uint16_t data_len;		/* 发送/接收的数据长度, 不是缓冲区的长度 */
	rt_uint8_t err;
};

struct em_distr_ms {
	rt_uint8_t pt_ct_sn[DEV_SN_MODE_LEN];
	s32_t pt_ct_v[3];
	s32_t pt_ct_i[3];
	s32_t pt_ct_app_p[3];
	s32_t pt_ct_ap_p[3];
};

extern rt_err_t create_frame_em(rt_uint8_t *send_frame_buf, struct frame_format_em *frame_em);

#endif


#endif