#include <rtthread.h>
#include <board.h>
#include "rtc.h"
#include <misc_lib.h>
#include <frame_em.h>
#include <ms_common.h>
#include <string.h>
#include <lwip/sockets.h>
#include <sys_cfg_api.h>

#ifdef RT_USING_FINSH
#include <finsh.h>
#endif

#define frame_client_info(x) printf_syn x

static int judge_writefd(const int sock);
static int judge_readfd(const int sock);

extern struct slave_emm_collector_info_st *slave_emmc_info;

struct em_distr_ms em_m_self_data;
extern struct msfd_report_collection_data_sn_st *pc_collect_data_m;

#ifdef RT_USING_FINSH	

typedef char (*ptct_sn_set_t)[DEV_SN_MODE_LEN];

/*
 * 如果找到了返回对应起始地址
 * 如果没有找到，当有空闲位置时返回一个空位置地址，否则，返回NULL
 * */
static char *find_ptct_sn(char *sn, ptct_sn_set_t ptct_sn_set, int max)
{
	int i;
	char *first_empty = NULL;
	char *tmp;

	for (i=0; i<max; ++i) {
		tmp = ptct_sn_set[i];
		if (0 == rt_memcmp(sn, tmp, DEV_SN_MODE_LEN)) {
			return tmp;
		}

		if (0 == ptct_sn_set[i][0])
			first_empty = ptct_sn_set[i];
	}

	return first_empty;
}

/*
 * 添加struct slave_emm_collector_info_st中的信息
 *
 * no与ip_addr一一对应
 * 一个ip_addr可以对应多个pt_sn和ct_sn
 * pt_sn, ct_sn可以时空字符串
 * */
int em_ms_add(int no, char *ip_addr, char *pt_sn, char *ct_sn)
{
	struct in_addr	emm_ip;
	char *sn;
	unsigned int len;

	if ((no > NUM_OF_SLAVE_EMM_MAX) || (no <= 0)) {
		printf_syn("wrong pt num(should 1-%d)!\n", NUM_OF_SLAVE_EMM_MAX);
		return RT_ERROR;
	}

	if (RT_NULL==ip_addr || NULL==pt_sn || NULL==ct_sn) {
		printf_syn("param has NULL pointer\n");
		return RT_ERROR;
	}

	if (NULL == slave_emmc_info) {
		printf_syn("func:%s, slave_emmc_info is NULL\n", __FUNCTION__);
		return RT_ERROR;
	}

	if (1 == inet_aton(ip_addr, &emm_ip)) {
	        read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_S_EMM_INFO, 0, slave_emmc_info);
	        --no;

		if (0 == slave_emmc_info[no].emm_ip.s_addr) {
			/* 该位置为空 */
			slave_emmc_info[no].emm_ip.s_addr = emm_ip.s_addr;
		} else if (slave_emmc_info[no].emm_ip.s_addr != emm_ip.s_addr) {
			printf_syn("this position has data(%s)", inet_ntoa(slave_emmc_info[no].emm_ip));
			return RT_ERROR;
		}

		/*  */
		len = rt_strlen(pt_sn);
		if (DEV_SN_MODE_LEN == len) {
			sn = find_ptct_sn(pt_sn, slave_emmc_info[no].ptc_sn, NUM_OF_PT_OF_COLLECT_EM_MAX);
			if (NULL != sn) {
				if (0 == sn[0])
					rt_memcpy(sn, pt_sn, DEV_SN_MODE_LEN); /* 拷贝到空位置 */
				else
					; /* 已存在, 不做任何操作 */
			} else {
				printf_syn("slave ptc sn-set had full\n");
			}
		} else if (0 != len) {
			printf_syn("sn:%s, len error\n", pt_sn);
		}

		len = rt_strlen(ct_sn);
		if (DEV_SN_MODE_LEN == len) {
			sn = find_ptct_sn(ct_sn, slave_emmc_info[no].ctc_sn, NUM_OF_CT_OF_COLLECT_EM_MAX);
			if (NULL != sn) {
				if (0 == sn[0])
					rt_memcpy(sn, ct_sn, DEV_SN_MODE_LEN); /* 拷贝到空位置 */
				else
					; /* 已存在, 不做任何操作 */
			} else {
				printf_syn("slave ctc sn-set had full\n");
			}
		} else if (0 != len) {
			printf_syn("sn:%s, len error\n", ct_sn);
		}

	        write_syscfgdata_tbl(SYSCFGDATA_TBL_S_EMM_INFO, 0, slave_emmc_info);
	        syscfgdata_syn_proc();
	} else {
		printf_syn("input ip is invalid(%s)\n", ip_addr);
		return RT_ERROR;
	}

	return RT_EOK;
}
FINSH_FUNCTION_EXPORT(em_ms_add, add ptc-ctc info to slave em info tbl);

/*
 * 删除struct slave_emm_collector_info_st中的信息
 *
 * ip_addr, pt_sn, ct_sn同时为空时, 删除指定的no的所有记录
 * 当no与ip_addr有效时，删除匹配的pt_sn, ct_sn
 * */
int em_ms_del(int no, char *ip_addr, char *pt_sn, char *ct_sn)
{
	struct in_addr	emm_ip;
	char *sn;
	unsigned int len1, len2, len3;

	if ((no > NUM_OF_SLAVE_EMM_MAX) || (no <= 0)) {
		printf_syn("wrong pt num(should 1-%d)!\n", NUM_OF_SLAVE_EMM_MAX);
		return RT_ERROR;
	}

	if (RT_NULL==ip_addr || NULL==pt_sn || NULL==ct_sn) {
		printf_syn("param has NULL pointer\n");
		return RT_ERROR;
	}

	if (NULL == slave_emmc_info) {
		printf_syn("func:%s, slave_emmc_info is NULL\n", __FUNCTION__);
		return RT_ERROR;
	}

        read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_S_EMM_INFO, 0, slave_emmc_info);
        --no;

        len1 = rt_strlen(ip_addr);
        len2 = rt_strlen(pt_sn);
        len3 = rt_strlen(ct_sn);

        if (0==len1 && 0==len2 && 0==len3) {
        	/* 删除编号为no的整个记录数据 */
        	rt_memset(&slave_emmc_info[no], 0, sizeof(*slave_emmc_info));
        } else if ((1==inet_aton(ip_addr, &emm_ip)) && (slave_emmc_info[no].emm_ip.s_addr == emm_ip.s_addr)) {
		/*  */
		if (DEV_SN_MODE_LEN == len2) {
			sn = find_ptct_sn(pt_sn, slave_emmc_info[no].ptc_sn, NUM_OF_PT_OF_COLLECT_EM_MAX);
			if (NULL!=sn && 0!=sn[0]) {
				rt_memset(sn, 0, DEV_SN_MODE_LEN);
			} else {
				printf_syn("slave ptc sn(%s) not find\n", pt_sn);
			}
		} else if (0 != len2) {
			printf_syn("sn:%s, len error\n", pt_sn);
		}

		if (DEV_SN_MODE_LEN == len3) {
			sn = find_ptct_sn(ct_sn, slave_emmc_info[no].ctc_sn, NUM_OF_CT_OF_COLLECT_EM_MAX);
			if (NULL!=sn && 0!=sn[0]) {
				rt_memset(sn, 0, DEV_SN_MODE_LEN);
			} else {
				printf_syn("slave ctc sn(%s) not find\n", ct_sn);
			}
		} else if (0 != len3) {
			printf_syn("sn:%s, len error\n", ct_sn);
		}
	} else {
		printf_syn("input ip is invalid(%s) or not eq db save ip(%s) that index is %d\n", ip_addr,
				inet_ntoa(slave_emmc_info[no].emm_ip), no+1);
		return RT_ERROR;
	}

        write_syscfgdata_tbl(SYSCFGDATA_TBL_S_EMM_INFO, 0, slave_emmc_info);
        syscfgdata_syn_proc();

	return RT_EOK;
}
FINSH_FUNCTION_EXPORT(em_ms_del, del ptc-ctc info from slave em info tbl);

static void print_ptct_sn(ptct_sn_set_t ptct_sn_set, int max)
{
	int i, j;
	char tmp[DEV_SN_BUF_STRING_WITH_NUL_LEN_MAX];

	j = 0;
	tmp[DEV_SN_MODE_LEN] = 0;
	for (i=0; i<max; ++i) {
		if ('\0' != ptct_sn_set[i][0]) {
			rt_strncpy(tmp, ptct_sn_set[i], DEV_SN_MODE_LEN);
			printf_syn("%s, ", tmp);

			if (0 == ++j%5)
				printf_syn("\n");
		}
	}

	printf_syn("\n");

	return;
}

int list_em_ms(void)
{
	int i;
	struct slave_emm_collector_info_st *sinfo;
	
	if (NULL == slave_emmc_info) {
		printf_syn("func:%s, slave_emmc_info is NULL\n", __FUNCTION__);
		return RT_ERROR;
	}

	for (i = 0; i < NUM_OF_SLAVE_EMM_MAX; ++i) {
		sinfo = slave_emmc_info + i;
		if (0 != sinfo->emm_ip.s_addr) {
			printf_syn("[index:%2d, slave-em-ip:%s]\n", i+1, inet_ntoa(sinfo->emm_ip));
			print_ptct_sn(sinfo->ptc_sn, NUM_OF_PT_OF_COLLECT_EM_MAX);
			print_ptct_sn(sinfo->ctc_sn, NUM_OF_CT_OF_COLLECT_EM_MAX);
		}
	}

	return RT_EOK;
}
FINSH_FUNCTION_EXPORT(list_em_ms, print slave em info tbl);
#endif



static rt_err_t check_recv_rep_frame(rt_uint8_t *recv_data, rt_uint16_t frame_len)
{
        if (frame_len < NET_PKT_LEN_REP)
                return RT_ERROR;
        if ((recv_data[0] != 0x59) || (recv_data[1] != 0x4a) || (recv_data[2] != 0x59) || (recv_data[3] != 0x4a))
                return RT_ERROR;
        
        switch(recv_data[4]){
        case FIPCMD_SEND_DATA: 
                if (frame_len == NET_PKT_LEN) {
			return RT_EOK;
		}
                break;
                
        case FIPCMD_RESPOND_DATA:
                if (frame_len == NET_PKT_LEN_REP) {
                        if (recv_data[5] == 0x01) {
                                return RT_CTRL_WRONG;
                        } else if (recv_data[5] == 0x00) {
                                return RT_CTRL_RTGHT;
                        } else {
                                return RT_ERROR;
                        }
                }
                break;

        default:
                break;                
        }
        
        return RT_ERROR;
}

static int judge_readfd(const int sock)
{
	struct fd_set	fds; 
	int	maxfdp = 0;
	struct timeval	timeout = {1,0}; /*select等待1秒，1秒轮询，要非阻塞就置0 */

	FD_ZERO(&fds); /*每次循环都要清空集合，否则不能检测描述符变化 */
	FD_SET(sock, &fds); /*添加描述符 */
	maxfdp = sock + 1;    /*描述符最大值加1 */

	switch (select(maxfdp, &fds, NULL, NULL, &timeout)) { 
	case -1: 
	        frame_client_info(("select error!\n"));
	        return RT_ERROR;

	case 1: 
	        if (FD_ISSET(sock, &fds)) { /*测试sock是否可读，即是否网络上有数据 */
	                 return RT_EOK;
	        }
	        break;
		
	default:
	        break;  

	}
	return RT_ERROR;
}

static int judge_writefd(const int sock)
{
        struct fd_set fds; 
        int	maxfdp = 0;
        struct timeval	timeout = {1,0}; /*select等待1秒，1秒轮询，要非阻塞就置0 */

        FD_ZERO(&fds); /*每次循环都要清空集合，否则不能检测描述符变化 */
        FD_SET(sock,&fds); /*添加描述符 */
        maxfdp = sock + 1;    /*描述符最大值加1 */

        switch (select(maxfdp, NULL, &fds, NULL, &timeout)) { 
        case -1: 
                frame_client_info(("select error!\n"));
                return RT_ERROR;

	case 1: 
                if (FD_ISSET(sock, &fds)) { /*测试sock是否可写，即是否网络上有数据 */
                         return RT_EOK;
                }
                break;

        default:
                break;  
        }
        return RT_ERROR;
}

/******************IP帧收发函数*************************/
enum tcp_transport_err_e send_em_ms_frame(rt_uint8_t *send_frame_buf,int send_len, rt_uint32_t ip_addr, rt_uint32_t port)
{
        int times = TRY_TIMES;
        int sock,result;
	int bytes_received = 0;
	rt_uint8_t *recv_data;
        struct sockaddr_in server_addr;
	enum tcp_transport_err_e tte_err = TTE_EOK;

	recv_data = rt_malloc(EM_FRAME_LEN);
	if (recv_data == RT_NULL) {
		printf_syn("memory malloc fail\n");
		return TTE_MALLOC_FAIL;
	}
        /* 创建一个socket，类型是SOCKET_STREAM，TCP类型 */
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                printf_syn("creat  Socket error\n");
		tte_err = TTE_CREATE_SOCKET_FAIL;
		goto ret_err;
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        server_addr.sin_addr.s_addr =  ip_addr;
        rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
	
	while (times--) {				/*三次连接，若建立失败则退出*/
                if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
			if (times == 0) {
				printf_syn("Connect error client test!\n");
				tte_err = TTE_CONNECT_FAIL;
				lwip_close(sock);
				goto ret_err;
			}
                } else {
			times = TRY_TIMES;
                        break;
                }
        }
	
        while (times--) {
                if (RT_EOK == judge_writefd(sock)) {
			frame_client_info(("tcp send sn:"));
			print_dev_sn(send_frame_buf + NET_PKT_SN_OFFSET, DEV_SN_MODE_LEN);
                        result = send(sock,send_frame_buf,send_len, 0);
			if (result == -1) {
				if (times == 0) {
					printf_syn("send fail\n");
					tte_err = TTE_SEND_FAIL;
					lwip_close(sock);
					goto ret_err;
				}
                                rt_thread_delay(10);
                        } else {
                        	tte_err = TTE_EOK;
                        	times = TRY_TIMES;
                                while (times--) {
                                        if (RT_EOK == judge_readfd(sock)) {
						rt_memset(recv_data, 0, EM_FRAME_LEN);
						bytes_received = recv(sock, recv_data, EM_FRAME_LEN, 0);
						if (bytes_received < 0) {
							printf_syn("recv tcp rep fail\n");
							tte_err = TTE_RECV_REP_FAIL;
							lwip_close(sock);
							goto ret_err;
                                            	}
                                                recv_data[bytes_received] = '\0';  
						if (RT_CTRL_RTGHT == check_recv_rep_frame(recv_data, bytes_received)) {
							lwip_close(sock);
							goto ret_err;
                                                } else {
							printf_syn("recv tcp rep fail\n");
							tte_err = TTE_RECV_REP_FAIL;
							lwip_close(sock);
							goto ret_err;
						}
                                        } else {
                                        	if (times == 0) {
							printf_syn("tcp dont read\n");
							tte_err = TTE_DONT_READ;
							lwip_close(sock);
							goto ret_err;
						}
					}
                                }
                        }
                } else {
			if (times == 0) {
	                	printf_syn("tcp dont write\n");
				tte_err = TTE_DONT_WRITE;
				lwip_close(sock);
				goto ret_err;
			}
			continue;
		}
                rt_thread_delay(10);                
        }
	lwip_close(sock);
ret_err:
	rt_free(recv_data);
	return tte_err;
}


static void frame_client_create_frame_head(struct frame_format_em *frame, enum frame_ctrl_em ctrl)
{
	frame->head1 = 0x59;
	frame->head2 = 0x4a;
	frame->head3 = 0x59;
	frame->head4 = 0x4a;
	frame->ctrl  = ctrl;
	frame->data_len = NET_DATA_LEN;
}

static void frame_client_create_net_data(struct msfd_report_collection_data_sn_st *data)
{
	int i;
	
	for (i=0; i<sizeof(data->coll_data.pt_ct_app_p)/sizeof(data->coll_data.pt_ct_app_p[0]); i++) {
		data->coll_data.pt_ct_app_p[i] = lwip_htonl(data->coll_data.pt_ct_app_p[i]);
	}

	for (i=0; i<sizeof(data->coll_data.pt_ct_ap_p)/sizeof(data->coll_data.pt_ct_ap_p[0]); i++) {
		data->coll_data.pt_ct_ap_p[i] = lwip_htonl(data->coll_data.pt_ct_ap_p[i]);
	}

	for (i=0; i<sizeof(data->coll_data.pt_ct_v)/sizeof(data->coll_data.pt_ct_v[0]); i++) {
		data->coll_data.pt_ct_v[i] = lwip_htonl(data->coll_data.pt_ct_v[i]);
	}

	for (i=0; i<sizeof(data->coll_data.pt_ct_i)/sizeof(data->coll_data.pt_ct_i[0]); i++) {
		data->coll_data.pt_ct_i[i] = lwip_htonl(data->coll_data.pt_ct_i[i]);
	}

	data->last_update_time = lwip_htonl(data->last_update_time);
}

static void frame_client_create_package_data(rt_uint8_t *pkt_data, struct frame_format_em *frame, rt_uint8_t *data)
{
	rt_uint8_t *tx_pch;

	tx_pch = pkt_data;
	*tx_pch++ = frame->head1;
	*tx_pch++ = frame->head2;
	*tx_pch++ = frame->head3;
	*tx_pch++ = frame->head4;
	*tx_pch++ = frame->ctrl;
	*tx_pch++ = lwip_htons(frame->data_len) & 0xff;
	*tx_pch++ = lwip_htons(frame->data_len) >> 8;
	rt_memcpy(tx_pch, data, frame->data_len);
}

static void frame_create_send_buffer(rt_uint8_t *pkt_data, struct frame_format_em *frame, struct msfd_report_collection_data_sn_st *coll_data)
{
	frame_client_create_frame_head(frame, FIPCMD_SEND_DATA);
	frame_client_create_net_data(coll_data);
	frame_client_create_package_data(pkt_data, frame, (rt_uint8_t *)coll_data);
}

/* EM分发PT/CT数据的函数，每次分发一个PT/CT的数据，如果N个PT/CT则需要分发N次 */
enum tcp_transport_err_e em_distribute_data_by_em_distrib_table(struct msfd_report_collection_data_sn_st *data)
{
	rt_uint8_t send_frame_buf[EM_FRAME_LEN] = {'\0'};
	struct frame_format_em frame_em;
	enum sink_data_dev_type_e type;
	enum tcp_transport_err_e tte_err = TTE_EOK;

	int i, max;
	char *sn;
	struct slave_emm_collector_info_st *sinfo;
	ptct_sn_set_t ptct_sn_set;
	
	/* slave_emmc_info所指区域常驻内存 */
	if (NULL == slave_emmc_info) {
		printf_syn("func:%s, slave_emmc_info is NULL\n", __FUNCTION__);
		return RT_ERROR;
	}

	type = si_get_dev_type((char *)data->sn);

	for (i = 0; i < NUM_OF_SLAVE_EMM_MAX; ++i) {
		sinfo = slave_emmc_info + i;
		if (0 == sinfo->emm_ip.s_addr)
			continue;

		/* 这里循环次数少, 所以把if语句放到循环体中，以减少代码量 */
		if (SDDT_PT == type) { /* PT */
			ptct_sn_set 	= sinfo->ptc_sn;
			max = NUM_OF_PT_OF_COLLECT_EM_MAX;
		} else if (SDDT_CT == type) {
			ptct_sn_set 	= sinfo->ctc_sn;
			max = NUM_OF_CT_OF_COLLECT_EM_MAX;
		} else {
			printf_syn("func:%s, recv an invalid dev-sn:", __FUNCTION__);
			print_dev_sn(data->sn, DEV_SN_MODE_LEN);
			break;
		}

		sn = find_ptct_sn((char *)data->sn, ptct_sn_set, max);
		if (NULL!=sn && 0!=sn[0]) {
			frame_create_send_buffer(send_frame_buf, &frame_em, data);
			tte_err = send_em_ms_frame(send_frame_buf, NET_PKT_LEN,
					sinfo->emm_ip.s_addr, SEV_PORT);
			if (TTE_EOK != tte_err) {
				printf_syn("func:%s, send data and return fail!\n", __FUNCTION__);
			}

			/* 找到后还要继续, 因为同一个pt/ct可能给不同的em共用 */
		}
	}

        return tte_err;
}

