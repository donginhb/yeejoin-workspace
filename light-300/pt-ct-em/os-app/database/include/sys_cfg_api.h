
#ifndef _SYS_CFG_API_H_
#define _SYS_CFG_API_H_

#include <rtdef.h>

#ifdef RT_USING_LWIP
#include <lwip/ip_addr.h>
#include <lwipusrlib.h>
#endif

#include <syscfgdata.h>
#include <sinkinfo_common.h>

/*
 * 定义数据结构
 */
struct uart_param {
	unsigned int  baudrate;
	unsigned char databits;
	unsigned char paritybit;
	unsigned char stopbits;
};


typedef unsigned char sys_cfg_err_t;

/*
 * 定义宏
 */
/* 串口相关宏定义 */
#define UART_PORT_NUM (3)
#define UART_DATA_BITS_MIN (5)
#define UART_DATA_BITS_MAX (8)

#define UART_STOP_BITS_MIN (1)
#define UART_STOP_BITS_MAX (2)

#define UART_PAR_NONE    0  // No parity
#define UART_PAR_EVEN    1  // Even parity
#define UART_PAR_ODD     2  // Odd parity
#define UART_PAR_ONE     3  // Parity bit is one
#define UART_PAR_ZERO    4  // Parity bit is zero


/*
 * 错误码
 */
enum sys_cfg_err {
	SYS_CFG_SUCC = 0,
	SYS_CFG_PARAM_ERR, /* 接口函数的参数错误 */
	SYS_CFG_DATA_ERR,  /* 设置的数据错误 */

	SYS_CFG_ERR_BUTT
};

extern void px_setout_handdebug(int id, int data, int chlx);
extern int is_usr_pw_matching(char *usr, char *pw);
extern int get_openlock_pw(char *lockpw, int buf_len);

//extern int do_set_rs232cfg(struct rs232_param *rs232cfg, int intno);
extern int get_devsn(char *str, int len);
extern int UART_if_Set( rt_uint32_t baut, rt_uint8_t datab, rt_uint8_t parityb, rt_uint8_t stopb, rt_uint8_t port);
extern int get_net_id(void);
extern int get_em_info(int emnum, int cmd, u32_t *value, u8_t*str);
extern int set_em_info(int emnum, int cmd, u32_t *value, u8_t *str);
extern int set_7880_adj(int id, int gain, int chlx, int old_reg, int old_output, int new_output);

#if PT_DEVICE || CT_DEVICE
extern int get_tw_syn_word(rt_uint16_t *synword);
#endif

//extern int get_pt_ct_sn(enum sinkinfo_dev_type_e dev_type, char *str, int len);

#if WIRELESS_MASTER_NODE
extern int prt_white(void);
extern int add_white(char *sn);
extern int del_white(char *sn);
extern int is_wl_white_list_empty(void);
#endif

extern void print_dev_sn(uint8_t *sn, int len);
extern int is_dev_sn_valid(uint8_t *sn, int len);
void rt_val_7880(void);
void get_st_from_485_to_cal_4432(void);

#if EM_MASTER_DEV || EM_MULTI_MASTER_DEV
extern int set_wl_netcfg_finish_flag(int is_finish);
extern int is_wl_netcfg_finish(void);
#endif

#if EM_ALL_TYPE_BASE
//extern int get_em_reg_info(struct electric_meter_reg_info_st *em_sns);
#endif

int mode7880_con(int id, int mc, int chlx);
int mode7880_mc(int id, int mc, int chlx);
int reg_em(int no, char *sn, int vport_no, char *ptc_sn, char *ctc_sn, char *ctc1_sn);



#endif
