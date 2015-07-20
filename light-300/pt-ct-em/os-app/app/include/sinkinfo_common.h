/*
 ******************************************************************************
 * sinkinfo_common.h
 *
 * 2013-09-25,  creat by David, zhaoshaowei@yeejoin.com
 ******************************************************************************
 */

#ifndef SINKINFO_COMMON_H_
#define SINKINFO_COMMON_H_

#include <ammeter_common.h>
#include <syscfgdata-common.h>
#include <syscfgdata.h>
//#include <ade7880_api.h>

#ifndef NULL
#define NULL ((void *)0)
#endif

#define EM_EVENT_SUPPERT 0
#define INVLIDE_DATAL 0x7fffffff
#define INVLIDE_DATAS 0x7fff


/*
 * 电压			0.1	V
 * 电流			0.01	A
 * 频率			0.1	Hz
 * 相位			0.1	度
 * 有功功率		0.01	kW
 * 无功功率		0.01	kvar
 * 视在功率		0.01	kVA
 * 功率因数		0.01
 * 电压失真		0.1	％
 * 电流失真		0.1	％
 * PT负荷(视在功率)	0.1	VA
 * CT负荷(视在功率)	0.1	VA
 * PT二次压降		0.001	％
 *
 * 总有功电能		0.01	kWh
 * 总无功电能		0.01	kvarh
 *
 * 电压波形采样	一个周期40个点，每个点24bits
 * 电流波形采样
 *
 * NOTE:
 * 1、小数数据，存储传输的值是将其扩大10/100/1000倍后的值；为简化数据处理，除了pt压降是1000倍之外，其余数据统一为100倍。
 * 2、采样数据按4-bytes传输（以网络序）。
 * */

/*
 * 只有明确指出从电表中读取的数据才从电表中读取, 其他的从ade7880中读取
 * 设备数据 -- 从ade7880获取，此处的设备指的是电表侧的设备
 * 电表数据 -- 通过485总线使用645规约获取
 * pt/ct负荷指的是"视在功率"
 *
 * !!NOTE:命令之间要连续，rep之间要连续
 * */
enum sinkinfo_cmd_e {
	/* 以下命令，abc三相数据分离 */
	SIC_GET_VOLTAGE			= 0x01,	/* 0x01, 设备电压 */
	SIC_GET_CURRENT,			/* 0x02, 设备电流 */
	SIC_GET_FREQUENCY,			/* 0x03, 设备频率 */
	SIC_GET_PHASE,				/* 0x04, 设备相位 */
	SIC_GET_ACTIVE_POWER,			/* 0x05, 设备有功功率 */
	SIC_GET_REACTIVE_POWER,			/* 0x06, 设备无功功率 */
	SIC_GET_APPARENT_POWER,			/* 0x07, 设备视在功率 */
	SIC_GET_POWER_FACTOR,			/* 0x08, 设备功率因数 */
	SIC_GET_VOLTAGE_DISTORTION,		/* 0x09, 设备电压失真 */
	SIC_GET_CURRENT_DISTORTION,		/* 0x0a, 设备电流失真 */
	SIC_GET_EM_VOLTAGE,			/* 0x0b, 电表中读取的电压 */
	SIC_GET_EM_CURRENT,			/* 0x0c, 电表中读取的电流 */
	SIC_GET_EM_ACTIVE_POWER,		/* 0x0d, 电表中读取的有功功率 */
	SIC_GET_EM_REACTIVE_POWER,		/* 0x0e, 电表中读取的无功功率 */
	SIC_GET_EM_POWER_FACTOR,		/* 0x0f, 电表中读取的功率因数 */
	SIC_GET_EM_PROTOCAL_TYPE,		/* 0x0f, 电表中规约类型/*/
	SIC_GET_EM_WIRE_CONNECT_MODE,		/* 0x0f, 电表中接线方式 */
	SIC_GET_EM_METER_DATE_AND_TIME,		/* 0x0f, 抄表电池电压 */
	SIC_GET_EM_METER_TEMPERATURE,		/* 0x0f, 表内温度 */
	SIC_GET_EM_CLOCK_BATTERY_VOL,		/* 0x0f, 时钟电池电压 */
	SIC_GET_EM_METER_COLLECT_BATTERY_VOL,		/* 0x0f, 抄表电池电压 */

	SIC_GET_PT_LOAD,			/* 0x10, pt负荷 */
	SIC_GET_CT_LOAD,			/* 0x11, ct负荷 */
	SIC_GET_PT_TEMP,			/* 0x10, pt侧温度 */
	SIC_GET_CT_TEMP,			/* 0x11, ct侧温度 */
	SIC_GET_PT_VOLTAGE_DROP,		/* 0x12, pt二次压降, 用pt侧电压减去电表侧电压 */
	SIC_GET_PA_ALL_INDEPENDENCE_PARAM,	/* 0x13, 获取电表侧a相所有abc三相独立的参数, ade7880 */
	SIC_GET_PB_ALL_INDEPENDENCE_PARAM,	/* 0x14, 获取电表侧b相所有abc三相独立的参数, ade7880 */
	SIC_GET_PC_ALL_INDEPENDENCE_PARAM,	/* 0x15, 获取电表侧c相所有abc三相独立的参数, ade7880 */
	SIC_GET_COPPER_AND_IRON_LOSSES_PARAM,	/* 0x16, 获取电表侧铜损铁损数据, ade7880 */
	SIC_GET_EM_PA_ALL_INDEPENDENCE_PARAM,	/* 0x17, 获取电表中a相所有abc三相独立的参数, by 645 */
	SIC_GET_EM_PB_ALL_INDEPENDENCE_PARAM,	/* 0x18, 获取电表中b相所有abc三相独立的参数, by 645 */
	SIC_GET_EM_PC_ALL_INDEPENDENCE_PARAM,	/* 0x19, 获取电表中c相所有abc三相独立的参数, by 645 */
	SIC_GET_EM_MOMENT_FREEZE_PARAM,	/* 0x1a, 获取电表中瞬时冻结数据, by 645 */
	SIC_GET_EM_TIMMING_FREEZE_PARAM,	/* 0x1b, 获取电表中定时冻结数据, by 645 */
	SIC_GET_EM_MAX_DEMAND_PARAM,	/* 0x1c, 获取电表中最大需量数据, by 645 */
	SIC_GET_EM_COPPER_AND_IRON_LOSSES_PARAM,	/* 0x1d, 获取电表中铜损铁损数据, by 645 */
	SIC_GET_PTC_ALL_DATA,			/* 0x1e, 获取pt采集器的所有采集数据 */
	SIC_GET_CTC_ALL_DATA,			/* 0x1f, 获取ct采集器的所有采集数据 */

	/* 以下命令，abc三相数据合并或不分离 */
	SIC_GET_EM_ACT_ELECTRIC_ENERGY,		/* 0x1b, 电表有功电能 */
	SIC_GET_EM_REACT_ELECTRIC_ENERGY,	/* 0x1c, 电表无功电能 */
	SIC_GET_DEV_ACT_ELECTRIC_ENERGY,	/* 0x1d, 设备有功电能 */
	SIC_GET_DEV_REACT_ELECTRIC_ENERGY,	/* 0x1e, 设备无功电能 */

	SIC_GET_EM_PARAM,			/* 获取不分abc三相的电表所有参数, by 645 */
	SIC_GET_DEV_PARAM,			/* 获取不分abc三相的设备所有参数, ade7880 */

//	SIC_GET_EM_MONTH_ELECTRIC_ENERGY,	/* 电表本月电量 */
//	SIC_GET_DEV_MONTH_ELECTRIC_ENERGY,	/* 设备本月电量 */
	SIC_GET_ACT_INACCURACY,			/* 实时有功电能误差 */
	SIC_GET_REACT_INACCURACY,		/* 实时无功电能误差 */
//	SIC_GET_MONTH_INACCURACY,		/* 本月计量误差 */

	/* 以下命令返回数据量较大，大约(40*3+4)字节 */
	SIC_GET_PAV_SAMPLE_DATA,		/* a相电压波形采样值 */
	SIC_GET_PBV_SAMPLE_DATA,
	SIC_GET_PCV_SAMPLE_DATA,
	SIC_GET_PAI_SAMPLE_DATA,		/* a相电流波形采样值 */
	SIC_GET_PBI_SAMPLE_DATA,
	SIC_GET_PCI_SAMPLE_DATA,

	SIC_GET_PAVI_SAMPLE_DATA,		/* a相电压、电流波形采样值 */
	SIC_GET_PBVI_SAMPLE_DATA,
	SIC_GET_PCVI_SAMPLE_DATA,


	/* 以下命令，abc三相数据分离(响应) */
	SIC_GET_VOLTAGE_REP	= 0x100,	/* 设备电压 */
	SIC_GET_CURRENT_REP,			/* 设备电流 */
	SIC_GET_FREQUENCY_REP,			/* 设备频率 */
	SIC_GET_PHASE_REP,			/* 设备相位 */
	SIC_GET_ACTIVE_POWER_REP,		/* 设备有功功率 */
	SIC_GET_REACTIVE_POWER_REP,		/* 设备无功功率 */
	SIC_GET_APPARENT_POWER_REP,		/* 设备视在功率 */
	SIC_GET_POWER_FACTOR_REP,		/* 设备功率因数 */
	SIC_GET_VOLTAGE_DISTORTION_REP,		/* 设备电压失真 */
	SIC_GET_CURRENT_DISTORTION_REP,		/* 设备电流失真 */
	SIC_GET_EM_VOLTAGE_REP,			/* 电表中读取的电压 */
	SIC_GET_EM_CURRENT_REP,			/* 电表中读取的电流 */
	SIC_GET_EM_ACTIVE_POWER_REP,		/* 电表中读取的有功功率 */
	SIC_GET_EM_REACTIVE_POWER_REP,		/* 电表中读取的无功功率 */
	SIC_GET_EM_POWER_FACTOR_REP,		/* 电表中读取的功率因数 */
	SIC_GET_EM_PROTOCAL_TYPE_REP,		/* 0x0f, 电表中规约类型 */
	SIC_GET_EM_WIRE_CONNECT_MODE_REP,		/* 0x0f, 电表中接线方式 */
	SIC_GET_EM_METER_DATE_AND_TIME_REP,
	SIC_GET_EM_METER_TEMPERATURE_REP,		/* 0x0f, 表内温度 */
	SIC_GET_EM_CLOCK_BATTERY_VOL_REP,		/* 0x0f, 时钟电池电压 */
	SIC_GET_EM_METER_COLLECT_BATTERY_VOL_REP,		/* 0x0f, 抄表电池电压 */
	
	SIC_GET_PT_LOAD_REP,			/* pt负荷 */
	SIC_GET_CT_LOAD_REP,			/* ct负荷 */
	SIC_GET_PT_TEMP_REP,			/* pt侧温度 */
	SIC_GET_CT_TEMP_REP,			/* ct侧温度 */
	SIC_GET_PT_VOLTAGE_DROP_REP,		/* pt二次压降 */
	SIC_GET_PA_ALL_INDEPENDENCE_PARAM_REP,	/* 获取电表侧a相所有abc三项独立的参数, ade7880 */
	SIC_GET_PB_ALL_INDEPENDENCE_PARAM_REP,	/* 获取电表侧b相所有abc三项独立的参数, ade7880 */
	SIC_GET_PC_ALL_INDEPENDENCE_PARAM_REP,	/* 获取电表侧c相所有abc三项独立的参数, ade7880 */
	SIC_GET_COPPER_AND_IRON_LOSSES_PARAM_REP,	/* 0x16, 获取电表侧铜损铁损数据, ade7880 */
	SIC_GET_EM_PA_ALL_INDEPENDENCE_PARAM_REP,	/* 获取电表侧a相所有abc三项独立的参数, by 645 */
	SIC_GET_EM_PB_ALL_INDEPENDENCE_PARAM_REP,	/* 获取电表侧b相所有abc三项独立的参数, by 645 */
	SIC_GET_EM_PC_ALL_INDEPENDENCE_PARAM_REP,	/* 获取电表侧c相所有abc三项独立的参数, by 645 */
	SIC_GET_EM_MOMENT_FREEZE_PARAM_REP,	/* 0x1a, 获取电表中瞬时冻结数据, by 645 */
	SIC_GET_EM_TIMMING_FREEZE_PARAM_REP,	/* 0x1b, 获取电表中定时冻结数据, by 645 */
	SIC_GET_EM_MAX_DEMAND_PARAM_REP,	/* 0x1c, 获取电表中最大需量数据, by 645 */
	SIC_GET_EM_COPPER_AND_IRON_LOSSES_PARAM_REP,	/* 0x1d, 获取电表中铜损铁损数据, by 645 */
	SIC_GET_PTC_ALL_DATA_REP,			/* 获取pt采集器的所有采集数据 */
	SIC_GET_CTC_ALL_DATA_REP,			/* 获取ct采集器的所有采集数据 */

	/* 以下命令，abc三相数据合并或不分离(响应) */
	SIC_GET_EM_ACT_ELECTRIC_ENERGY_REP,	/* 电表有功电能 */
	SIC_GET_EM_REACT_ELECTRIC_ENERGY_REP,	/* 电表无功电能 */
	SIC_GET_DEV_ACT_ELECTRIC_ENERGY_REP,	/* 设备有功电能 */
	SIC_GET_DEV_REACT_ELECTRIC_ENERGY_REP,	/* 设备无功电能 */

	SIC_GET_EM_PARAM_REP,			/* 获取不分abc三相的电表所有参数 */
	SIC_GET_DEV_PARAM_REP,			/* 获取不分abc三相的设备所有参数 */

//	SIC_GET_EM_MONTH_ELECTRIC_ENERGY_REP,	/* 电表本月电量 */
//	SIC_GET_DEV_MONTH_ELECTRIC_ENERGY_REP,	/* 设备本月电量 */
//	SIC_GET_ACT_INACCURACY_REP,		/* 实时有功电能误差 */
//	SIC_GET_REACT_INACCURACY_REP,		/* 实时无功电能误差 */
//	SIC_GET_MONTH_INACCURACY_REP,		/* 本月计量误差 */

	/* 以下命令返回数据量较大，大约(40*3+4)字节 */
	SIC_GET_PAV_SAMPLE_DATA_REP,		/* a相电压波形采样值 */
	SIC_GET_PBV_SAMPLE_DATA_REP,
	SIC_GET_PCV_SAMPLE_DATA_REP,
	SIC_GET_PAI_SAMPLE_DATA_REP,		/* a相电流波形采样值 */
	SIC_GET_PBI_SAMPLE_DATA_REP,
	SIC_GET_PCI_SAMPLE_DATA_REP,

	SIC_GET_PAVI_SAMPLE_DATA_REP,		/* a相电压、电流波形采样值 */
	SIC_GET_PBVI_SAMPLE_DATA_REP,
	SIC_GET_PCVI_SAMPLE_DATA_REP,
};



enum sinkinfo_error_e {
	SIE_OK		= 0,	/*  */
	SIE_FAIL,
	SIE_INVALID_CMD,	/*  */
	SIE_NULL_PTR,		/*  */
	SIE_BUF2SMALL,
	SIE_CMD_NOT_SUPPORT,	/* 不支持的命令 */
	SIE_GET_7880DATA_FAIL,
	SIE_SECOND_GET_DATA_FAIL,	/* 连续2次获取数据失败 */
	SIE_BUFFER_LEN_ERROR,
};

enum sinkinfo_dev_type_e {
	SDT_ELECTRIC_METER = 1,	/* 从电表中读取数据 */
	SDT_MASTER_PT,	/*主节点*/
	SDT_PT,
	SDT_CT,
	SDT_CT1,
	SDT_DEV,		/* 从电表侧设备的ade7880中读取数据 */
};


#define SINKINFO_NOT_IMPLEMENT_UINT8_VALUE	RT_UINT8_MAX
#define SINKINFO_NOT_IMPLEMENT_UINT16_VALUE	RT_UINT16_MAX
#define SINKINFO_NOT_IMPLEMENT_UINT32_VALUE	RT_UINT32_MAX
#define SINKINFO_NOT_IMPLEMENT_INT8_VALUE	RT_INT8_MAX
#define SINKINFO_NOT_IMPLEMENT_INT16_VALUE	RT_INT16_MAX
#define SINKINFO_NOT_IMPLEMENT_INT32_VALUE	RT_INT32_MAX

#define SINKINFO_GET_DATA_FAIL_UINT8_VALUE	(RT_UINT8_MAX - 1)
#define SINKINFO_GET_DATA_FAIL_UINT16_VALUE	(RT_UINT16_MAX - 1)
#define SINKINFO_GET_DATA_FAIL_UINT32_VALUE	(RT_UINT32_MAX - 1)
#define SINKINFO_GET_DATA_FAIL_INT8_VALUE	(RT_INT8_MAX - 1)
#define SINKINFO_GET_DATA_FAIL_INT16_VALUE	(RT_INT16_MAX - 1)
#define SINKINFO_GET_DATA_FAIL_INT32_VALUE	(RT_INT32_MAX - 1)


/* 波形采样的数据字节数 */
#define SINK_INFO_PX_SAMPLE_DOT_NUM	(40)
#define SINK_INFO_PX_BYTE_PER_DOT	(3)
#define SINK_INFO_PX_SAMPLE_BUF_SIZE	(SINK_INFO_PX_SAMPLE_DOT_NUM * SINK_INFO_PX_BYTE_PER_DOT)

/* 最大电表个数 */
#ifndef ELECTRIC_METER_NUMBER_MAX
#define ELECTRIC_METER_NUMBER_MAX (1)  //hongbin E
#endif

#define ELECTRIC_METER_MOMENT_FREEZE_MAX (3) 
#define ELECTRIC_METER_TIMING_FREEZE_MAX (12) 
#if EM_EVENT_SUPPERT
#define ELECTRIC_METER_EVENT_TIMES_MAX (10) 
#endif
#define EMC_HARMONIC_TIMES_MAX (21) 

/* devtype's type is 'enum sinkinfo_dev_type_e' */
#define creat_grp_devtype_no(grpno, devtype)	((grpno)<<2 | (devtype & 0x3))
#define get_grp_no_from_gdtno(gdtno)		((gdtno)>>2)
#define get_devtype_from_gdtno(gdtno)		((gdtno) & 0x3)

/* 
 * cmd type is 'enum sinkinfo_cmd_e'
 * dev id of em, pt, ct 统一编号
 */
#define creat_sinkinfo_ccmd(dev_id, cmd)	((dev_id<<24) | (cmd))
#define get_devid_from_ccmd(ccmd)		((ccmd)>>24 & 0xff)
#define get_sinkinfo_cmd_from_ccmd(ccmd)	((ccmd) & 0xffffff)



/* abc三相数据分离, ade7880采集的 */
struct sinkinfo_emc_px_independence_st {
	s32_t vx;	/* 电压, s24 -> 32zp */
	s32_t ix;	/* 电流, s24 -> 32zp */
	u32_t hzx;	/* 频率, u16 */
	s32_t phx;	/* 相位, u16 */
	s32_t viphx;	/* 电压电流间相位, u16 */
	s32_t apx;	/* 有功功率, s24 -> 32se */
	s32_t rapx;	/* 无功功率, s24 -> 32 */
	s32_t appx;	/* 视在功率, s24 -> 32se */
	s32_t pfx;	/* 功率因数,7880手册中描述有冲突, 认为是s16 */
	s32_t vdx;	/* 电压失真, s24 -> 32 */
	s32_t cdx;	/* 电流失真, s24 -> 32 */
};

/* abc三相数据合并或不分离，设备数据, ade7880采集的 */
struct sinkinfo_emc_dev_st {
	s32_t dev_act_electric_energy;		/* 设备有功电能, S32 */
	s32_t dev_react_electric_energy;	/* 设备无功电能, S32 */
	u32_t dev_month_electric_energy;	/* 设备本月电量 */
};

/* 铜损、铁损数据，7880采集*/
struct sinkinfo_emc_copper_iron_st {
	u32_t copper_apxT;	/* 铜损有功总电能补偿量 */
	u32_t iron_apxT;	/* 铁损有功总电能补偿量 */
	u32_t copper_apxA;	/* A相铜损有功电能补偿量	*/
	u32_t iron_apxA;	/* A相铁损有功电能补偿量 */
	u32_t copper_apxB;	/* B 相铜损有功电能补偿量 */
	u32_t iron_apxB;	/* B 相铁损有功电能补偿量 */
	u32_t copper_apxC;	/* C 相铜损有功电能补偿量 */
	u32_t iron_apxC;	/* C 相铁损有功电能补偿量 */
};

/* abc三相数据分离, 645规约采集的 */
struct sinkinfo_em_px_independence_st {
	u32_t vx;	/* 电压 */
	u32_t ix;	/* 电流 */
	u32_t apx;	/* 有功功率 */
	u32_t rapx;	/* 无功功率 */
	s32_t appx;	/* 视在功率 */
	u32_t pfx;	/* 功率因数 */
	s32_t phx;	/* 相位, u16 目前不支持，网管显示为无效值 */
	s32_t viphx;	/* 电压电流间相位, u16 目前不支持，网管显示为无效值 */
	u32_t hzx;	/* 频率, u16 目前不支持，网管显示为无效值 */
	s32_t vdx;	/* 电压失真 */
	s32_t cdx;	/* 电流失真 */
	u32_t taex;		/* (当前)正向有功电能, 返回4bytes数据 */
	u32_t rtaex; 	/* (当前)反向有功电能, 返回4bytes数据 */
};

/* abc三相数据合并或不分离，电表数据,  645规约采集的 */
struct sinkinfo_em_dev_st {
//	u32_t em_act_electric_energy;		/* 电表有功电能, 以bcd码存储 */
//	u32_t em_react_electric_energy;		/* 电表无功电能,  */
//	u32_t em_month_electric_energy;		/* 电表本月电量,  */

	u32_t em_act_total_energy;		/* 正向有功总电能, 返回4bytes数据 */
	u32_t em_act_rate1_energy; 		/* 正向有功费率1电能, 返回4bytes数据 */
	u32_t em_act_rate2_energy;  	/* 正向有功费率2电能, 返回4bytes数据 */
	u32_t em_act_rate3_energy;  	/* 正向有功费率3电能, 返回4bytes数据 */
	u32_t em_act_rate4_energy;  	/* 正向有功费率4电能, 返回4bytes数据 */

	u32_t em_reverse_act_total_energy; 		/* 反向有功总电能, 返回4bytes数据 */
	u32_t em_reverse_act_rate1_energy; 		/* 反向有功费率1电能, 返回4bytes数据 */
	u32_t em_reverse_act_rate2_energy; 		/* 反向有功费率2电能, 返回4bytes数据 */
	u32_t em_reverse_act_rate3_energy; 		/* 反向有功费率3电能, 返回4bytes数据 */
	u32_t em_reverse_act_rate4_energy; 		/* 反向有功费率4电能, 返回4bytes数据 */

	u32_t em_react_total_energy;		/* 正向无功总电能, 返回4bytes数据 */
	u32_t em_react_rate1_energy; 		/* 正向无功费率1电能, 返回4bytes数据 */
	u32_t em_react_rate2_energy; 		/* 正向无功费率2电能, 返回4bytes数据 */
	u32_t em_react_rate3_energy; 		/* 正向无功费率3电能, 返回4bytes数据 */
	u32_t em_react_rate4_energy; 		/* 正向无功费率4电能, 返回4bytes数据 */

	u32_t em_reverse_react_total_energy; 	/* 反向无功总电能, 返回4bytes数据 */
	u32_t em_reverse_react_rate1_energy; 	/* 反向无功费率1电能, 返回4bytes数据 */
	u32_t em_reverse_react_rate2_energy; 	/* 反向无功费率2电能, 返回4bytes数据 */
	u32_t em_reverse_react_rate3_energy; 	/* 反向无功费率3电能, 返回4bytes数据 */
	u32_t em_reverse_react_rate4_energy; 	/* 反向无功费率4电能, 返回4bytes数据 */

	u32_t em_combin_act_total_energy;		/* 组合有功总电能, 返回4bytes数据 */
	u32_t em_combin_act_rate1_energy;		/* 组合有功费率1电能, 返回4bytes数据 */
	u32_t em_combin_act_rate2_energy;		/* 组合有功费率2电能, 返回4bytes数据 */
	u32_t em_combin_act_rate3_energy;		/* 组合有功费率3电能, 返回4bytes数据 */
	u32_t em_combin_act_rate4_energy;		/* 组合有功费率4电能, 返回4bytes数据 */
	
	u8_t em_date_time[7+1];			/* 表内时间 */
	s32_t em_v_clock;			/* 时钟电池电压 */
	s32_t em_v_read_em;			/* 抄表电池电压 */
	s16_t em_temper;			/* 表内温度 */
	s16_t pad;
	
//	u32_t em_protocol_type;	/* 电表规约类型 */
//	u32_t em_wire_connect_mode;	/* 电表接线方式：三相三线 or 三相四线 */

	s32_t em_act_ee_inaccuracy;		/* 实时有功电能误差, 通过ade7880测得 */
	s32_t em_react_ee_inaccuracy;		/* 实时无功电能误差, 通过ade7880测得 */
};

/* 瞬时冻结数据，电表规约采集 */
struct sinkinfo_em_momentary_freeze_st {
	u8_t freeze_time[5+3];	/*瞬时冻结时间YYMMDDhhmm */
	u32_t act_elec_energy;		/* 瞬时冻结正向有功总电能 */
	u32_t reverse_act_elec_energy;		/* 瞬时冻结反向有功总电能 */
	u32_t apxT;	/* 瞬时冻结总有功功率 */
	u32_t apxA;	/* 瞬时冻结A相总有功功率 */
	u32_t apxB;	/* 瞬时冻结B相总有功功率 */
	u32_t apxC;	/* 瞬时冻结C相总有功功率 */
	u32_t rapxT;	/* 瞬时冻结总无功功率 */
	u32_t rapxA;	/* 瞬时冻结A相总无功功率 */
	u32_t rapxB;	/* 瞬时冻结B相总无功功率 */
	u32_t rapxC;	/* 瞬时冻结C相总无功功率 */
	u32_t act_max_demand;	/* 瞬时冻结正向有功总最大需量 */
	u8_t act_max_demand_time[5+3];	/*瞬时冻结正向有功总最大需量发生时间 */
	u32_t reverse_act_max_demand;	/* 定时冻结反向有功总最大需量 */
	u8_t reverse_act_max_demand_time[5+3];	/*瞬时冻结反向有功总最大需量发生时间 */
};

/*电压谐波含量, by 645 */
struct sinkinfo_em_vol_harmonic_st {
	s32_t vol_harmonic[EMC_HARMONIC_TIMES_MAX];	/* 电压1次谐波含量 by 645 */
};

/*电流谐波含量, by 645 */
struct sinkinfo_em_cur_harmonic_st {
	s32_t cur_harmonic[EMC_HARMONIC_TIMES_MAX];	/* 电流21次谐波含量 by 645 */
};


/* 谐波可以共用一个结构体 */
struct sinkinfo_em_harmonic_st {
	s32_t harmonic_data[EMC_HARMONIC_TIMES_MAX];
};

/* 定时冻结数据，电表规约采集 */
struct sinkinfo_em_timing_freeze_st {
	u8_t freeze_time[5+3];	/*定时冻结时间YYMMDDhhmm */
	u32_t act_elec_energy;		/* 定时冻结正向有功总电能 */
	u32_t reverse_act_elec_energy;		/* 定时冻结反向有功总电能 */
	u32_t apxT;	/* 定时冻结总有功功率 */
	u32_t apxA;	/* 定时冻结A相总有功功率 */
	u32_t apxB;	/* 定时冻结B相总有功功率 */
	u32_t apxC;	/* 定时冻结C相总有功功率 */
	u32_t rapxT;	/* 定时冻结总无功功率 */
	u32_t rapxA;	/* 定时冻结A相总无功功率 */
	u32_t rapxB;	/* 定时冻结B相总无功功率 */
	u32_t rapxC;	/* 定时冻结C相总无功功率 */

	u32_t act_max_demand;	/* 定时冻结正向有功总最大需量 */
	u8_t act_max_demand_time[5+3];	/* 定时冻结正向有功总最大需量发生时间 */
	u32_t reverse_act_max_demand;	/* 定时冻结反向有功总最大需量 */
	u8_t reverse_act_max_demand_time[5+3];	/* 定时冻结反向有功总最大需量发生时间 */
};

/* 最大需量数据，电表规约采集 */
struct sinkinfo_em_max_demand_st {
	u32_t act_max_demand_total;	/* 正向有功总最大需量 */
	u8_t act_max_demand_time_total[5+3];	/* 正向有功总最大需量发生时间*/

	u32_t act_max_demand_rate1;	/* 正向有功费率 1 最大需量 */
	u8_t act_max_demand_time_rate1[5+3];	/* 正向有功费率 1 最大需量发生时间 */
	u32_t act_max_demand_rate2;	/* 正向有功费率 2 最大需量 */
	u8_t act_max_demand_time_rate2[5+3];	/* 正向有功费率 2 最大需量发生时间 */
	u32_t act_max_demand_rate3;	/* 正向有功费率 3 最大需量 */
	u8_t act_max_demand_time_rate3[5+3];	/* 正向有功费率 3 最大需量发生时间 */
	u32_t act_max_demand_rate4;	/* 正向有功费率 4 最大需量 */
	u8_t act_max_demand_time_rate4[5+3];	/* 正向有功费率 4 最大需量发生时间 */

	u32_t react_max_demand_total;	/* 正向无功总最大需量 */
	u8_t react_max_demand_time_total[5+3];	/* 正向无功总最大需量发生时间*/

	u32_t react_max_demand_rate1;	/* 正向无功费率 1 最大需量 */
	u8_t react_max_demand_time_rate1[5+3];	/* 正向无功费率 1 最大需量发生时间 */
	u32_t react_max_demand_rate2;	/* 正向无功费率 2 最大需量 */
	u8_t react_max_demand_time_rate2[5+3];	/* 正向无功费率 2 最大需量发生时间 */
	u32_t react_max_demand_rate3;	/* 正向无功费率 3 最大需量 */
	u8_t react_max_demand_time_rate3[5+3];	/* 正向无功费率 3 最大需量发生时间 */
	u32_t react_max_demand_rate4;	/* 正向无功费率 4 最大需量 */
	u8_t react_max_demand_time_rate4[5+3];	/* 正向无功费率 4 最大需量发生时间 */
};

/* 铜损、铁损数据，电表规约采集*/
struct sinkinfo_em_copper_iron_st {
	u32_t copper_apxT;	/* 铜损有功总电能补偿量 */
	u32_t iron_apxT;	/* 铁损有功总电能补偿量 */
	u32_t copper_apxA;	/* A相铜损有功电能补偿量	*/
	u32_t iron_apxA;	/* A相铁损有功电能补偿量 */
	u32_t copper_apxB;	/* B 相铜损有功电能补偿量 */
	u32_t iron_apxB;	/* B 相铁损有功电能补偿量 */
	u32_t copper_apxC;	/* C 相铜损有功电能补偿量 */
	u32_t iron_apxC;	/* C 相铁损有功电能补偿量 */
};

#if 1
/*电表失压、过压、欠压、断相事件数据，电表规约采集 */
struct sinkinfo_em_volloss_event_st {
	u8_t start_time[6+2];	/*发生时刻YYMMDDhhmmss */
	u8_t end_time[6+2];	/*结束时刻YYMMDDhhmmss */
	u32_t vA;	/* A相电压 */
	u32_t iA;	/* 电流 */
	u32_t apA;	/* 有功功率 */
	u32_t rapA;	/* 无功功率 */
	u32_t pfA;	/* 功率因数 */
	u32_t vB;	/* B相电压 */
	u32_t iB;	/* 电流 */
	u32_t apB;	/* 有功功率 */
	u32_t rapB;	/* 无功功率 */
	u32_t pfB;	/* 功率因数 */
	u32_t vC;	/* C相电压 */
	u32_t iC;	/* 电流 */
	u32_t apC;	/* 有功功率 */
	u32_t rapC;	/* 无功功率 */
	u32_t pfC;	/* 功率因数 */
	u32_t ahnA;	/* A相安时数 */
	u32_t ahnB;	/* B相安时数 */
	u32_t ahnC;	/* C相安时数 */
};
/*电表失流、过流、断流事件，电表规约采集 */
struct sinkinfo_em_curloss_event_st {
	u8_t start_time[6+2];	/*发生时刻YYMMDDhhmmss */
	u8_t end_time[6+2];	/*结束时刻YYMMDDhhmmss */
	u32_t vA;	/* A相电压 */
	u32_t iA;	/* 电流 */
	u32_t apA;	/* 有功功率 */
	u32_t rapA;	/* 无功功率 */
	u32_t pfA;	/* 功率因数 */
	u32_t vB;	/* B相电压 */
	u32_t iB;	/* 电流 */
	u32_t apB;	/* 有功功率 */
	u32_t rapB;	/* 无功功率 */
	u32_t pfB;	/* 功率因数 */
	u32_t vC;	/* C相电压 */
	u32_t iC;	/* 电流 */
	u32_t apC;	/* 有功功率 */
	u32_t rapC;	/* 无功功率 */
	u32_t pfC;	/* 功率因数 */
};
/*电表清零事件，电表规约采集 */
struct sinkinfo_em_meterclear_event_st {
	u8_t start_time[6+2];	/*发生时刻YYMMDDhhmmss */
	u8_t operator_code[4];	/*C0C1C2C3 */
	u32_t em_act_elec_energy;		/* 正向有功总电能 */
	u32_t em_reverse_act_elec_energy;		/* 反向有功总电能 */
	u32_t em_react_elec_energy_quadrant1;		/* 第一象限无功总电能 */
	u32_t em_react_elec_energy_quadrant2;		/* 第二象限无功总电能 */
	u32_t em_react_elec_energy_quadrant3;		/* 第三象限无功总电能 */
	u32_t em_react_elec_energy_quadrant4;		/* 第四象限无功总电能 */
	u32_t pA_act_elec_energy;		/* A相正向有功总电能 */
	u32_t pA_reverse_act_elec_energy;		/* 反向有功总电能 */
	u32_t pA_react_elec_energy_quadrant1;		/* 第一象限无功总电能 */
	u32_t pA_react_elec_energy_quadrant2;		/* 第二象限无功总电能 */
	u32_t pA_react_elec_energy_quadrant3;		/* 第三象限无功总电能 */
	u32_t pA_react_elec_energy_quadrant4;		/* 第四象限无功总电能 */
	u32_t pB_act_elec_energy;		/* B相正向有功总电能 */
	u32_t pB_reverse_act_elec_energy;		/* 反向有功总电能 */
	u32_t pB_react_elec_energy_quadrant1;		/* 第一象限无功总电能 */
	u32_t pB_react_elec_energy_quadrant2;		/* 第二象限无功总电能 */
	u32_t pB_react_elec_energy_quadrant3;		/* 第三象限无功总电能 */
	u32_t pB_react_elec_energy_quadrant4;		/* 第四象限无功总电能 */
	u32_t pC_act_elec_energy;		/* C相正向有功总电能  */
	u32_t pC_reverse_act_elec_energy;		/* 反向有功总电能 */
	u32_t pC_react_elec_energy_quadrant1;		/* 第一象限无功总电能 */
	u32_t pC_react_elec_energy_quadrant2;		/* 第二象限无功总电能 */
	u32_t pC_react_elec_energy_quadrant3;		/* 第三象限无功总电能 */
	u32_t pC_react_elec_energy_quadrant4;		/* 第四象限无功总电能 */
};
/*需量清零事件，电表规约采集 */
struct sinkinfo_em_demandclear_event_st {
	u8_t start_time[6+2];	/*发生时刻YYMMDDhhmmss */
	u8_t operator_code[4];	/*C0C1C2C3 */
};
/*编程代码事件，电表规约采集 */
struct sinkinfo_em_program_event_st {
	u8_t start_time[6+2];	/*发生时刻YYMMDDhhmmss */
	u8_t operator_code[4];	/*C0C1C2C3 */
	u8_t data_flag[4*10];	/*编程的前10个数据标示码 */
};
/*校时事件，电表规约采集 */
struct sinkinfo_em_calibratetime_event_st {	
	u8_t operator_code[4];	/*C0C1C2C3 */
	u8_t before_time[6+2];	/*校时前时间YYMMDDhhmmss */
	u8_t after_time[6+2];	/*校时后时间YYMMDDhhmmss */
};
/*电压电流逆相序事件，电表规约采集 */
struct sinkinfo_em_rseqvol_event_st {	
	u8_t start_time[6+2];	/*发生时刻YYMMDDhhmmss */
	u8_t end_time[6+2];	/*结束时刻YYMMDDhhmmss */
};
#endif

struct sinkinfo_pt_ct_st {
	u32_t vx;	/* 电压 */
	u32_t ix;	/* 电流 */
	u32_t apx;	/* 有功功率 */
	s32_t appx;	/* 视在功率 */
	u32_t pfx;	/* 功率因数 */
//	u32_t admx;	/* 导纳,该值由网管软件计算得出 */
};

/* pt采集器相关信息 */
struct sinkinfo_ptc_st {
	struct sinkinfo_pt_ct_st pt_pa;
	struct sinkinfo_pt_ct_st pt_pb;
	struct sinkinfo_pt_ct_st pt_pc;
};

/* ct采集器相关信息 */
struct sinkinfo_ctc_st {
	struct sinkinfo_pt_ct_st ct_pa;
	struct sinkinfo_pt_ct_st ct_pb;
	struct sinkinfo_pt_ct_st ct_pc;
};

/* 三相电压、电流采样值 */
struct px_sample_data_st {
	signed char pa_vi_sample[2][SINK_INFO_PX_SAMPLE_BUF_SIZE];
	signed char pb_vi_sample[2][SINK_INFO_PX_SAMPLE_BUF_SIZE];
	signed char pc_vi_sample[2][SINK_INFO_PX_SAMPLE_BUF_SIZE];
};


union sinkinfo_ptc_ctc_st {
	struct sinkinfo_ptc_st ptc_data;
	struct sinkinfo_ctc_st ctc_data;
	struct sinkinfo_ctc_st ctc1_data;
};

/*
 *用于缓冲通过无线收集到的数据
 **/
struct sinkinfo_wl_data_item_st {
	char pt_ct_sn[DEV_SN_BUF_STRING_WITH_NUL_LEN_MAX];
	union sinkinfo_ptc_ctc_st item;
	rt_tick_t time_stamp;
};


enum sink_data_dev_type_e {
	SDDT_EMC = 1,
	SDDT_PT,
	SDDT_CT,
	SDDT_CT1,
	SDDT_BUTT
};
enum ammeter_wire_connect_mode{
	CONNECT_UNKNOWN = 0x00,
	CONNECT_34,
	CONNECT_33,			
	CONNECT_12,	/*单相二线制 */

	CONNECT_BUTT, 
};

//#ifndef __AMMETER_PROTOCAL_E__
//#define __AMMETER_PROTOCAL_E__
//enum ammeter_protocal_e {
//	AP_PROTOCOL_UNKNOWN,
//	AP_PROTOCOL_645_1997,
//	AP_PROTOCOL_645_2007,
//	AP_PROTOCOL_EDMI,
//	AP_PROTOCOL_DLMS,
//	AP_PROTOCOL_WS,
//	AP_PROTOCOL_IEC1107,
//	AP_PROTOCOL_ACTARIS,
//	AP_PROTOCOL_SIMENS,
//	AP_PROTOCOL_NOTHING
//};
//#endif

#if 0
enum electric_meter_protocol_e {
	EMP_645_97,
	EMP_645_07,
	EMP_DLMS,

	EMP_BUTT
};
#endif

struct register_em_info_st {
	unsigned long registered_em_vector;
	unsigned char registered_em_map_vport[NUM_OF_COLLECT_EM_MAX];
	char registered_em_sn[NUM_OF_COLLECT_EM_MAX][DEV_SN_BUF_STRING_WITH_NUL_LEN_MAX];
	char registered_ptc_sn[NUM_OF_COLLECT_EM_MAX][DEV_SN_BUF_STRING_WITH_NUL_LEN_MAX];
	char registered_ctc_sn[NUM_OF_COLLECT_EM_MAX][DEV_SN_BUF_STRING_WITH_NUL_LEN_MAX];
	char registered_ctc1_sn[NUM_OF_COLLECT_EM_MAX][DEV_SN_BUF_STRING_WITH_NUL_LEN_MAX];

	enum ammeter_protocal_e em_proto[NUM_OF_COLLECT_EM_MAX];
	enum ammeter_wire_connect_mode em_wire_con_mode[NUM_OF_COLLECT_EM_MAX];
};

extern enum sink_data_dev_type_e si_get_dev_type(char *sn);

#define IS_CHECK_EENERGY_USE_THREAD 1

extern int update_em_reg_info(void);
extern struct register_em_info_st register_em_info;

extern int si_init_wl_data_proc(void);
extern void si_deinit_wl_data_proc(void);
extern int si_update_wl_collect_data(char *sn, union sinkinfo_ptc_ctc_st *data, rt_tick_t time_stamp);
extern int si_get_item_in_wl_data(char *sn, union sinkinfo_ptc_ctc_st *item, rt_tick_t *time_stamp);

extern int si_get_ptc_ctc_sn_from_remote_em_sn(char *em_sn, char *ptc_ctc_sn);
extern enum sink_data_dev_type_e si_get_dev_type(char *sn);
extern int sinkinfo_fill_ptc_buf_when_get_data_fail(struct sinkinfo_ptc_st *pt_info);
extern int sinkinfo_fill_ctc_buf_when_get_data_fail(struct sinkinfo_ctc_st *ct_info);
extern void si_fill_emc_ciloss_when_get_data_fail(struct sinkinfo_emc_copper_iron_st *p);

#endif
