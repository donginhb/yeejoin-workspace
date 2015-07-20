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

#define SYSCFGDATA_TBL_BASE       ((rt_uint32_t *)(0x08000000 + 255 * (2*1024)))
#define SYSCFGDATA_TBL_SIZE       (sizeof(struct syscfgdata_tbl))
#define SYSCFGDATA_TBL_START_ADDR ((char *)SYSCFGDATA_TBL_BASE + (sizeof(struct tab_head)))

#define SYSCFGDATA_TBL_MAGIC_NUM  (0X5A5A6383UL)
#define IS_SYSCFG_TBL_VALID       (SYSCFGDATA_TBL_MAGIC_NUM == *SYSCFGDATA_TBL_BASE)

#define offsetof(type, member) \
    (( size_t)((char *)&(((type *)0)->member) - (char *)0))


struct login_param login_param_name[USR_NUM_MAX] = {
    {{"admin", "888888"}}
};

static struct syscfgdata_tbl_cache_st syscfgdata_tbl_cache;
static struct rt_semaphore write_syscfgdata_sem;

#define SYSCFG_DATA_LOG(x)   printf_syn x
#define SYSCFG_DATA_PRINT(x) //printf_syn x

#define SYSCFGDATA_TBL_MEMBER_SIZE(member) sizeof(((struct syscfgdata_tbl*)0)->member)
#define SYSCFGDATA_TBL_MEMBER_ADDR(member) ((char *)(SYSCFGDATA_TBL_BASE) + offsetof(struct syscfgdata_tbl, member))

/* syscfgdata_tbl_cache.systbl_flag_set */
#define is_syscfgdata_tbl_dirty(systbl_flag_set)  is_bit_set(systbl_flag_set, SYSTBL_FLAG_DATA_DIRTY)
#define set_syscfgdata_tbl_dirty(systbl_flag_set) set_bit(systbl_flag_set, SYSTBL_FLAG_DATA_DIRTY)
#define clr_syscfgdata_tbl_dirty(systbl_flag_set) clr_bit(systbl_flag_set, SYSTBL_FLAG_DATA_DIRTY)

#define is_syscfgdata_tbl_wthrough(systbl_flag_set)  is_bit_set(systbl_flag_set, SYSTBL_FLAG_WRITE_THROUGH)
#define set_syscfgdata_tbl_wthrough(systbl_flag_set) set_bit(systbl_flag_set, SYSTBL_FLAG_WRITE_THROUGH)
#define clr_syscfgdata_tbl_wthrough(systbl_flag_set) clr_bit(systbl_flag_set, SYSTBL_FLAG_WRITE_THROUGH)


static int write_whole_syscfgdata_tbl(struct syscfgdata_tbl *data);
static int read_whole_syscfgdata_tbl(struct syscfgdata_tbl *data);
static void print_syscfgdata(struct syscfgdata_tbl *data);

/*
 * 该函数用于在没有系统数据表格时, 建立数据表格
 */
int init_syscfgdata_tbl(void)
{
	int i;
	struct syscfgdata_tbl *p;

	rt_sem_init(&write_syscfgdata_sem, "sysdata", 1, RT_IPC_FLAG_PRIO);

	rt_memset(&syscfgdata_tbl_cache, 0, sizeof(syscfgdata_tbl_cache));
	p = &syscfgdata_tbl_cache.syscfg_data;

	if (IS_SYSCFG_TBL_VALID) {
		read_whole_syscfgdata_tbl(p);
		return RT_EOK;
	}

	/*
	 * 数据库为空, 需要创建数据库
	 */
	/* systbl_head */
	p->systbl_head.magic_num = SYSCFGDATA_TBL_MAGIC_NUM;

	/* m3_sys_info */
	p->m3_sys_info.sw_ver = (RT_APP_VERSION<<16) | (RT_APP_SUBVERSION<<8) | (RT_APP_REVISION);
	p->m3_sys_info.db_ver = (M3_DB_VERSION<<16) | (M3_DB_SUBVERSION<<8) | (M3_DB_REVISION);

	/* logindata */
	rt_memcpy(p->logindata, login_param_name, sizeof(p->logindata));

	/* ipcfg */
	for (i=0; i<IP_INTERFACE_NUM; ++i) {
		p->ipcfg[i].ipmode       = IPADDR_USE_STATIC;
		p->ipcfg[i].ipaddr.addr  = DEFAULT_IPADDR;
		p->ipcfg[i].gw.addr      = DEFAULT_GATEWAY_ADDR;
		p->ipcfg[i].netmask.addr = DEFAULT_NET_MASK;
	}

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

	i = write_whole_syscfgdata_tbl(p);

	return i;
}


/*
 * ind_m: master index
 * ind_s: slave index
 */

#define SYSCFGDATA_TBL_CACHE_MEMBER_SIZE(member) 		sizeof(((struct syscfgdata_tbl*)0)->member)
#define SYSCFGDATA_TBL_CACHE_MEMBER_ADDR(pcache, member)	((char *)&((pcache)->member))
#define SYSCFGDATA_TBL_CACHE_AMEMBER_ADDR(pcache, member)	((char *)((pcache)->member))

/* 有同步机制 */
int read_syscfgdata_tbl_from_cache(unsigned int ind_m, unsigned int ind_s, void *data)
{
	u32_t *p1;
	char  *p2;
	int i, cnt;

	struct syscfgdata_tbl *p;
	int ret = RT_EOK;
	
	if (ind_m >= SYSCFGDATA_TBL_BUTT || NULL == data || !IS_SYSCFG_TBL_VALID)
		return RT_ERROR;

	rt_sem_take(&write_syscfgdata_sem, RT_WAITING_FOREVER);
	
	p = &syscfgdata_tbl_cache.syscfg_data;

	/* 作为保护性验证 */
	if (SYSCFGDATA_TBL_MAGIC_NUM != p->systbl_head.magic_num) {
		printf_syn("the data of system config table cache is invalid!\n");
		if (0 != read_whole_syscfgdata_tbl(p)) {
			ret = RT_ERROR;
			goto out;
		}
	}

	p1 = (u32_t *)data;
	switch (ind_m) {
	case SYSCFGDATA_TBL_LOGINDATA:
		p2  = SYSCFGDATA_TBL_CACHE_AMEMBER_ADDR(p, logindata);
		cnt = SYSCFGDATA_TBL_CACHE_MEMBER_SIZE(logindata) >> 2;
		break;

	case SYSCFGDATA_TBL_IPCFG:
		if (ind_s >= IP_INTERFACE_NUM) {
			ret = RT_ERROR;
			goto out;
		}

		p2  = SYSCFGDATA_TBL_CACHE_AMEMBER_ADDR(p, ipcfg);
		p2 += sizeof(struct ip_param) * ind_s;
		cnt = sizeof(struct ip_param) >> 2;

		break;

	case SYSCFGDATA_TBL_RS232CFG:
		if (ind_s>=RS232_INTERFACE_NUM) {
			ret = RT_ERROR;
			goto out;
		}

		p2  = SYSCFGDATA_TBL_CACHE_AMEMBER_ADDR(p, rs232cfg);
		p2 += sizeof(struct rs232_param) * ind_s;
		cnt = (sizeof(struct rs232_param)>>2);
		break;

	case SYSCFGDATA_TBL_TMP_RH:
		p2  = SYSCFGDATA_TBL_CACHE_MEMBER_ADDR(p, t_rh_value);
		cnt = (sizeof(struct temp_rh_limen)>>2);
		break;

	case SYSCFGDATA_TBL_NMS_IF:
		p2  = SYSCFGDATA_TBL_CACHE_MEMBER_ADDR(p, nms_if);
		cnt = (sizeof(struct nms_if_info_st)>>2);
		break;

	case SYSCFGDATA_TBL_EPON_DEV:
		p2  = SYSCFGDATA_TBL_CACHE_MEMBER_ADDR(p, epon_dev);
		cnt = (sizeof(struct epon_device_st)>>2);
		break;

	case SYSCFGDATA_TBL_SYS_INFO:
		p2  = SYSCFGDATA_TBL_CACHE_MEMBER_ADDR(p, m3_sys_info);
		cnt = (sizeof(struct m3_sys_info_st)>>2);
		break;
	
	case SYSCFGDATA_TBL_SNMP_COMMU:
		p2  = SYSCFGDATA_TBL_CACHE_MEMBER_ADDR(p, snmp_community);
		cnt = (sizeof(struct snmp_community_st)>>2);
		break;

	default:
		ret = RT_ERROR;
		goto out;
	}

	for (i=0; i<cnt; ++i) {
		*p1++ = *((u32_t *)p2);
		p2 += 4;
	}

out:
	rt_sem_release(&write_syscfgdata_sem);
	return ret;
}

int read_syscfgdata_tbl(unsigned int ind_m, unsigned int ind_s, void *data)
{
	u32_t *p1;
	char  *p2;
	int i, cnt;

	if (ind_m >= SYSCFGDATA_TBL_BUTT || NULL == data || !IS_SYSCFG_TBL_VALID)
		return RT_ERROR;

	p1 = (u32_t *)data;
	switch (ind_m) {
	case SYSCFGDATA_TBL_LOGINDATA:
		p2  = SYSCFGDATA_TBL_MEMBER_ADDR(logindata);
		cnt = SYSCFGDATA_TBL_MEMBER_SIZE(logindata) >> 2;
		break;

	case SYSCFGDATA_TBL_IPCFG:
		if (ind_s >= IP_INTERFACE_NUM)
		    return RT_ERROR;

		p2  = SYSCFGDATA_TBL_MEMBER_ADDR(ipcfg);
		p2 += sizeof(struct ip_param) * ind_s;
		cnt = sizeof(struct ip_param) >> 2;

		break;

	case SYSCFGDATA_TBL_RS232CFG:
		if (ind_s>=RS232_INTERFACE_NUM)
		    return RT_ERROR;

		p2  = SYSCFGDATA_TBL_MEMBER_ADDR(rs232cfg);
		p2 += sizeof(struct rs232_param) * ind_s;
		cnt = (sizeof(struct rs232_param)>>2);
		break;

	case SYSCFGDATA_TBL_TMP_RH:
		p2  = SYSCFGDATA_TBL_MEMBER_ADDR(t_rh_value);
		cnt = (sizeof(struct temp_rh_limen)>>2);
		break;

	case SYSCFGDATA_TBL_NMS_IF:
		p2  = SYSCFGDATA_TBL_MEMBER_ADDR(nms_if);
		cnt = (sizeof(struct nms_if_info_st)>>2);
		break;

	case SYSCFGDATA_TBL_EPON_DEV:
		p2  = SYSCFGDATA_TBL_MEMBER_ADDR(epon_dev);
		cnt = (sizeof(struct epon_device_st)>>2);
		break;

	case SYSCFGDATA_TBL_SYS_INFO:
		p2  = SYSCFGDATA_TBL_MEMBER_ADDR(m3_sys_info);
		cnt = (sizeof(struct m3_sys_info_st)>>2);
		break;
	
	case SYSCFGDATA_TBL_SNMP_COMMU:
		p2  = SYSCFGDATA_TBL_MEMBER_ADDR(snmp_community);
		cnt = (sizeof(struct snmp_community_st)>>2);
		break;

	default:
		return RT_ERROR;
	}

	for (i=0; i<cnt; ++i) {
		*p1++ = *((u32_t *)p2);
		p2 += 4;
	}

	return RT_EOK;
}


/*
 * ind_m: master index
 * ind_s: slave index
 */
int write_syscfgdata_tbl(const unsigned int ind_m, const unsigned int ind_s, void *const data)
{
	int ret = RT_EOK, i, cnt;
	struct syscfgdata_tbl *p;
	u32_t *p1, *p2, *p3;
	
	if (ind_m >= SYSCFGDATA_TBL_BUTT || NULL == data)
		return RT_ERROR;

	rt_sem_take(&write_syscfgdata_sem, RT_WAITING_FOREVER);
	
	p = &syscfgdata_tbl_cache.syscfg_data;

	/* 作为保护性验证 */
	if (SYSCFGDATA_TBL_MAGIC_NUM != p->systbl_head.magic_num) {
		printf_syn("the data of system config table cache is invalid!\n");
		if (0 != read_whole_syscfgdata_tbl(p)) {
			ret = RT_ERROR;
			goto out;
		}
	}

	SYSCFG_DATA_PRINT(("usr[0]:%#x, pw[0]:%#x \n", p->logindata[0].login.usr[0], p->logindata[0].login.pw[0]));
	SYSCFG_DATA_PRINT(("func:%s, line:%d, ind_m:%u, ind_s:%u \n", __FUNCTION__, __LINE__, ind_m, ind_s));

	switch (ind_m) {
	case SYSCFGDATA_TBL_LOGINDATA:
		if (0 != ind_s) {
			ret = RT_ERROR;
			goto out;
		}

		p2  = (u32_t *)&(p->logindata[ind_s]);
		cnt = (sizeof(p->logindata[0])>>2);
		break;

	case SYSCFGDATA_TBL_IPCFG:
		if (0 != ind_s) {
		    ret = RT_ERROR;
		    goto out;
		}
		p2  = (u32_t *)p->ipcfg;
		cnt = (sizeof(p->ipcfg[0])>>2);
		break;

	case SYSCFGDATA_TBL_RS232CFG:
		if (ind_s>=RS232_INTERFACE_NUM) {
		    ret = RT_ERROR;
		    goto out;
		}

		p2  = (u32_t *)&(p->rs232cfg[ind_s]);
		cnt = (sizeof(p->rs232cfg[0])>>2);
		break;

	case SYSCFGDATA_TBL_TMP_RH:
		p2  = (u32_t *)&(p->t_rh_value);
		cnt = (sizeof(p->t_rh_value)>>2);
		break;

	case SYSCFGDATA_TBL_NMS_IF:
		p2  = (u32_t *)&(p->nms_if);
		cnt = (sizeof(p->nms_if)>>2);
		break;

	case SYSCFGDATA_TBL_EPON_DEV:
		p2  = (u32_t *)&(p->epon_dev);
		cnt = (sizeof(p->epon_dev)>>2);
		break;

	case SYSCFGDATA_TBL_SYS_INFO:
		p2  = (u32_t *)&(p->m3_sys_info);
		cnt = (sizeof(p->m3_sys_info)>>2);
		break;
	
	case SYSCFGDATA_TBL_SNMP_COMMU:
		p2  = (u32_t *)&(p->snmp_community);
		cnt = (sizeof(p->snmp_community)>>2);
		break;

	default:
		ret = RT_ERROR;
		goto out;
	}

	p1 = data;
	p3 = p2; /* 缓冲区中的位置 */
	/* 检查所写数据与syscfgdata_tbl_cache.syscfg_data中的是否一致 */
	for (i=0; i<cnt; ++i)
		if (*p1++ != *p2++)
			break;
	if (i < cnt) {
		/* 不一致, 写入syscfgdata_tbl_cache.syscfg_data中 */
		p1 = data;
		p2 = p3;
		for (i=0; i<cnt; ++i)
			*p2++ = *p1++;

		//write_whole_syscfgdata_tbl(p); /* mark by David */
		/* 设置数据已更新标志 */
		set_syscfgdata_tbl_dirty(syscfgdata_tbl_cache.systbl_flag_set);
	}        

out:
	rt_sem_release(&write_syscfgdata_sem);
	return ret;
}


/*
 * 该函数用于恢复系统默认设置
 */
int restore_default_syscfgdata(void)
{
	int i;
	struct syscfgdata_tbl *p;

	//rt_memset(&syscfgdata_tbl_cache, 0, sizeof(syscfgdata_tbl_cache));
	syscfgdata_tbl_cache.systbl_flag_set = 0;
	p = &syscfgdata_tbl_cache.syscfg_data;

	if (IS_SYSCFG_TBL_VALID) {
		read_whole_syscfgdata_tbl(p);

		/* logindata */
		rt_memcpy(p->logindata, login_param_name, sizeof(p->logindata));

		/* ipcfg */
		for (i=0; i<IP_INTERFACE_NUM; ++i) {
			p->ipcfg[i].ipmode       = IPADDR_USE_STATIC;
			p->ipcfg[i].ipaddr.addr  = DEFAULT_IPADDR;
			p->ipcfg[i].gw.addr      = DEFAULT_GATEWAY_ADDR;
			p->ipcfg[i].netmask.addr = DEFAULT_NET_MASK;
		}

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

		/* nms_if */
		p->nms_if.nms_ip	= WEBM_P_SERVER_DEFAULT_IP;
		p->nms_if.port		= WEBM_P_SERVER_DEFAULT_PORT;
		p->nms_if.prot_type	= WEBM_P_SERVER_DEFAULT_PROTO;
		p->nms_if.trap_ip	= WEBM_P_SERVER_DEFAULT_IP;
		p->nms_if.trap_port	= WEBM_P_SERVER_DEFAULT_PORT;
		p->nms_if.nms_reserve   = 0;
		p->nms_if.trap_enable_bits = 0;
		
		/* epon_dev初始值均为0, 不用再赋值 */

		/* snmp_community */
		rt_strncpy(p->snmp_community.get_commu,  "public",  SNMP_COMMUNITY_LEN_MAX);
		rt_strncpy(p->snmp_community.set_commu,  "private", SNMP_COMMUNITY_LEN_MAX);
		rt_strncpy(p->snmp_community.trap_commu, "public",  SNMP_COMMUNITY_LEN_MAX);

		i = write_whole_syscfgdata_tbl(p);

		return SUCC;
	} else {
		printf_syn("system config table had damaged!\n");
	}

	return FAIL;
}


/*
 ***********************************************************************************************
 * 在数据表格的成员增加或减少时, 以下函数不需要修改
 ***********************************************************************************************
 */

/*
 * NOTE:write flash is a slow process
 */
static int write_whole_syscfgdata_tbl(struct syscfgdata_tbl *data)
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
	status = FLASH_ErasePage((rt_uint32_t)SYSCFGDATA_TBL_START_ADDR); /* wait for complete in FLASH_ErasePage() */
	if (FLASH_COMPLETE != status)
		while(1); //return status;

	ps = (u32 *)data;
	pd = (u32 *)SYSCFGDATA_TBL_BASE;
	for (i=0; i < RT_ALIGN(SYSCFGDATA_TBL_SIZE, 4)/4; i++) {
		status = FLASH_ProgramWord((u32)pd, *ps); /* wait for complete in FLASH_ProgramWord()  */
			if (FLASH_COMPLETE != status)
				while(1); //return status;
		++pd;
		++ps;
	}
	FLASH_Lock();

	return FLASH_COMPLETE;
}

static int read_whole_syscfgdata_tbl(struct syscfgdata_tbl *data)
{
	u32_t *p1;
	volatile u32_t *p2;
	int i;

	if (NULL == data || !IS_SYSCFG_TBL_VALID)
		return -1;

	p1 = (u32_t *)data;
	p2 = SYSCFGDATA_TBL_BASE;
	for (i=0; i<(SYSCFGDATA_TBL_SIZE>>2); ++i) {
		*p1++ = *p2++;
	}

	print_syscfgdata(data);
	return 0;
}


static void print_syscfgdata(struct syscfgdata_tbl *data)
{
	int i;
    
	for (i=0; i<USR_NUM_MAX; ++i) {
		data->logindata[i].login.usr[USR_NAME_LEN_MAX] = '\0';
		data->logindata[i].login.pw[PW_LEN_MAX]        = '\0';
		SYSCFG_DATA_PRINT(("usr:%s, pw:%s \n", data->logindata[i].login.usr, data->logindata[i].login.pw));
	}

	for (i=0; i<IP_INTERFACE_NUM; ++i)
		SYSCFG_DATA_PRINT(("if_no:%d, ip:%#x, gw:%#x, mask:%#x, ipmode:%d \n", i, data->ipcfg[i].ipaddr.addr,
		    data->ipcfg[i].gw.addr, data->ipcfg[i].netmask.addr, data->ipcfg[i].ipmode));

	for (i=0; i<RS232_INTERFACE_NUM; ++i)
		SYSCFG_DATA_PRINT(("uart_no:%d, baud:%u, databits:%u, parity:%u, stopbit:%u \n", i, data->rs232cfg[i].baudrate,
		    data->rs232cfg[i].databits, data->rs232cfg[i].paritybit, data->rs232cfg[i].stopbits));

	SYSCFG_DATA_PRINT(("temp_h:%d, temp_l:%d, rh_h:%d, rh_l:%d\n", data->t_rh_value.temp_h,
		data->t_rh_value.temp_l, data->t_rh_value.rh_h, data->t_rh_value.rh_l));
}


void syscfgdata_syn_proc(void)
{
	struct syscfgdata_tbl *p;
	
	if (is_syscfgdata_tbl_dirty(syscfgdata_tbl_cache.systbl_flag_set)
			|| is_syscfgdata_tbl_wthrough(syscfgdata_tbl_cache.systbl_flag_set)) {
		p = rt_calloc(RT_ALIGN(SYSCFGDATA_TBL_SIZE, 4), 1);
		if (NULL == p) {
			printf_syn("func:%s(), line:%d:mem alloc fail\n", __FUNCTION__, __LINE__);
			return;
		}

		if (0 != read_whole_syscfgdata_tbl(p))
			goto free_entry;

		if (0==rt_memcmp(&syscfgdata_tbl_cache.syscfg_data, p, sizeof(syscfgdata_tbl_cache.syscfg_data)))
			goto free_entry;

		rt_sem_take(&write_syscfgdata_sem, RT_WAITING_FOREVER);
		write_whole_syscfgdata_tbl(&syscfgdata_tbl_cache.syscfg_data);
		clr_syscfgdata_tbl_dirty(syscfgdata_tbl_cache.systbl_flag_set);
		clr_syscfgdata_tbl_wthrough(syscfgdata_tbl_cache.systbl_flag_set);
		rt_sem_release(&write_syscfgdata_sem);

free_entry:
		rt_free(p);
	}


	return;
}


void syn_syscfgdata(void)
{
	rt_sem_take(&write_syscfgdata_sem, RT_WAITING_FOREVER);
	set_syscfgdata_tbl_wthrough(syscfgdata_tbl_cache.systbl_flag_set); /*  */
	rt_sem_release(&write_syscfgdata_sem);
	
	syscfgdata_syn_proc();
}
FINSH_FUNCTION_EXPORT(syn_syscfgdata, syn syscfgdata from cache to flash);


