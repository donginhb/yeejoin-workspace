/*
 * File      : startup.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://openlab.rt-thread.com/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2006-08-31     Bernard      first implementation
 */

#include <rthw.h>
#include <rtthread.h>

#include "stm32f10x.h"
#include "board.h"
#include "rtc.h"

/**
 * @addtogroup STM32
 */

/*@{*/

#define RT_STARTUP_DEBUG(x) //rt_kprintf x

//extern void rt_hw_spiflash_init();
//extern void sf_set_prote_level_to_none(void);

extern int  rt_application_init(void);
#ifdef RT_USING_FINSH
extern void finsh_system_init(void);
extern void finsh_set_device(const char* device);
#endif

#ifdef __CC_ARM
extern int Image$$RW_IRAM1$$ZI$$Limit;
#elif __ICCARM__
#pragma section="HEAP"
#else
extern int __bss_end;
#endif

/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(u8* file, u32 line)
{
	rt_kprintf("\n\r Wrong parameter value detected on\r\n");
	rt_kprintf("       file  %s\r\n", file);
	rt_kprintf("       line  %d\r\n", line);

	while (1) ;
}

/**
 * This function will startup RT-Thread RTOS.
 */
void rtthread_startup(void)
{
	/* init board */
	rt_hw_board_init();

	/* show version */
	rt_show_version();

	/* init tick */
	rt_system_tick_init();

	/* init kernel object */
	rt_system_object_init();

	/* init timer system */
	rt_system_timer_init();

#ifdef RT_USING_HEAP
#if STM32_EXT_SRAM
	rt_system_heap_init((void*)STM32_EXT_SRAM_BEGIN, (void*)STM32_EXT_SRAM_END);
	RT_STARTUP_DEBUG(("fun:%s, line:%d, STM32_EXT_SRAM_BEGIN:0x%x, STM32_EXT_SRAM_END:0x%x\n", __FUNCTION__, __LINE__,
			STM32_EXT_SRAM_BEGIN, STM32_EXT_SRAM_END));
#else
#ifdef __CC_ARM
	rt_system_heap_init((void*)&Image$$RW_IRAM1$$ZI$$Limit, (void*)STM32_SRAM_END);
#elif __ICCARM__
	rt_system_heap_init(__segment_end("HEAP"), (void*)STM32_SRAM_END);
#else
	/* init memory system */
	rt_system_heap_init((void*)&__bss_end, (void*)STM32_SRAM_END);
	RT_STARTUP_DEBUG(("fun:%s, line:%d, __bss_end:0x%x, STM32_SRAM_END:0x%x\n", __FUNCTION__, __LINE__,
			&__bss_end, STM32_SRAM_END));
#endif /* #ifdef __CC_ARM */
#endif /* #if STM32_EXT_SRAM */
#endif

	/* init scheduler system */
	rt_system_scheduler_init();
	RT_STARTUP_DEBUG(("fun:%s, line:%d\n", __FUNCTION__, __LINE__));
#if 0 //def RT_USING_DFS
	/* init sdcard driver */
#if STM32_USE_SDIO
	rt_hw_sdcard_init();
#else
	rt_hw_msd_init();
#endif

#else
	//rt_hw_spiflash_init();
#endif

#if RT_USING_RTC
	rt_hw_rtc_init();
#endif
	/* init all device */
	rt_device_init_all();
	RT_STARTUP_DEBUG(("fun:%s, line:%d\n", __FUNCTION__, __LINE__));
	/* init application */
	rt_application_init();

#ifdef RT_USING_FINSH
	/* init finsh */
	finsh_system_init();
	finsh_set_device(CONSOLE_DEVICE);
#endif

	/* init timer thread */
	rt_system_timer_thread_init();

	/* init idle thread */
	rt_thread_idle_init();

	//RT_STARTUP_DEBUG(("fun:%s, line:%d, 0x%x, 0x%x\n", __FUNCTION__, __LINE__, &_isr_vector_start, _isr_vector_start));
	/* start scheduler */
	rt_system_scheduler_start();

	/* never reach here */
	return ;
}

int main(void)
{
	/* disable interrupt first */
	rt_hw_interrupt_disable();
#ifdef  VECT_TAB_RAM
	/* Set the Vector Table base location at 0x20000000 */
	NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else  /* VECT_TAB_FLASH  */
	/* Set the Vector Table base location at 0x08000000
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0); */
	/* move int-vector-tbl */
	//NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x7800);
	//SCB->VTOR = NVIC_VectTab_FLASH | (0x7800 & (uint32_t)0x1FFFFF80);
#endif

	/* startup RT-Thread RTOS */
	rtthread_startup();

	return 0;
}

/*@}*/
