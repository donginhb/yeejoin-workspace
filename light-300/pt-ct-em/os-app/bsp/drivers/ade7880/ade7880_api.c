#include <stm32f10x.h>
#include <stm32f10x_spi.h>

#include <ade7880_api.h>
#include <ade7880_hw.h>
  
#include <syscfgdata.h>
#include <finsh.h>
#include <sys_cfg_api.h>

#if EM_ALL_TYPE_BASE
#include <sink_info.h>
#include <ammeter.h>
#endif

#define ade7880_debug(x) 	//printf_syn x
#define check_ee_debug(x) printf_syn x

#define ade7880_delay_after_opt()	rt_thread_delay(5)
#define AUTODEBUG_V  0 
#define AUTODEBUG_I  1
#define AUTODEBUG_P  2
#define AUTODEBUG_PHASE  3
#define AUTODEBUG_VICLR 2

extern volatile u32 AI_HSCD_BUFFER[40];
extern volatile u32 AV_HSCD_BUFFER[40];
extern volatile u32 BI_HSCD_BUFFER[40];
extern volatile u32 BV_HSCD_BUFFER[40];
extern volatile u32 CI_HSCD_BUFFER[40];  
extern volatile u32 CV_HSCD_BUFFER[40];
extern volatile s32 XFVAR_HSCD_BUFFER[3];

/*
 * 采样值是有符号数
 * */
int pa_ade7880_sample_data[2][SINK_INFO_PX_SAMPLE_DOT_NUM];
int pb_ade7880_sample_data[2][SINK_INFO_PX_SAMPLE_DOT_NUM];
int pc_ade7880_sample_data[2][SINK_INFO_PX_SAMPLE_DOT_NUM];

volatile static int pa_vkcpu_data;
volatile static int pb_vkcpu_data;
volatile static int pc_vkcpu_data;

volatile static int pa_ikcpu_data;
volatile static int pb_ikcpu_data;
volatile static int pc_ikcpu_data;

volatile static int pa_pkcpu_data;
volatile static int pb_pkcpu_data;
volatile static int pc_pkcpu_data;

volatile int connect33_data;


volatile static int avgain_reg_data;
volatile static int bvgain_reg_data;
volatile static int cvgain_reg_data;

volatile static int aigain_reg_data;
volatile static int bigain_reg_data;
volatile static int cigain_reg_data;

volatile static int apgain_reg_data;
volatile static int bpgain_reg_data;
volatile static int cpgain_reg_data;

volatile static int awattos_reg_data;
volatile static int bwattos_reg_data;
volatile static int cwattos_reg_data;

volatile static int avrmsos_reg_data;
volatile static int bvrmsos_reg_data;
volatile static int cvrmsos_reg_data;

volatile static int airmsos_reg_data;
volatile static int birmsos_reg_data;
volatile static int cirmsos_reg_data;

volatile static int cf1den_reg_data;
volatile static int cf2den_reg_data;
volatile static int cf3den_reg_data;
volatile static int aphcal_reg_data;
volatile static int bphcal_reg_data;
volatile static int cphcal_reg_data;
volatile static int wthr_reg_data;
volatile static int varthr_reg_data;

volatile enum eenergy_check_state_e check_eenergy_state;

volatile unsigned int check_eenergy_timer_clk_period_cnt;

volatile unsigned long em_energy_timer_clks_cnt_start;
volatile unsigned long em_energy_timer_clks_cnt_end;

volatile unsigned long ade7880_energy_timer_clks_cnt_start;
volatile unsigned long ade7880_energy_timer_clks_cnt_end;
  
volatile int em_energy_int_cnt;
volatile int ade7880_energy_int_cnt;

s16_t em_act_ee_inaccuracy;		/* 实时有功电能误差 */
s16_t em_react_ee_inaccuracy;		/* 实时无功电能误差 */
extern volatile u32 hsdc_transcomp_flag; /* HSDC控制变量 */
extern volatile s32 XFVAR_HSCD_BUFFER[3];
extern volatile u32 SPI_DMA_Table_serial_in[32];

struct harmonic_parameter_st harmonic;

extern void set_decoder_3to8_data(unsigned data);
extern void set_decoder_4to16_data(unsigned data);
//static void px_vi_signal_sample_(int px);
//static void set_ade7880_signal_sample_cmd(enum ade7880_signal_sample_cmd_e cmd);
//static enum ade7880_signal_sample_cmd_e get_ade7880_signal_sample_cmd(void);
static void set_pabc_adj_reg(enum ade7880_adjust_reg_grp_id_e id);
static void auto_powup_con(int chlx);
static void reset_origgain_output(void);
static void get_average_value(int id);
#if EM_ALL_TYPE_BASE
int get_mc_from_645(int chlx);
#endif
static int autode_pxbf[3];
static int pa_vip_k, pb_vip_k, pc_vip_k;

#define val_out_auto_debug 0  
#define ADE7880_USE_AVERAGE_V 1
#define ADE7880_USE_AVERAGE_I 0


#if 1 /* use ade7880 stub */

void test_ade7880_read_reg(int *reg1, int *reg2, int *reg3)
{
	Write_8bitReg_ADE7880(0xE702,0x69);
	*reg1 = Read_8bitReg_ADE7880(0xE702);

	Write_16bitReg_ADE7880(0xE610,0x1231);
	*reg2 = Read_16bitReg_ADE7880(0xE610);

	Write_32bitReg_ADE7880(0xE520,0x5353a5a5);
	*reg3 = Read_32bitReg_ADE7880(0xE520);

	return;
}
#if 0
/*
 * return value: 单位mV  电压有效值
 */
unsigned int px_virtual_mode_voltage(int px)
{
	u32 flag, flag_justice, virtual_voltage, value_voltage_in;

	virtual_voltage  = 0;
	value_voltage_in = 0;
	switch (px) {
	case PHASE_A:
		flag_justice = Read_32bitReg_ADE7880(STATUS0_Register_Address); 	
		if(BIT(17) == (flag_justice&BIT(17))) {
			virtual_voltage = Read_32bitReg_ADE7880(AVRMS_Register_Address);
			flag = Read_32bitReg_ADE7880(STATUS0_Register_Address); 
		 	Write_32bitReg_ADE7880(STATUS0_Register_Address,SET(flag , 17)); 
	 	} else {
	 		printf_syn("fun:%s, line:%d\n", __FUNCTION__, __LINE__);
	 	}
	 	value_voltage_in = VAL_A_VOLTAGE_TRANS(virtual_voltage);
	 	
		break;

	case PHASE_B:
		flag_justice = Read_32bitReg_ADE7880(STATUS0_Register_Address); 	
		if(BIT(17) == (flag_justice&BIT(17))){      
			virtual_voltage = Read_32bitReg_ADE7880(BVRMS_Register_Address);
			flag = Read_32bitReg_ADE7880(STATUS0_Register_Address); 
		 	Write_32bitReg_ADE7880(STATUS0_Register_Address,SET(flag , 17)); 
	 	} else {
	 		printf_syn("fun:%s, line:%d\n", __FUNCTION__, __LINE__);
	 	}
	 	value_voltage_in = VAL_B_VOLTAGE_TRANS(virtual_voltage);
	 	
		break; 

	case PHASE_C:
		flag_justice = Read_32bitReg_ADE7880(STATUS0_Register_Address); 	
		if(BIT(17) == (flag_justice&BIT(17))){      
			virtual_voltage = Read_32bitReg_ADE7880(CVRMS_Register_Address);
			flag = Read_32bitReg_ADE7880(STATUS0_Register_Address); 
		 	Write_32bitReg_ADE7880(STATUS0_Register_Address,SET(flag , 17)); 
	 	} else {
	 		printf_syn("fun:%s, line:%d\n", __FUNCTION__, __LINE__);
	 	}
	 	value_voltage_in = VAL_C_VOLTAGE_TRANS(virtual_voltage);
	 	
		break;
		
	default:  
		printf_syn("fun:%s, line:%d\n", __FUNCTION__, __LINE__);
		break;
	}

	/*数据格式化*/
	ade7880_debug(("fun:%s, line:%d, val:%d\n", __FUNCTION__, __LINE__, value_voltage_in));

	ade7880_delay_after_opt();

	return value_voltage_in;  
}


/*
 * return value: 单位mA 电流有效值
 */
unsigned int px_virtual_mode_current(int px)
{
	u32 flag,flag_justice,virtual_current,value_current_in;
	
	virtual_current = 0;
	value_current_in = 0;
	switch (px) {
	case PHASE_A:
		flag_justice = Read_32bitReg_ADE7880(STATUS0_Register_Address); 	
		if(BIT(17) == (flag_justice&BIT(17))){      
			virtual_current = Read_32bitReg_ADE7880(AIRMS_Register_Address);  
			flag = Read_32bitReg_ADE7880(STATUS0_Register_Address); 
		 	Write_32bitReg_ADE7880(STATUS0_Register_Address,SET(flag , 17)); 
	 	} else {
	 		printf_syn("fun:%s, line:%d\n", __FUNCTION__, __LINE__);
	 	}
	 	value_current_in = VAL_A_CURRENT_TRANS(virtual_current);
		break;

	case PHASE_B:
		flag_justice = Read_32bitReg_ADE7880(STATUS0_Register_Address); 	
		if(BIT(17) == (flag_justice&BIT(17))){      
			virtual_current = Read_32bitReg_ADE7880(BIRMS_Register_Address);  
			flag = Read_32bitReg_ADE7880(STATUS0_Register_Address); 
		 	Write_32bitReg_ADE7880(STATUS0_Register_Address,SET(flag , 17)); 
	 	} else {
	 		printf_syn("fun:%s, line:%d\n", __FUNCTION__, __LINE__);
	 	}
	 	value_current_in = VAL_B_CURRENT_TRANS(virtual_current);
		break; 

	case PHASE_C:
		flag_justice = Read_32bitReg_ADE7880(STATUS0_Register_Address); 	
		if(BIT(17) == (flag_justice&BIT(17))){      
			virtual_current = Read_32bitReg_ADE7880(CIRMS_Register_Address);  
			flag = Read_32bitReg_ADE7880(STATUS0_Register_Address); 
		 	Write_32bitReg_ADE7880(STATUS0_Register_Address,SET(flag , 17)); 
	 	} else {
	 		printf_syn("fun:%s, line:%d\n", __FUNCTION__, __LINE__);
	 	}
		  
	 	value_current_in = VAL_C_CURRENT_TRANS(virtual_current);
		break;  
	default:
		printf_syn("fun:%s, line:%d\n", __FUNCTION__, __LINE__);
		break;
	}

	ade7880_debug(("fun:%s, line:%d, val:%d\n", __FUNCTION__, __LINE__, value_current_in));

	ade7880_delay_after_opt();

	return value_current_in;
}
#else

enum ade7880_signal_sample_cmd_e ade7880_sample_cmd;

#if ADE7880_USE_SPI
static int wait_int_signal(void)
{
	rt_tick_t tick;
	volatile rt_tick_t tick1;

	ade7880_debug(("fun:%s, line:%d\n", __FUNCTION__, __LINE__));

	tick = rt_tick_get();
	while (is_not_int_signal_creat()) {
		tick1 = rt_tick_get();
		if ((tick1 - tick) > get_ticks_of_ms(30)) {
			printf_syn("wait ade7880 int signal too long time\n ");
			return FAIL;
		}
	}

	ade7880_debug(("fun:%s, line:%d\n", __FUNCTION__, __LINE__));

	return SUCC;
}

static int wait_zero_crossing_event(u32_t bit_mask)
{
	int flag_test;
	rt_tick_t tick;
	volatile rt_tick_t tick1;

	ade7880_debug(("fun:%s, line:%d\n", __FUNCTION__, __LINE__));
	tick = rt_tick_get();
	do {
		flag_test = Read_32bitReg_ADE7880(STATUS1_Register_Address) & bit_mask;
		tick1 = rt_tick_get();
		if ((tick1 - tick) > get_ticks_of_ms(30)) {
			printf_syn("wait ade7880 zero crossing too long time, will sw_reset 7880\n ");
//			Sw_Reset_ADE7880();
			return FAIL;
		}

	} while (bit_mask != flag_test);
	ade7880_debug(("fun:%s, line:%d\n", __FUNCTION__, __LINE__));

	return SUCC;
}
#endif

int wait_dsp_complete(u32_t bit_mask)
{
	rt_tick_t tick;
	volatile rt_tick_t tick1;
	int flag_test;

//	ade7880_debug(("fun:%s, line:%d\n", __FUNCTION__, __LINE__));

	tick = rt_tick_get();
	do {
		flag_test = (Read_32bitReg_ADE7880(STATUS0_Register_Address) & bit_mask);
		tick1 = rt_tick_get();
		if ((tick1 - tick) > get_ticks_of_ms(30)) {
			printf_syn("wait ade7880 dsp complete too long time\n ");
			return FAIL;
		}
	} while(bit_mask != flag_test);

//	ade7880_debug(("fun:%s, line:%d\n", __FUNCTION__, __LINE__));

	return SUCC;
}

void auto_set_powerup_workmode(int chlx)
{
	/* 上电后自动判断接线方式及自动配置脉冲输出 */
	auto_powup_con(chlx);
}
#define DEBUG_PU_CON 0

#if EM_ALL_TYPE_BASE
static int pre_mc[NUM_OF_COLLECT_EM_MAX];
#endif

static void auto_powup_con(int chlx)
{
	int ret, mc;

	ret  = px_phase_mode_position(PHASE_A);
#if DEBUG_PU_CON
	printf_syn("tphase:Uac %d\n",ret);
#endif
	ret = ret + 2400;
#if DEBUG_PU_CON
	printf_syn("ret:Uac+2400 %d\n",ret);
#endif
#if EM_ALL_TYPE_BASE
	mc = get_mc_from_645(chlx);
//	mc = pre_mc[chlx];
	printf_syn("\n[CH-%d]EM(SN:%s) mc is %d\n\n", chlx+1, register_em_info.registered_em_sn[chlx], mc);
#else  
	mc = 12800;  
#endif
	if((ret < 2300)&(ret > 1500)){  
		printf_syn("auto detect 3-phase-3-wire system now\n");
		mode7880_con(1, mc, chlx);
	}else if(ret <= 1500){
		printf_syn("auto detect 3-phase-4-wire system now\n");
		mode7880_con(0, mc, chlx);
	}else if(ret >= 2300){  
		printf_syn("auto detect debug-mode now\n");
		mode7880_con(0, mc, chlx); 
	}else{  
		printf_syn("init connect error\n");	
	}
}

#define DEBUG_MC_GET 0

#if EM_ALL_TYPE_BASE
int get_mc_from_645(int chlx)
{    
	enum frame_error_e result = FRAME_E_ERROR;
	rt_uint32_t len = 0, num = 0;
	rt_uint8_t buf[4] = {'\0'}; 
	char data[3] = {'\0'};

	if (chlx<0 || chlx>=NUM_OF_COLLECT_EM_MAX) {
		printf_syn("%s() chlx(%d) invalid\n", __func__, chlx);
		return 0;
	}

	if (0 == register_em_info.registered_em_sn[chlx][0])
		return 0;
   
#if DEBUG_MC_GET
	printf_syn("sn[0]: 0x%x  sn[1]: 0x%x  sn[2]: 0x%x  sn[3]: 0x%x  sn[4]: 0x%x  sn[5]: 0x%x  \n",
		sn_bcd[0],sn_bcd[1],sn_bcd[2],sn_bcd[3],sn_bcd[4],sn_bcd[5]);
#endif    
	result = get_power_data_from_ammeter((rt_uint8_t *)register_em_info.registered_em_sn[chlx], AC_POWER_CONSTANT, buf, &len, AMMETER_UART1);
	if (result == FRAME_E_OK) {  
		data[0] = (buf[0]&0xf) + (buf[0]>>4)*10;
		data[1] = (buf[1]&0xf) + (buf[1]>>4)*10;
		data[2] = (buf[2]&0xf) + (buf[2]>>4)*10;
		num = data[0] + data[1]*100 + data[2]*10000;
		pre_mc[chlx] = num;
		printf_syn("\nget em(SN:%s) mc(%d) succ\n\n", register_em_info.registered_em_sn[chlx], num);
		
	} else {
		printf_syn("\nget em(SN:%s) mc fail\n\n", register_em_info.registered_em_sn[chlx]);
	}

	return pre_mc[chlx];
}
#endif
 
#define USE_VECTORGRAPH 1
#define DEBUG_VECTORGRAPH 0 
#if USE_VECTORGRAPH
static int clr_reg_bit(int reg, int n)
{
	reg &= ~(BIT(n));
	return reg;
}

static int set_reg_bit(int reg, int n)
{
	reg |= BIT(n);
	return reg;
}

static void turn_vv_phase_sample(void)
{
	int reg;

	reg = Read_16bitReg_ADE7880(COMPMOD_Register_Address);	
#if DEBUG_VECTORGRAPH
	printf_syn("orig reg 0x%x\n",reg);
#endif
	Write_16bitReg_ADE7880(COMPMOD_Register_Address, set_reg_bit(reg,9));
	reg = Read_16bitReg_ADE7880(COMPMOD_Register_Address);
#if DEBUG_VECTORGRAPH
	printf_syn("1st ch 0x%x\n",reg);
#endif
	rt_thread_delay(get_ticks_of_ms(600)); /* old value 50ms */
}

static void turn_vi_phase_sample(void)
{
	int reg;

	reg = Read_16bitReg_ADE7880(COMPMOD_Register_Address);
	Write_16bitReg_ADE7880(COMPMOD_Register_Address, clr_reg_bit(reg,9));
#if DEBUG_VECTORGRAPH  
	printf_syn("2st ch 0x%x\n",reg); 
#endif  
	rt_thread_delay(get_ticks_of_ms(600)); /* old value 50ms */
}
  
struct vectorgraph_st vg_st; 
void vi_vector_graph_sampl(void)
{
	turn_vi_phase_sample();
	vg_st.viap = px_phase_mode_position(PHASE_A);
	vg_st.vibp = px_phase_mode_position(PHASE_B);
	vg_st.vicp = px_phase_mode_position(PHASE_C);	

	turn_vv_phase_sample();
	vg_st.vvap = px_phase_mode_position(PHASE_A);
	vg_st.vvbp = px_phase_mode_position(PHASE_B);
	vg_st.vvcp = px_phase_mode_position(PHASE_C);

#if DEBUG_VECTORGRAPH  
	printf_syn("vectorgraph_st:\nvvap(Uac) %d vvbp(Ubc) %d vvcp(Uab) %d \nviap %d vibp %d vicp %d \n",
		vg_st.vvap, vg_st.vvbp, vg_st.vvcp,
		vg_st.viap, vg_st.vibp, vg_st.vicp);
#endif    
}
#endif


//#include <sink_info.h>
static void update_vi_reg(int id, int a, int b, int c)
{
	/* 去能dsp写保护 */
	Write_8bitReg_ADE7880(0xE7FE, 0xAD);
	Write_8bitReg_ADE7880(0xE7E3, 0x00);
 
	switch(id){
		case AUTODEBUG_V:
			Write_32bitReg_ADE7880(AVGAIN_Register_Address, a);
			Write_32bitReg_ADE7880(BVGAIN_Register_Address, b);
			Write_32bitReg_ADE7880(CVGAIN_Register_Address, c);
			printf_syn("update [xVGAIN] dsp reg finished...\n");

			break;

		case AUTODEBUG_I:
			Write_32bitReg_ADE7880(AIGAIN_Register_Address, a);
			Write_32bitReg_ADE7880(BIGAIN_Register_Address, b);
			Write_32bitReg_ADE7880(CIGAIN_Register_Address, c);
			printf_syn("update [xIGAIN] dsp reg finished...\n");

			break;

		case AUTODEBUG_VICLR:
			Write_32bitReg_ADE7880(AVGAIN_Register_Address, 0);
			Write_32bitReg_ADE7880(BVGAIN_Register_Address, 0);
			Write_32bitReg_ADE7880(CVGAIN_Register_Address, 0);
		        
			Write_32bitReg_ADE7880(AIGAIN_Register_Address, 0);
			Write_32bitReg_ADE7880(BIGAIN_Register_Address, 0);
			Write_32bitReg_ADE7880(CIGAIN_Register_Address, 0);			
			printf_syn(" debug the plus matching starting...\n ");

			break;
		default:
			break;

	}     
	
	/* 使能dsp写保护 */
	Write_8bitReg_ADE7880(0xE7FE, 0xAD);
	Write_8bitReg_ADE7880(0xE7E3, 0x80);	
	rt_thread_delay(get_ticks_of_ms(85));
}
static void cal_gain_matching(int id, int cnt)
{
	struct gateway_em_st	 *em_info;

	em_info = rt_malloc(sizeof(*em_info));
	if (NULL == em_info) {
		printf_syn("%s(), alloc mem fail\n", __FUNCTION__);
		return ;
	}

#if 0 == EM_ALL_TYPE_BASE			
	cnt = 0;
#endif
	pa_vip_k = 0;
	pb_vip_k = 0;
	pc_vip_k = 0;
	
	switch(id){ 
		case AUTODEBUG_V:
			if (autode_pxbf[0] >= autode_pxbf[1]){  
				pb_vip_k = ((autode_pxbf[0] - autode_pxbf[1])*1024*64/autode_pxbf[1])*128;
			}else if(autode_pxbf[0] < autode_pxbf[1]){
				pb_vip_k = ((autode_pxbf[0] - autode_pxbf[1])*1024*64/autode_pxbf[1])*128;
			}  	  
			 	   
			if (autode_pxbf[0] >= autode_pxbf[2]){
				pc_vip_k = ((autode_pxbf[0] - autode_pxbf[2])*1024*64/autode_pxbf[2])*128;
			}else if(autode_pxbf[0] < autode_pxbf[2]){
				pc_vip_k = ((autode_pxbf[0] - autode_pxbf[2])*1024*64/autode_pxbf[2])*128;
			}
			printf_syn("calculate [xVRMS] completed:%d, %d, %d(0x%x, 0x%x, 0x%x)\n",
					pa_vip_k, pb_vip_k, pc_vip_k,
					pa_vip_k, pb_vip_k, pc_vip_k);

			break;
		
		case AUTODEBUG_I:
			pa_vip_k = 0; pb_vip_k = 0; pc_vip_k = 0;
			if (autode_pxbf[0] >= autode_pxbf[1]){
				pb_vip_k = ((autode_pxbf[0] - autode_pxbf[1])*1024*64/autode_pxbf[1])*128;
			}else if(autode_pxbf[0] < autode_pxbf[1]){
				pb_vip_k = ((autode_pxbf[0] - autode_pxbf[1])*1024*64/autode_pxbf[1])*128;
			}	 
				 
			if (autode_pxbf[0] >= autode_pxbf[2]){
				pc_vip_k = ((autode_pxbf[0] - autode_pxbf[2])*1024*64/autode_pxbf[2])*128;
			}else if(autode_pxbf[0] < autode_pxbf[2]){
				pc_vip_k = ((autode_pxbf[0] - autode_pxbf[2])*1024*64/autode_pxbf[2])*128;
			} 
			printf_syn("calculate [xIRMS] completed:%d, %d, %d(0x%x, 0x%x, 0x%x)\n",
					pa_vip_k, pb_vip_k, pc_vip_k,
					pa_vip_k, pb_vip_k, pc_vip_k);
			break;

		default:
			break;
	}

 	if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_GW_EM_INFO, 0, em_info)) {
		printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);

	}

	switch(id){
		case AUTODEBUG_V:
			em_info->chlx_st[cnt].pa_vgain = pa_vip_k;
			em_info->chlx_st[cnt].pb_vgain = pb_vip_k;
			em_info->chlx_st[cnt].pc_vgain = pc_vip_k;
			
			break;
		case AUTODEBUG_I:
			em_info->chlx_st[cnt].pa_igain = pa_vip_k;
			em_info->chlx_st[cnt].pb_igain = pb_vip_k;
			em_info->chlx_st[cnt].pc_igain = pc_vip_k;			
			break;			
		default:
			break;

	} 


	if (RT_EOK != write_syscfgdata_tbl(SYSCFGDATA_TBL_GW_EM_INFO, 0, em_info)) {
		printf_syn("%s(), write syscfg data tbl fail\n", __FUNCTION__);

	}

	switch(id){
	case AUTODEBUG_V:
		set_pabc_v_gain(em_info->chlx_st[cnt].pa_vgain,  em_info->chlx_st[cnt].pb_vgain,  em_info->chlx_st[cnt].pc_vgain);
		printf_syn("will update [xVGAIN] dsp reg...\n");	
		break;
	case AUTODEBUG_I:
		set_pabc_i_gain(em_info->chlx_st[cnt].pa_igain,  em_info->chlx_st[cnt].pb_igain,  em_info->chlx_st[cnt].pc_igain);
		printf_syn("will update [xIGAIN] dsp reg...\n");
		break;			
	default:
		break;

	}

	rt_free(em_info);

	return;
}

static void reset_xphcal_reg(int chlx_num)
{
	int a, b, c;
#if 0 == EM_ALL_TYPE_BASE
	chlx_num = 0;
#endif
	struct gateway_em_st	 *em_info;
	em_info = rt_malloc(sizeof(*em_info));
	if (NULL == em_info) {
		printf_syn("%s(), alloc mem fail\n", __FUNCTION__);
		return ;
	}
	if (RT_EOK != read_syscfgdata_tbl(SYSCFGDATA_TBL_GW_EM_INFO, 0, em_info)) {
		printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
		rt_free(em_info);
		return;
	}  
	set_pabc_xphcal(em_info->chlx_st[chlx_num].pa_phase, em_info->chlx_st[chlx_num].pb_phase, em_info->chlx_st[chlx_num].pc_phase);
	Write_8bitReg_ADE7880(0xE7FE, 0xAD);
	Write_8bitReg_ADE7880(0xE7E3, 0x00);
	get_pabc_xphcal(&a, &b, &c);   
	Write_16bitReg_ADE7880(APHCAL_Register_Address, a);   
	Write_16bitReg_ADE7880(BPHCAL_Register_Address, b);
	Write_16bitReg_ADE7880(CPHCAL_Register_Address, c);
	Write_8bitReg_ADE7880(0xE7FE, 0xAD);
	Write_8bitReg_ADE7880(0xE7E3, 0x80);
	rt_thread_delay(get_ticks_of_ms(600));

	rt_free(em_info);
}
/*
 *  自动增益匹配函数:匹配电压，电流
 *  三相电压输入有效值 60v
 *
 */  
void px_gain_matching_autodebug(int chlx_cnt)   
{
#if EM_ALL_TYPE_BASE && EM_MULTI_BASE
	u8 cnt;
#endif
	reset_origgain_output();
/* 匹配电压有效值 */

	/* 采样四次计算电压有效值的均值 */
	get_average_value(AUTODEBUG_V);

	/* 依据计算的电压有效值均值计算匹配寄存器的值 */
	cal_gain_matching(AUTODEBUG_V, 0);	
	update_vi_reg(AUTODEBUG_V, pa_vip_k, pb_vip_k, pc_vip_k);
	get_average_value(AUTODEBUG_V);

/* 匹配电流有效值 */
#if EM_ALL_TYPE_BASE && EM_MULTI_BASE
	/* em多通道电流 */
	for(cnt = 0; cnt < chlx_cnt; cnt++){
		/* 切换电流通道 */
		printf_syn("\n\ndebug gain matching chlx num(0~11): %d, \n", cnt);
		reset_xphcal_reg(chlx_cnt);
		set_decoder_4to16_data(cnt);
		/* 采样四次计算电流有效值的均值 */
		get_average_value(AUTODEBUG_I);
		/* 依据计算的电压有效值均值计算匹配寄存器的值 */
		cal_gain_matching(AUTODEBUG_I, cnt);			
		update_vi_reg(AUTODEBUG_I, pa_vip_k, pb_vip_k, pc_vip_k);
		get_average_value(AUTODEBUG_I);	
	}
#else
	/* ct,pt单通道电流 */
	/* 采样四次计算电流有效值的均值 */
	get_average_value(AUTODEBUG_I);
	reset_xphcal_reg(0);  
	/* 依据计算的电压有效值均值计算匹配寄存器的值 */
	cal_gain_matching(AUTODEBUG_I, 0);	
	update_vi_reg(AUTODEBUG_I, pa_vip_k, pb_vip_k, pc_vip_k);
	get_average_value(AUTODEBUG_I);
#endif
}
 
/*  
 * 自动电压电流有效值展示校准函数:  注:未测试，调整常数放在CPU中
 * 输入参数:校准仪电压有效值及电流有效值
 * 输入条件:电压电流输入相同，功率因数1，电压标称60v，电流标称5A
 */    
#define ADDUPCNT (4)
#define COMPCNT  (100)   
#define TEST_AUTO_POWER 0  
#if 1

static void reset_origgain_output(void)
{
	update_vi_reg(AUTODEBUG_VICLR,0,0,0);
}

static void get_average_value(int id)
{
	u8 cnt;
	s32_t vx;	/* 电压, s24 -> 32zp */
	s32_t ix;	/* 电流, s24 -> 32zp */

	autode_pxbf[0] = 0;
	autode_pxbf[1] = 0;
	autode_pxbf[2] = 0;

	switch(id){
		case AUTODEBUG_V:
			for(cnt = 0; cnt < ADDUPCNT; cnt++ ){
				px_virtual_mode_voltage(PHASE_A, & vx);
				autode_pxbf[0] = autode_pxbf[0] +  vx;

				px_virtual_mode_voltage(PHASE_B, & vx);
				autode_pxbf[1] = autode_pxbf[1] +  vx;

				px_virtual_mode_voltage(PHASE_C, & vx);
				autode_pxbf[2] = autode_pxbf[2] +  vx;

				printf_syn("pa_v_val: %d, pb_v_val: %d, pc_v_val: %d, cnt: %d, \n", autode_pxbf[0], autode_pxbf[1], autode_pxbf[2], cnt);
  			}
			
			break;
			
		case AUTODEBUG_I:
			for(cnt = 0; cnt < ADDUPCNT; cnt++ ){
				px_virtual_mode_current(PHASE_A, & ix);
				autode_pxbf[0] = autode_pxbf[0] +  ix;

				px_virtual_mode_current(PHASE_B, & ix);
				autode_pxbf[1] = autode_pxbf[1] +  ix;

				px_virtual_mode_current(PHASE_C, & ix);
				autode_pxbf[2] = autode_pxbf[2] +  ix;

				printf_syn("pa_i_val: %d, pb_i_val: %d, pc_i_val: %d, cnt: %d, \n", autode_pxbf[0], autode_pxbf[1], autode_pxbf[2], cnt);
		  	}			
			break;
			
		case AUTODEBUG_P:
			for(cnt = 0; cnt < ADDUPCNT; cnt++ ){
				autode_pxbf[0] = autode_pxbf[0] + px_active_mode_power(PHASE_A);
				autode_pxbf[1] = autode_pxbf[1] + px_active_mode_power(PHASE_B);
				autode_pxbf[2] = autode_pxbf[2] + px_active_mode_power(PHASE_C);

				printf_syn("pa_p_val: %d, pb_p_val: %d, pc_p_val: %d, cnt: %d, \n", autode_pxbf[0], autode_pxbf[1], autode_pxbf[2], cnt);
		  	}  			
			break;
		default:
			break;

	}

	autode_pxbf[0] = autode_pxbf[0] >> 2;	
	autode_pxbf[1] = autode_pxbf[1] >> 2;	
	autode_pxbf[2] = autode_pxbf[2] >> 2; 

	switch(id){
		case AUTODEBUG_V:
			printf_syn("Finally...pa_v_val: %d, pb_v_val: %d, pc_v_val: %d\n", autode_pxbf[0], autode_pxbf[1], autode_pxbf[2]);
			
			break;
			
		case AUTODEBUG_I:
			printf_syn("Finally...pa_i_val: %d, pb_i_val: %d, pc_i_val: %d\n", autode_pxbf[0], autode_pxbf[1], autode_pxbf[2]);

			break;
			
		case AUTODEBUG_P:
			printf_syn("Finally...pa_p_val: %d, pb_p_val: %d, pc_p_val: %d\n", autode_pxbf[0], autode_pxbf[1], autode_pxbf[2]);

			break;
		default:
			break;

	}
}

static void careless_debug_pxk(int setout, int id)
{
	pa_vip_k = 0;
	pb_vip_k = 0;
	pc_vip_k = 0;
	switch(id){
		case AUTODEBUG_V:
			pa_vip_k = ((setout*1000) / autode_pxbf[0])*10;
			pb_vip_k = ((setout*1000) / autode_pxbf[1])*10;
			pc_vip_k = ((setout*1000) / autode_pxbf[2])*10; 
			printf_syn("Orig... pa_v_k: %d, pb_v_k: %d, pc_v_k: %d\n", pa_vip_k, pb_vip_k, pc_vip_k);
			
			break;
			
		case AUTODEBUG_I:
			pa_vip_k = ((setout*1000) / autode_pxbf[0])*10;
			pb_vip_k = ((setout*1000) / autode_pxbf[1])*10;
			pc_vip_k = ((setout*1000) / autode_pxbf[2])*10; 
			printf_syn("Orig... pa_i_k: %d, pb_i_k: %d, pc_i_k: %d\n", pa_vip_k, pb_vip_k, pc_vip_k);			

			break;
			
		case AUTODEBUG_P:
#if 1
			pa_vip_k = ((setout*10000) / autode_pxbf[0]);
			pb_vip_k = ((setout*10000) / autode_pxbf[1]);
			pc_vip_k = ((setout*10000) / autode_pxbf[2]); 
#else
			pa_vip_k = ((setout*1000) / autode_pxbf[0])*10;
			pb_vip_k = ((setout*1000) / autode_pxbf[1])*10;
			pc_vip_k = ((setout*1000) / autode_pxbf[2])*10; 
#endif 
			printf_syn("Orig... pa_p_k: %d, pb_p_k: %d, pc_p_k: %d\n", pa_vip_k, pb_vip_k, pc_vip_k);			

			break;
		default:
			break;

	}
	printf_syn("data prepare complete,start auto debug...\n");

}

static void careless_debug_pabck(int pa, int pb, int pc, int id)
{
	pa_vip_k = 0;
	pb_vip_k = 0;
	pc_vip_k = 0;
	switch(id){
		case AUTODEBUG_V:
			pa_vip_k = ((pa*1000) / autode_pxbf[0])*10;
			pb_vip_k = ((pb*1000) / autode_pxbf[1])*10;
			pc_vip_k = ((pc*1000) / autode_pxbf[2])*10; 
			printf_syn("Orig... pa_v_k: %d, pb_v_k: %d, pc_v_k: %d\n", pa_vip_k, pb_vip_k, pc_vip_k);
			
			break;
			
		case AUTODEBUG_I:
			pa_vip_k = ((pa*1000) / autode_pxbf[0])*10;
			pb_vip_k = ((pb*1000) / autode_pxbf[1])*10;
			pc_vip_k = ((pc*1000) / autode_pxbf[2])*10; 
			printf_syn("Orig... pa_i_k: %d, pb_i_k: %d, pc_i_k: %d\n", pa_vip_k, pb_vip_k, pc_vip_k);			

			break;
			
		case AUTODEBUG_P:
#if 1 
			pa_vip_k = ((pa*10000) / autode_pxbf[0]);
			pb_vip_k = ((pb*10000) / autode_pxbf[1]);
			pc_vip_k = ((pc*10000) / autode_pxbf[2]); 
#else
			pa_vip_k = ((setout*1000) / autode_pxbf[0])*10;
			pb_vip_k = ((setout*1000) / autode_pxbf[1])*10;
			pc_vip_k = ((setout*1000) / autode_pxbf[2])*10; 
#endif 
			printf_syn("Orig... pa_p_k: %d, pb_p_k: %d, pc_p_k: %d\n", pa_vip_k, pb_vip_k, pc_vip_k);			

			break;
		default:
			break;

	}
	printf_syn("data prepare complete,start auto debug...\n");

}


static void compare_caredebug(int v_setout, int dest_val, int *addr,int val, int id)
{
	u8 cnt, flag = 0;
	
	switch(val){  
		case 0:
			for(cnt = 0; cnt < COMPCNT; cnt++ ){
				if(flag == 0){
					if(v_setout > dest_val){
						*addr = *addr + 1;  
 						if(id == AUTODEBUG_P){
							dest_val = (*addr * autode_pxbf[0])/10000;			
						}else{
							dest_val = (*addr * (autode_pxbf[0]/10))/1000;	
						}
						if(v_setout <= dest_val){
							flag = 1;
							break;
						}  

					}else if(v_setout < dest_val){
						*addr = *addr - 1;
 						if(id == AUTODEBUG_P){
							dest_val = (*addr * autode_pxbf[0])/10000;			
						}else{
							dest_val = (*addr * (autode_pxbf[0]/10))/1000;	
						}		
						if(v_setout >= dest_val){
							flag = 1;
							break;
						}
					}
				}
			}			
			break;

		case 1:
			for(cnt = 0; cnt < COMPCNT; cnt++ ){
				if(flag == 0){
					if(v_setout > dest_val){
						*addr = *addr + 1;  
 						if(id == AUTODEBUG_P){
							dest_val = (*addr * autode_pxbf[1])/10000;			
						}else{
							dest_val = (*addr * (autode_pxbf[1]/10))/1000;	
						}
						
						if(v_setout <= dest_val){
							flag = 1;
							break;
						}

					}else if(v_setout < dest_val){
						*addr = *addr - 1;
 						if(id == AUTODEBUG_P){
							dest_val = (*addr * autode_pxbf[1])/10000;			
						}else{
							dest_val = (*addr * (autode_pxbf[1]/10))/1000;	
						}		
						if(v_setout >= dest_val){
							flag = 1;
							break;
						}
					}
				}
			}			
			break; 
  
		case 2:
			for(cnt = 0; cnt < COMPCNT; cnt++ ){
				if(flag == 0){
					if(v_setout > dest_val){
						*addr = *addr + 1;  
 						if(id == AUTODEBUG_P){
							dest_val = (*addr * autode_pxbf[2])/10000;			
						}else{
							dest_val = (*addr * (autode_pxbf[2]/10))/1000;	
						}				
						if(v_setout <= dest_val){
							flag = 1;
							break;
						}

					}else if(v_setout < dest_val){
						*addr = *addr - 1;
 						if(id == AUTODEBUG_P){
							dest_val = (*addr * autode_pxbf[2])/10000;			
						}else{
							dest_val = (*addr * (autode_pxbf[2]/10))/1000;	
						}		
						if(v_setout >= dest_val){
							flag = 1;
							break;
						}
					}
				}
			}			
			break;

		default:
			break;
	}	
}

static void careful_debug_pxk(int setout, int id)
{
	int com_avip, com_bvip, com_cvip;
	switch(id){
		case AUTODEBUG_V:
			com_avip = (pa_vip_k * (autode_pxbf[0]/10))/1000;
			com_bvip = (pb_vip_k * (autode_pxbf[1]/10))/1000;
			com_cvip = (pc_vip_k * (autode_pxbf[2]/10))/1000;
			printf_syn("frist auto debug completed...\n");
			printf_syn("v_setout: %d, com_av: %d, pa_v_k: %d\n", setout, com_avip, pa_vip_k);
			printf_syn("v_setout: %d, com_bv: %d, pb_v_k: %d\n", setout, com_bvip, pb_vip_k);
			printf_syn("v_setout: %d, com_cv: %d, pc_v_k: %d\n", setout, com_cvip, pc_vip_k);
		 	 
			printf_syn("second auto debug...\n");  
		  
			compare_caredebug(setout, com_avip, &pa_vip_k, 0, id);
			compare_caredebug(setout, com_bvip, &pb_vip_k, 1, id);
			compare_caredebug(setout, com_cvip, &pc_vip_k, 2, id);
		  
			com_avip = (pa_vip_k * (autode_pxbf[0]/10))/1000;
			com_bvip = (pb_vip_k * (autode_pxbf[1]/10))/1000;
			com_cvip = (pc_vip_k * (autode_pxbf[2]/10))/1000;
			printf_syn("second auto debug completed...\n");
			printf_syn("v_setout: %d, com_av: %d, pa_v_k: %d\n", setout, com_avip, pa_vip_k);
			printf_syn("v_setout: %d, com_bv: %d, pb_v_k: %d\n", setout, com_bvip, pb_vip_k);
			printf_syn("v_setout: %d, com_cv: %d, pc_v_k: %d\n", setout, com_cvip, pc_vip_k);
			break;
			
		case AUTODEBUG_I:
			com_avip = (pa_vip_k * (autode_pxbf[0]/10))/1000;
			com_bvip = (pb_vip_k * (autode_pxbf[1]/10))/1000;
			com_cvip = (pc_vip_k * (autode_pxbf[2]/10))/1000;
			printf_syn("frist auto debug completed...\n");
			printf_syn("i_setout: %d, com_ai: %d, pa_i_k: %d\n", setout, com_avip, pa_vip_k);
			printf_syn("i_setout: %d, com_bi: %d, pb_i_k: %d\n", setout, com_bvip, pb_vip_k);
			printf_syn("i_setout: %d, com_ci: %d, pc_i_k: %d\n", setout, com_cvip, pc_vip_k);
		 	 
			printf_syn("second auto debug...\n");
		  
			compare_caredebug(setout, com_avip, &pa_vip_k, 0, id);
			compare_caredebug(setout, com_bvip, &pb_vip_k, 1, id);
			compare_caredebug(setout, com_cvip, &pc_vip_k, 2, id);
		  
			com_avip = (pa_vip_k * (autode_pxbf[0]/10))/1000;
			com_bvip = (pb_vip_k * (autode_pxbf[1]/10))/1000;
			com_cvip = (pc_vip_k * (autode_pxbf[2]/10))/1000;
			printf_syn("second auto debug completed...\n");
			printf_syn("i_setout: %d, com_ai: %d, pa_i_k: %d\n", setout, com_avip, pa_vip_k);
			printf_syn("i_setout: %d, com_bi: %d, pb_i_k: %d\n", setout, com_bvip, pb_vip_k);
			printf_syn("i_setout: %d, com_ci: %d, pc_i_k: %d\n", setout, com_cvip, pc_vip_k);
			break;
			
		case AUTODEBUG_P:
#if 1 
			com_avip = (pa_vip_k * autode_pxbf[0])/10000;
			com_bvip = (pb_vip_k * autode_pxbf[1])/10000;
			com_cvip = (pc_vip_k * autode_pxbf[2])/10000;
#else
			com_avip = (pa_vip_k * (autode_pxbf[0]/10))/1000;
			com_bvip = (pb_vip_k * (autode_pxbf[1]/10))/1000;
			com_cvip = (pc_vip_k * (autode_pxbf[2]/10))/1000;
#endif
			printf_syn("frist auto debug completed...\n");
			printf_syn("p_setout: %d, com_ap: %d, pa_p_k: %d\n", setout, com_avip, pa_vip_k);
			printf_syn("p_setout: %d, com_bp: %d, pb_p_k: %d\n", setout, com_bvip, pb_vip_k);
			printf_syn("p_setout: %d, com_cp: %d, pc_p_k: %d\n", setout, com_cvip, pc_vip_k);
		 	 
			printf_syn("second auto debug...\n");  
		  
			compare_caredebug(setout, com_avip, &pa_vip_k, 0, id);
			compare_caredebug(setout, com_bvip, &pb_vip_k, 1, id);
			compare_caredebug(setout, com_cvip, &pc_vip_k, 2, id);
		  
#if 1  
			com_avip = (pa_vip_k * autode_pxbf[0])/10000;
			com_bvip = (pb_vip_k * autode_pxbf[1])/10000;
			com_cvip = (pc_vip_k * autode_pxbf[2])/10000;
#else
			com_avip = (pa_vip_k * (autode_pxbf[0]/10))/1000;
			com_bvip = (pb_vip_k * (autode_pxbf[1]/10))/1000;
			com_cvip = (pc_vip_k * (autode_pxbf[2]/10))/1000;
#endif
			printf_syn("second auto debug completed...\n");
			printf_syn("p_setout: %d, com_ap: %d, pa_p_k: %d\n", setout, com_avip, pa_vip_k);
			printf_syn("p_setout: %d, com_bp: %d, pb_p_k: %d\n", setout, com_bvip, pb_vip_k);
			printf_syn("p_setout: %d, com_cp: %d, pc_p_k: %d\n", setout, com_cvip, pc_vip_k);
			break;

		default:
			break;

	}


}

static void careful_debug_pabck(int pa, int pb, int pc, int id)
{
	int com_avip, com_bvip, com_cvip;
	switch(id){
		case AUTODEBUG_V:
			com_avip = (pa_vip_k * (autode_pxbf[0]/10))/1000;
			com_bvip = (pb_vip_k * (autode_pxbf[1]/10))/1000;
			com_cvip = (pc_vip_k * (autode_pxbf[2]/10))/1000;
			printf_syn("frist auto debug completed...\n");
			printf_syn("va_setout: %d, com_av: %d, pa_v_k: %d\n", pa, com_avip, pa_vip_k);
			printf_syn("vb_setout: %d, com_bv: %d, pb_v_k: %d\n", pb, com_bvip, pb_vip_k);
			printf_syn("vc_setout: %d, com_cv: %d, pc_v_k: %d\n", pc, com_cvip, pc_vip_k);
		 	 
			printf_syn("second auto debug...\n");  
		  
			compare_caredebug(pa, com_avip, &pa_vip_k, 0, id);
			compare_caredebug(pb, com_bvip, &pb_vip_k, 1, id);
			compare_caredebug(pc, com_cvip, &pc_vip_k, 2, id);
		  
			com_avip = (pa_vip_k * (autode_pxbf[0]/10))/1000;
			com_bvip = (pb_vip_k * (autode_pxbf[1]/10))/1000;
			com_cvip = (pc_vip_k * (autode_pxbf[2]/10))/1000;
			printf_syn("second auto debug completed...\n");
			printf_syn("v_setout: %d, com_av: %d, pa_v_k: %d\n", pa, com_avip, pa_vip_k);
			printf_syn("v_setout: %d, com_bv: %d, pb_v_k: %d\n", pb, com_bvip, pb_vip_k);
			printf_syn("v_setout: %d, com_cv: %d, pc_v_k: %d\n", pc, com_cvip, pc_vip_k);
			break;
			
		case AUTODEBUG_I:
			com_avip = (pa_vip_k * (autode_pxbf[0]/10))/1000;
			com_bvip = (pb_vip_k * (autode_pxbf[1]/10))/1000;
			com_cvip = (pc_vip_k * (autode_pxbf[2]/10))/1000;
			printf_syn("frist auto debug completed...\n");
			printf_syn("i_setout: %d, com_ai: %d, pa_i_k: %d\n", pa, com_avip, pa_vip_k);
			printf_syn("i_setout: %d, com_bi: %d, pb_i_k: %d\n", pb, com_bvip, pb_vip_k);
			printf_syn("i_setout: %d, com_ci: %d, pc_i_k: %d\n", pc, com_cvip, pc_vip_k);
		 	 
			printf_syn("second auto debug...\n");
		  
			compare_caredebug(pa, com_avip, &pa_vip_k, 0, id);
			compare_caredebug(pb, com_bvip, &pb_vip_k, 1, id);
			compare_caredebug(pc, com_cvip, &pc_vip_k, 2, id);
		  
			com_avip = (pa_vip_k * (autode_pxbf[0]/10))/1000;
			com_bvip = (pb_vip_k * (autode_pxbf[1]/10))/1000;
			com_cvip = (pc_vip_k * (autode_pxbf[2]/10))/1000;
			printf_syn("second auto debug completed...\n");
			printf_syn("i_setout: %d, com_ai: %d, pa_i_k: %d\n", pa, com_avip, pa_vip_k);
			printf_syn("i_setout: %d, com_bi: %d, pb_i_k: %d\n", pb, com_bvip, pb_vip_k);
			printf_syn("i_setout: %d, com_ci: %d, pc_i_k: %d\n", pc, com_cvip, pc_vip_k);
			break;
			
		case AUTODEBUG_P:
#if 1 
			com_avip = (pa_vip_k * autode_pxbf[0])/10000;
			com_bvip = (pb_vip_k * autode_pxbf[1])/10000;
			com_cvip = (pc_vip_k * autode_pxbf[2])/10000;
#else
			com_avip = (pa_vip_k * (autode_pxbf[0]/10))/1000;
			com_bvip = (pb_vip_k * (autode_pxbf[1]/10))/1000;
			com_cvip = (pc_vip_k * (autode_pxbf[2]/10))/1000;
#endif
			printf_syn("frist auto debug completed...\n");
			printf_syn("p_setout: %d, com_ap: %d, pa_p_k: %d\n", pa, com_avip, pa_vip_k);
			printf_syn("p_setout: %d, com_bp: %d, pb_p_k: %d\n", pb, com_bvip, pb_vip_k);
			printf_syn("p_setout: %d, com_cp: %d, pc_p_k: %d\n", pc, com_cvip, pc_vip_k);
		 	 
			printf_syn("second auto debug...\n");  
		  
			compare_caredebug(pa, com_avip, &pa_vip_k, 0, id);
			compare_caredebug(pb, com_bvip, &pb_vip_k, 1, id);
			compare_caredebug(pc, com_cvip, &pc_vip_k, 2, id);
		  
#if 1  
			com_avip = (pa_vip_k * autode_pxbf[0])/10000;
			com_bvip = (pb_vip_k * autode_pxbf[1])/10000;
			com_cvip = (pc_vip_k * autode_pxbf[2])/10000;
#else
			com_avip = (pa_vip_k * (autode_pxbf[0]/10))/1000;
			com_bvip = (pb_vip_k * (autode_pxbf[1]/10))/1000;
			com_cvip = (pc_vip_k * (autode_pxbf[2]/10))/1000;
#endif
			printf_syn("second auto debug completed...\n");
			printf_syn("p_setout: %d, com_ap: %d, pa_p_k: %d\n", pa, com_avip, pa_vip_k);
			printf_syn("p_setout: %d, com_bp: %d, pb_p_k: %d\n", pb, com_bvip, pb_vip_k);
			printf_syn("p_setout: %d, com_cp: %d, pc_p_k: %d\n", pc, com_cvip, pc_vip_k);
			break;

		default:
			break;

	}


}


static void write_debug_into_flash(int id, int chlx)
{
	struct gateway_em_st	 *em_info;

	em_info = rt_malloc(sizeof(*em_info));
	if (NULL == em_info) {
		printf_syn("%s(), alloc mem fail\n", __FUNCTION__);
		return ;
	}

#if 0 == EM_ALL_TYPE_BASE			
	chlx = 0;
#endif
	/* 电压有效值相关参数写入flash */
	printf_syn("will update dsp reg...\n");

 	if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_GW_EM_INFO, 0, em_info)) {
		printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
	}

	switch(id){
		case AUTODEBUG_V:
			em_info->chlx_st[chlx].pa_vk = pa_vip_k;
			em_info->chlx_st[chlx].pb_vk = pb_vip_k;
			em_info->chlx_st[chlx].pc_vk = pc_vip_k;			

			break;
			
		case AUTODEBUG_I:
			em_info->chlx_st[chlx].pa_ik = pa_vip_k;
			em_info->chlx_st[chlx].pb_ik = pb_vip_k;
			em_info->chlx_st[chlx].pc_ik = pc_vip_k;			

			break;
			
		case AUTODEBUG_P:
			em_info->chlx_st[chlx].pa_pk = pa_vip_k;
			em_info->chlx_st[chlx].pb_pk = pb_vip_k;
			em_info->chlx_st[chlx].pc_pk = pc_vip_k;

			break;

		case AUTODEBUG_PHASE:
			em_info->chlx_st[chlx].pa_phase = pa_vip_k;
			em_info->chlx_st[chlx].pb_phase = pb_vip_k;
			em_info->chlx_st[chlx].pc_phase = pc_vip_k;
			break;
		default:
			break;

	}
    

	if (RT_EOK != write_syscfgdata_tbl(SYSCFGDATA_TBL_GW_EM_INFO, 0, em_info)) {
		printf_syn("%s(), write syscfg data tbl fail\n", __FUNCTION__);
	} 
	
	switch(id){
		case AUTODEBUG_V:
			set_vkcpu(pa_vip_k, pb_vip_k, pc_vip_k);
			printf_syn("update p_v_k dsp reg finished...\n");
			break;
			
		case AUTODEBUG_I:
		 	set_ikcpu(pa_vip_k, pb_vip_k, pc_vip_k);
			printf_syn("update p_i_k dsp reg finished...\n");			
			break;
			
		case AUTODEBUG_P:
			set_pkcpu(pa_vip_k, pb_vip_k, pc_vip_k);
			printf_syn("update p_p_k dsp reg finished...\n");			
			break;
		case AUTODEBUG_PHASE: 
			set_pabc_xphcal(pa_vip_k, pb_vip_k, pc_vip_k);
			printf_syn("update xphcal dsp reg finished...\n");			
			break;
		default:
			break;

	}

	rt_free(em_info);
}
#endif
static void reset_xvigain_xphcal_reg(int chlx_num)
{
	int a, b, c;
#if 0 == EM_ALL_TYPE_BASE
	chlx_num = 0;
#endif
	struct gateway_em_st	 *em_info;

	em_info = rt_malloc(sizeof(*em_info));
	if (NULL == em_info) {
		printf_syn("%s(), alloc mem fail\n", __FUNCTION__);
		return ;
	}
	if (RT_EOK != read_syscfgdata_tbl(SYSCFGDATA_TBL_GW_EM_INFO, 0, em_info)) {
		printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
		rt_free(em_info);
		return;
	}  
	set_pabc_v_gain(em_info->chlx_st[0].pa_vgain, em_info->chlx_st[0].pb_vgain, em_info->chlx_st[0].pc_vgain);
	set_pabc_i_gain(em_info->chlx_st[chlx_num].pa_igain, em_info->chlx_st[chlx_num].pb_igain, em_info->chlx_st[chlx_num].pc_igain);
	set_pabc_xphcal(em_info->chlx_st[chlx_num].pa_phase, em_info->chlx_st[chlx_num].pb_phase, em_info->chlx_st[chlx_num].pc_phase);
	Write_8bitReg_ADE7880(0xE7FE, 0xAD);
	Write_8bitReg_ADE7880(0xE7E3, 0x00);
	get_pabc_v_gain(&a, &b, &c); 
	Write_32bitReg_ADE7880(AVGAIN_Register_Address, a);
	Write_32bitReg_ADE7880(BVGAIN_Register_Address, b);
	Write_32bitReg_ADE7880(CVGAIN_Register_Address, c);
	get_pabc_i_gain(&a, &b, &c);
	Write_32bitReg_ADE7880(AIGAIN_Register_Address, a);
	Write_32bitReg_ADE7880(BIGAIN_Register_Address, b);
	Write_32bitReg_ADE7880(CIGAIN_Register_Address, c);
	get_pabc_xphcal(&a, &b, &c);   
	Write_16bitReg_ADE7880(APHCAL_Register_Address, a);   
	Write_16bitReg_ADE7880(BPHCAL_Register_Address, b);
	Write_16bitReg_ADE7880(CPHCAL_Register_Address, c);
	Write_8bitReg_ADE7880(0xE7FE, 0xAD);
	Write_8bitReg_ADE7880(0xE7E3, 0x80);
	rt_thread_delay(get_ticks_of_ms(600));
   
	rt_free(em_info); 
}

void px_vircur_setout_autodebug(int v_setout,int i_setout,int p_setout,int chlx_cnt)
{  
#if EM_ALL_TYPE_BASE && EM_MULTI_BASE
	u8 cnt;
#endif           
	set_vkcpu(10000, 10000, 10000);
	set_ikcpu(10000, 10000, 10000);
 	set_pkcpu(10000, 10000, 10000);
  
	reset_origgain_output();
/* 自动调试网管呈现电压有效值精度 */
#if EM_ALL_TYPE_BASE && EM_MULTI_BASE
	for(cnt = 0; cnt < 4; cnt++){	
		set_vkcpu(10000, 10000, 10000);
		/* 多通道电流 */
		reset_origgain_output();
		reset_xvigain_xphcal_reg(0);
		/* 电流通道切换 */
		printf_syn("\n\ndebug v-setout chlx num(0~3): %d, \n", cnt);
		set_decoder_3to8_data(cnt);
		rt_thread_delay(get_ticks_of_ms(200));		
		/* 粗调 */
		get_average_value(AUTODEBUG_V);
		careless_debug_pxk(v_setout, AUTODEBUG_V);
	      
		/* 细调 */
		get_average_value(AUTODEBUG_V);
		careful_debug_pxk(v_setout, AUTODEBUG_V);  

		write_debug_into_flash(AUTODEBUG_V, cnt);
	}
  
	set_decoder_3to8_data(0);
	rt_thread_delay(get_ticks_of_ms(200));	
#else
	/* pt,ct单通道电流 */
	reset_xvigain_xphcal_reg(0);
	/* 粗调 */
	get_average_value(AUTODEBUG_V);
	careless_debug_pxk(v_setout, AUTODEBUG_V);
  
	/* 细调 */
	get_average_value(AUTODEBUG_V);
	careful_debug_pxk(v_setout, AUTODEBUG_V);  

	write_debug_into_flash(AUTODEBUG_V, 0);
#endif
/* 自动调试网管呈现电流有效值精度 */
#if EM_ALL_TYPE_BASE && EM_MULTI_BASE
	/* 多通道电流 */
	for(cnt = 0; cnt < chlx_cnt; cnt++){
		set_ikcpu(10000, 10000, 10000);		
		reset_origgain_output();
		reset_xvigain_xphcal_reg(cnt);
		/* 电流通道切换 */
		printf_syn("\n\ndebug i-setout chlx num(0~11): %d, \n", cnt);
		set_decoder_4to16_data(cnt);	
		rt_thread_delay(get_ticks_of_ms(200));		
		
		/* 粗调 */
		get_average_value(AUTODEBUG_I);
		careless_debug_pxk(i_setout, AUTODEBUG_I);
	 	  
		/* 细调 */
		get_average_value(AUTODEBUG_I);
		careful_debug_pxk(i_setout, AUTODEBUG_I); 

		write_debug_into_flash(AUTODEBUG_I, cnt);
	}
#else
	/* pt,ct单通道电流 */
	/* 粗调 */
	get_average_value(AUTODEBUG_I);
	careless_debug_pxk(i_setout, AUTODEBUG_I);
 	  
	/* 细调 */
	get_average_value(AUTODEBUG_I);
	careful_debug_pxk(i_setout, AUTODEBUG_I); 

	write_debug_into_flash(AUTODEBUG_I, 0);
#endif  
/* 自动调试网管呈现功率精度参数 */
#if EM_ALL_TYPE_BASE && EM_MULTI_BASE
	/* 多通道电流 */
	for(cnt = 0; cnt < chlx_cnt; cnt++){
	 	set_pkcpu(10000, 10000, 10000);		
		reset_origgain_output();
		reset_xvigain_xphcal_reg(cnt);
		/* 电流通道切换 */
		printf_syn("\n\ndebug p-setout chlx num(0~11): %d, \n", cnt);
		set_decoder_4to16_data(cnt);	
		rt_thread_delay(get_ticks_of_ms(600));		
		
		/* 粗调 */
		get_average_value(AUTODEBUG_P);
		careless_debug_pxk(p_setout, AUTODEBUG_P);

		/* 细调 */
		get_average_value(AUTODEBUG_P);
		careful_debug_pxk(p_setout, AUTODEBUG_P);   	  

		write_debug_into_flash(AUTODEBUG_P, cnt);
	}
#else
	/* 粗调 */
	get_average_value(AUTODEBUG_P);
	careless_debug_pxk(p_setout, AUTODEBUG_P);

	/* 细调 */
	get_average_value(AUTODEBUG_P);
	careful_debug_pxk(p_setout, AUTODEBUG_P);   	  

	write_debug_into_flash(AUTODEBUG_P, 0);
#endif
}

void px_voltage_setout_autodebug(int pa,int pb,int pc,int chlx_cnt)
{  

	set_vkcpu(10000, 10000, 10000);
	reset_origgain_output();

/* 自动调试网管呈现电压有效值精度 */
#if EM_ALL_TYPE_BASE && EM_MULTI_BASE
	/* 多通道电流 */

	reset_xvigain_xphcal_reg(0);
	/* 电流通道切换 */
	printf_syn("\n\ndebug v-setout chlx num(from 0): %d, \n", 0);
	set_decoder_3to8_data(0);
	rt_thread_delay(get_ticks_of_ms(200));
	/* 粗调 */
	get_average_value(AUTODEBUG_V);
	careless_debug_pabck(pa, pb, pc, AUTODEBUG_V);
  
	/* 细调 */
	get_average_value(AUTODEBUG_V);
	careful_debug_pabck(pa, pb, pc, AUTODEBUG_V);  

	write_debug_into_flash(AUTODEBUG_V, 0);
#else
	/* pt,ct单通道电流 */
	reset_xvigain_xphcal_reg(0);
	/* 粗调 */
	get_average_value(AUTODEBUG_V);
	careless_debug_pabck(pa, pb, pc, AUTODEBUG_V);
  
	/* 细调 */
	get_average_value(AUTODEBUG_V);
	careful_debug_pabck(pa, pb, pc, AUTODEBUG_V);  

	write_debug_into_flash(AUTODEBUG_V, 0);
#endif

}


void px_setout_voltage(void)
{
	s32 val;

	px_virtual_mode_voltage(PHASE_A, &val);
	printf_syn("virtual_mode_voltage:\nv-a: %d \n", val);

	px_virtual_mode_voltage(PHASE_B, &val);
	printf_syn("virtual_mode_voltage:\nv-b: %d \n", val);

	px_virtual_mode_voltage(PHASE_C, &val); 
	printf_syn("virtual_mode_voltage:\nv-c: %d \n", val);
}

  
void px_setout_handdebug(int id, int data, int chlx)
{
	s32 val;	


	set_7880_adj(id,data,chlx,0, 0, 0);


	switch(id){
		case 51:
			px_virtual_mode_voltage(PHASE_A, &val);
			printf_syn("virtual_mode_voltage:\nv-a: %d \n", val);
  
			break;

		case 52:
			px_virtual_mode_voltage(PHASE_B, &val);
			printf_syn("virtual_mode_voltage:\nv-b: %d \n", val);

			break;

		case 53:
			px_virtual_mode_voltage(PHASE_C, &val);	
			printf_syn("virtual_mode_voltage:\nv-c: %d \n", val);

			break;

		case 61:
			px_virtual_mode_current(PHASE_A, &val);
			printf_syn("virtual_mode_current:\ni-a: %d \n", val);
  
			break;

		case 62:
			px_virtual_mode_current(PHASE_B, &val);
			printf_syn("virtual_mode_current:\ni-b: %d \n", val);

			break;

		case 63:
			px_virtual_mode_current(PHASE_C, &val);	
			printf_syn("virtual_mode_current:\ni-c: %d \n", val);

			break;

		case 71:
			val = px_active_mode_power(PHASE_A);
			printf_syn("active power:\np-a: %d  \n", val);
			
			break;

		case 72:
			val = px_active_mode_power(PHASE_B);
			printf_syn("active power:\np-b: %d  \n", val);

			break;

		case 73:
			val = px_active_mode_power(PHASE_C);	
			printf_syn("active power:\np-c: %d  \n", val);

			break;
	
		default:
			printf_syn("cmd not in the table...\n");			

	}
}


void px_current_setout_autodebug(int pa,int pb,int pc,int chlx_cnt)
{  
#if EM_ALL_TYPE_BASE && EM_MULTI_BASE
	u8 cnt;
#endif
	set_ikcpu(10000, 10000, 10000);
	reset_origgain_output();
/* 自动调试网管呈现电流有效值精度 */
#if EM_ALL_TYPE_BASE && EM_MULTI_BASE
	/* 多通道电流 */
	for(cnt = 0; cnt < chlx_cnt; cnt++){
		set_ikcpu(10000, 10000, 10000);		
		reset_xvigain_xphcal_reg(cnt);
		/* 电流通道切换 */
		printf_syn("\n\ndebug i-setout chlx num(from 0): %d, \n", cnt);
		set_decoder_4to16_data(cnt);	
		rt_thread_delay(get_ticks_of_ms(200));
		/* 粗调 */
		get_average_value(AUTODEBUG_I);
		careless_debug_pabck(pa, pb, pc, AUTODEBUG_I);
		  
		/* 细调 */
		get_average_value(AUTODEBUG_I);
		careful_debug_pabck(pa, pb, pc, AUTODEBUG_I); 

		write_debug_into_flash(AUTODEBUG_I, cnt);
	}
#else
	/* pt,ct单通道电流 */
	/* 粗调 */
	get_average_value(AUTODEBUG_I);
	careless_debug_pabck(pa, pb, pc, AUTODEBUG_I);
	  
	/* 细调 */
	get_average_value(AUTODEBUG_I);
	careful_debug_pabck(pa, pb, pc, AUTODEBUG_I); 

	write_debug_into_flash(AUTODEBUG_I, 0);
#endif  
}


void px_active_power_setout_autodebug(int pa,int pb,int pc,int chlx_cnt)
{  
#if EM_ALL_TYPE_BASE && EM_MULTI_BASE
	u8 cnt;
#endif
  
 	set_pkcpu(10000, 10000, 10000);
	reset_origgain_output();

/* 自动调试网管呈现功率精度参数 */
#if EM_ALL_TYPE_BASE && EM_MULTI_BASE
	/* 多通道电流 */
	for(cnt = 0; cnt < chlx_cnt; cnt++){
	 	set_pkcpu(10000, 10000, 10000);		
		reset_xvigain_xphcal_reg(cnt);
		/* 电流通道切换 */
		printf_syn("\n\ndebug p-setout chlx num(from 0): %d, \n", cnt);
		set_decoder_4to16_data(cnt);	  
		rt_thread_delay(get_ticks_of_ms(600));
		/* 粗调 */
		get_average_value(AUTODEBUG_P);
		careless_debug_pabck(pa, pb, pc, AUTODEBUG_P);

		/* 细调 */
		get_average_value(AUTODEBUG_P);
		careful_debug_pabck(pa, pb, pc, AUTODEBUG_P);	  

		write_debug_into_flash(AUTODEBUG_P, cnt);
	}
#else
	/* 粗调 */
	get_average_value(AUTODEBUG_P);
	careless_debug_pabck(pa, pb, pc, AUTODEBUG_P);

	/* 细调 */
	get_average_value(AUTODEBUG_P);
	careful_debug_pabck(pa ,pb, pc, AUTODEBUG_P);	  

	write_debug_into_flash(AUTODEBUG_P, 0);
#endif
}


#define PhaseX_REG_DEBUG(data) 	(((1000*(data))/176))  
#define DEBUG_PHASE 1
static void clr_xphcal_reg(void)
{
	Write_16bitReg_ADE7880(APHCAL_Register_Address, 0);
	Write_16bitReg_ADE7880(BPHCAL_Register_Address, 0);
	Write_16bitReg_ADE7880(CPHCAL_Register_Address, 0);
	rt_thread_delay(get_ticks_of_ms(50));
}
static void get_orig_px_phase(void)
{
	autode_pxbf[0] = 0;
	autode_pxbf[1] = 0;
	autode_pxbf[2] = 0;
	autode_pxbf[0] = px_phase_mode_position(PHASE_A);
	autode_pxbf[1] = px_phase_mode_position(PHASE_B);
	autode_pxbf[2] = px_phase_mode_position(PHASE_C);
#if DEBUG_PHASE
	printf_syn("pa_phase_val: %d, pb_phase_val: %d, pc_phase_val: %d\n",
		autode_pxbf[0], autode_pxbf[1], autode_pxbf[2]);
#endif
}
static void calc_xphcal_val(void)
{
	pa_vip_k = 0;
	pb_vip_k = 0;
	pc_vip_k = 0;
	if(autode_pxbf[0] < 0){
		pa_vip_k = PhaseX_REG_DEBUG(autode_pxbf[0]*(-1));
	} else if(autode_pxbf[0] >= 0){
		pa_vip_k = PhaseX_REG_DEBUG(autode_pxbf[0]) + 512;		
	}
	if(autode_pxbf[1] < 0){
		pb_vip_k = PhaseX_REG_DEBUG(autode_pxbf[1]*(-1));
	} else if(autode_pxbf[1] >= 0){
		pb_vip_k = PhaseX_REG_DEBUG(autode_pxbf[1]) + 512;		
	}
	if(autode_pxbf[2] < 0){
		pc_vip_k = PhaseX_REG_DEBUG(autode_pxbf[2]*(-1));
	} else if(autode_pxbf[2] >= 0){
		pc_vip_k = PhaseX_REG_DEBUG(autode_pxbf[2]) + 512;		
	}
	Write_16bitReg_ADE7880(APHCAL_Register_Address, pa_vip_k);
	Write_16bitReg_ADE7880(BPHCAL_Register_Address, pb_vip_k);
	Write_16bitReg_ADE7880(CPHCAL_Register_Address, pc_vip_k);	
#if DEBUG_PHASE
	printf_syn("pa_vip_k: %d, pb_vip_k: %d, pc_vip_k: %d, \n",
		pa_vip_k, pb_vip_k, pc_vip_k);
#endif	
}
static void vectorgraph_value_setout(int id)
{
	switch(id){
		case 0:
			printf_syn("before debug vectorgraph_st:\n");			
			break;
		case 1:
			printf_syn("after debug vectorgraph_st:\n");			
			break;
		default:
			break;
	}
	printf_syn("vectorgraph_st:\nvvap(Uac) %d vvbp(Ubc) %d vvcp(Uab) %d \nviap %d vibp %d vicp %d \n",
		vg_st.vvap, vg_st.vvbp, vg_st.vvcp,
		vg_st.viap, vg_st.vibp, vg_st.vicp);
}
void reset_chlx_reg(int chlx_num)
{
	int a, b, c;
#if 0 == EM_ALL_TYPE_BASE
	chlx_num = 0;
#endif
	struct gateway_em_st	 *em_info;

	em_info = rt_malloc(sizeof(*em_info));
	if (NULL == em_info) {
		printf_syn("%s(), alloc mem fail\n", __FUNCTION__);
		return ;
	}
	if (RT_EOK != read_syscfgdata_tbl(SYSCFGDATA_TBL_GW_EM_INFO, 0, em_info)) {
		printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
		rt_free(em_info);
		return;
	}
    
	set_pabc_v_gain(em_info->chlx_st[0].pa_vgain, em_info->chlx_st[0].pb_vgain, em_info->chlx_st[0].pc_vgain);
	set_pabc_i_gain(em_info->chlx_st[chlx_num].pa_igain, em_info->chlx_st[chlx_num].pb_igain, em_info->chlx_st[chlx_num].pc_igain);
	set_pabc_xphcal(em_info->chlx_st[chlx_num].pa_phase, em_info->chlx_st[chlx_num].pb_phase, em_info->chlx_st[chlx_num].pc_phase);
	set_pabc_p_gain(em_info->chlx_st[chlx_num].pa_pgain, em_info->chlx_st[chlx_num].pb_pgain, em_info->chlx_st[chlx_num].pc_pgain);
	set_pabc_wattos(em_info->chlx_st[chlx_num].pa_wattos, em_info->chlx_st[chlx_num].pb_wattos, em_info->chlx_st[chlx_num].pc_wattos);
	set_pabc_vrmsos(em_info->chlx_st[chlx_num].pa_vrmsos, em_info->chlx_st[chlx_num].pb_vrmsos, em_info->chlx_st[chlx_num].pc_vrmsos);
	set_pabc_irmsos(em_info->chlx_st[chlx_num].pa_irmsos, em_info->chlx_st[chlx_num].pb_irmsos, em_info->chlx_st[chlx_num].pc_irmsos);
	set_vkcpu(em_info->chlx_st[0].pa_vk, em_info->chlx_st[0].pb_vk, em_info->chlx_st[0].pc_vk);
	set_ikcpu(em_info->chlx_st[chlx_num].pa_ik, em_info->chlx_st[chlx_num].pb_ik, em_info->chlx_st[chlx_num].pc_ik);
	set_pkcpu(em_info->chlx_st[chlx_num].pa_pk, em_info->chlx_st[chlx_num].pb_pk, em_info->chlx_st[chlx_num].pc_pk);
	set_p_wthr(em_info->chlx_st[chlx_num].pabc_wthr);
	set_p_varthr(em_info->chlx_st[chlx_num].pabc_varthr);

	/* 去能dsp写保护 */
	Write_8bitReg_ADE7880(0xE7FE, 0xAD);
	Write_8bitReg_ADE7880(0xE7E3, 0x00);
	rt_thread_delay(get_ticks_of_ms(100));
	get_pabc_v_gain(&a, &b, &c); 
	Write_32bitReg_ADE7880(AVGAIN_Register_Address, a);
	Write_32bitReg_ADE7880(BVGAIN_Register_Address, b);
	rt_thread_delay(get_ticks_of_ms(100));
	Write_32bitReg_ADE7880(CVGAIN_Register_Address, c);
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
	get_pabc_irmsos_gain(&a, &b, &c); 
	Write_32bitReg_ADE7880(AIRMSOS_Register_Address, a);
	rt_thread_delay(get_ticks_of_ms(100));
	Write_32bitReg_ADE7880(BIRMSOS_Register_Address, b);
	Write_32bitReg_ADE7880(CIRMSOS_Register_Address, c);	
	rt_thread_delay(get_ticks_of_ms(100));
	get_pabc_xphcal(&a, &b, &c);   
	Write_16bitReg_ADE7880(APHCAL_Register_Address, a);   
	Write_16bitReg_ADE7880(BPHCAL_Register_Address, b);
	rt_thread_delay(get_ticks_of_ms(100));
	Write_16bitReg_ADE7880(CPHCAL_Register_Address, c);
	get_p_varthr_gain(&a);
	Write_8bitReg_ADE7880(VARTHR_Register_Address, a);	
	rt_thread_delay(get_ticks_of_ms(100));
	get_p_wthr_gain(&a);
	Write_8bitReg_ADE7880(WTHR_Register_Address, a);	
	Write_8bitReg_ADE7880(WTHR_Register_Address, a);
	rt_thread_delay(get_ticks_of_ms(100));
	Write_8bitReg_ADE7880(WTHR_Register_Address, a);
	Write_8bitReg_ADE7880(0xE7FE, 0xAD);
	rt_thread_delay(get_ticks_of_ms(100));
	Write_8bitReg_ADE7880(0xE7E3, 0x80);
	rt_thread_delay(get_ticks_of_ms(600));

	rt_free(em_info);
}


#if EM_ALL_TYPE_BASE && EM_MULTI_BASE
void switch_current_channels(int chlx)
{	
	printf_syn("switch current channels num(0~11): %d\n",chlx);	
	set_decoder_4to16_data(chlx);
	auto_set_powerup_workmode(chlx); 
	reset_chlx_reg(chlx);  
	scan_chlx_wire_connect_mode(chlx);	
}
#endif


void printf_virtual_vi(int chlxv, int chlxi, int cnt_num)
{
	s32 pa, pb, pc, cnt;
 
	printf_syn("voltage chl num(from 0): %d  current chl num(from 0): %d \n", chlxv, chlxi); 

#if EM_ALL_TYPE_BASE && EM_MULTI_BASE
	set_decoder_3to8_data(chlxv);
	set_decoder_4to16_data(chlxi);
	rt_thread_delay(get_ticks_of_ms(200));
#endif 
	for(cnt=0; cnt < cnt_num; cnt++){
		px_virtual_mode_current(PHASE_A, &pa);
		px_virtual_mode_current(PHASE_B, &pb);
		px_virtual_mode_current(PHASE_C, &pc);	

		printf_syn("virtual_mode_current:\ni-a: %d i-b: %d i-c: %d \n",
		pa, pb, pc);
	}

	for(cnt=0; cnt < cnt_num; cnt++){
		px_virtual_mode_voltage(PHASE_A, &pa);
		px_virtual_mode_voltage(PHASE_B, &pb);
		px_virtual_mode_voltage(PHASE_C, &pc);	

		printf_syn("virtual_mode_voltage:\nv-a: %d v-b: %d v-c: %d \n",
		pa, pb, pc);
	}

	for(cnt=0; cnt < cnt_num; cnt++){
		pa = px_active_mode_power(PHASE_A);
		pb = px_active_mode_power(PHASE_B);
		pc = px_active_mode_power(PHASE_C);	

		printf_syn("active power:\np-a: %d p-b: %d p-c: %d \n",
		pa, pb, pc);
	}
}

void printf_phase_vi(int chlxv, int chlxi, int cnt_num)
{  
	int cnt;  

	//printf_syn("voltage chl num(from 0): %d  current chl num(from 0): %d \n", chlxv, chlxi); 

#if EM_ALL_TYPE_BASE && EM_MULTI_BASE
	set_decoder_3to8_data(chlxv);
	set_decoder_4to16_data(chlxi);
#endif 
	rt_thread_delay(get_ticks_of_ms(200)); 
	turn_vv_phase_sample();
	vg_st.vvap = px_phase_mode_position(PHASE_A);
	vg_st.vvbp = px_phase_mode_position(PHASE_B);
	vg_st.vvcp = px_phase_mode_position(PHASE_C);
#if 0
	printf_syn("vectorgraph_st:\nvvap(Uac) %d vvbp(Ubc) %d vvcp(Uab) %d \n",
		vg_st.vvap, vg_st.vvbp, vg_st.vvcp);
#endif  
	turn_vi_phase_sample();	  
	for(cnt=0; cnt < cnt_num; cnt++){	    
		vg_st.viap = px_phase_mode_position(PHASE_A);
		vg_st.vibp = px_phase_mode_position(PHASE_B);
		vg_st.vicp = px_phase_mode_position(PHASE_C);	
#if 0		
		printf_syn("viap %d vibp %d vicp %d num:%d\n",
			vg_st.viap, vg_st.vibp, vg_st.vicp, cnt);
#else
		printf_syn(" %d %d %d %d %d %d \n", vg_st.vvap, vg_st.vvbp, vg_st.vvcp,
			vg_st.viap, vg_st.vibp, vg_st.vicp);

#endif
		rt_thread_delay(get_ticks_of_ms(20));   
	}   
	turn_vv_phase_sample();	
}  

int printf_reg_info_debug(int chlx)
{

	struct gateway_em_st	 *em_info;

	em_info = rt_malloc(sizeof(*em_info));
	if (NULL == em_info) {
		printf_syn("%s(), alloc mem fail\n", __FUNCTION__);
		return FAIL;
	}


	if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_GW_EM_INFO, 0, em_info)) {
		printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
		rt_free(em_info);
		return FAIL;
	}

	printf_syn(" %d %d %d %d %d %d %d %d %d \n", em_info->chlx_st[chlx].pa_vk, em_info->chlx_st[chlx].pb_vk, em_info->chlx_st[chlx].pc_vk,
		   em_info->chlx_st[chlx].pa_ik, em_info->chlx_st[chlx].pb_ik, em_info->chlx_st[chlx].pc_ik,
		   em_info->chlx_st[chlx].pa_pk, em_info->chlx_st[chlx].pb_pk, em_info->chlx_st[chlx].pc_pk);

	rt_free(em_info);

	return SUCC;
}

void px_phase_autodebug(int chlx_cnt)
{  
	u8 cnt;   
	for(cnt=0; cnt < chlx_cnt; cnt++){	    
		printf_syn("\ncurrent channel num(0~11): %d\n",cnt);
#if EM_ALL_TYPE_BASE && EM_MULTI_BASE
		set_decoder_3to8_data(0);
		set_decoder_4to16_data(cnt);
#endif  
		rt_thread_delay(get_ticks_of_ms(200));
		clr_xphcal_reg();   
		vi_vector_graph_sampl();
		vectorgraph_value_setout(0);
		turn_vi_phase_sample();
		clr_xphcal_reg();
		get_orig_px_phase();
		calc_xphcal_val();
		write_debug_into_flash(AUTODEBUG_PHASE, cnt);	   
		turn_vv_phase_sample();	
		vi_vector_graph_sampl();	
		vectorgraph_value_setout(1);
	}
}
 
void px_phase_autodebug_chlx(int chlx)  
{  
	    
	printf_syn("\ncurrent channel num(0~11): %d\n",chlx);
#if EM_ALL_TYPE_BASE && EM_MULTI_BASE
	set_decoder_3to8_data(0);
	set_decoder_4to16_data(chlx);
#endif  
	rt_thread_delay(get_ticks_of_ms(200));
	clr_xphcal_reg();   
	vi_vector_graph_sampl();
	vectorgraph_value_setout(0);
	turn_vi_phase_sample();  
	clr_xphcal_reg();
	get_orig_px_phase();
	calc_xphcal_val();
	write_debug_into_flash(AUTODEBUG_PHASE, chlx);	   
	turn_vv_phase_sample();	
	vi_vector_graph_sampl();	
	vectorgraph_value_setout(1);
}

static void init_harmonic_mode(int chlx, int px_add, int num)
{
	/* 去能dsp写保护 */
	Write_8bitReg_ADE7880(0xE7FE, 0xAD);
	Write_8bitReg_ADE7880(0xE7E3, 0x00);

	Write_16bitReg_ADE7880(HCONFIG_Register_Address, chlx);
	Write_8bitReg_ADE7880(px_add, num);

	/* 使能dsp写保护 */
	Write_8bitReg_ADE7880(0xE7FE, 0xAD);
	Write_8bitReg_ADE7880(0xE7E3, 0x80);
}
#define DEBUG_HARMONIC 0
#if DEBUG_HARMONIC
static void printf_harmonic_st(void)
{
	printf_syn("harmonic.vrms:%d\nharmonic.irms:%d\nharmonic.watt:%d\nharmonic.var:%d\n"
		"harmonic.va:  %d\nharmonic.pf:  %d\nharmonic.vhd: %d\nharmonic.ihd:%d\n",
		harmonic.vrms, harmonic.irms, harmonic.watt, harmonic.var,
		harmonic.va, harmonic.pf, harmonic.vhd, harmonic.ihd);
}
#endif

static void update_harmonic_st(int px, int num)
{    
	switch (px) {
	case PHASE_A: 
		harmonic.vrms = (pa_vkcpu_data * ((int)Read_32bitReg_ADE7880(HXVRMS_Register_Address)/10))/1000;			
		harmonic.irms =	(pa_ikcpu_data * ((int)Read_32bitReg_ADE7880(HXIRMS_Register_Address)/10))/1000;
		harmonic.watt = (pa_pkcpu_data * ((int)Read_32bitReg_ADE7880(HXWATT_Register_Address)/10/4))/1000;	
		harmonic.var  = (pa_pkcpu_data * ((int)Read_32bitReg_ADE7880(HXVAR_Register_Address)/10/4))/1000;	
		harmonic.va   = (pa_pkcpu_data * ((int)Read_32bitReg_ADE7880(HXVA_Register_Address)/10/4))/1000;	
		harmonic.pf   =	((250 * (int)Read_32bitReg_ADE7880(HXPF_Register_Address))/128) * 40 / 65536;
		harmonic.vhd  =	((250 * (int)Read_32bitReg_ADE7880(HXVHD_Register_Address))/16) * 40 / 131071; 
		harmonic.ihd  = ((250 * (int)Read_32bitReg_ADE7880(HXIHD_Register_Address))/16) * 40 / 131071; 
#if DEBUG_HARMONIC  
		printf_syn("harmonic_parameter_st:A  num:%d\n", num);
		printf_harmonic_st();
#endif
		break;

	case PHASE_B:
		harmonic.vrms = (pb_vkcpu_data * ((int)Read_32bitReg_ADE7880(HYVRMS_Register_Address)/10))/1000;			
		harmonic.irms =	(pb_ikcpu_data * ((int)Read_32bitReg_ADE7880(HYIRMS_Register_Address)/10))/1000;
		harmonic.watt = (pb_pkcpu_data * ((int)Read_32bitReg_ADE7880(HYWATT_Register_Address)/10/4))/1000;	
		harmonic.var  = (pb_pkcpu_data * ((int)Read_32bitReg_ADE7880(HYVAR_Register_Address)/10/4))/1000;	
		harmonic.va   = (pb_pkcpu_data * ((int)Read_32bitReg_ADE7880(HYVA_Register_Address)/10/4))/1000;	
		harmonic.pf   =	((250 * (int)Read_32bitReg_ADE7880(HYPF_Register_Address))/128) * 40 / 65536;
		harmonic.vhd  =	((250 * (int)Read_32bitReg_ADE7880(HYVHD_Register_Address))/16) * 40 / 131071; 
		harmonic.ihd  = ((250 * (int)Read_32bitReg_ADE7880(HYIHD_Register_Address))/16) * 40 / 131071; 

		if(connect33_data == 1){
			harmonic.vrms = 0;			
			harmonic.irms =	0;
			harmonic.watt = 0;
			harmonic.var  = 0;
			harmonic.va   = 0;
			harmonic.pf   =	0;
			harmonic.vhd  =	0;
			harmonic.ihd  = 0;
		}
#if DEBUG_HARMONIC  
		printf_syn("harmonic_parameter_st:B  num:%d\n", num);
		printf_harmonic_st();
#endif
		
		break;
  
	case PHASE_C:
		harmonic.vrms = (pc_vkcpu_data * ((int)Read_32bitReg_ADE7880(HZVRMS_Register_Address)/10))/1000;			
		harmonic.irms =	(pc_ikcpu_data * ((int)Read_32bitReg_ADE7880(HZIRMS_Register_Address)/10))/1000;
		harmonic.watt = (pc_pkcpu_data * ((int)Read_32bitReg_ADE7880(HZWATT_Register_Address)/10/4))/1000;	
		harmonic.var  = (pc_pkcpu_data * ((int)Read_32bitReg_ADE7880(HZVAR_Register_Address)/10/4))/1000;	
		harmonic.va   = (pc_pkcpu_data * ((int)Read_32bitReg_ADE7880(HZVA_Register_Address)/10/4))/1000;	
		harmonic.pf  =	((250 * (int)Read_32bitReg_ADE7880(HZPF_Register_Address))/128) * 40 / 65536;
		harmonic.vhd  =	((250 * (int)Read_32bitReg_ADE7880(HZVHD_Register_Address))/16) * 40 / 131071; 
		harmonic.ihd  = ((250 * (int)Read_32bitReg_ADE7880(HZIHD_Register_Address))/16) * 40 / 131071; 
#if DEBUG_HARMONIC
		printf_syn("harmonic_parameter_st:C  num:%d\n", num);
		printf_harmonic_st();
#endif
		break;

	default:

		break;
	}
}

/* 
 * return value: harmonic谐波表格
 */
void px_harmonic_mode_parameter(int px, int num)
{ 
	
	switch (px) {
	case PHASE_A:
		init_harmonic_mode(HCONFIG_Register_AVIn, HX_Register_Address, num);

		break;
  
	case PHASE_B:
		init_harmonic_mode(HCONFIG_Register_BVIn, HY_Register_Address, num);
		
		break;

	case PHASE_C:
		init_harmonic_mode(HCONFIG_Register_CVIn, HZ_Register_Address, num);

		break;

	default:
		break;
	}

	/* 更新reg设置为600ms  */
	rt_thread_delay(get_ticks_of_ms(600));
	update_harmonic_st(px, num);
		
}
  
#if EM_ALL_TYPE_BASE 
/* 扫描通道接线方式 */
#define DEBUG_SCANWIRE 0
static int auto_scan_chlx_connect(int chlx)
{
	s32 ret, val;
	 
	struct electric_meter_reg_info_st *amm_sn;

	val = CONNECT_UNKNOWN;

	amm_sn = rt_malloc(sizeof(*amm_sn));
	if (NULL == amm_sn) {
		printf_syn("func:%s, out of memory\n", __FUNCTION__);
		return CONNECT_UNKNOWN;
	}
 
	if (SUCC == get_em_reg_info(amm_sn)) {
		if ('\0' != (amm_sn->em_sn[chlx][0])){
			px_virtual_mode_current(PHASE_B, &ret);
			if(ret < 1500){     
#if DEBUG_SCANWIRE		
	  			printf_syn("ret:%d  chlx num:%d auto detect 3-phase-3-wire system \n", ret, chlx);
#endif  
				val = CONNECT_33;
			}else if(ret >= 1500){  
#if DEBUG_SCANWIRE		
				printf_syn("ret:%d  chlx num:%d auto detect 3-phase-4-wire system \n", ret, chlx);
#endif
				val = CONNECT_34;
			}
		}else {
#if DEBUG_SCANWIRE	
			printf_syn("chlx num:%d not connect wire \n", chlx);
#endif
		}

	}

	rt_free(amm_sn);	
	return val;
}
    
void scan_wire_connect_mode(void)
{ 
	int chlx;
  
	printf_syn("scan wire connect start\n");
	for(chlx = 0; chlx < NUM_OF_COLLECT_EM_MAX; chlx++){
		set_decoder_4to16_data(chlx);  
		rt_thread_delay(get_ticks_of_ms(200));
		register_em_info.em_wire_con_mode[chlx] = auto_scan_chlx_connect(chlx);
	}  

#if DEBUG_SCANWIRE  
	for(chlx = 0; chlx < NUM_OF_COLLECT_EM_MAX; chlx++){
		printf_syn("chlx num:%d  wire mode:%d\n", chlx, register_em_info.em_wire_con_mode[chlx]);
	}  
#endif
	printf_syn("scan wire connect end\n");	
}

void scan_chlx_wire_connect_mode(int chlx)
{ 

#if DEBUG_SCANWIRE  
	printf_syn("chlx wire connect start:%d\n", chlx);
#endif

	register_em_info.em_wire_con_mode[chlx] = auto_scan_chlx_connect(chlx);

#if DEBUG_SCANWIRE  
	printf_syn("chlx num:%d  wire mode:%d\n", chlx, register_em_info.em_wire_con_mode[chlx]);
	printf_syn("scan wire connect end\n");	
	
#endif  
}

#endif

/*
 * return value: 单位0.1mV  电压有效值
 */
int px_virtual_mode_voltage(int px, s32 *data)
{
	
	int ret = SUCC;
	s32 reg_val, convert_reg_val;
#if ADE7880_USE_AVERAGE_V
	s32 cnt;
#endif
	reg_val  = 0;
	convert_reg_val = 0;

	clr_ade7880_statusx(STATUS1_Register_Address);

	switch (px) {
	case PHASE_A:
#if ADE7880_USE_SPI		
		if (SUCC != wait_int_signal() || SUCC != wait_zero_crossing_event( BIT(9))) {
			ret = FAIL;
			goto ret_entry;
		}
#endif
		clr_ade7880_statusx(STATUS0_Register_Address);
#if ADE7880_USE_SPI
		if (SUCC != wait_dsp_complete(BIT(17))) {
			ret = FAIL;
			goto ret_entry;
		}
#endif
#if ADE7880_USE_AVERAGE_V
		for(cnt = 0; cnt < 4; cnt++){
			reg_val += Read_32bitReg_ADE7880(AVRMS_Register_Address);
		}
		reg_val	= reg_val >> 2;
#else
		reg_val = Read_32bitReg_ADE7880(AVRMS_Register_Address);
#endif
#if val_out_auto_debug
		convert_reg_val = VAL_A_VOLTAGE_TRANS(reg_val);
#else
 		convert_reg_val = (pa_vkcpu_data * (reg_val/10))/1000;				
#endif		 
 		break;

	case PHASE_B:
#if ADE7880_USE_SPI		
		if (SUCC!=wait_int_signal() || SUCC!=wait_zero_crossing_event( BIT(10))) {
			ret = FAIL;
			goto ret_entry;
		}
#endif
		clr_ade7880_statusx(STATUS0_Register_Address);
#if ADE7880_USE_SPI
		if (SUCC!=wait_dsp_complete(BIT(17))) {
			ret = FAIL;
			goto ret_entry;
		}
#endif
#if ADE7880_USE_AVERAGE_V
		for(cnt = 0; cnt < 4; cnt++){
			reg_val += Read_32bitReg_ADE7880(BVRMS_Register_Address);
		}
		reg_val	= reg_val >> 2;
#else 
		reg_val  = Read_32bitReg_ADE7880(BVRMS_Register_Address);
#endif
#if val_out_auto_debug
		convert_reg_val = VAL_B_VOLTAGE_TRANS(reg_val);
#else
		convert_reg_val = (pb_vkcpu_data * (reg_val/10))/1000;	
#endif
		if(connect33_data == 1){
			convert_reg_val = 0;
		}
		break;
 
	case PHASE_C:
#if ADE7880_USE_SPI		
		if (SUCC!=wait_int_signal() || SUCC!=wait_zero_crossing_event( BIT(11))) {
			ret = FAIL;
			goto ret_entry;
		}
#endif
		clr_ade7880_statusx(STATUS0_Register_Address);
#if ADE7880_USE_SPI
		if (SUCC != wait_dsp_complete(BIT(17))) {
			ret = FAIL;
			goto ret_entry;
		}
#endif
#if ADE7880_USE_AVERAGE_V
		for(cnt = 0; cnt < 4; cnt++){
			reg_val += Read_32bitReg_ADE7880(CVRMS_Register_Address);
		}
		reg_val	= reg_val >> 2;
#else
		reg_val = Read_32bitReg_ADE7880(CVRMS_Register_Address);
#endif
#if val_out_auto_debug
		convert_reg_val = VAL_C_VOLTAGE_TRANS(reg_val);
#else
 		convert_reg_val = (pc_vkcpu_data * (reg_val/10))/1000;	
#endif
		break;

	default:
		ret = FAIL;
		goto ret_entry;
	}

	ade7880_debug(("fun:%s, line:%d, val:%d\n", __FUNCTION__, __LINE__, convert_reg_val));
//	printf_syn("fun:%s, line:%d, val:%d\n", __FUNCTION__, __LINE__, convert_reg_val);

	*data = convert_reg_val;
	ade7880_delay_after_opt();

ret_entry:
	return ret;
}


/*
 * return value: 单位0.01mA 电流有效值
 */
int px_virtual_mode_current(int px, s32 *data)
{
	s32 reg_val, convert_reg_val;
	int ret = SUCC;
#if ADE7880_USE_AVERAGE_I
	s32 cnt;
#endif
	reg_val  = 0;
	convert_reg_val = 0;
	clr_ade7880_statusx(STATUS1_Register_Address);

	switch (px) {
	case PHASE_A:
#if ADE7880_USE_SPI		
		if (SUCC!=wait_int_signal() || SUCC!=wait_zero_crossing_event( BIT(12))) {
			ret = FAIL;
			goto ret_entry;
		}
#endif
		clr_ade7880_statusx(STATUS0_Register_Address);
#if ADE7880_USE_SPI
		if (SUCC != wait_dsp_complete(BIT(17))) {
			ret = FAIL;
			goto ret_entry;
		}
#endif
#if ADE7880_USE_AVERAGE_I
		for(cnt = 0; cnt < 4; cnt++){
			reg_val += Read_32bitReg_ADE7880(AIRMS_Register_Address);
		}
		reg_val = reg_val >> 2;
#else
		reg_val = Read_32bitReg_ADE7880(AIRMS_Register_Address);
#endif
#if val_out_auto_debug
		convert_reg_val = VAL_A_CURRENT_TRANS(reg_val);
#else
 		convert_reg_val = (pa_ikcpu_data * (reg_val/10))/1000;	
#endif
		break;

	case PHASE_B:
#if ADE7880_USE_SPI		
		if (SUCC!=wait_int_signal() || SUCC!=wait_zero_crossing_event( BIT(13))) {
			ret = FAIL;
			goto ret_entry;
		}
#endif
		clr_ade7880_statusx(STATUS0_Register_Address);
#if ADE7880_USE_SPI
		if (SUCC != wait_dsp_complete(BIT(17))) {
			ret = FAIL;
			goto ret_entry;
		}
#endif					
#if ADE7880_USE_AVERAGE_I
		for(cnt = 0; cnt < 4; cnt++){
			reg_val += Read_32bitReg_ADE7880(BIRMS_Register_Address);
		}
		reg_val = reg_val >> 2;
#else
		reg_val = Read_32bitReg_ADE7880(BIRMS_Register_Address);
#endif

#if val_out_auto_debug
		convert_reg_val = VAL_B_CURRENT_TRANS(reg_val);
#else
		convert_reg_val = (pb_ikcpu_data * (reg_val/10))/1000;	
#endif
		if(connect33_data == 1){
			convert_reg_val = 0;
		}
		break;

	case PHASE_C:
#if ADE7880_USE_SPI		
		if (SUCC!=wait_int_signal() || SUCC!=wait_zero_crossing_event( BIT(14))) {
			ret = FAIL;
			goto ret_entry;
		}
#endif    
		clr_ade7880_statusx(STATUS0_Register_Address);
#if ADE7880_USE_SPI
		if (SUCC != wait_dsp_complete(BIT(17))) {
			ret = FAIL;
			goto ret_entry;
		}
#endif    
#if ADE7880_USE_AVERAGE_I
		for(cnt = 0; cnt < 4; cnt++){
			reg_val += Read_32bitReg_ADE7880(CIRMS_Register_Address);
		}
		reg_val = reg_val >> 2;
#else
		reg_val = Read_32bitReg_ADE7880(CIRMS_Register_Address);
#endif
#if val_out_auto_debug
		convert_reg_val = VAL_C_CURRENT_TRANS(reg_val);
#else
		convert_reg_val = (pc_ikcpu_data * (reg_val/10))/1000;			
#endif
		break;

	default:
		ret = FAIL;
		goto ret_entry;
	}

	ade7880_debug(("fun:%s, line:%d, val:%d\n", __FUNCTION__, __LINE__, convert_reg_val));
//	printf_syn("fun:%s, line:%d, val:%d\n", __FUNCTION__, __LINE__, convert_reg_val);

	*data = convert_reg_val;
	ade7880_delay_after_opt();

ret_entry:
	return ret;
}
#endif

/*
 * return value: 单位0.1Hz 频率
 */
unsigned int px_frequency_mode_signal(int px)
{ 
	u32 flag,flag_justice,frequency_signal,frequency_signal_in;

	frequency_signal = 0;
	frequency_signal_in = 0;
	switch (px) {
	case PHASE_A:
		flag_justice = Read_32bitReg_ADE7880(STATUS0_Register_Address); 	
		if(BIT(17) == (flag_justice&BIT(17))){      
			frequency_signal = Read_16bitReg_ADE7880(APERIOD_Register_Address);  
			flag = Read_32bitReg_ADE7880(STATUS0_Register_Address); 
		 	Write_32bitReg_ADE7880(STATUS0_Register_Address,SET(flag , 17)); 
		 	
	 	} else {
	 		printf_syn("fun:%s, line:%d\n", __FUNCTION__, __LINE__);
	 	}
	 	frequency_signal_in = VAL_A_Frequency_TRANS(frequency_signal);
		break;
  
	case PHASE_B:  
		flag_justice = Read_32bitReg_ADE7880(STATUS0_Register_Address); 	
		if(BIT(17) == (flag_justice&BIT(17))){      
			frequency_signal = Read_16bitReg_ADE7880(BPERIOD_Register_Address);  
			flag = Read_32bitReg_ADE7880(STATUS0_Register_Address); 
		 	Write_32bitReg_ADE7880(STATUS0_Register_Address,SET(flag , 17)); 
		 	
	 	} else {
	 		printf_syn("fun:%s, line:%d\n", __FUNCTION__, __LINE__);
	 	}
	 	frequency_signal_in = VAL_B_Frequency_TRANS(frequency_signal);
		if(connect33_data == 1){
			frequency_signal_in = 0;
		}
		break; 

	case PHASE_C:
		flag_justice = Read_32bitReg_ADE7880(STATUS0_Register_Address); 	
		if(BIT(17) == (flag_justice&BIT(17))){      
			frequency_signal = Read_16bitReg_ADE7880(CPERIOD_Register_Address);  
			flag = Read_32bitReg_ADE7880(STATUS0_Register_Address); 
		 	Write_32bitReg_ADE7880(STATUS0_Register_Address,SET(flag , 17)); 
		 	
	 	} else {
	 		printf_syn("fun:%s, line:%d\n", __FUNCTION__, __LINE__);
	 	}
	 	frequency_signal_in = VAL_C_Frequency_TRANS(frequency_signal);
		break;
		
	default:
		printf_syn("fun:%s, line:%d\n", __FUNCTION__, __LINE__);
		break;
	}

	ade7880_debug(("fun:%s, line:%d, val:%d\n", __FUNCTION__, __LINE__, frequency_signal_in));

	ade7880_delay_after_opt();

	return frequency_signal_in;  
}

/*
 * return value: 单位0.1度  相位
 */
 #if 0
unsigned int px_phase_mode_position(int px)
{ 
	u32 flag,flag_justice;
	u32 phase_position_signal = 0, phase_position_signal_in = 0;

	switch (px) {
	case PHASE_A:
		flag_justice = Read_32bitReg_ADE7880(STATUS0_Register_Address); 	
		if(BIT(17) == (flag_justice&BIT(17))){      
			phase_position_signal = Read_16bitReg_ADE7880(ANGLE0_Register_Address);  
			flag = Read_32bitReg_ADE7880(STATUS0_Register_Address); 
		 	Write_32bitReg_ADE7880(STATUS0_Register_Address,SET(flag , 17)); 
	 	} else {
	 		printf_syn("fun:%s, line:%d\n", __FUNCTION__, __LINE__);
	 	}

		phase_position_signal_in = VAL_A_Phase_Position_TRANS(phase_position_signal);
		if( phase_position_signal_in > 1800 )
		{
			phase_position_signal_in = 3600 - phase_position_signal_in; 
		}
		
		break;
  
	case PHASE_B:
		flag_justice = Read_32bitReg_ADE7880(STATUS0_Register_Address); 	
		if(BIT(17) == (flag_justice&BIT(17))){      
			phase_position_signal = Read_16bitReg_ADE7880(ANGLE1_Register_Address);  
			flag = Read_32bitReg_ADE7880(STATUS0_Register_Address); 
		 	Write_32bitReg_ADE7880(STATUS0_Register_Address,SET(flag , 17)); 
	 	} else {
	 		printf_syn("fun:%s, line:%d\n", __FUNCTION__, __LINE__);
	 	}

		phase_position_signal_in = VAL_B_Phase_Position_TRANS(phase_position_signal);
		if( phase_position_signal_in > 1800 )
		{
			phase_position_signal_in = 3600 - phase_position_signal_in; 
		}
		if(connect33_data == 1){
			phase_position_signal_in = 0;
		}
		break;   

	case PHASE_C: 
		flag_justice = Read_32bitReg_ADE7880(STATUS0_Register_Address); 	
		if(BIT(17) == (flag_justice&BIT(17))){      
			phase_position_signal = Read_16bitReg_ADE7880(ANGLE2_Register_Address);  
			flag = Read_32bitReg_ADE7880(STATUS0_Register_Address); 
		 	Write_32bitReg_ADE7880(STATUS0_Register_Address,SET(flag , 17)); 
	 	} else {
	 		printf_syn("fun:%s, line:%d\n", __FUNCTION__, __LINE__);
	 	}

		phase_position_signal_in = VAL_C_Phase_Position_TRANS(phase_position_signal);
		if( phase_position_signal_in > 1800 )
		{
			phase_position_signal_in = 3600 - phase_position_signal_in; 
		} 
		break;
		
	default:  
		break;
	}
  
 
	ade7880_delay_after_opt();

 	return phase_position_signal_in;


}
#else
/* test 33 */
unsigned int px_phase_mode_position(int px)
{ 
	u32 flag,flag_justice;
	int phase_position_signal = 0, phase_position_signal_in = 0;

	switch (px) {
	case PHASE_A:
		flag_justice = Read_32bitReg_ADE7880(STATUS0_Register_Address); 	
		if(BIT(17) == (flag_justice&BIT(17))){      
			phase_position_signal = Read_16bitReg_ADE7880(ANGLE0_Register_Address);  
			flag = Read_32bitReg_ADE7880(STATUS0_Register_Address); 
		 	Write_32bitReg_ADE7880(STATUS0_Register_Address,SET(flag , 17)); 
	 	} else {
	 		printf_syn("fun:%s, line:%d\n", __FUNCTION__, __LINE__);
	 	}

		phase_position_signal_in = VAL_A_Phase_Position_TRANS(phase_position_signal);

		phase_position_signal_in = phase_position_signal_in - 3600; 		
		
		break;
  
	case PHASE_B:
		flag_justice = Read_32bitReg_ADE7880(STATUS0_Register_Address); 	
		if(BIT(17) == (flag_justice&BIT(17))){      
			phase_position_signal = Read_16bitReg_ADE7880(ANGLE1_Register_Address);  
			flag = Read_32bitReg_ADE7880(STATUS0_Register_Address); 
		 	Write_32bitReg_ADE7880(STATUS0_Register_Address,SET(flag , 17)); 
	 	} else {
	 		printf_syn("fun:%s, line:%d\n", __FUNCTION__, __LINE__);
	 	}

		phase_position_signal_in = VAL_B_Phase_Position_TRANS(phase_position_signal);

		phase_position_signal_in = phase_position_signal_in - 3600; 
  
		if(connect33_data == 1){
			phase_position_signal_in = 0;
		}
		break;   

	case PHASE_C: 
		flag_justice = Read_32bitReg_ADE7880(STATUS0_Register_Address); 	
		if(BIT(17) == (flag_justice&BIT(17))){      
			phase_position_signal = Read_16bitReg_ADE7880(ANGLE2_Register_Address);  
			flag = Read_32bitReg_ADE7880(STATUS0_Register_Address); 
		 	Write_32bitReg_ADE7880(STATUS0_Register_Address,SET(flag , 17)); 
	 	} else {
	 		printf_syn("fun:%s, line:%d\n", __FUNCTION__, __LINE__);
	 	}

		phase_position_signal_in = VAL_C_Phase_Position_TRANS(phase_position_signal);

		phase_position_signal_in = phase_position_signal_in - 3600; 

		break;
		
	default:  
		break;
	}
  
 
	ade7880_delay_after_opt();

 	return phase_position_signal_in;


}

#endif
/*
 * return value: 单位0.01w, 有功功率
 */
int px_active_mode_power(int px)
{ 
	u32 flag,flag_justice;
	s32 active_power = 0, active_power_in = 0;

	switch (px) {
	case PHASE_A:
		flag_justice = Read_32bitReg_ADE7880(STATUS0_Register_Address); 	
		if(BIT(17) == (flag_justice&BIT(17))){      
			active_power = Read_32bitReg_ADE7880(AWATT_Register_Address);  
			flag = Read_32bitReg_ADE7880(STATUS0_Register_Address); 
		 	Write_32bitReg_ADE7880(STATUS0_Register_Address,SET(flag , 17)); 
	 	} else {
	 		printf_syn("fun:%s, line:%d\n", __FUNCTION__, __LINE__);
	 	} 
	 	//active_power_in = VAL_A_ACTIVE_POWER_TRANS(active_power);
#if 1==TEST_AUTO_POWER 	     	
 		active_power_in = (pa_pkcpu_data * active_power)/10000;	
#else
		active_power_in = (pa_pkcpu_data * (active_power/10))/1000;	
#endif    
		break;
   
	case PHASE_B:
		flag_justice = Read_32bitReg_ADE7880(STATUS0_Register_Address); 	
		if(BIT(17) == (flag_justice&BIT(17))){      
			active_power = Read_32bitReg_ADE7880(BWATT_Register_Address);  
			flag = Read_32bitReg_ADE7880(STATUS0_Register_Address); 
		 	Write_32bitReg_ADE7880(STATUS0_Register_Address,SET(flag , 17)); 
	 	} else {
	 		printf_syn("fun:%s, line:%d\n", __FUNCTION__, __LINE__);
	 	}
	 	//active_power_in = VAL_B_ACTIVE_POWER_TRANS(active_power);
#if 1==TEST_AUTO_POWER 
 		active_power_in = (pb_pkcpu_data * active_power)/10000;	
#else 
 		active_power_in = (pb_pkcpu_data * (active_power/10))/1000;	
#endif
		if(connect33_data == 1){
			active_power_in = 0;
		}
		break; 

	case PHASE_C:
		flag_justice = Read_32bitReg_ADE7880(STATUS0_Register_Address); 	
		if(BIT(17) == (flag_justice&BIT(17))){      
			active_power = Read_32bitReg_ADE7880(CWATT_Register_Address);  
			flag = Read_32bitReg_ADE7880(STATUS0_Register_Address); 
		 	Write_32bitReg_ADE7880(STATUS0_Register_Address,SET(flag , 17)); 
	 	} else {
	 		printf_syn("fun:%s, line:%d\n", __FUNCTION__, __LINE__);
	 	}
	 	//active_power_in = VAL_C_ACTIVE_POWER_TRANS(active_power);
#if 1==TEST_AUTO_POWER 
 		active_power_in = (pc_pkcpu_data * active_power)/10000;	
#else  
		active_power_in = (pc_pkcpu_data * (active_power/10))/1000;	
#endif
		break;
		
	default:
		break;
	}


	
	ade7880_delay_after_opt();

	return active_power_in;


}

/*
 * return value: 单位0.01var, 无功功率
 */
int px_reactive_mode_power(int px)
{
//	s32 data;
	s32 reactive_power = 0, reactive_power_in = 0;

	switch (px) {
	case PHASE_A:
		//data = 0x60;
		Write_16bitReg_ADE7880(HCONFIG_Register_Address, HCONFIG_Register_AVIn);
		break;
  
	case PHASE_B:
		//data = 0x61;
		Write_16bitReg_ADE7880(HCONFIG_Register_Address, HCONFIG_Register_BVIn);
		break;

	case PHASE_C:
		//data = 0x62;
		Write_16bitReg_ADE7880(HCONFIG_Register_Address, HCONFIG_Register_CVIn);

		break;

	default:
		//data = 0x6f;
		break;
	}
	/* 更新速录设置为512ms 此延时需大于512ms */
	ade7880_delay_after_opt();/* ??? */
	ade7880_delay_after_opt();/* ??? */

	switch (px) {
	case PHASE_A: 
		//data = 0x60;
		reactive_power = Read_32bitReg_ADE7880(FVAR_Register_Address);
		reactive_power_in = (pa_pkcpu_data * reactive_power)/10000;
		break;

	case PHASE_B:
		//data = 0x61;
		reactive_power = Read_32bitReg_ADE7880(FVAR_Register_Address);
		reactive_power_in = (pb_pkcpu_data * reactive_power)/10000;
		if(connect33_data == 1){
			reactive_power_in = 0;
		}
		break;

	case PHASE_C:
		//data = 0x62;
		reactive_power = Read_32bitReg_ADE7880(FVAR_Register_Address);
		reactive_power_in = (pc_pkcpu_data * reactive_power)/10000;
		break;

	default:
		//data = 0x6f;
		break;
	}
	
	return reactive_power_in;
}


/*
 * return value: 单位0.01va, 视在功率
 */
int px_apparent_mode_power(int px)
{ 
	u32 flag,flag_justice;
	s32 apparent_power = 0, apparent_power_in = 0;

	switch (px) {
	case PHASE_A:
		flag_justice = Read_32bitReg_ADE7880(STATUS0_Register_Address); 	
		if(BIT(17) == (flag_justice&BIT(17))){      
			apparent_power = Read_32bitReg_ADE7880(AVA_Register_Address);  
			flag = Read_32bitReg_ADE7880(STATUS0_Register_Address); 
		 	Write_32bitReg_ADE7880(STATUS0_Register_Address,SET(flag , 17)); 
	 	} else {
	 		printf_syn("fun:%s, line:%d\n", __FUNCTION__, __LINE__);
	 	}
#if val_out_auto_debug
		apparent_power_in = VAL_A_APPARENT_POWER_TRANS(apparent_power);
#else
#if 1==TEST_AUTO_POWER 	     	
 		apparent_power_in = (pa_pkcpu_data * apparent_power)/10000;	
#else
		apparent_power_in = (pa_pkcpu_data * (apparent_power/10))/1000;	
#endif

#endif
		break;

	case PHASE_B:
		flag_justice = Read_32bitReg_ADE7880(STATUS0_Register_Address); 	
		if(BIT(17) == (flag_justice&BIT(17))){      
			apparent_power = Read_32bitReg_ADE7880(BVA_Register_Address);  
			flag = Read_32bitReg_ADE7880(STATUS0_Register_Address); 
		 	Write_32bitReg_ADE7880(STATUS0_Register_Address,SET(flag , 17)); 
	 	} else {
	 		printf_syn("fun:%s, line:%d\n", __FUNCTION__, __LINE__);
	 	}
#if val_out_auto_debug
		apparent_power_in = VAL_B_APPARENT_POWER_TRANS(apparent_power);
#else

#if 1==TEST_AUTO_POWER 	     	
 		apparent_power_in = (pb_pkcpu_data * apparent_power)/10000;	
#else
		apparent_power_in = (pb_pkcpu_data * (apparent_power/10))/1000;	
#endif

#endif
		if(connect33_data == 1){
			apparent_power_in = 0;
		}
		break; 

	case PHASE_C:
		flag_justice = Read_32bitReg_ADE7880(STATUS0_Register_Address); 	
		if(BIT(17) == (flag_justice&BIT(17))){      
			apparent_power = Read_32bitReg_ADE7880(CVA_Register_Address);  
			flag = Read_32bitReg_ADE7880(STATUS0_Register_Address); 
		 	Write_32bitReg_ADE7880(STATUS0_Register_Address,SET(flag , 17)); 
	 	} else {
	 		printf_syn("fun:%s, line:%d\n", __FUNCTION__, __LINE__);
	 	}
#if val_out_auto_debug
		apparent_power_in = VAL_C_APPARENT_POWER_TRANS(apparent_power);
#else

#if 1==TEST_AUTO_POWER 	     	
 		apparent_power_in = (pc_pkcpu_data * apparent_power)/10000;	
#else
		apparent_power_in = (pc_pkcpu_data * (apparent_power/10))/1000;	
#endif
#endif
		break;
		 
	default:
		break;
	}

	//printf_syn("fun:%s, line:%d, px:%d, apparent:%d\n", __FUNCTION__, __LINE__, px, apparent_power_in);

	ade7880_delay_after_opt();

	return apparent_power_in;


}

/*
 * return value:  0.0001 功率因数
 */
int px_factor_mode_power(int px)
{ 
	u32 flag,flag_justice;
	int factor_power = 0, factor_power_in = 0;
 
	switch (px) {
	case PHASE_A:
		flag_justice = Read_32bitReg_ADE7880(STATUS0_Register_Address); 	
		if(BIT(17) == (flag_justice&BIT(17))){      
			factor_power = (s16)Read_16bitReg_ADE7880(APF_Register_Address);
			flag = Read_32bitReg_ADE7880(STATUS0_Register_Address); 
		 	Write_32bitReg_ADE7880(STATUS0_Register_Address,SET(flag , 17)); 
	 	} else {
	 		printf_syn("fun:%s, line:%d\n", __FUNCTION__, __LINE__);
	 	}
		factor_power_in = VAL_A_Factor_Power_TRANS(factor_power);
		break;

	case PHASE_B:
		flag_justice = Read_32bitReg_ADE7880(STATUS0_Register_Address); 	
		if(BIT(17) == (flag_justice&BIT(17))){      
			factor_power = (s16)Read_16bitReg_ADE7880(BPF_Register_Address);
			flag = Read_32bitReg_ADE7880(STATUS0_Register_Address); 
		 	Write_32bitReg_ADE7880(STATUS0_Register_Address,SET(flag , 17)); 
	 	} else {
	 		printf_syn("fun:%s, line:%d\n", __FUNCTION__, __LINE__);
	 	}
		factor_power_in = VAL_B_Factor_Power_TRANS(factor_power);
		if(connect33_data == 1){
			factor_power_in = 0;
		}
		break; 

	case PHASE_C:
		flag_justice = Read_32bitReg_ADE7880(STATUS0_Register_Address); 	
		if(BIT(17) == (flag_justice&BIT(17))){      
			factor_power = (s16)Read_16bitReg_ADE7880(CPF_Register_Address);
			flag = Read_32bitReg_ADE7880(STATUS0_Register_Address); 
		 	Write_32bitReg_ADE7880(STATUS0_Register_Address,SET(flag , 17)); 
	 	} else {
	 		printf_syn("fun:%s, line:%d\n", __FUNCTION__, __LINE__);
	 	}
		factor_power_in = VAL_C_Factor_Power_TRANS(factor_power);
		break;
		
	default:
		break;
	}

	//printf_syn("fun:%s, line:%d, px:%d, factor:%d, %d\n", __FUNCTION__, __LINE__, px, factor_power_in, factor_power);

	ade7880_delay_after_opt();

	return factor_power_in;


}


/*
 * return value: 1% 电压失真
 */
int px_voltage_distortion(int px)
{
	int value_v_dis, value_v_dis_in;

	switch (px) {
	case PHASE_A:
		Write_16bitReg_ADE7880(HCONFIG_Register_Address, HCONFIG_Register_Data_Choice_A);
		rt_thread_delay(get_ticks_of_ms(760)); /*750ms*/
		value_v_dis = Read_32bitReg_ADE7880(VTHD_Register_Address);
		break;

	case PHASE_B:
		Write_16bitReg_ADE7880(HCONFIG_Register_Address, HCONFIG_Register_Data_Choice_B);
		rt_thread_delay(get_ticks_of_ms(760)); /*750ms*/
		value_v_dis = Read_32bitReg_ADE7880(VTHD_Register_Address);
		if(connect33_data == 1){
			value_v_dis = 0;
		}
		break;

	case PHASE_C:
		Write_16bitReg_ADE7880(HCONFIG_Register_Address, HCONFIG_Register_Data_Choice_C);
		rt_thread_delay(get_ticks_of_ms(760)); /*750ms*/
		value_v_dis = Read_32bitReg_ADE7880(VTHD_Register_Address);
		break;

	default:
		value_v_dis = 0;
		break;
	}

	value_v_dis_in =  VAL_VOLTAGE_DISTORTION_TRANS (value_v_dis);

	return value_v_dis_in;
}
    
/*
 * return value: 1%, 电流失真
 */
int px_current_distortion(int px)
{
	unsigned int value_i_dis, value_i_dis_in;

	switch (px) {
	case PHASE_A:
		Write_16bitReg_ADE7880(HCONFIG_Register_Address, HCONFIG_Register_Data_Choice_A);
		rt_thread_delay(get_ticks_of_ms(760)); /*750ms*/
		value_i_dis = Read_32bitReg_ADE7880(ITHD_Register_Address);
	 	break;

	case PHASE_B:
		Write_16bitReg_ADE7880(HCONFIG_Register_Address, HCONFIG_Register_Data_Choice_B);
		rt_thread_delay(get_ticks_of_ms(760)); /*750ms*/
		value_i_dis = Read_32bitReg_ADE7880(ITHD_Register_Address);
		if(connect33_data == 1){
			value_i_dis = 0;
		}
		break;

	case PHASE_C:
		Write_16bitReg_ADE7880(HCONFIG_Register_Address, HCONFIG_Register_Data_Choice_C);
		rt_thread_delay(get_ticks_of_ms(760)); /*750ms*/
		value_i_dis = Read_32bitReg_ADE7880(ITHD_Register_Address);
		break;

	default:
		value_i_dis = 0;
		break;
	}

	value_i_dis_in = VAL_CURRENT_DISTORTION_TRANS(value_i_dis);

	return value_i_dis_in;
}
  
/*
 * hsdc方式下获取abc电压电流波形及无功功率
 * 	
 * 电压电流波形数据分别存储在以下缓存:
 * AI_HSCD_BUFFER[40];
 * AV_HSCD_BUFFER[40];
 * BI_HSCD_BUFFER[40];
 * BV_HSCD_BUFFER[40];
 * CI_HSCD_BUFFER[40];
 * CV_HSCD_BUFFER[40];  
 * 无功功率数据存储在以下缓存:
 * XFVAR_HSCD_BUFFER[3];
 * */  
void px_vi_sample_reac_p_hsdc(void)
{
	int cnt;

	struct hsdc_data_buffer_st	 *hsdc_buffer;

	hsdc_buffer = rt_malloc(sizeof(*hsdc_buffer));
	if (NULL == hsdc_buffer) {
		printf_syn("%s(), alloc hsdc mem fail\n", __FUNCTION__);
		rt_free(hsdc_buffer);

		return ;
	}      

	for(cnt=0; cnt < sizeof(*hsdc_buffer); cnt++){
		hsdc_buffer->hsdc_rec_buffer[cnt]= 0;	

	}
  
 
	ade7880_spi_withdma_hsdccfg();
	rt_thread_delay(get_ticks_of_ms(50));

	DMA1_Channel2->CCR   &= ~(1 << 0);   // 关闭DMA传输
	DMA1_Channel2->CMAR   = (u32)(&(hsdc_buffer->hsdc_rec_buffer[0]));
	DMA1_Channel2->CNDTR  = 32*80;  
	DMA1_Channel2->CCR   |= 1 << 0;      // 开启DMA传输
	rt_thread_delay(get_ticks_of_ms(10));

	Write_16bitReg_ADE7880(CONFIG_Register_Address, CONFIG_Register_Data1);	 	      
	rt_thread_delay(get_ticks_of_ms(50));


	for(cnt=0; cnt<40; cnt++){
		
		AI_HSCD_BUFFER[cnt] =0;
		AI_HSCD_BUFFER[cnt] =0; 	
		AV_HSCD_BUFFER[cnt] =0;
		AV_HSCD_BUFFER[cnt] =0;

		BI_HSCD_BUFFER[cnt] =0;
		BI_HSCD_BUFFER[cnt] =0;
		BV_HSCD_BUFFER[cnt] =0; 
		BV_HSCD_BUFFER[cnt] =0;

		CI_HSCD_BUFFER[cnt] =0;
		CI_HSCD_BUFFER[cnt] =0;
		CV_HSCD_BUFFER[cnt] =0;
		CV_HSCD_BUFFER[cnt] =0;
  
		
		AI_HSCD_BUFFER[cnt] |= hsdc_buffer->hsdc_rec_buffer[2*32*cnt + 0] << 16;
		AI_HSCD_BUFFER[cnt] |= hsdc_buffer->hsdc_rec_buffer[2*32*cnt + 1];		
		AV_HSCD_BUFFER[cnt] |= hsdc_buffer->hsdc_rec_buffer[2*32*cnt + 2] << 16;
		AV_HSCD_BUFFER[cnt] |= hsdc_buffer->hsdc_rec_buffer[2*32*cnt + 3];		

		if(connect33_data == 1){
			BI_HSCD_BUFFER[cnt] = 0;
			BI_HSCD_BUFFER[cnt] = 0;	
			BV_HSCD_BUFFER[cnt] = 0;
			BV_HSCD_BUFFER[cnt] = 0;	
		}else{
			BI_HSCD_BUFFER[cnt] |= hsdc_buffer->hsdc_rec_buffer[2*32*cnt + 4] << 16;
			BI_HSCD_BUFFER[cnt] |= hsdc_buffer->hsdc_rec_buffer[2*32*cnt + 5];		
			BV_HSCD_BUFFER[cnt] |= hsdc_buffer->hsdc_rec_buffer[2*32*cnt + 6] << 16;
			BV_HSCD_BUFFER[cnt] |= hsdc_buffer->hsdc_rec_buffer[2*32*cnt + 7];		
		}

		CI_HSCD_BUFFER[cnt] |= hsdc_buffer->hsdc_rec_buffer[2*32*cnt + 8] << 16;
		CI_HSCD_BUFFER[cnt] |= hsdc_buffer->hsdc_rec_buffer[2*32*cnt + 9];		
		CV_HSCD_BUFFER[cnt] |= hsdc_buffer->hsdc_rec_buffer[2*32*cnt + 10] << 16;
		CV_HSCD_BUFFER[cnt] |= hsdc_buffer->hsdc_rec_buffer[2*32*cnt + 11];
	
	}
      

	XFVAR_HSCD_BUFFER[0] = 0;
	XFVAR_HSCD_BUFFER[1] = 0;
	XFVAR_HSCD_BUFFER[2] = 0;

	XFVAR_HSCD_BUFFER[0] |= hsdc_buffer->hsdc_rec_buffer[26] << 16;
	XFVAR_HSCD_BUFFER[0] |= hsdc_buffer->hsdc_rec_buffer[27];

	XFVAR_HSCD_BUFFER[1] |= hsdc_buffer->hsdc_rec_buffer[28] << 16;
	XFVAR_HSCD_BUFFER[1] |= hsdc_buffer->hsdc_rec_buffer[29];		

	XFVAR_HSCD_BUFFER[2] |= hsdc_buffer->hsdc_rec_buffer[30] << 16;
	XFVAR_HSCD_BUFFER[2] |= hsdc_buffer->hsdc_rec_buffer[31];
    
	XFVAR_HSCD_BUFFER[0] = (pa_pkcpu_data * XFVAR_HSCD_BUFFER[0])/10000;	
	XFVAR_HSCD_BUFFER[1] = (pb_pkcpu_data * XFVAR_HSCD_BUFFER[1])/10000;
	XFVAR_HSCD_BUFFER[2] = (pc_pkcpu_data * XFVAR_HSCD_BUFFER[2])/10000;

#if 0  
	printf_syn("XFVAR_HSCD_BUFFER[0]:%d  XFVAR_HSCD_BUFFER[1]:%d XFVAR_HSCD_BUFFER[2]:%d\n",
		XFVAR_HSCD_BUFFER[0], XFVAR_HSCD_BUFFER[1], XFVAR_HSCD_BUFFER[2]);	
  

	printf_syn("XFVAR_HSCD_BUFFER[0]:%d  XFVAR_HSCD_BUFFER[1]:%d	XFVAR_HSCD_BUFFER[2]:%d\n",
		XFVAR_HSCD_BUFFER[0], XFVAR_HSCD_BUFFER[1], XFVAR_HSCD_BUFFER[2]);	

	printf_syn("AI_HSCD_BUFFER[0]:%d  AI_HSCD_BUFFER[1]:%d	AI_HSCD_BUFFER[2]:%d... ... AI_HSCD_BUFFER[39]:%d\n",
		AI_HSCD_BUFFER[0], AI_HSCD_BUFFER[1], AI_HSCD_BUFFER[2], AI_HSCD_BUFFER[39]);	

	printf_syn("BI_HSCD_BUFFER[0]:%d  BI_HSCD_BUFFER[1]:%d	BI_HSCD_BUFFER[2]:%d... ... BI_HSCD_BUFFER[39]:%d\n",
		BI_HSCD_BUFFER[0], BI_HSCD_BUFFER[1], BI_HSCD_BUFFER[2], BI_HSCD_BUFFER[39]);	

	printf_syn("CI_HSCD_BUFFER[0]:%d  CI_HSCD_BUFFER[1]:%d	CI_HSCD_BUFFER[2]:%d... ... CI_HSCD_BUFFER[39]:%d\n",
		CI_HSCD_BUFFER[0], CI_HSCD_BUFFER[1], CI_HSCD_BUFFER[2], CI_HSCD_BUFFER[39]);	
  
#endif  
  
	rt_free(hsdc_buffer);

}
#if 0 /* spi mode */
/*
 * 电压波形 通信方式:spi
 *
 * 每个采样点的数据是3字节的有符号数, 以网络序存储
 * */
int px_v_signal_sample(int px, signed char *buf, int len)
{
	enum ade7880_signal_sample_cmd_e temp_cmd;
	int cnt, temp;
	int *p;

	if (buf==NULL || len<SINK_INFO_PX_SAMPLE_BUF_SIZE) {
		printf_syn("func:%s, buf:%x, len:%d\n", __FUNCTION__, buf, len);
		return FAIL;
	}
  
	//px_v_signal_sample_(px);

	cnt  = 0;
	do {
		rt_thread_delay(get_ticks_of_ms(30));
		temp_cmd = get_ade7880_signal_sample_cmd();

		if (++cnt > 3) {
			printf_syn("func:%s, wait too long time\n", __FUNCTION__);
			goto err_entry;
		}
	} while (ASSC_SAMPLE_NONE != temp_cmd);

	switch (px) {
	case PHASE_A:
		p = pa_ade7880_sample_data[0];
		break;

	case PHASE_B:
		p = pb_ade7880_sample_data[0];
		break;

	case PHASE_C:
		p = pc_ade7880_sample_data[0];
		break;

	default:
		goto err_entry;
		break;
	}

	for (cnt=0; cnt<SINK_INFO_PX_SAMPLE_DOT_NUM; ++cnt) {
		temp   = *p++;
		*buf++ = temp >> 16;
		*buf++ = temp >> 8;
		*buf++ = temp;
	}

	return SUCC;

err_entry:
	printf_syn("func:%s, error\n", __FUNCTION__);
	return FAIL;
}


/*
 * 电压电流波形 通信方式:spi
 *
 * 每个采样点的数据是3字节的有符号数, 以网络序存储
 * vs_buf -- 存储电压采样数据
 * is_buf -- 存储电流采样数据
 * */
int px_vi_signal_sample(int px, signed char *vs_buf, signed char *is_buf, int len)
{
	enum ade7880_signal_sample_cmd_e temp_cmd;
	int cnt, tempv, tempi;
	int *pv, *pi;

	if (vs_buf==NULL || is_buf==NULL || len<SINK_INFO_PX_SAMPLE_BUF_SIZE) {
		printf_syn("func:%s, vs_buf:0x%x, is_buf=0x%x, len:%d\n", __FUNCTION__, vs_buf, is_buf, len);
		return FAIL;
	}
#if ADE7880_USE_I2C_HSDC
	px_vi_signal_sample_(px);

	cnt  = 0;
	do {
		rt_thread_delay(get_ticks_of_ms(30));
		temp_cmd = get_ade7880_signal_sample_cmd();

		if (++cnt > 3) {
			printf_syn("func:%s, wait too long time\n", __FUNCTION__);
			goto err_entry;
		}
	} while (ASSC_SAMPLE_NONE != temp_cmd);

	switch (px) {
	case PHASE_A:
		pv = pa_ade7880_sample_data[0];
		pi = pa_ade7880_sample_data[1];
		break;

	case PHASE_B:
		pv = pb_ade7880_sample_data[0];
		pi = pb_ade7880_sample_data[1];
		break;

	case PHASE_C:
		pv = pc_ade7880_sample_data[0];
		pi = pc_ade7880_sample_data[1];
		break;

	default:
		goto err_entry;
		break;
	}
#else    
	switch (px) {
	case PHASE_A:
		pv = AV_HSCD_BUFFER[0];
		pi = AI_HSCD_BUFFER[0];
		break;

	case PHASE_B:
		pv = BV_HSCD_BUFFER[0];
		pi = BI_HSCD_BUFFER[0];
		break;

	case PHASE_C:
		pv = CV_HSCD_BUFFER[0];
		pi = BI_HSCD_BUFFER[0];
		break;

	default:
		goto err_entry;
		break;
	}
#endif
	for (cnt=0; cnt<SINK_INFO_PX_SAMPLE_DOT_NUM; ++cnt) {
		tempv   = *pv++;
		tempi	= *pi++;

		*vs_buf++ = tempv >> 16;
		*vs_buf++ = tempv >> 8;
		*vs_buf++ = tempv;
		*is_buf++ = tempi >> 16;
		*is_buf++ = tempi >> 8;
		*is_buf++ = tempi;
	}

	return SUCC;

err_entry:
	printf_syn("func:%s, error\n", __FUNCTION__);
	return FAIL;
}


/*
 * 通信方式:spi
 *
 */
void px_vi_signal_sample_(int px)
{
	switch (px) {
	case PHASE_A:
		set_ade7880_signal_sample_cmd(ASSC_SAMPLE_AVI);
		TIM4->CR1 |= CR1_CEN_Set; 		/* Enable the TIM Counter */
		break;

	case PHASE_B:
		set_ade7880_signal_sample_cmd(ASSC_SAMPLE_BVI);
		TIM4->CR1 |= CR1_CEN_Set;		/* Enable the TIM Counter */
		break;

	case PHASE_C:
		set_ade7880_signal_sample_cmd(ASSC_SAMPLE_CVI);
		TIM4->CR1 |= CR1_CEN_Set;		/* Enable the TIM Counter */
		break;

	default:
		printf_syn("func:%s,param(%d) error\n", __FUNCTION__, px);
		break;
	}

	return;


}

static void set_ade7880_signal_sample_cmd(enum ade7880_signal_sample_cmd_e cmd)
{
	ade7880_sample_cmd = cmd;
	return;
}

static enum ade7880_signal_sample_cmd_e get_ade7880_signal_sample_cmd(void)
{
	return ade7880_sample_cmd;
}



/*
 *  return value: 单位wh, 总有功电能累计
 */
int px_active_energy(int px, s32_t *data)
{
	int active_energy, active_energy_in;
	int ret = SUCC;

	switch (px) {
	case PHASE_A:
		clr_ade7880_statusx(STATUS0_Register_Address);
		if (SUCC != wait_dsp_complete(BIT(17))) {
			ret = FAIL;
			goto ret_entry;
		}
		active_energy = Read_32bitReg_ADE7880(AWATTHR_Register_Address);
		active_energy_in = VAL_A_ACTIVE_ENERGY_TRANS(active_energy);
	 	break;

	case PHASE_B:
		clr_ade7880_statusx(STATUS0_Register_Address);
		if (SUCC != wait_dsp_complete(BIT(17))) {
			ret = FAIL;
			goto ret_entry;
		}
		active_energy = Read_32bitReg_ADE7880(BWATTHR_Register_Address);
		active_energy_in = VAL_B_ACTIVE_ENERGY_TRANS(active_energy);
		break;

	case PHASE_C:
		clr_ade7880_statusx(STATUS0_Register_Address);
		if (SUCC != wait_dsp_complete(BIT(17))) {
			ret = FAIL;
			goto ret_entry;
		}
		active_energy = Read_32bitReg_ADE7880(CWATTHR_Register_Address);
		active_energy_in = VAL_C_ACTIVE_ENERGY_TRANS(active_energy);
		break;

	default:
		ret = FAIL;
		goto ret_entry;
	}

	*data = active_energy_in;
ret_entry:
	return ret;
}

/*
 *  return value: 单位 varh, 总基波无功电能
 *		 fundamental Wattless power
 */
int px_reactive_energy(int px, s32_t *data)
{
	int reactive_energy, reactive_energy_in;
	int ret;

	switch (px) {
	case PHASE_A:
		clr_ade7880_statusx(STATUS0_Register_Address);
		if (SUCC != wait_dsp_complete(BIT(17))) {
			ret = FAIL;
			goto ret_entry;
		}
		reactive_energy = Read_32bitReg_ADE7880(AFVARHR_Register_Address);
		reactive_energy_in = VAL_A_REACTIVE_ENERGY_TRANS(reactive_energy);
	 	break;

	case PHASE_B:
		clr_ade7880_statusx(STATUS0_Register_Address);
		if (SUCC != wait_dsp_complete(BIT(17))) {
			ret = FAIL;
			goto ret_entry;
		}
		reactive_energy = Read_32bitReg_ADE7880(BFVARHR_Register_Address);
		reactive_energy_in = VAL_B_REACTIVE_ENERGY_TRANS(reactive_energy);
		break;

	case PHASE_C:
		clr_ade7880_statusx(STATUS0_Register_Address);
		if (SUCC != wait_dsp_complete(BIT(17))) {
			ret = FAIL;
			goto ret_entry;
		}
		reactive_energy = Read_32bitReg_ADE7880(CFVARHR_Register_Address);
		reactive_energy_in = VAL_C_REACTIVE_ENERGY_TRANS(reactive_energy);
		break;

	default:
		ret = FAIL;
		goto ret_entry;
	}

	*data = reactive_energy_in;
ret_entry:
	return ret;
}
#endif

#else /* ade7880 stub */

/* ade7880 stub */
#define PHASE_A 0X01
#define PHASE_B 0X02
#define PHASE_C 0X03

/* test-cmd:2 */
unsigned int px_virtual_mode_voltage(int px)
{
	unsigned int data;

	switch (px) {
	case PHASE_A:
		data = 0x10;
		break;

	case PHASE_B:
		data = 0x11;
		break;

	case PHASE_C:
		data = 0x12;
		break;

	default:
		data = 0x1f;
		break;
	}

	return data;
}

/* test-cmd:3 */
unsigned int px_virtual_mode_current(int px)
{
	unsigned int data;

	switch (px) {
	case PHASE_A:
		data = 0x20;
		break;

	case PHASE_B:
		data = 0x21;
		break;

	case PHASE_C:
		data = 0x22;
		break;

	default:
		data = 0x2f;
		break;
	}

	return data;
}

/* test-cmd:4 */
unsigned int px_frequency_mode_signal(int px)
{
	unsigned int data;

	switch (px) {
	case PHASE_A:
		data = 0x30;
		break;

	case PHASE_B:
		data = 0x31;
		break;

	case PHASE_C:
		data = 0x32;
		break;

	default:
		data = 0x3f;
		break;
	}

	return data;
}

/* test-cmd:5 */
unsigned int px_phase_mode_position(int px)
{
	unsigned int data;

	switch (px) {
	case PHASE_A:
		data = 0x40;
		break;

	case PHASE_B:
		data = 0x41;
		break;

	case PHASE_C:
		data = 0x42;
		break;

	default:
		data = 0x4f;
		break;
	}

	return data;
}

/* test-cmd:6 */
int px_active_mode_power(int px)
{
	unsigned int data;

	switch (px) {
	case PHASE_A:
		data = 0x50;
		break;

	case PHASE_B:
		data = 0x51;
		break;

	case PHASE_C:
		data = 0x52;
		break;

	default:
		data = 0x5f;
		break;
	}

	return data;
}


/* test-cmd:7 */
unsigned int px_reactive_mode_power(int px)
{
	unsigned int data;

	switch (px) {
	case PHASE_A:
		data = 0x60;
		break;

	case PHASE_B:
		data = 0x61;
		break;

	case PHASE_C:
		data = 0x62;
		break;

	default:
		data = 0x6f;
		break;
	}

	return data;
}

/* test-cmd:8 */
unsigned int px_apparent_mode_power(int px)
{
	unsigned int data;

	switch (px) {
	case PHASE_A:
		data = 0x70;
		break;

	case PHASE_B:
		data = 0x71;
		break;

	case PHASE_C:
		data = 0x72;
		break;

	default:
		data = 0x7f;
		break;
	}

	return data;
}

/* test-cmd:9 */
unsigned int px_factor_mode_power(int px)
{
	unsigned int data;

	switch (px) {
	case PHASE_A:
		data = 0x80;
		break;

	case PHASE_B:
		data = 0x81;
		break;

	case PHASE_C:
		data = 0x82;
		break;

	default:
		data = 0x8f;
		break;
	}

	return data;
}

/* test-cmd:10 */
unsigned int px_voltage_distortion(int px)
{
	unsigned int data;

	switch (px) {
	case PHASE_A:
		data = 0x90;
		break;

	case PHASE_B:
		data = 0x91;
		break;

	case PHASE_C:
		data = 0x92;
		break;

	default:
		data = 0x9f;
		break;
	}

	return data;
}

/* test-cmd:11 */
unsigned int px_current_distortion(int px)
{
	unsigned int data;

	switch (px) {
	case PHASE_A:
		data = 0xa0;
		break;

	case PHASE_B:
		data = 0xa1;
		break;

	case PHASE_C:
		data = 0xa2;
		break;

	default:
		data = 0xaf;
		break;
	}

	return data;
}

/* test-cmd:12 */
int px_v_signal_sample(int px, unsigned char *buf, int len)
{
	if (buf==NULL || len<SINK_INFO_PX_SAMPLE_BUF_SIZE) {
		printf_syn("func:%s, buf:%x, len:%d\n", __FUNCTION__, buf, len);
		return -1;
	}

	switch (px) {
	case PHASE_A:
		*buf++ = 0xb0;
		*buf++ = 0xb1;
		break;

	case PHASE_B:
		*buf++ = 0xb2;
		*buf++ = 0xb3;
		break;

	case PHASE_C:
		*buf++ = 0xb4;
		*buf++ = 0xb5;
		break;

	default:
		*buf++ = 0xbf;
		*buf++ = 0xbf;
		break;
	}

	return 0;
}

/* test-cmd:13 */
int px_i_signal_sample(int px, unsigned char *buf, int len)
{
	if (buf==NULL || len<SINK_INFO_PX_SAMPLE_BUF_SIZE) {
		printf_syn("func:%s, buf:%x, len:%d\n", __FUNCTION__, buf, len);
		return -1;
	}

	switch (px) {
	case PHASE_A:
		*buf++ = 0xc0;
		*buf++ = 0xc1;
		break;

	case PHASE_B:
		*buf++ = 0xc2;
		*buf++ = 0xc3;
		break;

	case PHASE_C:
		*buf++ = 0xc4;
		*buf++ = 0xc5;
		break;

	default:
		*buf++ = 0xcf;
		*buf++ = 0xcf;
		break;
	}

	return 0;
}

#endif


void set_pabc_v_gain(int a, int b, int c)
{
	avgain_reg_data = a;
	bvgain_reg_data = b;
	cvgain_reg_data = c;

	return;
}

void set_pabc_i_gain(int a, int b, int c)
{
	aigain_reg_data = a;
	bigain_reg_data = b;
	cigain_reg_data = c;

	return;
}

void set_pabc_p_gain(int a, int b, int c)
{
	apgain_reg_data = a;
	bpgain_reg_data = b;
	cpgain_reg_data = c;

	return;
}

void set_pabc_wattos(int a, int b, int c)
{
	awattos_reg_data = a;
	bwattos_reg_data = b;
	cwattos_reg_data = c;

	return;
}

void set_pabc_vrmsos(int a, int b, int c)
{
	avrmsos_reg_data = a;
	bvrmsos_reg_data = b;
	cvrmsos_reg_data = c;

	return;
}

void set_pabc_irmsos(int a, int b, int c)
{
	airmsos_reg_data = a;
	birmsos_reg_data = b;
	cirmsos_reg_data = c;

	return;
}

void set_cfxden(int a, int b, int c)
{
	cf1den_reg_data = a;
	cf2den_reg_data = b;
	cf3den_reg_data = c;

	return;
}

void set_pabc_xphcal(int a, int b, int c)
{
	aphcal_reg_data = a;
	bphcal_reg_data = b;
	cphcal_reg_data = c;
	return;
}
void set_vkcpu(int a, int b, int c)
{
	pa_vkcpu_data = a;
	pb_vkcpu_data = b;
	pc_vkcpu_data = c;

	return;
}

void set_ikcpu(int a, int b, int c)
{
	pa_ikcpu_data = a;
	pb_ikcpu_data = b;
	pc_ikcpu_data = c;

	return;
}

void set_pkcpu(int a, int b, int c)
{
	pa_pkcpu_data = a;
	pb_pkcpu_data = b;
	pc_pkcpu_data = c;

	return;
}

void set_connect(int a)
{
	connect33_data = a;

	if(connect33_data == 1){
		printf_syn("connect mode 3-phase-3-wire system\n");
	}else if(connect33_data == 0){
		printf_syn("connect mode 3-phase-4-wire system\n");
	}else {
		printf_syn("not find the value,flash write something wrong...  ");
	}

	return;
}

void set_mc(int a)
{
	connect33_data = a;

	if(connect33_data == 1){
		printf_syn("connect mode 3-phase-3-wire system ");
	}else if(connect33_data == 0){
		printf_syn("connect mode 3-phase-4-wire system ");
	}else {
		printf_syn("not find the value,flash write something wrong...  ");
	}

	return;
}

void set_p_wthr(u8_t a)
{
	wthr_reg_data = a;

	return;
}

void set_p_varthr(u8_t a)
{
	varthr_reg_data = a;

	return;
}

void get_pabc_v_gain(int *a, int *b, int *c)
{
	 *a = avgain_reg_data;
	 *b = bvgain_reg_data;
	 *c = cvgain_reg_data;

	return;
}

void get_pabc_i_gain(int *a, int *b, int *c)
{
	 *a = aigain_reg_data;
	 *b = bigain_reg_data;
	 *c = cigain_reg_data;

	 return;
}

void get_pabc_p_gain(int *a, int *b, int *c)
{
	 *a = apgain_reg_data;
	 *b = bpgain_reg_data;
	 *c = cpgain_reg_data;

	 return;
}

void get_pabc_wattos_gain(int *a, int *b, int *c)
{
	 *a = awattos_reg_data;
	 *b = bwattos_reg_data;
	 *c = cwattos_reg_data;

	 return;
}

void get_pabc_vrmsos_gain(int *a, int *b, int *c)
{
	 *a = avrmsos_reg_data;
	 *b = bvrmsos_reg_data;
	 *c = cvrmsos_reg_data;

	 return;
}
 
void get_pabc_irmsos_gain(int *a, int *b, int *c)
{
	 *a = airmsos_reg_data;
	 *b = birmsos_reg_data;
	 *c = cirmsos_reg_data;

	 return;
}

void get_cfxden_gain(int *a, int *b, int *c)
{
	 *a = cf1den_reg_data;
	 *b = cf2den_reg_data;
	 *c = cf3den_reg_data;

	 return;
}
void get_pabc_xphcal(int *a, int *b, int *c)
{
	 *a = aphcal_reg_data;
	 *b = bphcal_reg_data;
	 *c = cphcal_reg_data;
	 return;
}
void get_p_wthr_gain(int *a)
{
	 *a = wthr_reg_data;

	 return;
}

void get_p_varthr_gain(int *a)
{
	 *a = varthr_reg_data;

	 return;
}

void set_7880adj_reg(enum ade7880_adjust_reg_grp_id_e id)
{
	u32_t t1, t2, t3;
	u16_t t5, t6, t7;

	if (AARI_READ_PXX_REG == id) {
		t1 = Read_32bitReg_ADE7880(AVGAIN_Register_Address);
		t2 = Read_32bitReg_ADE7880(BVGAIN_Register_Address);
		t3 = Read_32bitReg_ADE7880(CVGAIN_Register_Address);
		printf_syn("[vgain]reg data:0x%x, 0x%x, 0x%x\n", t1, t2, t3);

		t1 = Read_32bitReg_ADE7880(AIGAIN_Register_Address);
		t2 = Read_32bitReg_ADE7880(BIGAIN_Register_Address);
		t3 = Read_32bitReg_ADE7880(CIGAIN_Register_Address);
		printf_syn("[igain]reg data:0x%x, 0x%x, 0x%x\n", t1, t2, t3);

		t1 = Read_32bitReg_ADE7880(APGAIN_Register_Address);
		t2 = Read_32bitReg_ADE7880(BPGAIN_Register_Address);
		t3 = Read_32bitReg_ADE7880(CPGAIN_Register_Address);
		printf_syn("[pgain]reg data:0x%x, 0x%x, 0x%x\n", t1, t2, t3);

		t1 = Read_32bitReg_ADE7880(AWATTOS_Register_Address);
		t2 = Read_32bitReg_ADE7880(BWATTOS_Register_Address);
		t3 = Read_32bitReg_ADE7880(CWATTOS_Register_Address);
		printf_syn("[wattos]reg data:0x%x, 0x%x, 0x%x\n", t1, t2, t3);

		t1 = Read_32bitReg_ADE7880(AVRMSOS_Register_Address);
		t2 = Read_32bitReg_ADE7880(BVRMSOS_Register_Address);
		t3 = Read_32bitReg_ADE7880(CVRMSOS_Register_Address);
		printf_syn("[vrmsos]reg data:0x%x, 0x%x, 0x%x\n", t1, t2, t3);

		t1 = Read_32bitReg_ADE7880(AIRMSOS_Register_Address);
		t2 = Read_32bitReg_ADE7880(BIRMSOS_Register_Address);
		t3 = Read_32bitReg_ADE7880(CIRMSOS_Register_Address);
		printf_syn("[irmsos]reg data:0x%x, 0x%x, 0x%x\n", t1, t2, t3);
		t1 = Read_16bitReg_ADE7880(APHCAL_Register_Address);
		t2 = Read_16bitReg_ADE7880(BPHCAL_Register_Address);
		t3 = Read_16bitReg_ADE7880(CPHCAL_Register_Address);
		printf_syn("[xphcal]reg data:0x%x, 0x%x, 0x%x\n", t1, t2, t3);

		t5 = Read_16bitReg_ADE7880(CF1DEN_Register_Address);
		t6 = Read_16bitReg_ADE7880(CF2DEN_Register_Address);
		t7 = Read_16bitReg_ADE7880(CF3DEN_Register_Address);
		printf_syn("[cfxden]reg data:0x%x, 0x%x, 0x%x\n", t5, t6, t7);

		t1 = Read_8bitReg_ADE7880(WTHR_Register_Address);
		printf_syn("[wthr]reg data:0x%x\n", t1);

		t1 = Read_8bitReg_ADE7880(VARTHR_Register_Address);
		printf_syn("[varthr]reg data:0x%x\n", t1);
 

	} else {
		set_pabc_adj_reg(id);
		printf_syn("%s(), recv invalid id(%d), should 1/2\n", __FUNCTION__, id);
	}

	return;
}
FINSH_FUNCTION_EXPORT(set_7880adj_reg, "set ade7880 adjust reg");

static void set_pabc_adj_reg(enum ade7880_adjust_reg_grp_id_e id)
{
	u32_t 	t1, t2, t3;
	u32_t 	addr1, addr2, addr3;
	volatile int *p1, *p2, *p3;
	
	p1 = p2 = p3 = NULL;
	addr1 = addr2 = addr3 = 0;
	
	switch (id) {
	case AARI_PXV_GAIN:
		p1 =  &avgain_reg_data;
		p2 =  &bvgain_reg_data;
		p3 =  &cvgain_reg_data;

		addr1 = AVGAIN_Register_Address;
		addr2 = BVGAIN_Register_Address;
		addr3 = CVGAIN_Register_Address;
		break;

	case AARI_PXI_GAIN:
		p1 =  &aigain_reg_data;
		p2 =  &bigain_reg_data;
		p3 =  &cigain_reg_data;

		addr1 = AIGAIN_Register_Address;
		addr2 = BIGAIN_Register_Address;
		addr3 = CIGAIN_Register_Address;
		break;

	case AARI_PXP_GAIN:
		p1 =  &apgain_reg_data;
		p2 =  &bpgain_reg_data;
		p3 =  &cpgain_reg_data;

		addr1 = APGAIN_Register_Address;
		addr2 = BPGAIN_Register_Address;
		addr3 = CPGAIN_Register_Address;
		break;

	case AARI_PX_WATTOS:
		p1 =  &awattos_reg_data;
		p2 =  &bwattos_reg_data;
		p3 =  &cwattos_reg_data;

		addr1 = AWATTOS_Register_Address;
		addr2 = BWATTOS_Register_Address;
		addr3 = CWATTOS_Register_Address;
		break;


	case AARI_PX_VRMSOS:
		p1 =  &avrmsos_reg_data;
		p2 =  &bvrmsos_reg_data;
		p3 =  &cvrmsos_reg_data;

		addr1 = AVRMSOS_Register_Address;
		addr2 = BVRMSOS_Register_Address;
		addr3 = CVRMSOS_Register_Address;
		break;

	case AARI_PX_IRMSOS:
		p1 =  &airmsos_reg_data;
		p2 =  &birmsos_reg_data;
		p3 =  &cirmsos_reg_data;

		addr1 = AIRMSOS_Register_Address;
		addr2 = BIRMSOS_Register_Address;
		addr3 = CIRMSOS_Register_Address;
		break;

	case AARI_PX_CFXDEN:
		p1 =  &cf1den_reg_data;
		p2 =  &cf2den_reg_data;
		p3 =  &cf3den_reg_data;

		addr1 = CF1DEN_Register_Address;
		addr2 = CF2DEN_Register_Address;
		addr3 = CF3DEN_Register_Address;
		break;

	case AARI_PX_XPHCAL:
		p1 =  &aphcal_reg_data;
		p2 =  &bphcal_reg_data;
		p3 =  &cphcal_reg_data;
		addr1 = APHCAL_Register_Address;
		addr2 = BPHCAL_Register_Address;
		addr3 = CPHCAL_Register_Address;
		break;

	case AARI_PABC_WTHR:
		p1 =  &wthr_reg_data;
		p2 =  &varthr_reg_data;

		addr1 = WTHR_Register_Address;
		addr2 = VARTHR_Register_Address;
		break;


	default:
		printf_syn("%s(), recv invalid id(%d), should 1/2/3/4/5/6/7\n", __FUNCTION__, id);

		return;
	}


	if (0 != addr1)
		{
			if((CF1DEN_Register_Address == addr1) | (APHCAL_Register_Address == addr1)){
				t1 = Read_16bitReg_ADE7880(addr1);
			}
			else{ 
				t1 = Read_32bitReg_ADE7880(addr1);
			}	
		}
	if (0 != addr2)
		{
			if((CF2DEN_Register_Address == addr2) | (BPHCAL_Register_Address == addr2)){
				t2 = Read_16bitReg_ADE7880(addr2);
			}
			else{ 
				t2 = Read_32bitReg_ADE7880(addr2);
			}	
		}
	if (0 != addr3)
		{
			if((CF3DEN_Register_Address == addr3) | (CPHCAL_Register_Address == addr3)){
				t3 = Read_16bitReg_ADE7880(addr3);
			}
			else{ 
				t3 = Read_32bitReg_ADE7880(addr3);
			}	
		}

	printf_syn("--[reg-grp id:%d]--\nnew set data:0x%x, 0x%x, 0x%x\nold reg data:0x%x, 0x%x, 0x%x\n", id,
			NULL!=p1?*p1:0, NULL!=p2?*p2:0, NULL!=p3?*p3:0,
			t1, t2, t3);

	printf_syn("will modify ade7880 dsp reg...\n");

	/* 去能dsp写保护 */
	Write_8bitReg_ADE7880(0xE7FE, 0xAD);
	Write_8bitReg_ADE7880(0xE7E3, 0x00);
	Write_8bitReg_ADE7880(0xE7E3, 0x00);
	Write_8bitReg_ADE7880(0xE7E3, 0x00);

	if (NULL != p1){
		if(CF1DEN_Register_Address != addr1){
			Write_32bitReg_ADE7880(addr1, *p1);
		}else{
			Write_16bitReg_ADE7880(addr1, *p1);
		}
	}	

	if (NULL != p2){
		if(CF2DEN_Register_Address != addr2){
			Write_32bitReg_ADE7880(addr2, *p2);
		}else{
			Write_16bitReg_ADE7880(addr2, *p2);
		}
	}	

	if (NULL != p3){
		if(CF3DEN_Register_Address != addr3){
			Write_32bitReg_ADE7880(addr3, *p3);
		}else{
			Write_16bitReg_ADE7880(addr3, *p3);
		}
	}	


	/* 使能dsp写保护 */
	Write_8bitReg_ADE7880(0xE7FE, 0xAD);
	Write_8bitReg_ADE7880(0xE7E3, 0x80);
	Write_8bitReg_ADE7880(0xE7E3, 0x80);
	Write_8bitReg_ADE7880(0xE7E3, 0x80);

	printf_syn("modify ade7880 dsp reg over.\n");

	if (0 != addr1)
		{
			if(CF1DEN_Register_Address == addr1){
				t1 = Read_16bitReg_ADE7880(addr1);
			}
			else{ 
				t1 = Read_32bitReg_ADE7880(addr1);
			}	
		}
	if (0 != addr2)
		{
			if(CF2DEN_Register_Address == addr2){
				t2 = Read_16bitReg_ADE7880(addr2);
			}
			else{ 
				t2 = Read_32bitReg_ADE7880(addr2);
			}	
		}
	if (0 != addr3)
		{
			if(CF3DEN_Register_Address == addr3){
				t3 = Read_16bitReg_ADE7880(addr3);
			}
			else{ 
				t3 = Read_32bitReg_ADE7880(addr3);
			}	
		}

 	printf_syn("--[reg-grp id:%d]--\nnew reg data:0x%x, 0x%x, 0x%x\n", id, t1, t2, t3);

	return;
}

#if EM_ALL_TYPE_BASE
#define DISABLE_ENC38J60_WHEN_CHECK_ENERGY 0

static void timer4check_energy_init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	TIM_DeInit(CHECK_E_ENERGY_TIMER);

	RCC_APB1PeriphClockCmd(CHECK_E_ENERGY_TIMER_RCC, ENABLE);

	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode	= TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period	= 60000 - 1;	/* 定时时间为 (TIM_Period+1) * Tclk */

	 /* The counter clock frequency CK_CNT is equal to f CK_PSC / (PSC[15:0] + 1)
	  * 10us/clk
	  *  */
	TIM_TimeBaseStructure.TIM_Prescaler	= 72 - 1;
	TIM_TimeBaseInit(CHECK_E_ENERGY_TIMER, &TIM_TimeBaseStructure);

	TIM_ARRPreloadConfig(CHECK_E_ENERGY_TIMER, ENABLE);

  	TIM_ITConfig(CHECK_E_ENERGY_TIMER, TIM_IT_Update, ENABLE);

  	disable_timerx(CHECK_E_ENERGY_TIMER);

	return;
}

static void timer4check_energy_stop(void)
{
	TIM_DeInit(CHECK_E_ENERGY_TIMER);

	return;
}

static void check_act_energy_start(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

	timer4check_energy_init();
#if DISABLE_ENC38J60_WHEN_CHECK_ENERGY
	GPIO_EXTILineConfig(ENC28J60_INT_PORT_SOURCE, ENC28J60_INT_PIN_SOURCE);
	EXTI_InitStructure.EXTI_Line 	= ENC28J60_INT_EXT_LINE;
	EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = DISABLE;
	EXTI_Init(&EXTI_InitStructure);
	clear_exti_it_pending_bit(ENC28J60_INT_EXT_LINE);
#endif
	GPIO_EXTILineConfig(EM_ACTIVE_ENERGY_INT_PORT_SOURCE, EM_ACTIVE_ENERGY_INT_PIN_SOURCE);
	EXTI_InitStructure.EXTI_Line 	= EM_ACTIVE_ENERGY_INT_EXT_LINE;
	EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	GPIO_EXTILineConfig(ADE7880_ACTIVE_ENERGY_INT_PORT_SOURCE, ADE7880_ACTIVE_ENERGY_INT_PIN_SOURCE);
	EXTI_InitStructure.EXTI_Line 	= ADE7880_ACTIVE_ENERGY_INT_EXT_LINE;
	EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EM_ACTIVE_ENERGY_INT_NUM;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EM_EE_PREEMPTION_PRI;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = EM_EE_SUB_PRI;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = ADE7880_ACTIVE_ENERGY_INT_NUM;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = ADE7880_EE_PREEMPTION_PRI;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = ADE7880_EE_SUB_PRI;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	return;
}

static void check_act_energy_over(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

	timer4check_energy_stop();

	NVIC_InitStructure.NVIC_IRQChannel = EM_ACTIVE_ENERGY_INT_NUM;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EM_EE_PREEMPTION_PRI;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = EM_EE_SUB_PRI;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = ADE7880_ACTIVE_ENERGY_INT_NUM;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = ADE7880_EE_PREEMPTION_PRI;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = ADE7880_EE_SUB_PRI;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);

	EXTI_InitStructure.EXTI_Line 	= EM_ACTIVE_ENERGY_INT_EXT_LINE;
	EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = DISABLE;
	EXTI_Init(&EXTI_InitStructure);

	EXTI_InitStructure.EXTI_Line 	= ADE7880_ACTIVE_ENERGY_INT_EXT_LINE;
	EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = DISABLE;
	EXTI_Init(&EXTI_InitStructure);

#if DISABLE_ENC38J60_WHEN_CHECK_ENERGY
	GPIO_EXTILineConfig(ENC28J60_INT_PORT_SOURCE, ENC28J60_INT_PIN_SOURCE);
	EXTI_InitStructure.EXTI_Line 	= ENC28J60_INT_EXT_LINE;
	EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	EXTI_GenerateSWInterrupt(ENC28J60_INT_EXT_LINE);
#endif
	return;
}

static void check_react_energy_start(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

	timer4check_energy_init();

#if DISABLE_ENC38J60_WHEN_CHECK_ENERGY
	GPIO_EXTILineConfig(ENC28J60_INT_PORT_SOURCE, ENC28J60_INT_PIN_SOURCE);
	EXTI_InitStructure.EXTI_Line 	= ENC28J60_INT_EXT_LINE;
	EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = DISABLE;
	EXTI_Init(&EXTI_InitStructure);
	clear_exti_it_pending_bit(ENC28J60_INT_EXT_LINE);
#endif
	GPIO_EXTILineConfig(EM_REACTIVE_ENERGY_INT_PORT_SOURCE, EM_REACTIVE_ENERGY_INT_PIN_SOURCE);
	EXTI_InitStructure.EXTI_Line 	= EM_REACTIVE_ENERGY_INT_EXT_LINE;
	EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	GPIO_EXTILineConfig(ADE7880_REACTIVE_ENERGY_INT_PORT_SOURCE, ADE7880_REACTIVE_ENERGY_INT_PIN_SOURCE);
	EXTI_InitStructure.EXTI_Line 	= ADE7880_REACTIVE_ENERGY_INT_EXT_LINE;
	EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);


	NVIC_InitStructure.NVIC_IRQChannel = EM_REACTIVE_ENERGY_INT_NUM;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EM_EE_PREEMPTION_PRI;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = EM_EE_SUB_PRI;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = ADE7880_REACTIVE_ENERGY_INT_NUM;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = ADE7880_EE_PREEMPTION_PRI;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = ADE7880_EE_SUB_PRI;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	return;
}

static void check_react_energy_over(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

	timer4check_energy_stop();

	NVIC_InitStructure.NVIC_IRQChannel = EM_REACTIVE_ENERGY_INT_NUM;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EM_EE_PREEMPTION_PRI;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = EM_EE_SUB_PRI;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = ADE7880_REACTIVE_ENERGY_INT_NUM;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = ADE7880_EE_PREEMPTION_PRI;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = ADE7880_EE_SUB_PRI;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);

	EXTI_InitStructure.EXTI_Line 	= EM_REACTIVE_ENERGY_INT_EXT_LINE;
	EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = DISABLE;
	EXTI_Init(&EXTI_InitStructure);

	EXTI_InitStructure.EXTI_Line 	= ADE7880_REACTIVE_ENERGY_INT_EXT_LINE;
	EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = DISABLE;
	EXTI_Init(&EXTI_InitStructure);

#if DISABLE_ENC38J60_WHEN_CHECK_ENERGY
	GPIO_EXTILineConfig(ENC28J60_INT_PORT_SOURCE, ENC28J60_INT_PIN_SOURCE);
	EXTI_InitStructure.EXTI_Line 	= ENC28J60_INT_EXT_LINE;
	EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	EXTI_GenerateSWInterrupt(ENC28J60_INT_EXT_LINE);
#endif
	return;
}

static void check_energy_param_init(void)
{
	check_eenergy_timer_clk_period_cnt	= 0;

	em_energy_timer_clks_cnt_start		= 0;
	em_energy_timer_clks_cnt_end		= 0;

	ade7880_energy_timer_clks_cnt_start	= 0;
	ade7880_energy_timer_clks_cnt_end	= 0;

	em_energy_int_cnt		= 0;
	ade7880_energy_int_cnt		= 0;

	return;
}

#define ENERGY_PULSE_TIME_10US_CNT_MIN	(5*100)
/* 脉冲周期最大为104s */
#define ENERGY_PULSE_TIMEOUT_500MS_CNT	(104*3*2+2)
static int em_10us_sum, ade7880_10us_sum, em_7880_pulse_cnt;

#define is_measurement_pulse_over() (em_10us_sum>=ENERGY_PULSE_TIME_10US_CNT_MIN\
				|| ade7880_10us_sum>=ENERGY_PULSE_TIME_10US_CNT_MIN)
/*
 * 57.7V * 3 * 6A = 1038.6W
 * 1038.6WH -- 13294.08 pulse
 * 1s -- 3.69 pulse
 *
 * 留有余量后，脉冲最小周期为0.25s，0.25s的万分之一是25us，所以当前电能精度可以达到万分之一
 * */
static int calc_eenergy_inaccuracy(int is_act_eei)
{
	unsigned int em_energy_timerclk_cnt, ade7880_energy_timerclk_cnt, delta;
	int em_10us, ade7880_10us;
	int index;

	em_energy_timerclk_cnt		= em_energy_timer_clks_cnt_end - em_energy_timer_clks_cnt_start;
	ade7880_energy_timerclk_cnt	= ade7880_energy_timer_clks_cnt_end - ade7880_energy_timer_clks_cnt_start;

	delta = em_energy_timerclk_cnt>ade7880_energy_timerclk_cnt ? em_energy_timerclk_cnt-ade7880_energy_timerclk_cnt
			: ade7880_energy_timerclk_cnt-em_energy_timerclk_cnt;

	check_ee_debug(("60ms_cnt:%u, em_1us start-end:[%u,%u], 7880_1us start-end:[%u,%u], em_int:%d, 7880_int:%d\n",
			check_eenergy_timer_clk_period_cnt,
			em_energy_timer_clks_cnt_start, em_energy_timer_clks_cnt_end,
			ade7880_energy_timer_clks_cnt_start, ade7880_energy_timer_clks_cnt_end,
			em_energy_int_cnt, ade7880_energy_int_cnt));

	em_10us		= em_energy_timerclk_cnt/10;
	ade7880_10us	= ade7880_energy_timerclk_cnt/10;
	check_ee_debug(("em-pulse-period: %d(10us)\nade7880 pulse-period: %d(10us)\ndelta-pulse-period: %d(10us), %d(us)\n",
			em_10us, ade7880_10us, em_10us-ade7880_10us,
			em_energy_timerclk_cnt>ade7880_energy_timerclk_cnt?delta:-delta));

	em_10us_sum		+= em_10us;
	ade7880_10us_sum	+= ade7880_10us;
	if (0==em_10us_sum || 0==ade7880_10us_sum || is_measurement_pulse_over()) {
		if (0 != is_act_eei)
			em_act_ee_inaccuracy = -((em_10us_sum - ade7880_10us_sum)*10000/ade7880_10us_sum);
		else
			em_react_ee_inaccuracy = -((em_10us_sum - ade7880_10us_sum)*10000/ade7880_10us_sum);

		index = si_get_cur_em_index();
		check_ee_debug(("[ch#%d]em(SN:%s) act(react) ee inaccuracy:%d, %d\n",
				index+1, index>=0 ? register_em_info.registered_em_sn[index]:"",
				em_act_ee_inaccuracy, em_react_ee_inaccuracy));
	}

	return 0;
}

#if 0!=IS_CHECK_EENERGY_USE_THREAD
#if 0
static void rt_check_eenergy_entry(void* parameter);

void check_energy(int cmd)
{
	rt_thread_t thread_h1;

	switch (cmd) {
	case 1:
	case 2:
		thread_h1 = rt_thread_create("check_ee", rt_check_eenergy_entry, &cmd, 512, 16, 10);
		if (thread_h1 != RT_NULL) {
			rt_thread_startup(thread_h1);
			printf_syn("rt_check_eenergy_entry had start...\n");
		} else {
			printf_syn("create rt_check_eenergy_entry fail\n");
		}

		break;

	default:
		printf_syn("func:%s(), recv invalid cmd(%d)\n", __FUNCTION__, cmd);
		break;
	}

	return;
}
FINSH_FUNCTION_EXPORT(check_energy, "1-act-energy, 2-react-energy");


static void rt_check_eenergy_entry(void* parameter)
{
	int delay, cmd;
	unsigned int em_energy_timerclk_cnt, ade7880_energy_timerclk_cnt, delta;

	delay = 0;
	cmd = *(int *)parameter;

	switch (cmd) {
	case 1:
		if (ECS_IDLE != check_eenergy_state) {
			printf_syn("func:%s(), doing check energy\n", __FUNCTION__);
		} else {
			check_energy_param_init();
			check_act_energy_start();

			while (ECS_CHECK_OVER != check_eenergy_state) {
				if (delay++ < 20) {
					rt_thread_delay(get_ticks_of_ms(500));
				} else {
					printf_syn("================ error:wait over timeout ================\n");
					break;
				}
			}

			check_act_energy_over();
			check_eenergy_state = ECS_IDLE;
			printf_syn("================ check act energy done ================\n");
		}
		break;

	case 2:
		if (ECS_IDLE != check_eenergy_state) {
			printf_syn("func:%s(), doing check energy\n", __FUNCTION__);
		} else {
			check_energy_param_init();
			check_react_energy_start();

			rt_thread_delay(get_ticks_of_ms(100));

			while (ECS_CHECK_OVER != check_eenergy_state) {
				if (delay++ < 20) {
					rt_thread_delay(get_ticks_of_ms(500));
				} else {
					printf_syn("================ error:wait over timeout ================\n");
					break;
				}
			}

			check_react_energy_over();
			check_eenergy_state = ECS_IDLE;
			printf_syn("================ check react energy done ================\n");
		}
		break;

	default:
		printf_syn("func:%s(), recv invalid cmd(%d)\n", __FUNCTION__, cmd);
		goto ret_entry;
//		break;
	}

	em_energy_timerclk_cnt		= em_energy_timer_clks_cnt_end - em_energy_timer_clks_cnt_start;
	ade7880_energy_timerclk_cnt	= ade7880_energy_timer_clks_cnt_end - ade7880_energy_timer_clks_cnt_start;

	delta = em_energy_timerclk_cnt>ade7880_energy_timerclk_cnt ? em_energy_timerclk_cnt-ade7880_energy_timerclk_cnt
			: ade7880_energy_timerclk_cnt-em_energy_timerclk_cnt;

	printf_syn("em-plus-period: %d(10us)\nade7880 plus-period: %d(10us)\ndelta-plus-period: %d(10us)\n",
			em_energy_timerclk_cnt/10, ade7880_energy_timerclk_cnt/10, delta/10);

	printf_syn("60ms_cnt:%d, em_1us start-end:[%d,%d], 7880_1us start-end:[%d,%d], em_int:%d, 7880_int:%d\n",
			check_eenergy_timer_clk_period_cnt,
			em_energy_timer_clks_cnt_start, em_energy_timer_clks_cnt_end,
			ade7880_energy_timer_clks_cnt_start, ade7880_energy_timer_clks_cnt_end,
			em_energy_int_cnt, ade7880_energy_int_cnt);

	printf_syn("================ check eenergy thread over ================\n");

ret_entry:
	return;
}
#else

static int calc_eenergy_inaccuracy(int is_act_eei);

void rt_check_eenergy_entry(void* parameter)
{
	rt_err_t ret;
	rt_uint32_t e;
	int delay;

	while(1) {

		ret = rt_event_recv(&sinkinfo_event_set, SI_EVENT_EM_EEINACCURACY_START,
				RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &e);
		if (RT_EOK != ret) {
			printf_syn("%s(), line:%d, recv event fail(%d)\n", __FUNCTION__, __LINE__, ret);
			rt_thread_delay(get_ticks_of_ms(1500));
			continue;
		}

		rt_thread_delay(get_ticks_of_ms(1000)); /* 等待信号稳定 */

		/* 实时有功电能误差 */
		check_ee_debug(("================ check act energy start ================\n"));
		em_10us_sum		= 0;
		ade7880_10us_sum	= 0;
		em_7880_pulse_cnt	= 0;
act_ee_again:
		delay = 0;
		if (ECS_IDLE != check_eenergy_state) {
			printf_syn("func:%s(), doing check energy\n", __FUNCTION__);
		} else {
			check_energy_param_init();
			check_act_energy_start();

			while (ECS_CHECK_OVER != check_eenergy_state) {
				if (delay <= ENERGY_PULSE_TIMEOUT_500MS_CNT/2
						|| (delay<ENERGY_PULSE_TIMEOUT_500MS_CNT
								&& ECS_IDLE != check_eenergy_state)) {
					++delay;
					rt_thread_delay(get_ticks_of_ms(500));
				} else {
					check_eenergy_timeout(1);
					check_ee_debug(("================ act energy error:wait over timeout ================\n"));
					break;
				}
			}

			check_act_energy_over();
			check_eenergy_state = ECS_IDLE;
		}
		++em_7880_pulse_cnt;

		calc_eenergy_inaccuracy(1);

		if ((em_10us_sum && ade7880_10us_sum) && !is_measurement_pulse_over()) {
			goto act_ee_again;
		}

		check_ee_debug(("================ check act energy done ================\n\n"));

		rt_thread_delay(get_ticks_of_ms(500));
#if 1
		/* 实时无功电能误差 */
		check_ee_debug(("================ check react energy start ================\n"));
		em_10us_sum		= 0;
		ade7880_10us_sum	= 0;
		em_7880_pulse_cnt	= 0;
react_ee_again:
		delay = 0;
		if (ECS_IDLE != check_eenergy_state) {
			printf_syn("func:%s(), doing check energy\n", __FUNCTION__);
		} else {
			check_energy_param_init();
			check_react_energy_start();

//			rt_thread_delay(get_ticks_of_ms(100));

			while (ECS_CHECK_OVER != check_eenergy_state) {
				if (delay <= ENERGY_PULSE_TIMEOUT_500MS_CNT/2
						|| (delay<ENERGY_PULSE_TIMEOUT_500MS_CNT
								&& ECS_IDLE != check_eenergy_state)) {
					++delay;
					rt_thread_delay(get_ticks_of_ms(500));
				} else {
					check_eenergy_timeout(0);
					check_ee_debug(("================ react energy error:wait over timeout ================\n"));
					break;
				}
			}

			check_react_energy_over();
			check_eenergy_state = ECS_IDLE;
		}
		++em_7880_pulse_cnt;

		calc_eenergy_inaccuracy(0);

		if ((em_10us_sum && ade7880_10us_sum) && !is_measurement_pulse_over()) {
			goto react_ee_again;
		}

		check_ee_debug(("================ check react energy done ================\n\n"));
#endif

		rt_event_send(&sinkinfo_event_set, SI_EVENT_EM_EEINACCURACY_OVER);
	}

	return;
}
#endif

#else /* #if 0!=IS_CHECK_EENERGY_USE_THREAD */

void check_electric_energy(void)
{
	int delay;

	/* 实时有功电能误差 */
	delay = 0;
	check_ee_debug(("================ check act energy start ================\n"));
	if (ECS_IDLE != check_eenergy_state) {
		printf_syn("func:%s(), doing check energy\n", __FUNCTION__);
	} else {
		check_energy_param_init();
		check_act_energy_start();

		while (ECS_CHECK_OVER != check_eenergy_state) {
			if (delay++ < 40) {
				rt_thread_delay(get_ticks_of_ms(500));
			} else {
				check_ee_debug(("================ act energy error:wait over timeout ================\n"));
				break;
			}
		}

		check_act_energy_over();
		check_eenergy_state = ECS_IDLE;
	}

	calc_eenergy_inaccuracy(1);
	check_ee_debug(("================ check act energy done ================\n\n"));

//		rt_thread_delay(get_ticks_of_ms(500));
#if 1
	/* 实时无功电能误差 */
	delay = 0;
	check_ee_debug(("================ check react energy start ================\n"));
	if (ECS_IDLE != check_eenergy_state) {
		printf_syn("func:%s(), doing check energy\n", __FUNCTION__);
	} else {
		check_energy_param_init();
		check_react_energy_start();

		rt_thread_delay(get_ticks_of_ms(100));

		while (ECS_CHECK_OVER != check_eenergy_state) {
			if (delay++ < 40) {
				rt_thread_delay(get_ticks_of_ms(500));
			} else {
				check_ee_debug(("================ react energy error:wait over timeout ================\n"));
				break;
			}
		}

		check_react_energy_over();
		check_eenergy_state = ECS_IDLE;
	}
	calc_eenergy_inaccuracy(0);
	check_ee_debug(("================ check react energy done ================\n\n"));

	rt_event_send(&sinkinfo_event_set, SI_EVENT_EM_EEINACCURACY_OVER);

//	rt_thread_delay(get_ticks_of_ms(60 * 1000));
#endif


	return;
}

#endif

#endif /* #if EM_ALL_TYPE_BASE */
