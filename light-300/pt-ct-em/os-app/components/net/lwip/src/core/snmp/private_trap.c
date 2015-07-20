/*
*
*private trap function. coded by songhongbin
*
*/
 
#include "lwip/opt.h"
 
#if LWIP_SNMP
 
#include "lwip/inet.h"

#include "lwip/snmp_asn1.h"
#include "lwip/snmp_structs.h"
#include "lwip/snmp_msg.h"
#include "lwip/snmp.h"
#include "lwip/mem.h"
#include "lwip/tcpip.h"
#include "stdlib.h"
#include "lwip/private_trap.h"
#include "lwip/private_mib.h"
#include <sinkinfo_common.h>
#include <sinkinfo_api4mib.h>
#include <sys_cfg_api.h>
#include <finsh.h>
#include <sink_info.h>
#include <ammeter.h>
#include <misc_lib.h>
#include <ade7880_api.h>


#define NUM_PRIVATE_TRAP_LIST    100
 
static unsigned char SNMP_TRAP_0_FLAG = 1;
//static struct ip_addr SNMP_TRAP_0_ADDR;
extern struct snmp_msg_trap trap_msg;

struct trap_list trap_list_bank[NUM_PRIVATE_TRAP_LIST];
struct rt_semaphore get_trap_list_sema;
#define WAIT_MSG_TIMEOUT (get_ticks_of_ms(100))

struct snmp_obj_id 	lt300_netId_oid = {14,{1,3,6,1,4,1,MY_SNMP_ENTERPRISE_ID,2,3,1,1,1,1,0}} ;
struct snmp_obj_id 	lt300_typeId_oid = {14,{1,3,6,1,4,1,MY_SNMP_ENTERPRISE_ID,2,3,1,1,1,2,0}} ;
struct snmp_obj_id 	lt300_pt_sn_oid = {15,{1,3,6,1,4,1,MY_SNMP_ENTERPRISE_ID,2,3,2,3,1,2,0,0}} ;
struct snmp_obj_id 	lt300_ct_sn_oid = {15,{1,3,6,1,4,1,MY_SNMP_ENTERPRISE_ID,2,3,2,4,1,2,0,0}} ;
struct snmp_obj_id 	lt300_master_pt_sn_oid = {15,{1,3,6,1,4,1,MY_SNMP_ENTERPRISE_ID,2,3,2,3,1,2,0,0}} ;
struct snmp_obj_id  lt300_pt_trap_oid ={15,{1,3,6,1,4,1,MY_SNMP_ENTERPRISE_ID,2,3,2,2,1,6,0,0}} ;
struct snmp_obj_id 	lt300_wl_slave_trap_oid = {15,{1,3,6,1,4,1,MY_SNMP_ENTERPRISE_ID,2,3,2,3,1,14,0,0}} ;
struct snmp_obj_id 	lt300_wl_ct_trap_oid = {15,{1,3,6,1,4,1,MY_SNMP_ENTERPRISE_ID,2,3,2,4,1,14,0,0}} ;
struct snmp_obj_id 	lt300_meter_sn_oid = {15,{1,3,6,1,4,1,MY_SNMP_ENTERPRISE_ID,2,3,3,2,1,3,0,0}} ;
struct snmp_obj_id 	lt300_meter_alarm_oid = {15,{1,3,6,1,4,1,MY_SNMP_ENTERPRISE_ID,2,3,3,2,1,26,0,0}} ;

struct snmp_obj_id 	lt300_px_volloss_trap_oid = {15,{1,3,6,1,4,1,MY_SNMP_ENTERPRISE_ID,2,3,3,8,1,4,0,0}} ;
struct snmp_obj_id 	lt300_px_volover_trap_oid = {15,{1,3,6,1,4,1,MY_SNMP_ENTERPRISE_ID,2,3,3,8,1,8,0,0}} ;
struct snmp_obj_id 	lt300_px_volunder_trap_oid = {15,{1,3,6,1,4,1,MY_SNMP_ENTERPRISE_ID,2,3,3,8,1,12,0,0}} ;
struct snmp_obj_id 	lt300_px_phasebreak_trap_oid = {15,{1,3,6,1,4,1,MY_SNMP_ENTERPRISE_ID,2,3,3,8,1,16,0,0}} ;
struct snmp_obj_id 	lt300_px_curloss_trap_oid = {15,{1,3,6,1,4,1,MY_SNMP_ENTERPRISE_ID,2,3,3,8,1,20,0,0}} ;
struct snmp_obj_id 	lt300_px_curover_trap_oid = {15,{1,3,6,1,4,1,MY_SNMP_ENTERPRISE_ID,2,3,3,8,1,24,0,0}} ;
struct snmp_obj_id 	lt300_px_curbreak_trap_oid = {15,{1,3,6,1,4,1,MY_SNMP_ENTERPRISE_ID,2,3,3,8,1,28,0,0}} ;

struct snmp_obj_id 	lt300_em_meterclear_trap_oid = {15,{1,3,6,1,4,1,MY_SNMP_ENTERPRISE_ID,2,3,3,8,1,31,0,0}} ;
struct snmp_obj_id 	lt300_em_demandclear_trap_oid = {15,{1,3,6,1,4,1,MY_SNMP_ENTERPRISE_ID,2,3,3,8,1,34,0,0}} ;
struct snmp_obj_id 	lt300_em_program_trap_oid = {15,{1,3,6,1,4,1,MY_SNMP_ENTERPRISE_ID,2,3,3,8,1,37,0,0}} ;
struct snmp_obj_id 	lt300_em_calibratetime_trap_oid = {15,{1,3,6,1,4,1,MY_SNMP_ENTERPRISE_ID,2,3,3,8,1,40,0,0}} ;
struct snmp_obj_id 	lt300_em_rseqvol_trap_oid = {15,{1,3,6,1,4,1,MY_SNMP_ENTERPRISE_ID,2,3,3,8,1,43,0,0}} ;
struct snmp_obj_id 	lt300_em_rseqcur_trap_oid = {15,{1,3,6,1,4,1,MY_SNMP_ENTERPRISE_ID,2,3,3,8,1,46,0,0}} ;

struct snmp_obj_id 	lt300_connect_state_oid = {14,{1,3,6,1,4,1,MY_SNMP_ENTERPRISE_ID,2,3,1,1,1,6,0}} ;
struct snmp_obj_id 	lt300_em_address_oid = {14,{1,3,6,1,4,1,MY_SNMP_ENTERPRISE_ID,2,3,3,2,1,2,0}} ;
struct snmp_obj_id 	lt300_em_protocol_wiremode_baudrate_oid = {14,{1,3,6,1,4,1,MY_SNMP_ENTERPRISE_ID,2,3,3,2,1,4,0}} ;

struct snmp_obj_id 	lt300_momentary_freeze_data_oid = {15,{1,3,6,1,4,1,MY_SNMP_ENTERPRISE_ID,2,3,3,3,1,5,0,0}} ;
struct snmp_obj_id 	lt300_timing_freeze_data_oid = {15,{1,3,6,1,4,1,MY_SNMP_ENTERPRISE_ID,2,3,3,4,1,6,0,0}} ;
struct snmp_obj_id 	lt300_act_pulse_time_out_oid = {15,{1,3,6,1,4,1,MY_SNMP_ENTERPRISE_ID,2,3,3,2,1,24,0,0}} ;
struct snmp_obj_id 	lt300_react_pulse_time_out_oid = {15,{1,3,6,1,4,1,MY_SNMP_ENTERPRISE_ID,2,3,3,2,1,25,0,0}} ;


//struct snmp_obj_id gReopTrapID_oid = {12,{1,3,6,1,4,1,MY_SNMP_ENTERPRISE_ID,2,2,1,3,0}} ;
//struct snmp_obj_id sysupoidc = {9,{1, 3, 6, 1, 2, 1, 1, 3, 0}} ;
//struct snmp_obj_id snmpv2TrapOid = {11,{1,3,6,1,6,3,1,1,4,1,0}} ;		/*RFC1907*/
EnumString g_EnumPrtSAlrState[] = {
	{E_PrtSAlrState_off, "off"},
	{E_PrtSAlrState_on, "on"},
};

EnumString g_EnumAlrString[] = {
	{E_ALR_WL_SLAVE_RESPONSE_FAIL			  	, "wireless slave response failed"},
	{E_ALR_WL_SLAVE_RESPONSE_CHANGE_TO_NORMAL	, "wireless slave response changed to normal"},
	{E_ALR_WL_MASTER_RESPONSE_FAIL 	           	, "wireless master response failed"},
	{E_ALR_WL_MASTER_RESPONSE_CHANGE_TO_NORMAL	, "wireless master response changed to normal"},
	{E_ALR_CT_POWER_LOSS                       	, "CT power loss"},
	{E_ALR_CT_POWER_CHANGE_TO_NORMAL	      	, "CT power changed to normal"},
	
	{E_ALR_PA_VOLTAGE_LOSS_EVENT_APPEAR			, "phase A voltage losses happened"},
	{E_ALR_PA_VOLTAGE_LOSS_EVENT_DISAPPEAR		, "phase A voltage losses diasppeared"},
	{E_ALR_PA_VOLTAGE_OVER_EVENT_APPEAR			, "phase A voltage over happened"},
	{E_ALR_PA_VOLTAGE_OVER_EVENT_DISAPPEAR		, "phase A voltage over diasppeared"},
	{E_ALR_PA_VOLTAGE_UNDER_EVENT_APPEAR		, "phase A voltage under happened"},
	{E_ALR_PA_VOLTAGE_UNDER_EVENT_DISAPPEAR		, "phase A voltage under diasppeared"},
	{E_ALR_PA_PHASE_BREAK_EVENT_APPEAR			, "phase A phase break happened"},
	{E_ALR_PA_PHASE_BREAK_EVENT_DISAPPEAR		, "phase A phase break diasppeared"},
	{E_ALR_PA_CURRENT_LOSS_EVENT_APPEAR			, "phase A current losses happened"},
	{E_ALR_PA_CURRENT_LOSS_EVENT_DISAPPEAR		, "phase A current losses diasppeared"},
	{E_ALR_PA_CURRENT_OVER_EVENT_APPEAR			, "phase A current over happened"},
	{E_ALR_PA_CURRENT_OVER_EVENT_DISAPPEAR		, "phase A current over diasppeared"},
	{E_ALR_PA_CURRENT_BREAK_EVENT_APPEAR		, "phase A current break happened"},
	{E_ALR_PA_CURRENT_BREAK_EVENT_DISAPPEAR		, "phase A current break diasppeared"},

	{E_ALR_PB_VOLTAGE_LOSS_EVENT_APPEAR			, "phase B voltage losses happened"},
	{E_ALR_PB_VOLTAGE_LOSS_EVENT_DISAPPEAR		, "phase B voltage losses diasppeared"},
	{E_ALR_PB_VOLTAGE_OVER_EVENT_APPEAR			, "phase B voltage over happened"},
	{E_ALR_PB_VOLTAGE_OVER_EVENT_DISAPPEAR		, "phase B voltage over diasppeared"},
	{E_ALR_PB_VOLTAGE_UNDER_EVENT_APPEAR		, "phase B voltage under happened"},
	{E_ALR_PB_VOLTAGE_UNDER_EVENT_DISAPPEAR		, "phase B voltage under diasppeared"},
	{E_ALR_PB_PHASE_BREAK_EVENT_APPEAR			, "phase B phase break happened"},
	{E_ALR_PB_PHASE_BREAK_EVENT_DISAPPEAR		, "phase B phase break diasppeared"},
	{E_ALR_PB_CURRENT_LOSS_EVENT_APPEAR			, "phase B current losses happened"},
	{E_ALR_PB_CURRENT_LOSS_EVENT_DISAPPEAR		, "phase B current losses diasppeared"},
	{E_ALR_PB_CURRENT_OVER_EVENT_APPEAR			, "phase B current over happened"},
	{E_ALR_PB_CURRENT_OVER_EVENT_DISAPPEAR		, "phase B current over diasppeared"},
	{E_ALR_PB_CURRENT_BREAK_EVENT_APPEAR		, "phase B current break happened"},
	{E_ALR_PB_CURRENT_BREAK_EVENT_DISAPPEAR		, "phase B current break diasppeared"},

	{E_ALR_PC_VOLTAGE_LOSS_EVENT_DISAPPEAR		, "phase C voltage losses diasppeared"},
	{E_ALR_PC_VOLTAGE_OVER_EVENT_APPEAR			, "phase C voltage over happened"},
	{E_ALR_PC_VOLTAGE_OVER_EVENT_DISAPPEAR		, "phase C voltage over diasppeared"},
	{E_ALR_PC_VOLTAGE_UNDER_EVENT_APPEAR		, "phase C voltage under happened"},
	{E_ALR_PC_VOLTAGE_UNDER_EVENT_DISAPPEAR		, "phase C voltage under diasppeared"},
	{E_ALR_PC_PHASE_BREAK_EVENT_APPEAR			, "phase C phase break happened"},
	{E_ALR_PC_PHASE_BREAK_EVENT_DISAPPEAR		, "phase C phase break diasppeared"},
	{E_ALR_PC_CURRENT_LOSS_EVENT_APPEAR			, "phase C current losses happened"},
	{E_ALR_PC_CURRENT_LOSS_EVENT_DISAPPEAR		, "phase C current losses diasppeared"},
	{E_ALR_PC_CURRENT_OVER_EVENT_APPEAR			, "phase C current over happened"},
	{E_ALR_PC_CURRENT_OVER_EVENT_DISAPPEAR		, "phase C current over diasppeared"},
	{E_ALR_PC_CURRENT_BREAK_EVENT_APPEAR		, "phase C current break happened"},
	{E_ALR_PC_CURRENT_BREAK_EVENT_DISAPPEAR		, "phase C current break diasppeared"},

	{E_ALR_EM_METER_CLEAR_EVENT_APPEAR			, "meter clear event happened"},
	{E_ALR_EM_METER_CLEAR_EVENT_DISAPPEAR		, "meter clear event disappeared"},
	{E_ALR_EM_DEMAND_CLEAR_EVENT_APPEAR			, "demand clear event happened"},
	{E_ALR_EM_DEMAND_CLEAR_EVENT_DISAPPEAR		, "demand clear event disappeared"},
	{E_ALR_EM_PROGRAM_EVENT_APPEAR				, "program event happened"},
	{E_ALR_EM_PROGRAM_EVENT_DISAPPEAR			, "program event disappeared"},
	{E_ALR_EM_CALIBRATE_TIME_EVENT_APPEAR		, "calibrate time event happened"},
	{E_ALR_EM_CALIBRATE_TIME_EVENT_DISAPPEAR	, "calibrate time event disappeared"},
	{E_ALR_EM_REVERSE_SEQ_VOL_EVENT_APPEAR		, "voltage reverse phase sequence event happened"},
	{E_ALR_EM_REVERSE_SEQ_VOL_EVENT_DISAPPEAR	, "voltage reverse phase sequence disappeared"},
	{E_ALR_EM_REVERSE_SEQ_CUR_EVENT_APPEAR		, "current reverse phase sequence event happened"},
	{E_ALR_EM_REVERSE_SEQ_CUR_EVENT_DISAPPEAR	, "current reverse phase sequence disappeared"},

	{E_ALR_EM_CONNECT_STATE_CHANGE_EVENT		, "EM connect status changed"},
	{E_ALR_EM_PROTOCOL_BANDRATE_WIREMODE_EVENT	, "EM protocol wiremode or bandrate changed"},
	{E_ALR_EM_ADDRESS_CHANGE_EVENT				, "EM PT or CT address have changed"},
	{E_ALR_EM_MOMENTARY_FREEZE_DATA				, "EM momentary freeze data trap"},
	{E_ALR_EM_TIMING_FREEZE_DATA				, "EM timing freeze data trap"},
	{E_ALR_EM_ACT_PULSE_TIME_OUT				, "act energy pluse error for waitting over timeout"},
	{E_ALR_EM_REACT_PULSE_TIME_OUT				, "react energy pluse error for waittting over timeout"},
};

int g_size_EnumAlrString = sizeof(g_EnumAlrString)/sizeof(g_EnumAlrString[0]); 

/*find max day of month in certain year,zk,20070627*/
int maxday_of_month(int month, int year)
{
    int maxdayofmonth = 0;
    switch(month)
    {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
            maxdayofmonth = 31;
            break;
        case 4:
        case 6:
        case 9:
        case 11:
            maxdayofmonth = 30;
            break;
        case 2:
            if(((0 == year % 4) && (0 != year % 100))
                ||(0 == year % 400))
            {
                maxdayofmonth = 29;
            }
            else
            {
                maxdayofmonth = 28;
            }
            break;
        default:
            break;
    }
    return maxdayofmonth;
}

char* db_get_alarm_string( rt_uint16_t alarmNum )
{
	static char null_string[1]={0};
	int i;
	for( i =0; i< g_size_EnumAlrString; i++ ){
		if( alarmNum == g_EnumAlrString[i].value )
			return g_EnumAlrString[i].string;
	}
	return null_string;
}

void snmp_trap_init()
{
	struct nms_if_info_st nms_if;
	/* Set SNMP Trap destination */
	/*IP4_ADDR( &SNMP_TRAP_0_ADDR, 172, 16, 7, 7);
 	snmp_trap_dst_ip_set(0,&SNMP_TRAP_0_ADDR);
 	snmp_trap_dst_enable(0,SNMP_TRAP_0_FLAG);*/
	read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);

	nms_if.trap_ip = lwip_htonl(nms_if.trap_ip);
	snmp_trap_dst_ip_set(0, (ip_addr_t *)&nms_if.trap_ip);
	snmp_trap_dst_enable(0, SNMP_TRAP_0_FLAG);
   
	rt_sem_init(&get_trap_list_sema, "trap", 0, RT_IPC_FLAG_FIFO);
}
 
struct trap_list * get_next_free_private_trap_list()
{
 	rt_uint8_t index;
 	struct trap_list * result = NULL;
   
	//  SemaphoreTake(getTrapListSema, (portTickType) 10);
	rt_sem_take(&get_trap_list_sema, WAIT_MSG_TIMEOUT);
   
	for(index = 0; index < NUM_PRIVATE_TRAP_LIST; index++){
		if(!trap_list_bank[index].in_use){
			trap_list_bank[index].in_use = 1;
			result = &trap_list_bank[index];         
			break;
		}
	}
   
	rt_sem_release(&get_trap_list_sema);
 	return result;  
}
 
void free_private_trap_list(struct trap_list * list)
{
	snmp_varbind_list_free(&list->vb_root);
	list->ent_oid = NULL;
	list->spc_trap = 0;
	list->in_use = 0;
}
 
void send_trap_callback( void * parameters )
{
	struct trap_list * param;
   
	if( parameters != NULL ){
 		param = (struct trap_list *) parameters;
      
 		trap_msg.outvb = param->vb_root;
      
		if(ERR_OK == snmp_send_trap(SNMP_GENTRAP_ENTERPRISESPC,param->ent_oid,param->spc_trap))
			printf_syn("trap send success\n");

		free_private_trap_list(param);
	}
}
 
void snmp_trap_entry( void * pvParameters )
{
 	
 	struct snmp_obj_id objid = {MY_SNMP_SYSOBJID_LEN, MY_SNMP_SYSOBJID};
 	static unsigned char msg[]  = "Hello!Yeejoin light300 device trap";
 	static unsigned char msg2[] = "light300_device_second_trap";
 	static unsigned char msglen= 34;
   
	struct snmp_varbind *vb;
	struct trap_list *vb_list;
	(void) pvParameters;
   
	while (1) {
		
      
		vb_list = get_next_free_private_trap_list();
		vb_list->ent_oid = &objid;
		vb_list->spc_trap = 12;
      
		vb = snmp_varbind_alloc(&objid, SNMP_ASN1_OPAQUE, msglen);
      
		if (vb != NULL){
			rt_memcpy (vb->value, &msg, msglen);         
			snmp_varbind_tail_add(&vb_list->vb_root,vb);
		}
      
		vb = snmp_varbind_alloc(&objid, SNMP_ASN1_OPAQUE, sizeof(msg2)-1);
      
		if (vb != NULL){
			rt_memcpy (vb->value, &msg2, sizeof(msg2)-1);         
			snmp_varbind_tail_add(&vb_list->vb_root,vb);
		}
		tcpip_callback(send_trap_callback, vb_list);
      
	// Wait for the next cycle.
		rt_thread_delay(get_ticks_of_ms(100));
	}
   
}

int netid_portid_insert(rt_uint8_t netid, rt_uint8_t portid)
{
	lt300_typeId_oid.id[lt300_typeId_oid.len - 1] = netid;
	lt300_netId_oid.id[lt300_netId_oid.len - 1] = netid;
	
	lt300_pt_sn_oid.id[lt300_pt_sn_oid.len - 2] = netid;
	lt300_ct_sn_oid.id[lt300_ct_sn_oid.len - 2] = netid;
	lt300_master_pt_sn_oid.id[lt300_master_pt_sn_oid.len - 2] = netid;
	
	lt300_pt_trap_oid.id[lt300_pt_trap_oid.len - 2] = netid;
	lt300_wl_slave_trap_oid.id[lt300_wl_slave_trap_oid.len - 2] = netid;
	lt300_wl_ct_trap_oid.id[lt300_wl_ct_trap_oid.len - 2] = netid;

	lt300_pt_sn_oid.id[lt300_pt_sn_oid.len - 1] = portid;
	lt300_ct_sn_oid.id[lt300_ct_sn_oid.len - 1] = portid;
	lt300_master_pt_sn_oid.id[lt300_master_pt_sn_oid.len - 1] = portid;
	
	lt300_pt_trap_oid.id[lt300_pt_trap_oid.len - 1] = portid;
	lt300_wl_slave_trap_oid.id[lt300_wl_slave_trap_oid.len - 1] = portid;
	lt300_wl_ct_trap_oid.id[lt300_wl_ct_trap_oid.len - 1] = portid;

	lt300_meter_sn_oid.id[lt300_meter_sn_oid.len - 2] = netid;
	lt300_meter_sn_oid.id[lt300_meter_sn_oid.len - 1] = portid;
	lt300_meter_alarm_oid.id[lt300_meter_sn_oid.len - 2] = netid;
	lt300_meter_alarm_oid.id[lt300_meter_sn_oid.len - 1] = portid;

	lt300_px_volloss_trap_oid.id[lt300_px_volloss_trap_oid.len - 2] = netid;
	lt300_px_volloss_trap_oid.id[lt300_px_volloss_trap_oid.len - 1] = portid;
	lt300_px_volover_trap_oid.id[lt300_px_volover_trap_oid.len - 2] = netid;
	lt300_px_volover_trap_oid.id[lt300_px_volover_trap_oid.len - 1] = portid;
	lt300_px_volunder_trap_oid.id[lt300_px_volunder_trap_oid.len - 2] = netid;
	lt300_px_volunder_trap_oid.id[lt300_px_volunder_trap_oid.len - 1] = portid;
	lt300_px_phasebreak_trap_oid.id[lt300_px_phasebreak_trap_oid.len - 2] = netid;
	lt300_px_phasebreak_trap_oid.id[lt300_px_phasebreak_trap_oid.len - 1] = portid;
	lt300_px_curloss_trap_oid.id[lt300_px_curloss_trap_oid.len - 2] = netid;
	lt300_px_curloss_trap_oid.id[lt300_px_curloss_trap_oid.len - 1] = portid;
	lt300_px_curover_trap_oid.id[lt300_px_curover_trap_oid.len - 2] = netid;
	lt300_px_curover_trap_oid.id[lt300_px_curover_trap_oid.len - 1] = portid;
	lt300_px_curbreak_trap_oid.id[lt300_px_curbreak_trap_oid.len - 2] = netid;
	lt300_px_curbreak_trap_oid.id[lt300_px_curbreak_trap_oid.len - 1] = portid;
	
	lt300_em_meterclear_trap_oid.id[lt300_em_meterclear_trap_oid.len - 2] = netid;
	lt300_em_meterclear_trap_oid.id[lt300_em_meterclear_trap_oid.len - 1] = portid;
	lt300_em_demandclear_trap_oid.id[lt300_em_demandclear_trap_oid.len - 2] = netid;
	lt300_em_demandclear_trap_oid.id[lt300_em_demandclear_trap_oid.len - 1] = portid;
	lt300_em_program_trap_oid.id[lt300_em_program_trap_oid.len - 2] = netid;
	lt300_em_program_trap_oid.id[lt300_em_program_trap_oid.len - 1] = portid;
	lt300_em_calibratetime_trap_oid.id[lt300_em_calibratetime_trap_oid.len - 2] = netid;
	lt300_em_calibratetime_trap_oid.id[lt300_em_calibratetime_trap_oid.len - 1] = portid;
	lt300_em_rseqvol_trap_oid.id[lt300_em_rseqvol_trap_oid.len - 2] = netid;
	lt300_em_rseqvol_trap_oid.id[lt300_em_rseqvol_trap_oid.len - 1] = portid;
	lt300_em_rseqcur_trap_oid.id[lt300_em_rseqcur_trap_oid.len - 2] = netid;
	lt300_em_rseqcur_trap_oid.id[lt300_em_rseqcur_trap_oid.len - 1] = portid;

	lt300_connect_state_oid.id[lt300_connect_state_oid.len - 1] = netid;
	lt300_em_address_oid.id[lt300_em_address_oid.len - 1] = netid;
	lt300_em_protocol_wiremode_baudrate_oid.id[lt300_em_protocol_wiremode_baudrate_oid.len - 1] = netid;
	
	lt300_momentary_freeze_data_oid.id[lt300_momentary_freeze_data_oid.len - 2] = netid;
	lt300_momentary_freeze_data_oid.id[lt300_momentary_freeze_data_oid.len - 1] = portid;
	lt300_timing_freeze_data_oid.id[lt300_timing_freeze_data_oid.len - 2] = netid;
	lt300_timing_freeze_data_oid.id[lt300_timing_freeze_data_oid.len - 1] = portid;

	lt300_act_pulse_time_out_oid.id[lt300_act_pulse_time_out_oid.len - 2] = netid;
	lt300_act_pulse_time_out_oid.id[lt300_act_pulse_time_out_oid.len - 1] = portid;
	lt300_react_pulse_time_out_oid.id[lt300_react_pulse_time_out_oid.len - 2] = netid;
	lt300_react_pulse_time_out_oid.id[lt300_react_pulse_time_out_oid.len - 1] = portid;

	return RT_EOK;

}

int trap_send(rt_uint8_t portid,rt_uint8_t gtrap,rt_uint8_t strap, rt_uint8_t alarmcode, rt_uint8_t flag)
{
	struct snmp_obj_id objid = {MY_SNMP_SYSOBJID_LEN, MY_SNMP_SYSOBJID};
	struct snmp_varbind *vb;
	struct trap_list *vb_list;
	struct electric_meter_reg_info_st amm_sn;
	//struct celectric_meter_config_info_st amm_conf;
	rt_uint32_t data_len = 0;
	rt_uint8_t eventdata[128] ={0};
	char* message = db_get_alarm_string(alarmcode);
	rt_uint8_t neid = get_net_id();
	
	vb_list = get_next_free_private_trap_list();
	vb_list->ent_oid = &objid;
	vb_list->spc_trap = strap;

 	netid_portid_insert(neid,portid+1);

	 if (SUCC == get_em_reg_info(&amm_sn)) {
		 printf_syn("%s(), read meter SN data tbl succ\n", __FUNCTION__);
	 } else {
		 printf_syn("%s(), read meter SN data tbl fail\n", __FUNCTION__);
	 }

     /*EM type*/
	struct m3_sys_info_st lt300_sys_info;
	read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_SYS_INFO, 0, &lt300_sys_info);
	vb = snmp_varbind_alloc(&lt300_typeId_oid, SNMP_ASN1_OPAQUE,  rt_strlen(lt300_sys_info.dev_descr));
	if (vb != NULL){
		 rt_memcpy (vb->value, lt300_sys_info.dev_descr, rt_strlen(lt300_sys_info.dev_descr));	
		 snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 }
	 
	 /*EM net id*/
	vb = snmp_varbind_alloc(&lt300_netId_oid, SNMP_ASN1_OC_STR, 12);
	if (vb != NULL){
		 get_dev_sn_em_sn(SDT_DEV, vb->value, DEV_SN_BUF_CHARS_NUM_MAX, portid);
		 snmp_varbind_tail_add(&vb_list->vb_root,vb);
	}

    switch(alarmcode){
		case E_ALR_WL_SLAVE_RESPONSE_FAIL :
		case E_ALR_WL_SLAVE_RESPONSE_CHANGE_TO_NORMAL:
			/*PT CT sn id*/
			if(flag == FLAG_PT){
	 			vb = snmp_varbind_alloc(&lt300_pt_sn_oid, SNMP_ASN1_OC_STR, 12);
	 			if (vb != NULL){
					get_dev_sn_em_sn(SDT_PT, vb->value, DEV_SN_BUF_CHARS_NUM_MAX, portid);		
					snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 			}
			}else if(flag == FLAG_CT){
	 			vb = snmp_varbind_alloc(&lt300_ct_sn_oid, SNMP_ASN1_OC_STR, 12);
	 			if (vb != NULL){
					get_dev_sn_em_sn(SDT_CT, vb->value, DEV_SN_BUF_CHARS_NUM_MAX, portid);		
					snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 			}
			}
			vb = snmp_varbind_alloc(&lt300_wl_slave_trap_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
		break;
		case E_ALR_WL_MASTER_RESPONSE_FAIL :
		case E_ALR_WL_MASTER_RESPONSE_CHANGE_TO_NORMAL:
			vb = snmp_varbind_alloc(&lt300_master_pt_sn_oid, SNMP_ASN1_OC_STR, 12);
	 		if (vb != NULL){
				get_dev_sn_em_sn(SDT_MASTER_PT, vb->value, DEV_SN_BUF_CHARS_NUM_MAX, portid);		
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			vb = snmp_varbind_alloc(&lt300_pt_trap_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
		break;
		case E_ALR_CT_POWER_LOSS :
		case E_ALR_CT_POWER_CHANGE_TO_NORMAL:
			vb = snmp_varbind_alloc(&lt300_ct_sn_oid, SNMP_ASN1_OC_STR, 12);
	 		if (vb != NULL){
				get_dev_sn_em_sn(SDT_MASTER_PT, vb->value, DEV_SN_BUF_CHARS_NUM_MAX, portid);		
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			vb = snmp_varbind_alloc(&lt300_wl_ct_trap_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
		break;
		case E_ALR_PA_VOLTAGE_LOSS_EVENT_APPEAR:
		case E_ALR_PA_VOLTAGE_LOSS_EVENT_DISAPPEAR:{
			u32_t volloss_data[32]= {0};
			struct sinkinfo_em_volloss_event_st volloss_event;
			
			vb = snmp_varbind_alloc(&lt300_meter_sn_oid, SNMP_ASN1_OC_STR, 12);
	 		if (vb != NULL){
				/*if (SUCC == get_em_reg_info(&amm_sn)) {
					rt_memcpy(vb->value, amm_sn.em_sn[portid], DEV_SN_MODE_LEN);
				} else {
					printf_syn("%s(), read meter SN data tbl fail\n", __FUNCTION__);
				}*/
				get_dev_sn_em_sn(SDT_ELECTRIC_METER, vb->value, DEV_SN_MODE_LEN, portid);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			vb = snmp_varbind_alloc(&lt300_meter_alarm_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			if (FRAME_E_OK != get_event_data_from_ammeter((rt_uint8_t *)amm_sn.em_sn[portid], AMM_EVENT_A_LOSE_VOLTAGE, 1, eventdata, &data_len, RS485_PORT_USED_BY_645)){
				printf_syn("%s(), meter pa vol loss event data read fail\n", __FUNCTION__);

			}else{
				volloss_event.start_time[0] = eventdata[5];
				volloss_event.start_time[1] = eventdata[4];
				volloss_event.start_time[2] = eventdata[3];
				volloss_event.start_time[3] = eventdata[2];
				volloss_event.start_time[4] = eventdata[1];
				volloss_event.start_time[5] = eventdata[0];
				volloss_event.end_time[0] = eventdata[11];
				volloss_event.end_time[1] = eventdata[10];
				volloss_event.end_time[2] = eventdata[9];
				volloss_event.end_time[3] = eventdata[8];
				volloss_event.end_time[4] = eventdata[7];
				volloss_event.end_time[5] = eventdata[6];
				volloss_event.vA = eventdata[13]<<8 | eventdata[12];
				volloss_event.iA = eventdata[16]<<16 | eventdata[15]<<8 | eventdata[14];
				volloss_event.apA = eventdata[19]<<16 | eventdata[18]<<8 | eventdata[17];
				volloss_event.rapA = eventdata[22]<<16 | eventdata[21]<<8 | eventdata[20];
				volloss_event.pfA = eventdata[24]<<8 | eventdata[23];
				volloss_event.vB = eventdata[26]<<8 | eventdata[25];
				volloss_event.iB = eventdata[29]<<16 | eventdata[28]<<8 | eventdata[27];
				volloss_event.apB = eventdata[32]<<16 | eventdata[31]<<8 | eventdata[30];
				volloss_event.rapB = eventdata[35]<<16 | eventdata[34]<<8 | eventdata[33];
				volloss_event.pfB = eventdata[37]<<8 | eventdata[36];
				volloss_event.vC = eventdata[39]<<8 | eventdata[38];
				volloss_event.iC = eventdata[42]<<16 | eventdata[41]<<8 | eventdata[40];
				volloss_event.apC = eventdata[45]<<16 | eventdata[44]<<8 | eventdata[43];
				volloss_event.rapC = eventdata[48]<<16 | eventdata[47]<<8 | eventdata[46];
				volloss_event.pfC = eventdata[50]<<8 | eventdata[49];
				volloss_event.ahnA = eventdata[54]<<24 | eventdata[53]<<16 | eventdata[52]<<8 | eventdata[51];
				volloss_event.ahnB = eventdata[58]<<24 | eventdata[57]<<16 | eventdata[56]<<8 | eventdata[55];
				volloss_event.ahnC = eventdata[62]<<24 | eventdata[61]<<16 | eventdata[60]<<8 | eventdata[59];

				volloss_data[0] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.vA));
				volloss_data[1] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.iA));
				volloss_data[2] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.apA));
				volloss_data[3] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.rapA));
				volloss_data[4] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.pfA));
				volloss_data[5] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.vB));
				volloss_data[6] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.iB));
				volloss_data[7] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.apB));
				volloss_data[8] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.rapB));
				volloss_data[9] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.pfB));
				volloss_data[10] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.vC));
				volloss_data[11] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.iC));
				volloss_data[12] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.apC));
				volloss_data[13] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.rapC));
				volloss_data[14] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.pfC));
				volloss_data[15] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.ahnA));
				volloss_data[16] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.ahnB));
				volloss_data[17] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.ahnC));
			}
			vb = snmp_varbind_alloc(&lt300_px_volloss_trap_oid, SNMP_ASN1_OC_STR, sizeof(volloss_event)-2*2);
	 		if (vb != NULL){
				rt_memcpy((u8_t *)vb->value, volloss_event.start_time, sizeof(volloss_event.start_time)-2);
				rt_memcpy(((u8_t *)vb->value)+6, volloss_event.end_time, sizeof(volloss_event.end_time)-2);
				rt_memcpy(((u8_t *)vb->value)+12, (u8_t *)&volloss_data[0], sizeof(volloss_event.vA));
				rt_memcpy(((u8_t *)vb->value)+16, (u8_t *)&volloss_data[1], sizeof(volloss_event.iA));
				rt_memcpy(((u8_t *)vb->value)+20, (u8_t *)&volloss_data[2], sizeof(volloss_event.apA));
				rt_memcpy(((u8_t *)vb->value)+24, (u8_t *)&volloss_data[3], sizeof(volloss_event.rapA));
				rt_memcpy(((u8_t *)vb->value)+28, (u8_t *)&volloss_data[4], sizeof(volloss_event.pfA));
				rt_memcpy(((u8_t *)vb->value)+32, (u8_t *)&volloss_data[5], sizeof(volloss_event.vB));
				rt_memcpy(((u8_t *)vb->value)+36, (u8_t *)&volloss_data[6], sizeof(volloss_event.iB));
				rt_memcpy(((u8_t *)vb->value)+40, (u8_t *)&volloss_data[7], sizeof(volloss_event.apB));
				rt_memcpy(((u8_t *)vb->value)+44, (u8_t *)&volloss_data[8], sizeof(volloss_event.rapB));
				rt_memcpy(((u8_t *)vb->value)+48, (u8_t *)&volloss_data[9], sizeof(volloss_event.pfB));
				rt_memcpy(((u8_t *)vb->value)+52, (u8_t *)&volloss_data[10], sizeof(volloss_event.vC));
				rt_memcpy(((u8_t *)vb->value)+56, (u8_t *)&volloss_data[11], sizeof(volloss_event.iC));
				rt_memcpy(((u8_t *)vb->value)+60, (u8_t *)&volloss_data[12], sizeof(volloss_event.apC));
				rt_memcpy(((u8_t *)vb->value)+64, (u8_t *)&volloss_data[13], sizeof(volloss_event.rapC));
				rt_memcpy(((u8_t *)vb->value)+68, (u8_t *)&volloss_data[14], sizeof(volloss_event.pfC));
				rt_memcpy(((u8_t *)vb->value)+72, (u8_t *)&volloss_data[15], sizeof(volloss_event.ahnA));
				rt_memcpy(((u8_t *)vb->value)+76, (u8_t *)&volloss_data[16], sizeof(volloss_event.ahnB));
				rt_memcpy(((u8_t *)vb->value)+80, (u8_t *)&volloss_data[17], sizeof(volloss_event.ahnC));
			//	rt_memcpy((struct sinkinfo_em_volloss_event_st *)vb->value, &sinkinfo_all_em[portid].si_em_volloss_event_pa[1], SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
		}
		break;
		case E_ALR_PA_VOLTAGE_OVER_EVENT_APPEAR:
		case E_ALR_PA_VOLTAGE_OVER_EVENT_DISAPPEAR:{
			u32_t volloss_data[32]= {0};
			struct sinkinfo_em_volloss_event_st volloss_event;
			
			vb = snmp_varbind_alloc(&lt300_meter_sn_oid, SNMP_ASN1_OC_STR, 12);
	 		if (vb != NULL){
				/*if (SUCC == get_em_reg_info(&amm_sn)) {
					rt_memcpy(vb->value, amm_sn.em_sn[portid], DEV_SN_MODE_LEN);
				} else {
					printf_syn("%s(), read meter SN data tbl fail\n", __FUNCTION__);
				}*/
				get_dev_sn_em_sn(SDT_ELECTRIC_METER, vb->value, DEV_SN_MODE_LEN, portid);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			vb = snmp_varbind_alloc(&lt300_meter_alarm_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			if (FRAME_E_OK != get_event_data_from_ammeter((rt_uint8_t *)amm_sn.em_sn[portid], AMM_EVENT_A_OVER_VOLTAGE, 1, eventdata, &data_len, RS485_PORT_USED_BY_645)){
				printf_syn("%s(), meter pa vol over event data read fail\n", __FUNCTION__);

			}else{
				volloss_event.start_time[0] = eventdata[5];
				volloss_event.start_time[1] = eventdata[4];
				volloss_event.start_time[2] = eventdata[3];
				volloss_event.start_time[3] = eventdata[2];
				volloss_event.start_time[4] = eventdata[1];
				volloss_event.start_time[5] = eventdata[0];
				volloss_event.end_time[0] = eventdata[11];
				volloss_event.end_time[1] = eventdata[10];
				volloss_event.end_time[2] = eventdata[9];
				volloss_event.end_time[3] = eventdata[8];
				volloss_event.end_time[4] = eventdata[7];
				volloss_event.end_time[5] = eventdata[6];
				volloss_event.vA = eventdata[13]<<8 | eventdata[12];
				volloss_event.iA = eventdata[16]<<16 | eventdata[15]<<8 | eventdata[14];
				volloss_event.apA = eventdata[19]<<16 | eventdata[18]<<8 | eventdata[17];
				volloss_event.rapA = eventdata[22]<<16 | eventdata[21]<<8 | eventdata[20];
				volloss_event.pfA = eventdata[24]<<8 | eventdata[23];
				volloss_event.vB = eventdata[26]<<8 | eventdata[25];
				volloss_event.iB = eventdata[29]<<16 | eventdata[28]<<8 | eventdata[27];
				volloss_event.apB = eventdata[32]<<16 | eventdata[31]<<8 | eventdata[30];
				volloss_event.rapB = eventdata[35]<<16 | eventdata[34]<<8 | eventdata[33];
				volloss_event.pfB = eventdata[37]<<8 | eventdata[36];
				volloss_event.vC = eventdata[39]<<8 | eventdata[38];
				volloss_event.iC = eventdata[42]<<16 | eventdata[41]<<8 | eventdata[40];
				volloss_event.apC = eventdata[45]<<16 | eventdata[44]<<8 | eventdata[43];
				volloss_event.rapC = eventdata[48]<<16 | eventdata[47]<<8 | eventdata[46];
				volloss_event.pfC = eventdata[50]<<8 | eventdata[49];
				volloss_event.ahnA = eventdata[54]<<24 | eventdata[53]<<16 | eventdata[52]<<8 | eventdata[51];
				volloss_event.ahnB = eventdata[58]<<24 | eventdata[57]<<16 | eventdata[56]<<8 | eventdata[55];
				volloss_event.ahnC = eventdata[62]<<24 | eventdata[61]<<16 | eventdata[60]<<8 | eventdata[59];
				
				volloss_data[0] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.vA));
				volloss_data[1] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.iA));
				volloss_data[2] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.apA));
				volloss_data[3] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.rapA));
				volloss_data[4] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.pfA));
				volloss_data[5] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.vB));
				volloss_data[6] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.iB));
				volloss_data[7] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.apB));
				volloss_data[8] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.rapB));
				volloss_data[9] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.pfB));
				volloss_data[10] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.vC));
				volloss_data[11] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.iC));
				volloss_data[12] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.apC));
				volloss_data[13] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.rapC));
				volloss_data[14] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.pfC));
				volloss_data[15] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.ahnA));
				volloss_data[16] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.ahnB));
				volloss_data[17] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.ahnC));

			}		
			vb = snmp_varbind_alloc(&lt300_px_volover_trap_oid, SNMP_ASN1_OC_STR, sizeof(volloss_event)-2*2);
	 		if (vb != NULL){
				rt_memcpy((u8_t *)vb->value, volloss_event.start_time, sizeof(volloss_event.start_time)-2);
				rt_memcpy(((u8_t *)vb->value)+6, volloss_event.end_time, sizeof(volloss_event.end_time)-2);
				rt_memcpy(((u8_t *)vb->value)+12, (u8_t *)&volloss_data[0], sizeof(volloss_event.vA));
				rt_memcpy(((u8_t *)vb->value)+16, (u8_t *)&volloss_data[1], sizeof(volloss_event.iA));
				rt_memcpy(((u8_t *)vb->value)+20, (u8_t *)&volloss_data[2], sizeof(volloss_event.apA));
				rt_memcpy(((u8_t *)vb->value)+24, (u8_t *)&volloss_data[3], sizeof(volloss_event.rapA));
				rt_memcpy(((u8_t *)vb->value)+28, (u8_t *)&volloss_data[4], sizeof(volloss_event.pfA));
				rt_memcpy(((u8_t *)vb->value)+32, (u8_t *)&volloss_data[5], sizeof(volloss_event.vB));
				rt_memcpy(((u8_t *)vb->value)+36, (u8_t *)&volloss_data[6], sizeof(volloss_event.iB));
				rt_memcpy(((u8_t *)vb->value)+40, (u8_t *)&volloss_data[7], sizeof(volloss_event.apB));
				rt_memcpy(((u8_t *)vb->value)+44, (u8_t *)&volloss_data[8], sizeof(volloss_event.rapB));
				rt_memcpy(((u8_t *)vb->value)+48, (u8_t *)&volloss_data[9], sizeof(volloss_event.pfB));
				rt_memcpy(((u8_t *)vb->value)+52, (u8_t *)&volloss_data[10], sizeof(volloss_event.vC));
				rt_memcpy(((u8_t *)vb->value)+56, (u8_t *)&volloss_data[11], sizeof(volloss_event.iC));
				rt_memcpy(((u8_t *)vb->value)+60, (u8_t *)&volloss_data[12], sizeof(volloss_event.apC));
				rt_memcpy(((u8_t *)vb->value)+64, (u8_t *)&volloss_data[13], sizeof(volloss_event.rapC));
				rt_memcpy(((u8_t *)vb->value)+68, (u8_t *)&volloss_data[14], sizeof(volloss_event.pfC));
				rt_memcpy(((u8_t *)vb->value)+72, (u8_t *)&volloss_data[15], sizeof(volloss_event.ahnA));
				rt_memcpy(((u8_t *)vb->value)+76, (u8_t *)&volloss_data[16], sizeof(volloss_event.ahnB));
				rt_memcpy(((u8_t *)vb->value)+80, (u8_t *)&volloss_data[17], sizeof(volloss_event.ahnC));
				//rt_memcpy((struct sinkinfo_em_volloss_event_st *)vb->value, &sinkinfo_all_em[portid].si_em_volover_event_pa[1], SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE);		
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
		}
		break;
		case E_ALR_PA_VOLTAGE_UNDER_EVENT_APPEAR:
		case E_ALR_PA_VOLTAGE_UNDER_EVENT_DISAPPEAR:{
			u32_t volloss_data[32]= {0};
			struct sinkinfo_em_volloss_event_st volloss_event;
			
			vb = snmp_varbind_alloc(&lt300_meter_sn_oid, SNMP_ASN1_OC_STR, 12);
	 		if (vb != NULL){
				/*if (SUCC == get_em_reg_info(&amm_sn)) {
					rt_memcpy(vb->value, amm_sn.em_sn[portid], DEV_SN_MODE_LEN);
				} else {
					printf_syn("%s(), read meter SN data tbl fail\n", __FUNCTION__);
				}*/
				get_dev_sn_em_sn(SDT_ELECTRIC_METER, vb->value, DEV_SN_MODE_LEN, portid);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			vb = snmp_varbind_alloc(&lt300_meter_alarm_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			if (FRAME_E_OK != get_event_data_from_ammeter((rt_uint8_t *)amm_sn.em_sn[portid], AMM_EVENT_A_OWE_VOLTAGE, 1, eventdata, &data_len, RS485_PORT_USED_BY_645)){
				printf_syn("%s(), meter pa vol under event data read fail\n", __FUNCTION__);

			}else{
				volloss_event.start_time[0] = eventdata[5];
				volloss_event.start_time[1] = eventdata[4];
				volloss_event.start_time[2] = eventdata[3];
				volloss_event.start_time[3] = eventdata[2];
				volloss_event.start_time[4] = eventdata[1];
				volloss_event.start_time[5] = eventdata[0];
				volloss_event.end_time[0] = eventdata[11];
				volloss_event.end_time[1] = eventdata[10];
				volloss_event.end_time[2] = eventdata[9];
				volloss_event.end_time[3] = eventdata[8];
				volloss_event.end_time[4] = eventdata[7];
				volloss_event.end_time[5] = eventdata[6];
				volloss_event.vA = eventdata[13]<<8 | eventdata[12];
				volloss_event.iA = eventdata[16]<<16 | eventdata[15]<<8 | eventdata[14];
				volloss_event.apA = eventdata[19]<<16 | eventdata[18]<<8 | eventdata[17];
				volloss_event.rapA = eventdata[22]<<16 | eventdata[21]<<8 | eventdata[20];
				volloss_event.pfA = eventdata[24]<<8 | eventdata[23];
				volloss_event.vB = eventdata[26]<<8 | eventdata[25];
				volloss_event.iB = eventdata[29]<<16 | eventdata[28]<<8 | eventdata[27];
				volloss_event.apB = eventdata[32]<<16 | eventdata[31]<<8 | eventdata[30];
				volloss_event.rapB = eventdata[35]<<16 | eventdata[34]<<8 | eventdata[33];
				volloss_event.pfB = eventdata[37]<<8 | eventdata[36];
				volloss_event.vC = eventdata[39]<<8 | eventdata[38];
				volloss_event.iC = eventdata[42]<<16 | eventdata[41]<<8 | eventdata[40];
				volloss_event.apC = eventdata[45]<<16 | eventdata[44]<<8 | eventdata[43];
				volloss_event.rapC = eventdata[48]<<16 | eventdata[47]<<8 | eventdata[46];
				volloss_event.pfC = eventdata[50]<<8 | eventdata[49];
				volloss_event.ahnA = eventdata[54]<<24 | eventdata[53]<<16 | eventdata[52]<<8 | eventdata[51];
				volloss_event.ahnB = eventdata[58]<<24 | eventdata[57]<<16 | eventdata[56]<<8 | eventdata[55];
				volloss_event.ahnC = eventdata[62]<<24 | eventdata[61]<<16 | eventdata[60]<<8 | eventdata[59];
				
				volloss_data[0] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.vA));
				volloss_data[1] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.iA));
				volloss_data[2] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.apA));
				volloss_data[3] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.rapA));
				volloss_data[4] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.pfA));
				volloss_data[5] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.vB));
				volloss_data[6] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.iB));
				volloss_data[7] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.apB));
				volloss_data[8] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.rapB));
				volloss_data[9] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.pfB));
				volloss_data[10] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.vC));
				volloss_data[11] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.iC));
				volloss_data[12] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.apC));
				volloss_data[13] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.rapC));
				volloss_data[14] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.pfC));
				volloss_data[15] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.ahnA));
				volloss_data[16] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.ahnB));
				volloss_data[17] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.ahnC));

			}
			
			vb = snmp_varbind_alloc(&lt300_px_volunder_trap_oid, SNMP_ASN1_OC_STR, sizeof(volloss_event)-2*2);
	 		if (vb != NULL){
				rt_memcpy((u8_t *)vb->value, volloss_event.start_time, sizeof(volloss_event.start_time)-2);
				rt_memcpy(((u8_t *)vb->value)+6, volloss_event.end_time, sizeof(volloss_event.end_time)-2);
				rt_memcpy(((u8_t *)vb->value)+12, (u8_t *)&volloss_data[0], sizeof(volloss_event.vA));
				rt_memcpy(((u8_t *)vb->value)+16, (u8_t *)&volloss_data[1], sizeof(volloss_event.iA));
				rt_memcpy(((u8_t *)vb->value)+20, (u8_t *)&volloss_data[2], sizeof(volloss_event.apA));
				rt_memcpy(((u8_t *)vb->value)+24, (u8_t *)&volloss_data[3], sizeof(volloss_event.rapA));
				rt_memcpy(((u8_t *)vb->value)+28, (u8_t *)&volloss_data[4], sizeof(volloss_event.pfA));
				rt_memcpy(((u8_t *)vb->value)+32, (u8_t *)&volloss_data[5], sizeof(volloss_event.vB));
				rt_memcpy(((u8_t *)vb->value)+36, (u8_t *)&volloss_data[6], sizeof(volloss_event.iB));
				rt_memcpy(((u8_t *)vb->value)+40, (u8_t *)&volloss_data[7], sizeof(volloss_event.apB));
				rt_memcpy(((u8_t *)vb->value)+44, (u8_t *)&volloss_data[8], sizeof(volloss_event.rapB));
				rt_memcpy(((u8_t *)vb->value)+48, (u8_t *)&volloss_data[9], sizeof(volloss_event.pfB));
				rt_memcpy(((u8_t *)vb->value)+52, (u8_t *)&volloss_data[10], sizeof(volloss_event.vC));
				rt_memcpy(((u8_t *)vb->value)+56, (u8_t *)&volloss_data[11], sizeof(volloss_event.iC));
				rt_memcpy(((u8_t *)vb->value)+60, (u8_t *)&volloss_data[12], sizeof(volloss_event.apC));
				rt_memcpy(((u8_t *)vb->value)+64, (u8_t *)&volloss_data[13], sizeof(volloss_event.rapC));
				rt_memcpy(((u8_t *)vb->value)+68, (u8_t *)&volloss_data[14], sizeof(volloss_event.pfC));
				rt_memcpy(((u8_t *)vb->value)+72, (u8_t *)&volloss_data[15], sizeof(volloss_event.ahnA));
				rt_memcpy(((u8_t *)vb->value)+76, (u8_t *)&volloss_data[16], sizeof(volloss_event.ahnB));
				rt_memcpy(((u8_t *)vb->value)+80, (u8_t *)&volloss_data[17], sizeof(volloss_event.ahnC));

				//rt_memcpy((struct sinkinfo_em_volloss_event_st *)vb->value, &sinkinfo_all_em[portid].si_em_volunder_event_pa[1], SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
		}
		break;
		case E_ALR_PA_PHASE_BREAK_EVENT_APPEAR:
		case E_ALR_PA_PHASE_BREAK_EVENT_DISAPPEAR:{
			u32_t volloss_data[32]= {0};
			struct sinkinfo_em_volloss_event_st volloss_event;
			
			vb = snmp_varbind_alloc(&lt300_meter_sn_oid, SNMP_ASN1_OC_STR, 12);
	 		if (vb != NULL){
				get_dev_sn_em_sn(SDT_ELECTRIC_METER, vb->value, DEV_SN_MODE_LEN, portid);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			vb = snmp_varbind_alloc(&lt300_meter_alarm_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			if (FRAME_E_OK != get_event_data_from_ammeter((rt_uint8_t *)amm_sn.em_sn[portid], AMM_EVENT_A_BROKEN_PHASE, 1, eventdata, &data_len, RS485_PORT_USED_BY_645)){
				printf_syn("%s(), meter pa phase break event data read fail\n", __FUNCTION__);

			}else{
				volloss_event.start_time[0] = eventdata[5];
				volloss_event.start_time[1] = eventdata[4];
				volloss_event.start_time[2] = eventdata[3];
				volloss_event.start_time[3] = eventdata[2];
				volloss_event.start_time[4] = eventdata[1];
				volloss_event.start_time[5] = eventdata[0];
				volloss_event.end_time[0] = eventdata[11];
				volloss_event.end_time[1] = eventdata[10];
				volloss_event.end_time[2] = eventdata[9];
				volloss_event.end_time[3] = eventdata[8];
				volloss_event.end_time[4] = eventdata[7];
				volloss_event.end_time[5] = eventdata[6];
				volloss_event.vA = eventdata[13]<<8 | eventdata[12];
				volloss_event.iA = eventdata[16]<<16 | eventdata[15]<<8 | eventdata[14];
				volloss_event.apA = eventdata[19]<<16 | eventdata[18]<<8 | eventdata[17];
				volloss_event.rapA = eventdata[22]<<16 | eventdata[21]<<8 | eventdata[20];
				volloss_event.pfA = eventdata[24]<<8 | eventdata[23];
				volloss_event.vB = eventdata[26]<<8 | eventdata[25];
				volloss_event.iB = eventdata[29]<<16 | eventdata[28]<<8 | eventdata[27];
				volloss_event.apB = eventdata[32]<<16 | eventdata[31]<<8 | eventdata[30];
				volloss_event.rapB = eventdata[35]<<16 | eventdata[34]<<8 | eventdata[33];
				volloss_event.pfB = eventdata[37]<<8 | eventdata[36];
				volloss_event.vC = eventdata[39]<<8 | eventdata[38];
				volloss_event.iC = eventdata[42]<<16 | eventdata[41]<<8 | eventdata[40];
				volloss_event.apC = eventdata[45]<<16 | eventdata[44]<<8 | eventdata[43];
				volloss_event.rapC = eventdata[48]<<16 | eventdata[47]<<8 | eventdata[46];
				volloss_event.pfC = eventdata[50]<<8 | eventdata[49];
				volloss_event.ahnA = eventdata[54]<<24 | eventdata[53]<<16 | eventdata[52]<<8 | eventdata[51];
				volloss_event.ahnB = eventdata[58]<<24 | eventdata[57]<<16 | eventdata[56]<<8 | eventdata[55];
				volloss_event.ahnC = eventdata[62]<<24 | eventdata[61]<<16 | eventdata[60]<<8 | eventdata[59];
				
				volloss_data[0] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.vA));
				volloss_data[1] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.iA));
				volloss_data[2] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.apA));
				volloss_data[3] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.rapA));
				volloss_data[4] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.pfA));
				volloss_data[5] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.vB));
				volloss_data[6] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.iB));
				volloss_data[7] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.apB));
				volloss_data[8] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.rapB));
				volloss_data[9] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.pfB));
				volloss_data[10] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.vC));
				volloss_data[11] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.iC));
				volloss_data[12] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.apC));
				volloss_data[13] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.rapC));
				volloss_data[14] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.pfC));
				volloss_data[15] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.ahnA));
				volloss_data[16] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.ahnB));
				volloss_data[17] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.ahnC));

			}
			vb = snmp_varbind_alloc(&lt300_px_phasebreak_trap_oid, SNMP_ASN1_OC_STR, sizeof(volloss_event)-2*2);
	 		if (vb != NULL){
				rt_memcpy((u8_t *)vb->value, volloss_event.start_time, sizeof(volloss_event.start_time)-2);
				rt_memcpy(((u8_t *)vb->value)+6, volloss_event.end_time, sizeof(volloss_event.end_time)-2);
				rt_memcpy(((u8_t *)vb->value)+12, (u8_t *)&volloss_data[0], sizeof(volloss_event.vA));
				rt_memcpy(((u8_t *)vb->value)+16, (u8_t *)&volloss_data[1], sizeof(volloss_event.iA));
				rt_memcpy(((u8_t *)vb->value)+20, (u8_t *)&volloss_data[2], sizeof(volloss_event.apA));
				rt_memcpy(((u8_t *)vb->value)+24, (u8_t *)&volloss_data[3], sizeof(volloss_event.rapA));
				rt_memcpy(((u8_t *)vb->value)+28, (u8_t *)&volloss_data[4], sizeof(volloss_event.pfA));
				rt_memcpy(((u8_t *)vb->value)+32, (u8_t *)&volloss_data[5], sizeof(volloss_event.vB));
				rt_memcpy(((u8_t *)vb->value)+36, (u8_t *)&volloss_data[6], sizeof(volloss_event.iB));
				rt_memcpy(((u8_t *)vb->value)+40, (u8_t *)&volloss_data[7], sizeof(volloss_event.apB));
				rt_memcpy(((u8_t *)vb->value)+44, (u8_t *)&volloss_data[8], sizeof(volloss_event.rapB));
				rt_memcpy(((u8_t *)vb->value)+48, (u8_t *)&volloss_data[9], sizeof(volloss_event.pfB));
				rt_memcpy(((u8_t *)vb->value)+52, (u8_t *)&volloss_data[10], sizeof(volloss_event.vC));
				rt_memcpy(((u8_t *)vb->value)+56, (u8_t *)&volloss_data[11], sizeof(volloss_event.iC));
				rt_memcpy(((u8_t *)vb->value)+60, (u8_t *)&volloss_data[12], sizeof(volloss_event.apC));
				rt_memcpy(((u8_t *)vb->value)+64, (u8_t *)&volloss_data[13], sizeof(volloss_event.rapC));
				rt_memcpy(((u8_t *)vb->value)+68, (u8_t *)&volloss_data[14], sizeof(volloss_event.pfC));
				rt_memcpy(((u8_t *)vb->value)+72, (u8_t *)&volloss_data[15], sizeof(volloss_event.ahnA));
				rt_memcpy(((u8_t *)vb->value)+76, (u8_t *)&volloss_data[16], sizeof(volloss_event.ahnB));
				rt_memcpy(((u8_t *)vb->value)+80, (u8_t *)&volloss_data[17], sizeof(volloss_event.ahnC));				
				//rt_memcpy((struct sinkinfo_em_volloss_event_st *)vb->value, &sinkinfo_all_em[portid].si_em_phasebreak_event_pa[1], SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
		}
		break;
		case E_ALR_PA_CURRENT_LOSS_EVENT_APPEAR:
		case E_ALR_PA_CURRENT_LOSS_EVENT_DISAPPEAR:{
			u32_t curloss_data[32]= {0};
			struct sinkinfo_em_curloss_event_st curloss_event;
			
			vb = snmp_varbind_alloc(&lt300_meter_sn_oid, SNMP_ASN1_OC_STR, 12);
	 		if (vb != NULL){
				get_dev_sn_em_sn(SDT_ELECTRIC_METER, vb->value, DEV_SN_MODE_LEN, portid);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			vb = snmp_varbind_alloc(&lt300_meter_alarm_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			if (FRAME_E_OK != get_event_data_from_ammeter((rt_uint8_t *)amm_sn.em_sn[portid], AMM_EVENT_A_LOSE_CURRENT, 1, eventdata, &data_len, RS485_PORT_USED_BY_645)){
				printf_syn("%s(), meter pa cur loss event data read fail\n", __FUNCTION__);

			}else{
				curloss_event.start_time[0] = eventdata[5];
				curloss_event.start_time[1] = eventdata[4];
				curloss_event.start_time[2] = eventdata[3];
				curloss_event.start_time[3] = eventdata[2];
				curloss_event.start_time[4] = eventdata[1];
				curloss_event.start_time[5] = eventdata[0];
				curloss_event.end_time[0] = eventdata[11];
				curloss_event.end_time[1] = eventdata[10];
				curloss_event.end_time[2] = eventdata[9];
				curloss_event.end_time[3] = eventdata[8];
				curloss_event.end_time[4] = eventdata[7];
				curloss_event.end_time[5] = eventdata[6];
				curloss_event.vA = eventdata[13]<<8 | eventdata[12];
				curloss_event.iA = eventdata[16]<<16 | eventdata[15]<<8 | eventdata[14];
				curloss_event.apA = eventdata[19]<<16 | eventdata[18]<<8 | eventdata[17];
				curloss_event.rapA = eventdata[22]<<16 | eventdata[21]<<8 | eventdata[20];
				curloss_event.pfA = eventdata[24]<<8 | eventdata[23];
				curloss_event.vB = eventdata[26]<<8 | eventdata[25];
				curloss_event.iB = eventdata[29]<<16 | eventdata[28]<<8 | eventdata[27];
				curloss_event.apB = eventdata[32]<<16 | eventdata[31]<<8 | eventdata[30];
				curloss_event.rapB = eventdata[35]<<16 | eventdata[34]<<8 | eventdata[33];
				curloss_event.pfB = eventdata[37]<<8 | eventdata[36];
				curloss_event.vC = eventdata[39]<<8 | eventdata[38];
				curloss_event.iC = eventdata[42]<<16 | eventdata[41]<<8 | eventdata[40];
				curloss_event.apC = eventdata[45]<<16 | eventdata[44]<<8 | eventdata[43];
				curloss_event.rapC = eventdata[48]<<16 | eventdata[47]<<8 | eventdata[46];
				curloss_event.pfC = eventdata[50]<<8 | eventdata[49];
				
				curloss_data[0] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.vA));
				curloss_data[1] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.iA));
				curloss_data[2] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.apA));
				curloss_data[3] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.rapA));
				curloss_data[4] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.pfA));
				curloss_data[5] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.vB));
				curloss_data[6] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.iB));
				curloss_data[7] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.apB));
				curloss_data[8] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.rapB));
				curloss_data[9] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.pfB));
				curloss_data[10] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.vC));
				curloss_data[11] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.iC));
				curloss_data[12] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.apC));
				curloss_data[13] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.rapC));
				curloss_data[14] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.pfC));

			}
			vb = snmp_varbind_alloc(&lt300_px_curloss_trap_oid, SNMP_ASN1_OC_STR, sizeof(curloss_event)-2*2);
	 		if (vb != NULL){
				//rt_memcpy(vb->value, eventdata, data_len);
				rt_memcpy((u8_t *)vb->value, curloss_event.start_time, sizeof(curloss_event.start_time)-2);
				rt_memcpy(((u8_t *)vb->value)+6, curloss_event.end_time, sizeof(curloss_event.end_time)-2);
				rt_memcpy(((u8_t *)vb->value)+12, (u8_t *)&curloss_data[0], sizeof(curloss_event.vA));
				rt_memcpy(((u8_t *)vb->value)+16, (u8_t *)&curloss_data[1], sizeof(curloss_event.iA));
				rt_memcpy(((u8_t *)vb->value)+20, (u8_t *)&curloss_data[2], sizeof(curloss_event.apA));
				rt_memcpy(((u8_t *)vb->value)+24, (u8_t *)&curloss_data[3], sizeof(curloss_event.rapA));
				rt_memcpy(((u8_t *)vb->value)+28, (u8_t *)&curloss_data[4], sizeof(curloss_event.pfA));
				rt_memcpy(((u8_t *)vb->value)+32, (u8_t *)&curloss_data[5], sizeof(curloss_event.vB));
				rt_memcpy(((u8_t *)vb->value)+36, (u8_t *)&curloss_data[6], sizeof(curloss_event.iB));
				rt_memcpy(((u8_t *)vb->value)+40, (u8_t *)&curloss_data[7], sizeof(curloss_event.apB));
				rt_memcpy(((u8_t *)vb->value)+44, (u8_t *)&curloss_data[8], sizeof(curloss_event.rapB));
				rt_memcpy(((u8_t *)vb->value)+48, (u8_t *)&curloss_data[9], sizeof(curloss_event.pfB));
				rt_memcpy(((u8_t *)vb->value)+52, (u8_t *)&curloss_data[10], sizeof(curloss_event.vC));
				rt_memcpy(((u8_t *)vb->value)+56, (u8_t *)&curloss_data[11], sizeof(curloss_event.iC));
				rt_memcpy(((u8_t *)vb->value)+60, (u8_t *)&curloss_data[12], sizeof(curloss_event.apC));
				rt_memcpy(((u8_t *)vb->value)+64, (u8_t *)&curloss_data[13], sizeof(curloss_event.rapC));
				rt_memcpy(((u8_t *)vb->value)+68, (u8_t *)&curloss_data[14], sizeof(curloss_event.pfC));
				//rt_memcpy((struct sinkinfo_em_curloss_event_st *)vb->value, &sinkinfo_all_em[portid].si_em_curloss_event_pa[1], SINKINFO_EM_PX_CURLOSS_EVENT_SIZE);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
		}
		break;
		case E_ALR_PA_CURRENT_OVER_EVENT_APPEAR:
		case E_ALR_PA_CURRENT_OVER_EVENT_DISAPPEAR:{
			u32_t curloss_data[32]= {0};
			struct sinkinfo_em_curloss_event_st curloss_event;
			
			vb = snmp_varbind_alloc(&lt300_meter_sn_oid, SNMP_ASN1_OC_STR, 12);
	 		if (vb != NULL){
				get_dev_sn_em_sn(SDT_ELECTRIC_METER, vb->value, DEV_SN_MODE_LEN, portid);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			vb = snmp_varbind_alloc(&lt300_meter_alarm_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			if (FRAME_E_OK != get_event_data_from_ammeter((rt_uint8_t *)amm_sn.em_sn[portid], AMM_EVENT_A_OVER_CURRENT, 1, eventdata, &data_len, RS485_PORT_USED_BY_645)){
				printf_syn("%s(), meter pa cur over event data read fail\n", __FUNCTION__);

			}else{
				curloss_event.start_time[0] = eventdata[5];
				curloss_event.start_time[1] = eventdata[4];
				curloss_event.start_time[2] = eventdata[3];
				curloss_event.start_time[3] = eventdata[2];
				curloss_event.start_time[4] = eventdata[1];
				curloss_event.start_time[5] = eventdata[0];
				curloss_event.end_time[0] = eventdata[11];
				curloss_event.end_time[1] = eventdata[10];
				curloss_event.end_time[2] = eventdata[9];
				curloss_event.end_time[3] = eventdata[8];
				curloss_event.end_time[4] = eventdata[7];
				curloss_event.end_time[5] = eventdata[6];
				curloss_event.vA = eventdata[13]<<8 | eventdata[12];
				curloss_event.iA = eventdata[16]<<16 | eventdata[15]<<8 | eventdata[14];
				curloss_event.apA = eventdata[19]<<16 | eventdata[18]<<8 | eventdata[17];
				curloss_event.rapA = eventdata[22]<<16 | eventdata[21]<<8 | eventdata[20];
				curloss_event.pfA = eventdata[24]<<8 | eventdata[23];
				curloss_event.vB = eventdata[26]<<8 | eventdata[25];
				curloss_event.iB = eventdata[29]<<16 | eventdata[28]<<8 | eventdata[27];
				curloss_event.apB = eventdata[32]<<16 | eventdata[31]<<8 | eventdata[30];
				curloss_event.rapB = eventdata[35]<<16 | eventdata[34]<<8 | eventdata[33];
				curloss_event.pfB = eventdata[37]<<8 | eventdata[36];
				curloss_event.vC = eventdata[39]<<8 | eventdata[38];
				curloss_event.iC = eventdata[42]<<16 | eventdata[41]<<8 | eventdata[40];
				curloss_event.apC = eventdata[45]<<16 | eventdata[44]<<8 | eventdata[43];
				curloss_event.rapC = eventdata[48]<<16 | eventdata[47]<<8 | eventdata[46];
				curloss_event.pfC = eventdata[50]<<8 | eventdata[49];
				
				curloss_data[0] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.vA));
				curloss_data[1] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.iA));
				curloss_data[2] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.apA));
				curloss_data[3] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.rapA));
				curloss_data[4] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.pfA));
				curloss_data[5] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.vB));
				curloss_data[6] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.iB));
				curloss_data[7] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.apB));
				curloss_data[8] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.rapB));
				curloss_data[9] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.pfB));
				curloss_data[10] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.vC));
				curloss_data[11] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.iC));
				curloss_data[12] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.apC));
				curloss_data[13] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.rapC));
				curloss_data[14] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.pfC));

			}
			vb = snmp_varbind_alloc(&lt300_px_curover_trap_oid, SNMP_ASN1_OC_STR, sizeof(curloss_event)-2*2);
	 		if (vb != NULL){
				//rt_memcpy(vb->value, eventdata, data_len);
				rt_memcpy((u8_t *)vb->value, curloss_event.start_time, sizeof(curloss_event.start_time)-2);
				rt_memcpy(((u8_t *)vb->value)+6, curloss_event.end_time, sizeof(curloss_event.end_time)-2);
				rt_memcpy(((u8_t *)vb->value)+12, (u8_t *)&curloss_data[0], sizeof(curloss_event.vA));
				rt_memcpy(((u8_t *)vb->value)+16, (u8_t *)&curloss_data[1], sizeof(curloss_event.iA));
				rt_memcpy(((u8_t *)vb->value)+20, (u8_t *)&curloss_data[2], sizeof(curloss_event.apA));
				rt_memcpy(((u8_t *)vb->value)+24, (u8_t *)&curloss_data[3], sizeof(curloss_event.rapA));
				rt_memcpy(((u8_t *)vb->value)+28, (u8_t *)&curloss_data[4], sizeof(curloss_event.pfA));
				rt_memcpy(((u8_t *)vb->value)+32, (u8_t *)&curloss_data[5], sizeof(curloss_event.vB));
				rt_memcpy(((u8_t *)vb->value)+36, (u8_t *)&curloss_data[6], sizeof(curloss_event.iB));
				rt_memcpy(((u8_t *)vb->value)+40, (u8_t *)&curloss_data[7], sizeof(curloss_event.apB));
				rt_memcpy(((u8_t *)vb->value)+44, (u8_t *)&curloss_data[8], sizeof(curloss_event.rapB));
				rt_memcpy(((u8_t *)vb->value)+48, (u8_t *)&curloss_data[9], sizeof(curloss_event.pfB));
				rt_memcpy(((u8_t *)vb->value)+52, (u8_t *)&curloss_data[10], sizeof(curloss_event.vC));
				rt_memcpy(((u8_t *)vb->value)+56, (u8_t *)&curloss_data[11], sizeof(curloss_event.iC));
				rt_memcpy(((u8_t *)vb->value)+60, (u8_t *)&curloss_data[12], sizeof(curloss_event.apC));
				rt_memcpy(((u8_t *)vb->value)+64, (u8_t *)&curloss_data[13], sizeof(curloss_event.rapC));
				rt_memcpy(((u8_t *)vb->value)+68, (u8_t *)&curloss_data[14], sizeof(curloss_event.pfC));
				//rt_memcpy((struct sinkinfo_em_curloss_event_st *)vb->value, &sinkinfo_all_em[portid].si_em_curover_event_pa[1], SINKINFO_EM_PX_CURLOSS_EVENT_SIZE);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
		}
		break;
		case E_ALR_PA_CURRENT_BREAK_EVENT_APPEAR:
		case E_ALR_PA_CURRENT_BREAK_EVENT_DISAPPEAR:{
			u32_t curloss_data[32]= {0};
			struct sinkinfo_em_curloss_event_st curloss_event;
			
			vb = snmp_varbind_alloc(&lt300_meter_sn_oid, SNMP_ASN1_OC_STR, 12);
	 		if (vb != NULL){
				get_dev_sn_em_sn(SDT_ELECTRIC_METER, vb->value, DEV_SN_MODE_LEN, portid);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			vb = snmp_varbind_alloc(&lt300_meter_alarm_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			if (FRAME_E_OK != get_event_data_from_ammeter((rt_uint8_t *)amm_sn.em_sn[portid], AMM_EVENT_A_BROKEN_CURRENT, 1, eventdata, &data_len, RS485_PORT_USED_BY_645)){
				printf_syn("%s(), meter pa cur break event data read fail\n", __FUNCTION__);

			}else{
				curloss_event.start_time[0] = eventdata[5];
				curloss_event.start_time[1] = eventdata[4];
				curloss_event.start_time[2] = eventdata[3];
				curloss_event.start_time[3] = eventdata[2];
				curloss_event.start_time[4] = eventdata[1];
				curloss_event.start_time[5] = eventdata[0];
				curloss_event.end_time[0] = eventdata[11];
				curloss_event.end_time[1] = eventdata[10];
				curloss_event.end_time[2] = eventdata[9];
				curloss_event.end_time[3] = eventdata[8];
				curloss_event.end_time[4] = eventdata[7];
				curloss_event.end_time[5] = eventdata[6];
				curloss_event.vA = eventdata[13]<<8 | eventdata[12];
				curloss_event.iA = eventdata[16]<<16 | eventdata[15]<<8 | eventdata[14];
				curloss_event.apA = eventdata[19]<<16 | eventdata[18]<<8 | eventdata[17];
				curloss_event.rapA = eventdata[22]<<16 | eventdata[21]<<8 | eventdata[20];
				curloss_event.pfA = eventdata[24]<<8 | eventdata[23];
				curloss_event.vB = eventdata[26]<<8 | eventdata[25];
				curloss_event.iB = eventdata[29]<<16 | eventdata[28]<<8 | eventdata[27];
				curloss_event.apB = eventdata[32]<<16 | eventdata[31]<<8 | eventdata[30];
				curloss_event.rapB = eventdata[35]<<16 | eventdata[34]<<8 | eventdata[33];
				curloss_event.pfB = eventdata[37]<<8 | eventdata[36];
				curloss_event.vC = eventdata[39]<<8 | eventdata[38];
				curloss_event.iC = eventdata[42]<<16 | eventdata[41]<<8 | eventdata[40];
				curloss_event.apC = eventdata[45]<<16 | eventdata[44]<<8 | eventdata[43];
				curloss_event.rapC = eventdata[48]<<16 | eventdata[47]<<8 | eventdata[46];
				curloss_event.pfC = eventdata[50]<<8 | eventdata[49];
				
				curloss_data[0] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.vA));
				curloss_data[1] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.iA));
				curloss_data[2] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.apA));
				curloss_data[3] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.rapA));
				curloss_data[4] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.pfA));
				curloss_data[5] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.vB));
				curloss_data[6] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.iB));
				curloss_data[7] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.apB));
				curloss_data[8] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.rapB));
				curloss_data[9] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.pfB));
				curloss_data[10] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.vC));
				curloss_data[11] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.iC));
				curloss_data[12] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.apC));
				curloss_data[13] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.rapC));
				curloss_data[14] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.pfC));

			}
			vb = snmp_varbind_alloc(&lt300_px_curbreak_trap_oid, SNMP_ASN1_OC_STR, sizeof(curloss_event)-2*2);
	 		if (vb != NULL){
				//rt_memcpy(vb->value, eventdata, data_len);
				rt_memcpy((u8_t *)vb->value, curloss_event.start_time, sizeof(curloss_event.start_time)-2);
				rt_memcpy(((u8_t *)vb->value)+6, curloss_event.end_time, sizeof(curloss_event.end_time)-2);
				rt_memcpy(((u8_t *)vb->value)+12, (u8_t *)&curloss_data[0], sizeof(curloss_event.vA));
				rt_memcpy(((u8_t *)vb->value)+16, (u8_t *)&curloss_data[1], sizeof(curloss_event.iA));
				rt_memcpy(((u8_t *)vb->value)+20, (u8_t *)&curloss_data[2], sizeof(curloss_event.apA));
				rt_memcpy(((u8_t *)vb->value)+24, (u8_t *)&curloss_data[3], sizeof(curloss_event.rapA));
				rt_memcpy(((u8_t *)vb->value)+28, (u8_t *)&curloss_data[4], sizeof(curloss_event.pfA));
				rt_memcpy(((u8_t *)vb->value)+32, (u8_t *)&curloss_data[5], sizeof(curloss_event.vB));
				rt_memcpy(((u8_t *)vb->value)+36, (u8_t *)&curloss_data[6], sizeof(curloss_event.iB));
				rt_memcpy(((u8_t *)vb->value)+40, (u8_t *)&curloss_data[7], sizeof(curloss_event.apB));
				rt_memcpy(((u8_t *)vb->value)+44, (u8_t *)&curloss_data[8], sizeof(curloss_event.rapB));
				rt_memcpy(((u8_t *)vb->value)+48, (u8_t *)&curloss_data[9], sizeof(curloss_event.pfB));
				rt_memcpy(((u8_t *)vb->value)+52, (u8_t *)&curloss_data[10], sizeof(curloss_event.vC));
				rt_memcpy(((u8_t *)vb->value)+56, (u8_t *)&curloss_data[11], sizeof(curloss_event.iC));
				rt_memcpy(((u8_t *)vb->value)+60, (u8_t *)&curloss_data[12], sizeof(curloss_event.apC));
				rt_memcpy(((u8_t *)vb->value)+64, (u8_t *)&curloss_data[13], sizeof(curloss_event.rapC));
				rt_memcpy(((u8_t *)vb->value)+68, (u8_t *)&curloss_data[14], sizeof(curloss_event.pfC));
				//rt_memcpy((struct sinkinfo_em_curloss_event_st *)vb->value, &sinkinfo_all_em[portid].si_em_curbreak_event_pa[1], SINKINFO_EM_PX_CURLOSS_EVENT_SIZE);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
		}
		break;
		case E_ALR_PB_VOLTAGE_LOSS_EVENT_APPEAR:
		case E_ALR_PB_VOLTAGE_LOSS_EVENT_DISAPPEAR:{
			u32_t volloss_data[32]= {0};
			struct sinkinfo_em_volloss_event_st volloss_event;
			
			vb = snmp_varbind_alloc(&lt300_meter_sn_oid, SNMP_ASN1_OC_STR, 12);
	 		if (vb != NULL){
				get_dev_sn_em_sn(SDT_ELECTRIC_METER, vb->value, DEV_SN_MODE_LEN, portid);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			vb = snmp_varbind_alloc(&lt300_meter_alarm_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			if (FRAME_E_OK != get_event_data_from_ammeter((rt_uint8_t *)amm_sn.em_sn[portid], AMM_EVENT_B_LOSE_VOLTAGE, 1, eventdata, &data_len, RS485_PORT_USED_BY_645)){
				printf_syn("%s(), meter pb vol loss event data read fail\n", __FUNCTION__);

			}else{
				volloss_event.start_time[0] = eventdata[5];
				volloss_event.start_time[1] = eventdata[4];
				volloss_event.start_time[2] = eventdata[3];
				volloss_event.start_time[3] = eventdata[2];
				volloss_event.start_time[4] = eventdata[1];
				volloss_event.start_time[5] = eventdata[0];
				volloss_event.end_time[0] = eventdata[11];
				volloss_event.end_time[1] = eventdata[10];
				volloss_event.end_time[2] = eventdata[9];
				volloss_event.end_time[3] = eventdata[8];
				volloss_event.end_time[4] = eventdata[7];
				volloss_event.end_time[5] = eventdata[6];
				volloss_event.vA = eventdata[13]<<8 | eventdata[12];
				volloss_event.iA = eventdata[16]<<16 | eventdata[15]<<8 | eventdata[14];
				volloss_event.apA = eventdata[19]<<16 | eventdata[18]<<8 | eventdata[17];
				volloss_event.rapA = eventdata[22]<<16 | eventdata[21]<<8 | eventdata[20];
				volloss_event.pfA = eventdata[24]<<8 | eventdata[23];
				volloss_event.vB = eventdata[26]<<8 | eventdata[25];
				volloss_event.iB = eventdata[29]<<16 | eventdata[28]<<8 | eventdata[27];
				volloss_event.apB = eventdata[32]<<16 | eventdata[31]<<8 | eventdata[30];
				volloss_event.rapB = eventdata[35]<<16 | eventdata[34]<<8 | eventdata[33];
				volloss_event.pfB = eventdata[37]<<8 | eventdata[36];
				volloss_event.vC = eventdata[39]<<8 | eventdata[38];
				volloss_event.iC = eventdata[42]<<16 | eventdata[41]<<8 | eventdata[40];
				volloss_event.apC = eventdata[45]<<16 | eventdata[44]<<8 | eventdata[43];
				volloss_event.rapC = eventdata[48]<<16 | eventdata[47]<<8 | eventdata[46];
				volloss_event.pfC = eventdata[50]<<8 | eventdata[49];
				volloss_event.ahnA = eventdata[54]<<24 | eventdata[53]<<16 | eventdata[52]<<8 | eventdata[51];
				volloss_event.ahnB = eventdata[58]<<24 | eventdata[57]<<16 | eventdata[56]<<8 | eventdata[55];
				volloss_event.ahnC = eventdata[62]<<24 | eventdata[61]<<16 | eventdata[60]<<8 | eventdata[59];
				
				volloss_data[0] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.vA));
				volloss_data[1] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.iA));
				volloss_data[2] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.apA));
				volloss_data[3] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.rapA));
				volloss_data[4] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.pfA));
				volloss_data[5] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.vB));
				volloss_data[6] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.iB));
				volloss_data[7] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.apB));
				volloss_data[8] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.rapB));
				volloss_data[9] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.pfB));
				volloss_data[10] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.vC));
				volloss_data[11] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.iC));
				volloss_data[12] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.apC));
				volloss_data[13] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.rapC));
				volloss_data[14] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.pfC));
				volloss_data[15] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.ahnA));
				volloss_data[16] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.ahnB));
				volloss_data[17] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.ahnC));

			}
			vb = snmp_varbind_alloc(&lt300_px_volloss_trap_oid, SNMP_ASN1_OC_STR, sizeof(volloss_event)-2*2);
	 		if (vb != NULL){
				//rt_memcpy(vb->value, eventdata, data_len);
				rt_memcpy((u8_t *)vb->value, volloss_event.start_time, sizeof(volloss_event.start_time)-2);
				rt_memcpy(((u8_t *)vb->value)+6, volloss_event.end_time, sizeof(volloss_event.end_time)-2);
				rt_memcpy(((u8_t *)vb->value)+12, (u8_t *)&volloss_data[0], sizeof(volloss_event.vA));
				rt_memcpy(((u8_t *)vb->value)+16, (u8_t *)&volloss_data[1], sizeof(volloss_event.iA));
				rt_memcpy(((u8_t *)vb->value)+20, (u8_t *)&volloss_data[2], sizeof(volloss_event.apA));
				rt_memcpy(((u8_t *)vb->value)+24, (u8_t *)&volloss_data[3], sizeof(volloss_event.rapA));
				rt_memcpy(((u8_t *)vb->value)+28, (u8_t *)&volloss_data[4], sizeof(volloss_event.pfA));
				rt_memcpy(((u8_t *)vb->value)+32, (u8_t *)&volloss_data[5], sizeof(volloss_event.vB));
				rt_memcpy(((u8_t *)vb->value)+36, (u8_t *)&volloss_data[6], sizeof(volloss_event.iB));
				rt_memcpy(((u8_t *)vb->value)+40, (u8_t *)&volloss_data[7], sizeof(volloss_event.apB));
				rt_memcpy(((u8_t *)vb->value)+44, (u8_t *)&volloss_data[8], sizeof(volloss_event.rapB));
				rt_memcpy(((u8_t *)vb->value)+48, (u8_t *)&volloss_data[9], sizeof(volloss_event.pfB));
				rt_memcpy(((u8_t *)vb->value)+52, (u8_t *)&volloss_data[10], sizeof(volloss_event.vC));
				rt_memcpy(((u8_t *)vb->value)+56, (u8_t *)&volloss_data[11], sizeof(volloss_event.iC));
				rt_memcpy(((u8_t *)vb->value)+60, (u8_t *)&volloss_data[12], sizeof(volloss_event.apC));
				rt_memcpy(((u8_t *)vb->value)+64, (u8_t *)&volloss_data[13], sizeof(volloss_event.rapC));
				rt_memcpy(((u8_t *)vb->value)+68, (u8_t *)&volloss_data[14], sizeof(volloss_event.pfC));
				rt_memcpy(((u8_t *)vb->value)+72, (u8_t *)&volloss_data[15], sizeof(volloss_event.ahnA));
				rt_memcpy(((u8_t *)vb->value)+76, (u8_t *)&volloss_data[16], sizeof(volloss_event.ahnB));
				rt_memcpy(((u8_t *)vb->value)+80, (u8_t *)&volloss_data[17], sizeof(volloss_event.ahnC));	
				//rt_memcpy((struct sinkinfo_em_volloss_event_st *)vb->value, &sinkinfo_all_em[portid].si_em_volloss_event_pb[1], SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
		}
		break;
		case E_ALR_PB_VOLTAGE_OVER_EVENT_APPEAR:
		case E_ALR_PB_VOLTAGE_OVER_EVENT_DISAPPEAR:{
			u32_t volloss_data[32]= {0};
			struct sinkinfo_em_volloss_event_st volloss_event;
			
			vb = snmp_varbind_alloc(&lt300_meter_sn_oid, SNMP_ASN1_OC_STR, 12);
	 		if (vb != NULL){
				get_dev_sn_em_sn(SDT_ELECTRIC_METER, vb->value, DEV_SN_MODE_LEN, portid);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			vb = snmp_varbind_alloc(&lt300_meter_alarm_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			if (FRAME_E_OK != get_event_data_from_ammeter((rt_uint8_t *)amm_sn.em_sn[portid], AMM_EVENT_B_OVER_VOLTAGE, 1, eventdata, &data_len, RS485_PORT_USED_BY_645)){
				printf_syn("%s(), meter pb vol over event data read fail\n", __FUNCTION__);

			}else{
				volloss_event.start_time[0] = eventdata[5];
				volloss_event.start_time[1] = eventdata[4];
				volloss_event.start_time[2] = eventdata[3];
				volloss_event.start_time[3] = eventdata[2];
				volloss_event.start_time[4] = eventdata[1];
				volloss_event.start_time[5] = eventdata[0];
				volloss_event.end_time[0] = eventdata[11];
				volloss_event.end_time[1] = eventdata[10];
				volloss_event.end_time[2] = eventdata[9];
				volloss_event.end_time[3] = eventdata[8];
				volloss_event.end_time[4] = eventdata[7];
				volloss_event.end_time[5] = eventdata[6];
				volloss_event.vA = eventdata[13]<<8 | eventdata[12];
				volloss_event.iA = eventdata[16]<<16 | eventdata[15]<<8 | eventdata[14];
				volloss_event.apA = eventdata[19]<<16 | eventdata[18]<<8 | eventdata[17];
				volloss_event.rapA = eventdata[22]<<16 | eventdata[21]<<8 | eventdata[20];
				volloss_event.pfA = eventdata[24]<<8 | eventdata[23];
				volloss_event.vB = eventdata[26]<<8 | eventdata[25];
				volloss_event.iB = eventdata[29]<<16 | eventdata[28]<<8 | eventdata[27];
				volloss_event.apB = eventdata[32]<<16 | eventdata[31]<<8 | eventdata[30];
				volloss_event.rapB = eventdata[35]<<16 | eventdata[34]<<8 | eventdata[33];
				volloss_event.pfB = eventdata[37]<<8 | eventdata[36];
				volloss_event.vC = eventdata[39]<<8 | eventdata[38];
				volloss_event.iC = eventdata[42]<<16 | eventdata[41]<<8 | eventdata[40];
				volloss_event.apC = eventdata[45]<<16 | eventdata[44]<<8 | eventdata[43];
				volloss_event.rapC = eventdata[48]<<16 | eventdata[47]<<8 | eventdata[46];
				volloss_event.pfC = eventdata[50]<<8 | eventdata[49];
				volloss_event.ahnA = eventdata[54]<<24 | eventdata[53]<<16 | eventdata[52]<<8 | eventdata[51];
				volloss_event.ahnB = eventdata[58]<<24 | eventdata[57]<<16 | eventdata[56]<<8 | eventdata[55];
				volloss_event.ahnC = eventdata[62]<<24 | eventdata[61]<<16 | eventdata[60]<<8 | eventdata[59];
				
				volloss_data[0] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.vA));
				volloss_data[1] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.iA));
				volloss_data[2] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.apA));
				volloss_data[3] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.rapA));
				volloss_data[4] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.pfA));
				volloss_data[5] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.vB));
				volloss_data[6] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.iB));
				volloss_data[7] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.apB));
				volloss_data[8] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.rapB));
				volloss_data[9] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.pfB));
				volloss_data[10] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.vC));
				volloss_data[11] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.iC));
				volloss_data[12] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.apC));
				volloss_data[13] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.rapC));
				volloss_data[14] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.pfC));
				volloss_data[15] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.ahnA));
				volloss_data[16] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.ahnB));
				volloss_data[17] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.ahnC));

			}
			vb = snmp_varbind_alloc(&lt300_px_volover_trap_oid, SNMP_ASN1_OC_STR, sizeof(volloss_event)-2*2);
	 		if (vb != NULL){
				//rt_memcpy(vb->value, eventdata, data_len);
				rt_memcpy((u8_t *)vb->value, volloss_event.start_time, sizeof(volloss_event.start_time)-2);
				rt_memcpy(((u8_t *)vb->value)+6, volloss_event.end_time, sizeof(volloss_event.end_time)-2);
				rt_memcpy(((u8_t *)vb->value)+12, (u8_t *)&volloss_data[0], sizeof(volloss_event.vA));
				rt_memcpy(((u8_t *)vb->value)+16, (u8_t *)&volloss_data[1], sizeof(volloss_event.iA));
				rt_memcpy(((u8_t *)vb->value)+20, (u8_t *)&volloss_data[2], sizeof(volloss_event.apA));
				rt_memcpy(((u8_t *)vb->value)+24, (u8_t *)&volloss_data[3], sizeof(volloss_event.rapA));
				rt_memcpy(((u8_t *)vb->value)+28, (u8_t *)&volloss_data[4], sizeof(volloss_event.pfA));
				rt_memcpy(((u8_t *)vb->value)+32, (u8_t *)&volloss_data[5], sizeof(volloss_event.vB));
				rt_memcpy(((u8_t *)vb->value)+36, (u8_t *)&volloss_data[6], sizeof(volloss_event.iB));
				rt_memcpy(((u8_t *)vb->value)+40, (u8_t *)&volloss_data[7], sizeof(volloss_event.apB));
				rt_memcpy(((u8_t *)vb->value)+44, (u8_t *)&volloss_data[8], sizeof(volloss_event.rapB));
				rt_memcpy(((u8_t *)vb->value)+48, (u8_t *)&volloss_data[9], sizeof(volloss_event.pfB));
				rt_memcpy(((u8_t *)vb->value)+52, (u8_t *)&volloss_data[10], sizeof(volloss_event.vC));
				rt_memcpy(((u8_t *)vb->value)+56, (u8_t *)&volloss_data[11], sizeof(volloss_event.iC));
				rt_memcpy(((u8_t *)vb->value)+60, (u8_t *)&volloss_data[12], sizeof(volloss_event.apC));
				rt_memcpy(((u8_t *)vb->value)+64, (u8_t *)&volloss_data[13], sizeof(volloss_event.rapC));
				rt_memcpy(((u8_t *)vb->value)+68, (u8_t *)&volloss_data[14], sizeof(volloss_event.pfC));
				rt_memcpy(((u8_t *)vb->value)+72, (u8_t *)&volloss_data[15], sizeof(volloss_event.ahnA));
				rt_memcpy(((u8_t *)vb->value)+76, (u8_t *)&volloss_data[16], sizeof(volloss_event.ahnB));
				rt_memcpy(((u8_t *)vb->value)+80, (u8_t *)&volloss_data[17], sizeof(volloss_event.ahnC));	
				//rt_memcpy((struct sinkinfo_em_volloss_event_st *)vb->value, &sinkinfo_all_em[portid].si_em_volover_event_pb[1], SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE);		
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
		}
		break;
		case E_ALR_PB_VOLTAGE_UNDER_EVENT_APPEAR:
		case E_ALR_PB_VOLTAGE_UNDER_EVENT_DISAPPEAR:{
			u32_t volloss_data[32]= {0};
			struct sinkinfo_em_volloss_event_st volloss_event;
			
			vb = snmp_varbind_alloc(&lt300_meter_sn_oid, SNMP_ASN1_OC_STR, 12);
	 		if (vb != NULL){
				get_dev_sn_em_sn(SDT_ELECTRIC_METER, vb->value, DEV_SN_MODE_LEN, portid);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			vb = snmp_varbind_alloc(&lt300_meter_alarm_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			if (FRAME_E_OK != get_event_data_from_ammeter((rt_uint8_t *)amm_sn.em_sn[portid], AMM_EVENT_B_OWE_VOLTAGE, 1, eventdata, &data_len, RS485_PORT_USED_BY_645)){
				printf_syn("%s(), meter pb vol under event data read fail\n", __FUNCTION__);

			}else{
				volloss_event.start_time[0] = eventdata[5];
				volloss_event.start_time[1] = eventdata[4];
				volloss_event.start_time[2] = eventdata[3];
				volloss_event.start_time[3] = eventdata[2];
				volloss_event.start_time[4] = eventdata[1];
				volloss_event.start_time[5] = eventdata[0];
				volloss_event.end_time[0] = eventdata[11];
				volloss_event.end_time[1] = eventdata[10];
				volloss_event.end_time[2] = eventdata[9];
				volloss_event.end_time[3] = eventdata[8];
				volloss_event.end_time[4] = eventdata[7];
				volloss_event.end_time[5] = eventdata[6];
				volloss_event.vA = eventdata[13]<<8 | eventdata[12];
				volloss_event.iA = eventdata[16]<<16 | eventdata[15]<<8 | eventdata[14];
				volloss_event.apA = eventdata[19]<<16 | eventdata[18]<<8 | eventdata[17];
				volloss_event.rapA = eventdata[22]<<16 | eventdata[21]<<8 | eventdata[20];
				volloss_event.pfA = eventdata[24]<<8 | eventdata[23];
				volloss_event.vB = eventdata[26]<<8 | eventdata[25];
				volloss_event.iB = eventdata[29]<<16 | eventdata[28]<<8 | eventdata[27];
				volloss_event.apB = eventdata[32]<<16 | eventdata[31]<<8 | eventdata[30];
				volloss_event.rapB = eventdata[35]<<16 | eventdata[34]<<8 | eventdata[33];
				volloss_event.pfB = eventdata[37]<<8 | eventdata[36];
				volloss_event.vC = eventdata[39]<<8 | eventdata[38];
				volloss_event.iC = eventdata[42]<<16 | eventdata[41]<<8 | eventdata[40];
				volloss_event.apC = eventdata[45]<<16 | eventdata[44]<<8 | eventdata[43];
				volloss_event.rapC = eventdata[48]<<16 | eventdata[47]<<8 | eventdata[46];
				volloss_event.pfC = eventdata[50]<<8 | eventdata[49];
				volloss_event.ahnA = eventdata[54]<<24 | eventdata[53]<<16 | eventdata[52]<<8 | eventdata[51];
				volloss_event.ahnB = eventdata[58]<<24 | eventdata[57]<<16 | eventdata[56]<<8 | eventdata[55];
				volloss_event.ahnC = eventdata[62]<<24 | eventdata[61]<<16 | eventdata[60]<<8 | eventdata[59];
				
				volloss_data[0] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.vA));
				volloss_data[1] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.iA));
				volloss_data[2] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.apA));
				volloss_data[3] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.rapA));
				volloss_data[4] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.pfA));
				volloss_data[5] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.vB));
				volloss_data[6] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.iB));
				volloss_data[7] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.apB));
				volloss_data[8] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.rapB));
				volloss_data[9] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.pfB));
				volloss_data[10] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.vC));
				volloss_data[11] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.iC));
				volloss_data[12] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.apC));
				volloss_data[13] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.rapC));
				volloss_data[14] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.pfC));
				volloss_data[15] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.ahnA));
				volloss_data[16] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.ahnB));
				volloss_data[17] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.ahnC));

			}
			vb = snmp_varbind_alloc(&lt300_px_volunder_trap_oid, SNMP_ASN1_OC_STR, sizeof(volloss_event)-2*2);
	 		if (vb != NULL){
				//rt_memcpy(vb->value, eventdata, data_len);
				rt_memcpy((u8_t *)vb->value, volloss_event.start_time, sizeof(volloss_event.start_time)-2);
				rt_memcpy(((u8_t *)vb->value)+6, volloss_event.end_time, sizeof(volloss_event.end_time)-2);
				rt_memcpy(((u8_t *)vb->value)+12, (u8_t *)&volloss_data[0], sizeof(volloss_event.vA));
				rt_memcpy(((u8_t *)vb->value)+16, (u8_t *)&volloss_data[1], sizeof(volloss_event.iA));
				rt_memcpy(((u8_t *)vb->value)+20, (u8_t *)&volloss_data[2], sizeof(volloss_event.apA));
				rt_memcpy(((u8_t *)vb->value)+24, (u8_t *)&volloss_data[3], sizeof(volloss_event.rapA));
				rt_memcpy(((u8_t *)vb->value)+28, (u8_t *)&volloss_data[4], sizeof(volloss_event.pfA));
				rt_memcpy(((u8_t *)vb->value)+32, (u8_t *)&volloss_data[5], sizeof(volloss_event.vB));
				rt_memcpy(((u8_t *)vb->value)+36, (u8_t *)&volloss_data[6], sizeof(volloss_event.iB));
				rt_memcpy(((u8_t *)vb->value)+40, (u8_t *)&volloss_data[7], sizeof(volloss_event.apB));
				rt_memcpy(((u8_t *)vb->value)+44, (u8_t *)&volloss_data[8], sizeof(volloss_event.rapB));
				rt_memcpy(((u8_t *)vb->value)+48, (u8_t *)&volloss_data[9], sizeof(volloss_event.pfB));
				rt_memcpy(((u8_t *)vb->value)+52, (u8_t *)&volloss_data[10], sizeof(volloss_event.vC));
				rt_memcpy(((u8_t *)vb->value)+56, (u8_t *)&volloss_data[11], sizeof(volloss_event.iC));
				rt_memcpy(((u8_t *)vb->value)+60, (u8_t *)&volloss_data[12], sizeof(volloss_event.apC));
				rt_memcpy(((u8_t *)vb->value)+64, (u8_t *)&volloss_data[13], sizeof(volloss_event.rapC));
				rt_memcpy(((u8_t *)vb->value)+68, (u8_t *)&volloss_data[14], sizeof(volloss_event.pfC));
				rt_memcpy(((u8_t *)vb->value)+72, (u8_t *)&volloss_data[15], sizeof(volloss_event.ahnA));
				rt_memcpy(((u8_t *)vb->value)+76, (u8_t *)&volloss_data[16], sizeof(volloss_event.ahnB));
				rt_memcpy(((u8_t *)vb->value)+80, (u8_t *)&volloss_data[17], sizeof(volloss_event.ahnC));	
				//rt_memcpy((struct sinkinfo_em_volloss_event_st *)vb->value, &sinkinfo_all_em[portid].si_em_volunder_event_pb[1], SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
		}
		break;
		case E_ALR_PB_PHASE_BREAK_EVENT_APPEAR:
		case E_ALR_PB_PHASE_BREAK_EVENT_DISAPPEAR:{
			u32_t volloss_data[32]= {0};
			struct sinkinfo_em_volloss_event_st volloss_event;
			
			vb = snmp_varbind_alloc(&lt300_meter_sn_oid, SNMP_ASN1_OC_STR, 12);
	 		if (vb != NULL){
				get_dev_sn_em_sn(SDT_ELECTRIC_METER, vb->value, DEV_SN_MODE_LEN, portid);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			vb = snmp_varbind_alloc(&lt300_meter_alarm_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			if (FRAME_E_OK != get_event_data_from_ammeter((rt_uint8_t *)amm_sn.em_sn[portid], AMM_EVENT_B_BROKEN_PHASE, 1, eventdata, &data_len, RS485_PORT_USED_BY_645)){
				printf_syn("%s(), meter pb phase break event data read fail\n", __FUNCTION__);

			}else{
				volloss_event.start_time[0] = eventdata[5];
				volloss_event.start_time[1] = eventdata[4];
				volloss_event.start_time[2] = eventdata[3];
				volloss_event.start_time[3] = eventdata[2];
				volloss_event.start_time[4] = eventdata[1];
				volloss_event.start_time[5] = eventdata[0];
				volloss_event.end_time[0] = eventdata[11];
				volloss_event.end_time[1] = eventdata[10];
				volloss_event.end_time[2] = eventdata[9];
				volloss_event.end_time[3] = eventdata[8];
				volloss_event.end_time[4] = eventdata[7];
				volloss_event.end_time[5] = eventdata[6];
				volloss_event.vA = eventdata[13]<<8 | eventdata[12];
				volloss_event.iA = eventdata[16]<<16 | eventdata[15]<<8 | eventdata[14];
				volloss_event.apA = eventdata[19]<<16 | eventdata[18]<<8 | eventdata[17];
				volloss_event.rapA = eventdata[22]<<16 | eventdata[21]<<8 | eventdata[20];
				volloss_event.pfA = eventdata[24]<<8 | eventdata[23];
				volloss_event.vB = eventdata[26]<<8 | eventdata[25];
				volloss_event.iB = eventdata[29]<<16 | eventdata[28]<<8 | eventdata[27];
				volloss_event.apB = eventdata[32]<<16 | eventdata[31]<<8 | eventdata[30];
				volloss_event.rapB = eventdata[35]<<16 | eventdata[34]<<8 | eventdata[33];
				volloss_event.pfB = eventdata[37]<<8 | eventdata[36];
				volloss_event.vC = eventdata[39]<<8 | eventdata[38];
				volloss_event.iC = eventdata[42]<<16 | eventdata[41]<<8 | eventdata[40];
				volloss_event.apC = eventdata[45]<<16 | eventdata[44]<<8 | eventdata[43];
				volloss_event.rapC = eventdata[48]<<16 | eventdata[47]<<8 | eventdata[46];
				volloss_event.pfC = eventdata[50]<<8 | eventdata[49];
				volloss_event.ahnA = eventdata[54]<<24 | eventdata[53]<<16 | eventdata[52]<<8 | eventdata[51];
				volloss_event.ahnB = eventdata[58]<<24 | eventdata[57]<<16 | eventdata[56]<<8 | eventdata[55];
				volloss_event.ahnC = eventdata[62]<<24 | eventdata[61]<<16 | eventdata[60]<<8 | eventdata[59];
				
				volloss_data[0] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.vA));
				volloss_data[1] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.iA));
				volloss_data[2] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.apA));
				volloss_data[3] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.rapA));
				volloss_data[4] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.pfA));
				volloss_data[5] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.vB));
				volloss_data[6] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.iB));
				volloss_data[7] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.apB));
				volloss_data[8] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.rapB));
				volloss_data[9] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.pfB));
				volloss_data[10] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.vC));
				volloss_data[11] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.iC));
				volloss_data[12] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.apC));
				volloss_data[13] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.rapC));
				volloss_data[14] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.pfC));
				volloss_data[15] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.ahnA));
				volloss_data[16] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.ahnB));
				volloss_data[17] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.ahnC));

			}
			vb = snmp_varbind_alloc(&lt300_px_phasebreak_trap_oid, SNMP_ASN1_OC_STR, sizeof(volloss_event)-2*2);
	 		if (vb != NULL){
				//rt_memcpy(vb->value, eventdata, data_len);
				rt_memcpy((u8_t *)vb->value, volloss_event.start_time, sizeof(volloss_event.start_time)-2);
				rt_memcpy(((u8_t *)vb->value)+6, volloss_event.end_time, sizeof(volloss_event.end_time)-2);
				rt_memcpy(((u8_t *)vb->value)+12, (u8_t *)&volloss_data[0], sizeof(volloss_event.vA));
				rt_memcpy(((u8_t *)vb->value)+16, (u8_t *)&volloss_data[1], sizeof(volloss_event.iA));
				rt_memcpy(((u8_t *)vb->value)+20, (u8_t *)&volloss_data[2], sizeof(volloss_event.apA));
				rt_memcpy(((u8_t *)vb->value)+24, (u8_t *)&volloss_data[3], sizeof(volloss_event.rapA));
				rt_memcpy(((u8_t *)vb->value)+28, (u8_t *)&volloss_data[4], sizeof(volloss_event.pfA));
				rt_memcpy(((u8_t *)vb->value)+32, (u8_t *)&volloss_data[5], sizeof(volloss_event.vB));
				rt_memcpy(((u8_t *)vb->value)+36, (u8_t *)&volloss_data[6], sizeof(volloss_event.iB));
				rt_memcpy(((u8_t *)vb->value)+40, (u8_t *)&volloss_data[7], sizeof(volloss_event.apB));
				rt_memcpy(((u8_t *)vb->value)+44, (u8_t *)&volloss_data[8], sizeof(volloss_event.rapB));
				rt_memcpy(((u8_t *)vb->value)+48, (u8_t *)&volloss_data[9], sizeof(volloss_event.pfB));
				rt_memcpy(((u8_t *)vb->value)+52, (u8_t *)&volloss_data[10], sizeof(volloss_event.vC));
				rt_memcpy(((u8_t *)vb->value)+56, (u8_t *)&volloss_data[11], sizeof(volloss_event.iC));
				rt_memcpy(((u8_t *)vb->value)+60, (u8_t *)&volloss_data[12], sizeof(volloss_event.apC));
				rt_memcpy(((u8_t *)vb->value)+64, (u8_t *)&volloss_data[13], sizeof(volloss_event.rapC));
				rt_memcpy(((u8_t *)vb->value)+68, (u8_t *)&volloss_data[14], sizeof(volloss_event.pfC));
				rt_memcpy(((u8_t *)vb->value)+72, (u8_t *)&volloss_data[15], sizeof(volloss_event.ahnA));
				rt_memcpy(((u8_t *)vb->value)+76, (u8_t *)&volloss_data[16], sizeof(volloss_event.ahnB));
				rt_memcpy(((u8_t *)vb->value)+80, (u8_t *)&volloss_data[17], sizeof(volloss_event.ahnC));	
				//rt_memcpy((struct sinkinfo_em_volloss_event_st *)vb->value, &sinkinfo_all_em[portid].si_em_phasebreak_event_pb[1], SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
		}
		break;
		case E_ALR_PB_CURRENT_LOSS_EVENT_APPEAR:
		case E_ALR_PB_CURRENT_LOSS_EVENT_DISAPPEAR:{
			u32_t curloss_data[32]= {0};
			struct sinkinfo_em_curloss_event_st curloss_event;
			
			vb = snmp_varbind_alloc(&lt300_meter_sn_oid, SNMP_ASN1_OC_STR, 12);
	 		if (vb != NULL){
				get_dev_sn_em_sn(SDT_ELECTRIC_METER, vb->value, DEV_SN_MODE_LEN, portid);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			vb = snmp_varbind_alloc(&lt300_meter_alarm_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			if (FRAME_E_OK != get_event_data_from_ammeter((rt_uint8_t *)amm_sn.em_sn[portid], AMM_EVENT_B_LOSE_CURRENT, 1, eventdata, &data_len, RS485_PORT_USED_BY_645)){
				printf_syn("%s(), meter pb cur loss event data read fail\n", __FUNCTION__);

			}else{
				curloss_event.start_time[0] = eventdata[5];
				curloss_event.start_time[1] = eventdata[4];
				curloss_event.start_time[2] = eventdata[3];
				curloss_event.start_time[3] = eventdata[2];
				curloss_event.start_time[4] = eventdata[1];
				curloss_event.start_time[5] = eventdata[0];
				curloss_event.end_time[0] = eventdata[11];
				curloss_event.end_time[1] = eventdata[10];
				curloss_event.end_time[2] = eventdata[9];
				curloss_event.end_time[3] = eventdata[8];
				curloss_event.end_time[4] = eventdata[7];
				curloss_event.end_time[5] = eventdata[6];
				curloss_event.vA = eventdata[13]<<8 | eventdata[12];
				curloss_event.iA = eventdata[16]<<16 | eventdata[15]<<8 | eventdata[14];
				curloss_event.apA = eventdata[19]<<16 | eventdata[18]<<8 | eventdata[17];
				curloss_event.rapA = eventdata[22]<<16 | eventdata[21]<<8 | eventdata[20];
				curloss_event.pfA = eventdata[24]<<8 | eventdata[23];
				curloss_event.vB = eventdata[26]<<8 | eventdata[25];
				curloss_event.iB = eventdata[29]<<16 | eventdata[28]<<8 | eventdata[27];
				curloss_event.apB = eventdata[32]<<16 | eventdata[31]<<8 | eventdata[30];
				curloss_event.rapB = eventdata[35]<<16 | eventdata[34]<<8 | eventdata[33];
				curloss_event.pfB = eventdata[37]<<8 | eventdata[36];
				curloss_event.vC = eventdata[39]<<8 | eventdata[38];
				curloss_event.iC = eventdata[42]<<16 | eventdata[41]<<8 | eventdata[40];
				curloss_event.apC = eventdata[45]<<16 | eventdata[44]<<8 | eventdata[43];
				curloss_event.rapC = eventdata[48]<<16 | eventdata[47]<<8 | eventdata[46];
				curloss_event.pfC = eventdata[50]<<8 | eventdata[49];
				
				curloss_data[0] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.vA));
				curloss_data[1] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.iA));
				curloss_data[2] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.apA));
				curloss_data[3] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.rapA));
				curloss_data[4] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.pfA));
				curloss_data[5] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.vB));
				curloss_data[6] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.iB));
				curloss_data[7] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.apB));
				curloss_data[8] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.rapB));
				curloss_data[9] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.pfB));
				curloss_data[10] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.vC));
				curloss_data[11] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.iC));
				curloss_data[12] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.apC));
				curloss_data[13] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.rapC));
				curloss_data[14] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.pfC));
			}
			vb = snmp_varbind_alloc(&lt300_px_curloss_trap_oid, SNMP_ASN1_OC_STR, sizeof(curloss_event)-2*2);
	 		if (vb != NULL){
				//rt_memcpy(vb->value, eventdata, data_len);
				rt_memcpy((u8_t *)vb->value, curloss_event.start_time, sizeof(curloss_event.start_time)-2);
				rt_memcpy(((u8_t *)vb->value)+6, curloss_event.end_time, sizeof(curloss_event.end_time)-2);
				rt_memcpy(((u8_t *)vb->value)+12, (u8_t *)&curloss_data[0], sizeof(curloss_event.vA));
				rt_memcpy(((u8_t *)vb->value)+16, (u8_t *)&curloss_data[1], sizeof(curloss_event.iA));
				rt_memcpy(((u8_t *)vb->value)+20, (u8_t *)&curloss_data[2], sizeof(curloss_event.apA));
				rt_memcpy(((u8_t *)vb->value)+24, (u8_t *)&curloss_data[3], sizeof(curloss_event.rapA));
				rt_memcpy(((u8_t *)vb->value)+28, (u8_t *)&curloss_data[4], sizeof(curloss_event.pfA));
				rt_memcpy(((u8_t *)vb->value)+32, (u8_t *)&curloss_data[5], sizeof(curloss_event.vB));
				rt_memcpy(((u8_t *)vb->value)+36, (u8_t *)&curloss_data[6], sizeof(curloss_event.iB));
				rt_memcpy(((u8_t *)vb->value)+40, (u8_t *)&curloss_data[7], sizeof(curloss_event.apB));
				rt_memcpy(((u8_t *)vb->value)+44, (u8_t *)&curloss_data[8], sizeof(curloss_event.rapB));
				rt_memcpy(((u8_t *)vb->value)+48, (u8_t *)&curloss_data[9], sizeof(curloss_event.pfB));
				rt_memcpy(((u8_t *)vb->value)+52, (u8_t *)&curloss_data[10], sizeof(curloss_event.vC));
				rt_memcpy(((u8_t *)vb->value)+56, (u8_t *)&curloss_data[11], sizeof(curloss_event.iC));
				rt_memcpy(((u8_t *)vb->value)+60, (u8_t *)&curloss_data[12], sizeof(curloss_event.apC));
				rt_memcpy(((u8_t *)vb->value)+64, (u8_t *)&curloss_data[13], sizeof(curloss_event.rapC));
				rt_memcpy(((u8_t *)vb->value)+68, (u8_t *)&curloss_data[14], sizeof(curloss_event.pfC));				
				//rt_memcpy((struct sinkinfo_em_curloss_event_st *)vb->value, &sinkinfo_all_em[portid].si_em_curloss_event_pb[1], SINKINFO_EM_PX_CURLOSS_EVENT_SIZE);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
		}
		break;
		case E_ALR_PB_CURRENT_OVER_EVENT_APPEAR:
		case E_ALR_PB_CURRENT_OVER_EVENT_DISAPPEAR:{
			u32_t curloss_data[32]= {0};
			struct sinkinfo_em_curloss_event_st curloss_event;
			
			vb = snmp_varbind_alloc(&lt300_meter_sn_oid, SNMP_ASN1_OC_STR, 12);
	 		if (vb != NULL){
				get_dev_sn_em_sn(SDT_ELECTRIC_METER, vb->value, DEV_SN_MODE_LEN, portid);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			vb = snmp_varbind_alloc(&lt300_meter_alarm_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			if (FRAME_E_OK != get_event_data_from_ammeter((rt_uint8_t *)amm_sn.em_sn[portid], AMM_EVENT_B_OVER_CURRENT, 1, eventdata, &data_len, RS485_PORT_USED_BY_645)){
				printf_syn("%s(), meter pb cur over event data read fail\n", __FUNCTION__);

			}else{
				curloss_event.start_time[0] = eventdata[5];
				curloss_event.start_time[1] = eventdata[4];
				curloss_event.start_time[2] = eventdata[3];
				curloss_event.start_time[3] = eventdata[2];
				curloss_event.start_time[4] = eventdata[1];
				curloss_event.start_time[5] = eventdata[0];
				curloss_event.end_time[0] = eventdata[11];
				curloss_event.end_time[1] = eventdata[10];
				curloss_event.end_time[2] = eventdata[9];
				curloss_event.end_time[3] = eventdata[8];
				curloss_event.end_time[4] = eventdata[7];
				curloss_event.end_time[5] = eventdata[6];
				curloss_event.vA = eventdata[13]<<8 | eventdata[12];
				curloss_event.iA = eventdata[16]<<16 | eventdata[15]<<8 | eventdata[14];
				curloss_event.apA = eventdata[19]<<16 | eventdata[18]<<8 | eventdata[17];
				curloss_event.rapA = eventdata[22]<<16 | eventdata[21]<<8 | eventdata[20];
				curloss_event.pfA = eventdata[24]<<8 | eventdata[23];
				curloss_event.vB = eventdata[26]<<8 | eventdata[25];
				curloss_event.iB = eventdata[29]<<16 | eventdata[28]<<8 | eventdata[27];
				curloss_event.apB = eventdata[32]<<16 | eventdata[31]<<8 | eventdata[30];
				curloss_event.rapB = eventdata[35]<<16 | eventdata[34]<<8 | eventdata[33];
				curloss_event.pfB = eventdata[37]<<8 | eventdata[36];
				curloss_event.vC = eventdata[39]<<8 | eventdata[38];
				curloss_event.iC = eventdata[42]<<16 | eventdata[41]<<8 | eventdata[40];
				curloss_event.apC = eventdata[45]<<16 | eventdata[44]<<8 | eventdata[43];
				curloss_event.rapC = eventdata[48]<<16 | eventdata[47]<<8 | eventdata[46];
				curloss_event.pfC = eventdata[50]<<8 | eventdata[49];
				
				curloss_data[0] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.vA));
				curloss_data[1] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.iA));
				curloss_data[2] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.apA));
				curloss_data[3] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.rapA));
				curloss_data[4] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.pfA));
				curloss_data[5] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.vB));
				curloss_data[6] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.iB));
				curloss_data[7] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.apB));
				curloss_data[8] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.rapB));
				curloss_data[9] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.pfB));
				curloss_data[10] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.vC));
				curloss_data[11] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.iC));
				curloss_data[12] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.apC));
				curloss_data[13] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.rapC));
				curloss_data[14] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.pfC));

			}
			vb = snmp_varbind_alloc(&lt300_px_curover_trap_oid, SNMP_ASN1_OC_STR, sizeof(curloss_event)-2*2);
	 		if (vb != NULL){
				//rt_memcpy(vb->value, eventdata, data_len);
				rt_memcpy((u8_t *)vb->value, curloss_event.start_time, sizeof(curloss_event.start_time)-2);
				rt_memcpy(((u8_t *)vb->value)+6, curloss_event.end_time, sizeof(curloss_event.end_time)-2);
				rt_memcpy(((u8_t *)vb->value)+12, (u8_t *)&curloss_data[0], sizeof(curloss_event.vA));
				rt_memcpy(((u8_t *)vb->value)+16, (u8_t *)&curloss_data[1], sizeof(curloss_event.iA));
				rt_memcpy(((u8_t *)vb->value)+20, (u8_t *)&curloss_data[2], sizeof(curloss_event.apA));
				rt_memcpy(((u8_t *)vb->value)+24, (u8_t *)&curloss_data[3], sizeof(curloss_event.rapA));
				rt_memcpy(((u8_t *)vb->value)+28, (u8_t *)&curloss_data[4], sizeof(curloss_event.pfA));
				rt_memcpy(((u8_t *)vb->value)+32, (u8_t *)&curloss_data[5], sizeof(curloss_event.vB));
				rt_memcpy(((u8_t *)vb->value)+36, (u8_t *)&curloss_data[6], sizeof(curloss_event.iB));
				rt_memcpy(((u8_t *)vb->value)+40, (u8_t *)&curloss_data[7], sizeof(curloss_event.apB));
				rt_memcpy(((u8_t *)vb->value)+44, (u8_t *)&curloss_data[8], sizeof(curloss_event.rapB));
				rt_memcpy(((u8_t *)vb->value)+48, (u8_t *)&curloss_data[9], sizeof(curloss_event.pfB));
				rt_memcpy(((u8_t *)vb->value)+52, (u8_t *)&curloss_data[10], sizeof(curloss_event.vC));
				rt_memcpy(((u8_t *)vb->value)+56, (u8_t *)&curloss_data[11], sizeof(curloss_event.iC));
				rt_memcpy(((u8_t *)vb->value)+60, (u8_t *)&curloss_data[12], sizeof(curloss_event.apC));
				rt_memcpy(((u8_t *)vb->value)+64, (u8_t *)&curloss_data[13], sizeof(curloss_event.rapC));
				rt_memcpy(((u8_t *)vb->value)+68, (u8_t *)&curloss_data[14], sizeof(curloss_event.pfC));		
				//rt_memcpy((struct sinkinfo_em_curloss_event_st *)vb->value, &sinkinfo_all_em[portid].si_em_curover_event_pb[1], SINKINFO_EM_PX_CURLOSS_EVENT_SIZE);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
		}
		break;
		case E_ALR_PB_CURRENT_BREAK_EVENT_APPEAR:
		case E_ALR_PB_CURRENT_BREAK_EVENT_DISAPPEAR:{
			u32_t curloss_data[32]= {0};
			struct sinkinfo_em_curloss_event_st curloss_event;
			
			vb = snmp_varbind_alloc(&lt300_meter_sn_oid, SNMP_ASN1_OC_STR, 12);
	 		if (vb != NULL){
				get_dev_sn_em_sn(SDT_ELECTRIC_METER, vb->value, DEV_SN_MODE_LEN, portid);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			vb = snmp_varbind_alloc(&lt300_meter_alarm_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			if (FRAME_E_OK != get_event_data_from_ammeter((rt_uint8_t *)amm_sn.em_sn[portid], AMM_EVENT_B_BROKEN_CURRENT, 1, eventdata, &data_len, RS485_PORT_USED_BY_645)){
				printf_syn("%s(), meter pb cur break event data read fail\n", __FUNCTION__);

			}else{
				curloss_event.start_time[0] = eventdata[5];
				curloss_event.start_time[1] = eventdata[4];
				curloss_event.start_time[2] = eventdata[3];
				curloss_event.start_time[3] = eventdata[2];
				curloss_event.start_time[4] = eventdata[1];
				curloss_event.start_time[5] = eventdata[0];
				curloss_event.end_time[0] = eventdata[11];
				curloss_event.end_time[1] = eventdata[10];
				curloss_event.end_time[2] = eventdata[9];
				curloss_event.end_time[3] = eventdata[8];
				curloss_event.end_time[4] = eventdata[7];
				curloss_event.end_time[5] = eventdata[6];
				curloss_event.vA = eventdata[13]<<8 | eventdata[12];
				curloss_event.iA = eventdata[16]<<16 | eventdata[15]<<8 | eventdata[14];
				curloss_event.apA = eventdata[19]<<16 | eventdata[18]<<8 | eventdata[17];
				curloss_event.rapA = eventdata[22]<<16 | eventdata[21]<<8 | eventdata[20];
				curloss_event.pfA = eventdata[24]<<8 | eventdata[23];
				curloss_event.vB = eventdata[26]<<8 | eventdata[25];
				curloss_event.iB = eventdata[29]<<16 | eventdata[28]<<8 | eventdata[27];
				curloss_event.apB = eventdata[32]<<16 | eventdata[31]<<8 | eventdata[30];
				curloss_event.rapB = eventdata[35]<<16 | eventdata[34]<<8 | eventdata[33];
				curloss_event.pfB = eventdata[37]<<8 | eventdata[36];
				curloss_event.vC = eventdata[39]<<8 | eventdata[38];
				curloss_event.iC = eventdata[42]<<16 | eventdata[41]<<8 | eventdata[40];
				curloss_event.apC = eventdata[45]<<16 | eventdata[44]<<8 | eventdata[43];
				curloss_event.rapC = eventdata[48]<<16 | eventdata[47]<<8 | eventdata[46];
				curloss_event.pfC = eventdata[50]<<8 | eventdata[49];
				
				curloss_data[0] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.vA));
				curloss_data[1] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.iA));
				curloss_data[2] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.apA));
				curloss_data[3] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.rapA));
				curloss_data[4] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.pfA));
				curloss_data[5] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.vB));
				curloss_data[6] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.iB));
				curloss_data[7] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.apB));
				curloss_data[8] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.rapB));
				curloss_data[9] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.pfB));
				curloss_data[10] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.vC));
				curloss_data[11] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.iC));
				curloss_data[12] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.apC));
				curloss_data[13] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.rapC));
				curloss_data[14] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.pfC));
			}
			vb = snmp_varbind_alloc(&lt300_px_curbreak_trap_oid, SNMP_ASN1_OC_STR, sizeof(curloss_event)-2*2);
	 		if (vb != NULL){
				//rt_memcpy(vb->value, eventdata, data_len);
				rt_memcpy((u8_t *)vb->value, curloss_event.start_time, sizeof(curloss_event.start_time)-2);
				rt_memcpy(((u8_t *)vb->value)+6, curloss_event.end_time, sizeof(curloss_event.end_time)-2);
				rt_memcpy(((u8_t *)vb->value)+12, (u8_t *)&curloss_data[0], sizeof(curloss_event.vA));
				rt_memcpy(((u8_t *)vb->value)+16, (u8_t *)&curloss_data[1], sizeof(curloss_event.iA));
				rt_memcpy(((u8_t *)vb->value)+20, (u8_t *)&curloss_data[2], sizeof(curloss_event.apA));
				rt_memcpy(((u8_t *)vb->value)+24, (u8_t *)&curloss_data[3], sizeof(curloss_event.rapA));
				rt_memcpy(((u8_t *)vb->value)+28, (u8_t *)&curloss_data[4], sizeof(curloss_event.pfA));
				rt_memcpy(((u8_t *)vb->value)+32, (u8_t *)&curloss_data[5], sizeof(curloss_event.vB));
				rt_memcpy(((u8_t *)vb->value)+36, (u8_t *)&curloss_data[6], sizeof(curloss_event.iB));
				rt_memcpy(((u8_t *)vb->value)+40, (u8_t *)&curloss_data[7], sizeof(curloss_event.apB));
				rt_memcpy(((u8_t *)vb->value)+44, (u8_t *)&curloss_data[8], sizeof(curloss_event.rapB));
				rt_memcpy(((u8_t *)vb->value)+48, (u8_t *)&curloss_data[9], sizeof(curloss_event.pfB));
				rt_memcpy(((u8_t *)vb->value)+52, (u8_t *)&curloss_data[10], sizeof(curloss_event.vC));
				rt_memcpy(((u8_t *)vb->value)+56, (u8_t *)&curloss_data[11], sizeof(curloss_event.iC));
				rt_memcpy(((u8_t *)vb->value)+60, (u8_t *)&curloss_data[12], sizeof(curloss_event.apC));
				rt_memcpy(((u8_t *)vb->value)+64, (u8_t *)&curloss_data[13], sizeof(curloss_event.rapC));
				rt_memcpy(((u8_t *)vb->value)+68, (u8_t *)&curloss_data[14], sizeof(curloss_event.pfC));	
				//rt_memcpy((struct sinkinfo_em_curloss_event_st *)vb->value, &sinkinfo_all_em[portid].si_em_curbreak_event_pb[1], SINKINFO_EM_PX_CURLOSS_EVENT_SIZE);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
		}
		break;
		case E_ALR_PC_VOLTAGE_LOSS_EVENT_APPEAR:
		case E_ALR_PC_VOLTAGE_LOSS_EVENT_DISAPPEAR:{
			u32_t volloss_data[32]= {0};
			struct sinkinfo_em_volloss_event_st volloss_event;
			
			vb = snmp_varbind_alloc(&lt300_meter_sn_oid, SNMP_ASN1_OC_STR, 12);
	 		if (vb != NULL){
				get_dev_sn_em_sn(SDT_ELECTRIC_METER, vb->value, DEV_SN_MODE_LEN, portid);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			vb = snmp_varbind_alloc(&lt300_meter_alarm_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			if (FRAME_E_OK != get_event_data_from_ammeter((rt_uint8_t *)amm_sn.em_sn[portid], AMM_EVENT_C_LOSE_VOLTAGE, 1, eventdata, &data_len, RS485_PORT_USED_BY_645)){
				printf_syn("%s(), meter pc vol loss event data read fail\n", __FUNCTION__);

			}else{
				volloss_event.start_time[0] = eventdata[5];
				volloss_event.start_time[1] = eventdata[4];
				volloss_event.start_time[2] = eventdata[3];
				volloss_event.start_time[3] = eventdata[2];
				volloss_event.start_time[4] = eventdata[1];
				volloss_event.start_time[5] = eventdata[0];
				volloss_event.end_time[0] = eventdata[11];
				volloss_event.end_time[1] = eventdata[10];
				volloss_event.end_time[2] = eventdata[9];
				volloss_event.end_time[3] = eventdata[8];
				volloss_event.end_time[4] = eventdata[7];
				volloss_event.end_time[5] = eventdata[6];
				volloss_event.vA = eventdata[13]<<8 | eventdata[12];
				volloss_event.iA = eventdata[16]<<16 | eventdata[15]<<8 | eventdata[14];
				volloss_event.apA = eventdata[19]<<16 | eventdata[18]<<8 | eventdata[17];
				volloss_event.rapA = eventdata[22]<<16 | eventdata[21]<<8 | eventdata[20];
				volloss_event.pfA = eventdata[24]<<8 | eventdata[23];
				volloss_event.vB = eventdata[26]<<8 | eventdata[25];
				volloss_event.iB = eventdata[29]<<16 | eventdata[28]<<8 | eventdata[27];
				volloss_event.apB = eventdata[32]<<16 | eventdata[31]<<8 | eventdata[30];
				volloss_event.rapB = eventdata[35]<<16 | eventdata[34]<<8 | eventdata[33];
				volloss_event.pfB = eventdata[37]<<8 | eventdata[36];
				volloss_event.vC = eventdata[39]<<8 | eventdata[38];
				volloss_event.iC = eventdata[42]<<16 | eventdata[41]<<8 | eventdata[40];
				volloss_event.apC = eventdata[45]<<16 | eventdata[44]<<8 | eventdata[43];
				volloss_event.rapC = eventdata[48]<<16 | eventdata[47]<<8 | eventdata[46];
				volloss_event.pfC = eventdata[50]<<8 | eventdata[49];
				volloss_event.ahnA = eventdata[54]<<24 | eventdata[53]<<16 | eventdata[52]<<8 | eventdata[51];
				volloss_event.ahnB = eventdata[58]<<24 | eventdata[57]<<16 | eventdata[56]<<8 | eventdata[55];
				volloss_event.ahnC = eventdata[62]<<24 | eventdata[61]<<16 | eventdata[60]<<8 | eventdata[59];
				
				volloss_data[0] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.vA));
				volloss_data[1] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.iA));
				volloss_data[2] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.apA));
				volloss_data[3] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.rapA));
				volloss_data[4] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.pfA));
				volloss_data[5] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.vB));
				volloss_data[6] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.iB));
				volloss_data[7] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.apB));
				volloss_data[8] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.rapB));
				volloss_data[9] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.pfB));
				volloss_data[10] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.vC));
				volloss_data[11] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.iC));
				volloss_data[12] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.apC));
				volloss_data[13] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.rapC));
				volloss_data[14] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.pfC));
				volloss_data[15] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.ahnA));
				volloss_data[16] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.ahnB));
				volloss_data[17] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.ahnC));

			}
			vb = snmp_varbind_alloc(&lt300_px_volloss_trap_oid, SNMP_ASN1_OC_STR, sizeof(volloss_event)-2*2);
	 		if (vb != NULL){
				//rt_memcpy(vb->value, eventdata, data_len);
				rt_memcpy((u8_t *)vb->value, volloss_event.start_time, sizeof(volloss_event.start_time)-2);
				rt_memcpy(((u8_t *)vb->value)+6, volloss_event.end_time, sizeof(volloss_event.end_time)-2);
				rt_memcpy(((u8_t *)vb->value)+12, (u8_t *)&volloss_data[0], sizeof(volloss_event.vA));
				rt_memcpy(((u8_t *)vb->value)+16, (u8_t *)&volloss_data[1], sizeof(volloss_event.iA));
				rt_memcpy(((u8_t *)vb->value)+20, (u8_t *)&volloss_data[2], sizeof(volloss_event.apA));
				rt_memcpy(((u8_t *)vb->value)+24, (u8_t *)&volloss_data[3], sizeof(volloss_event.rapA));
				rt_memcpy(((u8_t *)vb->value)+28, (u8_t *)&volloss_data[4], sizeof(volloss_event.pfA));
				rt_memcpy(((u8_t *)vb->value)+32, (u8_t *)&volloss_data[5], sizeof(volloss_event.vB));
				rt_memcpy(((u8_t *)vb->value)+36, (u8_t *)&volloss_data[6], sizeof(volloss_event.iB));
				rt_memcpy(((u8_t *)vb->value)+40, (u8_t *)&volloss_data[7], sizeof(volloss_event.apB));
				rt_memcpy(((u8_t *)vb->value)+44, (u8_t *)&volloss_data[8], sizeof(volloss_event.rapB));
				rt_memcpy(((u8_t *)vb->value)+48, (u8_t *)&volloss_data[9], sizeof(volloss_event.pfB));
				rt_memcpy(((u8_t *)vb->value)+52, (u8_t *)&volloss_data[10], sizeof(volloss_event.vC));
				rt_memcpy(((u8_t *)vb->value)+56, (u8_t *)&volloss_data[11], sizeof(volloss_event.iC));
				rt_memcpy(((u8_t *)vb->value)+60, (u8_t *)&volloss_data[12], sizeof(volloss_event.apC));
				rt_memcpy(((u8_t *)vb->value)+64, (u8_t *)&volloss_data[13], sizeof(volloss_event.rapC));
				rt_memcpy(((u8_t *)vb->value)+68, (u8_t *)&volloss_data[14], sizeof(volloss_event.pfC));
				rt_memcpy(((u8_t *)vb->value)+72, (u8_t *)&volloss_data[15], sizeof(volloss_event.ahnA));
				rt_memcpy(((u8_t *)vb->value)+76, (u8_t *)&volloss_data[16], sizeof(volloss_event.ahnB));
				rt_memcpy(((u8_t *)vb->value)+80, (u8_t *)&volloss_data[17], sizeof(volloss_event.ahnC));	
				//rt_memcpy((struct sinkinfo_em_volloss_event_st *)vb->value, &sinkinfo_all_em[portid].si_em_volloss_event_pc[1], SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
		}
		break;
		case E_ALR_PC_VOLTAGE_OVER_EVENT_APPEAR:
		case E_ALR_PC_VOLTAGE_OVER_EVENT_DISAPPEAR:{
			u32_t volloss_data[32]= {0};
			struct sinkinfo_em_volloss_event_st volloss_event;
			
			vb = snmp_varbind_alloc(&lt300_meter_sn_oid, SNMP_ASN1_OC_STR, 12);
	 		if (vb != NULL){
				get_dev_sn_em_sn(SDT_ELECTRIC_METER, vb->value, DEV_SN_MODE_LEN, portid);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			vb = snmp_varbind_alloc(&lt300_meter_alarm_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			if (FRAME_E_OK != get_event_data_from_ammeter((rt_uint8_t *)amm_sn.em_sn[portid], AMM_EVENT_C_OVER_VOLTAGE, 1, eventdata, &data_len, RS485_PORT_USED_BY_645)){
				printf_syn("%s(), meter pc vol over event data read fail\n", __FUNCTION__);

			}else{
				volloss_event.start_time[0] = eventdata[5];
				volloss_event.start_time[1] = eventdata[4];
				volloss_event.start_time[2] = eventdata[3];
				volloss_event.start_time[3] = eventdata[2];
				volloss_event.start_time[4] = eventdata[1];
				volloss_event.start_time[5] = eventdata[0];
				volloss_event.end_time[0] = eventdata[11];
				volloss_event.end_time[1] = eventdata[10];
				volloss_event.end_time[2] = eventdata[9];
				volloss_event.end_time[3] = eventdata[8];
				volloss_event.end_time[4] = eventdata[7];
				volloss_event.end_time[5] = eventdata[6];
				volloss_event.vA = eventdata[13]<<8 | eventdata[12];
				volloss_event.iA = eventdata[16]<<16 | eventdata[15]<<8 | eventdata[14];
				volloss_event.apA = eventdata[19]<<16 | eventdata[18]<<8 | eventdata[17];
				volloss_event.rapA = eventdata[22]<<16 | eventdata[21]<<8 | eventdata[20];
				volloss_event.pfA = eventdata[24]<<8 | eventdata[23];
				volloss_event.vB = eventdata[26]<<8 | eventdata[25];
				volloss_event.iB = eventdata[29]<<16 | eventdata[28]<<8 | eventdata[27];
				volloss_event.apB = eventdata[32]<<16 | eventdata[31]<<8 | eventdata[30];
				volloss_event.rapB = eventdata[35]<<16 | eventdata[34]<<8 | eventdata[33];
				volloss_event.pfB = eventdata[37]<<8 | eventdata[36];
				volloss_event.vC = eventdata[39]<<8 | eventdata[38];
				volloss_event.iC = eventdata[42]<<16 | eventdata[41]<<8 | eventdata[40];
				volloss_event.apC = eventdata[45]<<16 | eventdata[44]<<8 | eventdata[43];
				volloss_event.rapC = eventdata[48]<<16 | eventdata[47]<<8 | eventdata[46];
				volloss_event.pfC = eventdata[50]<<8 | eventdata[49];
				volloss_event.ahnA = eventdata[54]<<24 | eventdata[53]<<16 | eventdata[52]<<8 | eventdata[51];
				volloss_event.ahnB = eventdata[58]<<24 | eventdata[57]<<16 | eventdata[56]<<8 | eventdata[55];
				volloss_event.ahnC = eventdata[62]<<24 | eventdata[61]<<16 | eventdata[60]<<8 | eventdata[59];
				
				volloss_data[0] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.vA));
				volloss_data[1] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.iA));
				volloss_data[2] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.apA));
				volloss_data[3] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.rapA));
				volloss_data[4] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.pfA));
				volloss_data[5] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.vB));
				volloss_data[6] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.iB));
				volloss_data[7] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.apB));
				volloss_data[8] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.rapB));
				volloss_data[9] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.pfB));
				volloss_data[10] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.vC));
				volloss_data[11] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.iC));
				volloss_data[12] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.apC));
				volloss_data[13] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.rapC));
				volloss_data[14] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.pfC));
				volloss_data[15] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.ahnA));
				volloss_data[16] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.ahnB));
				volloss_data[17] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.ahnC));
			}
			vb = snmp_varbind_alloc(&lt300_px_volover_trap_oid, SNMP_ASN1_OC_STR, sizeof(volloss_event)-2*2);
	 		if (vb != NULL){
				//rt_memcpy(vb->value, eventdata, data_len);
				rt_memcpy((u8_t *)vb->value, volloss_event.start_time, sizeof(volloss_event.start_time)-2);
				rt_memcpy(((u8_t *)vb->value)+6, volloss_event.end_time, sizeof(volloss_event.end_time)-2);
				rt_memcpy(((u8_t *)vb->value)+12, (u8_t *)&volloss_data[0], sizeof(volloss_event.vA));
				rt_memcpy(((u8_t *)vb->value)+16, (u8_t *)&volloss_data[1], sizeof(volloss_event.iA));
				rt_memcpy(((u8_t *)vb->value)+20, (u8_t *)&volloss_data[2], sizeof(volloss_event.apA));
				rt_memcpy(((u8_t *)vb->value)+24, (u8_t *)&volloss_data[3], sizeof(volloss_event.rapA));
				rt_memcpy(((u8_t *)vb->value)+28, (u8_t *)&volloss_data[4], sizeof(volloss_event.pfA));
				rt_memcpy(((u8_t *)vb->value)+32, (u8_t *)&volloss_data[5], sizeof(volloss_event.vB));
				rt_memcpy(((u8_t *)vb->value)+36, (u8_t *)&volloss_data[6], sizeof(volloss_event.iB));
				rt_memcpy(((u8_t *)vb->value)+40, (u8_t *)&volloss_data[7], sizeof(volloss_event.apB));
				rt_memcpy(((u8_t *)vb->value)+44, (u8_t *)&volloss_data[8], sizeof(volloss_event.rapB));
				rt_memcpy(((u8_t *)vb->value)+48, (u8_t *)&volloss_data[9], sizeof(volloss_event.pfB));
				rt_memcpy(((u8_t *)vb->value)+52, (u8_t *)&volloss_data[10], sizeof(volloss_event.vC));
				rt_memcpy(((u8_t *)vb->value)+56, (u8_t *)&volloss_data[11], sizeof(volloss_event.iC));
				rt_memcpy(((u8_t *)vb->value)+60, (u8_t *)&volloss_data[12], sizeof(volloss_event.apC));
				rt_memcpy(((u8_t *)vb->value)+64, (u8_t *)&volloss_data[13], sizeof(volloss_event.rapC));
				rt_memcpy(((u8_t *)vb->value)+68, (u8_t *)&volloss_data[14], sizeof(volloss_event.pfC));
				rt_memcpy(((u8_t *)vb->value)+72, (u8_t *)&volloss_data[15], sizeof(volloss_event.ahnA));
				rt_memcpy(((u8_t *)vb->value)+76, (u8_t *)&volloss_data[16], sizeof(volloss_event.ahnB));
				rt_memcpy(((u8_t *)vb->value)+80, (u8_t *)&volloss_data[17], sizeof(volloss_event.ahnC));		
				//rt_memcpy((struct sinkinfo_em_volloss_event_st *)vb->value, &sinkinfo_all_em[portid].si_em_volover_event_pc[1], SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE);		
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
		}
		break;
		case E_ALR_PC_VOLTAGE_UNDER_EVENT_APPEAR:
		case E_ALR_PC_VOLTAGE_UNDER_EVENT_DISAPPEAR:{
			u32_t volloss_data[32]= {0};
			struct sinkinfo_em_volloss_event_st volloss_event;
			
			vb = snmp_varbind_alloc(&lt300_meter_sn_oid, SNMP_ASN1_OC_STR, 12);
	 		if (vb != NULL){	
				get_dev_sn_em_sn(SDT_ELECTRIC_METER, vb->value, DEV_SN_MODE_LEN, portid);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			vb = snmp_varbind_alloc(&lt300_meter_alarm_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			if (FRAME_E_OK != get_event_data_from_ammeter((rt_uint8_t *)amm_sn.em_sn[portid], AMM_EVENT_C_OWE_VOLTAGE, 1, eventdata, &data_len, RS485_PORT_USED_BY_645)){
				printf_syn("%s(), meter pc vol under event data read fail\n", __FUNCTION__);

			}else{
				volloss_event.start_time[0] = eventdata[5];
				volloss_event.start_time[1] = eventdata[4];
				volloss_event.start_time[2] = eventdata[3];
				volloss_event.start_time[3] = eventdata[2];
				volloss_event.start_time[4] = eventdata[1];
				volloss_event.start_time[5] = eventdata[0];
				volloss_event.end_time[0] = eventdata[11];
				volloss_event.end_time[1] = eventdata[10];
				volloss_event.end_time[2] = eventdata[9];
				volloss_event.end_time[3] = eventdata[8];
				volloss_event.end_time[4] = eventdata[7];
				volloss_event.end_time[5] = eventdata[6];
				volloss_event.vA = eventdata[13]<<8 | eventdata[12];
				volloss_event.iA = eventdata[16]<<16 | eventdata[15]<<8 | eventdata[14];
				volloss_event.apA = eventdata[19]<<16 | eventdata[18]<<8 | eventdata[17];
				volloss_event.rapA = eventdata[22]<<16 | eventdata[21]<<8 | eventdata[20];
				volloss_event.pfA = eventdata[24]<<8 | eventdata[23];
				volloss_event.vB = eventdata[26]<<8 | eventdata[25];
				volloss_event.iB = eventdata[29]<<16 | eventdata[28]<<8 | eventdata[27];
				volloss_event.apB = eventdata[32]<<16 | eventdata[31]<<8 | eventdata[30];
				volloss_event.rapB = eventdata[35]<<16 | eventdata[34]<<8 | eventdata[33];
				volloss_event.pfB = eventdata[37]<<8 | eventdata[36];
				volloss_event.vC = eventdata[39]<<8 | eventdata[38];
				volloss_event.iC = eventdata[42]<<16 | eventdata[41]<<8 | eventdata[40];
				volloss_event.apC = eventdata[45]<<16 | eventdata[44]<<8 | eventdata[43];
				volloss_event.rapC = eventdata[48]<<16 | eventdata[47]<<8 | eventdata[46];
				volloss_event.pfC = eventdata[50]<<8 | eventdata[49];
				volloss_event.ahnA = eventdata[54]<<24 | eventdata[53]<<16 | eventdata[52]<<8 | eventdata[51];
				volloss_event.ahnB = eventdata[58]<<24 | eventdata[57]<<16 | eventdata[56]<<8 | eventdata[55];
				volloss_event.ahnC = eventdata[62]<<24 | eventdata[61]<<16 | eventdata[60]<<8 | eventdata[59];
				
				volloss_data[0] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.vA));
				volloss_data[1] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.iA));
				volloss_data[2] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.apA));
				volloss_data[3] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.rapA));
				volloss_data[4] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.pfA));
				volloss_data[5] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.vB));
				volloss_data[6] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.iB));
				volloss_data[7] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.apB));
				volloss_data[8] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.rapB));
				volloss_data[9] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.pfB));
				volloss_data[10] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.vC));
				volloss_data[11] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.iC));
				volloss_data[12] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.apC));
				volloss_data[13] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.rapC));
				volloss_data[14] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.pfC));
				volloss_data[15] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.ahnA));
				volloss_data[16] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.ahnB));
				volloss_data[17] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.ahnC));
			}
			vb = snmp_varbind_alloc(&lt300_px_volunder_trap_oid, SNMP_ASN1_OC_STR, sizeof(volloss_event)-2*2);
	 		if (vb != NULL){
				//rt_memcpy(vb->value, eventdata, data_len);
				rt_memcpy((u8_t *)vb->value, volloss_event.start_time, sizeof(volloss_event.start_time)-2);
				rt_memcpy(((u8_t *)vb->value)+6, volloss_event.end_time, sizeof(volloss_event.end_time)-2);
				rt_memcpy(((u8_t *)vb->value)+12, (u8_t *)&volloss_data[0], sizeof(volloss_event.vA));
				rt_memcpy(((u8_t *)vb->value)+16, (u8_t *)&volloss_data[1], sizeof(volloss_event.iA));
				rt_memcpy(((u8_t *)vb->value)+20, (u8_t *)&volloss_data[2], sizeof(volloss_event.apA));
				rt_memcpy(((u8_t *)vb->value)+24, (u8_t *)&volloss_data[3], sizeof(volloss_event.rapA));
				rt_memcpy(((u8_t *)vb->value)+28, (u8_t *)&volloss_data[4], sizeof(volloss_event.pfA));
				rt_memcpy(((u8_t *)vb->value)+32, (u8_t *)&volloss_data[5], sizeof(volloss_event.vB));
				rt_memcpy(((u8_t *)vb->value)+36, (u8_t *)&volloss_data[6], sizeof(volloss_event.iB));
				rt_memcpy(((u8_t *)vb->value)+40, (u8_t *)&volloss_data[7], sizeof(volloss_event.apB));
				rt_memcpy(((u8_t *)vb->value)+44, (u8_t *)&volloss_data[8], sizeof(volloss_event.rapB));
				rt_memcpy(((u8_t *)vb->value)+48, (u8_t *)&volloss_data[9], sizeof(volloss_event.pfB));
				rt_memcpy(((u8_t *)vb->value)+52, (u8_t *)&volloss_data[10], sizeof(volloss_event.vC));
				rt_memcpy(((u8_t *)vb->value)+56, (u8_t *)&volloss_data[11], sizeof(volloss_event.iC));
				rt_memcpy(((u8_t *)vb->value)+60, (u8_t *)&volloss_data[12], sizeof(volloss_event.apC));
				rt_memcpy(((u8_t *)vb->value)+64, (u8_t *)&volloss_data[13], sizeof(volloss_event.rapC));
				rt_memcpy(((u8_t *)vb->value)+68, (u8_t *)&volloss_data[14], sizeof(volloss_event.pfC));
				rt_memcpy(((u8_t *)vb->value)+72, (u8_t *)&volloss_data[15], sizeof(volloss_event.ahnA));
				rt_memcpy(((u8_t *)vb->value)+76, (u8_t *)&volloss_data[16], sizeof(volloss_event.ahnB));
				rt_memcpy(((u8_t *)vb->value)+80, (u8_t *)&volloss_data[17], sizeof(volloss_event.ahnC));		
				//rt_memcpy((struct sinkinfo_em_volloss_event_st *)vb->value, &sinkinfo_all_em[portid].si_em_volunder_event_pc[1], SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
		}
		break;
		case E_ALR_PC_PHASE_BREAK_EVENT_APPEAR:
		case E_ALR_PC_PHASE_BREAK_EVENT_DISAPPEAR:{
			u32_t volloss_data[32]= {0};
			struct sinkinfo_em_volloss_event_st volloss_event;
			
			vb = snmp_varbind_alloc(&lt300_meter_sn_oid, SNMP_ASN1_OC_STR, 12);
	 		if (vb != NULL){
				get_dev_sn_em_sn(SDT_ELECTRIC_METER, vb->value, DEV_SN_MODE_LEN, portid);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			vb = snmp_varbind_alloc(&lt300_meter_alarm_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			if (FRAME_E_OK != get_event_data_from_ammeter((rt_uint8_t *)amm_sn.em_sn[portid], AMM_EVENT_C_BROKEN_PHASE, 1, eventdata, &data_len, RS485_PORT_USED_BY_645)){
				printf_syn("%s(), meter pc phase break event data read fail\n", __FUNCTION__);

			}else{
				volloss_event.start_time[0] = eventdata[5];
				volloss_event.start_time[1] = eventdata[4];
				volloss_event.start_time[2] = eventdata[3];
				volloss_event.start_time[3] = eventdata[2];
				volloss_event.start_time[4] = eventdata[1];
				volloss_event.start_time[5] = eventdata[0];
				volloss_event.end_time[0] = eventdata[11];
				volloss_event.end_time[1] = eventdata[10];
				volloss_event.end_time[2] = eventdata[9];
				volloss_event.end_time[3] = eventdata[8];
				volloss_event.end_time[4] = eventdata[7];
				volloss_event.end_time[5] = eventdata[6];
				volloss_event.vA = eventdata[13]<<8 | eventdata[12];
				volloss_event.iA = eventdata[16]<<16 | eventdata[15]<<8 | eventdata[14];
				volloss_event.apA = eventdata[19]<<16 | eventdata[18]<<8 | eventdata[17];
				volloss_event.rapA = eventdata[22]<<16 | eventdata[21]<<8 | eventdata[20];
				volloss_event.pfA = eventdata[24]<<8 | eventdata[23];
				volloss_event.vB = eventdata[26]<<8 | eventdata[25];
				volloss_event.iB = eventdata[29]<<16 | eventdata[28]<<8 | eventdata[27];
				volloss_event.apB = eventdata[32]<<16 | eventdata[31]<<8 | eventdata[30];
				volloss_event.rapB = eventdata[35]<<16 | eventdata[34]<<8 | eventdata[33];
				volloss_event.pfB = eventdata[37]<<8 | eventdata[36];
				volloss_event.vC = eventdata[39]<<8 | eventdata[38];
				volloss_event.iC = eventdata[42]<<16 | eventdata[41]<<8 | eventdata[40];
				volloss_event.apC = eventdata[45]<<16 | eventdata[44]<<8 | eventdata[43];
				volloss_event.rapC = eventdata[48]<<16 | eventdata[47]<<8 | eventdata[46];
				volloss_event.pfC = eventdata[50]<<8 | eventdata[49];
				volloss_event.ahnA = eventdata[54]<<24 | eventdata[53]<<16 | eventdata[52]<<8 | eventdata[51];
				volloss_event.ahnB = eventdata[58]<<24 | eventdata[57]<<16 | eventdata[56]<<8 | eventdata[55];
				volloss_event.ahnC = eventdata[62]<<24 | eventdata[61]<<16 | eventdata[60]<<8 | eventdata[59];
				
				volloss_data[0] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.vA));
				volloss_data[1] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.iA));
				volloss_data[2] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.apA));
				volloss_data[3] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.rapA));
				volloss_data[4] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.pfA));
				volloss_data[5] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.vB));
				volloss_data[6] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.iB));
				volloss_data[7] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.apB));
				volloss_data[8] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.rapB));
				volloss_data[9] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.pfB));
				volloss_data[10] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.vC));
				volloss_data[11] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.iC));
				volloss_data[12] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.apC));
				volloss_data[13] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.rapC));
				volloss_data[14] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.pfC));
				volloss_data[15] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.ahnA));
				volloss_data[16] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.ahnB));
				volloss_data[17] = lwip_htonl(conv_4byte_bcd_to_long(volloss_event.ahnC));

			}
			vb = snmp_varbind_alloc(&lt300_px_phasebreak_trap_oid, SNMP_ASN1_OC_STR, sizeof(volloss_event)-2*2);
	 		if (vb != NULL){
				//rt_memcpy(vb->value, eventdata, data_len);
				rt_memcpy((u8_t *)vb->value, volloss_event.start_time, sizeof(volloss_event.start_time)-2);
				rt_memcpy(((u8_t *)vb->value)+6, volloss_event.end_time, sizeof(volloss_event.end_time)-2);
				rt_memcpy(((u8_t *)vb->value)+12, (u8_t *)&volloss_data[0], sizeof(volloss_event.vA));
				rt_memcpy(((u8_t *)vb->value)+16, (u8_t *)&volloss_data[1], sizeof(volloss_event.iA));
				rt_memcpy(((u8_t *)vb->value)+20, (u8_t *)&volloss_data[2], sizeof(volloss_event.apA));
				rt_memcpy(((u8_t *)vb->value)+24, (u8_t *)&volloss_data[3], sizeof(volloss_event.rapA));
				rt_memcpy(((u8_t *)vb->value)+28, (u8_t *)&volloss_data[4], sizeof(volloss_event.pfA));
				rt_memcpy(((u8_t *)vb->value)+32, (u8_t *)&volloss_data[5], sizeof(volloss_event.vB));
				rt_memcpy(((u8_t *)vb->value)+36, (u8_t *)&volloss_data[6], sizeof(volloss_event.iB));
				rt_memcpy(((u8_t *)vb->value)+40, (u8_t *)&volloss_data[7], sizeof(volloss_event.apB));
				rt_memcpy(((u8_t *)vb->value)+44, (u8_t *)&volloss_data[8], sizeof(volloss_event.rapB));
				rt_memcpy(((u8_t *)vb->value)+48, (u8_t *)&volloss_data[9], sizeof(volloss_event.pfB));
				rt_memcpy(((u8_t *)vb->value)+52, (u8_t *)&volloss_data[10], sizeof(volloss_event.vC));
				rt_memcpy(((u8_t *)vb->value)+56, (u8_t *)&volloss_data[11], sizeof(volloss_event.iC));
				rt_memcpy(((u8_t *)vb->value)+60, (u8_t *)&volloss_data[12], sizeof(volloss_event.apC));
				rt_memcpy(((u8_t *)vb->value)+64, (u8_t *)&volloss_data[13], sizeof(volloss_event.rapC));
				rt_memcpy(((u8_t *)vb->value)+68, (u8_t *)&volloss_data[14], sizeof(volloss_event.pfC));
				rt_memcpy(((u8_t *)vb->value)+72, (u8_t *)&volloss_data[15], sizeof(volloss_event.ahnA));
				rt_memcpy(((u8_t *)vb->value)+76, (u8_t *)&volloss_data[16], sizeof(volloss_event.ahnB));
				rt_memcpy(((u8_t *)vb->value)+80, (u8_t *)&volloss_data[17], sizeof(volloss_event.ahnC));		
				//rt_memcpy((struct sinkinfo_em_volloss_event_st *)vb->value, &sinkinfo_all_em[portid].si_em_phasebreak_event_pc[1], SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
		}
		break;
		case E_ALR_PC_CURRENT_LOSS_EVENT_APPEAR:
		case E_ALR_PC_CURRENT_LOSS_EVENT_DISAPPEAR:{
			u32_t curloss_data[32]= {0};
			struct sinkinfo_em_curloss_event_st curloss_event;
			
			vb = snmp_varbind_alloc(&lt300_meter_sn_oid, SNMP_ASN1_OC_STR, 12);
	 		if (vb != NULL){	
				get_dev_sn_em_sn(SDT_ELECTRIC_METER, vb->value, DEV_SN_MODE_LEN, portid);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			vb = snmp_varbind_alloc(&lt300_meter_alarm_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			if (FRAME_E_OK != get_event_data_from_ammeter((rt_uint8_t *)amm_sn.em_sn[portid], AMM_EVENT_C_LOSE_CURRENT, 1, eventdata, &data_len, RS485_PORT_USED_BY_645)){
				printf_syn("%s(), meter pc cur loss event data read fail\n", __FUNCTION__);

			}else{
				curloss_event.start_time[0] = eventdata[5];
				curloss_event.start_time[1] = eventdata[4];
				curloss_event.start_time[2] = eventdata[3];
				curloss_event.start_time[3] = eventdata[2];
				curloss_event.start_time[4] = eventdata[1];
				curloss_event.start_time[5] = eventdata[0];
				curloss_event.end_time[0] = eventdata[11];
				curloss_event.end_time[1] = eventdata[10];
				curloss_event.end_time[2] = eventdata[9];
				curloss_event.end_time[3] = eventdata[8];
				curloss_event.end_time[4] = eventdata[7];
				curloss_event.end_time[5] = eventdata[6];
				curloss_event.vA = eventdata[13]<<8 | eventdata[12];
				curloss_event.iA = eventdata[16]<<16 | eventdata[15]<<8 | eventdata[14];
				curloss_event.apA = eventdata[19]<<16 | eventdata[18]<<8 | eventdata[17];
				curloss_event.rapA = eventdata[22]<<16 | eventdata[21]<<8 | eventdata[20];
				curloss_event.pfA = eventdata[24]<<8 | eventdata[23];
				curloss_event.vB = eventdata[26]<<8 | eventdata[25];
				curloss_event.iB = eventdata[29]<<16 | eventdata[28]<<8 | eventdata[27];
				curloss_event.apB = eventdata[32]<<16 | eventdata[31]<<8 | eventdata[30];
				curloss_event.rapB = eventdata[35]<<16 | eventdata[34]<<8 | eventdata[33];
				curloss_event.pfB = eventdata[37]<<8 | eventdata[36];
				curloss_event.vC = eventdata[39]<<8 | eventdata[38];
				curloss_event.iC = eventdata[42]<<16 | eventdata[41]<<8 | eventdata[40];
				curloss_event.apC = eventdata[45]<<16 | eventdata[44]<<8 | eventdata[43];
				curloss_event.rapC = eventdata[48]<<16 | eventdata[47]<<8 | eventdata[46];
				curloss_event.pfC = eventdata[50]<<8 | eventdata[49];
				
				curloss_data[0] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.vA));
				curloss_data[1] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.iA));
				curloss_data[2] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.apA));
				curloss_data[3] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.rapA));
				curloss_data[4] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.pfA));
				curloss_data[5] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.vB));
				curloss_data[6] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.iB));
				curloss_data[7] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.apB));
				curloss_data[8] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.rapB));
				curloss_data[9] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.pfB));
				curloss_data[10] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.vC));
				curloss_data[11] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.iC));
				curloss_data[12] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.apC));
				curloss_data[13] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.rapC));
				curloss_data[14] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.pfC));
			}
			vb = snmp_varbind_alloc(&lt300_px_curloss_trap_oid, SNMP_ASN1_OC_STR, sizeof(curloss_event)-2*2);
	 		if (vb != NULL){
				//rt_memcpy(vb->value, eventdata, data_len);
				rt_memcpy((u8_t *)vb->value, curloss_event.start_time, sizeof(curloss_event.start_time)-2);
				rt_memcpy(((u8_t *)vb->value)+6, curloss_event.end_time, sizeof(curloss_event.end_time)-2);
				rt_memcpy(((u8_t *)vb->value)+12, (u8_t *)&curloss_data[0], sizeof(curloss_event.vA));
				rt_memcpy(((u8_t *)vb->value)+16, (u8_t *)&curloss_data[1], sizeof(curloss_event.iA));
				rt_memcpy(((u8_t *)vb->value)+20, (u8_t *)&curloss_data[2], sizeof(curloss_event.apA));
				rt_memcpy(((u8_t *)vb->value)+24, (u8_t *)&curloss_data[3], sizeof(curloss_event.rapA));
				rt_memcpy(((u8_t *)vb->value)+28, (u8_t *)&curloss_data[4], sizeof(curloss_event.pfA));
				rt_memcpy(((u8_t *)vb->value)+32, (u8_t *)&curloss_data[5], sizeof(curloss_event.vB));
				rt_memcpy(((u8_t *)vb->value)+36, (u8_t *)&curloss_data[6], sizeof(curloss_event.iB));
				rt_memcpy(((u8_t *)vb->value)+40, (u8_t *)&curloss_data[7], sizeof(curloss_event.apB));
				rt_memcpy(((u8_t *)vb->value)+44, (u8_t *)&curloss_data[8], sizeof(curloss_event.rapB));
				rt_memcpy(((u8_t *)vb->value)+48, (u8_t *)&curloss_data[9], sizeof(curloss_event.pfB));
				rt_memcpy(((u8_t *)vb->value)+52, (u8_t *)&curloss_data[10], sizeof(curloss_event.vC));
				rt_memcpy(((u8_t *)vb->value)+56, (u8_t *)&curloss_data[11], sizeof(curloss_event.iC));
				rt_memcpy(((u8_t *)vb->value)+60, (u8_t *)&curloss_data[12], sizeof(curloss_event.apC));
				rt_memcpy(((u8_t *)vb->value)+64, (u8_t *)&curloss_data[13], sizeof(curloss_event.rapC));
				rt_memcpy(((u8_t *)vb->value)+68, (u8_t *)&curloss_data[14], sizeof(curloss_event.pfC));	
				//rt_memcpy((struct sinkinfo_em_curloss_event_st *)vb->value, &sinkinfo_all_em[portid].si_em_curloss_event_pc[1], SINKINFO_EM_PX_CURLOSS_EVENT_SIZE);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
		}
		break;
		case E_ALR_PC_CURRENT_OVER_EVENT_APPEAR:
		case E_ALR_PC_CURRENT_OVER_EVENT_DISAPPEAR:{
			u32_t curloss_data[32]= {0};
			struct sinkinfo_em_curloss_event_st curloss_event;
			
			vb = snmp_varbind_alloc(&lt300_meter_sn_oid, SNMP_ASN1_OC_STR, 12);
	 		if (vb != NULL){
				get_dev_sn_em_sn(SDT_ELECTRIC_METER, vb->value, DEV_SN_MODE_LEN, portid);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			vb = snmp_varbind_alloc(&lt300_meter_alarm_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			if (FRAME_E_OK != get_event_data_from_ammeter((rt_uint8_t *)amm_sn.em_sn[portid], AMM_EVENT_C_OVER_CURRENT, 1, eventdata, &data_len, RS485_PORT_USED_BY_645)){
				printf_syn("%s(), meter pc cur over event data read fail\n", __FUNCTION__);

			}else{
				curloss_event.start_time[0] = eventdata[5];
				curloss_event.start_time[1] = eventdata[4];
				curloss_event.start_time[2] = eventdata[3];
				curloss_event.start_time[3] = eventdata[2];
				curloss_event.start_time[4] = eventdata[1];
				curloss_event.start_time[5] = eventdata[0];
				curloss_event.end_time[0] = eventdata[11];
				curloss_event.end_time[1] = eventdata[10];
				curloss_event.end_time[2] = eventdata[9];
				curloss_event.end_time[3] = eventdata[8];
				curloss_event.end_time[4] = eventdata[7];
				curloss_event.end_time[5] = eventdata[6];
				curloss_event.vA = eventdata[13]<<8 | eventdata[12];
				curloss_event.iA = eventdata[16]<<16 | eventdata[15]<<8 | eventdata[14];
				curloss_event.apA = eventdata[19]<<16 | eventdata[18]<<8 | eventdata[17];
				curloss_event.rapA = eventdata[22]<<16 | eventdata[21]<<8 | eventdata[20];
				curloss_event.pfA = eventdata[24]<<8 | eventdata[23];
				curloss_event.vB = eventdata[26]<<8 | eventdata[25];
				curloss_event.iB = eventdata[29]<<16 | eventdata[28]<<8 | eventdata[27];
				curloss_event.apB = eventdata[32]<<16 | eventdata[31]<<8 | eventdata[30];
				curloss_event.rapB = eventdata[35]<<16 | eventdata[34]<<8 | eventdata[33];
				curloss_event.pfB = eventdata[37]<<8 | eventdata[36];
				curloss_event.vC = eventdata[39]<<8 | eventdata[38];
				curloss_event.iC = eventdata[42]<<16 | eventdata[41]<<8 | eventdata[40];
				curloss_event.apC = eventdata[45]<<16 | eventdata[44]<<8 | eventdata[43];
				curloss_event.rapC = eventdata[48]<<16 | eventdata[47]<<8 | eventdata[46];
				curloss_event.pfC = eventdata[50]<<8 | eventdata[49];
				
				curloss_data[0] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.vA));
				curloss_data[1] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.iA));
				curloss_data[2] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.apA));
				curloss_data[3] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.rapA));
				curloss_data[4] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.pfA));
				curloss_data[5] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.vB));
				curloss_data[6] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.iB));
				curloss_data[7] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.apB));
				curloss_data[8] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.rapB));
				curloss_data[9] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.pfB));
				curloss_data[10] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.vC));
				curloss_data[11] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.iC));
				curloss_data[12] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.apC));
				curloss_data[13] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.rapC));
				curloss_data[14] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.pfC));
			}
			vb = snmp_varbind_alloc(&lt300_px_curover_trap_oid, SNMP_ASN1_OC_STR, sizeof(curloss_event)-2*2);
	 		if (vb != NULL){
				//rt_memcpy(vb->value, eventdata, data_len);
				rt_memcpy((u8_t *)vb->value, curloss_event.start_time, sizeof(curloss_event.start_time)-2);
				rt_memcpy(((u8_t *)vb->value)+6, curloss_event.end_time, sizeof(curloss_event.end_time)-2);
				rt_memcpy(((u8_t *)vb->value)+12, (u8_t *)&curloss_data[0], sizeof(curloss_event.vA));
				rt_memcpy(((u8_t *)vb->value)+16, (u8_t *)&curloss_data[1], sizeof(curloss_event.iA));
				rt_memcpy(((u8_t *)vb->value)+20, (u8_t *)&curloss_data[2], sizeof(curloss_event.apA));
				rt_memcpy(((u8_t *)vb->value)+24, (u8_t *)&curloss_data[3], sizeof(curloss_event.rapA));
				rt_memcpy(((u8_t *)vb->value)+28, (u8_t *)&curloss_data[4], sizeof(curloss_event.pfA));
				rt_memcpy(((u8_t *)vb->value)+32, (u8_t *)&curloss_data[5], sizeof(curloss_event.vB));
				rt_memcpy(((u8_t *)vb->value)+36, (u8_t *)&curloss_data[6], sizeof(curloss_event.iB));
				rt_memcpy(((u8_t *)vb->value)+40, (u8_t *)&curloss_data[7], sizeof(curloss_event.apB));
				rt_memcpy(((u8_t *)vb->value)+44, (u8_t *)&curloss_data[8], sizeof(curloss_event.rapB));
				rt_memcpy(((u8_t *)vb->value)+48, (u8_t *)&curloss_data[9], sizeof(curloss_event.pfB));
				rt_memcpy(((u8_t *)vb->value)+52, (u8_t *)&curloss_data[10], sizeof(curloss_event.vC));
				rt_memcpy(((u8_t *)vb->value)+56, (u8_t *)&curloss_data[11], sizeof(curloss_event.iC));
				rt_memcpy(((u8_t *)vb->value)+60, (u8_t *)&curloss_data[12], sizeof(curloss_event.apC));
				rt_memcpy(((u8_t *)vb->value)+64, (u8_t *)&curloss_data[13], sizeof(curloss_event.rapC));
				rt_memcpy(((u8_t *)vb->value)+68, (u8_t *)&curloss_data[14], sizeof(curloss_event.pfC));		
				//rt_memcpy((struct sinkinfo_em_curloss_event_st *)vb->value, &sinkinfo_all_em[portid].si_em_curover_event_pc[1], SINKINFO_EM_PX_CURLOSS_EVENT_SIZE);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
		}
		break;
		case E_ALR_PC_CURRENT_BREAK_EVENT_APPEAR:
		case E_ALR_PC_CURRENT_BREAK_EVENT_DISAPPEAR:{
			u32_t curloss_data[32]= {0};
			struct sinkinfo_em_curloss_event_st curloss_event;
			
			vb = snmp_varbind_alloc(&lt300_meter_sn_oid, SNMP_ASN1_OC_STR, 12);
	 		if (vb != NULL){
				get_dev_sn_em_sn(SDT_ELECTRIC_METER, vb->value, DEV_SN_MODE_LEN, portid);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			vb = snmp_varbind_alloc(&lt300_meter_alarm_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			if (FRAME_E_OK != get_event_data_from_ammeter((rt_uint8_t *)amm_sn.em_sn[portid], AMM_EVENT_C_BROKEN_CURRENT, 1, eventdata, &data_len, RS485_PORT_USED_BY_645)){
				printf_syn("%s(), meter pc cur break event data read fail\n", __FUNCTION__);

			}else{
				curloss_event.start_time[0] = eventdata[5];
				curloss_event.start_time[1] = eventdata[4];
				curloss_event.start_time[2] = eventdata[3];
				curloss_event.start_time[3] = eventdata[2];
				curloss_event.start_time[4] = eventdata[1];
				curloss_event.start_time[5] = eventdata[0];
				curloss_event.end_time[0] = eventdata[11];
				curloss_event.end_time[1] = eventdata[10];
				curloss_event.end_time[2] = eventdata[9];
				curloss_event.end_time[3] = eventdata[8];
				curloss_event.end_time[4] = eventdata[7];
				curloss_event.end_time[5] = eventdata[6];
				curloss_event.vA = eventdata[13]<<8 | eventdata[12];
				curloss_event.iA = eventdata[16]<<16 | eventdata[15]<<8 | eventdata[14];
				curloss_event.apA = eventdata[19]<<16 | eventdata[18]<<8 | eventdata[17];
				curloss_event.rapA = eventdata[22]<<16 | eventdata[21]<<8 | eventdata[20];
				curloss_event.pfA = eventdata[24]<<8 | eventdata[23];
				curloss_event.vB = eventdata[26]<<8 | eventdata[25];
				curloss_event.iB = eventdata[29]<<16 | eventdata[28]<<8 | eventdata[27];
				curloss_event.apB = eventdata[32]<<16 | eventdata[31]<<8 | eventdata[30];
				curloss_event.rapB = eventdata[35]<<16 | eventdata[34]<<8 | eventdata[33];
				curloss_event.pfB = eventdata[37]<<8 | eventdata[36];
				curloss_event.vC = eventdata[39]<<8 | eventdata[38];
				curloss_event.iC = eventdata[42]<<16 | eventdata[41]<<8 | eventdata[40];
				curloss_event.apC = eventdata[45]<<16 | eventdata[44]<<8 | eventdata[43];
				curloss_event.rapC = eventdata[48]<<16 | eventdata[47]<<8 | eventdata[46];
				curloss_event.pfC = eventdata[50]<<8 | eventdata[49];
				
				curloss_data[0] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.vA));
				curloss_data[1] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.iA));
				curloss_data[2] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.apA));
				curloss_data[3] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.rapA));
				curloss_data[4] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.pfA));
				curloss_data[5] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.vB));
				curloss_data[6] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.iB));
				curloss_data[7] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.apB));
				curloss_data[8] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.rapB));
				curloss_data[9] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.pfB));
				curloss_data[10] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.vC));
				curloss_data[11] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.iC));
				curloss_data[12] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.apC));
				curloss_data[13] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.rapC));
				curloss_data[14] = lwip_htonl(conv_4byte_bcd_to_long(curloss_event.pfC));
			}
			vb = snmp_varbind_alloc(&lt300_px_curbreak_trap_oid, SNMP_ASN1_OC_STR, sizeof(curloss_event)-2*2);
	 		if (vb != NULL){
				//rt_memcpy(vb->value, eventdata, data_len);
				rt_memcpy((u8_t *)vb->value, curloss_event.start_time, sizeof(curloss_event.start_time)-2);
				rt_memcpy(((u8_t *)vb->value)+6, curloss_event.end_time, sizeof(curloss_event.end_time)-2);
				rt_memcpy(((u8_t *)vb->value)+12, (u8_t *)&curloss_data[0], sizeof(curloss_event.vA));
				rt_memcpy(((u8_t *)vb->value)+16, (u8_t *)&curloss_data[1], sizeof(curloss_event.iA));
				rt_memcpy(((u8_t *)vb->value)+20, (u8_t *)&curloss_data[2], sizeof(curloss_event.apA));
				rt_memcpy(((u8_t *)vb->value)+24, (u8_t *)&curloss_data[3], sizeof(curloss_event.rapA));
				rt_memcpy(((u8_t *)vb->value)+28, (u8_t *)&curloss_data[4], sizeof(curloss_event.pfA));
				rt_memcpy(((u8_t *)vb->value)+32, (u8_t *)&curloss_data[5], sizeof(curloss_event.vB));
				rt_memcpy(((u8_t *)vb->value)+36, (u8_t *)&curloss_data[6], sizeof(curloss_event.iB));
				rt_memcpy(((u8_t *)vb->value)+40, (u8_t *)&curloss_data[7], sizeof(curloss_event.apB));
				rt_memcpy(((u8_t *)vb->value)+44, (u8_t *)&curloss_data[8], sizeof(curloss_event.rapB));
				rt_memcpy(((u8_t *)vb->value)+48, (u8_t *)&curloss_data[9], sizeof(curloss_event.pfB));
				rt_memcpy(((u8_t *)vb->value)+52, (u8_t *)&curloss_data[10], sizeof(curloss_event.vC));
				rt_memcpy(((u8_t *)vb->value)+56, (u8_t *)&curloss_data[11], sizeof(curloss_event.iC));
				rt_memcpy(((u8_t *)vb->value)+60, (u8_t *)&curloss_data[12], sizeof(curloss_event.apC));
				rt_memcpy(((u8_t *)vb->value)+64, (u8_t *)&curloss_data[13], sizeof(curloss_event.rapC));
				rt_memcpy(((u8_t *)vb->value)+68, (u8_t *)&curloss_data[14], sizeof(curloss_event.pfC));				
				//rt_memcpy((struct sinkinfo_em_curloss_event_st *)vb->value, &sinkinfo_all_em[portid].si_em_curbreak_event_pc[1], SINKINFO_EM_PX_CURLOSS_EVENT_SIZE);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
		}
		break;
		case E_ALR_EM_METER_CLEAR_EVENT_APPEAR:
		case E_ALR_EM_METER_CLEAR_EVENT_DISAPPEAR:{
			u32_t emclear_data[32]= {0};
			struct sinkinfo_em_meterclear_event_st meterclear_event;

			vb = snmp_varbind_alloc(&lt300_meter_sn_oid, SNMP_ASN1_OC_STR, 12);
	 		if (vb != NULL){
				get_dev_sn_em_sn(SDT_ELECTRIC_METER, vb->value, DEV_SN_MODE_LEN, portid);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			vb = snmp_varbind_alloc(&lt300_meter_alarm_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			if (FRAME_E_OK != get_event_data_from_ammeter((rt_uint8_t *)amm_sn.em_sn[portid], AMM_EVENT_AMMETER_RESET, 1, eventdata, &data_len, RS485_PORT_USED_BY_645)){
				printf_syn("%s(), meter meter clear event data read fail\n", __FUNCTION__);

			}else{
				meterclear_event.start_time[0] = eventdata[5];
				meterclear_event.start_time[1] = eventdata[4];
				meterclear_event.start_time[2] = eventdata[3];
				meterclear_event.start_time[3] = eventdata[2];
				meterclear_event.start_time[4] = eventdata[1];
				meterclear_event.start_time[5] = eventdata[0];
				meterclear_event.operator_code[0] = eventdata[9];
				meterclear_event.operator_code[1] = eventdata[8];
				meterclear_event.operator_code[2] = eventdata[7];
				meterclear_event.operator_code[3] = eventdata[6];
				meterclear_event.em_act_elec_energy = eventdata[13]<<24 | eventdata[12]<<16 | eventdata[11]<<8 | eventdata[10];
				meterclear_event.em_reverse_act_elec_energy = eventdata[17]<<24 | eventdata[16]<<16 | eventdata[15]<<8 | eventdata[14];
				meterclear_event.em_react_elec_energy_quadrant1 = eventdata[21]<<24 | eventdata[20]<<16 | eventdata[19]<<8 | eventdata[18];
				meterclear_event.em_react_elec_energy_quadrant2 = eventdata[25]<<24 | eventdata[24]<<16 | eventdata[23]<<8 | eventdata[22];
				meterclear_event.em_react_elec_energy_quadrant3 = eventdata[29]<<24 | eventdata[28]<<16 | eventdata[27]<<8 | eventdata[26];
				meterclear_event.em_react_elec_energy_quadrant4 = eventdata[33]<<24 | eventdata[32]<<16 | eventdata[31]<<8 | eventdata[30];
				meterclear_event.pA_act_elec_energy = eventdata[37]<<24 | eventdata[36]<<16 | eventdata[35]<<8 | eventdata[34];
				meterclear_event.pA_reverse_act_elec_energy = eventdata[41]<<24 | eventdata[40]<<16 | eventdata[39]<<8 | eventdata[38];
				meterclear_event.pA_react_elec_energy_quadrant1 = eventdata[45]<<24 | eventdata[44]<<16 | eventdata[43]<<8 | eventdata[42];
				meterclear_event.pA_react_elec_energy_quadrant2 = eventdata[49]<<24 | eventdata[48]<<16 | eventdata[47]<<8 | eventdata[46];
				meterclear_event.pA_react_elec_energy_quadrant3 = eventdata[53]<<24 | eventdata[52]<<16 | eventdata[51]<<8 | eventdata[50];
				meterclear_event.pA_react_elec_energy_quadrant4 = eventdata[57]<<24 | eventdata[56]<<16 | eventdata[55]<<8 | eventdata[54];
				meterclear_event.pB_act_elec_energy = eventdata[61]<<24 | eventdata[60]<<16 | eventdata[59]<<8 | eventdata[58];
				meterclear_event.pB_reverse_act_elec_energy = eventdata[65]<<24 | eventdata[64]<<16 | eventdata[63]<<8 | eventdata[62];
				meterclear_event.pB_react_elec_energy_quadrant1 = eventdata[69]<<24 | eventdata[68]<<16 | eventdata[67]<<8 | eventdata[66];
				meterclear_event.pB_react_elec_energy_quadrant2 = eventdata[73]<<24 | eventdata[72]<<16 | eventdata[71]<<8 | eventdata[70];
				meterclear_event.pB_react_elec_energy_quadrant3 = eventdata[77]<<24 | eventdata[76]<<16 | eventdata[75]<<8 | eventdata[74];
				meterclear_event.pB_react_elec_energy_quadrant4 = eventdata[81]<<24 | eventdata[80]<<16 | eventdata[79]<<8 | eventdata[78];
				meterclear_event.pC_act_elec_energy = eventdata[85]<<24 | eventdata[84]<<16 | eventdata[83]<<8 | eventdata[82];
				meterclear_event.pC_reverse_act_elec_energy = eventdata[89]<<24 | eventdata[88]<<16 | eventdata[87]<<8 | eventdata[86];
				meterclear_event.pC_react_elec_energy_quadrant1 = eventdata[93]<<24 | eventdata[92]<<16 | eventdata[91]<<8 | eventdata[90];
				meterclear_event.pC_react_elec_energy_quadrant2 = eventdata[97]<<24 | eventdata[96]<<16 | eventdata[95]<<8 | eventdata[94];
				meterclear_event.pC_react_elec_energy_quadrant3 = eventdata[101]<<24 | eventdata[100]<<16 | eventdata[99]<<8 | eventdata[98];
				meterclear_event.pC_react_elec_energy_quadrant4 = eventdata[105]<<24 | eventdata[104]<<16 | eventdata[103]<<8 | eventdata[102];
				
				emclear_data[0] = lwip_htonl(conv_4byte_bcd_to_long(meterclear_event.em_act_elec_energy));
				emclear_data[1] = lwip_htonl(conv_4byte_bcd_to_long(meterclear_event.em_reverse_act_elec_energy));
				emclear_data[2] = lwip_htonl(conv_4byte_bcd_to_long(meterclear_event.em_react_elec_energy_quadrant1));
				emclear_data[3] = lwip_htonl(conv_4byte_bcd_to_long(meterclear_event.em_react_elec_energy_quadrant2));
				emclear_data[4] = lwip_htonl(conv_4byte_bcd_to_long(meterclear_event.em_react_elec_energy_quadrant3));
				emclear_data[5] = lwip_htonl(conv_4byte_bcd_to_long(meterclear_event.em_react_elec_energy_quadrant4));
				emclear_data[6] = lwip_htonl(conv_4byte_bcd_to_long(meterclear_event.pA_act_elec_energy));
				emclear_data[7] = lwip_htonl(conv_4byte_bcd_to_long(meterclear_event.pA_reverse_act_elec_energy));
				emclear_data[8] = lwip_htonl(conv_4byte_bcd_to_long(meterclear_event.pA_react_elec_energy_quadrant1));
				emclear_data[9] = lwip_htonl(conv_4byte_bcd_to_long(meterclear_event.pA_react_elec_energy_quadrant2));
				emclear_data[10] = lwip_htonl(conv_4byte_bcd_to_long(meterclear_event.pA_react_elec_energy_quadrant3));
				emclear_data[11] = lwip_htonl(conv_4byte_bcd_to_long(meterclear_event.pA_react_elec_energy_quadrant4));
				emclear_data[12] = lwip_htonl(conv_4byte_bcd_to_long(meterclear_event.pB_act_elec_energy));
				emclear_data[13] = lwip_htonl(conv_4byte_bcd_to_long(meterclear_event.pB_reverse_act_elec_energy));
				emclear_data[14] = lwip_htonl(conv_4byte_bcd_to_long(meterclear_event.pB_react_elec_energy_quadrant1));
				emclear_data[15] = lwip_htonl(conv_4byte_bcd_to_long(meterclear_event.pB_react_elec_energy_quadrant2));
				emclear_data[16] = lwip_htonl(conv_4byte_bcd_to_long(meterclear_event.pB_react_elec_energy_quadrant3));
				emclear_data[17] = lwip_htonl(conv_4byte_bcd_to_long(meterclear_event.pB_react_elec_energy_quadrant4));
				emclear_data[18] = lwip_htonl(conv_4byte_bcd_to_long(meterclear_event.pC_act_elec_energy));
				emclear_data[19] = lwip_htonl(conv_4byte_bcd_to_long(meterclear_event.pC_reverse_act_elec_energy));
				emclear_data[20] = lwip_htonl(conv_4byte_bcd_to_long(meterclear_event.pC_react_elec_energy_quadrant1));
				emclear_data[21] = lwip_htonl(conv_4byte_bcd_to_long(meterclear_event.pC_react_elec_energy_quadrant2));
				emclear_data[22] = lwip_htonl(conv_4byte_bcd_to_long(meterclear_event.pC_react_elec_energy_quadrant3));
				emclear_data[23] = lwip_htonl(conv_4byte_bcd_to_long(meterclear_event.pC_react_elec_energy_quadrant4));

			}
			vb = snmp_varbind_alloc(&lt300_em_meterclear_trap_oid, SNMP_ASN1_OC_STR, sizeof(meterclear_event)-2);
	 		if (vb != NULL){
				//rt_memcpy(vb->value, eventdata, data_len);
				rt_memcpy((u8_t *)vb->value, meterclear_event.start_time, sizeof(meterclear_event.start_time)-2);
				rt_memcpy(((u8_t *)vb->value)+6, meterclear_event.operator_code, sizeof(meterclear_event.operator_code));
				rt_memcpy(((u8_t *)vb->value)+10, (u8_t *)&emclear_data[0], sizeof(meterclear_event.em_act_elec_energy));
				rt_memcpy(((u8_t *)vb->value)+14, (u8_t *)&emclear_data[1], sizeof(meterclear_event.em_reverse_act_elec_energy));
				rt_memcpy(((u8_t *)vb->value)+18, (u8_t *)&emclear_data[2], sizeof(meterclear_event.em_react_elec_energy_quadrant1));
				rt_memcpy(((u8_t *)vb->value)+22, (u8_t *)&emclear_data[3], sizeof(meterclear_event.em_react_elec_energy_quadrant2));
				rt_memcpy(((u8_t *)vb->value)+26, (u8_t *)&emclear_data[4], sizeof(meterclear_event.em_react_elec_energy_quadrant3));
				rt_memcpy(((u8_t *)vb->value)+30, (u8_t *)&emclear_data[5], sizeof(meterclear_event.em_react_elec_energy_quadrant4));
				rt_memcpy(((u8_t *)vb->value)+34, (u8_t *)&emclear_data[6], sizeof(meterclear_event.pA_act_elec_energy));
				rt_memcpy(((u8_t *)vb->value)+38, (u8_t *)&emclear_data[7], sizeof(meterclear_event.pA_reverse_act_elec_energy));
				rt_memcpy(((u8_t *)vb->value)+42, (u8_t *)&emclear_data[8], sizeof(meterclear_event.pA_react_elec_energy_quadrant1));
				rt_memcpy(((u8_t *)vb->value)+46, (u8_t *)&emclear_data[9], sizeof(meterclear_event.pA_react_elec_energy_quadrant2));
				rt_memcpy(((u8_t *)vb->value)+50, (u8_t *)&emclear_data[10], sizeof(meterclear_event.pA_react_elec_energy_quadrant3));
				rt_memcpy(((u8_t *)vb->value)+54, (u8_t *)&emclear_data[11], sizeof(meterclear_event.pA_react_elec_energy_quadrant4));
				rt_memcpy(((u8_t *)vb->value)+58, (u8_t *)&emclear_data[12], sizeof(meterclear_event.pB_act_elec_energy));
				rt_memcpy(((u8_t *)vb->value)+62, (u8_t *)&emclear_data[13], sizeof(meterclear_event.pB_reverse_act_elec_energy));
				rt_memcpy(((u8_t *)vb->value)+66, (u8_t *)&emclear_data[14], sizeof(meterclear_event.pB_react_elec_energy_quadrant1));	
				rt_memcpy(((u8_t *)vb->value)+70, (u8_t *)&emclear_data[15], sizeof(meterclear_event.pB_react_elec_energy_quadrant2));
				rt_memcpy(((u8_t *)vb->value)+74, (u8_t *)&emclear_data[16], sizeof(meterclear_event.pB_react_elec_energy_quadrant3));
				rt_memcpy(((u8_t *)vb->value)+78, (u8_t *)&emclear_data[17], sizeof(meterclear_event.pB_react_elec_energy_quadrant4));
				rt_memcpy(((u8_t *)vb->value)+82, (u8_t *)&emclear_data[18], sizeof(meterclear_event.pC_act_elec_energy));
				rt_memcpy(((u8_t *)vb->value)+86, (u8_t *)&emclear_data[19], sizeof(meterclear_event.pC_reverse_act_elec_energy));
				rt_memcpy(((u8_t *)vb->value)+90, (u8_t *)&emclear_data[20], sizeof(meterclear_event.pC_react_elec_energy_quadrant1));
				rt_memcpy(((u8_t *)vb->value)+94, (u8_t *)&emclear_data[21], sizeof(meterclear_event.pC_react_elec_energy_quadrant2));
				rt_memcpy(((u8_t *)vb->value)+98, (u8_t *)&emclear_data[22], sizeof(meterclear_event.pC_react_elec_energy_quadrant3));
				rt_memcpy(((u8_t *)vb->value)+102, (u8_t *)&emclear_data[23], sizeof(meterclear_event.pC_react_elec_energy_quadrant4));
				//rt_memcpy((struct sinkinfo_em_meterclear_event_st *)vb->value, &sinkinfo_all_em[portid].si_em_meterclear_event[1], SINKINFO_EM_METER_CLEAR_EVENT_SIZE);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
		}
		break;
		case E_ALR_EM_DEMAND_CLEAR_EVENT_APPEAR:
		case E_ALR_EM_DEMAND_CLEAR_EVENT_DISAPPEAR:{
			//u32_t demandclear_data[32]= {0};
			struct sinkinfo_em_demandclear_event_st demandclear_event;
			
			vb = snmp_varbind_alloc(&lt300_meter_sn_oid, SNMP_ASN1_OC_STR, 12);
	 		if (vb != NULL){
				get_dev_sn_em_sn(SDT_ELECTRIC_METER, vb->value, DEV_SN_MODE_LEN, portid);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			vb = snmp_varbind_alloc(&lt300_meter_alarm_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			if (FRAME_E_OK != get_event_data_from_ammeter((rt_uint8_t *)amm_sn.em_sn[portid], AMM_EVENT_REQUIRED_RESET, 1, eventdata, &data_len, RS485_PORT_USED_BY_645)){
				printf_syn("%s(), meter demand clear event data read fail\n", __FUNCTION__);

			}else{
				demandclear_event.start_time[0] = eventdata[5];
				demandclear_event.start_time[1] = eventdata[4];
				demandclear_event.start_time[2] = eventdata[3];
				demandclear_event.start_time[3] = eventdata[2];
				demandclear_event.start_time[4] = eventdata[1];
				demandclear_event.start_time[5] = eventdata[0];
				demandclear_event.operator_code[0] = eventdata[9];
				demandclear_event.operator_code[1] = eventdata[8];
				demandclear_event.operator_code[2] = eventdata[7];
				demandclear_event.operator_code[3] = eventdata[6];
			}
			vb = snmp_varbind_alloc(&lt300_em_demandclear_trap_oid, SNMP_ASN1_OC_STR, sizeof(demandclear_event)-2);
	 		if (vb != NULL){
				//rt_memcpy(vb->value, eventdata, data_len);
				rt_memcpy((u8_t *)vb->value, demandclear_event.start_time, sizeof(demandclear_event.start_time)-2);
				rt_memcpy(((u8_t *)vb->value)+6, demandclear_event.operator_code, sizeof(demandclear_event.operator_code));
				//rt_memcpy((struct sinkinfo_em_demandclear_event_st *)vb->value, &sinkinfo_all_em[portid].si_em_demandclear_event[1], SINKINFO_EM_DEMAND_CLEAR_EVENT_SIZE);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
		}
		break;
		case E_ALR_EM_PROGRAM_EVENT_APPEAR:
		case E_ALR_EM_PROGRAM_EVENT_DISAPPEAR:{
			//u32_t program_data[32]= {0};
			u32_t i,j;
			struct sinkinfo_em_program_event_st program_event;
			
			vb = snmp_varbind_alloc(&lt300_meter_sn_oid, SNMP_ASN1_OC_STR, 12);
	 		if (vb != NULL){
				get_dev_sn_em_sn(SDT_ELECTRIC_METER, vb->value, DEV_SN_MODE_LEN, portid);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			vb = snmp_varbind_alloc(&lt300_meter_alarm_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			if (FRAME_E_OK != get_event_data_from_ammeter((rt_uint8_t *)amm_sn.em_sn[portid], AMM_EVENT_PROGRAM_RECORD, 1, eventdata, &data_len, RS485_PORT_USED_BY_645)){
				printf_syn("%s(), meter program event data read fail\n", __FUNCTION__);

			}else{
				program_event.start_time[0] = eventdata[5];
				program_event.start_time[1] = eventdata[4];
				program_event.start_time[2] = eventdata[3];
				program_event.start_time[3] = eventdata[2];
				program_event.start_time[4] = eventdata[1];
				program_event.start_time[5] = eventdata[0];
				program_event.operator_code[0] = eventdata[9];
				program_event.operator_code[1] = eventdata[8];
				program_event.operator_code[2] = eventdata[7];
				program_event.operator_code[3] = eventdata[6];
				for(i=1;i<11;i++){
					for(j=1;j<5;j++){
						rt_memcpy(program_event.data_flag, &eventdata[10+4*i-j], sizeof(program_event.data_flag));
					}
				}
			}
			vb = snmp_varbind_alloc(&lt300_em_program_trap_oid, SNMP_ASN1_OC_STR, sizeof(program_event)-2);
	 		if (vb != NULL){
				//rt_memcpy(vb->value, eventdata, data_len);
				rt_memcpy((u8_t *)vb->value, program_event.start_time, sizeof(program_event.start_time)-2);
				rt_memcpy(((u8_t *)vb->value)+6, program_event.operator_code, sizeof(program_event.operator_code));
				rt_memcpy(((u8_t *)vb->value)+10, program_event.data_flag, sizeof(program_event.data_flag));
				//rt_memcpy((struct sinkinfo_em_program_event_st *)vb->value, &sinkinfo_all_em[portid].si_em_program_event[1], SINKINFO_EM_PROGRAM_EVENT_SIZE);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
		}
		break;
		case E_ALR_EM_CALIBRATE_TIME_EVENT_APPEAR:
		case E_ALR_EM_CALIBRATE_TIME_EVENT_DISAPPEAR:{
			//u32_t calibratetime_data[32]= {0};
			struct sinkinfo_em_calibratetime_event_st calibratetime_event;
			
			vb = snmp_varbind_alloc(&lt300_meter_sn_oid, SNMP_ASN1_OC_STR, 12);
	 		if (vb != NULL){
				get_dev_sn_em_sn(SDT_ELECTRIC_METER, vb->value, DEV_SN_MODE_LEN, portid);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			vb = snmp_varbind_alloc(&lt300_meter_alarm_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			if (FRAME_E_OK != get_event_data_from_ammeter((rt_uint8_t *)amm_sn.em_sn[portid], AMM_EVENT_TIMING_RECORD, 1, eventdata, &data_len, RS485_PORT_USED_BY_645)){
				printf_syn("%s(), meter calibrate time event data read fail\n", __FUNCTION__);

			}else{
				calibratetime_event.operator_code[0] = eventdata[3];
				calibratetime_event.operator_code[1] = eventdata[2];
				calibratetime_event.operator_code[2] = eventdata[1];
				calibratetime_event.operator_code[3] = eventdata[0];
				calibratetime_event.before_time[0] = eventdata[9];
				calibratetime_event.before_time[1] = eventdata[8];
				calibratetime_event.before_time[2] = eventdata[7];
				calibratetime_event.before_time[3] = eventdata[6];
				calibratetime_event.before_time[4] = eventdata[5];
				calibratetime_event.before_time[5] = eventdata[4];
				calibratetime_event.after_time[0] = eventdata[15];
				calibratetime_event.after_time[1] = eventdata[14];
				calibratetime_event.after_time[2] = eventdata[13];
				calibratetime_event.after_time[3] = eventdata[12];
				calibratetime_event.after_time[4] = eventdata[11];
				calibratetime_event.after_time[5] = eventdata[10];
			}
			vb = snmp_varbind_alloc(&lt300_em_calibratetime_trap_oid, SNMP_ASN1_OC_STR, sizeof(calibratetime_event)-2*2);
	 		if (vb != NULL){
				//rt_memcpy(vb->value, eventdata, data_len);
				rt_memcpy((u8_t *)vb->value, calibratetime_event.operator_code, sizeof(calibratetime_event.operator_code));
				rt_memcpy(((u8_t *)vb->value)+4, calibratetime_event.before_time, sizeof(calibratetime_event.before_time)-2);
				rt_memcpy(((u8_t *)vb->value)+10, calibratetime_event.after_time, sizeof(calibratetime_event.after_time)-2);
				//rt_memcpy((struct sinkinfo_em_calibratetime_event_st *)vb->value, &sinkinfo_all_em[portid].si_em_calibratetime_event[1], SINKINFO_EM_CALIBRATE_TIME_EVENT_SIZE);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
		}
		break;
		case E_ALR_EM_REVERSE_SEQ_VOL_EVENT_APPEAR:
		case E_ALR_EM_REVERSE_SEQ_VOL_EVENT_DISAPPEAR:{
			//u32_t rseqvol_data[32]= {0};
			struct sinkinfo_em_rseqvol_event_st rseqvol_event;
			
			vb = snmp_varbind_alloc(&lt300_meter_sn_oid, SNMP_ASN1_OC_STR, 12);
	 		if (vb != NULL){
				get_dev_sn_em_sn(SDT_ELECTRIC_METER, vb->value, DEV_SN_MODE_LEN, portid);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			vb = snmp_varbind_alloc(&lt300_meter_alarm_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			if (FRAME_E_OK != get_event_data_from_ammeter((rt_uint8_t *)amm_sn.em_sn[portid], AMM_EVENT_VOLTAGE_ANTI_PHASE_RECORD, 1, eventdata, &data_len, RS485_PORT_USED_BY_645)){
				printf_syn("%s(), meter reverse seq vol event data read fail\n", __FUNCTION__);

			}else{
				rseqvol_event.start_time[0] = eventdata[5];
				rseqvol_event.start_time[1] = eventdata[4];
				rseqvol_event.start_time[2] = eventdata[3];
				rseqvol_event.start_time[3] = eventdata[2];
				rseqvol_event.start_time[4] = eventdata[1];
				rseqvol_event.start_time[5] = eventdata[0];
				rseqvol_event.end_time[0] = eventdata[11];
				rseqvol_event.end_time[1] = eventdata[10];
				rseqvol_event.end_time[2] = eventdata[9];
				rseqvol_event.end_time[3] = eventdata[8];
				rseqvol_event.end_time[4] = eventdata[7];
				rseqvol_event.end_time[5] = eventdata[6];

			}
			vb = snmp_varbind_alloc(&lt300_em_rseqvol_trap_oid, SNMP_ASN1_OC_STR, sizeof(rseqvol_event)-2*2);
	 		if (vb != NULL){
				//rt_memcpy(vb->value, eventdata, data_len);
				rt_memcpy((u8_t *)vb->value, rseqvol_event.start_time, sizeof(rseqvol_event.start_time)-2);
				rt_memcpy(((u8_t *)vb->value)+6, rseqvol_event.end_time, sizeof(rseqvol_event.end_time)-2);
				//rt_memcpy((struct sinkinfo_em_rseqvol_event_st *)vb->value, &sinkinfo_all_em[portid].si_em_sreqvol_event[1], SINKINFO_EM_REVERSE_SEQVOL_EVENT_SIZE);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
		}
		break;
		case E_ALR_EM_REVERSE_SEQ_CUR_EVENT_APPEAR:
		case E_ALR_EM_REVERSE_SEQ_CUR_EVENT_DISAPPEAR:{
			//u32_t rseqvol_data[32]= {0};
			struct sinkinfo_em_rseqvol_event_st rseqvol_event;
			
			vb = snmp_varbind_alloc(&lt300_meter_sn_oid, SNMP_ASN1_OC_STR, 12);
	 		if (vb != NULL){
				get_dev_sn_em_sn(SDT_ELECTRIC_METER, vb->value, DEV_SN_MODE_LEN, portid);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			vb = snmp_varbind_alloc(&lt300_meter_alarm_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			if (FRAME_E_OK != get_event_data_from_ammeter((rt_uint8_t *)amm_sn.em_sn[portid], AMM_EVENT_CURRENT_ANTI_PHASE_RECORD, 1, eventdata, &data_len, RS485_PORT_USED_BY_645)){
				printf_syn("%s(), meter reverse seq cur event data read fail\n", __FUNCTION__);

			}else{
				rseqvol_event.start_time[0] = eventdata[5];
				rseqvol_event.start_time[1] = eventdata[4];
				rseqvol_event.start_time[2] = eventdata[3];
				rseqvol_event.start_time[3] = eventdata[2];
				rseqvol_event.start_time[4] = eventdata[1];
				rseqvol_event.start_time[5] = eventdata[0];
				rseqvol_event.end_time[0] = eventdata[11];
				rseqvol_event.end_time[1] = eventdata[10];
				rseqvol_event.end_time[2] = eventdata[9];
				rseqvol_event.end_time[3] = eventdata[8];
				rseqvol_event.end_time[4] = eventdata[7];
				rseqvol_event.end_time[5] = eventdata[6];
			}
			vb = snmp_varbind_alloc(&lt300_em_rseqcur_trap_oid, SNMP_ASN1_OC_STR, sizeof(rseqvol_event)-2*2);
	 		if (vb != NULL){
				//rt_memcpy(vb->value, eventdata, data_len);
				rt_memcpy((u8_t *)vb->value, rseqvol_event.start_time, sizeof(rseqvol_event.start_time)-2);
				rt_memcpy(((u8_t *)vb->value)+6, rseqvol_event.end_time, sizeof(rseqvol_event.end_time)-2);	
				//rt_memcpy((struct sinkinfo_em_rseqvol_event_st *)vb->value, &sinkinfo_all_em[portid].si_em_sreqcur_event[1], SINKINFO_EM_REVERSE_SEQVOL_EVENT_SIZE);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
		}
		break;
		case E_ALR_EM_CONNECT_STATE_CHANGE_EVENT:{
			//u8_t i;
			//u32_t protodata;
			u32_t meterstatus = 0;
		/*	vb = snmp_varbind_alloc(&lt300_meter_sn_oid, SNMP_ASN1_OC_STR, 12);
	 		if (vb != NULL){
				get_dev_sn_em_sn(SDT_ELECTRIC_METER, vb->value, DEV_SN_MODE_LEN, portid);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}*/
			vb = snmp_varbind_alloc(&lt300_meter_alarm_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			/*for (i = 0; i < NUM_OF_COLLECT_EM_MAX; i++) {
				get_em_proto(i, SIC_GET_EM_PROTOCAL_TYPE, &protodata);
				if (protodata != AP_PROTOCOL_UNKNOWN){
					set_bit(meterstatus, 1<<i);
				}
			}*/
			meterstatus = register_em_info.registered_em_vector; 
			vb = snmp_varbind_alloc(&lt300_connect_state_oid, SNMP_ASN1_INTEG, sizeof(meterstatus));
	 		if (vb != NULL){
				*((u32_t *)vb->value) = meterstatus;
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
		}
		break;
		case E_ALR_EM_ADDRESS_CHANGE_EVENT:{
			u32_t meterstatus = 0;
		/*	vb = snmp_varbind_alloc(&lt300_meter_sn_oid, SNMP_ASN1_OC_STR, 12);
	 		if (vb != NULL){
				get_dev_sn_em_sn(SDT_ELECTRIC_METER, vb->value, DEV_SN_MODE_LEN, portid);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}*/
			vb = snmp_varbind_alloc(&lt300_meter_alarm_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			meterstatus = register_em_info.registered_em_vector; 
			vb = snmp_varbind_alloc(&lt300_em_address_oid, SNMP_ASN1_INTEG, sizeof(meterstatus));
	 		if (vb != NULL){
				*((u32_t *)vb->value) = meterstatus;
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
		}
		break;
		case E_ALR_EM_PROTOCOL_BANDRATE_WIREMODE_EVENT:{
			u32_t meterstatus = 0;
		/*	vb = snmp_varbind_alloc(&lt300_meter_sn_oid, SNMP_ASN1_OC_STR, 12);
	 		if (vb != NULL){
				get_dev_sn_em_sn(SDT_ELECTRIC_METER, vb->value, DEV_SN_MODE_LEN, portid);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}*/
			vb = snmp_varbind_alloc(&lt300_meter_alarm_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			meterstatus = register_em_info.registered_em_vector; 
			vb = snmp_varbind_alloc(&lt300_em_protocol_wiremode_baudrate_oid, SNMP_ASN1_INTEG, sizeof(meterstatus));
	 		if (vb != NULL){
				*((u32_t *)vb->value) = meterstatus;
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
		}
		break;
		case E_ALR_EM_MOMENTARY_FREEZE_DATA:{
			struct sinkinfo_em_momentary_freeze_st mfz_ptr;
			u32_t freezedata[32]= {0};
			
			vb = snmp_varbind_alloc(&lt300_meter_sn_oid, SNMP_ASN1_OC_STR, 12);
	 		if (vb != NULL){
				get_dev_sn_em_sn(SDT_ELECTRIC_METER, vb->value, DEV_SN_MODE_LEN, portid);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			vb = snmp_varbind_alloc(&lt300_meter_alarm_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			rt_memcpy(&mfz_ptr, &sinkinfo_all_em[portid].si_em_mom_freeze[0], SINKINFO_EM_MOMENT_FREEZE_DATA_SIZE);
			//if(SIE_OK == get_sinkinfo_use_by_mib(portid,  0, SI_MGC_GET_EM_MOMENT_FREEZE_INFO, &mfz_ptr, SINKINFO_EM_MOMENT_FREEZE_DATA_SIZE)){
				if(mfz_ptr.act_elec_energy == INVLIDE_DATAL){
					freezedata[0] = lwip_htonl(mfz_ptr.act_elec_energy);
				}else{
					freezedata[0] = lwip_htonl(conv_4byte_bcd_to_long(mfz_ptr.act_elec_energy));
				}
				if(mfz_ptr.reverse_act_elec_energy == INVLIDE_DATAL){
					freezedata[1] = lwip_htonl(mfz_ptr.reverse_act_elec_energy);
				}else{
					freezedata[1] = lwip_htonl(conv_4byte_bcd_to_long(mfz_ptr.reverse_act_elec_energy));
				}
				if(mfz_ptr.apxT == INVLIDE_DATAL){
					freezedata[2] = lwip_htonl(mfz_ptr.apxT);
				}else{
					freezedata[2] = lwip_htonl(conv_4byte_bcd_to_long(mfz_ptr.apxT));
				}
				if(mfz_ptr.apxA == INVLIDE_DATAL){
					freezedata[3] = lwip_htonl(mfz_ptr.apxA);
				}else{
					freezedata[3] = lwip_htonl(conv_4byte_bcd_to_long(mfz_ptr.apxA));
				}
				if(mfz_ptr.apxB == INVLIDE_DATAL){
					freezedata[4] = lwip_htonl(mfz_ptr.apxB);
				}else{
					freezedata[4] = lwip_htonl(conv_4byte_bcd_to_long(mfz_ptr.apxB));
				}
				if(mfz_ptr.apxC == INVLIDE_DATAL){
					freezedata[5] = lwip_htonl(mfz_ptr.apxC);
				}else{
					freezedata[5] = lwip_htonl(conv_4byte_bcd_to_long(mfz_ptr.apxC));
				}
				if(mfz_ptr.rapxT == INVLIDE_DATAL){
					freezedata[6] = lwip_htonl(mfz_ptr.rapxT);
				}else{
					freezedata[6] = lwip_htonl(conv_4byte_bcd_to_long(mfz_ptr.rapxT));
				}
				if(mfz_ptr.rapxA == INVLIDE_DATAL){
					freezedata[7] = lwip_htonl(mfz_ptr.rapxA);
				}else{
					freezedata[7] = lwip_htonl(conv_4byte_bcd_to_long(mfz_ptr.rapxA));
				}
				if(mfz_ptr.rapxB == INVLIDE_DATAL){
					freezedata[8] = lwip_htonl(mfz_ptr.rapxB);
				}else{
					freezedata[8] = lwip_htonl(conv_4byte_bcd_to_long(mfz_ptr.rapxB));
				}
				if(mfz_ptr.rapxC == INVLIDE_DATAL){
					freezedata[9] = lwip_htonl(mfz_ptr.rapxC);
				}else{
					freezedata[9] = lwip_htonl(conv_4byte_bcd_to_long(mfz_ptr.rapxC));
				}
				if(mfz_ptr.act_max_demand == INVLIDE_DATAL){
					freezedata[10] = lwip_htonl(mfz_ptr.act_max_demand);
				}else{
					freezedata[10] = lwip_htonl(conv_4byte_bcd_to_long(mfz_ptr.act_max_demand));
				}
				if(mfz_ptr.reverse_act_max_demand == INVLIDE_DATAL){
					freezedata[11] = lwip_htonl(mfz_ptr.reverse_act_max_demand);
				}else{
					freezedata[11] = lwip_htonl(conv_4byte_bcd_to_long(mfz_ptr.reverse_act_max_demand));
				}
			
			//}
			vb = snmp_varbind_alloc(&lt300_em_rseqcur_trap_oid, SNMP_ASN1_OC_STR, (SINKINFO_EM_MOMENT_FREEZE_DATA_SIZE - 3*3));
	 		if (vb != NULL){
				rt_memcpy((u8_t *)vb->value, mfz_ptr.freeze_time, sizeof(mfz_ptr.freeze_time)-3);
				rt_memcpy(((u8_t *)vb->value)+5, (u8_t *)&freezedata[0], sizeof(mfz_ptr.act_elec_energy));
				rt_memcpy(((u8_t *)vb->value)+9, (u8_t *)&freezedata[1], sizeof(mfz_ptr.reverse_act_elec_energy));
				rt_memcpy(((u8_t *)vb->value)+13, (u8_t *)&freezedata[2], sizeof(mfz_ptr.apxT));
				rt_memcpy(((u8_t *)vb->value)+17, (u8_t *)&freezedata[3], sizeof(mfz_ptr.apxA));
				rt_memcpy(((u8_t *)vb->value)+21, (u8_t *)&freezedata[4], sizeof(mfz_ptr.apxB));
				rt_memcpy(((u8_t *)vb->value)+25, (u8_t *)&freezedata[5], sizeof(mfz_ptr.apxC));
				rt_memcpy(((u8_t *)vb->value)+29, (u8_t *)&freezedata[6], sizeof(mfz_ptr.rapxT));
				rt_memcpy(((u8_t *)vb->value)+33, (u8_t *)&freezedata[7], sizeof(mfz_ptr.rapxA));
				rt_memcpy(((u8_t *)vb->value)+37, (u8_t *)&freezedata[8], sizeof(mfz_ptr.rapxB));
				rt_memcpy(((u8_t *)vb->value)+41, (u8_t *)&freezedata[9], sizeof(mfz_ptr.rapxC));
				rt_memcpy(((u8_t *)vb->value)+45, (u8_t *)&freezedata[10], sizeof(mfz_ptr.act_max_demand));
				rt_memcpy(((u8_t *)vb->value)+49, mfz_ptr.act_max_demand_time, sizeof(mfz_ptr.act_max_demand_time)-3);
				rt_memcpy(((u8_t *)vb->value)+54, (u8_t *)&freezedata[11], sizeof(mfz_ptr.reverse_act_max_demand));
				rt_memcpy(((u8_t *)vb->value)+58, mfz_ptr.reverse_act_max_demand_time, sizeof(mfz_ptr.reverse_act_max_demand_time)-3);
				//rt_memcpy((struct sinkinfo_em_rseqvol_event_st *)vb->value, &sinkinfo_all_em[portid].si_em_sreqcur_event[1], SINKINFO_EM_REVERSE_SEQVOL_EVENT_SIZE);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			
		}
		break;
		case E_ALR_EM_TIMING_FREEZE_DATA:{
			struct sinkinfo_em_timing_freeze_st tfz_ptr;
			u32_t freezedata[32]= {0};
			
			vb = snmp_varbind_alloc(&lt300_meter_sn_oid, SNMP_ASN1_OC_STR, 12);
	 		if (vb != NULL){
				get_dev_sn_em_sn(SDT_ELECTRIC_METER, vb->value, DEV_SN_MODE_LEN, portid);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			vb = snmp_varbind_alloc(&lt300_meter_alarm_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			rt_memcpy(&tfz_ptr, &sinkinfo_all_em[portid].si_em_time_freeze[0], SINKINFO_EM_TIMING_FREEZE_DATA_SIZE);
			//if(SIE_OK == get_sinkinfo_use_by_mib(portid,  0, SI_MGC_GET_EM_TIMING_FREEZE_INFO, &tfz_ptr, SINKINFO_EM_TIMING_FREEZE_DATA_SIZE)){
				if(tfz_ptr.act_elec_energy == INVLIDE_DATAL){
					freezedata[0] = lwip_htonl(tfz_ptr.act_elec_energy);
				}else{
					freezedata[0] = lwip_htonl(conv_4byte_bcd_to_long(tfz_ptr.act_elec_energy));
				}
				if(tfz_ptr.reverse_act_elec_energy == INVLIDE_DATAL){
					freezedata[1] = lwip_htonl(tfz_ptr.reverse_act_elec_energy);
				}else{
					freezedata[1] = lwip_htonl(conv_4byte_bcd_to_long(tfz_ptr.reverse_act_elec_energy));
				}
				if(tfz_ptr.apxT == INVLIDE_DATAL){
					freezedata[2] = lwip_htonl(tfz_ptr.apxT);
				}else{
					freezedata[2] = lwip_htonl(conv_4byte_bcd_to_long(tfz_ptr.apxT));
				}
				if(tfz_ptr.apxA == INVLIDE_DATAL){
					freezedata[3] = lwip_htonl(tfz_ptr.apxA);
				}else{
					freezedata[3] = lwip_htonl(conv_4byte_bcd_to_long(tfz_ptr.apxA));
				}
				if(tfz_ptr.apxB == INVLIDE_DATAL){
					freezedata[4] = lwip_htonl(tfz_ptr.apxB);
				}else{
					freezedata[4] = lwip_htonl(conv_4byte_bcd_to_long(tfz_ptr.apxB));
				}
				if(tfz_ptr.apxC == INVLIDE_DATAL){
					freezedata[5] = lwip_htonl(tfz_ptr.apxC);
				}else{
					freezedata[5] = lwip_htonl(conv_4byte_bcd_to_long(tfz_ptr.apxC));
				}
				if(tfz_ptr.rapxT == INVLIDE_DATAL){
					freezedata[6] = lwip_htonl(tfz_ptr.rapxT);
				}else{
					freezedata[6] = lwip_htonl(conv_4byte_bcd_to_long(tfz_ptr.rapxT));
				}
				if(tfz_ptr.rapxA == INVLIDE_DATAL){
					freezedata[7] = lwip_htonl(tfz_ptr.rapxA);
				}else{
					freezedata[7] = lwip_htonl(conv_4byte_bcd_to_long(tfz_ptr.rapxA));
				}
				if(tfz_ptr.rapxB == INVLIDE_DATAL){
					freezedata[8] = lwip_htonl(tfz_ptr.rapxB);
				}else{
					freezedata[8] = lwip_htonl(conv_4byte_bcd_to_long(tfz_ptr.rapxB));
				}
				if(tfz_ptr.rapxC == INVLIDE_DATAL){
					freezedata[9] = lwip_htonl(tfz_ptr.rapxC);
				}else{
					freezedata[9] = lwip_htonl(conv_4byte_bcd_to_long(tfz_ptr.rapxC));
				}
				if(tfz_ptr.act_max_demand == INVLIDE_DATAL){
					freezedata[10] = lwip_htonl(tfz_ptr.act_max_demand);
				}else{
					freezedata[10] = lwip_htonl(conv_4byte_bcd_to_long(tfz_ptr.act_max_demand));
				}
				if(tfz_ptr.reverse_act_max_demand == INVLIDE_DATAL){
					freezedata[11] = lwip_htonl(tfz_ptr.reverse_act_max_demand);
				}else{
					freezedata[11] = lwip_htonl(conv_4byte_bcd_to_long(tfz_ptr.reverse_act_max_demand));
				}

			//}			
			vb = snmp_varbind_alloc(&lt300_em_rseqcur_trap_oid, SNMP_ASN1_OC_STR, (SINKINFO_EM_TIMING_FREEZE_DATA_SIZE - 3*3));
	 		if (vb != NULL){
				rt_memcpy((u8_t *)vb->value, tfz_ptr.freeze_time, sizeof(tfz_ptr.freeze_time)-3);
				rt_memcpy(((u8_t *)vb->value)+5, (u8_t *)&freezedata[0], sizeof(tfz_ptr.act_elec_energy));
				rt_memcpy(((u8_t *)vb->value)+9, (u8_t *)&freezedata[1], sizeof(tfz_ptr.reverse_act_elec_energy));
				rt_memcpy(((u8_t *)vb->value)+13, (u8_t *)&freezedata[2], sizeof(tfz_ptr.apxT));
				rt_memcpy(((u8_t *)vb->value)+17, (u8_t *)&freezedata[3], sizeof(tfz_ptr.apxA));
				rt_memcpy(((u8_t *)vb->value)+21, (u8_t *)&freezedata[4], sizeof(tfz_ptr.apxB));
				rt_memcpy(((u8_t *)vb->value)+25, (u8_t *)&freezedata[5], sizeof(tfz_ptr.apxC));
				rt_memcpy(((u8_t *)vb->value)+29, (u8_t *)&freezedata[6], sizeof(tfz_ptr.rapxT));
				rt_memcpy(((u8_t *)vb->value)+33, (u8_t *)&freezedata[7], sizeof(tfz_ptr.rapxA));
				rt_memcpy(((u8_t *)vb->value)+37, (u8_t *)&freezedata[8], sizeof(tfz_ptr.rapxB));
				rt_memcpy(((u8_t *)vb->value)+41, (u8_t *)&freezedata[9], sizeof(tfz_ptr.rapxC));
				rt_memcpy(((u8_t *)vb->value)+45, (u8_t *)&freezedata[10], sizeof(tfz_ptr.act_max_demand));
				rt_memcpy(((u8_t *)vb->value)+49, tfz_ptr.act_max_demand_time, sizeof(tfz_ptr.act_max_demand_time)-3);
				rt_memcpy(((u8_t *)vb->value)+54, (u8_t *)&freezedata[11], sizeof(tfz_ptr.reverse_act_max_demand));
				rt_memcpy(((u8_t *)vb->value)+58, tfz_ptr.reverse_act_max_demand_time, sizeof(tfz_ptr.reverse_act_max_demand_time)-3);				
				//rt_memcpy((struct sinkinfo_em_rseqvol_event_st *)vb->value, &sinkinfo_all_em[portid].si_em_sreqcur_event[1], SINKINFO_EM_REVERSE_SEQVOL_EVENT_SIZE);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			
		}
		break;
		case E_ALR_EM_ACT_PULSE_TIME_OUT:{
			vb = snmp_varbind_alloc(&lt300_meter_sn_oid, SNMP_ASN1_OC_STR, 12);
	 		if (vb != NULL){
				get_dev_sn_em_sn(SDT_ELECTRIC_METER, vb->value, DEV_SN_MODE_LEN, portid);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			vb = snmp_varbind_alloc(&lt300_meter_alarm_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			vb = snmp_varbind_alloc(&lt300_act_pulse_time_out_oid, SNMP_ASN1_INTEG, sizeof(u32_t));
	 		if (vb != NULL){
				*((u32_t *)(vb->value))= check_eenergy_state;
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
		}
		break;
		case E_ALR_EM_REACT_PULSE_TIME_OUT:{
			vb = snmp_varbind_alloc(&lt300_meter_sn_oid, SNMP_ASN1_OC_STR, 12);
	 		if (vb != NULL){
				get_dev_sn_em_sn(SDT_ELECTRIC_METER, vb->value, DEV_SN_MODE_LEN, portid);
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			vb = snmp_varbind_alloc(&lt300_meter_alarm_oid, SNMP_ASN1_OPAQUE, rt_strlen(message));
	 		if (vb != NULL){
				rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
			vb = snmp_varbind_alloc(&lt300_react_pulse_time_out_oid, SNMP_ASN1_INTEG, sizeof(u32_t));
	 		if (vb != NULL){
				//rt_memcpy ((char *)vb->value, message, rt_strlen(message));
				*((u32_t *)(vb->value))= check_eenergy_state;
				snmp_varbind_tail_add(&vb_list->vb_root,vb);
	 		}
		}
		break;

	}
	trap_msg.outvb = vb_list->vb_root;
	if(ERR_OK == snmp_send_trap(SNMP_GENTRAP_ENTERPRISESPC,vb_list->ent_oid,vb_list->spc_trap))
		printf_syn("trap send success\n");
	free_private_trap_list(vb_list);
	//tcpip_callback(send_trap_callback, vb_list);

	return RT_EOK;
}
FINSH_FUNCTION_EXPORT(trap_send, "portid:1-31, gtrap:0-6, strap:1-6, alarmcode:alarm type, Flag:dev type");

int connect_state_trap()  
{
	u8_t i;
	u32_t protodata;
	u32_t oldstatus = 0;
	u32_t newstatus = 0;

	for (i = 0; i < NUM_OF_COLLECT_EM_MAX; i++) {
		get_em_proto(i, SIC_GET_EM_PROTOCAL_TYPE, &protodata);
		if (protodata != AP_PROTOCOL_UNKNOWN){
			set_bit(oldstatus, 1<<i);
		}
	}

	if(RT_EOK != try_get_em_protoco_baud_info()) {
		printf_syn("ERROR: try_get_em_protoco_baud_info() \n");
		return RT_ERROR;
	}
	
	for (i = 0; i < NUM_OF_COLLECT_EM_MAX; i++) {
		get_em_proto(i, SIC_GET_EM_PROTOCAL_TYPE, &protodata);
		if (protodata != AP_PROTOCOL_UNKNOWN){
			set_bit(newstatus, 1<<i);
		}
	}

    if(oldstatus != newstatus)
		trap_send(1,SNMP_GENTRAP_ENTERPRISESPC, E_ALR_EM_CONNECT_STATE_CHANGE_EVENT, E_ALR_EM_CONNECT_STATE_CHANGE_EVENT, SDT_DEV);

	return RT_EOK;

} 

int timing_freeze_data_trap(rt_uint8_t portid)  
{
	u8_t i;
	u32_t freezetype;
	struct electric_meter_reg_info_st amm_sn;
	rt_uint32_t data_len = 0;
	rt_uint8_t meteraddr[16];
	u32_t freezedata[12]= {0};
	u8_t chardata[12]= {0};
	u8_t freeze_time[12]= {0};
	u8_t  year,month,day,hour,minite;
	u8_t  em_year,em_month,em_day,em_hour,em_minite;
	struct ammeter_time time;
	//struct tm curtime;
#if 0
	curtime = Time_GetCalendarTime();
	year = curtime.tm_year;
	month = curtime.tm_mon;
	day = curtime.tm_mday;
	hour = curtime.tm_hour;
	minite = curtime.tm_min;
#endif
	if (SUCC == get_em_reg_info(&amm_sn)) {
		rt_memcpy(meteraddr, amm_sn.em_sn[portid], DEV_SN_MODE_LEN);
	} else {
		printf_syn("%s(), read meter addr data tbl fail\n", __FUNCTION__);
	}

	if (SUCC == get_em_info(portid, 6, freezedata, chardata)){
		time.month = (chardata[0]-0x30)*10 + (chardata[1]-0x30);
		time.day = (chardata[2]-0x30)*10 + (chardata[3]-0x30);
		time.hour = (chardata[4]-0x30)*10 + (chardata[5]-0x30);
		time.minite = (chardata[6]-0x30)*10 + (chardata[7]-0x30);
	}

	if(FRAME_E_OK == get_power_data_from_ammeter(meteraddr, AC_DATE_AND_WEEK,
			freeze_time, &data_len, RS485_PORT_USED_BY_645)){
	
		em_year = conv_4byte_bcd_to_long(freeze_time[3]);
		em_month = conv_4byte_bcd_to_long(freeze_time[2]);
		em_day = conv_4byte_bcd_to_long(freeze_time[1]);
	
	} else {
		printf_syn("%s(), read EM date and week data fail\n", __FUNCTION__);
	}
	rt_thread_delay(get_ticks_of_ms(50));

	if(FRAME_E_OK == get_power_data_from_ammeter(meteraddr, AC_AMMETER_TIME,
			&freeze_time[4], &data_len, RS485_PORT_USED_BY_645)){
	
		em_hour = conv_4byte_bcd_to_long(freeze_time[6]);
		em_minite = conv_4byte_bcd_to_long(freeze_time[5]);
	
	} else {
		printf_syn("%s(), read EM time data fail\n", __FUNCTION__);
	}
	rt_thread_delay(get_ticks_of_ms(50));

	if (FRAME_E_OK == get_forzen_data_from_ammeter(meteraddr, CMD_FORZEN_TIMEING_TIME, 1, freeze_time, &data_len, RS485_PORT_USED_BY_645)){
		year = conv_4byte_bcd_to_long(freeze_time[4]);
		month = conv_4byte_bcd_to_long(freeze_time[3]);
		day = conv_4byte_bcd_to_long(freeze_time[2]);
		hour = conv_4byte_bcd_to_long(freeze_time[1]);
		minite = conv_4byte_bcd_to_long(freeze_time[0]);
	}else {
		printf_syn("%s(), read EM freeze time data fail\n", __FUNCTION__);
	}

	if(SUCC == get_em_info(portid, 7, freezedata, chardata)){
			freezetype = freezedata[0];
	}

	switch (freezetype) {
	case CMD_FREEZING_MONTH:	/* period = month */
		if((day == time.day) && (hour == time.hour) && (minite == time.minite)){
			if((day%(maxday_of_month(month, year))== 0)&&(em_day%(maxday_of_month(em_month, em_year))== 1)){
				if((hour == em_hour) && (minite == em_minite))
					for(i=0;i<3;i++){   /* try to trap tree times */
						trap_send(1,SNMP_GENTRAP_ENTERPRISESPC, E_ALR_EM_TIMING_FREEZE_DATA, E_ALR_EM_TIMING_FREEZE_DATA, SDT_DEV);
				    	rt_thread_delay(get_ticks_of_ms(50));
					}	
			}else if(day%(maxday_of_month(month, year))>0){
				if((month == em_month) && ((day+1) == em_day) && (hour == em_hour) && (minite == em_minite))
					for(i=0;i<3;i++){	/* try to trap tree times */
						trap_send(1,SNMP_GENTRAP_ENTERPRISESPC, E_ALR_EM_TIMING_FREEZE_DATA, E_ALR_EM_TIMING_FREEZE_DATA, SDT_DEV);
						rt_thread_delay(get_ticks_of_ms(50));
					}	
			}
		}
		break;

	case CMD_FREEZING_DAY:		/* period = day  */
		if((hour == time.hour) && (minite == time.minite)){
			if(((hour%24+1)== (em_hour%24))&& (minite == em_minite)){
				for(i=0;i<3;i++){	/* try to trap tree times */
					trap_send(1,SNMP_GENTRAP_ENTERPRISESPC, E_ALR_EM_TIMING_FREEZE_DATA, E_ALR_EM_TIMING_FREEZE_DATA, SDT_DEV);
					rt_thread_delay(get_ticks_of_ms(50));
				}
			}
		}
		break;

	case CMD_FREEZING_HOUR:		/* period = minite  */
		if(minite == time.minite){
			if((minite%60+3)== (em_minite%24)){
				for(i=0;i<3;i++){	/* try to trap tree times */
					trap_send(1,SNMP_GENTRAP_ENTERPRISESPC, E_ALR_EM_TIMING_FREEZE_DATA, E_ALR_EM_TIMING_FREEZE_DATA, SDT_DEV);
					rt_thread_delay(get_ticks_of_ms(50));
				}
			}
		}
		break;

	default:
		trap_send(1,SNMP_GENTRAP_ENTERPRISESPC, E_ALR_EM_MOMENTARY_FREEZE_DATA, E_ALR_EM_MOMENTARY_FREEZE_DATA, SDT_DEV);
		printf_syn("%s() not timing freeze type\n", __FUNCTION__);
		break;
	}

	return RT_EOK;

} 

void trap_test_init()  
{
	//鍒濆鍖栫數鑳借〃

	 rt_thread_t trap_thread ;  
       
    // 鍒涘缓浠诲姟绾跨▼   
 	trap_thread = rt_thread_create("trap",  
 		snmp_trap_entry, RT_NULL,  
		1024, 30, 10);  
    // 鍚姩浠诲姟绾跨▼   
	 if (trap_thread != RT_NULL)  
		rt_thread_startup(trap_thread);  

} 
#endif//LWIP_SNMP TRAP
