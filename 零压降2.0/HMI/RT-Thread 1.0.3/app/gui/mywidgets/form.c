/*
 ******************************************************************************
 * form.c
 *
 * 2013-1-31,  creat by David, zhaoshaowei@yeejoin.com
 ******************************************************************************
 */
#include <rtthread.h>
#include <rtgui/rtgui.h>
#include <rtgui/dc.h>

#include "form.h"

#define GAG_BEFORE_FORM_HEAD 8 /* 像素 */
#define GAG_BETWEEN_COL      1 /* x8像素(半字对应的宽度) */
#define GAG_BETWEEN_ROW 	 2 /* 像素 */


#define form_debug(x) //printf_syn x

static rt_bool_t rtgui_form_event_handler(struct rtgui_widget *widget, struct rtgui_event *event);
static void _rtgui_form_constructor(struct rtgui_form *form);
static int get_offset_of_col(struct rtgui_form *form, int col, int *offset);
static int draw_form_head(struct rtgui_form *form, const struct rtgui_font *font,
						  struct rtgui_dc* dc, struct rtgui_rect *rect, struct rtgui_rect *row_rect, char *buf);

/*
 * head_name[] -- 最后一个元素必须是NULL
 * rows -- 包含表头在内
 */
rtgui_form_t *rtgui_form_create(const char *head_name[], int rows, int cols, rtgui_rect_t *rect)
{
	int i, len;
	struct rtgui_form *form;

	if (NULL==head_name || NULL==rect)
		return NULL;

	form = (struct rtgui_form *)rtgui_widget_create(RTGUI_FORM_TYPE);
	if (NULL == form)
		return NULL;

	i = 0;
	while (NULL != head_name[i])
		++i;

	if (i != cols) {
		form_debug(("%s(), head items (%d) not same as cols(%d)!\n", __FUNCTION__, i, cols));
		goto err_entry;
	}

	form_debug(("%s(), line:%d: rows:%d, cols:%d!\n", __FUNCTION__, __LINE__, rows, cols));

	len = 0;
	for (i=0; i<cols; ++i) {
		len += rt_strlen(head_name[i]);
	}
	len += cols * GAG_BETWEEN_COL;

	form->fbody = rt_calloc(len*(rows-1), 1); /* 不包含表格头部占用的存储空间 */
	if (NULL == form->fbody) {
		form_debug(("fun:%s(), line:%d:%d malloc fail!\n", __FUNCTION__, __LINE__, len*(rows-1)));
		goto err_entry;
	}

	form->head_name = rt_calloc(cols, sizeof(head_name[0]));
	if (NULL == form->head_name) {
		form_debug(("fun:%s(), line:%d:%d malloc fail!\n", __FUNCTION__, __LINE__, cols * sizeof(head_name[0])));
		rt_free(form->fbody);
		goto err_entry;
	}

	form_debug(("%s(), line:%d: fbody-len:%d, head-name-len:%d!\n", __FUNCTION__, __LINE__,
				len*(rows-1), cols * sizeof(head_name[0])));

	for (i=0; i<cols; ++i) {
		form->head_name[i] = head_name[i];
	}
	form->head_item_cnt		= cols;
	form->row_cnt_of_fbody	= rows - 1;
	form->bytes_of_row		= len;

	rtgui_widget_set_rect(RTGUI_WIDGET(form), rect);

	return form;

err_entry:
	rtgui_widget_destroy(RTGUI_WIDGET(form));
	return NULL;
}


void rtgui_form_destroy(rtgui_form_t* form)
{
	if (NULL != form->head_name) {
		rt_free(form->head_name);
		form->head_name = NULL;
	}

	if (NULL != form->fbody) {
		rt_free(form->fbody);
		form->fbody = NULL;
	}

	/* destroy form */
	rtgui_widget_destroy(RTGUI_WIDGET(form));
	return;
}


/*
 * row, col 都是从0开始编号
 */
void rtgui_form_set_item(rtgui_form_t *form, const char * const item, int row, int col, int isupdate)
{
	int offset, len1, len2, i;
	char *pch;
	const char *pch1;

	if (NULL==form || NULL==item)
		return;

	if (row > form->row_cnt_of_fbody || col >= form->head_item_cnt) {
		printf_syn("fun:%s(), row(%d) or col(%d) error, should(%d, %d), %s!\n", __FUNCTION__, row, col,
				   (form->row_cnt_of_fbody+1), form->head_item_cnt, form->head_name[0]);
		return;
	}
	if (FAIL == get_offset_of_col(form, col, &offset))
		return;

	if (row < 1)
		return;
#if 0
	pch  = form->fbody + (row-1)*form->bytes_of_row;
	form_debug(("before set item[%s], %dth line:%s!\n", item, row, pch));
#endif
	/* 编译器对item的处理在某些条件下似乎会出现bug, const char *item */
	len1 = rt_strlen(item);
	len2 = rt_strlen(form->head_name[col]);
	pch  = form->fbody + (row-1)*form->bytes_of_row + offset;
	pch1 = item;
	i = MIN(len1, len2);

	form_debug(("len1:%d, len2:%d, i:%d, item[%s], %#x, %#x, %#x\n", len1, len2, i, item, item, *item, *(item+1)));

	len2 -= i;
	while (0 != i) {
		*pch++ = *pch1++;
		--i;
	}

	/* 用空格填充列剩余位置及列间隔 */
	len2 += GAG_BETWEEN_COL;
	while (len2--)
		*pch++ = ' ';

	/* 将每行的最后一列的最后一个字节设置为'\0' */
	if ((col+1) == form->head_item_cnt)
		*--pch = '\0';

#if 0
	pch  = form->fbody + (row-1)*form->bytes_of_row;
	form_debug(("after set, %dth line:%s!\n", row, pch));
#endif
	if (0 != isupdate)
		rtgui_form_update_row(form, row);

	//rtgui_widget_update(RTGUI_WIDGET(form));
	return;
}

/*
 * row, col 都是从0开始编号, 是表格的全局编号
 */
void rtgui_form_set_row(rtgui_form_t *form, const char *const rowstr, int row)
{
	int len1, len2, i;
	char *pch;
	const char *pch1;

	if (NULL==form || NULL==rowstr)
		return;

	if (row > form->row_cnt_of_fbody || row<1) {
		form_debug(("fun:%s(), row(%d) error!\n", __FUNCTION__, row));
		return;
	}

	/* 编译器对item的处理在某些条件下似乎会出现bug, const char *item */
	len1 = rt_strlen(rowstr);
	len2 = form->bytes_of_row;
	pch  = form->fbody + (row-1)*form->bytes_of_row;
	pch1 = rowstr;
	i = MIN(len1, len2);

	while (0 != i) {
		*pch++ = *pch1++;
		--i;
	}

	/* 将每行的最后一列的最后一个字节设置为'\0' */
	pch  = form->fbody + (row)*form->bytes_of_row - 1;
	*pch = '\0';

	return;
}


void rtgui_form_update_row(struct rtgui_form *form, int row)
{
	struct rtgui_dc* dc;
	struct rtgui_rect row_rect;
	const struct rtgui_font *font;
	char *pch1;
	int i;

	if (NULL==form || row<=0) {
		form_debug(("fun:%s(), param error(%x, %d)!\n", __FUNCTION__, form, row));
		return;
	}
	dc = rtgui_dc_begin_drawing(RTGUI_WIDGET(form));
	if (dc == RT_NULL) return;

	if (0xffff == form->form_rect.x1)
		return;

	font = RTGUI_DC_FONT(dc);
	if (font == RT_NULL) {
		/* use system default font */
		font = rtgui_font_default();
	}

	i = (row * (font->height + GAG_BETWEEN_ROW));
	row_rect.x1 = form->form_rect.x1;
	row_rect.y1 = form->form_rect.y1 + i;
	row_rect.x2 = form->form_rect.x2;
	row_rect.y2 = form->form_rect.y2 + i;

	pch1 = form->fbody + (row - 1) * form->bytes_of_row;
	rtgui_dc_draw_text(dc, pch1, &row_rect);

	rtgui_dc_end_drawing(dc);

	return;
}

void rtgui_form_ondraw(struct rtgui_form *form)
{
	struct rtgui_dc* dc;
	struct rtgui_rect rect, row_rect;
	const struct rtgui_font *font;
	char *buf;
	char *pch1;
	int i, j;

	if (NULL == form) {
		form_debug(("fun:%s(), param error!\n", __FUNCTION__));
		return;
	}

	dc = rtgui_dc_begin_drawing(RTGUI_WIDGET(form));
	if (dc == RT_NULL) return;

	buf = rt_malloc(form->bytes_of_row);
	if (NULL == buf) {
		form_debug(("fun:%s(), line:%d:%d malloc fail!\n", __FUNCTION__, __LINE__, form->bytes_of_row));
		rtgui_dc_end_drawing(dc);
		return;
	}

	rtgui_widget_get_rect(RTGUI_WIDGET(form), &rect);
	rtgui_dc_fill_rect(dc, &rect);
	font = RTGUI_DC_FONT(dc);
	if (font == RT_NULL)
		font = rtgui_font_default();

	draw_form_head(form, font, dc, &rect, &row_rect, buf);

	j = form->row_cnt_of_fbody;
	pch1 = form->fbody;
	for (i=0; i<j; ++i) {
		row_rect.y1 += font->height + GAG_BETWEEN_ROW;
		row_rect.y2 += font->height + GAG_BETWEEN_ROW;
		rtgui_dc_draw_text(dc, pch1, &row_rect);

		form_debug(("fun:%s(), %dth line:%s!\n", __FUNCTION__, i, pch1));

		pch1 += form->bytes_of_row;
	}

	rtgui_dc_end_drawing(dc);

	rt_free(buf);
	return;
}


static rt_bool_t rtgui_form_event_handler(struct rtgui_widget *widget, struct rtgui_event *event)
{
	struct rtgui_form* form;
	//struct rtgui_event_command* ecmd;

	form = RTGUI_FORM(widget);

	switch (event->type) {
	case RTGUI_EVENT_PAINT:
		rtgui_form_ondraw(form);
		break;
#if 0
	case RTGUI_EVENT_COMMAND:
		ecmd = (struct rtgui_event_command*)event;
		switch (ecmd->command_id) {
		case SE_FORM_UPDATE:
			if (RTGUI_WIDGET_IS_FOCUSED(RTGUI_WIDGET(se_form)))
				rtgui_widget_update(RTGUI_WIDGET(se_form));
			break;

		case RE_FORM_UPDATE:
			rtgui_widget_update(RTGUI_WIDGET(re_form));
			break;

		case SYS_FORM_UPDATE:
			rtgui_widget_update(RTGUI_WIDGET(sys_form));
			break;
		}
		break;
#endif
	default:
		/* use form event handler */
		return rtgui_widget_event_handler(widget, event);
	}

	return RT_FALSE;
}


static void _rtgui_form_constructor(struct rtgui_form *form)
{
	rt_memset(form, 0, sizeof(*form));

	/* form中元素已全部清零, 下面只需要对初值不为零的进行初始化 */
	/* set default widget rect and set event handler */
	rtgui_widget_set_event_handler(RTGUI_WIDGET(form),rtgui_form_event_handler);

	RTGUI_WIDGET(form)->flag |= RTGUI_WIDGET_FLAG_FOCUSABLE;
	form->form_rect.x1 = ~0;

	RTGUI_WIDGET_BACKGROUND(RTGUI_WIDGET(form)) = white;
	RTGUI_WIDGET_TEXTALIGN(RTGUI_WIDGET(form))  = RTGUI_ALIGN_CENTER_VERTICAL;

	return;
}

DEFINE_CLASS_TYPE(form, "form", RTGUI_WIDGET_TYPE,	_rtgui_form_constructor, RT_NULL, sizeof(struct rtgui_form));

static int get_offset_of_col(struct rtgui_form *form, int col, int *offset)
{
	int i, j, len;

	if (NULL == form || col >= form->head_item_cnt) {
		form_debug(("fun:%s(), col(%d) error!\n", __FUNCTION__, col));
		return FAIL;
	}

	j = col;
	len = 0;
	for(i=0; i<j; ++i)
		len += rt_strlen(form->head_name[i]);
	*offset = len + j*GAG_BETWEEN_COL;

	return SUCC;
}

static int draw_form_head(struct rtgui_form *form, const struct rtgui_font *font,
						  struct rtgui_dc* dc, struct rtgui_rect *rect, struct rtgui_rect *row_rect, char *buf)
{
	struct rtgui_rect form_rect;
	char *pch1;
	const char *pch2;
	int i, j, len;
#if 0
	form_debug(("func:%s, (%d,%d), (%d,%d),bytes:%d, h:%d, r:%d\n", __FUNCTION__,
				rect->x1, rect->y1, rect->x2, rect->y2, form->bytes_of_row, font->height, form->row_cnt_of_fbody));
#endif
	form_rect.x1 = 0;
	form_rect.y1 = 0;
	form_rect.x2 = (form->bytes_of_row + GAG_BETWEEN_COL*form->head_item_cnt) * 8;
	form_rect.y2 = (font->height + GAG_BETWEEN_ROW) * (form->row_cnt_of_fbody + 1);
	rtgui_rect_moveto_align(rect, &form_rect, RTGUI_ALIGN_CENTER_HORIZONTAL);
	form_rect.y1 += GAG_BEFORE_FORM_HEAD;
	form_rect.y2 += GAG_BEFORE_FORM_HEAD;

	if (0xffff == form->form_rect.x1) {
		form->form_rect = form_rect;
	}

	j = form->head_item_cnt;
	pch1 = buf;
	for (i=0; i<j; ++i) {
		pch2 = form->head_name[i];
		while ('\0' != *pch2)
			*pch1++ = *pch2++;

		/* 用空格填充列间隔 */
		len = GAG_BETWEEN_COL;
		while (len--)
			*pch1++ = ' ';
	}
	*--pch1 = '\0';

	row_rect->x1 = form_rect.x1;
	row_rect->y1 = form_rect.y1;
	row_rect->x2 = form_rect.x2;
	row_rect->y2 = form_rect.y1 + font->height;
#if 0
	form_debug(("func:%s, (%d,%d), (%d,%d),hic:%d, len:%d, buf:%s\n", __FUNCTION__,
				row_rect->x1, row_rect->y1, row_rect->x2,row_rect->y2,j,pch1 -buf, buf));
#endif
	rtgui_dc_draw_text(dc, buf, row_rect);
	rtgui_dc_draw_horizontal_line(dc, row_rect->x1, row_rect->x2, row_rect->y2+1);

	return SUCC;
}
