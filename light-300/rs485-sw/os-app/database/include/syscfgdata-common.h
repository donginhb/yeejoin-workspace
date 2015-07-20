/*
 ******************************************************************************
 * syscfgdata-common.h
 *
 *  Created on: 2014-4-24
 *      Author: David, zhaoshaowei@yeejoin.com
 *
 * COPYRIGHT (C) 2014, YEEJOIN (Beijing) Technology Development Co., Ltd.
 ******************************************************************************
 */

#ifndef SYSCFGDATA_COMMON_H_
#define SYSCFGDATA_COMMON_H_

#define USE_HEX_DEV_SN	0
/*
 * 年份 year
 * 产地 place of production
 * 批次 batch
 * 发射(T)/接收(R)/PT采集器(P)/CT采集器(C)
 * 序号 serial NO
 *
 * PT-F/CT-C/EM-E
 */
#define DEV_SN_STR_MODE		"yypp-bbxssss"
#define DEV_SN_HEX_MODE		"yyppbbxsssss"

/* 虚假EM的SN */
#define PHONY_EM_SN		"YJ-PHONYEM"
#define PHONY_EM_SN_LEN		10

#define DEV_TYPE_OFFSET_IN_STR_SN	(7)
#define DEV_TYPE_OFFSET_IN_HEX_SN	(6)

#define DEV_SN_STR_MODE_LEN 	(12)
#define DEV_SN_HEX_MODE_LEN 	(6)

#define DEV_SN_STR_LEN_MAX (DEV_SN_STR_MODE_LEN)
#define DEV_SN_HEX_LEN_MAX (DEV_SN_HEX_MODE_LEN)

#if 0!=USE_HEX_DEV_SN
#define DEV_SN_MODE		DEV_SN_HEX_MODE
#define DEV_TYPE_OFFSET_IN_SN	DEV_TYPE_OFFSET_IN_HEX_SN
#define DEV_SN_MODE_LEN 	DEV_SN_HEX_MODE_LEN
#define DEV_SN_LEN_MAX		DEV_SN_HEX_LEN_MAX
#else
#define DEV_SN_MODE		DEV_SN_STR_MODE
#define DEV_TYPE_OFFSET_IN_SN	DEV_TYPE_OFFSET_IN_STR_SN
#define DEV_SN_MODE_LEN 	DEV_SN_STR_MODE_LEN
#define DEV_SN_LEN_MAX		DEV_SN_STR_LEN_MAX
#endif


#define DEV_SN_BUF_STRING_WITH_NUL_LEN_MAX	(((DEV_SN_MODE_LEN+1)+3) & (~3U))
#define DEV_SN_BUF_CHARS_NUM_MAX 		(DEV_SN_BUF_STRING_WITH_NUL_LEN_MAX-1)


#define WIRELESS_SLAVE_NODE_MAX	(50)
#define WIRELESS_MASTER_NODE_MAX  (4)


#define NUM_OF_COLLECT_EM_MAX	(12)
#define LEN_OF_EM_SN_MAX	(DEV_SN_BUF_STRING_WITH_NUL_LEN_MAX)
#define SINKINFO_WL_DATA_ITEM_MAX_NO	(WIRELESS_MASTER_NODE_MAX * WIRELESS_SLAVE_NODE_MAX + WIRELESS_MASTER_NODE_MAX)

/* 一个主电表采集器可以对应的从电表采集器的最大个数 */
#define NUM_OF_SLAVE_EMM_MAX	(10)

#define NUM_OF_PT_OF_COLLECT_EM_MAX	(4)
/* 一条线路可能对应两个ctc */
#define NUM_OF_CT_OF_COLLECT_EM_MAX	(NUM_OF_COLLECT_EM_MAX*2)

#endif /* SYSCFGDATA_COMMON_H_ */
