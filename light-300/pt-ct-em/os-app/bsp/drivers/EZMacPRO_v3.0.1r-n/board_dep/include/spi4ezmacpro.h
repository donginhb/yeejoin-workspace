/*
 * spi4ezmacpro.h
 *
 *  Created on: 2013-11-28
 *      Author: David, zhaoshaowei@yeejoin.com
 */

#ifndef SPI4EZMACPRO_H_
#define SPI4EZMACPRO_H_

#include <compiler_defs.h>

#ifdef USE_MACRO_FUN_IN_SPI4EZMACPRO
#undef USE_MACRO_FUN_IN_SPI4EZMACPRO
#endif

#define USE_MACRO_FUN_IN_SPI4EZMACPRO 1



extern void macSpiWriteReg(U8, U8);
extern U8   macSpiReadReg(U8);
extern void macSpiWriteFIFO(U8, VARIABLE_SEGMENT_POINTER(payload, U8, BUFFER_MSPACE));
extern void macSpiReadFIFO(U8, VARIABLE_SEGMENT_POINTER(payload, U8, BUFFER_MSPACE));

extern void send_byte_to_si4432 (U8 reg, U8 value);
extern U8 read_byte_from_si4432(U8 reg);
extern void send_data_to_si4432_fifo(U8 n, VARIABLE_SEGMENT_POINTER(buffer, U8, BUFFER_MSPACE));
extern void read_data_from_si4432_fifo(U8 n, VARIABLE_SEGMENT_POINTER(buffer, U8, BUFFER_MSPACE));

#if USE_MACRO_FUN_IN_SPI4EZMACPRO
#define extIntSpiWriteReg(macReg, value) 	send_byte_to_si4432(macReg, value)
#define extIntSpiReadReg(macReg)		read_byte_from_si4432(macReg)
#define extIntSpiWriteFIFO(n, buffer)		send_data_to_si4432_fifo(n, buffer)
#define extIntSpiReadFIFO(n, buffer)		read_data_from_si4432_fifo(n, buffer)

#define timerIntSpiWriteReg(macReg, value)	send_byte_to_si4432(macReg, value)
#define timerIntSpiReadReg(macReg)		read_byte_from_si4432(macReg)
#else
extern void extIntSpiWriteReg (U8, U8);
extern U8   extIntSpiReadReg (U8);
extern void extIntSpiWriteFIFO (U8, VARIABLE_SEGMENT_POINTER(payload, U8, BUFFER_MSPACE));
extern void extIntSpiReadFIFO (U8, VARIABLE_SEGMENT_POINTER(payload, U8, BUFFER_MSPACE));

extern void timerIntSpiWriteReg (U8, U8);
extern U8   timerIntSpiReadReg (U8);
#endif

#endif /* SPI4EZMACPRO_H_ */
