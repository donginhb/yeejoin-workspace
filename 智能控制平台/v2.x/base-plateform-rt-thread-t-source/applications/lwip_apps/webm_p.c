#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"

#include "webm_p.h"

#include <board.h>
#include <stm32f10x.h>
#include <keyboard.h>
#include <ili9320_fontid.h>
#include <ili9320.h>
#include <misc_lib.h>
#include <am2301.h>

#if LWIP_NETCONN

#ifndef WEBM_P_DEBUG
#define WEBM_P_DEBUG         LWIP_DBG_ON
#endif

#define WEBM_P_S_THREAD_STACKSIZE (0x400)
#define WEBM_P_C_THREAD_STACKSIZE (0x300)
#define WEBM_P_THREAD_PRIO	(0x15)

#if 1
rt_uint32_t 	nms_ip;
u16 		nms_port;
#else
#define WEBM_P_SERVER_PORT (1234)
#define WEBM_P_IP   "172.24.4.250"
#endif

struct publiciy_st publicity_from_webm;
struct webm_ipc_mb webm_mb;

#define WEBM_P_PRINT_DEBUG(x) printf_syn x
#define WEBM_P_PRINT_LOG(x)   printf_syn x


static int webm_p_data_proc(struct netconn *conn);
static void webm_p_thread_entry(void *arg);
static int webm_p_cmd_analysis(struct netconn *conn, struct webm_data_st *rec_data);
static int webm_p_send_rep(struct netconn *conn, int cmd, int result);
static int webm_p_msg_package(struct webm_data_st *send_data, char *data_buf, int buflen);
static int webm_p_send_auth(struct netconn *conn, char *id, char *pw);
static int webm_p_send_query_pconsump(struct netconn *conn, char *id);
static void dis_pconsump_info_body(char *info_buf, int len);

static void webm_p_client_thread_entry(void *arg);
static void webm_client_msg_proc(struct webm_mb_buf *pwebmb);

static int webm_p_send_warning_info_and_proc_rep(int ill_open, int tmp_orun, int rh_orun);
static int webm_p_send_warninginfo(struct netconn *conn, int ill_open, int tmp_orun, int rh_orun);

extern int set_tmprh_limen(int temp_h, int temp_l, int rh_h, int rh_l);

/** Initialize the HTTP server (start its thread) */
void webm_p_thread_init(void)
{
	struct nms_if_info_st nms_if;

	publicity_from_webm.index = 0;
	publicity_from_webm.buf = NULL;

	read_syscfgdata_tbl(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
	nms_ip   = nms_if.nms_ip;
	nms_port = nms_if.port;
	
	if (NULL == sys_thread_new("webm_s", webm_p_thread_entry, NULL,
			WEBM_P_S_THREAD_STACKSIZE, WEBM_P_THREAD_PRIO)) {
		WEBM_P_PRINT_LOG(("creat webm server fail!\n"));
	} else {
		WEBM_P_PRINT_DEBUG(("creat webm server succ!\n"));
	}

	if (NULL == sys_thread_new("webm_c", webm_p_client_thread_entry, NULL,
			WEBM_P_C_THREAD_STACKSIZE, WEBM_P_THREAD_PRIO)) {
		WEBM_P_PRINT_LOG(("creat webm client fail!\n"));
	} else {
		WEBM_P_PRINT_DEBUG(("creat webm client succ!\n"));
	}
}

int webm_p_send_auth_and_proc_rep(char *id, char *pw)
{
	struct netconn *conn;
	err_t err;
	ip_addr_t addr;
	int ret = FAIL;
	
	/* Create a new TCP connection handle */
	conn = netconn_new(NETCONN_TCP);
	LWIP_ERROR("webm_p_thread: invalid conn", (conn != NULL), return ret;);

	addr.addr = (nms_ip);
	WEBM_P_PRINT_DEBUG(("tcp connect to ip:0x%x, port:%d\n", addr.addr, nms_port));
	err = netconn_connect(conn, &addr, nms_port);

	WEBM_P_PRINT_DEBUG(("fun:%s, line:%d, err:%d \n", __FUNCTION__, __LINE__, err));
        if (err == ERR_OK) {
        	if (FAIL == webm_p_send_auth(conn, id, pw)) {
        		WEBM_P_PRINT_LOG(("send auth fail!\n"));
        		goto ret_entry;
        	}
#if 1
		ret = webm_p_data_proc(conn);
#else
        	nbuf = netbuf_new();
        	if (NULL != nbuf) {
        		if (NULL!=netbuf_alloc(nbuf, 100)) {
        			if (ERR_OK == (err=netconn_recv(conn, &nbuf)))
					webm_p_data_proc(conn);
				else
					printf_syn("netconn_recv err(%d)!\n", err);
				netbuf_free(nbuf);
			}
			netbuf_delete(nbuf);
		} else {
			printf_syn("fun:%s, netbuf_new fail!\n",  __FUNCTION__);
		}
#endif
        } else {
        	WEBM_P_PRINT_LOG(("send auth. creat netconn err(%d)!\n", err));
        }

ret_entry:
	netconn_delete(conn);

	return ret;
}

static int webm_p_send_auth(struct netconn *conn, char *id, char *pw)
{
	char buf[4+sizeof(struct webm_data_st)+4];
	struct webm_data_st send_data;

	rt_memset(&send_data, 0, sizeof(send_data));
	send_data.cmd		= htonl(CM_AUTH_REQ);
	rt_strncpy(send_data.usr_id, id, USR_INFO_MAX);
	rt_strncpy(send_data.usr_pw, pw, USR_INFO_MAX);
	if (SUCC == webm_p_msg_package(&send_data, buf, sizeof(buf))) {
		if (ERR_OK != netconn_write(conn, buf, sizeof(buf), NETCONN_COPY)) {
			WEBM_P_PRINT_LOG(("func:%s, netconn_write() fail!\n"));
			return FAIL;
		}
	} else {
		WEBM_P_PRINT_LOG(("func:%s, msg package fail!\n"));
		return FAIL;
	}

	return SUCC;	
}


/* static struct ip_addr localip; */
/** The main function, never returns! */
static void webm_p_thread_entry(void *arg)
{
    struct netconn *conn, *newconn;
    err_t err;
    
    LWIP_UNUSED_ARG(arg);

    /* Create a new TCP connection handle */
    conn = netconn_new(NETCONN_TCP);
    LWIP_ERROR("webm_p_thread_entry: invalid conn", (conn != NULL), return;);
#if 0
    /* Bind to port 1234  with default IP address */
    netconn_bind(conn, NULL, 1234);
#else
    /* localip.addr = DEFAULT_IPADDR; */
    netconn_bind(conn, IP_ADDR_ANY, nms_port);  /* David, set IP */
#endif
    /* Put the connection into LISTEN state */
    netconn_listen(conn);

    do {
        err = netconn_accept(conn, &newconn);

        if (err == ERR_OK) {
            webm_p_data_proc(newconn);
            netconn_delete(newconn);
        }
    } while(err == ERR_OK);
    LWIP_DEBUGF(WEBM_P_DEBUG,
    		("webm_p_thread_entry: netconn_accept received error %d, shutting down",
            	err));
    netconn_close(conn);
    netconn_delete(conn);
}



static int webm_p_data_proc(struct netconn *conn)
{
	struct netbuf *inbuf;
	unsigned char *data;
	u16_t buflen;
	err_t err;

	int recv_cnt, databuf_cnt;
	int s_tag, e_tag;

	if (NULL == conn)
		return FAIL;

	recv_cnt = 0;
	while (ERR_OK == (err = netconn_recv(conn, &inbuf)) ) {
		//WEBM_P_PRINT_DEBUG(("func:%s(), recv_cnt:%d!\n", __FUNCTION__, recv_cnt));
		databuf_cnt = 0;
		do {
			int had_next_buf, is_publicity;
			
			netbuf_data(inbuf, (void**)&data, &buflen);

			if (0==recv_cnt &&  0==databuf_cnt) {
				if (CM_REV_PUBLICITY_INFO == ntohl(((struct webm_data_st*)(&data[4]))->cmd)) {
					is_publicity = 1;
					publicity_from_webm.index = 0;
				} else {
					is_publicity = 0;
				}
			}

			if (0 == is_publicity) {
				break;
			} else {/* 只有CM_REV_PUBLICITY_INFO才可能到这里 */
				if (NULL == publicity_from_webm.buf) {
					publicity_from_webm.buf = rt_malloc(4096);
					if (NULL == publicity_from_webm.buf)
						break;
				}

				if (publicity_from_webm.index+buflen < 4096) {
					rt_memcpy(publicity_from_webm.buf+publicity_from_webm.index, data, buflen);
					publicity_from_webm.index += buflen;
				}

				had_next_buf = netbuf_next(inbuf);
				if (had_next_buf < 0) {
					data   = publicity_from_webm.buf;
					buflen = publicity_from_webm.index;
					break;
				}
			}
			++databuf_cnt;
		} while (1);

		s_tag = ntohl(*(int *)data);
		e_tag = ntohl(*(int *)(data+buflen-4));
		
		if (START_FLAG_VALUE==s_tag && END_FLAG_VALUE==e_tag) {
			WEBM_P_PRINT_DEBUG(("\nrecv_cnt:%d, had recv %d bytes, will analysis cmd!\n",
				recv_cnt+1, buflen));
			webm_p_cmd_analysis(conn, (struct webm_data_st*)&data[4]);
			netbuf_delete(inbuf);
			break;
		} else { /*  */
			WEBM_P_PRINT_DEBUG(("\nrecv_cnt:%d, had recv %d bytes!", recv_cnt+1, buflen));
		}

		/* Delete the buffer (netconn_recv gives us ownership,
		so we have to make sure to deallocate the buffer) */
		netbuf_delete(inbuf);
		++recv_cnt;
	}

	WEBM_P_PRINT_DEBUG(("webm_p_data_proc() over, will close connect!\nnetconn_recv errno(%d)!\n", err));
	/* Close the connection */
	netconn_close(conn);

	return SUCC;
}


static int webm_p_cmd_analysis(struct netconn *conn, struct webm_data_st *rec_data)
{
	int cmd, rep_arg;
	//unsigned char *pch;

#if 0
	rec_data->usr_id[USR_INFO_MAX-1] = 0;
	rec_data->usr_pw[USR_INFO_MAX-1] = 0;
	printf_syn("cmd:%d, arg:%d, id:%s, pw:%s. datalen:%d\n", ntohl(rec_data->cmd), ntohl(rec_data->rep_arg),
			rec_data->usr_id, rec_data->usr_pw, sizeof(struct webm_data_st)+4+4);
#endif
	cmd = ntohl(rec_data->cmd);
	rep_arg = ntohl(rec_data->rep_arg);
	WEBM_P_PRINT_DEBUG(("%s(): cmd is %d, rep:%d\n", __FUNCTION__, cmd, rep_arg));
	switch (cmd) {
	case CM_OPEN_LOCK:
		WEBM_P_PRINT_LOG(("CM_OPEN_LOCK\n"));
		elock_open(dummy);
		webm_p_send_rep(conn, CM_OPEN_LOCK_REP, SUCC);
		break;

	case CM_CLOSE_LOCK:
		WEBM_P_PRINT_LOG(("CM_CLOSE_LOCK\n"));
		elock_close(dummy);
		webm_p_send_rep(conn, CM_CLOSE_LOCK_REP, SUCC);
		break;
	case CM_GET_TEMP_RH:
		webm_p_send_rep(conn, CM_GET_TEMP_RH_REP, SUCC);
		break;

	case CM_QUERY_EQ_RUNNING_STATE:
		if (SUCC==rep_arg) {
		} else {
		}
		break;

	case CM_REGISTER_TERMINAL:
		webm_p_send_rep(conn, CM_REGISTER_TERMINAL_REP, SUCC);
		break;

	case CM_SET_TMP_RH_LIMEN:
	{
		int temp_h, temp_l, rh_h, rh_l;
		u16 *pt, t;

		pt = (u16 *)(rec_data->usr_id);
		t = ntohs(*pt++);
		temp_l = (s16)t;
		t = ntohs(*pt);
		temp_h = (s16)t;
		
		pt = (u16 *)(rec_data->usr_pw);
		t = ntohs(*pt++);
		rh_l = t;
		t = ntohs(*pt);
		rh_h = t;

		set_tmprh_limen(temp_h, temp_l, rh_h, rh_l);

		webm_p_send_rep(conn, CM_SET_TMP_RH_LIMEN_REP, SUCC);
	}
		break;

	case CM_AUTH_REP:
		if (AUTH_STYLE_ELOCK_CTRL ==auth_style) {
			if (SUCC==rep_arg)
				elock_open(dummy);
			else
				elock_close(dummy);
		} else if (AUTH_STYLE_QUERY_PCOMSUMP==auth_style) {
			#if 1==USE_TO_7INCH_LCD
			if (SUCC==rep_arg) {
				webm_p_send_query_pconsump_and_proc_rep(input_id);
			} else {
				clr_body_zone();
				dis_query_pconsump_login();
			}
			#endif
		
		}
		break;

	case CM_REV_PUBLICITY_INFO:
		/* 数据已拷贝至publicity_from_webm.buf */
		webm_p_send_rep(conn, CM_REV_PUBLICITY_INFO_REP, SUCC);
		break;

	case CM_QUERY_PCOMSUMP_REP:
		#if 1==USE_TO_7INCH_LCD
		if (SUCC==rep_arg) {
			clr_body_zone();
			dis_pconsump_info(rec_data->usr_id, 12);
		} else {
		}
		#endif

		break;

	case CM_QUERY_EQ_RUNNING_STATE_REP:
		break;

	case CM_WARNNING_IFNO_NOTIFI_REP:
		break;
	
	default:
		WEBM_P_PRINT_LOG(("error %s(): cmd is %d\n", __FUNCTION__, cmd));
		webm_p_send_rep(conn, cmd, FAIL);
		return FAIL;
	}

	return SUCC;	
}


static int webm_p_send_rep(struct netconn *conn, int cmd, int result)
{
	char buf[4+sizeof(struct webm_data_st)+4];
	struct webm_data_st send_data;
	u16 *p;
	int temp;

	rt_memset(&send_data, 0, sizeof(send_data));
	send_data.cmd		= htonl(cmd);
	send_data.rep_arg	= htonl(result);

	switch (cmd) {
	case CM_GET_TEMP_RH_REP:
		if (tmp_from_am2301 & (1<<15))
			temp = -(tmp_from_am2301 & ~(1<<15));
		else
			temp = tmp_from_am2301;

		p = (u16 *)send_data.usr_id;
		*p = htons(temp);

		p = (u16 *)send_data.usr_pw;
		*p = htons(rh_from_am2301);
		break;

	case CM_QUERY_EQ_RUNNING_STATE_REP:
		send_data.usr_id[0] 	= 2;
		send_data.usr_id[1] 	= 2;
	
		break;
/*
	case CM_SET_TMP_RH_LIMEN_REP:
		break;
*/
	default:
		break;
	}

	if (SUCC == webm_p_msg_package(&send_data, buf, sizeof(buf)))
		netconn_write(conn, buf, sizeof(buf), NETCONN_COPY);
	else
		WEBM_P_PRINT_LOG(("package fail!\n"));

	WEBM_P_PRINT_DEBUG(("%s() over! cmd:0x%x\n", __FUNCTION__, cmd));
	return 0;
}

static int webm_p_msg_package(struct webm_data_st *send_data, char *data_buf, int buflen)
{
	int i;
	struct webm_data_st *p;

	if ((buflen < (4+sizeof(struct webm_data_st)+4)) || NULL==send_data || NULL==data_buf)
		return FAIL;

	i = 0;
	data_buf[i++] = 0x1F;
	data_buf[i++] = 0x55;
	data_buf[i++] = 0x3f;
	data_buf[i++] = 0x5a;

	p 		= (struct webm_data_st *)(data_buf+4);
	#if 0
	p->cmd		= htonl(send_data->cmd);
	p->rep_arg	= htonl(send_data->rep_arg);
	rt_strncpy(p->usr_id, send_data->usr_id, USR_INFO_MAX);
	rt_strncpy(p->usr_pw, send_data->usr_pw, USR_INFO_MAX);
	#else
	rt_memcpy(p, send_data, sizeof(*send_data));
	#endif

	i = 4 + sizeof(struct webm_data_st);
	data_buf[i++] = 0xAF;
	data_buf[i++] = 0x25;
	data_buf[i++] = 0xcf;
	data_buf[i++] = 0x6a;

	return SUCC;
}

#if 1==USE_TO_7INCH_LCD

void webm_p_send_query_pconsump_and_proc_rep(char *id)
{
	struct netconn *conn;
	err_t err;
	ip_addr_t addr;
	int ret = SUCC;
	
	/* Create a new TCP connection handle */
	conn = netconn_new(NETCONN_TCP);

	if (NULL == conn) {
		WEBM_P_PRINT_LOG(("webm_p_thread: invalid conn\n"));
		ret=1;
		goto ret_entry;
	}

	addr.addr = (nms_ip);
	WEBM_P_PRINT_DEBUG(("tcp connect to ip:0x%x, port:%d\n", addr.addr, nms_port));
	err = netconn_connect(conn, &addr, nms_port);

	WEBM_P_PRINT_DEBUG(("fun:%s, line:%d, err:%d \n", __FUNCTION__, __LINE__, err));
        if (err == ERR_OK) {
        	if (FAIL == webm_p_send_query_pconsump(conn, id)) {
        		WEBM_P_PRINT_LOG(("send query power consumption fail!\n"));
        		ret = 1;
        		goto ret_entry;
        	}

#if 1
		ret = webm_p_data_proc(conn);
#else
        	nbuf = netbuf_new();
        	if (NULL != nbuf) {
        		if (NULL!=netbuf_alloc(nbuf, 100)) {
        			if (ERR_OK == (err=netconn_recv(conn, &nbuf))) {
					webm_p_data_proc(conn);
				} else {
					printf_syn("netconn_recv err(%d)!\n", err);
					ret = 1;
				}
				netbuf_free(nbuf);
			} else {
				ret = 1;
			}
			netbuf_delete(nbuf);
		} else {
			ret = 1;
		}
#endif
        } else {
        	WEBM_P_PRINT_LOG(("send query. creat netconn err(%d)!\n", err));
        	ret = 1;
        }

ret_entry:
	netconn_delete(conn);
	if (0 != ret)
        	return_main_interface();

	return;
}


static int webm_p_send_query_pconsump(struct netconn *conn, char *id)
{
	char buf[4+sizeof(struct webm_data_st)+4];
	struct webm_data_st send_data;

	rt_memset(&send_data, 0, sizeof(send_data));
	send_data.cmd		= htonl(CM_QUERY_PCOMSUMP);
	rt_strncpy(send_data.usr_id, id, USR_INFO_MAX);
	send_data.usr_pw[0] = 0;
	if (SUCC == webm_p_msg_package(&send_data, buf, sizeof(buf))) {
		if (ERR_OK != netconn_write(conn, buf, sizeof(buf), NETCONN_COPY)) {
			WEBM_P_PRINT_LOG(("func:%s, netconn_write() fail!\n"));
			return FAIL;
		}
	} else {
		WEBM_P_PRINT_LOG(("func:%s, msg package fail!\n"));
		return FAIL;
	}

	return SUCC;	
}


void dis_pconsump_info(char *info_buf, int len)
{
	int i;
	
	/* 居民用电信息 */
	draw_title_line_bg();
	while (is_bit_set(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC))
		rt_thread_delay(get_ticks_of_ms(20));
	i = 0;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_JU1;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_MIN;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_YONG;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_DIAN;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_XIN;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_XI;
	set_bit(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC);
	dis_title_line(&textmenu2lcd_mb, i);

	dis_pconsump_info_body(info_buf, len);
}

static void dis_pconsump_info_body(char *info_buf, int len)
{
	int i, line_cnt = 0;
	int info_index  = 0;
	int temp;
	char buf[16], *pch;
	
	clr_body_zone();

	/* 1.当天用电信息 */
	while (is_bit_set(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC))
		rt_thread_delay(get_ticks_of_ms(20));
	i = 0;
	textmenu2lcd_mb.buf_pool[i++] = '1';
	textmenu2lcd_mb.buf_pool[i++] = '.';
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_DANG;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_TIAN;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_YONG;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_DIAN;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_XIN;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_XI;
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

	/* 当天0-6时用电量:xxx度 */
	while (is_bit_set(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC))
		rt_thread_delay(get_ticks_of_ms(20));
	i = 0;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_DANG;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_TIAN;
	textmenu2lcd_mb.buf_pool[i++] = '0';
	textmenu2lcd_mb.buf_pool[i++] = '-';
	textmenu2lcd_mb.buf_pool[i++] = '6';
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_SHI2;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_YONG;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_DIAN;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_LIANG;
	textmenu2lcd_mb.buf_pool[i++] = ':';

	temp = info_buf[info_index++];
	temp = (temp<<8) | info_buf[info_index++];
	i2str(buf, temp);
	pch = buf;
	while ('\0' != *pch)
		textmenu2lcd_mb.buf_pool[i++] = *pch++;

	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_DU;
	set_bit(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC);

	init_lcd_mb_st(&textmenu2lcd_mb.textmenu_msg);
	textmenu2lcd_mb.textmenu_msg.cmd		 = LCD_CMD_DIS_CHAR;
	textmenu2lcd_mb.textmenu_msg.start_x 		 = BODY_START_X + INDENT1_SIZE;
	textmenu2lcd_mb.textmenu_msg.start_y 		 = BODY_START_Y
			+ line_cnt*MIDDLE_CN_FONT_HEIGHT + LINE_GAP;
	textmenu2lcd_mb.textmenu_msg.color		 = Blue2;
	textmenu2lcd_mb.textmenu_msg.font_size.font_size = LCD_FONT_SIZE1;
	textmenu2lcd_mb.textmenu_msg.buf		 = textmenu2lcd_mb.buf_pool;
	textmenu2lcd_mb.textmenu_msg.len.len = i;

	rt_mb_send(&lcd_mb._mb, (rt_uint32_t)&textmenu2lcd_mb.textmenu_msg);
	++line_cnt;

	/* 当天6-12时用电量:xxx度 */
	while (is_bit_set(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC))
		rt_thread_delay(get_ticks_of_ms(20));
	i = 0;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_DANG;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_TIAN;
	textmenu2lcd_mb.buf_pool[i++] = '6';
	textmenu2lcd_mb.buf_pool[i++] = '-';
	textmenu2lcd_mb.buf_pool[i++] = '1';
	textmenu2lcd_mb.buf_pool[i++] = '2';
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_SHI2;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_YONG;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_DIAN;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_LIANG;
	textmenu2lcd_mb.buf_pool[i++] = ':';

	temp = info_buf[info_index++];
	temp = (temp<<8) | info_buf[info_index++];
	i2str(buf, temp);
	pch = buf;
	while ('\0' != *pch)
		textmenu2lcd_mb.buf_pool[i++] = *pch++;

	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_DU;
	set_bit(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC);

	init_lcd_mb_st(&textmenu2lcd_mb.textmenu_msg);
	textmenu2lcd_mb.textmenu_msg.cmd		 = LCD_CMD_DIS_CHAR;
	textmenu2lcd_mb.textmenu_msg.start_x 		 = BODY_START_X + INDENT1_SIZE;
	textmenu2lcd_mb.textmenu_msg.start_y 		 = BODY_START_Y
			+ line_cnt*MIDDLE_CN_FONT_HEIGHT + LINE_GAP;
	textmenu2lcd_mb.textmenu_msg.color		 = Blue2;
	textmenu2lcd_mb.textmenu_msg.font_size.font_size = LCD_FONT_SIZE1;
	textmenu2lcd_mb.textmenu_msg.buf		 = textmenu2lcd_mb.buf_pool;
	textmenu2lcd_mb.textmenu_msg.len.len = i;

	rt_mb_send(&lcd_mb._mb, (rt_uint32_t)&textmenu2lcd_mb.textmenu_msg);
	++line_cnt;

	/* 当天12-18时用电量:xxx度 */
	while (is_bit_set(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC))
		rt_thread_delay(get_ticks_of_ms(20));
	i = 0;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_DANG;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_TIAN;
	textmenu2lcd_mb.buf_pool[i++] = '1';
	textmenu2lcd_mb.buf_pool[i++] = '2';
	textmenu2lcd_mb.buf_pool[i++] = '-';
	textmenu2lcd_mb.buf_pool[i++] = '1';
	textmenu2lcd_mb.buf_pool[i++] = '8';
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_SHI2;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_YONG;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_DIAN;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_LIANG;
	textmenu2lcd_mb.buf_pool[i++] = ':';

	temp = info_buf[info_index++];
	temp = (temp<<8) | info_buf[info_index++];
	i2str(buf, temp);
	pch = buf;
	while ('\0' != *pch)
		textmenu2lcd_mb.buf_pool[i++] = *pch++;

	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_DU;
	set_bit(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC);

	init_lcd_mb_st(&textmenu2lcd_mb.textmenu_msg);
	textmenu2lcd_mb.textmenu_msg.cmd		 = LCD_CMD_DIS_CHAR;
	textmenu2lcd_mb.textmenu_msg.start_x 		 = BODY_START_X + INDENT1_SIZE;
	textmenu2lcd_mb.textmenu_msg.start_y 		 = BODY_START_Y
			+ line_cnt*MIDDLE_CN_FONT_HEIGHT + LINE_GAP;
	textmenu2lcd_mb.textmenu_msg.color		 = Blue2;
	textmenu2lcd_mb.textmenu_msg.font_size.font_size = LCD_FONT_SIZE1;
	textmenu2lcd_mb.textmenu_msg.buf		 = textmenu2lcd_mb.buf_pool;
	textmenu2lcd_mb.textmenu_msg.len.len = i;

	rt_mb_send(&lcd_mb._mb, (rt_uint32_t)&textmenu2lcd_mb.textmenu_msg);
	++line_cnt;

	/* 当天18-24时用电量:xxx度 */
	while (is_bit_set(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC))
		rt_thread_delay(get_ticks_of_ms(20));
	i = 0;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_DANG;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_TIAN;
	textmenu2lcd_mb.buf_pool[i++] = '1';
	textmenu2lcd_mb.buf_pool[i++] = '8';
	textmenu2lcd_mb.buf_pool[i++] = '-';
	textmenu2lcd_mb.buf_pool[i++] = '2';
	textmenu2lcd_mb.buf_pool[i++] = '4';
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_SHI2;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_YONG;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_DIAN;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_LIANG;
	textmenu2lcd_mb.buf_pool[i++] = ':';

	temp = info_buf[info_index++];
	temp = (temp<<8) | info_buf[info_index++];
	i2str(buf, temp);
	pch = buf;
	while ('\0' != *pch)
		textmenu2lcd_mb.buf_pool[i++] = *pch++;

	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_DU;
	set_bit(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC);

	init_lcd_mb_st(&textmenu2lcd_mb.textmenu_msg);
	textmenu2lcd_mb.textmenu_msg.cmd		 = LCD_CMD_DIS_CHAR;
	textmenu2lcd_mb.textmenu_msg.start_x 		 = BODY_START_X + INDENT1_SIZE;
	textmenu2lcd_mb.textmenu_msg.start_y 		 = BODY_START_Y
			+ line_cnt*MIDDLE_CN_FONT_HEIGHT + LINE_GAP;
	textmenu2lcd_mb.textmenu_msg.color		 = Blue2;
	textmenu2lcd_mb.textmenu_msg.font_size.font_size = LCD_FONT_SIZE1;
	textmenu2lcd_mb.textmenu_msg.buf		 = textmenu2lcd_mb.buf_pool;
	textmenu2lcd_mb.textmenu_msg.len.len = i;

	rt_mb_send(&lcd_mb._mb, (rt_uint32_t)&textmenu2lcd_mb.textmenu_msg);
	++line_cnt;

	/* 2.当月用电量:xxx度 */
	while (is_bit_set(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC))
		rt_thread_delay(get_ticks_of_ms(20));
	i = 0;
	textmenu2lcd_mb.buf_pool[i++] = '2';
	textmenu2lcd_mb.buf_pool[i++] = '.';
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_DANG;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_YUE;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_YONG;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_DIAN;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_LIANG;
	textmenu2lcd_mb.buf_pool[i++] = ':';

	temp = info_buf[info_index++];
	temp = (temp<<8) | info_buf[info_index++];
	i2str(buf, temp);
	pch = buf;
	while ('\0' != *pch)
		textmenu2lcd_mb.buf_pool[i++] = *pch++;

	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_DU;
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

	/* 3.上月用电量:xxx度 */
	while (is_bit_set(textmenu2lcd_mb.textmenu_msg.flag, BIT_IS_MSG_NOT_PROC))
		rt_thread_delay(get_ticks_of_ms(20));
	i = 0;
	textmenu2lcd_mb.buf_pool[i++] = '3';
	textmenu2lcd_mb.buf_pool[i++] = '.';
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_SHANG;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_YUE;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_YONG;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_DIAN;
	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_LIANG;
	textmenu2lcd_mb.buf_pool[i++] = ':';

	temp = info_buf[info_index++];
	temp = (temp<<8) | info_buf[info_index++];
	i2str(buf, temp);
	pch = buf;
	while ('\0' != *pch)
		textmenu2lcd_mb.buf_pool[i++] = *pch++;

	textmenu2lcd_mb.buf_pool[i++] = FONT_ID_CN_DU;
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

}
#endif

#if 1 /* webm_p client */


/* static struct ip_addr localip; */
/** The main function, never returns! */
static void webm_p_client_thread_entry(void *arg)
{
	struct webm_mb_buf *pwebmb;
	LWIP_UNUSED_ARG(arg);


	rt_thread_delay(get_ticks_of_ms(120*1000));
	
	while (1) {
		if (RT_EOK == rt_mb_recv(&webm_mb._mb, (rt_uint32_t *)&pwebmb, RT_WAITING_FOREVER)) {
			webm_client_msg_proc(pwebmb);
		}

	}

	return;
}

static void webm_client_msg_proc(struct webm_mb_buf *pwebmb)
{
	char *pch;
	
	if (NULL == pwebmb)
		return;

	WEBM_P_PRINT_DEBUG(("%s() recv cmd--0x%x\n", __FUNCTION__, pwebmb->cmd));
	switch (pwebmb->cmd) {
	case CM_AUTH_REQ:
		webm_p_send_auth_and_proc_rep(pwebmb->com_buf.usr_info.usrid, pwebmb->com_buf.usr_info.usrpw);
		break;
#if 1==USE_TO_7INCH_LCD
	case CM_QUERY_PCOMSUMP:
		webm_p_send_query_pconsump_and_proc_rep(pwebmb->com_buf.usr_info.usrid);
		break;
#endif

	case CM_WARNNING_IFNO_NOTIFI:
		pch = pwebmb->com_buf.usr_info.usrid;
		webm_p_send_warning_info_and_proc_rep(pch[0], pch[1], pch[2]);
		break;
		
	default:
		break;
	}
}


static int webm_p_send_warning_info_and_proc_rep(int ill_open, int tmp_orun, int rh_orun)
{
	struct netconn *conn;
	err_t err;
	ip_addr_t addr;
	int ret = FAIL;
	
	/* Create a new TCP connection handle */
	conn = netconn_new(NETCONN_TCP);
	LWIP_ERROR("webm_p_thread: invalid conn", (conn != NULL), return ret;);

	addr.addr = (nms_ip);
	WEBM_P_PRINT_DEBUG(("tcp connect to ip:0x%x, port:%d\n", addr.addr, nms_port));
	err = netconn_connect(conn, &addr, nms_port);

	WEBM_P_PRINT_DEBUG(("netconn_connect errno:%d. warning_info:0x%x, has_notifi:0x%x. need_delay:0x%x\n",
		err, warning_info, has_notifi_warning_info, need_delay_notifi));

        if (err == ERR_OK) {
        	if (FAIL == webm_p_send_warninginfo(conn, ill_open, tmp_orun, rh_orun)) {
        		WEBM_P_PRINT_LOG(("send warning info fail!\n"));
        		goto ret_entry;
        	}
		ret = webm_p_data_proc(conn);
        } else {
        	WEBM_P_PRINT_LOG(("send warning info(%d, %d, %d). creat netconn err(%d)!\n",
        		ill_open, tmp_orun, rh_orun, err));
        }

ret_entry:
	netconn_delete(conn);

	if (FAIL==ret) {
		if (0 != ill_open)
			set_bit(need_delay_notifi, ILLEGAL_OPEN_LOCK);

		if (0 != tmp_orun)
			set_bit(need_delay_notifi, TMP_OVERRUN);

		if (0 != rh_orun)
			set_bit(need_delay_notifi, RH_OVERRUN);
	} else {
		if (0 != ill_open)
			clr_bit(need_delay_notifi, ILLEGAL_OPEN_LOCK);

		if (0 != tmp_orun)
			clr_bit(need_delay_notifi, TMP_OVERRUN);

		if (0 != rh_orun)
			clr_bit(need_delay_notifi, RH_OVERRUN);
	}
	return ret;
}


static int webm_p_send_warninginfo(struct netconn *conn, int ill_open, int tmp_orun, int rh_orun)
{
	char buf[4+sizeof(struct webm_data_st)+4];
	struct webm_data_st send_data;

	rt_memset(&send_data, 0, sizeof(send_data));
	send_data.cmd		= htonl(CM_WARNNING_IFNO_NOTIFI);
	send_data.usr_id[0] = ill_open;
	send_data.usr_id[1] = tmp_orun;
	send_data.usr_id[2] = rh_orun;

	WEBM_P_PRINT_DEBUG(("%s() will send warning info(%d,%d,%d)\n", __FUNCTION__, ill_open, tmp_orun, rh_orun));
	
	if (SUCC == webm_p_msg_package(&send_data, buf, sizeof(buf))) {
		if (ERR_OK != netconn_write(conn, buf, sizeof(buf), NETCONN_COPY)) {
			WEBM_P_PRINT_LOG(("func:%s, netconn_write() fail!\n"));
			return FAIL;
		}
	} else {
		WEBM_P_PRINT_LOG(("func:%s, msg package fail!\n"));
		return FAIL;
	}

	return SUCC;	
}

#endif


#endif /* LWIP_NETCONN*/

