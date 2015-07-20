#include "..\Source\LIB_Config.h"
#include "..\Source\PF_Config.H"
#include "sys_comm.h"

#include ".\fwlib\stm32f10x.h"

extern int recv_devsn_frompc(int isstart);

/*
 * 按字节来发送一个字, 先发送高位字节
 * cnt是发送的字节个数, 取值范围是[1, 4], 小于4时截取地位字节发送
 */
void usartx_use_buf_tx(USART_TypeDef *usartx, unsigned long data, int cnt,
					   pf_putdata2buf put_data, pf_getbufdata get_data)
{
	int ret, int_state;
	char *p;

	int_state = get_usartx_send_int_state(usartx);
	disable_usartx_send_int(usartx);
	if (cnt > 4) {
		cnt = 4;
	} else if (cnt < 1) {
		cnt = 1;
	}

	p = (char *)&data;
	switch (cnt) {
		/*
		case 1:
		    break;
		*/
	case 2:
		++p;
		break;
	case 3:
		p += 2;
		break;

	case 4:
		p += 3;
		break;

	default:
		break;
	}

	do {
		ret = put_data(*p); /* return 0 -- failure */
		if (ret) {
			--cnt;
			--p;
		} else {
			break; /* 这里只做简单处理, 缓冲区满时, 丢弃未发送到数据, 使用中没有集中发送大于128B的数据不会发生这种情况 */
		}
	} while (cnt);

	if (1 != int_state && is_usartx_txd_empty(usartx))
		put_usartx_data(usartx, get_data());

	enable_usartx_send_int(usartx);

	return;
}



/*
 * 按字节来发送一个字, 先发送低位字节
 * cnt是发送的字节个数, 取值范围是[1, 4], 小于4时截取地位字节发送
 */
void usartx_use_buf_tx_lf(USART_TypeDef *usartx, unsigned long data, int cnt,
						  pf_putdata2buf put_data, pf_getbufdata get_data)
{
	int ret, int_state;
	char *p;

	int_state = get_usartx_send_int_state(usartx);
	disable_usartx_send_int(usartx);
	if (cnt > 4) {
		cnt = 4;
	} else if (cnt < 1) {
		cnt = 1;
	}

	p = (char *)&data;
	do {
		ret = put_data(*p); /* return 0 -- failure */
		if (ret) {
			--cnt;
			++p;
		} else {
			break; /* 这里只做简单处理, 缓冲区满时, 丢弃未发送到数据, 使用中没有集中发送大于128B的数据不会发生这种情况 */
		}
	} while (cnt);

	if (1 != int_state && is_usartx_txd_empty(usartx))
		put_usartx_data(usartx, get_data());

	enable_usartx_send_int(usartx);

	return;
}

enum uart_recv_state {
	UART_RECV_NULL     = 0,
	UART_RECV_1_BYTE   = 1,
	UART_RECV_2_BYTE   = 2,
	UART_RECV_3_BYTE   = 3,
	UART_RECV_WAITING  = 4,	
};

void usr_cmd_analysis(void)
{
	static int cmd, param;
	static int revc_state = UART_RECV_NULL;

	disable_usartx_recv_int(UART4PC);
	switch (revc_state) {
	case UART_RECV_NULL:
		if(0 != get_uart4pc_buf_len_fn()) {
			cmd = get_uart4pc_buf_data_fn(); /* mark */
			revc_state = UART_RECV_1_BYTE;
		} else {
			break;
		}

		if(0 != get_uart4pc_buf_len_fn()) {
			param = get_uart4pc_buf_data_fn(); /* mark */
			revc_state = UART_RECV_2_BYTE;
		} else {
			break;
		}

		if(0 != get_uart4pc_buf_len_fn()) {
			param = param<<8 | get_uart4pc_buf_data_fn();
			revc_state = UART_RECV_3_BYTE;
		} else {
			break;
		}
		break;

	case UART_RECV_1_BYTE:
		if(0 != get_uart4pc_buf_len_fn()) {
			param = get_uart4pc_buf_data_fn();
			revc_state = UART_RECV_2_BYTE;
		} else {
			break;
		}

		if(0 != get_uart4pc_buf_len_fn()) {
			param = param<<8 | get_uart4pc_buf_data_fn();
			revc_state = UART_RECV_3_BYTE;
		} else {
			break;
		}
		break;

	case UART_RECV_2_BYTE:
		if(0 != get_uart4pc_buf_len_fn()) {
			param = param<<8 | get_uart4pc_buf_data_fn();
			revc_state = UART_RECV_3_BYTE;
		} else {
			break;
		}
		break;
		/*
		    case UART_RECV_3_BYTE:
		        break;
		*/
	default:
		break;
	}


	if (UART_RECV_3_BYTE == revc_state) {
		if (SPC_SET_DEV_SN==cmd && param==0x534E) {
			revc_state = UART_RECV_WAITING;
			recv_devsn_frompc(1);
		} else {
			revc_state = UART_RECV_NULL;
			usr_cmd_proc(cmd, param);
		}
	} else if (UART_RECV_WAITING == revc_state) {
		if (0 != recv_devsn_frompc(0))
			revc_state = UART_RECV_NULL;
	} else if (revc_state >= UART_RECV_1_BYTE) {
		if (!is_cmd_valid(cmd)) {
			revc_state = UART_RECV_NULL;
			flush_uart4pc_buf_fn();
		}
	}

	enable_usartx_recv_int(UART4PC);

	return;
}


/**
 * This function will copy string no more than n bytes.
 *
 * @param dst the string to copy
 * @param src the string to be copied
 * @param n the maximum copied length
 *
 * @return the result
 */
char *strncpy(char *dst, const char *src, int n)
{
	if (n != 0) {
		char *d = dst;
		const char *s = src;

		do {
			if ((*d++ = *s++) == 0) {
				/* NUL pad the remaining n-1 bytes */
				while (--n != 0)
					*d++ = 0;
				break;
			}
		} while (--n != 0);
	}

	return (dst);
}


int strncmp(const char *cs, const char *ct, int count)
{
	register signed char __res = 0;

	while (count) {
		if ((__res = *cs - *ct++) != 0 || !*cs++)
			break;
		count --;
	}

	return __res;
}

