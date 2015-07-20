#ifndef TX_RX_COMM_H__
#define TX_RX_COMM_H__

/*
 * 年份 year 
 * 产地 place of production
 * 批次 batch
 * 发射/接收
 * 序号 serial NO
 */
#define DEV_SN_MODE "yypp-bbxssss"
#define DEV_SN_MODE_LEN  12
#define DEV_SN_5BITS_CNT 20

#define DEV_SN_5BITS_BUF_LEN 48

#define DEV_SN_LEN_MAX 23

/*
 * 传输15bits
 */
enum tx_rx_msg_mid { /* 3-bits */
	TRMI_CONNECT = 0,
	TRMI_PHASE_A = 1,
	TRMI_PHASE_B = 2,
	TRMI_PHASE_C = 3,
	TRMI_CPU_TEMPER = 4, /* 传输高15bits */
	TRMI_BOX_TEMPER = 5, /* 传输高15bits */
	TRMI_MISC_INFO = 6,  /* 第一个传主从id */
};

enum tx_rx_msg_sid { /* 5-bits */
	TRMIS_NONE = 0,
	TRMIS_PA_BININFO  = 1, /* bit4:过低, bit3:过高, bit2:缺相 */
	TRMIS_PB_BININFO  = 2,
	TRMIS_PC_BININFO  = 3,
	TRMIS_TX_VERSION  = 4, /* 共传送20bits, 主版本号使用高6bits, 其余各占7bits */
	TRMIS_TX_POWEROFF = 5, /* 采集端掉电 */
	TRMIS_TX_DEV_SN	  = 6, /*  */
};

#define convert_id2firstbyte(id)	(id<<5)
#define PHASEX_ID_MASK			(0x07<<5)
#define DATA_BITS_PRE_BYTE	(5)

#define get_dac_hbyte(val) ((val)>>8 & 0xff)
#define get_dac_lbits(val) ((val) & 0xff)


#define USE_12BITS_ADC_VAL 0
#define USE_15BITS_ADC_VAL 1


/* 此处的val是16bits */
#if USE_12BITS_ADC_VAL
#define get_height_adc_val(val) (((val)>>10) & 0x3f)
#define get_low_adc_val(val) 	(((val)>>4) & 0x3f)
#elif USE_15BITS_ADC_VAL
#define get_height_adc_val(val) (((val)>>11) & 0x1f)
#define get_middle_adc_val(val) (((val)>>6) & 0x1f)
#define get_low_adc_val(val) 	(((val)>>1) & 0x1f)
#endif


#define SYS_TICK_DIV_CNT	(33000)
//#define SYS_TICK_DIV_CNT	(30000)

#define XUS_PER_SYSTICK		(1000000/SYS_TICK_DIV_CNT)
#define SYSTICK_NUM_PER1S 	(1000000/XUS_PER_SYSTICK)
#define SYSTICK_NUM_PER750MS 	(750000/XUS_PER_SYSTICK)
#define SYSTICK_NUM_PER250MS 	(250000/XUS_PER_SYSTICK)

#define SYSTICK_NUM_PER100MS 	(100000/XUS_PER_SYSTICK)
#define SYSTICK_NUM_PER200MS 	(200000/XUS_PER_SYSTICK)
#define SYSTICK_NUM_PER300MS 	(300000/XUS_PER_SYSTICK)
#define SYSTICK_NUM_PER400MS 	(400000/XUS_PER_SYSTICK)
#define SYSTICK_NUM_PER500MS 	(500000/XUS_PER_SYSTICK)
#define SYSTICK_NUM_PER600MS 	(600000/XUS_PER_SYSTICK)
#define SYSTICK_NUM_PER700MS 	(700000/XUS_PER_SYSTICK)
#define SYSTICK_NUM_PER800MS 	(800000/XUS_PER_SYSTICK)
#define SYSTICK_NUM_PER900MS 	(900000/XUS_PER_SYSTICK)

/* 这里只是计算了三相adc数据, 没有包含检测数据在内 */
#define FIBRE_DATA_BYTES_PER1S  (3 * SYSTICK_NUM_PER1S)
#define FIBRE_DATA_BYTES_PER1MS ((3 * SYSTICK_NUM_PER1S)/1000)
#define FIBRE_DATA_BYTES_GAP    FIBRE_DATA_BYTES_PER1MS

extern int split_bytes_to_5bits(const unsigned char *bytes, int byten, unsigned char *bits_5, int bitn);
extern int merge_5bits_to_bytes(unsigned char *bytes, int byten, const unsigned char *bits_5, int bitn);


#endif

