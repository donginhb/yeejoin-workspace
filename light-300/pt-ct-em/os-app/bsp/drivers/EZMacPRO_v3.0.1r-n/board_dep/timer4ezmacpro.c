/*
 * timer4ezmacpro.c
 *
 *  Created on: 2013-11-28
 *      Author: David, zhaoshaowei@yeejoin.com
 *
 *  该文件中的函数, 主要参考'EZMacPRO_v3.0.1r/bsp/timer.c'
 */

#include <timer4ezmacpro.h>
#include <si4432_hardif.h>


SEGMENT_VARIABLE(EZMacProTimerMSB, U16,  EZMAC_PRO_GLOBAL_MSPACE);
SEGMENT_VARIABLE(EZMacProTimerLSB, U16,  EZMAC_PRO_GLOBAL_MSPACE);
SEGMENT_VARIABLE(is_mac_timer_inuse, U8, EZMAC_PRO_GLOBAL_MSPACE);


/*
 * comment by David
 *
 * ezmacpro timer只在下面4种情况下启动：
 *
 * EZMacProReg.name.MSR = EZMAC_PRO_WAKE_UP;
 * ......
 * macTimeout (TIMEOUT_XTAL_START); // set time out to XTAL start-up time
 *
 *
 * EZMacProReg.name.MSR = TX_STATE_BIT | TX_STATE_LBT_START_LISTEN;
 * ......
 * macTimeout(TIMEOUT_LBTI_ETSI); // start timer with LBT ETSI fix timeout
 *
 *
 * macTimeout(TimeoutTX_Packet); // start timer with packet transmit timeout
 * EZMacProReg.name.MSR = TX_STATE_BIT | TX_STATE_WAIT_FOR_TX;
 *
 * macTimeout(TimeoutChannelSearch); // start timer with channel search timeout
 * EZMacProReg.name.MSR = RX_STATE_BIT | RX_STATE_FREQUENCY_SEARCH;
 *
 * */

/*================================================================================================
 * Timer Functions for EZMacPro.c module
 *
 * Parameters   : U32 longTime
 * Notes:
 * This function is called when a interrupt event must initiate a timeout event.
 * A 32-bit union is used to provide word and byte access. The upper word is stored in
 * EZMacProTimerMSB. The The lower word is first negated then written to the TL0 and TH0 sfrs.
 *================================================================================================
 */
void macTimeout(U32 longTime)
{
	base_t flag;

	flag = disable_si4432_ext_int();
	set_timeout_and_start_timer(longTime);
	restore_si4432_ext_int(flag);

	return;
}



/*
 *================================================================================================
 * Timer Functions for externalInt.c module
 *
 * Function Name
 *    extIntTimeout()
 *
 * Return Value : None
 * Parameters   : U32 longTime
 *
 * Notes:
 *
 * This function is called when a interrupt event must initiate a timeout event.
 * A 32-bit union is used to provide word and byte access. The upper word is stored in
 * EZMacProTimerMSB. The The lower word is first negated then written to the TL0 and TH0 sfrs.
 *================================================================================================
 */
#if !USE_MACRO_FUN_IN_TIMER4EZMACPRO
void extIntTimeout (U32 longTime)
{
	set_timeout_and_start_timer(longTime);

	return;
}
#endif



/*
 * ================================================================================================
 * Timer Functions for timerInt.c module
 *
 * Function Name
 *    timerIntTimeout()
 *
 * Return Value : None
 * Parameters   : U32 longTime
 *
 * Notes:
 * This function is called when a timeout event must initiate a subsequent timeout event.
 * A 32-bit union is used to provide word and byte access. The upper word is stored in
 * EZMacProTimerMSB. The The lower word is first negated then written to the TL0 and TH0 sfrs.
 *
 * This function is not included for the Transmitter only configuration.
 *================================================================================================
 */
#ifndef TRANSMITTER_ONLY_OPERATION
#if !USE_MACRO_FUN_IN_TIMER4EZMACPRO
void timerIntTimeout (U32 longTime)
{
	set_timeout_and_start_timer(longTime);
}
#endif
#endif//TRANSMITTER_ONLY_OPERATION

/*
 * 该函数用于设置16-bit定时器
 *
 * time[15, 0]	-- 定时器溢出时间
 * time[31, 16]	-- 定时器溢出次数
 * timeout_cnt	-- 用来记录定时器溢出次数的变量, 在ISR中使用
 * */
void set_timeout_and_start_timer(U32 longTime)
{
	UU32 time;
	U16 timeout;

	DISABLE_MAC_TIMER_INTERRUPT();       // Disable Timer interrupt
	STOP_MAC_TIMER();                    // Stop Timer

	if (0 != is_mac_timer_inuse) {
		/* 用于检测定时器使用的冲突, 如果有冲突说明状态控制有异常, 暂时不做处理 */
	}

	is_mac_timer_inuse = 1;

	time.U32 = longTime;
	if (longTime > 65535) {
		EZMacProTimerMSB	= time.U16[MSB];
		timeout	= 65535;
	} else {
		EZMacProTimerMSB	= 0;
		timeout = time.U16[LSB];
	}
	EZMacProTimerLSB	= time.U16[LSB];

#if C51_SYNTAX_
	time.U16[LSB] 	= -time.U16[LSB];
	TIMER_LOW_BYTE 	= time.U8[b0];       // write LSB first
	TIMER_HIGH_BYTE = time.U8[b1];       // write MSB last
#else
	init_si4432_mac_timer(timeout);
#endif

	CLEAR_MAC_TIMER_INTERRUPT();         // Clear Timer interrupt
	START_MAC_TIMER();                   // Start Timer

	return;
}
