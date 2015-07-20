 /******************************************************************************
  * @file    ili9320.c
  * @author  www.armjishu.com 
  * @Update  www.armjishu.com 
  * @version V1.0
  * @date    03/16/2010
  * @brief   TFT Driver program
  ******************************************************************************/
  
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "ili9320.h"
#include "ili9320_font.h"
#include "ili9320_fontid.h"
#include <board.h>
#include <lcd_touch.h>

u16 DeviceIdCode;

  /*****************************
  **    硬件连接说明          **
  ** STM32         ili9320    **
  ** PE0~15 <----> DB0~15     **
  ** PD15   <----> nRD        **
  ** PD13   <----> RS         **
  ** PB14   <----> nWR        **
  ** PC8    <----> nCS        **
  ** Reset  <----> nReset     **
  ** VCC    <----> BK_LED     **
  ******************************/
  
/* Private typedef -----------------------------------------------------------*/
#if 0
static void LCD_DB_AS_InPut(void);
static void LCD_DB_AS_OutPut(void);
static void LCD_WriteReg(u16 LCD_Reg,u16 LCD_RegValue);
static u16 LCD_ReadReg(u8 LCD_Reg);
static u16 LCD_ReadSta(void);
//static void LCD_WriteCommand(u16 LCD_RegValue);
static void LCD_WriteRAM_Prepare(void);
static void LCD_WriteRAM(u16 RGB_Code);
static u16 LCD_ReadRAM(void);

static void Delay(__IO u32 nCount);
static void ili9320_Delay(vu32 nCount);
#endif
static void display_init_info(void);
static void fill_16x16_dot_matrix(u16 x, u16 y, char *fnt_mod, u16 charColor, u16 bkColor);

		

#if 0
/****************************************************************************
* 名    称：void ili9320_Initializtion()
* 功    能：初始化 ILI9320 控制器
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：ili9320_Initializtion();
* 作    者： www.armjishu.com TEAM
****************************************************************************/
void ili9320_Initializtion(void)
{
	u16 i;
#if 0
	u8 str[] = "www.yeejoin.com";
	u8 len = sizeof(str)-1;
	u16 StartX;
#endif
	LCD_WriteReg(0x0000,0x0001);  
	Delay(5); /* delay 50 ms */			//start internal osc
	LCD_DB_AS_InPut();
	Delay(1); /* delay 50 ms */			//start internal osc
	DeviceIdCode = LCD_ReadReg(0x0000);
	if(0 == DeviceIdCode)
		DeviceIdCode = LCD_ReadReg(0x0000);

	rt_kprintf("\n\r ###### DeviceIdCode = 0x%x ###### ", DeviceIdCode);

	if(DeviceIdCode == 0x0)
		DeviceIdCode=0x9320;

	LCD_DB_AS_OutPut();
	Delay(1); /* delay 50 ms */			//start internal osc

	if(DeviceIdCode == 0x8989)
		rt_kprintf("\n\r This LCD is SSD1289 ");

	if(DeviceIdCode == 0x9325)
		rt_kprintf("\n\r This LCD is ili9325 ");

	if(DeviceIdCode==0x9325 || DeviceIdCode==0x9328) {
		LCD_WriteReg(0x00e3,0x3008);
		LCD_WriteReg(0x00e7,0x0012);
		LCD_WriteReg(0x00ef,0x1231);//Set the internal vcore voltage
		// 		LCD_WriteReg(0x00e7,0x0010);      
		LCD_WriteReg(0x0000,0x0001);  			//start internal osc
		LCD_WriteReg(0x0001,0x0100);     
		LCD_WriteReg(0x0002,0x0700); 				//power on sequence                     
		LCD_WriteReg(0x0003,(1<<12)|(1<<5)|(1<<4) ); 	//65K 
		LCD_WriteReg(0x0004,0x0000);                                   
		LCD_WriteReg(0x0008,0x0207);	           
		LCD_WriteReg(0x0009,0x0000);         
		LCD_WriteReg(0x000a,0x0000); 				//display setting         
		LCD_WriteReg(0x000c,0x0001);				//display setting          
		LCD_WriteReg(0x000d,0x0000); 				//0f3c          
		LCD_WriteReg(0x000f,0x0000);
		//Power On sequence //
		LCD_WriteReg(0x0010,0x0000);   
		LCD_WriteReg(0x0011,0x0007);
		LCD_WriteReg(0x0012,0x0000);                                                                 
		LCD_WriteReg(0x0013,0x0000);                 
		for(i=50000;i>0;i--);
		for(i=50000;i>0;i--);
		LCD_WriteReg(0x0010,0x1590);   
		LCD_WriteReg(0x0011,0x0227);
		for(i=50000;i>0;i--);
		for(i=50000;i>0;i--);
		LCD_WriteReg(0x0012,0x009c);                  
		for(i=50000;i>0;i--);
		for(i=50000;i>0;i--);
		LCD_WriteReg(0x0013,0x1900);   
		LCD_WriteReg(0x0029,0x0023);
		LCD_WriteReg(0x002b,0x000e);
		for(i=50000;i>0;i--);
		for(i=50000;i>0;i--);
		LCD_WriteReg(0x0020,0x0000);                                                            
		LCD_WriteReg(0x0021,0x0000);           
		///////////////////////////////////////////////////////      
		for(i=50000;i>0;i--);
			for(i=50000;i>0;i--);
		LCD_WriteReg(0x0030,0x0007); 
		LCD_WriteReg(0x0031,0x0707);   
		LCD_WriteReg(0x0032,0x0006);
		LCD_WriteReg(0x0035,0x0704);
		LCD_WriteReg(0x0036,0x1f04); 
		LCD_WriteReg(0x0037,0x0004);
		LCD_WriteReg(0x0038,0x0000);        
		LCD_WriteReg(0x0039,0x0706);     
		LCD_WriteReg(0x003c,0x0701);
		LCD_WriteReg(0x003d,0x000f);
		for(i=50000;i>0;i--);
			for(i=50000;i>0;i--);
		LCD_WriteReg(0x0050,0x0000);        
		LCD_WriteReg(0x0051,0x00ef);   
		LCD_WriteReg(0x0052,0x0000);     
		LCD_WriteReg(0x0053,0x013f);
		LCD_WriteReg(0x0060,0xa700);        
		LCD_WriteReg(0x0061,0x0001); 
		LCD_WriteReg(0x006a,0x0000);
		LCD_WriteReg(0x0080,0x0000);
		LCD_WriteReg(0x0081,0x0000);
		LCD_WriteReg(0x0082,0x0000);
		LCD_WriteReg(0x0083,0x0000);
		LCD_WriteReg(0x0084,0x0000);
		LCD_WriteReg(0x0085,0x0000);

		LCD_WriteReg(0x0090,0x0010);     
		LCD_WriteReg(0x0092,0x0600);  
		if (DeviceIdCode==0x9328) {   
		       LCD_WriteReg(0x0093,0x0003);
		       LCD_WriteReg(0x0095,0x0110);
		       LCD_WriteReg(0x0097,0x0000);        
		       LCD_WriteReg(0x0098,0x0000);  
		}
		//display on sequence     
		LCD_WriteReg(0x0007,0x0133);

		LCD_WriteReg(0x0020,0x0000);                                                            
		LCD_WriteReg(0x0021,0x0000);
	} else if(DeviceIdCode==0x9320||DeviceIdCode==0x9300) {
		rt_kprintf("\n\r This LCD is ili%x.", DeviceIdCode);
		LCD_WriteReg(0x00,0x0001);
		LCD_WriteReg(0x01,0x0100);	//Driver Output Contral. 0x0100
		LCD_WriteReg(0x02,0x0700);	//LCD Driver Waveform Contral.
		LCD_WriteReg(0x03,0x1018);	//Entry Mode Set. 0x1018 0x1030
		//LCD_WriteReg(0x03,0x1018);	//Entry Mode Set.

		LCD_WriteReg(0x04,0x0000);	//Scalling Contral.
		LCD_WriteReg(0x08,0x0202);	//Display Contral 2.(0x0207)
		LCD_WriteReg(0x09,0x0000);	//Display Contral 3.(0x0000)
		LCD_WriteReg(0x0a,0x0000);	//Frame Cycle Contal.(0x0000)
		LCD_WriteReg(0x0c,(1<<0));	//Extern Display Interface Contral 1.(0x0000)
		LCD_WriteReg(0x0d,0x0000);	//Frame Maker Position.
		LCD_WriteReg(0x0f,0x0000);	//Extern Display Interface Contral 2.

		for(i=50000;i>0;i--);
		LCD_WriteReg(0x07,0x0101);	//Display Contral.
		for(i=50000;i>0;i--);

		LCD_WriteReg(0x10,(1<<12)|(0<<8)|(1<<7)|(1<<6)|(0<<4));	//Power Control 1.(0x16b0)
		LCD_WriteReg(0x11,0x0007);			//Power Control 2.(0x0001)
		LCD_WriteReg(0x12,(1<<8)|(1<<4)|(0<<0));	//Power Control 3.(0x0138)
		LCD_WriteReg(0x13,0x0b00);			//Power Control 4.
		LCD_WriteReg(0x29,0x0000);			//Power Control 7.

		LCD_WriteReg(0x2b,(1<<14)|(1<<4));
		
		LCD_WriteReg(0x50,0);		//Set X Start.
		LCD_WriteReg(0x51,239);	//Set X End.
		LCD_WriteReg(0x52,0);		//Set Y Start.
		LCD_WriteReg(0x53,319);	//Set Y End.

		LCD_WriteReg(0x60,0x2700);	//Driver Output Control.
		LCD_WriteReg(0x61,0x0001);	//Driver Output Control.
		LCD_WriteReg(0x6a,0x0000);	//Vertical Srcoll Control.

		LCD_WriteReg(0x80,0x0000);	//Display Position? Partial Display 1.
		LCD_WriteReg(0x81,0x0000);	//RAM Address Start? Partial Display 1.
		LCD_WriteReg(0x82,0x0000);	//RAM Address End-Partial Display 1.
		LCD_WriteReg(0x83,0x0000);	//Displsy Position? Partial Display 2.
		LCD_WriteReg(0x84,0x0000);	//RAM Address Start? Partial Display 2.
		LCD_WriteReg(0x85,0x0000);	//RAM Address End? Partial Display 2.

		LCD_WriteReg(0x90,(0<<7)|(16<<0));	//Frame Cycle Contral.(0x0013)
		LCD_WriteReg(0x92,0x0000);	//Panel Interface Contral 2.(0x0000)
		LCD_WriteReg(0x93,0x0001);	//Panel Interface Contral 3.
		LCD_WriteReg(0x95,0x0110);	//Frame Cycle Contral.(0x0110)
		LCD_WriteReg(0x97,(0<<8));	//
		LCD_WriteReg(0x98,0x0000);	//Frame Cycle Contral.
		for(i=50000;i>0;i--);
		LCD_WriteReg(0x07,0x0173);	//(0x0173)
		for(i=50000;i>0;i--);
	} else if(DeviceIdCode==0x9331)	{
		LCD_WriteReg(0x00E7, 0x1014);
		LCD_WriteReg(0x0001, 0x0100); // set SS and SM bit   0x0100
		LCD_WriteReg(0x0002, 0x0200); // set 1 line inversion
		LCD_WriteReg(0x0003, 0x1030); // set GRAM write direction and BGR=1.     0x1030
		LCD_WriteReg(0x0008, 0x0202); // set the back porch and front porch
		LCD_WriteReg(0x0009, 0x0000); // set non-display area refresh cycle ISC[3:0]
		LCD_WriteReg(0x000A, 0x0000); // FMARK function
		LCD_WriteReg(0x000C, 0x0000); // RGB interface setting
		LCD_WriteReg(0x000D, 0x0000); // Frame marker Position
		LCD_WriteReg(0x000F, 0x0000); // RGB interface polarity
		//*************Power On sequence ****************//
		LCD_WriteReg(0x0010, 0x0000); // SAP, BT[3:0], AP, DSTB, SLP, STB
		LCD_WriteReg(0x0011, 0x0007); // DC1[2:0], DC0[2:0], VC[2:0]
		LCD_WriteReg(0x0012, 0x0000); // VREG1OUT voltage
		LCD_WriteReg(0x0013, 0x0000); // VDV[4:0] for VCOM amplitude
		ili9320_Delay(200); // Dis-charge capacitor power voltage
		LCD_WriteReg(0x0010, 0x1690); // SAP, BT[3:0], AP, DSTB, SLP, STB
		LCD_WriteReg(0x0011, 0x0227); // DC1[2:0], DC0[2:0], VC[2:0]
		ili9320_Delay(50); // Delay 50ms
		LCD_WriteReg(0x0012, 0x000C); // Internal reference voltage= Vci;
		ili9320_Delay(50); // Delay 50ms
		LCD_WriteReg(0x0013, 0x0800); // Set VDV[4:0] for VCOM amplitude
		LCD_WriteReg(0x0029, 0x0011); // Set VCM[5:0] for VCOMH
		LCD_WriteReg(0x002B, 0x000B); // Set Frame Rate
		ili9320_Delay(50); // Delay 50ms
		LCD_WriteReg(0x0020, 0x0000); // GRAM horizontal Address
		LCD_WriteReg(0x0021, 0x0000); // GRAM Vertical Address
		// ----------- Adjust the Gamma Curve ----------//
		LCD_WriteReg(0x0030, 0x0000);
		LCD_WriteReg(0x0031, 0x0106);
		LCD_WriteReg(0x0032, 0x0000);
		LCD_WriteReg(0x0035, 0x0204);
		LCD_WriteReg(0x0036, 0x160A);
		LCD_WriteReg(0x0037, 0x0707);
		LCD_WriteReg(0x0038, 0x0106);
		LCD_WriteReg(0x0039, 0x0707);
		LCD_WriteReg(0x003C, 0x0402);
		LCD_WriteReg(0x003D, 0x0C0F);
		//------------------ Set GRAM area ---------------//
		LCD_WriteReg(0x0050, 0x0000); // Horizontal GRAM Start Address
		LCD_WriteReg(0x0051, 0x00EF); // Horizontal GRAM End Address
		LCD_WriteReg(0x0052, 0x0000); // Vertical GRAM Start Address
		LCD_WriteReg(0x0053, 0x013F); // Vertical GRAM Start Address
		LCD_WriteReg(0x0060, 0x2700); // Gate Scan Line
		LCD_WriteReg(0x0061, 0x0001); // NDL,VLE, REV
		LCD_WriteReg(0x006A, 0x0000); // set scrolling line
		//-------------- Partial Display Control ---------//
		LCD_WriteReg(0x0080, 0x0000);
		LCD_WriteReg(0x0081, 0x0000);
		LCD_WriteReg(0x0082, 0x0000);
		LCD_WriteReg(0x0083, 0x0000);
		LCD_WriteReg(0x0084, 0x0000);
		LCD_WriteReg(0x0085, 0x0000);
		//-------------- Panel Control -------------------//
		LCD_WriteReg(0x0090, 0x0010);
		LCD_WriteReg(0x0092, 0x0600);
		LCD_WriteReg(0x0007,0x0021);		
		ili9320_Delay(50);
		LCD_WriteReg(0x0007,0x0061);
		ili9320_Delay(50);
		LCD_WriteReg(0x0007,0x0133);  // 262K color and display ON
		ili9320_Delay(50);
	} else if(DeviceIdCode==0x9919) {
		//*********POWER ON &RESET DISPLAY OFF
		LCD_WriteReg(0x28,0x0006);
		LCD_WriteReg(0x00,0x0001);
		LCD_WriteReg(0x10,0x0000);
		LCD_WriteReg(0x01,0x72ef);
		LCD_WriteReg(0x02,0x0600);
		LCD_WriteReg(0x03,0x6a38);
		LCD_WriteReg(0x11,0x6874);//70
		 
		//  RAM WRITE DATA MASK
		LCD_WriteReg(0x0f,0x0000); 
		//  RAM WRITE DATA MASK
		LCD_WriteReg(0x0b,0x5308); 
		LCD_WriteReg(0x0c,0x0003);
		LCD_WriteReg(0x0d,0x000a);
		LCD_WriteReg(0x0e,0x2e00);  //0030
		LCD_WriteReg(0x1e,0x00be);
		LCD_WriteReg(0x25,0x8000);
		LCD_WriteReg(0x26,0x7800);
		LCD_WriteReg(0x27,0x0078);
		LCD_WriteReg(0x4e,0x0000);
		LCD_WriteReg(0x4f,0x0000);
		LCD_WriteReg(0x12,0x08d9);
		// -----------------Adjust the Gamma Curve----//
		LCD_WriteReg(0x30,0x0000);	 //0007
		LCD_WriteReg(0x31,0x0104);	   //0203
		LCD_WriteReg(0x32,0x0100);		//0001
		LCD_WriteReg(0x33,0x0305);	  //0007
		LCD_WriteReg(0x34,0x0505);	  //0007
		LCD_WriteReg(0x35,0x0305);		 //0407
		LCD_WriteReg(0x36,0x0707);		 //0407
		LCD_WriteReg(0x37,0x0300);		  //0607
		LCD_WriteReg(0x3a,0x1200);		 //0106
		LCD_WriteReg(0x3b,0x0800);		 
		LCD_WriteReg(0x07,0x0033);
	} else if(DeviceIdCode==0x1505) {
		// second release on 3/5  ,luminance is acceptable,water wave appear during camera preview
		LCD_WriteReg(0x0007,0x0000);
		ili9320_Delay(5);
		LCD_WriteReg(0x0012,0x011C);//0x011A   why need to set several times?
		LCD_WriteReg(0x00A4,0x0001);//NVM
		LCD_WriteReg(0x0008,0x000F);
		LCD_WriteReg(0x000A,0x0008);
		LCD_WriteReg(0x000D,0x0008);

		//GAMMA CONTROL/
		LCD_WriteReg(0x0030,0x0707);
		LCD_WriteReg(0x0031,0x0007); //0x0707
		LCD_WriteReg(0x0032,0x0603); 
		LCD_WriteReg(0x0033,0x0700); 
		LCD_WriteReg(0x0034,0x0202); 
		LCD_WriteReg(0x0035,0x0002); //?0x0606
		LCD_WriteReg(0x0036,0x1F0F);
		LCD_WriteReg(0x0037,0x0707); //0x0f0f  0x0105
		LCD_WriteReg(0x0038,0x0000); 
		LCD_WriteReg(0x0039,0x0000); 
		LCD_WriteReg(0x003A,0x0707); 
		LCD_WriteReg(0x003B,0x0000); //0x0303
		LCD_WriteReg(0x003C,0x0007); //?0x0707
		LCD_WriteReg(0x003D,0x0000); //0x1313//0x1f08
		ili9320_Delay(5);
		LCD_WriteReg(0x0007,0x0001);
		LCD_WriteReg(0x0017,0x0001);   //Power supply startup enable
		ili9320_Delay(5);

		//power control//
		LCD_WriteReg(0x0010,0x17A0); 
		LCD_WriteReg(0x0011,0x0217); //reference voltage VC[2:0]   Vciout = 1.00*Vcivl
		LCD_WriteReg(0x0012,0x011E);//0x011c  //Vreg1out = Vcilvl*1.80   is it the same as Vgama1out ?
		LCD_WriteReg(0x0013,0x0F00); //VDV[4:0]-->VCOM Amplitude VcomL = VcomH - Vcom Ampl
		LCD_WriteReg(0x002A,0x0000);  
		LCD_WriteReg(0x0029,0x000A); //0x0001F  Vcomh = VCM1[4:0]*Vreg1out    gate source voltage??
		LCD_WriteReg(0x0012,0x013E); // 0x013C  power supply on
		   //Coordinates Control//
		LCD_WriteReg(0x0050,0x0000);//0x0e00
		LCD_WriteReg(0x0051,0x00EF); 
		LCD_WriteReg(0x0052,0x0000); 
		LCD_WriteReg(0x0053,0x013F); 
		//Pannel Image Control//
		LCD_WriteReg(0x0060,0x2700); 
		LCD_WriteReg(0x0061,0x0001); 
		LCD_WriteReg(0x006A,0x0000); 
		LCD_WriteReg(0x0080,0x0000); 
		//Partial Image Control//
		LCD_WriteReg(0x0081,0x0000); 
		LCD_WriteReg(0x0082,0x0000); 
		LCD_WriteReg(0x0083,0x0000); 
		LCD_WriteReg(0x0084,0x0000); 
		LCD_WriteReg(0x0085,0x0000); 
		//Panel Interface Control//
		LCD_WriteReg(0x0090,0x0013); 	//0x0010 frenqucy
		LCD_WriteReg(0x0092,0x0300); 
		LCD_WriteReg(0x0093,0x0005); 
		LCD_WriteReg(0x0095,0x0000); 
		LCD_WriteReg(0x0097,0x0000); 
		LCD_WriteReg(0x0098,0x0000); 

		LCD_WriteReg(0x0001,0x0100); 
		LCD_WriteReg(0x0002,0x0700); 
		LCD_WriteReg(0x0003,0x1030); 
		LCD_WriteReg(0x0004,0x0000); 
		LCD_WriteReg(0x000C,0x0000); 
		LCD_WriteReg(0x000F,0x0000); 
		LCD_WriteReg(0x0020,0x0000); 
		LCD_WriteReg(0x0021,0x0000); 
		LCD_WriteReg(0x0007,0x0021); 
		ili9320_Delay(20);
		LCD_WriteReg(0x0007,0x0061); 
		ili9320_Delay(20);
		LCD_WriteReg(0x0007,0x0173); 
		ili9320_Delay(20);
	} else if(DeviceIdCode==0x8989) {
		LCD_WriteReg(0x0000,0x0001);	//打开晶振
		LCD_WriteReg(0x0003,0xA8A4);	//0xA8A4
		LCD_WriteReg(0x000C,0x0000);        
		LCD_WriteReg(0x000D,0x080C);        
		LCD_WriteReg(0x000E,0x2B00);        
		LCD_WriteReg(0x001E,0x00B0);        
		LCD_WriteReg(0x0001,0x293F);	//驱动输出控制320*240  0x693F  0x2B3F
		LCD_WriteReg(0x0002,0x0600);   	//LCD Driving Waveform control
		LCD_WriteReg(0x0010,0x0000);     
		LCD_WriteReg(0x0011,0x6070);	//0x4030	//定义数据格式  16位色	横屏 0x6058
		LCD_WriteReg(0x0005,0x0000);     
		LCD_WriteReg(0x0006,0x0000);     
		LCD_WriteReg(0x0016,0xEF1C);     
		LCD_WriteReg(0x0017,0x0003);     
		LCD_WriteReg(0x0007,0x0233);	//0x0233       
		LCD_WriteReg(0x000B,0x0000);     
		LCD_WriteReg(0x000F,0x0000);	//扫描开始地址
		LCD_WriteReg(0x0041,0x0000);     
		LCD_WriteReg(0x0042,0x0000);     
		LCD_WriteReg(0x0048,0x0000);     
		LCD_WriteReg(0x0049,0x013F);     
		LCD_WriteReg(0x004A,0x0000);     
		LCD_WriteReg(0x004B,0x0000);     
		LCD_WriteReg(0x0044,0xEF00);     
		LCD_WriteReg(0x0045,0x0000);     
		LCD_WriteReg(0x0046,0x013F);     
		LCD_WriteReg(0x0030,0x0707);     
		LCD_WriteReg(0x0031,0x0204);     
		LCD_WriteReg(0x0032,0x0204);     
		LCD_WriteReg(0x0033,0x0502);     
		LCD_WriteReg(0x0034,0x0507);     
		LCD_WriteReg(0x0035,0x0204);     
		LCD_WriteReg(0x0036,0x0204);     
		LCD_WriteReg(0x0037,0x0502);     
		LCD_WriteReg(0x003A,0x0302);     
		LCD_WriteReg(0x003B,0x0302);     
		LCD_WriteReg(0x0023,0x0000);     
		LCD_WriteReg(0x0024,0x0000);     
		LCD_WriteReg(0x0025,0x8000);     
		LCD_WriteReg(0x004e,0);        //列(X)首址0
		LCD_WriteReg(0x004f,0);        //行(Y)首址0
	} else {
		rt_kprintf("\n\r ###### Err: Unknow DeviceIdCode 0x%x ###### ", DeviceIdCode);
	}

	ili9320_Delay(10);

	ili9320_Clear(Green);

#if 0
	StartX = 0; //(320 - 16*len)/2;
	for (i=0; i<6; i++) {
		ili9320_PutChar_cn_24x24((StartX+24*i), 0, FONT_ID_CN_BEI+i, Blue, HyalineBackColor);
	}

	for (i=0;i<len;i++) {
		ili9320_PutChar_16x24((StartX+16*i), 24, str[i], Yellow, HyalineBackColor);
	}
#else
	display_init_info();
#endif
	ili9320_Delay(2000);
}

/****************************************************************************
* 名    称：void ili9320_SetCursor(u16 x,u16 y)
* 功    能：设置屏幕座标
* 入口参数：x      行座标
*           y      列座标
* 出口参数：无
* 说    明：
* 调用方法：ili9320_SetCursor(10,10);
****************************************************************************/
void ili9320_SetCursor(u16 x,u16 y)
{
	if(DeviceIdCode==0x8989)
	{
	 	LCD_WriteReg(0x004e,y);        //行
    	//LCD_WriteReg(0x004f,0x13f-x);  //列
    	LCD_WriteReg(0x004f,x);  //列
	}
	else if((DeviceIdCode==0x9320))
	{
  		LCD_WriteReg(0x0020,y); // 行
  		LCD_WriteReg(0x0021,0x13f-x); // 列
	}
	else if((DeviceIdCode==0x9919))
	{
		LCD_WriteReg(0x004e,x); // 行
  		LCD_WriteReg(0x004f,y); // 列	
	}
    /*
	else if((DeviceIdCode==0x9325))
	{
		LCD_WriteReg(0x0020,x); // 行
  		LCD_WriteReg(0x0021,y); // 列	
	}
	*/
	else
	{
  		LCD_WriteReg(0x0020,y); // 行
  		LCD_WriteReg(0x0021,0x13f-x); // 列
	}
}

/****************************************************************************
* 名    称：void ili9320_SetWindows(u16 StartX,u16 StartY,u16 EndX,u16 EndY)
* 功    能：设置窗口区域
* 入口参数：StartX     行起始座标
*           StartY     列起始座标
*           EndX       行结束座标
*           EndY       列结束座标
* 出口参数：无
* 说    明：
* 调用方法：ili9320_SetWindows(0,0,100,100)；
****************************************************************************/
void ili9320_SetWindows(u16 StartX,u16 StartY,u16 EndX,u16 EndY)
{
  ili9320_SetCursor(StartX,StartY);
  LCD_WriteReg(0x0050, StartX);
  LCD_WriteReg(0x0052, StartY);
  LCD_WriteReg(0x0051, EndX);
  LCD_WriteReg(0x0053, EndY);
}

/****************************************************************************
* 名    称：void ili9320_Clear(u16 dat)
* 功    能：将屏幕填充成指定的颜色，如清屏，则填充 0xffff
* 入口参数：dat      填充值
* 出口参数：无
* 说    明：
* 调用方法：ili9320_Clear(0xffff);
****************************************************************************/
void ili9320_Clear(u16 Color)
{
  u32 index=0;
  ili9320_SetCursor(0,0); 
  LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */

  // 视频加速 www.armjishu.com
  ClrCs
  SetRs
  for(index=0;index<76800;index++)
   {
      ClrWr
      LCD_Write(Color);
      SetWr
   }
  SetCs
    
  /*
  for(index=0;index<76800;index++)
   {
     LCD_WriteRAM(Color);
   }
   */
}

/****************************************************************************
* 名    称：u16 ili9320_GetPoint(u16 x,u16 y)
* 功    能：获取指定座标的颜色值
* 入口参数：x      行座标
*           y      列座标
* 出口参数：当前座标颜色值
* 说    明：
* 调用方法：i=ili9320_GetPoint(10,10);
****************************************************************************/
u16 ili9320_GetPoint(u16 x,u16 y)
{
  ili9320_SetCursor(x,y);
  return (ili9320_BGR2RGB(LCD_ReadRAM()));
}


/****************************************************************************
* 名    称：void ili9320_SetPoint(u16 x,u16 y,u16 point)
* 功    能：在指定座标画点
* 入口参数：x      行座标
*           y      列座标
*           point  点的颜色
* 出口参数：无
* 说    明：
* 调用方法：ili9320_SetPoint(10,10,0x0fe0);
****************************************************************************/
void ili9320_SetPoint(u16 x,u16 y,u16 point)
{
  //if ( (x>320)||(y>240) ) return;
  ili9320_SetCursor(x,y);

  LCD_WriteRAM_Prepare();
  LCD_WriteRAM(point);
}


/****************************************************************************
* 名    称：void ili9320_DrawPicture(u16 StartX,u16 StartY,u16 EndX,u16 EndY,u16 *pic)
* 功    能：在指定座标范围显示一副图片
* 入口参数：StartX     行起始座标
*           StartY     列起始座标
*           EndX       行结束座标
*           EndY       列结束座标
            pic        图片头指针
* 出口参数：无
* 说    明：图片取模格式为水平扫描，16位颜色模式
* 调用方法：ili9320_DrawPicture(0,0,100,100,(u16*)demo);
* 作    者： www.armjishu.com
****************************************************************************/
void ili9320_DrawPicture(u16 StartX,u16 StartY,u16 EndX,u16 EndY,u16 *pic)
{
  u32  i, total;
  u16 data1,data2,data3;
  u16 *picturepointer = pic;
  u16 x,y;
  
  printf_syn("ili9320_DrawPicture StartX %d StartY %d EndX %d EndY %d \n\r", StartX, StartY, EndX, EndY);

  x=StartX;
  y=StartY;
  
  total = (EndX - StartX + 1)*(EndY - StartY + 1 )/2;

  for (i=0;i<total;i++)
  {
      data1 = *picturepointer++;
      data2 = *picturepointer++;
      data3 = ((data1 >>3)& 0x001f) |((data1>>5) & 0x07E0) | ((data2<<8) & 0xF800);
      ili9320_SetPoint(x,y,data3);
      y++;
      if(y > EndY)
      {
          x++;
          y=StartY;
      }


      data1 = data2;
      data2 = *picturepointer++;
      data3 = ((data1 >>11)& 0x001f) |((data2<<3) & 0x07E0) | ((data2) & 0xF800);
      ili9320_SetPoint(x,y,data3);
      y++;
      if(y > EndY)
      {
          x++;
          y=StartY;
      }
  }

}

/****************************************************************************
* 名    称：void ili9320_DrawPicture(u16 StartX,u16 StartY,u16 EndX,u16 EndY,u16 *pic)
* 功    能：在指定座标范围显示一副图片
* 入口参数：StartX     行起始座标
*           StartY     列起始座标
*           EndX       行结束座标
*           EndY       列结束座标
            pic        图片头指针
* 出口参数：无
* 说    明：图片取模格式为水平扫描，16位颜色模式
* 调用方法：ili9320_DrawPicture(0,0,100,100,(u16*)demo);
* 作    者： www.armjishu.com
****************************************************************************/
#if 0
void ili9320_DrawPicture(u16 StartX,u16 StartY,u16 EndX,u16 EndY,u16 *pic)
{
  u32  i, total;
  u16 data1,data2,data3;
  u16 *picturepointer = pic;
  //ili9320_SetWindows(StartX,StartY,EndX,EndY);

  LCD_WriteReg(0x0003,(1<<12)|(0<<5)|(1<<4) ); 

  ili9320_SetCursor(StartX,StartY);
  
  LCD_WriteRAM_Prepare();
  total = (EndX + 1)*(EndY + 1 ) / 2;
  for (i=0;i<total;i++)
  {
      data1 = *picturepointer++;
      data2 = *picturepointer++;
      data3 = ((data1 >>3)& 0x001f) |((data1>>5) & 0x07E0) | ((data2<<8) & 0xF800);
      LCD_WriteRAM(data3);
      data1 = data2;
      data2 = *picturepointer++;
      data3 = ((data1 >>11)& 0x001f) |((data2<<3) & 0x07E0) | ((data2) & 0xF800);
      LCD_WriteRAM(data3);
  }

  LCD_WriteReg(0x0003,(1<<12)|(1<<5)|(1<<4) ); 
}
#endif 

/****************************************************************************
* 名    称：void ili9320_PutChar(u16 x,u16 y,u8 c,u16 charColor,u16 bkColor)
* 功    能：在指定座标显示一个8x16点阵的ascii字符
* 入口参数：x          行座标
*           y          列座标
*           charColor  字符的颜色
*           bkColor    字符背景颜色
* 出口参数：无
* 说    明：显示范围限定为可显示的ascii码
* 调用方法：ili9320_PutChar(10,10,'a',0x0000,0xffff);
****************************************************************************/
void ili9320_PutChar(u16 x,u16 y,u8 c,u16 charColor,u16 bkColor)  // Lihao
{
  u16 i=0;
  u16 j=0;
  
  u8 tmp_char=0;
  
  if(HyalineBackColor == bkColor)
  {
    for (i=0;i<16;i++)
    {
      tmp_char=ascii_8x16[((c-0x20)*16)+i];
      for (j=0;j<8;j++)
      {
        if ( ((tmp_char >> (7-j)) & 0x01) == 0x01)
          {
            ili9320_SetPoint(x+j,y+i,charColor); // 字符颜色
          }
          else
          {
            // do nothing // 透明背景
          }
      }
    }
  }
  else
  {
        for (i=0;i<16;i++)
    {
      tmp_char=ascii_8x16[((c-0x20)*16)+i];
      for (j=0;j<8;j++)
      {
        if ( ((tmp_char >> (7-j)) & 0x01) == 0x01)
          {
            ili9320_SetPoint(x+j,y+i,charColor); // 字符颜色
          }
          else
          {
            ili9320_SetPoint(x+j,y+i,bkColor); // 背景颜色
          }
      }
    }
  }
}
#endif
/****************************************************************************
* 名    称:
* 功    能：在指定座标显示一个16x24点阵的ascii字符
* 入口参数：x          行座标
*           y          列座标
*           charColor  字符的颜色
*           bkColor    字符背景颜色
* 出口参数：无
* 说    明：显示范围限定为可显示的ascii码
* 调用方法：ili9320_PutChar_16x24(10, 10, font_id, 0x0000, 0xffff);
****************************************************************************/
void ili9320_PutChar_16x24(u16 x, u16 y, u8 c, u16 charColor, u16 bkColor)
{
	u16 i=0, j=0;
	u16 tmp_char=0;

	if(HyalineBackColor == bkColor)	{
		for (i=0;i<24;i++) {
			tmp_char = ascii_table_16x24[((c-0x20)*24)+i];
			for (j=0;j<16;j++) {
				if ( ((tmp_char >> j) & 0x01) == 0x01)
					ili9320_SetPoint(x+j,y+i,charColor); // 字符颜色
				/* else, do nothing // 透明背景 */
			}
		}
	} else {
		for (i=0;i<24;i++) {
			tmp_char=ascii_table_16x24[((c-0x20)*24)+i];
			for (j=0;j<16;j++) {
				if ( ((tmp_char >> j) & 0x01) == 0x01)
					ili9320_SetPoint(x+j,y+i,charColor); // 字符颜色
				else
					ili9320_SetPoint(x+j,y+i,bkColor); // 背景颜色
			}
		}
	}

}

typedef char font24x24dot_data[72];
/****************************************************************************
* 名    称:
* 功    能：在指定座标显示一个24x24点阵的ascii字符
* 入口参数：x          行座标
*           y          列座标
*           charColor  字符的颜色
*           bkColor    字符背景颜色
* 出口参数：无
* 说    明：显示范围限定为可显示的ascii码
* 调用方法：ili9320_PutChar_cn_24x24(10, 10, font_id, 0x0000, 0xffff);
****************************************************************************/
void ili9320_PutChar_cn_24x24(u16 x, u16 y, u8 c, u16 charColor, u16 bkColor)
{
	int i, j, k;
	int tmp_char=0;
	char *fnt_mod;

	fnt_mod = (char *)(((font24x24dot_data*)font_modlib_cn_24x24) + c);

	for (i=0; i<24; i++) {
		tmp_char = *fnt_mod++;
		tmp_char <<= 8;
		tmp_char |= (*fnt_mod++);
		tmp_char <<= 8;
		tmp_char |= (*fnt_mod++);
#if 1
		for (j=23, k=0; j>=0; j--,k++) {
			if ( 1 == ((tmp_char >> j) & 0x01)) {
				ili9320_SetPoint(x+k, y+i, charColor); // 字符颜色
			} else if (HyalineBackColor != bkColor) {
				ili9320_SetPoint(x+k, y+i, bkColor); // 背景颜色
			}
			/* else, do nothing // 透明背景 */
		}
#else
		for (j=0; j<24; j++) {
			if ( ((tmp_char >> j) & 0x01) == 0x01) {
				ili9320_SetPoint(x+j, y+i, charColor); // 字符颜色
			} else if (HyalineBackColor != bkColor) {
				ili9320_SetPoint(x+j, y+i, bkColor); // 背景颜色
			}
			/* else, do nothing // 透明背景 */
		}
#endif
	}
}

/*
 * 提取点阵方向：横向（先左->右, 再上->下）
 * 字节掉转：否
 * 字节方式：C语言
 *
 * 位序、字节序都是 大端
 */
typedef char font40x40dot_data[200];
void ili9320_PutChar_cn_40x40(u16 x, u16 y, u8 c, u16 charColor, u16 bkColor)
{
	int i, j, k;
	int tmp_char=0;
	char *fnt_mod;

	fnt_mod = (char *)(((font40x40dot_data*)font_modlib_cn_40x40) + c);

	for (i=0; i<40; i++) {
		tmp_char = *fnt_mod++;
		tmp_char <<= 8;
		tmp_char |= (*fnt_mod++);
		tmp_char <<= 8;
		tmp_char |= (*fnt_mod++);
		tmp_char <<= 8;
		tmp_char |= (*fnt_mod++);

		for (j=31, k=0; j>=0; j--,k++) {
			if ( 1 == ((tmp_char >> j) & 0x01)) {
				ili9320_SetPoint(x+k, y+i, charColor); // 字符颜色
			} else if (HyalineBackColor != bkColor) {
				ili9320_SetPoint(x+k, y+i, bkColor); // 背景颜色
			}
			/* else, do nothing // 透明背景 */
		}

		tmp_char = 0;
		tmp_char |= (*fnt_mod++);
		for (j=7, k=32; j>=0; j--,k++) {
			if ( 1 == ((tmp_char >> j) & 0x01)) {
				ili9320_SetPoint(x+k, y+i, charColor); // 字符颜色
			} else if (HyalineBackColor != bkColor) {
				ili9320_SetPoint(x+k, y+i, bkColor); // 背景颜色
			}
			/* else, do nothing // 透明背景 */
		}
	
	}
}

/*
 * 提取点阵方向：横向（先左->右, 再上->下）
 * 字节掉转：否
 * 字节方式：C语言
 *
 * 位序、字节序都是 大端
 */
typedef char font16x16dot_data[32];
void ili9320_PutChar_cn_16x16(u16 x, u16 y, u8 c, u16 charColor, u16 bkColor)
{
	int i, j, k;
	int tmp_char=0;
	char *fnt_mod;

	fnt_mod = (char *)(((font16x16dot_data*)font_modlib_cn_16x16) + c);
#if 1
	fill_16x16_dot_matrix(x, y, fnt_mod, charColor, bkColor);
#else
	for (i=0; i<16; i++) {
		tmp_char = *fnt_mod++;
		tmp_char <<= 8;
		tmp_char |= (*fnt_mod++);
		for (j=15, k=0; j>=0; j--,k++) {
			if ( 1 == ((tmp_char >> j) & 0x01)) {
				ili9320_SetPoint(x+k, y+i, charColor); // 字符颜色
			} else if (HyalineBackColor != bkColor) {
				ili9320_SetPoint(x+k, y+i, bkColor); // 背景颜色
			}
			/* else, do nothing // 透明背景 */
		}
	}
#endif
}

/*
 * 提取点阵方向：横向（先左->右, 再上->下）
 * 字节掉转：否
 * 字节方式：C语言
 *
 * 位序、字节序都是 大端
 */
void put_gb2312_char_16x16(u16 x, u16 y, u16 code, u16 charColor, u16 bkColor)
{
	int i, j, k, qw;
	int tmp_char=0;
	char *fnt_mod;

	qw  	= code - 0xa0a0;
	fnt_mod = gb2312_16x16 + ((((qw>>8) & 0xff) - 1)*94 + ((qw & 0xff) - 1)) * 32;

	fill_16x16_dot_matrix(x, y, fnt_mod, charColor, bkColor);

	return;
}


/*
 * 提取点阵方向：横向（先左->右, 再上->下）
 * 字节掉转：否
 * 字节方式：C语言
 *
 * 位序、字节序都是 大端
 */
static void fill_16x16_dot_matrix(u16 x, u16 y, char *fnt_mod, u16 charColor, u16 bkColor)
{
	int i, j, k;
	int tmp_char=0;

	for (i=0; i<16; i++) {
		tmp_char = *fnt_mod++;
		tmp_char <<= 8;
		tmp_char |= (*fnt_mod++);
		for (j=15, k=0; j>=0; j--,k++) {
			if ( 1 == ((tmp_char >> j) & 0x01)) {
				ili9320_SetPoint(x+k, y+i, charColor); // 字符颜色
			} else if (HyalineBackColor != bkColor) {
				ili9320_SetPoint(x+k, y+i, bkColor); // 背景颜色
			}
			/* else, do nothing // 透明背景 */
		}
	}

}
#if 0
/****************************************************************************
* 名    称：u16 ili9320_BGR2RGB(u16 c)
* 功    能：RRRRRGGGGGGBBBBB 改为 BBBBBGGGGGGRRRRR 格式
* 入口参数：c      BRG 颜色值
* 出口参数：RGB 颜色值
* 说    明：内部函数调用
* 调用方法：
****************************************************************************/
u16 ili9320_BGR2RGB(u16 c)
{
  u16  r, g, b, rgb;

  b = (c>>0)  & 0x1f;
  g = (c>>5)  & 0x3f;
  r = (c>>11) & 0x1f;
  
  rgb =  (b<<11) + (g<<5) + (r<<0);

  return( rgb );
}

/****************************************************************************
* 名    称：void ili9320_BackLight(u8 status)
* 功    能：开、关液晶背光
* 入口参数：status     1:背光开  0:背光关
* 出口参数：无
* 说    明：
* 调用方法：ili9320_BackLight(1);
****************************************************************************/
void ili9320_BackLight(u8 status)
{
  if ( status >= 1 )
  {
    Lcd_Light_ON;
  }
  else
  {
    Lcd_Light_OFF;
  }
}

/*
 *
 ******************* static function define. *******************
 *
 */

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

/*******************************************************************************
* Function Name  : LCD_WriteReg
* Description    : Writes to the selected LCD register.
* Input          : - LCD_Reg: address of the selected register.
*                  - LCD_RegValue: value to write to the selected register.
* Output         : None
* Return         : None
* author         : www.armjishu.com 
*******************************************************************************/
static void LCD_WriteReg(u16 LCD_Reg,u16 LCD_RegValue)
{
  /* Write 16-bit Index, then Write Reg */
  ClrCs
  ClrRs
  ClrWr
  LCD_Write(LCD_Reg);
  SetWr
  /* Write 16-bit Reg */
  SetRs
  ClrWr
  LCD_Write(LCD_RegValue);
  SetWr
  SetCs
}

/*******************************************************************************
* Function Name  : LCD_ReadReg
* Description    : Reads the selected LCD Register.
* Input          : None
* Output         : None
* Return         : LCD Register Value.
*******************************************************************************/
static u16 LCD_ReadReg(u8 LCD_Reg)
{
  u16 data;
  
  /* Write 16-bit Index (then Read Reg) */
  ClrCs
  ClrRs
  ClrWr
  LCD_Write(LCD_Reg);
  SetWr

  /* Read 16-bit Reg */
  SetRs
  ClrRd
  SetRd
  data = LCD_Read(); 
 SetCs
    
 return    data;
}

static u16 LCD_ReadSta(void)
{
  u16 data;

  /* Write 16-bit Index, then Write Reg */
  SetRs
  ClrRd
  SetRd
  data = LCD_Read(); 
  SetCs
    
  return    data;
}

#if 0
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
/*******************************************************************************
* Function Name  : LCD_WriteRAM_Prepare
* Description    : Prepare to write to the LCD RAM.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void LCD_WriteRAM_Prepare(void)
{
  /* Write 16-bit Index, then Write Reg */
  ClrCs
  ClrRs
  ClrWr
  LCD_Write(R34);
  SetWr
  SetCs
}

/*******************************************************************************
* Function Name  : LCD_WriteRAM
* Description    : Writes to the LCD RAM.
* Input          : - RGB_Code: the pixel color in RGB mode (5-6-5).
* Output         : None
* Return         : None
*******************************************************************************/
static void LCD_WriteRAM(u16 RGB_Code)					 
{
  /* Write 16-bit Index, then Write Reg */
  ClrCs
  SetRs
  ClrWr
  LCD_Write(RGB_Code);
  SetWr
  SetCs
}

/*******************************************************************************
* Function Name  : LCD_ReadRAM
* Description    : Reads the LCD RAM.
* Input          : None
* Output         : None
* Return         : LCD RAM Value.
*******************************************************************************/
static u16 LCD_ReadRAM(void)
{
  u16 dummy;
  u16 data;
  LCD_WriteRAM_Prepare();
  LCD_DB_AS_InPut();
  dummy = LCD_ReadSta();
  dummy++;
  data = LCD_ReadSta();
  LCD_DB_AS_OutPut();
  
  return    data;
}

static void Delay(__IO u32 nCount)
{
    __IO  u32 TimingDelay; 
    while(nCount--)
    {
        for(TimingDelay=0;TimingDelay<1000;TimingDelay++);
    }
}

/****************************************************************************
* 名    称：void ili9320_Delay(vu32 nCount)
* 功    能：延时
* 入口参数：nCount   延时值
* 出口参数：无
* 说    明：
* 调用方法：ili9320_Delay(10000);
****************************************************************************/
static void ili9320_Delay(vu32 nCount)
{
   Delay(nCount);
  //for(; nCount != 0; nCount--);
}
#endif

static void display_init_info(void)
{
#if 0
	int i, len, line_mtr;
	int startx;
	u16 id_buf[20];

//ili9320_PutChar_cn_24x24(0, 0, FONT_ID_CN_NING, Blue, HyalineBackColor);
	/* 宁夏电力吴忠供电局 */
	startx = (320 - 24*9)/2;
	for (i=0; i<7; i++) {
		ili9320_PutChar_cn_24x24((startx+24*i), 0, FONT_ID_CN_NING+i, Blue, HyalineBackColor);
	}
	ili9320_PutChar_cn_24x24((u16)(startx+24*i), (u16)0, (u8)FONT_ID_CN_DIAN, Blue, HyalineBackColor);
	++i;
	ili9320_PutChar_cn_24x24((u16)(startx+24*i), (u16)0, (u8)FONT_ID_CN_JU, Blue, HyalineBackColor);

	/* 状态监测箱用户选项 */
	i = 0;
	id_buf[i++] = FONT_ID_CN_ZHUANG;
	id_buf[i++] = FONT_ID_CN_TAI;
	id_buf[i++] = FONT_ID_CN_JIAN;
	id_buf[i++] = FONT_ID_CN_CE;
	id_buf[i++] = FONT_ID_CN_XIANG;
	id_buf[i++] = FONT_ID_CN_YONG;
	id_buf[i++] = FONT_ID_CN_HU;
	id_buf[i++] = FONT_ID_CN_XUAN;
	id_buf[i++] = FONT_ID_CN_XIANG1;

	len = i;
	line_mtr = get_mtr_row_from_24x24_line(2);
	for (i=0; i<len; i++)
		ili9320_PutChar_cn_24x24((24*i), line_mtr, id_buf[i], Blue, HyalineBackColor);

	/* 1.智能门锁控制 */
	line_mtr = get_mtr_row_from_24x24_line(3);
	ili9320_PutChar_16x24(0,  line_mtr, '1', Blue, HyalineBackColor);
	ili9320_PutChar_16x24(16, line_mtr, '.', Blue, HyalineBackColor);
	i = 0;
	id_buf[i++] = FONT_ID_CN_ZHI;
	id_buf[i++] = FONT_ID_CN_NENG;
	id_buf[i++] = FONT_ID_CN_MEN;
	id_buf[i++] = FONT_ID_CN_SUO;
	id_buf[i++] = FONT_ID_CN_KONG;
	id_buf[i++] = FONT_ID_CN_ZHI1;

	len = i;
	for (i=0; i<len; i++)
		ili9320_PutChar_cn_24x24((24*i + 32), line_mtr, id_buf[i], Blue, HyalineBackColor);

	/* 2.用电信息自助查询 */
	line_mtr = get_mtr_row_from_24x24_line(4);
	ili9320_PutChar_16x24(0,  line_mtr, '2', Blue, HyalineBackColor);
	ili9320_PutChar_16x24(16, line_mtr, '.', Blue, HyalineBackColor);
	i = 0;
	id_buf[i++] = FONT_ID_CN_YONG;
	id_buf[i++] = FONT_ID_CN_DIAN;
	id_buf[i++] = FONT_ID_CN_XIN;
	id_buf[i++] = FONT_ID_CN_XI;
	id_buf[i++] = FONT_ID_CN_ZI;
	id_buf[i++] = FONT_ID_CN_ZHU;
	id_buf[i++] = FONT_ID_CN_CHA;
	id_buf[i++] = FONT_ID_CN_XUN;

	len = i;
	for (i=0; i<len; i++)
		ili9320_PutChar_cn_24x24((24*i + 32), line_mtr, id_buf[i], Blue, HyalineBackColor);

	/* 3.线路运行状态监测 */
	line_mtr = get_mtr_row_from_24x24_line(5);
	ili9320_PutChar_16x24(0,  line_mtr, '3', Blue, HyalineBackColor);
	ili9320_PutChar_16x24(16, line_mtr, '.', Blue, HyalineBackColor);
	i = 0;
	id_buf[i++] = FONT_ID_CN_XIAN1;
	id_buf[i++] = FONT_ID_CN_LU;
	id_buf[i++] = FONT_ID_CN_YUN;
	id_buf[i++] = FONT_ID_CN_XING;
	id_buf[i++] = FONT_ID_CN_ZHUANG;
	id_buf[i++] = FONT_ID_CN_TAI;
	id_buf[i++] = FONT_ID_CN_JIAN;
	id_buf[i++] = FONT_ID_CN_CE;

	len = i;
	for (i=0; i<len; i++)
		ili9320_PutChar_cn_24x24((24*i + 32), line_mtr, id_buf[i], Blue, HyalineBackColor);
#endif
	return;
}



#if 1
#include <ra8875.h>
/****************************************************************************
* 名    称：void ili9320_Initializtion()
* 功    能：初始化 ILI9320 控制器
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：ili9320_Initializtion();
****************************************************************************/
void ili9320_Initializtion()
{ 
  // LCD_Init1();
   Init_RA875();

   display_init_info();

}

/****************************************************************************
* 名    称：void ili9320_SetCursor(u16 x,u16 y)
* 功    能：设置屏幕座标
* 入口参数：x      行座标
*           y      列座标
* 出口参数：无
* 说    明：
* 调用方法：ili9320_SetCursor(10,10);
****************************************************************************/
void ili9320_SetCursor(u16 x,u16 y)
{				
 
   /*
	*(__IO uint16_t *) (Bank1_LCD_C)= 0x002A;
  	*(__IO uint16_t *) (Bank1_LCD_D)= x>>8; 	
  	*(__IO uint16_t *) (Bank1_LCD_D)= x&0x00ff; 
  	*(__IO uint16_t *) (Bank1_LCD_D)= 799>>8; 
  	*(__IO uint16_t *) (Bank1_LCD_D)= 799&0x00ff; 
  	*(__IO uint16_t *) (Bank1_LCD_C)= 0x002b; 
  	*(__IO uint16_t *) (Bank1_LCD_D)= y>>8; 
  	*(__IO uint16_t *) (Bank1_LCD_D)= y&0x00ff; 
  	*(__IO uint16_t *) (Bank1_LCD_D)= 479>>8; 
  	*(__IO uint16_t *) (Bank1_LCD_D)= 479&0x00ff;
	*/
	     *(__IO uint16_t *) (LCD_COMM_ADD)= 0x46;
 	  	 *(__IO uint16_t *) (LCD_DATA_ADD)= x; 
		 *(__IO uint16_t *) (LCD_COMM_ADD)= 0x47;
 	 	 *(__IO uint16_t *) (LCD_DATA_ADD)= x>>8; 
		
		 *(__IO uint16_t *) (LCD_COMM_ADD)= 0x48;
 	  	 *(__IO uint16_t *) (LCD_DATA_ADD)= y; 
		 *(__IO uint16_t *) (LCD_COMM_ADD)= 0x49;
 	 	 *(__IO uint16_t *) (LCD_DATA_ADD)= y>>8;
		






}

/****************************************************************************
* 名    称：void ili9320_SetWindows(u16 StartX,u16 StartY,u16 EndX,u16 EndY)
* 功    能：设置窗口区域
* 入口参数：StartX     行起始座标
*           StartY     列起始座标
*           EndX       行结束座标
*           EndY       列结束座标
* 出口参数：无
* 说    明：
* 调用方法：ili9320_SetWindows(0,0,100,100)；
****************************************************************************/
void ili9320_SetWindows(u16 StartX,u16 StartY,u16 EndX,u16 EndY)
{
 	/*
	 *(__IO uint16_t *) (Bank1_LCD_C)= 0x002A;
  	*(__IO uint16_t *) (Bank1_LCD_D)= StartX>>8; 	
  	*(__IO uint16_t *) (Bank1_LCD_D)= StartX&0x00ff; 
  	*(__IO uint16_t *) (Bank1_LCD_D)= EndX>>8; 
  	*(__IO uint16_t *) (Bank1_LCD_D)= EndX&0x00ff; 
  	*(__IO uint16_t *) (Bank1_LCD_C)= 0x002b; 
  	*(__IO uint16_t *) (Bank1_LCD_D)= StartY>>8; 
  	*(__IO uint16_t *) (Bank1_LCD_D)= StartY&0x00ff; 
  	*(__IO uint16_t *) (Bank1_LCD_D)= EndY>>8; 
  	*(__IO uint16_t *) (Bank1_LCD_D)= EndY&0x00ff; 
  	*(__IO uint16_t *) (Bank1_LCD_C)= 0x002c;
	  */
	     *(__IO uint16_t *) (LCD_COMM_ADD)= 0x30;
 	  	 *(__IO uint16_t *) (LCD_DATA_ADD)= StartX; 
		 *(__IO uint16_t *) (LCD_COMM_ADD)= 0x31;
 	 	 *(__IO uint16_t *) (LCD_DATA_ADD)= StartX>>8; 
		 *(__IO uint16_t *) (LCD_COMM_ADD)= 0x34;
 	  	 *(__IO uint16_t *) (LCD_DATA_ADD)= EndX; 
		 *(__IO uint16_t *) (LCD_COMM_ADD)= 0x35;
 	 	 *(__IO uint16_t *) (LCD_DATA_ADD)= EndX>>8;
		 *(__IO uint16_t *) (LCD_COMM_ADD)= 0x32;
 	  	 *(__IO uint16_t *) (LCD_DATA_ADD)= StartY; 
		 *(__IO uint16_t *) (LCD_COMM_ADD)= 0x33;
 	 	 *(__IO uint16_t *) (LCD_DATA_ADD)= StartY>>8;
		 *(__IO uint16_t *) (LCD_COMM_ADD)= 0x36;
 	  	 *(__IO uint16_t *) (LCD_DATA_ADD)= EndY; 
		 *(__IO uint16_t *) (LCD_COMM_ADD)= 0x37;
 	 	 *(__IO uint16_t *) (LCD_DATA_ADD)= EndY>>8;
		 *(__IO uint16_t *) (LCD_COMM_ADD)= 0x02;
		// *(__IO uint16_t *) (LCD_DATA_ADD)= PixelIndex;






  
}

/****************************************************************************
* 名    称：void ili9320_Clear(u16 dat)
* 功    能：将屏幕填充成指定的颜色，如清屏，则填充 0xffff
* 入口参数：dat      填充值
* 出口参数：无
* 说    明：
* 调用方法：ili9320_Clear(0xffff);
****************************************************************************/
void ili9320_Clear(u16 dat)
{
 

   unsigned int l=800,w;
   /*
    *(__IO uint16_t *) (Bank1_LCD_C)= 0x002A;
  	*(__IO uint16_t *) (Bank1_LCD_D)= 0>>8; 	
  	*(__IO uint16_t *) (Bank1_LCD_D)= 0&0x00ff; 
  	*(__IO uint16_t *) (Bank1_LCD_D)= 479>>8; 
  	*(__IO uint16_t *) (Bank1_LCD_D)= 479&0x00ff; 
  	*(__IO uint16_t *) (Bank1_LCD_C)= 0x002b; 
  	*(__IO uint16_t *) (Bank1_LCD_D)= 0>>8; 
  	*(__IO uint16_t *) (Bank1_LCD_D)= 0&0x00ff; 
  	*(__IO uint16_t *) (Bank1_LCD_D)= 271>>8; 
  	*(__IO uint16_t *) (Bank1_LCD_D)= 271&0x00ff; 
  	*(__IO uint16_t *) (Bank1_LCD_C)= 0x002c;
	(*/
	   /*  *(__IO uint16_t *) (LCD_COMM_ADD)= 0x30;
 	  	 *(__IO uint16_t *) (LCD_DATA_ADD)= 0; 
		 *(__IO uint16_t *) (LCD_COMM_ADD)= 0x31;
 	 	 *(__IO uint16_t *) (LCD_DATA_ADD)= 0>>8; 
		 *(__IO uint16_t *) (LCD_COMM_ADD)= 0x34;
 	  	 *(__IO uint16_t *) (LCD_DATA_ADD)= 799; 
		 *(__IO uint16_t *) (LCD_COMM_ADD)= 0x35;
 	 	 *(__IO uint16_t *) (LCD_DATA_ADD)= 799>>8;
		 *(__IO uint16_t *) (LCD_COMM_ADD)= 0x32;
 	  	 *(__IO uint16_t *) (LCD_DATA_ADD)= 0; 
		 *(__IO uint16_t *) (LCD_COMM_ADD)= 0x33;
 	 	 *(__IO uint16_t *) (LCD_DATA_ADD)= 0>>8;
		 *(__IO uint16_t *) (LCD_COMM_ADD)= 0x36;
 	  	 *(__IO uint16_t *) (LCD_DATA_ADD)= 479; 
		 *(__IO uint16_t *) (LCD_COMM_ADD)= 0x37;
 	 	 *(__IO uint16_t *) (LCD_DATA_ADD)= 479>>8;
		 *(__IO uint16_t *) (LCD_COMM_ADD)= 0x02;
		*/

		 *(__IO uint16_t *) (LCD_COMM_ADD)= 0x46;
 	  	 *(__IO uint16_t *) (LCD_DATA_ADD)= 0; 
		 *(__IO uint16_t *) (LCD_COMM_ADD)= 0x47;
 	 	 *(__IO uint16_t *) (LCD_DATA_ADD)= 0>>8; 
		
		 *(__IO uint16_t *) (LCD_COMM_ADD)= 0x48;
 	  	 *(__IO uint16_t *) (LCD_DATA_ADD)= 0; 
		 *(__IO uint16_t *) (LCD_COMM_ADD)= 0x49;
 	 	 *(__IO uint16_t *) (LCD_DATA_ADD)= 0>>8;
		 *(__IO uint16_t *) (LCD_COMM_ADD)= 0x02;


	
	while(l--)
	{
	    for(w=0;w<480;w++)
		{    
          //	LCD_WR_Data(dat);
		 // *(__IO uint16_t *) (Bank1_LCD_D)=dat;
		   *(__IO uint16_t *) (LCD_DATA_ADD)= dat;

		}
	}
}

/****************************************************************************
* 名    称：u16 ili9320_GetPoint(u16 x,u16 y)
* 功    能：获取指定座标的颜色值
* 入口参数：x      行座标
*           y      列座标
* 出口参数：当前座标颜色值
* 说    明：
* 调用方法：i=ili9320_GetPoint(10,10);
****************************************************************************/
u16 ili9320_GetPoint(u16 x,u16 y)
{ 
  
  u16 temp;
  /*
  *(__IO uint16_t *) (Bank1_LCD_C)= 0x002A;
  *(__IO uint16_t *) (Bank1_LCD_D)= x>>8; 	
  *(__IO uint16_t *) (Bank1_LCD_D)= x&0x00ff; 
  *(__IO uint16_t *) (Bank1_LCD_D)= 799>>8; 
  *(__IO uint16_t *) (Bank1_LCD_D)= 799&0x00ff; 
  *(__IO uint16_t *) (Bank1_LCD_C)= 0x002b; 
  *(__IO uint16_t *) (Bank1_LCD_D)= y>>8; 
  *(__IO uint16_t *) (Bank1_LCD_D)= y&0x00ff; 
  *(__IO uint16_t *) (Bank1_LCD_D)= 479>>8; 
  *(__IO uint16_t *) (Bank1_LCD_D)= 479&0x00ff; 
  *(__IO uint16_t *) (Bank1_LCD_C)= 0x002e;  
	*/
         *(__IO uint16_t *) (LCD_COMM_ADD)= 0x4a;
 	  	 *(__IO uint16_t *) (LCD_DATA_ADD)= x; 
		 *(__IO uint16_t *) (LCD_COMM_ADD)= 0x4b;
 	 	 *(__IO uint16_t *) (LCD_DATA_ADD)= x>>8; 
		
		 *(__IO uint16_t *) (LCD_COMM_ADD)= 0x4c;
 	  	 *(__IO uint16_t *) (LCD_DATA_ADD)= y; 
		 *(__IO uint16_t *) (LCD_COMM_ADD)= 0x4d;
 	 	 *(__IO uint16_t *) (LCD_DATA_ADD)= y>>8;
		 *(__IO uint16_t *) (LCD_COMM_ADD)= 0x02;

		 temp=*(__IO uint16_t *) (LCD_DATA_ADD);
		 temp=*(__IO uint16_t *) (LCD_DATA_ADD);

  return (temp);


}

/****************************************************************************
* 名    称：void ili9320_SetPoint(u16 x,u16 y,u16 point)
* 功    能：在指定座标画点
* 入口参数：x      行座标
*           y      列座标
*           point  点的颜色
* 出口参数：无
* 说    明：
* 调用方法：ili9320_SetPoint(10,10,0x0fe0);
****************************************************************************/
void ili9320_SetPoint(u16 x,u16 y,u16 point)
{
 
   /* *(__IO uint16_t *) (Bank1_LCD_C)= 0x002A;
	*(__IO uint16_t *) (Bank1_LCD_D)= x>>8; 	
	*(__IO uint16_t *) (Bank1_LCD_D)= x&0x00ff; 
	*(__IO uint16_t *) (Bank1_LCD_D)= 799>>8; 
	*(__IO uint16_t *) (Bank1_LCD_D)= 799&0x00ff; 
	*(__IO uint16_t *) (Bank1_LCD_C)= 0x002b; 
	*(__IO uint16_t *) (Bank1_LCD_D)= y>>8; 
	*(__IO uint16_t *) (Bank1_LCD_D)= y&0x00ff; 
	*(__IO uint16_t *) (Bank1_LCD_D)= 479>>8; 
	*(__IO uint16_t *) (Bank1_LCD_D)= 479&0x00ff; 
	*(__IO uint16_t *) (Bank1_LCD_C)= 0x002c;  
 	*(__IO uint16_t *) (Bank1_LCD_D)= point;
	*/
         *(__IO uint16_t *) (LCD_COMM_ADD)= 0x46;
 	  	 *(__IO uint16_t *) (LCD_DATA_ADD)= x; 
		 *(__IO uint16_t *) (LCD_COMM_ADD)= 0x47;
 	 	 *(__IO uint16_t *) (LCD_DATA_ADD)= x>>8; 
		
		 *(__IO uint16_t *) (LCD_COMM_ADD)= 0x48;
 	  	 *(__IO uint16_t *) (LCD_DATA_ADD)= y; 
		 *(__IO uint16_t *) (LCD_COMM_ADD)= 0x49;
 	 	 *(__IO uint16_t *) (LCD_DATA_ADD)= y>>8;
		 *(__IO uint16_t *) (LCD_COMM_ADD)= 0x02;
		 *(__IO uint16_t *) (LCD_DATA_ADD)= point;




   
}

/****************************************************************************
* 名    称：void ili9320_DrawPicture(u16 StartX,u16 StartY,u16 EndX,u16 EndY,u16 *pic)
* 功    能：在指定座标范围显示一副图片
* 入口参数：StartX     行起始座标
*           StartY     列起始座标
*           EndX       行结束座标
*           EndY       列结束座标
            pic        图片头指针
* 出口参数：无
* 说    明：图片取模格式为水平扫描，16位颜色模式
* 调用方法：ili9320_DrawPicture(0,0,100,100,(u16*)demo);
****************************************************************************/
void ili9320_DrawPicture(u16 StartX,u16 StartY,u16 EndX,u16 EndY,u16 *pic)
{
  u16  i;
  ili9320_SetWindows(StartX,StartY,EndX,EndY);
  ili9320_SetCursor(StartX,StartY);	
  //LCD_WR_REG(0x002c);    
  // *(__IO uint16_t *) (Bank1_LCD_C)= 0x002c;
 	*(__IO uint16_t *) (LCD_COMM_ADD)= 0x02;
 
  for (i=0;i<(EndX*EndY);i++)
  // LCD_WR_Data(*pic++);
// *(__IO uint16_t *) (Bank1_LCD_D)= *pic++;
  *(__IO uint16_t *) (LCD_DATA_ADD)= *pic++;

}

/****************************************************************************
* 名    称：void ili9320_PutChar(u16 x,u16 y,u8 c,u16 charColor,u16 bkColor)
* 功    能：在指定座标显示一个8x16点阵的ascii字符
* 入口参数：x          行座标
*           y          列座标
*           charColor  字符的颜色
*           bkColor    字符背景颜色
* 出口参数：无
* 说    明：显示范围限定为可显示的ascii码
* 调用方法：ili9320_PutChar(10,10,'a',0x0000,0xffff);
****************************************************************************/
void ili9320_PutChar(u16 x,u16 y,u8 c,u16 charColor,u16 bkColor)
{
  u16 i=0;
  u16 j=0;
  
  u8 tmp_char=0;

  for (i=0;i<16;i++)
  {
    tmp_char=ascii_8x16[((c-0x20)*16)+i];
    for (j=0;j<8;j++)
    {
      if ( ((tmp_char >> (7-j)) & 0x01) == 0x01)
        {
          ili9320_SetPoint(x+j,y+i,charColor); // 字符颜色
        }
        else if (HyalineBackColor != bkColor)
        {
          ili9320_SetPoint(x+j,y+i,bkColor); // 背景颜色
        }
    }
  }
}

/****************************************************************************
* 名    称：void ili9320_Test()
* 功    能：测试液晶屏
* 入口参数：无
* 出口参数：无
* 说    明：显示彩条，测试液晶屏是否正常工作
* 调用方法：ili9320_Test();
****************************************************************************/
void ili9320_Test()
{
/*
  u16 i,j;
  ili9320_SetCursor(0,0);
  LCD_WR_REG(0x002c);
  for(i=0;i<320;i++)
    for(j=0;j<240;j++)
    {
      if(i>279)LCD_WR_Data(0x0000);
      else if(i>239)LCD_WR_Data(0x001f);
      else if(i>199)LCD_WR_Data(0x07e0);
      else if(i>159)LCD_WR_Data(0x07ff);
      else if(i>119)LCD_WR_Data(0xf800);
      else if(i>79)LCD_WR_Data(0xf81f);
      else if(i>39)LCD_WR_Data(0xffe0);
      else LCD_WR_Data(0xffff);
    }
   */
}

/****************************************************************************
* 名    称：u16 ili9320_BGR2RGB(u16 c)
* 功    能：RRRRRGGGGGGBBBBB 改为 BBBBBGGGGGGRRRRR 格式
* 入口参数：c      BRG 颜色值
* 出口参数：RGB 颜色值
* 说    明：内部函数调用
* 调用方法：
****************************************************************************/
u16 ili9320_BGR2RGB(u16 c)
{
  u16  r, g, b;

  b = (c>>0)  & 0x1f;
  g = (c>>5)  & 0x3f;
  r = (c>>11) & 0x1f;
  
  return( (b<<11) + (g<<5) + (r<<0) );
}

/****************************************************************************
* 名    称：void ili9320_WriteIndex(u16 idx)
* 功    能：写 ili9320 控制器寄存器地址
* 入口参数：idx   寄存器地址
* 出口参数：无
* 说    明：调用前需先选中控制器，内部函数
* 调用方法：ili9320_WriteIndex(0x0000);
****************************************************************************/
void ili9320_WriteIndex(u16 idx)
{
 // LCD_WR_REG(idx);
 //*(__IO uint16_t *) (Bank1_LCD_D)= idx;
 	*(__IO uint16_t *) (LCD_COMM_ADD)= idx;


}

/****************************************************************************
* 名    称：void ili9320_WriteData(u16 dat)
* 功    能：写 ili9320 寄存器数据
* 入口参数：dat     寄存器数据
* 出口参数：无
* 说    明：向控制器指定地址写入数据，调用前需先写寄存器地址，内部函数
* 调用方法：ili9320_WriteData(0x1030)
****************************************************************************/
void ili9320_WriteData(u16 dat)
{
 // LCD_WR_Data(dat);
 // *(__IO uint16_t *) (Bank1_LCD_D)= dat;
  *(__IO uint16_t *) (LCD_DATA_ADD)= dat;
       // temp=*(__IO uint16_t *) (LCD_DATA_ADD);
		 //temp=*(__IO uint16_t *) (LCD_DATA_ADD);



}

/****************************************************************************
* 名    称：u16 ili9320_ReadData(void)
* 功    能：读取控制器数据
* 入口参数：无
* 出口参数：返回读取到的数据
* 说    明：内部函数
* 调用方法：i=ili9320_ReadData();
****************************************************************************/
u16 ili9320_ReadData(void)
{
  u16 val=0;
 // val=LCD_RD_data();
  // val=*(__IO uint16_t *) (Bank1_LCD_D);
   val=*(__IO uint16_t *) (LCD_DATA_ADD);
   val=*(__IO uint16_t *) (LCD_DATA_ADD);
  
  return val;
}

/****************************************************************************
* 名    称：u16 ili9320_ReadRegister(u16 index)
* 功    能：读取指定地址寄存器的值
* 入口参数：index    寄存器地址
* 出口参数：寄存器值
* 说    明：内部函数
* 调用方法：i=ili9320_ReadRegister(0x0022);
****************************************************************************/
u16 ili9320_ReadRegister(u16 index)
{
  u16 tmp;
  tmp= *(volatile unsigned int *)(0x60000000);
  
  return tmp;
}

/****************************************************************************
* 名    称：void ili9320_WriteRegister(u16 index,u16 dat)
* 功    能：写指定地址寄存器的值
* 入口参数：index    寄存器地址
*         ：dat      寄存器值
* 出口参数：无
* 说    明：内部函数
* 调用方法：ili9320_WriteRegister(0x0000,0x0001);
****************************************************************************/
void ili9320_WriteRegister(u16 index,u16 dat)
{
 /************************************************************************
  **                                                                    **
  ** nCS       ----\__________________________________________/-------  **
  ** RS        ------\____________/-----------------------------------  **
  ** nRD       -------------------------------------------------------  **
  ** nWR       --------\_______/--------\_____/-----------------------  **
  ** DB[0:15]  ---------[index]----------[data]-----------------------  **
  **                                                                    **
  ************************************************************************/
  
//  LCD_WR_CMD(index,dat);
   *(__IO uint16_t *) (LCD_COMM_ADD)= index;
   *(__IO uint16_t *) (LCD_DATA_ADD)= dat;


}

/****************************************************************************
* 名    称：void ili9320_Reset()
* 功    能：复位 ili9320 控制器
* 入口参数：无
* 出口参数：无
* 说    明：复位控制器，内部函数
* 调用方法：ili9320_Reset()
****************************************************************************/
void ili9320_Reset()
{
  /***************************************
   **                                   **
   **  -------\______________/-------   **
   **         | <---Tres---> |          **
   **                                   **
   **  Tres: Min.1ms                    **
   ***************************************/
    
}

/****************************************************************************
* 名    称：void ili9320_BackLight(u8 status)
* 功    能：开、关液晶背光
* 入口参数：status     1:背光开  0:背光关
* 出口参数：无
* 说    明：
* 调用方法：ili9320_BackLight(1);
****************************************************************************/
void ili9320_BackLight(u8 status)
{
  if ( status >= 1 )
  {
    //GPIO_SetBits(GPIOC,LCD_BK);
  }
  else
  {
    //GPIO_ResetBits(GPIOC,LCD_BK);
  }
}

/****************************************************************************
* 名    称：void ili9320_Delay(vu32 nCount)
* 功    能：延时
* 入口参数：nCount   延时值
* 出口参数：无
* 说    明：
* 调用方法：ili9320_Delay(10000);
****************************************************************************/
void ili9320_Delay(vu32 nCount)
{
  for(; nCount != 0; nCount--);
}





#endif


