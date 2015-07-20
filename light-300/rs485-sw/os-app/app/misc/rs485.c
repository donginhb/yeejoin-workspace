/*
 ******************************************************************************
 * rs485.c
 *
 * 2013-10-15,  creat by David, zhaoshaowei@yeejoin.com
 ******************************************************************************
 */

#include <rtdef.h>
#include <board.h>
#include <rs485.h>
#include <rs485_common.h>
#include <sys_cfg_api.h>
#include <misc_lib.h>
#include <tl16_uart.h>
#if WIRELESS_MASTER_NODE
#include <syscfgdata.h>
#endif

#if 1
#define WAIT_RX_MSG_TIMEOUT (get_ticks_of_ms(1500))
#else
#define WAIT_RX_MSG_TIMEOUT RT_WAITING_FOREVER
#endif


//struct rt_semaphore uart485_1_rx_byte_sem;
//struct rt_semaphore uart485_2_rx_byte_sem;
//struct rt_semaphore uart485_3_rx_byte_sem;

struct rt_event em_protocol_data_event_set;


static int set_485_tx_rx_state(USART_TypeDef *dev_485, int is_tx_state);
static rt_err_t uart485_1_rx_ind(rt_device_t dev, rt_size_t size);
static rt_err_t uart485_2_rx_ind(rt_device_t dev, rt_size_t size);
static rt_err_t uart485_3_rx_ind(rt_device_t dev, rt_size_t size);

static int set_tl16_485_tx_rx_state(rt_device_t dev_485, int is_tx_state);
static rt_err_t uart485_tl16_1_rx_ind(rt_device_t dev, rt_size_t size);
static rt_err_t uart485_tl16_2_rx_ind(rt_device_t dev, rt_size_t size);
static rt_err_t uart485_tl16_3_rx_ind(rt_device_t dev, rt_size_t size);
static rt_err_t uart485_tl16_4_rx_ind(rt_device_t dev, rt_size_t size);
static rt_err_t uart485_tl16_5_rx_ind(rt_device_t dev, rt_size_t size);
static rt_err_t uart485_tl16_6_rx_ind(rt_device_t dev, rt_size_t size);
static rt_err_t uart485_tl16_7_rx_ind(rt_device_t dev, rt_size_t size);
static rt_err_t uart485_tl16_8_rx_ind(rt_device_t dev, rt_size_t size);

static rt_device_t dev_485_1;
static rt_device_t dev_485_2;
static rt_device_t dev_485_3;

rt_device_t dev_485_tl16_1;
rt_device_t dev_485_tl16_2;
rt_device_t dev_485_tl16_3;
rt_device_t dev_485_tl16_4;
rt_device_t dev_485_tl16_5;
rt_device_t dev_485_tl16_6;
rt_device_t dev_485_tl16_7;
rt_device_t dev_485_tl16_8;

rt_uint8_t *rs485_recv_buf;


#define rs485_log(x) 		printf_syn x
#define rs485_info(x) 		printf_syn x
#define PRINT_DETAIL_MSG	1

void init_sys_485(void)
{

//	rt_sem_init(&uart485_1_rx_byte_sem, "u485-1", 0, RT_IPC_FLAG_PRIO);
//	rt_sem_init(&uart485_2_rx_byte_sem, "u485-2", 0, RT_IPC_FLAG_PRIO);
//	rt_sem_init(&uart485_3_rx_byte_sem, "u485-3", 0, RT_IPC_FLAG_PRIO);

	rt_event_init(&em_protocol_data_event_set, "tl16_eve", RT_IPC_FLAG_PRIO);

	rs485_recv_buf= (rt_uint8_t *)rt_malloc(FRAME_485_LEN);
	if(rs485_recv_buf == RT_NULL) {
		rs485_info(("func:%s(), line:%d malloc fail\n",__FUNCTION__, __LINE__));
		return;		
	}
	rt_memset(rs485_recv_buf, 0, FRAME_485_LEN);

	dev_485_1 = rt_device_find(UART_485_1_DEV);
	if (dev_485_1 != RT_NULL && rt_device_open(dev_485_1, RT_DEVICE_OFLAG_RDWR) == RT_EOK) {
		rs485_info(("open device:%s succ\n", UART_485_1_DEV));
		rt_device_set_rx_indicate(dev_485_1, uart485_1_rx_ind);
	} else {
		rs485_info(("can not find device:%s\n", UART_485_1_DEV));
//		goto err_ret;
	}

	dev_485_2 = rt_device_find(UART_485_2_DEV);
	if (dev_485_2 != RT_NULL && rt_device_open(dev_485_2, RT_DEVICE_OFLAG_RDWR) == RT_EOK) {
		rs485_info(("open device:%s succ\n", UART_485_2_DEV));
		rt_device_set_rx_indicate(dev_485_2, uart485_2_rx_ind);
	} else {
		rs485_info(("can not find device:%s\n", UART_485_2_DEV));
//		goto err_ret;
	}

	dev_485_3 = rt_device_find(UART_485_3_DEV);
	if (dev_485_3 != RT_NULL && rt_device_open(dev_485_3, RT_DEVICE_OFLAG_RDWR) == RT_EOK) {
		rs485_info(("open device:%s succ\n", UART_485_3_DEV));
		rt_device_set_rx_indicate(dev_485_3, uart485_3_rx_ind);
	} else {
		rs485_info(("can not find device:%s\n", UART_485_3_DEV));
//		goto err_ret;
	}

	dev_485_tl16_1 = rt_device_find(UART_485_TL16_1_DEV);
	if (dev_485_tl16_1 != RT_NULL && rt_device_open(dev_485_tl16_1, RT_DEVICE_OFLAG_RDWR) == RT_EOK) {
		rs485_info(("open device:%s succ\n", UART_485_TL16_1_DEV));
		rt_device_set_rx_indicate(dev_485_tl16_1, uart485_tl16_1_rx_ind);
	} else {
		rs485_info(("can not find device:%s\n", UART_485_TL16_1_DEV));
//		goto err_ret;
	}

	dev_485_tl16_2 = rt_device_find(UART_485_TL16_2_DEV);
	if (dev_485_tl16_2 != RT_NULL && rt_device_open(dev_485_tl16_2, RT_DEVICE_OFLAG_RDWR) == RT_EOK) {
		rs485_info(("open device:%s succ\n", UART_485_TL16_2_DEV));
		rt_device_set_rx_indicate(dev_485_tl16_2, uart485_tl16_2_rx_ind);
	} else {
		rs485_info(("can not find device:%s\n", UART_485_TL16_2_DEV));
//		goto err_ret;
	}

	dev_485_tl16_3 = rt_device_find(UART_485_TL16_3_DEV);
	if (dev_485_tl16_3 != RT_NULL && rt_device_open(dev_485_tl16_3, RT_DEVICE_OFLAG_RDWR) == RT_EOK) {
		rs485_info(("open device:%s succ\n", UART_485_TL16_3_DEV));
		rt_device_set_rx_indicate(dev_485_tl16_3, uart485_tl16_3_rx_ind);
	} else {
		rs485_info(("can not find device:%s\n", UART_485_TL16_3_DEV));
//		goto err_ret;
	}

	dev_485_tl16_4 = rt_device_find(UART_485_TL16_4_DEV);
	if (dev_485_tl16_4 != RT_NULL && rt_device_open(dev_485_tl16_4, RT_DEVICE_OFLAG_RDWR) == RT_EOK) {
		rs485_info(("open device:%s succ\n", UART_485_TL16_4_DEV));
		rt_device_set_rx_indicate(dev_485_tl16_4, uart485_tl16_4_rx_ind);
	} else {
		rs485_info(("can not find device:%s\n", UART_485_TL16_4_DEV));
//		goto err_ret;
	}

	dev_485_tl16_5 = rt_device_find(UART_485_TL16_5_DEV);
	if (dev_485_tl16_5 != RT_NULL && rt_device_open(dev_485_tl16_5, RT_DEVICE_OFLAG_RDWR) == RT_EOK) {
		rs485_info(("open device:%s succ\n", UART_485_TL16_5_DEV));
		rt_device_set_rx_indicate(dev_485_tl16_5, uart485_tl16_5_rx_ind);
	} else {
		rs485_info(("can not find device:%s\n", UART_485_TL16_5_DEV));
//		goto err_ret;
	}

	dev_485_tl16_6 = rt_device_find(UART_485_TL16_6_DEV);
	if (dev_485_tl16_6 != RT_NULL && rt_device_open(dev_485_tl16_6, RT_DEVICE_OFLAG_RDWR) == RT_EOK) {
		rs485_info(("open device:%s succ\n", UART_485_TL16_6_DEV));
		rt_device_set_rx_indicate(dev_485_tl16_6, uart485_tl16_6_rx_ind);
	} else {
		rs485_info(("can not find device:%s\n", UART_485_TL16_6_DEV));
//		goto err_ret;
	}

	dev_485_tl16_7 = rt_device_find(UART_485_TL16_7_DEV);
	if (dev_485_tl16_7 != RT_NULL && rt_device_open(dev_485_tl16_7, RT_DEVICE_OFLAG_RDWR) == RT_EOK) {
		rs485_info(("open device:%s succ\n", UART_485_TL16_7_DEV));
		rt_device_set_rx_indicate(dev_485_tl16_7, uart485_tl16_7_rx_ind);
	} else {
		rs485_info(("can not find device:%s\n", UART_485_TL16_7_DEV));
//		goto err_ret;
	}

	dev_485_tl16_8 = rt_device_find(UART_485_TL16_8_DEV);
	if (dev_485_tl16_8 != RT_NULL && rt_device_open(dev_485_tl16_8, RT_DEVICE_OFLAG_RDWR) == RT_EOK) {
		rs485_info(("open device:%s succ\n", UART_485_TL16_8_DEV));
		rt_device_set_rx_indicate(dev_485_tl16_8, uart485_tl16_8_rx_ind);
	} else {
		rs485_info(("can not find device:%s\n", UART_485_TL16_8_DEV));
//		goto err_ret;
	}

	return;

//err_ret:
//	rt_free(rs485_recv_buf);
//	return;
}



rt_size_t send_data_by_485(USART_TypeDef *dev_485, void *data, rt_size_t len)
{
	rt_size_t size;

	if (NULL==dev_485 || NULL==data) {
		rs485_info(("send_data_by_485() param invalid\n"));
		return 0;
	}

	set_485_tx_rx_state(dev_485, 1);
	//rt_thread_delay(2);

	if (UART_485_1_DEV_PTR == dev_485) {
		size = dev_485_1->write(dev_485_1, 0, data, len);
	} else if (UART_485_2_DEV_PTR == dev_485) {
		size = dev_485_2->write(dev_485_2, 0, data, len);
	} else if (UART_485_3_DEV_PTR == dev_485) {
		size = dev_485_3->write(dev_485_3, 0, data, len);
	} else {
		size = 0;
		rs485_info(("revc invalid 485 dev param(0x%x)\n", dev_485));
	}
	//rt_thread_delay(1);
	wait_usartx_send_over(dev_485);

	set_485_tx_rx_state(dev_485, 0);

	return size;
}

rt_size_t recv_data_by_485(USART_TypeDef *dev_485, void *buf, rt_size_t len)
{
	rt_size_t size;

	if (NULL==dev_485 || NULL==buf) {
		rs485_info(("recv_data_by_485() param invalid\n"));
		return 0;
	}

	if (UART_485_1_DEV_PTR == dev_485) {
		size = dev_485_1->read(dev_485_1, 0, buf, len);
	} else if (UART_485_2_DEV_PTR == dev_485) {
		size = dev_485_2->read(dev_485_2, 0, buf, len);
	} else if (UART_485_3_DEV_PTR == dev_485) {
		size = dev_485_3->read(dev_485_3, 0, buf, len);
	} else {
		size = 0;
		rs485_info(("*revc invalid 485 dev param(0x%x)\n", dev_485));
	}

	return size;
}


static int set_485_tx_rx_state(USART_TypeDef *dev_485, int is_tx_state)
{
	if (UART_485_1_DEV_PTR == dev_485) {
		if (0 != is_tx_state)
			tx_en_rev_disable_485_1();
		else
			tx_disable_rev_en_485_1();
	} else if (UART_485_2_DEV_PTR == dev_485) {
		if (0 != is_tx_state)
			tx_en_rev_disable_485_2();
		else
			tx_disable_rev_en_485_2();
	} else if (UART_485_3_DEV_PTR == dev_485) {
		if (0 != is_tx_state)
			tx_en_rev_disable_485_3();
		else
			tx_disable_rev_en_485_3();
	} else {
		rs485_info(("**revc invalid 485 dev param(0x%x)\n", dev_485));
	}

	return 0;
}

#define u485_ind_debug(x) //rt_kprintf x
static rt_err_t uart485_1_rx_ind(rt_device_t dev, rt_size_t size)
{
//	rt_sem_release(&uart485_1_rx_byte_sem);
	rt_event_send(&em_protocol_data_event_set, EVENT_BIT_UART485_1_RECV_DATA);
	u485_ind_debug(("%s(), size:%d\n", __func__, size));
	return RT_EOK;
}

static rt_err_t uart485_2_rx_ind(rt_device_t dev, rt_size_t size)
{
//	rt_sem_release(&uart485_2_rx_byte_sem);
	rt_event_send(&em_protocol_data_event_set, EVENT_BIT_UART485_2_RECV_DATA);
	u485_ind_debug(("%s(), size:%d\n", __func__, size));
	return RT_EOK;
}

static rt_err_t uart485_3_rx_ind(rt_device_t dev, rt_size_t size)
{
//	rt_sem_release(&uart485_3_rx_byte_sem);
	rt_event_send(&em_protocol_data_event_set, EVENT_BIT_UART485_3_RECV_DATA);
	u485_ind_debug(("%s(), size:%d\n", __func__, size));
	return RT_EOK;
}


rt_size_t send_data_by_tl16_485(rt_device_t dev_485, void *data, rt_size_t len)
{
	rt_size_t size;

	if (NULL==dev_485 || NULL==data) {
		rs485_info(("send_data_by_485() param invalid\n"));
		return 0;
	}

	set_tl16_485_tx_rx_state(dev_485, 1);

	//rt_thread_delay(2);
	size = dev_485->write(dev_485, 0, data, len);
	//rt_thread_delay(1);

	while (TRUE != is_tl16_485_tx_over(dev_485))
		;

	set_tl16_485_tx_rx_state(dev_485, 0);

	return size;
}

rt_size_t recv_data_by_tl16_485(rt_device_t dev_485, void *buf, rt_size_t len)
{
	rt_size_t size;

	if (NULL==dev_485 || NULL==buf) {
		rs485_info(("recv_data_by_485() param invalid\n"));
		return 0;
	}

	size = dev_485->read(dev_485, 0, buf, len);

	return size;
}


static int set_tl16_485_tx_rx_state(rt_device_t dev_485, int is_tx_state)
{
	if (is_tx_state) {
		tl16_485_rx_ctrl(dev_485, 0);
		tl16_485_tx_ctrl(dev_485, 1);
	} else {
		tl16_485_tx_ctrl(dev_485, 0);
		tl16_485_rx_ctrl(dev_485, 1);
	}

	return 0;
}

unsigned rx_ind_cnt[8];

static rt_err_t uart485_tl16_1_rx_ind(rt_device_t dev, rt_size_t size)
{
	rt_event_send(&em_protocol_data_event_set, EVENT_BIT_TL16_1_RECV_DATA);
	++rx_ind_cnt[0];

	return RT_EOK;
}

static rt_err_t uart485_tl16_2_rx_ind(rt_device_t dev, rt_size_t size)
{
	rt_event_send(&em_protocol_data_event_set, EVENT_BIT_TL16_2_RECV_DATA);
	++rx_ind_cnt[1];

	return RT_EOK;
}

static rt_err_t uart485_tl16_3_rx_ind(rt_device_t dev, rt_size_t size)
{
	rt_event_send(&em_protocol_data_event_set, EVENT_BIT_TL16_3_RECV_DATA);
	++rx_ind_cnt[2];

	return RT_EOK;
}

static rt_err_t uart485_tl16_4_rx_ind(rt_device_t dev, rt_size_t size)
{
	rt_event_send(&em_protocol_data_event_set, EVENT_BIT_TL16_4_RECV_DATA);
	++rx_ind_cnt[3];

	return RT_EOK;
}

static rt_err_t uart485_tl16_5_rx_ind(rt_device_t dev, rt_size_t size)
{
	rt_event_send(&em_protocol_data_event_set, EVENT_BIT_TL16_5_RECV_DATA);
	++rx_ind_cnt[4];

	return RT_EOK;
}

static rt_err_t uart485_tl16_6_rx_ind(rt_device_t dev, rt_size_t size)
{
	rt_event_send(&em_protocol_data_event_set, EVENT_BIT_TL16_6_RECV_DATA);
	++rx_ind_cnt[5];

	return RT_EOK;
}

static rt_err_t uart485_tl16_7_rx_ind(rt_device_t dev, rt_size_t size)
{
	rt_event_send(&em_protocol_data_event_set, EVENT_BIT_TL16_7_RECV_DATA);
	++rx_ind_cnt[6];

	return RT_EOK;
}

static rt_err_t uart485_tl16_8_rx_ind(rt_device_t dev, rt_size_t size)
{
	rt_event_send(&em_protocol_data_event_set, EVENT_BIT_TL16_8_RECV_DATA);
	++rx_ind_cnt[7];

	return RT_EOK;
}

void print_tl16_rx_ind_cnt(void)
{
	printf_syn("print_tl16_rx_ind_cnt:%u, %u, %u, %u, %u, %u, %u, %u\n",
			rx_ind_cnt[0], rx_ind_cnt[1], rx_ind_cnt[2], rx_ind_cnt[3],
			rx_ind_cnt[4], rx_ind_cnt[5], rx_ind_cnt[6], rx_ind_cnt[7]);
}


void *get_us485_dev_ptr(enum lt_485if_e rs485_if, int is_stm32_uart_st)
{
	void *ptr;

	switch (rs485_if) {
	case LT_485_TL16_U1:	ptr = dev_485_tl16_1;	break;
	case LT_485_TL16_U2:	ptr = dev_485_tl16_2;	break;
	case LT_485_TL16_U3:	ptr = dev_485_tl16_3;	break;
	case LT_485_TL16_U4:	ptr = dev_485_tl16_4;	break;
	case LT_485_TL16_U5:	ptr = dev_485_tl16_5;	break;
	case LT_485_TL16_U6:	ptr = dev_485_tl16_6;	break;
	case LT_485_TL16_U7:	ptr = dev_485_tl16_7;	break;
	case LT_485_TL16_U8:	ptr = dev_485_tl16_8;	break;

	case LT_485_EMC_UART:
		if (is_stm32_uart_st)
			ptr = UART_485_2_DEV_PTR;
		else
			ptr = dev_485_2;
		break;

	case LT_485_EM_UART_1:
		if (is_stm32_uart_st)
			ptr = UART_485_3_DEV_PTR;
		else
			ptr = dev_485_3;
		break;

	case LT_485_EM_UART_2:
		if (is_stm32_uart_st)
			ptr = UART_485_1_DEV_PTR;
		else
			ptr = dev_485_1;
		break;

	default:
		ptr = NULL;
		rs485_log(("%s() param invalid(%d)\n", __func__, rs485_if));
		break;
	}

	return ptr;
}


#if 0
#include <finsh.h>

void read_tl16_485data_echo(enum lt_485if_e rs485_if, int *had_proc)
{
	rt_size_t num, cnt;
	rt_device_t dev_485;
	char buf[32];


	dev_485 = get_us485_dev_ptr(rs485_if, 0);

	num = recv_data_by_tl16_485(dev_485, buf, sizeof(buf));
	if (0 != num) {
		++*had_proc;
		rt_thread_delay(get_ticks_of_ms(200));
		send_data_by_tl16_485(dev_485, buf, num);

		printf_syn("\nrecv data(%d) from dev_485_tl16(rs485_if:%d), will echo:\n", num, rs485_if);
		for (cnt=0; cnt<num; ++cnt) {
			printf_syn("%x ", buf[cnt]);
		}
		printf_syn("\n");
	}

	return;
}

void read_uart_onchip_485data_echo(enum lt_485if_e rs485_if, int *had_proc)
{
	rt_size_t num, cnt;
	USART_TypeDef *dev_485;
	char buf[32];

	dev_485 = get_us485_dev_ptr(rs485_if, 1);

	num = recv_data_by_485(dev_485, buf, sizeof(buf));
	if (0 != num) {
		++*had_proc;
		rt_thread_delay(get_ticks_of_ms(200));
		send_data_by_485(dev_485, buf, num);

		printf_syn("\nrecv data(%d) from dev_485_uart_on_chip(rs485_if:%d), will echo:\n", num, rs485_if);
		for (cnt=0; cnt<num; ++cnt) {
			printf_syn("%x ", buf[cnt]);
		}
		printf_syn("\n");
	}

	return;
}


void test485p(void)
{
	rt_uint32_t e;
	rt_err_t err;
	int had_proc;

	rs485_log(("%s(), wait event of recv data from 485-port...\n", __func__));

	while (1) {
		err = rt_event_recv(&em_protocol_data_event_set,
				EVENT_BIT_TL16_1_RECV_DATA | EVENT_BIT_TL16_2_RECV_DATA
				| EVENT_BIT_TL16_3_RECV_DATA | EVENT_BIT_TL16_4_RECV_DATA
				| EVENT_BIT_TL16_5_RECV_DATA | EVENT_BIT_TL16_6_RECV_DATA
				| EVENT_BIT_TL16_7_RECV_DATA | EVENT_BIT_TL16_8_RECV_DATA
				| EVENT_BIT_UART485_1_RECV_DATA | EVENT_BIT_UART485_2_RECV_DATA
				| EVENT_BIT_UART485_3_RECV_DATA,
				RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, get_ticks_of_ms(6 * 1000), &e);

		if (RT_EOK != err) {
			rs485_log(("%s(), recv event error(%d)\n", __func__, err));
			break;
		}

		rs485_log(("%s() recv event 0x%x\n", __func__, e));

		rt_thread_delay(get_ticks_of_ms(1000));

		do {
			had_proc = 0;

			if (is_bit_set(e, EVENT_BIT_TL16_1_RECV_DATA)) {
				read_tl16_485data_echo(LT_485_TL16_U1, &had_proc);
			}

			if (is_bit_set(e, EVENT_BIT_TL16_2_RECV_DATA)) {
				read_tl16_485data_echo(LT_485_TL16_U2, &had_proc);
			}

			if (is_bit_set(e, EVENT_BIT_TL16_3_RECV_DATA)) {
				read_tl16_485data_echo(LT_485_TL16_U3, &had_proc);
			}

			if (is_bit_set(e, EVENT_BIT_TL16_4_RECV_DATA)) {
				read_tl16_485data_echo(LT_485_TL16_U4, &had_proc);
			}

			if (is_bit_set(e, EVENT_BIT_TL16_5_RECV_DATA)) {
				read_tl16_485data_echo(LT_485_TL16_U5, &had_proc);
			}

			if (is_bit_set(e, EVENT_BIT_TL16_6_RECV_DATA)) {
				read_tl16_485data_echo(LT_485_TL16_U6, &had_proc);
			}

			if (is_bit_set(e, EVENT_BIT_TL16_7_RECV_DATA)) {
				read_tl16_485data_echo(LT_485_TL16_U7, &had_proc);
			}

			if (is_bit_set(e, EVENT_BIT_TL16_8_RECV_DATA)) {
				read_tl16_485data_echo(LT_485_TL16_U8, &had_proc);
			}

			if (is_bit_set(e, EVENT_BIT_UART485_1_RECV_DATA)) {
				read_uart_onchip_485data_echo(LT_485_EM_UART_2, &had_proc);
			}

			if (is_bit_set(e, EVENT_BIT_UART485_2_RECV_DATA)) {
				read_uart_onchip_485data_echo(LT_485_EMC_UART, &had_proc);
			}

			if (is_bit_set(e, EVENT_BIT_UART485_3_RECV_DATA)) {
				read_uart_onchip_485data_echo(LT_485_EM_UART_1, &had_proc);
			}
		} while (0 != had_proc);
	}
	rs485_log(("%s(), proc over event of recv data from 485-port\n", __func__));
}

FINSH_FUNCTION_EXPORT(test485p, test-485-port);
#endif
