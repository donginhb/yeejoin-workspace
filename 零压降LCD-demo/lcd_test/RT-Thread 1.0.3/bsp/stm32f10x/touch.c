#include <stdbool.h>
#include "stm32f10x.h"

#include "board.h"
#include "touch.h"

#include <rtthread.h>
#include <rtgui/event.h>
#include <rtgui/kbddef.h>
#include <rtgui/rtgui_server.h>
#include <rtgui/rtgui_system.h>
#include <syscfgdata.h>

int touch_print_on;

/*
 * 7  6 - 4  3      2     1-0
 * s  A2-A0 MODE SER/DFR PD1-PD0
 */
#define TOUCH_MSR_Y  0x90   //读X轴坐标指令 addr:1
#define TOUCH_MSR_X  0xD0   //读Y轴坐标指令 addr:3

/* pen Interrupt -- low  */
#define is_tsc2046_int_valid()	(!pin_in_is_set(TOUCH_INT_PORT, TOUCH_INT_PIN))

struct rtgui_touch_device {
	struct rt_device parent;

	rt_timer_t poll_timer;
	rt_uint16_t x, y;

	rt_bool_t calibrating;
	rt_touch_calibration_func_t calibration_func;

	rt_uint16_t min_x, max_x;
	rt_uint16_t min_y, max_y;
	rt_bool_t exchange_xy;
};

static struct rtgui_touch_device *touch = RT_NULL;
struct rt_semaphore touch_spi_lock;

extern unsigned char SPI_WriteByte(unsigned char data);
void exti_ctrl(rt_uint32_t enable);


void rt_hw_spi1_baud_rate(uint16_t SPI_BaudRatePrescaler)
{
#if 1
	TOUCH_USE_SPIX->CR1 &= ~SPI_BaudRatePrescaler_256;
	TOUCH_USE_SPIX->CR1 |= SPI_BaudRatePrescaler;
#endif
}

#if 0==TOUCH_USE_PHONY_SPI
#define SPI_WriteByte(data) spix_send_byte(TOUCH_USE_SPIX, data)

#if 0
uint8_t SPI_WriteByte(unsigned char data)
{
	//Wait until the transmit buffer is empty
	while (SPI_I2S_GetFlagStatus(TOUCH_USE_SPIX, SPI_I2S_FLAG_TXE) == RESET);
	// Send the byte
	SPI_I2S_SendData(TOUCH_USE_SPIX, data);

	//Wait until a data is received
	while (SPI_I2S_GetFlagStatus(TOUCH_USE_SPIX, SPI_I2S_FLAG_RXNE) == RESET);
	// Get the received data
	data = SPI_I2S_ReceiveData(TOUCH_USE_SPIX);

	// Return the shifted data
	return data;
}
#endif
#else
static void spip_mosi_write(int data)
{
	if (data)
		set_spip_mosi();
	else
		clr_spip_mosi();

	return;
}



/*
 * tsc2046
 * DOUT Data are shifted on the falling edge of DCLK. This output is high impedance when CSis high.
 * DIN  If CS is low, data are latched on the rising edge of DCLK
 *
 * mosi --> DIN, rising edge
 * miso --> DOUT, falling edge
 */
uint8_t SPI_WriteByte(unsigned char data)
{
	unsigned i, send, recv;

	//rt_kprintf("[spi send 0x%x)\n", data);
	send = 0;
	recv = 0;
	for(i = 0; i < 8; i++) {
		send = (data >> (7-i)) & 0x1 ;	//MSB在前,LSB在后
		spip_mosi_write(send);	 		//tsc2046, 时钟上升沿锁存DIN
		set_spip_sck();			//rising edge，一共8个
		spi_delay();
		clr_spip_sck();			//falling edge, 开始发送命令字
		spi_delay();

		recv <<= 1;
		recv |=  spip_miso_read();
	}
	//rt_kprintf("[spi recv 0x%x)\n", recv);

	return recv;
}
#endif


#if 1
/* delay 200ns
 * 1/72 us = 1000 / 72 ns
 * 200 ns = (1000/72) * (2*72/10) ns = 14.4
 */
void spi_delay(void)
{
	int i;
	for (i=0; i<7; ++i)
		;
	return;
}
#endif
#if 0
static void delay_us(int xus)
{
	int i;

	xus *= 36;
	for (i=0; i<xus; i++)
		;

	return;
}
#endif

u16 _AD2Y(u16 adx) //240
{
	u16 sx=0;
	int r = adx - 200;
	r *= 240;
	sx=r / (4000 - 280);
	if (sx<=0 || sx>240)
		return 0;
	return sx;
}


u16 _AD2X(u16 ady) //320
{
	u16 sy=0;
	int r = ady - 260;
	r *= 320;
	sy=r/(3960 - 360);
	if (sy<=0 || sy>320)
		return 0;
	return sy;
}
//SPI写数据
static void WriteDataTo7843(unsigned char num)
{
	SPI_WriteByte(num);
}

/* 不能小于3 */
#define TSC2046_SAMPLE_MAX_CNT (6)
static void rtgui_touch_calculate()
{
	rt_uint16_t tmpx[TSC2046_SAMPLE_MAX_CNT];
	rt_uint16_t tmpy[TSC2046_SAMPLE_MAX_CNT];
	unsigned int i;

	rt_uint32_t min_x = 0xFFFF,min_y = 0xFFFF;
	rt_uint32_t max_x = 0,max_y = 0;
	rt_uint32_t total_x = 0;
	rt_uint32_t total_y = 0;

	if (RT_NULL == touch)
		return;

	rt_sem_take(&touch_spi_lock, RT_WAITING_FOREVER);
	/* TOUCH_USE_SPIX configure */
	rt_hw_spi1_baud_rate(SPI_BaudRatePrescaler_32); /* 36M/32 = 1.125M */

	/* From the datasheet:
	* When the very first CLK after the control byte comes in, the
	* DOUT of ADS7843 is not valid. So we could only get 7bits from
	* the first SPI_WriteByte. And the got the following 5 bits from
	* another SPI_WriteByte.(aligned MSB)
	*/
	clr_port_pin(TOUCH_CS_PORT, TOUCH_CS_PIN);
	WriteDataTo7843( 1<<7 ); /* 打开中断 */

	WriteDataTo7843(TOUCH_MSR_X);
	for(i=0; i<TSC2046_SAMPLE_MAX_CNT; i++)	{
		//delay_us(3) ;
		tmpx[i] = (SPI_WriteByte(0x00) & 0x7F) << 5;
		tmpx[i] |= (SPI_WriteByte(TOUCH_MSR_Y) >> 3) & 0x1F;

		//delay_us(3) ;
		tmpy[i] = (SPI_WriteByte(0x00) & 0x7F) << 5;
		tmpy[i] |= (SPI_WriteByte(TOUCH_MSR_X) >> 3) & 0x1F;

		if (touch_print_on)
			rt_kprintf("[index %d]: (%d, %d)\n", i, tmpx[i], tmpy[i]);
		//delay_us(1);
	}
	set_port_pin(TOUCH_CS_PORT, TOUCH_CS_PIN);

	//丢弃第1个采样值, 去最高值与最低值,再取平均值
	for(i=1; i<TSC2046_SAMPLE_MAX_CNT; i++) {
		if( tmpx[i] < min_x )
			min_x = tmpx[i];
		if( tmpx[i] > max_x )
			max_x = tmpx[i];

		total_x += tmpx[i];

		if( tmpy[i] < min_y )
			min_y = tmpy[i];
		if( tmpy[i] > max_y )
			max_y = tmpy[i];

		total_y += tmpy[i];
	}

	total_x  = total_x - min_x - max_x;
	total_y  = total_y - min_y - max_y;
	if ((RT_TRUE!=touch->calibrating) && (0!=touch->exchange_xy)) {
		min_x = total_x;
		total_x = total_y;
		total_y = min_x;
	}

	touch->x = total_x / (TSC2046_SAMPLE_MAX_CNT-2-1);
	touch->y = total_y / (TSC2046_SAMPLE_MAX_CNT-2-1);

	if (touch_print_on)
		rt_kprintf("[sample avg]: (%d, %d)\n", touch->x, touch->y);

	rt_sem_release(&touch_spi_lock);

	/* if it's not in calibration status  */
	if (touch->calibrating != RT_TRUE) {
		if (touch->max_x > touch->min_x)
			touch->x = (touch->x - touch->min_x) * LCD_X_WIDTH/(touch->max_x - touch->min_x);
		else if (touch->max_x < touch->min_x)
			touch->x = (touch->min_x - touch->x) * LCD_X_WIDTH/(touch->min_x - touch->max_x);

		if (touch->max_y > touch->min_y)
			touch->y = (touch->y - touch->min_y) * LCD_Y_HEIGHT /(touch->max_y - touch->min_y);
		else if (touch->max_y < touch->min_y)
			touch->y = (touch->min_y - touch->y) * LCD_Y_HEIGHT /(touch->min_y - touch->max_y);

		// normalize the data
		if (touch->x & 0x8000)
			touch->x = 0;
		else if (touch->x > LCD_X_WIDTH)
			touch->x = LCD_X_WIDTH - 1;

		if (touch->y & 0x8000)
			touch->y = 0;
		else if (touch->y > LCD_Y_HEIGHT)
			touch->y = LCD_Y_HEIGHT - 1;
	}

	if (touch_print_on)
		rt_kprintf("max(%d, %d), min(%d,%d),calib:%d, (%d,%d)\n", touch->max_x, touch->max_y,
				   touch->min_x, touch->min_y, touch->calibrating, touch->x, touch->y);

	return;
}

#define previous_keep      8
void touch_timeout(void* parameter)
{  
	static unsigned int touched_down = 0;
	static struct _touch_previous {
		rt_uint32_t x;
		rt_uint32_t y;
	} touch_previous;

	struct rtgui_event_mouse emouse;
 
	rt_kprintf("test3\n");

	/* touch time is too short and we lost the position already. */
	if ((!touched_down) && pin_in_is_set(TOUCH_INT_PORT, TOUCH_INT_PIN))
		return;

	//rt_kprintf("line:%d, fun:%s, thread:%s\n",__LINE__, __FUNCTION__, rt_thread_self()->name);

	if (pin_in_is_set(TOUCH_INT_PORT, TOUCH_INT_PIN)) {
		int tmer = RT_TICK_PER_SECOND/8 ;

		exti_ctrl(1);
		emouse.parent.type  = RTGUI_EVENT_MOUSE_BUTTON;
		emouse.button 		= (RTGUI_MOUSE_BUTTON_LEFT |RTGUI_MOUSE_BUTTON_UP);

		/* use old value */
		emouse.x = touch->x;
		emouse.y = touch->y;

		/* stop timer */
		rt_timer_stop(touch->poll_timer);

		if (touch_print_on)
			rt_kprintf("touch up: (%d, %d)\n", emouse.x, emouse.y);

		touched_down = 0;

		if ((touch->calibrating == RT_TRUE) && (touch->calibration_func != RT_NULL)) {
			/* callback function */
			touch->calibration_func(emouse.x, emouse.y);
		}

		rt_timer_control(touch->poll_timer , RT_TIMER_CTRL_SET_TIME , &tmer);
	} else 	{
		if(touched_down == 0) {
			/* mark by David */
			//int tmer = RT_TICK_PER_SECOND/20 ;
			int tmer = RT_TICK_PER_SECOND/3 ;

			/* calculation */
			rtgui_touch_calculate();

			/* send mouse event */
			emouse.parent.type = RTGUI_EVENT_MOUSE_BUTTON;
			emouse.parent.sender = RT_NULL;

			emouse.x = touch->x;
			emouse.y = touch->y;

			touch_previous.x = touch->x;
			touch_previous.y = touch->y;

			/* init mouse button */
			emouse.button = (RTGUI_MOUSE_BUTTON_LEFT |RTGUI_MOUSE_BUTTON_DOWN);

			if (touch_print_on)
				rt_kprintf("touch down: (%d, %d)\n", emouse.x, emouse.y);

			touched_down = 1;
			rt_timer_control(touch->poll_timer , RT_TIMER_CTRL_SET_TIME , &tmer);
		} else {
			/* calculation */
			rtgui_touch_calculate();

			//判断移动距离是否小于previous_keep,减少误动作.
			if((touch_previous.x<touch->x+previous_keep)
			   && (touch_previous.x>touch->x-previous_keep)
			   && (touch_previous.y<touch->y+previous_keep)
			   && (touch_previous.y>touch->y-previous_keep)  )	{
				return;
			}

			touch_previous.x = touch->x;
			touch_previous.y = touch->y;

			/* send mouse event */
			emouse.parent.type = RTGUI_EVENT_MOUSE_BUTTON ;
			emouse.parent.sender = RT_NULL;

			emouse.x = touch->x;
			emouse.y = touch->y;

			/* init mouse button */
			emouse.button = (RTGUI_MOUSE_BUTTON_RIGHT |RTGUI_MOUSE_BUTTON_DOWN);

			if (touch_print_on)
				rt_kprintf("touch motion: (%d, %d)\n", emouse.x, emouse.y);
		}
	}

	/* send event to server */
	if (touch->calibrating != RT_TRUE)
		rtgui_server_post_event(&emouse.parent, sizeof(struct rtgui_event_mouse));

}

void exti_ctrl(rt_uint32_t enable)
{
	EXTI_InitTypeDef EXTI_InitStructure;

	/* Configure  EXTI  */
	EXTI_InitStructure.EXTI_Line 	= TOUCH_EXTI_LINE;
	EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//Falling下降沿 Rising上升

	if (enable)
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	else
		EXTI_InitStructure.EXTI_LineCmd = DISABLE;

	EXTI_Init(&EXTI_InitStructure);
	EXTI_ClearITPendingBit(TOUCH_EXTI_LINE);
}


/* RT-Thread Device Interface */
static rt_err_t rtgui_touch_init (rt_device_t dev)
{
	exti_ctrl(1);

	if (rt_sem_init(&touch_spi_lock, "tspilock", 1, RT_IPC_FLAG_FIFO) != RT_EOK) {
		rt_kprintf("init TOUCH_USE_SPIX lock semaphore failed\n");
	}

	clr_port_pin(TOUCH_CS_PORT, TOUCH_CS_PIN);
	WriteDataTo7843( 1<<7 ); /* 打开中断 */
	set_port_pin(TOUCH_CS_PORT, TOUCH_CS_PIN);

	return RT_EOK;
}

static rt_err_t rtgui_touch_control (rt_device_t dev, rt_uint8_t cmd, void *args)
{
	struct calibration_data* data;

	switch (cmd) {
	case RT_TOUCH_CALIBRATION:
		touch->calibrating = RT_TRUE;
		touch->calibration_func = (rt_touch_calibration_func_t)args;
		break;

	case RT_TOUCH_NORMAL:
		touch->calibrating = RT_FALSE;
		break;

	case RT_TOUCH_CALIBRATION_DATA:
		data = (struct calibration_data*) args;
		//update
		touch->min_x = data->min_x;
		touch->max_x = data->max_x;
		touch->min_y = data->min_y;
		touch->max_y = data->max_y;
		touch->exchange_xy = data->exchange_xy;
		break;
	}

	return RT_EOK;
}

#if 0
void EXTI1_IRQHandler(void)
{
	/* disable interrupt */
	exti_ctrl(0);

	/* start timer */
	rt_timer_start(touch->poll_timer);

	EXTI_ClearITPendingBit(EXTI_Line1);
}

#else
/*
 * #define TOUCH_EXTI_LINE                 EXTI_Line5
 */
void EXTI9_5_IRQHandler(void)
{
	/* enter interrupt */
	rt_interrupt_enter();

	rt_kprintf("test2\n");  

#if 1


	if (RESET != EXTI_GetITStatus(EXTI_Line7)) {
		/* disable interrupt */
		exti_ctrl(0);
		/* start timer */
		/* 注意touch_timeout函数怎么调用回去触摸屏坐标并且处理的，不理解mark by malooei */
		rt_timer_start(touch->poll_timer);

		EXTI_ClearITPendingBit(EXTI_Line7);
	}
#else
	if (RESET != EXTI_GetITStatus(EXTI_Line5)) {
		/* disable interrupt */
		exti_ctrl(0);
		/* start timer */
		rt_timer_start(touch->poll_timer);

		EXTI_ClearITPendingBit(EXTI_Line5);
	}

	if (RESET != EXTI_GetITStatus(EXTI_Line6)) {
		EXTI_ClearITPendingBit(EXTI_Line6);
	}

	if (RESET != EXTI_GetITStatus(EXTI_Line7)) {
		EXTI_ClearITPendingBit(EXTI_Line7);
		//rt_sem_release(&keyscan_chip_had_int);
	}

	if (RESET != EXTI_GetITStatus(EXTI_Line8)) {
		EXTI_ClearITPendingBit(EXTI_Line8);
	}

	if (RESET != EXTI_GetITStatus(EXTI_Line9)) {
		EXTI_ClearITPendingBit(EXTI_Line9);
	}
#endif
	/* leave interrupt */
	rt_interrupt_leave();

	return;
}
#endif

void rtgui_touch_hw_init(void)
{
	struct touch_calib_param touch_param;

	touch = (struct rtgui_touch_device*)rt_malloc (sizeof(struct rtgui_touch_device));
	if (touch == RT_NULL) return; /* no memory yet */

	/* clear device structure */
	rt_memset(&(touch->parent), 0, sizeof(struct rt_device));
	touch->calibrating = false;

	/* init device structure */
	touch->parent.type = RT_Device_Class_Unknown;
	touch->parent.init = rtgui_touch_init;
	touch->parent.control = rtgui_touch_control;
	touch->parent.user_data = RT_NULL;

	/* create 1/8 second timer */
	touch->poll_timer = rt_timer_create("touch", touch_timeout, RT_NULL,
										RT_TICK_PER_SECOND/8, RT_TIMER_FLAG_PERIODIC);
#if 0
	touch->min_x = X_ADC_MIN;
	touch->max_x = X_ADC_MAX;
	touch->min_y = Y_ADC_MIN;
	touch->max_y = Y_ADC_MAX;
#else
	if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_TOUCH_PARAM, 0, &touch_param)) {
		touch->min_x = X_ADC_MIN;
		touch->max_x = X_ADC_MAX;
		touch->min_y = Y_ADC_MIN;
		touch->max_y = Y_ADC_MAX;
	} else {
		touch->min_x = touch_param.x_min;
		touch->max_x = touch_param.x_max;
		touch->min_y = touch_param.y_min;
		touch->max_y = touch_param.y_max;  
	}
#endif
	/* register touch device to RT-Thread */
	rt_device_register(&(touch->parent), "touch", RT_DEVICE_FLAG_RDWR);
}

#if 0 //def RT_USING_FINSH

#include <finsh.h>

void touch_t( rt_uint16_t x , rt_uint16_t y )
{
	struct rtgui_event_mouse emouse ;
	emouse.parent.type = RTGUI_EVENT_MOUSE_BUTTON;
	emouse.parent.sender = RT_NULL;

	emouse.x = x ;
	emouse.y = y ;
	/* init mouse button */
	emouse.button = (RTGUI_MOUSE_BUTTON_LEFT |RTGUI_MOUSE_BUTTON_DOWN );
	rtgui_server_post_event(&emouse.parent, sizeof(struct rtgui_event_mouse));

	rt_thread_delay(2) ;
	emouse.button = (RTGUI_MOUSE_BUTTON_LEFT |RTGUI_MOUSE_BUTTON_UP );
	rtgui_server_post_event(&emouse.parent, sizeof(struct rtgui_event_mouse));
}
FINSH_FUNCTION_EXPORT(touch_t, x & y ) ;


void set_touch_min_max(int x_min, int x_max, int y_min, int y_max)
{
	touch->min_x = x_min;
	touch->max_x = x_max;
	touch->min_y = y_min;
	touch->max_y = y_max;
	return;
}
FINSH_FUNCTION_EXPORT(set_touch_min_max, x_min-x_max-y_min-y_max);
#endif




int get_touch_param(struct touch_calib_param *touch_param)
{
	return read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_TOUCH_PARAM, 0, touch_param);
}

int set_touch_param(struct touch_calib_param *touch_param)
{
	int ret;

	ret = write_syscfgdata_tbl(SYSCFGDATA_TBL_TOUCH_PARAM, 0, touch_param);
	//syscfgdata_syn_proc();

	return ret;
}


