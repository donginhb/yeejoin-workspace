/*
 * 2012-01-13 16:15:45
 */
#ifndef _SYSCFGDATA_H_
#define _SYSCFGDATA_H_

#include <rtdef.h>
#include <syscfgdata-common.h>
#include <485sw.h>

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
#endif

#define RS232_INTERFACE_NUM 5
#define RS485_INTERFACE_NUM 2

#define TL16_UART_NUM	8

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
#endif

/*
 * enum tl16c554_parity_e {
	TCL16C554_PARITY_NONE	= 0,
	TCL16C554_PARITY_EVEN,	// 偶校验
	TCL16C554_PARITY_ODD	// 奇校验
};
 * */
struct uart_param {
	rt_uint32_t baudrate;

	rt_uint8_t databits;
	rt_uint8_t paritybit;
	rt_uint8_t stopbits;
	union {
		rt_uint8_t reserve[1];
		rt_uint8_t protoc;	/* enum ammeter_protocol_e */
	};
};

struct electric_meter_reg_info_st {
	/* 是电能表sn，而不是电表采集器sn */
	char em_sn[NUM_OF_COLLECT_EM_MAX][DEV_SN_BUF_STRING_WITH_NUL_LEN_MAX];
	enum ammeter_protocol_e protocal[NUM_OF_COLLECT_EM_MAX];
	char em_grp_no[NUM_OF_COLLECT_EM_MAX];
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

	struct uart_param tl16_uart[TL16_UART_NUM];

	struct electric_meter_reg_info_st em_reg_info;
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

	SYSCFGDATA_TBL_TL16_UART,

	SYSCFGDATA_TBL_EM_REG_INFO,

	SYSCFGDATA_TBL_BUTT
};


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
