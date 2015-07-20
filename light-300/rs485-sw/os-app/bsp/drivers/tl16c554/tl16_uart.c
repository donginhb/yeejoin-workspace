/*
 ******************************************************************************
 * tl16_uart.c
 *
 *  Created on: 2015-1-4
 *      Author: David, zhaoshaowei@yeejoin.com
 *
 * COPYRIGHT (C) 2015, YEEJOIN (Beijing) Technology Development Co., Ltd.
 ******************************************************************************
 */



#include <syscfgdata.h>
#include <sys_cfg_api.h>
#include <board.h>

#include <tl16_uart.h>
#include <tl16_serial.h>



static rt_uint8_t  tl16_1_uart1_rx_buffer[TL16_UART_RX_BUFFER_SIZE];
static rt_uint8_t  tl16_1_uart2_rx_buffer[TL16_UART_RX_BUFFER_SIZE];
static rt_uint8_t  tl16_1_uart3_rx_buffer[TL16_UART_RX_BUFFER_SIZE];
static rt_uint8_t  tl16_1_uart4_rx_buffer[TL16_UART_RX_BUFFER_SIZE];

static rt_uint8_t  tl16_2_uart1_rx_buffer[TL16_UART_RX_BUFFER_SIZE];
static rt_uint8_t  tl16_2_uart2_rx_buffer[TL16_UART_RX_BUFFER_SIZE];
static rt_uint8_t  tl16_2_uart3_rx_buffer[TL16_UART_RX_BUFFER_SIZE];
static rt_uint8_t  tl16_2_uart4_rx_buffer[TL16_UART_RX_BUFFER_SIZE];

static struct tl16_uart_st tl16_uart1 = {TL16_1_CSA_ADDR};
static struct tl16_uart_st tl16_uart2 = {TL16_1_CSB_ADDR};
static struct tl16_uart_st tl16_uart3 = {TL16_1_CSC_ADDR};
static struct tl16_uart_st tl16_uart4 = {TL16_1_CSD_ADDR};

static struct tl16_uart_st tl16_uart5 = {TL16_2_CSA_ADDR};
static struct tl16_uart_st tl16_uart6 = {TL16_2_CSB_ADDR};
static struct tl16_uart_st tl16_uart7 = {TL16_2_CSC_ADDR};
static struct tl16_uart_st tl16_uart8 = {TL16_2_CSD_ADDR};

static struct tl16_serial_int_rx tl16_1_uart1_int_rx = {
		tl16_1_uart1_rx_buffer, sizeof(tl16_1_uart1_rx_buffer), 0, 0};
static struct tl16_serial_int_rx tl16_1_uart2_int_rx = {
		tl16_1_uart2_rx_buffer, sizeof(tl16_1_uart2_rx_buffer), 0, 0};
static struct tl16_serial_int_rx tl16_1_uart3_int_rx = {
		tl16_1_uart3_rx_buffer, sizeof(tl16_1_uart3_rx_buffer), 0, 0};
static struct tl16_serial_int_rx tl16_1_uart4_int_rx = {
		tl16_1_uart4_rx_buffer, sizeof(tl16_1_uart4_rx_buffer), 0, 0};

static struct tl16_serial_int_rx tl16_2_uart1_int_rx = {
		tl16_2_uart1_rx_buffer, sizeof(tl16_2_uart1_rx_buffer), 0, 0};
static struct tl16_serial_int_rx tl16_2_uart2_int_rx = {
		tl16_2_uart2_rx_buffer, sizeof(tl16_2_uart2_rx_buffer), 0, 0};
static struct tl16_serial_int_rx tl16_2_uart3_int_rx = {
		tl16_2_uart3_rx_buffer, sizeof(tl16_2_uart3_rx_buffer), 0, 0};
static struct tl16_serial_int_rx tl16_2_uart4_int_rx = {
		tl16_2_uart4_rx_buffer, sizeof(tl16_2_uart4_rx_buffer), 0, 0};

static struct tl16_serial_device uart1 = {&tl16_uart1, &tl16_1_uart1_int_rx};
static struct tl16_serial_device uart2 = {&tl16_uart2, &tl16_1_uart2_int_rx};
static struct tl16_serial_device uart3 = {&tl16_uart3, &tl16_1_uart3_int_rx};
static struct tl16_serial_device uart4 = {&tl16_uart4, &tl16_1_uart4_int_rx};
static struct tl16_serial_device uart5 = {&tl16_uart5, &tl16_2_uart1_int_rx};
static struct tl16_serial_device uart6 = {&tl16_uart6, &tl16_2_uart2_int_rx};
static struct tl16_serial_device uart7 = {&tl16_uart7, &tl16_2_uart3_int_rx};
static struct tl16_serial_device uart8 = {&tl16_uart8, &tl16_2_uart4_int_rx};


struct rt_device tl16_uart1_device;
struct rt_device tl16_uart2_device;
struct rt_device tl16_uart3_device;
struct rt_device tl16_uart4_device;
struct rt_device tl16_uart5_device;
struct rt_device tl16_uart6_device;
struct rt_device tl16_uart7_device;
struct rt_device tl16_uart8_device;

struct rt_mutex tl16_x_lock;


/*
 * Init all related hardware in here
 * rt_hw_serial_init() will register all supported USART device
 */
void rt_hw_tl16_usart_init(void)
{
	int i, index, ch_cnt;
	struct uart_param uartcfg;
	uint32_t regaddr;
	uint32_t reg_val;
	char name[12] = "tl16-u1";


	struct tl16_serial_device *uartx[] = {
		&uart1, &uart2, &uart3, &uart4,
		&uart5, &uart6, &uart7, &uart8,
	};
	struct rt_device	*tl16_uartx_dev[] = {
		&tl16_uart1_device, &tl16_uart2_device, &tl16_uart3_device, &tl16_uart4_device,
		&tl16_uart5_device, &tl16_uart6_device, &tl16_uart7_device, &tl16_uart8_device,
	};

	ch_cnt = 0;

	if (TRUE != tl16c554_chip_is_avaible(0)) {
		printf_syn("%s(), tl16c554_chip #0 is not avaible\n", __func__);
		return;
	}
	ch_cnt += 4;

	if (TRUE == tl16c554_chip_is_avaible(1)) {
		ch_cnt += 4;
	} else {
		printf_syn("%s(), tl16c554_chip #1 is not avaible\n", __func__);
	}

	index = rt_strlen(name) - 1;
	/* uart init */
	for (i=0; i<ch_cnt; ++i) {
		if (SUCC==get_tl16_uart_param(i+1, &uartcfg)) {// && 0!=uartcfg.baudrate) {

			tl16_set_buadrate(i+1, uartcfg.baudrate);
			if (0 != uartcfg.baudrate) {
				tl16_set_uart_other_param(i+1, uartcfg.databits,
						uartcfg.stopbits==1?1:2, uartcfg.paritybit);
				tl16_set_fifo_mode(i+1, 1);
				tl16_set_ier(i+1, TL16C554_RX_LINE_STATUS, 1);
	//			tl16_set_ier(i+1, TL16C554_TX_HOLD_REG_EMPTY, 1);
				tl16_set_ier(i+1, TL16C554_RX_DATA_AVAILABLE, 1);

				/* Bit 3: When MCR3 is set, the external serial channel interrupt is enabled. */
				regaddr = create_tl16_reg_addr(i+1, TL16_MCR_REG);
				reg_val = tl16_read_reg(regaddr);
				reg_val |= 1<<3;
				tl16_write_reg(regaddr, reg_val);
			}

			name[index] = '1' + i;

			/* register uart */
			rt_hw_tl16_serial_register(tl16_uartx_dev[i], name,
					RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX, uartx[i]);
			/* enable interrupt */
//			USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
		}

	}

	return;
}

rt_device_t get_tl16dev_from_x_csx(enum tl16_x_csx_addr_e x_csx)
{
	rt_device_t ret;

	switch (x_csx) {
	case TL16_1_CSA_ADDR:
		ret = &tl16_uart1_device;
		break;
	case TL16_1_CSB_ADDR:
		ret = &tl16_uart2_device;
		break;
	case TL16_1_CSC_ADDR:
		ret = &tl16_uart3_device;
		break;
	case TL16_1_CSD_ADDR:
		ret = &tl16_uart4_device;
		break;

	case TL16_2_CSA_ADDR:
		ret = &tl16_uart5_device;
		break;
	case TL16_2_CSB_ADDR:
		ret = &tl16_uart6_device;
		break;
	case TL16_2_CSC_ADDR:
		ret = &tl16_uart7_device;
		break;
	case TL16_2_CSD_ADDR:
		ret = &tl16_uart8_device;
		break;

	default:
		ret = NULL;
		rt_kprintf("%s():tl16_x_csx invalid\n", __func__);
		break;
	}

	return ret;
}

struct tl16_serial_device *get_tl16uart_from_x_csx(enum tl16_x_csx_addr_e x_csx)
{
	struct tl16_serial_device *ret;

	switch (x_csx) {
	case TL16_1_CSA_ADDR:
		ret = &uart1;
		break;
	case TL16_1_CSB_ADDR:
		ret = &uart2;
		break;
	case TL16_1_CSC_ADDR:
		ret = &uart3;
		break;
	case TL16_1_CSD_ADDR:
		ret = &uart4;
		break;

	case TL16_2_CSA_ADDR:
		ret = &uart5;
		break;
	case TL16_2_CSB_ADDR:
		ret = &uart6;
		break;
	case TL16_2_CSC_ADDR:
		ret = &uart7;
		break;
	case TL16_2_CSD_ADDR:
		ret = &uart8;
		break;

	default:
		ret = NULL;
		rt_kprintf("%s():tl16_x_csx invalid\n", __func__);
		break;
	}

	return ret;
}

int tl16_485_tx_ctrl(rt_device_t device, int is_enable)
{
	struct tl16_serial_device *uart;
	enum tl16_x_csx_addr_e x_csx;
	int ret = SUCC;

	if (NULL == device) {
		printf_syn("%s(), param invalid\n");
		return FAIL;
	}

	uart = (struct tl16_serial_device*) device->user_data;

	x_csx = NULL==uart->uart_device ? 0 : uart->uart_device->x_csx;

	if (is_enable) {
		tl16c554_1_cha_485tx_disable();
		tl16c554_1_chb_485tx_disable();
		tl16c554_1_chc_485tx_disable();
		tl16c554_1_chd_485tx_disable();
		tl16c554_2_cha_485tx_disable();
		tl16c554_2_chb_485tx_disable();
		tl16c554_2_chc_485tx_disable();
		tl16c554_2_chd_485tx_disable();
	}

	if (is_enable) {
		switch (x_csx) {
		case TL16_1_CSA_ADDR:
			tl16c554_1_cha_485tx_enable();
			break;
		case TL16_1_CSB_ADDR:
			tl16c554_1_chb_485tx_enable();
			break;
		case TL16_1_CSC_ADDR:
			tl16c554_1_chc_485tx_enable();
			break;
		case TL16_1_CSD_ADDR:
			tl16c554_1_chd_485tx_enable();
			break;

		case TL16_2_CSA_ADDR:
			tl16c554_2_cha_485tx_enable();
			break;
		case TL16_2_CSB_ADDR:
			tl16c554_2_chb_485tx_enable();
			break;
		case TL16_2_CSC_ADDR:
			tl16c554_2_chc_485tx_enable();
			break;
		case TL16_2_CSD_ADDR:
			tl16c554_2_chd_485tx_enable();
			break;

		default:
			ret = FAIL;
			rt_kprintf("%s():tl16_x_csx(%d) invalid\n", __func__, x_csx);
			break;
		}
	} else {
		switch (x_csx) {
		case TL16_1_CSA_ADDR:
			tl16c554_1_cha_485tx_disable();
			break;
		case TL16_1_CSB_ADDR:
			tl16c554_1_chb_485tx_disable();
			break;
		case TL16_1_CSC_ADDR:
			tl16c554_1_chc_485tx_disable();
			break;
		case TL16_1_CSD_ADDR:
			tl16c554_1_chd_485tx_disable();
			break;

		case TL16_2_CSA_ADDR:
			tl16c554_2_cha_485tx_disable();
			break;
		case TL16_2_CSB_ADDR:
			tl16c554_2_chb_485tx_disable();
			break;
		case TL16_2_CSC_ADDR:
			tl16c554_2_chc_485tx_disable();
			break;
		case TL16_2_CSD_ADDR:
			tl16c554_2_chd_485tx_disable();
			break;

		default:
			ret = FAIL;
			rt_kprintf("%s():tl16_x_csx(%d) invalid\n", __func__, x_csx);
			break;
		}
	}

	return ret;
}


int tl16_485_rx_ctrl(rt_device_t device, int is_enable)
{
	struct tl16_serial_device *uart;
	enum tl16_x_csx_addr_e x_csx;
	int ret = SUCC;

	if (NULL == device) {
		printf_syn("%s(), param invalid\n");
		return FAIL;
	}

	uart = (struct tl16_serial_device*) device->user_data;

	x_csx = NULL==uart->uart_device ? 0 : uart->uart_device->x_csx;

	if (x_csx>=TL16_1_CSA_ADDR && x_csx<=TL16_2_CSD_ADDR) {
		if (is_enable) {
			tl16c554_x_485rx_enable();
		} else {
			tl16c554_x_485rx_disable();
		}
	} else {
		ret = FAIL;
		rt_kprintf("%s():tl16_x_csx(%d) invalid\n", __func__, x_csx);
	}

	return ret;
}

int is_tl16_485_tx_over(rt_device_t device)
{
	struct tl16_serial_device *uart;
	enum tl16_x_csx_addr_e x_csx;
	int ret, temp;

	if (NULL == device) {
		printf_syn("%s(), param invalid\n");
		return TRUE;
	}

	uart = (struct tl16_serial_device*) device->user_data;
	x_csx = NULL==uart->uart_device ? 0 : uart->uart_device->x_csx;

	if (x_csx>=TL16_1_CSA_ADDR && x_csx<=TL16_2_CSD_ADDR) {
		temp = tl16_read_lsr(x_csx);
		/* Bit 6: LSR6 is the transmitter register empty (TEMT) bit. TEMT is set
		 * when the THR and the TSR are both empty.
		 * */
		if (temp & 1<<6) {
			ret = TRUE;
		} else {
			ret = FALSE;
		}
	} else {
		ret = TRUE;
		printf_syn("%s():tl16_x_csx(%d) invalid\n", __func__, x_csx);
	}

	return ret;
}
void print_tl16_uart_err_info(void)
{
	struct tl16_serial_device *serial_dev;
	enum tl16_x_csx_addr_e x_csx;

	for (x_csx=TL16_1_CSA_ADDR; x_csx<=TL16_2_CSD_ADDR; ++x_csx) {
		serial_dev = get_tl16uart_from_x_csx(x_csx);
		printf_syn("break int bit cnt:%d, fifo err cnt:%d, framing err cnt:%d, "
				"overrun err cnt:%d, parity err cnt:%d\n",
				serial_dev->uart_device->break_int_bit_cnt, serial_dev->uart_device->fifo_err_cnt,
				serial_dev->uart_device->framing_err_cnt, serial_dev->uart_device->overrun_err_cnt,
				serial_dev->uart_device->parity_err_cnt);
	}
}
