/**********************************************************************************
* Filename   	: ammeter.c
* Description 	: define the functions that read the ammeters data
* Begin time  	: 2014-04-21
* Finish time 	:
* Engineer		: zhanghonglei
*************************************************************************************/
#include <ammeter.h>
#include <rtdef.h>
#include <sys_cfg_api.h>
#include <sinkinfo_common.h>
#include <private_trap.h>
#include <sink_info.h>
#include <ammeter_645_proxy.h>
#include <ammeter_edmi.h>
#include <debug_sw.h>
#include <finsh.h>

#define ERROR_PRINTF(x) printf_syn x

#define TEST_EM_PROTOC	0

static rt_uint8_t READ_AMM_FLAG;
static struct Ammeter_Node ammeter_info[NUM_OF_COLLECT_EM_MAX];

/*********** 初始化函数 *************/
//static rt_err_t get_flash_amm_addr(void);
static rt_err_t get_save_ammeter_protocol(rt_uint8_t *input_addr, enum ammeter_protocal_e *output_protocal, enum ammeter_style *output_style);

/*********** 读表函数 *************/
static enum frame_error_e read_ammeter_power_data(rt_uint8_t *addr, enum ammeter_cmd_e cmd, rt_uint8_t *output_data, rt_uint32_t *output_data_len, enum ammeter_uart_e port);
static enum frame_error_e read_ammeter_forzen_data(rt_uint8_t *addr, enum ammeter_forzen_cmd cmd, rt_uint8_t forzen_times, rt_uint8_t *output_data, rt_uint32_t *output_data_len, enum ammeter_uart_e port);
static enum frame_error_e read_ammeter_maxneed_data(rt_uint8_t *addr, enum ammeter_maxneed_cmd cmd, rt_uint8_t *output_data, rt_uint32_t *output_data_len, enum ammeter_uart_e port);
static enum frame_error_e read_ammeter_event_data(rt_uint8_t *addr, enum ammeter_event_cmd cmd, rt_uint8_t event_times, rt_uint8_t *output_data, rt_uint32_t *output_data_len, enum ammeter_uart_e port);
static rt_err_t str_sn_change_to_bcd_sn(rt_uint8_t *input_str_sn, rt_uint8_t *output_bcd_sn);

/*********** 645-1997通讯规约 *************/
//static enum frame_error_e get_data_from_645_97(rt_uint8_t *addr, rt_uint32_t data_flag, rt_uint8_t *output_data, rt_uint32_t *output_data_len, enum ammeter_uart_e port);
//static rt_err_t output645_97power_data_flag(enum ammeter_cmd_e cmd, rt_uint32_t *output_data_flag, rt_uint32_t *output_move_len);
static rt_err_t output645_97maxneed_data_flag(enum ammeter_maxneed_cmd cmd, rt_uint32_t *output_maxneed_data_flag, rt_uint32_t *output_time_data_flag);

/*********** 645-2007通讯规约 *************/
//static enum frame_error_e get_data_from_645_07(rt_uint8_t *addr, rt_uint32_t data_flag, rt_uint8_t *output_data, rt_uint32_t *output_data_len, enum ammeter_uart_e port);
//static rt_err_t output645_07power_data_flag(enum ammeter_cmd_e cmd, rt_uint32_t *output_data_flag);
static rt_err_t output645_07forzen_data_flag(enum ammeter_forzen_cmd cmd, rt_uint32_t *output_data_flag, rt_uint32_t *output_data_len);
static rt_err_t output645_07maxneed_data_flag(enum ammeter_maxneed_cmd cmd, rt_uint32_t *output_data_flag);
static enum frame_error_e read_ammeter645_07_event_data(rt_uint8_t *addr, rt_uint32_t addr_num, enum ammeter_uart_e port);
static rt_err_t ammeter645_07_event_A_phase_upload(rt_uint32_t amm_addr_num, rt_uint16_t status_word);
static rt_err_t ammeter645_07_event_B_phase_upload(rt_uint32_t amm_addr_num, rt_uint16_t status_word);
static rt_err_t ammeter645_07_event_C_phase_upload(rt_uint32_t amm_addr_num, rt_uint16_t status_word);
static rt_err_t output645_07event_cmd_data_len(rt_uint32_t input_data_flag, rt_uint8_t *input_data, rt_uint32_t input_data_len, enum ammeter_event_cmd *output_cmd, rt_uint8_t *output_data, rt_uint32_t *output_data_len);
static rt_err_t output645_07event_data_flag(enum ammeter_event_cmd cmd, rt_uint32_t *output_data_flag);
//static rt_err_t output645_07baud_data_flag(enum ammeter_baud_cmd cmd, rt_uint32_t *output_data_flag);
//static enum frame_error_e setting645_07ammeter_baud_rate(rt_uint8_t *addr, enum ammeter_baud_cmd cmd, enum ammeter_uart_e port);

/*********** EDMI通讯规约 *************/
static enum frame_error_e get_data_from_edmi(rt_uint8_t *addr, rt_uint32_t data_flag, rt_uint8_t *output_data, rt_uint32_t *output_data_len, enum ammeter_uart_e port);
#if 0
static enum frame_error_e amemter_edmi_login(rt_uint8_t *addr, frame_edmi_ctrl_e ctrl, enum ammeter_uart_e port);
#endif
static rt_err_t output_edmi_data_flag(enum ammeter_style style, enum ammeter_cmd_e cmd, rt_uint32_t *output_data_flag, enum data_type *output_data_type, rt_int32_t *decimals_len);
static rt_err_t output_edmi_maxneed_data_flag(enum ammeter_maxneed_cmd cmd, rt_uint32_t *output_maxneed_data_flag, rt_uint32_t *decimals_len, rt_uint32_t *output_time_data_flag);
static rt_err_t string_change_to_bcd_hex(rt_uint8_t *input_data, rt_uint32_t input_data_len, rt_uint8_t *output_data, rt_uint32_t *output_data_len);
static rt_uint8_t choose_the_sign_bit(enum ammeter_cmd_e cmd);
static rt_err_t float_change_to_bcd_hex(enum ammeter_cmd_e cmd, rt_uint8_t *input_data, rt_uint32_t input_data_len, rt_int32_t input_decimals_len, rt_uint8_t *output_data, rt_uint32_t *output_data_len);
static rt_err_t integer_change_to_bcd_hex(rt_uint8_t *input_data, rt_uint32_t input_data_len, rt_uint8_t *output_data, rt_uint32_t *output_data_len);

/**********************************************************************************
* Function	 : 	void ammeter_init(void)
* Description: 	初始化电表函数
* Arguments	 : 	void
* Return	 :  void
*************************************************************************************/
rt_err_t ammeter_init(void)
{
	/* 初始化电能端口信号量  */
	ammeter_port_mutex_init();

	init_auto_negotiation_timer();
#if TEST_EM_PROTOC
	dsw_set(3,1,1);
	dsw_set(3,2,1);
	dsw_set(3,3,1);
	dsw_set(3,8,1);
#endif
	/* 初始化电能表 */
	if(RT_EOK != try_get_em_protoco_baud_info()) {
		ERROR_PRINTF(("ERR:%s(), try_get_em_protoco_baud_info fail\n", __FUNCTION__));
		return RT_ERROR;
	}

	//ammeter_645_proxy_init();

	return RT_EOK;;
}

/**********************************************************************************
* Function	 : 	rt_err_t try_get_em_protoco_baud_info(void)
* Description: 	初始化电表信息，存储电表的地址和所用规约类型
* Arguments	 : 	void
* Return	 : 	RT_EOK
*************************************************************************************/
extern int get_mc_from_645(int chlx);

#if 1
/*
 * 从数据表格的struct electric_meter_reg_info_st em_reg_info;成员提取信息，来填充
 * static struct Ammeter_Node ammeter_info[NUM_OF_COLLECT_EM_MAX];
 * */
void update_elctric_meter_info(void)
{
	int i = 0;
	rt_uint8_t addr[6] = {'\0'};
	rt_uint32_t len = 0;
	rt_uint8_t buf[16] = {'\0'};
	rt_uint8_t ammeter_mk3[8] = {0x32, 0x30, 0x30, 0x30, 0x2D, 0x30, 0x34};
	rt_uint8_t ammeter_mk6[8] = {0x32, 0x30, 0x30, 0x30, 0x2D, 0x30, 0x36};
	rt_uint8_t ammeter_mk6e[8] = {0x32, 0x30, 0x30, 0x30, 0x2D, 0x36, 0x45};
	struct electric_meter_reg_info_st *amm_sn;

	amm_sn = rt_malloc(sizeof(*amm_sn));
	if (NULL == amm_sn) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return;
	}

	rt_memset(&ammeter_info, 0x00, sizeof(struct Ammeter_Node));

	READ_AMM_FLAG = 0;
	if (SUCC == get_em_reg_info(amm_sn)) {
		for (i = 0; i < NUM_OF_COLLECT_EM_MAX; i++) {
			if (str_sn_change_to_bcd_sn((rt_uint8_t *)(amm_sn->em_sn[i]), addr) == RT_EOK) {
				rt_memcpy(ammeter_info[i].ammeter_addr, addr, 6);
			} else {
				ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
				goto END_ERROR;
			}

			ammeter_info[i].ammeter_protocal = amm_sn->em_proto[i];
			register_em_info.em_proto[i] = amm_sn->em_proto[i];		/* hongbin added */
			rt_memcpy(&ammeter_info[i].ammeter_baud, &amm_sn->usart_param[i], sizeof(struct uart_485_param));

//			printf_syn("========================== start ==========================\n");
//			printf_syn("ammeter_info[%d].ammeter_protocal = %d\n", i, ammeter_info[i].ammeter_protocal);

			#if 1
			if (ammeter_info[i].ammeter_protocal == AP_PROTOCOL_EDMI) {
#if 0
				amemter_edmi_login(ammeter_info[i].ammeter_addr, EDMI_CMD_EXIT, AMMETER_UART1);
#endif

				if (FRAME_E_OK == get_data_from_edmi(ammeter_info[i].ammeter_addr, 0xF000, buf, &len, AMMETER_UART1)) {
//					int j = 0;
//					for (j = 0; j < len; j++) {
//						printf_syn("%02x ", buf[j]);
//					}
//					printf_syn("\n");

					if (rt_memcmp(buf, ammeter_mk3, 7) == 0) {
						ammeter_info[i].ammeter_style = AM_MK3;
					} else if (rt_memcmp(buf, ammeter_mk6, 7) == 0){
						ammeter_info[i].ammeter_style = AM_MK6;
					} else if (rt_memcmp(buf, ammeter_mk6e, 7) == 0){
						ammeter_info[i].ammeter_style = AM_MK6E;
					} else {
						ammeter_info[i].ammeter_style = AM_UNKNOWN;
					}
				} else {
//					printf_syn("========================== AM_UNKNOWN ==========================\n");
//					ammeter_info[i].ammeter_style = AM_UNKNOWN;
				}
			}
			#else
			if (ammeter_info[i].ammeter_protocal == AP_PROTOCOL_EDMI) {
				if (FRAME_E_OK == amemter_edmi_login(ammeter_info[i].ammeter_addr, EDMI_CMD_PATTERN, AMMETER_UART1)
						&& FRAME_E_OK == amemter_edmi_login(ammeter_info[i].ammeter_addr, EDMI_CMD_LOGIN, AMMETER_UART1)
						&& FRAME_E_OK == get_data_from_edmi(ammeter_info[i].ammeter_addr, 0xF000, buf, &len, AMMETER_UART1)) {
					if (rt_memcmp(buf, ammeter_mk3, 7) == 0) {
					ammeter_info[i].ammeter_style = AM_MK3;
					} else if (rt_memcmp(buf, ammeter_mk6, 7) == 0){
					ammeter_info[i].ammeter_style = AM_MK6;
					} else if (rt_memcmp(buf, ammeter_mk6e, 7) == 0){
					ammeter_info[i].ammeter_style = AM_MK6E;
					} else {
					ammeter_info[i].ammeter_style = AM_UNKNOWN;
					}
				} else {
					ammeter_info[i].ammeter_style = AM_UNKNOWN;
				}
			}
			#endif
//			printf_syn("========================== end(%d) ==========================\n", ammeter_info[i].ammeter_style);
		}
	} else {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		goto END_ERROR;
	}

END_ERROR:
	rt_free(amm_sn);
	READ_AMM_FLAG = 1;

	return;
}

rt_err_t try_get_em_protoco_baud_info(void)
{
	update_elctric_meter_info();
	return RT_EOK;
}

#else
rt_err_t try_get_em_protoco_baud_info(void)
{
	rt_err_t ret = RT_EOK;
    rt_uint32_t i = 0;
    rt_uint32_t len = 0;
    rt_uint8_t buf[16] = {'\0'};
    rt_uint8_t ammeter_zero_addr[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    rt_uint8_t ammeter_invalid_addr[6] = {0x99, 0x99, 0x99, 0x99, 0x99, 0x99};
    rt_uint8_t ammeter_mk3[8] = {0x32, 0x30, 0x30, 0x30, 0x2D, 0x30, 0x34};
    rt_uint8_t ammeter_mk6[8] = {0x32, 0x30, 0x30, 0x30, 0x2D, 0x30, 0x36};
    rt_uint8_t ammeter_mk6e[8] = {0x32, 0x30, 0x30, 0x30, 0x2D, 0x36, 0x45};
    enum ammeter_uart_e port = AMMETER_UART1;
    struct uart_485_param uart_485_data;

    rt_memset(&uart_485_data, 0, sizeof(struct uart_485_param));

    READ_AMM_FLAG = 0;

	if (get_flash_amm_addr() != RT_EOK) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		ret = RT_ERROR;
		goto END;
	}

	uart_485_data.baudrate = 2400;
	uart_485_data.databits = 8;
	uart_485_data.paritybit = 1;
	uart_485_data.stopbits = 1;

	if (RT_EOK != set_485sw_and_em_usart(uart_485_data, port)) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		ret = RT_ERROR;
		goto END;
	}

	for (i = 0; i < NUM_OF_COLLECT_EM_MAX; i++) {
		if ((rt_memcmp(ammeter_info[i].ammeter_addr, ammeter_zero_addr, 6) > 0) &&
			(rt_memcmp(ammeter_info[i].ammeter_addr, ammeter_invalid_addr, 6) < 0)) {
			if (FRAME_E_OK == get_data_from_645_07(ammeter_info[i].ammeter_addr, 0X04000402, buf, &len, port)) {
				ammeter_info[i].ammeter_protocal = AP_PROTOCOL_645_2007;
				register_em_info.em_proto[i] = AP_PROTOCOL_645_2007;
				rt_memcpy(&(ammeter_info[i].ammeter_baud), &uart_485_data, sizeof(struct uart_485_param));
//				get_mc_from_645(i);
			} else {
//				临时注释掉, mark by David
//				ammeter_info[i].ammeter_protocal = AP_PROTOCOL_UNKNOWN;
//				register_em_info.em_proto[i] = AP_PROTOCOL_UNKNOWN;
			}
//			rt_thread_delay(get_ticks_of_ms(200));
		} else {
			ammeter_info[i].ammeter_protocal = AP_PROTOCOL_NOTHING;
			register_em_info.em_proto[i] = AP_PROTOCOL_NOTHING;
		}
	}

	uart_485_data.baudrate = 1200;
	uart_485_data.databits = 8;
	uart_485_data.paritybit = 1;
	uart_485_data.stopbits = 1;

	if (RT_EOK != set_485sw_and_em_usart(uart_485_data, port)) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		ret = RT_ERROR;
		goto END;
	}

	for (i = 0; i < NUM_OF_COLLECT_EM_MAX; i++) {
		if (ammeter_info[i].ammeter_protocal == AP_PROTOCOL_UNKNOWN) {
			if (FRAME_E_OK == get_data_from_645_07(ammeter_info[i].ammeter_addr, 0X04000402, buf, &len, port)) {
				ammeter_info[i].ammeter_protocal = AP_PROTOCOL_645_2007;
				register_em_info.em_proto[i] = AP_PROTOCOL_645_2007;
				rt_memcpy(&(ammeter_info[i].ammeter_baud), &uart_485_data, sizeof(struct uart_485_param));
//				get_mc_from_645(i);
			}
//			rt_thread_delay(get_ticks_of_ms(200));
		}

	}

	for (i = 0; i < NUM_OF_COLLECT_EM_MAX; i++) {
		if (ammeter_info[i].ammeter_protocal == AP_PROTOCOL_UNKNOWN) {
			if (FRAME_E_OK == get_data_from_645_97(ammeter_info[i].ammeter_addr, 0XC032, buf, &len, port)) {
				ammeter_info[i].ammeter_protocal = AP_PROTOCOL_645_1997;
				register_em_info.em_proto[i] = AP_PROTOCOL_645_1997;
				rt_memcpy(&(ammeter_info[i].ammeter_baud), &uart_485_data, sizeof(struct uart_485_param));
//				get_mc_from_645(i);
			}
//			rt_thread_delay(get_ticks_of_ms(200));
		}
	}

	uart_485_data.baudrate = 1200;
	uart_485_data.databits = 8;
	uart_485_data.paritybit = 0;
	uart_485_data.stopbits = 1;

	if (RT_EOK != set_485sw_and_em_usart(uart_485_data, port)) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		ret = RT_ERROR;
		goto END;
	}

	for (i = 0; i < NUM_OF_COLLECT_EM_MAX; i++) {
		if (ammeter_info[i].ammeter_protocal == AP_PROTOCOL_UNKNOWN) {
			if (FRAME_E_OK == amemter_edmi_login(ammeter_info[i].ammeter_addr, EDMI_CMD_PATTERN, port)) {
				if (FRAME_E_OK == amemter_edmi_login(ammeter_info[i].ammeter_addr, EDMI_CMD_LOGIN, port)) {
					ammeter_info[i].ammeter_protocal = AP_PROTOCOL_EDMI;
					register_em_info.em_proto[i] = AP_PROTOCOL_EDMI;
					rt_memcpy(&(ammeter_info[i].ammeter_baud), &uart_485_data, sizeof(struct uart_485_param));

					if (FRAME_E_OK == get_data_from_edmi(ammeter_info[i].ammeter_addr, 0xF000, buf, &len, port)) {
						if (rt_memcmp(buf, ammeter_mk3, 7) == 0) {
							ammeter_info[i].ammeter_style = AM_MK3;
						} else if (rt_memcmp(buf, ammeter_mk6, 7) == 0){
							ammeter_info[i].ammeter_style = AM_MK6;
						} else if (rt_memcmp(buf, ammeter_mk6e, 7) == 0){
							ammeter_info[i].ammeter_style = AM_MK6E;
						} else {
							ammeter_info[i].ammeter_style = AM_UNKNOWN;
						}
					} else {
						ammeter_info[i].ammeter_style = AM_UNKNOWN;
					}

//					if (ammeter_info[i].ammeter_style != AM_UNKNOWN)
//						get_mc_from_645(i);
				}
//				rt_thread_delay(get_ticks_of_ms(200));
			}
		}
	}

	uart_485_data.baudrate = 9600;
	uart_485_data.databits = 8;
	uart_485_data.paritybit = 0;
	uart_485_data.stopbits = 1;

	if (RT_EOK != set_485sw_and_em_usart(uart_485_data, port)) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		ret = RT_ERROR;
		goto END;
	}

	for (i = 0; i < NUM_OF_COLLECT_EM_MAX; i++) {
		if (ammeter_info[i].ammeter_protocal == AP_PROTOCOL_UNKNOWN) {
			if (FRAME_E_OK == amemter_edmi_login(ammeter_info[i].ammeter_addr, EDMI_CMD_PATTERN, port)) {
				if (FRAME_E_OK == amemter_edmi_login(ammeter_info[i].ammeter_addr, EDMI_CMD_LOGIN, port)) {
					ammeter_info[i].ammeter_protocal = AP_PROTOCOL_EDMI;
					register_em_info.em_proto[i] = AP_PROTOCOL_EDMI;
					rt_memcpy(&(ammeter_info[i].ammeter_baud), &uart_485_data, sizeof(struct uart_485_param));

					if (FRAME_E_OK == get_data_from_edmi(ammeter_info[i].ammeter_addr, 0xF000, buf, &len, port)) {
						if (rt_memcmp(buf, ammeter_mk3, 7) == 0) {
							ammeter_info[i].ammeter_style = AM_MK3;
						} else if (rt_memcmp(buf, ammeter_mk6, 7) == 0){
							ammeter_info[i].ammeter_style = AM_MK6;
						} else if (rt_memcmp(buf, ammeter_mk6e, 7) == 0){
							ammeter_info[i].ammeter_style = AM_MK6E;
						} else {
							ammeter_info[i].ammeter_style = AM_UNKNOWN;
						}
					} else {
						ammeter_info[i].ammeter_style = AM_UNKNOWN;
					}

//					if (ammeter_info[i].ammeter_style != AM_UNKNOWN)
//						get_mc_from_645(i);
				}
//				rt_thread_delay(get_ticks_of_ms(200));
			}
		}
	}

END:
	READ_AMM_FLAG = 1;

	return ret;
}
#endif

#if 0
/**********************************************************************************
* Function	 : 	static rt_err_t get_flash_amm_addr(void)
* Description: 	读取Flash中存储的电表的表号，并存储到struct Ammeter_Index ammeter_info中
* Arguments	 : 	void
* Return	 : 	RT_EOK
*************************************************************************************/
static rt_err_t get_flash_amm_addr(void)
{
	int i = 0;
	rt_uint8_t addr[6] = {'\0'};
	struct electric_meter_reg_info_st *amm_sn;

	amm_sn = rt_malloc(sizeof(*amm_sn));
	if (NULL == amm_sn) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	rt_memset(&ammeter_info, 0x00, sizeof(struct Ammeter_Node));

	if (SUCC == get_em_reg_info(amm_sn)) {
		for (i = 0; i < NUM_OF_COLLECT_EM_MAX; i++) {
			if (str_sn_change_to_bcd_sn((rt_uint8_t *)(amm_sn->em_sn[i]), addr) == RT_EOK) {
				rt_memcpy(ammeter_info[i].ammeter_addr, addr, 6);
			} else {
				ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
				goto END_ERROR;
			}
		}
	} else {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		goto END_ERROR;
	}

	rt_free(amm_sn);

	return RT_EOK;

END_ERROR:
	rt_free(amm_sn);

	return RT_ERROR;
}
#endif

/**********************************************************************************
* Function	 : 	static rt_err_t get_save_ammeter_protocol(rt_uint8_t *input_addr, enum ammeter_protocal_e *output_protocal, enum ammeter_style *output_style)
* Description: 	根据输入的BCD码格式的地址，获得电表所用的规约和电表类型
* Arguments	 : 	（1）addr：输入的电表的地址；（2）output_protocal：输出电表所用的规约；（3）output_style：输出电表的类型
* Return	 :	rt_err_t
*************************************************************************************/
static rt_err_t get_save_ammeter_protocol(rt_uint8_t *input_addr, enum ammeter_protocal_e *output_protocal, enum ammeter_style *output_style)
{
	rt_uint32_t i = 0;

	if (input_addr == NULL || output_protocal == NULL || output_style == NULL) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return AP_PROTOCOL_UNKNOWN;
	}

	for (i = 0; i < NUM_OF_COLLECT_EM_MAX; i++) {
		if (rt_memcmp(input_addr, ammeter_info[i].ammeter_addr, 6) == 0) {
			*output_protocal = ammeter_info[i].ammeter_protocal;
			*output_style = ammeter_info[i].ammeter_style;
			return RT_EOK;
		}
	}

	return RT_ERROR;
}
/**********************************************************************************
* Description: 	根据输入的BCD码格式的地址，获得电表485的波特率
* Arguments	 : 	（1）addr：输入的电表的地址；
* Return	 :	enum ammeter_baud_cmd：传出电表485的波特率
*************************************************************************************/
rt_err_t get_save_ammeter_baud(rt_uint8_t *addr, struct uart_485_param *uart_485_data)
{
	rt_uint32_t i = 0;

	if (addr == NULL || uart_485_data == NULL) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	for (i = 0; i < NUM_OF_COLLECT_EM_MAX; i++) {
		if (ammeter_info[i].ammeter_protocal != AP_PROTOCOL_UNKNOWN && ammeter_info[i].ammeter_protocal != AP_PROTOCOL_NOTHING) {
			if (rt_memcmp(addr, ammeter_info[i].ammeter_addr, 6) == 0) {
				rt_memcpy(uart_485_data, &(ammeter_info[i].ammeter_baud), sizeof(struct uart_485_param));
				return RT_EOK;
			}
		}
	}

	printf_syn("NOTE:%s(), get em(%02x%02x%02x%02x%02x%02x) baud error\n", __func__,
			addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
	return RT_ERROR;
}

/**********************************************************************************
* Function	 : 	enum frame_error_e get_power_data_from_ammeter(rt_uint8_t *str_addr, enum ammeter_cmd_e cmd, rt_uint8_t *output_data, rt_uint32_t *output_data_len, enum ammeter_uart_e port)
* Description: 	获取电表的电能量数据
* Arguments	 : 	(1)str_addr:输入字符串类型的电表地址；（2）cmd：输入命令；（3）output_data：输出数据；
* 				(4)output_data_len:输出数据长度；（5）port：发送数据的端口
* Return	 : 	enum frame_error_e
*************************************************************************************/
enum frame_error_e get_power_data_from_ammeter(rt_uint8_t *str_addr, enum ammeter_cmd_e cmd, rt_uint8_t *output_data, rt_uint32_t *output_data_len, enum ammeter_uart_e port)
{
	int i = 0;
	enum frame_error_e result = FRAME_E_ERROR;
	rt_uint8_t bcd_addr[6] = {'\0'};

	if (str_addr == NULL || output_data == NULL || output_data_len == NULL) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	for (i = 0; i < 30; i++) {
		if (READ_AMM_FLAG == 0) {
			rt_thread_delay(100);
		} else {
			break;
		}
	}

	if ((rt_memcmp(str_addr, "000000000000", 12) <= 0) || (rt_memcmp(str_addr, "999999999999", 12) > 0)){
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	if (str_sn_change_to_bcd_sn(str_addr, bcd_addr) != RT_EOK) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	result = read_ammeter_power_data(bcd_addr, cmd, output_data, output_data_len, port);
	if (result != FRAME_E_OK) {
		ERROR_PRINTF(("ERR: func:%s, sn = %s, cmd = %d\n", __FUNCTION__, str_addr, cmd));
		return result;
	}
#if 0
	meterp_debug_main(("SUCC: get_power_data_from_ammeter: sn = %s, cmd = %d\n", str_addr, cmd));
	meterp_debug_main(("len = %d, buf = ", *output_data_len));
	for (i= 0; i < *output_data_len; i++) {
		meterp_debug_main((" %02x", output_data[i]));
	}
	meterp_debug_main(("\n"));
#endif
	return FRAME_E_OK;
}
/**********************************************************************************
* Function	 : 	enum frame_error_e get_maxneed_data_from_ammeter(rt_uint8_t *str_addr, enum ammeter_maxneed_cmd cmd, rt_uint8_t *output_data, rt_uint32_t *output_data_len, enum ammeter_uart_e port)
* Description: 	获取电表的最大需量数据
* Arguments	 : 	(1)str_addr:输入字符串类型的电表地址；（2）cmd：输入命令；（3）output_data：输出数据；
* 				(4)output_data_len:输出数据长度；（5）port：发送数据的端口
* Return	 : 	enum frame_error_e
*************************************************************************************/
enum frame_error_e get_maxneed_data_from_ammeter(rt_uint8_t *str_addr, enum ammeter_maxneed_cmd cmd, rt_uint8_t *output_data, rt_uint32_t *output_data_len, enum ammeter_uart_e port)
{
	int i = 0;
	enum frame_error_e result = FRAME_E_ERROR;
	rt_uint8_t bcd_addr[6] = {'\0'};

	if (str_addr == NULL || output_data == NULL || output_data_len == NULL) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	for (i = 0; i < 30; i++) {
		if (READ_AMM_FLAG == 0) {
			rt_thread_delay(100);
		} else {
			break;
		}
	}

	if ((rt_memcmp(str_addr, "000000000000", 12) <= 0) || (rt_memcmp(str_addr, "999999999999", 12) > 0)){
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	if (str_sn_change_to_bcd_sn(str_addr, bcd_addr) != RT_EOK) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	result = read_ammeter_maxneed_data(bcd_addr, cmd, output_data, output_data_len, port);
	if (result != FRAME_E_OK) {
		ERROR_PRINTF(("ERR: func:%s, sn = %s, cmd = %d\n", __FUNCTION__, str_addr, cmd));
		return result;
	}
#if 0
	meterp_debug_main(("get_maxneed_data_from_ammeter: sn = %s, cmd = %d\n", str_addr, cmd));
	meterp_debug_main(("len = %d, buf = ", *output_data_len));
	for (i= 0; i < *output_data_len; i++) {
		meterp_debug_main((" %02x", output_data[i]));
	}
	meterp_debug_main(("\n"));
#endif
	return FRAME_E_OK;
}
/**********************************************************************************
* Function	 : 	enum frame_error_e get_forzen_data_from_ammeter(rt_uint8_t *str_addr, enum ammeter_forzen_cmd cmd, rt_uint8_t forzen_times, rt_uint8_t *output_data, rt_uint32_t *output_data_len, enum ammeter_uart_e port)
* Description: 	获取电表的冻结数据
* Arguments	 : 	(1)str_addr:输入字符串类型的电表地址；2）cmd：输入命令；（3）forzen_times：读取第几次的冻结数据（4）output_data：输出数据；
* 				(5)output_data_len:输出数据长度；（6）port：发送数据的端口
* Return	 : 	enum frame_error_e
*************************************************************************************/
enum frame_error_e get_forzen_data_from_ammeter(rt_uint8_t *str_addr, enum ammeter_forzen_cmd cmd, rt_uint8_t forzen_times, rt_uint8_t *output_data, rt_uint32_t *output_data_len, enum ammeter_uart_e port)
{
	int i = 0;
	enum frame_error_e result = FRAME_E_ERROR;
	rt_uint8_t bcd_addr[6] = {'\0'};

	if (str_addr == NULL || output_data == NULL || output_data_len == NULL) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	for (i = 0; i < 30; i++) {
		if (READ_AMM_FLAG == 0) {
			rt_thread_delay(100);
		} else {
			break;
		}
	}

	if ((rt_memcmp(str_addr, "000000000000", 12) <= 0) || (rt_memcmp(str_addr, "999999999999", 12) > 0)){
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	if (str_sn_change_to_bcd_sn(str_addr, bcd_addr) != RT_EOK) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	result = read_ammeter_forzen_data(bcd_addr, cmd, forzen_times, output_data, output_data_len, port);
	if (result != FRAME_E_OK) {
		ERROR_PRINTF(("ERR: func:%s, sn = %s, cmd = %d\n", __FUNCTION__, str_addr, cmd));
		return result;
	}
#if 0
	meterp_debug_main(("get_forzen_data_from_ammeter: sn = %s, cmd = %d\n", str_addr, cmd));
	meterp_debug_main(("len = %d, buf = ", *output_data_len));
	for (i= 0; i < *output_data_len; i++) {
		meterp_debug_main((" %02x", output_data[i]));
	}
	meterp_debug_main(("\n"));
#endif
	return FRAME_E_OK;
}

/**********************************************************************************
* Function	 : 	enum frame_error_e get_event_data_from_ammeter(rt_uint8_t *str_addr, enum ammeter_event_cmd cmd, rt_uint8_t *output_data, rt_uint32_t *output_data_len, enum ammeter_uart_e port)
* Description: 	获取电表的事件数据
* Arguments	 : 	(1)str_addr:输入字符串类型的电表地址；（（2）cmd：输入命令;（3）event_times：读取第几次的事件（4）output_data：输出数据；
* 				(5)output_data_len:输出数据长度；（6）port：发送数据的端口
* Return	 : 	enum frame_error_e
*************************************************************************************/
enum frame_error_e get_event_data_from_ammeter(rt_uint8_t *str_addr, enum ammeter_event_cmd cmd, rt_uint8_t event_times, rt_uint8_t *output_data, rt_uint32_t *output_data_len, enum ammeter_uart_e port)
{
	int i = 0;
	enum frame_error_e result = FRAME_E_ERROR;
	rt_uint8_t bcd_addr[6] = {'\0'};

	if (str_addr == NULL || output_data == NULL || output_data_len == NULL) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	for (i = 0; i < 30; i++) {
		if (READ_AMM_FLAG == 0) {
			rt_thread_delay(100);
		} else {
			break;
		}
	}

	if ((rt_memcmp(str_addr, "000000000000", 12) <= 0) || (rt_memcmp(str_addr, "999999999999", 12) > 0)){
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	if (str_sn_change_to_bcd_sn(str_addr, bcd_addr) != RT_EOK) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	result = read_ammeter_event_data(bcd_addr, cmd, event_times, output_data, output_data_len, port);
	if (result != FRAME_E_OK) {
		ERROR_PRINTF(("ERR: func:%s, sn = %s, cmd = %d\n", __FUNCTION__, str_addr, cmd));
		return result;
	}
#if 0
	meterp_debug_main(("get_event_data_from_ammeter: sn = %s, cmd = %d\n", str_addr, cmd));
	meterp_debug_main(("len = %d, buf = ", *output_data_len));
	for (i= 0; i < *output_data_len; i++) {
		meterp_debug_main((" %02x", output_data[i]));
	}
	meterp_debug_main(("\n"));
#endif
	return FRAME_E_OK;
}
/**********************************************************************************
* Function	 : 	rt_err_t setting_all_ammeter_time(struct ammeter_time *time, enum ammeter_uart_e port)
* Description: 	广播设置电表时间，
* 				1.广播校时不要求应答。
*				2.仅当从站的日历和时钟与主站的时差在±5min 以内时执行校时命令，即将从站的日
*				历时钟调整到与命令下达的日历时钟一致。
*				3.不能在午夜0 时和23时校时，以免影响在0 时进行的某些例行操作。
*				4.每天只允许校对一次。
* Arguments	 : 	(1)time:输入校时的时间；（2）port：电表多连接的端口；
* Return	 : 	RT_EOK
*************************************************************************************/
rt_err_t setting_all_ammeter_time(struct ammeter_time *time, enum ammeter_uart_e port)
{
	rt_uint8_t i = 0;
	rt_uint8_t time_buf[6] = {'\0'};
	rt_uint8_t recv_buf[8] = {'\0'};
	rt_uint8_t broadcast_addr[6] = {0x99, 0x99, 0x99, 0x99, 0x99, 0x99};
	enum frame_error_e result = FRAME_E_ERROR;
	struct frame645_97param send_645_97frame_data;
	struct frame645_97param recv_645_97frame_data;
	struct send_frame_param send_645_07frame_data;
	struct recv_frame_param recv_645_07frame_data;
	struct frame_edmi_param send_edmi_frame_data;
	struct frame_edmi_param recv_edmi_frame_data;

	rt_memset(&send_645_97frame_data, 0x00, sizeof(struct frame645_97param));
	rt_memset(&recv_645_97frame_data, 0x00, sizeof(struct frame645_97param));
	rt_memset(&send_645_07frame_data, 0x00, sizeof(struct send_frame_param));
	rt_memset(&recv_645_07frame_data, 0x00, sizeof(struct recv_frame_param));
	rt_memset(&send_edmi_frame_data, 0x00, sizeof(struct frame_edmi_param));
	rt_memset(&recv_edmi_frame_data, 0x00, sizeof(struct frame_edmi_param));

	if (time->year < 2014 || time->year > 2050) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	if (time->month < 0 || time->month > 12) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	if (time->day < 0 || time->day > 31) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	if (time->hour <= 0 || time->hour >= 23) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	if (time->minite < 0 || time->minite >= 60) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	if (time->seconds < 0 || time->seconds >= 60) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	time_buf[0] = (time->seconds%10)| ((time->seconds/10)<<4);
	time_buf[1] = (time->minite%10)| ((time->minite/10)<<4);
	time_buf[2] = (time->hour%10)| ((time->hour/10)<<4);
	time_buf[3] = (time->day%10)| ((time->day/10)<<4);
	time_buf[4] = (time->month%10)| ((time->month/10)<<4);
	time_buf[5] = (time->year%10)| (((time->year%100)/10)<<4);

	rt_memcpy(send_645_97frame_data.addr, broadcast_addr, 6);
	send_645_97frame_data.ctrl = F645_97CMD_CHECK_TIME ;
	send_645_97frame_data.data = time_buf;

	result = transmit_ammeter645_97_data(&send_645_97frame_data, &recv_645_97frame_data, port);
	if (result != FRAME_E_OK) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	rt_memcpy(send_645_07frame_data.addr, broadcast_addr, 6);
	send_645_07frame_data.ctrl = F645_07CMD_BROADCASTING_TIME ;
	send_645_07frame_data.data = time_buf;

	result = transmit_ammeter645_07_data(&send_645_07frame_data, &recv_645_07frame_data, port);
	if (result != FRAME_E_OK) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	time_buf[0] = time->day;
	time_buf[1] = time->month;
	time_buf[2] = time->year%100;
	time_buf[3] = time->hour;
	time_buf[4] = time->minite;
	time_buf[5] = time->seconds;

	for (i = 0; i < NUM_OF_COLLECT_EM_MAX; i++) {
		if (ammeter_info[i].ammeter_protocal == AP_PROTOCOL_EDMI) {
			rt_memcpy(send_edmi_frame_data.dest_addr, ammeter_info[i].ammeter_addr, 6);
			send_edmi_frame_data.ctrl = EDMI_CMD_WRITE_EXTEND_REGISTER ;
			send_edmi_frame_data.data = time_buf;
			send_edmi_frame_data.data_len = 6;
			send_edmi_frame_data.data_flag = 0xF03D;
			recv_edmi_frame_data.data = recv_buf;

			result = transmit_ammeter_edmi_data(&send_edmi_frame_data, &recv_edmi_frame_data, port);
			if (result != FRAME_E_OK) {
				ERROR_PRINTF(("ERROR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
				return RT_ERROR;
			}
		}
	}

	return RT_EOK;
}
/**********************************************************************************
* Function	 : 	enum frame_error_e setting_ammeter_forzen_data_time(rt_uint8_t *str_addr, enum ammeter_forzen_time_cmd cmd, struct ammeter_time *time, enum ammeter_uart_e port)
* Description: 	设置电表的定时冻结时间
* Arguments	 : 	(1)str_addr:输入字符串类型的电表地址；(2)cmd:输入定时冻结的周期；（3）time：输入定时冻结的时间；（4）port：连接电表的端口
* Return	 : 	RT_EOK
*************************************************************************************/
enum frame_error_e setting_ammeter_forzen_data_time(rt_uint8_t *str_addr, enum ammeter_forzen_time_cmd cmd, struct ammeter_time *time, enum ammeter_uart_e port)
{
	rt_uint8_t time_buf[6] = {'\0'};
	rt_uint8_t bcd_addr[6] = {'\0'};
	rt_uint8_t buf[4] = {'\0'};
	enum ammeter_protocal_e protocal = AP_PROTOCOL_UNKNOWN;
	enum ammeter_style style = AM_UNKNOWN;
	struct send_frame_param send_645_07frame_data;
	struct recv_frame_param recv_645_07frame_data;

	if (str_addr == NULL) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	if ((rt_memcmp(str_addr, "000000000000", 12) <= 0) || (rt_memcmp(str_addr, "999999999999", 12) > 0)){
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	if (str_sn_change_to_bcd_sn(str_addr, bcd_addr) != RT_EOK) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	if (get_save_ammeter_protocol(bcd_addr, &protocal, &style) == RT_EOK) {
		if (protocal != AP_PROTOCOL_645_2007) {
			ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
			return FRAME_E_ERROR;
		}
	} else {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	if (cmd == CMD_FREEZING_NOW) { /* 瞬时冻结 */
		time_buf[0] = 0x99;
		time_buf[1] = 0x99;
		time_buf[2] = 0x99;
		time_buf[3] = 0x99;
	} else {
		if (time->minite < 0 || time->minite >= 60) {
			ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
			return FRAME_E_ERROR;
		} else {
			time_buf[0] = (time->minite%10)| ((time->minite/10)<<4);
		}

		if (cmd == CMD_FREEZING_HOUR) { /* 以小时为周期定时冻结 */
			time_buf[1] = 0x99;
			time_buf[2] = 0x99;
			time_buf[3] = 0x99;
		} else {
			if (time->hour < 0 || time->hour >= 24) {
				ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
				return FRAME_E_ERROR;
			} else {
				time_buf[1] = (time->hour%10)| ((time->hour/10)<<4);
			}

			if (cmd == CMD_FREEZING_DAY) { /* 以日为周期定时冻结 */
				time_buf[2] = 0x99;
				time_buf[3] = 0x99;
			} else {
				if (time->day < 0 || time->day > 31) {
					ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
					return FRAME_E_ERROR;
				} else {
					time_buf[2] = (time->day%10)| ((time->day/10)<<4);
				}

				if (cmd == CMD_FREEZING_MONTH) { /* 以月为周期定时冻结 */
					time_buf[3] = 0x99;
				} else {
					ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
					return FRAME_E_ERROR;
				}
			}
		}
	}

	rt_memset(&send_645_07frame_data, 0x00, sizeof(struct send_frame_param));
	rt_memset(&recv_645_07frame_data, 0x00, sizeof(struct recv_frame_param));

	rt_memcpy(send_645_07frame_data.addr, bcd_addr, 6);
	send_645_07frame_data.ctrl = F645_07CMD_FREEZING_CMD;
	send_645_07frame_data.data = time_buf;
	recv_645_07frame_data.data = buf;

	if (FRAME_E_OK != transmit_ammeter645_07_data(&send_645_07frame_data, &recv_645_07frame_data, port)) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	return FRAME_E_OK;
}
/**********************************************************************************
* Function	 : 	enum frame_error_e reported_ammeter_happened_event(rt_uint8_t *str_addr, enum ammeter_uart_e port)
* Description: 	判断事件是否发生,并上报发生的事件
* Arguments	 : 	(1)str_addr:字符串格式的电表地址;(2)port：电表连接的端口；
* Return	 : 	enum frame_error_e
*************************************************************************************/
enum frame_error_e reported_ammeter_happened_event(rt_uint8_t *str_addr, enum ammeter_uart_e port)
{
	rt_uint8_t i = 0;
	rt_uint8_t bcd_addr[6] = {'\0'};
	enum frame_error_e result = FRAME_E_ERROR;
	enum ammeter_protocal_e ammeter_protocal = AP_PROTOCOL_UNKNOWN;

	if ((rt_memcmp(str_addr, "000000000000", 12) <= 0) || (rt_memcmp(str_addr, "999999999999", 12) > 0)){
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	if (str_sn_change_to_bcd_sn(str_addr, bcd_addr) != RT_EOK) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	for (i = 0; i < NUM_OF_COLLECT_EM_MAX; i++) {
		if (rt_memcmp(bcd_addr, ammeter_info[i].ammeter_addr, 6) == 0) {
			ammeter_protocal = ammeter_info[i].ammeter_protocal;
			break;
		}
	}

	if (ammeter_protocal == AP_PROTOCOL_645_2007) {
		result = read_ammeter645_07_event_data(bcd_addr, i, port);
		if (result != FRAME_E_OK) {
			ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
			return result;
		}
	}

	return FRAME_E_OK;
}
/**********************************************************************************
* Function	 : 	static enum frame_error_e read_ammeter_power_data(rt_uint8_t *addr, enum ammeter_cmd_e cmd, rt_uint8_t *output_data, rt_uint32_t *output_data_len, enum ammeter_uart_e port)
* Description: 	读出电表的电能量数据
* Arguments	 : 	(1)addr:输入电表地址；（2）cmd：输入命令；（3）output_data：输出数据；
* 				(4)output_data_len:输出数据长度；（5）port：发送数据的端口
* Return	 : 	enum frame_error_e
*************************************************************************************/
static enum frame_error_e read_ammeter_power_data(rt_uint8_t *addr, enum ammeter_cmd_e cmd, rt_uint8_t *output_data, rt_uint32_t *output_data_len, enum ammeter_uart_e port)
{
	rt_uint32_t i = 0, len = 0;
	rt_uint8_t buf[64] = {'\0'};
	rt_uint32_t data_flag = 0;
	rt_int32_t decimals_len = 0;
	rt_uint32_t move_len = 0;
	rt_uint32_t tmp = 0;
	enum ammeter_style style = AM_UNKNOWN;
	enum data_type data_type = DATA_TYPE_UNKNOWN;
	enum frame_error_e result = FRAME_E_ERROR;
	enum ammeter_protocal_e ammeter_protocal = AP_PROTOCOL_UNKNOWN;

	if (addr == NULL || output_data == NULL || output_data_len == NULL) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	if (get_save_ammeter_protocol(addr, &ammeter_protocal, &style) != RT_EOK) {
		ERROR_PRINTF(("ERR: func:%s, line(%d), get_save_ammeter_protocol err\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	if (ammeter_protocal == AP_PROTOCOL_645_1997) {
		if (output645_97power_data_flag(cmd, &data_flag, &move_len) != RT_EOK) {
				ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
				return FRAME_E_ERROR;
		}
		result = get_data_from_645_97(addr, data_flag, buf, &len, port);
		if (result == FRAME_E_OK) {
			if (len <= 3 && move_len != 0) {
				rt_memcpy(&tmp, buf, len);
				tmp = tmp << (4 * move_len);
				*output_data_len = len + (move_len/2 + move_len%2);
				rt_memcpy(output_data, &tmp, *output_data_len);
			} else {
				*output_data_len = len;
				rt_memcpy(output_data, buf, len);
			}
		} else {
			ERROR_PRINTF(("ERR: func:%s, line(%d), err(%d)\n", __FUNCTION__, __LINE__,
					result));
			return result;
		}
	} else if (ammeter_protocal == AP_PROTOCOL_645_2007){
		if (output645_07power_data_flag(cmd, &data_flag) != RT_EOK) {
			ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
			return FRAME_E_ERROR;
		}
		result = get_data_from_645_07(addr, data_flag, output_data, output_data_len, port);
		if (result != FRAME_E_OK) {
			ERROR_PRINTF(("ERR: func:%s, line(%d), err(%d)\n", __FUNCTION__, __LINE__,
					result));
			return result;
		}
	} else if (ammeter_protocal == AP_PROTOCOL_EDMI){
		if (output_edmi_data_flag(style, cmd, &data_flag, &data_type, &decimals_len) != RT_EOK) {
			ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
			return FRAME_E_ERROR;
		}

		result = get_data_from_edmi(addr, data_flag, buf, &len, port);
		if (result == FRAME_E_OK) {
			if (data_type == DATA_TYPE_SINGLE_FLOAT) {
				if (RT_EOK != float_change_to_bcd_hex(cmd, buf, len, decimals_len, output_data, output_data_len)) {
					ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
					return FRAME_E_ERROR;
				}
			} else if (data_type == DATA_TYPE_STRING) {
				if (RT_EOK != string_change_to_bcd_hex(buf, len, output_data, output_data_len)) {
					ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
					return FRAME_E_ERROR;
				}
			} else if (data_type == DATA_TYPE_INTEGER) {
				if (RT_EOK != integer_change_to_bcd_hex(buf, len, output_data, output_data_len)) {
					ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
					return FRAME_E_ERROR;
				}
			} else if (data_type == DATA_TYPE_DATE){
				*output_data = 0x00;
				for (i = 0; i < len; i++) {
					*(output_data + i + 1) = *(buf + i) % 10;
					*(output_data + i + 1) |= ((*(buf + i) / 10) % 10) << 4;
				}
				*output_data_len = len + 1;
			} else if (data_type == DATA_TYPE_TIME){
				for (i = 0; i < len; i++) {
					*(output_data + len - 1 - i) = *(buf + i) % 10;
					*(output_data + len - 1 - i) |= ((*(buf + i) / 10) % 10) << 4;
				}
				*output_data_len = len;
			} else {
				ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
				return FRAME_E_ERROR;
			}
		} else {
			ERROR_PRINTF(("ERR: func:%s, line(%d), err(%d)\n", __FUNCTION__, __LINE__,
					result));
			return result;
		}
	} else {
		ERROR_PRINTF(("ERR: func:%s, line(%d), ammeter_protocal:%d\n", __FUNCTION__,
				__LINE__, ammeter_protocal));
		return FRAME_E_ERROR;
	}

	return FRAME_E_OK;
}
/**********************************************************************************
* Function	 : 	static enum frame_error_e read_ammeter_forzen_data(rt_uint8_t *addr, enum ammeter_forzen_cmd cmd, rt_uint8_t forzen_times, rt_uint8_t *output_data,
* 				rt_uint32_t *output_data_len, enum ammeter_uart_e port)
* Description: 	读出电表的冻结数据
* Arguments	 : 	(1)addr:输入电表地址；（2）cmd：输入命令；（3）forzen_times：读取第几次的冻结数据（4）output_data：输出数据；
* 				(5)output_data_len:输出数据长度；（6）port：发送数据的端口
* Return	 : 	RT_EOK
*************************************************************************************/
static enum frame_error_e read_ammeter_forzen_data(rt_uint8_t *addr, enum ammeter_forzen_cmd cmd, rt_uint8_t forzen_times, rt_uint8_t *output_data, rt_uint32_t *output_data_len, enum ammeter_uart_e port)
{
	rt_uint32_t len = 0;
	rt_uint32_t data_len = 0;
	rt_uint32_t data_flag = 0;
	enum frame_error_e result = FRAME_E_ERROR;
	enum ammeter_style style = AM_UNKNOWN;
	enum ammeter_protocal_e ammeter_protocal = AP_PROTOCOL_UNKNOWN;

	if (addr == NULL || output_data == NULL || output_data_len == NULL) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	if (forzen_times < 1 || forzen_times > 12) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	if (get_save_ammeter_protocol(addr, &ammeter_protocal, &style) != RT_EOK) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	if (ammeter_protocal == AP_PROTOCOL_645_1997) {
		return FRAME_E_ERROR;
	} else if (ammeter_protocal == AP_PROTOCOL_645_2007) {
		if (output645_07forzen_data_flag(cmd, &data_flag, &data_len) != RT_EOK) {
			ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
			return FRAME_E_ERROR;
		}

		data_flag = data_flag + forzen_times - 1;
		result = get_data_from_645_07(addr, data_flag, output_data, &len, port);
		if (result == FRAME_E_OK) {
			if (data_len > len) {
				ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
				return FRAME_E_ERROR;
			} else {
				*output_data_len = data_len;
			}
		} else {
			ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
			return result;
		}
	} else {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	return FRAME_E_OK;
}
/**********************************************************************************
* Function	 : 	static enum frame_error_e read_ammeter_maxneed_data(rt_uint8_t *addr, enum ammeter_forzen_cmd cmd, rt_uint8_t *output_data,
* 				rt_uint32_t *output_data_len, enum ammeter_uart_e port)
* Description: 	读出电表的最大需量数据
* Arguments	 : 	(1)addr:输入电表地址；（2）cmd：输入命令；（3）output_data：输出数据；
* 				(4)output_data_len:输出数据长度；（5）port：发送数据的端口
* Return	 : 	RT_EOK
*************************************************************************************/
static enum frame_error_e read_ammeter_maxneed_data(rt_uint8_t *addr, enum ammeter_maxneed_cmd cmd, rt_uint8_t *output_data, rt_uint32_t *output_data_len, enum ammeter_uart_e port)
{
	rt_uint32_t len = 0;
	rt_uint32_t data_len = 0;
	rt_uint32_t decimals_len = 0;
	rt_uint32_t data_flag = 0;
	rt_uint32_t maxneed_data_flag = 0;
	rt_uint32_t time_data_flag = 0;
	rt_uint8_t buf[8] = {'\0'};
	enum ammeter_style style = AM_UNKNOWN;
	enum frame_error_e result = FRAME_E_ERROR;
	enum ammeter_protocal_e ammeter_protocal = AP_PROTOCOL_UNKNOWN;

	if (addr == NULL || output_data == NULL || output_data_len == NULL) {
		ERROR_PRINTF(("Ert_uint32_tR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	if (get_save_ammeter_protocol(addr, &ammeter_protocal, &style) != RT_EOK) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	if (ammeter_protocal == AP_PROTOCOL_645_1997) {
		if (output645_97maxneed_data_flag(cmd, &maxneed_data_flag, &time_data_flag) != RT_EOK) {
			ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
			return FRAME_E_ERROR;
		}

		result = get_data_from_645_97(addr, maxneed_data_flag, buf, &len, port);
		if (result == FRAME_E_OK) {
			rt_memcpy(output_data, buf, 3);
		} else {
			ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
			return result;
		}

		rt_memset(buf, 0, sizeof(buf));
		result = get_data_from_645_97(addr, time_data_flag, buf, &len, port);
		if (result == FRAME_E_OK) {
			rt_memcpy(output_data+4, buf, 4);
			*output_data_len = 8;
		} else {
			ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
			return result;
		}
	} else if (ammeter_protocal == AP_PROTOCOL_645_2007){
		if (output645_07maxneed_data_flag(cmd, &data_flag) != RT_EOK) {
			ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
			return FRAME_E_ERROR;
		}
		result = get_data_from_645_07(addr, data_flag, output_data, output_data_len, port);
		if (result != FRAME_E_OK) {
			ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
			return result;
		}
	} else if (ammeter_protocal == AP_PROTOCOL_EDMI) {
		if (output_edmi_maxneed_data_flag(cmd, &maxneed_data_flag, &decimals_len, &time_data_flag) != RT_EOK) {
			ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
			return FRAME_E_ERROR;
		}

		result = get_data_from_edmi(addr, maxneed_data_flag, buf, &len, port);
		if (result == FRAME_E_OK) {
			if (RT_EOK != float_change_to_bcd_hex(cmd, buf, len, decimals_len, output_data, &data_len)) {
				ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
				return FRAME_E_ERROR;
			}
		} else {
			ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
			return result;
		}

		data_len = 3;
		rt_memset(buf, 0, sizeof(buf));
		result = get_data_from_edmi(addr, time_data_flag, buf, &len, port);
		if (result == FRAME_E_OK) {
			if (RT_EOK != integer_change_to_bcd_hex(buf, len, output_data + data_len, output_data_len)) {
				ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
				return FRAME_E_ERROR;
			}
			*output_data_len += data_len;
		} else {
			ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
			return result;
		}
	} else {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	return FRAME_E_OK;
}
/**********************************************************************************
* Function	 : 	static enum frame_error_e read_ammeter_event_data(rt_uint8_t *addr, enum ammeter_event_cmd cmd, rt_uint8_t event_times, rt_uint8_t *output_data,
* 				rt_uint32_t *output_data_len, enum ammeter_uart_e port)
* Description: 	读出电表的事件数据
* Arguments	 : 	(1)addr:输入电表地址；（2）cmd：输入命令;（3）event_times：读取第几次的事件（4）output_data：输出数据；
* 				(5)output_data_len:输出数据长度；（6）port：发送数据的端口
* Return	 : 	RT_EOK
*************************************************************************************/
static enum frame_error_e read_ammeter_event_data(rt_uint8_t *addr, enum ammeter_event_cmd cmd, rt_uint8_t event_times, rt_uint8_t *output_data, rt_uint32_t *output_data_len, enum ammeter_uart_e port)
{
	rt_uint32_t len = 0;
	rt_uint32_t data_flag = 0;
	rt_uint8_t buf[256] = {'\0'};
	enum ammeter_style style = AM_UNKNOWN;
	enum frame_error_e result = FRAME_E_ERROR;
	enum ammeter_protocal_e ammeter_protocal = AP_PROTOCOL_UNKNOWN;
	enum ammeter_event_cmd upload_cmd = AMM_EVENT_NOTHING;

	if (addr == NULL || output_data == NULL || output_data_len == NULL) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	if (event_times < 1 || event_times > 10) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	if (get_save_ammeter_protocol(addr, &ammeter_protocal, &style) != RT_EOK) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	if (ammeter_protocal == AP_PROTOCOL_645_1997) {
		return FRAME_E_ERROR;
	} else if (ammeter_protocal == AP_PROTOCOL_645_2007) {
		if (output645_07event_data_flag(cmd, &data_flag) != RT_EOK) {
			ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
			return FRAME_E_ERROR;
		}
		if (cmd==AMM_EVENT_LOSE_VOLTAGE||cmd==AMM_EVENT_OWE_VOLTAGE||cmd==AMM_EVENT_OVER_VOLTAGE||cmd==AMM_EVENT_BROKEN_PHASE||cmd==AMM_EVENT_LOSE_CURRENT||
				cmd==AMM_EVENT_OVER_CURRENT||cmd==AMM_EVENT_BROKEN_CURRENT||cmd==AMM_EVENT_AMM_RESET||cmd==AMM_EVENT_REQUIRED_RESET||cmd==AMM_EVENT_CALIBRATION_TIME
				||cmd==AMM_EVENT_PROGRAMMING||cmd==AMM_EVENT_VOLTAGE_INVERSE_PHASE||cmd==AMM_EVENT_CURRENT_INVERSE_PHASE) {

			result = get_data_from_645_07(addr, data_flag, output_data, output_data_len, port);
			if (result == FRAME_E_OK) {
				return FRAME_E_OK;
			} else {
				ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
				return result;
			}
		}

		data_flag = data_flag + event_times;
		result = get_data_from_645_07(addr, data_flag, buf, &len, port);
		if (result == FRAME_E_OK) {
			if (RT_EOK != output645_07event_cmd_data_len(data_flag, buf, len, &upload_cmd, output_data, output_data_len)) {
				ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
				return FRAME_E_ERROR;
			}
		} else {
			ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
			return result;
		}
	} else {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	return FRAME_E_OK;
}

/**********************************************************************************
* Function	 : 	static rt_err_t str_sn_change_to_bcd_sn(rt_uint8_t *input_str_sn, rt_uint8_t *output_bcd_sn)
* Description: 	将字符串形式的SN号转化为BCD码形式的SN号
* Arguments	 : 	(1)input_str_sn：输入存储SN号的数组；（2）output_bcd_sn：输出BCD码格式的SN号
* Return	 : 	RT_EOK
*************************************************************************************/
static rt_err_t str_sn_change_to_bcd_sn(rt_uint8_t *input_str_sn, rt_uint8_t *output_bcd_sn)
{
	rt_uint32_t i = 0, count = 0;
	rt_uint8_t str[2] = {'\0'};
	rt_uint8_t addr_str[12] = {'\0'};

	if (input_str_sn == NULL || output_bcd_sn == NULL) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	count = rt_strlen((char *)input_str_sn);
	if (count < 12) {
		rt_memcpy(addr_str + 12 - count, input_str_sn, count);
	} else {
		rt_memcpy(addr_str, input_str_sn, 12);
	}

	for (i = 0; i < 6; i++) {
		str[0] = addr_str[i*2];
		str[1] = addr_str[i*2+1];

		if (str[0] >= 0x30)
			str[0] = str[0] - 0x30;
		if (str[1] >= 0x30)
			str[1] = str[1] - 0x30;

		output_bcd_sn[i] = str[0]<<4 | str[1];
	}

	return RT_EOK;
}

/**********************************************************************************
* Function	 : 	static enum frame_error_e get_data_from_645_97(rt_uint8_t *addr, rt_uint32_t data_flag, rt_uint8_t *output_data, rt_uint32_t *output_data_len, enum ammeter_uart_e port)
* Description: 	读取645-1997规约电表的数据
* Arguments	 : 	（1）addr：输入的电表的地址；（2）data_flag：输入的数据标识；（3）output_data：传出的数据；（4）output_data_len：传出的数据长度；（5）port：连接电表所用的端口
* Return	 :	enum frame_error_e
*************************************************************************************/
enum frame_error_e get_data_from_645_97(rt_uint8_t *addr, rt_uint32_t data_flag, rt_uint8_t *output_data, rt_uint32_t *output_data_len, enum ammeter_uart_e port)
{
	enum frame_error_e result = FRAME_E_ERROR;
	struct frame645_97param send_645_97frame_data;
	struct frame645_97param recv_645_97frame_data;

	if (addr == NULL || output_data == NULL || output_data_len == NULL) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	rt_memset(&send_645_97frame_data, 0x00, sizeof(struct frame645_97param));
	rt_memset(&recv_645_97frame_data, 0x00, sizeof(struct frame645_97param));

	rt_memcpy(send_645_97frame_data.addr, addr, 6);
	send_645_97frame_data.ctrl = F645_97CMD_READ_DATA;

	send_645_97frame_data.data_flag = data_flag;
	recv_645_97frame_data.data = output_data;

	result = transmit_ammeter645_97_data(&send_645_97frame_data, &recv_645_97frame_data, port);
	if (result == FRAME_E_OK) {
		*output_data_len = recv_645_97frame_data.data_len;
	} else {
	//	ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return result;
	}

	return FRAME_E_OK;
}
/**********************************************************************************
* Function	 : 	rt_err_t output645_97power_data_flag(enum ammeter_cmd_e cmd, rt_uint32_t *output_data_flag, rt_uint32_t *output_move_len)
* Description: 	根据输入的读取电能量的命令，输出645_97规约对应的数据标识
* Arguments	 : 	（1）cmd：输入的读表命令；（2）output_data_flag：输出645_97规约相应的数据类型; (3)output_move_len:与645-2007相比少几位小数
* Return	 :	RT_EOK
*************************************************************************************/
rt_err_t output645_97power_data_flag(enum ammeter_cmd_e cmd, rt_uint32_t *output_data_flag, rt_uint32_t *output_move_len)
{
	if (output_data_flag == NULL) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	switch (cmd) {
#if 0
	AC_COMBINATION_ACTIVE_TOTAL_POWER,		/* 组合有功总电能, 返回4bytes数据 */
	AC_COMBINATION_ACTIVE_RATE1_POWER,		/* 组合有功费率1电能, 返回4bytes数据 */
	AC_COMBINATION_ACTIVE_RATE2_POWER,		/* 组合有功费率2电能, 返回4bytes数据 */
	AC_COMBINATION_ACTIVE_RATE3_POWER,		/* 组合有功费率3电能, 返回4bytes数据 */
	AC_COMBINATION_ACTIVE_RATE4_POWER,		/* 组合有功费率4电能, 返回4bytes数据 */
#endif
	case AC_POSITIVE_ACTIVE_POWER:		/* 正向有功电能, 返回4bytes数据 */
		*output_data_flag = 0x9010;
		*output_move_len = 0;
		break;
	case AC_POSITIVE_ACTIVE_RATE1_POWER:  	/* 正向有功费率1电能, 返回4bytes数据 */
		*output_data_flag = 0x9011;
		*output_move_len = 0;
		break;
	case AC_POSITIVE_ACTIVE_RATE2_POWER:  	/* 正向有功费率2电能, 返回4bytes数据 */
		*output_data_flag = 0x9012;
		*output_move_len = 0;
		break;
	case AC_POSITIVE_ACTIVE_RATE3_POWER:  	/* 正向有功费率3电能, 返回4bytes数据 */
		*output_data_flag = 0x9013;
		*output_move_len = 0;
		break;
	case AC_POSITIVE_ACTIVE_RATE4_POWER:  	/* 正向有功费率4电能, 返回4bytes数据 */
		*output_data_flag = 0x9014;
		*output_move_len = 0;
		break;

	case AC_REPOSITIVE_ACTIVE_POWER: 	/* 反向有功电能, 返回4bytes数据 */
		*output_data_flag = 0x9020;
		*output_move_len = 0;
		break;
	case AC_REPOSITIVE_ACTIVE_RATE1_POWER: 	/* 反向有功费率1电能, 返回4bytes数据 */
		*output_data_flag = 0x9021;
		*output_move_len = 0;
		break;
	case AC_REPOSITIVE_ACTIVE_RATE2_POWER: 	/* 反向有功费率2电能, 返回4bytes数据 */
		*output_data_flag = 0x9022;
		*output_move_len = 0;
		break;
	case AC_REPOSITIVE_ACTIVE_RATE3_POWER: 	/* 反向有功费率3电能, 返回4bytes数据 */
		*output_data_flag = 0x9023;
		*output_move_len = 0;
		break;
	case AC_REPOSITIVE_ACTIVE_RATE4_POWER: 	/* 反向有功费率4电能, 返回4bytes数据 */
		*output_data_flag = 0x9024;
		*output_move_len = 0;
		break;
	case AC_POSITIVE_WATTLESS_POWER: 	/* 正向无功电能, 返回4bytes数据 */
		*output_data_flag = 0x9110;
		*output_move_len = 0;
		break;
	case AC_POSITIVE_WATTLESS_RATE1_POWER: 	/* 正向无功费率1电能, 返回4bytes数据 */
		*output_data_flag = 0x9111;
		*output_move_len = 0;
		break;
	case AC_POSITIVE_WATTLESS_RATE2_POWER: 	/* 正向无功费率2电能, 返回4bytes数据 */
		*output_data_flag = 0x9112;
		*output_move_len = 0;
		break;
	case AC_POSITIVE_WATTLESS_RATE3_POWER: 	/* 正向无功费率3电能, 返回4bytes数据 */
		*output_data_flag = 0x9113;
		*output_move_len = 0;
		break;
	case AC_POSITIVE_WATTLESS_RATE4_POWER: 	/* 正向无功费率4电能, 返回4bytes数据 */
		*output_data_flag = 0x9114;
		*output_move_len = 0;
		break;

	case AC_REPOSITIVE_WATTLESS_POWER: 	/* 反向无功电能, 返回4bytes数据 */
		*output_data_flag = 0x9120;
		*output_move_len = 0;
		break;
	case AC_REPOSITIVE_WATTLESS_RATE1_POWER: /* 反向无功费率1电能, 返回4bytes数据 */
		*output_data_flag = 0x9121;
		*output_move_len = 0;
		break;
	case AC_REPOSITIVE_WATTLESS_RATE2_POWER: /* 反向无功费率2电能, 返回4bytes数据 */
		*output_data_flag = 0x9122;
		*output_move_len = 0;
		break;
	case AC_REPOSITIVE_WATTLESS_RATE3_POWER: /* 反向无功费率3电能, 返回4bytes数据 */
		*output_data_flag = 0x9123;
		*output_move_len = 0;
		break;
	case AC_REPOSITIVE_WATTLESS_RATE4_POWER: /* 反向无功费率4电能, 返回4bytes数据 */
		*output_data_flag = 0x9124;
		*output_move_len = 0;
		break;
#if 0
	AC_A_POSITIVE_ACTIVE_POWER, 		/* (当前)A相正向有功电能, 返回4bytes数据 */
	AC_A_REPOSITIVE_ACTIVE_POWER, 		/* (当前)A相反向有功电能, 返回4bytes数据 */
	AC_B_POSITIVE_ACTIVE_POWER, 		/* (当前)B相正向有功电能, 返回4bytes数据 */
	AC_B_REPOSITIVE_ACTIVE_POWER, 		/* (当前)B相反向有功电能, 返回4bytes数据 */
	AC_C_POSITIVE_ACTIVE_POWER, 		/* (当前)C相正向有功电能, 返回4bytes数据 */
	AC_C_REPOSITIVE_ACTIVE_POWER, 		/* (当前)C相反向有功电能, 返回4bytes数据 */
#endif

	case AC_A_VOLTAGE: 					/* A相电压, 返回2bytes数据 */
		*output_data_flag = 0xB611;
		*output_move_len = 1;
		break;
	case AC_B_VOLTAGE: 					/* B相电压, 返回2bytes数据 */
		*output_data_flag = 0xB612;
		*output_move_len = 1;
		break;
	case AC_C_VOLTAGE: 					/* C相电压, 返回2bytes数据 */
		*output_data_flag = 0xB613;
		*output_move_len = 1;
		break;

	case AC_A_CURRENT: 					/* A相电流, 返回2bytes数据 */
		*output_data_flag = 0xB621;
		*output_move_len = 1;
		break;
	case AC_B_CURRENT: 					/* B相电流, 返回2bytes数据 */
		*output_data_flag = 0xB622;
		*output_move_len = 1;
		break;
	case AC_C_CURRENT: 					/* C相电流, 返回2bytes数据 */
		*output_data_flag = 0xB623;
		*output_move_len = 1;
		break;

	case AC_INSTANT_ACTIVE_POWER: 		/* 瞬时有功功率, 返回3bytes数据 */
		*output_data_flag = 0xB630;
		*output_move_len = 0;
		break;
	case AC_A_ACTIVE_POWER: 				/* A相有功功率, 返回3bytes数据 */
		*output_data_flag = 0xB631;
		*output_move_len = 0;
		break;
	case AC_B_ACTIVE_POWER: 				/* B相有功功率, 返回3bytes数据 */
		*output_data_flag = 0xB632;
		*output_move_len = 0;
		break;
	case AC_C_ACTIVE_POWER: 				/* C相有功功率, 返回3bytes数据 */
		*output_data_flag = 0xB633;
		*output_move_len = 0;
		break;

	case AC_INSTANT_REACTIVE_POWER: 		/* 瞬时无功功率, 返回2bytes数据 */
		*output_data_flag = 0xB640;
		*output_move_len = 2;
		break;
	case AC_A_REACTIVE_POWER: 			/* A相无功功率, 返回2bytes数据 */
		*output_data_flag = 0xB641;
		*output_move_len = 2;
		break;
	case AC_B_REACTIVE_POWER: 			/* B相无功功率, 返回2bytes数据 */
		*output_data_flag = 0xB642;
		*output_move_len = 2;
		break;
	case AC_C_REACTIVE_POWER: 			/* C相无功功率, 返回2bytes数据 */
		*output_data_flag = 0xB643;
		*output_move_len = 2;
		break;

	case AC_TOTAL_POWER_FACTOR: 			/* 总功率因数, 返回2bytes数据 */
		*output_data_flag = 0xB650;
		*output_move_len = 0;
		break;
	case AC_A_POWER_FACTOR: 				/* A相功率因数, 返回2bytes数据 */
		*output_data_flag = 0xB651;
		*output_move_len = 0;
		break;
	case AC_B_POWER_FACTOR: 				/* B相功率因数, 返回2bytes数据 */
		*output_data_flag = 0xB652;
		*output_move_len = 0;
		break;
	case AC_C_POWER_FACTOR: 				/* C相功率因数, 返回2bytes数据 */
		*output_data_flag = 0xB653;
		*output_move_len = 0;
		break;

	case AC_DATE_AND_WEEK:				/* 日期及周次,年月日星期,返回4bytes数据  */
		*output_data_flag = 0XC010;
		*output_move_len = 0;
		break;
	case AC_AMMETER_TIME:				/* 时间,时分秒,返回3bytes数据 */
		*output_data_flag = 0XC011;
		*output_move_len = 0;
		break;
	case AC_POWER_CONSTANT:				/* 电表常数(有功),返回3bytes数据 */
		*output_data_flag = 0XC030;
		*output_move_len = 0;
		break;
	case AC_GET_AMMETR_ADDR: 			/* 电表的表号, 返回6bytes数据, 高位在前 */
		*output_data_flag = 0XC032;
		*output_move_len = 0;
		break;

	default:
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	return RT_EOK;
}
/**********************************************************************************
* Function	 : 	static rt_err_t output645_97maxneed_data_flag(enum ammeter_maxneed_cmd cmd, rt_uint32_t *output_maxneed_data_flag, rt_uint32_t *output_time_data_flag)
* Description: 	根据输入的读取最大需量数据的命令，输出645_97规约对应的最大需量数据标识和发生时间数据标识
* Arguments	 : 	（1）cmd：输入的读表命令；（2）output_maxneed_data_flag：输出645_97规约相应的最大需量数据标识;（3）output_time_data_flag：输出645_97规约相应的发生时间数据标识
* Return	 :	RT_EOK
*************************************************************************************/
static rt_err_t output645_97maxneed_data_flag(enum ammeter_maxneed_cmd cmd, rt_uint32_t *output_maxneed_data_flag, rt_uint32_t *output_time_data_flag)
{
	rt_uint32_t maxneed_data_flag = 0;
	rt_uint32_t time_data_flag = 0;

	if (output_maxneed_data_flag == NULL || output_time_data_flag == NULL) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	switch (cmd) {
	case CMD_MAXNEED_TOTAL_POSITIVE_ACTIVE:     /* (当前)正向有功总最大需量及发生时间, 返回8bytes数据  */
		maxneed_data_flag = 0xA010;
		time_data_flag = 0xB010;
		break;
	case CMD_MAXNEED_RATE1_POSITIVE_ACTIVE:     /* (当前)正向有功费率1最大需量及发生时间, 返回8bytes数据  */
		maxneed_data_flag = 0xA011;
		time_data_flag = 0xB011;
		break;
	case CMD_MAXNEED_RATE2_POSITIVE_ACTIVE:     /* (当前)正向有功费率2最大需量及发生时间, 返回8bytes数据  */
		maxneed_data_flag = 0xA012;
		time_data_flag = 0xB012;
		break;
	case CMD_MAXNEED_RATE3_POSITIVE_ACTIVE:     /* (当前)正向有功费率3最大需量及发生时间, 返回8bytes数据  */
		maxneed_data_flag = 0xA013;
		time_data_flag = 0xB013;
		break;
	case CMD_MAXNEED_RATE4_POSITIVE_ACTIVE:     /* (当前)正向有功费率4最大需量及发生时间, 返回8bytes数据  */
		maxneed_data_flag = 0xA014;
		time_data_flag = 0xB014;
		break;
	case CMD_MAXNEED_TOTAL_REPOSITIVE_ACTIVE:     /* (当前)反向有功总最大需量及发生时间, 返回8bytes数据  */
		maxneed_data_flag = 0xA020;
		time_data_flag = 0xB020;
		break;
	case CMD_MAXNEED_RATE1_REPOSITIVE_ACTIVE:     /* (当前)反向有功费率1最大需量及发生时间, 返回8bytes数据  */
		maxneed_data_flag = 0xA021;
		time_data_flag = 0xB021;
		break;
	case CMD_MAXNEED_RATE2_REPOSITIVE_ACTIVE:     /* (当前)反向有功费率2最大需量及发生时间, 返回8bytes数据  */
		maxneed_data_flag = 0xA022;
		time_data_flag = 0xB022;
		break;
	case CMD_MAXNEED_RATE3_REPOSITIVE_ACTIVE:     /* (当前)反向有功费率3最大需量及发生时间, 返回8bytes数据 */
		maxneed_data_flag = 0xA023;
		time_data_flag = 0xB023;
		break;
	case CMD_MAXNEED_RATE4_REPOSITIVE_ACTIVE:     /* (当前)反向有功费率4最大需量及发生时间, 返回8bytes数据 */
		maxneed_data_flag = 0xA024;
		time_data_flag = 0xB024;
		break;

	default:
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	*output_maxneed_data_flag = maxneed_data_flag;
	*output_time_data_flag = time_data_flag;

	return RT_EOK;
}
/**********************************************************************************
* Function	 : 	static enum frame_error_e get_data_from_645_07(rt_uint8_t *addr, rt_uint32_t data_flag, rt_uint8_t *output_data, rt_uint32_t *output_data_len, enum ammeter_uart_e port)
* Description: 	读取645-2007规约电表的数据
* Arguments	 : 	（1）addr：输入的电表的地址；（2）data_flag：输入的数据标识；（3）output_data：传出的数据；（4）output_data_len：传出的数据长度；（5）port：连接电表所用的端口
* Return	 :	enum frame_error_e
*************************************************************************************/
enum frame_error_e get_data_from_645_07(rt_uint8_t *addr, rt_uint32_t data_flag, rt_uint8_t *output_data, rt_uint32_t *output_data_len, enum ammeter_uart_e port)
{
	enum frame_error_e result = FRAME_E_ERROR;
	struct send_frame_param send_645_07frame_data;
	struct recv_frame_param recv_645_07frame_data;

	if (addr == NULL || output_data == NULL || output_data_len == NULL) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	rt_memset(&send_645_07frame_data, 0x00, sizeof(struct send_frame_param));
	rt_memset(&recv_645_07frame_data, 0x00, sizeof(struct recv_frame_param));

	rt_memcpy(send_645_07frame_data.addr, addr, 6);
	send_645_07frame_data.ctrl = F645_07CMD_READ_DATA;
	send_645_07frame_data.data_flag = data_flag;

	recv_645_07frame_data.data = output_data;

	result = transmit_ammeter645_07_data(&send_645_07frame_data, &recv_645_07frame_data, port);
	if (result == FRAME_E_OK) {
		*output_data_len = recv_645_07frame_data.data_len;
	} else {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return result;
	}

	return FRAME_E_OK;
}
/**********************************************************************************
* Function	 : 	static rt_err_t output645_07power_data_flag(enum ammeter_cmd_e cmd, rt_uint32_t *output_data_flag)
* Description: 	根据输入的读取电能量的命令，输出645_07规约对应的数据标识
* Arguments	 : 	（1）cmd：输入的读表命令；（2）output_data_flag：输出645_07规约相应的数据类型
* Return	 :	RT_EOK
*************************************************************************************/
rt_err_t output645_07power_data_flag(enum ammeter_cmd_e cmd, rt_uint32_t *output_data_flag)
{
	if (output_data_flag == NULL) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	switch (cmd) {
	case AC_COMBINATION_ACTIVE_TOTAL_POWER:		/* 组合有功总电能, 返回4bytes数据 */
		*output_data_flag = 0x00000000;
		break;
	case AC_COMBINATION_ACTIVE_RATE1_POWER:		/* 组合有功费率1电能, 返回4bytes数据 */
		*output_data_flag = 0x00000100;
		break;
	case AC_COMBINATION_ACTIVE_RATE2_POWER:		/* 组合有功费率2电能, 返回4bytes数据 */
		*output_data_flag = 0x00000200;
		break;
	case AC_COMBINATION_ACTIVE_RATE3_POWER:		/* 组合有功费率3电能, 返回4bytes数据 */
		*output_data_flag = 0x00000300;
		break;
	case AC_COMBINATION_ACTIVE_RATE4_POWER:		/* 组合有功费率4电能, 返回4bytes数据 */
		*output_data_flag = 0x00000400;
		break;

	case AC_POSITIVE_ACTIVE_POWER:			/* 正向有功电能, 返回4bytes数据 */
		*output_data_flag = 0x00010000;
		break;
	case AC_POSITIVE_ACTIVE_RATE1_POWER:  	/* 正向有功费率1电能, 返回4bytes数据 */
		*output_data_flag = 0x00010100;
		break;
	case AC_POSITIVE_ACTIVE_RATE2_POWER:  	/* 正向有功费率2电能, 返回4bytes数据 */
		*output_data_flag = 0x00010200;
		break;
	case AC_POSITIVE_ACTIVE_RATE3_POWER:  	/* 正向有功费率3电能, 返回4bytes数据 */
		*output_data_flag = 0x00010300;
		break;
	case AC_POSITIVE_ACTIVE_RATE4_POWER:  	/* 正向有功费率4电能, 返回4bytes数据 */
		*output_data_flag = 0x00010400;
		break;

	case AC_REPOSITIVE_ACTIVE_POWER: 	/* 反向有功电能, 返回4bytes数据 */
		*output_data_flag = 0x00020000;
		break;
	case AC_REPOSITIVE_ACTIVE_RATE1_POWER: 	/* 反向有功费率1电能, 返回4bytes数据 */
		*output_data_flag = 0x00020100;
		break;
	case AC_REPOSITIVE_ACTIVE_RATE2_POWER: 	/* 反向有功费率2电能, 返回4bytes数据 */
		*output_data_flag = 0x00020200;
		break;
	case AC_REPOSITIVE_ACTIVE_RATE3_POWER: 	/* 反向有功费率3电能, 返回4bytes数据 */
		*output_data_flag = 0x00020300;
		break;
	case AC_REPOSITIVE_ACTIVE_RATE4_POWER: 	/* 反向有功费率4电能, 返回4bytes数据 */
		*output_data_flag = 0x00020400;
		break;

	case AC_POSITIVE_WATTLESS_POWER: 	/* 正向无功电能, 返回4bytes数据 */
		*output_data_flag = 0x00030000;
		break;
	case AC_POSITIVE_WATTLESS_RATE1_POWER: 	/* 正向无功费率1电能, 返回4bytes数据 */
		*output_data_flag = 0x00030100;
			break;
	case AC_POSITIVE_WATTLESS_RATE2_POWER: 	/* 正向无功费率2电能, 返回4bytes数据 */
		*output_data_flag = 0x00030200;
			break;
	case AC_POSITIVE_WATTLESS_RATE3_POWER: 	/* 正向无功费率3电能, 返回4bytes数据 */
		*output_data_flag = 0x00030300;
			break;
	case AC_POSITIVE_WATTLESS_RATE4_POWER: 	/* 正向无功费率4电能, 返回4bytes数据 */
		*output_data_flag = 0x00030400;
			break;

	case AC_REPOSITIVE_WATTLESS_POWER: 	/* 反向无功电能, 返回4bytes数据 */
		*output_data_flag = 0x00040000;
		break;
	case AC_REPOSITIVE_WATTLESS_RATE1_POWER: /* 反向无功费率1电能, 返回4bytes数据 */
		*output_data_flag = 0x00040100;
		break;
	case AC_REPOSITIVE_WATTLESS_RATE2_POWER: /* 反向无功费率2电能, 返回4bytes数据 */
		*output_data_flag = 0x00040200;
		break;
	case AC_REPOSITIVE_WATTLESS_RATE3_POWER: /* 反向无功费率3电能, 返回4bytes数据 */
		*output_data_flag = 0x00040300;
		break;
	case AC_REPOSITIVE_WATTLESS_RATE4_POWER: /* 反向无功费率4电能, 返回4bytes数据 */
		*output_data_flag = 0x00040400;
		break;

	case AC_A_POSITIVE_ACTIVE_POWER: 		/* (当前)A相正向有功电能, 返回4bytes数据 */
		*output_data_flag = 0x00150000;
		break;
	case AC_A_REPOSITIVE_ACTIVE_POWER: 		/* (当前)A相反向有功电能, 返回4bytes数据 */
		*output_data_flag = 0x00160000;
		break;
	case AC_B_POSITIVE_ACTIVE_POWER: 		/* (当前)B相正向有功电能, 返回4bytes数据 */
		*output_data_flag = 0x00290000;
		break;
	case AC_B_REPOSITIVE_ACTIVE_POWER: 		/* (当前)B相反向有功电能, 返回4bytes数据 */
		*output_data_flag = 0x002A0000;
		break;
	case AC_C_POSITIVE_ACTIVE_POWER: 		/* (当前)C相正向有功电能, 返回4bytes数据 */
		*output_data_flag = 0x003D0000;
		break;
	case AC_C_REPOSITIVE_ACTIVE_POWER: 		/* (当前)C相反向有功电能, 返回4bytes数据 */
		*output_data_flag = 0x003E0000;
		break;

	case AC_A_VOLTAGE: 					/* A相电压, 返回2bytes数据 */
		*output_data_flag = 0x02010100;
		break;
	case AC_B_VOLTAGE: 					/* B相电压, 返回2bytes数据 */
		*output_data_flag = 0x02010200;
		break;
	case AC_C_VOLTAGE: 					/* C相电压, 返回2bytes数据 */
		*output_data_flag = 0x02010300;
		break;

	case AC_A_CURRENT: 					/* A相电流, 返回2bytes数据 */
		*output_data_flag = 0x02020100;
		break;
	case AC_B_CURRENT: 					/* B相电流, 返回2bytes数据 */
		*output_data_flag = 0x02020200;
		break;
	case AC_C_CURRENT: 					/* C相电流, 返回2bytes数据 */
		*output_data_flag = 0x02020300;
		break;

	case AC_INSTANT_ACTIVE_POWER: 		/* 瞬时有功功率, 返回3bytes数据 */
		*output_data_flag = 0x02030000;
		break;
	case AC_A_ACTIVE_POWER: 				/* A相有功功率, 返回3bytes数据 */
		*output_data_flag = 0x02030100;
		break;
	case AC_B_ACTIVE_POWER: 				/* B相有功功率, 返回3bytes数据 */
		*output_data_flag = 0x02030200;
		break;
	case AC_C_ACTIVE_POWER: 				/* C相有功功率, 返回3bytes数据 */
		*output_data_flag = 0x02030300;
		break;

	case AC_INSTANT_REACTIVE_POWER: 		/* 瞬时无功功率, 返回2bytes数据 */
		*output_data_flag = 0x02040000;
		break;
	case AC_A_REACTIVE_POWER: 			/* A相无功功率, 返回2bytes数据 */
		*output_data_flag = 0x02040100;
		break;
	case AC_B_REACTIVE_POWER: 			/* B相无功功率, 返回2bytes数据 */
		*output_data_flag = 0x02040200;
		break;
	case AC_C_REACTIVE_POWER: 			/* C相无功功率, 返回2bytes数据 */
		*output_data_flag = 0x02040300;
		break;

	case AC_TOTAL_POWER_FACTOR: 			/* 总功率因数, 返回2bytes数据 */
		*output_data_flag = 0x02060000;
		break;
	case AC_A_POWER_FACTOR: 				/* A相功率因数, 返回2bytes数据 */
		*output_data_flag = 0x02060100;
		break;
	case AC_B_POWER_FACTOR: 				/* B相功率因数, 返回2bytes数据 */
		*output_data_flag = 0x02060200;
		break;
	case AC_C_POWER_FACTOR: 				/* C相功率因数, 返回2bytes数据 */
		*output_data_flag = 0x02060300;
		break;

	case AC_TOTAL_APPARENT_POWER:		/* 瞬时总视在功率, 返回3bytes数据 */
		*output_data_flag = 0X02050000;
		break;
	case AC_A_APPARENT_POWER:			/* 瞬时A相视在功率, 返回3bytes数据 */
		*output_data_flag = 0X02050100;
		break;
	case AC_B_APPARENT_POWER:			/* 瞬时B相视在功率, 返回3bytes数据 */
		*output_data_flag = 0X02050200;
		break;
	case AC_C_APPARENT_POWER:			/* 瞬时C相视在功率, 返回3bytes数据 */
		*output_data_flag = 0X02050300;
		break;
	case AC_A_VOLTAGE_DISTORTION:		/* A相电压波形失真度, 返回2bytes数据 */
		*output_data_flag = 0X02080100;
		break;
	case AC_B_VOLTAGE_DISTORTION:		/* B相电压波形失真度, 返回2bytes数据 */
		*output_data_flag = 0X02080200;
		break;
	case AC_C_VOLTAGE_DISTORTION:		/* C相电压波形失真度, 返回2bytes数据 */
		*output_data_flag = 0X02080300;
		break;
	case AC_A_CURRENT_DISTORTION:		/* A相电流波形失真度, 返回2bytes数据 */
		*output_data_flag = 0X02090100;
		break;
	case AC_B_CURRENT_DISTORTION:		/* B相电流波形失真度, 返回2bytes数据 */
		*output_data_flag = 0X02090200;
		break;
	case AC_C_CURRENT_DISTORTION:		/* C相电流波形失真度, 返回2bytes数据 */
		*output_data_flag = 0X02090300;
		break;
	case AC_A_VOLTAGE_HARMONIC:				/* A相电压谐波含量数据块, 返回42bytes数据 */
		*output_data_flag = 0X020A01FF;
		break;
	case AC_B_VOLTAGE_HARMONIC:				/* B相电压谐波含量数据块, 返回42bytes数据  */
		*output_data_flag = 0X020A02FF;
		break;
	case AC_C_VOLTAGE_HARMONIC:				/* C相电压谐波含量数据块, 返回42bytes数据  */
		*output_data_flag = 0X020A03FF;
		break;
	case AC_A_CURRENT_HARMONIC:				/* A相电流谐波含量数据块, 返回42bytes数据  */
		*output_data_flag = 0X020B01FF;
		break;
	case AC_B_CURRENT_HARMONIC:				/* B相电流谐波含量数据块, 返回42bytes数据  */
		*output_data_flag = 0X020B02FF;
		break;
	case AC_C_CURRENT_HARMONIC:				/* C相电流谐波含量数据块, 返回42bytes数据  */
		*output_data_flag = 0X020B03FF;
		break;
	case AC_DATE_AND_WEEK:				/* 日期及周次,年月日星期,返回4bytes数据  */
		*output_data_flag = 0X04000101;
		break;
	case AC_AMMETER_TIME:				/* 时间,时分秒,返回3bytes数据 */
		*output_data_flag = 0X04000102;
		break;
	case AC_POWER_CONSTANT:				/* 电表常数(有功), 返回3bytes数据  */
		*output_data_flag = 0X04000409;
		break;
	case AC_GET_AMMETR_ADDR: 			/* 电表的表号, 返回6bytes数据, 高位在前 */
		*output_data_flag = 0X04000402;
		break;
	case AC_POWER_FREQUENCY:			/* 电网频率, 返回2bytes数据*/
		*output_data_flag = 0X02800002;
		break;
	case AC_AMMETER_TEMPERATURE: 		/* 表内温度, 返回2bytes数据 */
		*output_data_flag = 0X02800007;
		break;
	case AC_CLOCK_BATTERY_VOLTAGE: 		/* 时钟电池电压(内部), 返回2bytes数据 */
		*output_data_flag = 0X02800008;
		break;
	case AC_METER_READ_BATTERY_VOLTAGE: 	/* 停电抄表电池电压 (外部), 返回2bytes数据 */
		*output_data_flag = 0X02800009;
		break;
	case AC_TOTAL_COPPER_LOSS_ACTIVE_POWER: 	/* (当前)铜损有功总电能补偿量 , 返回4bytes数据 */
		*output_data_flag = 0X00850000;
		break;
	case AC_A_COPPER_LOSS_ACTIVE_POWER: 		/* (当前)A相铜损有功电能补偿量, 返回4bytes数据 */
		*output_data_flag = 0X00990000;
		break;
	case AC_B_COPPER_LOSS_ACTIVE_POWER: 		/* (当前)B相铜损有功电能补偿量 , 返回4bytes数据 */
		*output_data_flag = 0X00AD0000;
		break;
	case AC_C_COPPER_LOSS_ACTIVE_POWER: 		/* (当前)C相铜损有功电能补偿量, 返回4bytes数据  */
		*output_data_flag = 0X00C10000;
		break;
	case AC_TOTAL_IRON_LOSS_ACTIVE_POWER: 	/* (当前)铁损有功总电能补偿量, 返回4bytes数据  */
		*output_data_flag = 0X00860000;
		break;
	case AC_A_IRON_LOSS_ACTIVE_POWER: 		/* (当前)A相铁损有功电能补偿量, 返回4bytes数据  */
		*output_data_flag = 0X009A0000;
		break;
	case AC_B_IRON_LOSS_ACTIVE_POWER: 		/* (当前)B相铁损有功电能补偿量, 返回4bytes数据  */
		*output_data_flag = 0X00AE0000;
		break;
	case AC_C_IRON_LOSS_ACTIVE_POWER: 		/* (当前)C相铁损有功电能补偿量 , 返回4bytes数据 */
		*output_data_flag = 0X00C20000;
		break;
	default:
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	return RT_EOK;
}
/**********************************************************************************
* Function	 : 	static rt_err_t output645_07forzen_data_flag(enum ammeter_forzen_cmd cmd, rt_uint32_t *output_data_flag, rt_uint32_t *output_data_len)
* Description: 	根据输入的读取冻结数据得命令，输出645_07规约对应的数据标识
* Arguments	 : 	（1）cmd：输入的读表命令；（2）output_data_flag：输出645_07规约相应的数据类型.(3)电表所应输出数据的长度
* Return	 :	RT_EOK
*************************************************************************************/
static rt_err_t output645_07forzen_data_flag(enum ammeter_forzen_cmd cmd, rt_uint32_t *output_data_flag, rt_uint32_t *output_data_len)
{
	if (output_data_flag == RT_NULL || output_data_len == RT_NULL) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	switch (cmd) {
	case CMD_FORZEN_TIMEING_TIME:     				/* （上1次）定时冻结时间, 返回5bytes数据  */
		*output_data_flag = 0x05000001;
		*output_data_len = 5;
		break;
	case CMD_FORZEN_TIMEING_POSITIVE_ACTIVE_POWER:   /* （上1次）定时冻结正向有功总电能数据, 返回4bytes数据  */
		*output_data_flag = 0x05000101;
		*output_data_len = 4;
		break;
	case CMD_FORZEN_TIMEING_REPOSITIVE_ACTIVE_POWER: /* （上1次）定时冻结反向有功总电能数据, 返回4bytes数据  */
		*output_data_flag = 0x05000201;
		*output_data_len = 4;
		break;
	case CMD_FORZEN_TIMEING_POSITIVE_WATTLESS_POWER: /* （上1次）定时冻结正向无功总电能数据, 返回4bytes数据  */
		*output_data_flag = 0x05000301;
		*output_data_len = 4;
		break;
	case CMD_FORZEN_TIMEING_REPOSITIVE_WATTLESS_POWER: /* （上1次）定时冻结反向无功总电能数据, 返回4bytes数据  */
		*output_data_flag = 0x05000401;
		*output_data_len = 4;
		break;
	case CMD_FORZEN_TIMEING_POSITIVE_MAXNEED_AND_TIME: 	/* （上1次）定时冻结正向有功总最大需量及发生时间数据, 返回8bytes数据  */
		*output_data_flag = 0x05000901;
		*output_data_len = 8;
		break;
	case CMD_FORZEN_TIMEING_REPOSITIVE_MAXNEED_AND_TIME: /* （上1次）定时冻结反向有功总最大需量及发生时间数据, 返回8bytes数据  */
		*output_data_flag = 0x05000A01;
		*output_data_len = 8;
		break;
	case CMD_FORZEN_TIMEING_VARIABLE_DATA: 			/* （上1次）定时冻结变量数据, 返回3×8bytes数据  */
		*output_data_flag = 0x05001001;
		*output_data_len = 24;
		break;

	case CMD_FORZEN_INSTANT_TIME:     				/* （上1次）瞬时冻结时间, 返回5bytes数据  */
		*output_data_flag = 0x05010001;
		*output_data_len = 5;
		break;
	case CMD_FORZEN_INSTANT_POSITIVE_ACTIVE_POWER:   /* （上1次）瞬时冻结正向有功总电能数据, 返回4bytes数据  */
		*output_data_flag = 0x05010101;
		*output_data_len = 4;
		break;
	case CMD_FORZEN_INSTANT_REPOSITIVE_ACTIVE_POWER: /* （上1次）瞬时冻结反向有功总电能数据, 返回4bytes数据  */
		*output_data_flag = 0x05010201;
		*output_data_len = 4;
		break;
	case CMD_FORZEN_INSTANT_POSITIVE_WATTLESS_POWER: /* （上1次）瞬时冻结正向无功总电能数据, 返回4bytes数据  */
		*output_data_flag = 0x05010301;
		*output_data_len = 4;
		break;
	case CMD_FORZEN_INSTANT_REPOSITIVE_WATTLESS_POWER: /* （上1次）瞬时冻结反向无功总电能数据, 返回4bytes数据  */
		*output_data_flag = 0x05010401;
		*output_data_len = 4;
		break;
	case CMD_FORZEN_INSTANT_POSITIVE_MAXNEED_AND_TIME: 	/* （上1次）瞬时冻结正向有功总最大需量及发生时间数据, 返回8bytes数据  */
		*output_data_flag = 0x05010901;
		*output_data_len = 8;
		break;
	case CMD_FORZEN_TINSTANT_REPOSITIVE_MAXNEED_AND_TIME:/* （上1次）瞬时冻结反向有功总最大需量及发生时间数据, 返回8bytes数据  */
		*output_data_flag = 0x05010A01;
		*output_data_len = 8;
		break;
	case CMD_FORZEN_INSTANT_VARIABLE_DATA: 			/* （上1次）瞬时冻结变量数据, 返回3×8bytes数据  */
		*output_data_flag = 0x05011001;
		*output_data_len = 24;
		break;
	default:
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	return RT_EOK;
}

/**********************************************************************************
* Function	 : 	static rt_err_t output645_07maxneed_data_flag(enum ammeter_maxneed_cmd cmd, rt_uint32_t *output_data_flag)
* Description: 	根据输入的读取最大需量数据的命令，输出645_07规约对应的数据标识
* Arguments	 : 	（1）cmd：输入的读表命令；（2）output_data_flag：输出645_07规约相应的数据标识
* Return	 :	RT_EOK
*************************************************************************************/
static rt_err_t output645_07maxneed_data_flag(enum ammeter_maxneed_cmd cmd, rt_uint32_t *output_data_flag)
{
	if (output_data_flag == NULL) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	switch (cmd) {
	case CMD_MAXNEED_TOTAL_POSITIVE_ACTIVE:     /* (当前)正向有功总最大需量及发生时间, 返回8bytes数据  */
		*output_data_flag = 0x01010000;
		break;
	case CMD_MAXNEED_RATE1_POSITIVE_ACTIVE:     /* (当前)正向有功费率1最大需量及发生时间, 返回8bytes数据  */
		*output_data_flag = 0x01010100;
		break;
	case CMD_MAXNEED_RATE2_POSITIVE_ACTIVE:     /* (当前)正向有功费率2最大需量及发生时间, 返回8bytes数据  */
		*output_data_flag = 0x01010200;
		break;
	case CMD_MAXNEED_RATE3_POSITIVE_ACTIVE:     /* (当前)正向有功费率3最大需量及发生时间, 返回8bytes数据  */
		*output_data_flag = 0x01010300;
		break;
	case CMD_MAXNEED_RATE4_POSITIVE_ACTIVE:     /* (当前)正向有功费率4最大需量及发生时间, 返回8bytes数据  */
		*output_data_flag = 0x01010400;
		break;
	case CMD_MAXNEED_TOTAL_REPOSITIVE_ACTIVE:     /* (当前)反向有功总最大需量及发生时间, 返回8bytes数据  */
		*output_data_flag = 0x01020000;
		break;
	case CMD_MAXNEED_RATE1_REPOSITIVE_ACTIVE:     /* (当前)反向有功费率1最大需量及发生时间, 返回8bytes数据  */
		*output_data_flag = 0x01020100;
		break;
	case CMD_MAXNEED_RATE2_REPOSITIVE_ACTIVE:     /* (当前)反向有功费率2最大需量及发生时间, 返回8bytes数据  */
		*output_data_flag = 0x01020200;
		break;
	case CMD_MAXNEED_RATE3_REPOSITIVE_ACTIVE:     /* (当前)反向有功费率3最大需量及发生时间, 返回8bytes数据 */
		*output_data_flag = 0x01020300;
		break;
	case CMD_MAXNEED_RATE4_REPOSITIVE_ACTIVE:     /* (当前)反向有功费率4最大需量及发生时间, 返回8bytes数据 */
		*output_data_flag = 0x01020400;
		break;
	default:
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	return RT_EOK;
}
/**********************************************************************************
* Function	 : 	static enum frame_error_e read_ammeter645_07_event_data(rt_uint8_t *addr, rt_uint32_t addr_num, enum ammeter_uart_e port)
* Description: 	读取645-07规约电表的事件数据
* Arguments	 : 	（1）addr：输入的电表地址；(2)addr_num:输入的电表地址的序号；（3）port：电表连接的端口；
* Return	 :	enum frame_error_e
*************************************************************************************/
static enum frame_error_e read_ammeter645_07_event_data(rt_uint8_t *addr, rt_uint32_t addr_num, enum ammeter_uart_e port)
{
	rt_uint16_t status_word = 0;
	static rt_uint8_t event_flag = 0;
	static rt_uint8_t event_info[NUM_OF_COLLECT_EM_MAX][6][3];
	rt_uint32_t len = 0;
	rt_uint8_t buf[16] = {'\0'};
	enum frame_error_e result = FRAME_E_ERROR;

	if (addr_num < 0 || addr_num >= NUM_OF_COLLECT_EM_MAX) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	/* 电表运行状态字（A相故障状态） */
	result = get_data_from_645_07(addr, 0x04000504, buf, &len, port);
	if (result == FRAME_E_OK) {
		status_word = buf[0] | buf[1] << 8;
		if (status_word != 0) {
			ammeter645_07_event_A_phase_upload(addr_num, status_word);
//			meterp_debug_main(("meter A phase status changed: %04x\n", status_word));
		}
	}

	rt_memset(buf, 0, sizeof(buf));

	/* 电表运行状态字（B相故障状态） */
	result = get_data_from_645_07(addr, 0x04000505, buf, &len, port);
	if (result == FRAME_E_OK) {
		status_word = buf[0] | buf[1] << 8;
		if (status_word != 0) {
			ammeter645_07_event_B_phase_upload(addr_num, status_word);
//			meterp_debug_main(("meter B phase status changed: %04x\n", status_word));
		}
	}

	rt_memset(buf, 0, sizeof(buf));

	/* 电表运行状态字（C相故障状态） */
	result = get_data_from_645_07(addr, 0x04000506, buf, &len, port);
	if (result == FRAME_E_OK) {
		status_word = buf[0] | buf[1] << 8;
		if (status_word != 0) {
			ammeter645_07_event_C_phase_upload(addr_num, status_word);
//			meterp_debug_main(("meter C phase status changed: %04x\n", status_word));
		}
	}

	rt_memset(buf, 0, sizeof(buf));

	/* 编程总次数， 返回3bytes数据 */
	result = get_data_from_645_07(addr, 0x03300000, buf, &len, port);
	if (result == FRAME_E_OK) {
		if (event_flag == 0) {
			rt_memcpy(event_info[addr_num][0], buf, 3);
		} else {
			if (rt_memcmp(event_info[addr_num][0], buf, 3) != 0) {
				trap_send(addr_num, 6, E_ALR_EM_PROGRAM_EVENT_APPEAR, E_ALR_EM_PROGRAM_EVENT_APPEAR, FLAG_EM);
				rt_memcpy(event_info[addr_num][0], buf, 3);
				rt_thread_delay(get_ticks_of_ms(50));
//				meterp_debug_main(("programming total times changed: %02x %02x %02x\n", buf[0], buf[1], buf[2]));
			}
		}
	}

	rt_memset(buf, 0, sizeof(buf));

	/* 电表清零总次数 ， 返回3bytes数据 */
	result = get_data_from_645_07(addr, 0x03300100, buf, &len, port);
	if (result == FRAME_E_OK) {
		if (event_flag == 0) {
			rt_memcpy(event_info[addr_num][1], buf, 3);
		} else {
			if (rt_memcmp(event_info[addr_num][1], buf, 3) != 0) {
				trap_send(addr_num, 6, E_ALR_EM_METER_CLEAR_EVENT_APPEAR, E_ALR_EM_METER_CLEAR_EVENT_APPEAR, FLAG_EM);
				rt_memcpy(event_info[addr_num][1], buf, 3);
				rt_thread_delay(get_ticks_of_ms(50));
//				meterp_debug_main(("meter clear total times changed: %02x %02x %02x\n", buf[0], buf[1], buf[2]));
			}
		}
	}

	rt_memset(buf, 0, sizeof(buf));

	/* 需量清零总次数 ， 返回3bytes数据 */
	result = get_data_from_645_07(addr, 0x03300200, buf, &len, port);
	if (result == FRAME_E_OK) {
		if (event_flag == 0) {
			rt_memcpy(event_info[addr_num][2], buf, 3);
		} else {
			if (rt_memcmp(event_info[addr_num][2], buf, 3) != 0) {
				trap_send(addr_num, 6, E_ALR_EM_DEMAND_CLEAR_EVENT_APPEAR, E_ALR_EM_DEMAND_CLEAR_EVENT_APPEAR, FLAG_EM);
				rt_memcpy(event_info[addr_num][2], buf, 3);
				rt_thread_delay(get_ticks_of_ms(50));
//				meterp_debug_main(("demand clear total times changed: %02x %02x %02x\n", buf[0], buf[1], buf[2]));
			}
		}
	}

	rt_memset(buf, 0, sizeof(buf));

	/* 校时总次数， 返回3bytes数据 */
	result = get_data_from_645_07(addr, 0x03300400, buf, &len, port);
	if (result == FRAME_E_OK) {
		if (event_flag == 0) {
			rt_memcpy(event_info[addr_num][3], buf, 3);
		} else {
			if (rt_memcmp(event_info[addr_num][3], buf, 3) != 0) {
				trap_send(addr_num, 6, E_ALR_EM_CALIBRATE_TIME_EVENT_APPEAR, E_ALR_EM_CALIBRATE_TIME_EVENT_APPEAR, FLAG_EM);
				rt_memcpy(event_info[addr_num][3], buf, 3);
				rt_thread_delay(get_ticks_of_ms(50));
//				meterp_debug_main(("timing total times changed: %02x %02x %02x\n", buf[0], buf[1], buf[2]));
			}
		}
	}

	rt_memset(buf, 0, sizeof(buf));

	/* 电压逆相序总次数，返回3bytes数据 */
	result = get_data_from_645_07(addr, 0x03070000, buf, &len, port);
	if (result == FRAME_E_OK) {
		if (event_flag == 0) {
			rt_memcpy(event_info[addr_num][4], buf, 3);
		} else {
			if (rt_memcmp(event_info[addr_num][4], buf, 3) != 0) {
				trap_send(addr_num, 6, E_ALR_EM_REVERSE_SEQ_VOL_EVENT_APPEAR, E_ALR_EM_REVERSE_SEQ_VOL_EVENT_APPEAR, FLAG_EM);
				rt_memcpy(event_info[addr_num][4], buf, 3);
				rt_thread_delay(get_ticks_of_ms(50));
//				meterp_debug_main(("voltage Reverse phase sequence total times changed: %02x %02x %02x\n", buf[0], buf[1], buf[2]));
			}
		}
	}

	rt_memset(buf, 0, sizeof(buf));

	/* 电流逆相序总次数，返回3bytes数据 */
	result = get_data_from_645_07(addr, 0x03080000, buf, &len, port);
	if (result == FRAME_E_OK) {
		if (event_flag == 0) {
			rt_memcpy(event_info[addr_num][5], buf, 3);
		} else {
			if (rt_memcmp(event_info[addr_num][5], buf, 3) != 0) {
				trap_send(addr_num, 6, E_ALR_EM_REVERSE_SEQ_CUR_EVENT_APPEAR, E_ALR_EM_REVERSE_SEQ_CUR_EVENT_APPEAR, FLAG_EM);
				rt_memcpy(event_info[addr_num][5], buf, 3);
				rt_thread_delay(get_ticks_of_ms(50));
//				meterp_debug_main(("current Reverse phase sequence total times changed: %02x %02x %02x\n", buf[0], buf[1], buf[2]));
			}
		}
	}

	event_flag = 1;

	return FRAME_E_OK;
}
/**********************************************************************************
* Function	 : 	static rt_err_t ammeter645_07_event_A_phase_upload(rt_uint32_t amm_addr_num, rt_uint16_t status_word)
* Description: 	上传电表A相事件
* Arguments	 : 	(1)amm_addr_num:输入电表地址的编号；（（2）status_word：输入的电表状态字;
* Return	 : 	return RT_EOK;
*************************************************************************************/
static rt_err_t ammeter645_07_event_A_phase_upload(rt_uint32_t amm_addr_num, rt_uint16_t status_word)
{
	rt_uint8_t n = 0;
	rt_uint8_t status = 0;
	static rt_uint8_t status_record[NUM_OF_COLLECT_EM_MAX][16];

	if (amm_addr_num < 0 || amm_addr_num >= NUM_OF_COLLECT_EM_MAX) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	n = 0;
	status = (status_word >>n) & 0x01;
	if (status == 1 && status_record[amm_addr_num][n] == 0) {
		trap_send(amm_addr_num, 6, E_ALR_PA_VOLTAGE_LOSS_EVENT_APPEAR, E_ALR_PA_VOLTAGE_LOSS_EVENT_APPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 1;
		rt_thread_delay(get_ticks_of_ms(50));
	} else if (status == 0 && status_record[amm_addr_num][n] == 1) {
		trap_send(amm_addr_num, 6, E_ALR_PA_VOLTAGE_LOSS_EVENT_DISAPPEAR, E_ALR_PA_VOLTAGE_LOSS_EVENT_DISAPPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 0;
		rt_thread_delay(get_ticks_of_ms(50));
	}
	n = 1;
	status = (status_word >>n) & 0x01;
	if (status == 1 && status_record[amm_addr_num][n] == 0) {
		trap_send(amm_addr_num, 6, E_ALR_PA_VOLTAGE_UNDER_EVENT_APPEAR, E_ALR_PA_VOLTAGE_UNDER_EVENT_APPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 1;
		rt_thread_delay(get_ticks_of_ms(50));
	} else if (status == 0 && status_record[amm_addr_num][n] == 1) {
		trap_send(amm_addr_num, 6, E_ALR_PA_VOLTAGE_UNDER_EVENT_DISAPPEAR, E_ALR_PA_VOLTAGE_UNDER_EVENT_DISAPPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 0;
		rt_thread_delay(get_ticks_of_ms(50));
	}
	n = 2;
	status = (status_word >>n) & 0x01;
	if (status == 1 && status_record[amm_addr_num][n] == 0) {
		trap_send(amm_addr_num, 6, E_ALR_PA_VOLTAGE_OVER_EVENT_APPEAR, E_ALR_PA_VOLTAGE_OVER_EVENT_APPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 1;
		rt_thread_delay(get_ticks_of_ms(50));
	} else if (status == 0 && status_record[amm_addr_num][n] == 1) {
		trap_send(amm_addr_num, 6, E_ALR_PA_VOLTAGE_OVER_EVENT_DISAPPEAR, E_ALR_PA_VOLTAGE_OVER_EVENT_DISAPPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 0;
		rt_thread_delay(get_ticks_of_ms(50));
	}
	n = 3;
	status = (status_word >>n) & 0x01;
	if (status == 1 && status_record[amm_addr_num][n] == 0) {
		trap_send(amm_addr_num, 6, E_ALR_PA_CURRENT_LOSS_EVENT_APPEAR, E_ALR_PA_CURRENT_LOSS_EVENT_APPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 1;
		rt_thread_delay(get_ticks_of_ms(50));
	} else if (status == 0 && status_record[amm_addr_num][n] == 1) {
		trap_send(amm_addr_num, 6, E_ALR_PA_CURRENT_LOSS_EVENT_DISAPPEAR, E_ALR_PA_CURRENT_LOSS_EVENT_DISAPPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 0;
		rt_thread_delay(get_ticks_of_ms(50));
	}
	n = 4;
	status = (status_word >>n) & 0x01;
	if (status == 1 && status_record[amm_addr_num][n] == 0) {
		trap_send(amm_addr_num, 6, E_ALR_PA_CURRENT_OVER_EVENT_APPEAR, E_ALR_PA_CURRENT_OVER_EVENT_APPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 1;
		rt_thread_delay(get_ticks_of_ms(50));
	} else if (status == 0 && status_record[amm_addr_num][n] == 1) {
		trap_send(amm_addr_num, 6, E_ALR_PA_CURRENT_OVER_EVENT_DISAPPEAR, E_ALR_PA_CURRENT_OVER_EVENT_DISAPPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 0;
		rt_thread_delay(get_ticks_of_ms(50));
	}
	n = 7;
	status = (status_word >>n) & 0x01;
	if (status == 1 && status_record[amm_addr_num][n] == 0) {
		trap_send(amm_addr_num, 6, E_ALR_PA_PHASE_BREAK_EVENT_APPEAR, E_ALR_PA_PHASE_BREAK_EVENT_APPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 1;
		rt_thread_delay(get_ticks_of_ms(50));
	} else if (status == 0 && status_record[amm_addr_num][n] == 1) {
		trap_send(amm_addr_num, 6, E_ALR_PA_PHASE_BREAK_EVENT_DISAPPEAR	, E_ALR_PA_PHASE_BREAK_EVENT_DISAPPEAR	, FLAG_EM);
		status_record[amm_addr_num][n] = 0;
		rt_thread_delay(get_ticks_of_ms(50));
	}
	n = 8;
	status = (status_word >>n) & 0x01;
	if (status == 1 && status_record[amm_addr_num][n] == 0) {
		trap_send(amm_addr_num, 6, E_ALR_PA_CURRENT_BREAK_EVENT_APPEAR, E_ALR_PA_CURRENT_BREAK_EVENT_APPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 1;
		rt_thread_delay(get_ticks_of_ms(50));
	} else if (status == 0 && status_record[amm_addr_num][n] == 1) {
		trap_send(amm_addr_num, 6, E_ALR_PA_CURRENT_BREAK_EVENT_DISAPPEAR, E_ALR_PA_CURRENT_BREAK_EVENT_DISAPPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 0;
		rt_thread_delay(get_ticks_of_ms(50));
	}

	return RT_EOK;
}
/**********************************************************************************
* Function	 : 	static rt_err_t ammeter645_07_event_B_phase_upload(rt_uint32_t amm_addr_num, rt_uint16_t status_word)
* Description: 	上传电表B相事件
* Arguments	 : 	(1)amm_addr_num:输入电表地址的编号；（（2）status_word：输入的电表状态字;
* Return	 : 	return RT_EOK;
*************************************************************************************/
static rt_err_t ammeter645_07_event_B_phase_upload(rt_uint32_t amm_addr_num, rt_uint16_t status_word)
{
	rt_uint8_t n = 0;
	rt_uint8_t status = 0;
	static rt_uint8_t status_record[NUM_OF_COLLECT_EM_MAX][16];

	if (amm_addr_num < 0 || amm_addr_num >= NUM_OF_COLLECT_EM_MAX) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	n = 0;
	status = (status_word >>n) & 0x01;
	if (status == 1 && status_record[amm_addr_num][n] == 0) {
		trap_send(amm_addr_num, 6, E_ALR_PB_VOLTAGE_LOSS_EVENT_APPEAR, E_ALR_PB_VOLTAGE_LOSS_EVENT_APPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 1;
		rt_thread_delay(get_ticks_of_ms(50));
	} else if (status == 0 && status_record[amm_addr_num][n] == 1) {
		trap_send(amm_addr_num, 6, E_ALR_PB_VOLTAGE_LOSS_EVENT_DISAPPEAR, E_ALR_PB_VOLTAGE_LOSS_EVENT_DISAPPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 0;
		rt_thread_delay(get_ticks_of_ms(50));
	}
	n = 1;
	status = (status_word >>n) & 0x01;
	if (status == 1 && status_record[amm_addr_num][n] == 0) {
		trap_send(amm_addr_num, 6, E_ALR_PB_VOLTAGE_UNDER_EVENT_APPEAR, E_ALR_PB_VOLTAGE_UNDER_EVENT_APPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 1;
		rt_thread_delay(get_ticks_of_ms(50));
	} else if (status == 0 && status_record[amm_addr_num][n] == 1) {
		trap_send(amm_addr_num, 6, E_ALR_PB_VOLTAGE_UNDER_EVENT_DISAPPEAR, E_ALR_PB_VOLTAGE_UNDER_EVENT_DISAPPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 0;
		rt_thread_delay(get_ticks_of_ms(50));
	}
	n = 2;
	status = (status_word >>n) & 0x01;
	if (status == 1 && status_record[amm_addr_num][n] == 0) {
		trap_send(amm_addr_num, 6, E_ALR_PB_VOLTAGE_OVER_EVENT_APPEAR, E_ALR_PB_VOLTAGE_OVER_EVENT_APPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 1;
		rt_thread_delay(get_ticks_of_ms(50));
	} else if (status == 0 && status_record[amm_addr_num][n] == 1) {
		trap_send(amm_addr_num, 6, E_ALR_PB_VOLTAGE_OVER_EVENT_DISAPPEAR, E_ALR_PB_VOLTAGE_OVER_EVENT_DISAPPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 0;
		rt_thread_delay(get_ticks_of_ms(50));
	}
	n = 3;
	status = (status_word >>n) & 0x01;
	if (status == 1 && status_record[amm_addr_num][n] == 0) {
		trap_send(amm_addr_num, 6, E_ALR_PB_CURRENT_LOSS_EVENT_APPEAR, E_ALR_PB_CURRENT_LOSS_EVENT_APPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 1;
		rt_thread_delay(get_ticks_of_ms(50));
	} else if (status == 0 && status_record[amm_addr_num][n] == 1) {
		trap_send(amm_addr_num, 6, E_ALR_PB_CURRENT_LOSS_EVENT_DISAPPEAR, E_ALR_PB_CURRENT_LOSS_EVENT_DISAPPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 0;
		rt_thread_delay(get_ticks_of_ms(50));
	}
	n = 4;
	status = (status_word >>n) & 0x01;
	if (status == 1 && status_record[amm_addr_num][n] == 0) {
		trap_send(amm_addr_num, 6, E_ALR_PB_CURRENT_OVER_EVENT_APPEAR, E_ALR_PB_CURRENT_OVER_EVENT_APPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 1;
		rt_thread_delay(get_ticks_of_ms(50));
	} else if (status == 0 && status_record[amm_addr_num][n] == 1) {
		trap_send(amm_addr_num, 6, E_ALR_PB_CURRENT_OVER_EVENT_DISAPPEAR, E_ALR_PB_CURRENT_OVER_EVENT_DISAPPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 0;
		rt_thread_delay(get_ticks_of_ms(50));
	}
	n = 7;
	status = (status_word >>n) & 0x01;
	if (status == 1 && status_record[amm_addr_num][n] == 0) {
		trap_send(amm_addr_num, 6, E_ALR_PB_PHASE_BREAK_EVENT_APPEAR, E_ALR_PB_PHASE_BREAK_EVENT_APPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 1;
		rt_thread_delay(get_ticks_of_ms(50));
	} else if (status == 0 && status_record[amm_addr_num][n] == 1) {
		trap_send(amm_addr_num, 6, E_ALR_PB_PHASE_BREAK_EVENT_DISAPPEAR	, E_ALR_PB_PHASE_BREAK_EVENT_DISAPPEAR	, FLAG_EM);
		status_record[amm_addr_num][n] = 0;
		rt_thread_delay(get_ticks_of_ms(50));
	}
	n = 8;
	status = (status_word >>n) & 0x01;
	if (status == 1 && status_record[amm_addr_num][n] == 0) {
		trap_send(amm_addr_num, 6, E_ALR_PB_CURRENT_BREAK_EVENT_APPEAR, E_ALR_PB_CURRENT_BREAK_EVENT_APPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 1;
		rt_thread_delay(get_ticks_of_ms(50));
	} else if (status == 0 && status_record[amm_addr_num][n] == 1) {
		trap_send(amm_addr_num, 6, E_ALR_PB_CURRENT_BREAK_EVENT_DISAPPEAR, E_ALR_PB_CURRENT_BREAK_EVENT_DISAPPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 0;
		rt_thread_delay(get_ticks_of_ms(50));
	}

	return RT_EOK;
}
/**********************************************************************************
* Function	 : 	static rt_err_t ammeter645_07_event_C_phase_upload(rt_uint32_t amm_addr_num, rt_uint16_t status_word)
* Description: 	上传电表C相事件
* Arguments	 : 	(1)amm_addr_num:输入电表地址的编号；（（2）status_word：输入的电表状态字;
* Return	 : 	return RT_EOK;
*************************************************************************************/
static rt_err_t ammeter645_07_event_C_phase_upload(rt_uint32_t amm_addr_num, rt_uint16_t status_word)
{
	rt_uint8_t n = 0;
	rt_uint8_t status = 0;
	static rt_uint8_t status_record[NUM_OF_COLLECT_EM_MAX][16];

	if (amm_addr_num < 0 || amm_addr_num >= NUM_OF_COLLECT_EM_MAX) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	n = 0;
	status = (status_word >>n) & 0x01;
	if (status == 1 && status_record[amm_addr_num][n] == 0) {
		trap_send(amm_addr_num, 6, E_ALR_PC_VOLTAGE_LOSS_EVENT_APPEAR, E_ALR_PC_VOLTAGE_LOSS_EVENT_APPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 1;
		rt_thread_delay(get_ticks_of_ms(50));
	} else if (status == 0 && status_record[amm_addr_num][n] == 1) {
		trap_send(amm_addr_num, 6, E_ALR_PC_VOLTAGE_LOSS_EVENT_DISAPPEAR, E_ALR_PC_VOLTAGE_LOSS_EVENT_DISAPPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 0;
		rt_thread_delay(get_ticks_of_ms(50));
	}
	n = 1;
	status = (status_word >>n) & 0x01;
	if (status == 1 && status_record[amm_addr_num][n] == 0) {
		trap_send(amm_addr_num, 6, E_ALR_PC_VOLTAGE_UNDER_EVENT_APPEAR, E_ALR_PC_VOLTAGE_UNDER_EVENT_APPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 1;
		rt_thread_delay(get_ticks_of_ms(50));
	} else if (status == 0 && status_record[amm_addr_num][n] == 1) {
		trap_send(amm_addr_num, 6, E_ALR_PC_VOLTAGE_UNDER_EVENT_DISAPPEAR, E_ALR_PC_VOLTAGE_UNDER_EVENT_DISAPPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 0;
		rt_thread_delay(get_ticks_of_ms(50));
	}
	n = 2;
	status = (status_word >>n) & 0x01;
	if (status == 1 && status_record[amm_addr_num][n] == 0) {
		trap_send(amm_addr_num, 6, E_ALR_PC_VOLTAGE_OVER_EVENT_APPEAR, E_ALR_PC_VOLTAGE_OVER_EVENT_APPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 1;
		rt_thread_delay(get_ticks_of_ms(50));
	} else if (status == 0 && status_record[amm_addr_num][n] == 1) {
		trap_send(amm_addr_num, 6, E_ALR_PC_VOLTAGE_OVER_EVENT_DISAPPEAR, E_ALR_PC_VOLTAGE_OVER_EVENT_DISAPPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 0;
		rt_thread_delay(get_ticks_of_ms(50));
	}
	n = 3;
	status = (status_word >>n) & 0x01;
	if (status == 1 && status_record[amm_addr_num][n] == 0) {
		trap_send(amm_addr_num, 6, E_ALR_PC_CURRENT_LOSS_EVENT_APPEAR, E_ALR_PC_CURRENT_LOSS_EVENT_APPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 1;
		rt_thread_delay(get_ticks_of_ms(50));
	} else if (status == 0 && status_record[amm_addr_num][n] == 1) {
		trap_send(amm_addr_num, 6, E_ALR_PC_CURRENT_LOSS_EVENT_DISAPPEAR, E_ALR_PC_CURRENT_LOSS_EVENT_DISAPPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 0;
		rt_thread_delay(get_ticks_of_ms(50));
	}
	n = 4;
	status = (status_word >>n) & 0x01;
	if (status == 1 && status_record[amm_addr_num][n] == 0) {
		trap_send(amm_addr_num, 6, E_ALR_PC_CURRENT_OVER_EVENT_APPEAR, E_ALR_PC_CURRENT_OVER_EVENT_APPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 1;
		rt_thread_delay(get_ticks_of_ms(50));
	} else if (status == 0 && status_record[amm_addr_num][n] == 1) {
		trap_send(amm_addr_num, 6, E_ALR_PC_CURRENT_OVER_EVENT_DISAPPEAR, E_ALR_PC_CURRENT_OVER_EVENT_DISAPPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 0;
		rt_thread_delay(get_ticks_of_ms(50));
	}
	n = 7;
	status = (status_word >>n) & 0x01;
	if (status == 1 && status_record[amm_addr_num][n] == 0) {
		trap_send(amm_addr_num, 6, E_ALR_PC_PHASE_BREAK_EVENT_APPEAR, E_ALR_PC_PHASE_BREAK_EVENT_APPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 1;
		rt_thread_delay(get_ticks_of_ms(50));
	} else if (status == 0 && status_record[amm_addr_num][n] == 1) {
		trap_send(amm_addr_num, 6, E_ALR_PC_PHASE_BREAK_EVENT_DISAPPEAR	, E_ALR_PC_PHASE_BREAK_EVENT_DISAPPEAR	, FLAG_EM);
		status_record[amm_addr_num][n] = 0;
		rt_thread_delay(get_ticks_of_ms(50));
	}
	n = 8;
	status = (status_word >>n) & 0x01;
	if (status == 1 && status_record[amm_addr_num][n] == 0) {
		trap_send(amm_addr_num, 6, E_ALR_PC_CURRENT_BREAK_EVENT_APPEAR, E_ALR_PC_CURRENT_BREAK_EVENT_APPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 1;
		rt_thread_delay(get_ticks_of_ms(50));
	} else if (status == 0 && status_record[amm_addr_num][n] == 1) {
		trap_send(amm_addr_num, 6, E_ALR_PC_CURRENT_BREAK_EVENT_DISAPPEAR, E_ALR_PC_CURRENT_BREAK_EVENT_DISAPPEAR, FLAG_EM);
		status_record[amm_addr_num][n] = 0;
		rt_thread_delay(get_ticks_of_ms(50));
	}

	return RT_EOK;
}

/**********************************************************************************
* Function	 : 	static rt_err_t output645_07event_data_flag(enum ammeter_event_cmd cmd, rt_uint32_t *output_data_flag)
* Description: 	根据输入的事件命令，输出645-2007中相对应的数据标识
* Arguments	 : 	（1）cmd：输入的事件命令；（2）data_flag：输出的数据标识；
* Return	 :	RT_EOK
*************************************************************************************/
static rt_err_t output645_07event_data_flag(enum ammeter_event_cmd cmd, rt_uint32_t *output_data_flag)
{
	if (output_data_flag == NULL) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	switch (cmd) {
	case AMM_EVENT_LOSE_VOLTAGE:	/* ABC相失压总次数，总累计时间 */
		*output_data_flag = 0x03010000;
		break;
	case AMM_EVENT_A_LOSE_VOLTAGE:	/* A相失压记录 */
		*output_data_flag = 0x03010101;
		break;
	case AMM_EVENT_B_LOSE_VOLTAGE:	/* B相失压记录 */
		*output_data_flag = 0x03010201;
		break;
	case AMM_EVENT_C_LOSE_VOLTAGE:	/* C相失压记录 */
		*output_data_flag = 0x03010301;
		break;
	case AMM_EVENT_OWE_VOLTAGE:		/* ABC相欠压总次数，总累计时间 */
		*output_data_flag = 0x03020000;
		break;
	case AMM_EVENT_A_OWE_VOLTAGE:	/* A相欠压记录 */
		*output_data_flag = 0x03020101;
		break;
	case AMM_EVENT_B_OWE_VOLTAGE:	/* B相欠压记录  */
		*output_data_flag = 0x03020201;
		break;
	case AMM_EVENT_C_OWE_VOLTAGE:	/* C相欠压记录  */
		*output_data_flag = 0x03020301;
		break;
	case AMM_EVENT_OVER_VOLTAGE:	/* ABC相过压总次数，总累计时间 */
		*output_data_flag = 0x03030000;
		break;
	case AMM_EVENT_A_OVER_VOLTAGE:	/* A相过压记录  */
		*output_data_flag = 0x03030101;
		break;
	case AMM_EVENT_B_OVER_VOLTAGE:	/* B相过压记录 */
		*output_data_flag = 0x03030201;
		break;
	case AMM_EVENT_C_OVER_VOLTAGE:	/* C相过压记录 */
		*output_data_flag = 0x03030301;
		break;
	case AMM_EVENT_BROKEN_PHASE:	/* ABC相断相总次数，总累计时间 */
		*output_data_flag = 0x03040000;
		break;
	case AMM_EVENT_A_BROKEN_PHASE:	/* A相断相记录  */
		*output_data_flag = 0x03040101;
		break;
	case AMM_EVENT_B_BROKEN_PHASE:	/* B相断相记录  */
		*output_data_flag = 0x03040201;
		break;
	case AMM_EVENT_C_BROKEN_PHASE:	/* C相断相记录  */
		*output_data_flag = 0x03040301;
		break;
	case AMM_EVENT_LOSE_CURRENT:
		*output_data_flag = 0x030B0000; /* ABC相失流总次数，总累计时间 */
		break;
	case AMM_EVENT_A_LOSE_CURRENT: /* A相失流记录  */
		*output_data_flag = 0x030B0101;
		break;
	case AMM_EVENT_B_LOSE_CURRENT:	/* B相失流记录  */
		*output_data_flag = 0x030B0201;
		break;
	case AMM_EVENT_C_LOSE_CURRENT:	/* C相失流记录  */
		*output_data_flag = 0x030B0301;
		break;
	case AMM_EVENT_OVER_CURRENT:	/* ABC相过流总次数，总累计时间 */
		*output_data_flag = 0x030C0000;
		break;
	case AMM_EVENT_A_OVER_CURRENT: 	/* A相过流记录 */
		*output_data_flag = 0x030C0101;
		break;
	case AMM_EVENT_B_OVER_CURRENT: 	/* B相过流记录 */
		*output_data_flag = 0x030C0201;
		break;
	case AMM_EVENT_C_OVER_CURRENT: 	/* C相过流记录 */
		*output_data_flag = 0x030C0301;
		break;
	case AMM_EVENT_BROKEN_CURRENT:
		*output_data_flag = 0x030D0000; /* ABC相断流总次数，总累计时间 */
		break;
	case AMM_EVENT_A_BROKEN_CURRENT: /* A相断流记录 */
		*output_data_flag = 0x030D0101;
		break;
	case AMM_EVENT_B_BROKEN_CURRENT: /* B相断流记录 */
		*output_data_flag = 0x030D0201;
		break;
	case AMM_EVENT_C_BROKEN_CURRENT: /* C相断流总次数 */
		*output_data_flag = 0x030D0301;
		break;
	case AMM_EVENT_AMM_RESET: 		/* 电表清零总次数 */
		*output_data_flag = 0x03300100;
		break;
	case AMM_EVENT_AMMETER_RESET:  /* 电表清零记录 */
		*output_data_flag = 0x03300101;
		break;
	case AMM_EVENT_REQUIRED_RESET:	/* 需量清零总次数 */
		*output_data_flag = 0x03300200;
		break;
	case AMM_EVENT_NEED_RESET:/* 需量清零记录 */
		*output_data_flag = 0x03300201;
		break;
	case AMM_EVENT_CALIBRATION_TIME:	/* 校时总次数 */
		*output_data_flag = 0x03300400;
		break;
	case AMM_EVENT_TIMING_RECORD:	/* 校时记录 */
		*output_data_flag = 0x03300401;
		break;
	case AMM_EVENT_PROGRAMMING:		/* 编程总次数 */
		*output_data_flag = 0x03300000;
		break;
	case AMM_EVENT_PROGRAM_RECORD:/* 编程记录 */
		*output_data_flag = 0x03300001;
		break;
	case AMM_EVENT_VOLTAGE_INVERSE_PHASE:	/* 电压逆相序总次数，总累计时间 */
		*output_data_flag = 0x03070000;
		break;
	case AMM_EVENT_VOLTAGE_ANTI_PHASE_RECORD:/* 电压逆相序记录  */
		*output_data_flag = 0x03070001;
		break;
	case AMM_EVENT_CURRENT_INVERSE_PHASE:	/* 电流逆相序总次数，总累计时间 */
		*output_data_flag = 0x03080000;
		break;
	case AMM_EVENT_CURRENT_ANTI_PHASE_RECORD:/* 电流逆相序记录 */
		*output_data_flag = 0x03080001;
		break;

	default:
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	return RT_EOK;
}

/**********************************************************************************
* Function	 : 	static rt_err_t output645_07event_cmd_data_len(rt_uint32_t input_data_flag, rt_uint8_t *input_data, rt_uint32_t input_data_len, enum ammeter_event_cmd *output_cmd,
* 				rt_uint8_t *output_data, rt_uint32_t *output_data_len)
* Description: 	根据输入的645-2007数据标识、数据和数据长度，输出相应的事件命令，裁剪后的数据和数据长度
* Arguments	 : 	（1）input_data_flag：输入的数据标识；（2）input_data：输入的数据；（3）input_data_len：输入的数据长度；
* 				（4）output_cmd：输出数据标识对应的事件命令；（5）output_data：输出裁剪后的数据；（6）output_data_len:输出数据长度
* Return	 :	RT_EOK
*************************************************************************************/
static rt_err_t output645_07event_cmd_data_len(rt_uint32_t input_data_flag, rt_uint8_t *input_data, rt_uint32_t input_data_len, enum ammeter_event_cmd *output_cmd, rt_uint8_t *output_data, rt_uint32_t *output_data_len)
{
	rt_uint32_t len = 0;
	rt_uint8_t buf[256] = {'\0'};

	if (input_data == NULL || output_cmd == NULL || output_data == NULL || output_data_len == NULL) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	switch (input_data_flag) {
		case 0x03010:
			*output_cmd = AMM_EVENT_A_LOSE_VOLTAGE;	/* A相失压记录 */
			break;
		case 0x03010201:
			*output_cmd = AMM_EVENT_B_LOSE_VOLTAGE;	/* B相失压记录 */
			break;
		case 0x03010301:
			*output_cmd = AMM_EVENT_C_LOSE_VOLTAGE;	/* C相失压记录 */
			break;
		case 0x03020101:
			*output_cmd = AMM_EVENT_A_OWE_VOLTAGE;	/* A相欠压记录 */
			break;
		case 0x03020201:
			*output_cmd = AMM_EVENT_B_OWE_VOLTAGE;	/* B相欠压记录  */
			break;
		case 0x03020301:
			*output_cmd = AMM_EVENT_C_OWE_VOLTAGE;	/* C相欠压记录  */
			break;
		case 0x03030101:
			*output_cmd = AMM_EVENT_A_OVER_VOLTAGE;	/* A相过压记录  */
			break;
		case 0x03030201:
			*output_cmd = AMM_EVENT_B_OVER_VOLTAGE;	/* B相过压记录 */
			break;
		case 0x03030301:
			*output_cmd = AMM_EVENT_C_OVER_VOLTAGE;	/* C相过压记录 */
			break;
		case 0x03040101:
			*output_cmd = AMM_EVENT_A_BROKEN_PHASE;	/* A相断相记录  */
			break;
		case 0x03040201:
			*output_cmd = AMM_EVENT_B_BROKEN_PHASE;	/* B相断相记录  */
			break;
		case 0x03040301:
			*output_cmd = AMM_EVENT_C_BROKEN_PHASE;	/* C相断相记录  */
			break;

		default:
			*output_cmd = AMM_EVENT_NOTHING;	/* 没有事件发生 */
			break;
		}

		if (*output_cmd != AMM_EVENT_NOTHING) {
			/* 发生时刻6bytes，结束时刻6bytes */
			rt_memcpy(buf, input_data, 12);
			/* 失压时刻电压2bytes，失压时刻电流3bytes，
			失压时刻有功功率3bytes，失压时刻无功功率3bytes，
			失压时刻功率因数2bytes， */
			rt_memcpy(buf + 12, input_data + 44, 13);
			/* 失压期间安时数4bytes */
			rt_memcpy(buf + 25, input_data + input_data_len - 12, 4);
			len = 29;

			rt_memcpy(output_data, buf, len);
			*output_data_len = len;

			return RT_EOK;
		}

	switch (input_data_flag) {
	case 0x030B0101:
		*output_cmd = AMM_EVENT_A_LOSE_CURRENT; /* A相失流记录  */
		break;
	case 0x030B0201:
		*output_cmd = AMM_EVENT_B_LOSE_CURRENT;	/* B相失流记录  */
		break;
	case 0x030B0301:
		*output_cmd = AMM_EVENT_C_LOSE_CURRENT;	/* C相失流记录  */
		break;
	case 0x030C0101:
		*output_cmd = AMM_EVENT_A_OVER_CURRENT; 	/* A相过流记录 */
		break;
	case 0x030C0201:
		*output_cmd = AMM_EVENT_B_OVER_CURRENT; 	/* B相过流记录 */
		break;
	case 0x030C0301:
		*output_cmd = AMM_EVENT_C_OVER_CURRENT; 	/* C相过流记录 */
		break;
	case 0x030D0101:
		*output_cmd = AMM_EVENT_A_BROKEN_CURRENT; /* A相断流记录 */
		break;
	case 0x030D0201:
		*output_cmd = AMM_EVENT_B_BROKEN_CURRENT; /* B相断流记录 */
		break;
	case 0x030D0301:
		*output_cmd = AMM_EVENT_C_BROKEN_CURRENT; /* C相断流总次数 */
		break;
	default:
		*output_cmd = AMM_EVENT_NOTHING;	/* 没有事件发生 */
		break;
	}

	if (*output_cmd != AMM_EVENT_NOTHING) {
		/* 发生时刻6bytes，结束时刻6bytes */
		rt_memcpy(buf, input_data, 12);
		/* 失压时刻电压2bytes，失压时刻电流3bytes，
		失压时刻有功功率3bytes，失压时刻无功功率3bytes，
		失压时刻功率因数2bytes， */
		rt_memcpy(buf + 12, input_data + 44, 13);
		len = 25;

		rt_memcpy(output_data, buf, len);
		*output_data_len = len;

		return RT_EOK;
	}

	switch (input_data_flag) {
	case 0x03300101:
		*output_cmd = AMM_EVENT_AMMETER_RESET;  /* 电表清零记录 */
		rt_memcpy(buf, input_data, input_data_len);
		len = input_data_len;
		break;
	case 0x03300201:
		*output_cmd = AMM_EVENT_NEED_RESET;/* 需量清零记录 */
		/* 需量清零记录， 返回10bytes数据:发生时刻6bytes,操作者代码4bytes */
		rt_memcpy(buf, input_data, 10);
		len = 10;
		break;
	case 0x03300401:
		*output_cmd = AMM_EVENT_TIMING_RECORD;/* 校时记录 */
		/* 校时记录， 返回16bytes数据 :操作者代码4bytes,校时前时间6bytes,校时后时间6bytes */
		rt_memcpy(buf, input_data, input_data_len);
		len = input_data_len;
		break;
	case 0x03300001:
		*output_cmd = AMM_EVENT_PROGRAM_RECORD;/* 编程记录 */
		/* 编程记录， 返回50bytes数据:发生时刻6bytes,操作者代码4bytes,编程的前10个数据标识码(不足补FFFFFFFFH) 4×10bytes */
		rt_memcpy(buf, input_data, input_data_len);
		len = input_data_len;
		break;
	case 0x03070001:
		*output_cmd = AMM_EVENT_VOLTAGE_ANTI_PHASE_RECORD;/* 电压逆相序记录  */
		/* 电压逆相序记录， 返回12bytes数据:发生时刻6bytes,结束时刻6bytes */
		rt_memcpy(buf, input_data, 12);
		len = 12;
		break;
	case 0x03080001:
		*output_cmd = AMM_EVENT_CURRENT_ANTI_PHASE_RECORD;/* 电流逆相序记录 */
		/* 电流逆相序记录， 返回12bytes数据:发生时刻6bytes,结束时刻6bytes */
		rt_memcpy(buf, input_data, 12);
		len = 12;
		break;

	default:
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	rt_memcpy(output_data, buf, len);
	*output_data_len = len;

	return RT_EOK;
}

/**********************************************************************************
* Function	 : 	static enum frame_error_e get_data_from_edmi(rt_uint8_t *addr, rt_uint32_t data_flag, rt_uint8_t *output_data, rt_uint32_t *output_data_len, enum ammeter_uart_e port)
* Description: 	读取EDMI规约电表的数据
* Arguments	 : 	（1）addr：输入的电表的地址；（2）data_flag：输入的数据标识；（3）output_data：传出的数据；（4）output_data_len：传出的数据长度；（5）port：连接电表所用的端口
* Return	 :	enum frame_error_e
*************************************************************************************/
static enum frame_error_e get_data_from_edmi(rt_uint8_t *addr, rt_uint32_t data_flag, rt_uint8_t *output_data, rt_uint32_t *output_data_len, enum ammeter_uart_e port)
{
	enum frame_error_e result = FRAME_E_ERROR;
	struct frame_edmi_param send_frame_data;
	struct frame_edmi_param recv_frame_data;

	if (addr == NULL || output_data == NULL || output_data_len == NULL) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	rt_memset(&send_frame_data, 0x00, sizeof(struct send_frame_param));
	rt_memset(&recv_frame_data, 0x00, sizeof(struct recv_frame_param));

	rt_memcpy(send_frame_data.dest_addr, addr, 6);
	send_frame_data.ctrl = EDMI_CMD_READ_REGISTER;
	send_frame_data.data_flag = data_flag;
	recv_frame_data.data = output_data;

	result = transmit_ammeter_edmi_data(&send_frame_data, &recv_frame_data, port);
	if (result == FRAME_E_OK) {
		*output_data_len = recv_frame_data.data_len;
	} else {
		ERROR_PRINTF(("ERR: %s(), line(%d), %x\n", __FUNCTION__, __LINE__, data_flag));
		return result;
	}

	return FRAME_E_OK;
}
#if 0
/**********************************************************************************
* Function	 : 	static enum frame_error_e amemter_edmi_login(rt_uint8_t *addr, frame_edmi_ctrl_e ctrl, enum ammeter_uart_e port)
* Description: 	登陆ＥＤＭＩ电表
* Arguments	 : 	（1）addr：输入的电表的地址；（2）ctrl：输入的登陆命令；（３）port：连接电表所用的端口
* Return	 :	enum frame_error_e
*************************************************************************************/
static enum frame_error_e amemter_edmi_login(rt_uint8_t *addr, frame_edmi_ctrl_e ctrl, enum ammeter_uart_e port)
{
	rt_uint8_t buf[8] = {'\0'};
	enum frame_error_e result = FRAME_E_ERROR;
	struct frame_edmi_param send_frame_data;
	struct frame_edmi_param recv_frame_data;

	if (addr == NULL ) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return FRAME_E_ERROR;
	}

	rt_memset(&send_frame_data, 0x00, sizeof(struct send_frame_param));
	rt_memset(&recv_frame_data, 0x00, sizeof(struct recv_frame_param));

	rt_memcpy(send_frame_data.dest_addr, addr, 6);
	send_frame_data.ctrl = ctrl;
	recv_frame_data.data = buf;

	result = transmit_ammeter_edmi_data(&send_frame_data, &recv_frame_data, port);
	if (result != FRAME_E_OK) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return result;
	}

	return FRAME_E_OK;
}
#endif
/**********************************************************************************
* Function	 : 	static rt_err_t output_edmi_data_flag(enum ammeter_style style, enum ammeter_cmd_e cmd, rt_uint32_t *output_data_flag, enum data_type *output_data_type, rt_int32_t *decimals_len)
* Description: 	根据输入的读取电能量的命令，输出ＥＤＭＩ规约对应的数据标识、数据类型和数据长度
* Arguments	 : 	（1）style:输入的电表类型；（2）cmd：输入的读表命令；（3）output_data_flag：输出对应的数据标识；（4）output_data_type：输出对应的数据类型；（5）output_data_len：输出对应数据的数据长度
* Return	 :	RT_EOK
*************************************************************************************/
static rt_err_t output_edmi_data_flag(enum ammeter_style style, enum ammeter_cmd_e cmd, rt_uint32_t *output_data_flag, enum data_type *output_data_type, rt_int32_t *decimals_len)
{
	if (output_data_flag == NULL || output_data_type == NULL || decimals_len == NULL) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	rt_uint8_t flag = 0;

	switch (cmd) {
	case AC_POSITIVE_ACTIVE_POWER:			/* 正向有功电能, 返回4bytes数据 */
		*output_data_flag = 0x0169;
		*decimals_len = -1;
		break;
	case AC_POSITIVE_ACTIVE_RATE1_POWER:  	/* 正向有功费率1电能, 返回4bytes数据 */
		*output_data_flag = 0x0160;
		*decimals_len = -1;
		break;
	case AC_POSITIVE_ACTIVE_RATE2_POWER:  	/* 正向有功费率2电能, 返回4bytes数据 */
		*output_data_flag = 0x0161;
		*decimals_len = -1;
		break;
	case AC_POSITIVE_ACTIVE_RATE3_POWER:  	/* 正向有功费率3电能, 返回4bytes数据 */
		*output_data_flag = 0x0162;
		*decimals_len = -1;
		break;
	case AC_POSITIVE_ACTIVE_RATE4_POWER:  	/* 正向有功费率4电能, 返回4bytes数据 */
		*output_data_flag = 0x0163;
		*decimals_len = -1;
		break;

	case AC_REPOSITIVE_ACTIVE_POWER: 	/* 反向有功电能, 返回4bytes数据 */
		*output_data_flag = 0x0069;
		*decimals_len = -1;
		break;
	case AC_REPOSITIVE_ACTIVE_RATE1_POWER: 	/* 反向有功费率1电能, 返回4bytes数据 */
		*output_data_flag = 0x0060;
		*decimals_len = -1;
		break;
	case AC_REPOSITIVE_ACTIVE_RATE2_POWER: 	/* 反向有功费率2电能, 返回4bytes数据 */
		*output_data_flag = 0x0061;
		*decimals_len = -1;
		break;
	case AC_REPOSITIVE_ACTIVE_RATE3_POWER: 	/* 反向有功费率3电能, 返回4bytes数据 */
		*output_data_flag = 0x0062;
		*decimals_len = -1;
		break;
	case AC_REPOSITIVE_ACTIVE_RATE4_POWER: 	/* 反向有功费率4电能, 返回4bytes数据 */
		*output_data_flag = 0x0063;
		*decimals_len = -1;
		break;

	case AC_POSITIVE_WATTLESS_POWER: 	/* 正向无功电能, 返回4bytes数据 */
		*output_data_flag = 0x0369;
		*decimals_len = -1;
		break;
	case AC_POSITIVE_WATTLESS_RATE1_POWER: 	/* 正向无功费率1电能, 返回4bytes数据 */
		*output_data_flag = 0x0360;
		*decimals_len = -1;
			break;
	case AC_POSITIVE_WATTLESS_RATE2_POWER: 	/* 正向无功费率2电能, 返回4bytes数据 */
		*output_data_flag = 0x0361;
		*decimals_len = -1;
			break;
	case AC_POSITIVE_WATTLESS_RATE3_POWER: 	/* 正向无功费率3电能, 返回4bytes数据 */
		*output_data_flag = 0x0362;
		*decimals_len = -1;
			break;
	case AC_POSITIVE_WATTLESS_RATE4_POWER: 	/* 正向无功费率4电能, 返回4bytes数据 */
		*output_data_flag = 0x0363;
		*decimals_len = -1;
			break;

	case AC_REPOSITIVE_WATTLESS_POWER: 	/* 反向无功电能, 返回4bytes数据 */
		*output_data_flag = 0x0269;
		*decimals_len = -1;
		break;
	case AC_REPOSITIVE_WATTLESS_RATE1_POWER: /* 反向无功费率1电能, 返回4bytes数据 */
		*output_data_flag = 0x0260;
		*decimals_len = -1;
		break;
	case AC_REPOSITIVE_WATTLESS_RATE2_POWER: /* 反向无功费率2电能, 返回4bytes数据 */
		*output_data_flag = 0x0261;
		*decimals_len = -1;
		break;
	case AC_REPOSITIVE_WATTLESS_RATE3_POWER: /* 反向无功费率3电能, 返回4bytes数据 */
		*output_data_flag = 0x0262;
		*decimals_len = -1;
		break;
	case AC_REPOSITIVE_WATTLESS_RATE4_POWER: /* 反向无功费率4电能, 返回4bytes数据 */
		*output_data_flag = 0x0263;
		*decimals_len = -1;
		break;

	case AC_A_VOLTAGE: 					/* A相电压, 返回2bytes数据 */
		*output_data_flag = 0xE000;
		*decimals_len = 1;
		break;
	case AC_B_VOLTAGE: 					/* B相电压, 返回2bytes数据 */
		*output_data_flag = 0xE001;
		*decimals_len = 1;
		break;
	case AC_C_VOLTAGE: 					/* C相电压, 返回2bytes数据 */
		*output_data_flag = 0xE002;
		*decimals_len = 1;
		break;

	case AC_A_CURRENT: 					/* A相电流, 返回2bytes数据 */
		*output_data_flag = 0xE010;
		*decimals_len = 3;
		break;
	case AC_B_CURRENT: 					/* B相电流, 返回2bytes数据 */
		*output_data_flag = 0xE011;
		*decimals_len = 3;
		break;
	case AC_C_CURRENT: 					/* C相电流, 返回2bytes数据 */
		*output_data_flag = 0xE012;
		*decimals_len = 3;
		break;

	case AC_INSTANT_ACTIVE_POWER: 		/* 瞬时有功功率, 返回3bytes数据 */
		*output_data_flag = 0xE033;
		*decimals_len = 1;
		break;
	case AC_A_ACTIVE_POWER: 				/* A相有功功率, 返回3bytes数据 */
		*output_data_flag = 0xE030;
		*decimals_len = 1;
		break;
	case AC_B_ACTIVE_POWER: 				/* B相有功功率, 返回3bytes数据 */
		*output_data_flag = 0xE031;
		*decimals_len = 1;
		break;
	case AC_C_ACTIVE_POWER: 				/* C相有功功率, 返回3bytes数据 */
		*output_data_flag = 0xE032;
		*decimals_len = 1;
		break;

	case AC_INSTANT_REACTIVE_POWER: 		/* 瞬时无功功率, 返回2bytes数据 */
		*output_data_flag = 0xE043;
		*decimals_len = 1;
		break;
	case AC_A_REACTIVE_POWER: 			/* A相无功功率, 返回2bytes数据 */
		*output_data_flag = 0xE040;
		*decimals_len = 1;
		break;
	case AC_B_REACTIVE_POWER: 			/* B相无功功率, 返回2bytes数据 */
		*output_data_flag = 0xE041;
		*decimals_len = 1;
		break;
	case AC_C_REACTIVE_POWER: 			/* C相无功功率, 返回2bytes数据 */
		*output_data_flag = 0xE042;
		*decimals_len = 1;
		break;

	case AC_TOTAL_POWER_FACTOR: 			/* 总功率因数, 返回2bytes数据 */
		*output_data_flag = 0xE026;
		*decimals_len = 3;
		break;
	case AC_A_POWER_FACTOR: 				/* A相功率因数, 返回2bytes数据 */
		*output_data_flag = 0xE026;
		*decimals_len = 3;
		break;
	case AC_B_POWER_FACTOR: 				/* B相功率因数, 返回2bytes数据 */
		*output_data_flag = 0xE026;
		*decimals_len = 3;
		break;
	case AC_C_POWER_FACTOR: 				/* C相功率因数, 返回2bytes数据 */
		*output_data_flag = 0xE026;
		*decimals_len = 3;
		break;

	case AC_TOTAL_APPARENT_POWER:		/* 瞬时总视在功率, 返回3bytes数据 */
		*output_data_flag = 0XE053;
		*decimals_len = 1;
		break;
	case AC_A_APPARENT_POWER:			/* 瞬时A相视在功率, 返回3bytes数据 */
		*output_data_flag = 0XE050;
		*decimals_len = 1;
		break;
	case AC_B_APPARENT_POWER:			/* 瞬时B相视在功率, 返回3bytes数据 */
		*output_data_flag = 0XE051;
		*decimals_len = 1;
		break;
	case AC_C_APPARENT_POWER:			/* 瞬时C相视在功率, 返回3bytes数据 */
		*output_data_flag = 0XE052;
		*decimals_len = 1;
		break;
	case AC_POWER_FREQUENCY:			/* 电网频率, 返回2bytes数据*/
		*output_data_flag = 0XE060;
		*decimals_len = 2;
		break;
	case AC_POWER_CONSTANT:				/* 电表常数(有功), 返回4bytes数据  */
		if (style == AM_MK6E || style == AM_MK6) {
			*output_data_flag = 0XF920;
		} else if (style == AM_MK3) {
			*output_data_flag = 0XF24A;
		} else {
			*output_data_flag = 0XF920;
		}
		*decimals_len = 0;
		break;
	default:
		flag = 1;
		break;
	}

	if (flag == 0) {
		*output_data_type = DATA_TYPE_SINGLE_FLOAT;
		return RT_EOK;
	}

	switch (cmd) {
	case AC_DATE_AND_WEEK:				/* 日期及周次,年月日星期,返回4bytes数据  */
		*output_data_flag = 0XF010;
		*output_data_type = DATA_TYPE_DATE;
		break;
	case AC_AMMETER_TIME:				/* 时间,时分秒,返回3bytes数据 */
		*output_data_flag = 0XF011;
		*output_data_type = DATA_TYPE_TIME;
		break;
	case AC_GET_AMMETR_ADDR: 			/* 电表的表号, 返回6bytes数据, 高位在前 */
		*output_data_flag = 0XF002;
		*output_data_type = DATA_TYPE_STRING;
		break;
	default:
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	return RT_EOK;
}
/**********************************************************************************
* Function	 : 	static rt_err_t output_edmi_maxneed_data_flag(enum ammeter_maxneed_cmd cmd, rt_uint32_t *output_maxneed_data_flag, rt_uint32_t *output_time_data_flag)
* Description: 	根据输入的读取最大需量的命令，输出ＥＤＭＩ规约对应的数据标识
* Arguments	 : 	（1）cmd：输入的读表命令；（2）output_maxneed_data_flag：输出对应的最大需量得数据标识；（３）output_time_data_flag：输出对应的最大需量发生时间的数据标识
* Return	 :	RT_EOK
*************************************************************************************/
static rt_err_t output_edmi_maxneed_data_flag(enum ammeter_maxneed_cmd cmd, rt_uint32_t *output_maxneed_data_flag, rt_uint32_t *decimals_len, rt_uint32_t *output_time_data_flag)
{
	if (output_maxneed_data_flag == NULL || output_time_data_flag == NULL) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	switch (cmd) {
	case CMD_MAXNEED_TOTAL_POSITIVE_ACTIVE:     /* (当前)正向有功总最大需量及发生时间, 返回8bytes数据  */
		*output_maxneed_data_flag = 0x1109;
		*output_time_data_flag = 0x8109;
		break;
	case CMD_MAXNEED_RATE1_POSITIVE_ACTIVE:     /* (当前)正向有功费率1最大需量及发生时间, 返回8bytes数据  */
		*output_maxneed_data_flag = 0x1100;
		*output_time_data_flag = 0x8100;
		break;
	case CMD_MAXNEED_RATE2_POSITIVE_ACTIVE:     /* (当前)正向有功费率2最大需量及发生时间, 返回8bytes数据  */
		*output_maxneed_data_flag = 0x1101;
		*output_time_data_flag = 0x8101;
		break;
	case CMD_MAXNEED_RATE3_POSITIVE_ACTIVE:     /* (当前)正向有功费率3最大需量及发生时间, 返回8bytes数据  */
		*output_maxneed_data_flag = 0x1102;
		*output_time_data_flag = 0x8102;
		break;
	case CMD_MAXNEED_RATE4_POSITIVE_ACTIVE:     /* (当前)正向有功费率4最大需量及发生时间, 返回8bytes数据  */
		*output_maxneed_data_flag = 0x1103;
		*output_time_data_flag = 0x8103;
		break;
	case CMD_MAXNEED_TOTAL_REPOSITIVE_ACTIVE:     /* (当前)反向有功总最大需量及发生时间, 返回8bytes数据  */
		*output_maxneed_data_flag = 0x1009;
		*output_time_data_flag = 0x8009;
		break;
	case CMD_MAXNEED_RATE1_REPOSITIVE_ACTIVE:     /* (当前)反向有功费率1最大需量及发生时间, 返回8bytes数据  */
		*output_maxneed_data_flag = 0x1000;
		*output_time_data_flag = 0x8000;
		break;
	case CMD_MAXNEED_RATE2_REPOSITIVE_ACTIVE:     /* (当前)反向有功费率2最大需量及发生时间, 返回8bytes数据  */
		*output_maxneed_data_flag = 0x1001;
		*output_time_data_flag = 0x8001;
		break;
	case CMD_MAXNEED_RATE3_REPOSITIVE_ACTIVE:     /* (当前)反向有功费率3最大需量及发生时间, 返回8bytes数据 */
		*output_maxneed_data_flag = 0x1002;
		*output_time_data_flag = 0x8002;
		break;
	case CMD_MAXNEED_RATE4_REPOSITIVE_ACTIVE:     /* (当前)反向有功费率4最大需量及发生时间, 返回8bytes数据 */
		*output_maxneed_data_flag = 0x1003;
		*output_time_data_flag = 0x8003;
		break;
	default:
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	*decimals_len = 4;

	return RT_EOK;
}
/**********************************************************************************
* Function	 : 	static rt_err_t string_change_to_bcd_hex(rt_uint8_t *input_data, rt_uint32_t input_data_len, rt_uint8_t *output_data, rt_uint32_t *output_data_len)
* Description: 	把字符串数据转化为１６进制的BCD码格式数据
* Arguments	 : 	（1）input_data：输入的字符串数据；（2）input_data_len：输入的字符串数据的长度；（３）output_data：输出的16进制BCD码数据；（4）output_data_len：输出的16进制ＢＣＤ码数据的长度
* Return	 :	RT_EOK
*************************************************************************************/
static rt_err_t string_change_to_bcd_hex(rt_uint8_t *input_data, rt_uint32_t input_data_len, rt_uint8_t *output_data, rt_uint32_t *output_data_len)
{
	rt_uint32_t i = 0;
	rt_uint32_t n = 0;

	if (input_data == RT_NULL || output_data == RT_NULL || output_data_len == RT_NULL) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	if (*(input_data + input_data_len - 1) != 0x00) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	for (i = 0; i < input_data_len - 2; i+=2) {
		*(output_data + n) = *(input_data + input_data_len - 2 - i) - 0x30;
		*(output_data + n) |= (*(input_data + input_data_len - 3 - i) - 0x30) << 4;
		n++;
	}

	if ((input_data_len - 1) % 2 == 1) {
		*(output_data + n) = *input_data - 0x30;
		n++;
	}

	*output_data_len = n;

	return RT_EOK;
}
/**********************************************************************************
* Function	 : 	static rt_err_t float_change_to_bcd_hex(enum ammeter_cmd_e cmd, rt_uint8_t *input_data, rt_uint32_t input_data_len, rt_int32_t input_decimals_len, rt_uint8_t *output_data, rt_uint32_t *output_data_len)
* Description: 	把浮点数数据转化为１６进制的BCD码格式数据
* Arguments	 : 	（１）cmd：输入读取电表数据的命令；（２）input_data：输入的浮点数数据；（３）input_data_len：输入的浮点数数据的长度；（４）input_decimals_len:有正负之分，当为正时表示取输出数据的几位小数，当为负时表示消除输出数据整数的最后几位；（5） output_data：输出的16进制BCD码数据；
* 				（6）output_data_len：输出的16进制ＢＣＤ码数据的长度
* Return	 :	RT_EOK
*************************************************************************************/
static rt_err_t float_change_to_bcd_hex(enum ammeter_cmd_e cmd, rt_uint8_t *input_data,
		rt_uint32_t input_data_len, rt_int32_t input_decimals_len,
		rt_uint8_t *output_data, rt_uint32_t *output_data_len)
{
	rt_uint32_t i = 0, n = 1;
	rt_uint8_t sign = 0;
	float tmp = 0;
	rt_int32_t data = 0;
	rt_uint8_t buf[4] = {'\0'};

	if (input_data == RT_NULL || output_data == RT_NULL || output_data_len == RT_NULL) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	if (input_data_len != 4 || input_decimals_len < -4 || input_decimals_len > 4) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	rt_memset(output_data, 0x00, 4);

	buf[0] = input_data[3];
	buf[1] = input_data[2];
	buf[2] = input_data[1];
	buf[3] = input_data[0];
	rt_memcpy(&tmp, buf, 4);

	if (tmp < 0) {
		sign = 1;
		tmp = -tmp;
	}

	if (cmd == AC_POWER_CONSTANT) {
		tmp = 1000 / tmp;
	}

	if (input_decimals_len > 0) {
		for (i = 0; i < input_decimals_len; i++) {
			n *= 10;
		}
		data = tmp * n;
	} else if (input_decimals_len < 0){
		for (i = 0; i < -input_decimals_len; i++) {
			n *= 10;
		}
		data = tmp / n;
	} else {
		data = tmp;
	}

	i = 0;
	while (data > 0) {
		*(output_data + i) = data % 10;
		data /= 10;
		*(output_data + i) |= (data  % 10) << 4;
		data /= 10;
		i++;
	}

	if (sign > 0) {
		n = 0;
		n = choose_the_sign_bit(cmd);
		if (n != 0) {
		 *(output_data + n - 1) += 0x80;
		}
	}

	*output_data_len = 4;

	return RT_EOK;
}

/**********************************************************************************
* Function	 : 	static rt_uint8_t choose_the_sign_bit(enum ammeter_cmd_e cmd)
* Description: 	根据输入的命令，判断数据的符号位在第几个字节
* Arguments	 : 	（1）cmd：输入的读表命令
* Return	 :	rt_uint8_t
*************************************************************************************/
static rt_uint8_t choose_the_sign_bit(enum ammeter_cmd_e cmd)
{
	switch (cmd) {
	case AC_A_CURRENT: 						/* A相电流, 返回3bytes数据 */
	case AC_B_CURRENT: 						/* B相电流, 返回3bytes数据 */
	case AC_C_CURRENT: 						/* C相电流, 返回3bytes数据 */
	case AC_INSTANT_ACTIVE_POWER: 			/* 瞬时有功功率, 返回3bytes数据 */
	case AC_A_ACTIVE_POWER: 				/* A相有功功率, 返回3bytes数据 */
	case AC_B_ACTIVE_POWER: 				/* B相有功功率, 返回3bytes数据 */
	case AC_C_ACTIVE_POWER: 				/* C相有功功率, 返回3bytes数据 */
	case AC_INSTANT_REACTIVE_POWER: 		/* 瞬时无功功率, 返回3bytes数据 */
	case AC_A_REACTIVE_POWER: 				/* A相无功功率, 返回3bytes数据 */
	case AC_B_REACTIVE_POWER: 				/* B相无功功率, 返回3bytes数据 */
	case AC_C_REACTIVE_POWER: 				/* C相无功功率, 返回3bytes数据 */
		return 3;

	case AC_TOTAL_POWER_FACTOR: 				/* 总功率因数, 返回2bytes数据 */
	case AC_A_POWER_FACTOR: 					/* A相功率因数, 返回2bytes数据 */
	case AC_B_POWER_FACTOR: 					/* B相功率因数, 返回2bytes数据 */
	case AC_C_POWER_FACTOR: 					/* C相功率因数, 返回2bytes数据 */
		return 2;
	default:
		return 0;
	}

	return 0;
}
/**********************************************************************************
* Function	 : 	static rt_err_t integer_change_to_bcd_hex(rt_uint8_t *input_data, rt_uint32_t input_data_len, rt_uint8_t *output_data, rt_uint32_t *output_data_len)
* Description: 	把整数数据转化为１６进制的BCD码格式数据
* Arguments	 : 	（1）input_data：输入的整数数据；（2）input_data_len：输入的整数数据的长度；（３）output_data：输出的16进制BCD码数据；（4）output_data_len：输出的16进制ＢＣＤ码数据的长度
* Return	 :	RT_EOK
*************************************************************************************/
static rt_err_t integer_change_to_bcd_hex(rt_uint8_t *input_data, rt_uint32_t input_data_len, rt_uint8_t *output_data, rt_uint32_t *output_data_len)
{
	rt_uint32_t i = 0;

	if (input_data == RT_NULL || output_data == RT_NULL || output_data_len == RT_NULL) {
		ERROR_PRINTF(("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		return RT_ERROR;
	}

	for (i = 0; i < input_data_len; i++) {
		*(output_data + i) = *(input_data + i) % 10;
		*(output_data + i) |= ((*(input_data + i) / 10) % 10) << 4;
	}

	*output_data_len = input_data_len;

	return RT_EOK;
}



void time_set(rt_uint32_t year, rt_uint32_t month, rt_uint32_t day, rt_uint32_t hour, rt_uint32_t minite)
{
	struct ammeter_time time;

	rt_memset(&time, 0, sizeof(struct ammeter_time));

	time.year = year;
	time.month = month;
	time.day = day;
	time.hour = hour;
	time.minite = minite;
	time.seconds = 0;

	if (RT_EOK != setting_all_ammeter_time(&time, AMMETER_UART1)) {
		printf_syn("ERR: setting all ammeter time error!\n");
	}
}
FINSH_FUNCTION_EXPORT(time_set, "year, month, day, hour, minite");


rt_err_t test_forzen_now(void)
{
	enum frame_error_e result = FRAME_E_ERROR;
	struct ammeter_time time;
	rt_uint8_t buf[12] = {'9','9','9','9','9','9','9','9','9','9','9','9'};

	time.year = 2014;
	time.month = 7;
	time.day = 10;
	time.hour = 9;
	time.minite = 01;
	time.seconds = 01;

	result = setting_ammeter_forzen_data_time(buf, CMD_FREEZING_NOW, &time, AMMETER_UART1);
	if (result == FRAME_E_ERROR) {
		printf_syn("setting ammeter forzen data time ERROR\n");
		return RT_ERROR;
	}

	return RT_EOK;
}

