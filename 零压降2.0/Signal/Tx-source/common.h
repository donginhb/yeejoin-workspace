#ifndef COMMON_H__
#define COMMON_H__

#define UART_AMPLIFY_ADDR		(0x0801FC00)
#define MAGIC_NUM_OF_CFG_PARAM_TBL  	(0x5A12A578UL)

#define TX_VERSION	2L      /* major version number */
#define TX_SUBVERSION	1L      /* minor version number */
#define TX_REVISION	1L      /* revise version number */

#define DEBUG_CHECKINFO_TRAN 0

#define SELF_LEARN_STDV_START	(0xa512345a)
#define SELF_LEARN_STDV_END	(0x5a1234a5)

/*
 * 实测值是16116(0x3ef4), mark by David
 *
 * #define ORIG_AVERAGE_VALUE  (15360)
 */
#define ORIG_AVERAGE_VALUE  (16116)

#if 1 /* David */
/*
 * !!命令中使用网络序
 *
 * 0x47   G
 * 0x4E   N
 * 0x53   S
 *
 * @@固化参数
 * 将配置参数写入flash, 需要发送字节序列0x69, 0xaa, 0xaa
 */
enum SERIAL_PORT_COMMAND {
	SPC_GET_DEV_SN				= 0X47, /* GSN, 0X47.0X53.0X4E */
	SPC_SET_DEV_SN				= 0X53, /* SSN, 0X53.0X53.0X4E */
	
        WRITE_CFG_PARAM2FLASH                   = 0x69,   /* 将配置参数写入flash, 需要发送字节序列0x69, 0xaa, 0xaa */
        READ_SYS_RUN_TIME                       = 0x6A,   /* 读系统运行时间, 需要发送字节序列0x6a, 0xa5, 0xa5 */
        READ_TEMP_VALUE                         = 0x6B,   /* 读温度传感器的值, 需要发送字节序列0x6b, 0xa5, 0xa5 */
        READ_VERSION                            = 0x6C,   /* 读软件版本号, 需要发送字节序列0x6c, 0xa5, 0xa5 */
        READ_SET_PARAM_VALUE                    = 0x6D,   /* 读参数的设定值, 需要发送字节序列0x6d, 0xa5, 0xa5 */

        SELF_LEARN_STDV_ADC_VALU		= 0x70, /* 自学习标准电压对应的adc值 */
        SET_PX_ADC_VALU_MAX_PERCENT		= 0x74, /* 设置A/B/C相上限值比标准值多出的百分比 */
        SET_PX_ADC_VALU_MIN_PERCENT		= 0x75, /* 设置A/B/C相下限值比标准值多出的百分比 */
        
#if 1==DEBUG_CHECKINFO_TRAN
        SET_DEBUG_PARAM				= 0x80,	/*  */
        CLR_DEBUG_PARAM				= 0x81,	/*  */
#endif
        READ_DEBUG_PARAM1			= 0x82,	/*  */
        WRITE_DEBUG_PARAM1			= 0x83,	/*  */
        WRITE_DEBUG_PARAM2			= 0x84,	/*  */
        WRITE_DEBUG_PARAM3			= 0x85,	/*  */
        WRITE_DEBUG_PARAM4			= 0x86,	/*  */
        WRITE_DEBUG_PARAM5			= 0x87,	/*  */

        ENTER_SET_PARAM_MODE                    = 0x90,   /* 进入设定参数模式, 需要发送字节序列0x90, 0xa5, 0xa5 */
        EXIT_SET_PARAM_MODE                     = 0x91,   /* 退出设定参数模式, 需要发送字节序列0x91, 0xa5, 0xa5 */
};


enum READ_DEBUG_PARAM1_COMMAND {
	RDP_READ_TEMP_PARAM			= 0X0001,
	RDP_READ_TEMP_PARAM1			= 0X0002,
	RDP_READ_TEMP_PARAM2			= 0X0003,
	RDP_READ_TEMP_PARAM3			= 0X0004,
	RDP_READ_TEMP_PARAM4			= 0X0005,

	RDP_READ_ADC_SAMPLE_VAL			= 0X5A50,	/* 读ADC的瞬时采样值 */
	RDP_READ_ADC_SAMPLE_AVG_VAL		= 0X5A51,	/* 读ADC的采样平均值 */
	RDP_READ_LEARN_AVG_VAL			= 0X5A52,	/* 读学习到的ABC三相的标准电压adc值 */

	RDP_READ_PX_STATE			= 0X5A53,	/* 读ABC三相的状态 */

};

#endif


/* 缓冲区大小 */
#define UART1_DMA_BUF_LEN 4

/* 缓冲区存放数据的个数 */
#define UART1_DMA_BUF_CNT 3


#if 1==DEBUG_CHECKINFO_TRAN
#define PA_TOO_L	0xa1
#define PA_TOO_H	0xa2
#define PA_LOST		0xa3

#define PB_TOO_L	0xb1
#define PB_TOO_H	0xb2
#define PB_LOST		0xb3

#define PC_TOO_L	0xc1
#define PC_TOO_H	0xc2
#define PC_LOST		0xc3
#endif


extern volatile unsigned int timer3_int_cnt;

/*
 * bit 0: pa lost phase
 * bit 1: pb lost phase
 * bit 2: pc lost phase
 * bit 3: sent data
 */
#define SYS_MISC_FLAG_PA_LOST 0X01
#define SYS_MISC_FLAG_PB_LOST 0X02
#define SYS_MISC_FLAG_PC_LOST 0X04
#define SYS_MISC_FLAG_SEND_DATA 0X08

extern volatile unsigned int sys_misc_flag;

#define set_pa_lost_flag() set_bit(sys_misc_flag, SYS_MISC_FLAG_PA_LOST)
#define clr_pa_lost_flag() clr_bit(sys_misc_flag, SYS_MISC_FLAG_PA_LOST)
#define is_pa_lost_phase() is_bit_set(sys_misc_flag, SYS_MISC_FLAG_PA_LOST)

#define set_pb_lost_flag() set_bit(sys_misc_flag, SYS_MISC_FLAG_PB_LOST)
#define clr_pb_lost_flag() clr_bit(sys_misc_flag, SYS_MISC_FLAG_PB_LOST)
#define is_pb_lost_phase() is_bit_set(sys_misc_flag, SYS_MISC_FLAG_PB_LOST)

#define set_pc_lost_flag() set_bit(sys_misc_flag, SYS_MISC_FLAG_PC_LOST)
#define clr_pc_lost_flag() clr_bit(sys_misc_flag, SYS_MISC_FLAG_PC_LOST)
#define is_pc_lost_phase() is_bit_set(sys_misc_flag, SYS_MISC_FLAG_PC_LOST)

#define set_send_data_flag() set_bit(sys_misc_flag, SYS_MISC_FLAG_SEND_DATA)
#define clr_send_data_flag() clr_bit(sys_misc_flag, SYS_MISC_FLAG_SEND_DATA)
#define is_send_data() is_bit_set(sys_misc_flag, SYS_MISC_FLAG_SEND_DATA)

#endif
