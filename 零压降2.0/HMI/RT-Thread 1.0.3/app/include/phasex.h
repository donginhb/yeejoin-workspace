#ifndef PHASEX_H__
#define PHASEX_H__

#include <form.h>

/*
 * 使用32-bits位向量表示发射端、接收端的相关状态
 *
 * 接收端使用低16bits, 发射端使用高16bits
 */
#if 0 /* 接收端输出是否正常, 用是否超限就可以表示 */
/* px输出是否正常, 1-正常, 0-异常 */
#define RE_PA_OUTPUT_BIT (0X0001UL)
#define RE_PB_OUTPUT_BIT (0X0002UL)
#define RE_PC_OUTPUT_BIT (0X0004UL)
#define RE_PX_OUTPUT_BIT (0X0007UL)
#endif
/* px是否掉电(缺相), 1-掉电, 0-未掉电 */
#define SE_PA_POWER_FAIL_BIT (0X010000UL)
#define SE_PB_POWER_FAIL_BIT (0X020000UL)
#define SE_PC_POWER_FAIL_BIT (0X040000UL)
#define SE_PX_POWER_FAIL_BIT (0X070000UL)

extern volatile int is_px_bin_info_modifing;

#define set_dev_px_state(vector, bit_mask) do {\
	is_px_bin_info_modifing = 1;\
	set_bit(vector, bit_mask);\
	is_px_bin_info_modifing = 0;\
} while(0)
#define clr_dev_px_state(vector, bit_mask) clr_bit(vector, bit_mask)

#if 0
#define is_sre_px_fault()	((dev_px_state_vec&SE_PX_POWER_FAIL_BIT) || ((dev_px_state_vec&RE_PX_OUTPUT_BIT) != RE_PX_OUTPUT_BIT))
#else
#define is_sre_px_fault()	((dev_px_state_vec&SE_PX_POWER_FAIL_BIT))
#endif
/*
 * 是否超限,可以取三个值[过高, 正常, 过低]
 * voltage too-heigh, normal, too-low
 */
#define VOLTAGE_TOO_HEIGH 	0X04
#define VOLTAGE_NORMAL    	0X02
#define VOLTAGE_TOO_LOW   	0X01

#define VOLTAGE_STATE_MASK	0X07
#define RE_PA_VOL_STATE_OFFSET 0
#define RE_PB_VOL_STATE_OFFSET 4
#define RE_PC_VOL_STATE_OFFSET 8
#define SE_PA_VOL_STATE_OFFSET 12
#define SE_PB_VOL_STATE_OFFSET 16
#define SE_PC_VOL_STATE_OFFSET 20

#define SRE_PX_VOL_NORMAL_BIT   (VOLTAGE_NORMAL |\
				(VOLTAGE_NORMAL<<RE_PB_VOL_STATE_OFFSET) |\
				(VOLTAGE_NORMAL<<RE_PC_VOL_STATE_OFFSET) |\
				(VOLTAGE_NORMAL<<SE_PA_VOL_STATE_OFFSET) |\
				(VOLTAGE_NORMAL<<SE_PB_VOL_STATE_OFFSET) |\
				(VOLTAGE_NORMAL<<SE_PC_VOL_STATE_OFFSET))

#define set_px_vol_state(vector, offset, val) do {\
	is_px_bin_info_modifing = 1;\
	vector = (vector&(~(VOLTAGE_STATE_MASK<<offset))) | (val<<offset);\
	is_px_bin_info_modifing = 0;\
} while (0)
#define get_px_vol_state(vector, offset, val) ((vector>>offset) & val)

#define is_px_vol_state_toolow(vector, offset) get_px_vol_state(vector, offset, VOLTAGE_TOO_LOW)
#define is_px_vol_state_normal(vector, offset) get_px_vol_state(vector, offset, VOLTAGE_NORMAL)
#define is_px_vol_state_tooheigh(vector, offset) get_px_vol_state(vector, offset, VOLTAGE_TOO_HEIGH)

#define is_sre_px_vol_not_normal() ((dev_px_vol_state_vec&SRE_PX_VOL_NORMAL_BIT) != SRE_PX_VOL_NORMAL_BIT)

/*
 * 定义相关数组索引
 */
enum avg_val_index_e {
	AVI_RE_PA,
	AVI_RE_PB,
	AVI_RE_PC,
	AVI_RE_END
};

/* constant temperature box */
enum sys_temperature_inde_e {
	STI_RE_CPU_T,
	STI_RE_CTB_T,
	STI_SE_CPU_T,
	STI_SE_CTB_T,
	STI_XE_T_END
};

extern volatile unsigned long dev_px_state_vec;
extern volatile unsigned long dev_px_vol_state_vec;

extern short int avg_val[AVI_RE_END];
extern short int sys_temper[STI_XE_T_END];

extern rtgui_form_t *se_form;
extern rtgui_form_t *re_form;
extern rtgui_form_t *sys_form;
extern rtgui_form_t *other_form;

extern const char * const print_info_str[7];

enum print_info_str_id_e {
	PIS_ID_GUOGAO,
	PIS_ID_ZHENGCHANG,
	PIS_ID_GUODI,
	PIS_ID_YICHANG,
	PIS_ID_SHI,
	PIS_ID_FOU,
};

#endif
