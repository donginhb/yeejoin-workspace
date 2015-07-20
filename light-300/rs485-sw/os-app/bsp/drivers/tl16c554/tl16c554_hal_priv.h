/*
 ******************************************************************************
 * tl16c554_hal_priv.h
 *
 *  Created on: 2014-12-31
 *      Author: David, zhaoshaowei@yeejoin.com
 *
 * COPYRIGHT (C) 2014, YEEJOIN (Beijing) Technology Development Co., Ltd.
 ******************************************************************************
 */


#ifndef TL16C554_HAL_PRIV_H_
#define TL16C554_HAL_PRIV_H_


#define is_x_csx_valid(x)	((x)>=TL16_1_CSA_ADDR && (x)<=TL16_2_CSD_ADDR)

#define get_cs_code_from_regaddr(regaddr)	((regaddr)>>TL16_X_CSX_ADDR_OFFSET & 0xff)
#define get_reg_from_regaddr(regaddr)		((regaddr) & 0xff)




#endif /* TL16C554_HAL_PRIV_H_ */
