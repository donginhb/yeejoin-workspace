#ifndef STM32F10X_IT_H__
#define STM32F10X_IT_H__

#define DEBUG_COMPARE_VALUE 0
#define DEBUG_AVERAGE_VALUE 0

/*34000下的0.5s的采样次数*/
#define SAMPLE_NUMOF500MS (SYSTICK_NUM_PER500MS/3)

#define ADJUST_SAMPLE_8 (8)

/* 衰减后的均值 三相理论上均值一样 零位电压1.235v,对应14bit数字量8095，对应16bit数字量32380
 *
 * 32380
 */
#define EXOTERICA_AVERAGE_VALUE  33134

/* 0x816E == 33134 */
/* 32374.29 */
#define EXOTERICA_PX_CENTER_VAL  (0x816EUL)

/* 由信号源输出信号的幅值V经过0.0118577的衰减送入AD，成为输入幅值为v*0.0118577的正弦输入信号
 * 其均值为v1=(v*2/3.14)*0.0118577，二进制为v1*65535/2.5
 *
 * 信号源输入为峰值为88v
 */

#if 0
extern volatile unsigned int pa_crest_adc_val;
extern volatile unsigned int pa_center_adc_val;
extern volatile unsigned int pa_trough_adc_val;

extern volatile unsigned int pb_crest_adc_val;
extern volatile unsigned int pb_center_adc_val;
extern volatile unsigned int pb_trough_adc_val;

extern volatile unsigned int pc_crest_adc_val;
extern volatile unsigned int pc_center_adc_val;
extern volatile unsigned int pc_trough_adc_val;
#endif

extern volatile unsigned int pa_sample_avg_val;
extern volatile unsigned int pb_sample_avg_val;
extern volatile unsigned int pc_sample_avg_val;
extern volatile int read_px_sample_avg;

extern volatile s32 pa_learn_sample_sum;
extern volatile s32 pb_learn_sample_sum;
extern volatile s32 pc_learn_sample_sum;
extern volatile unsigned int cnt_of_learn_sample_sum;

#if 1
extern volatile s32 pa_sample_sum;
extern volatile s32 pb_sample_sum;
extern volatile s32 pc_sample_sum;

extern volatile s32 pa_sample4read;
extern volatile s32 pb_sample4read;
extern volatile s32 pc_sample4read;

#endif

#endif
