/**********************************************************************************
* Filename   	: ammeter_edmi.c
* Description 	: define the functions that read the 645-1997 ammeters data
* Begin time  	: 2013-10-10
* Finish time 	:
* Engineer		: zhanghonglei
*************************************************************************************/
#include <rtthread.h>
#include <board.h>
#include <math.h>
#include <ammeter_edmi.h>
#include <ammeter.h>
#include <debug_sw.h>

#define ERROR_PRINTF(x) printf_syn x

#define PRINT_SEND_FRAME_BUF_DATA	1

static rt_err_t create_frame_edmi(rt_uint8_t *send_frame_buf, rt_uint32_t *send_frame_len, frame_edmi_param_t *frame_param);
static enum frame_error_e analysis_frame_edmi(rt_uint8_t *recv_frame_buf,rt_uint8_t recv_frame_length, frame_edmi_param_t *frame_param);
static enum frame_error_e send_recv_edmi_frame(frame_edmi_param_t *send_frame_data, frame_edmi_param_t *recv_frame_data, enum ammeter_uart_e port);
static void create_check_code(rt_uint8_t *cp, rt_uint32_t len, rt_uint8_t check[2]);

/* 由表发生器计算生成的check_code查询表 */
static rt_uint16_t check_code_tab[256] = {
	0x0000,0x1021,0x2042,0x3063,0x4084,0x50A5,0x60C6,0x70E7,
	0x8108,0x9129,0xA14A,0xB16B,0xC18C,0xD1AD,0xE1CE,0xF1EF,
	0x1231,0x0210,0x3273,0x2252,0x52B5,0x4294,0x72F7,0x62D6,
	0x9339,0x8318,0xB37B,0xA35A,0xD3BD,0xC39C,0xF3FF,0xE3DE,
	0x2462,0x3443,0x0420,0x1401,0x64E6,0x74C7,0x44A4,0x5485,
	0xA56A,0xB54B,0x8528,0x9509,0xE5EE,0xF5CF,0xC5AC,0xD58D,
	0x3653,0x2672,0x1611,0x0630,0x76D7,0x66F6,0x5695,0x46B4,
	0xB75B,0xA77A,0x9719,0x8738,0xF7DF,0xE7FE,0xD79D,0xC7BC,
	0x48C4,0x58E5,0x6886,0x78A7,0x0840,0x1861,0x2802,0x3823,
	0xC9CC,0xD9ED,0xE98E,0xF9AF,0x8948,0x9969,0xA90A,0xB92B,
	0x5AF5,0x4AD4,0x7AB7,0x6A96,0x1A71,0x0A50,0x3A33,0x2A12,
	0xDBFD,0xCBDC,0xFBBF,0xEB9E,0x9B79,0x8B58,0xBB3B,0xAB1A,
	0x6CA6,0x7C87,0x4CE4,0x5CC5,0x2C22,0x3C03,0x0C60,0x1C41,
	0xEDAE,0xFD8F,0xCDEC,0xDDCD,0xAD2A,0xBD0B,0x8D68,0x9D49,
	0x7E97,0x6EB6,0x5ED5,0x4EF4,0x3E13,0x2E32,0x1E51,0x0E70,
	0xFF9F,0xEFBE,0xDFDD,0xCFFC,0xBF1B,0xAF3A,0x9F59,0x8F78,
	0x9188,0x81A9,0xB1CA,0xA1EB,0xD10C,0xC12D,0xF14E,0xE16F,
	0x1080,0x00A1,0x30C2,0x20E3,0x5004,0x4025,0x7046,0x6067,
	0x83B9,0x9398,0xA3FB,0xB3DA,0xC33D,0xD31C,0xE37F,0xF35E,
	0x02B1,0x1290,0x22F3,0x32D2,0x4235,0x5214,0x6277,0x7256,
	0xB5EA,0xA5CB,0x95A8,0x8589,0xF56E,0xE54F,0xD52C,0xC50D,
	0x34E2,0x24C3,0x14A0,0x0481,0x7466,0x6447,0x5424,0x4405,
	0xA7DB,0xB7FA,0x8799,0x97B8,0xE75F,0xF77E,0xC71D,0xD73C,
	0x26D3,0x36F2,0x0691,0x16B0,0x6657,0x7676,0x4615,0x5634,
	0xD94C,0xC96D,0xF90E,0xE92F,0x99C8,0x89E9,0xB98A,0xA9AB,
	0x5844,0x4865,0x7806,0x6827,0x18C0,0x08E1,0x3882,0x28A3,
	0xCB7D,0xDB5C,0xEB3F,0xFB1E,0x8BF9,0x9BD8,0xABBB,0xBB9A,
	0x4A75,0x5A54,0x6A37,0x7A16,0x0AF1,0x1AD0,0x2AB3,0x3A92,
	0xFD2E,0xED0F,0xDD6C,0xCD4D,0xBDAA,0xAD8B,0x9DE8,0x8DC9,
	0x7C26,0x6C07,0x5C64,0x4C45,0x3CA2,0x2C83,0x1CE0,0x0CC1,
	0xEF1F,0xFF3E,0xCF5D,0xDF7C,0xAF9B,0xBFBA,0x8FD9,0x9FF8,
	0x6E17,0x7E36,0x4E55,0x5E74,0x2E93,0x3EB2,0x0ED1,0x1EF0
};

/**********************************************************************************
* Function	 : 	enum frame_error_e transmit_ammeter_edmi_data(frame_edmi_param_t *send_frame_data,
* 				frame_edmi_param_t *recv_frame_data, enum ammeter_uart_e port)
* Description: 	传输645-1997规约的电表的信息
* Arguments	 : 	(1)send_frame_data:输入电表信息的结构体；（2）recv_frame_data：输出电表信息的结构体；（3）port：发送数据的端口；
* Return	 : 	FRAME_E_OK
*************************************************************************************/
enum frame_error_e transmit_ammeter_edmi_data(frame_edmi_param_t *send_frame_data, frame_edmi_param_t *recv_frame_data, enum ammeter_uart_e port)
{
	int i = 0;
	static rt_uint16_t serial_num = 0x14;
	enum frame_error_e result = FRAME_E_ERROR;
	frame_edmi_ctrl_e record_ctrl;

	for (i = 0; i < TRY_GET_EM_DATA_CNT_MAX; i++)	{
		if (++serial_num > 0x01FF) {
			serial_num = 0x14;
		}
		send_frame_data->serial_num = serial_num;

		result = send_recv_edmi_frame(send_frame_data, recv_frame_data, port);
		if (result == FRAME_E_OK) {

			meterp_debug_edmi(("%s(), cnt:%d, ctrl:%x, prev-ctrl:%x\n", __func__, i,
					send_frame_data->ctrl, record_ctrl));

			if (serial_num != recv_frame_data->serial_num) {
				ERROR_PRINTF(("ERR: func:%s, line(%d), edmi seq-num invalid(%d, %d)\n",
						__FUNCTION__, __LINE__, serial_num, recv_frame_data->serial_num));
			} else if (send_frame_data->ctrl == EDMI_CMD_LOGIN) {
				send_frame_data->ctrl = record_ctrl;
				result = FRAME_E_FLAG_ERROR;
				rt_thread_delay(get_ticks_of_ms(600));
				continue;
			} else {
				/* nothing */
			}
			break;
		} else if (result == FRAME_E_FLAG_ERROR) {
			record_ctrl = send_frame_data->ctrl;
			send_frame_data->ctrl = EDMI_CMD_LOGIN;
		}

		rt_thread_delay(get_ticks_of_ms(1000));
	}

	return result;
}
/**********************************************************************************
* Function	 : 	static enum frame_error_e send_recv_edmi_frame(frame_edmi_param_t *send_frame_data,
* 				frame_edmi_param_t *recv_frame_data, enum ammeter_uart_e port)
* Description: 	发送和接收645-1997规约电表的信息
* Arguments	 : 	(1)send_frame_data:输入电表信息的结构体；（2）recv_frame_data：输出电表信息的结构体；（3）port：发送数据的端口；
* Return	 : 	FRAME_E_OK
*************************************************************************************/
static enum frame_error_e send_recv_edmi_frame(frame_edmi_param_t *send_frame_data, frame_edmi_param_t *recv_frame_data, enum ammeter_uart_e port)
{
	rt_err_t ret = RT_EOK;
	enum frame_error_e result = FRAME_E_OK;
	rt_uint32_t recv_frame_len = 0;
	rt_uint32_t send_frame_len = 0;
	rt_uint8_t recv_frame_buf[256] = {'\0'};
	rt_uint8_t send_frame_buf[64] = {'\0'};
    struct uart_485_param uart_485_data;

    rt_memset(&uart_485_data, 0, sizeof(struct uart_485_param));

	if(RT_EOK != create_frame_edmi(send_frame_buf, &send_frame_len, send_frame_data)) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	if(RT_EOK == ammeter_mutex_take(port, 500)) {
		if (RT_EOK == get_save_ammeter_baud(send_frame_data->dest_addr, &uart_485_data)) {
			if (RT_EOK != set_485sw_and_em_usart(uart_485_data, port, AP_PROTOCOL_EDMI)) {
				ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
				goto END_MUTEX_TAKE;
			}
		} else {
			uart_485_data.baudrate = 1200;
			uart_485_data.databits = 8;
			uart_485_data.paritybit = 0;
			uart_485_data.stopbits = 1;
			if (RT_EOK != set_485sw_and_em_usart(uart_485_data, port, AP_PROTOCOL_EDMI)) {
				ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
				goto END_MUTEX_TAKE;
			}
		}

		em_protoco_send_data_by_485(get_485_port_from_ammeter_uart(port), send_frame_buf, send_frame_len);

	#if PRINT_SEND_FRAME_BUF_DATA
		if (is_sub_m_on(dsw_sub_module[DMN_METER_PROTO], DSMN_MP_EDMI)) {
			print_buf_in_hex("send_edmi_frame_buf", (char *)send_frame_buf, send_frame_len);
		}
	#endif

		ret = recv_data_from_485(port, recv_frame_buf, (rt_uint32_t *)&recv_frame_len,
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
		result = analysis_frame_edmi(recv_frame_buf, recv_frame_len, recv_frame_data);
		if(result == FRAME_E_OK) {
			if (recv_frame_data->ctrl == 0x18) {
				ERROR_PRINTF(("Not recognize this EDMI reg! ctrl:%x, dataflag:%x\n\n",
						send_frame_data->ctrl, send_frame_data->data_flag));
				return FRAME_E_FLAG_ERROR;
			}
			if (recv_frame_data->ctrl == 0x15) {
				return FRAME_E_CS_ERR;
			}

			return FRAME_E_OK;
		} else {
			ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
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

/* *计算并输出 check_code */
static void create_check_code(rt_uint8_t *cp, rt_uint32_t len, rt_uint8_t check[2])
{
	rt_uint16_t check_code = 0;

	while (len--) {
		check_code = (check_code << 8) ^ check_code_tab[((unsigned char)(check_code >> 8) ^ *cp++) & 0xff];
	}

	check[0] = (check_code >> 8) & 0xff;
	check[1] = check_code & 0xff;

	return;
}

/**********************************************************************************
* Function	 : 	static rt_err_t create_frame_edmi(rt_uint8_t *send_frame_buf, frame_edmi_param_t *frame_param)
* Description: 	创建645-1997帧格式
* Arguments	 : 	(1)send_frame_buf:输出的帧数据；（2）frame_param：组成帧格式的各种参数
* Return	 : 	RT_EOK
*************************************************************************************/
static rt_err_t create_frame_edmi(rt_uint8_t *send_frame_buf, rt_uint32_t *send_frame_len, frame_edmi_param_t *frame_param)
{
	rt_uint8_t i = 0, temp = 0, p;
	rt_uint8_t check[2] = {'\0'};
	rt_uint8_t ammeter_flag[2] = {'\0'};
	rt_uint32_t sn = 0;
	rt_uint8_t buf[16] = {0x45, 0x44, 0x4D, 0x49, 0x2C, 0x49, 0x4D, 0x44, 0x45, 0x49, 0x4D, 0x44, 0x45, 0x00};
	rt_uint8_t tmp_buf[64] = {'\0'};

	if (RT_NULL==send_frame_buf || RT_NULL==send_frame_len || RT_NULL==frame_param) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_EIO;
	}

	/* 对数组进行赋值 */
	*send_frame_buf       = 0x02;
	*(send_frame_buf + 1) = 0x45;

	for (i = 0; i < 6; i++) {
		tmp_buf[i] = (*((frame_param->dest_addr) + i) & 0x0f) + ((*((frame_param->dest_addr) + i) >> 4) * 10);
	}
	sn = tmp_buf[0]*10000000000 + tmp_buf[1]*100000000 + tmp_buf[2]*1000000 + tmp_buf[3]*10000 + tmp_buf[4]*100 + tmp_buf[5];
	rt_memset(tmp_buf, 0, sizeof(tmp_buf));

	ammeter_flag[0] = (frame_param->data_flag>>8) & 0xff;
	ammeter_flag[1] = frame_param->data_flag & 0xff;

	for (i = 0; i < 4; i++) {
		*(send_frame_buf + 5 - i) = (sn >> i * 8) & 0xff;
		*(send_frame_buf + 6 + i) = 0x00;
	}

	*(send_frame_buf + 9) = 0x01;
	*(send_frame_buf + 10) = (frame_param->serial_num >> 8) & 0xff;
	*(send_frame_buf + 11) = frame_param->serial_num & 0xff;

	if (frame_param->ctrl == EDMI_CMD_PATTERN) { /* 进入命令模式 */
		temp = 0;
		goto crc;
	}


	*(send_frame_buf+12)= frame_param->ctrl;	/* 电能表控制码 */
	temp = 1;

	switch (frame_param->ctrl) {
	case EDMI_CMD_LOGIN:					/* 登录命令 */
		rt_memcpy(send_frame_buf+12+temp, buf, 14);
		temp += 14;
		break;

	case EDMI_CMD_EXIT:						/* 退出命令 */
		*(send_frame_buf+13)= 0x00;			/* 空终止符 */
		temp += 1;
		break;

	case EDMI_CMD_REGISTER_INFO:			/* 查询寄存器信息 */
		*(send_frame_buf+12+temp) = ammeter_flag[0];
		*(send_frame_buf+13+temp) = ammeter_flag[1];
		temp += 2;
		break;

	case EDMI_CMD_READ_REGISTER:			    /* 读寄存器 */
		*(send_frame_buf+12+temp) = ammeter_flag[0];
		*(send_frame_buf+13+temp) = ammeter_flag[1];
//		*(send_frame_buf+14+temp) = 0x44;
		temp += 2;
		break;

	case EDMI_CMD_WRITE_REGISTER:			    /* 写寄存器 */
		*(send_frame_buf+12+temp) = ammeter_flag[0];
		*(send_frame_buf+13+temp) = ammeter_flag[1];

		for(i = 0; i < frame_param->data_len; i++)
			*(send_frame_buf + 14 + temp + i) = *(frame_param->data + i);
		temp += frame_param->data_len + 2;
		break;

	case EDMI_CMD_WRITE_EXTEND_REGISTER:			    /* 写扩展寄存器 */
		*(send_frame_buf+12+temp) = 0x00;
		temp += 1;
		*(send_frame_buf+12+temp) = 0x00;
		temp += 1;
		*(send_frame_buf+12+temp) = ammeter_flag[0];
		*(send_frame_buf+13+temp) = ammeter_flag[1];

		for(i = 0; i < frame_param->data_len; i++)
			*(send_frame_buf + 14 + temp + i) = *(frame_param->data + i);
		temp += frame_param->data_len + 2;
		break;

	default:
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ENOMEM;
	}

crc:
	create_check_code(send_frame_buf, 12+temp, check);		/*校验码*/
	*(send_frame_buf + 12 + temp) = check[0];
	*(send_frame_buf + 13 + temp) = check[1];
	temp += 2;

	for (i = 0; i < temp; i++) {
			p = *(send_frame_buf + 12 + i);
				if (p == 0x02 || p == 0x03 || p == 0x10 || p == 0x11 || p == 0x13) {
					rt_memcpy(tmp_buf, send_frame_buf + 12 + i, temp - i);
					rt_memcpy(send_frame_buf + 13 + i, tmp_buf, temp - i);
					*(send_frame_buf + 12 + i)= 0x10;
					*(send_frame_buf + 13 + i) += 0x40;
					temp += 1;
				}
		}

	temp += 12;
	*(send_frame_buf + temp) = 0x03;		/*结束符*/
	*send_frame_len = temp + 1;

	return RT_EOK;
}
/**********************************************************************************
* Function	 : 	static enum frame_error_e analysis_frame_edmi(rt_uint8_t *recv_frame_buf,rt_uint8_t recv_frame_length, frame_edmi_param_t *frame_param)
* Description: 	解析615-1997帧数据
* Arguments	 : 	(1)recv_frame_buf:输入帧数据；（2）recv_frame_length：输入数据帧的长度；（3）frame_param：传输数据帧解析后的各项参数
* Return	 : 	RT_EOK
*************************************************************************************/
static enum frame_error_e analysis_frame_edmi(rt_uint8_t *recv_frame_buf,rt_uint8_t recv_frame_length, frame_edmi_param_t *frame_param)
{
	 rt_uint8_t i = 0, flag = 0, check[2] = {'\0'};
	 rt_uint8_t *start = RT_NULL;
	 rt_uint8_t *end = RT_NULL;
	 rt_uint8_t cunt = 0;

	 if (recv_frame_buf == RT_NULL || frame_param == RT_NULL || frame_param->data == RT_NULL) {
	 	ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
	 	return FRAME_E_ERROR;
	 }

	 for (i = 0; i < recv_frame_length; i++) {
		 if (*recv_frame_buf == 0x02 && flag == 0) { /*search the 02H*/
			 start = recv_frame_buf;
			 flag = 1;
		 }

		 if (*recv_frame_buf == 0x03 && flag == 1) { /*search the 03H*/
			 end = recv_frame_buf;
			 break;
		 }
		 recv_frame_buf++;
	 }

	 if (i == recv_frame_length) {
		 ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		 return FRAME_E_FRAG;	/*此帧是个片段*/
	 }

	 cunt = end - start + 1;

	 for (i = 0; i < cunt - 12; i++) {
		 if (*(start + 12 + i) == 0x10) {
			 rt_memcpy(start + 12 + i, start + 13 + i, cunt - 12 - i);
			 *(start + 12 + i) -= 0x40;
			 cunt--;
		 }
	 }

	create_check_code(start, cunt - 3, check);		/*校验码*/
	if (*(start + cunt - 3) != check[0] || *(start + cunt - 2) != check[1]) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_CS_ERR;	   /*校验错误*/
	}

//	frame_param->src_addr = *(start+2) << 24 | *(start+3) << 16 | *(start+4) << 8 | *(start+5);
//	frame_param->dest_addr = *(start+6) << 24 | *(start+7) << 16 | *(start+8) << 8 | *(start+9);
	frame_param->serial_num = *(start+10) << 8 | *(start+11);
	frame_param->ctrl = *(start+12);

	if (cunt == 18){
		frame_param->data_flag =  *(start+13) << 8 | *(start+14);
		frame_param->data_len =  0;
	} else if (cunt > 18) {
		frame_param->data_flag =  *(start+13) << 8 | *(start+14);
		frame_param->data_len =  cunt - 18;
		rt_memcpy(frame_param->data, start + 15, cunt - 18);
	} else {
		frame_param->data_flag =  0;
		frame_param->data_len =  0;
	}

	return FRAME_E_OK; 			/*执行正确，返回0*/
}





