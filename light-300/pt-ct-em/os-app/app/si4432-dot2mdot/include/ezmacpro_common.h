/*
 * ezmacpro_common.h
 *
 *  Created on: 2013-11-27
 *      Author: David, zhaoshaowei@yeejoin.com
 *
 *  2013-12-12 -- add ezmacpro comment by David
 */


#ifndef MS_COMMON_H_
#define MS_COMMON_H_

#include <compiler_defs.h>
#include <EZMacPro_Defs.h>

#include <EZMacPro_CallBacks.h>
#include <EZMacPro.h>



/*
 * * EZMacProReg.name.MCR(Master Control Register) * *
 *
 * bit7:
 * 	1 - use CID(customer ID)
 * 	0 - don't use CID
 *
 * bit[6:5]:
 * 	用来指示4个速度等级: (2400), (9600), (50000), (128000L)
 * 	EZMacProByteTime[index], Parameters[index][MAX_CHANNEL_NUMBER])的一维下标
 *
 * 	note:
 * 	const SEGMENT_VARIABLE (Parameters[4][11], U8, SEG_CODE)
 * 	const SEGMENT_VARIABLE( EZMacProByteTime[4], U16, SEG_CODE)
 *
 *
 *
 * bit[4:3]:
 *	EZMacProReg.name.RCTRL[1:0]
 *
 * bit2:
 * 	1 - use dynamic payload length.(read out the received packet length from SI4432_RECEIVED_PACKET_LENGTH
 * 	    and save to PLEN MAC register)
 * 	0 -
 * 	note: SI4432_RECEIVED_PACKET_LENGTH(0x4b)--Length Byte of the Received Packet during fixpklen = 0.
 *
 * bit[1:0]:
 * 	number of used channel(1-4)
 *	Parameters[]	PR1,	PR2,  PR3,  PR4 (apreamble length when 1-4 channel used)
 * */




/*
 * * EZMacProReg.name.SECR(State & Error Counter Control Register) * *
 *
 * bit[7,6] -- the next state after transmit control bits
 * bit[5,4] -- the next state after receive  control bits
 * bit[3,0] -- SECR register error codes, define in EZMacPro.h
 *     	#define EZMAC_PRO_ERROR_BAD_CRC        0x01
 *	#define EZMAC_PRO_ERROR_BAD_ADDR       0x02
 *	#define EZMAC_PRO_ERROR_BAD_CID        0x04
 *	#define EZMAC_PRO_ERROR_CHANNEL_BUSY   0x08
 * */
/* next_state_after_txrx_ctrl_bits */
#define NEXT_STATE_IS_SLEEP_MODE	0X0
#define NEXT_STATE_IS_SLEEP_RX		0X2
#define NEXT_STATE_IS_SLEEP_IDLE	0X1

#define NEXT_STATE_CTRL_BITS_AFTER_TX_OFFSET	6
#define NEXT_STATE_CTRL_BITS_AFTER_RX_OFFSET	4





/*
 * * EZMacProReg.name.TCR(Transmit Control Register) * *
 *
 * bit7:
 * 	1 - enable ACKRQ (request ACK)
 * 	0 - disable ACKRQ
 *
 * bit[6:4]:
 * 	TX Output Power.
 * 	The output power is configurable from +13 dBm to –8 dBm (Si4430/31), and from +20 dBM to –1 dBM (Si4432) in
 *   ~3 dB steps. txpow[2:0]=000 corresponds to min output power, while txpow[2:0]=111 corresponds to max output power.
 *
 *
 * bit3:
 * 	1 - Listen Before Talk will be performed before each frame transmission.
 * 	0 - disable lbt
 *
 * bit2:
 * 	1 - Automatic Frequency Change feature is on. (then send the same packet on the four channels)
 * 	0 - Automatic Frequency Change feature is off.
 *
 * bit[1:0]:
 * 	似乎没有使用
 * */
#define LBT_SWITCH_BIT		(0x08)

#define lbt_switch_on(tcr)	((tcr) | LBT_SWITCH_BIT)
#define lbt_switch_off(tcr)	((tcr) & ~LBT_SWITCH_BIT)




/*
 * * EZMacProReg.name.RCR(Receiver Control Register) * *
 *
 * bit7:
 * 	packet forward enable bit
 *
 * bit[6:3]:
 *	channel mask bits bit[3, 6] -- channel[0, 3], EZMacProReg.name.FR[0-3]
 *
 * bit2:
 * 	search channel enable/disable(1/0)
 *
 * bit[1:0]:
 * 	似乎没有使用
 * */




/*
 * if CID is used:
 *	//set the control byte of the ACK packet( clear ACKREQ bit, set ACK bit)
 *	extIntSpiWriteReg(SI4432_TRANSMIT_HEADER_3, ((EZMacProReg.name.RCTRL & ~0x04) | 0x08 | temp8));
 *	extIntSpiWriteReg(SI4432_TRANSMIT_HEADER_2, EZMacProReg.name.RCID); // copy CID from RCID
 *	extIntSpiWriteReg(SI4432_TRANSMIT_HEADER_1, EZMacProReg.name.SFID); // set Sender ID to Self ID
 *	extIntSpiWriteReg(SI4432_TRANSMIT_HEADER_0, EZMacProReg.name.RSID); // set DID to the Received SID
 * else:
 *	//set the control byte of the ACK packet( clear ACKREQ bit, set ACK bit)
 *	extIntSpiWriteReg(SI4432_TRANSMIT_HEADER_3, ((EZMacProReg.name.RCTRL & ~0x04) | 0x08 | temp8));
 *	extIntSpiWriteReg(SI4432_TRANSMIT_HEADER_2, EZMacProReg.name.SFID); // set Sender ID to Self ID
 *	extIntSpiWriteReg(SI4432_TRANSMIT_HEADER_1, EZMacProReg.name.RSID); // set DID to the Received SID
 *
 * // decrement radius
 * // already checked for non-zero radius
 * temp8 = EZMacProReg.name.RCTRL;
 * temp8--;
 * // write modified RX Header back to TX
 * // The transmit registers are volatile and need to be restored by transmit function
 * // Only Extended packet format supports forwarding
 * // Set the packet headers
 * if CID is used:
 * 	extIntSpiWriteReg(SI4432_TRANSMIT_HEADER_0, EZMacProReg.name.DID);
 * 	extIntSpiWriteReg(SI4432_TRANSMIT_HEADER_1, EZMacProReg.name.RSID);
 * 	extIntSpiWriteReg(SI4432_TRANSMIT_HEADER_2, EZMacProReg.name.RCID);
 * 	extIntSpiWriteReg(SI4432_TRANSMIT_HEADER_3, temp8);
 * else:
 * 	extIntSpiWriteReg(SI4432_TRANSMIT_HEADER_1, EZMacProReg.name.DID);
 * 	extIntSpiWriteReg(SI4432_TRANSMIT_HEADER_2, EZMacProReg.name.RSID);
 * 	extIntSpiWriteReg(SI4432_TRANSMIT_HEADER_3, temp8);
 *
 *
 * if CID is used:
 * 	EZMacProReg.name.RCTRL = extIntSpiReadReg(SI4432_RECEIVED_HEADER_3);
 * 	EZMacProReg.name.RCID  = extIntSpiReadReg(SI4432_RECEIVED_HEADER_2);
 * 	EZMacProReg.name.RSID  = extIntSpiReadReg(SI4432_RECEIVED_HEADER_1);
 * 	EZMacProReg.name.DID   = extIntSpiReadReg(SI4432_RECEIVED_HEADER_0);
 * else:
 * 	EZMacProReg.name.RCTRL = extIntSpiReadReg(SI4432_RECEIVED_HEADER_3);
 * 	EZMacProReg.name.RSID  = extIntSpiReadReg(SI4432_RECEIVED_HEADER_2);
 * 	EZMacProReg.name.DID   = extIntSpiReadReg(SI4432_RECEIVED_HEADER_1);
 *
 *
 *
 *
 * */

/*
 * * EZMacProReg.name.RCTRL(Received Control Byte) * *
 *
 * bit[7:4]:
 *	ForwardedPacketTableEntry.seq, 转发时使用的序号
 *
 * bit3:
 * 	1 - an ACK message
 * 	0 - not an ACK message
 *
 * bit2:
 * 	1 - request ACK
 * 	0 - not request ACK
 *
 * bit[1:0]:
 * 	pkt forward radius
 * */




/*
 * * EZMacProReg.name.RCID(Received Customer ID) * *
 *
 * NO NOTE
 * */




/*
 * * EZMacProReg.name.RSID(Received Sender ID) * *
 *
 * NOTE: 255 is the Sender broadcast address ID
 * */




/*
 * * EZMacProReg.name.DID(Destination ID) * *
 *
 * NO NOTE
 * */






/*
 * * EZMacProReg.name.PFCR(Packet Filter Control Register) * *
 *
 * bit7:
 *  	Customer ID filter enable bit
 *  	1 - enabled, CID will be checked
 *  	0 - disable
 * bit6:
 * 	Sender filter is enable bit
 * 	1 - enable, SID will be checked
 * 	0 - disable
 *
 * bit5:
 * 	Destination filter is enable bit
 * 	1 - enable, DID will be checked
 * 	0 - disable
 *
 * bit4:
 * 	Multi-cast filter is enabled, reference EZMacProReg.name.MCA_MCM(Multicast Address / Multicast Mask)
 * 	1 - enable
 * 	0 - disable
 *
 * bit3:
 * 	Broadcast(0xff) filter is enable bit
 * 	1 - enable, DID will be checked
 * 	0 - disable
 *
 * bit2:
 *	Packet Length filter enable bit
 *	1 - enable, Received Packet length will be checked
 *	0 - disable
 *
 * bit1:
 * 	promiscuous mode enable bit, Enabling the promiscuous mode by setting the PREN bit of the PCFR register
 * 	1 - promiscuous mode, all the address filters and the packet length filter will be ignored
 * 	0 - Packet Filter enable
 *
 * bit0:
 *	Multi-cast address mode enable bit, reference bit4
 *	1 - enable
 *	0 - disable
 * */




/*
 * * EZMacProReg.name.SFLT(Sender ID Filter) * *
 * * EZMacProReg.name.SMSK(Sender ID Filter Mask) * *
 *
 * (rsid & EZMacProReg.name.SMSK) == (EZMacProReg.name.SFLT & EZMacProReg.name.SMSK)
 *
 * */







/*
 * * EZMacProReg.name.MCA_MCM(Multicast Address / Multicast Mask) * *
 *
 * (rdid & EZMacProReg.name.MCA_MCM) == (EZMacProReg.name.SFID & EZMacProReg.name.MCA_MCM)
 * ((EZMacProReg.name.PFCR & 0x01) == 0x01) && (rdid == EZMacProReg.name.MCA_MCM)
 * */




/*
 * * EZMacProReg.name.MPL(Maximum Packet Length) * *
 *
 * NO NOTE
 * */




/*
 * * EZMacProReg.name.MSR(MAC Status Register) * *
 *
 * #define EZMAC_PRO_SLEEP     0x00
 * #define EZMAC_PRO_IDLE      0x40
 * #define EZMAC_PRO_WAKE_UP   0x80
 * #define WAKE_UP_ERROR       0x8F
 *
 * #define TX_STATE_BIT        0x10
 * #define RX_STATE_BIT        0x20
 *
 * bit[3:0] -- [0, 14] use as sub state
 * */




/*
 * * EZMacProReg.name.RSR(Receive Status Register) * *
 *
 * save the receive status to the RSR Mac register
 * bit7:
 * 	1 - (rdid == EZMacProReg.name.SFID)
 * bit6:
 * 	1 - rdid == EZMacProReg.name.MCA_MCM
 * 		(((EZMacProReg.name.PFCR & 0x10) == 0x10) && ((EZMacProReg.name.PFCR & 0x01) == 0x01))
 * bit5:
 * 	1 - (rdid == 0xFF)
 * */




/*
 * * EZMacProReg.name.RFSR(Received Frequency Status Register) * *
 *
 * store the current freq. channel
 * */




/*
 * * EZMacProReg.name.RSSI(Received Signal Strength Indicator) * *
 *
 * extIntSpiReadReg(SI4432_RECEIVED_SIGNAL_STRENGTH_INDICATOR);
 * */




/*
 * * EZMacProReg.name.SCID(Self Customer ID) * *
 *
 * macSpiWriteReg(SI4432_TRANSMIT_HEADER_2, EZMacProReg.name.SCID);
 * */




/*
 * * EZMacProReg.name.SFID(Self ID) * *
 *
 * NO NOTE
 * */




/*
 * * EZMacProReg.name.PLEN(Payload Length) * *
 *
 * EZMacProReg.name.PLEN = extIntSpiReadReg(SI4432_RECEIVED_PACKET_LENGTH)
 * */




/*
 * * EZMacProReg.name.LBTIR(Listen Before Talk  Interval Register) * *
 *
 * bit7:
 * 	1 - bit[6:0] * 100us
 * 	0 - bit[6:0] * (time/byte)
 * */




/*
 * * EZMacProReg.name.LBTLR(Listen Before Talk Limit Register) * *
 *
 *  macSpiWriteReg(SI4432_RSSI_THRESHOLD, EZMacProReg.name.LBTLR);
 *  Register 27h. RSSI Threshold for Clear Channel Indicator
 *
 * */




/*
 * EZRadioPRO-Radio register
 *
 * Note:
 *  The period of the wake-up timer can be calculated as TWUT = (4 x M x 2^R)/32.768 ms.
 *  M = (TWUT * 32.768) / (4 * 2^R)
 *  assume R = 4, then M = (TWUT * 32.768) / (4 * 16) = (TWUT * 32.768) / (64)
 *
 * ***Register 14h. Wake-Up Timer Period 1 (Wake Up Timer Exponent (R) Value)
 *  wtr[4:0], Reset value = xxx00011
 *  Maximum value for R is decimal 20. A value greater than 20 will yield a result as if 20 were written. R Value = 0 can be written here.
 *
 *  Note:
 *  R = 0 is allowed, and the maximum value for R is decimal 20. A value greater than 20 will result in the same as if 20 was written.
 *
 * ***Register 15h. Wake-Up Timer Period 2 (Wake Up Timer Mantissa (M) Value)
 *  wtm[15:8], Reset value = 00000000
 *
 * ***Register 16h. Wake-Up Timer Period 3 (Wake Up Timer Mantissa (M) Value)
 *  wtm[7:0], Reset value = 00000001
 *  M[7:0] = 0 is not valid here. Write at least decimal 1.
 * */


/*
 * EZmacPRO register map to EZRadioPRO-Radio register
 *  LFTMR2 -- SI4432_WAKE_UP_TIMER_PERIOD_1 (R, Maximum is 20(0x14))
 *  LFTMR1 -- SI4432_WAKE_UP_TIMER_PERIOD_2 (M[15:8])
 *  LFTMR0 -- SI4432_WAKE_UP_TIMER_PERIOD_3 (M[7:0])
 * */



/*!
 * LFT timeout macros used exclusively with LFTMR registers.
 */

/* assume WTR = 3, time range[1ms, 67107ms] == range[1ms, 67.107s]
 * (65,535*32.768)/32 = 2,147,450.88 / 32 = 67,107.84
 * */
#define LFTMR2_TIMEOUT_MSEC(t)              (3)
#define LFTMR1_TIMEOUT_MSEC(t)              ((((t*32768) / (4*8*1000)) >> 8) & 0xff)
#define LFTMR0_TIMEOUT_MSEC(t)              ((((t*32768) / (4*8*1000)))      & 0xff)

/* assume WTR = 13, time range[1s, 65535s]
 * 2^13 * 4 = 8192*4 = 32768
 * */
#define LFTMR2_TIMEOUT_SEC(t)               (13)
#define LFTMR1_TIMEOUT_SEC(t)               ((((t*32768) / (4*8192*1)) >> 8) & 0xff)
#define LFTMR0_TIMEOUT_SEC(t)               ((((t*32768) / (4*8192*1)))      & 0xff)

/* 1-enable, 0-disable */
#define LFTMR2_WAKEUP_TIMER_ENABLED_BIT	(0X80)

/* 1-Internal 32KHz RC Oscillator, 0-External 32KHz Oscillator
 *
 * SI4432_OPERATING_AND_FUNCTION_CONTROL_1.bit4(32,768 kHz Crystal Oscillator Select.):
 *  0 -- RC oscillator
 *  1 -- 32 kHz crystal
 *
 * */
#define LFTMR2_32KHZ_OSCILLATOR_BIT	(0x40)



#define STARTUP_TIMEOUT_S                   (3)         // sec


/*
 *
 *
 * */
#define ezmacpro_timer_isr 	timerIntT3_ISR
#define ezmacpro_exti_isr	externalIntISR

#define EZMAC_BROADCAST_ADDR	(0XFF)


enum ezmacpro_state_e {
	EZMACS_WAKEUP	= 0,		/* reset si4432 or exit sleep-mode后，进入idle-mode之前的临时状态  */
	EZMACS_SLEEP		,       /* si4432已进入sleep-mode */
	EZMACS_IDLE		,       /* si4432已进入idle-mode */
	EZMACS_RX		,       /* si4432已进入rx-mode, RX_STATE_FREQUENCY_SEARCH */
	EZMACS_TX		,       /* si4432已进入tx-mode, TX_STATE_LBT_START_LISTEN/TX_STATE_WAIT_FOR_TX */
	EZMACS_ERROR		,       /* WAKE_UP_ERROR / RX_ERROR_STATE / TX_ERROR_STATE */

	EZMACS_LFTIMER_EXPIRED	,        /* 'Low Frequency Timer' timeout, Wake up timer interrupt is occurred */
	EZMACS_LOW_BATTERY	,        /* low battery detect interrupt is occurred */
	EZMACS_SYNCWORD_RECEIVED,        /* sync word detect interrupt is occured */
	EZMACS_CRC_ERR		,        /* CRC error occurred */
	EZMACS_PKT_DISCARED	,        /* 丢弃了接收到的无用数据包(目标地址与本地不符, 也不需要转发) */
	EZMACS_PKT_RECEIVED	,        /* 已接收到数据包, 并且已将数据读入ezmacpro的rx-buffer */
	EZMACS_PKT_FORWARDING	,        /* 即将转发数据包 */
	EZMACS_PKT_SENT		,        /* 数据包发送完成 */
	EZMACS_LBT_TIMEOUT	,	/* ezmacpro LBT timeout, EZMAC_PRO_ERROR_CHANNEL_BUSY
						-(TX_ERROR_CHANNEL_BUSY,RX_ERROR_FORWARDING_WAIT_FOR_TX) */
	EZMACS_ACK_TIMEOUT	,	/* TX_STATE_WAIT_FOR_ACK timeout */
	EZMACS_ACK_SENDING	,       /* 即将自动发送ack, customise Ack Packet payload */
};


#define PAYLOAD_LENGTH_MAX (64)

struct ezmacpro_pkt_header_t {
	ubase_t  dst_id;
};

#ifndef SUCC
#define SUCC  0
#endif

#ifndef FAIL
#define FAIL  1
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

enum send_pkt2ezmacpro_flag_bit_e {
	SPEFB_FLAG_NONE		= 0X00,
	SPEFB_FORCE_SEND	= 0X01,	/* 无论mac处于什么状态, 都发送新数据 */
	SPEFB_SET_MAC_TCR	= 0x02, /* mac Transmit Control Register */
};

#if C51_SYNTAX_
typedef base_t send_pkt2ezmacpro_flag_t;
#else
typedef enum send_pkt2ezmacpro_flag_bit_e send_pkt2ezmacpro_flag_t;
#endif


enum wait_emmacpro_into_state_err_e {
	WEIS_OK,
	WEIS_FAIL,

	WEIS_TX_CHANNEL_BUSY,
	WEIS_TX_ERROR_STATE,
	WEIS_RX_ERROR_STATE,
};
#if C51_SYNTAX_
typedef base_t wait_emmacpro_into_state_err_t;
#else
typedef enum wait_emmacpro_into_state_err_e wait_emmacpro_into_state_err_t;
#endif



#define WAIT_EMMACPRO_INTO_STATE_FOREVER (U16)(~0)

#ifndef set_bit
#define set_bit(bit_vector, mask)     ((bit_vector) |=  (mask))
#endif
#ifndef clr_bit
#define clr_bit(bit_vector, mask)     ((bit_vector) &= ~(mask))
#endif
#ifndef reverse_bit
#define reverse_bit(bit_vector, mask) ((bit_vector) ^=  (mask))
#endif
#ifndef is_bit_set
#define is_bit_set(bit_vector, mask)  (!!((bit_vector) & (mask)))
#endif
#ifndef is_bit_clr
#define is_bit_clr(bit_vector, mask)  (!((bit_vector) &  (mask)))
#endif

extern wait_emmacpro_into_state_err_t wait_emmacpro_into_state(enum ezmacpro_state_e state, U16 timeout_ms);

extern int sent_pkt_to_ezmacpro(struct ezmacpro_pkt_header_t *h, U8 *payload, ubase_t len, send_pkt2ezmacpro_flag_t flag);
extern int recv_pkt_from_ezmacpro(U8 *buf, ubase_t buf_len, ubase_t *payload_len, ubase_t *rssi);
extern int write_ezmacpro_reg_check_if_succ(MacRegs  name, U8 value);
extern int recv_pkt_from_ezmacpro_prepare(void);

extern ubase_t get_ezmacproc_fsm_state(void);
extern int set_ezmacpro_secr_next_state_ctrl_bits(base_t is_after_tx, ubase_t next_state);

extern void print_callback_var(void);

extern int is_ezmac_send_pkt_over(void);
extern int is_ezmac_had_recv_pkt(void);
extern int is_ezmac_in_recv_state(void);
extern int is_ezmac_in_idle_state(void);
extern uint8_t get_pkt_sid(void);
extern uint8_t get_pkt_did(void);
extern base_t set_ezmac_into_idle_state(void);

INTERRUPT(externalIntISR, INTERRUPT_INT0);
INTERRUPT(timerIntT3_ISR, INTERRUPT_TIMER3);


#endif /* MS_COMMON_H_ */
