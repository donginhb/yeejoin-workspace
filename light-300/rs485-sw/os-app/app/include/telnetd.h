/*
 * telnetd.h
 */

/**
  ******************************************************************************
  * @file    helloworld.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11/20/2009
  * @brief   This file contains all the functions prototypes for the helloworld.c
  *          file.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TELNETD_H__
#define __TELNETD_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <syscfgdata.h>
#include <rtdef.h>

#include <base_ds.h>


enum telnetio_state {
	TELS_UN_CONNECT = 0,
	TELS_LOGIN_NAME,
	TELS_LOGIN_PW,
	//TELS_CONNECT,
	TELS_NORMAL,

	TELS_IAC,
	TELS_WILL,
	TELS_WONT,
	TELS_DO,
	TELS_DONT,
	TELS_IAC_OPT, /* option */

	TELS_CLOSE,
	TELS_LOGOUT,
};

struct telnetio_dev {
	char dev_name[8];
	enum telnetio_state state;     /* must initial to 0 */
	enum telnetio_state iac_state; /* must initial to 0 */
	int  td_flag;

	struct rt_device dev;
	struct usr_pw_pair usrpw;

	struct rb rx_rb_buf;
	struct rb tx_rb_buf;

	unsigned char iac_buf[64];
	int iac_index;
};

#define RX_RB_BUF_SIZE (128)
#define TX_RB_BUF_SIZE (1024*2)



/** @defgroup telnetd_Exported_Functions
  * @{
  */

void telnetd_init(void);

/**
  * @}
  */



#ifdef __cplusplus
}
#endif

#endif /* __HELLOWERLOD_H */


/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/


