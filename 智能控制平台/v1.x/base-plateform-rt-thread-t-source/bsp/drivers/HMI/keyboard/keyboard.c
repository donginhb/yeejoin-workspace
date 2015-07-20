
#include <stm32f10x.h>
#include <stm32f10x_i2c.h>

#include "keyboard.h"
#include "tca8418.h"
#include <board.h>
#include <syscfgdata.h>

#include <ra8875.h>
#include <lcd_touch.h>
#include <state_grid_logo.h>
#include <rtdef.h>
#include <am2301.h>

#include <sys_app_cfg.h>


#define I2C1_DR_Address       0x40005410

//#define ClockSpeed      400000
#define ClockSpeed      100000
#define INVALID_KEY     (-1)

#if 1
#define keyboard_debug(x)  printf_syn x 
#else
#define keyboard_debug(x)  /* printf_syn x */
#endif

#if 0
#define  I2C1_BUFF_SIZE  (12)
char i2c1_buff_tx[I2C1_BUFF_SIZE];
char i2c1_buff_rx[I2C1_BUFF_SIZE];
#endif

#define KEY_E_BUF_SIZE  16
static char key_e_buf[KEY_E_BUF_SIZE];

struct rt_semaphore keyscan_chip_had_int;

/*
 * 临时处理文本菜单, 简单的开闭锁
 */
#if 1
/*
 **LEVEL1_MENU entry:
 * 1.智能门锁控制
 * 2.用电信息自助查询
 * 3.线路运行状态监测
 * 4.环境参数
 * 5.电力宣传
 */
enum proc_key_state_e {
	WAIT_ENTER_LEVEL1_MENU_NO	= 0,
	WAIT_ENTER_USRID,
	WAIT_ENTER_USRPW,
	WAIT_ENTER_LOCK_CTR_CMD,
	WAIT_ENTER_RETUREN_CMD,
};

enum proc_key_state_e proc_key_state = WAIT_ENTER_LEVEL1_MENU_NO;
int auth_style;


char input_id[PWD_LEN];
//static char input_pw[PWD_LEN];

//struct webm_mb_entity key_webm_mb;
#if 1==USE_TO_7INCH_LCD
char *publicity_info = "宁夏吴忠供电局为民工程“用心”服务\n\n"
"    4月19日，吴忠市利通区富平街，电力部门的新型开关柜经过试验合格，已运抵现场等待更换。这是吴忠供电局城市配电"
"自动化改造项目的一个改造现场，为了减少由于工作对客户的停电次数和时间，他们采取“零点行动”、“带电作业”、"
"“同步工作”等方法，用心做好这个为民工程。\n"
"    城市配电自动化是为实现配电网信息化、自动化、互动化，从而促进配电网生产运行和客户服务现代化管理水平的整体"
"提升。它的建成将进一步促进客户端电器智能化应用水平，为将来“物联网”的应用做准备。但原有的设备已不能够满足要"
"求，供电部门本着“你用电，我用心”的服务理念，倡导“四个服务”， 在现有一座220kV变电站，四座110kV变电站向城区"
"民居工程、市政办公、学校、商业、工业园区客户供电的基础上， 积极投入人力物力，计划改造或更换环网柜32台、分支"
"箱97台，柱上开关86台，完善相应的通信设备和自动化实现设备，配检中心青年突击队员积极发挥党、团员先锋模范作用，"
"合理分工，密切配合，协同作战，在为民服务创先争优方面 “走在前，做表率”。\n"
"    工程中，该局以制度保进度，以技术培训和第三方监理保质量，以现场布控保安全，统筹考虑，合理安排停电计划，尽"
"可能将用户接入、用户消缺、市政改造、线路检修等事件安排同步进行，避免同一线路的反复停电；带电更换保险；晚上零"
"点以后客户休息时间施工；在施工过程中，工作人员提前到达施工现场，提前做好相关的开工前准备，做到工作的无缝衔"
"接，减少对停电时间的不必要浪费，更好地服务利通区37万城区居民。";
#endif
#endif

#if 0==USE_KEY_CTR_CHIP
extern int is_matrixkey_had_hit(void);
extern int do_matrixkey_debounce(void);
extern int get_matrixkey_value(void);
extern void wait_matrixkey_release(void);
extern void set_rowin_colout(void);
extern void set_rowout_colin(void);
#endif
//static void stm32_i2c_init(void);
//static int get_key_eval_from_tca8418(char *key_e_buf);

static int analysis_key_event(char *key_e_buf, int cnt);
static int process_key(enum key_no keyno);
static enum key_no get_keyno_from_keyval(enum key_value keyval);

int is_usr_input_idandpw_correct(char *id, char *pw);
//static void pw_correct_process(void);
//static void pw_error_process(void);

#if 1
#include <lcd_touch.h>
#include <ili9320_fontid.h>

extern void webm_p_send_query_pconsump_and_proc_rep(char *id);
void dis_publicity_info(char *pch, int len);

#endif


void keyboard_init(void)
{
#if 1==USE_KEY_CTR_CHIP
	rt_sem_init(&keyscan_chip_had_int, "keypress", 0, RT_IPC_FLAG_FIFO);

#if 1==USE_KEY_CTR_RA8875
	init_ra8875_keyscan();
#elif 1==USE_KEY_CTR_TCA8418
	stm32_i2c_init();
	tca8418_init();
#endif

#endif
	return;
}




void thread_keyboard_entry(void* parameter)
{
	//int keyval;

	//int i, cnt, bufindex;

//keyboard_debug(("fun:%s, line:%d\n", __FUNCTION__, __LINE__));

#if 1==USE_TO_7INCH_LCD
	dis_top_frame();
	
	/* 状态监测箱用户选项 */
	i = 0;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_ZHI;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_NENG;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_KONG;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_ZHI1;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_XIANG;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_YING;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_YONG;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_XUAN;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_XIANG1;
	set_bit(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC);
	dis_title_line(&textmenu2lcd_mb, i);

	while (is_bit_set(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC))
		rt_thread_delay(get_ticks_of_ms(20));
	for (i=0; i<7; i++) {
		textmenu2lcd_mb.buf_pool[i] = FONT_ID_CN_NING+i;
	}
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_DIAN;
	textmenu2lcd_mb.buf_pool[i++]   = FONT_ID_CN_JU;
	set_bit(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC);
	dis_bottom_line(&textmenu2lcd_mb, i);

	dis_body_content_level1();
#endif

	while (1) {
#if 1==USE_KEY_CTR_CHIP

#if 1==USE_KEY_CTR_RA8875
		if (RT_EOK == rt_sem_take(&keyscan_chip_had_int, RT_WAITING_FOREVER)) {
			bufindex = 0;
			if (is_had_hitkey()) {
				cnt = get_hitkey_num();
				cnt = MIN(cnt, sizeof(key_e_buf)/sizeof(key_e_buf[0])); //做为保护
				for (i=0; i<cnt; i++)
					key_e_buf[i+bufindex] = get_hitkey_code(i);
				bufindex = cnt;
				analysis_key_event(key_e_buf, cnt);
				clr_key_status_bit();
			}
		}

#elif 1==USE_KEY_CTR_TCA8418
		if (RT_EOK == rt_sem_take(&keyscan_chip_had_int, RT_WAITING_FOREVER)) {
			cnt = get_key_eval_from_tca8418(key_e_buf);
			if (0 != cnt)
				analysis_key_event(key_e_buf, cnt);
		}
#endif

#else
		if (is_matrixkey_had_hit() && do_matrixkey_debounce()) {
			key_e_buf[0] = get_matrixkey_value();
			analysis_key_event(key_e_buf, 1);
			wait_matrixkey_release();			
		}

		rt_thread_delay(get_ticks_of_ms(200));
#endif
	}
}

#if 1==USE_KEY_CTR_TCA8418
static void stm32_i2c_init(void)
{
	I2C_InitTypeDef  I2C_InitStructure;
	//DMA_InitTypeDef  DMA_InitStructure;

	I2C_DeInit(I2C1);

	/* I2C1 configuration ------------------------------------------------------*/
	I2C_InitStructure.I2C_Mode                  = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle             = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1           = 0x30;
	I2C_InitStructure.I2C_Ack                   = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress   = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed            = ClockSpeed;
	I2C_Init(I2C1, &I2C_InitStructure);

#if 0
	/* DMA1 channel6 configuration ----------------------------------------------*/
	DMA_DeInit(DMA1_Channel6);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)I2C1_DR_Address;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)i2c1_buff_tx;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_BufferSize = I2C1_BUFF_SIZE;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel6, &DMA_InitStructure);


	/* DMA1 channel7 configuration ---------------------------------------------*/
	DMA_DeInit(DMA1_Channel7);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)I2C1_DR_Address;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)i2c1_buff_rx;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = I2C1_BUFF_SIZE;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_Init(DMA1_Channel7, &DMA_InitStructure);
#endif

	/* Enable I2C  ------------------------------------------------------------*/
	I2C_Cmd(I2C1, ENABLE);
	I2C_AcknowledgeConfig(I2C1, ENABLE);
#if 0
	I2C_DMACmd(I2C1, ENABLE);

#endif
}

/*
 * read all key event in tca8418 register KEY_EVENT_A
 *
 * key event -- store up to 10 key press and releases.
 *
 * return key event number.
 */
static int get_key_eval_from_tca8418(char *key_e_buf)
{
	int temp, cnt;
	char key_event_cnt;

	if (SUCC != tca8418_read_reg(REG_KEY_LCK_EC, &key_event_cnt))
		return 0;

	key_event_cnt &= 0x0f;
	if (0==key_event_cnt)
		return 0;
		
	if (key_event_cnt > KEY_E_BUF_SIZE)
		key_event_cnt = KEY_E_BUF_SIZE;

	temp = key_event_cnt;
	while (temp-- > 0) {
		tca8418_read_reg(REG_KEY_EVENT_A, key_e_buf++);
	}

	cnt = key_event_cnt - temp;
	if (SUCC==tca8418_read_reg(REG_KEY_LCK_EC, &key_event_cnt)) {
		key_event_cnt &= 0x0f;
		if (0==key_event_cnt)
			goto ret;
		
		if (key_event_cnt > (KEY_E_BUF_SIZE - cnt))
			key_event_cnt = KEY_E_BUF_SIZE - cnt;

		temp = key_event_cnt;
		while (temp-- > 0) {
			tca8418_read_reg(REG_KEY_EVENT_A, key_e_buf++);
		}

		cnt += key_event_cnt - temp;
	}

ret:
	tca8418_write_reg(REG_INT_STAT, 0x01); /* clear k_int bit */
	return cnt;
}
#endif

/*
 * 现在只做简单处理, 假设同一个按键的 '按下', '释放' 是连续的
 */
static int analysis_key_event(char *key_e_buf, int cnt)
{
	//static int last_key_e = INVALID_KEY;
	int keyval, temp;

	while (0 != cnt--) {
		temp = *key_e_buf;
keyboard_debug(("fun:%s, line:%d, key_e:0x%x\n", __FUNCTION__, __LINE__, temp));
#if 1
		keyval = get_keyno_from_keyval(temp);
		process_key(keyval);
		++key_e_buf;
		
#else
		if (INVALID_KEY == last_key_e) {
			last_key_e = *key_e_buf++;
			continue;
		}

		if ((last_key_e & 0x80) && !(temp & 0x80)
				&& ((last_key_e & 0x7f) == temp)) {
			keyval = get_keyno_from_keyval(temp);
			process_key(keyval);

		}

		++key_e_buf;
		last_key_e = INVALID_KEY;
#endif
	}

	return 0;
}

static int process_key(enum key_no keyno)
{
#if 0==USE_TO_7INCH_LCD
        keyboard_debug(("key_no: 0x%x\n", keyno));
	switch (keyno) {
	case KEY_NO_NUM0 :
		break;
	case KEY_NO_NUM1 :
		break;
	case KEY_NO_NUM2 :
		break;
	case KEY_NO_NUM3 :
		break;
	case KEY_NO_NUM4 :
		break;
	case KEY_NO_NUM5 :
		break;
	case KEY_NO_NUM6 :
		break;
	case KEY_NO_NUM7 :
		break;
	case KEY_NO_NUM8 :
		break;
	case KEY_NO_NUM9 :
		break;
	case KEY_NO_ENTER:
		break;
	case KEY_NO_MENU :
		break;
	case KEY_NO_ESC  :
		break;
	case KEY_NO_UP   :
		break;
	case KEY_NO_DOWN :
		break;

	case KEY_NO_MODIFY :
		break;
	case KEY_NO_DOT :
		break;
	case KEY_NO_STAR :
		break;
	case KEY_NO_LINE :
		break;

	default:
		break;
	}

	return 0;
#else
	int len, tempchar;
	char *pch;

        keyboard_debug(("process_key() -- key_no: 0x%x, proc_key_state:%d\n", keyno, proc_key_state));

	if (KEY_NO_ESC == keyno) {
		/* return main interface */
		return_main_interface();

		return 0;
	}
	
	switch (proc_key_state) {
	case WAIT_ENTER_LEVEL1_MENU_NO:
		if (KEY_NO_NUM1==keyno) {
			/* display control lock login; */
			clr_body_zone();
			proc_key_state = WAIT_ENTER_USRID;
			auth_style = AUTH_STYLE_ELOCK_CTRL;
			dis_ctr_elock_login();
		} else if(KEY_NO_NUM2==keyno) {
			clr_body_zone();
			proc_key_state = WAIT_ENTER_USRID;
			auth_style = AUTH_STYLE_QUERY_PCOMSUMP;
			dis_query_pconsump_login();
		} else if(KEY_NO_NUM3==keyno) {
			clr_body_zone();
			proc_key_state = WAIT_ENTER_RETUREN_CMD;
			dis_net_link_state(UNKNOW_STATE, UNKNOW_STATE);
		} else if(KEY_NO_NUM4==keyno) {
			clr_body_zone();
			proc_key_state = WAIT_ENTER_RETUREN_CMD;
			/* temperature, relative humidity */
			dis_tmp_rh();
		} else if(KEY_NO_NUM5==keyno) {
			clr_body_zone();
			proc_key_state = WAIT_ENTER_RETUREN_CMD;
			/* query warning infomation  */
			dis_warning_info();
		} else if(KEY_NO_NUM6==keyno) {
			clr_body_zone();
			proc_key_state = WAIT_ENTER_RETUREN_CMD;
			/* Publicity of electric power, 电力宣传 */
			if (NULL != publicity_from_webm.buf) {
				pch = (char*)get_publicity_start_addr(publicity_from_webm.buf);
				dis_publicity_info(pch, ntohs(*((u16*)(pch-2))) );
			} else {
				dis_publicity_info(publicity_info, rt_strlen(publicity_info));
			}
		} else {
			/* don't process other menu entrys */
		}
		break;

	case WAIT_ENTER_USRID         :
		if (KEY_NO_ENTER == keyno) {
			clr_arrowhead(USRID_ARROWHEAD_X, USRID_ARROWHEAD_Y);
			proc_key_state = WAIT_ENTER_USRPW;
			dis_arrowhead(PW_ARROWHEAD_X, PW_ARROWHEAD_Y);
		} else if (KEY_NO_MODIFY == keyno) {
			rt_memset(input_id, 0, sizeof(input_id));
			clr_usrid_zone();
		} else if (keyno>=KEY_NO_NUM0 && keyno<=KEY_NO_NUM9) {
			len = rt_strlen(input_id);
			if (len < sizeof(input_id)-1) {
				if (KEY_NO_STAR == keyno)
					tempchar = '*';
				else if (KEY_NO_DOT == keyno)
					tempchar = '.';
				else if (KEY_NO_LINE == keyno)
					tempchar = '-';
				else
					tempchar = keyno + '0';
				input_id[len++] = tempchar;
				input_id[len]   = '\0';

				//print char on lcd;
				dis_one_char(tempchar, USRID_START_X+len*MIDDLE_EN_FONT_WIDTH,
						USRID_START_Y+LINE_GAP);

			} /* else drop input because pw that input is too long */
		}
		break;

	case WAIT_ENTER_USRPW         :
		if (KEY_NO_ENTER == keyno) {
			#if 1
			if (is_usr_input_idandpw_correct(input_id, input_pw)) {
			#else
			if (SUCC == webm_p_send_auth_and_proc_rep(input_id, input_pw)) {
			#endif
				proc_key_state = WAIT_ENTER_LOCK_CTR_CMD;
				pw_correct_process();
			} else {
				proc_key_state = WAIT_ENTER_USRID;
				pw_error_process();
			}
			rt_memset(input_id, 0, sizeof(input_id));
			rt_memset(input_pw, 0, sizeof(input_pw));
		} else if (KEY_NO_MODIFY == keyno) {
			rt_memset(input_pw, 0, sizeof(input_pw));
			clr_pw_zone();
		} else  if (keyno>=KEY_NO_NUM0 && keyno<=KEY_NO_NUM9) {
			len = rt_strlen(input_pw);
			if (len < sizeof(input_pw)-1) {
				input_pw[len++] = keyno + '0';
				input_pw[len]   = '\0';
				//print * on lcd;
				dis_one_char('*', PW_START_X+len*MIDDLE_EN_FONT_WIDTH,
						PW_START_Y+LINE_GAP);
			} /* else drop input because pw that input is too long */
		}
		break;

	case WAIT_ENTER_LOCK_CTR_CMD  :
		/* 1--open, 2--close */
		if (1==keyno) {
			elock_open(dummy);
			dis_arrowhead(LOCK_OPEN_ARROWHEAD_X, LOCK_OPEN_ARROWHEAD_Y);
		} else if (2==keyno) {
			elock_close(dummy);
			return_main_interface(); /* return main interface */
		}
		
		break;

	default:
		break;
	}


	return 0;
#endif
}


static enum key_no get_keyno_from_keyval(enum key_value keyval)
{
    int ret = INVALID_KEY;
#if 1
    switch (keyval) {
	case KEYV_R0C0:
	    	ret = KEY_NO_LINE;
		break;
	case KEYV_R0C1:
	    	ret = KEY_NO_ENTER;
		break;
	case KEYV_R0C2:
	    	ret = KEY_NO_MODIFY;
		break;
	case KEYV_R0C3:
	    	ret = KEY_NO_ESC;
		break;

	case KEYV_R1C0:
	    	ret = KEY_NO_DOT;
		break;
	case KEYV_R1C1:
	    	ret = KEY_NO_NUM9;
		break;
	case KEYV_R1C2:
	    	ret = KEY_NO_NUM6;
		break;
	case KEYV_R1C3:
	    	ret = KEY_NO_NUM3;
		break;

	case KEYV_R2C0:
	   	ret = KEY_NO_NUM0;
		break;
	case KEYV_R2C1:
	    	ret = KEY_NO_NUM8;
		break;
	case KEYV_R2C2:
	    	ret = KEY_NO_NUM5;
		break;
	case KEYV_R2C3:
	    	ret = KEY_NO_NUM2;
		break;

	case KEYV_R3C0:
	    	ret = KEY_NO_STAR;
		break;
	case KEYV_R3C1:
	    	ret = KEY_NO_NUM7;
		break;
	case KEYV_R3C2:
	    	ret = KEY_NO_NUM4;
		break;
	case KEYV_R3C3:
		ret = KEY_NO_NUM1;
		break;

	default:
		break;
    }
#else
    switch (keyval) {
	case KEYV_R0C0:
	    	ret = KEY_NO_LINE;
		break;
	case KEYV_R0C1:
	    	ret = KEY_NO_ENTER;
		break;
	case KEYV_R0C2:
	    	ret = KEY_NO_MODIFY;
		break;
	case KEYV_R0C3:
	    	ret = KEY_NO_ESC;
		break;
	case KEYV_R0C4:
	case KEYV_R0C5:
	case KEYV_R0C6:
	case KEYV_R0C7:
	case KEYV_R0C8:
	case KEYV_R0C9:
		break;

	case KEYV_R1C0:
	    	ret = KEY_NO_DOT;
		break;
	case KEYV_R1C1:
	    	ret = KEY_NO_NUM9;
		break;
	case KEYV_R1C2:
	    	ret = KEY_NO_NUM6;
		break;
	case KEYV_R1C3:
	    	ret = KEY_NO_NUM3;
		break;
	case KEYV_R1C4:
	case KEYV_R1C5:
	case KEYV_R1C6:
	case KEYV_R1C7:
	case KEYV_R1C8:
	case KEYV_R1C9:
		break;

	case KEYV_R2C0:
	   	ret = KEY_NO_NUM0;
		break;
	case KEYV_R2C1:
	    	ret = KEY_NO_NUM8;
		break;
	case KEYV_R2C2:
	    	ret = KEY_NO_NUM5;
		break;
	case KEYV_R2C3:
	    	ret = KEY_NO_NUM2;
		break;
	case KEYV_R2C4:
	case KEYV_R2C5:
	case KEYV_R2C6:
	case KEYV_R2C7:
	case KEYV_R2C8:
	case KEYV_R2C9:
		break;

	case KEYV_R3C0:
	    	ret = KEY_NO_STAR;
		break;
	case KEYV_R3C1:
	    	ret = KEY_NO_NUM7;
		break;
	case KEYV_R3C2:
	    	ret = KEY_NO_NUM4;
		break;
	case KEYV_R3C3:
		ret = KEY_NO_NUM1;
		break;

	case KEYV_R3C4:
	case KEYV_R3C5:
	case KEYV_R3C6:
	case KEYV_R3C7:
	case KEYV_R3C8:
	case KEYV_R3C9:
	case KEYV_R4C0:
	case KEYV_R4C1:
	case KEYV_R4C2:
	case KEYV_R4C3:
	case KEYV_R4C4:
	case KEYV_R4C5:
	case KEYV_R4C6:
	case KEYV_R4C7:
	case KEYV_R4C8:
	case KEYV_R4C9:
	case KEYV_R5C0:
	case KEYV_R5C1:
	case KEYV_R5C2:
	case KEYV_R5C3:
	case KEYV_R5C4:
	case KEYV_R5C5:
	case KEYV_R5C6:
	case KEYV_R5C7:
	case KEYV_R5C8:
	case KEYV_R5C9:
	case KEYV_R6C0:
	case KEYV_R6C1:
	case KEYV_R6C2:
	case KEYV_R6C3:
	case KEYV_R6C4:
	case KEYV_R6C5:
	case KEYV_R6C6:
	case KEYV_R6C7:
	case KEYV_R6C8:
	case KEYV_R6C9:
	case KEYV_R7C0:
	case KEYV_R7C1:
	case KEYV_R7C2:
	case KEYV_R7C3:
	case KEYV_R7C4:
	case KEYV_R7C5:
	case KEYV_R7C6:
	case KEYV_R7C7:
	case KEYV_R7C8:
	case KEYV_R7C9:    
		break;

	default:
		break;
    }
#endif
    return ret;
}


#if 1
#include <finsh.h>
int elock(int open)
{
	if (1 == open) {
		elock_open(dummy);
		elock_open_trap();
		printf_syn("elock had open!\n");
	} else if (0 == open) {
		elock_close(dummy);
		elock_close_trap();
		printf_syn("elock had close!\n");
	} else {
		return FAIL;
	}
	
	return SUCC;
}
FINSH_FUNCTION_EXPORT(elock, elock ctrl:0-close 1-open);
#endif



#if 1==USE_TO_7INCH_LCD
/*
 * 国家电网图标 -- 104*104
 */
/*
 * 菜单操作时, 显示界面的刷新
 */
#include <ili9320.h>

#define STATE_GRID_LOGO_COLOR		(0X33AA)
#define STATE_GRID_LOGO_CHAR_COLOR	(0X0aa9)
#define STATE_GRID_LOGO_W 	(104)
#define STATE_GRID_LOGO_H 	(104)


void ra8875_draw_circle(int x, int y, int r, int color, int isfill);
void ra8875_draw_rect(int xs, int ys, int xe, int ye, int color, int isfill);

static void pw_error_process(void)
{
	
	if (AUTH_STYLE_ELOCK_CTRL ==auth_style) {
		clr_body_zone();
		dis_ctr_elock_login();
	} else if (AUTH_STYLE_QUERY_PCOMSUMP==auth_style) {
		clr_body_zone();
		dis_query_pconsump_login();
	}

	return;
}

extern void dis_pconsump_info(char *info_buf, int len);
static void pw_correct_process(void)
{
#if 0
	char buf[16];
	u16 *pu16;
#endif
	keyboard_debug(("func:%s, line:%d\n", __FUNCTION__, __LINE__));
	
	if (AUTH_STYLE_ELOCK_CTRL ==auth_style) {
		clr_body_zone();
		dis_ctr_elock_interface();
	} else if (AUTH_STYLE_QUERY_PCOMSUMP==auth_style) {
#if 0
		pu16 = (u16 *)buf;

		*pu16++ = 0x12;
		*pu16++ = 0x13;
		*pu16++ = 0x22;
		*pu16++ = 0x23;
		*pu16++ = 0x32;
		*pu16++ = 0x33;
	
		clr_body_zone();
		dis_pconsump_info(buf, 16);
#else
#if 0
		webm_p_send_query_pconsump_and_proc_rep(input_id);
#else
		key_webm_mb.mb_ent.cmd = CM_QUERY_PCOMSUMP;
		rt_strncpy(key_webm_mb.mb_ent.com_buf.usr_info.usrid, input_id,
			sizeof(key_webm_mb.mb_ent.com_buf.usr_info.usrid));

		//rt_mb_send_wait(&webm_mb._mb, (rt_uint32_t)&key_webm_mb.mb_ent, RT_WAITING_FOREVER);
#endif	
		
#endif
	}
	
}

void draw_title_line_bg(void)
{
	ra8875_draw_circle(WORK_ZONE_START_X, WORK_ZONE_START_Y+CIRCLE_Y_OFFSET,
			CIRCLE_Y_OFFSET, Green, 1);

	ra8875_draw_rect(WORK_ZONE_START_X, WORK_ZONE_START_Y,
		800-WORK_ZONE_START_X, WORK_ZONE_START_Y+LARGE_CN_FONT_HEIGHT+2*TITLE_GAP, Green, 1);

	ra8875_draw_circle(800-WORK_ZONE_START_X, WORK_ZONE_START_Y+CIRCLE_Y_OFFSET,
			CIRCLE_Y_OFFSET, Green, 1);

}

void dis_title_line(struct tm2lcd_mb_st *textmenu2lcd_mb_p, int len)
{

	init_lcd_mb_st(&textmenu2lcd_mb_p->textmenu_msg);
	textmenu2lcd_mb_p->textmenu_msg.cmd		    = LCD_CMD_DIS_CHAR;
	textmenu2lcd_mb_p->textmenu_msg.start_x 	    = WORK_ZONE_START_X
								+(800-2*WORK_ZONE_START_X -40*len)/2;
	textmenu2lcd_mb_p->textmenu_msg.start_y 	    = WORK_ZONE_START_Y + TITLE_GAP;
	textmenu2lcd_mb_p->textmenu_msg.color		    = Blue2;
	textmenu2lcd_mb_p->textmenu_msg.font_size.font_size = LCD_FONT_SIZE2;
	textmenu2lcd_mb_p->textmenu_msg.buf		    = textmenu2lcd_mb_p->buf_pool;
	textmenu2lcd_mb_p->textmenu_msg.len.len 	    = len;

	rt_mb_send(&lcd_mb._mb, (rt_uint32_t)&textmenu2lcd_mb_p->textmenu_msg);

}


void dis_bottom_line(struct tm2lcd_mb_st *textmenu2lcd_mb_p, int len)
{

	init_lcd_mb_st(&textmenu2lcd_mb_p->textmenu_msg);
	textmenu2lcd_mb_p->textmenu_msg.cmd			= LCD_CMD_DIS_CHAR;
	textmenu2lcd_mb_p->textmenu_msg.start_x 		= BOTTOM_LINE_START_X;
	textmenu2lcd_mb_p->textmenu_msg.start_y 		= BOTTOM_LINE_START_Y;
	textmenu2lcd_mb_p->textmenu_msg.color			= STATE_GRID_LOGO_CHAR_COLOR;
	textmenu2lcd_mb_p->textmenu_msg.font_size.font_size 	= LCD_FONT_SIZE1;
	textmenu2lcd_mb_p->textmenu_msg.buf			= textmenu2lcd_mb_p->buf_pool;
	textmenu2lcd_mb_p->textmenu_msg.len.len 		= len;

	rt_mb_send(&lcd_mb._mb, (rt_uint32_t)&textmenu2lcd_mb_p->textmenu_msg);

}

/* 主内容区 */
void clr_body_zone(void)
{
	ra8875_draw_rect(BODY_START_X, BODY_START_Y, BODY_END_X, BODY_END_Y, White, 1);
}

/* 清除阅读模式区域 */
void clr_read_mode_zone(void)
{
	ra8875_draw_rect(READ_MODE_ZONE_START_X, READ_MODE_ZONE_START_Y,
		READ_MODE_ZONE_END_X, READ_MODE_ZONE_END_Y, White, 1);
}


void dis_body_content_level1(void)
{
	int i, menu_no;
	int line_cnt = 0;

	menu_no = 1;
	/* 1.智能门锁控制 */
	while (is_bit_set(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC))
		rt_thread_delay(get_ticks_of_ms(20));
	i = 0;
	textmenu2lcd_mb.buf_pool[i++] = menu_no+ '0';
	textmenu2lcd_mb.buf_pool[i++] = '.';
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_ZHI;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_NENG;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_MEN;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_SUO;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_KONG;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_ZHI1;
	set_bit(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC);

	init_lcd_mb_st(&textmenu2lcd_mb.textmenu_msg);
	textmenu2lcd_mb.textmenu_msg.cmd		 = LCD_CMD_DIS_CHAR;
	textmenu2lcd_mb.textmenu_msg.start_x 		 = BODY_START_X;
	textmenu2lcd_mb.textmenu_msg.start_y 		 = BODY_START_Y 
			+ line_cnt*MIDDLE_CN_FONT_HEIGHT  + LINE_GAP;
	textmenu2lcd_mb.textmenu_msg.color		 = Blue2;
	textmenu2lcd_mb.textmenu_msg.font_size.font_size = LCD_FONT_SIZE1;
	textmenu2lcd_mb.textmenu_msg.buf		 = textmenu2lcd_mb.buf_pool;
	textmenu2lcd_mb.textmenu_msg.len.len = i;

	rt_mb_send(&lcd_mb._mb, (rt_uint32_t)&textmenu2lcd_mb.textmenu_msg);
	++line_cnt;
	++menu_no;



	/* 2.用电信息自助查询 */
	while (is_bit_set(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC))
		rt_thread_delay(get_ticks_of_ms(20));
	i = 0;
	textmenu2lcd_mb.buf_pool[i++] = menu_no+ '0';
	textmenu2lcd_mb.buf_pool[i++] = '.';
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_YONG;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_DIAN;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_XIN;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_XI;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_ZI;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_ZHU;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_CHA;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_XUN;
	set_bit(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC);

	init_lcd_mb_st(&textmenu2lcd_mb.textmenu_msg);
	textmenu2lcd_mb.textmenu_msg.cmd		 = LCD_CMD_DIS_CHAR;
	textmenu2lcd_mb.textmenu_msg.start_x 		 = BODY_START_X;
	textmenu2lcd_mb.textmenu_msg.start_y 		 = BODY_START_Y
			+ line_cnt*MIDDLE_CN_FONT_HEIGHT + LINE_GAP;
	textmenu2lcd_mb.textmenu_msg.color		 = Blue2;
	textmenu2lcd_mb.textmenu_msg.font_size.font_size = LCD_FONT_SIZE1;
	textmenu2lcd_mb.textmenu_msg.buf		 = textmenu2lcd_mb.buf_pool;
	textmenu2lcd_mb.textmenu_msg.len.len = i;

	rt_mb_send(&lcd_mb._mb, (rt_uint32_t)&textmenu2lcd_mb.textmenu_msg);
	++line_cnt;
	++menu_no;

	/* 3.线路运行状态监测 */
	while (is_bit_set(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC))
		rt_thread_delay(get_ticks_of_ms(20));
	i = 0;
	textmenu2lcd_mb.buf_pool[i++] = menu_no+ '0';
	textmenu2lcd_mb.buf_pool[i++] = '.';
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_XIAN1;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_LU;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_YUN;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_XING;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_ZHUANG;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_TAI;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_JIAN;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_CE;
	set_bit(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC);

	init_lcd_mb_st(&textmenu2lcd_mb.textmenu_msg);
	textmenu2lcd_mb.textmenu_msg.cmd		 = LCD_CMD_DIS_CHAR;
	textmenu2lcd_mb.textmenu_msg.start_x 		 = BODY_START_X;
	textmenu2lcd_mb.textmenu_msg.start_y 		 = BODY_START_Y
			+ line_cnt*MIDDLE_CN_FONT_HEIGHT + LINE_GAP;
	textmenu2lcd_mb.textmenu_msg.color		 = Blue2;
	textmenu2lcd_mb.textmenu_msg.font_size.font_size = LCD_FONT_SIZE1;
	textmenu2lcd_mb.textmenu_msg.buf		 = textmenu2lcd_mb.buf_pool;
	textmenu2lcd_mb.textmenu_msg.len.len = i;

	rt_mb_send(&lcd_mb._mb, (rt_uint32_t)&textmenu2lcd_mb.textmenu_msg);
	++line_cnt;
	++menu_no;

	/* 4.环境参数查询 */
	while (is_bit_set(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC))
		rt_thread_delay(get_ticks_of_ms(20));
	i = 0;
	textmenu2lcd_mb.buf_pool[i++] = menu_no+ '0';
	textmenu2lcd_mb.buf_pool[i++] = '.';
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_HUAN;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_JING1;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_CAN;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_SHU;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_CHA;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_XUN;
	set_bit(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC);

	init_lcd_mb_st(&textmenu2lcd_mb.textmenu_msg);
	textmenu2lcd_mb.textmenu_msg.cmd		 = LCD_CMD_DIS_CHAR;
	textmenu2lcd_mb.textmenu_msg.start_x 		 = BODY_START_X;
	textmenu2lcd_mb.textmenu_msg.start_y 		 = BODY_START_Y
			+ line_cnt*MIDDLE_CN_FONT_HEIGHT + LINE_GAP;
	textmenu2lcd_mb.textmenu_msg.color		 = Blue2;
	textmenu2lcd_mb.textmenu_msg.font_size.font_size = LCD_FONT_SIZE1;
	textmenu2lcd_mb.textmenu_msg.buf		 = textmenu2lcd_mb.buf_pool;
	textmenu2lcd_mb.textmenu_msg.len.len = i;

	rt_mb_send(&lcd_mb._mb, (rt_uint32_t)&textmenu2lcd_mb.textmenu_msg);
	++line_cnt;
	++menu_no;

	/* 5.告警信息查询 */
	while (is_bit_set(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC))
		rt_thread_delay(get_ticks_of_ms(20));
	i = 0;
	textmenu2lcd_mb.buf_pool[i++] = menu_no+ '0';
	textmenu2lcd_mb.buf_pool[i++] = '.';
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_GAO;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_JING2;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_XIN;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_XI;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_CHA;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_XUN;
	set_bit(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC);

	init_lcd_mb_st(&textmenu2lcd_mb.textmenu_msg);
	textmenu2lcd_mb.textmenu_msg.cmd		 = LCD_CMD_DIS_CHAR;
	textmenu2lcd_mb.textmenu_msg.start_x 		 = BODY_START_X;
	textmenu2lcd_mb.textmenu_msg.start_y 		 = BODY_START_Y
			+ line_cnt*MIDDLE_CN_FONT_HEIGHT + LINE_GAP;
	textmenu2lcd_mb.textmenu_msg.color		 = Blue2;
	textmenu2lcd_mb.textmenu_msg.font_size.font_size = LCD_FONT_SIZE1;
	textmenu2lcd_mb.textmenu_msg.buf		 = textmenu2lcd_mb.buf_pool;
	textmenu2lcd_mb.textmenu_msg.len.len = i;

	rt_mb_send(&lcd_mb._mb, (rt_uint32_t)&textmenu2lcd_mb.textmenu_msg);
	++line_cnt;
	++menu_no;

	/* 6.电力宣传 */
	while (is_bit_set(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC))
		rt_thread_delay(get_ticks_of_ms(20));
	i = 0;
	textmenu2lcd_mb.buf_pool[i++] = menu_no+ '0';
	textmenu2lcd_mb.buf_pool[i++] = '.';
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_DIAN;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_LI;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_XUAN1;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_CHUAN;
	set_bit(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC);

	init_lcd_mb_st(&textmenu2lcd_mb.textmenu_msg);
	textmenu2lcd_mb.textmenu_msg.cmd		 = LCD_CMD_DIS_CHAR;
	textmenu2lcd_mb.textmenu_msg.start_x 		 = BODY_START_X;
	textmenu2lcd_mb.textmenu_msg.start_y 		 = BODY_START_Y
			+ line_cnt*MIDDLE_CN_FONT_HEIGHT + LINE_GAP;
	textmenu2lcd_mb.textmenu_msg.color		 = Blue2;//Grey;
	textmenu2lcd_mb.textmenu_msg.font_size.font_size = LCD_FONT_SIZE1;
	textmenu2lcd_mb.textmenu_msg.buf		 = textmenu2lcd_mb.buf_pool;
	textmenu2lcd_mb.textmenu_msg.len.len = i;

	rt_mb_send(&lcd_mb._mb, (rt_uint32_t)&textmenu2lcd_mb.textmenu_msg);
	++line_cnt;
	++menu_no;

}


#define USRID_ZONE_STARTX (BODY_START_X + USRID_POS_OFFSET_X + 24*3+16)
#define USRID_ZONE_STARTY (BODY_START_Y + USRID_POS_OFFSET_Y + LINE_GAP)
#define USRID_ZONE_ENDX   (BODY_END_X)
#define USRID_ZONE_ENDY   (USRID_ZONE_STARTY + MIDDLE_CN_FONT_HEIGHT)

#define PW_ZONE_STARTX (BODY_START_X + PW_POS_OFFSET_X + 24*2+16 )
#define PW_ZONE_STARTY (USRID_ZONE_ENDY + LINE_GAP)
#define PW_ZONE_ENDX   (USRID_ZONE_ENDX)
#define PW_ZONE_ENDY   (PW_ZONE_STARTY + MIDDLE_CN_FONT_HEIGHT)

void clr_usrid_zone(void)
{
	ra8875_draw_rect(USRID_ZONE_STARTX, USRID_ZONE_STARTY, USRID_ZONE_ENDX, USRID_ZONE_ENDY, White, 1);
}

void clr_pw_zone(void)
{
	ra8875_draw_rect(PW_ZONE_STARTX, PW_ZONE_STARTY, PW_ZONE_ENDX, PW_ZONE_ENDY, White, 1);
}

static void dis_login_prompt(void)
{
	int i;
	int line_cnt = 0;

	/* 用户名 */
	while (is_bit_set(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC))
		rt_thread_delay(get_ticks_of_ms(20));
	i = 0;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_YONG;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_HU;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_MING;
	textmenu2lcd_mb.buf_pool[i++] = ':';
	set_bit(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC);

	init_lcd_mb_st(&textmenu2lcd_mb.textmenu_msg);
	textmenu2lcd_mb.textmenu_msg.cmd		 = LCD_CMD_DIS_CHAR;
	textmenu2lcd_mb.textmenu_msg.start_x 		 = BODY_START_X + USRID_POS_OFFSET_X;
	textmenu2lcd_mb.textmenu_msg.start_y 		 = BODY_START_Y + USRID_POS_OFFSET_Y
			+ line_cnt*MIDDLE_CN_FONT_HEIGHT + LINE_GAP;
	textmenu2lcd_mb.textmenu_msg.color		 = Blue2;
	textmenu2lcd_mb.textmenu_msg.font_size.font_size = LCD_FONT_SIZE1;
	textmenu2lcd_mb.textmenu_msg.buf		 = textmenu2lcd_mb.buf_pool;
	textmenu2lcd_mb.textmenu_msg.len.len = i;

	rt_mb_send(&lcd_mb._mb, (rt_uint32_t)&textmenu2lcd_mb.textmenu_msg);
	++line_cnt;

	/* 密码 */
	while (is_bit_set(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC))
		rt_thread_delay(get_ticks_of_ms(20));
	i = 0;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_MI;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_MA;
	textmenu2lcd_mb.buf_pool[i++] = ':';
	set_bit(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC);

	init_lcd_mb_st(&textmenu2lcd_mb.textmenu_msg);
	textmenu2lcd_mb.textmenu_msg.cmd		 = LCD_CMD_DIS_CHAR;
	textmenu2lcd_mb.textmenu_msg.start_x 		 = BODY_START_X + PW_POS_OFFSET_X;
	textmenu2lcd_mb.textmenu_msg.start_y 		 = BODY_START_Y + PW_POS_OFFSET_Y
			+ line_cnt*MIDDLE_CN_FONT_HEIGHT + LINE_GAP;
	textmenu2lcd_mb.textmenu_msg.color		 = Blue2;
	textmenu2lcd_mb.textmenu_msg.font_size.font_size = LCD_FONT_SIZE1;
	textmenu2lcd_mb.textmenu_msg.buf		 = textmenu2lcd_mb.buf_pool;
	textmenu2lcd_mb.textmenu_msg.len.len = i;

	rt_mb_send(&lcd_mb._mb, (rt_uint32_t)&textmenu2lcd_mb.textmenu_msg);

}


void dis_ctr_elock_login(void)
{
	int i;

	dis_arrowhead(USRID_ARROWHEAD_X, USRID_ARROWHEAD_Y);
	
	/* 智能门锁控制 */
	draw_title_line_bg();
	while (is_bit_set(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC))
		rt_thread_delay(get_ticks_of_ms(20));
	i = 0;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_ZHI;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_NENG;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_MEN;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_SUO;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_KONG;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_ZHI1;
	set_bit(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC);
	dis_title_line(&textmenu2lcd_mb, i);

	dis_login_prompt();

	return;
}

void dis_query_pconsump_login(void)
{
	int i;

	dis_arrowhead(USRID_ARROWHEAD_X, USRID_ARROWHEAD_Y);
	
	/* 居民用户 */
	draw_title_line_bg();
	while (is_bit_set(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC))
		rt_thread_delay(get_ticks_of_ms(20));
	i = 0;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_JU1;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_MIN;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_YONG;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_HU;
 	set_bit(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC);
	dis_title_line(&textmenu2lcd_mb, i);

	dis_login_prompt();

	return;
}


void dis_ctr_elock_interface(void)
{
	int i;
	int line_cnt = 0;

	/* 1.打开 */
	while (is_bit_set(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC))
		rt_thread_delay(get_ticks_of_ms(20));
	i = 0;
	textmenu2lcd_mb.buf_pool[i++] = '1';
	textmenu2lcd_mb.buf_pool[i++] = '.';
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_DA;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_KAI;
	set_bit(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC);

	init_lcd_mb_st(&textmenu2lcd_mb.textmenu_msg);
	textmenu2lcd_mb.textmenu_msg.cmd		 = LCD_CMD_DIS_CHAR;
	textmenu2lcd_mb.textmenu_msg.start_x 		 = BODY_START_X + USRID_POS_OFFSET_X;
	textmenu2lcd_mb.textmenu_msg.start_y 		 = BODY_START_Y + USRID_POS_OFFSET_Y
			+ line_cnt*MIDDLE_CN_FONT_HEIGHT + LINE_GAP;
	textmenu2lcd_mb.textmenu_msg.color		 = Blue2;
	textmenu2lcd_mb.textmenu_msg.font_size.font_size = LCD_FONT_SIZE1;
	textmenu2lcd_mb.textmenu_msg.buf		 = textmenu2lcd_mb.buf_pool;
	textmenu2lcd_mb.textmenu_msg.len.len = i;

	rt_mb_send(&lcd_mb._mb, (rt_uint32_t)&textmenu2lcd_mb.textmenu_msg);
	++line_cnt;

	/* 2.关闭 */
	while (is_bit_set(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC))
		rt_thread_delay(get_ticks_of_ms(20));
	i = 0;
	textmenu2lcd_mb.buf_pool[i++] = '2';
	textmenu2lcd_mb.buf_pool[i++] = '.';
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_GUAN;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_BI;
	set_bit(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC);

	init_lcd_mb_st(&textmenu2lcd_mb.textmenu_msg);
	textmenu2lcd_mb.textmenu_msg.cmd		 = LCD_CMD_DIS_CHAR;
	textmenu2lcd_mb.textmenu_msg.start_x 		 = BODY_START_X + USRID_POS_OFFSET_X;
	textmenu2lcd_mb.textmenu_msg.start_y 		 = BODY_START_Y + USRID_POS_OFFSET_Y
			+ line_cnt*MIDDLE_CN_FONT_HEIGHT + LINE_GAP;
	textmenu2lcd_mb.textmenu_msg.color		 = Blue2;
	textmenu2lcd_mb.textmenu_msg.font_size.font_size = LCD_FONT_SIZE1;
	textmenu2lcd_mb.textmenu_msg.buf		 = textmenu2lcd_mb.buf_pool;
	textmenu2lcd_mb.textmenu_msg.len.len = i;

	rt_mb_send(&lcd_mb._mb, (rt_uint32_t)&textmenu2lcd_mb.textmenu_msg);

}

void dis_main_interface(void)
{
	int i;
	
	clr_read_mode_zone();

	draw_title_line_bg();
	i = 0;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_ZHI;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_NENG;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_KONG;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_ZHI1;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_XIANG;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_YING;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_YONG;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_XUAN;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_XIANG1;
	set_bit(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC);
	dis_title_line(&textmenu2lcd_mb, i);

	dis_body_content_level1();

	return;
}


void dis_one_char(int char_id, int x, int y)
{
	/* 用户名 */
	while (is_bit_set(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC))
		rt_thread_delay(get_ticks_of_ms(20));
	textmenu2lcd_mb.buf_pool[0] = char_id;
	set_bit(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC);

	init_lcd_mb_st(&textmenu2lcd_mb.textmenu_msg);
	textmenu2lcd_mb.textmenu_msg.cmd		 = LCD_CMD_DIS_CHAR;
	textmenu2lcd_mb.textmenu_msg.start_x 		 = x;
	textmenu2lcd_mb.textmenu_msg.start_y 		 = y;
	textmenu2lcd_mb.textmenu_msg.color		 = Blue2;
	textmenu2lcd_mb.textmenu_msg.font_size.font_size = LCD_FONT_SIZE1;
	textmenu2lcd_mb.textmenu_msg.buf		 = textmenu2lcd_mb.buf_pool;
	textmenu2lcd_mb.textmenu_msg.len.len = 1;

	rt_mb_send(&lcd_mb._mb, (rt_uint32_t)&textmenu2lcd_mb.textmenu_msg);

	return;
}

void dis_arrowhead(int x, int y)
{

	/* -->, 16*24 dot */
	while (is_bit_set(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC))
		rt_thread_delay(get_ticks_of_ms(20));
	textmenu2lcd_mb.buf_pool[0] = '-';
	textmenu2lcd_mb.buf_pool[1] = '-';
	textmenu2lcd_mb.buf_pool[2] = '>';
	set_bit(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC);

	init_lcd_mb_st(&textmenu2lcd_mb.textmenu_msg);
	textmenu2lcd_mb.textmenu_msg.cmd		 = LCD_CMD_DIS_CHAR;
	textmenu2lcd_mb.textmenu_msg.start_x 		 = x;
	textmenu2lcd_mb.textmenu_msg.start_y 		 = y;
	textmenu2lcd_mb.textmenu_msg.color		 = Blue2;
	textmenu2lcd_mb.textmenu_msg.font_size.font_size = LCD_FONT_SIZE1;
	textmenu2lcd_mb.textmenu_msg.buf		 = textmenu2lcd_mb.buf_pool;
	textmenu2lcd_mb.textmenu_msg.len.len = 3;

	rt_mb_send(&lcd_mb._mb, (rt_uint32_t)&textmenu2lcd_mb.textmenu_msg);

	return;
}

void clr_arrowhead(int x, int y)
{
	ra8875_draw_rect(x, y, x+16*3, y+24, White, 1);

	return;
}

void dis_top_frame(void)
{
	int i;
	
	lcd_draw_homochromy_color_pic(2, 2, 104, 104, STATE_GRID_LOGO_COLOR, White,
			state_grid_logo_104x104bmp);


	/* 国家电网 */
	while (is_bit_set(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC))
		rt_thread_delay(get_ticks_of_ms(20));
	i = 0;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_GUO;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_JIA;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_DIAN;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_WANG;
	set_bit(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC);

	init_lcd_mb_st(&textmenu2lcd_mb.textmenu_msg);
	textmenu2lcd_mb.textmenu_msg.cmd		 = LCD_CMD_DIS_CHAR;
	textmenu2lcd_mb.textmenu_msg.start_x 		 = STATE_GRID_LOGO_W + 10;
	textmenu2lcd_mb.textmenu_msg.start_y 		 = 5;
	textmenu2lcd_mb.textmenu_msg.color		 = STATE_GRID_LOGO_CHAR_COLOR;
	textmenu2lcd_mb.textmenu_msg.font_size.font_size = LCD_FONT_SIZE2;
	textmenu2lcd_mb.textmenu_msg.buf		 = textmenu2lcd_mb.buf_pool;
	textmenu2lcd_mb.textmenu_msg.len.len = i;

	rt_mb_send(&lcd_mb._mb, (rt_uint32_t)&textmenu2lcd_mb.textmenu_msg);


	/* 宁夏电力公司 */
	while (is_bit_set(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC))
		rt_thread_delay(get_ticks_of_ms(20));
	i = 0;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_NING;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_XIA;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_DIAN;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_LI;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_GONG;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_SI;
	set_bit(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC);

	init_lcd_mb_st(&textmenu2lcd_mb.textmenu_msg);
	textmenu2lcd_mb.textmenu_msg.cmd		 = LCD_CMD_DIS_CHAR;
	textmenu2lcd_mb.textmenu_msg.start_x 		 = STATE_GRID_LOGO_W + 10;
	textmenu2lcd_mb.textmenu_msg.start_y 		 = 5 + LARGE_CN_FONT_HEIGHT + 10;
	textmenu2lcd_mb.textmenu_msg.color		 = STATE_GRID_LOGO_CHAR_COLOR;
	textmenu2lcd_mb.textmenu_msg.font_size.font_size = LCD_FONT_SIZE1;
	textmenu2lcd_mb.textmenu_msg.buf		 = textmenu2lcd_mb.buf_pool;
	textmenu2lcd_mb.textmenu_msg.len.len = i;

	rt_mb_send(&lcd_mb._mb, (rt_uint32_t)&textmenu2lcd_mb.textmenu_msg);
	
}
void return_main_interface(void)
{
	proc_key_state = WAIT_ENTER_LEVEL1_MENU_NO;
	auth_style     = AUTH_STYLE_INVALID;
	dis_main_interface();
	rt_memset(input_id, 0, sizeof(input_id));
	rt_memset(input_pw, 0, sizeof(input_pw));

	return;
}


void dis_publicity_info(char *pch, int len)
{
	int i, byte_cnt;
	const char *p;

	if (NULL==pch) {
		printf_syn("publicity info pointer is NULL!\n");
		return;
	}
	
	/* 电力宣传 */
	draw_title_line_bg();
	while (is_bit_set(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC))
		rt_thread_delay(get_ticks_of_ms(20));
	i = 0;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_DIAN;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_LI;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_XUAN1;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_CHUAN;
	set_bit(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC);
	dis_title_line(&textmenu2lcd_mb, i);


	byte_cnt = 0;
	p = pch;
	while ('\n'!=*p && '\0'!=*p) {
		++byte_cnt;
		++p;
	}

	/* 内容 */
	while (is_bit_set(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC))
		rt_thread_delay(get_ticks_of_ms(20));
	set_bit(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC);

	init_lcd_mb_st(&textmenu2lcd_mb.textmenu_msg);
	textmenu2lcd_mb.textmenu_msg.cmd	= LCD_CMD_DIS_GB2312_CHAR;
	textmenu2lcd_mb.textmenu_msg.start_x	= READ_MODE_ZONE_START_X + 
						+ (READ_MODE_ZONE_END_X-READ_MODE_ZONE_START_X-8*byte_cnt)/2;
	textmenu2lcd_mb.textmenu_msg.start_y	= READ_MODE_ZONE_START_Y + LINE_GAP;
	textmenu2lcd_mb.textmenu_msg.color	= Blue2;
	textmenu2lcd_mb.textmenu_msg.buf	= (font_id_t *)pch;
	textmenu2lcd_mb.textmenu_msg.len.len    = len;

	rt_mb_send(&lcd_mb._mb, (rt_uint32_t)&textmenu2lcd_mb.textmenu_msg);

	
	return;
}

/*
 * 临时处理文本菜单, 简单的开闭锁
 */
int is_usr_input_idandpw_correct(char *id, char *pw)
{
	char key_pwd[PWD_LEN];

	if ((AUTH_STYLE_ELOCK_CTRL==auth_style) || (AUTH_STYLE_QUERY_PCOMSUMP == auth_style)) {
		get_openlock_pw(key_pwd, sizeof(key_pwd));
		keyboard_debug(("key_pw:%s, inpu_pw:%s\n", key_pwd, pw));

		return (0 == rt_strncmp(key_pwd, input_pw, PWD_LEN));
#if 0
	} else if (AUTH_STYLE_QUERY_PCOMSUMP == auth_style) {
		#if 0
		return TRUE;
		#else
		if (SUCC == webm_p_send_auth_and_proc_rep(input_id, input_pw)) {
			return TRUE;
		} else {
			//auth_error_process();
		}
		#endif
	} else {
		;
#endif
	}

	return FALSE;

}



#endif


#if 0==USE_KEY_CTR_CHIP
/*
 * PE[7:10]	-- ra8875.KIN[0:3], column
 * PE[11:14]	-- ra8875.KOUT[0:3], row
 */
#define KEY_ROW_BITS (GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14)
#define KEY_COL_BITS (GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10)
#define KEY_SHIFT_BITS 7

#define key_row_output_0(port) gpio_bits_reset(port, KEY_ROW_BITS)
#define key_col_output_0(port) gpio_bits_reset(port, KEY_COL_BITS)

/*
 * 	C0	C1	C2	C3
 * R0	
 * R1	
 * R2	
 * R3	
 * 
 * R[3:0]C[3:0]
 * 
 * 	KEYV_R0C0 = 0xee,
 * 	KEYV_R0C1 = 0xed,
 * 	KEYV_R0C2 = 0xeb,
 * 	KEYV_R0C3 = 0xe7,
 * 	KEYV_R1C0 = 0xde,
 * 	KEYV_R1C1 = 0xdd,
 * 	KEYV_R1C2 = 0xdb,
 * 	KEYV_R1C3 = 0xd7,
 * 	KEYV_R2C0 = 0xbe,
 * 	KEYV_R2C1 = 0xbd,
 * 	KEYV_R2C2 = 0xbb,
 * 	KEYV_R2C3 = 0xb7,
 * 	KEYV_R3C0 = 0x7e,
 * 	KEYV_R3C1 = 0x7d,
 * 	KEYV_R3C2 = 0x7b,
 * 	KEYV_R3C3 = 0x77,
 */ 
/*
 * 如果有键按下时返回1, 否则返回0
 */
int is_matrixkey_had_hit(void)
{
	int temp;

	set_rowin_colout();
	key_col_output_0(GPIOE);

	/* 延迟 */
	temp = 3;
	while (temp)
		--temp;
	
	temp = GPIOE->IDR;
	temp &= KEY_ROW_BITS;

	return (temp ^ KEY_ROW_BITS);
}

/*
 * NOTE: 该函数与is_matrixkey_had_hit()函数有关联!!!
 * 如果没有抖动, 确实有键按下时返回1, 否则返回0
 */
int do_matrixkey_debounce(void)
{
	int temp;
	
	rt_thread_delay(get_ticks_of_ms(20));

	temp = GPIOE->IDR;
	temp &= KEY_ROW_BITS;

	return (temp ^ KEY_ROW_BITS);
}

/*
 * 
 */
int get_matrixkey_value(void)
{
	unsigned int keyval, temp;

	/* 判断是否有键按下时, 已设置为 row input, col output */
	keyval = GPIOE->IDR;
	keyval &= KEY_ROW_BITS;

	set_rowout_colin();
	key_row_output_0(GPIOE);
	/* 延迟 */
	temp = 3;
	while (temp)
		--temp;
	
	temp = GPIOE->IDR;
	temp &= KEY_COL_BITS;

	/*  */
	keyval |= temp;
	keyval >>= KEY_SHIFT_BITS;

	return keyval;
}

/*
 * 
 */
void wait_matrixkey_release(void)
{
	while (is_matrixkey_had_hit())
		rt_thread_delay(get_ticks_of_ms(300));

	return;
}

/*
 * PE[7:10]	-- ra8875.KIN[0:3], column
 * PE[11:14]	-- ra8875.KOUT[0:3], row
 */
/*
 * I/O pin setting -- row input pin, column output pin
 */
void set_rowin_colout(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* row */
	GPIO_InitStructure.GPIO_Pin	= GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* column */
	GPIO_InitStructure.GPIO_Pin	= GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	return;
}

/*
 * PE[7:10]	-- ra8875.KIN[0:3], column
 * PE[11:14]	-- ra8875.KOUT[0:3], row
 */
/*
 * I/O pin setting -- row output pin, column input pin
 */
void set_rowout_colin(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* row */
	GPIO_InitStructure.GPIO_Pin	= GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* column */
	GPIO_InitStructure.GPIO_Pin	= GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	return;
}

#endif
