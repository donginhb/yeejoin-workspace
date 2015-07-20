/*
 * 2012-01-13 16:15:45
 */
#ifndef _SYSCFGDATA_H_
#define _SYSCFGDATA_H_

#include <rtdef.h>

#include <lwipusrlib.h>
#include <lwip/ip_addr.h>
#include <lwip/api.h>
#include <lwip/snmp_msg.h>

struct rs232_param{
    rt_uint32_t baudrate;
    rt_uint8_t  databits;
    rt_uint8_t  paritybit;
    rt_uint8_t  stopbits;
};


#define USR_NAME_LEN_MAX    31
#define PW_LEN_MAX          31
struct usr_pw_pair{
    char usr[USR_NAME_LEN_MAX+1];
    char pw[PW_LEN_MAX+1];
};

#define USR_NUM_MAX 1
struct login_param{
    struct usr_pw_pair login;
};


/* data had modify */
#define SYSTBL_FLAG_DATA_DIRTY		0x01
/* write data to flash immediately */
#define SYSTBL_FLAG_WRITE_THROUGH 	0x02

struct tab_head {
	rt_uint32_t magic_num; /* NOTE: must place at the first word of table */
};

#define DEV_SN_LEN_MAX 15
struct m3_sys_info_st {
	/* rt_uint32_t hw_ver; */	/* ����I/O�ڲ��뿪��ȷ��Ӳ���汾�� */
	rt_uint32_t sw_ver;		/* ����Զ�������汾�ŵ��ж� */
	rt_uint32_t db_ver;		/* ���ݿ�汾 */
	char dev_sn[DEV_SN_LEN_MAX+1];	/* �豸���к� */
	rt_uint32_t neid;		/* ��ԪID */
	rt_uint16_t delay_auto_elock_time; /* �Զ��ӳٹر�elock��ʱ��, ��λ�Ƿ��� */
	rt_uint16_t m3sys_reserve;
};

/*
 * �ڲ��洢ʱ, �¶Ȱ����з������Ĳ���洢
 */
struct temp_rh_limen {
	rt_int16_t	temp_h;
	rt_int16_t	temp_l;
	rt_uint16_t	rh_h;
	rt_uint16_t	rh_l;
	rt_uint16_t	warn_mask; /* ����ȷ�����ص�led���ڸ澯ʱ�Ƿ���˸, 1-mask, 0-unmask bit0:temperature, bit1:rh */
	rt_uint16_t	tmp_rh_reserve;
};

/* ���ܽӿ������Ϣ, Network Management System */
#define NMS_TRAP_ENABLE_BIT 0x01
struct nms_if_info_st {
	rt_uint32_t nms_ip; 	/* ip address */
	u16	port;		/* listening port */
	u16	prot_type;	/* protocal type, enum netconn_type--api.h */
	rt_uint32_t trap_ip; 	/* ip address */
	u16	trap_port;	/* listening port */
	u16	nms_reserve;	/* reserve */
	rt_uint32_t trap_enable_bits;
	
};

/* epon�豸ip */
#define EPON_DEV_DETECT_EN_BIT  0X01
#define EPON_DEV_ALARM_MASK_BIT 0X02
struct epon_device_st {
	rt_uint32_t onu_ip;
	rt_uint32_t olt_ip;
	rt_uint32_t epon_bitv;
};

/* snmp�������� */
struct snmp_community_st {
	char get_commu[SNMP_COMMUNITY_LEN_MAX];		/* �������� */
	char set_commu[SNMP_COMMUNITY_LEN_MAX];		/* д������ */
	char trap_commu[SNMP_COMMUNITY_LEN_MAX];	/* trap������ */
};

/* ip address of target*/
#define RT_LWIP_IPADDR0	172
#define RT_LWIP_IPADDR1	24
#define RT_LWIP_IPADDR2	4
#define RT_LWIP_IPADDR3	219

/* gateway address of target*/
#define RT_LWIP_GWADDR0	172
#define RT_LWIP_GWADDR1	24
#define RT_LWIP_GWADDR2	4
#define RT_LWIP_GWADDR3	254

/* mask address of target*/
#define RT_LWIP_MSKADDR0	255
#define RT_LWIP_MSKADDR1	255
#define RT_LWIP_MSKADDR2	255
#define RT_LWIP_MSKADDR3	0

/**
 * Default TCP/IP Address Configuration.  Static IP Configuration is used if
 * DHCP times out.
 */
#define DEFAULT_IPADDR        ((RT_LWIP_IPADDR0 << 24) | (RT_LWIP_IPADDR1 << 16) | (RT_LWIP_IPADDR2 << 8) | RT_LWIP_IPADDR3)
#define DEFAULT_GATEWAY_ADDR  ((RT_LWIP_GWADDR0 << 24) | (RT_LWIP_GWADDR1 << 16) | (RT_LWIP_GWADDR2 << 8) | RT_LWIP_GWADDR3)
#define DEFAULT_NET_MASK      ((RT_LWIP_MSKADDR0 << 24) | (RT_LWIP_MSKADDR1 << 16) | (RT_LWIP_MSKADDR2 << 8) | RT_LWIP_MSKADDR3)

#define IP_INTERFACE_NUM 1
#define RS232_INTERFACE_NUM 2

struct syscfgdata_tbl{
	struct tab_head systbl_head;
	struct m3_sys_info_st m3_sys_info;	/* !!NOTE:�ýṹ������������ʼλ�ò��ܸı�, Զ��������Ҫʹ�� */
	struct login_param logindata[USR_NUM_MAX];
	struct ip_param ipcfg[IP_INTERFACE_NUM];
	struct rs232_param rs232cfg[RS232_INTERFACE_NUM];
	struct temp_rh_limen t_rh_value;
	struct nms_if_info_st nms_if;
	struct epon_device_st epon_dev;
	struct snmp_community_st snmp_community;
};


struct syscfgdata_tbl_cache_st {
	volatile rt_uint32_t systbl_flag_set;
	struct syscfgdata_tbl syscfg_data;
};

enum syscfgdata_subtbl_ind{
    SYSCFGDATA_TBL_SYS_INFO	= 0,
    SYSCFGDATA_TBL_LOGINDATA,
    SYSCFGDATA_TBL_IPCFG,
    SYSCFGDATA_TBL_RS232CFG,
    SYSCFGDATA_TBL_TMP_RH,
    SYSCFGDATA_TBL_NMS_IF,
    SYSCFGDATA_TBL_EPON_DEV,
    SYSCFGDATA_TBL_SNMP_COMMU,
    
    SYSCFGDATA_TBL_BUTT
};

#define RS232_DEFAULT_BAUDRATE   (115200)
#define RS232_DEFAULT_DATABITS   (8)
#define RS232_DEFAULT_PARITYBIT  (UART_PAR_NONE)
#define RS232_DEFAULT_STOPBITS   (1)

/* �¶�ʹ���з�����, ��ʵ��ֵ��10�� */
#define TMP_DEFAULT_LIMEN_H	(450)
#define TMP_DEFAULT_LIMEN_L	(30)

/* ���ʪ��ʹ���޷�����, ��ʵ��ֵ��10�� */
#define RH_DEFAULT_LIMEN_H	(800)
#define RH_DEFAULT_LIMEN_L	(100)

/*
    // OUI 00-80-E1 STMICROELECTRONICS
    stm32_eth_device.dev_addr[0] = 0x00;
    stm32_eth_device.dev_addr[1] = 0x80;
    stm32_eth_device.dev_addr[2] = 0xE1;
    // generate MAC addr from 96bit unique ID (only for test)
    stm32_eth_device.dev_addr[3] = *(rt_uint8_t*)(0x1FFFF7E8+7);
    stm32_eth_device.dev_addr[4] = *(rt_uint8_t*)(0x1FFFF7E8+8);
    stm32_eth_device.dev_addr[5] = *(rt_uint8_t*)(0x1FFFF7E8+9);

 */
#define MACOCT5 (*(rt_uint8_t*)(0x1FFFF7E8+9))
#define MACOCT4 (*(rt_uint8_t*)(0x1FFFF7E8+8))
#define MACOCT3 (*(rt_uint8_t*)(0x1FFFF7E8+7))
#define MACOCT2 (0XE1)
#define MACOCT1 (0X80)
#define MACOCT0 (0X00)


/*
 * ������ز���
 */
#define WEBM_P_SERVER_DEFAULT_IP    (192<<24 | 168<<16 | 1<<8 | 21)
#define WEBM_P_SERVER_DEFAULT_PORT  (1234)
#define WEBM_P_SERVER_DEFAULT_PROTO (NETCONN_TCP)

extern struct login_param login_param_name[USR_NUM_MAX];

extern int init_syscfgdata_tbl(void);
extern int read_syscfgdata_tbl(unsigned int ind_m, unsigned int ind_s, void *data);
extern int write_syscfgdata_tbl(const unsigned int ind_m, const unsigned int ind_s, void *const data);
extern int restore_default_syscfgdata(void);

extern void syscfgdata_syn_proc(void);

#endif
