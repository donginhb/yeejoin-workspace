/*
 * 2012-01-13 16:15:45
 */
#ifndef _SYSCFGDATA_H_
#define _SYSCFGDATA_H_

#include <rtdef.h>
#include <ammeter_common.h>
#include <syscfgdata-common.h>
#include <sinkinfo_common.h>

#if RT_USING_TCPIP_STACK
#include <lwipusrlib.h>
#include <lwip/ip_addr.h>
#include <lwip/api.h>
#include <lwip/snmp_msg.h>
#include <lwip/inet.h>

/* ip address of target*/
#define RT_LWIP_IPADDR0	172
#define RT_LWIP_IPADDR1	24
#define RT_LWIP_IPADDR2	4
#define RT_LWIP_IPADDR3	219

/* gateway address of target*/
#define RT_LWIP_GWADDR0	172
#define RT_LWIP_GWADDR1	24
#define RT_LWIP_GWADDR2	4
#define RT_LWIP_GWADDR3	254

/* mask address of target*/
#define RT_LWIP_MSKADDR0	255
#define RT_LWIP_MSKADDR1	255
#define RT_LWIP_MSKADDR2	255
#define RT_LWIP_MSKADDR3	0

/**
 * Default TCP/IP Address Configuration.  Static IP Configuration is used if
 * DHCP times out.
 */
#define DEFAULT_IPADDR        ((RT_LWIP_IPADDR0 << 24) | (RT_LWIP_IPADDR1 << 16) | (RT_LWIP_IPADDR2 << 8) | RT_LWIP_IPADDR3)
#define DEFAULT_GATEWAY_ADDR  ((RT_LWIP_GWADDR0 << 24) | (RT_LWIP_GWADDR1 << 16) | (RT_LWIP_GWADDR2 << 8) | RT_LWIP_GWADDR3)
#define DEFAULT_NET_MASK      ((RT_LWIP_MSKADDR0 << 24) | (RT_LWIP_MSKADDR1 << 16) | (RT_LWIP_MSKADDR2 << 8) | RT_LWIP_MSKADDR3)

#define IP_INTERFACE_NUM 1
#endif /* RT_USING_TCPIP_STACK */

#define RS232_INTERFACE_NUM 3
#define RS485_INTERFACE_NUM 2

#define USR_NAME_LEN_MAX    31
#define PW_LEN_MAX          31
#define USR_NUM_MAX 1

/* data had modify */
#define SYSTBL_FLAG_DATA_DIRTY		0x01
/* write data to flash immediately */
#define SYSTBL_FLAG_WRITE_THROUGH 	0x02



/* epon设备ip */
#define EPON_DEV_DETECT_EN_BIT  0X01
#define EPON_DEV_ALARM_MASK_BIT 0X02

#define OOK	0x01
#define FSK	0x02
#define GFSK	0x03

#define RS232_DEFAULT_BAUDRATE   (115200)
#define RS232_DEFAULT_DATABITS   (8)
#define RS232_DEFAULT_PARITYBIT  (UART_PAR_NONE)
#define RS232_DEFAULT_STOPBITS   (1)

/* 温度使用有符号数, 是实际值的10倍 */
#define TMP_DEFAULT_LIMEN_H	(450)
#define TMP_DEFAULT_LIMEN_L	(30)

/* 相对湿度使用无符号数, 是实际值的10倍 */
#define RH_DEFAULT_LIMEN_H	(800)
#define RH_DEFAULT_LIMEN_L	(100)

/*
    // OUI 00-80-E1 STMICROELECTRONICS
    stm32_eth_device.dev_addr[0] = 0x00;
    stm32_eth_device.dev_addr[1] = 0x80;
    stm32_eth_device.dev_addr[2] = 0xE1;
    // generate MAC addr from 96bit unique ID (only for test)
    stm32_eth_device.dev_addr[3] = *(rt_uint8_t*)(0x1FFFF7E8+7);
    stm32_eth_device.dev_addr[4] = *(rt_uint8_t*)(0x1FFFF7E8+8);
    stm32_eth_device.dev_addr[5] = *(rt_uint8_t*)(0x1FFFF7E8+9);

 */
#define MACOCT5 (*(rt_uint8_t*)(0x1FFFF7E8+9))
#define MACOCT4 (*(rt_uint8_t*)(0x1FFFF7E8+8))
#define MACOCT3 (*(rt_uint8_t*)(0x1FFFF7E8+7))
#define MACOCT2 (0XE1)
#define MACOCT1 (0X80)
#define MACOCT0 (0X00)


/*
 * 网管相关参数
 */
#define WEBM_P_SERVER_DEFAULT_IP    (192<<24 | 168<<16 | 1<<8 | 21)
#define WEBM_P_SERVER_DEFAULT_PORT  (1234)
#define WEBM_P_SERVER_DEFAULT_PROTO (NETCONN_TCP)

/* touch chip
 *
 */
#if 0
/* ssd1289 */
#define Y_ADC_MIN 723
#define Y_ADC_MAX 3562

#define X_ADC_MIN 771
#define X_ADC_MAX 3694
#else
/* ili9320 */
#define Y_ADC_MIN 3841
#define Y_ADC_MAX 222

#define X_ADC_MIN 355
#define X_ADC_MAX 3934
#endif

#define RTC_CALIBRATION_DEF_VALUE 0xff

/*
 * baudrate:
 * databits: 这里时严格意义上的数据位，不包含校验位(stm32的uart的databits包含校验位)
 * paritybit: 0 -- No parity, 1 -- Even parity, 2 -- Odd parity
 * stopbits: 1 -- 1, 15 -- 1.5, 2 -- 2
 *
 * #define UART_PAR_NONE    0  // No parity
 * #define UART_PAR_EVEN    1  // Even parity
 * #define UART_PAR_ODD     2  // Odd parity
 * */
struct rs232_param {
	rt_uint32_t baudrate;

	rt_uint8_t databits;
	rt_uint8_t paritybit;
	rt_uint8_t stopbits;
	rt_uint8_t cur_alarm;	      /* current alarm*/

	rt_uint8_t alarm_mask;	      /* alarm mask */
	rt_uint8_t reserve[3];
};


struct usr_pw_pair {
	char usr[USR_NAME_LEN_MAX+1];
	char pw[PW_LEN_MAX+1];
};

struct login_param {
	struct usr_pw_pair login;
};

struct tab_head {
	rt_uint32_t magic_num; /* NOTE: must place at the first word of table */
};

/* !!NOTE: boot 中有用 */
struct m3_sys_info_st {
	/* rt_uint32_t hw_ver; */	/* 根据I/O口拨码开关确定硬件版本号 */
	rt_uint32_t sw_ver;		/* 用于远程升级版本号的判断 */
	rt_int32_t  sw_file_mtime;	/* the mtime of os-app rbin-file */
	rt_uint32_t db_ver;		/* 数据库版本 */
	char dev_sn[DEV_SN_BUF_STRING_WITH_NUL_LEN_MAX];	/* 设备序列号 */
	char dev_descr[32];		/*设备信息描述 */ //hongbin E
	rt_uint32_t neid;		/* 网元ID */
	rt_uint16_t delay_auto_elock_time; /* 自动延迟关闭elock的时间, 单位是分钟 */
	rt_uint16_t m3sys_reserve;
};

/*
 * 内部存储时, 温度按照有符号数的补码存储
 */
struct temp_rh_limen {
	rt_int16_t	temp_h;
	rt_int16_t	temp_l;
	rt_uint16_t	rh_h;
	rt_uint16_t	rh_l;
	rt_uint16_t	warn_mask; /* 用于确定本地的led灯在告警时是否闪烁, 1-mask, 0-unmask bit0:temperature, bit1:rh */
	rt_uint16_t	tmp_rh_reserve;
};

struct touch_calib_param {
	rt_uint16_t	x_min;
	rt_uint16_t	x_max;
	rt_uint16_t	y_min;
	rt_uint16_t	y_max;
};

struct misc_byte_info_st {
	unsigned char rtc_cali;
	unsigned char pad0;
	unsigned char pad1;
	unsigned char pad2;
};

#if RT_USING_TCPIP_STACK
struct nms_if_info_st {
	rt_uint32_t nms_ip; 	/* ip address */
	u16	port;		/* listening port */
	u16	prot_type;	/* protocal type, enum netconn_type--api.h */
	rt_uint32_t trap_ip; 	/* ip address */
	u16	trap_port;	/* listening port */
	u16	nms_reserve;	/* reserve */
	rt_uint32_t trap_enable_bits;

};

struct epon_device_st {
	rt_uint32_t onu_ip;
	rt_uint32_t olt_ip;
	rt_uint32_t epon_bitv;
};

/* snmp的团体字 */
struct snmp_community_st {
	char get_commu[SNMP_COMMUNITY_LEN_MAX];		/* 读团体字 */
	char set_commu[SNMP_COMMUNITY_LEN_MAX];		/* 写团体字 */
	char trap_commu[SNMP_COMMUNITY_LEN_MAX];	/* trap团体字 */
};
#endif /* RT_USING_TCPIP_STACK */

//#if RT_USING_SI4432_MAC
/* 无线TinyWire configration,包括采集器,PT,CT */
struct tinywireless_if_info_st {
	int master_index;	/* 主节点索引*/
	rt_uint16_t 	sync_word; 	/* 同步字 */
	rt_uint8_t 	mod_type;	/* modulation type */
	rt_uint8_t 	mch_code;	/* manchester code */

	rt_uint16_t 	center_freq; 	/* 中心频率 */
	rt_uint8_t 	channal_num;	/* channal number */
	rt_uint8_t 	cur_alarm;	/* current alarm */
	rt_uint32_t 	Tx_dev;	     	/* TX deviation */
	rt_uint32_t 	Rx_dev;    	/* RX deviation */
	rt_uint32_t	 Rx_bw;	    	/* RX bandwith */
	rt_uint32_t 	Tx_rate;	/* transmit rate */
	rt_uint32_t 	Rx_rate;	/* receive rate */
	int   		Tx_power;	/* transmit power */
	int   		Rx_sensitivity;	/* RX sensitivity */
	rt_uint8_t 	alarm_mask;	/* alarm mask */

	uint8_t 	slave_id;	/* 用于主节点响应注册请求时, 返回节点id */
	uint8_t		channel_no;	/* 无线通道号 */
	uint8_t		cid;		/* 会话id */
};
//#endif

#if RT_USING_GPRS
/*GPRS  configration*/
struct gprs_if_info_st {
	struct ip_addr server_ipaddr; 	/*server ip address */
	struct ip_addr server_netmask;
	struct ip_addr server_gw;
	u16	server_port;		/* server listening port */
	u16	prot_type;
	char master_telnum[20];		/* 主站电话号码 */
	char sms_telnum[20];		/* 短信中心电话号码 */
	char apn_name[USR_NAME_LEN_MAX+1];  /*APN接入点名称*/
	char apn_usr[USR_NAME_LEN_MAX+1];
	char apn_pw[PW_LEN_MAX+1];
};/*共128bytes*/
#endif

#if EM_ALL_TYPE_BASE
#define CHLX_CURRENT  12
#else
#define CHLX_CURRENT  1
#endif

struct chlx_em_reg_st {
	int pa_vgain, pb_vgain, pc_vgain;		/* a相电压增益寄存器参数 */

	int pa_igain, pb_igain, pc_igain;		/* a相电流增益寄存器参数 */

	int pa_pgain, pb_pgain, pc_pgain;		/* A相功率增益调整, power gain adjust */

	int pa_wattos, pb_wattos, pc_wattos;		/* A相总有功功率失调调整, total active power offset adjust */

	int pa_vrmsos, pb_vrmsos, pc_vrmsos;		/* A相电压有效值偏移调整, total active power offset adjust */

	int pa_irmsos, pb_irmsos, pc_irmsos;		/* A相电流有效值偏移调整, total active power offset adjust */

	int cf1den, cf2den, cf3den;			/* cf1分频系数 */

	int pa_vk, pb_vk, pc_vk;			/* a相电压转换常数 */

	int pa_ik, pb_ik, pc_ik;			/* a相电流转换常数 */

	int pa_pk, pb_pk, pc_pk;			/* a相功率转换常数 */

	int pa_phase, pb_phase, pc_phase; 			/* 相位补偿 */

	u8 pabc_varthr;			/* 总无功功率CF2输出管脚比较阈值 */
	u8 pabc_wthr;			/* 总有功功率CF1输出管脚比较阈值 */
	u8 pad[2];
/* 自动调试信息 */ 
	int connect33;		/* 接线方式默认三相三 1，三相四是0 */
	int mc_constant;	/* 电表常数mc */
	u32 cf_k;		/* 脉冲输出自动调试k值 */
 	u32 cf_k1;
};
struct gateway_em_st {
	struct chlx_em_reg_st chlx_st[CHLX_CURRENT];	
};

struct hsdc_data_buffer_st {
	u32 hsdc_rec_buffer[32*80];	
};  


#define EM_SLAVE_NUM (20)
//#if WIRELESS_MASTER_NODE
struct wireless_register_white_st {
	unsigned char sn_bytes[DEV_SN_MODE_LEN];
};
//#endif

#if EM_ALL_TYPE_BASE

#define WLM_485S_FLAGS_HAD_FINISH_WL_NETCFG_BIT		(0X01)

struct wl_netcfg_param_info_st {
	rt_uint16_t 	sync_word; 	/* 同步字 */
	rt_uint16_t 	center_freq; 	/* 中心频率 */

	rt_uint8_t 	channal_num;	/* channal number */
	uint8_t		channel_no;	/* 无线通道号 */
	uint8_t		cid;		/* 会话id */
	uint8_t		pad;
};

#if 0
/* 仅用于测试数据库升级 */
struct wl_master_485_slave_info_tbl_prev {
	uint8_t pt_sn[WIRELESS_MASTER_NODE_MAX][DEV_SN_MODE_LEN];
	struct wl_netcfg_param_info_st wl_param[WIRELESS_MASTER_NODE_MAX];

	/*
	 * bit0 -- 是否已完成无线半自动组网, WLM_485S_FLAGS_HAD_FINISH_WL_NETCFG_BIT
	 * */
	rt_uint8_t	flags;
	rt_uint8_t	reserve1;
	rt_uint16_t	reserve2;
};

struct wl_master_485_slave_info_tbl {
	uint8_t pt_sn[WIRELESS_MASTER_NODE_MAX][DEV_SN_MODE_LEN];
	struct wl_netcfg_param_info_st wl_param[WIRELESS_MASTER_NODE_MAX];

	/*
	 * bit0 -- 是否已完成无线半自动组网, WLM_485S_FLAGS_HAD_FINISH_WL_NETCFG_BIT
	 * */
	rt_uint8_t	flags;
	rt_uint8_t	reserve1;
	rt_uint16_t	reserve2;
	uint32_t temp;
};
#else
struct wl_master_485_slave_info_tbl {
	uint8_t pt_sn[WIRELESS_MASTER_NODE_MAX][DEV_SN_MODE_LEN];
	struct wl_netcfg_param_info_st wl_param[WIRELESS_MASTER_NODE_MAX];

	/*
	 * bit0 -- 是否已完成无线半自动组网, WLM_485S_FLAGS_HAD_FINISH_WL_NETCFG_BIT
	 * */
	rt_uint8_t	flags;
	rt_uint8_t	reserve1;
	rt_uint16_t	reserve2;
};
#endif

/*
 * 该数据结构用于'主电表采集器'根据pt、ct采集器的sn，查找应该将采集数据发送给那个'从电表采集器'
 * */
struct slave_emm_collector_info_st {
	/* 用于主电表采集器使用tcp分发pt、ct数据给其他从电表采集器 */
	struct in_addr	emm_ip;
	rt_uint16_t	emm_port;
	rt_uint16_t	reserve;

	/* 电表采集器sn */
	/* char emm_sn[DEV_SN_MODE_LEN]; 使用ip做为主键*/
	char ptc_sn[NUM_OF_PT_OF_COLLECT_EM_MAX][DEV_SN_MODE_LEN];
	char ctc_sn[NUM_OF_CT_OF_COLLECT_EM_MAX][DEV_SN_MODE_LEN];
};
#endif /* EM_ALL_TYPE_BASE */

#if EM_ALL_TYPE_BASE
/*
 * baudrate:
 * databits: 这里时严格意义上的数据位，不包含校验位(stm32的uart的databits包含校验位)
 * paritybit: 0 -- No parity, 1 -- Even parity, 2 -- Odd parity
 * stopbits: 1 -- 1, 15 -- 1.5, 2 -- 2
 *
 * #define UART_PAR_NONE    0  // No parity
 * #define UART_PAR_EVEN    1  // Even parity
 * #define UART_PAR_ODD     2  // Odd parity
 * */
struct uart_485_param
{
	rt_uint32_t baudrate;
	rt_uint8_t 	databits;
	rt_uint8_t 	paritybit;
	rt_uint8_t 	stopbits;
	rt_uint8_t 	resv;
};

/* em是否连接485sw */
enum connect_485sw_status {
	EM_NOT_CONNECT_485SW,
	EM_CONNECT_485SW
};

/*
 * 该数据结构用于记录一个电表采集器，采集了那些电表信息,以及与该电表对应的pt、ct采集器的sn
 * */
struct electric_meter_reg_info_st {
	/* 是电能表sn，而不是电表采集器sn */
	char em_sn[NUM_OF_COLLECT_EM_MAX][DEV_SN_BUF_STRING_WITH_NUL_LEN_MAX];
	char ptc_sn[NUM_OF_COLLECT_EM_MAX][DEV_SN_BUF_STRING_WITH_NUL_LEN_MAX];
	char ctc_sn[NUM_OF_COLLECT_EM_MAX][DEV_SN_BUF_STRING_WITH_NUL_LEN_MAX];
	char ctc1_sn[NUM_OF_COLLECT_EM_MAX][DEV_SN_BUF_STRING_WITH_NUL_LEN_MAX]; /* 一条线路可能对应两个ctc */
	char vport_no[NUM_OF_COLLECT_EM_MAX];
	enum ammeter_protocal_e em_proto[NUM_OF_COLLECT_EM_MAX];				/* 电表所用规约 */
	struct uart_485_param usart_param[NUM_OF_COLLECT_EM_MAX];				/* 电表485所用波特率*/
	enum connect_485sw_status connect_485sw_status;						/* em是否连接485sw */
};

/* 这个结构体需要考虑使用空间!!!! mark by David */
struct celectric_meter_config_info_st {
	//struct ammeter_time em_timing;	/* calibrate time YYMMDDhhmmss */
	rt_uint8_t em_timing[12];	/* calibrate time YYMMDDhhmmss */
	u8 pad[2];
	rt_uint8_t meter_connect_mode[NUM_OF_COLLECT_EM_MAX];
	rt_uint8_t calibrate_time_enable[NUM_OF_COLLECT_EM_MAX];
	rt_uint8_t moment_freeze_times[NUM_OF_COLLECT_EM_MAX];	/*momentary freeze times(1-3),for getting this times freeze data*/
	rt_uint8_t timing_freeze_times[NUM_OF_COLLECT_EM_MAX];	/*timing freeze times(1-12),for getting this times freeze data*/
	rt_uint8_t timing_freeze_time_set[NUM_OF_COLLECT_EM_MAX][8];
	rt_uint8_t timing_freeze_type[NUM_OF_COLLECT_EM_MAX];
	rt_uint8_t moment_freeze_enable[NUM_OF_COLLECT_EM_MAX];
	rt_uint8_t timing_freeze_enable[NUM_OF_COLLECT_EM_MAX];
	rt_uint8_t meter_protocal_type[NUM_OF_COLLECT_EM_MAX];
#if 1	
	rt_uint32_t volloss_event_total_times[NUM_OF_COLLECT_EM_MAX][6];
	rt_uint8_t volloss_event_times[NUM_OF_COLLECT_EM_MAX];
	rt_uint8_t volloss_event_source[NUM_OF_COLLECT_EM_MAX];
	rt_uint32_t volover_event_total_times[NUM_OF_COLLECT_EM_MAX][6];
	rt_uint8_t volover_event_times[NUM_OF_COLLECT_EM_MAX];
	rt_uint8_t volover_event_source[NUM_OF_COLLECT_EM_MAX];
	rt_uint32_t volunder_event_total_times[NUM_OF_COLLECT_EM_MAX][6];
	rt_uint8_t volunder_event_times[NUM_OF_COLLECT_EM_MAX];
	rt_uint8_t volunder_event_source[NUM_OF_COLLECT_EM_MAX];
	rt_uint32_t phasebreak_event_total_times[NUM_OF_COLLECT_EM_MAX][6];
	rt_uint8_t phasebreak_event_times[NUM_OF_COLLECT_EM_MAX];
	rt_uint8_t phasebreak_event_source[NUM_OF_COLLECT_EM_MAX];
	rt_uint32_t curloss_event_total_times[NUM_OF_COLLECT_EM_MAX][6];
	rt_uint8_t curloss_event_times[NUM_OF_COLLECT_EM_MAX];
	rt_uint8_t curloss_event_source[NUM_OF_COLLECT_EM_MAX];
	rt_uint32_t curover_event_total_times[NUM_OF_COLLECT_EM_MAX][6];
	rt_uint8_t curover_event_times[NUM_OF_COLLECT_EM_MAX];
	rt_uint8_t curover_event_source[NUM_OF_COLLECT_EM_MAX];
	rt_uint32_t curbreak_event_total_times[NUM_OF_COLLECT_EM_MAX][6];
	rt_uint8_t curbreak_event_times[NUM_OF_COLLECT_EM_MAX];
	rt_uint8_t curbreak_event_source[NUM_OF_COLLECT_EM_MAX];
	rt_uint32_t meterclear_event_total_times[NUM_OF_COLLECT_EM_MAX];
	rt_uint8_t meterclear_event_times[NUM_OF_COLLECT_EM_MAX];
	rt_uint32_t demandclear_event_total_times[NUM_OF_COLLECT_EM_MAX];
	rt_uint8_t demandclear_event_times[NUM_OF_COLLECT_EM_MAX];
	rt_uint32_t program_event_total_times[NUM_OF_COLLECT_EM_MAX];
	rt_uint8_t program_event_times[NUM_OF_COLLECT_EM_MAX];
	rt_uint32_t calibratetime_event_total_times[NUM_OF_COLLECT_EM_MAX];
	rt_uint8_t calibratetime_event_times[NUM_OF_COLLECT_EM_MAX];
	rt_uint32_t rseqvol_event_total_times[NUM_OF_COLLECT_EM_MAX][2];
	rt_uint8_t rseqvol_event_times[NUM_OF_COLLECT_EM_MAX];
	rt_uint32_t rseqcur_event_total_times[NUM_OF_COLLECT_EM_MAX][2];
	rt_uint8_t rseqcur_event_times[NUM_OF_COLLECT_EM_MAX];
#endif
};
#endif /* EM_ALL_TYPE_BASE */


struct syscfgdata_tbl_prev {
	struct tab_head 	 systbl_head;
	struct m3_sys_info_st 	 m3_sys_info;	/* !!NOTE:该结构体相对与表格的起始位置不能改变, 远程升级需要使用 */
	struct login_param 	 logindata[USR_NUM_MAX];
#if RT_USING_TCPIP_STACK
	struct ip_param 	 ipcfg[IP_INTERFACE_NUM];
#endif
	struct rs232_param 	 rs232cfg[RS232_INTERFACE_NUM];
	struct temp_rh_limen 	 t_rh_value;
#if RT_USING_TCPIP_STACK
	struct nms_if_info_st 	 nms_if;
	struct epon_device_st 	 epon_dev;
	struct snmp_community_st snmp_community;
#endif
	struct touch_calib_param touch_param;
	struct misc_byte_info_st misc_byteinfo;
#if RT_USING_GPRS
	struct gprs_if_info_st 	 gprs_info;
#endif

	struct tinywireless_if_info_st tw_param;
//#if PT_DEVICE || CT_DEVICE
	struct wireless_register_white_st white_register[WIRELESS_SLAVE_NODE_MAX];
//#endif
	struct gateway_em_st	 gw_em_info;	/* 关口表相关信息 */

#if EM_ALL_TYPE_BASE
#if 0
	/* 仅用于测试数据库升级 */
	struct wl_master_485_slave_info_tbl_prev wl_master_info;
#else
	struct wl_master_485_slave_info_tbl wl_master_info;
#endif
	struct slave_emm_collector_info_st s_emm_info[NUM_OF_SLAVE_EMM_MAX];
	struct electric_meter_reg_info_st em_reg_info;
	struct celectric_meter_config_info_st em_config_info;
#endif
};

struct syscfgdata_tbl {
	struct tab_head 	 systbl_head;
	struct m3_sys_info_st 	 m3_sys_info;	/* !!NOTE:该结构体相对与表格的起始位置不能改变, 远程升级需要使用 */
	struct login_param 	 logindata[USR_NUM_MAX];
#if RT_USING_TCPIP_STACK
	struct ip_param 	 ipcfg[IP_INTERFACE_NUM];
#endif
	struct rs232_param 	 rs232cfg[RS232_INTERFACE_NUM];
	struct temp_rh_limen 	 t_rh_value;
#if RT_USING_TCPIP_STACK
	struct nms_if_info_st 	 nms_if;
	struct epon_device_st 	 epon_dev;
	struct snmp_community_st snmp_community;
#endif
	struct touch_calib_param touch_param;
	struct misc_byte_info_st misc_byteinfo;
#if RT_USING_GPRS
	struct gprs_if_info_st 	 gprs_info;
#endif

	struct tinywireless_if_info_st tw_param;
//#if PT_DEVICE || CT_DEVICE
	struct wireless_register_white_st white_register[WIRELESS_SLAVE_NODE_MAX];
//#endif
	struct gateway_em_st	 gw_em_info;	/* 关口表相关信息 */

#if EM_ALL_TYPE_BASE
	struct wl_master_485_slave_info_tbl wl_master_info;
	struct slave_emm_collector_info_st s_emm_info[NUM_OF_SLAVE_EMM_MAX];
	struct electric_meter_reg_info_st em_reg_info;
	struct celectric_meter_config_info_st em_config_info;
#endif

};

struct syscfgdata_tbl_cache_st {
	volatile rt_uint32_t systbl_flag_set;
	struct syscfgdata_tbl syscfg_data;
};

enum syscfgdata_subtbl_ind {
	SYSCFGDATA_TBL_SYS_INFO	= 0,
	SYSCFGDATA_TBL_LOGINDATA,
#if RT_USING_TCPIP_STACK
	SYSCFGDATA_TBL_IPCFG,
#endif
	SYSCFGDATA_TBL_RS232CFG,
	SYSCFGDATA_TBL_TMP_RH,
#if RT_USING_TCPIP_STACK
	SYSCFGDATA_TBL_NMS_IF,
	SYSCFGDATA_TBL_EPON_DEV,
	SYSCFGDATA_TBL_SNMP_COMMU,
#endif
	SYSCFGDATA_TBL_TOUCH_PARAM,
	SYSCFGDATA_TBL_MISC_BYTE_INFO,

#if RT_USING_GPRS
	SYSCFGDATA_TBL_GPRSCFG,
#endif

	SYSCFGDATA_TBL_TW_INFO,
//#if PT_DEVICE || CT_DEVICE
	SYSCFGDATA_TBL_REGISTER_WHITER,
//#endif
	SYSCFGDATA_TBL_GW_EM_INFO,

#if EM_ALL_TYPE_BASE
	SYSCFGDATA_TBL_WLM_485S_INFO,
	SYSCFGDATA_TBL_S_EMM_INFO,
	SYSCFGDATA_TBL_EM_REG_INFO,
	SYSCFGDATA_TBL_EM_CONFIG_INFO,
#endif
	SYSCFGDATA_TBL_BUTT
};


extern struct hsdc_data_buffer_st  *hsdc_buffer;
extern struct login_param login_param_name[USR_NUM_MAX];

extern int init_syscfgdata_tbl(void);
extern int read_syscfgdata_tbl_from_cache(unsigned int ind_m, unsigned int ind_s, void *data);
extern int read_syscfgdata_tbl(unsigned int ind_m, unsigned int ind_s, void *data);
extern int write_syscfgdata_tbl(const unsigned int ind_m, const unsigned int ind_s, void *const data);
extern int restore_default_syscfgdata(void);

extern void syscfgdata_syn_proc(void);

extern int do_set_rs232cfg(struct rs232_param *rs232cfg, int intno);
#if EM_ALL_TYPE_BASE
extern int get_em_reg_info(struct electric_meter_reg_info_st *em_sns);
#endif


struct poweroff_info_st {
	rt_uint32_t poi_magic;

	rt_uint16_t poi_tx_poweroff_cnt;
	rt_uint16_t poi_rx_poweroff_cnt;

	rt_uint32_t poi_tx_poweroff_t0;
	rt_uint32_t poi_tx_poweroff_t1;
	rt_uint32_t poi_tx_poweroff_t2;

	rt_uint32_t poi_rx_poweroff_t0;
	rt_uint32_t poi_rx_poweroff_t1;
	rt_uint32_t poi_rx_poweroff_t2;
};

extern int write_whole_poweroff_info_tbl(struct poweroff_info_st *data);
extern int read_whole_poweroff_info_tbl(struct poweroff_info_st *data);
extern int init_poweroff_info_tbl(struct poweroff_info_st *data);

#endif
