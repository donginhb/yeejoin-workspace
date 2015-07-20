#include "lcd_touch.h"
#include "ili9320.h"
#include "ili9320_fontid.h"
#include <rtdef.h>
#include <keyboard.h>

#define lcd_debug 


struct lcd_ipc_mb lcd_mb;

static struct cursor_pos cur_pos;
static u16 char_color; /* 字符颜色 */
static u16 bg_color;   /* 背景颜色 */

static void analysis_lcd_msg(struct lcd_mb_st *lcd_buf);
static void display_ascii_char(u16 font_id, lcd_color_t color, lcd_color_t bg, enum lcd_font_size_e size);
static void display_cn_char(u16 font_id, lcd_color_t color, lcd_color_t bg, enum lcd_font_size_e size);
static void adjust_x_y_position(int x_offset, int y_offset, int line_h);
extern int is_lcd_msg_cmd_valid(int cmd);

#if 1
static void proc_dis_char_msg(struct lcd_mb_st *lcd_buf);
static void proc_clr_rect_msg(struct lcd_mb_st *lcd_buf);
void lcd_draw_homochromy_color_pic(u16 x, u16 y, u16 w, u16 h,
		lcd_color_t fg_c, lcd_color_t bg_c,char *pic);
		
void ra8875_draw_circle(int x, int y, int r, int color, int isfill);
void ra8875_draw_rect(int xs, int ys, int xe, int ye, int color, int isfill);

void RA8875_WAITSTATUS(void);
void WriteCommand(unsigned int index);
void    WriteData(unsigned int val);

static void proc_dis_gb2312_char_msg(struct lcd_mb_st *lcd_buf, lcd_color_t color, lcd_color_t bgcolor);

#endif

extern void draw_title_line_bg(void);

void thread_lcd_entry(void* parameter)
{
	struct lcd_mb_st *lcd_buf;
	
	//rt_kprintf("fun:%s, line:%d\n", __FUNCTION__, __LINE__);

	char_color = Blue;
	bg_color   = White;

	draw_title_line_bg();

	while (1) {
		if (RT_EOK == rt_mb_recv(&lcd_mb._mb, (rt_uint32_t *)&lcd_buf, RT_WAITING_FOREVER)) {
			analysis_lcd_msg(lcd_buf);
		}
	}
}

int get_lcd_cur_pos(struct cursor_pos *pos)
{
	if (NULL == pos)
		return FAIL;

	pos->x = cur_pos.x;
	pos->y = cur_pos.y;

	return SUCC;
}

int set_lcd_cur_pos(struct cursor_pos *pos)
{
	if (NULL == pos)
		return FAIL;

	cur_pos.x = pos->x;
	cur_pos.y = pos->y;

	return SUCC;
}

void init_lcd_mb_st(struct lcd_mb_st *buf_st)
{
	if (NULL == buf_st)
		return;

	buf_st->cmd			= LCD_CMD_INVALID;
	buf_st->start_x 		= USE_DEFAULT_MATRIX_POS;
	buf_st->start_y 		= USE_DEFAULT_MATRIX_POS;
	buf_st->color   		= USE_DEFAULT_COLOR;
	buf_st->bgcolor			= HyalineBackColor;
	buf_st->font_size.font_size	= LCD_FONT_DEFAULT_SIZE;
	buf_st->len.len			= 0;
	buf_st->height.height		= 0;
	buf_st->buf     		= NULL;
	buf_st->callback_fn		= NULL;
	buf_st->flag			= 0;

	return;
}


#if 1
void clear_line24dot(int line, u16 color)
{
	int i, j;
	int line_dot;

	line_dot = get_mtr_row_from_24x24_line(line);
	for (i=0; i<24; i++) {
		for (j=0; j<=COLUMN_MATRIX_MAX; j++)
			ili9320_SetPoint(j, line_dot+i, color);
	}

	return;	
}
#endif
/*
 ***************************************************************************************
 */
static void analysis_lcd_msg(struct lcd_mb_st *lcd_buf)
{
	
	if (NULL == lcd_buf)
		return;

	switch (lcd_buf->cmd) {
	case LCD_CMD_DIS_CHAR:
	case LCD_CMD_DIS_GB2312_CHAR:
		proc_dis_char_msg(lcd_buf);
		break;

	case LCD_CMD_CLR_RECTANGLE:
		proc_clr_rect_msg(lcd_buf);
		break;

	case LCD_CMD_DRAW_RECTANGLE:
		ra8875_draw_rect(lcd_buf->start_x, lcd_buf->start_y, lcd_buf->len.end_x,
			lcd_buf->height.end_y, lcd_buf->color, lcd_buf->font_size.isfill);
		break;

	case LCD_CMD_DRAW_CIRCLE:
		ra8875_draw_circle(lcd_buf->start_x, lcd_buf->start_y, lcd_buf->len.radius,
				lcd_buf->color, lcd_buf->font_size.isfill);
		break;

	default:
		break;
	}

	/* 无论命令是否有效, 都将以处理标志置1 */
	clr_bit(lcd_buf->flag, BIT_IS_MSG_NOT_PROC);
	
	return;
}

int is_lcd_msg_cmd_valid(int cmd)
{
	return ((cmd>=LCD_CMD_DIS_CHAR)&&(cmd<LCD_CMD_INVALID) ? 1 : 0);
}

static void display_ascii_char(u16 font_id, lcd_color_t color, lcd_color_t bg, enum lcd_font_size_e size)
{
	/* judge id if valid */
	if (font_id<' ' || font_id>'~') {
		return;
	}

	if (LCD_FONT_SIZE1==size || LCD_FONT_DEFAULT_SIZE==size) {
		if (cur_pos.x+16 > READ_MODE_ZONE_END_X)
			adjust_x_y_position(-cur_pos.x+READ_MODE_ZONE_START_X, 24, LINE_HIGHT_Xx16_MAX);
		ili9320_PutChar_16x24(cur_pos.x, cur_pos.y, font_id, color, bg);
		adjust_x_y_position(16, 0, LINE_HIGHT_Xx24_MAX);
	}

	return;
}

static void display_cn_char(u16 font_id, lcd_color_t color, lcd_color_t bg, enum lcd_font_size_e size)
{
	/* judge id if valid */
	if (font_id<FONT_ID_CN_BEI || font_id>=FONT_ID_CN_INVALID) {
		return;
	}

	if (LCD_FONT_SIZE0==size) {
		if (cur_pos.x+16 > READ_MODE_ZONE_END_X)
			adjust_x_y_position(-cur_pos.x+READ_MODE_ZONE_START_X, 16, LINE_HIGHT_Xx16_MAX);
		ili9320_PutChar_cn_16x16(cur_pos.x, cur_pos.y, font_id, color, bg);
		adjust_x_y_position(16, 0, LINE_HIGHT_Xx16_MAX);
	} else if (LCD_FONT_SIZE1==size) {
		if (cur_pos.x+24 > READ_MODE_ZONE_END_X)
			adjust_x_y_position(-cur_pos.x+READ_MODE_ZONE_START_X, 24, LINE_HIGHT_Xx24_MAX);
		ili9320_PutChar_cn_24x24(cur_pos.x, cur_pos.y, font_id, color, bg);
		adjust_x_y_position(24, 0, LINE_HIGHT_Xx24_MAX);
	} else if (LCD_FONT_SIZE2==size) {
		if (cur_pos.x+40 > READ_MODE_ZONE_END_X)
			adjust_x_y_position(-cur_pos.x+READ_MODE_ZONE_START_X, 40, LINE_HIGHT_Xx40_MAX);
		ili9320_PutChar_cn_40x40(cur_pos.x, cur_pos.y, font_id, color, bg);
		adjust_x_y_position(40, 0, LINE_HIGHT_Xx40_MAX);
	}

	return;
}

static void adjust_x_y_position(int x_offset, int y_offset, int line_h)
{
	int x, y;

	x = cur_pos.x + x_offset;
	y = cur_pos.y + y_offset;

	if (y > Y_POSITION_MAX) {
		ili9320_Clear(bg_color);//clear screen
		cur_pos.x = 0;
		cur_pos.y = 0;
	} else if (x > X_POSITION_MAX) {
		cur_pos.x = 0;

		y += line_h; /* mark by David */
		if ( y > Y_POSITION_MAX) {
			ili9320_Clear(bg_color);//clear screen
			cur_pos.x = 0;
			cur_pos.y = 0;
		} else {
			cur_pos.y = y;
		}
	} else {
		cur_pos.x = x;
		cur_pos.y = y;
	}

	return;
}


static void proc_dis_char_msg(struct lcd_mb_st *lcd_buf)
{
	int cnt, is_restore_pos = 0;
	const font_id_t *buf;
	font_id_t font_id;
	struct cursor_pos save_pos;
	struct cursor_pos pos;
	lcd_color_t color;
	enum lcd_font_size_e font_size;

	if (USE_DEFAULT_MATRIX_POS!=lcd_buf->start_x
		&& USE_DEFAULT_MATRIX_POS!=lcd_buf->start_y) {
		save_lcd_cur_pos(&save_pos);
		pos.x = lcd_buf->start_x;
		pos.y = lcd_buf->start_y;
		set_lcd_cur_pos(&pos);
		is_restore_pos = 1;
	}
/*
	lcd_debug("fun:%s, line:%d, cur_pos.x:%d, cur_pos.y:%d, cnt:%d\n", __FUNCTION__, __LINE__,
		cur_pos.x, cur_pos.y, lcd_buf->len.len);
*/	
	if (USE_DEFAULT_COLOR == lcd_buf->color)	
		color = char_color;
	else
		color = lcd_buf->color;

	if (LCD_CMD_DIS_GB2312_CHAR == lcd_buf->cmd) {
		proc_dis_gb2312_char_msg(lcd_buf, color, lcd_buf->bgcolor);
	} else {
		buf = lcd_buf->buf;
		cnt = lcd_buf->len.len;
		font_size = lcd_buf->font_size.font_size;
		get_lcd_cur_pos(&pos);
		while (cnt-- > 0) {
			font_id = *buf++;
			if ('\n'==font_id) {
				if (LCD_FONT_SIZE0 == font_size)
					adjust_x_y_position(-pos.x+READ_MODE_ZONE_START_X, 16, LINE_HIGHT_Xx16_MAX);
				else if (LCD_FONT_SIZE1==font_size)
					adjust_x_y_position(-pos.x+READ_MODE_ZONE_START_X, 24, LINE_HIGHT_Xx24_MAX);
				else if (LCD_FONT_SIZE1==font_size)
					adjust_x_y_position(-pos.x+READ_MODE_ZONE_START_X, 40, LINE_HIGHT_Xx40_MAX);
			}else if (font_id < CN_FONT_ID_BASE)
				display_ascii_char(font_id, color, lcd_buf->bgcolor, font_size);
			else
				display_cn_char(font_id, color, lcd_buf->bgcolor, font_size);
		}
	}

	if (0 != is_restore_pos)
		restore_lcd_cur_pos(&save_pos);

	if (NULL != lcd_buf->callback_fn)
		lcd_buf->callback_fn();

	return;
}

static void proc_dis_gb2312_char_msg(struct lcd_mb_st *lcd_buf, lcd_color_t color, lcd_color_t bgcolor)
{
	unsigned char *pch;
	struct cursor_pos pos;
	font_id_t font_id;
	int len;

	pch = (unsigned char *)lcd_buf->buf;
	len = lcd_buf->len.len;

	while (0 != len) {
		font_id = *pch++;
		--len;
		if (font_id > 128) {
			font_id <<= 8;
			font_id |= *pch++;
			--len;
		}
		
		if ('\n'==font_id) {
			get_lcd_cur_pos(&pos);
			adjust_x_y_position(-pos.x+READ_MODE_ZONE_START_X, 16, LINE_HIGHT_Xx16_MAX);			
		} else if (font_id < 128) {
			if (cur_pos.x+8 > READ_MODE_ZONE_END_X)
				adjust_x_y_position(-cur_pos.x+READ_MODE_ZONE_START_X, 16, LINE_HIGHT_Xx16_MAX);
			/* 8x16 ascii */
			ili9320_PutChar(cur_pos.x, cur_pos.y, font_id, color, bgcolor);
			adjust_x_y_position(8, 0, LINE_HIGHT_Xx16_MAX);
		} else {
			if (cur_pos.x+16 > READ_MODE_ZONE_END_X)
				adjust_x_y_position(-cur_pos.x+READ_MODE_ZONE_START_X, 16, LINE_HIGHT_Xx16_MAX);
			put_gb2312_char_16x16(cur_pos.x, cur_pos.y, font_id, color, bgcolor);
			adjust_x_y_position(16, 0, LINE_HIGHT_Xx16_MAX);

		}
	}
	
}

static void proc_clr_rect_msg(struct lcd_mb_st *lcd_buf)
{
	int i, j;
	int startx, starty;
	int h, w;
	lcd_color_t color;

	if (USE_DEFAULT_COLOR == lcd_buf->color)	
		color = bg_color;
	else
		color = lcd_buf->color;

	startx	= lcd_buf->start_x;
	starty	= lcd_buf->start_y;
	h 	= lcd_buf->height.height;
	w	= lcd_buf->len.width;
	color	= lcd_buf->color;
	
	for (i=0; i<h; i++) {
		for (j=0; j<w; j++)
			ili9320_SetPoint(startx+j, starty+i, color);
	}


	if (NULL != lcd_buf->callback_fn)
		lcd_buf->callback_fn();

	return;

}

/*
 * lcd_draw_homochromy_color_picture
 * w必须是8的倍数
 */
void lcd_draw_homochromy_color_pic(const u16 x, const u16 y, const u16 w, const u16 h,
		lcd_color_t fg_c, lcd_color_t bg_c,char *pic)
{
	int ix, jy, i, j;
	int tmp_char=0 ;

	if (NULL==pic || (0!=(w&0x7)))
		return;

	for (jy=0; jy<h; jy++) {
		ix = 0;
		for (i=0; i<(w/8/4); i++) {
			tmp_char = *pic++;
			tmp_char <<= 8;
			tmp_char |= (*pic++);
			tmp_char <<= 8;
			tmp_char |= (*pic++);
			tmp_char <<= 8;
			tmp_char |= (*pic++);

			for (j=31; j>=0; j--,ix++) {
				if ( 1 == ((tmp_char >> j) & 0x01)) {
					ili9320_SetPoint(x+ix, y+jy, fg_c); // 字符颜色
				} else if (HyalineBackColor != bg_c) {
					ili9320_SetPoint(x+ix, y+jy, bg_c); // 背景颜色
				}
				/* else, do nothing // 透明背景 */
			}
		}

		j = w/8 % 4;
		if (0 != j) {
			tmp_char = 0;
			for (i=0; i<j; i++) {
				tmp_char <<= 8;
				tmp_char |= *pic++;
			}

			for (i=j*8-1; i>=0; i--,ix++) {
				if ( 1 == ((tmp_char >> i) & 0x01)) {
					ili9320_SetPoint(x+ix, y+jy, fg_c); // 字符颜色
				} else if (HyalineBackColor != bg_c) {
					ili9320_SetPoint(x+ix, y+jy, bg_c); // 背景颜色
				}
				/* else, do nothing // 透明背景 */
			}

		}
		
	}

	return;	
}


/*
 * REG[90h] Draw Line/Circle/Square Control Register (DCR) 
 * 
 * bit7  Draw Line/Square/Triangle Start Signal 
 * Write Function 
 * 0 : Stop the drawing function. 1 : Start the drawing function. 
 * Read Function 
 * 0 : Drawing function complete. 1 : Drawing function is processing. 
 * 
 * bit6 Draw Circle Start Signal 
 * Write Function 
 * 0 : Stop the circle drawing function. 1 : Start the circle drawing function. 
 * Read Function 
 * 0 : Circle drawing function complete. 1 : Circle drawing function is processing. 
 * 
 * bit5 Fill the Circle/Square/Triangle Signal 
 * 0 : Non fill.  1 : Fill. 
 * 
 * bit4 Draw Line or Square Select Signal 
 * 0 : Draw line. 1 : Draw square. 
 * 
 * bit3-1 NA  0  RO 
 * 
 * bit0 Draw Triangle or Line/Square Select Signal 
 * 0 : Draw Line or Square 1 : Draw Triangle
 */
#define RA8875_REG_DCR		(0X90)

/*
 * REG[63h] Foreground Color Register 0 (FGCR0) 
 * Foreground Color Red[4:0] 
 * If REG[10h] Bit[3:2] is set to 256 colors, the register only uses Bit[2:0]. 
 * If REG[10h] Bit[3:2] is set to 65K colors, the register uses Bit[4:0].
 * 
 * REG[64h] Foreground Color Register 1 (FGCR1) 
 * Foreground Color Green[5:0] 
 * If REG[10h] Bit[3:2] is set to 256 colors, the register only uses Bit[2:0]. 
 * If REG[10h] Bit[3:2] is set to 65K colors, the register uses Bit[5:0]. 
 * 
 * REG[65h] Foreground Color Register 2 (FGCR2) 
 * Foreground Color Blue[4:0] 
 * If REG[10h] Bit[3:2] is set to 256 colors, the register only uses Bit[1:0]. 
 * If REG[10h] Bit[3:2] is set to 65K colors, the register uses Bit[4:0]. 
 */

/*
 * 1. set the center of the circle, reg[0x99:0x9c]
 * 2. set the radius of the circle, reg[0x9d]
 * 3. set the color  of the circle, reg[0x63:0x65]
 * 4. start draw 
 *	reg[0x90]bit6: 1--start, 0--stop
 *	reg[0x90]bit5: 1--fill, 0--don't fill
 *
 * reg[0x99].bit[7:0] -- h[7:0], horizontal
 * reg[0x9a].bit[1:0] -- h[9:8], horizontal
 * reg[0x9b].bit[7:0] -- v[7:0], vertical
 * reg[0x9c].bit[7:0] -- v[8], vertical
 * reg[0x9d].bit[7:0] -- r[7:0], radius
 */
void ra8875_draw_circle(int x, int y, int r, int color, int isfill)
{
	RA8875_WAITSTATUS();
	
	/* center */
	WriteCommand(0x99);
	WriteData(x & 0xff);
	WriteCommand(0x9a);
	WriteData((x>>8) & 0x03);

	WriteCommand(0x9b);
	WriteData(y & 0xff);
	WriteCommand(0x9c);
	WriteData((y>>8) & 0x03);

	/* radius */
	WriteCommand(0x9d);
	WriteData(r & 0xff);

	/* color red */
	WriteCommand(0x63);
	WriteData((color>>11) & 0x1f);

	/* color green */
	WriteCommand(0x64);
	WriteData((color>>5) & 0x3f);

	/* color blue */
	WriteCommand(0x65);
	WriteData((color) & 0x1f);

	WriteCommand(RA8875_REG_DCR);
	if (isfill)
		WriteData(0x60);
	else
		WriteData(0x40);
	

	return;
}

/*
 * 1. set start point,  reg[0x91:0x94]
 * 2. set end point,    reg[0x95:0x98]
 * 3. set color, 	reg[0x63:0x65]
 * 4. set RA8875_REG_DCR reg
 *	reg[0x90].bit7 = 1   -- start
 *	reg[0x90].bit5 = 0/1 -- don't fill/fill
 *	reg[0x90].bit4 = 1   -- draw square
 *	reg[0x90].bit0 = 0   -- draw line or square
 */
void ra8875_draw_rect(int xs, int ys, int xe, int ye, int color, int isfill)
{
	RA8875_WAITSTATUS();
	
	/* start */
	WriteCommand(0x91);
	WriteData(xs & 0xff);
	WriteCommand(0x92);
	WriteData((xs>>8) & 0x03);

	WriteCommand(0x93);
	WriteData(ys & 0xff);
	WriteCommand(0x94);
	WriteData((ys>>8) & 0x03);

	/* end */
	WriteCommand(0x95);
	WriteData(xe & 0xff);
	WriteCommand(0x96);
	WriteData((xe>>8) & 0x03);

	WriteCommand(0x97);
	WriteData(ye & 0xff);
	WriteCommand(0x98);
	WriteData((ye>>8) & 0x03);

	/* color red */
	WriteCommand(0x63);
	WriteData((color>>11) & 0x1f);

	/* color green */
	WriteCommand(0x64);
	WriteData((color>>5) & 0x3f);

	/* color blue */
	WriteCommand(0x65);
	WriteData((color) & 0x1f);

	WriteCommand(RA8875_REG_DCR);
	if (isfill)
		WriteData(0xb0);
	else
		WriteData(0x90);
	

	return;
}






