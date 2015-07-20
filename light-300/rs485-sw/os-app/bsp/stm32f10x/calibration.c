#include <rtgui/rtgui.h>
#include <rtgui/dc.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/widgets/window.h>

#include "touch.h"

enum calibration_step_e {
	CALI_STEP_LEFTTOP	= 0,
	CALI_STEP_RIGHTTOP	= 1,
	CALI_STEP_RIGHTBOTTOM	= 2,
	CALI_STEP_LEFTBOTTOM	= 3,
	CALI_STEP_CENTER	= 4,
	CALI_STEP_CNT
};
#define TOUCH_WIN_UPDATE		1
#define TOUCH_WIN_CLOSE			2

#define CALIBRATION_WIDTH		15
#define CALIBRATION_HEIGHT		15

struct calibration_session {
	rt_uint8_t step;
	rt_uint16_t width;
	rt_uint16_t height;
	struct calibration_data data;

	int data_x[CALI_STEP_CNT];
	int data_y[CALI_STEP_CNT];

	rt_device_t device;
	rt_thread_t tid;
};
static struct calibration_session* calibration_ptr = RT_NULL;

#define abs(x) ((x)>=0 ? (x) : -(x))

int calc_max_xy(void)
{
	int delta_x, delta_y;
	int exchange_xy;
	int *px, *py;

	if (RT_NULL == calibration_ptr)
		return FAIL;

	px = calibration_ptr->data_x;
	py = calibration_ptr->data_y;
	exchange_xy = 0;
#if 0
	for (delta_x=0; delta_x<CALI_STEP_CNT; ++delta_x)
		rt_kprintf("index[%d]:(%d, %d)!\n", delta_x, px[delta_x], py[delta_x]);
#endif

	delta_x = px[CALI_STEP_RIGHTTOP] - px[CALI_STEP_LEFTTOP];
	delta_y = py[CALI_STEP_RIGHTTOP] - py[CALI_STEP_LEFTTOP];
	if (abs(delta_y) > abs(delta_x))
		exchange_xy = 1;

	delta_x = px[CALI_STEP_LEFTBOTTOM] - px[CALI_STEP_LEFTTOP];
	delta_y = py[CALI_STEP_LEFTBOTTOM] - py[CALI_STEP_LEFTTOP];
	if ((abs(delta_x) > abs(delta_y)) && (0 == exchange_xy)) {
		rt_kprintf("some error!\n");
		return FAIL;
	}

	if (0 == exchange_xy) {
		calibration_ptr->data.min_x = (px[CALI_STEP_LEFTTOP] + px[CALI_STEP_LEFTBOTTOM]) >> 1;
		calibration_ptr->data.max_x = (px[CALI_STEP_RIGHTTOP] + px[CALI_STEP_RIGHTBOTTOM]) >> 1;
		calibration_ptr->data.min_y = (py[CALI_STEP_LEFTTOP] + py[CALI_STEP_RIGHTTOP]) >> 1;
		calibration_ptr->data.max_y = (py[CALI_STEP_LEFTBOTTOM] + py[CALI_STEP_RIGHTBOTTOM]) >> 1;
	} else {
		calibration_ptr->data.min_x = (py[CALI_STEP_LEFTTOP] + py[CALI_STEP_LEFTBOTTOM]) >> 1;
		calibration_ptr->data.max_x = (py[CALI_STEP_RIGHTTOP] + py[CALI_STEP_RIGHTBOTTOM]) >> 1;
		calibration_ptr->data.min_y = (px[CALI_STEP_LEFTTOP] + px[CALI_STEP_RIGHTTOP]) >> 1;
		calibration_ptr->data.max_y = (px[CALI_STEP_LEFTBOTTOM] + px[CALI_STEP_RIGHTBOTTOM]) >> 1;
	}
	calibration_ptr->data.exchange_xy = exchange_xy;

	return SUCC;

}

static void calibration_data_post(rt_uint16_t x, rt_uint16_t y)
{
	rt_uint16_t w, h;
	struct rtgui_event_command ecmd;
	struct touch_calib_param touch_param;

	if (RT_NULL == calibration_ptr)
		return;

	calibration_ptr->data_x[calibration_ptr->step] = x;
	calibration_ptr->data_y[calibration_ptr->step] = y;
	rt_kprintf("[step-%d, (%d, %d)]\n", calibration_ptr->step, x, y);

	switch (calibration_ptr->step) {
	case CALI_STEP_LEFTTOP:
	case CALI_STEP_RIGHTTOP:
	case CALI_STEP_RIGHTBOTTOM:
	case CALI_STEP_LEFTBOTTOM:
		/*calibration_ptr->data_x[calibration_ptr->step] = x;
		calibration_ptr->data_y[calibration_ptr->step] = y;*/
		break;

	case CALI_STEP_CENTER:
		/*calibration_ptr->data_x[calibration_ptr->step] = x;
		calibration_ptr->data_y[calibration_ptr->step] = y;*/

		calc_max_xy();

		/* calibration done */
		RTGUI_EVENT_COMMAND_INIT(&ecmd);
		ecmd.command_id = TOUCH_WIN_CLOSE;

		/* calculate calibrated data */
		if (calibration_ptr->data.max_x > calibration_ptr->data.min_x)
			w = calibration_ptr->data.max_x - calibration_ptr->data.min_x;
		else
			w = calibration_ptr->data.min_x - calibration_ptr->data.max_x;
		w = (w*CALIBRATION_WIDTH) / (calibration_ptr->width - 2*CALIBRATION_WIDTH);

		if (calibration_ptr->data.max_y > calibration_ptr->data.min_y)
			h = calibration_ptr->data.max_y - calibration_ptr->data.min_y;
		else
			h = calibration_ptr->data.min_y - calibration_ptr->data.max_y;
		h = (h*CALIBRATION_HEIGHT) / (calibration_ptr->height - 2*CALIBRATION_HEIGHT);

		rt_kprintf("w: %d, h: %d, exchange_xy:%d\n", w, h, calibration_ptr->data.exchange_xy);

		if (calibration_ptr->data.max_x > calibration_ptr->data.min_x) {
			calibration_ptr->data.min_x -= w;
			calibration_ptr->data.max_x += w;
		} else {
			calibration_ptr->data.min_x += w;
			calibration_ptr->data.max_x -= w;
		}

		if (calibration_ptr->data.max_y > calibration_ptr->data.min_y) {
			calibration_ptr->data.min_y -= h;
			calibration_ptr->data.max_y += h;
		} else {
			calibration_ptr->data.min_y += h;
			calibration_ptr->data.max_y -= h;
		}
		rt_kprintf("calibration data(min, max): (%d, %d), (%d, %d)\n",
				   calibration_ptr->data.min_x, calibration_ptr->data.max_x,
				   calibration_ptr->data.min_y, calibration_ptr->data.max_y);
		touch_param.x_min = calibration_ptr->data.min_x;
		touch_param.x_max = calibration_ptr->data.max_x;
		touch_param.y_min = calibration_ptr->data.min_y;
		touch_param.y_max = calibration_ptr->data.max_y;
		set_touch_param(&touch_param);

		rtgui_thread_send(calibration_ptr->tid, &ecmd.parent, sizeof(struct rtgui_event_command));
		return;
	}
	calibration_ptr->step++;

	/* post command event */
	RTGUI_EVENT_COMMAND_INIT(&ecmd);
	ecmd.command_id = TOUCH_WIN_UPDATE;
	rtgui_thread_send(calibration_ptr->tid, &ecmd.parent, sizeof(struct rtgui_event_command));

	return;
}

rt_bool_t calibration_event_handler(struct rtgui_widget* widget, struct rtgui_event* event)
{
	struct rtgui_dc* dc;
	struct rtgui_rect rect;
	struct rtgui_event_command* ecmd;

	switch (event->type) {
	case RTGUI_EVENT_PAINT:
		dc = rtgui_dc_begin_drawing(widget);
		if (dc == RT_NULL) break;

		/* get rect information */
		rtgui_widget_get_rect(widget, &rect);

		/* clear whole window */
		RTGUI_WIDGET_BACKGROUND(widget) = white;
		rtgui_dc_fill_rect(dc, &rect);

		/* reset color */
		RTGUI_WIDGET_BACKGROUND(widget) = green;
		RTGUI_WIDGET_FOREGROUND(widget) = black;

		switch (calibration_ptr->step) {
		case CALI_STEP_LEFTTOP:
			rtgui_dc_draw_hline(dc, 0, 2 * CALIBRATION_WIDTH, CALIBRATION_HEIGHT);
			rtgui_dc_draw_vline(dc, CALIBRATION_WIDTH, 0, 2 * CALIBRATION_HEIGHT);
			RTGUI_WIDGET_FOREGROUND(widget) = blue;
			rtgui_dc_fill_circle(dc, CALIBRATION_WIDTH, CALIBRATION_HEIGHT, 4);
			break;

		case CALI_STEP_RIGHTTOP:
			rtgui_dc_draw_hline(dc, calibration_ptr->width - 2 * CALIBRATION_WIDTH,
								calibration_ptr->width, CALIBRATION_HEIGHT);
			rtgui_dc_draw_vline(dc, calibration_ptr->width - CALIBRATION_WIDTH, 0, 2 * CALIBRATION_HEIGHT);
			RTGUI_WIDGET_FOREGROUND(widget) = blue;
			rtgui_dc_fill_circle(dc, calibration_ptr->width - CALIBRATION_WIDTH, CALIBRATION_HEIGHT, 4);
			break;

		case CALI_STEP_LEFTBOTTOM:
			rtgui_dc_draw_hline(dc, 0, 2 * CALIBRATION_WIDTH, calibration_ptr->height - CALIBRATION_HEIGHT);
			rtgui_dc_draw_vline(dc, CALIBRATION_WIDTH, calibration_ptr->height - 2 * CALIBRATION_HEIGHT, calibration_ptr->height);
			RTGUI_WIDGET_FOREGROUND(widget) = blue;
			rtgui_dc_fill_circle(dc, CALIBRATION_WIDTH, calibration_ptr->height - CALIBRATION_HEIGHT, 4);
			break;

		case CALI_STEP_RIGHTBOTTOM:
			rtgui_dc_draw_hline(dc, calibration_ptr->width - 2 * CALIBRATION_WIDTH,
								calibration_ptr->width, calibration_ptr->height - CALIBRATION_HEIGHT);
			rtgui_dc_draw_vline(dc, calibration_ptr->width - CALIBRATION_WIDTH, calibration_ptr->height - 2 * CALIBRATION_HEIGHT, calibration_ptr->height);
			RTGUI_WIDGET_FOREGROUND(widget) = blue;
			rtgui_dc_fill_circle(dc, calibration_ptr->width - CALIBRATION_WIDTH, calibration_ptr->height - CALIBRATION_HEIGHT, 4);
			break;

		case CALI_STEP_CENTER:
			rtgui_dc_draw_hline(dc, calibration_ptr->width/2 - CALIBRATION_WIDTH, calibration_ptr->width/2 + CALIBRATION_WIDTH, calibration_ptr->height/2);
			rtgui_dc_draw_vline(dc, calibration_ptr->width/2, calibration_ptr->height/2 - CALIBRATION_HEIGHT, calibration_ptr->height/2 + CALIBRATION_HEIGHT);
			RTGUI_WIDGET_FOREGROUND(widget) = blue;
			rtgui_dc_fill_circle(dc, calibration_ptr->width/2, calibration_ptr->height/2, 4);
			break;
		}
		rtgui_dc_end_drawing(dc);
		break;

	case RTGUI_EVENT_COMMAND:
		ecmd = (struct rtgui_event_command*)event;
		switch (ecmd->command_id) {
		case TOUCH_WIN_UPDATE:
			rtgui_widget_update(widget);
			break;
		case TOUCH_WIN_CLOSE:
			syscfgdata_syn_proc();
			rtgui_win_close(RTGUI_WIN(widget));
			break;
		}
		return RT_TRUE;

	default:
		rtgui_win_event_handler(widget, event);
	}

	return RT_FALSE;
}

void calibration_entry(void* parameter)
{
	rt_mq_t mq;
	rtgui_win_t* win;
	struct rtgui_rect rect;

	say_thread_start();

	mq = rt_mq_create("cali", 40, 8, RT_IPC_FLAG_FIFO);
	if (mq == RT_NULL) return;

	rtgui_thread_register(rt_thread_self(), mq);

	rtgui_graphic_driver_get_rect(rtgui_graphic_driver_get_default(), &rect);

	/* set screen rect */
	calibration_ptr->width = rect.x2;
	calibration_ptr->height = rect.y2;

	rt_kprintf("fun:%s, w:%d, h:%d\n", __FUNCTION__, rect.x2, rect.y2);
	/* create calibration window */
	win = rtgui_win_create(RT_NULL,
						   "calibration", &rect, RTGUI_WIN_STYLE_NO_TITLE | RTGUI_WIN_STYLE_NO_BORDER);
	rtgui_widget_set_event_handler(RTGUI_WIDGET(win), calibration_event_handler);
	if (win != RT_NULL) {
		rtgui_win_show(win, RT_FALSE);
		// rtgui_widget_update(RTGUI_WIDGET(win));
		rtgui_win_event_loop(win);
	}

	rtgui_thread_deregister(rt_thread_self());
	rt_mq_delete(mq);

	/* set calibration data */
	rt_device_control(calibration_ptr->device, RT_TOUCH_CALIBRATION_DATA, &calibration_ptr->data);

	/* recover to normal */
	rt_device_control(calibration_ptr->device, RT_TOUCH_NORMAL, RT_NULL);

	/* release memory */
	rt_free(calibration_ptr);
	calibration_ptr = RT_NULL;
}

void calibration_init()
{
	rt_device_t device;

	device = rt_device_find("touch");
	if (device == RT_NULL) return; /* no this device */

	calibration_ptr = (struct calibration_session*)rt_malloc(sizeof(struct calibration_session));
	rt_memset(calibration_ptr, 0, sizeof(struct calibration_data));
	calibration_ptr->device = device;

	rt_device_control(calibration_ptr->device, RT_TOUCH_CALIBRATION, (void*)calibration_data_post);

	calibration_ptr->tid = rt_thread_create("cali", calibration_entry, RT_NULL,
											2048, 20, 5);
	if (calibration_ptr->tid != RT_NULL) rt_thread_startup(calibration_ptr->tid);
}

#ifdef RT_USING_FINSH
#include <finsh.h>
void calibration()
{
	calibration_init();
}
FINSH_FUNCTION_EXPORT(calibration, perform touch calibration);
#endif
