/*
 ******************************************************************************
 * auto_negoti_timer.h
 *
 *  Created on: 2015-01-31
 *      Author: David, zhaoshaowei@yeejoin.com
 *
 * COPYRIGHT (C) 2015, YEEJOIN (Beijing) Technology Development Co., Ltd.
 ******************************************************************************
 */

#ifndef OS_APP_APP_INCLUDE_AUTO_NEGOTI_TIMER_H_
#define OS_APP_APP_INCLUDE_AUTO_NEGOTI_TIMER_H_

#include <app-hw-resource.h>

extern void init_auto_negoti_timer(uint16_t tim_period);
extern void start_auto_negoti_timer(void);
extern void stop_auto_negoti_timer(void);
extern void reset_auto_negoti_timer(void);

extern void cfg_auto_negoti_timer_nvic(void);
extern void enable_auto_negoti_timer_int(void);

#endif /* OS_APP_APP_INCLUDE_AUTO_NEGOTI_TIMER_H_ */
