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

#include <rthw.h>
#include <rtdef.h>

#include <board.h>
#include <rtthread.h>

#include <keyboard.h>
#include <rtconfig.h>

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
#include <netif/stm32netif.h>
#endif
#include <stm32_eth.h>
#include <lcd_touch.h>
#include <am2301.h>
#include <webm_p.h>
#include <sys_app_cfg.h>
#include <enc28j60_io.h>
#include <finsh.h>
#include <lwip/stats.h>


#if 0// defined(RT_USING_DEVICE) && defined(RT_USING_CONSOLE)
static rt_device_t _console_device = RT_NULL;
#endif

#define RT_APPS_INIT_DEBUG(x) //rt_kprintf x
#define RT_APPS_INIT_LOG(x)   //rt_kprintf x

struct rt_semaphore print_sem;

extern int init_syscfgdata_tbl(void);
extern void telnetd_init(void);
extern void httpd_init(void);
extern void rt_sys_misc_entry(void* param);
extern void webm_p_thread_init(void);
extern void ili9320_Initializtion(void);
#if USE_STM32_WWDG
extern void rt_system_daemon_entry(void* param);
#endif

void Delay_ARMJISHU(__IO uint32_t nCount)
{
  for (; nCount != 0; nCount--);
}



void rt_init_thread_entry(void* parameter)
{
	RT_APPS_INIT_DEBUG(("invoke rt_init_thread_entry()!\n"));

	/* init system data cfg table */
	init_syscfgdata_tbl();

	/* Filesystem Initialization */
#ifdef RT_USING_DFS
	{
	/* init the device filesystem */
	dfs_init();

#ifdef RT_USING_DFS_ELMFAT
	/* init the elm chan FatFs filesystam*/
	elm_init();

	/* init sdcard driver */
	rt_hw_msd_init();

	/* mount sd card fat partition 1 as root directory */
	if (dfs_mount("sd0", "/", "elm", 0, 0) == 0)
	{
		RT_APPS_INIT_DEBUG(("File System initialized!\n"));
	}
	else
		RT_APPS_INIT_DEBUG(("File System initialzation failed!\n"));
#endif
	}
#endif

	/* LwIP Initialization */
#ifdef RT_USING_LWIP
	{
	extern void lwip_system_init(void);

	/* register ethernetif device */
	eth_system_device_init();
	RT_APPS_INIT_DEBUG(("eth_system_device_init() succ!\n"));

	rt_hw_stm32_eth_init();
	RT_APPS_INIT_DEBUG(("func:%s, line:%u, eie:0x%x!\n", __FUNCTION__, __LINE__, enc28j60_read(EIE)));
	/* to assure Ethernet Phy work well */ //lihao
	//Ethernet_Security(); 
	RT_APPS_INIT_DEBUG(("func:%s, line:%u!\n", __FUNCTION__, __LINE__));

	/* re-init device driver */
	rt_device_init_all();
	RT_APPS_INIT_DEBUG(("func:%s, line:%u, eie:0x%x!\n", __FUNCTION__, __LINE__, enc28j60_read(EIE)));

	/* init lwip system */
	lwip_system_init();
	RT_APPS_INIT_LOG(("TCP/IP initialized, eie:0x%x!\n", enc28j60_read(EIE)));

	/* init net apps */
	//httpd_init();
	//telnetd_init();

	//webm_p_thread_init();

	RT_APPS_INIT_DEBUG(("func:%s, line:%u, eie:0x%x!\n", __FUNCTION__, __LINE__, enc28j60_read(EIE)));
	}
#endif

	nvic_cfg_app();
}


#if USE_LED_TEST
extern volatile int warning_led_vector;

 char elock_state_pro=0;
 char door_state_pro=0;

/* miscellaneous */
void rt_sys_misc_entry(void* param)
{
    int cnt = 0;
    char value;
    struct nms_if_info_st nms_if;
 
    (void)param;
    

    door_state_pro 	= !is_door_open (ELOCK_SWITCH_PORT, ELOCK_SWITCH_PIN);
    elock_state_pro	= !is_elock_open (ELOCK_PORT, ELOCK_PIN);

    while (1) {

	if (is_bit_set(warning_led_vector, ILLEGAL_OPEN_LOCK)) {
		/* 1Hz */
		if (cnt & 0x02)
			led_on(LED_PORTX, FAULT_LED); 
		else
			led_off(LED_PORTX, FAULT_LED);

	}

	if (is_bit_set(warning_led_vector, TMP_OVERRUN)
		||is_bit_set(warning_led_vector, RH_OVERRUN)) {
		/* 1Hz blink */
		if (cnt & 0x02)
			led_on(LED_PORTX, WARNING_LED); 
		else
			led_off(LED_PORTX, WARNING_LED);
	}

	/* 2Hz blink */
	if (cnt & 0x01) {
		led_on(LED_PORTX, EPON_STATE_LED);
	} else {
		led_off(LED_PORTX, EPON_STATE_LED);
	}
	
        rt_thread_delay(get_ticks_of_ms(250));

	read_syscfgdata_tbl(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
	if (is_bit_set(nms_if.trap_enable_bits, NMS_TRAP_ENABLE_BIT)){		//系统总告警是否开
		value	= !is_elock_open (ELOCK_PORT, ELOCK_PIN);
		if (value != elock_state_pro) {
			snmp_yeejoin_trap (YTI_ELOCK_STATUS_CHANGE);
			elock_state_pro = value;	//门的状态保留
		}

		value 	= !is_door_open (ELOCK_SWITCH_PORT, ELOCK_SWITCH_PIN);
		if (value != door_state_pro) {
			snmp_yeejoin_trap (YTI_DOOR_STATUS_CHANGE);
			door_state_pro = value;	//锁状态保留	
		}

		if ((0 == door_state_pro) & (1 == elock_state_pro)) {
			snmp_yeejoin_trap (YTI_ILLEGAL_OPEN_ELOCK);	
		}
	
	}
			
 


	if (cnt > 10*4)
        	check_if_has_warning(cnt);

	/* 256s(4.27min)同步一次syscfgdata_tbl_cache.syscfg_data */
	if (0!=cnt && (0 == (cnt & 0x3ff))) {
		syscfgdata_syn_proc();
	}

        ++cnt;
    }
}
#endif

int rt_application_init(void)
{
	rt_thread_t init_thread;
	rt_thread_t th_handle;

	rt_sem_init(&print_sem, "prtsem", 1, RT_IPC_FLAG_PRIO);
#if 1==USE_KEY_CTR_RA8875
	rt_mb_init(&lcd_mb._mb, "lcdmb", lcd_mb.pool, sizeof(lcd_mb.pool)/4, RT_IPC_FLAG_FIFO);
#endif
	//rt_mb_init(&webm_mb._mb, "webmmb", webm_mb.pool, sizeof(webm_mb.pool)/4, RT_IPC_FLAG_FIFO);

#if (RT_THREAD_PRIORITY_MAX == 32)
	init_thread = rt_thread_create("init",
	                           rt_init_thread_entry, RT_NULL,
	                           2048, 8, 20);
#else
	init_thread = rt_thread_create("init",
	                           rt_init_thread_entry, RT_NULL,
	                           2048, 80, 20);
#endif

	if (init_thread != RT_NULL)
		rt_thread_startup(init_thread);

#if 1
	th_handle = rt_thread_create("am2301", thread_am2301_entry, RT_NULL,
	                           0X400, 0x10, 20);
	RT_APPS_INIT_DEBUG(("fun:%s, line:%d, th_handle:%x\n", __FUNCTION__, __LINE__, th_handle));
	if (th_handle != RT_NULL)
		rt_thread_startup(th_handle);
#endif

#if USE_LED_TEST
	RT_APPS_INIT_DEBUG(("fun:%s, line:%d\n", __FUNCTION__, __LINE__));
	th_handle = rt_thread_create("misc", rt_sys_misc_entry, RT_NULL,
	                           0x300, RT_THREAD_PRIORITY_MAX-2, 8);
	if (th_handle != RT_NULL)
		rt_thread_startup(th_handle);
#endif


#if 1==USE_KEY_CTR_RA8875
	/* init lcd controller */
	ili9320_Initializtion();
	th_handle = rt_thread_create("lcd-th", thread_lcd_entry, RT_NULL,
	                           0x200, 10, 20);
	RT_APPS_INIT_DEBUG(("fun:%s, line:%d, th_handle:%x\n", __FUNCTION__, __LINE__, th_handle));
	if (th_handle != RT_NULL)
		rt_thread_startup(th_handle);
#endif


#if 1==USE_TO_7INCH_LCD
	keyboard_init();
	th_handle = rt_thread_create("keypad",
	                           thread_keyboard_entry, RT_NULL,
	                           0x400, 0x11, 20);
	RT_APPS_INIT_DEBUG(("fun:%s, line:%d, th_handle:%x\n", __FUNCTION__, __LINE__, th_handle));
	if (th_handle != RT_NULL)
		rt_thread_startup(th_handle);
#endif


#if USE_STM32_WWDG
	th_handle = rt_thread_create("daemon", rt_system_daemon_entry, RT_NULL,
	                           0x200, 0x06, 3);
	RT_APPS_INIT_DEBUG(("fun:%s, line:%d, th_handle:%x\n", __FUNCTION__, __LINE__, th_handle));
	if (th_handle != RT_NULL)
		rt_thread_startup(th_handle);

#endif
	RT_APPS_INIT_DEBUG(("fun:%s, line:%d\n", __FUNCTION__, __LINE__));
	return 0;
}

#if USE_STM32_WWDG
extern void wwdg_init(void);
static int is_sys_network_ok(void);

/* 单位是ms */
#define CHECK_SYS_PERIOD  (1000)
#define WAIT_SPI_TIMEOUT  (1000)
#define WAIT_SPI_TIME     (3000)
#define DELAY_JUDGE_PKT_RX_CNT_TIME (6000)

volatile int is_need_fed_wwdg;

/*
 * 定时对系统关键任务做检查以决定是否 喂窗口看门狗
 */
void rt_system_daemon_entry(void* param)
{

	(void)param;
	
	wwdg_init();
	is_need_fed_wwdg = 1;
	
	while (1) {
		rt_thread_delay(get_ticks_of_ms(CHECK_SYS_PERIOD));

		/* 执行时间不能超过 CHECK_SYS_PERIOD-tWWDG (ms)
		 * 还应留有一定裕量, 以防止由于isr占用时间而延误
		 */
		if (is_sys_network_ok()) {
			is_need_fed_wwdg = 1;
		} else {
			is_need_fed_wwdg = 0;
		}
	}
}

void wwdg_init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);
	WWDG_SetPrescaler(WWDG_Prescaler_8);
	/* 不使用过早喂狗复位功能 */
	WWDG_SetWindowValue(0x7f);

	WWDG_Enable(WWDG_RELOAD_VALUE);
	//rt_kprintf("func:%s, line:%d, CR:0x%x, CFR:0x%x, SR:0x%x\n", __FUNCTION__, __LINE__, WWDG->CR, WWDG->CFR, WWDG->SR);

	return;		
}

/*
 * 只能由rt_system_daemon_entry()调用
 */
/* 故障情况判断
 *
 * 1. 当TCPIP_STACK_TX_CNT增加时, enc28j60_int_cnt_tx在1s内没有发生变化 -- 协议栈正常, enc28j60故障或其驱动故障
 * 2. 当enc28j60_pkt_cnt_rx增加时, TCPIP_STACK_RX_CNT在1s内没有发生变化 -- enc28j60及其驱动正常, 协议栈故障
 *
 * 3. 当enc28j60_read(EPKTCNT)不为零时, enc28j60_pkt_cnt_rx在1s内没有发生变化 -- enc28j60故障或其驱动故障
 *
 * TCPIP_STACK_TX_CNT, TCPIP_STACK_RX_CNT不好确定, 现在只做3的判断
 */
#if 0
#define TCPIP_STACK_TX_CNT ()
#define TCPIP_STACK_RX_CNT ()
	/*  */
	lwip_stats.ip.xmit
	lwip_stats.ip.recv
#endif
/* !!!NOTE:应该以最快速度返回, 挂起时间必须有限制, 否则可能引起看门狗定时器溢出 */
static int is_sys_network_ok(void)
{
	static int is_delay_judge;
	static rt_uint32_t rx_pkt_cnt;
	static int wait_spi_timeout;

	int cnt, ret = TRUE;
	rt_err_t result;

	if (0 == is_delay_judge) {
		/* get enc28j60 spi */
		result = rt_sem_take(&enc28j60_spi_free, get_ticks_of_ms(WAIT_SPI_TIMEOUT));
		if (RT_EOK == result) {
			wait_spi_timeout = 0;
		} else if (-RT_ETIMEOUT == result) {
			++wait_spi_timeout;
			/* 连续3s内都获取不到spi, 说明系统异常, 不再喂狗 */
			if (wait_spi_timeout > WAIT_SPI_TIME/(WAIT_SPI_TIMEOUT+CHECK_SYS_PERIOD)) {
				rt_kprintf("%s()  wait spi timeout(%d) !\n", __FUNCTION__, wait_spi_timeout);
				return FALSE;
			} else
				return TRUE;
		} else {
			printf_syn("%s() get enc28j60_spi_free fail!\n", __FUNCTION__);
			goto reset_sys;
		}
		enc28j60_nvic_cfg(DISABLE);

		cnt  =  enc28j60_read(EPKTCNT);
		if (0 != cnt) {
			++is_delay_judge;
			rx_pkt_cnt = enc28j60_pkt_cnt_rx;
		}

		enc28j60_nvic_cfg(ENABLE);
		rt_sem_release(&enc28j60_spi_free);
	} else {
		/* 6s后查看enc28j60_pkt_cnt_rx是否有变化 */
		if (is_delay_judge >= DELAY_JUDGE_PKT_RX_CNT_TIME/CHECK_SYS_PERIOD) {
			is_delay_judge = 0;
			ret = (rx_pkt_cnt != enc28j60_pkt_cnt_rx) ? TRUE : FALSE;
		} else {
			++is_delay_judge;
		}
	}

	if (FALSE == ret)
		rt_kprintf("%s() enc28j60_pkt_cnt_rx not change!\n", __FUNCTION__);

	return ret;

reset_sys:
	reset_whole_system();
	return FALSE;	
}

#endif

#if 1

int sys_info(void)
{
	rt_tick_t ticks_temp;
	int days, hours, mins, seconds;
	
	ticks_temp = rt_tick_get();

	ticks_temp /= RT_TICK_PER_SECOND;
	days  = ticks_temp / (24*3600);
	
	ticks_temp %= 24*3600;
	hours = ticks_temp / 3600;

	ticks_temp %= 3600;
	mins  = ticks_temp / 60;

	seconds  = ticks_temp % 60;
	
	printf_syn("system had run time is %d days %d:%d:%d(H:M:S)\n", days, hours, mins, seconds);
	
	printf_syn("enc28j60_int_cnt_rx:%lu, enc28j60_int_cnt_tx:%lu, enc28j60_pkt_cnt_rx:%lu, enc28j60_pkt_cnt_tx:%lu!\n", 
			enc28j60_int_cnt_rx, enc28j60_int_cnt_tx, enc28j60_pkt_cnt_rx, enc28j60_pkt_cnt_tx);

	
	return 0;
}
FINSH_FUNCTION_EXPORT(sys_info, print sys info);


extern void stats_display(unsigned int dis_vector);
FINSH_FUNCTION_EXPORT(stats_display, "lwip stats display bit0-bit10:link,arp,ipfrag,ipstats,igmp,icmp,udp,tcp,mem,memp,sys");
#endif


/*@}*/
