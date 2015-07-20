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


unsigned char dsw_module[DMN_NUM];
unsigned long dsw_sub_module[DMN_NUM];


static void set_sub_module_sw(enum dsw_module_no_e m, int sm, int on_off, unsigned long *p);
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
	case DMN_SINKINFO:
		if (sm < DSMN_SI_NUM) {
			set_sub_module_sw(m, sm, on_off, &dsw_sub_module[DMN_SINKINFO]);
		} else {
			printf_syn("sinkinfo module's sub module no(%d) is invalid\n", sm);
		}
		break;

	case DMN_ADE7880:		/* 用于7880模块 */
		if (sm < DSMN_ADE7880_NUM) {
			set_sub_module_sw(m, sm, on_off, &dsw_sub_module[DMN_ADE7880]);
		} else {
			printf_syn("ADE7880 module's sub module no(%d) is invalid\n", sm);
		}
		break;

	case DMN_SI4432:		/* 用于4432模块 */
		if (sm < DSMN_SI4432_NUM) {
			set_sub_module_sw(m, sm, on_off, &dsw_sub_module[DMN_SI4432]);
		} else {
			printf_syn("SI4432 module's sub module no(%d) is invalid\n", sm);
		}
		break;

	case DMN_METER_PROTO:	/* 用于电表通信协议模块, meter communication protocol */
		if (sm < DSMN_MP_NUM) {
			set_sub_module_sw(m, sm, on_off, &dsw_sub_module[DMN_METER_PROTO]);
		} else {
			printf_syn("meter communication protocol module's sub module no(%d) is invalid\n", sm);
		}
		break;

	case DMN_PRIV_MIB:		/* 用于私有mib模块 */
		if (sm < DSMN_PM_NUM) {
			set_sub_module_sw(m, sm, on_off, &dsw_sub_module[DMN_PRIV_MIB]);
		} else {
			printf_syn("priv mib module's sub module no(%d) is invalid\n", sm);
		}
		break;

	case DMN_TRANS_PTCT_DATA:	/* 用于传输ptc、ctc采集数据的模块 */
		if (sm < DSMN_TPD_NUM) {
			set_sub_module_sw(m, sm, on_off, &dsw_sub_module[DMN_TRANS_PTCT_DATA]);
		} else {
			printf_syn("trans ptct data module's sub module no(%d) is invalid\n", sm);
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
static void set_sub_module_sw(enum dsw_module_no_e m, int sm, int on_off, unsigned long *p)
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
