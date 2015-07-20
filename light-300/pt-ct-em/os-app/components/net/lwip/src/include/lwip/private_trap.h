#ifndef _LWIP_PRIVATE_TRAP_H_
#define _LWIP_PRIVATE_TRAP_H_

//#define SNMP_SEND_TRAP 		1

#define MY_SNMP_SYSOBJID_LEN 7
#define MY_SNMP_ENTERPRISE_ID 40409
#define MY_SNMP_SYSOBJID {1, 3, 6, 1, 4, 1, MY_SNMP_ENTERPRISE_ID}

#define		FLAG_PT			1
#define		FLAG_CT			2
#define		FLAG_EM			3

#define 	STRAP_wl_Status 1
#define 	STRAP_ct_power_Status 2
#define 	STRAP_EMStatus 3
#define 	STRAP_485Status 4
#define 	STRAP_alarm 5

#define 	lt300_SYS_ALARM_MAX_LEN 64

typedef struct EnumString_s {
	rt_uint32_t value;
	char *string;
}EnumString;

typedef enum{
	E_PrtSAlrState_off 	= 2,
	E_PrtSAlrState_on 	= 3,
}EnumPrtSAlrStateField;

typedef enum {
	E_ALR_WL_SLAVE_RESPONSE_FAIL				= 1,
	E_ALR_WL_SLAVE_RESPONSE_CHANGE_TO_NORMAL	= 2,
	E_ALR_WL_MASTER_RESPONSE_FAIL 				= 3,
	E_ALR_WL_MASTER_RESPONSE_CHANGE_TO_NORMAL	= 4,
	E_ALR_CT_POWER_LOSS	 						= 5,
	E_ALR_CT_POWER_CHANGE_TO_NORMAL				= 6,
	
	E_ALR_PA_VOLTAGE_LOSS_EVENT_APPEAR,
	E_ALR_PA_VOLTAGE_LOSS_EVENT_DISAPPEAR,
	E_ALR_PA_VOLTAGE_OVER_EVENT_APPEAR,
	E_ALR_PA_VOLTAGE_OVER_EVENT_DISAPPEAR,
	E_ALR_PA_VOLTAGE_UNDER_EVENT_APPEAR,
	E_ALR_PA_VOLTAGE_UNDER_EVENT_DISAPPEAR,
	E_ALR_PA_PHASE_BREAK_EVENT_APPEAR,
	E_ALR_PA_PHASE_BREAK_EVENT_DISAPPEAR,
	E_ALR_PA_CURRENT_LOSS_EVENT_APPEAR,
	E_ALR_PA_CURRENT_LOSS_EVENT_DISAPPEAR,
	E_ALR_PA_CURRENT_OVER_EVENT_APPEAR,
	E_ALR_PA_CURRENT_OVER_EVENT_DISAPPEAR,
	E_ALR_PA_CURRENT_BREAK_EVENT_APPEAR,
	E_ALR_PA_CURRENT_BREAK_EVENT_DISAPPEAR,

	E_ALR_PB_VOLTAGE_LOSS_EVENT_APPEAR,
	E_ALR_PB_VOLTAGE_LOSS_EVENT_DISAPPEAR,
	E_ALR_PB_VOLTAGE_OVER_EVENT_APPEAR,
	E_ALR_PB_VOLTAGE_OVER_EVENT_DISAPPEAR,
	E_ALR_PB_VOLTAGE_UNDER_EVENT_APPEAR,
	E_ALR_PB_VOLTAGE_UNDER_EVENT_DISAPPEAR,
	E_ALR_PB_PHASE_BREAK_EVENT_APPEAR,
	E_ALR_PB_PHASE_BREAK_EVENT_DISAPPEAR,
	E_ALR_PB_CURRENT_LOSS_EVENT_APPEAR,
	E_ALR_PB_CURRENT_LOSS_EVENT_DISAPPEAR,
	E_ALR_PB_CURRENT_OVER_EVENT_APPEAR,
	E_ALR_PB_CURRENT_OVER_EVENT_DISAPPEAR,
	E_ALR_PB_CURRENT_BREAK_EVENT_APPEAR,
	E_ALR_PB_CURRENT_BREAK_EVENT_DISAPPEAR,

	E_ALR_PC_VOLTAGE_LOSS_EVENT_APPEAR,
	E_ALR_PC_VOLTAGE_LOSS_EVENT_DISAPPEAR,
	E_ALR_PC_VOLTAGE_OVER_EVENT_APPEAR,
	E_ALR_PC_VOLTAGE_OVER_EVENT_DISAPPEAR,
	E_ALR_PC_VOLTAGE_UNDER_EVENT_APPEAR,
	E_ALR_PC_VOLTAGE_UNDER_EVENT_DISAPPEAR,
	E_ALR_PC_PHASE_BREAK_EVENT_APPEAR,
	E_ALR_PC_PHASE_BREAK_EVENT_DISAPPEAR,
	E_ALR_PC_CURRENT_LOSS_EVENT_APPEAR,
	E_ALR_PC_CURRENT_LOSS_EVENT_DISAPPEAR,
	E_ALR_PC_CURRENT_OVER_EVENT_APPEAR,
	E_ALR_PC_CURRENT_OVER_EVENT_DISAPPEAR,
	E_ALR_PC_CURRENT_BREAK_EVENT_APPEAR,
	E_ALR_PC_CURRENT_BREAK_EVENT_DISAPPEAR,

	E_ALR_EM_METER_CLEAR_EVENT_APPEAR,
	E_ALR_EM_METER_CLEAR_EVENT_DISAPPEAR,
	E_ALR_EM_DEMAND_CLEAR_EVENT_APPEAR,
	E_ALR_EM_DEMAND_CLEAR_EVENT_DISAPPEAR,
	E_ALR_EM_PROGRAM_EVENT_APPEAR,
	E_ALR_EM_PROGRAM_EVENT_DISAPPEAR,
	E_ALR_EM_CALIBRATE_TIME_EVENT_APPEAR,
	E_ALR_EM_CALIBRATE_TIME_EVENT_DISAPPEAR,
	E_ALR_EM_REVERSE_SEQ_VOL_EVENT_APPEAR,
	E_ALR_EM_REVERSE_SEQ_VOL_EVENT_DISAPPEAR,
	E_ALR_EM_REVERSE_SEQ_CUR_EVENT_APPEAR,
	E_ALR_EM_REVERSE_SEQ_CUR_EVENT_DISAPPEAR,

	E_ALR_EM_CONNECT_STATE_CHANGE_EVENT,
	E_ALR_EM_PROTOCOL_BANDRATE_WIREMODE_EVENT,
	E_ALR_EM_ADDRESS_CHANGE_EVENT,
	E_ALR_EM_MOMENTARY_FREEZE_DATA,
	E_ALR_EM_TIMING_FREEZE_DATA,
	E_ALR_EM_ACT_PULSE_TIME_OUT,
	E_ALR_EM_REACT_PULSE_TIME_OUT,
}EnumAlrStringField;

typedef struct
{
	rt_uint32_t	trapIp ;
	rt_uint8_t	snmpVersion ;
	rt_uint32_t	trapPort ;
	char		trapComunity[SNMP_COMMUNITY_LEN_MAX] ;
	rt_uint8_t	valid ;
}lt300_TrapInfo_t ;

struct trap_list
{
	struct snmp_varbind_root vb_root;
	struct snmp_obj_id *ent_oid;
	s32_t spc_trap;
	u8_t in_use;   
};
 
#if 0

struct private_trap
{
  /* source enterprise ID (sysObjectID) */
  struct snmp_obj_id *enterprise;  
  /* trap ID */
  struct snmp_obj_id *trap_oid;
  
  /* specific trap code */
  u32_t spc_trap;
  
  /* object value ASN1 type */
  u8_t value_type;
  /* object value length (in u8_t) */
  u8_t value_len;
  /* object value */
  void *value;
  /* indicate that the trap is sent */
  u8_t in_use;
};

typedef struct snmp_message
{
   uint32_t                 		COMMAND;
   struct ip_addr			ADDRESS;
   unsigned char			TRAP;
   char 					*message;
} SNMP_MESSAGE, *SNMP_MESSAGE_PTR;

extern void snmp_thread_entry(void *arg);

extern void send_trap(struct ip_addr addr, unsigned char trap, char *message);
#endif
void snmp_trap_init();
void trap_test_init();
int trap_send(rt_uint8_t portid,rt_uint8_t gtrap,rt_uint8_t strap, rt_uint8_t alarmcode, rt_uint8_t flag);
int timing_freeze_data_trap(rt_uint8_t portid);
int connect_state_trap();
int maxday_of_month(int month, int year);
#endif /* PRIVATE_TRAP_H_ */