/*
 ******************************************************************************
 * sink-data-proc.c
 *
 *  Created on: 2014-4-24
 *      Author: David, zhaoshaowei@yeejoin.com
 *
 * COPYRIGHT (C) 2014, YEEJOIN (Beijing) Technology Development Co., Ltd.
 ******************************************************************************
 */

#include <sink_info.h>
#include <debug_sw.h>

static struct sinkinfo_wl_data_item_st *sink_wl_collect_data;
static struct rt_semaphore sink_wl_data_sem;


static struct sinkinfo_wl_data_item_st *si_lookup_item_in_wl_data(char *sn);
static int si_insert_item_to_wl_data(char *sn, union sinkinfo_ptc_ctc_st *data, rt_tick_t time_stamp);
static void si_fill_ptctst_when_get_data_fail(struct sinkinfo_pt_ct_st *p);


int si_init_wl_data_proc(void)
{
	sink_wl_collect_data = rt_calloc(SINKINFO_WL_DATA_ITEM_MAX_NO * sizeof(*sink_wl_collect_data), 1);
	if (NULL == sink_wl_collect_data) {
		printf_syn("alloc sink_wl_collect_data mem fail\n");
		return FAIL;
	}

	rt_sem_init(&sink_wl_data_sem, "wl-data", 1, RT_IPC_FLAG_PRIO);

	return SUCC;
}

void si_deinit_wl_data_proc(void)
{
	rt_free(sink_wl_collect_data);
}

/*
 * 如果已缓存, 则更新；否则，当有空闲空间时，插入新数据；其他情况返回失败
 * */
int si_update_wl_collect_data(char *sn, union sinkinfo_ptc_ctc_st *data, rt_tick_t time_stamp)
{
	struct sinkinfo_wl_data_item_st *p;
	rt_err_t ret;

	if (NULL==sn || NULL==data) {
		printf_syn("func:%s(), param is NULL\n", __FUNCTION__);
		return FAIL;
	}

	if (NULL == sink_wl_collect_data) {
		printf_syn("func:%s(),  sink_wl_collect_data is NULL\n", __FUNCTION__);
		return FAIL;
	}

	ret = rt_sem_take(&sink_wl_data_sem, RT_WAITING_FOREVER);
	if (RT_EOK != ret) {
		printf_syn("take sink_wl_data_sem fail(%d)\n", ret);
		return SIE_FAIL;
	}

	sinki_debug_data(("func:%s(), line:%d, sn:%s\n", __FUNCTION__, __LINE__, sn));

	p = si_lookup_item_in_wl_data(sn);
	if (NULL != p) {
#if 0
		enum sink_data_dev_type_e dev_type;

		dev_type = si_get_dev_type(sn);
		if (SDDT_PT == dev_type) {
			rt_memcpy(&p->item.ptc_data, &data->ptc_data, sizeof(p->item.ptc_data));
		} else if (SDDT_PT == dev_type) {
			rt_memcpy(&p->item.ctc_data, &data->ctc_data, sizeof(p->item.ctc_data));
		} else {
			/* mark by David */
		}
#else
		rt_memcpy(&p->item, data, sizeof(p->item));
#endif
		p->time_stamp = time_stamp;
	} else {
		si_insert_item_to_wl_data(sn, data, time_stamp);
	}

	rt_sem_release(&sink_wl_data_sem);

	return SUCC;
}

int si_get_item_in_wl_data(char *sn, union sinkinfo_ptc_ctc_st *item, rt_tick_t *time_stamp)
{
	rt_err_t sem_ret;
	struct sinkinfo_wl_data_item_st *p;
	int ret = SUCC;

	if (NULL==sn || NULL==item || NULL==time_stamp) {
		printf_syn("func:%s(), param is NULL\n", __FUNCTION__);
		return FAIL;
	}

	sem_ret = rt_sem_take(&sink_wl_data_sem, RT_WAITING_FOREVER);
	if (RT_EOK != sem_ret) {
		printf_syn("take sink_wl_data_sem fail(%d)\n", ret);
		return FAIL;
	}

	p = si_lookup_item_in_wl_data(sn);
	if (NULL == p) {
		ret = FAIL;
	} else {
		rt_memcpy(item, &p->item, sizeof(*item));
		*time_stamp = p->time_stamp;
	}

	rt_sem_release(&sink_wl_data_sem);

	return ret;
}

int si_get_ptc_ctc_sn_from_remote_em_sn(char *em_sn, char *ptc_ctc_sn)
{
	if (NULL==em_sn || NULL==ptc_ctc_sn) {
		printf_syn("func:%s(), param is NULL\n", __FUNCTION__);
		return FAIL;
	}

	return SUCC;
}

enum sink_data_dev_type_e si_get_dev_type(char *sn)
{
	enum sink_data_dev_type_e dtype;

	if (NULL == sn) {
		printf_syn("func:%s(), param is NULL\n", __FUNCTION__);
		return SDDT_BUTT;
	}

#if 0!=USE_HEX_DEV_SN
	switch (((DEV_TYPE_OFFSET_IN_SN) & 0x1 ? (sn[DEV_TYPE_OFFSET_IN_SN>>1]>>4) : sn[DEV_TYPE_OFFSET_IN_SN>>1])
			& 0x0f) {
	case 0xf:
		dtype = SDDT_PT;
		break;

	case 0xc:
		dtype = SDDT_CT;
		break;

	case 0xe:
		dtype = SDDT_EMC;
		break;

	default:
		dtype = SDDT_BUTT;
		break;
	}
#else
	switch (sn[DEV_TYPE_OFFSET_IN_SN]) {
	case 'F':
	case 'f':
		dtype = SDDT_PT;
		break;

	case 'C':
	case 'c':
		dtype = SDDT_CT;
		break;

	case 'E':
	case 'e':
		dtype = SDDT_EMC;
		break;

	default:
		dtype = SDDT_BUTT;
		break;
	}

#endif
	return dtype;
}

int sinkinfo_fill_ptc_buf_when_get_data_fail(struct sinkinfo_ptc_st *pt_info)
{
	if (NULL == pt_info) {
		printf_syn("func:%s(), param is NULL\n", __FUNCTION__);
		return FAIL;
	}

	si_fill_ptctst_when_get_data_fail(&pt_info->pt_pa);
	si_fill_ptctst_when_get_data_fail(&pt_info->pt_pb);
	si_fill_ptctst_when_get_data_fail(&pt_info->pt_pc);

	return SUCC;
}

int sinkinfo_fill_ctc_buf_when_get_data_fail(struct sinkinfo_ctc_st *ct_info)
{
	if (NULL == ct_info) {
		printf_syn("func:%s(), param is NULL\n", __FUNCTION__);
		return FAIL;
	}

	si_fill_ptctst_when_get_data_fail(&ct_info->ct_pa);
	si_fill_ptctst_when_get_data_fail(&ct_info->ct_pb);
	si_fill_ptctst_when_get_data_fail(&ct_info->ct_pc);

	return SUCC;
}


static void si_fill_ptctst_when_get_data_fail(struct sinkinfo_pt_ct_st *p)
{
	p->appx	= SINKINFO_GET_DATA_FAIL_INT32_VALUE;
	p->apx	= SINKINFO_GET_DATA_FAIL_UINT32_VALUE;
	p->ix	= SINKINFO_GET_DATA_FAIL_UINT32_VALUE;
	p->pfx	= SINKINFO_GET_DATA_FAIL_UINT32_VALUE;
	p->vx	= SINKINFO_GET_DATA_FAIL_UINT32_VALUE;

	return;
}
void si_fill_emc_ciloss_when_get_data_fail(struct sinkinfo_emc_copper_iron_st *p)
{
	p->copper_apxT = INVLIDE_DATAL;
	p->copper_apxA = INVLIDE_DATAL;
	p->copper_apxB = INVLIDE_DATAL;
	p->copper_apxC = INVLIDE_DATAL;
	p->iron_apxT = INVLIDE_DATAL;
	p->iron_apxA = INVLIDE_DATAL;
	p->iron_apxB = INVLIDE_DATAL;
	p->iron_apxC = INVLIDE_DATAL;

	return;
}

/* 查找指定sn的设备数据是否已缓存 */
static struct sinkinfo_wl_data_item_st *si_lookup_item_in_wl_data(char *sn)
{
	int i;
	struct sinkinfo_wl_data_item_st *p;

	/* 查找是否已存储到缓冲区 */
	p = sink_wl_collect_data;
	for (i=0; i<SINKINFO_WL_DATA_ITEM_MAX_NO; ++i) {
		if (0 == rt_strncmp(p->pt_ct_sn, sn, sizeof(p->pt_ct_sn)))
			break;
		++p;
	}

	sinki_debug_data(("func:%s(), line:%d, sn:%s\n", __FUNCTION__, __LINE__, sn));

	if (i < SINKINFO_WL_DATA_ITEM_MAX_NO)
		return p;
	else
		return NULL;

}

/*
 * 将新数据插入到下标最小的空位置
 * */
static int si_insert_item_to_wl_data(char *sn, union sinkinfo_ptc_ctc_st *data, rt_tick_t time_stamp)
{
	int i;
	struct sinkinfo_wl_data_item_st *p;

	/* 查找是否已存储到缓冲区 */
	p = sink_wl_collect_data;
	for (i=0; i<SINKINFO_WL_DATA_ITEM_MAX_NO; ++i) {
		if ('\0' == p->pt_ct_sn[0])
			break;
		++p;
	}

	if (i < SINKINFO_WL_DATA_ITEM_MAX_NO) {
		rt_strncpy(p->pt_ct_sn, sn, sizeof(p->pt_ct_sn));
		rt_memcpy(&p->item, data, sizeof(p->item));
		p->time_stamp = time_stamp;

		sinki_debug_data(("func:%s(), inser data sn:%s\n", __FUNCTION__, sn));
	} else {
		printf_syn("func:%s(), wl-data-buffer is full\n", __FUNCTION__);
		return FAIL;
	}

	return SUCC;
}


#if 1
extern void print_pt_ct_st_info(char *title, struct sinkinfo_pt_ct_st *info);

void print_sink_data(void)
{
	int i, j, k;
	struct sinkinfo_wl_data_item_st *p;
	rt_err_t ret;
	char title[DEV_SN_BUF_STRING_WITH_NUL_LEN_MAX+4];

	if (NULL == sink_wl_collect_data) {
		printf_syn("func:%s(),  sink_wl_collect_data is NULL\n", __FUNCTION__);
		return;
	}

	ret = rt_sem_take(&sink_wl_data_sem, RT_WAITING_FOREVER);
	if (RT_EOK != ret) {
		printf_syn("take sink_wl_data_sem fail(%d)\n", ret);
		return;
	}

	p = sink_wl_collect_data;
#if 0
	rt_strncpy(title, p->pt_ct_sn, sizeof(title));
	i = rt_strlen(p->pt_ct_sn);
	title[i++] = ' ';
	title[i++] = 'p';
	title[i++] = 'a';
	j = i - 1;
	title[i++] = '-';
	title[i++] = '\0';
#endif

	for (k=0; k<SINKINFO_WL_DATA_ITEM_MAX_NO; ++k) {
		if ('\0' != p->pt_ct_sn[0]) {
			
			rt_strncpy(title, p->pt_ct_sn, sizeof(title));
			i = rt_strlen(p->pt_ct_sn);
			title[i++] = ' ';
			title[i++] = 'p';
			title[i++] = 'a';
			j = i - 1;
			title[i++] = '-';
			title[i++] = '\0';
			
			printf_syn("index %4d:\n", k);
			print_pt_ct_st_info(title, &p->item.ptc_data.pt_pa);
			title[j] = 'b';
			print_pt_ct_st_info(title, &p->item.ptc_data.pt_pb);
			title[j] = 'c';
			print_pt_ct_st_info(title, &p->item.ptc_data.pt_pc);
		}

		++p;
	}

	rt_sem_release(&sink_wl_data_sem);

	return;
}

#endif


