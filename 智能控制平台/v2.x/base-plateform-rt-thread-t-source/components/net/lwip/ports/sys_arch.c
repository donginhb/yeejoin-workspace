/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/*
 * 写该文件主要参考资料:
 * lwip-1.4.0\doc\sys_arch.txt
 * http://lwip.wikia.com/wiki/Porting_for_an_OS_1.4.0#sys_arch.c
 *
 * There has been some changes in this layer since 1.3.2, most prominently the 
 * functions returns an error value and created objects are returned via a pointer
 * provided as an argument to the function.
 *
 * NOTE!!
 * Be careful with using mem_malloc() in sys_arch. Specifically, when mem_init()
 * is run (in mem.c), it allocates a semaphore to block concurrent access in its routines.
 * If sys_sem_new() calls mem_malloc to allocate its semaphore, then it will be calling mem_malloc()
 * before the mem.c system has been initialized!
 *
 */
 
#include <rthw.h>
#include <rtthread.h>
#include <rtdef.h>

/* lwIP includes. */
#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/sys.h"
#include "lwip/mem.h"
#include "lwip/stats.h"
#include "lwip/tcpip.h"
#include "lwip/netifapi.h"

#include "netif/stm32netif.h"

#include <syscfgdata.h>
#include <misc_lib.h>

struct timeoutlist
{
	//struct sys_timeouts timeouts;
	struct rt_thread *tid; //xTaskHandle pid;	
};

/* This is the number of threads that can be started with sys_thread_new() */
#define SYS_THREAD_MAX 4

static struct timeoutlist s_timeoutlist[SYS_THREAD_MAX];
static u16_t s_nextthread = 0;


static int mb_cnt  = -1;
static int sem_cnt = -1;

#if 0
#define SYS_ARCH_DEBUG(x) printf_syn x
#else
#define SYS_ARCH_DEBUG(x)
#endif
#define SYS_ARCH_LOG(x)   printf_syn x

/*-----------------------------------------------------------------------------------*/
/** Create a new mbox of specified size
 * @param mbox pointer to the mbox to create
 * @param size (miminum) number of messages in this mbox
 * @return ERR_OK if successful, another err_t otherwise */

/* David: lwip-1.4.0\doc\sys_arch.txt中对该函数的说明没有更新, sys.h中有该函数原型, 2011-7-5 */
err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
	char name[12];

	//LWIP_UNUSED_ARG(size);
	get_ipc_id_name(name, "lwip", &mb_cnt);
	if (NULL != (*mbox = rt_mb_create(name, size, RT_IPC_FLAG_FIFO))) {
#if SYS_STATS
        ++lwip_stats.sys.mbox.used;
        if (lwip_stats.sys.mbox.max < lwip_stats.sys.mbox.used) {
           lwip_stats.sys.mbox.max = lwip_stats.sys.mbox.used;
	    }
#endif /* SYS_STATS */
	    return ERR_OK;
	} else {
#if SYS_STATS
        ++lwip_stats.sys.mbox.err;
#endif /* SYS_STATS */
		*mbox = NULL;
	    return ERR_MEM;
	}
}

/*-----------------------------------------------------------------------------------*/
/*
  Deallocates a mailbox. If there are messages still present in the
  mailbox when the mailbox is deallocated, it is an indication of a
  programming error in lwIP and the developer should be notified.
*/
/** Delete an mbox
 * @param mbox mbox to delete
 *
 * create: David, 2012-02-09
 */
/* David, lwip-1.4.0\doc\sys_arch.txt中对该函数的说明没有更新, sys.h中有该函数原型 */
void sys_mbox_free(sys_mbox_t *mbox)
{
#if SYS_STATS
     --lwip_stats.sys.mbox.used;
#endif /* SYS_STATS */

	if (0 != (*mbox)->entry) {
		/* there are messages still present in the mailbox */
#if SYS_STATS
	    lwip_stats.sys.mbox.err++;
#endif /* SYS_STATS */
		// TODO notify the user of failure.
	}

	rt_mb_delete(*mbox);
}

/*-----------------------------------------------------------------------------------*/
// Posts the "msg" to the mailbox. This function have to block until the "msg" is really posted.
/** Post a message to an mbox - may not fail
 * -> blocks if full, only used from tasks not from ISR
 * @param mbox mbox to posts the message
 * @param msg message to post (ATTENTION: can be NULL)
 * create: David, 2012-02-10
 */
/* David, lwip-1.4.0\doc\sys_arch.txt中对该函数的说明没有更新, sys.h中有该函数原型 */
void sys_mbox_post(sys_mbox_t *mbox, void *msg)
{
	if (rt_mb_send_wait(*mbox, (rt_uint32_t)msg, RT_WAITING_FOREVER) != RT_EOK)
		RT_DEBUG_LOG(RT_DEBUG_IPC, ("sys_mbox_post fail. thread:%s\n", (rt_thread_self())->name));

	return;	
}


/*-----------------------------------------------------------------------------------*/
//   Try to post the "msg" to the mailbox.Returns ERR_MEM if this one is full, else, ERR_OK if the "msg" is posted.
/** Try to post a message to an mbox - may fail if full or ISR
 * @param mbox mbox to posts the message
 * @param msg message to post (ATTENTION: can be NULL)
 * creat: David, 2012-02-10 15:12:18
 */
/* David, lwip-1.4.0\doc\sys_arch.txt中对该函数的说明没有更新, sys.h中有该函数原型 */
err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
    err_t result;

    if (RT_EOK == rt_mb_send(*mbox, (rt_uint32_t)msg)) {
        result = ERR_OK;
    } else { // could not post, queue must be full
        result = ERR_MEM;
#if SYS_STATS
        lwip_stats.sys.mbox.err++;
#endif /* SYS_STATS */
    }

    return result;
}

/*-----------------------------------------------------------------------------------*/
/*
  Blocks the thread until a message arrives in the mailbox, but does
  not block the thread longer than "timeout" milliseconds (similar to
  the sys_arch_sem_wait() function). The "msg" argument is a result
  parameter that is set by the function (i.e., by doing "*msg =
  ptr"). The "msg" parameter maybe NULL to indicate that the message
  should be dropped.

  The return values are the same as for the sys_arch_sem_wait() function:
  Number of milliseconds spent waiting or SYS_ARCH_TIMEOUT if there was a
  timeout.

  Note that a function with a similar name, sys_mbox_fetch(), is
  implemented by lwIP.
*/
/** Wait for a new message to arrive in the mbox
 * @param mbox mbox to get a message from
 * @param msg pointer where the message is stored
 * @param timeout maximum time (in milliseconds) to wait for a message
 * @return time (in milliseconds) waited for a message, may be 0 if not waited
           or SYS_ARCH_TIMEOUT on timeout
 *         The returned time has to be accurate to prevent timer jitter!
 * creat: David, 2012-02-10 15:21:31
 */
/* David, lwip-1.4.0\doc\sys_arch.txt中对该函数的说明没有更新, sys.h中有该函数原型 */
u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
	void *dummyptr;
	rt_tick_t start, end;
	rt_uint32_t elapsed;

	if (NULL == msg) {
		msg = &dummyptr;
	}
		
	start = rt_tick_get();
	if (0 != timeout) {
		if (RT_EOK == rt_mb_recv(*mbox, (rt_uint32_t*)(msg), get_ticks_of_ms(timeout))) { /* David, (rt_uint32_t*)(*msg) */
			end = rt_tick_get();
			elapsed = get_ms_of_ticks(end - start);
			return ( elapsed );
		} else { // timed out blocking for message
			*msg = NULL;
			return SYS_ARCH_TIMEOUT;
		}
	} else { // block forever for a message.
		if (RT_EOK != rt_mb_recv(*mbox, (rt_uint32_t*)(msg), RT_WAITING_FOREVER)) // time is arbitrary
		    RT_DEBUG_LOG(RT_DEBUG_IPC, ("sys_arch_mbox_fetch fail. thread:%s\n", (rt_thread_self())->name));
		end = rt_tick_get();
		elapsed = get_ms_of_ticks(end - start);

		return elapsed; // return time blocked TODO test	
	}
}

/*-----------------------------------------------------------------------------------*/
/*
  Similar to sys_arch_mbox_fetch, but if message is not ready immediately, we'll
  return with SYS_MBOX_EMPTY.  On success, 0 is returned.
*/
/* Allow port to override with a macro, e.g. special timout for sys_arch_mbox_fetch() */
/** Wait for a new message to arrive in the mbox
 * @param mbox mbox to get a message from
 * @param msg pointer where the message is stored
 * @param timeout maximum time (in milliseconds) to wait for a message
 * @return 0 (milliseconds) if a message has been received
 *         or SYS_MBOX_EMPTY if the mailbox is empty
 * creat: David, 2012-02-10 15:49:29
 */
/* David, lwip-1.4.0\doc\sys_arch.txt中对该函数的说明没有更新, sys.h中有该函数原型 */
u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
    void *dummyptr;

	if (NULL == msg) {
		msg = &dummyptr;
	}

    if (RT_EOK == rt_mb_recv(*mbox, (rt_uint32_t*)(msg), 0))
        return ERR_OK;
    else
        return SYS_MBOX_EMPTY;
}



/* Semaphore functions: */

/*-----------------------------------------------------------------------------------*/
//  Creates and returns a new semaphore. The "count" argument specifies
//  the initial state of the semaphore.
/** Create a new semaphore
 * @param sem pointer to the semaphore to create
 * @param count initial count of the semaphore
 * @return ERR_OK if successful, another err_t otherwise 
 * Creat: David, 2012-02-10 15:50:38, LwIP代码使用sys_sem_new()时, count的可能取值只是0, 1
 */
/* David, lwip-1.4.0\doc\sys_arch.txt中对该函数的说明没有更新, sys.h中有该函数原型 */
err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
    char name[12];

	get_ipc_id_name(name, "lwip", &sem_cnt);
	if (NULL != (*sem = rt_sem_create(name, count, RT_IPC_FLAG_FIFO))) {
#if SYS_STATS
    	++lwip_stats.sys.sem.used;
     	if (lwip_stats.sys.sem.max < lwip_stats.sys.sem.used) {
    		lwip_stats.sys.sem.max = lwip_stats.sys.sem.used;
    	}
#endif /* SYS_STATS */
	    return ERR_OK;
	} else {
#if SYS_STATS
        ++lwip_stats.sys.sem.err;
#endif /* SYS_STATS */
		*sem = NULL;
	    return ERR_MEM;
	}

	return ERR_OK;
}

/*-----------------------------------------------------------------------------------*/
/*
  Blocks the thread while waiting for the semaphore to be
  signaled. If the "timeout" argument is non-zero, the thread should
  only be blocked for the specified time (measured in milliseconds).

  If the timeout argument is non-zero, the return value is the number of
  milliseconds spent waiting for the semaphore to be signaled. If the
  semaphore wasn't signaled within the specified time, the return value is
  SYS_ARCH_TIMEOUT. If the thread didn't have to wait for the semaphore
  (i.e., it was already signaled), the function may return zero.

  Notice that lwIP implements a function with a similar name,
  sys_sem_wait(), that uses the sys_arch_sem_wait() function.
*/
/** Wait for a semaphore for the specified timeout
 * @param sem the semaphore to wait for
 * @param timeout timeout in milliseconds to wait (0 = wait forever)
 * @return time (in milliseconds) waited for the semaphore
 *         or SYS_ARCH_TIMEOUT on timeout
 * creat: David, 2012-02-10 16:11:43
 *      lwip-1.4.0\doc\sys_arch.txt中对该函数的说明没有更新, sys.h中有该函数原型
 */
u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
    rt_tick_t   start, end;
    rt_uint32_t elapsed;

	start = rt_tick_get();

	if (timeout != 0) {
		if (RT_EOK == rt_sem_take(*sem, get_ticks_of_ms(timeout))) {
		    goto succ_out;
		} else {
			return SYS_ARCH_TIMEOUT;
		}
	} else { // must block without a timeout
		if (RT_EOK != rt_sem_take(*sem, RT_WAITING_FOREVER)) {
		    RT_DEBUG_LOG(RT_DEBUG_IPC, ("sys_arch_sem_wait fail. thread:%s\n", (rt_thread_self())->name));
		}
		goto succ_out;
	}

succ_out:
	end = rt_tick_get();
	elapsed = get_ms_of_ticks(end - start);
	return elapsed; // return time blocked TODO test	
}

/*-----------------------------------------------------------------------------------*/
// Signals a semaphore
/** Signals a semaphore
 * @param sem the semaphore to signal
 *
 * creat: David, 2012-02-10 16:20:16
 *      lwip-1.4.0\doc\sys_arch.txt中对该函数的说明没有更新, sys.h中有该函数原型
 */
void sys_sem_signal(sys_sem_t *sem)
{
	rt_sem_release(*sem);
}

/*-----------------------------------------------------------------------------------*/
// Deallocates a semaphore
/** Delete a semaphore
 * @param sem semaphore to delete
 *
 * creat: David, 2012-02-10 16:20:50
 *      lwip-1.4.0\doc\sys_arch.txt中对该函数的说明没有更新, sys.h中有该函数原型
 */
void sys_sem_free(sys_sem_t *sem)
{
#if SYS_STATS
      --lwip_stats.sys.sem.used;
#endif /* SYS_STATS */
			
	rt_sem_delete(*sem);
}

#if 0 /* LWIP_COMPAT_MUTEX */
/** Create a new mutex
 * @param mutex pointer to the mutex to create
 * @return a new mutex */
err_t sys_mutex_new(sys_mutex_t *mutex);
/** Lock a mutex
 * @param mutex the mutex to lock */
void sys_mutex_lock(sys_mutex_t *mutex);
/** Unlock a mutex
 * @param mutex the mutex to unlock */
void sys_mutex_unlock(sys_mutex_t *mutex);
/** Delete a semaphore
 * @param mutex the mutex to delete */
void sys_mutex_free(sys_mutex_t *mutex); 

#ifndef sys_mutex_valid
/** Check if a mutex is valid/allocated: return 1 for valid, 0 for invalid */
int sys_mutex_valid(sys_mutex_t *mutex);
#endif

#ifndef sys_mutex_set_invalid
/** Set a mutex invalid so that sys_mutex_valid returns 0 */
void sys_mutex_set_invalid(sys_mutex_t *mutex);
#endif
#endif


/*-----------------------------------------------------------------------------------*/
// Initialize sys arch, Is called to initialize the sys_arch layer.
/*
 * void sys_init(void). lwIP system initialization. This function is called before
 * the any other sys_arch-function is called and is meant to be used to initialize
 * anything that has to be up and running for the rest of the functions to work.
 * for example to set up a pool of semaphores.
 */
/* sys_init() must be called before anthing else. */
void sys_init(void)
{
	int i;

	// Initialize the the per-thread sys_timeouts structures
	// make sure there are no valid pids in the list
	for (i = 0; i < SYS_THREAD_MAX; i++) {
		s_timeoutlist[i].tid = 0;
		//s_timeoutlist[i].timeouts.next = NULL;
	}

	// keep track of how many threads have been created
	s_nextthread = 0;
}




/*-----------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
// TODO
/*-----------------------------------------------------------------------------------*/
/*
  Starts a new thread with priority "prio" that will begin its execution in the
  function "thread()". The "arg" argument will be passed as an argument to the
  thread() function. The id of the new thread is returned. Both the id and
  the priority are system dependent.
*/
/** The only thread function:
 * Creates a new thread
 * @param name human-readable name for the thread (used for debugging purposes)
 * @param thread thread-function
 * @param arg parameter passed to 'thread'
 * @param stacksize stack size in bytes for the new thread (may be ignored by ports)
 * @param prio priority of the new thread (may be ignored by ports)
 *
 * creat: David, 2012-02-10 16:21:52
 *      lwip-1.4.0\doc\sys_arch.txt中对该函数的说明没有更新, sys.h中有该函数原型
 */

sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread, void *arg, int stacksize, int prio)
{
    rt_thread_t th;

    SYS_ARCH_DEBUG(("name:%s, stack size:%d, pri:%d, s_nextthread:%d.\n", name, stacksize, prio, s_nextthread));
    if (s_nextthread < SYS_THREAD_MAX) {
        th = rt_thread_create(name, thread, arg, stacksize, prio, 16);
        if (NULL != th) {
    	    /* For each task created, store the task handle (pid) in the timers array.
    	       This scheme doesn't allow for threads to be deleted */
    	    s_timeoutlist[s_nextthread++].tid = th;

	        SYS_ARCH_DEBUG(("lwip creat thread(%s) succ! \n",name));
	        
        	/* startup thread */
        	rt_thread_startup(th);
        	
		    return th;
	    } else {
	        SYS_ARCH_LOG(("lwip creat thread(%s) fail! \n",name));
	 	   return NULL;
	    }
    } else {
        SYS_ARCH_LOG(("SYS_THREAD_MAX value is %d!!!\n", SYS_THREAD_MAX));
        return NULL;
    }
}

/*
  This optional function does a "fast" critical region protection and returns
  the previous protection level. This function is only called during very short
  critical regions. An embedded system which supports ISR-based drivers might
  want to implement this function by disabling interrupts. Task-based systems
  might want to implement this by using a mutex or disabling tasking. This
  function should support recursive calls from the same task or interrupt. In
  other words, sys_arch_protect() could be called while already protected. In
  that case the return value indicates that it is already protected.

  sys_arch_protect() is only required if your port is supporting an operating
  system.
*/
sys_prot_t sys_arch_protect(void)
{
	rt_base_t level;
	/* disable interrupt */
	level = rt_hw_interrupt_disable();
	/* must also lock scheduler */
	rt_enter_critical();	
	return level;
}

/*
  This optional function does a "fast" set of critical region protection to the
  value specified by pval. See the documentation for sys_arch_protect() for
  more information. This function is only required if your port is supporting
  an operating system.
*/
void sys_arch_unprotect(sys_prot_t pval)
{
	/* unlock scheduler */	
	rt_exit_critical();	
	/* enable interrupt */	
	rt_hw_interrupt_enable(pval);	
	return;
}

#if 0
/* Time functions. */
#ifndef sys_msleep
void sys_msleep(u32_t ms); /* only has a (close to) 1 jiffy resolution. */
#endif

#ifndef sys_jiffies
/** Ticks/jiffies since power up. */
u32_t sys_jiffies(void);
#endif

/** Returns the current time in milliseconds,
 * may be the same as sys_jiffies or at least based on it. */
u32_t sys_now(void)
{
    return get_ms_of_ticks(rt_tick_get());
}

/*
 * Prints an assertion messages and aborts execution.
 */
void sys_assert( const char *msg )
{	
	( void ) msg;
	/*FSL:only needed for debugging
	printf(msg);
	printf("\n\r");
	*/
    vPortEnterCritical(  );
    for(;;)
    ;
}
#endif

/** Returns the current time in milliseconds,
 * may be the same as sys_jiffies or at least based on it. */
u32_t sys_now(void)
{
    return get_ms_of_ticks(rt_tick_get());
}


#if 0 /* sys_arch_timeouts() is not needed any more */
  ++ Bugfixes:

  2011-04-20: Simon Goldschmidt
  * sys_arch.txt: sys_arch_timeouts() is not needed any more.

lwIP timeouts has been removed since 1.3.2, the functionality still exists but
has been moved out of the porting layer and into lwIP itself to make porting
easier and more straightforward.
/*-----------------------------------------------------------------------------------*/
/*
  Returns a pointer to the per-thread sys_timeouts structure. In lwIP,
  each thread has a list of timeouts which is represented as a linked
  list of sys_timeout structures. The sys_timeouts structure holds a
  pointer to a linked list of timeouts. This function is called by
  the lwIP timeout scheduler and must not return a NULL value.

  In a single threaded sys_arch implementation, this function will
  simply return a pointer to a global sys_timeouts variable stored in
  the sys_arch module.
*/
struct sys_timeouts *sys_arch_timeouts(void)
{
int i;
xTaskHandle pid;
struct timeoutlist *tl;

	pid = xTaskGetCurrentTaskHandle( );

	for(i = 0; i < s_nextthread; i++)
	{
		tl = &(s_timeoutlist[i]);
		if(tl->pid == pid)
		{
			return &(tl->timeouts);
		}
	}

	// Error
	return NULL;
}
#endif


#include <enc28j60_io.h>


/*
 * lwip初始化
 */

/* introduce from kservice.c */
#define rt_list_entry(node, type, member) \
    ((type *)((char *)(node) - (unsigned long)(&((type *)0)->member)))

static err_t netif_device_init(struct netif *netif)
{
	struct eth_device *ethif;

	ethif = (struct eth_device*)netif->state;
	if (ethif != RT_NULL)
	{
		rt_device_t device;
		
		/* get device object */
		device = (rt_device_t) ethif;
		if (rt_device_init(device) != RT_EOK)
		{
			return ERR_IF;
		}
		return ERR_OK;
	}

	return ERR_IF;
}

static void tcpip_init_done_callback(void *arg)
{
	rt_device_t device;
	struct eth_device *ethif;
	struct ip_addr ipaddr, netmask, gw;
	struct rt_list_node* node;
	struct rt_object* object;
	struct rt_object_information *information;

	extern struct rt_object_information rt_object_container[];

	LWIP_ASSERT("invalid arg.\n",arg);

	IP4_ADDR(&gw, 0,0,0,0);
	IP4_ADDR(&ipaddr, 0,0,0,0);
	IP4_ADDR(&netmask, 0,0,0,0);

	/* enter critical */
	rt_enter_critical();

	/* for each network interfaces */
	information = &rt_object_container[RT_Object_Class_Device];
	for (node = information->object_list.next; node != &(information->object_list); node = node->next)
	{
		object = rt_list_entry(node, struct rt_object, list);
		device = (rt_device_t) object;
		if (device->type == RT_Device_Class_NetIf)
		{
			ethif = (struct eth_device*)device;

			/* leave critical */
			rt_exit_critical();

			netif_add(ethif->netif, &ipaddr, &netmask, &gw, 
				ethif, netif_device_init, tcpip_input);

			if (netif_default == RT_NULL)
				netif_set_default(ethif->netif);

#if LWIP_DHCP
			dhcp_start(ethif->netif);
#else
			netif_set_up(ethif->netif);
#endif

#ifdef LWIP_NETIF_LINK_CALLBACK
			netif_set_link_up(ethif->netif);
#endif

			/* enter critical */
			rt_enter_critical();
		}
	}

	/* leave critical */
	rt_exit_critical();

	rt_sem_release((rt_sem_t)arg);

	enc28j60_nvic_cfg(ENABLE);
	enc28j60_write(EIR, 0);
	enc28j60_set_bits(ECON1, (1 << ECON1_RXEN));
	/* enable interrupt, David  */
	enc28j60_set_bits(EIE, (1 << EIE_INTIE) | (1<<EIE_PKTIE) | (1<<EIE_TXIE));
}




/**
 * LwIP system initialization
 */
void lwip_system_init(void)
{
	rt_err_t rc;
	struct rt_semaphore done_sem;
	struct ip_param ipcfg;
	
	rc = rt_sem_init(&done_sem, "done", 0, RT_IPC_FLAG_FIFO);
	
	if(rc != RT_EOK)
	{
    	LWIP_ASSERT("Failed to create semaphore", 0);
		return;
	}

	tcpip_init(tcpip_init_done_callback,(void *)&done_sem);

	/* waiting for initialization done */
	if (rt_sem_take(&done_sem, RT_WAITING_FOREVER) != RT_EOK)
	{
		rt_sem_detach(&done_sem);
		return;
	}
	rt_sem_detach(&done_sem);

	/* set default ip address */
#if !LWIP_DHCP
	{
		struct ip_addr ipaddr, netmask, gw;

		read_syscfgdata_tbl(SYSCFGDATA_TBL_IPCFG, 0, &ipcfg);
#if 0
		IP4_ADDR(&ipaddr, RT_LWIP_IPADDR0, RT_LWIP_IPADDR1, RT_LWIP_IPADDR2, RT_LWIP_IPADDR3);
		IP4_ADDR(&gw, RT_LWIP_GWADDR0, RT_LWIP_GWADDR1, RT_LWIP_GWADDR2, RT_LWIP_GWADDR3);
		IP4_ADDR(&netmask, RT_LWIP_MSKADDR0, RT_LWIP_MSKADDR1, RT_LWIP_MSKADDR2, RT_LWIP_MSKADDR3);
#else
		ipaddr.addr 	= htonl(ipcfg.ipaddr.addr);
		gw.addr 	= htonl(ipcfg.gw.addr);
		netmask.addr 	= htonl(ipcfg.netmask.addr);
#endif
		
		netifapi_netif_set_addr(netif_default, &ipaddr, &netmask, &gw);
		clr_bit(netif_default->flags, NETIF_FLAG_DHCP);
	}

#else
	/* 未测试 */
	{
		struct ip_addr ipaddr, netmask, gw;

		read_syscfgdata_tbl(SYSCFGDATA_TBL_IPCFG, 0, &ipcfg);
		if (IPADDR_USE_STATIC == ipcfg.ipmode) {
			ipaddr.addr 	= htonl(ipcfg.ipaddr.addr);
			gw.addr 	= htonl(ipcfg.gw.addr);
			netmask.addr 	= htonl(ipcfg.netmask.addr);
			clr_bit(netif_default->flags, NETIF_FLAG_DHCP);
		} else if (IPADDR_USE_DHCP == ipcfg.ipmode) {
			set_bit(netif_default->flags, NETIF_FLAG_DHCP);
		}

		netifapi_netif_set_addr(netif_default, &ipaddr, &netmask, &gw);
	}
#endif

	return;
}

