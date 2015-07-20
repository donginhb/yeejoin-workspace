
#ifndef _SYS_CFG_API_H_
#define _SYS_CFG_API_H_

#include <rtdef.h>

#ifdef RT_USING_LWIP
#include <lwip/ip_addr.h>
#include <lwipusrlib.h>
#endif

#include <syscfgdata.h>

/*
 * 定义数据结构
 */

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


extern void print_dev_sn(uint8_t *sn, int len);
extern int is_dev_sn_valid(uint8_t *sn, int len);
extern int get_tl16_uart_param(int no, struct uart_param *uartcfg);
extern int get_em_reg_info(struct electric_meter_reg_info_st *em_sns);
extern int get_reg_em_grpno_proto(char *sn, enum ammeter_protocol_e *proto, char *grp_no);

#endif
