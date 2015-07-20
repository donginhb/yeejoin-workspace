#ifndef STM32F10X_IT_H__
#define STM32F10X_IT_H__

#define DEBUG_COMPARE_VALUE 0
#define DEBUG_AVERAGE_VALUE 0

/*34000�µ�0.5s�Ĳ�������*/
#define SAMPLE_NUMOF500MS (SYSTICK_NUM_PER500MS/3)

#define ADJUST_SAMPLE_8 (8)

/* ˥����ľ�ֵ ���������Ͼ�ֵһ�� ��λ��ѹ1.235v,��Ӧ14bit������8095����Ӧ16bit������32380
 *
 * 32380
 */
#define EXOTERICA_AVERAGE_VALUE  33134

/* 0x816E == 33134 */
/* 32374.29 */
#define EXOTERICA_PX_CENTER_VAL  (0x816EUL)

/* ���ź�Դ����źŵķ�ֵV����0.0118577��˥������AD����Ϊ�����ֵΪv*0.0118577�����������ź�
 * ���ֵΪv1=(v*2/3.14)*0.0118577��������Ϊv1*65535/2.5
 *
 * �ź�Դ����Ϊ��ֵΪ88v
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
