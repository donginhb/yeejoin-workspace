/*
 * 2012-01-13 16:15:45
 */
#ifndef _SYSCFGDATA_H_
#define _SYSCFGDATA_H_

#include <rtdef.h>


struct rs232_param{
    rt_uint32_t baudrate;
    rt_uint8_t  databits;
    rt_uint8_t  paritybit;
    rt_uint8_t  stopbits;
};


#define USR_NAME_LEN_MAX    31
#define PW_LEN_MAX          31
struct usr_pw_pair{
    char usr[USR_NAME_LEN_MAX+1];
    char pw[PW_LEN_MAX+1];
};

#define USR_NUM_MAX 1
struct login_param{
    struct usr_pw_pair login;
};


/* data had modify */
#define SYSTBL_FLAG_DATA_DIRTY		0x01
/* write data to flash immediately */
#define SYSTBL_FLAG_WRITE_THROUGH 	0x02

struct tab_head {
	rt_uint32_t magic_num; /* NOTE: must place at the first word of table */
};

#define DEV_SN_LEN_MAX 23
/* !!NOTE: boot 中有用 */
struct m3_sys_info_st {
	/* rt_uint32_t hw_ver; */	/* 根据I/O口拨码开关确定硬件版本号 */
	rt_uint32_t sw_ver;		/* 用于远程升级版本号的判断 */
	rt_int32_t  sw_file_mtime;	/* the mtime of os-app rbin-file */
	rt_uint32_t db_ver;		/* 数据库版本 */
	char dev_sn[DEV_SN_LEN_MAX+1];	/* 设备序列号 */
	char match_sn[2][DEV_SN_LEN_MAX+1];
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

#define RS232_INTERFACE_NUM 2
struct syscfgdata_tbl{
	struct tab_head systbl_head;
	struct m3_sys_info_st m3_sys_info;	/* !!NOTE:该结构体相对与表格的起始位置不能改变, 远程升级需要使用 */
	struct login_param logindata[USR_NUM_MAX];
	struct rs232_param rs232cfg[RS232_INTERFACE_NUM];
	struct temp_rh_limen t_rh_value;
	struct touch_calib_param touch_param;
	struct misc_byte_info_st misc_byteinfo;
};


struct syscfgdata_tbl_cache_st {
	volatile rt_uint32_t systbl_flag_set;
	struct syscfgdata_tbl syscfg_data;
};

enum syscfgdata_subtbl_ind{
    SYSCFGDATA_TBL_SYS_INFO	= 0,
    SYSCFGDATA_TBL_LOGINDATA,
    SYSCFGDATA_TBL_RS232CFG,
    SYSCFGDATA_TBL_TMP_RH,
    SYSCFGDATA_TBL_TOUCH_PARAM,
    SYSCFGDATA_TBL_MISC_BYTE_INFO,
    
    SYSCFGDATA_TBL_BUTT
};

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

extern struct login_param login_param_name[USR_NUM_MAX];

extern int init_syscfgdata_tbl(void);
extern int read_syscfgdata_tbl_from_cache(unsigned int ind_m, unsigned int ind_s, void *data);
extern int read_syscfgdata_tbl(unsigned int ind_m, unsigned int ind_s, void *data);
extern int write_syscfgdata_tbl(const unsigned int ind_m, const unsigned int ind_s, void *const data);
extern int restore_default_syscfgdata(void);

extern void syscfgdata_syn_proc(void);



struct poweroff_info_st {
	rt_uint32_t poi_magic;

	rt_uint16_t poi_tx1_poweroff_cnt;
	rt_uint16_t poi_tx2_poweroff_cnt;
	rt_uint16_t poi_rx_poweroff_cnt;

	rt_uint32_t poi_tx1_poweroff_t0;
	rt_uint32_t poi_tx1_poweroff_t1;
	rt_uint32_t poi_tx1_poweroff_t2;

	rt_uint32_t poi_tx2_poweroff_t0;
	rt_uint32_t poi_tx2_poweroff_t1;
	rt_uint32_t poi_tx2_poweroff_t2;

	rt_uint32_t poi_rx_poweroff_t0;
	rt_uint32_t poi_rx_poweroff_t1;
	rt_uint32_t poi_rx_poweroff_t2;
};

extern int write_whole_poweroff_info_tbl(struct poweroff_info_st *data);
extern int read_whole_poweroff_info_tbl(struct poweroff_info_st *data);
extern int init_poweroff_info_tbl(struct poweroff_info_st *data);

#endif
