#ifndef STM32F10X_IT_H_
#define STM32F10X_IT_H_

#include "tx_rx_comm.h"

#define ADJ_PA_ZEROPOS_BIT 0X01
#define ADJ_PB_ZEROPOS_BIT 0X02
#define ADJ_PC_ZEROPOS_BIT 0X04
extern volatile unsigned int adj_px_zero_pos_vec;

#define is_doing_pa_zeropos_adj() is_bit_set(adj_px_zero_pos_vec, ADJ_PA_ZEROPOS_BIT)
#define is_doing_pb_zeropos_adj() is_bit_set(adj_px_zero_pos_vec, ADJ_PB_ZEROPOS_BIT)
#define is_doing_pc_zeropos_adj() is_bit_set(adj_px_zero_pos_vec, ADJ_PC_ZEROPOS_BIT)
#define is_doing_px_zeropos_adj() is_bit_set(adj_px_zero_pos_vec, ADJ_PA_ZEROPOS_BIT|ADJ_PB_ZEROPOS_BIT|ADJ_PC_ZEROPOS_BIT)

#define is_not_doing_pa_zeropos_adj() is_bit_clr(adj_px_zero_pos_vec, ADJ_PA_ZEROPOS_BIT)
#define is_not_doing_pb_zeropos_adj() is_bit_clr(adj_px_zero_pos_vec, ADJ_PB_ZEROPOS_BIT)
#define is_not_doing_pc_zeropos_adj() is_bit_clr(adj_px_zero_pos_vec, ADJ_PC_ZEROPOS_BIT)


#define start_pa_zeropos_adj() set_bit(adj_px_zero_pos_vec, ADJ_PA_ZEROPOS_BIT)
#define start_pb_zeropos_adj() set_bit(adj_px_zero_pos_vec, ADJ_PB_ZEROPOS_BIT)
#define start_pc_zeropos_adj() set_bit(adj_px_zero_pos_vec, ADJ_PC_ZEROPOS_BIT)

#define done_pa_zeropos_adj() clr_bit(adj_px_zero_pos_vec, ADJ_PA_ZEROPOS_BIT)
#define done_pb_zeropos_adj() clr_bit(adj_px_zero_pos_vec, ADJ_PB_ZEROPOS_BIT)
#define done_pc_zeropos_adj() clr_bit(adj_px_zero_pos_vec, ADJ_PC_ZEROPOS_BIT)

extern volatile unsigned int pa_zeropos_sum;
extern volatile unsigned int pb_zeropos_sum;
extern volatile unsigned int pc_zeropos_sum;

extern volatile unsigned int pa_zeropos_adc_cnt;
extern volatile unsigned int pb_zeropos_adc_cnt;
extern volatile unsigned int pc_zeropos_adc_cnt;



extern volatile unsigned int prev_send_soft_ver;
extern unsigned char prev_send_tx_dev_sn_bits_5[DEV_SN_5BITS_BUF_LEN];	/* 设备序列号 */
/*
 * bit0 -- 发送版本号
 * bit1 -- 发送dev-sn
 * */
#define SEND_SOFT_VERSION2LCD_CPU_BIT_MASK	0X01
#define SEND_DEV_SN2LCD_CPU_BIT_MASK		0X02
extern volatile unsigned int need_send_some_info2lcdcpu;


extern volatile s32  cpu_temper;
extern volatile unsigned int tim3_8ms_cnt;

extern unsigned char tx_dev_sn_bits_5[DEV_SN_5BITS_BUF_LEN];

extern unsigned char tx_dev_sn[DEV_SN_LEN_MAX+1];
extern volatile int tx_dev_sn_buf_state;

extern volatile int non_change_txe_info_send_cnt;

#endif

