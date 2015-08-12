/*
 * form.h
 *
 * 2013-1-31,  creat by David, zhaoshaowei@yeejoin.com
 */

#ifndef FORM_H__
#define FORM_H__

#include <rtgui/rtgui.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/widgets/widget.h>

DECLARE_CLASS_TYPE(form);
/** Gets the type of a form */
#define RTGUI_FORM_TYPE		(RTGUI_TYPE(form))
/** Casts the object to a form */
#define RTGUI_FORM(obj)		(RTGUI_OBJECT_CAST((obj), RTGUI_FORM_TYPE, rtgui_form_t))
/** Checks if the object is a form */
#define RTGUI_IS_FORM(obj)	(RTGUI_OBJECT_CHECK_TYPE((obj), RTGUI_FORM_TYPE))


struct rtgui_form
{
	struct rtgui_widget parent;

	/* widget private data */
	const char **head_name;		/* 以NULL指针结束 */
	rt_uint16_t head_item_cnt;	/* 表格列数 */

	rt_uint16_t row_cnt_of_fbody; /* 不包含表格头 */
	char *fbody;
	rt_uint16_t bytes_of_row;	/* 存储表格一行内容, 需要的字节数 */
	rt_uint16_t reserve;

	struct rtgui_rect form_rect;

};
typedef struct rtgui_form rtgui_form_t;

extern rtgui_form_t *rtgui_form_create(const char *head_name[], int rows, int cols, rtgui_rect_t *rect);
extern void rtgui_form_destroy(rtgui_form_t* form);
extern void rtgui_form_set_item(rtgui_form_t *form, const char * const item, int row, int col, int isupdate);
extern void rtgui_form_set_row(rtgui_form_t *form, const char *const rowstr, int row);
extern void rtgui_form_update_row(struct rtgui_form *form, int row);
extern void rtgui_form_ondraw(struct rtgui_form *form);
extern void rtgui_val_add_point_offset(char *item);

#endif

