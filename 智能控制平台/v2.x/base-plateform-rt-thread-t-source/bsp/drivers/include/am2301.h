#ifndef AM2301_H__
#define AM2301_H__

#include <syscfgdata.h>

#define ILLEGAL_OPEN_LOCK 0X01
#define TMP_OVERRUN	0X02
#define RH_OVERRUN	0X04
	
#if 1
enum equipment_state {
	NORMAL_OPERATION	= 0,
	ABNORMITY_OPERATION	= 1,
	UNKNOW_STATE		= 2,
};
#endif

extern volatile u16 tmp_from_am2301;
extern volatile u16 rh_from_am2301;

extern volatile int warning_info;
extern volatile int has_notifi_warning_info;
extern volatile int warning_led_vector;
extern volatile int need_delay_notifi;

extern void thread_am2301_entry(void* parameter);

extern void dis_net_link_state(int epon_state, int onu_state);
extern void dis_tmp_rh(void);
extern void dis_warning_info(void);
extern int check_if_has_warning(int tick_cnt);

extern int read_temp_rh_limen(struct temp_rh_limen *t_rh);
extern int save_temp_rh_limen(struct temp_rh_limen *t_rh);

#endif
