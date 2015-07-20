#include <stm32f10x.h>
#include <stm32f10x_spi.h>
#include "stm32f10x_i2c.h"
#include "ade7880_hw.h"
#include "ade7880_api.h"
#include <ms_common.h>


void Wait_7880_Ready(void);  
#if 0 == ADE7880_USE_I2C_HSDC
static void TIM4_Configuration(void);
#endif
static void soft_delay(vu32 ncount);

volatile u16 SPI_DMA_Table_serial_in[32];
volatile u32 SPI_DMA_HSCD_BUFFER[16];
volatile u32 AI_HSCD_BUFFER[40];
volatile u32 AV_HSCD_BUFFER[40];
volatile u32 BI_HSCD_BUFFER[40];
volatile u32 BV_HSCD_BUFFER[40];
volatile u32 CI_HSCD_BUFFER[40];
volatile u32 CV_HSCD_BUFFER[40];
volatile s32 XFVAR_HSCD_BUFFER[3];
/* I2C ACK mask */
#define CR1_ACK_Set             ((uint16_t)0x0400)
#define CR1_ACK_Reset           ((uint16_t)0xFBFF)
/* I2C START mask */
#define CR1_START_Set           ((uint16_t)0x0100)
#define CR1_START_Reset         ((uint16_t)0xFEFF)
/* I2C STOP mask */
#define CR1_STOP_Set            ((uint16_t)0x0200)
#define CR1_STOP_Reset          ((uint16_t)0xFDFF)

#define CNT_TIM			(10000)
#if 0
void delay_ade7880_api(vu32 ncount)
{
  for(;ncount>0;ncount--);
}
#endif

void start_7880(void)
{
	struct gateway_em_st	 *em_info;
	unsigned int t_reg;
	int cnt;

	em_info = rt_malloc(sizeof(*em_info));
	if (NULL == em_info) {
		printf_syn("%s(), alloc mem fail\n", __FUNCTION__);
		return ;
	}

	printf_syn("func:%s, line:%d, start....\n", __FUNCTION__, __LINE__);

	if (RT_EOK != read_syscfgdata_tbl(SYSCFGDATA_TBL_GW_EM_INFO, 0, em_info)) {
		printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
		rt_free(em_info);
		return;
	}

	set_pabc_v_gain(em_info->chlx_st[0].pa_vgain, em_info->chlx_st[0].pb_vgain, em_info->chlx_st[0].pc_vgain);
	set_pabc_i_gain(em_info->chlx_st[0].pa_igain, em_info->chlx_st[0].pb_igain, em_info->chlx_st[0].pc_igain);
	set_pabc_xphcal(em_info->chlx_st[0].pa_phase, em_info->chlx_st[0].pb_phase, em_info->chlx_st[0].pc_phase);
	set_pabc_p_gain(em_info->chlx_st[0].pa_pgain, em_info->chlx_st[0].pb_pgain, em_info->chlx_st[0].pc_pgain);
	set_pabc_wattos(em_info->chlx_st[0].pa_wattos, em_info->chlx_st[0].pb_wattos, em_info->chlx_st[0].pc_wattos);
	set_pabc_vrmsos(em_info->chlx_st[0].pa_vrmsos, em_info->chlx_st[0].pb_vrmsos, em_info->chlx_st[0].pc_vrmsos);
	set_pabc_irmsos(em_info->chlx_st[0].pa_irmsos, em_info->chlx_st[0].pb_irmsos, em_info->chlx_st[0].pc_irmsos);
	set_vkcpu(em_info->chlx_st[0].pa_vk, em_info->chlx_st[0].pb_vk, em_info->chlx_st[0].pc_vk);
	set_ikcpu(em_info->chlx_st[0].pa_ik, em_info->chlx_st[0].pb_ik, em_info->chlx_st[0].pc_ik);
	set_pkcpu(em_info->chlx_st[0].pa_pk, em_info->chlx_st[0].pb_pk, em_info->chlx_st[0].pc_pk);
	set_cfxden(em_info->chlx_st[0].cf1den, em_info->chlx_st[0].cf2den, em_info->chlx_st[0].cf3den);
	set_p_wthr(em_info->chlx_st[0].pabc_wthr);
	set_p_varthr(em_info->chlx_st[0].pabc_varthr);

	rt_thread_delay(get_ticks_of_ms(40)); /* 上电后, 等待7880进入PM3模式 */

#if ADE7880_USE_SPI
	Work_Mode_Psm0();	/* 将7880设置为PM0模式 */

//	Hw_Reset_ADE7880();
	do {
		rt_thread_delay(get_ticks_of_ms(10));
		t_reg = Read_32bitReg_ADE7880(STATUS1_Register_Address);
	} while (!(t_reg & (1<<15)));
	Write_32bitReg_ADE7880(STATUS1_Register_Address, t_reg);

	Hw_Choose_SPI();

	rt_thread_delay(get_ticks_of_ms(85));
	Fast_Startup_ADE7880_ToMCModel();

	rt_thread_delay(get_ticks_of_ms(85));

	Wait_7880_Ready();
#else
	cnt = 0;
	do {
		rt_thread_delay(get_ticks_of_ms(10));
		t_reg = Read_32bitReg_ADE7880(STATUS1_Register_Address);

		if (++cnt > 100) {
			printf_syn("wait 7880 ready fail, will again...\n");
			cnt = 0;
		}
	} while (!(t_reg & (1<<15)));
	Write_32bitReg_ADE7880(STATUS1_Register_Address, t_reg);

	rt_thread_delay(get_ticks_of_ms(90));
	Fast_Startup_ADE7880_ToMCModel();

	rt_thread_delay(get_ticks_of_ms(90));
#if EM_ALL_TYPE_BASE 
	scan_wire_connect_mode();
#endif		  
	auto_set_powerup_workmode(0);  
#endif

#if 0 == ADE7880_USE_I2C_HSDC
	TIM4_Configuration();
#endif

#if EM_ALL_TYPE_BASE 
	px_vi_sample_reac_p_hsdc();
#endif	
	printf_syn("func:%s, line:%d, over....\n", __FUNCTION__, __LINE__);

	rt_free(em_info);
	return;
}

void start_7880_i2c(void)
{ 
#if 0
	start_7880();
#else
	Hw_Reset_ADE7880();
	Work_Mode_Psm0();	/* 将7880设置为PM0模式 */

	turnhigh_ade7880_cs();   
#endif
}

static void soft_delay(vu32 ncount)    //延时函数
{
  for(;ncount>0;ncount--);
}


void Work_Mode_Psm0(void)
{
	turnhigh_ade7880_PSM0_PM0(); /* ade7880正常工作模式 */
	turnlow_ade7880_PSM0_PM1();
}  
   
void Hw_Choose_SPI(void)
{   
	turnhigh_ade7880_cs(); /* 硬件选择ade7880通信方式 三个下降沿 */
	rt_thread_delay(get_ticks_of_ms(20));
	turnlow_ade7880_cs();
	rt_thread_delay(get_ticks_of_ms(20));

	turnhigh_ade7880_cs();
	rt_thread_delay(get_ticks_of_ms(20));
	turnlow_ade7880_cs();
	rt_thread_delay(get_ticks_of_ms(20));

	turnhigh_ade7880_cs();
	rt_thread_delay(get_ticks_of_ms(20));
	turnlow_ade7880_cs();
	rt_thread_delay(get_ticks_of_ms(20));

	turnhigh_ade7880_cs();
	rt_thread_delay(get_ticks_of_ms(20));

	/* 操作CONFIG2寄存器用于锁定SPI通信 */
	Write_8bitReg_ADE7880(CONFIG2_Register_Address,CONFIG2_Register_Data);
}

void Hw_Choose_I2C(void)
{   	
	turnhigh_ade7880_cs(); 
	rt_thread_delay(get_ticks_of_ms(20));

	/* 操作CONFIG2寄存器用于锁定I2C通信 */
	Write_8bitReg_ADE7880(CONFIG2_Register_Address,CONFIG2_Register_Data1);
}

void Hw_Reset_ADE7880(void)
{
	/*硬件选择ade7880通信方式I2C 硬件连接PA3 由高电平切换至低电平并且保持至少10us时间*/
	turnlow_ade7880_reset();
	soft_delay(20000);;
	//rt_thread_delay(get_ticks_of_ms(110));
	turnhigh_ade7880_reset();

}  

#if 0
void Fast_Startup_ADE7880_ToMCModel(void) 
{
	/*Step 1*/  /*选择相电流，电压，零线电流通道内PGA增益*/
	/*Gain寄存器中的位[2:0] (PGA1 电流通道)、位[5:3] (PGA2 零线电流通道)和位[8:6](PGA3 电压通道)*/
	Write_16bitReg_ADE7880(Gain_Register_Address,Gain_Register_Data);
	/*Step 2*/  /*若使用罗氏线圈，使能相电流或零线电流通道内的数字积分器*/
	Write_16bitReg_ADE7880(CONFIG_Register_Address,CONFIG_Register_Data);
	Write_8bitReg_ADE7880(CONFIG3_Register_Address,CONFIG3_Register_Data);
	/*Step 3*/  /*如果频率f为6 0Hz，将COMPMODE寄存器中的位14 (SELFREQ)置1*/
	Write_16bitReg_ADE7880(COMPMOD_Register_Address,COMPMOD_Register_Data);
	/*Step 4*/  /*基于公式初始化CF1DEN、CF2DEN和CF3DEN寄存器*/ //MArk! 怎么配
	Write_16bitReg_ADE7880(CF1DEN_Register_Address,CF1DEN_Register_Data);
	Write_16bitReg_ADE7880(CF2DEN_Register_Address,CF2DEN_Register_Data);
	Write_16bitReg_ADE7880(CF3DEN_Register_Address,CF3DEN_Register_Data);
	/*Step 5*/  /*基于公式初始化WTHR、VARTHR、VATHR、VLEVEL和VNOM寄存器*/
	Write_8bitReg_ADE7880(WTHR_Register_Address,WTHR_Register_Data);
	Write_8bitReg_ADE7880(VARTHR_Register_Address,VARTHR_Register_Data);
	Write_8bitReg_ADE7880(VATHR_Register_Address,VATHR_Register_Data);
	Write_32bitReg_ADE7880(VLEVEL_Register_Address,VLEVEL_Register_Data);
	Write_32bitReg_ADE7880(VNOM_Register_Address,VNOM_Register_Data);
	/*Step 6*/  /*使能数据存储器RAM保护 向地址0xE7FE写0xAD 向地址0xE7E3写0x80  */
	//SPI1_Write_32bitReg_ADE7880(VNOM_Register_Address,VNOM_Register_Data);
	//SPI1_Write_32bitReg_ADE7880(VNOM_Register_Address,VNOM_Register_Data);
	Write_8bitReg_ADE7880(0xE7FE,0xAD); //mark 返回E9 F8
	Write_8bitReg_ADE7880(0xE7E3,0x80);
	Write_8bitReg_ADE7880(0xE7E3,0x80);
	Write_8bitReg_ADE7880(0xE7E3,0x80);
	/*Step 7*/  /*设Run = 1，启动DSP*/
	//Write_16bitReg_ADE7880(Run_Register_Address,Run_Register_Data);
	Write_16bitReg_ADE7880(Run_Register_Address,Run_Register_Data);
}
#else
void Fast_Startup_ADE7880_ToMCModel(void)
{
	int a, b, c;

	/*Step 1*/  /*选择相电流，电压，零线电流通道内PGA增益*/
	/*Gain寄存器中的位[2:0] (PGA1 电流通道)、位[5:3] (PGA2 零线电流通道)和位[8:6](PGA3 电压通道)*/
	Write_16bitReg_ADE7880(Gain_Register_Address, Gain_Register_Data);

	/*Step 2*/  /*若使用罗氏线圈，使能相电流或零线电流通道内的数字积分器*/
#if ADE7880_USE_SPI
	Write_16bitReg_ADE7880(CONFIG_Register_Address, CONFIG_Register_Data);
#else
	/* hsdc 接口配置 */
	Write_8bitReg_ADE7880(HSDC_CFG_Register_Address, HSDC_CFG_Register_Data);
	rt_thread_delay(get_ticks_of_ms(100));
	Write_16bitReg_ADE7880(CONFIG_Register_Address, CONFIG_Register_Data);
#endif   
	rt_thread_delay(get_ticks_of_ms(100));

	Write_8bitReg_ADE7880(CONFIG3_Register_Address, CONFIG3_Register_Data);
	rt_thread_delay(get_ticks_of_ms(100));

	/*Step 3*/  /*如果频率f为6 0Hz，将COMPMODE寄存器中的位14 (SELFREQ)置1*/
	if(connect33_data == 1){
		Write_16bitReg_ADE7880(COMPMOD_Register_Address, COMPMOD_Register_Data1);
	}else if(connect33_data == 0){
		Write_16bitReg_ADE7880(COMPMOD_Register_Address, COMPMOD_Register_Data); 
	}
	rt_thread_delay(get_ticks_of_ms(100));
 
	/*Step 4*/  /*基于公式初始化CF1DEN、CF2DEN和CF3DEN寄存器*/ //MArk! 怎么配
	Write_16bitReg_ADE7880(CF1DEN_Register_Address, CF1DEN_Register_Data);
	rt_thread_delay(get_ticks_of_ms(100));
	Write_16bitReg_ADE7880(CF2DEN_Register_Address, CF2DEN_Register_Data);
	Write_16bitReg_ADE7880(CF3DEN_Register_Address, CF3DEN_Register_Data);
	rt_thread_delay(get_ticks_of_ms(100));

	/*Step 5*/  /*基于公式初始化WTHR、VARTHR、VATHR、VLEVEL和VNOM寄存器*/
	Write_8bitReg_ADE7880(WTHR_Register_Address, WTHR_Register_Data);
	Write_8bitReg_ADE7880(VARTHR_Register_Address, VARTHR_Register_Data);
	rt_thread_delay(get_ticks_of_ms(100));
	Write_8bitReg_ADE7880(VATHR_Register_Address, VATHR_Register_Data);
	rt_thread_delay(get_ticks_of_ms(100));

	Write_32bitReg_ADE7880(VLEVEL_Register_Address, VLEVEL_Register_Data);
	rt_thread_delay(get_ticks_of_ms(100));
	Write_32bitReg_ADE7880(VNOM_Register_Address, VNOM_Register_Data);
	rt_thread_delay(get_ticks_of_ms(100));
 
	/* 电能相关设置 */
	Write_16bitReg_ADE7880(CFMODE_Register_Address,CFMODE_Register_Data); /* 设置CF1管脚输出频率与总有功功率成正比 */
	rt_thread_delay(get_ticks_of_ms(100));

	if(connect33_data == 1){
		Write_8bitReg_ADE7880(ACCMODE_Register_Address, ACCMODE_Register_Data);
	}else if(connect33_data == 0){
		Write_8bitReg_ADE7880(ACCMODE_Register_Address, ACCMODE_Register_Data1);
	}
	rt_thread_delay(get_ticks_of_ms(100));

	/*Step 6*/  /*使能数据存储器RAM保护 向地址0xE7FE写0xAD 向地址0xE7E3写0x80  */
	//Write_32bitReg_ADE7880(VNOM_Register_Address,VNOM_Register_Data);
	//Write_32bitReg_ADE7880(VNOM_Register_Address,VNOM_Register_Data);
	Write_8bitReg_ADE7880(0xE7FE, 0xAD); //mark 返回E9 F8
	rt_thread_delay(get_ticks_of_ms(100));
	Write_16bitReg_ADE7880(0xE90C, 0x3BD); /*test*/
	Write_8bitReg_ADE7880(0xE7EF, 0x00);
	rt_thread_delay(get_ticks_of_ms(100));
	
	Write_32bitReg_ADE7880(MASK1_Register_Address, MASK1_Register_Data);
	Write_32bitReg_ADE7880(MASK1_Register_Address, MASK1_Register_Data);
	rt_thread_delay(get_ticks_of_ms(100));
	Write_32bitReg_ADE7880(MASK1_Register_Address, MASK1_Register_Data);


	/*Step 7*/  /*设Run = 1，启动DSP*/
	Write_16bitReg_ADE7880(Run_Register_Address,Run_Register_Data);

	rt_thread_delay(get_ticks_of_ms(100));
	/* 去能dsp写保护 */
	Write_8bitReg_ADE7880(0xE7FE, 0xAD);
	Write_8bitReg_ADE7880(0xE7E3, 0x00);

	rt_thread_delay(get_ticks_of_ms(100));
	get_pabc_v_gain(&a, &b, &c); 
	Write_32bitReg_ADE7880(AVGAIN_Register_Address, a);
	Write_32bitReg_ADE7880(BVGAIN_Register_Address, b);
	rt_thread_delay(get_ticks_of_ms(100));
	Write_32bitReg_ADE7880(CVGAIN_Register_Address, c);
	rt_thread_delay(get_ticks_of_ms(100));

	get_pabc_i_gain(&a, &b, &c);
	Write_32bitReg_ADE7880(AIGAIN_Register_Address, a);
	rt_thread_delay(get_ticks_of_ms(100));
	Write_32bitReg_ADE7880(BIGAIN_Register_Address, b);
	Write_32bitReg_ADE7880(CIGAIN_Register_Address, c);
	rt_thread_delay(get_ticks_of_ms(100));
	
	get_pabc_p_gain(&a, &b, &c);
	Write_32bitReg_ADE7880(APGAIN_Register_Address, a);
	Write_32bitReg_ADE7880(BPGAIN_Register_Address, b);
	rt_thread_delay(get_ticks_of_ms(100));
	Write_32bitReg_ADE7880(CPGAIN_Register_Address, c);	
	rt_thread_delay(get_ticks_of_ms(100));

	get_pabc_wattos_gain(&a, &b, &c); 
	Write_32bitReg_ADE7880(AWATTOS_Register_Address, a);
	rt_thread_delay(get_ticks_of_ms(100));
	Write_32bitReg_ADE7880(BWATTOS_Register_Address, b);
	Write_32bitReg_ADE7880(CWATTOS_Register_Address, c);	
	rt_thread_delay(get_ticks_of_ms(100));

	get_pabc_vrmsos_gain(&a, &b, &c); 
	Write_32bitReg_ADE7880(AVRMSOS_Register_Address, a);
	Write_32bitReg_ADE7880(BVRMSOS_Register_Address, b);
	rt_thread_delay(get_ticks_of_ms(100));
	Write_32bitReg_ADE7880(CVRMSOS_Register_Address, c);	
	rt_thread_delay(get_ticks_of_ms(100));
 
	get_pabc_irmsos_gain(&a, &b, &c); 
	Write_32bitReg_ADE7880(AIRMSOS_Register_Address, a);
	rt_thread_delay(get_ticks_of_ms(100));
	Write_32bitReg_ADE7880(BIRMSOS_Register_Address, b);
	Write_32bitReg_ADE7880(CIRMSOS_Register_Address, c);	
	rt_thread_delay(get_ticks_of_ms(100));

	get_cfxden_gain(&a, &b, &c); 
	Write_16bitReg_ADE7880(CF1DEN_Register_Address, a);   
	Write_16bitReg_ADE7880(CF2DEN_Register_Address, b);
	rt_thread_delay(get_ticks_of_ms(100));
	Write_16bitReg_ADE7880(CF3DEN_Register_Address, c);
	rt_thread_delay(get_ticks_of_ms(100));

	get_pabc_xphcal(&a, &b, &c);   
	Write_16bitReg_ADE7880(APHCAL_Register_Address, a);   
	rt_thread_delay(get_ticks_of_ms(100));
	Write_16bitReg_ADE7880(BPHCAL_Register_Address, b);
	Write_16bitReg_ADE7880(CPHCAL_Register_Address, c);
	rt_thread_delay(get_ticks_of_ms(100));

	get_p_varthr_gain(&a);
	Write_8bitReg_ADE7880(VARTHR_Register_Address, a);	
	get_p_wthr_gain(&a);
	Write_8bitReg_ADE7880(WTHR_Register_Address, a);	
	rt_thread_delay(get_ticks_of_ms(100));
	Write_8bitReg_ADE7880(WTHR_Register_Address, a);
	Write_8bitReg_ADE7880(WTHR_Register_Address, a);
	rt_thread_delay(get_ticks_of_ms(100));
	
	/* 使能dsp写保护 */
	Write_8bitReg_ADE7880(0xE7FE, 0xAD);
	Write_8bitReg_ADE7880(0xE7E3, 0x80);

	//delay_ade7880(6000000);/*750ms*/ /*test*/
	rt_thread_delay(get_ticks_of_ms(600));

}
#endif


unsigned int Read_8bitReg_ADE7880(u16 address)
{
	uint8_t data;
	enum i2c_err_e i2c_ret_err;

#if ADE7880_USE_SPI		
	ade7880_get_spi();
	turnlow_ade7880_cs();/*SS管脚拉低 开始spi方式读取*/
		
	spix_send_byte(SPI_USE_BY_ADE7880, 0x01);
	spix_send_byte(SPI_USE_BY_ADE7880, (u8)(address>>8));
	spix_send_byte(SPI_USE_BY_ADE7880, (u8)(address));
	temp = spix_rec_byte(SPI_USE_BY_ADE7880, SPI_DUMMY_OF_ADE7880);
		
	turnhigh_ade7880_cs();/*SS管脚拉高 开始spi方式读取*/
	ade7880_release_spi();
#else	
	//printf_syn("NO:r1\n"); 
 	/* Enable the selected I2C peripheral */
	I2C_Cmd(I2C_USE_BY_ADE7880, ENABLE);

	i2c_ret_err = i2cx_read_byte(I2C_USE_BY_ADE7880, 0x71, 0x70, address, 16, &data);
	if (IEE_OK != i2c_ret_err) {
		data = 0;
		rt_kprintf("%s(), line:%d, read data fail(%d)\n", __FUNCTION__, __LINE__, i2c_ret_err);
	}

	/* Disable the selected I2C peripheral */
	//I2C2->CR1 &= CR1_PE_Reset;
	I2C_Cmd(I2C_USE_BY_ADE7880, DISABLE);
	//rt_thread_delay(get_ticks_of_ms(1));
	soft_delay(CNT_TIM);
#endif 
	return data;
} 

unsigned int Read_16bitReg_ADE7880(u16 address)
{
	uint16_t data;
	enum i2c_err_e i2c_ret_err;
	u32 ret_value;

#if ADE7880_USE_SPI
	ade7880_get_spi();
	turnlow_ade7880_cs();

	spix_send_byte(SPI_USE_BY_ADE7880, 0x01);
	spix_send_byte(SPI_USE_BY_ADE7880, (u8)(address>>8));
	spix_send_byte(SPI_USE_BY_ADE7880, (u8)(address));
	tc1 = spix_rec_byte(SPI_USE_BY_ADE7880, SPI_DUMMY_OF_ADE7880);
	tc2 = spix_rec_byte(SPI_USE_BY_ADE7880, SPI_DUMMY_OF_ADE7880);

	turnhigh_ade7880_cs();
	ade7880_release_spi();
#else
	//printf_syn("NO:r1\n");
	/* Enable the selected I2C peripheral */
	//I2C2->CR1 |= CR1_PE_Set; 
	I2C_Cmd(I2C_USE_BY_ADE7880, ENABLE);

	i2c_ret_err = i2cx_read_bytes(I2C_USE_BY_ADE7880, 0x71, 0x70, address, 16, (unsigned char *)&data, sizeof(data));
	if (IEE_OK != i2c_ret_err) {
		ret_value = 0;
		rt_kprintf("%s(), line:%d, read data fail(%d)\n", __FUNCTION__, __LINE__, i2c_ret_err);
	} else {
		ret_value = ms_ntohs(data);
	}

	//printf_syn("NO:r4\n");
	/* Disable the selected I2C peripheral */
	//I2C2->CR1 &= CR1_PE_Reset;
	I2C_Cmd(I2C_USE_BY_ADE7880, DISABLE);
	//rt_thread_delay(get_ticks_of_ms(1));
	soft_delay(CNT_TIM);
#endif

	return ret_value;
} 


unsigned int Read_32bitReg_ADE7880(u16 address)
{
	uint32_t data;
	enum i2c_err_e i2c_ret_err;
	u32 ret_value;

#if ADE7880_USE_SPI
	ade7880_get_spi();
	turnlow_ade7880_cs();

	spix_send_byte(SPI_USE_BY_ADE7880, 0x01);
	spix_send_byte(SPI_USE_BY_ADE7880, (u8)(address>>8));
	spix_send_byte(SPI_USE_BY_ADE7880, (u8)(address));
	tc1 = spix_rec_byte(SPI_USE_BY_ADE7880, SPI_DUMMY_OF_ADE7880);
	tc2 = spix_rec_byte(SPI_USE_BY_ADE7880, SPI_DUMMY_OF_ADE7880);
	tc3 = spix_rec_byte(SPI_USE_BY_ADE7880, SPI_DUMMY_OF_ADE7880);
	tc4 = spix_rec_byte(SPI_USE_BY_ADE7880, SPI_DUMMY_OF_ADE7880);

	turnhigh_ade7880_cs();
	ade7880_release_spi();
#else
	//printf_syn("NO:r1\n");
	/* Enable the selected I2C peripheral */
	//I2C2->CR1 |= CR1_PE_Set; 
	I2C_Cmd(I2C_USE_BY_ADE7880, ENABLE);

	i2c_ret_err = i2cx_read_bytes(I2C_USE_BY_ADE7880, 0x71, 0x70, address, 16, (unsigned char *)&data, sizeof(data));
	if (IEE_OK != i2c_ret_err) {
		ret_value = 0;
		rt_kprintf("%s(), line:%d, read data fail(%d)\n", __FUNCTION__, __LINE__, i2c_ret_err);
	} else {
		ret_value = ms_ntohl(data);
	}

	//printf_syn("NO:r4\n");	
	/* Disable the selected I2C peripheral */
	//I2C2->CR1 &= CR1_PE_Reset;
	I2C_Cmd(I2C_USE_BY_ADE7880, DISABLE);
	//rt_thread_delay(get_ticks_of_ms(1));
	soft_delay(CNT_TIM);
#endif

	return ret_value;
}



void Write_8bitReg_ADE7880(u16 address, u8 data)
{
	enum i2c_err_e i2c_ret_err;

#if ADE7880_USE_SPI
	ade7880_get_spi();
	turnlow_ade7880_cs();/*SS管脚拉低 开始spi方式写入*/

	spix_send_byte(SPI_USE_BY_ADE7880, 0x00);
	spix_send_byte(SPI_USE_BY_ADE7880, (address>>8));
	spix_send_byte(SPI_USE_BY_ADE7880, address);
	spix_send_byte(SPI_USE_BY_ADE7880, data);

	turnhigh_ade7880_cs();/*SS管脚拉高 结束spi方式写入*/
	ade7880_release_spi();
#else  
	//printf_syn("NO:w1\n");  
	/* Enable the selected I2C peripheral */
	//I2C2->CR1 |= CR1_PE_Set;
	I2C_Cmd(I2C_USE_BY_ADE7880, ENABLE);

	i2c_ret_err = i2cx_write_byte(I2C_USE_BY_ADE7880, 0x70, address, 16, data);
	if (IEE_OK != i2c_ret_err) {
		rt_kprintf("%s(), line:%d, read data fail(%d)\n", __FUNCTION__, __LINE__, i2c_ret_err);
	}

	I2C_Cmd(I2C_USE_BY_ADE7880, DISABLE);
	//rt_thread_delay(get_ticks_of_ms(1));
	soft_delay(CNT_TIM);
#endif
}

void Write_16bitReg_ADE7880(u16 address, u16 data)
{
	enum i2c_err_e i2c_ret_err;

#if ADE7880_USE_SPI
	ade7880_get_spi();
	turnlow_ade7880_cs();//SS管脚拉低 开始spi方式写入

	spix_send_byte(SPI_USE_BY_ADE7880, 0x00);
	spix_send_byte(SPI_USE_BY_ADE7880, (u8)(address>>8));
	spix_send_byte(SPI_USE_BY_ADE7880, (u8)(address));
	spix_send_byte(SPI_USE_BY_ADE7880, (u8)(data>>8));
	spix_send_byte(SPI_USE_BY_ADE7880, (u8)(data));

	turnhigh_ade7880_cs();
	ade7880_release_spi();

#else
  	//printf_syn("NO:w1\n");
	/* Enable the selected I2C peripheral */
	I2C_Cmd(I2C2, ENABLE); 

	data = ms_htons(data);
	i2c_ret_err = i2cx_write_bytes(I2C_USE_BY_ADE7880, 0x70, address, 16, (unsigned char *)&data, sizeof(data));
	if (IEE_OK != i2c_ret_err) {
		rt_kprintf("%s(), line:%d, read data fail(%d)\n", __FUNCTION__, __LINE__, i2c_ret_err);
	}

	I2C_Cmd(I2C_USE_BY_ADE7880, DISABLE);
	//rt_thread_delay(get_ticks_of_ms(1));
	soft_delay(CNT_TIM);
#endif
}

void Write_32bitReg_ADE7880(u16 address, u32 data)
{  
	enum i2c_err_e i2c_ret_err;

#if ADE7880_USE_SPI
	ade7880_get_spi();
	turnlow_ade7880_cs();
 
	spix_send_byte(SPI_USE_BY_ADE7880, 0x00);
	spix_send_byte(SPI_USE_BY_ADE7880, (u8)(address>>8));
	spix_send_byte(SPI_USE_BY_ADE7880, (u8)(address));
 	spix_send_byte(SPI_USE_BY_ADE7880, (u8)(data>>24));
	spix_send_byte(SPI_USE_BY_ADE7880, (u8)(data>>16));
	spix_send_byte(SPI_USE_BY_ADE7880, (u8)(data>>8));
	spix_send_byte(SPI_USE_BY_ADE7880, (u8)(data));

	turnhigh_ade7880_cs();
	ade7880_release_spi();
#else
	//printf_syn("NO:w1\n");
	/* Enable the selected I2C peripheral */
	//I2C2->CR1 |= CR1_PE_Set;
	I2C_Cmd(I2C_USE_BY_ADE7880, ENABLE);

	data = ms_htonl(data);
	i2c_ret_err = i2cx_write_bytes(I2C_USE_BY_ADE7880, 0x70, address, 16, (unsigned char *)&data, sizeof(data));
	if (IEE_OK != i2c_ret_err) {
		rt_kprintf("%s(), line:%d, read data fail(%d)\n", __FUNCTION__, __LINE__, i2c_ret_err);
	}


	I2C_Cmd(I2C_USE_BY_ADE7880, DISABLE);
	//rt_thread_delay(get_ticks_of_ms(1));	 
	soft_delay(CNT_TIM);
#endif
}    
  
void Sw_Reset_ADE7880(void)
{
	int  value;

	value = Read_16bitReg_ADE7880(CONFIG_Register_Address);
	Write_16bitReg_ADE7880(CONFIG_Register_Address, SET(value , 7));

	//delay_ade7880_api(200000); /*15ms*/ /* mark by David */
	rt_thread_delay(get_ticks_of_ms(30));

	Fast_Startup_ADE7880_ToMCModel();

	return;
}

void ade7880_spi_cfg(void)
{
	SPI_InitTypeDef   SPI_InitStructure;

	/*7880 spi 时钟速率最大2.5M spi时钟分频系数32 得spi时钟速率2.25M */
	SPI_Cmd(ADE7880_SPIX, DISABLE);     			 /*必须先禁能,才能改变MODE*/

	SPI_InitStructure.SPI_Direction		= SPI_Direction_2Lines_FullDuplex; /*SPI设置为双线双向全双工*/
	SPI_InitStructure.SPI_Mode		= SPI_Mode_Master;      /*设置为主SPI*/
	SPI_InitStructure.SPI_DataSize		= SPI_DataSize_8b;  /*8位帧结构*/
	SPI_InitStructure.SPI_CPOL		= SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA		= SPI_CPHA_2Edge;       /*数据捕获于第1个时钟沿*/
	SPI_InitStructure.SPI_NSS		= SPI_NSS_Soft;          /*软件控制NSS信号*/
	SPI_InitStructure.SPI_BaudRatePrescaler	= SPI_BaudRatePrescaler_32; /*比特率预分频值为4*/
	SPI_InitStructure.SPI_FirstBit		= SPI_FirstBit_MSB;   /*数据大头传输*/
	SPI_InitStructure.SPI_CRCPolynomial	= 7;  	 /*定义用于CRC值计算的多项式7*/
	SPI_Init(ADE7880_SPIX, &SPI_InitStructure);

	SPI_Cmd(ADE7880_SPIX, ENABLE);

	return;
}

void ade7880_spi_hsdccfg(void)
{ 
	SPI_InitTypeDef   SPI_InitStructure;

	/* 作为ade7880 hsdc接口接收数据 hsdc时钟频率8M */
	SPI_Cmd(ADE7880_SPIX, DISABLE);     			 /*必须先禁能,才能改变MODE*/

	SPI_InitStructure.SPI_Direction		= SPI_Direction_2Lines_FullDuplex; /*SPI设置为双线双向全双工*/
	SPI_InitStructure.SPI_Mode		= SPI_Mode_Master;      /*设置为主SPI*/
	SPI_InitStructure.SPI_DataSize		= SPI_DataSize_16b;  /*8位帧结构*/
	SPI_InitStructure.SPI_CPOL		= SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA		= SPI_CPHA_2Edge;       /*数据捕获于第1个时钟沿*/
	SPI_InitStructure.SPI_NSS		= SPI_NSS_Soft;          /*软件控制NSS信号*/
	SPI_InitStructure.SPI_BaudRatePrescaler	= SPI_BaudRatePrescaler_8; /*比特率预分频值为4*/
	SPI_InitStructure.SPI_FirstBit		= SPI_FirstBit_MSB;   /*数据大头传输*/
	SPI_InitStructure.SPI_CRCPolynomial	= 7;  	 /*定义用于CRC值计算的多项式7*/
	SPI_Init(ADE7880_SPIX, &SPI_InitStructure);

	SPI_Cmd(ADE7880_SPIX, ENABLE);

	return;
} 

void ade7880_spi_withdma_hsdccfg(void)
{
	SPI_InitTypeDef  SPI_InitStructure;
  
	SPI_I2S_DeInit(ADE7880_SPIX);
	SPI_Cmd(ADE7880_SPIX, DISABLE);//必须先禁能,才能改变MODE
	SPI_InitStructure.SPI_Direction         = SPI_Direction_1Line_Rx; //SPI_Direction_1Line_Tx, SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode              = SPI_Mode_Slave;  
	SPI_InitStructure.SPI_DataSize          = SPI_DataSize_16b;  
	SPI_InitStructure.SPI_CPOL              = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA              = SPI_CPHA_2Edge;  
	SPI_InitStructure.SPI_NSS               = SPI_NSS_Soft;   
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
	SPI_InitStructure.SPI_FirstBit          = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial     = 7;
	SPI_Init(ADE7880_SPIX, &SPI_InitStructure);     
  
	SPI_I2S_DMACmd(ADE7880_SPIX, SPI_I2S_DMAReq_Rx, ENABLE);
	//SPI_I2S_DMACmd(ADE7880_SPIX, SPI_I2S_DMAReq_Tx, ENABLE);
	SPI_Cmd(ADE7880_SPIX, ENABLE);
}   
            
void dma_configuration_spi1_rx(void)  
{
	DMA_InitTypeDef  DMA_InitStructure;
            
	/* 允许 DMA1 */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	DMA_DeInit(DMA1_Channel2);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&(SPI1->DR));
	DMA_InitStructure.DMA_MemoryBaseAddr     = (u32)(&(SPI_DMA_Table_serial_in[0]));
	DMA_InitStructure.DMA_DIR                = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize         = 32;          
	DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;  
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_Word;
	DMA_InitStructure.DMA_Mode               = DMA_Mode_Normal; 
	DMA_InitStructure.DMA_Priority           = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_M2M                = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel2, &DMA_InitStructure);  
	DMA_Cmd(DMA1_Channel2, ENABLE);    
	DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, ENABLE);	//DMA2通道2传输完成中断
} 
  
void Wait_7880_Ready(void)
{
	u32 temp;

	printf_syn("test ade7880\n");

	px_virtual_mode_voltage(PHASE_A, (s32 *)&temp); /* 电压有效值ok */
	px_virtual_mode_voltage(PHASE_B, (s32 *)&temp); /* 电压有效值ok */
	px_virtual_mode_voltage(PHASE_C, (s32 *)&temp); /* 电压有效值ok */

	px_virtual_mode_current(PHASE_A, (s32_t *)&temp); /* 电流有效值ok */
	px_virtual_mode_current(PHASE_B, (s32_t *)&temp); /* 电流有效值ok */
	px_virtual_mode_current(PHASE_C, (s32_t *)&temp); /* 电流有效值ok */

	px_frequency_mode_signal(PHASE_A); /* 频率ok */
	px_frequency_mode_signal(PHASE_B); /* 频率ok */
	px_frequency_mode_signal(PHASE_C); /* 频率ok */

	px_phase_mode_position(PHASE_A); /* 相位ok */
	px_phase_mode_position(PHASE_B); /* 相位ok */
	px_phase_mode_position(PHASE_C); /* 相位ok */

	printf_syn("ade7880 working...\n");

	return;
}


#if 0 == ADE7880_USE_I2C_HSDC
static void TIM4_Configuration(void)
{
	TIM_TimeBaseInitTypeDef 	 TIM_TimeBaseStructure;

 	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
 	TIM_DeInit(TIM4);

	/* Time Base configuration */
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Prescaler = 72;
	TIM_TimeBaseStructure.TIM_Period =500; //17000-1; ---- 120/70 s
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	TIM_ARRPreloadConfig(TIM4, ENABLE);

  	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
  	TIM_Cmd(TIM4, DISABLE);
}
#endif

#if ADE7880_SI4432_SHARE_SPI
extern void cfg_spi_pin_used_by_ade7880(void);

int ade7880_get_spi(void)
{

	rt_sem_take(&spi_sem_share_by_ade7880, RT_WAITING_FOREVER);
	cfg_spi_pin_used_by_ade7880();
	ade7880_spi_cfg();

	return 0;

}

int ade7880_release_spi(void)
{
	SPI_Cmd(ADE7880_SPIX, DISABLE);
	rt_sem_release(&spi_sem_share_by_ade7880);

	return 0;
}
#endif
