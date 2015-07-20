#include <rtgui/rtgui.h>
#include <rtgui/rtgui_system.h>

#include <rtgui/widgets/view.h>
#include <rtgui/widgets/workbench.h>
#include "demo_view.h"
#include "demo_view_x.h"

#include <rtgui/image.h>
#include <rtgui/dc.h>
#include <rtgui/driver.h>

static rt_bool_t demo_workbench_event_handler(struct rtgui_widget* widget, struct rtgui_event* event)
{
	/* 我们目前只对按键事件感兴趣。如果当前workbench处于模式显示状态，忽略它  */
	if ((event->type == RTGUI_EVENT_KBD) && !RTGUI_WORKBENCH_IS_MODAL_MODE(RTGUI_WORKBENCH(widget))) {
		struct rtgui_event_kbd* ekbd = (struct rtgui_event_kbd*)event;

		if (ekbd->type == RTGUI_KEYDOWN) {
			if (ekbd->key == RTGUIK_RIGHT) {
				demo_view_next(RT_NULL, RT_NULL);
				return RT_TRUE;
			} else if (ekbd->key == RTGUIK_LEFT) {
				demo_view_prev(RT_NULL, RT_NULL);
				return RT_TRUE;
			}
		}
	}

	/* 如果不是绘制事件，使用view原来的事件处理函数处理 */
	return rtgui_workbench_event_handler(widget, event);
}

static void workbench_entry(void* parameter)
{
	rt_mq_t mq;
	struct rtgui_workbench* workbench;

	/* 创建GUI应用需要的消息队列 */
#ifdef RTGUI_USING_SMALL_SIZE
	mq = rt_mq_create("workbench", 32, 32, RT_IPC_FLAG_FIFO);
#else
	mq = rt_mq_create("workbench", 256, 32, RT_IPC_FLAG_FIFO);
#endif
	/* 注册当前线程为GUI线程 */
	rtgui_thread_register(rt_thread_self(), mq);

	/* 创建一个工作台 */
	workbench = rtgui_workbench_create("main", "workbench");
	if (workbench == RT_NULL) return;

	rtgui_widget_set_event_handler(RTGUI_WIDGET(workbench), demo_workbench_event_handler);

	/* 初始化各个例子的视图 */
#if RT_VERSION == 4
	demo_view_benchmark(workbench);
#endif

	demo_view_dc(workbench);
#if RT_VERSION == 4
#ifdef RTGUI_USING_TTF
	demo_view_ttf(workbench);
#endif
#endif

#ifndef RTGUI_USING_SMALL_SIZE
	demo_view_dc_buffer(workbench);
#endif
	demo_view_animation(workbench);
#ifndef RTGUI_USING_SMALL_SIZE
	demo_view_buffer_animation(workbench);
	// demo_view_instrument_panel(workbench);
#endif
	demo_view_window(workbench);
	demo_view_label(workbench);
	demo_view_button(workbench);
	demo_view_checkbox(workbench);
	demo_view_progressbar(workbench);
	demo_view_scrollbar(workbench);
	demo_view_radiobox(workbench);
	demo_view_textbox(workbench);
	demo_view_listbox(workbench);
	demo_view_menu(workbench);
	demo_view_listctrl(workbench);
	demo_view_combobox(workbench);
	demo_view_slider(workbench);
	demo_view_notebook(workbench);
	demo_view_mywidget(workbench);
#if defined(RTGUI_USING_DFS_FILERW) || defined(RTGUI_USING_STDIO_FILERW)
	demo_view_image(workbench);
#endif
#ifdef RT_USING_MODULE
#if defined(RTGUI_USING_DFS_FILERW) || defined(RTGUI_USING_STDIO_FILERW)
	demo_view_module(workbench);
#endif
#endif
	demo_listview_view(workbench);
	demo_listview_icon_view(workbench);
#if defined(RTGUI_USING_DFS_FILERW) || defined(RTGUI_USING_STDIO_FILERW)
	demo_fn_view(workbench);
#endif

	/* 显示视图 */
	demo_view_show();

	/* 执行工作台事件循环 */
	rtgui_workbench_event_loop(workbench);

	/* 去注册GUI线程 */
	rtgui_thread_deregister(rt_thread_self());
	rt_mq_delete(mq);
}

void workbench_init()
{
	static rt_bool_t inited = RT_FALSE;

	if (inited == RT_FALSE) { /* 避免重复初始化而做的保护 */
		rt_thread_t tid;

		tid = rt_thread_create("wb",
							   workbench_entry, RT_NULL,
							   2048 * 2, 0x14, 10);

		if (tid != RT_NULL) rt_thread_startup(tid);

		inited = RT_TRUE;
	}
}

#ifdef RT_USING_FINSH
#include <finsh.h>

void workbench()
{
	workbench_init();
}
/* finsh的命令输出，可以直接执行workbench()函数以执行上面的函数 */
FINSH_FUNCTION_EXPORT(workbench, workbench demo)

/**/
struct rtgui_dc_hw {
	struct rtgui_dc parent;
	rtgui_widget_t *owner;
	const struct rtgui_graphic_driver* hw_driver;
};

extern const struct rtgui_dc_engine dc_hw_engine;
extern struct rtgui_image_engine rtgui_image_hdc_engine;

void load_img_test(char *fname)
{
	struct rtgui_filerw *filerw;
	struct rtgui_image image;
	struct rtgui_rect rect;
	struct rtgui_dc_hw *dc;
	struct rtgui_image_engine *img_eng;

	printf_syn("fun:%s, line:%d, fn:%s\n", __FUNCTION__, __LINE__, fname);

	filerw = rtgui_filerw_create_file(fname, "rb");
	if (NULL == filerw) {
		printf_syn("fun:%s, line:%d, fn:%s\n", __FUNCTION__, __LINE__, fname);
		return;
	}

	img_eng = &rtgui_image_hdc_engine;

	if (RT_TRUE != img_eng->image_check(filerw)) {
		printf_syn("fun:%s, line:%d\n", __FUNCTION__, __LINE__);
		return;
	}

	if (RT_TRUE != img_eng->image_load(&image, filerw, RT_FALSE)) {
		printf_syn("fun:%s, line:%d\n", __FUNCTION__, __LINE__);
		return;
	}

	dc = (struct rtgui_dc_hw*) rtgui_malloc(sizeof(struct rtgui_dc_hw));
	dc->parent.type = RTGUI_DC_HW;
	dc->parent.engine = &dc_hw_engine;
	dc->owner = NULL;
	dc->hw_driver = rtgui_graphic_driver_get_default();

	rect.x1 = 0;
	rect.y1 = 0;
	rect.x2 = rtgui_graphic_driver_get_default()->width;
	rect.y2 = rtgui_graphic_driver_get_default()->height;
	printf_syn("fun:%s, line:%d, 0x%x\n", __FUNCTION__, __LINE__, img_eng->image_blit);
	img_eng->image_blit(&image, (struct rtgui_dc*)dc, &rect);

	rt_thread_delay(get_ticks_of_ms(5000));
	printf_syn("fun:%s, line:%d, fn:%s\n", __FUNCTION__, __LINE__, fname);

	img_eng->image_unload(&image);
	printf_syn("fun:%s, line:%d, fn:%s\n", __FUNCTION__, __LINE__, fname);

	//rect.x1 = rtgui_filerw_close(filerw); /* unload 已close */

	printf_syn("fun:%s, line:%d, ret:%d, fn:%s\n", __FUNCTION__, __LINE__, rect.x1, fname);

	rtgui_free(dc);
	return;
}
FINSH_FUNCTION_EXPORT(load_img_test, load_img_test filename)

#endif
