/*
 * sinkinfo_api4mib.c
 *
 * 2013-09-24,  creat by David, zhaoshaowei@yeejoin.com
 */

#include <rtthread.h>
#include <sinkinfo_common.h>
#include <sink_info.h>
#include <misc_lib.h>
#include <syscfgdata.h>
#include <sys_cfg_api.h>

#include <sinkinfo_api4mib.h>


#if 1
/*
 * 获取"区分三相的数据"，但不包括"波形采用数据"(波形数据有专用接口函数)
 *
 * 当返回值为SIE_OK时，非空指针指向的变量存储了返回值,不需要获取的使用NULL
 * 例如,获取A相电压的调用形式为：get_sinkinfo_abc_param(SIC_GET_VOLTAGE, &a, NULL, NULL)
 */
enum sinkinfo_error_e get_sinkinfo_abc_param(int em_no, int ptct_no,enum sinkinfo_cmd_e cmd, u32_t *pa, u32_t *pb, u32_t *pc)
{
	enum sinkinfo_error_e ret = SIE_OK;

	if (NULL==pa && NULL==pb && NULL==pc)
		return SIE_NULL_PTR;

	ret = rt_sem_take(&sinkinfo_sem, RT_WAITING_FOREVER);
	if (RT_EOK != ret) {
		printf_syn("take sinkinfo_sem fail(%d)\n", ret);
		return SIE_FAIL;
	}

	switch (cmd) {
	case SIC_GET_VOLTAGE:
		if (NULL != pa)
			*pa = sinkinfo_all_em[em_no].si_emc_ind_pa.vx;
		
		if (NULL != pb)
			*pb = sinkinfo_all_em[em_no].si_emc_ind_pb.vx;

		if (NULL != pc)
			*pc = sinkinfo_all_em[em_no].si_emc_ind_pc.vx;
		break;

	case SIC_GET_CURRENT:
		if (NULL != pa)
			*pa = sinkinfo_all_em[em_no].si_emc_ind_pa.ix;

		if (NULL != pb)
			*pb = sinkinfo_all_em[em_no].si_emc_ind_pb.ix;

		if (NULL != pc)
			*pc = sinkinfo_all_em[em_no].si_emc_ind_pc.ix;
		break;

	case SIC_GET_FREQUENCY:
		if (NULL != pa)
			*pa = sinkinfo_all_em[em_no].si_emc_ind_pa.hzx;

		if (NULL != pb)
			*pb = sinkinfo_all_em[em_no].si_emc_ind_pb.hzx;

		if (NULL != pc)
			*pc = sinkinfo_all_em[em_no].si_emc_ind_pc.hzx;
		break;

	case SIC_GET_PHASE:
		if (NULL != pa)
			*pa = sinkinfo_all_em[em_no].si_emc_ind_pa.phx;

		if (NULL != pb)
			*pb = sinkinfo_all_em[em_no].si_emc_ind_pb.phx;

		if (NULL != pc)
			*pc = sinkinfo_all_em[em_no].si_emc_ind_pc.phx;
		break;

	case SIC_GET_ACTIVE_POWER:
		if (NULL != pa)
			*pa = sinkinfo_all_em[em_no].si_emc_ind_pa.apx;

		if (NULL != pb)
			*pb = sinkinfo_all_em[em_no].si_emc_ind_pb.apx;

		if (NULL != pc)
			*pc = sinkinfo_all_em[em_no].si_emc_ind_pc.apx;
		break;

	case SIC_GET_REACTIVE_POWER:
		if (NULL != pa)
//			*pa = sinkinfo.si_emc_ind_pa.rapx;
			*pa = lwip_htonl(conv_4byte_bcd_to_long(sinkinfo_all_em[em_no].si_em_ind_pa.rapx));

		if (NULL != pb)
//			*pb = sinkinfo.si_emc_ind_pb.rapx;
			*pb = lwip_htonl(conv_4byte_bcd_to_long(sinkinfo_all_em[em_no].si_em_ind_pb.rapx));

		if (NULL != pc)
//			*pc = sinkinfo.si_emc_ind_pc.rapx;
			*pc = lwip_htonl(conv_4byte_bcd_to_long(sinkinfo_all_em[em_no].si_em_ind_pc.rapx));
		break;

	case SIC_GET_APPARENT_POWER:
		if (NULL != pa)
			*pa = sinkinfo_all_em[em_no].si_emc_ind_pa.appx;

		if (NULL != pb)
			*pb =sinkinfo_all_em[em_no].si_emc_ind_pb.appx;

		if (NULL != pc)
			*pc = sinkinfo_all_em[em_no].si_emc_ind_pc.appx;
		break;

	case SIC_GET_POWER_FACTOR:
		if (NULL != pa)
			*pa = sinkinfo_all_em[em_no].si_emc_ind_pa.pfx;

		if (NULL != pb)
			*pb = sinkinfo_all_em[em_no].si_emc_ind_pb.pfx;

		if (NULL != pc)
			*pc = sinkinfo_all_em[em_no].si_emc_ind_pc.pfx;
		break;

	case SIC_GET_VOLTAGE_DISTORTION:
		if (NULL != pa)
			*pa = INVLIDE_DATAL; //sinkinfo.si_emc_ind_pa.vdx; /* 该版本不实现, 所有二进制bit置1 */

		if (NULL != pb)
			*pb = INVLIDE_DATAL; //sinkinfo.si_emc_ind_pb.vdx;

		if (NULL != pc)
			*pc = INVLIDE_DATAL; //sinkinfo.si_emc_ind_pc.vdx;
		break;

	case SIC_GET_CURRENT_DISTORTION:
		if (NULL != pa)
			*pa =  INVLIDE_DATAL; //sinkinfo.si_emc_ind_pa.cdx; /* 该版本不实现, 所有二进制bit置1 */

		if (NULL != pb)
			*pb =  INVLIDE_DATAL; //sinkinfo.si_emc_ind_pb.cdx;

		if (NULL != pc)
			*pc =  INVLIDE_DATAL; //sinkinfo.si_emc_ind_pc.cdx;
		break;

	case SIC_GET_EM_VOLTAGE:			/* 电表中读取的电压 */
		if (NULL != pa)
			*pa = lwip_htonl(conv_4byte_bcd_to_long(sinkinfo_all_em[em_no].si_em_ind_pa.vx));

		if (NULL != pb)
			*pb = lwip_htonl(conv_4byte_bcd_to_long(sinkinfo_all_em[em_no].si_em_ind_pb.vx));

		if (NULL != pc)
			*pc = lwip_htonl(conv_4byte_bcd_to_long(sinkinfo_all_em[em_no].si_em_ind_pc.vx));
		break;

	case SIC_GET_EM_CURRENT:			/* 电表中读取的电流 */
		if (NULL != pa)
			*pa = lwip_htonl(conv_4byte_bcd_to_long(sinkinfo_all_em[em_no].si_em_ind_pa.ix));

		if (NULL != pb)
			*pb = lwip_htonl(conv_4byte_bcd_to_long(sinkinfo_all_em[em_no].si_em_ind_pb.ix));

		if (NULL != pc)
			*pc = lwip_htonl(conv_4byte_bcd_to_long(sinkinfo_all_em[em_no].si_em_ind_pc.ix));
		break;

	case SIC_GET_EM_ACTIVE_POWER:		/* 电表中读取的有功功率 */
		if (NULL != pa)
			*pa = lwip_htonl(conv_4byte_bcd_to_long(sinkinfo_all_em[em_no].si_em_ind_pa.apx));

		if (NULL != pb)
			*pb = lwip_htonl(conv_4byte_bcd_to_long(sinkinfo_all_em[em_no].si_em_ind_pb.apx));

		if (NULL != pc)
			*pc = lwip_htonl(conv_4byte_bcd_to_long(sinkinfo_all_em[em_no].si_em_ind_pc.apx));
		break;

	case SIC_GET_EM_REACTIVE_POWER:		/* 电表中读取的无功功率 */
		if (NULL != pa)
			*pa = lwip_htonl(conv_4byte_bcd_to_long(sinkinfo_all_em[em_no].si_em_ind_pa.rapx));

		if (NULL != pb)
			*pb = lwip_htonl(conv_4byte_bcd_to_long(sinkinfo_all_em[em_no].si_em_ind_pb.rapx));

		if (NULL != pc)
			*pc = lwip_htonl(conv_4byte_bcd_to_long(sinkinfo_all_em[em_no].si_em_ind_pc.rapx));
		break;

	case SIC_GET_EM_POWER_FACTOR:		/* 电表中读取的功率因数 */
		if (NULL != pa)
			*pa = lwip_htonl(conv_4byte_bcd_to_long(sinkinfo_all_em[em_no].si_em_ind_pa.pfx));

		if (NULL != pb)
			*pb = lwip_htonl(conv_4byte_bcd_to_long(sinkinfo_all_em[em_no].si_em_ind_pb.pfx));

		if (NULL != pc)
			*pc = lwip_htonl(conv_4byte_bcd_to_long(sinkinfo_all_em[em_no].si_em_ind_pc.pfx));
		break;


	case SIC_GET_PT_LOAD:	/* pt负荷 */
		if (NULL != pa)
			*pa = sinkinfo_all_em[em_no].pt_info.pt_pa.appx;

		if (NULL != pb)
			*pb = sinkinfo_all_em[em_no].pt_info.pt_pb.appx;

		if (NULL != pc)
			*pc = sinkinfo_all_em[em_no].pt_info.pt_pc.appx;
		break;

	case SIC_GET_CT_LOAD:	/* ct负荷 */
		if (ptct_no == 1){
			if (NULL != pa)
				*pa = sinkinfo_all_em[em_no].ct_info.ct_pa.appx;

		if (NULL != pb)
			*pb = sinkinfo_all_em[em_no].ct_info.ct_pb.appx;

			if (NULL != pc)
				*pc = sinkinfo_all_em[em_no].ct_info.ct_pc.appx;
		}else if (ptct_no == 2){
			if (NULL != pa)
				*pa = sinkinfo_all_em[em_no].ct1_info.ct_pa.appx;

			if (NULL != pb)
				*pb = sinkinfo_all_em[em_no].ct1_info.ct_pb.appx;

			if (NULL != pc)
				*pc = sinkinfo_all_em[em_no].ct1_info.ct_pc.appx;
		}
		break;

	case SIC_GET_PT_VOLTAGE_DROP: /* 二次侧电压 */
		if (NULL != pa)
			*pa = sinkinfo_all_em[em_no].pt_info.pt_pa.vx;

		if (NULL != pb)
			*pb = sinkinfo_all_em[em_no].pt_info.pt_pb.vx;

		if (NULL != pc)
			*pc = sinkinfo_all_em[em_no].pt_info.pt_pc.vx;
		break;

	default:
		ret = SIE_INVALID_CMD;
		break;
	}

	rt_sem_release(&sinkinfo_sem);

	return ret;
}


/*
 * 获取电压、电流波形采样数据, 每个点是4字节的有符号数据, 以网络序存储
 *
 * 当返回值为SIE_OK时，非空指针指向的内存区域存储了返回值
 */
enum sinkinfo_error_e get_sinkinfo_sample_data(int em_no, enum sinkinfo_cmd_e cmd, void *data_buf, int len)
{
	enum sinkinfo_error_e ret = SIE_OK;

	if (len < SINK_INFO_PX_SAMPLE_BUF_SIZE) {
		printf_syn("func:%s, buf too small(%d)\n", __FUNCTION__, len);
		return SIE_BUF2SMALL;
	}

//	ret = rt_sem_take(&px_sample_data_sem, RT_WAITING_FOREVER);
	ret = rt_sem_take(&sinkinfo_sem, RT_WAITING_FOREVER);
	if (RT_EOK != ret) {
		printf_syn("take px_sample_data_sem fail(%d)\n", ret);
		return SIE_FAIL;
	}


	switch (cmd) {
	/* 以下命令返回数据量较大，大约(40*3+4)字节 */
	case SIC_GET_PAV_SAMPLE_DATA:		/* a相电压波形采样值 */
//		conv_3bsinged_to_4bsinged(data_buf, len, sinkinfo_all_em[0].px_vi_sample.pa_vi_sample[0], SINK_INFO_PX_SAMPLE_BUF_SIZE);
		rt_memcpy(data_buf, sinkinfo_all_em[em_no].px_vi_sample.pa_vi_sample[0], len);
		break;

	case SIC_GET_PBV_SAMPLE_DATA:
//		conv_3bsinged_to_4bsinged(data_buf, len, sinkinfo_all_em[0].px_vi_sample.pb_vi_sample[0], SINK_INFO_PX_SAMPLE_BUF_SIZE);
		rt_memcpy(data_buf, sinkinfo_all_em[em_no].px_vi_sample.pb_vi_sample[0], len);
		break;

	case SIC_GET_PCV_SAMPLE_DATA:
//		conv_3bsinged_to_4bsinged(data_buf, len, sinkinfo_all_em[0].px_vi_sample.pc_vi_sample[0], SINK_INFO_PX_SAMPLE_BUF_SIZE);
		rt_memcpy(data_buf, sinkinfo_all_em[em_no].px_vi_sample.pc_vi_sample[0], len);
		break;

	case SIC_GET_PAI_SAMPLE_DATA:		/* a相电流波形采样值 */
//		conv_3bsinged_to_4bsinged(data_buf, len, sinkinfo_all_em[0].px_vi_sample.pa_vi_sample[1], SINK_INFO_PX_SAMPLE_BUF_SIZE);
		rt_memcpy(data_buf, sinkinfo_all_em[em_no].px_vi_sample.pa_vi_sample[1], len);
		break;

	case SIC_GET_PBI_SAMPLE_DATA:
//		conv_3bsinged_to_4bsinged(data_buf, len, sinkinfo_all_em[0].px_vi_sample.pb_vi_sample[1], SINK_INFO_PX_SAMPLE_BUF_SIZE);
		rt_memcpy(data_buf, sinkinfo_all_em[em_no].px_vi_sample.pb_vi_sample[1], len);
		break;

	case SIC_GET_PCI_SAMPLE_DATA:
//		conv_3bsinged_to_4bsinged(data_buf, len, sinkinfo_all_em[0].px_vi_sample.pc_vi_sample[1], SINK_INFO_PX_SAMPLE_BUF_SIZE);
		rt_memcpy(data_buf, sinkinfo_all_em[em_no].px_vi_sample.pc_vi_sample[1], len);
		break;

	default:
		ret = SIE_INVALID_CMD;
		printf_syn("func:%s, invalid cmd\n", __FUNCTION__);
		break;
	}

//	rt_sem_release(&px_sample_data_sem);
	rt_sem_release(&sinkinfo_sem);

	return ret;
}

/*
 * 获取除了"区分三相的数据"以及"波形采用数据"以外的其他数据
 *
 * 当返回值为SIE_OK时，非空指针指向的变量存储了返回值
 */
enum sinkinfo_error_e get_sinkinfo_other_param(int em_no, enum sinkinfo_cmd_e cmd, u32_t *pinfo)
{
	enum sinkinfo_error_e ret = SIE_OK;
	int temp;

	if (NULL==pinfo)
		return SIE_NULL_PTR;

	ret = rt_sem_take(&sinkinfo_sem, RT_WAITING_FOREVER);
	if (RT_EOK != ret) {
		printf_syn("take sinkinfo_sem fail(%d)\n", ret);
		return SIE_FAIL;
	}

	switch (cmd) {
	case SIC_GET_EM_ACT_ELECTRIC_ENERGY:
		if(sinkinfo_all_em[em_no].em_dev_info.em_act_total_energy == INVLIDE_DATAL){
			*pinfo = lwip_htonl(sinkinfo_all_em[em_no].em_dev_info.em_act_total_energy);
		}else{
			*pinfo = lwip_htonl(conv_4byte_bcd_to_long(sinkinfo_all_em[em_no].em_dev_info.em_act_total_energy));
		}
		break;

	case SIC_GET_DEV_ACT_ELECTRIC_ENERGY:
		*pinfo = lwip_htonl(conv_4byte_bcd_to_long(sinkinfo_all_em[em_no].em_dev_info.em_act_total_energy)); //sinkinfo.emc_dev_info.dev_act_electric_energy;
		break;

	case SIC_GET_EM_REACT_ELECTRIC_ENERGY:
		if(sinkinfo_all_em[em_no].em_dev_info.em_react_total_energy == INVLIDE_DATAL){
			*pinfo = lwip_htonl(sinkinfo_all_em[em_no].em_dev_info.em_react_total_energy);
		}else{
			*pinfo = lwip_htonl(conv_4byte_bcd_to_long(sinkinfo_all_em[em_no].em_dev_info.em_react_total_energy));
		}
		break;

	case SIC_GET_DEV_REACT_ELECTRIC_ENERGY:
		*pinfo = lwip_htonl(conv_4byte_bcd_to_long(sinkinfo_all_em[em_no].em_dev_info.em_react_total_energy)); //sinkinfo.emc_dev_info.dev_react_electric_energy;
		break;

	case SIC_GET_ACT_INACCURACY:
		if(sinkinfo_all_em[em_no].em_dev_info.em_act_ee_inaccuracy == INVLIDE_DATAL){
			temp = sinkinfo_all_em[em_no].em_dev_info.em_act_ee_inaccuracy;
		}else{
			temp = conv_4byte_bcd_to_long(sinkinfo_all_em[em_no].em_dev_info.em_act_ee_inaccuracy);
		}
		*pinfo = lwip_htonl(temp);
		break;
		
	case SIC_GET_REACT_INACCURACY:
		if(sinkinfo_all_em[em_no].em_dev_info.em_react_ee_inaccuracy == INVLIDE_DATAL){
			temp = sinkinfo_all_em[em_no].em_dev_info.em_react_ee_inaccuracy;
		}else{
			temp = conv_4byte_bcd_to_long(sinkinfo_all_em[em_no].em_dev_info.em_react_ee_inaccuracy);
		}
		*pinfo = lwip_htonl(temp);
		break;
		
	case SIC_GET_EM_METER_TEMPERATURE:
		if(sinkinfo_all_em[em_no].em_dev_info.em_temper == INVLIDE_DATAS){
			temp = sinkinfo_all_em[em_no].em_dev_info.em_temper;
		}else{
			temp = conv_4byte_bcd_to_long((unsigned long)sinkinfo_all_em[em_no].em_dev_info.em_temper);

		}
		*pinfo = lwip_htonl(temp);
		break;
		
	case SIC_GET_EM_CLOCK_BATTERY_VOL:
		if(sinkinfo_all_em[em_no].em_dev_info.em_v_clock == INVLIDE_DATAL){
			temp = sinkinfo_all_em[em_no].em_dev_info.em_v_clock;

		}else{
			temp = conv_4byte_bcd_to_long(sinkinfo_all_em[em_no].em_dev_info.em_v_clock);
		}
		*pinfo = lwip_htonl(temp);
		break;
		
	case SIC_GET_EM_METER_COLLECT_BATTERY_VOL:
		if(sinkinfo_all_em[em_no].em_dev_info.em_v_read_em == INVLIDE_DATAL){
			temp = sinkinfo_all_em[em_no].em_dev_info.em_v_read_em;

		}else{
			temp = conv_4byte_bcd_to_long(sinkinfo_all_em[em_no].em_dev_info.em_v_read_em);
		}
		*pinfo = lwip_htonl(temp);
		break;

	case SIC_GET_PT_TEMP:
		temp = sinkinfo_all_em[em_no].pttmp;
		*pinfo = lwip_htonl(temp);
		break;
	
	case SIC_GET_CT_TEMP:
		temp = sinkinfo_all_em[em_no].cttmp;
		*pinfo = lwip_htonl(temp);
		break;

#if 0
	case SIC_GET_EM_MONTH_ELECTRIC_ENERGY:
		break;

	case SIC_GET_DEV_MONTH_ELECTRIC_ENERGY:
		break;

	case SIC_GET_MONTH_INACCURACY:
		break;
#endif
	default:
		ret = SIE_INVALID_CMD;
		break;
	}

	rt_sem_release(&sinkinfo_sem);

	return ret;
}

enum sinkinfo_error_e get_em_proto(int em_no, enum sinkinfo_cmd_e cmd, u32_t *pinfo)
{
	enum sinkinfo_error_e ret = SIE_OK;
	//int temp;

	if (NULL==pinfo)
		return SIE_NULL_PTR;

	switch (cmd) {
	case SIC_GET_EM_PROTOCAL_TYPE:
		*pinfo = register_em_info.em_proto[em_no];
		break;

	case SIC_GET_EM_WIRE_CONNECT_MODE:
		*pinfo = register_em_info.em_wire_con_mode[em_no];

		break;
		
	case SIC_GET_EM_METER_DATE_AND_TIME:

		break;
	default:
		ret = SIE_INVALID_CMD;
		break;
	}

	return ret;
}

enum sinkinfo_error_e set_em_proto(int em_no, enum sinkinfo_cmd_e cmd, u32_t *pinfo)
{
	enum sinkinfo_error_e ret = SIE_OK;
	//int temp;

	if (NULL==pinfo)
		return SIE_NULL_PTR;

	switch (cmd) {
	case SIC_GET_EM_PROTOCAL_TYPE:
		register_em_info.em_proto[em_no] = *pinfo;
		break;

	case SIC_GET_EM_WIRE_CONNECT_MODE:
		register_em_info.em_wire_con_mode[em_no] = *pinfo ;
		break;
		
	case SIC_GET_EM_METER_DATE_AND_TIME:

		break;
	default:
		ret = SIE_INVALID_CMD;
		break;
	}
	return ret;
}

enum sinkinfo_error_e get_dev_sn_em_sn(enum sinkinfo_dev_type_e devt, char *str, int len, int em_no)
{
	struct gateway_em_st	 *em_info;
	enum sinkinfo_error_e ret = SIE_OK;
	//struct electric_meter_reg_info_st amm_sn;

	if (NULL==str)
		return SIE_NULL_PTR;

	em_info = rt_malloc(sizeof(*em_info));
	if (NULL == em_info) {
		printf_syn("%s(), alloc mem fail\n", __FUNCTION__);
		return SIE_FAIL;
	}

	/*ret = rt_sem_take(&sinkinfo_sem, RT_WAITING_FOREVER);
	ret = rt_sem_take(&register_em_info_sem, RT_WAITING_FOREVER);
	if (RT_EOK != ret) {
		printf_syn("%s(), take register_em_info_sem fail(%d)\n", __FUNCTION__, ret);
		rt_free(em_info);
		return SIE_FAIL;
	}*/


	if (SDT_PT==devt || SDT_CT==devt || SDT_MASTER_PT==devt)
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_GW_EM_INFO, 0, em_info)) {
			printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
			ret = SIE_FAIL;
			goto ret_entry;
		}

	switch (devt) {
	case SDT_ELECTRIC_METER:	/* 返回电表表号 */
		/*if (SUCC == get_em_reg_info(&amm_sn)) {
			rt_memcpy(str, amm_sn.em_sn[em_no], DEV_SN_MODE_LEN);
		} else {
			printf_syn("%s(), read meter SN data tbl fail\n", __FUNCTION__);
		}*/
		rt_strncpy(str, register_em_info.registered_em_sn[em_no], len);
		break;

	case SDT_PT:		/* 返回pt采集器的SN */
		rt_strncpy(str, register_em_info.registered_ptc_sn[em_no], len);
		break;

	case SDT_CT:		/* 返回ct采集器的SN */
		rt_strncpy(str, register_em_info.registered_ctc_sn[em_no], len);
		break;

	case SDT_CT1:		/* 返回ct采集器的SN */
		rt_strncpy(str, register_em_info.registered_ctc1_sn[em_no], len);
		break;

	case SDT_MASTER_PT:		/* 返回ct采集器的SN */
		*str = register_em_info.registered_em_map_vport[em_no];

		break;

	case SDT_DEV:		/* 返回电表采集器的SN */
		get_devsn(str, len);
		break;

	default:
		ret = SIE_FAIL;
		printf_syn("%s() recv invalid dev-type\n", __FUNCTION__);
		break;
	}

ret_entry:
	rt_free(em_info);
	//rt_sem_release(&register_em_info_sem);
	//rt_sem_release(&sinkinfo_sem);

	return ret;
}

int set_rf_param(u32_t fre, u32_t tx_baud, u32_t tx_fd, u32_t rx_baud, u32_t rx_fd)
{
   /*由宗澎增加内容*/

   return RT_EOK;
}
#endif

#define data_len_error_proc()	do {\
		ret = SIE_BUFFER_LEN_ERROR;\
		printf_syn("func:%s(), line:%d, data_len error:%d\n", __FUNCTION__, __LINE__, data_len);\
	} while (0)


/*
 * em_no	-- 电表编号号, 该编号可以根据sn查找到
 * cmd		-- 获取数据的命令
 * data		-- 存储数据的缓冲区地址
 * data_len	-- 缓冲区长度
 *
 * !!NOTE: 该函数返回的数据都是以主机序存储
 * */
enum sinkinfo_error_e get_sinkinfo_use_by_mib(int em_no, int ptct_no, enum si_mib_getdata_cmd_e cmd, void *data, unsigned data_len)
{
	enum sinkinfo_error_e ret = SIE_OK;
	struct celectric_meter_config_info_st amm_conf;
	int i,times;
    rt_uint32_t harmonicdata[EMC_HARMONIC_TIMES_MAX] = {0};
	if (NULL == data) {
		printf_syn("func:%s(), ptr param is NULL\n", __FUNCTION__);
		return SIE_NULL_PTR;
	}

	if ((em_no<0) || (em_no>NUM_OF_COLLECT_EM_MAX)) {
		printf_syn("func:%s(), em_no is invalid(%d)\n", __FUNCTION__, em_no);
		return SIE_FAIL;
	}
	
	ret = rt_sem_take(&sinkinfo_sem, RT_WAITING_FOREVER);
	if (RT_EOK != ret) {
		printf_syn("take sinkinfo_sem fail(%d)\n", ret);
		return SIE_FAIL;
	}

	switch (cmd) {
	case SI_MGC_GET_EMC_PA_INFO:		/* 获取电表采集器收集到的‘区分ABC三相’的A相数据 */
		if (data_len == SINKINFO_EMC_PX_DATA_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_emc_ind_pa, data_len);
		} else {
			data_len_error_proc();
		}
		break;

	case SI_MGC_GET_EMC_PB_INFO:
		if (data_len == SINKINFO_EMC_PX_DATA_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_emc_ind_pb, data_len);
		} else {
			data_len_error_proc();
		}
		break;

	case SI_MGC_GET_EMC_PC_INFO:
		if (data_len == SINKINFO_EMC_PX_DATA_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_emc_ind_pc, data_len);
		} else {
			data_len_error_proc();
		}
		break;

	case SI_MGC_GET_EMC_PA_VOL_HARMONIC_INFO:
		if (data_len == SINKINFO_EM_PX_HARMONIC_DATA_SIZE) {
		
			for(i=0; i< EMC_HARMONIC_TIMES_MAX; i++){
				harmonicdata[i] = sinkinfo_all_em[em_no].si_harmonic_pa[i].vrms;
			}
			//data = harmonicdata;
			rt_memcpy(data, harmonicdata, data_len);
		} else {
			data_len_error_proc();
		}
		break;
	case SI_MGC_GET_EMC_PA_CUR_HARMONIC_INFO:
		if (data_len == SINKINFO_EM_PX_HARMONIC_DATA_SIZE) {
			for(i=0; i< EMC_HARMONIC_TIMES_MAX; i++){
				harmonicdata[i] = sinkinfo_all_em[em_no].si_harmonic_pa[i].irms;
			}
			rt_memcpy(data, harmonicdata, data_len);
		} else {
			data_len_error_proc();
		}	
		break;
	case SI_MGC_GET_EMC_PA_ACT_HARMONIC_INFO:
		if (data_len == SINKINFO_EM_PX_HARMONIC_DATA_SIZE) {
			for(i=0; i< EMC_HARMONIC_TIMES_MAX; i++){
				harmonicdata[i] = sinkinfo_all_em[em_no].si_harmonic_pa[i].watt;
			}
			rt_memcpy(data, harmonicdata, data_len);
		} else {
			data_len_error_proc();
		}	
		break;
	case SI_MGC_GET_EMC_PB_VOL_HARMONIC_INFO:
		if (data_len == SINKINFO_EM_PX_HARMONIC_DATA_SIZE) {
		
			for(i=0; i< EMC_HARMONIC_TIMES_MAX; i++){
				harmonicdata[i] = sinkinfo_all_em[em_no].si_harmonic_pb[i].vrms;
			}
			rt_memcpy(data, harmonicdata, data_len);
		} else {
			data_len_error_proc();
		}
		break;
	case SI_MGC_GET_EMC_PB_CUR_HARMONIC_INFO:
		if (data_len == SINKINFO_EM_PX_HARMONIC_DATA_SIZE) {
		
			for(i=0; i< EMC_HARMONIC_TIMES_MAX; i++){
				harmonicdata[i] = sinkinfo_all_em[em_no].si_harmonic_pb[i].irms;
			}
			rt_memcpy(data, harmonicdata, data_len);
		} else {
			data_len_error_proc();
		}	
		break;
	case SI_MGC_GET_EMC_PB_ACT_HARMONIC_INFO:
		if (data_len == SINKINFO_EM_PX_HARMONIC_DATA_SIZE) {
			for(i=0; i< EMC_HARMONIC_TIMES_MAX; i++){
				harmonicdata[i] = sinkinfo_all_em[em_no].si_harmonic_pb[i].watt;
			}
			rt_memcpy(data, harmonicdata, data_len);
		} else {
			data_len_error_proc();
		}	
		break;	
	case SI_MGC_GET_EMC_PC_VOL_HARMONIC_INFO:
		if (data_len == SINKINFO_EM_PX_HARMONIC_DATA_SIZE) {
			for(i=0; i< EMC_HARMONIC_TIMES_MAX; i++){
				harmonicdata[i] = sinkinfo_all_em[em_no].si_harmonic_pc[i].vrms;
			}
			rt_memcpy(data, harmonicdata, data_len);
		} else {
			data_len_error_proc();
		}
		break;
	case SI_MGC_GET_EMC_PC_CUR_HARMONIC_INFO:
		if (data_len == SINKINFO_EM_PX_HARMONIC_DATA_SIZE) {
			for(i=0; i< EMC_HARMONIC_TIMES_MAX; i++){
				harmonicdata[i] = sinkinfo_all_em[em_no].si_harmonic_pc[i].irms;
			}
			rt_memcpy(data, harmonicdata, data_len);
		} else {
			data_len_error_proc();
		}
		break;	
	case SI_MGC_GET_EMC_PC_ACT_HARMONIC_INFO:
		if (data_len == SINKINFO_EM_PX_HARMONIC_DATA_SIZE) {
			for(i=0; i< EMC_HARMONIC_TIMES_MAX; i++){
				harmonicdata[i] = sinkinfo_all_em[em_no].si_harmonic_pc[i].watt;
			}
			rt_memcpy(data, harmonicdata, data_len);
		} else {
			data_len_error_proc();
		}		
		break;	
	case SI_MGC_GET_EMC_DEV_INFO:	/* 获取电表采集器收集到的‘不区分ABC三相’的数据 */
		if (data_len == SINKINFO_EMC_DEV_DATA_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].emc_dev_info, data_len);
		} else {
			data_len_error_proc();
		}
		break;

	case SI_MGC_GET_EMC_COPPER_IRON_LOSS_INFO:	/* 获取电表采集器收集到的‘不区分ABC三相’的数据 */
		if (data_len == SINKINFO_EMC_COPPER_IRON_LOSSES_DATA_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].emc_copper_iron_info, data_len);
		} else {
			data_len_error_proc();
		}
		break;

	case SI_MGC_GET_EM_PA_INFO:		/* 获取电表中的‘区分ABC三相’的A相数据 */
		if (data_len == SINKINFO_EM_PX_DATA_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_ind_pa, data_len);
		} else {
			data_len_error_proc();
		}
		break;

	case SI_MGC_GET_EM_PB_INFO:
		if (data_len == SINKINFO_EM_PX_DATA_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_ind_pb, data_len);
		} else {
			data_len_error_proc();
		}
		break;

	case SI_MGC_GET_EM_PC_INFO:
		if (data_len == SINKINFO_EM_PX_DATA_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_ind_pc, data_len);
		} else {
			data_len_error_proc();
		}
		break;
	case SI_MGC_GET_EM_PA_VOL_HARMONIC_INFO:
		if (data_len == SINKINFO_EM_PX_HARMONIC_DATA_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_vol_harmonic_pa, data_len);
		} else {
			data_len_error_proc();
		}
		break;
	case SI_MGC_GET_EM_PA_CUR_HARMONIC_INFO:
		if (data_len == SINKINFO_EM_PX_HARMONIC_DATA_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_cur_harmonic_pa, data_len);
		} else {
			data_len_error_proc();
		}
		break;
	case SI_MGC_GET_EM_PB_VOL_HARMONIC_INFO:
		if (data_len == SINKINFO_EM_PX_HARMONIC_DATA_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_vol_harmonic_pb, data_len);
		} else {
			data_len_error_proc();
		}
		break;
	case SI_MGC_GET_EM_PB_CUR_HARMONIC_INFO:
		if (data_len == SINKINFO_EM_PX_HARMONIC_DATA_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_cur_harmonic_pb, data_len);
		} else {
			data_len_error_proc();
		}
		break;	
	case SI_MGC_GET_EM_PC_VOL_HARMONIC_INFO:
		if (data_len == SINKINFO_EM_PX_HARMONIC_DATA_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_vol_harmonic_pc, data_len);
		} else {
			data_len_error_proc();
		}
		break;
	case SI_MGC_GET_EM_PC_CUR_HARMONIC_INFO:
		if (data_len == SINKINFO_EM_PX_HARMONIC_DATA_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_cur_harmonic_pc, data_len);
		} else {
			data_len_error_proc();
		}
		break;	
		
#if EM_EVENT_SUPPERT

	case SI_MGC_GET_EM_PA_VOLLOSS_EVENT_INFO:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		times = amm_conf.volloss_event_times[em_no];

		if (data_len == SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_volloss_event_pa[times], data_len);
		} else {
			data_len_error_proc();
		}
		break;
	case SI_MGC_GET_EM_PA_VOLOVER_EVENT_INFO:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		times = amm_conf.volover_event_times[em_no];

		if (data_len == SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_volover_event_pa[times], data_len);
		} else {
			data_len_error_proc();
		}
		break;
	case SI_MGC_GET_EM_PA_VOLUNDER_EVENT_INFO:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		times = amm_conf.volunder_event_times[em_no];

		if (data_len == SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_volunder_event_pa[times], data_len);
		} else {
			data_len_error_proc();
		}
		break;
	case SI_MGC_GET_EM_PA_PHASEBREAK_EVENT_INFO:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		times = amm_conf.phasebreak_event_times[em_no];

		if (data_len == SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_phasebreak_event_pa[times], data_len);
		} else {
			data_len_error_proc();
		}
		break;
	case SI_MGC_GET_EM_PA_CURLOSS_EVENT_INFO:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		times = amm_conf.curloss_event_times[em_no];

		if (data_len == SINKINFO_EM_PX_CURLOSS_EVENT_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_curloss_event_pa[times], data_len);
		} else {
			data_len_error_proc();
		}
		break;
	case SI_MGC_GET_EM_PA_CUROVER_EVENT_INFO:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		times = amm_conf.curover_event_times[em_no];

		if (data_len == SINKINFO_EM_PX_CURLOSS_EVENT_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_curover_event_pa[times], data_len);
		} else {
			data_len_error_proc();
		}
		break;
	case SI_MGC_GET_EM_PA_CURBREAK_EVENT_INFO:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		times = amm_conf.curbreak_event_times[em_no];

		if (data_len == SINKINFO_EM_PX_CURLOSS_EVENT_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_curbreak_event_pa[times], data_len);
		} else {
			data_len_error_proc();
		}
		break;

	case SI_MGC_GET_EM_PB_VOLLOSS_EVENT_INFO:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		times = amm_conf.volloss_event_times[em_no];

		if (data_len == SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_volloss_event_pb[times], data_len);
		} else {
			data_len_error_proc();
		}
		break;
	case SI_MGC_GET_EM_PB_VOLOVER_EVENT_INFO:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		times = amm_conf.volover_event_times[em_no];

		if (data_len == SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_volover_event_pb[times], data_len);
		} else {
			data_len_error_proc();
		}
		break;
	case SI_MGC_GET_EM_PB_VOLUNDER_EVENT_INFO:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		times = amm_conf.volunder_event_times[em_no];

		if (data_len == SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_volunder_event_pb[times], data_len);
		} else {
			data_len_error_proc();
		}
		break;
	case SI_MGC_GET_EM_PB_PHASEBREAK_EVENT_INFO:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		times = amm_conf.phasebreak_event_times[em_no];

		if (data_len == SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_phasebreak_event_pb[times], data_len);
		} else {
			data_len_error_proc();
		}
		break;
	case SI_MGC_GET_EM_PB_CURLOSS_EVENT_INFO:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		times = amm_conf.curloss_event_times[em_no];

		if (data_len == SINKINFO_EM_PX_CURLOSS_EVENT_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_curloss_event_pb[times], data_len);
		} else {
			data_len_error_proc();
		}
		break;
	case SI_MGC_GET_EM_PB_CUROVER_EVENT_INFO:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		times = amm_conf.curover_event_times[em_no];

		if (data_len == SINKINFO_EM_PX_CURLOSS_EVENT_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_curover_event_pb[times], data_len);
		} else {
			data_len_error_proc();
		}
		break;
	case SI_MGC_GET_EM_PB_CURBREAK_EVENT_INFO:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		times = amm_conf.curbreak_event_times[em_no];

		if (data_len == SINKINFO_EM_PX_CURLOSS_EVENT_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_curbreak_event_pb[times], data_len);
		} else {
			data_len_error_proc();
		}
		break;
	case SI_MGC_GET_EM_PC_VOLLOSS_EVENT_INFO:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		times = amm_conf.volloss_event_times[em_no];

		if (data_len == SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_volloss_event_pc[times], data_len);
		} else {
			data_len_error_proc();
		}
		break;
	case SI_MGC_GET_EM_PC_VOLOVER_EVENT_INFO:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		times = amm_conf.volover_event_times[em_no];

		if (data_len == SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_volover_event_pc[times], data_len);
		} else {
			data_len_error_proc();
		}
		break;
	case SI_MGC_GET_EM_PC_VOLUNDER_EVENT_INFO:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		times = amm_conf.volunder_event_times[em_no];

		if (data_len == SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_volunder_event_pc[times], data_len);
		} else {
			data_len_error_proc();
		}
		break;
	case SI_MGC_GET_EM_PC_PHASEBREAK_EVENT_INFO:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		times = amm_conf.phasebreak_event_times[em_no];

		if (data_len == SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_phasebreak_event_pc[times], data_len);
		} else {
			data_len_error_proc();
		}
		break;
	case SI_MGC_GET_EM_PC_CURLOSS_EVENT_INFO:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		times = amm_conf.curloss_event_times[em_no];

		if (data_len == SINKINFO_EM_PX_CURLOSS_EVENT_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_curloss_event_pc[times], data_len);
		} else {
			data_len_error_proc();
		}
		break;
	case SI_MGC_GET_EM_PC_CUROVER_EVENT_INFO:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		times = amm_conf.curover_event_times[em_no];

		if (data_len == SINKINFO_EM_PX_CURLOSS_EVENT_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_curover_event_pc[times], data_len);
		} else {
			data_len_error_proc();
		}
		break;
	case SI_MGC_GET_EM_PC_CURBREAK_EVENT_INFO:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		times = amm_conf.curbreak_event_times[em_no];

		if (data_len == SINKINFO_EM_PX_CURLOSS_EVENT_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_curbreak_event_pc[times], data_len);
		} else {
			data_len_error_proc();
		}
		break;
	case SI_MGC_GET_EM_METER_CLEAR_EVENT_INFO:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		times = amm_conf.meterclear_event_times[em_no];

		if (data_len == SINKINFO_EM_METER_CLEAR_EVENT_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_meterclear_event[times], data_len);
		} else {
			data_len_error_proc();
		}
		break;
	case SI_MGC_GET_EM_DEMAND_CLEAR_EVENT_INFO:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		times = amm_conf.demandclear_event_times[em_no];

		if (data_len == SINKINFO_EM_DEMAND_CLEAR_EVENT_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_demandclear_event[times], data_len);
		} else {
			data_len_error_proc();
		}
		break;
	case SI_MGC_GET_EM_PROGRAM_EVENT_INFO:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		times = amm_conf.program_event_times[em_no];

		if (data_len == SINKINFO_EM_PROGRAM_EVENT_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_program_event[times], data_len);
		} else {
			data_len_error_proc();
		}
		break;
	case SI_MGC_GET_EM_CALIBRATE_TIME_EVENT_INFO:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		times = amm_conf.calibratetime_event_times[em_no];

		if (data_len == SINKINFO_EM_CALIBRATE_TIME_EVENT_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_calibratetime_event[times], data_len);
		} else {
			data_len_error_proc();
		}
		break;
	case SI_MGC_GET_EM_REVERSE_REQ_VOL_EVENT_INFO:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		times = amm_conf.rseqvol_event_times[em_no];

		if (data_len == SINKINFO_EM_REVERSE_SEQVOL_EVENT_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_sreqvol_event[times], data_len);
		} else {
			data_len_error_proc();
		}
		break;
	case SI_MGC_GET_EM_REVERSE_REQ_CUR_EVENT_INFO:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		times = amm_conf.rseqcur_event_times[em_no];

		if (data_len == SINKINFO_EM_REVERSE_SEQVOL_EVENT_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_sreqcur_event[times], data_len);
		} else {
			data_len_error_proc();
		}
		break;
#endif

	case SI_MGC_GET_EM_DEV_INFO:		/* 获取电表中的‘不区分ABC三相’的数据 */
		if (data_len == SINKINFO_EM_DEV_DATA_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].em_dev_info, data_len);
		} else {
			data_len_error_proc();
		}
		break;

	case SI_MGC_GET_EM_MOMENT_FREEZE_INFO:		/* 获取电表中的‘不区分ABC三相’的数据 */
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read meter cfg data tbl fail\n", __FUNCTION__);
		}	
		times = amm_conf.moment_freeze_times[em_no];
		if (data_len == SINKINFO_EM_MOMENT_FREEZE_DATA_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_mom_freeze[times], data_len);
		} else {
			data_len_error_proc();
		}
		break;

	case SI_MGC_GET_EM_TIMING_FREEZE_INFO:		/* 获取电表中的‘不区分ABC三相’的数据 */
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read meter cfg data tbl fail\n", __FUNCTION__);
		}	
		times = amm_conf.timing_freeze_times[em_no];
		if (data_len == SINKINFO_EM_TIMING_FREEZE_DATA_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_time_freeze[times], data_len);
		} else {
			data_len_error_proc();
		}
		break;

	case SI_MGC_GET_EM_MAX_DEMAND_INFO:		/* 获取电表中的‘不区分ABC三相’的数据 */
		if (data_len == SINKINFO_EM_MAX_DEMAND_DATA_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_max_demand, data_len);
		} else {
			data_len_error_proc();
		}
		break;

	case SI_MGC_GET_EM_COPPER_IRON_LOSS_INFO:		/* 获取电表中的‘不区分ABC三相’的数据 */
		if (data_len == SINKINFO_EM_COPPER_IRON_LOSSES_DATA_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].si_em_copper_iron, data_len);
		} else {
			data_len_error_proc();
		}
		break;

	case SI_MGC_GET_PTC_PA_INFO:		/* 获取PT采集器收集到的数据 */
		if (data_len == SINKINFO_PTC_PX_DATA_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].pt_info.pt_pa, data_len);
		} else {
			data_len_error_proc();
		}
		break;

	case SI_MGC_GET_PTC_PB_INFO:		/* 获取CT采集器收集到的数据 */
		if (data_len == SINKINFO_PTC_PX_DATA_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].pt_info.pt_pb, data_len);
		} else {
			data_len_error_proc();
		}
		break;
	case SI_MGC_GET_PTC_PC_INFO:		/* 获取PT采集器收集到的数据 */
		if (data_len == SINKINFO_PTC_PX_DATA_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].pt_info.pt_pc, data_len);
		} else {
			data_len_error_proc();
		}
		break;
		
	case SI_MGC_GET_CTC_PA_INFO:		/* 获取CT采集器收集到的数据 */
		if(ptct_no == 1){
			if (data_len == SINKINFO_PTC_PX_DATA_SIZE) {
				rt_memcpy(data, &sinkinfo_all_em[em_no].ct_info.ct_pa, data_len);
			} else {
				data_len_error_proc();
			}
		}else if(ptct_no == 2){
			if (data_len == SINKINFO_PTC_PX_DATA_SIZE) {
				rt_memcpy(data, &sinkinfo_all_em[em_no].ct1_info.ct_pa, data_len);
			} else {
				data_len_error_proc();
			}
		}	
		break;
	case SI_MGC_GET_CTC_PB_INFO:		/* 获取CT采集器收集到的数据 */
		if(ptct_no == 1){
			if (data_len == SINKINFO_PTC_PX_DATA_SIZE) {
				rt_memcpy(data, &sinkinfo_all_em[em_no].ct_info.ct_pb, data_len);
			} else {
				data_len_error_proc();
			}
		}else if(ptct_no == 2){
			if (data_len == SINKINFO_PTC_PX_DATA_SIZE) {
				rt_memcpy(data, &sinkinfo_all_em[em_no].ct1_info.ct_pb, data_len);
			} else {
				data_len_error_proc();
			}
		}
		break;
	case SI_MGC_GET_CTC_PC_INFO:		/* 获取CT采集器收集到的数据 */
		if(ptct_no == 1){
			if (data_len == SINKINFO_PTC_PX_DATA_SIZE) {
				rt_memcpy(data, &sinkinfo_all_em[em_no].ct_info.ct_pc, data_len);
			} else {
				data_len_error_proc();
			}
		}else if(ptct_no == 2){
			if (data_len == SINKINFO_PTC_PX_DATA_SIZE) {
				rt_memcpy(data, &sinkinfo_all_em[em_no].ct1_info.ct_pc, data_len);
			} else {
				data_len_error_proc();
			}
		}
		break;
	case SI_MGC_GET_PTC_INFO:		/* 获取PT采集器收集到的数据 */
		if (data_len == SINKINFO_PTC_DATA_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].pt_info, data_len);
		} else {
			data_len_error_proc();
		}
		break;

	case SI_MGC_GET_CTC_INFO:		/* 获取CT采集器收集到的数据 */
		if(ptct_no == 1){
			if (data_len == SINKINFO_CTC_DATA_SIZE) {
				rt_memcpy(data, &sinkinfo_all_em[em_no].ct_info, data_len);
			} else {
				data_len_error_proc();
			}
		}else if(ptct_no == 2){
			if (data_len == SINKINFO_CTC_DATA_SIZE) {
				rt_memcpy(data, &sinkinfo_all_em[em_no].ct1_info, data_len);
			} else {
				data_len_error_proc();
			}
		}
		break;

	case SI_MGC_GET_PA_VSAMPLE:		/* 获取A相电压波形数据 */
		if (data_len == SINKINFO_PX_XSAMPLE_DATA_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].px_vi_sample.pa_vi_sample[0], data_len);
		} else {
			data_len_error_proc();
		}
		break;

	case SI_MGC_GET_PA_ISAMPLE:		/* 获取A相电流波形数据 */
		if (data_len == SINKINFO_PX_XSAMPLE_DATA_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].px_vi_sample.pa_vi_sample[1], data_len);
		} else {
			data_len_error_proc();
		}
		break;

	case SI_MGC_GET_PB_VSAMPLE:		/* 获取B相电压波形数据 */
		if (data_len == SINKINFO_PX_XSAMPLE_DATA_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].px_vi_sample.pb_vi_sample[0], data_len);
		} else {
			data_len_error_proc();
		}
		break;

	case SI_MGC_GET_PB_ISAMPLE:		/* 获取B相电流波形数据 */
		if (data_len == SINKINFO_PX_XSAMPLE_DATA_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].px_vi_sample.pb_vi_sample[1], data_len);
		} else {
			data_len_error_proc();
		}
		break;

	case SI_MGC_GET_PC_VSAMPLE:		/* 获取C相电压波形数据 */
		if (data_len == SINKINFO_PX_XSAMPLE_DATA_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].px_vi_sample.pc_vi_sample[0], data_len);
		} else {
			data_len_error_proc();
		}
		break;

	case SI_MGC_GET_PC_ISAMPLE:		/* 获取C相电流波形数据 */
		if (data_len == SINKINFO_PX_XSAMPLE_DATA_SIZE) {
			rt_memcpy(data, &sinkinfo_all_em[em_no].px_vi_sample.pc_vi_sample[1], data_len);
		} else {
			data_len_error_proc();
		}
		break;

	default:
		ret = SIE_CMD_NOT_SUPPORT;
		break;

	}
	
	rt_sem_release(&sinkinfo_sem);

	return ret;
}

