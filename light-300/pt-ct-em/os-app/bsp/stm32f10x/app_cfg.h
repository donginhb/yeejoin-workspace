/*
 ******************************************************************************
 * app_cfg.h
 *
 *  Created on: 2014-2-24
 *      Author: David, zhaoshaowei@yeejoin.com
 *
 * COPYRIGHT (C) 2014, YEEJOIN (Beijing) Technology Development Co., Ltd.
 ******************************************************************************
 */

#ifndef APP_CFG_H_
#define APP_CFG_H_

/* 老硬件的4432与7880共用spi */
#define USE_OLD_HARDWARE	0



#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 1234
#endif

#ifndef BIG_ENDIAN
#define BIG_ENDIAN 4321
#endif

#ifndef BYTE_ORDER
#define BYTE_ORDER  LITTLE_ENDIAN
#endif


#if USE_OLD_HARDWARE
/* only one can true */
#define ADE7880_USE_SPI		1
#define ADE7880_USE_I2C_HSDC	0

#define ADE7880_SPIFLASH_SHARE_SPI	0
#define ADE7880_SI4432_SHARE_SPI	1
#else
/* only one can true */
#define ADE7880_USE_SPI		0
#define ADE7880_USE_I2C_HSDC	1

#define ADE7880_SPIFLASH_SHARE_SPI	1
#define ADE7880_SI4432_SHARE_SPI	0

#endif


#endif /* APP_CFG_H_ */
