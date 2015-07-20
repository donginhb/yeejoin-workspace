/**********************************************************************************
* Filename   	: ammeter_645_proxy.c
* Begin time  	: 2014-06-18
* Engineer		: zhanghonglei
*************************************************************************************/
#include <ammeter_645_proxy.h>
#include <ammeter_usart.h>
#include <rtdef.h>
#include <board.h>

#define DEBUG 0
#define ERROR_PRINTF(x) printf_syn x

static void ammeter_645_proxy_thread_entry(void* parameter);
static rt_err_t ammeter_645_proxy(enum ammeter_uart_e Collector_port, enum ammeter_uart_e ammeter_port,rt_uint32_t format_time_out, rt_uint32_t byte_time_out);

/**********************************************************************************
* Function	 : 	void ammeter_645_proxy_init(void)
* Description: 	透传数据初始化函数
* Arguments	 : 	void
* Return	 : 	void
*************************************************************************************/
void ammeter_645_proxy_init(void)
{
	rt_thread_t amm_645_proxy;
	
	/* 创建任务线程 */
	amm_645_proxy = rt_thread_create("645_proxy",  ammeter_645_proxy_thread_entry, RT_NULL,
	600, 10, 7);
	/* 启动任务线程 */
	if (amm_645_proxy != RT_NULL)
		rt_thread_startup(amm_645_proxy);

	return;
} 

/**********************************************************************************
* Function	 : 	static void ammeter_645_proxy_thread_entry(void* parameter)
* Description: 	透传数据线程入口函数
* Arguments	 : 	void
* Return	 : 	void
*************************************************************************************/
static void ammeter_645_proxy_thread_entry(void* parameter)
{	
	int ret = RT_ERROR;
	
	while (1) {
		ret = ammeter_645_proxy(AMMETER_UART3, AMMETER_UART1, 500, 100);
		if(ret != RT_EOK) {
			ERROR_PRINTF(("ERROR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
			return;
		}
	}
	
	return;
}
/**********************************************************************************
* Function	 : 	static rt_err_t ammeter_645_proxy(enum ammeter_uart_e Collector_port, enum ammeter_uart_e ammeter_port, rt_uint32_t format_time_out, rt_uint32_t byte_time_out)
* Description: 	接收采集器的数据发送到电表，并把电表返回的数据发送到采集器
* Arguments	 : 	（1）Collector_port：连接采集器的端口。（2）ammeter_port：连接电表的端口。
* 				（3）format_time_out：帧间隔超时时间（ms）。（4）byte_time_out：字节间隔超时时间（ms）。
* Return	 : 	RT_EOK
*************************************************************************************/
static rt_err_t ammeter_645_proxy(enum ammeter_uart_e Collector_port, enum ammeter_uart_e ammeter_port, rt_uint32_t format_time_out, rt_uint32_t byte_time_out)
{
	rt_err_t ret = RT_EOK;
	rt_uint8_t recv_buf[256] = {'\0'};
	rt_uint32_t recv_len = 0;

	if (RT_EOK == recv_data_from_485(Collector_port, recv_buf, &recv_len, RT_WAITING_FOREVER, byte_time_out)) {
#if DEBUG
		rt_uint32_t k = 0;
		printf_syn("\nrecv_len = %d\n", recv_len);
		printf_syn("recv buf: ");
		for (k= 0; k < recv_len;k++)
		{
 			printf_syn("%x ", recv_buf[k]);
		}
		printf_syn("\n\n");
#endif
		ret = ammeter_mutex_take(ammeter_port, 1000);
		if(ret != RT_EOK) {
			ERROR_PRINTF(("ERROR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
			return RT_ERROR;
		}

		send_data_by_485(get_485_port_from_ammeter_uart(ammeter_port), recv_buf, recv_len);
		rt_memset(recv_buf, 0, sizeof(recv_buf));
		recv_len = 0;

		ret = recv_data_from_485(ammeter_port, recv_buf, &recv_len, format_time_out, byte_time_out);

		if(RT_EOK != ammeter_mutex_release(ammeter_port)) {
			ERROR_PRINTF(("ERROR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
			return RT_ERROR;
		}

		if(ret == RT_EOK) {
			send_data_by_485(get_485_port_from_ammeter_uart(Collector_port), recv_buf, recv_len);
		} else {
			ERROR_PRINTF(("ERROR: func:%s, line(%d)\n", __FUNCTION__, __LINE__));
		}
	}

	return RT_EOK;
}


