/*
 * info_tran.h
 *
 * 2013-03-03,  creat by David, zhaoshaowei@yeejoin.com
 */

#ifndef INFO_TRAN_H__
#define INFO_TRAN_H__

#include <rtdef.h>
#include <zvd_gui_app.h>

/*
 * ���:		year(��λ����)
 * ����:		place of production(��Ч�ַ���26����д��ĸ)
 * ����:		batch(��λ����)
 * ����/����:	T/R
 * ����ͬ���豸���: number
 * ���:		  serial NO(Լ��0001��5999ΪOpticX-200�ı�ţ�6000��9999ΪOpticX-200S�ı��)
 */
/*
 * ����"����ͬ���豸���"�ĺ������£�����OpticX-200�豸���ֵ��Ϊ1������OpticX-200S�Ľ����豸��Ϊ1��
 * 	����OpticX-200S�ķ����豸һ��Ϊ1����һ��Ϊ2.
 * OpticX-200��һ���豸��������һ���Ƿ���ˣ�һ���ǽ��նˣ������ߵ�sn���ֻ�ǣ�"yyp-bbxnssss"�е�xλ�ã�T/R����
 * 	���磬(14A-01T10001, 14A-01R10001)����һ��OpticX-200�豸��
 * OpticX-200S��һ���豸����������������ˣ�һ�����նˣ���������˵�sn���ֻ��"yyp-bbxnssss"�е�nλ��(1/2)��
 * 	���磬(14A-01T16000, 14A-01T26000, 14A-01R16000),��һ��OpticX-200S�豸��
 */
#define DEV_SN_MODE "yyp-bbxnssss"
#define DEV_SN_MODE_LEN  12
#define DEV_SN_5BITS_CNT 20

#define DEV_SN_5BITS_BUF_LEN 48

#define DEV_SN_LEN_MAX 23

/* OpticX-200/OpticX-200S�豸������SN�е��±�λ�� */
#define OPTICX200_DEV_TYPE_IN_SN_INDEX	6
#define OPTICX200_DEV_NO_IN_ONESET_IN_SN_INDEX	7

/* һ��IDռ��4bits
 *
 * ���紫��a��ƽ��ֵ���贫��5 bytes
 * (ITID_TXPA_INFO<<4 | PII_AVG_VAL) 
 * (0000 avgval_bit[15:12])  (0000 avgval_bit[11:8]) (0000 avgval_bit[7:4])  (0000 avgval_bit[3:0])
 */

/* 0 -- 15, һ������id */
enum info_type_id {
	ITID_NONE = 0,
	ITID_TXPA_INFO = 1,
	ITID_TXPB_INFO = 2,
	ITID_TXPC_INFO = 3,
	ITID_RXPA_INFO = 4,
	ITID_RXPB_INFO = 5,
	ITID_RXPC_INFO = 6,

	ITID_TXE_INFO = 7,
	ITID_RXE_INFO = 8,

	ITID_RXE_ACK  = 9,
};

/* 0 -- 15, ��������id */
enum px_info_id {
	PII_BINARY_INFO = 1, /* bit3:Ƿѹ��bit2:��ѹ��bit1:ȱ��(�ɼ���)/�����(�ָ���) */
	PII_AVG_VAL	= 2, /* ƽ��ֵ(16bits) */
};

/* 0 -- 15, ��������id */
enum trxe_info_id {
	TRXII_BINARY_INFO 		= 1, /* bit3:�ɼ��˵���/�ָ����л���PT��bit2:���ն˽ӵ��������� */
	TRXII_CPU_TEMP	  		= 2, /* CPU�¶�(16bits) */
	TRXII_BOX_TEMP   		= 3, /* ���º��¶�(16bits) */
	TRXII_VERSION	  		= 4, /* ����汾��(24bits) */
	TRXII_RX_CONTINUE_OVERLOAD_CNT	= 5, /* ���Ͳɼ���������⵽���ش���(4bits) */
	TRXII_DEV_SN	  		= 6, /* �����SN(DEV_SN_MODE_LEN bytes) */
	TRXII_USE_FIBER_NO		= 7, /* ��ǰʹ�õĹ���ͷ��, 0--��ʾ���л���pt��, 1--ʹ��1�Ź���ͷ, 2--ʹ��2�Ź���ͷ
					      * ��������� enum sys_use_channel_e
	 	 	 	 	      */
};

enum uart3_ack_format {
	TRX_SWITCH2PT = 1,/* �Ѿ��л���PT�� */
};

enum uart3_cmd_format {
	CMD_HEAD  = 0x55,/* ����ͷ */

	SWITCH2PT 	= 0x56,	/* �л���PT�� */
	SWITCH_FROM_PT	= 0x57,	/* "�л�����ѹ���������" */
};

#define creat_info_id(m_id, s_id) ((m_id)<<4 | (s_id))


enum sys_use_channel_e {
	SUC_PT_CHANNEL		= 0,
	SUC_FIBER_CHANNEL_1,
	SUC_FIBER_CHANNEL_2,

	SUC_BUTT
};

enum sys_led_channel_e {
	sys_led_CHANNEL_d1		= 0,
	sys_led_CHANNEL_d2,
	sys_led_CHANNEL_d3,
	sys_led_CHANNEL_d4,

	sys_led_CHANNEL_d5,

	sys_led_1hz	=	2,
	sys_led_2hz	=	1,

};


#define SYS_MISC_BIT_TXE_POWEROFF		0X01
#define SYS_MISC_BIT_RXE_SWITCH2PT		0X02
#define SYS_MISC_BIT_FIBRE_OPTICAL		0X04

#define is_switch2pt_sysmisc() (is_bit_set(sys_misc_info, SYS_MISC_BIT_RXE_SWITCH2PT))
#define set_switch2pt_flag_sysmisc() (set_bit(sys_misc_info, SYS_MISC_BIT_RXE_SWITCH2PT))
#define clr_switch2pt_flag_sysmisc() (clr_bit(sys_misc_info, SYS_MISC_BIT_RXE_SWITCH2PT))

#define is_fibre_recv_data_sysmisc() (is_bit_set(sys_misc_info, SYS_MISC_BIT_FIBRE_OPTICAL))
#define set_fibre_recv_data_flag_sysmisc() (set_bit(sys_misc_info, SYS_MISC_BIT_FIBRE_OPTICAL))
#define clr_fibre_recv_data_flag_sysmisc() (clr_bit(sys_misc_info, SYS_MISC_BIT_FIBRE_OPTICAL))

extern unsigned long se_version;
extern unsigned long re_version;
extern volatile unsigned long sys_misc_info;


extern void info_tran_stream_analysis(rt_device_t dev);
extern int convert_ver2str(unsigned int ver, char *str);

extern int send_cmd_to_rxe(int is_need_start_timer, enum uart3_cmd_format cmd);

extern int send_cmd2workbench(enum app_workbench_cmd_id cmd, char *cmd_str);

extern void init_sys_use_channel(void);
extern int set_sys_use_channel(enum sys_use_channel_e channel);
extern enum sys_use_channel_e get_sys_use_channel(void);
extern char convert_cur_channel_to_char(void);

#endif
