/***********************************************************
*   函数库说明：系统主函数                                 *
*   版本：    Ver1.0                                       *
*   作者：    zzjjhh250/ZZJJHH250 @ (CopyRight)            *
*   创建日期：08/31/2010                                   *
* -------------------------------------------------------- *
*  [硬件说明]                                              *
*   处理器：    STM32F103VBT6                              *
*   系统时钟：  外部8M/PLL = 72M                           *
* -------------------------------------------------------- *
*  [支 持 库]                                              *
*   支持库名称：PF_Config.h                                *
*   需要版本：  -----                                      *
*   声明库说明：硬件平台配置声明库                         *
*														   *
*   支持库名称：LIB_Config.h                               *
*   需要版本：  -----                                      *
*   声明库说明：常用库配置声明                             *
* -------------------------------------------------------- *
*  [版本更新]                                              *
*   修改：                                                 *
*   修改日期：                                             *
*   版本：                                                 *
* -------------------------------------------------------- *
*  [版本历史]                                              *
*  														   *
* V4.0.0  												   *
* 时间：2011.07.07 早									   *
* 修改情况一览：	1、加放狗命令 ；   					   *
* 				2、将固话零位修改OK 					   *
* 				3、幅值步进分辨率1/15000				   *
* 				4、相位步进分辨率45.5us/364                *
* -------------------------------------------------------- *
*  [使用说明]                                              *
***********************************************************/

/********************
* 头 文 件 配 置 区 *
********************/
#include "..\Source\LIB_Config.h"
#include "..\Source\PF_Config.H"
#include "sys_comm.h"

#include ".\fwlib\stm32f10x.h"
#include "stm32f10x_rcc.h"

#include ".\FWLib\stm32f10x_wwdg.h"
#include "ads8329.h"
#include "board.h"
#include "common.h"
#include "stm32f10x_it.h"
#include ".\fwlib\stm32f10x_exti.h"

//#include "tctrl.h"
struct signal_cfg_param_tbl signal_cfg_param;

static int is_had_enter_set_param_mode = 0;

static void _get_sig_cfg_param(struct signal_cfg_param_tbl *cfg_param, int is_init);
static void init_signal_cfg_param_tbl(struct signal_cfg_param_tbl *cfg_param);
static void read_sig_cfg_param_from_flash(struct signal_cfg_param_tbl *cfg_param);
FLASH_Status write_sig_cfg_param_to_flash(struct signal_cfg_param_tbl *cfg_param);
FLASH_Status write_sig_cfg_param_to_flash_for_init(struct signal_cfg_param_tbl *cfg_param);


extern void usr_cmd_analysis(void);
extern int is_cmd_valid(int cmd);
extern void usr_cmd_proc(int cmd, s32 param);

extern void wwdg_init(void);
void get_threshold_val(void);
extern int get_ver_str(char str[], int len);

extern void send_dev_sn2pc(void);
extern int recv_devsn_frompc(int isstart);
extern int proc_recv_devsn(char *str);

/* 温度传感器的值 */
volatile s32  cpu_temper;
char tx_dev_sn[DEV_SN_LEN_MAX+1];

extern volatile unsigned int enter_set_param_mode_time;
extern volatile int is_turnoff_signal_led;
extern volatile int is_need_write_cfg_data_to_flash;

extern volatile unsigned int tim3_8ms_cnt;
extern volatile unsigned int box_temper_send;

unsigned char tx_dev_sn_bits_5[DEV_SN_5BITS_BUF_LEN];

/* !!!NOTE: isr 中会修改 px_zero_position_value */
void get_sig_cfg_param(int is_init)
{
	_get_sig_cfg_param(&signal_cfg_param, is_init); /* David */

	get_threshold_val();

	if (split_bytes_to_5bits((unsigned char *)(signal_cfg_param.dev_sn), DEV_SN_MODE_LEN,
			tx_dev_sn_bits_5, sizeof(tx_dev_sn_bits_5)-1) < 0) {
		send_data_to_pc('5'<<24 | 'B'<<16 | 'I'<<8 | 'S', 4);
	}
	
	return;
}

/***********************************************************
*   函数说明：系统主函数                                   *
*   输入：    无                                           *
*   输出：    无                                           *
*   调用函数：无                                           *
***********************************************************/

int main(void)
{
	System_Init();	   //与库没有关系 这个是片内设备和片外设备的总初始化

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if USE_STM32_WWDG
	wwdg_init();
#endif

	flush_uart4pc_buf_fn();

	while (1) {
#if USE_STM32_WWDG
		WWDG->CR = WWDG_RELOAD_VALUE; /* 喂狗 */
#endif

		usr_cmd_analysis();

		if(is_had_enter_adjust_value_mode == SELF_LEARN_STDV_END) {
			is_had_enter_adjust_value_mode = 0;
			send_data_to_pc(signal_cfg_param.pa_learn_avg_val, 2);
			send_data_to_pc(signal_cfg_param.pb_learn_avg_val, 2);
			send_data_to_pc(signal_cfg_param.pc_learn_avg_val, 2);
			get_threshold_val();
		}

#if 1
		/* 中断间隔8ms,1hz大约记64，2hz大约32 */
		/* 2Hz */
		if (0 != (timer3_int_cnt & (1<<5))) {
			if (is_send_data())
				on_send_data_led();
			else
				off_send_data_led();
		} else {
			off_send_data_led();
		}

		/* 1Hz */
		if (0 != (timer3_int_cnt & (1<<6))) {
			on_running_led();

			if (is_pa_lost_phase())
				on_pa_losting_phase_led();
			else
				off_pa_losting_phase_led();

			if (is_pb_lost_phase())
				on_pb_losting_phase_led();
			else
				off_pb_losting_phase_led();

			if (is_pc_lost_phase())
				on_pc_losting_phase_led();
			else
				off_pc_losting_phase_led();
		} else {
			off_running_led();

			off_pa_losting_phase_led();
			off_pb_losting_phase_led();
			off_pc_losting_phase_led();
		}
#endif
		//box_tmper_ctrl();
	}//while超级循环

}//main函数的结束

static void _get_sig_cfg_param(struct signal_cfg_param_tbl *cfg_param, int is_init)
{
	read_sig_cfg_param_from_flash(cfg_param);

	if (MAGIC_NUM_OF_CFG_PARAM_TBL != cfg_param->magic_num) {
		init_signal_cfg_param_tbl(cfg_param);
		if (0 != is_init)
			write_sig_cfg_param_to_flash_for_init(cfg_param);  /* 写flash有可能失败 */
		else
			write_sig_cfg_param_to_flash(cfg_param);  /* 写flash有可能失败 */
	}

	return;
}



static void init_signal_cfg_param_tbl(struct signal_cfg_param_tbl *cfg_param)
{
	cfg_param->magic_num        = MAGIC_NUM_OF_CFG_PARAM_TBL;

	cfg_param->temper_pwm 	    = 128;

	cfg_param->pa_learn_avg_val = ORIG_AVERAGE_VALUE;
	cfg_param->pb_learn_avg_val = ORIG_AVERAGE_VALUE;
	cfg_param->pc_learn_avg_val = ORIG_AVERAGE_VALUE;

	cfg_param->px_more_percent_max = 20;
	cfg_param->px_less_percent_max = 20;

	strncpy(cfg_param->dev_sn, DEV_SN_MODE, sizeof(cfg_param->dev_sn));
	
	cfg_param->pad1 = ~0;
	cfg_param->pad  = ~0;

	return;
}

static void read_sig_cfg_param_from_flash(struct signal_cfg_param_tbl *cfg_param)
{
	u32 *ps, *pd;
	int i;

	ps = (u32 *)UART_AMPLIFY_ADDR;
	pd = (u32 *)cfg_param;
	for (i=0; i < (sizeof(struct signal_cfg_param_tbl)/4); i++) {
		*pd++ = *ps++;

	}
}

FLASH_Status write_sig_cfg_param_to_flash(struct signal_cfg_param_tbl *cfg_param)
{
	u32 *ps, *pd, temp;
	int i;
	FLASH_Status status;

#if USE_STM32_WWDG
	WWDG->CR = WWDG_RELOAD_VALUE; /* 喂狗 */
#endif

	FLASH_Unlock();
	/* Clear pending flags (if any) */
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPTERR | FLASH_FLAG_WRPRTERR |
					FLASH_FLAG_PGERR | FLASH_FLAG_BSY);

	status = FLASH_ErasePage(UART_AMPLIFY_ADDR);	  //刷除 数据
#if USE_STM32_WWDG
	WWDG->CR = WWDG_RELOAD_VALUE; /* 喂狗 */
#endif
	if (FLASH_COMPLETE != status)
		while(1); //return status;

	ps = (u32 *)cfg_param;
	pd = (u32 *)UART_AMPLIFY_ADDR;
	for (i=0; i < (sizeof(struct signal_cfg_param_tbl)/4); i++) {
		temp = *ps;
		status = FLASH_ProgramWord((u32)pd , temp);
		send_data_to_pc(temp, 4); /* mark by David */

#if USE_STM32_WWDG
		WWDG->CR = WWDG_RELOAD_VALUE; /* 喂狗 */
#endif

		if (FLASH_COMPLETE != status)
			while(1); //return status;
		++pd;
		++ps;

	}
	FLASH_Lock();

	return FLASH_COMPLETE;
}



FLASH_Status write_sig_cfg_param_to_flash_for_init(struct signal_cfg_param_tbl *cfg_param)
{
	u32 *ps, *pd, temp;
	int i;
	FLASH_Status status;

	FLASH_Unlock();
	/* Clear pending flags (if any) */
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPTERR | FLASH_FLAG_WRPRTERR |
					FLASH_FLAG_PGERR | FLASH_FLAG_BSY);

#if 1
	status = FLASH_ErasePage(UART_AMPLIFY_ADDR);	  //刷除 数据
	if (FLASH_COMPLETE != status)
		while(1); //return status;
#endif
	ps = (u32 *)cfg_param;
	pd = (u32 *)UART_AMPLIFY_ADDR;
	for (i=0; i < (sizeof(struct signal_cfg_param_tbl)/4); i++) {
		temp = *ps;
		status = FLASH_ProgramWord((u32)pd , temp);

		if (FLASH_COMPLETE != status)
			while(1); //return status;
		++pd;
		++ps;

	}
	FLASH_Lock();

	return FLASH_COMPLETE;
}

int is_cmd_valid(int cmd)
{
	switch (cmd) {
	case WRITE_CFG_PARAM2FLASH               :
	case READ_SYS_RUN_TIME                        :
	case READ_TEMP_VALUE                     :
	case READ_VERSION                        :
	case READ_SET_PARAM_VALUE:
	case ENTER_SET_PARAM_MODE                :
	case EXIT_SET_PARAM_MODE                 :
	case SELF_LEARN_STDV_ADC_VALU:
	case SET_PX_ADC_VALU_MAX_PERCENT:
	case SET_PX_ADC_VALU_MIN_PERCENT:
#if 1==DEBUG_CHECKINFO_TRAN
	case SET_DEBUG_PARAM:
	case CLR_DEBUG_PARAM:
#endif
	case READ_DEBUG_PARAM1:
	case WRITE_DEBUG_PARAM1:
	case WRITE_DEBUG_PARAM2:
	case WRITE_DEBUG_PARAM3:
	case WRITE_DEBUG_PARAM4:
	case WRITE_DEBUG_PARAM5:
	case SPC_SET_DEV_SN:
	case SPC_GET_DEV_SN:
		return 1;

	default:
		return 0;
	}

}

volatile unsigned int systime_cnt;
extern void send_sys_time_info2uart(unsigned int sec);
extern volatile int pa_check_bin_info, pb_check_bin_info, pc_check_bin_info;


void get_threshold_val(void)
{
	/*初始比较阈值 不做调试为百分之十浮动 */
	pa_avg_val_max  = (signal_cfg_param.pa_learn_avg_val * (100 + signal_cfg_param.px_more_percent_max)) / 100;
	pb_avg_val_max  = (signal_cfg_param.pb_learn_avg_val * (100 + signal_cfg_param.px_more_percent_max)) / 100;
	pc_avg_val_max  = (signal_cfg_param.pc_learn_avg_val * (100 + signal_cfg_param.px_more_percent_max)) / 100;

	pa_avg_val_min  = (signal_cfg_param.pa_learn_avg_val * (100 - signal_cfg_param.px_less_percent_max)) / 100;
	pb_avg_val_min  = (signal_cfg_param.pb_learn_avg_val * (100 - signal_cfg_param.px_less_percent_max)) / 100;
	pc_avg_val_min  = (signal_cfg_param.pc_learn_avg_val * (100 - signal_cfg_param.px_less_percent_max)) / 100;

	pa_lost_avg_val = (signal_cfg_param.pa_learn_avg_val * 20) / 100;
	pb_lost_avg_val = (signal_cfg_param.pb_learn_avg_val * 20) / 100;
	pc_lost_avg_val = (signal_cfg_param.pc_learn_avg_val * 20) / 100;

	return;
}

#if TEST_TX_POWEROFF

static void start_dma_and_start_uart_gap(void)
{
	volatile int delay_var;

	delay_var = 0;
	delay_var = 1;
	delay_var = 2;
	delay_var = 3;
	delay_var = 4;
	delay_var = 5;
	delay_var = 6;
	delay_var = 7;

	delay_var = 8;
	delay_var = 9;

	return;
}


volatile int send_powerdown_info_flag;
void send_powerdown_info(void)
{
	send_powerdown_info_flag = 1;

	disable_dmax_y(DMA1_Channel4);
	DMA1->IFCR = DMA1_IT_GL4; /* Clear the selected DMA interrupt pending bits */

	uart1_dma_buf[0] = (convert_id2firstbyte(TRMI_MISC_INFO) | TRMIS_TX_POWEROFF);
	DMA1_Channel4->CNDTR = 1; /* Write to DMAy Channelx CNDTR */

	while (!is_usartx_txd_empty(USART1))
		;

	enable_dmax_y(DMA1_Channel4);
	start_dma_and_start_uart_gap();

	while (!is_usartx_txd_empty(USART1))
		;

	send_powerdown_info_flag = 0;

	return;
}
#endif


void usr_cmd_proc(int cmd, s32 param)
{
	int temp;
	char str[12];

	param &= 0xffff;
	temp  =  (cmd << 16) | param;
	if (0==is_had_enter_set_param_mode && 0x90a5a5!=temp) {
		return;
	}

	send_data_to_pc(temp, 3);

	switch(cmd) {
	case WRITE_CFG_PARAM2FLASH: /* 将配置参数写入flash, 需要发送字节序列0x69, 0xaa, 0xaa */
		if (param == 0xAAAA)
			write_sig_cfg_param_to_flash(&signal_cfg_param);
		break;

	case READ_SYS_RUN_TIME: /* 读系统运行时间, 需要发送字节序列0x6a, 0xa5, 0xa5 */
		send_sys_time_info2uart(tim3_8ms_cnt/125);
		break;

	case READ_TEMP_VALUE: /* 读温度传感器的值, 需要发送字节序列0x6b, 0xa5, 0xa5 */
		temp = (param);
		if (temp == 0xA5A5) {
			send_data_to_pc(cpu_temper, 4);
		} else if (temp == 0xA5A4) {
			send_data_to_pc(box_temper_send, 4);
		}
		break;

	case READ_VERSION:  /* 读软件版本号, 需要发送字节序列0x6c, 0xa5, 0xa5 */
		if ((param) == 0xA5A5) {
			get_ver_str(str, sizeof(str));

			temp = *(unsigned int*)str;
			send_data_to_pc_lf(temp, 4);
			temp = *(unsigned int*)(str + 4);
			send_data_to_pc_lf(temp, 4);
			temp = *(unsigned int*)(str + 8);
			send_data_to_pc_lf(temp, 2);
		}
		break;
#if 1
	case SELF_LEARN_STDV_ADC_VALU: //自校准
		if ((param) == 0xA6A6) {
			pa_learn_sample_sum = 0;
			pb_learn_sample_sum = 0;
			pc_learn_sample_sum = 0;
			cnt_of_learn_sample_sum = 0;

			is_had_enter_adjust_value_mode = SELF_LEARN_STDV_START;
		}

		break;

	case SET_PX_ADC_VALU_MAX_PERCENT:
		if ((param) <= 30) {
			signal_cfg_param.px_more_percent_max = (param);
			pa_avg_val_max = (signal_cfg_param.pa_learn_avg_val * (100 + param)) / 100;
			pb_avg_val_max = (signal_cfg_param.pb_learn_avg_val * (100 + param)) / 100;
			pc_avg_val_max = (signal_cfg_param.pc_learn_avg_val * (100 + param)) / 100;
		} else if((param) > 30) {
			signal_cfg_param.px_more_percent_max = 30;
			pa_avg_val_max = (signal_cfg_param.pa_learn_avg_val * 130) / 100;
			pb_avg_val_max = (signal_cfg_param.pb_learn_avg_val * 130) / 100;
			pc_avg_val_max = (signal_cfg_param.pc_learn_avg_val * 130) / 100;
		}

		break;

	case SET_PX_ADC_VALU_MIN_PERCENT:
		if ((param) <= 30) {
			signal_cfg_param.px_less_percent_max = (param);
			pa_avg_val_min = (signal_cfg_param.pa_learn_avg_val * (100 - param)) / 100;
			pb_avg_val_min = (signal_cfg_param.pb_learn_avg_val * (100 - param)) / 100;
			pc_avg_val_min = (signal_cfg_param.pc_learn_avg_val * (100 - param)) / 100;
		} else if((param) > 30) {
			signal_cfg_param.px_less_percent_max = 30;
			pa_avg_val_min = (signal_cfg_param.pa_learn_avg_val * 70) / 100;
			pb_avg_val_min = (signal_cfg_param.pb_learn_avg_val * 70) / 100;
			pc_avg_val_min = (signal_cfg_param.pc_learn_avg_val * 70) / 100;
		}
		break;

#endif

#if 1==DEBUG_CHECKINFO_TRAN
	case SET_DEBUG_PARAM:
		switch ((param)) {
		case PA_TOO_L:
			pa_check_bin_info |= 1 << 4;
			break;
		case PA_TOO_H:
			pa_check_bin_info |= 1 << 3;
			break;
		case PA_LOST:
			pa_check_bin_info |= 1 << 2;
			break;

		case PB_TOO_L:
			pb_check_bin_info |= 1 << 4;
			break;
		case PB_TOO_H:
			pb_check_bin_info |= 1 << 3;
			break;
		case PB_LOST:
			pb_check_bin_info |= 1 << 2;
			break;

		case PC_TOO_L:
			pc_check_bin_info |= 1 << 4;
			break;
		case PC_TOO_H:
			pc_check_bin_info |= 1 << 3;
			break;
		case PC_LOST:
			pc_check_bin_info |= 1 << 2;
			break;
		default:
			send_data_to_pc(param, 2);
			break;
		}
		break;

	case CLR_DEBUG_PARAM:
		switch ((param)) {
		case PA_TOO_L:
			pa_check_bin_info &= ~(1 << 4);
			break;
		case PA_TOO_H:
			pa_check_bin_info &= ~(1 << 3);
			break;
		case PA_LOST:
			pa_check_bin_info &= ~(1 << 2);
			break;

		case PB_TOO_L:
			pb_check_bin_info &= ~(1 << 4);
			break;
		case PB_TOO_H:
			pb_check_bin_info &= ~(1 << 3);
			break;
		case PB_LOST:
			pb_check_bin_info &= ~(1 << 2);
			break;

		case PC_TOO_L:
			pc_check_bin_info &= ~(1 << 4);
			break;
		case PC_TOO_H:
			pc_check_bin_info &= ~(1 << 3);
			break;
		case PC_LOST:
			pc_check_bin_info &= ~(1 << 2);
			break;
		default:
			send_data_to_pc(param, 2);
			break;
		}
		break;
#endif

	case ENTER_SET_PARAM_MODE:  /* 进入设定参数模式, 需要发送字节序列0x90, 0xa5, 0xa5 */
		if ((param) == 0xA5A5) {
			is_had_enter_set_param_mode = 1;
		}
		break;

	case EXIT_SET_PARAM_MODE:  /* 退出设定参数模式, 需要发送字节序列0x91, 0xa5, 0xa5 */
		if ((param) == 0xA5A5) {
			is_had_enter_set_param_mode = 0;
		}
		break;

	case READ_SET_PARAM_VALUE:
		if ((param) == 0xA5A5) {
			send_data_to_pc(signal_cfg_param.pa_learn_avg_val, 2);
			send_data_to_pc(signal_cfg_param.pb_learn_avg_val, 2);
			send_data_to_pc(signal_cfg_param.pc_learn_avg_val, 2);
			send_data_to_pc(signal_cfg_param.px_more_percent_max, 2);
			send_data_to_pc(signal_cfg_param.px_less_percent_max, 2);
		}
		break;

	case SPC_GET_DEV_SN:
		if ((param) == 0x534E)
			send_dev_sn2pc();
		break;

	case SPC_SET_DEV_SN:
		if ((param) == 0x534E)
			proc_recv_devsn(tx_dev_sn);
		break;


	case READ_DEBUG_PARAM1:
		switch ((param)) {
		case RDP_READ_ADC_SAMPLE_VAL:
			send_data_to_pc(pa_sample4read, 4);
			send_data_to_pc(pb_sample4read, 4);
			send_data_to_pc(pc_sample4read, 4);
#if TEST_TX_POWEROFF
#if  !USE_PVD_CHECK_POWEROFF
			EXTI_GenerateSWInterrupt(EXTI_Line1);
#else
			EXTI_GenerateSWInterrupt(EXTI_Line16);
#endif

			//send_powerdown_info();
#endif
			break;

		case RDP_READ_ADC_SAMPLE_AVG_VAL:
			read_px_sample_avg = 1;
			send_data_to_pc(pa_sample_avg_val, 4);
			send_data_to_pc(pb_sample_avg_val, 4);
			send_data_to_pc(pc_sample_avg_val, 4);
			read_px_sample_avg = 0;
			break;

		case RDP_READ_LEARN_AVG_VAL:
			send_data_to_pc(signal_cfg_param.pa_learn_avg_val, 4);
			send_data_to_pc(signal_cfg_param.pb_learn_avg_val, 4);
			send_data_to_pc(signal_cfg_param.pc_learn_avg_val, 4);
			break;

		case RDP_READ_PX_STATE:
			send_data_to_pc(sys_misc_flag, 4);

			send_data_to_pc(pa_avg_val_max , 4);
			send_data_to_pc(pa_avg_val_min , 4);
			send_data_to_pc(pa_lost_avg_val, 4);

			send_data_to_pc(pb_avg_val_max , 4);
			send_data_to_pc(pb_avg_val_min , 4);
			send_data_to_pc(pb_lost_avg_val, 4);

			send_data_to_pc(pc_avg_val_max , 4);
			send_data_to_pc(pc_avg_val_min , 4);
			send_data_to_pc(pc_lost_avg_val, 4);

			break;

		case RDP_READ_TEMP_PARAM:
			break;

		case RDP_READ_TEMP_PARAM1:
			break;

		default:
			break;
		}
		break;

	case WRITE_DEBUG_PARAM1:
		break;

	case WRITE_DEBUG_PARAM2:
		break;
	
	case WRITE_DEBUG_PARAM3:
		break;

	default:
		break;

	}

	return;
}

void send_dev_sn2pc(void)
{
	int i;

	for (i=0; i<sizeof(signal_cfg_param.dev_sn) && '\0'!=signal_cfg_param.dev_sn[i]; ++i)
		send_data_to_pc(signal_cfg_param.dev_sn[i], 1);

	return;
}

/*
 * 返回值:
 * 	1  -- 接收完成
 *	0  -- 未完成
 *	-1 -- 有错误
 */
int recv_devsn_frompc(int isstart)
{
	static int index;
	static unsigned int delay_cnt;

	int i, len, ret = 0;

	if (isstart) {
		index  	  = 0;
		delay_cnt = tim3_8ms_cnt;

		for (i=0; i<sizeof(tx_dev_sn); ++i)
			tx_dev_sn[i] = 0;
	}

	if (tim3_8ms_cnt-delay_cnt > 150) { /* 1.2s timeout */
		flush_uart4pc_buf_fn();
		return -1;
	}

	len = strlen(DEV_SN_MODE);

	if (index < len) { /* 未接收完 */
		while(0 != get_uart4pc_buf_len_fn()) {
			if (index < len) {
				tx_dev_sn[index++] = get_uart4pc_buf_data_fn();
				ret = 0;
			} else {
				ret = 1;
				break;
			}
		}
	} else {
		ret = 1;
	}

	enable_usartx_recv_int(UART4PC);

	if (1 == ret)
		usr_cmd_proc(SPC_SET_DEV_SN, 0x534E);

	return ret;
}


int proc_recv_devsn(char *str)
{
	int len;
#if 0
	if (0 == strncmp(signal_cfg_param.dev_sn, DEV_SN_MODE, sizeof(signal_cfg_param.dev_sn))) {
		len = strlen(DEV_SN_MODE);
		if (len == strlen(str)) {
			strncpy(signal_cfg_param.dev_sn, str, DEV_SN_LEN_MAX);
			signal_cfg_param.dev_sn[DEV_SN_LEN_MAX] = '\0';

			if (split_bytes_to_5bits((unsigned char *)(signal_cfg_param.dev_sn), DEV_SN_MODE_LEN,
					tx_dev_sn_bits_5, sizeof(tx_dev_sn_bits_5)-1) < 0) {
				send_data_to_pc('6'<<24 | 'B'<<16 | 'I'<<8 | 'S', 4);
			}

		} else {
			send_data_to_pc('S'<<24 | 'N'<<16 | 'E'<<8 | 'R', 4);
		}
	} else {
		send_data_to_pc('S'<<24 | 'N'<<16 | 'H'<<8 | 'S', 4);
	}
#else
	len = strlen(DEV_SN_MODE);
	if (len == strlen(str)) {
		strncpy(signal_cfg_param.dev_sn, str, DEV_SN_LEN_MAX);
		signal_cfg_param.dev_sn[DEV_SN_LEN_MAX] = '\0';

		if (split_bytes_to_5bits((unsigned char *)(signal_cfg_param.dev_sn), DEV_SN_MODE_LEN,
				tx_dev_sn_bits_5, sizeof(tx_dev_sn_bits_5)-1) < 0) {
			send_data_to_pc('6'<<24 | 'B'<<16 | 'I'<<8 | 'S', 4);
		}

	} else {
		send_data_to_pc('S'<<24 | 'N'<<16 | 'E'<<8 | 'R', 4);
	}
#endif
	return 0;

}

#if USE_STM32_IWDG
/*
 * These timings are given for a 40 kHz clock but the microcontroller’s internal RC frequency can vary
 * from 30 to 60 kHz.
 * t0 = 1/f40 * cnt / pre_div
 * t1 = 1/f30 * cnt / pre_div = 4/3 * t0
 * t2 = 1/f60 * cnt / pre_div = 2/3 * t0
 *
 * Prescaler divider 	PR[2:0] bits 	Min timeout (ms) RL[11:0]=0x000 	Max timeout (ms) RL[11:0]=0xFFF
 * /4 			0 		0.1 					409.6
 */

/*
 * t = (reload_val + 1) * 1/f40 / pre_div
 *
 * t0(ms)	t1(ms)	t2(ms)
 * 0.1		0.13	0.067
 * 409.6	546.13	273.07
 * 100(0x3e7)	133.3   66.67
 *
 * 30(0x12b)	40	20
 * 90(0x383)	120	60
 *
 * IWDG_RELOAD_VALUE -- max 0xfff
 */
#define IWDG_RELOAD_VALUE (0x383)

void iwdg_init(void)
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

	IWDG_SetPrescaler(IWDG_Prescaler_4);
	IWDG_SetReload(IWDG_RELOAD_VALUE);
#if 0
	rt_kprintf("IWDG->SR:0x%x\n", IWDG->SR);
	/* 在没有iwdg没有enable时, 状态标志可能不会更新, ysh>>IWDG->SR:0x3 */
	while (SET==IWDG_GetFlagStatus(IWDG_FLAG_PVU))
		; /* nothing, 等待寄存器更新完成 */
	while (SET==IWDG_GetFlagStatus(IWDG_FLAG_RVU))
		; /* nothing, 等待寄存器更新完成 */
#endif
	/* 禁止更新寄存器应该只是禁止软件来更新, 不会停止正在进行的更新(由硬件正在进行的更新) */
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Disable);

	IWDG_Enable();
	return;
}
#endif

#if USE_STM32_WWDG
void wwdg_init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);
	WWDG_SetPrescaler(WWDG_Prescaler_8);
	/* 不使用过早喂狗复位功能 */
	WWDG_SetWindowValue(0x7f);

	WWDG_Enable(WWDG_RELOAD_VALUE);
	//rt_kprintf("func:%s, line:%d, CR:0x%x, CFR:0x%x, SR:0x%x\n", __FUNCTION__, __LINE__, WWDG->CR, WWDG->CFR, WWDG->SR);

	return;
}
#endif

#define SYS_DAYS  "days,"
#define SYS_HOURS "hours,"
#define SYS_MINS  "mins,"
#define SYS_SECS  "secs\n"

void send_sys_time_info2uart(unsigned int sec)
{
	int days, h, m, s, temp, i, len;
	char str[24];

	temp = sec;
	days = temp / (24*60*60);
	i2str(str, days);
	len = strlen(str);
	for (i=0; i<len; ++i)
		send_data_to_pc_lf(*(str+i), 1);
	send_data_to_pc_lf(*((unsigned int *)(SYS_DAYS)), 4);
	send_data_to_pc_lf(*((unsigned int *)(SYS_DAYS+4)), 1);

	temp %= 24*60*60;
	h	 = temp / (60*60);
	i2str(str, h);
	len = strlen(str);
	for (i=0; i<len; ++i)
		send_data_to_pc_lf(*(str+i), 1);
	send_data_to_pc_lf(*((unsigned int *)(SYS_HOURS)), 4);
	send_data_to_pc_lf(*((unsigned int *)(SYS_HOURS+4)), 2);

	temp %= 60*60;
	m 	 = temp / 60;
	i2str(str, m);
	len = strlen(str);
	for (i=0; i<len; ++i)
		send_data_to_pc_lf(*(str+i), 1);
	send_data_to_pc_lf(*((unsigned int *)(SYS_MINS)), 4);
	send_data_to_pc_lf(*((unsigned int *)(SYS_MINS+4)), 1);

	s = temp % 60;
	i2str(str, s);
	len = strlen(str);
	for (i=0; i<len; ++i)
		send_data_to_pc_lf(*(str+i), 1);
	send_data_to_pc_lf(*((unsigned int *)(SYS_SECS)), 4);
	send_data_to_pc_lf(*((unsigned int *)(SYS_SECS+4)), 1);

	return;
}

int i2str(char *str, int n)
{
	char *pch;
	char ch;

	if (NULL==str)
		return FAIL;

	if (0 == n) {
		*str++ = '0';
		*str   = '\0';
		return SUCC;
	}

	if (n < 0) {
		*str++ = '-';
		n = -n;
	}

	pch = str;
	while (0 != n) {
		*pch++ = '0' + n%10;
		n /= 10;
	}

	*pch-- = '\0';
	while (str < pch) {
		ch = *pch;
		*pch-- = *str;
		*str++ = ch;
	}

	return SUCC;
}

int strlen(char *str)
{
	int len;

	if (NULL == str)
		return 0;

	len = 0;
	while (0 != *str++)
		++len;

	return len;

}

/*
 * #define TX_VERSION "TX-NG-0.12"
 * TX-NG-0.12
 * TX-V0.0.0
 */
int get_ver_str(char str[], int len)
{
	char buf[32];

	if (len < 10)
		return 1;

	*str++ = 'T';
	*str++ = 'X';
	*str++ = '-';
	*str++ = 'V';
	*str++ = '0' + TX_VERSION;
	*str++ = '.';
	*str++ = '0' + TX_SUBVERSION;
	*str++ = '.';

	i2str(buf, TX_REVISION);

	*str++ = buf[0];
	*str++ = buf[1];

	return 0;
}

