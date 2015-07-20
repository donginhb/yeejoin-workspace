/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 */

/**
 * @addtogroup STM32
 */
/*@{*/

#include <board.h>
#include <rtthread.h>
#include <finsh.h>
#include <syscfgdata.h>

#ifdef RT_USING_DFS
/* dfs init */
#include <dfs_init.h>
/* dfs filesystem:ELM filesystem init */
#include <dfs_elm.h>
/* dfs Filesystem APIs */
#include <dfs_fs.h>
#endif

#ifdef RT_USING_LWIP
#include <lwip/sys.h>
#include <lwip/api.h>
#include <netif/ethernetif.h>
#endif

#ifdef RT_USING_RTGUI
#include <rtgui/rtgui.h>
#include <rtgui/rtgui_server.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/driver.h>
#endif
#include <rxe_msg_proc.h>
#include <zvd_gui_app.h>

#include "sst25.h"
#include <info_tran.h>
#include <phasex.h>

#define USART3_ACK_TIMEOUT  200

extern unsigned char tx_dev_sn[DEV_SN_LEN_MAX+1];	/* 设备序列号 */


extern void sst25_set_prote_level_to_none(void);
extern void nvic_cfg_app(void);
extern int init_syscfgdata_tbl(void);

static rt_uint32_t misc_stack[512/4];
static struct rt_thread misc_thread;

extern volatile unsigned int is_system_powerdown;
extern volatile unsigned int sys_powerdown_delay4confirm;

static volatile int is_need_send_switch2pt_cmd_again;

extern struct rt_device uart3_device;
extern unsigned char info_tx_buf[64];
extern struct rt_semaphore uart3_tx_sem;



static void rt_sys_misc_entry(void* parameter)
{
	unsigned int cnt=0;

	while (1) {
		cnt++;
		rt_thread_delay(get_ticks_of_ms(250));

#if 1	/* 2Hz blink */
		if (is_fibre_recv_data_sysmisc())
			data_led_blink();
		else
			data_led_off();
#endif
#if 1
		switch (get_sys_use_channel()) {
		case SUC_PT_CHANNEL:	/* 0.5Hz */
			if (!(cnt & 0x03))
				running_led_blink();
			break;

		case SUC_FIBER_CHANNEL_1: /* 1Hz */
			if (cnt & 0x01)
				running_led_blink();
			break;

		case SUC_FIBER_CHANNEL_2: /* 2Hz */
			running_led_blink();
			break;

		default:
			printf_syn("func:%s(), invalid cru-use-channel(%d)\n ", __FUNCTION__, get_sys_use_channel());
			break;
		}
#endif


#if 1	/* 1Hz blink */
		if (cnt & 0x01) {
//			running_led_blink();

			if (0==is_px_bin_info_modifing) {
				if (is_sre_px_fault() || is_sre_px_vol_not_normal() || !is_fibre_recv_data_sysmisc()) {
					printf_syn("is_sre_px_fault:0x%x, is_sre_px_vol_not_normal:0x%x, is not recv fibre data:0x%x\n",
							   is_sre_px_fault(), is_sre_px_vol_not_normal(), is_fibre_recv_data_sysmisc());

					printf_syn("[HNL] [SE-PCPBPA.RE-PCPBPA]sre px normal:0x%x, sre px state:0x%x\n",
							   SRE_PX_VOL_NORMAL_BIT, (dev_px_vol_state_vec&SRE_PX_VOL_NORMAL_BIT));

					fault_led_blink();
				} else {
					fault_led_off();
				}
			}

			if (!disable_buzzer && buzzer_tweet)
				buzzer_rolling_over(buzzer_gpio, buzzer_pin);
			else
				buzzer_off(buzzer_gpio, buzzer_pin);
		}
#endif

		/* 256s(4.27min)同步一次syscfgdata_tbl_cache.syscfg_data */
		if (0!=cnt && (0 == (cnt & 0x3ff))) {
			flush_sfwb(0); /* mark by David */
			syscfgdata_syn_proc();
		}

		if (is_system_powerdown) {
			if ((rt_tick_get() - sys_powerdown_delay4confirm) >=100 ) {
				is_system_powerdown = 0;
			}
		}

		if (0 != is_need_send_switch2pt_cmd_again) {
			is_need_send_switch2pt_cmd_again = 0;
			send_cmd_to_rxe(0, SWITCH2PT);
			printf_syn("uart3_ack_timeout timer is timerout!\n");

		}

	} /* while(1) */

}

void rt_init_thread_entry(void* parameter)
{
	rt_device_t rtc_dev;

	/* init system data cfg table */
	init_syscfgdata_tbl();
	init_poweroff_info_tbl(&poweroff_info_data);

	check_poweroff_info_and_save2flash();

	/* Filesystem Initialization */
#ifdef RT_USING_DFS
	rt_hw_spiflash_init();
	/* init the device filesystem */
	dfs_init();
#ifdef RT_USING_DFS_ELMFAT
	/* init the elm chan FatFs filesystam*/
	elm_init();

	/* mount spi-flash fat partition 1 as root directory */
	if (dfs_mount("sf0", "/", "elm", 0, 0) == 0)
		rt_kprintf("File System initialized!\n");
	else
		rt_kprintf("File System initialzation failed!\n");
#endif

#endif

	/* LwIP Initialization */
#ifdef RT_USING_LWIP
	{
		extern void lwip_sys_init(void);

		/* register ethernetif device */
		eth_system_device_init();

#ifdef STM32F10X_CL
		rt_hw_stm32_eth_init();
#else
		/* STM32F103 */
#if STM32_ETH_IF == 0
		rt_hw_enc28j60_init();
#elif STM32_ETH_IF == 1
		rt_hw_dm9000_init();
#endif
#endif

		/* re-init device driver */
		rt_device_init_all();

		/* init lwip system */
		lwip_sys_init();
		rt_kprintf("TCP/IP initialized!\n");
	}
#endif

#ifdef RT_USING_RTGUI
	{
		extern void rtgui_startup();
		extern void rt_hw_lcd_init();
		extern void rtgui_touch_hw_init(void);

		rt_device_t lcd;

		/* init lcd */
		rt_hw_lcd_init();

		/* init touch panel */
		rtgui_touch_hw_init();

		/* re-init device driver */
		rt_device_init_all();

		/* find lcd device */
		lcd = rt_device_find("lcd");

		/* set lcd device as rtgui graphic driver */
		rtgui_graphic_set_device(lcd);

		/* startup rtgui */
		rtgui_startup();
	}
#endif /* #ifdef RT_USING_RTGUI */

	sst25_set_prote_level_to_none();

	buzzer_off(buzzer_gpio, buzzer_pin);

	rtc_dev = rt_device_find("rtc");
	if (NULL != rtc_dev) {
		rtc_dev->control(rtc_dev, RT_DEVICE_CTRL_RTC_CALI_SET, NULL);
	} else {
		rt_kprintf("find rtc device fail\n");
	}

	nvic_cfg_app();
#if USE_STM32_IWDG
	IWDG_Enable();
#endif
}



extern struct rt_semaphore print_sem;
extern struct rt_semaphore spiflash_sem;
extern struct rt_semaphore recv_rxe_msg_sem;
extern struct rt_semaphore uart3_tx_sem;
extern struct rt_timer uart3_ack_timer;
extern void uart3_ack_timeout(void* parameter);
int rt_application_init()
{
	rt_thread_t thread_h;
	rt_err_t result;
	int i;

	//sst25_set_prote_level_to_none();

	rt_sem_init(&print_sem, "prtsem", 1, RT_IPC_FLAG_PRIO);
	rt_sem_init(&spiflash_sem, "sfsem", 1, RT_IPC_FLAG_PRIO);
	rt_sem_init(&recv_rxe_msg_sem, "rrmsem", 0, RT_IPC_FLAG_PRIO);
	rt_sem_init(&uart3_tx_sem, "u3txsem", 1, RT_IPC_FLAG_PRIO);
	rt_timer_init(&uart3_ack_timer, 
			"u3at", 
			uart3_ack_timeout, 
			RT_NULL, 
			USART3_ACK_TIMEOUT,
			RT_TIMER_FLAG_PERIODIC);

	for (i=0; i<DEV_SN_MODE_LEN; ++i)
		tx_dev_sn[i] = *(DEV_SN_MODE+i);


	/* init led thread */
	result = rt_thread_init(&misc_thread, "misc", rt_sys_misc_entry, RT_NULL, (rt_uint8_t*)&misc_stack[0],
							sizeof(misc_stack), 0x19, 5);
	if (result == RT_EOK)
		rt_thread_startup(&misc_thread);

#if (RT_THREAD_PRIORITY_MAX == 32)
	thread_h = rt_thread_create("init", rt_init_thread_entry, RT_NULL, 2048, 8, 20);
#else
	thread_h = rt_thread_create("init", rt_init_thread_entry, RT_NULL, 2048, 80, 20);
#endif
	if (thread_h != RT_NULL)
		rt_thread_startup(thread_h);

	thread_h = rt_thread_create("rrmproc", rt_recv_rxe_msg_proc_entry, RT_NULL, 1024, 0x18, 12);
	if (thread_h != RT_NULL)
		rt_thread_startup(thread_h);

	return 0;
}


void uart3_ack_timeout(void* parameter)
{
	/* switch2pt */
	static unsigned int u3_ack_to_times = 0;

	u3_ack_to_times++;
	if (u3_ack_to_times >= 3) {
		u3_ack_to_times = 0;
		rt_timer_stop(&uart3_ack_timer);
		return;
	}

	is_need_send_switch2pt_cmd_again = 1;

	return;
}

#if 0
#include <sst25.h>
enum sst25_cmd {
	SST25_READ_ID = 1,
	SST25_READ_DATA = 2,
	SST25_WRITE_DATA = 3,
	SST25_READ_SR    = 4,
	SST25_SET_PROTE  = 5,
};
void sst25_test(int cmd, unsigned long addr, int len, int init_data)
{
	unsigned char buf[16];
	int i;

	if (len > sizeof(buf))
		len = sizeof(buf);
	printf_syn("cmd:%d, addr:0x%x, len:%d, init_data:%d\n", cmd, addr, len, init_data);
	switch (cmd) {
	case SST25_READ_ID:
		sst25_read_jedec_id(buf, sizeof(buf));
		printf_syn("0x%x, 0x%x, 0x%x\n", buf[0], buf[1], buf[2]);
		break;

	case SST25_READ_DATA:
		sst25_read_data(addr, buf, sizeof(buf), 0);
		for (i=0; i<len; i+=4)
			printf_syn("[i:%d]0x%x, 0x%x, 0x%x, 0x%x\n", i, buf[i], buf[i+1], buf[i+2], buf[i+3]);

		break;
#if 0
	case SST25_WRITE_DATA:
		for (i=0; i<len; ++i)
			buf[i] = init_data + i;
		sst25_write_data_by_sector(addr, buf, len);
		break;
#endif
	case SST25_READ_SR:
		printf_syn("sst25 SR:0x%x!\n", sst25_read_sr());
		break;

	case SST25_SET_PROTE:
		sst25_set_prote_level_to_none();
		break;

	default:
		printf_syn("%s() cmd(%d) err!\n", __FUNCTION__, cmd);
		break;
	}

	printf_syn("%s() over!\n", __FUNCTION__);

	return;
}
FINSH_FUNCTION_EXPORT(sst25_test, cmd-addr-len-initdata);
#endif

#if 0
#include <time.h>
unsigned int lcd_getdeviceid(void);

void tmp_cmd(int cmd)
{
	int temp;
	struct tm *ptime;
	char timestr[20];

	switch (cmd) {
	case 1:
		temp = lcd_getdeviceid();
		printf_syn("lcd dev id:0x%x\n", temp);
		tx_dev_sn[DEV_SN_LEN_MAX] = 0;
		printf_syn("tx_dev_sn:%s\n", tx_dev_sn);
		break;

	case 2:
		PWR_BackupAccessCmd(ENABLE);

		printf_syn("valid-flag:0x%x, rx poweroff cnt:%d, time:0x%x\n",
				   BKP_ReadBackupRegister(POWEROFF_INFO_VALID_FLAG_BKP16BITS),
				   BKP_ReadBackupRegister(RX_POWEROFF_CNT_BKP16BITS),
				   read_poweroff_time_from_bkp_r(RX_POWEROFF_N_BKP16BITS_H, RX_POWEROFF_N_BKP16BITS_L));

		printf_syn("tx poweroff cnt:%d, time:0x%x, 0x%x, 0x%x\n",
				   BKP_ReadBackupRegister(TX1_POWEROFF_CNT_BKP16BITS),
				   read_poweroff_time_from_bkp_r(TX1_POWEROFF_0_BKP16BITS_H, TX1_POWEROFF_0_BKP16BITS_L),
				   read_poweroff_time_from_bkp_r(TX1_POWEROFF_1_BKP16BITS_H, TX1_POWEROFF_1_BKP16BITS_L),
				   read_poweroff_time_from_bkp_r(TX1_POWEROFF_2_BKP16BITS_H, TX1_POWEROFF_2_BKP16BITS_L));

		PWR_BackupAccessCmd(DISABLE);
		break;
	case 3:
		printf_syn("tx poweroff cnt:%d, time:0x%x, 0x%x, 0x%x\n", poweroff_info_data.poi_tx1_poweroff_cnt,
				   poweroff_info_data.poi_tx1_poweroff_t0, poweroff_info_data.poi_tx1_poweroff_t1,
				   poweroff_info_data.poi_tx1_poweroff_t2);
		printf_syn("tx2 poweroff cnt:%d, time:0x%x, 0x%x, 0x%x\n", poweroff_info_data.poi_tx2_poweroff_cnt,
				   poweroff_info_data.poi_tx2_poweroff_t0, poweroff_info_data.poi_tx2_poweroff_t1,
				   poweroff_info_data.poi_tx2_poweroff_t2);

		printf_syn("rx poweroff cnt:%d, time:0x%x, 0x%x, 0x%x\n", poweroff_info_data.poi_rx_poweroff_cnt,
				   poweroff_info_data.poi_rx_poweroff_t0, poweroff_info_data.poi_rx_poweroff_t1,
				   poweroff_info_data.poi_rx_poweroff_t2);

		ptime = localtime(&(poweroff_info_data.poi_tx1_poweroff_t0));
		rt_sprintf(timestr, "%4d%02d%02d.%02d%02d%02d", (ptime->tm_year+1900), (ptime->tm_mon+1),
				   ptime->tm_mday,	ptime->tm_hour, ptime->tm_min, ptime->tm_sec);
		printf_syn("tx_t0:%s\n", timestr);

		ptime = localtime(&(poweroff_info_data.poi_tx1_poweroff_t1));
		rt_sprintf(timestr, "%4d%02d%02d.%02d%02d%02d", (ptime->tm_year+1900), (ptime->tm_mon+1),
				   ptime->tm_mday,	ptime->tm_hour, ptime->tm_min, ptime->tm_sec);
		printf_syn("tx_t1:%s\n", timestr);

		ptime = localtime(&(poweroff_info_data.poi_tx1_poweroff_t2));
		rt_sprintf(timestr, "%4d%02d%02d.%02d%02d%02d", (ptime->tm_year+1900), (ptime->tm_mon+1),
				   ptime->tm_mday,	ptime->tm_hour, ptime->tm_min, ptime->tm_sec);
		printf_syn("tx_t2:%s\n", timestr);



		ptime = localtime(&(poweroff_info_data.poi_tx2_poweroff_t0));
		rt_sprintf(timestr, "%4d%02d%02d.%02d%02d%02d", (ptime->tm_year+1900), (ptime->tm_mon+1),
				   ptime->tm_mday,	ptime->tm_hour, ptime->tm_min, ptime->tm_sec);
		printf_syn("tx2_t0:%s\n", timestr);

		ptime = localtime(&(poweroff_info_data.poi_tx2_poweroff_t1));
		rt_sprintf(timestr, "%4d%02d%02d.%02d%02d%02d", (ptime->tm_year+1900), (ptime->tm_mon+1),
				   ptime->tm_mday,	ptime->tm_hour, ptime->tm_min, ptime->tm_sec);
		printf_syn("tx2_t1:%s\n", timestr);

		ptime = localtime(&(poweroff_info_data.poi_tx2_poweroff_t2));
		rt_sprintf(timestr, "%4d%02d%02d.%02d%02d%02d", (ptime->tm_year+1900), (ptime->tm_mon+1),
				   ptime->tm_mday,	ptime->tm_hour, ptime->tm_min, ptime->tm_sec);
		printf_syn("tx2_t2:%s\n", timestr);



		ptime = localtime(&(poweroff_info_data.poi_rx_poweroff_t0));
		rt_sprintf(timestr, "%4d%02d%02d.%02d%02d%02d", (ptime->tm_year+1900), (ptime->tm_mon+1),
				   ptime->tm_mday,	ptime->tm_hour, ptime->tm_min, ptime->tm_sec);
		printf_syn("rx_t0:%s\n", timestr);

		ptime = localtime(&(poweroff_info_data.poi_rx_poweroff_t1));
		rt_sprintf(timestr, "%4d%02d%02d.%02d%02d%02d", (ptime->tm_year+1900), (ptime->tm_mon+1),
				   ptime->tm_mday,	ptime->tm_hour, ptime->tm_min, ptime->tm_sec);
		printf_syn("rx_t1:%s\n", timestr);

		ptime = localtime(&(poweroff_info_data.poi_rx_poweroff_t2));
		rt_sprintf(timestr, "%4d%02d%02d.%02d%02d%02d", (ptime->tm_year+1900), (ptime->tm_mon+1),
				   ptime->tm_mday,	ptime->tm_hour, ptime->tm_min, ptime->tm_sec);
		printf_syn("rx_t2:%s\n", timestr);

		break;

	default:
		break;
	}

	return;
}
FINSH_FUNCTION_EXPORT(tmp_cmd, cmd);
#endif

#if 1
extern volatile unsigned long rtc_1s_int_cnt;

int sys_info(void)
{
	rt_tick_t rtc_secs;
	int days, hours, mins, seconds;

	rtc_secs = rtc_1s_int_cnt;
	days  = rtc_secs / (24*3600);

	rtc_secs %= 24*3600;
	hours = rtc_secs / 3600;

	rtc_secs %= 3600;
	mins  = rtc_secs / 60;

	seconds  = rtc_secs % 60;

	printf_syn("system had run time is %d days %d:%d:%d(H:M:S)\n", days, hours, mins, seconds);

	return 0;
}
FINSH_FUNCTION_EXPORT(sys_info, print sys info);


extern int touch_print_on;

int set_print_sw(int index, int is_on)
{
	switch (index) {
	case 1:
		touch_print_on = is_on;
		break;

	default:
		break;
	}

	return 0;
}
FINSH_FUNCTION_EXPORT(set_print_sw, set print msg switch);
#endif


/*@}*/
