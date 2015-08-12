#ifndef __TOUCH_H__
#define __TOUCH_H__

#include <rtthread.h>
#include <syscfgdata.h>

#define RT_TOUCH_NORMAL				0
#define RT_TOUCH_CALIBRATION_DATA	1
#define RT_TOUCH_CALIBRATION 		2

struct calibration_data
{
	rt_uint16_t min_x, max_x;
	rt_uint16_t min_y, max_y;
	rt_bool_t exchange_xy;

};

typedef void (*rt_touch_calibration_func_t)(rt_uint16_t x, rt_uint16_t y);

void rtgui_touch_hw_init(void);

int get_touch_param(struct touch_calib_param *touch_param);
int set_touch_param(struct touch_calib_param *touch_param);


#endif

