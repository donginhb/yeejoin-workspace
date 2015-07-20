/*
 ******************************************************************************
 * sink_info.c
 *
 * 2013-09-25,  creat by David, zhaoshaowei@yeejoin.com
 ******************************************************************************
 */
#include <rtdef.h>
#include <sink_info.h>
#include <rtthread.h>
#include <ade7880_api.h>
#include <master_app.h>

#include <ammeter.h>
#include <app-thread.h>
#include <board.h>
#include <rs485.h>
#include <misc_lib.h>
#include <debug_sw.h>
#include "lwip/private_trap.h"


#define sinkinfo_log(x)		printf_syn x

#define TEST_EM_PROTOC	0

/* 收集数据的时间间隔, 单位是10ms */
#if TEST_EM_PROTOC
#define UPDTAE_SINKINFO_TIME_GAP	(500)
#else
#define UPDTAE_SINKINFO_TIME_GAP	(100)
#endif

/* 两个645命令之间的最小间隔时间, 单位是ms */
#define MIN_TIME_GAP_OF_TWO_645CMD	(1100)

/* 使用7880检测谐波的时间间隔 */
#define HARMONIC_TIME_GAP_MS (5*60*1000)

#define USE_OLD_645_CODE	1

#define INVALID_DATA_GET_DATA_FAIL	(0)

#define INVALID_EM_MAP_VPORT_NO (0xff)

#define is_phony_em(index) !(rt_strncmp(PHONY_EM_SN, \
		register_em_info.registered_em_sn[index], PHONY_EM_SN_LEN))


struct rt_semaphore sinkinfo_sem;
struct sink_em_relative_info_st *sinkinfo_all_em;		/* 存储电表相关信息，以供IP网络获取 */
//static char electric_meter_sn_cache[16];
struct rt_semaphore px_sample_data_sem;

unsigned int sinkinfo_print_switch;

extern s16_t em_act_ee_inaccuracy;		/* 实时有功电能误差 */
extern s16_t em_react_ee_inaccuracy;		/* 实时无功电能误差 */

extern rt_mailbox_t wl_master_data_over485_mb;


static void rt_update_sinkinfo_entry(void* parameter);
static void rt_sinkinfo_7880_entry(void* parameter);
static void rt_sinkinfo_em_entry(void* parameter);
static void rt_sinkinfo_ptc_ctc_entry(void* parameter);

static void do_get_7880_data(union get_7880_data_st *p, int is_get_harmonic);
static void do_get_ptc_ctc_data(union sinkinfo_ptc_ctc_st *p);
static void do_get_em_data(union get_em_data_st *p);

static enum frame_error_e get_data_from_em_xprotocol(enum ammeter_cmd_e cmd, u32_t *bcd_4bytes, enum ammeter_uart_e port_485);

#if USE_OLD_645_CODE && !TEST_EM_PROTOC
static enum ammeter_protocal_e get_current_em_protocol(int cur_em_index);
#endif

//extern void rt_check_eenergy_entry(void* parameter);
extern volatile s32 XFVAR_HSCD_BUFFER[3];
extern volatile u32 AI_HSCD_BUFFER[40];
extern volatile u32 AV_HSCD_BUFFER[40];
extern volatile u32 BI_HSCD_BUFFER[40];
extern volatile u32 BV_HSCD_BUFFER[40];
extern volatile u32 CI_HSCD_BUFFER[40];  
extern volatile u32 CV_HSCD_BUFFER[40];

struct si_used_em_index_st {
	s8 prev_em_index;
	s8 cur_em_index;

	s8 is_phony_em;
	s8 reserve;
};

static struct si_used_em_index_st si_used_em_index;

static uint8_t is_had_finish_wl_netcfg;
struct register_em_info_st register_em_info;

struct rt_event sinkinfo_event_set;

#if EM_MASTER_DEV || EM_MULTI_MASTER_DEV
struct slave_emm_collector_info_st *slave_emmc_info;
#endif

//static int switch_to_cur_em(int cur_em_index);
static int get_next_collect_em_index(void);
static void get_one_em_all_sinkinfo(int cur_em_index);



void sinkinfo_ipc_init(void)
{
	rt_sem_init(&sinkinfo_sem, "sinkinfo", 1, RT_IPC_FLAG_PRIO);
	rt_sem_init(&px_sample_data_sem, "sampled", 1, RT_IPC_FLAG_PRIO);
	//rt_sem_init(&register_em_info_sem, "register", 1, RT_IPC_FLAG_PRIO);

	rt_event_init(&sinkinfo_event_set, "si_eve", RT_IPC_FLAG_PRIO);

	set_bit(sinkinfo_print_switch, SINKINFO_7880_GETDATA_FAIL_BIT);
	set_bit(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT);

	return;
}

void sinkinfo_init(void)
{
	rt_thread_t thread_h1, thread_h2, thread_h3, thread_h4;
#if 0!=IS_CHECK_EENERGY_USE_THREAD
	rt_thread_t thread_h5;
#endif

	sinkinfo_all_em = rt_calloc(NUM_OF_COLLECT_EM_MAX * sizeof(*sinkinfo_all_em), 1);
	if (NULL == sinkinfo_all_em) {
		printf_syn("alloc sinkinfo_all_em mem fail\n");
		return;
	}

	if (SUCC != si_init_wl_data_proc()) {
		printf_syn("si_init_wl_data_proc() fail\n");
		goto err;
	}

	update_em_reg_info();
#if EM_MASTER_DEV || EM_MULTI_MASTER_DEV
	slave_emmc_info = rt_calloc(sizeof(struct slave_emm_collector_info_st)*NUM_OF_SLAVE_EMM_MAX, 1);
	if (NULL == slave_emmc_info) {
		printf_syn("func:%s(), out of memory(slave_emmc_info %d bytes)\n", __FUNCTION__,
				sizeof(struct slave_emm_collector_info_st)*NUM_OF_SLAVE_EMM_MAX);
		goto err1;
	}
	read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_S_EMM_INFO, 0, slave_emmc_info);

	if (is_wl_netcfg_finish()) {
		is_had_finish_wl_netcfg = TRUE;
	} else {
		is_had_finish_wl_netcfg = FALSE;
	}
#elif EM_MULTI_SLAVE_DEV
	is_had_finish_wl_netcfg = TRUE;
#endif

	thread_h1 = rt_thread_create("upd_si", rt_update_sinkinfo_entry, RT_NULL, 2*1024, THREAD_PRIORITY_UPDATE_SINK_INFO, 10);
	if (thread_h1 != RT_NULL) {
		rt_thread_startup(thread_h1);
	} else {
		printf_syn("create rt_update_sinkinfo_entry fail\n");
		goto err2;
	}

	thread_h2 = rt_thread_create("si_7880", rt_sinkinfo_7880_entry, RT_NULL, 2*1024, THREAD_PRIORITY_GET_SINK_INFO, 30);
	if (thread_h2 != RT_NULL) {
		rt_thread_startup(thread_h2);
	} else {
		printf_syn("create rt_get_sinkinfo_entry fail\n");
		goto err3;
	}

	thread_h3 = rt_thread_create("si_em", rt_sinkinfo_em_entry, RT_NULL, 5*1024, THREAD_PRIORITY_GET_SINK_INFO, 50);
	if (thread_h3 != RT_NULL) {
		rt_thread_startup(thread_h3);
	} else {
		printf_syn("create rt_get_sinkinfo_entry fail\n");
		goto err4;
	}

	thread_h4 = rt_thread_create("si_pt_ct", rt_sinkinfo_ptc_ctc_entry, RT_NULL, 512, THREAD_PRIORITY_GET_SINK_INFO, 30);
	if (thread_h4 != RT_NULL) {
		rt_thread_startup(thread_h4);
	} else {
		printf_syn("create rt_get_sinkinfo_entry fail\n");
		goto err5;
	}

	printf_syn("creat upd_si, si_xx threads succ!\n");
#if 0!=IS_CHECK_EENERGY_USE_THREAD /* mark by David */
	thread_h5 = rt_thread_create("check_ee", rt_check_eenergy_entry, RT_NULL, /*512*/3*1024, 16, 50);
	if (thread_h5 != RT_NULL) {
		rt_thread_startup(thread_h5);
		printf_syn("rt_check_eenergy_entry had start...\n");
	} else {
		printf_syn("create rt_check_eenergy_entry fail\n");
		goto err6;
	}
#endif

	sinki_debug_body(("%s() succ\n", __FUNCTION__));
	return;
err6:
	rt_thread_delete(thread_h4);
err5:
	rt_thread_delete(thread_h3);
err4:
	rt_thread_delete(thread_h2);
err3:
	rt_thread_delete(thread_h1);
err2:
#if EM_MASTER_DEV || EM_MULTI_MASTER_DEV
	rt_free(slave_emmc_info);
err1:
	si_deinit_wl_data_proc();
#else
	si_deinit_wl_data_proc();
#endif
err:
	rt_free(sinkinfo_all_em);

	sinkinfo_log(("%s() fail\n", __FUNCTION__));
	rt_sem_detach(&sinkinfo_sem);
	rt_sem_detach(&px_sample_data_sem);

	return;
}

void si_set_is_had_finish_wl_netcfg_flag(int is_had_finish)
{
	if (is_had_finish) {
		is_had_finish_wl_netcfg = TRUE;
	} else {
		is_had_finish_wl_netcfg = FALSE;
	}

	return;
}


int update_em_reg_info(void)
{

	int i;
	struct electric_meter_reg_info_st *em_sns;

	em_sns = rt_malloc(sizeof(*em_sns));
	if (NULL == em_sns) {
		printf_syn("%s(), alloc mem fail\n", __FUNCTION__);
		return FAIL;
	}

	if (SUCC == get_em_reg_info(em_sns)) {
		for (i=0; i<NUM_OF_COLLECT_EM_MAX; ++i) {
			if ('\0' != em_sns->em_sn[i][0]) {
				set_bit(register_em_info.registered_em_vector, 1<<i);
				register_em_info.registered_em_map_vport[i] = em_sns->vport_no[i];
				rt_strncpy(register_em_info.registered_em_sn[i], em_sns->em_sn[i],
						sizeof(register_em_info.registered_em_sn[0]));
				rt_strncpy(register_em_info.registered_ptc_sn[i], em_sns->ptc_sn[i],
						sizeof(register_em_info.registered_ptc_sn[0]));
				rt_strncpy(register_em_info.registered_ctc_sn[i], em_sns->ctc_sn[i],
						sizeof(register_em_info.registered_ctc_sn[0]));
				rt_strncpy(register_em_info.registered_ctc1_sn[i], em_sns->ctc1_sn[i],
						sizeof(register_em_info.registered_ctc1_sn[0]));
				/* 电表所用规约在 ammete.c 中初始化时做判断， 然后存储在 此结构体中 */
	//			register_em_info.em_proto[i] =
//						get_electric_meter_protocol(register_em_info.registered_em_sn[i], AMMETER_UART2);
			} else {
				clr_bit(register_em_info.registered_em_vector, 1<<i);
				register_em_info.registered_em_map_vport[i]	= INVALID_EM_MAP_VPORT_NO;
				register_em_info.registered_em_sn[i][0]		= '\0';
				register_em_info.registered_ptc_sn[i][0]	= '\0';
				register_em_info.registered_ctc_sn[i][0]	= '\0';
				register_em_info.registered_ctc1_sn[i][0]	= '\0';
				register_em_info.em_proto[i]			= AP_PROTOCOL_UNKNOWN;
			}
		}
	}

	rt_free(em_sns);

	return SUCC;
}

int si_get_cur_em_index(void)
{
	return si_used_em_index.cur_em_index;
}

/*
 * 用于更新本地缓存数据sinkinfo，以便通过IP网络能够获取新数据（当前是通过snmp获取）
 * !!未实现采样数据的传输
 */
static void rt_update_sinkinfo_entry(void* parameter)
{
	rt_err_t err;
	rt_uint32_t e;
	rt_uint32_t set;

	say_thread_start();

#if TEST_EM_PROTOC
	dsw_set(0,0,1);
	dsw_set(0,1,1);
#endif
	while (1) {
		si_used_em_index.prev_em_index = si_used_em_index.cur_em_index;
		si_used_em_index.cur_em_index  = get_next_collect_em_index();
		if (si_used_em_index.cur_em_index < 0)
			goto nothing;

		si_used_em_index.is_phony_em = is_phony_em(si_used_em_index.cur_em_index);

#if TEST_EM_PROTOC
		set = SI_EVENT_GET_EM_OVER;
#else
		if (si_used_em_index.is_phony_em) {
			set = SI_EVENT_GET_7880_OVER;
		} else {
			set = SI_EVENT_GET_EM_OVER | SI_EVENT_GET_7880_OVER | SI_EVENT_EM_EEINACCURACY_OVER;

			/* 通过485总线获取无线主节点收集的数据, 并将这些数据更新到缓冲区中 */
#if EM_MASTER_DEV || EM_MULTI_MASTER_DEV
			if (is_had_finish_wl_netcfg) {
				if (RT_EOK != rt_mb_send(wl_master_data_over485_mb, MD_SEND_READ_MASTER_DATA_CMD)) {
					//printf_syn("send wl_master_data_over485_mb fail\n");
				} else {
					set |= SI_EVENT_GET_PT_CT_OVER;
				}
			}
#endif
		}
#endif
		sinki_debug_body(("%s() start, systicks:%u\n", __FUNCTION__, rt_tick_get()));

		get_one_em_all_sinkinfo(si_used_em_index.cur_em_index);

		err = rt_event_recv(&sinkinfo_event_set, set,
				RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &e);
		if (RT_EOK != err) {
			printf_syn("recv isr event error(%d)", err);
		} else {
			err = rt_sem_take(&sinkinfo_sem, RT_WAITING_FOREVER);
			if (RT_EOK == err) {
				sinkinfo_all_em[si_used_em_index.cur_em_index].em_dev_info.em_act_ee_inaccuracy	=
						em_act_ee_inaccuracy;
				sinkinfo_all_em[si_used_em_index.cur_em_index].em_dev_info.em_react_ee_inaccuracy =
						em_react_ee_inaccuracy;
				rt_sem_release(&sinkinfo_sem);
			} else {
				printf_syn("take sinkinfo_sem fail(%d)\n", err);
			}
		}

		sinki_debug_body(("%s() end, systicks:%u\n", __FUNCTION__, rt_tick_get()));
nothing:
		rt_thread_delay(get_ticks_of_ms(UPDTAE_SINKINFO_TIME_GAP * 10));
	}

	return;
}

static int copy_data_use_by_sinkinfo_buf(void *dst, unsigned dst_len,
		void *src, unsigned src_len, const char *file, int line)
{
	rt_err_t ret;

	if (NULL==dst || NULL==src || NULL==file) {
		printf_syn("%s() param is NULL\n", __FUNCTION__);
		return FAIL;
	}

	if (dst_len != src_len) {
		printf_syn("%s() copy data len must equal\n", __FUNCTION__);
		return FAIL;
	}

	ret = rt_sem_take(&sinkinfo_sem, RT_WAITING_FOREVER);
	if (RT_EOK == ret) {
		rt_memcpy(dst, src, dst_len);
		rt_sem_release(&sinkinfo_sem);
	} else {
		printf_syn("%s() take sinkinfo_sem fail(%d). caller pos:%s(), %d\n", __FUNCTION__, ret,
				file, line);

		return FAIL;
	}

	return SUCC;
}



/*
 * 该函数用于获取7880的采集数据
 * */
static void rt_sinkinfo_7880_entry(void* parameter)
{
	rt_err_t ret;
	rt_uint32_t e;
	static rt_tick_t harmonic_gap[NUM_OF_COLLECT_EM_MAX];
	union get_7880_data_st cache_7880;
	int i;

	say_thread_start();

	while (1) {
		ret = rt_event_recv(&sinkinfo_event_set, SI_EVENT_GET_7880_START,
				RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &e);
		if (RT_EOK != ret) {
			printf_syn("%s(), line:%d, recv event fail(%d)\n", __FUNCTION__, __LINE__, ret);
			rt_thread_delay(get_ticks_of_ms(1500));
			continue;
		}

		rt_thread_delay(get_ticks_of_ms(1000)); /* 等待信号稳定 */

		i = si_used_em_index.cur_em_index;

		sinki_debug_body(("%s() start, systicks:%u\n", __FUNCTION__, rt_tick_get()));
		if ((rt_tick_get() - harmonic_gap[i]) >= get_ticks_of_ms(HARMONIC_TIME_GAP_MS)) {
			harmonic_gap[i] = rt_tick_get();
			do_get_7880_data(&cache_7880, 1);
		} else {
			do_get_7880_data(&cache_7880, 0);
		}
		sinki_debug_body(("%s() end, systicks:%u\n", __FUNCTION__, rt_tick_get()));

		rt_event_send(&sinkinfo_event_set, SI_EVENT_GET_7880_OVER);

	}

	return;
}

/*
 * 该函数用于通过645获取电表的数据
 * */
static void rt_sinkinfo_em_entry(void* parameter)
{
	rt_err_t ret;
	rt_uint32_t e;
	rt_uint32_t state;
	rt_uint8_t chardata[12] = {0};
	union get_em_data_st cache_em;

	say_thread_start();

	while (1) {
		ret = rt_event_recv(&sinkinfo_event_set, SI_EVENT_GET_EM_START,
				RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &e);
		if (RT_EOK != ret) {
			printf_syn("%s(), line:%d, recv event fail(%d)\n", __FUNCTION__, __LINE__, ret);
			rt_thread_delay(get_ticks_of_ms(1500));
			continue;
		}

		sinki_debug_body(("%s() start, systicks:%u\n", __FUNCTION__, rt_tick_get()));
		do_get_em_data(&cache_em);
		sinki_debug_body(("%s() end, systicks:%u\n", __FUNCTION__, rt_tick_get()));

		if(SUCC == get_em_info(si_used_em_index.cur_em_index, 9, &state, chardata)){
			if(state & 0x01)
				timing_freeze_data_trap(si_used_em_index.cur_em_index) ;
		}
		
		//connect_state_trap() ;

		rt_event_send(&sinkinfo_event_set, SI_EVENT_GET_EM_OVER);	
	}

	return;
}

/*
 * 该函数用于获取pt、ct的采集数据
 * */
static void rt_sinkinfo_ptc_ctc_entry(void* parameter)
{
	rt_err_t ret;
	rt_uint32_t e;

	union sinkinfo_ptc_ctc_st cache_ptc_ctc;

	say_thread_start();

	while (1) {
		ret = rt_event_recv(&sinkinfo_event_set, SI_EVENT_GET_PT_CT_START,
				RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &e);
		if (RT_EOK != ret) {
			printf_syn("%s(), line:%d, recv event fail(%d)\n", __FUNCTION__, __LINE__, ret);
			rt_thread_delay(get_ticks_of_ms(1500));
			continue;
		}

		sinki_debug_body(("%s() start, systicks:%u\n", __FUNCTION__, rt_tick_get()));
		do_get_ptc_ctc_data(&cache_ptc_ctc);
		sinki_debug_body(("%s() end, systicks:%u\n", __FUNCTION__, rt_tick_get()));

		rt_event_send(&sinkinfo_event_set, SI_EVENT_GET_PT_CT_OVER);
	}

	return;
}


int switch_to_cur_em(int cur_em_index)
{
#if EM_MULTI_BASE
	led_no_blink_164_vector(get_run164led_bit_mask(PTU164_V,
			register_em_info.registered_em_map_vport[si_used_em_index.prev_em_index]));
	led_no_blink_164_vector(get_run164led_bit_mask(PTU164_I, si_used_em_index.prev_em_index));
	led_off_164_vector(get_run164led_bit_mask(PTU164_V,
			register_em_info.registered_em_map_vport[si_used_em_index.prev_em_index]));
	led_off_164_vector(get_run164led_bit_mask(PTU164_I, si_used_em_index.prev_em_index));

	/* 3-8, 4-16 */

	set_decoder_3to8_data(7);
	set_decoder_4to16_data(15);
	rt_thread_delay(get_ticks_of_ms(100)); /* 为已使用的通道之间切换建立死区 */

	set_decoder_3to8_data(register_em_info.registered_em_map_vport[cur_em_index]);	/* 电压选通 */
	set_decoder_4to16_data(cur_em_index);	/* 电流选通 */
	rt_thread_delay(get_ticks_of_ms(300));
	auto_set_powerup_workmode(cur_em_index); 
	reset_chlx_reg(cur_em_index);  

	led_blink_164_vector(get_run164led_bit_mask(PTU164_V,
			register_em_info.registered_em_map_vport[si_used_em_index.cur_em_index]));
	led_blink_164_vector(get_run164led_bit_mask(PTU164_I, si_used_em_index.cur_em_index));
#endif

	printf_syn("current v-port:%d, i-port:%d, em-sn:%s, ptc-sn:%s, ctc-sn:%s\n",
			register_em_info.registered_em_map_vport[cur_em_index]+1, cur_em_index+1,
			register_em_info.registered_em_sn[cur_em_index],
			register_em_info.registered_ptc_sn[cur_em_index],
			register_em_info.registered_ctc_sn[cur_em_index]);

	return 0;
}

/*
 * 返回值范围是[0, NUM_OF_COLLECT_EM_MAX)
 * */
static int get_next_collect_em_index(void)
{
	static int cur_index = -1;
	int i;

	if (0 == register_em_info.registered_em_vector) {
		cur_index = -1;
		return -1;
	}
again:
	for (i=cur_index+1; i<NUM_OF_COLLECT_EM_MAX; ++i) {
		if (is_bit_set(register_em_info.registered_em_vector, 1<<i)) {
			cur_index = i;
			break;
		}
	}

	if (i >= NUM_OF_COLLECT_EM_MAX) {
		cur_index = -1;
		goto again;
	}

	return cur_index;
}

static void get_one_em_all_sinkinfo(int cur_em_index)
{
	rt_uint32_t set;

	switch_to_cur_em(cur_em_index);

	printf_syn("will update sink-info data...\n");

#if TEST_EM_PROTOC
	set = SI_EVENT_GET_EM_START;
#else
	/* get_em_sn_from_645(electric_meter_sn_cache, RS485_PORT_USED_BY_645); */
	if (si_used_em_index.is_phony_em) {
		set = SI_EVENT_GET_7880_START;
	} else {
		set = SI_EVENT_GET_EM_START | SI_EVENT_GET_7880_START | SI_EVENT_EM_EEINACCURACY_START;
		if (is_had_finish_wl_netcfg) {
			set |= SI_EVENT_GET_PT_CT_START;
		}
	}
#endif
	rt_event_send(&sinkinfo_event_set, set);

	return;
}


static void do_get_7880_data(union get_7880_data_st *p, int is_get_harmonic)
{
	int i;

	signed char *vs_buf, *is_buf;
	u32 cnt, tempv, tempi;
	volatile u32 *pv, *pi;

	if (NULL == p) {
		printf_syn("func:%s(), param is NULL\n", __FUNCTION__);
		return;
	}
#if 0==IS_CHECK_EENERGY_USE_THREAD
	check_electric_energy();
#endif
	px_vi_sample_reac_p_hsdc();
	vi_vector_graph_sampl();

	/* 获取电表侧a相所有abc三项独立的参数 */
	rt_memset(&p->si_ind_px, 0, sizeof(p->si_ind_px));
#if 0
	p->si_ind_px.vx   = 1;
	p->si_ind_px.ix   = 2;
	p->si_ind_px.hzx  = 3;
	p->si_ind_px.phx  = 4;
	p->si_ind_px.apx  = 5;
	p->si_ind_px.rapx = 6;
	p->si_ind_px.appx = 7;
	p->si_ind_px.pfx  = 8;
	p->si_ind_px.vdx  = 9;
	p->si_ind_px.cdx  = 10;
	printf_syn("func:%s(), line:%d\n", __FUNCTION__, __LINE__);
#else
	if (SUCC!=px_virtual_mode_voltage (PHASE_A, &(p->si_ind_px.vx))
			|| SUCC!=px_virtual_mode_current (PHASE_A, &(p->si_ind_px.ix)) ) {
		if (is_bit_set(sinkinfo_print_switch, SINKINFO_7880_GETDATA_FAIL_BIT))
			printf_syn("get pa vol/curr fail");
	}

	p->si_ind_px.hzx  = px_frequency_mode_signal(PHASE_A);
	p->si_ind_px.phx  = px_phase_mode_position(PHASE_A);
	p->si_ind_px.viphx  = vg_st.viap;
	p->si_ind_px.apx  = px_active_mode_power(PHASE_A);
#if ADE7880_USE_I2C_HSDC
	p->si_ind_px.rapx = XFVAR_HSCD_BUFFER[0];
#else
	p->si_ind_px.rapx = px_reactive_mode_power(PHASE_A);
#endif
	p->si_ind_px.appx = px_apparent_mode_power(PHASE_A);
	p->si_ind_px.pfx  = px_factor_mode_power(PHASE_A);
	p->si_ind_px.vdx  = px_voltage_distortion(PHASE_A);
	p->si_ind_px.cdx  = px_current_distortion(PHASE_A);
#endif
	copy_data_use_by_sinkinfo_buf(&sinkinfo_all_em[si_used_em_index.cur_em_index].si_emc_ind_pa,
			sizeof(sinkinfo_all_em[0].si_emc_ind_pa), &p->si_ind_px,
			sizeof(p->si_ind_px), __FUNCTION__, __LINE__);

//	printf_syn("func:%s, line:%d,%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n", __FUNCTION__, __LINE__,
//			p->si_ind_px.vx, p->si_ind_px.ix,
//			p->si_ind_px.hzx, p->si_ind_px.phx,
//			p->si_ind_px.viphx, p->si_ind_px.apx,
//			p->si_ind_px.rapx, p->si_ind_px.appx,
//			p->si_ind_px.pfx, p->si_ind_px.vdx,
//			p->si_ind_px.cdx);

	/* 获取电表侧b相所有abc三项独立的参数 */
	rt_memset(&p->si_ind_px, 0, sizeof(p->si_ind_px));
	if (SUCC!=px_virtual_mode_voltage (PHASE_B, &(p->si_ind_px.vx))
			|| SUCC!=px_virtual_mode_current (PHASE_B, &(p->si_ind_px.ix))) {
		if (is_bit_set(sinkinfo_print_switch, SINKINFO_7880_GETDATA_FAIL_BIT))
			printf_syn("get pb vol/curr fail");
	}

	if(0 != connect33_data){
		p->si_ind_px.hzx  = 0;
		p->si_ind_px.phx  = 0;
		p->si_ind_px.viphx  = 0;
		p->si_ind_px.apx  = 0;
		p->si_ind_px.rapx = 0;
		p->si_ind_px.appx = 0;
		p->si_ind_px.pfx  = 0;
		p->si_ind_px.vdx  = 0;
		p->si_ind_px.cdx  = 0;
	} else {
		p->si_ind_px.hzx  = px_frequency_mode_signal(PHASE_B);
		p->si_ind_px.phx  = px_phase_mode_position  (PHASE_B);
		p->si_ind_px.viphx  = vg_st.vibp;
		p->si_ind_px.apx  = px_active_mode_power	 (PHASE_B);
#if ADE7880_USE_I2C_HSDC
		p->si_ind_px.rapx = XFVAR_HSCD_BUFFER[1];
#else
		p->si_ind_px.rapx = px_reactive_mode_power  (PHASE_B);
#endif
		p->si_ind_px.appx = px_apparent_mode_power  (PHASE_B);
		p->si_ind_px.pfx  = px_factor_mode_power    (PHASE_B);
		p->si_ind_px.vdx  = px_voltage_distortion   (PHASE_B);
		p->si_ind_px.cdx  = px_current_distortion   (PHASE_B);
	}
	copy_data_use_by_sinkinfo_buf(&sinkinfo_all_em[si_used_em_index.cur_em_index].si_emc_ind_pb,
			sizeof(sinkinfo_all_em[0].si_emc_ind_pb), &p->si_ind_px,
			sizeof(p->si_ind_px), __FUNCTION__, __LINE__);



	/* 获取电表侧c相所有abc三项独立的参数 */
	rt_memset(&p->si_ind_px, 0, sizeof(p->si_ind_px));
	if (SUCC!=px_virtual_mode_voltage (PHASE_C, &(p->si_ind_px.vx))
			|| SUCC!=px_virtual_mode_current (PHASE_C, &(p->si_ind_px.ix))) {
		if (is_bit_set(sinkinfo_print_switch, SINKINFO_7880_GETDATA_FAIL_BIT))
			printf_syn("get pc vol/curr fail");
	}

	p->si_ind_px.hzx  = px_frequency_mode_signal(PHASE_C);
	p->si_ind_px.phx  = px_phase_mode_position  (PHASE_C);
	p->si_ind_px.viphx  = vg_st.vicp;
	p->si_ind_px.apx  = px_active_mode_power	 (PHASE_C);
#if ADE7880_USE_I2C_HSDC
	p->si_ind_px.rapx = XFVAR_HSCD_BUFFER[2];
#else
	p->si_ind_px.rapx = px_reactive_mode_power  (PHASE_C);
#endif
	p->si_ind_px.appx = px_apparent_mode_power  (PHASE_C);
	p->si_ind_px.pfx  = px_factor_mode_power    (PHASE_C);
	p->si_ind_px.vdx  = px_voltage_distortion   (PHASE_C);
	p->si_ind_px.cdx  = px_current_distortion   (PHASE_C);

	copy_data_use_by_sinkinfo_buf(&sinkinfo_all_em[si_used_em_index.cur_em_index].si_emc_ind_pc,
			sizeof(sinkinfo_all_em[0].si_emc_ind_pc), &p->si_ind_px,
			sizeof(p->si_ind_px), __FUNCTION__, __LINE__);


	/* a相电压、电流波形采样值 */
	rt_memset(p->px_vi_sample, 0, sizeof(p->px_vi_sample));
	pv = AV_HSCD_BUFFER;
	pi = AI_HSCD_BUFFER;
	vs_buf = p->px_vi_sample[0];
	is_buf = p->px_vi_sample[1];
	for (cnt=0; cnt<SINK_INFO_PX_SAMPLE_DOT_NUM; ++cnt) {
		tempv   = *pv++;
		tempi	= *pi++;

		*vs_buf++ = tempv >> 16;
		*vs_buf++ = tempv >> 8;
		*vs_buf++ = tempv;
		*is_buf++ = tempi >> 16;
		*is_buf++ = tempi >> 8;
		*is_buf++ = tempi;
	}

	copy_data_use_by_sinkinfo_buf(sinkinfo_all_em[si_used_em_index.cur_em_index].px_vi_sample.pa_vi_sample,
			sizeof(sinkinfo_all_em[0].px_vi_sample.pa_vi_sample), p->px_vi_sample,
			sizeof(p->px_vi_sample), __FUNCTION__, __LINE__);


	/* b相电压、电流波形采样值 */
	rt_memset(p->px_vi_sample, 0, sizeof(p->px_vi_sample));
	pv = BV_HSCD_BUFFER;
	pi = BI_HSCD_BUFFER;
	vs_buf = p->px_vi_sample[0];
	is_buf = p->px_vi_sample[1];
	for (cnt=0; cnt<SINK_INFO_PX_SAMPLE_DOT_NUM; ++cnt) {
		tempv   = *pv++;
		tempi	= *pi++;

		*vs_buf++ = tempv >> 16;
		*vs_buf++ = tempv >> 8;
		*vs_buf++ = tempv;
		*is_buf++ = tempi >> 16;
		*is_buf++ = tempi >> 8;
		*is_buf++ = tempi;
	}

	copy_data_use_by_sinkinfo_buf(sinkinfo_all_em[si_used_em_index.cur_em_index].px_vi_sample.pb_vi_sample,
			sizeof(sinkinfo_all_em[0].px_vi_sample.pb_vi_sample), p->px_vi_sample,
			sizeof(p->px_vi_sample), __FUNCTION__, __LINE__);

	/* c相电压、电流波形采样值 */
	rt_memset(p->px_vi_sample, 0, sizeof(p->px_vi_sample));
	pv = CV_HSCD_BUFFER;
	pi = CI_HSCD_BUFFER;
	vs_buf = p->px_vi_sample[0];
	is_buf = p->px_vi_sample[1];
	for (cnt=0; cnt<SINK_INFO_PX_SAMPLE_DOT_NUM; ++cnt) {
		tempv   = *pv++;
		tempi	= *pi++;

		*vs_buf++ = tempv >> 16;
		*vs_buf++ = tempv >> 8;
		*vs_buf++ = tempv;
		*is_buf++ = tempi >> 16;
		*is_buf++ = tempi >> 8;
		*is_buf++ = tempi;
	}

	copy_data_use_by_sinkinfo_buf(sinkinfo_all_em[si_used_em_index.cur_em_index].px_vi_sample.pc_vi_sample,
			sizeof(sinkinfo_all_em[0].px_vi_sample.pc_vi_sample), p->px_vi_sample,
			sizeof(p->px_vi_sample), __FUNCTION__, __LINE__);

	/* 获取电表侧铜损、铁损参数, by 7880 */
	rt_memset(&p->emc_copper_iron_info, 0, sizeof(p->emc_copper_iron_info));
	/* 暂不支持，返回无效值 */
	si_fill_emc_ciloss_when_get_data_fail(&p->emc_copper_iron_info);

	copy_data_use_by_sinkinfo_buf(&sinkinfo_all_em[si_used_em_index.cur_em_index].emc_copper_iron_info,
			sizeof(sinkinfo_all_em[0].emc_copper_iron_info), &p->emc_copper_iron_info,
			sizeof(p->emc_copper_iron_info), __FUNCTION__, __LINE__);


	/* 谐波比较耗费时间, mark by David */
	if (0 != is_get_harmonic) {
		rt_memset(p->si_harmonic_px, 0, sizeof(p->si_harmonic_px));
		for(i=0; i<EMC_HARMONIC_TIMES_MAX; i++){
			px_harmonic_mode_parameter(PHASE_A, i+1);
			p->si_harmonic_px[i].vrms = harmonic.vrms;
			p->si_harmonic_px[i].irms = harmonic.irms;
			p->si_harmonic_px[i].watt = harmonic.watt;
		}
		copy_data_use_by_sinkinfo_buf(&sinkinfo_all_em[si_used_em_index.cur_em_index].si_harmonic_pa,
				sizeof(sinkinfo_all_em[0].si_harmonic_pa), p->si_harmonic_px,
				sizeof(p->si_harmonic_px), __FUNCTION__, __LINE__);

		rt_memset(p->si_harmonic_px, 0, sizeof(p->si_harmonic_px));
		for(i=0; i<EMC_HARMONIC_TIMES_MAX; i++){
			px_harmonic_mode_parameter(PHASE_B, i+1);
			p->si_harmonic_px[i].vrms = harmonic.vrms;
			p->si_harmonic_px[i].irms = harmonic.irms;
			p->si_harmonic_px[i].watt = harmonic.watt;
		}
		copy_data_use_by_sinkinfo_buf(&sinkinfo_all_em[si_used_em_index.cur_em_index].si_harmonic_pb,
				sizeof(sinkinfo_all_em[0].si_harmonic_pb), p->si_harmonic_px,
				sizeof(p->si_harmonic_px), __FUNCTION__, __LINE__);

		rt_memset(p->si_harmonic_px, 0, sizeof(p->si_harmonic_px));
		for(i=0; i<EMC_HARMONIC_TIMES_MAX; i++){
			px_harmonic_mode_parameter(PHASE_C, i+1);
			p->si_harmonic_px[i].vrms = harmonic.vrms;
			p->si_harmonic_px[i].irms = harmonic.irms;
			p->si_harmonic_px[i].watt = harmonic.watt;
		}
		copy_data_use_by_sinkinfo_buf(&sinkinfo_all_em[si_used_em_index.cur_em_index].si_harmonic_pc,
				sizeof(sinkinfo_all_em[0].si_harmonic_pc), p->si_harmonic_px,
				sizeof(p->si_harmonic_px), __FUNCTION__, __LINE__);
	}

	return;
}


#if 1
static int si_get_local_ptc_ctc_sn(enum sink_data_dev_type_e t, char *ptc_ctc_sn, int len)
{
	if (NULL==ptc_ctc_sn) {
		printf_syn("func:%s(), param is NULL\n", __FUNCTION__);
		return FAIL;
	}

	switch (t) {
	case SDDT_PT:
		rt_strncpy(ptc_ctc_sn, register_em_info.registered_ptc_sn[si_used_em_index.cur_em_index], len);
		break;

	case SDDT_CT:
		rt_strncpy(ptc_ctc_sn, register_em_info.registered_ctc_sn[si_used_em_index.cur_em_index], len);
		break;
		
	case SDDT_CT1:
		rt_strncpy(ptc_ctc_sn, register_em_info.registered_ctc1_sn[si_used_em_index.cur_em_index], len);
		break;

	default:
		printf_syn("func:%s(), param error\n", __FUNCTION__);
		ptc_ctc_sn[0] = '\0';
		return FAIL;
	}

	return SUCC;
}

static void do_get_ptc_ctc_data(union sinkinfo_ptc_ctc_st *p)
{
	rt_tick_t time_stamp;
	char ptc_ctc_sn[DEV_SN_BUF_STRING_WITH_NUL_LEN_MAX];

	if (NULL==p) {
		printf_syn("func:%s(), param is NULL\n", __FUNCTION__);
		return ;
	}

	si_get_local_ptc_ctc_sn(SDDT_PT, ptc_ctc_sn, sizeof(ptc_ctc_sn));
	if (SUCC != si_get_item_in_wl_data(ptc_ctc_sn, p, &time_stamp)) {
		rt_memset(&p->ptc_data, 0, sizeof(p->ptc_data));
		sinki_debug_body(("func:%s, line:%d, get pt-wl data error, sn:%s\n", __FUNCTION__, __LINE__,
				ptc_ctc_sn));
	}
	copy_data_use_by_sinkinfo_buf(&sinkinfo_all_em[si_used_em_index.cur_em_index].pt_info,
			sizeof(sinkinfo_all_em[0].pt_info), &p->ptc_data,
			sizeof(p->ptc_data), __FUNCTION__, __LINE__);

	si_get_local_ptc_ctc_sn(SDDT_CT, ptc_ctc_sn, sizeof(ptc_ctc_sn));
	if (SUCC != si_get_item_in_wl_data(ptc_ctc_sn, p, &time_stamp)) {
		rt_memset(&p->ctc_data, 0, sizeof(p->ctc_data));
		sinki_debug_body(("func:%s, line:%d, get ct-wl data error, sn:%s\n", __FUNCTION__, __LINE__,
				ptc_ctc_sn));
	}
	copy_data_use_by_sinkinfo_buf(&sinkinfo_all_em[si_used_em_index.cur_em_index].ct_info,
			sizeof(sinkinfo_all_em[0].ct_info), &p->ctc_data,
			sizeof(p->ctc_data), __FUNCTION__, __LINE__);

	si_get_local_ptc_ctc_sn(SDDT_CT1, ptc_ctc_sn, sizeof(ptc_ctc_sn));
	if (SUCC != si_get_item_in_wl_data(ptc_ctc_sn, p, &time_stamp)) {
		rt_memset(&p->ctc1_data, 0, sizeof(p->ctc1_data));
		sinki_debug_body(("func:%s, line:%d, get ct1-wl data error, sn:%s\n", __FUNCTION__, __LINE__,
				ptc_ctc_sn));
	}
	copy_data_use_by_sinkinfo_buf(&sinkinfo_all_em[si_used_em_index.cur_em_index].ct1_info,
			sizeof(sinkinfo_all_em[0].ct1_info), &p->ctc1_data,
			sizeof(p->ctc1_data), __FUNCTION__, __LINE__);

	return;
}

#endif



static rt_uint8_t buf_of_recv_645_rep[256];
/* 0x99是广播地址, 0xaa是通配地址 */
#if 1
//static rt_uint8_t broadcast_addr_645[] = {0x99, 0x99, 0x99, 0x99, 0x99, 0x99};
#else
static rt_uint8_t broadcast_addr_645[] = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
#endif

#if !TEST_EM_PROTOC

#if 0
static void do_get_em_data_harmonic(union get_em_data_st *p, rt_uint8_t *meteraddr)
{
	int i, j;
	rt_uint32_t data_len = 0;
	enum frame_error_e err_em_frame;

//	for(i = 0; i< EMC_HARMONIC_TIMES_MAX; i++){
//		p->si_em_vol_harmonic_px.vol_harmonic[i] = INVLIDE_DATAL;
//		p->si_em_cur_harmonic_px.cur_harmonic[i] = INVLIDE_DATAL;
//	}

	/* A相电压谐波 */
	err_em_frame = get_power_data_from_ammeter(meteraddr, AC_A_VOLTAGE_HARMONIC, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		for(i = 0; i< EMC_HARMONIC_TIMES_MAX; i++){
//			p->harmonic_px.harmonic_data[i] = INVLIDE_DATAL;
//		}

		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em pa power factor fail(%d)\n", err_em_frame));
	} else {
		j = 0;
		for (i=0; i<EMC_HARMONIC_TIMES_MAX; ++i) {
			p->harmonic_px.harmonic_data[i] = buf_of_recv_645_rep[j+1]<<8
					| buf_of_recv_645_rep[j];
			j += 2;
		}
		copy_data_use_by_sinkinfo_buf(&sinkinfo_all_em[si_used_em_index.cur_em_index].si_em_vol_harmonic_pa,
				sizeof(sinkinfo_all_em[0].si_em_vol_harmonic_pa), &p->harmonic_px,
				sizeof(p->harmonic_px), __FUNCTION__, __LINE__);

	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	/* A相电流谐波 */
	err_em_frame = get_power_data_from_ammeter(meteraddr, AC_A_CURRENT_HARMONIC, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		for(i = 0; i< EMC_HARMONIC_TIMES_MAX; i++){
//			p->harmonic_px.harmonic_data[i] = INVLIDE_DATAL;
//		}

		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em pa power factor fail(%d)\n", err_em_frame));
	} else {
		j = 0;
		for (i=0; i<EMC_HARMONIC_TIMES_MAX; ++i) {
			p->harmonic_px.harmonic_data[i] = buf_of_recv_645_rep[j+1]<<8
					| buf_of_recv_645_rep[j];
			j += 2;
		}

		copy_data_use_by_sinkinfo_buf(&sinkinfo_all_em[si_used_em_index.cur_em_index].si_em_cur_harmonic_pa,
				sizeof(sinkinfo_all_em[0].si_em_cur_harmonic_pa), &p->harmonic_px,
				sizeof(p->harmonic_px), __FUNCTION__, __LINE__);
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));


	/* B相电压谐波 */
	err_em_frame = get_power_data_from_ammeter(meteraddr, AC_B_VOLTAGE_HARMONIC, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		for(i = 0; i< EMC_HARMONIC_TIMES_MAX; i++){
//			p->harmonic_px.harmonic_data[i] = INVLIDE_DATAL;
//		}

		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em pa power factor fail(%d)\n", err_em_frame));
	} else {
		j = 0;
		for (i=0; i<EMC_HARMONIC_TIMES_MAX; ++i) {
			p->harmonic_px.harmonic_data[i] = buf_of_recv_645_rep[j+1]<<8
					| buf_of_recv_645_rep[j];
			j += 2;
		}
		copy_data_use_by_sinkinfo_buf(&sinkinfo_all_em[si_used_em_index.cur_em_index].si_em_vol_harmonic_pb,
				sizeof(sinkinfo_all_em[0].si_em_vol_harmonic_pb), &p->harmonic_px,
				sizeof(p->harmonic_px), __FUNCTION__, __LINE__);

	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	/* B相电流谐波 */
	err_em_frame = get_power_data_from_ammeter(meteraddr, AC_B_CURRENT_HARMONIC, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		for(i = 0; i< EMC_HARMONIC_TIMES_MAX; i++){
//			p->harmonic_px.harmonic_data[i] = INVLIDE_DATAL;
//		}

		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em pa power factor fail(%d)\n", err_em_frame));
	} else {
		j = 0;
		for (i=0; i<EMC_HARMONIC_TIMES_MAX; ++i) {
			p->harmonic_px.harmonic_data[i] = buf_of_recv_645_rep[j+1]<<8
					| buf_of_recv_645_rep[j];
			j += 2;
		}

		copy_data_use_by_sinkinfo_buf(&sinkinfo_all_em[si_used_em_index.cur_em_index].si_em_cur_harmonic_pb,
				sizeof(sinkinfo_all_em[0].si_em_cur_harmonic_pb), &p->harmonic_px,
				sizeof(p->harmonic_px), __FUNCTION__, __LINE__);
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	/* C相电压谐波 */
	err_em_frame = get_power_data_from_ammeter(meteraddr, AC_C_VOLTAGE_HARMONIC, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		for(i = 0; i< EMC_HARMONIC_TIMES_MAX; i++){
//			p->harmonic_px.harmonic_data[i] = INVLIDE_DATAL;
//		}

		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em pa power factor fail(%d)\n", err_em_frame));
	} else {
		j = 0;
		for (i=0; i<EMC_HARMONIC_TIMES_MAX; ++i) {
			p->harmonic_px.harmonic_data[i] = buf_of_recv_645_rep[j+1]<<8
					| buf_of_recv_645_rep[j];
			j += 2;
		}
		copy_data_use_by_sinkinfo_buf(&sinkinfo_all_em[si_used_em_index.cur_em_index].si_em_vol_harmonic_pc,
				sizeof(sinkinfo_all_em[0].si_em_vol_harmonic_pc), &p->harmonic_px,
				sizeof(p->harmonic_px), __FUNCTION__, __LINE__);

	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	/* C相电流谐波 */
	err_em_frame = get_power_data_from_ammeter(meteraddr, AC_C_CURRENT_HARMONIC, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		for(i = 0; i< EMC_HARMONIC_TIMES_MAX; i++){
//			p->harmonic_px.harmonic_data[i] = INVLIDE_DATAL;
//		}

		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em pa power factor fail(%d)\n", err_em_frame));
	} else {
		j = 0;
		for (i=0; i<EMC_HARMONIC_TIMES_MAX; ++i) {
			p->harmonic_px.harmonic_data[i] = buf_of_recv_645_rep[j+1]<<8
					| buf_of_recv_645_rep[j];
			j += 2;
		}

		copy_data_use_by_sinkinfo_buf(&sinkinfo_all_em[si_used_em_index.cur_em_index].si_em_cur_harmonic_pc,
				sizeof(sinkinfo_all_em[0].si_em_cur_harmonic_pc), &p->harmonic_px,
				sizeof(p->harmonic_px), __FUNCTION__, __LINE__);
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	return;

}

static void do_get_em_data_copper_iron(union get_em_data_st *p, rt_uint32_t protocal, rt_uint8_t *meteraddr)
{
	enum frame_error_e err_em_frame;

	/* 获取电表中铜损、铁损的参数, by 645 */
//	rt_memset(&p->si_em_copper_iron, 0, sizeof(p->si_em_copper_iron));
	copy_data_use_by_sinkinfo_buf(&p->si_em_copper_iron, sizeof(p->si_em_copper_iron),
			&sinkinfo_all_em[si_used_em_index.cur_em_index].si_em_copper_iron,
			sizeof(sinkinfo_all_em[0].si_em_copper_iron),
			__FUNCTION__, __LINE__);

	/* 铜损有功总电能补偿量, 返回4bytes数据 */
	if(protocal == AP_PROTOCOL_645_2007){
		err_em_frame = get_data_from_em_xprotocol(AC_TOTAL_COPPER_LOSS_ACTIVE_POWER,
				&p->si_em_copper_iron.copper_apxT, RS485_PORT_USED_BY_645);
		if (FRAME_E_OK != err_em_frame) {
//			p->si_em_copper_iron.copper_apxT = INVLIDE_DATAL;
			if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
				sinkinfo_log(("get MAX demand total fail(%d)\n", err_em_frame));
		}
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* A相铜损有功电能补偿量, 返回4bytes数据 */
		err_em_frame = get_data_from_em_xprotocol(AC_A_COPPER_LOSS_ACTIVE_POWER,
				&p->si_em_copper_iron.copper_apxA, RS485_PORT_USED_BY_645);
		if (FRAME_E_OK != err_em_frame) {
//			p->si_em_copper_iron.copper_apxA = INVLIDE_DATAL;
			if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
				sinkinfo_log(("get em MAX demand total time fail(%d)\n", err_em_frame));
		}
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* B 相铜损有功电能补偿量, 返回4bytes数据 */
		err_em_frame = get_data_from_em_xprotocol(AC_B_COPPER_LOSS_ACTIVE_POWER,
				&p->si_em_copper_iron.copper_apxB, RS485_PORT_USED_BY_645);
		if (FRAME_E_OK != err_em_frame) {
//			p->si_em_copper_iron.copper_apxB = INVLIDE_DATAL;

			if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
				sinkinfo_log(("get em MAX demand total time fail(%d)\n", err_em_frame));
		}
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/*C 相铜损有功电能补偿量 , 返回2bytes数据 */
		err_em_frame = get_data_from_em_xprotocol(AC_C_COPPER_LOSS_ACTIVE_POWER,
				&p->si_em_copper_iron.copper_apxC, RS485_PORT_USED_BY_645);
		if (FRAME_E_OK != err_em_frame) {
//			p->si_em_copper_iron.copper_apxC = INVLIDE_DATAL;

			if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
				sinkinfo_log(("get em MAX demand total time fail(%d)\n", err_em_frame));
		}
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/*铁损有功总电能补偿量 , 返回4bytes数据 */
		err_em_frame = get_data_from_em_xprotocol(AC_TOTAL_IRON_LOSS_ACTIVE_POWER,
				&p->si_em_copper_iron.iron_apxT, RS485_PORT_USED_BY_645);
		if (FRAME_E_OK != err_em_frame) {
//			p->si_em_copper_iron.iron_apxT = INVLIDE_DATAL;

			if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
				sinkinfo_log(("get em MAX demand total time fail(%d)\n", err_em_frame));
		}
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/*A相铁损有功电能补偿量 , 返回4bytes数据 */
		err_em_frame = get_data_from_em_xprotocol(AC_A_IRON_LOSS_ACTIVE_POWER,
				&p->si_em_copper_iron.iron_apxA, RS485_PORT_USED_BY_645);
		if (FRAME_E_OK != err_em_frame) {
//			p->si_em_copper_iron.iron_apxA = INVLIDE_DATAL;

			if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
				sinkinfo_log(("get em MAX demand total time fail(%d)\n", err_em_frame));
		}
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* B 相铁损有功电能补偿量, 返回4bytes数据 */
		err_em_frame = get_data_from_em_xprotocol(AC_B_IRON_LOSS_ACTIVE_POWER,
				&p->si_em_copper_iron.iron_apxB, RS485_PORT_USED_BY_645);
		if (FRAME_E_OK != err_em_frame) {
//			p->si_em_copper_iron.iron_apxB = INVLIDE_DATAL;

			if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
				sinkinfo_log(("get em MAX demand total time fail(%d)\n", err_em_frame));
		}
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* C 相铁损有功电能补偿量, 返回4bytes数据 */
		err_em_frame = get_data_from_em_xprotocol(AC_C_IRON_LOSS_ACTIVE_POWER,
				&p->si_em_copper_iron.iron_apxC, RS485_PORT_USED_BY_645);
		if (FRAME_E_OK != err_em_frame) {
//			p->si_em_copper_iron.iron_apxC = INVLIDE_DATAL;

			if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
				sinkinfo_log(("get em MAX demand total time fail(%d)\n", err_em_frame));
		}
	}else if(protocal == AP_PROTOCOL_645_1997 || protocal == AP_PROTOCOL_EDMI || protocal == AP_PROTOCOL_UNKNOWN){
		p->si_em_copper_iron.copper_apxT = INVLIDE_DATAL;
		p->si_em_copper_iron.copper_apxA = INVLIDE_DATAL;
		p->si_em_copper_iron.copper_apxB = INVLIDE_DATAL;
		p->si_em_copper_iron.copper_apxC = INVLIDE_DATAL;
		p->si_em_copper_iron.iron_apxT = INVLIDE_DATAL;
		p->si_em_copper_iron.iron_apxA = INVLIDE_DATAL;
		p->si_em_copper_iron.iron_apxB = INVLIDE_DATAL;
		p->si_em_copper_iron.iron_apxC = INVLIDE_DATAL;
	}

	copy_data_use_by_sinkinfo_buf(&sinkinfo_all_em[si_used_em_index.cur_em_index].si_em_copper_iron,
			sizeof(sinkinfo_all_em[0].si_em_copper_iron), &p->si_em_copper_iron,
			sizeof(p->si_em_copper_iron), __FUNCTION__, __LINE__);

	return;
}
#endif

static void do_get_em_data_max_demand(union get_em_data_st *p, rt_uint32_t protocal, rt_uint8_t *meteraddr)
{
	rt_uint32_t data_len = 0;
	enum frame_error_e err_em_frame;

	/* 获取电表中最大需量的参数, by 645 */
	rt_memset(&p->si_em_max_demand, 0, sizeof(p->si_em_max_demand));

	/* 正向有功总最大需量及发生时间, 返回8bytes数据 */
	err_em_frame = get_maxneed_data_from_ammeter(meteraddr, CMD_MAXNEED_TOTAL_POSITIVE_ACTIVE, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
		p->si_em_max_demand.act_max_demand_total = INVLIDE_DATAL;
		rt_memset(p->si_em_max_demand.act_max_demand_time_total, INVALID_DATA_GET_DATA_FAIL, sizeof(p->si_em_max_demand.act_max_demand_time_total)-3);

		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get MAX demand total fail(%d)\n", err_em_frame));
	} else {
		p->si_em_max_demand.act_max_demand_total = buf_of_recv_645_rep[2]<<16 | buf_of_recv_645_rep[1]<<8 | buf_of_recv_645_rep[0];
		p->si_em_max_demand.act_max_demand_time_total[0] = buf_of_recv_645_rep[7];
		p->si_em_max_demand.act_max_demand_time_total[1] = buf_of_recv_645_rep[6];
		p->si_em_max_demand.act_max_demand_time_total[2] = buf_of_recv_645_rep[5];
		p->si_em_max_demand.act_max_demand_time_total[3] = buf_of_recv_645_rep[4];
		p->si_em_max_demand.act_max_demand_time_total[4] = buf_of_recv_645_rep[3];
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	/* 正向有功费率 1 最大需量及发生时间 , 返回8bytes数据 */
	err_em_frame = get_maxneed_data_from_ammeter(meteraddr, CMD_MAXNEED_RATE1_POSITIVE_ACTIVE, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
		p->si_em_max_demand.act_max_demand_rate1 = INVLIDE_DATAL;
		rt_memset(p->si_em_max_demand.act_max_demand_time_rate1, INVALID_DATA_GET_DATA_FAIL, sizeof(p->si_em_max_demand.act_max_demand_time_rate1)-3);

		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em MAX demand total time fail(%d)\n", err_em_frame));
	} else {
		p->si_em_max_demand.act_max_demand_rate1 = buf_of_recv_645_rep[2]<<16 | buf_of_recv_645_rep[1]<<8 | buf_of_recv_645_rep[0];
		p->si_em_max_demand.act_max_demand_time_rate1[0] = buf_of_recv_645_rep[7];
		p->si_em_max_demand.act_max_demand_time_rate1[1] = buf_of_recv_645_rep[6];
		p->si_em_max_demand.act_max_demand_time_rate1[2] = buf_of_recv_645_rep[5];
		p->si_em_max_demand.act_max_demand_time_rate1[3] = buf_of_recv_645_rep[4];
		p->si_em_max_demand.act_max_demand_time_rate1[4] = buf_of_recv_645_rep[3];
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	/* 正向有功费率 2 最大需量及发生时间 , 返回8bytes数据 */
	err_em_frame = get_maxneed_data_from_ammeter(meteraddr, CMD_MAXNEED_RATE2_POSITIVE_ACTIVE, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
		p->si_em_max_demand.act_max_demand_rate2 = INVLIDE_DATAL;
		rt_memset(p->si_em_max_demand.act_max_demand_time_rate2, INVALID_DATA_GET_DATA_FAIL, sizeof(p->si_em_max_demand.act_max_demand_time_rate2)-3);

		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em timing freeze reverse active power fail(%d)\n", err_em_frame));
	} else {
		p->si_em_max_demand.act_max_demand_rate2 = buf_of_recv_645_rep[2]<<16 | buf_of_recv_645_rep[1]<<8 | buf_of_recv_645_rep[0];
		p->si_em_max_demand.act_max_demand_time_rate2[0] = buf_of_recv_645_rep[7];
		p->si_em_max_demand.act_max_demand_time_rate2[1] = buf_of_recv_645_rep[6];
		p->si_em_max_demand.act_max_demand_time_rate2[2] = buf_of_recv_645_rep[5];
		p->si_em_max_demand.act_max_demand_time_rate2[3] = buf_of_recv_645_rep[4];
		p->si_em_max_demand.act_max_demand_time_rate2[4] = buf_of_recv_645_rep[3];
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	/* 正向有功费率 3 最大需量及发生时间 , 返回8bytes数据 */
	err_em_frame = get_maxneed_data_from_ammeter(meteraddr, CMD_MAXNEED_RATE3_POSITIVE_ACTIVE, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
		p->si_em_max_demand.act_max_demand_rate3 = INVLIDE_DATAL;
		rt_memset(p->si_em_max_demand.act_max_demand_time_rate3, INVALID_DATA_GET_DATA_FAIL, sizeof(p->si_em_max_demand.act_max_demand_time_rate3)-3);

		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em timing freeze variable data fail(%d)\n", err_em_frame));
	} else {
		p->si_em_max_demand.act_max_demand_rate3 = buf_of_recv_645_rep[2]<<16 | buf_of_recv_645_rep[1]<<8 | buf_of_recv_645_rep[0];
		p->si_em_max_demand.act_max_demand_time_rate3[0] = buf_of_recv_645_rep[7];
		p->si_em_max_demand.act_max_demand_time_rate3[1] = buf_of_recv_645_rep[6];
		p->si_em_max_demand.act_max_demand_time_rate3[2] = buf_of_recv_645_rep[5];
		p->si_em_max_demand.act_max_demand_time_rate3[3] = buf_of_recv_645_rep[4];
		p->si_em_max_demand.act_max_demand_time_rate3[4] = buf_of_recv_645_rep[3];
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	/* 正向有功费率 4 最大需量及发生时间, 返回8bytes数据 */
	err_em_frame = get_maxneed_data_from_ammeter(meteraddr, CMD_MAXNEED_RATE4_POSITIVE_ACTIVE, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
		p->si_em_max_demand.act_max_demand_rate4 = INVLIDE_DATAL;
		rt_memset(p->si_em_max_demand.act_max_demand_time_rate4, INVALID_DATA_GET_DATA_FAIL, sizeof(p->si_em_max_demand.act_max_demand_time_rate4)-3);

		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em moment freeze max demand fail(%d)\n", err_em_frame));
	} else {
		p->si_em_max_demand.act_max_demand_rate4 = buf_of_recv_645_rep[2]<<16 | buf_of_recv_645_rep[1]<<8 | buf_of_recv_645_rep[0];
		p->si_em_max_demand.act_max_demand_time_rate4[0] = buf_of_recv_645_rep[7];
		p->si_em_max_demand.act_max_demand_time_rate4[1] = buf_of_recv_645_rep[6];
		p->si_em_max_demand.act_max_demand_time_rate4[2] = buf_of_recv_645_rep[5];
		p->si_em_max_demand.act_max_demand_time_rate4[3] = buf_of_recv_645_rep[4];
		p->si_em_max_demand.act_max_demand_time_rate4[4] = buf_of_recv_645_rep[3];
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	/* 正向无功总最大需量及发生时间, 返回8bytes数据 */
	err_em_frame = get_maxneed_data_from_ammeter(meteraddr, CMD_MAXNEED_TOTAL_REPOSITIVE_ACTIVE, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
		p->si_em_max_demand.react_max_demand_total = INVLIDE_DATAL;
		rt_memset(p->si_em_max_demand.react_max_demand_time_total, INVALID_DATA_GET_DATA_FAIL, sizeof(p->si_em_max_demand.react_max_demand_time_total)-3);

		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em moment freeze max demand fail(%d)\n", err_em_frame));
	} else {
		p->si_em_max_demand.react_max_demand_total = buf_of_recv_645_rep[2]<<16 | buf_of_recv_645_rep[1]<<8 | buf_of_recv_645_rep[0];
		p->si_em_max_demand.react_max_demand_time_total[0] = buf_of_recv_645_rep[7];
		p->si_em_max_demand.react_max_demand_time_total[1] = buf_of_recv_645_rep[6];
		p->si_em_max_demand.react_max_demand_time_total[2] = buf_of_recv_645_rep[5];
		p->si_em_max_demand.react_max_demand_time_total[3] = buf_of_recv_645_rep[4];
		p->si_em_max_demand.react_max_demand_time_total[4] = buf_of_recv_645_rep[3];
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	/* 正向无功费率 1 最大需量及发生时间 , 返回8bytes数据 */
	err_em_frame = get_maxneed_data_from_ammeter(meteraddr, CMD_MAXNEED_RATE1_REPOSITIVE_ACTIVE, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
		p->si_em_max_demand.react_max_demand_rate1 = INVLIDE_DATAL;
		rt_memset(p->si_em_max_demand.react_max_demand_time_rate1, INVALID_DATA_GET_DATA_FAIL, sizeof(p->si_em_max_demand.react_max_demand_time_rate1)-3);

		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em moment freeze max demand fail(%d)\n", err_em_frame));
	} else {
		p->si_em_max_demand.react_max_demand_rate1 = buf_of_recv_645_rep[2]<<16 | buf_of_recv_645_rep[1]<<8 | buf_of_recv_645_rep[0];
		p->si_em_max_demand.react_max_demand_time_rate1[0] = buf_of_recv_645_rep[7];
		p->si_em_max_demand.react_max_demand_time_rate1[1] = buf_of_recv_645_rep[6];
		p->si_em_max_demand.react_max_demand_time_rate1[2] = buf_of_recv_645_rep[5];
		p->si_em_max_demand.react_max_demand_time_rate1[3] = buf_of_recv_645_rep[4];
		p->si_em_max_demand.react_max_demand_time_rate1[4] = buf_of_recv_645_rep[3];
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	/* 正向无功费率 2 最大需量及发生时间 , 返回8bytes数据 */
	err_em_frame = get_maxneed_data_from_ammeter(meteraddr, CMD_MAXNEED_RATE2_REPOSITIVE_ACTIVE, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
		p->si_em_max_demand.react_max_demand_rate2 = INVLIDE_DATAL;
		rt_memset(p->si_em_max_demand.react_max_demand_time_rate2, INVALID_DATA_GET_DATA_FAIL, sizeof(p->si_em_max_demand.react_max_demand_time_rate2)-3);

		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em moment freeze max demand fail(%d)\n", err_em_frame));
	} else {
		p->si_em_max_demand.react_max_demand_rate2 = buf_of_recv_645_rep[2]<<16 | buf_of_recv_645_rep[1]<<8 | buf_of_recv_645_rep[0];
		p->si_em_max_demand.react_max_demand_time_rate2[0] = buf_of_recv_645_rep[7];
		p->si_em_max_demand.react_max_demand_time_rate2[1] = buf_of_recv_645_rep[6];
		p->si_em_max_demand.react_max_demand_time_rate2[2] = buf_of_recv_645_rep[5];
		p->si_em_max_demand.react_max_demand_time_rate2[3] = buf_of_recv_645_rep[4];
		p->si_em_max_demand.react_max_demand_time_rate2[4] = buf_of_recv_645_rep[3];
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	/* 正向无功费率 3 最大需量及发生时间 , 返回8bytes数据 */
	err_em_frame = get_maxneed_data_from_ammeter(meteraddr, CMD_MAXNEED_RATE3_REPOSITIVE_ACTIVE, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
		p->si_em_max_demand.react_max_demand_rate3 = INVLIDE_DATAL;
		rt_memset(p->si_em_max_demand.react_max_demand_time_rate3, INVALID_DATA_GET_DATA_FAIL, sizeof(p->si_em_max_demand.react_max_demand_time_rate3)-3);

		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em moment freeze max demand fail(%d)\n", err_em_frame));
	} else {
		p->si_em_max_demand.react_max_demand_rate3 = buf_of_recv_645_rep[2]<<16 | buf_of_recv_645_rep[1]<<8 | buf_of_recv_645_rep[0];
		p->si_em_max_demand.react_max_demand_time_rate3[0] = buf_of_recv_645_rep[7];
		p->si_em_max_demand.react_max_demand_time_rate3[1] = buf_of_recv_645_rep[6];
		p->si_em_max_demand.react_max_demand_time_rate3[2] = buf_of_recv_645_rep[5];
		p->si_em_max_demand.react_max_demand_time_rate3[3] = buf_of_recv_645_rep[4];
		p->si_em_max_demand.react_max_demand_time_rate3[4] = buf_of_recv_645_rep[3];
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	/* 正向无功费率 4 最大需量及发生时间, 返回8bytes数据 */
	err_em_frame = get_maxneed_data_from_ammeter(meteraddr, CMD_MAXNEED_RATE4_REPOSITIVE_ACTIVE, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
		p->si_em_max_demand.react_max_demand_rate4 = INVLIDE_DATAL;
		rt_memset(p->si_em_max_demand.react_max_demand_time_rate4, INVALID_DATA_GET_DATA_FAIL, sizeof(p->si_em_max_demand.react_max_demand_time_rate4)-3);

		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em moment freeze max demand fail(%d)\n", err_em_frame));
	} else {
		p->si_em_max_demand.react_max_demand_rate4 = buf_of_recv_645_rep[2]<<16 | buf_of_recv_645_rep[1]<<8 | buf_of_recv_645_rep[0];
		p->si_em_max_demand.react_max_demand_time_rate4[0] = buf_of_recv_645_rep[7];
		p->si_em_max_demand.react_max_demand_time_rate4[1] = buf_of_recv_645_rep[6];
		p->si_em_max_demand.react_max_demand_time_rate4[2] = buf_of_recv_645_rep[5];
		p->si_em_max_demand.react_max_demand_time_rate4[3] = buf_of_recv_645_rep[4];
		p->si_em_max_demand.react_max_demand_time_rate4[4] = buf_of_recv_645_rep[3];
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	copy_data_use_by_sinkinfo_buf(&sinkinfo_all_em[si_used_em_index.cur_em_index].si_em_max_demand,
			sizeof(sinkinfo_all_em[0].si_em_max_demand), &p->si_em_max_demand,
			sizeof(p->si_em_max_demand), __FUNCTION__, __LINE__);

}

static void do_get_em_data_timing_freeze(union get_em_data_st *p, rt_uint32_t protocal, rt_uint8_t *meteraddr)
{
	rt_uint32_t data_len = 0;
	enum frame_error_e err_em_frame;
	int i;

	/* 获取电表中定时冻结的参数, by 645 */

	/* 定时冻结时间, 返回2bytes数据 */
	if(protocal == AP_PROTOCOL_645_2007){
		for(i = 0; i<ELECTRIC_METER_TIMING_FREEZE_MAX; i++){
//			rt_memset(&p->si_em_time_freeze, 0, sizeof(p->si_em_time_freeze));
			copy_data_use_by_sinkinfo_buf(&p->si_em_time_freeze, sizeof(p->si_em_time_freeze),
					&sinkinfo_all_em[si_used_em_index.cur_em_index].si_em_time_freeze[i],
					sizeof(sinkinfo_all_em[0].si_em_time_freeze[0]),
					__FUNCTION__, __LINE__);

			err_em_frame = get_forzen_data_from_ammeter(meteraddr, CMD_FORZEN_TIMEING_TIME, i+1, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
			if (FRAME_E_OK != err_em_frame) {
				rt_memset(p->si_em_time_freeze.freeze_time, INVALID_DATA_GET_DATA_FAIL, sizeof(p->si_em_time_freeze.freeze_time)-3);

				if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
					sinkinfo_log(("get em timing freeze time fail(%d)\n", err_em_frame));
			} else {
				p->si_em_time_freeze.freeze_time[0] = buf_of_recv_645_rep[4];
				p->si_em_time_freeze.freeze_time[1] = buf_of_recv_645_rep[3];
				p->si_em_time_freeze.freeze_time[2] = buf_of_recv_645_rep[2];
				p->si_em_time_freeze.freeze_time[3] = buf_of_recv_645_rep[1];
				p->si_em_time_freeze.freeze_time[4] = buf_of_recv_645_rep[0];
			}
			rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

			/* 定时冻结正向有功总电能, 返回2bytes数据 */
			err_em_frame = get_forzen_data_from_ammeter(meteraddr, CMD_FORZEN_TIMEING_POSITIVE_ACTIVE_POWER, i+1, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
			if (FRAME_E_OK != err_em_frame) {
//				p->si_em_time_freeze.act_elec_energy = INVLIDE_DATAL ;

				if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
					sinkinfo_log(("get em timing freeze active power fail(%d)\n", err_em_frame));
			} else {
				p->si_em_time_freeze.act_elec_energy = buf_of_recv_645_rep[3]<<24 | buf_of_recv_645_rep[2]<<16 | buf_of_recv_645_rep[1]<<8 | buf_of_recv_645_rep[0];
			}
			rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

			/* 定时冻结反向有功总电能, 返回3bytes数据 */
			err_em_frame = get_forzen_data_from_ammeter(meteraddr, CMD_FORZEN_TIMEING_REPOSITIVE_ACTIVE_POWER, i+1, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
			if (FRAME_E_OK != err_em_frame) {
//				p->si_em_time_freeze.reverse_act_elec_energy = INVLIDE_DATAL;

				if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
					sinkinfo_log(("get em timing freeze reverse active power fail(%d)\n", err_em_frame));
			} else {
				p->si_em_time_freeze.reverse_act_elec_energy = buf_of_recv_645_rep[3]<<24 | buf_of_recv_645_rep[2]<<16 | buf_of_recv_645_rep[1]<<8 | buf_of_recv_645_rep[0];
			}
			rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

			/* 定时冻结总有功无功功率，ABC三相有功无功功率, 返回24bytes数据 */
			err_em_frame = get_forzen_data_from_ammeter(meteraddr, CMD_FORZEN_TIMEING_VARIABLE_DATA, i+1, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
			if (FRAME_E_OK != err_em_frame) {
				p->si_em_time_freeze.apxT = INVLIDE_DATAL;
				p->si_em_time_freeze.apxA = INVLIDE_DATAL;
				p->si_em_time_freeze.apxB = INVLIDE_DATAL;
				p->si_em_time_freeze.apxC = INVLIDE_DATAL;
				p->si_em_time_freeze.rapxT = INVLIDE_DATAL;
				p->si_em_time_freeze.rapxA = INVLIDE_DATAL;
				p->si_em_time_freeze.rapxB = INVLIDE_DATAL;
				p->si_em_time_freeze.rapxC = INVLIDE_DATAL;

				if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
					sinkinfo_log(("get em timing freeze variable data fail(%d)\n", err_em_frame));
			} else {
				p->si_em_time_freeze.apxT = buf_of_recv_645_rep[2]<<16 | buf_of_recv_645_rep[1]<<8 | buf_of_recv_645_rep[0];
				p->si_em_time_freeze.apxA= buf_of_recv_645_rep[5]<<16 | buf_of_recv_645_rep[4]<<8 | buf_of_recv_645_rep[3];
				p->si_em_time_freeze.apxB= buf_of_recv_645_rep[8]<<16 | buf_of_recv_645_rep[7]<<8 | buf_of_recv_645_rep[6];
				p->si_em_time_freeze.apxC= buf_of_recv_645_rep[11]<<16 | buf_of_recv_645_rep[10]<<8 | buf_of_recv_645_rep[9];
				p->si_em_time_freeze.rapxT = buf_of_recv_645_rep[14]<<16 | buf_of_recv_645_rep[13]<<8 | buf_of_recv_645_rep[12];
				p->si_em_time_freeze.rapxA= buf_of_recv_645_rep[17]<<16 | buf_of_recv_645_rep[16]<<8 | buf_of_recv_645_rep[15];
				p->si_em_time_freeze.rapxB= buf_of_recv_645_rep[20]<<16 | buf_of_recv_645_rep[19]<<8 | buf_of_recv_645_rep[18];
				p->si_em_time_freeze.rapxC= buf_of_recv_645_rep[23]<<16 | buf_of_recv_645_rep[22]<<8 | buf_of_recv_645_rep[21];
			}
			rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

			/* 定时冻结正向有功总最大需量及发生时间, 返回8bytes数据 */
			err_em_frame = get_forzen_data_from_ammeter(meteraddr, CMD_FORZEN_TIMEING_POSITIVE_MAXNEED_AND_TIME, i+1, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
			if (FRAME_E_OK != err_em_frame) {
				p->si_em_time_freeze.act_max_demand = INVLIDE_DATAL;
				rt_memset(p->si_em_time_freeze.act_max_demand_time, INVALID_DATA_GET_DATA_FAIL, sizeof(p->si_em_time_freeze.act_max_demand_time)-3);

				if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
					sinkinfo_log(("get em moment freeze max demand fail(%d)\n", err_em_frame));
			} else {
				p->si_em_time_freeze.act_max_demand = buf_of_recv_645_rep[2]<<16 | buf_of_recv_645_rep[1]<<8 | buf_of_recv_645_rep[0];
				p->si_em_time_freeze.act_max_demand_time[0] = buf_of_recv_645_rep[7];
				p->si_em_time_freeze.act_max_demand_time[1] = buf_of_recv_645_rep[6];
				p->si_em_time_freeze.act_max_demand_time[2] = buf_of_recv_645_rep[5];
				p->si_em_time_freeze.act_max_demand_time[3] = buf_of_recv_645_rep[4];
				p->si_em_time_freeze.act_max_demand_time[4] = buf_of_recv_645_rep[3];
			}
			rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

			/* 定时冻结反向有功总最大需量及发生时间, 返回8bytes数据 */
			err_em_frame = get_forzen_data_from_ammeter(meteraddr, CMD_FORZEN_TIMEING_REPOSITIVE_MAXNEED_AND_TIME, i+1, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
			if (FRAME_E_OK != err_em_frame) {
				p->si_em_time_freeze.reverse_act_max_demand = INVLIDE_DATAL;
				rt_memset(p->si_em_time_freeze.reverse_act_max_demand_time, INVALID_DATA_GET_DATA_FAIL, sizeof(p->si_em_time_freeze.reverse_act_max_demand_time)-3);

				if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
					sinkinfo_log(("get em moment freeze max demand fail(%d)\n", err_em_frame));
			} else {
				p->si_em_time_freeze.reverse_act_max_demand = buf_of_recv_645_rep[2]<<16 | buf_of_recv_645_rep[1]<<8 | buf_of_recv_645_rep[0];
				p->si_em_time_freeze.reverse_act_max_demand_time[0] = buf_of_recv_645_rep[7];
				p->si_em_time_freeze.reverse_act_max_demand_time[1] = buf_of_recv_645_rep[6];
				p->si_em_time_freeze.reverse_act_max_demand_time[2] = buf_of_recv_645_rep[5];
				p->si_em_time_freeze.reverse_act_max_demand_time[3] = buf_of_recv_645_rep[4];
				p->si_em_time_freeze.reverse_act_max_demand_time[4] = buf_of_recv_645_rep[3];
			}
			rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

			copy_data_use_by_sinkinfo_buf(&sinkinfo_all_em[si_used_em_index.cur_em_index].si_em_time_freeze[i],
					sizeof(sinkinfo_all_em[0].si_em_time_freeze[0]), &p->si_em_time_freeze,
					sizeof(p->si_em_time_freeze), __FUNCTION__, __LINE__);

		}
	}else if(protocal == AP_PROTOCOL_645_1997 || protocal == AP_PROTOCOL_EDMI || protocal == AP_PROTOCOL_UNKNOWN){
//		rt_memset(&p->si_em_time_freeze, 0, sizeof(p->si_em_time_freeze));

		rt_memset(p->si_em_time_freeze.freeze_time, INVALID_DATA_GET_DATA_FAIL, sizeof(p->si_em_time_freeze.freeze_time)-3);
		p->si_em_time_freeze.act_elec_energy = INVLIDE_DATAL ;
		p->si_em_time_freeze.reverse_act_elec_energy = INVLIDE_DATAL;
		p->si_em_time_freeze.apxT = INVLIDE_DATAL;
		p->si_em_time_freeze.apxA = INVLIDE_DATAL;
		p->si_em_time_freeze.apxB = INVLIDE_DATAL;
		p->si_em_time_freeze.apxC = INVLIDE_DATAL;
		p->si_em_time_freeze.rapxT = INVLIDE_DATAL;
		p->si_em_time_freeze.rapxA = INVLIDE_DATAL;
		p->si_em_time_freeze.rapxB = INVLIDE_DATAL;
		p->si_em_time_freeze.rapxC = INVLIDE_DATAL;
		p->si_em_time_freeze.act_max_demand = INVLIDE_DATAL;
		rt_memset(p->si_em_time_freeze.act_max_demand_time, INVALID_DATA_GET_DATA_FAIL, sizeof(p->si_em_time_freeze.act_max_demand_time)-3);
		p->si_em_time_freeze.reverse_act_max_demand = INVLIDE_DATAL;
		rt_memset(p->si_em_time_freeze.reverse_act_max_demand_time, INVALID_DATA_GET_DATA_FAIL, sizeof(p->si_em_time_freeze.reverse_act_max_demand_time)-3);

		for(i = 0; i<ELECTRIC_METER_TIMING_FREEZE_MAX; i++){
			copy_data_use_by_sinkinfo_buf(&sinkinfo_all_em[si_used_em_index.cur_em_index].si_em_time_freeze[i],
					sizeof(sinkinfo_all_em[0].si_em_time_freeze[0]), &p->si_em_time_freeze,
					sizeof(p->si_em_time_freeze), __FUNCTION__, __LINE__);
		}
	}

	return;
}

static void do_get_em_data_momentary_freeze(union get_em_data_st *p, rt_uint32_t protocal, rt_uint8_t *meteraddr)
{
	rt_uint32_t data_len = 0;
	enum frame_error_e err_em_frame;
	int i;

	/* 获取电表中瞬时冻结的参数, by 645 */
//	rt_memset(&p->si_em_mom_freeze, 0, sizeof(p->si_em_mom_freeze));

	/* 瞬时冻结时间 */
	if(protocal == AP_PROTOCOL_645_2007){
		for(i = 0; i<ELECTRIC_METER_MOMENT_FREEZE_MAX; i++){
			copy_data_use_by_sinkinfo_buf(&p->si_em_mom_freeze, sizeof(p->si_em_mom_freeze),
					&sinkinfo_all_em[si_used_em_index.cur_em_index].si_em_mom_freeze[i],
					sizeof(sinkinfo_all_em[0].si_em_mom_freeze[0]),
					__FUNCTION__, __LINE__);

			err_em_frame = get_forzen_data_from_ammeter(meteraddr, CMD_FORZEN_INSTANT_TIME, i+1,buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
			if (FRAME_E_OK != err_em_frame) {
				rt_memset(p->si_em_mom_freeze.freeze_time, INVALID_DATA_GET_DATA_FAIL, sizeof(p->si_em_mom_freeze.freeze_time)-3);
				if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
					sinkinfo_log(("get em moment freeze time fail(%d)\n", err_em_frame));
			} else {
				p->si_em_mom_freeze.freeze_time[0] = buf_of_recv_645_rep[4];
				p->si_em_mom_freeze.freeze_time[1] = buf_of_recv_645_rep[3];
				p->si_em_mom_freeze.freeze_time[2] = buf_of_recv_645_rep[2];
				p->si_em_mom_freeze.freeze_time[3] = buf_of_recv_645_rep[1];
				p->si_em_mom_freeze.freeze_time[4] = buf_of_recv_645_rep[0];
			}
			rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

			/* 瞬时冻结正向有功总电能, 返回2bytes数据 */
			err_em_frame = get_forzen_data_from_ammeter(meteraddr, CMD_FORZEN_INSTANT_POSITIVE_ACTIVE_POWER, i+1,buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
			if (FRAME_E_OK != err_em_frame) {
//				p->si_em_mom_freeze.act_elec_energy = INVLIDE_DATAL;
				if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
					sinkinfo_log(("get em moment freeze active power fail(%d)\n", err_em_frame));
			} else {
				p->si_em_mom_freeze.act_elec_energy = buf_of_recv_645_rep[3]<<24 | buf_of_recv_645_rep[2]<<16 | buf_of_recv_645_rep[1]<<8 | buf_of_recv_645_rep[0];
			}
			rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

			/* 瞬时冻结反向有功总电能, 返回3bytes数据 */
			err_em_frame = get_forzen_data_from_ammeter(meteraddr, CMD_FORZEN_INSTANT_REPOSITIVE_ACTIVE_POWER, i+1,buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
			if (FRAME_E_OK != err_em_frame) {
//				p->si_em_mom_freeze.reverse_act_elec_energy = INVLIDE_DATAL;
				if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
					sinkinfo_log(("get em moment freeze reverse active power fail(%d)\n", err_em_frame));
			} else {
				p->si_em_mom_freeze.reverse_act_elec_energy = buf_of_recv_645_rep[3]<<24 | buf_of_recv_645_rep[2]<<16 | buf_of_recv_645_rep[1]<<8 | buf_of_recv_645_rep[0];
			}
			rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

			/* 瞬时冻结变量参数，包括总有功无功功率，A、B、C三相有功无功功率, 返回2bytes数据 */
			err_em_frame = get_forzen_data_from_ammeter(meteraddr, CMD_FORZEN_INSTANT_VARIABLE_DATA, i+1, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
			if (FRAME_E_OK != err_em_frame) {
				p->si_em_mom_freeze.apxT = INVLIDE_DATAL;
				p->si_em_mom_freeze.apxA = INVLIDE_DATAL;
				p->si_em_mom_freeze.apxB = INVLIDE_DATAL;
				p->si_em_mom_freeze.apxC = INVLIDE_DATAL;
				p->si_em_mom_freeze.rapxT = INVLIDE_DATAL;
				p->si_em_mom_freeze.rapxA = INVLIDE_DATAL;
				p->si_em_mom_freeze.rapxB = INVLIDE_DATAL;
				p->si_em_mom_freeze.rapxC = INVLIDE_DATAL;

				if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
					sinkinfo_log(("get em moment freeze variable data fail(%d)\n", err_em_frame));
			} else {
				p->si_em_mom_freeze.apxT = buf_of_recv_645_rep[2]<<16 | buf_of_recv_645_rep[1]<<8 | buf_of_recv_645_rep[0];
				p->si_em_mom_freeze.apxA= buf_of_recv_645_rep[5]<<16 | buf_of_recv_645_rep[4]<<8 | buf_of_recv_645_rep[3];
				p->si_em_mom_freeze.apxB= buf_of_recv_645_rep[8]<<16 | buf_of_recv_645_rep[7]<<8 | buf_of_recv_645_rep[6];
				p->si_em_mom_freeze.apxC= buf_of_recv_645_rep[11]<<16 | buf_of_recv_645_rep[10]<<8 |buf_of_recv_645_rep[9];
				p->si_em_mom_freeze.rapxT = buf_of_recv_645_rep[14]<<16 | buf_of_recv_645_rep[13]<<8 | buf_of_recv_645_rep[12];
				p->si_em_mom_freeze.rapxA= buf_of_recv_645_rep[17]<<16 | buf_of_recv_645_rep[16]<<8 | buf_of_recv_645_rep[15];
				p->si_em_mom_freeze.rapxB= buf_of_recv_645_rep[20]<<16 | buf_of_recv_645_rep[19]<<8 | buf_of_recv_645_rep[18];
				p->si_em_mom_freeze.rapxC= buf_of_recv_645_rep[23]<<16 | buf_of_recv_645_rep[22]<<8 | buf_of_recv_645_rep[21];
			}
			rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

			/* 瞬时冻结正向有功总最大需量及发生时间, 返回2bytes数据 */
			err_em_frame = get_forzen_data_from_ammeter(meteraddr, CMD_FORZEN_INSTANT_POSITIVE_MAXNEED_AND_TIME, i+1, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
			if (FRAME_E_OK != err_em_frame) {
				p->si_em_mom_freeze.act_max_demand = INVLIDE_DATAL;
				rt_memset(p->si_em_mom_freeze.act_max_demand_time, INVALID_DATA_GET_DATA_FAIL, sizeof(p->si_em_mom_freeze.act_max_demand_time)-3);

				if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
					sinkinfo_log(("get em moment freeze max demand fail(%d)\n", err_em_frame));
			} else {
				p->si_em_mom_freeze.act_max_demand = buf_of_recv_645_rep[2]<<16 | buf_of_recv_645_rep[1]<<8 | buf_of_recv_645_rep[0];
				p->si_em_mom_freeze.act_max_demand_time[0] = buf_of_recv_645_rep[7];
				p->si_em_mom_freeze.act_max_demand_time[1] = buf_of_recv_645_rep[6];
				p->si_em_mom_freeze.act_max_demand_time[2] = buf_of_recv_645_rep[5];
				p->si_em_mom_freeze.act_max_demand_time[3] = buf_of_recv_645_rep[4];
				p->si_em_mom_freeze.act_max_demand_time[4] = buf_of_recv_645_rep[3];
			}
			rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

			/* 瞬时冻结反向有功总最大需量及发生时间, 返回2bytes数据 */
			err_em_frame = get_forzen_data_from_ammeter(meteraddr, CMD_FORZEN_TINSTANT_REPOSITIVE_MAXNEED_AND_TIME, i+1, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
			if (FRAME_E_OK != err_em_frame) {
				p->si_em_mom_freeze.reverse_act_max_demand = INVLIDE_DATAL;
				rt_memset(p->si_em_mom_freeze.reverse_act_max_demand_time, INVALID_DATA_GET_DATA_FAIL, sizeof(p->si_em_mom_freeze.reverse_act_max_demand_time)-3);

				if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
					sinkinfo_log(("get em moment freeze max demand fail(%d)\n", err_em_frame));
			} else {
				p->si_em_mom_freeze.reverse_act_max_demand = buf_of_recv_645_rep[2]<<16 | buf_of_recv_645_rep[1]<<8 | buf_of_recv_645_rep[0];
				p->si_em_mom_freeze.reverse_act_max_demand_time[0] = buf_of_recv_645_rep[7];
				p->si_em_mom_freeze.reverse_act_max_demand_time[1] = buf_of_recv_645_rep[6];
				p->si_em_mom_freeze.reverse_act_max_demand_time[2] = buf_of_recv_645_rep[5];
				p->si_em_mom_freeze.reverse_act_max_demand_time[3] = buf_of_recv_645_rep[4];
				p->si_em_mom_freeze.reverse_act_max_demand_time[4] = buf_of_recv_645_rep[3];
			}
			rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

			copy_data_use_by_sinkinfo_buf(&sinkinfo_all_em[si_used_em_index.cur_em_index].si_em_mom_freeze[i],
					sizeof(sinkinfo_all_em[0].si_em_mom_freeze[0]), &p->si_em_mom_freeze,
					sizeof(p->si_em_mom_freeze), __FUNCTION__, __LINE__);

		}
	}else if(protocal == AP_PROTOCOL_645_1997 || protocal == AP_PROTOCOL_EDMI || protocal == AP_PROTOCOL_UNKNOWN){
		rt_memset(p->si_em_mom_freeze.freeze_time, INVALID_DATA_GET_DATA_FAIL, sizeof(p->si_em_mom_freeze.freeze_time)-3);
		p->si_em_mom_freeze.act_elec_energy = INVLIDE_DATAL ;
		p->si_em_mom_freeze.reverse_act_elec_energy = INVLIDE_DATAL;
		p->si_em_mom_freeze.apxT = INVLIDE_DATAL;
		p->si_em_mom_freeze.apxA = INVLIDE_DATAL;
		p->si_em_mom_freeze.apxB = INVLIDE_DATAL;
		p->si_em_mom_freeze.apxC = INVLIDE_DATAL;
		p->si_em_mom_freeze.rapxT = INVLIDE_DATAL;
		p->si_em_mom_freeze.rapxA = INVLIDE_DATAL;
		p->si_em_mom_freeze.rapxB = INVLIDE_DATAL;
		p->si_em_mom_freeze.rapxC = INVLIDE_DATAL;
		p->si_em_mom_freeze.act_max_demand = INVLIDE_DATAL;
		rt_memset(p->si_em_mom_freeze.act_max_demand_time, INVALID_DATA_GET_DATA_FAIL, sizeof(p->si_em_mom_freeze.act_max_demand_time)-3);
		p->si_em_mom_freeze.reverse_act_max_demand = INVLIDE_DATAL;
		rt_memset(p->si_em_mom_freeze.reverse_act_max_demand_time, INVALID_DATA_GET_DATA_FAIL, sizeof(p->si_em_mom_freeze.reverse_act_max_demand_time)-3);

		for(i = 0; i<ELECTRIC_METER_MOMENT_FREEZE_MAX; i++){
			copy_data_use_by_sinkinfo_buf(&sinkinfo_all_em[si_used_em_index.cur_em_index].si_em_mom_freeze[i],
					sizeof(sinkinfo_all_em[0].si_em_mom_freeze[0]), &p->si_em_mom_freeze,
					sizeof(p->si_em_mom_freeze), __FUNCTION__, __LINE__);
		}
	}


}
#endif

/*
 * !!NOTE: 该函数不可重入
 * */
#if TEST_EM_PROTOC
static void do_get_em_data(union get_em_data_st *p)
{
	enum frame_error_e err_em_frame;
	struct electric_meter_reg_info_st amm_sn;
	rt_uint8_t meteraddr[16];
//	rt_uint32_t protocal;

	if (NULL == p) {
		printf_syn("func:%s(), param is NULL\n", __FUNCTION__);
		return;
	}

	rt_memset(meteraddr, 0, sizeof(meteraddr));

//	protocal = get_current_em_protocol(si_used_em_index.cur_em_index);

	if (SUCC == get_em_reg_info(&amm_sn)) {
		rt_memcpy(meteraddr, amm_sn.em_sn[si_used_em_index.cur_em_index], DEV_SN_MODE_LEN);
	} else {
		printf_syn("%s(), read meter addr data tbl fail\n", __FUNCTION__);
	}

	/* 获取不分abc三相的电表所有参数 */
//	rt_memset(&p->em_dev_info, 0, sizeof(p->em_dev_info));
	copy_data_use_by_sinkinfo_buf(&p->em_dev_info, sizeof(p->em_dev_info),
			&sinkinfo_all_em[si_used_em_index.cur_em_index].em_dev_info,
			sizeof(sinkinfo_all_em[0].em_dev_info),
			__FUNCTION__, __LINE__);

	/* 电表有功电能, 正向有功总电能, 返回4bytes数据 */
	err_em_frame = get_data_from_em_xprotocol(AC_POSITIVE_ACTIVE_POWER,
			&p->em_dev_info.em_act_total_energy, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		p->em_dev_info.em_act_total_energy = INVLIDE_DATAL;
		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em act electric energy fail(%d)\n", err_em_frame));
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));


	copy_data_use_by_sinkinfo_buf(&sinkinfo_all_em[si_used_em_index.cur_em_index].em_dev_info,
			sizeof(sinkinfo_all_em[0].em_dev_info), &p->em_dev_info,
			sizeof(p->em_dev_info), __FUNCTION__, __LINE__);
	return;
}
#else
static void do_get_em_data(union get_em_data_st *p)
{
	enum frame_error_e err_em_frame;
	struct electric_meter_reg_info_st amm_sn;
	rt_uint32_t data_len = 0;

	rt_uint8_t meteraddr[16];
	rt_uint32_t protocal;

	if (NULL == p) {
		printf_syn("func:%s(), param is NULL\n", __FUNCTION__);
		return;
	}

	rt_memset(meteraddr, 0, sizeof(meteraddr));

	protocal = get_current_em_protocol(si_used_em_index.cur_em_index);

	if (SUCC == get_em_reg_info(&amm_sn)) {
		rt_memcpy(meteraddr, amm_sn.em_sn[si_used_em_index.cur_em_index], DEV_SN_MODE_LEN);
	} else {
		printf_syn("%s(), read meter addr data tbl fail\n", __FUNCTION__);
	}

	/* 获取不分abc三相的电表所有参数 */
//	rt_memset(&p->em_dev_info, 0, sizeof(p->em_dev_info));
	copy_data_use_by_sinkinfo_buf(&p->em_dev_info, sizeof(p->em_dev_info),
			&sinkinfo_all_em[si_used_em_index.cur_em_index].em_dev_info,
			sizeof(sinkinfo_all_em[0].em_dev_info),
			__FUNCTION__, __LINE__);

	/* 电表有功电能, 正向有功总电能, 返回4bytes数据 */
	err_em_frame = get_data_from_em_xprotocol(AC_POSITIVE_ACTIVE_POWER,
			&p->em_dev_info.em_act_total_energy, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		p->em_dev_info.em_act_total_energy = INVLIDE_DATAL;
		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em act electric energy fail(%d)\n", err_em_frame));
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	/* 电表有功电能, 正向有功费率1电能, 返回4bytes数据 */
	err_em_frame = get_data_from_em_xprotocol(AC_POSITIVE_ACTIVE_RATE1_POWER,
			&p->em_dev_info.em_act_rate1_energy, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		p->em_dev_info.em_act_rate1_energy = INVLIDE_DATAL;
		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em act rate1 electric energy fail(%d)\n", err_em_frame));
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	/* 电表有功电能, 正向有功费率2电能, 返回4bytes数据 */
	err_em_frame = get_data_from_em_xprotocol(AC_POSITIVE_ACTIVE_RATE2_POWER,
			&p->em_dev_info.em_act_rate2_energy, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		p->em_dev_info.em_act_rate2_energy = INVLIDE_DATAL;
		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em act rate2 electric energy fail(%d)\n", err_em_frame));
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));
	
	/* 电表有功电能, 正向有功费率3电能, 返回4bytes数据 */
	err_em_frame = get_data_from_em_xprotocol(AC_POSITIVE_ACTIVE_RATE3_POWER,
			&p->em_dev_info.em_act_rate3_energy, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		p->em_dev_info.em_act_rate3_energy = INVLIDE_DATAL;
		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em act rate3 electric energy fail(%d)\n", err_em_frame));
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	/* 电表有功电能, 正向有功费率4电能, 返回4bytes数据 */
	err_em_frame = get_data_from_em_xprotocol(AC_POSITIVE_ACTIVE_RATE4_POWER,
			&p->em_dev_info.em_act_rate4_energy, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		p->em_dev_info.em_act_rate4_energy = INVLIDE_DATAL;
		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em act rate4 electric energy fail(%d)\n", err_em_frame));
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));	
	
	/* 电表有功电能, 反向有功总电能, 返回4bytes数据 */
	err_em_frame = get_data_from_em_xprotocol(AC_REPOSITIVE_ACTIVE_POWER,
			&p->em_dev_info.em_reverse_act_total_energy, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		p->em_dev_info.em_reverse_act_total_energy = INVLIDE_DATAL;
		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em reverse act electric energy fail(%d)\n", err_em_frame));
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	/* 电表有功电能, 反向有功费率1电能, 返回4bytes数据 */
	err_em_frame = get_data_from_em_xprotocol(AC_REPOSITIVE_ACTIVE_RATE1_POWER,
			&p->em_dev_info.em_reverse_act_rate1_energy, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		p->em_dev_info.em_reverse_act_rate1_energy = INVLIDE_DATAL;
		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em reverse act rate1 electric energy fail(%d)\n", err_em_frame));
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	/* 电表有功电能, 反向有功费率2电能, 返回4bytes数据 */
	err_em_frame = get_data_from_em_xprotocol(AC_REPOSITIVE_ACTIVE_RATE2_POWER,
			&p->em_dev_info.em_reverse_act_rate2_energy, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		p->em_dev_info.em_reverse_act_rate2_energy = INVLIDE_DATAL;
		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em reverse act rate2 electric energy fail(%d)\n", err_em_frame));
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));
	
	/* 电表有功电能, 反向有功费率3电能, 返回4bytes数据 */
	err_em_frame = get_data_from_em_xprotocol(AC_REPOSITIVE_ACTIVE_RATE3_POWER,
			&p->em_dev_info.em_reverse_act_rate3_energy, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		p->em_dev_info.em_reverse_act_rate3_energy = INVLIDE_DATAL;
		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em reverse act rate3 electric energy fail(%d)\n", err_em_frame));
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	/* 电表有功电能, 反向有功费率4电能, 返回4bytes数据 */
	err_em_frame = get_data_from_em_xprotocol(AC_REPOSITIVE_ACTIVE_RATE4_POWER,
			&p->em_dev_info.em_reverse_act_rate4_energy, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		p->em_dev_info.em_reverse_act_rate4_energy = INVLIDE_DATAL;
		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em reverse act rate4 electric energy fail(%d)\n", err_em_frame));
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));
	/* 电表无功电能, 正向无功总电能, 返回4bytes数据 */
	/* 是不是97/07都应该获取该数据???? mark by David */
	err_em_frame = get_data_from_em_xprotocol(AC_POSITIVE_WATTLESS_POWER,
			&p->em_dev_info.em_react_total_energy, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		p->em_dev_info.em_react_total_energy = INVLIDE_DATAL;
		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em react electric energy fail(%d)\n", err_em_frame));
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));
	
	/* 电表无功电能, 正向无功费率1电能, 返回4bytes数据 */
	err_em_frame = get_data_from_em_xprotocol(AC_POSITIVE_WATTLESS_RATE1_POWER,
			&p->em_dev_info.em_react_rate1_energy, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		p->em_dev_info.em_react_rate1_energy = INVLIDE_DATAL;
		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em react rate1 electric energy fail(%d)\n", err_em_frame));
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	/* 电表无功电能, 正向无功费率2电能, 返回4bytes数据 */
	err_em_frame = get_data_from_em_xprotocol(AC_POSITIVE_WATTLESS_RATE2_POWER,
			&p->em_dev_info.em_react_rate2_energy, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		p->em_dev_info.em_react_rate2_energy = INVLIDE_DATAL;
		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em react rate2 electric energy fail(%d)\n", err_em_frame));
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	/* 电表无功电能, 正向无功费率3电能, 返回4bytes数据 */
	err_em_frame = get_data_from_em_xprotocol(AC_POSITIVE_WATTLESS_RATE3_POWER,
			&p->em_dev_info.em_react_rate3_energy, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		p->em_dev_info.em_react_rate3_energy = INVLIDE_DATAL;
		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em react rate3 electric energy fail(%d)\n", err_em_frame));
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	/* 电表无功电能, 正向无功费率4电能, 返回4bytes数据 */
	err_em_frame = get_data_from_em_xprotocol(AC_POSITIVE_WATTLESS_RATE4_POWER,
			&p->em_dev_info.em_react_rate4_energy, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		p->em_dev_info.em_react_rate4_energy = INVLIDE_DATAL;
		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em react rate4 electric energy fail(%d)\n", err_em_frame));
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	/* 电表无功电能, 反向无功总电能, 返回4bytes数据 */
	err_em_frame = get_data_from_em_xprotocol(AC_REPOSITIVE_WATTLESS_POWER,
			&p->em_dev_info.em_reverse_react_total_energy, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		p->em_dev_info.em_reverse_react_total_energy = INVLIDE_DATAL;
		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em reverse react electric energy fail(%d)\n", err_em_frame));
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));
	
	/* 电表无功电能, 反向无功费率1电能, 返回4bytes数据 */
	err_em_frame = get_data_from_em_xprotocol(AC_REPOSITIVE_WATTLESS_RATE1_POWER,
			&p->em_dev_info.em_reverse_react_rate1_energy, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		p->em_dev_info.em_reverse_react_rate1_energy = INVLIDE_DATAL;
		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em reverse react rate1 electric energy fail(%d)\n", err_em_frame));
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	/* 电表无功电能, 反向无功费率2电能, 返回4bytes数据 */
	err_em_frame = get_data_from_em_xprotocol(AC_REPOSITIVE_WATTLESS_RATE2_POWER,
			&p->em_dev_info.em_reverse_react_rate2_energy, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		p->em_dev_info.em_reverse_react_rate2_energy = INVLIDE_DATAL;
		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em reverse react rate2 electric energy fail(%d)\n", err_em_frame));
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	/* 电表无功电能, 反向无功费率3电能, 返回4bytes数据 */
	err_em_frame = get_data_from_em_xprotocol(AC_REPOSITIVE_WATTLESS_RATE3_POWER,
			&p->em_dev_info.em_reverse_react_rate3_energy, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		p->em_dev_info.em_reverse_react_rate3_energy = INVLIDE_DATAL;
		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em reverse react rate3 electric energy fail(%d)\n", err_em_frame));
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	/* 电表无功电能, 反向无功费率4电能, 返回4bytes数据 */
	err_em_frame = get_data_from_em_xprotocol(AC_REPOSITIVE_WATTLESS_RATE4_POWER,
			&p->em_dev_info.em_reverse_react_rate4_energy, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		p->em_dev_info.em_reverse_react_rate4_energy = INVLIDE_DATAL;
		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em reverse react rate4 electric energy fail(%d)\n", err_em_frame));
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	/* 电表内日期, 返回4bytes数据 */
	err_em_frame = get_power_data_from_ammeter(meteraddr, AC_DATE_AND_WEEK,
			buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK == err_em_frame) {
		p->em_dev_info.em_date_time[0] = buf_of_recv_645_rep[3];
		p->em_dev_info.em_date_time[1] = buf_of_recv_645_rep[2];
		p->em_dev_info.em_date_time[2] = buf_of_recv_645_rep[1];
		p->em_dev_info.em_date_time[3] = buf_of_recv_645_rep[0];
	} else {
		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em data and week fail(%d)\n", err_em_frame));
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	/* 电表内的时间, 返回3bytes */
	err_em_frame = get_power_data_from_ammeter(meteraddr, AC_AMMETER_TIME,
			&buf_of_recv_645_rep[4], &data_len, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK == err_em_frame) {
		p->em_dev_info.em_date_time[4] = buf_of_recv_645_rep[6];
		p->em_dev_info.em_date_time[5] = buf_of_recv_645_rep[5];
		p->em_dev_info.em_date_time[6] = buf_of_recv_645_rep[4];
	} else {
		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em time fail(%d)\n", err_em_frame));
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	if(protocal == AP_PROTOCOL_645_2007) {
		/* 电表组合有功电能, 组合有功总电能, 返回4bytes数据 */
		err_em_frame = get_data_from_em_xprotocol(AC_COMBINATION_ACTIVE_TOTAL_POWER,
				&p->em_dev_info.em_combin_act_total_energy, RS485_PORT_USED_BY_645);
		if (FRAME_E_OK != err_em_frame) {
//			p->em_dev_info.em_combin_act_total_energy = INVLIDE_DATAL;
			if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
				sinkinfo_log(("get em combin act electric energy fail(%d)\n", err_em_frame));
		}
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));
	
		/* 电表组合有功电能, 组合有功费率1电能, 返回4bytes数据 */
		err_em_frame = get_data_from_em_xprotocol(AC_COMBINATION_ACTIVE_RATE1_POWER,
				&p->em_dev_info.em_combin_act_rate1_energy, RS485_PORT_USED_BY_645);
		if (FRAME_E_OK != err_em_frame) {
//			p->em_dev_info.em_combin_act_rate1_energy = INVLIDE_DATAL;
			if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
				sinkinfo_log(("get em combin act rate1 electric energy fail(%d)\n", err_em_frame));
		}
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* 电表组合有功电能, 组合有功费率2电能, 返回4bytes数据 */
		err_em_frame = get_data_from_em_xprotocol(AC_COMBINATION_ACTIVE_RATE2_POWER,
				&p->em_dev_info.em_combin_act_rate2_energy, RS485_PORT_USED_BY_645);
		if (FRAME_E_OK != err_em_frame) {
//			p->em_dev_info.em_combin_act_rate2_energy = INVLIDE_DATAL;
			if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
				sinkinfo_log(("get em combin act rate2 electric energy fail(%d)\n", err_em_frame));
		}
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* 电表组合有功电能, 组合有功费率3电能, 返回4bytes数据 */
		err_em_frame = get_data_from_em_xprotocol(AC_COMBINATION_ACTIVE_RATE3_POWER,
				&p->em_dev_info.em_combin_act_rate3_energy, RS485_PORT_USED_BY_645);
		if (FRAME_E_OK != err_em_frame) {
//			p->em_dev_info.em_combin_act_rate3_energy = INVLIDE_DATAL;
			if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
				sinkinfo_log(("get em combin act rate3 electric energy fail(%d)\n", err_em_frame));
		}
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* 电表组合有功电能, 组合有功费率4电能, 返回4bytes数据 */
		err_em_frame = get_data_from_em_xprotocol(AC_COMBINATION_ACTIVE_RATE4_POWER,
				&p->em_dev_info.em_combin_act_rate4_energy, RS485_PORT_USED_BY_645);
		if (FRAME_E_OK != err_em_frame) {
//			p->em_dev_info.em_combin_act_rate4_energy = INVLIDE_DATAL;
			if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
				sinkinfo_log(("get em combin act rate4 electric energy fail(%d)\n", err_em_frame));
		}
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));
		/* 电表内温度, 返回4bytes数据 */
		err_em_frame = get_power_data_from_ammeter(meteraddr, AC_AMMETER_TEMPERATURE, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
		if (FRAME_E_OK != err_em_frame) {
			p->em_dev_info.em_temper = INVLIDE_DATAS;
			if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
				sinkinfo_log(("get em temperature fail(%d)\n", err_em_frame));
		} else {
			p->em_dev_info.em_temper = buf_of_recv_645_rep[1]<<8 | buf_of_recv_645_rep[0];
		}
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* 电表时钟电池电压, 返回4bytes数据 */
		err_em_frame = get_power_data_from_ammeter(meteraddr, AC_CLOCK_BATTERY_VOLTAGE, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
		if (FRAME_E_OK != err_em_frame) {
//			p->em_dev_info.em_v_clock = INVLIDE_DATAL;
			if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
				sinkinfo_log(("get em clock battery vol fail(%d)\n", err_em_frame));
		} else {
			p->em_dev_info.em_v_clock = buf_of_recv_645_rep[1]<<8 | buf_of_recv_645_rep[0];
		}
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* 电表抄表电池电压, 返回4bytes数据 */
		err_em_frame = get_power_data_from_ammeter(meteraddr, AC_METER_READ_BATTERY_VOLTAGE, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
		if (FRAME_E_OK != err_em_frame) {
//			p->em_dev_info.em_v_read_em = INVLIDE_DATAL;
			if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
				sinkinfo_log(("get em collect battery vol fail(%d)\n", err_em_frame));
		} else {
			p->em_dev_info.em_v_read_em = buf_of_recv_645_rep[1]<<8 | buf_of_recv_645_rep[0];
		}
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

//		p->em_dev_info.em_react_total_energy = INVLIDE_DATAL;

	}else if(protocal == AP_PROTOCOL_645_1997 || protocal == AP_PROTOCOL_EDMI || protocal == AP_PROTOCOL_UNKNOWN){
//		/* 电表无功电能, 正向无功电能, 返回4bytes数据 */
//		/* 是不是97/07都应该获取该数据???? mark by David */
//		err_em_frame = get_data_from_em_xprotocol(AC_POSITIVE_WATTLESS_POWER,
//				&p->em_dev_info.em_react_total_energy, RS485_PORT_USED_BY_645);
//		if (FRAME_E_OK != err_em_frame) {
//			p->em_dev_info.em_react_total_energy = INVLIDE_DATAL;
//			if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
//				sinkinfo_log(("get em react electric energy fail(%d)\n", err_em_frame));
//		}
//		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		p->em_dev_info.em_temper = INVLIDE_DATAS;
		p->em_dev_info.em_v_clock = INVLIDE_DATAL;
		p->em_dev_info.em_v_read_em = INVLIDE_DATAL;
		p->em_dev_info.em_combin_act_total_energy = INVLIDE_DATAL;
		p->em_dev_info.em_combin_act_rate1_energy = INVLIDE_DATAL;
		p->em_dev_info.em_combin_act_rate2_energy = INVLIDE_DATAL;
		p->em_dev_info.em_combin_act_rate3_energy = INVLIDE_DATAL;
		p->em_dev_info.em_combin_act_rate4_energy = INVLIDE_DATAL;
	}

	/* 电能误差可以不是每次都测试, 如果不测试就保留上次的值 */
//	p->em_dev_info.em_act_ee_inaccuracy = sinkinfo_all_em[si_used_em_index.cur_em_index].em_dev_info.em_act_ee_inaccuracy;
//	p->em_dev_info.em_react_ee_inaccuracy = sinkinfo_all_em[si_used_em_index.cur_em_index].em_dev_info.em_react_ee_inaccuracy;

	copy_data_use_by_sinkinfo_buf(&sinkinfo_all_em[si_used_em_index.cur_em_index].em_dev_info,
			sizeof(sinkinfo_all_em[0].em_dev_info), &p->em_dev_info,
			sizeof(p->em_dev_info), __FUNCTION__, __LINE__);

#if 0 //EM_EVENT_SUPPERT /* 对电表事件的代码没有进行修正, mark by David */
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* A相功率因数, 返回2bytes数据 */
		for(i = 0; i<ELECTRIC_METER_EVENT_TIMES_MAX; i++){
		err_em_frame = get_event_data_from_ammeter(meteraddr, AMM_EVENT_AMMETER_RESET, i+1, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);

		if (FRAME_E_OK != err_em_frame) {
			if (is_bit_set(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT)) {
				err_code = SIE_SECOND_GET_DATA_FAIL;
			} else {
				err_code = SIE_FAIL;
				set_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
			}

			sinkinfo_log(("get em pa power factor fail(%d)\n", err_em_frame));
			break;
		} else {
			rt_memcpy(p->si_em_meterclear_event[i].start_time, (rt_uint32_t *)buf_of_recv_645_rep, sizeof(p->si_em_meterclear_event[i].start_time));
			rt_memcpy(p->si_em_meterclear_event[i].operator_code, &buf_of_recv_645_rep[6], sizeof(p->si_em_meterclear_event[i].operator_code));
			p->si_em_meterclear_event[i].em_act_elec_energy = buf_of_recv_645_rep[13]<<24 | buf_of_recv_645_rep[12]<<16 | buf_of_recv_645_rep[11]<<8 | buf_of_recv_645_rep[10];
			p->si_em_meterclear_event[i].em_reverse_act_elec_energy = buf_of_recv_645_rep[17]<<24 | buf_of_recv_645_rep[16]<<16 | buf_of_recv_645_rep[15]<<8 | buf_of_recv_645_rep[14];
			p->si_em_meterclear_event[i].em_react_elec_energy_quadrant1 = buf_of_recv_645_rep[21]<<24 | buf_of_recv_645_rep[20]<<16 | buf_of_recv_645_rep[19]<<8 | buf_of_recv_645_rep[18];
			p->si_em_meterclear_event[i].em_react_elec_energy_quadrant2 = buf_of_recv_645_rep[25]<<24 | buf_of_recv_645_rep[24]<<16 | buf_of_recv_645_rep[23]<<8 | buf_of_recv_645_rep[22];
			p->si_em_meterclear_event[i].em_react_elec_energy_quadrant3 = buf_of_recv_645_rep[29]<<24 | buf_of_recv_645_rep[28]<<16 | buf_of_recv_645_rep[27]<<8 | buf_of_recv_645_rep[26];
			p->si_em_meterclear_event[i].em_react_elec_energy_quadrant4 = buf_of_recv_645_rep[33]<<24 | buf_of_recv_645_rep[32]<<16 | buf_of_recv_645_rep[31]<<8 | buf_of_recv_645_rep[30];
			p->si_em_meterclear_event[i].pA_act_elec_energy = buf_of_recv_645_rep[37]<<24 | buf_of_recv_645_rep[36]<<16 | buf_of_recv_645_rep[35]<<8 | buf_of_recv_645_rep[34];
			p->si_em_meterclear_event[i].pA_reverse_act_elec_energy = buf_of_recv_645_rep[41]<<24 | buf_of_recv_645_rep[40]<<16 | buf_of_recv_645_rep[39]<<8 | buf_of_recv_645_rep[38];
			p->si_em_meterclear_event[i].pA_react_elec_energy_quadrant1 = buf_of_recv_645_rep[45]<<24 | buf_of_recv_645_rep[44]<<16 | buf_of_recv_645_rep[43]<<8 | buf_of_recv_645_rep[42];
			p->si_em_meterclear_event[i].pA_react_elec_energy_quadrant2 = buf_of_recv_645_rep[49]<<24 | buf_of_recv_645_rep[48]<<16 | buf_of_recv_645_rep[47]<<8 | buf_of_recv_645_rep[46];
			p->si_em_meterclear_event[i].pA_react_elec_energy_quadrant3 = buf_of_recv_645_rep[53]<<24 | buf_of_recv_645_rep[52]<<16 | buf_of_recv_645_rep[51]<<8 | buf_of_recv_645_rep[50];
			p->si_em_meterclear_event[i].pA_react_elec_energy_quadrant4 = buf_of_recv_645_rep[57]<<24 | buf_of_recv_645_rep[56]<<16 | buf_of_recv_645_rep[55]<<8 | buf_of_recv_645_rep[54];
			p->si_em_meterclear_event[i].pB_act_elec_energy = buf_of_recv_645_rep[61]<<24 | buf_of_recv_645_rep[60]<<16 | buf_of_recv_645_rep[59]<<8 | buf_of_recv_645_rep[58];
			p->si_em_meterclear_event[i].pB_reverse_act_elec_energy = buf_of_recv_645_rep[65]<<24 | buf_of_recv_645_rep[64]<<16 | buf_of_recv_645_rep[63]<<8 | buf_of_recv_645_rep[62];
			p->si_em_meterclear_event[i].pB_react_elec_energy_quadrant1 = buf_of_recv_645_rep[69]<<24 | buf_of_recv_645_rep[68]<<16 | buf_of_recv_645_rep[67]<<8 | buf_of_recv_645_rep[66];
			p->si_em_meterclear_event[i].pB_react_elec_energy_quadrant2 = buf_of_recv_645_rep[73]<<24 | buf_of_recv_645_rep[72]<<16 | buf_of_recv_645_rep[71]<<8 | buf_of_recv_645_rep[70];
			p->si_em_meterclear_event[i].pB_react_elec_energy_quadrant3 = buf_of_recv_645_rep[77]<<24 | buf_of_recv_645_rep[76]<<16 | buf_of_recv_645_rep[75]<<8 | buf_of_recv_645_rep[74];
			p->si_em_meterclear_event[i].pB_react_elec_energy_quadrant4 = buf_of_recv_645_rep[81]<<24 | buf_of_recv_645_rep[80]<<16 | buf_of_recv_645_rep[79]<<8 | buf_of_recv_645_rep[78];
			p->si_em_meterclear_event[i].pC_act_elec_energy = buf_of_recv_645_rep[85]<<24 | buf_of_recv_645_rep[84]<<16 | buf_of_recv_645_rep[83]<<8 | buf_of_recv_645_rep[82];
			p->si_em_meterclear_event[i].pC_reverse_act_elec_energy = buf_of_recv_645_rep[89]<<24 | buf_of_recv_645_rep[88]<<16 | buf_of_recv_645_rep[87]<<8 | buf_of_recv_645_rep[86];
			p->si_em_meterclear_event[i].pC_react_elec_energy_quadrant1 = buf_of_recv_645_rep[93]<<24 | buf_of_recv_645_rep[92]<<16 | buf_of_recv_645_rep[91]<<8 | buf_of_recv_645_rep[90];
			p->si_em_meterclear_event[i].pC_react_elec_energy_quadrant2 = buf_of_recv_645_rep[97]<<24 | buf_of_recv_645_rep[96]<<16 | buf_of_recv_645_rep[95]<<8 | buf_of_recv_645_rep[94];
			p->si_em_meterclear_event[i].pC_react_elec_energy_quadrant3 = buf_of_recv_645_rep[101]<<24 | buf_of_recv_645_rep[100]<<16 | buf_of_recv_645_rep[99]<<8 | buf_of_recv_645_rep[98];
			p->si_em_meterclear_event[i].pC_react_elec_energy_quadrant4 = buf_of_recv_645_rep[105]<<24 | buf_of_recv_645_rep[104]<<16 | buf_of_recv_645_rep[103]<<8 | buf_of_recv_645_rep[102];
			clr_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
		}

		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* A相功率因数, 返回2bytes数据 */

		err_em_frame = get_event_data_from_ammeter(meteraddr, AMM_EVENT_NEED_RESET, i+1, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);

		if (FRAME_E_OK != err_em_frame) {
			if (is_bit_set(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT)) {
				err_code = SIE_SECOND_GET_DATA_FAIL;
			} else {
				err_code = SIE_FAIL;
				set_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
			}

			sinkinfo_log(("get em pa power factor fail(%d)\n", err_em_frame));
			break;
		} else {
			rt_memcpy(p->si_em_demandclear_event[i].start_time, (rt_uint32_t *)buf_of_recv_645_rep, sizeof(p->si_em_demandclear_event[i].start_time));
			rt_memcpy(p->si_em_demandclear_event[i].operator_code, &buf_of_recv_645_rep[6], sizeof(p->si_em_demandclear_event[i].operator_code));

			clr_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
		}

		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* A相功率因数, 返回2bytes数据 */

		err_em_frame = get_event_data_from_ammeter(meteraddr, AMM_EVENT_TIMING_RECORD, i+1, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
		
		if (FRAME_E_OK != err_em_frame) {
			if (is_bit_set(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT)) {
				err_code = SIE_SECOND_GET_DATA_FAIL;
			} else {
				err_code = SIE_FAIL;
				set_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
			}

			sinkinfo_log(("get em pa power factor fail(%d)\n", err_em_frame));
			break;
		} else {
			rt_memcpy(p->si_em_calibratetime_event[i].operator_code, buf_of_recv_645_rep, sizeof(p->si_em_calibratetime_event[i].operator_code));
			rt_memcpy(p->si_em_calibratetime_event[i].before_time, (rt_uint32_t *)&buf_of_recv_645_rep[4], sizeof(p->si_em_calibratetime_event[i].before_time));
			rt_memcpy(p->si_em_calibratetime_event[i].after_time, (rt_uint32_t *)&buf_of_recv_645_rep[10], sizeof(p->si_em_calibratetime_event[i].after_time));

			clr_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
		}

		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* A相功率因数, 返回2bytes数据 */

		err_em_frame = get_event_data_from_ammeter(meteraddr, AMM_EVENT_PROGRAM_RECORD, i+1, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);

		if (FRAME_E_OK != err_em_frame) {
			if (is_bit_set(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT)) {
				err_code = SIE_SECOND_GET_DATA_FAIL;
			} else {
				err_code = SIE_FAIL;
				set_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
			}

			sinkinfo_log(("get em pa power factor fail(%d)\n", err_em_frame));
			break;
		} else {
			rt_memcpy(p->si_em_program_event[i].start_time, (rt_uint32_t *)buf_of_recv_645_rep, sizeof(p->si_em_program_event[i].start_time));
			rt_memcpy(p->si_em_program_event[i].operator_code, &buf_of_recv_645_rep[6], sizeof(p->si_em_program_event[i].operator_code));
			rt_memcpy(p->si_em_program_event[i].data_flag, &buf_of_recv_645_rep[10], sizeof(p->si_em_program_event[i].data_flag));

			clr_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
		}
		
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* A相功率因数, 返回2bytes数据 */
		
		err_em_frame = get_event_data_from_ammeter(meteraddr, AMM_EVENT_VOLTAGE_ANTI_PHASE_RECORD, i+1, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
		
		if (FRAME_E_OK != err_em_frame) {
			if (is_bit_set(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT)) {
				err_code = SIE_SECOND_GET_DATA_FAIL;
			} else {
				err_code = SIE_FAIL;
				set_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
			}

			sinkinfo_log(("get em pa power factor fail(%d)\n", err_em_frame));
			break;
		} else {
			rt_memcpy(p->si_em_sreqvol_event[i].start_time, (rt_uint32_t *)buf_of_recv_645_rep, sizeof(p->si_em_sreqvol_event[i].start_time));
			rt_memcpy(p->si_em_sreqvol_event[i].end_time, (rt_uint32_t *)&buf_of_recv_645_rep[6], sizeof(p->si_em_sreqvol_event[i].end_time));

			clr_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
		}
		
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* A相功率因数, 返回2bytes数据 */
		err_em_frame = get_event_data_from_ammeter(meteraddr, AMM_EVENT_CURRENT_ANTI_PHASE_RECORD, i+1, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);

		if (FRAME_E_OK != err_em_frame) {
			if (is_bit_set(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT)) {
				err_code = SIE_SECOND_GET_DATA_FAIL;
			} else {
				err_code = SIE_FAIL;
				set_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
			}

			sinkinfo_log(("get em current reverse current sequence fail (%d)\n", err_em_frame));
			break;
		} else {
			rt_memcpy(p->si_em_sreqcur_event[i].start_time, (rt_uint32_t *)buf_of_recv_645_rep, sizeof(p->si_em_sreqcur_event[i].start_time));
			rt_memcpy(p->si_em_sreqcur_event[i].end_time, (rt_uint32_t *)&buf_of_recv_645_rep[6], sizeof(p->si_em_sreqcur_event[i].end_time));
			clr_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
		}

		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD)); //hongbin E
		}
#endif

	/* 获取电表中a相所有abc三相独立的参数, by 645 */
//	rt_memset(&p->si_em_ind_px, 0, sizeof(p->si_em_ind_px));
	copy_data_use_by_sinkinfo_buf(&p->si_em_ind_px, sizeof(p->si_em_ind_px),
			&sinkinfo_all_em[si_used_em_index.cur_em_index].si_em_ind_pa,
			sizeof(sinkinfo_all_em[0].si_em_ind_pa),
			__FUNCTION__, __LINE__);

	/* A相电压, 返回2bytes数据 */
	err_em_frame = get_data_from_em_xprotocol(AC_A_VOLTAGE,
			&p->si_em_ind_px.vx, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		p->si_em_ind_px.vx = INVLIDE_DATAL;
		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em pa voltage fail(%d)\n", err_em_frame));
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	/* A相电流, 返回2bytes数据 */
	err_em_frame = get_data_from_em_xprotocol(AC_A_CURRENT,
			&p->si_em_ind_px.ix, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		p->si_em_ind_px.ix = INVLIDE_DATAL;
		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em pa current fail(%d)\n", err_em_frame));
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	/* A相有功功率, 返回3bytes数据 */
	err_em_frame = get_data_from_em_xprotocol(AC_A_ACTIVE_POWER,
			&p->si_em_ind_px.apx, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		p->si_em_ind_px.apx = INVLIDE_DATAL;
		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em pa act power fail(%d)\n", err_em_frame));
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	/* A相无功功率, 返回2bytes数据 */
	err_em_frame = get_data_from_em_xprotocol(AC_A_REACTIVE_POWER,
			&p->si_em_ind_px.rapx, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		p->si_em_ind_px.rapx = INVLIDE_DATAL;
		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em pa react power fail(%d)\n", err_em_frame));
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	/* A相功率因数, 返回2bytes数据 */
	err_em_frame = get_data_from_em_xprotocol(AC_A_POWER_FACTOR,
			&p->si_em_ind_px.pfx, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		p->si_em_ind_px.pfx = INVLIDE_DATAL;
		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em pa power factor fail(%d)\n", err_em_frame));
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	if(protocal == AP_PROTOCOL_645_2007){
		/* A相无功功率, 返回2bytes数据 */
		err_em_frame = get_data_from_em_xprotocol(AC_A_APPARENT_POWER,
				(u32_t *)&p->si_em_ind_px.appx, RS485_PORT_USED_BY_645);
		if (FRAME_E_OK != err_em_frame) {
//			p->si_em_ind_px.appx = INVLIDE_DATAL;
			if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
				sinkinfo_log(("get em pa react power fail(%d)\n", err_em_frame));
		}
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* A相功率因数, 返回2bytes数据 */
		err_em_frame = get_data_from_em_xprotocol(AC_A_VOLTAGE_DISTORTION,
				(u32_t *)&p->si_em_ind_px.vdx, RS485_PORT_USED_BY_645);
		if (FRAME_E_OK != err_em_frame) {
//			p->si_em_ind_px.vdx = INVLIDE_DATAL;
			if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
				sinkinfo_log(("get em pa power factor fail(%d)\n", err_em_frame));
		}
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* A相功率因数, 返回2bytes数据 */
		err_em_frame = get_data_from_em_xprotocol(AC_A_CURRENT_DISTORTION,
				(u32_t *)&p->si_em_ind_px.cdx, RS485_PORT_USED_BY_645);
		if (FRAME_E_OK != err_em_frame) {
//			p->si_em_ind_px.cdx = INVLIDE_DATAL;
			if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
				sinkinfo_log(("get em pa power factor fail(%d)\n", err_em_frame));
		}
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));


		/* 电网频率, 07规约中没有区分abc三相的频率, 统一使用电网频率, 返回2bytes数据 */
		err_em_frame = get_data_from_em_xprotocol(AC_POWER_FREQUENCY,
				(u32_t *)&p->si_em_ind_px.hzx, RS485_PORT_USED_BY_645);
		if (FRAME_E_OK != err_em_frame) {
//			p->si_em_ind_px.hzx = INVLIDE_DATAL;
			if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
				sinkinfo_log(("get em Grid frequency fail(%d)\n", err_em_frame));
		}
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));
		
		/* A相正向有功总电能 */
		err_em_frame = get_data_from_em_xprotocol(AC_A_POSITIVE_ACTIVE_POWER,
				(u32_t *)&p->si_em_ind_px.taex, RS485_PORT_USED_BY_645);
		if (FRAME_E_OK != err_em_frame) {
//			p->si_em_ind_px.taex = INVLIDE_DATAL;
			if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
				sinkinfo_log(("get em phase A total active energy (%d)\n", err_em_frame));
		}
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));
		
		/* A相反向有功总电能 */
		err_em_frame = get_data_from_em_xprotocol(AC_A_REPOSITIVE_ACTIVE_POWER,
				(u32_t *)&p->si_em_ind_px.rtaex, RS485_PORT_USED_BY_645);
		if (FRAME_E_OK != err_em_frame) {
//			p->si_em_ind_px.rtaex = INVLIDE_DATAL;
			if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
				sinkinfo_log(("get em phase A totalreverse active energy (%d)\n", err_em_frame));
		}
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	}else if(protocal == AP_PROTOCOL_EDMI){
		/* A相无功功率, 返回2bytes数据 */
		err_em_frame = get_data_from_em_xprotocol(AC_A_APPARENT_POWER,
				(u32_t *)&p->si_em_ind_px.appx, RS485_PORT_USED_BY_645);
		if (FRAME_E_OK != err_em_frame) {
//			p->si_em_ind_px.appx = INVLIDE_DATAL;
			if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
				sinkinfo_log(("get em pa react power fail(%d)\n", err_em_frame));
		}
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* 电网频率, 07规约中没有区分abc三相的频率, 统一使用电网频率, 返回2bytes数据 */
		err_em_frame = get_data_from_em_xprotocol(AC_POWER_FREQUENCY,
				(u32_t *)&p->si_em_ind_px.hzx, RS485_PORT_USED_BY_645);
		if (FRAME_E_OK != err_em_frame) {
//			p->si_em_ind_px.hzx = INVLIDE_DATAL;
			if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
				sinkinfo_log(("get em Grid frequency fail(%d)\n", err_em_frame));
		}
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));
		
		p->si_em_ind_px.vdx = INVLIDE_DATAL;
		p->si_em_ind_px.cdx = INVLIDE_DATAL;
		p->si_em_ind_px.taex = INVLIDE_DATAL;
		p->si_em_ind_px.rtaex = INVLIDE_DATAL;
	}else if(protocal == AP_PROTOCOL_645_1997 || protocal == AP_PROTOCOL_UNKNOWN){
		p->si_em_ind_px.appx = INVLIDE_DATAL;
		p->si_em_ind_px.vdx = INVLIDE_DATAL;
		p->si_em_ind_px.cdx = INVLIDE_DATAL;
		p->si_em_ind_px.hzx = INVLIDE_DATAL;
		p->si_em_ind_px.taex = INVLIDE_DATAL;
		p->si_em_ind_px.rtaex = INVLIDE_DATAL;
	}
	p->si_em_ind_px.phx = INVLIDE_DATAL;
	p->si_em_ind_px.viphx = INVLIDE_DATAL;

	copy_data_use_by_sinkinfo_buf(&sinkinfo_all_em[si_used_em_index.cur_em_index].si_em_ind_pa,
			sizeof(sinkinfo_all_em[0].si_em_ind_pa), &p->si_em_ind_px,
			sizeof(p->si_em_ind_px), __FUNCTION__, __LINE__);


#if 0 //EM_EVENT_SUPPERT /* 对电表事件的代码没有进行修正, mark by David */
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* A相功率因数, 返回2bytes数据 */
		for(i = 0; i<ELECTRIC_METER_EVENT_TIMES_MAX; i++){
		err_em_frame = get_event_data_from_ammeter(meteraddr, AMM_EVENT_A_LOSE_VOLTAGE, i+1, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);

		if (FRAME_E_OK != err_em_frame) {
			if (is_bit_set(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT)) {
				err_code = SIE_SECOND_GET_DATA_FAIL;
			} else {
				err_code = SIE_FAIL;
				set_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
			}

			sinkinfo_log(("get em pa power factor fail(%d)\n", err_em_frame));
			break;
		} else {
			rt_memcpy(p->si_em_volloss_event_px[i].start_time, (rt_uint32_t *)buf_of_recv_645_rep, sizeof(p->si_em_volloss_event_px[i].start_time));
			rt_memcpy(p->si_em_volloss_event_px[i].end_time, (rt_uint32_t *)&buf_of_recv_645_rep[6], sizeof(p->si_em_volloss_event_px[i].end_time));
			p->si_em_volloss_event_px[i].vA = buf_of_recv_645_rep[13]<<8 | buf_of_recv_645_rep[12];
			p->si_em_volloss_event_px[i].iA = buf_of_recv_645_rep[16]<<16 | buf_of_recv_645_rep[15]<<8 | buf_of_recv_645_rep[14];
			p->si_em_volloss_event_px[i].apA = buf_of_recv_645_rep[19]<<16 | buf_of_recv_645_rep[18]<<8 | buf_of_recv_645_rep[17];
			p->si_em_volloss_event_px[i].rapA = buf_of_recv_645_rep[22]<<16 | buf_of_recv_645_rep[21]<<8 | buf_of_recv_645_rep[20];
			p->si_em_volloss_event_px[i].pfA = buf_of_recv_645_rep[24]<<8 | buf_of_recv_645_rep[23];
			p->si_em_volloss_event_px[i].vB = buf_of_recv_645_rep[26]<<8 | buf_of_recv_645_rep[25];
			p->si_em_volloss_event_px[i].iB = buf_of_recv_645_rep[29]<<16 | buf_of_recv_645_rep[28]<<8 | buf_of_recv_645_rep[27];
			p->si_em_volloss_event_px[i].apB = buf_of_recv_645_rep[32]<<16 | buf_of_recv_645_rep[31]<<8 | buf_of_recv_645_rep[30];
			p->si_em_volloss_event_px[i].rapB = buf_of_recv_645_rep[35]<<16 | buf_of_recv_645_rep[34]<<8 | buf_of_recv_645_rep[33];
			p->si_em_volloss_event_px[i].pfB = buf_of_recv_645_rep[37]<<8 | buf_of_recv_645_rep[36];
			p->si_em_volloss_event_px[i].vC = buf_of_recv_645_rep[39]<<8 | buf_of_recv_645_rep[38];
			p->si_em_volloss_event_px[i].iC = buf_of_recv_645_rep[42]<<16 | buf_of_recv_645_rep[41]<<8 | buf_of_recv_645_rep[40];
			p->si_em_volloss_event_px[i].apC = buf_of_recv_645_rep[45]<<16 | buf_of_recv_645_rep[44]<<8 | buf_of_recv_645_rep[43];
			p->si_em_volloss_event_px[i].rapC = buf_of_recv_645_rep[48]<<16 | buf_of_recv_645_rep[47]<<8 | buf_of_recv_645_rep[46];
			p->si_em_volloss_event_px[i].pfC = buf_of_recv_645_rep[50]<<8 | buf_of_recv_645_rep[49];
			p->si_em_volloss_event_px[i].ahnA = buf_of_recv_645_rep[54]<<24 | buf_of_recv_645_rep[53]<<16 | buf_of_recv_645_rep[52]<<8 | buf_of_recv_645_rep[51];
			p->si_em_volloss_event_px[i].ahnB = buf_of_recv_645_rep[58]<<24 | buf_of_recv_645_rep[57]<<16 | buf_of_recv_645_rep[56]<<8 | buf_of_recv_645_rep[55];
			p->si_em_volloss_event_px[i].ahnC = buf_of_recv_645_rep[62]<<24 | buf_of_recv_645_rep[61]<<16 | buf_of_recv_645_rep[60]<<8 | buf_of_recv_645_rep[59];
			clr_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
		}

		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* A相功率因数, 返回2bytes数据 */
		err_em_frame = get_event_data_from_ammeter(meteraddr, AMM_EVENT_A_OVER_VOLTAGE, i+1, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);

		if (FRAME_E_OK != err_em_frame) {
			if (is_bit_set(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT)) {
				err_code = SIE_SECOND_GET_DATA_FAIL;
			} else {
				err_code = SIE_FAIL;
				set_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
			}

			sinkinfo_log(("get em pa power factor fail(%d)\n", err_em_frame));
			break;
		} else {
			rt_memcpy(p->si_em_volover_event_px[i].start_time, (rt_uint32_t *)buf_of_recv_645_rep, sizeof(p->si_em_volover_event_px[i].start_time));
			rt_memcpy(p->si_em_volover_event_px[i].end_time, (rt_uint32_t *)&buf_of_recv_645_rep[6], sizeof(p->si_em_volover_event_px[i].end_time));
			p->si_em_volover_event_px[i].vA = buf_of_recv_645_rep[13]<<8 | buf_of_recv_645_rep[12];
			p->si_em_volover_event_px[i].iA = buf_of_recv_645_rep[16]<<16 | buf_of_recv_645_rep[15]<<8 | buf_of_recv_645_rep[14];
			p->si_em_volover_event_px[i].apA = buf_of_recv_645_rep[19]<<16 | buf_of_recv_645_rep[18]<<8 | buf_of_recv_645_rep[17];
			p->si_em_volover_event_px[i].rapA = buf_of_recv_645_rep[22]<<16 | buf_of_recv_645_rep[21]<<8 | buf_of_recv_645_rep[20];
			p->si_em_volover_event_px[i].pfA = buf_of_recv_645_rep[24]<<8 | buf_of_recv_645_rep[23];
			p->si_em_volover_event_px[i].vB = buf_of_recv_645_rep[26]<<8 | buf_of_recv_645_rep[25];
			p->si_em_volover_event_px[i].iB = buf_of_recv_645_rep[29]<<16 | buf_of_recv_645_rep[28]<<8 | buf_of_recv_645_rep[27];
			p->si_em_volover_event_px[i].apB = buf_of_recv_645_rep[32]<<16 | buf_of_recv_645_rep[31]<<8 | buf_of_recv_645_rep[30];
			p->si_em_volover_event_px[i].rapB = buf_of_recv_645_rep[35]<<16 | buf_of_recv_645_rep[34]<<8 | buf_of_recv_645_rep[33];
			p->si_em_volover_event_px[i].pfB = buf_of_recv_645_rep[37]<<8 | buf_of_recv_645_rep[36];
			p->si_em_volover_event_px[i].vC = buf_of_recv_645_rep[39]<<8 | buf_of_recv_645_rep[38];
			p->si_em_volover_event_px[i].iC = buf_of_recv_645_rep[42]<<16 | buf_of_recv_645_rep[41]<<8 | buf_of_recv_645_rep[40];
			p->si_em_volover_event_px[i].apC = buf_of_recv_645_rep[45]<<16 | buf_of_recv_645_rep[44]<<8 | buf_of_recv_645_rep[43];
			p->si_em_volover_event_px[i].rapC = buf_of_recv_645_rep[48]<<16 | buf_of_recv_645_rep[47]<<8 | buf_of_recv_645_rep[46];
			p->si_em_volover_event_px[i].pfC = buf_of_recv_645_rep[50]<<8 | buf_of_recv_645_rep[49];
			p->si_em_volover_event_px[i].ahnA = buf_of_recv_645_rep[54]<<24 | buf_of_recv_645_rep[53]<<16 | buf_of_recv_645_rep[52]<<8 | buf_of_recv_645_rep[51];
			p->si_em_volover_event_px[i].ahnB = buf_of_recv_645_rep[58]<<24 | buf_of_recv_645_rep[57]<<16 | buf_of_recv_645_rep[56]<<8 | buf_of_recv_645_rep[55];
			p->si_em_volover_event_px[i].ahnC = buf_of_recv_645_rep[62]<<24 | buf_of_recv_645_rep[61]<<16 | buf_of_recv_645_rep[60]<<8 | buf_of_recv_645_rep[59];
			clr_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
		}

		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* A相功率因数, 返回2bytes数据 */
		err_em_frame = get_event_data_from_ammeter(meteraddr, AMM_EVENT_A_OWE_VOLTAGE, i+1, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);

		if (FRAME_E_OK != err_em_frame) {
			if (is_bit_set(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT)) {
				err_code = SIE_SECOND_GET_DATA_FAIL;
			} else {
				err_code = SIE_FAIL;
				set_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
			}

			sinkinfo_log(("get em pa power factor fail(%d)\n", err_em_frame));
			break;
		} else {
			rt_memcpy(p->si_em_volunder_event_px[i].start_time, (rt_uint32_t *)buf_of_recv_645_rep, sizeof(p->si_em_volunder_event_px[i].start_time));
			rt_memcpy(p->si_em_volunder_event_px[i].end_time, (rt_uint32_t *)&buf_of_recv_645_rep[6], sizeof(p->si_em_volunder_event_px[i].end_time));
			p->si_em_volunder_event_px[i].vA = buf_of_recv_645_rep[13]<<8 | buf_of_recv_645_rep[12];
			p->si_em_volunder_event_px[i].iA = buf_of_recv_645_rep[16]<<16 | buf_of_recv_645_rep[15]<<8 | buf_of_recv_645_rep[14];
			p->si_em_volunder_event_px[i].apA = buf_of_recv_645_rep[19]<<16 | buf_of_recv_645_rep[18]<<8 | buf_of_recv_645_rep[17];
			p->si_em_volunder_event_px[i].rapA = buf_of_recv_645_rep[22]<<16 | buf_of_recv_645_rep[21]<<8 | buf_of_recv_645_rep[20];
			p->si_em_volunder_event_px[i].pfA = buf_of_recv_645_rep[24]<<8 | buf_of_recv_645_rep[23];
			p->si_em_volunder_event_px[i].vB = buf_of_recv_645_rep[26]<<8 | buf_of_recv_645_rep[25];
			p->si_em_volunder_event_px[i].iB = buf_of_recv_645_rep[29]<<16 | buf_of_recv_645_rep[28]<<8 | buf_of_recv_645_rep[27];
			p->si_em_volunder_event_px[i].apB = buf_of_recv_645_rep[32]<<16 | buf_of_recv_645_rep[31]<<8 | buf_of_recv_645_rep[30];
			p->si_em_volunder_event_px[i].rapB = buf_of_recv_645_rep[35]<<16 | buf_of_recv_645_rep[34]<<8 | buf_of_recv_645_rep[33];
			p->si_em_volunder_event_px[i].pfB = buf_of_recv_645_rep[37]<<8 | buf_of_recv_645_rep[36];
			p->si_em_volunder_event_px[i].vC = buf_of_recv_645_rep[39]<<8 | buf_of_recv_645_rep[38];
			p->si_em_volunder_event_px[i].iC = buf_of_recv_645_rep[42]<<16 | buf_of_recv_645_rep[41]<<8 | buf_of_recv_645_rep[40];
			p->si_em_volunder_event_px[i].apC = buf_of_recv_645_rep[45]<<16 | buf_of_recv_645_rep[44]<<8 | buf_of_recv_645_rep[43];
			p->si_em_volunder_event_px[i].rapC = buf_of_recv_645_rep[48]<<16 | buf_of_recv_645_rep[47]<<8 | buf_of_recv_645_rep[46];
			p->si_em_volunder_event_px[i].pfC = buf_of_recv_645_rep[50]<<8 | buf_of_recv_645_rep[49];
			p->si_em_volunder_event_px[i].ahnA = buf_of_recv_645_rep[54]<<24 | buf_of_recv_645_rep[53]<<16 | buf_of_recv_645_rep[52]<<8 | buf_of_recv_645_rep[51];
			p->si_em_volunder_event_px[i].ahnB = buf_of_recv_645_rep[58]<<24 | buf_of_recv_645_rep[57]<<16 | buf_of_recv_645_rep[56]<<8 | buf_of_recv_645_rep[55];
			p->si_em_volunder_event_px[i].ahnC = buf_of_recv_645_rep[62]<<24 | buf_of_recv_645_rep[61]<<16 | buf_of_recv_645_rep[60]<<8 | buf_of_recv_645_rep[59];
			clr_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
		}

		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* A相功率因数, 返回2bytes数据 */
		err_em_frame = get_event_data_from_ammeter(meteraddr, AMM_EVENT_A_BROKEN_PHASE, i+1, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
		
		if (FRAME_E_OK != err_em_frame) {
			if (is_bit_set(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT)) {
				err_code = SIE_SECOND_GET_DATA_FAIL;
			} else {
				err_code = SIE_FAIL;
				set_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
			}

			sinkinfo_log(("get em pa power factor fail(%d)\n", err_em_frame));
			break;
		} else {
			rt_memcpy(p->si_em_phasebreak_event_px[i].start_time, (rt_uint32_t *)buf_of_recv_645_rep, sizeof(p->si_em_phasebreak_event_px[i].start_time));
			rt_memcpy(p->si_em_phasebreak_event_px[i].end_time, (rt_uint32_t *)&buf_of_recv_645_rep[6], sizeof(p->si_em_phasebreak_event_px[i].end_time));
			p->si_em_phasebreak_event_px[i].vA = buf_of_recv_645_rep[13]<<8 | buf_of_recv_645_rep[12];
			p->si_em_phasebreak_event_px[i].iA = buf_of_recv_645_rep[16]<<16 | buf_of_recv_645_rep[15]<<8 | buf_of_recv_645_rep[14];
			p->si_em_phasebreak_event_px[i].apA = buf_of_recv_645_rep[19]<<16 | buf_of_recv_645_rep[18]<<8 | buf_of_recv_645_rep[17];
			p->si_em_phasebreak_event_px[i].rapA = buf_of_recv_645_rep[22]<<16 | buf_of_recv_645_rep[21]<<8 | buf_of_recv_645_rep[20];
			p->si_em_phasebreak_event_px[i].pfA = buf_of_recv_645_rep[24]<<8 | buf_of_recv_645_rep[23];
			p->si_em_phasebreak_event_px[i].vB = buf_of_recv_645_rep[26]<<8 | buf_of_recv_645_rep[25];
			p->si_em_phasebreak_event_px[i].iB = buf_of_recv_645_rep[29]<<16 | buf_of_recv_645_rep[28]<<8 | buf_of_recv_645_rep[27];
			p->si_em_phasebreak_event_px[i].apB = buf_of_recv_645_rep[32]<<16 | buf_of_recv_645_rep[31]<<8 | buf_of_recv_645_rep[30];
			p->si_em_phasebreak_event_px[i].rapB = buf_of_recv_645_rep[35]<<16 | buf_of_recv_645_rep[34]<<8 | buf_of_recv_645_rep[33];
			p->si_em_phasebreak_event_px[i].pfB = buf_of_recv_645_rep[37]<<8 | buf_of_recv_645_rep[36];
			p->si_em_phasebreak_event_px[i].vC = buf_of_recv_645_rep[39]<<8 | buf_of_recv_645_rep[38];
			p->si_em_phasebreak_event_px[i].iC = buf_of_recv_645_rep[42]<<16 | buf_of_recv_645_rep[41]<<8 | buf_of_recv_645_rep[40];
			p->si_em_phasebreak_event_px[i].apC = buf_of_recv_645_rep[45]<<16 | buf_of_recv_645_rep[44]<<8 | buf_of_recv_645_rep[43];
			p->si_em_phasebreak_event_px[i].rapC = buf_of_recv_645_rep[48]<<16 | buf_of_recv_645_rep[47]<<8 | buf_of_recv_645_rep[46];
			p->si_em_phasebreak_event_px[i].pfC = buf_of_recv_645_rep[50]<<8 | buf_of_recv_645_rep[49];
			p->si_em_phasebreak_event_px[i].ahnA = buf_of_recv_645_rep[54]<<24 | buf_of_recv_645_rep[53]<<16 | buf_of_recv_645_rep[52]<<8 | buf_of_recv_645_rep[51];
			p->si_em_phasebreak_event_px[i].ahnB = buf_of_recv_645_rep[58]<<24 | buf_of_recv_645_rep[57]<<16 | buf_of_recv_645_rep[56]<<8 | buf_of_recv_645_rep[55];
			p->si_em_phasebreak_event_px[i].ahnC = buf_of_recv_645_rep[62]<<24 | buf_of_recv_645_rep[61]<<16 | buf_of_recv_645_rep[60]<<8 | buf_of_recv_645_rep[59];
			clr_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
		}

		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* A相功率因数, 返回2bytes数据 */
		err_em_frame = get_event_data_from_ammeter(meteraddr, AMM_EVENT_A_LOSE_CURRENT, i+1, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);

		if (FRAME_E_OK != err_em_frame) {
			if (is_bit_set(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT)) {
				err_code = SIE_SECOND_GET_DATA_FAIL;
			} else {
				err_code = SIE_FAIL;
				set_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
			}

			sinkinfo_log(("get em pa power factor fail(%d)\n", err_em_frame));
			break;
		} else {
			rt_memcpy(p->si_em_curloss_event_px[i].start_time, (rt_uint32_t *)buf_of_recv_645_rep, sizeof(p->si_em_curloss_event_px[i].start_time));
			rt_memcpy(p->si_em_curloss_event_px[i].end_time, (rt_uint32_t *)&buf_of_recv_645_rep[6], sizeof(p->si_em_curloss_event_px[i].end_time));
			p->si_em_curloss_event_px[i].vA = buf_of_recv_645_rep[13]<<8 | buf_of_recv_645_rep[12];
			p->si_em_curloss_event_px[i].iA = buf_of_recv_645_rep[16]<<16 | buf_of_recv_645_rep[15]<<8 | buf_of_recv_645_rep[14];
			p->si_em_curloss_event_px[i].apA = buf_of_recv_645_rep[19]<<16 | buf_of_recv_645_rep[18]<<8 | buf_of_recv_645_rep[17];
			p->si_em_curloss_event_px[i].rapA = buf_of_recv_645_rep[22]<<16 | buf_of_recv_645_rep[21]<<8 | buf_of_recv_645_rep[20];
			p->si_em_curloss_event_px[i].pfA = buf_of_recv_645_rep[24]<<8 | buf_of_recv_645_rep[23];
			p->si_em_curloss_event_px[i].vB = buf_of_recv_645_rep[26]<<8 | buf_of_recv_645_rep[25];
			p->si_em_curloss_event_px[i].iB = buf_of_recv_645_rep[29]<<16 | buf_of_recv_645_rep[28]<<8 | buf_of_recv_645_rep[27];
			p->si_em_curloss_event_px[i].apB = buf_of_recv_645_rep[32]<<16 | buf_of_recv_645_rep[31]<<8 | buf_of_recv_645_rep[30];
			p->si_em_curloss_event_px[i].rapB = buf_of_recv_645_rep[35]<<16 | buf_of_recv_645_rep[34]<<8 | buf_of_recv_645_rep[33];
			p->si_em_curloss_event_px[i].pfB = buf_of_recv_645_rep[37]<<8 | buf_of_recv_645_rep[36];
			p->si_em_curloss_event_px[i].vC = buf_of_recv_645_rep[39]<<8 | buf_of_recv_645_rep[38];
			p->si_em_curloss_event_px[i].iC = buf_of_recv_645_rep[42]<<16 | buf_of_recv_645_rep[41]<<8 | buf_of_recv_645_rep[40];
			p->si_em_curloss_event_px[i].apC = buf_of_recv_645_rep[45]<<16 | buf_of_recv_645_rep[44]<<8 | buf_of_recv_645_rep[43];
			p->si_em_curloss_event_px[i].rapC = buf_of_recv_645_rep[48]<<16 | buf_of_recv_645_rep[47]<<8 | buf_of_recv_645_rep[46];
			p->si_em_curloss_event_px[i].pfC = buf_of_recv_645_rep[50]<<8 | buf_of_recv_645_rep[49];
			clr_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
		}

		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* A相功率因数, 返回2bytes数据 */
		err_em_frame = get_event_data_from_ammeter(meteraddr, AMM_EVENT_A_OVER_CURRENT, i+1, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);

		if (FRAME_E_OK != err_em_frame) {
			if (is_bit_set(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT)) {
				err_code = SIE_SECOND_GET_DATA_FAIL;
			} else {
				err_code = SIE_FAIL;
				set_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
			}

			sinkinfo_log(("get em pa power factor fail(%d)\n", err_em_frame));
			break;
		} else {
			rt_memcpy(p->si_em_curover_event_px[i].start_time, (rt_uint32_t *)buf_of_recv_645_rep, sizeof(p->si_em_curover_event_px[i].start_time));
			rt_memcpy(p->si_em_curover_event_px[i].end_time, (rt_uint32_t *)&buf_of_recv_645_rep[6], sizeof(p->si_em_curover_event_px[i].end_time));
			p->si_em_curover_event_px[i].vA = buf_of_recv_645_rep[13]<<8 | buf_of_recv_645_rep[12];
			p->si_em_curover_event_px[i].iA = buf_of_recv_645_rep[16]<<16 | buf_of_recv_645_rep[15]<<8 | buf_of_recv_645_rep[14];
			p->si_em_curover_event_px[i].apA = buf_of_recv_645_rep[19]<<16 | buf_of_recv_645_rep[18]<<8 | buf_of_recv_645_rep[17];
			p->si_em_curover_event_px[i].rapA = buf_of_recv_645_rep[22]<<16 | buf_of_recv_645_rep[21]<<8 | buf_of_recv_645_rep[20];
			p->si_em_curover_event_px[i].pfA = buf_of_recv_645_rep[24]<<8 | buf_of_recv_645_rep[23];
			p->si_em_curover_event_px[i].vB = buf_of_recv_645_rep[26]<<8 | buf_of_recv_645_rep[25];
			p->si_em_curover_event_px[i].iB = buf_of_recv_645_rep[29]<<16 | buf_of_recv_645_rep[28]<<8 | buf_of_recv_645_rep[27];
			p->si_em_curover_event_px[i].apB = buf_of_recv_645_rep[32]<<16 | buf_of_recv_645_rep[31]<<8 | buf_of_recv_645_rep[30];
			p->si_em_curover_event_px[i].rapB = buf_of_recv_645_rep[35]<<16 | buf_of_recv_645_rep[34]<<8 | buf_of_recv_645_rep[33];
			p->si_em_curover_event_px[i].pfB = buf_of_recv_645_rep[37]<<8 | buf_of_recv_645_rep[36];
			p->si_em_curover_event_px[i].vC = buf_of_recv_645_rep[39]<<8 | buf_of_recv_645_rep[38];
			p->si_em_curover_event_px[i].iC = buf_of_recv_645_rep[42]<<16 | buf_of_recv_645_rep[41]<<8 | buf_of_recv_645_rep[40];
			p->si_em_curover_event_px[i].apC = buf_of_recv_645_rep[45]<<16 | buf_of_recv_645_rep[44]<<8 | buf_of_recv_645_rep[43];
			p->si_em_curover_event_px[i].rapC = buf_of_recv_645_rep[48]<<16 | buf_of_recv_645_rep[47]<<8 | buf_of_recv_645_rep[46];
			p->si_em_curover_event_px[i].pfC = buf_of_recv_645_rep[50]<<8 | buf_of_recv_645_rep[49];
			clr_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
		}

		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* A相功率因数, 返回2bytes数据 */
		err_em_frame = get_event_data_from_ammeter(meteraddr, AMM_EVENT_A_BROKEN_CURRENT, i+1, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
		
		if (FRAME_E_OK != err_em_frame) {
			if (is_bit_set(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT)) {
				err_code = SIE_SECOND_GET_DATA_FAIL;
			} else {
				err_code = SIE_FAIL;
				set_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
			}

			sinkinfo_log(("get em pa power factor fail(%d)\n", err_em_frame));
			break;
		} else {
			rt_memcpy(p->si_em_curbreak_event_px[i].start_time, (rt_uint32_t *)buf_of_recv_645_rep, sizeof(p->si_em_curbreak_event_px[i].start_time));
			rt_memcpy(p->si_em_curbreak_event_px[i].end_time, (rt_uint32_t *)&buf_of_recv_645_rep[6], sizeof(p->si_em_curbreak_event_px[i].end_time));
			p->si_em_curbreak_event_px[i].vA = buf_of_recv_645_rep[13]<<8 | buf_of_recv_645_rep[12];
			p->si_em_curbreak_event_px[i].iA = buf_of_recv_645_rep[16]<<16 | buf_of_recv_645_rep[15]<<8 | buf_of_recv_645_rep[14];
			p->si_em_curbreak_event_px[i].apA = buf_of_recv_645_rep[19]<<16 | buf_of_recv_645_rep[18]<<8 | buf_of_recv_645_rep[17];
			p->si_em_curbreak_event_px[i].rapA = buf_of_recv_645_rep[22]<<16 | buf_of_recv_645_rep[21]<<8 | buf_of_recv_645_rep[20];
			p->si_em_curbreak_event_px[i].pfA = buf_of_recv_645_rep[24]<<8 | buf_of_recv_645_rep[23];
			p->si_em_curbreak_event_px[i].vB = buf_of_recv_645_rep[26]<<8 | buf_of_recv_645_rep[25];
			p->si_em_curbreak_event_px[i].iB = buf_of_recv_645_rep[29]<<16 | buf_of_recv_645_rep[28]<<8 | buf_of_recv_645_rep[27];
			p->si_em_curbreak_event_px[i].apB = buf_of_recv_645_rep[32]<<16 | buf_of_recv_645_rep[31]<<8 | buf_of_recv_645_rep[30];
			p->si_em_curbreak_event_px[i].rapB = buf_of_recv_645_rep[35]<<16 | buf_of_recv_645_rep[34]<<8 | buf_of_recv_645_rep[33];
			p->si_em_curbreak_event_px[i].pfB = buf_of_recv_645_rep[37]<<8 | buf_of_recv_645_rep[36];
			p->si_em_curbreak_event_px[i].vC = buf_of_recv_645_rep[39]<<8 | buf_of_recv_645_rep[38];
			p->si_em_curbreak_event_px[i].iC = buf_of_recv_645_rep[42]<<16 | buf_of_recv_645_rep[41]<<8 | buf_of_recv_645_rep[40];
			p->si_em_curbreak_event_px[i].apC = buf_of_recv_645_rep[45]<<16 | buf_of_recv_645_rep[44]<<8 | buf_of_recv_645_rep[43];
			p->si_em_curbreak_event_px[i].rapC = buf_of_recv_645_rep[48]<<16 | buf_of_recv_645_rep[47]<<8 | buf_of_recv_645_rep[46];
			p->si_em_curbreak_event_px[i].pfC = buf_of_recv_645_rep[50]<<8 | buf_of_recv_645_rep[49];
			clr_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
		}

		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));
		}
#endif


	/* 获取电表中b相所有abc三相独立的参数, by 645 */
	if (CONNECT_33 == register_em_info.em_wire_con_mode[si_used_em_index.cur_em_index]) {
		/* 3相3线的B相应该没有数据 */
		p->si_em_ind_px.vx = INVLIDE_DATAL;
		p->si_em_ind_px.ix = INVLIDE_DATAL;
		p->si_em_ind_px.apx = INVLIDE_DATAL;
		p->si_em_ind_px.rapx = INVLIDE_DATAL;
		p->si_em_ind_px.appx = INVLIDE_DATAL;
		p->si_em_ind_px.pfx = INVLIDE_DATAL;
		p->si_em_ind_px.phx = INVLIDE_DATAL;
		p->si_em_ind_px.viphx = INVLIDE_DATAL;
		p->si_em_ind_px.hzx = INVLIDE_DATAL;
		p->si_em_ind_px.vdx = INVLIDE_DATAL;
		p->si_em_ind_px.cdx = INVLIDE_DATAL;
		p->si_em_ind_px.taex = INVLIDE_DATAL;
		p->si_em_ind_px.rtaex = INVLIDE_DATAL;
	} else {
//		rt_memset(&p->si_em_ind_px, 0, sizeof(p->si_em_ind_px));
		copy_data_use_by_sinkinfo_buf(&p->si_em_ind_px, sizeof(p->si_em_ind_px),
				&sinkinfo_all_em[si_used_em_index.cur_em_index].si_em_ind_pb,
				sizeof(sinkinfo_all_em[0].si_em_ind_pb),
				__FUNCTION__, __LINE__);

		/* B相电压, 返回2bytes数据 */
		err_em_frame = get_data_from_em_xprotocol(AC_B_VOLTAGE,
				&p->si_em_ind_px.vx, RS485_PORT_USED_BY_645);
		if (FRAME_E_OK != err_em_frame) {
//			p->si_em_ind_px.vx = INVLIDE_DATAL;
			if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
				sinkinfo_log(("get em pb voltage fail(%d)\n", err_em_frame));
		}
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* B相电流, 返回2bytes数据 */
		err_em_frame = get_data_from_em_xprotocol(AC_B_CURRENT,
				&p->si_em_ind_px.ix, RS485_PORT_USED_BY_645);
		if (FRAME_E_OK != err_em_frame) {
//			p->si_em_ind_px.ix = INVLIDE_DATAL;
			if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
				sinkinfo_log(("get em pb current fail(%d)\n", err_em_frame));
		}
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* B相有功功率, 返回3bytes数据 */
		err_em_frame = get_data_from_em_xprotocol(AC_B_ACTIVE_POWER,
				&p->si_em_ind_px.apx, RS485_PORT_USED_BY_645);
		if (FRAME_E_OK != err_em_frame) {
//			p->si_em_ind_px.apx = INVLIDE_DATAL;
			if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
				sinkinfo_log(("get em pb act power fail(%d)\n", err_em_frame));
		}
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* B相无功功率, 返回2bytes数据 */
		err_em_frame = get_data_from_em_xprotocol(AC_B_REACTIVE_POWER,
				&p->si_em_ind_px.rapx, RS485_PORT_USED_BY_645);
		if (FRAME_E_OK != err_em_frame) {
//			p->si_em_ind_px.rapx = INVLIDE_DATAL;
			if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
				sinkinfo_log(("get em pb react power fail(%d)\n", err_em_frame));
		}
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* B相功率因数, 返回2bytes数据 */
		err_em_frame = get_data_from_em_xprotocol(AC_B_POWER_FACTOR,
				&p->si_em_ind_px.pfx, RS485_PORT_USED_BY_645);
		if (FRAME_E_OK != err_em_frame) {
//			p->si_em_ind_px.pfx = INVLIDE_DATAL;
			if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
				sinkinfo_log(("get em pb power factor fail(%d)\n", err_em_frame));
		}
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		if(protocal == AP_PROTOCOL_645_2007){
			/* B相无功功率, 返回2bytes数据 */
			err_em_frame = get_data_from_em_xprotocol(AC_B_APPARENT_POWER,
					(u32_t *)&p->si_em_ind_px.appx, RS485_PORT_USED_BY_645);
			if (FRAME_E_OK != err_em_frame) {
//				p->si_em_ind_px.appx = INVLIDE_DATAL;
				if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
					sinkinfo_log(("get em pb react power fail(%d)\n", err_em_frame));
			}
			rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

			/* B相功率因数, 返回2bytes数据 */
			err_em_frame = get_data_from_em_xprotocol(AC_B_VOLTAGE_DISTORTION,
					(u32_t *)&p->si_em_ind_px.vdx, RS485_PORT_USED_BY_645);
			if (FRAME_E_OK != err_em_frame) {
//				p->si_em_ind_px.vdx = INVLIDE_DATAL;
				if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
					sinkinfo_log(("get em pb power factor fail(%d)\n", err_em_frame));
			}
			rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

			/* B相功率因数, 返回2bytes数据 */
			err_em_frame = get_data_from_em_xprotocol(AC_B_CURRENT_DISTORTION,
					(u32_t *)&p->si_em_ind_px.cdx, RS485_PORT_USED_BY_645);
			if (FRAME_E_OK != err_em_frame) {
//				p->si_em_ind_px.cdx = INVLIDE_DATAL;
				if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
					sinkinfo_log(("get em pb power factor fail(%d)\n", err_em_frame));
			}
			rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

			/* B相正向有功总电能 */
			err_em_frame = get_data_from_em_xprotocol(AC_B_POSITIVE_ACTIVE_POWER,
					(u32_t *)&p->si_em_ind_px.taex, RS485_PORT_USED_BY_645);
			if (FRAME_E_OK != err_em_frame) {
//				p->si_em_ind_px.taex = INVLIDE_DATAL;
				if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
					sinkinfo_log(("get em phase B total active energy (%d)\n", err_em_frame));
			}
			rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

			/* B相反向有功总电能 */
			err_em_frame = get_data_from_em_xprotocol(AC_B_REPOSITIVE_ACTIVE_POWER,
					(u32_t *)&p->si_em_ind_px.rtaex, RS485_PORT_USED_BY_645);
			if (FRAME_E_OK != err_em_frame) {
//				p->si_em_ind_px.rtaex = INVLIDE_DATAL;
				if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
					sinkinfo_log(("get em phase B totalreverse active energy (%d)\n", err_em_frame));
			}
			rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));
		}else if(protocal == AP_PROTOCOL_EDMI){
			/* A相无功功率, 返回2bytes数据 */
			err_em_frame = get_data_from_em_xprotocol(AC_B_APPARENT_POWER,
					(u32_t *)&p->si_em_ind_px.appx, RS485_PORT_USED_BY_645);
			if (FRAME_E_OK != err_em_frame) {
//				p->si_em_ind_px.appx = INVLIDE_DATAL;
				if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
					sinkinfo_log(("get em pb react power fail(%d)\n", err_em_frame));
			}
			rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

			p->si_em_ind_px.vdx = INVLIDE_DATAL;
			p->si_em_ind_px.cdx = INVLIDE_DATAL;
			p->si_em_ind_px.taex = INVLIDE_DATAL;
			p->si_em_ind_px.rtaex = INVLIDE_DATAL;

		}else if(protocal == AP_PROTOCOL_645_1997 || protocal == AP_PROTOCOL_UNKNOWN){
			p->si_em_ind_px.appx = INVLIDE_DATAL;
			p->si_em_ind_px.vdx = INVLIDE_DATAL;
			p->si_em_ind_px.cdx = INVLIDE_DATAL;
			p->si_em_ind_px.taex = INVLIDE_DATAL;
			p->si_em_ind_px.rtaex = INVLIDE_DATAL;
		}
		p->si_em_ind_px.phx = INVLIDE_DATAL;
		p->si_em_ind_px.viphx = INVLIDE_DATAL;
		/* 电网频率, 07规约中没有区分abc三相的频率, 统一使用电网频率, 返回2bytes数据 */
		if(register_em_info.em_wire_con_mode[si_used_em_index.cur_em_index]== CONNECT_34){
			p->si_em_ind_px.hzx = sinkinfo_all_em[si_used_em_index.cur_em_index].si_em_ind_pa.hzx;
		}
		else{
			p->si_em_ind_px.hzx = INVLIDE_DATAL;
		}
	}
	copy_data_use_by_sinkinfo_buf(&sinkinfo_all_em[si_used_em_index.cur_em_index].si_em_ind_pb,
			sizeof(sinkinfo_all_em[0].si_em_ind_pb), &p->si_em_ind_px,
			sizeof(p->si_em_ind_px), __FUNCTION__, __LINE__);

#if 0 //EM_EVENT_SUPPERT /* 对电表事件的代码没有进行修正, mark by David */
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* A相功率因数, 返回2bytes数据 */
		for(i = 0; i<ELECTRIC_METER_EVENT_TIMES_MAX; i++){
		err_em_frame = get_event_data_from_ammeter(meteraddr, AMM_EVENT_B_LOSE_VOLTAGE, i+1, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
		
		if (FRAME_E_OK != err_em_frame) {
			if (is_bit_set(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT)) {
				err_code = SIE_SECOND_GET_DATA_FAIL;
			} else {
				err_code = SIE_FAIL;
				set_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
			}

			sinkinfo_log(("get em pa power factor fail(%d)\n", err_em_frame));
			break;
		} else {
			rt_memcpy(p->si_em_volloss_event_px[i].start_time, (rt_uint32_t *)buf_of_recv_645_rep, sizeof(p->si_em_volloss_event_px[i].start_time));
			rt_memcpy(p->si_em_volloss_event_px[i].end_time, (rt_uint32_t *)&buf_of_recv_645_rep[6], sizeof(p->si_em_volloss_event_px[i].end_time));
			p->si_em_volloss_event_px[i].vA = buf_of_recv_645_rep[13]<<8 | buf_of_recv_645_rep[12];
			p->si_em_volloss_event_px[i].iA = buf_of_recv_645_rep[16]<<16 | buf_of_recv_645_rep[15]<<8 | buf_of_recv_645_rep[14];
			p->si_em_volloss_event_px[i].apA = buf_of_recv_645_rep[19]<<16 | buf_of_recv_645_rep[18]<<8 | buf_of_recv_645_rep[17];
			p->si_em_volloss_event_px[i].rapA = buf_of_recv_645_rep[22]<<16 | buf_of_recv_645_rep[21]<<8 | buf_of_recv_645_rep[20];
			p->si_em_volloss_event_px[i].pfA = buf_of_recv_645_rep[24]<<8 | buf_of_recv_645_rep[23];
			p->si_em_volloss_event_px[i].vB = buf_of_recv_645_rep[26]<<8 | buf_of_recv_645_rep[25];
			p->si_em_volloss_event_px[i].iB = buf_of_recv_645_rep[29]<<16 | buf_of_recv_645_rep[28]<<8 | buf_of_recv_645_rep[27];
			p->si_em_volloss_event_px[i].apB = buf_of_recv_645_rep[32]<<16 | buf_of_recv_645_rep[31]<<8 | buf_of_recv_645_rep[30];
			p->si_em_volloss_event_px[i].rapB = buf_of_recv_645_rep[35]<<16 | buf_of_recv_645_rep[34]<<8 | buf_of_recv_645_rep[33];
			p->si_em_volloss_event_px[i].pfB = buf_of_recv_645_rep[37]<<8 | buf_of_recv_645_rep[36];
			p->si_em_volloss_event_px[i].vC = buf_of_recv_645_rep[39]<<8 | buf_of_recv_645_rep[38];
			p->si_em_volloss_event_px[i].iC = buf_of_recv_645_rep[42]<<16 | buf_of_recv_645_rep[41]<<8 | buf_of_recv_645_rep[40];
			p->si_em_volloss_event_px[i].apC = buf_of_recv_645_rep[45]<<16 | buf_of_recv_645_rep[44]<<8 | buf_of_recv_645_rep[43];
			p->si_em_volloss_event_px[i].rapC = buf_of_recv_645_rep[48]<<16 | buf_of_recv_645_rep[47]<<8 | buf_of_recv_645_rep[46];
			p->si_em_volloss_event_px[i].pfC = buf_of_recv_645_rep[50]<<8 | buf_of_recv_645_rep[49];
			p->si_em_volloss_event_px[i].ahnA = buf_of_recv_645_rep[54]<<24 | buf_of_recv_645_rep[53]<<16 | buf_of_recv_645_rep[52]<<8 | buf_of_recv_645_rep[51];
			p->si_em_volloss_event_px[i].ahnB = buf_of_recv_645_rep[58]<<24 | buf_of_recv_645_rep[57]<<16 | buf_of_recv_645_rep[56]<<8 | buf_of_recv_645_rep[55];
			p->si_em_volloss_event_px[i].ahnC = buf_of_recv_645_rep[62]<<24 | buf_of_recv_645_rep[61]<<16 | buf_of_recv_645_rep[60]<<8 | buf_of_recv_645_rep[59];
			clr_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
		}
		
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* A相功率因数, 返回2bytes数据 */
		err_em_frame = get_event_data_from_ammeter(meteraddr, AMM_EVENT_B_OVER_VOLTAGE, i+1, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
		
		if (FRAME_E_OK != err_em_frame) {
			if (is_bit_set(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT)) {
				err_code = SIE_SECOND_GET_DATA_FAIL;
			} else {
				err_code = SIE_FAIL;
				set_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
			}

			sinkinfo_log(("get em pa power factor fail(%d)\n", err_em_frame));
			break;
		} else {
			rt_memcpy(p->si_em_volover_event_px[i].start_time, (rt_uint32_t *)buf_of_recv_645_rep, sizeof(p->si_em_volover_event_px[i].start_time));
			rt_memcpy(p->si_em_volover_event_px[i].end_time, (rt_uint32_t *)&buf_of_recv_645_rep[6], sizeof(p->si_em_volover_event_px[i].end_time));
			p->si_em_volover_event_px[i].vA = buf_of_recv_645_rep[13]<<8 | buf_of_recv_645_rep[12];
			p->si_em_volover_event_px[i].iA = buf_of_recv_645_rep[16]<<16 | buf_of_recv_645_rep[15]<<8 | buf_of_recv_645_rep[14];
			p->si_em_volover_event_px[i].apA = buf_of_recv_645_rep[19]<<16 | buf_of_recv_645_rep[18]<<8 | buf_of_recv_645_rep[17];
			p->si_em_volover_event_px[i].rapA = buf_of_recv_645_rep[22]<<16 | buf_of_recv_645_rep[21]<<8 | buf_of_recv_645_rep[20];
			p->si_em_volover_event_px[i].pfA = buf_of_recv_645_rep[24]<<8 | buf_of_recv_645_rep[23];
			p->si_em_volover_event_px[i].vB = buf_of_recv_645_rep[26]<<8 | buf_of_recv_645_rep[25];
			p->si_em_volover_event_px[i].iB = buf_of_recv_645_rep[29]<<16 | buf_of_recv_645_rep[28]<<8 | buf_of_recv_645_rep[27];
			p->si_em_volover_event_px[i].apB = buf_of_recv_645_rep[32]<<16 | buf_of_recv_645_rep[31]<<8 | buf_of_recv_645_rep[30];
			p->si_em_volover_event_px[i].rapB = buf_of_recv_645_rep[35]<<16 | buf_of_recv_645_rep[34]<<8 | buf_of_recv_645_rep[33];
			p->si_em_volover_event_px[i].pfB = buf_of_recv_645_rep[37]<<8 | buf_of_recv_645_rep[36];
			p->si_em_volover_event_px[i].vC = buf_of_recv_645_rep[39]<<8 | buf_of_recv_645_rep[38];
			p->si_em_volover_event_px[i].iC = buf_of_recv_645_rep[42]<<16 | buf_of_recv_645_rep[41]<<8 | buf_of_recv_645_rep[40];
			p->si_em_volover_event_px[i].apC = buf_of_recv_645_rep[45]<<16 | buf_of_recv_645_rep[44]<<8 | buf_of_recv_645_rep[43];
			p->si_em_volover_event_px[i].rapC = buf_of_recv_645_rep[48]<<16 | buf_of_recv_645_rep[47]<<8 | buf_of_recv_645_rep[46];
			p->si_em_volover_event_px[i].pfC = buf_of_recv_645_rep[50]<<8 | buf_of_recv_645_rep[49];
			p->si_em_volover_event_px[i].ahnA = buf_of_recv_645_rep[54]<<24 | buf_of_recv_645_rep[53]<<16 | buf_of_recv_645_rep[52]<<8 | buf_of_recv_645_rep[51];
			p->si_em_volover_event_px[i].ahnB = buf_of_recv_645_rep[58]<<24 | buf_of_recv_645_rep[57]<<16 | buf_of_recv_645_rep[56]<<8 | buf_of_recv_645_rep[55];
			p->si_em_volover_event_px[i].ahnC = buf_of_recv_645_rep[62]<<24 | buf_of_recv_645_rep[61]<<16 | buf_of_recv_645_rep[60]<<8 | buf_of_recv_645_rep[59];
			clr_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
		}
		
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* A相功率因数, 返回2bytes数据 */
		err_em_frame = get_event_data_from_ammeter(meteraddr, AMM_EVENT_B_OWE_VOLTAGE, i+1, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
		
		if (FRAME_E_OK != err_em_frame) {
			if (is_bit_set(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT)) {
				err_code = SIE_SECOND_GET_DATA_FAIL;
			} else {
				err_code = SIE_FAIL;
				set_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
			}

			sinkinfo_log(("get em pa power factor fail(%d)\n", err_em_frame));
			break;
		} else {
			rt_memcpy(p->si_em_volunder_event_px[i].start_time, (rt_uint32_t *)buf_of_recv_645_rep, sizeof(p->si_em_volunder_event_px[i].start_time));
			rt_memcpy(p->si_em_volunder_event_px[i].end_time, (rt_uint32_t *)&buf_of_recv_645_rep[6], sizeof(p->si_em_volunder_event_px[i].end_time));
			p->si_em_volunder_event_px[i].vA = buf_of_recv_645_rep[13]<<8 | buf_of_recv_645_rep[12];
			p->si_em_volunder_event_px[i].iA = buf_of_recv_645_rep[16]<<16 | buf_of_recv_645_rep[15]<<8 | buf_of_recv_645_rep[14];
			p->si_em_volunder_event_px[i].apA = buf_of_recv_645_rep[19]<<16 | buf_of_recv_645_rep[18]<<8 | buf_of_recv_645_rep[17];
			p->si_em_volunder_event_px[i].rapA = buf_of_recv_645_rep[22]<<16 | buf_of_recv_645_rep[21]<<8 | buf_of_recv_645_rep[20];
			p->si_em_volunder_event_px[i].pfA = buf_of_recv_645_rep[24]<<8 | buf_of_recv_645_rep[23];
			p->si_em_volunder_event_px[i].vB = buf_of_recv_645_rep[26]<<8 | buf_of_recv_645_rep[25];
			p->si_em_volunder_event_px[i].iB = buf_of_recv_645_rep[29]<<16 | buf_of_recv_645_rep[28]<<8 | buf_of_recv_645_rep[27];
			p->si_em_volunder_event_px[i].apB = buf_of_recv_645_rep[32]<<16 | buf_of_recv_645_rep[31]<<8 | buf_of_recv_645_rep[30];
			p->si_em_volunder_event_px[i].rapB = buf_of_recv_645_rep[35]<<16 | buf_of_recv_645_rep[34]<<8 | buf_of_recv_645_rep[33];
			p->si_em_volunder_event_px[i].pfB = buf_of_recv_645_rep[37]<<8 | buf_of_recv_645_rep[36];
			p->si_em_volunder_event_px[i].vC = buf_of_recv_645_rep[39]<<8 | buf_of_recv_645_rep[38];
			p->si_em_volunder_event_px[i].iC = buf_of_recv_645_rep[42]<<16 | buf_of_recv_645_rep[41]<<8 | buf_of_recv_645_rep[40];
			p->si_em_volunder_event_px[i].apC = buf_of_recv_645_rep[45]<<16 | buf_of_recv_645_rep[44]<<8 | buf_of_recv_645_rep[43];
			p->si_em_volunder_event_px[i].rapC = buf_of_recv_645_rep[48]<<16 | buf_of_recv_645_rep[47]<<8 | buf_of_recv_645_rep[46];
			p->si_em_volunder_event_px[i].pfC = buf_of_recv_645_rep[50]<<8 | buf_of_recv_645_rep[49];
			p->si_em_volunder_event_px[i].ahnA = buf_of_recv_645_rep[54]<<24 | buf_of_recv_645_rep[53]<<16 | buf_of_recv_645_rep[52]<<8 | buf_of_recv_645_rep[51];
			p->si_em_volunder_event_px[i].ahnB = buf_of_recv_645_rep[58]<<24 | buf_of_recv_645_rep[57]<<16 | buf_of_recv_645_rep[56]<<8 | buf_of_recv_645_rep[55];
			p->si_em_volunder_event_px[i].ahnC = buf_of_recv_645_rep[62]<<24 | buf_of_recv_645_rep[61]<<16 | buf_of_recv_645_rep[60]<<8 | buf_of_recv_645_rep[59];
			clr_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
		}
		
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* A相功率因数, 返回2bytes数据 */
		err_em_frame = get_event_data_from_ammeter(meteraddr, AMM_EVENT_B_BROKEN_PHASE, i+1, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
		
		if (FRAME_E_OK != err_em_frame) {
			if (is_bit_set(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT)) {
				err_code = SIE_SECOND_GET_DATA_FAIL;
			} else {
				err_code = SIE_FAIL;
				set_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
			}

			sinkinfo_log(("get em pa power factor fail(%d)\n", err_em_frame));
			break;
		} else {
			rt_memcpy(p->si_em_phasebreak_event_px[i].start_time, (rt_uint32_t *)buf_of_recv_645_rep, sizeof(p->si_em_phasebreak_event_px[i].start_time));
			rt_memcpy(p->si_em_phasebreak_event_px[i].end_time, (rt_uint32_t *)&buf_of_recv_645_rep[6], sizeof(p->si_em_phasebreak_event_px[i].end_time));
			p->si_em_phasebreak_event_px[i].vA = buf_of_recv_645_rep[13]<<8 | buf_of_recv_645_rep[12];
			p->si_em_phasebreak_event_px[i].iA = buf_of_recv_645_rep[16]<<16 | buf_of_recv_645_rep[15]<<8 | buf_of_recv_645_rep[14];
			p->si_em_phasebreak_event_px[i].apA = buf_of_recv_645_rep[19]<<16 | buf_of_recv_645_rep[18]<<8 | buf_of_recv_645_rep[17];
			p->si_em_phasebreak_event_px[i].rapA = buf_of_recv_645_rep[22]<<16 | buf_of_recv_645_rep[21]<<8 | buf_of_recv_645_rep[20];
			p->si_em_phasebreak_event_px[i].pfA = buf_of_recv_645_rep[24]<<8 | buf_of_recv_645_rep[23];
			p->si_em_phasebreak_event_px[i].vB = buf_of_recv_645_rep[26]<<8 | buf_of_recv_645_rep[25];
			p->si_em_phasebreak_event_px[i].iB = buf_of_recv_645_rep[29]<<16 | buf_of_recv_645_rep[28]<<8 | buf_of_recv_645_rep[27];
			p->si_em_phasebreak_event_px[i].apB = buf_of_recv_645_rep[32]<<16 | buf_of_recv_645_rep[31]<<8 | buf_of_recv_645_rep[30];
			p->si_em_phasebreak_event_px[i].rapB = buf_of_recv_645_rep[35]<<16 | buf_of_recv_645_rep[34]<<8 | buf_of_recv_645_rep[33];
			p->si_em_phasebreak_event_px[i].pfB = buf_of_recv_645_rep[37]<<8 | buf_of_recv_645_rep[36];
			p->si_em_phasebreak_event_px[i].vC = buf_of_recv_645_rep[39]<<8 | buf_of_recv_645_rep[38];
			p->si_em_phasebreak_event_px[i].iC = buf_of_recv_645_rep[42]<<16 | buf_of_recv_645_rep[41]<<8 | buf_of_recv_645_rep[40];
			p->si_em_phasebreak_event_px[i].apC = buf_of_recv_645_rep[45]<<16 | buf_of_recv_645_rep[44]<<8 | buf_of_recv_645_rep[43];
			p->si_em_phasebreak_event_px[i].rapC = buf_of_recv_645_rep[48]<<16 | buf_of_recv_645_rep[47]<<8 | buf_of_recv_645_rep[46];
			p->si_em_phasebreak_event_px[i].pfC = buf_of_recv_645_rep[50]<<8 | buf_of_recv_645_rep[49];
			p->si_em_phasebreak_event_px[i].ahnA = buf_of_recv_645_rep[54]<<24 | buf_of_recv_645_rep[53]<<16 | buf_of_recv_645_rep[52]<<8 | buf_of_recv_645_rep[51];
			p->si_em_phasebreak_event_px[i].ahnB = buf_of_recv_645_rep[58]<<24 | buf_of_recv_645_rep[57]<<16 | buf_of_recv_645_rep[56]<<8 | buf_of_recv_645_rep[55];
			p->si_em_phasebreak_event_px[i].ahnC = buf_of_recv_645_rep[62]<<24 | buf_of_recv_645_rep[61]<<16 | buf_of_recv_645_rep[60]<<8 | buf_of_recv_645_rep[59];
			clr_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
		}
		
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* A相功率因数, 返回2bytes数据 */
		err_em_frame = get_event_data_from_ammeter(meteraddr, AMM_EVENT_B_LOSE_CURRENT, i+1, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
		
		if (FRAME_E_OK != err_em_frame) {
			if (is_bit_set(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT)) {
				err_code = SIE_SECOND_GET_DATA_FAIL;
			} else {
				err_code = SIE_FAIL;
				set_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
			}

			sinkinfo_log(("get em pa power factor fail(%d)\n", err_em_frame));
			break;
		} else {
			rt_memcpy(p->si_em_curloss_event_px[i].start_time, (rt_uint32_t *)buf_of_recv_645_rep, sizeof(p->si_em_curloss_event_px[i].start_time));
			rt_memcpy(p->si_em_curloss_event_px[i].end_time, (rt_uint32_t *)&buf_of_recv_645_rep[6], sizeof(p->si_em_curloss_event_px[i].end_time));
			p->si_em_curloss_event_px[i].vA = buf_of_recv_645_rep[13]<<8 | buf_of_recv_645_rep[12];
			p->si_em_curloss_event_px[i].iA = buf_of_recv_645_rep[16]<<16 | buf_of_recv_645_rep[15]<<8 | buf_of_recv_645_rep[14];
			p->si_em_curloss_event_px[i].apA = buf_of_recv_645_rep[19]<<16 | buf_of_recv_645_rep[18]<<8 | buf_of_recv_645_rep[17];
			p->si_em_curloss_event_px[i].rapA = buf_of_recv_645_rep[22]<<16 | buf_of_recv_645_rep[21]<<8 | buf_of_recv_645_rep[20];
			p->si_em_curloss_event_px[i].pfA = buf_of_recv_645_rep[24]<<8 | buf_of_recv_645_rep[23];
			p->si_em_curloss_event_px[i].vB = buf_of_recv_645_rep[26]<<8 | buf_of_recv_645_rep[25];
			p->si_em_curloss_event_px[i].iB = buf_of_recv_645_rep[29]<<16 | buf_of_recv_645_rep[28]<<8 | buf_of_recv_645_rep[27];
			p->si_em_curloss_event_px[i].apB = buf_of_recv_645_rep[32]<<16 | buf_of_recv_645_rep[31]<<8 | buf_of_recv_645_rep[30];
			p->si_em_curloss_event_px[i].rapB = buf_of_recv_645_rep[35]<<16 | buf_of_recv_645_rep[34]<<8 | buf_of_recv_645_rep[33];
			p->si_em_curloss_event_px[i].pfB = buf_of_recv_645_rep[37]<<8 | buf_of_recv_645_rep[36];
			p->si_em_curloss_event_px[i].vC = buf_of_recv_645_rep[39]<<8 | buf_of_recv_645_rep[38];
			p->si_em_curloss_event_px[i].iC = buf_of_recv_645_rep[42]<<16 | buf_of_recv_645_rep[41]<<8 | buf_of_recv_645_rep[40];
			p->si_em_curloss_event_px[i].apC = buf_of_recv_645_rep[45]<<16 | buf_of_recv_645_rep[44]<<8 | buf_of_recv_645_rep[43];
			p->si_em_curloss_event_px[i].rapC = buf_of_recv_645_rep[48]<<16 | buf_of_recv_645_rep[47]<<8 | buf_of_recv_645_rep[46];
			p->si_em_curloss_event_px[i].pfC = buf_of_recv_645_rep[50]<<8 | buf_of_recv_645_rep[49];
			clr_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
		}
		
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* A相功率因数, 返回2bytes数据 */
		err_em_frame = get_event_data_from_ammeter(meteraddr, AMM_EVENT_B_OVER_CURRENT, i+1, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
		
		if (FRAME_E_OK != err_em_frame) {
			if (is_bit_set(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT)) {
				err_code = SIE_SECOND_GET_DATA_FAIL;
			} else {
				err_code = SIE_FAIL;
				set_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
			}

			sinkinfo_log(("get em pa power factor fail(%d)\n", err_em_frame));
			break;
		} else {
			rt_memcpy(p->si_em_curover_event_px[i].start_time, (rt_uint32_t *)buf_of_recv_645_rep, sizeof(p->si_em_curover_event_px[i].start_time));
			rt_memcpy(p->si_em_curover_event_px[i].end_time, (rt_uint32_t *)&buf_of_recv_645_rep[6], sizeof(p->si_em_curover_event_px[i].end_time));
			p->si_em_curover_event_px[i].vA = buf_of_recv_645_rep[13]<<8 | buf_of_recv_645_rep[12];
			p->si_em_curover_event_px[i].iA = buf_of_recv_645_rep[16]<<16 | buf_of_recv_645_rep[15]<<8 | buf_of_recv_645_rep[14];
			p->si_em_curover_event_px[i].apA = buf_of_recv_645_rep[19]<<16 | buf_of_recv_645_rep[18]<<8 | buf_of_recv_645_rep[17];
			p->si_em_curover_event_px[i].rapA = buf_of_recv_645_rep[22]<<16 | buf_of_recv_645_rep[21]<<8 | buf_of_recv_645_rep[20];
			p->si_em_curover_event_px[i].pfA = buf_of_recv_645_rep[24]<<8 | buf_of_recv_645_rep[23];
			p->si_em_curover_event_px[i].vB = buf_of_recv_645_rep[26]<<8 | buf_of_recv_645_rep[25];
			p->si_em_curover_event_px[i].iB = buf_of_recv_645_rep[29]<<16 | buf_of_recv_645_rep[28]<<8 | buf_of_recv_645_rep[27];
			p->si_em_curover_event_px[i].apB = buf_of_recv_645_rep[32]<<16 | buf_of_recv_645_rep[31]<<8 | buf_of_recv_645_rep[30];
			p->si_em_curover_event_px[i].rapB = buf_of_recv_645_rep[35]<<16 | buf_of_recv_645_rep[34]<<8 | buf_of_recv_645_rep[33];
			p->si_em_curover_event_px[i].pfB = buf_of_recv_645_rep[37]<<8 | buf_of_recv_645_rep[36];
			p->si_em_curover_event_px[i].vC = buf_of_recv_645_rep[39]<<8 | buf_of_recv_645_rep[38];
			p->si_em_curover_event_px[i].iC = buf_of_recv_645_rep[42]<<16 | buf_of_recv_645_rep[41]<<8 | buf_of_recv_645_rep[40];
			p->si_em_curover_event_px[i].apC = buf_of_recv_645_rep[45]<<16 | buf_of_recv_645_rep[44]<<8 | buf_of_recv_645_rep[43];
			p->si_em_curover_event_px[i].rapC = buf_of_recv_645_rep[48]<<16 | buf_of_recv_645_rep[47]<<8 | buf_of_recv_645_rep[46];
			p->si_em_curover_event_px[i].pfC = buf_of_recv_645_rep[50]<<8 | buf_of_recv_645_rep[49];
			clr_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
		}
		
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* A相功率因数, 返回2bytes数据 */
		err_em_frame = get_event_data_from_ammeter(meteraddr, AMM_EVENT_B_BROKEN_CURRENT, i+1, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
		
		if (FRAME_E_OK != err_em_frame) {
			if (is_bit_set(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT)) {
				err_code = SIE_SECOND_GET_DATA_FAIL;
			} else {
				err_code = SIE_FAIL;
				set_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
			}

			sinkinfo_log(("get em pa power factor fail(%d)\n", err_em_frame));
			break;
		} else {
			rt_memcpy(p->si_em_curbreak_event_px[i].start_time, (rt_uint32_t *)buf_of_recv_645_rep, sizeof(p->si_em_curbreak_event_px[i].start_time));
			rt_memcpy(p->si_em_curbreak_event_px[i].end_time, (rt_uint32_t *)&buf_of_recv_645_rep[6], sizeof(p->si_em_curbreak_event_px[i].end_time));
			p->si_em_curbreak_event_px[i].vA = buf_of_recv_645_rep[13]<<8 | buf_of_recv_645_rep[12];
			p->si_em_curbreak_event_px[i].iA = buf_of_recv_645_rep[16]<<16 | buf_of_recv_645_rep[15]<<8 | buf_of_recv_645_rep[14];
			p->si_em_curbreak_event_px[i].apA = buf_of_recv_645_rep[19]<<16 | buf_of_recv_645_rep[18]<<8 | buf_of_recv_645_rep[17];
			p->si_em_curbreak_event_px[i].rapA = buf_of_recv_645_rep[22]<<16 | buf_of_recv_645_rep[21]<<8 | buf_of_recv_645_rep[20];
			p->si_em_curbreak_event_px[i].pfA = buf_of_recv_645_rep[24]<<8 | buf_of_recv_645_rep[23];
			p->si_em_curbreak_event_px[i].vB = buf_of_recv_645_rep[26]<<8 | buf_of_recv_645_rep[25];
			p->si_em_curbreak_event_px[i].iB = buf_of_recv_645_rep[29]<<16 | buf_of_recv_645_rep[28]<<8 | buf_of_recv_645_rep[27];
			p->si_em_curbreak_event_px[i].apB = buf_of_recv_645_rep[32]<<16 | buf_of_recv_645_rep[31]<<8 | buf_of_recv_645_rep[30];
			p->si_em_curbreak_event_px[i].rapB = buf_of_recv_645_rep[35]<<16 | buf_of_recv_645_rep[34]<<8 | buf_of_recv_645_rep[33];
			p->si_em_curbreak_event_px[i].pfB = buf_of_recv_645_rep[37]<<8 | buf_of_recv_645_rep[36];
			p->si_em_curbreak_event_px[i].vC = buf_of_recv_645_rep[39]<<8 | buf_of_recv_645_rep[38];
			p->si_em_curbreak_event_px[i].iC = buf_of_recv_645_rep[42]<<16 | buf_of_recv_645_rep[41]<<8 | buf_of_recv_645_rep[40];
			p->si_em_curbreak_event_px[i].apC = buf_of_recv_645_rep[45]<<16 | buf_of_recv_645_rep[44]<<8 | buf_of_recv_645_rep[43];
			p->si_em_curbreak_event_px[i].rapC = buf_of_recv_645_rep[48]<<16 | buf_of_recv_645_rep[47]<<8 | buf_of_recv_645_rep[46];
			p->si_em_curbreak_event_px[i].pfC = buf_of_recv_645_rep[50]<<8 | buf_of_recv_645_rep[49];
			clr_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
		}

		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));
		}
#endif


	/* 获取电表中c相所有abc三相独立的参数, by 645 */
//	rt_memset(&p->si_em_ind_px, 0, sizeof(p->si_em_ind_px));
	copy_data_use_by_sinkinfo_buf(&p->si_em_ind_px, sizeof(p->si_em_ind_px),
			&sinkinfo_all_em[si_used_em_index.cur_em_index].si_em_ind_pc,
			sizeof(sinkinfo_all_em[0].si_em_ind_pc),
			__FUNCTION__, __LINE__);


	/* C相电压, 返回2bytes数据 */
	err_em_frame = get_data_from_em_xprotocol(AC_C_VOLTAGE,
			&p->si_em_ind_px.vx, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		p->si_em_ind_px.vx = INVLIDE_DATAL;
		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em pc voltage fail(%d)\n", err_em_frame));
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	/* C相电流, 返回2bytes数据 */
	err_em_frame = get_data_from_em_xprotocol(AC_C_CURRENT,
			&p->si_em_ind_px.ix, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		p->si_em_ind_px.ix = INVLIDE_DATAL;
		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em pc current fail(%d)\n", err_em_frame));
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	/* C相有功功率, 返回3bytes数据 */
	err_em_frame = get_data_from_em_xprotocol(AC_C_ACTIVE_POWER,
			&p->si_em_ind_px.apx, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		p->si_em_ind_px.apx = INVLIDE_DATAL;
		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em pc act power fail(%d)\n", err_em_frame));
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	/* C相无功功率, 返回2bytes数据 */
	err_em_frame = get_data_from_em_xprotocol(AC_C_REACTIVE_POWER,
			&p->si_em_ind_px.rapx, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		p->si_em_ind_px.rapx = INVLIDE_DATAL;
		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em pc react power fail(%d)\n", err_em_frame));
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	/* C相功率因数, 返回2bytes数据 */
	err_em_frame = get_data_from_em_xprotocol(AC_C_POWER_FACTOR,
			&p->si_em_ind_px.pfx, RS485_PORT_USED_BY_645);
	if (FRAME_E_OK != err_em_frame) {
//		p->si_em_ind_px.pfx = INVLIDE_DATAL;
		if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
			sinkinfo_log(("get em pc power factor fail(%d)\n", err_em_frame));
	}
	rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

	if(protocal == AP_PROTOCOL_645_2007){
		/* C相无功功率, 返回2bytes数据 */
		err_em_frame = get_data_from_em_xprotocol(AC_C_APPARENT_POWER,
				(u32_t *)&p->si_em_ind_px.appx, RS485_PORT_USED_BY_645);
		if (FRAME_E_OK != err_em_frame) {
//			p->si_em_ind_px.appx = INVLIDE_DATAL;
			if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
				sinkinfo_log(("get em pc react power fail(%d)\n", err_em_frame));
		}
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* C相功率因数, 返回2bytes数据 */
		err_em_frame = get_data_from_em_xprotocol(AC_C_VOLTAGE_DISTORTION,
				(u32_t *)&p->si_em_ind_px.vdx, RS485_PORT_USED_BY_645);
		if (FRAME_E_OK != err_em_frame) {
//			p->si_em_ind_px.vdx = INVLIDE_DATAL;
			if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
				sinkinfo_log(("get em pc power factor fail(%d)\n", err_em_frame));
		}
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* C相功率因数, 返回2bytes数据 */
		err_em_frame = get_data_from_em_xprotocol(AC_C_CURRENT_DISTORTION,
				(u32_t *)&p->si_em_ind_px.cdx, RS485_PORT_USED_BY_645);
		if (FRAME_E_OK != err_em_frame) {
//			p->si_em_ind_px.cdx = INVLIDE_DATAL;
			if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
				sinkinfo_log(("get em pc power factor fail(%d)\n", err_em_frame));
		}
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* C相正向有功总电能 */
		err_em_frame = get_data_from_em_xprotocol(AC_C_POSITIVE_ACTIVE_POWER,
				(u32_t *)&p->si_em_ind_px.taex, RS485_PORT_USED_BY_645);
		if (FRAME_E_OK != err_em_frame) {
//			p->si_em_ind_px.taex = INVLIDE_DATAL;
			if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
				sinkinfo_log(("get em phase C total active energy (%d)\n", err_em_frame));
		}
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));
		
		/* C相反向有功总电能 */
		err_em_frame = get_data_from_em_xprotocol(AC_C_REPOSITIVE_ACTIVE_POWER,
				(u32_t *)&p->si_em_ind_px.rtaex, RS485_PORT_USED_BY_645);
		if (FRAME_E_OK != err_em_frame) {
//			p->si_em_ind_px.rtaex = INVLIDE_DATAL;
			if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
				sinkinfo_log(("get em phase C totalreverse active energy (%d)\n", err_em_frame));
		}
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));
	}else if(protocal == AP_PROTOCOL_EDMI){
		/* C相无功功率, 返回2bytes数据 */
		err_em_frame = get_data_from_em_xprotocol(AC_C_APPARENT_POWER,
				(u32_t *)&p->si_em_ind_px.appx, RS485_PORT_USED_BY_645);
		if (FRAME_E_OK != err_em_frame) {
//			p->si_em_ind_px.appx = INVLIDE_DATAL;
			if (is_bit_set(sinkinfo_print_switch, SINKINFO_EM_GETDATA_FAIL_BIT))
				sinkinfo_log(("get em pc react power fail(%d)\n", err_em_frame));
		}
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));
		
		p->si_em_ind_px.vdx = INVLIDE_DATAL;
		p->si_em_ind_px.cdx = INVLIDE_DATAL;
		p->si_em_ind_px.taex = INVLIDE_DATAL;
		p->si_em_ind_px.rtaex = INVLIDE_DATAL;

	}else if(protocal == AP_PROTOCOL_645_1997 || protocal == AP_PROTOCOL_UNKNOWN){
		p->si_em_ind_px.appx = INVLIDE_DATAL;
		p->si_em_ind_px.vdx = INVLIDE_DATAL;
		p->si_em_ind_px.cdx = INVLIDE_DATAL;
		p->si_em_ind_px.taex = INVLIDE_DATAL;
		p->si_em_ind_px.rtaex = INVLIDE_DATAL;
	}
	p->si_em_ind_px.phx = INVLIDE_DATAL;
	p->si_em_ind_px.viphx = INVLIDE_DATAL;
	/* 电网频率, 07规约中没有区分abc三相的频率, 统一使用电网频率, 返回2bytes数据 */
	p->si_em_ind_px.hzx = sinkinfo_all_em[si_used_em_index.cur_em_index].si_em_ind_pa.hzx;

	copy_data_use_by_sinkinfo_buf(&sinkinfo_all_em[si_used_em_index.cur_em_index].si_em_ind_pc,
			sizeof(sinkinfo_all_em[0].si_em_ind_pc), &p->si_em_ind_px,
			sizeof(p->si_em_ind_px), __FUNCTION__, __LINE__);

#if 0 //EM_EVENT_SUPPERT /* 对电表事件的代码没有进行修正, mark by David */
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* A相功率因数, 返回2bytes数据 */
		for(i = 0; i<ELECTRIC_METER_EVENT_TIMES_MAX; i++){
		err_em_frame = get_event_data_from_ammeter(meteraddr, AMM_EVENT_C_LOSE_VOLTAGE, i+1, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
		
		if (FRAME_E_OK != err_em_frame) {
			if (is_bit_set(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT)) {
				err_code = SIE_SECOND_GET_DATA_FAIL;
			} else {
				err_code = SIE_FAIL;
				set_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
			}

			sinkinfo_log(("get em pa power factor fail(%d)\n", err_em_frame));
			break;
		} else {
			rt_memcpy(p->si_em_volloss_event_px[i].start_time, (rt_uint32_t *)buf_of_recv_645_rep, sizeof(p->si_em_volloss_event_px[i].start_time));
			rt_memcpy(p->si_em_volloss_event_px[i].end_time, (rt_uint32_t *)&buf_of_recv_645_rep[6], sizeof(p->si_em_volloss_event_px[i].end_time));
			p->si_em_volloss_event_px[i].vA = buf_of_recv_645_rep[13]<<8 | buf_of_recv_645_rep[12];
			p->si_em_volloss_event_px[i].iA = buf_of_recv_645_rep[16]<<16 | buf_of_recv_645_rep[15]<<8 | buf_of_recv_645_rep[14];
			p->si_em_volloss_event_px[i].apA = buf_of_recv_645_rep[19]<<16 | buf_of_recv_645_rep[18]<<8 | buf_of_recv_645_rep[17];
			p->si_em_volloss_event_px[i].rapA = buf_of_recv_645_rep[22]<<16 | buf_of_recv_645_rep[21]<<8 | buf_of_recv_645_rep[20];
			p->si_em_volloss_event_px[i].pfA = buf_of_recv_645_rep[24]<<8 | buf_of_recv_645_rep[23];
			p->si_em_volloss_event_px[i].vB = buf_of_recv_645_rep[26]<<8 | buf_of_recv_645_rep[25];
			p->si_em_volloss_event_px[i].iB = buf_of_recv_645_rep[29]<<16 | buf_of_recv_645_rep[28]<<8 | buf_of_recv_645_rep[27];
			p->si_em_volloss_event_px[i].apB = buf_of_recv_645_rep[32]<<16 | buf_of_recv_645_rep[31]<<8 | buf_of_recv_645_rep[30];
			p->si_em_volloss_event_px[i].rapB = buf_of_recv_645_rep[35]<<16 | buf_of_recv_645_rep[34]<<8 | buf_of_recv_645_rep[33];
			p->si_em_volloss_event_px[i].pfB = buf_of_recv_645_rep[37]<<8 | buf_of_recv_645_rep[36];
			p->si_em_volloss_event_px[i].vC = buf_of_recv_645_rep[39]<<8 | buf_of_recv_645_rep[38];
			p->si_em_volloss_event_px[i].iC = buf_of_recv_645_rep[42]<<16 | buf_of_recv_645_rep[41]<<8 | buf_of_recv_645_rep[40];
			p->si_em_volloss_event_px[i].apC = buf_of_recv_645_rep[45]<<16 | buf_of_recv_645_rep[44]<<8 | buf_of_recv_645_rep[43];
			p->si_em_volloss_event_px[i].rapC = buf_of_recv_645_rep[48]<<16 | buf_of_recv_645_rep[47]<<8 | buf_of_recv_645_rep[46];
			p->si_em_volloss_event_px[i].pfC = buf_of_recv_645_rep[50]<<8 | buf_of_recv_645_rep[49];
			p->si_em_volloss_event_px[i].ahnA = buf_of_recv_645_rep[54]<<24 | buf_of_recv_645_rep[53]<<16 | buf_of_recv_645_rep[52]<<8 | buf_of_recv_645_rep[51];
			p->si_em_volloss_event_px[i].ahnB = buf_of_recv_645_rep[58]<<24 | buf_of_recv_645_rep[57]<<16 | buf_of_recv_645_rep[56]<<8 | buf_of_recv_645_rep[55];
			p->si_em_volloss_event_px[i].ahnC = buf_of_recv_645_rep[62]<<24 | buf_of_recv_645_rep[61]<<16 | buf_of_recv_645_rep[60]<<8 | buf_of_recv_645_rep[59];
			clr_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
		}
		
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* A相功率因数, 返回2bytes数据 */
		err_em_frame = get_event_data_from_ammeter(meteraddr, AMM_EVENT_C_OVER_VOLTAGE, i+1, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
		
		if (FRAME_E_OK != err_em_frame) {
			if (is_bit_set(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT)) {
				err_code = SIE_SECOND_GET_DATA_FAIL;
			} else {
				err_code = SIE_FAIL;
				set_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
			}

			sinkinfo_log(("get em pa power factor fail(%d)\n", err_em_frame));
			break;
		} else {
			rt_memcpy(p->si_em_volover_event_px[i].start_time, (rt_uint32_t *)buf_of_recv_645_rep, sizeof(p->si_em_volover_event_px[i].start_time));
			rt_memcpy(p->si_em_volover_event_px[i].end_time, (rt_uint32_t *)&buf_of_recv_645_rep[6], sizeof(p->si_em_volover_event_px[i].end_time));
			p->si_em_volover_event_px[i].vA = buf_of_recv_645_rep[13]<<8 | buf_of_recv_645_rep[12];
			p->si_em_volover_event_px[i].iA = buf_of_recv_645_rep[16]<<16 | buf_of_recv_645_rep[15]<<8 | buf_of_recv_645_rep[14];
			p->si_em_volover_event_px[i].apA = buf_of_recv_645_rep[19]<<16 | buf_of_recv_645_rep[18]<<8 | buf_of_recv_645_rep[17];
			p->si_em_volover_event_px[i].rapA = buf_of_recv_645_rep[22]<<16 | buf_of_recv_645_rep[21]<<8 | buf_of_recv_645_rep[20];
			p->si_em_volover_event_px[i].pfA = buf_of_recv_645_rep[24]<<8 | buf_of_recv_645_rep[23];
			p->si_em_volover_event_px[i].vB = buf_of_recv_645_rep[26]<<8 | buf_of_recv_645_rep[25];
			p->si_em_volover_event_px[i].iB = buf_of_recv_645_rep[29]<<16 | buf_of_recv_645_rep[28]<<8 | buf_of_recv_645_rep[27];
			p->si_em_volover_event_px[i].apB = buf_of_recv_645_rep[32]<<16 | buf_of_recv_645_rep[31]<<8 | buf_of_recv_645_rep[30];
			p->si_em_volover_event_px[i].rapB = buf_of_recv_645_rep[35]<<16 | buf_of_recv_645_rep[34]<<8 | buf_of_recv_645_rep[33];
			p->si_em_volover_event_px[i].pfB = buf_of_recv_645_rep[37]<<8 | buf_of_recv_645_rep[36];
			p->si_em_volover_event_px[i].vC = buf_of_recv_645_rep[39]<<8 | buf_of_recv_645_rep[38];
			p->si_em_volover_event_px[i].iC = buf_of_recv_645_rep[42]<<16 | buf_of_recv_645_rep[41]<<8 | buf_of_recv_645_rep[40];
			p->si_em_volover_event_px[i].apC = buf_of_recv_645_rep[45]<<16 | buf_of_recv_645_rep[44]<<8 | buf_of_recv_645_rep[43];
			p->si_em_volover_event_px[i].rapC = buf_of_recv_645_rep[48]<<16 | buf_of_recv_645_rep[47]<<8 | buf_of_recv_645_rep[46];
			p->si_em_volover_event_px[i].pfC = buf_of_recv_645_rep[50]<<8 | buf_of_recv_645_rep[49];
			p->si_em_volover_event_px[i].ahnA = buf_of_recv_645_rep[54]<<24 | buf_of_recv_645_rep[53]<<16 | buf_of_recv_645_rep[52]<<8 | buf_of_recv_645_rep[51];
			p->si_em_volover_event_px[i].ahnB = buf_of_recv_645_rep[58]<<24 | buf_of_recv_645_rep[57]<<16 | buf_of_recv_645_rep[56]<<8 | buf_of_recv_645_rep[55];
			p->si_em_volover_event_px[i].ahnC = buf_of_recv_645_rep[62]<<24 | buf_of_recv_645_rep[61]<<16 | buf_of_recv_645_rep[60]<<8 | buf_of_recv_645_rep[59];
			clr_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
		}
		
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* A相功率因数, 返回2bytes数据 */
		err_em_frame = get_event_data_from_ammeter(meteraddr, AMM_EVENT_C_OWE_VOLTAGE, i+1, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
		
		if (FRAME_E_OK != err_em_frame) {
			if (is_bit_set(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT)) {
				err_code = SIE_SECOND_GET_DATA_FAIL;
			} else {
				err_code = SIE_FAIL;
				set_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
			}

			sinkinfo_log(("get em pa power factor fail(%d)\n", err_em_frame));
			break;
		} else {
			rt_memcpy(p->si_em_volunder_event_px[i].start_time, (rt_uint32_t *)buf_of_recv_645_rep, sizeof(p->si_em_volunder_event_px[i].start_time));
			rt_memcpy(p->si_em_volunder_event_px[i].end_time, (rt_uint32_t *)&buf_of_recv_645_rep[6], sizeof(p->si_em_volunder_event_px[i].end_time));
			p->si_em_volunder_event_px[i].vA = buf_of_recv_645_rep[13]<<8 | buf_of_recv_645_rep[12];
			p->si_em_volunder_event_px[i].iA = buf_of_recv_645_rep[16]<<16 | buf_of_recv_645_rep[15]<<8 | buf_of_recv_645_rep[14];
			p->si_em_volunder_event_px[i].apA = buf_of_recv_645_rep[19]<<16 | buf_of_recv_645_rep[18]<<8 | buf_of_recv_645_rep[17];
			p->si_em_volunder_event_px[i].rapA = buf_of_recv_645_rep[22]<<16 | buf_of_recv_645_rep[21]<<8 | buf_of_recv_645_rep[20];
			p->si_em_volunder_event_px[i].pfA = buf_of_recv_645_rep[24]<<8 | buf_of_recv_645_rep[23];
			p->si_em_volunder_event_px[i].vB = buf_of_recv_645_rep[26]<<8 | buf_of_recv_645_rep[25];
			p->si_em_volunder_event_px[i].iB = buf_of_recv_645_rep[29]<<16 | buf_of_recv_645_rep[28]<<8 | buf_of_recv_645_rep[27];
			p->si_em_volunder_event_px[i].apB = buf_of_recv_645_rep[32]<<16 | buf_of_recv_645_rep[31]<<8 | buf_of_recv_645_rep[30];
			p->si_em_volunder_event_px[i].rapB = buf_of_recv_645_rep[35]<<16 | buf_of_recv_645_rep[34]<<8 | buf_of_recv_645_rep[33];
			p->si_em_volunder_event_px[i].pfB = buf_of_recv_645_rep[37]<<8 | buf_of_recv_645_rep[36];
			p->si_em_volunder_event_px[i].vC = buf_of_recv_645_rep[39]<<8 | buf_of_recv_645_rep[38];
			p->si_em_volunder_event_px[i].iC = buf_of_recv_645_rep[42]<<16 | buf_of_recv_645_rep[41]<<8 | buf_of_recv_645_rep[40];
			p->si_em_volunder_event_px[i].apC = buf_of_recv_645_rep[45]<<16 | buf_of_recv_645_rep[44]<<8 | buf_of_recv_645_rep[43];
			p->si_em_volunder_event_px[i].rapC = buf_of_recv_645_rep[48]<<16 | buf_of_recv_645_rep[47]<<8 | buf_of_recv_645_rep[46];
			p->si_em_volunder_event_px[i].pfC = buf_of_recv_645_rep[50]<<8 | buf_of_recv_645_rep[49];
			p->si_em_volunder_event_px[i].ahnA = buf_of_recv_645_rep[54]<<24 | buf_of_recv_645_rep[53]<<16 | buf_of_recv_645_rep[52]<<8 | buf_of_recv_645_rep[51];
			p->si_em_volunder_event_px[i].ahnB = buf_of_recv_645_rep[58]<<24 | buf_of_recv_645_rep[57]<<16 | buf_of_recv_645_rep[56]<<8 | buf_of_recv_645_rep[55];
			p->si_em_volunder_event_px[i].ahnC = buf_of_recv_645_rep[62]<<24 | buf_of_recv_645_rep[61]<<16 | buf_of_recv_645_rep[60]<<8 | buf_of_recv_645_rep[59];
			clr_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
		}
		
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* A相功率因数, 返回2bytes数据 */
		err_em_frame = get_event_data_from_ammeter(meteraddr, AMM_EVENT_C_BROKEN_PHASE, i+1, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
		
		if (FRAME_E_OK != err_em_frame) {
			if (is_bit_set(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT)) {
				err_code = SIE_SECOND_GET_DATA_FAIL;
			} else {
				err_code = SIE_FAIL;
				set_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
			}

			sinkinfo_log(("get em pa power factor fail(%d)\n", err_em_frame));
			break;
		} else {
			rt_memcpy(p->si_em_phasebreak_event_px[i].start_time, (rt_uint32_t *)buf_of_recv_645_rep, sizeof(p->si_em_phasebreak_event_px[i].start_time));
			rt_memcpy(p->si_em_phasebreak_event_px[i].end_time, (rt_uint32_t *)&buf_of_recv_645_rep[6], sizeof(p->si_em_phasebreak_event_px[i].end_time));
			p->si_em_phasebreak_event_px[i].vA = buf_of_recv_645_rep[13]<<8 | buf_of_recv_645_rep[12];
			p->si_em_phasebreak_event_px[i].iA = buf_of_recv_645_rep[16]<<16 | buf_of_recv_645_rep[15]<<8 | buf_of_recv_645_rep[14];
			p->si_em_phasebreak_event_px[i].apA = buf_of_recv_645_rep[19]<<16 | buf_of_recv_645_rep[18]<<8 | buf_of_recv_645_rep[17];
			p->si_em_phasebreak_event_px[i].rapA = buf_of_recv_645_rep[22]<<16 | buf_of_recv_645_rep[21]<<8 | buf_of_recv_645_rep[20];
			p->si_em_phasebreak_event_px[i].pfA = buf_of_recv_645_rep[24]<<8 | buf_of_recv_645_rep[23];
			p->si_em_phasebreak_event_px[i].vB = buf_of_recv_645_rep[26]<<8 | buf_of_recv_645_rep[25];
			p->si_em_phasebreak_event_px[i].iB = buf_of_recv_645_rep[29]<<16 | buf_of_recv_645_rep[28]<<8 | buf_of_recv_645_rep[27];
			p->si_em_phasebreak_event_px[i].apB = buf_of_recv_645_rep[32]<<16 | buf_of_recv_645_rep[31]<<8 | buf_of_recv_645_rep[30];
			p->si_em_phasebreak_event_px[i].rapB = buf_of_recv_645_rep[35]<<16 | buf_of_recv_645_rep[34]<<8 | buf_of_recv_645_rep[33];
			p->si_em_phasebreak_event_px[i].pfB = buf_of_recv_645_rep[37]<<8 | buf_of_recv_645_rep[36];
			p->si_em_phasebreak_event_px[i].vC = buf_of_recv_645_rep[39]<<8 | buf_of_recv_645_rep[38];
			p->si_em_phasebreak_event_px[i].iC = buf_of_recv_645_rep[42]<<16 | buf_of_recv_645_rep[41]<<8 | buf_of_recv_645_rep[40];
			p->si_em_phasebreak_event_px[i].apC = buf_of_recv_645_rep[45]<<16 | buf_of_recv_645_rep[44]<<8 | buf_of_recv_645_rep[43];
			p->si_em_phasebreak_event_px[i].rapC = buf_of_recv_645_rep[48]<<16 | buf_of_recv_645_rep[47]<<8 | buf_of_recv_645_rep[46];
			p->si_em_phasebreak_event_px[i].pfC = buf_of_recv_645_rep[50]<<8 | buf_of_recv_645_rep[49];
			p->si_em_phasebreak_event_px[i].ahnA = buf_of_recv_645_rep[54]<<24 | buf_of_recv_645_rep[53]<<16 | buf_of_recv_645_rep[52]<<8 | buf_of_recv_645_rep[51];
			p->si_em_phasebreak_event_px[i].ahnB = buf_of_recv_645_rep[58]<<24 | buf_of_recv_645_rep[57]<<16 | buf_of_recv_645_rep[56]<<8 | buf_of_recv_645_rep[55];
			p->si_em_phasebreak_event_px[i].ahnC = buf_of_recv_645_rep[62]<<24 | buf_of_recv_645_rep[61]<<16 | buf_of_recv_645_rep[60]<<8 | buf_of_recv_645_rep[59];
			clr_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
		}
		
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* A相功率因数, 返回2bytes数据 */
		err_em_frame = get_event_data_from_ammeter(meteraddr, AMM_EVENT_C_LOSE_CURRENT, i+1, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
		
		if (FRAME_E_OK != err_em_frame) {
			if (is_bit_set(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT)) {
				err_code = SIE_SECOND_GET_DATA_FAIL;
			} else {
				err_code = SIE_FAIL;
				set_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
			}

			sinkinfo_log(("get em pa power factor fail(%d)\n", err_em_frame));
			break;
		} else {
			rt_memcpy(p->si_em_curloss_event_px[i].start_time, (rt_uint32_t *)buf_of_recv_645_rep, sizeof(p->si_em_curloss_event_px[i].start_time));
			rt_memcpy(p->si_em_curloss_event_px[i].end_time, (rt_uint32_t *)&buf_of_recv_645_rep[6], sizeof(p->si_em_curloss_event_px[i].end_time));
			p->si_em_curloss_event_px[i].vA = buf_of_recv_645_rep[13]<<8 | buf_of_recv_645_rep[12];
			p->si_em_curloss_event_px[i].iA = buf_of_recv_645_rep[16]<<16 | buf_of_recv_645_rep[15]<<8 | buf_of_recv_645_rep[14];
			p->si_em_curloss_event_px[i].apA = buf_of_recv_645_rep[19]<<16 | buf_of_recv_645_rep[18]<<8 | buf_of_recv_645_rep[17];
			p->si_em_curloss_event_px[i].rapA = buf_of_recv_645_rep[22]<<16 | buf_of_recv_645_rep[21]<<8 | buf_of_recv_645_rep[20];
			p->si_em_curloss_event_px[i].pfA = buf_of_recv_645_rep[24]<<8 | buf_of_recv_645_rep[23];
			p->si_em_curloss_event_px[i].vB = buf_of_recv_645_rep[26]<<8 | buf_of_recv_645_rep[25];
			p->si_em_curloss_event_px[i].iB = buf_of_recv_645_rep[29]<<16 | buf_of_recv_645_rep[28]<<8 | buf_of_recv_645_rep[27];
			p->si_em_curloss_event_px[i].apB = buf_of_recv_645_rep[32]<<16 | buf_of_recv_645_rep[31]<<8 | buf_of_recv_645_rep[30];
			p->si_em_curloss_event_px[i].rapB = buf_of_recv_645_rep[35]<<16 | buf_of_recv_645_rep[34]<<8 | buf_of_recv_645_rep[33];
			p->si_em_curloss_event_px[i].pfB = buf_of_recv_645_rep[37]<<8 | buf_of_recv_645_rep[36];
			p->si_em_curloss_event_px[i].vC = buf_of_recv_645_rep[39]<<8 | buf_of_recv_645_rep[38];
			p->si_em_curloss_event_px[i].iC = buf_of_recv_645_rep[42]<<16 | buf_of_recv_645_rep[41]<<8 | buf_of_recv_645_rep[40];
			p->si_em_curloss_event_px[i].apC = buf_of_recv_645_rep[45]<<16 | buf_of_recv_645_rep[44]<<8 | buf_of_recv_645_rep[43];
			p->si_em_curloss_event_px[i].rapC = buf_of_recv_645_rep[48]<<16 | buf_of_recv_645_rep[47]<<8 | buf_of_recv_645_rep[46];
			p->si_em_curloss_event_px[i].pfC = buf_of_recv_645_rep[50]<<8 | buf_of_recv_645_rep[49];
			clr_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
		}
		
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* A相功率因数, 返回2bytes数据 */
		err_em_frame = get_event_data_from_ammeter(meteraddr, AMM_EVENT_C_OVER_CURRENT, i+1, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
		
		if (FRAME_E_OK != err_em_frame) {
			if (is_bit_set(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT)) {
				err_code = SIE_SECOND_GET_DATA_FAIL;
			} else {
				err_code = SIE_FAIL;
				set_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
			}

			sinkinfo_log(("get em pa power factor fail(%d)\n", err_em_frame));
			break;
		} else {
			rt_memcpy(p->si_em_curover_event_px[i].start_time, (rt_uint32_t *)buf_of_recv_645_rep, sizeof(p->si_em_curover_event_px[i].start_time));
			rt_memcpy(p->si_em_curover_event_px[i].end_time, (rt_uint32_t *)&buf_of_recv_645_rep[6], sizeof(p->si_em_curover_event_px[i].end_time));
			p->si_em_curover_event_px[i].vA = buf_of_recv_645_rep[13]<<8 | buf_of_recv_645_rep[12];
			p->si_em_curover_event_px[i].iA = buf_of_recv_645_rep[16]<<16 | buf_of_recv_645_rep[15]<<8 | buf_of_recv_645_rep[14];
			p->si_em_curover_event_px[i].apA = buf_of_recv_645_rep[19]<<16 | buf_of_recv_645_rep[18]<<8 | buf_of_recv_645_rep[17];
			p->si_em_curover_event_px[i].rapA = buf_of_recv_645_rep[22]<<16 | buf_of_recv_645_rep[21]<<8 | buf_of_recv_645_rep[20];
			p->si_em_curover_event_px[i].pfA = buf_of_recv_645_rep[24]<<8 | buf_of_recv_645_rep[23];
			p->si_em_curover_event_px[i].vB = buf_of_recv_645_rep[26]<<8 | buf_of_recv_645_rep[25];
			p->si_em_curover_event_px[i].iB = buf_of_recv_645_rep[29]<<16 | buf_of_recv_645_rep[28]<<8 | buf_of_recv_645_rep[27];
			p->si_em_curover_event_px[i].apB = buf_of_recv_645_rep[32]<<16 | buf_of_recv_645_rep[31]<<8 | buf_of_recv_645_rep[30];
			p->si_em_curover_event_px[i].rapB = buf_of_recv_645_rep[35]<<16 | buf_of_recv_645_rep[34]<<8 | buf_of_recv_645_rep[33];
			p->si_em_curover_event_px[i].pfB = buf_of_recv_645_rep[37]<<8 | buf_of_recv_645_rep[36];
			p->si_em_curover_event_px[i].vC = buf_of_recv_645_rep[39]<<8 | buf_of_recv_645_rep[38];
			p->si_em_curover_event_px[i].iC = buf_of_recv_645_rep[42]<<16 | buf_of_recv_645_rep[41]<<8 | buf_of_recv_645_rep[40];
			p->si_em_curover_event_px[i].apC = buf_of_recv_645_rep[45]<<16 | buf_of_recv_645_rep[44]<<8 | buf_of_recv_645_rep[43];
			p->si_em_curover_event_px[i].rapC = buf_of_recv_645_rep[48]<<16 | buf_of_recv_645_rep[47]<<8 | buf_of_recv_645_rep[46];
			p->si_em_curover_event_px[i].pfC = buf_of_recv_645_rep[50]<<8 | buf_of_recv_645_rep[49];
			clr_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
		}
		
		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));

		/* A相功率因数, 返回2bytes数据 */
		err_em_frame = get_event_data_from_ammeter(meteraddr, AMM_EVENT_C_BROKEN_CURRENT, i+1, buf_of_recv_645_rep, &data_len, RS485_PORT_USED_BY_645);
		
		if (FRAME_E_OK != err_em_frame) {
			if (is_bit_set(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT)) {
				err_code = SIE_SECOND_GET_DATA_FAIL;
			} else {
				err_code = SIE_FAIL;
				set_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
			}

			sinkinfo_log(("get em pa power factor fail(%d)\n", err_em_frame));
			break;
		} else {
			rt_memcpy(p->si_em_curbreak_event_px[i].start_time, (rt_uint32_t *)buf_of_recv_645_rep, sizeof(p->si_em_curbreak_event_px[i].start_time));
			rt_memcpy(p->si_em_curbreak_event_px[i].end_time, (rt_uint32_t *)&buf_of_recv_645_rep[6], sizeof(p->si_em_curbreak_event_px[i].end_time));
			p->si_em_curbreak_event_px[i].vA = buf_of_recv_645_rep[13]<<8 | buf_of_recv_645_rep[12];
			p->si_em_curbreak_event_px[i].iA = buf_of_recv_645_rep[16]<<16 | buf_of_recv_645_rep[15]<<8 | buf_of_recv_645_rep[14];
			p->si_em_curbreak_event_px[i].apA = buf_of_recv_645_rep[19]<<16 | buf_of_recv_645_rep[18]<<8 | buf_of_recv_645_rep[17];
			p->si_em_curbreak_event_px[i].rapA = buf_of_recv_645_rep[22]<<16 | buf_of_recv_645_rep[21]<<8 | buf_of_recv_645_rep[20];
			p->si_em_curbreak_event_px[i].pfA = buf_of_recv_645_rep[24]<<8 | buf_of_recv_645_rep[23];
			p->si_em_curbreak_event_px[i].vB = buf_of_recv_645_rep[26]<<8 | buf_of_recv_645_rep[25];
			p->si_em_curbreak_event_px[i].iB = buf_of_recv_645_rep[29]<<16 | buf_of_recv_645_rep[28]<<8 | buf_of_recv_645_rep[27];
			p->si_em_curbreak_event_px[i].apB = buf_of_recv_645_rep[32]<<16 | buf_of_recv_645_rep[31]<<8 | buf_of_recv_645_rep[30];
			p->si_em_curbreak_event_px[i].rapB = buf_of_recv_645_rep[35]<<16 | buf_of_recv_645_rep[34]<<8 | buf_of_recv_645_rep[33];
			p->si_em_curbreak_event_px[i].pfB = buf_of_recv_645_rep[37]<<8 | buf_of_recv_645_rep[36];
			p->si_em_curbreak_event_px[i].vC = buf_of_recv_645_rep[39]<<8 | buf_of_recv_645_rep[38];
			p->si_em_curbreak_event_px[i].iC = buf_of_recv_645_rep[42]<<16 | buf_of_recv_645_rep[41]<<8 | buf_of_recv_645_rep[40];
			p->si_em_curbreak_event_px[i].apC = buf_of_recv_645_rep[45]<<16 | buf_of_recv_645_rep[44]<<8 | buf_of_recv_645_rep[43];
			p->si_em_curbreak_event_px[i].rapC = buf_of_recv_645_rep[48]<<16 | buf_of_recv_645_rep[47]<<8 | buf_of_recv_645_rep[46];
			p->si_em_curbreak_event_px[i].pfC = buf_of_recv_645_rep[50]<<8 | buf_of_recv_645_rep[49];
			clr_bit(sink_em_645_info_fail_bit_flag, GET_PA_FACTOR_FAIL_BIT);
		}

		rt_thread_delay(get_ticks_of_ms(MIN_TIME_GAP_OF_TWO_645CMD));
		}
#endif

#if 0
	if (protocal == AP_PROTOCOL_645_2007) {
		do_get_em_data_harmonic(p, meteraddr);
	}
	do_get_em_data_copper_iron(p, protocal, meteraddr);
#endif
	do_get_em_data_max_demand(p,  protocal,  meteraddr);
	do_get_em_data_timing_freeze(p,  protocal,  meteraddr);
	do_get_em_data_momentary_freeze(p,  protocal, meteraddr);
	reported_ammeter_happened_event(meteraddr, RS485_PORT_USED_BY_645);

	return;
}
#endif

/*
 *
 * !!NOTE: 该函数不可重入
 * */
static enum frame_error_e get_data_from_em_xprotocol(enum ammeter_cmd_e cmd, u32_t *bcd_4bytes, enum ammeter_uart_e port_485)
{
	enum frame_error_e err_em_frame;
	struct electric_meter_reg_info_st amm_sn;

	rt_uint32_t data_len = 0;
	rt_uint8_t addr[16] = {'\0'};
	rt_uint8_t *data = NULL;

	if (NULL==bcd_4bytes) {
		printf_syn("func:%s(), param is NULL\n", __FUNCTION__);
		return FRAME_E_ERROR;
	}

	if (SUCC == get_em_reg_info(&amm_sn)) {
		rt_memcpy(addr, amm_sn.em_sn[si_used_em_index.cur_em_index], DEV_SN_MODE_LEN);

//		del_multi_zero_in_str((char *)addr, sizeof(addr));

//		printf_syn("%s(), sn:%s, addr:%s\n", __FUNCTION__, amm_sn.em_sn[si_used_em_index.cur_em_index],
//				addr);
	} else {
		printf_syn("%s(), read one ammeter addr data tbl fail\n", __FUNCTION__);
	}

	data = buf_of_recv_645_rep;
	//rt_memcpy(addr, broadcast_addr_645, sizeof(addr));
	err_em_frame = get_power_data_from_ammeter(addr, cmd, data, &data_len, port_485);

#if 0
	printf_syn("cmd:0x%x, recv_data len:%d, recv data:0x%02x, 0x%02x, 0x%02x, 0x%02x\n", cmd, data_len,
			data[0], data[1], data[2], data[3]);
#else
	sinki_debug_body(("cmd:0x%x, recv_data len:%d, recv data:0x%02x, 0x%02x, 0x%02x, 0x%02x\n", cmd, data_len,
			data[0], data[1], data[2], data[3]));
#endif

	if (FRAME_E_OK == err_em_frame) {
#if 0
		if(data_len == 4) {
			*bcd_4bytes = data[0]<<24 | data[1]<<16 | data[2]<<8 | data[3];
		}else if(data_len == 3){
			*bcd_4bytes = data[0]<<24 | data[1]<<16 | data[2]<<8;
		}else if(data_len == 2){
			*bcd_4bytes = data[0]<<24 | data[1]<<16;
		}else if(data_len == 1){
			*bcd_4bytes = data[0]<<24;
#else
		if(data_len == 4) {
			*bcd_4bytes = data[0] | data[1]<<8 | data[2]<<16 | data[3]<<24;
		}else if(data_len == 3){
			*bcd_4bytes = data[0] | data[1]<<8 | data[2]<<16;
		}else if(data_len == 2){
			*bcd_4bytes = data[0] | data[1]<<8;
		}else if(data_len == 1){
			*bcd_4bytes = data[0];
#endif
		}else if((data_len > 4) && (data_len <= sizeof(buf_of_recv_645_rep))){
			bcd_4bytes = (u32_t *)data;

		}else{
			sinkinfo_log(("%s() recv data len err(%d)\n", __FUNCTION__, data_len));
			err_em_frame = FRAME_E_ERROR;
		}
	} else {
		sinkinfo_log(("%s() fail(%d)\n", __FUNCTION__, err_em_frame));
	}

	return err_em_frame;
}

#if 0
/*
 * !!NOTE: 该函数不可重入
 * */
static enum em_frame_state_e get_em_sn_from_645(char *em_sn, enum ammeter_uart_e port_485)
{
	enum em_frame_state_e err_em_frame;
	struct frame_format_param send_data;
	struct frame_format_param recv_data;
	int i;
	unsigned int temp;

	/* 填充发送命令结构体 */
	rt_memcpy(send_data.addr, broadcast_addr_645, sizeof(send_data.addr));
	send_data.ctrl		= F645CMD_READ_DATA;
	send_data.data_flag	= AC_GET_AMMETR_ADDR;
	send_data.data		= RT_NULL;
	send_data.data_len	= 0;

	/* 填充接收结构体 */
	rt_memset(&recv_data, 0, sizeof(recv_data));
	rt_memset(buf_of_recv_645_rep, 0, sizeof(buf_of_recv_645_rep));
	recv_data.data = buf_of_recv_645_rep;

	err_em_frame = read_ammeter_data(&send_data, &recv_data, port_485);
#if 0
	printf_syn("data-flag:0x%x, recv_data len:%d, recv data:0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\n", send_data.data_flag, recv_data.data_len,
			recv_data.data[0], recv_data.data[1], recv_data.data[2], recv_data.data[3], recv_data.data[4], recv_data.data[5]);

//	printf_syn("addr:0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\n",
//			recv_data.addr[0], recv_data.addr[1], recv_data.addr[2], recv_data.addr[3], recv_data.addr[4], recv_data.addr[5]);
#else
	sinki_debug_body(("data-flag:0x%x, recv_data len:%d, recv data:0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\n", send_data.data_flag, recv_data.data_len,
			recv_data.data[0], recv_data.data[1], recv_data.data[2], recv_data.data[3], recv_data.data[4], recv_data.data[5]));
#endif
	if (FRAME_E_OK == err_em_frame && 6==recv_data.data_len) {
		for (i=0; i<6; ++i) {
			temp = recv_data.data[i];
			if (0 != temp) {
				*em_sn++ = (temp >> 4)  + '0';
				*em_sn++ = (temp &  0xf) + '0';
			}
		}
	} else {
		sinkinfo_log(("%s() fail(%d), recv data len:%d\n", __FUNCTION__, err_em_frame, recv_data.data_len));
	}

	return err_em_frame;
}
#endif

#if USE_OLD_645_CODE && !TEST_EM_PROTOC
static enum ammeter_protocal_e get_current_em_protocol(int cur_em_index)
{
	if (cur_em_index>=0 && cur_em_index<NUM_OF_COLLECT_EM_MAX)
		return register_em_info.em_proto[cur_em_index];
	else
		return AP_PROTOCOL_UNKNOWN;
}
#endif

void check_eenergy_timeout(int is_act_ee)
{
/*
 * 当前通道对应的电表SN
	int index;

	index = si_get_cur_em_index();
	register_em_info.registered_em_sn[index];
*/
	if (is_act_ee) {
		/* 测量有功脉冲周期时超时 */
    	trap_send(si_used_em_index.cur_em_index,SNMP_GENTRAP_ENTERPRISESPC, E_ALR_EM_ACT_PULSE_TIME_OUT, E_ALR_EM_ACT_PULSE_TIME_OUT, SDT_ELECTRIC_METER);
		rt_thread_delay(get_ticks_of_ms(50));
		printf_syn("active power pulse time out trap send success\n");
	} else {
		/* 测量无功脉冲周期时超时 */
		trap_send(si_used_em_index.cur_em_index,SNMP_GENTRAP_ENTERPRISESPC, E_ALR_EM_REACT_PULSE_TIME_OUT, E_ALR_EM_REACT_PULSE_TIME_OUT, SDT_ELECTRIC_METER);
		rt_thread_delay(get_ticks_of_ms(50));
		printf_syn("reactive power pulse time out trap send success\n");
	}

}
