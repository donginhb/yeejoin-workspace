/*
 * info_tran.c
 *
 * 2013-03-03,  creat by David, zhaoshaowei@yeejoin.com
 *
 * 该信息传输实现方法，要考虑减少传输字节数，以减轻信号恢复cpu的负荷。
 * 每个字节的高4bits作为信息类型标识, 低4bits用来传输数据
 *     高4bits全0时表示与上一个字节的数据为同一组, 数据的结束有不同信息类型来表示, 用状态机来实现
 */
#include <info_tran.h>
#include <phasex.h>
#include <rtdef.h>
#include <rtthread.h>
#include <zvd_gui_app.h>
#include <misc_lib.h>
#include <string.h>
#include <board.h>
#include <rtgui/widgets/window.h>

#define CELSIUS_STR "℃"

#define is_px_toolow(val)		((val) & (1<<(3+4)))
#define is_px_tooheigh(val)		((val) & (1<<(2+4)))
#define is_px_lost_phase(val)	((val) & (1<<(1+4)))
//#define is_px_no_output(val)	((val) & (1<<(1+4)))

#define is_colletion_end_power_down(val)	((val) & (1<<(3+4)))
#define is_restore_end_switch2pt(val)		((val) & (1<<(3+4)))
#define is_restore_end_recv_fibre_data(val)	((val) & (1<<(2+4)))

#define get_16bits_val(b0, b1) ((b0)<<8 | (b1))
#define get_24bits_val(b0, b1, b2) ((b0)<<16 | (b1)<<8 | (b2))

#define info_tran_debug(x) //printf_syn x


enum infos_analysis_state {
	IAS_IDEL			= 0, /* 初始状态 */
	IAS_WAIT_DATA       = 1, /* 等待数据 */
	IAS_RDATA_OVER      = 2,
};

struct info_tran_data_st {
	unsigned char data[DEV_SN_LEN_MAX]; /* 大小由最大数据长度决定 */
	int data_len; /* 长度是data_len个4 bits */
	int ind;
};

struct mf5x_temper_adc_pair_st {
	rt_int16_t temper;
	rt_uint16_t adc_val;
};

unsigned char info_tran_buf[64];
unsigned char info_tx_buf[64];
struct info_tran_data_st info_tran_data;

#define MF5X_TABLE_ENUM_NUM 27
#define mf5x_table_3v3 mf58_table_3v3
#define mf5x_table_5v0 mf58_table_5v0
#if 0
const struct mf5x_temper_adc_pair_st mf52_table[MF5X_TABLE_ENUM_NUM] = {
	{-30,	3881},
	{-25,	3809},
	{-20,	3718},
	{-15,	3608},
	{-10,	3475},
	{-5,	3319},
	{0,	3719},
	{5,	2944},
	{10,	2730},
	{15,	2505},
	{20,	2275},
	{25,	2047},
	{30,	1826},
	{35,	1615},
	{40,	1420},
	{45,	1242},
	{50,	1081},
	{55,	938},
	{60,	812},
	{65,	702},
	{70,	607},
	{75,	525},
	{80,	455},
	{85,	394},
	{90,	341},
	{95,	297},
	{100,	258},
};
#else
const struct mf5x_temper_adc_pair_st mf58_table_3v3[MF5X_TABLE_ENUM_NUM] = {
	{-30,	3873},
	{-25,	3802},
	{-20,	3712},
	{-15,	3601},
	{-10,	3469},
	{-5,	3313},
	{0,	3135},
	{5,	2938},
	{10,	2724},
	{15,	2500},
	{20,	2273},
	{25,	2047},
	{30,	1826},
	{35,	1616},
	{40,	1421},
	{45,	1242},
	{50,	1081},
	{55,	937 },
	{60,	811 },
	{65,	701 },
	{70,	606 },
	{75,	526 },
	{80,	457 },
	{85,	397 },
	{90,	346 },
	{95,	301 },
	{100,	263 },
};

const struct mf5x_temper_adc_pair_st mf58_table_5v0[MF5X_TABLE_ENUM_NUM] = {
	{-30,	5868},
	{-25,	5760},
	{-20,	5624},
	{-15,	5457},
	{-10,	5256},
	{-5,	5020},
	{0,	4751},
	{5,	4452},
	{10,	4128},
	{15,	3789},
	{20,	3444},
	{25,	3102},
	{30,	2767},
	{35,	2449},
	{40,	2153},
	{45,	1882},
	{50,	1638},
	{55,	1420},
	{60,	1229},
	{65,	1063},
	{70,	919 },
	{75,	798 },
	{80,	692 },
	{85,	602 },
	{90,	524 },
	{95,	456 },
	{100,	398 },
};
#endif

#define IsUpper(c)	(((c)>='A')&&((c)<='Z'))
#define IsLower(c)	(((c)>='a')&&((c)<='z'))
#define IsDigit(c)	(((c)>='0')&&((c)<='9'))

#define is_valid_devsn_char(c)  (IsUpper(c) || IsLower(c) || IsDigit(c) || '-'==(c))


/*
 * bit 0:采集端是否掉电
 * bit 1:恢复端是否切换至pt电缆.
 * bit 2:恢复端是否接收到光纤数据
 */
volatile unsigned long sys_misc_info;


/*
 * byte3:0x00
 * byte2:主版本号
 * byte1:次版本号
 * byte0:修订号
 */
unsigned long se_version;
unsigned long re_version;

volatile int is_px_bin_info_modifing;

static char avg_val_str[AVI_RE_END][16];
static char sys_temper_str[STI_XE_T_END][16];

static enum sys_use_channel_e cur_use_channel;

unsigned char tx_dev_sn[DEV_SN_LEN_MAX+1];	/* 设备序列号 */

static int get_info_data_len(int m_id, int s_id);
static int proc_recv_info(int id);
static int convert_box_adc2temper(int vol, int adc, char *str);
static int convert_avg_val2effective_val(int adc, char *str);
static void colletion_end_power_down_proc(int is_poweroff);
static void rx_overload_proc(int cnt);
static void write_tx_poweroff_info2bkp(void);
static int check_match_sn(char *str);
static int check_software_match(char *ver);

extern int creat_devsn(char *str, int len, int tx_rx_e);



void info_tran_stream_analysis(rt_device_t dev)
{
	static int info_id;
	static int revc_state = IAS_IDEL;
	int cnt, index, temp;

	index = 0;
	cnt   = dev->read(dev, 0, info_tran_buf, sizeof(info_tran_buf));
	while (0 != cnt--) {
		temp = info_tran_buf[index++];
#if 0
		info_tran_debug(("index:%d, data:0x%x\n", index-1, temp));
#endif
		switch (revc_state) {
		case IAS_IDEL:
			info_id = temp;
			if (0 != (info_id & 0xf0)) {
				info_tran_data.data_len = get_info_data_len((info_id & 0xf0)>>4, info_id & 0x0f);
				if (0 != info_tran_data.data_len) {
					/* 由于每个字节是4位有效数据，所以实际字节长度是除以2 */
					temp = info_tran_data.data_len>>1;
					if ( temp <= sizeof(info_tran_data.data)) {
						rt_memset(info_tran_data.data, 0, sizeof(info_tran_data.data));
						revc_state		= IAS_WAIT_DATA;
						info_tran_data.ind 	= 0;
					} else {
						rt_kprintf("error!! func:%s, data buf too small\n", __FUNCTION__);
					}
				}
			}
			break;

		case IAS_WAIT_DATA:
			if (0 == (temp&0xf0)) {
				if (0 == (info_tran_data.ind&0x01)) {
					info_tran_data.data[info_tran_data.ind>>1] |= temp<<4;
				} else {
					info_tran_data.data[info_tran_data.ind>>1] |= temp;
				}

				++info_tran_data.ind;
				if (info_tran_data.ind >= info_tran_data.data_len) {
					/* 数据接收完毕 */
					proc_recv_info(info_id);
					revc_state = IAS_IDEL;
				}
			} else {
				revc_state = IAS_IDEL;
				rt_kprintf("func:%s, data error, id:0x%x \n", __FUNCTION__, info_id);
			}
			break;

		default:
			break;
		}
	}

	return;
}

/*
 * 返回的长度是x个4 bits
 */
static int get_info_data_len(int m_id, int s_id)
{
	int len;

	len = 0;
	switch (m_id) {
	case ITID_TXPA_INFO:
	case ITID_TXPB_INFO:
	case ITID_TXPC_INFO:
	case ITID_RXPA_INFO:
	case ITID_RXPB_INFO:
	case ITID_RXPC_INFO:
	case ITID_RXE_ACK  :
		if ((PII_BINARY_INFO == s_id) || (TRX_SWITCH2PT == s_id))
			len = 1;
		else if (PII_AVG_VAL == s_id)
			len = 4;
		break;

	case ITID_TXE_INFO :
		if (TRXII_DEV_SN == s_id) {
			len = DEV_SN_MODE_LEN * 2;
			break;
		}
		/* no break; */
	case ITID_RXE_INFO :
		if (TRXII_BINARY_INFO==s_id || TRXII_USE_FIBER_NO==s_id)
			len = 1;
		else if (TRXII_CPU_TEMP==s_id || TRXII_BOX_TEMP==s_id)
			len = 4;
		else if (TRXII_VERSION == s_id)
			len = 6;
		else if (TRXII_RX_CONTINUE_OVERLOAD_CNT == s_id)
			len = 1;
		break;

	default:
		break;
	}

	return len;
}



static int proc_recv_info(int id)
{
	int m_id, s_id;
	int is_update_se_table, is_update_re_table, is_update_sys_table; //, is_not_match_sn;
	int temp, temper;
	char str[48];

	is_update_se_table  = 0;
	is_update_re_table  = 0;
	is_update_sys_table = 0;
//	is_not_match_sn = 0;
	str[0] = '\0';

	m_id = (id & 0xf0)>>4;
	s_id = id & 0x0f;

	if (0x11==id || 0x21==id || 0x31==id)
		info_tran_debug(("id:0x%x, data[0:3]:0x%x, 0x%x, 0x%x, 0x%x\n", id, info_tran_data.data[0],
						 info_tran_data.data[1], info_tran_data.data[2], info_tran_data.data[3]));

	switch (m_id) {
	case ITID_TXPA_INFO:
		if (PII_BINARY_INFO == s_id) {
			if (is_px_toolow(info_tran_data.data[0])) {
				if (!is_px_vol_state_toolow(dev_px_vol_state_vec, SE_PA_VOL_STATE_OFFSET)) {
					++is_update_se_table;
					set_px_vol_state(dev_px_vol_state_vec, SE_PA_VOL_STATE_OFFSET, VOLTAGE_TOO_LOW);
					rtgui_form_set_item(se_form, print_info_str[PIS_ID_GUODI],
										SE_PA_OVER_RANGE_ROW, SE_PA_OVER_RANGE_COL, 0);
				}
			} else if (is_px_tooheigh(info_tran_data.data[0])) {
				if (!is_px_vol_state_tooheigh(dev_px_vol_state_vec, SE_PA_VOL_STATE_OFFSET)) {
					++is_update_se_table;
					set_px_vol_state(dev_px_vol_state_vec, SE_PA_VOL_STATE_OFFSET, VOLTAGE_TOO_HEIGH);
					rtgui_form_set_item(se_form, print_info_str[PIS_ID_GUOGAO],
										SE_PA_OVER_RANGE_ROW, SE_PA_OVER_RANGE_COL, 0);
				}
			} else {
				if (!is_px_vol_state_normal(dev_px_vol_state_vec, SE_PA_VOL_STATE_OFFSET)) {
					++is_update_se_table;
					set_px_vol_state(dev_px_vol_state_vec, SE_PA_VOL_STATE_OFFSET, VOLTAGE_NORMAL);
					rtgui_form_set_item(se_form, print_info_str[PIS_ID_ZHENGCHANG],
										SE_PA_OVER_RANGE_ROW, SE_PA_OVER_RANGE_COL, 0);
				}
			}

			if (is_px_lost_phase(info_tran_data.data[0])) {
				if (is_bit_clr(dev_px_state_vec, SE_PA_POWER_FAIL_BIT)) {
					++is_update_se_table;
					set_dev_px_state(dev_px_state_vec, SE_PA_POWER_FAIL_BIT);
					rtgui_form_set_item(se_form, print_info_str[PIS_ID_SHI], SE_PA_POWER_DOWN_ROW,
										SE_PA_POWER_DOWN_COL, 0);
				}
			} else {
				if (is_bit_set(dev_px_state_vec, SE_PA_POWER_FAIL_BIT)) {
					++is_update_se_table;
					clr_dev_px_state(dev_px_state_vec, SE_PA_POWER_FAIL_BIT);
					rtgui_form_set_item(se_form, print_info_str[PIS_ID_FOU], SE_PA_POWER_DOWN_ROW,
										SE_PA_POWER_DOWN_COL, 0);
				}
			}
		}
		break;

	case ITID_TXPB_INFO:
		if (PII_BINARY_INFO == s_id) {
			if (is_px_toolow(info_tran_data.data[0])) {
				if (!is_px_vol_state_toolow(dev_px_vol_state_vec, SE_PB_VOL_STATE_OFFSET)) {
					++is_update_se_table;
					set_px_vol_state(dev_px_vol_state_vec, SE_PB_VOL_STATE_OFFSET, VOLTAGE_TOO_LOW);
					rtgui_form_set_item(se_form, print_info_str[PIS_ID_GUODI],
										SE_PB_OVER_RANGE_ROW, SE_PB_OVER_RANGE_COL, 0);
				}
			} else if (is_px_tooheigh(info_tran_data.data[0])) {
				if (!is_px_vol_state_tooheigh(dev_px_vol_state_vec, SE_PB_VOL_STATE_OFFSET)) {
					++is_update_se_table;
					set_px_vol_state(dev_px_vol_state_vec, SE_PB_VOL_STATE_OFFSET, VOLTAGE_TOO_HEIGH);
					rtgui_form_set_item(se_form, print_info_str[PIS_ID_GUOGAO],
										SE_PB_OVER_RANGE_ROW, SE_PB_OVER_RANGE_COL, 0);
				}
			} else {
				if (!is_px_vol_state_normal(dev_px_vol_state_vec, SE_PB_VOL_STATE_OFFSET)) {
					++is_update_se_table;
					set_px_vol_state(dev_px_vol_state_vec, SE_PB_VOL_STATE_OFFSET, VOLTAGE_NORMAL);
					rtgui_form_set_item(se_form, print_info_str[PIS_ID_ZHENGCHANG],
										SE_PB_OVER_RANGE_ROW, SE_PB_OVER_RANGE_COL, 0);
				}
			}

			if (is_px_lost_phase(info_tran_data.data[0])) {
				if (is_bit_clr(dev_px_state_vec, SE_PB_POWER_FAIL_BIT)) {
					++is_update_se_table;
					set_dev_px_state(dev_px_state_vec, SE_PB_POWER_FAIL_BIT);
					rtgui_form_set_item(se_form, print_info_str[PIS_ID_SHI], SE_PB_POWER_DOWN_ROW,
										SE_PB_POWER_DOWN_COL, 0);
				}
			} else {
				if (is_bit_set(dev_px_state_vec, SE_PB_POWER_FAIL_BIT)) {
					++is_update_se_table;
					clr_dev_px_state(dev_px_state_vec, SE_PB_POWER_FAIL_BIT);
					rtgui_form_set_item(se_form, print_info_str[PIS_ID_FOU], SE_PB_POWER_DOWN_ROW,
										SE_PB_POWER_DOWN_COL, 0);
				}
			}
		}
		break;

	case ITID_TXPC_INFO:
		if (PII_BINARY_INFO == s_id) {
			if (is_px_toolow(info_tran_data.data[0])) {
				if (!is_px_vol_state_toolow(dev_px_vol_state_vec, SE_PC_VOL_STATE_OFFSET)) {
					++is_update_se_table;
					set_px_vol_state(dev_px_vol_state_vec, SE_PC_VOL_STATE_OFFSET, VOLTAGE_TOO_LOW);
					rtgui_form_set_item(se_form, print_info_str[PIS_ID_GUODI],
										SE_PC_OVER_RANGE_ROW, SE_PC_OVER_RANGE_COL, 0);
				}
			} else if (is_px_tooheigh(info_tran_data.data[0])) {
				if (!is_px_vol_state_tooheigh(dev_px_vol_state_vec, SE_PC_VOL_STATE_OFFSET)) {
					++is_update_se_table;
					set_px_vol_state(dev_px_vol_state_vec, SE_PC_VOL_STATE_OFFSET, VOLTAGE_TOO_HEIGH);
					rtgui_form_set_item(se_form, print_info_str[PIS_ID_GUOGAO],
										SE_PC_OVER_RANGE_ROW, SE_PC_OVER_RANGE_COL, 0);
				}
			} else {
				if (!is_px_vol_state_normal(dev_px_vol_state_vec, SE_PC_VOL_STATE_OFFSET)) {
					++is_update_se_table;
					set_px_vol_state(dev_px_vol_state_vec, SE_PC_VOL_STATE_OFFSET, VOLTAGE_NORMAL);
					rtgui_form_set_item(se_form, print_info_str[PIS_ID_ZHENGCHANG],
										SE_PC_OVER_RANGE_ROW, SE_PC_OVER_RANGE_COL, 0);
				}
			}

			if (is_px_lost_phase(info_tran_data.data[0])) {
				if (is_bit_clr(dev_px_state_vec, SE_PC_POWER_FAIL_BIT)) {
					++is_update_se_table;
					set_dev_px_state(dev_px_state_vec, SE_PC_POWER_FAIL_BIT);
					rtgui_form_set_item(se_form, print_info_str[PIS_ID_SHI], SE_PC_POWER_DOWN_ROW,
										SE_PC_POWER_DOWN_COL, 0);
				}
			} else {
				if (is_bit_set(dev_px_state_vec, SE_PC_POWER_FAIL_BIT)) {
					++is_update_se_table;
					clr_dev_px_state(dev_px_state_vec, SE_PC_POWER_FAIL_BIT);
					rtgui_form_set_item(se_form, print_info_str[PIS_ID_FOU], SE_PC_POWER_DOWN_ROW,
										SE_PC_POWER_DOWN_COL, 0);
				}
			}
		}
		break;

	case ITID_RXPA_INFO:
		if (PII_BINARY_INFO == s_id) {
			if (is_px_toolow(info_tran_data.data[0])) {
				if (!is_px_vol_state_toolow(dev_px_vol_state_vec, RE_PA_VOL_STATE_OFFSET)) {
					++is_update_re_table;
					set_px_vol_state(dev_px_vol_state_vec, RE_PA_VOL_STATE_OFFSET, VOLTAGE_TOO_LOW);
					rtgui_form_set_item(re_form, print_info_str[PIS_ID_GUODI],
										RE_PA_OVER_RANGE_ROW, RE_PA_OVER_RANGE_COL, 0);
				}
			} else if (is_px_tooheigh(info_tran_data.data[0])) {
				if (!is_px_vol_state_tooheigh(dev_px_vol_state_vec, RE_PA_VOL_STATE_OFFSET)) {
					++is_update_re_table;
					set_px_vol_state(dev_px_vol_state_vec, RE_PA_VOL_STATE_OFFSET, VOLTAGE_TOO_HEIGH);
					rtgui_form_set_item(re_form, print_info_str[PIS_ID_GUOGAO],
										RE_PA_OVER_RANGE_ROW, RE_PA_OVER_RANGE_COL, 0);
				}
			} else {
				if (!is_px_vol_state_normal(dev_px_vol_state_vec, RE_PA_VOL_STATE_OFFSET)) {
					++is_update_re_table;
					set_px_vol_state(dev_px_vol_state_vec, RE_PA_VOL_STATE_OFFSET, VOLTAGE_NORMAL);
					rtgui_form_set_item(re_form, print_info_str[PIS_ID_ZHENGCHANG],
										RE_PA_OVER_RANGE_ROW, RE_PA_OVER_RANGE_COL, 0);
				}
			}

		} else if (PII_AVG_VAL == s_id) {
			temp = get_16bits_val(info_tran_data.data[0], info_tran_data.data[1]);
			if (temp != avg_val[AVI_RE_PA]) {
				avg_val[AVI_RE_PA] = temp;
				convert_avg_val2effective_val(temp, str);
				if (0 != rt_strncmp(avg_val_str[AVI_RE_PA], str, sizeof(avg_val_str[AVI_RE_PA]))) {
					++is_update_re_table;
					rt_strncpy(avg_val_str[AVI_RE_PA], str, sizeof(avg_val_str[AVI_RE_PA]));
					rtgui_form_set_item(re_form, str, RE_PA_AVG_VALUE_ROW, RE_PA_AVG_VALUE_COL, 0);
				}
			}
		}
		break;

	case ITID_RXPB_INFO:
		if (PII_BINARY_INFO == s_id) {
			if (is_px_toolow(info_tran_data.data[0])) {
				if (!is_px_vol_state_toolow(dev_px_vol_state_vec, RE_PB_VOL_STATE_OFFSET)) {
					++is_update_re_table;
					set_px_vol_state(dev_px_vol_state_vec, RE_PB_VOL_STATE_OFFSET, VOLTAGE_TOO_LOW);
					rtgui_form_set_item(re_form, print_info_str[PIS_ID_GUODI],
										RE_PB_OVER_RANGE_ROW, RE_PB_OVER_RANGE_COL, 0);
				}
			} else if (is_px_tooheigh(info_tran_data.data[0])) {
				if (!is_px_vol_state_tooheigh(dev_px_vol_state_vec, RE_PB_VOL_STATE_OFFSET)) {
					++is_update_re_table;
					set_px_vol_state(dev_px_vol_state_vec, RE_PB_VOL_STATE_OFFSET, VOLTAGE_TOO_HEIGH);
					rtgui_form_set_item(re_form, print_info_str[PIS_ID_GUOGAO],
										RE_PB_OVER_RANGE_ROW, RE_PB_OVER_RANGE_COL, 0);
				}
			} else {
				if (!is_px_vol_state_normal(dev_px_vol_state_vec, RE_PB_VOL_STATE_OFFSET)) {
					++is_update_re_table;
					set_px_vol_state(dev_px_vol_state_vec, RE_PB_VOL_STATE_OFFSET, VOLTAGE_NORMAL);
					rtgui_form_set_item(re_form, print_info_str[PIS_ID_ZHENGCHANG],
										RE_PB_OVER_RANGE_ROW, RE_PB_OVER_RANGE_COL, 0);
				}
			}

		} else if (PII_AVG_VAL == s_id) {
			temp = get_16bits_val(info_tran_data.data[0], info_tran_data.data[1]);
			if (temp != avg_val[AVI_RE_PB]) {
				avg_val[AVI_RE_PB] = temp;
				convert_avg_val2effective_val(temp, str);
				if (0 != rt_strncmp(avg_val_str[AVI_RE_PB], str, sizeof(avg_val_str[AVI_RE_PB]))) {
					++is_update_re_table;
					rt_strncpy(avg_val_str[AVI_RE_PB], str, sizeof(avg_val_str[AVI_RE_PB]));
					rtgui_form_set_item(re_form, str, RE_PB_AVG_VALUE_ROW, RE_PB_AVG_VALUE_COL, 0);
				}
			}
		}
		break;

	case ITID_RXPC_INFO:
		if (PII_BINARY_INFO == s_id) {
			if (is_px_toolow(info_tran_data.data[0])) {
				if (!is_px_vol_state_toolow(dev_px_vol_state_vec, RE_PC_VOL_STATE_OFFSET)) {
					++is_update_re_table;
					set_px_vol_state(dev_px_vol_state_vec, RE_PC_VOL_STATE_OFFSET, VOLTAGE_TOO_LOW);
					rtgui_form_set_item(re_form, print_info_str[PIS_ID_GUODI],
										RE_PC_OVER_RANGE_ROW, RE_PC_OVER_RANGE_COL, 0);
				}
			} else if (is_px_tooheigh(info_tran_data.data[0])) {
				if (!is_px_vol_state_tooheigh(dev_px_vol_state_vec, RE_PC_VOL_STATE_OFFSET)) {
					++is_update_re_table;
					set_px_vol_state(dev_px_vol_state_vec, RE_PC_VOL_STATE_OFFSET, VOLTAGE_TOO_HEIGH);
					rtgui_form_set_item(re_form, print_info_str[PIS_ID_GUOGAO],
										RE_PC_OVER_RANGE_ROW, RE_PC_OVER_RANGE_COL, 0);
				}
			} else {
				if (!is_px_vol_state_normal(dev_px_vol_state_vec, RE_PC_VOL_STATE_OFFSET)) {
					++is_update_re_table;
					set_px_vol_state(dev_px_vol_state_vec, RE_PC_VOL_STATE_OFFSET, VOLTAGE_NORMAL);
					rtgui_form_set_item(re_form, print_info_str[PIS_ID_ZHENGCHANG],
										RE_PC_OVER_RANGE_ROW, RE_PC_OVER_RANGE_COL, 0);
				}
			}

		} else if (PII_AVG_VAL == s_id) {
			temp = get_16bits_val(info_tran_data.data[0], info_tran_data.data[1]);
			if (temp != avg_val[AVI_RE_PC]) {
				avg_val[AVI_RE_PC] = temp;
				convert_avg_val2effective_val(temp, str);
				if (0 != rt_strncmp(avg_val_str[AVI_RE_PC], str, sizeof(avg_val_str[AVI_RE_PC]))) {
					++is_update_re_table;
					rt_strncpy(avg_val_str[AVI_RE_PC], str, sizeof(avg_val_str[AVI_RE_PC]));
					rtgui_form_set_item(re_form, str, RE_PC_AVG_VALUE_ROW, RE_PC_AVG_VALUE_COL, 0);
				}
			}
		}
		break;

	case ITID_TXE_INFO :
		if (TRXII_BINARY_INFO == s_id) {
			if(is_colletion_end_power_down(info_tran_data.data[0]))
				colletion_end_power_down_proc(1); // mark by David
			else
				colletion_end_power_down_proc(0);
		} else if (TRXII_CPU_TEMP==s_id) {
			temp = get_16bits_val(info_tran_data.data[0], info_tran_data.data[1]);
#if 0
			temper = (143 - temp*330/4095) * 1000/430 + 25;
#else
			temper = (140 - temp*330/4095) * 1000/460 + 25;
#endif
			if (temper>=-40 && temper<=100 && temp!=sys_temper[STI_SE_CPU_T]) {
				++is_update_sys_table;
				sys_temper[STI_SE_CPU_T] = temp;
				i2str(str, temper);
				strcat(str, CELSIUS_STR);
				rtgui_form_set_item(sys_form, str, RE_SE_CPU_TEMP_ROW, RE_SE_CPU_TEMP_COL, 0);
			}
		} else if (TRXII_BOX_TEMP==s_id) {
			temp = get_16bits_val(info_tran_data.data[0], info_tran_data.data[1]);
			if (temp != sys_temper[STI_SE_CTB_T]) {
				sys_temper[STI_SE_CTB_T] =  temp;
				convert_box_adc2temper(1, temp, str);
				if (0 != rt_strncmp(sys_temper_str[STI_SE_CTB_T], str, sizeof(sys_temper_str[STI_SE_CTB_T]))) {
					++is_update_sys_table;
					rt_strncpy(sys_temper_str[STI_SE_CTB_T], str, sizeof(sys_temper_str[STI_SE_CTB_T]));
					rtgui_form_set_item(sys_form, str, RE_SE_BOX_TEMP_ROW, RE_SE_BOX_TEMP_COL, 0);
				}
			}
		} else if (TRXII_VERSION == s_id) {
			temp = get_24bits_val(info_tran_data.data[0], info_tran_data.data[1], info_tran_data.data[2]);
			if (temp != se_version) {
				++is_update_sys_table;
				se_version = temp;
				convert_ver2str(temp, str);
				rtgui_form_set_item(sys_form, str, RE_SE_SOFT_VER_ROW, RE_SE_SOFT_VER_COL, 0);
			}
		} else if (TRXII_DEV_SN == s_id) {
			rt_strncpy(str, (char *)(info_tran_data.data), sizeof(str));

			printf_syn("line:%d, %s(), recv tx-sn:%s\n", __LINE__, __FUNCTION__, str);

			for (temp=0; temp<DEV_SN_MODE_LEN; ++temp)
				if (!is_valid_devsn_char(str[temp])) {
					printf_syn("had recv devsn invalid char '%c'(%d)\n", str[temp], str[temp]);
					break;
				}

			if ((temp>=DEV_SN_MODE_LEN) && (0 != rt_strncmp(str, (char *)tx_dev_sn, DEV_SN_MODE_LEN))) {
				++is_update_sys_table;

				rt_strncpy((char *)tx_dev_sn, str, DEV_SN_MODE_LEN);
				creat_devsn(str, sizeof(str), 0);
				rtgui_form_set_row(sys_form, str, TE_SN_ROW);

				if (FAIL == check_match_sn((char *)tx_dev_sn)) {
					/* send not match cmd,and to pt. */
					send_cmd2workbench(SN_NOT_MATCH, NULL);
				} else {
					send_cmd_to_rxe(0, SWITCH_FROM_PT); /* 取消"切换到pt", 也就是"切换到零压降的输出端" */
					//send match cmd.
					if (RT_NULL != sn_not_match_win) {
						rtgui_win_destroy(sn_not_match_win);
					}
				}
			}
		}
		break;

	case ITID_RXE_INFO :
		switch (s_id) {
		case TRXII_BINARY_INFO:
			/* mark by David */
			if(is_restore_end_switch2pt(info_tran_data.data[0])) {
				buzzer_tweet = 1;
				if (!is_switch2pt_sysmisc()) {
					set_switch2pt_flag_sysmisc();
					send_cmd2workbench(UPDATE_SW_CABLE, NULL);
				}
			} else {
				buzzer_tweet = 0;
				if (is_switch2pt_sysmisc()) {
					clr_switch2pt_flag_sysmisc();
					send_cmd2workbench(UPDATE_SW_CABLE, NULL);
				}
			}

			if (is_restore_end_recv_fibre_data(info_tran_data.data[0])) {
				if (!is_fibre_recv_data_sysmisc()) {
					set_fibre_recv_data_flag_sysmisc();
				}
			} else {
				if (is_fibre_recv_data_sysmisc()) {
					clr_fibre_recv_data_flag_sysmisc();
				}
			}
			break;

		case TRXII_CPU_TEMP:
			temp = get_16bits_val(info_tran_data.data[0], info_tran_data.data[1]);
#if 0
			temper = (143 - temp*330/4095) * 1000/430 + 25;
#else
			temper = (140 - temp*330/4095) * 1000/460 + 25;
#endif

			if (temper>=-40 && temper<=100 && temp!=sys_temper[STI_RE_CPU_T]) {
				++is_update_sys_table;
				sys_temper[STI_RE_CPU_T] = temp;
				i2str(str, temper);
				strcat(str, CELSIUS_STR);
				rtgui_form_set_item(sys_form, str, RE_RE_CPU_TEMP_ROW, RE_RE_CPU_TEMP_COL, 0);
			}
			break;

		case TRXII_BOX_TEMP:
			temp = get_16bits_val(info_tran_data.data[0], info_tran_data.data[1]);
			if (temp != sys_temper[STI_RE_CTB_T]) {
				sys_temper[STI_RE_CTB_T] =  temp;
				convert_box_adc2temper(0, temp, str);
				if (0 != rt_strncmp(sys_temper_str[STI_RE_CTB_T], str, sizeof(sys_temper_str[STI_RE_CTB_T]))) {
					++is_update_sys_table;
					rt_strncpy(sys_temper_str[STI_RE_CTB_T], str, sizeof(sys_temper_str[STI_RE_CTB_T]));
					rtgui_form_set_item(sys_form, str, RE_RE_BOX_TEMP_ROW, RE_RE_BOX_TEMP_COL, 0);
				}
			}
			break;

		case TRXII_VERSION:
			temp = get_24bits_val(info_tran_data.data[0], info_tran_data.data[1], info_tran_data.data[2]);
			if (temp != re_version) {
				++is_update_sys_table;
				re_version = temp;
				convert_ver2str(temp, str);
				rtgui_form_set_item(sys_form, str, RE_RE_SOFT_VER_ROW, RE_RE_SOFT_VER_COL, 0);

				if (FAIL == check_software_match(str)) {
					/* send not match cmd,and to pt. */
					send_cmd2workbench(SOFTWARE_VER_NOT_MATCH, NULL);
				} else {
					send_cmd_to_rxe(0, SWITCH_FROM_PT); /* 取消"切换到pt", 也就是"切换到零压降的输出端" */
					if (RT_NULL != software_version_not_match_win) {
						rtgui_win_destroy(software_version_not_match_win);
					}
				}
			}
			break;

		case TRXII_RX_CONTINUE_OVERLOAD_CNT:
			rx_overload_proc(info_tran_data.data[0]>>4);
			break;

		case TRXII_USE_FIBER_NO:
			printf_syn("%s(), recv current use fiber NO. is %d\n", __FUNCTION__, info_tran_data.data[0]>>4);

			temp = info_tran_data.data[0]>>4;
			if (temp != get_sys_use_channel()) {
				if (SUCC != set_sys_use_channel(temp)) {
					printf_syn("recv invalid channel-no(%d) from rxe\n", temp);
				} else {
#if USE_OPTICX_200S_VERSION
					str[0] = convert_cur_channel_to_char();
					str[1] = '\0';
					send_cmd2workbench(UPDATE_CUR_CHANNEL_NO, str);
#endif
				}
			}
			break;

		default:
			printf_syn("%s() recv invalid s_id %d\n", __FUNCTION__, s_id);
			break;
		}
		break;

	case ITID_RXE_ACK:
		if (TRX_SWITCH2PT == s_id) {
			extern struct rt_timer uart3_ack_timer;
			
			send_cmd2workbench(SWITCH_2_PT, NULL);
			rt_timer_stop(&uart3_ack_timer);

			rt_kprintf("timer is stop!\n");
		}
		break;

	default:
		break;
	}

	if(0 != is_update_se_table)
		send_cmd2workbench(SE_FORM_UPDATE, NULL);

	if(0 != is_update_re_table)
		send_cmd2workbench(RE_FORM_UPDATE, NULL);

	if(0 != is_update_sys_table)
		send_cmd2workbench(SYS_FORM_UPDATE, NULL);

	return 0;
}

static int check_match_sn(char *str)
{
	struct m3_sys_info_st sys_info;

	if (NULL == str)
		return FAIL;

#if 0 /* 调用该函数时, 已确保了字符串的有效性 */
	for (temp=0; temp<DEV_SN_MODE_LEN; ++temp)
		if (!is_valid_devsn_char(str[temp])) {
			printf_syn("had recv devsn invalid char '%c'(%d)\n", str[temp], str[temp]);
			break;
		}


	if (temp>=DEV_SN_MODE_LEN) {
		read_syscfgdata_tbl(SYSCFGDATA_TBL_SYS_INFO, 0, &sys_info);

		if ((0 == rt_strncmp(str, (char *)sys_info.match_sn[0], DEV_SN_MODE_LEN)) || 
			(0 == rt_strncmp(str, (char *)sys_info.match_sn[1], DEV_SN_MODE_LEN))) {
			return SUCC;
		}
	}
#else
	rt_memset(&sys_info, 0, sizeof(sys_info));
	read_syscfgdata_tbl(SYSCFGDATA_TBL_SYS_INFO, 0, &sys_info);

	if ((0 == rt_strncmp(str, (char *)sys_info.match_sn[0], DEV_SN_MODE_LEN)) ||
		(0 == rt_strncmp(str, (char *)sys_info.match_sn[1], DEV_SN_MODE_LEN))) {
		return SUCC;
	} else {
		if ('\0' != sys_info.match_sn[0][0]) {
			printf_syn("shoule '%s' ", sys_info.match_sn[0]);

			if ('\0' != sys_info.match_sn[1][0]) {
				printf_syn("or '%s' ", sys_info.match_sn[1]);
			}

			printf_syn("but recv tx sn:%s\n ", str);

		} else {
			printf_syn("NOT config match sn in database\n");
		}
	}
#endif
	return FAIL;
}

static int check_software_match(char *ver)
{
	if (NULL == ver)
		return FAIL;

#if USE_OPTICX_200S_VERSION
	if (0 != (ver[0] & 0x01))
		return SUCC;
	else
		return FAIL;
#else
	if (0 == (ver[0] & 0x01))
		return SUCC;
	else
		return FAIL;
#endif
}

int send_cmd2workbench(enum app_workbench_cmd_id cmd, char *cmd_str)
{
	rt_err_t ret;
	struct rtgui_event_command ecmd;
	rt_thread_t th;
	int i;

	th = rt_thread_find("wb");
	if (NULL == th) {
		printf_syn("func:%s, find workbench fail\n", __FUNCTION__);
		return RT_ERROR;
	}

	if (MOUSE_BUTTON_EVENT == cmd) {
		/* post command event */
		RTGUI_EVENT_MOUSE_BUTTON_INIT(&ecmd);

		printf_syn("%s(), line:%d\n", __FUNCTION__, __LINE__);
	} else {
		/* post command event */
		RTGUI_EVENT_COMMAND_INIT(&ecmd);
		ecmd.command_id = cmd;
		if (NULL != cmd_str) {
			i = 0;
			while ('\0' != *cmd_str && i<RTGUI_NAME_MAX) {
				ecmd.command_string[i++] = *cmd_str++;
			}

			if (i<RTGUI_NAME_MAX)
				ecmd.command_string[i] = '\0';
			else
				ecmd.command_string[RTGUI_NAME_MAX-1] = '\0';
		} else {
			ecmd.command_string[0] = '\0';
		}
	}

	ret = rtgui_thread_send(th, &ecmd.parent, sizeof(struct rtgui_event_command));
	if (RT_EOK != ret)
		printf_syn("send enent cmd fail(%d)\n", ret);

	return ret;
}


/*
 * str指向的空间要 >= 16 bytes
 */
int convert_ver2str(unsigned int ver, char *str)
{
	int temp;
	char vers[16];
	char *pch;

	temp = (ver >> 16) & 0xff;
	i2str(vers, temp);

	pch = vers;
	while ('\0' != *pch)
		++pch;
	*pch++ = '.';
	temp = (ver >> 8) & 0xff;
	i2str(pch, temp);

	while ('\0' != *pch)
		++pch;
	*pch++ = '.';
	temp = (ver) & 0xff;
	i2str(pch, temp);

	rt_strncpy(str, vers, sizeof(vers));
	str[15] = 0;

	return 0;
}

/*
 * vol:
 *	1 -- 3.3v
 *	0 -- 5.0v
 */
static int convert_box_adc2temper(int vol, int adc, char *str)
{
	int i, temper;
	int diff, min_diff_index, abs_value;
	int adcval_per_celsius;
	const struct mf5x_temper_adc_pair_st *pmf5x_tbl;

	if (vol)
		pmf5x_tbl = mf5x_table_3v3;
	else
		pmf5x_tbl = mf5x_table_5v0;

	if (adc >= pmf5x_tbl[0].adc_val) {
		temper = pmf5x_tbl[0].temper;
		goto had_get_temper;
	}

	if (adc <= pmf5x_tbl[MF5X_TABLE_ENUM_NUM-1].adc_val) {
		temper = pmf5x_tbl[MF5X_TABLE_ENUM_NUM-1].temper;
		goto had_get_temper;
	}

	diff = 4096;
	min_diff_index = 0;
	for (i=0; i<MF5X_TABLE_ENUM_NUM; ++i) {
		abs_value = sub_abs(pmf5x_tbl[i].adc_val, adc);
		if (abs_value < diff) {
			diff = abs_value;
			min_diff_index = i;
			if (0 == abs_value)
				break;
		}
	}

	if ((adc < pmf5x_tbl[min_diff_index].adc_val)) {
		++min_diff_index;
	}

	adcval_per_celsius = (pmf5x_tbl[min_diff_index-1].adc_val - pmf5x_tbl[min_diff_index].adc_val) / 5;
	temper = pmf5x_tbl[min_diff_index].temper - (adc - pmf5x_tbl[min_diff_index].adc_val)/adcval_per_celsius;

had_get_temper:
	i2str(str, temper);
	strcat(str, CELSIUS_STR);
#if 0
	info_tran_debug(("func:%s, adc:%d, adc-temper-pair(%d, %d)\n", __FUNCTION__, adc,
					 mf52_table[min_diff_index].adc_val, pmf5x_tbl[min_diff_index].temper));
#endif
	return 0;
}


/*
 * 2055(adc) 	-- 57660mv
 * 1(adc)		-- 28.06mv
 */
/*
 * E:\notepad-work\assist-eswd\zvd>calc_vol_adc.exe
 * pls input std voltage(mv):57700
 * min-std-max:46160.000000, 57700.000000, 69240.000000
 * pls input (vol-mv, adc)1:47950 1632
 * pls input (vol-mv, adc)2:60760 2069
 * pls input (vol-mv, adc)3:67880 2311
 * xmv_per_adc1-3: 29.381127, 29.366844, 29.372566
 * xmv_per_adc:29.373512
 * voltage(min, std, max):49045.000000, 57700.000000, 66355.000000
 * voltage-adc(min, std, max):1669.701575, 1964.354795, 2259.008014
 */
static int convert_avg_val2effective_val(int adc, char *str)
{
	int effec_val;
	char *pch;

	effec_val = (adc * 2937) / (1000*100);
	i2str(str, effec_val);

	pch = str;
	while ('\0' != *pch)
		++pch;

	*pch++ = 'V';
	*pch   = '\0';
#if 0
	info_tran_debug(("func:%s, adc:%d, effec_val:%d\n", __FUNCTION__, adc, effec_val));
#endif
	return 0;
}

static void colletion_end_power_down_proc(int is_poweroff)
{
	char buf[4];

	if (is_poweroff) {
		buf[0] = '1';
		write_tx_poweroff_info2bkp();
	} else {
		buf[0] = '0';
	}
	buf[1] = '\0';

	send_cmd2workbench(UPDATE_HAD_POWEROFF, buf);

	return;
}

static void rx_overload_proc(int cnt)
{
#if 1==DISPLAY_OVERLOAD_INFO
	char buf[4];

	buf[0] = '0' + cnt;
	buf[1] = '\0';

	send_cmd2workbench(UPDATE_OVERLOAD_CNT, buf);
#else
	static unsigned int continue_overload_cnt;

	if (0 == cnt) {
		continue_overload_cnt = 0;
		printf_syn("small than max load\n");
	} else {
		continue_overload_cnt += cnt;
		/* print overload cnt to uart  */
		printf_syn("continue_overload_cnt is %u\n", continue_overload_cnt);
	}
#endif

	return;
}


/*
 * 该函数用于在接收到采集端掉电信息时, 将采集端掉电信息写到备份寄存器
 */
static void write_tx_poweroff_info2bkp(void)
{
	unsigned poweroff_cnt;
	unsigned long time_bkp;
	unsigned time_bkp_h, time_bkp_l;

	if (NULL == rtc_dev)
		return;

	rt_device_control(rtc_dev, RT_DEVICE_CTRL_RTC_GET_TIME, &time_bkp);
	time_bkp_h = (time_bkp>>16) & 0xffff;
	time_bkp_l = time_bkp & 0xffff;

	PWR_BackupAccessCmd(ENABLE);

	if (SUC_FIBER_CHANNEL_2 == get_sys_use_channel()) {
		poweroff_cnt = BKP_ReadBackupRegister(TX2_POWEROFF_CNT_BKP16BITS);
		BKP_WriteBackupRegister(TX2_POWEROFF_CNT_BKP16BITS, ++poweroff_cnt);
		poweroff_info_data.poi_tx2_poweroff_cnt = poweroff_cnt;

		switch (poweroff_cnt % 3) {
		case 1:
			BKP_WriteBackupRegister(TX2_POWEROFF_1_BKP16BITS_H, time_bkp_h);
			BKP_WriteBackupRegister(TX2_POWEROFF_1_BKP16BITS_L, time_bkp_l);
			poweroff_info_data.poi_tx2_poweroff_t1 = time_bkp;
			break;

		case 2:
			BKP_WriteBackupRegister(TX2_POWEROFF_2_BKP16BITS_H, time_bkp_h);
			BKP_WriteBackupRegister(TX2_POWEROFF_2_BKP16BITS_L, time_bkp_l);
			poweroff_info_data.poi_tx2_poweroff_t2 = time_bkp;
			break;

		case 0:
			BKP_WriteBackupRegister(TX2_POWEROFF_0_BKP16BITS_H  , time_bkp_h);
			BKP_WriteBackupRegister(TX2_POWEROFF_0_BKP16BITS_L  , time_bkp_l);
			poweroff_info_data.poi_tx2_poweroff_t0 = time_bkp;
			break;

		default:
			break;
		}
	} else {
		poweroff_cnt = BKP_ReadBackupRegister(TX1_POWEROFF_CNT_BKP16BITS);
		BKP_WriteBackupRegister(TX1_POWEROFF_CNT_BKP16BITS, ++poweroff_cnt);
		poweroff_info_data.poi_tx1_poweroff_cnt = poweroff_cnt;

		switch (poweroff_cnt % 3) {
		case 1:
			BKP_WriteBackupRegister(TX1_POWEROFF_1_BKP16BITS_H, time_bkp_h);
			BKP_WriteBackupRegister(TX1_POWEROFF_1_BKP16BITS_L, time_bkp_l);
			poweroff_info_data.poi_tx1_poweroff_t1 = time_bkp;
			break;

		case 2:
			BKP_WriteBackupRegister(TX1_POWEROFF_2_BKP16BITS_H, time_bkp_h);
			BKP_WriteBackupRegister(TX1_POWEROFF_2_BKP16BITS_L, time_bkp_l);
			poweroff_info_data.poi_tx1_poweroff_t2 = time_bkp;
			break;

		case 0:
			BKP_WriteBackupRegister(TX1_POWEROFF_0_BKP16BITS_H  , time_bkp_h);
			BKP_WriteBackupRegister(TX1_POWEROFF_0_BKP16BITS_L  , time_bkp_l);
			poweroff_info_data.poi_tx1_poweroff_t0 = time_bkp;
			break;

		default:
			break;
		}
	}


	PWR_BackupAccessCmd(DISABLE);

	update_tx_poweroff_info_of_otherform();
	send_cmd2workbench(OTHER_FORM_UPDATE, NULL);

	return;
}

int send_cmd_to_rxe(int is_need_start_timer, enum uart3_cmd_format cmd)
{
	/* switch2pt */
	extern struct rt_device uart3_device;
	extern unsigned char info_tx_buf[64];
	extern struct rt_semaphore uart3_tx_sem;
	extern struct rt_timer uart3_ack_timer;

	rt_sem_take(&uart3_tx_sem, RT_WAITING_FOREVER);
	info_tx_buf[0] = CMD_HEAD;
	info_tx_buf[1] = cmd;
	rt_device_write(&uart3_device, 0, info_tx_buf, 2);

	if (0 != is_need_start_timer)
		rt_timer_start(&uart3_ack_timer);


	printf_syn("send cmd(0x%x) to rxe\n", info_tx_buf[1]);

	return 0;
}

void init_sys_use_channel(void)
{
	cur_use_channel = SUC_FIBER_CHANNEL_1;

	return;
}

int set_sys_use_channel(enum sys_use_channel_e channel)
{
	if (channel>=SUC_PT_CHANNEL && channel<SUC_BUTT) {
		cur_use_channel = channel;
		return SUCC;
	} else {
		printf_syn("func:%s() recv invalid param(%d)\n", __FUNCTION__, channel);
		return FAIL;
	}
}

enum sys_use_channel_e get_sys_use_channel(void)
{
	return cur_use_channel;
}

char convert_cur_channel_to_char(void)
{
	char ch;

	switch (cur_use_channel) {
	case SUC_PT_CHANNEL:
		ch = 'P';
		break;

	case SUC_FIBER_CHANNEL_1:
		ch = '1';
		break;

	case SUC_FIBER_CHANNEL_2:
		ch = '2';
		break;

	default:
		ch = 'E';
		break;
	}

	return ch;
}
