/* RT-Thread config file */
#ifndef __RTTHREAD_CFG_H__
#define __RTTHREAD_CFG_H__


#include "app_cfg.h"

/*
 * app_cfg_pt_m		无线主节点
 * app_cfg_pt_s 	无线从节点
 * app_cfg_ct_m 	无线主节点
 * app_cfg_ct_s 	无线从节点
 * app_cfg_emc_m 	300设备的485主节点
 * app_cfg_emc_s 	300设备的485主节点
 *
 * app_cfg_emm_m	300M设备的485主节点
 * app_cfg_emm_s	300M设备的485从节点
 * */

//#define APP_CFG_FILENAME "app_cfg_pt_m.h"
//#include "app_cfg_pt_m.h"

//#define APP_CFG_FILENAME "app_cfg_pt_s.h"
//#include "app_cfg_pt_s.h"

//#define APP_CFG_FILENAME "app_cfg_ct_m.h"
//#include "app_cfg_ct_m.h"

//#define APP_CFG_FILENAME "app_cfg_ct_s.h"
//#include "app_cfg_ct_s.h"

#if 0
//#define APP_CFG_FILENAME "app_cfg_emc_m.h"
//#include "app_cfg_emc_m.h"
//#define APP_CFG_FILENAME "app_cfg_emc_s.h"
//#include "app_cfg_emc_s.h"
#endif

#define APP_CFG_FILENAME "app_cfg_emm_m.h"
#include "app_cfg_emm_m.h"

#if 0
//#define APP_CFG_FILENAME "app_cfg_emm_s.h"
//#include "app_cfg_emm_s.h"
#endif

#define COMPILE_MISC_TEST_FILE	1


/* RT_NAME_MAX*/
#define RT_NAME_MAX	8

/* RT_ALIGN_SIZE*/
#define RT_ALIGN_SIZE	8

/* PRIORITY_MAX */
#define RT_THREAD_PRIORITY_MAX	32

/* Tick per Second */
#define RT_TICK_PER_SECOND	100



/* SECTION: RT_DEBUG */
#define RT_DEBUG	1

#define RT_THREAD_DEBUG		0
#define RT_DEBUG_MEM		0



#define RT_USING_OVERFLOW_CHECK

/* Using Hook */
#define RT_USING_HOOK

/* Using Software Timer */
/* #define RT_USING_TIMER_SOFT */
#define RT_TIMER_THREAD_PRIO		4
#define RT_TIMER_THREAD_STACK_SIZE	512
#define RT_TIMER_TICK_PER_SECOND	10



/* SECTION: IPC */
/* Using Semaphore*/
#define RT_USING_SEMAPHORE

/* Using Event */
#define RT_USING_EVENT

/* Using MailBox */
#define RT_USING_MAILBOX

#if EM_ALL_TYPE_BASE
/* Using Mutex */
#define RT_USING_MUTEX

/* Using Message Queue */
#define RT_USING_MESSAGEQUEUE
#endif /* WIRELESS_MASTER_NODE */



/* SECTION: Memory Management */
/* Using Memory Pool Management*/
#define RT_USING_MEMPOOL

/* Using Dynamic Heap Management */
#define RT_USING_HEAP

/* Using Small MM */
#define RT_USING_SMALL_MEM



/* SECTION: Device System */
/* Using Device System */
#define RT_USING_DEVICE
#define RT_USING_UART1
#define RT_USING_UART2
/* #define RT_USING_UART3 GPIO与I2C2的冲突 */
#define RT_USING_UART4
#define RT_USE_UART1_REMAP 0
#define RT_USE_UART2_REMAP 0

#define RT_USING_UART5

#define RT_UART1_USE_DMA 0
#define RT_UART2_USE_DMA 0
#define RT_UART3_USE_DMA 0



/* SECTION: Console options */
#define RT_USING_CONSOLE
/* the buffer size of console*/
#define RT_CONSOLEBUF_SIZE	512

/* is console needn't login? */
#define RT_NEED_FINSH_PROC_LOGIN 0

#define RT_USE_TELNET_D 0



/* SECTION: finsh, a C-Express shell */
#define RT_USING_FINSH
#define FINSH_THREAD_PRIORITY 0x5
/* Using symbol table */
#define FINSH_USING_SYMTAB
#define FINSH_USING_DESCRIPTION



/* SECTION: device filesystem */
#if 0!=RT_USING_FILESYSTEM

#define RT_USING_DFS

#define RT_USING_DFS_UFFS
#define RT_USING_DFS_ELMFAT

/* the max number of mounted filesystem */
#define DFS_FILESYSTEMS_MAX			2
/* the max number of opened files 		*/
#define DFS_FD_MAX					4

#define RT_DFS_ELM_WORD_ACCESS
/* Reentrancy (thread safe) of the FatFs module.  */
#define RT_DFS_ELM_REENTRANT
/* Number of volumes (logical drives) to be used. */
#define RT_DFS_ELM_DRIVES			2
/* #define RT_DFS_ELM_USE_LFN			1 */
#define RT_DFS_ELM_MAX_LFN			255
/* Maximum sector size to be handled. */
#define RT_DFS_ELM_MAX_SECTOR_SIZE  512
#endif



/* SECTION: lwip, a lighwight TCP/IP protocol stack */
#if 0!=RT_USING_TCPIP_STACK
#define RT_USING_LWIP

/* LwIP uses RT-Thread Memory Management */
#define RT_LWIP_USING_RT_MEM
/* Enable ICMP protocol*/
#define RT_LWIP_ICMP
/* Enable UDP protocol*/
#define RT_LWIP_UDP
/* Enable TCP protocol*/
#define RT_LWIP_TCP
/* Enable DNS */
#define RT_LWIP_DNS

#define RT_LWIP_SNMP 1

/* the number of simulatenously active TCP connections*/
#define RT_LWIP_TCP_PCB_NUM	5

/* Using DHCP */
/* #define RT_LWIP_DHCP */

/* tcp thread options */
#define RT_LWIP_TCPTHREAD_PRIORITY		12
#define RT_LWIP_TCPTHREAD_MBOX_SIZE		10
#define RT_LWIP_TCPTHREAD_STACKSIZE		1024

/* ethernet if thread options */
#define RT_LWIP_ETHTHREAD_PRIORITY		15
#define RT_LWIP_ETHTHREAD_MBOX_SIZE		10
#define RT_LWIP_ETHTHREAD_STACKSIZE		512

/* TCP sender buffer space */
#define RT_LWIP_TCP_SND_BUF	8192
/* TCP receive window. */
#define RT_LWIP_TCP_WND		8192
#endif



/* SECTION: RT-Thread/GUI */
#if 0!=RT_USING_GUI
#define RT_USING_RTGUI

/* name length of RTGUI object */
#define RTGUI_NAME_MAX		12
/* support 16 weight font */
#define RTGUI_USING_FONT16
/* support Chinese font */
#define RTGUI_USING_FONTHZ
/* use DFS as file interface */
#define RTGUI_USING_DFS_FILERW
/* use font file as Chinese font */
#define RTGUI_USING_HZ_FILE
/* use Chinese bitmap font */
#define RTGUI_USING_HZ_BMP
/* use small size in RTGUI */
#define RTGUI_USING_SMALL_SIZE
/* use mouse cursor */
/* #define RTGUI_USING_MOUSE_CURSOR */
/* default font size in RTGUI */
#define RTGUI_DEFAULT_FONT_SIZE	16

//#define RT_RTGUI_DEMO 1

/* image support */
/* #define RTGUI_IMAGE_XPM */
/* #define RTGUI_IMAGE_BMP */

#endif

//#define RT_USING_ZMODEM


#define RT_USING_PRIV_DATABASE 1

#endif
