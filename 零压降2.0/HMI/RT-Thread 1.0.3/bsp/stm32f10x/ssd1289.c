#include "ssd1289.h"

// Compatible list:
// ssd1289

//内联函数定义,用以提高性能
#ifdef __CC_ARM                			 /* ARM Compiler 	*/
#define lcd_inline   				static __inline
#elif defined (__ICCARM__)        		/* for IAR Compiler */
#define lcd_inline 					inline
#elif defined (__GNUC__)        		/* GNU GCC Compiler */
#define lcd_inline 					static __inline
#else
#define lcd_inline                  static
#endif

#define rw_data_prepare()               write_cmd(0x22)


/********* control ***********/
#include "stm32f10x.h"
#include "board.h"

//输出重定向.当不进行重定向时.
#define printf               rt_kprintf //使用rt_kprintf来输出
//#define printf(...)                       //无输出

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

static void delay(int cnt)
{
	volatile unsigned int dl;
	while(cnt--) {
		for(dl=0; dl<500; dl++);
	}
}

#if 1
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
#else
/*******************************************************************************
* Function Name  : LCD_DB_AS_InPut
* Description    : config MCU LCD_DB pins AS InPut
* Input          : None
* Output         : None
* Return         : None.
*******************************************************************************/
static void LCD_DB_AS_InPut(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	// DB15--0
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
}

/*******************************************************************************
* Function Name  : LCD_DB_AS_OutPut
* Description    : config MCU LCD_DB pins AS OutPut
* Input          : None
* Output         : None
* Return         : None.
*******************************************************************************/
static void LCD_DB_AS_OutPut(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	// DB15--0
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
}

#if 0
static u16 lcd_read_sr(void)
{
	u16 data;

	/* Write 16-bit Index, then Write Reg */
	SetRs;
	ClrRd;
	SetRd;
	data = LCD_Read();
	SetCs;

	return    data;
}

static void LCD_WriteCommand(u16 LCD_RegValue)
{
	/* Write 16-bit Index, then Write Reg */
	ClrCs
	ClrRs
	ClrWr
	LCD_Write(LCD_RegValue);
	SetWr
	SetCs
}
#endif

/*
 * write data to 'Index Register'
 */
void write_cmd(unsigned short cmd)
{
	/* Write 16-bit Index */
	ClrCs;
	ClrRs;
	ClrWr;
	LCD_Write(cmd);
	SetWr;
	SetRs;
	SetCs;

}

/*
 * read data from Data Bus
 */
unsigned short read_data(void)
{
	int temp;

	LCD_DB_AS_InPut();
	SetRs;

	ClrCs;
	ClrRd;
	temp = LCD_Read();
	SetRd;
	SetCs;

	return temp;
}

/*
 * write data to Data Bus
 */
void write_data(unsigned short data_code )
{
	LCD_DB_AS_OutPut();
	SetRs;

	ClrCs;
	ClrWr;
	LCD_Write(data_code);
	SetWr;
	SetCs;

	return;
}

void write_reg(unsigned char reg_addr,unsigned short reg_val)
{
	write_cmd(reg_addr);
	write_data(reg_val);
}

unsigned short read_reg(unsigned char reg_addr)
{
	unsigned short val=0;
	write_cmd(reg_addr);
	val = read_data();
	return (val);
}

#endif
/********* control <只移植以上函数即可> ***********/

static unsigned short deviceid=0;//设置一个静态变量用来保存LCD的ID

//static unsigned short BGR2RGB(unsigned short c)
//{
//    u16  r, g, b, rgb;
//
//    b = (c>>0)  & 0x1f;
//    g = (c>>5)  & 0x3f;
//    r = (c>>11) & 0x1f;
//
//    rgb =  (b<<11) + (g<<5) + (r<<0);
//
//    return( rgb );
//}

static void lcd_SetCursor(unsigned int x,unsigned int y)
{
#if 1==LCD_USE_VERTICAL_STROKE /* mark by David */
	/* 竖屏 */
	write_reg(0x004e,x);    /* 0-239 */
	write_reg(0x004f,y);    /* 0-319 */
#else
	/* 横屏 */
	write_reg(0x004e, y);    /* 0-239 */
	write_reg(0x004f, x);    /* 0-319 */

#endif
}

/* 读取指定地址的GRAM */
static unsigned short lcd_read_gram(unsigned int x,unsigned int y)
{
	volatile unsigned short temp;
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
//    /* [5:4]-ID~ID0 [3]-AM-1垂直-0水平 */
//    write_reg(0x0003,(1<<12)|(1<<5)|(1<<4) | (0<<3) );

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

	if( (temp1 == 0x5555) && (temp2 == 0xAAAA) ) {
		printf("data bus test pass!\r\n");
	} else {
		printf("data bus test error: 0x%x 0x%x\r\n",temp1,temp2);
	}
}

void ssd1289_init(void)
{
	deviceid = read_reg(0x00);

	/* deviceid check */
	if( deviceid != 0x8989 ) {
		printf("Invalid LCD ID:0x%x\r\n",deviceid);
		printf("Please check you hardware and configure.\r\n");
	} else {
		printf("\r\nLCD Device ID : 0x%x\r\n",deviceid);
	}

	// power supply setting
	// set R07h at 0021h (GON=1,DTE=0,D[1:0]=01)
	write_reg(0x0007,0x0021);
	// set R00h at 0001h (OSCEN=1)
	write_reg(0x0000,0x0001);
	// set R07h at 0023h (GON=1,DTE=0,D[1:0]=11)
	write_reg(0x0007,0x0023);
	// set R10h at 0000h (Exit sleep mode)
	write_reg(0x0010,0x0000);
	// Wait 30ms
	delay(3000);
	// set R07h at 0033h (GON=1,DTE=1,D[1:0]=11)
	write_reg(0x0007,0x0033);
	// Entry mode setting (R11h)
	// R11H Entry mode
	// vsmode DFM1 DFM0 TRANS OEDef WMode DMode1 DMode0 TY1 TY0 ID1 ID0 AM LG2 LG2 LG0
	//   0     1    1     0     0     0     0      0     0   1   1   1  *   0   0   0
	write_reg(0x0011,0x6058); /* mark by David */
	// LCD driver AC setting (R02h)
	write_reg(0x0002,0x0600);
	// power control 1
	// DCT3 DCT2 DCT1 DCT0 BT2 BT1 BT0 0 DC3 DC2 DC1 DC0 AP2 AP1 AP0 0
	// 1     0    1    0    1   0   0  0  1   0   1   0   0   1   0  0
	// DCT[3:0] fosc/4 BT[2:0]  DC{3:0] fosc/4
	write_reg(0x0003,0x0804);//0xA8A4
	write_reg(0x000C,0x0000);//
	write_reg(0x000D,0x0808);// 0x080C --> 0x0808
	// power control 4
	// 0 0 VCOMG VDV4 VDV3 VDV2 VDV1 VDV0 0 0 0 0 0 0 0 0
	// 0 0   1    0    1    0    1    1   0 0 0 0 0 0 0 0
	write_reg(0x000E,0x2900);
	write_reg(0x001E,0x00B8);
	/*
	 * 0x2B3F  ---- 10 1011 0011 1111
	 * write_reg(0x0001,0x2B3F);//驱动输出控制320*240  0x6B3F, mark by David
	 */
	write_reg(0x0001, 0x2B3F &(~(1<<9)));//驱动输出控制320*240  0x6B3F, mark by David

	write_reg(0x0010,0x0000);
	write_reg(0x0005,0x0000);
	write_reg(0x0006,0x0000);
	write_reg(0x0016,0xEF1C);
	write_reg(0x0017,0x0003);
	write_reg(0x0007,0x0233);//0x0233
	write_reg(0x000B,0x0000|(3<<6));
	write_reg(0x000F,0x0000);//扫描开始地址
	write_reg(0x0041,0x0000);
	write_reg(0x0042,0x0000);
	write_reg(0x0048,0x0000);
	write_reg(0x0049,0x013F);
	write_reg(0x004A,0x0000);
	write_reg(0x004B,0x0000);
	write_reg(0x0044,0xEF00);
	write_reg(0x0045,0x0000);
	write_reg(0x0046,0x013F);
	write_reg(0x0030,0x0707);
	write_reg(0x0031,0x0204);
	write_reg(0x0032,0x0204);
	write_reg(0x0033,0x0502);
	write_reg(0x0034,0x0507);
	write_reg(0x0035,0x0204);
	write_reg(0x0036,0x0204);
	write_reg(0x0037,0x0502);
	write_reg(0x003A,0x0302);
	write_reg(0x003B,0x0302);
	write_reg(0x0023,0x0000);
	write_reg(0x0024,0x0000);
	write_reg(0x0025,0x8000);   // 65hz
	write_reg(0x004f,0);        // 行首址0
	write_reg(0x004e,0);        // 列首址0

	//数据总线测试,用于测试硬件连接是否正常.
	lcd_data_bus_test();
	//GRAM测试,此测试可以测试LCD控制器内部GRAM.测试通过保证硬件正常
//    lcd_gram_test();

	//清屏
	lcd_clear( Blue );
}

/*  设置像素点 颜色,X,Y */
void ssd1289_lcd_set_pixel(const char* pixel, int x, int y)
{
	lcd_SetCursor(x,y);

	rw_data_prepare();
	write_data(*(rt_uint16_t*)pixel);
}

/* 获取像素点颜色 */
void ssd1289_lcd_get_pixel(char* pixel, int x, int y)
{
	*(rt_uint16_t*)pixel = lcd_read_gram(x, y);
}

/* 画水平线 */
void ssd1289_lcd_draw_hline(const char* pixel, int x1, int x2, int y)
{
	/* [5:4]-ID~ID0 [3]-AM-1垂直-0水平 */
#if 1==LCD_USE_VERTICAL_STROKE
	write_reg(0x0011, 0x6030 | (0<<3)); // AM=0 hline
#else
	write_reg(0x0011, 0x6030 | (1<<3)); // AM=0 hline
#endif

	lcd_SetCursor(x1, y);
	rw_data_prepare(); /* Prepare to write GRAM */
	while (x1 < x2) {
		write_data(*(rt_uint16_t*)pixel);
		x1++;
	}
}

/* 垂直线 */
void ssd1289_lcd_draw_vline(const char* pixel, int x, int y1, int y2)
{
	/* [5:4]-ID~ID0 [3]-AM-1垂直-0水平 */
#if 1==LCD_USE_VERTICAL_STROKE
	write_reg(0x0011,0x6070 | (1<<3)); // AM=0 vline
#else
	write_reg(0x0011,0x6070 | (0<<3)); // AM=0 vline
#endif

	lcd_SetCursor(x, y1);
	rw_data_prepare(); /* Prepare to write GRAM */
	while (y1 < y2) {
		write_data(*(rt_uint16_t*)pixel);
		y1++;
	}
}

/* blit a line */
void ssd1289_lcd_blit_line(const char* pixels, int x, int y, rt_size_t size)
{
	rt_uint16_t *ptr;

	ptr = (rt_uint16_t*)pixels;

	/* [5:4]-ID~ID0 [3]-AM-1垂直-0水平 */
#if 1==LCD_USE_VERTICAL_STROKE
	write_reg(0x0011,0x6070 | (0<<3)); // AM=0 hline
#else
	write_reg(0x0011,0x6070 | (1<<3)); // AM=0 hline
#endif

	lcd_SetCursor(x, y);
	rw_data_prepare(); /* Prepare to write GRAM */
	while (size) {
		write_data(*ptr ++);
		size --;
	}
}

struct rt_device_graphic_ops ssd1289_ops = {
	ssd1289_lcd_set_pixel,
	ssd1289_lcd_get_pixel,
	ssd1289_lcd_draw_hline,
	ssd1289_lcd_draw_vline,
	ssd1289_lcd_blit_line
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
	rt_thread_delay(1);
#endif
	/* register lcd device */
	_lcd_device.type  = RT_Device_Class_Graphic;
	_lcd_device.init  = lcd_init;
	_lcd_device.open  = lcd_open;
	_lcd_device.close = lcd_close;
	_lcd_device.control = lcd_control;
	_lcd_device.read  = RT_NULL;
	_lcd_device.write = RT_NULL;

	_lcd_device.user_data = &ssd1289_ops;
	ssd1289_init();

	/* register graphic device driver */
	rt_device_register(&_lcd_device, "lcd",
					   RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE);
}

