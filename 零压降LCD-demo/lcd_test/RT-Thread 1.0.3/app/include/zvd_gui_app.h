#ifndef ZVD_GUI_APP_H__
#define ZVD_GUI_APP_H__

#include <rtgui/rtgui.h>
#include <rtgui/widgets/view.h>
#include <rtgui/widgets/workbench.h>
#include <rtgui/widgets/notebook.h>
#include <rtgui/widgets/label.h>
#include <rtgui/widgets/textview.h>

#define SYS_TIME_USE_TEXTBOX 1
#define DISPLAY_OVERLOAD_INFO 0

#define TITLE_PANEL_HEIGHT 25
#define STATE_PANEL_HEIGHT (25 + 3*16)
#define WIDGET_ROW_GAP (4)
#define FRAME_GAP (10)

#if USE_OPTICX_200S_VERSION
#define DEVICE_NAME	"OpticX-200S高精度光纤数字转换系统"
#else
#define DEVICE_NAME	"OpticX-200高精度光纤数字转换系统"
#endif
 
#if LCD_TEST_MALOOEI
#define MALOOEI_TITLE "Graphic Template----malooei "
#endif

#define RE_BUZZER_DISABLE_STR 	"禁止蜂鸣器"
#define RE_IF_SWITCH2CABLE_STR 	"已切换至电缆:"
#define TE_SN_STR 		"发射端SN:"
#define RE_SN_STR 		"接收端SN:"
#define TE_HAD_POWEROFF_STR 	"发射端已掉电:"
#define RE_OVERLOAD_CNT_STR 	"过流次数:"
#define SWITCH_TO_PT_STR        "切换到PT"
#define RE_CUR_CHANNEL_NO_STR	"当前通道:"

#define NO_POWEROFF_TIME	"--"

#define RE_FORM_ROWS 5
#define RE_FORM_COLS 3

#define SE_FORM_ROWS 6
#define SE_FORM_COLS 3

#define SYS_FORM_ROWS 5
#define SYS_FORM_COLS 3

#define OTHER_FORM_ROWS 5
#define OTHER_FORM_COLS 3

enum app_workbench_cmd_id {
	SE_FORM_UPDATE = 1,
	RE_FORM_UPDATE,
	SYS_FORM_UPDATE,
	OTHER_FORM_UPDATE,

	SN_NOT_MATCH,
	SOFTWARE_VER_NOT_MATCH,
	SWITCH_2_PT,
	UPDATE_SYS_TIME,
	UPDATE_SW_CABLE,
	UPDATE_OVERLOAD_CNT,
	UPDATE_HAD_POWEROFF,
	UPDATE_CUR_CHANNEL_NO,

	MOUSE_BUTTON_EVENT,
};

/* 发射端表格行列号宏定义 */
#define SE_PA_OVER_RANGE_ROW 1
#define SE_PA_OVER_RANGE_COL 1
#define SE_PA_POWER_DOWN_ROW 1
#define SE_PA_POWER_DOWN_COL 2

#define SE_PB_OVER_RANGE_ROW 2
#define SE_PB_OVER_RANGE_COL 1
#define SE_PB_POWER_DOWN_ROW 2
#define SE_PB_POWER_DOWN_COL 2

#define SE_PC_OVER_RANGE_ROW 3
#define SE_PC_OVER_RANGE_COL 1
#define SE_PC_POWER_DOWN_ROW 3
#define SE_PC_POWER_DOWN_COL 2


/* 接收端表格行列号宏定义 */
#define RE_PA_AVG_VALUE_ROW 1
#define RE_PA_AVG_VALUE_COL 1
#define RE_PA_OVER_RANGE_ROW 1
#define RE_PA_OVER_RANGE_COL 2

#define RE_PB_AVG_VALUE_ROW 2
#define RE_PB_AVG_VALUE_COL 1
#define RE_PB_OVER_RANGE_ROW 2
#define RE_PB_OVER_RANGE_COL 2

#define RE_PC_AVG_VALUE_ROW 3
#define RE_PC_AVG_VALUE_COL 1
#define RE_PC_OVER_RANGE_ROW 3
#define RE_PC_OVER_RANGE_COL 2

/* 系统表格行列号宏定义 */
#define RE_RE_CPU_TEMP_ROW 1
#define RE_RE_CPU_TEMP_COL 1
#define RE_RE_BOX_TEMP_ROW 1
#define RE_RE_BOX_TEMP_COL 2
#define RE_RE_SOFT_VER_ROW 1
#define RE_RE_SOFT_VER_COL 3

#define RE_SE_CPU_TEMP_ROW 2
#define RE_SE_CPU_TEMP_COL 1
#define RE_SE_BOX_TEMP_ROW 2
#define RE_SE_BOX_TEMP_COL 2
#define RE_SE_SOFT_VER_ROW 2
#define RE_SE_SOFT_VER_COL 3

//#define RE_BUZZER_STATE_ROW 	4
//#define RE_IF_SWITCH2CABLE_ROW 	4
#define TE_SN_ROW 				4
#define RE_SN_ROW 				5


/* other表格行列号宏定义 */
#define OT_SE_POWEROFF_CNT_ROW	1
#define OT_SE_POWEROFF_CNT_COL	1
#define OT_SE_N_ROW				2
#define OT_SE_N_COL				1
#define OT_SE_N_1_ROW			3
#define OT_SE_N_1_COL			1
#define OT_SE_N_2_ROW			4
#define OT_SE_N_2_COL			1

#define OT_RE_POWEROFF_CNT_ROW	1
#define OT_RE_POWEROFF_CNT_COL	2
#define OT_RE_N_ROW				2
#define OT_RE_N_COL				2
#define OT_RE_N_1_ROW			3
#define OT_RE_N_1_COL			2
#define OT_RE_N_2_ROW			4
#define OT_RE_N_2_COL			2


extern rtgui_notebook_t *zvd_notebook;
extern rtgui_textview_t *switch_cable_yn_textview;

extern rtgui_textview_t *tx_poweroff_yn_textview;
#if 1==DISPLAY_OVERLOAD_INFO
extern rtgui_textview_t *rx_overload_cnt_textview;
#endif
#if USE_OPTICX_200S_VERSION
extern rtgui_textview_t *rxe_cur_channel_no_textview;
#endif

/* 是否禁止蜂鸣器工作 */
extern volatile int disable_buzzer;
/* 蜂鸣器是否需要鸣叫 */
extern volatile int buzzer_tweet;
extern char sys_time_str[20];
extern rt_device_t rtc_dev;

#if SYS_TIME_USE_TEXTBOX
extern struct rtgui_textbox *sys_time_textbox;
#else
extern rtgui_textview_t *sys_time_textview;
#endif

extern rt_thread_t wb_thread;

extern rtgui_win_t *sn_not_match_win;
extern rtgui_win_t *software_version_not_match_win;


extern void rtgui_startup(void);
extern void app_view_get_can_use_rect(rtgui_view_t* view, rtgui_rect_t *rect);
//extern void app_view_get_logic_rect(rtgui_view_t* view, rtgui_rect_t *rect);

extern int app_view_structure(struct rtgui_workbench* workbench, struct rtgui_view* view);
extern int app_view_ml_structure(struct rtgui_workbench* workbench, struct rtgui_view* view);

extern void app_view_clean(void);

extern int get_sys_time4label (char *timestr);
extern int get_timestr(rt_uint32_t time, char *str, int len);
extern int get_tx_last3poweroff_time(rt_uint32_t *ntime, rt_uint32_t *n_1_time, rt_uint32_t *n_2_time);
extern int get_tx2_last3poweroff_time(rt_uint32_t *ntime, rt_uint32_t *n_1_time, rt_uint32_t *n_2_time);
extern int get_rx_last3poweroff_time(rt_uint32_t *ntime, rt_uint32_t *n_1_time, rt_uint32_t *n_2_time);
extern int update_tx_poweroff_info_of_otherform(void);
extern int update_rx_poweroff_info_of_otherform(void);

extern int creat_sn_not_match_win(struct rtgui_widget* widget);
extern int creat_software_ver_not_match_win(struct rtgui_widget* widget);

#endif

