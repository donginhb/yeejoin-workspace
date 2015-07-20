/**********************************************************************************
* Filename   	: ammeter_645_07.c
* Description 	: define the functions that read the 645-2007 ammeters data
* Begin time  	: 2013-10-10
* Finish time 	: 
* Engineer		: zhanghonglei
*************************************************************************************/
#include <rtthread.h>
#include <board.h>
#include <ammeter_645_07.h>
#include <ammeter.h>
#include <debug_sw.h>

#define ERROR_PRINTF(x) printf_syn x

#define PRINT_SEND_FRAME_BUF_DATA	1

#define REDUND_LENGTH_645_07 16	   //include 4 0xFE

static rt_err_t create_frame_645_07(rt_uint8_t *send_frame_buf, struct send_frame_param *frame_param);
static enum frame_error_e analysis_frame_645_07(rt_uint8_t *recv_frame_buf,rt_uint32_t recv_frame_length, struct recv_frame_param *frame_param);
static enum frame_error_e send_recv_645_07_frame(struct send_frame_param *send_frame_data, struct recv_frame_param *recv_frame_data, enum ammeter_uart_e port);

/**********************************************************************************
* Function	 : 	enum frame_error_e transmit_ammeter645_07_data(struct send_frame_param *send_frame_data,
* 				struct recv_frame_param *recv_frame_data, enum ammeter_uart_e port)
* Description: 	传输645-2007规约的电表的信息
* Arguments	 : 	(1)send_frame_data:输入电表信息的结构体；（2）recv_frame_data：输出电表信息的结构体；（3）port：发送数据的端口；
* Return	 : 	FRAME_E_OK
*************************************************************************************/
enum frame_error_e transmit_ammeter645_07_data(struct send_frame_param *send_frame_data, struct recv_frame_param *recv_frame_data, enum ammeter_uart_e port)
{
	rt_uint32_t i = 0;
	enum frame_error_e result = FRAME_E_ERROR;
	
	for (i = 0; i < TRY_GET_EM_DATA_CNT_MAX; i++)	{
		result = send_recv_645_07_frame(send_frame_data, recv_frame_data, port);
		if (result == FRAME_E_OK || result == FRAME_E_FLAG_ERROR) {
			return result;
		} 

		rt_thread_delay(get_ticks_of_ms(1000));
	}
		
	return FRAME_E_ERROR;
}

/**********************************************************************************
* Function	 : 	static enum frame_error_e send_recv_645_07_frame(struct send_frame_param *send_frame_data,
* 				struct recv_frame_param *recv_frame_data, enum ammeter_uart_e port)
* Description: 	发送和接收645-2007规约电表的信息
* Arguments	 : 	(1)send_frame_data:输入电表信息的结构体；（2）recv_frame_data：输出电表信息的结构体；（3）port：发送数据的端口；
* Return	 : 	FRAME_E_OK
*************************************************************************************/
static enum frame_error_e send_recv_645_07_frame(struct send_frame_param *send_frame_data, struct recv_frame_param *recv_frame_data, enum ammeter_uart_e port)
{
	rt_err_t ret = RT_ERROR;
	enum frame_error_e result = FRAME_E_ERROR;
	rt_uint32_t recv_frame_len = 0;
	rt_uint8_t recv_frame_buf[256];
	rt_uint8_t send_frame_buf[64];
	rt_uint8_t broadcast_addr[] = {0x99, 0x99, 0x99, 0x99, 0x99, 0x99};
    struct uart_485_param uart_485_data;

    rt_memset(&uart_485_data, 0, sizeof(struct uart_485_param));


	if(RT_EOK != create_frame_645_07(send_frame_buf, send_frame_data)) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	if(RT_EOK == ammeter_mutex_take(port, 500)) {
		if (rt_memcmp(send_frame_data->addr, broadcast_addr, 6) == 0) {
			uart_485_data.baudrate = 2400;
			uart_485_data.databits = 8;
			uart_485_data.paritybit = 1;
			uart_485_data.stopbits = 1;

			if (RT_EOK != set_485sw_and_em_usart(uart_485_data, port, AP_PROTOCOL_645_2007)) {
				ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
				goto END_MUTEX_TAKE;
			}

			uart_485_data.baudrate = 1200;
			uart_485_data.databits = 8;
			uart_485_data.paritybit = 1;
			uart_485_data.stopbits = 1;

			em_protoco_send_data_by_485(get_485_port_from_ammeter_uart(port), send_frame_buf, REDUND_LENGTH_645_07 + send_frame_buf[13]);

			if (RT_EOK != set_485sw_and_em_usart(uart_485_data, port, AP_PROTOCOL_645_2007)) {
				ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
				goto END_MUTEX_TAKE;
			}


			em_protoco_send_data_by_485(get_485_port_from_ammeter_uart(port), send_frame_buf, REDUND_LENGTH_645_07 + send_frame_buf[13]);

			if(RT_EOK != ammeter_mutex_release(port)) {
				ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
				return FRAME_E_ERROR;
			}

			return FRAME_E_OK;
		}

		if (RT_EOK == get_save_ammeter_baud(send_frame_data->addr, &uart_485_data)) {
			if (RT_EOK != set_485sw_and_em_usart(uart_485_data, port, AP_PROTOCOL_645_2007)) {
				ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
				goto END_MUTEX_TAKE;
			}
		} else {
			uart_485_data.baudrate = 2400;
			uart_485_data.databits = 8;
			uart_485_data.paritybit = 1;
			uart_485_data.stopbits = 1;
			if (RT_EOK != set_485sw_and_em_usart(uart_485_data, port, AP_PROTOCOL_645_2007)) {
				ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
				goto END_MUTEX_TAKE;
			}
		}

	#if PRINT_SEND_FRAME_BUF_DATA
		if (is_sub_m_on(dsw_sub_module[DMN_METER_PROTO], DSMN_MP_645_2007)) {
			print_buf_in_hex("send_645_07_frame_buf", (char *)send_frame_buf,
					REDUND_LENGTH_645_07 + send_frame_buf[13]);
		}
	#endif

		em_protoco_send_data_by_485(get_485_port_from_ammeter_uart(port), send_frame_buf, REDUND_LENGTH_645_07 + send_frame_buf[13]);

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
		result = analysis_frame_645_07(recv_frame_buf, recv_frame_len, recv_frame_data);
		if(result == FRAME_E_OK) {
			if (rt_memcmp(send_frame_data->addr, recv_frame_data->addr, 6) != 0) {
				ERROR_PRINTF(("Receive 645-07 frame addr not send cmd result\n"));
				return FRAME_E_ADDR_MISMATCH;
			} else {
				if(((recv_frame_data->ctrl) & 0xf0) == 0xD0) {
					ERROR_PRINTF(("Not recognize this 645-07 flag!\n\n"));
					return FRAME_E_FLAG_ERROR;
				} else {
					if((send_frame_data->data_flag != recv_frame_data->data_flag) && (0 != recv_frame_data->data_flag)) {
						ERROR_PRINTF(("Receive 645-07 frame flag not send cmd result!\n"));
						return FRAME_E_FLAG_MISMATCH;
					} else {
						return FRAME_E_OK;
					}
				}
			}
		} else {
			ERROR_PRINTF(("Analyse 645-07 Frame Error(%d)!\n", result));
			return result;
		}
	} else {
		ERROR_PRINTF(("ERR: func:%s, line(%d), err(%d)\n", __FUNCTION__, __LINE__,
				ret));

		return FRAME_E_REVE_TIMEOUT;
	}

END_MUTEX_TAKE:

	if(RT_EOK != ammeter_mutex_release(port)) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	return FRAME_E_ERROR;
}

/**********************************************************************************
* Function	 : 	static rt_err_t create_frame_645_07(rt_uint8_t *send_frame_buf, struct send_frame_param *frame_param)
* Description: 	创建645-2007帧格式
* Arguments	 : 	(1)send_frame_buf:输出数据帧；（2）frame_param：组成帧格式的各种参数
* Return	 : 	RT_EOK
*************************************************************************************/
static rt_err_t create_frame_645_07(rt_uint8_t *send_frame_buf, struct send_frame_param *frame_param)
{
	rt_uint8_t i = 0, check = 0, temp;
	rt_uint8_t ammeter_flag[4] = {'\0'};
	rt_uint8_t ammeter_password[4] = {'\0'};
	rt_uint8_t ammeter_operator_code[4] = {'\0'};
	rt_uint8_t ammeter_addr[6] = {'\0'};

	if (RT_NULL==send_frame_buf || RT_NULL==frame_param) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_EIO;
	}

	if (rt_memcmp(frame_param->addr, ammeter_addr, 6) == 0) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	for (i = 0; i < 4; i++) {
		ammeter_flag[i] = (frame_param->data_flag>>(8 * i)) & 0xff;
		ammeter_password[i] = (frame_param->password>>(8 * i)) & 0xff;
		ammeter_operator_code[i] = (frame_param->operator_code>>(8 * i)) & 0xff;
	}

	/* 对数组进行赋值 */
	*send_frame_buf       = 0xFE;
	*(send_frame_buf + 1) = 0xFE;
	*(send_frame_buf + 2) = 0xFE;
	*(send_frame_buf + 3) = 0xFE;
	*(send_frame_buf + 4) = 0x68;

	for(i=0; i<6; i++)
		*(send_frame_buf+5+i)=*(frame_param->addr+5-i);		   /* 电能表地址域 */

	*(send_frame_buf+11)= 0x68;
	*(send_frame_buf+12)= frame_param->ctrl;					   /* 电能表控制码 */

	switch (frame_param->ctrl) {
	case F645_07CMD_READ_DATA:			    /* 读数据 */
		temp = 0x04;
		for(i = 0; i < temp; i++)
			*(send_frame_buf + 14 + i) = *(ammeter_flag + i) + 0x33;
		
		switch (frame_param->data_len) {
		case 0:
			break;

		case 1:	
			*(send_frame_buf + 14 + temp) = frame_param->data[0]+ 0x33;
			break;

		case 6:
			*(send_frame_buf + 14 + temp) = frame_param->data[0] + 0x33;
			for(i = 1; i < 6; i++)
				*(send_frame_buf + 14 + temp + i) = *(frame_param->data + i) + 0x33;
			break;

			default:
				ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
				return RT_ERROR;
		}
		*(send_frame_buf + 13) = temp + frame_param->data_len;			
		break;

	case F645_07CMD_READ_FOLLOW_DATA:			/*读后续数据*/
		temp = 0x04;
		*(send_frame_buf + 13) = temp + 1;
		for(i = 0; i < temp; i++)
			*(send_frame_buf + 14 + i) = *(ammeter_flag + i) + 0x33;
		*(send_frame_buf + 14 + temp) = frame_param->frame_order_num + 0x33;
		break;

	case F645_07CMD_WRITE_DATA:				/* 写数据 */
		temp = 0x04;
		*(send_frame_buf + 13) = temp + 8 + frame_param->data_len;
		for(i = 0; i < temp; i++)
			*(send_frame_buf + 14 + i) = *(ammeter_flag + i) + 0x33;
		for(i = 0; i < 4; i++)
			*(send_frame_buf + 14 + temp + i) = *(ammeter_password + i) + 0x33;
		for(i = 0; i < 4; i++)
			*(send_frame_buf + 18 + temp + i) = *(ammeter_operator_code + i) + 0x33;
		for(i = 0; i < frame_param->data_len; i++)
			*(send_frame_buf + 22 + temp + i) = *(frame_param->data + i) + 0x33;
		break;

	case F645_07CMD_READ_DEV_ADDR:				/* 读通信地址*/
		*(send_frame_buf + 13) = 0x00;
		break;

	case F645_07CMD_WRITE_DEV_ADDR:				/* 写通信地址*/
		temp = 0x06;
		*(send_frame_buf+13) = temp;
		for(i = 0; i < temp; i++)
			*(send_frame_buf + 14 + i) = *(frame_param->data + i) + 0x33;
		break;

	case F645_07CMD_BROADCASTING_TIME:				/* 广播校时 */
		temp = 0x06;
		*(send_frame_buf+13) = temp;
		for(i = 0; i < temp; i++)
			*(send_frame_buf + 14 + i) = *(frame_param->data + i) + 0x33;
		break;

	case F645_07CMD_FREEZING_CMD:					/* 冻结命令 */
		temp = 0x04;
		*(send_frame_buf+13) = temp;
		for(i = 0; i < temp; i++)
			*(send_frame_buf + 14 + i) = *(frame_param->data + i) + 0x33;
		break;

	case F645_07CMD_CHANGE_BAUD:		  	/*更改通信速率*/
		*(send_frame_buf+13) = 0x01;
		*(send_frame_buf + 14) = ammeter_flag[0] + 0x33;	 /*电能表地址域*/
		break;

	case F645_07CMD_CHANGE_PASSWORD:		  	/*修改密码*/
		temp = 0x0C;
		*(send_frame_buf + 13) = temp;
		for(i = 0; i < 4; i++)
			*(send_frame_buf + 14 + i) = *(ammeter_flag + i) + 0x33;
		for(i = 0; i < 4; i++)
			*(send_frame_buf + 18 + i) = *(ammeter_password + i) + 0x33;
		for(i = 0; i < 4; i++)
			*(send_frame_buf + 22 + i) = *(frame_param->data + i) + 0x33;
		break;

	case F645_07CMD_MAXNEED_CLEAR:		  	/*最大需量清零*/
	case F645_07CMD_AMMETER_CLEAR:		  	/* 电表清零 */
		temp = 0x08;
		*(send_frame_buf + 13) = temp;
		for(i = 0; i < 4; i++)
			*(send_frame_buf + 14 + i) = *(ammeter_password + i) + 0x33;
		for(i = 0; i < 4; i++)
			*(send_frame_buf + 18 + i) = *(ammeter_operator_code + i) + 0x33;
		break;

	case F645_07CMD_EVENT_CLEAR:		  	/* 事件清零 */
		temp = 0x0C;
		*(send_frame_buf + 13) = temp;
		for(i = 0; i < 4; i++)
			*(send_frame_buf + 14 + i) = *(ammeter_password + i) + 0x33;
		for(i = 0; i < 4; i++)
			*(send_frame_buf + 18 + i) = *(ammeter_operator_code + i) + 0x33;
		for(i = 0; i < 4; i++)
			*(send_frame_buf + 22 + i) = *(ammeter_flag + i) + 0x33;
		break;

	case F645_07CMD_TRIPPING_ALARM_SECURITY:		  	/* 跳合闸、报警、保电 */
		temp = 0x08 + frame_param->data_len;
		*(send_frame_buf + 13) = temp;
		for(i = 0; i < 4; i++)
			*(send_frame_buf + 14 + i) = *(ammeter_password + i) + 0x33;
		for(i = 0; i < 4; i++)
			*(send_frame_buf + 18+ i) = *(ammeter_operator_code + i) + 0x33;
		for(i = 0; i < frame_param->data_len; i++)
			*(send_frame_buf + 22 + i) = *(frame_param->data + i) + 0x33;
		break;

	case F645_07CMD_MUX_FUN_OUTPUT_CMD:		  	/* 多功能端子输出控制命令 */
		*(send_frame_buf + 13) = 0x01;
		*(send_frame_buf + 14) = ammeter_flag[0] + 0x33;
		break;

	case F645_07CMD_SECURITY_AUTHENTICATION_CMD:		  /* 安全认证命令 */
		temp = 0x08 + frame_param->data_len;
		*(send_frame_buf + 13) = temp;
		for(i = 0; i < 4; i++)
			*(send_frame_buf + 14 + i) = *(ammeter_password + i) + 0x33;
		for(i = 0; i < 4; i++)
			*(send_frame_buf + 18+ i) = *(ammeter_operator_code + i) + 0x33;
		for(i = 0; i < frame_param->data_len; i++)
			*(send_frame_buf + 22 + i) = *(frame_param->data + i) + 0x33;
		break;

	default:
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ENOMEM;
	}

	for(i = 4;i < *(send_frame_buf+13)+14;i++)
		check += (*(send_frame_buf+i));						/*校验码*/

	*(send_frame_buf + *(send_frame_buf+13)+14)=(rt_uint8_t)(check&0xff);
	*(send_frame_buf + *(send_frame_buf+13)+14+1)=0x16; 		/*结束符*/

	return RT_EOK;
}
/**********************************************************************************
* Function	 : 	static enum frame_error_e analysis_frame_645_07(rt_uint8_t *recv_frame_buf,
* 				rt_uint8_t recv_frame_length, struct recv_frame_param *frame_param)
* Description: 	解析615-2007帧数据
* Arguments	 : 	(1)recv_frame_buf:输入数据帧；（2）recv_frame_length：输入数据帧的长度；（3）frame_param：传输数据帧解析后的各项参数
* Return	 : 	RT_EOK
*************************************************************************************/
static enum frame_error_e analysis_frame_645_07(rt_uint8_t *recv_frame_buf, rt_uint32_t recv_frame_length, struct recv_frame_param *frame_param)
{
	rt_uint32_t i = 0;
	 rt_uint8_t check = 0, tmp = 0;
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
		print_buf_in_hex("ERR:analysis_frame_645_07 recv an error frame", (char *)buf, length);
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

	tmp = *(start+8);
	frame_param->ctrl = tmp;

	if((frame_param->ctrl & 0xf0) == 0xD0) {
		frame_param->data_len = *(start+9);
		frame_param->data_flag = 0;
		start = start + 10;
	} else {
		if (((frame_param->ctrl) == 0x91) || ((frame_param->ctrl) == 0x83)) {
			frame_param->data_flag =((*(start+10)-0x33)&0xff)|(((*(start+11)-0x33)&0xff)<<8)|(((*(start+12)-0x33)&0xff)<<16)|(((*(start+13)-0x33)&0xff)<<24) ;
			frame_param->data_len = *(start+9) - 4;
			start = start + 14;
			len_temp = len_temp - 4;
		} else if ((frame_param->ctrl) == 0x92) {
			frame_param->data_flag =(*(start+10)-0x33)|((*(start+11)-0x33)<<8)|((*(start+12)-0x33)<<16)|((*(start+13)-0x33)<<24) ;
			frame_param->data_len = *(start+9) - 5;
			start = start + 14;
			len_temp = len_temp - 5;
			frame_param->frame_order_num = *(start  + len_temp) -0x33;
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


