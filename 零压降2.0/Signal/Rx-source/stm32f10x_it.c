/*
 * stm32f10x_it.c
 *
 * Creat date 2011-10-13 17:58:23
 *  Copyright (C) 2011 Zhaoshaowei(David)
 */

#include "..\Source\LIB_Config.h"
#include "..\Source\PF_Config.H"
#include ".\fwlib\stm32f10x_exti.h"
#include ".\fwlib\stm32f10x.h"
#include ".\fwlib\stm32f10x_flash.h"
#include "sys_comm.h"
#include "common.h"
#include "tx_rx_comm.h"
#include "signal_proc.h"
#include "info_tran.h"
#include "board.h"
#include "stm32f10x_it.h"
#include "uart3.h"

#define USE_NEW_RX_OVERLOAD_ALGORITHM 1

enum recv_state_e {
	RS_WAIT_MSG_ID = 0,
	RS_WAIT_MIDDLE_MSG,
	RS_WAIT_LAST_MSG,
	RS_RECV_DATA_OVER
};

#define DROP_SAMPLE_CNT_MAX (440)

#define CCR_ENABLE_Set          ((uint32_t)0x00000001)
#define CR2_EXTTRIG_SWSTART_Set     ((uint32_t)0x00500000)
#define CCR_ENABLE_Reset        ((uint32_t)0xFFFFFFFE)

#define set_tx_px_toolow_lost_state(vec)  ((vec)=0x14)

/* 对当前应用来说ram空间比较足, cpu负荷过重, 以效率优先, 尽量使用字 */
struct check_info_st {
	unsigned int msg_id;
	int had_recv_bytes;

	unsigned int cpu_temper;
	unsigned int box_temper;
	unsigned int pa_bininfo;
	unsigned int pb_bininfo;
	unsigned int pc_bininfo;
	unsigned int soft_ver;
	unsigned char tx_devsn_5bit_buf[DEV_SN_5BITS_BUF_LEN];
};

#if USE_NEW_RX_OVERLOAD_ALGORITHM
volatile unsigned int rx_continue_overload_cnt_prev;
volatile unsigned int rx_continue_overload_cnt;
volatile unsigned int rx_continue_overload_delay;
#else
volatile unsigned int overcurrent_cnt;
volatile unsigned int overcurrent_door_judge;
volatile unsigned int delay_overcurrent_state;
#endif

extern struct signal_cfg_param_tbl signal_cfg_param;


volatile s32 real_pa_zero_position_value;
volatile s32 real_pb_zero_position_value;
volatile s32 real_pc_zero_position_value;


static volatile s32 pa_insert_val_1;
static volatile s32 pa_insert_val_2;

static volatile s32 pb_insert_val_1;
static volatile s32 pb_insert_val_2;

static volatile s32 pc_insert_val_1;
static volatile s32 pc_insert_val_2;
#if 0 == USE_ORG_ADC_VALUE
static int pa_delta_y_old[NUM_OF_RECORDING_DELTA_Y];
static int pb_delta_y_old[NUM_OF_RECORDING_DELTA_Y];
static int pc_delta_y_old[NUM_OF_RECORDING_DELTA_Y];
static int pa_org_val_prev, pb_org_val_prev, pc_org_val_prev;
#endif

volatile int pa_estimate_delta_y;
volatile int pb_estimate_delta_y;
volatile int pc_estimate_delta_y;


volatile unsigned int box_temper_sum;
volatile unsigned int pa_avg_val_sum;
volatile unsigned int pb_avg_val_sum;
volatile unsigned int pc_avg_val_sum;
volatile unsigned int cpu_temper_sum;

volatile unsigned int pa_avg_val_sum4read;
volatile unsigned int pb_avg_val_sum4read;
volatile unsigned int pc_avg_val_sum4read;

volatile unsigned int box_temper_max;
volatile unsigned int box_temper_min;
volatile unsigned int pa_avg_val_max;
volatile unsigned int pa_avg_val_min;
volatile unsigned int pb_avg_val_max;
volatile unsigned int pb_avg_val_min;
volatile unsigned int pc_avg_val_max;
volatile unsigned int pc_avg_val_min;
volatile unsigned int cpu_temper_max;
volatile unsigned int cpu_temper_min;

/* cpu温度值, 单位是摄氏度 */
volatile s32  cpu_temper;


/* 采集端信息记录 */
struct check_info_st check_info;
volatile unsigned int tx_cpu_temper, tx_cpu_temper_prev;
volatile unsigned int tx_box_temper, tx_box_temper_prev;
volatile unsigned int tx_pa_check_bin_info, tx_pa_check_bin_info_prev;
volatile unsigned int tx_pb_check_bin_info, tx_pb_check_bin_info_prev;
volatile unsigned int tx_pc_check_bin_info, tx_pc_check_bin_info_prev;

/* tx_soft_version[3]: 0-未接收到版本号, 1-已缓存到本地, 2-已发送完成
 * 0:major version, 1:minor version, 2:revise version, 3:表示是否接收到tx的版本号
 */
unsigned char tx_soft_version[4];
#if 0
unsigned char tx_dev_sn[DEV_SN_LEN_MAX+1] = {
	'Y', 'Y', 'P', 'P', '-', 'B', 'B', 'S',
	'S', 'S', 'S', '1', '2', '3', 'X', '-',
	'1', '1', '9'
};	/* 设备序列号 */
#else
unsigned char tx_dev_sn[DEV_SN_LEN_MAX+1];	/* 设备序列号 */
#endif

volatile unsigned int prev_send_soft_ver;
unsigned char prev_send_tx_dev_sn_bits_5[DEV_SN_5BITS_BUF_LEN];	/* 设备序列号 */

volatile unsigned int need_send_some_info2lcdcpu;


/* 0-未使用, 1-已缓存到本地, 2-正在给lcd-cpu发送, 3已发送完成 */
volatile int tx_dev_sn_buf_state;

unsigned char tx_dev_sn_bits_5[DEV_SN_5BITS_BUF_LEN];

volatile int tx_cpu_temper_using, tx_box_temper_using;
volatile int tx_pa_check_bin_info_using, tx_pb_check_bin_info_using, tx_pc_check_bin_info_using;
//volatile int tx_soft_version_using;

#define TX_POWEROFF_CONST 0X12A5
#define TX_POWERON_CONST  0X5AA5

volatile unsigned int tx_power_off = TX_POWERON_CONST;
volatile unsigned int tx_power_off_prev = TX_POWERON_CONST;
volatile unsigned int is_tx_poweroff_state_had_send = 1;
volatile unsigned int tx_poweroff_judge_delay;


volatile unsigned int fibre_optical_data_cnt;
volatile unsigned int fibre_optical_int_cnt4clr_dac;

volatile unsigned int tim3_8ms_cnt;

/* 恒温盒温度的adc值 */
unsigned int box_temper_prev_send;
unsigned int box_temper_adc4tctrl;

volatile unsigned int adj_px_zero_pos_vec;

volatile unsigned int pa_zeropos_sum;
volatile unsigned int pb_zeropos_sum;
volatile unsigned int pc_zeropos_sum;

volatile unsigned int pa_zeropos_adc_cnt;
volatile unsigned int pb_zeropos_adc_cnt;
volatile unsigned int pc_zeropos_adc_cnt;

#if USE_PVD
volatile unsigned int is_system_powerdown;
volatile unsigned int sys_powerdown_delay4confirm;
#endif

volatile unsigned int exti5_10_cnt;

volatile int non_change_txe_info_send_cnt;

extern void proc_org_data(unsigned int phase_id, int px_org_val);

static void proc_adc_data_and_send(void);
static void check_if_rx_px_fault(void);
static void send_tx_checkinfo(void);
/* static void judge_px_over_limit(void); */

extern void enable_protect_electric_relay_pin(unsigned int pin);
extern void disable_protect_electric_relay_pin(void);



static void memcpy_word(int *dst, const int *src, unsigned cnt);

#if PROC_ESTIMATE_DELTA_Y
extern struct interval_for_estimate_info_st pa_interval_info;
extern struct interval_for_estimate_info_st pb_interval_info;
extern struct interval_for_estimate_info_st pc_interval_info;

/*
 * 返回处理过的px_delta_y
 * */
static int proc_estimate_delta_y(int px_delta_y, int px_sample_val,
		struct interval_for_estimate_info_st *iinfo, volatile struct px_peak_trough_info_st *p);
static int find_real_sin_val_interval(int y, const int *interval_tbl);
static void calc_real_interval_info(int amplitude, struct interval_for_estimate_info_st *iinfo);
#endif

void write_byte2lcd_cpu(u8 DATA)
{
	put_usartx_data(USART3, DATA);

	while(!is_usartx_txd_empty(USART3))
		;
}

/*******************************************************************************
* Function Name  : USART2_IRQHandler
* Description    : This function handles USART2 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USART1_IRQHandler(void)
{
	static unsigned int rmsg_state = RS_WAIT_MSG_ID;
	static unsigned int recv_msg_id, recv_msg;

	unsigned int id_zone;
	unsigned int rx_data;

	++fibre_optical_int_cnt4clr_dac;

	/* USART_ClearITPendingBit(USART2,USART_IT_RXNE);
	* It is cleared by a read to the USART_DR register. The RXNE flag can also be cleared by
	* writing a zero to it. This clearing sequence is recommended only for multibuffer
	*/
	rx_data  = USART1->DR;
	id_zone  = (rx_data & PHASEX_ID_MASK);

#if DEBUG_DATA_TRANS
	USART_ITConfig(USART2,USART_IT_RXNE,DISABLE);
	USART_ITConfig(USART2,USART_IT_TXE,DISABLE);
	send_data_to_pc(creat_log_ind(0x30), 4);
	send_data_to_pc(rx_data, 4);
	send_data_to_pc(rmsg_state, 4);
#endif
	if (is_tx_poweroff_state_had_send && (sub_abs(tx_poweroff_judge_delay, tim3_8ms_cnt) > 100))
		tx_power_off = TX_POWERON_CONST;

	switch (rmsg_state) {
	case RS_WAIT_MSG_ID:
		if (TRMI_CONNECT != id_zone) {
			if (id_zone>=convert_id2firstbyte(TRMI_PHASE_A)
				&& id_zone<=convert_id2firstbyte(TRMI_PHASE_C)) {
				recv_msg_id = id_zone;
				recv_msg    = (rx_data & ~(PHASEX_ID_MASK)) << DATA_BITS_PRE_BYTE*2;
				rmsg_state  = RS_WAIT_MIDDLE_MSG;
			} else if (id_zone>=convert_id2firstbyte(TRMI_CPU_TEMPER)
					   && id_zone<=convert_id2firstbyte(TRMI_MISC_INFO)) {
				check_info.had_recv_bytes = 1;
				if (id_zone == convert_id2firstbyte(TRMI_MISC_INFO)) {
					/* 采集端掉电 */
					if ((convert_id2firstbyte(TRMI_MISC_INFO) | TRMIS_TX_POWEROFF) == rx_data) {
						tx_power_off = TX_POWEROFF_CONST;
						if (tx_power_off_prev ^ tx_power_off)
							is_tx_poweroff_state_had_send = 0;

						check_info.msg_id = 0;
					} else {
						check_info.msg_id = rx_data; /* 第一个字节包含主从id号 */
					}
				} else {
					check_info.msg_id = id_zone;
					switch (check_info.msg_id) {
					case convert_id2firstbyte(TRMI_CPU_TEMPER):
						if (0 != tx_cpu_temper_using) {
							check_info.msg_id = 0;
							break;
						}
						check_info.cpu_temper = (rx_data & ~(PHASEX_ID_MASK)) << 11; /* bit[15:11] */
						break;

					case convert_id2firstbyte(TRMI_BOX_TEMPER):
						if (0 != tx_box_temper_using) {
							check_info.msg_id = 0;
							break;
						}
						check_info.box_temper = (rx_data & ~(PHASEX_ID_MASK)) << 11; /* bit[15:11] */
						break;

					default:
						break;
					}
				}
			}
		} else if (0 != check_info.msg_id) {
			/* 是否接收到检测信息 */
			switch (check_info.msg_id) {
			case convert_id2firstbyte(TRMI_CPU_TEMPER):
				if (1 == check_info.had_recv_bytes) {
					check_info.cpu_temper |= rx_data << 6; /* bit[10:6] */
					++check_info.had_recv_bytes;
				} else if (2 == check_info.had_recv_bytes) {
					check_info.cpu_temper |= rx_data << 1; /* bit[5:1] */

					tx_cpu_temper = check_info.cpu_temper;
					check_info.msg_id = 0;
					tx_cpu_temper_using = 1;
				}
				break;

			case convert_id2firstbyte(TRMI_BOX_TEMPER):
				if (1 == check_info.had_recv_bytes) {
					check_info.box_temper |= rx_data << 6; /* bit[10:6] */
					++check_info.had_recv_bytes;
				} else if (2 == check_info.had_recv_bytes) {
					check_info.box_temper |= rx_data << 1; /* bit[5:1] */

					tx_box_temper = check_info.box_temper;
					check_info.msg_id = 0;
					tx_box_temper_using = 1;
				}
				break;

			case convert_id2firstbyte(TRMI_MISC_INFO) | TRMIS_PA_BININFO:
				if (0 != tx_pa_check_bin_info_using) {
					check_info.msg_id = 0;
					break;
				}

				tx_pa_check_bin_info = rx_data;
				check_info.msg_id = 0;
				tx_pa_check_bin_info_using = 1;
				break;

			case convert_id2firstbyte(TRMI_MISC_INFO) | TRMIS_PB_BININFO:
				if (0 != tx_pb_check_bin_info_using) {
					check_info.msg_id = 0;
					break;
				}
				tx_pb_check_bin_info = rx_data;
				check_info.msg_id = 0;
				tx_pb_check_bin_info_using = 1;
				break;

			case convert_id2firstbyte(TRMI_MISC_INFO) | TRMIS_PC_BININFO:
				if (0 != tx_pc_check_bin_info_using) {
					check_info.msg_id = 0;
					break;
				}
				tx_pc_check_bin_info = rx_data;
				check_info.msg_id = 0;
				tx_pc_check_bin_info_using = 1;
				break;

			case convert_id2firstbyte(TRMI_MISC_INFO) | TRMIS_TX_VERSION:
				if ((0 != tx_soft_version[3])) { // || (0 != tx_soft_version_using)) {
					check_info.msg_id = 0;
					break;
				}

				if (1 == check_info.had_recv_bytes) {
					check_info.soft_ver = rx_data << 15;	/* bit[19:15] */
					++check_info.had_recv_bytes;
				} else if (2 == check_info.had_recv_bytes) {
					check_info.soft_ver |= rx_data << 10;	/* bit[14:10] */
					++check_info.had_recv_bytes;
				} else if (3 == check_info.had_recv_bytes) {
					check_info.soft_ver |= rx_data << 5;	/* bit[9:5] */
					++check_info.had_recv_bytes;
				} else if (4 == check_info.had_recv_bytes) {
					check_info.soft_ver |= rx_data; 	/* bit[4:0] */

					tx_soft_version[0] = (check_info.soft_ver >> 14) & 0x3f; /* bit[19:14] */
					tx_soft_version[1] = (check_info.soft_ver >> 7) & 0x7f; /* bit[13:7] */
					tx_soft_version[2] = (check_info.soft_ver) & 0x7f; /* bit[6:0] */
					tx_soft_version[3] = 1; /* 已接收到发射端版本号 */

					check_info.msg_id = 0;
					//tx_soft_version_using = 1;

					if (prev_send_soft_ver != check_info.soft_ver) {
						prev_send_soft_ver = check_info.soft_ver;
						set_bit(need_send_some_info2lcdcpu, SEND_SOFT_VERSION2LCD_CPU_BIT_MASK);
					} else if ((non_change_txe_info_send_cnt >= 3)){
						tx_soft_version[3] = 0;
					}
				}
				break;

			case convert_id2firstbyte(TRMI_MISC_INFO) | TRMIS_TX_DEV_SN:
				if (0 == tx_dev_sn_buf_state) {
					check_info.tx_devsn_5bit_buf[check_info.had_recv_bytes-1] = rx_data;
					++check_info.had_recv_bytes;

					if (check_info.had_recv_bytes >= (DEV_SN_5BITS_CNT+1)) {
						for (rx_data=0; rx_data<DEV_SN_5BITS_CNT; ++rx_data)
							tx_dev_sn_bits_5[rx_data] = check_info.tx_devsn_5bit_buf[rx_data];
						tx_dev_sn_buf_state = 1;
					}
				} else {
					check_info.msg_id = 0;
				}
				break;

			default:
				check_info.msg_id = 0;
				break;
			}
		}
		break;

	case RS_WAIT_MIDDLE_MSG:
		if (TRMI_CONNECT == id_zone) {
			recv_msg   |= rx_data << DATA_BITS_PRE_BYTE; /* rx_data的高三位为0 */
			rmsg_state = RS_WAIT_LAST_MSG;
		} else {
			/* 采集端掉电 */
			if ((convert_id2firstbyte(TRMI_MISC_INFO) | TRMIS_TX_POWEROFF) == rx_data) {
				tx_power_off = TX_POWEROFF_CONST;
				if (tx_power_off_prev ^ tx_power_off)
					is_tx_poweroff_state_had_send = 0;
				check_info.msg_id = 0;
			}
			rmsg_state = RS_WAIT_MSG_ID;
		}
		break;

	case RS_WAIT_LAST_MSG:
		if (TRMI_CONNECT == id_zone) {
			recv_msg   |= rx_data;
			rmsg_state = RS_RECV_DATA_OVER;
		} else {
			/* 采集端掉电 */
			if ((convert_id2firstbyte(TRMI_MISC_INFO) | TRMIS_TX_POWEROFF) == rx_data) {
				tx_power_off = TX_POWEROFF_CONST;
				if (tx_power_off_prev ^ tx_power_off)
					is_tx_poweroff_state_had_send = 0;

				check_info.msg_id = 0;
			}
			rmsg_state = RS_WAIT_MSG_ID;
		}
		break;

	default:
		break;
	}

#if DEBUG_DATA_TRANS
	send_data_to_pc(creat_log_ind(0x31), 4);
	send_data_to_pc(rmsg_state, 4);
	send_data_to_pc(recv_msg_id, 4);
	send_data_to_pc(recv_msg, 4);
#endif

	if (RS_RECV_DATA_OVER == rmsg_state) {
#if DEBUG_DATA_TRANS
		send_data_to_pc(creat_log_ind(0x42), 4);
#endif
		rmsg_state = RS_WAIT_MSG_ID;
		/* do_data_proc */
		proc_org_data(recv_msg_id, recv_msg<<1);
#if USE_ORG_ADC_VALUE || DONT_INSERT_VAL
		if (convert_id2firstbyte(TRMI_PHASE_C) == recv_msg_id)
#endif
		{
			/* 以下依据DAC76321逻辑真值表来写时序 */
			/*
			 * load信号接地有风险, 详见datasheet p16--table II note 7, add by David
			 * datasheet p17--figure 15: cs, load, ldac timing
			 */
			GPIOB->BRR = GPIO_Pin_12; /* LDAC拉低 */
			GPIOA->BRR = GPIO_Pin_4;  /* CS拉低 */

			disable_dmax_y(DMA1_Channel3);
			DMA1_Channel3->CMAR  =  (u32)(&(SPI_DMA_Table_serial_in[0]));
			DMA1_Channel3->CNDTR =  6;
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

#if DEBUG_DATA_TRANS
			send_data_to_pc(creat_log_ind(0x41), 4);
#endif
		}
	}
	/* It is cleared by a software sequence (an read to the
	* USART_SR register followed by a read to the USART_DR register).
	*/
	if (is_usartx_overrun_err(USART1)) {
		id_zone = USART1->SR;
		rx_data = USART1->DR;
	}

#if DEBUG_DATA_TRANS
	USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);
	USART_ITConfig(USART2,USART_IT_TXE,ENABLE);
	send_data_to_pc(0xaa, 4);
#endif

	++fibre_optical_data_cnt;

	return;
}

#define PA_1TH_INDEX_IN_DMA_BUF (4)
#define PA_2TH_INDEX_IN_DMA_BUF (5)

#define PB_1TH_INDEX_IN_DMA_BUF (2)
#define PB_2TH_INDEX_IN_DMA_BUF (3)

#define PC_1TH_INDEX_IN_DMA_BUF (0)
#define PC_2TH_INDEX_IN_DMA_BUF (1)

/* 当前一个周期采样226个点 */
#define PX_ZERO_POS_SAMPLE_CNT_SUM (226)


/* -------------------------------------------------------------------------------------- */

/* 有效值为51.7V时，计算的波峰波谷adc值的差 */
#define DIFF_PEAK_TROUGH_MIN	(34436)

/* 有效值为57.7V时，计算的波峰波谷adc值的差 */
#define DIFF_PEAK_TROUGH_STD	(38448)

/* 有效值为57.7V时，波峰、波谷adc值 */
#define PEAK_ADC_VALUE_STD	(43750)
#define TROUGH_ADC_VALUE_STD	(5302)

#define ADC_VALUE_MAX	(0XFFFF)
#define ADC_VALUE_MIN	(0)

#if RECORD_ADC_PX_PEAK_TROUGH_INFO
volatile struct px_peak_trough_info_st pa_adc_peak_trough_info;
volatile struct px_peak_trough_info_st pb_adc_peak_trough_info;
volatile struct px_peak_trough_info_st pc_adc_peak_trough_info;
#endif

#if RECORD_DAC_PX_PEAK_TROUGH_INFO
struct px_peak_trough_sample_st {
	int cnt;
	int sum_peak;
	int sum_trough;
};
#define PX_PEAK_TROUGH_SAMPLE_NUM	(5)

volatile struct px_peak_trough_sample_st pa_peak_trough_sample;
volatile struct px_peak_trough_sample_st pb_peak_trough_sample;
volatile struct px_peak_trough_sample_st pc_peak_trough_sample;

volatile struct px_peak_trough_info_st pa_dac_peak_trough_info;
volatile struct px_peak_trough_info_st pb_dac_peak_trough_info;
volatile struct px_peak_trough_info_st pc_dac_peak_trough_info;

volatile struct px_peak_trough_info_st pa_dac_peak_trough_info_cache;
volatile struct px_peak_trough_info_st pb_dac_peak_trough_info_cache;
volatile struct px_peak_trough_info_st pc_dac_peak_trough_info_cache;
#endif

#if PROC_ESTIMATE_DELTA_Y
struct interval_for_estimate_info_st pa_interval_info;
struct interval_for_estimate_info_st pb_interval_info;
struct interval_for_estimate_info_st pc_interval_info;

struct interval_for_estimate_info_st pa_interval_info_cache;
struct interval_for_estimate_info_st pb_interval_info_cache;
struct interval_for_estimate_info_st pc_interval_info_cache;

volatile int is_need_update_wave_info;
volatile int had_change_peak_trough_val;
#endif

void init_px_peak_trough_info_st(volatile struct px_peak_trough_info_st *p)
{
	if (NULL == p)
		return;

	p->px_approx_peak_val	= PEAK_ADC_VALUE_STD;
	p->px_approx_trough_val	= TROUGH_ADC_VALUE_STD;
	p->px_approx_zero_pos	= TROUGH_ADC_VALUE_STD + (PEAK_ADC_VALUE_STD - TROUGH_ADC_VALUE_STD)/2;

	p->px_diff_peak_trough	= DIFF_PEAK_TROUGH_STD;

	p->px_instant_peak_val	 = ADC_VALUE_MIN;
	p->px_instant_trough_val = ADC_VALUE_MAX;

	return;
}


void stm32f10x_it_init(void)
{
#if RECORD_ADC_PX_PEAK_TROUGH_INFO
	init_px_peak_trough_info_st(&pa_adc_peak_trough_info);
	init_px_peak_trough_info_st(&pb_adc_peak_trough_info);
	init_px_peak_trough_info_st(&pc_adc_peak_trough_info);
#endif
#if RECORD_DAC_PX_PEAK_TROUGH_INFO
	init_px_peak_trough_info_st(&pa_dac_peak_trough_info);
	init_px_peak_trough_info_st(&pb_dac_peak_trough_info);
	init_px_peak_trough_info_st(&pc_dac_peak_trough_info);

	init_px_peak_trough_info_st(&pa_dac_peak_trough_info_cache);
	init_px_peak_trough_info_st(&pb_dac_peak_trough_info_cache);
	init_px_peak_trough_info_st(&pc_dac_peak_trough_info_cache);

#endif
#if PROC_ESTIMATE_DELTA_Y
	calc_real_interval_info(DIFF_PEAK_TROUGH_STD/2, &pa_interval_info);
	calc_real_interval_info(DIFF_PEAK_TROUGH_STD/2, &pb_interval_info);
	calc_real_interval_info(DIFF_PEAK_TROUGH_STD/2, &pc_interval_info);

	calc_real_interval_info(DIFF_PEAK_TROUGH_STD/2, &pa_interval_info_cache);
	calc_real_interval_info(DIFF_PEAK_TROUGH_STD/2, &pb_interval_info_cache);
	calc_real_interval_info(DIFF_PEAK_TROUGH_STD/2, &pc_interval_info_cache);
#endif
	return;
}

/*
 * 更新波峰、波谷的瞬时值
 * */
static void update_peak_trough(int px_org_val, volatile struct px_peak_trough_info_st *p)
{
	/* 易受干扰影响 */
	if (px_org_val < p->px_instant_trough_val)
		p->px_instant_trough_val = px_org_val;
	else if (px_org_val > p->px_instant_peak_val)
		p->px_instant_peak_val = px_org_val;
	else
		;

	return;
}

/*
 * 根据波峰、波谷的瞬时值，更新波峰、波谷的差值以及近似值
 * */
static void update_diff_peak_trough(volatile struct px_peak_trough_info_st *p,
		volatile struct px_peak_trough_sample_st *sample)
{
	int diff;
	int ins_peak, ins_trough;
	int cnt;

	/* 两个波峰波谷采样周期之间，会丢弃一次波峰波谷值 */
	cnt = sample->cnt++;

	if (cnt >= PX_PEAK_TROUGH_SAMPLE_NUM) {
		ins_peak	= sample->sum_peak / cnt;
		ins_trough	= sample->sum_trough / cnt;

		diff = ins_peak - ins_trough;
		if (DIFF_PEAK_TROUGH_MIN < diff) {
			p->px_diff_peak_trough	= diff;
			p->px_approx_peak_val	= ins_peak;
			p->px_approx_trough_val	= ins_trough;
			p->px_approx_zero_pos	= ins_trough + (ins_peak - ins_trough)/2;
		}

		sample->cnt		= 0;
		sample->sum_peak	= 0;
		sample->sum_trough	= 0;

		++had_change_peak_trough_val;
	} else {
		sample->sum_peak	+= p->px_instant_peak_val;
		sample->sum_trough	+= p->px_instant_trough_val;
	}

	p->px_instant_peak_val	 = ADC_VALUE_MIN;
	p->px_instant_trough_val = ADC_VALUE_MAX;

	return;
}


#if PROC_ESTIMATE_DELTA_Y
/*
 * 采样点的sin(x)值乘以10000，x -- [0, PI/2]
 *
 * 0到PI/2的10000*sin(x)值
 * */
static const int sin_val_of_interval[NUM_INTERVAL_FOR_SETIMATE] = {
	    0,   286,   571,   856,  1140,  1423,  1705,  1986,  2265,  2542,
	 2817,  3090,  3360,  3628,  3893,  4154,  4412,  4667,  4917,  5164,
	 5406,  5644,  5878,  6106,  6330,  6549,  6762,  6969,  7171,  7367,
	 7557,  7741,  7919,  8090,  8255,  8413,  8563,  8707,  8844,  8974,
	 9096,  9211,  9319,  9418,  9511,  9595,  9671,  9740,  9801,  9854,
	 9898,  9935,  9963,  9984,  9996	/* 大于9996的算作[9996,10000)区间， 小于0的算作[0, 286)区间 */
};

/*
 * 每个区间的最大delta-y值乘以10000, (sin(x1) - sin(x2)) * 10000
 *
 * 相邻的sin(x)差值的10000倍
 * */
static const int delta_y_max_interval[NUM_INTERVAL_FOR_SETIMATE] = {
	  286,   285,   285,   284,   283,   282,   281,   279,   277,   275,
	  273,   270,   268,   265,   261,   258,   255,   250,   247,   242,
	  238,   234,   228,   224,   219,   213,   207,   202,   196,   190,
	  184,   178,   171,   165,   158,   150,   144,   137,   130,   122,
	  115,   108,    99,    93,    84,    76,    69,    61,    53,    44,
	   37,    28,    21,    12,     4
};

/*
 * 返回处理过的px_delta_y
 * */
static int proc_estimate_delta_y(int px_delta_y, int px_sample_val,
		struct interval_for_estimate_info_st *iinfo, volatile struct px_peak_trough_info_st *p)
{
	int y;
	int abs_tmp, interval_index, delta_y_max;

	y	= px_sample_val - p->px_approx_zero_pos;
	abs_tmp	= abs(y);

	interval_index	= find_real_sin_val_interval(abs_tmp, iinfo->real_sin_val_of_interval);
	delta_y_max	= iinfo->real_delta_y_max_interval[interval_index];

	abs_tmp	= abs(px_delta_y);
	if (abs_tmp > delta_y_max) {
		if (px_delta_y < 0)
			px_delta_y = -delta_y_max/2;
		else
			px_delta_y = delta_y_max/2;
	}

	return px_delta_y;
}


static int find_real_sin_val_interval(int y, const int *interval_tbl)
{
	int low, high, mid;

	if (y <= 0) {
		return 0;
	} else if(y >= interval_tbl[NUM_INTERVAL_FOR_SETIMATE - 1]) {
		return NUM_INTERVAL_FOR_SETIMATE - 1;
	} else {
		low 	= 1;
		high	= NUM_INTERVAL_FOR_SETIMATE - 2;
		while (low <= high) {
			mid = low + (high-low)/2;

			if (y == interval_tbl[mid]) {
				break;
			} else if (y < interval_tbl[mid]) {
				high = mid - 1;
			} else {
				low = mid + 1;
			}
		}

	}

	if (low <= high)
		return mid;
	else
		return high;
}


static void calc_real_interval_info(int amplitude, struct interval_for_estimate_info_st *iinfo)
{
	int i;
	int *pd1, *pd2;
	const int *ps1, *ps2;

	pd1 = iinfo->real_sin_val_of_interval;
	pd2 = iinfo->real_delta_y_max_interval;
	ps1 = sin_val_of_interval;
	ps2 = delta_y_max_interval;
	for (i=0; i<NUM_INTERVAL_FOR_SETIMATE; ++i) {
		*pd1++ = (amplitude * *ps1++) / 10000;
		*pd2++ =( amplitude * *ps2++) / 10000;
	}

	return;
}
#endif

/*
 * 应该对应16-bits adc值
 */
void proc_org_data(unsigned int phase_id, int px_org_val)
{
#if USE_ORG_ADC_VALUE
	switch (phase_id) {
	case convert_id2firstbyte(TRMI_PHASE_A):
		SPI_DMA_Table_serial_in[PA_1TH_INDEX_IN_DMA_BUF] = get_dac_hbyte(px_org_val);
		SPI_DMA_Table_serial_in[PA_2TH_INDEX_IN_DMA_BUF] = get_dac_lbits(px_org_val);
		break;
	case convert_id2firstbyte(TRMI_PHASE_B):
		SPI_DMA_Table_serial_in[PB_1TH_INDEX_IN_DMA_BUF] = get_dac_hbyte(px_org_val);
		SPI_DMA_Table_serial_in[PB_2TH_INDEX_IN_DMA_BUF] = get_dac_lbits(px_org_val);
		break;
	case convert_id2firstbyte(TRMI_PHASE_C):
		SPI_DMA_Table_serial_in[PC_1TH_INDEX_IN_DMA_BUF] = get_dac_hbyte(px_org_val);
		SPI_DMA_Table_serial_in[PC_2TH_INDEX_IN_DMA_BUF] = get_dac_lbits(px_org_val);
		break;
	default:
		break;
	}
#else
	static int drap_sample_cnt;
	int px_delta_y;

	if (DROP_SAMPLE_CNT_MAX == drap_sample_cnt) {
		if (is_need_update_wave_info) {
			memcpy_word((int *)&pa_dac_peak_trough_info, (int *)&pa_dac_peak_trough_info_cache,
					sizeof(pa_dac_peak_trough_info)/(sizeof(int)));
			memcpy_word((int *)&pb_dac_peak_trough_info, (int *)&pb_dac_peak_trough_info_cache,
					sizeof(pb_dac_peak_trough_info)/(sizeof(int)));
			memcpy_word((int *)&pc_dac_peak_trough_info, (int *)&pc_dac_peak_trough_info_cache,
					sizeof(pc_dac_peak_trough_info)/(sizeof(int)));

			memcpy_word((int *)&pa_interval_info, (int *)&pa_interval_info,
					sizeof(pa_interval_info)/(sizeof(int)));
			memcpy_word((int *)&pb_interval_info, (int *)&pb_interval_info,
					sizeof(pb_interval_info)/(sizeof(int)));
			memcpy_word((int *)&pc_interval_info, (int *)&pc_interval_info,
					sizeof(pc_interval_info)/(sizeof(int)));

			is_need_update_wave_info = 0;
		}

		switch (phase_id) {
		case convert_id2firstbyte(TRMI_PHASE_A):
#if RECORD_ADC_PX_PEAK_TROUGH_INFO
			update_peak_trough(px_org_val, &pa_adc_peak_trough_info);
#endif
			if (is_not_doing_pa_zeropos_adj()) {
				/* 修正采集端的零位偏移 */
				px_org_val += signal_cfg_param.px_adj_param[cur_fiber_channel_no].pa_zeropos_adj_val - IDEAL_ZERO_POSITION_VALUE;
				adj_px_adc_val(real_pa_zero_position_value,
						signal_cfg_param.px_adj_param[cur_fiber_channel_no].pa_amplify_adj,
						signal_cfg_param.px_adj_param[cur_fiber_channel_no].pa_amp_factor_positive_adj);

				proc_delta_val(pa_org_val_prev, pa_delta_y_old, pa_estimate_delta_y);
#if PROC_ESTIMATE_DELTA_Y
				pa_estimate_delta_y = proc_estimate_delta_y(pa_estimate_delta_y, px_org_val,
						&pa_interval_info, &pa_dac_peak_trough_info);
#endif
				adj_px_phase(signal_cfg_param.px_adj_param[cur_fiber_channel_no].pa_phase_time_adj,
						pa_estimate_delta_y);
#if RECORD_DAC_PX_PEAK_TROUGH_INFO
				update_peak_trough(px_org_val, &pa_dac_peak_trough_info_cache);
#endif
				calc_insert_val(pa_insert_val_1, pa_insert_val_2, pa_estimate_delta_y);
			} else {
				if (pa_zeropos_adc_cnt < PX_ZERO_POS_SAMPLE_CNT_SUM) {
					pa_zeropos_sum += px_org_val;
					++pa_zeropos_adc_cnt;
					if ((PX_ZERO_POS_SAMPLE_CNT_SUM == pa_zeropos_adc_cnt)) {
						real_pa_zero_position_value = pa_zeropos_sum / PX_ZERO_POS_SAMPLE_CNT_SUM;
						real_pa_zero_position_value += signal_cfg_param.px_adj_param[cur_fiber_channel_no].pa_zeropos_adj_val
													   - IDEAL_ZERO_POSITION_VALUE;
						pa_insert_val_1 = real_pa_zero_position_value;
						pa_insert_val_2 = real_pa_zero_position_value;
						px_org_val      = real_pa_zero_position_value;
						signal_cfg_param.px_adj_param[cur_fiber_channel_no].pa_zeropos_real_val = real_pa_zero_position_value;
					} else {
						pa_insert_val_1 = px_org_val;
						pa_insert_val_2 = px_org_val;
					}
				} else {
					pa_insert_val_1 = real_pa_zero_position_value;
					pa_insert_val_2 = real_pa_zero_position_value;
					px_org_val      = real_pa_zero_position_value;
				}
			}

			SPI_DMA_Table_serial_in[PA_1TH_INDEX_IN_DMA_BUF] = get_dac_hbyte(px_org_val);
			SPI_DMA_Table_serial_in[PA_2TH_INDEX_IN_DMA_BUF] = get_dac_lbits(px_org_val);
#if 0==DONT_INSERT_VAL
			SPI_DMA_Table_serial_in[PB_1TH_INDEX_IN_DMA_BUF] = get_dac_hbyte(pb_insert_val_2);
			SPI_DMA_Table_serial_in[PB_2TH_INDEX_IN_DMA_BUF] = get_dac_lbits(pb_insert_val_2);
			SPI_DMA_Table_serial_in[PC_1TH_INDEX_IN_DMA_BUF] = get_dac_hbyte(pc_insert_val_1);
			SPI_DMA_Table_serial_in[PC_2TH_INDEX_IN_DMA_BUF] = get_dac_lbits(pc_insert_val_1);
#endif
			break;

		case convert_id2firstbyte(TRMI_PHASE_B):
#if RECORD_ADC_PX_PEAK_TROUGH_INFO
			update_peak_trough(px_org_val, &pb_adc_peak_trough_info);
#endif

			if (is_not_doing_pb_zeropos_adj()) {
				/* 修正采集端的零位偏移 */
				px_org_val += signal_cfg_param.px_adj_param[cur_fiber_channel_no].pb_zeropos_adj_val - IDEAL_ZERO_POSITION_VALUE;
				adj_px_adc_val(real_pb_zero_position_value,
						signal_cfg_param.px_adj_param[cur_fiber_channel_no].pb_amplify_adj,
						signal_cfg_param.px_adj_param[cur_fiber_channel_no].pb_amp_factor_positive_adj);

				proc_delta_val(pb_org_val_prev, pb_delta_y_old, pb_estimate_delta_y);
#if PROC_ESTIMATE_DELTA_Y
				pb_estimate_delta_y = proc_estimate_delta_y(pb_estimate_delta_y, px_org_val,
						&pb_interval_info, &pb_dac_peak_trough_info);
#endif
				adj_px_phase(signal_cfg_param.px_adj_param[cur_fiber_channel_no].pb_phase_time_adj,
						pb_estimate_delta_y);
#if RECORD_DAC_PX_PEAK_TROUGH_INFO
				update_peak_trough(px_org_val, &pb_dac_peak_trough_info_cache);
#endif
				calc_insert_val(pb_insert_val_1, pb_insert_val_2, pb_estimate_delta_y);
			} else {
				if (pb_zeropos_adc_cnt < PX_ZERO_POS_SAMPLE_CNT_SUM) {
					pb_zeropos_sum += px_org_val;
					++pb_zeropos_adc_cnt;
					if ((PX_ZERO_POS_SAMPLE_CNT_SUM == pb_zeropos_adc_cnt)) {
						real_pb_zero_position_value = pb_zeropos_sum / PX_ZERO_POS_SAMPLE_CNT_SUM;
						real_pb_zero_position_value += signal_cfg_param.px_adj_param[cur_fiber_channel_no].pb_zeropos_adj_val
													   - IDEAL_ZERO_POSITION_VALUE;
						pb_insert_val_1 = real_pb_zero_position_value;
						pb_insert_val_2 = real_pb_zero_position_value;
						px_org_val      = real_pb_zero_position_value;
						signal_cfg_param.px_adj_param[cur_fiber_channel_no].pb_zeropos_real_val = real_pb_zero_position_value;
					} else {
						pb_insert_val_1 = px_org_val;
						pb_insert_val_2 = px_org_val;
					}
				} else {
					pb_insert_val_1 = real_pb_zero_position_value;
					pb_insert_val_2 = real_pb_zero_position_value;
					px_org_val      = real_pb_zero_position_value;
				}

			}

#if 0==DONT_INSERT_VAL
			SPI_DMA_Table_serial_in[PA_1TH_INDEX_IN_DMA_BUF] = get_dac_hbyte(pa_insert_val_1);
			SPI_DMA_Table_serial_in[PA_2TH_INDEX_IN_DMA_BUF] = get_dac_lbits(pa_insert_val_1);
#endif
			SPI_DMA_Table_serial_in[PB_1TH_INDEX_IN_DMA_BUF] = get_dac_hbyte(px_org_val);
			SPI_DMA_Table_serial_in[PB_2TH_INDEX_IN_DMA_BUF] = get_dac_lbits(px_org_val);
#if 0==DONT_INSERT_VAL
			SPI_DMA_Table_serial_in[PC_1TH_INDEX_IN_DMA_BUF] = get_dac_hbyte(pc_insert_val_2);
			SPI_DMA_Table_serial_in[PC_2TH_INDEX_IN_DMA_BUF] = get_dac_lbits(pc_insert_val_2);
#endif
			break;

		case convert_id2firstbyte(TRMI_PHASE_C):
#if RECORD_ADC_PX_PEAK_TROUGH_INFO
			update_peak_trough(px_org_val, &pc_adc_peak_trough_info);
#endif
			if (is_not_doing_pc_zeropos_adj()) {
				/* 修正采集端的零位偏移 */
				px_org_val += signal_cfg_param.px_adj_param[cur_fiber_channel_no].pc_zeropos_adj_val - IDEAL_ZERO_POSITION_VALUE;
				adj_px_adc_val(real_pc_zero_position_value,
						signal_cfg_param.px_adj_param[cur_fiber_channel_no].pc_amplify_adj,
						signal_cfg_param.px_adj_param[cur_fiber_channel_no].pc_amp_factor_positive_adj);

				proc_delta_val(pc_org_val_prev, pc_delta_y_old, pc_estimate_delta_y);
#if PROC_ESTIMATE_DELTA_Y
				pc_estimate_delta_y = proc_estimate_delta_y(pc_estimate_delta_y, px_org_val,
						&pc_interval_info, &pc_dac_peak_trough_info);
#endif
				adj_px_phase(signal_cfg_param.px_adj_param[cur_fiber_channel_no].pc_phase_time_adj,
						pc_estimate_delta_y);
#if RECORD_DAC_PX_PEAK_TROUGH_INFO
				update_peak_trough(px_org_val, &pc_dac_peak_trough_info_cache);
#endif
				calc_insert_val(pc_insert_val_1, pc_insert_val_2, pc_estimate_delta_y);
			} else {
				if (pc_zeropos_adc_cnt < PX_ZERO_POS_SAMPLE_CNT_SUM) {
					pc_zeropos_sum += px_org_val;
					++pc_zeropos_adc_cnt;
					if ((PX_ZERO_POS_SAMPLE_CNT_SUM == pc_zeropos_adc_cnt)) {
						real_pc_zero_position_value = pc_zeropos_sum / PX_ZERO_POS_SAMPLE_CNT_SUM;
						real_pc_zero_position_value += signal_cfg_param.px_adj_param[cur_fiber_channel_no].pc_zeropos_adj_val
													   - IDEAL_ZERO_POSITION_VALUE;
						pc_insert_val_1 = real_pc_zero_position_value;
						pc_insert_val_2 = real_pc_zero_position_value;
						px_org_val      = real_pc_zero_position_value;
						signal_cfg_param.px_adj_param[cur_fiber_channel_no].pc_zeropos_real_val = real_pc_zero_position_value;
					} else {
						pc_insert_val_1 = px_org_val;
						pc_insert_val_2 = px_org_val;
					}
				} else {
					pc_insert_val_1 = real_pc_zero_position_value;
					pc_insert_val_2 = real_pc_zero_position_value;
					px_org_val      = real_pc_zero_position_value;
				}

			}

#if 0==DONT_INSERT_VAL
			SPI_DMA_Table_serial_in[PA_1TH_INDEX_IN_DMA_BUF] = get_dac_hbyte(pa_insert_val_2);
			SPI_DMA_Table_serial_in[PA_2TH_INDEX_IN_DMA_BUF] = get_dac_lbits(pa_insert_val_2);
			SPI_DMA_Table_serial_in[PB_1TH_INDEX_IN_DMA_BUF] = get_dac_hbyte(pb_insert_val_1);
			SPI_DMA_Table_serial_in[PB_2TH_INDEX_IN_DMA_BUF] = get_dac_lbits(pb_insert_val_1);
#endif
			SPI_DMA_Table_serial_in[PC_1TH_INDEX_IN_DMA_BUF] = get_dac_hbyte(px_org_val);
			SPI_DMA_Table_serial_in[PC_2TH_INDEX_IN_DMA_BUF] = get_dac_lbits(px_org_val);
			break;

		default:
			break;
		}
	} else {
		switch (phase_id) {
		case convert_id2firstbyte(TRMI_PHASE_A):
#if RECORD_ADC_PX_PEAK_TROUGH_INFO
			update_peak_trough(px_org_val, &pa_adc_peak_trough_info);
#endif
			px_delta_y = px_org_val - pa_org_val_prev;
			pa_org_val_prev = px_org_val;
			UPDATE_OLD_DELTA_Y(px_delta_y, pa_delta_y_old);

#if RECORD_DAC_PX_PEAK_TROUGH_INFO
			update_peak_trough(px_org_val, &pa_dac_peak_trough_info_cache);
#endif
			pa_estimate_delta_y = ESTIMATE_DELTA_Y_VALUE((pa_delta_y_old));
			(pa_insert_val_1) = px_org_val + pa_estimate_delta_y / 3;
			(pa_insert_val_2) = px_org_val + (pa_estimate_delta_y*2) / 3;
			break;

		case convert_id2firstbyte(TRMI_PHASE_B):
#if RECORD_ADC_PX_PEAK_TROUGH_INFO
			update_peak_trough(px_org_val, &pb_adc_peak_trough_info);
#endif
			px_delta_y = px_org_val - pb_org_val_prev;
			pb_org_val_prev = px_org_val;
			UPDATE_OLD_DELTA_Y(px_delta_y, pb_delta_y_old);

#if RECORD_DAC_PX_PEAK_TROUGH_INFO
			update_peak_trough(px_org_val, &pb_dac_peak_trough_info_cache);
#endif
			pb_estimate_delta_y = ESTIMATE_DELTA_Y_VALUE((pb_delta_y_old));
			(pb_insert_val_1) = px_org_val + pb_estimate_delta_y / 3;
			(pb_insert_val_2) = px_org_val + (pb_estimate_delta_y*2) / 3;
			break;

		case convert_id2firstbyte(TRMI_PHASE_C):
#if RECORD_ADC_PX_PEAK_TROUGH_INFO
			update_peak_trough(px_org_val, &pc_adc_peak_trough_info);
#endif
			px_delta_y = px_org_val - pc_org_val_prev;
			pc_org_val_prev = px_org_val;
			UPDATE_OLD_DELTA_Y(px_delta_y, pc_delta_y_old);

#if RECORD_DAC_PX_PEAK_TROUGH_INFO
			update_peak_trough(px_org_val, &pc_dac_peak_trough_info_cache);
#endif
			pc_estimate_delta_y = ESTIMATE_DELTA_Y_VALUE((pc_delta_y_old));
			(pc_insert_val_1) = px_org_val + pc_estimate_delta_y / 3;
			(pc_insert_val_2) = px_org_val + (pc_estimate_delta_y*2) / 3;
			break;

		default:
			break;
		}

		++drap_sample_cnt;
		SPI_DMA_Table_serial_in[PA_1TH_INDEX_IN_DMA_BUF] = get_dac_hbyte(real_pa_zero_position_value);
		SPI_DMA_Table_serial_in[PA_2TH_INDEX_IN_DMA_BUF] = get_dac_lbits(real_pa_zero_position_value);
		SPI_DMA_Table_serial_in[PB_1TH_INDEX_IN_DMA_BUF] = get_dac_hbyte(real_pb_zero_position_value);
		SPI_DMA_Table_serial_in[PB_2TH_INDEX_IN_DMA_BUF] = get_dac_lbits(real_pb_zero_position_value);
		SPI_DMA_Table_serial_in[PC_1TH_INDEX_IN_DMA_BUF] = get_dac_hbyte(real_pc_zero_position_value);
		SPI_DMA_Table_serial_in[PC_2TH_INDEX_IN_DMA_BUF] = get_dac_lbits(real_pc_zero_position_value);
	}
#endif
	return;
}


#define TIM_cnt (2)
#define TIM_1S	(125)

extern void overload_exti_init(void);


/*******************************************************************************
* Function Name  : TIM3_IRQHandler
* Description    : This function handles TIM3 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM3_IRQHandler(void)
{
	static int sample_cnt;
	static int delay_cnt;
	static unsigned int delay_powerup;
	static int rx_continue_overload_cnt_had_send;
#if USE_OPTICX_200S_VERSION
	static enum sys_use_channel_e current_use_channel = SUC_BUTT;
	enum sys_use_channel_e cur_channel;

	extern enum sys_use_channel_e get_cur_use_channel(void);
#endif
	unsigned int cpu_temper_adc;

	/* Clear the interrupt pending flag */
	TIM3->SR = (uint16_t)~TIM_FLAG_Update;
#if USE_STM32_IWDG
	/* Reloads IWDG counter with value defined in the reload register */
	/* #define KR_KEY_Reload    ((uint16_t)0xAAAA) */
	IWDG->KR = 0xAAAA;
#endif

	++tim3_8ms_cnt;

	if (fibre_optical_int_cnt4clr_dac < 3) {
		clr_dac();
//		tx_soft_version[3]  = 0; /* 不断发送版本号, 在接收端不断电时, 采集端可能会改变 */
//		tx_dev_sn_buf_state = 0; /* 不断发送sn号, 在接收端不断电时, 采集端可能会改变 */
//		non_change_txe_info_send_cnt = 0;
	}

	fibre_optical_int_cnt4clr_dac = 0;

	if (delay_cnt < (3*TIM3_INT_CNT_PER1S)) {
		/* delay 3s */
		++delay_cnt;
		return;
	}

	/* 延迟给110v电路供电 */
	if(1 != delay_powerup) {
		turnon_110v_power();
		delay_powerup = 1;
		overload_exti_init();
	}
#if USE_PVD
	if (is_system_powerdown) {
		if ((tim3_8ms_cnt - sys_powerdown_delay4confirm) >= 125) {
			/* 如果掉电后1000ms, 系统还在运行, 说明电源有波动, 此时应恢复系统 */
			disable_protect_electric_relay_pin();
			turnon_110v_power();

			is_system_powerdown = 0;
			sys_powerdown_delay4confirm = 0;

			send_data_to_pc(0xaa55aa01, 4);

		} else {
			return;
		}
	}
#endif

#if USE_NEW_RX_OVERLOAD_ALGORITHM
	if (0 != rx_continue_overload_cnt) {
		if ((rx_continue_overload_cnt < 5) && ((tim3_8ms_cnt - rx_continue_overload_delay) >= ((2*(rx_continue_overload_cnt)) * 125)) ) {
			if (rx_continue_overload_cnt_prev == rx_continue_overload_cnt) {
				rx_continue_overload_cnt_prev = rx_continue_overload_cnt = 0;
			} else {
				rx_continue_overload_cnt_prev = rx_continue_overload_cnt;
				rx_continue_overload_delay    = tim3_8ms_cnt;
				turnon_110v_power();
			}
		} else if ((rx_continue_overload_cnt >= 5) && ((tim3_8ms_cnt - rx_continue_overload_delay) >= (10 * 125))) {
			/* per 10s */
			if (rx_continue_overload_cnt_prev == rx_continue_overload_cnt) {
				rx_continue_overload_cnt_prev = rx_continue_overload_cnt = 0;
			} else {
				rx_continue_overload_cnt_prev = rx_continue_overload_cnt;
				rx_continue_overload_delay    = tim3_8ms_cnt;
				turnon_110v_power();
			}
		} else {
			/* 未超时 */
		}
	}

	if (rx_continue_overload_cnt_had_send != rx_continue_overload_cnt) {
		write_byte2lcd_cpu(creat_info_id(ITID_RXE_INFO, TRXII_RX_CONTINUE_OVERLOAD_CNT));
		if (0 != rx_continue_overload_cnt) /* 发送增量值 */
			write_byte2lcd_cpu((u8)((rx_continue_overload_cnt - rx_continue_overload_cnt_had_send) & 0x0f));
		else
			write_byte2lcd_cpu(0); /* state ok */

		rx_continue_overload_cnt_had_send = rx_continue_overload_cnt;
	}
#else
	if(0 != overcurrent_door_judge) {
		if((TIM_cnt*TIM_1S) == (tim3_8ms_cnt-overcurrent_cnt)) {
			turnon_110v_power();
			if(delay_overcurrent_state>=6) {
				delay_overcurrent_state=0;
				turnoff_110v_power();
				overcurrent_door_judge=0;
			}
		}
	}

	if(((TIM_cnt+1)*TIM_1S) == (tim3_8ms_cnt-overcurrent_cnt)) {
		delay_overcurrent_state=0;
		overcurrent_door_judge=0;
	}
#endif

	DMA1_Channel1->CCR &= CCR_ENABLE_Reset; /* Disable the selected DMAy Channelx */

	if (tim3_8ms_cnt%10 == 0)
		check_if_rx_px_fault();

	cpu_temper_adc = ADC_GetInjectedConversionValue(ADC1 , ADC_InjectedChannel_1);
	if (0 == sample_cnt) {
		box_temper_sum = 0;
		pa_avg_val_sum = 0;
		pb_avg_val_sum = 0;
		pc_avg_val_sum = 0;
		cpu_temper_sum = 0;

		box_temper_max = adc_buf[0];
		box_temper_min = adc_buf[0];
		pa_avg_val_max = adc_buf[3];
		pa_avg_val_min = adc_buf[3];
		pb_avg_val_max = adc_buf[2];
		pb_avg_val_min = adc_buf[2];
		pc_avg_val_max = adc_buf[1];
		pc_avg_val_min = adc_buf[1];
		cpu_temper_max = cpu_temper_adc;
		cpu_temper_min = cpu_temper_adc;

		//check_if_rx_px_fault();
	} else if (ADC_CNT_PER_SAMPLE/2 == sample_cnt) {
		send_tx_checkinfo();
	}

	box_temper_sum += adc_buf[0];
	pc_avg_val_sum += adc_buf[1];
	pb_avg_val_sum += adc_buf[2];
	pa_avg_val_sum += adc_buf[3];
	cpu_temper_sum += cpu_temper_adc;

	if (box_temper_max < adc_buf[0])
		box_temper_max = adc_buf[0];
	else if (box_temper_min > adc_buf[0])
		box_temper_min = adc_buf[0];

	if (pa_avg_val_max < adc_buf[3])
		pa_avg_val_max = adc_buf[3];
	else if (pa_avg_val_min > adc_buf[3])
		pa_avg_val_min = adc_buf[3];

	if (pb_avg_val_max < adc_buf[2])
		pb_avg_val_max = adc_buf[2];
	else if (pb_avg_val_min > adc_buf[2])
		pb_avg_val_min = adc_buf[2];


	if (pc_avg_val_max < adc_buf[1])
		pc_avg_val_max = adc_buf[1];
	else if (pc_avg_val_min > adc_buf[1])
		pc_avg_val_min = adc_buf[1];

	if (cpu_temper_max < cpu_temper_adc)
		cpu_temper_max = cpu_temper_adc;
	else if (cpu_temper_min > cpu_temper_adc)
		cpu_temper_min = cpu_temper_adc;

	++sample_cnt;

	if (ADC_CNT_PER_SAMPLE == sample_cnt) {
		proc_adc_data_and_send();
		sample_cnt = 0;
	}

#if USE_OPTICX_200S_VERSION

	cur_channel = get_cur_use_channel();
	if (current_use_channel != cur_channel) {
		current_use_channel = cur_channel;

		switch (cur_channel) {
		case SUC_PT_CHANNEL:
			/* set_to_use_optical_fiber1(); 如果切换到pt, 则保留之前使用的光纤通道
			 * 与该强制切换到pt的对应取消放到get_cur_use_channel()函数中
			 * */
			force_output_switch2pt(UIUS_CHANNEL_INDICATION);
			break;

		case SUC_FIBER_CHANNEL_1:
			set_to_use_optical_fiber1();
			break;

		case SUC_FIBER_CHANNEL_2:
			set_to_use_optical_fiber2();
			break;

		default:
			/* get_cur_use_channel()只可能返回上面三个值之一 */
			break;
		}
		write_byte2lcd_cpu(creat_info_id(ITID_RXE_INFO, TRXII_USE_FIBER_NO));
		write_byte2lcd_cpu(cur_channel);
	}
#endif


	DMA1->IFCR = DMA1_IT_GL1; /* Clear the selected DMA interrupt pending bits */
	DMA1_Channel1->CNDTR = 4; /* Write to DMAy Channelx CNDTR */
	DMA1_Channel1->CCR |= DMA_IT_TC;
	DMA1_Channel1->CCR |= CCR_ENABLE_Set; /* Enable the selected DMAy Channelx */

	/* 人工打开ADC转换.*/
	ADC1->CR2 |= CR2_EXTTRIG_SWSTART_Set;

	/* 1016ms更新一次 */
	if (0 == (tim3_8ms_cnt & ((1<<7)-1))) {
#if RECORD_ADC_PX_PEAK_TROUGH_INFO
		update_diff_peak_trough(&pa_adc_peak_trough_info);
		update_diff_peak_trough(&pb_adc_peak_trough_info);
		update_diff_peak_trough(&pc_adc_peak_trough_info);
#endif
#if RECORD_DAC_PX_PEAK_TROUGH_INFO
		update_diff_peak_trough(&pa_dac_peak_trough_info_cache, &pa_peak_trough_sample);
		update_diff_peak_trough(&pb_dac_peak_trough_info_cache, &pb_peak_trough_sample);
		update_diff_peak_trough(&pc_dac_peak_trough_info_cache, &pc_peak_trough_sample);
#endif
#if PROC_ESTIMATE_DELTA_Y
		if (had_change_peak_trough_val >= 3) {
			calc_real_interval_info(pa_dac_peak_trough_info.px_diff_peak_trough/2,
					&pa_interval_info_cache);
			calc_real_interval_info(pb_dac_peak_trough_info.px_diff_peak_trough/2,
					&pb_interval_info_cache);
			calc_real_interval_info(pc_dac_peak_trough_info.px_diff_peak_trough/2,
					&pc_interval_info_cache);

			is_need_update_wave_info = 1;
			had_change_peak_trough_val = 0;
		}
#endif
	}

	return;
}

/*
ADC1, ADC_Channel_0->PA0->恒温盒内温度   1
ADC1, ADC_Channel_1->PA1->C相平均值      2
ADC1, ADC_Channel_8->PB0->B相平均值      3
ADC1, ADC_Channel_9->PB1->A相平均值      4
*/
static void proc_adc_data_and_send(void)
{
	static unsigned int pa_avg_val_prev_send;
	static unsigned int pb_avg_val_prev_send;
	static unsigned int pc_avg_val_prev_send;
	static unsigned int cpu_temper_prev_send;
	//static unsigned int cpu_temper_old;

	static int ver_no_send_cnt;

	int i;

	box_temper_sum -= box_temper_max + box_temper_min;
	pa_avg_val_sum -= pa_avg_val_max + pa_avg_val_min;
	pb_avg_val_sum -= pb_avg_val_max + pb_avg_val_min;
	pc_avg_val_sum -= pc_avg_val_max + pc_avg_val_min;
	cpu_temper_sum -= cpu_temper_max + cpu_temper_min;

	box_temper_sum /= ADC_CNT_PER_SAMPLE - 2;
	box_temper_adc4tctrl = box_temper_sum;
	pa_avg_val_sum4read = pa_avg_val_sum / (ADC_CNT_PER_SAMPLE - 2);
	pb_avg_val_sum4read = pb_avg_val_sum / (ADC_CNT_PER_SAMPLE - 2);
	pc_avg_val_sum4read = pc_avg_val_sum / (ADC_CNT_PER_SAMPLE - 2);
	cpu_temper_sum /= ADC_CNT_PER_SAMPLE - 2;

	/* judge_px_over_limit(); */

	if (ver_no_send_cnt < 2) {
		++ver_no_send_cnt;

		/*接收端软件版本号(24bit)  ID-84   */
		write_byte2lcd_cpu(creat_info_id(ITID_RXE_INFO, TRXII_VERSION));
		write_byte2lcd_cpu((u8)((RX_VERSION>>4)    & 0xf));
		write_byte2lcd_cpu((u8)((RX_VERSION)       & 0xf));
		write_byte2lcd_cpu((u8)((RX_SUBVERSION>>4) & 0xf));
		write_byte2lcd_cpu((u8)((RX_SUBVERSION)    & 0xf));
		write_byte2lcd_cpu((u8)((RX_REVISION>>4)   & 0xf));
		write_byte2lcd_cpu((u8)((RX_REVISION)      & 0xf));
	}

	if (non_change_txe_info_send_cnt < 3) {
		if (1 == tx_soft_version[3]) {
			/* 发射端软件版本号(24bit)  ID-74   */
			write_byte2lcd_cpu(creat_info_id(ITID_TXE_INFO, TRXII_VERSION));
			write_byte2lcd_cpu((u8)((tx_soft_version[0]>>4) & 0xf));
			write_byte2lcd_cpu((u8)((tx_soft_version[0])    & 0xf));
			write_byte2lcd_cpu((u8)((tx_soft_version[1]>>4) & 0xf));
			write_byte2lcd_cpu((u8)((tx_soft_version[1])    & 0xf));
			write_byte2lcd_cpu((u8)((tx_soft_version[2]>>4) & 0xf));
			write_byte2lcd_cpu((u8)((tx_soft_version[2])    & 0xf));

			tx_soft_version[3] = 2;
		}
//		tx_soft_version_using = 0;

		if (2 == tx_dev_sn_buf_state) {
			/* 发射端SN(DEV_SN_MODE_LEN bytes)  ID-76   */
			write_byte2lcd_cpu(creat_info_id(ITID_TXE_INFO, TRXII_DEV_SN));
			for (i=0; i<DEV_SN_MODE_LEN; ++i) {
				write_byte2lcd_cpu((u8)((tx_dev_sn[i]>>4) & 0xf));
				write_byte2lcd_cpu((u8)((tx_dev_sn[i])    & 0xf));
			}

			tx_dev_sn_buf_state = 3;
		}

		if (2==tx_soft_version[3] && 3==tx_dev_sn_buf_state) {
			++non_change_txe_info_send_cnt;
			clr_bit(need_send_some_info2lcdcpu, SEND_SOFT_VERSION2LCD_CPU_BIT_MASK);
			clr_bit(need_send_some_info2lcdcpu, SEND_DEV_SN2LCD_CPU_BIT_MASK);
			tx_soft_version[3]  = 0;
			tx_dev_sn_buf_state = 0;
		}
	} else if (is_bit_set(need_send_some_info2lcdcpu, SEND_SOFT_VERSION2LCD_CPU_BIT_MASK)) {
		/* 发射端软件版本号(24bit)  ID-74   */
		write_byte2lcd_cpu(creat_info_id(ITID_TXE_INFO, TRXII_VERSION));
		write_byte2lcd_cpu((u8)((tx_soft_version[0]>>4) & 0xf));
		write_byte2lcd_cpu((u8)((tx_soft_version[0])    & 0xf));
		write_byte2lcd_cpu((u8)((tx_soft_version[1]>>4) & 0xf));
		write_byte2lcd_cpu((u8)((tx_soft_version[1])    & 0xf));
		write_byte2lcd_cpu((u8)((tx_soft_version[2]>>4) & 0xf));
		write_byte2lcd_cpu((u8)((tx_soft_version[2])    & 0xf));

		clr_bit(need_send_some_info2lcdcpu, SEND_SOFT_VERSION2LCD_CPU_BIT_MASK);
		tx_soft_version[3]  = 0;
	} else if (is_bit_set(need_send_some_info2lcdcpu, SEND_DEV_SN2LCD_CPU_BIT_MASK)) {
		/* 发射端SN(DEV_SN_MODE_LEN bytes)  ID-76   */
		write_byte2lcd_cpu(creat_info_id(ITID_TXE_INFO, TRXII_DEV_SN));
		for (i=0; i<DEV_SN_MODE_LEN; ++i) {
			write_byte2lcd_cpu((u8)((tx_dev_sn[i]>>4) & 0xf));
			write_byte2lcd_cpu((u8)((tx_dev_sn[i])    & 0xf));
		}

		clr_bit(need_send_some_info2lcdcpu, SEND_DEV_SN2LCD_CPU_BIT_MASK);
		tx_dev_sn_buf_state = 0;
	}


	if (sub_abs(box_temper_prev_send, box_temper_sum) > 10) {
		box_temper_prev_send = box_temper_sum;
		/*保温盒温度数据(16bit)  ID-83   */
		write_byte2lcd_cpu(creat_info_id(ITID_RXE_INFO, TRXII_BOX_TEMP));
		write_byte2lcd_cpu((u8)((box_temper_prev_send>>12) & 0xf));
		write_byte2lcd_cpu((u8)((box_temper_prev_send>>8)  & 0xf));
		write_byte2lcd_cpu((u8)((box_temper_prev_send>>4)  & 0xf));
		write_byte2lcd_cpu((u8)(box_temper_prev_send       & 0xf));
	}

	if (pa_avg_val_prev_send != pa_avg_val_sum4read) {
		pa_avg_val_prev_send = pa_avg_val_sum4read;
		/*接收端A相均值(16bit)  ID-42   */
		write_byte2lcd_cpu(creat_info_id(ITID_RXPA_INFO, PII_AVG_VAL));
		write_byte2lcd_cpu((u8)((pa_avg_val_prev_send>>12) & 0xf));
		write_byte2lcd_cpu((u8)((pa_avg_val_prev_send>>8)  & 0xf));
		write_byte2lcd_cpu((u8)((pa_avg_val_prev_send>>4)  & 0xf));
		write_byte2lcd_cpu((u8)(pa_avg_val_prev_send       & 0xf));
	}

	if (pb_avg_val_prev_send != pb_avg_val_sum4read) {
		pb_avg_val_prev_send = pb_avg_val_sum4read;
		/*接收端B相均值(16bit)  ID-52   */
		write_byte2lcd_cpu(creat_info_id(ITID_RXPB_INFO, PII_AVG_VAL));
		write_byte2lcd_cpu((u8)((pb_avg_val_prev_send>>12) & 0xf));
		write_byte2lcd_cpu((u8)((pb_avg_val_prev_send>>8)  & 0xf));
		write_byte2lcd_cpu((u8)((pb_avg_val_prev_send>>4)  & 0xf));
		write_byte2lcd_cpu((u8)(pb_avg_val_prev_send       & 0xf));
	}

	if (pc_avg_val_prev_send != pc_avg_val_sum4read) {
		pc_avg_val_prev_send = pc_avg_val_sum4read;
		/*接收端C相均值(16bit)  ID-62   */
		write_byte2lcd_cpu(creat_info_id(ITID_RXPC_INFO, PII_AVG_VAL));
		write_byte2lcd_cpu((u8)((pc_avg_val_prev_send>>12) & 0xf));
		write_byte2lcd_cpu((u8)((pc_avg_val_prev_send>>8)  & 0xf));
		write_byte2lcd_cpu((u8)((pc_avg_val_prev_send>>4)  & 0xf));
		write_byte2lcd_cpu((u8)(pc_avg_val_prev_send       & 0xf));
	}


	if (sub_abs(cpu_temper_prev_send, cpu_temper_sum) > 10) {
		cpu_temper_prev_send = cpu_temper_sum;
#if 1
		/*接收端CPU温度(16bit)  ID-82   */
		write_byte2lcd_cpu(creat_info_id(ITID_RXE_INFO, TRXII_CPU_TEMP));
		write_byte2lcd_cpu((u8)((cpu_temper_prev_send>>12) & 0xf));
		write_byte2lcd_cpu((u8)((cpu_temper_prev_send>>8)  & 0xf));
		write_byte2lcd_cpu((u8)((cpu_temper_prev_send>>4)  & 0xf));
		write_byte2lcd_cpu((u8)(cpu_temper_prev_send       & 0xf));

		/* T = (V25 - Vsense)/(Avg_slope) + 25
		 * V25 = 1.43v
		 * Avg_slope = 4.3 mv/度
		 */
#if 0
		cpu_temper = (143 - (int)cpu_temper_prev_send*330/4095) * 1000/430 + 25;
#else
		cpu_temper = (140 - (int)cpu_temper_prev_send*330/4095) * 1000/460 + 25;
#endif
#else
		if (cpu_temper_old != cpu_temper) {
			cpu_temper_old = cpu_temper;
			/*接收端CPU温度(16bit)  ID-82   */
			write_byte2lcd_cpu(creat_info_id(ITID_RXE_INFO, TRXII_CPU_TEMP));
			write_byte2lcd_cpu((u8)((cpu_temper>>12) & 0xf));
			write_byte2lcd_cpu((u8)((cpu_temper>>8)  & 0xf));
			write_byte2lcd_cpu((u8)((cpu_temper>>4)  & 0xf));
			write_byte2lcd_cpu((u8)(cpu_temper       & 0xf));
		}
#endif
	}

	return;
}

#if 0
/*
 * 57.7  +-15%
 *
 * 57700 * 0.85 = 49045 (mv) --- 1747.86(adc)
 * 57700 * 1.15 = 66355 (mv) --- 2364.75(adc)
 *
 * 50000 mv -- 1781.89(adc)
 * 65000 mv -- 2316.46(adc)
 *
 * 2055(adc) 	-- 57660mv
 * 1(adc)	-- 28.06mv
 */
/*
 * E:\notepad-work\assist-eswd\zvd>calc_vol_adc.exe
 * pls input std voltage(mv):57700
 * min-std-max:46160.000000, 57700.000000, 69240.000000
 * pls input (vol-mv, adc)1:47950 1632
 * pls input (vol-mv, adc)2:60760 2069
 * pls input (vol-mv, adc)3:67880 2311
 * xmv_per_adc1-3: 29.381127, 29.366844, 29.372566
 * xmv_per_adc:29.373512
 * voltage(min, std, max):49045.000000, 57700.000000, 66355.000000
 * voltage-adc(min, std, max):1669.701575, 1964.354795, 2259.008014
 *
 * 2258 -- 0x8D2
 * 1670 -- 0x686
 */
#define PX_VOLTAGE_ADC_MAX (2258)
#define PX_VOLTAGE_ADC_MIN (1670)

#define is_px_voltage_overlimit(px_voltage) (px_voltage>PX_VOLTAGE_ADC_MAX || px_voltage<PX_VOLTAGE_ADC_MIN)

/*
 * 该函数是为了防止输出电压在硬件过限判断的临界点时, 继电器频繁切换
 *
 * 大约1s调用一次
 */
static void judge_px_over_limit(void)
{
#if 0
	static int continue_overlimit_cnt;
	static int is_had_protect_electric_relay;

	if (is_px_voltage_overlimit(pa_avg_val_sum4read) || is_px_voltage_overlimit(pb_avg_val_sum4read)
		|| is_px_voltage_overlimit(pc_avg_val_sum4read)) {
		if (!is_had_protect_electric_relay && (++continue_overlimit_cnt >= 2)) {
			is_had_protect_electric_relay = 1;
			enable_protect_electric_relay_pin();
			clr_port_pin(protect_electric_relay_gpio, protect_electric_relay_pin);
		}
	} else if (1==is_had_protect_electric_relay) {
		continue_overlimit_cnt = 0;
		is_had_protect_electric_relay	= 0;
		disable_protect_electric_relay_pin();
	}
#else
	if (is_px_voltage_overlimit(pa_avg_val_sum4read) || is_px_voltage_overlimit(pb_avg_val_sum4read)
		|| is_px_voltage_overlimit(pc_avg_val_sum4read)) {
		enable_protect_electric_relay_pin();
		clr_port_pin(protect_electric_relay_gpio, protect_electric_relay_pin);
	} else {
		disable_protect_electric_relay_pin();
	}

#endif
	return;
}
#endif

/*
 * 版本号在proc_adc_data_and_send()函数中处理
 */
static void send_tx_checkinfo(void)
{
	static int is_not_first;

	if (sub_abs(tx_cpu_temper_prev, tx_cpu_temper) > 10) {
		tx_cpu_temper_prev = tx_cpu_temper;
		/* 发送端CPU温度(16bit)  ID-72 */
		write_byte2lcd_cpu(creat_info_id(ITID_TXE_INFO, TRXII_CPU_TEMP));
		write_byte2lcd_cpu((u8)((tx_cpu_temper_prev>>12) & 0x0f));
		write_byte2lcd_cpu((u8)((tx_cpu_temper_prev>>8)  & 0x0f));
		write_byte2lcd_cpu((u8)((tx_cpu_temper_prev>>4)  & 0x0f));
		write_byte2lcd_cpu((u8)(tx_cpu_temper_prev       & 0x0f));
	}
	tx_cpu_temper_using = 0;

	if (sub_abs(tx_box_temper_prev, tx_box_temper) > 10) {
		tx_box_temper_prev = tx_box_temper;
		/* 发送端保温盒温度数据(16bit)  ID-73   */
		write_byte2lcd_cpu(creat_info_id(ITID_TXE_INFO, TRXII_BOX_TEMP));
		write_byte2lcd_cpu((u8)((tx_box_temper_prev>>12) & 0x0f));
		write_byte2lcd_cpu((u8)((tx_box_temper_prev>>8)  & 0x0f));
		write_byte2lcd_cpu((u8)((tx_box_temper_prev>>4)  & 0x0f));
		write_byte2lcd_cpu((u8)(tx_box_temper_prev       & 0x0f));
	}
	tx_box_temper_using = 0;

	if (is_not_first) {
		if (tx_pa_check_bin_info_prev != tx_pa_check_bin_info) {
			tx_pa_check_bin_info_prev = tx_pa_check_bin_info;
			write_byte2lcd_cpu(creat_info_id(ITID_TXPA_INFO, PII_BINARY_INFO));
			write_byte2lcd_cpu((u8)((tx_pa_check_bin_info_prev>>1) & 0x0f));
		}
		tx_pa_check_bin_info_using = 0;

		if (tx_pb_check_bin_info_prev != tx_pb_check_bin_info) {
			tx_pb_check_bin_info_prev = tx_pb_check_bin_info;
			write_byte2lcd_cpu(creat_info_id(ITID_TXPB_INFO, PII_BINARY_INFO));
			write_byte2lcd_cpu((u8)((tx_pb_check_bin_info_prev>>1) & 0x0f));
		}
		tx_pb_check_bin_info_using = 0;

		if (tx_pc_check_bin_info_prev != tx_pc_check_bin_info) {
			tx_pc_check_bin_info_prev = tx_pc_check_bin_info;
			write_byte2lcd_cpu(creat_info_id(ITID_TXPC_INFO, PII_BINARY_INFO));
			write_byte2lcd_cpu((u8)((tx_pc_check_bin_info_prev>>1) & 0x0f));
		}
		tx_pc_check_bin_info_using = 0;
	} else {
		++is_not_first;

		tx_pa_check_bin_info_prev = tx_pa_check_bin_info;
		write_byte2lcd_cpu(creat_info_id(ITID_TXPA_INFO, PII_BINARY_INFO));
		write_byte2lcd_cpu((u8)((tx_pa_check_bin_info_prev>>1) & 0x0f));
		tx_pa_check_bin_info_using = 0;

		tx_pb_check_bin_info_prev = tx_pb_check_bin_info;
		write_byte2lcd_cpu(creat_info_id(ITID_TXPB_INFO, PII_BINARY_INFO));
		write_byte2lcd_cpu((u8)((tx_pb_check_bin_info_prev>>1) & 0x0f));
		tx_pb_check_bin_info_using = 0;

		tx_pc_check_bin_info_prev = tx_pc_check_bin_info;
		write_byte2lcd_cpu(creat_info_id(ITID_TXPC_INFO, PII_BINARY_INFO));
		write_byte2lcd_cpu((u8)((tx_pc_check_bin_info_prev>>1) & 0x0f));
		tx_pc_check_bin_info_using = 0;
	}

	if (tx_power_off_prev ^ tx_power_off) {
		tx_poweroff_judge_delay = tim3_8ms_cnt;
		write_byte2lcd_cpu(creat_info_id(ITID_TXE_INFO, PII_BINARY_INFO));
		if (TX_POWEROFF_CONST == tx_power_off)
			write_byte2lcd_cpu((u8)(1<<3));
		else
			write_byte2lcd_cpu(0);

		tx_power_off_prev = tx_power_off;
		is_tx_poweroff_state_had_send = 1;
	}

	return;
}

/*
 * pa_too_h -- pb9
 * pa_too_l -- pb8
 * pb_too_h -- pb7
 * pb_too_l -- pb6
 * pc_too_h -- pb5
 * pc_too_l -- pb4
 *
 * 1--正常
 * 0--异常
 *
 * !!!!  任何一相异常(过高/过低), 硬件都会切换到电缆
 */
#define send_px_bin_info(px_h_pin, px_l_pin, px_msg_mid) do{\
	temp = 0;\
	if (is_px_too_horl(port1_data, px_h_pin))\
		temp |=  1 << 2;\
	if (is_px_too_horl(port1_data, px_l_pin))\
		temp |=  1 << 3;\
	write_byte2lcd_cpu(creat_info_id(px_msg_mid, PII_BINARY_INFO));\
	write_byte2lcd_cpu((u8)(temp&0x0f));\
}while(0)

#define is_px_too_horl(data, pin) is_bit_clr(data, pin)
#define PX_HORL_MASK ((0X3F)<<4)
#define SWITCH2PT_MASK GPIO_Pin_15
#define is_had_switch2pt(bitv) is_bit_set(bitv, switch2pt_indication_pin)

#define SWITCH2PT_BIT_MASK (1<<3)
#define FIBRE_DATA_BIT_MASK (1<<2)

static void check_if_rx_px_fault(void)
{
	static unsigned int fault_state_prev;
	static unsigned int re_bininfo_prev;
	static int is_not_first;
	static unsigned int delay1s;

	unsigned int port1_data, port2_data, had_change, temp;


	if (0 == delay1s) {
		port1_data = GPIOB->IDR & PX_HORL_MASK;
#if 1
		if (is_px_too_horl(port1_data, (pa_too_heigh_pin | pa_too_low_pin
						| pb_too_heigh_pin | pb_too_low_pin
						| pc_too_heigh_pin | pc_too_low_pin))) {
			force_output_switch2pt(UIUS_OUTPUT_TOO_LOW_OR_HEIGH);
			delay1s = tim3_8ms_cnt;
		}
#else
		if (is_px_too_horl(port1_data, pa_too_heigh_pin)) {
			enable_protect_electric_relay_pin(pa_too_low_pin);
			clr_port_pin(px_too_lh_gpio, pa_too_low_pin);
			delay1s = tim3_8ms_cnt;
		} else if (is_px_too_horl(port1_data, pa_too_low_pin)) {
			enable_protect_electric_relay_pin(pa_too_heigh_pin);
			clr_port_pin(px_too_lh_gpio, pa_too_heigh_pin);
			delay1s = tim3_8ms_cnt;
		} else if (is_px_too_horl(port1_data, pb_too_heigh_pin)) {
			enable_protect_electric_relay_pin(pb_too_low_pin);
			clr_port_pin(px_too_lh_gpio, pb_too_low_pin);
			delay1s = tim3_8ms_cnt;
		} else if (is_px_too_horl(port1_data, pb_too_low_pin)) {
			enable_protect_electric_relay_pin(pb_too_heigh_pin);
			clr_port_pin(px_too_lh_gpio, pb_too_heigh_pin);
			delay1s = tim3_8ms_cnt;
		} else if (is_px_too_horl(port1_data, pc_too_heigh_pin)) {
			enable_protect_electric_relay_pin(pc_too_low_pin);
			clr_port_pin(px_too_lh_gpio, pc_too_low_pin);
			delay1s = tim3_8ms_cnt;
		} else if (is_px_too_horl(port1_data, pc_too_low_pin)) {
			enable_protect_electric_relay_pin(pc_too_heigh_pin);
			clr_port_pin(px_too_lh_gpio, pc_too_heigh_pin);
			delay1s = tim3_8ms_cnt;
		} else {
			; /* nothing */
		}
#endif

	} else if (tim3_8ms_cnt-delay1s >= 125){
		cancel_force_output_switch2pt(UIUS_OUTPUT_TOO_LOW_OR_HEIGH);
		delay1s = 0;

		port1_data = GPIOB->IDR & PX_HORL_MASK;
	} else {
		port1_data = fault_state_prev;
	}

	port2_data = GPIOA->IDR & SWITCH2PT_MASK;

	if (is_not_first) {
		had_change = fault_state_prev ^ port1_data;
		if (0 != had_change) {
			if (had_change & (pa_too_heigh_pin | pa_too_low_pin))
				send_px_bin_info(pa_too_heigh_pin, pa_too_low_pin, ITID_RXPA_INFO);

			if (had_change & (pb_too_heigh_pin | pb_too_low_pin))
				send_px_bin_info(pb_too_heigh_pin, pb_too_low_pin, ITID_RXPB_INFO);

			if (had_change & (pc_too_heigh_pin | pc_too_low_pin))
				send_px_bin_info(pc_too_heigh_pin, pc_too_low_pin, ITID_RXPC_INFO);

			fault_state_prev = port1_data;
		}

		temp = 0;
		if (is_had_switch2pt(port2_data))
			set_bit(temp, SWITCH2PT_BIT_MASK);
		else
			clr_bit(temp, SWITCH2PT_BIT_MASK);

		if (fibre_optical_data_cnt > (FIBRE_DATA_BYTES_PER1MS*TIM3_PERIOD_MS - FIBRE_DATA_BYTES_GAP))
			set_bit(temp, FIBRE_DATA_BIT_MASK);
		else { /* 无光纤数据时, 认为发射端3相都缺相/过低 */
			clr_bit(temp, FIBRE_DATA_BIT_MASK);
			set_tx_px_toolow_lost_state(tx_pa_check_bin_info);
			set_tx_px_toolow_lost_state(tx_pb_check_bin_info);
			set_tx_px_toolow_lost_state(tx_pc_check_bin_info);
		}

		had_change = re_bininfo_prev ^ temp;
		if (0 != had_change) {
			re_bininfo_prev = temp;
			write_byte2lcd_cpu(creat_info_id(ITID_RXE_INFO, TRXII_BINARY_INFO));
			write_byte2lcd_cpu(re_bininfo_prev);
		}
	} else {
		++is_not_first;

		send_px_bin_info(pa_too_heigh_pin, pa_too_low_pin, ITID_RXPA_INFO);
		send_px_bin_info(pb_too_heigh_pin, pb_too_low_pin, ITID_RXPB_INFO);
		send_px_bin_info(pc_too_heigh_pin, pc_too_low_pin, ITID_RXPC_INFO);
		fault_state_prev = port1_data;

		if (is_had_switch2pt(port2_data))
			set_bit(re_bininfo_prev, SWITCH2PT_BIT_MASK);
		else
			clr_bit(re_bininfo_prev, SWITCH2PT_BIT_MASK);

		if (fibre_optical_data_cnt > (FIBRE_DATA_BYTES_PER1MS*TIM3_PERIOD_MS - FIBRE_DATA_BYTES_GAP))
			set_bit(re_bininfo_prev, FIBRE_DATA_BIT_MASK);
		else { /* 无光纤数据时, 认为发射端3相都缺相/过低 */
			clr_bit(temp, FIBRE_DATA_BIT_MASK);
			set_tx_px_toolow_lost_state(tx_pa_check_bin_info);
			set_tx_px_toolow_lost_state(tx_pb_check_bin_info);
			set_tx_px_toolow_lost_state(tx_pc_check_bin_info);
		}

		write_byte2lcd_cpu(creat_info_id(ITID_RXE_INFO, TRXII_BINARY_INFO));
		write_byte2lcd_cpu(re_bininfo_prev);

	}

	fibre_optical_data_cnt = 0;

	return;
}


/*******************************************************************************
* Function Name  : USART1_IRQHandler
* Description    : This function handles USART1 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI15_10_IRQHandler(void)
{
	turnoff_110v_power();

	++exti5_10_cnt;
#if USE_NEW_RX_OVERLOAD_ALGORITHM
	++rx_continue_overload_cnt;
	rx_continue_overload_delay = tim3_8ms_cnt;

	/* 似乎有抖动, 该isr不能返回太快, David */
	EXTI_ClearITPendingBit(EXTI_Line14);
	//EXTI->PR = EXTI_Line14;
#else
	overcurrent_cnt = tim3_8ms_cnt;
	overcurrent_door_judge = 1;
	delay_overcurrent_state++;
	if (RESET != EXTI_GetITStatus(EXTI_Line14)) {
		EXTI_ClearITPendingBit(EXTI_Line14);
	}
#endif

	return;
}


void USART2_IRQHandler(void)
{
	/* 内容有待整理，先调光数据接收 2013/3/1 USART2作为调试接口，有待整理此代码 */
	if (is_usartx_rxd_not_empty(USART2)) {
		/* 读数据时硬件会清楚USART_IT_RXNE位 */
		if (0 == USART2_GetRxBufferLeftLength())
			USART2_GetRxBufferData(); /* 接收缓冲区满, 丢弃一个老数据 */
		USART2_PutDatatoRxBuffer(get_usartx_data(USART2));
	}
	if (is_usartx_txd_empty(USART2)) {
		if(USART2_GetTxBufferCurrentSize()!= 0) {
			put_usartx_data(USART2, USART2_GetTxBufferData());
		} else {
			disable_usartx_send_int(USART2); //UASRT2_StopSend();
		}
	}
	if (is_usartx_overrun_err(USART2)) {
		USART_ClearFlag(USART2,USART_FLAG_ORE);	//清除ORE
		USART_ReceiveData(USART2);				//读DR
	}
}

void USART3_IRQHandler(void)
{
	if (is_usartx_rxd_not_empty(USART3)) {
		// 读数据时硬件会清楚USART_IT_RXNE位
		if (0 == USART3_GetRxBufferLeftLength())
			USART3_GetRxBufferData(); // 接收缓冲区满, 丢弃一个老数据 
		USART3_PutDatatoRxBuffer(get_usartx_data(USART3));	
	}
	if (is_usartx_overrun_err(USART3)) {
		USART_ClearFlag(USART3,USART_FLAG_ORE);	//清除ORE
		USART_ReceiveData(USART3);				//读DR
	}		
}

#if USE_PVD
void PVD_IRQHandler(void)
{
	/* 系统上电前500ms, 不认为是掉电; 若已处于系统掉电状态, 将不再处理 */
	if ((tim3_8ms_cnt < 62) || is_system_powerdown) {
		//EXTI_ClearITPendingBit(EXTI_Line16);
		EXTI->PR = EXTI_Line16;
		return;
	}

	#if 0
	/* 使输出继电器处于常态, 以减少耗电 */
	enable_protect_electric_relay_pin(pa_too_low_pin);
	clr_port_pin(px_too_lh_gpio, pa_too_low_pin);
	#endif

	turnoff_110v_power();
	off_running_led();

	is_system_powerdown = 1;
	sys_powerdown_delay4confirm = tim3_8ms_cnt;
	EXTI_ClearITPendingBit(EXTI_Line16);

	return;
}
#endif

static void memcpy_word(int *dst, const int *src, unsigned cnt)
{
	while (cnt-- > 0)
		*dst++ = *src++;

	return;
}
