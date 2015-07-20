#include <rtdef.h>
#include <sys_cfg_api.h>
#include <syscfgdata.h>
#include "stm32f10x_usart.h"
//#include "Httpd_script.h"

#include <misc_lib.h>

#include <finsh.h>
#include <info_tran.h>

#define SYSCFG_API_LOG(x)   printf_syn x
#define SYSCFG_API_DEBUG(x) //printf_syn x

static int creat_match_sn_from_dev_sn(char *dev_sn);

/*
 * 提供shell接口命令
 */

#if 0
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
#endif

int get_devsn(char *str, int len)
{
	struct m3_sys_info_st sys_info;

	if (NULL == str)
		return FAIL;

	read_syscfgdata_tbl(SYSCFGDATA_TBL_SYS_INFO, 0, &sys_info);

	rt_strncpy(str, sys_info.dev_sn, len);

	return SUCC;
}

extern int set_sn_s(char *str);
int set_sn(char *str)
{
#if 1
	return set_sn_s(str);
#else
	int len;
	struct m3_sys_info_st sys_info;

	if (NULL == str)
		return FAIL;

	read_syscfgdata_tbl(SYSCFGDATA_TBL_SYS_INFO, 0, &sys_info);

	if (0 == rt_strncmp(sys_info.dev_sn, DEV_SN_MODE, sizeof(sys_info.dev_sn))) {
		len = rt_strlen(DEV_SN_MODE);
		if (len == rt_strlen(str)) {
			rt_strncpy(sys_info.dev_sn, str, DEV_SN_LEN_MAX);
			sys_info.dev_sn[DEV_SN_LEN_MAX] = '\0';
			write_syscfgdata_tbl(SYSCFGDATA_TBL_SYS_INFO, 0, &sys_info);
			syscfgdata_syn_proc();
			printf_syn("set dev SN success\n");

			creat_match_sn_from_dev_sn(str);
		} else {
			printf_syn("SN string len error, shoule be %d\n", len);
		}
	} else {
		printf_syn("SN had set:%s\n", sys_info.dev_sn);
	}


	return SUCC;
#endif
}
FINSH_FUNCTION_EXPORT(set_sn,  SN string);

int set_sn_s(char *str)
{
	int len;
	struct m3_sys_info_st sys_info;

	if (NULL == str)
		return FAIL;

	read_syscfgdata_tbl(SYSCFGDATA_TBL_SYS_INFO, 0, &sys_info);

	len = rt_strlen(DEV_SN_MODE);
	if (len == rt_strlen(str)) {
		rt_strncpy(sys_info.dev_sn, str, DEV_SN_LEN_MAX);
		sys_info.dev_sn[DEV_SN_LEN_MAX] = '\0';
		write_syscfgdata_tbl(SYSCFGDATA_TBL_SYS_INFO, 0, &sys_info);
		syscfgdata_syn_proc();
		printf_syn("set dev SN success\n");

		creat_match_sn_from_dev_sn(str);
	} else {
		printf_syn("SN string len error, shoule be %d\n", len);
	}

	return SUCC;
}
FINSH_FUNCTION_EXPORT(set_sn_s,  super set SN string);

int get_sn(void)
{
	char buf[DEV_SN_MODE_LEN%4 ? ((DEV_SN_MODE_LEN/4)+1)*4 : DEV_SN_MODE_LEN+4];

	if (SUCC == get_devsn(buf, sizeof(buf))) {
		buf[DEV_SN_MODE_LEN] = '\0';
		printf_syn("%s\n", buf);
	} else {
		printf_syn("get sn fail\n");
	}

	return SUCC;
}
FINSH_FUNCTION_EXPORT(get_sn, get dev sn);

int set_match_sn(char *str, int index)
{
	int len;
	struct m3_sys_info_st sys_info;

	if (NULL == str) {
		printf_syn("func:%s, NULL pointer\n", __FUNCTION__);
		return FAIL;
	}

	if (index != 0 && index != 1) {
		printf_syn("func:%s(), index(%d) param is error.\n", __FUNCTION__, index);
		return FAIL;
	}

	rt_memset(&sys_info, 0, sizeof(sys_info));
	read_syscfgdata_tbl(SYSCFGDATA_TBL_SYS_INFO, 0, &sys_info);

	/* 不再判断匹配sn是否设置过, 直接设置匹配sn */
	len = rt_strlen(DEV_SN_MODE);
	if (len == rt_strlen(str)) {
		rt_strncpy(sys_info.match_sn[index], str, DEV_SN_LEN_MAX);
		sys_info.match_sn[index][DEV_SN_LEN_MAX] = '\0';
		write_syscfgdata_tbl(SYSCFGDATA_TBL_SYS_INFO, 0, &sys_info);
		syscfgdata_syn_proc();
		printf_syn("match dev SN %d success\n",index);
	} else {
		printf_syn("match SN string len error, shoule be %d\n", len);
	}
	
	return SUCC;
}
FINSH_FUNCTION_EXPORT(set_match_sn, set match SN string);

int get_matchsn(char *str, int len,int index)
{
	struct m3_sys_info_st sys_info;

	if (NULL == str)
		return FAIL;

	if (index != 0 && index != 1) {
		printf_syn("func:%s(), index(%d) param is error.\n", __FUNCTION__, index);
		return FAIL;
	}
	
	read_syscfgdata_tbl(SYSCFGDATA_TBL_SYS_INFO, 0, &sys_info);

	rt_strncpy(str, sys_info.match_sn[index], len);

	return SUCC;
}

int get_match_sn(void)
{
	char buf[DEV_SN_MODE_LEN%4 ? ((DEV_SN_MODE_LEN/4)+1)*4 : DEV_SN_MODE_LEN+4];

	if (SUCC == get_matchsn(buf, sizeof(buf),0)) {
		buf[DEV_SN_MODE_LEN] = '\0';
		printf_syn("match sn-1: '%s'\n", buf);
	} else {
		printf_syn("get match sn-1 fail\n");
	}

	if (SUCC == get_matchsn(buf, sizeof(buf),1)) {
		buf[DEV_SN_MODE_LEN] = '\0';
		printf_syn("match sn-2: '%s'\n", buf);
	} else {
		printf_syn("get match sn-2 fail\n");
	}

	return SUCC;
}
FINSH_FUNCTION_EXPORT(get_match_sn, get match SN string);

static int creat_match_sn_from_dev_sn(char *dev_sn)
{
	char str[DEV_SN_LEN_MAX+1];
	struct m3_sys_info_st sys_info;

	if (NULL == dev_sn) {
		printf_syn("func:%s, NULL pointer\n", __FUNCTION__);
		return FAIL;
	}

	rt_memset(&sys_info, 0, sizeof(sys_info));
	read_syscfgdata_tbl(SYSCFGDATA_TBL_SYS_INFO, 0, &sys_info);

	rt_strncpy(str, dev_sn, sizeof(str));
	str[OPTICX200_DEV_TYPE_IN_SN_INDEX] = 'T';
	str[OPTICX200_DEV_NO_IN_ONESET_IN_SN_INDEX] = '1';
	rt_strncpy(sys_info.match_sn[0], str, DEV_SN_LEN_MAX);
	sys_info.match_sn[0][DEV_SN_LEN_MAX] = '\0';

	str[OPTICX200_DEV_NO_IN_ONESET_IN_SN_INDEX] = '2';
	rt_strncpy(sys_info.match_sn[1], str, DEV_SN_LEN_MAX);
	sys_info.match_sn[1][DEV_SN_LEN_MAX] = '\0';

	write_syscfgdata_tbl(SYSCFGDATA_TBL_SYS_INFO, 0, &sys_info);
	syscfgdata_syn_proc();
	printf_syn("creat match dev SN success('%s', '%s')\n", sys_info.match_sn[0], sys_info.match_sn[1]);

	return SUCC;
}

long version(void)
{
	struct m3_sys_info_st sys_info;
	char datestr[12];

	//rt_show_version();
	read_syscfgdata_tbl(SYSCFGDATA_TBL_SYS_INFO, 0, &sys_info);

	printf_syn("HW ver:%d.%d.%d\n", M3_HW_VERSION, M3_HW_SUBVERSION, M3_HW_REVISION);
	printf_syn("OS ver:%d.%d.%d\n", RT_VERSION, RT_SUBVERSION, RT_REVISION);
	printf_syn("DB ver:%d.%d.%d\n", (sys_info.db_ver>>16)&0xff, (sys_info.db_ver>>8)&0xff, (sys_info.db_ver)&0xff);
	printf_syn("App ver:%d.%d.%d build %s %s(mtime:%d s)\n", (sys_info.sw_ver>>16)&0xff, (sys_info.sw_ver>>8)&0xff, (sys_info.sw_ver)&0xff,
			   __DATE__, __TIME__, sys_info.sw_file_mtime);

	convert_date_format(__DATE__, datestr);

	printf_syn("Standard ver:%s_%d.%d.%d.%s_%s_%d\n", M3_SYS_TYPE_DESCR, (sys_info.sw_ver>>16)&0xff, (sys_info.sw_ver>>8)&0xff, (sys_info.sw_ver)&0xff,
			   datestr, RT_APP_VER_TYPE, RT_APP_MODIFY_VERSION);

	return 0;
}
FINSH_FUNCTION_EXPORT(version, show sys version information);


/*
 * 以前的函数, 不应该放在这里, 但是......
 */
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

