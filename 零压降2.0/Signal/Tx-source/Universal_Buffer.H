//*****************************************************************************
//
// File Name	: 'Universal_Buffer.c'
// Title		: Multipurpose byte buffer structure and methods
// Author		: ZZJJHH250/zzjjhh250 - Copyright (C) 2001-2002
// Created		: 8/31/2010
// Revised		: 8/31/2010
// Version		: 1.0
// Target MCU	: any
// Editor Tabs	: 4
//
// This code is universal management-functions
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
typedef struct struct_cBuffer {
        unsigned char *dataptr;			///< the physical memory address where the buffer is stored
        unsigned int size;				///< the allocated size of the buffer
        unsigned int datalength;		///< the length of the data currently in the buffer
        unsigned int dataindex;			///< the index into the buffer where the data starts
} cBuffer;


/********************
*   函 数 引 用 区  *
********************/

extern void	bufferInit(volatile cBuffer* mbuffer, unsigned char *start, unsigned int size);	//! initialize a buffer to start at a given address and have given size

extern unsigned char	bufferGetFromFront(volatile cBuffer* mbuffer);	   //! get the first byte from the front of the buffer

extern void bufferDumpFromFront(volatile cBuffer* mbuffer, unsigned int numbytes); //! dump (discard) the first numbytes from the front of the buffer

extern unsigned char	bufferGetAtIndex(volatile cBuffer* mbuffer, unsigned int index); //! get a byte at the specified index in the buffer (kind of like array access)
// ** note: this does not remove the byte that was read from the buffer
extern unsigned char bufferAddToEnd(volatile cBuffer* mbuffer, unsigned char dat);  //! add a byte to the end of the buffer

extern unsigned int	bufferIsNotFull(volatile cBuffer* mbuffer);//! check if the buffer is full/not full (returns zero value if full)

extern void		bufferFlush(volatile cBuffer* mbuffer);  //! flush (clear) the contents of the buffer

extern unsigned int bufferGetSize(volatile cBuffer* mbuffer);


/********************
*   全局变量引用区  *
********************/


#endif

