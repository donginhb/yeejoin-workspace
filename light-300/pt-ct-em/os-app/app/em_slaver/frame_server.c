#include <rtthread.h>
#include <lwip/sockets.h> 
#include <frame_em.h>
#include <string.h>
#include <sys_cfg_api.h>

#define frame_server_info(x) printf_syn x

static rt_uint8_t r_frame[] = {0x59, 0x4a,0x59, 0x4a,0x81,0x00};
static rt_uint8_t w_frame[] = {0x59, 0x4a,0x59, 0x4a,0x81,0x01};
struct em_distr_ms em_s_self_data;

static void updata_tcp_data_to_ram(struct em_distr_ms *data)
{
	enum sink_data_dev_type_e type;
	struct sinkinfo_wl_data_item_st wl_data;
	rt_tick_t time_stamp;

	rt_memset(&wl_data, 0, sizeof(wl_data));

	rt_memcpy(wl_data.pt_ct_sn, data->pt_ct_sn, DEV_SN_MODE_LEN);
	type = si_get_dev_type(wl_data.pt_ct_sn);
	time_stamp = rt_tick_get();
	if (type == SDDT_PT) {
		wl_data.item.ptc_data.pt_pa.vx = data->pt_ct_v[0];
		wl_data.item.ptc_data.pt_pa.appx = data->pt_ct_app_p[0];
		wl_data.item.ptc_data.pt_pa.apx = data->pt_ct_ap_p[0];
		wl_data.item.ptc_data.pt_pa.ix = data->pt_ct_i[0];
		if (data->pt_ct_app_p[0] != 0) {
			wl_data.item.ptc_data.pt_pa.pfx = data->pt_ct_ap_p[0]/data->pt_ct_app_p[0];
		} else {
			frame_server_info(("app_p_A is zero\n"));
		}
		//wl_data.item.ptc_data.pt_pa.admx = RT_INT32_MAX; /*�����ܼ���õ�*/
		
		wl_data.item.ptc_data.pt_pb.vx = data->pt_ct_v[1];
		wl_data.item.ptc_data.pt_pb.appx = data->pt_ct_app_p[1];
		wl_data.item.ptc_data.pt_pb.apx = data->pt_ct_ap_p[1];
		wl_data.item.ptc_data.pt_pb.ix = data->pt_ct_i[1];
		if (data->pt_ct_app_p[1] != 0) {
			wl_data.item.ptc_data.pt_pb.pfx = data->pt_ct_ap_p[1]/data->pt_ct_app_p[1];
		} else {
			frame_server_info(("app_p_B is zero\n"));
		}
		//wl_data.item.ptc_data.pt_pb.admx = RT_INT32_MAX; 
		
		wl_data.item.ptc_data.pt_pc.vx = data->pt_ct_v[2];
		wl_data.item.ptc_data.pt_pc.appx = data->pt_ct_app_p[2];
		wl_data.item.ptc_data.pt_pc.apx = data->pt_ct_ap_p[2];
		wl_data.item.ptc_data.pt_pc.ix = data->pt_ct_i[2];
		if (data->pt_ct_app_p[2] != 0) {
			wl_data.item.ptc_data.pt_pc.pfx = data->pt_ct_ap_p[2]/data->pt_ct_app_p[2];
		} else {
			frame_server_info(("app_p_C is zero\n"));
		}
		//wl_data.item.ptc_data.pt_pc.admx = RT_INT32_MAX; 
		si_update_wl_collect_data(wl_data.pt_ct_sn, &wl_data.item, time_stamp);
	} else if (type == SDDT_CT) {
		wl_data.item.ctc_data.ct_pa.vx = data->pt_ct_v[0];
		wl_data.item.ctc_data.ct_pa.appx = data->pt_ct_app_p[0];
		wl_data.item.ctc_data.ct_pa.apx = data->pt_ct_ap_p[0];
		wl_data.item.ctc_data.ct_pa.ix = data->pt_ct_i[0];
		if (data->pt_ct_app_p[0] != 0) {
			wl_data.item.ctc_data.ct_pa.pfx = data->pt_ct_ap_p[0]/data->pt_ct_app_p[0];
		} else {
			frame_server_info(("app_p_A is zero\n"));
		}
		//wl_data.item.ctc_data.ct_pa.admx = RT_INT32_MAX;
		
		wl_data.item.ctc_data.ct_pb.vx = data->pt_ct_v[1];
		wl_data.item.ctc_data.ct_pb.appx = data->pt_ct_app_p[1];
		wl_data.item.ctc_data.ct_pb.apx = data->pt_ct_ap_p[1];
		wl_data.item.ctc_data.ct_pb.ix = data->pt_ct_i[1];
		if (data->pt_ct_app_p[1] != 0) {
			wl_data.item.ctc_data.ct_pb.pfx = data->pt_ct_ap_p[1]/data->pt_ct_app_p[1];
		} else {
			frame_server_info(("app_p_B is zero\n"));
		}
		//wl_data.item.ctc_data.ct_pb.admx = RT_INT32_MAX;
		
		wl_data.item.ctc_data.ct_pc.vx = data->pt_ct_v[2];
		wl_data.item.ctc_data.ct_pc.appx = data->pt_ct_app_p[2];
		wl_data.item.ctc_data.ct_pc.apx = data->pt_ct_ap_p[2];
		wl_data.item.ctc_data.ct_pc.ix = data->pt_ct_i[2];
		if (data->pt_ct_app_p[2] != 0) {
			wl_data.item.ctc_data.ct_pc.pfx = data->pt_ct_ap_p[2]/data->pt_ct_app_p[2];
		} else {
			frame_server_info(("app_p_C is zero\n"));
		}
		//wl_data.item.ctc_data.ct_pc.admx = RT_INT32_MAX;
		si_update_wl_collect_data(wl_data.pt_ct_sn, &wl_data.item, time_stamp);
	} else {
		printf_syn("func:%s(), get invalid dev-type:%d\n", __FUNCTION__, type);
	}
}

static void save_em_data(rt_uint8_t *recv_data, int flag)
{
	rt_memcpy(em_s_self_data.pt_ct_sn, recv_data + NET_PKT_SN_OFFSET, DEV_SN_MODE_LEN);
	em_s_self_data.pt_ct_app_p[0] = lwip_ntohl(*(u32_t *)(recv_data + NET_PKT_APP_OFFSET_A));
	em_s_self_data.pt_ct_app_p[1] = lwip_ntohl(*(u32_t *)(recv_data + NET_PKT_APP_OFFSET_B));
	em_s_self_data.pt_ct_app_p[2] = lwip_ntohl(*(u32_t *)(recv_data + NET_PKT_APP_OFFSET_C));

	em_s_self_data.pt_ct_ap_p[0] = lwip_ntohl(*(u32_t *)(recv_data + NET_PKT_AP_OFFSET_A));
	em_s_self_data.pt_ct_ap_p[1] = lwip_ntohl(*(u32_t *)(recv_data + NET_PKT_AP_OFFSET_B));
	em_s_self_data.pt_ct_ap_p[2] = lwip_ntohl(*(u32_t *)(recv_data + NET_PKT_AP_OFFSET_C));

	em_s_self_data.pt_ct_v[0] = lwip_ntohl(*(u32_t *)(recv_data + NET_PKT_V_OFFSET_A));
	em_s_self_data.pt_ct_v[1] = lwip_ntohl(*(u32_t *)(recv_data + NET_PKT_V_OFFSET_B));
	em_s_self_data.pt_ct_v[2] = lwip_ntohl(*(u32_t *)(recv_data + NET_PKT_V_OFFSET_C));

	em_s_self_data.pt_ct_i[0] = lwip_ntohl(*(u32_t *)(recv_data + NET_PKT_I_OFFSET_A));
	em_s_self_data.pt_ct_i[1] = lwip_ntohl(*(u32_t *)(recv_data + NET_PKT_I_OFFSET_B));
	em_s_self_data.pt_ct_i[2] = lwip_ntohl(*(u32_t *)(recv_data + NET_PKT_I_OFFSET_C));

	print_dev_sn(em_s_self_data.pt_ct_sn, sizeof(em_s_self_data.pt_ct_sn));
#if 1
	frame_server_info(("voltage:%10d, %10d, %10d\n", em_s_self_data.pt_ct_v[0],
			em_s_self_data.pt_ct_v[1], em_s_self_data.pt_ct_v[2]));

	frame_server_info(("current:%10d, %10d, %10d\n", em_s_self_data.pt_ct_i[0],
			em_s_self_data.pt_ct_i[1], em_s_self_data.pt_ct_i[2]));

	frame_server_info(("  app-p:%10d, %10d, %10d\n", em_s_self_data.pt_ct_app_p[0],
			em_s_self_data.pt_ct_app_p[1], em_s_self_data.pt_ct_app_p[2]));

	frame_server_info(("  act-p:%10d, %10d, %10d\n\n", em_s_self_data.pt_ct_ap_p[0],
			em_s_self_data.pt_ct_ap_p[1], em_s_self_data.pt_ct_ap_p[2]));
#endif
	updata_tcp_data_to_ram(&em_s_self_data);
}
static rt_err_t check_recv_data_frame(rt_uint8_t *recv_data, rt_uint16_t frame_len)
{	
	enum sink_data_dev_type_e type;

        if (frame_len < NET_PKT_LEN_REP)
                return RT_ERROR;
        if ((recv_data[0] != 0x59) || (recv_data[1] != 0x4a) || (recv_data[2] != 0x59) || (recv_data[3] != 0x4a))
                return RT_ERROR;
        
        switch (recv_data[4]) {
        case FIPCMD_SEND_DATA: 
                if (frame_len == NET_PKT_LEN){
			type = si_get_dev_type((char *)(recv_data + NET_PKT_SN_OFFSET));
			if (SDDT_PT == type) {
				save_em_data(recv_data, SDDT_PT);
				return RT_EOK;
			} else if (SDDT_CT == type) {
				save_em_data(recv_data, SDDT_CT);
				return RT_EOK;
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

	struct fd_set fds; 
	int maxfdp = 0;
	struct timeval timeout = {1,0}; //select等待1秒，1秒轮询，要非阻塞就置0 

	FD_ZERO(&fds); //每次循环都要清空集合，否则不能检测描述符变化
	FD_SET(sock, &fds); //添加描述符 
	maxfdp= sock + 1;    //描述符最大值加1

	switch (select(maxfdp, &fds, NULL, NULL, &timeout)) { 
	case -1: 
	        frame_server_info(("select error!\n"));
	        return RT_ERROR;

	case 1: 
	        if (FD_ISSET(sock, &fds)) { //测试sock是否可读，即是否网络上有数据
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
	int maxfdp = 0;
	struct timeval timeout = {1,0}; //select等待1秒，1秒轮询，要非阻塞就置0 

	FD_ZERO(&fds); //每次循环都要清空集合，否则不能检测描述符变化
	FD_SET(sock, &fds); //添加描述符 
	maxfdp = sock + 1;    //描述符最大值加1

	switch (select(maxfdp, NULL, &fds, NULL, &timeout)) {
	case -1: 
	        frame_server_info(("select error!\n"));
	        return RT_ERROR;

	case 1: 
	        if (FD_ISSET(sock, &fds)) { //测试sock是否可写，即是否网络上有数据
	                 return RT_EOK;
	        }
	        break;

	default:
	        break;
	}
	return RT_ERROR;
}

void recv_em_frame(void* parameter)
{
	rt_uint8_t *recv_data;
	rt_uint32_t sin_size;
	int sock, connected, bytes_received;
	struct sockaddr_in server_addr, client_addr;
	rt_bool_t stop = RT_FALSE; 
	rt_uint32_t	times;

	recv_data = rt_malloc(EM_FRAME_LEN); 
	if (recv_data == RT_NULL) {
		printf_syn("No memory\n");
		return;
	}

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf_syn("create Socket error\n");
		rt_free(recv_data);
		return;
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SEV_PORT); 
	server_addr.sin_addr.s_addr = INADDR_ANY;
	rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));


	if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
		printf_syn("Unable to bind\n");
		rt_free(recv_data);
		return;
	}

	if (listen(sock, 5) == -1) {
		printf_syn("Listen error\n");
		/* release recv buffer */
		rt_free(recv_data);
		return;
	}
	
	frame_server_info(("\nTCPServer Waiting for client on port 5000...\n"));
	while (stop != RT_TRUE) {
		sin_size = sizeof(struct sockaddr_in);
		connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);
		frame_server_info(("I got a connection from (%s , %d)\n",
				   inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port)));
		times = TRY_TIMES;
		while (times--) {
			if(RT_EOK == judge_readfd(connected)) {
				bytes_received = recv(connected,recv_data, EM_FRAME_LEN, 0);
				if (bytes_received < 0) {
					times = 0;
					break;
				}
				//recv_data[bytes_received] = '\0';
				if (RT_EOK == check_recv_data_frame((rt_uint8_t *)recv_data, bytes_received)) {
					times = TRY_TIMES;
					while (times--) {
						if (RT_EOK == judge_writefd(connected)) {
							send(connected, r_frame, 6, 0);
							times = 0;
							break;
						}
					}
				} else {
					send(connected, w_frame, 6, 0);
				}
			}
			if (times == 0) {
				lwip_close(connected);
				break;
			}
		}
	}

	lwip_close(sock);
	rt_free(recv_data);
	return;
}

