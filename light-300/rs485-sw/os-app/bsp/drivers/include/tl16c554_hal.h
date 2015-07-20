/*
 ******************************************************************************
 * tl16c554_hal.h
 *
 *  Created on: 2014-12-31
 *      Author: David, zhaoshaowei@yeejoin.com
 *
 * COPYRIGHT (C) 2014, YEEJOIN (Beijing) Technology Development Co., Ltd.
 ******************************************************************************
 */


#ifndef TL16C554_HAL_H_
#define TL16C554_HAL_H_

/*
 * page15 - page16
 * CONTROL 		  MNEMONIC 	STATUS 			MNEMONIC DATA 				MNEMONIC
 * Line control register  LCR 		Line status register 	LSR 	 Receiver buffer register 	RBR
 * FIFO control register  FCR 		Modem status register 	MSR 	 Transmitter holding register 	THR
 * Modem control register MCR
 * Divisor latch LSB 	  DLL
 * Divisor latch MSB 	  DLM
 * Interrupt enable register IER
 *
 * DLAB is the divisor latch access bit and bit 7 in the LCR.
 *
 * The scratch register is an 8-bit read/write register that has no affect on either channel in the ACE.
 * It is intended to be used by the programmer to hold data temporarily.
 *
 *
 * REGISTER/SIGNAL			RESET CONTROL	RESET STATE
 * Interrupt enable register 		Reset 		All bits cleared (0 −3 forced and 4 −7 permanent)
 * Interrupt identification register 	Reset 		Bit 0 is set, bits 1, 2, 3, 6, and 7 are cleared, Bits 4 −5 are permanently cleared
 * Line control register 		Reset 		All bits cleared
 * Modem control register 		Reset 		All bits cleared (5 −7 permanent)
 * FIFO control register 		Reset 		All bits cleared
 * Line status register 		Reset 		All bits cleared, except bits 5 and 6 are set
 * Modem status register 		Reset 		Bits 0 −3 cleared, bits 4 −7 input signals
 * TXx 					Reset 		High
 * Interrupt (RCVR ERRS) 		Read LSR/Reset 	Low
 * Interrupt (receiver data ready) 	Read RBR/Reset 	Low
 * Interrupt (THRE)			Read IIR/Write THR/Reset 	Low
 * Interrupt (modem status changes)	Read MSR/Reset 	Low
 * RTS 					Reset 		High
 * DTR 					Reset 		High
 * */
#define TL16_RBR_REG	0	/* Receiver buffer register, read only, DLAB=0 */
#define TL16_THR_REG	0	/* Transmitter holding register, write only, DLAB=0 */
#define TL16_DLL_REG	0	/* Divisor latch LSB, DLAB=1 */
#define TL16_DLM_REG	1	/* Divisor latch MSB, DLAB=1 */
#define TL16_IER_REG	1	/* Interrupt enable register, DLAB=0 */
#define TL16_FCR_REG	2	/* FIFO control register, write only */
#define TL16_IIR_REG	2	/* interrupt identification register, read only */
#define TL16_LCR_REG	3	/* Line control register */
#define TL16_MCR_REG	4	/* Modem control register */
#define TL16_LSR_REG	5	/* Line status register */
#define TL16_MSR_REG	6	/* Modem status register */
#define TL16_SCR_REG	7	/* scratch pad register */

#define TL16_X_CSX_ADDR_OFFSET	8
#define create_tl16_reg_addr(cs, reg)		((cs)<<TL16_X_CSX_ADDR_OFFSET | (reg))


enum tl16_x_csx_addr_e {
	TL16_1_CSA_ADDR	= 1,
	TL16_1_CSB_ADDR,
	TL16_1_CSC_ADDR,
	TL16_1_CSD_ADDR,

	TL16_2_CSA_ADDR,
	TL16_2_CSB_ADDR,
	TL16_2_CSC_ADDR,
	TL16_2_CSD_ADDR,

};

enum tl16c554_data_bits_e {
	TCL16C554_DATABITS_5 =	5,
	TCL16C554_DATABITS_6,
	TCL16C554_DATABITS_7,
	TCL16C554_DATABITS_8,
};

/* LCR.2: 1 = 1.5 Stop Bits if 5 Data Bits Selected 2 Stop Bits if 6, 7, 8 Data Bits Selected */
enum tl16c554_stop_bits_e {
	TCL16C554_STOPBITS_1	= 1,
	TCL16C554_STOPBITS_15_OR_2
};

enum tl16c554_parity_e {
	TCL16C554_PARITY_NONE	= 0,
	TCL16C554_PARITY_EVEN,	/* 偶校验 */
	TCL16C554_PARITY_ODD	/* 奇校验 */
};

/*
 * IER -- interrupt enable register
 * bit7: 0
 * bit6: 0
 * bit5: 0
 * bit4: 0
 * bit3: (EDSSI) Enable modem status interrupt
 * bit2: (ERLSI) Enable receiver line status interrupt
 * bit1: (ETBEI) Enable transmitter holding register empty interrupt
 * bit0: (ERBI) Enable received data available interrupt
 * */
enum tl16c554_ie_e {
	TL16C554_MODEM_STATUS,
	TL16C554_RX_LINE_STATUS,
	TL16C554_TX_HOLD_REG_EMPTY,
	TL16C554_RX_DATA_AVAILABLE
};


#define if_can_send2tl16_uart(x_csx)	(tl16_read_lsr(x_csx) & (1<<5))
#define if_tl16_uart_not_empty(x_csx)	(tl16_read_lsr(x_csx) & (1))

extern uint8_t tl16_read_reg(uint32_t addr);
extern void tl16_write_reg(uint32_t addr, uint8_t data);

extern int tl16_set_buadrate(enum tl16_x_csx_addr_e x_csx, int baud);
extern int tl16_set_uart_other_param(enum tl16_x_csx_addr_e x_csx, enum tl16c554_data_bits_e databits,
		enum tl16c554_stop_bits_e stopbits, enum tl16c554_parity_e parity);
extern int tl16c554_chip_is_avaible(int chip_no);
extern int tl16_uart_param_check(rt_uint32_t baud, rt_uint8_t datab, rt_uint8_t parity, rt_uint8_t stopb);
extern int tl16_set_fifo_mode(enum tl16_x_csx_addr_e x_csx, int is_fifo);
extern int tl16_set_ier(enum tl16_x_csx_addr_e x_csx, enum tl16c554_ie_e ie, int is_enable);
extern uint8_t tl16_read_iir(enum tl16_x_csx_addr_e x_csx);
extern uint8_t tl16_read_lsr(enum tl16_x_csx_addr_e x_csx);
extern int tl16_set_loop_mode(enum tl16_x_csx_addr_e x_csx, int is_loop);


#endif /* TL16C554_HAL_H_ */
