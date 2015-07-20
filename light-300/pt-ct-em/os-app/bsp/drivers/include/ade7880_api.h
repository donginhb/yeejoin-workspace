/*
 * Description:
 * ade7880 api.h
 *
 */
#ifndef ADE7880_API_H_
#define ADE7880_API_H_

#include "stm32f10x.h"
#include <sinkinfo_common.h>

#define PHASE_A 0X01
#define PHASE_B 0X02
#define PHASE_C 0X03

#define CR1_CEN_Set                 ((uint16_t)0x0001)
#define CR1_CEN_Reset               ((uint16_t)0x03FE)

#define clr_ade7880_statusx(statux)  Write_32bitReg_ADE7880(statux, 0xffffffff)

enum ade7880_signal_sample_cmd_e {
	ASSC_SAMPLE_NONE,
	ASSC_SAMPLE_AVI,
	ASSC_SAMPLE_BVI,
	ASSC_SAMPLE_CVI
};

enum ade7880_adjust_reg_grp_id_e {
	AARI_READ_PXX_REG	= 0,
	AARI_PXV_GAIN		= 1,
	AARI_PXI_GAIN		= 2,
	AARI_PXP_GAIN		= 3,
	AARI_PX_WATTOS		= 4,
	AARI_PABC_WTHR		= 5,
	AARI_PX_VRMSOS		= 6,
	AARI_PX_IRMSOS		= 7,
	AARI_PX_CFXDEN		= 8,
	AARI_PX_XPHCAL		= 9,
};

enum eenergy_check_state_e {
	ECS_IDLE,
	ECS_TIMER_RUNNING,
	ECS_CHECK_OVER,
};



/* 电压电流矢量表参数 */
struct vectorgraph_st {
	s32_t vvap;
	s32_t vvbp;
	s32_t vvcp;
	s32_t viap;
	s32_t vibp;
	s32_t vicp;
};

struct harmonic_parameter_st{
	s32_t vrms;	/* 电压谐波 */
	s32_t irms;	/* 电流谐波 */
	s32_t watt;	/* 有功功率谐波 */
	s32_t var;	/* 无功功率谐波 */
	s32_t va;	/* 视在功率谐波 */
	s32_t pf;	/* 功率因数谐波 */
	s32_t vhd;	/* 相电压谐波对基波的谐波失真 */
	s32_t ihd;	/* 相电流谐波对基波的谐波失真 */
};

extern enum ade7880_signal_sample_cmd_e ade7880_sample_cmd;
extern struct harmonic_parameter_st harmonic;


extern int pa_ade7880_sample_data[2][SINK_INFO_PX_SAMPLE_DOT_NUM];
extern int pb_ade7880_sample_data[2][SINK_INFO_PX_SAMPLE_DOT_NUM];
extern int pc_ade7880_sample_data[2][SINK_INFO_PX_SAMPLE_DOT_NUM];

extern volatile enum eenergy_check_state_e check_eenergy_state;

extern volatile unsigned int check_eenergy_timer_clk_period_cnt;;
extern volatile unsigned long em_energy_timer_clks_cnt_start;
extern volatile unsigned long em_energy_timer_clks_cnt_end;

extern volatile unsigned long ade7880_energy_timer_clks_cnt_start;
extern volatile unsigned long ade7880_energy_timer_clks_cnt_end;

extern volatile int em_energy_int_cnt;
extern volatile int ade7880_energy_int_cnt;
extern volatile int connect33_data;

extern void delay_ade7880(vu32 ncount);




extern int px_virtual_mode_voltage(int px, s32 *data);
extern int px_virtual_mode_current(int px, s32 *data);
extern unsigned int px_frequency_mode_signal(int px);
extern unsigned int px_phase_mode_position(int px);
extern int px_active_mode_power(int px);
extern int px_apparent_mode_power(int px);
extern int px_factor_mode_power(int px);
extern int px_reactive_mode_power(int px);
extern int px_voltage_distortion(int px);
extern int px_current_distortion(int px);

#if 0
extern int px_active_energy(int px, s32_t *data);
extern int px_reactive_energy(int px, s32_t *data);
#endif

//extern void px_setout_handdebug(int id, int data, int chlx);
extern void px_setout_voltage(void);
extern void px_voltage_setout_autodebug(int pa,int pb,int pc,int chlx_cnt);
extern void px_current_setout_autodebug(int pa,int pb,int pc,int chlx_cnt);
extern void px_active_power_setout_autodebug(int pa,int pb,int pc,int chlx_cnt);
extern int printf_reg_info_debug(int chlx);
extern void printf_virtual_vi(int chlxv, int chlxi, int cnt_num);
extern void printf_phase_vi(int chlxv, int chlxi, int cnt_num);
extern int px_vi_signal_sample(int px, signed char *vs_buf, signed char *is_buf, int len);
extern int wait_dsp_complete(u32_t bit_mask);
extern void auto_set_powerup_workmode(int chlx);
extern void vi_vector_graph_sampl(void);
extern void px_phase_autodebug(int chlx_cnt);
extern void px_phase_autodebug_chlx(int chlx);  
extern void switch_current_channels(int chlx);
extern void scan_chlx_wire_connect_mode(int chlx);

extern void px_harmonic_mode_parameter(int px, int num);
extern void reset_chlx_reg(int chlx_num);
#if EM_ALL_TYPE_BASE 
extern void scan_wire_connect_mode(void);
#endif
extern void px_power_adjust_autodebug(void);
//extern void px_plus_adjust_autodebug(int offset);
extern void px_vircur_setout_autodebug(int v_setout,int i_setout,int p_setout,int chlx_cnt);
extern void px_vi_sample_reac_p_hsdc(void);
extern void px_gain_matching_autodebug(int chlx_cnt);   
extern void set_pabc_v_gain(int a, int b, int c);
extern void set_pabc_i_gain(int a, int b, int c);
extern void set_pabc_p_gain(int a, int b, int c);
extern void set_pabc_wattos(int a, int b, int c);
extern void set_pabc_vrmsos(int a, int b, int c);
extern void set_pabc_irmsos(int a, int b, int c);
extern void set_vkcpu(int a, int b, int c);
extern void set_ikcpu(int a, int b, int c);
extern void set_pkcpu(int a, int b, int c);
extern void set_connect(int a);
extern void set_cfxden(int a, int b, int c);
extern void set_pabc_xphcal(int a, int b, int c);
extern void set_p_wthr(u8_t a);
extern void set_p_varthr(u8_t a);

extern void get_pabc_v_gain(int *a, int *b, int *c);
extern void get_pabc_i_gain(int *a, int *b, int *c);
extern void get_pabc_p_gain(int *a, int *b, int *c);
extern void get_pabc_wattos_gain(int *a, int *b, int *c);
extern void get_pabc_vrmsos_gain(int *a, int *b, int *c);
extern void get_pabc_irmsos_gain(int *a, int *b, int *c);
extern void get_cfxden_gain(int *a, int *b, int *c);
extern void get_pabc_xphcal(int *a, int *b, int *c);
extern void get_p_wthr_gain(int *a);
extern void get_p_varthr_gain(int *a);  


extern void set_7880adj_reg(enum ade7880_adjust_reg_grp_id_e id);

#if 0!=IS_CHECK_EENERGY_USE_THREAD
extern void rt_check_eenergy_entry(void* parameter);
#else
extern void check_electric_energy(void);
#endif

#endif
