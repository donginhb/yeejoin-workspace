#include <rtgui/rtgui.h>
#include <rtgui/rtgui_server.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/driver.h>
#include <rtgui/widgets/view.h>
#include <rtgui/widgets/workbench.h>
#include <rtgui/widgets/notebook.h>
#include <rtgui/widgets/textbox.h>
#include <rtgui/widgets/window.h>

#include <phasex.h>

#include "zvd_gui_app.h"
#include <info_tran.h>


#define zvd_guiinit_debug(x) //printf_syn x
#define zvd_guiinit_log(x) printf_syn x


#ifdef RTGUI_USING_SMALL_SIZE
#define WB_MSG_SIZE 32
#else
#define WB_MSG_SIZE 256
#endif

rt_thread_t wb_thread;


static void panel_init(void);
static void workbench_init(void);
static void workbench_entry(void* parameter);
static rt_bool_t app_workbench_event_handler(struct rtgui_widget* widget, struct rtgui_event* event);
static rtgui_view_t* app_view_creat(rtgui_workbench_t* workbench, const char* title);

extern rtgui_win_t *switch2pt_confirm_win;
extern rtgui_win_t *sn_not_match_win;


/* GUI相关演示入口，需放在线程中进行初始化 */
void rtgui_startup(void)
{
	init_sys_use_channel();

	rtgui_system_server_init();

	panel_init();
	workbench_init();
}

/* 这个函数用于返回演示视图的对外可用区域 */
void app_view_get_can_use_rect(rtgui_view_t* view, rtgui_rect_t *rect)
{
	RT_ASSERT(view != RT_NULL);
	RT_ASSERT(rect != RT_NULL);

	rtgui_widget_get_rect(RTGUI_WIDGET(view), rect);
	rtgui_widget_rect_to_device(RTGUI_WIDGET(view), rect);
	/* 去除标题和状态的区域 */
	rect->y1 += TITLE_PANEL_HEIGHT;
	rect->y2 -= STATE_PANEL_HEIGHT;

	return;
}

#if 0
void app_view_get_logic_rect(rtgui_view_t* view, rtgui_rect_t *rect)
{
	RT_ASSERT(view != RT_NULL);
	RT_ASSERT(rect != RT_NULL);

	rtgui_widget_get_rect(RTGUI_WIDGET(view), rect);
	/* 去除标题和状态的区域 */
	rect->y1 += TITLE_PANEL_HEIGHT;
	rect->y2 -= STATE_PANEL_HEIGHT;

	return;
}
#endif



/*
 * Panel demo for 240x320
 * info panel: (0,  0) - (240, 25)
 * main panel: (0, 25) - (240, 320)
 */
static void panel_init(void)
{
	rtgui_rect_t rect;

	/* register main panel */
	rect.x1 = 0;
	rect.y1 = 0;
	rect.x2 = rtgui_graphic_driver_get_default()->width;
	rect.y2 = rtgui_graphic_driver_get_default()->height;

	zvd_guiinit_debug(("fun:%s(), line:%d, (%d,%d) (%d,%d)!\n", __FUNCTION__, __LINE__,
					   rect.x1, rect.y1, rect.x2, rect.y2));

	rtgui_panel_register("main", &rect);

	rtgui_panel_set_default_focused("main");

	return;
}


static void workbench_init(void)
{
	wb_thread = rt_thread_create("wb", workbench_entry, RT_NULL, 2048*2, 0x14, 10);

	if (wb_thread != RT_NULL)
		rt_thread_startup(wb_thread);

	return;
}


static void workbench_entry(void* parameter)
{
	rt_mq_t mq;
	struct rtgui_workbench* workbench;
	struct rtgui_view* view;

	mq = rt_mq_create("workbench", WB_MSG_SIZE, 32, RT_IPC_FLAG_FIFO); /* 创建GUI应用需要的消息队列 */
	if (mq == RT_NULL) return;
	rtgui_thread_register(rt_thread_self(), mq); /* 注册当前线程为GUI线程 */

	workbench = rtgui_workbench_create("main", "workbench");
	if (workbench == RT_NULL) {
		rt_kprintf("workbench create fail\n");
		return;
	}

	rtgui_widget_set_event_handler(RTGUI_WIDGET(workbench), app_workbench_event_handler);
	zvd_guiinit_debug(("set event handler:#%x\n", app_workbench_event_handler));

	view = app_view_creat(workbench, "mainv");
	if (SUCC != app_view_structure(workbench, view)) {
		rt_kprintf("app_view_structure() fail!\n");
		return;
	}

	rtgui_workbench_event_loop(workbench); /* 执行工作台事件循环 */

	app_view_clean();
	rtgui_workbench_destroy(workbench);
	rtgui_thread_deregister(rt_thread_self());
	rt_mq_delete(mq);

	return;
}


/*
 * 如果事件处理成功(即此事件是控件所感兴趣的事件，被这个空间所处理)，那么应该返回RT TRUE，
 * 上层事件处理函数将不再把此事件派发给其他事件处理函数进行处理。如果返回RT FALSE，上层
 * 事件处理函数将继续把这个事件传递给其他控件进行解析。
 */
static rt_bool_t app_workbench_event_handler(struct rtgui_widget* widget, struct rtgui_event* event)
{
	struct rtgui_event_command* ecmd;
	rtgui_widget_t *w;
//	char buf[8];

	zvd_guiinit_debug(("func:%s(), type:%d\n", __FUNCTION__, event->type));

	switch (event->type) {
	case RTGUI_EVENT_COMMAND:
		ecmd = (struct rtgui_event_command*)event;

		zvd_guiinit_debug(("func:%s(), cmd id:%d, cmd_str:%s\n", __FUNCTION__, ecmd->command_id, ecmd->command_string));

		if (UPDATE_SYS_TIME == ecmd->command_id) {
#if SYS_TIME_USE_TEXTBOX
			if (NULL != sys_time_textbox) {
				get_sys_time4label(sys_time_str);
				zvd_guiinit_debug(("func:%s(), sys_time_str:%s\n", __FUNCTION__, sys_time_str));
				rtgui_textbox_set_value(sys_time_textbox, sys_time_str);
				//rtgui_widget_update(RTGUI_WIDGET(sys_time_textbox));
			}
#else
			if (NULL != sys_time_textview) {
				get_sys_time4label(sys_time_str);
				zvd_guiinit_debug(("func:%s(), sys_time_str:%s\n", __FUNCTION__, sys_time_str));
				rtgui_textview_set_text(sys_time_textview, sys_time_str);
				//rtgui_widget_update(RTGUI_WIDGET(sys_time_textview));
			}

#endif
		} else if (UPDATE_SW_CABLE == ecmd->command_id) {
			if (NULL != switch_cable_yn_textview) {
				if (is_switch2pt_sysmisc()) {
					RTGUI_WIDGET_FOREGROUND(RTGUI_WIDGET(switch_cable_yn_textview)) = red;
					rtgui_textview_set_text(switch_cable_yn_textview, print_info_str[PIS_ID_SHI]);
				} else {
					RTGUI_WIDGET_FOREGROUND(RTGUI_WIDGET(switch_cable_yn_textview)) = black;
					rtgui_textview_set_text(switch_cable_yn_textview, print_info_str[PIS_ID_FOU]);
				}

				//rtgui_widget_update(RTGUI_WIDGET(switch_cable_yn_textview));
			}
		} else if (SWITCH_2_PT == ecmd->command_id) {
			if (NULL != switch_cable_yn_textview) {
				RTGUI_WIDGET_FOREGROUND(RTGUI_WIDGET(switch_cable_yn_textview)) = red;
				rtgui_textview_set_text(switch_cable_yn_textview, print_info_str[PIS_ID_SHI]);

				//RTGUI_WIDGET_FOREGROUND(RTGUI_WIDGET(switch_cable_yn_textview)) = black;
				//rtgui_textview_set_text(switch_cable_yn_textview, print_info_str[PIS_ID_FOU]);
			}
		}
#if 1==DISPLAY_OVERLOAD_INFO

		else if (UPDATE_OVERLOAD_CNT == ecmd->command_id) {
			if (NULL != rx_overload_cnt_textview) {
				if ('0' != ecmd->command_string[0]) {
					RTGUI_WIDGET_FOREGROUND(RTGUI_WIDGET(rx_overload_cnt_textview)) = red;
					rtgui_textview_set_text(rx_overload_cnt_textview, ecmd->command_string);
				} else {
					RTGUI_WIDGET_FOREGROUND(RTGUI_WIDGET(rx_overload_cnt_textview)) = black;
					rtgui_textview_set_text(rx_overload_cnt_textview, ecmd->command_string);
				}

				//rtgui_widget_update(RTGUI_WIDGET(rx_overload_cnt_textview));
			}

		}
#endif
		else if (UPDATE_HAD_POWEROFF == ecmd->command_id) {
			if (NULL != tx_poweroff_yn_textview) {
				if ('0' != ecmd->command_string[0]) {
					RTGUI_WIDGET_FOREGROUND(RTGUI_WIDGET(tx_poweroff_yn_textview)) = red;
					rtgui_textview_set_text(tx_poweroff_yn_textview, print_info_str[PIS_ID_SHI]);
				} else {
					RTGUI_WIDGET_FOREGROUND(RTGUI_WIDGET(tx_poweroff_yn_textview)) = black;
					rtgui_textview_set_text(tx_poweroff_yn_textview, print_info_str[PIS_ID_FOU]);
				}

				//rtgui_widget_update(RTGUI_WIDGET(tx_poweroff_yn_textview));
			}
		}
#if USE_OPTICX_200S_VERSION
		else if (UPDATE_CUR_CHANNEL_NO == ecmd->command_id) {
			if (NULL != rxe_cur_channel_no_textview) {
				if ('E'==ecmd->command_string[0]
				                              || 'P'==ecmd->command_string[0]) {
					RTGUI_WIDGET_FOREGROUND(RTGUI_WIDGET(rxe_cur_channel_no_textview)) = red;
					rtgui_textview_set_text(rxe_cur_channel_no_textview, ecmd->command_string);
				} else {
					RTGUI_WIDGET_FOREGROUND(RTGUI_WIDGET(rxe_cur_channel_no_textview)) = black;
					rtgui_textview_set_text(rxe_cur_channel_no_textview, ecmd->command_string);
				}
			}
		}
#endif
		else if (SN_NOT_MATCH == ecmd->command_id) {
			creat_sn_not_match_win(widget);
		} else if (SOFTWARE_VER_NOT_MATCH == ecmd->command_id) {
			creat_software_ver_not_match_win(widget);
		} else {
			w =	rtgui_notebook_get_current(zvd_notebook);
			zvd_guiinit_debug(("se_form:#%x, re_form:#%x, sys_form:#%x\n", se_form, re_form, sys_form));
			zvd_guiinit_debug(("notebook:#%x, w:#%x, cmd_id:%d, cur:%d, cnt:%d\n",
							   zvd_notebook, w, ecmd->command_id, zvd_notebook->current, rtgui_notebook_get_count(zvd_notebook)));
			zvd_guiinit_debug(("#%x, #%x, #%x\n", rtgui_notebook_get_index(zvd_notebook, 0), rtgui_notebook_get_index(zvd_notebook, 1),
							   rtgui_notebook_get_index(zvd_notebook, 2)));

			switch (ecmd->command_id) {
			case SE_FORM_UPDATE:
				if (RTGUI_WIDGET(se_form) == w)
					rtgui_widget_update(RTGUI_WIDGET(se_form));
				break;

			case RE_FORM_UPDATE:
				if (RTGUI_WIDGET(re_form) == w)
					rtgui_widget_update(RTGUI_WIDGET(re_form));
				break;

			case SYS_FORM_UPDATE:
				if (RTGUI_WIDGET(sys_form) == w)
					rtgui_widget_update(RTGUI_WIDGET(sys_form));
				break;

			case OTHER_FORM_UPDATE:
				if (RTGUI_WIDGET(other_form) == w)
					rtgui_widget_update(RTGUI_WIDGET(other_form));
				break;
			}

			if (ecmd->command_id>=SE_FORM_UPDATE && ecmd->command_id<=OTHER_FORM_UPDATE) {
				if (RT_NULL != sn_not_match_win) {
					rtgui_widget_update(RTGUI_WIDGET(sn_not_match_win));
				}

				if (RT_NULL != software_version_not_match_win) {
					rtgui_widget_update(RTGUI_WIDGET(software_version_not_match_win));
				}

				if (RT_NULL != switch2pt_confirm_win) {
					rtgui_widget_update(RTGUI_WIDGET(switch2pt_confirm_win));
				}
			}

		}
		break;

	default:
		zvd_guiinit_debug(("func:%s(), type:%d\n", __FUNCTION__, event->type));
		return rtgui_workbench_event_handler(widget, event);
	}

	return RT_FALSE;
}


/* 创建一个演示视图，需提供父workbench和演示用的标题 */
static rtgui_view_t* app_view_creat(rtgui_workbench_t* workbench, const char* title)
{
	struct rtgui_view* view;

	view = rtgui_view_create(title);
	if (view == RT_NULL) return RT_NULL;

	rtgui_workbench_add_view(workbench, view);

	return view;
}



#if 0
#ifdef RT_USING_FINSH
#include <finsh.h>

#include <rtgui/image.h>
#include <rtgui/dc.h>
#include <rtgui/driver.h>

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

	if (NULL == fname)
		return;

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
#endif
