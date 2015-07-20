/*
 * 在没有完善的flash驱动的情况下, 使用该文件实现系统数据的存取
 * 2012-01-13 16:05:33
 */

#include <rtdef.h>
#include <rtthread.h>

#include <sys_cfg_api.h>
#include <syscfgdata.h>
#include <stm32f10x_flash.h>
#include <finsh.h>


#define PAGE_NUM_USED_BY_SYSCFGDATA_TBL		(5)
#define SIZE_PER_FLASH_PAGE			(2*1024)

/*
 * STM32F107VC有256KB片上flash, STM32F107VC的片上flash的块(页)大小为2KB
 * 把最后一个块用来存储系统中的配置数据
 * STM32F107VC的片上flash的页编号范围是[0, 127]
 */
/*
 * STM32F103Ve有512KB片上flash, STM32F103Ve的片上flash的块(页)大小为2KB
 * 把最后一个块用来存储系统中的配置数据
 * STM32F107VC的片上flash的页编号范围是[0, 255]
 */
/*
 *
 * 'STM32F10X_LD','STM32F10X_LD_VL',
 * 'STM32F10X_MD','STM32F10X_MD_VL',
 * 'STM32F10X_HD','STM32F10X_HD_VL',
 * 'STM32F10X_XL','STM32F10X_CL'
 * */
#if defined(STM32F10X_HD)
#define LAST_PAGE_NUM_OF_ONCHIP_FLASH	(512/(SIZE_PER_FLASH_PAGE/1024) - PAGE_NUM_USED_BY_SYSCFGDATA_TBL)
#elif defined(STM32F10X_XL)
#define LAST_PAGE_NUM_OF_ONCHIP_FLASH	(768/(SIZE_PER_FLASH_PAGE/1024) - PAGE_NUM_USED_BY_SYSCFGDATA_TBL)
#endif

#define SYSCFGDATA_TBL_FLASH_SIZE		(PAGE_NUM_USED_BY_SYSCFGDATA_TBL * SIZE_PER_FLASH_PAGE)

#define SYSCFGDATA_TBL_BASE_OF_FLASH       	((rt_uint32_t *)(0x08000000 + LAST_PAGE_NUM_OF_ONCHIP_FLASH * (SIZE_PER_FLASH_PAGE)))
#define SYSCFGDATA_TBL_SIZE_OF_FLASH       	(sizeof(struct syscfgdata_tbl))
#define SYSCFGDATA_TBLDATA_START_ADDR_OF_FLASH 	((char *)SYSCFGDATA_TBL_BASE_OF_FLASH + (sizeof(struct tab_head)))
#define SYSCFGDATA_TBL_BASE_ADDR_OF_FLASH  	((struct syscfgdata_tbl*)SYSCFGDATA_TBL_BASE_OF_FLASH)

#define SYSCFGDATA_TBL_MAGIC_NUM  	(0X5A5A6383UL)
#define IS_SYSCFG_TBL_OF_FLASH_VALID	(SYSCFGDATA_TBL_MAGIC_NUM == *SYSCFGDATA_TBL_BASE_OF_FLASH)

#define POWEROFF_INFO_TBL_BASE_OF_FLASH		((rt_uint32_t *)(0x08000000 + (LAST_PAGE_NUM_OF_ONCHIP_FLASH-1) * (2*1024)))
#define POWEROFF_INFO_TBL_SIZE_OF_FLASH     	(sizeof(struct poweroff_info_st))
#define POWEROFF_INFO_TBL_MAGIC_NUM  		(0XA5A56383UL)
#define IS_POWEROFF_INFO_TBL_OF_FLASH_VALID	(POWEROFF_INFO_TBL_MAGIC_NUM == *POWEROFF_INFO_TBL_BASE_OF_FLASH)

#define offsetof(type, member)  (( size_t)((char *)&(((type *)0)->member) - (char *)0))

struct login_param login_param_name[USR_NUM_MAX] = {
	{{"admin", "888888"}}
};

static struct syscfgdata_tbl_cache_st *syscfgdata_tbl_cache;
static struct rt_semaphore write_syscfgdata_sem;

#define SYSCFG_DATA_LOG(x)   rt_kprintf x
#define SYSCFG_DATA_PRINT(x) //rt_kprintf x

#define SYSCFGDATA_TBL_MEMBER_SIZE(member) sizeof(((struct syscfgdata_tbl*)0)->member)
#define SYSCFGDATA_TBL_MEMBER_ADDR_OF_FLASH(member) ((char *)(SYSCFGDATA_TBL_BASE_OF_FLASH) + offsetof(struct syscfgdata_tbl, member))

#define SYSCFGDATA_TBL_MEMBER_ADDR(ptbl, member)	((char *)&((ptbl)->member))
#define SYSCFGDATA_TBL_AMEMBER_ADDR(ptbl, member)	((char *)((ptbl)->member))


/* syscfgdata_tbl_cache->systbl_flag_set */
#define is_syscfgdata_tbl_dirty(systbl_flag_set)  is_bit_set(systbl_flag_set, SYSTBL_FLAG_DATA_DIRTY)
#define set_syscfgdata_tbl_dirty(systbl_flag_set) set_bit(systbl_flag_set, SYSTBL_FLAG_DATA_DIRTY)
#define clr_syscfgdata_tbl_dirty(systbl_flag_set) clr_bit(systbl_flag_set, SYSTBL_FLAG_DATA_DIRTY)

#define is_syscfgdata_tbl_wthrough(systbl_flag_set)  is_bit_set(systbl_flag_set, SYSTBL_FLAG_WRITE_THROUGH)
#define set_syscfgdata_tbl_wthrough(systbl_flag_set) set_bit(systbl_flag_set, SYSTBL_FLAG_WRITE_THROUGH)
#define clr_syscfgdata_tbl_wthrough(systbl_flag_set) clr_bit(systbl_flag_set, SYSTBL_FLAG_WRITE_THROUGH)


static int write_whole_syscfgdata_tbl(struct syscfgdata_tbl *data);
static int read_whole_syscfgdata_tbl(struct syscfgdata_tbl *data);
static int
get_member_start_and_size(int ind_m, int ind_s, struct syscfgdata_tbl *p, char **ps, int *cnt);
static int set_systbl_to_default_value(struct syscfgdata_tbl *p);



/*
 * 该函数用于将用户配置数据恢复到系统默认值
 */
int restore_default_syscfgdata(void)
{
	int i, ret = SUCC;
	struct syscfgdata_tbl *p;
	rt_err_t ret_sem;

	ret_sem = rt_sem_take(&write_syscfgdata_sem, RT_WAITING_FOREVER);
	if (RT_EOK != ret_sem) {
		SYSCFG_DATA_LOG(("func:%s, line:%d, error(%d)", __FUNCTION__, __LINE__, ret_sem));
		return RT_ERROR;
	}

	syscfgdata_tbl_cache->systbl_flag_set = 0;
	p = &syscfgdata_tbl_cache->syscfg_data;

	if (IS_SYSCFG_TBL_OF_FLASH_VALID) {
		read_whole_syscfgdata_tbl(p);

		/* logindata */
		rt_memcpy(p->logindata, login_param_name, sizeof(p->logindata));
#if RT_USING_TCPIP_STACK
		/* ipcfg */
		for (i=0; i<IP_INTERFACE_NUM; ++i) {
			p->ipcfg[i].ipmode       = IPADDR_USE_STATIC;
			p->ipcfg[i].ipaddr.addr  = DEFAULT_IPADDR;
			p->ipcfg[i].gw.addr      = DEFAULT_GATEWAY_ADDR;
			p->ipcfg[i].netmask.addr = DEFAULT_NET_MASK;
		}
#endif
		/* rs232cfg */
		for (i=0; i<RS232_INTERFACE_NUM; ++i) {
			p->rs232cfg[i].baudrate  = RS232_DEFAULT_BAUDRATE;
			p->rs232cfg[i].databits  = RS232_DEFAULT_DATABITS;
			p->rs232cfg[i].paritybit = RS232_DEFAULT_PARITYBIT;
			p->rs232cfg[i].stopbits  = RS232_DEFAULT_STOPBITS;
		}


		/* t_rh_value */
		p->t_rh_value.temp_h	= TMP_DEFAULT_LIMEN_H;
		p->t_rh_value.temp_l	= TMP_DEFAULT_LIMEN_L;
		p->t_rh_value.rh_h	= RH_DEFAULT_LIMEN_H;
		p->t_rh_value.rh_l	= RH_DEFAULT_LIMEN_L;
#if RT_USING_TCPIP_STACK
		/* nms_if */
		p->nms_if.nms_ip	= WEBM_P_SERVER_DEFAULT_IP;
		p->nms_if.port		= WEBM_P_SERVER_DEFAULT_PORT;
		p->nms_if.prot_type	= WEBM_P_SERVER_DEFAULT_PROTO;
		p->nms_if.trap_ip	= WEBM_P_SERVER_DEFAULT_IP;
		p->nms_if.trap_port	= SNMP_TRAP_PORT;

		/* snmp_community */
		rt_strncpy(p->snmp_community.get_commu,  "public",  SNMP_COMMUNITY_LEN_MAX);
		rt_strncpy(p->snmp_community.set_commu,  "private", SNMP_COMMUNITY_LEN_MAX);
		rt_strncpy(p->snmp_community.trap_commu, "public",  SNMP_COMMUNITY_LEN_MAX);
#endif
		/* touch calibration param */
		p->touch_param.x_min = X_ADC_MIN;
		p->touch_param.x_max = X_ADC_MAX;
		p->touch_param.y_min = Y_ADC_MIN;
		p->touch_param.y_max = Y_ADC_MAX;

		p->misc_byteinfo.rtc_cali = RTC_CALIBRATION_DEF_VALUE;
		p->misc_byteinfo.pad0     = 0;
		p->misc_byteinfo.pad1     = 0;
		p->misc_byteinfo.pad2     = 0;

		i = write_whole_syscfgdata_tbl(p);
	} else {
		printf_syn("system config table had damaged!\n");
		ret = FAIL;
	}

	rt_sem_release(&write_syscfgdata_sem);
	return ret;
}

/*
 * 用于建立表格时，调用来初始化表格成员
 * 只被init_syscfgdata_tbl()调用
 * */
static int set_systbl_to_default_value(struct syscfgdata_tbl *p)
{
	int i;

	if (NULL == p) {
		SYSCFG_DATA_LOG(("fun:%s, pointer is NULL\n", __FUNCTION__));
		return FAIL;
	}

	/* m3_sys_info */
	p->m3_sys_info.sw_ver = (RT_APP_VERSION<<16) | (RT_APP_SUBVERSION<<8) | (RT_APP_REVISION);
	p->m3_sys_info.sw_file_mtime = 0;
	p->m3_sys_info.neid = 1;
	p->m3_sys_info.db_ver = (M3_DB_VERSION<<16) | (M3_DB_SUBVERSION<<8) | (M3_DB_REVISION);
#if 0 != USE_HEX_DEV_SN
	rt_memset(p->m3_sys_info.dev_sn, 0, sizeof(p->m3_sys_info.dev_sn));
#else
	rt_strncpy(p->m3_sys_info.dev_sn, DEV_SN_MODE, sizeof(p->m3_sys_info.dev_sn)-1);
#endif
	rt_strncpy(p->m3_sys_info.dev_descr, SYS_TYPE_DESCR, sizeof(p->m3_sys_info.dev_descr)-1);

	/* logindata */
	rt_memcpy(p->logindata, login_param_name, sizeof(p->logindata));
#if RT_USING_TCPIP_STACK
	/* ipcfg */
	for (i=0; i<IP_INTERFACE_NUM; ++i) {
		p->ipcfg[i].ipmode       = IPADDR_USE_STATIC;
		p->ipcfg[i].ipaddr.addr  = DEFAULT_IPADDR;
		p->ipcfg[i].gw.addr      = DEFAULT_GATEWAY_ADDR;
		p->ipcfg[i].netmask.addr = DEFAULT_NET_MASK;
	}
#endif
	/* rs232cfg */
	for (i=0; i<RS232_INTERFACE_NUM; ++i) {
		p->rs232cfg[i].baudrate  = RS232_DEFAULT_BAUDRATE;
		p->rs232cfg[i].databits  = RS232_DEFAULT_DATABITS;
		p->rs232cfg[i].paritybit = RS232_DEFAULT_PARITYBIT;
		p->rs232cfg[i].stopbits  = RS232_DEFAULT_STOPBITS;
	}

	/* t_rh_value */
	p->t_rh_value.temp_h	= TMP_DEFAULT_LIMEN_H;
	p->t_rh_value.temp_l	= TMP_DEFAULT_LIMEN_L;
	p->t_rh_value.rh_h	= RH_DEFAULT_LIMEN_H;
	p->t_rh_value.rh_l	= RH_DEFAULT_LIMEN_L;
#if RT_USING_TCPIP_STACK
	/* nms_if */
	p->nms_if.nms_ip	= WEBM_P_SERVER_DEFAULT_IP;
	p->nms_if.port		= WEBM_P_SERVER_DEFAULT_PORT;
	p->nms_if.prot_type	= WEBM_P_SERVER_DEFAULT_PROTO;
	p->nms_if.trap_ip	= WEBM_P_SERVER_DEFAULT_IP;
	p->nms_if.trap_port	= SNMP_TRAP_PORT;

	/* epon_dev初始值均为0, 不用再赋值 */

	/* snmp_community */
	rt_strncpy(p->snmp_community.get_commu,  "public",  SNMP_COMMUNITY_LEN_MAX);
	rt_strncpy(p->snmp_community.set_commu,  "private", SNMP_COMMUNITY_LEN_MAX);
	rt_strncpy(p->snmp_community.trap_commu, "public",  SNMP_COMMUNITY_LEN_MAX);
#endif

	/* touch calibration param */
	p->touch_param.x_min = X_ADC_MIN;
	p->touch_param.x_max = X_ADC_MAX;
	p->touch_param.y_min = Y_ADC_MIN;
	p->touch_param.y_max = Y_ADC_MAX;

	p->misc_byteinfo.rtc_cali = RTC_CALIBRATION_DEF_VALUE;
	p->misc_byteinfo.pad0     = 0;
	p->misc_byteinfo.pad1     = 0;
	p->misc_byteinfo.pad2     = 0;

	return SUCC;
}

/*
 * 获取系统参数表中成员起始地址和大小
 * 以结构体为单位
 * 大小的单位是字
 */
static int
get_member_start_and_size(int ind_m, int ind_s, struct syscfgdata_tbl *p, char **ps, int *cnt)
{
	int ret = RT_EOK;

	switch (ind_m) {
	case SYSCFGDATA_TBL_SYS_INFO:
		*ps  = SYSCFGDATA_TBL_MEMBER_ADDR(p, m3_sys_info);
		*cnt = sizeof(p->m3_sys_info) >> 2;
		break;

	case SYSCFGDATA_TBL_LOGINDATA:
		if (ind_s>=USR_NUM_MAX) {
			ret = RT_ERROR;
			break;
		}

		*ps  = SYSCFGDATA_TBL_MEMBER_ADDR(p, logindata[ind_s]);
		*cnt = sizeof(p->logindata[0]) >> 2;
		break;
#if RT_USING_TCPIP_STACK
	case SYSCFGDATA_TBL_IPCFG:
		if (ind_s>=IP_INTERFACE_NUM) {
			ret = RT_ERROR;
			break;
		}

		*ps  = SYSCFGDATA_TBL_MEMBER_ADDR(p, ipcfg[ind_s]);
		*cnt = sizeof(p->ipcfg) >> 2;
		break;
#endif
	case SYSCFGDATA_TBL_RS232CFG:
		if (ind_s>=RS232_INTERFACE_NUM) {
			ret = RT_ERROR;
			break;
		}

		*ps  = SYSCFGDATA_TBL_MEMBER_ADDR(p, rs232cfg[ind_s]);
		*cnt = sizeof(p->rs232cfg[0]) >> 2;
		break;

	case SYSCFGDATA_TBL_TMP_RH:
		*ps  = SYSCFGDATA_TBL_MEMBER_ADDR(p, t_rh_value);
		*cnt = sizeof(p->t_rh_value) >> 2;
		break;
#if RT_USING_TCPIP_STACK
	case SYSCFGDATA_TBL_NMS_IF:
		*ps  = SYSCFGDATA_TBL_MEMBER_ADDR(p, nms_if);
		*cnt = sizeof(p->nms_if) >> 2;
		break;

	case SYSCFGDATA_TBL_EPON_DEV:
		*ps  = SYSCFGDATA_TBL_MEMBER_ADDR(p, epon_dev);
		*cnt = sizeof(p->epon_dev) >> 2;
		break;

	case SYSCFGDATA_TBL_SNMP_COMMU:
		*ps  = SYSCFGDATA_TBL_MEMBER_ADDR(p, snmp_community);
		*cnt = sizeof(p->snmp_community) >> 2;
		break;
#endif
	case SYSCFGDATA_TBL_TOUCH_PARAM:
		*ps  = SYSCFGDATA_TBL_MEMBER_ADDR(p, touch_param);
		*cnt = sizeof(p->touch_param) >> 2;
		break;

	case SYSCFGDATA_TBL_MISC_BYTE_INFO:
		*ps  = SYSCFGDATA_TBL_MEMBER_ADDR(p, misc_byteinfo);
		*cnt = sizeof(p->misc_byteinfo) >> 2;
		break;

	case SYSCFGDATA_TBL_TL16_UART:
		if (ind_s>=TL16_UART_NUM) {
			ret = RT_ERROR;
			break;
		}

		*ps  = SYSCFGDATA_TBL_MEMBER_ADDR(p, tl16_uart[ind_s]);
		*cnt = sizeof(p->tl16_uart[0]) >> 2;
		break;

	case SYSCFGDATA_TBL_EM_REG_INFO:
		*ps  = SYSCFGDATA_TBL_MEMBER_ADDR(p, em_reg_info);
		*cnt = sizeof(p->em_reg_info) >> 2;
		break;

	default:
		ret = RT_ERROR;
		break;
	}

	SYSCFG_DATA_PRINT(( "ind_m:%d, ps:0x%x, cnt:%d\n",  ind_m, *ps, *cnt));

	return ret;
}


/*
 ***********************************************************************************************
 * 在数据表格的成员增加或减少时, 以下函数不需要修改
 ***********************************************************************************************
 */

/*
 * 该函数用于在没有系统数据表格时, 建立数据表格
 */
int init_syscfgdata_tbl(void)
{
	int i;
	struct syscfgdata_tbl *p;
	rt_uint32_t ver_temp1, ver_temp2;

	if (SYSCFGDATA_TBL_SIZE_OF_FLASH > (PAGE_NUM_USED_BY_SYSCFGDATA_TBL * SIZE_PER_FLASH_PAGE)) {
		rt_kprintf("error:sys table(%u) too large(shoule small than %u)", SYSCFGDATA_TBL_SIZE_OF_FLASH,
				PAGE_NUM_USED_BY_SYSCFGDATA_TBL * SIZE_PER_FLASH_PAGE);
		while(1);
	}

	syscfgdata_tbl_cache = rt_calloc(sizeof(*syscfgdata_tbl_cache), 1);
	if (NULL == syscfgdata_tbl_cache) {
		rt_kprintf("func:%s, out of mem\n", __FUNCTION__);
		while(1);
	}

	if (RT_EOK != rt_sem_init(&write_syscfgdata_sem, "sysdata", 1, RT_IPC_FLAG_PRIO)) {
		rt_kprintf("func:%s, sem init fail\n", __FUNCTION__);
		while(1);
	}

	p = &syscfgdata_tbl_cache->syscfg_data;

	SYSCFG_DATA_PRINT(("sys tbl flash-addr:0x:%x, cache-addr:0x%x\n"  , SYSCFGDATA_TBL_BASE_OF_FLASH, p));

	if (IS_SYSCFG_TBL_OF_FLASH_VALID) {
		read_whole_syscfgdata_tbl(p);

		ver_temp1 = (RT_APP_VERSION<<16) | (RT_APP_SUBVERSION<<8) | (RT_APP_REVISION);
		ver_temp2 = (M3_DB_VERSION<<16) | (M3_DB_SUBVERSION<<8) | (M3_DB_REVISION);
		if (ver_temp1!=p->m3_sys_info.sw_ver || ver_temp2!=p->m3_sys_info.db_ver) {
			p->m3_sys_info.sw_ver = ver_temp1;
			p->m3_sys_info.db_ver = ver_temp2;
			set_syscfgdata_tbl_dirty(syscfgdata_tbl_cache->systbl_flag_set);
		}

		return RT_EOK;
	}

	/* 数据库为空, 需要创建数据库 */
	/* systbl_head */
	p->systbl_head.magic_num = SYSCFGDATA_TBL_MAGIC_NUM;
	set_systbl_to_default_value(p);

	i = write_whole_syscfgdata_tbl(p);

	SYSCFG_DATA_PRINT(("sys tbl size:%d, offset:%d\n", SYSCFGDATA_TBL_SIZE_OF_FLASH,
					   offsetof(struct syscfgdata_tbl, misc_byteinfo)));

	return i;
}

/*
 * ind_m: master index
 * ind_s: slave index
 */

/* 有同步机制 */
int read_syscfgdata_tbl_from_cache(unsigned int ind_m, unsigned int ind_s, void *data)
{
	char  *p1;
	char  *p2;
	int i, cnt;
	rt_err_t ret_sem;

	struct syscfgdata_tbl *p;
	int ret = RT_EOK;

	if (ind_m >= SYSCFGDATA_TBL_BUTT || NULL == data || !IS_SYSCFG_TBL_OF_FLASH_VALID)
		return RT_ERROR;

	ret_sem = rt_sem_take(&write_syscfgdata_sem, RT_WAITING_FOREVER);
	if (RT_EOK != ret_sem) {
		SYSCFG_DATA_LOG(("func:%s, line:%d, error(%d)", __FUNCTION__, __LINE__, ret_sem));
		return RT_ERROR;
	}

	p = &syscfgdata_tbl_cache->syscfg_data;

	/* 作为保护性验证 */
	if (SYSCFGDATA_TBL_MAGIC_NUM != p->systbl_head.magic_num) {
		printf_syn("the data of system config table cache is invalid!\n");
		if (0 != read_whole_syscfgdata_tbl(p)) {
			ret = RT_ERROR;
			goto out;
		}
	}
	
	p1 = data;
	ret = get_member_start_and_size(ind_m, ind_s, p, &p2, &cnt);
	if(RT_EOK != ret) {
		SYSCFG_DATA_LOG(("func:%s, error(%d)", __FUNCTION__, ret));
		goto out;
	}
	
	cnt *= 4;
	for (i=0; i<cnt; ++i)
		*p1++ = *p2++;
out:
	rt_sem_release(&write_syscfgdata_sem);
	return ret;
}

int read_syscfgdata_tbl(unsigned int ind_m, unsigned int ind_s, void *data)
{
	char  *p1;
	char  *p2;
	int i, cnt, ret = RT_EOK;
	rt_err_t ret_sem;

	if (ind_m >= SYSCFGDATA_TBL_BUTT || NULL == data || !IS_SYSCFG_TBL_OF_FLASH_VALID)
		return RT_ERROR;

	ret_sem = rt_sem_take(&write_syscfgdata_sem, RT_WAITING_FOREVER);
	if (RT_EOK != ret_sem) {
		SYSCFG_DATA_LOG(("func:%s, line:%d, error(%d)", __FUNCTION__, __LINE__, ret_sem));
		return RT_ERROR;
	}

	p1 = data;
	if(RT_EOK != get_member_start_and_size(ind_m, ind_s, SYSCFGDATA_TBL_BASE_ADDR_OF_FLASH, &p2, &cnt)) {
		SYSCFG_DATA_LOG(("func:%s, error", __FUNCTION__));
		ret = RT_ERROR;
		goto ret_entry;
	}

	cnt *= 4;
	for (i=0; i<cnt; ++i)
		*p1++ = *p2++;

ret_entry:
	rt_sem_release(&write_syscfgdata_sem);
	return ret;
}


/*
 * ind_m: master index
 * ind_s: slave index
 */
int write_syscfgdata_tbl(const unsigned int ind_m, const unsigned int ind_s, void *const data)
{
	int ret = RT_EOK, i, cnt;
	struct syscfgdata_tbl *p;
	char *p1, *p2, *p3;
	rt_err_t ret_sem;

	if (ind_m >= SYSCFGDATA_TBL_BUTT || NULL == data)
		return RT_ERROR;

	ret_sem = rt_sem_take(&write_syscfgdata_sem, RT_WAITING_FOREVER);
	if (RT_EOK != ret_sem) {
		SYSCFG_DATA_LOG(("func:%s, line:%d, error(%d)", __FUNCTION__, __LINE__, ret_sem));
		return RT_ERROR;
	}

	p = &syscfgdata_tbl_cache->syscfg_data;

	/* 作为保护性验证 */
	if (SYSCFGDATA_TBL_MAGIC_NUM != p->systbl_head.magic_num) {
		rt_kprintf("the data of system config table cache is invalid!\n");
		if (0 != read_whole_syscfgdata_tbl(p)) {
			ret = RT_ERROR;
			goto out;
		}
	}

	SYSCFG_DATA_PRINT(("usr[0]:%#x, pw[0]:%#x \n", p->logindata[0].login.usr[0], p->logindata[0].login.pw[0]));
	SYSCFG_DATA_PRINT(("func:%s, line:%d, ind_m:%u, ind_s:%u \n", __FUNCTION__, __LINE__, ind_m, ind_s));

	ret = get_member_start_and_size(ind_m, ind_s, p, &p2, &cnt);
	if(RT_EOK != ret) {
		SYSCFG_DATA_LOG(("func:%s, error(%d)", __FUNCTION__, ret));
		goto out;
	}

	cnt *= 4;
	p1 = data;
	p3 = p2; /* 缓冲区中的位置 */
	/* 检查所写数据与syscfgdata_tbl_cache->syscfg_data中的是否一致 */
	for (i=0; i<cnt; ++i)
		if (*p1++ != *p2++)
			break;

	if (i < cnt) {
		/* 不一致, 写入syscfgdata_tbl_cache->syscfg_data中 */
		p1 = data;
		p2 = p3;
		for (i=0; i<cnt; ++i)
			*p2++ = *p1++;

		//write_whole_syscfgdata_tbl(p); /* mark by David */
		/* 设置数据已更新标志 */
		set_syscfgdata_tbl_dirty(syscfgdata_tbl_cache->systbl_flag_set);
	}

out:
	rt_sem_release(&write_syscfgdata_sem);
	return ret;
}


void syscfgdata_syn_proc(void)
{
	struct syscfgdata_tbl *p;
	rt_err_t ret_sem;

	if (is_syscfgdata_tbl_dirty(syscfgdata_tbl_cache->systbl_flag_set)
		|| is_syscfgdata_tbl_wthrough(syscfgdata_tbl_cache->systbl_flag_set)) {
		p = rt_calloc(RT_ALIGN(SYSCFGDATA_TBL_SIZE_OF_FLASH, 4), 1);
		if (NULL == p) {
			printf_syn("func:%s(), line:%d:mem alloc fail\n", __FUNCTION__, __LINE__);
			return;
		}

		ret_sem = rt_sem_take(&write_syscfgdata_sem, RT_WAITING_FOREVER);
		if (RT_EOK != ret_sem) {
			SYSCFG_DATA_LOG(("func:%s, line:%d, error(%d)", __FUNCTION__, __LINE__, ret_sem));
			goto free_entry;
		}

		if (0 != read_whole_syscfgdata_tbl(p))
			goto release_sem;

		if (0==rt_memcmp(&syscfgdata_tbl_cache->syscfg_data, p, sizeof(syscfgdata_tbl_cache->syscfg_data)))
			goto release_sem;


		write_whole_syscfgdata_tbl(&syscfgdata_tbl_cache->syscfg_data);
		clr_syscfgdata_tbl_dirty(syscfgdata_tbl_cache->systbl_flag_set);
		clr_syscfgdata_tbl_wthrough(syscfgdata_tbl_cache->systbl_flag_set);
release_sem:
		rt_sem_release(&write_syscfgdata_sem);
free_entry:
		rt_free(p);
	}

	return;
}


/*
 * NOTE:write flash is a slow process
 */
static void do_write_flash_page(void *const page_addr, void *const data_page, const u32 write_data_len)
{
	int i, w_cnt;
	u32 *ps, *pd;
	FLASH_Status status;

	w_cnt = RT_ALIGN(write_data_len, 4) / 4;

	ps = data_page;
	pd = page_addr;
	for (i=0; i<w_cnt; i++) {
		if (*pd++ != *ps++)
			break;
	}

	if (i < w_cnt) {
		status = FLASH_ErasePage((rt_uint32_t)page_addr); /* wait for complete in FLASH_ErasePage() */
		if (FLASH_COMPLETE != status) {
			rt_kprintf("%s(), line:%d, erase page fail(%d)\n", __FUNCTION__, __LINE__, status);
			while(1); //return status;
		}

		ps = data_page;
		pd = page_addr;
		for (i=0; i < w_cnt; i++) {
			status = FLASH_ProgramWord((u32)pd, *ps); /* wait for complete in FLASH_ProgramWord()  */
			if (FLASH_COMPLETE != status) {
				rt_kprintf("%s(), line:%d, program word fail(%d)\n", __FUNCTION__, __LINE__, status);
				while(1); //return status;
			}
			++pd;
			++ps;
		}
	}

	return;
}

/*
 * NOTE:write flash is a slow process
 */
static int write_whole_syscfgdata_tbl(struct syscfgdata_tbl *data)
{
	unsigned char *page_addr;
	unsigned char *data_page;
	u32 all_data_len;

	if (NULL == data)
		return -RT_ERROR;

	FLASH_Unlock();
	/* Clear pending flags (if any) */
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPTERR | FLASH_FLAG_WRPRTERR | FLASH_FLAG_PGERR | FLASH_FLAG_BSY);

	page_addr	= (unsigned char *)SYSCFGDATA_TBL_BASE_OF_FLASH;
	data_page	= (unsigned char *)data;
	all_data_len	= SYSCFGDATA_TBL_SIZE_OF_FLASH;
	while (all_data_len > 0) {
		if (all_data_len > SIZE_PER_FLASH_PAGE) {
			do_write_flash_page(page_addr, data_page, SIZE_PER_FLASH_PAGE);
			page_addr += SIZE_PER_FLASH_PAGE;
			data_page += SIZE_PER_FLASH_PAGE;
			all_data_len -= SIZE_PER_FLASH_PAGE;
		} else {
			do_write_flash_page(page_addr, data, RT_ALIGN(all_data_len, 4));
			all_data_len = 0;
			break;
		}
	}

	FLASH_Lock();

	return FLASH_COMPLETE;
}

static int read_whole_syscfgdata_tbl(struct syscfgdata_tbl *data)
{
	u32_t *p1;
	volatile u32_t *p2;
	int i;

	if (NULL == data || !IS_SYSCFG_TBL_OF_FLASH_VALID)
		return -1;

	p1 = (u32_t *)data;
	p2 = SYSCFGDATA_TBL_BASE_OF_FLASH;
	for (i=0; i<(SYSCFGDATA_TBL_SIZE_OF_FLASH>>2); ++i) {
		*p1++ = *p2++;
	}

	return 0;
}

#if 0
static void print_syscfgdata(struct syscfgdata_tbl *data)
{
	int i;

	for (i=0; i<USR_NUM_MAX; ++i) {
		data->logindata[i].login.usr[USR_NAME_LEN_MAX] = '\0';
		data->logindata[i].login.pw[PW_LEN_MAX]        = '\0';
		SYSCFG_DATA_PRINT(("usr:%s, pw:%s \n", data->logindata[i].login.usr, data->logindata[i].login.pw));
	}
#if RT_USING_TCPIP_STACK
	for (i=0; i<IP_INTERFACE_NUM; ++i)
		SYSCFG_DATA_PRINT(("if_no:%d, ip:%#x, gw:%#x, mask:%#x, ipmode:%d \n", i, data->ipcfg[i].ipaddr.addr,
						   data->ipcfg[i].gw.addr, data->ipcfg[i].netmask.addr, data->ipcfg[i].ipmode));
#endif
	for (i=0; i<RS232_INTERFACE_NUM; ++i)
		SYSCFG_DATA_PRINT(("uart_no:%d, baud:%u, databits:%u, parity:%u, stopbit:%u \n", i, data->rs232cfg[i].baudrate,
						   data->rs232cfg[i].databits, data->rs232cfg[i].paritybit, data->rs232cfg[i].stopbits));

	SYSCFG_DATA_PRINT(("temp_h:%d, temp_l:%d, rh_h:%d, rh_l:%d\n", data->t_rh_value.temp_h,
					   data->t_rh_value.temp_l, data->t_rh_value.rh_h, data->t_rh_value.rh_l));

	SYSCFG_DATA_PRINT(("touch (%d,%d), (%d,%d)\n", data->touch_param.x_min,
					   data->touch_param.y_min, data->touch_param.x_max, data->touch_param.y_max));

	SYSCFG_DATA_PRINT(("rtc cali:%d, (%d, %d, %d)\n", data->misc_byteinfo.rtc_cali,
					   data->misc_byteinfo.pad0, data->misc_byteinfo.pad1, data->misc_byteinfo.pad2));
}
#endif



#if 0
int write_whole_poweroff_info_tbl(struct poweroff_info_st *data)
{
	u32 *ps, *pd;
	int i;
	FLASH_Status status;

	if (NULL == data)
		return -RT_ERROR;

	FLASH_Unlock();
	/* Clear pending flags (if any) */
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPTERR | FLASH_FLAG_WRPRTERR |
					FLASH_FLAG_PGERR | FLASH_FLAG_BSY);

	status = FLASH_ErasePage((rt_uint32_t)POWEROFF_INFO_TBL_BASE_OF_FLASH); /* wait for complete in FLASH_ErasePage() */
	if (FLASH_COMPLETE != status)
		while(1); //return status;

	ps = (u32 *)data;
	pd = (u32 *)POWEROFF_INFO_TBL_BASE_OF_FLASH;
	for (i=0; i < RT_ALIGN(POWEROFF_INFO_TBL_SIZE_OF_FLASH, 4)/4; i++) {
		status = FLASH_ProgramWord((u32)pd, *ps); /* wait for complete in FLASH_ProgramWord()  */
		if (FLASH_COMPLETE != status)
			while(1); //return status;
		++pd;
		++ps;
	}
	FLASH_Lock();

	return FLASH_COMPLETE;
}

int read_whole_poweroff_info_tbl(struct poweroff_info_st *data)
{
	u32_t *p1;
	volatile u32_t *p2;
	int i;

	if (NULL == data || !IS_POWEROFF_INFO_TBL_OF_FLASH_VALID)
		return -1;

	p1 = (u32_t *)data;
	p2 = POWEROFF_INFO_TBL_BASE_OF_FLASH;
	for (i=0; i<(POWEROFF_INFO_TBL_SIZE_OF_FLASH>>2); ++i) {
		*p1++ = *p2++;
	}

	return 0;
}


int init_poweroff_info_tbl(struct poweroff_info_st *data)
{
	if (NULL == data)
		return 1;

	if (!IS_POWEROFF_INFO_TBL_OF_FLASH_VALID) {
		data->poi_magic			= POWEROFF_INFO_TBL_MAGIC_NUM;

		data->poi_tx_poweroff_cnt	= 0;
		data->poi_rx_poweroff_cnt    = 0;

		data->poi_tx_poweroff_t0     = 0;
		data->poi_tx_poweroff_t1     = 0;
		data->poi_tx_poweroff_t2     = 0;

		data->poi_rx_poweroff_t0     = 0;
		data->poi_rx_poweroff_t1     = 0;
		data->poi_rx_poweroff_t2     = 0;

		write_whole_poweroff_info_tbl(data);
	} else {
		read_whole_poweroff_info_tbl(data);
	}

	return 0;
}

#endif

