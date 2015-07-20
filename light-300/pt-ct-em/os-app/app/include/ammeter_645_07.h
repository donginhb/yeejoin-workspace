/**********************************************************************************
* Filename   	: ammeter_645_07.h
* Description 	: 645-2007 ammeter Arguments
* Begin time  	: 2013-10-10
* Finish time 	: 
* Engineer		: zhanghonglei
*************************************************************************************/
#ifndef __AMMETER_645_07_H__
#define __AMMETER_645_07_H__

#include <ammeter_common.h>
#include <ammeter_usart.h>


/*** 帧格式控制码 ***/
enum frame645_07ctrl_e {
	F645_07CMD_READ_DATA   					= 0x11,		/* 读数据 */
	F645_07CMD_READ_FOLLOW_DATA  			= 0x12, 	/* 读后续数据 */
	F645_07CMD_WRITE_DATA    				= 0x14, 	/* 写数据 */
	F645_07CMD_READ_DEV_ADDR  				= 0x13, 	/* 读通信地址*/
	F645_07CMD_WRITE_DEV_ADDR  				= 0x15, 	/* 写通信地址*/
	F645_07CMD_BROADCASTING_TIME  			= 0x08, 	/* 广播校时 */
	F645_07CMD_FREEZING_CMD					= 0x16, 	/* 冻结命令 */
	F645_07CMD_CHANGE_BAUD   				= 0x17, 	/* 更改通信速率 */
	F645_07CMD_CHANGE_PASSWORD   			= 0x18, 	/* 修改密码 */
	F645_07CMD_MAXNEED_CLEAR   				= 0x19, 	/* 最大需量清零 */
	F645_07CMD_AMMETER_CLEAR   				= 0x1A, 	/* 电表清零 */
	F645_07CMD_EVENT_CLEAR   				= 0x1B, 	/* 事件清零 */
	F645_07CMD_TRIPPING_ALARM_SECURITY 		= 0x1C, 	/* 跳合闸、报警、保电 */
	F645_07CMD_MUX_FUN_OUTPUT_CMD 			= 0x1D, 	/* 多功能端子输出控制命令 */
	F645_07CMD_SECURITY_AUTHENTICATION_CMD 	= 0x03, 	/* 安全认证命令 */

	F645_07CMD_NORMAL_RESPONSE     			= 0x90, 	/* 从站正常应答无后续数据 */
 	F645_07CMD_ABNORMAL_RESPONSE   			= 0xD0, 	/* 从站异常应答 */
 	F645_07CMD_NORMAL_RESPONSE_FOLLOW_DATA 	= 0xB0, 	/* 从站正常应答有后续数据 */
};



/* 负荷记录 */
enum frame645_07_load_record_e {

	LOAD_RECORD_NO = 0X00,
	LOAD_RECORD_BLOCKS_NUM = 0X01,
	LOAD_RECORD_BLOCKS_NUM_AND_TIME = 0X06,
};

struct frame645_07_load_record {
	enum frame645_07_load_record_e m;
	rt_uint8_t block_num;
	rt_uint8_t load_record_time[5];
};

/*** 帧格式各参数 ***/
struct send_frame_param
{
	rt_uint8_t addr[6];            	/* 地址 */
	enum frame645_07ctrl_e ctrl;	/* 控制码 */
	rt_uint32_t data_flag;			/* 数据标识 */
	rt_uint32_t password;			/* 电能表密码 */
	rt_uint32_t operator_code;		/* 操作者代码 */
	rt_uint8_t *data;				/* 存储发送/接收数据的起始地址 */
	rt_uint8_t data_len;			/* 发送/接收的数据长度, 不是缓冲区的长度 */

	rt_uint8_t frame_order_num; 	/* 帧序号 */
};

/*** 帧格式各参数 ***/
struct recv_frame_param
{
	rt_uint8_t addr[6];            	/* 地址 */
	enum frame645_07ctrl_e ctrl;	/* 控制码 */
	rt_uint32_t data_flag;			/* 数据标识 */
	rt_uint8_t *data;				/* 存储发送/接收数据的起始地址 */
	rt_uint8_t data_len;			/* 发送/接收的数据长度, 不是缓冲区的长度 */

	rt_uint8_t frame_order_num;    	/* 帧序号 */
};
extern enum frame_error_e transmit_ammeter645_07_data(struct send_frame_param *send_frame_data, struct recv_frame_param *recv_frame_data, enum ammeter_uart_e port);

#endif
