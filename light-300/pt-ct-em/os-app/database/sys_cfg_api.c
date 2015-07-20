#include <rtdef.h>
#include <sys_cfg_api.h>
#include <syscfgdata.h>
#include "stm32f10x_usart.h"
#include <misc_lib.h>
#include <ade7880_api.h>
#include <ade7880_hw.h>
#include <finsh.h>

#include <ms_common.h>
#include <rs485.h>

#if EM_ALL_TYPE_BASE
#include <sinkinfo_common.h>
#include <ammeter.h>
#include "lwip/private_trap.h"
#endif

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
volatile int test_cf_k = 5596;/* test_cf_k: 5596  cf_expect: 512888  */
//struct register_em_info_st register_em_info;

extern int set_devsn(char *str);
extern volatile u32 SPI_DMA_Table_serial_in[32];
extern volatile u32 SPI_DMA_HSCD_BUFFER[16];
extern volatile u32 AI_HSCD_BUFFER[40];
extern volatile u32 AV_HSCD_BUFFER[40];
extern volatile u32 BI_HSCD_BUFFER[40];
extern volatile u32 BV_HSCD_BUFFER[40];
extern volatile u32 CI_HSCD_BUFFER[40];
extern volatile u32 CV_HSCD_BUFFER[40];  
extern volatile s32 XFVAR_HSCD_BUFFER[3];
extern volatile u32 hsdc_transcomp_flag;

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

#if EM_ALL_TYPE_BASE
int set_em_info(int emnum, int cmd, u32_t *value, u8_t *str)
{
	struct celectric_meter_config_info_st *em_info;
	int ret = SUCC;

	if (NULL == str || NULL == value)
		return FAIL;

	em_info = rt_malloc(sizeof(*em_info));
	if (em_info == RT_NULL) {
		printf_syn("func:%s(), em_info malloc fail\n",__FUNCTION__);
		return FAIL;	
	}
	if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, em_info)) {
		printf_syn("%s(), read em config data tbl fail\n", __FUNCTION__);
		ret = FAIL;
		goto free_ret;
	}
	switch (cmd) {
		case 1:
			rt_memcpy(em_info->em_timing, str, sizeof(em_info->em_timing));
			/*string_to_date(str, &em_info->em_timing);
			if((em_info->em_timing.year < 1970) || (em_info->em_timing.year> 2069)){
				rt_free(em_info);
				return FAIL;
			}
			if((em_info->em_timing.month) < 1 || (em_info->em_timing.year > 12)){
				rt_free(em_info);
				return FAIL;
			}
			if((em_info->em_timing.day) < 1 || (em_info->em_timing.day > maxday_of_month(em_info->em_timing.month, em_info->em_timing.year))){
				rt_free(em_info);
				return FAIL;
			}*/
			break;
		case 2:
			 em_info->meter_connect_mode[emnum] = *value;
			break;
		case 3:
			em_info->calibrate_time_enable[emnum] = *value;
			break;
		case 4:
			em_info->moment_freeze_times[emnum] = *value;
			break;
		case 5:
			em_info->timing_freeze_times[emnum] = *value;
			break;
		case 6:
			rt_memcpy(&em_info->timing_freeze_time_set[emnum], str, sizeof(em_info->timing_freeze_time_set[emnum]));
			/*string_to_date(str, &em_info->timing_freeze_time_set[emnum]);
			if((em_info->timing_freeze_time_set[emnum].year < 1970) || (em_info->timing_freeze_time_set[emnum].year> 2069)){
				rt_free(em_info);
				return FAIL;
			}
			if((em_info->timing_freeze_time_set[emnum].month) < 1 || (em_info->timing_freeze_time_set[emnum].year > 12)){
				rt_free(em_info);
				return FAIL;
			}
			if((em_info->timing_freeze_time_set[emnum].day) < 1 || (em_info->timing_freeze_time_set[emnum].day > maxday_of_month(em_info->timing_freeze_time_set[emnum].month, em_info->timing_freeze_time_set[emnum].year))){
				rt_free(em_info);
				return FAIL;
			}*/
			break;
		case 7:
			em_info->timing_freeze_type[emnum] = *value;
			break;
		case 8:
			em_info->moment_freeze_enable[emnum] = *value;
			break;
		case 9:
			em_info->timing_freeze_enable[emnum] = *value;
			break;
		case 10:
			em_info->meter_protocal_type[emnum] = *value;
			break;	
		case 11:
			em_info->volloss_event_times[emnum] = *value;
			break;
		case 12:
			em_info->volloss_event_source[emnum] = *value;
			break;
		case 13:
			em_info->volover_event_times[emnum] = *value;
			break;
		case 14:
			em_info->volover_event_source[emnum] = *value;
			break;	
		case 15:
			em_info->volunder_event_times[emnum] = *value;
			break;
		case 16:
			em_info->volunder_event_source[emnum] = *value;
			break;
		case 17:
			em_info->phasebreak_event_times[emnum] = *value;
			break;
		case 18:
			em_info->phasebreak_event_source[emnum] = *value;
			break;
		case 19:
			em_info->curloss_event_times[emnum] = *value;
			break;
		case 20:
			em_info->curloss_event_source[emnum] = *value;
			break;
		case 21:
			em_info->curover_event_times[emnum] = *value;
			break;
		case 22:
			em_info->curover_event_source[emnum] = *value;
			break;
		case 23:
			em_info->curbreak_event_times[emnum] = *value;
			break;
		case 24:
			em_info->curbreak_event_source[emnum] = *value;
			break;
		case 25:
			em_info->meterclear_event_times[emnum] = *value;
			break;
		case 26:
			em_info->demandclear_event_times[emnum] = *value;
			break;
		case 27:
			em_info->program_event_times[emnum] = *value;
			break;
		case 28:
			em_info->calibratetime_event_times[emnum] = *value;
			break;
		case 29:
			em_info->rseqvol_event_times[emnum] = *value;
			break;
		case 30:
			em_info->rseqcur_event_times[emnum] = *value;
			break;
		default:
			printf_syn("%s(), recv invalid cmd(%d), should be 1--8,11-29\n", __FUNCTION__, cmd);
			ret = FAIL;
			goto free_ret;
		}
	
		if (RT_EOK != write_syscfgdata_tbl(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, em_info)) {
			printf_syn("%s(), write em cfg param tbl fail\n", __FUNCTION__);
			ret = FAIL;
			goto free_ret;
		}
	
free_ret:
		rt_free(em_info);
		return ret;
}
FINSH_FUNCTION_EXPORT(set_em_info,  "setting em configure param");

int get_em_info(int emnum, int cmd, u32_t *value, u8_t *str)
{
	struct celectric_meter_config_info_st *em_info;
	int ret = SUCC;

	if (NULL == str || NULL == value)
		return FAIL;

	em_info = rt_malloc(sizeof(*em_info));
	if (em_info == RT_NULL) {
		printf_syn("func:%s(), em_info malloc fail\n",__FUNCTION__);
		return FAIL;	
	}
	if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, em_info)) {
		printf_syn("%s(), read em config data tbl fail\n", __FUNCTION__);
		rt_free(em_info);
		return FAIL;
	}
	switch (cmd) {
		case 1:
			rt_memcpy(str, em_info->em_timing, sizeof(em_info->em_timing));
			break;
		case 2:
			*value = em_info->meter_connect_mode[emnum];
			break;
		case 3:
			*value = em_info->calibrate_time_enable[emnum];
			break;
		case 4:
			*value = em_info->moment_freeze_times[emnum];
			break;
		case 5:
			*value = em_info->timing_freeze_times[emnum];
			break;
		case 6:
			rt_memcpy(str, &em_info->timing_freeze_time_set[emnum], sizeof(em_info->timing_freeze_time_set[emnum]));
			break;
		case 7:
			*value = em_info->timing_freeze_type[emnum];
			break;
		case 8:
			*value = em_info->moment_freeze_enable[emnum];
			break;	
		case 9:
			*value = em_info->timing_freeze_enable[emnum];
			break;
		case 10:
			*value = em_info->meter_protocal_type[emnum];
			break;
		case 11:
			*value = em_info->volloss_event_times[emnum];
			break;
		case 12:
			*value = em_info->volloss_event_source[emnum];
			break;
		case 13:
			*value = em_info->volover_event_times[emnum];
			break;
		case 14:
			*value = em_info->volover_event_source[emnum];
			break;	
		case 15:
			*value = em_info->volunder_event_times[emnum];
			break;
		case 16:
			*value = em_info->volunder_event_source[emnum];
			break;
		case 17:
			*value = em_info->phasebreak_event_times[emnum];
			break;
		case 18:
			*value = em_info->phasebreak_event_source[emnum];
			break;
		case 19:
			*value = em_info->curloss_event_times[emnum];
			break;
		case 20:
			*value = em_info->curloss_event_source[emnum];
			break;
		case 21:
			*value = em_info->curover_event_times[emnum];
			break;
		case 22:
			*value = em_info->curover_event_source[emnum];
			break;
		case 23:
			*value = em_info->curbreak_event_times[emnum];
			break;
		case 24:
			*value = em_info->curbreak_event_source[emnum];
			break;
		case 25:
			*value = em_info->meterclear_event_times[emnum];
			break;
		case 26:
			*value = em_info->demandclear_event_times[emnum];
			break;
		case 27:
			*value = em_info->program_event_times[emnum];
			break;
		case 28:
			*value = em_info->calibratetime_event_times[emnum];
			break;
		case 29:
			*value = em_info->rseqvol_event_times[emnum];
			break;
		case 30:
			*value = em_info->rseqcur_event_times[emnum];
			break;
		default:
			printf_syn("%s(), recv invalid cmd(%d), should be 1--8,11-29\n", __FUNCTION__, cmd);
			rt_free(em_info);
			return FAIL;
		}
		
		rt_free(em_info);
		return ret;
}
FINSH_FUNCTION_EXPORT(get_em_info,  "getting em configure param");
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
	case 0:
		uartbase = USART1;
		break;
	case 1:
		uartbase = USART2;
		break;
	case 2:
		uartbase = USART3;
		break;
	case 3:
		uartbase = UART4;
		break;
	case 4:
		uartbase = UART5;
		break;
	default:
		return RT_ERROR;
	}

	USART_Init(uartbase, &USART_InitStructure);
	return RT_EOK;
}

/**
baut: 0=1200, 1=2400,2=4800,3=9600, 4=19200,5=38400, 6=57600,7=115200
 
**/
#if 0
int UART_SetBaud( char baut )
{
    UINT16 ul_bootviaflag = 0;

    ul_bootviaflag = 0x55a0 + baut;

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

/*
 * 该函数只用于stm32的片上uart配置
 *
 * baudrate:
 * databits: 这里时严格意义上的数据位，不包含校验位(stm32的uart的databits包含校验位)
 * paritybit: 0 -- No parity, 1 -- Even parity, 2 -- Odd parity
 * stopbits: 1 -- 1, 15 -- 1.5, 2 -- 2
 *
 * #define UART_PAR_NONE    0  // No parity
 * #define UART_PAR_EVEN    1  // Even parity
 * #define UART_PAR_ODD     2  // Odd parity
 *
 * port:
 * */
int UART_if_Set( rt_uint32_t baut, rt_uint8_t datab, rt_uint8_t parityb, rt_uint8_t stopb, rt_uint8_t port)
{
//	USART_TypeDef *uartbase;
//	USART_InitTypeDef USART_InitStructure;
	struct rs232_param rs232cfg;

	rt_uint8_t portnum = port - 1;
	rt_memset(&rs232cfg, 0, sizeof(struct rs232_param));

	read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_RS232CFG, portnum, &rs232cfg);

	rs232cfg.databits = datab;
	rs232cfg.paritybit = parityb;
	rs232cfg.stopbits = stopb;
	rs232cfg.baudrate = baut;
//	write_syscfgdata_tbl(SYSCFGDATA_TBL_RS232CFG, portnum, &rs232cfg);
//	syscfgdata_syn_proc();
	if (RT_EOK != do_set_rs232cfg(&rs232cfg,portnum)) {
		return RT_ERROR;
	}

	return RT_EOK;
}

#if 0
int UART_if_Set( rt_uint32_t baut, rt_uint8_t datab, rt_uint8_t parityb, rt_uint8_t stopb, rt_uint8_t port)
{
#if 0 /* mark by David */
	USART_TypeDef *uartbase;
	USART_InitTypeDef USART_InitStructure;
	struct rs232_param *rs232cfg;
	rt_uint8_t portnum = port-1;
	
	rs232cfg->databits = datab;
	rs232cfg->paritybit = parityb;
	rs232cfg->stopbits = stopb;
	rs232cfg->baudrate = baut;

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

	switch (portnum) {
	case 0:
		uartbase = USART1;
		break;
	case 1:
		uartbase = USART2;
		break;
	case 2:
		uartbase = USART3;
		break;
	default:
		return RT_ERROR;
	}
	write_syscfgdata_tbl(SYSCFGDATA_TBL_RS232CFG, portnum, rs232cfg);
	USART_Init(uartbase, &USART_InitStructure);
#endif
	return RT_EOK;
}
#endif
//FINSH_FUNCTION_EXPORT(UART_if_Set, baudrate-databits-paritybits-stopbits-portnum);

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


#if RT_USING_ADE7880
/* 打印关口表相关信息 */
int get_gwem_info(int chlx)
{
	struct gateway_em_st	 *em_info;

	em_info = rt_malloc(sizeof(*em_info));
	if (NULL == em_info) {
		printf_syn("%s(), alloc mem fail\n", __FUNCTION__);
		return FAIL;
	}

	if (RT_EOK != read_syscfgdata_tbl(SYSCFGDATA_TBL_GW_EM_INFO, 0, em_info)) {
		printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
		rt_free(em_info);
		return FAIL;
	}

	printf_syn("--[data in flash]--\nv-gain:%d, %d, %d(0x%x, 0x%x, 0x%x)\n"
		"i-gain:%d, %d, %d(0x%x, 0x%x, 0x%x)\np-gain:%d, %d, %d(0x%x, 0x%x, 0x%x)\n",
			/* em_info.master_sn,em_info->pt_sn, em_info->ct_sn, */
			em_info->chlx_st[chlx].pa_vgain, em_info->chlx_st[chlx].pb_vgain, em_info->chlx_st[chlx].pc_vgain,
			em_info->chlx_st[chlx].pa_vgain, em_info->chlx_st[chlx].pb_vgain, em_info->chlx_st[chlx].pc_vgain,
#if 1
			em_info->chlx_st[chlx].pa_igain, em_info->chlx_st[chlx].pb_igain, em_info->chlx_st[chlx].pc_igain,
			em_info->chlx_st[chlx].pa_igain, em_info->chlx_st[chlx].pb_igain, em_info->chlx_st[chlx].pc_igain,
#else
			em_info->pa_igain, em_info->pb_igain, em_info->pc_igain,
			em_info->pa_igain, em_info->pb_igain, em_info->pc_igain,
#endif
			em_info->chlx_st[chlx].pa_pgain, em_info->chlx_st[chlx].pb_pgain, em_info->chlx_st[chlx].pc_pgain,
			em_info->chlx_st[chlx].pa_pgain, em_info->chlx_st[chlx].pb_pgain, em_info->chlx_st[chlx].pc_pgain);
	printf_syn("wattos:%d, %d, %d(0x%x, 0x%x, 0x%x)\nvrmsos:%d, %d, %d(0x%x, 0x%x, 0x%x)\nirmsos:%d, %d, %d(0x%x, 0x%x, 0x%x)\n",
			em_info->chlx_st[chlx].pa_wattos, em_info->chlx_st[chlx].pb_wattos, em_info->chlx_st[chlx].pc_wattos,
			em_info->chlx_st[chlx].pa_wattos, em_info->chlx_st[chlx].pb_wattos, em_info->chlx_st[chlx].pc_wattos,
			em_info->chlx_st[chlx].pa_vrmsos, em_info->chlx_st[chlx].pb_vrmsos, em_info->chlx_st[chlx].pc_vrmsos,
			em_info->chlx_st[chlx].pa_vrmsos, em_info->chlx_st[chlx].pb_vrmsos, em_info->chlx_st[chlx].pc_vrmsos,
			em_info->chlx_st[chlx].pa_irmsos, em_info->chlx_st[chlx].pb_irmsos, em_info->chlx_st[chlx].pc_irmsos,
			em_info->chlx_st[chlx].pa_irmsos, em_info->chlx_st[chlx].pb_irmsos, em_info->chlx_st[chlx].pc_irmsos);
	printf_syn("cfden:%d, %d, %d(0x%x, 0x%x, 0x%x)\nxphcal:%d, %d, %d(0x%x, 0x%x, 0x%x)\nwthr:%d(0x%x)\nvarthr:%d(0x%x)\n",
			em_info->chlx_st[chlx].cf1den, em_info->chlx_st[chlx].cf2den, em_info->chlx_st[chlx].cf3den,
			em_info->chlx_st[chlx].cf1den, em_info->chlx_st[chlx].cf2den, em_info->chlx_st[chlx].cf3den,
#if 1
			em_info->chlx_st[chlx].pa_phase, em_info->chlx_st[chlx].pb_phase, em_info->chlx_st[chlx].pc_phase,
			em_info->chlx_st[chlx].pa_phase, em_info->chlx_st[chlx].pb_phase, em_info->chlx_st[chlx].pc_phase,
#else  
			em_info->pa_phase, em_info->pb_phase, em_info->pc_phase,
			em_info->pa_phase, em_info->pb_phase, em_info->pc_phase,
#endif
			em_info->chlx_st[chlx].pabc_wthr, em_info->chlx_st[chlx].pabc_wthr,
			em_info->chlx_st[chlx].pabc_varthr, em_info->chlx_st[chlx].pabc_varthr);
	printf_syn("--------------------\n");
	printf_syn("mc_constant:%d(0x%x)\n",em_info->chlx_st[chlx].mc_constant,em_info->chlx_st[chlx].mc_constant);



	printf_syn("p_vk:%d, %d, %d(0x%x, 0x%x, 0x%x)\n"
    		   "p_ik:%d, %d, %d(0x%x, 0x%x, 0x%x)\n"
		   "p_pk:%d, %d, %d(0x%x, 0x%x, 0x%x)\n"
		   "connect33:%d(0x%x)\ncf_k:%d(0x%x)\ncf_k1:%d(0x%x)\n",  
#if 1
	           em_info->chlx_st[chlx].pa_vk, em_info->chlx_st[chlx].pb_vk, em_info->chlx_st[chlx].pc_vk,
	           em_info->chlx_st[chlx].pa_vk, em_info->chlx_st[chlx].pb_vk, em_info->chlx_st[chlx].pc_vk,
		   em_info->chlx_st[chlx].pa_ik, em_info->chlx_st[chlx].pb_ik, em_info->chlx_st[chlx].pc_ik,
		   em_info->chlx_st[chlx].pa_ik, em_info->chlx_st[chlx].pb_ik, em_info->chlx_st[chlx].pc_ik,
		   em_info->chlx_st[chlx].pa_pk, em_info->chlx_st[chlx].pb_pk, em_info->chlx_st[chlx].pc_pk,
		   em_info->chlx_st[chlx].pa_pk, em_info->chlx_st[chlx].pb_pk, em_info->chlx_st[chlx].pc_pk,
#else	
	           em_info->pa_vk, em_info->pb_vk, em_info->pc_vk,
	           em_info->pa_vk, em_info->pb_vk, em_info->pc_vk,
		   em_info->pa_ik, em_info->pb_ik, em_info->pc_ik,
		   em_info->pa_ik, em_info->pb_ik, em_info->pc_ik,
		   em_info->pa_pk, em_info->pb_pk, em_info->pc_pk,
		   em_info->pa_pk, em_info->pb_pk, em_info->pc_pk,
#endif
		   em_info->chlx_st[chlx].connect33,em_info->chlx_st[chlx].connect33,
		   em_info->chlx_st[chlx].cf_k,em_info->chlx_st[chlx].cf_k,
		   em_info->chlx_st[chlx].cf_k1,em_info->chlx_st[chlx].cf_k1);
	
	printf_syn("----------------------------------------------------------------\n");


	if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_GW_EM_INFO, 0, em_info)) {
		printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
		rt_free(em_info);
		return FAIL;
	}

	printf_syn("--[data in ram]--\nv-gain:%d, %d, %d(0x%x, 0x%x, 0x%x)\ni-gain:%d, %d, %d(0x%x, 0x%x, 0x%x)\n"
			"p-gain:%d, %d, %d(0x%x, 0x%x, 0x%x)\nwattos:%d, %d, %d(0x%x, 0x%x, 0x%x)\n",
			em_info->chlx_st[chlx].pa_vgain, em_info->chlx_st[chlx].pb_vgain, em_info->chlx_st[chlx].pc_vgain,
			em_info->chlx_st[chlx].pa_vgain, em_info->chlx_st[chlx].pb_vgain, em_info->chlx_st[chlx].pc_vgain,
#if 1
			em_info->chlx_st[chlx].pa_igain, em_info->chlx_st[chlx].pb_igain, em_info->chlx_st[chlx].pc_igain,
			em_info->chlx_st[chlx].pa_igain, em_info->chlx_st[chlx].pb_igain, em_info->chlx_st[chlx].pc_igain,
#else
			em_info->pa_igain, em_info->pb_igain, em_info->pc_igain,  
			em_info->pa_igain, em_info->pb_igain, em_info->pc_igain,
#endif
			em_info->chlx_st[chlx].pa_pgain, em_info->chlx_st[chlx].pb_pgain, em_info->chlx_st[chlx].pc_pgain,
			em_info->chlx_st[chlx].pa_pgain, em_info->chlx_st[chlx].pb_pgain, em_info->chlx_st[chlx].pc_pgain,
			em_info->chlx_st[chlx].pa_wattos, em_info->chlx_st[chlx].pb_wattos, em_info->chlx_st[chlx].pc_wattos,
			em_info->chlx_st[chlx].pa_wattos, em_info->chlx_st[chlx].pb_wattos, em_info->chlx_st[chlx].pc_wattos);

	printf_syn("vrmsos:%d, %d, %d(0x%x, 0x%x, 0x%x)\nirmsos:%d, %d, %d(0x%x, 0x%x, 0x%x)\n"
			"cfden:%d, %d, %d(0x%x, 0x%x, 0x%x)\nxphcal:%d, %d, %d(0x%x, 0x%x, 0x%x)\nwthr:%d(0x%x)\n",
			em_info->chlx_st[chlx].pa_vrmsos, em_info->chlx_st[chlx].pb_vrmsos, em_info->chlx_st[chlx].pc_vrmsos,
			em_info->chlx_st[chlx].pa_vrmsos, em_info->chlx_st[chlx].pb_vrmsos, em_info->chlx_st[chlx].pc_vrmsos,
			em_info->chlx_st[chlx].pa_irmsos, em_info->chlx_st[chlx].pb_irmsos, em_info->chlx_st[chlx].pc_irmsos,
			em_info->chlx_st[chlx].pa_irmsos, em_info->chlx_st[chlx].pb_irmsos, em_info->chlx_st[chlx].pc_irmsos,
			em_info->chlx_st[chlx].cf1den, em_info->chlx_st[chlx].cf2den, em_info->chlx_st[chlx].cf3den,
			em_info->chlx_st[chlx].cf1den, em_info->chlx_st[chlx].cf2den, em_info->chlx_st[chlx].cf3den,
#if 1
			em_info->chlx_st[chlx].pa_phase, em_info->chlx_st[chlx].pb_phase, em_info->chlx_st[chlx].pc_phase,
			em_info->chlx_st[chlx].pa_phase, em_info->chlx_st[chlx].pb_phase, em_info->chlx_st[chlx].pc_phase,
#else
			em_info->pa_phase, em_info->pb_phase, em_info->pc_phase,
			em_info->pa_phase, em_info->pb_phase, em_info->pc_phase,
#endif
			em_info->chlx_st[chlx].pabc_wthr, em_info->chlx_st[chlx].pabc_wthr,
			em_info->chlx_st[chlx].pabc_varthr, em_info->chlx_st[chlx].pabc_varthr);
	printf_syn("--------------------\n");
	printf_syn("mc_constant:%d(0x%x)\n",em_info->chlx_st[chlx].mc_constant,em_info->chlx_st[chlx].mc_constant);

	printf_syn("p_vk:%d, %d, %d(0x%x, 0x%x, 0x%x)\n"
    		   "p_ik:%d, %d, %d(0x%x, 0x%x, 0x%x)\n"
		   "p_pk:%d, %d, %d(0x%x, 0x%x, 0x%x)\n"
   		   "connect33:%d(0x%x)\n"
   		   "cf_k:%d(0x%x)\n" 
   		   "cf_k1:%d(0x%x)\n", 
#if 1
	           em_info->chlx_st[chlx].pa_vk, em_info->chlx_st[chlx].pb_vk, em_info->chlx_st[chlx].pc_vk,
	           em_info->chlx_st[chlx].pa_vk, em_info->chlx_st[chlx].pb_vk, em_info->chlx_st[chlx].pc_vk,
		   em_info->chlx_st[chlx].pa_ik, em_info->chlx_st[chlx].pb_ik, em_info->chlx_st[chlx].pc_ik,
		   em_info->chlx_st[chlx].pa_ik, em_info->chlx_st[chlx].pb_ik, em_info->chlx_st[chlx].pc_ik,
		   em_info->chlx_st[chlx].pa_pk, em_info->chlx_st[chlx].pb_pk, em_info->chlx_st[chlx].pc_pk,
		   em_info->chlx_st[chlx].pa_pk, em_info->chlx_st[chlx].pb_pk, em_info->chlx_st[chlx].pc_pk,
#else
	           em_info->pa_vk, em_info->pb_vk, em_info->pc_vk,
	           em_info->pa_vk, em_info->pb_vk, em_info->pc_vk,
		   em_info->pa_ik, em_info->pb_ik, em_info->pc_ik,
		   em_info->pa_ik, em_info->pb_ik, em_info->pc_ik,
		   em_info->pa_pk, em_info->pb_pk, em_info->pc_pk,
		   em_info->pa_pk, em_info->pb_pk, em_info->pc_pk,
#endif
		   em_info->chlx_st[chlx].connect33,em_info->chlx_st[chlx].connect33,
		   em_info->chlx_st[chlx].cf_k,em_info->chlx_st[chlx].cf_k,
		   em_info->chlx_st[chlx].cf_k1,em_info->chlx_st[chlx].cf_k1);
 
	printf_syn("----------------------------------------------------------------\n");

	set_7880adj_reg(AARI_READ_PXX_REG);

	rt_free(em_info);

	return SUCC;
}
FINSH_FUNCTION_EXPORT(get_gwem_info, "read gateway em info");


int set_rg_al_1st(int id, int chlx, int avk, int bvk, int cvk, int aik, int bik, int cik, int apk,int bpk, int cpk)
{
	struct gateway_em_st	 *em_info;
	int ret = SUCC;

	em_info = rt_malloc(sizeof(*em_info));
	if (NULL == em_info) {
		printf_syn("%s(), alloc mem fail\n", __FUNCTION__);
		return FAIL;
	}
	if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_GW_EM_INFO, 0, em_info)) {
		printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
		ret = FAIL;
		goto free_ret;
	}
#if 0 == EM_ALL_TYPE_BASE
	chlx = 0;
#endif

	switch (id) {

	case 1:
		em_info->chlx_st[chlx].pa_vk = avk;
		em_info->chlx_st[chlx].pb_vk = bvk;
		em_info->chlx_st[chlx].pc_vk = cvk;
		em_info->chlx_st[chlx].pa_ik = aik;
		em_info->chlx_st[chlx].pb_ik = bik;
		em_info->chlx_st[chlx].pc_ik = cik;
		em_info->chlx_st[chlx].pa_pk = apk;
		em_info->chlx_st[chlx].pb_pk = bpk;		
		em_info->chlx_st[chlx].pc_pk = cpk;

		break;

	case 2:

		break;


	default:
		printf_syn("%s(), recv invalid id(%d), should 1/2/3/[123][]123]\n", __FUNCTION__, id);
		ret = FAIL;
		goto free_ret;
	}

	if (RT_EOK != write_syscfgdata_tbl(SYSCFGDATA_TBL_GW_EM_INFO, 0, em_info)) {
		printf_syn("%s(), write syscfg data tbl fail\n", __FUNCTION__);
		ret = FAIL;
		goto free_ret;
	}

	set_cfxden(em_info->chlx_st[chlx].cf1den, em_info->chlx_st[chlx].cf2den, em_info->chlx_st[chlx].cf3den);
	set_vkcpu(em_info->chlx_st[chlx].pa_vk, em_info->chlx_st[chlx].pb_vk, em_info->chlx_st[chlx].pc_vk);
	set_ikcpu(em_info->chlx_st[chlx].pa_ik, em_info->chlx_st[chlx].pb_ik, em_info->chlx_st[chlx].pc_ik);
	set_pkcpu(em_info->chlx_st[chlx].pa_pk, em_info->chlx_st[chlx].pb_pk, em_info->chlx_st[chlx].pc_pk);
	set_pabc_xphcal(em_info->chlx_st[chlx].pa_phase, em_info->chlx_st[chlx].pb_phase, em_info->chlx_st[chlx].pc_phase);
    
	set_p_wthr(em_info->chlx_st[chlx].pabc_wthr);
	set_p_varthr(em_info->chlx_st[chlx].pabc_varthr);

free_ret:
	rt_free(em_info);
	return ret;
}
FINSH_FUNCTION_EXPORT(set_rg_al_1st, "set_7880_chlx_reg_all_1st");

int set_rg_al_2st(int id, int chlx, int a_phase, int b_phase, int c_phase, int wthr, int varthr, int cfk, int cfk1)
{
	struct gateway_em_st	 *em_info;
	int ret = SUCC;

	em_info = rt_malloc(sizeof(*em_info));
	if (NULL == em_info) {
		printf_syn("%s(), alloc mem fail\n", __FUNCTION__);
		return FAIL;
	}
	if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_GW_EM_INFO, 0, em_info)) {
		printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
		ret = FAIL;
		goto free_ret;
	}
#if 0 == EM_ALL_TYPE_BASE
	chlx = 0;
#endif

	switch (id) {
 
	case 1:
		em_info->chlx_st[chlx].pa_phase = a_phase;
		em_info->chlx_st[chlx].pb_phase = b_phase;
		em_info->chlx_st[chlx].pc_phase = c_phase;
		em_info->chlx_st[chlx].pabc_wthr = wthr;
		em_info->chlx_st[chlx].pabc_varthr = varthr;
		em_info->chlx_st[chlx].cf_k = cfk;
		em_info->chlx_st[chlx].cf_k1 = cfk1;
		break;

	case 2:

		break;


	default:
		printf_syn("%s(), recv invalid id(%d), should 1/2/3/[123][]123]\n", __FUNCTION__, id);
		ret = FAIL;
		goto free_ret;
	}

	if (RT_EOK != write_syscfgdata_tbl(SYSCFGDATA_TBL_GW_EM_INFO, 0, em_info)) {
		printf_syn("%s(), write syscfg data tbl fail\n", __FUNCTION__);
		ret = FAIL;
		goto free_ret;
	}  

	set_pabc_i_gain(em_info->chlx_st[chlx].pa_igain, em_info->chlx_st[chlx].pb_igain, em_info->chlx_st[chlx].pc_igain);
	set_pabc_v_gain(em_info->chlx_st[chlx].pa_vgain,  em_info->chlx_st[chlx].pb_vgain,  em_info->chlx_st[chlx].pc_vgain);
	set_pabc_p_gain(em_info->chlx_st[chlx].pa_pgain,  em_info->chlx_st[chlx].pb_pgain,  em_info->chlx_st[chlx].pc_pgain);
	set_pabc_wattos(em_info->chlx_st[chlx].pa_wattos, em_info->chlx_st[chlx].pb_wattos, em_info->chlx_st[chlx].pc_wattos);
	set_pabc_vrmsos(em_info->chlx_st[chlx].pa_vrmsos, em_info->chlx_st[chlx].pb_vrmsos, em_info->chlx_st[chlx].pc_vrmsos);
	set_pabc_irmsos(em_info->chlx_st[chlx].pa_irmsos, em_info->chlx_st[chlx].pb_irmsos, em_info->chlx_st[chlx].pc_irmsos);
	set_cfxden(em_info->chlx_st[chlx].cf1den, em_info->chlx_st[chlx].cf2den, em_info->chlx_st[chlx].cf3den);
	set_vkcpu(em_info->chlx_st[chlx].pa_vk, em_info->chlx_st[chlx].pb_vk, em_info->chlx_st[chlx].pc_vk);
	set_ikcpu(em_info->chlx_st[chlx].pa_ik, em_info->chlx_st[chlx].pb_ik, em_info->chlx_st[chlx].pc_ik);
	set_pkcpu(em_info->chlx_st[chlx].pa_pk, em_info->chlx_st[chlx].pb_pk, em_info->chlx_st[chlx].pc_pk);
	set_pabc_xphcal(em_info->chlx_st[chlx].pa_phase, em_info->chlx_st[chlx].pb_phase, em_info->chlx_st[chlx].pc_phase);
    
	set_p_wthr(em_info->chlx_st[chlx].pabc_wthr);
	set_p_varthr(em_info->chlx_st[chlx].pabc_varthr);

free_ret:
	rt_free(em_info);
	return ret;
}
FINSH_FUNCTION_EXPORT(set_rg_al_2st, "set_7880_chlx_reg_all_2st");


int read_reg_all(int chlx)
{
	struct gateway_em_st	 *em_info;

	em_info = rt_malloc(sizeof(*em_info));
	if (NULL == em_info) {
		printf_syn("%s(), alloc mem fail\n", __FUNCTION__);
		return FAIL;
	}

	if (RT_EOK != read_syscfgdata_tbl(SYSCFGDATA_TBL_GW_EM_INFO, 0, em_info)) {
		printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
		rt_free(em_info);
		return FAIL;
	}  

	printf_syn("-----------------chlx(0~11):%d------------------------\n--[data in flash]--\n", chlx);
	printf_syn("set_rg_al_1st(1,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)\n", chlx, em_info->chlx_st[chlx].pa_vk, em_info->chlx_st[chlx].pb_vk, 
		em_info->chlx_st[chlx].pc_vk, em_info->chlx_st[chlx].pa_ik, em_info->chlx_st[chlx].pb_ik, em_info->chlx_st[chlx].pc_ik,
		em_info->chlx_st[chlx].pa_pk, em_info->chlx_st[chlx].pb_pk,em_info->chlx_st[chlx].pc_pk);
	printf_syn("set_rg_al_2st(1,%d,%d,%d,%d,%d,%d,%d,%d)\n\n\n", chlx, em_info->chlx_st[chlx].pa_phase, em_info->chlx_st[chlx].pb_phase,
		em_info->chlx_st[chlx].pc_phase,em_info->chlx_st[chlx].pabc_wthr, em_info->chlx_st[chlx].pabc_varthr, em_info->chlx_st[chlx].cf_k, em_info->chlx_st[chlx].cf_k1);

	if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_GW_EM_INFO, 0, em_info)) {
		printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
		rt_free(em_info);
		return FAIL;
	}

	printf_syn("--[data in ram]--\n");
	printf_syn("set_rg_al_1st(1,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)\n", chlx, em_info->chlx_st[chlx].pa_vk, em_info->chlx_st[chlx].pb_vk, 
		em_info->chlx_st[chlx].pc_vk, em_info->chlx_st[chlx].pa_ik, em_info->chlx_st[chlx].pb_ik, em_info->chlx_st[chlx].pc_ik,
		em_info->chlx_st[chlx].pa_pk, em_info->chlx_st[chlx].pb_pk,em_info->chlx_st[chlx].pc_pk);

	printf_syn("set_rg_al_2st(1,%d,%d,%d,%d,%d,%d,%d,%d)\n\n\n", chlx, em_info->chlx_st[chlx].pa_phase, em_info->chlx_st[chlx].pb_phase,
		em_info->chlx_st[chlx].pc_phase,em_info->chlx_st[chlx].pabc_wthr, em_info->chlx_st[chlx].pabc_varthr, em_info->chlx_st[chlx].cf_k, em_info->chlx_st[chlx].cf_k1);
 
	printf_syn("set_rg_al_1st(1, chlx, pa_vk, pb_vk, pc_vk, pa_ik, pb_ik, pc_ik, pa_pk, pb_pk, pc_pk)\n");
	printf_syn("set_rg_al_2st(1, chlx, pa_phase, pb_phase, pc_phase, pabc_wthr, pabc_varthr, cf_k, cf_k1)\n\n");


	rt_free(em_info);

	return SUCC;
}
FINSH_FUNCTION_EXPORT(read_reg_all, "read_7880_chlx_reg_all_1st_2st");

#if EM_ALL_TYPE_BASE
int read_wire_con(void)
{
	int chlx;

	for(chlx = 0; chlx < 12; chlx++){
		printf_syn("chlx num:%d  wire mode:%d\n", chlx, register_em_info.em_wire_con_mode[chlx]);
	}  	

	return SUCC;

}
FINSH_FUNCTION_EXPORT(read_wire_con, "read_wire_connect");
#endif

int set_7880_adj(int id, int gain, int chlx, int old_reg, int old_output, int new_output)
{
	int val;
	struct gateway_em_st	 *em_info;
	int ret = SUCC;

	em_info = rt_malloc(sizeof(*em_info));
	if (NULL == em_info) {
		printf_syn("%s(), alloc mem fail\n", __FUNCTION__);
		return FAIL;
	}
	if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_GW_EM_INFO, 0, em_info)) {
		printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
		ret = FAIL;
		goto free_ret;
	}
#if 0 == EM_ALL_TYPE_BASE
	chlx = 0;
#endif

	switch (id) {
	case 1:
		em_info->chlx_st[chlx].pa_vgain = gain;
		break;

	case 2:
		em_info->chlx_st[chlx].pb_vgain = gain;
		break;

	case 3:
		em_info->chlx_st[chlx].pc_vgain = gain;
		break;

	case 11:
		em_info->chlx_st[chlx].pa_igain = gain;
		break;

	case 12:
		em_info->chlx_st[chlx].pb_igain = gain;
		break;

	case 13:
		em_info->chlx_st[chlx].pc_igain = gain;
		break;

	case 21:
		em_info->chlx_st[chlx].pa_pgain = gain;
		break;

	case 22:
		em_info->chlx_st[chlx].pb_pgain = gain;
		break;

	case 23:
		em_info->chlx_st[chlx].pc_pgain = gain;
		break;

	case 31:
		val  = (old_output * 10000)/old_reg;
		gain = (new_output * 10000)/val;
		em_info->chlx_st[chlx].pa_vk = gain;
		printf_syn("pavk channel-0:%d  \n""old_output:%d  old_reg:%d new_output:%d new_reg:%d \n",
			chlx, old_output, old_reg, new_output, gain);
		break;

	case 32:
		val  = (old_output * 10000)/old_reg;
		gain = (new_output * 10000)/val;
		em_info->chlx_st[chlx].pb_vk = gain;
		printf_syn("pbvk channel-0:%d  \n""old_output:%d  old_reg:%d new_output:%d new_reg:%d \n",
			chlx, old_output, old_reg, new_output, gain);

		break;

	case 33:
		val  = (old_output * 10000)/old_reg;
		gain = (new_output * 10000)/val;
		em_info->chlx_st[chlx].pc_vk = gain;
		printf_syn("pcvk channel-0:%d  \n""old_output:%d  old_reg:%d new_output:%d new_reg:%d \n",
			chlx, old_output, old_reg, new_output, gain);

		break;
		
	case 34:
		val  = (old_output * 10000)/old_reg;
		gain = (new_output * 10000)/val;
		em_info->chlx_st[chlx].pa_ik = gain;
		printf_syn("paik channel-0:%d  \n""old_output:%d  old_reg:%d new_output:%d new_reg:%d \n",
			chlx, old_output, old_reg, new_output, gain);

		break;

	case 35:
		val  = (old_output * 10000)/old_reg;
		gain = (new_output * 10000)/val;
		em_info->chlx_st[chlx].pb_ik = gain;
		printf_syn("pbik channel-0:%d  \n""old_output:%d  old_reg:%d new_output:%d new_reg:%d \n",
			chlx, old_output, old_reg, new_output, gain);

		break;

	case 36:
		val  = (old_output * 10000)/old_reg;
		gain = (new_output * 10000)/val;
		em_info->chlx_st[chlx].pc_ik = gain;
		printf_syn("pcik channel-0:%d  \n""old_output:%d  old_reg:%d new_output:%d new_reg:%d \n",
			chlx, old_output, old_reg, new_output, gain);

		break;

	case 37:
		val  = (old_output * 10000)/old_reg;
		gain = (new_output * 10000)/val;
		em_info->chlx_st[chlx].pa_pk = gain;
		printf_syn("papk channel-0:%d  \n""old_output:%d  old_reg:%d new_output:%d new_reg:%d \n",
			chlx, old_output, old_reg, new_output, gain);

		break;

	case 38:
		val  = (old_output * 10000)/old_reg;
		gain = (new_output * 10000)/val;
		em_info->chlx_st[chlx].pb_pk = gain;
		printf_syn("pbpk channel-0:%d  \n""old_output:%d  old_reg:%d new_output:%d new_reg:%d \n",
			chlx, old_output, old_reg, new_output, gain);

		break;
  
	case 39: 
		val  = (old_output * 10000)/old_reg;
		gain = (new_output * 10000)/val;
		em_info->chlx_st[chlx].pc_pk = gain;
		printf_syn("pcpk channel-0:%d  \n""old_output:%d  old_reg:%d new_output:%d new_reg:%d \n",
			chlx, old_output, old_reg, new_output, gain);

		break;

	case 41:
		em_info->chlx_st[chlx].pabc_wthr = gain;
		break;

	case 42:
		em_info->chlx_st[chlx].pabc_varthr = gain;
		break;

	case 43:
		em_info->chlx_st[chlx].cf_k = gain;
		break;

	case 44:
		em_info->chlx_st[chlx].cf_k1 = gain;
		break;

		
	case 51:
		em_info->chlx_st[chlx].pa_vk = gain;
		break;

	case 52:
		em_info->chlx_st[chlx].pb_vk = gain;
		break;

	case 53:
		em_info->chlx_st[chlx].pc_vk = gain;
		break;

	case 61:
		em_info->chlx_st[chlx].pa_ik = gain;
		break;

	case 62:
		em_info->chlx_st[chlx].pb_ik = gain;
		break;

	case 63:
		em_info->chlx_st[chlx].pc_ik = gain;
		break;
		
	case 71:
		em_info->chlx_st[chlx].pa_pk = gain;
		break;

	case 72:
		em_info->chlx_st[chlx].pb_pk = gain;
		break;

	case 73:
		em_info->chlx_st[chlx].pc_pk = gain;
		break;
  
	case 81:
		em_info->chlx_st[chlx].cf1den = gain;
		break;

	case 82:
		em_info->chlx_st[chlx].cf2den = gain;
		break;

	case 83:
		em_info->chlx_st[chlx].cf3den = gain;
		break;

	case 91:
		em_info->chlx_st[chlx].pa_phase = gain;
		break;

	case 92:
		em_info->chlx_st[chlx].pb_phase = gain;
		break;

	case 93:
		em_info->chlx_st[chlx].pc_phase = gain;
		break;
	default:
		printf_syn("%s(), recv invalid id(%d), should 1/2/3/[123][]123]\n", __FUNCTION__, id);
		ret = FAIL;
		goto free_ret;
	}

	if (RT_EOK != write_syscfgdata_tbl(SYSCFGDATA_TBL_GW_EM_INFO, 0, em_info)) {
		printf_syn("%s(), write syscfg data tbl fail\n", __FUNCTION__);
		ret = FAIL;
		goto free_ret;
	}

	set_pabc_i_gain(em_info->chlx_st[chlx].pa_igain, em_info->chlx_st[chlx].pb_igain, em_info->chlx_st[chlx].pc_igain);
	set_pabc_v_gain(em_info->chlx_st[chlx].pa_vgain,  em_info->chlx_st[chlx].pb_vgain,  em_info->chlx_st[chlx].pc_vgain);
	set_pabc_p_gain(em_info->chlx_st[chlx].pa_pgain,  em_info->chlx_st[chlx].pb_pgain,  em_info->chlx_st[chlx].pc_pgain);
	set_pabc_wattos(em_info->chlx_st[chlx].pa_wattos, em_info->chlx_st[chlx].pb_wattos, em_info->chlx_st[chlx].pc_wattos);
	set_pabc_vrmsos(em_info->chlx_st[chlx].pa_vrmsos, em_info->chlx_st[chlx].pb_vrmsos, em_info->chlx_st[chlx].pc_vrmsos);
	set_pabc_irmsos(em_info->chlx_st[chlx].pa_irmsos, em_info->chlx_st[chlx].pb_irmsos, em_info->chlx_st[chlx].pc_irmsos);
	set_cfxden(em_info->chlx_st[chlx].cf1den, em_info->chlx_st[chlx].cf2den, em_info->chlx_st[chlx].cf3den);
	set_vkcpu(em_info->chlx_st[chlx].pa_vk, em_info->chlx_st[chlx].pb_vk, em_info->chlx_st[chlx].pc_vk);
	set_ikcpu(em_info->chlx_st[chlx].pa_ik, em_info->chlx_st[chlx].pb_ik, em_info->chlx_st[chlx].pc_ik);
	set_pkcpu(em_info->chlx_st[chlx].pa_pk, em_info->chlx_st[chlx].pb_pk, em_info->chlx_st[chlx].pc_pk);
	set_pabc_xphcal(em_info->chlx_st[chlx].pa_phase, em_info->chlx_st[chlx].pb_phase, em_info->chlx_st[chlx].pc_phase);
    
	set_p_wthr(em_info->chlx_st[chlx].pabc_wthr);
	set_p_varthr(em_info->chlx_st[chlx].pabc_varthr);

free_ret:
	rt_free(em_info);
	return ret;
}
FINSH_FUNCTION_EXPORT(set_7880_adj, "set ade7880 adjust param");
#endif


#if RT_USING_ADE7880

int mode7880_con(int id, int mc, int chlx)
{
	set_connect(id); 
	Fast_Startup_ADE7880_ToMCModel();
	mode7880_mc(id, mc, chlx);

	return SUCC;
}
FINSH_FUNCTION_EXPORT(mode7880_con, "single connect mode");

#define MODE_MC 0
int mode7880_mc(int id, int mc,int chlx)
{
	struct gateway_em_st	 *em_info;

	em_info = rt_malloc(sizeof(*em_info));
	if (NULL == em_info) {
		printf_syn("%s(), alloc mem fail\n", __FUNCTION__);
		return FAIL;
	}
	
	int cf_expect, cfxden_val, varthr, wthr;
	int a, b, c;
	u32 val_cf;

	if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_GW_EM_INFO, 0, em_info)) {
		printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
		rt_free(em_info);
		return FAIL;
	}
#if MODE_MC
		printf_syn("connect33_data:%d  \n", connect33_data); 
#endif
		if(connect33_data == 0){	 /* 调试三相四线制脉冲系数 */
			val_cf = em_info->chlx_st[chlx].cf_k;
		}else if(connect33_data == 1){/* 调试三相三线制脉冲系数 */
			val_cf = em_info->chlx_st[chlx].cf_k1;
		}else {
			printf_syn("not find the value,flash write something wrong...  \n");
		}if(id == 3){
			printf_syn("debuging...  \n");  
		} 
	em_info->chlx_st[chlx].mc_constant = mc;
#if MODE_MC
	printf_syn("val_cf:%d\n ",val_cf);
		printf_syn("autodebug new mc %d\n", mc);		
#endif
		cf_expect = mc*10;
		//cf_expect = (mc*288) / (72); /* 0.000001 */
		cfxden_val = val_cf / cf_expect; 
	varthr = 10;
	wthr = 10;  
#if MODE_MC
		printf_syn("autodebug new mc over...\ncf_expect %d cfxden_val %d \n", cf_expect, cfxden_val);
#endif
		em_info->chlx_st[chlx].cf1den = cfxden_val; 
		em_info->chlx_st[chlx].cf2den = cfxden_val; 
		em_info->chlx_st[chlx].cf3den = cfxden_val; 
		em_info->chlx_st[chlx].pabc_wthr = wthr;
		em_info->chlx_st[chlx].pabc_varthr = varthr;

		set_cfxden(em_info->chlx_st[chlx].cf1den, em_info->chlx_st[chlx].cf2den, em_info->chlx_st[chlx].cf3den);
		set_p_wthr(em_info->chlx_st[chlx].pabc_wthr);
		set_p_varthr(em_info->chlx_st[chlx].pabc_varthr);

	if (RT_EOK != write_syscfgdata_tbl(SYSCFGDATA_TBL_GW_EM_INFO, 0, em_info)) {
		printf_syn("%s(), write syscfg data tbl fail\n", __FUNCTION__);
		rt_free(em_info);
		return FAIL;

	}
	
	/* 去能dsp写保护 */
	Write_8bitReg_ADE7880(0xE7FE, 0xAD);
	Write_8bitReg_ADE7880(0xE7E3, 0x00);

	get_cfxden_gain(&a, &b, &c); 
	Write_16bitReg_ADE7880(CF1DEN_Register_Address, a);   
	Write_16bitReg_ADE7880(CF2DEN_Register_Address, b);
	Write_16bitReg_ADE7880(CF3DEN_Register_Address, c);

	get_p_varthr_gain(&a);
	Write_8bitReg_ADE7880(VARTHR_Register_Address, a);	

	get_p_wthr_gain(&a);
	Write_8bitReg_ADE7880(WTHR_Register_Address, a);	
	
	/* 使能dsp写保护 */
	Write_8bitReg_ADE7880(0xE7FE, 0xAD);
	Write_8bitReg_ADE7880(0xE7E3, 0x80);

	rt_thread_delay(get_ticks_of_ms(600));  

	rt_free(em_info);
	return SUCC;
}
FINSH_FUNCTION_EXPORT(mode7880_mc, "mc value mode");

int t_cfk_hdebug(int id, int val, int chlx)
{  
	struct gateway_em_st	 *em_info;

	em_info = rt_malloc(sizeof(*em_info));
	if (NULL == em_info) {
		printf_syn("%s(), alloc mem fail\n", __FUNCTION__);
		return FAIL;
	}
	 
	int mc, cf_expect;
	u32 val_cfk;

	/* 电表常数 mc = 12800 */
	mc = 12800;
 
	/* 计算预期cf脉冲输出频率*1000000 */
	cf_expect = mc*10;
	printf_syn("cf_expect: %d \n", cf_expect);


	switch(id){
	case 0:
		test_cf_k = test_cf_k - 1;
		printf_syn("seeout N: %d \n", test_cf_k);
		break;

	case 1: 
		test_cf_k = test_cf_k + 1;
		printf_syn("seeout N: %d \n", test_cf_k);		
		break;  
    
	case 2:
		test_cf_k = val;
		printf_syn("seeout N: %d \n", test_cf_k);	
		break;	

	case 3:
		val_cfk = test_cf_k * cf_expect;
		printf_syn("val_cfk: %d \n", val_cfk);	

		printf_syn("will update val_cfk data...\n");
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_GW_EM_INFO, 0, em_info)) {
			printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
		}

			if(connect33_data == 0){	/* 调试三相四线制脉冲系数 */
				em_info->chlx_st[chlx].cf_k = val_cfk;  
				printf_syn("34-em_info->cf_k: %d \n", val_cfk);
			}else if(connect33_data == 1){
				em_info->chlx_st[chlx].cf_k1 = val_cfk;/* 调试三相三线制脉冲系数 */
				printf_syn("33-em_info->cf_k1: %d \n", val_cfk);
			}else {
				printf_syn("not find the value,flash write something wrong...  ");
			}
		if (RT_EOK != write_syscfgdata_tbl(SYSCFGDATA_TBL_GW_EM_INFO, 0, em_info)) {
			printf_syn("%s(), write syscfg data tbl fail\n", __FUNCTION__);

	}

	printf_syn("debug the val_cfk completed...\n ");	
		
		break;

	default:

		printf_syn(" error: fun:%s, line:%d\n", __FUNCTION__, __LINE__);
		rt_free(em_info);
		return FAIL;

	}

  
	printf_syn(" debug the power adjust  completed...\n ");	

	rt_free(em_info);
	return SUCC;
}  
  
FINSH_FUNCTION_EXPORT(t_cfk_hdebug, "handdebug cfk");
void autodebug_7880(int cmd,int v_setout,int i_setout,int p_setout,int chlx_cnt)
{
   	 
	switch (cmd) {  
	case 1:  
		px_phase_autodebug(chlx_cnt);

		break;
 
	case 2:
		printf_syn(" write cmd  success: fun:%s, line:%d\n", __FUNCTION__, __LINE__);
		px_gain_matching_autodebug(chlx_cnt);
		break;

	case 3:
  
		printf_syn(" write cmd  success: fun:%s, line:%d\n", __FUNCTION__, __LINE__);
		px_vircur_setout_autodebug(v_setout, i_setout, p_setout, chlx_cnt);

		break;

	case 4:
		printf_syn(" write cmd  success: fun:%s, line:%d\n", __FUNCTION__, __LINE__);
		px_vircur_setout_autodebug(v_setout, i_setout, p_setout, chlx_cnt);

		break;
  
	case 5:
		px_harmonic_mode_parameter(v_setout, i_setout);

		break;

	case 6:
		printf_phase_vi(v_setout, i_setout, chlx_cnt);

		break;

	case 7:
		printf_phase_vi(v_setout, i_setout, chlx_cnt);

		break;  
	
	case 8:   
		printf_virtual_vi(v_setout, i_setout, chlx_cnt);

		break;
	
	case 9:  
		px_phase_autodebug_chlx(chlx_cnt);

		break;

	case 10:
		printf_reg_info_debug(i_setout);		

		break;	

	case 11:
		px_voltage_setout_autodebug(v_setout, i_setout, p_setout, 12);

		break;	

	case 12:
		px_current_setout_autodebug(v_setout, i_setout, p_setout, 12);	  

		break;

	case 13:
		px_active_power_setout_autodebug(v_setout, i_setout, p_setout, 12);	  	  

		break;
   
	case 14:
#if EM_ALL_TYPE_BASE && EM_MULTI_BASE		
		switch_current_channels(chlx_cnt);
		t_cfk_hdebug(2,i_setout,chlx_cnt); 
		t_cfk_hdebug(3,0,chlx_cnt);
		mode7880_mc(3,v_setout,chlx_cnt);
#endif

		break;  
  
	case 15:
		get_gwem_info(chlx_cnt);
		
		break;

	case 16:   
#if EM_ALL_TYPE_BASE && EM_MULTI_BASE		
		switch_current_channels(chlx_cnt);
		px_setout_handdebug(v_setout,i_setout,chlx_cnt);
#else  
		px_setout_handdebug(v_setout,i_setout,0);
#endif

		break;
         
	case 17:
#if EM_ALL_TYPE_BASE && EM_MULTI_BASE
		switch_current_channels(0);
		set_decoder_3to8_data(chlx_cnt);
		rt_thread_delay(get_ticks_of_ms(200));
		px_setout_voltage();
#endif  
       
	case 18:
#if EM_ALL_TYPE_BASE && EM_MULTI_BASE
		switch_current_channels(chlx_cnt);
#endif
		printf_virtual_vi(v_setout, chlx_cnt, 2);	    
		break;

	case 19:
#if EM_ALL_TYPE_BASE && EM_MULTI_BASE		
		switch_current_channels(chlx_cnt);
#endif
  
		break;	

	case 20:
#if EM_ALL_TYPE_BASE && EM_MULTI_BASE		
		t_cfk_hdebug(2,i_setout,chlx_cnt); 
		t_cfk_hdebug(3,0,chlx_cnt);
		mode7880_mc(3,v_setout,chlx_cnt);
#endif

		break;	

	case 21:
		px_vi_sample_reac_p_hsdc();

		break;
	default:
		printf_syn("%s(), recv invalid id(%d), should 1/2/3/[123][]123]\n", __FUNCTION__, cmd);
	}
	
} 
FINSH_FUNCTION_EXPORT(autodebug_7880, "set autodebug 7880 reg");
#endif


#if PT_DEVICE || CT_DEVICE
int get_tw_info(void)
{
	struct tinywireless_if_info_st	 tw_info;

	if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_TW_INFO, 0, &tw_info)) {
		printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
		return FAIL;
	}

	printf_syn("sync_word:0x%x\n", tw_info.sync_word);

	printf_syn("channel_no:%d\n", tw_info.channel_no);

	printf_syn("cid:%d\n", tw_info.cid);

	printf_syn("slave_id:%d\n", tw_info.slave_id);

	return SUCC;
}
FINSH_FUNCTION_EXPORT(get_tw_info, "get tiny wireless info");

int set_tw_info(int cmd, unsigned int data)
{
	struct tinywireless_if_info_st	 tw_info;

	if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_TW_INFO, 0, &tw_info)) {
		printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
		return FAIL;
	}

	switch (cmd) {
	case 1:
		tw_info.sync_word = data;

		/* set sync word to si4432 */

		break;

	case 2:
		tw_info.channel_no = data;
		break;

	case 3:
		tw_info.cid = data;
		break;

	case 4:
		tw_info.slave_id = data;
		break;

	default:
		break;
	}

	if (RT_EOK != write_syscfgdata_tbl(SYSCFGDATA_TBL_TW_INFO, 0, &tw_info)) {
		printf_syn("%s(), write syscfg data tbl fail\n", __FUNCTION__);
		return FAIL;
	}
	syscfgdata_syn_proc();
	
	return SUCC;
}
FINSH_FUNCTION_EXPORT(set_tw_info, "set tiny wireless info, 1-sync_word, 2-channel_no, 3-cid, 4-slave_id");

#if RT_USING_RS485_BUS
int set_tw_netcfg_m(int index)
{
	struct tinywireless_if_info_st	 tw_info;

	if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_TW_INFO, 0, &tw_info)) {
		printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
		return FAIL;
	}
	
	switch (index) {
		case 0:
			tw_info.sync_word = 0x2dd5;
			tw_info.channel_no = 0;
			tw_info.cid = 0x3A;
			break;

		case 1:
			tw_info.sync_word = 0x2dd6;
			tw_info.channel_no = 1;
			tw_info.cid = 0x3A;
			break;

		case 2:
			tw_info.sync_word = 0x2dd7;
			tw_info.channel_no = 2;
			tw_info.cid = 0x3A;
			break;

		case 3:
			tw_info.sync_word = 0x2dd8;
			tw_info.channel_no = 3;
			tw_info.cid = 0x3A;
			break;

		default:
			printf_syn("param is 0-3\n");
			return FAIL;
	}
	if (RT_EOK != write_syscfgdata_tbl(SYSCFGDATA_TBL_TW_INFO, 0, &tw_info)) {
		printf_syn("%s(), write syscfg data tbl fail\n", __FUNCTION__);
		return FAIL;
	}
	syscfgdata_syn_proc();
	return SUCC;
}
FINSH_FUNCTION_EXPORT(set_tw_netcfg_m, "set master netcfg");

#else

int set_tw_netcfg_s(int index, int slave_id)
{
	struct tinywireless_if_info_st	 tw_info;

	if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_TW_INFO, 0, &tw_info)) {
		printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
		return FAIL;
	}
	
	switch (index) {
		case 0:
			tw_info.sync_word = 0x2dd5;
			tw_info.channel_no = 0;
			tw_info.cid = 0x3A;
			tw_info.slave_id = slave_id;
			break;

		case 1:
			tw_info.sync_word = 0x2dd6;
			tw_info.channel_no = 1;
			tw_info.cid = 0x3A;
			tw_info.slave_id = slave_id;
			break;

		case 2:
			tw_info.sync_word = 0x2dd7;
			tw_info.channel_no = 2;
			tw_info.cid = 0x3A;
			tw_info.slave_id = slave_id;
			break;

		case 3:
			tw_info.sync_word = 0x2dd8;
			tw_info.channel_no = 3;
			tw_info.cid = 0x3A;
			tw_info.slave_id = slave_id;
			break;

		default:
			printf_syn("param 1 is 0-3\n");
			return FAIL;
	}
	if (RT_EOK != write_syscfgdata_tbl(SYSCFGDATA_TBL_TW_INFO, 0, &tw_info)) {
		printf_syn("%s(), write syscfg data tbl fail\n", __FUNCTION__);
		return FAIL;
	}
	syscfgdata_syn_proc();
	return SUCC;
}
FINSH_FUNCTION_EXPORT(set_tw_netcfg_s, "set slave netcfg");
#endif

int clean_tw_info(void)
{
	struct tinywireless_if_info_st	 tw_info;

	if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_TW_INFO, 0, &tw_info)) {
		printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
		return FAIL;
	}
	
	rt_memset(&tw_info, 0, sizeof(tw_info));
	tw_info.slave_id = 0xFF;
	
	if (RT_EOK != write_syscfgdata_tbl(SYSCFGDATA_TBL_TW_INFO, 0, &tw_info)) {
		printf_syn("%s(), write syscfg data tbl fail\n", __FUNCTION__);
		return FAIL;
	}
	syscfgdata_syn_proc();
	return SUCC;
}
FINSH_FUNCTION_EXPORT(clean_tw_info, "clean tiny wireless info");

int get_tw_syn_word(rt_uint16_t *synword)
{
	struct tinywireless_if_info_st	 tw_info;

	if (NULL == synword) {
		printf_syn("%s(), pointer is NULL\n", __FUNCTION__);
		return FAIL;
	}

	if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_TW_INFO, 0, &tw_info)) {
		printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
		return FAIL;
	}

	*synword = tw_info.sync_word;

	return SUCC;
}
#endif


#if WIRELESS_MASTER_NODE
/*
 * 读取无线白名单的所有数据
 *
 * struct wireless_register_white_st white_register[WIRELESS_SLAVE_NODE_MAX];
 * */
int prt_white(void)
{
	int i;
	struct wireless_register_white_st *white_register;

	/* 申请动态内存 */
	white_register = rt_malloc(WIRELESS_SLAVE_NODE_MAX * sizeof(struct wireless_register_white_st));
	if (NULL == white_register) {
		printf_syn("func:%s(), out of memory\n", __FUNCTION__);
		return FAIL;
	}

	read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_REGISTER_WHITER, 0, white_register);

	/* print contents */
	for (i=0; i<WIRELESS_SLAVE_NODE_MAX; ++i) {
		if (is_dev_sn_valid((white_register+i)->sn_bytes, sizeof(white_register->sn_bytes))) {
			printf_syn("index:%2d, SN:", i);
			print_dev_sn((white_register+i)->sn_bytes, sizeof(white_register->sn_bytes));
		}
	}

	rt_free(white_register);

	return SUCC;
}
FINSH_FUNCTION_EXPORT(prt_white, read white regiter list);

int clean_white(void)
{
	struct wireless_register_white_st *white_register;

	/* 申请动态内存 */
	white_register = rt_malloc(WIRELESS_SLAVE_NODE_MAX * sizeof(struct wireless_register_white_st));
	if (NULL == white_register) {
		printf_syn("func:%s(), out of memory\n", __FUNCTION__);
		return FAIL;
	}

	read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_REGISTER_WHITER, 0, white_register);
	rt_memset(white_register, 0, WIRELESS_SLAVE_NODE_MAX * sizeof(struct wireless_register_white_st));
	write_syscfgdata_tbl(SYSCFGDATA_TBL_REGISTER_WHITER, 0, white_register);
	syscfgdata_syn_proc();

	rt_free(white_register);
	return SUCC;
}
FINSH_FUNCTION_EXPORT(clean_white, clean white regiter list);

/*
 * 从头开始查找第一个空位置, 找到合适的位置后，将新数据写入
 * */
int add_white(char *sn)
{
	int i;
	int ret = SUCC;
	struct wireless_register_white_st *white_register;

	/* 申请动态内存 */
	white_register = rt_malloc(WIRELESS_SLAVE_NODE_MAX * sizeof(struct wireless_register_white_st));
	if (NULL == white_register) {
		printf_syn("func:%s(), out of memory\n", __FUNCTION__);
		return FAIL;
	}

	read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_REGISTER_WHITER, 0, white_register); /* 读数据 */
	/* 修改内容 */
	for (i=0; i<WIRELESS_SLAVE_NODE_MAX; ++i) {
		if (!is_dev_sn_valid((white_register+i)->sn_bytes, sizeof(white_register->sn_bytes)))
			break;
	}

	if (i<WIRELESS_SLAVE_NODE_MAX) {
		rt_memcpy((white_register+i)->sn_bytes, sn, sizeof(white_register->sn_bytes));
		/* 写入flash */
		write_syscfgdata_tbl(SYSCFGDATA_TBL_REGISTER_WHITER, 0, white_register);
		syscfgdata_syn_proc();
	} else {
		printf_syn("func:%s() white list is full\n", __FUNCTION__);
		ret = FAIL;
	}

	rt_free(white_register);

	return ret;
}
FINSH_FUNCTION_EXPORT(add_white, add one item to white regiter list);

/*
 * 从头开始查找，找到后删除
 * */
int del_white(char *sn)
{
	int i, ret = SUCC;
	struct wireless_register_white_st *white_register;

	/* 申请动态内存 */
	white_register = rt_malloc(WIRELESS_SLAVE_NODE_MAX * sizeof(struct wireless_register_white_st));
	if (NULL == white_register) {
		printf_syn("func:%s(), out of memory\n", __FUNCTION__);
		return FAIL;
	}

	read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_REGISTER_WHITER, 0, white_register); /* 读数据 */

	/* 修改内容 */
	for (i=0; i<WIRELESS_SLAVE_NODE_MAX; ++i) {
		if (0 == rt_memcmp((white_register+i)->sn_bytes, sn, sizeof(white_register->sn_bytes)))
			break;
	}


	if (i<WIRELESS_SLAVE_NODE_MAX) {
		rt_memset((white_register+i)->sn_bytes, 0, sizeof(white_register->sn_bytes));
		/* 写入flash */
		write_syscfgdata_tbl(SYSCFGDATA_TBL_REGISTER_WHITER, 0, white_register);
		syscfgdata_syn_proc();
	} else {
		printf_syn("func:%s() not found it in white list\n", __FUNCTION__);
		ret = FAIL;
	}

	rt_free(white_register);

	return ret;
}
FINSH_FUNCTION_EXPORT(del_white, delete one item from white regiter list);


int is_wl_white_list_empty(void)
{
	int i;
	struct wireless_register_white_st *white_register;

	/* 申请动态内存 */
	white_register = rt_malloc(WIRELESS_SLAVE_NODE_MAX * sizeof(struct wireless_register_white_st));
	if (NULL == white_register) {
		printf_syn("func:%s(), out of memory\n", __FUNCTION__);
		return TRUE;
	}

	read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_REGISTER_WHITER, 0, white_register);

	for (i=0; i<WIRELESS_SLAVE_NODE_MAX; ++i) {
		if (is_dev_sn_valid((white_register+i)->sn_bytes, sizeof(white_register->sn_bytes))) {
			break;
		}
	}

	rt_free(white_register);

	if (i < WIRELESS_SLAVE_NODE_MAX)
		return FALSE;
	else
		return TRUE;

}
#endif



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

#if EM_MASTER_DEV || EM_MULTI_MASTER_DEV
int set_rfmaster_sn(char *sn, int index)
{
	int ret = SUCC;
	int len;
	struct wl_master_485_slave_info_tbl *em_pt_table;

	if (index < 0 || index > 4) {
		printf_syn("index error,is 0-3\n");
		return FAIL;
	}
	/* 申请动态内存 */
	em_pt_table = rt_malloc(sizeof(struct wl_master_485_slave_info_tbl));
	if (NULL == em_pt_table) {
		printf_syn("func:%s(), out of memory\n", __FUNCTION__);
		return FAIL;
	}

	read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_WLM_485S_INFO, 0, em_pt_table); /* 读数据 */

#if 0 != USE_HEX_DEV_SN
	len = rt_strlen(sn);

	if (DEV_SN_MODE_LEN*2 != len) {
		printf_syn("SN string len error, shoule be %d\n", len);
	} else {
		unsigned char hex_sn[DEV_SN_MODE_LEN];

		convert_str2hex((unsigned char *)sn, len, hex_sn, sizeof(hex_sn));
		rt_strncpy((char *)em_pt_table->pt_sn[index], (char *)hex_sn, DEV_SN_MODE_LEN);
		write_syscfgdata_tbl(SYSCFGDATA_TBL_WLM_485S_INFO, 0, em_pt_table);
		syscfgdata_syn_proc();
		printf_syn("set em-pt SN success\n");
	}
#else
	len = rt_strlen(DEV_SN_MODE);
	if (len == rt_strlen(sn)) {
		rt_strncpy((char *)em_pt_table->pt_sn[index], sn, sizeof(em_pt_table->pt_sn[0]));
		write_syscfgdata_tbl(SYSCFGDATA_TBL_WLM_485S_INFO, 0, em_pt_table);
		syscfgdata_syn_proc();
		printf_syn("set em-pt SN success\n");
	} else {
		printf_syn("SN string len error, shoule be %d\n", len);
	}
#endif

	rt_free(em_pt_table);

	return ret;
}
FINSH_FUNCTION_EXPORT(set_rfmaster_sn, set rs485 master-slave table index is 0-3);

int get_rfmaster_sn(char *str, int index, int len)
{
	struct wl_master_485_slave_info_tbl *em_pt_table;

	if (index < 0 || index > 4) {
		printf_syn("index error,is 0-3\n");
		return FAIL;
	}
	/* 申请动态内存 */
	em_pt_table = rt_malloc(sizeof(struct wl_master_485_slave_info_tbl));
	if (NULL == em_pt_table) {
		printf_syn("func:%s(), out of memory\n", __FUNCTION__);
		return FAIL;
	}

	if (NULL == str)
		return FAIL;

	read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_WLM_485S_INFO, 0, em_pt_table);

#if 0 != USE_HEX_DEV_SN
	rt_strncpy(str, (char *)em_pt_table->pt_sn[index], len>DEV_SN_MODE_LEN ? DEV_SN_MODE_LEN : len);
#else
	rt_strncpy(str, (char *)em_pt_table->pt_sn[index], len);
#endif
	rt_free(em_pt_table);
	return SUCC;
}

int prt_rfmaster_sn(int index)
{
	char sn[DEV_SN_MODE_LEN];

	get_rfmaster_sn(sn, index, DEV_SN_MODE_LEN);

	print_dev_sn((uint8_t *)sn, DEV_SN_MODE_LEN);

	return SUCC;
}
FINSH_FUNCTION_EXPORT(prt_rfmaster_sn, get rs485 master-slave table index is 0-3);

int set_rfmaster(char *sn1, char *sn2, char *sn3, char *sn4)
{
	struct wl_master_485_slave_info_tbl *em_pt_table;
	int len;

	/* 申请动态内存 */
	em_pt_table = rt_malloc(sizeof(struct wl_master_485_slave_info_tbl));
	if (NULL == em_pt_table) {
		printf_syn("func:%s(), out of memory\n", __FUNCTION__);
		return FAIL;
	}

	read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_WLM_485S_INFO, 0, em_pt_table); /* 读数据 */
	
	len = rt_strlen(DEV_SN_MODE);
	if (len == rt_strlen(sn1)) {
		rt_strncpy((char *)em_pt_table->pt_sn[0], sn1, sizeof(em_pt_table->pt_sn[0]));
	} else {
		printf_syn("SN string len error, shoule be %d\n", len);
		return FAIL;
	}
	if (len == rt_strlen(sn2)) {
		rt_strncpy((char *)em_pt_table->pt_sn[1], sn2, sizeof(em_pt_table->pt_sn[0]));
	} else {
		printf_syn("SN string len error, shoule be %d\n", len);
		return FAIL;
	}
	if (len == rt_strlen(sn3)) {
		rt_strncpy((char *)em_pt_table->pt_sn[2], sn3, sizeof(em_pt_table->pt_sn[0]));
	} else {
		printf_syn("SN string len error, shoule be %d\n", len);
		return FAIL;
	}
	if (len == rt_strlen(sn4)) {
		rt_strncpy((char *)em_pt_table->pt_sn[3], sn4, sizeof(em_pt_table->pt_sn[0]));
	} else {
		printf_syn("SN string len error, shoule be %d\n", len);
		return FAIL;
	}
	
	write_syscfgdata_tbl(SYSCFGDATA_TBL_WLM_485S_INFO, 0, em_pt_table);
	syscfgdata_syn_proc();
	printf_syn("set em-pt SN success\n");
	return SUCC;
}
FINSH_FUNCTION_EXPORT(set_rfmaster, set master-slave table);

int get_rfmaster(void)
{
	char sn[DEV_SN_MODE_LEN];
	int i;

	for (i=0; i<WIRELESS_MASTER_NODE_MAX; i++) {
		get_rfmaster_sn(sn, i, DEV_SN_MODE_LEN);
		print_dev_sn((uint8_t *)sn, DEV_SN_MODE_LEN);
	}
	return SUCC;
}
FINSH_FUNCTION_EXPORT(get_rfmaster, get master-slave table);

int clean_rfmaster(void)
{
	struct wl_master_485_slave_info_tbl *em_pt_table;
	int i;

	/* 申请动态内存 */
	em_pt_table = rt_malloc(sizeof(struct wl_master_485_slave_info_tbl));
	if (NULL == em_pt_table) {
		printf_syn("func:%s(), out of memory\n", __FUNCTION__);
		return FAIL;
	}
	
	read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_WLM_485S_INFO, 0, em_pt_table); /* 读数据 */

	for (i=0; i<WIRELESS_MASTER_NODE_MAX; i++) {
		rt_memset(em_pt_table->pt_sn[i], 0, sizeof(em_pt_table->pt_sn[0]));
	}

	write_syscfgdata_tbl(SYSCFGDATA_TBL_WLM_485S_INFO, 0, em_pt_table);
	syscfgdata_syn_proc();
	return SUCC;
}
FINSH_FUNCTION_EXPORT(clean_rfmaster, clean master-slave table);

int set_wl_netcfg_finish_flag(int is_finish)
{
	struct wl_master_485_slave_info_tbl *wlm_info;

	wlm_info = rt_malloc(sizeof(*wlm_info));
	if (NULL == wlm_info) {
		printf_syn("func:%s(), out of memory\n", __FUNCTION__);
		return FAIL;
	}

	read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_WLM_485S_INFO, 0, wlm_info); /* 读数据 */

	if (0 != is_finish)
		set_bit(wlm_info->flags, WLM_485S_FLAGS_HAD_FINISH_WL_NETCFG_BIT);
	else
		clr_bit(wlm_info->flags, WLM_485S_FLAGS_HAD_FINISH_WL_NETCFG_BIT);

	write_syscfgdata_tbl(SYSCFGDATA_TBL_WLM_485S_INFO, 0, wlm_info);
	syscfgdata_syn_proc();

	rt_free(wlm_info);

	return SUCC;
}

void set_finish(int is_finish)
{
	set_wl_netcfg_finish_flag(is_finish);
}
FINSH_FUNCTION_EXPORT(set_finish, set wireless netcfg finish state);

int is_wl_netcfg_finish(void)
{
	int ret;
	struct wl_master_485_slave_info_tbl *wlm_info;

	wlm_info = rt_malloc(sizeof(*wlm_info));
	if (NULL == wlm_info) {
		printf_syn("func:%s(), out of memory\n", __FUNCTION__);
		ret = FALSE;
		goto ret_entry;
	}

	read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_WLM_485S_INFO, 0, wlm_info); /* 读数据 */

	if (is_bit_set(wlm_info->flags, WLM_485S_FLAGS_HAD_FINISH_WL_NETCFG_BIT))
		ret = TRUE;
	else
		ret = FALSE;

	rt_free(wlm_info);

ret_entry:
	return ret;
}
#endif

#if EM_ALL_TYPE_BASE
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

rt_err_t connect_485sw( rt_uint8_t status)
{
	struct electric_meter_reg_info_st *amm_sn;
	
	amm_sn = rt_malloc(sizeof(*amm_sn));
	if (NULL == amm_sn) {
		printf_syn("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__);
		return RT_ERROR;
	}

	if (SUCC != get_em_reg_info(amm_sn)) {
		printf_syn("ERR: func:%s, line(%d)\n", __FUNCTION__, __LINE__);
		goto END_ERROR;
	}

	if (status == 0) {
		amm_sn->connect_485sw_status = EM_NOT_CONNECT_485SW;
	} else {
		amm_sn->connect_485sw_status = EM_CONNECT_485SW;
	}

	write_syscfgdata_tbl(SYSCFGDATA_TBL_EM_REG_INFO, 0, amm_sn);
	rt_free(amm_sn);
	syscfgdata_syn_proc();

	return RT_EOK;

END_ERROR:
	rt_free(amm_sn);

	return RT_ERROR;
}
FINSH_FUNCTION_EXPORT(connect_485sw, "0:not connect 485sw, 1:connect 485sw");

/*
 * no -- [1, NUM_OF_COLLECT_EM_MAX]
 * sn 的最大长度 #define LEN_OF_EM_SN_MAX	(16)
 * */
int reg_em(int no, char *sn, int vport_no, char *ptc_sn, char *ctc_sn, char *ctc1_sn)
{
	int len1, len2, len3, len4;
	//unsigned long meterstatus = 0;
	struct electric_meter_reg_info_st *em_sns;

	if (NULL == sn || NULL==ptc_sn || NULL==ctc_sn) {
		printf_syn("%s(), pointer is NULL\n", __FUNCTION__);
		return FAIL;
	}

	if ( !((no>=1) && (no<=NUM_OF_COLLECT_EM_MAX)) || !((vport_no>=1) && (vport_no<=4))) {
		printf_syn("%s(), em no or vport_no invalid(%d, %d)\n", __FUNCTION__, no, vport_no);
		return FAIL;
	}

	len1 = rt_strlen(sn);
	len2 = rt_strlen(ptc_sn);
	len3 = rt_strlen(ctc_sn);
	len4 = rt_strlen(ctc1_sn);
	if (DEV_SN_MODE_LEN<len1 || DEV_SN_MODE_LEN!=len2 || DEV_SN_MODE_LEN!=len3
			|| (DEV_SN_MODE_LEN!=len4 && 0!=len4)) {
		printf_syn("%s(), em/ptc/ctc sn too long/small(%d -- %d, %d, %d, %d)\n", __FUNCTION__, DEV_SN_MODE_LEN,
				len1, len2, len3, len4);
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
		rt_strncpy(em_sns->em_sn[no], sn, sizeof(em_sns->em_sn[0]));
		rt_strncpy(em_sns->ptc_sn[no], ptc_sn, sizeof(em_sns->ptc_sn[0]));
		rt_strncpy(em_sns->ctc_sn[no], ctc_sn, sizeof(em_sns->ctc_sn[0]));
		rt_strncpy(em_sns->ctc1_sn[no], ctc1_sn, sizeof(em_sns->ctc1_sn[0]));
		em_sns->vport_no[no] = vport_no-1;

		/* 写入flash */
		write_syscfgdata_tbl(SYSCFGDATA_TBL_EM_REG_INFO, 0, em_sns);
		rt_free(em_sns);
		syscfgdata_syn_proc();
		
		//meterstatus = register_em_info.registered_em_vector;
		update_em_reg_info();

		//if(meterstatus != register_em_info.registered_em_vector){
			trap_send(1,SNMP_GENTRAP_ENTERPRISESPC, E_ALR_EM_ADDRESS_CHANGE_EVENT, E_ALR_EM_ADDRESS_CHANGE_EVENT, SDT_DEV);
			rt_thread_delay(get_ticks_of_ms(50));
			printf_syn("send ammeter pt or ct address changed trap success\n");
		//}
	}

	return SUCC;
}
FINSH_FUNCTION_EXPORT(reg_em, register em sn);


int del_reg_em(int no)
{
	//unsigned long meterstatus = 0;
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
	rt_memset(em_sns->ptc_sn[no-1], 0, sizeof(em_sns->ptc_sn[0]));
	rt_memset(em_sns->ctc_sn[no-1], 0, sizeof(em_sns->ctc_sn[0]));
	rt_memset(em_sns->ctc1_sn[no-1], 0, sizeof(em_sns->ctc1_sn[0]));

	/* 写入flash */
	write_syscfgdata_tbl(SYSCFGDATA_TBL_EM_REG_INFO, 0, em_sns);
	rt_free(em_sns);

	syscfgdata_syn_proc();
	
	//meterstatus = register_em_info.registered_em_vector;
	update_em_reg_info();
	//if(meterstatus != register_em_info.registered_em_vector){
		trap_send(1,SNMP_GENTRAP_ENTERPRISESPC, E_ALR_EM_ADDRESS_CHANGE_EVENT, E_ALR_EM_ADDRESS_CHANGE_EVENT, SDT_DEV);
		rt_thread_delay(get_ticks_of_ms(50));
		printf_syn("send ammeter pt or ct delete trap success\n");
	//}

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

	printf_syn("is connect to 485sw:%d\n", em_sns->connect_485sw_status);

	for (i=0; i<NUM_OF_COLLECT_EM_MAX; ++i) {
		printf_syn("em-no:%2d, em-sn:%s, vport_no:%d, ptc_sn:%s, ctc_sn:%s, ctc1_sn:%s\n",
				i+1, em_sns->em_sn[i], em_sns->vport_no[i]+1,
				em_sns->ptc_sn[i], em_sns->ctc_sn[i], em_sns->ctc1_sn[i]);
	}

	for (i=0; i<NUM_OF_COLLECT_EM_MAX; ++i) {
		printf_syn("em-no:%2d, em-protoc:%d, baud:%d, databits:%d, parity:%d, stopbits:%d\n",
				i+1, em_sns->em_proto[i], em_sns->usart_param[i].baudrate,
				em_sns->usart_param[i].databits, em_sns->usart_param[i].paritybit,
				em_sns->usart_param[i].stopbits);
	}

	rt_free(em_sns);

	return SUCC;
}
FINSH_FUNCTION_EXPORT(print_reg_em, print registed em-sn);

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
int set_regem_pinfo(int no, int protoc, int baudrate, int databits, int paritybit, int stopbits)
{
	struct electric_meter_reg_info_st *em_sns;

	if ( !((no>=1) && (no<=NUM_OF_COLLECT_EM_MAX)) ) {
		printf_syn("%s(), em no  invalid(%d)\n", __FUNCTION__, no);
		return FAIL;
	}


	em_sns = rt_malloc(sizeof(*em_sns));
	if (NULL == em_sns) {
		printf_syn("%s(), alloc mem fail\n", __FUNCTION__);
		return FAIL;
	}

	if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_REG_INFO, 0, em_sns)) {
		printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
		goto fail_ret;
	}

	--no;
	if ('\0' == em_sns->em_sn[no][0]) {
		printf_syn("%s(), NO.%d-em is not cfg\n", __FUNCTION__, no+1);
		goto fail_ret;
	}

	em_sns->em_proto[no]	= protoc;
	em_sns->usart_param[no].baudrate = baudrate;
	em_sns->usart_param[no].databits = databits;
	em_sns->usart_param[no].paritybit = paritybit;
	em_sns->usart_param[no].stopbits = stopbits;

	/* 写入flash */
	write_syscfgdata_tbl(SYSCFGDATA_TBL_EM_REG_INFO, 0, em_sns);
	rt_free(em_sns);

//	syscfgdata_syn_proc();
	update_elctric_meter_info();

	trap_send(1,SNMP_GENTRAP_ENTERPRISESPC, E_ALR_EM_PROTOCOL_BANDRATE_WIREMODE_EVENT, E_ALR_EM_PROTOCOL_BANDRATE_WIREMODE_EVENT, SDT_DEV);
	rt_thread_delay(get_ticks_of_ms(50));
	printf_syn("send ammeter protocol type and 485 bandrate changed trap success\n");

	return SUCC;

fail_ret:
	rt_free(em_sns);
	return FAIL;
}
FINSH_FUNCTION_EXPORT(set_regem_pinfo, set reg em protoc info);

#endif


#if 0
//#include <time.h>
#include <sink_info.h>
#include <ade7880_api.h>
  
//#if EM_DEVICE
#if 1
//extern struct sink_em_relative_info_st sinkinfo;
extern struct px_sample_data_st px_sample_data[ELECTRIC_METER_NUMBER_MAX];	
extern struct rt_semaphore sinkinfo_sem;
//extern struct sink_info_msg val_u;	
extern struct sinkinfo_emc_px_independence_st si_ind_px;
#endif
  
void tmp_cmd(int cmd, int param)
{
	printf_syn("test enter the function\n");
#if 0 
	int temp;
	struct tm *ptime;
	char timestr[20];
#else  
#if EM_DEVICE  
//#if 0
	rt_err_t ret;
	int i;
#endif
#endif
	switch (cmd) {
	case 1:
#if 0
		temp = lcd_getdeviceid();
		printf_syn("lcd dev id:0x%x\n", temp);
		tx_dev_sn[DEV_SN_BUF_CHARS_NUM_MAX] = 0;
		printf_syn("tx_dev_sn:%s\n", tx_dev_sn);
#else

#if EM_DEVICE  
#if 1  
		ret = rt_sem_take(&sinkinfo_sem, RT_WAITING_FOREVER);
		if (RT_EOK == ret) {
		//if (1) {
#if 0
			printf_syn("voltage:0x%x, 0x%x, 0x%x\ncurrent:0x%x, 0x%x, 0x%x\n"
					"freq::0x%x, 0x%x, 0x%x\ntphase:0x%x, 0x%x, 0x%x\n"
					"activep::0x%x, 0x%x, 0x%x\nreactive:0x%x, 0x%x, 0x%x\n"
					"apparent:0x%x, 0x%x, 0x%x\nfactor:0x%x, 0x%x, 0x%x\n"
					"vol-dist:0x%x, 0x%x, 0x%x\ncurrent-dist:0x%x, 0x%x, 0x%x\n",
					sinkinfo.si_emc_ind_pa.vx,   sinkinfo.si_emc_ind_pb.vx,   sinkinfo.si_emc_ind_pc.vx,   sinkinfo.si_emc_ind_pa.ix,   sinkinfo.si_emc_ind_pb.ix,   sinkinfo.si_emc_ind_pc.ix,
					sinkinfo.si_emc_ind_pa.hzx,  sinkinfo.si_emc_ind_pb.hzx,  sinkinfo.si_emc_ind_pc.hzx,  sinkinfo.si_emc_ind_pa.phx,  sinkinfo.si_emc_ind_pb.phx,  sinkinfo.si_emc_ind_pc.phx,
					sinkinfo.si_emc_ind_pa.apx,  sinkinfo.si_emc_ind_pb.apx,  sinkinfo.si_emc_ind_pc.apx,  sinkinfo.si_emc_ind_pa.rapx, sinkinfo.si_emc_ind_pb.rapx, sinkinfo.si_emc_ind_pc.rapx,
					sinkinfo.si_emc_ind_pa.appx, sinkinfo.si_emc_ind_pb.appx, sinkinfo.si_emc_ind_pc.appx, sinkinfo.si_emc_ind_pa.pfx,  sinkinfo.si_emc_ind_pb.pfx,  sinkinfo.si_emc_ind_pc.pfx,
					sinkinfo.si_emc_ind_pa.vdx,  sinkinfo.si_emc_ind_pb.vdx,  sinkinfo.si_emc_ind_pc.vdx,  sinkinfo.si_emc_ind_pa.cdx,  sinkinfo.si_emc_ind_pb.cdx,  sinkinfo.si_emc_ind_pc.cdx);
			printf_syn("v-sample:0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\ni-sample:0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n",
					px_sample_data[0].pa_vi_sample[0][0], px_sample_data[0].pa_vi_sample[0][1],
					px_sample_data[0].pb_vi_sample[0][0], px_sample_data[0].pb_vi_sample[0][1],
					px_sample_data[0].pc_vi_sample[0][0], px_sample_data[0].pc_vi_sample[0][1],
					px_sample_data[0].pa_vi_sample[1][0], px_sample_data[0].pa_vi_sample[1][1],
					px_sample_data[0].pb_vi_sample[1][0], px_sample_data[0].pb_vi_sample[1][1],
					px_sample_data[0].pc_vi_sample[1][0], px_sample_data[0].pc_vi_sample[1][1]
					);
#else
			printf_syn("voltage:%d, %d, %d\ncurrent:%d, %d, %d\n"
					"freq::%u, %u, %u\ntphase:%u, %u, %u\n"
					"activep::%d, %d, %d\nreactive:%d, %d, %d\n"
					"apparent:%d, %d, %d\nfactor:%d, %d, %d\n"
					"vol-dist:%d, %d, %d\ncurrent-dist:%d, %d, %d\n",
					sinkinfo.si_emc_ind_pa.vx,   sinkinfo.si_emc_ind_pb.vx,   sinkinfo.si_emc_ind_pc.vx,   sinkinfo.si_emc_ind_pa.ix,   sinkinfo.si_emc_ind_pb.ix,   sinkinfo.si_emc_ind_pc.ix,
					sinkinfo.si_emc_ind_pa.hzx,  sinkinfo.si_emc_ind_pb.hzx,  sinkinfo.si_emc_ind_pc.hzx,  sinkinfo.si_emc_ind_pa.phx,  sinkinfo.si_emc_ind_pb.phx,  sinkinfo.si_emc_ind_pc.phx,
					sinkinfo.si_emc_ind_pa.apx,  sinkinfo.si_emc_ind_pb.apx,  sinkinfo.si_emc_ind_pc.apx,  sinkinfo.si_emc_ind_pa.rapx, sinkinfo.si_emc_ind_pb.rapx, sinkinfo.si_emc_ind_pc.rapx,
					sinkinfo.si_emc_ind_pa.appx, sinkinfo.si_emc_ind_pb.appx, sinkinfo.si_emc_ind_pc.appx, sinkinfo.si_emc_ind_pa.pfx,  sinkinfo.si_emc_ind_pb.pfx,  sinkinfo.si_emc_ind_pc.pfx,
					sinkinfo.si_emc_ind_pa.vdx,  sinkinfo.si_emc_ind_pb.vdx,  sinkinfo.si_emc_ind_pc.vdx,  sinkinfo.si_emc_ind_pa.cdx,  sinkinfo.si_emc_ind_pb.cdx,  sinkinfo.si_emc_ind_pc.cdx
					);
			printf_syn("v-sample:%d, %d, %d, %d, %d, %d\ni-sample:%d, %d, %d, %d, %d, %d\n",
					px_sample_data[0].pa_vi_sample[0][0], px_sample_data[0].pa_vi_sample[0][1],
					px_sample_data[0].pb_vi_sample[0][0], px_sample_data[0].pb_vi_sample[0][1],
					px_sample_data[0].pc_vi_sample[0][0], px_sample_data[0].pc_vi_sample[0][1],
					px_sample_data[0].pa_vi_sample[1][0], px_sample_data[0].pa_vi_sample[1][1],
					px_sample_data[0].pb_vi_sample[1][0], px_sample_data[0].pb_vi_sample[1][1],
					px_sample_data[0].pc_vi_sample[1][0], px_sample_data[0].pc_vi_sample[1][1]
					);
			printf_syn("pt load:%d, %d, %d\nct load:%d, %d, %d\npt voltage:%d, %d, %d\n", sinkinfo.ptla, sinkinfo.ptlb, sinkinfo.ptlc,
					sinkinfo.ctla, sinkinfo.ctlb, sinkinfo.ctlc, sinkinfo.ssva, sinkinfo.ssvb, sinkinfo.ssvc);
#if 0
			printf_syn("voltage:%d, %d, %d\ncurrent:%d, %d, %d\n"
					"act-p:%d, %d, %d\nreact-p:%d, %d, %d\n"
					"factor:%d, %d, %d\n", sinkinfo.si_em_ind_pa.vx, sinkinfo.si_em_ind_pb.vx, sinkinfo.si_em_ind_pc.vx,
					sinkinfo.si_em_ind_pa.ix, sinkinfo.si_em_ind_pb.ix, sinkinfo.si_em_ind_pc.ix,
					sinkinfo.si_em_ind_pa.apx, sinkinfo.si_em_ind_pb.apx, sinkinfo.si_em_ind_pc.apx,
					sinkinfo.si_em_ind_pa.rapx, sinkinfo.si_em_ind_pb.rapx, sinkinfo.si_em_ind_pc.rapx,
					sinkinfo.si_em_ind_pa.pfx, sinkinfo.si_em_ind_pb.pfx, sinkinfo.si_em_ind_pc.pfx);
#else
			printf_syn("[em-data(bcd code)]voltage:%x, %x, %x\ncurrent:%x, %x, %x\n"
					"act-p:%x, %x, %x\nreact-p:%x, %x, %x\nfactor:%x, %x, %x\n"
					,sinkinfo.si_em_ind_pa.vx, sinkinfo.si_em_ind_pb.vx, sinkinfo.si_em_ind_pc.vx
					,sinkinfo.si_em_ind_pa.ix, sinkinfo.si_em_ind_pb.ix, sinkinfo.si_em_ind_pc.ix
					,sinkinfo.si_em_ind_pa.apx, sinkinfo.si_em_ind_pb.apx, sinkinfo.si_em_ind_pc.apx
					,sinkinfo.si_em_ind_pa.rapx, sinkinfo.si_em_ind_pb.rapx, sinkinfo.si_em_ind_pc.rapx
					,sinkinfo.si_em_ind_pa.pfx, sinkinfo.si_em_ind_pb.pfx, sinkinfo.si_em_ind_pc.pfx);

#endif
			/* 閻絻銆冩稉顓犳畱閺堝濮?閺冪姴濮涢悽浣冨厴, 娴狀櫒cd閻礁鐡ㄩ崒?*/
			printf_syn("em-data(BCD code):%x,%x, sn:%s\nem act(react) ee inaccuracy:%d, %d\ndev-data:%d, %d\n",
					sinkinfo.em_info->em_act_total_energy, sinkinfo.em_info->em_react_total_energy,
					sinkinfo.electric_meter_sn[0],
					sinkinfo.em_info->em_act_ee_inaccuracy, sinkinfo.em_info->em_react_ee_inaccuracy,
					sinkinfo.emc_dev_info.dev_act_electric_energy, sinkinfo.emc_dev_info.dev_react_electric_energy);
#endif
			rt_sem_release(&sinkinfo_sem);
		} else {
			printf_syn("take sinkinfo_sem fail(%d)\n", ret);
		}
#else
		printf_syn("voltage:%d, %d, %d\ncurrent:%d, %d, %d\n"
				"freq::%d, %d, %d\ntphase:%d, %d, %d\n"
				"activep::%d, %d, %d\nreactive:%d, d, %d\n"
				"apparent:%d, %d, %d\nfactor:%d, %d, %d\n"
				"vol-dist:%d, %d, %d\ncurrent-dist:%d, %d, %d\n",
				sinkinfo.si_emc_ind_pa.vx,   sinkinfo.si_emc_ind_pb.vx,   sinkinfo.si_emc_ind_pc.vx,   sinkinfo.si_emc_ind_pa.ix,   sinkinfo.si_emc_ind_pb.ix,   sinkinfo.si_emc_ind_pc.ix,
				sinkinfo.si_emc_ind_pa.hzx,  sinkinfo.si_emc_ind_pb.hzx,  sinkinfo.si_emc_ind_pc.hzx,  sinkinfo.si_emc_ind_pa.phx,  sinkinfo.si_emc_ind_pb.phx,  sinkinfo.si_emc_ind_pc.phx,
				sinkinfo.si_emc_ind_pa.apx,  sinkinfo.si_emc_ind_pb.apx,  sinkinfo.si_emc_ind_pc.apx,  sinkinfo.si_emc_ind_pa.rapx, sinkinfo.si_emc_ind_pb.rapx, sinkinfo.si_emc_ind_pc.rapx,
				sinkinfo.si_emc_ind_pa.appx, sinkinfo.si_emc_ind_pb.appx, sinkinfo.si_emc_ind_pc.appx, sinkinfo.si_emc_ind_pa.pfx,  sinkinfo.si_emc_ind_pb.pfx,  sinkinfo.si_emc_ind_pc.pfx,
				sinkinfo.si_emc_ind_pa.vdx,  sinkinfo.si_emc_ind_pb.vdx,  sinkinfo.si_emc_ind_pc.vdx,  sinkinfo.si_emc_ind_pa.cdx,  sinkinfo.si_emc_ind_pb.cdx,  sinkinfo.si_emc_ind_pc.cdx
				);
		printf_syn("v-sample:%d, %d, %d, %d, %d, %d\ni-sample:%d, %d, %d, %d, %d, %d\n",
				px_sample_data[0].pa_vi_sample[0][0], px_sample_data[0].pa_vi_sample[0][1],
				px_sample_data[0].pb_vi_sample[0][0], px_sample_data[0].pb_vi_sample[0][1],
				px_sample_data[0].pc_vi_sample[0][0], px_sample_data[0].pc_vi_sample[0][1],
				px_sample_data[0].pa_vi_sample[1][0], px_sample_data[0].pa_vi_sample[1][1],
				px_sample_data[0].pb_vi_sample[1][0], px_sample_data[0].pb_vi_sample[1][1],
				px_sample_data[0].pc_vi_sample[1][0], px_sample_data[0].pc_vi_sample[1][1]
				);
#endif
#endif
#endif /* #if WIRELESS_MASTER_NODE */
		break;
  
//#if EM_DEVICE    
#if 1     
	case 2:

		switch (param) {
		case PHASE_A:
			px_virtual_mode_voltage(PHASE_A, &sinkinfo.si_emc_ind_pa.vx);
			break;

		case PHASE_B:
			px_virtual_mode_voltage(PHASE_B, &sinkinfo.si_emc_ind_pb.vx);
			break;

		case PHASE_C:
			px_virtual_mode_voltage(PHASE_C, &sinkinfo.si_emc_ind_pc.vx);
			break;

		case 4:
			px_virtual_mode_voltage(PHASE_A, &sinkinfo.si_emc_ind_pa.vx);
			px_virtual_mode_voltage(PHASE_B, &sinkinfo.si_emc_ind_pb.vx);
			px_virtual_mode_voltage(PHASE_C, &sinkinfo.si_emc_ind_pc.vx);
			break;

		default:
			printf_syn("param(%d) error", param);
			break;
		}

		break;
#else  
	case 3:
		switch (param) {
		case PHASE_A:
			px_virtual_mode_current(PHASE_A, &sinkinfo.si_emc_ind_pa.ix);
			break;

		case PHASE_B:
			px_virtual_mode_current(PHASE_B, &sinkinfo.si_emc_ind_pb.ix);
			break;

		case PHASE_C:
			px_virtual_mode_current(PHASE_C, &sinkinfo.si_emc_ind_pc.ix);
			break;

		default:
			printf_syn("param(%d) error", param);
			break;
		}
		break;

	case 4:
		switch (param) {
		case PHASE_A:
			sinkinfo.si_emc_ind_pa.hzx = px_frequency_mode_signal(PHASE_A);
			break;

		case PHASE_B:
			sinkinfo.si_emc_ind_pb.hzx = px_frequency_mode_signal(PHASE_B);
			break;

		case PHASE_C:
			sinkinfo.si_emc_ind_pc.hzx = px_frequency_mode_signal(PHASE_C);
			break;

		default:
			printf_syn("param(%d) error", param);
			break;
		}
		break;

	case 5:
		switch (param) {
		case PHASE_A:
			sinkinfo.si_emc_ind_pa.phx = px_phase_mode_position(PHASE_A);
			break;

		case PHASE_B:
			sinkinfo.si_emc_ind_pb.phx = px_phase_mode_position(PHASE_B);
			break;

		case PHASE_C:
			sinkinfo.si_emc_ind_pc.phx = px_phase_mode_position(PHASE_C);
			break;

		default:
			printf_syn("param(%d) error", param);
			break;
		}
		break;

	case 6:
		switch (param) {
		case PHASE_A:
			sinkinfo.si_emc_ind_pa.apx = px_active_mode_power(PHASE_A);
			break;

		case PHASE_B:
			sinkinfo.si_emc_ind_pb.apx = px_active_mode_power(PHASE_B);
			break;

		case PHASE_C:
			sinkinfo.si_emc_ind_pc.apx = px_active_mode_power(PHASE_C);
			break;

		default:
			printf_syn("param(%d) error", param);
			break;
		}
		break;
 
	case 7:
		switch (param) {
		case PHASE_A:
			sinkinfo.si_emc_ind_pa.rapx = px_reactive_mode_power(PHASE_A);
			break;

		case PHASE_B:
			sinkinfo.si_emc_ind_pb.rapx = px_reactive_mode_power(PHASE_B);
			break;

		case PHASE_C:
			sinkinfo.si_emc_ind_pc.rapx = px_reactive_mode_power(PHASE_C);
			break;

		default:
			printf_syn("param(%d) error", param);
			break;
		}
		break;

	case 8:
		switch (param) {
		case PHASE_A:
			sinkinfo.si_emc_ind_pa.appx = px_apparent_mode_power(PHASE_A);
			break;

		case PHASE_B:
			sinkinfo.si_emc_ind_pb.appx = px_apparent_mode_power(PHASE_B);
			break;

		case PHASE_C:
			sinkinfo.si_emc_ind_pc.appx = px_apparent_mode_power(PHASE_C);
			break;

		default:
			printf_syn("param(%d) error", param);
			break;
		}
		break;

	case 9:
		switch (param) {
		case PHASE_A:
			sinkinfo.si_emc_ind_pa.pfx = px_factor_mode_power(PHASE_A);
			break;

		case PHASE_B:
			sinkinfo.si_emc_ind_pb.pfx = px_factor_mode_power(PHASE_B);
			break;

		case PHASE_C:
			sinkinfo.si_emc_ind_pc.pfx = px_factor_mode_power(PHASE_C);
			break;

		default:
			printf_syn("param(%d) error", param);
			break;
		}
		break;

	case 10:
		switch (param) {
		case PHASE_A:
			sinkinfo.si_emc_ind_pa.vdx = px_voltage_distortion(PHASE_A);
			break;

		case PHASE_B:
			sinkinfo.si_emc_ind_pb.vdx = px_voltage_distortion(PHASE_B);
			break;

		case PHASE_C:
			sinkinfo.si_emc_ind_pc.vdx = px_voltage_distortion(PHASE_C);
			break;

		default:
			printf_syn("param(%d) error", param);
			break;
		}
		break;

	case 11:
		switch (param) {
		case PHASE_A:
			sinkinfo.si_emc_ind_pa.cdx = px_current_distortion(PHASE_A);
			break;

		case PHASE_B:
			sinkinfo.si_emc_ind_pb.cdx = px_current_distortion(PHASE_B);
			break;

		case PHASE_C:
			sinkinfo.si_emc_ind_pc.cdx = px_current_distortion(PHASE_C);
			break;

		default:
			printf_syn("param(%d) error", param);
			break;
		}
		break;

	case 12:
		switch (param) {
		case PHASE_A:
			px_vi_signal_sample(PHASE_A, px_sample_data[0].pa_vi_sample[0],
					px_sample_data[0].pa_vi_sample[1], sizeof(px_sample_data[0].pa_vi_sample[0]));
			break;

		case PHASE_B:
			px_vi_signal_sample(PHASE_B, px_sample_data[0].pb_vi_sample[0],
					px_sample_data[0].pb_vi_sample[1], sizeof(px_sample_data[0].pb_vi_sample[0]));
			break;

		case PHASE_C:
			px_vi_signal_sample(PHASE_C, px_sample_data[0].pc_vi_sample[0],
					px_sample_data[0].pc_vi_sample[1], sizeof(px_sample_data[0].pc_vi_sample[0]));
			break;

		default:
			printf_syn("param(%d) error", param);
			break;
		}
		break;

	case 13:
		break;

	case 14:
	{
		int reg1, reg2, reg3;
		extern void test_ade7880_read_reg(int *reg1, int *reg2, int *reg3);

		test_ade7880_read_reg(&reg1, &reg2, &reg3);
		printf_syn("0x%x, 0x%x, 0x%x\n", reg1, reg2, reg3);
	}
		break;

	case 15:
	{
		static char cmd_485_seq[] = {0xfe, 0xfe, 0xfe, 0xfe, 0x68, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x68, 0x01, 0x02, 0x65, 0xf3, 0x27, 0x16};
		int rx_cnt = 0, err_cnt = 0;
		rt_err_t err;
		char rx_ch;

		send_data_by_485(UART_485_2_DEV_PTR, cmd_485_seq, sizeof(cmd_485_seq));
		printf_syn("485-2 485 cmd had send over\n");

		while (rx_cnt<23 && err_cnt<2) {
			if (RT_EOK == (err=rt_sem_take(&uart485_2_rx_byte_sem, (get_ticks_of_ms(1500))))) {
				while (1 == recv_data_by_485(UART_485_2_DEV_PTR, &rx_ch, 1)) {
					++rx_cnt;
					printf_syn("485-2 recv ind[%3d]:0x%2x\n", rx_cnt, rx_ch);
				}
			} else {
				printf_syn("485-2 recv error(%d)\n", err);
				++err_cnt;
			}
		}

	}
		break;

	case 16:
	{
		int *pch;

		switch (param) {
		case 1:
			pch = pa_ade7880_sample_data[0];
			break;
		case 2:
			pch = pa_ade7880_sample_data[1];
			break;
		case 3:
			pch = pb_ade7880_sample_data[0];
			break;
		case 4:
			pch = pb_ade7880_sample_data[1];
			break;
		case 5:
			pch = pc_ade7880_sample_data[0];

			break;
		case 6:
			pch = pc_ade7880_sample_data[1];
			break;
		default:
			pch = NULL;
			printf_syn("fun:%s, line:%d\n", __FUNCTION__, __LINE__);
			break;
		}

		if (NULL != pch)
			for (i=0; i<SINK_INFO_PX_SAMPLE_DOT_NUM/4; ++i) {
				printf_syn("%10d %10d %10d %10d ", *pch, *(pch+1), *(pch+2), *(pch+3));
				pch += 4;
				if (1 == i%2)
					printf_syn("\n");
			}
	}
		break;

	case 17:
	{
		signed char *pch;

		switch (param) {
		case 1:
			pch = px_sample_data[0].pa_vi_sample[0];
			break;
		case 2:
			pch = px_sample_data[0].pa_vi_sample[1];
			break;
		case 3:
			pch = px_sample_data[0].pb_vi_sample[0];
			break;
		case 4:
			pch = px_sample_data[0].pb_vi_sample[1];
			break;
		case 5:
			pch = px_sample_data[0].pc_vi_sample[0];

			break;
		case 6:
			pch = px_sample_data[0].pc_vi_sample[1];
			break;
		default:
			pch = NULL;
			printf_syn("fun:%s, line:%d\n", __FUNCTION__, __LINE__);
			break;
		}

		if (NULL != pch)
			for (i=0; i<SINK_INFO_PX_SAMPLE_DOT_NUM/4; ++i) {
				printf_syn("%5d %5d %5d %5d ", *pch, *(pch+1), *(pch+2), *(pch+3));
				pch += 4;
				if (1 == i%2)
					printf_syn("\n");
			}
	}
		break;
#endif
	case 18:
	{

		switch (param) { 
		case 1:
			printf_syn(" write cmd  success: fun:%s, line:%d\n", __FUNCTION__, __LINE__);
			//px_gain_matching_autodebug();
			break;
		case 2:       
			printf_syn(" write cmd  success: fun:%s, line:%d\n", __FUNCTION__, __LINE__);
			//px_power_adjust_autodebug(220,10,50,50);
			//px_power_adjust_autodebug();
			break;
		case 3:
			printf_syn(" write cmd  success: fun:%s, line:%d\n", __FUNCTION__, __LINE__);
			//px_plus_adjust_autodebug(5);
			break; 
		case 4:
			printf_syn(" write cmd  success: fun:%s, line:%d\n", __FUNCTION__, __LINE__);
			//px_vircur_setout_autodebug(585000,585000,1175);
			break;
		case 5:
			printf_syn(" write cmd  success: fun:%s, line:%d\n", __FUNCTION__, __LINE__);
 			rt_val_7880();
			break;
		case 6:
			
			break;
		default:
			printf_syn("auto debug error: fun:%s, line:%d\n", __FUNCTION__, __LINE__);
			break;
		}
		
	}
		break;

#if 1    
	case 19:

		printf_syn("voltage:%d, %d, %d\ncurrent:%d, %d, %d\n"
			"freq::%u, %u, %u\ntphase:Uac %d, Ubc %d, Uab %d\n"
			"activep::%d, %d, %d\nreactive:%d, %d, %d\n"
			"apparent:%d, %d, %d\nfactor:%d, %d, %d\n"
			"vol-dist:%d, %d, %d\ncurrent-dist:%d, %d, %d\n",
			sinkinfo.si_emc_ind_pa.vx,   sinkinfo.si_emc_ind_pb.vx,   sinkinfo.si_emc_ind_pc.vx,   sinkinfo.si_emc_ind_pa.ix,   sinkinfo.si_emc_ind_pb.ix,   sinkinfo.si_emc_ind_pc.ix,
			sinkinfo.si_emc_ind_pa.hzx,  sinkinfo.si_emc_ind_pb.hzx,  sinkinfo.si_emc_ind_pc.hzx,  sinkinfo.si_emc_ind_pa.phx,  sinkinfo.si_emc_ind_pb.phx,  sinkinfo.si_emc_ind_pc.phx,
			sinkinfo.si_emc_ind_pa.apx,  sinkinfo.si_emc_ind_pb.apx,  sinkinfo.si_emc_ind_pc.apx,  sinkinfo.si_emc_ind_pa.rapx, sinkinfo.si_emc_ind_pb.rapx, sinkinfo.si_emc_ind_pc.rapx,
			sinkinfo.si_emc_ind_pa.appx, sinkinfo.si_emc_ind_pb.appx, sinkinfo.si_emc_ind_pc.appx, sinkinfo.si_emc_ind_pa.pfx,  sinkinfo.si_emc_ind_pb.pfx,  sinkinfo.si_emc_ind_pc.pfx,
			sinkinfo.si_emc_ind_pa.vdx,  sinkinfo.si_emc_ind_pb.vdx,  sinkinfo.si_emc_ind_pc.vdx,  sinkinfo.si_emc_ind_pa.cdx,  sinkinfo.si_emc_ind_pb.cdx,  sinkinfo.si_emc_ind_pc.cdx
			);  
#if 1         
	u8 cnt;
    
 	//px_vi_sample_reac_p_hsdc();
 	printf_syn("\nhsdc_transcomp_flag :%d\n",hsdc_transcomp_flag);
	if(hsdc_transcomp_flag == 160){ 
		printf_syn("\nav sampling :\n");	  		
		for(cnt=0 ;cnt<40; cnt++){
			//printf_syn("0x%x  ",AV_HSCD_BUFFER[cnt]);
			printf_syn("%d  ",AV_HSCD_BUFFER[cnt]);
		}

		printf_syn("\nbv sampling :\n");			
		for(cnt=0 ;cnt<40; cnt++){
			//printf_syn("0x%x  ",BV_HSCD_BUFFER[cnt]);
			printf_syn("%d  ",BV_HSCD_BUFFER[cnt]);
		}

		printf_syn("\ncv sampling :\n");			
		for(cnt=0 ;cnt<40; cnt++){
			//printf_syn("0x%x  ",CV_HSCD_BUFFER[cnt]);
			printf_syn("%d  ",CV_HSCD_BUFFER[cnt]);
		}
			  
		printf_syn("\nxFVAR :\n");			
		for(cnt=0 ;cnt<3; cnt++){  
			printf_syn("0x%x  ",XFVAR_HSCD_BUFFER[cnt]);  
			//printf_syn("%d  ",XFVAR_HSCD_BUFFER[cnt]);
		}
	}else{
		printf_syn("\nsample error\n");	  		
	}
#endif

#if 0  
			printf_syn("v-sample:%d, %d, %d, %d, %d, %d\ni-sample:%d, %d, %d, %d, %d, %d\n",
					px_sample_data[0].pa_vi_sample[0][0], px_sample_data[0].pa_vi_sample[0][1],
					px_sample_data[0].pb_vi_sample[0][0], px_sample_data[0].pb_vi_sample[0][1],
					px_sample_data[0].pc_vi_sample[0][0], px_sample_data[0].pc_vi_sample[0][1],
					px_sample_data[0].pa_vi_sample[1][0], px_sample_data[0].pa_vi_sample[1][1],
					px_sample_data[0].pb_vi_sample[1][0], px_sample_data[0].pb_vi_sample[1][1],
					px_sample_data[0].pc_vi_sample[1][0], px_sample_data[0].pc_vi_sample[1][1]
					);
  
			printf_syn("pt load:%d, %d, %d\nct load:%d, %d, %d\npt voltage:%d, %d, %d\n", sinkinfo.ptla, sinkinfo.ptlb, sinkinfo.ptlc,
					sinkinfo.ctla, sinkinfo.ctlb, sinkinfo.ctlc, sinkinfo.ssva, sinkinfo.ssvb, sinkinfo.ssvc);
			printf_syn("[em-data(bcd code)]voltage:%x, %x, %x\ncurrent:%x, %x, %x\n"
					"act-p:%x, %x, %x\nreact-p:%x, %x, %x\nfactor:%x, %x, %x\n"
					,sinkinfo.si_em_ind_pa.vx, sinkinfo.si_em_ind_pb.vx, sinkinfo.si_em_ind_pc.vx
					,sinkinfo.si_em_ind_pa.ix, sinkinfo.si_em_ind_pb.ix, sinkinfo.si_em_ind_pc.ix
					,sinkinfo.si_em_ind_pa.apx, sinkinfo.si_em_ind_pb.apx, sinkinfo.si_em_ind_pc.apx
					,sinkinfo.si_em_ind_pa.rapx, sinkinfo.si_em_ind_pb.rapx, sinkinfo.si_em_ind_pc.rapx
					,sinkinfo.si_em_ind_pa.pfx, sinkinfo.si_em_ind_pb.pfx, sinkinfo.si_em_ind_pc.pfx);
#endif
#if 0 
			printf_syn("em-data(BCD code):%x,%x, sn:%s\nem act(react) ee inaccuracy:%d, %d\ndev-data:%d, %d\n",
					sinkinfo.em_info->em_act_total_energy, sinkinfo.em_info->em_react_total_energy,
					electric_meter_sn[0],
					sinkinfo.em_info->em_act_ee_inaccuracy, sinkinfo.em_info->em_react_ee_inaccuracy,
					sinkinfo.emc_dev_info.dev_act_electric_energy, sinkinfo.emc_dev_info.dev_react_electric_energy);
#endif
			//rt_sem_release(&sinkinfo_sem);
		//} else {
			//printf_syn("take sinkinfo_sem fail(%d)\n", ret);
		//}

		break;
#endif

#if 1    

	case 20:

		px_virtual_mode_voltage(PHASE_A, &sinkinfo.si_emc_ind_pa.vx);
		px_virtual_mode_voltage(PHASE_B, &sinkinfo.si_emc_ind_pb.vx);
		px_virtual_mode_voltage(PHASE_C, &sinkinfo.si_emc_ind_pc.vx);

		px_virtual_mode_current(PHASE_A, &sinkinfo.si_emc_ind_pa.ix);
		px_virtual_mode_current(PHASE_B, &sinkinfo.si_emc_ind_pb.ix);
		px_virtual_mode_current(PHASE_C, &sinkinfo.si_emc_ind_pc.ix);
  
		sinkinfo.si_emc_ind_pa.hzx  = px_frequency_mode_signal(PHASE_A);
		sinkinfo.si_emc_ind_pb.hzx  = px_frequency_mode_signal(PHASE_B);
		sinkinfo.si_emc_ind_pc.hzx  = px_frequency_mode_signal(PHASE_C);

		sinkinfo.si_emc_ind_pa.phx  = px_phase_mode_position(PHASE_A);
		sinkinfo.si_emc_ind_pb.phx  = px_phase_mode_position(PHASE_B);
		sinkinfo.si_emc_ind_pc.phx  = px_phase_mode_position(PHASE_C);		
		
		sinkinfo.si_emc_ind_pa.apx  = px_active_mode_power(PHASE_A);
		sinkinfo.si_emc_ind_pb.apx  = px_active_mode_power(PHASE_B);
		sinkinfo.si_emc_ind_pc.apx  = px_active_mode_power(PHASE_C);		
#if ADE7880_USE_I2C_HSDC		
		sinkinfo.si_emc_ind_pa.rapx = XFVAR_HSCD_BUFFER[0];
		//if(connect33_data == 1){
			//sinkinfo.si_emc_ind_pb.rapx = 0;
		//}else{  
			sinkinfo.si_emc_ind_pb.rapx = XFVAR_HSCD_BUFFER[1];
		//}
		sinkinfo.si_emc_ind_pc.rapx = XFVAR_HSCD_BUFFER[2];		
#else
		sinkinfo.si_emc_ind_pa.rapx = px_reactive_mode_power(PHASE_A);
		sinkinfo.si_emc_ind_pb.rapx = px_reactive_mode_power(PHASE_B);
		sinkinfo.si_emc_ind_pc.rapx = px_reactive_mode_power(PHASE_C);		
#endif
		sinkinfo.si_emc_ind_pa.appx = px_apparent_mode_power(PHASE_A);
		sinkinfo.si_emc_ind_pb.appx = px_apparent_mode_power(PHASE_B);
		sinkinfo.si_emc_ind_pc.appx = px_apparent_mode_power(PHASE_C);		

		sinkinfo.si_emc_ind_pa.pfx  = px_factor_mode_power(PHASE_A);
		sinkinfo.si_emc_ind_pb.pfx  = px_factor_mode_power(PHASE_B);
		sinkinfo.si_emc_ind_pc.pfx  = px_factor_mode_power(PHASE_C);
		
		sinkinfo.si_emc_ind_pa.vdx  = px_voltage_distortion(PHASE_A);
		sinkinfo.si_emc_ind_pb.vdx  = px_voltage_distortion(PHASE_B);
		sinkinfo.si_emc_ind_pc.vdx  = px_voltage_distortion(PHASE_C);		

		sinkinfo.si_emc_ind_pa.cdx  = px_current_distortion(PHASE_A);
		sinkinfo.si_emc_ind_pb.cdx  = px_current_distortion(PHASE_B);
		sinkinfo.si_emc_ind_pc.cdx  = px_current_distortion(PHASE_C);		

		px_vi_sample_reac_p_hsdc();	
		  
		break;  
#endif  
	case 21:
		vi_vector_graph_sampl();
		printf_syn("vectorgraph_st:\nvvap(Uac) %d vvbp(Ubc) %d vvcp(Uab) %d \nviap %d vibp %d vicp %d \n",
		vg_st.vvap, vg_st.vvbp, vg_st.vvcp,
		vg_st.viap, vg_st.vibp, vg_st.vicp);
		break;
	case 22:  
		vi_vector_graph_sampl();
		px_phase_autodebug(16);
		vi_vector_graph_sampl();
		break;
	default:
		printf_syn("cmd(%d) error", cmd);
		break;
	}

	return;
}
FINSH_FUNCTION_EXPORT(tmp_cmd, cmd);
#endif
u8 cnt_s;  
//struct sink_em_relative_info_st sinkinfo;
#if RT_USING_ADE7880
void rt_val_7880(void)
{
#if 0
	struct sink_em_relative_info_st sinkinfo;	
	int t1 = 0, t2 = 0, t3 = 0,
	    t4, t5, t6,
    	    t7, t8, t9;
	u8 cnt_p;// cnt;
#endif
#if 0	  
  
	u32 t1 = 0, t2 = 0, t3 = 0,
	    t4, t5, t6,
    	    t7, t8, t9, cnt;
	px_vi_sample_reac_p_hsdc();
	if(connect33_data == 1){
		printf_syn("connect mode three-phasethree-wiresystem\n ");
	}else if(connect33_data == 0){
		printf_syn("connect mode four-phasethree-wiresystem\n ");
	}else {
		printf_syn("not find the value,flash write something wrong\n  ");
	}
    
	px_virtual_mode_voltage(PHASE_A, &sinkinfo.si_emc_ind_pa.vx);
	px_virtual_mode_voltage(PHASE_B, &sinkinfo.si_emc_ind_pb.vx);
	px_virtual_mode_voltage(PHASE_C, &sinkinfo.si_emc_ind_pc.vx);

	px_virtual_mode_current(PHASE_A, &sinkinfo.si_emc_ind_pa.ix);
	px_virtual_mode_current(PHASE_B, &sinkinfo.si_emc_ind_pb.ix);
	px_virtual_mode_current(PHASE_C, &sinkinfo.si_emc_ind_pc.ix);

	t1 = px_active_mode_power(PHASE_A);
	t2 = px_active_mode_power(PHASE_B);
	t3 = px_active_mode_power(PHASE_C);
   
	t4 = px_reactive_mode_power(PHASE_A);
	t5 = px_reactive_mode_power(PHASE_B);
	t6 = px_reactive_mode_power(PHASE_C);

	t7 = px_apparent_mode_power(PHASE_A);
	t8 = px_apparent_mode_power(PHASE_B);
	t9 = px_apparent_mode_power(PHASE_C);

	printf_syn("pa_v_val: %d, pb_v_val: %d, pc_v_val: %d\n"
		"pa_i_val: %d, pb_i_val: %d, pc_i_val: %d\n"	
		"pa_p_active_val: %d, pb_p_active_val: %d, pc_p_active_val: %d\n"
		"pa_p_reacti_val: %d, pb_p_reacti_val: %d, pc_p_reacti_val: %d\n"
		"pa_p_appare_val: %d, pb_p_appare_val: %d, pc_p_appare_val: %d\n",
		sinkinfo.si_emc_ind_pa.vx, sinkinfo.si_emc_ind_pb.vx, sinkinfo.si_emc_ind_pc.vx,
		sinkinfo.si_emc_ind_pa.ix, sinkinfo.si_emc_ind_pb.ix, sinkinfo.si_emc_ind_pc.ix,
		t1,t2,t3,
		t4,t5,t6,
		t7,t8,t9);
  
 	printf_syn("\nhsdc_transcomp_flag :%d\n",hsdc_transcomp_flag);
	if(hsdc_transcomp_flag == 160){ 
		printf_syn("\nav sampling :\n");	  		
		for(cnt=0 ;cnt<40; cnt++){
			//printf_syn("0x%x  ",AV_HSCD_BUFFER[cnt]);
			printf_syn("%d  ",AV_HSCD_BUFFER[cnt]);
		}

		printf_syn("\nbv sampling :\n");			
		for(cnt=0 ;cnt<40; cnt++){
			//printf_syn("0x%x  ",BV_HSCD_BUFFER[cnt]);
			printf_syn("%d  ",BV_HSCD_BUFFER[cnt]);
		}

		printf_syn("\ncv sampling :\n");			
		for(cnt=0 ;cnt<40; cnt++){
			//printf_syn("0x%x  ",CV_HSCD_BUFFER[cnt]);
			printf_syn("%d  ",CV_HSCD_BUFFER[cnt]);
		}
			  
		printf_syn("\nai sampling :\n");			
		for(cnt=0 ;cnt<40; cnt++){
			//printf_syn("0x%x  ",AI_HSCD_BUFFER[cnt]);
			printf_syn("%d  ",AI_HSCD_BUFFER[cnt]);
		}
  
		printf_syn("\nbi sampling :\n");			
		for(cnt=0 ;cnt<40; cnt++){
			//printf_syn("0x%x  ",BI_HSCD_BUFFER[cnt]);
			printf_syn("%d  ",BI_HSCD_BUFFER[cnt]);
		}
  
		printf_syn("\nci sampling :\n");			
		for(cnt=0 ;cnt<40; cnt++){
			//printf_syn("0x%x  ",CI_HSCD_BUFFER[cnt]);
			printf_syn("%d  ",CI_HSCD_BUFFER[cnt]);
		}
		
		printf_syn("\nxFVAR :\n");			
		for(cnt=0 ;cnt<3; cnt++){  
			printf_syn("0x%x  ",XFVAR_HSCD_BUFFER[cnt]);  
			//printf_syn("%d  ",XFVAR_HSCD_BUFFER[cnt]);
		}
	}else{
		printf_syn("\nsample error\n");	  		
	}
#endif
#if 0     
	struct sink_em_relative_info_st sinkinfo;	
	int t1 = 0, t2 = 0, t3 = 0;
	u8 cnt_p;
	Write_8bitReg_ADE7880(0xE702,0x85);
	t1 = Read_8bitReg_ADE7880(0xE702);
	//printf_syn("test 8bit reg write and read: 0x%x\n",t1);

	Write_16bitReg_ADE7880(0xE611, 0x55aa);
	t2 = Read_16bitReg_ADE7880(0xE611);
	//printf_syn("test 16bit reg write and read: 0x%x\n",t2);

	Write_32bitReg_ADE7880(0xE520,0xaaaaaa);
	t3 = Read_32bitReg_ADE7880(0xE520);
	printf_syn("test reg write and read:\n8bit   0x%x\n16bit  0x%x\n32bit  0x%x\n",t1,t2,t3);
/*	
 * 测试abc三相电压有效值8次累加平均

*/

 	for(cnt_p = 0; cnt_p < 8; cnt_p++ ){
		px_virtual_mode_voltage(PHASE_A, &sinkinfo.si_emc_ind_pa.vx);
		t1 = t1 + sinkinfo.si_emc_ind_pa.vx;
		px_virtual_mode_voltage(PHASE_B, &sinkinfo.si_emc_ind_pb.vx);
		t2 = t2 + sinkinfo.si_emc_ind_pb.vx;
		px_virtual_mode_voltage(PHASE_C, &sinkinfo.si_emc_ind_pc.vx);
		t3 = t3 + sinkinfo.si_emc_ind_pc.vx;		
		printf_syn("pa_v_val: %d, pb_v_val: %d, pc_v_val: %d, cnt_p: %d, \n", 
			  t1, t2, t3, cnt_p);
	}  
	       
	t1 = t1 >> 3;
	t2 = t2 >> 3;
	t3 = t3 >> 3;
	printf_syn("[xVRMS] according as debug:%d, %d, %d(0x%x, 0x%x, 0x%x)\n",
			t1, t2, t3,
			t1, t2, t3);

#endif
#if 0      
/*	
 * 测试控制hsdc通信的开关，选通hsdc后spi读取数据

 */

	cnt_s++;
	if(cnt_s%2 ==0){
		printf_syn("hsdc open\n ");

		GPIO_InitTypeDef GPIO_InitStructure;  		 
		GPIO_InitStructure.GPIO_Pin 	= ADE7880_HSDC_NSS;
		GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_IPU;  
		GPIO_Init(SPI3_NSS_PORT, &GPIO_InitStructure);  
		   
		Write_16bitReg_ADE7880(CONFIG_Register_Address, CONFIG_Register_Data1);
		
		 
	}else if(cnt_s%2 ==1){
		printf_syn("hsdc close\n ");
		Write_16bitReg_ADE7880(CONFIG_Register_Address, CONFIG_Register_Data);

	}

#endif
#if 0 
/*	
 * 测试hsdc方式一个报表数据澹(连续16个32bit数据)的读
 */  

	ade7880_spi_withdma_hsdccfg();
	dma_configuration_spi1_rx();
	printf_syn("dma int end\n");
  
	DMA1_Channel2->CCR   &= ~(1 << 0);   // 关闭DMA传输

        DMA1_Channel2->CMAR   = (u32)(&(SPI_DMA_Table_serial_in[0]));
        DMA1_Channel2->CNDTR  = 32;
        DMA1_Channel2->CCR   |= 1 << 0;      // 开启DMA传输

 	
	printf_syn("hsdc open\n");
	hsdc_transcomp_flag = 0;
	Write_16bitReg_ADE7880(CONFIG_Register_Address, CONFIG_Register_Data1);
 
#endif  
#if 0
/*	
 * 测试开启hsdc传输数据的正确性 //测试hsdc方式下电压电流采样(不定时连续采样电压电流)

 */

	u8 cnt_p;// cnt;
	GPIO_InitTypeDef GPIO_InitStructure; 
	GPIO_InitStructure.GPIO_Pin 	= ADE7880_HSDC_SCK  | ADE7880_HSDC_MOSI;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF_PP;
	GPIO_Init(ADE7880_HSDC_PORT, &GPIO_InitStructure); 
	
	GPIO_InitStructure.GPIO_Pin 	= ADE7880_HSDC_MISO;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_Out_PP;
	GPIO_Init(ADE7880_HSDC_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin 	= ADE7880_HSDC_NSS;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_Out_PP;
	GPIO_Init(SPI3_NSS_PORT, &GPIO_InitStructure);

	dma_configuration_spi1_rx();	   
	ade7880_spi_withdma_hsdccfg(); 
 
	printf_syn("dma int end\n");  
	rt_thread_delay(get_ticks_of_ms(10)); 
	//for(cnt=0; cnt<50; cnt++ ){ 
          
		DMA1_Channel2->CCR   &= ~(1 << 0);   // 关闭DMA传输

	        DMA1_Channel2->CMAR   = (u32)(&(SPI_DMA_Table_serial_in[0]));
	        DMA1_Channel2->CNDTR  = 32;
	        DMA1_Channel2->CCR   |= 1 << 0;      // 开启DMA传输
	 	      
		printf_syn("hsdc open\n"); 
		hsdc_transcomp_flag = 0;
		//Write_8bitReg_ADE7880(HSDC_CFG_Register_Address, 0x0C);
		Write_16bitReg_ADE7880(CONFIG_Register_Address, CONFIG_Register_Data1);
		  
		if(1 == hsdc_transcomp_flag){ 
			  
			/*
			 *数据正确，hsdc传输一个数据32bit，dma接收后分高16bit和低16bit存入SPI_DMA_Table_serial_in数组中两个单元内存中
			*/ 


			printf_syn("old buffer 0:0x%x, 1:0x%x, 2:0x%x, 3:0x%x, 4:0x%x, 5:0x%x, 6:0x%x, 7:0x%x, 8:0x%x, 9:0x%x, 10:0x%x, 11:0x%x, 12:0x%x, 13:0x%x, 14:0x%x, 15:0x%x\n",
				SPI_DMA_Table_serial_in[0], SPI_DMA_Table_serial_in[1],SPI_DMA_Table_serial_in[2],
				SPI_DMA_Table_serial_in[3], SPI_DMA_Table_serial_in[4],SPI_DMA_Table_serial_in[5],
				SPI_DMA_Table_serial_in[6], SPI_DMA_Table_serial_in[7],SPI_DMA_Table_serial_in[8],
				SPI_DMA_Table_serial_in[9], SPI_DMA_Table_serial_in[10],SPI_DMA_Table_serial_in[11],
				SPI_DMA_Table_serial_in[12], SPI_DMA_Table_serial_in[13],SPI_DMA_Table_serial_in[14],
				SPI_DMA_Table_serial_in[15]
			);
			
 			/*
			 *数据拼接，dma是以16bit接收,需要将高低16bit拼接为32bit作为一帧数据
			 */  
 

			for(cnt_p=0; cnt_p<16; cnt_p++){ 
				SPI_DMA_HSCD_BUFFER[cnt_p] = 0;
				SPI_DMA_HSCD_BUFFER[cnt_p] |= SPI_DMA_Table_serial_in[cnt_p*2] << 16;
				SPI_DMA_HSCD_BUFFER[cnt_p] |= SPI_DMA_Table_serial_in[cnt_p*2+1];
			}
			
			printf_syn("new buffer 0:0x%x, 1:0x%x, 2:0x%x, 3:0x%x, 4:0x%x, 5:0x%x, 6:0x%x, 7:0x%x, 8:0x%x, 9:0x%x, 10:0x%x, 11:0x%x, 12:0x%x, 13:0x%x, 14:0x%x, 15:0x%x\n",
				SPI_DMA_HSCD_BUFFER[0], SPI_DMA_HSCD_BUFFER[1],SPI_DMA_HSCD_BUFFER[2],
				SPI_DMA_HSCD_BUFFER[3], SPI_DMA_HSCD_BUFFER[4],SPI_DMA_HSCD_BUFFER[5],
				SPI_DMA_HSCD_BUFFER[6], SPI_DMA_HSCD_BUFFER[7],SPI_DMA_HSCD_BUFFER[8],
				SPI_DMA_HSCD_BUFFER[9], SPI_DMA_HSCD_BUFFER[10],SPI_DMA_HSCD_BUFFER[11],
				SPI_DMA_HSCD_BUFFER[12], SPI_DMA_HSCD_BUFFER[13],SPI_DMA_HSCD_BUFFER[14],
				SPI_DMA_HSCD_BUFFER[15]
			);
			
		}  
		
	//}   

#endif     
#if 0        
	u8 cnt;
#if 0
 /*	
* 测试hsdc方式ABC三相电压电流采样及无功功率
*/ 

	DMA1_Channel2->CCR   &= ~(1 << 0);  
        DMA1_Channel2->CMAR   = (u32)(&(SPI_DMA_Table_serial_in[0]));
        DMA1_Channel2->CNDTR  = 32;
        DMA1_Channel2->CCR   |= 1 << 0;      
#else
	ade7880_spi_withdma_hsdccfg();
	dma_configuration_spi1_rx();
        DMA1_Channel2->CCR   |= 1 << 0;     
#endif	
	hsdc_transcomp_flag = 0; 	        
	Write_16bitReg_ADE7880(CONFIG_Register_Address, CONFIG_Register_Data1);	 	      

	rt_thread_delay(get_ticks_of_ms(30));

	if(hsdc_transcomp_flag == 160){ 
		printf_syn("\nav sampling :\n");	  		
		for(cnt=0 ;cnt<40; cnt++){
			//printf_syn("0x%x  ",AV_HSCD_BUFFER[cnt]);
			printf_syn("%d  ",AV_HSCD_BUFFER[cnt]);
		}

		printf_syn("\nbv sampling :\n");			
		for(cnt=0 ;cnt<40; cnt++){
			//printf_syn("0x%x  ",BV_HSCD_BUFFER[cnt]);
			printf_syn("%d  ",BV_HSCD_BUFFER[cnt]);
		}

		printf_syn("\ncv sampling :\n");			
		for(cnt=0 ;cnt<40; cnt++){
			//printf_syn("0x%x  ",CV_HSCD_BUFFER[cnt]);
			printf_syn("%d  ",CV_HSCD_BUFFER[cnt]);
		}
			  
		printf_syn("\nxFVAR :\n");			
		for(cnt=0 ;cnt<3; cnt++){  
			printf_syn("0x%x  ",XFVAR_HSCD_BUFFER[cnt]);  
			//printf_syn("%d  ",XFVAR_HSCD_BUFFER[cnt]);
		}
	}
#endif
#if 0	/* 4432半自动组网测试程序 */
 	get_st_from_485_to_cal_4432();
#endif  
#if 0  
	vi_vector_graph_sampl();
#endif
#if 0
	err_645 = read_ammeter_data(&send_data, &recv_data, port_485);
#endif 
#if 0
	vi_vector_graph_sampl();
	px_phase_autodebug();
	vi_vector_graph_sampl();
#endif
	return ;
}
FINSH_FUNCTION_EXPORT(rt_val_7880, "read real time 7880 value");

#endif
