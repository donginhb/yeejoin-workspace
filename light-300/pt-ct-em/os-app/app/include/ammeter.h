/**********************************************************************************
* Filename   	: ammeter.h
* Description 	: define structs and varibles of read ammeter function
* Begin time  	: 2013-6-1
* Finish time 	:
* Engineer		: zhanghonglei
*************************************************************************************/
#ifndef __AMMETER_H__
#define __AMMETER_H__

#include <rtdef.h>
#include <misc_lib.h>
#include <ammeter_645_97.h>
#include <ammeter_645_07.h>

enum data_type {
	DATA_TYPE_UNKNOWN,
	DATA_TYPE_SINGLE_FLOAT,
	DATA_TYPE_DOUBLE_FLOAT,
	DATA_TYPE_STRING,
	DATA_TYPE_INTEGER,
	DATA_TYPE_DATE,
	DATA_TYPE_TIME,
	DATA_TYPE_UNSIGNED_INTEGER,
};

enum ammeter_cmd_e {
	AC_COMBINATION_ACTIVE_TOTAL_POWER,		/* 组合有功总电能, 返回4bytes数据 */
	AC_COMBINATION_ACTIVE_RATE1_POWER,		/* 组合有功费率1电能, 返回4bytes数据 */
	AC_COMBINATION_ACTIVE_RATE2_POWER,		/* 组合有功费率2电能, 返回4bytes数据 */
	AC_COMBINATION_ACTIVE_RATE3_POWER,		/* 组合有功费率3电能, 返回4bytes数据 */
	AC_COMBINATION_ACTIVE_RATE4_POWER,		/* 组合有功费率4电能, 返回4bytes数据 */

	AC_POSITIVE_ACTIVE_POWER,			/* 正向有功总电能, 返回4bytes数据 */
	AC_POSITIVE_ACTIVE_RATE1_POWER,  	/* 正向有功费率1电能, 返回4bytes数据 */
	AC_POSITIVE_ACTIVE_RATE2_POWER,  	/* 正向有功费率2电能, 返回4bytes数据 */
	AC_POSITIVE_ACTIVE_RATE3_POWER,  	/* 正向有功费率3电能, 返回4bytes数据 */
	AC_POSITIVE_ACTIVE_RATE4_POWER,  	/* 正向有功费率4电能, 返回4bytes数据 */

	AC_REPOSITIVE_ACTIVE_POWER, 		/* 反向有功总电能, 返回4bytes数据 */
	AC_REPOSITIVE_ACTIVE_RATE1_POWER, 	/* 反向有功费率1电能, 返回4bytes数据 */
	AC_REPOSITIVE_ACTIVE_RATE2_POWER, 	/* 反向有功费率2电能, 返回4bytes数据 */
	AC_REPOSITIVE_ACTIVE_RATE3_POWER, 	/* 反向有功费率3电能, 返回4bytes数据 */
	AC_REPOSITIVE_ACTIVE_RATE4_POWER, 	/* 反向有功费率4电能, 返回4bytes数据 */

	AC_POSITIVE_WATTLESS_POWER, 		/* 正向无功总电能, 返回4bytes数据 */
	AC_POSITIVE_WATTLESS_RATE1_POWER, 	/* 正向无功费率1电能, 返回4bytes数据 */
	AC_POSITIVE_WATTLESS_RATE2_POWER, 	/* 正向无功费率2电能, 返回4bytes数据 */
	AC_POSITIVE_WATTLESS_RATE3_POWER, 	/* 正向无功费率3电能, 返回4bytes数据 */
	AC_POSITIVE_WATTLESS_RATE4_POWER, 	/* 正向无功费率4电能, 返回4bytes数据 */

	AC_REPOSITIVE_WATTLESS_POWER, 		/* 反向无功总电能, 返回4bytes数据 */
	AC_REPOSITIVE_WATTLESS_RATE1_POWER, /* 反向无功费率1电能, 返回4bytes数据 */
	AC_REPOSITIVE_WATTLESS_RATE2_POWER, /* 反向无功费率2电能, 返回4bytes数据 */
	AC_REPOSITIVE_WATTLESS_RATE3_POWER, /* 反向无功费率3电能, 返回4bytes数据 */
	AC_REPOSITIVE_WATTLESS_RATE4_POWER, /* 反向无功费率4电能, 返回4bytes数据 */

	AC_A_POSITIVE_ACTIVE_POWER, 		/* (当前)A相正向有功电能, 返回4bytes数据 */
	AC_A_REPOSITIVE_ACTIVE_POWER, 		/* (当前)A相反向有功电能, 返回4bytes数据 */
	AC_B_POSITIVE_ACTIVE_POWER, 		/* (当前)B相正向有功电能, 返回4bytes数据 */
	AC_B_REPOSITIVE_ACTIVE_POWER, 		/* (当前)B相反向有功电能, 返回4bytes数据 */
	AC_C_POSITIVE_ACTIVE_POWER, 		/* (当前)C相正向有功电能, 返回4bytes数据 */
	AC_C_REPOSITIVE_ACTIVE_POWER, 		/* (当前)C相反向有功电能, 返回4bytes数据 */

	AC_A_VOLTAGE, 						/* A相电压, 返回2bytes数据 */
	AC_B_VOLTAGE, 						/* B相电压, 返回2bytes数据 */
	AC_C_VOLTAGE, 						/* C相电压, 返回2bytes数据 */
	AC_A_CURRENT, 						/* A相电流, 返回2bytes数据 */
	AC_B_CURRENT, 						/* B相电流, 返回2bytes数据 */
	AC_C_CURRENT, 						/* C相电流, 返回2bytes数据 */
	AC_INSTANT_ACTIVE_POWER, 			/* 瞬时有功功率, 返回3bytes数据 */
	AC_A_ACTIVE_POWER, 					/* A相有功功率, 返回3bytes数据 */
	AC_B_ACTIVE_POWER, 					/* B相有功功率, 返回3bytes数据 */
	AC_C_ACTIVE_POWER, 					/* C相有功功率, 返回3bytes数据 */
	AC_INSTANT_REACTIVE_POWER, 			/* 瞬时无功功率, 返回2bytes数据 */
	AC_A_REACTIVE_POWER, 				/* A相无功功率, 返回2bytes数据 */
	AC_B_REACTIVE_POWER, 				/* B相无功功率, 返回2bytes数据 */
	AC_C_REACTIVE_POWER, 				/* C相无功功率, 返回2bytes数据 */
	AC_TOTAL_POWER_FACTOR, 				/* 总功率因数, 返回2bytes数据 */
	AC_A_POWER_FACTOR, 					/* A相功率因数, 返回2bytes数据 */
	AC_B_POWER_FACTOR, 					/* B相功率因数, 返回2bytes数据 */
	AC_C_POWER_FACTOR, 					/* C相功率因数, 返回2bytes数据 */
	AC_TOTAL_APPARENT_POWER,			/* 瞬时总视在功率, 返回3bytes数据 */
	AC_A_APPARENT_POWER,				/* 瞬时A相视在功率, 返回3bytes数据 */
	AC_B_APPARENT_POWER,				/* 瞬时B相视在功率, 返回3bytes数据 */
	AC_C_APPARENT_POWER,				/* 瞬时C相视在功率, 返回3bytes数据 */
	AC_A_VOLTAGE_DISTORTION,			/* A相电压波形失真度, 返回2bytes数据 */
	AC_B_VOLTAGE_DISTORTION,			/* B相电压波形失真度, 返回2bytes数据 */
	AC_C_VOLTAGE_DISTORTION,			/* C相电压波形失真度, 返回2bytes数据 */
	AC_A_CURRENT_DISTORTION,			/* A相电流波形失真度, 返回2bytes数据 */
	AC_B_CURRENT_DISTORTION,			/* B相电流波形失真度, 返回2bytes数据 */
	AC_C_CURRENT_DISTORTION,			/* C相电流波形失真度, 返回2bytes数据 */
	AC_A_VOLTAGE_HARMONIC,				/* A相电压谐波含量数据块, 返回42bytes数据 */
	AC_B_VOLTAGE_HARMONIC,				/* B相电压谐波含量数据块, 返回42bytes数据  */
	AC_C_VOLTAGE_HARMONIC,				/* C相电压谐波含量数据块, 返回42bytes数据  */
	AC_A_CURRENT_HARMONIC,				/* A相电流谐波含量数据块, 返回42bytes数据  */
	AC_B_CURRENT_HARMONIC,				/* B相电流谐波含量数据块, 返回42bytes数据  */
	AC_C_CURRENT_HARMONIC,				/* C相电流谐波含量数据块, 返回42bytes数据  */
	AC_DATE_AND_WEEK,					/* 日期及周次,年月日星期,返回4bytes数据  */
	AC_AMMETER_TIME,					/* 时间,时分秒,返回3bytes数据 */
	AC_POWER_CONSTANT,					/* 电表常数(有功), 返回3bytes数据  */
	AC_GET_AMMETR_ADDR, 				/* 电表的表号, 返回6bytes数据, 高位在前 */
	AC_POWER_FREQUENCY, 				/* 电网频率 , 返回2bytes数据 */
	AC_AMMETER_TEMPERATURE, 			/* 表内温度度, 返回2bytes数据 */
	AC_CLOCK_BATTERY_VOLTAGE, 			/* 时钟电池电压(内部), 返回2bytes数据 */
	AC_METER_READ_BATTERY_VOLTAGE, 		/* 停电抄表电池电压 (外部), 返回2bytes数据 */
	AC_TOTAL_COPPER_LOSS_ACTIVE_POWER, 	/* (当前)铜损有功总电能补偿量 , 返回4bytes数据 */
	AC_A_COPPER_LOSS_ACTIVE_POWER, 		/* (当前)A相铜损有功电能补偿量, 返回4bytes数据 */
	AC_B_COPPER_LOSS_ACTIVE_POWER, 		/* (当前)B相铜损有功电能补偿量 , 返回4bytes数据 */
	AC_C_COPPER_LOSS_ACTIVE_POWER, 		/* (当前)C相铜损有功电能补偿量, 返回4bytes数据  */
	AC_TOTAL_IRON_LOSS_ACTIVE_POWER, 	/* (当前)铁损有功总电能补偿量, 返回4bytes数据  */
	AC_A_IRON_LOSS_ACTIVE_POWER, 		/* (当前)A相铁损有功电能补偿量, 返回4bytes数据  */
	AC_B_IRON_LOSS_ACTIVE_POWER, 		/* (当前)B相铁损有功电能补偿量, 返回4bytes数据  */
	AC_C_IRON_LOSS_ACTIVE_POWER, 		/* (当前)C相铁损有功电能补偿量 , 返回4bytes数据 */
};

enum ammeter_forzen_time_cmd {
	CMD_FREEZING_MONTH = 1,					/* 以月为周期定时冻结 */
	CMD_FREEZING_DAY,					/* 以日为周期定时冻结 */
	CMD_FREEZING_HOUR,					/* 以小时为周期定时冻结 */
	CMD_FREEZING_NOW					/* 瞬时冻结 */
};

enum ammeter_forzen_cmd {
	CMD_FORZEN_TIMEING_TIME,     					/* （上1次）定时冻结时间, 返回5bytes数据  */
	CMD_FORZEN_TIMEING_POSITIVE_ACTIVE_POWER,   	/* （上1次）定时冻结正向有功总电能数据, 返回4bytes数据  */
	CMD_FORZEN_TIMEING_REPOSITIVE_ACTIVE_POWER, 	/* （上1次）定时冻结反向有功总电能数据, 返回4bytes数据  */
	CMD_FORZEN_TIMEING_POSITIVE_WATTLESS_POWER,		/* （上1次）定时冻结正向无功总电能数据, 返回4bytes数据  */
	CMD_FORZEN_TIMEING_REPOSITIVE_WATTLESS_POWER,	/* （上1次）定时冻结反向无功总电能数据, 返回4bytes数据  */
	CMD_FORZEN_TIMEING_POSITIVE_MAXNEED_AND_TIME, 	/* （上1次）定时冻结正向有功总最大需量及发生时间数据, 返回8bytes数据  */
	CMD_FORZEN_TIMEING_REPOSITIVE_MAXNEED_AND_TIME, /* （上1次）定时冻结反向有功总最大需量及发生时间数据, 返回8bytes数据  */
	CMD_FORZEN_TIMEING_VARIABLE_DATA, 				/* （上1次）定时冻结变量数据, 返回3×8bytes数据  */

	CMD_FORZEN_INSTANT_TIME,     					/* （上1次）瞬时冻结时间, 返回5bytes数据  */
	CMD_FORZEN_INSTANT_POSITIVE_ACTIVE_POWER,   	/* （上1次）瞬时冻结正向有功总电能数据, 返回4bytes数据  */
	CMD_FORZEN_INSTANT_REPOSITIVE_ACTIVE_POWER, 	/* （上1次）瞬时冻结反向有功总电能数据, 返回4bytes数据  */
	CMD_FORZEN_INSTANT_POSITIVE_WATTLESS_POWER,		/* （上1次）瞬时冻结正向无功总电能数据, 返回4bytes数据  */
	CMD_FORZEN_INSTANT_REPOSITIVE_WATTLESS_POWER,	/* （上1次）瞬时冻结反向无功总电能数据, 返回4bytes数据  */
	CMD_FORZEN_INSTANT_POSITIVE_MAXNEED_AND_TIME, 	/* （上1次）瞬时冻结正向有功总最大需量及发生时间数据, 返回8bytes数据  */
	CMD_FORZEN_TINSTANT_REPOSITIVE_MAXNEED_AND_TIME,/* （上1次）瞬时冻结反向有功总最大需量及发生时间数据, 返回8bytes数据  */
	CMD_FORZEN_INSTANT_VARIABLE_DATA, 				/* （上1次）瞬时冻结变量数据, 返回3×8bytes数据  */
};

enum ammeter_maxneed_cmd {
	CMD_MAXNEED_TOTAL_POSITIVE_ACTIVE,     	/* (当前)正向有功总最大需量及发生时间, 返回8bytes数据  */
	CMD_MAXNEED_RATE1_POSITIVE_ACTIVE,     	/* (当前)正向有功费率1最大需量及发生时间, 返回8bytes数据  */
	CMD_MAXNEED_RATE2_POSITIVE_ACTIVE,     	/* (当前)正向有功费率2最大需量及发生时间, 返回8bytes数据  */
	CMD_MAXNEED_RATE3_POSITIVE_ACTIVE,     	/* (当前)正向有功费率3最大需量及发生时间, 返回8bytes数据  */
	CMD_MAXNEED_RATE4_POSITIVE_ACTIVE,     	/* (当前)正向有功费率4最大需量及发生时间, 返回8bytes数据  */

	CMD_MAXNEED_TOTAL_REPOSITIVE_ACTIVE,     /* (当前)反向有功总最大需量及发生时间, 返回8bytes数据  */
	CMD_MAXNEED_RATE1_REPOSITIVE_ACTIVE,     /* (当前)反向有功费率1最大需量及发生时间, 返回8bytes数据  */
	CMD_MAXNEED_RATE2_REPOSITIVE_ACTIVE,     /* (当前)反向有功费率2最大需量及发生时间, 返回8bytes数据  */
	CMD_MAXNEED_RATE3_REPOSITIVE_ACTIVE,     /* (当前)反向有功费率3最大需量及发生时间, 返回8bytes数据 */
	CMD_MAXNEED_RATE4_REPOSITIVE_ACTIVE,     /* (当前)反向有功费率4最大需量及发生时间, 返回8bytes数据 */
};

/* 电表事件上传命令标识 */
enum ammeter_event_cmd {
	AMM_EVENT_NOTHING,
	AMM_EVENT_LOSE_VOLTAGE,		/* ABC相失压总次数，总累计时间 */
	/* A相失压记录，返回29bytes数据，
	发生时刻6bytes，结束时刻6bytes，
	失压时刻A相电压2bytes，失压时刻A相电流3bytes，
	失压时刻A相有功功率3bytes，失压时刻A相无功功率3bytes，
	失压时刻A相功率因数2bytes，失压期间A相安时数4bytes*/
	AMM_EVENT_A_LOSE_VOLTAGE,
	AMM_EVENT_B_LOSE_VOLTAGE,	/* B相失压记录， (同A相失压)*/
	AMM_EVENT_C_LOSE_VOLTAGE,	/* C相失压记录，(同A相失压) */

	AMM_EVENT_OWE_VOLTAGE,		/* ABC相欠压总次数，总累计时间 */
	AMM_EVENT_A_OWE_VOLTAGE,	/* A相欠压记录，(同A相失压) */
	AMM_EVENT_B_OWE_VOLTAGE,	/* B相欠压记录， (同A相失压) */
	AMM_EVENT_C_OWE_VOLTAGE,	/* C相欠压记录， (同A相失压) */

	AMM_EVENT_OVER_VOLTAGE,		/* ABC相过压总次数，总累计时间 */
	AMM_EVENT_A_OVER_VOLTAGE,	/* A相过压记录， (同A相失压) */
	AMM_EVENT_B_OVER_VOLTAGE,	/* B相过压记录，(同A相失压)  */
	AMM_EVENT_C_OVER_VOLTAGE,	/* C相过压记录，(同A相失压)  */

	AMM_EVENT_BROKEN_PHASE,		/* ABC相断相总次数，总累计时间 */
	AMM_EVENT_A_BROKEN_PHASE,	/* A相断相记录， (同A相失压) */
	AMM_EVENT_B_BROKEN_PHASE,	/* B相断相记录， (同A相失压) */
	AMM_EVENT_C_BROKEN_PHASE,	/* C相断相记录， (同A相失压) */

	AMM_EVENT_LOSE_CURRENT, 	/* ABC相失流总次数，总累计时间 */
	/* A相失流记录，返回25bytes数据，
	发生时刻6bytes，结束时刻6bytes，
	失流时刻A相电压2bytes，失流时刻A相电流3bytes ，
	失流时刻A相有功功率3bytes，失流时刻A相无功功率3bytes，
	失流时刻A相功率因数2bytes*/
	AMM_EVENT_A_LOSE_CURRENT,
	AMM_EVENT_B_LOSE_CURRENT,	/* B相失流记录， (同失流) */
	AMM_EVENT_C_LOSE_CURRENT,	/* C相失流记录， (同失流) */

	AMM_EVENT_OVER_CURRENT, /* ABC相过流总次数，总累计时间 */
	AMM_EVENT_A_OVER_CURRENT, 	/* A相过流记录，  (同失流) */
	AMM_EVENT_B_OVER_CURRENT, 	/* B相过流记录， (同失流)  */
	AMM_EVENT_C_OVER_CURRENT, 	/* C相过流记录， (同失流)  */

	AMM_EVENT_BROKEN_CURRENT, 	/* ABC相断流总次数，总累计时间 */
	AMM_EVENT_A_BROKEN_CURRENT, /* A相断流记录， (同失流)  */
	AMM_EVENT_B_BROKEN_CURRENT, /* B相断流记录， (同失流)  */
	AMM_EVENT_C_BROKEN_CURRENT, /* C相断流总次数，(同失流) */

	AMM_EVENT_AMM_RESET, 		/* 电表清零总次数 */
	/* 电表清零记录,返回106bytes数据，
	发生时刻6bytes,
	操作者代码4bytes,
	电表清零前正向有功总电能4bytes,
	电表清零前反向有功总电能4bytes,
	电表清零前第一象限无功总电能4bytes,
	电表清零前第二象限无功总电能4bytes,
	电表清零前第三象限无功总电能4bytes,
	电表清零前第四象限无功总电能4bytes,
	电表清零前A相正向有功电能 4bytes,
	电表清零前A相反向有功电能4bytes,
	电表清零前A相第一象限无功电能4bytes,
	电表清零前A相第二象限无功电能4bytes,
	电表清零前A相第三象限无功电能4bytes,
	电表清零前A相第四象限无功电能4bytes,
	电表清零前B相正向有功电能4bytes,
	电表清零前B相反向有功电能4bytes,
	电表清零前B相第一象限无功电能4bytes,
	电表清零前B相第二象限无功电能4bytes,
	电表清零前B相第三象限无功电能4bytes,
	电表清零前B相第四象限无功电能4bytes,
	电表清零前C相正向有功电能4bytes,
	电表清零前C相反向有功电能4bytes,
	电表清零前C相第一象限无功电能4bytes,
	电表清零前C相第二象限无功电能4bytes,
	电表清零前C相第三象限无功电能4bytes,
	电表清零前C相第四象限无功电能4bytes,*/
	AMM_EVENT_AMMETER_RESET,

	AMM_EVENT_REQUIRED_RESET, 	/* 需量清零总次数 */
	AMM_EVENT_NEED_RESET,		/* 需量清零记录， 返回10bytes数据:发生时刻6bytes,操作者代码4bytes */

	AMM_EVENT_CALIBRATION_TIME, /* 校时总次数 */
	AMM_EVENT_TIMING_RECORD,	/* 校时记录， 返回16bytes数据 :操作者代码4bytes,校时前时间6bytes,校时后时间6bytes */

	AMM_EVENT_PROGRAMMING, 		/* 编程总次数 */
	AMM_EVENT_PROGRAM_RECORD,	/* 编程记录， 返回50bytes数据:发生时刻6bytes,操作者代码4bytes,编程的前10个数据标识码(不足补FFFFFFFFH) 4×10bytes */

	AMM_EVENT_VOLTAGE_INVERSE_PHASE, /* 电压逆相序总次数，总累计时间 */
	AMM_EVENT_VOLTAGE_ANTI_PHASE_RECORD,/* 电压逆相序记录， 返回12bytes数据:发生时刻6bytes,结束时刻6bytes */

	AMM_EVENT_CURRENT_INVERSE_PHASE, /* 电流逆相序总次数，总累计时间 */
	AMM_EVENT_CURRENT_ANTI_PHASE_RECORD,/* 电流逆相序记录， 返回12bytes数据:发生时刻6bytes,结束时刻6bytes */
};



/******************电能表类型***************/
enum ammeter_style {
	AM_UNKNOWN = 0x00,
//	AM_SIGNLE_PHASE,
//	AM_MUX_FUNC,
//	AM_YUFUFEI,
//	AM_FUFEILV,

	AM_MK3,
	AM_MK6,
	AM_MK6E
};

/****** 读表数据分类 *********/
enum data_flag_style {
	DATA_FLAG_UNKNOWN = 0x00,
	DATA_FLAG_POWER,				/* 电能量 */
	DATA_FLAG_MAXNEED_TIME,			/* 最大需量及发生时间 */
	DATA_FLAG_VARIABLES,           	/* 变量 */
	DATA_FLAG_PARAMETER_VARIABLES,	/* 参变量 */
	DATA_FLAG_EVENT,            	/* 事件 */
	DATA_FLAG_FROZEN_DATA,          /* 冻结数据 */
	DATA_FLAG_LOAD_RECORDS          /* 负荷记录数据 */
};


//电能表参数
struct Ammeter_Node
{
    rt_uint8_t ammeter_addr[6];                  //电能表地址域
    enum ammeter_style ammeter_style;          	//电能表类型
    enum ammeter_protocal_e ammeter_protocal;    //电能表数据传输所用协议
    struct uart_485_param ammeter_baud;          	//电能表所用串口的波特率
};

#if 0
//电能表参数索引
struct  Ammeter_Index
{
	rt_uint32_t ammeter_num;
	struct  Ammeter_Node  ammeter_data[NUM_OF_COLLECT_EM_MAX];
};
#endif

#if 0
//电能表各类型的数量
struct  AmmeterStyleNum
{
	rt_uint8_t Single;
	rt_uint8_t FuFeiLv;
	rt_uint8_t YuFuFei;
	rt_uint8_t MuxFunc;
};
#endif


#if 0
struct Item_Infor
{
	rt_uint8_t item_di[4];     /* 数据标识 */
	rt_uint8_t item_length;    /* 数据标识对应的数据长度 */
};
/* 电能表类型数据标识索引 */
struct Ammeter_Style
{
	rt_uint8_t ammeter_style;       	/* 电能表类型 */
	rt_uint8_t item_num;           	 	/* 数据标识的数量 */
	struct Item_Infor item_addr[2]; 	/* 数据标识首地址 */
};
#endif


#if 0
/*** 帧格式各参数 ***/
struct frame_format_param
{
	rt_uint8_t src_addr[6];         /* 源地址 */
	rt_uint8_t dest_addr[6];        /* 目的地址 */
	rt_uint8_t ctrl;				/* 控制码 */
	enum ammeter_cmd_e data_flag;	/* 数据标识 */
	rt_uint8_t *data;				/* 存储发送/接收数据的起始地址 */
	rt_uint32_t data_len;			/* 发送/接收的数据长度, 不是缓冲区的长度 */
};
#endif


#define TRY_GET_EM_DATA_CNT_MAX	(3)

#define EM_PROTO_INTER_FRAME_TIMEOUT_MS	(500+50)
#define EM_PROTO_INTER_BYTE_TIMEOUT_MS	(100)



/* 初始化电表函数 */
extern rt_err_t ammeter_init(void);
/* 判断电表所用的规约  */
extern rt_err_t try_get_em_protoco_baud_info(void);

/* 判断事件是否发生,并上报发生的事件 */
extern enum frame_error_e reported_ammeter_happened_event(rt_uint8_t *str_addr, enum ammeter_uart_e port);
/* 广播设置电表时间 */
extern rt_err_t setting_all_ammeter_time(struct ammeter_time *time, enum ammeter_uart_e port);
/* 设置电表的定时冻结时间 */
extern enum frame_error_e setting_ammeter_forzen_data_time(rt_uint8_t *str_addr, enum ammeter_forzen_time_cmd cmd,
			struct ammeter_time *time, enum ammeter_uart_e port);
/* 获取电表的电能量数据 */
extern enum frame_error_e get_power_data_from_ammeter(rt_uint8_t *str_addr, enum ammeter_cmd_e cmd,
		rt_uint8_t *output_data, rt_uint32_t *output_data_len, enum ammeter_uart_e port);
/* 获取电表的冻结数据 */
extern enum frame_error_e get_forzen_data_from_ammeter(rt_uint8_t *str_addr,
		enum ammeter_forzen_cmd cmd, rt_uint8_t forzen_times, rt_uint8_t *output_data,
		rt_uint32_t *output_data_len, enum ammeter_uart_e port);
/* 获取电表的最大需量数据 */
extern enum frame_error_e get_maxneed_data_from_ammeter(rt_uint8_t *str_addr,
		enum ammeter_maxneed_cmd cmd, rt_uint8_t *output_data,
		rt_uint32_t *output_data_len, enum ammeter_uart_e port);
/* 获取电表的事件数据 */
extern enum frame_error_e get_event_data_from_ammeter(rt_uint8_t *str_addr,
		enum ammeter_event_cmd cmd, rt_uint8_t event_times, rt_uint8_t *output_data,
		rt_uint32_t *output_data_len, enum ammeter_uart_e port);

extern enum frame_error_e get_data_from_645_97(rt_uint8_t *addr, rt_uint32_t data_flag, rt_uint8_t *output_data, rt_uint32_t *output_data_len, enum ammeter_uart_e port);
extern enum frame_error_e get_data_from_645_07(rt_uint8_t *addr, rt_uint32_t data_flag, rt_uint8_t *output_data, rt_uint32_t *output_data_len, enum ammeter_uart_e port);

extern rt_err_t output645_97power_data_flag(enum ammeter_cmd_e cmd, rt_uint32_t *output_data_flag, rt_uint32_t *output_move_len);
extern rt_err_t output645_07power_data_flag(enum ammeter_cmd_e cmd, rt_uint32_t *output_data_flag);

extern enum ammeter_baud_cmd uart_baud;
/* 获取储存在全局变量中的电表所用串口波特率 */
extern rt_err_t get_save_ammeter_baud(rt_uint8_t *addr, struct uart_485_param *uart_485_data);

extern void update_elctric_meter_info(void);

#endif
