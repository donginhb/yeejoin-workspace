/*
 * Description:
 * ade7880 spi通信方式相关代码
 * 在不同硬件平台下使用时需注意首先配置ade7880硬件抽象层有关信息
 *
 */
#include <stm32f10x.h>

#include <board.h>

#define SPI_USE_BY_ADE7880	ADE7880_SPIX
#define I2C_USE_BY_ADE7880	ADE7880_I2CX
#define SPI_DUMMY_OF_ADE7880	(0XFF)

/* ade7880硬件配置区 */
/*
#define ADE7880_CS_PORT GPIOA
#define ADE7880_CS_PIN GPIO_Pin_4
#define enable_ade7880()  clr_port_pin(port, pin)
#define disable_ade7880() set_port_pin(port, pin)
*/
#define turnhigh_ade7880_cs()        	set_port_pin(ADE7880_CS_PORT, ADE7880_CS_PIN)
#define turnlow_ade7880_cs()        	clr_port_pin(ADE7880_CS_PORT, ADE7880_CS_PIN)
#define turnhigh_ade7880_reset()        set_port_pin(ADE7880_RESET_GPIO, ADE7880_RESET_PIN)  //新版pc0
#define turnlow_ade7880_reset()         clr_port_pin(ADE7880_RESET_GPIO, ADE7880_RESET_PIN)

#define turnhigh_ade7880_PSM0_PM0()     set_port_pin(ADE7880_PM0_GPIO, ADE7880_PM0_PIN)//新版pc2
#define turnlow_ade7880_PSM0_PM1()      clr_port_pin(ADE7880_PM1_GPIO, ADE7880_PM1_PIN)//新版pc1

#define is_not_int_signal_creat() 	pin_in_is_set(ADE7880_INT1_PORT, ADE7880_INT1_PIN)

#define spi_i2s_send_hword(data)		((ADE7880_SPIX)->DR = (u16)data)
#define spi_i2s_recv_hword()			((ADE7880_SPIX)->DR)
#define is_spix_sr_flag_set_FLAG_TXE()		(0 != ((ADE7880_SPIX)->SR & (SPI_I2S_FLAG_TXE)))
#define is_spix_sr_flag_set_FLAG_RXNE(flag)	(0 != ((ADE7880_SPIX)->SR & (SPI_I2S_FLAG_RXNE)))

#define BIT(n)                      (1<<n)  //set_bit
#define SET(reg , n)                 reg |= BIT(n)
#define CLR(reg , n)                 reg &= ~(BIT(n))


/* 临时测试, vf4 */
#define VAL_A_CURRENT_TRANS(data) (data)
#define VAL_B_CURRENT_TRANS(data) (data)
#define VAL_C_CURRENT_TRANS(data) (data)

#define VAL_A_VOLTAGE_TRANS(data) (data)
#define VAL_B_VOLTAGE_TRANS(data) (data)
#define VAL_C_VOLTAGE_TRANS(data) (data)

#define VAL_A_APPARENT_POWER_TRANS(data) (data)
#define VAL_B_APPARENT_POWER_TRANS(data) (data)
#define VAL_C_APPARENT_POWER_TRANS(data) (data)
#define VAL_A_ACTIVE_POWER_TRANS(data) (data)
#define VAL_B_ACTIVE_POWER_TRANS(data) (data)
#define VAL_C_ACTIVE_POWER_TRANS(data) (data)


#define VAL_A_REACTIVE_POWER_TRANS(data) 	(data)
#define VAL_B_REACTIVE_POWER_TRANS(data) 	(data)
#define VAL_C_REACTIVE_POWER_TRANS(data) 	(data)

#define VAL_A_Factor_Power_TRANS(data) 		((10000*data)/32768)
#define VAL_B_Factor_Power_TRANS(data) 		((10000*data)/32768)
#define VAL_C_Factor_Power_TRANS(data) 		((10000*data)/32768)
#define VAL_A_Frequency_TRANS(data) 		(2560000/(data))
#define VAL_B_Frequency_TRANS(data) 		(2560000/(data))
#define VAL_C_Frequency_TRANS(data) 		(2560000/(data))
#define VAL_A_Phase_Position_TRANS(data) 	(((1406*(data))/2000))
#define VAL_B_Phase_Position_TRANS(data) 	(((1406*(data))/2000))
#define VAL_C_Phase_Position_TRANS(data) 	(((1406*(data))/2000))

#define VAL_VOLTAGE_DISTORTION_TRANS(data) 	((100*data)/8388608)
#define VAL_CURRENT_DISTORTION_TRANS(data) 	((100*data)/8388608)

/* 单位wh */
#define VAL_A_ACTIVE_ENERGY_TRANS(data) 	(data)
#define VAL_B_ACTIVE_ENERGY_TRANS(data) 	(data)
#define VAL_C_ACTIVE_ENERGY_TRANS(data) 	(data)

#define VAL_A_REACTIVE_ENERGY_TRANS(data) 	(data)
#define VAL_B_REACTIVE_ENERGY_TRANS(data) 	(data)
#define VAL_C_REACTIVE_ENERGY_TRANS(data) 	(data)

/*32bit reg*/
#define AVRMS_Register_Address 		(0x43C1) /* 电压有效值 */
#define BVRMS_Register_Address 		(0x43C3)
#define CVRMS_Register_Address 		(0x43C5)
#define AIRMS_Register_Address 		(0x43C0) /* 电流有效值 */
#define BIRMS_Register_Address 		(0x43C2)
#define CIRMS_Register_Address 		(0x43C4)
#define AVA_Register_Address 		(0xE519)  /* 视在功率的瞬时值 */
#define BVA_Register_Address 		(0xE51A)
#define CVA_Register_Address 		(0xE51B)
#define AWATT_Register_Address 		(0xE513)/* 总有功功率的瞬时值 */
#define BWATT_Register_Address 		(0xE514)
#define CWATT_Register_Address 		(0xE515)
#define VAWV_Register_Address 		(0xE510) /* 采样电压值 */
#define VBWV_Register_Address 		(0xE511)
#define VCWV_Register_Address 		(0xE512)
#define IAWV_Register_Address 		(0xE50C) /* 采样电流值 */
#define IBWV_Register_Address 		(0xE50D)
#define ICWV_Register_Address 		(0xE50E)
#define INWV_Register_Address 		(0xE50F)
#define VLEVEL_Register_Address 	(0x439F)/* 状态寄存器(配置电表模式) */
#define VLEVEL_Register_Data 		(0x000000)
#define VNOM_Register_Address 		(0xE520)
#define VNOM_Register_Data 		(0x000000)
#define STATUS0_Register_Address 	(0xE502)
#define STATUS1_Register_Address 	(0xE503)

#define FVAR_Register_Address 		(0xE883) /* 基波成分无功功率 */
#define VTHD_Register_Address 		(0xE886) /* 相电压总谐波失真 */
#define ITHD_Register_Address 		(0xE887) /* 相电流总谐波失真 */

#define AWATTHR_Register_Address 	(0xE400) /* A相总有功电能累计 */
#define BWATTHR_Register_Address 	(0xE401) /* B相总有功电能累计 */
#define CWATTHR_Register_Address 	(0xE402) /* C相总有功电能累计 */

#define AFVARHR_Register_Address 	(0xE409) /* A相总基波无功电能累计 */
#define BFVARHR_Register_Address 	(0xE40A) /* B相总基波无功电能累计 */
#define CFVARHR_Register_Address 	(0xE40B) /* C相总基波无功电能累计 */

#define HXVRMS_Register_Address		(0xE888) /* 相电压谐波X的有效值 */
#define HXIRMS_Register_Address		(0xE889) /* 相电流谐波X的有效值 */
#define HXWATT_Register_Address		(0xE88A) /* 谐波X的有功功率 */
#define HXVAR_Register_Address		(0xE88B) /* 谐波X的无功功率 */
#define HXVA_Register_Address		(0xE88C) /* 谐波X的视在功率 */
#define HXPF_Register_Address		(0xE88D) /* 谐波X的功率因数 */
#define HXVHD_Register_Address		(0xE88E) /* 相电压谐波X相对于基波的谐波失真 */
#define HXIHD_Register_Address		(0xE88F) /* 相电流谐波X相对于基波的谐波失真 */

#define HYVRMS_Register_Address		(0xE890) /* 相电压谐波Y的有效值 */
#define HYIRMS_Register_Address		(0xE891) /* 相电流谐波Y的有效值 */
#define HYWATT_Register_Address		(0xE892) /* 谐波Y的有功功率 */
#define HYVAR_Register_Address		(0xE893) /* 谐波Y的无功功率 */
#define HYVA_Register_Address		(0xE894) /* 谐波Y的视在功率 */
#define HYPF_Register_Address		(0xE895) /* 谐波Y的功率因数 */
#define HYVHD_Register_Address		(0xE896) /* 相电压谐波Y相对于基波的谐波失真 */
#define HYIHD_Register_Address		(0xE897) /* 相电流谐波Y相对于基波的谐波失真 */
 
#define HZVRMS_Register_Address		(0xE898) /* 相电压谐波Z的有效值 */
#define HZIRMS_Register_Address		(0xE899) /* 相电流谐波Z的有效值 */
#define HZWATT_Register_Address		(0xE89A) /* 谐波Z的有功功率 */
#define HZVAR_Register_Address		(0xE89B) /* 谐波Z的无功功率 */
#define HZVA_Register_Address		(0xE89C) /* 谐波Z的视在功率 */
#define HZPF_Register_Address		(0xE89D) /* 谐波Z的功率因数 */
#define HZVHD_Register_Address		(0xE89E) /* 相电压谐波Z相对于基波的谐波失真 */
#define HZIHD_Register_Address		(0xE89F) /* 相电流谐波Z相对于基波的谐波失真 */

/* 16bit reg */
//寄存器(地址0xE900)
#define HCONFIG_Register_Address 	(0xE900)/* 状态寄存器(配置电表模式) */
#define HCONFIG_Register_AVIn 		(0x00a0)/* 谐波A相输入 */
#define HCONFIG_Register_BVIn 		(0x01a2)
#define HCONFIG_Register_CVIn 		(0x02a4) 


#define Gain_Register_Address 		(0xE60F)/* 状态寄存器(配置电表模式) */
#define Gain_Register_Data 	    	(0x0004) /* IPAG16 */    
#define COMPMOD_Register_Address 	(0xE60E)
#define COMPMOD_Register_Data 		(0x03FF) //0x03ff
#define COMPMOD_Register_Data1 		(0x03ED) //0x03ff

#define CONFIG_Register_Address 	(0xE618)
#define CONFIG_Register_Data 		(0x0002)
#define CONFIG_Register_Data1 		(0x0042) //bit6 使能HSDC接口
#define CF1DEN_Register_Address 	(0xE611)
#define CF1DEN_Register_Data 		(0x030D)  /* 由公式计算得出，1lsb=0.0001wh */
#define CF2DEN_Register_Address 	(0xE612)
#define CF2DEN_Register_Data 		(0x030D)
#define CF3DEN_Register_Address 	(0xE613)
#define CF3DEN_Register_Data 		(0x0000)
#define Run_Register_Address 		(0xE228)
#define Run_Register_Data 		(0x0001)

#define MASK1_Register_Address 		(0xE50B) /* 中断管理器1 */
#define MASK1_Register_Data 		(0x00007E00) /* 使能AV,BV,CV,AI,BI,CI过零中断 */

#define APF_Register_Address 		(0xE902)/* A相功率因数 */
#define BPF_Register_Address 		(0xE903)
#define CPF_Register_Address 		(0xE904)
#define APERIOD_Register_Address 	(0xE905)/* A相周期 用以测频率 */
#define BPERIOD_Register_Address 	(0xE906)
#define CPERIOD_Register_Address 	(0xE907)
#define ANGLE0_Register_Address 	(0xE601)/* AB相电压相位差 */
#define ANGLE1_Register_Address 	(0xE602)
#define ANGLE2_Register_Address 	(0xE603)

#define HCONFIG_Register_Data_Choice_A 	(0x0008) /* 谐波引擎配置 */
#define HCONFIG_Register_Data_Choice_B 	(0x000A) /* 谐波引擎配置 */
#define HCONFIG_Register_Data_Choice_C 	(0x000C) /* 谐波引擎配置 */

#define CFMODE_Register_Address 	(0xE610)
#define CFMODE_Register_Data 		(0x08A0) /* 设置7880 CF1使能频率与总有功功率成正比 */
#define ACCMODE_Register_Address 	(0xE701) /* 用以配置电能计量的接线方式 */
#define ACCMODE_Register_Data 		(0x90)  	  /* 接三相三线 */
#define ACCMODE_Register_Data1 		(0x80)

#define APHCAL_Register_Address 	(0xE614) /* 相位补偿 */
#define BPHCAL_Register_Address 	(0xE615)
#define CPHCAL_Register_Address 	(0xE616)
/*8bit reg*/  
#define HSDC_CFG_Register_Address	(0xE706)/* 4M 16个32位字 ss输出引脚低电平有效 */
#define HSDC_CFG_Register_Data 		(0x05) 
#define CONFIG2_Register_Address 	(0xEC01)/* 状态寄存器(配置电表模式) */
#define CONFIG2_Register_Data 		(0x00) 
#define CONFIG2_Register_Data1 		(0x02)  
#define CONFIG3_Register_Address 	(0xEA00) 
#define CONFIG3_Register_Data 		(0x01)  
#define WTHR_Register_Address 		(0xEA02)
#define VARTHR_Register_Address 	(0xEA03)
#define VATHR_Register_Address 		(0xEA04)
#define VATHR_Register_Data 		(0x03)

#define WTHR_Register_Data 		(0x29)   /* 与有功电能计量有关， 1lsb=0.0001wh此处计算出为41.4 */
#define VARTHR_Register_Data 		(0x29)  /* 与基波无功电能计量有关，此处计算出为4.14 */

#define HX_Register_Address 		(0xEA08) /* 谐波计算监控的谐波的指数 */
#define HY_Register_Address 		(0xEA09)
#define HZ_Register_Address 		(0xEA0A)
#define USE_AMMETER_MEASURE_ERROR 0
#define TEST_8BIT_REG 0
#define TEST_16BIT_REG 0
#define TEST_32BIT_REG 0
#define MC_MODE_33 0

extern void delay_ade7880_api(vu32 ncount);
extern void USART1_Write_ByteToPc(u8 DATA);
extern void Work_Mode_Psm0(void);
extern void Hw_Choose_SPI(void);
extern void Hw_Reset_ADE7880(void);
extern void Fast_Startup_ADE7880_ToMCModel(void);
extern unsigned int Read_8bitReg_ADE7880(u16 address);
extern unsigned int Read_16bitReg_ADE7880(u16 address);
extern unsigned int Read_32bitReg_ADE7880(u16 address);
extern void Write_8bitReg_ADE7880(u16 address,u8 data);
extern void Write_16bitReg_ADE7880(u16 address,u16 data);
extern void Write_32bitReg_ADE7880(u16 address,u32 data);

extern void dma_configuration_spi1_rx(void);
extern void ade7880_spi_withdma_hsdccfg(void);
extern void start_7880_i2c(void);
extern void Sw_Reset_ADE7880(void);
extern int ade7880_get_spi(void);
extern int ade7880_release_spi(void);


#define AVGAIN_Register_Address (0x4381)   /* A相电压增益调整 */
#define BVGAIN_Register_Address (0x4383)   /* B相电压增益调整 */
#define CVGAIN_Register_Address (0x4385)   /* C相电压增益调整 */

#define AIGAIN_Register_Address (0x4380)   /* A相电压增益调整 */
#define BIGAIN_Register_Address (0x4382)   /* B相电压增益调整 */
#define CIGAIN_Register_Address (0x4384)   /* C相电压增益调整 */

#define APGAIN_Register_Address (0x4389)   /* A相功率增益调整, power gain adjust */
#define BPGAIN_Register_Address (0x438B)   /* B相功率增益调整 */
#define CPGAIN_Register_Address (0x438D)   /* C相功率增益调整 */

#define AVRMSOS_Register_Address (0x4390)
#define BVRMSOS_Register_Address (0x4392)
#define CVRMSOS_Register_Address (0x4394)

#define AIRMSOS_Register_Address (0x438F)
#define BIRMSOS_Register_Address (0x4391)
#define CIRMSOS_Register_Address (0x4393)

#define AWATTOS_Register_Address (0x438A)
#define BWATTOS_Register_Address (0x438C)
#define CWATTOS_Register_Address (0x438E)




#if 0
#define AVGAIN_Register_Data    (0x0F41893) /* A相电压增益调整 */
#define BVGAIN_Register_Data    (0x0F41893) /* B相电压增益调整 */
#define CVGAIN_Register_Data    (0x0F41893) /* C相电压增益调整 */
#else
//#define AVGAIN_Register_Data    (0x400000) /* A相电压增益调整 */
//#define AVGAIN_Register_Data    (0x000000) /* A相电压增益调整 */
#define AVGAIN_Register_Data    (0xffc00000) /* A相电压增益调整 */

#define BVGAIN_Register_Data    (0x000000) /* B相电压增益调整 */
//#define BVGAIN_Register_Data    (0x400000) /* B相电压增益调整 */

#define CVGAIN_Register_Data    (0x000000) /* C相电压增益调整 */
//#define CVGAIN_Register_Data    (0xff8be76d) /* C相电压增益调整 */
//#define CVGAIN_Register_Data    (0xC00000) /* C相电压增益调整 */

#endif
