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
#include <rtdef.h>
#include <board.h>
#include <rtthread.h>
#include <finsh.h>
#include <syscfgdata.h>
#include <ade7880_hw.h>
#include <ade7880_api.h>

#if EM_ALL_TYPE_BASE
#include <sink_info.h>
#include <ammeter.h>
#endif

#if RT_USING_FILESYSTEM
/* dfs init */
#include <dfs_init.h>
/* dfs filesystem:ELM filesystem init */
#include <dfs_elm.h>
/* dfs Filesystem APIs */
#include <dfs_fs.h>
#endif

#if RT_USING_TCPIP_STACK
#include <lwip/sys.h>
#include <lwip/api.h>
#endif

#if RT_USING_GUI
#include <rtgui/rtgui.h>
#include <rtgui/rtgui_server.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/driver.h>
#endif

#if RT_USING_SERIAL_FLASH
#include <spiflash.h>
#include <sf_hal.h>
#endif

#if RT_USING_TCPIP_STACK
#include <enc28j60_io.h>
#include <stm32_eth.h>
#include <telnetd.h>
#include <lwip/private_trap.h>
#endif

#include <rs485.h>
#include <app-thread.h>

#define RT_APPS_INIT_DEBUG(x) //rt_kprintf x
#define RT_APPS_INIT_LOG(x)   rt_kprintf x

#define TEST_485	0
#define TEST_TRAP	0
#define TEST_164_LED	0

/*
 * 0 -- 不测试
 * 1 -- 逐个输出
 * 2 -- 只报告异常
 * 3 -- 手动
 */
#define TEST_PSRAM_MODE 0


extern void init_tcpip_lwip_stack(void);

extern void nvic_cfg_app(void);
extern int init_syscfgdata_tbl(void);

extern void start_7880(void);
extern int creat_4432master_th(void);
extern int creat_4432slave_th(void);
extern void rt_val_7880(void);  

static rt_uint32_t misc_stack[512/4];
static struct rt_thread misc_thread;
extern int rs485_master_init(void);
extern int rs485_slave_init(void);

#if EM_ALL_TYPE_BASE
extern void recv_em_frame(void* parameter);
#endif



static void rt_sys_misc_entry(void* parameter)
{
	unsigned int cnt=0;
#if 1==TEST_PSRAM_MODE || 2==TEST_PSRAM_MODE /* test PSRAM */
	volatile rt_uint32_t *p = (rt_uint32_t *)STM32_EXT_SRAM_START_ADDR;
	rt_uint32_t i = 0;
#endif

	(void)parameter;

	say_thread_start();

#if EM_MULTI_BASE
#if TEST_164_LED
	serial_to_parallel_bit_vector = 0xa5a5a5a5;
#else
	serial_to_parallel_bit_vector = 0;
	serial_to_parallel_bit_vector_prev = 1;
#endif
#endif
	while (1) {
		cnt++;
		rt_thread_delay(get_ticks_of_ms(250));

#if 1	/* 1Hz blink */
		if (cnt & 0x01) {
			running_led_blink();
			//led_blink(led2_gpio, led2_pin);
#if EM_MULTI_BASE
#if TEST_164_LED
			serial_to_parallel_bit_vector ^= 0xffffffff;
#else
			serial_to_parallel_bit_vector ^= serial_to_parallel_bit_vector_blink;
#endif
			if (serial_to_parallel_bit_vector_prev != serial_to_parallel_bit_vector) {
				write_32bit_to_74hc164(serial_to_parallel_bit_vector);
				serial_to_parallel_bit_vector_prev = serial_to_parallel_bit_vector;
			}
#endif
		}
#endif
#if 0	/* 0.5Hz */
		if (cnt & 0x02) {
		}
#endif
		/* 256s(4.27min) syscfgdata_tbl_cache.syscfg_data */
		if (0!=cnt && (0 == (cnt & 0x3ff))) {
#if RT_USING_FILESYSTEM
			flush_sfwb(0); /* mark by David */
#endif
			syscfgdata_syn_proc();
		}

#if 1==TEST_PSRAM_MODE /* test PSRAM */
	if ((0 == (cnt & 0x7)) && (p < STM32_EXT_SRAM_START_ADDR+STM32_EXT_SRAM_MAX_LEN)) {
		if (0 != i) {
			*p = i++;
			printf_syn("addr:0x%x, value:0x%x\n", p, *p);
		} else {
			rt_uint8_t *pch;
			*p = 0x12345678;
			pch = (rt_uint8_t *)p;
			printf_syn("addr:0x%x, value:0x%x, [0-3]:0x%x, 0x%x, 0x%x, 0x%x\n",
				p, *p, pch[0], pch[1], pch[2], pch[3]);
			++i;
		}
		++p;
	}
#elif 2==TEST_PSRAM_MODE
	if ((0 == (cnt & 0x7)) && (p == STM32_EXT_SRAM_START_ADDR)) {
		for (i=0; i<(STM32_EXT_SRAM_MAX_LEN/4); ++i) {
			*p = i;
			++p;
		}
		printf_syn("write psram over! last addr:0x%x, last value:0x%x, i:0x%x\n", (p-1), *(p-1), i);

		p = (rt_uint32_t *)STM32_EXT_SRAM_START_ADDR;
		//for (i=0; i<(STM32_EXT_SRAM_MAX_LEN/4); ++i) {
		for (i=0; i<3; ++i) {
			if (*p != i) {
				printf_syn("addr:0x%x, value:0x%x, i:0x%x\n", p, *p, i);
			}
			++p;
		}

		p = (rt_uint32_t *)STM32_EXT_SRAM_START_ADDR;
		//for (i=0; i<(STM32_EXT_SRAM_MAX_LEN/4); ++i) {
		for (i=0; i<3; ++i) {
			if (*p != i) {
				printf_syn("addr:0x%x, value:0x%x, i:0x%x\n", p, *p, i);
			}
			++p;
		}

		printf_syn("read psram over!\n");
	}
#endif

#if USE_STM32_IWDG
		/* Reloads IWDG counter with value defined in the reload register */
		/* #define KR_KEY_Reload    ((uint16_t)0xAAAA) */
		IWDG->KR = 0xAAAA;
#endif
	} /* while(1) */

}

#if RT_USING_ADE7880 || RT_USING_SI4432_MAC
extern void tim2_isr(void);
extern void tim4_isr(void);
extern void si4432_mac_exti_isr(void);
static void rt_isr_event_entry(void* parameter)
{
	rt_err_t err;
	rt_uint32_t e;

	printf_syn("rt_isr_event_entry()\n");
	say_thread_start();

	while (1) {
		err = rt_event_recv(&isr_event_set
				, EVENT_BIT_NEED_RUN_TIM2_IRQ | EVENT_BIT_NEED_RUN_TIM4_IRQ | EVENT_BIT_NEED_RUN_SI4432_MAC_EXTI_IRQ
				, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &e);

		if (RT_EOK != err) {
			printf_syn("recv isr event error(%d)", err);
		} else {
#if RT_USING_SI4432_MAC
			if (is_bit_set(isr_event_var_flag, EVENT_BIT_NEED_RUN_SI4432_MAC_EXTI_IRQ)) {
				clr_bit(isr_event_var_flag, EVENT_BIT_NEED_RUN_SI4432_MAC_EXTI_IRQ);
				si4432_mac_exti_isr();
			}
#endif
#if RT_USING_ADE7880
			if (is_bit_set(isr_event_var_flag, EVENT_BIT_NEED_RUN_TIM4_IRQ)) {
				clr_bit(isr_event_var_flag, EVENT_BIT_NEED_RUN_TIM4_IRQ);
#if 0 == ADE7880_USE_I2C_HSDC
 				tim4_isr();
#endif
			}
#endif
#if RT_USING_SI4432_MAC
			if (is_bit_set(isr_event_var_flag, EVENT_BIT_NEED_RUN_SI4432_MAC_EXTI_IRQ)) {
				clr_bit(isr_event_var_flag, EVENT_BIT_NEED_RUN_SI4432_MAC_EXTI_IRQ);
				si4432_mac_exti_isr();
			}

			if (is_bit_set(isr_event_var_flag, EVENT_BIT_NEED_RUN_TIM2_IRQ)) {
				clr_bit(isr_event_var_flag, EVENT_BIT_NEED_RUN_TIM2_IRQ);
				tim2_isr();
			}
#endif
		}
	}

	return;
}
#endif

#if TEST_485

struct rt_semaphore uart485_1_rx_msg_sem;
struct rt_semaphore uart485_2_rx_msg_sem;
struct rt_semaphore uart485_3_rx_msg_sem;
#if 1
#define WAIT_RX_MSG_TIMEOUT (get_ticks_of_ms(1500))
#else
#define WAIT_RX_MSG_TIMEOUT RT_WAITING_FOREVER
#endif

static rt_err_t uart485_1_rx_ind(rt_device_t dev, rt_size_t size)
{
	rt_sem_release(&uart485_1_rx_msg_sem);

//	rt_kprintf("**%s(), len:%u\n", __FUNCTION__, size);
	return RT_EOK;
}

static rt_err_t uart485_2_rx_ind(rt_device_t dev, rt_size_t size)
{
	rt_sem_release(&uart485_2_rx_msg_sem);

//	rt_kprintf("**%s(), len:%u\n", __FUNCTION__, size);
	return RT_EOK;
}

static rt_err_t uart485_3_rx_ind(rt_device_t dev, rt_size_t size)
{
	rt_sem_release(&uart485_3_rx_msg_sem);

//	rt_kprintf("**%s(), len:%u\n", __FUNCTION__, size);
	return RT_EOK;
}

/*
 * FE FE FE FE 68 AA AA AA AA AA AA 68 01 02 43 C3 D5 16
羽 16:41:57
这个数据帧是什么命令，应该返回什么数据？
张宏磊 16:42:56r
正向有功电能
张宏磊 16:43:12
返回68 43 97 35 14 00 00 68 81 06 43 C3 7B 96 57 33 1B 16
 * */
char cmd_485_seq[] = {0xfe, 0xfe, 0xfe, 0xfe, 0x68, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x68, 0x01, 0x02, 0x65, 0xf3, 0x27, 0x16};

static void rt_test_485_entry(void* parameter)
{
	rt_device_t dev_485_1;
	rt_device_t dev_485_2;
	rt_device_t dev_485_3;

	rt_size_t read_cnt;

	unsigned char tx_ch = 0;
	unsigned char rx_ch = 0;
	unsigned char rx_buf[4];

	rt_sem_init(&uart485_1_rx_msg_sem, "u485-1", 0, RT_IPC_FLAG_PRIO);
	rt_sem_init(&uart485_2_rx_msg_sem, "u485-2", 0, RT_IPC_FLAG_PRIO);
	rt_sem_init(&uart485_3_rx_msg_sem, "u485-3", 0, RT_IPC_FLAG_PRIO);
#if !EM_ALL_TYPE_BASE
	uart485_mb = rt_mb_create("485_mb", 1, RT_IPC_FLAG_FIFO);
	if (RT_NULL == uart485_mb) {
		printf_syn("func:%s(), line:%d malloc fail\n",__FUNCTION__, __LINE__);
		return;
	}
#endif
	dev_485_1 = rt_device_find(UART_485_1_DEV);
	if (dev_485_1 != RT_NULL && rt_device_open(dev_485_1, RT_DEVICE_OFLAG_RDWR) == RT_EOK) {
		printf_syn("rt_test_485_entry open device:%s succ\n", UART_485_1_DEV);
		rt_device_set_rx_indicate(dev_485_1, uart485_1_rx_ind);
	} else {
		printf_syn("rt_test_485_entry can not find device:%s\n", UART_485_1_DEV);
		return;
	}

	dev_485_2 = rt_device_find(UART_485_2_DEV);
	if (dev_485_2 != RT_NULL && rt_device_open(dev_485_2, RT_DEVICE_OFLAG_RDWR) == RT_EOK) {
		printf_syn("rt_test_485_entry open device:%s succ\n", UART_485_2_DEV);
		rt_device_set_rx_indicate(dev_485_2, uart485_2_rx_ind);
	} else {
		printf_syn("rt_test_485_entry can not find device:%s\n", UART_485_2_DEV);
		return;
	}

	dev_485_3 = rt_device_find(UART_485_3_DEV);
	if (dev_485_3 != RT_NULL && rt_device_open(dev_485_3, RT_DEVICE_OFLAG_RDWR) == RT_EOK) {
		printf_syn("rt_test_485_entry open device:%s succ\n", UART_485_3_DEV);
		rt_device_set_rx_indicate(dev_485_3, uart485_3_rx_ind);
	} else {
		printf_syn("rt_test_485_entry can not find device:%s\n", UART_485_3_DEV);
		return;
	}

	while (1) {
#if 0
		tx_en_rev_disable_485_1();
#else
#if 0
		int rx_cnt, err_cnt;
		rt_err_t err;
		int i;

		rx_cnt = 0;
		err_cnt = 0;

		printf_syn("send cmd: ");
		for (i=0; i<sizeof(cmd_485_seq); ++i)
			printf_syn("%02x ", cmd_485_seq[i]);
		printf_syn("\n");

		//rt_enter_critical();
		tx_en_rev_disable_485_2();
		//rt_thread_delay(2);
		dev_485_2->write(dev_485_2, 0, cmd_485_seq, sizeof(cmd_485_seq));
		//rt_thread_delay(1);
		wait_usartx_send_over(USART3);
			
		tx_disable_rev_en_485_2();

		//rt_exit_critical();

		printf_syn("485-2 485 cmd had send over\n");

		while (rx_cnt<23 && err_cnt<2) {
			if (RT_EOK == (err=rt_sem_take(&uart485_2_rx_msg_sem, WAIT_RX_MSG_TIMEOUT))) {
				while (1 == dev_485_2->read(dev_485_2, 0, &rx_ch, 1)) {
					++rx_cnt;
					printf_syn("485-2 recv ind[%3d]:0x%2x\n", rx_cnt, rx_ch);
				}
			} else {
				printf_syn("485-2 recv error(%d)\n", err);
				++err_cnt;
			}
		}

		rt_thread_delay(get_ticks_of_ms(5 * 1000));

#else
		tx_en_rev_disable_485_1();
		rt_thread_delay(2);
		printf_syn("485-1 send:%d\n", tx_ch);
		dev_485_1->write(dev_485_1, 0, &tx_ch, 1);
		rt_thread_delay(2);
		tx_disable_rev_en_485_1();
#if 0
#if 0
		if (RT_EOK == rt_sem_take(&uart485_3_rx_msg_sem, WAIT_RX_MSG_TIMEOUT)) {
			read_cnt = dev_485_3->read(dev_485_3, 0, rx_buf, sizeof(rx_buf));
			if (read_cnt != 0) {
				rx_ch = rx_buf[read_cnt - 1];
			} else {
				rx_ch = 0;
				printf_syn("485-3 recv error(%d)\n", rt_get_errno());
			}

			printf_syn("485-3 recv:%d, read_cnt:%d\n", rx_ch, read_cnt);

			rt_thread_delay(5);

			tx_en_rev_disable_485_3();
			rt_thread_delay(2);
			printf_syn("485-3 send:%d\n", rx_ch);
			dev_485_3->write(dev_485_3, 0, &rx_ch, 1);
			rt_thread_delay(2);
			tx_disable_rev_en_485_3();

		} else {
			printf_syn("485-3 recv error\n");
		}
#else
		if (RT_EOK == rt_sem_take(&uart485_2_rx_msg_sem, WAIT_RX_MSG_TIMEOUT)) {
			read_cnt = dev_485_2->read(dev_485_2, 0, rx_buf, sizeof(rx_buf));
			if (read_cnt != 0) {
				rx_ch = rx_buf[read_cnt - 1];
			} else {
				rx_ch = 0;
				printf_syn("485-2 recv error(%d)\n", rt_get_errno());
			}

			printf_syn("485-2 recv:%d, read_cnt:%d\n", rx_ch, read_cnt);

			rt_thread_delay(5);

			tx_en_rev_disable_485_2();
			rt_thread_delay(2);
			printf_syn("485-2 send:%d\n", rx_ch);
			dev_485_2->write(dev_485_2, 0, &rx_ch, 1);
			rt_thread_delay(2);
			tx_disable_rev_en_485_2();

		} else {
			printf_syn("485-2 recv error\n");
		}

#endif
#else
		if (tx_ch & 0x01) {
			if (RT_EOK == rt_sem_take(&uart485_3_rx_msg_sem, WAIT_RX_MSG_TIMEOUT)) {
				read_cnt = dev_485_3->read(dev_485_3, 0, rx_buf, sizeof(rx_buf));
				if (read_cnt != 0) {
					rx_ch = rx_buf[read_cnt - 1];
				} else {
					rx_ch = 0;
					printf_syn("485-3 recv error(%d)\n", rt_get_errno());
				}

				printf_syn("485-3 recv:%d, read_cnt:%d\n", rx_ch, read_cnt);

				rt_thread_delay(5);

				tx_en_rev_disable_485_3();
				rt_thread_delay(2);
				printf_syn("485-3 send:%d\n", rx_ch);
				dev_485_3->write(dev_485_3, 0, &rx_ch, 1);
				rt_thread_delay(2);
				tx_disable_rev_en_485_3();

			} else {
				printf_syn("485-3 recv error\n");
			}
		} else {
			if (RT_EOK == rt_sem_take(&uart485_2_rx_msg_sem, WAIT_RX_MSG_TIMEOUT)) {
				read_cnt = dev_485_2->read(dev_485_2, 0, rx_buf, sizeof(rx_buf));
				if (read_cnt != 0) {
					rx_ch = rx_buf[read_cnt - 1];
				} else {
					rx_ch = 0;
					printf_syn("485-2 recv error(%d)\n", rt_get_errno());
				}

				printf_syn("485-2 recv:%d, read_cnt:%d\n", rx_ch, read_cnt);

				rt_thread_delay(5);

				tx_en_rev_disable_485_2();
				rt_thread_delay(2);
				printf_syn("485-2 send:%d\n", rx_ch);
				dev_485_2->write(dev_485_2, 0, &rx_ch, 1);
				rt_thread_delay(2);
				tx_disable_rev_en_485_2();

			} else {
				printf_syn("485-2 recv error\n");
			}
		}
#endif
		if (RT_EOK == rt_sem_take(&uart485_1_rx_msg_sem, WAIT_RX_MSG_TIMEOUT)) {
			dev_485_1->read(dev_485_1, 0, &rx_ch, 1);
			printf_syn("485-1 recv echo:%d\n\n", rx_ch);

		} else {
			printf_syn("485-1 recv echo error\n\n");
		}

		rt_thread_delay(get_ticks_of_ms(1*1000));
		++tx_ch;

#endif
#endif
	} /* while(1) */

	return;
}
#endif

#if EM_ALL_TYPE_BASE
extern void sinkinfo_init(void);
extern void sinkinfo_ipc_init(void);
#endif

void rt_init_thread_entry(void* parameter)
{
#if RT_USING_RTC
	rt_device_t rtc_dev;
#endif
#if TEST_TRAP || EM_ALL_TYPE_BASE || TEST_485
	rt_thread_t thread_h;
#endif
	RT_APPS_INIT_DEBUG(("invoke rt_init_thread_entry()!\n"));

	say_thread_start();

#if TEST_485
	thread_h = rt_thread_create("t485", rt_test_485_entry, RT_NULL, 512, 30, 10);
	if (thread_h != RT_NULL)
		rt_thread_startup(thread_h);
#endif
#if TEST_TRAP
		trap_test_init();
#endif

#if EM_ALL_TYPE_BASE && !TEST_485
	/* recv ptct info server */
	thread_h = rt_thread_create("rpct-ser", recv_em_frame, RT_NULL, 1024, 30, 10);
	if (thread_h != RT_NULL)
		rt_thread_startup(thread_h);
#endif

#if RT_USING_RS485_BUS  && !TEST_485
#if EM_MASTER_DEV || EM_MULTI_MASTER_DEV
	rs485_master_init(); /* David */
#elif WIRELESS_MASTER_NODE
	rs485_slave_init();
#endif
#endif

	/* Filesystem Initialization */
#if RT_USING_FILESYSTEM
	RT_APPS_INIT_DEBUG(("will init fs\n"));
	rt_hw_spiflash_init();
	sf_set_prote_level_to_none();

	/* init the device filesystem */
	dfs_init();
#ifdef RT_USING_DFS_ELMFAT
	/* init the elm chan FatFs filesystam*/
	elm_init();

#if 1
	/* mount spi-flash fat partition 1 as root directory */
	if (dfs_mount("sf0", "/", "elm", 0, 0) == 0)
#else
	/* mount sd card fat partition 1 as root directory */
	if (dfs_mount("sd0", "/", "elm", 0, 0) == 0)
#endif
		rt_kprintf("File System initialized!\n");
	else
		rt_kprintf("File System initialzation failed!\n");
#endif

#endif

	/* LwIP Initialization */
#if RT_USING_TCPIP_STACK
	init_tcpip_lwip_stack();
#endif

#if RT_USING_GUI
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
#endif /* #if RT_USING_GUI */

	nvic_cfg_app();
#if USE_STM32_IWDG
	IWDG_Enable();
#endif

#if RT_USING_RTC
	rtc_dev = rt_device_find("rtc");
	if (NULL != rtc_dev) {
		rtc_dev->control(rtc_dev, RT_DEVICE_CTRL_RTC_CALI_SET, NULL);
	} else {
		rt_kprintf("find rtc device fail\n");
	}
#endif

#if EM_ALL_TYPE_BASE
	extern void sinkinfo_ipc_init(void);
	sinkinfo_ipc_init();
#endif
#if (RT_USING_RS485_BUS && EM_ALL_TYPE_BASE  && !TEST_485)
	ammeter_init();
#endif

#if WIRELESS_MASTER_NODE
	creat_4432master_th();
#elif WIRELESS_SLAVE_NODE /* #if WIRELESS_MASTER_NODE */
	creat_4432slave_th();
#else
	/* none */
#endif /* #if WIRELESS_MASTER_NODE */

#if RT_USING_ADE7880 && !TEST_485
	start_7880();	/* 有较长的延迟, 秒级别, David */
#endif

#if EM_ALL_TYPE_BASE && !TEST_485
	sinkinfo_init();
#endif

}



extern struct rt_semaphore print_sem;

#if RT_USING_FILESYSTEM
extern struct rt_semaphore spiflash_sem;
#endif
int rt_application_init()
{
	rt_thread_t thread_h;
	rt_err_t result;

	/* init system data cfg table */
	init_syscfgdata_tbl();

	rt_sem_init(&print_sem, "prtsem", 1, RT_IPC_FLAG_PRIO);
#if RT_USING_FILESYSTEM
	rt_sem_init(&spiflash_sem, "sfsem", 1, RT_IPC_FLAG_PRIO);
#endif
#if (ADE7880_SPIFLASH_SHARE_SPI || ADE7880_SI4432_SHARE_SPI)
	rt_sem_init(&spi_sem_share_by_ade7880, "spi3sem", 1, RT_IPC_FLAG_PRIO);
#endif

#if RT_USING_ADE7880 || RT_USING_SI4432_MAC
	rt_event_init(& isr_event_set, "isr_eve", RT_IPC_FLAG_PRIO);
#endif

#if (RT_USING_RS485_BUS && !TEST_485)
	init_sys_485();
#endif

	RT_APPS_INIT_DEBUG(("func:%s(), will init rt_sys_misc_entry\n", __FUNCTION__));
	/* init led thread */
	result = rt_thread_init(&misc_thread, "misc", rt_sys_misc_entry,
			RT_NULL, (rt_uint8_t*)&misc_stack[0], sizeof(misc_stack), 0x19, 5);
	if (result == RT_EOK)
		rt_thread_startup(&misc_thread);

#if RT_USING_ADE7880 || RT_USING_SI4432_MAC
	thread_h = rt_thread_create("isr_e", rt_isr_event_entry, RT_NULL, 512, ISR_EVENT_THREAD_PRIORITY, 10);
	if (thread_h != RT_NULL)
		rt_thread_startup(thread_h);
#endif

#if (RT_THREAD_PRIORITY_MAX == 32)
	thread_h = rt_thread_create("init", rt_init_thread_entry, RT_NULL, 2048, 8, 20);
#else
	thread_h = rt_thread_create("init", rt_init_thread_entry, RT_NULL, 2048, 80, 20);
#endif
	if (thread_h != RT_NULL)
		rt_thread_startup(thread_h);

	RT_APPS_INIT_DEBUG(("func:%s(), base thread initial over\n", __FUNCTION__));

	return 0;
}


#if 1
int sys_info(void)
{
	rt_tick_t rtc_secs;
	int days, hours, mins, seconds;
#if RT_USING_RTC
	extern volatile unsigned long rtc_1s_int_cnt;
	rtc_secs = rtc_1s_int_cnt;
#else
	rtc_secs = rt_tick_get() / RT_TICK_PER_SECOND;
#endif
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
#endif


#if RT_USING_TCPIP_STACK
void init_tcpip_lwip_stack(void)
{
	extern void lwip_system_init(void);
	extern rt_err_t eth_system_device_init(void);

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
#if RT_USING_HTTPSERVER_RAW
	httpd_init();
#endif

#if RT_USING_TELNETD
	telnetd_init();
#endif
}
#endif




/*@}*/
