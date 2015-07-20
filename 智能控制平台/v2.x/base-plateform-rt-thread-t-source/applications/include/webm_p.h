#ifndef WEBM_P_H__
#define WEBM_P_H__


#define START_FLAG_VALUE 0X1F553F5A
#define END_FLAG_VALUE   0XAF25CF6A

#define USR_INFO_MAX 12

#define SUCC    0
#define FAIL    1

struct webm_data_st {
	int  cmd;
	int  rep_arg;
	char usr_id[USR_INFO_MAX];
	char usr_pw[USR_INFO_MAX];
};


enum command_mnemonic {
	CM_INVALID			= 0,	/* invalid cmd */
	CM_AUTH_REQ			= 1,	/* authentication request */
	CM_OPEN_LOCK			= 2,	/*  */
	CM_CLOSE_LOCK			= 3,	/*  */
	CM_GET_TEMP_RH			= 4,	/* temperature, relative humidity */
	CM_QUERY_PCOMSUMP		= 5,	/* query power consumption */
	CM_QUERY_EQ_RUNNING_STATE	= 6,	/* query equipment running state */
	CM_REV_PUBLICITY_INFO		= 7,	/* 接收宣传消息 */
	CM_REGISTER_TERMINAL 		= 8,	/*  */
	CM_WARNNING_IFNO_NOTIFI 	= 9,	/* warning information notification */
	CM_SET_TMP_RH_LIMEN     	= 10,   /* set temperature and relative humidity limen */

	CM_AUTH_REP			= 0x100, /* authentication response */
	CM_OPEN_LOCK_REP		= 0x101, /* open lock response */
	CM_CLOSE_LOCK_REP		= 0x102, /* open lock response */
	CM_GET_TEMP_RH_REP		= 0x103, /* get temperature, relative humidity response */
	CM_QUERY_PCOMSUMP_REP		= 0x104, /* query power consumption response */
	CM_QUERY_EQ_RUNNING_STATE_REP	= 0x105, /* query equipment running state response */
	CM_REV_PUBLICITY_INFO_REP	= 0x106, /* 接收宣传消息响应 */
	CM_REGISTER_TERMINAL_REP     	= 0x107, /*  */
	CM_WARNNING_IFNO_NOTIFI_REP  	= 0x108, /* warning information notification response */
	CM_SET_TMP_RH_LIMEN_REP      	= 0X109, /* CM_SET_TMP_RH_LIMEN response */
};

struct webm_mb_buf {
	int cmd;
	union {
		struct {
			char usrid[USR_INFO_MAX];
			char usrpw[USR_INFO_MAX];
		}usr_info;
	} com_buf;
};

struct webm_ipc_mb {
    struct rt_mailbox _mb;
    struct webm_mb_buf *pool[5];
};

struct publiciy_st {
	int index;
	unsigned char *buf;
};

struct webm_mb_entity {
	struct webm_mb_buf mb_ent;
	int flag;
};


extern struct webm_ipc_mb webm_mb;
extern struct publiciy_st publicity_from_webm;

#define get_publicity_start_addr(data) (data+4+sizeof(struct webm_data_st)+2)

/*
 * only use by M3
 */
extern void webm_p_thread_init(void);
extern int webm_p_send_auth_and_proc_rep(char *id, char *pw);

extern void dis_pconsump_info(char *info_buf, int len);
extern void webm_p_send_query_pconsump_and_proc_rep(char *id);

#endif
