/*
 * File      : telnetio.c
 * COPYRIGHT (C) 2012, David
 *
 * Change Logs:
 * Date           Author       Notes
 * 2012-03-01     David        first version
 */

#include <rtdef.h>

#include <lwip/err.h>
#include <lwip/tcp.h>

#include <base_ds.h>
#include "telnetd.h"


#define MAX_TELIO_CNT 1

/*
 * 以下是telnetio_dev结构中td_flag的位含义
 */
#define IS_SEND_MAIL_FROM_SHELL2TELNET_BITMASK 0x01

#define TELNETIO_DEBUG(x)   //printf_syn x


/*
 * shell将数据写入telnetio_dev.tx_rb_buf时, 给tcpip线程发送消息
 */
struct telnet_shell_ipc {
	struct rt_mailbox telnet_shell_mb;
	rt_device_t *pool[10];
} tel_shell_ipc;

static int telio_cnt;


/* RT-Thread Device Interface */
static rt_err_t rt_telnetio_init (rt_device_t dev)
{
	return RT_EOK;
}

static rt_err_t rt_telnetio_open(rt_device_t dev, rt_uint16_t oflag)
{
	return RT_EOK;
}

static rt_err_t rt_telnetio_close(rt_device_t dev)
{
	return RT_EOK;
}

static rt_size_t rt_telnetio_read (rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
	struct tcp_pcb *pcb;
	struct telnetio_dev *p;
	rt_size_t cnt;

	if (NULL==dev || (0!=size && NULL==buffer))
		return 0;

	/* invoke callback */
	if (0==size && dev->rx_indicate != RT_NULL)
		dev->rx_indicate(dev, size);
	else {
		pcb = dev->user_data;
		p   = pcb->callback_arg;
		rt_sem_take(p->rx_rb_buf.rw_sem, RT_WAITING_FOREVER);
		cnt = rb_read(&(p->rx_rb_buf), buffer, size);
		rt_sem_release(p->rx_rb_buf.rw_sem);

		return cnt;
	}

	return 0;
}

static rt_size_t rt_telnetio_write (rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
	struct tcp_pcb *pcb;
	struct telnetio_dev *p;
	const char *pch;
	int cnt;

	(void)pos;

	if (NULL == dev)
		return 0;

	if (0!=size && NULL!=buffer) {
		pcb = dev->user_data;
		p   = pcb->callback_arg;
		rt_sem_take(p->tx_rb_buf.rw_sem, RT_WAITING_FOREVER);

		pch = buffer;
		cnt = size;
		while (cnt > 0) {
			if ('\n'!=*pch) {
				if (1 != rb_write(&p->tx_rb_buf, pch, 1))
					break;
			} else {
				if (2 != rb_write(&p->tx_rb_buf, (char*)"\r\n", 2))
					break;
			}
			++pch;
			--cnt;
		}

		rt_sem_release(p->tx_rb_buf.rw_sem);

		if (0!=(size -= cnt) && is_bit_clr(p->td_flag, IS_SEND_MAIL_FROM_SHELL2TELNET_BITMASK)) {
			if (RT_EOK == rt_mb_send(&tel_shell_ipc.telnet_shell_mb, (rt_uint32_t)dev))
				set_bit(p->td_flag, IS_SEND_MAIL_FROM_SHELL2TELNET_BITMASK);

		}
	}

	return size;
}

static rt_err_t rt_telnetio_control (rt_device_t dev, rt_uint8_t cmd, void *args)
{

	return RT_EOK;
}

/*
 * telnetio register
 */
rt_err_t rt_telnetio_register(rt_device_t device, const char* name, rt_uint32_t flag, struct tcp_pcb *pcb)
{
	RT_ASSERT(device != RT_NULL);

	device->type 		= RT_Device_Class_Char;
	device->rx_indicate = RT_NULL;
	device->tx_complete = RT_NULL;
	device->init 		= rt_telnetio_init;
	device->open		= rt_telnetio_open;
	device->close		= rt_telnetio_close;
	device->read 		= rt_telnetio_read;
	device->write 		= rt_telnetio_write;
	device->control 	= rt_telnetio_control;
	device->user_data	= pcb;

	/* register a character device */
	return rt_device_register(device, name, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STREAM | flag);
}

int telnetio_dev_no_get(void)
{
	return telio_cnt;
}


struct telnetio_dev *telnetio_dev_creat(void)
{
	int ret;
	struct telnetio_dev *p;

	if (telio_cnt >= MAX_TELIO_CNT)
		return NULL;

	p = mem_calloc(sizeof(struct telnetio_dev), 1);
	if (NULL == p) {
		return RT_NULL;
	} else {
		ret = rb_init(&p->rx_rb_buf, RX_RB_BUF_SIZE);
		if (RT_EOK != ret) {
			mem_free(p);
			return RT_NULL;
		}

		ret = rb_init(&p->tx_rb_buf, TX_RB_BUF_SIZE);
		if (RT_EOK != ret) {
			rb_destroy(&p->rx_rb_buf);
			mem_free(p);
			return RT_NULL;
		}
		p->td_flag = 0;
	}

	if (0 == telio_cnt) {
		rt_mb_init(& tel_shell_ipc.telnet_shell_mb,
				   "telsh",
				   tel_shell_ipc.pool,
				   sizeof(tel_shell_ipc.pool)/4,
				   RT_IPC_FLAG_FIFO);
	}
	++telio_cnt;

	return p;
}


void telnetio_dev_delete(struct telnetio_dev *teldev)
{
	if (NULL == teldev)
		return;

	rb_destroy(&teldev->rx_rb_buf);
	rb_destroy(&teldev->tx_rb_buf);
	mem_free(teldev);

	--telio_cnt;

	if (0 == telio_cnt)
		rt_mb_detach(&tel_shell_ipc.telnet_shell_mb);

	return;
}

static int telnetio_tx_rb_buf_flush(rt_device_t dev)
{
#define TEMP_BUF_SIZE (128)
	struct tcp_pcb *pcb;
	struct telnetio_dev *p;
	int num;
	err_t ret_err;
	char *buf;

	buf = rt_malloc(TEMP_BUF_SIZE);
	if (NULL == buf)
		return FAIL;

	pcb = dev->user_data;
	p   = pcb->callback_arg;
	/* invoke by tcpip_thread(), shoundn't use RT_WAITING_FOREVER */
	if (RT_EOK != rt_sem_take(p->tx_rb_buf.rw_sem, 0))
		return FAIL;

	while(0 != (num=rb_get_used_bytes_num(&p->tx_rb_buf))) {
		num = MIN(num, TEMP_BUF_SIZE);
		rb_read(&p->tx_rb_buf, buf, num);

		do {
			ret_err = tcp_write(pcb, buf, num, TCP_WRITE_FLAG_COPY);
			if (ERR_MEM == ret_err) {
				if ((tcp_sndbuf(pcb) == 0) ||
					(tcp_sndqueuelen(pcb) >= TCP_SND_QUEUELEN)) {
					num = 1; /* no need to try smaller sizes */
				} else {
					num /= 2;
				}
			}
		} while ((ret_err == ERR_MEM) && (num > 1));

		if (ret_err == ERR_MEM)
			break;
	}
	rt_sem_release(p->tx_rb_buf.rw_sem);

	rt_free(buf);

	return SUCC;
}


/* invoke by tcpip_thread() */
void try_flush_telio_txbuf(void)
{
	rt_device_t dev = NULL;
	struct tcp_pcb *pcb;
	struct telnetio_dev *p;
	rt_err_t ret_err;

	do {
		ret_err = rt_mb_recv(&tel_shell_ipc.telnet_shell_mb, (rt_uint32_t *)&dev, 0);

		if (RT_EOK==ret_err && NULL!=dev) {
			if (FAIL == telnetio_tx_rb_buf_flush(dev))
				break;

			pcb = dev->user_data;
			p   = pcb->callback_arg;
			clr_bit(p->td_flag, IS_SEND_MAIL_FROM_SHELL2TELNET_BITMASK);
			dev = NULL;
		} else {
			break;
		}
	} while (1);

	return;
}

