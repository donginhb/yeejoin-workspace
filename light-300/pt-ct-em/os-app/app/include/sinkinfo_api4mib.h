/*
 * sinkinfo_api4mib.h
 *
 * 2013-09-24,  creat by David, zhaoshaowei@yeejoin.com
 */


#ifndef SINKINFO_API4MIB_H_
#define SINKINFO_API4MIB_H_

#include <sinkinfo_common.h>

#define SINKINFO_EMC_PX_DATA_SIZE	sizeof(struct sinkinfo_emc_px_independence_st)
#define SINKINFO_EMC_DEV_DATA_SIZE	sizeof(struct sinkinfo_emc_dev_st)
#define SINKINFO_EM_PX_DATA_SIZE	sizeof(struct sinkinfo_em_px_independence_st)
#define SINKINFO_EM_DEV_DATA_SIZE	sizeof(struct sinkinfo_em_dev_st)
#define SINKINFO_PTC_PX_DATA_SIZE		sizeof(struct sinkinfo_pt_ct_st)
#define SINKINFO_PTC_DATA_SIZE		sizeof(struct sinkinfo_ptc_st)
#define SINKINFO_CTC_DATA_SIZE		sizeof(struct sinkinfo_ctc_st)
#define SINKINFO_PX_XSAMPLE_DATA_SIZE	(SINK_INFO_PX_SAMPLE_BUF_SIZE)
#define SINKINFO_EM_COPPER_IRON_LOSSES_DATA_SIZE	sizeof(struct sinkinfo_em_copper_iron_st)
#define SINKINFO_EMC_COPPER_IRON_LOSSES_DATA_SIZE	sizeof(struct sinkinfo_emc_copper_iron_st)
#define SINKINFO_EM_MOMENT_FREEZE_DATA_SIZE	sizeof(struct sinkinfo_em_momentary_freeze_st)
#define SINKINFO_EM_TIMING_FREEZE_DATA_SIZE	sizeof(struct sinkinfo_em_timing_freeze_st)
#define SINKINFO_EM_MAX_DEMAND_DATA_SIZE	sizeof(struct sinkinfo_em_max_demand_st)
#define SINKINFO_EM_PX_HARMONIC_DATA_SIZE	sizeof(struct sinkinfo_em_vol_harmonic_st)

#if EM_EVENT_SUPPERT
#define SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE	sizeof(struct sinkinfo_em_volloss_event_st)
#define SINKINFO_EM_PX_CURLOSS_EVENT_SIZE	sizeof(struct sinkinfo_em_curloss_event_st)
#define SINKINFO_EM_METER_CLEAR_EVENT_SIZE	sizeof(struct sinkinfo_em_meterclear_event_st)
#define SINKINFO_EM_DEMAND_CLEAR_EVENT_SIZE	sizeof(struct sinkinfo_em_demandclear_event_st)
#define SINKINFO_EM_PROGRAM_EVENT_SIZE	sizeof(struct sinkinfo_em_program_event_st)
#define SINKINFO_EM_CALIBRATE_TIME_EVENT_SIZE	sizeof(struct sinkinfo_em_calibratetime_event_st)
#define SINKINFO_EM_REVERSE_SEQVOL_EVENT_SIZE	sizeof(struct sinkinfo_em_rseqvol_event_st)
#endif

enum si_mib_getdata_cmd_e {
	SI_MGC_GET_EMC_PA_INFO,		/* 获取电表采集器收集到的‘区分ABC三相’的A相数据 */
	SI_MGC_GET_EMC_PB_INFO,
	SI_MGC_GET_EMC_PC_INFO,
	SI_MGC_GET_EMC_PA_VOL_HARMONIC_INFO,		/* 获取电表中的‘区分ABC三相’的A相谐波数据 */
	SI_MGC_GET_EMC_PA_CUR_HARMONIC_INFO,		/* 获取电表中的‘区分ABC三相’的A相谐波数据 */
	SI_MGC_GET_EMC_PA_ACT_HARMONIC_INFO,		/* 获取电表中的‘区分ABC三相’的A相谐波数据 */
	SI_MGC_GET_EMC_PB_VOL_HARMONIC_INFO,
	SI_MGC_GET_EMC_PB_CUR_HARMONIC_INFO,
	SI_MGC_GET_EMC_PB_ACT_HARMONIC_INFO,
	SI_MGC_GET_EMC_PC_VOL_HARMONIC_INFO,
	SI_MGC_GET_EMC_PC_CUR_HARMONIC_INFO,
	SI_MGC_GET_EMC_PC_ACT_HARMONIC_INFO,
	SI_MGC_GET_EMC_DEV_INFO,	/* 获取电表采集器收集到的‘不区分ABC三相’的数据 */
	SI_MGC_GET_EMC_COPPER_IRON_LOSS_INFO,	/* 获取电表采集器收集到的‘铜损、铁损’的数据 */
	
	SI_MGC_GET_EM_PA_INFO,		/* 获取电表中的‘区分ABC三相’的A相数据 */
	SI_MGC_GET_EM_PB_INFO,
	SI_MGC_GET_EM_PC_INFO,
	SI_MGC_GET_EM_PA_VOL_HARMONIC_INFO,		/* 获取电表中的‘区分ABC三相’的A相谐波数据 */
	SI_MGC_GET_EM_PA_CUR_HARMONIC_INFO,		/* 获取电表中的‘区分ABC三相’的A相谐波数据 */
	SI_MGC_GET_EM_PB_VOL_HARMONIC_INFO,
	SI_MGC_GET_EM_PB_CUR_HARMONIC_INFO,
	SI_MGC_GET_EM_PC_VOL_HARMONIC_INFO,
	SI_MGC_GET_EM_PC_CUR_HARMONIC_INFO,
	
#if EM_EVENT_SUPPERT
	SI_MGC_GET_EM_PA_VOLLOSS_EVENT_INFO,		/* 获取电表中的‘区分ABC三相’的A相失压事件 */
	SI_MGC_GET_EM_PA_VOLOVER_EVENT_INFO,		/* 获取电表中的‘区分ABC三相’的A相过压事件 */
	SI_MGC_GET_EM_PA_VOLUNDER_EVENT_INFO,		/* 获取电表中的‘区分ABC三相’的A相欠压事件 */
	SI_MGC_GET_EM_PA_PHASEBREAK_EVENT_INFO,		/* 获取电表中的‘区分ABC三相’的A相断相事件 */
	SI_MGC_GET_EM_PA_CURLOSS_EVENT_INFO,		/* 获取电表中的‘区分ABC三相’的A相失流事件 */
	SI_MGC_GET_EM_PA_CUROVER_EVENT_INFO,		/* 获取电表中的‘区分ABC三相’的A相过流事件 */
	SI_MGC_GET_EM_PA_CURBREAK_EVENT_INFO,		/* 获取电表中的‘区分ABC三相’的A相断流事件 */
	SI_MGC_GET_EM_PB_VOLLOSS_EVENT_INFO,		/* 获取电表中的‘区分ABC三相’的B相失压事件 */
	SI_MGC_GET_EM_PB_VOLOVER_EVENT_INFO,		/* 获取电表中的‘区分ABC三相’的B相过压事件 */
	SI_MGC_GET_EM_PB_VOLUNDER_EVENT_INFO,		/* 获取电表中的‘区分ABC三相’的B相欠压事件 */
	SI_MGC_GET_EM_PB_PHASEBREAK_EVENT_INFO,		/* 获取电表中的‘区分ABC三相’的B相断相事件 */
	SI_MGC_GET_EM_PB_CURLOSS_EVENT_INFO,		/* 获取电表中的‘区分ABC三相’的B相失流事件 */
	SI_MGC_GET_EM_PB_CUROVER_EVENT_INFO,		/* 获取电表中的‘区分ABC三相’的B相过流事件 */
	SI_MGC_GET_EM_PB_CURBREAK_EVENT_INFO,		/* 获取电表中的‘区分ABC三相’的B相断流事件 */
	SI_MGC_GET_EM_PC_VOLLOSS_EVENT_INFO,		/* 获取电表中的‘区分ABC三相’的C相失压事件 */
	SI_MGC_GET_EM_PC_VOLOVER_EVENT_INFO,		/* 获取电表中的‘区分ABC三相’的C相过压事件 */
	SI_MGC_GET_EM_PC_VOLUNDER_EVENT_INFO,		/* 获取电表中的‘区分ABC三相’的C相欠压事件 */
	SI_MGC_GET_EM_PC_PHASEBREAK_EVENT_INFO,		/* 获取电表中的‘区分ABC三相’的C相断相事件 */
	SI_MGC_GET_EM_PC_CURLOSS_EVENT_INFO,		/* 获取电表中的‘区分ABC三相’的C相失流事件 */
	SI_MGC_GET_EM_PC_CUROVER_EVENT_INFO,		/* 获取电表中的‘区分ABC三相’的C相过流事件 */
	SI_MGC_GET_EM_PC_CURBREAK_EVENT_INFO,		/* 获取电表中的‘区分ABC三相’的C相断流事件 */	
	SI_MGC_GET_EM_METER_CLEAR_EVENT_INFO,		/* 获取电表中电表清零事件 */
	SI_MGC_GET_EM_DEMAND_CLEAR_EVENT_INFO,		/* 获取电表中需量清零事件 */
	SI_MGC_GET_EM_PROGRAM_EVENT_INFO,		/* 获取电表中编程事件 */
	SI_MGC_GET_EM_CALIBRATE_TIME_EVENT_INFO,		/* 获取电表中校时事件 */
	SI_MGC_GET_EM_REVERSE_REQ_VOL_EVENT_INFO,		/* 获取电表中电压逆向序事件 */
	SI_MGC_GET_EM_REVERSE_REQ_CUR_EVENT_INFO,		/* 获取电表中电流逆向序事件 */
#endif
	SI_MGC_GET_EM_DEV_INFO,		/* 获取电表中的‘不区分ABC三相’的数据 */
	SI_MGC_GET_EM_MOMENT_FREEZE_INFO,		/* 获取电表中的‘瞬时冻结’的数据 */
	SI_MGC_GET_EM_TIMING_FREEZE_INFO,		/* 获取电表中的‘定时冻结’的数据 */
	SI_MGC_GET_EM_MAX_DEMAND_INFO,		/* 获取电表中的‘最大需量’的数据 */
	SI_MGC_GET_EM_COPPER_IRON_LOSS_INFO,	/* 获取电表中的‘铜损、铁损’的数据 */
	
	SI_MGC_GET_PTC_PA_INFO,		/* 获取PT采集器收集到'区分ABC三相'的数据 */
	SI_MGC_GET_PTC_PB_INFO,		
	SI_MGC_GET_PTC_PC_INFO,		
	SI_MGC_GET_CTC_PA_INFO,		
	SI_MGC_GET_CTC_PB_INFO,		
	SI_MGC_GET_CTC_PC_INFO,		
	SI_MGC_GET_PTC_INFO,		/* 获取PT采集器收集到的数据 */
	SI_MGC_GET_CTC_INFO,		/* 获取CT采集器收集到的数据 */

	SI_MGC_GET_PA_VSAMPLE,		/* 获取A相电压波形数据 */
	SI_MGC_GET_PA_ISAMPLE,		/* 获取A相电流波形数据 */
	SI_MGC_GET_PB_VSAMPLE,		/* 获取B相电压波形数据 */
	SI_MGC_GET_PB_ISAMPLE,		/* 获取B相电流波形数据 */
	SI_MGC_GET_PC_VSAMPLE,		/* 获取C相电压波形数据 */
	SI_MGC_GET_PC_ISAMPLE,		/* 获取C相电流波形数据 */

	SI_MGC_GET_BUTT
};

extern enum sinkinfo_error_e get_sinkinfo_use_by_mib(int em_no, int ptct_no, enum si_mib_getdata_cmd_e cmd, void *data, unsigned data_len);


extern enum sinkinfo_error_e get_sinkinfo_abc_param(int em_no, int ptct_no, enum sinkinfo_cmd_e cmd, u32_t *pa, u32_t *pb, u32_t *pc);
extern enum sinkinfo_error_e get_sinkinfo_other_param(int em_no, enum sinkinfo_cmd_e cmd, u32_t *pinfo);
extern enum sinkinfo_error_e get_sinkinfo_sample_data(int em_no, enum sinkinfo_cmd_e cmd, void *data_buf, int len);
extern enum sinkinfo_error_e get_dev_sn_em_sn(enum sinkinfo_dev_type_e devt, char *str, int len, int em_no);
extern enum sinkinfo_error_e get_em_proto(int em_no, enum sinkinfo_cmd_e cmd, u32_t *pinfo);
extern enum sinkinfo_error_e set_em_proto(int em_no, enum sinkinfo_cmd_e cmd, u32_t *pinfo);

extern int set_rf_param(u32_t fre, u32_t tx_baud, u32_t tx_fd, u32_t rx_baud, u32_t rx_fd);
#endif
