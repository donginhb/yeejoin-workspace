#ifndef COMMON_H__
#define COMMON_H__

/* !!NOTE:是否编译为OpticX-200S的软件 */
#define USE_OPTICX_200S_VERSION	1


#if USE_OPTICX_200S_VERSION
#define MAGIC_NUM_OF_CFG_PARAM_TBL	(0xA512A578UL)
#else
#define MAGIC_NUM_OF_CFG_PARAM_TBL	(0x5A12A578UL)
#endif

#define UART_AMPLIFY_ADDR		(0x0801FC00)

#define NUM_OF_PX_ZERO_POS_MIC_ADJ_CNT  (100)
#define IDEAL_ZERO_POSITION_VALUE 	(32768)

/* 理论计算值 */
#define REAL_ZERO_POSITION_DEF_VALUE 	(32380)




/*
 * 版本号约定：
 * 	OpticX-200的主版本号为偶数
 * 	OpticX-200S的主版本号为奇数
 * */
#if USE_OPTICX_200S_VERSION
#define RX_VERSION	3L      /* major version number, OpticX-200S的主版本号为奇数 */
#define RX_SUBVERSION	1L      /* minor version number */
#define RX_REVISION	10L     /* revise version number */
#else
#define RX_VERSION	2L      /* major version number, OpticX-200的主版本号为偶数 */
#define RX_SUBVERSION	1L      /* minor version number */
#define RX_REVISION	9L     /* revise version number */
#endif


/*
 * 0x4000 -- 16384
 */
#define PX_ZERO_POSITION_DEF_VAL    	(0X8000UL)
#define PX_AMPLIFY_DEF_VAL          	(0X8000UL)
#define PX_AMP_FACTOR_POSITIVE_DEF_VAL	(0X8000UL)
#define PX_AMP_FACTOR_NEGTIVE_DEF_VAL   (0X8000UL)
#define PX_PHASE_TIME_DEF_VAL       	(16384L)

#define PA_AMPLIFY_DEF_VAL          (PX_AMPLIFY_DEF_VAL)
#define PB_AMPLIFY_DEF_VAL          (PX_AMPLIFY_DEF_VAL)
#define PC_AMPLIFY_DEF_VAL          (PX_AMPLIFY_DEF_VAL)

#define PA_PHASE_TIME_DEF_VAL       (PX_PHASE_TIME_DEF_VAL)
#define PB_PHASE_TIME_DEF_VAL       (PX_PHASE_TIME_DEF_VAL)
#define PC_PHASE_TIME_DEF_VAL       (PX_PHASE_TIME_DEF_VAL)

#define PA_ZERO_POSITION_DEF_ADJ_VAL    (IDEAL_ZERO_POSITION_VALUE)
#define PB_ZERO_POSITION_DEF_ADJ_VAL    (IDEAL_ZERO_POSITION_VALUE)
#define PC_ZERO_POSITION_DEF_ADJ_VAL    (IDEAL_ZERO_POSITION_VALUE)

#define PA_AMP_FACTOR_POSITIVE_DEF_VAL  (PX_AMP_FACTOR_POSITIVE_DEF_VAL)
#define PB_AMP_FACTOR_POSITIVE_DEF_VAL  (PX_AMP_FACTOR_POSITIVE_DEF_VAL)
#define PC_AMP_FACTOR_POSITIVE_DEF_VAL  (PX_AMP_FACTOR_POSITIVE_DEF_VAL)

#define PX_AMPLIFY_BASE         (32768L)
#define PX_AMP_FACTOR_P_BASE    (32768L)
#define PX_AMP_FACTOR_N_BASE    (32768L)
#define PX_PHASE_TIME_BASE      (16384L)

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
 *   调零位之前需要发送"开始零位调整"命令--即字节序列(0xC8, 0xA5, 0xA5)
 *   调零位之后需要发送"停止零位调整"命令--即字节序列(0xC9, 0xA5, 0xA5)
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
        READ_SYS_RUN_TIME                       = 0x6A,   /* 读系统运行时间, 需要发送字节序列0x6a, 0xa5, 0xa5 */
        READ_TEMP_VALUE                         = 0x6B,   /* 读温度传感器的值, 需要发送字节序列0x6b, 0xa5, 0xa5 */
        READ_VERSION                            = 0x6C,   /* 读软件版本号, 需要发送字节序列0x6c, 0xa5, 0xa5 */
        READ_SET_PARAM_VALUE                    = 0x6D,   /* 读参数的设定值, 需要发送字节序列0x6d, 0xa5, 0xa5 */
        SET_CHANNEL_VALID_INDICATION_LEVEL	= 0x6E,	/* 配置通道指示信号有效电平, 需要发送字节序列0x6E, 0xa5, 0xa*  */

        READ_DEBUG_PARAM1			= 0x82,	/*  */
        WRITE_DEBUG_PARAM1			= 0x83,	/*  */
        WRITE_DEBUG_PARAM2			= 0x84,	/*  */
        WRITE_DEBUG_PARAM3			= 0x85,	/*  */
        WRITE_DEBUG_PARAM4			= 0x86,	/*  */
        WRITE_DEBUG_PARAM5			= 0x87,	/*  */

        ENTER_SET_PARAM_MODE                    = 0x90,   /* 进入设定参数模式, 需要发送字节序列0x90, 0xa5, 0xa5 */
        EXIT_SET_PARAM_MODE                     = 0x91,   /* 退出设定参数模式, 需要发送字节序列0x91, 0xa5, 0xa5 */

        ADJ_PA_AMPLIFY                          = 0xAA,   /* 调整A相幅值 */
        ADJ_PB_AMPLIFY                          = 0xAB,
        ADJ_PC_AMPLIFY                          = 0xAC,

        ADJ_PA_PHASE_TIME                       = 0xBA,   /* 调整A相相位 */
        ADJ_PB_PHASE_TIME                       = 0xBB,
        ADJ_PC_PHASE_TIME                       = 0xBC,
#if 0
        START_ZERO_POSITION_ADJ                 = 0xC8,   /* 开始零位调整, 需要发送字节序列0xc8, 0xa5, 0xa5 */
        STOP_ZERO_POSITION_ADJ                  = 0xC9,   /* 停止零位调整, 需要发送字节序列0xc9, 0xa5, 0xa5 */
#endif
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

enum READ_DEBUG_PARAM1_COMMAND {
	RDP_READ_TEMP_PARAM			= 0X0001,
	RDP_READ_TEMP_PARAM1			= 0X0002,
	RDP_READ_TEMP_PARAM2			= 0X0003,
	RDP_READ_TEMP_PARAM3			= 0X0004,
	RDP_READ_TEMP_PARAM4			= 0X0005,
	RDP_READ_PX_REAL_ZERO_POS_VAL		= 0X5A50,	/* 读ABC三相真实零位值 */
	RDP_READ_OVERLOAD_PARAM_VAL		= 0X5A51,	/* 读过载相关参数 */
	RDP_READ_AVG_VAL_SUM			= 0X5A52,
};


struct px_peak_trough_info_st {
	/* The difference between the peaks and troughs */
	int px_diff_peak_trough;

	/* Instantaneous, 即时更新 */
	int px_instant_peak_val;
	int px_instant_trough_val;

	/* Approximate, 每隔一定时间段更新一次 */
	int px_approx_peak_val;
	int px_approx_trough_val;
	int px_approx_zero_pos;
};

#define RECORD_ADC_PX_PEAK_TROUGH_INFO	0
#define RECORD_DAC_PX_PEAK_TROUGH_INFO	1
#define PROC_ESTIMATE_DELTA_Y		1


#define NUM_ADC_SAMPLE  		(220)
#define NUM_INTERVAL_FOR_SETIMATE	(NUM_ADC_SAMPLE/4)

#if PROC_ESTIMATE_DELTA_Y
/* interval estimation */
struct interval_for_estimate_info_st {
	int real_sin_val_of_interval[NUM_INTERVAL_FOR_SETIMATE];
	int real_delta_y_max_interval[NUM_INTERVAL_FOR_SETIMATE];
};
#endif

#if 0
#define reset_dma_spi1_tx     do { CS1_H; DMA_Configuration_SPI1_TX();} while (0)
#define start_dma_spi1_tx     do { CS1_L; enable_dmax_y(DMA1_Channel3); enable_spix(SPI1);} while (0)
#define wait_dma_spi1_tx_over do {  while(!is_spix_txbuf_empty(SPI1)) ; while(is_spix_busy(SPI1)) ; }while (0)
#define dma_spi1_tx_had_voer  do { CS1_H; disable_spix(SPI1); } while (0)
#endif

extern volatile int cur_fiber_channel_no;
extern volatile int non_change_txe_info_send_cnt;



#endif
