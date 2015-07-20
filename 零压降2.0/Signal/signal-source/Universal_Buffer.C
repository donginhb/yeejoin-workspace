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

/********************
* 头 文 件 配 置 区 *
********************/
#include "Universal_Buffer.h"

/*------------------*
*   常 数 宏 定 义  *
*------------------*/




/***********************************************************
*   函数说明：缓冲区初始化函数 // initialization           *
*   输入：    1.管理结构体，2.缓冲区起始地址，3.缓冲区大小 *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ：   无                                           *
***********************************************************/
void bufferInit(cBuffer* mbuffer, unsigned char *start, unsigned int size)
{	 
	// set start pointer of the buffer
	mbuffer->dataptr = start;
	mbuffer->size = size;
	// initialize index and length
	mbuffer->dataindex = 0;
	mbuffer->datalength = 0;
}


/***********************************************************
*   函数说明：从缓冲区获取一个数据函数 // access routines  *
*   输入：    1.管理结构体                                 *
*   输出：    返回未读取的第一个值                         *
*   调用函数：无                                           *
*   说明 ：   长度自动减小 头指针加一                      *
***********************************************************/
unsigned char  bufferGetFromFront(cBuffer* mbuffer)
{
	unsigned char data = 0;
	 
    // check to see if there's data in the buffer
	if(mbuffer->datalength)
	{
		// get the first character from buffer
		data = mbuffer->dataptr[mbuffer->dataindex];
		// move index down and decrement length
		mbuffer->dataindex++;
		if(mbuffer->dataindex >= mbuffer->size)
		{
			mbuffer->dataindex -= mbuffer->size;
		}
		mbuffer->datalength--;
	}
	 	 
	// return
	return data;
}

/***********************************************************
*   函数说明：从缓冲区开始处删除数量的数据                 *
*   输入：    1.管理结构体 2.要删除数据的数量              *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ：   无                                           *
***********************************************************/
void bufferDumpFromFront(cBuffer* mbuffer, unsigned int numbytes)
{
	 
	 
	// dump numbytes from the front of the buffer
	// are we dumping less than the entire buffer?
	if(numbytes < mbuffer->datalength)
	{
		// move index down by numbytes and decrement length by numbytes
		mbuffer->dataindex += numbytes;
		if(mbuffer->dataindex >= mbuffer->size)
		{
			mbuffer->dataindex -= mbuffer->size;
		}
		mbuffer->datalength -= numbytes;
	}
	else
	{
		// flush the whole buffer
		mbuffer->datalength = 0;
	}
	 
	 
}

/***********************************************************
*   函数说明：从缓冲区读取指定位置的数据                   *
*   输入：    1.管理结构体 2.要读取数据的位置              *
*   输出：    读取的指定位置的数据                         *
*   调用函数：无                                           *
*   说明 ：   这个函数读取后的数据不会被删除               *
***********************************************************/
unsigned char bufferGetAtIndex(cBuffer* mbuffer, unsigned int index)
{
	  
	// return character at index in buffer
	unsigned char data = mbuffer->dataptr[(mbuffer->dataindex+index)%(mbuffer->size)];
	 
	return data;
}

/***********************************************************
*   函数说明：向缓冲区中添加数据                           *
*   输入：    1.管理结构体 2.要添加的数据                  *
*   输出：    是否添加成功的信号                           *
*   调用函数：无                                           *
*   说明 ：   无                                           *
***********************************************************/
unsigned char bufferAddToEnd(cBuffer* mbuffer, unsigned char dat)
{

	// make sure the buffer has room
	if(mbuffer->datalength < mbuffer->size)
	{
		// save data byte at end of buffer
		mbuffer->dataptr[(mbuffer->dataindex + mbuffer->datalength) % mbuffer->size] = dat;
		// increment the length
		mbuffer->datalength++;
		// return success
		return 0x01;
	}
	  
	// return failure
	return 0x00;
}

/***********************************************************
*   函数说明：判断缓冲区是否已满                           *
*   输入：    1.管理结构体                                 *
*   输出：    缓冲区剩余的数据的数量返回                   *
*   调用函数：无                                           *
*   说明 ：   无                                           *
***********************************************************/
unsigned int bufferIsNotFull(cBuffer* mbuffer)
{
	  
	// check to see if the buffer has room
	// return true if there is room
	unsigned int bytesleft = (mbuffer->size - mbuffer->datalength);
	  
	return bytesleft;
}


/***********************************************************
*   函数说明：缓冲区清空                                   *
*   输入：    1.管理结构体                                 *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ：   无                                           *
***********************************************************/
void  bufferFlush(cBuffer* mbuffer)
{

	// flush contents of the buffer
	mbuffer->datalength = 0;
	 	 
}
/***********************************************************
*   函数说明：缓冲区当前大小获取                           *
*   输入：    1.管理结构体                                 *
*   输出：    无                                           *
*   调用函数：无                                           *
*   说明 ：   无                                           *
***********************************************************/
unsigned int bufferGetSize(cBuffer* mbuffer)
{

	return (mbuffer->datalength) ;	 	 
}

