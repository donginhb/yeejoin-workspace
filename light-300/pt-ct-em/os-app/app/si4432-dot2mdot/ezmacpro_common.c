/*
 * ezmacpro_common.c
 *
 *  Created on: 2013-11-27
 *      Author: David, zhaoshaowei@yeejoin.com
 *
 *  (ezmacpro common)
 */



#include <rtthread.h>

#include <ezmacpro_common.h>

#include <EZMacPro_Defs.h>
#include <EZMacPro.h>
#include <compiler_defs.h>

#include <si4432_v2.h>
#include <spi4ezmacpro.h>

#define ms_common_debug(x)	//printf_syn x
#define ms_common_log(x)	printf_syn x
#define ms_common_info(x)	printf_syn x

#define do_something_when_wait_flag()	rt_thread_delay(get_ticks_of_ms(20))
#define get_wait_time_cnt(x_ms)		((x_ms)/20)

/*
 * state	-- wait into state
 * timeout_ms	-- timeout unit is ms
 *
 *
 *
 * */
wait_emmacpro_into_state_err_t wait_emmacpro_into_state(enum ezmacpro_state_e state, U16 timeout_ms)
{
	ubase_t cnt_max, cnt, temp;
	wait_emmacpro_into_state_err_t ret = WEIS_FAIL;

	cnt = 0;
	if (WAIT_EMMACPRO_INTO_STATE_FOREVER != timeout_ms) {
		cnt_max = get_wait_time_cnt(timeout_ms);
		if (0 == cnt_max)
			cnt_max = 1;
	} else {
		cnt_max = WAIT_EMMACPRO_INTO_STATE_FOREVER;
	}

	switch (state) {
	/* reset si4432 or exit sleep-mode后，进入idle-mode之前的临时状态  */
	case EZMACS_WAKEUP	:
		/* Wait until device goes to Sleep. */
		while (!fEZMacPRO_StateSleepEntered) {
			do_something_when_wait_flag();
			if (WAIT_EMMACPRO_INTO_STATE_FOREVER!=cnt_max && ++cnt>=cnt_max)
				goto err_ret;
		}
		clr_ezmac_state_trans_flag(fEZMacPRO_StateWakeUpEntered);
		clr_ezmac_state_trans_flag(fEZMacPRO_StateSleepEntered);
		break;

	/* si4432已进入sleep-mode */
	case EZMACS_SLEEP	:
		/* Wait until device goes to Sleep. */
		while (!fEZMacPRO_StateSleepEntered) {
			do_something_when_wait_flag();
			if (WAIT_EMMACPRO_INTO_STATE_FOREVER!=cnt_max && ++cnt>=cnt_max)
				goto err_ret;
		}
		clr_ezmac_state_trans_flag(fEZMacPRO_StateSleepEntered);
		break;

	/* si4432已进入idle-mode */
	case EZMACS_IDLE	:
		if (is_ezmac_in_idle_state()) {
			clr_ezmac_state_trans_flag(fEZMacPRO_StateIdleEntered);
			break;
		}

		/* Wait until device goes to Idle. */
		while (!fEZMacPRO_StateIdleEntered) {
			do_something_when_wait_flag();
			if (WAIT_EMMACPRO_INTO_STATE_FOREVER!=cnt_max && ++cnt>=cnt_max)
				goto err_ret;
		}
		clr_ezmac_state_trans_flag(fEZMacPRO_StateIdleEntered);
		break;

	/* si4432已进入rx-mode, RX_STATE_FREQUENCY_SEARCH */
	case EZMACS_RX	:
		/* Wait until radio is placed to RX. */
		while (!fEZMacPRO_StateRxEntered) {
			do_something_when_wait_flag();
			if (WAIT_EMMACPRO_INTO_STATE_FOREVER!=cnt_max && ++cnt>=cnt_max)
				goto err_ret;
		}
		clr_ezmac_state_trans_flag(fEZMacPRO_StateRxEntered);
		break;

	/* si4432已进入tx-mode, TX_STATE_LBT_START_LISTEN/TX_STATE_WAIT_FOR_TX */
	case EZMACS_TX	:
		break;

	/* WAKE_UP_ERROR / RX_ERROR_STATE / TX_ERROR_STATE */
	case EZMACS_ERROR	:
		break;

	/* ======================================== */

	/* 'Low Frequency Timer' timeout, Wake up timer interrupt is occurred */
	case EZMACS_LFTIMER_EXPIRED	:
		/* Wait here until LFT expires. */
		while(!fEZMacPRO_LFTimerExpired) {
			do_something_when_wait_flag();
			if (WAIT_EMMACPRO_INTO_STATE_FOREVER!=cnt_max && ++cnt>=cnt_max)
				goto err_ret;
		}
		clr_ezmac_state_trans_flag(fEZMacPRO_LFTimerExpired);
		break;

	/* low battery detect interrupt is occurred */
	case EZMACS_LOW_BATTERY		:
		break;

	/* sync word detect interrupt is occured */
	case EZMACS_SYNCWORD_RECEIVED	:
		break;

	/* CRC error occurred */
	case EZMACS_CRC_ERR		:
		break;

	/* 丢弃了接收到的无用数据包(目标地址与本地不符, 也不需要转发) */
	case EZMACS_PKT_DISCARED		:
		break;

	/* 已接收到数据包, 并且已将数据读入ezmacpro的rx-buffer */
	case EZMACS_PKT_RECEIVED		:
		while (!fEZMacPRO_PacketReceived) {
			do_something_when_wait_flag();

			temp = get_ezmacpro_msr();

			if ((RX_STATE_BIT | RX_ERROR_STATE) == temp) {
				ret = WEIS_RX_ERROR_STATE;
				goto err_ret;
			}

			if (WAIT_EMMACPRO_INTO_STATE_FOREVER!=cnt_max && ++cnt>=cnt_max)
				goto err_ret;
		}
		clr_ezmac_state_trans_flag(fEZMacPRO_PacketReceived);
		break;

	/* 即将转发数据包 */
	case EZMACS_PKT_FORWARDING	:
		break;

	/* 数据包发送完成 */
	case EZMACS_PKT_SENT		:
		while (!fEZMacPRO_PacketSent) {
			do_something_when_wait_flag();

			temp = get_ezmacpro_msr();

			if ((TX_STATE_BIT | TX_ERROR_CHANNEL_BUSY) == temp) {
				ret = WEIS_TX_CHANNEL_BUSY;
				ms_common_info(("channel busy\n"));
				goto err_ret;
			} else if ((TX_STATE_BIT | TX_ERROR_STATE) == temp) {
				ret = WEIS_TX_ERROR_STATE;
				ms_common_info(("tx error,send timeout\n"));
				goto err_ret;
			}

			if (WAIT_EMMACPRO_INTO_STATE_FOREVER!=cnt_max && ++cnt>=cnt_max)
				goto err_ret;
		}
		clr_ezmac_state_trans_flag(fEZMacPRO_PacketSent);
		break;

	/* ezmacpro LBT timeout,
	 * EZMAC_PRO_ERROR_CHANNEL_BUSY-(TX_ERROR_CHANNEL_BUSY,RX_ERROR_FORWARDING_WAIT_FOR_TX) */
	case EZMACS_LBT_TIMEOUT		:
		break;

	/* TX_STATE_WAIT_FOR_ACK timeout */
	case EZMACS_ACK_TIMEOUT		:
		break;

	/* 即将自动发送ack, customise Ack Packet payload */
	case EZMACS_ACK_SENDING		:
		break;

	default:
		ms_common_log(("func:%s() param error.\n", __FUNCTION__));
		goto err_ret;
	}


	return WEIS_OK;

err_ret:
//	ms_common_debug(("wait into state-%d error, current state:0x%x\n", state, EZMacProReg.name.MSR));

	return ret;
}

/*
 * 设置Low Frequency Timer
 *
 * NOTE:
 * 	lft在ezmacpro中没有使用, mac层之上的软件可以使用
 * */
int set_ezmacpro_lft_timer(base_t is_ms, uint16_t timeout)
{
	int ret;
	uint16_t t0, t1, t2;
	MacParams mac_ret;

	ret = SUCC;

	if (0 != is_ms) {
		t0 = LFTMR0_TIMEOUT_MSEC(timeout);
		t1 = LFTMR1_TIMEOUT_MSEC(timeout);
		t2 = 0x80 | LFTMR2_TIMEOUT_MSEC(timeout);
	} else {
		t0 = LFTMR0_TIMEOUT_SEC(timeout);
		t1 = LFTMR1_TIMEOUT_SEC(timeout);
		t2 = 0x80 | LFTMR2_TIMEOUT_SEC(timeout);
	}

	/* Configure and start 200msec timeout for Beacon time frame. */
	mac_ret = EZMacPRO_Reg_Write(LFTMR0,t0);
	if (MAC_OK != mac_ret) {
		ms_common_log(("write lftmro fail(%d), when send ezmac pkt\n", mac_ret));
		ret = FAIL;
		goto ret_entry;
	}

	mac_ret = EZMacPRO_Reg_Write(LFTMR1, t1);
	if (MAC_OK != mac_ret) {
		ms_common_log(("write lftmr1 fail(%d), when send ezmac pkt\n", mac_ret));
		ret = FAIL;
		goto ret_entry;
	}

	mac_ret = EZMacPRO_Reg_Write(LFTMR2, t2);
	if (MAC_OK != mac_ret) {
		ms_common_log(("lftmr2 fail(%d), when send ezmac pkt\n", mac_ret));
		ret = FAIL;
		goto ret_entry;
	}

ret_entry:
	return ret;
}

/*
 * 填充发送缓冲区，将si4432设置为tx-mode，发送完成后，ezmacpro会根据设置将si4432设置为sleep-mode/rx-mode/idle-mode
 *
 * return SUCC / FAIL
 * */
int sent_pkt_to_ezmacpro(struct ezmacpro_pkt_header_t *h, U8 *payload, ubase_t len, send_pkt2ezmacpro_flag_t flag)
{
	int ret;
	ubase_t state;
	MacParams mac_ret;

	if (NULL==h || NULL==payload) {
		ms_common_info(("func:%s() param error", __FUNCTION__));
		return FAIL;
	}

	ret   = SUCC;

	state = get_ezmacproc_fsm_state();
	if (EZMAC_PRO_IDLE != state) {
		if (is_bit_set(flag, SPEFB_FORCE_SEND)) {
			ms_common_debug(("func:%s(), line:%d, mac msr:0x%x\n", __FUNCTION__, __LINE__, state));
			mac_ret = EZMacPRO_Idle(); /* Go to Idle state. */

			if (0==fEZMacPRO_StateIdleEntered && MAC_OK!=mac_ret) {
				ms_common_log(("into ezmacpro idle fail(0x%x), when send ezmac pkt\n", mac_ret));
				ret = FAIL;
				goto ret_entry;
			}

			if (WEIS_OK != wait_emmacpro_into_state(EZMACS_IDLE, 2000)) {
				ret = FAIL;
				ms_common_info(("waiting into idel state fail, when send pkt to ezmacpro.\n"));
				goto ret_entry;
			}
		} else {
			ret = FAIL;
			ms_common_info(("mac is not in idle state(0x%x), cann't send pkt.\n", state));
			goto ret_entry;
		}
	}

//	ms_common_debug(("func:%s(), line:%d, state:0x%x\n", __FUNCTION__, __LINE__, state));

//	if (is_bit_set(flag, SPEFB_SET_MAC_TCR)) {
//		mac_ret = EZMacPRO_Reg_Write(TCR, (0x70 | LBT_SWITCH));
//		if (MAC_OK != mac_ret) {
//			ms_common_log(("write TCR fail(%d), when send ezmac pkt\n", mac_ret));
//			ret = FAIL;
//			goto ret_entry;
//		}
//	}


	mac_ret = EZMacPRO_Reg_Write(DID, h->dst_id); // Set Destination ID
	if (MAC_OK != mac_ret) {
		ms_common_log(("write DID fail(%d), when send ezmac pkt\n", mac_ret));
		ret = FAIL;
		goto ret_entry;
	}

	/* Write the packet length and payload to the TX buffer. */
	mac_ret = EZMacPRO_TxBuf_Write(len, payload);
	if (MAC_OK != mac_ret) {
		ms_common_log(("write tx-buf fail(%d), when send ezmac pkt\n", mac_ret));
		ret = FAIL;
		goto ret_entry;
	}

	/* Send the packet. */
	mac_ret = EZMacPRO_Transmit();
	if (MAC_OK != mac_ret) {
		ms_common_log(("transmit fail(%d), when send ezmac pkt\n", mac_ret));
		ret = FAIL;
		goto ret_entry;
	}

	ms_common_debug(("func:%s(), line:%d, had sent %d byte(s) data\n", __FUNCTION__, __LINE__, len));

//	/* Wait until device goes to Idle. */
//	if (WEIS_OK != wait_emmacpro_into_state(EZMACS_IDLE)) {
//		ret = FAIL;
//		ms_common_log(("after send pkt waiting into idel state fail(%d), when send ezmac pkt\n", mac_ret));
//		goto ret_entry;
//	}
//	clr_ezmac_state_trans_flag(fEZMacPRO_StateIdleEntered);

ret_entry:
	return ret;
}

/*
 * NOTE: 必须在fEZMacPRO_PacketReceived有效后调用
 * */
int recv_pkt_from_ezmacpro(U8 *buf, ubase_t buf_len, ubase_t *payload_len, ubase_t *rssi)
{
	int ret;

	if (NULL==buf || NULL==payload_len || buf_len<PAYLOAD_LENGTH_MAX || NULL==rssi) {
		ms_common_info(("func:%s() param error, buf_len:%d\n", __FUNCTION__, buf_len));
		return FAIL;
	}

	ret = SUCC;

#if 0
	if (fEZMacPRO_PacketReceived) {
		fEZMacPRO_PacketReceived = 0; /* Clear flag. */
		EZMacPRO_RxBuf_Read(payload_len, buf); /* Read out the payload. */
	} else {
		ms_common_log(("not received pkt, when recv pkt\n"));
		ret = FAIL;
	}
#else
	EZMacPRO_RxBuf_Read(payload_len, buf); /* Read out the payload. */
	*rssi = get_ezmacpro_rssi();
#endif
	ms_common_debug(("func:%s(), line:%d, recv %d byte(s) data\n", __FUNCTION__, __LINE__, *payload_len));

	return ret;
}

int recv_pkt_from_ezmacpro_prepare(void)
{
	MacParams mac_ret;
	int ret = SUCC;

	set_ezmac_into_idle_state(); /* 先让ezmac显式进入idle状态, 以避免芯片被干扰而异常, 无法进入接收状态, mark by David */

	/* Go to receive state. */
	mac_ret = EZMacPRO_Receive();
	if (MAC_OK != mac_ret) {
		ms_common_log(("rf into receive state fail(%d), when send ezmac pkt\n", mac_ret));
		ret = FAIL;
		goto ret_entry;
	}

	/* Wait until radio is placed to RX. */
	if (WEIS_OK != wait_emmacpro_into_state(EZMACS_RX, 2000)) {
		ret = FAIL;
		ms_common_log(("waiting into rx-state fail.\n"));
		goto ret_entry;
	}

	ms_common_debug(("func:%s(), line:%d\n", __FUNCTION__, __LINE__));
	rt_thread_delay(1); /* ??? David 20140603 */
ret_entry:
	return ret;
}


/*
 * 如果失败返回1，否则返回0
 * */
int write_ezmacpro_reg_check_if_succ(MacRegs  name, U8 value)
{
	MacParams ret;

	ret = EZMacPRO_Reg_Write(name, value);
	if (MAC_OK != ret) {
		ms_common_log(("write EZMacPRO_Reg fail(%d), reg name:%d, value:%d\n", ret, name, value));
		return 1;
	} else {
		return 0;
	}
}


/*
 * #define EZMAC_PRO_SLEEP     0x00
 * #define EZMAC_PRO_IDLE      0x40
 * #define EZMAC_PRO_WAKE_UP   0x80
 * #define WAKE_UP_ERROR       0x8F
 *
 * #define TX_STATE_BIT        0x10
 * #define RX_STATE_BIT        0x20
 *
 * */
ubase_t get_ezmacproc_fsm_state(void)
{
//	ms_common_info(("func:%s(), msr:0x%x, secr:0x%x\n", __FUNCTION__,
//			EZMacProReg.name.MSR, EZMacProReg.name.SECR));

	return EZMacProReg.name.MSR;
}


/*
 * EZMacProReg.name.SECR -- State & Error Counter Control Register
 *     bit[7,6] -- the next state after transmit control bits
 *     bit[5,4] -- the next state after receive  control bits
 *     bit[3,0] -- SECR register error codes, define in EZMacPro.h
 *
 * the next state after transmit/receive define:
 *	0	-- go to sleep mode
 *	2	-- go to RX mode
 *	other	-- go to Idle mode, disable RX & TX
 *
 * next_state 使用如下宏定义
 *    #define NEXT_STATE_IS_SLEEP_MODE	0X0
 *    #define NEXT_STATE_IS_SLEEP_RX	0X2
 *    #define NEXT_STATE_IS_SLEEP_IDLE	0X1
 *
 *    #define NEXT_STATE_CTRL_BITS_AFTER_TX_OFFSET	6
 *    #define NEXT_STATE_CTRL_BITS_AFTER_RX_OFFSET	4
 * */
int set_ezmacpro_secr_next_state_ctrl_bits(base_t is_after_tx, ubase_t next_state)
{
	next_state &= 0x3; /* 做为保护 */

	if (0 != is_after_tx)
		EZMacProReg.name.SECR |= next_state << NEXT_STATE_CTRL_BITS_AFTER_TX_OFFSET;
	else
		EZMacProReg.name.SECR |= next_state << NEXT_STATE_CTRL_BITS_AFTER_RX_OFFSET;

	return SUCC;
}


void print_callback_var(void)
{
	ms_common_info(("wakeup:%d, sleep:%d, idle:%d, rx:%d, tx:%d, error:%d\n"
			"lft:%d, lowbattery:%d, synword:%d, crcerr:%d, pktdiscard:%d, pkt-recv:%d\n"
			"pkt-forward:%d, pkt-sent:%d, lbt-timeout:%d, ack-to:%d, ack-sending:%d\n",
			fEZMacPRO_StateWakeUpEntered, fEZMacPRO_StateSleepEntered, fEZMacPRO_StateIdleEntered,
			fEZMacPRO_StateRxEntered, fEZMacPRO_StateTxEntered, fEZMacPRO_StateErrorEntered,
			fEZMacPRO_LFTimerExpired, fEZMacPRO_LowBattery, fEZMacPRO_SyncWordReceived,
			fEZMacPRO_CRCError, fEZMacPRO_PacketDiscarded, fEZMacPRO_PacketReceived,
			fEZMacPRO_PacketForwarding, fEZMacPRO_PacketSent, fEZMacPRO_LBTTimeout,
			fEZMacPRO_AckTimeout, fEZMacPRO_AckSending
	));

	return;
}


int is_ezmac_send_pkt_over(void)
{
	if (0 != fEZMacPRO_PacketSent) {
		/* 数据包发送完成 */
		clr_ezmac_state_trans_flag(fEZMacPRO_PacketSent);
		return TRUE;
	} else {
		return FALSE;
	}
}

int is_ezmac_had_recv_pkt(void)
{
	if (0 != fEZMacPRO_PacketReceived) {
		/* 已接收到数据包, 并且已将数据读入ezmacpro的rx-buffer */
		clr_ezmac_state_trans_flag(fEZMacPRO_PacketReceived);
		return TRUE;
	} else {
		return FALSE;
	}
}

int is_ezmac_in_recv_state(void)
{
	ubase_t temp;

	/* 尽可能减少使用spi, 使用中发现有mac state与si4432状态不一致的问题, 所以这里对两个都进行判断 */
	if (is_bit_set(EZMacProReg.name.MSR, RX_STATE_BIT)) {
#if 1		/* mark by zp */
		temp = macSpiReadReg(SI4432_DEVICE_STATUS);
		/* Chip Power State. bit[1:0] ---- 00: Idle State, 01: RX State, 10: TX State */
		if ((temp & SI4432_CPS_MASK) == 0x01) {
			return 1;
		} else {
			/* 最好设置si4432进入idle状态, mark by David */
			set_ezmac_into_idle_state();

			ms_common_log(("mac state is rx(0x%x), but si4432 state is not rx(0x%x)\n",
					EZMacProReg.name.MSR, temp));
			return -1;
		}
#else
		return 1;
#endif
	} else {
		return 0;
	}

/*	return (is_bit_set(EZMacProReg.name.MSR, RX_STATE_BIT) && ((temp & SI4432_CPS_MASK) == 0x01)); */
}


int is_ezmac_in_idle_state(void)
{
	return (EZMAC_PRO_IDLE == EZMacProReg.name.MSR);
}


uint8_t get_pkt_sid(void)
{
	return EZMacProReg.name.RSID;
}

uint8_t get_pkt_did(void)
{
	return EZMacProReg.name.DID;
}

base_t set_ezmac_into_idle_state(void)
{
	MacParams mac_ret;

	ms_common_debug(("func:%s(), line:%d\n", __FUNCTION__, __LINE__));

	mac_ret = EZMacPRO_Idle(); /* Go to Idle state. */

	if (0==fEZMacPRO_StateIdleEntered && MAC_OK!=mac_ret) {
		ms_common_log(("into ezmacpro idle fail(0x%x)\n", mac_ret));
		goto ret_entry;
	}

	if (WEIS_OK != wait_emmacpro_into_state(EZMACS_IDLE, 50)) {
		ms_common_info(("waiting into idel state fail, when into ezmac idle state.\n"));
		goto ret_entry;
	}

	return SUCC;

ret_entry:
	return FAIL;
}
