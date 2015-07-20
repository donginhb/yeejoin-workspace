#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

enum key_no {
    KEY_NO_NUM0  = 0,
    KEY_NO_NUM1  = 1,
    KEY_NO_NUM2  = 2,
    KEY_NO_NUM3  = 3,
    KEY_NO_NUM4  = 4,
    KEY_NO_NUM5  = 5,
    KEY_NO_NUM6  = 6,
    KEY_NO_NUM7  = 7,
    KEY_NO_NUM8  = 8,
    KEY_NO_NUM9  = 9,

    KEY_NO_ENTER = 10,
    KEY_NO_MENU  = 11,
    KEY_NO_ESC   = 12,
    KEY_NO_UP    = 13,
    KEY_NO_DOWN  = 14,

    KEY_NO_MODIFY	= 15,
    KEY_NO_DOT		= 16, /* char '.' */
    KEY_NO_STAR		= 17, /* char '*' */
    KEY_NO_LINE		= 18, /* char '-' */
};

#define PWD_LEN 16


extern char input_id[PWD_LEN];

extern void keyboard_init(void);
extern void thread_keyboard_entry(void* parameter);


#if 1
#define LOGO_SIZE  (104)

#define WORK_ZONE_START_X (LOGO_SIZE+106)
#define WORK_ZONE_START_Y (LOGO_SIZE)

#define TITLE_GAP (5)
#define CIRCLE_Y_OFFSET (LARGE_CN_FONT_HEIGHT/2+TITLE_GAP)

#define BOTTOM_LINE_RIGHT_GAP (100)
#define BOTTOM_LINE_DOWN_GAP (8)
#define BOTTOM_LINE_START_X (X_POSITION_MAX+1 - 9*24 - BOTTOM_LINE_RIGHT_GAP)
#define BOTTOM_LINE_START_Y (Y_POSITION_MAX+1 - BOTTOM_LINE_DOWN_GAP - MIDDLE_CN_FONT_HEIGHT)

#define BODY_START_X (WORK_ZONE_START_X)
#define BODY_END_X   (X_POSITION_MAX - 10)

#define BODY_START_Y (WORK_ZONE_START_Y+2*CIRCLE_Y_OFFSET)
#define BODY_END_Y   (BOTTOM_LINE_START_Y - 10)

#define LINE_GAP (3)
#define INDENT1_SIZE (32)

/* 提示语位置 */
#define USRID_POS_OFFSET_X (3*16+5)
#define USRID_POS_OFFSET_Y (2*MIDDLE_CN_FONT_HEIGHT)
#define PW_POS_OFFSET_X (USRID_POS_OFFSET_X)
#define PW_POS_OFFSET_Y (USRID_POS_OFFSET_Y)

/* ID回显位置 */
#define USRID_START_X (BODY_START_X+USRID_POS_OFFSET_X+24*3+16)
#define USRID_START_Y (BODY_START_Y+USRID_POS_OFFSET_Y)

/* pw回显位置 */
#define PW_START_X (USRID_START_X)
#define PW_START_Y (USRID_START_Y+24+LINE_GAP)

#define USRID_ARROWHEAD_X (BODY_START_X)
#define USRID_ARROWHEAD_Y (BODY_START_Y+USRID_POS_OFFSET_Y)

#define PW_ARROWHEAD_X (BODY_START_X)
#define PW_ARROWHEAD_Y (USRID_ARROWHEAD_Y+MIDDLE_CN_FONT_HEIGHT)

#define LOCK_OPEN_ARROWHEAD_X (USRID_ARROWHEAD_X)
#define LOCK_OPEN_ARROWHEAD_Y (USRID_ARROWHEAD_Y)
#define LOCK_CLOSE_ARROWHEAD_X (PW_ARROWHEAD_X)
#define LOCK_CLOSE_ARROWHEAD_Y (PW_ARROWHEAD_Y)

/* read zone, 电力宣传lcd区域 */
#define READ_MODE_READ_WRITE_GAP (10)

#define READ_MODE_ZONE_START_X (READ_MODE_READ_WRITE_GAP)
#if 0
#define READ_MODE_ZONE_START_Y (WORK_ZONE_START_Y)
#else
#define READ_MODE_ZONE_START_Y (BODY_START_Y)
#endif
#define READ_MODE_ZONE_END_X (X_POSITION_MAX-READ_MODE_READ_WRITE_GAP)
#define READ_MODE_ZONE_END_Y (BODY_END_Y)

#endif

#if 1
#include <lcd_touch.h>
#include <stm32f10x.h>


enum auth_style_e {
	AUTH_STYLE_INVALID		= 0,
	AUTH_STYLE_ELOCK_CTRL		= 1,
	AUTH_STYLE_QUERY_PCOMSUMP,
};
extern int auth_style;

struct tm2lcd_mb_st {
	struct lcd_mb_st textmenu_msg;
	u16 buf_pool[28];
} textmenu2lcd_mb;

extern void draw_title_line_bg(void);
extern void dis_title_line(struct tm2lcd_mb_st *textmenu2lcd_mb_p, int len);
extern void dis_bottom_line(struct tm2lcd_mb_st *textmenu2lcd_mb_p, int len);
extern void clr_body_zone(void);
extern void dis_body_content_level1(void);
extern void dis_ctr_elock_login(void);
extern void dis_query_pconsump_login(void);
extern void dis_ctr_elock_interface(void);
extern void dis_main_interface(void);
extern void dis_one_char(int char_id, int x, int y);
extern void dis_arrowhead(int x, int y);
extern void clr_arrowhead(int x, int y);
extern void dis_top_frame(void);
extern void return_main_interface(void);

extern void clr_usrid_zone(void);
extern void clr_pw_zone(void);

#endif



#endif
