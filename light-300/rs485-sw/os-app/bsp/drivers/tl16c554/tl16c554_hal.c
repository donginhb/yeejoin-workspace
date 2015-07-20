/*
 ******************************************************************************
 * tl16c554_hal.c
 *
 *  Created on: 2014-12-31
 *      Author: David, zhaoshaowei@yeejoin.com
 *
 * COPYRIGHT (C) 2014, YEEJOIN (Beijing) Technology Development Co., Ltd.
 ******************************************************************************
 */

/*
 * tw3 Pulse duration, RESET min--1000ns
 * */
#include <rtdef.h>
#include <rtthread.h>

#include <board.h>

#include <tl16c554_hal.h>
#include "tl16c554_hal_priv.h"

#define tl16_debug(x) 	//rt_kprintf x
#define tl16_log(x) 	rt_kprintf x


#define TL16C554_ADDR_BANK_OFFSET	0X60000000


static void tl16c554_cs_act_f(enum tl16_x_csx_addr_e x_csx);
static void tl16c554_cs_deact_f(enum tl16_x_csx_addr_e x_csx);
static int tl16_get_baud_divisor(int baud, uint32_t *div);
static int tl16_clr_divisor(enum tl16_x_csx_addr_e x_csx);



uint8_t tl16_read_reg(uint32_t addr)
{
	int x_csx;
	volatile char *reg;
	uint8_t retval;

	x_csx = get_cs_code_from_regaddr(addr);

	reg = (char *)get_reg_from_regaddr(addr);
	reg += TL16C554_ADDR_BANK_OFFSET;

	tl16c554_cs_act_f(x_csx);
	retval = *reg;
	tl16c554_cs_deact_f(x_csx);

	tl16_debug(("%s(), addr:0x%x, x_csx:0x%x, reg:0x%x\n", __func__, addr, x_csx, reg));

	return retval;
}

void tl16_write_reg(uint32_t addr, uint8_t data)
{
	int x_csx;
	volatile char *reg;

	x_csx = get_cs_code_from_regaddr(addr);

	reg = (char *)get_reg_from_regaddr(addr);
	reg += TL16C554_ADDR_BANK_OFFSET;

	tl16c554_cs_act_f(x_csx);
	*reg = data;
	tl16c554_cs_deact_f(x_csx);

	tl16_debug(("%s(), addr:0x%x, x_csx:0x%x, reg:0x%x\n", __func__, addr, x_csx, reg));

	return;
}

int tl16_enable_chx(enum tl16_x_csx_addr_e x_csx, int baud)
{
	return tl16_set_buadrate(x_csx, baud);
}

int tl16_disable_chx(enum tl16_x_csx_addr_e x_csx)
{
	return tl16_clr_divisor(x_csx);
}

/*
 * 8-MHz Clock
 * */
int tl16_set_buadrate(enum tl16_x_csx_addr_e x_csx, int baud)
{
	uint32_t regaddr;
	uint32_t regval;
	uint32_t divisor;

	if (!is_x_csx_valid(x_csx)) {
		tl16_log(("%s():tl16_x_csx(%d) invalid\n", __func__, x_csx));
		return FALSE;
	}

	regaddr = create_tl16_reg_addr(x_csx, TL16_LCR_REG);

	regval = tl16_read_reg(regaddr);
	regval |= 1<<7;
	tl16_write_reg(regaddr, regval);

	if (SUCC == tl16_get_baud_divisor(baud, &divisor)) {
		tl16_write_reg(create_tl16_reg_addr(x_csx, TL16_DLM_REG), divisor>>8 & 0xff);
		tl16_write_reg(create_tl16_reg_addr(x_csx, TL16_DLL_REG), divisor    & 0xff);
	} else {
		tl16_log(("%s():tl16_x_csx(%d) tl16_get_baud_divisor fail\n", __func__, x_csx));
	}

	regval = tl16_read_reg(regaddr);
	regval &= ~(1<<7);
	tl16_write_reg(regaddr, regval);

	return SUCC;
}


int tl16_set_uart_other_param(enum tl16_x_csx_addr_e x_csx, enum tl16c554_data_bits_e databits,
		enum tl16c554_stop_bits_e stopbits, enum tl16c554_parity_e parity)
{
	uint32_t regaddr;
	uint32_t regval;
	uint8_t temp;

	if (!is_x_csx_valid(x_csx)) {
		tl16_log(("%s():tl16_x_csx(%d) invalid\n", __func__, x_csx));
		return FALSE;
	}

	temp = 0;

	switch (databits) {
	case TCL16C554_DATABITS_5:
		/* temp |= 0; */
		break;

	case TCL16C554_DATABITS_6:
		temp |= 1;
		break;

	case TCL16C554_DATABITS_7:
		temp |= 2;
		break;

	case TCL16C554_DATABITS_8:
		temp |= 3;
		break;

	default:
		tl16_log(("%s(), x_csx-%d:databits(%d) invalid\n", __func__, x_csx, databits));
		goto err;
	}

	switch (stopbits) {
	case TCL16C554_STOPBITS_1:
		/* temp |= 0; */
		break;

	case TCL16C554_STOPBITS_15_OR_2:
		temp |= 1<<2;
		break;

	default:
		tl16_log(("%s(), x_csx-%d:stopbits(%d) invalid\n", __func__, x_csx, stopbits));
		goto err;
	}

	switch (parity) {
	case TCL16C554_PARITY_NONE:
		/* temp |= 0; */
		break;

	case TCL16C554_PARITY_EVEN:
		temp |= 1<<3 | 1<<4;
		break;

	case TCL16C554_PARITY_ODD:
		temp |= 1<<3;
		break;

	default:
		tl16_log(("%s(), x_csx-%d:parity(%d) invalid\n", __func__, x_csx, parity));
		goto err;
	}

	regaddr = create_tl16_reg_addr(x_csx, TL16_LCR_REG);

	regval = tl16_read_reg(regaddr);
	regval &= ~(0x1ff);
	regval |= temp;
	tl16_write_reg(regaddr, regval);

	return SUCC;

err:
	return FAIL;
}




int tl16_uart_param_check(rt_uint32_t baud, rt_uint8_t datab, rt_uint8_t parity, rt_uint8_t stopb)
{
	int ret = SUCC;

	switch (baud) {
	case 300:	case 600:	case 1200:	case 1800:
	case 2000:	case 2400:	case 3600:	case 4800:
	case 7200:	case 9600:	case 19200:	case 38400:
	case 56000:	case 128000:
		break;
	default:
		tl16_log(("%s(), invalid baud(%d)\n", __func__, baud));
		ret = FAIL;
		goto ret_entry;
	}

	switch (datab) {
	case 5:	case 6:	case 7:	case 8:
		break;
	default:
		tl16_log(("%s(), invalid datab(%d)\n", __func__, datab));
		ret = FAIL;
		goto ret_entry;
	}

	switch (parity) {
	case 0:	case 1:	case 2:
		break;
	default:
		tl16_log(("%s(), invalid parity(%d)\n", __func__, parity));
		ret = FAIL;
		goto ret_entry;
	}

	switch (stopb) {
	case 1:	case 2:
		break;
	case 15:
		if (5!=datab) {
			tl16_log(("%s(), invalid stopb(%d)\n", __func__, stopb));
			ret = FAIL;
			goto ret_entry;
		}
		break;
	default:
		tl16_log(("%s(), invalid stopb(%d)\n", __func__, stopb));
		ret = FAIL;
		goto ret_entry;
	}

ret_entry:
	return ret;
}


int tl16c554_chip_is_avaible(int chip_no)
{
	enum tl16_x_csx_addr_e x_csx;
	uint32_t regaddr;
	uint32_t regval;
	uint32_t temp;
	int i;

	if (0 == chip_no) {
		x_csx = TL16_1_CSA_ADDR;
	} else if (1 == chip_no) {
		x_csx = TL16_2_CSA_ADDR;
	} else {
		tl16_log(("%s():chip_no(%d) invalid\n", __func__, chip_no));
		return FALSE;
	}

	tl16_log(("will write 0xb0, 0xb1, 0xb2, 0xb3 to channel a/b/c/d SCR\n"));
	regval = 0xb0;
	for (i=0; i<4; ++i) {
		regaddr = create_tl16_reg_addr(x_csx, TL16_SCR_REG);
		tl16_write_reg(regaddr, regval);
		++x_csx;
		++regval;
	}

	x_csx -= 4;
	temp = 0;
	for (i=0; i<4; ++i) {
		regaddr = create_tl16_reg_addr(x_csx, TL16_SCR_REG);
		regval = tl16_read_reg(regaddr);
		++x_csx;

		temp |= regval << i*8;
	}

	tl16_log(("read from channel a/b/c/d SCR:#%x, #%x, #%x, #%x\n\n",
			temp & 0xff, temp>>8 & 0xff, temp>>16 & 0xff, temp>>24 & 0xff));

	if (0xb3b2b1b0 != temp) {
		tl16_log(("tl16c554 chip(%d) is not avaible\n\n", chip_no));
		return FALSE;
	} else {
		return TRUE;
	}

}

/*
 * FCR (write only)
 * bit7:Receiver Trigger(MSB)
 * bit6:Receiver Trigger(LSB) 00b-1, 01b-4, 10b-8, 11b-14
 * bit5:Reserved
 * bit4:Reserved
 * bit3:DMA mode select
 * bit2:Transmit FIFO reset
 * bit1:Receiver FIFO reset
 * bit0:FIFO Enable
 *
 * In mode 0, RXRDY is asserted (low) when the receive FIFO is not empty; it is released (high) when the FIFO
 * is empty. In this way, the receiver FIFO is read when RXRDY is asserted (low).
 *
 * In mode 1, RXRDY is asserted (low) when the receive FIFO has filled to the trigger level or a character time-out
 * has occurred (four character times with no transmission of characters); it is released (high) when the FIFO is
 * empty. In this mode, multiple received characters are read by the DMA device, reducing the number of times
 * it is interrupted.
 * */
#define TL16_FCR_FIFO_MODE	0X49	/* 0b01001001 */
int tl16_set_fifo_mode(enum tl16_x_csx_addr_e x_csx, int is_fifo)
{
	uint32_t regaddr;

	if (!is_x_csx_valid(x_csx)) {
		tl16_log(("%s():tl16_x_csx(%d) invalid\n", __func__, x_csx));
		return FALSE;
	}

	regaddr = create_tl16_reg_addr(x_csx, TL16_FCR_REG);
	if (is_fifo) {
		tl16_write_reg(regaddr, TL16_FCR_FIFO_MODE);
	} else {
		tl16_write_reg(regaddr, 0);
	}

	return SUCC;
}

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
 *
 *
 * */
int tl16_set_ier(enum tl16_x_csx_addr_e x_csx, enum tl16c554_ie_e ie, int is_enable)
{
	uint32_t regaddr;
	uint32_t regval;

	if (!is_x_csx_valid(x_csx)) {
		tl16_log(("%s():tl16_x_csx(%d) invalid\n", __func__, x_csx));
		return FALSE;
	}

	regaddr = create_tl16_reg_addr(x_csx, TL16_IER_REG);
	regval  = tl16_read_reg(regaddr);

	if (is_enable) {
		switch (ie) {
		case TL16C554_MODEM_STATUS:
			regval |= 1<<3;
			break;

		case TL16C554_RX_LINE_STATUS:
			regval |= 1<<2;
			break;

		case TL16C554_TX_HOLD_REG_EMPTY:
			regval |= 1<<1;
			break;

		case TL16C554_RX_DATA_AVAILABLE:
			regval |= 1;
			break;

		default:
			tl16_log(("%s(): tl16c554_ie(%d) invalid\n", __func__, ie));
			break;
		}
	} else {
		switch (ie) {
		case TL16C554_MODEM_STATUS:
			regval &= ~(1<<3);
			break;

		case TL16C554_RX_LINE_STATUS:
			regval &= ~(1<<2);
			break;

		case TL16C554_TX_HOLD_REG_EMPTY:
			regval &= ~(1<<1);
			break;

		case TL16C554_RX_DATA_AVAILABLE:
			regval &= ~(1);
			break;

		default:
			tl16_log(("%s(): **tl16c554_ie(%d) invalid\n", __func__, ie));
			break;
		}

	}

	tl16_write_reg(regaddr, regval);

	return SUCC;
}

/*
 * Bit 0: 	 IIR0 indicates whether an interrupt is pending. When IIR0 is cleared, an interrupt is pending.
 * Bits 1 and 2: IIR1 and IIR2 identify the highest priority interrupt pending as indicated in Table 5.
 * Bit 3: 	 IIR3 is always cleared when in the TL16C450 mode. This bit is set along with bit 2 when in the FIFO
 * 		 mode and a trigger change level interrupt is pending.
 * Bits 4 and 5: IIR4 and IIR5 are always cleared.
 * Bits 6 and 7: IIR6 and IIR7 are set when FCR0 = 1.
 * */
uint8_t tl16_read_iir(enum tl16_x_csx_addr_e x_csx)
{
	uint32_t regaddr;
	uint32_t regval;

	if (!is_x_csx_valid(x_csx)) {
		tl16_log(("%s():tl16_x_csx(%d) invalid\n", __func__, x_csx));
		return 0x01;
	}

	regaddr = create_tl16_reg_addr(x_csx, TL16_IIR_REG);
	regval  = tl16_read_reg(regaddr);

	return regval;
}

/*
 * LSR BITS						1		0
 * LSR0 data ready (DR)					Ready 		Not ready
 * LSR1 overrun error (OE) 				Error 		No error
 * LSR2 parity error (PE) 				Error 		No error
 * LSR3 framing error (FE) 				Error 		No error
 * LSR4 break interrupt (BI) 				Break 		No break
 * LSR5 transmitter holding register empty (THRE) 	Empty 		Not empty
 * LSR6 transmitter register empty (TEMT)		Empty 		Not empty
 * LSR7 receiver FIFO error				Error in FIFO 	No error in FIFO
 * */
uint8_t tl16_read_lsr(enum tl16_x_csx_addr_e x_csx)
{
	uint32_t regaddr;
	uint32_t regval;

	if (!is_x_csx_valid(x_csx)) {
		tl16_log(("%s():tl16_x_csx(%d) invalid\n", __func__, x_csx));
		return 0;
	}

	regaddr = create_tl16_reg_addr(x_csx, TL16_LSR_REG);
	regval  = tl16_read_reg(regaddr);

	return regval;
}

/*
 * Bit 4: MCR4 provides a local loopback feature for diagnostic testing of the channel. When MCR4 is set,
 * serial output TXx is set to the marking (high) state and SIN is disconnected. The output of the TSR is looped
 * back into the RSR input. The four modem control inputs (CTS, DSR, DCD, and RI) are disconnected. The
 * modem control outputs (DTR and RTS) are internally connected to the four modem control inputs. The
 * modem control output terminals are forced to their inactive (high) state on the TL16C554. In the diagnostic
 * mode, data transmitted is immediately received. This allows the processor to verify the transmit and receive
 * data paths of the selected serial channel. Interrupt control is fully operational; however, interrupts are
 * generated by controlling the lower four MCR bits internally. Interrupts are not generated by activity on the
 * external terminals represented by those four bits.
 * */
int tl16_set_loop_mode(enum tl16_x_csx_addr_e x_csx, int is_loop)
{
	uint32_t regaddr;
	uint32_t regval;

	if (!is_x_csx_valid(x_csx)) {
		tl16_log(("%s():tl16_x_csx(%d) invalid\n", __func__, x_csx));
		return FALSE;
	}

	regaddr = create_tl16_reg_addr(x_csx, TL16_MCR_REG);
	regval  = tl16_read_reg(regaddr);
	if (is_loop) {
		regval |= 1<<4;
	} else {
		regval &= ~(1<<4);
	}
	tl16_write_reg(regaddr, regval);

	return SUCC;
}


static void tl16c554_cs_act_f(enum tl16_x_csx_addr_e x_csx)
{
	switch (x_csx) {
	case TL16_1_CSA_ADDR:
		tl16c554_1_csa_act();
		break;
	case TL16_1_CSB_ADDR:
		tl16c554_1_csb_act();
		break;
	case TL16_1_CSC_ADDR:
		tl16c554_1_csc_act();
		break;
	case TL16_1_CSD_ADDR:
		tl16c554_1_csd_act();
		break;

	case TL16_2_CSA_ADDR:
		tl16c554_2_csa_act();
		break;
	case TL16_2_CSB_ADDR:
		tl16c554_2_csb_act();
		break;
	case TL16_2_CSC_ADDR:
		tl16c554_2_csc_act();
		break;
	case TL16_2_CSD_ADDR:
		tl16c554_2_csd_act();
		break;

	default:
		tl16_log(("%s():tl16_x_csx invalid\n", __func__));
		break;
	}
}

static void tl16c554_cs_deact_f(enum tl16_x_csx_addr_e x_csx)
{
	switch (x_csx) {
	case TL16_1_CSA_ADDR:
		tl16c554_1_csa_deact();
		break;
	case TL16_1_CSB_ADDR:
		tl16c554_1_csb_deact();
		break;
	case TL16_1_CSC_ADDR:
		tl16c554_1_csc_deact();
		break;
	case TL16_1_CSD_ADDR:
		tl16c554_1_csd_deact();
		break;

	case TL16_2_CSA_ADDR:
		tl16c554_2_csa_deact();
		break;
	case TL16_2_CSB_ADDR:
		tl16c554_2_csb_deact();
		break;
	case TL16_2_CSC_ADDR:
		tl16c554_2_csc_deact();
		break;
	case TL16_2_CSD_ADDR:
		tl16c554_2_csd_deact();
		break;

	default:
		tl16_log(("%s():tl16_x_csx invalid\n", __func__));
		break;
	}
}


static int tl16_get_baud_divisor(int baud, uint32_t *div)
{
	uint32_t divisor;

	switch (baud) {
	case 300:
		divisor = 1667;
		break;

	case 600:
		divisor = 883;
		break;

	case 1200:
		divisor = 417;
		break;

	case 1800:
		divisor = 277;
		break;

	case 2000:
		divisor = 250;
		break;

	case 2400:
		divisor = 208;
		break;

	case 3600:
		divisor = 139;
		break;

	case 4800:
		divisor = 104;
		break;

	case 7200:
		divisor = 69;
		break;

	case 9600:
		divisor = 52;
		break;

	case 19200:
		divisor = 26;
		break;

	case 38400:
		divisor = 13;
		break;

	case 56000:
		divisor = 9;
		break;

	case 128000:
		divisor = 4;
		break;
	case 0:
		divisor = 0;
		break;

	default:
		tl16_log(("%s(), invalid baud(%d)\n", __func__, baud));
		return FALSE;
	}

	*div = divisor;

	return SUCC;
}

static int tl16_clr_divisor(enum tl16_x_csx_addr_e x_csx)
{
	uint32_t regaddr;
	uint32_t regval;

	if (!is_x_csx_valid(x_csx)) {
		tl16_log(("%s():tl16_x_csx(%d) invalid\n", __func__, x_csx));
		return FALSE;
	}

	regaddr = create_tl16_reg_addr(x_csx, TL16_LCR_REG);

	regval = tl16_read_reg(regaddr);
	regval |= 1<<7;
	tl16_write_reg(regaddr, regval);

	tl16_write_reg(create_tl16_reg_addr(x_csx, TL16_DLM_REG), 0);
	tl16_write_reg(create_tl16_reg_addr(x_csx, TL16_DLL_REG), 0);

	regval = tl16_read_reg(regaddr);
	regval &= ~(1<<7);
	tl16_write_reg(regaddr, regval);

	return SUCC;
}

#if 1
#include <finsh.h>
extern void print_tl16_uart_err_info(void);

static int tl16_dev_rw_data(int ch_no, int is_read, int data);

static void print_tl16_state(void)
{
	int i, j;
	unsigned char buf[8+2];
	uint32_t regaddr;
	uint32_t regval;
	uint32_t divisor;


	for (j=TL16_1_CSA_ADDR; j<=TL16_2_CSD_ADDR; ++j) {
		for (i=1; i<8; ++i) {
			buf[i] = tl16_read_reg(create_tl16_reg_addr(j, i));
		}

		buf[0] = tl16_read_reg(create_tl16_reg_addr(j, 0));

		regaddr = create_tl16_reg_addr(j, TL16_LCR_REG);
		regval = tl16_read_reg(regaddr);
		regval |= 1<<7;
		tl16_write_reg(regaddr, regval);

		divisor = tl16_read_reg(create_tl16_reg_addr(j, TL16_DLM_REG));
		divisor <<= 8;
		divisor |= tl16_read_reg(create_tl16_reg_addr(j, TL16_DLL_REG));

		regval = tl16_read_reg(regaddr);
		regval &= ~(1<<7);
		tl16_write_reg(regaddr, regval);

		printf_syn("[ch %d]rbr:0x%x, ier:0x%x, iir:0x%x, lcr:0x%x, mcr:0x%x, "
				"lsr:0x%x, msr:0x%x, scr:0x%x, divisor:%d\n",
				j, buf[0], buf[1], buf[2], buf[3], buf[4],
				buf[5], buf[6], buf[7], divisor);
	}

	return;
}

static void tl16_init_for_test_isr(int ch)
{
	if (ch<1 || ch>8) {
		printf_syn("%s(), param error(%d)\n", __func__, ch);
		return;
	}

	ch += TL16_1_CSA_ADDR - 1;

	tl16_set_buadrate(ch, 9600);
	tl16_set_uart_other_param(ch, 8, 1, 0);
	tl16_set_fifo_mode(ch, 1);

//	tl16_set_loop_mode(ch, 1);

	tl16_set_ier(ch, TL16C554_RX_LINE_STATUS, 1);
	tl16_set_ier(ch, TL16C554_RX_DATA_AVAILABLE, 1);
//	tl16_set_ier(TL16_1_CSA_ADDR, TL16C554_TX_HOLD_REG_EMPTY, 1);

}


int tl16c554_chip_is_avaible(int chip_no);

void tl16_test(int cmd, unsigned data)
{
	uint32_t regaddr;
	uint32_t reg_val;

	printf_syn("%s(), cmd:%d, data:0x%x\n", __func__, cmd, data);

	switch (cmd) {
	case 1:
		tl16c554_chip_is_avaible(0);
		tl16c554_chip_is_avaible(1);
		break;

	case 2:
		switch (data) {
		case 1:
			reg_val = tl16_read_reg(create_tl16_reg_addr(TL16_1_CSA_ADDR, TL16_RBR_REG));
			printf_syn("[ch %d]rbr:0x%x\n", data, reg_val);
			break;
		case 2:
			reg_val = tl16_read_reg(create_tl16_reg_addr(TL16_1_CSB_ADDR, TL16_RBR_REG));
			printf_syn("[ch %d]rbr:0x%x\n", data, reg_val);
			break;
		case 3:
			reg_val = tl16_read_reg(create_tl16_reg_addr(TL16_1_CSC_ADDR, TL16_RBR_REG));
			printf_syn("[ch %d]rbr:0x%x\n", data, reg_val);
			break;
		case 4:
			reg_val = tl16_read_reg(create_tl16_reg_addr(TL16_1_CSD_ADDR, TL16_RBR_REG));
			printf_syn("[ch %d]rbr:0x%x\n", data, reg_val);
			break;
		case 5:
			reg_val = tl16_read_reg(create_tl16_reg_addr(TL16_2_CSA_ADDR, TL16_RBR_REG));
			printf_syn("[ch %d]rbr:0x%x\n", data, reg_val);
			break;
		case 6:
			reg_val = tl16_read_reg(create_tl16_reg_addr(TL16_2_CSB_ADDR, TL16_RBR_REG));
			printf_syn("[ch %d]rbr:0x%x\n", data, reg_val);
			break;
		case 7:
			reg_val = tl16_read_reg(create_tl16_reg_addr(TL16_2_CSC_ADDR, TL16_RBR_REG));
			printf_syn("[ch %d]rbr:0x%x\n", data, reg_val);
			break;
		case 8:
			reg_val = tl16_read_reg(create_tl16_reg_addr(TL16_2_CSD_ADDR, TL16_RBR_REG));
			printf_syn("[ch %d]rbr:0x%x\n", data, reg_val);
			break;
		}
		break;

	case 3:
		switch (data) {
		case 1:
			tl16_write_reg(create_tl16_reg_addr(TL16_1_CSA_ADDR, TL16_THR_REG), 0xa1);
			printf_syn("[ch %d]write 0x%x to thr\n", data, 0xa1);
			break;
		case 2:
			tl16_write_reg(create_tl16_reg_addr(TL16_1_CSB_ADDR, TL16_THR_REG), 0xa2);
			printf_syn("[ch %d]write 0x%x to thr\n", data, 0xa2);
			break;
		case 3:
			tl16_write_reg(create_tl16_reg_addr(TL16_1_CSC_ADDR, TL16_THR_REG), 0xa3);
			printf_syn("[ch %d]write 0x%x to thr\n", data, 0xa3);
			break;
		case 4:
			tl16_write_reg(create_tl16_reg_addr(TL16_1_CSD_ADDR, TL16_THR_REG), 0xa4);
			printf_syn("[ch %d]write 0x%x to thr\n", data, 0xa4);
			break;
		case 5:
			tl16_write_reg(create_tl16_reg_addr(TL16_2_CSA_ADDR, TL16_THR_REG), 0xa5);
			printf_syn("[ch %d]write 0x%x to thr\n", data, 0xa5);
			break;
		case 6:
			tl16_write_reg(create_tl16_reg_addr(TL16_2_CSB_ADDR, TL16_THR_REG), 0xa6);
			printf_syn("[ch %d]write 0x%x to thr\n", data, 0xa6);
			break;
		case 7:
			tl16_write_reg(create_tl16_reg_addr(TL16_2_CSC_ADDR, TL16_THR_REG), 0xa7);
			printf_syn("[ch %d]write 0x%x to thr\n", data, 0xa7);
			break;
		case 8:
			tl16_write_reg(create_tl16_reg_addr(TL16_2_CSD_ADDR, TL16_THR_REG), 0xa0);
			printf_syn("[ch %d]write 0x%x to thr\n", data, 0xa0);
			break;
		}
		break;

	case 40:
		switch (data) {
		case 1:
			reg_val = tl16_read_reg(create_tl16_reg_addr(TL16_1_CSA_ADDR, TL16_IIR_REG));
			printf_syn("[ch %d]iir:#%x\n", data, reg_val);
			break;
		case 2:
			reg_val = tl16_read_reg(create_tl16_reg_addr(TL16_1_CSB_ADDR, TL16_IIR_REG));
			printf_syn("[ch %d]iir:#%x\n", data, reg_val);
			break;
		case 3:
			reg_val = tl16_read_reg(create_tl16_reg_addr(TL16_1_CSC_ADDR, TL16_IIR_REG));
			printf_syn("[ch %d]iir:#%x\n", data, reg_val);
			break;
		case 4:
			reg_val = tl16_read_reg(create_tl16_reg_addr(TL16_1_CSD_ADDR, TL16_IIR_REG));
			printf_syn("[ch %d]iir:#%x\n", data, reg_val);
			break;
		case 5:
			reg_val = tl16_read_reg(create_tl16_reg_addr(TL16_2_CSA_ADDR, TL16_IIR_REG));
			printf_syn("[ch %d]iir:#%x\n", data, reg_val);
			break;
		case 6:
			reg_val = tl16_read_reg(create_tl16_reg_addr(TL16_2_CSB_ADDR, TL16_IIR_REG));
			printf_syn("[ch %d]iir:#%x\n", data, reg_val);
			break;
		case 7:
			reg_val = tl16_read_reg(create_tl16_reg_addr(TL16_2_CSC_ADDR, TL16_IIR_REG));
			printf_syn("[ch %d]iir:#%x\n", data, reg_val);
			break;
		case 8:
			reg_val = tl16_read_reg(create_tl16_reg_addr(TL16_2_CSD_ADDR, TL16_IIR_REG));
			printf_syn("[ch %d]iir:#%x\n", data, reg_val);
			break;
		}

		break;
	case 41:
		switch (data) {
		case 1:
			reg_val = tl16_read_reg(create_tl16_reg_addr(TL16_1_CSA_ADDR, TL16_LCR_REG));
			printf_syn("[ch %d]lcr:#%x\n", data, reg_val);
			break;
		case 2:
			reg_val = tl16_read_reg(create_tl16_reg_addr(TL16_1_CSB_ADDR, TL16_LCR_REG));
			printf_syn("[ch %d]lcr:#%x\n", data, reg_val);
			break;
		case 3:
			reg_val = tl16_read_reg(create_tl16_reg_addr(TL16_1_CSC_ADDR, TL16_LCR_REG));
			printf_syn("[ch %d]lcr:#%x\n", data, reg_val);
			break;
		case 4:
			reg_val = tl16_read_reg(create_tl16_reg_addr(TL16_1_CSD_ADDR, TL16_LCR_REG));
			printf_syn("[ch %d]lcr:#%x\n", data, reg_val);
			break;
		case 5:
			reg_val = tl16_read_reg(create_tl16_reg_addr(TL16_2_CSA_ADDR, TL16_LCR_REG));
			printf_syn("[ch %d]lcr:#%x\n", data, reg_val);
			break;
		case 6:
			reg_val = tl16_read_reg(create_tl16_reg_addr(TL16_2_CSB_ADDR, TL16_LCR_REG));
			printf_syn("[ch %d]lcr:#%x\n", data, reg_val);
			break;
		case 7:
			reg_val = tl16_read_reg(create_tl16_reg_addr(TL16_2_CSC_ADDR, TL16_LCR_REG));
			printf_syn("[ch %d]lcr:#%x\n", data, reg_val);
			break;
		case 8:
			reg_val = tl16_read_reg(create_tl16_reg_addr(TL16_2_CSD_ADDR, TL16_LCR_REG));
			printf_syn("[ch %d]lcr:#%x\n", data, reg_val);
			break;
		}

		break;

	case 5:
		switch (data) {
		case 1:
			tl16_write_reg(create_tl16_reg_addr(TL16_1_CSA_ADDR, TL16_SCR_REG), 0xa8);
			printf_syn("[ch %d]write scr:#%x\n", data, 0xa8);
			break;
		case 2:
			tl16_write_reg(create_tl16_reg_addr(TL16_1_CSB_ADDR, TL16_SCR_REG), 0xa9);
			printf_syn("[ch %d]write scr:#%x\n", data, 0xa9);
			break;
		case 3:
			tl16_write_reg(create_tl16_reg_addr(TL16_1_CSC_ADDR, TL16_SCR_REG), 0xaa);
			printf_syn("[ch %d]write scr:#%x\n", data, 0xaa);
			break;
		case 4:
			tl16_write_reg(create_tl16_reg_addr(TL16_1_CSD_ADDR, TL16_SCR_REG), 0xab);
			printf_syn("[ch %d]write scr:#%x\n", data, 0xab);
			break;
		case 5:
			tl16_write_reg(create_tl16_reg_addr(TL16_2_CSA_ADDR, TL16_SCR_REG), 0xac);
			printf_syn("[ch %d]write scr:#%x\n", data, 0xac);
			break;
		case 6:
			tl16_write_reg(create_tl16_reg_addr(TL16_2_CSB_ADDR, TL16_SCR_REG), 0xad);
			printf_syn("[ch %d]write scr:#%x\n", data, 0xad);
			break;
		case 7:
			tl16_write_reg(create_tl16_reg_addr(TL16_2_CSC_ADDR, TL16_SCR_REG), 0xae);
			printf_syn("[ch %d]write scr:#%x\n", data, 0xae);
			break;
		case 8:
			tl16_write_reg(create_tl16_reg_addr(TL16_2_CSD_ADDR, TL16_SCR_REG), 0xaf);
			printf_syn("[ch %d]write scr:#%x\n", data, 0xaf);
			break;
		}
		break;
	case 6:
		switch (data) {
		case 1:
			reg_val = tl16_read_reg(create_tl16_reg_addr(TL16_1_CSA_ADDR, TL16_SCR_REG));
			printf_syn("[ch %d]read scr:#%x\n", data, reg_val);
			break;
		case 2:
			reg_val = tl16_read_reg(create_tl16_reg_addr(TL16_1_CSB_ADDR, TL16_SCR_REG));
			printf_syn("[ch %d]read scr:#%x\n", data, reg_val);
			break;
		case 3:
			reg_val = tl16_read_reg(create_tl16_reg_addr(TL16_1_CSC_ADDR, TL16_SCR_REG));
			printf_syn("[ch %d]read scr:#%x\n", data, reg_val);
			break;
		case 4:
			reg_val = tl16_read_reg(create_tl16_reg_addr(TL16_1_CSD_ADDR, TL16_SCR_REG));
			printf_syn("[ch %d]read scr:#%x\n", data, reg_val);
			break;
		case 5:
			reg_val = tl16_read_reg(create_tl16_reg_addr(TL16_2_CSA_ADDR, TL16_SCR_REG));
			printf_syn("[ch %d]read scr:#%x\n", data, reg_val);
			break;
		case 6:
			reg_val = tl16_read_reg(create_tl16_reg_addr(TL16_2_CSB_ADDR, TL16_SCR_REG));
			printf_syn("[ch %d]read scr:#%x\n", data, reg_val);
			break;
		case 7:
			reg_val = tl16_read_reg(create_tl16_reg_addr(TL16_2_CSC_ADDR, TL16_SCR_REG));
			printf_syn("[ch %d]read scr:#%x\n", data, reg_val);
			break;
		case 8:
			reg_val = tl16_read_reg(create_tl16_reg_addr(TL16_2_CSD_ADDR, TL16_SCR_REG));
			printf_syn("[ch %d]read scr:#%x\n", data, reg_val);
			break;
		}
		break;

	case 7:
		if (data) {
			tl16c554_1_reset_act();
			tl16c554_2_reset_act();
			printf_syn("chip 1,2 reset act\n");
		} else {
			tl16c554_1_reset_deact();
			tl16c554_2_reset_deact();
			printf_syn("chip 1,2 reset deact\n");
		}
		break;
	case 8:
		switch (data) {
		case 1:
			tl16c554_1_csa_act();
			break;
		case 2:
			tl16c554_1_csb_act();
			break;
		case 3:
			tl16c554_1_csc_act();
			break;
		case 4:
			tl16c554_1_csd_act();
			break;
		case 5:
			tl16c554_2_csa_act();
			break;
		case 6:
			tl16c554_2_csb_act();
			break;
		case 7:
			tl16c554_2_csc_act();
			break;
		case 8:
			tl16c554_2_csd_act();
			break;
		}
		break;

	case 9:
		switch (data) {
		case 1:
			tl16c554_1_csa_deact();
			break;
		case 2:
			tl16c554_1_csb_deact();
			break;
		case 3:
			tl16c554_1_csc_deact();
			break;
		case 4:
			tl16c554_1_csd_deact();
			break;
		case 5:
			tl16c554_2_csa_deact();
			break;
		case 6:
			tl16c554_2_csb_deact();
			break;
		case 7:
			tl16c554_2_csc_deact();
			break;
		case 8:
			tl16c554_2_csd_deact();
			break;
		}
		break;

	case 10:
		switch (data) {
		case 1:
			tl16c554_1_cha_485tx_enable();
			break;
		case 2:
			tl16c554_1_chb_485tx_enable();
			break;
		case 3:
			tl16c554_1_chc_485tx_enable();
			break;
		case 4:
			tl16c554_1_chd_485tx_enable();
			break;
		case 5:
			tl16c554_2_cha_485tx_enable();
			break;
		case 6:
			tl16c554_2_chb_485tx_enable();
			break;
		case 7:
			tl16c554_2_chc_485tx_enable();
			break;
		case 8:
			tl16c554_2_chd_485tx_enable();
			break;
		case 9:
			tl16c554_1_cha_485tx_enable();
			tl16c554_1_chb_485tx_enable();
			tl16c554_1_chc_485tx_enable();
			tl16c554_1_chd_485tx_enable();
			tl16c554_2_cha_485tx_enable();
			tl16c554_2_chb_485tx_enable();
			tl16c554_2_chc_485tx_enable();
			tl16c554_2_chd_485tx_enable();
			break;
		}
		break;

	case 11:
		switch (data) {
		case 1:
			tl16c554_1_cha_485tx_disable();
			break;
		case 2:
			tl16c554_1_chb_485tx_disable();
			break;
		case 3:
			tl16c554_1_chc_485tx_disable();
			break;
		case 4:
			tl16c554_1_chd_485tx_disable();
			break;
		case 5:
			tl16c554_2_cha_485tx_disable();
			break;
		case 6:
			tl16c554_2_chb_485tx_disable();
			break;
		case 7:
			tl16c554_2_chc_485tx_disable();
			break;
		case 8:
			tl16c554_2_chd_485tx_disable();
			break;
		case 9:
			tl16c554_1_cha_485tx_disable();
			tl16c554_1_chb_485tx_disable();
			tl16c554_1_chc_485tx_disable();
			tl16c554_1_chd_485tx_disable();
			tl16c554_2_cha_485tx_disable();
			tl16c554_2_chb_485tx_disable();
			tl16c554_2_chc_485tx_disable();
			tl16c554_2_chd_485tx_disable();
			break;
		}
		break;

	case 12:
		if (data) {
			tl16c554_x_485rx_enable();
		} else {
			tl16c554_x_485rx_disable();
		}
		break;

	case 13:
		if (data) {
			tl16_set_fifo_mode(TL16_1_CSA_ADDR, 1);
			printf_syn("set ch-1 to fifo mode\n");
		} else {
			tl16_set_fifo_mode(TL16_1_CSA_ADDR, 0);
			printf_syn("cancel ch-1 from fifo mode\n");
		}
		break;

	case 14:
		if (data) {
			tl16_set_ier(TL16_1_CSA_ADDR, TL16C554_RX_LINE_STATUS, 1);
			tl16_set_ier(TL16_1_CSA_ADDR, TL16C554_TX_HOLD_REG_EMPTY, 1);
			tl16_set_ier(TL16_1_CSA_ADDR, TL16C554_RX_DATA_AVAILABLE, 1);
			printf_syn("enable ch-1 rx-line-status, tx-hold, rx-data\n");
		} else {
			tl16_set_ier(TL16_1_CSA_ADDR, TL16C554_RX_LINE_STATUS, 0);
			tl16_set_ier(TL16_1_CSA_ADDR, TL16C554_TX_HOLD_REG_EMPTY, 0);
			tl16_set_ier(TL16_1_CSA_ADDR, TL16C554_RX_DATA_AVAILABLE, 0);
			printf_syn("disable ch-1 rx-line-status, tx-hold, rx-data\n");
		}
		break;

	case 15:
		if (data) {
			tl16_set_loop_mode(TL16_1_CSA_ADDR, 1);
			printf_syn("set ch-1 to loop mode\n");
		} else {
			tl16_set_loop_mode(TL16_1_CSA_ADDR, 0);
			printf_syn("cancel ch-1 loop mode\n");
		}
		break;

	case 16:
		print_tl16_state();
		break;

	case 17:
		tl16_set_buadrate(TL16_1_CSA_ADDR, 9600);
		tl16_set_uart_other_param(TL16_1_CSA_ADDR, 8, 1, 0);
		printf_syn("set ch-1 baud to 9600bps[8,1,none]\n");
		break;

	case 18:
		tl16_clr_divisor(TL16_1_CSA_ADDR);
		printf_syn("clr ch-1 divisor\n");
		break;

	case 19:
		tl16_init_for_test_isr(data);
		break;

	case 20:
		regaddr = create_tl16_reg_addr(TL16_1_CSA_ADDR, TL16_MCR_REG);
		reg_val = tl16_read_reg(regaddr);
		reg_val |= 1<<3;
		tl16_write_reg(regaddr, reg_val);
		break;

	case 21:
		rt_event_send(&isr_event_set, EVENT_BIT_NEED_RUN_TL16_1_IRQ);
		break;

	case 22:
		if (data>=TL16_1_CSA_ADDR && data<=TL16_2_CSD_ADDR)
			tl16_disable_chx(data);
		else
			printf_syn("ch-no(%d) error\n", data);
		break;
	case 23:
		tl16_dev_rw_data(data, 1, 0);
		break;
	case 24:
		tl16_dev_rw_data(data, 0, 0x80+data);
		break;

	case 25:
		print_tl16_uart_err_info();
		break;

	case 26:
	{
		extern rt_device_t dev_485_tl16_1;
		extern rt_device_t dev_485_tl16_2;
		extern rt_device_t dev_485_tl16_3;
		extern rt_device_t dev_485_tl16_4;
		extern rt_device_t dev_485_tl16_5;
		extern rt_device_t dev_485_tl16_6;
		extern rt_device_t dev_485_tl16_7;
		extern rt_device_t dev_485_tl16_8;
		char buf[4];
		int nr;

		switch (data) {
		case 1:
			if (NULL != dev_485_tl16_1) {
				nr = dev_485_tl16_1->read(dev_485_tl16_1, 0, buf, sizeof(buf));
				if (0 != nr) {
					printf_syn("read %d data from dev_485_tl16_1:0x%x, 0x%x, 0x%x, 0x%x\n",
							nr, buf[0], buf[1], buf[2], buf[3]);
				} else {
					printf_syn("dev_485_tl16_1 have not recv data\n");
				}
			}
			break;
		case 2:
			if (NULL != dev_485_tl16_2) {
				nr = dev_485_tl16_2->read(dev_485_tl16_2, 0, buf, sizeof(buf));
				if (0 != nr) {
					printf_syn("read %d data from dev_485_tl16_2:0x%x, 0x%x, 0x%x, 0x%x\n",
							nr, buf[0], buf[1], buf[2], buf[3]);
				} else {
					printf_syn("dev_485_tl16_2 have not recv data\n");
				}
			}
			break;
		case 3:
			if (NULL != dev_485_tl16_3) {
				nr = dev_485_tl16_3->read(dev_485_tl16_3, 0, buf, sizeof(buf));
				if (0 != nr) {
					printf_syn("read %d data from dev_485_tl16_3:0x%x, 0x%x, 0x%x, 0x%x\n",
							nr, buf[0], buf[1], buf[2], buf[3]);
				} else {
					printf_syn("dev_485_tl16_3 have not recv data\n");
				}
			}
			break;
		case 4:
			if (NULL != dev_485_tl16_4) {
				nr = dev_485_tl16_4->read(dev_485_tl16_4, 0, buf, sizeof(buf));
				if (0 != nr) {
					printf_syn("read %d data from dev_485_tl16_4:0x%x, 0x%x, 0x%x, 0x%x\n",
							nr, buf[0], buf[1], buf[2], buf[3]);
				} else {
					printf_syn("dev_485_tl16_4 have not recv data\n");
				}
			}
			break;
		case 5:
			if (NULL != dev_485_tl16_5) {
				nr = dev_485_tl16_5->read(dev_485_tl16_5, 0, buf, sizeof(buf));
				if (0 != nr) {
					printf_syn("read %d data from dev_485_tl16_5:0x%x, 0x%x, 0x%x, 0x%x\n",
							nr, buf[0], buf[1], buf[2], buf[3]);
				} else {
					printf_syn("dev_485_tl16_5 have not recv data\n");
				}
			}
			break;
		case 6:
			if (NULL != dev_485_tl16_6) {
				nr = dev_485_tl16_6->read(dev_485_tl16_6, 0, buf, sizeof(buf));
				if (0 != nr) {
					printf_syn("read %d data from dev_485_tl16_6:0x%x, 0x%x, 0x%x, 0x%x\n",
							nr, buf[0], buf[1], buf[2], buf[3]);
				} else {
					printf_syn("dev_485_tl16_6 have not recv data\n");
				}
			}
			break;
		case 7:
			if (NULL != dev_485_tl16_7) {
				nr = dev_485_tl16_7->read(dev_485_tl16_7, 0, buf, sizeof(buf));
				if (0 != nr) {
					printf_syn("read %d data from dev_485_tl16_7:0x%x, 0x%x, 0x%x, 0x%x\n",
							nr, buf[0], buf[1], buf[2], buf[3]);
				} else {
					printf_syn("dev_485_tl16_7 have not recv data\n");
				}
			}
			break;
		case 8:
			if (NULL != dev_485_tl16_8) {
				nr = dev_485_tl16_8->read(dev_485_tl16_8, 0, buf, sizeof(buf));
				if (0 != nr) {
					printf_syn("read %d data from dev_485_tl16_8:0x%x, 0x%x, 0x%x, 0x%x\n",
							nr, buf[0], buf[1], buf[2], buf[3]);
				} else {
					printf_syn("dev_485_tl16_8 have not recv data\n");
				}
			}
			break;
		}



	}
		break;

	case 27:
	{
		void print_tl16_rx_ind_cnt(void);

		print_tl16_rx_ind_cnt();
	}
		break;
	default:
		break;
	}
}

static int tl16_dev_rw_data(int ch_no, int is_read, int data)
{
	rt_device_t dev;
	char name[12] = "tl16-u1";
	char buf[4];
	int index;

	if (!(ch_no>=TL16_1_CSA_ADDR && ch_no<=TL16_2_CSD_ADDR)) {
		printf_syn("ch-no(%d) error\n", ch_no);
		return FAIL;
	}

	index = rt_strlen(name) - 1;
	name[index] = '0' + ch_no;

	dev = rt_device_find(name);
	if (NULL != dev) {
		if (is_read) {
			if(1 == dev->read(dev, 0, buf, 1))
				printf_syn("read data(%d) from %s\n", buf[0], name);
			else
				printf_syn("read data from %s fail\n", name);
		} else {
			buf[0] = data;
			if(1 == dev->write(dev, 0, buf, 1))
				printf_syn("write data(%d) to %s\n", buf[0], name);
			else
				printf_syn("write data(%d) to %s fail\n", buf[0], name);
		}
	} else {
		rt_kprintf("find %s device fail\n", name);
		return FAIL;
	}

	return SUCC;
}

FINSH_FUNCTION_EXPORT(tl16_test, tl16_test cmd-data);
#endif
