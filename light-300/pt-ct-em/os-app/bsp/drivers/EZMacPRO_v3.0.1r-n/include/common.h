/*
 * common.h
 *
 *  modify by David, zhaoshaowei@yeejoin.com, 2013-11-25
 *
 * description:
 *
 */
#ifndef COMMON_H_
#define COMMON_H_

#include "EZMacPro_cfg.h"

#if C51_SYNTAX_

#define REGISTER_MSPACE                 SEG_IDATA
//#define REGISTER_MSPACE               SEG_DATA
#define BUFFER_MSPACE                   SEG_XDATA
#define FORWARDED_PACKET_TABLE_MSPACE   SEG_XDATA
#define TEST_CODE_MSPACE                SEG_XDATA
#define EZMAC_PRO_GLOBAL_MSPACE         SEG_DATA
#define APPLICATION_MSPACE              SEG_XDATA

#else

#define REGISTER_MSPACE
//#define REGISTER_MSPACE
#define BUFFER_MSPACE
#define FORWARDED_PACKET_TABLE_MSPACE
#define TEST_CODE_MSPACE
#define EZMAC_PRO_GLOBAL_MSPACE
#define APPLICATION_MSPACE

#endif /* C51_SYNTAX_ */
/*!
 * Software Development Board.
 */
#ifdef SDBC
#define MCU_F930
#define SOFTWARE_DEVELOPMENT_BOARD
#define SPI_ENABLED
#define TIMER_ENABLED
#define DOG_LCD_ENABLED
#endif //SDBC


/*!
 * EZLink module.
 */
#ifdef EZLINK
#define MCU_F930
#define EZLINK_MODULE
#define SPI_ENABLED
#define TIMER_ENABLED
#endif //SDBC


/*!
 * Si1000 on an Si1000MB module.
 */
#ifdef SI1000MB_SI1000
#define MCM_SI1000
#define SI1000_DAUGHTERCARD_SI1000_MOTHERBOARD
#define SPI_ENABLED
#define TIMER_ENABLED
#endif //SDBC


/*!
 * Si1010 on an Si1000MB module.
 */
#ifdef SI1000MB_SI1010
#define MCM_SI1010
#define SI1010_DAUGHTERCARD_SI1000_MOTHERBOARD
#define SPI_ENABLED
#define TIMER_ENABLED
#endif //SDBC


/*!
 * Enable trace through UART.
 */
#ifdef TRACE_ENABLED
#define UART0_ENABLED
#endif //TRACE_ENABLED


#include "compiler_defs.h"
//#include "bsp\bsp.h"
#include "stack.h"

/* add by David */
#include <si4432_hardif.h>
#include <spi4ezmacpro.h>
#include <timer4ezmacpro.h>
#include <si4432_v2.h>
#include <si443x_const_b1.h>

#ifndef unuse_variable
#define unuse_variable(var) (void)(var)
#endif


#if 1
#include <rtthread.h>

#define ezmacpro_debug(x)	printf_syn x
#define ezmacpro_debug_k(x)	rt_kprintf x
#endif



#endif
