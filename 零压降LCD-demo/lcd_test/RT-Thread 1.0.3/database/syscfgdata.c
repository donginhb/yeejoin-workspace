/*
 * ��û�����Ƶ�flash�����������, ʹ�ø��ļ�ʵ��ϵͳ���ݵĴ�ȡ
 * 2012-01-13 16:05:33
 */

#include <rtdef.h>
#include <rtthread.h>

#include <sys_cfg_api.h>
#include <syscfgdata.h>
#include <stm32f10x_flash.h>
#include <finsh.h>
#include <info_tran.h>

/*
 * STM32F107VC��256KBƬ��flash, STM32F107VC��Ƭ��flash�Ŀ�(ҳ)��СΪ2KB
 * �����һ���������洢ϵͳ�е���������
 * STM32F107VC��Ƭ��flash��ҳ��ŷ�Χ��[0, 127]
 */
/*
 * STM32F103Ve��512KBƬ��flash, STM32F103Ve��Ƭ��flash�Ŀ�(ҳ)��СΪ2KB
 * �����һ���������洢ϵͳ�е���������
 * STM32F107VC��Ƭ��flash��ҳ��ŷ�Χ��[0, 255]
 */
#if USE_OPTICX_200S_VERSION
#define SYSCFGDATA_TBL_MAGIC_NUM	(0X5A5A3683UL)
#define POWEROFF_INFO_TBL_MAGIC_NUM  	(0XA5A53683UL)
#else
#define SYSCFGDATA_TBL_MAGIC_NUM	(0X5A5A6383UL)
#define POWEROFF_INFO_TBL_MAGIC_NUM  	(0XA5A56383UL)
#endif

#define SYSCFGDATA_TBL_BASE_OF_FLASH       ((rt_uint32_t *)(0x08000000 + 255 * (2*1024)))
#define SYSCFGDATA_TBL_SIZE_OF_FLASH       (sizeof(struct syscfgdata_tbl))
#define SYSCFGDATA_TBLDATA_START_ADDR_OF_FLASH ((char *)SYSCFGDATA_TBL_BASE_OF_FLASH + (sizeof(struct tab_head)))
#define SYSCFGDATA_TBL_BASE_ADDR_OF_FLASH  ((struct syscfgdata_tbl*)SYSCFGDATA_TBL_BASE_OF_FLASH)

#define IS_SYSCFG_TBL_OF_FLASH_VALID	(SYSCFGDATA_TBL_MAGIC_NUM == *SYSCFGDATA_TBL_BASE_OF_FLASH)

#define POWEROFF_INFO_TBL_BASE_OF_FLASH		((rt_uint32_t *)(0x08000000 + 254 * (2*1024)))
#define POWEROFF_INFO_TBL_SIZE_OF_FLASH     (sizeof(struct poweroff_info_st))
#define IS_POWEROFF_INFO_TBL_OF_FLASH_VALID	(POWEROFF_INFO_TBL_MAGIC_NUM == *POWEROFF_INFO_TBL_BASE_OF_FLASH)

#define offsetof(type, member)  (( size_t)((char *)&(((type *)0)->member) - (char *)0))

struct login_param login_param_name[USR_NUM_MAX] = {
	{{"admin", "888888"}}
};

static struct syscfgdata_tbl_cache_st syscfgdata_tbl_cache;
static struct rt_semaphore write_syscfgdata_sem;

#define SYSCFG_DATA_LOG(x)   rt_kprintf x
#define SYSCFG_DATA_PRINT(x) //rt_kprintf x

#define SYSCFGDATA_TBL_MEMBER_SIZE(member) sizeof(((struct syscfgdata_tbl*)0)->member)
#define SYSCFGDATA_TBL_MEMBER_ADDR_OF_FLASH(member) ((char *)(SYSCFGDATA_TBL_BASE_OF_FLASH) + offsetof(struct syscfgdata_tbl, member))

#define SYSCFGDATA_TBL_MEMBER_ADDR(ptbl, member)	((char *)&((ptbl)->member))
#define SYSCFGDATA_TBL_AMEMBER_ADDR(ptbl, member)	((char *)((ptbl)->member))


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
static int
get_member_start_and_size(int ind_m, int ind_s, struct syscfgdata_tbl *p, char **ps, int *cnt);

/*
 * �ú���������û��ϵͳ���ݱ��ʱ, �������ݱ��
 */
int init_syscfgdata_tbl(void)
{
	int i;
	struct syscfgdata_tbl *p;

	if (SYSCFGDATA_TBL_SIZE_OF_FLASH > (2*1024)) {
		rt_kprintf("error:sys table too large...");
		while(1);
	}

	rt_sem_init(&write_syscfgdata_sem, "sysdata", 1, RT_IPC_FLAG_PRIO);

	rt_memset(&syscfgdata_tbl_cache, 0, sizeof(syscfgdata_tbl_cache));
	p = &syscfgdata_tbl_cache.syscfg_data;

	SYSCFG_DATA_PRINT(("sys tbl flash-addr:0x:%x, cache-addr:0x%x\n"  , SYSCFGDATA_TBL_BASE_OF_FLASH, p));

	if (IS_SYSCFG_TBL_OF_FLASH_VALID) {
		read_whole_syscfgdata_tbl(p);
		return RT_EOK;
	}

	/*
	 * ���ݿ�Ϊ��, ��Ҫ�������ݿ�
	 */
	/* systbl_head */
	p->systbl_head.magic_num = SYSCFGDATA_TBL_MAGIC_NUM;

	/* m3_sys_info */
	p->m3_sys_info.sw_ver = (RT_APP_VERSION<<16) | (RT_APP_SUBVERSION<<8) | (RT_APP_REVISION);
	p->m3_sys_info.sw_file_mtime = 0;
	p->m3_sys_info.db_ver = (M3_DB_VERSION<<16) | (M3_DB_SUBVERSION<<8) | (M3_DB_REVISION);
	rt_strncpy(p->m3_sys_info.dev_sn, DEV_SN_MODE, sizeof(p->m3_sys_info.dev_sn)-1);

	rt_strncpy(p->m3_sys_info.match_sn[0], DEV_SN_MODE, sizeof(p->m3_sys_info.match_sn[0])-1);
	rt_strncpy(p->m3_sys_info.match_sn[1], DEV_SN_MODE, sizeof(p->m3_sys_info.match_sn[1])-1);

	/* logindata */
	rt_memcpy(p->logindata, login_param_name, sizeof(p->logindata));


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

	SYSCFG_DATA_PRINT(("sys tbl size:%d, offset:%d\n", SYSCFGDATA_TBL_SIZE_OF_FLASH,
					   offsetof(struct syscfgdata_tbl, misc_byteinfo)));

	return i;
}

/*
 * �ú������ڻָ�ϵͳĬ������
 */
int restore_default_syscfgdata(void)
{
	int i;
	struct syscfgdata_tbl *p;

	//rt_memset(&syscfgdata_tbl_cache, 0, sizeof(syscfgdata_tbl_cache));
	syscfgdata_tbl_cache.systbl_flag_set = 0;
	p = &syscfgdata_tbl_cache.syscfg_data;

	if (IS_SYSCFG_TBL_OF_FLASH_VALID) {
		read_whole_syscfgdata_tbl(p);

		/* logindata */
		rt_memcpy(p->logindata, login_param_name, sizeof(p->logindata));

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

		return SUCC;
	} else {
		printf_syn("system config table had damaged!\n");
	}

	return FAIL;
}

/*
 * ��ȡϵͳ�������г�Ա��ʼ��ַ�ʹ�С
 * �Խṹ��Ϊ��λ
 * ��С�ĵ�λ����
 */
static int
get_member_start_and_size(int ind_m, int ind_s, struct syscfgdata_tbl *p, char **ps, int *cnt)
{
	switch (ind_m) {
	case SYSCFGDATA_TBL_LOGINDATA:
		if (ind_s>=USR_NUM_MAX)
			return RT_ERROR;

		*ps  = SYSCFGDATA_TBL_MEMBER_ADDR(p, logindata[ind_s]);
		*cnt = sizeof(p->logindata[0]) >> 2;
		break;

	case SYSCFGDATA_TBL_RS232CFG:
		if (ind_s>=RS232_INTERFACE_NUM)
			return RT_ERROR;

		*ps  = SYSCFGDATA_TBL_MEMBER_ADDR(p, rs232cfg[ind_s]);
		*cnt = (sizeof(p->rs232cfg[0]) >> 2);
		break;

	case SYSCFGDATA_TBL_TMP_RH:
		*ps  = SYSCFGDATA_TBL_MEMBER_ADDR(p, t_rh_value);
		*cnt = (sizeof(p->t_rh_value) >> 2);
		break;

	case SYSCFGDATA_TBL_SYS_INFO:
		*ps  = SYSCFGDATA_TBL_MEMBER_ADDR(p, m3_sys_info);
		*cnt = (sizeof(p->m3_sys_info) >> 2);
		break;

	case SYSCFGDATA_TBL_TOUCH_PARAM:
		*ps  = SYSCFGDATA_TBL_MEMBER_ADDR(p, touch_param);
		*cnt = (sizeof(p->touch_param) >> 2);
		break;

	case SYSCFGDATA_TBL_MISC_BYTE_INFO:
		*ps  = SYSCFGDATA_TBL_MEMBER_ADDR(p, misc_byteinfo);
		*cnt = (sizeof(p->misc_byteinfo) >> 2);
		break;

	default:
		return RT_ERROR;
	}

	SYSCFG_DATA_PRINT(( "ind_m:%d, ps:0x%x, cnt:%d\n",  ind_m, *ps, *cnt));

	return RT_EOK;
}


/*
 ***********************************************************************************************
 * �����ݱ��ĳ�Ա���ӻ����ʱ, ���º�������Ҫ�޸�
 ***********************************************************************************************
 */
/*
 * ind_m: master index
 * ind_s: slave index
 */

/* ��ͬ������ */
int read_syscfgdata_tbl_from_cache(unsigned int ind_m, unsigned int ind_s, void *data)
{
	u32_t *p1;
	char  *p2;
	int i, cnt;

	struct syscfgdata_tbl *p;
	int ret = RT_EOK;

	if (ind_m >= SYSCFGDATA_TBL_BUTT || NULL == data || !IS_SYSCFG_TBL_OF_FLASH_VALID)
		return RT_ERROR;

	rt_sem_take(&write_syscfgdata_sem, RT_WAITING_FOREVER);

	p = &syscfgdata_tbl_cache.syscfg_data;

	/* ��Ϊ��������֤ */
	if (SYSCFGDATA_TBL_MAGIC_NUM != p->systbl_head.magic_num) {
		printf_syn("the data of system config table cache is invalid!\n");
		if (0 != read_whole_syscfgdata_tbl(p)) {
			ret = RT_ERROR;
			goto out;
		}
	}

	p1 = (u32_t *)data;
	ret = get_member_start_and_size(ind_m, ind_s, p, &p2, &cnt);
	if(RT_EOK != ret) {
		SYSCFG_DATA_LOG(("func:%s, error(%d)", __FUNCTION__, ret));
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

	if (ind_m >= SYSCFGDATA_TBL_BUTT || NULL == data || !IS_SYSCFG_TBL_OF_FLASH_VALID)
		return RT_ERROR;

	p1 = (u32_t *)data;
	if(RT_EOK != get_member_start_and_size(ind_m, ind_s, SYSCFGDATA_TBL_BASE_ADDR_OF_FLASH, &p2, &cnt)) {
		SYSCFG_DATA_LOG(("func:%s, error", __FUNCTION__));
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

	/* ��Ϊ��������֤ */
	if (SYSCFGDATA_TBL_MAGIC_NUM != p->systbl_head.magic_num) {
		rt_kprintf("the data of system config table cache is invalid!\n");
		if (0 != read_whole_syscfgdata_tbl(p)) {
			ret = RT_ERROR;
			goto out;
		}
	}

	SYSCFG_DATA_PRINT(("usr[0]:%#x, pw[0]:%#x \n", p->logindata[0].login.usr[0], p->logindata[0].login.pw[0]));
	SYSCFG_DATA_PRINT(("func:%s, line:%d, ind_m:%u, ind_s:%u \n", __FUNCTION__, __LINE__, ind_m, ind_s));

	ret = get_member_start_and_size(ind_m, ind_s, p, (char **)&p2, &cnt);
	if(RT_EOK != ret) {
		SYSCFG_DATA_LOG(("func:%s, error(%d)", __FUNCTION__, ret));
		goto out;
	}

	p1 = data;
	p3 = p2; /* �������е�λ�� */
	/* �����д������syscfgdata_tbl_cache.syscfg_data�е��Ƿ�һ�� */
	for (i=0; i<cnt; ++i)
		if (*p1++ != *p2++)
			break;

	if (i < cnt) {
		/* ��һ��, д��syscfgdata_tbl_cache.syscfg_data�� */
		p1 = data;
		p2 = p3;
		for (i=0; i<cnt; ++i)
			*p2++ = *p1++;

		//write_whole_syscfgdata_tbl(p); /* mark by David */
		/* ���������Ѹ��±�־ */
		set_syscfgdata_tbl_dirty(syscfgdata_tbl_cache.systbl_flag_set);
	}

out:
	rt_sem_release(&write_syscfgdata_sem);
	return ret;
}


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
	status = FLASH_ErasePage((rt_uint32_t)SYSCFGDATA_TBL_BASE_OF_FLASH); /* wait for complete in FLASH_ErasePage() */
	if (FLASH_COMPLETE != status)
		while(1); //return status;

	ps = (u32 *)data;
	pd = (u32 *)SYSCFGDATA_TBL_BASE_OF_FLASH;
	for (i=0; i < RT_ALIGN(SYSCFGDATA_TBL_SIZE_OF_FLASH, 4)/4; i++) {
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

	if (NULL == data || !IS_SYSCFG_TBL_OF_FLASH_VALID)
		return -1;

	p1 = (u32_t *)data;
	p2 = SYSCFGDATA_TBL_BASE_OF_FLASH;
	for (i=0; i<(SYSCFGDATA_TBL_SIZE_OF_FLASH>>2); ++i) {
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


void syscfgdata_syn_proc(void)
{
	struct syscfgdata_tbl *p;

	if (is_syscfgdata_tbl_dirty(syscfgdata_tbl_cache.systbl_flag_set)
		|| is_syscfgdata_tbl_wthrough(syscfgdata_tbl_cache.systbl_flag_set)) {
		p = rt_calloc(RT_ALIGN(SYSCFGDATA_TBL_SIZE_OF_FLASH, 4), 1);
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

#if 0
void syn_syscfgdata(void)
{
	rt_sem_take(&write_syscfgdata_sem, RT_WAITING_FOREVER);
	set_syscfgdata_tbl_wthrough(syscfgdata_tbl_cache.systbl_flag_set); /*  */
	rt_sem_release(&write_syscfgdata_sem);

	syscfgdata_syn_proc();
}
//FINSH_FUNCTION_EXPORT(syn_syscfgdata, syn syscfgdata from cache to flash);
#endif
#if 1

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
		rt_memset(data, 0, sizeof(*data));
		data->poi_magic			= POWEROFF_INFO_TBL_MAGIC_NUM;

		write_whole_poweroff_info_tbl(data);
	} else {
		read_whole_poweroff_info_tbl(data);
	}

	return 0;
}

#endif

