/*
 ******************************************************************************
 * app_cfg_emm_m.h
 *
 *  Created on: 2014-4-17
 *      Author: David, zhaoshaowei@yeejoin.com
 *
 * COPYRIGHT (C) 2014, YEEJOIN (Beijing) Technology Development Co., Ltd.
 ******************************************************************************
 */

#ifndef APP_CFG_EMM_M_H_
#define APP_CFG_EMM_M_H_

#define RS485_SW_DEVICE	1


/* rand func */
#define ENABLE_RAND             0

/* hardware module cfg */
#define RT_USING_SERIAL_FLASH	0
#define RT_USING_ENC28J60	0
#define RT_USING_EXT_SRAM	0
#define RT_USING_TL16C554	1
#define RT_USING_STM32_FSMC	1
#define RT_EXT_SRAM_MULTIPLEXED	0

/* software module cfg */
#define RT_USING_RTC		0
#define RT_USING_DLT645		0
#define RT_USING_RS485_BUS	1

#define RT_USING_FILESYSTEM	0
#define RT_USING_UFFS		0
#define RT_USING_ELMFAT		0

#define RT_USING_TCPIP_STACK	0
#define RT_USING_TELNETD	0
#define RT_USING_TFTP		0
#define RT_USING_HTTPSERVER_RAW	0

#define RT_USING_GUI		0


#endif /* APP_CFG_EMM_M_H_ */
