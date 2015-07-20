/**********************************************************************************
* Filename   	: ammeter_usart.c
* Description 	: define the functions that ammeters Communication port
* Begin time  	: 2014-01-07
* Finish time 	: 
* Engineer		: zhanghonglei
*************************************************************************************/
#include <rtdef.h>
#include <ammeter_usart.h>
#include <board.h>
#include <debug_sw.h>
#include <sys_cfg_api.h>
#include <misc_lib.h>
#include <auto_negoti_timer.h>

#define ERROR_PRINTF(x) printf_syn x

#define lt485_debug(x)	printf_syn x

#define PRINT_UART_TX_RX_DATA	1

struct uart_485_param uart_485_param_record;

static struct rt_mutex uart485_1_ammeter_mutex;
static struct rt_mutex uart485_2_ammeter_mutex;

/* 表示emc是否处于協商状态 */
static volatile int is_emc_in_negotiation_state;

static int start_auto_negotiation_timer(void);
//static int stop_auto_negotiation_timer(void);
//static int reset_auto_negotiation_timer(void);
static void set_emc_uart_to_def_param(void);

int is_emc_in_negotiation_state_f(void)
{
	return is_emc_in_negotiation_state;
}

/**********************************************************************************
* Function	 : 	void ammeter_port_mutex_init(void)
* Description	 : 	初始化电表端口的信号量
* Arguments	 : 	void
* Return	 : 	void
*************************************************************************************/
void ammeter_port_mutex_init(void)
{
	rt_mutex_init(&uart485_1_ammeter_mutex, "em1", RT_IPC_FLAG_PRIO);
	rt_mutex_init(&uart485_2_ammeter_mutex, "em2", RT_IPC_FLAG_PRIO);
	
	return;
}

/**********************************************************************************
* Function	 : 	rt_err_t uart485_sem_take(enum ammeter_uart_e port, rt_int32_t time)
* Description: 	获取信息到达的信号量
* Arguments	 : 	（1）port：电表所连接的端口；（2）time：获取信号量的超时时间
* Return	 : 	RT_EOK
*************************************************************************************/
rt_err_t uart485_sem_take(enum ammeter_uart_e port, rt_int32_t time)
{
	rt_err_t ret = RT_EOK;
	rt_int32_t tmp = 0;
	
	if (time != RT_WAITING_FOREVER) {
		tmp = get_ticks_of_ms(time);
	} else {
		tmp = time;
	}

	switch (port) {
	case AMMETER_UART1:
		ret = rt_sem_take(&uart485_1_rx_byte_sem, tmp);
		break;
		
	case AMMETER_UART2:
		ret = rt_sem_take(&uart485_2_rx_byte_sem, tmp);
		break;
		
	default:
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}
	
	return ret;
}

/**********************************************************************************
* Function	 : 	rt_err_t ammeter_mutex_take(enum ammeter_uart_e port, rt_int32_t time)
* Description: 	获取电表端口的信号量
* Arguments	 : 	（1）port：电表所连接的端口；（2）time：获取信号量的超时时间
* Return	 : 	RT_EOK
*************************************************************************************/
rt_err_t ammeter_mutex_take(enum ammeter_uart_e port, rt_int32_t time)
{
	rt_err_t ret = RT_EOK;
	
	if (time != RT_WAITING_FOREVER)
		time = get_ticks_of_ms(time);
	
	switch (port) {
	case AMMETER_UART1:
		ret = rt_mutex_take(&uart485_1_ammeter_mutex, time);
		break;
		
	case AMMETER_UART2:
		ret = rt_mutex_take(&uart485_2_ammeter_mutex, time);
		break;
		
	default:
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}
	
	return ret;
}

/**********************************************************************************
* Function	 : 	rt_err_t ammeter_mutex_release(enum ammeter_uart_e port)
* Description: 	释放电表端口的信号量
* Arguments	 : 	（1）port：电表所连接的端口；
* Return	 : 	RT_EOK
*************************************************************************************/
rt_err_t ammeter_mutex_release(enum ammeter_uart_e port)
{
	rt_err_t ret = RT_EOK;
	
	switch (port) {
	case AMMETER_UART1:
		ret = rt_mutex_release(&uart485_1_ammeter_mutex);
		break;
		
	case AMMETER_UART2:
		ret = rt_mutex_release(&uart485_2_ammeter_mutex);
		break;
		
	default:
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}
	
	return ret;
}

/**********************************************************************************
* Function	 : 	USART_TypeDef *get_485_port_from_ammeter_uart(enum ammeter_uart_e port)
* Description: 	电表自定义端口所对应的实际硬件485端口
* Arguments	 : 	（1）port：电表所连接的端口；
* Return	 : 	USART_TypeDef *ptr;
*************************************************************************************/
USART_TypeDef *get_485_port_from_ammeter_uart(enum ammeter_uart_e port)
{
	USART_TypeDef *ptr;

	switch (port) {
	case AMMETER_UART1:
		ptr = UART_485_1_DEV_PTR;
		break;

	case AMMETER_UART2:
		ptr = UART_485_2_DEV_PTR;
		break;
		
	case AMMETER_UART3:
		ptr = UART_485_3_DEV_PTR;
		break;
		
	default:
		ptr = NULL;
		break;
	}

	return ptr;
}
/**********************************************************************************
* Function	 : 	rt_err_t recv_data_from_485(enum ammeter_uart_e port, rt_uint8_t recv_buf[256],
* 		rt_uint32_t *recv_len, rt_int32_t format_time_out, rt_int32_t byte_time_out)
* Description: 	读取电表发送的数据
* Arguments	 : 	（1）port：电表所连接的端口；（2）recv_buf：输出接收到的数据；（3）recv_len:输出接收到的数据长度；
* 				（4）format_time_out：设置接收帧数据的超时时间；（5）byte_time_out：设置接收字节数据的超时时间
* Return	 : 	RT_EOK
*************************************************************************************/
rt_err_t recv_data_from_485(enum ammeter_uart_e port, rt_uint8_t recv_buf[256],
		rt_uint32_t *recv_len, rt_int32_t format_time_out, rt_int32_t byte_time_out)
{
	rt_uint32_t len = 0;
	rt_uint32_t tmp = 0;
	rt_uint8_t buf[256] = {"\0"};

	if (RT_EOK == uart485_sem_take(port, format_time_out)) {
		tmp = recv_data_by_485(get_485_port_from_ammeter_uart(port), buf, 256);
		len = tmp;
//		reset_auto_negotiation_timer();

		while (1) {
			if (RT_EOK == uart485_sem_take(port, byte_time_out)) {
				tmp = recv_data_by_485(get_485_port_from_ammeter_uart(port), buf + len, 256-len);
				len += tmp;
			} else {
				break;
			}

			if (len >= 256)
				break;
		}

		*recv_len = len;
		rt_memcpy(recv_buf, buf, len);
#if PRINT_UART_TX_RX_DATA
		if (is_sub_m_on(dsw_sub_module[DMN_METER_PROTO], DSMN_MP_USART_RECV)) {
			print_buf_in_hex("recv_data_from_485", (char *)buf, len);
		}
#endif
		return RT_EOK;
	} else {
//		ERROR_PRINTF(("recevie frame timed out!\n"));
		return RT_ETIMEOUT;
	}

	return RT_ERROR;
}
#ifdef USING_RS485SW_USART

static rt_err_t create_frame_485sw(rt_uint8_t *send_frame_buf, rt_uint32_t *send_frame_len,
		struct uart_485_param data, enum ammeter_protocal_e amm_protoc)
{
	rt_uint8_t i = 0, check = 0;

	if (RT_NULL==send_frame_buf) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	/* 帧起始符 */
	*send_frame_buf       = 0x57;
	/* 波特率 */
	rt_memcpy(send_frame_buf+1, &(data.baudrate), 3);

	*(send_frame_buf + 4) = 0x57;
	/* 控制码 */
	*(send_frame_buf + 5) = 0x0;
	/* 数据长度 */
	*(send_frame_buf + 6) = 0x2;
	/* 数据位 */
	*(send_frame_buf + 7) = data.databits << 4;
	/* 校验位 */
	*(send_frame_buf + 7) += data.paritybit << 2;
	/* 停止位 */
	*(send_frame_buf + 7) += data.stopbits;
	/* 电表协议 */
	*(send_frame_buf + 8) += amm_protoc;
	/* 帧校验码 */
	for (i = 0; i < 9; i++) {
		check += *(send_frame_buf+i);
	}
	*(send_frame_buf + 9) = check;
	/* 结束符 */
	*(send_frame_buf + 10) = 0x75;

	*send_frame_len = 11;

	return RT_EOK;
}

static rt_err_t analysis_frame_485sw(rt_uint8_t *recv_frame_buf, rt_uint32_t recv_frame_len, rt_uint32_t *baut, rt_uint8_t *ctrl)
{
	rt_uint8_t i = 0, check = 0;

	if (RT_NULL == recv_frame_buf || RT_NULL == baut || RT_NULL == ctrl) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	/* 帧起始符 */

	if (*recv_frame_buf != 0x57 || *(recv_frame_buf + 4) != 0x57 || *(recv_frame_buf + 8) != 0x75) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	/* 帧校验码 */
	for (i = 0; i < 7; i++) {
		check += *(recv_frame_buf+i);
	}
	if (*(recv_frame_buf + 7) != check) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	/* 波特率 */
	rt_memcpy(baut, recv_frame_buf + 1, 3);
	*ctrl = *(recv_frame_buf + 5);

	return RT_EOK;
}

static rt_err_t set_485sw_usart(struct uart_485_param data, enum ammeter_uart_e port,
		enum ammeter_protocal_e amm_protoc)
{
	rt_err_t ret = RT_ERROR;
	rt_uint32_t recv_baut = 0;
	rt_uint32_t send_frame_len = 0;
	rt_uint32_t recv_frame_len = 0;
	rt_uint8_t recv_frame_buf[256] = {'\0'};
	rt_uint8_t send_frame_buf[16] = {'\0'};
	rt_uint8_t ctrl = 0, i = 0;

	if(RT_EOK != create_frame_485sw(send_frame_buf, &send_frame_len, data, amm_protoc)) {
		ERROR_PRINTF(("ERR: func:%s, line(%d) create_frame_485sw fail\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	for (i = 0; i < TRY_GET_EM_DATA_CNT_MAX; i++) {
		if(RT_EOK == ammeter_mutex_take(port, 500)) {
			send_data_by_485(get_485_port_from_ammeter_uart(port), send_frame_buf, send_frame_len);
	#if PRINT_UART_TX_RX_DATA
			if (is_sub_m_on(dsw_sub_module[DMN_METER_PROTO], DSMN_MP_USART_RECV)) {
				print_buf_in_hex("send_485sw_usart_frame_buf",
						(char *)send_frame_buf, send_frame_len);
			}
	#endif
			if (RT_EOK == recv_data_from_485(port, recv_frame_buf, &recv_frame_len,
					EM_PROTO_INTER_FRAME_TIMEOUT_MS, EM_PROTO_INTER_BYTE_TIMEOUT_MS)) {
#if 0 //PRINT_UART_TX_RX_DATA
				if (is_sub_m_on(dsw_sub_module[DMN_METER_PROTO], DSMN_MP_USART_RECV)) {
					print_buf_in_hex("recv_485sw_usart_frame_buf",
							(char *)recv_frame_buf, recv_frame_len);
				}
#endif
				if (RT_EOK == analysis_frame_485sw(recv_frame_buf, recv_frame_len, &recv_baut, &ctrl)) {
					if (ctrl == 0x81) {
						start_auto_negotiation_timer();
						ret = RT_EOK;
					} else {
						ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
						ret = RT_ERROR;
					}
				} else {
					ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
					ret = RT_ERROR;
				}
			} else {
				ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
				ret = RT_ETIMEOUT;
			}

			if(RT_EOK != ammeter_mutex_release(port)) {
				ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
				return FRAME_E_ERROR;
			}
		} else {
			printf_syn("%s(), port-%d ammeter_sem_take fail(%d)\n", __func__, port, ret);
		}


		if (ret == RT_EOK) {
			break;
		}

		rt_thread_delay(get_ticks_of_ms(300));
	}

	return ret;
}
#endif

static rt_err_t set_emc_to_485sw_uart_param(struct uart_485_param *data)
{
	rt_err_t ret = RT_EOK;

	if (rt_memcmp(&uart_485_param_record, data, sizeof(struct uart_485_param)) != 0) {
		ret = UART_if_Set(data->baudrate, data->databits, data->paritybit, data->stopbits, 2);
		if (RT_EOK != ret) {
			ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
			return RT_ERROR;
		} else {
			rt_memcpy(&uart_485_param_record, &data, sizeof(struct uart_485_param));
			sinki_debug_body(("\n%s() ok: %d,%d,%d,%d, cur_tick:%u\n\n", __func__,
					data->baudrate, data->databits,
					data->paritybit, data->stopbits, rt_tick_get()));

		}
	}

	return ret;
}

rt_err_t set_485sw_and_em_usart(struct uart_485_param data, enum ammeter_uart_e port_485,
		enum ammeter_protocal_e amm_protoc)
{
	rt_err_t ret = RT_EOK;
	int cnt, fail;
	struct electric_meter_reg_info_st *amm_sn;
	enum connect_485sw_status connect_485sw_status;

	amm_sn = rt_malloc(sizeof(*amm_sn));
	if (NULL == amm_sn) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	if (SUCC != get_em_reg_info(amm_sn)) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		goto END_ERROR;
	}

	connect_485sw_status = amm_sn->connect_485sw_status;
	rt_free(amm_sn);

	if (connect_485sw_status == EM_NOT_CONNECT_485SW) {
		if (RT_EOK != set_emc_to_485sw_uart_param(&data)) {
			ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
			return RT_ERROR;
		}

		return RT_EOK;
	}


again:
	cnt = 0;
	fail = 0;
	if (is_emc_in_negotiation_state) {
		do {
			rt_thread_delay(get_ticks_of_ms(100+cnt*10));
			set_emc_uart_to_def_param();
			ret = set_485sw_usart(data, port_485, amm_protoc);
			if (RT_EOK == ret) {
				ret = set_emc_to_485sw_uart_param(&data);
				fail = 0;
			} else {
				printf_syn("NOTE:set_485sw_usart fail\n");
				fail = 1;
			}
			++cnt;
		} while (fail && cnt<4);
	} else {
		if (rt_memcmp(&uart_485_param_record, &data, sizeof(struct uart_485_param)) != 0) {
			rt_thread_delay(get_ticks_of_ms(50));
			goto again;
		}
	}

	return ret;

END_ERROR:
	rt_free(amm_sn);

	return RT_ERROR;
}



rt_size_t em_protoco_send_data_by_485(USART_TypeDef *dev_485, void *data, rt_size_t len)
{
	rt_size_t ret;

//	reset_auto_negotiation_timer();
	ret = send_data_by_485(dev_485, data, len);
//	clr_rx_buf_recv_data_by_485(dev_485);
//	reset_auto_negotiation_timer();

	return ret;
}


#if 1 /* add by David */

#define EMC_AUTO_ENTER_NEGOTIATION_STATE_TIME_MS (900-3)

#define lt485_debug_t(x) 	//printf_syn x


struct rt_timer emc_auto_enter_negotiation_state_timer;

static void set_emc_uart_to_def_param(void)
{
	USART_InitTypeDef USART_InitStructure;

	USART_StructInit(&USART_InitStructure);

	USART_InitStructure.USART_BaudRate = 2400;
	USART_InitStructure.USART_WordLength = USART_WordLength_9b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_Even;

	uart_485_param_record.baudrate = 2400;
	uart_485_param_record.databits = 9;
	uart_485_param_record.paritybit = 1;
	uart_485_param_record.stopbits = 1;

	USART_Cmd(USART2, DISABLE);
	USART_Init(USART2, &USART_InitStructure);
	USART_Cmd(USART2, ENABLE);

	return;
}

void emc_state_timeout(void *p)
{
	if (is_sub_m_on(dsw_sub_module[DMN_SINKINFO], DSMN_SI_BODY)) {
		rt_kprintf("\n%s(), cur_tick:%u\n\n", __func__, rt_tick_get());
	}

	set_emc_uart_to_def_param();
	is_emc_in_negotiation_state = 1;
}


void init_auto_negotiation_timer(void)
{
#if 0
	rt_timer_init(&emc_auto_enter_negotiation_state_timer, "emcstat",   /* 定时器名字 */
			emc_state_timeout, /* 超时时回调的处理函数 */
			NULL, /* 超时函数的入口参数 */
			get_ticks_of_ms(EMC_AUTO_ENTER_NEGOTIATION_STATE_TIME_MS),
			RT_TIMER_FLAG_ONE_SHOT); /* 单次定时器 */
#else
	init_auto_negoti_timer(get_auto_negoti_tikcs_of_ms(EMC_AUTO_ENTER_NEGOTIATION_STATE_TIME_MS));
	cfg_auto_negoti_timer_nvic();
	enable_auto_negoti_timer_int();
#endif
	is_emc_in_negotiation_state = 1;

}

static int start_auto_negotiation_timer(void)
{
#if 0
	if (is_bit_set(emc_auto_enter_negotiation_state_timer.parent.flag, RT_TIMER_FLAG_ACTIVATED))
		return SUCC;

	/* 只有已启动时，才会失败 */
	if (RT_EOK == rt_timer_start(&emc_auto_enter_negotiation_state_timer)) {
		lt485_debug_t(("%s() init_tick:%u, timeout_tick:%u, cur_tick:%u\n",
				__func__,
				emc_auto_enter_negotiation_state_timer.init_tick,
				emc_auto_enter_negotiation_state_timer.timeout_tick,
				rt_tick_get()));
		is_emc_in_negotiation_state = 0;
	} else {
//		lt485_debug_t(("warning:%s() lt-if-%d\n", __func__, lt_if));
		return FAIL;
	}
#else
	start_auto_negoti_timer();
	is_emc_in_negotiation_state = 0;
	meterp_debug_uart(("%s() cur os tick:%u\n", __func__, rt_tick_get()));
#endif
	return SUCC;
}

#if 0
static int stop_auto_negotiation_timer(void)
{
	/* 只有未启动时，才会失败 */
	if (RT_EOK == rt_timer_stop(&emc_auto_enter_negotiation_state_timer)) {
		lt485_debug_t(("%s() init_tick:%u, timeout_tick:%u, cur_tick:%u\n",
				__func__,
				emc_auto_enter_negotiation_state_timer.init_tick,
				emc_auto_enter_negotiation_state_timer.timeout_tick,
				rt_tick_get()));

		set_emc_uart_to_def_param();
		is_emc_in_negotiation_state = 1;
	} else {
//		lt485_debug_t(("warning:%s() lt-if-%d\n", __func__, lt_if));
		return FAIL;
	}

	return SUCC;
}
#endif

int reset_auto_negotiation_timer(void)
{
#if 0
	if(is_bit_set(emc_auto_enter_negotiation_state_timer.parent.flag, RT_TIMER_FLAG_ACTIVATED)) {
		rt_timer_stop(&emc_auto_enter_negotiation_state_timer);
		rt_timer_start(&emc_auto_enter_negotiation_state_timer);

		lt485_debug_t(("%s() init_tick:%u, timeout_tick:%u, cur_tick:%u\n",
				__func__,
				emc_auto_enter_negotiation_state_timer.init_tick,
				emc_auto_enter_negotiation_state_timer.timeout_tick,
				rt_tick_get()));
	} else {
//		lt485_debug_t(("warning:%s() lt-if-%d\n", __func__, lt_if));
	}
#else
	reset_auto_negoti_timer();
	lt485_debug_t(("%s() cur os tick:%u\n", __func__, rt_tick_get()));
#endif
	return SUCC;
}



#endif

