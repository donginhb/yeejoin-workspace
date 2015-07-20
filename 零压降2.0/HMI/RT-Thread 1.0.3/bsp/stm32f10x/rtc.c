/*
 * File      : rtc.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version.
 * 2011-11-26     aozima       implementation time.
 */

#include <rtthread.h>
#include <stm32f10x.h>
#include "rtc.h"

#include <board.h>
#include <misc_lib.h>

#include <syscfgdata.h>


static struct rt_device rtc;

static char rtc_ppm[128];
static unsigned short rtc_secinmonth[128];

#define set_rtc_cali_param set_rtc_cali

int set_rtc_cali(unsigned char rtc_cali);

static rt_err_t rt_rtc_open(rt_device_t dev, rt_uint16_t oflag)
{
	if (dev->rx_indicate != RT_NULL) {
		/* Open Interrupt */
	}

	return RT_EOK;
}

static rt_size_t rt_rtc_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
	return 0;
}

static rt_err_t rt_rtc_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
	rt_time_t *time;
	unsigned char calivalue;

	RT_ASSERT(dev != RT_NULL);

	switch (cmd) {
	case RT_DEVICE_CTRL_RTC_GET_TIME:
		time = (rt_time_t *)args;
		/* read device */
		*time = RTC_GetCounter();
		break;

	case RT_DEVICE_CTRL_RTC_SET_TIME:
		time = (rt_time_t *)args;
		/* Enable PWR and BKP clocks */
		//RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

		/* Allow access to BKP Domain */
		PWR_BackupAccessCmd(ENABLE);

		/* Wait until last write operation on RTC registers has finished */
		RTC_WaitForLastTask();

		/* Change the current time */
		RTC_SetCounter(*time);

		/* Wait until last write operation on RTC registers has finished */
		RTC_WaitForLastTask();

		BKP_WriteBackupRegister(RTC_SET_FLAG_BKP16BITS, 0xA5A5);
		PWR_BackupAccessCmd(DISABLE);

		break;

	case RT_DEVICE_CTRL_RTC_CALI_SET:
		/*calibration clock default*/
		if (RT_EOK == get_rtc_cali_param(&calivalue)) {
			if (RTC_CALIBRATION_DEF_VALUE == calivalue) {
				rt_kprintf("rtc not cfg cali val\n");
			} else if (calivalue<=0x7f) {
				PWR_BackupAccessCmd(ENABLE);
				BKP_SetRTCCalibrationValue(calivalue);
				PWR_BackupAccessCmd(DISABLE);
			} else
				rt_kprintf("rtc calivalue(%d) error\n", calivalue);


		} else {
			rt_kprintf("get rtc cali value fail\n");
		}

		break;

	default:
		break;
	}

	return RT_EOK;
}

/*******************************************************************************
* Function Name  : RTC_Configuration
* Description    : Configures the RTC.
* Input          : None
* Output         : None
* Return         : 0 reday,-1 error.
*******************************************************************************/
int RTC_Configuration(void)
{
	u32 count=0x200000;

	/* Enable PWR and BKP clocks */
	//RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

	/* Allow access to BKP Domain */
	PWR_BackupAccessCmd(ENABLE);

	/* Reset Backup Domain */
	BKP_DeInit();

	/* Enable LSE */
	RCC_LSEConfig(RCC_LSE_ON);
	/* Wait till LSE is ready */
	while ( (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) && (--count) );
	if ( count == 0 ) {
		return -1;
	}

	/* Select LSE as RTC Clock Source */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

	/* Enable RTC Clock */
	RCC_RTCCLKCmd(ENABLE);

	/* Wait for RTC registers synchronization */
	RTC_WaitForSynchro();

	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();

	/* Set RTC prescaler: set RTC period to 1sec */
	RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */

	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();

	RTC_ITConfig(RTC_IT_SEC, ENABLE); // 使能RTC秒中断
	RTC_WaitForLastTask();

	PWR_BackupAccessCmd(DISABLE);

	return 0;
}

void rt_hw_rtc_init(void)
{
	rtc.type	= RT_Device_Class_RTC;

	if (BKP_ReadBackupRegister(RTC_SET_FLAG_BKP16BITS) != 0xA5A5) {
		rt_kprintf("rtc is not configured\n");
		rt_kprintf("please configure with set_date and set_time\n");
		if ( RTC_Configuration() != 0) {
			rt_kprintf("rtc configure fail...\r\n");
			return ;
		}
	} else {
		/* Wait for RTC registers synchronization */
		RTC_WaitForSynchro();

		/* Wait until last write operation on RTC registers has finished */
		RTC_WaitForLastTask();
		RTC_ITConfig(RTC_IT_SEC, ENABLE); // 使能RTC秒中断
		RTC_WaitForLastTask();
	}

	/* register rtc device */
	rtc.init 	= RT_NULL;
	rtc.open 	= rt_rtc_open;
	rtc.close	= RT_NULL;
	rtc.read 	= rt_rtc_read;
	rtc.write	= RT_NULL;
	rtc.control = rt_rtc_control;

	/* no private */
	rtc.user_data = RT_NULL;

	rt_device_register(&rtc, "rtc", RT_DEVICE_FLAG_RDWR);

	return;
}

#include <time.h>
#if defined (__IAR_SYSTEMS_ICC__) &&  (__VER__) >= 6020000   /* for IAR 6.2 later Compiler */
#pragma module_name = "?time"
time_t (__time32)(time_t *t)                                 /* Only supports 32-bit timestamp */
#else
time_t time(time_t* t)
#endif
{
	rt_device_t device;
	time_t time=0;

	device = rt_device_find("rtc");
	if (device != RT_NULL) {
		rt_device_control(device, RT_DEVICE_CTRL_RTC_GET_TIME, &time);
		if (t != RT_NULL) *t = time;
	}

	return time;
}

#ifdef RT_USING_FINSH
#include <finsh.h>

void set_date(rt_uint32_t year, rt_uint32_t month, rt_uint32_t day)
{
	time_t now;
	struct tm* ti;
	rt_device_t device;

	ti = RT_NULL;
	/* get current time */
	time(&now);

	ti = localtime(&now);
	if (ti != RT_NULL) {
		ti->tm_year = year - 1900;
		ti->tm_mon 	= month - 1; /* ti->tm_mon 	= month; 0~11 */
		ti->tm_mday = day;
	}

	now = mktime(ti);

	device = rt_device_find("rtc");
	if (device != RT_NULL) {
		rt_rtc_control(device, RT_DEVICE_CTRL_RTC_SET_TIME, &now);
	}
}
FINSH_FUNCTION_EXPORT(set_date, set date. e.g: set_date(2010,2,28))

void set_time(rt_uint32_t hour, rt_uint32_t minute, rt_uint32_t second)
{
	time_t now;
	struct tm* ti;
	rt_device_t device;

	ti = RT_NULL;
	/* get current time */
	time(&now);

	ti = localtime(&now);
	if (ti != RT_NULL) {
		ti->tm_hour = hour;
		ti->tm_min 	= minute;
		ti->tm_sec 	= second;
	}

	now = mktime(ti);
	device = rt_device_find("rtc");
	if (device != RT_NULL) {
		rt_rtc_control(device, RT_DEVICE_CTRL_RTC_SET_TIME, &now);
	}
}
FINSH_FUNCTION_EXPORT(set_time, set time. e.g: set_time(23,59,59))

void list_date(void)
{
	time_t now;

	time(&now);
	rt_kprintf("%s\n", ctime(&now));
}
FINSH_FUNCTION_EXPORT(list_date, show date and time.)

/* 配置是否使能64分频引脚 */
void cfg_rtc_pin(int enable)
{
	if (1 == enable) {
		BKP_TamperPinCmd(ENABLE);
		BKP_RTCOutputConfig(BKP_RTCOutputSource_CalibClock);
	} else {
		BKP_TamperPinCmd(DISABLE);
		BKP_RTCOutputConfig(BKP_RTCOutputSource_None);
	}

}
FINSH_FUNCTION_EXPORT(cfg_rtc_pin, 1-enable 0-disable)


#define STAND_CALI_F_OUT (32768/64)

#define STD_MIN_VAL 100000
/* 通过测量值校准rtc */
void cali_rtc_by_clk(int std_f, int measure_f)
{
	int Deviation = 0;
	int CalibStep = 0;
	int i;

	if((std_f<STD_MIN_VAL) || (measure_f<STD_MIN_VAL) || ((measure_f-std_f) < 0)) {
		printf_syn("give value error, std_f:%d, measure_f:%d\n",std_f,measure_f);
		return;
	}

	Deviation = (measure_f-std_f)*1000000/std_f;
	CalibStep = Deviation;

	if(CalibStep < (Deviation*1000 + 500)/1000)
		CalibStep += 1;  // 四舍五入

	if(CalibStep > 127)
		CalibStep = 127; // 校准值应在0—127之间

	for (i=0; i<sizeof(rtc_ppm); ++i) {
		if (rtc_ppm[i] == CalibStep) {
			break;
		}
	}

	if (i<sizeof(rtc_ppm)) {

		set_rtc_cali_param(i);

		if (rt_rtc_control != rtc.control) {
			rtc.control(&rtc, RT_DEVICE_CTRL_RTC_CALI_SET, NULL);
		} else {
			rt_kprintf("no rtc device fail\n");
		}
		//BKP_SetRTCCalibrationValue(i);
	} else {
		printf_syn("give value error, std_f:%d, measure_f:%d\n,",std_f,measure_f);
	}

}
FINSH_FUNCTION_EXPORT(cali_rtc_by_clk, std_f measure_f)

/* 通过时差校准rtc */
void cali_rtc_by_sec(int diff_sec_per_month)
{
	int Deviation = 0;
	int i,diff;

	if((diff_sec_per_month > 314) || (diff_sec_per_month < 0)) {
		printf_syn("give different sec per month value error, diff_sec_per_month:%d\n",diff_sec_per_month);
		return;
	}

	diff = 100;
	for (i=0; i<sizeof(rtc_secinmonth); ++i) {
		Deviation = sub_abs(diff_sec_per_month,rtc_secinmonth[i]);

		if (Deviation < diff) {
			diff = Deviation;
			if ((0 == Deviation) || (1 == Deviation)) {
				set_rtc_cali_param(i);

				if (rt_rtc_control != rtc.control) {
					rtc.control(&rtc, RT_DEVICE_CTRL_RTC_CALI_SET, NULL);
				} else {
					rt_kprintf("no rtc device fail\n");
				}
				//BKP_SetRTCCalibrationValue(i);
				break;
			}
		}
	}
}
FINSH_FUNCTION_EXPORT(cali_rtc_by_sec, diff_sec_per_month)


int get_rtc_cali_param(unsigned char *rtc_cali)
{
	int ret;
	struct misc_byte_info_st misc_byteinfo;

	if (NULL == rtc_cali)
		return RT_ERROR;

	ret = read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_MISC_BYTE_INFO, 0, &misc_byteinfo);
	if (RT_EOK == ret) {
		*rtc_cali = misc_byteinfo.rtc_cali;
	} else {
		printf_syn("get rtc cali error\n");
	}

	return ret;
}

//int set_rtc_cali_param(unsigned char rtc_cali)
int set_rtc_cali(unsigned char rtc_cali)
{
	int ret;
	struct misc_byte_info_st misc_byteinfo;

	if (rtc_cali<1 || rtc_cali>127) {
		printf_syn("rtc cali(%u) param error\n", rtc_cali);
		return RT_ERROR;
	}

	ret = read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_MISC_BYTE_INFO, 0, &misc_byteinfo);
	if (RT_EOK == ret) {
		misc_byteinfo.rtc_cali = rtc_cali;
		ret = write_syscfgdata_tbl(SYSCFGDATA_TBL_MISC_BYTE_INFO, 0, &misc_byteinfo);
		syscfgdata_syn_proc();
	} else {
		printf_syn("get rtc cali error\n");
	}

	return ret;
}
FINSH_FUNCTION_EXPORT(set_rtc_cali, rtc_cali)

#endif


/* Calibration value -- Value in ppm rounded to the nearest ppm */
static char rtc_ppm[128] = {
	0  ,
	1  ,
	2  ,
	3  ,
	4  ,
	5  ,
	6  ,
	7  ,
	8  ,
	9  ,
	10 ,
	10 ,
	11 ,
	12 ,
	13 ,
	14 ,
	15 ,
	16 ,
	17 ,
	18 ,
	19 ,
	20 ,
	21 ,
	22 ,
	23 ,
	24 ,
	25 ,
	26 ,
	27 ,
	28 ,
	29 ,
	30 ,
	31 ,
	31 ,
	32 ,
	33 ,
	34 ,
	35 ,
	36 ,
	37 ,
	38 ,
	39 ,
	40 ,
	41 ,
	42 ,
	43 ,
	44 ,
	45 ,
	46 ,
	47 ,
	48 ,
	49 ,
	50 ,
	51 ,
	51 ,
	52 ,
	53 ,
	54 ,
	55 ,
	56 ,
	57 ,
	58 ,
	59 ,
	60 ,
	61 ,
	62 ,
	63 ,
	64 ,
	65 ,
	66 ,
	67 ,
	68 ,
	69 ,
	70 ,
	71 ,
	72 ,
	72 ,
	73 ,
	74 ,
	75 ,
	76 ,
	77 ,
	78 ,
	79 ,
	80 ,
	81 ,
	82 ,
	83 ,
	84 ,
	85 ,
	86 ,
	87 ,
	88 ,
	89 ,
	90 ,
	91 ,
	92 ,
	93 ,
	93 ,
	94 ,
	95 ,
	96 ,
	97 ,
	98 ,
	99 ,
	100,
	101,
	102,
	103,
	104,
	105,
	106,
	107,
	108,
	109,
	110,
	111,
	112,
	113,
	113,
	114,
	115,
	116,
	117,
	118,
	119,
	120,
	121
};


/* Calibration value -- Value in seconds per month (30 days) rounded to the nearest second */
static unsigned short rtc_secinmonth[128] = {
	0  ,
	2  ,
	5  ,
	7  ,
	10 ,
	12 ,
	15 ,
	17 ,
	20 ,
	22 ,
	25 ,
	27 ,
	30 ,
	32 ,
	35 ,
	37 ,
	40 ,
	42 ,
	44 ,
	47 ,
	49 ,
	52 ,
	54 ,
	57 ,
	59 ,
	62 ,
	64 ,
	67 ,
	69 ,
	72 ,
	74 ,
	77 ,
	79 ,
	82 ,
	84 ,
	87 ,
	89 ,
	91 ,
	94 ,
	96 ,
	99 ,
	101,
	104,
	106,
	109,
	111,
	114,
	116,
	119,
	121,
	124,
	126,
	129,
	131,
	133,
	136,
	138,
	141,
	143,
	146,
	148,
	151,
	153,
	156,
	158,
	161,
	163,
	166,
	168,
	171,
	173,
	176,
	178,
	180,
	183,
	185,
	188,
	190,
	193,
	195,
	198,
	200,
	203,
	205,
	208,
	210,
	213,
	215,
	218,
	220,
	222,
	225,
	227,
	230,
	232,
	235,
	237,
	240,
	242,
	245,
	247,
	250,
	252,
	255,
	257,
	260,
	262,
	264,
	267,
	269,
	272,
	274,
	277,
	279,
	282,
	284,
	287,
	289,
	292,
	294,
	297,
	299,
	302,
	304,
	307,
	309,
	311,
	314
};

