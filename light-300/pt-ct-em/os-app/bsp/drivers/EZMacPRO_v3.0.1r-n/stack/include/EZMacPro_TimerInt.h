/*!\file EZMacPro_TimerInt.h
 * \brief Header of EZMacPro_TimerInt.c.
 *
 * \n EZMacPRO version: 3.0.1r
 *
 * \n This software must be used in accordance with the End User License
 * \n Agreement.
 *
 * \b COPYRIGHT
 * \n Copyright 2012 Silicon Laboratories, Inc.
 * \n http://www.silabs.com
 */

#ifndef _EZMACPRO_TIMERINT_H_
#define _EZMACPRO_TIMERINT_H_

#include "EZMacPro_cfg.h"

/* ======================================= *
 *          D E F I N I T I O N S          *
 * ======================================= */

typedef struct PacketFiltersBytes {
	U8 rcid;
	U8 rsid;
	U8 rdid;
	U8 packetlength;
} PacketFiltersBytes;


/* ======================================= *
 *     G L O B A L   V A R I A B L E S     *
 * ======================================= */

/* ======================================= *
 *  F U N C T I O N   P R O T O T Y P E S  *
 * ======================================= */

INTERRUPT(timerIntT3_ISR, INTERRUPT_TIMER3);

#endif //_EZMACPRO_TIMERINT_H_
