/*
 * timer4ezmacpro.h
 *
 *  Created on: 2013-11-28
 *      Author: David, zhaoshaowei@yeejoin.com
 */

#ifndef TIMER4EZMACPRO_H_
#define TIMER4EZMACPRO_H_

#include <compiler_defs.h>

#ifdef USE_MACRO_FUN_IN_TIMER4EZMACPRO
#undef USE_MACRO_FUN_IN_TIMER4EZMACPRO
#endif

#define USE_MACRO_FUN_IN_TIMER4EZMACPRO 1

extern SEGMENT_VARIABLE(EZMacProTimerMSB, U16, EZMAC_PRO_GLOBAL_MSPACE);
extern SEGMENT_VARIABLE(EZMacProTimerLSB, U16,  EZMAC_PRO_GLOBAL_MSPACE);

extern SEGMENT_VARIABLE(is_mac_timer_inuse, U8, EZMAC_PRO_GLOBAL_MSPACE);

extern void macTimeout (U32);
extern void set_timeout_and_start_timer(U32 longTime);

#if USE_MACRO_FUN_IN_TIMER4EZMACPRO
#define extIntTimeout(time)	set_timeout_and_start_timer(time)
#define timerIntTimeout(time)	set_timeout_and_start_timer(time)
#else
void extIntTimeout (U32);
void timerIntTimeout (U32);
#endif


#endif /* TIMER4EZMACPRO_H_ */
