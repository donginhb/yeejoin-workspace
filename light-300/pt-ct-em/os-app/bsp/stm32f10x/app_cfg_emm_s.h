/*
 ******************************************************************************
 * app_cfg_emm_s.h
 *
 *  Created on: 2014-4-24
 *      Author: David, zhaoshaowei@yeejoin.com
 *
 * COPYRIGHT (C) 2014, YEEJOIN (Beijing) Technology Development Co., Ltd.
 ******************************************************************************
 */

#ifndef APP_CFG_EMM_S_H_
#define APP_CFG_EMM_S_H_


/* rand func */
#define ENABLE_RAND             0

/* only one can true */
#define WIRELESS_MASTER_NODE	0
#define WIRELESS_SLAVE_NODE	0

/* only one can true */
#define PT_DEVICE	0
#define CT_DEVICE	0
#define EM_MASTER_DEV	0
#define EM_SLAVE_DEV	0
#define EM_MULTI_MASTER_DEV	0
#define EM_MULTI_SLAVE_DEV	1

/* (EM_MASTER_DEV || EM_SLAVE_DEV) */
#define EM_DEV_BASE		0
/* (EM_MULTI_MASTER_DEV || EM_MULTI_SLAVE_DEV) */
#define EM_MULTI_BASE		1
/* (EM_DEV_BASE || EM_MULTI_BASE) */
#define EM_ALL_TYPE_BASE	1

/* hardware module cfg */
#define RT_USING_ADE7880	1
#define RT_USING_SI4432_MAC	0
#define RT_USING_GPRS		0
#define RT_USING_SERIAL_FLASH	0
#define RT_USING_ENC28J60	1
#define RT_USING_EXT_SRAM	1
#define RT_EXT_SRAM_MULTIPLEXED	0

/* software module cfg */
#define RT_USING_RTC		1
#define RT_USING_DLT645		1
#define RT_USING_RS485_BUS	1

#define RT_USING_FILESYSTEM	0
#define RT_USING_UFFS		0
#define RT_USING_ELMFAT		0

#define RT_USING_TCPIP_STACK	1
#define RT_USING_TELNETD	0
#define RT_USING_TFTP		0
#define RT_USING_HTTPSERVER_RAW	0

#define RT_USING_GUI		0


#endif /* APP_CFG_EMM_S_H_ */
