/********************
* 头 文 件 配 置 区 *
********************/ 
#include "..\Source\LIB_Config.h"
#include "..\Source\PF_Config.H"	
#include "sys_comm.h"


#define SIGS_VERSION "SIGS-3.3"

#define UART_AMPLIFY_ADDR		(0x0801FC00) 


#if 1 /* David */
/*
 * !!命令中使用网络序
 *
 * @@固化参数
 *   将配置参数写入flash, 需要发送字节序列0x69, 0xaa, 0xaa
 *
 * @@调整幅值
 *   命令后有两个字节的参数, 默认值是0x8000(十进制数是32768)
 *
 * @@调整相位
 *   命令后有两个字节的参数, 默认值是0x8000(十进制数是32768)
 *
 * @@调整零位
 *   调零位之前需要发送"开始零位调整"命令--即字节序列(0xCD, 0x5A, 0x5A)
 *   调零位之后需要发送"停止零位调整"命令--即字节序列(0xCE, 0xA5, 0xA5)
 *   命令后有两个字节的参数, 默认值是0x0000, 第一个字节是粗调, 第二个字节是正向微调
 *       用于粗调的第一个字节采用有符号数的原码表示, 即bit7为符号位
 *       用于微调的第二个字节采用无符号数的原码表示, 即bit7~bit0均为数据位, 取值范围是[0, 100]
 *       #define NUM_OF_PX_ZERO_POS_MIC_ADJ_CNT  (100)  ---- 方波一个周期对应的systick中断次数
 *
 * @@调整正半周放大倍数
 *   命令后有两个字节的参数, 默认值是0x8000(十进制数是32768)
 *
 * @@调整负半周放大倍数
 *   命令后有两个字节的参数, 默认值是0x8000(十进制数是32768)
 *
 *
 */
enum SERIAL_PORT_COMMAND {
    WRITE_CFG_PARAM2FLASH                   = 0x69,   /* 将配置参数写入flash, 需要发送字节序列0x69, 0xaa, 0xaa */

    READ_TEMP_VALUE                         = 0x6B,   /* 读温度传感器的值, 需要发送字节序列0x6b, 0xa5, 0xa5 */
    READ_VERSION                            = 0x6C,   /* 读软件版本号, 需要发送字节序列0x6c, 0xa5, 0xa5 */
    READ_SET_PARAM_VALUE                    = 0x6D,   /* 读参数的设定值, 需要发送字节序列0x6d, 0xa5, 0xa5 */

    ENTER_SET_PARAM_MODE                    = 0x90,   /* 进入设定参数模式, 需要发送字节序列0x90, 0xa5, 0xa5 */
    EXIT_SET_PARAM_MODE                     = 0x91,   /* 退出设定参数模式, 需要发送字节序列0x91, 0xa5, 0xa5 */
    READ_DEBUG_INFO_0                       = 0x92,   /* 读调试信息, 需要发送字节序列0x92, 0xa5, 0xa5 */
    READ_DEBUG_INFO_1                       = 0x93,   /* 读调试信息, 需要发送字节序列0x93, 0xa5, 0xa5 */
    READ_DEBUG_INFO_2                       = 0x94,   /* 读调试信息, 需要发送字节序列0x94, 0xa5, 0xa5 */
    READ_DEBUG_INFO_3                       = 0x95,   /* 读调试信息, 需要发送字节序列0x95, 0xa5, 0xa5 */

    ADJ_PA_AMPLIFY                          = 0xAA,   /* 调整A相幅值 */
    ADJ_PB_AMPLIFY                          = 0xAB,
    ADJ_PC_AMPLIFY                          = 0xAC,

    ADJ_PA_PHASE_TIME                       = 0xBA,   /* 调整A相相位 */
    ADJ_PB_PHASE_TIME                       = 0xBB,
    ADJ_PC_PHASE_TIME                       = 0xBC,

    START_ZERO_POSITION_ADJ                 = 0xC8,   /* 开始零位调整, 需要发送字节序列0xc8, 0xa5, 0xa5 */
    STOP_ZERO_POSITION_ADJ                  = 0xC9,   /* 停止零位调整, 需要发送字节序列0xc9, 0xa5, 0xa5 */
    ADJ_PA_ZERO_POSITION                    = 0xCA,   /* 调整A相零位 */
    ADJ_PB_ZERO_POSITION                    = 0xCB,
    ADJ_PC_ZERO_POSITION                    = 0xCC,

    ADJ_PA_AMPLIFICATION_FACTOR_POSITIVE    = 0xDA,   /* 调整A相正半周放大系数 */
    ADJ_PB_AMPLIFICATION_FACTOR_POSITIVE    = 0xDB,    
    ADJ_PC_AMPLIFICATION_FACTOR_POSITIVE    = 0xDC,    
    ADJ_PA_AMPLIFICATION_FACTOR_NEGTIVE     = 0xDD,   /* 调整A相负半周放大系数 */
    ADJ_PB_AMPLIFICATION_FACTOR_NEGTIVE     = 0xDE,    
    ADJ_PC_AMPLIFICATION_FACTOR_NEGTIVE     = 0xDF,   
};

#endif



struct signal_cfg_param_tbl {
    u32 magic_num;
    u16 pa_amplify_adj;             /* default value: 0x8000 */
    u16 pb_amplify_adj;
    u16 pc_amplify_adj;

    u16 pa_phase_time_adj;          /* default value: 0x8000 */
    u16 pb_phase_time_adj;
    u16 pc_phase_time_adj;

    u16 pa_zero_position_adj;       /* default value: 0x00 */
    u16 pb_zero_position_adj;
    u16 pc_zero_position_adj;

    u16 pa_amp_factor_positive_adj;   /* amplification_factor default value: 0x8000 */
    u16 pb_amp_factor_positive_adj;
    u16 pc_amp_factor_positive_adj;

    u16 pa_amp_factor_negtive_adj;    /* default value: 0x8000 */
    u16 pb_amp_factor_negtive_adj;
    u16 pc_amp_factor_negtive_adj;
    u16 pad;
};

#define MAGIC_NUM_OF_CFG_PARAM_TBL  (0x5A12A578UL)
#define PX_AMPLIFY_DEF_VAL          (0X8000UL)
#define PX_PHASE_TIME_DEF_VAL       (0X8000UL)
#define PX_ZERO_POSITION_DEF_VAL    (0X8000UL)
#define PX_AMP_FACTOR_POSITIVE_DEF_VAL  (0X8000UL)
#define PX_AMP_FACTOR_NEGTIVE_DEF_VAL   (0X8000UL)

#define PA_AMPLIFY_DEF_VAL          (0X56A0UL)
#define PB_AMPLIFY_DEF_VAL          (0X53A0UL)
#define PC_AMPLIFY_DEF_VAL          (0X53E0UL)

#define PA_PHASE_TIME_DEF_VAL       (0X6DD2UL)
#define PB_PHASE_TIME_DEF_VAL       (0X7396UL)
#define PC_PHASE_TIME_DEF_VAL       (0X8E06UL)

#define PA_ZERO_POSITION_DEF_VAL    (0X8300UL)
#define PB_ZERO_POSITION_DEF_VAL    (0X9800UL)
#define PC_ZERO_POSITION_DEF_VAL    (0X9800UL)

#define PA_AMP_FACTOR_POSITIVE_DEF_VAL  (0X8080UL)
#define PB_AMP_FACTOR_POSITIVE_DEF_VAL  (0X8080UL)
#define PC_AMP_FACTOR_POSITIVE_DEF_VAL  (0X8080UL)
#define IDEAL_ZERO_POSITION_VALUE (2048)



/* 0x8000 = 2^15 = 32768, 0x4000 = 2^14 = 16384 */
#define PX_AMPLIFY_BASE         (32768L)
#define PX_PHASE_TIME_BASE      (16384L)
#define PX_AMP_FACTOR_P_BASE    (32768L)
#define PX_AMP_FACTOR_N_BASE    (32768L)

#define PX_AMPLIFY_BASE_SHIFT       (15)
#define PX_PHASE_TIME_BASE_SHIFT    (14)
#define PX_AMP_FACTOR_P_BASE_SHIFT  (15)
#define PX_AMP_FACTOR_N_BASE_SHIFT  (15)




#define NUM_OF_RECORDING_DELTA_Y (3)
#define UPDATE_OLD_DELTA_Y(delta_y, delta_y_old)  do {\
    delta_y_old[0] = delta_y_old[1];\
    delta_y_old[1] = delta_y_old[2];\
    delta_y_old[2] = delta_y;\
}while(0)
#define ESTIMATE_DELTA_Y_VALUE(delta_y_seq) (((delta_y_seq[2]) + (delta_y_seq[1]) + (delta_y_seq[0])) / 3)

static s16 pa_delta_y_old[NUM_OF_RECORDING_DELTA_Y];
static s16 pb_delta_y_old[NUM_OF_RECORDING_DELTA_Y];
static s16 pc_delta_y_old[NUM_OF_RECORDING_DELTA_Y];

static struct signal_cfg_param_tbl signal_cfg_param;

volatile int is_need_send_new_data2dac = 0;
volatile s32 pa_org_value;   /* 保存查表数据 */
volatile s32 pb_org_value;
volatile s32 pc_org_value;

s32 pa_org_value_pre;   /* 保存上一次数据 */
s32 pb_org_value_pre;
s32 pc_org_value_pre;

static int is_doing_adj_zero_position = 0;

volatile s32 pa_zero_position_value;
volatile s32 pb_zero_position_value;
volatile s32 pc_zero_position_value;

u32 pa_zero_pos_mic_adj_cnt; /* 用于0位微调, 调整占空比 */
u32 pb_zero_pos_mic_adj_cnt;
u32 pc_zero_pos_mic_adj_cnt;

/* 方波的逻辑电平对应的ad值 */
u16 pa_zero_pos_square_lgc_0; 
u16 pa_zero_pos_square_lgc_1;

u16 pb_zero_pos_square_lgc_0; 
u16 pb_zero_pos_square_lgc_1;

u16 pc_zero_pos_square_lgc_0; 
u16 pc_zero_pos_square_lgc_1;


static int is_had_enter_set_param_mode = 0;

/* 温度传感器的值 */
extern volatile s32  dwTemper;
extern volatile int is_over_load;

#if 0
#define UPDATE_PX_ZERO_POS_ADJ_PARAM(param, sys_tbl_px_param, px_zero_pos_val, px_zero_pos_mic_adj, temp)  do{\
    sys_tbl_px_param = param & 0xffff;\
    temp = sys_tbl_px_param >> 8;\
    if (temp & 0x80)\
        px_zero_pos_val = IDEAL_ZERO_POSITION_VALUE - (temp & 0x7f);\
    else\
        px_zero_pos_val = IDEAL_ZERO_POSITION_VALUE + temp;\
    temp = sys_tbl_px_param & 0xff;\
    if (temp > 128)\
        px_zero_pos_mic_adj = NUM_OF_PX_ZERO_POS_MIC_ADJ_CNT;\
    else if (temp < 0)\
        px_zero_pos_mic_adj = 0;\
    else\
        px_zero_pos_mic_adj = temp;\
}while(0)
#else
/* 为了调整简单, 将零位调整的默认值设为0x8000
 * 将temp = (temp * NUM_OF_PX_ZERO_POS_MIC_ADJ_CNT) / 255;中的255改为256
 */
#define UPDATE_PX_ZERO_POS_ADJ_PARAM(param, sys_tbl_px_param, px_zero_pos_val, px_zero_pos_mic_adj, temp)  do{\
    sys_tbl_px_param = param & 0xffff;\
    temp = sys_tbl_px_param >> 8;\
    px_zero_pos_val = IDEAL_ZERO_POSITION_VALUE + temp - (1<<7);\
    temp = sys_tbl_px_param & 0xff;\
    temp = (temp * NUM_OF_PX_ZERO_POS_MIC_ADJ_CNT) / 256;\
    px_zero_pos_mic_adj = temp;\
}while(0)
#endif


static void _get_sig_cfg_param(volatile struct signal_cfg_param_tbl *cfg_param, int is_init);
static void init_signal_cfg_param_tbl(volatile struct signal_cfg_param_tbl *cfg_param);
static void read_sig_cfg_param_from_flash(volatile struct signal_cfg_param_tbl *cfg_param);
FLASH_Status write_sig_cfg_param_to_flash(volatile struct signal_cfg_param_tbl *cfg_param);
FLASH_Status write_sig_cfg_param_to_flash_for_init(volatile struct signal_cfg_param_tbl *cfg_param);

void adj_px_data(s32 *pa, s32 *pb, s32 *pc);

extern void usr_cmd_analysis(void);
extern int is_cmd_valid(int cmd);
extern void usr_cmd_proc(int cmd, s32 param);
extern void usart1_use_buf_tx( int data, int cnt);
extern void usart1_use_buf_tx_lf( int data, int cnt);



/*
 *******************************************************************************
 */

extern unsigned long sys_ticks_cnt;
 
#define NUM_OF_DONT_ADJ_DATA  (30)

/* 1 tick = 66.67us */
#define TICKS_OF_1S     (1000000 / 66.67)
#define RUN_LED_HZ                  (3)
#define RUN_LED_HZ_OF_OVERLOAD      (1)

#define RUN_LED_HZ_CNT              (1.0/RUN_LED_HZ * TICKS_OF_1S)
#define RUN_LED_HZ_CNT_OF_OVERLOAD  (1.0/RUN_LED_HZ_OF_OVERLOAD * TICKS_OF_1S)

unsigned long run_led_cnt;

/***********************************************************
*   函数说明：系统主函数                                   *
*   输入：    无                                           *
*   输出：    无                                           *
*   调用函数：无                                           *
***********************************************************/
int main(void)
{	
	s32 pa_value, pb_value, pc_value;
	static int ingnore_cnt = 0;

	System_Init();	   //与库没有关系 这个是片内设备和片外设备的总初始化

    SPI_DMA_Table[0] = DAC_CLR_CMD;  //其他数据字节不管
    
    TIM_Cmd(TIM2, ENABLE);
    TIM_Cmd(TIM3, ENABLE);

	FIBER_STATE_OFF;  
    CS1_H;
    run_led_cnt = sys_ticks_cnt;
    
    while (1) {	 
		FIBER_STATE_OFF;  
		usr_cmd_analysis();

        /* 运行指示灯 */
        if (0 != is_over_load) {
            if ((sys_ticks_cnt - run_led_cnt) > RUN_LED_HZ_CNT_OF_OVERLOAD/2) {
                run_led_cnt = sys_ticks_cnt;
                GPIOB->ODR ^= GPIO_Pin_8;
            }
        } else {
            if ((sys_ticks_cnt - run_led_cnt) > RUN_LED_HZ_CNT/2) {
                run_led_cnt = sys_ticks_cnt;
                GPIOB->ODR ^= GPIO_Pin_8;
            }
        }

        if (0 != is_need_send_new_data2dac) {
            is_need_send_new_data2dac = 0;
            if (0 != is_over_load) {
                CS1_H;
            	SPI_DMA_Table[0] = DAC_CLR_CMD;
            	DMA_Configuration_SPI1_TX();
            	CS1_L;
            	enable_dmax_y(DMA1_Channel3);
            } else {
                if (0 != is_doing_adj_zero_position) { /* 正在进行零位调整 */
                    pa_value = pa_zero_position_value;
                    pb_value = pb_zero_position_value;
                    pc_value = pc_zero_position_value;
                    ingnore_cnt = 0;
                } else { /* 正常输出三相数据 */
                    #if 1
                    if (ingnore_cnt > NUM_OF_DONT_ADJ_DATA) {
                        adj_px_data(&pa_value, &pb_value, &pc_value);
                    } else {
                        ++ingnore_cnt;
                        pa_value = pa_org_value;
                        pb_value = pb_org_value;
                        pc_value = pc_org_value;
                    }
                    #else /* test */
                    pa_value = pa_org_value;
                    pb_value = pb_org_value;
                    pc_value = pc_org_value;
                    #endif

                    UPDATE_OLD_DELTA_Y(pa_value - pa_org_value_pre, pa_delta_y_old);
                    UPDATE_OLD_DELTA_Y(pb_value - pb_org_value_pre, pb_delta_y_old);
                    UPDATE_OLD_DELTA_Y(pc_value - pc_org_value_pre, pc_delta_y_old);

                    pa_org_value_pre = pa_value;
                    pb_org_value_pre = pb_value;
                    pc_org_value_pre = pc_value;
                }

            	SPI_DMA_Table[0] = CMD_DAC0;
            	SPI_DMA_Table[1] = (pa_value>>4) & 0xff;
            	SPI_DMA_Table[2] = (pa_value<<4) & 0xf0;  //低字节 放到 数组最后
            	DMA_Configuration_SPI1_TX();
            	CS1_L;
            	DMA_Cmd(DMA1_Channel3,ENABLE);	 //DMA通道号 使能
                while(	SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE) == RESET )
                ;

            	SPI_DMA_Table[0] = CMD_DAC1;
            	SPI_DMA_Table[1] = (pb_value>>4) & 0xff;
            	SPI_DMA_Table[2] = (pb_value<<4) & 0xf0;  //低字节 放到 数组最后
            	DMA_Configuration_SPI1_TX();
            	CS1_L;
            	DMA_Cmd(DMA1_Channel3,ENABLE);	 //DMA通道号 使能
                while(	SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE) == RESET )
                ;

            	SPI_DMA_Table[0] = CMD_DAC2;
            	SPI_DMA_Table[1] = (pc_value>>4) & 0xff;
            	SPI_DMA_Table[2] = (pc_value<<4) & 0xf0;  //低字节 放到 数组最后
            	DMA_Configuration_SPI1_TX();
            	CS1_L;
            	DMA_Cmd(DMA1_Channel3,ENABLE);	 //DMA通道号 使能
             }
         }

	} 

}


static void adj_px_data(s32 *pa, s32 *pb, s32 *pc)
{
    s32 pa_value, pb_value, pc_value;

    /* 根据零位对采样值进行修正, 零位值可能会动态变化 +/-1 */
    pa_value = pa_org_value + (pa_zero_position_value - IDEAL_ZERO_POSITION_VALUE);
    pb_value = pb_org_value + (pb_zero_position_value - IDEAL_ZERO_POSITION_VALUE);
    pc_value = pc_org_value + (pc_zero_position_value - IDEAL_ZERO_POSITION_VALUE);
#if 1
    /* 根据幅值对采样值进行修正 */
    /* pa_value = pa_zero_position_value + ((pa_value - pa_zero_position_value) * signal_cfg_param.pa_amplify_adj / PX_AMPLIFY_BASE); */
    pa_value = pa_zero_position_value + ((pa_value - pa_zero_position_value) * (s32)signal_cfg_param.pa_amplify_adj / PX_AMPLIFY_BASE);
    pb_value = pb_zero_position_value + ((pb_value - pb_zero_position_value) * (s32)signal_cfg_param.pb_amplify_adj / PX_AMPLIFY_BASE);
    pc_value = pc_zero_position_value + ((pc_value - pc_zero_position_value) * (s32)signal_cfg_param.pc_amplify_adj / PX_AMPLIFY_BASE);
#endif

    /* 根据放大系数对采样值进行微调 */
    if (pa_value >= pa_zero_position_value)
        pa_value = pa_zero_position_value
                    + ((pa_value - pa_zero_position_value) * (s32)signal_cfg_param.pa_amp_factor_positive_adj / PX_AMP_FACTOR_P_BASE);
#if 0
    else 
        pa_value = pa_zero_position_value
                    + ((pa_value - pa_zero_position_value) * ((s32)signal_cfg_param.pa_amp_factor_negtive_adj & 0xffff) / PX_AMP_FACTOR_N_BASE);
#endif
    if (pb_value >= pb_zero_position_value)
        pb_value = pb_zero_position_value
                    + ((pb_value - pb_zero_position_value) * ((s32)signal_cfg_param.pb_amp_factor_positive_adj & 0xffff) / PX_AMP_FACTOR_P_BASE);
#if 0
    else 
        pb_value = pb_zero_position_value
                    + ((pb_value - pb_zero_position_value) * ((s32)signal_cfg_param.pb_amp_factor_negtive_adj & 0xffff) / PX_AMP_FACTOR_N_BASE);
#endif
    if (pc_value >= pc_zero_position_value)
        pc_value = pc_zero_position_value
                    + ((pc_value - pc_zero_position_value) * ((s32)signal_cfg_param.pc_amp_factor_positive_adj & 0xffff) / PX_AMP_FACTOR_P_BASE);
#if 0
    else 
        pc_value = pc_zero_position_value
                    + ((pc_value - pc_zero_position_value) * ((s32)signal_cfg_param.pc_amp_factor_negtive_adj & 0xffff) / PX_AMP_FACTOR_N_BASE);
#endif    
    /* 根据相位对采样值进行修正 */
    /* delta值不需要变符号 */
    pa_value +=  ESTIMATE_DELTA_Y_VALUE(pa_delta_y_old)*((s32)signal_cfg_param.pa_phase_time_adj - PX_PHASE_TIME_DEF_VAL) / PX_PHASE_TIME_BASE;
    pb_value +=  ESTIMATE_DELTA_Y_VALUE(pb_delta_y_old)*((s32)signal_cfg_param.pb_phase_time_adj - PX_PHASE_TIME_DEF_VAL) / PX_PHASE_TIME_BASE;
    pc_value +=  ESTIMATE_DELTA_Y_VALUE(pc_delta_y_old)*((s32)signal_cfg_param.pc_phase_time_adj - PX_PHASE_TIME_DEF_VAL) / PX_PHASE_TIME_BASE;

    *pa = pa_value;
    *pb = pb_value;
    *pc = pc_value;
    return;
}




/*
 *******************************************************************************
 */
/* !!!NOTE: isr 中会修改 px_zero_position_value */
void get_sig_cfg_param(int is_init)
{
	int temp, px_zero_pos_value;

    _get_sig_cfg_param(&signal_cfg_param, is_init); /* David */

    UPDATE_PX_ZERO_POS_ADJ_PARAM(signal_cfg_param.pa_zero_position_adj, signal_cfg_param.pa_zero_position_adj,
                                    px_zero_pos_value, pa_zero_pos_mic_adj_cnt, temp);
    pa_zero_pos_square_lgc_0 = px_zero_pos_value; 
    pa_zero_pos_square_lgc_1 = px_zero_pos_value + 1; 

    UPDATE_PX_ZERO_POS_ADJ_PARAM(signal_cfg_param.pb_zero_position_adj, signal_cfg_param.pb_zero_position_adj,
                                    px_zero_pos_value, pb_zero_pos_mic_adj_cnt, temp);
    pb_zero_pos_square_lgc_0 = px_zero_pos_value; 
    pb_zero_pos_square_lgc_1 = px_zero_pos_value + 1; 

    UPDATE_PX_ZERO_POS_ADJ_PARAM(signal_cfg_param.pc_zero_position_adj, signal_cfg_param.pc_zero_position_adj,
                                    px_zero_pos_value, pc_zero_pos_mic_adj_cnt, temp);
    pc_zero_pos_square_lgc_0 = px_zero_pos_value; 
    pc_zero_pos_square_lgc_1 = px_zero_pos_value + 1;


    return;
}


static void _get_sig_cfg_param(volatile struct signal_cfg_param_tbl *cfg_param, int is_init)
{
    read_sig_cfg_param_from_flash(cfg_param);

    if (MAGIC_NUM_OF_CFG_PARAM_TBL != cfg_param->magic_num) {
        init_signal_cfg_param_tbl(cfg_param);
        if (0 != is_init)
            write_sig_cfg_param_to_flash_for_init(cfg_param);  /* 写flash有可能失败 */        
        else
            write_sig_cfg_param_to_flash(cfg_param);  /* 写flash有可能失败 */        
    }

    return;
}



static void init_signal_cfg_param_tbl(volatile struct signal_cfg_param_tbl *cfg_param)
{
    cfg_param->magic_num            = MAGIC_NUM_OF_CFG_PARAM_TBL;
    cfg_param->pa_amplify_adj       = PA_AMPLIFY_DEF_VAL;
    cfg_param->pb_amplify_adj       = PB_AMPLIFY_DEF_VAL;
    cfg_param->pc_amplify_adj       = PC_AMPLIFY_DEF_VAL;

    cfg_param->pa_phase_time_adj    = PX_PHASE_TIME_DEF_VAL;
    cfg_param->pb_phase_time_adj    = PX_PHASE_TIME_DEF_VAL;
    cfg_param->pc_phase_time_adj    = PX_PHASE_TIME_DEF_VAL;

    cfg_param->pa_zero_position_adj = PX_ZERO_POSITION_DEF_VAL;
    cfg_param->pb_zero_position_adj = PX_ZERO_POSITION_DEF_VAL;
    cfg_param->pc_zero_position_adj = PX_ZERO_POSITION_DEF_VAL;

    cfg_param->pa_amp_factor_positive_adj = PX_AMP_FACTOR_POSITIVE_DEF_VAL;
    cfg_param->pb_amp_factor_positive_adj = PX_AMP_FACTOR_POSITIVE_DEF_VAL;
    cfg_param->pc_amp_factor_positive_adj = PX_AMP_FACTOR_POSITIVE_DEF_VAL;

    cfg_param->pa_amp_factor_negtive_adj  = PX_AMP_FACTOR_NEGTIVE_DEF_VAL;
    cfg_param->pb_amp_factor_negtive_adj  = PX_AMP_FACTOR_NEGTIVE_DEF_VAL;
    cfg_param->pc_amp_factor_negtive_adj  = PX_AMP_FACTOR_NEGTIVE_DEF_VAL;

    cfg_param->pad = 0xffff;

    return;
}

static void read_sig_cfg_param_from_flash(volatile struct signal_cfg_param_tbl *cfg_param)
{
    u32 *ps, *pd;
    int i;

    ps = (u32 *)UART_AMPLIFY_ADDR;
    pd = (u32 *)cfg_param;
    for (i=0; i < (sizeof(struct signal_cfg_param_tbl)/4); i++) {
        *pd++ = *ps++;

    }
}

FLASH_Status write_sig_cfg_param_to_flash(volatile struct signal_cfg_param_tbl *cfg_param)
{
    u32 *ps, *pd, temp;
    int i;
    FLASH_Status status;

	FLASH_Unlock();
    /* Clear pending flags (if any) */  
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPTERR | FLASH_FLAG_WRPRTERR | 
                  FLASH_FLAG_PGERR | FLASH_FLAG_BSY); 

#if 1
    status = FLASH_ErasePage(UART_AMPLIFY_ADDR);	  //刷除 数据
	if (FLASH_COMPLETE != status)
	    while(1); //return status;
#endif
    ps = (u32 *)cfg_param;
    pd = (u32 *)UART_AMPLIFY_ADDR;
    for (i=0; i < (sizeof(struct signal_cfg_param_tbl)/4); i++) {
        temp = *ps;
        status = FLASH_ProgramWord((u32)pd , temp);
        usart1_use_buf_tx(temp, 4);

    	if (FLASH_COMPLETE != status)
    	    while(1); //return status;
        ++pd;
        ++ps;

    }
	FLASH_Lock();

	return FLASH_COMPLETE;
}





FLASH_Status write_sig_cfg_param_to_flash_for_init(volatile struct signal_cfg_param_tbl *cfg_param)
{
    u32 *ps, *pd, temp;
    int i;
    FLASH_Status status;

	FLASH_Unlock();
    /* Clear pending flags (if any) */  
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPTERR | FLASH_FLAG_WRPRTERR | 
                  FLASH_FLAG_PGERR | FLASH_FLAG_BSY); 

#if 1
    status = FLASH_ErasePage(UART_AMPLIFY_ADDR);	  //刷除 数据
	if (FLASH_COMPLETE != status)
	    while(1); //return status;
#endif
    ps = (u32 *)cfg_param;
    pd = (u32 *)UART_AMPLIFY_ADDR;
    for (i=0; i < (sizeof(struct signal_cfg_param_tbl)/4); i++) {
        temp = *ps;
        status = FLASH_ProgramWord((u32)pd , temp);

    	if (FLASH_COMPLETE != status)
    	    while(1); //return status;
        ++pd;
        ++ps;

    }
	FLASH_Lock();

	return FLASH_COMPLETE;
}


/*
 *******************************************************************************
 */
enum uart_recv_state {
    UART_RECV_NULL       = 0,
    UART_RECV_1_BYTE     = 1,
    UART_RECV_2_BYTE     = 2,
    UART_RECV_3_BYTE     = 3,
};

void usr_cmd_analysis(void)
{
    static int cmd, param;
    static int revc_state = UART_RECV_NULL;

	disable_usartx_recv_int(USART1);
    switch (revc_state) {
    case UART_RECV_NULL:
        if(0 != USART1_GetRxBufferCurrentSize()) {
			cmd = USART1_GetRxBufferData(); /* mark */
			revc_state = UART_RECV_1_BYTE;
	    } else {
	        break;
	    }

        if(0 != USART1_GetRxBufferCurrentSize()) {
			param = USART1_GetRxBufferData(); /* mark */
			revc_state = UART_RECV_2_BYTE;
	    } else {
	        break;
	    }

        if(0 != USART1_GetRxBufferCurrentSize()) {
			param = param<<8 | USART1_GetRxBufferData();
			revc_state = UART_RECV_3_BYTE;
	    } else {
	        break;
	    }
        break;

    case UART_RECV_1_BYTE:
        if(0 != USART1_GetRxBufferCurrentSize()) {
			param = USART1_GetRxBufferData();
			revc_state = UART_RECV_2_BYTE;
	    } else {
	        break;
	    }

        if(0 != USART1_GetRxBufferCurrentSize()) {
			param = param<<8 | USART1_GetRxBufferData();
			revc_state = UART_RECV_3_BYTE;
	    } else {
	        break;
	    }
        break;

    case UART_RECV_2_BYTE:
        if(0 != USART1_GetRxBufferCurrentSize()) {
			param = param<<8 | USART1_GetRxBufferData();
			revc_state = UART_RECV_3_BYTE;
	    } else {
	        break;
	    }
        break;
/*
    case UART_RECV_3_BYTE:
        break;
*/
    default:
        break;
    }


    if (UART_RECV_3_BYTE == revc_state) {
        revc_state = UART_RECV_NULL;
        usr_cmd_proc(cmd, param);
    } else if (revc_state >= UART_RECV_1_BYTE) {
        if (!is_cmd_valid(cmd)) {
            revc_state = UART_RECV_NULL;
            USART1_FlushRxBuffer();
        }
    }

    enable_usartx_recv_int(USART1);

    return;
}



int is_cmd_valid(int cmd)
{
    switch (cmd) {
    case WRITE_CFG_PARAM2FLASH               :
    case READ_TEMP_VALUE                     :
    case READ_VERSION                        :
    case READ_SET_PARAM_VALUE:
    case ENTER_SET_PARAM_MODE                :
    case EXIT_SET_PARAM_MODE                 :
    case READ_DEBUG_INFO_0                   :
    case READ_DEBUG_INFO_1                   :
    case READ_DEBUG_INFO_2                   :
    case READ_DEBUG_INFO_3                   :
    case ADJ_PA_AMPLIFY                      :
    case ADJ_PB_AMPLIFY                      :
    case ADJ_PC_AMPLIFY                      :
    case ADJ_PA_PHASE_TIME                   :
    case ADJ_PB_PHASE_TIME                   :
    case ADJ_PC_PHASE_TIME                   :
    case START_ZERO_POSITION_ADJ:
    case STOP_ZERO_POSITION_ADJ:
    case ADJ_PA_ZERO_POSITION                :
    case ADJ_PB_ZERO_POSITION                :
    case ADJ_PC_ZERO_POSITION                :
    case ADJ_PA_AMPLIFICATION_FACTOR_POSITIVE:
    case ADJ_PB_AMPLIFICATION_FACTOR_POSITIVE:
    case ADJ_PC_AMPLIFICATION_FACTOR_POSITIVE:
    case ADJ_PA_AMPLIFICATION_FACTOR_NEGTIVE :
    case ADJ_PB_AMPLIFICATION_FACTOR_NEGTIVE :
    case ADJ_PC_AMPLIFICATION_FACTOR_NEGTIVE :
        return 1;

    default:
        return 0;
    }

}

extern volatile int exti2_cnt;

void usr_cmd_proc(int cmd, s32 param)
{
    int temp, px_zero_pos_value;

    temp = (cmd << 16) | (param & 0xffff);
    if (0==is_had_enter_set_param_mode && 0x90a5a5!=temp) {
        return;
    }

    usart1_use_buf_tx(temp, 3);
    switch(cmd) {
    case WRITE_CFG_PARAM2FLASH: /* 将配置参数写入flash, 需要发送字节序列0x69, 0xaa, 0xaa */
        if ((param & 0xffff) == 0xAAAA)
            write_sig_cfg_param_to_flash(&signal_cfg_param);
        break;

    case READ_TEMP_VALUE: /* 读温度传感器的值, 需要发送字节序列0x6b, 0xa5, 0xa5 */
        if ((param & 0xffff) == 0xA5A5) {
            usart1_use_buf_tx(dwTemper, 4);
        }
        break;
    case READ_VERSION:  /* 读软件版本号, 需要发送字节序列0x6c, 0xa5, 0xa5 */
        if ((param & 0xffff) == 0xA5A5) {
            temp = *(unsigned int*)SIGS_VERSION;
            usart1_use_buf_tx_lf(temp, 4);

            temp = *(unsigned int*)(SIGS_VERSION + 4);
            usart1_use_buf_tx_lf(temp, 4);

        }
        break;

    case ENTER_SET_PARAM_MODE:  /* 进入设定参数模式, 需要发送字节序列0x90, 0xa5, 0xa5 */
        if ((param & 0xffff) == 0xA5A5) {
            is_had_enter_set_param_mode = 1;
        }
        break;
    case EXIT_SET_PARAM_MODE:  /* 退出设定参数模式, 需要发送字节序列0x91, 0xa5, 0xa5 */
        if ((param & 0xffff) == 0xA5A5) {
            is_had_enter_set_param_mode = 0;
        }
        break;

    case READ_DEBUG_INFO_0:      /* 读调试信息, 需要发送字节序列0x92, 0xa5, 0xa5 */
        if ((param & 0xffff) == 0xA5A5) {
            usart1_use_buf_tx(exti2_cnt, 4);
        }
        break;

    case READ_DEBUG_INFO_1:      /* 读调试信息, 需要发送字节序列0x93, 0xa5, 0xa5 */
        if ((param & 0xffff) == 0xA5A5) {
        }
        break;

    case READ_DEBUG_INFO_2:      /* 读调试信息, 需要发送字节序列0x94, 0xa5, 0xa5 */
        if ((param & 0xffff) == 0xA5A5) {
        }
        break;

    case READ_DEBUG_INFO_3:      /* 读调试信息, 需要发送字节序列0x95, 0xa5, 0xa5 */
        if ((param & 0xffff) == 0xA5A5) {
        }
        break;


#if 1
    case ADJ_PA_AMPLIFY:   /* 调整A相幅值 */
        signal_cfg_param.pa_amplify_adj = param & 0xffff;
        break;
    case ADJ_PB_AMPLIFY:
        signal_cfg_param.pb_amplify_adj = param & 0xffff;
        break;
    case ADJ_PC_AMPLIFY:
        signal_cfg_param.pc_amplify_adj = param & 0xffff;
        break;
#endif
    case ADJ_PA_PHASE_TIME:   /* 调整A相相位 */
        signal_cfg_param.pa_phase_time_adj = param & 0xffff;
        break;
    case ADJ_PB_PHASE_TIME:
        signal_cfg_param.pb_phase_time_adj = param & 0xffff;
        break;
    case ADJ_PC_PHASE_TIME:
        signal_cfg_param.pc_phase_time_adj = param & 0xffff;
        break;

    case START_ZERO_POSITION_ADJ:   /* 开始零位调整 */
        if (0xa5a5 == param)
            is_doing_adj_zero_position = 1;
        break;
    case STOP_ZERO_POSITION_ADJ:   /* 停止零位调整 */
        if (0xa5a5 == param)
            is_doing_adj_zero_position = 0;
        break;
    case ADJ_PA_ZERO_POSITION:   /* 调整A相零位 */
        UPDATE_PX_ZERO_POS_ADJ_PARAM(param, signal_cfg_param.pa_zero_position_adj, px_zero_pos_value,
                                        pa_zero_pos_mic_adj_cnt, temp);
        pa_zero_pos_square_lgc_0 = px_zero_pos_value; 
        pa_zero_pos_square_lgc_1 = px_zero_pos_value + 1; 
        break;
    case ADJ_PB_ZERO_POSITION:
        UPDATE_PX_ZERO_POS_ADJ_PARAM(param, signal_cfg_param.pb_zero_position_adj, px_zero_pos_value,
                                        pb_zero_pos_mic_adj_cnt, temp);
        pb_zero_pos_square_lgc_0 = px_zero_pos_value; 
        pb_zero_pos_square_lgc_1 = px_zero_pos_value + 1; 
        break;
    case ADJ_PC_ZERO_POSITION:
        UPDATE_PX_ZERO_POS_ADJ_PARAM(param, signal_cfg_param.pc_zero_position_adj, px_zero_pos_value,
                                        pc_zero_pos_mic_adj_cnt, temp);
        pc_zero_pos_square_lgc_0 = px_zero_pos_value; 
        pc_zero_pos_square_lgc_1 = px_zero_pos_value + 1; 
        break;

    case ADJ_PA_AMPLIFICATION_FACTOR_POSITIVE:   /* 调整A相正半周放大系数 */
        signal_cfg_param.pa_amp_factor_positive_adj = param & 0xffff;
        break;
    case ADJ_PB_AMPLIFICATION_FACTOR_POSITIVE:
        signal_cfg_param.pb_amp_factor_positive_adj = param & 0xffff;
        break;
    case ADJ_PC_AMPLIFICATION_FACTOR_POSITIVE:
        signal_cfg_param.pc_amp_factor_positive_adj = param & 0xffff;
        break;

    case ADJ_PA_AMPLIFICATION_FACTOR_NEGTIVE:   /* 调整A相负半周放大系数 */
        signal_cfg_param.pa_amp_factor_negtive_adj = param & 0xffff;
        break;
    case ADJ_PB_AMPLIFICATION_FACTOR_NEGTIVE:
        signal_cfg_param.pb_amp_factor_negtive_adj = param & 0xffff;
        break;
    case ADJ_PC_AMPLIFICATION_FACTOR_NEGTIVE:
        signal_cfg_param.pc_amp_factor_negtive_adj = param & 0xffff;
        break;


    case READ_SET_PARAM_VALUE:
        if ((param & 0xffff) == 0xA5A5) {
            send_data_to_pc(signal_cfg_param.pa_zero_position_adj, 2);
            send_data_to_pc(signal_cfg_param.pa_amp_factor_positive_adj, 2);
            send_data_to_pc(signal_cfg_param.pa_amplify_adj, 2);
            send_data_to_pc(signal_cfg_param.pa_phase_time_adj, 2);

            send_data_to_pc(signal_cfg_param.pb_zero_position_adj, 2);
            send_data_to_pc(signal_cfg_param.pb_amp_factor_positive_adj, 2);
            send_data_to_pc(signal_cfg_param.pb_amplify_adj, 2);
            send_data_to_pc(signal_cfg_param.pb_phase_time_adj, 2);

            send_data_to_pc(signal_cfg_param.pc_zero_position_adj, 2);
            send_data_to_pc(signal_cfg_param.pc_amp_factor_positive_adj, 2);
            send_data_to_pc(signal_cfg_param.pc_amplify_adj, 2);
            send_data_to_pc(signal_cfg_param.pc_phase_time_adj, 2);
        }
        break;


    default:
        break;
   
    }
    return;
}




/*
 * 按字节来发送一个字, 先发送高位字节
 * cnt是发送的字节个数, 取值范围是[1, 4], 小于4时截取地位字节发送
 */
void usart1_use_buf_tx( int data, int cnt)
{
    int ret, int_state;
    char *p;

    int_state = get_usartx_send_int_state(USART1);
    disable_usartx_send_int(USART1);
    if (cnt > 4) {
        cnt = 4;
    } else if (cnt < 1) {
        cnt = 1;
    }
    
    p = (char *)&data;
    switch (cnt) {
    /*
    case 1:
        break;
    */
    case 2:
        ++p;
        break;
    case 3:
        p += 2;
        break;

    case 4:
        p += 3;
        break;

    default:
        break;
    }

    do {
        ret = USART1_PutDatatoTxBuffer(*p); /* return 0 -- failure */
        if (ret) {
            --cnt;
            --p;
        } else {
            break; /* 这里只做简单处理, 缓冲区满时, 丢弃未发送到数据, 使用中没有集中发送大于128B的数据不会发生这种情况 */
        }
    } while (cnt);

    if (1 != int_state)
        put_usartx_data(USART1, USART1_GetTxBufferData());

    enable_usartx_send_int(USART1);
    
    return;
}



/*
 * 按字节来发送一个字, 先发送低位字节
 * cnt是发送的字节个数, 取值范围是[1, 4], 小于4时截取地位字节发送
 */
void usart1_use_buf_tx_lf( int data, int cnt)
{
    int ret, int_state;
    char *p;

    int_state = get_usartx_send_int_state(USART1);
    disable_usartx_send_int(USART1);
    if (cnt > 4) {
        cnt = 4;
    } else if (cnt < 1) {
        cnt = 1;
    }
    
    p = (char *)&data;
    do {
        ret = USART1_PutDatatoTxBuffer(*p); /* return 0 -- failure */
        if (ret) {
            --cnt;
            ++p;
        } else {
            break; /* 这里只做简单处理, 缓冲区满时, 丢弃未发送到数据, 使用中没有集中发送大于128B的数据不会发生这种情况 */
        }
    } while (cnt);

    if (1 != int_state)
        put_usartx_data(USART1, USART1_GetTxBufferData());

    enable_usartx_send_int(USART1);
    
    return;
}



