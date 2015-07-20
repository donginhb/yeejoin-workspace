//*****************************************************************************
//
// File Name	: 'Universal_Buffer.h'
// Title		: Multipurpose byte buffer structure and methods
// Author		: Pascal Stang - Copyright (C) 2001-2002
// Created		: 9/23/2001
// Revised		: 11/16/2002
// Version		: 1.1
// Target MCU	: any
// Editor Tabs	: 4
//
///  Overview	:
///		This byte-buffer structure provides an easy and efficient way to store
///		and process a stream of bytes.?You can create as many buffers as you
///		like (within memory limits), and then use this common set of functions to
///		access each buffer.?The buffers are designed for FIFO operation (first
///		in, first out).?This means that the first byte you put in the buffer
///		will be the first one you get when you read out the buffer.?Supported
///		functions include buffer initialize, get byte from front of buffer, add
///		byte to end of buffer, check if buffer is full, and flush buffer.?The
///		buffer uses a circular design so no copying of data is ever necessary.
///		This buffer is not dynamically allocated, it has a user-defined fixed
///		maximum size.?This buffer is used in many places in the avrlib code.
//
// 		This code is distributed under the GNU Public License
//			which can be found at http://www.gnu.org/licenses/gpl.txt
//
//*****************************************************************************


#ifndef __UNIVERSAL_BUFFER_H
#define __UNIVERSAL_BUFFER_H


/********************
* 头 文 件 配 置 区 *
********************/

/********************
*   系 统 宏 定 义  *
********************/

/*------------------*
*   常 数 宏 定 义  *
*------------------*/

/*------------------*
*   动 作 宏 定 义  *
*------------------*/

/********************
*    结构体定义区   *
********************/
typedef struct struct_cBuffer
{
	unsigned char *dataptr;			///< the physical memory address where the buffer is stored
	unsigned int size;				///< the allocated size of the buffer
	unsigned int datalength;		///< the length of the data currently in the buffer
	unsigned int dataindex;			///< the index into the buffer where the data starts
} cBuffer;


/********************
*   函 数 引 用 区  *
********************/  

extern void	bufferInit(cBuffer* mbuffer, unsigned char *start, unsigned int size);	//! initialize a buffer to start at a given address and have given size

extern unsigned char	bufferGetFromFront(cBuffer* mbuffer);	   //! get the first byte from the front of the buffer

extern void bufferDumpFromFront(cBuffer* mbuffer, unsigned int numbytes); //! dump (discard) the first numbytes from the front of the buffer

extern unsigned char	bufferGetAtIndex(cBuffer* mbuffer, unsigned int index); //! get a byte at the specified index in the buffer (kind of like array access)
																				// ** note: this does not remove the byte that was read from the buffer
extern unsigned char bufferAddToEnd(cBuffer* mbuffer, unsigned char dat);  //! add a byte to the end of the buffer

extern unsigned int	bufferIsNotFull(cBuffer* mbuffer);//! check if the buffer is full/not full (returns zero value if full)

extern void		bufferFlush(cBuffer* mbuffer);  //! flush (clear) the contents of the buffer

extern unsigned int bufferGetSize(cBuffer* mbuffer);


/********************
*   全局变量引用区  *
********************/


#endif

