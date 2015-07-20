/**********************************************************************************
* Filename   	: ammeter_645_97.h
* Description 	: 645-1997 ammeter Arguments
* Begin time  	: 2013-10-10
* Finish time 	: 
* Engineer		: zhanghonglei
*************************************************************************************/
#ifndef __AMMETER_645_97_H__
#define __AMMETER_645_97_H__

#include <ammeter_common.h>
#include <ammeter_usart.h>



/*** 帧格式控制码 ***/
enum frame645_97ctrl_e {
	F645_97CMD_READ_DATA   					= 0x01, /* 00001：读数据 */
	F645_97CMD_READ_FOLLOW_DATA  			= 0x02, /* 00010：读后续数据 */
	F645_97CMD_REREAD_DATA  				= 0x03, /* 00011：重读数据 */
	F645_97CMD_WRITE_DATA    				= 0x04, /* 00100：写数据 */
	F645_97CMD_CHECK_TIME  					= 0x08, /* 01000：广播校时 */
	F645_97CMD_WRITE_DEV_ADDRESS			= 0x0A, /* 01010：写设备地址 */
	F645_97CMD_CHANGE_BAUD   				= 0x0C, /* 01100：更改通信速率 */
	F645_97CMD_CHANGE_PASSWORD   			= 0x0F, /* 01111：修改密码 */
	F645_97CMD_MAXNEED_CLEAR   				= 0x10, /* 10000：最大需量清零 */
	F645_97CMD_NORMAL_RESPONSE     			= 0x80, /* 从站正常应答无后续数据 */
 	F645_97CMD_ABNORMAL_RESPONSE   			= 0xC0, /* 从站异常应答 */
 	F645_97CMD_NORMAL_RESPONSE_FOLLOW_DATA 	= 0xA0, /* 从站正常应答有后续数据 */
};


/*** 帧格式各参数 ***/
struct frame645_97param
{
	rt_uint8_t addr[6];            	/* 地址 */
	enum frame645_97ctrl_e ctrl;	/* 控制码 */
	rt_uint32_t data_flag;			/* 数据标识 */
	rt_uint8_t *data;				/* 存储发送/接收数据的起始地址 */
	rt_uint8_t data_len;			/* 发送/接收的数据长度, 不是缓冲区的长度 */
};

extern enum frame_error_e transmit_ammeter645_97_data(struct frame645_97param *send_frame_data,struct frame645_97param *recv_frame_data, enum ammeter_uart_e port);

#endif
