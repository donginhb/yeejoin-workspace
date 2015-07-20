/*************STMicroelectronics (C) COPYRIGHT 2008*********/
/***********************************************************
*   函数库说明：硬件平台中断函数库                         *
*   版本：    Ver1.0                                       *
*   作者：    zzjjhh250/ZZJJHH250 @ (CopyRight)            *
*   创建日期：08/31/2010                                   *
* -------------------------------------------------------- *
*  [硬件说明]                                              *
*   处理器：    STM32F103VBT6                              *
*   系统时钟：  外部8M/PLL = 72M                           *
* -------------------------------------------------------- *
*  [支 持 库]                                              *
*   库名称：    PF_Config.h                                *
*   需要版本：  -----                                      *
*   支持库说明：硬件平台配置声明库                         *
* -------------------------------------------------------- *
*  [版本更新]                                              *
*   修改：                                                 *
*   修改日期：                                             *
*   版本：                                                 *
* -------------------------------------------------------- *
*  [版本历史]                                              *
* -------------------------------------------------------- *
*  [使用说明]：											   *
*				1.可调用EX_Support.c模块中各自对应的处理函 *
*			      数处理中断。                             *
*				2.所有中断响应函数，一般如果中断比较简易， *
*				  则直接在此编写，否则可以在EX_Support.c   *
***********************************************************/

/********************
* 头 文 件 配 置 区 *
********************/
/* Includes ------------------------------------------------------------------*/
#include "..\Source\LIB_Config.h"
#include "..\Source\PF_Config.H"

#include ".\fwlib\stm32f10x.h"
#include ".\fwlib\stm32f10x_exti.h"
#include "sys_comm.h"
#include "ads8329.h"
#include "board.h"
#include "tx_rx_comm.h"
#include "common.h"
#include "stm32f10x_it.h"

#if 1
/* USART UE Mask */
#define CR1_UE_Set                ((uint16_t)0x2000)  /* USART Enable Mask */
#define CR1_UE_Reset              ((uint16_t)0xDFFF)  /* USART Disable Mask */

/* DMA ENABLE mask */
#define CCR_ENABLE_Set          ((uint32_t)0x00000001)
#define CR2_EXTTRIG_SWSTART_Set ((uint32_t)0x00500000)
#define CCR_ENABLE_Reset        ((uint32_t)0xFFFFFFFE)

#endif

#define judge_px_and_set_check_info(px_vol_sample_avg, px_check_bin_info, px_max, px_min, px_none, set_px_lost_flag, clr_px_lost_flag)  do{\
	if (px_vol_sample_avg > px_max) {\
		px_check_bin_info = 1 << 3;\
	} else if (px_vol_sample_avg < px_min){\
		px_check_bin_info = 1 << 4;\
	} else {\
		px_check_bin_info = 0;\
	}\
	if (px_vol_sample_avg < px_none){\
		px_check_bin_info |= 1 << 2;\
		set_px_lost_flag();\
	} else {\
		clr_px_lost_flag();\
	}\
} while(0)

#define send_checkinfo()  do{\
	if (tx_check_info.next_ind < tx_check_info.end_ind) {\
		uart1_dma_buf[3] = tx_check_info.buf[tx_check_info.next_ind++];\
		adc_dambuf_len   = UART1_DMA_BUF_CNT + 1;\
	} else {\
		adc_dambuf_len          = UART1_DMA_BUF_CNT;\
		tx_check_info.need_send = 0;\
	}\
}while(0)

static volatile int adc_dambuf_len = UART1_DMA_BUF_CNT;
volatile unsigned int cpu_temper_send, box_temper_send;

volatile int pa_check_bin_info, pb_check_bin_info, pc_check_bin_info;

volatile unsigned int box_temper_sum;
volatile unsigned int cpu_temper_sum;

volatile unsigned int box_temper_max;
volatile unsigned int box_temper_min;
volatile unsigned int cpu_temper_max;
volatile unsigned int cpu_temper_min;

volatile unsigned int timer3_int_cnt;
volatile unsigned int start_usart2dma_cnt;

/*
 * bit 0: pa lost phase
 * bit 1: pb lost phase
 * bit 2: pc lost phase
 * bit 3: sent data
 */
volatile unsigned int sys_misc_flag;

#if 1
volatile s32 pa_sample_sum;
volatile s32 pb_sample_sum;
volatile s32 pc_sample_sum;
volatile unsigned int cnt_of_pc_sample_sum;

volatile s32 pa_learn_sample_sum;
volatile s32 pb_learn_sample_sum;
volatile s32 pc_learn_sample_sum;
volatile unsigned int cnt_of_learn_sample_sum;

volatile unsigned int pa_sample_avg_val;
volatile unsigned int pb_sample_avg_val;
volatile unsigned int pc_sample_avg_val;

volatile int read_px_sample_avg;

volatile s32 pa_sample4read;
volatile s32 pb_sample4read;
volatile s32 pc_sample4read;

#endif

volatile unsigned int tim3_8ms_cnt;
extern volatile s32  cpu_temper;

extern struct signal_cfg_param_tbl signal_cfg_param;
extern FLASH_Status write_sig_cfg_param_to_flash(volatile struct signal_cfg_param_tbl *cfg_param);
extern unsigned char tx_dev_sn_bits_5[DEV_SN_5BITS_BUF_LEN];

/*******************************************************************************
* Function Name  : DMAChannel7_IRQHandler
* Description    : This function handles DMA Stream 7 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
/*USART1 Tx 发送DMA方式test*/
void DMA1_Channel4_IRQHandler(void)
{
	DMA1_Channel4->CCR &= CCR_ENABLE_Reset; /* Disable the selected DMAy Channelx */
	DMA1->IFCR = DMA1_IT_GL4; /* Clear the selected DMA interrupt pending bits */
}


#define DEBUG_DATA_TRANS 0

struct tx_check_info_st {
	unsigned char buf[DEV_SN_5BITS_BUF_LEN];
	int next_ind;
	int end_ind;
	int need_send;
};

struct tx_check_info_st tx_check_info;



/*此处中断时按照定时点人工开启ADC test*/
void SysTick_Handler(void)
{
	static int   simple_adc;
	static unsigned int sys_tick_cnt;
	unsigned int px_adc_value;

	unsigned char *pch;

#if TEST_TX_POWEROFF
	extern volatile int send_powerdown_info_flag;
	if (send_powerdown_info_flag)
		return;
#endif
	//led_blink(led2_gpio, led2_pin);

#if DEBUG_DATA_TRANS
	send_data_to_pc(creat_log_ind(0x50), 4);
	send_data_to_pc(((UART1_DMA_BUF_CNT<<24) | simple_adc), 4);
#endif

#if 1
	/* 由于采集端不能接收恢复端的数据, 所以, 不管采集端的信息是否变更, 都需要定时发送给恢复端 */
	++sys_tick_cnt;
	switch (sys_tick_cnt) {
	case SYSTICK_NUM_PER100MS:
		tx_check_info.buf[0] = convert_id2firstbyte(TRMI_CPU_TEMPER) | ((cpu_temper_send>>11) & 0x1f); /* bit[15:11] */
		tx_check_info.buf[1] = ((cpu_temper_send>>6) & 0x1f); /* bit[10:6] */
		tx_check_info.buf[2] = ((cpu_temper_send>>1) & 0x1f); /* bit[5:1] */

		tx_check_info.end_ind  = 3;
		tx_check_info.next_ind = 0;
		tx_check_info.need_send = 1;
		break;

	case SYSTICK_NUM_PER200MS:
		tx_check_info.buf[0] = convert_id2firstbyte(TRMI_BOX_TEMPER) | ((box_temper_send>>11) & 0x1f); /* bit[15:11] */
		tx_check_info.buf[1] = ((box_temper_send>>6) & 0x1f); /* bit[10:6] */
		tx_check_info.buf[2] = ((box_temper_send>>1) & 0x1f); /* bit[5:1] */

		tx_check_info.end_ind  = 3;
		tx_check_info.next_ind = 0;
		tx_check_info.need_send = 1;
		break;

	case SYSTICK_NUM_PER300MS:
		tx_check_info.buf[0] = convert_id2firstbyte(TRMI_MISC_INFO) | TRMIS_PA_BININFO;
		tx_check_info.buf[1] = pa_check_bin_info;
		tx_check_info.buf[2] = convert_id2firstbyte(TRMI_MISC_INFO) | TRMIS_PB_BININFO;
		tx_check_info.buf[3] = pb_check_bin_info;
		tx_check_info.buf[4] = convert_id2firstbyte(TRMI_MISC_INFO) | TRMIS_PC_BININFO;
		tx_check_info.buf[5] = pc_check_bin_info;

		tx_check_info.end_ind  = 6;
		tx_check_info.next_ind = 0;
		tx_check_info.need_send = 1;
		break;

	case SYSTICK_NUM_PER400MS:
		tx_check_info.buf[0] = convert_id2firstbyte(TRMI_MISC_INFO) | TRMIS_TX_VERSION;
		tx_check_info.buf[1] = (TX_VERSION>>1) & 0x1f; /* m_v[5:1] */
		tx_check_info.buf[2] = ((TX_VERSION & 0x1) << 4) | ((TX_SUBVERSION>>3) & 0xf); /* m_v[0], s_v[6:3] */
		tx_check_info.buf[3] = ((TX_SUBVERSION & 0x7) << 2) | ((TX_REVISION>>5) & 0x3); /* s_v[2:0], r_v[6:5] */
		tx_check_info.buf[4] = (TX_REVISION & 0x1f); /* r_v[4:0] */

		tx_check_info.end_ind  = 5;
		tx_check_info.next_ind = 0;
		tx_check_info.need_send = 1;
		break;

	case SYSTICK_NUM_PER500MS:
		tx_check_info.buf[0] = convert_id2firstbyte(TRMI_MISC_INFO) | TRMIS_TX_DEV_SN;
		pch = &tx_check_info.buf[1];
		for (px_adc_value=0; px_adc_value<DEV_SN_5BITS_CNT; ++px_adc_value)
			pch[px_adc_value] = tx_dev_sn_bits_5[px_adc_value];
		tx_check_info.end_ind   = DEV_SN_5BITS_CNT + 1;
		tx_check_info.next_ind  = 0;
		tx_check_info.need_send = 1;
		break;

	case SYSTICK_NUM_PER1S:
		sys_tick_cnt = 0;
		break;

	default:
		break;
	}
#endif

	/*以下为判断此次中断要进行哪路信号的采样，需要一个标志计数中断次数，继而去余数进行多路的选通*/
	/*注意:systick的间隔中断时间长度还没有修改*/
#if 1
	/*准备进行A通道的采样*/
	if(0 == simple_adc) {
#if USE_12BITS_ADC_VAL
		/*实际采样值*/
		px_adc_value = ads8329_converter_channelx(PA_ADS8329_CH_PIN) >> 4;
		/*12bit数据传输*/
		uart1_dma_buf[1] =  px_adc_value & 0x3F;
		uart1_dma_buf[0] = (px_adc_value>>6 & 0x3F) | convert_id2firstbyte(TRMI_PHASE_A);
#elif USE_15BITS_ADC_VAL
		/*实际采样值*/
		px_adc_value = ads8329_converter_channelx(PA_ADS8329_CH_PIN);
		pa_sample_sum += sub_abs(px_adc_value, EXOTERICA_PX_CENTER_VAL);

		uart1_dma_buf[0] = get_height_adc_val(px_adc_value) | convert_id2firstbyte(TRMI_PHASE_A);
		uart1_dma_buf[1] = get_middle_adc_val(px_adc_value);
		uart1_dma_buf[2] = get_low_adc_val(px_adc_value);

		if (0 != tx_check_info.need_send)
			send_checkinfo();
#endif
		pa_sample4read = px_adc_value;
		simple_adc = 1;
	} else if(1 == simple_adc) {
#if USE_12BITS_ADC_VAL
		/*实际采样值*/
		px_adc_value = ads8329_converter_channelx(PB_ADS8329_CH_PIN) >> 4;
		/*12bit传输*/
		uart1_dma_buf[1] = px_adc_value & 0x3F;
		uart1_dma_buf[0] = (px_adc_value>>6 & 0x3F) | convert_id2firstbyte(TRMI_PHASE_B);
#elif USE_15BITS_ADC_VAL
		/*实际采样值*/
		px_adc_value = ads8329_converter_channelx(PB_ADS8329_CH_PIN);
		pb_sample_sum += sub_abs(px_adc_value, EXOTERICA_PX_CENTER_VAL);

		uart1_dma_buf[0] = get_height_adc_val(px_adc_value) | convert_id2firstbyte(TRMI_PHASE_B);
		uart1_dma_buf[1] = get_middle_adc_val(px_adc_value);
		uart1_dma_buf[2] = get_low_adc_val(px_adc_value);

		if (0 != tx_check_info.need_send)
			send_checkinfo();
#endif
		pb_sample4read = px_adc_value;
		simple_adc = 2;
	} else if(2 == simple_adc) {
#if USE_12BITS_ADC_VAL
		/*实际采样值*/
		px_adc_value = ads8329_converter_channelx(PC_ADS8329_CH_PIN) >> 4;
		/*12bit数据传输*/
		uart1_dma_buf[1] = px_adc_value & 0x3F;
		uart1_dma_buf[0] = (px_adc_value>>6 & 0x3F) | convert_id2firstbyte(TRMI_PHASE_C);
#elif USE_15BITS_ADC_VAL
		/*实际采样值*/
		px_adc_value = ads8329_converter_channelx(PC_ADS8329_CH_PIN);
		pc_sample_sum += sub_abs(px_adc_value, EXOTERICA_PX_CENTER_VAL);
		++cnt_of_pc_sample_sum;

		uart1_dma_buf[0] = get_height_adc_val(px_adc_value) | convert_id2firstbyte(TRMI_PHASE_C);
		uart1_dma_buf[1] = get_middle_adc_val(px_adc_value);
		uart1_dma_buf[2] = get_low_adc_val(px_adc_value);

		if (0 != tx_check_info.need_send)
			send_checkinfo();
#endif
		pc_sample4read = px_adc_value;
		simple_adc = 0;
	}

#if DEBUG_DATA_TRANS
	send_data_to_pc(creat_log_ind(0x51), 4);
	send_data_to_pc(simple_adc, 4);
	send_data_to_pc(px_adc_value, 4);
	send_data_to_pc(((uart1_dma_buf[0]<<16) | (uart1_dma_buf[1]<<8) | (uart1_dma_buf[2])), 4);
#endif

#if 1
	/* 2013/3/20 half_cycle_simple_cnt为0.5s的采样次数 */
	if (SAMPLE_NUMOF500MS == cnt_of_pc_sample_sum) {/*检测数据已经累加到0.5s个采样点数*/
		cnt_of_pc_sample_sum = 0;

		if (0 == read_px_sample_avg) {
			pa_sample_avg_val = pa_sample_sum / SAMPLE_NUMOF500MS;
			pb_sample_avg_val = pb_sample_sum / SAMPLE_NUMOF500MS;
			pc_sample_avg_val = pc_sample_sum / SAMPLE_NUMOF500MS;
		}

		if (is_had_enter_adjust_value_mode == SELF_LEARN_STDV_START) {
			pa_learn_sample_sum += pa_sample_avg_val;
			pb_learn_sample_sum += pb_sample_avg_val;
			pc_learn_sample_sum += pc_sample_avg_val;
			++cnt_of_learn_sample_sum;

			if(ADJUST_SAMPLE_8 == cnt_of_learn_sample_sum) {
				is_had_enter_adjust_value_mode = SELF_LEARN_STDV_END;
				pa_learn_sample_sum /= ADJUST_SAMPLE_8;
				pb_learn_sample_sum /= ADJUST_SAMPLE_8;
				pc_learn_sample_sum /= ADJUST_SAMPLE_8;

				signal_cfg_param.pa_learn_avg_val = pa_learn_sample_sum;
				signal_cfg_param.pb_learn_avg_val = pb_learn_sample_sum;
				signal_cfg_param.pc_learn_avg_val = pc_learn_sample_sum;
			}
		}

#if 0==DEBUG_CHECKINFO_TRAN
		judge_px_and_set_check_info(pa_sample_avg_val, pa_check_bin_info, pa_avg_val_max,
									pa_avg_val_min, pa_lost_avg_val, set_pa_lost_flag, clr_pa_lost_flag);

		judge_px_and_set_check_info(pb_sample_avg_val, pb_check_bin_info, pb_avg_val_max,
									pb_avg_val_min, pb_lost_avg_val, set_pb_lost_flag, clr_pb_lost_flag);

		judge_px_and_set_check_info(pc_sample_avg_val, pc_check_bin_info, pc_avg_val_max,
									pc_avg_val_min, pc_lost_avg_val, set_pc_lost_flag, clr_pc_lost_flag);
#endif
		pa_sample_sum = 0;
		pb_sample_sum = 0;
		pc_sample_sum = 0;
	}
#endif

	USART1->CR1 |= CR1_UE_Set;
	DMA1_Channel4->CNDTR = adc_dambuf_len; /* Write to DMAy Channelx CNDTR */
	DMA1_Channel4->CCR |= CCR_ENABLE_Set;  /*用前后加脉冲的方式得执行此语句用时465ns*/
#endif

	++start_usart2dma_cnt;
	return;
}

/*******************************************************************************
* Function Name  : USART1_IRQHandler
* Description    : This function handles USART1 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

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
	unsigned int cpu_temper_adc;

	//led_blink(led1_gpio, led1_pin);

	/* Clear the interrupt pending flag */
	TIM3->SR = (uint16_t)~TIM_FLAG_Update;
#if USE_STM32_IWDG
	/* Reloads IWDG counter with value defined in the reload register */
	/* #define KR_KEY_Reload    ((uint16_t)0xAAAA) */
	IWDG->KR = 0xAAAA;
#endif

	++tim3_8ms_cnt;

	if (delay_cnt++ < 100) {
		return;
	}

	DMA1_Channel1->CCR &= CCR_ENABLE_Reset; /* Disable the selected DMAy Channelx */

	cpu_temper_adc = ADC_GetInjectedConversionValue(ADC1 , ADC_InjectedChannel_1);
	if (0 == sample_cnt) {
		box_temper_sum = 0;
		cpu_temper_sum = 0;

		box_temper_max = adc_buf4temper[0];
		box_temper_min = adc_buf4temper[0];
		cpu_temper_max = cpu_temper_adc;
		cpu_temper_min = cpu_temper_adc;
	}

	box_temper_sum += adc_buf4temper[0];
	cpu_temper_sum += cpu_temper_adc;

	if (box_temper_max < adc_buf4temper[0])
		box_temper_max = adc_buf4temper[0];
	else if (box_temper_min > adc_buf4temper[0])
		box_temper_min = adc_buf4temper[0];

	if (cpu_temper_max < cpu_temper_adc)
		cpu_temper_max = cpu_temper_adc;
	else if (cpu_temper_min > cpu_temper_adc)
		cpu_temper_min = cpu_temper_adc;

	++sample_cnt;

	if (ADC_CNT_PER_SAMPLE == sample_cnt) {
		sample_cnt = 0;

		box_temper_sum -= box_temper_max + box_temper_min;
		cpu_temper_sum -= cpu_temper_max + cpu_temper_min;

		box_temper_send = box_temper_sum / (ADC_CNT_PER_SAMPLE - 2);
		cpu_temper_send = cpu_temper_sum / (ADC_CNT_PER_SAMPLE - 2);

		/* T = (V25 - Vsense)/(Avg_slope) + 25
		 * V25 = 1.43v
		 * Avg_slope = 4.3 mv/度
		 */
#if 0
		cpu_temper = (143 - (int)cpu_temper_send*330/4095) * 1000/430 + 25;
#else
		cpu_temper = (140 - (int)cpu_temper_send*330/4095) * 1000/460 + 25;
#endif

	}


	DMA1->IFCR = DMA1_IT_GL1; /* Clear the selected DMA interrupt pending bits */
	DMA1_Channel1->CNDTR = 1; /* Write to DMAy Channelx CNDTR */
	DMA1_Channel1->CCR |= DMA_IT_TC;
	DMA1_Channel1->CCR |= CCR_ENABLE_Set; /* Enable the selected DMAy Channelx */

	/* 人工打开ADC转换.*/
	ADC1->CR2 |= CR2_EXTTRIG_SWSTART_Set;

	if (start_usart2dma_cnt > (3*(TIM3_PERIOD_MS*SYSTICK_NUM_PER1S/1000)/5) ) {
		set_send_data_flag();
	} else {
		clr_send_data_flag();
	}
	start_usart2dma_cnt = 0;

	++timer3_int_cnt;

	return;
}

#if !USE_PVD_CHECK_POWEROFF

void EXTI1_IRQHandler(void)
{
	volatile int delay_var;
#if TEST_TX_POWEROFF
	int_led_blink();
#endif
	disable_dmax_y(DMA1_Channel4);
	DMA1->IFCR = DMA1_IT_GL4; /* Clear the selected DMA interrupt pending bits */

	uart1_dma_buf[0] = (convert_id2firstbyte(TRMI_MISC_INFO) | TRMIS_TX_POWEROFF);
	DMA1_Channel4->CNDTR = 1; /* Write to DMAy Channelx CNDTR */

	while (!is_usartx_txd_empty(USART1))
		;

	enable_dmax_y(DMA1_Channel4);
	delay_var = 0;
	delay_var = 1;
	delay_var = 2;

	while (!is_usartx_txd_empty(USART1))
		;

	EXTI_ClearITPendingBit(EXTI_Line1);

	return;
}

#else

void PVD_IRQHandler(void)
{
	volatile int delay_var;
#if TEST_TX_POWEROFF
	int_led_blink();
#endif

	/* 系统上电前500ms, 不认为是掉电 */
	if ((tim3_8ms_cnt < 62)) {
		//EXTI_ClearITPendingBit(EXTI_Line16);
		EXTI->PR = EXTI_Line16;
		return;
	}

	disable_dmax_y(DMA1_Channel4);
	DMA1->IFCR = DMA1_IT_GL4; /* Clear the selected DMA interrupt pending bits */

	uart1_dma_buf[0] = (convert_id2firstbyte(TRMI_MISC_INFO) | TRMIS_TX_POWEROFF);
	DMA1_Channel4->CNDTR = 1; /* Write to DMAy Channelx CNDTR */

	while (!is_usartx_txd_empty(USART1))
		;

	enable_dmax_y(DMA1_Channel4);
	delay_var = 0;
	delay_var = 1;
	delay_var = 2;

	while (!is_usartx_txd_empty(USART1))
		;

#if TEST_TX_POWEROFF
	int_led_blink();
#endif

	EXTI_ClearITPendingBit(EXTI_Line16);

	return;
}

#endif

