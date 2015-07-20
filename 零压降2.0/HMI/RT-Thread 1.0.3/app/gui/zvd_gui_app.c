#include <rtgui/widgets/notebook.h>
#include <rtgui/widgets/label.h>
#include <misc_lib.h>

#include <zvd_gui_app.h>
#include <form.h>
#include <phasex.h>
#include <sys_cfg_api.h>
#include <info_tran.h>
#include <rtgui/widgets/button.h>
#include <rtgui/widgets/textbox.h>
#include <rtgui/widgets/window.h>
#include <rtgui/color.h>

#include <time.h>
#include <board.h>

#define zvd_gui_debug(x) //printf_syn x

rtgui_form_t *se_form;
rtgui_form_t *re_form;
rtgui_form_t *sys_form;
rtgui_form_t *other_form;
rtgui_win_t *switch2pt_confirm_win;
rtgui_win_t *sn_not_match_win;
rtgui_win_t *software_version_not_match_win;


static void cancel_btn_onbutton(rtgui_widget_t* widget, struct rtgui_event* event);
static void sure_btn_onbutton(rtgui_widget_t* widget, struct rtgui_event* event);
static void creat_switch2pt_win(rtgui_widget_t* widget);

const char *re_form_head[4] = {
	"相 ",
	"有效值",
	"输出 ",
	NULL
};

const char *se_form_head[4] = {
	"相 ",
	"超限 ",
	"缺相 ",
	NULL
};

const char *sys_form_head[5] = {
	"设备  ",
	"CPU温度",
	"恒温温度",
	"版本号",
	NULL
};

const char *other_form_head[4] = {
	"    ",
	"发射端掉电   ",
	"接收端掉电   ",
	NULL
};

const char * const print_info_str[7] = {
	"过高",
	"正常",
	"过低",
	"异常",
	"是",
	"否",
	NULL
};


volatile unsigned long dev_px_state_vec = 0x07;
/* 记录发射端/接收端三相电压是否超限 */
volatile unsigned long dev_px_vol_state_vec = SRE_PX_VOL_NORMAL_BIT;

short int avg_val[AVI_RE_END];
short int sys_temper[STI_XE_T_END];

char sys_time_str[20];
rt_device_t rtc_dev;
#if SYS_TIME_USE_TEXTBOX
struct rtgui_textbox *sys_time_textbox;
#else
rtgui_textview_t *sys_time_textview;
#endif

/* 是否禁止蜂鸣器工作 */
volatile int disable_buzzer;
/* 蜂鸣器是否需要鸣叫 */
volatile int buzzer_tweet;

static void set_form_px_col(rtgui_form_t *form);
static void buzzer_btn_onbutton(rtgui_widget_t* widget, struct rtgui_event* event);
static void switch2pt_btn_onbutton(rtgui_widget_t* widget, struct rtgui_event* event);

rtgui_notebook_t *zvd_notebook;

//rtgui_label_t *buzzer_label;
rtgui_button_t *buzzer_button;
rtgui_button_t *switch2pt_button;

rtgui_label_t *switch_cable_label;
rtgui_textview_t *switch_cable_yn_textview;

rtgui_label_t *tx_poweroff_label;
rtgui_textview_t *tx_poweroff_yn_textview;

#if USE_OPTICX_200S_VERSION
rtgui_label_t *rxe_cur_channel_label;
rtgui_textview_t *rxe_cur_channel_no_textview;
#endif

#if 1==DISPLAY_OVERLOAD_INFO
rtgui_textview_t *rx_overload_cnt_textview;
#endif

#if 0
/* 用于显示水平方向的radio文本项数组 */
static char* radio_item_h[2] = {"是", "否"};
#endif

extern unsigned char tx_dev_sn[DEV_SN_LEN_MAX+1];	/* 设备序列号 */

extern int creat_devsn(char *str, int len, int tx_rx_e);


/*
 * 构建应用视图
 */
int app_view_structure(struct rtgui_workbench *workbench, struct rtgui_view *view)
{
	int ret = SUCC;
	rtgui_rect_t rect, body_rect;
	rtgui_label_t* label;
	char str[48];
	//rtgui_radiobox_t *radiobox;
	//rtgui_button_t* button;
	struct tm *ptime;
#if 1==DISPLAY_OVERLOAD_INFO
	rtgui_label_t *rx_overload_label;
#endif

	app_view_get_can_use_rect(view, &rect);
	zvd_gui_debug(("fun:%s(), line:%d, (%d,%d) (%d,%d)!\n", __FUNCTION__, __LINE__,
				   rect.x1, rect.y1, rect.x2, rect.y2));

	body_rect.x1 = 0;
	body_rect.y1 = 0;
	body_rect.x2 = rect.x2;
	body_rect.y2 = TITLE_PANEL_HEIGHT;
	label = rtgui_label_create(DEVICE_NAME);
	if (NULL != label) {
		RTGUI_WIDGET_TEXTALIGN(RTGUI_WIDGET(label)) = RTGUI_ALIGN_CENTER_HORIZONTAL | RTGUI_ALIGN_CENTER_VERTICAL;
		RTGUI_WIDGET_FOREGROUND(RTGUI_WIDGET(label)) = blue;
		rtgui_widget_set_rect(RTGUI_WIDGET(label), &body_rect);
		/* view是一个container控件，调用add_child方法添加这个label控件 */
		rtgui_container_add_child(RTGUI_CONTAINER(view), RTGUI_WIDGET(label));
	} else {
		printf_syn("creat title label fail\n");
	}

	body_rect.x1 = 0;
	body_rect.y1 = TITLE_PANEL_HEIGHT;
	body_rect.x2 = rect.x2;
	body_rect.y2 = rect.y2;
	zvd_gui_debug(("fun:%s(), line:%d, (%d,%d) (%d,%d)!\n", __FUNCTION__, __LINE__,
				   body_rect.x1, body_rect.y1, body_rect.x2, body_rect.y2));

	zvd_notebook = rtgui_notebook_create(&body_rect, RTGUI_NOTEBOOK_TOP);
	rtgui_container_add_child(RTGUI_CONTAINER(view), RTGUI_WIDGET(zvd_notebook));

	re_form = rtgui_form_create(re_form_head, RE_FORM_ROWS, RE_FORM_COLS, &body_rect);
	rtgui_notebook_add(zvd_notebook, "接收端", RTGUI_WIDGET(re_form));
#if 1
	set_form_px_col(re_form);
	i2str(str, avg_val[AVI_RE_PA]);
	rtgui_form_set_item(re_form, str, RE_PA_AVG_VALUE_ROW, RE_PA_AVG_VALUE_COL, 0);
	rtgui_form_set_item(re_form, print_info_str[PIS_ID_ZHENGCHANG], RE_PA_OVER_RANGE_ROW, RE_PA_OVER_RANGE_COL, 0);

	i2str(str, avg_val[AVI_RE_PB]);
	rtgui_form_set_item(re_form, str, RE_PB_AVG_VALUE_ROW, RE_PB_AVG_VALUE_COL, 0);
	rtgui_form_set_item(re_form, print_info_str[PIS_ID_ZHENGCHANG], RE_PB_OVER_RANGE_ROW, RE_PB_OVER_RANGE_COL, 0);

	i2str(str, avg_val[AVI_RE_PC]);
	rtgui_form_set_item(re_form, str, RE_PC_AVG_VALUE_ROW, RE_PC_AVG_VALUE_COL, 0);
	rtgui_form_set_item(re_form, print_info_str[PIS_ID_ZHENGCHANG], RE_PC_OVER_RANGE_ROW, RE_PC_OVER_RANGE_COL, 0);
#endif

	se_form = rtgui_form_create(se_form_head, SE_FORM_ROWS, SE_FORM_COLS, &body_rect);
	rtgui_notebook_add(zvd_notebook, "发射端", RTGUI_WIDGET(se_form));
#if 1
	set_form_px_col(se_form);
	rtgui_form_set_item(se_form, print_info_str[PIS_ID_ZHENGCHANG], SE_PA_OVER_RANGE_ROW, SE_PA_OVER_RANGE_COL, 0);
	rtgui_form_set_item(se_form, print_info_str[PIS_ID_FOU], SE_PA_POWER_DOWN_ROW, SE_PA_POWER_DOWN_COL, 0);

	rtgui_form_set_item(se_form, print_info_str[PIS_ID_ZHENGCHANG], SE_PB_OVER_RANGE_ROW, SE_PB_OVER_RANGE_COL, 0);
	rtgui_form_set_item(se_form, print_info_str[PIS_ID_FOU], SE_PB_POWER_DOWN_ROW, SE_PB_POWER_DOWN_COL, 0);

	rtgui_form_set_item(se_form, print_info_str[PIS_ID_ZHENGCHANG], SE_PC_OVER_RANGE_ROW, SE_PC_OVER_RANGE_COL, 0);
	rtgui_form_set_item(se_form, print_info_str[PIS_ID_FOU], SE_PC_POWER_DOWN_ROW, SE_PC_POWER_DOWN_COL, 0);
#endif

	sys_form = rtgui_form_create(sys_form_head, SYS_FORM_ROWS, SYS_FORM_COLS, &body_rect);
	rtgui_notebook_add(zvd_notebook, "系统", RTGUI_WIDGET(sys_form));
#if 1
	rtgui_form_set_item(sys_form, "接收端", 1, 0, 0);
	i2str(str, sys_temper[STI_RE_CPU_T]);
	rtgui_form_set_item(sys_form, str, 		RE_RE_CPU_TEMP_ROW, RE_RE_CPU_TEMP_COL, 0);
	i2str(str, sys_temper[STI_RE_CTB_T]);
	rtgui_form_set_item(sys_form, str, 		RE_RE_BOX_TEMP_ROW, RE_RE_BOX_TEMP_COL, 0);
	convert_ver2str(re_version, str);
	rtgui_form_set_item(sys_form, str, 	RE_RE_SOFT_VER_ROW, RE_RE_SOFT_VER_COL, 0);

	rtgui_form_set_item(sys_form, "发射端", 2, 0, 0);
	i2str(str, sys_temper[STI_SE_CPU_T]);
	rtgui_form_set_item(sys_form, str, 		RE_SE_CPU_TEMP_ROW, RE_SE_CPU_TEMP_COL, 0);
	i2str(str, sys_temper[STI_SE_CTB_T]);
	rtgui_form_set_item(sys_form, str, 		RE_SE_BOX_TEMP_ROW, RE_SE_BOX_TEMP_COL, 0);
	convert_ver2str(se_version, str);
	rtgui_form_set_item(sys_form, str, 	RE_SE_SOFT_VER_ROW, RE_SE_SOFT_VER_COL, 0);

	creat_devsn(str, sizeof(str), 0);
	rtgui_form_set_row(sys_form, str, TE_SN_ROW);

	creat_devsn(str, sizeof(str), 1);
	rtgui_form_set_row(sys_form, str, RE_SN_ROW);
#endif


	other_form = rtgui_form_create(other_form_head, OTHER_FORM_ROWS, OTHER_FORM_COLS, &body_rect);
	rtgui_notebook_add(zvd_notebook, "掉电", RTGUI_WIDGET(other_form));
#if 1
	rtgui_form_set_item(other_form, "次数", 1, 0, 0);
	rtgui_form_set_item(other_form, "n", 2, 0, 0);
	rtgui_form_set_item(other_form, "n-1", 3, 0, 0);
	rtgui_form_set_item(other_form, "n-2", 4, 0, 0);
	update_tx_poweroff_info_of_otherform();
	update_rx_poweroff_info_of_otherform();
#endif

	body_rect.x1 = FRAME_GAP;
	body_rect.y1 = body_rect.y2 + WIDGET_ROW_GAP;
	body_rect.x2 = FRAME_GAP + 8 * rt_strlen(RE_IF_SWITCH2CABLE_STR);
	body_rect.y2 = body_rect.y1 + 16;
	zvd_gui_debug(("fun:%s(), line:%d, (%d,%d) (%d,%d)!\n", __FUNCTION__, __LINE__,
				   body_rect.x1, body_rect.y1, body_rect.x2, body_rect.y2));

	switch_cable_label = rtgui_label_create(RE_IF_SWITCH2CABLE_STR);
	if (NULL != switch_cable_label) {
		RTGUI_WIDGET_TEXTALIGN(RTGUI_WIDGET(switch_cable_label)) =  RTGUI_ALIGN_CENTER_VERTICAL;
		//RTGUI_WIDGET_FOREGROUND(RTGUI_WIDGET(switch_cable_label)) = blue;
		rtgui_widget_set_rect(RTGUI_WIDGET(switch_cable_label), &body_rect);
		/* view是一个container控件，调用add_child方法添加这个label控件 */
		rtgui_container_add_child(RTGUI_CONTAINER(view), RTGUI_WIDGET(switch_cable_label));
	} else {
		printf_syn("creat cable label fail\n");
	}

#if 1
	body_rect.x1 = body_rect.x2 + 2;
	body_rect.x2 = body_rect.x1 + 16 + 8;
	zvd_gui_debug(("fun:%s(), line:%d, (%d,%d) (%d,%d)!\n", __FUNCTION__, __LINE__,
				   body_rect.x1, body_rect.y1, body_rect.x2, body_rect.y2));

	switch_cable_yn_textview = rtgui_textview_create(print_info_str[PIS_ID_FOU], &body_rect);
	if (NULL != switch_cable_yn_textview) {
		RTGUI_WIDGET_TEXTALIGN(RTGUI_WIDGET(switch_cable_yn_textview)) = RTGUI_ALIGN_CENTER_HORIZONTAL
				| RTGUI_ALIGN_CENTER_VERTICAL;
		//RTGUI_WIDGET_FOREGROUND(RTGUI_WIDGET(switch_cable_yn_textview)) = red;
		rtgui_container_add_child(RTGUI_CONTAINER(view), RTGUI_WIDGET(switch_cable_yn_textview));
	} else {
		printf_syn("creat cable yn textview fail\n");
	}
#endif

#if 1
	body_rect.x1 = body_rect.x2 + 2 + 2*8;
	body_rect.x2 = body_rect.x1 + 8*rt_strlen(TE_HAD_POWEROFF_STR);
	zvd_gui_debug(("fun:%s(), line:%d, (%d,%d) (%d,%d)!\n", __FUNCTION__, __LINE__,
				   body_rect.x1, body_rect.y1, body_rect.x2, body_rect.y2));

	tx_poweroff_label = rtgui_label_create(TE_HAD_POWEROFF_STR);
	if (NULL != tx_poweroff_label) {
		RTGUI_WIDGET_TEXTALIGN(RTGUI_WIDGET(tx_poweroff_label)) =  RTGUI_ALIGN_CENTER_VERTICAL;
		rtgui_widget_set_rect(RTGUI_WIDGET(tx_poweroff_label), &body_rect);
		rtgui_container_add_child(RTGUI_CONTAINER(view), RTGUI_WIDGET(tx_poweroff_label));
	} else {
		printf_syn("tx poweroff label fail\n");
	}

	body_rect.x1 = body_rect.x2 + 2;
	body_rect.x2 = body_rect.x1 + 16 + 8;
	zvd_gui_debug(("fun:%s(), line:%d, (%d,%d) (%d,%d)!\n", __FUNCTION__, __LINE__,
				   body_rect.x1, body_rect.y1, body_rect.x2, body_rect.y2));

	tx_poweroff_yn_textview = rtgui_textview_create(print_info_str[PIS_ID_FOU], &body_rect);
	if (NULL != tx_poweroff_yn_textview) {
		RTGUI_WIDGET_TEXTALIGN(RTGUI_WIDGET(tx_poweroff_yn_textview)) = RTGUI_ALIGN_CENTER_HORIZONTAL
				| RTGUI_ALIGN_CENTER_VERTICAL;
		rtgui_container_add_child(RTGUI_CONTAINER(view), RTGUI_WIDGET(tx_poweroff_yn_textview));
	} else {
		printf_syn("creat cable yn textview fail\n");
	}
#endif


#if 1
	body_rect.x1 = FRAME_GAP;
	body_rect.y1 = body_rect.y2 + WIDGET_ROW_GAP;
	body_rect.x2 = FRAME_GAP + (8+1) * rt_strlen(RE_BUZZER_DISABLE_STR);
	body_rect.y2 = body_rect.y1 + 16 + 2*WIDGET_ROW_GAP;
	zvd_gui_debug(("fun:%s(), line:%d, (%d,%d) (%d,%d)!\n", __FUNCTION__, __LINE__,
				   body_rect.x1, body_rect.y1, body_rect.x2, body_rect.y2));

	buzzer_button = rtgui_pushbutton_create(RE_BUZZER_DISABLE_STR);
	if (NULL != buzzer_button) {
		RTGUI_WIDGET_TEXTALIGN(RTGUI_WIDGET(buzzer_button)) = RTGUI_ALIGN_CENTER_HORIZONTAL
				| RTGUI_ALIGN_CENTER_VERTICAL;
		//RTGUI_WIDGET_FOREGROUND(RTGUI_WIDGET(buzzer_button)) = blue;
		rtgui_widget_set_rect(RTGUI_WIDGET(buzzer_button), &body_rect);
		rtgui_container_add_child(RTGUI_CONTAINER(view), RTGUI_WIDGET(buzzer_button));

		rtgui_button_set_onbutton(buzzer_button, buzzer_btn_onbutton);
	} else {
		printf_syn("creat buzzer pushbutton fail\n");
	}

	body_rect.x1 = body_rect.x2 + FRAME_GAP;
	/* body_rect.y1 = body_rect.y1; */ /* 与前一个按钮在同一行, 所以, y1, y2保持不变 */
	body_rect.x2 = body_rect.x1 + (8+1) * rt_strlen(SWITCH_TO_PT_STR);;
	/* body_rect.y2 = body_rect.y2; */ /* 与前一个按钮在同一行, 所以, y1, y2保持不变 */
	
	switch2pt_button = rtgui_pushbutton_create(SWITCH_TO_PT_STR);
	if (NULL != switch2pt_button) {
		RTGUI_WIDGET_TEXTALIGN(RTGUI_WIDGET(switch2pt_button)) = RTGUI_ALIGN_CENTER_HORIZONTAL
				| RTGUI_ALIGN_CENTER_VERTICAL;
		//RTGUI_WIDGET_FOREGROUND(RTGUI_WIDGET(switch2pt_button)) = blue;
		rtgui_widget_set_rect(RTGUI_WIDGET(switch2pt_button), &body_rect);
		rtgui_container_add_child(RTGUI_CONTAINER(view), RTGUI_WIDGET(switch2pt_button));
		rtgui_button_set_onbutton(switch2pt_button, switch2pt_btn_onbutton);
	} else {
		printf_syn("creat switch button fail\n");
	}

#if USE_OPTICX_200S_VERSION
	body_rect.x1 = body_rect.x2 + FRAME_GAP;
	/* body_rect.y1 = body_rect.y1; */ /* 与前一个按钮在同一行, 所以, y1, y2保持不变 */
	body_rect.x2 = body_rect.x1 + (8) * rt_strlen(RE_CUR_CHANNEL_NO_STR) + 2;
	/* body_rect.y2 = body_rect.y2; */ /* 与前一个按钮在同一行, 所以, y1, y2保持不变 */
	zvd_gui_debug(("fun:%s(), line:%d, (%d,%d) (%d,%d)!\n", __FUNCTION__, __LINE__,
				   body_rect.x1, body_rect.y1, body_rect.x2, body_rect.y2));
	rxe_cur_channel_label = rtgui_label_create(RE_CUR_CHANNEL_NO_STR);
	if (NULL != rxe_cur_channel_label) {
		RTGUI_WIDGET_TEXTALIGN(RTGUI_WIDGET(rxe_cur_channel_label)) =  RTGUI_ALIGN_CENTER_VERTICAL;
		rtgui_widget_set_rect(RTGUI_WIDGET(rxe_cur_channel_label), &body_rect);
		rtgui_container_add_child(RTGUI_CONTAINER(view), RTGUI_WIDGET(rxe_cur_channel_label));
	} else {
		printf_syn("rxe cru channel label fail\n");
	}

	body_rect.x1 = body_rect.x2 + 2;
	body_rect.x2 = body_rect.x1 + 8 + 8;
	zvd_gui_debug(("fun:%s(), line:%d, (%d,%d) (%d,%d)!\n", __FUNCTION__, __LINE__,
				   body_rect.x1, body_rect.y1, body_rect.x2, body_rect.y2));
	str[0] = convert_cur_channel_to_char();
	str[1] = '\0';
	rxe_cur_channel_no_textview = rtgui_textview_create(str, &body_rect);
	if (NULL != rxe_cur_channel_no_textview) {
		RTGUI_WIDGET_TEXTALIGN(RTGUI_WIDGET(rxe_cur_channel_no_textview)) = RTGUI_ALIGN_CENTER_HORIZONTAL
				| RTGUI_ALIGN_CENTER_VERTICAL;
		rtgui_container_add_child(RTGUI_CONTAINER(view), RTGUI_WIDGET(rxe_cur_channel_no_textview));
	} else {
		printf_syn("creat cable yn textview fail\n");
	}
#endif
#endif

#if 1==DISPLAY_OVERLOAD_INFO
	body_rect.x1 = body_rect.x2 + 2 + 7*8;
	body_rect.x2 = body_rect.x1 + 8*rt_strlen(RE_OVERLOAD_CNT_STR);
	zvd_gui_debug(("fun:%s(), line:%d, (%d,%d) (%d,%d)!\n", __FUNCTION__, __LINE__,
				   body_rect.x1, body_rect.y1, body_rect.x2, body_rect.y2));

	rx_overload_label = rtgui_label_create(RE_OVERLOAD_CNT_STR);
	if (NULL != rx_overload_label) {
		RTGUI_WIDGET_TEXTALIGN(RTGUI_WIDGET(rx_overload_label)) =  RTGUI_ALIGN_CENTER_VERTICAL;
		rtgui_widget_set_rect(RTGUI_WIDGET(rx_overload_label), &body_rect);
		rtgui_container_add_child(RTGUI_CONTAINER(view), RTGUI_WIDGET(rx_overload_label));
	} else {
		printf_syn("tx poweroff label fail\n");
	}

	body_rect.x1 = body_rect.x2 + 2;
	body_rect.x2 = body_rect.x1 + 16 + 8;
	zvd_gui_debug(("fun:%s(), line:%d, (%d,%d) (%d,%d)!\n", __FUNCTION__, __LINE__,
				   body_rect.x1, body_rect.y1, body_rect.x2, body_rect.y2));

	rx_overload_cnt_textview = rtgui_textview_create("0", &body_rect);
	if (NULL != rx_overload_cnt_textview) {
		RTGUI_WIDGET_TEXTALIGN(RTGUI_WIDGET(rx_overload_cnt_textview)) = RTGUI_ALIGN_CENTER_HORIZONTAL | RTGUI_ALIGN_CENTER_VERTICAL;
		rtgui_container_add_child(RTGUI_CONTAINER(view), RTGUI_WIDGET(rx_overload_cnt_textview));
	} else {
		printf_syn("creat cable yn textview fail\n");
	}
#endif



#if 1
	/* 创建时间标签, 2013-03-18 09:20 */
	rtc_dev = rt_device_find("rtc");
	if (NULL != rtc_dev) {
		time_t time;
		rt_device_control(rtc_dev, RT_DEVICE_CTRL_RTC_GET_TIME, &time);
		ptime = localtime(&time);
		rt_sprintf(sys_time_str, "%4d-%02d-%02d %02d:%02d:%02d", (ptime->tm_year+1900), (ptime->tm_mon+1),
				   ptime->tm_mday,	ptime->tm_hour, ptime->tm_min, ptime->tm_sec);

		body_rect.x2 = rtgui_graphic_driver_get_default()->width;
		body_rect.y2 = rtgui_graphic_driver_get_default()->height;
		body_rect.x1 = body_rect.x2 - (8+1) * 19;
		body_rect.y1 = body_rect.y2 - 20;
#if SYS_TIME_USE_TEXTBOX
		sys_time_textbox = rtgui_textbox_create(sys_time_str, RTGUI_TEXTBOX_SINGLE | RTGUI_TEXTBOX_CARET_HIDE);
		if (NULL != sys_time_textbox) {
			RTGUI_WIDGET_TEXTALIGN(RTGUI_WIDGET(sys_time_textbox)) = RTGUI_ALIGN_CENTER_HORIZONTAL | RTGUI_ALIGN_CENTER_VERTICAL;
			//RTGUI_WIDGET_FOREGROUND(RTGUI_WIDGET(sys_time_textbox)) = blue;
			RTGUI_WIDGET_BACKGROUND(RTGUI_WIDGET(sys_time_textbox)) = blue; //default_background;
			//RTGUI_WIDGET_BACKGROUND(RTGUI_WIDGET(view));
			rtgui_widget_set_rect(RTGUI_WIDGET(sys_time_textbox), &body_rect);
			rtgui_container_add_child(RTGUI_CONTAINER(view), RTGUI_WIDGET(sys_time_textbox));

		} else {
			printf_syn("creat sys time textbox fail\n");
		}
#else
		sys_time_textview = rtgui_textview_create(sys_time_str, &body_rect);
		if (NULL != sys_time_textview) {
			RTGUI_WIDGET_TEXTALIGN(RTGUI_WIDGET(sys_time_textview)) = RTGUI_ALIGN_CENTER_HORIZONTAL | RTGUI_ALIGN_CENTER_VERTICAL;
			//RTGUI_WIDGET_FOREGROUND(RTGUI_WIDGET(sys_time_textbox)) = blue;
			rtgui_container_add_child(RTGUI_CONTAINER(view), RTGUI_WIDGET(sys_time_textview));
		} else {
			printf_syn("creat sys time textview fail\n");
		}
#endif

	} else {
		printf_syn("find rtc device fail\n");
	}

#endif
	rtgui_view_show(view, RT_FALSE); /* 显示视图 */

	return ret;
}

void app_view_clean(void)
{
	if (NULL != se_form) {
		rtgui_form_destroy(se_form);
		se_form = NULL;
	}

	if (NULL != re_form) {
		rtgui_form_destroy(re_form);
		re_form = NULL;
	}

	if (NULL != sys_form) {
		rtgui_form_destroy(sys_form);
		sys_form = NULL;
	}

	return;
}


int get_sys_time4label(char *timestr)
{
	time_t time;
	struct tm *ptime;

	if (NULL == rtc_dev)
		return FAIL;

	rt_device_control(rtc_dev, RT_DEVICE_CTRL_RTC_GET_TIME, &time);
	ptime = localtime(&time);
	rt_sprintf(timestr, "%4d-%02d-%02d %02d:%02d:%02d", (ptime->tm_year+1900), (ptime->tm_mon+1),
			   ptime->tm_mday,	ptime->tm_hour, ptime->tm_min, ptime->tm_sec);

	return SUCC;
}


static void set_form_px_col(rtgui_form_t *form)
{
	if (NULL == form)
		return;

	rtgui_form_set_item(form, "A", 1, 0, 0);
	rtgui_form_set_item(form, "B", 2, 0, 0);
	rtgui_form_set_item(form, "C", 3, 0, 0);

	return;
}


/* 触发文件列表视图的按钮回调函数 */
static void buzzer_btn_onbutton(rtgui_widget_t* widget, struct rtgui_event* event)
{
	zvd_gui_debug(("fun:%s(), line:%d, type:%d, flag:0x%x!\n", __FUNCTION__, __LINE__, event->type,
				   buzzer_button->flag));

	if (buzzer_button->flag & RTGUI_BUTTON_FLAG_PRESS) {
		disable_buzzer = 1;
	} else {
		disable_buzzer = 0;
	}
}


static void switch2pt_btn_onbutton(rtgui_widget_t* widget, struct rtgui_event* event)
{
	zvd_gui_debug(("fun:%s(), line:%d, type:%d, flag:0x%x!\n", __FUNCTION__, __LINE__, event->type,
				   switch2pt_button->flag));

	if (switch2pt_button->flag & RTGUI_BUTTON_FLAG_PRESS) {
		creat_switch2pt_win(widget);
	} else {
		send_cmd_to_rxe(0, SWITCH_FROM_PT); /* 取消"切换到pt", 也就是"切换到零压降的输出端" */
	}

}


static void creat_switch2pt_win(rtgui_widget_t* widget)
{
	rtgui_label_t *label;
	rtgui_button_t *cancel_button;
	rtgui_button_t *sure_button;
	rtgui_toplevel_t *parent;
	rtgui_rect_t rect = {60, 45, 260, 160};

	parent = RTGUI_TOPLEVEL(rtgui_widget_get_toplevel(widget));

	switch2pt_confirm_win = rtgui_win_create(parent, "警告", &rect, RTGUI_WIN_STYLE_MODAL);

	rtgui_widget_get_extent(RTGUI_WIDGET(switch2pt_confirm_win), &rect);
	rect.x1 += 30;
	rect.x2 -= 30;
	rect.y1 += 20;
	rect.y2 = rect.y1 + 30;
	label = rtgui_label_create("确定切换到PT侧?");
	if (NULL != label) {
		rtgui_widget_set_rect(RTGUI_WIDGET(label), &rect);
		rtgui_container_add_child(RTGUI_CONTAINER(switch2pt_confirm_win), RTGUI_WIDGET(label));			
	}

	rtgui_widget_get_extent(RTGUI_WIDGET(switch2pt_confirm_win),&rect);
	rect.x1 += 20;
	rect.x2 -= 120;
	rect.y1 += 70;
	rect.y2 -= 20;
	sure_button = rtgui_button_create("确定");
	if (NULL != sure_button) {
		RTGUI_WIDGET_TEXTALIGN(RTGUI_WIDGET(sure_button)) = RTGUI_ALIGN_CENTER_HORIZONTAL | RTGUI_ALIGN_CENTER_VERTICAL;
		rtgui_widget_set_rect(RTGUI_WIDGET(sure_button), &rect);
		rtgui_container_add_child(RTGUI_CONTAINER(switch2pt_confirm_win), RTGUI_WIDGET(sure_button));
		rtgui_button_set_onbutton(sure_button, sure_btn_onbutton);
	} else {
		printf_syn("creat sure button fail\n");
	}
	
	rtgui_widget_get_extent(RTGUI_WIDGET(switch2pt_confirm_win),&rect);
	rect.x1 += 120;
	rect.x2 -= 20;
	rect.y1 += 70;
	rect.y2 -= 20;
	cancel_button = rtgui_button_create("取消");
	if (NULL != cancel_button) {
		RTGUI_WIDGET_TEXTALIGN(RTGUI_WIDGET(cancel_button)) = RTGUI_ALIGN_CENTER_HORIZONTAL | RTGUI_ALIGN_CENTER_VERTICAL;
		rtgui_widget_set_rect(RTGUI_WIDGET(cancel_button), &rect);
		rtgui_container_add_child(RTGUI_CONTAINER(switch2pt_confirm_win), RTGUI_WIDGET(cancel_button));
		rtgui_button_set_onbutton(cancel_button, cancel_btn_onbutton);
	} else {
		printf_syn("creat cancel button fail\n");
	}
	
	/* 模态显示窗口 */
	rtgui_win_show(switch2pt_confirm_win, RT_TRUE);

	/* 采用模态显示窗口，关闭时不会自行删除窗口，需要主动删除窗口 */
	rtgui_win_destroy(switch2pt_confirm_win);
	switch2pt_confirm_win = NULL;

	return;
}

static void cancel_btn_onbutton(rtgui_widget_t* widget, struct rtgui_event* event)
{
	if (NULL != switch2pt_confirm_win) {
		printf_syn("fun:%s(), switch2pt_win pointer cancel button\n", __FUNCTION__);
		rtgui_win_destroy(switch2pt_confirm_win);
	} else {
		printf_syn("fun:%s(), switch2pt_win pointer is NULL\n", __FUNCTION__);
	}

	return;
}

static void sure_btn_onbutton(rtgui_widget_t* widget, struct rtgui_event* event)
{
	if (NULL != switch2pt_confirm_win) {
		send_cmd_to_rxe(1, SWITCH2PT);
		printf_syn("fun:%s(), switch2pt_win pointer confirm button\n", __FUNCTION__);
		rtgui_win_destroy(switch2pt_confirm_win);
	} else {
		printf_syn("fun:%s(), switch2pt_win pointer is NULL\n", __FUNCTION__);
	}

	return;
}

/*
 * yyyymmdd.hhmm
 *
 * !!NOTE:len(str) >= 14
 */
int get_timestr(rt_uint32_t time, char *str, int len)
{
	struct tm *ptime;

	if (len < 14)
		return FAIL;

	if (0 != time) {
		ptime = localtime(&time);
		rt_sprintf(str, "%4d%02d%02d.%02d%02d", (ptime->tm_year+1900), (ptime->tm_mon+1),
				   ptime->tm_mday,	ptime->tm_hour, ptime->tm_min);
	} else {
		rt_strncpy(str, NO_POWEROFF_TIME, len);
	}

	return SUCC;
}

int get_tx_last3poweroff_time(rt_uint32_t *ntime, rt_uint32_t *n_1_time, rt_uint32_t *n_2_time)
{
	if (NULL==ntime || NULL==n_1_time || NULL==n_2_time)
		return FAIL;

	switch (poweroff_info_data.poi_tx1_poweroff_cnt % 3) {
	case 1:
		*ntime 	  = poweroff_info_data.poi_tx1_poweroff_t1;
		*n_1_time = poweroff_info_data.poi_tx1_poweroff_t0;
		*n_2_time = poweroff_info_data.poi_tx1_poweroff_t2;
		break;

	case 2:
		*ntime 	  = poweroff_info_data.poi_tx1_poweroff_t2;
		*n_1_time = poweroff_info_data.poi_tx1_poweroff_t1;
		*n_2_time = poweroff_info_data.poi_tx1_poweroff_t0;
		break;

	case 0:
		*ntime 	  = poweroff_info_data.poi_tx1_poweroff_t0;
		*n_1_time = poweroff_info_data.poi_tx1_poweroff_t2;
		*n_2_time = poweroff_info_data.poi_tx1_poweroff_t1;
		break;

	default:
		break;
	}

	return SUCC;
}

int get_tx2_last3poweroff_time(rt_uint32_t *ntime, rt_uint32_t *n_1_time, rt_uint32_t *n_2_time)
{
	if (NULL==ntime || NULL==n_1_time || NULL==n_2_time)
		return FAIL;

	switch (poweroff_info_data.poi_tx2_poweroff_cnt % 3) {
	case 1:
		*ntime 	  = poweroff_info_data.poi_tx2_poweroff_t1;
		*n_1_time = poweroff_info_data.poi_tx2_poweroff_t0;
		*n_2_time = poweroff_info_data.poi_tx2_poweroff_t2;
		break;

	case 2:
		*ntime 	  = poweroff_info_data.poi_tx2_poweroff_t2;
		*n_1_time = poweroff_info_data.poi_tx2_poweroff_t1;
		*n_2_time = poweroff_info_data.poi_tx2_poweroff_t0;
		break;

	case 0:
		*ntime 	  = poweroff_info_data.poi_tx2_poweroff_t0;
		*n_1_time = poweroff_info_data.poi_tx2_poweroff_t2;
		*n_2_time = poweroff_info_data.poi_tx2_poweroff_t1;
		break;

	default:
		break;
	}

	return SUCC;
}

int get_rx_last3poweroff_time(rt_uint32_t *ntime, rt_uint32_t *n_1_time, rt_uint32_t *n_2_time)
{
	if (NULL==ntime || NULL==n_1_time || NULL==n_2_time)
		return FAIL;

	switch (poweroff_info_data.poi_rx_poweroff_cnt % 3) {
	case 1:
		*ntime 	  = poweroff_info_data.poi_rx_poweroff_t1;
		*n_1_time = poweroff_info_data.poi_rx_poweroff_t0;
		*n_2_time = poweroff_info_data.poi_rx_poweroff_t2;
		break;

	case 2:
		*ntime 	  = poweroff_info_data.poi_rx_poweroff_t2;
		*n_1_time = poweroff_info_data.poi_rx_poweroff_t1;
		*n_2_time = poweroff_info_data.poi_rx_poweroff_t0;
		break;

	case 0:
		*ntime 	  = poweroff_info_data.poi_rx_poweroff_t0;
		*n_1_time = poweroff_info_data.poi_rx_poweroff_t2;
		*n_2_time = poweroff_info_data.poi_rx_poweroff_t1;
		break;

	default:
		break;
	}

	return SUCC;
}

int update_tx_poweroff_info_of_otherform(void)
{
	char str[16];
	rt_uint32_t ntime, n_1_time, n_2_time;

	if (SUC_FIBER_CHANNEL_2 == get_sys_use_channel()) {
		i2str(str, poweroff_info_data.poi_tx2_poweroff_cnt);
		get_tx2_last3poweroff_time(&ntime, &n_1_time, &n_2_time);
	} else {
		i2str(str, poweroff_info_data.poi_tx1_poweroff_cnt);
		get_tx_last3poweroff_time(&ntime, &n_1_time, &n_2_time);
	}

	rtgui_form_set_item(other_form, str, OT_SE_POWEROFF_CNT_ROW, OT_SE_POWEROFF_CNT_COL, 0);

	get_timestr(ntime, str, sizeof(str));
	rtgui_form_set_item(other_form, str, OT_SE_N_ROW, OT_SE_N_COL, 0);
	get_timestr(n_1_time, str, sizeof(str));
	rtgui_form_set_item(other_form, str, OT_SE_N_1_ROW, OT_SE_N_1_COL, 0);
	get_timestr(n_2_time, str, sizeof(str));
	rtgui_form_set_item(other_form, str, OT_SE_N_2_ROW, OT_SE_N_2_COL, 0);

	return SUCC;
}

int update_rx_poweroff_info_of_otherform(void)
{
	char str[16];
	rt_uint32_t ntime, n_1_time, n_2_time;

	i2str(str, poweroff_info_data.poi_rx_poweroff_cnt);
	rtgui_form_set_item(other_form, str, OT_RE_POWEROFF_CNT_ROW, OT_RE_POWEROFF_CNT_COL, 0);

	get_rx_last3poweroff_time(&ntime, &n_1_time, &n_2_time);

	get_timestr(ntime, str, sizeof(str));
	rtgui_form_set_item(other_form, str, OT_RE_N_ROW, OT_RE_N_COL, 0);
	get_timestr(n_1_time, str, sizeof(str));
	rtgui_form_set_item(other_form, str, OT_RE_N_1_ROW, OT_RE_N_1_COL, 0);
	get_timestr(n_2_time, str, sizeof(str));
	rtgui_form_set_item(other_form, str, OT_RE_N_2_ROW, OT_RE_N_2_COL, 0);

	return SUCC;
}

/*
 * tx_rx_e:
 *	0 - tx
 *	1 - rx
 */
int creat_devsn(char *str, int len, int tx_rx_e)
{
	char *pchs, *pchd;

	if (NULL==str || len<48) {
		printf_syn("%s() param error\n", __FUNCTION__);
		return FAIL;
	}

	pchd = str;

	switch (tx_rx_e) {
	case 0:
		pchs = TE_SN_STR;
		while ('\0' != *pchs)
			*pchd++ = *pchs++;
		rt_strncpy(pchd, (char *)tx_dev_sn, len-(pchd - str));
		break;

	case 1:
		pchs = RE_SN_STR;
		while ('\0' != *pchs)
			*pchd++ = *pchs++;
		get_devsn(pchd, len-(pchd - str));
		break;

	default:
		printf_syn("%s() tx_rx_e error\n", __FUNCTION__);
		return FAIL;
	}

	return SUCC;
}

int creat_sn_not_match_win(struct rtgui_widget* widget)
{
	rtgui_rect_t rect = {60, 45, 260, 160};
	rtgui_label_t *label1;
	rtgui_label_t *label2;
	rtgui_toplevel_t *parent;

	if (RT_NULL != sn_not_match_win)
		return FAIL;

	parent = RTGUI_TOPLEVEL(rtgui_widget_get_toplevel(widget));

	sn_not_match_win = rtgui_win_create(parent,
		"警告", &rect, RTGUI_WIN_STYLE_MODAL);

	rtgui_widget_get_extent(RTGUI_WIDGET(sn_not_match_win),&rect);
	rect.x1 += 20;
	rect.x2 -= 20;
	rect.y1 += 20;
	rect.y2 = rect.y1 + 30;
	label1 = rtgui_label_create("发射端与接收端的SN号");
	if (NULL != label1) {
		rtgui_widget_set_rect(RTGUI_WIDGET(label1), &rect);
		rtgui_container_add_child(RTGUI_CONTAINER(sn_not_match_win), RTGUI_WIDGET(label1));
	}

	rtgui_widget_get_extent(RTGUI_WIDGET(sn_not_match_win),&rect);
	rect.x1 += 70;
	rect.x2 -= 30;
	rect.y1 += 50;
	rect.y2 = rect.y1 + 30;
	label2 = rtgui_label_create("不匹配!");
	if (NULL != label2) {
		rtgui_widget_set_rect(RTGUI_WIDGET(label2), &rect);
		rtgui_container_add_child(RTGUI_CONTAINER(sn_not_match_win), RTGUI_WIDGET(label2));
	}

	send_cmd_to_rxe(1, SWITCH2PT);

	rtgui_win_show(sn_not_match_win, RT_TRUE);

	rtgui_win_destroy(sn_not_match_win);
	sn_not_match_win = RT_NULL;

	return SUCC;
}

int creat_software_ver_not_match_win(struct rtgui_widget* widget)
{
	rtgui_rect_t rect = {60, 45, 260, 160};
	rtgui_label_t *label1;
	rtgui_label_t *label2;
	rtgui_toplevel_t *parent;

	if (RT_NULL != software_version_not_match_win)
		return FAIL;

	parent = RTGUI_TOPLEVEL(rtgui_widget_get_toplevel(widget));

	software_version_not_match_win = rtgui_win_create(parent,
		"致命错误", &rect, RTGUI_WIN_STYLE_MODAL);

	rtgui_widget_get_extent(RTGUI_WIDGET(software_version_not_match_win),&rect);
	rect.x1 += 20;
	rect.x2 -= 20;
	rect.y1 += 20;
	rect.y2 = rect.y1 + 30;
	label1 = rtgui_label_create("接收端与lcd软件版本");
	if (NULL != label1) {
		rtgui_widget_set_rect(RTGUI_WIDGET(label1), &rect);
		rtgui_container_add_child(RTGUI_CONTAINER(software_version_not_match_win), RTGUI_WIDGET(label1));
	}

	rtgui_widget_get_extent(RTGUI_WIDGET(software_version_not_match_win),&rect);
	rect.x1 += 70;
	rect.x2 -= 30;
	rect.y1 += 50;
	rect.y2 = rect.y1 + 30;
	label2 = rtgui_label_create("不匹配!");
	if (NULL != label2) {
		rtgui_widget_set_rect(RTGUI_WIDGET(label2), &rect);
		rtgui_container_add_child(RTGUI_CONTAINER(software_version_not_match_win), RTGUI_WIDGET(label2));
	}

	send_cmd_to_rxe(1, SWITCH2PT);

	rtgui_win_show(software_version_not_match_win, RT_TRUE);

	rtgui_win_destroy(software_version_not_match_win);
	software_version_not_match_win = RT_NULL;

	return SUCC;
}
