/*
 ******************************************************************************
 * debug_sw.c
 *
 *  Created on: 2014-10-16
 *      Author: David, zhaoshaowei@yeejoin.com
 *
 * COPYRIGHT (C) 2014, YEEJOIN (Beijing) Technology Development Co., Ltd.
 ******************************************************************************
 */

#include <debug_sw.h>
#include <finsh.h>
#include <misc_lib.h>


volatile unsigned char dsw_module[DMN_NUM];
volatile unsigned long dsw_sub_module[DMN_NUM];


static void set_sub_module_sw(enum dsw_module_no_e m, int sm, int on_off, volatile unsigned long *p);
static void turnoff_all_debug(void);

void dsw_set(enum dsw_module_no_e m, int sm, int on_off)
{
	if (255==m && 255==sm && 0==on_off) {
		turnoff_all_debug();
		return;
	}

	if (m<0 || m>=DMN_NUM || sm<0 || sm>31) {
		printf_syn("module no(%d) or sub-module no (%d) is invalid\n", m, sm);
		return;
	}

	switch (m) {
	case DMN_485SW:
		if (sm < DSMN_485SW_NUM) {
			if (sm == DSMN_485SW_ALL_SUM_MOD) {
				dsw_module[DMN_485SW] = 1;
				dsw_sub_module[DMN_485SW] = ~0;
			} else {
				set_sub_module_sw(m, sm, on_off, &dsw_sub_module[DMN_485SW]);
			}
		} else {
			printf_syn("485sw module's sub module no(%d) is invalid\n", sm);
		}
		break;

	case DMN_METER_PROTO:	/* 用于电表通信协议模块, meter communication protocol */
		if (sm < DSMN_MP_NUM) {
			set_sub_module_sw(m, sm, on_off, &dsw_sub_module[DMN_METER_PROTO]);
		} else {
			printf_syn("meter communication protocol module's sub module no(%d) is invalid\n", sm);
		}
		break;

	default:
		/* enum dsw_module_no_e是连续的不应该出现这种情况 */
		printf_syn("module no(%d) is invalid\n", m);
		break;
	}

	return;
}

FINSH_FUNCTION_EXPORT(dsw_set, set debug_info_switch);


/*
 * 该函数仅在该文件中使用, 认为传递的入口参数都是合法的
 * */
static void set_sub_module_sw(enum dsw_module_no_e m, int sm, int on_off, volatile unsigned long *p)
{
	unsigned long  temp;

	temp = *p;
	if (on_off) {
		set_bit(temp, 1<<sm);
	} else {
		clr_bit(temp, 1<<sm);
	}
	*p = temp;

	if (0 == temp)
		dsw_module[m] = 0;
	else
		dsw_module[m] = 1;

	return;
}

static void turnoff_all_debug(void)
{
	int i;

	for (i=0; i<DMN_NUM; ++i) {
		dsw_module[i]	  = 0;
		dsw_sub_module[i] = 0;
	}

	printf_syn("had turnoff all debug switch\n");

	return;
}
