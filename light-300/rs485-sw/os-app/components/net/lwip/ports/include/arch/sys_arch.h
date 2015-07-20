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
#ifndef __SYS_RTXC_H__
#define __SYS_RTXC_H__



/*
 * SYS_MBOX_NULL, SYS_SEM_NULL, almost always set equal to NULL.
 * These define what a failure to create the system type will return
 */
#define SYS_MBOX_NULL (rt_mailbox_t)0
#define SYS_SEM_NULL  (rt_sem_t)0


/*
 * sys_prot_t sys_arch_protect(void);
 * void sys_arch_unprotect(sys_prot_t pval);
 * Define the sys_prot_t type to reflect what these two functions need to operate on.
 *
 */
typedef rt_base_t sys_prot_t;

/*
 * sys_sem_t, sys_mbox_t, sys_thread_t all refer to the appropriate types,
 * whether native OS versions or wrappers used to make them conform to what lwIP expects.
 * Usualy these are typedefed directly to the systems corresponding types
 * but if wrapper functions are used these may be just about anything.
 * References to a list of pre-alloced objects is but one example.
 */
typedef rt_sem_t     sys_sem_t;
typedef rt_mailbox_t sys_mbox_t;
typedef rt_thread_t  sys_thread_t;


/*
 * --sys_arch.h is also where sys_sem_valid(sem), sys_sem_set_valid(sem), sys_mbox_valid(mbox) and
 * sys_mbox_set_valid(mbox) macros are created.
 * --sys_sem_valid(sem) verifies that a semaphore is valid and can be used.
 * --sys_sem_set_invalid(sem) sets a semaphore to a value that will be interpreted as invalid.
 * lwIP uses this macro to make sure an old reference can't be reused.
 * --sys_mbox_valid(mbox) and sys_mbox_set_invalid(mbox) works precisely as their similes
 * for semaphores but as creation may differ between mailboxes and semaphores they have their own criteria.
 */
#if 0
#ifndef sys_sem_valid
/** Check if a sempahore is valid/allocated: return 1 for valid, 0 for invalid */
int sys_sem_valid(sys_sem_t *sem);
#endif
#ifndef sys_sem_set_invalid
/** Set a semaphore invalid so that sys_sem_valid returns 0 */
void sys_sem_set_invalid(sys_sem_t *sem);
#endif


#ifndef sys_mbox_valid
/** Check if an mbox is valid/allocated: return 1 for valid, 0 for invalid */
int sys_mbox_valid(sys_mbox_t *mbox);
#endif
#ifndef sys_mbox_set_invalid
/** Set an mbox invalid so that sys_mbox_valid returns 0 */
void sys_mbox_set_invalid(sys_mbox_t *mbox);
#endif
#endif
#define sys_sem_valid(sem) (((sem) != NULL) && (*(sem) != NULL))
#define sys_sem_set_invalid(sem) do { if((sem) != NULL) { *(sem) = NULL; }}while(0)

/* sys_mbox_valid调用时, 都传递一个变量的地址, 所以不用检查((mbox) != NULL) */
#define sys_mbox_valid(mbox) ((*(mbox) != NULL))
#define sys_mbox_set_invalid(mbox) do { if((mbox) != NULL) { *(mbox) = NULL; }}while(0)


/*
 * Define LWIP_COMPAT_MUTEX if the port has no mutexes and binary semaphores
 * should be used instead
 * 可以进行优化!!!!!
 */
#define LWIP_COMPAT_MUTEX 1




/* IPC constants. */
#define archMESG_QUEUE_LENGTH	( 6 )
#define archPOST_BLOCK_TIME_MS	( ( unsigned long ) 10000 )

#define archMAILBOX_SIZE_MAX  (6)

#endif /* __SYS_RTXC_H__ */

