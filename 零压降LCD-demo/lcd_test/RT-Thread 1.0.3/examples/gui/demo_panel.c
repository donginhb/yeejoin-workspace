#include <rtgui/rtgui.h>
#include <rtgui/driver.h>
#include <rtgui/rtgui_server.h>

/*
 * Panel demo for 240x320
 * info panel: (0,  0) - (240, 25)
 * main panel: (0, 25) - (240, 320)
 */
void panel_init(void)
{
	rtgui_rect_t rect;

#if 0
	/* register dock panel */
	rect.x1 = 0;
	rect.y1 = 0;
	rect.x2 = rtgui_graphic_driver_get_default()->width;
	rect.y2 = 24;
	rtgui_panel_register("info", &rect);

	/* register main panel */
	rect.x1 = 0;
	rect.y1 = 25;
	rect.x2 = rtgui_graphic_driver_get_default()->width;
	rect.y2 = rtgui_graphic_driver_get_default()->height;
	rtgui_panel_register("main", &rect);
#else
	/* register main panel */
	rect.x1 = 0;
	rect.y1 = 0;
	rect.x2 = rtgui_graphic_driver_get_default()->width;
	rect.y2 = rtgui_graphic_driver_get_default()->height;
	rtgui_panel_register("main", &rect);
#endif
	rtgui_panel_set_default_focused("main");

}
