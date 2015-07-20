/*
 ******************************************************************************
 * sink_info.h
 *
 * 2013-09-25,  creat by David, zhaoshaowei@yeejoin.com
 ******************************************************************************
 */

#ifndef SINK_INFO_H_
#define SINK_INFO_H_

#include <rtdef.h>
#include <sinkinfo_common.h>
#include <syscfgdata.h>
#include <sys_cfg_api.h>
#include <ade7880_api.h>


struct sink_em_relative_info_st {
	struct sinkinfo_emc_px_independence_st si_emc_ind_pa;
	struct sinkinfo_emc_px_independence_st si_emc_ind_pb;
	struct sinkinfo_emc_px_independence_st si_emc_ind_pc;
	struct harmonic_parameter_st si_harmonic_pa[EMC_HARMONIC_TIMES_MAX];
	struct harmonic_parameter_st si_harmonic_pb[EMC_HARMONIC_TIMES_MAX];
	struct harmonic_parameter_st si_harmonic_pc[EMC_HARMONIC_TIMES_MAX];
	struct sinkinfo_emc_dev_st emc_dev_info;
	struct sinkinfo_emc_copper_iron_st emc_copper_iron_info;
	struct px_sample_data_st px_vi_sample;
	

	struct sinkinfo_em_px_independence_st si_em_ind_pa;
	struct sinkinfo_em_px_independence_st si_em_ind_pb;
	struct sinkinfo_em_px_independence_st si_em_ind_pc;
	struct sinkinfo_em_vol_harmonic_st si_em_vol_harmonic_pa;
	struct sinkinfo_em_vol_harmonic_st si_em_vol_harmonic_pb;
	struct sinkinfo_em_vol_harmonic_st si_em_vol_harmonic_pc;
	struct sinkinfo_em_cur_harmonic_st si_em_cur_harmonic_pa;
	struct sinkinfo_em_cur_harmonic_st si_em_cur_harmonic_pb;
	struct sinkinfo_em_cur_harmonic_st si_em_cur_harmonic_pc;
	struct sinkinfo_em_momentary_freeze_st si_em_mom_freeze[ELECTRIC_METER_MOMENT_FREEZE_MAX];
	struct sinkinfo_em_timing_freeze_st si_em_time_freeze[ELECTRIC_METER_TIMING_FREEZE_MAX];
	struct sinkinfo_em_max_demand_st si_em_max_demand;
	struct sinkinfo_em_copper_iron_st si_em_copper_iron;
	struct sinkinfo_em_dev_st  em_dev_info;
	
#if EM_EVENT_SUPPERT
	struct sinkinfo_em_volloss_event_st si_em_volloss_event_pa[ELECTRIC_METER_EVENT_TIMES_MAX];
	struct sinkinfo_em_volloss_event_st si_em_volloss_event_pb[ELECTRIC_METER_EVENT_TIMES_MAX];
	struct sinkinfo_em_volloss_event_st si_em_volloss_event_pc[ELECTRIC_METER_EVENT_TIMES_MAX];
	struct sinkinfo_em_volloss_event_st si_em_volover_event_pa[ELECTRIC_METER_EVENT_TIMES_MAX];
	struct sinkinfo_em_volloss_event_st si_em_volover_event_pb[ELECTRIC_METER_EVENT_TIMES_MAX];
	struct sinkinfo_em_volloss_event_st si_em_volover_event_pc[ELECTRIC_METER_EVENT_TIMES_MAX];
	struct sinkinfo_em_volloss_event_st si_em_volunder_event_pa[ELECTRIC_METER_EVENT_TIMES_MAX];
	struct sinkinfo_em_volloss_event_st si_em_volunder_event_pb[ELECTRIC_METER_EVENT_TIMES_MAX];
	struct sinkinfo_em_volloss_event_st si_em_volunder_event_pc[ELECTRIC_METER_EVENT_TIMES_MAX];
	struct sinkinfo_em_volloss_event_st si_em_phasebreak_event_pa[ELECTRIC_METER_EVENT_TIMES_MAX];
	struct sinkinfo_em_volloss_event_st si_em_phasebreak_event_pb[ELECTRIC_METER_EVENT_TIMES_MAX];
	struct sinkinfo_em_volloss_event_st si_em_phasebreak_event_pc[ELECTRIC_METER_EVENT_TIMES_MAX];
	struct sinkinfo_em_curloss_event_st si_em_curloss_event_pa[ELECTRIC_METER_EVENT_TIMES_MAX];
	struct sinkinfo_em_curloss_event_st si_em_curloss_event_pb[ELECTRIC_METER_EVENT_TIMES_MAX];
	struct sinkinfo_em_curloss_event_st si_em_curloss_event_pc[ELECTRIC_METER_EVENT_TIMES_MAX];
	struct sinkinfo_em_curloss_event_st si_em_curover_event_pa[ELECTRIC_METER_EVENT_TIMES_MAX];
	struct sinkinfo_em_curloss_event_st si_em_curover_event_pb[ELECTRIC_METER_EVENT_TIMES_MAX];
	struct sinkinfo_em_curloss_event_st si_em_curover_event_pc[ELECTRIC_METER_EVENT_TIMES_MAX];
	struct sinkinfo_em_curloss_event_st si_em_curbreak_event_pa[ELECTRIC_METER_EVENT_TIMES_MAX];
	struct sinkinfo_em_curloss_event_st si_em_curbreak_event_pb[ELECTRIC_METER_EVENT_TIMES_MAX];
	struct sinkinfo_em_curloss_event_st si_em_curbreak_event_pc[ELECTRIC_METER_EVENT_TIMES_MAX];
	struct sinkinfo_em_meterclear_event_st si_em_meterclear_event[ELECTRIC_METER_EVENT_TIMES_MAX];
	struct sinkinfo_em_demandclear_event_st si_em_demandclear_event[ELECTRIC_METER_EVENT_TIMES_MAX];
	struct sinkinfo_em_program_event_st si_em_program_event[ELECTRIC_METER_EVENT_TIMES_MAX];
	struct sinkinfo_em_calibratetime_event_st si_em_calibratetime_event[ELECTRIC_METER_EVENT_TIMES_MAX];
	struct sinkinfo_em_rseqvol_event_st si_em_sreqvol_event[ELECTRIC_METER_EVENT_TIMES_MAX];
	struct sinkinfo_em_rseqvol_event_st si_em_sreqcur_event[ELECTRIC_METER_EVENT_TIMES_MAX];
#endif
	struct sinkinfo_ptc_st pt_info;
	struct sinkinfo_ctc_st ct_info;
	struct sinkinfo_ctc_st ct1_info;

	char electric_meter_sn[LEN_OF_EM_SN_MAX];

	s32_t pttmp, cttmp;	/* pt、CT侧温度 */
};


/* 以下是命令的相关结构 */
struct sink_info_cmd_st {
	u32_t ccmd;	/* dev-no/dev-id & cmd */
	u8_t  gdtno;	/* group no & dev type */
	u8_t  data_len;	/* 表示共用体有效数据个数 */
	u8_t  pad1;
	u8_t  pad2;
	enum sinkinfo_error_e err_code;
};

struct sink_info_msg {
	struct sink_info_cmd_st cmd;
	union {
		struct {
			u32_t val_pa;	/* abc三相数据分离时，a相的数据；abc三相数据合并或不分离的数据 */
			u32_t val_pb;	/* abc三相数据分离时，b相的数据 */
			u32_t val_pc;	/* abc三相数据分离时，c相的数据 */
		} val_st;

		struct sinkinfo_emc_px_independence_st	si_ind_px;
		struct sinkinfo_em_px_independence_st	si_em_ind_px;
		struct sinkinfo_em_dev_st		em_dev_info;
		struct sinkinfo_emc_dev_st		emc_dev_info;
		struct sinkinfo_emc_copper_iron_st emc_copper_iron_info;
		struct sinkinfo_em_copper_iron_st si_em_copper_iron;
		struct sinkinfo_em_momentary_freeze_st si_em_mom_freeze[ELECTRIC_METER_MOMENT_FREEZE_MAX];
		struct sinkinfo_em_timing_freeze_st si_em_time_freeze[ELECTRIC_METER_TIMING_FREEZE_MAX];
		struct sinkinfo_em_max_demand_st si_em_max_demand;
		
#if EM_EVENT_SUPPERT
		struct sinkinfo_em_volloss_event_st si_em_volloss_event_px[ELECTRIC_METER_EVENT_TIMES_MAX];
		struct sinkinfo_em_volloss_event_st si_em_volover_event_px[ELECTRIC_METER_EVENT_TIMES_MAX];
		struct sinkinfo_em_volloss_event_st si_em_volunder_event_px[ELECTRIC_METER_EVENT_TIMES_MAX];
		struct sinkinfo_em_volloss_event_st si_em_phasebreak_event_px[ELECTRIC_METER_EVENT_TIMES_MAX];
		struct sinkinfo_em_curloss_event_st si_em_curloss_event_px[ELECTRIC_METER_EVENT_TIMES_MAX];
		struct sinkinfo_em_curloss_event_st si_em_curover_event_px[ELECTRIC_METER_EVENT_TIMES_MAX];
		struct sinkinfo_em_curloss_event_st si_em_curbreak_event_px[ELECTRIC_METER_EVENT_TIMES_MAX];
		struct sinkinfo_em_meterclear_event_st si_em_meterclear_event[ELECTRIC_METER_EVENT_TIMES_MAX];
		struct sinkinfo_em_demandclear_event_st si_em_demandclear_event[ELECTRIC_METER_EVENT_TIMES_MAX];
		struct sinkinfo_em_program_event_st si_em_program_event[ELECTRIC_METER_EVENT_TIMES_MAX];
		struct sinkinfo_em_calibratetime_event_st si_em_calibratetime_event[ELECTRIC_METER_EVENT_TIMES_MAX];
		struct sinkinfo_em_rseqvol_event_st si_em_sreqvol_event[ELECTRIC_METER_EVENT_TIMES_MAX];
		struct sinkinfo_em_rseqvol_event_st si_em_sreqcur_event[ELECTRIC_METER_EVENT_TIMES_MAX];
#endif
		struct sinkinfo_ptc_st pt_info;
		struct sinkinfo_ctc_st ct_info;
		struct sinkinfo_ctc_st ct1_info;

		signed char sample_data[2][SINK_INFO_PX_SAMPLE_BUF_SIZE];	/* 采样数据以网络序存储 */
	} val_u;

	struct harmonic_parameter_st si_harmonic_px[EMC_HARMONIC_TIMES_MAX];
	struct sinkinfo_em_vol_harmonic_st si_em_vol_harmonic_px;
	struct sinkinfo_em_cur_harmonic_st si_em_cur_harmonic_px;

};


union get_7880_data_st {
	struct sinkinfo_emc_px_independence_st si_ind_px;
	struct harmonic_parameter_st si_harmonic_px[EMC_HARMONIC_TIMES_MAX];
	struct sinkinfo_emc_dev_st emc_dev_info;
	struct sinkinfo_emc_copper_iron_st emc_copper_iron_info;
	signed char px_vi_sample[2][SINK_INFO_PX_SAMPLE_BUF_SIZE];
};

union get_em_data_st {
	struct sinkinfo_em_px_independence_st si_em_ind_px;
	struct sinkinfo_em_harmonic_st harmonic_px;
	struct sinkinfo_em_momentary_freeze_st si_em_mom_freeze;
	struct sinkinfo_em_timing_freeze_st si_em_time_freeze;
	struct sinkinfo_em_max_demand_st si_em_max_demand;
	struct sinkinfo_em_copper_iron_st si_em_copper_iron;
	struct sinkinfo_em_dev_st  em_dev_info;
};



#define SINK_INFO_CMD_SIZE		(sizeof(struct sink_info_cmd_st))
/* abc三相分离的数据字节数 */
#define SINK_INFO_PABC_SIZE		(12)
#define SINK_INFO_PABC_ALL_SIZE		(sizeof(struct sinkinfo_emc_px_independence_st))
#define SINK_INFO_EM_PABC_ALL_SIZE	(sizeof(struct sinkinfo_em_px_independence_st))
#define SINK_INFO_EM_VCA_HAR_SIZE	(sizeof(struct sinkinfo_em_vol_harmonic_st))
#define SINK_INFO_EMC_VCA_HAR_SIZE	((sizeof(struct harmonic_parameter_st))*EMC_HARMONIC_TIMES_MAX)

#define SINK_INFO_PTC_DATA_ALL_SIZE	(sizeof(struct sinkinfo_ptc_st))
#define SINK_INFO_CTC_DATA_ALL_SIZE	(sizeof(struct sinkinfo_ctc_st))

#define SINK_INFO_EMC_COPPER_IRON_LOSSES_SIZE	(sizeof(struct sinkinfo_emc_copper_iron_st))
#define SINK_INFO_EM_COPPER_IRON_LOSSES_SIZE	(sizeof(struct sinkinfo_ctc_st))

#define SINK_INFO_MOMENT_FREEZE_SIZE	(sizeof(struct sinkinfo_em_momentary_freeze_st))
#define SINK_INFO_TIMING_FREEZE_SIZE	(sizeof(struct sinkinfo_em_timing_freeze_st))
#define SINK_INFO_MAX_DEMAND_SIZE	(sizeof(struct sinkinfo_em_max_demand_st))

/* abc三相不分的数据字节数 */
#define SINK_INFO_EM_DEV_SIZE		(4)
#define SINK_INFO_EM_ALL_SIZE		(sizeof(struct sinkinfo_em_dev_st))
#define SINK_INFO_DEV_ALL_SIZE		(sizeof(struct sinkinfo_emc_dev_st))

#if EM_EVENT_SUPPERT
#define SINK_INFO_EM_PX_VOLLOSS_EVENT_SIZE	sizeof(struct sinkinfo_em_volloss_event_st)
#define SINK_INFO_EM_PX_CURLOSS_EVENT_SIZE	sizeof(struct sinkinfo_em_curloss_event_st)
#define SINK_INFO_EM_METER_CLEAR_EVENT_SIZE	sizeof(struct sinkinfo_em_meterclear_event_st)
#define SINK_INFO_EM_DEMAND_CLEAR_EVENT_SIZE	sizeof(struct sinkinfo_em_demandclear_event_st)
#define SINK_INFO_EM_PROGRAM_EVENT_SIZE	sizeof(struct sinkinfo_em_program_event_st)
#define SINK_INFO_EM_CALIBRATE_TIME_EVENT_SIZE	sizeof(struct sinkinfo_em_calibratetime_event_st)
#define SINK_INFO_EM_REVERSE_SEQVOL_EVENT_SIZE	sizeof(struct sinkinfo_em_rseqvol_event_st)
#endif

#define SINKINFO_7880_GETDATA_FAIL_BIT	0X1
#define SINKINFO_EM_GETDATA_FAIL_BIT	0X2
extern unsigned int sinkinfo_print_switch;


#define SI_EVENT_GET_EM_START	(1<<0)
#define SI_EVENT_GET_EM_OVER	(1<<1)
#define SI_EVENT_GET_7880_START	(1<<2)
#define SI_EVENT_GET_7880_OVER	(1<<3)
#define SI_EVENT_GET_PT_CT_START	(1<<4)
#define SI_EVENT_GET_PT_CT_OVER		(1<<5)
#define SI_EVENT_EM_EEINACCURACY_START	(1<<6)
#define SI_EVENT_EM_EEINACCURACY_OVER	(1<<7)
extern  struct rt_event sinkinfo_event_set;



extern struct rt_semaphore sinkinfo_sem;
extern struct vectorgraph_st vg_st;

extern struct sink_em_relative_info_st *sinkinfo_all_em;
//extern struct register_em_info_st register_em_info;

//extern struct px_sample_data_st px_sample_data[ELECTRIC_METER_NUMBER_MAX];	/* 存放各个电表的电压、电流波形采用数据,以供IP网络获取 */
extern struct rt_semaphore px_sample_data_sem;

extern void sinkinfo_init(void);
extern void si_set_is_had_finish_wl_netcfg_flag(int is_had_finish);
extern void check_eenergy_timeout(int is_act_ee);
extern int si_get_cur_em_index(void);

#endif
