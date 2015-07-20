#ifndef LCD_TOUCH_H__
#define LCD_TOUCH_H__

#include <rtdef.h>
#include <stm32f10x.h>



#define LARGE_CN_FONT_WIDTH   (32)
#define LARGE_CN_FONT_HEIGHT  (40)
#define MIDDLE_CN_FONT_WIDTH  (24)
#define MIDDLE_CN_FONT_HEIGHT (24)
#define SMALL_CN_FONT_WIDTH   (16)
#define SMALL_CN_FONT_HEIGHT  (16)

#define LARGE_EN_FONT_WIDTH   ()
#define LARGE_EN_FONT_HEIGHT  ()
#define MIDDLE_EN_FONT_WIDTH  (16)
#define MIDDLE_EN_FONT_HEIGHT (24)
#define SMALL_EN_FONT_WIDTH   (8)
#define SMALL_EN_FONT_HEIGHT  (16)


struct lcd_ipc_mb {
    struct rt_mailbox _mb;
    struct lcd_mb_st *pool[10];
};

enum lcd_mb_cmd_e {
	LCD_CMD_DIS_CHAR	= 0,
	LCD_CMD_CLR_RECTANGLE,
	LCD_CMD_DRAW_RECTANGLE,
	LCD_CMD_DRAW_CIRCLE,
	LCD_CMD_DIS_GB2312_CHAR,

	LCD_CMD_INVALID
};

#define LCD_FONT_DEFAULT_SIZE LCD_FONT_SIZE1
enum lcd_font_size_e {
	LCD_FONT_SIZE0	= 0,	/* small */
	LCD_FONT_SIZE1	= 1,	/* middle */
	LCD_FONT_SIZE2	= 2,	/* large */
	LCD_FONT_SIZE3	= 3,	/*  */

	LCD_FONT_INVALID

};

#define USE_DEFAULT_MATRIX_POS	(-1)
#define USE_DEFAULT_COLOR	(-1)

#define BIT_IS_MSG_NOT_PROC		0X01

typedef u16 font_id_t;
typedef u16 lcd_color_t;
typedef void (*lcdm_callback_t)(void);
union len_u {
	u16 width; /* 矩形区域, 表示宽度(以像素为单位) */
	s16 end_x;	
	u16 len;   /* 显示字符时, 表示字符个数 */
	s16 radius;
	
};

union height_u {
	u16 height; /* 矩形区域, 表示高度(以像素为单位) */
	s16 end_y;
};

union font_size_u {
	s16 font_size;
	s16 isfill;
};

struct lcd_mb_st {
	s16 		  cmd;
	s16 		  start_x; /* 起始点阵x坐标, -1表示没有指定位置 */
	s16 		  start_y; /* 起始点阵y坐标, -1表示没有指定位置 */
	lcd_color_t 	  color;
	lcd_color_t	  bgcolor;
	union font_size_u font_size;
	union len_u	  len;
	union height_u	  height;
	const font_id_t   *buf;	 /* 需要显示的字符id */
	lcdm_callback_t	  callback_fn;	 /* 消息处理完成后的回调函数 */
	int		  flag;
};

/*
 * x -- H, [0, 319]
 * y -- V, [0, 239]
 */
struct cursor_pos {
	u16 x;
	u16 y;
};

extern struct lcd_ipc_mb lcd_mb;

#define save_lcd_cur_pos(pos) 		get_lcd_cur_pos((pos))
#define restore_lcd_cur_pos(pos)	set_lcd_cur_pos((pos))


/*
 * line/column <--> dot-matrix row/column
 *
 * 3.2'' screen, dot-matrix
 ** Horizontal, x[0, 320)  -- 320/16 = 20, 320/24 = 13.3
 ** Vertical,   y[0, 240)  -- 240/16 = 15, 240/24 = 10
 *
 * 7'' screen, dot-matrix
 ** Horizontal, x[0, 800)
 ** Vertical,   y[0, 480)
 *
 * number start at 0
 */
#define X_POSITION_MAX 799
#define Y_POSITION_MAX 479

#define ROW_MATRIX_MAX		Y_POSITION_MAX
#define COLUMN_MATRIX_MAX	X_POSITION_MAX

#define LINE_16X24_MAX		(10 - 1)
#define COLUMN_16X24_MAX	(20 - 1)
#define LINE_24X24_MAX		(10 - 1)
#define COLUMN_24X24_MAX	(13 - 1)

#define LINE_HIGHT_GAP		(2)
#define LINE_HIGHT_Xx16_MAX	(16 + LINE_HIGHT_GAP)
#define LINE_HIGHT_Xx24_MAX	(24 + LINE_HIGHT_GAP)
#define LINE_HIGHT_Xx40_MAX	(40 + LINE_HIGHT_GAP)


#define get_16x24_line_from_mtr(row)	((row) / 24)
#define get_16x24_col_from_mtr(column)	((column) / 16)
#define get_24x24_line_from_mtr(row)	((row) / 24)
#define get_24x24_col_from_mtr(column)	((column) / 24)

#define get_mtr_row_from_16x24_line(row)	((row) * 24)
#define get_mtr_col_from_16x24_col(column)	((column) * 16)
#define get_mtr_row_from_24x24_line(row)	((row) * 24)
#define get_mtr_col_from_24x24_col(column)	((column) * 24)



extern void thread_lcd_entry(void* parameter);
extern int get_lcd_cur_pos(struct cursor_pos *pos);
extern int set_lcd_cur_pos(struct cursor_pos *pos);
extern void init_lcd_mb_st(struct lcd_mb_st *buf_st);

extern void lcd_draw_homochromy_color_pic(u16 x, u16 y, u16 w, u16 h,
		lcd_color_t fg_c, lcd_color_t bg_c,char *pic);

#endif

