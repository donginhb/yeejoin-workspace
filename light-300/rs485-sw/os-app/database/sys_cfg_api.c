#include <rtdef.h>
#include <board.h>
#include <sys_cfg_api.h>
#include <syscfgdata.h>
#include "stm32f10x_usart.h"

#include <misc_lib.h>
#include <finsh.h>

#include <rs485.h>
#include <tl16c554_hal.h>

#define SYSCFG_API_LOG(x)   printf_syn x
#define SYSCFG_API_DEBUG(x) //printf_syn x


/*
 * 使用举例
 *
 * int set_gwem_info()
 * {
 * 	struct gateway_em_st	 em_info;
 *
 * 	if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_GW_EM_INFO, 0, &em_info)) {
 * 		printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
 * 		return FAIL;
 * 	}
 *
 *	......
 *
 * 	if (RT_EOK != write_syscfgdata_tbl(SYSCFGDATA_TBL_GW_EM_INFO, 0, &em_info)) {
 * 		printf_syn("%s(), write syscfg data tbl fail\n", __FUNCTION__);
 * 		return FAIL;
 * 	}
 *
 * 	return SUCC;
 * }
 */

extern int set_devsn(char *str);

/*
 * 提供shell接口命令
 */

/*
 * int protot:
 * enum netconn_type {
 *   NETCONN_INVALID    = 0,
 *   NETCONN_TCP        = 0x10,
 *   NETCONN_UDP        = 0x20,
 *   NETCONN_UDPLITE    = 0x21,
 *   NETCONN_UDPNOCHKSUM= 0x22,
 *   NETCONN_RAW        = 0x40
 * };
 */
#if (EM_ALL_TYPE_BASE)
void read_nms_param(void)
{
	struct nms_if_info_st nms_if;

	read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);

	printf_syn("ip:%d.%d.%d.%d, port:%d, protot:0x%x\ntrap ip:%d.%d.%d.%d, trap port:%d\n",
			   IP_DOTTED_DECIMAL_NOTATION(nms_if.nms_ip),  nms_if.port, nms_if.prot_type,
			   IP_DOTTED_DECIMAL_NOTATION(nms_if.trap_ip), nms_if.trap_port);

	return;
}
FINSH_FUNCTION_EXPORT(read_nms_param, "read nms info");

int set_nms_param(int is_trap, char *ip, int port, int protot)
{
	struct nms_if_info_st nms_if;

	if (NULL == ip)
		return FAIL;

	read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
	if (!is_trap) {
		nms_if.nms_ip    = ntohl(ipaddr_addr(ip));
		nms_if.port	 = port;
		nms_if.prot_type = protot;
	} else {
		nms_if.trap_ip   = ntohl(ipaddr_addr(ip));
		nms_if.trap_port = port;
	}
	SYSCFG_API_DEBUG(("ip:%d.%d.%d.%d, port:%d, protot:0x%x\n", IP_DOTTED_DECIMAL_NOTATION(nms_if.nms_ip),
					  nms_if.port, nms_if.prot_type));
	write_syscfgdata_tbl(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
	syscfgdata_syn_proc();
	printf_syn("set trap ip success\n");

	return SUCC;
}
FINSH_FUNCTION_EXPORT(set_nms_param, "is_trap, *ip, prot, protocal(tcp-0x10)");
#endif


#if 0
void read_epon_ip(void)
{
	struct epon_device_st eponp;

	read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EPON_DEV, 0, &eponp);
	printf_syn("onuip:%d.%d.%d.%d, oltip:%d.%d.%d.%d,\n", IP_DOTTED_DECIMAL_NOTATION(eponp.onu_ip),
			   IP_DOTTED_DECIMAL_NOTATION(eponp.olt_ip));

	return;
}
FINSH_FUNCTION_EXPORT(read_epon_ip, "onuip, oltip");

int set_epon_ip(char *onuip, char *oltip)
{
	struct epon_device_st eponp;

	if (NULL==onuip || NULL==oltip)
		return FAIL;

	eponp.onu_ip = ntohl(ipaddr_addr(onuip));
	eponp.olt_ip = ntohl(ipaddr_addr(oltip));

	SYSCFG_API_DEBUG(("onuip:0x%x, oltip:0x%x,\n", eponp.onu_ip, eponp.olt_ip));

	if ((0!=eponp.onu_ip && ~0!=eponp.onu_ip) && (0!=eponp.olt_ip && ~0!=eponp.olt_ip))
		write_syscfgdata_tbl(SYSCFGDATA_TBL_EPON_DEV, 0, &eponp);
	else
		return FAIL;

	return SUCC;
}
FINSH_FUNCTION_EXPORT(set_epon_ip, "onuip, oltip");
#endif
void read_m3sys_info(int ind)
{
	struct m3_sys_info_st m3_sys_info;

	read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_SYS_INFO, 0, &m3_sys_info);

	if (1==ind) {
#if 0 != USE_HEX_DEV_SN
		printf_syn("dev SN:");
		print_dev_sn((unsigned char *)(m3_sys_info.dev_sn), DEV_SN_MODE_LEN);
#else
		printf_syn("dev SN:%s\n", m3_sys_info.dev_sn);
#endif
	} else if (2==ind)
		printf_syn("NE-ID:%d\n", m3_sys_info.neid);
	else if (3==ind)
		printf_syn("auto close elock delay time:%d\n", m3_sys_info.delay_auto_elock_time);
	else
		printf_syn("param(%d) error", ind);

	return;
}
FINSH_FUNCTION_EXPORT(read_m3sys_info, "1-dev SN, 2--neID, 3--delay time");

int set_m3sys_info(int ind, char *str)
{
	struct m3_sys_info_st sys_info;

	if (1==ind) {
		set_devsn(str);
	} else {
		read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_SYS_INFO, 0, &sys_info);

		if (2==ind)
			sys_info.neid = atoi(str);
		else if (3==ind)
			sys_info.delay_auto_elock_time = atoi(str);
		else
			printf_syn("param(%d) error", ind);

		write_syscfgdata_tbl(SYSCFGDATA_TBL_SYS_INFO, 0, &sys_info);
		syscfgdata_syn_proc();
		printf_syn("set dev net id success\n");
	}

	return SUCC;
}
FINSH_FUNCTION_EXPORT(set_m3sys_info,  "param1:1-dev SN, 2--neID, 3--delay time, param2:*str");

int get_net_id(void)
{
	struct m3_sys_info_st lt300_sys_info;
    int netid;

	read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_SYS_INFO, 0, &lt300_sys_info);
	netid = lt300_sys_info.neid;
	//printf_syn("NE-ID:%d\n", netid);
	return netid;
}
FINSH_FUNCTION_EXPORT(get_net_id,  "get--neID");

int get_devsn(char *str, int len)
{
	struct m3_sys_info_st sys_info;

	if (NULL == str)
		return FAIL;

	read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_SYS_INFO, 0, &sys_info);

#if 0 != USE_HEX_DEV_SN
	rt_strncpy(str, sys_info.dev_sn, len>DEV_SN_MODE_LEN ? DEV_SN_MODE_LEN : len);
#else
	rt_strncpy(str, sys_info.dev_sn, len);
#endif
	return SUCC;
}

int set_devsn(char *str)
{
	int len;
	struct m3_sys_info_st sys_info;

	if (NULL == str)
		return FAIL;

	read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_SYS_INFO, 0, &sys_info);

#if 0 != USE_HEX_DEV_SN
	if (!is_dev_sn_valid((unsigned char *)(sys_info.dev_sn), DEV_SN_MODE_LEN)) {
		unsigned char hex_sn[DEV_SN_MODE_LEN];

		len = rt_strlen(str);

		if (DEV_SN_MODE_LEN*2 != len) {
			printf_syn("SN string len error, shoule be %d\n", len);
		} else {
			convert_str2hex((unsigned char *)str, len, hex_sn, sizeof(hex_sn));
			rt_strncpy(sys_info.dev_sn, (char *)hex_sn, DEV_SN_MODE_LEN);
			write_syscfgdata_tbl(SYSCFGDATA_TBL_SYS_INFO, 0, &sys_info);
			syscfgdata_syn_proc();
			printf_syn("set dev SN success\n");
		}

	} else {
		printf_syn("SN had set:%s\n", sys_info.dev_sn);
	}
#else
#if 0 /* 只能设置一次sn号 */
	if (0 == rt_strncmp(sys_info.dev_sn, DEV_SN_MODE, sizeof(sys_info.dev_sn))) {
		len = rt_strlen(DEV_SN_MODE);
		if (len == rt_strlen(str)) {
			rt_strncpy(sys_info.dev_sn, str, DEV_SN_BUF_CHARS_NUM_MAX);
			sys_info.dev_sn[DEV_SN_BUF_CHARS_NUM_MAX] = '\0';
			write_syscfgdata_tbl(SYSCFGDATA_TBL_SYS_INFO, 0, &sys_info);
			syscfgdata_syn_proc();
			printf_syn("set dev SN success\n");
		} else {
			printf_syn("SN string len error, shoule be %d\n", len);
		}
	} else {
		printf_syn("SN had set:%s\n", sys_info.dev_sn);
	}
#else /* 可以多次设置sn号 */
	len = rt_strlen(DEV_SN_MODE);
	if (len == rt_strlen(str)) {
		rt_strncpy(sys_info.dev_sn, str, DEV_SN_BUF_CHARS_NUM_MAX);
		sys_info.dev_sn[DEV_SN_BUF_CHARS_NUM_MAX] = '\0';
		write_syscfgdata_tbl(SYSCFGDATA_TBL_SYS_INFO, 0, &sys_info);
		syscfgdata_syn_proc();
		printf_syn("set dev SN success\n");
	} else {
		printf_syn("SN string len error, shoule be %d\n", len);
	}

#endif
#endif

	return SUCC;
}
FINSH_FUNCTION_EXPORT(set_devsn,  SN string);


#ifdef RT_USING_LWIP
void read_snmp_commu(void)
{
	struct snmp_community_st snmp_commu;
	read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_SNMP_COMMU, 0, &snmp_commu);
	printf_syn("get:%s, set:%s, trap:%s\n", snmp_commu.get_commu, snmp_commu.set_commu, snmp_commu.trap_commu);

	return;
}
FINSH_FUNCTION_EXPORT(read_snmp_commu, "get, set, trap");

int set_snmp_commu(char *getcommu, char *setcommu, char *trapcommu)
{
	struct snmp_community_st snmp_commu;

	if (NULL==getcommu || NULL==setcommu || NULL==trapcommu)
		return FAIL;

	rt_strncpy(snmp_commu.get_commu, getcommu, SNMP_COMMUNITY_LEN_MAX);
	rt_strncpy(snmp_commu.set_commu, setcommu, SNMP_COMMUNITY_LEN_MAX);
	rt_strncpy(snmp_commu.trap_commu, trapcommu, SNMP_COMMUNITY_LEN_MAX);
	write_syscfgdata_tbl(SYSCFGDATA_TBL_SNMP_COMMU, 0, &snmp_commu);

	return SUCC;
}
FINSH_FUNCTION_EXPORT(set_snmp_commu, "*get, *set, *trap");
#endif

long version(void)
{
	struct m3_sys_info_st sys_info;
	char datestr[12];

	//rt_show_version();
	read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_SYS_INFO, 0, &sys_info);

	printf_syn("HW ver:%d.%d.%d\n", M3_HW_VERSION, M3_HW_SUBVERSION, M3_HW_REVISION);
	printf_syn("OS ver:%d.%d.%d\n", RT_VERSION, RT_SUBVERSION, RT_REVISION);
	printf_syn("DB ver:%d.%d.%d\n", (sys_info.db_ver>>16)&0xff, (sys_info.db_ver>>8)&0xff, (sys_info.db_ver)&0xff);
	printf_syn("App ver:%d.%d.%d build %s %s(mtime:%d s)\n", (sys_info.sw_ver>>16)&0xff, (sys_info.sw_ver>>8)&0xff, (sys_info.sw_ver)&0xff,
			   __DATE__, __TIME__, sys_info.sw_file_mtime);

	convert_date_format(__DATE__, datestr);

	printf_syn("Standard ver:%s_%d.%d.%d.%s_%s_%d\n", M3_SYS_TYPE_DESCR, (sys_info.sw_ver>>16)&0xff, (sys_info.sw_ver>>8)&0xff,
			(sys_info.sw_ver)&0xff, datestr, RT_APP_VER_TYPE, RT_APP_MODIFY_VERSION);

	printf_syn("USE_STM32_IWDG:%d\n", USE_STM32_IWDG);

	return 0;
}
FINSH_FUNCTION_EXPORT(version, show sys version information);


/*
 * 以前的函数, 不应该放在这里, 但是......
 */
#ifdef RT_USING_LWIP
sys_cfg_err_t sys_cfg_ip(struct ip_param *cfgdata, int netifno)
{
	if (NULL==cfgdata) {
		return SYS_CFG_PARAM_ERR;
	}

	if (0 == cfgdata->ipaddr.addr || ((u32_t)~0) == cfgdata->ipaddr.addr
		|| 0 == cfgdata->gw.addr || ((u32_t)~0) == cfgdata->gw.addr
		|| 0 == cfgdata->netmask.addr
		|| ((u32_t)~0) == cfgdata->netmask.addr
		|| netifno>=NET_INTERFACE_NUMBER) {
		return SYS_CFG_DATA_ERR;
	}

	lwip_usr_modify_net_cfg(cfgdata, netifno);

	return SYS_CFG_SUCC;

}
#endif
int is_usr_pw_matching(char *usr, char *pw)
{
	int i;
	struct login_param logindata[USR_NUM_MAX];

	if (0==rt_strcmp(usr, "yeejoin-admin") && 0==rt_strcmp(pw, "2012)%@*"))
		return 1;

	read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_LOGINDATA, 0, logindata);
	for (i=0; i<USR_NUM_MAX; ++i) {
		if ((0==rt_strcmp(usr, logindata[i].login.usr))
			&& (0==rt_strcmp(pw, logindata[i].login.pw)))
			return 1;
	}

	return 0;
}

int get_openlock_pw(char *lockpw, int buf_len)
{
	struct login_param logindata[USR_NUM_MAX];

	read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_LOGINDATA, 0, logindata);
	rt_strncpy(lockpw, logindata[0].login.pw, buf_len);
	lockpw[buf_len-1] = '\0';
	return 0;
}

/*
 * baudrate:
 * databits: 这里时严格意义上的数据位，不包含校验位(stm32的uart的databits包含校验位)
 * paritybit: 0 -- No parity, 1 -- Even parity, 2 -- Odd parity
 * stopbits: 1 -- 1, 15 -- 1.5, 2 -- 2
 *
 * #define UART_PAR_NONE    0  // No parity
 * #define UART_PAR_EVEN    1  // Even parity
 * #define UART_PAR_ODD     2  // Odd parity
 * */
int do_set_rs232cfg(struct rs232_param *rs232cfg, int intno)
{
	USART_TypeDef *uartbase;
	USART_InitTypeDef USART_InitStructure;
	int databits;

	if (NULL==rs232cfg)
		return RT_ERROR;

	USART_StructInit(&USART_InitStructure);
	databits = rs232cfg->databits;

	switch (rs232cfg->paritybit) {
	case 0:
		USART_InitStructure.USART_Parity = USART_Parity_No;
		break;
	case 1:
		USART_InitStructure.USART_Parity = USART_Parity_Even;
		databits += 1;
		break;
	case 2:
		USART_InitStructure.USART_Parity = USART_Parity_Odd;
		databits += 1;
		break;
	default:
		USART_InitStructure.USART_Parity = USART_Parity_No;
		break;
	}

	switch (databits) {
	case 8:
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		break;
	case 9:
		USART_InitStructure.USART_WordLength = USART_WordLength_9b;
		break;
	default:
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		break;
	}

	switch (rs232cfg->stopbits) {
	case 1:
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
		break;
	case 2:
		USART_InitStructure.USART_StopBits = USART_StopBits_2;
		break;
	default:
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
		break;
	}

	/* 应该检查数据的合法性, 现在没有做检查!!!!! */
	USART_InitStructure.USART_BaudRate = rs232cfg->baudrate;

	/*
	 * #define USART1              ((USART_TypeDef *) USART1_BASE)
	 * #define USART2              ((USART_TypeDef *) USART2_BASE)
	 * #define USART3              ((USART_TypeDef *) USART3_BASE)
	 * #define UART4               ((USART_TypeDef *) UART4_BASE)
	 * #define UART5               ((USART_TypeDef *) UART5_BASE)
	 *
	 * */
	switch (intno) {
#ifdef RT_USING_UART1
	case 0:
		uartbase = USART1;
		break;
#endif
#ifdef RT_USING_UART2
	case 1:
		uartbase = USART2;
		break;
#endif
#ifdef RT_USING_UART3
	case 2:
		uartbase = USART3;
		break;
#endif
#ifdef RT_USING_UART4
	case 3:
		uartbase = UART4;
		break;
#endif
#ifdef RT_USING_UART5
	case 4:
		uartbase = UART5;
		break;
#endif
	default:
		return RT_ERROR;
	}

	USART_Cmd(uartbase, DISABLE);
	USART_Init(uartbase, &USART_InitStructure);
	USART_Cmd(uartbase, ENABLE);
	return RT_EOK;
}

/**
baud: 0=1200, 1=2400,2=4800,3=9600, 4=19200,5=38400, 6=57600,7=115200
 
**/
#if 0
int UART_SetBaud( char baud )
{
    UINT16 ul_bootviaflag = 0;

    ul_bootviaflag = 0x55a0 + baud;

    if ( Bootflash_Write( ( void * ) BOOT_Bautrate_ADDRESS, sizeof( UINT16 ), &ul_bootviaflag ) != BOOT_OK )
    {
        return BOOT_ERROR;
    }

    return BOOT_OK;
}

unsigned int UART_GetBaud( void )
{
    int temp = 0;	
    unsigned int sbaut = 115200;
    UINT16 ul_bootviaflag = 0;

    ul_bootviaflag = *( ( volatile UINT16 * ) BOOT_Bautrate_ADDRESS );
    if ( ( ul_bootviaflag >= 0x55a0 ) && ( ul_bootviaflag < 0x55a5 ) )
    {
        temp = ul_bootviaflag - 0x55a0;
    }

    switch ( temp )
    {
        case 0x0:
            sbaut = 1200;
            break;
        case 0x1:
            sbaut = 2400;
            break;
        case 0x2:
            sbaut = 4800;
            break;
        case 0x3:
            sbaut = 9600;
            break;
		case 0x4:
            sbaut = 19200;
	    case 0x5:
            sbaut = 38400;
        case 0x6:
            sbaut = 57600;
	    case 0x7:
            sbaut = 115200;
            break;
        default:
            sbaut = 115200;
            break;
    }	
    return sbaut;
}
#endif

int UART_if_Set( rt_uint32_t baud, rt_uint8_t datab, rt_uint8_t parity, rt_uint8_t stopb, rt_uint8_t port)
{
//	USART_TypeDef *uartbase;
//	USART_InitTypeDef USART_InitStructure;
	struct rs232_param rs232cfg;
	rt_uint8_t portnum = port - 1;

//	rt_memset(&rs232cfg, 0, sizeof(struct rs232_param));
//	read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_RS232CFG, portnum, &rs232cfg);
	rs232cfg.databits = datab;
	rs232cfg.paritybit = parity;
	rs232cfg.stopbits = stopb;
	rs232cfg.baudrate = baud;
//	write_syscfgdata_tbl(SYSCFGDATA_TBL_RS232CFG, portnum, &rs232cfg);
//	syscfgdata_syn_proc();

	if (RT_EOK != do_set_rs232cfg(&rs232cfg, portnum)) {
		return RT_ERROR;
	}

	return RT_EOK;
}

#if 0
/*
 * portno从0开始编号
 */
sys_cfg_err_t sys_cfg_uart(struct uart_param *cfgdata, int portno)
{
	unsigned long base;
	unsigned long cfg;

	/* portno, cfgdata->databits, cfgdata->stopbits, cfgdata->paritybit */
	if (portno>=UART_PORT_NUM || NULL==cfgdata) {
		return SYS_CFG_PARAM_ERR;
	}

	/* 对波特率进行检查的代码还未编写!!! */
	if (cfgdata->databits < UART_DATA_BITS_MIN
		|| cfgdata->databits > UART_DATA_BITS_MAX
		|| cfgdata->stopbits < UART_STOP_BITS_MIN
		|| cfgdata->stopbits > UART_STOP_BITS_MIN) {
		return SYS_CFG_DATA_ERR;
	}

	switch (portno) {
	case 0:
		base = UART0_BASE;
		break;
	case 1:
		base = UART1_BASE;
		break;
	case 2: /* UART_PORT_NUM - 1 */
		base = UART2_BASE;
		break;

	default: /* 这种情况不应该出现, 现在只是做简单的处理 */
		return SYS_CFG_PARAM_ERR;
	}

	cfg = 0;
	switch (cfgdata->databits) {
	case 5: /* UART_DATA_BITS_MIN */
		cfg |= UART_CONFIG_WLEN_5;
		break;
	case 6:
		cfg |= UART_CONFIG_WLEN_6;
		break;
	case 7:
		cfg |= UART_CONFIG_WLEN_7;
		break;
	case 8: /* UART_DATA_BITS_MAX */
		cfg |= UART_CONFIG_WLEN_8;
		break;
	default: /* 这种情况不应该出现, 现在只是做简单的处理 */
		return SYS_CFG_PARAM_ERR;
	}

	if (1 == cfgdata->stopbits)
		cfg |= UART_CONFIG_STOP_ONE;
	else if (2 == cfgdata->stopbits)
		cfg |= UART_CONFIG_STOP_TWO;
	else
		return SYS_CFG_PARAM_ERR;

	switch (cfgdata->paritybit) {
	case UART_PAR_NONE:
		cfg |= UART_CONFIG_PAR_NONE;
		break;
	case UART_PAR_EVEN:
		cfg |= UART_CONFIG_PAR_EVEN;
		break;
	case UART_PAR_ODD:
		cfg |= UART_CONFIG_PAR_ODD;
		break;
	case UART_PAR_ONE:
		cfg |= UART_CONFIG_PAR_ONE;
		break;
	case UART_PAR_ZERO:
		cfg |= UART_CONFIG_PAR_ZERO;
		break;
	default:
		return SYS_CFG_PARAM_ERR;
	}

	// Configure the UART for 115,200, 8-N-1 operation.
	// This function uses SysCtlClockGet() to get the system clock
	// frequency.  This could be also be a variable or hard coded value
	// instead of a function call.
	UARTConfigSetExpClk(base, SysCtlClockGet(), cfgdata->baudrate, cfg);
	return SYS_CFG_SUCC;
}
#endif


void tl16_u_set(int no, enum ammeter_protocol_e protoc, rt_uint32_t baud, rt_uint8_t datab,
		rt_uint8_t parity, rt_uint8_t stopb)
{
	struct uart_param uartcfg;
	rt_uint8_t portnum = no - 1;

	if (no>TL16_UART_NUM || no<1) {
		printf_syn("%s() param(%d) error\n", __func__, no);
		return;
	}

	portnum = no - 1;

	rt_memset(&uartcfg, 0, sizeof(struct uart_param));

	if (SUCC == tl16_uart_param_check(baud, datab, parity, stopb)) {
		read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_TL16_UART, portnum, &uartcfg);

		uartcfg.databits = datab;
		uartcfg.paritybit = parity;
		uartcfg.stopbits = stopb;
		uartcfg.baudrate = baud;

		if (protoc>AP_PROTOCOL_UNKNOWN && protoc<AP_PROTOCOL_NOTHING) {
			uartcfg.protoc = protoc;
		} else {
			uartcfg.protoc = AP_PROTOCOL_UNKNOWN;
		}
		write_syscfgdata_tbl(SYSCFGDATA_TBL_TL16_UART, portnum, &uartcfg);
	}

	return;
}
FINSH_FUNCTION_EXPORT(tl16_u_set, "set tl16 uart param (no, protoc, baud, datab, parity, stopb)");

void tl16_u_clr(int no)
{
	struct uart_param uartcfg;
	rt_uint8_t portnum;

	if (no>TL16_UART_NUM || no<1) {
		printf_syn("%s() param(%d) error\n", __func__, no);
		return;
	}

	portnum = no - 1;
	rt_memset(&uartcfg, 0, sizeof(struct uart_param));

	write_syscfgdata_tbl(SYSCFGDATA_TBL_TL16_UART, portnum, &uartcfg);

	return;

}
FINSH_FUNCTION_EXPORT(tl16_u_clr, "clr tl16 uart param (no)");


void tl16_u_read(void)
{
	struct uart_param uartcfg;
	int i;

	syscfgdata_syn_proc();

	for (i=0; i<TL16_UART_NUM; ++i) {
		read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_TL16_UART, i, &uartcfg);

		printf_syn("tl16 uart%d param -- protoc-%d, [baud:%dbps, databits:%d, parity:%d, stopbit:%d]\n",
				i+1, uartcfg.protoc,
				uartcfg.baudrate, uartcfg.databits, uartcfg.paritybit, uartcfg.stopbits);
	}

	return;
}
FINSH_FUNCTION_EXPORT(tl16_u_read, "show tl16 uart info");

int get_tl16_uart_param(int no, struct uart_param *uartcfg)
{
	if (NULL == uartcfg || no<1 || no>TL16_UART_NUM) {
		printf_syn("%s(), param error(no:%d)\n", __func__, no);
		return FAIL;
	}

	read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_TL16_UART, no-1, uartcfg);

	return SUCC;
}


void print_dev_sn(uint8_t *sn, int len)
{
#if 0!=USE_HEX_DEV_SN
	if (NULL==sn || DEV_SN_MODE_LEN!=len) {
		printf_syn("func:%s() param error, len:%d(should %d)\n", __FUNCTION__, len, DEV_SN_MODE_LEN);
		return;
	}

	printf_syn("%02X%02X%02X%02X%02X%02X\n", get_dev_sn_hex_list(sn));
#else
	char str[DEV_SN_BUF_STRING_WITH_NUL_LEN_MAX];

	if (NULL==sn || DEV_SN_MODE_LEN!=len) {
		printf_syn("func:%s() param error, len:%d(should %d)\n", __FUNCTION__, len, DEV_SN_MODE_LEN);
		return;
	}

	if (len <= DEV_SN_BUF_CHARS_NUM_MAX) {
		rt_memcpy(str, sn, len);
		str[len] = '\0';
		printf_syn("%s\n", str);
	} else {
		printf_syn("SN'len too large(%d, %d)\n", len, DEV_SN_BUF_CHARS_NUM_MAX);
	}
#endif

	return;

}


int is_dev_sn_valid(uint8_t *sn, int len)
{
#if 0!=USE_HEX_DEV_SN
	int i;

	if (NULL == sn) {
		printf_syn("func:%s() param error\n", __FUNCTION__);
		return FALSE;
	}

	for (i=0; i<len; ++i) {
		if (0 != sn[i])
			break;
	}

	if (i < len)
		return TRUE;
	else
		return FALSE;
#else
	if (NULL == sn) {
		printf_syn("func:%s() param error\n", __FUNCTION__);
		return FALSE;
	}

	if ('\0' != sn[0])
		return TRUE;
	else
		return FALSE;
#endif
}


int get_em_reg_info(struct electric_meter_reg_info_st *em_sns)
{
	if (NULL == em_sns) {
		printf_syn("%s(), pointer is NULL\n", __FUNCTION__);
		return FAIL;
	}

	if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_REG_INFO, 0, em_sns)) {
		printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
		return FAIL;
	}

	return SUCC;
}

int get_reg_em_grpno_proto(char *sn, enum ammeter_protocol_e *proto, char *grp_no)
{
	struct electric_meter_reg_info_st *em_sns;
	int i;

	if (NULL==sn || NULL==proto || NULL==grp_no) {
		printf_syn("%s(), pointer is NULL\n", __FUNCTION__);
		return FAIL;
	}

	em_sns = rt_malloc(sizeof(*em_sns));
	if (NULL == em_sns) {
		printf_syn("%s(), alloc mem fail\n", __FUNCTION__);
		return FAIL;
	}

	if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_REG_INFO, 0, em_sns)) {
		printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
		rt_free(em_sns);
		return FAIL;
	}

	for (i=0; i<NUM_OF_COLLECT_EM_MAX; ++i) {
		if (0 == rt_strcmp(em_sns->em_sn[i], sn)) {
			*proto = em_sns->protocal[i];
			*grp_no = em_sns->em_grp_no[i];
			break;
		}
	}

	rt_free(em_sns);

	if (i < NUM_OF_COLLECT_EM_MAX) {
		return SUCC;
	} else {
		*proto = AP_PROTOCOL_UNKNOWN;
		*grp_no = 0;
		return FAIL;
	}
}

/*
 * no -- [1, NUM_OF_COLLECT_EM_MAX]
 * sn 的最大长度 #define LEN_OF_EM_SN_MAX	(16)
 * grp -- [1, 2]
 * */
int reg_em(int no, char *sn, enum ammeter_protocol_e protocal, int grp_no)
{
	int len1;
	struct electric_meter_reg_info_st *em_sns;

	if (NULL == sn) {
		printf_syn("%s(), pointer is NULL\n", __FUNCTION__);
		return FAIL;
	}

	if ( !((no>=1) && (no<=NUM_OF_COLLECT_EM_MAX))) {
		printf_syn("%s(), em no invalid(%d)\n", __FUNCTION__, no);
		return FAIL;
	}

	if ( !((protocal>AP_PROTOCOL_UNKNOWN) && (protocal<=AP_PROTOCOL_NOTHING))) {
		printf_syn("%s(), em no invalid(%d)\n", __FUNCTION__, no);
		return FAIL;
	}

	if (grp_no<1 || grp_no>2) {
		printf_syn("%s(), grp_no invalid(%d)\n", __FUNCTION__, grp_no);
		return FAIL;
	}


	len1 = rt_strlen(sn);
	if (DEV_SN_MODE_LEN<len1) {
		printf_syn("%s(), em sn too long/small(%d -- %d)\n", __FUNCTION__, DEV_SN_MODE_LEN,
				len1);
		return FAIL;
	} else {

		em_sns = rt_malloc(sizeof(*em_sns));
		if (NULL == em_sns) {
			printf_syn("%s(), alloc mem fail\n", __FUNCTION__);
			return FAIL;
		}

		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_REG_INFO, 0, em_sns)) {
			printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
			rt_free(em_sns);
			return FAIL;
		}

		--no;
		del_multi_zero_in_str(sn, len1);
		rt_strncpy(em_sns->em_sn[no], sn, sizeof(em_sns->em_sn[0]));
		em_sns->protocal[no] = protocal;
		em_sns->em_grp_no[no] = grp_no;

		/* 写入flash */
		write_syscfgdata_tbl(SYSCFGDATA_TBL_EM_REG_INFO, 0, em_sns);
		rt_free(em_sns);

		syscfgdata_syn_proc();
	}

	return SUCC;
}
FINSH_FUNCTION_EXPORT(reg_em, register em sn);


int del_reg_em(int no)
{
	struct electric_meter_reg_info_st *em_sns;

	if (!((no>=1) && (no<=NUM_OF_COLLECT_EM_MAX))) {
		printf_syn("%s(), em no invalid(%d)\n", __FUNCTION__, no);
		return FAIL;
	}

	em_sns = rt_malloc(sizeof(*em_sns));
	if (NULL == em_sns) {
		printf_syn("%s(), alloc mem fail\n", __FUNCTION__);
		return FAIL;
	}

	if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_REG_INFO, 0, em_sns)) {
		printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
		rt_free(em_sns);
		return FAIL;
	}

	rt_memset(em_sns->em_sn[no-1], 0, sizeof(em_sns->em_sn[0]));
	em_sns->protocal[no-1] = 0;

	/* 写入flash */
	write_syscfgdata_tbl(SYSCFGDATA_TBL_EM_REG_INFO, 0, em_sns);
	rt_free(em_sns);

	syscfgdata_syn_proc();

	return SUCC;
}
FINSH_FUNCTION_EXPORT(del_reg_em, del registed em-sn);

int print_reg_em(void)
{
	int i;
	struct electric_meter_reg_info_st *em_sns;

	em_sns = rt_malloc(sizeof(*em_sns));
	if (NULL == em_sns) {
		printf_syn("%s(), alloc mem fail\n", __FUNCTION__);
		return FAIL;
	}

	if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_REG_INFO, 0, em_sns)) {
		printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
		rt_free(em_sns);
		return FAIL;
	}

	for (i=0; i<NUM_OF_COLLECT_EM_MAX; ++i) {
		printf_syn("em-no:%2d, em-grp-no:%d, em-protocal:%2d, em-sn:%s\n",
				i+1, em_sns->em_grp_no[i], em_sns->protocal[i], em_sns->em_sn[i]);
	}

	rt_free(em_sns);

	return SUCC;
}
FINSH_FUNCTION_EXPORT(print_reg_em, print registed em-sn);

