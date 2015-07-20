/**********************************************************************************
* Filename   	: ammeter_edmi.h
* Description 	: edmi ammeter Arguments
* Begin time  	: 2014-07-31
* Finish time 	:
* Engineer		: zhanghonglei
*************************************************************************************/
#ifndef __AMMETER_EDMI_H__
#define __AMMETER_EDMI_H__

#include <ammeter_usart.h>

/*** 帧格式控制码 ***/
typedef enum frame_edmi_ctrl {
	EDMI_CMD_PATTERN   						= 0x0,  /* 命令模式 */
	EDMI_CMD_LOGIN   						= 0x4C, /* 登录命令 */
	EDMI_CMD_EXIT   						= 0x58, /* 退出命令 */
	EDMI_CMD_REGISTER_INFO  				= 0x49, /* 查询寄存器信息 */
	EDMI_CMD_READ_REGISTER   				= 0x52, /* 读寄存器 */
	EDMI_CMD_WRITE_REGISTER    				= 0x57, /* 写寄存器 */
	EDMI_CMD_READ_EXTEND_REGISTER   		= 0x4D, /* 读扩展寄存器 */
	EDMI_CMD_WRITE_EXTEND_REGISTER    		= 0x4E, /* 写扩展寄存器 */
	EDMI_CMD_WRITE_EXTEND_INFO    			= 0x30, /* 返回寄存器的信息 */
	EDMI_CMD_READ_MORE_EXTEND_REGISTER   	= 0x41, /* 读多个扩展寄存器 */
	EDMI_CMD_WRITE_MORE_EXTEND_REGISTER   	= 0x42, /* 写多个扩展寄存器 */
} frame_edmi_ctrl_e;


/*** 帧格式各参数 ***/
typedef struct frame_edmi_param
{
//	rt_uint32_t src_addr;         	/* 源地址 */
	rt_uint8_t dest_addr[6];        	/* 目的地址 */
	rt_uint16_t serial_num;        	/* 序列号 */
	frame_edmi_ctrl_e ctrl;	      	/* 控制码 */
	rt_uint32_t data_flag;			/* 数据标识 */
	rt_uint8_t *data;				/* 存储发送/接收数据的起始地址 */
	rt_uint8_t data_len;			/* 发送/接收的数据长度, 不是缓冲区的长度 */
} frame_edmi_param_t;

extern enum frame_error_e transmit_ammeter_edmi_data(frame_edmi_param_t *send_frame_data, frame_edmi_param_t *recv_frame_data, enum ammeter_uart_e port);

#endif
