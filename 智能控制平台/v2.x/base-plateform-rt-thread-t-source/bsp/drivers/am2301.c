#include <board.h>
#include <rtthread.h>
#include <rtdef.h>
#include <stm32f10x_gpio.h>

#include <lcd_touch.h>
#include <ili9320_fontid.h>

#include "misc_lib.h"
#include <keyboard.h>
#include <am2301.h>
#include <ili9320.h>
#include <syscfgdata.h>
#include <finsh.h>

#include <webm_p.h>

#define am2301_debug

static char am2301_buf[5];

volatile u16 tmp_from_am2301, rh_from_am2301;
volatile int warning_info;
volatile int has_notifi_warning_info;
volatile int warning_led_vector;
volatile int need_delay_notifi;

struct tmp_rh_limen_max_s {
	s16	tmp_h_max;
	s16	tmp_l_min;
	u16	rh_h_max;
	u16	rh_l_min;
}tmp_rh_limen_max;


static int read_data_from_am2301(char *buf);
//static void display_am2301_data(int tmp, int rh);
static void waiting_x_us(int us);
static void am2301_sda_as_input(void);
static void am2301_sda_as_output(void);

struct temp_rh_limen t_rh_value;

void thread_am2301_entry(void* parameter)
{
	int tmp, rh;

	(void)parameter;
	
	/* 上电后等待3s, am2301进入稳定状态 */
	rt_thread_delay(get_ticks_of_ms(3000));

	if (SUCC != read_temp_rh_limen(&t_rh_value)) {
		printf_syn("func:%s, line:%d, err.\n", __FUNCTION__, __LINE__);
		t_rh_value.temp_h 	= TMP_DEFAULT_LIMEN_H;
		t_rh_value.temp_l 	= TMP_DEFAULT_LIMEN_L;
		t_rh_value.rh_h		= RH_DEFAULT_LIMEN_H;
		t_rh_value.rh_l		= RH_DEFAULT_LIMEN_L;
	}

	rt_memset(&tmp_rh_limen_max, ~0, sizeof(tmp_rh_limen_max));

	while (1) {
		if (SUCC == read_data_from_am2301(am2301_buf)) {
			tmp = am2301_buf[2]<<8 | am2301_buf[1];
			rh  = am2301_buf[4]<<8 | am2301_buf[3];
			#if 1
			tmp_from_am2301 = tmp;
			rh_from_am2301  = rh;

			if (tmp_from_am2301 & 0x8000)
				tmp = -(tmp_from_am2301 & 0x7fff);

			if (tmp > t_rh_value.temp_h) {
				if ((~0 == tmp_rh_limen_max.tmp_h_max) || (tmp>tmp_rh_limen_max.tmp_h_max)) {
					tmp_rh_limen_max.tmp_h_max = tmp;
					set_bit(warning_info, TMP_OVERRUN);
					set_bit(warning_led_vector, TMP_OVERRUN);
				}
			} else if (tmp < t_rh_value.temp_l) {
				if ((~0 == tmp_rh_limen_max.tmp_l_min) || (tmp<tmp_rh_limen_max.tmp_l_min)) {
					tmp_rh_limen_max.tmp_l_min = tmp;
					set_bit(warning_info, TMP_OVERRUN);
					set_bit(warning_led_vector, TMP_OVERRUN);
				} 				
			} else {
				clr_bit(warning_info, TMP_OVERRUN);
				clr_bit(has_notifi_warning_info, TMP_OVERRUN);
				clr_bit(warning_led_vector, TMP_OVERRUN);
				led_off(LED_PORTX, WARNING_LED);
			}

			if (rh > t_rh_value.rh_h) {
				if (((~0&0xffff) == tmp_rh_limen_max.rh_h_max) || (rh>tmp_rh_limen_max.rh_h_max)) {
					tmp_rh_limen_max.rh_h_max = rh;
					set_bit(warning_info, RH_OVERRUN);
					set_bit(warning_led_vector, RH_OVERRUN);
				} 				
			} else if (rh < t_rh_value.rh_l) {
				if (((~0&0xffff) == tmp_rh_limen_max.rh_l_min) || (rh<tmp_rh_limen_max.rh_l_min)) {
					tmp_rh_limen_max.rh_l_min = rh;
					set_bit(warning_info, RH_OVERRUN);
					set_bit(warning_led_vector, RH_OVERRUN);
				} 				
			} else {
				clr_bit(warning_info, RH_OVERRUN);
				clr_bit(has_notifi_warning_info, RH_OVERRUN);
				clr_bit(warning_led_vector, RH_OVERRUN);
				led_off(LED_PORTX, WARNING_LED);
			}
/*
printf_syn("func:%s, line:%d,tmp:%d, rh:%d, (%d, %d, %d, %d).warning_info:%x, has_notifi_warning_info:%x, warning_led_vector:%x\n", __FUNCTION__, __LINE__, tmp, rh,
	t_rh_value.temp_h, t_rh_value.temp_l, t_rh_value.rh_h, t_rh_value.rh_l, warning_info, has_notifi_warning_info, warning_led_vector);
*/			
			#else
			display_am2301_data(tmp, rh);
			#endif
		} else {
			/* printf_syn("func:%s, read am2301 fail\n", __FUNCTION__); */
			tmp_from_am2301 = 0;
			rh_from_am2301  = 0;
		}

		/* 间隔35s读一次数据 */
		rt_thread_delay(get_ticks_of_ms(35000));
	}

	return;
}

/*
 * signal 0 H -- 22, 26, 30 us
 * signal 1 H -- 68, 70, 75 us
 *
 * 湿度：湿度分辨率是16bit，高位在前，传感器传出的湿度值是实际值的10倍
 * 温度：温度分辨率是16bit，高位在前，传感器传出的温度值是实际值的10倍，
 *	 使用16bit有符号数的原码表示
 */

/* 反汇编 */
#define SIG0_H_COUNT_OF_1US (72/6)

#define SIG0_H_T_CNT_MIN
//#define SIG0_H_T_CNT_MAX (30*SIG0_H_COUNT_OF_1US)
/* 120, 60,  */
#define SIG0_H_T_CNT_MAX (80)

#define SIG1_H_T_CNT_MIN
#define SIG1_H_T_CNT_MAX

/* 接收数据位时最大等待计数 */
#define SIG01_TIMEOUT_MAX (320)

static int read_data_from_am2301(char *buf)
{
	int bit_cnt, t_cnt;
	int data4_1, data0;
	u32 start_ticks;
	
	/* 保持1ms低电平 */
	am2301_sda_as_output();
	clr_port_pin(TMP_PORT, TMP_Pin);
	waiting_x_us(1000);

	/* cpu释放总线30us */
	am2301_sda_as_input();
	waiting_x_us(1);

	/* 高电平时, am2301未响应 */
	start_ticks = rt_tick_get();
	while(pin_in_is_set(TMP_PORT, TMP_Pin)) {
		if (rt_tick_get()-start_ticks >= 2) {
			goto err_entry;
		}
	}
	
	/* 响应低电平 */
	start_ticks = rt_tick_get();
	while(!pin_in_is_set(TMP_PORT, TMP_Pin)) {
		if (rt_tick_get()-start_ticks >= 2) {
			goto err_entry;
		}
	}

	/* 响应高电平 */
	start_ticks = rt_tick_get();
	while(pin_in_is_set(TMP_PORT, TMP_Pin)) {
		if (rt_tick_get()-start_ticks >= 2) {
			goto err_entry;
		}
	}

	bit_cnt = 0;
	data4_1 = 0;
	data0   = 0;
	do {
		/* 数据低电平 */
		t_cnt = 0;
		while(!pin_in_is_set(TMP_PORT, TMP_Pin)) {
			++t_cnt;
			if (t_cnt > SIG01_TIMEOUT_MAX) {
				goto err_entry;
			}
		}

		/* 数据高电平 */
		t_cnt = 0;
		while(pin_in_is_set(TMP_PORT, TMP_Pin)) {
			++t_cnt;
			if (t_cnt > SIG01_TIMEOUT_MAX) {
				goto err_entry;
			}
		}

//am2301_debug("t_cnt:%d\n", t_cnt);
		if (t_cnt <= SIG0_H_T_CNT_MAX) {
			// sig 0
			if (bit_cnt < 32) 
				data4_1 <<= 1;
			else
				data0   <<= 1;
 		} else {
			// sig 1
			if (bit_cnt < 32) {
				data4_1 <<= 1;
				data4_1 |=  1;
			} else {
				data0   <<= 1;
				data0   |=  1;
			}
		}
	} while (++bit_cnt < 40);

	buf[0] = data0;
	buf[1] = data4_1       & 0xff;
	buf[2] = (data4_1>>8)  & 0xff;
	buf[3] = (data4_1>>16) & 0xff;
	buf[4] = (data4_1>>24) & 0xff;

	if (data0 == ((buf[1]+buf[2]+buf[3]+buf[4]) & 0xff))
		return SUCC;

err_entry:
		return FAIL;
}

#if 0
static struct lcd_mb_st am2301_lcd_buf;
static u16 buf_pool[28];

/* temperature, relative humidity */
static void display_am2301_data(int tmp, int rh)
{
	u16 *pbuf;
	char str[16];
	char *pch;
	
	//am2301_debug("tmp:%d, rh:%d\n", tmp, rh);

	pbuf = buf_pool;

	/* temperature */
	*pbuf++ = FONT_ID_CN_WEN;
	*pbuf++ = FONT_ID_CN_DU;
	*pbuf++ = ':';
	if (tmp & (1<<15))
		tmp = -(tmp & 0x7fff);
	i2str(str, tmp);
	pch = str;
	while ('\0' != *pch)
		*pbuf++ = *pch++;
	*pbuf   = *(pbuf-1);
	*(pbuf-1) = '.';
	++pbuf;
	*pbuf++ = FONT_ID_CN_DU_SYMBOL;

	/* gap */
	*pbuf++ = '\n';

	/* relative humidity */
	*pbuf++ = FONT_ID_CN_SHI;
	*pbuf++ = FONT_ID_CN_DU;
	*pbuf++ = ':';
	i2str(str, rh);
	pch = str;
	while ('\0' != *pch)
		*pbuf++ = *pch++;
	*pbuf   = *(pbuf-1);
	*(pbuf-1) = '.';
	++pbuf;
	*pbuf++ = '%';

	init_lcd_mb_st(&am2301_lcd_buf);
	am2301_lcd_buf.cmd	= LCD_CMD_DIS_CHAR;
	am2301_lcd_buf.start_x 	= get_mtr_col_from_24x24_col(0);
	am2301_lcd_buf.start_y 	= get_mtr_row_from_24x24_line(LINE_24X24_MAX-1);
	am2301_lcd_buf.buf     	= buf_pool;
	am2301_lcd_buf.len.len	= pbuf - buf_pool;

	rt_mb_send(&lcd_mb._mb, (rt_uint32_t)&am2301_lcd_buf);
	return;
}
#endif
/*
 * 反汇编
 * #define COUNT_OF_1US (72/7)
 * 
 * 实测(cpu--72MHz)
 ** 6 -- 1.08us -- waiting_x_us(1000)
 ** 5 -- 0.84us -- waiting_x_us(1000)
 */
#define COUNT_OF_1US (6)

static void waiting_x_us(int us)
{
	int cnt;

	cnt = us * COUNT_OF_1US;
	while (cnt >= 0)
		--cnt;

	return;
}

static void am2301_sda_as_input(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin   = TMP_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
	GPIO_Init(TMP_PORT, &GPIO_InitStructure);

	return;
}



static void am2301_sda_as_output(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin   = TMP_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;
	GPIO_Init(TMP_PORT, &GPIO_InitStructure);
	
	return;
}

#if 1

void dis_net_link_state(int epon_state, int onu_state)
{
	int i, line_cnt;

	/* 线路运行状态监测 */
	draw_title_line_bg();
	while (is_bit_set(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC))
		rt_thread_delay(get_ticks_of_ms(20));
	i = 0;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_XIAN1;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_LU;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_YUN;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_XING;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_ZHUANG;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_TAI;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_JIAN;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_CE;
	set_bit(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC);
	dis_title_line(&textmenu2lcd_mb, i);


	line_cnt = 2;

	/* EPON线路:xxx */
	while (is_bit_set(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC))
		rt_thread_delay(get_ticks_of_ms(20));
	i = 0;
	textmenu2lcd_mb.buf_pool[i++] = 'E';
	textmenu2lcd_mb.buf_pool[i++] = 'P';
	textmenu2lcd_mb.buf_pool[i++] = 'O';
	textmenu2lcd_mb.buf_pool[i++] = 'N';
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_XIAN1;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_LU;
	textmenu2lcd_mb.buf_pool[i++] = ':';
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_YUN;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_XING;
	switch (epon_state) {
	case NORMAL_OPERATION:
		textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_ZHENG;
		textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_CHANG;
		break;
	case ABNORMITY_OPERATION:
		textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_YI1;
		textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_CHANG;
		break;
	default:
		i -= 2;
		textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_ZHUANG;
		textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_TAI;
		textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_WEI;
		textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_ZHI2;
		break;
	}
	set_bit(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC);

	init_lcd_mb_st(&textmenu2lcd_mb.textmenu_msg);
	textmenu2lcd_mb.textmenu_msg.cmd		 = LCD_CMD_DIS_CHAR;
	textmenu2lcd_mb.textmenu_msg.start_x 		 = BODY_START_X;
	textmenu2lcd_mb.textmenu_msg.start_y 		 = BODY_START_Y 
				+ line_cnt*MIDDLE_CN_FONT_HEIGHT + LINE_GAP;
	textmenu2lcd_mb.textmenu_msg.color		 = Blue2;
	textmenu2lcd_mb.textmenu_msg.font_size.font_size = LCD_FONT_SIZE1;
	textmenu2lcd_mb.textmenu_msg.buf		 = textmenu2lcd_mb.buf_pool;
	textmenu2lcd_mb.textmenu_msg.len.len = i;

	rt_mb_send(&lcd_mb._mb, (rt_uint32_t)&textmenu2lcd_mb.textmenu_msg);
	++line_cnt;

	/* ONU设备:xxx */
	while (is_bit_set(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC))
		rt_thread_delay(get_ticks_of_ms(20));
	i = 0;
	textmenu2lcd_mb.buf_pool[i++] = 'O';
	textmenu2lcd_mb.buf_pool[i++] = 'N';
	textmenu2lcd_mb.buf_pool[i++] = 'U';
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_SHE;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_BEI1;
	textmenu2lcd_mb.buf_pool[i++] = ':';
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_YUN;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_XING;
	switch (epon_state) {
	case NORMAL_OPERATION:
		textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_ZHENG;
		textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_CHANG;
		break;
	case ABNORMITY_OPERATION:
		textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_YI1;
		textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_CHANG;
		break;
	default:
		i -= 2;
		textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_ZHUANG;
		textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_TAI;
		textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_WEI;
		textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_ZHI2;
		break;
	}
	set_bit(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC);

	init_lcd_mb_st(&textmenu2lcd_mb.textmenu_msg);
	textmenu2lcd_mb.textmenu_msg.cmd		 = LCD_CMD_DIS_CHAR;
	textmenu2lcd_mb.textmenu_msg.start_x 		 = BODY_START_X;
	textmenu2lcd_mb.textmenu_msg.start_y 		 = BODY_START_Y
			+ line_cnt*MIDDLE_CN_FONT_HEIGHT + LINE_GAP;
	textmenu2lcd_mb.textmenu_msg.color		 = Blue2;
	textmenu2lcd_mb.textmenu_msg.font_size.font_size = LCD_FONT_SIZE1;
	textmenu2lcd_mb.textmenu_msg.buf		 = textmenu2lcd_mb.buf_pool;
	textmenu2lcd_mb.textmenu_msg.len.len = i;

	rt_mb_send(&lcd_mb._mb, (rt_uint32_t)&textmenu2lcd_mb.textmenu_msg);
	++line_cnt;
	
	return;
}

void dis_tmp_rh(void)
{
	u16 *pbuf;
	char str[16];
	char *pch;
	int tmp, rh, i;

	/* 环境参数 */
	draw_title_line_bg();
	while (is_bit_set(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC))
		rt_thread_delay(get_ticks_of_ms(20));
	set_bit(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC);

	i = 0;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_HUAN;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_JING1;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_CAN;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_SHU;
	dis_title_line(&textmenu2lcd_mb, i);

	tmp = tmp_from_am2301;
	rh  = rh_from_am2301;	

	while (is_bit_set(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC))
		rt_thread_delay(get_ticks_of_ms(20));
	set_bit(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC);

	pbuf = textmenu2lcd_mb.buf_pool;

	/* temperature */
	*pbuf++ = FONT_ID_CN_WEN;
	*pbuf++ = FONT_ID_CN_DU;
	*pbuf++ = ':';
	if (tmp & (1<<15))
		tmp = -(tmp & 0x7fff);
	i2str(str, tmp);
	pch = str;
	while ('\0' != *pch)
		*pbuf++ = *pch++;
	*pbuf   = *(pbuf-1);
	*(pbuf-1) = '.';
	++pbuf;
	*pbuf++ = FONT_ID_CN_DU_SYMBOL;

	/* gap */
	//*pbuf++ = '\n';

	*pbuf++ = ' ';
	*pbuf++ = ' ';

	/* relative humidity */
	*pbuf++ = FONT_ID_CN_SHI;
	*pbuf++ = FONT_ID_CN_DU;
	*pbuf++ = ':';
	i2str(str, rh);
	pch = str;
	while ('\0' != *pch)
		*pbuf++ = *pch++;
	*pbuf   = *(pbuf-1);
	*(pbuf-1) = '.';
	++pbuf;
	*pbuf++ = '%';

	init_lcd_mb_st(&textmenu2lcd_mb.textmenu_msg);
	textmenu2lcd_mb.textmenu_msg.cmd		 = LCD_CMD_DIS_CHAR;
	textmenu2lcd_mb.textmenu_msg.start_x 		 = BODY_START_X;
	textmenu2lcd_mb.textmenu_msg.start_y 		 = BODY_START_Y
			+ 2*MIDDLE_CN_FONT_HEIGHT + LINE_GAP;
	textmenu2lcd_mb.textmenu_msg.color		 = Blue2;
	textmenu2lcd_mb.textmenu_msg.font_size.font_size = LCD_FONT_SIZE1;
	textmenu2lcd_mb.textmenu_msg.buf		 = textmenu2lcd_mb.buf_pool;
	textmenu2lcd_mb.textmenu_msg.len.len = pbuf - textmenu2lcd_mb.buf_pool;

	rt_mb_send(&lcd_mb._mb, (rt_uint32_t)&textmenu2lcd_mb.textmenu_msg);

	return;
}


int read_temp_rh_limen(struct temp_rh_limen *t_rh)
{
	if (NULL == t_rh)
		return FAIL;

	return read_syscfgdata_tbl(SYSCFGDATA_TBL_TMP_RH, 0, t_rh);
}

int save_temp_rh_limen(struct temp_rh_limen *t_rh)
{
	if (NULL == t_rh)
		return FAIL;

	return write_syscfgdata_tbl(SYSCFGDATA_TBL_TMP_RH, 0, t_rh);

}


int set_tmprh_limen(int temp_h, int temp_l, int rh_h, int rh_l)
{
	int ret;
	struct temp_rh_limen t_rh;
	int temp, temp1;

	if (0==temp_h && 0==temp_l && 0==rh_h && 0==rh_l) {
		if (tmp_from_am2301 & (1<<15))
			temp = -(tmp_from_am2301 & ~(1<<15));
		else
			temp = tmp_from_am2301;
		temp1 = rh_from_am2301;

		read_temp_rh_limen(&t_rh);

		printf_syn("cur temp:%d, rh:%d\n", temp, temp1);

		printf_syn("limen setting--temp_h:%d, temp_l:%d, rh_h:%d, rh_l:%d\noverrange--"
			"temph:%d, templ:%d, rh_h:%d, rh_l:%d\n", t_rh.temp_h, t_rh.temp_l,
			t_rh.rh_h, t_rh.rh_l, tmp_rh_limen_max.tmp_h_max, tmp_rh_limen_max.tmp_l_min,
			tmp_rh_limen_max.rh_h_max, tmp_rh_limen_max.rh_l_min);

			

		return SUCC;
	}

	t_rh.temp_h = temp_h;
	t_rh.temp_l = temp_l;
	t_rh.rh_h   = rh_h;
	t_rh.rh_l   = rh_l;

	ret = save_temp_rh_limen(&t_rh);
	read_temp_rh_limen(&t_rh_value);

	return ret;
}
FINSH_FUNCTION_EXPORT(set_tmprh_limen, "temp_h, temp_l, rh_h, rh_l");

#endif

#if 1
void dis_warning_info(void)
{
	u16 *pbuf;
	int i, line_cnt = 2;
	char str[16];
	char *pch;
	s16 tmp;
	u16 rh;

	/* 告警信息 */
	draw_title_line_bg();
	while (is_bit_set(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC))
		rt_thread_delay(get_ticks_of_ms(20));
	set_bit(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC);

	i = 0;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_GAO;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_JING2;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_XIN;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_XI;
	dis_title_line(&textmenu2lcd_mb, i);

	/* 温度过限: */
	while (is_bit_set(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC))
		rt_thread_delay(get_ticks_of_ms(20));
	set_bit(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC);

	/* temperature */
	pbuf = textmenu2lcd_mb.buf_pool;
	*pbuf++ = FONT_ID_CN_WEN;
	*pbuf++ = FONT_ID_CN_DU;
	*pbuf++ = FONT_ID_CN_GUO1;
	*pbuf++ = FONT_ID_CN_XIAN;
	*pbuf++ = ':';
	tmp = tmp_rh_limen_max.tmp_l_min;
	if (~tmp) {
		i2str(str, tmp);
		pch = str;
		while ('\0' != *pch)
			*pbuf++ = *pch++;
		*pbuf   = *(pbuf-1);
		*(pbuf-1) = '.';
		++pbuf;
	} else {
		*pbuf++ = '-';
	}
	*pbuf++ = '/';
	tmp = tmp_rh_limen_max.tmp_h_max;
	if (~tmp) {
		i2str(str, tmp);
		pch = str;
		while ('\0' != *pch)
			*pbuf++ = *pch++;
		*pbuf   = *(pbuf-1);
		*(pbuf-1) = '.';
		++pbuf;
	} else {
		*pbuf++ = '-';
	}
	
	*pbuf++ = FONT_ID_CN_DU_SYMBOL;

	init_lcd_mb_st(&textmenu2lcd_mb.textmenu_msg);
	textmenu2lcd_mb.textmenu_msg.cmd		 = LCD_CMD_DIS_CHAR;
	textmenu2lcd_mb.textmenu_msg.start_x 		 = BODY_START_X;
	textmenu2lcd_mb.textmenu_msg.start_y 		 = BODY_START_Y
			+ (line_cnt++)*MIDDLE_CN_FONT_HEIGHT + LINE_GAP;
	textmenu2lcd_mb.textmenu_msg.color		 = Blue2;
	textmenu2lcd_mb.textmenu_msg.font_size.font_size = LCD_FONT_SIZE1;
	textmenu2lcd_mb.textmenu_msg.buf		 = textmenu2lcd_mb.buf_pool;
	textmenu2lcd_mb.textmenu_msg.len.len = pbuf - textmenu2lcd_mb.buf_pool;

	rt_mb_send(&lcd_mb._mb, (rt_uint32_t)&textmenu2lcd_mb.textmenu_msg);


	/* 湿度过限: */
	while (is_bit_set(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC))
		rt_thread_delay(get_ticks_of_ms(20));
	set_bit(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC);

	pbuf = textmenu2lcd_mb.buf_pool;
	*pbuf++ = FONT_ID_CN_SHI;
	*pbuf++ = FONT_ID_CN_DU;
	*pbuf++ = FONT_ID_CN_GUO1;
	*pbuf++ = FONT_ID_CN_XIAN;
	*pbuf++ = ':';
	rh = tmp_rh_limen_max.rh_l_min;
	if ((~rh) & 0xffff) {
		i2str(str, rh);
		pch = str;
		while ('\0' != *pch)
			*pbuf++ = *pch++;
		*pbuf   = *(pbuf-1);
		*(pbuf-1) = '.';
		++pbuf;
	} else {
		*pbuf++ = '-';
	}
	*pbuf++ = '/';
	rh = tmp_rh_limen_max.rh_h_max;
	if ((~rh) & 0xffff) {
		i2str(str, rh);
		pch = str;
		while ('\0' != *pch)
			*pbuf++ = *pch++;
		*pbuf   = *(pbuf-1);
		*(pbuf-1) = '.';
		++pbuf;
	} else {
		*pbuf++ = '-';
	}
	
	*pbuf++ = '%';

	init_lcd_mb_st(&textmenu2lcd_mb.textmenu_msg);
	textmenu2lcd_mb.textmenu_msg.cmd		 = LCD_CMD_DIS_CHAR;
	textmenu2lcd_mb.textmenu_msg.start_x 		 = BODY_START_X;
	textmenu2lcd_mb.textmenu_msg.start_y 		 = BODY_START_Y
			+ (line_cnt++)*MIDDLE_CN_FONT_HEIGHT + LINE_GAP;
	textmenu2lcd_mb.textmenu_msg.color		 = Blue2;
	textmenu2lcd_mb.textmenu_msg.font_size.font_size = LCD_FONT_SIZE1;
	textmenu2lcd_mb.textmenu_msg.buf		 = textmenu2lcd_mb.buf_pool;
	textmenu2lcd_mb.textmenu_msg.len.len = pbuf - textmenu2lcd_mb.buf_pool;

	rt_mb_send(&lcd_mb._mb, (rt_uint32_t)&textmenu2lcd_mb.textmenu_msg);

	return;
}


/* 延迟时间是180s, 1s有4个计数 */
#define DELAY_NOTIFI_TIME (180*4)
#define DELAY_COLSE_ELOCK_TIME (30*4)
#define WARN_STATE_NOTIFI_PERIOD (360*4)

#define IS_ELOCK_OPENED_BIT4DELAY_CLOSE 0X01

struct webm_mb_entity warn_webm_mb;
/*
 * 锁门打开 开关断开 高电平
 *
 * #define elock_open  set_port_pin
 * #define elock_close clr_port_pin
 */
int check_if_has_warning(int tick_cnt)
{
	static int cnt;
	static int auto_close_lock_cnt;
	static int status_flage;
	static int wlock_cnt; /* 非法开锁状态持续, 间隔发送告警 */
	static int wtmp_cnt;
	static int wrh_cnt;

	/* 延迟, 自动关闭电子锁 */
	if (is_elock_open(ELOCK_PORT, ELOCK_PIN)) {
		if (!pin_in_is_set(ELOCK_SWITCH_PORT, ELOCK_SWITCH_PIN)) { /* 箱门是关闭的 */
			if (is_bit_clr(status_flage, IS_ELOCK_OPENED_BIT4DELAY_CLOSE)) {
				set_bit(status_flage, IS_ELOCK_OPENED_BIT4DELAY_CLOSE);
				auto_close_lock_cnt = tick_cnt;
			} else if (tick_cnt-auto_close_lock_cnt > DELAY_COLSE_ELOCK_TIME) {
				clr_bit(status_flage, IS_ELOCK_OPENED_BIT4DELAY_CLOSE);
				elock_close(dummy);
			}
			/*  else, 如果延迟时间不够, 什么也不做 */
		} else 
			clr_bit(status_flage, IS_ELOCK_OPENED_BIT4DELAY_CLOSE);
	} else {
		clr_bit(status_flage, IS_ELOCK_OPENED_BIT4DELAY_CLOSE);
	}

	/* 非法开锁的判断 */
	if (pin_in_is_set(ELOCK_SWITCH_PORT, ELOCK_SWITCH_PIN) && !is_elock_open(ELOCK_PORT, ELOCK_PIN)) {
		/* 非法开锁 */
		set_bit(warning_info, ILLEGAL_OPEN_LOCK);
		set_bit(warning_led_vector, ILLEGAL_OPEN_LOCK);
	} else {
		clr_bit(warning_info, ILLEGAL_OPEN_LOCK);
		clr_bit(has_notifi_warning_info, ILLEGAL_OPEN_LOCK);
		clr_bit(warning_led_vector, ILLEGAL_OPEN_LOCK);
		led_off(LED_PORTX, FAULT_LED);
	}

	/* 是否有告警信息需要发送 */
	if (0!=warning_info || 0!=need_delay_notifi) {
		rt_memset(&warn_webm_mb, 0, sizeof(warn_webm_mb));
		warn_webm_mb.mb_ent.cmd = CM_WARNNING_IFNO_NOTIFI;

		if (is_bit_set(warning_info, ILLEGAL_OPEN_LOCK)) { 
			if (is_bit_clr(has_notifi_warning_info, ILLEGAL_OPEN_LOCK)
					|| tick_cnt-wlock_cnt > WARN_STATE_NOTIFI_PERIOD) {
				set_bit(has_notifi_warning_info, ILLEGAL_OPEN_LOCK);
				warn_webm_mb.mb_ent.com_buf.usr_info.usrid[0] = 1;
				warn_webm_mb.flag += 1;
				wlock_cnt = tick_cnt;
			}
		} else if (is_bit_set(need_delay_notifi, ILLEGAL_OPEN_LOCK) && (tick_cnt-cnt > DELAY_NOTIFI_TIME)) {
			warn_webm_mb.mb_ent.com_buf.usr_info.usrid[0] = 1;
			warn_webm_mb.flag += 1;
		}

		if (is_bit_set(warning_info, TMP_OVERRUN)) {
			if (is_bit_clr(has_notifi_warning_info, TMP_OVERRUN)
					|| tick_cnt-wtmp_cnt > WARN_STATE_NOTIFI_PERIOD) {
				set_bit(has_notifi_warning_info, TMP_OVERRUN);
				warn_webm_mb.mb_ent.com_buf.usr_info.usrid[1] = 1;
				warn_webm_mb.flag += 1;
				wtmp_cnt = tick_cnt;
			}
		} else if (is_bit_set(need_delay_notifi, TMP_OVERRUN)
				&& ((tick_cnt-cnt > DELAY_NOTIFI_TIME) || 0!=warn_webm_mb.flag)) {
			warn_webm_mb.mb_ent.com_buf.usr_info.usrid[1] = 1;
			warn_webm_mb.flag += 1;
		}

		if (is_bit_set(warning_info, RH_OVERRUN)) {
			if (is_bit_clr(has_notifi_warning_info, RH_OVERRUN)
					|| tick_cnt-wrh_cnt > WARN_STATE_NOTIFI_PERIOD) {
				set_bit(has_notifi_warning_info, RH_OVERRUN);
				warn_webm_mb.mb_ent.com_buf.usr_info.usrid[2] = 1;
				warn_webm_mb.flag += 1;
				wrh_cnt = tick_cnt;
			}
		} else if (is_bit_set(need_delay_notifi, RH_OVERRUN)
				&& ((tick_cnt-cnt > DELAY_NOTIFI_TIME) || 0!=warn_webm_mb.flag)) {
			warn_webm_mb.mb_ent.com_buf.usr_info.usrid[2] = 1;
			warn_webm_mb.flag += 1;
		}

		if (0 != warn_webm_mb.flag) {
			cnt = tick_cnt;
			rt_mb_send_wait(&webm_mb._mb, (rt_uint32_t)&warn_webm_mb.mb_ent, 0);
		}
	}

	return 0;
}
#endif

