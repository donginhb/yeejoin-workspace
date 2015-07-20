/**********************************************************************************
* Filename   	: ammeter_645_97.c
* Description 	: define the functions that read the 645-1997 ammeters data
* Begin time  	: 2013-10-10
* Finish time 	: 
* Engineer		: zhanghonglei
*************************************************************************************/
#include <rtthread.h>
#include <board.h>
#include <ammeter_645_97.h>
#include <ammeter.h>
#include <debug_sw.h>

#define ERROR_PRINTF(x) printf_syn x

#define PRINT_SEND_FRAME_BUF_DATA	1

#define REDUND_LENGTH_645_97 16	   //include 4 0xFE

static rt_err_t create_frame_645_97(rt_uint8_t *send_frame_buf, struct frame645_97param *frame_param);
static enum frame_error_e analysis_frame_645_97(rt_uint8_t *buf,rt_uint32_t recv_frame_length, struct frame645_97param *frame_param);
static enum frame_error_e send_recv_645_97frame(struct frame645_97param *send_frame_data, struct frame645_97param *recv_frame_data, enum ammeter_uart_e port);

/**********************************************************************************
* Function	 : 	enum frame_error_e transmit_ammeter645_97_data(struct frame645_97param *send_frame_data,
* 				struct frame645_97param *recv_frame_data, enum ammeter_uart_e port)
* Description: 	传输645-1997规约的电表的信息
* Arguments	 : 	(1)send_frame_data:输入电表信息的结构体；（2）recv_frame_data：输出电表信息的结构体；（3）port：发送数据的端口；
* Return	 : 	FRAME_E_OK
*************************************************************************************/
enum frame_error_e transmit_ammeter645_97_data(struct frame645_97param *send_frame_data, struct frame645_97param *recv_frame_data, enum ammeter_uart_e port)
{
	int i = 0;
	enum frame_error_e result = FRAME_E_ERROR;
	
	for (i = 0; i < TRY_GET_EM_DATA_CNT_MAX; i++)	{
		result = send_recv_645_97frame(send_frame_data, recv_frame_data, port);
		if (result == FRAME_E_OK || result == FRAME_E_FLAG_ERROR) {
			return result;
		} 

		rt_thread_delay(get_ticks_of_ms(1000));
	}
		
	return FRAME_E_ERROR;
}
/**********************************************************************************
* Function	 : 	static enum frame_error_e send_recv_645_97frame(struct frame645_97param *send_frame_data,
* 				struct frame645_97param *recv_frame_data, enum ammeter_uart_e port)
* Description: 	发送和接收645-1997规约电表的信息
* Arguments	 : 	(1)send_frame_data:输入电表信息的结构体；（2）recv_frame_data：输出电表信息的结构体；（3）port：发送数据的端口；
* Return	 : 	FRAME_E_OK
*************************************************************************************/
static enum frame_error_e send_recv_645_97frame(struct frame645_97param *send_frame_data, struct frame645_97param *recv_frame_data, enum ammeter_uart_e port)
{
	rt_uint32_t recv_frame_len = 0;
	rt_uint8_t recv_frame_buf[256] = {'\0'};
	rt_uint8_t send_frame_buf[64] = {'\0'};
	rt_uint8_t broadcast_addr[] = {0x99, 0x99, 0x99, 0x99, 0x99, 0x99};
	enum frame_error_e result = FRAME_E_OK;
	rt_err_t ret = RT_EOK;
    struct uart_485_param uart_485_data;

    rt_memset(&uart_485_data, 0, sizeof(struct uart_485_param));
	
	if(RT_EOK != create_frame_645_97(send_frame_buf, send_frame_data)) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	if(RT_EOK == ammeter_mutex_take(port, 500)) {
		uart_485_data.baudrate = 1200;
		uart_485_data.databits = 8;
		uart_485_data.paritybit = 1;
		uart_485_data.stopbits = 1;

		if (RT_EOK != set_485sw_and_em_usart(uart_485_data, port, AP_PROTOCOL_645_1997)) {
			ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
			goto END_MUTEX_TAKE;
		}

		em_protoco_send_data_by_485(get_485_port_from_ammeter_uart(port), send_frame_buf, REDUND_LENGTH_645_97 + send_frame_buf[13]);
	#if PRINT_SEND_FRAME_BUF_DATA
		if (is_sub_m_on(dsw_sub_module[DMN_METER_PROTO], DSMN_MP_645_1997)) {
			print_buf_in_hex("send_645_97frame_buf", (char *)send_frame_buf,
					REDUND_LENGTH_645_97 + send_frame_buf[13]);
		}
	#endif
		if (send_frame_data->ctrl == F645_97CMD_CHECK_TIME) {
			if(RT_EOK != ammeter_mutex_release(port)) {
				ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
				return FRAME_E_ERROR;
			}
			return FRAME_E_OK;
		}

		ret = recv_data_from_485(port, recv_frame_buf, &recv_frame_len,
				EM_PROTO_INTER_FRAME_TIMEOUT_MS, EM_PROTO_INTER_BYTE_TIMEOUT_MS);

		if(RT_EOK != ammeter_mutex_release(port)) {
			ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
			return FRAME_E_ERROR;
		}
	} else {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	if (ret == RT_EOK) {
		result = analysis_frame_645_97(recv_frame_buf, recv_frame_len, recv_frame_data);
		if(result == FRAME_E_OK) {
			if((rt_memcmp(send_frame_data->addr, recv_frame_data->addr, 6) != 0)
					&& (rt_memcmp(send_frame_data->addr, broadcast_addr, 6) != 0)) {
				ERROR_PRINTF(("Receive 645-97 frame addr not send cmd result\n"));
				return FRAME_E_ADDR_MISMATCH;
			} else {
				if(((recv_frame_data->ctrl) & 0xf0) == 0xC0) {
					ERROR_PRINTF(("Not recognize this 645-97 flag!\n\n"));
					return FRAME_E_FLAG_ERROR;
				} else {
					if (recv_frame_data->ctrl == F645_97CMD_READ_DATA || recv_frame_data->ctrl == F645_97CMD_READ_FOLLOW_DATA
						|| recv_frame_data->ctrl == F645_97CMD_REREAD_DATA) {
						if(send_frame_data->data_flag != recv_frame_data->data_flag) {
							return FRAME_E_FLAG_MISMATCH;
						}
					} else {
						return FRAME_E_OK;
					}
				}
			}
		} else {
			return result;
		}
	} else {
		return FRAME_E_REVE_TIMEOUT;
	}

	return FRAME_E_ERROR;

END_MUTEX_TAKE:

	if(RT_EOK != ammeter_mutex_release(port)) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	return FRAME_E_ERROR;
}


/**********************************************************************************
* Function	 : 	static rt_err_t create_frame_645_97(rt_uint8_t *send_frame_buf, struct frame645_97param *frame_param)
* Description: 	创建645-1997帧格式
* Arguments	 : 	(1)send_frame_buf:输出的帧数据；（2）frame_param：组成帧格式的各种参数
* Return	 : 	RT_EOK
*************************************************************************************/
static rt_err_t create_frame_645_97(rt_uint8_t *send_frame_buf, struct frame645_97param *frame_param)
{
	rt_uint8_t i = 0, check = 0, temp = 0;
	rt_uint8_t ammeter_flag[2] = {'\0'};
	rt_uint8_t ammeter_addr[6] = {'\0'};

	if (RT_NULL==send_frame_buf || RT_NULL==frame_param) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_EIO;
	}

	if (rt_memcmp(frame_param->addr, ammeter_addr, 6) == 0) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	ammeter_flag[0] = frame_param->data_flag & 0xff;
	ammeter_flag[1] = (frame_param->data_flag>>8) & 0xff;

	/* 对数组进行赋值 */
	*send_frame_buf       = 0xFE;
	*(send_frame_buf + 1) = 0xFE;
	*(send_frame_buf + 2) = 0xFE;
	*(send_frame_buf + 3) = 0xFE;
	*(send_frame_buf + 4) = 0x68;

	for(i=0; i<6; i++)
		*(send_frame_buf+5+i)=*(frame_param->addr+5-i);/* 电能表地址域 */

	*(send_frame_buf+11)= 0x68;
	*(send_frame_buf+12)= frame_param->ctrl;	/* 电能表控制码 */

	switch (frame_param->ctrl) {
	case F645_97CMD_READ_DATA:			    /* 读数据 */
		temp = 0x02;
		*(send_frame_buf + 13) = temp;
		for(i = 0; i < temp; i++)
			*(send_frame_buf + 14 + i) = *(ammeter_flag + i) + 0x33;
		break;

	case F645_97CMD_READ_FOLLOW_DATA:			/*读后续数据*/
		temp = 0x02;
		*(send_frame_buf + 13) = temp;
		for(i = 0; i < *(send_frame_buf + 13); i++)
			*(send_frame_buf + 14 + i) = *(ammeter_flag + i) + 0x33;
		break;

	case F645_97CMD_REREAD_DATA:				/*重读数据*/
		*(send_frame_buf + 13) = 0x00;
		break;

	case F645_97CMD_WRITE_DATA:				/*写数据*/
		temp = 0x02;
		*(send_frame_buf + 13) = temp + frame_param->data_len;
		for(i = 0; i < temp; i++)
			*(send_frame_buf + 14 + i) = *(ammeter_flag + i) + 0x33;
		for(i = 0; i < frame_param->data_len; i++)
			*(send_frame_buf + 16 + i) = *(frame_param->data + i) + 0x33;
		break;

	case F645_97CMD_CHECK_TIME:				/*广播校时*/
		temp = 0x06;
		*(send_frame_buf+13) = temp;
		for(i = 0; i < temp; i++)
			*(send_frame_buf + 14 + i) = *(frame_param->data + i) + 0x33;
		break;

	case F645_97CMD_WRITE_DEV_ADDRESS:		/*修改设备地址*/
		temp = 0x06;
		*(send_frame_buf+13) = temp;
		for(i = 0; i < temp; i++)
			*(send_frame_buf + 14 + i) = *(frame_param->data + i) + 0x33;
		break;

	case F645_97CMD_CHANGE_BAUD:		  		/*更改通信速率*/
		*(send_frame_buf+13) = 0x01;
		*(send_frame_buf + 14) = ammeter_flag[0] + 0x33;
		break;

	case F645_97CMD_CHANGE_PASSWORD:		  	/*修改密码*/
		temp = 0x08;
		*(send_frame_buf+13) = temp;
		for(i = 0; i < temp; i++)
			*(send_frame_buf + 14 + i) = *(frame_param->data + i) + 0x33;
		break;

	case F645_97CMD_MAXNEED_CLEAR:		  	/*最大需量清零*/
		*(send_frame_buf + 13) = 0x00;
		break;

	default:
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ENOMEM;
	}

	for(i = 4;i < *(send_frame_buf+13)+14;i++)
		check += (*(send_frame_buf+i));		/*校验码*/

	*(send_frame_buf + *(send_frame_buf+13)+14)=(rt_uint8_t)(check&0xff);
	*(send_frame_buf + *(send_frame_buf+13)+14+1)=0x16; 		/*结束符*/

	return RT_EOK;
}
/**********************************************************************************
* Function	 : 	static enum frame_error_e analysis_frame_645_97(rt_uint8_t *recv_frame_buf,rt_uint8_t recv_frame_length, struct frame645_97param *frame_param)
* Description: 	解析615-1997帧数据
* Arguments	 : 	(1)recv_frame_buf:输入帧数据；（2）recv_frame_length：输入数据帧的长度；（3）frame_param：传输数据帧解析后的各项参数
* Return	 : 	RT_EOK
*************************************************************************************/
static enum frame_error_e analysis_frame_645_97(rt_uint8_t *recv_frame_buf, rt_uint32_t recv_frame_length, struct frame645_97param *frame_param)
{
	rt_uint32_t i = 0;
	rt_uint8_t check = 0;
	 rt_uint8_t *start = RT_NULL;
	 rt_uint8_t len_temp = 0;
	 rt_uint32_t length = 0;
	 rt_uint8_t buf[256] = {'\0'};

	 if (recv_frame_buf == RT_NULL || frame_param == RT_NULL || frame_param->data == RT_NULL) {
	 	ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
	 	return FRAME_E_ERROR;
	 }

	 length = recv_frame_length;
	 if (length > 256)
	 		length = 256;

	rt_memcpy(buf, recv_frame_buf, length);

	for (i = 0; i < length; i++) {
		if (buf[i] == 0x68) { /*search the 68H*/
			break;
		}
	}

	if (i == length) {
		print_buf_in_hex("ERR:analysis_frame_645_97 recv an error frame", (char *)buf, length);
		return FRAME_E_FRAG;	/*此帧是个片段*/
	}

	 start = buf + i;
	 if((*(start + 7)) != 0x68) {
		 ERROR_PRINTF(("ERR: func:%s, line(%d), recv_data:", __FUNCTION__, __LINE__));
		 return FRAME_E_FRAG;		/*帧格式错误*/
	 }

	 len_temp = (*(start + 9));

	 if (i + 12 + len_temp > 256) {
		ERROR_PRINTF(("ERR: func:%s, line(%d), recv_data:", __FUNCTION__, __LINE__));
		return FRAME_E_FRAG;	/*此帧是个片段*/
	 }

	 for(i = 0; i < 10 + len_temp; i++)
		 check += start[i];

	 check = (rt_uint8_t)(check & 0xff);

	 if(check != start[10 + len_temp]) {
		 ERROR_PRINTF(("ERR: func:%s, line(%d), recv_data:", __FUNCTION__, __LINE__));
		 return FRAME_E_CS_ERR;	   /*校验错误*/
	 }

#if 0
	for(i=12; i<10+len_temp; i++) {
		if((*(start+i)) < 0x33)
			return FRAME_E_DATA_ERR;  /*数据错误*/
	}
#endif

	if(frame_param->addr != RT_NULL)
		for(i = 0;i < 6;i++)
			frame_param->addr[i] = *(start+6-i);

	frame_param->ctrl = *(start+8);

	if(((frame_param->ctrl) & 0xf0) == 0xC0) {
		frame_param->data_len = *(start+9);
		frame_param->data_flag = 0;
		start = start + 10;
	} else {
		if (((frame_param->ctrl & 0x0f) == 0x01)|| ((frame_param->ctrl & 0x0f) == 0x02)
			|| ((frame_param->ctrl & 0x0f) == 0x03)) {
			frame_param->data_len = *(start+9) - 2;
			frame_param->data_flag = ((*(start + 10) - 0x33)&0xff) | (((*(start + 11) - 0x33)&0xff) << 8);
			start = start + 12;
			len_temp = len_temp - 2;
		} else {
			frame_param->data_len = *(start+9);
			frame_param->data_flag = 0;
			start = start + 10;
		}
	}

	for(i=0;i<len_temp;i++)
		frame_param->data[i] = ((*(start + i)) - 0x33);

	return FRAME_E_OK; 			/*执行正确，返回0*/
}


