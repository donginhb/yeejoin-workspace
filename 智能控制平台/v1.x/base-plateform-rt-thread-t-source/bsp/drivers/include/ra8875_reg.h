#ifndef RA8875_REG_H__
#define RA8875_REG_H__

#define RA8875_REG_STATUS	0x00
#define RA8875_REG_PWRR 	0x01 /*Power and Display Control Register*/		
#define RA8875_REG_MRWC		0x02 /*Memory Read/White Command*/
#define RA8875_REG_PCSR		0x04 /*Pixel Clock Setting Register*/
#define RA8875_REG_SROC		0x05 /*Serial Flash/ROM Configuration Register*/
#define RA8875_REG_SFCLR	0x06 /*Serial Flash/ROM CLK Setting Register*/
#define RA8875_REG_SYSR		0x10 /*System Configuration Register*/
#define RA8875_REG_GPI		0x12 /*General Purpose Input*/
#define RA8875_REG_GPO		0x13 /*General Purpose Output*/
#define RA8875_REG_HDWR		0x14 /*LCD Horizontal Display Width Register*/
#define RA8875_REG_HNDFTR	0x15 /*Horizontal Non-Display Period Fine Tuning Option Register*/
#define RA8875_REG_HNDR		0x16 /*LCD Horizontal Non-Display Period Register*/
#define RA8875_REG_HSTR		0x17 /*HSYNC	Start Position Register*/
#define RA8875_REG_HPWR		0x18 /*HSYNC	Pulse Width Register*/
#define RA8875_REG_VDHR0	0x19 /*LCD Vertical Display Height Register*/
#define RA8875_REG_VDHR1	0x1A /*LCD Vertical Display Height Register0*/
#define RA8875_REG_VNDR0	0x1B /*LCD Veryical Non-Display Period Register*/
#define RA8875_REG_VNDR1	0x1C /*LCD Veryical Non-Display Period Register*/
#define RA8875_REG_VSTR0	0x1D /*VSYNC Start Position Register*/
#define RA8875_REG_VSTR1	0x1E /*VSYNC Start Position Register*/
#define RA8875_REG_VPWR		0x1F /*VSYNC	Pulse Width Register*/
#define RA8875_REG_DPCR		0x20 /*Display Configuration Register*/
#define RA8875_REG_FNCR0	0x21 /*Font Control Register 0*/
#define RA8875_REG_FNCR1	0x22 /*Font Control Register 1*/
#define RA8875_REG_CGSR		0x23 /*CGRAM Select Register*/
#define RA8875_REG_HOFS0	0x24 /*Horizontal Scroll Offset Register 0*/
#define RA8875_REG_HOFS1	0x25 /*Horizontal Scroll Offset Register 1*/
#define RA8875_REG_VOFS0	0x26 /*Vertical Scroll Offset Register 0*/
#define RA8875_REG_VOFS1	0x27 /*Vertical Scroll Offset Register 1*/
#define RA8875_REG_FLDR		0x29 /*Font Line Distance Setting Register*/
#define RA8875_REG_F_CURXL	0x2A /*Font Write Cursor Horizontal Position Register 0*/
#define RA8875_REG_F_CURXH	0x2B /*FOnt Write Cursor Horizontal Position Register 1*/
#define RA8875_REG_F_CURYL	0x2C /*Font Write Cursor Vertical Position Register 0*/
#define RA8875_REG_F_CURYH	0x2D /*Font Write Cursor Vertical Position Register	1*/
#define RA8875_REG_FWTSR	0x2E /*Font Write Type Setting Register*/
#define RA8875_REG_SFRS		0x2F /*Serial Font ROM Setting*/
#define RA8875_REG_HSAW0	0x30 /*Horizontal Start Point 0 of Active Window*/
#define RA8875_REG_HSAW1	0x31 /*Horizontal Start Point 1 of Active Window*/
#define RA8875_REG_VSAW0	0x32 /*Vertical Start Point 0 of Active Window*/
#define RA8875_REG_VSAW1	0x33 /*Vertical Start Point 1 of Active Window*/
#define RA8875_REG_HEAW0	0x34 /*Horizontal End Point 0 of Active Window*/
#define RA8875_REG_HEAW1	0x35 /*Horizontal End Point 1 of Active Window*/
#define RA8875_REG_VEAW0	0x36 /*Vertical End point of Active Window 0*/
#define RA8875_REG_VEAW1	0x37 /*Vertical End point of Active Window 1*/
#define RA8875_REG_HSSW0	0x38 /*Horizontal Start Point 0 of Scroll Window*/
#define RA8875_REG_HSSW1	0x39 /*Horizontal Start Point 1 of Scroll Window*/
#define RA8875_REG_VSSW0	0x3A /*Vertical Start Point 0 of Scroll Window*/
#define RA8875_REG_VSSW1	0x3B /*Vertical Start Point 1 of Scroll Window*/
#define RA8875_REG_HESW0	0x3C /*Horizontal End Point 0 of Scroll Window*/
#define RA8875_REG_HESW1	0x3D /*Horizontal End Point 1 of Scroll Window*/
#define RA8875_REG_VESW0	0x3E /*Vertical End Point 0 of Scroll Window*/
#define RA8875_REG_VESW1	0x3F /*Vertical End Point 1 of Scroll Window*/
#define RA8875_REG_MWCR0	0x40 /*Memory Write Control Register 0*/
#define RA8875_REG_MWCR1	0x41 /*Memory Write Control Register 1*/
#define RA8875_REG_BTCR		0x44 /*Blink Time Control Register*/
#define RA8875_REG_MRCD		0x45 /*Memory Read Cursor Direction*/
#define RA8875_REG_CURH0	0x46 /*Memory Write Cursor Horizontal Position Register 0*/
#define RA8875_REG_CURH1	0x47 /*Memory Write Cursor Horizontal Position Register 1*/
#define RA8875_REG_CURV0	0x48 /*Memory Write Cursor Vertical Position Register 0*/
#define RA8875_REG_CURV1	0x49 /*Memory Write Cursor Vertical Position Register 1*/
#define RA8875_REG_RCURH0	0x4A /*Memory Read Cursor Horizontal Position Register 0*/
#define RA8875_REG_RCURH1	0x4B /*Memory Read Cursor Horizontal Position Register 1*/
#define RA8875_REG_RCURV0	0x4C /*Memory Read Cursor Vertical Position Register 0*/
#define RA8875_REG_RCURV1	0x4D /*Memory Read Cursor Vertical Position Register 1*/
#define RA8875_REG_CURHS	0x4E /*Font Write Cursor and Memory Write Cursor Horizontal Size Register*/
#define RA8875_REG_CURVS	0x4F /* Font Write Cursor Vertical Size Register*/
#define RA8875_REG_BECR0	0x50 /*BTE Function Control Register 0*/
#define RA8875_REG_BECR1	0x51 /*BTE Function Control Register 1*/
#define RA8875_REG_LTPR0	0x52 /*Layer Transparency Register 0*/
#define RA8875_REG_LTPR1	0x53 /*Layer Transparency Register 1*/
#define RA8875_REG_HSBE0	0x54 /*Horizontal Source Point 0 of BTE*/
#define RA8875_REG_HSBE1	0x55 /*Horizontal Source Point 1 of BTE*/
#define RA8875_REG_VSBE0	0x56 /*Vertical Source Point 0 of BTE*/
#define RA8875_REG_VSBE1	0x57 /*Vertical Source Point 1 of BTE*/
#define RA8875_REG_HDBE0	0x58 /*Horizontal Destination Point 0 of BTE*/
#define RA8875_REG_HDBE1	0x59 /*Horizontal Destination Point 1 of BTE*/
#define RA8875_REG_VDBE0	0x5A /*Vertical Destination Point 0 of BTE*/
#define RA8875_REG_VDBE1	0x5B /*Vertical Destination Point 1 of BTE*/
#define RA8875_REG_BEWR0	0x5C /*BTE Width Register 0 */
#define RA8875_REG_BEWR1	0x5D /*BTE Width Register 1 */
#define RA8875_REG_BEHR0	0x5E /*BTE Height Register 0 */
#define RA8875_REG_BEHR1	0x5F /*BTE Height Register 1 */
#define RA8875_REG_BGCR0	0x60 /*Background Color Register 0 */
#define RA8875_REG_BGCR1	0x61 /*Background Color Register 1 */
#define RA8875_REG_BGCR2	0x62 /*Background Color Register 2 */
#define RA8875_REG_FGCR0	0x63 /*Foreground Color Register 0*/
#define RA8875_REG_FGCR1	0x64 /*Foreground Color Register 1*/
#define RA8875_REG_FGCR2	0x65 /*Foreground Color Register 2*/
#define RA8875_REG_PTNO		0x66 /*Pattern Set No for BTE */
#define RA8875_REG_BGTR0	0x67 /*Background Color Register for Transparent 0 */
#define RA8875_REG_BGTR1	0x68 /*Background Color Register for Transparent 1 */
#define RA8875_REG_BGTR2	0x69 /*Background Color Register for Transparent 2 */
#define RA8875_REG_TPCR0	0x70 /*Touch Panel Control Register 0*/
#define RA8875_REG_TPCR1	0x71 /*Touch Panel Control Register 1*/
#define RA8875_REG_TPXH		0x72 /*Touch Panel X High Byte Data Register*/
#define RA8875_REG_TPYH		0x73 /*Touch Panel Y High Byte Data Register*/
#define RA8875_REG_TPXYL	0x74 /*Touch Panel X/Y Low Byte Data Register */
#define RA8875_REG_GCHP0	0x80 /*Graphic Cursor Horizontal Position Register 0 */
#define RA8875_REG_GCHP1	0x81 /*Graphic Cursor Horizontal Position Register 1*/
#define RA8875_REG_GCVP0	0x82 /*Graphic Cursor Vertical Position Register 0 */
#define RA8875_REG_GCVP1	0x83 /*Graphic Cursor Vertical Position Register 1 */
#define RA8875_REG_GCC0		0x84 /*Graphic Cursor Color 0 */
#define RA8875_REG_GCC1		0x85 /*Graphic Cursor Color 1 */
#define RA8875_REG_PLLC1	0x88 /*PLL Control Register 1*/
#define RA8875_REG_PLLC2	0x89 /*PLL Control Register 2*/
#define RA8875_REG_P1CR		0x8A /*PWM1 Control Register*/
#define RA8875_REG_P1DCR	0x8B /*PWM1 Duty Cycle Register*/
#define RA8875_REG_P2CR		0x8C /* PWM2 Control Register*/
#define RA8875_REG_P2DCR	0x8D /*PWM2 Control Register*/
#define RA8875_REG_MCLR		0x8E /*Memory Clear Control Register*/
#define RA8875_REG_DCR		0x90 /*Draw Line/Circle/Square Control Register*/
#define RA8875_REG_DLHSR0	0x91 /*Draw Line/Square Horizontal Start Address Register 0*/
#define RA8875_REG_DLHSR1	0x92 /*Draw Line/Square Horizontal Start Address Register 1*/
#define RA8875_REG_DLVSR0	0x93 /*Draw Line/Square Vertical Start Address Register 0  */
#define RA8875_REG_DLVSR1	0x94 /*Draw Line/Square Vertical Start Address Register 1  */
#define RA8875_REG_DLHER0	0x95 /*Draw Line/Square Horizontal End Address Register 0*/
#define RA8875_REG_DLHER1	0x96 /*Draw Line/Square Horizontal End Address Register 1*/
#define RA8875_REG_DLVER0	0x97 /*Draw Line/Square Vertical End Address Register 0*/
#define RA8875_REG_DLVER1	0x98 /*Draw Line/Square Vertical End Address Register 0*/
#define RA8875_REG_DCHR0	0x99 /*Draw Circle Center Horizontal Address Register 0*/
#define RA8875_REG_DCHR1	0x9A /*Draw Circle Center Horizontal Address Register 1*/
#define RA8875_REG_DCVR0	0x9B /*Draw Circle Center Vertical Address Register 0*/
#define RA8875_REG_DCVR1	0x9C /*Draw Circle Center Vertical Address Register 1*/
#define RA8875_REG_DCRR		0x9D /*Draw Circle Radius Register*/
#define RA8875_REG_DE_EC_CSCR	0xA0 /*Draw Ellipse/Ellipse Curve/Circle Square Control Register */
#define RA8875_REG_ELL_A0	0xA1 /*Draw Ellipse/Circle Square Long axis Setting Register*/
#define RA8875_REG_ELL_A1	0xA2 /*Draw Ellipse/Circle Square Long axis Setting Register*/
#define RA8875_REG_ELL_B0	0xA3 /*Draw Ellipse/Circle Square Short axis Setting Register*/
#define RA8875_REG_ELL_B1	0xA4 /*Draw Ellipse/Circle Square Short axis Setting Register*/
#define RA8875_REG_DEHR0	0xA5 /*Draw Ellipse/Circle Square Center Horizontal Address Register 0*/
#define RA8875_REG_DEHR1	0xA6 /*Draw Ellipse/Circle Square Center Horizontal Address Register 1*/
#define RA8875_REG_DEVR0	0xA7 /*Draw Ellipse/Circle Square Center Vertical Address Register 0*/
#define RA8875_REG_DEVR1	0xA8 /*Draw Ellipse/Circle Square Center Vertical Address Register 1*/
#define RA8875_REG_DTPH0	0xA9 /*Draw Triangle Point 2 Horizontal Address Register 0 */
#define RA8875_REG_DTPH1	0xAA /*Draw Triangle Point 2 Horizontal Address Register 1 */
#define RA8875_REG_DTPV0	0xAB /*Draw Triangle Point 2 Vertical Address Register 0*/
#define RA8875_REG_DTPV1	0xAC /*Draw Triangle Point 2 Vertical Address Register 1*/
#define RA8875_REG_SSAR0	0xB0 /*Source Starting Address REG 0*/
#define RA8875_REG_SSAR1	0xB1 /*Source Starting Address REG 1*/
#define RA8875_REG_SSAR2	0xB2 /*Source Starting Address REG 2*/
#define RA8875_REG_DTNR0	0xB4 /*Block Width REG 0(BWR0) / DMA Transfer Number REG 0*/
#define RA8875_REG_BWR1		0xB5 /*Block Width REG 1*/
#define RA8875_REG_DTNR1	0xB6 /*Block Width REG 0(BWR0) / DMA Transfer Number REG 1*/
#define RA8875_REG_BHR1		0xB7 /*Block Height REG 1*/
#define RA8875_REG_DTNR2	0xB8 /*Source Picture Width REG 0(SPWR0) / DMA Transfer Number REG 2*/
#define RA8875_REG_SPWR1	0xB9 /*Source Picture Width REG 1*/
#define RA8875_REG_DMACR	0xBF /*DMA Configuration REG*/
#define RA8875_REG_KSCR1	0xC0 /*Key-Scan Control Register 1*/
#define RA8875_REG_KSCR2	0xC1 /*Key-Scan Control Register 2*/
#define RA8875_REG_KSDR0	0xC2 /*Key-Scan Data Register */
#define RA8875_REG_KSDR1	0xC3 /*Key-Scan Data Register */
#define RA8875_REG_KSDR2	0xC4 /*Key-Scan Data Register */
#define RA8875_REG_GPIOX	0xC7 /*Extra General Purpose IO Register*/
#define RA8875_REG_FWSAXA0	0xD0 /*Floating Windows Start Address XA 0*/
#define RA8875_REG_FWSAXA1	0xD1 /*Floating Windows Start Address XA 1*/
#define RA8875_REG_FWSAYA0	0xD2 /*Floating Windows Start Address YA 0*/
#define RA8875_REG_FWSAYA1	0xD3 /*Floating Windows Start Address YA 1*/
#define RA8875_REG_FWW0		0xD4 /* Floating Windows Width 0*/
#define RA8875_REG_FWW1		0xD5 /* Floating Windows Width 1*/
#define RA8875_REG_FWH0		0xD6 /*Floating Windows Height 0*/
#define RA8875_REG_FWH1		0xD7 /*Floating Windows Height 1*/
#define RA8875_REG_FWDXA0	0xD8 /*Floating Windows Display X Address 0*/
#define RA8875_REG_FWDXA1	0xD9 /*Floating Windows Display X Address 1*/
#define RA8875_REG_FWDYA0	0xDA /*Floating Windows Display Y Address 0*/
#define RA8875_REG_FWDYA1	0xDB /*Floating Windows Display Y Address 1*/
#define RA8875_REG_SACS_MODE	0xE0 /*Serial Flash/ROM Direct Access Mode*/
#define RA8875_REG_SACS_ADDR	0xE1 /*Serial Flash/ROM Direct Access Mode Addres*/
#define RA8875_REG_SACS_DATA	0xE2 /*Serial Flash/ROM Direct Access Data Read*/
#define RA8875_REG_INTC1	0xF0 /*Interrupt Control Register 1*/
#define RA8875_REG_INTC2	0xF1 /*Interrupt Control Register 2*/

#endif                               