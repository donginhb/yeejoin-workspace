/*
 ******************************************************************************
 * rxe_msg_proc.c
 *
 * 2013-03-14,  creat by David, zhaoshaowei@yeejoin.com
 ******************************************************************************
 */
#include <rtdef.h>
#include <rtthread.h>
#include <board.h>
#include <form.h>
#include <phasex.h>
#include <info_tran.h>
#include <zvd_gui_app.h>

struct rt_semaphore recv_rxe_msg_sem;

static rt_err_t recv_rxe_rx_ind(rt_device_t dev, rt_size_t size);


void rt_recv_rxe_msg_proc_entry(void* parameter)
{
	rt_device_t dev;
	rt_err_t ret;

	dev = rt_device_find(RECV_REX_DEVICE);
	if (dev != RT_NULL && rt_device_open(dev, RT_DEVICE_OFLAG_RDWR) == RT_EOK) {
		rt_device_set_rx_indicate(dev, recv_rxe_rx_ind);
	} else {
		rt_kprintf("can not find recv_rxe_msg device:%s\n", RECV_REX_DEVICE);
		return;
	}

	while (1) {
		ret = rt_sem_take(&recv_rxe_msg_sem, RT_WAITING_FOREVER);
		if (RT_EOK == ret) {
			info_tran_stream_analysis(dev);
		} else {
			rt_kprintf("take recv_rxe_msg_sem error(%d)\n", ret);
		}
	}

	return;
}


static rt_err_t recv_rxe_rx_ind(rt_device_t dev, rt_size_t size)
{
	/* release semaphore to let recv_rxe_msg_proc thread rx data */
	rt_sem_release(&recv_rxe_msg_sem);

	return RT_EOK;
}



#if 0
#include <finsh.h>


int set_form_item(int cmd)
{
	struct rtgui_event_command ecmd;
	rt_thread_t th;

	th = rt_thread_find("wb");
	if (NULL == th) {
		rt_kprintf("func:%s, error\n", __FUNCTION__);
		return 1;
	}

	rt_kprintf("wb thread:#%x @ func:%s()\n", th, __FUNCTION__);

	switch (cmd) {
	case 0:
		rtgui_form_set_item(se_form, print_info_str[PIS_ID_GUOGAO], 1, 1, 0);

		/* post command event */
		RTGUI_EVENT_COMMAND_INIT(&ecmd);
		ecmd.command_id = SE_FORM_UPDATE;
		if (RT_EOK != rtgui_thread_send(th, &ecmd.parent, sizeof(struct rtgui_event_command)))
			rt_kprintf("send enent cmd fail\n");
		else
			rt_kprintf("send enent cmd succ\n");
		break;

	case 1:
		rtgui_form_set_item(se_form, print_info_str[PIS_ID_SHI], 1, 2, 0);
		rtgui_form_ondraw(se_form);
		break;

	default:
		break;
	}

	return 0;
}
FINSH_FUNCTION_EXPORT(set_form_item, cmd);
#endif

