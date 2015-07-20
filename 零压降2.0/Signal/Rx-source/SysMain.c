/***********************************************************
*   函数库说明：底层硬件初始化函数库                       *
*   版本：    Ver1.0                                       *
*   作者：    zzjjhh250/ZZJJHH250 @ (CopyRight)            *
*   创建日期：11/13/2010                                   *
* -------------------------------------------------------- *
*  [硬件说明]                                              *
*   处理器：    STM32F103VBT6                              *
*   系统时钟：  外部8M/PLL = 72M                           *
* -------------------------------------------------------- *
*  [支 持 库]                                              *
*   支持库名称：PF_Config.h                                *
*   需要版本：  -----                                      *
*   声明库说明：硬件平台配置声明库                         *
* -------------------------------------------------------- *
*  [版本更新]                                              *
*   修改：                                                 *
*   修改日期：                                             *
*   版本：                                                 *
* -------------------------------------------------------- *
*  [版本历史]                                              *
* -------------------------------------------------------- *
*  [使用说明]                                              *
***********************************************************/

/********************
* 头 文 件 配 置 区 *
********************/
#include "..\Source\LIB_Config.h"
#include "..\Source\PF_Config.H"
#include ".\fwlib\stm32f10x_flash.h"
#include "sys_comm.h"

#include "stm32f10x.h"
#include "stm32f10x_rcc.h"

#include ".\FWLib\stm32f10x_wwdg.h"
#include "common.h"
#include "signal_proc.h"
#include "stm32f10x_it.h"
#include "board.h"
#include "tx_rx_comm.h"
#include "uart3.h"
//#include "tctrl.h"
#include "info_tran.h"



/********************
*   全局变量声明区  *
********************/
struct signal_cfg_param_tbl signal_cfg_param;

/*
 * 这个变量只在set_to_use_optical_fiber1()和set_to_use_optical_fiber2()中设置
 *
 * current_use_fiber_channel_no
 *  */
volatile int cur_fiber_channel_no;

static int is_had_enter_set_param_mode = 0;


static void init_signal_cfg_param_tbl(volatile struct signal_cfg_param_tbl *cfg_param);
static void read_sig_cfg_param_from_flash(volatile struct signal_cfg_param_tbl *cfg_param);
FLASH_Status write_sig_cfg_param_to_flash(volatile struct signal_cfg_param_tbl *cfg_param);

static void _get_sig_cfg_param(volatile struct signal_cfg_param_tbl *cfg_param, int is_init);
FLASH_Status write_sig_cfg_param_to_flash_for_init(volatile struct signal_cfg_param_tbl *cfg_param);

extern void wwdg_init(void);
extern void send_sys_time_info2uart(unsigned int sec);
extern int get_ver_str(char str[], int len);
extern void send_txdev_sn2pc(void);

void clr_dac(void)
{
	disable_usartx(USART1);

	/* 以下依据DAC76321逻辑真值表来写时序 */
	GPIOB->BRR = GPIO_Pin_12; /* LDAC拉低 */
	GPIOA->BRR = GPIO_Pin_4;  /* CS拉低 */

	disable_dmax_y(DMA1_Channel3);
	disable_spix(SPI1);
	DMA1_Channel3->CMAR  =  (u32)(&(SPI_DMA_Table_serial_in_zero[0]));
	DMA1_Channel3->CNDTR =  6;
	enable_spix(SPI1);
	enable_dmax_y(DMA1_Channel3);

	while (!is_dmax_channely_tcif_set(DMA1, 3))
		;
	while (is_spix_txbuf_not_empty(SPI1))
		;

	/*以下基于DAC7631数据手册时序编写*/
	GPIOA->BRR  = GPIO_Pin_4;  /*CS拉低 */ /* mark by David */
	GPIOA->BSRR = GPIO_Pin_4;  /*CS拉高 */
	GPIOB->BSRR = GPIO_Pin_12; /*LDAC拉高 */

	clear_dmax_channely_tcif(DMA1, 3);

	enable_usartx(USART1);
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
	int i, temp;

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	System_Init();	   //与库没有关系 这个是片内设备和片外设备的总初始化
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	flush_uart4pc_buf_fn();
	flush_uart3_buf_fn();

#if USE_STM32_WWDG
	wwdg_init();
#endif

	while(1) {
#if USE_STM32_WWDG
		WWDG->CR = WWDG_RELOAD_VALUE; /* 喂狗 */
#endif
		usr_cmd_analysis();
		proc_data_uart3();
#if 1
		/* 1Hz */
		if (0 != (tim3_8ms_cnt & (1<<6))) {
			on_running_led();
		} else {
			off_running_led();
		}

		if (1 == tx_dev_sn_buf_state) {
			for (i=0; i<DEV_SN_5BITS_CNT; ++i) {
				if (tx_dev_sn_bits_5[i] != prev_send_tx_dev_sn_bits_5[i])
					break;
			}

			if (i < DEV_SN_5BITS_CNT) {
				temp = merge_5bits_to_bytes(tx_dev_sn, sizeof(tx_dev_sn),
						tx_dev_sn_bits_5, DEV_SN_5BITS_CNT);
				if (temp > 0) {
					extern unsigned char tx_soft_version[4];

					for (i=0; i<DEV_SN_5BITS_CNT; ++i) {
						prev_send_tx_dev_sn_bits_5[i] = tx_dev_sn_bits_5[i];
					}
					tx_dev_sn_buf_state = 2;
					set_bit(need_send_some_info2lcdcpu, SEND_DEV_SN2LCD_CPU_BIT_MASK);
#if 0
					send_data_to_pc((unsigned)0xf1<<24 | (unsigned)0xf2<<16 | (unsigned)0xf3<<8 | 'A', 4);
					send_data_to_pc(non_change_txe_info_send_cnt, 4);
					send_data_to_pc(need_send_some_info2lcdcpu, 4);
					send_data_to_pc(tx_dev_sn_buf_state, 4);
					send_data_to_pc((unsigned)0xf1<<24 | (unsigned)0xf2<<16 | (unsigned)0xf3<<8 | tx_soft_version[3], 4);
#endif

				} else {
					send_data_to_pc('M'<<24 | 'B'<<16 | 'I'<<8 | 'S', 4);
					tx_dev_sn_buf_state = 0;
				}
			} else {
				tx_dev_sn_buf_state = 0;
			}
				

		}
#endif
		//box_tmper_ctrl();
	}  /* end while(1) */
}


/* !!!NOTE: isr 中会修改 px_zero_position_value */
void get_sig_cfg_param(int is_init)
{
	_get_sig_cfg_param(&signal_cfg_param, is_init); /* David */

	real_pa_zero_position_value   = signal_cfg_param.px_adj_param[cur_fiber_channel_no].pa_zeropos_real_val;
	real_pb_zero_position_value   = signal_cfg_param.px_adj_param[cur_fiber_channel_no].pb_zeropos_real_val;
	real_pc_zero_position_value   = signal_cfg_param.px_adj_param[cur_fiber_channel_no].pc_zeropos_real_val;

	return;
}


static void _get_sig_cfg_param(volatile struct signal_cfg_param_tbl *cfg_param, int is_init)
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

static void init_adj_zvd_param(volatile struct adj_zvd_param_st *adj_param)
{
	adj_param->pa_amplify_adj       = PA_AMPLIFY_DEF_VAL;
	adj_param->pb_amplify_adj       = PB_AMPLIFY_DEF_VAL;
	adj_param->pc_amplify_adj       = PC_AMPLIFY_DEF_VAL;

	adj_param->pa_phase_time_adj    = PA_PHASE_TIME_DEF_VAL;
	adj_param->pb_phase_time_adj    = PB_PHASE_TIME_DEF_VAL;
	adj_param->pc_phase_time_adj    = PC_PHASE_TIME_DEF_VAL;

	adj_param->pa_zeropos_adj_val = PA_ZERO_POSITION_DEF_ADJ_VAL;
	adj_param->pb_zeropos_adj_val = PB_ZERO_POSITION_DEF_ADJ_VAL;
	adj_param->pc_zeropos_adj_val = PC_ZERO_POSITION_DEF_ADJ_VAL;

	adj_param->pa_zeropos_real_val = REAL_ZERO_POSITION_DEF_VALUE;
	adj_param->pb_zeropos_real_val = REAL_ZERO_POSITION_DEF_VALUE;
	adj_param->pc_zeropos_real_val = REAL_ZERO_POSITION_DEF_VALUE;

	adj_param->pa_amp_factor_positive_adj = PA_AMP_FACTOR_POSITIVE_DEF_VAL;
	adj_param->pb_amp_factor_positive_adj = PB_AMP_FACTOR_POSITIVE_DEF_VAL;
	adj_param->pc_amp_factor_positive_adj = PC_AMP_FACTOR_POSITIVE_DEF_VAL;

	adj_param->pa_amp_factor_negtive_adj  = PX_AMP_FACTOR_NEGTIVE_DEF_VAL;
	adj_param->pb_amp_factor_negtive_adj  = PX_AMP_FACTOR_NEGTIVE_DEF_VAL;
	adj_param->pc_amp_factor_negtive_adj  = PX_AMP_FACTOR_NEGTIVE_DEF_VAL;

	return;
}
static void init_signal_cfg_param_tbl(volatile struct signal_cfg_param_tbl *cfg_param)
{
	int i;

	cfg_param->magic_num            = MAGIC_NUM_OF_CFG_PARAM_TBL;

	for (i=0; i<FIBER_CHANNEL_NUM_MAX; ++i) {
		init_adj_zvd_param(&cfg_param->px_adj_param[i]);
	}

	signal_cfg_param.channel_valid_indication_level = 0;
	signal_cfg_param.pad[0] = 0;
	signal_cfg_param.pad[1] = 0;
	signal_cfg_param.pad[2] = 0;

	return;
}


static void read_sig_cfg_param_from_flash(volatile struct signal_cfg_param_tbl *cfg_param)
{
	u32 *ps, *pd;
	int i;
	ps = (u32 *)UART_AMPLIFY_ADDR;
	pd = (u32 *)cfg_param;
	for (i=0; i < (sizeof(struct signal_cfg_param_tbl)/4); i++) {
		*pd++ = *ps++;
	}
}


FLASH_Status write_sig_cfg_param_to_flash(volatile struct signal_cfg_param_tbl *cfg_param)
{
	u32 *ps, *pd, temp;
	int i;
	FLASH_Status status;
	FLASH_Unlock();
	/* Clear pending flags (if any) */
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPTERR | FLASH_FLAG_WRPRTERR |
					FLASH_FLAG_PGERR | FLASH_FLAG_BSY);
#if USE_STM32_WWDG
	WWDG->CR = WWDG_RELOAD_VALUE; /* 喂狗 */
#endif
#if 1
	status = FLASH_ErasePage(UART_AMPLIFY_ADDR);	  //刷除 数据
	if (FLASH_COMPLETE != status)
		while(1); //return status;
#endif
#if USE_STM32_WWDG
	WWDG->CR = WWDG_RELOAD_VALUE; /* 喂狗 */
#endif
	ps = (u32 *)cfg_param;
	pd = (u32 *)UART_AMPLIFY_ADDR;
	for (i=0; i < (sizeof(struct signal_cfg_param_tbl)/4); i++) {
		temp = *ps;
		status = FLASH_ProgramWord((u32)pd , temp);

		if (i < 40/4) /* 为了兼容以前的调试软件 */
			send_data_to_pc(temp, 4);
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


FLASH_Status write_sig_cfg_param_to_flash_for_init(volatile struct signal_cfg_param_tbl *cfg_param)
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


#if 0
extern volatile u32 dma1_6_cnt;
extern volatile u32 tim4_cnt;
extern volatile u32 rx_data_err_cnt;
#endif
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
	case ADJ_PA_AMPLIFY                      :
	case ADJ_PB_AMPLIFY                      :
	case ADJ_PC_AMPLIFY                      :
	case ADJ_PA_PHASE_TIME                   :
	case ADJ_PB_PHASE_TIME                   :
	case ADJ_PC_PHASE_TIME                   :
	case ADJ_PA_ZERO_POSITION                :
	case ADJ_PB_ZERO_POSITION                :
	case ADJ_PC_ZERO_POSITION                :
	case ADJ_PA_AMPLIFICATION_FACTOR_POSITIVE:
	case ADJ_PB_AMPLIFICATION_FACTOR_POSITIVE:
	case ADJ_PC_AMPLIFICATION_FACTOR_POSITIVE:
	case ADJ_PA_AMPLIFICATION_FACTOR_NEGTIVE :
	case ADJ_PB_AMPLIFICATION_FACTOR_NEGTIVE :
	case ADJ_PC_AMPLIFICATION_FACTOR_NEGTIVE :
	case READ_DEBUG_PARAM1:
	case WRITE_DEBUG_PARAM1:
	case WRITE_DEBUG_PARAM2:
	case WRITE_DEBUG_PARAM3:
	case WRITE_DEBUG_PARAM4:
	case WRITE_DEBUG_PARAM5:
	case SET_CHANNEL_VALID_INDICATION_LEVEL:
		return 1;

	default:
		return 0;
	}
}

extern volatile unsigned int tim3_8ms_cnt;
extern unsigned int box_temper_prev_send;

extern volatile unsigned int rx_continue_overload_cnt_prev;
extern volatile unsigned int rx_continue_overload_cnt;
extern volatile unsigned int exti5_10_cnt;

extern volatile unsigned int pa_avg_val_sum4read;
extern volatile unsigned int pb_avg_val_sum4read;
extern volatile unsigned int pc_avg_val_sum4read;

#if RECORD_ADC_PX_PEAK_TROUGH_INFO
/* Approximate, 每隔一定时间段更新一次 */
extern volatile struct px_peak_trough_info_st pa_adc_peak_trough_info;
extern volatile struct px_peak_trough_info_st pb_adc_peak_trough_info;
extern volatile struct px_peak_trough_info_st pc_adc_peak_trough_info;
#endif
#if RECORD_DAC_PX_PEAK_TROUGH_INFO
extern volatile struct px_peak_trough_info_st pa_dac_peak_trough_info;
extern volatile struct px_peak_trough_info_st pb_dac_peak_trough_info;
extern volatile struct px_peak_trough_info_st pc_dac_peak_trough_info;
#endif

void usr_cmd_proc(int cmd, s32 param)
{
	int temp;
	char str[12];

	param &= 0xffff;
	temp = (cmd << 16) | param;
	if (0==is_had_enter_set_param_mode && 0x90a5a5!=temp) {
		return;
	}

	if (is_doing_px_zeropos_adj()) {
		if (is_doing_pa_zeropos_adj() && (ADJ_PA_ZERO_POSITION!=cmd))
			done_pa_zeropos_adj();

		if (is_doing_pb_zeropos_adj() && (ADJ_PB_ZERO_POSITION!=cmd))
			done_pb_zeropos_adj();

		if (is_doing_pc_zeropos_adj() && (ADJ_PC_ZERO_POSITION!=cmd))
			done_pc_zeropos_adj();
	}

	send_data_to_pc(temp, 3);
	switch(cmd) {
	case WRITE_CFG_PARAM2FLASH: /* 将配置参数写入flash, 需要发送字节序列0x69, 0xaa, 0xaa */
		if ((param) == 0xAAAA)
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
			send_data_to_pc(box_temper_prev_send, 4);
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

	case ADJ_PA_AMPLIFY:   /* 调整A相幅值 */
		signal_cfg_param.px_adj_param[cur_fiber_channel_no].pa_amplify_adj = param;
		break;

	case ADJ_PB_AMPLIFY:
		signal_cfg_param.px_adj_param[cur_fiber_channel_no].pb_amplify_adj = param;
		break;

	case ADJ_PC_AMPLIFY:
		signal_cfg_param.px_adj_param[cur_fiber_channel_no].pc_amplify_adj = param;
		break;

	case ADJ_PA_PHASE_TIME:   /* 调整A相相位 */
		signal_cfg_param.px_adj_param[cur_fiber_channel_no].pa_phase_time_adj = param;
		break;

	case ADJ_PB_PHASE_TIME:
		signal_cfg_param.px_adj_param[cur_fiber_channel_no].pb_phase_time_adj = param;
		break;

	case ADJ_PC_PHASE_TIME:
		signal_cfg_param.px_adj_param[cur_fiber_channel_no].pc_phase_time_adj = param;
		break;

	case ADJ_PA_ZERO_POSITION:   /* 调整A相零位 */
		pa_zeropos_sum 	   = 0;
		pa_zeropos_adc_cnt = 0;
		signal_cfg_param.px_adj_param[cur_fiber_channel_no].pa_zeropos_adj_val = param;
		start_pa_zeropos_adj();
		break;

	case ADJ_PB_ZERO_POSITION:
		pb_zeropos_sum 	   = 0;
		pb_zeropos_adc_cnt = 0;
		signal_cfg_param.px_adj_param[cur_fiber_channel_no].pb_zeropos_adj_val = param;
		start_pb_zeropos_adj();
		break;

	case ADJ_PC_ZERO_POSITION:
		pc_zeropos_sum 	   = 0;
		pc_zeropos_adc_cnt = 0;
		signal_cfg_param.px_adj_param[cur_fiber_channel_no].pc_zeropos_adj_val = param;
		start_pc_zeropos_adj();
		break;

	case ADJ_PA_AMPLIFICATION_FACTOR_POSITIVE:   /* 调整A相正半周放大系数 */
		signal_cfg_param.px_adj_param[cur_fiber_channel_no].pa_amp_factor_positive_adj = param;
		break;

	case ADJ_PB_AMPLIFICATION_FACTOR_POSITIVE:
		signal_cfg_param.px_adj_param[cur_fiber_channel_no].pb_amp_factor_positive_adj = param;
		break;

	case ADJ_PC_AMPLIFICATION_FACTOR_POSITIVE:
		signal_cfg_param.px_adj_param[cur_fiber_channel_no].pc_amp_factor_positive_adj = param;
		break;

	case READ_SET_PARAM_VALUE:
		if ((param) == 0xA5A5) {
			send_data_to_pc(signal_cfg_param.px_adj_param[cur_fiber_channel_no].pa_zeropos_adj_val, 2);
			send_data_to_pc(signal_cfg_param.px_adj_param[cur_fiber_channel_no].pa_amp_factor_positive_adj, 2);
			send_data_to_pc(signal_cfg_param.px_adj_param[cur_fiber_channel_no].pa_amplify_adj, 2);
			send_data_to_pc(signal_cfg_param.px_adj_param[cur_fiber_channel_no].pa_phase_time_adj, 2);

			send_data_to_pc(signal_cfg_param.px_adj_param[cur_fiber_channel_no].pb_zeropos_adj_val, 2);
			send_data_to_pc(signal_cfg_param.px_adj_param[cur_fiber_channel_no].pb_amp_factor_positive_adj, 2);
			send_data_to_pc(signal_cfg_param.px_adj_param[cur_fiber_channel_no].pb_amplify_adj, 2);
			send_data_to_pc(signal_cfg_param.px_adj_param[cur_fiber_channel_no].pb_phase_time_adj, 2);

			send_data_to_pc(signal_cfg_param.px_adj_param[cur_fiber_channel_no].pc_zeropos_adj_val, 2);
			send_data_to_pc(signal_cfg_param.px_adj_param[cur_fiber_channel_no].pc_amp_factor_positive_adj, 2);
			send_data_to_pc(signal_cfg_param.px_adj_param[cur_fiber_channel_no].pc_amplify_adj, 2);
			send_data_to_pc(signal_cfg_param.px_adj_param[cur_fiber_channel_no].pc_phase_time_adj, 2);

			send_data_to_pc(signal_cfg_param.channel_valid_indication_level, 1);
		}
		break;

	case READ_DEBUG_PARAM1:
		switch ((param)) {
		case RDP_READ_TEMP_PARAM:
			break;

		case RDP_READ_PX_REAL_ZERO_POS_VAL:
			send_data_to_pc(real_pa_zero_position_value, 4);
			send_data_to_pc(real_pb_zero_position_value, 4);
			send_data_to_pc(real_pc_zero_position_value, 4);
			break;

		case RDP_READ_OVERLOAD_PARAM_VAL:
			send_data_to_pc(rx_continue_overload_cnt_prev, 4);
			send_data_to_pc(rx_continue_overload_cnt, 4);
			send_data_to_pc(exti5_10_cnt, 4);
			break;

		case RDP_READ_AVG_VAL_SUM:
			send_data_to_pc(pa_avg_val_sum4read, 4);
			send_data_to_pc(pb_avg_val_sum4read, 4);
			send_data_to_pc(pc_avg_val_sum4read, 4);
			break;

		case RDP_READ_TEMP_PARAM1:
		{
			int max, min;

#if RECORD_ADC_PX_PEAK_TROUGH_INFO
			max = pa_adc_peak_trough_info.px_approx_peak_val;
			min = pa_adc_peak_trough_info.px_approx_trough_val;
			send_data_to_pc(max, 4);
			send_data_to_pc(min, 4);

			max = pb_adc_peak_trough_info.px_approx_peak_val;
			min = pb_adc_peak_trough_info.px_approx_trough_val;
			send_data_to_pc(max, 4);
			send_data_to_pc(min, 4);

			max = pc_adc_peak_trough_info.px_approx_peak_val;
			min = pc_adc_peak_trough_info.px_approx_trough_val;
			send_data_to_pc(max, 4);
			send_data_to_pc(min, 4);
#endif
#if RECORD_DAC_PX_PEAK_TROUGH_INFO
			max = pa_dac_peak_trough_info.px_approx_peak_val;
			min = pa_dac_peak_trough_info.px_approx_trough_val;
			send_data_to_pc(max, 4);
			send_data_to_pc(min, 4);

			max = pb_dac_peak_trough_info.px_approx_peak_val;
			min = pb_dac_peak_trough_info.px_approx_trough_val;
			send_data_to_pc(max, 4);
			send_data_to_pc(min, 4);

			max = pc_dac_peak_trough_info.px_approx_peak_val;
			min = pc_dac_peak_trough_info.px_approx_trough_val;
			send_data_to_pc(max, 4);
			send_data_to_pc(min, 4);
#endif

		}
			break;

		case RDP_READ_TEMP_PARAM2:
			send_txdev_sn2pc();
			break;

		case RDP_READ_TEMP_PARAM3:
			break;

		case RDP_READ_TEMP_PARAM4:
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

	case SET_CHANNEL_VALID_INDICATION_LEVEL: /* 配置通道指示信号有效电平, 需要发送字节序列0x6E, 0xa5, 0xa*  */
		if ((param & 0xfff0) == 0xA5A0) {
			if (0 != (param & 0x0f)) {
				signal_cfg_param.channel_valid_indication_level = 1;
			} else {
				signal_cfg_param.channel_valid_indication_level = 0;
			}
		}
		break;

	default:
		break;

	}

	return;
}

extern unsigned char tx_dev_sn[DEV_SN_LEN_MAX+1];	/* 设备序列号 */
void send_txdev_sn2pc(void)
{
	int i;

	for (i=0; i<sizeof(tx_dev_sn) && '\0'!=tx_dev_sn[i]; ++i)
		send_data_to_pc(tx_dev_sn[i], 1);

	return;
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
	len = strlen_(str);
	for (i=0; i<len; ++i)
		send_data_to_pc_lf(*(str+i), 1);
	send_data_to_pc_lf(*((unsigned int *)(SYS_DAYS)), 4);
	send_data_to_pc_lf(*((unsigned int *)(SYS_DAYS+4)), 1);

	temp %= 24*60*60;
	h	 = temp / (60*60);
	i2str(str, h);
	len = strlen_(str);
	for (i=0; i<len; ++i)
		send_data_to_pc_lf(*(str+i), 1);
	send_data_to_pc_lf(*((unsigned int *)(SYS_HOURS)), 4);
	send_data_to_pc_lf(*((unsigned int *)(SYS_HOURS+4)), 2);

	temp %= 60*60;
	m 	 = temp / 60;
	i2str(str, m);
	len = strlen_(str);
	for (i=0; i<len; ++i)
		send_data_to_pc_lf(*(str+i), 1);
	send_data_to_pc_lf(*((unsigned int *)(SYS_MINS)), 4);
	send_data_to_pc_lf(*((unsigned int *)(SYS_MINS+4)), 1);

	s = temp % 60;
	i2str(str, s);
	len = strlen_(str);
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

int strlen_(char *str)
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
 * #define TX_VERSION "RX-NG-0.12"
 * RX-NG-0.12
 * RX-V0.0.0
 */
int get_ver_str(char str[], int len)
{
	char buf[32];

	if (len < 10)
		return 1;

	*str++ = 'R';
	*str++ = 'X';
	*str++ = '-';
	*str++ = 'V';
	*str++ = '0' + RX_VERSION;
	*str++ = '.';
	*str++ = '0' + RX_SUBVERSION;
	*str++ = '.';

	i2str(buf, RX_REVISION);

	*str++ = buf[0];
	*str++ = buf[1];

	return 0;
}


#if USE_OPTICX_200S_VERSION

#define is_channel_indication_value_valid(x) ((1==(x)) || (2==(x)))

static int is_had_cancel_force_output_switch2pt_lcdcmd(void);

enum sys_use_channel_e get_cur_use_channel(void)
{
	unsigned temp;
	enum sys_use_channel_e cur_channel;
	static unsigned channel_indication_val = 0xff;

	temp = get_channel_indication_value();

	if (0xff!=channel_indication_val && !is_channel_indication_value_valid(channel_indication_val)
			&& (is_channel_indication_value_valid(temp))) {
		cancel_force_output_switch2pt(UIUS_CHANNEL_INDICATION);
	}

	channel_indication_val = temp;

	if (is_had_force_output_switch2pt()) {
		if (is_had_cancel_force_output_switch2pt_lcdcmd() && is_channel_indication_value_valid(temp))
			cancel_force_output_switch2pt(UIUS_CHANNEL_INDICATION);
		else
			return SUC_PT_CHANNEL;
	}

#if 0
	switch (channel_indication_val) {
	case 1:
		cur_channel = SUC_FIBER_CHANNEL_1;
		break;

	case 2:
		cur_channel = SUC_FIBER_CHANNEL_2;
		break;

#if CHANNEL_INDICATION_VALID_VALUE
	case 0: /* 两个通道均未使用, 认为他没有接指示信号, 使用默认光纤头 */
		cur_channel = SUC_FIBER_CHANNEL_1;
		break;
#else
	case 3: /* 两个通道均未使用, 认为他没有接指示信号, 使用默认光纤头 */
		cur_channel = SUC_FIBER_CHANNEL_1;
		break;
#endif

	default:
		/* 如果异常，则使用默认光纤头, 并且强制切换到pt */
		cur_channel = SUC_PT_CHANNEL;
		break;
	}
#else
	switch (channel_indication_val) {
	case 1:
		if (0 == signal_cfg_param.channel_valid_indication_level)
			cur_channel = SUC_FIBER_CHANNEL_1;
		else
			cur_channel = SUC_FIBER_CHANNEL_2;
		break;

	case 2:
		if (0 == signal_cfg_param.channel_valid_indication_level)
			cur_channel = SUC_FIBER_CHANNEL_2;
		else
			cur_channel = SUC_FIBER_CHANNEL_1;
		break;

	default:
		/* 如果异常，则使用默认光纤头, 并且强制切换到pt */
		cur_channel = SUC_PT_CHANNEL;
		break;
	}

#endif
	return cur_channel;
}

void set_to_use_optical_fiber1(void)
{
	set_port_pin(use_channel_x_ctrl_gpio, use_channel_x_ctrl_pin);
	cur_fiber_channel_no = 0;
	real_pa_zero_position_value = signal_cfg_param.px_adj_param[0].pa_zeropos_real_val;
	real_pb_zero_position_value = signal_cfg_param.px_adj_param[0].pb_zeropos_real_val;
	real_pc_zero_position_value = signal_cfg_param.px_adj_param[0].pc_zeropos_real_val;

	return;
}


void set_to_use_optical_fiber2(void)
{
	clr_port_pin(use_channel_x_ctrl_gpio, use_channel_x_ctrl_pin);
	cur_fiber_channel_no = 1;
	real_pa_zero_position_value = signal_cfg_param.px_adj_param[1].pa_zeropos_real_val;
	real_pb_zero_position_value = signal_cfg_param.px_adj_param[1].pb_zeropos_real_val;
	real_pc_zero_position_value = signal_cfg_param.px_adj_param[1].pc_zeropos_real_val;

	return;
}

#endif

#if 1
static unsigned force_switch2pt_uid_vector;

/*
 * 在软件中有3中情况会强制切换到pt:
 * 1. 检测到输出过高、过低时
 * 2. 接收到lcd-cpu的强制切换命令
 * 3. 通道指示信号异常
 * */
void force_output_switch2pt(enum user_id_of_use_switch2pt_e uid)
{
	set_bit(force_switch2pt_uid_vector, uid);

	clr_port_pin(px_falt_sw_gpio, px_falt_sw_pin);

	return;
}

void cancel_force_output_switch2pt(enum user_id_of_use_switch2pt_e uid)
{

	clr_bit(force_switch2pt_uid_vector, uid);

	if (0 == force_switch2pt_uid_vector)
		set_port_pin(px_falt_sw_gpio, px_falt_sw_pin);

	return;
}

#if USE_OPTICX_200S_VERSION
static int is_had_cancel_force_output_switch2pt_lcdcmd(void)
{
	return is_bit_clr(force_switch2pt_uid_vector, UIUS_SWITCH_CMD_FROM_LCD_CPU);
}
#endif
#endif


