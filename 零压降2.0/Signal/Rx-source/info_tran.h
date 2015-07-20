/*
 * info_tran.h
 *
 * 2013-03-03,  creat by David, zhaoshaowei@yeejoin.com
 */

#ifndef INFO_TRAN_H__
#define INFO_TRAN_H__

#if 0
#include <rtdef.h>
#include <zvd_gui_app.h>

/*
 * 年份 year
 * 产地 place of production
 * 批次 batch
 * 发射/接收
 * 序号 serial NO
 */
#define DEV_SN_MODE "yypp-bbxssss"
#define DEV_SN_MODE_LEN  12
#define DEV_SN_5BITS_CNT 20

#define DEV_SN_5BITS_BUF_LEN 48

#define DEV_SN_LEN_MAX 23
#endif

/* 一个ID占用4bits
 *
 * 例如传送a相平均值共需传送5 bytes
 * (ITID_TXPA_INFO<<4 | PII_AVG_VAL) 
 * (0000 avgval_bit[15:12])  (0000 avgval_bit[11:8]) (0000 avgval_bit[7:4])  (0000 avgval_bit[3:0])
 */

/* 0 -- 15, 一级类型id */
enum info_type_id {
	ITID_NONE = 0,
	ITID_TXPA_INFO = 1,
	ITID_TXPB_INFO = 2,
	ITID_TXPC_INFO = 3,
	ITID_RXPA_INFO = 4,
	ITID_RXPB_INFO = 5,
	ITID_RXPC_INFO = 6,

	ITID_TXE_INFO = 7,
	ITID_RXE_INFO = 8,

	ITID_RXE_ACK  = 9,
};

/* 0 -- 15, 二级类型id */
enum px_info_id {
	PII_BINARY_INFO = 1, /* bit3:欠压、bit2:过压、bit1:缺相(采集端)/无输出(恢复端) */
	PII_AVG_VAL	= 2, /* 平均值(16bits) */
};

/* 0 -- 15, 二级类型id */
enum trxe_info_id {
	TRXII_BINARY_INFO 		= 1, /* bit3:采集端掉电/恢复端切换至PT、bit2:接收端接到光纤数据 */
	TRXII_CPU_TEMP	  		= 2, /* CPU温度(16bits) */
	TRXII_BOX_TEMP   		= 3, /* 恒温盒温度(16bits) */
	TRXII_VERSION	  		= 4, /* 软件版本号(24bits) */
	TRXII_RX_CONTINUE_OVERLOAD_CNT	= 5, /* 发送采集端连续检测到过载次数(4bits) */
	TRXII_DEV_SN	  		= 6, /* 发射端SN(DEV_SN_MODE_LEN bytes) */
	TRXII_USE_FIBER_NO		= 7, /* 当前使用的光纤头号, 0--表示已切换到pt侧, 1--使用1号光纤头, 2--使用2号光纤头
					      * 常量定义见 enum sys_use_channel_e
	 	 	 	 	      */
};

enum uart3_ack_format {
	TRX_SWITCH2PT = 1,/* 已经切换到PT侧 */
};

enum uart3_cmd_format {
	CMD_HEAD  = 0x55,/* 命令头 */

	SWITCH2PT 	= 0x56,	/* 切换到PT侧 */
	SWITCH_FROM_PT	= 0x57,	/* "切换到零压降的输出端" */
};

#define creat_info_id(m_id, s_id) ((m_id)<<4 | (s_id))


enum sys_use_channel_e {
	SUC_PT_CHANNEL		= 0,
	SUC_FIBER_CHANNEL_1,
	SUC_FIBER_CHANNEL_2,

	SUC_PAD,

	SUC_FORCE_SWITCH2PT,

	SUC_BUTT
};

#if 0
#define SYS_MISC_BIT_TXE_POWEROFF		0X01
#define SYS_MISC_BIT_RXE_SWITCH2PT		0X02
#define SYS_MISC_BIT_FIBRE_OPTICAL		0X04

#define is_switch2pt_sysmisc() (is_bit_set(sys_misc_info, SYS_MISC_BIT_RXE_SWITCH2PT))
#define set_switch2pt_flag_sysmisc() (set_bit(sys_misc_info, SYS_MISC_BIT_RXE_SWITCH2PT))
#define clr_switch2pt_flag_sysmisc() (clr_bit(sys_misc_info, SYS_MISC_BIT_RXE_SWITCH2PT))

#define is_fibre_recv_data_sysmisc() (is_bit_set(sys_misc_info, SYS_MISC_BIT_FIBRE_OPTICAL))
#define set_fibre_recv_data_flag_sysmisc() (set_bit(sys_misc_info, SYS_MISC_BIT_FIBRE_OPTICAL))
#define clr_fibre_recv_data_flag_sysmisc() (clr_bit(sys_misc_info, SYS_MISC_BIT_FIBRE_OPTICAL))

extern unsigned long se_version;
extern unsigned long re_version;
extern volatile unsigned long sys_misc_info;


extern void info_tran_stream_analysis(rt_device_t dev);
extern int convert_ver2str(unsigned int ver, char *str);

extern int send_cmd_to_rxe(int is_need_start_timer, enum uart3_cmd_format cmd);

extern int send_cmd2workbench(enum app_workbench_cmd_id cmd, char *cmd_str);

extern void init_sys_use_channel(void);
extern int set_sys_use_channel(enum sys_use_channel_e channel);
extern enum sys_use_channel_e get_sys_use_channel(void);
#endif
#endif
