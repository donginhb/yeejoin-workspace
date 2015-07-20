#include "ili_lcd_general.h"

// Compatible list:
// ili9320 ili9325 ili9328
// LG4531

//内联函数定义,用以提高性能
#ifdef __CC_ARM                			 /* ARM Compiler 	*/
#define lcd_inline   				static __inline
#elif defined (__ICCARM__)        		/* for IAR Compiler */
#define lcd_inline 					inline
#elif defined (__GNUC__)        		/* GNU GCC Compiler */
#define lcd_inline 					static __inline
#else
#define lcd_inline                 static
#endif

#define rw_data_prepare()               write_cmd(0x22)


/********* control ***********/
#include "stm32f10x.h"
#include "board.h"

//输出重定向.当不进行重定向时.
#define printf               rt_kprintf //使用rt_kprintf来输出

#if 0
/* LCD is connected to the FSMC_Bank1_NOR/SRAM2 and NE2 is used as ship select signal */
/* RS <==> A2 */
#define LCD_REG              (*((volatile unsigned short *) 0x64000000)) /* RS = 0 */
#define LCD_RAM              (*((volatile unsigned short *) 0x64000008)) /* RS = 1 */
#else
/* LCD is connected to the FSMC_Bank1_NOR/SRAM1 and NE1 is used as ship select signal */
/* RS <==> A16 */
#define LCD_REG              (*((volatile unsigned short *) 0x60000000)) /* RS = 0 */
#define LCD_RAM              (*((volatile unsigned short *) 0x60020000)) /* RS = 1 */
#endif
#define LCD_USE_VERTICAL_STROKE 0
//#define _ILI_REVERSE_DIRECTION_
#define LCDC_ENTRY_MODE_SET_BASE (1<<12)

#define LCDC_ENTRY_MODE_HDVD (0<<5 | 0<<4)
#define LCDC_ENTRY_MODE_HIVD (0<<5 | 1<<4)
#define LCDC_ENTRY_MODE_HDVI (1<<5 | 0<<4)
#define LCDC_ENTRY_MODE_HIVI (1<<5 | 1<<4)

#define LCDC_ENTRY_MODE_HV_DI LCDC_ENTRY_MODE_HIVD

#define LCDC_ENTRY_MODE_HLINE_HV_DI LCDC_ENTRY_MODE_HDVD
#define LCDC_ENTRY_MODE_VLINE_HV_DI LCDC_ENTRY_MODE_HDVD
#define LCDC_ENTRY_MODE_BLIT_HV_DI  LCDC_ENTRY_MODE_HIVI


static void delay(int cnt)
{
	volatile unsigned int dl;
	while(cnt--) {
		for(dl=0; dl<500; dl++);
	}
}

lcd_inline void write_cmd(unsigned short cmd)
{
	LCD_REG = cmd;
}

lcd_inline unsigned short read_data(void)
{
	return LCD_RAM;
}

lcd_inline void write_data(unsigned short data_code )
{
	LCD_RAM = data_code;
}

lcd_inline void write_reg(unsigned char reg_addr,unsigned short reg_val)
{
	write_cmd(reg_addr);
	write_data(reg_val);
}

lcd_inline unsigned short read_reg(unsigned char reg_addr)
{
	unsigned short val=0;
	write_cmd(reg_addr);
	val = read_data();
	return (val);
}


/********* control <只移植以上函数即可> ***********/

static unsigned short deviceid=0;//设置一个静态变量用来保存LCD的ID

//返回LCD的ID
unsigned int lcd_getdeviceid(void)
{
	return deviceid;
}

static unsigned short BGR2RGB(unsigned short c)
{
	u16  r, g, b, rgb;

	b = (c>>0)  & 0x1f;
	g = (c>>5)  & 0x3f;
	r = (c>>11) & 0x1f;

	rgb =  (b<<11) + (g<<5) + (r<<0);

	return( rgb );
}

static void lcd_SetCursor(unsigned int x,unsigned int y)
{
#if 1==LCD_USE_VERTICAL_STROKE /* mark by David */
	/* 竖屏 */
	write_reg(32, x);    /* 0-239 */
	write_reg(33, y);    /* 0-319 */
#else
	/* 横屏 */
#if 0
	write_reg(32, y);    /* 0-239 */
	write_reg(33, x);    /* 0-319 */
#else /* 旋转180 */
	write_reg(32, 239-y);    /* 0-239 */
	write_reg(33, 319-x);    /* 0-319 */
#endif
#endif
}


/* 读取指定地址的GRAM */
static unsigned short lcd_read_gram(unsigned int x,unsigned int y)
{
	unsigned short temp;
	lcd_SetCursor(x,y);
	rw_data_prepare();
	/* dummy read */
	temp = read_data();
	temp = read_data();
	return temp;
}

static void lcd_clear(unsigned short Color)
{
	unsigned int index=0;
	lcd_SetCursor(0,0);
	rw_data_prepare();                      /* Prepare to write GRAM */
	for (index=0; index<(LCD_WIDTH*LCD_HEIGHT); index++) {
		write_data(Color);
	}
}

static void lcd_data_bus_test(void)
{
	unsigned short temp1;
	unsigned short temp2;

	/* wirte */
	lcd_SetCursor(0,0);
	rw_data_prepare();
	write_data(0x5555);

	lcd_SetCursor(1,0);
	rw_data_prepare();
	write_data(0xAAAA);

	/* read */
	lcd_SetCursor(0,0);
	temp1 = lcd_read_gram(0,0);

	lcd_SetCursor(1,0);
	temp2 = lcd_read_gram(1,0);

	if ((deviceid ==0x9325) || (deviceid ==0x9328) || (deviceid ==0x9320)) {
		temp1 = BGR2RGB( temp1 );
		temp2 = BGR2RGB( temp2 );
	}

	if( (temp1 == 0x5555) && (temp2 == 0xAAAA) ) {
		printf("data bus test pass!\r\n");
	} else {
		printf("data bus test error: 0x%x 0x%x\r\n",temp1,temp2);
	}
}

void Delay(__IO u32 nCount)
{
	__IO  u32 TimingDelay;
	while(nCount--) {
		for(TimingDelay=0; TimingDelay<1000; TimingDelay++);
	}
}


/*
 * ili9320
 * Driver Output Control (R01h)
 * Entry Mode (R03h)
 */
void ili_lcdc_init(void)
{
	//lcd_port_init();
	deviceid = read_reg(0x00);

	/* deviceid check */
	if( (deviceid != 0x4531) && (deviceid != 0x7783) && (deviceid != 0x9320) &&
		(deviceid != 0x9325) && (deviceid != 0x9328) && (deviceid != 0x9300)) {
		printf("Invalid LCD ID:0x%x\n", deviceid);
		printf("Please check you hardware and configure.\n");
		//while(1);
		return ;
	} else {
		printf("\nLCD Device ID : 0x%x\n", deviceid);
	}

	if (deviceid==0x9325|| deviceid==0x9328) {
		write_reg(0xe7,0x0010);
		write_reg(0x00,0x0001);  			        //start internal osc
#if defined(_ILI_REVERSE_DIRECTION_)
		write_reg(0x01,0x0000);                    //Reverse Display
#else
		write_reg(0x01,0x0100);                    //
#endif
		write_reg(0x02,0x0700); 				    //power on sequence
		/* [5:4]-ID1~ID0 [3]-AM-1垂直-0水平 */
		write_reg(0x03,(1<<12)|(1<<5)|(0<<4) | (1<<3) );
		write_reg(0x04,0x0000);
		write_reg(0x08,0x0207);
		write_reg(0x09,0x0000);
		write_reg(0x0a,0x0000); 				//display setting
		write_reg(0x0c,0x0001);				//display setting
		write_reg(0x0d,0x0000); 				//0f3c
		write_reg(0x0f,0x0000);
		//Power On sequence //
		write_reg(0x10,0x0000);
		write_reg(0x11,0x0007);
		write_reg(0x12,0x0000);
		write_reg(0x13,0x0000);
		delay(15);
		write_reg(0x10,0x1590);
		write_reg(0x11,0x0227);
		delay(15);
		write_reg(0x12,0x009c);
		delay(15);
		write_reg(0x13,0x1900);
		write_reg(0x29,0x0023);
		write_reg(0x2b,0x000e);
		delay(15);
		write_reg(0x20,0x0000);
		write_reg(0x21,0x0000);
		delay(15);
		write_reg(0x30,0x0007);
		write_reg(0x31,0x0707);
		write_reg(0x32,0x0006);
		write_reg(0x35,0x0704);
		write_reg(0x36,0x1f04);
		write_reg(0x37,0x0004);
		write_reg(0x38,0x0000);
		write_reg(0x39,0x0706);
		write_reg(0x3c,0x0701);
		write_reg(0x3d,0x000f);
		delay(15);
		write_reg(0x50,0x0000);
		write_reg(0x51,0x00ef);
		write_reg(0x52,0x0000);
		write_reg(0x53,0x013f);
#if defined(_ILI_REVERSE_DIRECTION_)
		write_reg(0x60,0x2700);
#else
		write_reg(0x60,0xA700);
#endif
		write_reg(0x61,0x0001);
		write_reg(0x6a,0x0000);
		write_reg(0x80,0x0000);
		write_reg(0x81,0x0000);
		write_reg(0x82,0x0000);
		write_reg(0x83,0x0000);
		write_reg(0x84,0x0000);
		write_reg(0x85,0x0000);
		write_reg(0x90,0x0010);
		write_reg(0x92,0x0000);
		write_reg(0x93,0x0003);
		write_reg(0x95,0x0110);
		write_reg(0x97,0x0000);
		write_reg(0x98,0x0000);
		//display on sequence
		write_reg(0x07,0x0133);
		write_reg(0x20,0x0000);
		write_reg(0x21,0x0000);

	} else if( deviceid==0x9320 || deviceid==0x9300) {

		write_reg(0x00,0x0000);
#if defined(_ILI_REVERSE_DIRECTION_)
		write_reg(0x01, 0x0100);  //Reverse Display
		write_reg(0x60, 0xA700);
#else
		write_reg(0x01, 0x0000);
		write_reg(0x60, 0x2700);
#endif
		write_reg(0x02,0x0700);	//LCD Driver Waveform Contral.
#if 1
		/* Entry Mode Set 0x1030, 0x1018 */
#if 1==LCD_USE_VERTICAL_STROKE
		write_reg(0x03, LCDC_ENTRY_MODE_SET_BASE | LCDC_ENTRY_MODE_HV_DI | (1<<3) );
#else
		write_reg(0x03, LCDC_ENTRY_MODE_SET_BASE | LCDC_ENTRY_MODE_HV_DI | (0<<3) );
#endif
#endif
		write_reg(0x04,0x0000);	//Scalling Contral.
		write_reg(0x08,0x0202);	//Display Contral 2.(0x0207)
		write_reg(0x09,0x0000);	//Display Contral 3.(0x0000)
		write_reg(0x0a,0x0000);	//Frame Cycle Contal.(0x0000)
		write_reg(0x0c,(1<<0));	//Extern Display Interface Contral 1.(0x0000)
		write_reg(0x0d,0x0000);	//Frame Maker Position.
		write_reg(0x0f,0x0000);	//Extern Display Interface Contral 2.

		delay(15);
		write_reg(0x07,0x0101);	//Display Contral.
		delay(15);

		write_reg(0x10,(1<<12)|(0<<8)|(1<<7)|(1<<6)|(0<<4));	//Power Control 1.(0x16b0)
		write_reg(0x11,0x0007);								//Power Control 2.(0x0001)
		write_reg(0x12,(1<<8)|(1<<4)|(0<<0));					//Power Control 3.(0x0138)
		write_reg(0x13,0x0b00);								//Power Control 4.
		write_reg(0x29,0x0000);								//Power Control 7.

		write_reg(0x2b,(1<<14)|(1<<4));

		write_reg(0x50,0);		//Set X Start.
		write_reg(0x51,239);	//Set X End.
		write_reg(0x52,0);		//Set Y Start.
		write_reg(0x53,319);	//Set Y End.

		write_reg(0x61,0x0001);	//Driver Output Control.
		write_reg(0x6a,0x0000);	//Vertical Srcoll Control.

		write_reg(0x80,0x0000);	//Display Position? Partial Display 1.
		write_reg(0x81,0x0000);	//RAM Address Start? Partial Display 1.
		write_reg(0x82,0x0000);	//RAM Address End-Partial Display 1.
		write_reg(0x83,0x0000);	//Displsy Position? Partial Display 2.
		write_reg(0x84,0x0000);	//RAM Address Start? Partial Display 2.
		write_reg(0x85,0x0000);	//RAM Address End? Partial Display 2.

		write_reg(0x90,(0<<7)|(16<<0));	//Frame Cycle Contral.(0x0013)
		write_reg(0x92,0x0000);	//Panel Interface Contral 2.(0x0000)
		write_reg(0x93,0x0001);	//Panel Interface Contral 3.
		write_reg(0x95,0x0110);	//Frame Cycle Contral.(0x0110)
		write_reg(0x97,(0<<8));	//
		write_reg(0x98,0x0000);	//Frame Cycle Contral.

		write_reg(0x07,0x0133);	//(0x0173)
	} else if( deviceid==0x4531 ) {
		// Setup display
		write_reg(0x00,0x0001);
		write_reg(0x10,0x0628);
		write_reg(0x12,0x0006);
		write_reg(0x13,0x0A32);
		write_reg(0x11,0x0040);
		write_reg(0x15,0x0050);
		write_reg(0x12,0x0016);
		delay(15);
		write_reg(0x10,0x5660);
		delay(15);
		write_reg(0x13,0x2A4E);
#if defined(_ILI_REVERSE_DIRECTION_)
		write_reg(0x01,0x0100);
#else
		write_reg(0x01,0x0000);
#endif
		write_reg(0x02,0x0300);

		write_reg(0x03,0x1030);
//	    write_reg(0x03,0x1038);

		write_reg(0x08,0x0202);
		write_reg(0x0A,0x0000);
		write_reg(0x30,0x0000);
		write_reg(0x31,0x0402);
		write_reg(0x32,0x0106);
		write_reg(0x33,0x0700);
		write_reg(0x34,0x0104);
		write_reg(0x35,0x0301);
		write_reg(0x36,0x0707);
		write_reg(0x37,0x0305);
		write_reg(0x38,0x0208);
		write_reg(0x39,0x0F0B);
		delay(15);
		write_reg(0x41,0x0002);

#if defined(_ILI_REVERSE_DIRECTION_)
		write_reg(0x60,0x2700);
#else
		write_reg(0x60,0xA700);
#endif

		write_reg(0x61,0x0001);
		write_reg(0x90,0x0119);
		write_reg(0x92,0x010A);
		write_reg(0x93,0x0004);
		write_reg(0xA0,0x0100);
//	    write_reg(0x07,0x0001);
		delay(15);
//	    write_reg(0x07,0x0021);
		delay(15);
//	    write_reg(0x07,0x0023);
		delay(15);
//	    write_reg(0x07,0x0033);
		delay(15);
		write_reg(0x07,0x0133);
		delay(15);
		write_reg(0xA0,0x0000);
		delay(20);
	} else if( deviceid ==0x7783) {
		// Start Initial Sequence
		write_reg(0xFF,0x0001);
		write_reg(0xF3,0x0008);
		write_reg(0x01,0x0100);
		write_reg(0x02,0x0700);
		write_reg(0x03,0x1030);  //0x1030
		write_reg(0x08,0x0302);
		write_reg(0x08,0x0207);
		write_reg(0x09,0x0000);
		write_reg(0x0A,0x0000);
		write_reg(0x10,0x0000);  //0x0790
		write_reg(0x11,0x0005);
		write_reg(0x12,0x0000);
		write_reg(0x13,0x0000);
		delay(20);
		write_reg(0x10,0x12B0);
		delay(20);
		write_reg(0x11,0x0007);
		delay(20);
		write_reg(0x12,0x008B);
		delay(20);
		write_reg(0x13,0x1700);
		delay(20);
		write_reg(0x29,0x0022);

		//################# void Gamma_Set(void) ####################//
		write_reg(0x30,0x0000);
		write_reg(0x31,0x0707);
		write_reg(0x32,0x0505);
		write_reg(0x35,0x0107);
		write_reg(0x36,0x0008);
		write_reg(0x37,0x0000);
		write_reg(0x38,0x0202);
		write_reg(0x39,0x0106);
		write_reg(0x3C,0x0202);
		write_reg(0x3D,0x0408);
		delay(20);
		write_reg(0x50,0x0000);
		write_reg(0x51,0x00EF);
		write_reg(0x52,0x0000);
		write_reg(0x53,0x013F);
		write_reg(0x60,0xA700);
		write_reg(0x61,0x0001);
		write_reg(0x90,0x0033);
		write_reg(0x2B,0x000B);
		write_reg(0x07,0x0133);
		delay(20);
	}

	//数据总线测试,用于测试硬件连接是否正常.
	lcd_data_bus_test();

	//清屏
	lcd_clear( Blue ); /* mark by David */
	//printf("r03h:0x%x, r01h:0x%x, r60h:0x%x,\n", read_reg(3), read_reg(1), read_reg(0x60));
}

/*  设置像素点 颜色,X,Y */
void rt_hw_lcd_set_pixel(const char* pixel, int x, int y)
{
	lcd_SetCursor(x,y);

	rw_data_prepare();
	write_data(*(rt_uint16_t*)pixel);
}

/* 获取像素点颜色 */
void rt_hw_lcd_get_pixel(char* pixel, int x, int y)
{
	*(rt_uint16_t*)pixel = BGR2RGB( lcd_read_gram(x,y) );
}

/* 画水平线 */
void rt_hw_lcd_draw_hline(const char* pixel, int x1, int x2, int y)
{
	/* [5:4]-ID~ID0 [3]-AM-1垂直-0水平 */
#if 1==LCD_USE_VERTICAL_STROKE
	write_reg(0x03, LCDC_ENTRY_MODE_SET_BASE | LCDC_ENTRY_MODE_HLINE_HV_DI | (0<<3) );
#else
	write_reg(0x03, LCDC_ENTRY_MODE_SET_BASE | LCDC_ENTRY_MODE_HLINE_HV_DI | (1<<3) );
#endif

	lcd_SetCursor(x1, y);
	rw_data_prepare(); /* Prepare to write GRAM */
	while (x1 < x2) {
		write_data( *(rt_uint16_t*)pixel );
		x1++;
	}
}

/* 垂直线 */
void rt_hw_lcd_draw_vline(const char* pixel, int x, int y1, int y2)
{
	/* [5:4]-ID~ID0 [3]-AM-1垂直-0水平 */
#if 1==LCD_USE_VERTICAL_STROKE
	write_reg(0x03, LCDC_ENTRY_MODE_SET_BASE | LCDC_ENTRY_MODE_VLINE_HV_DI | (1<<3) );
#else
#if 0
	write_reg(0x03, LCDC_ENTRY_MODE_SET_BASE | LCDC_ENTRY_MODE_VLINE_HV_DI | (0<<3) );
#else
	write_reg(0x03, LCDC_ENTRY_MODE_SET_BASE | LCDC_ENTRY_MODE_VLINE_HV_DI | (0<<3) );
#endif
#endif

	lcd_SetCursor(x, y1);
	rw_data_prepare(); /* Prepare to write GRAM */
	while (y1 < y2) {
		write_data( *(rt_uint16_t*)pixel );
		y1++;
	}
}

/* ?? */
void rt_hw_lcd_draw_blit_line(const char* pixels, int x, int y, rt_size_t size)
{
	rt_uint16_t *ptr;

	ptr = (rt_uint16_t*)pixels;

	/* [5:4]-ID~ID0 [3]-AM-1垂直-0水平 */
#if 1==LCD_USE_VERTICAL_STROKE
	write_reg(0x03, LCDC_ENTRY_MODE_SET_BASE | LCDC_ENTRY_MODE_BLIT_HV_DI | (0<<3) );
#else
	write_reg(0x03, LCDC_ENTRY_MODE_SET_BASE | LCDC_ENTRY_MODE_BLIT_HV_DI | (1<<3) );
#endif

	lcd_SetCursor(x, y);
	rw_data_prepare(); /* Prepare to write GRAM */
	while (size) {
		write_data(*ptr ++);
		size --;
	}
}

struct rt_device_graphic_ops lcd_ili_ops = {
	rt_hw_lcd_set_pixel,
	rt_hw_lcd_get_pixel,
	rt_hw_lcd_draw_hline,
	rt_hw_lcd_draw_vline,
	rt_hw_lcd_draw_blit_line
};

struct rt_device _lcd_device;
static rt_err_t lcd_init(rt_device_t dev)
{
	return RT_EOK;
}

static rt_err_t lcd_open(rt_device_t dev, rt_uint16_t oflag)
{
	return RT_EOK;
}

static rt_err_t lcd_close(rt_device_t dev)
{
	return RT_EOK;
}

static rt_err_t lcd_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
	switch (cmd) {
	case RTGRAPHIC_CTRL_GET_INFO: {
		struct rt_device_graphic_info *info;

		info = (struct rt_device_graphic_info*) args;
		RT_ASSERT(info != RT_NULL);

		info->bits_per_pixel = 16;
		info->pixel_format = RTGRAPHIC_PIXEL_FORMAT_RGB565P;
		info->framebuffer = RT_NULL;
		info->width = LCD_X_WIDTH;
		info->height = LCD_Y_HEIGHT;
	}
	break;

	case RTGRAPHIC_CTRL_RECT_UPDATE:
		/* nothong to be done */
		break;

	default:
		break;
	}

	return RT_EOK;
}


void rt_hw_lcd_init(void)
{
#if 0
	/* LCD RESET */
	/* PF10 : LCD RESET */
	{
		GPIO_InitTypeDef GPIO_InitStructure;

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);

		GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
		GPIO_Init(GPIOF,&GPIO_InitStructure);

		GPIO_ResetBits(GPIOF,GPIO_Pin_10);
		GPIO_SetBits(GPIOF,GPIO_Pin_10);
		/* wait for lcd reset */
		rt_thread_delay(1);
	}
#else
	rt_thread_delay(10);
	set_port_pin(lcd_reset_gpio, lcd_reset_pin);
#endif
	/* register lcd device */
	_lcd_device.type  = RT_Device_Class_Graphic;
	_lcd_device.init  = lcd_init;
	_lcd_device.open  = lcd_open;
	_lcd_device.close = lcd_close;
	_lcd_device.control = lcd_control;
	_lcd_device.read  = RT_NULL;
	_lcd_device.write = RT_NULL;

	_lcd_device.user_data = &lcd_ili_ops;
	ili_lcdc_init();

	/* register graphic device driver */
	rt_device_register(&_lcd_device, "lcd",
					   RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE);
}
