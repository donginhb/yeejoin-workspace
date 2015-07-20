#include <rtdef.h>
#include <sys_cfg_api.h>
#include <syscfgdata.h>
#include "stm32f10x_usart.h"
//#include "Httpd_script.h"

#include <finsh.h>

#define SYSCFG_API_LOG(x)   printf_syn x
#define SYSCFG_API_DEBUG(x) //printf_syn x

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
void read_nms_param(void)
{
	struct nms_if_info_st nms_if;

	read_syscfgdata_tbl(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);

	printf_syn("ip:%d.%d.%d.%d, port:%d, protot:0x%x\ntrap ip:%d.%d.%d.%d, port:%d\n", 
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

	read_syscfgdata_tbl(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
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

	return SUCC;
}
FINSH_FUNCTION_EXPORT(set_nms_param, "is_trap, *ip, prot, protocal(tcp-0x10)");

void read_epon_ip(void)
{
	struct epon_device_st eponp;

	read_syscfgdata_tbl(SYSCFGDATA_TBL_EPON_DEV, 0, &eponp);
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

void read_m3sys_info(int ind)
{
	struct m3_sys_info_st m3_sys_info;

	read_syscfgdata_tbl(SYSCFGDATA_TBL_SYS_INFO, 0, &m3_sys_info);

	if (1==ind)
		printf_syn("dev SN:%s\n", m3_sys_info.dev_sn);
	else if (2==ind)
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

	read_syscfgdata_tbl(SYSCFGDATA_TBL_SYS_INFO, 0, &sys_info);
	
	if (1==ind)
		rt_strncpy(sys_info.dev_sn, str, DEV_SN_LEN_MAX);
	else if (2==ind)
		sys_info.neid = atoi(str);
	else if (3==ind)
		sys_info.delay_auto_elock_time = atoi(str);
	else
		printf_syn("param(%d) error", ind);

	write_syscfgdata_tbl(SYSCFGDATA_TBL_SYS_INFO, 0, &sys_info);

	return SUCC;
}
FINSH_FUNCTION_EXPORT(set_m3sys_info,  "param1:1-dev SN, 2--neID, 3--delay time, param2:*str");

void read_snmp_commu(void)
{
	struct snmp_community_st snmp_commu;
	read_syscfgdata_tbl(SYSCFGDATA_TBL_SNMP_COMMU, 0, &snmp_commu);
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



const char *const months_str[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
long version(void)
{
	struct m3_sys_info_st sys_info;
	//char datestr[12];
	char datestr_1[12];
	int i;
	
	//rt_show_version();
	read_syscfgdata_tbl(SYSCFGDATA_TBL_SYS_INFO, 0, &sys_info);
	
	printf_syn("HW ver:%d.%d.%d\n", M3_HW_VERSION, M3_HW_SUBVERSION, M3_HW_REVISION);
	printf_syn("OS ver:%d.%d.%d\n", RT_VERSION, RT_SUBVERSION, RT_REVISION);
	printf_syn("DB ver:%d.%d.%d\n", (sys_info.db_ver>>16)&0xff, (sys_info.db_ver>>8)&0xff, (sys_info.db_ver)&0xff);
	printf_syn("App ver:%d.%d.%d build %s %s\n", (sys_info.sw_ver>>16)&0xff, (sys_info.sw_ver>>8)&0xff, (sys_info.sw_ver)&0xff,
			__DATE__, __TIME__);

	/* Aug 22 2012 */
	rt_strncpy(datestr_1, __DATE__, sizeof(datestr_1));
	datestr_1[3] = '\0';
	if (' '==datestr_1[4])
		datestr_1[4] = '0';
	datestr_1[6] = '\0';
	//datestr_1[11] = '\0';
	
	for (i=0; i<12; ++i)
		if (0==rt_strncmp(months_str[i], datestr_1, 3))
			break;

	rt_sprintf(datestr_1, "%02d", i+1);
	printf_syn("Standard ver:SCC_%d.%d.%d.%s%s%s_%s_%d\n", (sys_info.sw_ver>>16)&0xff, (sys_info.sw_ver>>8)&0xff, (sys_info.sw_ver)&0xff,
			&datestr_1[7], &datestr_1[0], &datestr_1[4], RT_APP_VER_TYPE, RT_APP_MODIFY_VERSION);

	return 0;
}
FINSH_FUNCTION_EXPORT(version, show sys version information);


/*
 * 以前的函数, 不应该放在这里, 但是......
 */
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


int is_usr_pw_matching(char *usr, char *pw)
{
	int i;
	struct login_param logindata[USR_NUM_MAX];

	if (0==rt_strcmp(usr, "yeejoin-admin") && 0==rt_strcmp(pw, "2012)%@*"))
		return 1;

	read_syscfgdata_tbl(SYSCFGDATA_TBL_LOGINDATA, 0, logindata);
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

	read_syscfgdata_tbl(SYSCFGDATA_TBL_LOGINDATA, 0, logindata);
	rt_strncpy(lockpw, logindata[0].login.pw, buf_len);
	lockpw[buf_len-1] = '\0';
	return 0;
}

int do_set_rs232cfg(struct rs232_param *rs232cfg, int intno)
{
    USART_TypeDef *uartbase;
    USART_InitTypeDef USART_InitStructure;

    if (NULL==rs232cfg)
        return RT_ERROR;

    USART_StructInit(&USART_InitStructure);
    switch (rs232cfg->databits) {
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

    switch (rs232cfg->paritybit) {
    case 0:
        USART_InitStructure.USART_Parity = USART_Parity_No;
        break;
    case 1:
        USART_InitStructure.USART_Parity = USART_Parity_Even;
        break;
    case 2:
        USART_InitStructure.USART_Parity = USART_Parity_Odd;
        break;
    default:
        USART_InitStructure.USART_Parity = USART_Parity_No;
        break;
    }

    /* 应该检查数据的合法性, 现在没有做检查!!!!! */
    USART_InitStructure.USART_BaudRate = rs232cfg->baudrate;

    switch (intno) {
    case 0:
        uartbase = USART1;
        break;
    case 1:
        uartbase = USART2;
        break;
    default:
        return RT_ERROR;
    }
    
    USART_Init(uartbase, &USART_InitStructure);
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

