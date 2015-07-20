/**
 * @file
 * lwip Private MIB
 *
 * @todo create MIB file for this example
 * @note the lwip enterprise tree root (26381) is owned by the lwIP project.
 * It is NOT allowed to allocate new objects under this ID (26381) without our,
 * the lwip developers, permission!
 *
 * Please apply for your own ID with IANA: http://www.iana.org/numbers.html
 *
 * lwip        OBJECT IDENTIFIER ::= { enterprises 26381 }
 * example     OBJECT IDENTIFIER ::= { lwip 1 }
 */

/*
 * Copyright (c) 2006 Axon Digital Design B.V., The Netherlands.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * Author: Christiaan Simons <christiaan.simons@axon.tv>
 */
#include <string.h>      
#include <lwip/opt.h>
#include <syscfgdata.h>
#include <lwip/private_mib.h>
#include <finsh.h>
#include <misc_lib.h>

#if LWIP_SNMP

#include <lwip/snmp_asn1.h>
#include <lwip/snmp_msg.h>
#include <board.h>
#include <rtc.h>

#include <sys_cfg_api.h>
#include <sinkinfo_common.h>
#include <syscfgdata-common.h>
#include <sinkinfo_api4mib.h>
#include <sink_info.h>
#include <sys/time.h>
#include <lwip/private_trap.h>
#include "netif/stm32netif.h"
#include <ade7880_api.h>
#include <ammeter.h>


#define PRIVMIB_DEBUG(x) //printf_syn x

#define PRIVMIB_INFO(x)	//printf_syn x

struct line_data{
	unsigned char time;   //HHMM
	rt_uint32_t LActPowTotalCurve;
	rt_uint32_t LReActPowTotalCurve;

	rt_uint32_t LVolCurveA;
	rt_uint32_t LVolCurveB;
	rt_uint32_t LVolCurveC;
	rt_uint32_t LCurCurveA;
	rt_uint32_t LCurCurveB;
	rt_uint32_t LCurCurveC;
	rt_uint32_t LVolDistortA;
	rt_uint32_t LVolDistortB;
	rt_uint32_t LVolDistortC;
	rt_uint32_t LCurDistortA;
	rt_uint32_t LCurDistortB;
	rt_uint32_t LCurDistortC;
	rt_uint32_t LActPowT;
	rt_uint32_t LActPowA;
	rt_uint32_t LActPowB;
	rt_uint32_t LActPowC;
	rt_uint32_t LReActPowT;
	rt_uint32_t LReActPowA;
	rt_uint32_t LReActPowB;
	rt_uint32_t LReActPowC;
	rt_uint32_t LPowFacT;
	rt_uint32_t LPowFacA;
	rt_uint32_t LPowFacB;
	rt_uint32_t LPowFacC;
	rt_uint32_t LApparentA;
	rt_uint32_t LApparentB;
	rt_uint32_t LApparentC;
	rt_uint32_t LPhaseA;
	rt_uint32_t LPhaseB;
	rt_uint32_t LPhaseC;
	rt_uint32_t LFrequencyA;
	rt_uint32_t LFrequencyB;
	rt_uint32_t LFrequencyC;
//	rt_uint8_t LSamplePAV[160];
//	rt_uint8_t LSamplePBV[160];
//	rt_uint8_t LSamplePCV[160];
//	rt_uint8_t LSamplePAI[160];
//	rt_uint8_t LSamplePBI[160];
//	rt_uint8_t LSamplePCI[160];
};

struct curve_data{
	unsigned char time;   //HHMM
	rt_uint32_t ActPowTotalCurve;
	rt_uint32_t ReActPowTotalCurve;

	rt_uint32_t VolCurveA;
	rt_uint32_t VolCurveB;
	rt_uint32_t VolCurveC;
	rt_uint32_t CurCurveA;
	rt_uint32_t CurCurveB;
	rt_uint32_t CurCurveC;
	rt_uint32_t ActPowT;
	rt_uint32_t ActPowA;
	rt_uint32_t ActPowB;
	rt_uint32_t ActPowC;
	rt_uint32_t ReActPowT;
	rt_uint32_t ReActPowA;
	rt_uint32_t ReActPowB;
	rt_uint32_t ReActPowC;
	rt_uint32_t PowFacT;
	rt_uint32_t PowFacA;
	rt_uint32_t PowFacB;
	rt_uint32_t PowFacC;
};

struct pt_data{
	unsigned char time;   //HHMM
	rt_uint32_t PtVolCurveA;
	rt_uint32_t PtVolCurveB;
	rt_uint32_t PtVolCurveC;
	rt_uint32_t PtActPowA;
	rt_uint32_t PtActPowB;
	rt_uint32_t PtActPowC;
	rt_uint32_t Pt2VolADrop;
	rt_uint32_t Pt2VolBDrop;
	rt_uint32_t Pt2VolCDrop;

};
struct ct_data{
	unsigned char time;   //HHMM
	rt_uint32_t CtCurCurveA;
	rt_uint32_t CtCurCurveB;
	rt_uint32_t CtCurCurveC;
	rt_uint32_t CtActPowA;
	rt_uint32_t CtActPowB;
	rt_uint32_t CtActPowC;
	rt_uint32_t loop2LoadA;
	rt_uint32_t loop2LoadB;
	rt_uint32_t loop2LoadC;

};

struct collect_line_data_info_st {
	struct line_data LineData;       /* line current data */
	rt_uint32_t LMonActTotal;	/* Line mouth total  data */
	rt_uint8_t  lcur_alarm[4];		/* current alarm*/
	rt_uint8_t  lalarm_mask[4];		/* alarm mask */
};

struct collect_meter_data_info_st {
	rt_uint8_t	meter_addr[6];	     /* meter address */
	rt_uint8_t	meter_snid[6];	     /* meter SN ID */
	struct curve_data MeterData;     /* meter current data */
	rt_uint32_t MonActTotal;     /* meter mouth total  data */
	rt_uint8_t	mcur_alarm[4];	      /* current alarm*/
	rt_uint8_t	malarm_mask[4];	      /* alarm mask */
	rt_uint8_t	mfreeze_alarm;	      /* current alarm*/
	rt_uint8_t	mfreeze_alarm_mask;	      /* alarm mask */
	rt_uint8_t	tfreeze_alarm;	      /* current alarm*/
	rt_uint8_t	tfreeze_alarm_mask;	      /* alarm mask */	
};

struct collect_pt_data_info_st {
	struct pt_data PtData;           /* PT current data */
	rt_uint8_t	ptcur_alarm;	      /* current alarm*/
	rt_uint8_t	ptalarm_mask;	      /* alarm mask */
};

struct collect_ct_data_info_st {
	struct ct_data CtData;           /* CT current data */
	rt_uint8_t	ctcur_alarm;	      /* current alarm*/
	rt_uint8_t	ctalarm_mask;	      /* alarm mask */
};


struct data_dev{
	unsigned char time;   //HHMM
	rt_uint32_t VolDev;
	rt_uint32_t CurDev;
	rt_uint32_t ActPowDev;
	rt_uint32_t ReActPowDev;
	rt_uint32_t PowFacDev;
	rt_uint32_t ActTotalDev;
	rt_uint32_t ReActTotalDev;
	rt_uint32_t ActPowTDev;
	rt_uint32_t ReActPowTDev;
	rt_uint32_t PowFacTDev;
	rt_uint32_t MonTotalDev;

};

struct dev_thd{
	unsigned char time;   //HHMM
	rt_uint32_t VolDevThd;
	rt_uint32_t CurDevThd;
	rt_uint32_t ActPowDevThd;
	rt_uint32_t ReActPowDevThd;
	rt_uint32_t PowFacDevThd;
	rt_uint32_t ActTotalDevThd;
	rt_uint32_t ReActTotalDevThd;
	rt_uint32_t ActPowTDevThd;
	rt_uint32_t ReActPowTDevThd;
	rt_uint32_t PowFacTDevThd;
	rt_uint32_t MonTotalDevThd;

};

struct data_analysis_info_st{
	struct data_dev	phaseA_dev; 	/* A相数据误差阀值*/
	struct data_dev	phaseB_dev;		/* B相数据误差阀值*/
	struct data_dev	phaseC_dev;	    /* C相数据误差阀值*/
	struct data_dev	total_dev;	    /* 总数据误差阀值*/
	struct dev_thd 	phaseA_datathd; 	/* A相数据误差阀值0.1%*/
	struct dev_thd 	phaseB_datathd;		/* B相数据误差阀值0.1%*/
	struct dev_thd	phaseC_datathd;	    /* C相数据误差阀值0.1%*/
	struct dev_thd 	total_datathd;	    /* 总数据误差阀值0.1%*/
	rt_uint8_t		thdexc_alarm[4];	      /* A相B相C相total*/
	rt_uint8_t		thdexc_mask[4];	      /* A相B相C相total*/
};


/**
 * IANA assigned enterprise ID for yeejoin is 28381
 * @see http://www.iana.org/assignments/enterprise-numbers
 *
 * @note this enterprise ID is assigned to the yeejoin,
 * all object identifiers living under this ID are assigned
 * by the YEEJOIN (BEIJING) TECHNOLOGY DEVELOPING CO., LTD.!
 * @note don't change this define, use snmp_set_sysobjid()
 *
 * If you need to create your own private MIB you'll need
 * to apply for your own enterprise ID with IANA:
 * http://www.iana.org/numbers.html
 */
#define YEEJOIN_ENTERPRISE_ID 40409

static void yjlt300sys_netcfg_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od);
static void yjlt300sys_netcfg_entry_get_value(struct obj_def *od, u16_t len, void *value);
static u8_t yjlt300sys_netcfg_entry_set_test(struct obj_def *od, u16_t len, void *value);
static void yjlt300sys_netcfg_entry_set_value(struct obj_def *od, u16_t len, void *value);

static void yjlt300sys_mcfg_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od);
static void yjlt300sys_mcfg_entry_get_value(struct obj_def *od, u16_t len, void *value);
static u8_t yjlt300sys_mcfg_entry_set_test(struct obj_def *od, u16_t len, void *value);
static void yjlt300sys_mcfg_entry_set_value(struct obj_def *od, u16_t len, void *value);

static void yjlt300sys_trapcfg_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od);
static void yjlt300sys_trapcfg_entry_get_value(struct obj_def *od, u16_t len, void *value);
static u8_t yjlt300sys_trapcfg_entry_set_test(struct obj_def *od, u16_t len, void *value);
static void yjlt300sys_trapcfg_entry_set_value(struct obj_def *od, u16_t len, void *value);

static void yjlt300sys_envpcfg_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od);
static void yjlt300sys_envpcfg_entry_get_value(struct obj_def *od, u16_t len, void *value);
static u8_t yjlt300sys_envpcfg_entry_set_test(struct obj_def *od, u16_t len, void *value);
static void yjlt300sys_envpcfg_entry_set_value(struct obj_def *od, u16_t len, void *value);

#if RT_USING_GPRS
static void yjlt300sys_gprscfg_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od);
static void yjlt300sys_gprscfg_entry_get_value(struct obj_def *od, u16_t len, void *value);
static u8_t yjlt300sys_gprscfg_entry_set_test(struct obj_def *od, u16_t len, void *value);
static void yjlt300sys_gprscfg_entry_set_value(struct obj_def *od, u16_t len, void *value);
#endif
static void yjlt300if_ethcfg_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od);
static void yjlt300if_ethcfg_entry_get_value(struct obj_def *od, u16_t len, void *value);
static u8_t yjlt300if_ethcfg_entry_set_test(struct obj_def *od, u16_t len, void *value);
static void yjlt300if_ethcfg_entry_set_value(struct obj_def *od, u16_t len, void *value);

static void yjlt300if_485cfg_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od);
static void yjlt300if_485cfg_entry_get_value(struct obj_def *od, u16_t len, void *value);
static u8_t yjlt300if_485cfg_entry_set_test(struct obj_def *od, u16_t len, void *value);
static void yjlt300if_485cfg_entry_set_value(struct obj_def *od, u16_t len, void *value);
#if 0
static void yjlt300if_twcfg_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od);
static void yjlt300if_twcfg_entry_get_value(struct obj_def *od, u16_t len, void *value);
static u8_t yjlt300if_twcfg_entry_set_test(struct obj_def *od, u16_t len, void *value);
static void yjlt300if_twcfg_entry_set_value(struct obj_def *od, u16_t len, void *value);
#endif
static void yjlt300if_pttwcfg_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od);
static void yjlt300if_pttwcfg_entry_get_value(struct obj_def *od, u16_t len, void *value);
static u8_t yjlt300if_pttwcfg_entry_set_test(struct obj_def *od, u16_t len, void *value);
static void yjlt300if_pttwcfg_entry_set_value(struct obj_def *od, u16_t len, void *value);

static void yjlt300if_cttwcfg_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od);
static void yjlt300if_cttwcfg_entry_get_value(struct obj_def *od, u16_t len, void *value);
static u8_t yjlt300if_cttwcfg_entry_set_test(struct obj_def *od, u16_t len, void *value);
static void yjlt300if_cttwcfg_entry_set_value(struct obj_def *od, u16_t len, void *value);

static void yjlt300sys_linedata_read_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od);
static void yjlt300sys_linedata_read_entry_get_value(struct obj_def *od, u16_t len, void *value);
static u8_t yjlt300sys_linedata_read_entry_set_test(struct obj_def *od, u16_t len, void *value);
static void yjlt300sys_linedata_read_entry_set_value(struct obj_def *od, u16_t len, void *value);

static void yjlt300sys_meterdata_read_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od);
static void yjlt300sys_meterdata_read_entry_get_value(struct obj_def *od, u16_t len, void *value);
static u8_t yjlt300sys_meterdata_read_entry_set_test(struct obj_def *od, u16_t len, void *value);
static void yjlt300sys_meterdata_read_entry_set_value(struct obj_def *od, u16_t len, void *value);

static void yjlt300sys_meter_momentfreezedata_read_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od);
static void yjlt300sys_meter_momentfreezedata_read_entry_get_value(struct obj_def *od, u16_t len, void *value);
static u8_t yjlt300sys_meter_momentfreezedata_read_entry_set_test(struct obj_def *od, u16_t len, void *value);
static void yjlt300sys_meter_momentfreezedata_read_entry_set_value(struct obj_def *od, u16_t len, void *value);

static void yjlt300sys_meter_timingfreezedata_read_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od);
static void yjlt300sys_meter_timingfreezedata_read_entry_get_value(struct obj_def *od, u16_t len, void *value);
static u8_t yjlt300sys_meter_timingfreezedata_read_entry_set_test(struct obj_def *od, u16_t len, void *value);
static void yjlt300sys_meter_timingfreezedata_read_entry_set_value(struct obj_def *od, u16_t len, void *value);

#if EM_EVENT_SUPPERT
static void yjlt300sys_meterevent_read_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od);
static void yjlt300sys_meterevent_read_entry_get_value(struct obj_def *od, u16_t len, void *value);
static u8_t yjlt300sys_meterevent_read_entry_set_test(struct obj_def *od, u16_t len, void *value);
static void yjlt300sys_meterevent_read_entry_set_value(struct obj_def *od, u16_t len, void *value);
#endif
static void yjlt300sys_ptdata_read_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od);
static void yjlt300sys_ptdata_read_entry_get_value(struct obj_def *od, u16_t len, void *value);
static u8_t yjlt300sys_ptdata_read_entry_set_test(struct obj_def *od, u16_t len, void *value);
static void yjlt300sys_ptdata_read_entry_set_value(struct obj_def *od, u16_t len, void *value);

static void yjlt300sys_ctdata_read_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od);
static void yjlt300sys_ctdata_read_entry_get_value(struct obj_def *od, u16_t len, void *value);
static u8_t yjlt300sys_ctdata_read_entry_set_test(struct obj_def *od, u16_t len, void *value);
static void yjlt300sys_ctdata_read_entry_set_value(struct obj_def *od, u16_t len, void *value);

static void yjlt300sys_data_analysis_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od);
static void yjlt300sys_data_analysis_entry_get_value(struct obj_def *od, u16_t len, void *value);
static u8_t yjlt300sys_data_analysis_entry_set_test(struct obj_def *od, u16_t len, void *value);
static void yjlt300sys_data_analysis_entry_set_value(struct obj_def *od, u16_t len, void *value);

static void yjlt300sys_dev_thd_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od);
static void yjlt300sys_dev_thd_entry_get_value(struct obj_def *od, u16_t len, void *value);
static u8_t yjlt300sys_dev_thd_entry_set_test(struct obj_def *od, u16_t len, void *value);
static void yjlt300sys_dev_thd_entry_set_value(struct obj_def *od, u16_t len, void *value);

static void yjlt300sys_thdexc_alarm_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od);
static void yjlt300sys_thdexc_alarm_entry_get_value(struct obj_def *od, u16_t len, void *value);
static u8_t yjlt300sys_thdexc_alarm_entry_set_test(struct obj_def *od, u16_t len, void *value);
static void yjlt300sys_thdexc_alarm_entry_set_value(struct obj_def *od, u16_t len, void *value);

//static void yjlt300_trap_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od);
//static void yjlt300_trap_get_value(struct obj_def *od, u16_t len, void *value);
//static u8_t yjlt300_trap_set_test(struct obj_def *od, u16_t len, void *value);
//static void yjlt300_trap_set_value(struct obj_def *od, u16_t len, void *value);

/* lwip .1.3.6.1.4.1.26381 */

/* yjlt300sys_mcfg_entry .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.1.1.1.1 */
/*
 * lt300SYS_MCFG_DEV_INFO
 * "Byte 0 : system's type ;
 * Byte 1 : number of port ;
 * Byte 2 : Soft's Version (major);
 * Byte 3 : Soft's Version (minor);
 * Byte 4 : Soft's Version (revise);
 * Byte 5 : Hardware's Version(major);
 * Byte 6 : Hardware's Version(minor);
 * Byte 7 : Hardware's Version(revise);
 * Byte 8 : FE Cooper port number
 * Byte 9 : FE Fiber port number
 * Byte 10: GE Cooper port number
 * Byte 11: GE Fiber port number
 * Byte 12 : E1 port number
 * Byte 13 : Serial port number
 * Byte 14 : USB-A port number
 * Byte 15 : USB OTG port number
 * Byte 16 : MMC port number
 */
enum yjlt300sys_mcfg_entry_leaf_id_e {
	LT300SYS_MCFG_DEV_SN_ID	= 1,	/* The SN number of the device. */
	LT300SYS_MCFG_SYS_DESCR_ID,	/* the Descr of yeeJOINSC device System */
	LT300SYS_MCFG_DEV_VER_ID,		/* software and hardware version */
	LT300SYS_MCFG_DEV_INFO_ID,		/*port information  */
	LT300SYS_MCFG_ALARM_STATUS_ID,
	LT300SYS_METERDATA_CONNECTING_STATUS_ID, /* meter connection status*/
	LT300SYS_MCFG_NE_ID_ID,		/* NetId */
	LT300SYS_MCFG_SYS_DEFAULT_ID,
	LT300SYS_MCFG_SYS_REBOOT_ID,
};
struct mib_list_rootnode yjlt300sys_mcfg_root = {
	&yjlt300sys_mcfg_entry_get_object_def,
	&yjlt300sys_mcfg_entry_get_value,
	&yjlt300sys_mcfg_entry_set_test,
	&yjlt300sys_mcfg_entry_set_value,
	MIB_NODE_LR,
	0,
	NULL,
	NULL,
	0
};
const s32_t yjlt300sys_mcfg_entry_ids[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
struct mib_node* const yjlt300sys_mcfg_entry_nodes[9] = {
	(struct mib_node*)&yjlt300sys_mcfg_root, (struct mib_node*)&yjlt300sys_mcfg_root,
	(struct mib_node*)&yjlt300sys_mcfg_root, (struct mib_node*)&yjlt300sys_mcfg_root,
	(struct mib_node*)&yjlt300sys_mcfg_root, (struct mib_node*)&yjlt300sys_mcfg_root,
	(struct mib_node*)&yjlt300sys_mcfg_root, (struct mib_node*)&yjlt300sys_mcfg_root,
	(struct mib_node*)&yjlt300sys_mcfg_root,
};
const struct mib_array_node yjlt300sys_mcfg_entry = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	9,
	yjlt300sys_mcfg_entry_ids,
	yjlt300sys_mcfg_entry_nodes
};
/** yjlt300sys_mcfg_tbl(system manage table)   .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.1.1.1 */
s32_t yjlt300sys_mcfg_tbl_ids[1] = {1};
struct mib_node* yjlt300sys_mcfg_tbl_nodes[1] = {
	(struct mib_node*)&yjlt300sys_mcfg_entry,
};
struct mib_ram_array_node yjlt300sys_mcfg_tbl = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_RA,
	0,
	yjlt300sys_mcfg_tbl_ids,
	yjlt300sys_mcfg_tbl_nodes
};

/* yjlt300sys_netcfg_entry .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.1.2.1 */
enum yjlt300sys_netcfg_entry_leaf_id_e {
	LT300SYS_NETCFG_LOCAL_IP_ID = 1,
	LT300SYS_NETCFG_LOCAL_SUBNET_ID,
	LT300SYS_NETCFG_LOCAL_GW_ID,
	LT300SYS_NETCFG_LOCAL_MAC_ID,
	LT300SYS_NETCFG_READ_COMMUNITY_ID,
	LT300SYS_NETCFG_WRITE_COMMUNITY_ID,
	LT300SYS_NETCFG_DATE_TIME_ID,
};
struct mib_list_rootnode yjlt300sys_netcfg_root = {
	&yjlt300sys_netcfg_entry_get_object_def,
	&yjlt300sys_netcfg_entry_get_value,
	&yjlt300sys_netcfg_entry_set_test,
	&yjlt300sys_netcfg_entry_set_value,
	MIB_NODE_LR,
	0,
	NULL,
	NULL,
	0
};
const s32_t yjlt300sys_netcfg_entry_ids[7] = {1, 2, 3, 4, 5, 6, 7};
struct mib_node* const yjlt300sys_netcfg_entry_nodes[7] = {
	(struct mib_node*)&yjlt300sys_netcfg_root, (struct mib_node*)&yjlt300sys_netcfg_root,
	(struct mib_node*)&yjlt300sys_netcfg_root, (struct mib_node*)&yjlt300sys_netcfg_root,
	(struct mib_node*)&yjlt300sys_netcfg_root, (struct mib_node*)&yjlt300sys_netcfg_root,
	(struct mib_node*)&yjlt300sys_netcfg_root,
};
const struct mib_array_node yjlt300sys_netcfg_entry = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	7,
	yjlt300sys_netcfg_entry_ids,
	yjlt300sys_netcfg_entry_nodes
};
/** yjlt300sys_netcfg_tbl   .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.1.1.2 */
s32_t yjlt300sys_netcfg_tbl_ids[1] = {1};
struct mib_node* yjlt300sys_netcfg_tbl_nodes[1] = {
	(struct mib_node*)&yjlt300sys_netcfg_entry,
};
struct mib_ram_array_node yjlt300sys_netcfg_tbl = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_RA,
	0,
	yjlt300sys_netcfg_tbl_ids,
	yjlt300sys_netcfg_tbl_nodes
};

#if RT_USING_GPRS
/* yjlt300sys_gprscfg_entry .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.1.3.1 */
enum yjlt300sys_gprscfg_entry_leaf_id_e {
	LT300SYS_GPRSCFG_SERVER_IP_ID = 1,
	LT300SYS_GPRSCFG_SERVER_SUBNET_ID,
	LT300SYS_GPRSCFG_SERVER_GW_ID,
	LT300SYS_GPRSCFG_SERVER_PORT_ID,
	LT300SYS_GPRSCFG_MASTER_TELNUM_ID,
	LT300SYS_GPRSCFG_SMS_CENTERNUM_ID,
	LT300SYS_GPRSCFG_APN_NAME_ID,
	LT300SYS_GPRSCFG_APN_USERNAME_ID,
	LT300SYS_GPRSCFG_APN_USERPW_ID,
};
struct mib_list_rootnode yjlt300sys_gprscfg_root = {
	&yjlt300sys_gprscfg_entry_get_object_def,
	&yjlt300sys_gprscfg_entry_get_value,
	&yjlt300sys_gprscfg_entry_set_test,
	&yjlt300sys_gprscfg_entry_set_value,
	MIB_NODE_LR,
	0,
	NULL,
	NULL,
	0
};
const s32_t yjlt300sys_gprscfg_entry_ids[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
struct mib_node* const yjlt300sys_gprscfg_entry_nodes[9] = {
	(struct mib_node*)&yjlt300sys_gprscfg_root, (struct mib_node*)&yjlt300sys_gprscfg_root,
	(struct mib_node*)&yjlt300sys_gprscfg_root, (struct mib_node*)&yjlt300sys_gprscfg_root,
	(struct mib_node*)&yjlt300sys_gprscfg_root, (struct mib_node*)&yjlt300sys_gprscfg_root,
	(struct mib_node*)&yjlt300sys_gprscfg_root, (struct mib_node*)&yjlt300sys_gprscfg_root,
	(struct mib_node*)&yjlt300sys_gprscfg_root,
};
const struct mib_array_node yjlt300sys_gprscfg_entry = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	9,
	yjlt300sys_gprscfg_entry_ids,
	yjlt300sys_gprscfg_entry_nodes
};
/** yjlt300sys_netcfg_tbl   .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.1.3 */
s32_t yjlt300sys_gprscfg_tbl_ids[1] = {1};
struct mib_node* yjlt300sys_gprscfg_tbl_nodes[1] = {
	(struct mib_node*)&yjlt300sys_gprscfg_entry,
};
struct mib_ram_array_node yjlt300sys_gprscfg_tbl = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_RA,
	0,
	yjlt300sys_gprscfg_tbl_ids,
	yjlt300sys_gprscfg_tbl_nodes
};
#endif
/* yjlt300sys_trapcfg_entry .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.1.4.1 */
/*
 * trap类型
 * 0 coldStart
 * 1 warmStart
 * 2 linkDown
 * 3 linUp
 * 4 authenticationFailure
 * 5 egpNeighborLoss
 * 6 enterpriseSpecific
 * yjlt300sys_trapcfg只提供对6类型的配置
 */
enum yjlt300sys_trapcfg_entry_leaf_id_e {
	LT300SYS_TRAPCFG_TRAP_TYPE_ID = 1,	/* 0-6 */
	LT300SYS_TRAPCFG_ADDR_ID,
	LT300SYS_TRAPCFG_SNMP_VER_ID,	/* 0:V1 ; 1:V2C */
	LT300SYS_TRAPCFG_TARGET_PORT_ID,	/* default:162 */
	LT300SYS_TRAPCFG_COMMUNITY_ID,	/* default:public */
	LT300SYS_TRAPCFG_VALID_ID,		/* 1:valid ; 0 : invalid */
	LT300SYS_TRAPCFG_PHASEA_VOL_LOSS_EVENT_ID,		/* 1:valid ; 0 : invalid */
	LT300SYS_TRAPCFG_PHASEA_CUR_LOSS_EVENT_ID,		/* 1:valid ; 0 : invalid */
	LT300SYS_TRAPCFG_METER_CLEAR_EVENT_ID,
	LT300SYS_TRAPCFG_DEMAND_CLEAR_EVENT_ID,
	LT300SYS_TRAPCFG_PROGRAM_EVENT_ID,
	LT300SYS_TRAPCFG_CALIBRATE_TIME_EVENT_ID,
	LT300SYS_TRAPCFG_REVERSE_SEQ_VOL_EVENT_ID,
	LT300SYS_TRAPCFG_REVERSE_SEQ_CUR_EVENT_ID,
};
struct mib_list_rootnode yjlt300sys_trapcfg_root = {
	&yjlt300sys_trapcfg_entry_get_object_def,
	&yjlt300sys_trapcfg_entry_get_value,
	&yjlt300sys_trapcfg_entry_set_test,
	&yjlt300sys_trapcfg_entry_set_value,
	MIB_NODE_LR,
	0,
	NULL,
	NULL,
	0
};
const s32_t yjlt300sys_trapcfg_entry_ids[14] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
struct mib_node* const yjlt300sys_trapcfg_entry_nodes[14] = {
	(struct mib_node*)&yjlt300sys_trapcfg_root, (struct mib_node*)&yjlt300sys_trapcfg_root,
	(struct mib_node*)&yjlt300sys_trapcfg_root, (struct mib_node*)&yjlt300sys_trapcfg_root,
	(struct mib_node*)&yjlt300sys_trapcfg_root, (struct mib_node*)&yjlt300sys_trapcfg_root,
	(struct mib_node*)&yjlt300sys_trapcfg_root, (struct mib_node*)&yjlt300sys_trapcfg_root,
	(struct mib_node*)&yjlt300sys_trapcfg_root, (struct mib_node*)&yjlt300sys_trapcfg_root,
	(struct mib_node*)&yjlt300sys_trapcfg_root, (struct mib_node*)&yjlt300sys_trapcfg_root,
	(struct mib_node*)&yjlt300sys_trapcfg_root, (struct mib_node*)&yjlt300sys_trapcfg_root,
};
const struct mib_array_node yjlt300sys_trapcfg_entry = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	14,
	yjlt300sys_trapcfg_entry_ids,
	yjlt300sys_trapcfg_entry_nodes
};
/** yjlt300sys_trapcfg_tbl   .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.1.4 */
s32_t yjlt300sys_trapcfg_tbl_ids[1] = {1};
struct mib_node* yjlt300sys_trapcfg_tbl_nodes[1] = {
	(struct mib_node*)&yjlt300sys_trapcfg_entry,
};
struct mib_ram_array_node yjlt300sys_trapcfg_tbl = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_RA,
	0,
	yjlt300sys_trapcfg_tbl_ids,
	yjlt300sys_trapcfg_tbl_nodes
};

/**
 * yj_lt300_sys .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.1
 */
#if RT_USING_GPRS
const s32_t yj_lt300_sys_ids[4] = {1, 2, 3, 4};
struct mib_node* const yj_lt300_sys_nodes[4] = {
	(struct mib_node*)&yjlt300sys_mcfg_tbl,
	(struct mib_node*)&yjlt300sys_netcfg_tbl,
	(struct mib_node*)&yjlt300sys_gprscfg_tbl,
	(struct mib_node*)&yjlt300sys_trapcfg_tbl,
};
const struct mib_array_node yj_lt300_sys = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	4,
	yj_lt300_sys_ids,
	yj_lt300_sys_nodes
};
#else
const s32_t yj_lt300_sys_ids[3] = {1, 2, 3};
struct mib_node* const yj_lt300_sys_nodes[3] = {
	(struct mib_node*)&yjlt300sys_mcfg_tbl,
	(struct mib_node*)&yjlt300sys_netcfg_tbl,
	(struct mib_node*)&yjlt300sys_trapcfg_tbl,
};
const struct mib_array_node yj_lt300_sys = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	3,
	yj_lt300_sys_ids,
	yj_lt300_sys_nodes
};
#endif
/* yj_lt300_if_leaf  .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.2.x */
enum yjlt300if_485cfg_entry_leaf_id_e {
    LT300IF_RS485_IF_INDEX_ID	= 1,
	LT300IF_RS485_RATE_CTRL_ID,	    /* 1: closed, 0: opened */
	LT300IF_RS485_DATA_BITS_ID,     /* 1~30min (s), 如果不在这个范围内, 就禁止自动关锁 */
	LT300IF_RS485_PARITY_BITS_ID,		/* 1: closed, 0: opened */
	LT300IF_RS485_STOP_BITS_ID,		/* 1: closed, 0: opened */
	LT300IF_RS485_CURRENT_ALARM_ID,		/* 1: closed, 0: opened */
	LT300IF_RS485_ALARM_MASK_ID,		/* 1: closed, 0: opened */
};
struct mib_list_rootnode yjlt300if_485cfg_root = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_LR,
	0,
	NULL,
	NULL,
	0
};
const s32_t yjlt300if_485cfg_entry_ids[7] = {1, 2, 3, 4, 5, 6, 7};
struct mib_node* const yjlt300if_485cfg_entry_nodes[7] = {
	(struct mib_node*)&yjlt300if_485cfg_root, (struct mib_node*)&yjlt300if_485cfg_root,
	(struct mib_node*)&yjlt300if_485cfg_root, (struct mib_node*)&yjlt300if_485cfg_root,
	(struct mib_node*)&yjlt300if_485cfg_root, (struct mib_node*)&yjlt300if_485cfg_root,
	(struct mib_node*)&yjlt300if_485cfg_root,
};
struct mib_array_node yjlt300if_485cfg_entry = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	7,
	yjlt300if_485cfg_entry_ids,
	yjlt300if_485cfg_entry_nodes
};
/** yjlt300sys_trapcfg_tbl   .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.2.2*/
s32_t yjlt300if_485cfg_tbl_ids[1] = {1};
struct mib_node* yjlt300if_485cfg_tbl_nodes[1] = {
	(struct mib_node*)&yjlt300if_485cfg_entry,
};
struct mib_ram_array_node yjlt300if_485cfg_tbl = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_RA,
	0,
	yjlt300if_485cfg_tbl_ids,
	yjlt300if_485cfg_tbl_nodes
};

/* yj_lt300_if_leaf  .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.2.x */
enum yjlt300if_ethcfg_entry_leaf_id_e {
 	LT300IF_ETH_IF_INDEX_ID	= 1,
	LT300IF_ETH_CFG_INFO_ID,	    /* 1: closed, 0: opened */
	LT300IF_ETH_CUR_CFG_INFO_ID,     /* 1~30min (s), 如果不在这个范围内, 就禁止自动关锁 */
	LT300IF_ETH_PERFORM_COUNTER_ID,		/* 1: closed, 0: opened */
	LT300IF_ETH_CURRENT_ALARM_ID,		/* 1: closed, 0: opened */
	LT300IF_ETH_ALARM_MASK_ID,		/* 1: closed, 0: opened */
};

struct mib_list_rootnode yjlt300if_ethcfg_root = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_LR,
	0,
	NULL,
	NULL,
	0
};
const s32_t yjlt300if_ethcfg_entry_ids[6] = {1, 2, 3, 4, 5, 6};
struct mib_node* const yjlt300if_ethcfg_entry_nodes[6] = {
	(struct mib_node*)&yjlt300if_ethcfg_root, (struct mib_node*)&yjlt300if_ethcfg_root,
	(struct mib_node*)&yjlt300if_ethcfg_root, (struct mib_node*)&yjlt300if_ethcfg_root,
	(struct mib_node*)&yjlt300if_ethcfg_root, (struct mib_node*)&yjlt300if_ethcfg_root,
};
const struct mib_array_node yjlt300if_ethcfg_entry = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	6,
	yjlt300if_ethcfg_entry_ids,
	yjlt300if_ethcfg_entry_nodes
};
/** yjlt300sys_trapcfg_tbl   .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.2.2*/
s32_t yjlt300if_ethcfg_tbl_ids[1] = {1};
struct mib_node* yjlt300if_ethcfg_tbl_nodes[1] = {
	(struct mib_node*)&yjlt300if_ethcfg_entry,
};
struct mib_ram_array_node yjlt300if_ethcfg_tbl = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_RA,
	0,
	yjlt300if_ethcfg_tbl_ids,
	yjlt300if_ethcfg_tbl_nodes
};
#if 0
/* yj_lt300_if_leaf  .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.2.x */
enum yjlt300if_twcfg_entry_leaf_id_e {
    LT300IF_TWCFG_INDEX_ID	= 1,
 	LT300IF_TWCFG_SNID_ID,
	LT300IF_TWCFG_MODULATE_TYPE_ID,	    /* 1: closed, 0: opened */
	LT300IF_TWCFG_MANCHESTER_CODE_ID,     /* 1~30min (s), 如果不在这个范围内, 就禁止自动关锁 */
	LT300IF_TWCFG_CENTER_FRQ_ID,		/* 1: closed, 0: opened */
	LT300IF_TWCFG_CHANEL_NUM_ID,		/* 1: closed, 0: opened */
	LT300IF_TWCFG_TX_DEV_ID,		/* 1: closed, 0: opened */
	LT300IF_TWCFG_RX_DEV_ID,		/* 1: closed, 0: opened */
	LT300IF_TWCFG_RX_BW_ID,		/* 1: closed, 0: opened */
	LT300IF_TWCFG_TX_RATA_ID,		/* 1: closed, 0: opened */
	LT300IF_TWCFG_RX_RATA_ID,		/* 1: closed, 0: opened */
	LT300IF_TWCFG_TX_POWER_ID,		/* 1: closed, 0: opened */
	LT300IF_TWCFG_RX_SENSITIVITY_ID,		/* 1: closed, 0: opened */
	LT300IF_TWCFG_CUR_ALARM_ID,		/* 1: closed, 0: opened */
	LT300IF_TWCFG_ALARM_MASK_ID,		/* 1: closed, 0: opened */
};
struct mib_list_rootnode yjlt300if_twcfg_root = {
	&yjlt300if_twcfg_entry_get_object_def,
	&yjlt300if_twcfg_entry_get_value,
	&yjlt300if_twcfg_entry_set_test,
	&yjlt300if_twcfg_entry_set_value,
	MIB_NODE_LR,
	0,
	NULL,
	NULL,
	0
};
const s32_t yjlt300if_twcfg_entry_ids[15] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,15};
struct mib_node* const yjlt300if_twcfg_entry_nodes[15] = {
	(struct mib_node*)&yjlt300if_twcfg_root, (struct mib_node*)&yjlt300if_twcfg_root,
	(struct mib_node*)&yjlt300if_twcfg_root, (struct mib_node*)&yjlt300if_twcfg_root,
	(struct mib_node*)&yjlt300if_twcfg_root, (struct mib_node*)&yjlt300if_twcfg_root,
	(struct mib_node*)&yjlt300if_twcfg_root, (struct mib_node*)&yjlt300if_twcfg_root,
	(struct mib_node*)&yjlt300if_twcfg_root, (struct mib_node*)&yjlt300if_twcfg_root,
	(struct mib_node*)&yjlt300if_twcfg_root, (struct mib_node*)&yjlt300if_twcfg_root,
	(struct mib_node*)&yjlt300if_twcfg_root, (struct mib_node*)&yjlt300if_twcfg_root,
	(struct mib_node*)&yjlt300if_twcfg_root,
};
const struct mib_array_node yjlt300if_twcfg_entry = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	15,
	yjlt300if_twcfg_entry_ids,
	yjlt300if_twcfg_entry_nodes
};
/** yjlt300sys_trapcfg_tbl   .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.2.3*/
s32_t yjlt300if_twcfg_tbl_ids[1] = {1};
struct mib_node* yjlt300if_twcfg_tbl_nodes[1] = {
	(struct mib_node*)&yjlt300if_twcfg_entry,
};
struct mib_ram_array_node yjlt300if_twcfg_tbl = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_RA,
	0,
	yjlt300if_twcfg_tbl_ids,
	yjlt300if_twcfg_tbl_nodes
};
#endif
/* yj_lt300_if_leaf  .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.2.x */
enum yjlt300if_pttwcfg_entry_leaf_id_e {
	LT300IF_PT_TWCFG_INDEX_ID	= 1,
 	LT300IF_PT_TWCFG_SNID_ID	,
	LT300IF_PT_TWCFG_MODULATE_TYPE_ID,	    /* 1: closed, 0: opened */
	LT300IF_PT_TWCFG_MANCHESTER_CODE_ID,     /* 1~30min (s), 如果不在这个范围内, 就禁止自动关锁 */
	LT300IF_PT_TWCFG_CENTER_FRQ_ID,		/* 1: closed, 0: opened */
	LT300IF_PT_TWCFG_CHANEL_NUM_ID,		/* 1: closed, 0: opened */
	LT300IF_PT_TWCFG_TX_DEV_ID,		/* 1: closed, 0: opened */
	LT300IF_PT_TWCFG_RX_DEV_ID,		/* 1: closed, 0: opened */
	LT300IF_PT_TWCFG_RX_BW_ID,		/* 1: closed, 0: opened */
	LT300IF_PT_TWCFG_TX_RATA_ID,		/* 1: closed, 0: opened */
	LT300IF_PT_TWCFG_RX_RATA_ID,		/* 1: closed, 0: opened */
	LT300IF_PT_TWCFG_TX_POWER_ID,		/* 1: closed, 0: opened */
	LT300IF_PT_TWCFG_RX_SENSITIVITY_ID,		/* 1: closed, 0: opened */
	LT300IF_PT_TWCFG_CUR_ALARM_ID,		/* 1: closed, 0: opened */
	LT300IF_PT_TWCFG_ALARM_MASK_ID,		/* 1: closed, 0: opened */
};
struct mib_list_rootnode yjlt300if_pttwcfg_root = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_LR,
	0,
	NULL,
	NULL,
	0
};
const s32_t yjlt300if_pttwcfg_entry_ids[15] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
struct mib_node* const yjlt300if_pttwcfg_entry_nodes[15] = {
	(struct mib_node*)&yjlt300if_pttwcfg_root, (struct mib_node*)&yjlt300if_pttwcfg_root,
	(struct mib_node*)&yjlt300if_pttwcfg_root, (struct mib_node*)&yjlt300if_pttwcfg_root,
	(struct mib_node*)&yjlt300if_pttwcfg_root, (struct mib_node*)&yjlt300if_pttwcfg_root,
	(struct mib_node*)&yjlt300if_pttwcfg_root, (struct mib_node*)&yjlt300if_pttwcfg_root,
	(struct mib_node*)&yjlt300if_pttwcfg_root, (struct mib_node*)&yjlt300if_pttwcfg_root,
	(struct mib_node*)&yjlt300if_pttwcfg_root, (struct mib_node*)&yjlt300if_pttwcfg_root,
	(struct mib_node*)&yjlt300if_pttwcfg_root, (struct mib_node*)&yjlt300if_pttwcfg_root,
	(struct mib_node*)&yjlt300if_pttwcfg_root,
};
const struct mib_array_node yjlt300if_pttwcfg_entry = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	15,
	yjlt300if_pttwcfg_entry_ids,
	yjlt300if_pttwcfg_entry_nodes
};
/** yjlt300sys_trapcfg_tbl   .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.2.3*/
s32_t yjlt300if_pttwcfg_tbl_ids[1] = {1};
struct mib_node* yjlt300if_pttwcfg_tbl_nodes[1] = {
	(struct mib_node*)&yjlt300if_pttwcfg_entry,
};
struct mib_ram_array_node yjlt300if_pttwcfg_tbl = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_RA,
	0,
	yjlt300if_pttwcfg_tbl_ids,
	yjlt300if_pttwcfg_tbl_nodes
};

/* yj_lt300_if_leaf  .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.2.x */
enum yjlt300if_cttwcfg_entry_leaf_id_e {
    LT300IF_CT_TWCFG_INDEX_ID	= 1,
 	LT300IF_CT_TWCFG_SNID_ID	,
	LT300IF_CT_TWCFG_MODULATE_TYPE_ID,	    /* 1: closed, 0: opened */
	LT300IF_CT_TWCFG_MANCHESTER_CODE_ID,     /* 1~30min (s), 如果不在这个范围内, 就禁止自动关锁 */
	LT300IF_CT_TWCFG_CENTER_FRQ_ID,		/* 1: closed, 0: opened */
	LT300IF_CT_TWCFG_CHANEL_NUM_ID,		/* 1: closed, 0: opened */
	LT300IF_CT_TWCFG_TX_DEV_ID,		/* 1: closed, 0: opened */
	LT300IF_CT_TWCFG_RX_DEV_ID,		/* 1: closed, 0: opened */
	LT300IF_CT_TWCFG_RX_BW_ID,		/* 1: closed, 0: opened */
	LT300IF_CT_TWCFG_TX_RATA_ID,		/* 1: closed, 0: opened */
	LT300IF_CT_TWCFG_RX_RATA_ID,		/* 1: closed, 0: opened */
	LT300IF_CT_TWCFG_TX_POWER_ID,		/* 1: closed, 0: opened */
	LT300IF_CT_TWCFG_RX_SENSITIVITY_ID,		/* 1: closed, 0: opened */
	LT300IF_CT_TWCFG_CUR_ALARM_ID,		/* 1: closed, 0: opened */
	LT300IF_CT_TWCFG_ALARM_MASK_ID,		/* 1: closed, 0: opened */
};
struct mib_list_rootnode yjlt300if_cttwcfg_root = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_LR,
	0,
	NULL,
	NULL,
	0
};
const s32_t yjlt300if_cttwcfg_entry_ids[15] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
struct mib_node* const yjlt300if_cttwcfg_entry_nodes[15] = {
	(struct mib_node*)&yjlt300if_cttwcfg_root, (struct mib_node*)&yjlt300if_cttwcfg_root,
	(struct mib_node*)&yjlt300if_cttwcfg_root, (struct mib_node*)&yjlt300if_cttwcfg_root,
	(struct mib_node*)&yjlt300if_cttwcfg_root, (struct mib_node*)&yjlt300if_cttwcfg_root,
	(struct mib_node*)&yjlt300if_cttwcfg_root, (struct mib_node*)&yjlt300if_cttwcfg_root,
	(struct mib_node*)&yjlt300if_cttwcfg_root, (struct mib_node*)&yjlt300if_cttwcfg_root,
	(struct mib_node*)&yjlt300if_cttwcfg_root, (struct mib_node*)&yjlt300if_cttwcfg_root,
	(struct mib_node*)&yjlt300if_cttwcfg_root, (struct mib_node*)&yjlt300if_cttwcfg_root,
	(struct mib_node*)&yjlt300if_cttwcfg_root,
};
const struct mib_array_node yjlt300if_cttwcfg_entry = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	15,
	yjlt300if_cttwcfg_entry_ids,
	yjlt300if_cttwcfg_entry_nodes
};
/** yjlt300sys_trapcfg_tbl   .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.2.3*/
s32_t yjlt300if_cttwcfg_tbl_ids[1] = {1};
struct mib_node* yjlt300if_cttwcfg_tbl_nodes[1] = {
	(struct mib_node*)&yjlt300if_cttwcfg_entry,
};
struct mib_ram_array_node yjlt300if_cttwcfg_tbl = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_RA,
	0,
	yjlt300if_cttwcfg_tbl_ids,
	yjlt300if_cttwcfg_tbl_nodes
};
/**
 * yj_lt300_sys .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.2
 */
const s32_t yj_lt300_if_ids[4] = {1, 2, 3, 4};
struct mib_node* const yj_lt300_if_nodes[5] = {
	(struct mib_node*)&yjlt300if_ethcfg_tbl,
	(struct mib_node*)&yjlt300if_485cfg_tbl,
	//(struct mib_node*)&yjlt300if_twcfg_tbl,
	(struct mib_node*)&yjlt300if_pttwcfg_tbl,
	(struct mib_node*)&yjlt300if_cttwcfg_tbl,
};
const struct mib_array_node yj_lt300_if = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	4,
	yj_lt300_if_ids,
	yj_lt300_if_nodes
};

/* yjlt300sys_eponcfg_entry(epon-line status detection cfg) .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.3.x */
enum yjlt300sys_linedata_read_entry_leaf_id_e {
	LT300SYS_LINEDATA_INDEX_ID = 1,
	LT300SYS_LINEDATA_PHASEA_READ_ID,/* A 相数据*/
	LT300SYS_LINEDATA_PHASEB_READ_ID,	/*  B 相数据*/
	LT300SYS_LINEDATA_PHASEC_READ_ID,	/* C 相数据 */
	LT300SYS_LINEDATA_TOTAL_READ_ID,	/* 总电能 功率 */
	LT300SYS_LINEDATA_PHASEA_VOL_SAMPLE_READ_ID, /*A相电压电流采样值*/
	LT300SYS_LINEDATA_PHASEA_CUR_SAMPLE_READ_ID, /*A相电压电流采样值*/
	LT300SYS_LINEDATA_PHASEB_VOL_SAMPLE_READ_ID,
	LT300SYS_LINEDATA_PHASEB_CUR_SAMPLE_READ_ID,
	LT300SYS_LINEDATA_PHASEC_VOL_SAMPLE_READ_ID,
	LT300SYS_LINEDATA_PHASEC_CUR_SAMPLE_READ_ID,
	LT300SYS_LINEDATA_COPPER_IRON_LOSSES_READ_ID, /* Copper and Iron losses data*/
	LT300SYS_LINEDATA_PHASEA_VOL_HARMONIC_READ_ID, /* phase A 21 voltage harmonic data*/
	LT300SYS_LINEDATA_PHASEA_CUR_HARMONIC_READ_ID, /* phase A 21 current harmonic data*/
	LT300SYS_LINEDATA_PHASEA_ACT_HARMONIC_READ_ID, /* phase A 21 active power harmonic data*/
	LT300SYS_LINEDATA_PHASEB_VOL_HARMONIC_READ_ID, /* phase B 21 voltage harmonic data*/
	LT300SYS_LINEDATA_PHASEB_CUR_HARMONIC_READ_ID, /* phase B 21 current harmonic data*/
	LT300SYS_LINEDATA_PHASEB_ACT_HARMONIC_READ_ID, /* phase B 21 active power harmonic data*/
	LT300SYS_LINEDATA_PHASEC_VOL_HARMONIC_READ_ID, /* phase C 21 voltage harmonic data*/
	LT300SYS_LINEDATA_PHASEC_CUR_HARMONIC_READ_ID, /* phase C 21 current harmonic data*/
	LT300SYS_LINEDATA_PHASEC_ACT_HARMONIC_READ_ID, /* phase C 21 active power harmonic data*/
	LT300SYS_LINEDATA_READ_ALARM_ID,	/* bit1:SC <--> ONU link alarm 0 : Normal , 1 : Unlink bit2:SC <--> OLT link alarm 0 : Normal , 1 : Unlink  */
	LT300SYS_LINEDATA_READ_ALARM_MASK_ID,	/* 1: Mask ; 0 : Unmask */
};
struct mib_list_rootnode yjlt300sys_linedata_read_root = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_LR,
	0,
	NULL,
	NULL,
	0
};

const s32_t yjlt300sys_linedata_read_entry_ids[23] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23};
struct mib_node* const yjlt300sys_linedata_read_entry_nodes[23] = {
	(struct mib_node*)&yjlt300sys_linedata_read_root, (struct mib_node*)&yjlt300sys_linedata_read_root,
	(struct mib_node*)&yjlt300sys_linedata_read_root, (struct mib_node*)&yjlt300sys_linedata_read_root,
	(struct mib_node*)&yjlt300sys_linedata_read_root, (struct mib_node*)&yjlt300sys_linedata_read_root,
	(struct mib_node*)&yjlt300sys_linedata_read_root, (struct mib_node*)&yjlt300sys_linedata_read_root,
	(struct mib_node*)&yjlt300sys_linedata_read_root, (struct mib_node*)&yjlt300sys_linedata_read_root,
	(struct mib_node*)&yjlt300sys_linedata_read_root, (struct mib_node*)&yjlt300sys_linedata_read_root,
	(struct mib_node*)&yjlt300sys_linedata_read_root, (struct mib_node*)&yjlt300sys_linedata_read_root,
	(struct mib_node*)&yjlt300sys_linedata_read_root, (struct mib_node*)&yjlt300sys_linedata_read_root,
	(struct mib_node*)&yjlt300sys_linedata_read_root, (struct mib_node*)&yjlt300sys_linedata_read_root,
	(struct mib_node*)&yjlt300sys_linedata_read_root, (struct mib_node*)&yjlt300sys_linedata_read_root,
	(struct mib_node*)&yjlt300sys_linedata_read_root, (struct mib_node*)&yjlt300sys_linedata_read_root,
	(struct mib_node*)&yjlt300sys_linedata_read_root, 
};
const struct mib_array_node yjlt300sys_linedata_read_entry = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	23,
	yjlt300sys_linedata_read_entry_ids,
	yjlt300sys_linedata_read_entry_nodes
};
/** yjlt300sys_envpcfg_tbl   .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.3.1 */
s32_t yjlt300sys_linedata_read_tbl_ids[1] = {1};
struct mib_node* yjlt300sys_linedata_read_tbl_nodes[1] = {
	(struct mib_node*)&yjlt300sys_linedata_read_entry,
};
struct mib_ram_array_node yjlt300sys_linedata_read_tbl = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_RA,
	0,
	yjlt300sys_linedata_read_tbl_ids,
	yjlt300sys_linedata_read_tbl_nodes
};

/* yjlt300sys_eponcfg_entry(epon-line status detection cfg) .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.3.x */
enum yjlt300sys_meterdata_read_entry_leaf_id_e {
	LT300SYS_METER_INDEX_ID = 1,
	LT300SYS_METER_ADDR_ID ,  /*表地址*/
	LT300SYS_METER_SNID_ID,	    /*  表号*/
	LT300SYS_METER_PROTOCAL_TYPE_ID,	/*电表规约类型*/
	LT300SYS_METER_WIRE_CONN_MODE_ID,	/*  接线方式*/
	LT300SYS_METER_CALIBRATE_TIME_VALID_ID,	/*电表规约类型*/
	LT300SYS_METER_CALIBRATE_TIME_VALUE_ID,	/*  接线方式*/
	LT300SYS_METER_DATE_TIME_VALUE_ID, /*	接线方式*/
	LT300SYS_METERDATA_PHASEA_READ_ID, 	/* A 相数据*/
	LT300SYS_METERDATA_PHASEB_READ_ID, 	/*  B 相数据*/
	LT300SYS_METERDATA_PHASEC_READ_ID, 	/* C 相数据 */
	LT300SYS_METERDATA_TOTAL_READ_ID, 	/* 总电能 功率 */
	LT300SYS_METERDATA_MAX_DEMAND_DATA_ID,	/*电表最大需量数据*/ 
	LT300SYS_METERDATA_COPPER_IRON_LOSSES_DATA_ID,	/*电表铜损、铁损数据*/
	LT300SYS_METER_TEMPERATURE_ID,	/* 表内温度*/
	LT300SYS_METER_CLOCK_BATTERY_VOL_ID,	/* 时钟电池电压*/
	LT300SYS_METER_COLLECT_BATTERY_VOL_ID,	/* 抄表电池电压*/
	LT300SYS_METERDATA_PHASEA_VOL_HARMONIC_READ_ID, /* phase A 21 voltage harmonic data*/
	LT300SYS_METERDATA_PHASEA_CUR_HARMONIC_READ_ID, /* phase A 21 current harmonic data*/
	LT300SYS_METERDATA_PHASEB_VOL_HARMONIC_READ_ID, /* phase B 21 voltage harmonic data*/
	LT300SYS_METERDATA_PHASEB_CUR_HARMONIC_READ_ID, /* phase B 21 current harmonic data*/
	LT300SYS_METERDATA_PHASEC_VOL_HARMONIC_READ_ID, /* phase C 21 voltage harmonic data*/
	LT300SYS_METERDATA_PHASEC_CUR_HARMONIC_READ_ID, /* phase C 21 current harmonic data*/
	LT300SYS_METERDATA_ACT_PULSE_TIME_OUT_ID, /* phase C 21 current harmonic data*/
	LT300SYS_METERDATA_REACT_PULSE_TIME_OUT_ID, /* phase C 21 current harmonic data*/
	LT300SYS_METERDATA_READ_ALARM_ID,	/* bit1:SC <--> ONU link alarm 0 : Normal , 1 : Unlink bit2:SC <--> OLT link alarm 0 : Normal , 1 : Unlink  */
	LT300SYS_METERDATA_READ_ALARM_MASK_ID,	/* 1: Mask ; 0 : Unmask */
};
struct mib_list_rootnode yjlt300sys_meterdata_read_root = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_LR,
	0,
	NULL,
	NULL,
	0
};
const s32_t yjlt300sys_meterdata_read_entry_ids[27] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27};
struct mib_node* const yjlt300sys_meterdata_read_entry_nodes[27] = {
	(struct mib_node*)&yjlt300sys_meterdata_read_root, (struct mib_node*)&yjlt300sys_meterdata_read_root,
	(struct mib_node*)&yjlt300sys_meterdata_read_root, (struct mib_node*)&yjlt300sys_meterdata_read_root,
	(struct mib_node*)&yjlt300sys_meterdata_read_root, (struct mib_node*)&yjlt300sys_meterdata_read_root,
	(struct mib_node*)&yjlt300sys_meterdata_read_root, (struct mib_node*)&yjlt300sys_meterdata_read_root,
	(struct mib_node*)&yjlt300sys_meterdata_read_root, (struct mib_node*)&yjlt300sys_meterdata_read_root,
	(struct mib_node*)&yjlt300sys_meterdata_read_root, (struct mib_node*)&yjlt300sys_meterdata_read_root,
	(struct mib_node*)&yjlt300sys_meterdata_read_root, (struct mib_node*)&yjlt300sys_meterdata_read_root,
	(struct mib_node*)&yjlt300sys_meterdata_read_root, (struct mib_node*)&yjlt300sys_meterdata_read_root,
	(struct mib_node*)&yjlt300sys_meterdata_read_root, (struct mib_node*)&yjlt300sys_meterdata_read_root,
	(struct mib_node*)&yjlt300sys_meterdata_read_root, (struct mib_node*)&yjlt300sys_meterdata_read_root,
	(struct mib_node*)&yjlt300sys_meterdata_read_root, (struct mib_node*)&yjlt300sys_meterdata_read_root,
	(struct mib_node*)&yjlt300sys_meterdata_read_root, (struct mib_node*)&yjlt300sys_meterdata_read_root,
	(struct mib_node*)&yjlt300sys_meterdata_read_root, (struct mib_node*)&yjlt300sys_meterdata_read_root,
	(struct mib_node*)&yjlt300sys_meterdata_read_root,
};
const struct mib_array_node yjlt300sys_meterdata_read_entry = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	27,
	yjlt300sys_meterdata_read_entry_ids,
	yjlt300sys_meterdata_read_entry_nodes
};
/** yjlt300sys_envpcfg_tbl   .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.3.2 */
s32_t yjlt300sys_meterdata_read_tbl_ids[1] = {1};
struct mib_node* yjlt300sys_meterdata_read_tbl_nodes[1] = {
	(struct mib_node*)&yjlt300sys_meterdata_read_entry,
};
struct mib_ram_array_node yjlt300sys_meterdata_read_tbl = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_RA,
	0,
	yjlt300sys_meterdata_read_tbl_ids,
	yjlt300sys_meterdata_read_tbl_nodes
};

/* yjlt300sys_eponcfg_entry(epon-line status detection cfg) .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.3.x */
enum yjlt300sys_meter_momentfreezedata_read_entry_leaf_id_e {
	LT300SYS_METER_MOMENT_FREEZE_TIMES_ID = 1,
	LT300SYS_METER_MOMENT_FREEZE_MAX_TIMES_ID ,  /*表地址*/
	LT300SYS_METER_MOMENT_FREEZE_TIME_ID,	    /*  表号*/
	LT300SYS_METER_MOMENT_FREEZE_ENABLE_ID,	    /*  表号*/
	LT300SYS_METER_MOMENT_FREEZE_DATA_ID,	/*电表规约类型*/
	LT300SYS_METER_MOMENT_FREEZE_DATA_READ_ALARM_ID,	/*电表规约类型*/
	LT300SYS_METER_MOMENT_FREEZE_DATA_READ_ALARM_MASK_ID,	/*电表规约类型*/
};
struct mib_list_rootnode yjlt300sys_meter_momentfreezedata_read_root = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_LR,
	0,
	NULL,
	NULL,
	0
};
const s32_t yjlt300sys_meter_momentfreezedata_read_entry_ids[7] = {1, 2, 3, 4, 5, 6, 7};
struct mib_node* const yjlt300sys_meter_momentfreezedata_read_entry_nodes[7] = {
	(struct mib_node*)&yjlt300sys_meter_momentfreezedata_read_root, (struct mib_node*)&yjlt300sys_meter_momentfreezedata_read_root,
	(struct mib_node*)&yjlt300sys_meter_momentfreezedata_read_root, (struct mib_node*)&yjlt300sys_meter_momentfreezedata_read_root,
	(struct mib_node*)&yjlt300sys_meter_momentfreezedata_read_root, (struct mib_node*)&yjlt300sys_meter_momentfreezedata_read_root,
	(struct mib_node*)&yjlt300sys_meter_momentfreezedata_read_root,
};
const struct mib_array_node yjlt300sys_meter_momentfreezedata_read_entry = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	7,
	yjlt300sys_meter_momentfreezedata_read_entry_ids,
	yjlt300sys_meter_momentfreezedata_read_entry_nodes
};
/** yjlt300sys_envpcfg_tbl   .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.3.2 */
s32_t yjlt300sys_meter_momentfreezedata_read_tbl_ids[1] = {1};
struct mib_node* yjlt300sys_meter_momentfreezedata_read_tbl_nodes[1] = {
	(struct mib_node*)&yjlt300sys_meter_momentfreezedata_read_entry,
};
struct mib_ram_array_node yjlt300sys_meter_momentfreezedata_read_tbl = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_RA,
	0,
	yjlt300sys_meter_momentfreezedata_read_tbl_ids,
	yjlt300sys_meter_momentfreezedata_read_tbl_nodes
};

/* yjlt300sys_eponcfg_entry(epon-line status detection cfg) .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.3.x */
enum yjlt300sys_meter_timingfreezedata_read_entry_leaf_id_e {
	LT300SYS_METER_TIMING_FREEZE_TIMES_ID = 1,
	LT300SYS_METER_TIMING_FREEZE_MAX_TIMES_ID ,  /*表地址*/
	LT300SYS_METER_TIMING_FREEZE_TIME_ID,	    /*  表号*/
	LT300SYS_METER_TIMING_FREEZE_TYPE_ID,	    /*  表号*/
	LT300SYS_METER_TIMING_FREEZE_ENABLE_ID,	    /*  表号*/
	LT300SYS_METER_TIMING_FREEZE_DATA_ID,	/*电表规约类型*/
	LT300SYS_METER_TIMING_FREEZE_DATA_READ_ALARM_ID,	/*电表规约类型*/
	LT300SYS_METER_TIMING_FREEZE_DATA_READ_ALARM_MASK_ID,	/*电表规约类型*/
};
struct mib_list_rootnode yjlt300sys_meter_timingfreezedata_read_root = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_LR,
	0,
	NULL,
	NULL,
	0
};
const s32_t yjlt300sys_meter_timingfreezedata_read_entry_ids[8] = {1, 2, 3, 4, 5, 6, 7, 8};
struct mib_node* const yjlt300sys_meter_timingfreezedata_read_entry_nodes[8] = {
	(struct mib_node*)&yjlt300sys_meter_timingfreezedata_read_root, (struct mib_node*)&yjlt300sys_meter_timingfreezedata_read_root,
	(struct mib_node*)&yjlt300sys_meter_timingfreezedata_read_root, (struct mib_node*)&yjlt300sys_meter_timingfreezedata_read_root,
	(struct mib_node*)&yjlt300sys_meter_timingfreezedata_read_root, (struct mib_node*)&yjlt300sys_meter_timingfreezedata_read_root,
	(struct mib_node*)&yjlt300sys_meter_timingfreezedata_read_root, (struct mib_node*)&yjlt300sys_meter_timingfreezedata_read_root,
};
const struct mib_array_node yjlt300sys_meter_timingfreezedata_read_entry = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	8,
	yjlt300sys_meter_timingfreezedata_read_entry_ids,
	yjlt300sys_meter_timingfreezedata_read_entry_nodes
};
/** yjlt300sys_envpcfg_tbl   .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.3.2 */
s32_t yjlt300sys_meter_timingfreezedata_read_tbl_ids[1] = {1};
struct mib_node* yjlt300sys_meter_timingfreezedata_read_tbl_nodes[1] = {
	(struct mib_node*)&yjlt300sys_meter_timingfreezedata_read_entry,
};
struct mib_ram_array_node yjlt300sys_meter_timingfreezedata_read_tbl = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_RA,
	0,
	yjlt300sys_meter_timingfreezedata_read_tbl_ids,
	yjlt300sys_meter_timingfreezedata_read_tbl_nodes
};



/* yjlt300sys_eponcfg_entry(epon-line status detection cfg) .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.3.x */
enum yjlt300sys_ptdata_read_entry_leaf_id_e { 
	LT300SYS_PTDATA_PHASEA_READ_ID = 1, /* A 相数据*/
	LT300SYS_PTDATA_PHASEB_READ_ID, /* pt ����*/
	LT300SYS_PTDATA_PHASEC_READ_ID, /* pt �й�����*/
	LT300SYS_PTDATA_LOAD_READ_ID,	/* PT ����*/
	LT300SYS_PTDATA_2VOLDROP_READ_ID,	/* C 相数据 */
	LT300SYS_PTDATA_TEMPERATURE_READ_ID, /* pt ���¶�*/
	LT300SYS_PTDATA_READ_ALARM_ID,	/* bit1:SC <--> ONU link alarm 0 : Normal , 1 : Unlink bit2:SC <--> OLT link alarm 0 : Normal , 1 : Unlink  */
	LT300SYS_PTDATA_READ_ALARM_MASK_ID,	/* 1: Mask ; 0 : Unmask */
};
struct mib_list_rootnode yjlt300sys_ptdata_read_root = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_LR,
	0,
	NULL,
	NULL,
	0
};
const s32_t yjlt300sys_ptdata_read_entry_ids[8] = {1, 2, 3, 4, 5, 6, 7, 8};
struct mib_node* const yjlt300sys_ptdata_read_entry_nodes[8] = {
	(struct mib_node*)&yjlt300sys_ptdata_read_root, (struct mib_node*)&yjlt300sys_ptdata_read_root,
	(struct mib_node*)&yjlt300sys_ptdata_read_root, (struct mib_node*)&yjlt300sys_ptdata_read_root,
	(struct mib_node*)&yjlt300sys_ptdata_read_root, (struct mib_node*)&yjlt300sys_ptdata_read_root,
	(struct mib_node*)&yjlt300sys_ptdata_read_root, (struct mib_node*)&yjlt300sys_ptdata_read_root,
};
const struct mib_array_node yjlt300sys_ptdata_read_entry = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	8,
	yjlt300sys_ptdata_read_entry_ids,
	yjlt300sys_ptdata_read_entry_nodes
};
/** yjlt300sys_envpcfg_tbl   .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.3.3 */
s32_t yjlt300sys_ptdata_read_tbl_ids[1] = {1};
struct mib_node* yjlt300sys_ptdata_read_tbl_nodes[1] = {
	(struct mib_node*)&yjlt300sys_ptdata_read_entry,
};
struct mib_ram_array_node yjlt300sys_ptdata_read_tbl = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_RA,
	0,
	yjlt300sys_ptdata_read_tbl_ids,
	yjlt300sys_ptdata_read_tbl_nodes
};
/* yjlt300sys_eponcfg_entry(epon-line status detection cfg) .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.3.x */
enum yjlt300sys_ctdata_read_entry_leaf_id_e {
	LT300SYS_CTDATA_PHASEA_READ_ID = 1, /* A 相数据*/
	LT300SYS_CTDATA_PHASEB_READ_ID, /* CT ��ѹ*/
	LT300SYS_CTDATA_PHASEC_READ_ID, /* CT �й�����*/
	LT300SYS_CTDATA_LOAD_READ_ID,	/*CT ����*/
	LT300SYS_CTDATA_TEMPERATURE_READ_ID, /* CT �¶�*/
	LT300SYS_CTDATA_READ_ALARM_ID,	/* bit1:SC <--> ONU link alarm 0 : Normal , 1 : Unlink  bit2:SC <--> OLT link alarm 0 : Normal , 1 : Unlink  */
	LT300SYS_CTDATA_READ_ALARM_MASK_ID,	/* 1: Mask ; 0 : Unmask */
};
struct mib_list_rootnode yjlt300sys_ctdata_read_root = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_LR,
	0,
	NULL,
	NULL,
	0
};
const s32_t yjlt300sys_ctdata_read_entry_ids[7] = {1, 2, 3, 4, 5, 6, 7};
struct mib_node* const yjlt300sys_ctdata_read_entry_nodes[7] = {
	(struct mib_node*)&yjlt300sys_ctdata_read_root, (struct mib_node*)&yjlt300sys_ctdata_read_root,
	(struct mib_node*)&yjlt300sys_ctdata_read_root, (struct mib_node*)&yjlt300sys_ctdata_read_root,
	(struct mib_node*)&yjlt300sys_ctdata_read_root, (struct mib_node*)&yjlt300sys_ctdata_read_root,
	(struct mib_node*)&yjlt300sys_ctdata_read_root,
};
const struct mib_array_node yjlt300sys_ctdata_read_entry = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	7,
	yjlt300sys_ctdata_read_entry_ids,
	yjlt300sys_ctdata_read_entry_nodes
};
/** yjlt300sys_envpcfg_tbl   .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.3.4 */
s32_t yjlt300sys_ctdata_read_tbl_ids[1] = {1};
struct mib_node* yjlt300sys_ctdata_read_tbl_nodes[1] = {
	(struct mib_node*)&yjlt300sys_ctdata_read_entry,
};
struct mib_ram_array_node yjlt300sys_ctdata_read_tbl = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_RA,
	0,
	yjlt300sys_ctdata_read_tbl_ids,
	yjlt300sys_ctdata_read_tbl_nodes
};

#if 1
/* yjlt300sys_meter_event_entry(epon-line status detection cfg) .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.3.x */
enum yjlt300sys_meterevent_read_entry_leaf_id_e {
	LT300SYS_METER_VOL_LOSS_TOTAL_TIMES_ID = 1,  /*ʧѹ*/
	LT300SYS_METER_VOL_LOSS_TIMES_SET_ID,  /*ʧѹ*/
	LT300SYS_METER_VOL_LOSS_EVENT_SOURCE_ID,  /*ʧѹ*/
	LT300SYS_METER_VOL_LOSS_EVENT_ID,  /*ʧѹ*/
	LT300SYS_METER_VOL_OVER_TOTAL_TIMES_ID,  /*��ѹ*/
	LT300SYS_METER_VOL_OVER_TIMES_SET_ID,  /*��ѹ*/
	LT300SYS_METER_VOL_OVER_EVENT_SOURCE_ID,  /*ʧѹ*/
	LT300SYS_METER_VOL_OVER_EVENT_ID,  /*��ѹ*/
	LT300SYS_METER_VOL_UNDER_TOTAL_TIMES,  /*Ƿѹ*/
	LT300SYS_METER_VOL_UNDER_TIMES_SET_ID,  /*Ƿѹ*/
	LT300SYS_METER_VOL_UNDER_EVENT_SOURCE_ID,  /*ʧѹ*/
	LT300SYS_METER_VOL_UNDER_EVENT_ID,  /*Ƿѹ*/
	LT300SYS_METER_PHASE_BREAK_TOTAL_TIMES_ID,  /*����*/
	LT300SYS_METER_PHASE_BREAK_TIMES_SET_ID,  /*����*/
	LT300SYS_METER_PHASE_BREAK_EVENT_SOURCE_ID,  /*ʧѹ*/
	LT300SYS_METER_PHASE_BREAK_EVENT_ID,  /*����*/
	LT300SYS_METER_CUR_LOSS_TOTAL_TIMES_ID,  /*ʧ��*/
	LT300SYS_METER_CUR_LOSS_TIMES_SET_ID,  /*ʧ��*/
	LT300SYS_METER_CUR_LOSS_EVENT_SOURCE_ID,  /*ʧѹ*/
	LT300SYS_METER_CUR_LOSS_EVENT_ID,  /*ʧ��*/
	LT300SYS_METER_CUR_OVER_TOTAL_TIMES_ID,  /*����*/
	LT300SYS_METER_CUR_OVER_TIMES_SET_ID,  /*����*/
	LT300SYS_METER_CUR_OVER_EVENT_SOURCE_ID,  /*ʧѹ*/
	LT300SYS_METER_CUR_OVER_EVENT_ID,  /*����*/
	LT300SYS_METER_CUR_BREAK_TOTAL_TIMES_ID,  /*����*/
	LT300SYS_METER_CUR_BREAK_TIMES_SET_ID,  /*����*/
	LT300SYS_METER_CUR_BREAK_EVENT_SOURCE_ID,  /*ʧѹ*/
	LT300SYS_METER_CUR_BREAK_EVENT_ID,  /*����*/
	LT300SYS_METER_CLEAR_EVENT_TOTAL_TIMES_ID,  /*�������*/
	LT300SYS_METER_CLEAR_TIMES_SET_ID,  /*�������*/
	LT300SYS_METER_CLEAR_EVENT_ID,  /*�������*/
	LT300SYS_METER_DEMAND_CLEAR_TOTAL_TIMES_ID,  /*��������*/
	LT300SYS_METER_DEMAND_CLEAR_TIMES_SET_ID,  /*��������*/
	LT300SYS_METER_DEMAND_CLEAR_EVENT_ID,  /*��������*/
	LT300SYS_METER_PROGRAMMING_TOTAL_TIMES_ID,  /*���*/
	LT300SYS_METER_PROGRAMMING_TIMES_SET_ID,  /*���*/
	LT300SYS_METER_PROGRAMMING_EVENT_ID,  /*���*/
	LT300SYS_METER_CALIBRATE_TIME_TOTAL_TIMES_ID,  /*Уʱ*/
	LT300SYS_METER_CALIBRATE_TIME_TIMES_SET_ID,  /*Уʱ*/	
	LT300SYS_METER_CALIBRATE_TIME_EVENT_ID,  /*Уʱ*/
	LT300SYS_METER_VOL_REV_PHASE_TOTAL_TIMES_ID,  /*��ѹ������*/
	LT300SYS_METER_VOL_REV_PHASE_TIMES_SET_ID,  /*��ѹ������*/
	LT300SYS_METER_VOL_REV_PHASE_EVENT_ID,  /*��ѹ������*/
	LT300SYS_METER_CUL_REV_PHASE_TOTAL_TIMES_ID,  /*����������*/
	LT300SYS_METER_CUL_REV_PHASE_TIMES_SET_ID,  /*����������*/
	LT300SYS_METER_CUL_REV_PHASE_EVENT_ID,  /*����������*/
};
struct mib_list_rootnode yjlt300sys_meterevent_read_root = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_LR,
	0,
	NULL,
	NULL,
	0
};
const s32_t yjlt300sys_meterevent_read_entry_ids[46] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,14,15,16,17,18,19,20,21,22,23,
														24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46};
struct mib_node* const yjlt300sys_meterevent_read_entry_nodes[46] = {
	(struct mib_node*)&yjlt300sys_meterevent_read_root, (struct mib_node*)&yjlt300sys_meterevent_read_root,
	(struct mib_node*)&yjlt300sys_meterevent_read_root, (struct mib_node*)&yjlt300sys_meterevent_read_root,
	(struct mib_node*)&yjlt300sys_meterevent_read_root, (struct mib_node*)&yjlt300sys_meterevent_read_root,
	(struct mib_node*)&yjlt300sys_meterevent_read_root, (struct mib_node*)&yjlt300sys_meterevent_read_root,
	(struct mib_node*)&yjlt300sys_meterevent_read_root, (struct mib_node*)&yjlt300sys_meterevent_read_root,
	(struct mib_node*)&yjlt300sys_meterevent_read_root, (struct mib_node*)&yjlt300sys_meterevent_read_root,
	(struct mib_node*)&yjlt300sys_meterevent_read_root, (struct mib_node*)&yjlt300sys_meterevent_read_root,
	(struct mib_node*)&yjlt300sys_meterevent_read_root, (struct mib_node*)&yjlt300sys_meterevent_read_root,
	(struct mib_node*)&yjlt300sys_meterevent_read_root, (struct mib_node*)&yjlt300sys_meterevent_read_root,
	(struct mib_node*)&yjlt300sys_meterevent_read_root, (struct mib_node*)&yjlt300sys_meterevent_read_root,
	(struct mib_node*)&yjlt300sys_meterevent_read_root, (struct mib_node*)&yjlt300sys_meterevent_read_root,
	(struct mib_node*)&yjlt300sys_meterevent_read_root, (struct mib_node*)&yjlt300sys_meterevent_read_root,
	(struct mib_node*)&yjlt300sys_meterevent_read_root, (struct mib_node*)&yjlt300sys_meterevent_read_root,
	(struct mib_node*)&yjlt300sys_meterevent_read_root, (struct mib_node*)&yjlt300sys_meterevent_read_root,
	(struct mib_node*)&yjlt300sys_meterevent_read_root, (struct mib_node*)&yjlt300sys_meterevent_read_root,
	(struct mib_node*)&yjlt300sys_meterevent_read_root, (struct mib_node*)&yjlt300sys_meterevent_read_root,
	(struct mib_node*)&yjlt300sys_meterevent_read_root, (struct mib_node*)&yjlt300sys_meterevent_read_root,
	(struct mib_node*)&yjlt300sys_meterevent_read_root, (struct mib_node*)&yjlt300sys_meterevent_read_root,
	(struct mib_node*)&yjlt300sys_meterevent_read_root, (struct mib_node*)&yjlt300sys_meterevent_read_root,
	(struct mib_node*)&yjlt300sys_meterevent_read_root, (struct mib_node*)&yjlt300sys_meterevent_read_root,
	(struct mib_node*)&yjlt300sys_meterevent_read_root, (struct mib_node*)&yjlt300sys_meterevent_read_root,
	(struct mib_node*)&yjlt300sys_meterevent_read_root, (struct mib_node*)&yjlt300sys_meterevent_read_root,
	(struct mib_node*)&yjlt300sys_meterevent_read_root, (struct mib_node*)&yjlt300sys_meterevent_read_root,
};
const struct mib_array_node yjlt300sys_meterevent_read_entry = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	46,
	yjlt300sys_meterevent_read_entry_ids,
	yjlt300sys_meterevent_read_entry_nodes
};
/** yjlt300sys_envpcfg_tbl   .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.3.2 */
s32_t yjlt300sys_meterevent_read_tbl_ids[1] = {1};
struct mib_node* yjlt300sys_meterevent_read_tbl_nodes[1] = {
	(struct mib_node*)&yjlt300sys_meterevent_read_entry,
};
struct mib_ram_array_node yjlt300sys_meterevent_read_tbl = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_RA,
	0,
	yjlt300sys_meterevent_read_tbl_ids,
	yjlt300sys_meterevent_read_tbl_nodes
};
#endif
/* yjlt300sys_envpcfg_entry(environment parameter cfg) .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.2.3.1 */
enum yjlt300sys_envpcfg_entry_leaf_id_e {
	LT300SYS_ENVPCFG_TMP_VALUE_ID = 1,	/* get temperature and relative humidity,byte0-1: Temperature, byte2-3: relative Humidity */
	LT300SYS_ENVPCFG_RH_VALUE_ID,
	LT300SYS_ENVPCFG_TMP_LIMEN_ID,		/* set temperature limen, 下限, 上限 */
	LT300SYS_ENVPCFG_RH_LIMEN_ID,		/* set relative humidity limen */
	LT300SYS_ENVPCFG_TMP_ALARM_ID,		/* temperature or relative humidity out of gauge alarm */
	LT300SYS_ENVPCFG_RH_ALARM_ID,
	LT300SYS_ENVPCFG_TMP_ALARM_MASK_ID,	/*  */
	LT300SYS_ENVPCFG_RH_ALARM_MASK_ID,	/*  */
};
struct mib_list_rootnode yjlt300sys_envpcfg_root = {
	&yjlt300sys_envpcfg_entry_get_object_def,
	&yjlt300sys_envpcfg_entry_get_value,
	&yjlt300sys_envpcfg_entry_set_test,
	&yjlt300sys_envpcfg_entry_set_value,
	MIB_NODE_LR,
	0,
	NULL,
	NULL,
	0
};
const s32_t yjlt300sys_envpcfg_entry_ids[8] = {1, 2, 3, 4, 5, 6, 7, 8};
struct mib_node* const yjlt300sys_envpcfg_entry_nodes[8] = {
	(struct mib_node*)&yjlt300sys_envpcfg_root, (struct mib_node*)&yjlt300sys_envpcfg_root,
	(struct mib_node*)&yjlt300sys_envpcfg_root, (struct mib_node*)&yjlt300sys_envpcfg_root,
	(struct mib_node*)&yjlt300sys_envpcfg_root, (struct mib_node*)&yjlt300sys_envpcfg_root,
	(struct mib_node*)&yjlt300sys_envpcfg_root, (struct mib_node*)&yjlt300sys_envpcfg_root,
};
const struct mib_array_node yjlt300sys_envpcfg_entry = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	8,
	yjlt300sys_envpcfg_entry_ids,
	yjlt300sys_envpcfg_entry_nodes
};
/** yjlt300sys_envpcfg_tbl   .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.2.3 */
s32_t yjlt300sys_envpcfg_tbl_ids[1] = {1};
struct mib_node* yjlt300sys_envpcfg_tbl_nodes[1] = {
	(struct mib_node*)&yjlt300sys_envpcfg_entry,
};
struct mib_ram_array_node yjlt300sys_envpcfg_tbl = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_RA,
	0,
	yjlt300sys_envpcfg_tbl_ids,
	yjlt300sys_envpcfg_tbl_nodes
};

/*
 * yj_lt300_if  .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.2
 */
const s32_t yj_lt300_datacol_ids[8] = {1, 2, 3, 4, 5, 6, 7, 8};
struct mib_node* const yj_lt300_datacol_nodes[8] = {
	(struct mib_node*)&yjlt300sys_linedata_read_tbl,
	(struct mib_node*)&yjlt300sys_meterdata_read_tbl,
	(struct mib_node*)&yjlt300sys_meter_momentfreezedata_read_tbl,
	(struct mib_node*)&yjlt300sys_meter_timingfreezedata_read_tbl,
	(struct mib_node*)&yjlt300sys_ptdata_read_tbl,   
	(struct mib_node*)&yjlt300sys_ctdata_read_tbl,
	(struct mib_node*)&yjlt300sys_envpcfg_tbl,
	(struct mib_node*)&yjlt300sys_meterevent_read_tbl,
};
const struct mib_array_node yj_lt300_datacol = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	8,
	yj_lt300_datacol_ids,
	yj_lt300_datacol_nodes
};

/* yjlt300sys_eponcfg_entry(epon-line status detection cfg) .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.4.x */
enum yjlt300sys_data_analysis_entry_leaf_id_e {
	LT300SYS_PHASEA_DEV_ID = 1,  /*表地址*/
	LT300SYS_PHASEB_DEV_ID , /* A 相数据*/
	LT300SYS_PHASEC_DEV_ID, 
	LT300SYS_TOTAL_DEV_ID ,
};
struct mib_list_rootnode yjlt300sys_data_analysis_root = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_LR,
	0,
	NULL,
	NULL,
	0
};
const s32_t yjlt300sys_data_analysis_entry_ids[4] = {1, 2, 3, 4};
struct mib_node* const yjlt300sys_data_analysis_entry_nodes[4] = {
	(struct mib_node*)&yjlt300sys_data_analysis_root, (struct mib_node*)&yjlt300sys_data_analysis_root,
	(struct mib_node*)&yjlt300sys_data_analysis_root, (struct mib_node*)&yjlt300sys_data_analysis_root,
};
const struct mib_array_node yjlt300sys_data_analysis_entry = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	4,
	yjlt300sys_data_analysis_entry_ids,
	yjlt300sys_data_analysis_entry_nodes
};
/** yjlt300sys_envpcfg_tbl   .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.4.1 */
s32_t yjlt300sys_data_analysis_tbl_ids[1] = {1};
struct mib_node* yjlt300sys_data_analysis_tbl_nodes[1] = {
	(struct mib_node*)&yjlt300sys_data_analysis_entry,
};
struct mib_ram_array_node yjlt300sys_data_analysis_tbl = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_RA,
	0,
	yjlt300sys_data_analysis_tbl_ids,
	yjlt300sys_data_analysis_tbl_nodes
};

/* yjlt300sys_eponcfg_entry(epon-line status detection cfg) .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.4.x */
enum yjlt300sys_dev_thd_entry_leaf_id_e {
	LT300SYS_PHASEA_DEV_THD_ID = 1,  /*表地址*/
	LT300SYS_PHASEB_DEV_THD_ID , /* A 相数据*/
	LT300SYS_PHASEC_DEV_THD_ID, 
	LT300SYS_TOTAL_DEV_THD_ID ,
};
struct mib_list_rootnode yjlt300sys_dev_thd_root = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_LR,
	0,
	NULL,
	NULL,
	0
};
const s32_t yjlt300sys_dev_thd_entry_ids[4] = {1, 2, 3, 4};
struct mib_node* const yjlt300sys_dev_thd_entry_nodes[4] = {
	(struct mib_node*)&yjlt300sys_dev_thd_root, (struct mib_node*)&yjlt300sys_dev_thd_root,
	(struct mib_node*)&yjlt300sys_dev_thd_root, (struct mib_node*)&yjlt300sys_dev_thd_root,
};
const struct mib_array_node yjlt300sys_dev_thd_entry = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	4,
	yjlt300sys_dev_thd_entry_ids,
	yjlt300sys_dev_thd_entry_nodes
};
/** yjlt300sys_envpcfg_tbl   .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.4.3 */
s32_t yjlt300sys_dev_thd_tbl_ids[1] = {1};
struct mib_node* yjlt300sys_dev_thd_tbl_nodes[1] = {
	(struct mib_node*)&yjlt300sys_dev_thd_entry,
};
struct mib_ram_array_node yjlt300sys_dev_thd_tbl = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_RA,
	0,
	yjlt300sys_dev_thd_tbl_ids,
	yjlt300sys_dev_thd_tbl_nodes
};

/* yjlt300sys_eponcfg_entry(epon-line status detection cfg) .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.4.x */
enum yjlt300sys_thdexc_alarm_entry_leaf_id_e {
    LT300SYS_PHASEA_THDEXC_ALARM_ID = 1,  /*表地址*/
	LT300SYS_PHASEB_THDEXC_ALARM_ID , /* A 相数据*/
	LT300SYS_PHASEC_THDEXC_ALARM_ID, 
	LT300SYS_TOTAL_THDEXC_ALARM_ID ,
};
struct mib_list_rootnode yjlt300sys_thdexc_alarm_root = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_LR,
	0,
	NULL,
	NULL,
	0
};
const s32_t yjlt300sys_thdexc_alarm_entry_ids[4] = {1, 2, 3, 4};
struct mib_node* const yjlt300sys_thdexc_alarm_entry_nodes[4] = {
	(struct mib_node*)&yjlt300sys_thdexc_alarm_root, (struct mib_node*)&yjlt300sys_thdexc_alarm_root,
	(struct mib_node*)&yjlt300sys_thdexc_alarm_root, (struct mib_node*)&yjlt300sys_thdexc_alarm_root,
};
const struct mib_array_node yjlt300sys_thdexc_alarm_entry = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	4,
	yjlt300sys_thdexc_alarm_entry_ids,
	yjlt300sys_thdexc_alarm_entry_nodes
};
/** yjlt300sys_envpcfg_tbl   .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.4.2 */
s32_t yjlt300sys_thdexc_alarm_tbl_ids[1] = {1};
struct mib_node* yjlt300sys_thdexc_alarm_tbl_nodes[1] = {
	(struct mib_node*)&yjlt300sys_thdexc_alarm_entry,
};
struct mib_ram_array_node yjlt300sys_thdexc_alarm_tbl = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_RA,
	0,
	yjlt300sys_thdexc_alarm_tbl_ids,
	yjlt300sys_thdexc_alarm_tbl_nodes
};

/*
 * yj_lt300_if  .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3.2
 */
const s32_t yj_lt300_analysis_ids[3] = {1, 2, 3};
struct mib_node* const yj_lt300_analysis_nodes[3] = {
	(struct mib_node*)&yjlt300sys_data_analysis_tbl, 
	(struct mib_node*)&yjlt300sys_dev_thd_tbl,
	(struct mib_node*)&yjlt300sys_thdexc_alarm_tbl,   
};
const struct mib_array_node yj_lt300_analysis = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	3,
	yj_lt300_analysis_ids,
	yj_lt300_analysis_nodes
};

enum yjlt300sys_trap_id_e {
	LT300SYS_ETH_LINK_STATUS_CHANGE = 1,	/* 以太网口状态改变 */
	LT300SYS_485_LINK_STATUS_CHANGE,		/* 485 接口状态改变*/
	LT300SYS_DEV_TW_STATUS_CHANGE,	/* 采集器无线接口通信状态改变*/
	LT300SYS_PT_TW_STATUS_CHANGE,	/* PT侧无线接口通信状态改变*/
	LT300SYS_CT_TW_STATUS_CHANGE,	/* CT侧无线接口通信状态改变 */
	LT300SYS_TEMP_RH_OUTOF_GAUGE,	      /* 温度或湿度超限, M3需要实现 */
	LT300SYS_LINE_DATA_READ_STATUS_CHANGE,	/* 线路计量告警状态 */
	LT300SYS_METER_DATA_READ_STATUS_CHANGE,	/* 电表采集告警状态*/
	LT300SYS_PT_DATA_READ_STATUS_CHANGE,	   /* PT侧数据采集告警状态*/
	LT300SYS_CT_DATA_READ_STATUS_CHANGE,   	/* CT侧数据告警状态 */
	LT300SYS_PHASEA_THDEXC_ALARM_STATUS,		/* A相数据误差超限告警 */
	LT300SYS_PHASEB_THDEXC_ALARM_STATUS,		/* B相数据误差超限告警 */
	LT300SYS_PHASEC_THDEXC_ALARM_STATUS,		/* C相数据误差超限告警 */
	LT300SYS_TOTAL_THDEXC_ALARM_STATUS,		/* 总电能数据误差超限告警 */
};

/*             0 1 2 3 4 5 6 */
/* system .1.3.6.1.2.1.1 */
/*const mib_scalar_node yjlt300_trap_scalar = {
	&yjlt300_trap_get_object_def,
	&yjlt300_trap_get_value,
	&yjlt300_trap_set_test,
	&yjlt300_trap_set_value,
	MIB_NODE_SC,
	0
};*/
const mib_scalar_node yjlt300_trap_scalar = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_SC,
	0
};
const s32_t yj_lt300_trap_ids[14] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
struct mib_node* const yj_lt300_trap_nodes[14] = {
	(struct mib_node*)&yjlt300_trap_scalar, (struct mib_node*)&yjlt300_trap_scalar,
	(struct mib_node*)&yjlt300_trap_scalar, (struct mib_node*)&yjlt300_trap_scalar,
	(struct mib_node*)&yjlt300_trap_scalar, (struct mib_node*)&yjlt300_trap_scalar,
	(struct mib_node*)&yjlt300_trap_scalar, (struct mib_node*)&yjlt300_trap_scalar,
	(struct mib_node*)&yjlt300_trap_scalar, (struct mib_node*)&yjlt300_trap_scalar,
	(struct mib_node*)&yjlt300_trap_scalar, (struct mib_node*)&yjlt300_trap_scalar,
	(struct mib_node*)&yjlt300_trap_scalar, (struct mib_node*)&yjlt300_trap_scalar,
};
/* work around name issue with 'sys_tem', some compiler(s?) seem to reserve 'system' */
const struct mib_array_node yj_lt300_trap = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	14,
	yj_lt300_trap_ids,
	yj_lt300_trap_nodes
};

/*
 * yj_lt300_trap .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.1.3
 *
 * .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.1.3.1 -- elock status change: When elock status changed,send the trap.
 * .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.1.3.2 -- door status change
 * .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.1.3.3 -- etnernet link status change
 * .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.1.3.4 -- temp and rh alarm
 * .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.1.3.5 --
 */


/*
 * yeejoin-scb .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.3
 */
const s32_t yj_lt300_ids[5] = { 1, 2, 3, 4, 5};
struct mib_node* const yj_lt300_nodes[5] = {
	(struct mib_node* const)&yj_lt300_sys,
	(struct mib_node* const)&yj_lt300_if,
	(struct mib_node* const)&yj_lt300_datacol,
	(struct mib_node* const)&yj_lt300_analysis,
	(struct mib_node* const)&yj_lt300_trap,
};
const struct mib_array_node yj_lt300 = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	5,
	yj_lt300_ids,
	yj_lt300_nodes
};

/* devices .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2 */
const s32_t devices_ids[1] = {3};
struct mib_node* const devices_nodes[1] = { (struct mib_node* const)&yj_lt300 };
const struct mib_array_node devices = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	1,
	devices_ids,
	devices_nodes
};


/* yeejoin .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID */
const s32_t yeejoin_ids[1] = {2};
struct mib_node* const yeejoin_nodes[1] = { (struct mib_node* const)&devices };
const struct mib_array_node yeejoin = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	1,
	yeejoin_ids,
	yeejoin_nodes
};

/* enterprises .1.3.6.1.4.1 */
const s32_t enterprises_ids[1] = {YEEJOIN_ENTERPRISE_ID};
struct mib_node* const enterprises_nodes[1] = { (struct mib_node* const)&yeejoin };
const struct mib_array_node enterprises = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	1,
	enterprises_ids,
	enterprises_nodes
};

/* private .1.3.6.1.4 */
const s32_t private_ids[1] = { 1 };
struct mib_node* const private_nodes[1] = { (struct mib_node* const)&enterprises };
const struct mib_array_node mib_private = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	1,
	private_ids,
	private_nodes
};
void snmp_insert_netidx_tree(void)
{
  	struct mib_list_node *if_node = NULL;
  	struct mib_list_rootnode *at_rn;
 	u8_t tree;
 	int net_id = get_net_id();
	
#if RT_USING_GPRS
	int level = 5;
#else
	int level = 4;
#endif

	for (tree = 0; tree < level; tree++){
 	 	if (tree == 0){
   			at_rn = &yjlt300sys_mcfg_root;
   	 	}
		else if(tree == 1){
    			at_rn = &yjlt300sys_netcfg_root;	
    	}
		else if(tree == 2){
    			at_rn = &yjlt300sys_trapcfg_root;
    	}
#if RT_USING_GPRS
		else if(tree == 3){
    			at_rn = &yjlt300sys_gprscfg_root;
    	}
		else if(tree == 4){
    			at_rn = &yjlt300sys_envpcfg_root;
    	}
#else
		else if(tree == 3){
    			at_rn = &yjlt300sys_envpcfg_root;
    	}
#endif
		snmp_mib_node_insert(at_rn, net_id, &if_node);
	}
  
 	 /* enable getnext traversal on filled table */
	yjlt300sys_mcfg_tbl.maxlength = 1;
	yjlt300sys_netcfg_tbl.maxlength = 1;
	yjlt300sys_trapcfg_tbl.maxlength = 1;
#if RT_USING_GPRS	
	yjlt300sys_gprscfg_tbl.maxlength = 1;
#endif
	yjlt300sys_envpcfg_tbl.maxlength = 1;
}

void snmp_inc_rs485(u8_t port)
{
	struct mib_list_rootnode *rs485_rn;
	struct mib_list_node *rs485_node ;
	s32_t rs485idx[2];
	rs485idx[0] = get_net_id();
	rs485idx[1] = port + 1;
	u8_t level;
	rs485_rn = &yjlt300if_485cfg_root;
	 
	for (level = 0; level < 2; level++){
		rs485_node = NULL;
		snmp_mib_node_insert(rs485_rn, rs485idx[level], &rs485_node);
		if ((level != 1) && (rs485_node != NULL)){
    			 if (rs485_node->nptr == NULL){
       			 rs485_rn = snmp_mib_lrn_alloc();
       			 rs485_node->nptr = (struct mib_node*)rs485_rn;
				 if (rs485_rn != NULL){
					if (level == 0){
           					rs485_rn->get_object_def = yjlt300if_485cfg_entry_get_object_def;
            				rs485_rn->get_value = yjlt300if_485cfg_entry_get_value;
           					rs485_rn->set_test = yjlt300if_485cfg_entry_set_test;
           					rs485_rn->set_value = yjlt300if_485cfg_entry_set_value;
         				 }
        			 }
      		 		else {
         					/* ipa_rn == NULL, malloc failure */
         		 			LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_inc_rs485_ipaddridx_tree() insert failed, mem full"));
         					break;
        	  			}
     			}
      			else{
       				rs485_rn = (struct mib_list_rootnode*)rs485_node->nptr;
     				}
   		}
	}
	/* enable getnext traversal on filled table */
	yjlt300if_485cfg_tbl.maxlength = 1;
}

void snmp_inc_eth(u8_t port)
{
	struct mib_list_rootnode *eth_rn;
	struct mib_list_node *eth_node ;
	s32_t ethidx[2];
	ethidx[0] = get_net_id();
	ethidx[1] = port + 1;
	u8_t level;
	eth_rn = &yjlt300if_ethcfg_root;
	for (level = 0; level < 2; level++){
		eth_node = NULL;
		snmp_mib_node_insert(eth_rn, ethidx[level], &eth_node);
		if ((level != 1) && (eth_node != NULL)){
    			 if (eth_node->nptr == NULL){
       			 eth_rn = snmp_mib_lrn_alloc();
       			 eth_node->nptr = (struct mib_node*)eth_rn;
				 if (eth_rn != NULL){
					if (level == 0){
           					eth_rn->get_object_def = yjlt300if_ethcfg_entry_get_object_def;
            				eth_rn->get_value = yjlt300if_ethcfg_entry_get_value;
           					eth_rn->set_test = yjlt300if_ethcfg_entry_set_test;
           					eth_rn->set_value = yjlt300if_ethcfg_entry_set_value;
         				 }
        			 }
      		 		else {
         					/* ipa_rn == NULL, malloc failure */
         		 			LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_inc_eth_ipaddridx_tree() insert failed, mem full"));
         					break;
        	  			}
     			}
      			else{
       				eth_rn = (struct mib_list_rootnode*)eth_node->nptr;
     				}
   		}
	}

	/* enable getnext traversal on filled table */
	yjlt300if_ethcfg_tbl.maxlength = 1;
}
#if 0
void snmp_inc_trap(u8_t port)
{
	struct mib_list_rootnode *trap_rn;
	struct mib_list_node *trap_node ;
	s32_t trapidx[2];
	trapidx[0] = get_net_id();
	trapidx[1] = port ;
	u8_t level;
	trap_rn = &yjlt300sys_trapcfg_root;

	for (level = 0; level < 2; level++){
		trap_node = NULL;
		snmp_mib_node_insert(trap_rn, trapidx[level], &trap_node);
		if ((level != 1) && (trap_node != NULL)){
    		if (trap_node->nptr == NULL){
       			trap_rn = snmp_mib_lrn_alloc();
       			trap_node->nptr = (struct mib_node*)trap_rn;
				if (trap_rn != NULL){
					if (level == 0){
           					trap_rn->get_object_def = yjlt300sys_trapcfg_entry_get_object_def;
            				trap_rn->get_value = yjlt300sys_trapcfg_entry_get_value;
           					trap_rn->set_test = yjlt300sys_trapcfg_entry_set_test;
           					trap_rn->set_value = yjlt300sys_trapcfg_entry_set_value;
         			}
        		}
      		 	else {
         			/* ipa_rn == NULL, malloc failure */
         		 	LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_inc_trap_ipaddridx_tree() insert failed, mem full"));
         			break;
        	  		}
     		}
      		else{
       			trap_rn = (struct mib_list_rootnode*)trap_node->nptr;
     		}
   		}
	}
	
	/* enable getnext traversal on filled table */
	yjlt300sys_trapcfg_tbl.maxlength = 1;
}
#endif
void snmp_inc_line(u8_t port)
{
	struct mib_list_rootnode *line_rn;
	struct mib_list_node *line_node ;
	s32_t lineptidx[2];
	lineptidx[0] = get_net_id();
	lineptidx[1] = port + 1;
	u8_t level;
	line_rn = &yjlt300sys_linedata_read_root;
	
	for (level = 0; level < 2; level++){
		line_node = NULL;
		snmp_mib_node_insert(line_rn, lineptidx[level], &line_node);
		if ((level != 1) && (line_node != NULL)){
    		if (line_node->nptr == NULL){
       			line_rn = snmp_mib_lrn_alloc();
       			line_node->nptr = (struct mib_node*)line_rn;
				if (line_rn != NULL){
					if (level == 0){
           					line_rn->get_object_def = yjlt300sys_linedata_read_entry_get_object_def;
            				line_rn->get_value = yjlt300sys_linedata_read_entry_get_value;
           					line_rn->set_test = yjlt300sys_linedata_read_entry_set_test;
           					line_rn->set_value = yjlt300sys_linedata_read_entry_set_value;
         			}
        		}
      		 	else {
         			/* ipa_rn == NULL, malloc failure */
         		 	LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_inc_line() insert failed, mem full"));
         			break;
        	  	}
     		}
      		else{
       			line_rn = (struct mib_list_rootnode*)line_node->nptr;
     		}
   		}
	}
	
	/* enable getnext traversal on filled table */
	yjlt300sys_linedata_read_tbl.maxlength = 1;
}

void snmp_inc_meter(u8_t port)
{
	struct mib_list_rootnode *meter_rn;
	struct mib_list_node *meter_node ;
	s32_t meteridx[2];
	meteridx[0] = get_net_id();
	meteridx[1] = port + 1;
	u8_t level,tree;
		
	for (tree = 0; tree < 2; tree ++){   
		if (tree == 0){
			meter_rn = &yjlt300sys_meterdata_read_root;
		}
		else{
			meter_rn = &yjlt300sys_meterevent_read_root;
    		}
		for (level = 0; level < 2; level++){
			meter_node = NULL;
			snmp_mib_node_insert(meter_rn, meteridx[level], &meter_node);
			if ((level != 1) && (meter_node != NULL)){
				if (meter_node->nptr == NULL){
					meter_rn = snmp_mib_lrn_alloc();
       				meter_node->nptr = (struct mib_node*)meter_rn;
					if (meter_rn != NULL){
						if (level == 0){
							 if(tree == 0){
								    meter_rn->get_object_def = yjlt300sys_meterdata_read_entry_get_object_def;
            						meter_rn->get_value = yjlt300sys_meterdata_read_entry_get_value;
           						 	meter_rn->set_test = yjlt300sys_meterdata_read_entry_set_test;
           						 	meter_rn->set_value = yjlt300sys_meterdata_read_entry_set_value;
							  }
							 else{
#if EM_EVENT_SUPPERT
									meter_rn->get_object_def = yjlt300sys_meterevent_read_entry_get_object_def;
            						meter_rn->get_value = yjlt300sys_meterevent_read_entry_get_value;
           						 	meter_rn->set_test = yjlt300sys_meterevent_read_entry_set_test;
           						 	meter_rn->set_value = yjlt300sys_meterevent_read_entry_set_value;
#endif
							 }
         				}
        			}
      		 		else {
         				/* ipa_rn == NULL, malloc failure */
         		 		LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_inc_pttw_ipaddridx_tree() insert failed, mem full"));
         				break;
        	  		}
     			}
      			else{
       				meter_rn = (struct mib_list_rootnode*)meter_node->nptr;
     			}
   			}
		}
	}
	/* enable getnext traversal on filled table */
	yjlt300sys_meterdata_read_tbl.maxlength = 1;
	yjlt300sys_meterevent_read_tbl.maxlength = 1;
}
void snmp_inc_meter_freeze(u8_t port)
{
	struct mib_list_rootnode *meter_rn;
	struct mib_list_node *meter_node ;
	s32_t meteridx[2];
	meteridx[0] = get_net_id();
	meteridx[1] = port + 1;
	u8_t level,tree;
		
	for (tree = 0; tree < 2; tree ++){   
		if (tree == 0){
			meter_rn = &yjlt300sys_meter_momentfreezedata_read_root;
		}
		else{
			meter_rn = &yjlt300sys_meter_timingfreezedata_read_root;
    		}
		for (level = 0; level < 2; level++){
			meter_node = NULL;
			snmp_mib_node_insert(meter_rn, meteridx[level], &meter_node);
			if ((level != 1) && (meter_node != NULL)){
				if (meter_node->nptr == NULL){
					meter_rn = snmp_mib_lrn_alloc();
       				meter_node->nptr = (struct mib_node*)meter_rn;
					if (meter_rn != NULL){
						if (level == 0){
							 if(tree == 0){
								    meter_rn->get_object_def = yjlt300sys_meter_momentfreezedata_read_entry_get_object_def;
            						meter_rn->get_value = yjlt300sys_meter_momentfreezedata_read_entry_get_value;
           						 	meter_rn->set_test = yjlt300sys_meter_momentfreezedata_read_entry_set_test;
           						 	meter_rn->set_value = yjlt300sys_meter_momentfreezedata_read_entry_set_value;
							  }
							 else{
									meter_rn->get_object_def = yjlt300sys_meter_timingfreezedata_read_entry_get_object_def;
            						meter_rn->get_value = yjlt300sys_meter_timingfreezedata_read_entry_get_value;
           						 	meter_rn->set_test = yjlt300sys_meter_timingfreezedata_read_entry_set_test;
           						 	meter_rn->set_value = yjlt300sys_meter_timingfreezedata_read_entry_set_value;
							 }
         				}
        			}
      		 		else {
         				/* ipa_rn == NULL, malloc failure */
         		 		LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_inc_pttw_ipaddridx_tree() insert failed, mem full"));
         				break;
        	  		}
     			}
      			else{
       				meter_rn = (struct mib_list_rootnode*)meter_node->nptr;
     			}
   			}
		}
	}
	/* enable getnext traversal on filled table */
	yjlt300sys_meter_momentfreezedata_read_tbl.maxlength = 1;
	yjlt300sys_meter_timingfreezedata_read_tbl.maxlength = 1;
}

#if 0
void snmp_inc_master_pt(u8_t port)
{
	struct mib_list_rootnode *master_pt_rn;
	struct mib_list_node *master_pt_node ;
	s32_t masterptidx[2];
	masterptidx[0] = get_net_id();
	masterptidx[1] = port + 1;
	u8_t level;
	master_pt_rn = &yjlt300if_twcfg_root;
	
	for (level = 0; level < 2; level++){
		master_pt_node = NULL;
		snmp_mib_node_insert(master_pt_rn, masterptidx[level], &master_pt_node);
		if ((level != 1) && (master_pt_node != NULL)){
    			 if (master_pt_node->nptr == NULL){
       			 master_pt_rn = snmp_mib_lrn_alloc();
       			 master_pt_node->nptr = (struct mib_node*)master_pt_rn;
				 if (master_pt_rn != NULL){
					if (level == 0){
           					master_pt_rn->get_object_def = yjlt300if_twcfg_entry_get_object_def;
            				master_pt_rn->get_value = yjlt300if_twcfg_entry_get_value;
           					master_pt_rn->set_test = yjlt300if_twcfg_entry_set_test;
           					master_pt_rn->set_value = yjlt300if_twcfg_entry_set_value;
         				 }
        			}
      		 		else {
         					/* ipa_rn == NULL, malloc failure */
         		 			LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_inc_master_pt_ipaddridx_tree() insert failed, mem full"));
         					break;
        	  		}
     			}
      			else{
       				master_pt_rn = (struct mib_list_rootnode*)master_pt_node->nptr;
     			}
   		}
	}
	
	/* enable getnext traversal on filled table */
	yjlt300if_twcfg_tbl.maxlength = 1;
}
#endif
void snmp_inc_pttw(u8_t port, u8_t index)
{
	struct mib_list_rootnode *pt_rn;
	struct mib_list_node *pt_node ;
	u8_t level, tree;
	s32_t ptidx[3];
	ptidx[0] = get_net_id();
	ptidx[1] = port + 1;   // em index
	ptidx[2] = index + 1;   // current suport only 1 pt config
	
	for (tree = 0; tree < 2; tree ++){   
		if (tree == 0){
			pt_rn = &yjlt300if_pttwcfg_root;
		}
		else{
			pt_rn = &yjlt300sys_ptdata_read_root;
    		}
		for (level = 0; level < 3; level++){
			pt_node = NULL;
			snmp_mib_node_insert(pt_rn, ptidx[level], &pt_node);
			if ((level != 2) && (pt_node != NULL)){
    			 if (pt_node->nptr == NULL){
       				 pt_rn = snmp_mib_lrn_alloc();
       				 pt_node->nptr = (struct mib_node*)pt_rn;
					 if (pt_rn != NULL){
        				 if (level == 1){
							 if(tree == 0){
								    pt_rn->get_object_def = yjlt300if_pttwcfg_entry_get_object_def;
            						pt_rn->get_value = yjlt300if_pttwcfg_entry_get_value;
           						 	pt_rn->set_test = yjlt300if_pttwcfg_entry_set_test;
           						 	pt_rn->set_value = yjlt300if_pttwcfg_entry_set_value;
							  }
							 else{
									pt_rn->get_object_def = yjlt300sys_ptdata_read_entry_get_object_def;
            						pt_rn->get_value = yjlt300sys_ptdata_read_entry_get_value;
           						 	pt_rn->set_test = yjlt300sys_ptdata_read_entry_set_test;
           						 	pt_rn->set_value = yjlt300sys_ptdata_read_entry_set_value;
							 }
         				}
					}
      		 		else {
         				/* ipa_rn == NULL, malloc failure */
         		 		LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_inc_pttw_ipaddridx_tree() insert failed, mem full"));
         				break;
        	  		}
     			}
      			else{
       				pt_rn = (struct mib_list_rootnode*)pt_node->nptr;
     			}
   			}
		}
	}
	/* enable getnext traversal on filled table */
	yjlt300if_pttwcfg_tbl.maxlength = 1;
	yjlt300sys_ptdata_read_tbl.maxlength = 1;
}

void snmp_inc_cttw(u8_t port, u8_t index)
{
	struct mib_list_rootnode *ct_rn;
	struct mib_list_node *ct_node ;
	u8_t level, tree;
	s32_t ctidx[3];
	ctidx[0] = get_net_id();
	ctidx[1] = port + 1;   // em index
	ctidx[2] = index + 1;   // current suport only 2 ct config
	
	for (tree = 0; tree < 2; tree ++){   
		if (tree == 0){
			
			ct_rn = &yjlt300if_cttwcfg_root;
		}
		else{
     			ct_rn = &yjlt300sys_ctdata_read_root;
    		}
		for (level = 0; level < 3; level++){
			ct_node = NULL;
			snmp_mib_node_insert(ct_rn, ctidx[level], &ct_node);
			if ((level != 2) && (ct_node != NULL)){
    			 	if (ct_node->nptr == NULL)
      				{
       				 ct_rn = snmp_mib_lrn_alloc();
       				 ct_node->nptr = (struct mib_node*)ct_rn;
      					 if (ct_rn != NULL)
      					 {
        					  if (level == 1){
							 if(tree == 0){
								ct_rn->get_object_def = yjlt300if_cttwcfg_entry_get_object_def;
            						ct_rn->get_value = yjlt300if_cttwcfg_entry_get_value;
           						 	ct_rn->set_test = yjlt300if_cttwcfg_entry_set_test;
           						 	ct_rn->set_value = yjlt300if_cttwcfg_entry_set_value;
							  }
							 else{
								ct_rn->get_object_def = yjlt300sys_ctdata_read_entry_get_object_def;
            						ct_rn->get_value = yjlt300sys_ctdata_read_entry_get_value;
           						 	ct_rn->set_test = yjlt300sys_ctdata_read_entry_set_test;
           						 	ct_rn->set_value = yjlt300sys_ctdata_read_entry_set_value;
							 }
         					  }
        				 }
      		 			 else {
         					  /* ipa_rn == NULL, malloc failure */
         		 			 LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_inc_cttw_ipaddridx_tree() insert failed, mem full"));
         					 break;
        	  			}
     				}
      				else{
       				ct_rn = (struct mib_list_rootnode*)ct_node->nptr;
     				}
   			}
		}
	}
	/* enable getnext traversal on filled table */
	yjlt300if_cttwcfg_tbl.maxlength = 1;
	yjlt300sys_ctdata_read_tbl.maxlength = 1;
}

void snmp_inc_data_analysis(u8_t port)
{
	struct mib_list_rootnode *line_rn;
	struct mib_list_node *line_node ;
	s32_t lineptidx[2];
	lineptidx[0] = get_net_id();
	lineptidx[1] = port + 1;
	u8_t level;
	line_rn = &yjlt300sys_data_analysis_root;
	
	for (level = 0; level < 2; level++){
		line_node = NULL;
		snmp_mib_node_insert(line_rn, lineptidx[level], &line_node);
		if ((level != 1) && (line_node != NULL)){
    		if (line_node->nptr == NULL){
       			line_rn = snmp_mib_lrn_alloc();
       			line_node->nptr = (struct mib_node*)line_rn;
				if (line_rn != NULL){
					if (level == 0){
           					line_rn->get_object_def = yjlt300sys_data_analysis_entry_get_object_def;
            				line_rn->get_value = yjlt300sys_data_analysis_entry_get_value;
           					line_rn->set_test = yjlt300sys_data_analysis_entry_set_test;
           					line_rn->set_value = yjlt300sys_data_analysis_entry_set_value;
         			}
        		}
      		 	else {
         			/* ipa_rn == NULL, malloc failure */
         		 	LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_inc_data_analysis() insert failed, mem full"));
         			break;
        	  	}
     		}
      		else{
       			line_rn = (struct mib_list_rootnode*)line_node->nptr;
     		}
   		}
	}
	
	/* enable getnext traversal on filled table */
	yjlt300sys_data_analysis_tbl.maxlength = 1;
}

void snmp_inc_dev_thd(u8_t port)
{
	struct mib_list_rootnode *line_rn;
	struct mib_list_node *line_node ;
	s32_t lineptidx[2];
	lineptidx[0] = get_net_id();
	lineptidx[1] = port + 1;
	u8_t level;
	line_rn = &yjlt300sys_dev_thd_root;
	
	for (level = 0; level < 2; level++){
		line_node = NULL;
		snmp_mib_node_insert(line_rn, lineptidx[level], &line_node);
		if ((level != 1) && (line_node != NULL)){
    		if (line_node->nptr == NULL){
       			line_rn = snmp_mib_lrn_alloc();
       			line_node->nptr = (struct mib_node*)line_rn;
				if (line_rn != NULL){
					if (level == 0){
           					line_rn->get_object_def = yjlt300sys_dev_thd_entry_get_object_def;
            				line_rn->get_value = yjlt300sys_dev_thd_entry_get_value;
           					line_rn->set_test = yjlt300sys_dev_thd_entry_set_test;
           					line_rn->set_value = yjlt300sys_dev_thd_entry_set_value;
         			}
        		}
      		 	else {
         			/* ipa_rn == NULL, malloc failure */
         		 	LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_inc_dev_thd() insert failed, mem full"));
         			break;
        	  	}
     		}
      		else{
       			line_rn = (struct mib_list_rootnode*)line_node->nptr;
     		}
   		}
	}
	
	/* enable getnext traversal on filled table */
	yjlt300sys_dev_thd_tbl.maxlength = 1;
}

void snmp_inc_thdexc_alarm(u8_t port)
{
	struct mib_list_rootnode *line_rn;
	struct mib_list_node *line_node ;
	s32_t lineptidx[2];
	lineptidx[0] = get_net_id();
	lineptidx[1] = port + 1;
	u8_t level;
	line_rn = &yjlt300sys_thdexc_alarm_root;
	
	for (level = 0; level < 2; level++){
		line_node = NULL;
		snmp_mib_node_insert(line_rn, lineptidx[level], &line_node);
		if ((level != 1) && (line_node != NULL)){
    		if (line_node->nptr == NULL){
       			line_rn = snmp_mib_lrn_alloc();
       			line_node->nptr = (struct mib_node*)line_rn;
				if (line_rn != NULL){
					if (level == 0){
           					line_rn->get_object_def = yjlt300sys_thdexc_alarm_entry_get_object_def;
            				line_rn->get_value = yjlt300sys_thdexc_alarm_entry_get_value;
           					line_rn->set_test = yjlt300sys_thdexc_alarm_entry_set_test;
           					line_rn->set_value = yjlt300sys_thdexc_alarm_entry_set_value;
         			}
        		}
      		 	else {
         			/* ipa_rn == NULL, malloc failure */
         		 	LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_inc_thdexc_alarm() insert failed, mem full"));
         			break;
        	  	}
     		}
      		else{
       			line_rn = (struct mib_list_rootnode*)line_node->nptr;
     		}
   		}
	}
	
	/* enable getnext traversal on filled table */
	yjlt300sys_thdexc_alarm_tbl.maxlength = 1;
}

/*************************时间设置*****************************/
rt_uint32_t test_strtoul(char *str, int base)
{
	char *stopstring;
	char strtemp[128];

	strncpy(strtemp, str, 128);
	strtemp[127] = 0;
	return (simple_strtoul(strtemp, &stopstring, base));
}

void date2string(struct ammeter_time *Time, char *buffer)
{
	int length = 0;

	length += rt_sprintf(buffer+length, "%04d-", Time->year);
	length += rt_sprintf(buffer+length, "%02d-", Time->month);
	length += rt_sprintf(buffer+length, "%02d", Time->day);
	length += rt_sprintf(buffer+length, "%02d:", Time->hour);
	length += rt_sprintf(buffer+length, "%02d:", Time->minite);
	length += rt_sprintf(buffer+length, "%02d", Time->seconds);
}

/*************************configure the system date and time*****************************/
#if 1
void string2date(char *buffer, struct tm *Time)
{
	char *tok;
	char temp[32];

	strncpy(temp, buffer, 32);
	temp[31] = 0;
	//Time->MA_Yday = 0;
	//Time->MA_Wday = 0;
	
	tok = strtok(temp, "-");
	if (tok) {
		Time->tm_year = test_strtoul(tok, 10);
		tok = strtok(NULL, "-");
		if (tok) {
			Time->tm_mon = test_strtoul(tok, 10);
			tok = strtok(NULL, "-");
			if (tok) {
				Time->tm_mday = test_strtoul(tok, 10);
				tok = strtok(temp, ":");
	            if (tok) {
		            Time->tm_hour = test_strtoul(tok, 10);
		            tok = strtok(NULL, ":");
		             if (tok) {
			             Time->tm_min = test_strtoul(tok, 10);
			             tok = strtok(NULL, ":");
			             if (tok) {
				             Time->tm_sec = test_strtoul(tok, 10);
			             }
		             }
	            }
			}
		}
	}
	
}

void string_to_date(char *buffer, struct ammeter_time *Time)
{
	char *tok;
	char temp[32];

	strncpy(temp, buffer, 32);
	temp[31] = 0;
	
	tok = strtok(temp, "-");
	if (tok) {
		Time->year = test_strtoul(tok, 10);
		tok = strtok(NULL, "-");
		if (tok) {
			Time->month = test_strtoul(tok, 10);
			tok = strtok(NULL, "-");
			if (tok) {
				Time->day = test_strtoul(tok, 10);
				tok = strtok(temp, ":");
	            if (tok) {
		            Time->hour = test_strtoul(tok, 10);
		            tok = strtok(NULL, ":");
		             if (tok) {
			             Time->minite = test_strtoul(tok, 10);
			             tok = strtok(NULL, ":");
			             if (tok) {
				             Time->seconds = test_strtoul(tok, 10);
			             }
		             }
	            }
			}
		}
	}
	
}
#endif


/**
 * Initialises this private MIB before use.
 */
void lwip_privmib_init(void)
{
	//struct snmp_community_st snmp_commu;
    u8_t i,j,k;
	   
	printf_syn("SNMP private MIB start.\n");

	//read_syscfgdata_tbl(SYSCFGDATA_TBL_SNMP_COMMU, 0, &snmp_commu);
	//rt_strncpy(snmp_publiccommunity, snmp_commu.get_commu, SNMP_COMMUNITY_LEN_MAX);


	/* Set SNMP Trap destination */
    snmp_trap_init();
	
	snmp_insert_netidx_tree();
	for(i = 0; i < RS485_INTERFACE_NUM; i++){
		snmp_inc_rs485( i );
	}
	for(i = 0; i < IP_INTERFACE_NUM; i++){
		snmp_inc_eth(i);
	}
	for(k = 0; k < NUM_OF_COLLECT_EM_MAX; k++){
		snmp_inc_meter( k );
		snmp_inc_meter_freeze( k );
		snmp_inc_line( k );
		snmp_inc_pttw( k, 0);
		for(j = 0; j < 2; j++){
			snmp_inc_cttw( k, j);
		}
		snmp_inc_data_analysis( k );
		snmp_inc_dev_thd( k );
		snmp_inc_thdexc_alarm( k );
	}
	return;
}

/*
 ******************************************************************************
 * smart control cabinet
 * creat by David
 *
 ******************************************************************************
 */

static void
yjlt300if_485cfg_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
{
	/* return to object name, adding index depth (1) */
	ident_len += 2;
	ident     -= 2;

	if (ident_len == 3) {
		u8_t id;

		od->id_inst_len = ident_len;
		od->id_inst_ptr = ident;

		LWIP_ASSERT("invalid id", (ident[0] >= 0) && (ident[0] <= 0xff));

		od->instance = MIB_OBJECT_TAB;
		id = (u8_t)ident[0];
		switch (id) {
		case LT300IF_RS485_IF_INDEX_ID: /* 485 interface index*/
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len = sizeof(u32_t);
		    break;
		case LT300IF_RS485_RATE_CTRL_ID:   /*速率配置*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type =  (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = sizeof(u32_t);
			break;

		case LT300IF_RS485_DATA_BITS_ID:   /*数据位 */
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type =  (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = sizeof(u32_t);
			break;
		case LT300IF_RS485_PARITY_BITS_ID: /* 校验位*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type =  (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = sizeof(u32_t);
		    break;
		case LT300IF_RS485_STOP_BITS_ID: /* 停止位 */
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type =  (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = sizeof(u32_t);
		    break;
		case LT300IF_RS485_CURRENT_ALARM_ID: /* 接口告警类型*/
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type =  (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = sizeof(u32_t);
		    break;
		case LT300IF_RS485_ALARM_MASK_ID:   /* 告警屏蔽*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type =  (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = sizeof(u32_t);
		    break;
		default:
			LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no such object\n"));
			od->instance = MIB_OBJECT_NONE;
			break;
		}
	} else {
		LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no scalar\n"));
		od->instance = MIB_OBJECT_NONE;
	}

	return;
}

static void yjlt300if_485cfg_entry_get_value(struct obj_def *od, u16_t len, void *value)
{
	u32_t *uint_ptr = (u32_t *)value;
	u8_t id;
	u8_t rs485pid;
	struct rs232_param rs232cfg;
	
	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));
	
	id = (u8_t)od->id_inst_ptr[0];
	rs485pid = (u8_t)od->id_inst_ptr[2];
	read_syscfgdata_tbl(SYSCFGDATA_TBL_RS232CFG, rs485pid, &rs232cfg);
	
	switch (id) {
	case LT300IF_RS485_IF_INDEX_ID: 
		*uint_ptr = rs485pid;
		break;

	case LT300IF_RS485_RATE_CTRL_ID:   /*speed*/
		*uint_ptr = rs232cfg.baudrate;
	    break;

	case LT300IF_RS485_DATA_BITS_ID:   /*data bits*/
		*uint_ptr = rs232cfg.databits;
		break;
	case LT300IF_RS485_PARITY_BITS_ID:  /* parity bits */
		*uint_ptr = rs232cfg.paritybit;
		break;
	case LT300IF_RS485_STOP_BITS_ID:    /* stop bits */
		*uint_ptr = rs232cfg.stopbits;
		break;
	case LT300IF_RS485_CURRENT_ALARM_ID: /* alarm status */
		*uint_ptr = rs232cfg.cur_alarm;
		break;
	case LT300IF_RS485_ALARM_MASK_ID:     /* alarm mask */
		*uint_ptr = rs232cfg.alarm_mask;
		break;
	default: /*  */
		break;

	}

	return;
}

/**
 * Test snmp object value before setting.
 *
 * @param od is the object definition
 * @param len return value space (in bytes)
 * @param value points to (varbind) space to copy value from.
 */
static u8_t yjlt300if_485cfg_entry_set_test(struct obj_def *od, u16_t len, void *value)
{
	u8_t id, set_ok;

	LWIP_UNUSED_ARG(len);
	set_ok = 0;
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	id = (u8_t)od->id_inst_ptr[0];
	PRIVMIB_DEBUG(("line:%d, id:%d, ip:%d.%d.%d.%d\n", __LINE__, id, *pch, *(pch+1), *(pch+2), *(pch+3)));
	switch (id) {
	case LT300IF_RS485_RATE_CTRL_ID:
	case LT300IF_RS485_DATA_BITS_ID:
	case LT300IF_RS485_PARITY_BITS_ID:
	case LT300IF_RS485_STOP_BITS_ID:
	case LT300IF_RS485_ALARM_MASK_ID:
		set_ok = 1;
		break;
	
	default:
		break;
	}

	return set_ok;
}

static void yjlt300if_485cfg_entry_set_value(struct obj_def *od, u16_t len, void *value)
{
	u8_t id;
	u8_t rs485pid;
	struct rs232_param rs232cfg;

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	id = (u8_t)od->id_inst_ptr[0];
	rs485pid = (u8_t)od->id_inst_ptr[2];
	read_syscfgdata_tbl(SYSCFGDATA_TBL_RS232CFG, rs485pid, &rs232cfg); /* 可以节省程序存储空间 */
	
		/* @todo @fixme: which kind of pointer is 'value'? s32_t or u8_t??? */
		switch (id) {
		case LT300IF_RS485_RATE_CTRL_ID:
		//	UART_if_Set( *((u32_t *)value), rs232cfg.databits, rs232cfg.paritybit, rs232cfg.stopbits, rs485pid);
			rs232cfg.baudrate = *((u32_t *)value);
			do_set_rs232cfg(&rs232cfg, rs485pid);
			break;
	
		case LT300IF_RS485_DATA_BITS_ID:
			rs232cfg.databits = *((u32_t *)value);
			do_set_rs232cfg(&rs232cfg, rs485pid);
			break;
	
		case LT300IF_RS485_PARITY_BITS_ID:
			rs232cfg.paritybit = *((u32_t *)value);
			do_set_rs232cfg(&rs232cfg, rs485pid);
			break;
		
		case LT300IF_RS485_STOP_BITS_ID:
			rs232cfg.stopbits = *((u32_t *)value);
			do_set_rs232cfg(&rs232cfg, rs485pid);
			break;
		case LT300IF_RS485_ALARM_MASK_ID:
			rs232cfg.alarm_mask = *((u32_t *)value);  // hongbin E
			break;
	
		default:
			break;
		}
		
	write_syscfgdata_tbl(SYSCFGDATA_TBL_IPCFG, rs485pid, &rs232cfg);
		
	PRIVMIB_DEBUG(("line:%d, id:%d, ip:%d.%d.%d.%d\n", __LINE__, id, *pch, *(pch+1), *(pch+2), *(pch+3)));

	return;
}

static void
yjlt300if_ethcfg_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
{
	/* return to object name, adding index depth (1) */
	ident_len += 2;
	ident     -= 2;

	if (ident_len == 3) {
		u8_t id;

		od->id_inst_len = ident_len;
		od->id_inst_ptr = ident;

		LWIP_ASSERT("invalid id", (ident[0] >= 0) && (ident[0] <= 0xff));
		id = (u8_t)ident[0];
		
		od->instance = MIB_OBJECT_TAB;
		switch (id) {
		case LT300IF_ETH_IF_INDEX_ID: /* 485 interface index*/
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type =  (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len = sizeof(u32_t);
		    break;
		case LT300IF_ETH_CFG_INFO_ID:   /*速率配置*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type =  (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len = 5;
			break;

		case LT300IF_ETH_CUR_CFG_INFO_ID:   /*数据位 */
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type =  (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len = 5;
			break;
		case LT300IF_ETH_PERFORM_COUNTER_ID: /* 校验位*/
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type =  (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len = 56;
		    break;
		case LT300IF_ETH_CURRENT_ALARM_ID: /* 停止位 */
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = 4;
		    break;

		case LT300IF_ETH_ALARM_MASK_ID:   /* 告警屏蔽*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = sizeof(u32_t);
		    break;
		default:
			LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no such object\n"));
			od->instance = MIB_OBJECT_NONE;
			break;
		}
	} else {
		LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no scalar\n"));
		od->instance = MIB_OBJECT_NONE;
	}

	return;
}

static void yjlt300if_ethcfg_entry_get_value(struct obj_def *od, u16_t len, void *value)
{
	u32_t *uint_ptr = value;
	u8_t id;
	
	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));
	
	id = (u8_t)od->id_inst_ptr[0];
	//read_syscfgdata_tbl(SYSCFGDATA_TBL_RS232CFG, rs485pid-1, &rs232cfg);
	
	switch (id) {
	case LT300IF_ETH_IF_INDEX_ID: 
		*uint_ptr = od->id_inst_ptr[2];
		break;

	case LT300IF_ETH_CFG_INFO_ID:   /*speed*/
		//*uint_ptr = rs232cfg.baudrate;
		break;

	case LT300IF_ETH_CUR_CFG_INFO_ID:   /*data bits*/
		//*uint_ptr = rs232cfg.databits;
		break;
	case LT300IF_ETH_PERFORM_COUNTER_ID:  /* parity bits */
		//*uint_ptr = rs232cfg.paritybit;
		break;
	case LT300IF_ETH_CURRENT_ALARM_ID: /* alarm status */
		//*uint_ptr = rs232cfg.cur_alarm;
		break;
	case LT300IF_ETH_ALARM_MASK_ID:     /* alarm mask */
		//*uint_ptr = rs232cfg.alarm_mask;
		break;
	default: /*  */
		break;

	}

	return;
}

/**
 * Test snmp object value before setting.
 *
 * @param od is the object definition
 * @param len return value space (in bytes)
 * @param value points to (varbind) space to copy value from.
 */
static u8_t yjlt300if_ethcfg_entry_set_test(struct obj_def *od, u16_t len, void *value)
{
	u8_t id, set_ok;

	LWIP_UNUSED_ARG(len);
	set_ok = 0;
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	id = (u8_t)od->id_inst_ptr[0];
	PRIVMIB_DEBUG(("line:%d, id:%d, ip:%d.%d.%d.%d\n", __LINE__, id, *pch, *(pch+1), *(pch+2), *(pch+3)));
	switch (id) {
	case LT300IF_ETH_CFG_INFO_ID:
	case LT300IF_ETH_ALARM_MASK_ID:
		set_ok = 1;
		break;
	
	default:
		break;
	}

	return set_ok;
}

static void yjlt300if_ethcfg_entry_set_value(struct obj_def *od, u16_t len, void *value)
{
	u8_t id;


	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	id = (u8_t)od->id_inst_ptr[0];
	
	
		/* @todo @fixme: which kind of pointer is 'value'? s32_t or u8_t??? */
		switch (id) {
		case LT300IF_ETH_CFG_INFO_ID:
			
			break;
	
		case LT300IF_ETH_ALARM_MASK_ID:
			
			break;

	
		default:
			break;
		}
		
		
		PRIVMIB_DEBUG(("line:%d, id:%d, ip:%d.%d.%d.%d\n", __LINE__, id, *pch, *(pch+1), *(pch+2), *(pch+3)));

	return;
}
	
/*
 ******************************************************************************
 * smart control cabinet
 * creat by David
 *
 ******************************************************************************
 */
#if 0

static void
yjlt300if_twcfg_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
{
	/* return to object name, adding index depth (1) */
	ident_len += 2;
	ident     -= 2;

	if (ident_len == 3) {
		u8_t id;

		od->id_inst_len = ident_len;
		od->id_inst_ptr = ident;

		LWIP_ASSERT("invalid id", (ident[0] >= 0) && (ident[0] <= 0xff));

		od->instance = MIB_OBJECT_TAB;
		id = (u8_t)ident[0];
		switch (id) {
		case LT300IF_TWCFG_INDEX_ID: /* 485 interface index*/
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV| SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len = sizeof(u32_t);
		    break;	
		case LT300IF_TWCFG_SNID_ID: /* 主节点SN 号*/
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len = 12;
		    break;
		case LT300IF_TWCFG_MODULATE_TYPE_ID:   /*调制方式*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = sizeof(u32_t);
			break;

		case LT300IF_TWCFG_MANCHESTER_CODE_ID:   /*曼彻斯特编码*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = sizeof(u32_t);
			break;
		case LT300IF_TWCFG_CENTER_FRQ_ID: /* 中心频率*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = sizeof(u32_t);
		    break;
		case LT300IF_TWCFG_CHANEL_NUM_ID: /* 信道数目*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = sizeof(u32_t);
		    break;
		case LT300IF_TWCFG_TX_DEV_ID: /*发送频偏*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = sizeof(u32_t);
		    break;
		case LT300IF_TWCFG_RX_DEV_ID:   /* 接收频偏*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = sizeof(u32_t);
		    break;
		case LT300IF_TWCFG_RX_BW_ID:   /* 接收带宽*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = sizeof(u32_t);
		    break;
		case LT300IF_TWCFG_TX_RATA_ID:   /* 接收发送速率*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = sizeof(u32_t);
		    break;
		case LT300IF_TWCFG_RX_RATA_ID:   /* 接收速率*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = sizeof(u32_t);
		    break;
		case LT300IF_TWCFG_TX_POWER_ID:   /* 发送功率*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = sizeof(u32_t);
		    break;
		case LT300IF_TWCFG_RX_SENSITIVITY_ID:   /* 接收灵敏度*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = sizeof(u32_t);
		    break;
		case LT300IF_TWCFG_CUR_ALARM_ID:   /* 当前告警*/
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = 4;
		    break;
		case LT300IF_TWCFG_ALARM_MASK_ID:   /* 告警屏蔽*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = 4;
		    break;
		default:
			LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no such object\n"));
			od->instance = MIB_OBJECT_NONE;
			break;
		}
	} else {
		LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no scalar\n"));
		od->instance = MIB_OBJECT_NONE;
	}

	return;
}

static void yjlt300if_twcfg_entry_get_value(struct obj_def *od, u16_t len, void *value)
{
	u32_t *uint_ptr = value;
	char *pch = value;
	u8_t id;
	//u8_t rs485pid;
	struct tinywireless_if_info_st twcfg;
	
	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));
	
	id = (u8_t)od->id_inst_ptr[0];
	
	read_syscfgdata_tbl(SYSCFGDATA_TBL_TW_INFO, 0, &twcfg);
	
	switch (id) {
	case LT300IF_TWCFG_INDEX_ID: 
		*uint_ptr = od->id_inst_ptr[2];
		break;	

	case LT300IF_TWCFG_SNID_ID: 
		//get_dev_sn_em_sn(SDT_MASTER_PT, pch, DEV_SN_BUF_CHARS_NUM_MAX);
		get_rfmaster_sn(pch, od->id_inst_ptr[2], DEV_SN_BUF_CHARS_NUM_MAX);
		break;

	case LT300IF_TWCFG_MODULATE_TYPE_ID:   /*speed*/
		*uint_ptr = twcfg.mod_type;
	    break;

	case LT300IF_TWCFG_MANCHESTER_CODE_ID:   /*data bits*/
		*uint_ptr = twcfg.mch_code;
		break;
	case LT300IF_TWCFG_CENTER_FRQ_ID:  /* parity bits */
		*uint_ptr = twcfg.center_freq;
		break;
	case LT300IF_TWCFG_CHANEL_NUM_ID:    /* stop bits */
		*uint_ptr = twcfg.channal_num;
		break;
	case LT300IF_TWCFG_TX_DEV_ID: /* alarm status */
		*uint_ptr = twcfg.Tx_dev;
		break;
	case LT300IF_TWCFG_RX_DEV_ID:     /* alarm mask */
		*uint_ptr = twcfg.Rx_dev;
		break;
	case LT300IF_TWCFG_RX_BW_ID:  /* parity bits */
		*uint_ptr = twcfg.Rx_bw;
		break;
	case LT300IF_TWCFG_TX_RATA_ID:    /* stop bits */
		*uint_ptr = twcfg.Tx_rate;
		break;
	case LT300IF_TWCFG_RX_RATA_ID: /* alarm status */
	    *uint_ptr = twcfg.Rx_rate;
		break;
	case LT300IF_TWCFG_TX_POWER_ID:     /* alarm mask */
		*uint_ptr = twcfg.Tx_power;
		break;
	case LT300IF_TWCFG_RX_SENSITIVITY_ID:    /* stop bits */
		*uint_ptr = twcfg.Rx_sensitivity;
		break;
	case LT300IF_TWCFG_CUR_ALARM_ID: /* alarm status */
		*uint_ptr = twcfg.cur_alarm;
		break;
	case LT300IF_TWCFG_ALARM_MASK_ID:     /* alarm mask */
		*uint_ptr = twcfg.alarm_mask;
		break;
	default: /*  */
		break;

	}

	return;
}

/**
 * Test snmp object value before setting.
 *
 * @param od is the object definition
 * @param len return value space (in bytes)
 * @param value points to (varbind) space to copy value from.
 */
static u8_t yjlt300if_twcfg_entry_set_test(struct obj_def *od, u16_t len, void *value)
{
	u8_t id, set_ok;

	LWIP_UNUSED_ARG(len);
	set_ok = 0;
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	id = (u8_t)od->id_inst_ptr[0];
	PRIVMIB_DEBUG(("line:%d, id:%d, ip:%d.%d.%d.%d\n", __LINE__, id, *pch, *(pch+1), *(pch+2), *(pch+3)));
	switch (id) {
	case LT300IF_TWCFG_MODULATE_TYPE_ID:
	case LT300IF_TWCFG_MANCHESTER_CODE_ID:
	case LT300IF_TWCFG_CENTER_FRQ_ID:
	case LT300IF_TWCFG_CHANEL_NUM_ID:
	case LT300IF_TWCFG_TX_DEV_ID:
	case LT300IF_TWCFG_RX_DEV_ID:
	case LT300IF_TWCFG_RX_BW_ID:
	case LT300IF_TWCFG_TX_RATA_ID:
	case LT300IF_TWCFG_RX_RATA_ID:
	case LT300IF_TWCFG_TX_POWER_ID:
	case LT300IF_TWCFG_RX_SENSITIVITY_ID:
	case LT300IF_TWCFG_ALARM_MASK_ID:
		set_ok = 1;
		break;
	
	default:
		break;
	}

	return set_ok;
}

static void yjlt300if_twcfg_entry_set_value(struct obj_def *od, u16_t len, void *value)
{
	u32_t *uint_ptr = value;
	//char  *pch      = value;
	u8_t id;
	struct tinywireless_if_info_st twcfg;
	
	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	id = (u8_t)od->id_inst_ptr[0];
	read_syscfgdata_tbl(SYSCFGDATA_TBL_TW_INFO,0, &twcfg); /* 可以节省程序存储空间 */
	
	/* @todo @fixme: which kind of pointer is 'value'? s32_t or u8_t??? */
	switch (id) {
/*	case LT300IF_TWCFG_SNID_ID:
		rt_memcpy(&twcfg.sn_id, pch, sizeof(twcfg.sn_id));
		break;*/

	case LT300IF_TWCFG_MODULATE_TYPE_ID:
		twcfg.mod_type = *uint_ptr; 
		break;

	case LT300IF_TWCFG_MANCHESTER_CODE_ID:
		twcfg.mch_code = *uint_ptr;
		break;

	case LT300IF_TWCFG_CENTER_FRQ_ID:
		set_rf_param(*uint_ptr, twcfg.Tx_rate, twcfg.Tx_dev, twcfg.Rx_rate, twcfg.Rx_dev) ;
		twcfg.center_freq = *uint_ptr;
		break;
	case LT300IF_TWCFG_CHANEL_NUM_ID:
		twcfg.channal_num = *uint_ptr;
		break;
	case LT300IF_TWCFG_TX_DEV_ID:
		set_rf_param(twcfg.center_freq, twcfg.Tx_rate, *uint_ptr, twcfg.Rx_rate, twcfg.Rx_dev) ;
		twcfg.Tx_dev = *uint_ptr;
		break;

	case LT300IF_TWCFG_RX_DEV_ID:
		set_rf_param(twcfg.center_freq, twcfg.Tx_rate, twcfg.Tx_dev, twcfg.Rx_rate, *uint_ptr) ;
		twcfg.Rx_dev = *uint_ptr;
		break;
	
	case LT300IF_TWCFG_RX_BW_ID:
		twcfg.Rx_bw = *uint_ptr;
		break;
	case LT300IF_TWCFG_TX_RATA_ID:
		set_rf_param(twcfg.center_freq, *uint_ptr, twcfg.Tx_dev, twcfg.Rx_rate, twcfg.Rx_dev) ;
		twcfg.Tx_rate = *uint_ptr;
		break;

	case LT300IF_TWCFG_RX_RATA_ID:
		set_rf_param(twcfg.center_freq, twcfg.Tx_rate, twcfg.Tx_dev, *uint_ptr, twcfg.Rx_dev) ;
		twcfg.Rx_rate = *uint_ptr;
		break;
	
	case LT300IF_TWCFG_TX_POWER_ID:
		twcfg.Tx_power = *uint_ptr;
		break;
	case LT300IF_TWCFG_RX_SENSITIVITY_ID:
		twcfg.Rx_sensitivity = *uint_ptr;
		break;
	case LT300IF_TWCFG_ALARM_MASK_ID:
		twcfg.alarm_mask= *uint_ptr;
		break;
	default:
		break;
	}
	
	write_syscfgdata_tbl(SYSCFGDATA_TBL_TW_INFO, 0, &twcfg);
	
	PRIVMIB_DEBUG(("line:%d, id:%d, ip:%d.%d.%d.%d\n", __LINE__, id, *pch, *(pch+1), *(pch+2), *(pch+3)));

	return;
}
#endif
/*
 ******************************************************************************
 * smart control cabinet
 * creat by David
 *
 ******************************************************************************
 */

static void
yjlt300if_pttwcfg_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
{
	/* return to object name, adding index depth (1) */
	ident_len += 3;
	ident     -= 3;

	if (ident_len == 4) {
		u8_t id;

		od->id_inst_len = ident_len;
		od->id_inst_ptr = ident;

		LWIP_ASSERT("invalid id", (ident[0] >= 0) && (ident[0] <= 0xff));

		od->instance = MIB_OBJECT_TAB;
		id = (u8_t)ident[0];
		switch (id) {
		case LT300IF_PT_TWCFG_INDEX_ID: /* PT  index*/
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV| SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len = sizeof(u32_t);
		    break;
		case LT300IF_PT_TWCFG_SNID_ID: /* PT SN index*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len = DEV_SN_MODE_LEN;
		    break;
		case LT300IF_PT_TWCFG_MODULATE_TYPE_ID:   /*调制方式*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = sizeof(u32_t);
			break;

		case LT300IF_PT_TWCFG_MANCHESTER_CODE_ID:   /*曼彻斯特编码*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = sizeof(u32_t);
			break;
		case LT300IF_PT_TWCFG_CENTER_FRQ_ID: /* 中心频率*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = sizeof(u32_t);
		    break;
		case LT300IF_PT_TWCFG_CHANEL_NUM_ID: /* 信道数目*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = sizeof(u32_t);
		    break;
		case LT300IF_PT_TWCFG_TX_DEV_ID: /*发送频偏*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = sizeof(u32_t);
		    break;
		case LT300IF_PT_TWCFG_RX_DEV_ID:   /* 接收频偏*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = sizeof(u32_t);
		    break;
		case LT300IF_PT_TWCFG_RX_BW_ID:   /* 接收带宽*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = sizeof(u32_t);
		    break;
		case LT300IF_PT_TWCFG_TX_RATA_ID:   /* 接收发送速率*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = sizeof(u32_t);
		    break;
		case LT300IF_PT_TWCFG_RX_RATA_ID:   /* 接收速率*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = sizeof(u32_t);
		    break;
		case LT300IF_PT_TWCFG_TX_POWER_ID:   /* 发送功率*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len = sizeof(u32_t);
		    break;
		case LT300IF_PT_TWCFG_RX_SENSITIVITY_ID:   /* 接收灵敏度*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len = sizeof(u32_t);
		    break;
		case LT300IF_PT_TWCFG_CUR_ALARM_ID:   /* 当前告警*/
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = 4;
		    break;
		case LT300IF_PT_TWCFG_ALARM_MASK_ID:   /* 告警屏蔽*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = 4;
		    break;
		default:
			LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no such object\n"));
			od->instance = MIB_OBJECT_NONE;
			break;
		}
	} else {
		LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no scalar\n"));
		od->instance = MIB_OBJECT_NONE;
	}

	return;
}

static void yjlt300if_pttwcfg_entry_get_value(struct obj_def *od, u16_t len, void *value)
{
	u32_t *uint_ptr = (u32_t *)value;
	char *pch = (char *)value;
	u8_t id;
	//u8_t rs485pid;
	struct tinywireless_if_info_st twcfg;
	//struct electric_meter_reg_info_st amm_sn;
	
	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));
	
	id = (u8_t)od->id_inst_ptr[0];
	
	read_syscfgdata_tbl(SYSCFGDATA_TBL_TW_INFO, 0, &twcfg);
	
	switch (id) {
	case LT300IF_PT_TWCFG_INDEX_ID: 
		*uint_ptr = od->id_inst_ptr[3];
		break;	
	case LT300IF_PT_TWCFG_SNID_ID: 
		/*if (SUCC == get_em_reg_info(&amm_sn)) {
				rt_memcpy(pch, amm_sn.ptc_sn[od->id_inst_ptr[2]-1], DEV_SN_MODE_LEN);
		} else {
			printf_syn("%s(), read PT SN data tbl fail\n", __FUNCTION__);
		}*/
		get_dev_sn_em_sn(SDT_PT, pch, DEV_SN_MODE_LEN, od->id_inst_ptr[2]-1);
		break;
	case LT300IF_PT_TWCFG_MODULATE_TYPE_ID:   /*speed*/
		*uint_ptr = twcfg.mod_type;
	    break;

	case LT300IF_PT_TWCFG_MANCHESTER_CODE_ID:   /*data bits*/
		*uint_ptr = twcfg.mch_code;
		break;
	case LT300IF_PT_TWCFG_CENTER_FRQ_ID:  /* parity bits */
		*uint_ptr = twcfg.center_freq;
		break;
	case LT300IF_PT_TWCFG_CHANEL_NUM_ID:    /* stop bits */
		*uint_ptr = twcfg.channal_num;
		break;
	case LT300IF_PT_TWCFG_TX_DEV_ID: /* alarm status */
		*uint_ptr = twcfg.Tx_dev;
		break;
	case LT300IF_PT_TWCFG_RX_DEV_ID:     /* alarm mask */
		*uint_ptr = twcfg.Rx_dev;
		break;
	case LT300IF_PT_TWCFG_RX_BW_ID:  /* parity bits */
		*uint_ptr = twcfg.Rx_bw;
		break;
	case LT300IF_PT_TWCFG_TX_RATA_ID:    /* stop bits */
		*uint_ptr = twcfg.Tx_rate;
		break;
	case LT300IF_PT_TWCFG_RX_RATA_ID: /* alarm status */
	    *uint_ptr = twcfg.Rx_rate;
		break;
	case LT300IF_PT_TWCFG_TX_POWER_ID:     /* alarm mask */
		*uint_ptr = twcfg.Tx_power;
		break;
	case LT300IF_PT_TWCFG_RX_SENSITIVITY_ID:    /* stop bits */
		*uint_ptr = twcfg.Rx_sensitivity;
		break;
	case LT300IF_PT_TWCFG_CUR_ALARM_ID: /* alarm status */
		*uint_ptr = twcfg.cur_alarm;
		break;
	case LT300IF_PT_TWCFG_ALARM_MASK_ID:     /* alarm mask */
		*uint_ptr = twcfg.alarm_mask;
		break;
	default: /*  */
		break;

	}

	return;
}

/**
 * Test snmp object value before setting.
 *
 * @param od is the object definition
 * @param len return value space (in bytes)
 * @param value points to (varbind) space to copy value from.
 */
static u8_t yjlt300if_pttwcfg_entry_set_test(struct obj_def *od, u16_t len, void *value)
{
	u8_t id, set_ok;

	LWIP_UNUSED_ARG(len);
	set_ok = 0;
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	id = (u8_t)od->id_inst_ptr[0];
	PRIVMIB_DEBUG(("line:%d, id:%d, ip:%d.%d.%d.%d\n", __LINE__, id, *pch, *(pch+1), *(pch+2), *(pch+3)));
	switch (id) {
	case LT300IF_PT_TWCFG_SNID_ID: 
	case LT300IF_PT_TWCFG_MODULATE_TYPE_ID:
	case LT300IF_PT_TWCFG_MANCHESTER_CODE_ID:
	case LT300IF_PT_TWCFG_CENTER_FRQ_ID:
	case LT300IF_PT_TWCFG_CHANEL_NUM_ID:
	case LT300IF_PT_TWCFG_TX_DEV_ID:
	case LT300IF_PT_TWCFG_RX_DEV_ID:
	case LT300IF_PT_TWCFG_RX_BW_ID:
	case LT300IF_PT_TWCFG_TX_RATA_ID:
	case LT300IF_PT_TWCFG_RX_RATA_ID:
	case LT300IF_PT_TWCFG_TX_POWER_ID:
	case LT300IF_PT_TWCFG_RX_SENSITIVITY_ID:
	case LT300IF_PT_TWCFG_ALARM_MASK_ID:
		set_ok = 1;
		break;
	
	default:
		break;
	}

	return set_ok;
}

static void yjlt300if_pttwcfg_entry_set_value(struct obj_def *od, u16_t len, void *value)
{
	u32_t *uint_ptr = value;
	char  *pch      = (char  *)value;
	u8_t id;
	struct tinywireless_if_info_st twcfg;
	char em_sn[DEV_SN_MODE_LEN+1]={'\0'};
	char ct_sn[DEV_SN_MODE_LEN+1]={'\0'};
	char ct1_sn[DEV_SN_MODE_LEN+1]={'\0'};
	char port;
	
	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	id = (u8_t)od->id_inst_ptr[0];
	read_syscfgdata_tbl(SYSCFGDATA_TBL_TW_INFO,0, &twcfg); /* 可以节省程序存储空间 */
	
	/* @todo @fixme: which kind of pointer is 'value'? s32_t or u8_t??? */
	switch (id) {
	case LT300IF_PT_TWCFG_SNID_ID:
		get_dev_sn_em_sn(SDT_ELECTRIC_METER, em_sn, DEV_SN_MODE_LEN, od->id_inst_ptr[2]-1);
		get_dev_sn_em_sn(SDT_CT, ct_sn, DEV_SN_MODE_LEN, od->id_inst_ptr[2]-1);
		get_dev_sn_em_sn(SDT_CT1, ct1_sn, DEV_SN_MODE_LEN, od->id_inst_ptr[2]-1);
		get_dev_sn_em_sn(SDT_MASTER_PT, &port, DEV_SN_MODE_LEN, od->id_inst_ptr[2]-1);
		reg_em(od->id_inst_ptr[2]-1, em_sn, port, pch, ct_sn, ct1_sn);
		break;
	case LT300IF_PT_TWCFG_MODULATE_TYPE_ID:
		twcfg.mod_type = *uint_ptr;  // hongbin E
		break;

	case LT300IF_PT_TWCFG_MANCHESTER_CODE_ID:
		twcfg.mch_code = *uint_ptr;
		break;

	case LT300IF_PT_TWCFG_CENTER_FRQ_ID:
		set_rf_param(*uint_ptr, twcfg.Tx_rate, twcfg.Tx_dev, twcfg.Rx_rate, twcfg.Rx_dev) ;
		twcfg.center_freq = *uint_ptr;
		break;
	case LT300IF_PT_TWCFG_CHANEL_NUM_ID:
		twcfg.channal_num = *uint_ptr;
		break;
	case LT300IF_PT_TWCFG_TX_DEV_ID:
		set_rf_param(twcfg.center_freq, twcfg.Tx_rate, *uint_ptr, twcfg.Rx_rate, twcfg.Rx_dev) ;
		twcfg.Tx_dev = *uint_ptr;
		break;

	case LT300IF_PT_TWCFG_RX_DEV_ID:
		set_rf_param(twcfg.center_freq, twcfg.Tx_rate, twcfg.Tx_dev, twcfg.Rx_rate, *uint_ptr) ;
		twcfg.Rx_dev = *uint_ptr;
		break;
	
	case LT300IF_PT_TWCFG_RX_BW_ID:
		twcfg.Rx_bw = *uint_ptr;
		break;
	case LT300IF_PT_TWCFG_TX_RATA_ID:
		set_rf_param(twcfg.center_freq, *uint_ptr, twcfg.Tx_dev, twcfg.Rx_rate, twcfg.Rx_dev) ;
		twcfg.Tx_rate = *uint_ptr;
		break;

	case LT300IF_PT_TWCFG_RX_RATA_ID:
		set_rf_param(twcfg.center_freq, twcfg.Tx_rate, twcfg.Tx_dev, *uint_ptr, twcfg.Rx_dev) ;
		twcfg.Rx_rate = *uint_ptr;
		break;
	
	case LT300IF_PT_TWCFG_TX_POWER_ID:
		twcfg.Tx_power = *uint_ptr;
		break;
	case LT300IF_PT_TWCFG_RX_SENSITIVITY_ID:
		twcfg.Rx_sensitivity = *uint_ptr;
		break;
	case LT300IF_PT_TWCFG_ALARM_MASK_ID:
		twcfg.alarm_mask= *uint_ptr;
		break;
	default:
		break;
	}
	
	write_syscfgdata_tbl(SYSCFGDATA_TBL_TW_INFO, 0, &twcfg);
	
	PRIVMIB_DEBUG(("line:%d, id:%d, ip:%d.%d.%d.%d\n", __LINE__, id, *pch, *(pch+1), *(pch+2), *(pch+3)));

	return;
}

/*
 ******************************************************************************
 * smart control cabinet
 * creat by David
 *
 ******************************************************************************
 */

static void
yjlt300if_cttwcfg_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
{
	/* return to object name, adding index depth (1) */
	ident_len += 3;
	ident     -= 3;

	if (ident_len == 4) {
		u8_t id;

		od->id_inst_len = ident_len;
		od->id_inst_ptr = ident;

		LWIP_ASSERT("invalid id", (ident[0] >= 0) && (ident[0] <= 0xff));

		od->instance = MIB_OBJECT_TAB;
		id = (u8_t)ident[0];
		switch (id) {
		case LT300IF_CT_TWCFG_INDEX_ID: /* 485 interface index*/
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV| SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len = sizeof(u32_t);
		    break;
		case LT300IF_CT_TWCFG_SNID_ID: /* 485 interface index*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len = DEV_SN_MODE_LEN;
		    break;
		case LT300IF_CT_TWCFG_MODULATE_TYPE_ID:   /*调制方式*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = sizeof(u32_t);
			break;

		case LT300IF_CT_TWCFG_MANCHESTER_CODE_ID:   /*曼彻斯特编码*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = sizeof(u32_t);
			break;
		case LT300IF_CT_TWCFG_CENTER_FRQ_ID: /* 中心频率*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = sizeof(u32_t);
		    break;
		case LT300IF_CT_TWCFG_CHANEL_NUM_ID: /* 信道数目*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = sizeof(u32_t);
		    break;
		case LT300IF_CT_TWCFG_TX_DEV_ID: /*发送频偏*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = sizeof(u32_t);
		    break;
		case LT300IF_CT_TWCFG_RX_DEV_ID:   /* 接收频偏*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = sizeof(u32_t);
		    break;
		case LT300IF_CT_TWCFG_RX_BW_ID:   /* 接收带宽*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = sizeof(u32_t);
		    break;
		case LT300IF_CT_TWCFG_TX_RATA_ID:   /* 接收发送速率*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = sizeof(u32_t);
		    break;
		case LT300IF_CT_TWCFG_RX_RATA_ID:   /* 接收速率*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = sizeof(u32_t);
		    break;
		case LT300IF_CT_TWCFG_TX_POWER_ID:   /* 发送功率*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len = sizeof(u32_t);
		    break;
		case LT300IF_CT_TWCFG_RX_SENSITIVITY_ID:   /* 接收灵敏度*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len = sizeof(u32_t);
		    break;
		case LT300IF_CT_TWCFG_CUR_ALARM_ID:   /* 当前告警*/
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = 4;
		    break;
		case LT300IF_CT_TWCFG_ALARM_MASK_ID:   /* 告警屏蔽*/
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len = 4;
		    break;
		default:
			LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no such object\n"));
			od->instance = MIB_OBJECT_NONE;
			break;
		}
	} else {
		LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no scalar\n"));
		od->instance = MIB_OBJECT_NONE;
	}

	return;
}

static void yjlt300if_cttwcfg_entry_get_value(struct obj_def *od, u16_t len, void *value)
{
	u32_t *uint_ptr = (u32_t *)value;
	char *pch = (char *)value;
	u8_t id;
	//u8_t rs485pid;
	struct tinywireless_if_info_st twcfg;
	//struct electric_meter_reg_info_st amm_sn;
	
	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));
	
	id = (u8_t)od->id_inst_ptr[0];
	
	read_syscfgdata_tbl(SYSCFGDATA_TBL_TW_INFO, 0, &twcfg);
	
	switch (id) {
	case LT300IF_CT_TWCFG_INDEX_ID: 
		*uint_ptr = od->id_inst_ptr[3];
		break;
	case LT300IF_CT_TWCFG_SNID_ID: 
		/*if (SUCC == get_em_reg_info(&amm_sn)) {
			rt_memcpy(pch, amm_sn.ctc_sn[od->id_inst_ptr[2]-1], DEV_SN_MODE_LEN);
		} else {
			printf_syn("%s(), read CT SN data tbl fail\n", __FUNCTION__);
		}*/
		
		if(od->id_inst_ptr[3]==1){
			get_dev_sn_em_sn(SDT_CT, pch, DEV_SN_MODE_LEN, od->id_inst_ptr[2]-1);
		}else if(od->id_inst_ptr[3]== 2){
			get_dev_sn_em_sn(SDT_CT1, pch, DEV_SN_MODE_LEN, od->id_inst_ptr[2]-1);
		}
		break;

	case LT300IF_CT_TWCFG_MODULATE_TYPE_ID:   /*speed*/
		*uint_ptr = twcfg.mod_type;
	    break;

	case LT300IF_CT_TWCFG_MANCHESTER_CODE_ID:   /*data bits*/
		*uint_ptr = twcfg.mch_code;
		break;
	case LT300IF_CT_TWCFG_CENTER_FRQ_ID:  /* parity bits */
		*uint_ptr = twcfg.center_freq;
		break;
	case LT300IF_CT_TWCFG_CHANEL_NUM_ID:    /* stop bits */
		*uint_ptr = twcfg.channal_num;
		break;
	case LT300IF_CT_TWCFG_TX_DEV_ID: /* alarm status */
		*uint_ptr = twcfg.Tx_dev;
		break;
	case LT300IF_CT_TWCFG_RX_DEV_ID:     /* alarm mask */
		*uint_ptr = twcfg.Rx_dev;
		break;
	case LT300IF_CT_TWCFG_RX_BW_ID:  /* parity bits */
		*uint_ptr = twcfg.Rx_bw;
		break;
	case LT300IF_CT_TWCFG_TX_RATA_ID:    /* stop bits */
		*uint_ptr = twcfg.Tx_rate;
		break;
	case LT300IF_CT_TWCFG_RX_RATA_ID: /* alarm status */
	       *uint_ptr = twcfg.Rx_rate;
		break;
	case LT300IF_CT_TWCFG_TX_POWER_ID:     /* alarm mask */
		*uint_ptr = twcfg.Tx_power;
		break;
	case LT300IF_CT_TWCFG_RX_SENSITIVITY_ID:    /* stop bits */
		*uint_ptr = twcfg.Rx_sensitivity;
		break;
	case LT300IF_CT_TWCFG_CUR_ALARM_ID: /* alarm status */
		*uint_ptr = twcfg.cur_alarm;
		break;
	case LT300IF_CT_TWCFG_ALARM_MASK_ID:     /* alarm mask */
		*uint_ptr = twcfg.alarm_mask;
		break;
	default: /*  */
		break;

	}

	return;
}

/**
 * Test snmp object value before setting.
 *
 * @param od is the object definition
 * @param len return value space (in bytes)
 * @param value points to (varbind) space to copy value from.
 */
static u8_t yjlt300if_cttwcfg_entry_set_test(struct obj_def *od, u16_t len, void *value)
{
	u8_t id, set_ok;

	LWIP_UNUSED_ARG(len);
	set_ok = 0;
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	id = (u8_t)od->id_inst_ptr[0];
	PRIVMIB_DEBUG(("line:%d, id:%d, ip:%d.%d.%d.%d\n", __LINE__, id, *pch, *(pch+1), *(pch+2), *(pch+3)));
	switch (id) {
	case LT300IF_CT_TWCFG_SNID_ID: 	
	case LT300IF_CT_TWCFG_MODULATE_TYPE_ID:
	case LT300IF_CT_TWCFG_MANCHESTER_CODE_ID:
	case LT300IF_CT_TWCFG_CENTER_FRQ_ID:
	case LT300IF_CT_TWCFG_CHANEL_NUM_ID:
	case LT300IF_CT_TWCFG_TX_DEV_ID:
	case LT300IF_CT_TWCFG_RX_DEV_ID:
	case LT300IF_CT_TWCFG_RX_BW_ID:
	case LT300IF_CT_TWCFG_TX_RATA_ID:
	case LT300IF_CT_TWCFG_RX_RATA_ID:
	case LT300IF_CT_TWCFG_TX_POWER_ID:
	case LT300IF_CT_TWCFG_RX_SENSITIVITY_ID:
	case LT300IF_CT_TWCFG_ALARM_MASK_ID:
		set_ok = 1;
		break;
	
	default:
		break;
	}

	return set_ok;
}

static void yjlt300if_cttwcfg_entry_set_value(struct obj_def *od, u16_t len, void *value)
{
	u32_t *uint_ptr = value;
	char  *pch      = (char  *)value;
	u8_t id;
	struct tinywireless_if_info_st twcfg;
	char em_sn[DEV_SN_MODE_LEN+1]={'\0'};
	char pt_sn[DEV_SN_MODE_LEN+1]={'\0'};
	char ct_sn[DEV_SN_MODE_LEN+1]={'\0'};
	char ct1_sn[DEV_SN_MODE_LEN+1]={'\0'};
	char port;
	
	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	id = (u8_t)od->id_inst_ptr[0];
	read_syscfgdata_tbl(SYSCFGDATA_TBL_TW_INFO,0, &twcfg); /* 可以节省程序存储空间 */
	
	/* @todo @fixme: which kind of pointer is 'value'? s32_t or u8_t??? */
	switch (id) {
	case LT300IF_CT_TWCFG_SNID_ID:
		get_dev_sn_em_sn(SDT_ELECTRIC_METER, em_sn, DEV_SN_MODE_LEN, od->id_inst_ptr[2]-1);
		get_dev_sn_em_sn(SDT_PT, pt_sn, DEV_SN_MODE_LEN, od->id_inst_ptr[2]-1);
		get_dev_sn_em_sn(SDT_CT, ct_sn, DEV_SN_MODE_LEN, od->id_inst_ptr[2]-1);
		get_dev_sn_em_sn(SDT_CT1, ct1_sn, DEV_SN_MODE_LEN, od->id_inst_ptr[2]-1);
		get_dev_sn_em_sn(SDT_MASTER_PT, &port, DEV_SN_MODE_LEN, od->id_inst_ptr[2]-1);
		if(od->id_inst_ptr[3]==1){
			reg_em(od->id_inst_ptr[2]-1, em_sn, port, pt_sn, pch, ct1_sn);
		}else if(od->id_inst_ptr[3]== 2){
			reg_em(od->id_inst_ptr[2]-1, em_sn, port, pt_sn, ct_sn, pch);
		}
		break;
	case LT300IF_CT_TWCFG_MODULATE_TYPE_ID:
		twcfg.mod_type = *uint_ptr;  // hongbin E
		break;

	case LT300IF_CT_TWCFG_MANCHESTER_CODE_ID:
		twcfg.mch_code = *uint_ptr;
		break;

	case LT300IF_CT_TWCFG_CENTER_FRQ_ID:
		set_rf_param(*uint_ptr, twcfg.Tx_rate, twcfg.Tx_dev, twcfg.Rx_rate, twcfg.Rx_dev) ;
		twcfg.center_freq = *uint_ptr;
		break;
	case LT300IF_CT_TWCFG_CHANEL_NUM_ID:
		twcfg.channal_num = *uint_ptr;
		break;
	case LT300IF_CT_TWCFG_TX_DEV_ID:
		set_rf_param(twcfg.center_freq, twcfg.Tx_rate, *uint_ptr, twcfg.Rx_rate, twcfg.Rx_dev) ;
		twcfg.Tx_dev = *uint_ptr;
		break;

	case LT300IF_CT_TWCFG_RX_DEV_ID:
		set_rf_param(twcfg.center_freq, twcfg.Tx_rate, twcfg.Tx_dev, twcfg.Rx_rate, *uint_ptr) ;
		twcfg.Rx_dev = *uint_ptr;
		break;
	
	case LT300IF_CT_TWCFG_RX_BW_ID:
		twcfg.Rx_bw = *uint_ptr;
		break;
	case LT300IF_CT_TWCFG_TX_RATA_ID:
		set_rf_param(twcfg.center_freq, *uint_ptr, twcfg.Tx_dev, twcfg.Rx_rate, twcfg.Rx_dev) ;
		twcfg.Tx_rate = *uint_ptr;
		break;

	case LT300IF_CT_TWCFG_RX_RATA_ID:
		set_rf_param(twcfg.center_freq, twcfg.Tx_rate, twcfg.Tx_dev, *uint_ptr, twcfg.Rx_dev) ;
		twcfg.Rx_rate = *uint_ptr;
		break;
	
	case LT300IF_CT_TWCFG_TX_POWER_ID:
		twcfg.Tx_power = *uint_ptr;
		break;
	case LT300IF_CT_TWCFG_RX_SENSITIVITY_ID:
		twcfg.Rx_sensitivity = *uint_ptr;
		break;
	case LT300IF_CT_TWCFG_ALARM_MASK_ID:
		twcfg.alarm_mask= *uint_ptr;
		break;
	default:
		break;
	}
	
	write_syscfgdata_tbl(SYSCFGDATA_TBL_TW_INFO, 0, &twcfg);
	
	PRIVMIB_DEBUG(("line:%d, id:%d, ip:%d.%d.%d.%d\n", __LINE__, id, *pch, *(pch+1), *(pch+2), *(pch+3)));

	return;
}
/*
 ******************************************************************************
 * smart control cabinet
 * creat by David
 *
 ******************************************************************************
 */
static void yjlt300sys_netcfg_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
{
	/* return to object name, adding index depth (1) */
	ident_len += 1;
	ident     -= 1;

	if (ident_len == 2) {
		u8_t id;

		od->id_inst_len = ident_len;
		od->id_inst_ptr = ident;

		LWIP_ASSERT("invalid id", (ident[0] >= 0) && (ident[0] <= 0xff));
		id = (u8_t)ident[0];
		
		od->instance = MIB_OBJECT_TAB;
		switch (id) {
		case LT300SYS_NETCFG_LOCAL_IP_ID           :
		case LT300SYS_NETCFG_LOCAL_SUBNET_ID       :
		case LT300SYS_NETCFG_LOCAL_GW_ID           :
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_IPADDR);
			od->v_len    = 4;
			break;

		case LT300SYS_NETCFG_LOCAL_MAC_ID          :
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 6; /* string */
			break;

		case LT300SYS_NETCFG_READ_COMMUNITY_ID     :
		case LT300SYS_NETCFG_WRITE_COMMUNITY_ID    :
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = SNMP_COMMUNITY_LEN_MAX;
			break;
		case LT300SYS_NETCFG_DATE_TIME_ID    :
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 24;
			break;
		default:
			LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no such object\n"));
			od->instance = MIB_OBJECT_NONE;
			break;
		}
	} else {
		LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no scalar\n"));
		od->instance = MIB_OBJECT_NONE;
	}

	return;
}

static void yjlt300sys_netcfg_entry_get_value(struct obj_def *od, u16_t len, void *value)
{
	//u32_t *uint_ptr = value;
	char  *pch      = value;
	struct ip_param ipcfg;
	u8_t id;
	
	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_NETCFG_LOCAL_IP_ID           :{
		ip_addr_t *dst = (ip_addr_t*)value;
		read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_IPCFG, 0, &ipcfg);
		ipcfg.ipaddr.addr = lwip_htonl(ipcfg.ipaddr.addr);
		rt_memcpy(dst, (ip_addr_t *)&ipcfg.ipaddr.addr, sizeof(ip_addr_t));
	}
		break;

	case LT300SYS_NETCFG_LOCAL_SUBNET_ID       :{
		ip_addr_t *dst = (ip_addr_t*)value;
		read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_IPCFG, 0, &ipcfg);
		ipcfg.netmask.addr = lwip_htonl(ipcfg.netmask.addr);
		rt_memcpy(dst, (ip_addr_t *)&ipcfg.netmask.addr, sizeof(ip_addr_t));
	}
		break;

	case LT300SYS_NETCFG_LOCAL_GW_ID           :{
		ip_addr_t *dst = (ip_addr_t*)value;
		read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_IPCFG, 0, &ipcfg);
		ipcfg.gw.addr = lwip_htonl(ipcfg.gw.addr);
		rt_memcpy(dst, (ip_addr_t *)&ipcfg.gw.addr, sizeof(ip_addr_t));
	}
		break;

	case LT300SYS_NETCFG_LOCAL_MAC_ID          :{
		*pch = MACOCT0;
	   	*(pch+1) = MACOCT1;
		*(pch+2) = MACOCT2;
		*(pch+3) = MACOCT3;
		*(pch+4) = MACOCT4;
		*(pch+5) = MACOCT5;	
	}
		break;

		/* NOTE: 没有区分get, set, trap的community */
	case LT300SYS_NETCFG_READ_COMMUNITY_ID     :
	case LT300SYS_NETCFG_WRITE_COMMUNITY_ID    :
		rt_strncpy(value, snmp_publiccommunity, SNMP_COMMUNITY_LEN_MAX);
		break;
    case LT300SYS_NETCFG_DATE_TIME_ID    :
#if 0 /* mark by David */
		length += rt_sprintf(pch+length,  "%04d-", realtime.tm_year);
		length += rt_sprintf(pch+length,  "%02d-", realtime.tm_mon);
		length += rt_sprintf(pch+length,  "%02d ",  realtime.tm_mday);
		length += rt_sprintf(pch+length,  "%02d:", realtime.tm_hour);
		length += rt_sprintf(pch+length,  "%02d:", realtime.tm_min);
		length += rt_sprintf(pch+length,  "%02d",  realtime.tm_sec);
#endif
		break;
	default: /*  */
		break;

	}

	return;
}

static u8_t yjlt300sys_netcfg_entry_set_test(struct obj_def *od, u16_t len, void *value)
{
	u8_t id, set_ok;
	//u8_t *pch = value;

	LWIP_UNUSED_ARG(len);
	set_ok = 0;
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));


	id = (u8_t)od->id_inst_ptr[0];
	PRIVMIB_DEBUG(("line:%d, id:%d, ip:%d.%d.%d.%d\n", __LINE__, id, *pch, *(pch+1), *(pch+2), *(pch+3)));
	switch (id) {
	case LT300SYS_NETCFG_LOCAL_IP_ID           :
	case LT300SYS_NETCFG_LOCAL_SUBNET_ID       :
	case LT300SYS_NETCFG_LOCAL_GW_ID           :
	case LT300SYS_NETCFG_READ_COMMUNITY_ID     :
	case LT300SYS_NETCFG_WRITE_COMMUNITY_ID    :
	case LT300SYS_NETCFG_DATE_TIME_ID    :
		set_ok = 1;
		break;

	default:
		break;
	}

	return set_ok;
}

static void yjlt300sys_netcfg_entry_set_value(struct obj_def *od, u16_t len, void *value)
{
	u8_t id;
	//u8_t *pch = value;
	struct ip_param ipcfg;
	struct netif * netif = netif_list;

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	/* @todo @fixme: which kind of pointer is 'value'? s32_t or u8_t??? */
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_NETCFG_LOCAL_IP_ID           :{
		ip_addr_t *dst = (ip_addr_t*)value;
		//u32_t gw,mask;
		u32_t addr;
		read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_IPCFG, 0, &ipcfg);
		//gw = ntohl(ipcfg.gw.addr);
		//mask = ntohl(ipcfg.netmask.addr);
		netif_set_ipaddr(netif, dst);
		rt_memcpy(&addr, dst, sizeof(ip_addr_t));
		ipcfg.ipaddr.addr = lwip_htonl(addr);
		write_syscfgdata_tbl(SYSCFGDATA_TBL_IPCFG, 0, &ipcfg);
		//set_if("e0", ipaddr_ntoa(dst), inet_ntoa(gw), inet_ntoa(mask));
	}
		break;

	case LT300SYS_NETCFG_LOCAL_SUBNET_ID       :{
		ip_addr_t *dst = (ip_addr_t*)value;
		u32_t addr;
		read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_IPCFG, 0, &ipcfg);
		netif_set_netmask(netif, dst);
		rt_memcpy(&addr, dst, sizeof(ip_addr_t));
		ipcfg.netmask.addr = lwip_htonl(addr);
		write_syscfgdata_tbl(SYSCFGDATA_TBL_IPCFG, 0, &ipcfg);
	}
		break;

	case LT300SYS_NETCFG_LOCAL_GW_ID           :{
		ip_addr_t *dst = (ip_addr_t*)value;
		u32_t addr;
		read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_IPCFG, 0, &ipcfg);
		netif_set_gw(netif, dst);
		rt_memcpy(&addr, dst, sizeof(ip_addr_t));
		ipcfg.gw.addr = lwip_htonl(addr);
		write_syscfgdata_tbl(SYSCFGDATA_TBL_IPCFG, 0, &ipcfg);
	}
		break;

		/* NOTE: 没有区分get, set, trap的community */
	case LT300SYS_NETCFG_READ_COMMUNITY_ID    :
	case LT300SYS_NETCFG_WRITE_COMMUNITY_ID    : {
		struct snmp_community_st snmp_commu;
		read_syscfgdata_tbl(SYSCFGDATA_TBL_SNMP_COMMU, 0, &snmp_commu);
		rt_strncpy(snmp_commu.get_commu, value, SNMP_COMMUNITY_LEN_MAX);
		write_syscfgdata_tbl(SYSCFGDATA_TBL_SNMP_COMMU, 0, &snmp_commu);
	}
	break;
    case LT300SYS_NETCFG_DATE_TIME_ID    : {
#if 0 /* mark by David */
	    string2date(pch, &realtime);
		if((realtime.tm_year < 1970) || (realtime.tm_year > 2069))
			return RT_ERROR;
		if((realtime.tm_mon) < 1 || (realtime.tm_mon > 12))
			return RT_ERROR;
		if((realtime.tm_mday) < 1 || (realtime.tm_mday > GetPreMonDayNum(realtime.tm_mon, realtime.tm_year)))
			return RT_ERROR;
		Time_SetCalendarTime(realtime);
#endif
    }
	break;
	default:
		break;
	}

	if (LT300SYS_NETCFG_LOCAL_IP_ID==id || LT300SYS_NETCFG_LOCAL_SUBNET_ID==id || LT300SYS_NETCFG_LOCAL_GW_ID==id) {
		write_syscfgdata_tbl(SYSCFGDATA_TBL_IPCFG, 0, &ipcfg);
	}

	PRIVMIB_DEBUG(("line:%d, id:%d, ip:%d.%d.%d.%d\n", __LINE__, id, *pch, *(pch+1), *(pch+2), *(pch+3)));

	return;
}

/*
 * system manage
 */
static void yjlt300sys_mcfg_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
{
	/* return to object name, adding index depth (1) */
	ident_len += 1;
	ident     -= 1;

	if (ident_len == 2) {
		u8_t id;

		od->id_inst_len = ident_len;
		od->id_inst_ptr = ident;

		LWIP_ASSERT("invalid id", (ident[0] >= 0) && (ident[0] <= 0xff));
		id = (u8_t)ident[0];
	//	LWIP_DEBUGF(SNMP_MIB_DEBUG,("get_object_def mcfg.%"U16_F"\n",(u16_t)id));
		
		od->instance = MIB_OBJECT_TAB;
		switch (id) {
		case LT300SYS_MCFG_DEV_SN_ID	:
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = sizeof(s32_t);
			break;

		case LT300SYS_MCFG_SYS_DESCR_ID    :
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = lt300_SYS_DESCR_INFO_MAX_LEN;
			break;
		case LT300SYS_MCFG_DEV_VER_ID     :
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 6;
			break;

		case LT300SYS_MCFG_DEV_INFO_ID     :
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = lt300_SYS_DEV_INFO_MAX_LEN;
			break;

		case LT300SYS_MCFG_ALARM_STATUS_ID :
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len    = 4;
			break;
		case LT300SYS_METERDATA_CONNECTING_STATUS_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access	 = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len	 = sizeof(u32_t);
			break;

		case LT300SYS_MCFG_NE_ID_ID        :
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    =12;
			break;

		case LT300SYS_MCFG_SYS_DEFAULT_ID  :
		case LT300SYS_MCFG_SYS_REBOOT_ID   :
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_WRITE_ONLY;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len    = 4;
			break;

		default:
			LWIP_DEBUGF(SNMP_MIB_DEBUG,("mcfg_get_object_def: no such object\n"));
			od->instance = MIB_OBJECT_NONE;
			break;
		}
	} else {
		LWIP_DEBUGF(SNMP_MIB_DEBUG,("mcfg_get_object_def: no scalar\n"));
		od->instance = MIB_OBJECT_NONE;
	}

	return;
}

static void yjlt300sys_mcfg_entry_get_value(struct obj_def *od, u16_t len, void *value)
{
	u32_t *uint_ptr = (u32_t*)value;
	u8_t  *pch      = (u8_t*)value;
	struct m3_sys_info_st sys_info;
	u8_t id,i;
	u32_t protodata;
	u32_t meterstatus = 0;

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	read_syscfgdata_tbl(SYSCFGDATA_TBL_SYS_INFO, 0, &sys_info);

	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_MCFG_DEV_SN_ID: {	/* dev index. */
		*uint_ptr = od->id_inst_ptr[1];   //hongbin E 
		}
		break;

	case LT300SYS_MCFG_SYS_DESCR_ID: {
		//rt_strncpy(value, LT300_SYS_TYPE_DESCR, rt_strlen(LT300_SYS_TYPE_DESCR));
		struct m3_sys_info_st lt300_sys_info;
		read_syscfgdata_tbl(SYSCFGDATA_TBL_SYS_INFO, 0, &lt300_sys_info);
		rt_strncpy((char *)value, lt300_sys_info.dev_descr, sizeof(sys_info.dev_descr));
		}
		break;
	case LT300SYS_MCFG_DEV_VER_ID     :
		/* lt300_SYS_DEV_INFO_MAX_LEN */
		*pch++ = (sys_info.sw_ver>>16)&0xff;	/* Byte 1 : Soft's Version (major) */
		*pch++ = (sys_info.sw_ver>>8)&0xff;	/* Byte 2 : Soft's Version (minor) */
		*pch++ = (sys_info.sw_ver)&0xff;	/* Byte 3 : Soft's Version (revise) */
		*pch++ = M3_HW_VERSION;		/* Byte 4 : Hardware's Version(major) */
		*pch++ = M3_HW_SUBVERSION;		/* Byte 5 : Hardware's Version(minor)  */
		*pch++ = M3_HW_REVISION;		/* Byte 6 : Hardware's Version(revise) */
		break;
	case LT300SYS_MCFG_DEV_INFO_ID     :
		/* lt300_SYS_DEV_INFO_MAX_LEN */
		*pch++ = 3;				/* Byte 1 : FE Cooper port number */
		*pch++ = 0;				/* Byte 2 : FE Fiber port number  */
		*pch++ = 0;				/* Byte 3: GE Cooper port number */
		*pch++ = 0;				/* Byte 4: GE Fiber port number */
		*pch++ = 0;				/* Byte 5 : E1 port number */
		*pch++ = 2; 			/* Byte 6 : 485 port number */
		*pch++ = 0;				/* Byte 7 : USB-A port number */
		*pch++ = 0;				/* Byte 8 : USB OTG port number */
		*pch++ = 0;				/* Byte 9 : MMC port number */
		*pch++ = 1;				/* Byte 10 : terminal port number */
		break;

	case LT300SYS_MCFG_ALARM_STATUS_ID :	/* mark by David */
		*uint_ptr = 1;            //总告警 即所有告警相或
		break;
	case LT300SYS_METERDATA_CONNECTING_STATUS_ID:
		for (i = 0; i < NUM_OF_COLLECT_EM_MAX; i++) {
			get_em_proto(i, SIC_GET_EM_PROTOCAL_TYPE, &protodata);
			if (protodata != AP_PROTOCOL_UNKNOWN){
				//meterstatus |= (1 << i);
				set_bit(meterstatus, 1<<i);
			}
		}
		*uint_ptr = meterstatus;
		break;

	case LT300SYS_MCFG_NE_ID_ID:
	       get_dev_sn_em_sn(SDT_DEV, (char *)value, DEV_SN_BUF_CHARS_NUM_MAX, od->id_inst_ptr[1]);	
		break;

	default:
		break;
	}

	return;
}

static u8_t yjlt300sys_mcfg_entry_set_test(struct obj_def *od, u16_t len, void *value)
{
	u8_t id, set_ok;
	//u8_t *pch = value;

	LWIP_UNUSED_ARG(len);
	set_ok = 0;
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	id = (u8_t)od->id_inst_ptr[0];
	PRIVMIB_DEBUG(("line:%d, id:%d, ip:%d.%d.%d.%d\n", __LINE__, id, *pch, *(pch+1), *(pch+2), *(pch+3)));
	switch (id) {
	case LT300SYS_MCFG_SYS_DESCR_ID    :
	case LT300SYS_MCFG_NE_ID_ID        :
	case LT300SYS_MCFG_SYS_DEFAULT_ID  :
	case LT300SYS_MCFG_SYS_REBOOT_ID   :
#if 0
		if ((0==*sint_ptr) || (1==*sint_ptr))
			set_ok = 1;
#else
		set_ok = 1;
#endif
		break;

	default:
		break;
	}

	return set_ok;
}

static void yjlt300sys_mcfg_entry_set_value(struct obj_def *od, u16_t len, void *value)
{
	u8_t id;
	char *pch =( char *)value;

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	/* @todo @fixme: which kind of pointer is 'value'? s32_t or u8_t??? */
	id = (u8_t)od->id_inst_ptr[0];
	PRIVMIB_DEBUG(("line:%d, id:%d, ip:%d.%d.%d.%d\n", __LINE__, id, *pch, *(pch+1), *(pch+2), *(pch+3)));

	switch (id) {
	case LT300SYS_MCFG_SYS_DESCR_ID    : {
		struct m3_sys_info_st sys_info;
		read_syscfgdata_tbl(SYSCFGDATA_TBL_SYS_INFO, 0, &sys_info);
		rt_strncpy(sys_info.dev_descr, value, sizeof(sys_info.dev_descr)-1);
		write_syscfgdata_tbl(SYSCFGDATA_TBL_SYS_INFO, 0, &sys_info);

	}
	break;
	case LT300SYS_MCFG_NE_ID_ID        : {
		struct m3_sys_info_st sys_info;
		read_syscfgdata_tbl(SYSCFGDATA_TBL_SYS_INFO, 0, &sys_info);
		rt_strncpy(sys_info.dev_sn, pch, DEV_SN_BUF_CHARS_NUM_MAX);
		sys_info.dev_sn[DEV_SN_BUF_CHARS_NUM_MAX] = '\0';
		write_syscfgdata_tbl(SYSCFGDATA_TBL_SYS_INFO, 0, &sys_info);
	}
	break;

	case LT300SYS_MCFG_SYS_DEFAULT_ID  :
		if (1==(*((u32_t *)value)))
			restore_default_syscfgdata();
		break;

	case LT300SYS_MCFG_SYS_REBOOT_ID   :
		if (1==(*((u32_t *)value)))
			reset_whole_system();
		break;

	default:
		break;
	}

	return;
}

#if RT_USING_GPRS
/*
 * GPRS cfg
 */
static void yjlt300sys_gprscfg_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
{
	/* return to object name, adding index depth (1) */
	ident_len += 1;
	ident     -= 1;

	if (ident_len == 2) {
		u8_t id;

		od->id_inst_len = ident_len;
		od->id_inst_ptr = ident;

		LWIP_ASSERT("invalid id", (ident[0] >= 0) && (ident[0] <= 0xff));

		id = (u8_t)ident[0];
		od->instance = MIB_OBJECT_TAB;
		switch (id) {
		case LT300SYS_GPRSCFG_SERVER_IP_ID           :
		case LT300SYS_GPRSCFG_SERVER_SUBNET_ID       :
		case LT300SYS_GPRSCFG_SERVER_GW_ID           :
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_IPADDR);
			od->v_len    = 4;
			break;

		case LT300SYS_GPRSCFG_SERVER_PORT_ID         :
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len    = 4; 
			break;

		case LT300SYS_GPRSCFG_MASTER_TELNUM_ID     :
		case LT300SYS_GPRSCFG_SMS_CENTERNUM_ID     :
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 12;
			break;
		case LT300SYS_GPRSCFG_APN_NAME_ID    :
		case LT300SYS_GPRSCFG_APN_USERNAME_ID    :
		case LT300SYS_GPRSCFG_APN_USERPW_ID    :
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 32;
			break;
		default:
			LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no such object\n"));
			od->instance = MIB_OBJECT_NONE;
			break;
		}
	} else {
		LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no scalar\n"));
		od->instance = MIB_OBJECT_NONE;
	}

	return;
}

static void yjlt300sys_gprscfg_entry_get_value(struct obj_def *od, u16_t len, void *value)
{
	u32_t *uint_ptr = value;
	char  *pch      = value;
	struct gprs_if_info_st gprs_if;
	u8_t id;
 //   int length = 0;
	
	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	read_syscfgdata_tbl(SYSCFGDATA_TBL_GPRSCFG, 0, &gprs_if); /* 可以节省程序存储空间 */

	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_GPRSCFG_SERVER_IP_ID           :
		*uint_ptr = lwip_htonl(gprs_if.server_ipaddr.addr);
		break;

	case LT300SYS_GPRSCFG_SERVER_SUBNET_ID       :
		*uint_ptr = lwip_htonl(gprs_if.server_netmask.addr);
		break;

	case LT300SYS_GPRSCFG_SERVER_GW_ID           :
		*uint_ptr = lwip_htonl(gprs_if.server_gw.addr);
		break;

	case LT300SYS_GPRSCFG_SERVER_PORT_ID          :
		*uint_ptr = gprs_if.server_port;
		break;
	case LT300SYS_GPRSCFG_MASTER_TELNUM_ID     :
		rt_strncpy(pch, gprs_if.master_telnum, sizeof(gprs_if.master_telnum));
		break;
	case LT300SYS_GPRSCFG_SMS_CENTERNUM_ID     :
		rt_strncpy(pch, gprs_if.sms_telnum, sizeof(gprs_if.sms_telnum));
		break;
		
	case LT300SYS_GPRSCFG_APN_NAME_ID     :
		rt_strncpy(pch, gprs_if.apn_name, sizeof(gprs_if.apn_name));
		break;
	case LT300SYS_GPRSCFG_APN_USERNAME_ID    :
		rt_strncpy(pch, gprs_if.apn_usr, sizeof(gprs_if.apn_usr));
		break;
	case LT300SYS_GPRSCFG_APN_USERPW_ID    :
		rt_strncpy(pch, gprs_if.apn_pw, sizeof(gprs_if.apn_pw));
		break;
	default: /*  */
		break;

	}

	return;
}

static u8_t yjlt300sys_gprscfg_entry_set_test(struct obj_def *od, u16_t len, void *value)
{
	u8_t id, set_ok;
	//u8_t *pch = value;

	LWIP_UNUSED_ARG(len);
	set_ok = 0;
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));


	id = (u8_t)od->id_inst_ptr[0];
	PRIVMIB_DEBUG(("line:%d, id:%d, ip:%d.%d.%d.%d\n", __LINE__, id, *pch, *(pch+1), *(pch+2), *(pch+3)));
	switch (id) {
	case LT300SYS_GPRSCFG_SERVER_IP_ID          :
	case LT300SYS_GPRSCFG_SERVER_SUBNET_ID      :
	case LT300SYS_GPRSCFG_SERVER_GW_ID           :
	case LT300SYS_GPRSCFG_SERVER_PORT_ID     :
	case LT300SYS_GPRSCFG_MASTER_TELNUM_ID    :
	case LT300SYS_GPRSCFG_SMS_CENTERNUM_ID    :
	case LT300SYS_GPRSCFG_APN_NAME_ID    :
	case LT300SYS_GPRSCFG_APN_USERNAME_ID   :
	case LT300SYS_GPRSCFG_APN_USERPW_ID  :	
		set_ok = 1;
		break;

	default:
		break;
	}

	return set_ok;
}

static void yjlt300sys_gprscfg_entry_set_value(struct obj_def *od, u16_t len, void *value)
{
	u8_t id;
	char *pch = value;
	struct gprs_if_info_st gprs_if;
	
	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	read_syscfgdata_tbl(SYSCFGDATA_TBL_GPRSCFG, 0, &gprs_if); /* 可以节省程序存储空间 */

	/* @todo @fixme: which kind of pointer is 'value'? s32_t or u8_t??? */
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case  LT300SYS_GPRSCFG_SERVER_IP_ID         :
		gprs_if.server_ipaddr.addr =  lwip_ntohl(*((u32_t *)value));
		break;

	case  LT300SYS_GPRSCFG_SERVER_SUBNET_ID     :
		gprs_if.server_netmask.addr =  lwip_ntohl(*((u32_t *)value));
		break;

	case  LT300SYS_GPRSCFG_SERVER_GW_ID         :
		gprs_if.server_gw.addr =  lwip_ntohl(*((u32_t *)value));
		break;
		
	case LT300SYS_GPRSCFG_SERVER_PORT_ID    :
		 gprs_if.server_port = *(u16_t *)value ;
		break;
	case LT300SYS_GPRSCFG_MASTER_TELNUM_ID    :
		rt_strncpy(gprs_if.master_telnum, pch, sizeof(gprs_if.master_telnum));
	    break;
	case LT300SYS_GPRSCFG_SMS_CENTERNUM_ID    :
		rt_strncpy(gprs_if.sms_telnum, pch, sizeof(gprs_if.sms_telnum));
		break;
	case LT300SYS_GPRSCFG_APN_NAME_ID    :
		rt_strncpy(gprs_if.apn_name, pch, sizeof(gprs_if.apn_name));
		break;
	case LT300SYS_GPRSCFG_APN_USERNAME_ID   : 
		rt_strncpy(gprs_if.apn_usr, pch, sizeof(gprs_if.apn_usr));
	    break;
	case  LT300SYS_GPRSCFG_APN_USERPW_ID   :
		rt_strncpy(gprs_if.apn_pw, pch, sizeof(gprs_if.apn_pw));
		break;
	default:
		break;
	}

	write_syscfgdata_tbl(SYSCFGDATA_TBL_IPCFG, 0, &gprs_if);
	

	PRIVMIB_DEBUG(("line:%d, id:%d, ip:%d.%d.%d.%d\n", __LINE__, id, *pch, *(pch+1), *(pch+2), *(pch+3)));

	return;
}
#endif
/*
 * trap cfg
 */
static void yjlt300sys_trapcfg_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
{
	/* return to object name, adding index depth (1) */
	ident_len += 1;
	ident     -= 1;

	if (ident_len == 2) {
		u8_t id;

		od->id_inst_len = ident_len;
		od->id_inst_ptr = ident;

		LWIP_ASSERT("invalid id", (ident[0] >= 0) && (ident[0] <= 0xff));

		id = (u8_t)ident[0];
		od->instance = MIB_OBJECT_TAB;
		switch (id) {
		case LT300SYS_TRAPCFG_TRAP_TYPE_ID		:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = 4;
			break;

		case LT300SYS_TRAPCFG_ADDR_ID              :
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_IPADDR);
			od->v_len    = 4;
			break;

		case LT300SYS_TRAPCFG_SNMP_VER_ID          :
		case LT300SYS_TRAPCFG_TARGET_PORT_ID       :
		case LT300SYS_TRAPCFG_VALID_ID             :
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = 4;
			break;
		case LT300SYS_TRAPCFG_PHASEA_VOL_LOSS_EVENT_ID:
		case LT300SYS_TRAPCFG_PHASEA_CUR_LOSS_EVENT_ID:	
		case LT300SYS_TRAPCFG_METER_CLEAR_EVENT_ID:		
		case LT300SYS_TRAPCFG_DEMAND_CLEAR_EVENT_ID: 
		case LT300SYS_TRAPCFG_PROGRAM_EVENT_ID:
		case LT300SYS_TRAPCFG_CALIBRATE_TIME_EVENT_ID: 
		case LT300SYS_TRAPCFG_REVERSE_SEQ_VOL_EVENT_ID: 
		case LT300SYS_TRAPCFG_REVERSE_SEQ_CUR_EVENT_ID: 
			od->access	 = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len	 = 4;
			break;

		case LT300SYS_TRAPCFG_COMMUNITY_ID         :
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = SNMP_COMMUNITY_LEN_MAX;
			break;

		default:
			LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no such object\n"));
			od->instance = MIB_OBJECT_NONE;
			break;
		}
	} else {
		LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no scalar\n"));
		od->instance = MIB_OBJECT_NONE;
	}

	return;
}

static void yjlt300sys_trapcfg_entry_get_value(struct obj_def *od, u16_t len, void *value)
{
	u32_t *uint_ptr = value;
	u8_t id;
	struct nms_if_info_st nms_if;

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_TRAPCFG_TRAP_TYPE_ID		:
		*uint_ptr = SNMP_GENTRAP_ENTERPRISESPC;
		break;

	case LT300SYS_TRAPCFG_ADDR_ID              :{
			ip_addr_t *dst = (ip_addr_t*)value;
			read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
			nms_if.trap_ip = lwip_htonl(nms_if.trap_ip);
			rt_memcpy(dst, (ip_addr_t *)&nms_if.trap_ip, sizeof(ip_addr_t));
		}
		break;

	case LT300SYS_TRAPCFG_SNMP_VER_ID          :
		*uint_ptr = snmp_version;
		break;

	case LT300SYS_TRAPCFG_TARGET_PORT_ID       :{
			read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
			*uint_ptr = nms_if.trap_port;
		}
		break;

	case LT300SYS_TRAPCFG_COMMUNITY_ID         : {
		struct snmp_community_st snmp_commu;
		read_syscfgdata_tbl(SYSCFGDATA_TBL_SNMP_COMMU, 0, &snmp_commu);
		rt_strncpy(value, snmp_commu.get_commu, SNMP_COMMUNITY_LEN_MAX);
	}
	break;

	case LT300SYS_TRAPCFG_VALID_ID             : {
			read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
			*uint_ptr = nms_if.trap_enable_bits;
		}
		break;
	case LT300SYS_TRAPCFG_PHASEA_VOL_LOSS_EVENT_ID:{
			read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
			*uint_ptr = nms_if.trap_enable_bits;
		}
		break;
	case LT300SYS_TRAPCFG_PHASEA_CUR_LOSS_EVENT_ID: {
			read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
			*uint_ptr = nms_if.trap_enable_bits;
		}
		break;
	case LT300SYS_TRAPCFG_METER_CLEAR_EVENT_ID: {
			read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
			*uint_ptr = nms_if.trap_enable_bits;
		}
		break;
	case LT300SYS_TRAPCFG_DEMAND_CLEAR_EVENT_ID: {
			read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
			*uint_ptr = nms_if.trap_enable_bits;
		}
		break;
	case LT300SYS_TRAPCFG_PROGRAM_EVENT_ID:{
			read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
			*uint_ptr = nms_if.trap_enable_bits;
		}
		break;
	case LT300SYS_TRAPCFG_CALIBRATE_TIME_EVENT_ID: {
			read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
			*uint_ptr = nms_if.trap_enable_bits;
		}
		break;
	case LT300SYS_TRAPCFG_REVERSE_SEQ_VOL_EVENT_ID: {
			read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
			*uint_ptr = nms_if.trap_enable_bits;
		}
		break;
	case LT300SYS_TRAPCFG_REVERSE_SEQ_CUR_EVENT_ID: {
			read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
			*uint_ptr = nms_if.trap_enable_bits;
		}
		break;

	default:
		break;
	}

	return;
}

static u8_t yjlt300sys_trapcfg_entry_set_test(struct obj_def *od, u16_t len, void *value)
{
	u8_t id, set_ok;

	LWIP_UNUSED_ARG(len);
	set_ok = 0;
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_TRAPCFG_ADDR_ID              :
	case LT300SYS_TRAPCFG_TARGET_PORT_ID       :
	case LT300SYS_TRAPCFG_COMMUNITY_ID         :
	case LT300SYS_TRAPCFG_VALID_ID             :
	case LT300SYS_TRAPCFG_PHASEA_VOL_LOSS_EVENT_ID:
	case LT300SYS_TRAPCFG_PHASEA_CUR_LOSS_EVENT_ID:	
	case LT300SYS_TRAPCFG_METER_CLEAR_EVENT_ID:		
	case LT300SYS_TRAPCFG_DEMAND_CLEAR_EVENT_ID: 
	case LT300SYS_TRAPCFG_PROGRAM_EVENT_ID:
	case LT300SYS_TRAPCFG_CALIBRATE_TIME_EVENT_ID: 
	case LT300SYS_TRAPCFG_REVERSE_SEQ_VOL_EVENT_ID: 
	case LT300SYS_TRAPCFG_REVERSE_SEQ_CUR_EVENT_ID: 
#if 0
		if ((0==*sint_ptr) || (1==*sint_ptr))
			set_ok = 1;
#else
		set_ok = 1;
#endif
		break;
#if 0
		/* lt300只支持v1版本 */
	case lt300SYS_TRAPCFG_SNMP_VER_ID          :
		break;
#endif

	default:
		break;
	}


	return set_ok;
}

static void yjlt300sys_trapcfg_entry_set_value(struct obj_def *od, u16_t len, void *value)
{
	u8_t id;
	u32_t *uint_ptr = (u32_t *)value;
	u8_t *pch = (u8_t *)value;
	struct nms_if_info_st nms_if;

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	read_syscfgdata_tbl(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);

	/* @todo @fixme: which kind of pointer is 'value'? s32_t or u8_t??? */
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_TRAPCFG_ADDR_ID              :{
			ip_addr_t *dst = (ip_addr_t*)value;
			u32_t addr;
			read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
			snmp_trap_dst_ip_set(0, dst);
			//snmp_trap_dst_enable(0, SNMP_TRAP_0_FLAG);
			rt_memcpy(&addr, dst, sizeof(ip_addr_t));
			nms_if.trap_ip = lwip_htonl(addr);
			write_syscfgdata_tbl(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
		}
		break;

	case LT300SYS_TRAPCFG_TARGET_PORT_ID       :{
			read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
			nms_if.trap_port = *uint_ptr;
			write_syscfgdata_tbl(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
		}
		break;

	case LT300SYS_TRAPCFG_COMMUNITY_ID         : {
			struct snmp_community_st snmp_commu;
			read_syscfgdata_tbl(SYSCFGDATA_TBL_SNMP_COMMU, 0, &snmp_commu);
			rt_strncpy(snmp_commu.get_commu, value, SNMP_COMMUNITY_LEN_MAX);
			write_syscfgdata_tbl(SYSCFGDATA_TBL_SNMP_COMMU, 0, &snmp_commu);
		}
	break;

	case LT300SYS_TRAPCFG_VALID_ID             :{
			read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
			nms_if.trap_enable_bits = *uint_ptr; 
			write_syscfgdata_tbl(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
			nms_if.trap_ip = lwip_htonl(nms_if.trap_ip);
			snmp_trap_dst_ip_set(0, (ip_addr_t *)&nms_if.trap_ip);
			snmp_trap_dst_enable(0, *pch);
		}
		break;
	case LT300SYS_TRAPCFG_PHASEA_VOL_LOSS_EVENT_ID:{
			read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
			nms_if.trap_enable_bits = *uint_ptr;
			if(nms_if.trap_enable_bits == 1){
				trap_send(1,SNMP_GENTRAP_ENTERPRISESPC, E_ALR_PA_VOLTAGE_LOSS_EVENT_APPEAR, E_ALR_PA_VOLTAGE_LOSS_EVENT_APPEAR, SDT_ELECTRIC_METER);
			}
			//write_syscfgdata_tbl(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
		}
		break;
	case LT300SYS_TRAPCFG_PHASEA_CUR_LOSS_EVENT_ID: {
			read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
			nms_if.trap_enable_bits = *uint_ptr; 
			if(nms_if.trap_enable_bits == 1){
				trap_send(1,SNMP_GENTRAP_ENTERPRISESPC, E_ALR_PA_CURRENT_LOSS_EVENT_APPEAR, E_ALR_PA_CURRENT_LOSS_EVENT_APPEAR, SDT_ELECTRIC_METER);
			}
			//write_syscfgdata_tbl(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
		}
		break;
	case LT300SYS_TRAPCFG_METER_CLEAR_EVENT_ID: {
			read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
			nms_if.trap_enable_bits = *uint_ptr;	
			if(nms_if.trap_enable_bits == 1){
				trap_send(1,SNMP_GENTRAP_ENTERPRISESPC, E_ALR_EM_METER_CLEAR_EVENT_APPEAR, E_ALR_EM_METER_CLEAR_EVENT_APPEAR, SDT_ELECTRIC_METER);
			}
			//write_syscfgdata_tbl(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
		}
		break;
	case LT300SYS_TRAPCFG_DEMAND_CLEAR_EVENT_ID:{
			read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
			nms_if.trap_enable_bits = *uint_ptr;
			if(nms_if.trap_enable_bits == 1){
				trap_send(1,SNMP_GENTRAP_ENTERPRISESPC, E_ALR_EM_DEMAND_CLEAR_EVENT_APPEAR, E_ALR_EM_DEMAND_CLEAR_EVENT_APPEAR, SDT_ELECTRIC_METER);
			}
			//write_syscfgdata_tbl(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
		}
		break;
	case LT300SYS_TRAPCFG_PROGRAM_EVENT_ID:{
			read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
			nms_if.trap_enable_bits = *uint_ptr;	
			if(nms_if.trap_enable_bits == 1){
				trap_send(1,SNMP_GENTRAP_ENTERPRISESPC, E_ALR_EM_PROGRAM_EVENT_APPEAR, E_ALR_EM_PROGRAM_EVENT_APPEAR, SDT_ELECTRIC_METER);
			}
			//write_syscfgdata_tbl(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
		}
		break;
	case LT300SYS_TRAPCFG_CALIBRATE_TIME_EVENT_ID:{
			read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
			nms_if.trap_enable_bits = *uint_ptr;	
			if(nms_if.trap_enable_bits == 1){
				trap_send(1,SNMP_GENTRAP_ENTERPRISESPC, E_ALR_EM_CALIBRATE_TIME_EVENT_APPEAR, E_ALR_EM_CALIBRATE_TIME_EVENT_APPEAR, SDT_ELECTRIC_METER);
			}
			//write_syscfgdata_tbl(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
		}
		break;
	case LT300SYS_TRAPCFG_REVERSE_SEQ_VOL_EVENT_ID: {
			read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
			nms_if.trap_enable_bits = *uint_ptr;	
			if(nms_if.trap_enable_bits == 1){
				trap_send(1,SNMP_GENTRAP_ENTERPRISESPC, E_ALR_EM_REVERSE_SEQ_VOL_EVENT_APPEAR, E_ALR_EM_REVERSE_SEQ_VOL_EVENT_APPEAR, SDT_ELECTRIC_METER);
			}
			//write_syscfgdata_tbl(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
		}
		break;
	case LT300SYS_TRAPCFG_REVERSE_SEQ_CUR_EVENT_ID:{
			read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
			nms_if.trap_enable_bits = *uint_ptr;
			if(nms_if.trap_enable_bits == 1){
				trap_send(1,SNMP_GENTRAP_ENTERPRISESPC, E_ALR_EM_REVERSE_SEQ_CUR_EVENT_APPEAR, E_ALR_EM_REVERSE_SEQ_CUR_EVENT_APPEAR, SDT_ELECTRIC_METER);
			}
			//write_syscfgdata_tbl(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
		}
		break;

	default:
		break;
	}

	if (LT300SYS_TRAPCFG_ADDR_ID==id || LT300SYS_TRAPCFG_TARGET_PORT_ID==id || LT300SYS_TRAPCFG_VALID_ID==id){
		syscfgdata_syn_proc();
	}

	return;
}

/*
 * env parameter cfg
 */
static void yjlt300sys_envpcfg_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
{
	/* return to object name, adding index depth (1) */
	ident_len += 1;
	ident     -= 1;

	if (ident_len == 2) {
		u8_t id;

		od->id_inst_len = ident_len;
		od->id_inst_ptr = ident;

		LWIP_ASSERT("invalid id", (ident[0] >= 0) && (ident[0] <= 0xff));
		od->instance = MIB_OBJECT_TAB;
		
		id = (u8_t)ident[0];
		switch (id) {
		case LT300SYS_ENVPCFG_TMP_VALUE_ID	:
		case LT300SYS_ENVPCFG_RH_VALUE_ID:
		case LT300SYS_ENVPCFG_TMP_ALARM_ID:
		case LT300SYS_ENVPCFG_RH_ALARM_ID      :
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len    = 4;
			break;

		case LT300SYS_ENVPCFG_TMP_LIMEN_ID         :
		case LT300SYS_ENVPCFG_RH_LIMEN_ID          :
		case LT300SYS_ENVPCFG_TMP_ALARM_MASK_ID :
		case LT300SYS_ENVPCFG_RH_ALARM_MASK_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len    = 4;
			break;

		default:
			LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no such object\n"));
			od->instance = MIB_OBJECT_NONE;
			break;
		}
	} else {
		LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no scalar\n"));
		od->instance = MIB_OBJECT_NONE;
	}

	return;
}

static void yjlt300sys_envpcfg_entry_get_value(struct obj_def *od, u16_t len, void *value)
{
	u8_t id;
  //  struct temp_rh_limen tem_rh;
	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_ENVPCFG_TMP_VALUE_ID	:
		break;
		
    case LT300SYS_ENVPCFG_RH_VALUE_ID:
		break;
		
	case LT300SYS_ENVPCFG_TMP_LIMEN_ID:
		break;

	case LT300SYS_ENVPCFG_RH_LIMEN_ID:
		break;

	case LT300SYS_ENVPCFG_TMP_ALARM_ID:
		break;

	case LT300SYS_ENVPCFG_RH_ALARM_ID:
		break;

	case LT300SYS_ENVPCFG_TMP_ALARM_MASK_ID : /* mark by David */
		break;

	case LT300SYS_ENVPCFG_RH_ALARM_MASK_ID: /* mark by David */
		break;

	default:
		break;
	}

	return;
}

static u8_t yjlt300sys_envpcfg_entry_set_test(struct obj_def *od, u16_t len, void *value)
{
	u8_t id, set_ok;

	LWIP_UNUSED_ARG(len);
	set_ok = 0;
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_ENVPCFG_TMP_LIMEN_ID         :
	case LT300SYS_ENVPCFG_RH_LIMEN_ID          :
	case LT300SYS_ENVPCFG_TMP_ALARM_MASK_ID :
	case LT300SYS_ENVPCFG_RH_ALARM_MASK_ID:
#if 0
		if ((0==*sint_ptr) || (1==*sint_ptr))
			set_ok = 1;
#else
		set_ok = 1;
#endif
		break;

	default:
		break;
	}

	return set_ok;
}

static void yjlt300sys_envpcfg_entry_set_value(struct obj_def *od, u16_t len, void *value)
{
	u8_t id;

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	/* @todo @fixme: which kind of pointer is 'value'? s32_t or u8_t??? */
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_ENVPCFG_TMP_LIMEN_ID         :
		break;

	case LT300SYS_ENVPCFG_RH_LIMEN_ID          :
		break;

	case LT300SYS_ENVPCFG_TMP_ALARM_MASK_ID :
		break;

	case LT300SYS_ENVPCFG_RH_ALARM_MASK_ID :
		break;

	default:
		break;
	}

	return;
}

/*
 * line data collection
 */
static void yjlt300sys_linedata_read_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
{
	/* return to object name, adding index depth (1) */
	ident_len += 2;
	ident     -= 2;

	if (ident_len == 3) {
		u8_t id;

		od->id_inst_len = ident_len;
		od->id_inst_ptr = ident;

		LWIP_ASSERT("invalid id", (ident[0] >= 0) && (ident[0] <= 0xff));
		od->instance = MIB_OBJECT_TAB;
		id = (u8_t)ident[0];
		switch (id) {
		case LT300SYS_LINEDATA_INDEX_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV| SNMP_ASN1_PRIMIT| SNMP_ASN1_INTEG);
			od->v_len    = sizeof(u32_t);
			break;
		case LT300SYS_LINEDATA_PHASEA_READ_ID:
		case LT300SYS_LINEDATA_PHASEB_READ_ID:
		case LT300SYS_LINEDATA_PHASEC_READ_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV| SNMP_ASN1_PRIMIT| SNMP_ASN1_OC_STR);
			od->v_len    = 44;
			break;

		case LT300SYS_LINEDATA_TOTAL_READ_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 8;
			break;
		case LT300SYS_LINEDATA_PHASEA_VOL_SAMPLE_READ_ID:
		case LT300SYS_LINEDATA_PHASEB_VOL_SAMPLE_READ_ID:
		case LT300SYS_LINEDATA_PHASEC_VOL_SAMPLE_READ_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 120;
			break;
		case LT300SYS_LINEDATA_PHASEA_CUR_SAMPLE_READ_ID:
		case LT300SYS_LINEDATA_PHASEB_CUR_SAMPLE_READ_ID:
		case LT300SYS_LINEDATA_PHASEC_CUR_SAMPLE_READ_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 120;
			break;
		case LT300SYS_LINEDATA_COPPER_IRON_LOSSES_READ_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access	 = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV| SNMP_ASN1_PRIMIT| SNMP_ASN1_OC_STR);
			od->v_len	 = 32;
			break;
		case LT300SYS_LINEDATA_PHASEA_VOL_HARMONIC_READ_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access	 = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV| SNMP_ASN1_PRIMIT| SNMP_ASN1_OC_STR);
			od->v_len	 = 84;
			break;
		case LT300SYS_LINEDATA_PHASEA_CUR_HARMONIC_READ_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access	 = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV| SNMP_ASN1_PRIMIT| SNMP_ASN1_OC_STR);
			od->v_len	 = 84;
			break;
		case LT300SYS_LINEDATA_PHASEA_ACT_HARMONIC_READ_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access	 = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV| SNMP_ASN1_PRIMIT| SNMP_ASN1_OC_STR);
			od->v_len	 = 84;
			break;
		case LT300SYS_LINEDATA_PHASEB_VOL_HARMONIC_READ_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access	 = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV| SNMP_ASN1_PRIMIT| SNMP_ASN1_OC_STR);
			od->v_len	 = 84;
			break;
		case LT300SYS_LINEDATA_PHASEB_CUR_HARMONIC_READ_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access	 = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV| SNMP_ASN1_PRIMIT| SNMP_ASN1_OC_STR);
			od->v_len	 = 84;
			break;
		case LT300SYS_LINEDATA_PHASEB_ACT_HARMONIC_READ_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access	 = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV| SNMP_ASN1_PRIMIT| SNMP_ASN1_OC_STR);
			od->v_len	 = 84;
			break;
		case LT300SYS_LINEDATA_PHASEC_VOL_HARMONIC_READ_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access	 = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV| SNMP_ASN1_PRIMIT| SNMP_ASN1_OC_STR);
			od->v_len	 = 84;
			break;
		case LT300SYS_LINEDATA_PHASEC_CUR_HARMONIC_READ_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access	 = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV| SNMP_ASN1_PRIMIT| SNMP_ASN1_OC_STR);
			od->v_len	 = 84;
			break;
		case LT300SYS_LINEDATA_PHASEC_ACT_HARMONIC_READ_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access	 = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV| SNMP_ASN1_PRIMIT| SNMP_ASN1_OC_STR);
			od->v_len	 = 84;
			break;		
		case LT300SYS_LINEDATA_READ_ALARM_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 4;
			break;
		case LT300SYS_LINEDATA_READ_ALARM_MASK_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 4;
			break;

		default:
			LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no such object\n"));
			od->instance = MIB_OBJECT_NONE;
			break;
		}
	} else {
		LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no scalar\n"));
		od->instance = MIB_OBJECT_NONE;
	}

	return;
}

static void yjlt300sys_linedata_read_entry_get_value(struct obj_def *od, u16_t len, void *value)
{
	u32_t *u32_ptr = (u32_t *)value;
	u8_t  *pch = (u8_t  *)value;
	struct collect_line_data_info_st data;
	//struct px_sample_data_st *sam_ptr = (struct px_sample_data_st *)value;
	u32_t linedata[32]={0};
	
	
	u8_t id,i;
	
	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_LINEDATA_INDEX_ID:
		*u32_ptr = od->id_inst_ptr[2];
		break;
	case LT300SYS_LINEDATA_PHASEA_READ_ID:
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EMC_PA_INFO, linedata, SINKINFO_EMC_PX_DATA_SIZE)){
			for(i=0; i<SINKINFO_EMC_PX_DATA_SIZE/sizeof(u32_t); i++){
				*(u32_ptr+i)=lwip_htonl(linedata[i]);
				//printf_syn("value:%8x\n",*(u32_ptr+i));
			}
			//rt_memcpy(pch, (u8_t *)linedata, SINKINFO_EMC_PX_DATA_SIZE);
			
		}
		else{
			
			printf_syn("line data  phaseA PARAM read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EMC_PA_INFO, linedata, SINKINFO_EMC_PX_DATA_SIZE)){
 				printf_syn("line data phaseA voltage read failed!\r\n");
			}
			else{
				for(i=0; i<SINKINFO_EMC_PX_DATA_SIZE/sizeof(u32_t); i++)
					*(u32_ptr+i) = lwip_htonl(linedata[i]);
			
				//rt_memcpy(u32_ptr, linedata, SINKINFO_EMC_PX_DATA_SIZE);
			}
		}
#if 0
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_VOLTAGE, uint_ptr, &data.LineData.LVolCurveB, &data.LineData.LVolCurveC)){
			lwip_htonl(*uint_ptr);
		}
		else{
			//LWIP_DEBUGF(SNMP_MIB_DEBUG,("line_data_read_voltage: errer, please read again!\n"));
			printf_syn("line data  phaseA voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_VOLTAGE, uint_ptr, &data.LineData.LVolCurveB, &data.LineData.LVolCurveC)){
 				printf_syn("line data phaseA voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*uint_ptr);
			}
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], SIC_GET_CURRENT, uint_ptr+1, &data.LineData.LCurCurveB, &data.LineData.LCurCurveC)){
			lwip_htonl(*(uint_ptr+1));
		}
		else{
			printf_syn("line data  phaseA voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_CURRENT, uint_ptr+1, &data.LineData.LCurCurveB, &data.LineData.LCurCurveC)){
 				printf_syn("line data phaseA voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+1));
			}
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_ACTIVE_POWER, uint_ptr+2, &data.LineData.LActPowB, &data.LineData.LActPowC)){
			lwip_htonl(*(uint_ptr+2));
		}
		else{
			printf_syn("line data  phaseA voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_ACTIVE_POWER, uint_ptr+2, &data.LineData.LActPowB, &data.LineData.LActPowC)){
 				printf_syn("line data phaseA voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+2));
			}
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_REACTIVE_POWER, uint_ptr+3, &data.LineData.LReActPowB, &data.LineData.LReActPowC)){
			lwip_htonl(*(uint_ptr+3));
		}
		else{
			printf_syn("line data  phaseA voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_REACTIVE_POWER, uint_ptr+3, &data.LineData.LReActPowB, &data.LineData.LReActPowC)){
 				printf_syn("line data phaseA voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+3));
			}
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_POWER_FACTOR, uint_ptr+4, &data.LineData.LPowFacB, &data.LineData.LPowFacC)){
			lwip_htonl(*(uint_ptr+4));
		}
		else{
			printf_syn("line data  phaseA voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_POWER_FACTOR, uint_ptr+4, &data.LineData.LPowFacB, &data.LineData.LPowFacC)){
 				printf_syn("line data phaseA voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+4));
			}
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_APPARENT_POWER, uint_ptr+5, &data.LineData.LApparentB, &data.LineData.LApparentC)){
			lwip_htonl(*(uint_ptr+5));
		}
		else{
			printf_syn("line data  phaseA voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_APPARENT_POWER, uint_ptr+5, &data.LineData.LApparentB, &data.LineData.LApparentC)){
 				printf_syn("line data phaseA voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+5));
			}
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_PHASE, uint_ptr+6, &data.LineData.LPhaseB, &data.LineData.LPhaseC)){
			lwip_htonl(*(uint_ptr+6));
		}
		else{
			printf_syn("line data  phaseA voltage read failed!try read again!\r\n");
			if(SIE_OK !=  get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_PHASE, uint_ptr+6, &data.LineData.LPhaseB, &data.LineData.LPhaseC)){
 				printf_syn("line data phaseA voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+6));
			}
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_FREQUENCY, uint_ptr+7, &data.LineData.LFrequencyB, &data.LineData.LFrequencyC)){
			lwip_htonl(*(uint_ptr+7));
		}
		else{
			printf_syn("line data  phaseA voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_FREQUENCY, uint_ptr+7, &data.LineData.LFrequencyB, &data.LineData.LFrequencyC)){
 				printf_syn("line data phaseA voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+7));
			}
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_VOLTAGE_DISTORTION, uint_ptr+8, &data.LineData.LVolDistortB, &data.LineData.LVolDistortC)){
			lwip_htonl(*(uint_ptr+8));
		}
		else{
			printf_syn("line data  phaseA voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_VOLTAGE_DISTORTION, uint_ptr+8, &data.LineData.LVolDistortB, &data.LineData.LVolDistortC)){
 				printf_syn("line data phaseA voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+8));
			}
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_CURRENT_DISTORTION, uint_ptr+9, &data.LineData.LCurDistortB, &data.LineData.LCurDistortC)){
			lwip_htonl(*(uint_ptr+9));
		}
		else{
			printf_syn("line data  phaseA voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_CURRENT_DISTORTION, uint_ptr+9, &data.LineData.LCurDistortB, &data.LineData.LCurDistortC)){
 				printf_syn("line data phaseA voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+9));
			}
		}
#endif	
		break;

	case LT300SYS_LINEDATA_PHASEB_READ_ID:
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EMC_PB_INFO, linedata, SINKINFO_EMC_PX_DATA_SIZE)){
			for(i=0; i<SINKINFO_EMC_PX_DATA_SIZE/sizeof(u32_t); i++)
				*(u32_ptr+i) = lwip_htonl(linedata[i]);
			
			//rt_memcpy(u32_ptr, linedata, SINKINFO_EMC_PX_DATA_SIZE);
		}
		else{
			printf_syn("line data  phaseB voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EMC_PB_INFO, linedata, SINKINFO_EMC_PX_DATA_SIZE)){
 				printf_syn("line data phaseB voltage read failed!\r\n");
			}
			else{
				for(i=0; i<SINKINFO_EMC_PX_DATA_SIZE/sizeof(u32_t); i++)
					*(u32_ptr+i) = lwip_htonl(linedata[i]);
			
				//rt_memcpy(u32_ptr, linedata, SINKINFO_EMC_PX_DATA_SIZE);
			}
		}
#if 0		
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_VOLTAGE, &data.LineData.LVolCurveA, uint_ptr, &data.LineData.LVolCurveC)){
			lwip_htonl(*uint_ptr);
		}
		else{
			printf_syn("line data phaseB voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_VOLTAGE, &data.LineData.LVolCurveA, uint_ptr, &data.LineData.LVolCurveC)){
				printf_syn("line data phaseB voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*uint_ptr);
			}	
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_CURRENT, &data.LineData.LCurCurveA, uint_ptr+1, &data.LineData.LCurCurveC)){
			lwip_htonl(*(uint_ptr+1));
		}
		else{
			printf_syn("line data phaseB voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_CURRENT, &data.LineData.LCurCurveA, uint_ptr+1, &data.LineData.LCurCurveC)){
				printf_syn("line data phaseB voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+1));
			}	
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_ACTIVE_POWER, &data.LineData.LActPowA, uint_ptr+2, &data.LineData.LActPowC)){
			lwip_htonl(*(uint_ptr+2));
		}
		else{
			printf_syn("line data phaseB voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_ACTIVE_POWER, &data.LineData.LActPowA, uint_ptr+2, &data.LineData.LActPowC)){
				printf_syn("line data phaseB voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+2));
			}	
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_REACTIVE_POWER, &data.LineData.LReActPowA, uint_ptr+3, &data.LineData.LReActPowC)){
			lwip_htonl(*(uint_ptr+3));
		}
		else{
			printf_syn("line data phaseB voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_REACTIVE_POWER, &data.LineData.LReActPowA, uint_ptr+3, &data.LineData.LReActPowC)){
				printf_syn("line data phaseB voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+3));
			}	
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_POWER_FACTOR, &data.LineData.LPowFacA, uint_ptr+4, &data.LineData.LPowFacC)){
			lwip_htonl(*(uint_ptr+4));
		}
		else{
			printf_syn("line data phaseB voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], SIC_GET_POWER_FACTOR, &data.LineData.LPowFacA, uint_ptr+4, &data.LineData.LPowFacC)){
				printf_syn("line data phaseB voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+4));
			}	
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_APPARENT_POWER, &data.LineData.LApparentA, uint_ptr+5, &data.LineData.LApparentC)){
			lwip_htonl(*(uint_ptr+5));
		}
		else{
			printf_syn("line data phaseB voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_APPARENT_POWER, &data.LineData.LApparentA, uint_ptr+5, &data.LineData.LApparentC)){
				printf_syn("line data phaseB voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+5));
			}	
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_PHASE, &data.LineData.LPhaseA, uint_ptr+6, &data.LineData.LPhaseC)){
			lwip_htonl(*(uint_ptr+6));
		}
		else{
			printf_syn("line data phaseB voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], SIC_GET_PHASE, &data.LineData.LPhaseA, uint_ptr+6, &data.LineData.LPhaseC)){
				printf_syn("line data phaseB voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+6));
			}	
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_FREQUENCY, &data.LineData.LFrequencyA, uint_ptr+7, &data.LineData.LFrequencyC)){
			lwip_htonl(*(uint_ptr+7));
		}
		else{
			printf_syn("line data phaseB voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_FREQUENCY, &data.LineData.LFrequencyA, uint_ptr+7, &data.LineData.LFrequencyC)){
				printf_syn("line data phaseB voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+7));
			}	
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_VOLTAGE_DISTORTION, &data.LineData.LVolDistortA, uint_ptr+8, &data.LineData.LVolDistortC)){
			lwip_htonl(*(uint_ptr+8));
		}
		else{
			printf_syn("line data phaseB voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_VOLTAGE_DISTORTION, &data.LineData.LVolDistortA, uint_ptr+8, &data.LineData.LVolDistortC)){
				printf_syn("line data phaseB voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+8));
			}	
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_CURRENT_DISTORTION, &data.LineData.LCurDistortA, uint_ptr+9, &data.LineData.LCurDistortC)){
			lwip_htonl(*(uint_ptr+9));
		}
		else{
			printf_syn("line data phaseB voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_CURRENT_DISTORTION, &data.LineData.LCurDistortA, uint_ptr+9, &data.LineData.LCurDistortC)){
				printf_syn("line data phaseB voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+9));
			}	
		}
#endif
		break;

	case LT300SYS_LINEDATA_PHASEC_READ_ID:
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EMC_PC_INFO, linedata, SINKINFO_EMC_PX_DATA_SIZE)){
			for(i=0; i<SINKINFO_EMC_PX_DATA_SIZE/sizeof(u32_t); i++)
				*(u32_ptr+i) = lwip_htonl(linedata[i]);
			
			//rt_memcpy(u32_ptr, linedata, SINKINFO_EMC_PX_DATA_SIZE);
		}
		else{
			//LWIP_DEBUGF(SNMP_MIB_DEBUG,("line_data_read_voltage: errer, please read again!\n"));
			printf_syn("line data  phaseC voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EMC_PC_INFO, linedata, SINKINFO_EMC_PX_DATA_SIZE)){
 				printf_syn("line data phaseC voltage read failed!\r\n");
			}
			else{
				for(i=0; i<SINKINFO_EMC_PX_DATA_SIZE/sizeof(u32_t); i++)
					*(u32_ptr+i) = lwip_htonl(linedata[i]);
			
				//rt_memcpy(u32_ptr, linedata, SINKINFO_EMC_PX_DATA_SIZE);
			}
		}
#if 0		
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_VOLTAGE, &data.LineData.LVolCurveA, &data.LineData.LVolCurveB, uint_ptr)){
			lwip_htonl(*uint_ptr);
		}
		else{
			printf_syn("line data phaseC voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_VOLTAGE, &data.LineData.LVolCurveA, &data.LineData.LVolCurveB, uint_ptr)){
				printf_syn("line data  phaseC voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*uint_ptr);
			}
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_CURRENT, &data.LineData.LCurCurveA, &data.LineData.LCurCurveB, uint_ptr+1)){
			lwip_htonl(*(uint_ptr+1));
		}
		else{
			printf_syn("line data phaseC voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_CURRENT, &data.LineData.LCurCurveA, &data.LineData.LCurCurveB, uint_ptr+1)){
				printf_syn("line data  phaseC voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+1));
			}
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_ACTIVE_POWER, &data.LineData.LActPowA, &data.LineData.LActPowB, uint_ptr+2)){
			lwip_htonl(*(uint_ptr+2));
		}
		else{
			printf_syn("line data phaseC voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_ACTIVE_POWER, &data.LineData.LActPowA, &data.LineData.LActPowB, uint_ptr+2)){
				printf_syn("line data  phaseC voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+2));
			}
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_REACTIVE_POWER, &data.LineData.LReActPowA, &data.LineData.LReActPowB, uint_ptr+3)){
			lwip_htonl(*(uint_ptr+3));
		}
		else{
			printf_syn("line data phaseC voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_REACTIVE_POWER, &data.LineData.LReActPowA, &data.LineData.LReActPowB, uint_ptr+3)){
				printf_syn("line data  phaseC voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+3));
			}
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_POWER_FACTOR, &data.LineData.LPowFacA, &data.LineData.LPowFacB, uint_ptr+4)){
			lwip_htonl(*(uint_ptr+4));
		}
		else{
			printf_syn("line data phaseC voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_POWER_FACTOR, &data.LineData.LPowFacA, &data.LineData.LPowFacB, uint_ptr+4)){
				printf_syn("line data  phaseC voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+4));
			}
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_APPARENT_POWER, &data.LineData.LApparentA, &data.LineData.LApparentB, uint_ptr+5)){
			lwip_htonl(*(uint_ptr+5));
		}
		else{
			printf_syn("line data phaseC voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_APPARENT_POWER, &data.LineData.LApparentA, &data.LineData.LApparentB, uint_ptr+5)){
				printf_syn("line data  phaseC voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+5));
			}
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_PHASE, &data.LineData.LPhaseA, &data.LineData.LPhaseB, uint_ptr+6)){
			lwip_htonl(*(uint_ptr+6));
		}
		else{
			printf_syn("line data phaseC voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_PHASE, &data.LineData.LPhaseA, &data.LineData.LPhaseB, uint_ptr+6)){
				printf_syn("line data  phaseC voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+6));
			}
		}
		if(SIE_OK ==get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_FREQUENCY, &data.LineData.LFrequencyA, &data.LineData.LFrequencyB, uint_ptr+7)){
			lwip_htonl(*(uint_ptr+7));
		}
		else{
			printf_syn("line data phaseC voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], SIC_GET_FREQUENCY, &data.LineData.LFrequencyA, &data.LineData.LFrequencyB, uint_ptr+7)){
				printf_syn("line data  phaseC voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+7));
			}
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_VOLTAGE_DISTORTION, &data.LineData.LVolDistortA, &data.LineData.LVolDistortB, uint_ptr+8)){
			lwip_htonl(*(uint_ptr+8));
		}
		else{
			printf_syn("line data phaseC voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_VOLTAGE_DISTORTION, &data.LineData.LVolDistortA, &data.LineData.LVolDistortB, uint_ptr+8)){
				printf_syn("line data  phaseC voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+8));
			}
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_CURRENT_DISTORTION, &data.LineData.LCurDistortA, &data.LineData.LCurDistortB, uint_ptr+9)){
			lwip_htonl(*(uint_ptr+9));
		}
		else{
			printf_syn("line data phaseC voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_CURRENT_DISTORTION, &data.LineData.LCurDistortA, &data.LineData.LCurDistortB, uint_ptr+9)){
				printf_syn("line data  phaseC voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+9));
			}
		}
#endif
		break;

	case LT300SYS_LINEDATA_TOTAL_READ_ID:
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EMC_DEV_INFO, linedata, SINKINFO_EMC_DEV_DATA_SIZE)){
			for(i=0; i<SINKINFO_EMC_DEV_DATA_SIZE/sizeof(u32_t); i++)
				*(u32_ptr+i) = lwip_htonl(linedata[i]);
			
			//rt_memcpy(u32_ptr, linedata, SINKINFO_EMC_DEV_DATA_SIZE);
		}
		else{
			//LWIP_DEBUGF(SNMP_MIB_DEBUG,("line_data_read_voltage: errer, please read again!\n"));
			printf_syn("line data tatal voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EMC_DEV_INFO, linedata, SINKINFO_EMC_DEV_DATA_SIZE)){
 				printf_syn("line data total voltage read failed!\r\n");
			}
			else{
				for(i=0; i<SINKINFO_EMC_DEV_DATA_SIZE/sizeof(u32_t); i++)
					*(u32_ptr+i) = lwip_htonl(linedata[i]);
			
				//rt_memcpy(u32_ptr, linedata, SINKINFO_EMC_DEV_DATA_SIZE);
			}
		}
#if 0
		if(SIE_OK == get_sinkinfo_other_param(od->id_inst_ptr[2], SIC_GET_DEV_ACT_ELECTRIC_ENERGY, uint_ptr)){
			lwip_htonl(*uint_ptr);
		}
		else{
			printf_syn("line data phaseC voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_other_param(od->id_inst_ptr[2], SIC_GET_DEV_ACT_ELECTRIC_ENERGY, uint_ptr)){
				printf_syn("line data  phaseC voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*uint_ptr);
			}
		}
		if(SIE_OK == get_sinkinfo_other_param(od->id_inst_ptr[2], SIC_GET_DEV_REACT_ELECTRIC_ENERGY, uint_ptr+1)){
			lwip_htonl(*(uint_ptr+1));
		}
		else{
			printf_syn("line data phaseC voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_other_param(od->id_inst_ptr[2], SIC_GET_DEV_REACT_ELECTRIC_ENERGY, uint_ptr+1)){
				printf_syn("line data  phaseC voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+1));
			}
		}
#endif
		
		break;
	case LT300SYS_LINEDATA_PHASEA_VOL_SAMPLE_READ_ID:
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_PA_VSAMPLE, pch, SINKINFO_PX_XSAMPLE_DATA_SIZE)){
			PRIVMIB_INFO(("line data total voltage read SUCCESS!\r\n"));
		}
		else{
			//LWIP_DEBUGF(SNMP_MIB_DEBUG,("line_data_read_voltage: errer, please read again!\n"));
			printf_syn("line data  phaseC voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_PA_VSAMPLE, pch, SINKINFO_PX_XSAMPLE_DATA_SIZE)){
 				printf_syn("line data total voltage read failed!\r\n");
			}
			else{
				PRIVMIB_INFO(("line data total voltage second read SUCCESS!\r\n"));
			}
		}
#if 0		
		if(SIE_OK == get_sinkinfo_sample_data(od->id_inst_ptr[2], SIC_GET_PAV_SAMPLE_DATA, pch, SINK_INFO_PX_SAMPLE_DOT_NUM * 3)){
			//printf_syn("line data phaseA voltage sample read failed!try read again!\r\n");
		}
		else{
			printf_syn("line data phaseA voltage sample read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_sample_data(od->id_inst_ptr[2], SIC_GET_PAV_SAMPLE_DATA, pch, SINK_INFO_PX_SAMPLE_DOT_NUM * 3)){
				printf_syn("line data phaseA voltage sample read failed!\r\n");
			}
		}
#endif
		break;
	case LT300SYS_LINEDATA_PHASEA_CUR_SAMPLE_READ_ID:
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_PA_ISAMPLE, pch, SINKINFO_PX_XSAMPLE_DATA_SIZE)){
			PRIVMIB_INFO(("line data total voltage read SUCCESS!\r\n"));
		}
		else{
			//LWIP_DEBUGF(SNMP_MIB_DEBUG,("line_data_read_voltage: errer, please read again!\n"));
			printf_syn("line data  phaseC voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_PA_ISAMPLE, pch, SINKINFO_PX_XSAMPLE_DATA_SIZE)){
 				printf_syn("line data total voltage read failed!\r\n");
			}
			else{
				PRIVMIB_INFO(("line data total voltage second read SUCCESS!\r\n"));
			}
		}
#if 0
		if(SIE_OK == get_sinkinfo_sample_data(od->id_inst_ptr[2], SIC_GET_PAI_SAMPLE_DATA, pch, SINK_INFO_PX_SAMPLE_DOT_NUM * 3)){
			//printf_syn("line data phaseA voltage sample read failed!try read again!\r\n");
		}
		else{
			printf_syn("line data phaseA voltage sample read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_sample_data(od->id_inst_ptr[2], SIC_GET_PAI_SAMPLE_DATA, pch, SINK_INFO_PX_SAMPLE_DOT_NUM * 3)){
				printf_syn("line data phaseA voltage sample read failed!\r\n");
			}
		}
#endif
		break;
	case LT300SYS_LINEDATA_PHASEB_VOL_SAMPLE_READ_ID:
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_PB_VSAMPLE, pch, SINKINFO_PX_XSAMPLE_DATA_SIZE)){
			PRIVMIB_INFO(("line data total voltage read SUCCESS!\r\n"));
		}
		else{
		//LWIP_DEBUGF(SNMP_MIB_DEBUG,("line_data_read_voltage: errer, please read again!\n"));
		printf_syn("line data  phaseC voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_PB_VSAMPLE, pch, SINKINFO_PX_XSAMPLE_DATA_SIZE)){
 				printf_syn("line data total voltage read failed!\r\n");
			}
			else{
				PRIVMIB_INFO(("line data total voltage second read SUCCESS!\r\n"));
			}
		}
#if 0		
		if(SIE_OK == get_sinkinfo_sample_data(od->id_inst_ptr[2], SIC_GET_PBV_SAMPLE_DATA, pch, SINK_INFO_PX_SAMPLE_DOT_NUM * 3)){
			//printf_syn("line data phaseA voltage sample read failed!try read again!\r\n");
		}
		else{
			printf_syn("line data phaseA voltage sample read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_sample_data(od->id_inst_ptr[2], SIC_GET_PBV_SAMPLE_DATA, pch, SINK_INFO_PX_SAMPLE_DOT_NUM * 3)){
				printf_syn("line data phaseA voltage sample read failed!\r\n");
			}
		}
#endif
	//	rt_memcpy(data.LineData.LSamplePBV, pch, sizeof(data.LineData.LSamplePBV));
		break;
	case LT300SYS_LINEDATA_PHASEB_CUR_SAMPLE_READ_ID:
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_PB_ISAMPLE, pch, SINKINFO_PX_XSAMPLE_DATA_SIZE)){
			PRIVMIB_INFO(("line data total voltage read SUCCESS!\r\n"));
		}
		else{
			//LWIP_DEBUGF(SNMP_MIB_DEBUG,("line_data_read_voltage: errer, please read again!\n"));
			printf_syn("line data  phaseC voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_PB_ISAMPLE, pch, SINKINFO_PX_XSAMPLE_DATA_SIZE)){
 				printf_syn("line data total voltage read failed!\r\n");
			}
			else{
				PRIVMIB_INFO(("line data total voltage second read SUCCESS!\r\n"));
			}
		}
#if 0		
		if(SIE_OK == get_sinkinfo_sample_data(od->id_inst_ptr[2], SIC_GET_PBI_SAMPLE_DATA, pch, SINK_INFO_PX_SAMPLE_DOT_NUM * 3)){
			//printf_syn("line data phaseA voltage sample read failed!try read again!\r\n");
		}
		else{
			printf_syn("line data phaseA voltage sample read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_sample_data(od->id_inst_ptr[2], SIC_GET_PBI_SAMPLE_DATA, pch, SINK_INFO_PX_SAMPLE_DOT_NUM * 3)){
				printf_syn("line data phaseA voltage sample read failed!\r\n");
			}
		}
#endif
	//	rt_memcpy(data.LineData.LSamplePBI, pch, sizeof(data.LineData.LSamplePBI));	
		break;
	case LT300SYS_LINEDATA_PHASEC_VOL_SAMPLE_READ_ID:
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_PC_VSAMPLE, pch, SINKINFO_PX_XSAMPLE_DATA_SIZE)){
			PRIVMIB_INFO(("line data total voltage read SUCCESS!\r\n"));
		}
		else{
			//LWIP_DEBUGF(SNMP_MIB_DEBUG,("line_data_read_voltage: errer, please read again!\n"));
			printf_syn("line data  phaseC voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_PC_VSAMPLE, pch, SINKINFO_PX_XSAMPLE_DATA_SIZE)){
 				printf_syn("line data total voltage read failed!\r\n");
			}
			else{
				PRIVMIB_INFO(("line data total voltage second read SUCCESS!\r\n"));
			}
		}
#if 0
		if(SIE_OK == get_sinkinfo_sample_data(od->id_inst_ptr[2], SIC_GET_PCV_SAMPLE_DATA, pch, SINK_INFO_PX_SAMPLE_DOT_NUM * 3)){
			//printf_syn("line data phaseA voltage sample read failed!try read again!\r\n");
		}
		else{
			printf_syn("line data phaseA voltage sample read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_sample_data(od->id_inst_ptr[2], SIC_GET_PCV_SAMPLE_DATA, pch, SINK_INFO_PX_SAMPLE_DOT_NUM * 3)){
				printf_syn("line data phaseA voltage sample read failed!\r\n");
			}
		}
#endif
	//	rt_memcpy(data.LineData.LSamplePCV, pch, sizeof(data.LineData.LSamplePCV));
		break;
	case LT300SYS_LINEDATA_PHASEC_CUR_SAMPLE_READ_ID:
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_PC_ISAMPLE, pch, SINKINFO_PX_XSAMPLE_DATA_SIZE)){
			PRIVMIB_INFO(("line data total voltage read SUCCESS!\r\n"));
		}
		else{
			//LWIP_DEBUGF(SNMP_MIB_DEBUG,("line_data_read_voltage: errer, please read again!\n"));
			printf_syn("line data  phaseC voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_PC_ISAMPLE, pch, SINKINFO_PX_XSAMPLE_DATA_SIZE)){
 				printf_syn("line data total voltage read failed!\r\n");
			}
			else{
				PRIVMIB_INFO(("line data total voltage second read SUCCESS!\r\n"));
			}
		}
#if 0
		if(SIE_OK == get_sinkinfo_sample_data(od->id_inst_ptr[2], SIC_GET_PCI_SAMPLE_DATA, pch, SINK_INFO_PX_SAMPLE_DOT_NUM * 3)){
			//printf_syn("line data phaseA voltage sample read failed!try read again!\r\n");
		}
		else{
			printf_syn("line data phaseA voltage sample read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_sample_data(od->id_inst_ptr[2], SIC_GET_PCI_SAMPLE_DATA, pch, SINK_INFO_PX_SAMPLE_DOT_NUM * 3)){
				printf_syn("line data phaseA voltage sample read failed!\r\n");
			}
		}
#endif
	//	rt_memcpy(data.LineData.LSamplePCI, pch, sizeof(data.LineData.LSamplePCI));	
		break;
	case LT300SYS_LINEDATA_COPPER_IRON_LOSSES_READ_ID:
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EMC_COPPER_IRON_LOSS_INFO, linedata, SINKINFO_EMC_COPPER_IRON_LOSSES_DATA_SIZE)){
			for(i=0; i<SINKINFO_EMC_COPPER_IRON_LOSSES_DATA_SIZE/sizeof(u32_t); i++)
				*(u32_ptr+i) = lwip_htonl(linedata[i]);
			
			//rt_memcpy(u32_ptr, linedata, SINKINFO_EMC_COPPER_IRON_LOSSES_DATA_SIZE);
		}
		else{
			printf_syn("line data  phaseC voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EMC_COPPER_IRON_LOSS_INFO, linedata, SINKINFO_EMC_COPPER_IRON_LOSSES_DATA_SIZE)){
 				printf_syn("line copper and iron losses read failed!\r\n");
			}
			else{
				for(i=0; i<SINKINFO_EMC_COPPER_IRON_LOSSES_DATA_SIZE/sizeof(u32_t); i++)
					*(u32_ptr+i) = lwip_htonl(linedata[i]);
			
				//rt_memcpy(u32_ptr, linedata, SINKINFO_EMC_COPPER_IRON_LOSSES_DATA_SIZE);
			}
		}
		break;
	case LT300SYS_LINEDATA_PHASEA_VOL_HARMONIC_READ_ID:
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EMC_PA_VOL_HARMONIC_INFO, linedata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE)){
			for(i=0; i<SINKINFO_EM_PX_HARMONIC_DATA_SIZE/sizeof(u32_t); i++)
				*(u32_ptr+i) = lwip_htonl(linedata[i]);
			
			//rt_memcpy(u32_ptr, linedata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE);
		}
		else{
		
			printf_syn("line  phaseA voltage harmonic  read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EMC_PA_VOL_HARMONIC_INFO, linedata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE)){
 				printf_syn("line phaseA voltage harmonic read failed!\r\n");
			}
			else{
				for(i=0; i<SINKINFO_EM_PX_HARMONIC_DATA_SIZE/sizeof(u32_t); i++)
					*(u32_ptr+i) = lwip_htonl(linedata[i]);
			
				//rt_memcpy(u32_ptr, linedata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE);
			}
		}
		break;
	case LT300SYS_LINEDATA_PHASEA_CUR_HARMONIC_READ_ID:
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EMC_PA_CUR_HARMONIC_INFO, linedata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE)){
			for(i=0; i<SINKINFO_EM_PX_HARMONIC_DATA_SIZE/sizeof(u32_t); i++)
				*(u32_ptr+i) = lwip_htonl(linedata[i]);
			
			//rt_memcpy(u32_ptr, linedata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE);
		}
		else{
			//LWIP_DEBUGF(SNMP_MIB_DEBUG,("line_data_read_voltage: errer, please read again!\n"));
			printf_syn("line  phaseA current harmonic read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EMC_PA_CUR_HARMONIC_INFO, linedata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE)){
 				printf_syn("line phaseA current harmonicread failed!\r\n");
			}
			else{
				for(i=0; i<SINKINFO_EM_PX_HARMONIC_DATA_SIZE/sizeof(u32_t); i++)
					*(u32_ptr+i) = lwip_htonl(linedata[i]);
			
				//rt_memcpy(u32_ptr, linedata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE);
			}
		}
		break;
	case LT300SYS_LINEDATA_PHASEA_ACT_HARMONIC_READ_ID:
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EMC_PA_ACT_HARMONIC_INFO, linedata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE)){
			for(i=0; i<SINKINFO_EM_PX_HARMONIC_DATA_SIZE/sizeof(u32_t); i++)
				*(u32_ptr+i) = lwip_htonl(linedata[i]);
			
			//rt_memcpy(u32_ptr, linedata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE);
		}
		else{
			printf_syn("line phaseA active power harmonic  read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EMC_PA_ACT_HARMONIC_INFO, linedata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE)){
 				printf_syn("line phaseA active power harmonic  read failed!\r\n");
			}
			else{
				for(i=0; i<SINKINFO_EM_PX_HARMONIC_DATA_SIZE/sizeof(u32_t); i++)
					*(u32_ptr+i) = lwip_htonl(linedata[i]);
			
				//rt_memcpy(u32_ptr, linedata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE);
			}
		}
		break;
	case LT300SYS_LINEDATA_PHASEB_VOL_HARMONIC_READ_ID:
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EMC_PB_VOL_HARMONIC_INFO, linedata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE)){
			for(i=0; i<SINKINFO_EM_PX_HARMONIC_DATA_SIZE/sizeof(u32_t); i++)
				*(u32_ptr+i) = lwip_htonl(linedata[i]);
			
			//rt_memcpy(u32_ptr, linedata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE);
		}
		else{
			printf_syn("line  phaseB voltage harmonic  read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EMC_PB_VOL_HARMONIC_INFO, linedata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE)){
 				printf_syn("line phaseB voltage harmonic read failed!\r\n");
			}
			else{
				for(i=0; i<SINKINFO_EM_PX_HARMONIC_DATA_SIZE/sizeof(u32_t); i++)
					*(u32_ptr+i) = lwip_htonl(linedata[i]);
			
				//rt_memcpy(u32_ptr, linedata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE);
			}
		}
		break;
	case LT300SYS_LINEDATA_PHASEB_CUR_HARMONIC_READ_ID:
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EMC_PB_CUR_HARMONIC_INFO, linedata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE)){
			for(i=0; i<SINKINFO_EM_PX_HARMONIC_DATA_SIZE/sizeof(u32_t); i++)
				*(u32_ptr+i) = lwip_htonl(linedata[i]);
			
			//rt_memcpy(u32_ptr, linedata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE);
		}
		else{
			printf_syn("line  phaseB current harmonic read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EMC_PB_CUR_HARMONIC_INFO, linedata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE)){
 				printf_syn("line phaseB current harmonicread failed!\r\n");
			}
			else{
				for(i=0; i<SINKINFO_EM_PX_HARMONIC_DATA_SIZE/sizeof(u32_t); i++)
					*(u32_ptr+i) = lwip_htonl(linedata[i]);
			
				//rt_memcpy(u32_ptr, linedata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE);
			}
		}
		break;
	case LT300SYS_LINEDATA_PHASEB_ACT_HARMONIC_READ_ID:
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EMC_PB_ACT_HARMONIC_INFO, linedata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE)){
			for(i=0; i<SINKINFO_EM_PX_HARMONIC_DATA_SIZE/sizeof(u32_t); i++)
				*(u32_ptr+i) = lwip_htonl(linedata[i]);
			
			//rt_memcpy(u32_ptr, linedata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE);
		}
		else{
			printf_syn("line phaseB active power harmonic  read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EMC_PB_ACT_HARMONIC_INFO, linedata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE)){
 				printf_syn("line phaseB active power harmonic  read failed!\r\n");
			}
			else{
				for(i=0; i<SINKINFO_EM_PX_HARMONIC_DATA_SIZE/sizeof(u32_t); i++)
					*(u32_ptr+i) = lwip_htonl(linedata[i]);
			
				//rt_memcpy(u32_ptr, linedata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE);
			}
		}
		break;
	case LT300SYS_LINEDATA_PHASEC_VOL_HARMONIC_READ_ID:
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EMC_PC_VOL_HARMONIC_INFO, linedata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE)){
			for(i=0; i<SINKINFO_EM_PX_HARMONIC_DATA_SIZE/sizeof(u32_t); i++)
				*(u32_ptr+i) = lwip_htonl(linedata[i]);
			
			//rt_memcpy(u32_ptr, linedata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE);
		}
		else{
			
			printf_syn("line  phaseC voltage harmonic  read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EMC_PC_VOL_HARMONIC_INFO, linedata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE)){
 				printf_syn("line phaseC voltage harmonic read failed!\r\n");
			}
			else{
				for(i=0; i<SINKINFO_EM_PX_HARMONIC_DATA_SIZE/sizeof(u32_t); i++)
					*(u32_ptr+i) = lwip_htonl(linedata[i]);
			
				//rt_memcpy(u32_ptr, linedata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE);
			}
		}
		break;
	case LT300SYS_LINEDATA_PHASEC_CUR_HARMONIC_READ_ID:
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EMC_PC_CUR_HARMONIC_INFO, linedata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE)){
			for(i=0; i<SINKINFO_EM_PX_HARMONIC_DATA_SIZE/sizeof(u32_t); i++)
				*(u32_ptr+i) = lwip_htonl(linedata[i]);
			
			//rt_memcpy(u32_ptr, linedata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE);
		}
		else{
			printf_syn("line  phaseC current harmonic read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EMC_PC_CUR_HARMONIC_INFO, linedata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE)){
 				printf_syn("line phaseC current harmonicread failed!\r\n");
			}
			else{
				for(i=0; i<SINKINFO_EM_PX_HARMONIC_DATA_SIZE/sizeof(u32_t); i++)
					*(u32_ptr+i) = lwip_htonl(linedata[i]);
			
				//rt_memcpy(u32_ptr, linedata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE);
			}
		}
		break;
	case LT300SYS_LINEDATA_PHASEC_ACT_HARMONIC_READ_ID:
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EMC_PC_ACT_HARMONIC_INFO, linedata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE)){
			for(i=0; i<SINKINFO_EM_PX_HARMONIC_DATA_SIZE/sizeof(u32_t); i++)
				*(u32_ptr+i) = lwip_htonl(linedata[i]);
			
			//rt_memcpy(u32_ptr, linedata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE);
		}
		else{
			printf_syn("line phaseC active power harmonic  read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EMC_PC_ACT_HARMONIC_INFO, linedata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE)){
 				printf_syn("line phaseC active power harmonic  read failed!\r\n");
			}
			else{
				for(i=0; i<SINKINFO_EM_PX_HARMONIC_DATA_SIZE/sizeof(u32_t); i++)
					*(u32_ptr+i) = lwip_htonl(linedata[i]);
			
				//rt_memcpy(u32_ptr, linedata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE);
			}
		}
		break;
	case LT300SYS_LINEDATA_READ_ALARM_ID: /* mark by David */
		ocstrncpy(pch, data.lcur_alarm, sizeof(data.lcur_alarm));
		break;

	case LT300SYS_LINEDATA_READ_ALARM_MASK_ID:
		ocstrncpy(pch, data.lalarm_mask, sizeof(data.lalarm_mask));
		break;
		
	default:
		break;
	}

	return;
}

static u8_t yjlt300sys_linedata_read_entry_set_test(struct obj_def *od, u16_t len, void *value)
{
	u8_t id, set_ok;

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	set_ok = 0;
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_LINEDATA_PHASEA_READ_ID:	
	case LT300SYS_LINEDATA_PHASEB_READ_ID:	
	case LT300SYS_LINEDATA_PHASEC_READ_ID:	
	case LT300SYS_LINEDATA_TOTAL_READ_ID:
	case LT300SYS_LINEDATA_READ_ALARM_MASK_ID:	
#if 0
		if ((0==*sint_ptr) || (1==*sint_ptr))
			set_ok = 1;
#else
		set_ok = 1;
#endif
		break;

	default:
		break;
	}

	return set_ok;
}

static void yjlt300sys_linedata_read_entry_set_value(struct obj_def *od, u16_t len, void *value)
{
	//u32_t *uint_ptr =(u32_t *) value;
	u8_t *pch = (u8_t *)value;
	struct collect_line_data_info_st data;
	u8_t id;
    
	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	/* @todo @fixme: which kind of pointer is 'value'? s32_t or u8_t??? */
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_LINEDATA_PHASEA_READ_ID:	
	case LT300SYS_LINEDATA_PHASEB_READ_ID:	
	case LT300SYS_LINEDATA_PHASEC_READ_ID:	
	case LT300SYS_LINEDATA_TOTAL_READ_ID:
		break;
		
	case LT300SYS_LINEDATA_READ_ALARM_MASK_ID:
		ocstrncpy(data.lalarm_mask, pch, sizeof(data.lalarm_mask));
		break;

	default:
		break;
	}

	return;
}

/*
 * meter data collecting
 */
static void yjlt300sys_meterdata_read_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
{
	/* return to object name, adding index depth (1) */
	ident_len += 2;
	ident     -= 2;

	if (ident_len == 3) {
		u8_t id;

		od->id_inst_len = ident_len;
		od->id_inst_ptr = ident;

		LWIP_ASSERT("invalid id", (ident[0] >= 0) && (ident[0] <= 0xff));

		id = (u8_t)ident[0];
		od->instance = MIB_OBJECT_TAB;
		switch (id) {
		case LT300SYS_METER_INDEX_ID:
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = sizeof(u32_t);
			break;
		case LT300SYS_METER_ADDR_ID:
			
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC| SNMP_ASN1_PRIMIT | SNMP_ASN1_OPAQUE);
			od->v_len    = 12;
			break;
		case LT300SYS_METER_SNID_ID:
		
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC| SNMP_ASN1_PRIMIT | SNMP_ASN1_OPAQUE);
			od->v_len    = 12;
			break;
		case LT300SYS_METER_PROTOCAL_TYPE_ID:
			
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV| SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = sizeof(u32_t);
			break;
		case LT300SYS_METER_WIRE_CONN_MODE_ID:
			
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV| SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = sizeof(u32_t);
			break;	
		case LT300SYS_METER_CALIBRATE_TIME_VALID_ID:
		
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV| SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = sizeof(u32_t);
			break;
		case LT300SYS_METER_CALIBRATE_TIME_VALUE_ID:
			
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV| SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 12;
			break;
		case LT300SYS_METER_DATE_TIME_VALUE_ID:
		
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV| SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 7;
			break;
		case LT300SYS_METERDATA_PHASEA_READ_ID:
		case LT300SYS_METERDATA_PHASEB_READ_ID:
		case LT300SYS_METERDATA_PHASEC_READ_ID:
			
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 52;
			break;

		case LT300SYS_METERDATA_TOTAL_READ_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 100;
			break;
		case LT300SYS_METERDATA_MAX_DEMAND_DATA_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 90;
			break;
		case LT300SYS_METERDATA_COPPER_IRON_LOSSES_DATA_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 32;
			break;
		case LT300SYS_METER_TEMPERATURE_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 2;
			break;
		case LT300SYS_METER_CLOCK_BATTERY_VOL_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 4;
			break;
		case LT300SYS_METER_COLLECT_BATTERY_VOL_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 4;
			break;
		case LT300SYS_METERDATA_PHASEA_VOL_HARMONIC_READ_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 84;
			break;

		case LT300SYS_METERDATA_PHASEA_CUR_HARMONIC_READ_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 84;
			break;
		case LT300SYS_METERDATA_PHASEB_VOL_HARMONIC_READ_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 84;
			break;
		case LT300SYS_METERDATA_PHASEB_CUR_HARMONIC_READ_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 84;
			break;
		case LT300SYS_METERDATA_PHASEC_VOL_HARMONIC_READ_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 84;
			break;
		case LT300SYS_METERDATA_PHASEC_CUR_HARMONIC_READ_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 84;
			break;	
		case LT300SYS_METERDATA_ACT_PULSE_TIME_OUT_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = sizeof(u32_t);
			break;
		case LT300SYS_METERDATA_REACT_PULSE_TIME_OUT_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = sizeof(u32_t);
			break;
		case LT300SYS_METERDATA_READ_ALARM_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 4;
			break;
		case LT300SYS_METERDATA_READ_ALARM_MASK_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 4;
			break;

		default:
			LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no such object\n"));
			od->instance = MIB_OBJECT_NONE;
			break;
		}
	} else {
		LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no scalar\n"));
		od->instance = MIB_OBJECT_NONE;
	}

	return;
}

static void yjlt300sys_meterdata_read_entry_get_value(struct obj_def *od, u16_t len, void *value)
{	
	u32_t *u32_ptr = (u32_t *)value;
	u8_t *pch = (u8_t *)value;
	char *snid =(char *)value;
	struct collect_meter_data_info_st data;
	//struct electric_meter_reg_info_st amm_sn;
	//struct celectric_meter_config_info_st amm_conf;
	//struct gateway_em_st	 em_info;
	struct sinkinfo_em_dev_st em_ptr;
	struct sinkinfo_em_max_demand_st mde_ptr;
	u8_t id,i;
	u32_t meterdata[32]={0};
	u8_t chardata[12]={0};
	
	rt_memset(meterdata, 0, sizeof(meterdata));

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_METER_INDEX_ID:
		*u32_ptr = od->id_inst_ptr[2];
		break;		
	case LT300SYS_METER_ADDR_ID:
		/*if (SUCC == get_em_reg_info(&amm_sn)) {
			rt_memcpy(snid, amm_sn.em_sn[od->id_inst_ptr[2]-1], DEV_SN_MODE_LEN);
		} else {
			printf_syn("%s(), read meter SN data tbl fail\n", __FUNCTION__);
		}*/
		get_dev_sn_em_sn(SDT_ELECTRIC_METER, snid, DEV_SN_MODE_LEN, od->id_inst_ptr[2]-1);
		break;
	case LT300SYS_METER_SNID_ID:	
		/*if (SUCC == get_em_reg_info(&amm_sn)) {
			rt_memcpy(snid, amm_sn.em_sn[od->id_inst_ptr[2]-1], DEV_SN_MODE_LEN);
		} else {
			printf_syn("%s(), read meter SN data tbl fail\n", __FUNCTION__);
		}*/
		get_dev_sn_em_sn(SDT_ELECTRIC_METER, snid, DEV_SN_MODE_LEN, od->id_inst_ptr[2]-1);
		break;
	case LT300SYS_METER_PROTOCAL_TYPE_ID:
		if(SIE_OK == get_em_proto(od->id_inst_ptr[2]-1, SIC_GET_EM_PROTOCAL_TYPE, &meterdata[0])){
			*u32_ptr = meterdata[0];
		}
		else{
			printf_syn("meter protocol type read failed!try read again!\r\n");
			if(SIE_OK != get_em_proto(od->id_inst_ptr[2]-1, SIC_GET_EM_PROTOCAL_TYPE, &meterdata[0])){
				printf_syn("meter protocal type read failed!\r\n");
			}
			else{
				*u32_ptr = meterdata[0];
			}
		}	
		break;
	case LT300SYS_METER_WIRE_CONN_MODE_ID:
		if(SIE_OK == get_em_proto(od->id_inst_ptr[2]-1, SIC_GET_EM_WIRE_CONNECT_MODE, &meterdata[0])){
			*u32_ptr = meterdata[0];
		}
		else{
			printf_syn("meter wire connect mode read failed!try read again!\r\n");
			if(SIE_OK != get_em_proto(od->id_inst_ptr[2]-1, SIC_GET_EM_WIRE_CONNECT_MODE, &meterdata[0])){
				printf_syn("meter wire connect mode read failed!\r\n");
			}
			else{
				*u32_ptr = meterdata[0];
			}
		}
		break;	
	case LT300SYS_METER_CALIBRATE_TIME_VALID_ID:
		/*if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
		}	
		*u32_ptr = (rt_uint32_t)amm_conf.clibrate_time_enable[od->id_inst_ptr[2]-1];*/
		if(SUCC == get_em_info(od->id_inst_ptr[2]-1, 3, meterdata, chardata)){
			*u32_ptr = meterdata[0];
		}
		else{
			printf_syn("%s(), get em info tbl fail\n", __FUNCTION__);
		}
		break;	
	case LT300SYS_METER_CALIBRATE_TIME_VALUE_ID:
		if(SUCC == get_em_info(od->id_inst_ptr[2]-1, 1, meterdata, pch)){
			/*for(i=0; i<6; i++)
				lwip_htonl(meterdata[i]);
			
			rt_memcpy(u32_ptr, meterdata, sizeof(struct ammeter_time));	*/
		}
		else{
			printf_syn("%s(), get em info tbl fail\n", __FUNCTION__);
		}
		break;	
	case LT300SYS_METER_DATE_TIME_VALUE_ID:
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EM_DEV_INFO, &em_ptr, SINKINFO_EM_DEV_DATA_SIZE)){
			rt_memcpy(pch, em_ptr.em_date_time, sizeof(em_ptr.em_date_time)-1);	
		}
		else{
			printf_syn("em data  phaseC PARAM read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EM_DEV_INFO, &em_ptr, SINKINFO_EM_DEV_DATA_SIZE)){
 				printf_syn("em data phaseC voltage read failed!\r\n");
			}
			else{
				rt_memcpy(pch, em_ptr.em_date_time, sizeof(em_ptr.em_date_time)-1);	
				PRIVMIB_INFO(("em data phaseC voltage second read SUCCESS!\r\n"));
			}
		}			
		break;	

	case LT300SYS_METERDATA_PHASEA_READ_ID:
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EM_PA_INFO, meterdata, SINKINFO_EM_PX_DATA_SIZE)){
			for(i=0; i<SINKINFO_EM_PX_DATA_SIZE/sizeof(u32_t); i++){
				if(meterdata[i] == INVLIDE_DATAL){
						*(u32_ptr+i) = lwip_htonl(meterdata[i]);

				}else{
						*(u32_ptr+i) = lwip_htonl(conv_4byte_bcd_to_long(meterdata[i]));
				}
			}
			//rt_memcpy(u32_ptr, meterdata, SINKINFO_EM_PX_DATA_SIZE);
		}
		else{
			//LWIP_DEBUGF(SNMP_MIB_DEBUG,("line_data_read_voltage: errer, please read again!\n"));
			printf_syn("EM data  phaseA PARAM read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EM_PA_INFO, meterdata, SINKINFO_EM_PX_DATA_SIZE)){
 				printf_syn("EM data phaseA voltage read failed!\r\n");
			}
			else{
				for(i=0; i<SINKINFO_EM_PX_DATA_SIZE/sizeof(u32_t); i++){
					if(meterdata[i] == INVLIDE_DATAL){
						*(u32_ptr+i) = lwip_htonl(meterdata[i]);

					}else{
						*(u32_ptr+i) = lwip_htonl(conv_4byte_bcd_to_long(meterdata[i]));
					}
				}
				//rt_memcpy(u32_ptr, meterdata, SINKINFO_EM_PX_DATA_SIZE);
			}
		}
#if 0		
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_EM_VOLTAGE, uint_ptr, &data.MeterData.VolCurveB, &data.MeterData.VolCurveC)){
			lwip_htonl(*uint_ptr);
		}
		else{
			printf_syn("meter data phaseA voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_EM_VOLTAGE, uint_ptr, &data.MeterData.VolCurveB, &data.MeterData.VolCurveC)){
				printf_syn("meter data  phaseA voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*uint_ptr);
			}
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_EM_CURRENT, uint_ptr+1, &data.MeterData.CurCurveB, &data.MeterData.CurCurveC)){
			lwip_htonl(*(uint_ptr+1));
		}
		else{
			printf_syn("meter data phaseA voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_EM_CURRENT, uint_ptr+1, &data.MeterData.CurCurveB, &data.MeterData.CurCurveC)){
				printf_syn("meter data  phaseA voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+1));
			}
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_EM_ACTIVE_POWER, uint_ptr+2, &data.MeterData.ActPowB, &data.MeterData.ActPowC)){
			lwip_htonl(*(uint_ptr+2));
		}
		else{
			printf_syn("meter data phaseA voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_EM_ACTIVE_POWER, uint_ptr+2, &data.MeterData.ActPowB, &data.MeterData.ActPowC)){
				printf_syn("meter data  phaseA voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+2));
			}
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_EM_REACTIVE_POWER, uint_ptr+3, &data.MeterData.ReActPowB, &data.MeterData.ReActPowC)){
			lwip_htonl(*(uint_ptr+3));
		}
		else{
			printf_syn("meter data phaseA voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_EM_REACTIVE_POWER, uint_ptr+3, &data.MeterData.ReActPowB, &data.MeterData.ReActPowC)){
				printf_syn("meter data  phaseA voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+3));
			}
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_EM_POWER_FACTOR, uint_ptr+4, &data.MeterData.PowFacB, &data.MeterData.PowFacC)){
			lwip_htonl(*(uint_ptr+4));
		}
		else{
			printf_syn("meter data phaseA voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_EM_POWER_FACTOR, uint_ptr+4, &data.MeterData.PowFacB, &data.MeterData.PowFacC)){
				printf_syn("meter data  phaseA voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+4));
			}
		}
#endif	
		break;

	case LT300SYS_METERDATA_PHASEB_READ_ID:
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EM_PB_INFO, meterdata, SINKINFO_EM_PX_DATA_SIZE)){
			for(i=0; i<SINKINFO_EM_PX_DATA_SIZE/sizeof(u32_t); i++){
				if(meterdata[i] == INVLIDE_DATAL){
						*(u32_ptr+i) = lwip_htonl(meterdata[i]);

				}else{
						*(u32_ptr+i) = lwip_htonl(conv_4byte_bcd_to_long(meterdata[i]));
				}
			}
			//rt_memcpy(u32_ptr, meterdata, SINKINFO_EM_PX_DATA_SIZE);	
		}
		else{
			//LWIP_DEBUGF(SNMP_MIB_DEBUG,("line_data_read_voltage: errer, please read again!\n"));
			printf_syn("EM data  phaseB PARAM read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EM_PB_INFO, meterdata, SINKINFO_EM_PX_DATA_SIZE)){
 				printf_syn("EM data phaseB voltage read failed!\r\n");
			}
			else{
				for(i=0; i<SINKINFO_EM_PX_DATA_SIZE/sizeof(u32_t); i++){
					if(meterdata[i] == INVLIDE_DATAL){
						*(u32_ptr+i) = lwip_htonl(meterdata[i]);

					}else{
						*(u32_ptr+i) = lwip_htonl(conv_4byte_bcd_to_long(meterdata[i]));
					}
				}
				//rt_memcpy(u32_ptr, meterdata, SINKINFO_EM_PX_DATA_SIZE);	
			}
		}
#if 0		
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_EM_VOLTAGE, &data.MeterData.VolCurveA, uint_ptr, &data.MeterData.VolCurveC)){
			lwip_htonl(*uint_ptr);
		}
		else{
			printf_syn("meter data phaseB voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_EM_VOLTAGE, &data.MeterData.VolCurveA, uint_ptr, &data.MeterData.VolCurveC)){
				printf_syn("meter data  phaseB voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*uint_ptr);
			}
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_EM_CURRENT, &data.MeterData.CurCurveA, uint_ptr+1, &data.MeterData.CurCurveC)){
			lwip_htonl(*(uint_ptr+1));
		}
		else{
			printf_syn("meter data phaseB voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_EM_CURRENT, &data.MeterData.CurCurveA, uint_ptr+1, &data.MeterData.CurCurveC)){
				printf_syn("meter data  phaseB voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+1));
			}
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_EM_ACTIVE_POWER, &data.MeterData.ActPowA, uint_ptr+2, &data.MeterData.ActPowC)){
			lwip_htonl(*(uint_ptr+2));
		}
		else{
			printf_syn("meter data phaseB voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_EM_ACTIVE_POWER, &data.MeterData.ActPowA, uint_ptr+2, &data.MeterData.ActPowC)){
				printf_syn("meter data  phaseB voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+2));
			}
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_EM_REACTIVE_POWER, &data.MeterData.ReActPowA, uint_ptr+3, &data.MeterData.ReActPowC)){
			lwip_htonl(*(uint_ptr+3));
		}
		else{
			printf_syn("meter data phaseB voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_EM_REACTIVE_POWER, &data.MeterData.ReActPowA, uint_ptr+3, &data.MeterData.ReActPowC)){
				printf_syn("meter data  phaseB voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+3));
			}
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_EM_POWER_FACTOR, &data.MeterData.PowFacA, uint_ptr+4, &data.MeterData.PowFacC)){
			lwip_htonl(*(uint_ptr+4));
		}
		else{
			printf_syn("meter data phaseB voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_EM_POWER_FACTOR, &data.MeterData.PowFacA, uint_ptr+4, &data.MeterData.PowFacC)){
				printf_syn("meter data  phaseB voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+4));
			}
		}
#endif
		break;

	case LT300SYS_METERDATA_PHASEC_READ_ID:
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EM_PC_INFO, meterdata, SINKINFO_EM_PX_DATA_SIZE)){
			for(i=0; i<SINKINFO_EM_PX_DATA_SIZE/sizeof(u32_t); i++){
				if(meterdata[i] == INVLIDE_DATAL){
						*(u32_ptr+i) = lwip_htonl(meterdata[i]);

				}else{
						*(u32_ptr+i) = lwip_htonl(conv_4byte_bcd_to_long(meterdata[i]));
				}
			}
			//rt_memcpy(u32_ptr, meterdata, SINKINFO_EM_PX_DATA_SIZE);	
		}
		else{
			printf_syn("em data  phaseC PARAM read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EM_PC_INFO, meterdata, SINKINFO_EM_PX_DATA_SIZE)){
 				printf_syn("em data phaseC voltage read failed!\r\n");
			}
			else{
				for(i=0; i<SINKINFO_EM_PX_DATA_SIZE/sizeof(u32_t); i++){
					if(meterdata[i] == INVLIDE_DATAL){
						*(u32_ptr+i) = lwip_htonl(meterdata[i]);

					}else{
						*(u32_ptr+i) = lwip_htonl(conv_4byte_bcd_to_long(meterdata[i]));
					}
				}
				//rt_memcpy(u32_ptr, meterdata, SINKINFO_EM_PX_DATA_SIZE);	
			}
		}
#if 0		
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_EM_VOLTAGE, &data.MeterData.VolCurveA, &data.MeterData.VolCurveB, uint_ptr)){
			lwip_htonl(*uint_ptr);
		}
		else{
			printf_syn("meter data phaseC voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_EM_VOLTAGE, &data.MeterData.VolCurveA, &data.MeterData.VolCurveB, uint_ptr)){
				printf_syn("meter data  phaseC voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*uint_ptr);
			}
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_EM_CURRENT, &data.MeterData.CurCurveA, &data.MeterData.CurCurveB, uint_ptr+1)){
			lwip_htonl(*(uint_ptr+1));
		}
		else{
			printf_syn("meter data phaseC voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_EM_CURRENT, &data.MeterData.CurCurveA, &data.MeterData.CurCurveB, uint_ptr+1)){
				printf_syn("meter data  phaseC voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+1));
			}
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_EM_ACTIVE_POWER, &data.MeterData.ActPowA, &data.MeterData.ActPowB, uint_ptr+2)){
			lwip_htonl(*(uint_ptr+2));
		}
		else{
			printf_syn("meter data phaseC voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_EM_ACTIVE_POWER, &data.MeterData.ActPowA, &data.MeterData.ActPowB, uint_ptr+2)){
				printf_syn("meter data  phaseC voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+2));
			}
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_EM_REACTIVE_POWER, &data.MeterData.ReActPowA, &data.MeterData.ReActPowB, uint_ptr+3)){
			lwip_htonl(*(uint_ptr+3));
		}
		else{
			printf_syn("meter data phaseC voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_EM_REACTIVE_POWER, &data.MeterData.ReActPowA, &data.MeterData.ReActPowB, uint_ptr+3)){
				printf_syn("meter data  phaseC voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+3));
			}
		}
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_EM_POWER_FACTOR, &data.MeterData.PowFacA, &data.MeterData.PowFacB, uint_ptr+4)){
			lwip_htonl(*(uint_ptr+4));
		}
		else{
			printf_syn("meter data phaseC voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2], 0, SIC_GET_EM_POWER_FACTOR, &data.MeterData.PowFacA, &data.MeterData.PowFacB, uint_ptr+4)){
				printf_syn("meter data  phaseC voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+4));
			}
		}
#endif	
		break;

	case LT300SYS_METERDATA_TOTAL_READ_ID:
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EM_DEV_INFO, &em_ptr, SINKINFO_EM_DEV_DATA_SIZE)){
			if(em_ptr.em_act_total_energy == INVLIDE_DATAL){
				meterdata[0] = lwip_htonl(em_ptr.em_act_total_energy);
			}else{
				meterdata[0] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_act_total_energy));
			}
			if(em_ptr.em_act_rate1_energy == INVLIDE_DATAL){
				meterdata[1] = lwip_htonl(em_ptr.em_act_rate1_energy);
			}else{
				meterdata[1] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_act_rate1_energy));
			}
			if(em_ptr.em_act_rate2_energy == INVLIDE_DATAL){
				meterdata[2] = lwip_htonl(em_ptr.em_act_rate2_energy);
			}else{
				meterdata[2] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_act_rate2_energy));
			}
			if(em_ptr.em_act_rate3_energy == INVLIDE_DATAL){
				meterdata[3] = lwip_htonl(em_ptr.em_act_rate3_energy);
			}else{
				meterdata[3] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_act_rate3_energy));
			}
			if(em_ptr.em_act_rate4_energy == INVLIDE_DATAL){
				meterdata[4] = lwip_htonl(em_ptr.em_act_rate4_energy);
			}else{
				meterdata[4] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_act_rate4_energy));
			}

			if(em_ptr.em_reverse_act_total_energy == INVLIDE_DATAL){
				meterdata[5] = lwip_htonl(em_ptr.em_reverse_act_total_energy);
			}else{
				meterdata[5] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_reverse_act_total_energy));
			}
			if(em_ptr.em_reverse_act_rate1_energy == INVLIDE_DATAL){
				meterdata[6] = lwip_htonl(em_ptr.em_reverse_act_rate1_energy);
			}else{
				meterdata[6] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_reverse_act_rate1_energy));
			}
			if(em_ptr.em_reverse_act_rate2_energy == INVLIDE_DATAL){
				meterdata[7] = lwip_htonl(em_ptr.em_reverse_act_rate2_energy);
			}else{
				meterdata[7] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_reverse_act_rate2_energy));
			}
			if(em_ptr.em_reverse_act_rate3_energy == INVLIDE_DATAL){
				meterdata[8] = lwip_htonl(em_ptr.em_reverse_act_rate3_energy);
			}else{
				meterdata[8] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_reverse_act_rate3_energy));
			}
			if(em_ptr.em_reverse_act_rate4_energy == INVLIDE_DATAL){
				meterdata[9] = lwip_htonl(em_ptr.em_reverse_act_rate4_energy);
			}else{
				meterdata[9] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_reverse_act_rate4_energy));
			}
			if(em_ptr.em_react_total_energy == INVLIDE_DATAL){
				meterdata[10] = lwip_htonl(em_ptr.em_react_total_energy);
			}else{
				meterdata[10] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_react_total_energy));
			}
			if(em_ptr.em_react_rate1_energy == INVLIDE_DATAL){
				meterdata[11] = lwip_htonl(em_ptr.em_react_rate1_energy);
			}else{
				meterdata[11] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_react_rate1_energy));
			}
			if(em_ptr.em_react_rate2_energy == INVLIDE_DATAL){
				meterdata[12] = lwip_htonl(em_ptr.em_react_rate2_energy);
			}else{
				meterdata[12] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_react_rate2_energy));
			}
			if(em_ptr.em_react_rate3_energy == INVLIDE_DATAL){
				meterdata[13] = lwip_htonl(em_ptr.em_react_rate3_energy);
			}else{
				meterdata[13] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_react_rate3_energy));
			}
			if(em_ptr.em_react_rate4_energy == INVLIDE_DATAL){
				meterdata[14] = lwip_htonl(em_ptr.em_react_rate4_energy);
			}else{
				meterdata[14] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_react_rate4_energy));
			}
			if(em_ptr.em_reverse_react_total_energy == INVLIDE_DATAL){
				meterdata[15] = lwip_htonl(em_ptr.em_reverse_react_total_energy);
			}else{
				meterdata[15] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_reverse_react_total_energy));
			}
			if(em_ptr.em_reverse_react_rate1_energy == INVLIDE_DATAL){
				meterdata[16] = lwip_htonl(em_ptr.em_reverse_react_rate1_energy);
			}else{
				meterdata[16] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_reverse_react_rate1_energy));
			}
			if(em_ptr.em_reverse_react_rate2_energy == INVLIDE_DATAL){
				meterdata[17] = lwip_htonl(em_ptr.em_reverse_react_rate2_energy);
			}else{
				meterdata[17] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_reverse_react_rate2_energy));
			}
			if(em_ptr.em_reverse_react_rate3_energy == INVLIDE_DATAL){
				meterdata[18] = lwip_htonl(em_ptr.em_reverse_react_rate3_energy);
			}else{
				meterdata[18] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_reverse_react_rate3_energy));
			}
			if(em_ptr.em_reverse_react_rate4_energy == INVLIDE_DATAL){
				meterdata[19] = lwip_htonl(em_ptr.em_reverse_react_rate4_energy);
			}else{
				meterdata[19] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_reverse_react_rate4_energy));
			}
			if(em_ptr.em_combin_act_total_energy == INVLIDE_DATAL){
				meterdata[20] = lwip_htonl(em_ptr.em_combin_act_total_energy);
			}else{
				meterdata[20] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_combin_act_total_energy));
			}
			if(em_ptr.em_combin_act_rate1_energy == INVLIDE_DATAL){
				meterdata[21] = lwip_htonl(em_ptr.em_combin_act_rate1_energy);
			}else{
				meterdata[21] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_combin_act_rate1_energy));
			}
			if(em_ptr.em_combin_act_rate2_energy == INVLIDE_DATAL){
				meterdata[22] = lwip_htonl(em_ptr.em_combin_act_rate2_energy);
			}else{
				meterdata[22] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_combin_act_rate2_energy));
			}
			if(em_ptr.em_combin_act_rate3_energy == INVLIDE_DATAL){
				meterdata[23] = lwip_htonl(em_ptr.em_combin_act_rate3_energy);
			}else{
				meterdata[23] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_combin_act_rate3_energy));
			}
			if(em_ptr.em_combin_act_rate4_energy == INVLIDE_DATAL){
				meterdata[24] = lwip_htonl(em_ptr.em_combin_act_rate4_energy);
			}else{
				meterdata[24] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_combin_act_rate4_energy));
			}

			rt_memcpy(u32_ptr, meterdata, sizeof(em_ptr.em_act_total_energy)*25);
		}
		else{
			printf_syn("em data  phaseC PARAM read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EM_DEV_INFO, &em_ptr, SINKINFO_EM_DEV_DATA_SIZE)){
 				printf_syn("em data phaseC voltage read failed!\r\n");
			}
			else{
				if(em_ptr.em_act_total_energy == INVLIDE_DATAL){
					meterdata[0] = lwip_htonl(em_ptr.em_act_total_energy);
				}else{
					meterdata[0] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_act_total_energy));
				}
				if(em_ptr.em_act_rate1_energy == INVLIDE_DATAL){
					meterdata[1] = lwip_htonl(em_ptr.em_act_rate1_energy);
				}else{
					meterdata[1] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_act_rate1_energy));
				}
				if(em_ptr.em_act_rate2_energy == INVLIDE_DATAL){
					meterdata[2] = lwip_htonl(em_ptr.em_act_rate2_energy);
				}else{
					meterdata[2] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_act_rate2_energy));
				}
				if(em_ptr.em_act_rate3_energy == INVLIDE_DATAL){
					meterdata[3] = lwip_htonl(em_ptr.em_act_rate3_energy);
				}else{
					meterdata[3] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_act_rate3_energy));
				}
				if(em_ptr.em_act_rate4_energy == INVLIDE_DATAL){
					meterdata[4] = lwip_htonl(em_ptr.em_act_rate4_energy);
				}else{
					meterdata[4] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_act_rate4_energy));
				}

				if(em_ptr.em_reverse_act_total_energy == INVLIDE_DATAL){
					meterdata[5] = lwip_htonl(em_ptr.em_reverse_act_total_energy);
				}else{
					meterdata[5] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_reverse_act_total_energy));
				}
				if(em_ptr.em_reverse_act_rate1_energy == INVLIDE_DATAL){
					meterdata[6] = lwip_htonl(em_ptr.em_reverse_act_rate1_energy);
				}else{
					meterdata[6] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_reverse_act_rate1_energy));
				}
				if(em_ptr.em_reverse_act_rate2_energy == INVLIDE_DATAL){
					meterdata[7] = lwip_htonl(em_ptr.em_reverse_act_rate2_energy);
				}else{
					meterdata[7] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_reverse_act_rate2_energy));
				}
				if(em_ptr.em_reverse_act_rate3_energy == INVLIDE_DATAL){
					meterdata[8] = lwip_htonl(em_ptr.em_reverse_act_rate3_energy);
				}else{
					meterdata[8] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_reverse_act_rate3_energy));
				}
				if(em_ptr.em_reverse_act_rate4_energy == INVLIDE_DATAL){
					meterdata[9] = lwip_htonl(em_ptr.em_reverse_act_rate4_energy);
				}else{
					meterdata[9] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_reverse_act_rate4_energy));
				}
				if(em_ptr.em_react_total_energy == INVLIDE_DATAL){
					meterdata[10] = lwip_htonl(em_ptr.em_react_total_energy);
				}else{
					meterdata[10] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_react_total_energy));
				}
				if(em_ptr.em_react_rate1_energy == INVLIDE_DATAL){
					meterdata[11] = lwip_htonl(em_ptr.em_react_rate1_energy);
				}else{
					meterdata[11] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_react_rate1_energy));
				}
				if(em_ptr.em_react_rate2_energy == INVLIDE_DATAL){
					meterdata[12] = lwip_htonl(em_ptr.em_react_rate2_energy);
				}else{
					meterdata[12] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_react_rate2_energy));
				}
				if(em_ptr.em_react_rate3_energy == INVLIDE_DATAL){
					meterdata[13] = lwip_htonl(em_ptr.em_react_rate3_energy);
				}else{
					meterdata[13] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_react_rate3_energy));
				}
				if(em_ptr.em_react_rate4_energy == INVLIDE_DATAL){
					meterdata[14] = lwip_htonl(em_ptr.em_react_rate4_energy);
				}else{
					meterdata[14] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_react_rate4_energy));
				}
				if(em_ptr.em_reverse_react_total_energy == INVLIDE_DATAL){
					meterdata[15] = lwip_htonl(em_ptr.em_reverse_react_total_energy);
				}else{
					meterdata[15] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_reverse_react_total_energy));
				}
				if(em_ptr.em_reverse_react_rate1_energy == INVLIDE_DATAL){
					meterdata[16] = lwip_htonl(em_ptr.em_reverse_react_rate1_energy);
				}else{
					meterdata[16] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_reverse_react_rate1_energy));
				}
				if(em_ptr.em_reverse_react_rate2_energy == INVLIDE_DATAL){
					meterdata[17] = lwip_htonl(em_ptr.em_reverse_react_rate2_energy);
				}else{
					meterdata[17] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_reverse_react_rate2_energy));
				}
				if(em_ptr.em_reverse_react_rate3_energy == INVLIDE_DATAL){
					meterdata[18] = lwip_htonl(em_ptr.em_reverse_react_rate3_energy);
				}else{
					meterdata[18] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_reverse_react_rate3_energy));
				}
				if(em_ptr.em_reverse_react_rate4_energy == INVLIDE_DATAL){
					meterdata[19] = lwip_htonl(em_ptr.em_reverse_react_rate4_energy);
				}else{
					meterdata[19] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_reverse_react_rate4_energy));
				}
				if(em_ptr.em_combin_act_total_energy == INVLIDE_DATAL){
					meterdata[20] = lwip_htonl(em_ptr.em_combin_act_total_energy);
				}else{
					meterdata[20] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_combin_act_total_energy));
				}
				if(em_ptr.em_combin_act_rate1_energy == INVLIDE_DATAL){
					meterdata[21] = lwip_htonl(em_ptr.em_combin_act_rate1_energy);
				}else{
					meterdata[21] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_combin_act_rate1_energy));
				}
				if(em_ptr.em_combin_act_rate2_energy == INVLIDE_DATAL){
					meterdata[22] = lwip_htonl(em_ptr.em_combin_act_rate2_energy);
				}else{
					meterdata[22] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_combin_act_rate2_energy));
				}
				if(em_ptr.em_combin_act_rate3_energy == INVLIDE_DATAL){
					meterdata[23] = lwip_htonl(em_ptr.em_combin_act_rate3_energy);
				}else{
					meterdata[23] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_combin_act_rate3_energy));
				}
				if(em_ptr.em_combin_act_rate4_energy == INVLIDE_DATAL){
					meterdata[24] = lwip_htonl(em_ptr.em_combin_act_rate4_energy);
				}else{
					meterdata[24] = lwip_htonl(conv_4byte_bcd_to_long(em_ptr.em_combin_act_rate4_energy));
				}

				rt_memcpy(u32_ptr, meterdata, sizeof(em_ptr.em_act_total_energy)*25);
		
			}
		}
#if 0
		if(SIE_OK == get_sinkinfo_other_param(od->id_inst_ptr[2], SIC_GET_EM_ACT_ELECTRIC_ENERGY, uint_ptr)){
			lwip_htonl(*uint_ptr);
		}
		else{
			printf_syn("meter data total voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_other_param(od->id_inst_ptr[2], SIC_GET_EM_ACT_ELECTRIC_ENERGY, uint_ptr)){
				printf_syn("meter data  total voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*uint_ptr);
			}
		}
		if(SIE_OK == get_sinkinfo_other_param(od->id_inst_ptr[2], SIC_GET_EM_REACT_ELECTRIC_ENERGY, uint_ptr+1)){
			lwip_htonl(*(uint_ptr+1));
		}
		else{
			printf_syn("meter data total voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_other_param(od->id_inst_ptr[2], SIC_GET_EM_REACT_ELECTRIC_ENERGY, uint_ptr+1)){
				printf_syn("meter data  total voltage read failed!\r\n");
			}
			else{
				lwip_htonl(*(uint_ptr+1));
			}
		}
#endif	
		break;
		
	case LT300SYS_METERDATA_MAX_DEMAND_DATA_ID: 
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EM_MAX_DEMAND_INFO, &mde_ptr, SINKINFO_EM_MAX_DEMAND_DATA_SIZE)){
			if(mde_ptr.act_max_demand_total == INVLIDE_DATAL){
				meterdata[0] = lwip_htonl(mde_ptr.act_max_demand_total);
			}else{
				meterdata[0] = lwip_htonl(conv_4byte_bcd_to_long(mde_ptr.act_max_demand_total));
			}
			if(mde_ptr.act_max_demand_rate1 == INVLIDE_DATAL){
				meterdata[1] = lwip_htonl(mde_ptr.act_max_demand_rate1);
			}else{
				meterdata[1] = lwip_htonl(conv_4byte_bcd_to_long(mde_ptr.act_max_demand_rate1));
			}
			if(mde_ptr.act_max_demand_rate2 == INVLIDE_DATAL){
				meterdata[2] = lwip_htonl(mde_ptr.act_max_demand_rate2);
			}else{
				meterdata[2] = lwip_htonl(conv_4byte_bcd_to_long(mde_ptr.act_max_demand_rate2));
			}
			if(mde_ptr.act_max_demand_rate3 == INVLIDE_DATAL){
				meterdata[3] = lwip_htonl(mde_ptr.act_max_demand_rate3);
			}else{
				meterdata[3] = lwip_htonl(conv_4byte_bcd_to_long(mde_ptr.act_max_demand_rate3));
			}
			if(mde_ptr.act_max_demand_rate4 == INVLIDE_DATAL){
				meterdata[4] = lwip_htonl(mde_ptr.act_max_demand_rate4);
			}else{
				meterdata[4] = lwip_htonl(conv_4byte_bcd_to_long(mde_ptr.act_max_demand_rate4));
			}
			if(mde_ptr.react_max_demand_total == INVLIDE_DATAL){
				meterdata[5] = lwip_htonl(mde_ptr.react_max_demand_total);
			}else{
				meterdata[5] = lwip_htonl(conv_4byte_bcd_to_long(mde_ptr.react_max_demand_total));
			}
			if(mde_ptr.react_max_demand_rate1 == INVLIDE_DATAL){
				meterdata[6] = lwip_htonl(mde_ptr.react_max_demand_rate1);
			}else{
				meterdata[6] = lwip_htonl(conv_4byte_bcd_to_long(mde_ptr.react_max_demand_rate1));
			}
			if(mde_ptr.react_max_demand_rate2 == INVLIDE_DATAL){
				meterdata[7] = lwip_htonl(mde_ptr.react_max_demand_rate2);
			}else{
				meterdata[7] = lwip_htonl(conv_4byte_bcd_to_long(mde_ptr.react_max_demand_rate2));
			}
			if(mde_ptr.react_max_demand_rate3 == INVLIDE_DATAL){
				meterdata[8] = lwip_htonl(mde_ptr.react_max_demand_rate3);
			}else{
				meterdata[8] = lwip_htonl(conv_4byte_bcd_to_long(mde_ptr.react_max_demand_rate3));
			}
			if(mde_ptr.react_max_demand_rate4 == INVLIDE_DATAL){
				meterdata[9] = lwip_htonl(mde_ptr.react_max_demand_rate4);
			}else{
				meterdata[9] = lwip_htonl(conv_4byte_bcd_to_long(mde_ptr.react_max_demand_rate4));
			}
			rt_memcpy(pch, (u8_t *)&meterdata[0], sizeof(mde_ptr.act_max_demand_total));
			rt_memcpy(pch+4, mde_ptr.act_max_demand_time_total, sizeof(mde_ptr.act_max_demand_time_total)-3);
			rt_memcpy(pch+9, (u8_t *)&meterdata[1], sizeof(mde_ptr.act_max_demand_rate1));
			rt_memcpy(pch+13, mde_ptr.act_max_demand_time_rate1, sizeof(mde_ptr.act_max_demand_time_rate1)-3);
			rt_memcpy(pch+18, (u8_t *)&meterdata[2], sizeof(mde_ptr.act_max_demand_rate2));
			rt_memcpy(pch+22, mde_ptr.act_max_demand_time_rate2, sizeof(mde_ptr.act_max_demand_time_rate2)-3);
			rt_memcpy(pch+27, (u8_t *)&meterdata[3], sizeof(mde_ptr.act_max_demand_rate3));
			rt_memcpy(pch+31, mde_ptr.act_max_demand_time_rate3, sizeof(mde_ptr.act_max_demand_time_rate3)-3);
			rt_memcpy(pch+36, (u8_t *)&meterdata[4], sizeof(mde_ptr.act_max_demand_rate4));
			rt_memcpy(pch+40, mde_ptr.act_max_demand_time_rate4, sizeof(mde_ptr.act_max_demand_time_rate4)-3);
			rt_memcpy(pch+45, (u8_t *)&meterdata[5], sizeof(mde_ptr.react_max_demand_total));
			rt_memcpy(pch+49, mde_ptr.react_max_demand_time_total, sizeof(mde_ptr.react_max_demand_time_total)-3);
			rt_memcpy(pch+54, (u8_t *)&meterdata[6], sizeof(mde_ptr.react_max_demand_rate1));
			rt_memcpy(pch+58, mde_ptr.react_max_demand_time_rate1, sizeof(mde_ptr.react_max_demand_time_rate1)-3);
			rt_memcpy(pch+63, (u8_t *)&meterdata[7], sizeof(mde_ptr.react_max_demand_rate2));
			rt_memcpy(pch+67, mde_ptr.react_max_demand_time_rate2, sizeof(mde_ptr.react_max_demand_time_rate2)-3);
			rt_memcpy(pch+72, (u8_t *)&meterdata[8], sizeof(mde_ptr.react_max_demand_rate3));
			rt_memcpy(pch+76, mde_ptr.react_max_demand_time_rate3, sizeof(mde_ptr.react_max_demand_time_rate3)-3);
			rt_memcpy(pch+81, (u8_t *)&meterdata[9], sizeof(mde_ptr.react_max_demand_rate4));
			rt_memcpy(pch+85, mde_ptr.react_max_demand_time_rate4, sizeof(mde_ptr.react_max_demand_time_rate4)-3);
			
		}
		else{
			printf_syn("EM timing freeze data read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EM_MAX_DEMAND_INFO, &mde_ptr, SINKINFO_EM_MAX_DEMAND_DATA_SIZE)){
 				printf_syn("EM MAX data read failed!\r\n");
			}
			else{
				if(mde_ptr.act_max_demand_total == INVLIDE_DATAL){
					meterdata[0] = lwip_htonl(mde_ptr.act_max_demand_total);
				}else{
					meterdata[0] = lwip_htonl(conv_4byte_bcd_to_long(mde_ptr.act_max_demand_total));
				}
				if(mde_ptr.act_max_demand_rate1 == INVLIDE_DATAL){
					meterdata[1] = lwip_htonl(mde_ptr.act_max_demand_rate1);
				}else{
					meterdata[1] = lwip_htonl(conv_4byte_bcd_to_long(mde_ptr.act_max_demand_rate1));
				}
				if(mde_ptr.act_max_demand_rate2 == INVLIDE_DATAL){
					meterdata[2] = lwip_htonl(mde_ptr.act_max_demand_rate2);
				}else{
					meterdata[2] = lwip_htonl(conv_4byte_bcd_to_long(mde_ptr.act_max_demand_rate2));
				}
				if(mde_ptr.act_max_demand_rate3 == INVLIDE_DATAL){
					meterdata[3] = lwip_htonl(mde_ptr.act_max_demand_rate3);
				}else{
					meterdata[3] = lwip_htonl(conv_4byte_bcd_to_long(mde_ptr.act_max_demand_rate3));
				}
				if(mde_ptr.act_max_demand_rate4 == INVLIDE_DATAL){
					meterdata[4] = lwip_htonl(mde_ptr.act_max_demand_rate4);
				}else{
					meterdata[4] = lwip_htonl(conv_4byte_bcd_to_long(mde_ptr.act_max_demand_rate4));
				}
				if(mde_ptr.react_max_demand_total == INVLIDE_DATAL){
					meterdata[5] = lwip_htonl(mde_ptr.react_max_demand_total);
				}else{
					meterdata[5] = lwip_htonl(conv_4byte_bcd_to_long(mde_ptr.react_max_demand_total));
				}
				if(mde_ptr.react_max_demand_rate1 == INVLIDE_DATAL){
					meterdata[6] = lwip_htonl(mde_ptr.react_max_demand_rate1);
				}else{
					meterdata[6] = lwip_htonl(conv_4byte_bcd_to_long(mde_ptr.react_max_demand_rate1));
				}
				if(mde_ptr.react_max_demand_rate2 == INVLIDE_DATAL){
					meterdata[7] = lwip_htonl(mde_ptr.react_max_demand_rate2);
				}else{
					meterdata[7] = lwip_htonl(conv_4byte_bcd_to_long(mde_ptr.react_max_demand_rate2));
				}
				if(mde_ptr.react_max_demand_rate3 == INVLIDE_DATAL){
					meterdata[8] = lwip_htonl(mde_ptr.react_max_demand_rate3);
				}else{
					meterdata[8] = lwip_htonl(conv_4byte_bcd_to_long(mde_ptr.react_max_demand_rate3));
				}
				if(mde_ptr.react_max_demand_rate4 == INVLIDE_DATAL){
					meterdata[9] = lwip_htonl(mde_ptr.react_max_demand_rate4);
				}else{
					meterdata[9] = lwip_htonl(conv_4byte_bcd_to_long(mde_ptr.react_max_demand_rate4));
				}
				rt_memcpy(pch, (u8_t *)&meterdata[0], sizeof(mde_ptr.act_max_demand_total));
				rt_memcpy(pch+4, mde_ptr.act_max_demand_time_total, sizeof(mde_ptr.act_max_demand_time_total)-3);
				rt_memcpy(pch+9, (u8_t *)&meterdata[1], sizeof(mde_ptr.act_max_demand_rate1));
				rt_memcpy(pch+13, mde_ptr.act_max_demand_time_rate1, sizeof(mde_ptr.act_max_demand_time_rate1)-3);
				rt_memcpy(pch+18, (u8_t *)&meterdata[2], sizeof(mde_ptr.act_max_demand_rate2));
				rt_memcpy(pch+22, mde_ptr.act_max_demand_time_rate2, sizeof(mde_ptr.act_max_demand_time_rate2)-3);
				rt_memcpy(pch+27, (u8_t *)&meterdata[3], sizeof(mde_ptr.act_max_demand_rate3));
				rt_memcpy(pch+31, mde_ptr.act_max_demand_time_rate3, sizeof(mde_ptr.act_max_demand_time_rate3)-3);
				rt_memcpy(pch+36, (u8_t *)&meterdata[4], sizeof(mde_ptr.act_max_demand_rate4));
				rt_memcpy(pch+40, mde_ptr.act_max_demand_time_rate4, sizeof(mde_ptr.act_max_demand_time_rate4)-3);
				rt_memcpy(pch+45, (u8_t *)&meterdata[5], sizeof(mde_ptr.react_max_demand_total));
				rt_memcpy(pch+49, mde_ptr.react_max_demand_time_total, sizeof(mde_ptr.react_max_demand_time_total)-3);
				rt_memcpy(pch+54, (u8_t *)&meterdata[6], sizeof(mde_ptr.react_max_demand_rate1));
				rt_memcpy(pch+58, mde_ptr.react_max_demand_time_rate1, sizeof(mde_ptr.react_max_demand_time_rate1)-3);
				rt_memcpy(pch+63, (u8_t *)&meterdata[7], sizeof(mde_ptr.react_max_demand_rate2));
				rt_memcpy(pch+67, mde_ptr.react_max_demand_time_rate2, sizeof(mde_ptr.react_max_demand_time_rate2)-3);
				rt_memcpy(pch+72, (u8_t *)&meterdata[8], sizeof(mde_ptr.react_max_demand_rate3));
				rt_memcpy(pch+76, mde_ptr.react_max_demand_time_rate3, sizeof(mde_ptr.react_max_demand_time_rate3)-3);
				rt_memcpy(pch+81, (u8_t *)&meterdata[9], sizeof(mde_ptr.react_max_demand_rate4));
				rt_memcpy(pch+85, mde_ptr.react_max_demand_time_rate4, sizeof(mde_ptr.react_max_demand_time_rate4)-3);
			
			}
		}				
		break;
		
	case LT300SYS_METERDATA_COPPER_IRON_LOSSES_DATA_ID: 
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EM_COPPER_IRON_LOSS_INFO, meterdata, SINKINFO_EM_COPPER_IRON_LOSSES_DATA_SIZE)){
			for(i=0; i<SINKINFO_EM_COPPER_IRON_LOSSES_DATA_SIZE/sizeof(u32_t); i++){
				if(meterdata[i] == INVLIDE_DATAL){
					*(u32_ptr+i) = lwip_htonl(meterdata[i]);

				}else{
					*(u32_ptr+i) = lwip_htonl(conv_4byte_bcd_to_long(meterdata[i]));
				}
			//rt_memcpy(u32_ptr, meterdata, SINKINFO_EM_COPPER_IRON_LOSSES_DATA_SIZE);	
			}
		}
		else{
			//LWIP_DEBUGF(SNMP_MIB_DEBUG,("line_data_read_voltage: errer, please read again!\n"));
			printf_syn("EM timing freeze data read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EM_COPPER_IRON_LOSS_INFO, meterdata, SINKINFO_EM_COPPER_IRON_LOSSES_DATA_SIZE)){
 				printf_syn("EM timing freeze data read failed!\r\n");
			}
			else{
				for(i=0; i<SINKINFO_EM_COPPER_IRON_LOSSES_DATA_SIZE/sizeof(u32_t); i++){
					if(meterdata[i] == INVLIDE_DATAL){
						*(u32_ptr+i) = lwip_htonl(meterdata[i]);

					}else{
						*(u32_ptr+i) = lwip_htonl(conv_4byte_bcd_to_long(meterdata[i]));
					}
				//rt_memcpy(u32_ptr, meterdata, SINKINFO_EM_COPPER_IRON_LOSSES_DATA_SIZE);	
				}
			}
		}				
		break;

	case LT300SYS_METER_TEMPERATURE_ID: 
		if(SIE_OK == get_sinkinfo_other_param(od->id_inst_ptr[2]-1, SIC_GET_EM_METER_TEMPERATURE, meterdata)){
			//*u32_ptr = (meterdata[0] >> 16);
			rt_memcpy(chardata, (u8_t *)&meterdata[0], sizeof(meterdata[0]));
			*pch = chardata[2];
			*(pch+1) = chardata[3];
		}
		else{
			printf_syn("meter data total voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_other_param(od->id_inst_ptr[2]-1, SIC_GET_EM_METER_TEMPERATURE, meterdata)){
				printf_syn("meter data  total voltage read failed!\r\n");
			}
			else{
				//*u32_ptr = (meterdata[0] >> 16);
				rt_memcpy(chardata, (u8_t *)&meterdata[0], sizeof(meterdata[0]));
				*pch = chardata[2];
				*(pch+1) = chardata[3];
			}
		}		
		break;
	case LT300SYS_METER_CLOCK_BATTERY_VOL_ID: 
		if(SIE_OK == get_sinkinfo_other_param(od->id_inst_ptr[2]-1, SIC_GET_EM_CLOCK_BATTERY_VOL, u32_ptr)){
			//lwip_htonl(*u32_ptr);
		}
		else{
			printf_syn("meter data total voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_other_param(od->id_inst_ptr[2]-1, SIC_GET_EM_CLOCK_BATTERY_VOL, u32_ptr)){
				printf_syn("meter data  total voltage read failed!\r\n");
			}
			else{
				//lwip_htonl(*u32_ptr);
			}
		}		
		break;
	case LT300SYS_METER_COLLECT_BATTERY_VOL_ID:
		if(SIE_OK == get_sinkinfo_other_param(od->id_inst_ptr[2]-1, SIC_GET_EM_METER_COLLECT_BATTERY_VOL, u32_ptr)){
			//lwip_htonl(*u32_ptr);
		}
		else{
			printf_syn("meter data total voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_other_param(od->id_inst_ptr[2]-1, SIC_GET_EM_METER_COLLECT_BATTERY_VOL, u32_ptr)){
				printf_syn("meter data  total voltage read failed!\r\n");
			}
			else{
				//lwip_htonl(*u32_ptr);
			}
		}		
		break;
	case LT300SYS_METERDATA_PHASEA_VOL_HARMONIC_READ_ID: 
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EM_PA_VOL_HARMONIC_INFO, meterdata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE)){
			for(i=0; i<SINKINFO_EM_PX_HARMONIC_DATA_SIZE/sizeof(u32_t); i++){
				if(meterdata[i] == INVLIDE_DATAL){
						*(u32_ptr+i) = lwip_htonl(meterdata[i]);

				}else{
						*(u32_ptr+i) = lwip_htonl(conv_4byte_bcd_to_long(meterdata[i]));
				}
			}
			//rt_memcpy(u32_ptr, meterdata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE);
		}
		else{
			printf_syn("EM timing freeze data read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EM_PA_VOL_HARMONIC_INFO, meterdata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE)){
 				printf_syn("EM timing freeze data read failed!\r\n");
			}
			else{
				for(i=0; i<SINKINFO_EM_PX_HARMONIC_DATA_SIZE/sizeof(u32_t); i++){
					if(meterdata[i] == INVLIDE_DATAL){
						*(u32_ptr+i) = lwip_htonl(meterdata[i]);

					}else{
						*(u32_ptr+i) = lwip_htonl(conv_4byte_bcd_to_long(meterdata[i]));
					}
				}
				//rt_memcpy(u32_ptr, meterdata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE);
			}
		}				
		break;
	case LT300SYS_METERDATA_PHASEA_CUR_HARMONIC_READ_ID: 
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EM_PA_CUR_HARMONIC_INFO, meterdata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE)){
			for(i=0; i<SINKINFO_EM_PX_HARMONIC_DATA_SIZE/sizeof(u32_t); i++){
				if(meterdata[i] == INVLIDE_DATAL){
						*(u32_ptr+i) = lwip_htonl(meterdata[i]);

				}else{
						*(u32_ptr+i) = lwip_htonl(conv_4byte_bcd_to_long(meterdata[i]));
				}	
			}
			//rt_memcpy(u32_ptr, meterdata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE);

		}
		else{
			printf_syn("EM timing freeze data read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EM_PA_CUR_HARMONIC_INFO, meterdata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE)){
 				printf_syn("EM timing freeze data read failed!\r\n");
			}
			else{
				for(i=0; i<SINKINFO_EM_PX_HARMONIC_DATA_SIZE/sizeof(u32_t); i++){
					if(meterdata[i] == INVLIDE_DATAL){
						*(u32_ptr+i) = lwip_htonl(meterdata[i]);

					}else{
						*(u32_ptr+i) = lwip_htonl(conv_4byte_bcd_to_long(meterdata[i]));
					}
				}
				//rt_memcpy(u32_ptr, meterdata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE);
			}
		}				
		break;
	case LT300SYS_METERDATA_PHASEB_VOL_HARMONIC_READ_ID: 
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EM_PB_VOL_HARMONIC_INFO, meterdata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE)){
			for(i=0; i<SINKINFO_EM_PX_HARMONIC_DATA_SIZE/sizeof(u32_t); i++){
				if(meterdata[i] == INVLIDE_DATAL){
						*(u32_ptr+i) = lwip_htonl(meterdata[i]);

				}else{
						*(u32_ptr+i) = lwip_htonl(conv_4byte_bcd_to_long(meterdata[i]));
				}
			}
			//rt_memcpy(u32_ptr, meterdata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE);
		}
		else{
			printf_syn("EM timing freeze data read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EM_PB_VOL_HARMONIC_INFO, meterdata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE)){
 				printf_syn("EM timing freeze data read failed!\r\n");
			}
			else{
				for(i=0; i<SINKINFO_EM_PX_HARMONIC_DATA_SIZE/sizeof(u32_t); i++){
					if(meterdata[i] == INVLIDE_DATAL){
						*(u32_ptr+i) = lwip_htonl(meterdata[i]);

					}else{
						*(u32_ptr+i) = lwip_htonl(conv_4byte_bcd_to_long(meterdata[i]));
					}
				}
				//rt_memcpy(u32_ptr, meterdata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE);
			}
		}				
		break;
	case LT300SYS_METERDATA_PHASEB_CUR_HARMONIC_READ_ID: 
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EM_PB_CUR_HARMONIC_INFO, meterdata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE)){
			for(i=0; i<SINKINFO_EM_PX_HARMONIC_DATA_SIZE/sizeof(u32_t); i++){
				if(meterdata[i] == INVLIDE_DATAL){
						*(u32_ptr+i) = lwip_htonl(meterdata[i]);

				}else{
						*(u32_ptr+i) = lwip_htonl(conv_4byte_bcd_to_long(meterdata[i]));
				}	
			}
			//rt_memcpy(u32_ptr, meterdata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE);
		}
		else{
			//LWIP_DEBUGF(SNMP_MIB_DEBUG,("line_data_read_voltage: errer, please read again!\n"));
			printf_syn("EM timing freeze data read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EM_PB_CUR_HARMONIC_INFO, meterdata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE)){
 				printf_syn("EM timing freeze data read failed!\r\n");
			}
			else{
				for(i=0; i<SINKINFO_EM_PX_HARMONIC_DATA_SIZE/sizeof(u32_t); i++){
					if(meterdata[i] == INVLIDE_DATAL){
						*(u32_ptr+i) = lwip_htonl(meterdata[i]);

					}else{
						*(u32_ptr+i) = lwip_htonl(conv_4byte_bcd_to_long(meterdata[i]));
					}	
				}
				//rt_memcpy(u32_ptr, meterdata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE);
			}
		}				
		break;
	case LT300SYS_METERDATA_PHASEC_VOL_HARMONIC_READ_ID: 
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EM_PC_VOL_HARMONIC_INFO, meterdata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE)){
			for(i=0; i<SINKINFO_EM_PX_HARMONIC_DATA_SIZE/sizeof(u32_t); i++){
				if(meterdata[i] == INVLIDE_DATAL){
						*(u32_ptr+i) = lwip_htonl(meterdata[i]);

				}else{
						*(u32_ptr+i) = lwip_htonl(conv_4byte_bcd_to_long(meterdata[i]));
				}	
			}
			//rt_memcpy(u32_ptr, meterdata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE);
		}
		else{
			printf_syn("EM timing freeze data read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EM_PC_VOL_HARMONIC_INFO, meterdata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE)){
 				printf_syn("EM timing freeze data read failed!\r\n");
			}
			else{
				for(i=0; i<SINKINFO_EM_PX_HARMONIC_DATA_SIZE/sizeof(u32_t); i++){
					if(meterdata[i] == INVLIDE_DATAL){
						*(u32_ptr+i) = lwip_htonl(meterdata[i]);

					}else{
						*(u32_ptr+i) = lwip_htonl(conv_4byte_bcd_to_long(meterdata[i]));
					}
				}
				//rt_memcpy(u32_ptr, meterdata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE);
			}
		}				
		break;
	case LT300SYS_METERDATA_PHASEC_CUR_HARMONIC_READ_ID: 
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EM_PC_CUR_HARMONIC_INFO, meterdata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE)){
			for(i=0; i<SINKINFO_EM_PX_HARMONIC_DATA_SIZE/sizeof(u32_t); i++){
				if(meterdata[i] == INVLIDE_DATAL){
						*(u32_ptr+i) = lwip_htonl(meterdata[i]);

				}else{
						*(u32_ptr+i) = lwip_htonl(conv_4byte_bcd_to_long(meterdata[i]));
				}
			}
			//rt_memcpy(u32_ptr, meterdata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE);
		}
		else{
			printf_syn("EM timing freeze data read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, 0, SI_MGC_GET_EM_PC_CUR_HARMONIC_INFO, meterdata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE)){
 				printf_syn("EM timing freeze data read failed!\r\n");
			}
			else{
				for(i=0; i<SINKINFO_EM_PX_HARMONIC_DATA_SIZE/sizeof(u32_t); i++){
					if(meterdata[i] == INVLIDE_DATAL){
						*(u32_ptr+i) = lwip_htonl(meterdata[i]);

					}else{
						*(u32_ptr+i) = lwip_htonl(conv_4byte_bcd_to_long(meterdata[i]));
					}
				}
				//rt_memcpy(u32_ptr, meterdata, SINKINFO_EM_PX_HARMONIC_DATA_SIZE);
			}
		}				
		break;	
	case LT300SYS_METERDATA_ACT_PULSE_TIME_OUT_ID:
		*u32_ptr = check_eenergy_state;
		break;

	case LT300SYS_METERDATA_REACT_PULSE_TIME_OUT_ID:
		*u32_ptr = check_eenergy_state;
		break;
	case LT300SYS_METERDATA_READ_ALARM_ID: /* mark by David */
		ocstrncpy(pch, data.mcur_alarm, sizeof(data.mcur_alarm));
		break;

	case LT300SYS_METERDATA_READ_ALARM_MASK_ID:
		ocstrncpy(pch, data.malarm_mask, sizeof(data.malarm_mask));
		break;
		
	default:
		break;
	}

	return;
}

static u8_t yjlt300sys_meterdata_read_entry_set_test(struct obj_def *od, u16_t len, void *value)
{
	u8_t id, set_ok;

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	set_ok = 0;
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_METER_WIRE_CONN_MODE_ID:
	case LT300SYS_METER_PROTOCAL_TYPE_ID:
	case LT300SYS_METER_CALIBRATE_TIME_VALID_ID:
	case LT300SYS_METER_CALIBRATE_TIME_VALUE_ID:
	case LT300SYS_METER_ADDR_ID:
	case LT300SYS_METER_SNID_ID:	
	case LT300SYS_METERDATA_READ_ALARM_MASK_ID:	
#if 0
		if ((0==*sint_ptr) || (1==*sint_ptr))
			set_ok = 1;
#else
		set_ok = 1;
#endif
		break;

	default:
		break;
	}

	return set_ok;
}

static void yjlt300sys_meterdata_read_entry_set_value(struct obj_def *od, u16_t len, void *value)
{
	u32_t *uint_ptr = (u32_t *)value;
	u8_t *pch = (u8_t *)value;
	char *snid = (char *)value;
	struct collect_meter_data_info_st data;
	struct ammeter_time time;
	u32_t meterdata[12];
	//struct gateway_em_st	 em_info;
	//struct celectric_meter_config_info_st amm_conf;
	u8_t id;
    char pt_sn[DEV_SN_MODE_LEN+1]={'\0'};
	char ct_sn[DEV_SN_MODE_LEN+1]={'\0'};
	char ct1_sn[DEV_SN_MODE_LEN+1]={'\0'};
	u8_t chardata[16]={0};
	char port;
	
	
	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	/* @todo @fixme: which kind of pointer is 'value'? s32_t or u8_t??? */
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_METER_ADDR_ID:
		get_dev_sn_em_sn(SDT_PT, pt_sn, DEV_SN_MODE_LEN, od->id_inst_ptr[2]-1);
		get_dev_sn_em_sn(SDT_CT, ct_sn, DEV_SN_MODE_LEN, od->id_inst_ptr[2]-1);
		get_dev_sn_em_sn(SDT_CT1, ct1_sn, DEV_SN_MODE_LEN, od->id_inst_ptr[2]-1);
		get_dev_sn_em_sn(SDT_MASTER_PT, &port, DEV_SN_MODE_LEN, od->id_inst_ptr[2]-1);
		reg_em(od->id_inst_ptr[2]-1, snid, port, pt_sn, ct_sn, ct1_sn);
		break;
	case LT300SYS_METER_SNID_ID:
		get_dev_sn_em_sn(SDT_PT, pt_sn, DEV_SN_MODE_LEN, od->id_inst_ptr[2]-1);
		get_dev_sn_em_sn(SDT_CT, ct_sn, DEV_SN_MODE_LEN, od->id_inst_ptr[2]-1);
		get_dev_sn_em_sn(SDT_CT1, ct1_sn, DEV_SN_MODE_LEN, od->id_inst_ptr[2]-1);
		get_dev_sn_em_sn(SDT_MASTER_PT, &port, DEV_SN_MODE_LEN, od->id_inst_ptr[2]-1);
		reg_em(od->id_inst_ptr[2]-1, snid, port, pt_sn, ct_sn, ct1_sn);
		break;
	case LT300SYS_METER_PROTOCAL_TYPE_ID:
		set_em_info(od->id_inst_ptr[2]-1, 10, uint_ptr, pch);
		set_em_proto(od->id_inst_ptr[2]-1, SIC_GET_EM_PROTOCAL_TYPE, uint_ptr);
		break;
	case LT300SYS_METER_WIRE_CONN_MODE_ID:
		/*if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_GW_EM_INFO, 0, &em_info)) {
			printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
		}
		em_info.chlx_st[od->id_inst_ptr[2]-1].connect33 = *uint_ptr;*/
		set_em_info(od->id_inst_ptr[2]-1, 2, uint_ptr, pch);
		set_em_proto(od->id_inst_ptr[2]-1, SIC_GET_EM_PROTOCAL_TYPE, uint_ptr);
		break;	
	case LT300SYS_METER_CALIBRATE_TIME_VALID_ID:
		if(*uint_ptr == 1){
			get_em_info(od->id_inst_ptr[2]-1, 1, meterdata,chardata);
			time.year = (chardata[0]-0x30)*10 + (chardata[1]-0x30);
			time.month = (chardata[2]-0x30)*10 + (chardata[3]-0x30);
			time.day = (chardata[4]-0x30)*10 + (chardata[5]-0x30);
			time.hour = (chardata[6]-0x30)*10 + (chardata[7]-0x30);
			time.minite = (chardata[8]-0x30)*10 + (chardata[9]-0x30);
			time.seconds = (chardata[10]-0x30)*10 + (chardata[11]-0x30);
			//rt_memcpy(&time, (rt_uint32_t *)chardata, sizeof(struct ammeter_time));
			set_em_info(od->id_inst_ptr[2]-1, 3, uint_ptr, pch);
			setting_all_ammeter_time(&time, RS485_PORT_USED_BY_645);
		}else {
			set_em_info(od->id_inst_ptr[2]-1, 3, uint_ptr, pch);
		}
		break;
	case LT300SYS_METER_CALIBRATE_TIME_VALUE_ID:
		/*if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
		}
		rt_memcpy(&amm_conf.em_timing, (struct ammeter_time *)value, sizeof(struct ammeter_time));
		write_syscfgdata_tbl(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf);
		syscfgdata_syn_proc();*/
		set_em_info(od->id_inst_ptr[2]-1, 1, uint_ptr, pch);
		break;

		
	case LT300SYS_METERDATA_READ_ALARM_MASK_ID:
		ocstrncpy(data.malarm_mask, pch, sizeof(data.malarm_mask));
		break;

	default:
		break;
	}


	return;
}

/*
 * meter momentary freeze data collecting
 */
static void yjlt300sys_meter_momentfreezedata_read_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
{
	/* return to object name, adding index depth (1) */
	ident_len += 2;
	ident     -= 2;

	if (ident_len == 3) {
		u8_t id;

		od->id_inst_len = ident_len;
		od->id_inst_ptr = ident;

		LWIP_ASSERT("invalid id", (ident[0] >= 0) && (ident[0] <= 0xff));

		id = (u8_t)ident[0];
		od->instance = MIB_OBJECT_TAB;
		switch (id) {
		case LT300SYS_METER_MOMENT_FREEZE_TIMES_ID:
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = sizeof(u32_t);
			break;
		case LT300SYS_METER_MOMENT_FREEZE_MAX_TIMES_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = sizeof(u32_t);
			break;
		case LT300SYS_METER_MOMENT_FREEZE_TIME_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 8;
			break;
		case LT300SYS_METER_MOMENT_FREEZE_ENABLE_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = 4;
			break;
		case LT300SYS_METER_MOMENT_FREEZE_DATA_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 63;
			break;
		case LT300SYS_METER_MOMENT_FREEZE_DATA_READ_ALARM_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = 4;
			break;
		case LT300SYS_METER_MOMENT_FREEZE_DATA_READ_ALARM_MASK_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = 4;
			break;

		default:
			LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no such object\n"));
			od->instance = MIB_OBJECT_NONE;
			break;
		}
	} else {
		LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no scalar\n"));
		od->instance = MIB_OBJECT_NONE;
	}

	return;
}

static void yjlt300sys_meter_momentfreezedata_read_entry_get_value(struct obj_def *od, u16_t len, void *value)
{
	u32_t *u32_ptr = (u32_t *)value;
	u8_t *pch = (u8_t *)value;
	//struct gateway_em_st	 *em_info;
	//struct celectric_meter_config_info_st amm_conf;
	struct collect_meter_data_info_st data;
	struct sinkinfo_em_momentary_freeze_st mfz_ptr;
	u8_t id;
	//u8_t i;
	u32_t freezedata[32]= {0};
	u8_t chardata[12]= {0};

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_METER_MOMENT_FREEZE_TIMES_ID:
		if(SUCC == get_em_info(od->id_inst_ptr[2]-1, 4, freezedata, chardata)){
			*u32_ptr = freezedata[0];
		}
		else{
			printf_syn("%s(), get em info tbl fail\n", __FUNCTION__);
		}
		break;		
	case LT300SYS_METER_MOMENT_FREEZE_MAX_TIMES_ID:
		*u32_ptr = INVLIDE_DATAS;
		break;
	case LT300SYS_METER_MOMENT_FREEZE_TIME_ID:
		if(SUCC == get_em_info(od->id_inst_ptr[2]-1, 6, freezedata, pch)){
			/*for(i=0; i<6; i++)
				lwip_htonl(freezedata[i]);
			
			rt_memcpy(u32_ptr, &freezedata[1], sizeof(struct ammeter_time)-2);	*/
		}
		else{
			printf_syn("%s(), get em info tbl fail\n", __FUNCTION__);
		}
		break;
	case LT300SYS_METER_MOMENT_FREEZE_ENABLE_ID:
		if(SUCC == get_em_info(od->id_inst_ptr[2]-1, 8, freezedata, chardata)){
			*u32_ptr = freezedata[0];
		}
		else{
			printf_syn("%s(), get em info tbl fail\n", __FUNCTION__);
		}
		break;

	case LT300SYS_METER_MOMENT_FREEZE_DATA_ID:
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_MOMENT_FREEZE_INFO, &mfz_ptr, SINKINFO_EM_MOMENT_FREEZE_DATA_SIZE)){
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
			rt_memcpy(pch, mfz_ptr.freeze_time, sizeof(mfz_ptr.freeze_time)-3);
			rt_memcpy(pch+5, (u8_t *)&freezedata[0], sizeof(mfz_ptr.act_elec_energy));
			rt_memcpy(pch+9, (u8_t *)&freezedata[1], sizeof(mfz_ptr.reverse_act_elec_energy));
			rt_memcpy(pch+13, (u8_t *)&freezedata[2], sizeof(mfz_ptr.apxT));
			rt_memcpy(pch+17, (u8_t *)&freezedata[3], sizeof(mfz_ptr.apxA));
			rt_memcpy(pch+21, (u8_t *)&freezedata[4], sizeof(mfz_ptr.apxB));
			rt_memcpy(pch+25, (u8_t *)&freezedata[5], sizeof(mfz_ptr.apxC));
			rt_memcpy(pch+29, (u8_t *)&freezedata[6], sizeof(mfz_ptr.rapxT));
			rt_memcpy(pch+33, (u8_t *)&freezedata[7], sizeof(mfz_ptr.rapxA));
			rt_memcpy(pch+37, (u8_t *)&freezedata[8], sizeof(mfz_ptr.rapxB));
			rt_memcpy(pch+41, (u8_t *)&freezedata[9], sizeof(mfz_ptr.rapxC));
			rt_memcpy(pch+45, (u8_t *)&freezedata[10], sizeof(mfz_ptr.act_max_demand));
			rt_memcpy(pch+49, mfz_ptr.act_max_demand_time, sizeof(mfz_ptr.act_max_demand_time)-3);
			rt_memcpy(pch+54, (u8_t *)&freezedata[11], sizeof(mfz_ptr.reverse_act_max_demand));
			rt_memcpy(pch+58, mfz_ptr.reverse_act_max_demand_time, sizeof(mfz_ptr.reverse_act_max_demand_time)-3);
		}
		else{
			//LWIP_DEBUGF(SNMP_MIB_DEBUG,("line_data_read_voltage: errer, please read again!\n"));
			printf_syn("EM moment freeze data read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_MOMENT_FREEZE_INFO, &mfz_ptr, SINKINFO_EM_MOMENT_FREEZE_DATA_SIZE)){
 				printf_syn("EM moment freeze data read failed!\r\n");
			}
			else{
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
				rt_memcpy(pch, mfz_ptr.freeze_time, sizeof(mfz_ptr.freeze_time)-3);
				rt_memcpy(pch+5, (u8_t *)&freezedata[0], sizeof(mfz_ptr.act_elec_energy));
				rt_memcpy(pch+9, (u8_t *)&freezedata[1], sizeof(mfz_ptr.reverse_act_elec_energy));
				rt_memcpy(pch+13, (u8_t *)&freezedata[2], sizeof(mfz_ptr.apxT));
				rt_memcpy(pch+17, (u8_t *)&freezedata[3], sizeof(mfz_ptr.apxA));
				rt_memcpy(pch+21, (u8_t *)&freezedata[4], sizeof(mfz_ptr.apxB));
				rt_memcpy(pch+25, (u8_t *)&freezedata[5], sizeof(mfz_ptr.apxC));
				rt_memcpy(pch+29, (u8_t *)&freezedata[6], sizeof(mfz_ptr.rapxT));
				rt_memcpy(pch+33, (u8_t *)&freezedata[7], sizeof(mfz_ptr.rapxA));
				rt_memcpy(pch+37, (u8_t *)&freezedata[8], sizeof(mfz_ptr.rapxB));
				rt_memcpy(pch+41, (u8_t *)&freezedata[9], sizeof(mfz_ptr.rapxC));
				rt_memcpy(pch+45, (u8_t *)&freezedata[10], sizeof(mfz_ptr.act_max_demand));
				rt_memcpy(pch+49, mfz_ptr.act_max_demand_time, sizeof(mfz_ptr.act_max_demand_time)-3);
				rt_memcpy(pch+54, (u8_t *)&freezedata[11], sizeof(mfz_ptr.reverse_act_max_demand));
				rt_memcpy(pch+58, mfz_ptr.reverse_act_max_demand_time, sizeof(mfz_ptr.reverse_act_max_demand_time)-3);
			}
		}
		break;

	case LT300SYS_METER_MOMENT_FREEZE_DATA_READ_ALARM_ID: 
		*pch = data.mfreeze_alarm;
		break;

	case LT300SYS_METER_MOMENT_FREEZE_DATA_READ_ALARM_MASK_ID:
		*pch = data.mfreeze_alarm_mask;
		break;
		
	default:
		break;
	}

	return;
}

static u8_t yjlt300sys_meter_momentfreezedata_read_entry_set_test(struct obj_def *od, u16_t len, void *value)
{
	u8_t id, set_ok;

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	set_ok = 0;
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_METER_MOMENT_FREEZE_TIMES_ID:
	case LT300SYS_METER_MOMENT_FREEZE_TIME_ID:
	case LT300SYS_METER_MOMENT_FREEZE_ENABLE_ID:
	case LT300SYS_METER_MOMENT_FREEZE_DATA_READ_ALARM_MASK_ID:
#if 0
		if ((0==*sint_ptr) || (1==*sint_ptr))
			set_ok = 1;
#else
		set_ok = 1;
#endif
		break;

	default:
		break;
	}

	return set_ok;
}

static void yjlt300sys_meter_momentfreezedata_read_entry_set_value(struct obj_def *od, u16_t len, void *value)
{
	u32_t *uint_ptr = (u32_t *)value;
	u8_t *pch = (u8_t *)value;
	//struct collect_meter_data_info_st data;
	//struct celectric_meter_config_info_st amm_conf;
	struct ammeter_time time;
	u32_t freezedata[12]= {0};
	u8_t chardata[12]= {0};
	char snid[DEV_SN_MODE_LEN+1]= {'\0'};
	u8_t id,i;
    u32_t freezestate;
		
	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	/* @todo @fixme: which kind of pointer is 'value'? s32_t or u8_t??? */
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_METER_MOMENT_FREEZE_TIMES_ID:
		/*if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
		}	
		amm_conf.moment_freeze_times[od->id_inst_ptr[2]-1] = *pch;
		write_syscfgdata_tbl(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf);
		syscfgdata_syn_proc();*/
		set_em_info(od->id_inst_ptr[2]-1, 4, uint_ptr, pch);
		break;
	case LT300SYS_METER_MOMENT_FREEZE_TIME_ID:
		set_em_info(od->id_inst_ptr[2]-1, 6, uint_ptr, pch);
		break;
	case LT300SYS_METER_MOMENT_FREEZE_ENABLE_ID:
		if(*((u32_t *)value)== 1){
			get_em_info(od->id_inst_ptr[2]-1, 6, freezedata, chardata);
			time.month = (chardata[0]-0x30)*10 + (chardata[1]-0x30);
			time.day = (chardata[2]-0x30)*10 + (chardata[3]-0x30);
			time.hour = (chardata[4]-0x30)*10 + (chardata[5]-0x30);
			time.minite = (chardata[6]-0x30)*10 + (chardata[7]-0x30);
			//rt_memcpy(&time, (u32_t *)chardata, sizeof(struct ammeter_time));
			
			get_dev_sn_em_sn(SDT_ELECTRIC_METER, snid, DEV_SN_MODE_LEN, od->id_inst_ptr[2]-1);
			if(FRAME_E_OK == setting_ammeter_forzen_data_time((rt_uint8_t *)snid, CMD_FREEZING_NOW, &time, RS485_PORT_USED_BY_645)){
				freezestate = 1;
				set_em_info(od->id_inst_ptr[2]-1, 8, &freezestate, pch);
				for(i=0;i<3;i++){   /* try to trap tree times */
					trap_send(od->id_inst_ptr[2]-1,SNMP_GENTRAP_ENTERPRISESPC, E_ALR_EM_MOMENTARY_FREEZE_DATA, E_ALR_EM_MOMENTARY_FREEZE_DATA, SDT_ELECTRIC_METER);
					rt_thread_delay(get_ticks_of_ms(50));
				}
			}else{
				freezestate = 0;
				set_em_info(od->id_inst_ptr[2]-1, 8, &freezestate, pch);
				printf_syn("Set EM moment freeze data and time failed!\r\n");
			}
			
		}else {
			freezestate = 0;
			set_em_info(od->id_inst_ptr[2]-1, 8, &freezestate, pch);
		}	

		break;
	case LT300SYS_METER_MOMENT_FREEZE_DATA_READ_ALARM_MASK_ID:
		// data.mfreeze_alarm_mask = *pch;
		break;

	default:
		break;
	}


	return;
}

/*
 * meter timing freeze data collecting
 */
static void yjlt300sys_meter_timingfreezedata_read_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
{
	/* return to object name, adding index depth (1) */
	ident_len += 2;
	ident     -= 2;

	if (ident_len == 3) {
		u8_t id;

		od->id_inst_len = ident_len;
		od->id_inst_ptr = ident;

		LWIP_ASSERT("invalid id", (ident[0] >= 0) && (ident[0] <= 0xff));

		id = (u8_t)ident[0];
		od->instance = MIB_OBJECT_TAB;
		switch (id) {
		case LT300SYS_METER_TIMING_FREEZE_TIMES_ID:
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = sizeof(u32_t);
			break;
		case LT300SYS_METER_TIMING_FREEZE_MAX_TIMES_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = sizeof(u32_t);
			break;
		case LT300SYS_METER_TIMING_FREEZE_TIME_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 8;
			break;
		case LT300SYS_METER_TIMING_FREEZE_TYPE_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = sizeof(u32_t);
			break;
		case LT300SYS_METER_TIMING_FREEZE_ENABLE_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = sizeof(u32_t);
			break;
		case LT300SYS_METER_TIMING_FREEZE_DATA_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 63;
			break;
		case LT300SYS_METER_TIMING_FREEZE_DATA_READ_ALARM_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = 4;
			break;
		case LT300SYS_METER_TIMING_FREEZE_DATA_READ_ALARM_MASK_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = 4;
			break;

		default:
			LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no such object\n"));
			od->instance = MIB_OBJECT_NONE;
			break;
		}
	} else {
		LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no scalar\n"));
		od->instance = MIB_OBJECT_NONE;
	}

	return;
}

static void yjlt300sys_meter_timingfreezedata_read_entry_get_value(struct obj_def *od, u16_t len, void *value)
{

	u32_t *u32_ptr = (u32_t *)value;
	u8_t *pch = (u8_t *)value;
	struct collect_meter_data_info_st data;
	//struct celectric_meter_config_info_st amm_conf;
	struct sinkinfo_em_timing_freeze_st tfz_ptr;
	u32_t freezedata[32]= {0};
	u8_t chardata[12]= {0};
	u8_t id;
	//u8_t i;
	//char str[12]= {'\0'};

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_METER_TIMING_FREEZE_TIMES_ID:
		if(SUCC == get_em_info(od->id_inst_ptr[2]-1, 5, freezedata, chardata)){
			*u32_ptr = freezedata[0];
		}
		else{
			printf_syn("%s(), get em info tbl fail\n", __FUNCTION__);
		}
		break;		
	case LT300SYS_METER_TIMING_FREEZE_MAX_TIMES_ID:
		*u32_ptr = INVLIDE_DATAS;
		break;
	case LT300SYS_METER_TIMING_FREEZE_TIME_ID:			
		if(SUCC == get_em_info(od->id_inst_ptr[2]-1, 6, freezedata, pch)){
			/*for(i=0; i<6; i++)
				lwip_htonl(freezedata[i]);
			
			rt_memcpy(u32_ptr, &freezedata[1], sizeof(struct ammeter_time)-2);	*/
		}
		else{
			printf_syn("%s(), get em info tbl fail\n", __FUNCTION__);
		}
		break;
	case LT300SYS_METER_TIMING_FREEZE_TYPE_ID:	
		if(SUCC == get_em_info(od->id_inst_ptr[2]-1, 7, freezedata, chardata)){
			*u32_ptr = freezedata[0];
		}
		else{
			printf_syn("%s(), get em info tbl fail\n", __FUNCTION__);
		}
		break;

	case LT300SYS_METER_TIMING_FREEZE_ENABLE_ID:
		if(SUCC == get_em_info(od->id_inst_ptr[2]-1, 9, freezedata, chardata)){
			*u32_ptr = freezedata[0];
		}
		else{
			printf_syn("%s(), get em info tbl fail\n", __FUNCTION__);
		}
		break;		
	case LT300SYS_METER_TIMING_FREEZE_DATA_ID:
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_TIMING_FREEZE_INFO, &tfz_ptr, SINKINFO_EM_TIMING_FREEZE_DATA_SIZE)){
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
			rt_memcpy(pch, tfz_ptr.freeze_time, sizeof(tfz_ptr.freeze_time)-3);
			rt_memcpy(pch+5, (u8_t *)&freezedata[0], sizeof(tfz_ptr.act_elec_energy));
			rt_memcpy(pch+9, (u8_t *)&freezedata[1], sizeof(tfz_ptr.reverse_act_elec_energy));
			rt_memcpy(pch+13, (u8_t *)&freezedata[2], sizeof(tfz_ptr.apxT));
			rt_memcpy(pch+17, (u8_t *)&freezedata[3], sizeof(tfz_ptr.apxA));
			rt_memcpy(pch+21, (u8_t *)&freezedata[4], sizeof(tfz_ptr.apxB));
			rt_memcpy(pch+25, (u8_t *)&freezedata[5], sizeof(tfz_ptr.apxC));
			rt_memcpy(pch+29, (u8_t *)&freezedata[6], sizeof(tfz_ptr.rapxT));
			rt_memcpy(pch+33, (u8_t *)&freezedata[7], sizeof(tfz_ptr.rapxA));
			rt_memcpy(pch+37, (u8_t *)&freezedata[8], sizeof(tfz_ptr.rapxB));
			rt_memcpy(pch+41, (u8_t *)&freezedata[9], sizeof(tfz_ptr.rapxC));
			rt_memcpy(pch+45, (u8_t *)&freezedata[10], sizeof(tfz_ptr.act_max_demand));
			rt_memcpy(pch+49, tfz_ptr.act_max_demand_time, sizeof(tfz_ptr.act_max_demand_time)-3);
			rt_memcpy(pch+54, (u8_t *)&freezedata[11], sizeof(tfz_ptr.reverse_act_max_demand));
			rt_memcpy(pch+58, tfz_ptr.reverse_act_max_demand_time, sizeof(tfz_ptr.reverse_act_max_demand_time)-3);

		}
		else{
			printf_syn("EM timing freeze data read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_TIMING_FREEZE_INFO, &tfz_ptr, SINKINFO_EM_TIMING_FREEZE_DATA_SIZE)){
 				printf_syn("EM timing freeze data read failed!\r\n");
			}
			else{
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
				rt_memcpy(pch, tfz_ptr.freeze_time, sizeof(tfz_ptr.freeze_time)-3);
				rt_memcpy(pch+5, (u8_t *)&freezedata[0], sizeof(tfz_ptr.act_elec_energy));
				rt_memcpy(pch+9, (u8_t *)&freezedata[1], sizeof(tfz_ptr.reverse_act_elec_energy));
				rt_memcpy(pch+13, (u8_t *)&freezedata[2], sizeof(tfz_ptr.apxT));
				rt_memcpy(pch+17, (u8_t *)&freezedata[3], sizeof(tfz_ptr.apxA));
				rt_memcpy(pch+21, (u8_t *)&freezedata[4], sizeof(tfz_ptr.apxB));
				rt_memcpy(pch+25, (u8_t *)&freezedata[5], sizeof(tfz_ptr.apxC));
				rt_memcpy(pch+29, (u8_t *)&freezedata[6], sizeof(tfz_ptr.rapxT));
				rt_memcpy(pch+33, (u8_t *)&freezedata[7], sizeof(tfz_ptr.rapxA));
				rt_memcpy(pch+37, (u8_t *)&freezedata[8], sizeof(tfz_ptr.rapxB));
				rt_memcpy(pch+41, (u8_t *)&freezedata[9], sizeof(tfz_ptr.rapxC));
				rt_memcpy(pch+45, (u8_t *)&freezedata[10], sizeof(tfz_ptr.act_max_demand));
				rt_memcpy(pch+49, tfz_ptr.act_max_demand_time, sizeof(tfz_ptr.act_max_demand_time)-3);
				rt_memcpy(pch+54, (u8_t *)&freezedata[11], sizeof(tfz_ptr.reverse_act_max_demand));
				rt_memcpy(pch+58, tfz_ptr.reverse_act_max_demand_time, sizeof(tfz_ptr.reverse_act_max_demand_time)-3);
			}
		}			
		break;

	case LT300SYS_METER_TIMING_FREEZE_DATA_READ_ALARM_ID: 
		*pch = data.tfreeze_alarm;
		break;

	case LT300SYS_METER_TIMING_FREEZE_DATA_READ_ALARM_MASK_ID:
		*pch = data.tfreeze_alarm_mask;
		break;
		
	default:
		break;
	}

	return;
}

static u8_t yjlt300sys_meter_timingfreezedata_read_entry_set_test(struct obj_def *od, u16_t len, void *value)
{
	u8_t id, set_ok;

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	set_ok = 0;
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_METER_TIMING_FREEZE_TIMES_ID:
	case LT300SYS_METER_TIMING_FREEZE_TIME_ID:
	case LT300SYS_METER_TIMING_FREEZE_TYPE_ID:
	case LT300SYS_METER_TIMING_FREEZE_ENABLE_ID:
	case LT300SYS_METER_TIMING_FREEZE_DATA_READ_ALARM_MASK_ID:
#if 0
		if ((0==*sint_ptr) || (1==*sint_ptr))
			set_ok = 1;
#else
		set_ok = 1;
#endif
		break;

	default:
		break;
	}

	return set_ok;
}

static void yjlt300sys_meter_timingfreezedata_read_entry_set_value(struct obj_def *od, u16_t len, void *value)
{
	u32_t *uint_ptr = (u32_t *)value;
	u8_t *pch = (u8_t *)value;
	//struct collect_meter_data_info_st data;
	//struct celectric_meter_config_info_st amm_conf;
	struct ammeter_time time;
	u32_t freezedata[12]= {0};
	u8_t chardata[12]= {0};
	char snid[DEV_SN_MODE_LEN+1]= {'\0'};
	u8_t id;
    u32_t freezestate;

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	/* @todo @fixme: which kind of pointer is 'value'? s32_t or u8_t??? */
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_METER_TIMING_FREEZE_TIMES_ID:
		set_em_info(od->id_inst_ptr[2]-1, 5, uint_ptr, pch);
		break;
	case LT300SYS_METER_TIMING_FREEZE_TIME_ID:
		set_em_info(od->id_inst_ptr[2]-1, 6, uint_ptr, pch);
		break;
	case LT300SYS_METER_TIMING_FREEZE_TYPE_ID:
		set_em_info(od->id_inst_ptr[2]-1, 7, uint_ptr, pch);
		break;

	case LT300SYS_METER_TIMING_FREEZE_ENABLE_ID:
		if(*((u32_t *)value)== 1){
			get_em_info(od->id_inst_ptr[2]-1, 6, freezedata, chardata);
			time.month = (chardata[0]-0x30)*10 + (chardata[1]-0x30);
			time.day = (chardata[2]-0x30)*10 + (chardata[3]-0x30);
			time.hour = (chardata[4]-0x30)*10 + (chardata[5]-0x30);
			time.minite = (chardata[6]-0x30)*10 + (chardata[7]-0x30);
			//rt_memcpy(&time, (u32_t *)chardata, sizeof(struct ammeter_time));
			get_em_info(od->id_inst_ptr[2]-1, 7, freezedata, chardata);
			
			get_dev_sn_em_sn(SDT_ELECTRIC_METER, snid, DEV_SN_MODE_LEN, od->id_inst_ptr[2]-1);
			if(FRAME_E_OK == setting_ammeter_forzen_data_time((rt_uint8_t *)snid, freezedata[0], &time, RS485_PORT_USED_BY_645)){
				freezestate = 1;
				set_em_info(od->id_inst_ptr[2]-1, 9, &freezestate, pch);
			}else{
				freezestate = 0;
				set_em_info(od->id_inst_ptr[2]-1, 9, &freezestate, pch);
				printf_syn("Set EM timing freeze data and time failed!\r\n");
			}
		}else {
			freezestate = 0;
			set_em_info(od->id_inst_ptr[2]-1, 9, &freezestate, pch);
		}	
		break;

	case LT300SYS_METER_TIMING_FREEZE_DATA_READ_ALARM_MASK_ID:
		//data.tfreeze_alarm_mask = *pch;
		break;

	default:
		break;
	}


	return;
}

#if EM_EVENT_SUPPERT
/*
 * meter event data collecting
 */
static void yjlt300sys_meterevent_read_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
{
	/* return to object name, adding index depth (1) */
	ident_len += 2;
	ident     -= 2;

	if (ident_len == 3) {
		u8_t id;

		od->id_inst_len = ident_len;
		od->id_inst_ptr = ident;

		LWIP_ASSERT("invalid id", (ident[0] >= 0) && (ident[0] <= 0xff));

		id = (u8_t)ident[0];
		od->instance = MIB_OBJECT_TAB;
		switch (id) {
		case LT300SYS_METER_VOL_LOSS_TOTAL_TIMES_ID:
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 24;
			break;
		case LT300SYS_METER_VOL_LOSS_TIMES_SET_ID:
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = sizeof(u32_t);
			break;
		case LT300SYS_METER_VOL_LOSS_EVENT_SOURCE_ID:
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = sizeof(u32_t);
			break;
		case LT300SYS_METER_VOL_LOSS_EVENT_ID:
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 120;
			break;
		case LT300SYS_METER_VOL_OVER_TOTAL_TIMES_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 24;
			break;
		case LT300SYS_METER_VOL_OVER_TIMES_SET_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = sizeof(u32_t);
			break;
		case LT300SYS_METER_VOL_OVER_EVENT_SOURCE_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access	 = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len	 = sizeof(u32_t);
			break;
		case LT300SYS_METER_VOL_OVER_EVENT_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 120;
			break;
		case LT300SYS_METER_VOL_UNDER_TOTAL_TIMES:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 24;
			break;
		case LT300SYS_METER_VOL_UNDER_TIMES_SET_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = sizeof(u32_t);
			break;
		case LT300SYS_METER_VOL_UNDER_EVENT_SOURCE_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = sizeof(u32_t);
			break;
		case LT300SYS_METER_VOL_UNDER_EVENT_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 120;
			break;
		case LT300SYS_METER_PHASE_BREAK_TOTAL_TIMES_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access	 = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len	 = 24;
			break;
		case LT300SYS_METER_PHASE_BREAK_TIMES_SET_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = sizeof(u32_t);
			break;
		case LT300SYS_METER_PHASE_BREAK_EVENT_SOURCE_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = sizeof(u32_t);
			break;			
		case LT300SYS_METER_PHASE_BREAK_EVENT_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 120;
			break;
		case LT300SYS_METER_CUR_LOSS_TOTAL_TIMES_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV| SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 24;
			break;
		case LT300SYS_METER_CUR_LOSS_TIMES_SET_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV| SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = sizeof(u32_t);
			break;
		case LT300SYS_METER_CUR_LOSS_EVENT_SOURCE_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV| SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = sizeof(u32_t);
			break;
		case LT300SYS_METER_CUR_LOSS_EVENT_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV| SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 108;
			break;	
		case LT300SYS_METER_CUR_OVER_TOTAL_TIMES_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 24;
			break;
		case LT300SYS_METER_CUR_OVER_TIMES_SET_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = sizeof(u32_t);
			break;
		case LT300SYS_METER_CUR_OVER_EVENT_SOURCE_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = sizeof(u32_t);
			break;
		case LT300SYS_METER_CUR_OVER_EVENT_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 108;
			break;
		case LT300SYS_METER_CUR_BREAK_TOTAL_TIMES_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access	 = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len	 = 24;
			break;
		case LT300SYS_METER_CUR_BREAK_TIMES_SET_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access	 = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len	 = sizeof(u32_t);
			break;
		case LT300SYS_METER_CUR_BREAK_EVENT_SOURCE_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access	 = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len	 = sizeof(u32_t);
			break;
		case LT300SYS_METER_CUR_BREAK_EVENT_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 108;
			break;
		case LT300SYS_METER_CLEAR_EVENT_TOTAL_TIMES_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = sizeof(u32_t);
			break;
		case LT300SYS_METER_CLEAR_TIMES_SET_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = sizeof(u32_t);
			break;
		case LT300SYS_METER_CLEAR_EVENT_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 124;
			break;
		case LT300SYS_METER_DEMAND_CLEAR_TOTAL_TIMES_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = sizeof(u32_t);
			break;
		case LT300SYS_METER_DEMAND_CLEAR_TIMES_SET_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = sizeof(u32_t);
			break;
		case LT300SYS_METER_DEMAND_CLEAR_EVENT_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 28;
			break;
		case LT300SYS_METER_PROGRAMMING_TOTAL_TIMES_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = sizeof(u32_t);
			break;
		case LT300SYS_METER_PROGRAMMING_TIMES_SET_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = sizeof(u32_t);
			break;
		case LT300SYS_METER_PROGRAMMING_EVENT_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 68;
			break;
		case LT300SYS_METER_CALIBRATE_TIME_TOTAL_TIMES_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = sizeof(u32_t);
			break;
		case LT300SYS_METER_CALIBRATE_TIME_TIMES_SET_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = sizeof(u32_t);
			break;
		case LT300SYS_METER_CALIBRATE_TIME_EVENT_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 52;
			break;
		case LT300SYS_METER_VOL_REV_PHASE_TOTAL_TIMES_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = sizeof(u32_t);
			break;
		case LT300SYS_METER_VOL_REV_PHASE_TIMES_SET_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = sizeof(u32_t);
			break;
		case LT300SYS_METER_VOL_REV_PHASE_EVENT_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 48;
			break;
		case LT300SYS_METER_CUL_REV_PHASE_TOTAL_TIMES_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = sizeof(u32_t);
			break;
		case LT300SYS_METER_CUL_REV_PHASE_TIMES_SET_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len    = sizeof(u32_t);
			break;			
		case LT300SYS_METER_CUL_REV_PHASE_EVENT_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 48;
			break;

		default:
			LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no such object\n"));
			od->instance = MIB_OBJECT_NONE;
			break;
		}
	} else {
		LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no scalar\n"));
		od->instance = MIB_OBJECT_NONE;
	}

	return;
}

static void yjlt300sys_meterevent_read_entry_get_value(struct obj_def *od, u16_t len, void *value)
{
	u32_t *uint_ptr =(u32_t *) value;
	//u8_t *pch = (u8_t *)value;
	
	//struct collect_meter_data_info_st data;
	struct electric_meter_reg_info_st amm_sn;
	struct celectric_meter_config_info_st amm_conf;
	u8_t id;
	rt_uint32_t data_len = 0;
	u8_t eventdata[32];
	rt_uint8_t meteraddr[12];

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_METER_VOL_LOSS_TOTAL_TIMES_ID:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		if (SUCC == get_em_reg_info(&amm_sn)) {
			rt_memcpy(meteraddr, amm_sn.em_sn[od->id_inst_ptr[2]-1], DEV_SN_MODE_LEN);
		} else {
			printf_syn("%s(), read meter addr data tbl fail\n", __FUNCTION__);
		}
		if (FRAME_E_OK == get_event_data_from_ammeter(meteraddr, AMM_EVENT_LOSE_VOLTAGE, amm_conf.volloss_event_times[od->id_inst_ptr[2]-1], eventdata,  &data_len, RS485_PORT_USED_BY_645)){
			amm_conf.volloss_event_total_times[od->id_inst_ptr[2]-1][0] = eventdata[2]<<16 | eventdata[1]<<8 | eventdata[0];
			amm_conf.volloss_event_total_times[od->id_inst_ptr[2]-1][1] = eventdata[5]<<16 | eventdata[4]<<8 | eventdata[3];
			amm_conf.volloss_event_total_times[od->id_inst_ptr[2]-1][2] = eventdata[8]<<16 | eventdata[7]<<8 | eventdata[6];
			amm_conf.volloss_event_total_times[od->id_inst_ptr[2]-1][3] = eventdata[11]<<16 | eventdata[10]<<8 | eventdata[9];
			amm_conf.volloss_event_total_times[od->id_inst_ptr[2]-1][4] = eventdata[14]<<16 | eventdata[13]<<8 | eventdata[12];
			amm_conf.volloss_event_total_times[od->id_inst_ptr[2]-1][5] = eventdata[17]<<16 | eventdata[16]<<8 | eventdata[15];
		}
		else{
			printf_syn("%s(), read total volloss times fail\n", __FUNCTION__);
		}
//		rt_memcpy(uint_ptr, amm_conf.volloss_event_total_times[od->id_inst_ptr[2]-1], sizeof(amm_conf.volloss_event_total_times[od->id_inst_ptr[2]-1])); David
		break;	
	case LT300SYS_METER_VOL_LOSS_TIMES_SET_ID:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		*uint_ptr = (rt_uint32_t)amm_conf.volloss_event_times[od->id_inst_ptr[2]-1];	
		break;	
	case LT300SYS_METER_VOL_LOSS_EVENT_SOURCE_ID:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		*uint_ptr = (rt_uint32_t)amm_conf.volloss_event_source[od->id_inst_ptr[2]-1];			
		break;	
	case LT300SYS_METER_VOL_LOSS_EVENT_ID:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}
		if (SUCC == get_em_reg_info(&amm_sn)) {
			printf_syn("%s(), read meter SN data tbl fail\n", __FUNCTION__);
		} 
		if(amm_conf.volloss_event_source[od->id_inst_ptr[2]-1] == PHASEA){
			//get_event_data_from_ammeter(amm_sn.em_sn[od->id_inst_ptr[2]-1], AMM_EVENT_A_LOSE_VOLTAGE, amm_conf.volloss_event_times[od->id_inst_ptr[2]-1], pch, &data_len, RS485_PORT_USED_BY_645);
			if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PA_VOLLOSS_EVENT_INFO, (struct sinkinfo_em_volloss_event_st *)value, SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE)){
				PRIVMIB_INFO(("EM voltage loss event data read SUCCESS!\r\n"));
			}
			else{
				printf_syn("EM voltage loss event data read failed!try read again!\r\n");
				if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PA_VOLLOSS_EVENT_INFO, (struct sinkinfo_em_volloss_event_st *)value, SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE)){
 					printf_syn("EM voltage loss event data read failed!\r\n");
				}
				else{
					PRIVMIB_INFO(("EM voltage loss event data read SUCCESS!\r\n"));
				}
			}
		}else if(amm_conf.volloss_event_source[od->id_inst_ptr[2]-1] == PHASEB){
			//get_event_data_from_ammeter(amm_sn.em_sn[od->id_inst_ptr[2]-1], AMM_EVENT_B_LOSE_VOLTAGE, amm_conf.volloss_event_times[od->id_inst_ptr[2]-1], pch, &data_len, RS485_PORT_USED_BY_645);
			if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PB_VOLLOSS_EVENT_INFO, (struct sinkinfo_em_volloss_event_st *)value, SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE)){
				PRIVMIB_INFO(("EM voltage loss event data read SUCCESS!\r\n"));
			}
			else{
				printf_syn("EM voltage loss event data read failed!try read again!\r\n");
				if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PB_VOLLOSS_EVENT_INFO, (struct sinkinfo_em_volloss_event_st *)value, SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE)){
					printf_syn("EM voltage loss event data read failed!\r\n");
				}
				else{
					PRIVMIB_INFO(("EM voltage loss event data read SUCCESS!\r\n"));
				}
			}
		}else if(amm_conf.volloss_event_source[od->id_inst_ptr[2]-1] == PHASEC){
			//get_event_data_from_ammeter(amm_sn.em_sn[od->id_inst_ptr[2]-1], AMM_EVENT_C_LOSE_VOLTAGE, amm_conf.volloss_event_times[od->id_inst_ptr[2]-1], pch, &data_len, RS485_PORT_USED_BY_645);
			if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PC_VOLLOSS_EVENT_INFO, (struct sinkinfo_em_volloss_event_st *)value, SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE)){
				PRIVMIB_INFO(("EM voltage loss event data read SUCCESS!\r\n"));
			}
			else{
				printf_syn("EM voltage loss event data read failed!try read again!\r\n");
				if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PC_VOLLOSS_EVENT_INFO, (struct sinkinfo_em_volloss_event_st *)value, SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE)){
					printf_syn("EM voltage loss event data read failed!\r\n");
				}
				else{
					PRIVMIB_INFO(("EM voltage loss event data read SUCCESS!\r\n"));
				}
			}

		}else{
			printf_syn("%s(), cann't read meter event data\n", __FUNCTION__);
		}
		break;		

	case LT300SYS_METER_VOL_OVER_TOTAL_TIMES_ID:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}
		if (SUCC == get_em_reg_info(&amm_sn)) {
			rt_memcpy(meteraddr, amm_sn.em_sn[od->id_inst_ptr[2]-1], DEV_SN_MODE_LEN);
		} else {
			printf_syn("%s(), read meter addr data tbl fail\n", __FUNCTION__);
		}
		if (FRAME_E_OK == get_event_data_from_ammeter(meteraddr, AMM_EVENT_OVER_VOLTAGE, amm_conf.volover_event_times[od->id_inst_ptr[2]-1], eventdata,  &data_len, RS485_PORT_USED_BY_645)){
			amm_conf.volover_event_total_times[od->id_inst_ptr[2]-1][0] = eventdata[2]<<16 | eventdata[1]<<8 | eventdata[0];
			amm_conf.volover_event_total_times[od->id_inst_ptr[2]-1][1] = eventdata[5]<<16 | eventdata[4]<<8 | eventdata[3];
			amm_conf.volover_event_total_times[od->id_inst_ptr[2]-1][2] = eventdata[8]<<16 | eventdata[7]<<8 | eventdata[6];
			amm_conf.volover_event_total_times[od->id_inst_ptr[2]-1][3] = eventdata[11]<<16 | eventdata[10]<<8 | eventdata[9];
			amm_conf.volover_event_total_times[od->id_inst_ptr[2]-1][4] = eventdata[14]<<16 | eventdata[13]<<8 | eventdata[12];
			amm_conf.volover_event_total_times[od->id_inst_ptr[2]-1][5] = eventdata[17]<<16 | eventdata[16]<<8 | eventdata[15];
		}
		else{
			printf_syn("%s(), read total volloss times fail\n", __FUNCTION__);
		}
//		rt_memcpy(uint_ptr, amm_conf.volover_event_total_times[od->id_inst_ptr[2]-1], sizeof(amm_conf.volover_event_total_times[od->id_inst_ptr[2]-1])); David
		break;
	case LT300SYS_METER_VOL_OVER_TIMES_SET_ID:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		*uint_ptr = (rt_uint32_t)amm_conf.volover_event_times[od->id_inst_ptr[2]-1];	
		break;
	case LT300SYS_METER_VOL_OVER_EVENT_SOURCE_ID:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		*uint_ptr = (rt_uint32_t)amm_conf.volover_event_source[od->id_inst_ptr[2]-1];		
		break;
	case LT300SYS_METER_VOL_OVER_EVENT_ID:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}
		if (SUCC == get_em_reg_info(&amm_sn)) {
			printf_syn("%s(), read meter SN data tbl fail\n", __FUNCTION__);
		} 
		if(amm_conf.volloss_event_source[od->id_inst_ptr[2]-1] == PHASEA){
			//get_event_data_from_ammeter(amm_sn.em_sn[od->id_inst_ptr[2]-1], AMM_EVENT_A_OVER_VOLTAGE, amm_conf.volover_event_times[od->id_inst_ptr[2]-1], pch, &data_len, RS485_PORT_USED_BY_645);
			if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PA_VOLOVER_EVENT_INFO, (struct sinkinfo_em_volloss_event_st *)value, SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE)){
				PRIVMIB_INFO(("EM voltage over event data read SUCCESS!\r\n"));
			}
			else{
				printf_syn("EM voltage over event data read failed!try read again!\r\n");
				if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PA_VOLOVER_EVENT_INFO, (struct sinkinfo_em_volloss_event_st *)value, SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE)){
					printf_syn("EM voltage over event data read failed!\r\n");
				}
				else{
					PRIVMIB_INFO(("EM voltage over event data read SUCCESS!\r\n"));
				}
			}
		}else if(amm_conf.volloss_event_source[od->id_inst_ptr[2]-1] == PHASEB){
			//get_event_data_from_ammeter(amm_sn.em_sn[od->id_inst_ptr[2]-1], AMM_EVENT_B_OVER_VOLTAGE, amm_conf.volover_event_times[od->id_inst_ptr[2]-1], pch, &data_len, RS485_PORT_USED_BY_645);
			if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PB_VOLOVER_EVENT_INFO, (struct sinkinfo_em_volloss_event_st *)value, SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE)){
				PRIVMIB_INFO(("EM voltage over event data read SUCCESS!\r\n"));
			}
			else{
				printf_syn("EM voltage over event data read failed!try read again!\r\n");
				if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PB_VOLOVER_EVENT_INFO, (struct sinkinfo_em_volloss_event_st *)value, SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE)){
					printf_syn("EM voltage over event data read failed!\r\n");
				}
				else{
					PRIVMIB_INFO(("EM voltage over event data read SUCCESS!\r\n"));
				}
			}

		}else if(amm_conf.volloss_event_source[od->id_inst_ptr[2]-1] == PHASEC){
			//get_event_data_from_ammeter(amm_sn.em_sn[od->id_inst_ptr[2]-1], AMM_EVENT_C_OVER_VOLTAGE, amm_conf.volover_event_times[od->id_inst_ptr[2]-1], pch, &data_len, RS485_PORT_USED_BY_645);
			if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PC_VOLOVER_EVENT_INFO, (struct sinkinfo_em_volloss_event_st *)value, SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE)){
				PRIVMIB_INFO(("EM voltage over event data read SUCCESS!\r\n"));
			}
			else{
				printf_syn("EM voltage over event data read failed!try read again!\r\n");
				if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PC_VOLOVER_EVENT_INFO, (struct sinkinfo_em_volloss_event_st *)value, SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE)){
					printf_syn("EM voltage over event data read failed!\r\n");
				}
				else{
					PRIVMIB_INFO(("EM voltage over event data read SUCCESS!\r\n"));
				}
			}

		}else{
			printf_syn("%s(), cann't read meter event data\n", __FUNCTION__);
		}
		break;

	case LT300SYS_METER_VOL_UNDER_TOTAL_TIMES:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}
		if (SUCC == get_em_reg_info(&amm_sn)) {
			rt_memcpy(meteraddr, amm_sn.em_sn[od->id_inst_ptr[2]-1], DEV_SN_MODE_LEN);
		} else {
			printf_syn("%s(), read meter addr data tbl fail\n", __FUNCTION__);
		}
		if (FRAME_E_OK == get_event_data_from_ammeter(meteraddr, AMM_EVENT_OWE_VOLTAGE, amm_conf.volunder_event_times[od->id_inst_ptr[2]-1], eventdata,  &data_len, RS485_PORT_USED_BY_645)){
			amm_conf.volunder_event_total_times[od->id_inst_ptr[2]-1][0] = eventdata[2]<<16 | eventdata[1]<<8 | eventdata[0];
			amm_conf.volunder_event_total_times[od->id_inst_ptr[2]-1][1] = eventdata[5]<<16 | eventdata[4]<<8 | eventdata[3];
			amm_conf.volunder_event_total_times[od->id_inst_ptr[2]-1][2] = eventdata[8]<<16 | eventdata[7]<<8 | eventdata[6];
			amm_conf.volunder_event_total_times[od->id_inst_ptr[2]-1][3] = eventdata[11]<<16 | eventdata[10]<<8 | eventdata[9];
			amm_conf.volunder_event_total_times[od->id_inst_ptr[2]-1][4] = eventdata[14]<<16 | eventdata[13]<<8 | eventdata[12];
			amm_conf.volunder_event_total_times[od->id_inst_ptr[2]-1][5] = eventdata[17]<<16 | eventdata[16]<<8 | eventdata[15];
		}
		else{
			printf_syn("%s(), read total volloss times fail\n", __FUNCTION__);
		}
//		rt_memcpy((rt_uint32_t *)value, amm_conf.volunder_event_total_times[od->id_inst_ptr[2]-1], sizeof(amm_conf.volunder_event_total_times[od->id_inst_ptr[2]-1])); David
		break;

	case LT300SYS_METER_VOL_UNDER_TIMES_SET_ID:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		*uint_ptr = (rt_uint32_t)amm_conf.volunder_event_times[od->id_inst_ptr[2]-1];		
		break;

	case LT300SYS_METER_VOL_UNDER_EVENT_SOURCE_ID:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		*uint_ptr = (rt_uint32_t)amm_conf.volunder_event_source[od->id_inst_ptr[2]-1];		
		break;

	case LT300SYS_METER_VOL_UNDER_EVENT_ID:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}
		if (SUCC == get_em_reg_info(&amm_sn)) {
			printf_syn("%s(), read meter SN data tbl fail\n", __FUNCTION__);
		} 
		if(amm_conf.volloss_event_source[od->id_inst_ptr[2]-1] == PHASEA){
			//get_event_data_from_ammeter(amm_sn.em_sn[od->id_inst_ptr[2]-1], AMM_EVENT_A_OWE_VOLTAGE, amm_conf.volunder_event_times[od->id_inst_ptr[2]-1], pch, &data_len, RS485_PORT_USED_BY_645);
			if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PA_VOLUNDER_EVENT_INFO, (struct sinkinfo_em_volloss_event_st *)value, SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE)){
				PRIVMIB_INFO(("EM voltage under event data read SUCCESS!\r\n"));
			}
			else{
				printf_syn("EM voltage under event data read failed!try read again!\r\n");
				if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PA_VOLUNDER_EVENT_INFO, (struct sinkinfo_em_volloss_event_st *)value, SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE)){
					printf_syn("EM voltage under event data read failed!\r\n");
				}
				else{
					PRIVMIB_INFO(("EM voltage under event data read SUCCESS!\r\n"));
				}
			}
		}else if(amm_conf.volloss_event_source[od->id_inst_ptr[2]-1] == PHASEB){
			//get_event_data_from_ammeter(amm_sn.em_sn[od->id_inst_ptr[2]-1], AMM_EVENT_B_OWE_VOLTAGE, amm_conf.volunder_event_times[od->id_inst_ptr[2]-1], pch, &data_len, RS485_PORT_USED_BY_645);
			if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PB_VOLUNDER_EVENT_INFO, (struct sinkinfo_em_volloss_event_st *)value, SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE)){
				PRIVMIB_INFO(("EM voltage under event data read SUCCESS!\r\n"));
			}
			else{
				printf_syn("EM voltage under event data read failed!try read again!\r\n");
				if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PB_VOLUNDER_EVENT_INFO, (struct sinkinfo_em_volloss_event_st *)value, SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE)){
					printf_syn("EM voltage under event data read failed!\r\n");
				}
				else{
					PRIVMIB_INFO(("EM voltage under event data read SUCCESS!\r\n"));
				}
			}
		}else if(amm_conf.volloss_event_source[od->id_inst_ptr[2]-1] == PHASEC){
			//get_event_data_from_ammeter(amm_sn.em_sn[od->id_inst_ptr[2]-1], AMM_EVENT_C_OWE_VOLTAGE, amm_conf.volunder_event_times[od->id_inst_ptr[2]-1], pch, &data_len, RS485_PORT_USED_BY_645);
			if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PC_VOLUNDER_EVENT_INFO, (struct sinkinfo_em_volloss_event_st *)value, SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE)){
				PRIVMIB_INFO(("EM voltage under event data read SUCCESS!\r\n"));
			}
			else{
				printf_syn("EM voltage under event data read failed!try read again!\r\n");
				if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PC_VOLUNDER_EVENT_INFO, (struct sinkinfo_em_volloss_event_st *)value, SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE)){
					printf_syn("EM voltage under event data read failed!\r\n");
				}
				else{
					PRIVMIB_INFO(("EM voltage under event data read SUCCESS!\r\n"));
				}
			}
		}else{
			printf_syn("%s(), cann't read meter event data\n", __FUNCTION__);
		}	
		break;
	case LT300SYS_METER_PHASE_BREAK_TOTAL_TIMES_ID:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}
		if (SUCC == get_em_reg_info(&amm_sn)) {
			rt_memcpy(meteraddr, amm_sn.em_sn[od->id_inst_ptr[2]-1], DEV_SN_MODE_LEN);
		} else {
			printf_syn("%s(), read meter addr data tbl fail\n", __FUNCTION__);
		}
		if (FRAME_E_OK == get_event_data_from_ammeter(meteraddr, AMM_EVENT_BROKEN_PHASE, amm_conf.phasebreak_event_times[od->id_inst_ptr[2]-1], eventdata,  &data_len, RS485_PORT_USED_BY_645)){
			amm_conf.phasebreak_event_total_times[od->id_inst_ptr[2]-1][0] = eventdata[2]<<16 | eventdata[1]<<8 | eventdata[0];
			amm_conf.phasebreak_event_total_times[od->id_inst_ptr[2]-1][1] = eventdata[5]<<16 | eventdata[4]<<8 | eventdata[3];
			amm_conf.phasebreak_event_total_times[od->id_inst_ptr[2]-1][2] = eventdata[8]<<16 | eventdata[7]<<8 | eventdata[6];
			amm_conf.phasebreak_event_total_times[od->id_inst_ptr[2]-1][3] = eventdata[11]<<16 | eventdata[10]<<8 | eventdata[9];
			amm_conf.phasebreak_event_total_times[od->id_inst_ptr[2]-1][4] = eventdata[14]<<16 | eventdata[13]<<8 | eventdata[12];
			amm_conf.phasebreak_event_total_times[od->id_inst_ptr[2]-1][5] = eventdata[17]<<16 | eventdata[16]<<8 | eventdata[15];
		}
		else{
			printf_syn("%s(), read total volloss times fail\n", __FUNCTION__);
		}
//		rt_memcpy((rt_uint32_t *)value, amm_conf.phasebreak_event_total_times[od->id_inst_ptr[2]-1], sizeof(amm_conf.phasebreak_event_total_times[od->id_inst_ptr[2]-1])); David
		break;
	case LT300SYS_METER_PHASE_BREAK_TIMES_SET_ID:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		*uint_ptr = (rt_uint32_t)amm_conf.phasebreak_event_times[od->id_inst_ptr[2]-1];			
		
		break;
	case LT300SYS_METER_PHASE_BREAK_EVENT_SOURCE_ID:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		*uint_ptr = (rt_uint32_t)amm_conf.phasebreak_event_source[od->id_inst_ptr[2]-1];			
		break;
	case LT300SYS_METER_PHASE_BREAK_EVENT_ID:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}
		if (SUCC == get_em_reg_info(&amm_sn)) {
			printf_syn("%s(), read meter SN data tbl fail\n", __FUNCTION__);
		} 
		if(amm_conf.volloss_event_source[od->id_inst_ptr[2]-1] == PHASEA){
			//get_event_data_from_ammeter(amm_sn.em_sn[od->id_inst_ptr[2]-1], AMM_EVENT_A_BROKEN_PHASE, amm_conf.phasebreak_event_times[od->id_inst_ptr[2]-1], pch, &data_len, RS485_PORT_USED_BY_645);
			if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PA_PHASEBREAK_EVENT_INFO, (struct sinkinfo_em_volloss_event_st *)value, SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE)){
				PRIVMIB_INFO(("EM phase break event data read SUCCESS!\r\n"));
			}
			else{
				printf_syn("EM phase break event data read failed!try read again!\r\n");
				if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PA_PHASEBREAK_EVENT_INFO, (struct sinkinfo_em_volloss_event_st *)value, SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE)){
					printf_syn("EM phase break event data read failed!\r\n");
				}
				else{
					PRIVMIB_INFO(("EM phase break event data read SUCCESS!\r\n"));
				}
			}
		}else if(amm_conf.volloss_event_source[od->id_inst_ptr[2]-1] == PHASEB){
			//get_event_data_from_ammeter(amm_sn.em_sn[od->id_inst_ptr[2]-1], AMM_EVENT_B_BROKEN_PHASE, amm_conf.phasebreak_event_times[od->id_inst_ptr[2]-1], pch, &data_len, RS485_PORT_USED_BY_645);
			if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PB_PHASEBREAK_EVENT_INFO, (struct sinkinfo_em_volloss_event_st *)value, SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE)){
				PRIVMIB_INFO(("EM phase break event data read SUCCESS!\r\n"));
			}
			else{
				printf_syn("EM phase break event data read failed!try read again!\r\n");
				if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PB_PHASEBREAK_EVENT_INFO, (struct sinkinfo_em_volloss_event_st *)value, SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE)){
					printf_syn("EM phase break event data read failed!\r\n");
				}
				else{
					PRIVMIB_INFO(("EM phase break event data read SUCCESS!\r\n"));
				}
			}
		}else if(amm_conf.volloss_event_source[od->id_inst_ptr[2]-1] == PHASEC){
			//get_event_data_from_ammeter(amm_sn.em_sn[od->id_inst_ptr[2]-1], AMM_EVENT_C_BROKEN_PHASE, amm_conf.phasebreak_event_times[od->id_inst_ptr[2]-1], pch, &data_len, RS485_PORT_USED_BY_645);
			if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PC_PHASEBREAK_EVENT_INFO, (struct sinkinfo_em_volloss_event_st *)value, SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE)){
				PRIVMIB_INFO(("EM phase break event data read SUCCESS!\r\n"));
			}
			else{
				printf_syn("EM phase break event data read failed!try read again!\r\n");
				if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PC_PHASEBREAK_EVENT_INFO, (struct sinkinfo_em_volloss_event_st *)value, SINKINFO_EM_PX_VOLLOSS_EVENT_SIZE)){
					printf_syn("EM phase break event data read failed!\r\n");
				}
				else{
					PRIVMIB_INFO(("EM phase break event data read SUCCESS!\r\n"));
				}
			}
		}else{
			printf_syn("%s(), cann't read meter event data\n", __FUNCTION__);
		}					
		break;
	case LT300SYS_METER_CUR_LOSS_TOTAL_TIMES_ID:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}
		if (SUCC == get_em_reg_info(&amm_sn)) {
			rt_memcpy(meteraddr, amm_sn.em_sn[od->id_inst_ptr[2]-1], DEV_SN_MODE_LEN);
		} else {
			printf_syn("%s(), read meter addr data tbl fail\n", __FUNCTION__);
		}
		if (FRAME_E_OK == get_event_data_from_ammeter(meteraddr, AMM_EVENT_LOSE_CURRENT, amm_conf.curloss_event_times[od->id_inst_ptr[2]-1], eventdata,  &data_len, RS485_PORT_USED_BY_645)){
			amm_conf.curloss_event_total_times[od->id_inst_ptr[2]-1][0] = eventdata[2]<<16 | eventdata[1]<<8 | eventdata[0];
			amm_conf.curloss_event_total_times[od->id_inst_ptr[2]-1][1] = eventdata[5]<<16 | eventdata[4]<<8 | eventdata[3];
			amm_conf.curloss_event_total_times[od->id_inst_ptr[2]-1][2] = eventdata[8]<<16 | eventdata[7]<<8 | eventdata[6];
			amm_conf.curloss_event_total_times[od->id_inst_ptr[2]-1][3] = eventdata[11]<<16 | eventdata[10]<<8 | eventdata[9];
			amm_conf.curloss_event_total_times[od->id_inst_ptr[2]-1][4] = eventdata[14]<<16 | eventdata[13]<<8 | eventdata[12];
			amm_conf.curloss_event_total_times[od->id_inst_ptr[2]-1][5] = eventdata[17]<<16 | eventdata[16]<<8 | eventdata[15];
		}
		else{
			printf_syn("%s(), read total volloss times fail\n", __FUNCTION__);
		}
//		rt_memcpy((rt_uint32_t *)value, amm_conf.curloss_event_total_times[od->id_inst_ptr[2]-1], sizeof(amm_conf.curloss_event_total_times[od->id_inst_ptr[2]-1]));David
		break;
	case LT300SYS_METER_CUR_LOSS_TIMES_SET_ID:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		*uint_ptr = (rt_uint32_t)amm_conf.curloss_event_times[od->id_inst_ptr[2]-1];	
		break;
	case LT300SYS_METER_CUR_LOSS_EVENT_SOURCE_ID:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		*uint_ptr = (rt_uint32_t)amm_conf.curloss_event_source[od->id_inst_ptr[2]-1];	
		break;
	case LT300SYS_METER_CUR_LOSS_EVENT_ID:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}
		if (SUCC == get_em_reg_info(&amm_sn)) {
			printf_syn("%s(), read meter SN data tbl fail\n", __FUNCTION__);
		} 
		if(amm_conf.volloss_event_source[od->id_inst_ptr[2]-1] == PHASEA){
			//get_event_data_from_ammeter(amm_sn.em_sn[od->id_inst_ptr[2]-1], AMM_EVENT_A_LOSE_CURRENT, amm_conf.curloss_event_times[od->id_inst_ptr[2]-1], pch, &data_len, RS485_PORT_USED_BY_645);
			if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PA_CURLOSS_EVENT_INFO, (struct sinkinfo_em_curloss_event_st *)value, SINKINFO_EM_PX_CURLOSS_EVENT_SIZE)){
				PRIVMIB_INFO(("EM current loss event data read SUCCESS!\r\n"));
			}
			else{
				printf_syn("EM current loss event data read failed!try read again!\r\n");
				if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PA_CURLOSS_EVENT_INFO, (struct sinkinfo_em_curloss_event_st *)value, SINKINFO_EM_PX_CURLOSS_EVENT_SIZE)){
					printf_syn("EM current loss event data read failed!\r\n");
				}
				else{
					PRIVMIB_INFO(("EM current loss event data read SUCCESS!\r\n"));
				}
			}
		}else if(amm_conf.volloss_event_source[od->id_inst_ptr[2]-1] == PHASEB){
			//get_event_data_from_ammeter(amm_sn.em_sn[od->id_inst_ptr[2]-1], AMM_EVENT_B_LOSE_CURRENT, amm_conf.curloss_event_times[od->id_inst_ptr[2]-1], pch, &data_len, RS485_PORT_USED_BY_645);
			if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PB_CURLOSS_EVENT_INFO, (struct sinkinfo_em_curloss_event_st *)value, SINKINFO_EM_PX_CURLOSS_EVENT_SIZE)){
				PRIVMIB_INFO(("EM current loss event data read SUCCESS!\r\n"));
			}
			else{
				printf_syn("EM current loss event data read failed!try read again!\r\n");
				if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PB_CURLOSS_EVENT_INFO, (struct sinkinfo_em_curloss_event_st *)value, SINKINFO_EM_PX_CURLOSS_EVENT_SIZE)){
					printf_syn("EM current loss event data read failed!\r\n");
				}
				else{
					PRIVMIB_INFO(("EM current loss event data read SUCCESS!\r\n"));
				}
			}
		}else if(amm_conf.volloss_event_source[od->id_inst_ptr[2]-1] == PHASEC){
			//get_event_data_from_ammeter(amm_sn.em_sn[od->id_inst_ptr[2]-1], AMM_EVENT_C_LOSE_CURRENT, amm_conf.curloss_event_times[od->id_inst_ptr[2]-1], pch, &data_len, RS485_PORT_USED_BY_645);
			if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PC_CURLOSS_EVENT_INFO, (struct sinkinfo_em_curloss_event_st *)value, SINKINFO_EM_PX_CURLOSS_EVENT_SIZE)){
				PRIVMIB_INFO(("EM current loss event data read SUCCESS!\r\n"));
			}
			else{
				printf_syn("EM current loss event data read failed!try read again!\r\n");
				if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PC_CURLOSS_EVENT_INFO, (struct sinkinfo_em_curloss_event_st *)value, SINKINFO_EM_PX_CURLOSS_EVENT_SIZE)){
					printf_syn("EM current loss event data read failed!\r\n");
				}
				else{
					PRIVMIB_INFO(("EM current loss event data read SUCCESS!\r\n"));
				}
			}
		}else{
			printf_syn("%s(), cann't read meter event data\n", __FUNCTION__);
		}	
		break;		
	case LT300SYS_METER_CUR_OVER_TOTAL_TIMES_ID:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}
		if (SUCC == get_em_reg_info(&amm_sn)) {
			rt_memcpy(meteraddr, amm_sn.em_sn[od->id_inst_ptr[2]-1], DEV_SN_MODE_LEN);
		} else {
			printf_syn("%s(), read meter addr data tbl fail\n", __FUNCTION__);
		}
		if (FRAME_E_OK == get_event_data_from_ammeter(meteraddr, AMM_EVENT_OVER_CURRENT, amm_conf.curover_event_times[od->id_inst_ptr[2]-1], eventdata,  &data_len, RS485_PORT_USED_BY_645)){
			amm_conf.curover_event_total_times[od->id_inst_ptr[2]-1][0] = eventdata[2]<<16 | eventdata[1]<<8 | eventdata[0];
			amm_conf.curover_event_total_times[od->id_inst_ptr[2]-1][1] = eventdata[5]<<16 | eventdata[4]<<8 | eventdata[3];
			amm_conf.curover_event_total_times[od->id_inst_ptr[2]-1][2] = eventdata[8]<<16 | eventdata[7]<<8 | eventdata[6];
			amm_conf.curover_event_total_times[od->id_inst_ptr[2]-1][3] = eventdata[11]<<16 | eventdata[10]<<8 | eventdata[9];
			amm_conf.curover_event_total_times[od->id_inst_ptr[2]-1][4] = eventdata[14]<<16 | eventdata[13]<<8 | eventdata[12];
			amm_conf.curover_event_total_times[od->id_inst_ptr[2]-1][5] = eventdata[17]<<16 | eventdata[16]<<8 | eventdata[15];
		}
		else{
			printf_syn("%s(), read total volloss times fail\n", __FUNCTION__);
		}
//		rt_memcpy((rt_uint32_t *)value, amm_conf.curover_event_total_times[od->id_inst_ptr[2]-1], sizeof(amm_conf.curover_event_total_times[od->id_inst_ptr[2]-1]));David
	
		break;
	case LT300SYS_METER_CUR_OVER_TIMES_SET_ID:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		*uint_ptr = (rt_uint32_t)amm_conf.curover_event_times[od->id_inst_ptr[2]-1];			
		
		break;
	case LT300SYS_METER_CUR_OVER_EVENT_SOURCE_ID:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		*uint_ptr = (rt_uint32_t)amm_conf.curover_event_source[od->id_inst_ptr[2]-1];				
			
		break;
	case LT300SYS_METER_CUR_OVER_EVENT_ID:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}
		if (SUCC == get_em_reg_info(&amm_sn)) {
			printf_syn("%s(), read meter SN data tbl fail\n", __FUNCTION__);
		} 
		if(amm_conf.volloss_event_source[od->id_inst_ptr[2]-1] == PHASEA){
			//get_event_data_from_ammeter(amm_sn.em_sn[od->id_inst_ptr[2]-1], AMM_EVENT_A_OVER_CURRENT, amm_conf.curover_event_times[od->id_inst_ptr[2]-1], pch, &data_len, RS485_PORT_USED_BY_645);
			if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PA_CUROVER_EVENT_INFO, (struct sinkinfo_em_curloss_event_st *)value, SINKINFO_EM_PX_CURLOSS_EVENT_SIZE)){
				PRIVMIB_INFO(("EM current over event data read SUCCESS!\r\n"));
			}
			else{
				printf_syn("EM current over event data read failed!try read again!\r\n");
				if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PA_CUROVER_EVENT_INFO, (struct sinkinfo_em_curloss_event_st *)value, SINKINFO_EM_PX_CURLOSS_EVENT_SIZE)){
					printf_syn("EM current over event data read failed!\r\n");
				}
				else{
					PRIVMIB_INFO(("EM current over event data read SUCCESS!\r\n"));
				}
			}
		}else if(amm_conf.volloss_event_source[od->id_inst_ptr[2]-1] == PHASEB){
			//get_event_data_from_ammeter(amm_sn.em_sn[od->id_inst_ptr[2]-1], AMM_EVENT_B_OVER_CURRENT, amm_conf.curover_event_times[od->id_inst_ptr[2]-1], pch, &data_len, RS485_PORT_USED_BY_645);
			if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PB_CUROVER_EVENT_INFO, (struct sinkinfo_em_curloss_event_st *)value, SINKINFO_EM_PX_CURLOSS_EVENT_SIZE)){
				PRIVMIB_INFO(("EM current over event data read SUCCESS!\r\n"));
			}
			else{
				printf_syn("EM current over event data read failed!try read again!\r\n");
				if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PB_CUROVER_EVENT_INFO, (struct sinkinfo_em_curloss_event_st *)value, SINKINFO_EM_PX_CURLOSS_EVENT_SIZE)){
					printf_syn("EM current over event data read failed!\r\n");
				}
				else{
					PRIVMIB_INFO(("EM current over event data read SUCCESS!\r\n"));
				}
			}
		}else if(amm_conf.volloss_event_source[od->id_inst_ptr[2]-1] == PHASEC){
			//get_event_data_from_ammeter(amm_sn.em_sn[od->id_inst_ptr[2]-1], AMM_EVENT_C_OVER_CURRENT, amm_conf.curover_event_times[od->id_inst_ptr[2]-1], pch, &data_len, RS485_PORT_USED_BY_645);
			if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PC_CUROVER_EVENT_INFO, (struct sinkinfo_em_curloss_event_st *)value, SINKINFO_EM_PX_CURLOSS_EVENT_SIZE)){
				PRIVMIB_INFO(("EM current over event data read SUCCESS!\r\n"));
			}
			else{
				printf_syn("EM current over event data read failed!try read again!\r\n");
				if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PC_CUROVER_EVENT_INFO, (struct sinkinfo_em_curloss_event_st *)value, SINKINFO_EM_PX_CURLOSS_EVENT_SIZE)){
					printf_syn("EM current over event data read failed!\r\n");
				}
				else{
					PRIVMIB_INFO(("EM current over event data read SUCCESS!\r\n"));
				}
			}
		}else{
			printf_syn("%s(), cann't read meter event data\n", __FUNCTION__);
		}		
		break;
	case LT300SYS_METER_CUR_BREAK_TOTAL_TIMES_ID:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}
		if (SUCC == get_em_reg_info(&amm_sn)) {
			rt_memcpy(meteraddr, amm_sn.em_sn[od->id_inst_ptr[2]-1], DEV_SN_MODE_LEN);
		} else {
			printf_syn("%s(), read meter addr data tbl fail\n", __FUNCTION__);
		}
		if (FRAME_E_OK == get_event_data_from_ammeter(meteraddr, AMM_EVENT_BROKEN_CURRENT, amm_conf.curbreak_event_times[od->id_inst_ptr[2]-1], eventdata,  &data_len, RS485_PORT_USED_BY_645)){
			amm_conf.curbreak_event_total_times[od->id_inst_ptr[2]-1][0] = eventdata[2]<<16 | eventdata[1]<<8 | eventdata[0];
			amm_conf.curbreak_event_total_times[od->id_inst_ptr[2]-1][1] = eventdata[5]<<16 | eventdata[4]<<8 | eventdata[3];
			amm_conf.curbreak_event_total_times[od->id_inst_ptr[2]-1][2] = eventdata[8]<<16 | eventdata[7]<<8 | eventdata[6];
			amm_conf.curbreak_event_total_times[od->id_inst_ptr[2]-1][3] = eventdata[11]<<16 | eventdata[10]<<8 | eventdata[9];
			amm_conf.curbreak_event_total_times[od->id_inst_ptr[2]-1][4] = eventdata[14]<<16 | eventdata[13]<<8 | eventdata[12];
			amm_conf.curbreak_event_total_times[od->id_inst_ptr[2]-1][5] = eventdata[17]<<16 | eventdata[16]<<8 | eventdata[15];
		}
		else{
			printf_syn("%s(), read total volloss times fail\n", __FUNCTION__);
		}
//		rt_memcpy((rt_uint32_t *)value, amm_conf.curbreak_event_total_times[od->id_inst_ptr[2]-1], sizeof(amm_conf.curbreak_event_total_times[od->id_inst_ptr[2]-1]));David
		break;

	case LT300SYS_METER_CUR_BREAK_TIMES_SET_ID:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		*uint_ptr = (rt_uint32_t)amm_conf.curbreak_event_times[od->id_inst_ptr[2]-1];		
		break;

	case LT300SYS_METER_CUR_BREAK_EVENT_SOURCE_ID:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		*uint_ptr = (rt_uint32_t)amm_conf.curbreak_event_source[od->id_inst_ptr[2]-1];		
		break;

	case LT300SYS_METER_CUR_BREAK_EVENT_ID:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}
		if (SUCC == get_em_reg_info(&amm_sn)) {
			printf_syn("%s(), read meter SN data tbl fail\n", __FUNCTION__);
		} 
		if(amm_conf.volloss_event_source[od->id_inst_ptr[2]-1] == PHASEA){
			//get_event_data_from_ammeter(amm_sn.em_sn[od->id_inst_ptr[2]-1], AMM_EVENT_A_BROKEN_CURRENT, amm_conf.curbreak_event_times[od->id_inst_ptr[2]-1], pch, &data_len, RS485_PORT_USED_BY_645);
			if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PA_CURBREAK_EVENT_INFO, (struct sinkinfo_em_curloss_event_st *)value, SINKINFO_EM_PX_CURLOSS_EVENT_SIZE)){
				PRIVMIB_INFO(("EM current break event data read SUCCESS!\r\n"));
			}
			else{
				printf_syn("EM current break event data read failed!try read again!\r\n");
				if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PA_CURBREAK_EVENT_INFO, (struct sinkinfo_em_curloss_event_st *)value, SINKINFO_EM_PX_CURLOSS_EVENT_SIZE)){
					printf_syn("EM current break event data read failed!\r\n");
				}
				else{
					PRIVMIB_INFO(("EM current break event data read SUCCESS!\r\n"));
				}
			}
		}else if(amm_conf.volloss_event_source[od->id_inst_ptr[2]-1] == PHASEB){
			//get_event_data_from_ammeter(amm_sn.em_sn[od->id_inst_ptr[2]-1], AMM_EVENT_B_BROKEN_CURRENT, amm_conf.curbreak_event_times[od->id_inst_ptr[2]-1], pch, &data_len, RS485_PORT_USED_BY_645);
			if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PB_CURBREAK_EVENT_INFO, (struct sinkinfo_em_curloss_event_st *)value, SINKINFO_EM_PX_CURLOSS_EVENT_SIZE)){
				PRIVMIB_INFO(("EM current break event data read SUCCESS!\r\n"));
			}
			else{
				printf_syn("EM current break event data read failed!try read again!\r\n");
				if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PB_CURBREAK_EVENT_INFO, (struct sinkinfo_em_curloss_event_st *)value, SINKINFO_EM_PX_CURLOSS_EVENT_SIZE)){
					printf_syn("EM current break event data read failed!\r\n");
				}
				else{
					PRIVMIB_INFO(("EM current break event data read SUCCESS!\r\n"));
				}
			}

		}else if(amm_conf.volloss_event_source[od->id_inst_ptr[2]-1] == PHASEC){
			//get_event_data_from_ammeter(amm_sn.em_sn[od->id_inst_ptr[2]-1], AMM_EVENT_C_BROKEN_CURRENT, amm_conf.curbreak_event_times[od->id_inst_ptr[2]-1], pch, &data_len, RS485_PORT_USED_BY_645);
			if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PC_CURBREAK_EVENT_INFO, (struct sinkinfo_em_curloss_event_st *)value, SINKINFO_EM_PX_CURLOSS_EVENT_SIZE)){
				PRIVMIB_INFO(("EM current break event data read SUCCESS!\r\n"));
			}
			else{
				printf_syn("EM current break event data read failed!try read again!\r\n");
				if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PC_CURBREAK_EVENT_INFO, (struct sinkinfo_em_curloss_event_st *)value, SINKINFO_EM_PX_CURLOSS_EVENT_SIZE)){
					printf_syn("EM current break event data read failed!\r\n");
				}
				else{
					PRIVMIB_INFO(("EM current break event data read SUCCESS!\r\n"));
				}
			}

		}else{
			printf_syn("%s(), cann't read meter event data\n", __FUNCTION__);
		}		
		break;

	case LT300SYS_METER_CLEAR_EVENT_TOTAL_TIMES_ID:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}
		if (SUCC == get_em_reg_info(&amm_sn)) {
			rt_memcpy(meteraddr, amm_sn.em_sn[od->id_inst_ptr[2]-1], DEV_SN_MODE_LEN);
		} else {
			printf_syn("%s(), read meter addr data tbl fail\n", __FUNCTION__);
		}
		if (FRAME_E_OK == get_event_data_from_ammeter(meteraddr, AMM_EVENT_AMM_RESET, amm_conf.meterclear_event_times[od->id_inst_ptr[2]-1], eventdata,  &data_len, RS485_PORT_USED_BY_645)){
			amm_conf.meterclear_event_total_times[od->id_inst_ptr[2]-1] = eventdata[2]<<16 | eventdata[1]<<8 | eventdata[0];	
		}
		else{
			printf_syn("%s(), read total volloss times fail\n", __FUNCTION__);
		}
			
		*uint_ptr = amm_conf.meterclear_event_total_times[od->id_inst_ptr[2]-1];
		break;
	case LT300SYS_METER_CLEAR_TIMES_SET_ID:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		*uint_ptr = (rt_uint32_t)amm_conf.meterclear_event_times[od->id_inst_ptr[2]-1];	
	
		break;
	case LT300SYS_METER_CLEAR_EVENT_ID:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}
		if (SUCC == get_em_reg_info(&amm_sn)) {
			printf_syn("%s(), read meter SN data tbl fail\n", __FUNCTION__);
		} 
		//get_event_data_from_ammeter(amm_sn.em_sn[od->id_inst_ptr[2]-1], AMM_EVENT_AMMETER_RESET, amm_conf.meterclear_event_times[od->id_inst_ptr[2]-1], pch, &data_len, RS485_PORT_USED_BY_645);
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_METER_CLEAR_EVENT_INFO, (struct sinkinfo_em_meterclear_event_st *)value, SINKINFO_EM_METER_CLEAR_EVENT_SIZE)){
			PRIVMIB_INFO(("EM meter clear event data read SUCCESS!\r\n"));
		}
		else{
			printf_syn("EM meter clear event data read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_METER_CLEAR_EVENT_INFO, (struct sinkinfo_em_meterclear_event_st *)value, SINKINFO_EM_METER_CLEAR_EVENT_SIZE)){
				printf_syn("EM meter clear event data read failed!\r\n");
			}
			else{
				PRIVMIB_INFO(("EM meter clear event data read SUCCESS!\r\n"));
			}
		}	
		break;
	case LT300SYS_METER_DEMAND_CLEAR_TOTAL_TIMES_ID:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}
		if (SUCC == get_em_reg_info(&amm_sn)) {
			rt_memcpy(meteraddr, amm_sn.em_sn[od->id_inst_ptr[2]-1], DEV_SN_MODE_LEN);
		} else {
			printf_syn("%s(), read meter addr data tbl fail\n", __FUNCTION__);
		}
		if (FRAME_E_OK == get_event_data_from_ammeter(meteraddr, AMM_EVENT_REQUIRED_RESET, amm_conf.demandclear_event_times[od->id_inst_ptr[2]-1], eventdata,  &data_len, RS485_PORT_USED_BY_645)){
			amm_conf.demandclear_event_total_times[od->id_inst_ptr[2]-1] = eventdata[2]<<16 | eventdata[1]<<8 | eventdata[0];	
		}
		else{
			printf_syn("%s(), read total volloss times fail\n", __FUNCTION__);
		}
			
		*uint_ptr = amm_conf.demandclear_event_total_times[od->id_inst_ptr[2]-1];	
		
		break;
	case LT300SYS_METER_DEMAND_CLEAR_TIMES_SET_ID:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		*uint_ptr = (rt_uint32_t)amm_conf.demandclear_event_times[od->id_inst_ptr[2]-1];		
			
		break;

	case LT300SYS_METER_DEMAND_CLEAR_EVENT_ID:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}
		if (SUCC == get_em_reg_info(&amm_sn)) {
			printf_syn("%s(), read meter SN data tbl fail\n", __FUNCTION__);
		} 	
	//	get_event_data_from_ammeter(amm_sn.em_sn[od->id_inst_ptr[2]-1], AMM_EVENT_NEED_RESET, amm_conf.demandclear_event_times[od->id_inst_ptr[2]-1], pch, &data_len, RS485_PORT_USED_BY_645);	
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_DEMAND_CLEAR_EVENT_INFO, (struct sinkinfo_em_demandclear_event_st *)value, SINKINFO_EM_DEMAND_CLEAR_EVENT_SIZE)){
			PRIVMIB_INFO(("EM demand clear event data read SUCCESS!\r\n"));
		}
		else{
			printf_syn("EM demand clear event data read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_DEMAND_CLEAR_EVENT_INFO, (struct sinkinfo_em_demandclear_event_st *)value, SINKINFO_EM_DEMAND_CLEAR_EVENT_SIZE)){
				printf_syn("EM demand clear event data read failed!\r\n");
			}
			else{
				PRIVMIB_INFO(("EM demand clear event data read SUCCESS!\r\n"));
			}
		}
		break;
	case LT300SYS_METER_PROGRAMMING_TOTAL_TIMES_ID:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}
		if (SUCC == get_em_reg_info(&amm_sn)) {
			rt_memcpy(meteraddr, amm_sn.em_sn[od->id_inst_ptr[2]-1], DEV_SN_MODE_LEN);
		} else {
			printf_syn("%s(), read meter addr data tbl fail\n", __FUNCTION__);
		}
		if (FRAME_E_OK == get_event_data_from_ammeter(meteraddr, AMM_EVENT_PROGRAMMING, amm_conf.program_event_times[od->id_inst_ptr[2]-1], eventdata,  &data_len, RS485_PORT_USED_BY_645)){
			amm_conf.program_event_total_times[od->id_inst_ptr[2]-1] = eventdata[2]<<16 | eventdata[1]<<8 | eventdata[0];	
		}
		else{
			printf_syn("%s(), read total volloss times fail\n", __FUNCTION__);
		}
			
		*uint_ptr = amm_conf.program_event_total_times[od->id_inst_ptr[2]-1];		
		break;

	case LT300SYS_METER_PROGRAMMING_TIMES_SET_ID:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		*uint_ptr = (rt_uint32_t)amm_conf.program_event_times[od->id_inst_ptr[2]-1];		
		break;

	case LT300SYS_METER_PROGRAMMING_EVENT_ID:
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}
		if (SUCC == get_em_reg_info(&amm_sn)) {
			printf_syn("%s(), read meter SN data tbl fail\n", __FUNCTION__);
		} 
		//get_event_data_from_ammeter(amm_sn.em_sn[od->id_inst_ptr[2]-1], AMM_EVENT_PROGRAM_RECORD, amm_conf.program_event_times[od->id_inst_ptr[2]-1], pch, &data_len, RS485_PORT_USED_BY_645);
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PROGRAM_EVENT_INFO, (struct sinkinfo_em_program_event_st *)value, SINKINFO_EM_PROGRAM_EVENT_SIZE)){
			PRIVMIB_INFO(("EM program event data read SUCCESS!\r\n"));
		}
		else{
			printf_syn("EM program event data read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_PROGRAM_EVENT_INFO, (struct sinkinfo_em_program_event_st *)value, SINKINFO_EM_PROGRAM_EVENT_SIZE)){
				printf_syn("EM program event data read failed!\r\n");
			}
			else{
				PRIVMIB_INFO(("EM program event data read SUCCESS!\r\n"));
			}
		}		
		break;		
	case LT300SYS_METER_CALIBRATE_TIME_TOTAL_TIMES_ID: 
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}
		if (SUCC == get_em_reg_info(&amm_sn)) {
			rt_memcpy(meteraddr, amm_sn.em_sn[od->id_inst_ptr[2]-1], DEV_SN_MODE_LEN);
		} else {
			printf_syn("%s(), read meter addr data tbl fail\n", __FUNCTION__);
		}
		if (FRAME_E_OK == get_event_data_from_ammeter(meteraddr, AMM_EVENT_CALIBRATION_TIME, amm_conf.calibratetime_event_times[od->id_inst_ptr[2]-1], eventdata,  &data_len, RS485_PORT_USED_BY_645)){
			amm_conf.calibratetime_event_total_times[od->id_inst_ptr[2]-1] = eventdata[2]<<16 | eventdata[1]<<8 | eventdata[0];	
		}
		else{
			printf_syn("%s(), read total volloss times fail\n", __FUNCTION__);
		}
			
		*uint_ptr = amm_conf.calibratetime_event_total_times[od->id_inst_ptr[2]-1];			
		break;

	case LT300SYS_METER_CALIBRATE_TIME_TIMES_SET_ID: 
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		*uint_ptr = (rt_uint32_t)amm_conf.calibratetime_event_times[od->id_inst_ptr[2]-1];			
		break;

	case LT300SYS_METER_CALIBRATE_TIME_EVENT_ID: 
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}
		if (SUCC == get_em_reg_info(&amm_sn)) {
			printf_syn("%s(), read meter SN data tbl fail\n", __FUNCTION__);
		} 	
		//get_event_data_from_ammeter(amm_sn.em_sn[od->id_inst_ptr[2]-1], AMM_EVENT_TIMING_RECORD, amm_conf.calibratetime_event_times[od->id_inst_ptr[2]-1], pch, &data_len, RS485_PORT_USED_BY_645);			
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_CALIBRATE_TIME_EVENT_INFO, (struct sinkinfo_em_calibratetime_event_st *)value, SINKINFO_EM_CALIBRATE_TIME_EVENT_SIZE)){
			PRIVMIB_INFO(("EM calibrate time event data read SUCCESS!\r\n"));
		}
		else{
			printf_syn("EM calibrate time event data read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_CALIBRATE_TIME_EVENT_INFO, (struct sinkinfo_em_calibratetime_event_st *)value, SINKINFO_EM_CALIBRATE_TIME_EVENT_SIZE)){
				printf_syn("EM calibrate time event data read failed!\r\n");
			}
			else{
				PRIVMIB_INFO(("EM calibrate time event data read SUCCESS!\r\n"));
			}
		}
		break;		
	case LT300SYS_METER_VOL_REV_PHASE_TOTAL_TIMES_ID: 
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		} 
		if (SUCC == get_em_reg_info(&amm_sn)) {
			rt_memcpy(meteraddr, amm_sn.em_sn[od->id_inst_ptr[2]-1], DEV_SN_MODE_LEN);
		} else {
			printf_syn("%s(), read meter addr data tbl fail\n", __FUNCTION__);
		}
		if (FRAME_E_OK == get_event_data_from_ammeter(meteraddr, AMM_EVENT_VOLTAGE_INVERSE_PHASE, amm_conf.rseqvol_event_times[od->id_inst_ptr[2]-1], eventdata,  &data_len, RS485_PORT_USED_BY_645)){
			amm_conf.rseqvol_event_total_times[od->id_inst_ptr[2]-1][0] = eventdata[2]<<16 | eventdata[1]<<8 | eventdata[0];
			amm_conf.rseqvol_event_total_times[od->id_inst_ptr[2]-1][1] = eventdata[5]<<16 | eventdata[4]<<8 | eventdata[3];
		}
		else{
			printf_syn("%s(), read total volloss times fail\n", __FUNCTION__);
		}
		rt_memcpy((rt_uint32_t *)value, amm_conf.rseqvol_event_total_times[od->id_inst_ptr[2]-1], sizeof(amm_conf.rseqvol_event_total_times[od->id_inst_ptr[2]-1]));				
		break;

	case LT300SYS_METER_VOL_REV_PHASE_TIMES_SET_ID: 
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		*uint_ptr = (rt_uint32_t)amm_conf.rseqvol_event_times[od->id_inst_ptr[2]-1];			
		break;

	case LT300SYS_METER_VOL_REV_PHASE_EVENT_ID: 
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}
		if (SUCC == get_em_reg_info(&amm_sn)) {
			printf_syn("%s(), read meter SN data tbl fail\n", __FUNCTION__);
		} 
		//get_event_data_from_ammeter(amm_sn.em_sn[od->id_inst_ptr[2]-1], AMM_EVENT_VOLTAGE_ANTI_PHASE_RECORD, amm_conf.rseqvol_event_times[od->id_inst_ptr[2]-1], pch, &data_len, RS485_PORT_USED_BY_645);			
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_REVERSE_REQ_VOL_EVENT_INFO, (struct sinkinfo_em_rseqvol_event_st *)value, SINKINFO_EM_REVERSE_SEQVOL_EVENT_SIZE)){
			PRIVMIB_INFO(("EM reverse sequence vol event data read SUCCESS!\r\n"));
		}
		else{
			printf_syn("EM reverse sequence vol event data read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_REVERSE_REQ_VOL_EVENT_INFO, (struct sinkinfo_em_rseqvol_event_st *)value, SINKINFO_EM_REVERSE_SEQVOL_EVENT_SIZE)){
				printf_syn("EM reverse sequence vol event data read failed!\r\n");
			}
			else{
				PRIVMIB_INFO(("EM reverse sequence vol event data read SUCCESS!\r\n"));
			}
		}
		break;		
	case LT300SYS_METER_CUL_REV_PHASE_TOTAL_TIMES_ID: 
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}
		if (SUCC == get_em_reg_info(&amm_sn)) {
			rt_memcpy(meteraddr, amm_sn.em_sn[od->id_inst_ptr[2]-1], DEV_SN_MODE_LEN);
		} else {
			printf_syn("%s(), read meter addr data tbl fail\n", __FUNCTION__);
		}
		if (FRAME_E_OK == get_event_data_from_ammeter(meteraddr, AMM_EVENT_CURRENT_INVERSE_PHASE, amm_conf.rseqcur_event_times[od->id_inst_ptr[2]-1], eventdata,  &data_len, RS485_PORT_USED_BY_645)){
			amm_conf.rseqcur_event_total_times[od->id_inst_ptr[2]-1][0] = eventdata[2]<<16 | eventdata[1]<<8 | eventdata[0];
			amm_conf.rseqcur_event_total_times[od->id_inst_ptr[2]-1][1] = eventdata[5]<<16 | eventdata[4]<<8 | eventdata[3];
		}
		else{
			printf_syn("%s(), read total volloss times fail\n", __FUNCTION__);
		}
		rt_memcpy((rt_uint32_t *)value, amm_conf.rseqcur_event_total_times[od->id_inst_ptr[2]-1], sizeof(amm_conf.rseqcur_event_total_times[od->id_inst_ptr[2]-1]));			
		break;
	case LT300SYS_METER_CUL_REV_PHASE_TIMES_SET_ID: 
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}	
		*uint_ptr = (rt_uint32_t)amm_conf.rseqcur_event_times[od->id_inst_ptr[2]-1];			
		break;

	case LT300SYS_METER_CUL_REV_PHASE_EVENT_ID: 
		if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read col event data tbl fail\n", __FUNCTION__);
		}
		if (SUCC == get_em_reg_info(&amm_sn)) {
			printf_syn("%s(), read meter SN data tbl fail\n", __FUNCTION__);
		} 
		//get_event_data_from_ammeter(amm_sn.em_sn[od->id_inst_ptr[2]-1], AMM_EVENT_CURRENT_ANTI_PHASE_RECORD, amm_conf.rseqcur_event_times[od->id_inst_ptr[2]-1], pch, &data_len, RS485_PORT_USED_BY_645);			
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_REVERSE_REQ_CUR_EVENT_INFO, (struct sinkinfo_em_rseqvol_event_st *)value, SINKINFO_EM_REVERSE_SEQVOL_EVENT_SIZE)){
			PRIVMIB_INFO(("EM reverse sequence current event data read SUCCESS!\r\n"));
		}
		else{
			printf_syn("EM reverse sequence current event data read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_REVERSE_REQ_CUR_EVENT_INFO, (struct sinkinfo_em_rseqvol_event_st *)value, SINKINFO_EM_REVERSE_SEQVOL_EVENT_SIZE)){
				printf_syn("EM reverse sequence current event data read failed!\r\n");
			}
			else{
				PRIVMIB_INFO(("EM reverse sequence current event data read SUCCESS!\r\n"));
			}
		}
		break;		
	default:
		break;
	}

	return;
}

static u8_t yjlt300sys_meterevent_read_entry_set_test(struct obj_def *od, u16_t len, void *value)
{
	//u32_t *uint_ptr = value;
	u8_t id, set_ok;

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	set_ok = 0;
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
		
	case LT300SYS_METER_VOL_LOSS_TIMES_SET_ID:
	case LT300SYS_METER_VOL_LOSS_EVENT_SOURCE_ID:
	case LT300SYS_METER_VOL_OVER_TIMES_SET_ID:
	case LT300SYS_METER_VOL_OVER_EVENT_SOURCE_ID:
	case LT300SYS_METER_VOL_UNDER_TIMES_SET_ID:			
	case LT300SYS_METER_VOL_UNDER_EVENT_SOURCE_ID:
	case LT300SYS_METER_PHASE_BREAK_TIMES_SET_ID:	
	case LT300SYS_METER_PHASE_BREAK_EVENT_SOURCE_ID:
	case LT300SYS_METER_CUR_LOSS_TIMES_SET_ID:	
	case LT300SYS_METER_CUR_LOSS_EVENT_SOURCE_ID:	
	case LT300SYS_METER_CUR_OVER_TIMES_SET_ID:	
	case LT300SYS_METER_CUR_OVER_EVENT_SOURCE_ID:
	case LT300SYS_METER_CUR_BREAK_TIMES_SET_ID:	
	case LT300SYS_METER_CUR_BREAK_EVENT_SOURCE_ID: 
	case LT300SYS_METER_CLEAR_TIMES_SET_ID:	
	case LT300SYS_METER_DEMAND_CLEAR_TIMES_SET_ID: 
	case LT300SYS_METER_PROGRAMMING_TIMES_SET_ID: 
	case LT300SYS_METER_CALIBRATE_TIME_TIMES_SET_ID: 
	case LT300SYS_METER_VOL_REV_PHASE_TIMES_SET_ID: 
	case LT300SYS_METER_CUL_REV_PHASE_TIMES_SET_ID: 

#if 0
		if ((0==*sint_ptr) || (1==*sint_ptr))
			set_ok = 1;
#else
		set_ok = 1;
#endif
		break;

	default:
		break;
	}

	return set_ok;
}

static void yjlt300sys_meterevent_read_entry_set_value(struct obj_def *od, u16_t len, void *value)
{
	u32_t *uint_ptr = (u32_t *)value;
	u8_t *pch = (u8_t *)value;
	//struct collect_meter_data_info_st data;
	//struct celectric_meter_config_info_st amm_conf;
	u8_t id;
    

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	/* @todo @fixme: which kind of pointer is 'value'? s32_t or u8_t??? */
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_METER_VOL_LOSS_TIMES_SET_ID:
		/*if (RT_EOK != read_syscfgdata_tbl_from_cache(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf)) {
			printf_syn("%s(), read syscfg data tbl fail\n", __FUNCTION__);
		}	
		amm_conf.volloss_event_times[od->id_inst_ptr[2]-1] = *pch;
		write_syscfgdata_tbl(SYSCFGDATA_TBL_EM_CONFIG_INFO, 0, &amm_conf);
		syscfgdata_syn_proc();*/
		set_em_info(od->id_inst_ptr[2]-1, 11, uint_ptr, pch);
		break;
	case LT300SYS_METER_VOL_LOSS_EVENT_SOURCE_ID:
		set_em_info(od->id_inst_ptr[2]-1, 12, uint_ptr, pch);
		break;
	case LT300SYS_METER_VOL_OVER_TIMES_SET_ID:
		set_em_info(od->id_inst_ptr[2]-1, 13, uint_ptr, pch);
		break;
	case LT300SYS_METER_VOL_OVER_EVENT_SOURCE_ID:
		set_em_info(od->id_inst_ptr[2]-1, 14, uint_ptr, pch);
		break;
	case LT300SYS_METER_VOL_UNDER_TIMES_SET_ID:
		set_em_info(od->id_inst_ptr[2]-1, 15, uint_ptr, pch);
		break;
	case LT300SYS_METER_VOL_UNDER_EVENT_SOURCE_ID:
		set_em_info(od->id_inst_ptr[2]-1, 16, uint_ptr, pch);
		break;
	case LT300SYS_METER_PHASE_BREAK_TIMES_SET_ID:
		set_em_info(od->id_inst_ptr[2]-1, 17, uint_ptr, pch);
		break;
	case LT300SYS_METER_PHASE_BREAK_EVENT_SOURCE_ID:
		set_em_info(od->id_inst_ptr[2]-1, 18, uint_ptr, pch);
		break;
	case LT300SYS_METER_CUR_LOSS_TIMES_SET_ID:
		set_em_info(od->id_inst_ptr[2]-1, 19, uint_ptr, pch);
		break;
	case LT300SYS_METER_CUR_LOSS_EVENT_SOURCE_ID:
		set_em_info(od->id_inst_ptr[2]-1, 20, uint_ptr, pch);
		break;
	case LT300SYS_METER_CUR_OVER_TIMES_SET_ID:
		set_em_info(od->id_inst_ptr[2]-1, 21, uint_ptr, pch);
		break;
	case LT300SYS_METER_CUR_OVER_EVENT_SOURCE_ID:
		set_em_info(od->id_inst_ptr[2]-1, 22, uint_ptr, pch);
		break;
	case LT300SYS_METER_CUR_BREAK_TIMES_SET_ID:
		set_em_info(od->id_inst_ptr[2]-1, 23, uint_ptr, pch);
		break;
	case LT300SYS_METER_CUR_BREAK_EVENT_SOURCE_ID:
		set_em_info(od->id_inst_ptr[2]-1, 24, uint_ptr, pch);
		break;
	case LT300SYS_METER_CLEAR_TIMES_SET_ID:
		set_em_info(od->id_inst_ptr[2]-1, 25, uint_ptr, pch);
		break;
	case LT300SYS_METER_DEMAND_CLEAR_TIMES_SET_ID:
		set_em_info(od->id_inst_ptr[2]-1, 26, uint_ptr, pch);
		break;
	case LT300SYS_METER_PROGRAMMING_TIMES_SET_ID:
		set_em_info(od->id_inst_ptr[2]-1, 27, uint_ptr, pch);
		break;
	case LT300SYS_METER_CALIBRATE_TIME_TIMES_SET_ID:
		set_em_info(od->id_inst_ptr[2]-1, 28, uint_ptr, pch);
		break;
	case LT300SYS_METER_VOL_REV_PHASE_TIMES_SET_ID:
		set_em_info(od->id_inst_ptr[2]-1, 29, uint_ptr, pch);
		break;
	case LT300SYS_METER_CUL_REV_PHASE_TIMES_SET_ID:
		set_em_info(od->id_inst_ptr[2]-1, 30, uint_ptr, pch);
		break;

	default:
		break;
	}


	return;
}
#endif

/*
 * PT data collecting
 */
static void yjlt300sys_ptdata_read_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
{
	/* return to object name, adding index depth (1) */
	ident_len += 3;
	ident     -= 3;

	if (ident_len == 4) {
		u8_t id;

		od->id_inst_len = ident_len;
		od->id_inst_ptr = ident;

		LWIP_ASSERT("invalid id", (ident[0] >= 0) && (ident[0] <= 0xff));

		id = (u8_t)ident[0];
		od->instance = MIB_OBJECT_TAB;
		switch (id) {
		case LT300SYS_PTDATA_PHASEA_READ_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 20;
			break;
		case LT300SYS_PTDATA_PHASEB_READ_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 20;
			break;
		case LT300SYS_PTDATA_PHASEC_READ_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 20;
			break;

		case LT300SYS_PTDATA_LOAD_READ_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 12;
			break;

		case LT300SYS_PTDATA_2VOLDROP_READ_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 12;
			break;
		case LT300SYS_PTDATA_TEMPERATURE_READ_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 2;
			break;
		case LT300SYS_PTDATA_READ_ALARM_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len    = sizeof(u32_t);
			break;

		case LT300SYS_PTDATA_READ_ALARM_MASK_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len    = sizeof(u32_t);
			break;

		default:
			LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no such object\n"));
			od->instance = MIB_OBJECT_NONE;
			break;
		}
	} else {
		LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no scalar\n"));
		od->instance = MIB_OBJECT_NONE;
	}

	return;
}

static void yjlt300sys_ptdata_read_entry_get_value(struct obj_def *od, u16_t len, void *value)
{
	u32_t *u32_ptr =(u32_t *)value;
	u8_t *pch =(u8_t *)value;
	//struct sinkinfo_pt_ct_st uint_ptr;
	struct collect_pt_data_info_st data;
	u8_t id,i;
	u32_t ptdata[32]={0};
	
	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));
	
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_PTDATA_PHASEA_READ_ID:
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, od->id_inst_ptr[3], SI_MGC_GET_PTC_PA_INFO, ptdata, SINKINFO_PTC_PX_DATA_SIZE)){
			for(i=0; i<SINKINFO_PTC_PX_DATA_SIZE/sizeof(u32_t); i++)
				*(u32_ptr+i) = lwip_htonl(ptdata[i]);
			
			//rt_memcpy(u32_ptr, ptdata, SINKINFO_PTC_PX_DATA_SIZE);
		}
		else{
			
			printf_syn("PT data  phaseA PARAM read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, od->id_inst_ptr[3], SI_MGC_GET_PTC_PA_INFO, ptdata, SINKINFO_PTC_PX_DATA_SIZE)){
 				printf_syn("PT data phaseA voltage read failed!\r\n");
			}
			else{
				for(i=0; i<SINKINFO_PTC_PX_DATA_SIZE/sizeof(u32_t); i++)
					*(u32_ptr+i) = lwip_htonl(ptdata[i]);
			
				//rt_memcpy(u32_ptr, ptdata, SINKINFO_PTC_PX_DATA_SIZE);
			}
		}
//		get_sinkinfo_abc_param(od->id_inst_ptr[2]-1, 0, SIC_GET_PT_VOL, uint_ptr, uint_ptr+1, uint_ptr+2); /* mark by David */
		break;	
	case LT300SYS_PTDATA_PHASEB_READ_ID:
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, od->id_inst_ptr[3], SI_MGC_GET_PTC_PB_INFO, ptdata, SINKINFO_PTC_PX_DATA_SIZE)){
			for(i=0; i<SINKINFO_PTC_PX_DATA_SIZE/sizeof(u32_t); i++)
				*(u32_ptr+i) = lwip_htonl(ptdata[i]);
			
			//rt_memcpy(u32_ptr, ptdata, SINKINFO_PTC_PX_DATA_SIZE);
		}
		else{
			
			printf_syn("PT data  phaseB PARAM read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, od->id_inst_ptr[3], SI_MGC_GET_PTC_PB_INFO, ptdata, SINKINFO_PTC_PX_DATA_SIZE)){
				printf_syn("PT data phaseB voltage read failed!\r\n");
			}
			else{
				for(i=0; i<SINKINFO_PTC_PX_DATA_SIZE/sizeof(u32_t); i++)
					*(u32_ptr+i) = lwip_htonl(ptdata[i]);
			
				//rt_memcpy(u32_ptr, ptdata, SINKINFO_PTC_PX_DATA_SIZE);
			}
		}

		break;	
	case LT300SYS_PTDATA_PHASEC_READ_ID:
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, od->id_inst_ptr[3], SI_MGC_GET_PTC_PC_INFO, ptdata, SINKINFO_PTC_PX_DATA_SIZE)){
			for(i=0; i<SINKINFO_PTC_PX_DATA_SIZE/sizeof(u32_t); i++)
				*(u32_ptr+i) = lwip_htonl(ptdata[i]);
			
			//rt_memcpy(u32_ptr, ptdata, SINKINFO_PTC_PX_DATA_SIZE);
		}
		else{
			
			printf_syn("PT data  phaseC PARAM read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, od->id_inst_ptr[3], SI_MGC_GET_PTC_PC_INFO, ptdata, SINKINFO_PTC_PX_DATA_SIZE)){
 				printf_syn("PT data phaseC voltage read failed!\r\n");
			}
			else{
				
				for(i=0; i<SINKINFO_PTC_PX_DATA_SIZE/sizeof(u32_t); i++)
					*(u32_ptr+i) = lwip_htonl(ptdata[i]);
			
				//rt_memcpy(u32_ptr, ptdata, SINKINFO_PTC_PX_DATA_SIZE);
			}
		}		
		break;	

	case LT300SYS_PTDATA_LOAD_READ_ID:
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2]-1, od->id_inst_ptr[3], SIC_GET_PT_LOAD, ptdata, ptdata+1, ptdata+2)){
			*(u32_ptr) = lwip_htonl(ptdata[0]);
			*(u32_ptr+1) = lwip_htonl(ptdata[1]);  
			*(u32_ptr+2) = lwip_htonl(ptdata[2]);
		}
		else{
			//LWIP_DEBUGF(SNMP_MIB_DEBUG,("line_data_read_voltage: errer, please read again!\n"));
			printf_syn("PT data  phaseA voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2]-1, od->id_inst_ptr[3], SIC_GET_PT_LOAD, ptdata, ptdata+1, ptdata+2)){
 				printf_syn("PT data phaseA voltage read failed!\r\n");
			}else{
				*(u32_ptr) = lwip_htonl(ptdata[0]);
				*(u32_ptr+1) = lwip_htonl(ptdata[1]);  
				*(u32_ptr+2) = lwip_htonl(ptdata[2]);
			}
			
		}
		rt_memcpy(&data.PtData.PtActPowA, u32_ptr, sizeof(data.PtData.PtActPowA));
		rt_memcpy(&data.PtData.PtActPowB, u32_ptr+1, sizeof(data.PtData.PtActPowB));
		rt_memcpy(&data.PtData.PtActPowC, u32_ptr+2, sizeof(data.PtData.PtActPowC));
		break;

	case LT300SYS_PTDATA_2VOLDROP_READ_ID:
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2]-1, od->id_inst_ptr[3], SIC_GET_PT_VOLTAGE_DROP, ptdata, ptdata+1, ptdata+2)){
			*(u32_ptr) = lwip_htonl(ptdata[0]);
			*(u32_ptr+1) = lwip_htonl(ptdata[1]);  
			*(u32_ptr+2) = lwip_htonl(ptdata[2]);
		}
		else{
			printf_syn("PT data  phaseA voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2]-1, od->id_inst_ptr[3], SIC_GET_PT_VOLTAGE_DROP, ptdata, ptdata+1, ptdata+2)){
 				printf_syn("PT data phaseA voltage read failed!\r\n");
			}else{
				*(u32_ptr) = lwip_htonl(ptdata[0]);
				*(u32_ptr+1) = lwip_htonl(ptdata[1]);  
				*(u32_ptr+2) = lwip_htonl(ptdata[2]);
			}
			
		}
		rt_memcpy(&data.PtData.Pt2VolADrop, u32_ptr, sizeof(data.PtData.Pt2VolADrop));
		rt_memcpy(&data.PtData.Pt2VolBDrop, u32_ptr+1, sizeof(data.PtData.Pt2VolBDrop));
		rt_memcpy(&data.PtData.Pt2VolCDrop, u32_ptr+2, sizeof(data.PtData.Pt2VolCDrop));
		break;
		
	case LT300SYS_PTDATA_TEMPERATURE_READ_ID:
		*u32_ptr = lwip_htons(INVLIDE_DATAS);			
		break;

	case LT300SYS_PTDATA_READ_ALARM_ID: /* mark by David */
		*pch = data.ptcur_alarm;
		break;

	case LT300SYS_PTDATA_READ_ALARM_MASK_ID:
		*pch = data.ptalarm_mask;
		break;
		
	default:
		break;
	}
//	write_syscfgdata_tbl(SYSCFGDATA_TBL_PT_DATA, 0, &data);  //hongbin E
	return;
}

static u8_t yjlt300sys_ptdata_read_entry_set_test(struct obj_def *od, u16_t len, void *value)
{
	u8_t id, set_ok;

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	set_ok = 0;
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_PTDATA_PHASEA_READ_ID:
	case LT300SYS_PTDATA_LOAD_READ_ID:
	case LT300SYS_PTDATA_2VOLDROP_READ_ID:	
	case LT300SYS_PTDATA_READ_ALARM_MASK_ID:	
#if 0
		if ((0==*sint_ptr) || (1==*sint_ptr))
			set_ok = 1;
#else
		set_ok = 1;
#endif
		break;

	default:
		break;
	}

	return set_ok;
}

static void yjlt300sys_ptdata_read_entry_set_value(struct obj_def *od, u16_t len, void *value)
{
//	u32_t *uint_ptr = (u32_t *)value;
//	struct collect_pt_data_info_st data;
	u8_t id;
    
	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

//	read_syscfgdata_tbl(SYSCFGDATA_TBL_PT_DATA, 0, &data);

	/* @todo @fixme: which kind of pointer is 'value'? s32_t or u8_t??? */
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_PTDATA_PHASEA_READ_ID:	
	case LT300SYS_PTDATA_LOAD_READ_ID:	
	case LT300SYS_PTDATA_2VOLDROP_READ_ID:	
		break;
		
	case LT300SYS_METERDATA_READ_ALARM_MASK_ID:
//	   data.ptalarm_mask = *uint_ptr;
		break;

	default:
		break;
	}

//	write_syscfgdata_tbl(SYSCFGDATA_TBL_PT_DATA, 0, &data);

	return;
}

/*
 * CT data collecting
 */
static void yjlt300sys_ctdata_read_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
{
	/* return to object name, adding index depth (1) */
	ident_len += 3;
	ident     -= 3;

	if (ident_len == 4) {
		u8_t id;

		od->id_inst_len = ident_len;
		od->id_inst_ptr = ident;

		LWIP_ASSERT("invalid id", (ident[0] >= 0) && (ident[0] <= 0xff));

		id = (u8_t)ident[0];
		od->instance = MIB_OBJECT_TAB;
		switch (id) {
		case LT300SYS_CTDATA_PHASEA_READ_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 20;
			break;
		case LT300SYS_CTDATA_PHASEB_READ_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 20;
			break;
		case LT300SYS_CTDATA_PHASEC_READ_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 20;
			break;

		case LT300SYS_CTDATA_LOAD_READ_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 12;
			break;

		case LT300SYS_CTDATA_TEMPERATURE_READ_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 2;
			break;
		case LT300SYS_CTDATA_READ_ALARM_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len    = sizeof(u32_t);
			break;

		case LT300SYS_CTDATA_READ_ALARM_MASK_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len    = sizeof(u32_t);
			break;

		default:
			LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no such object\n"));
			od->instance = MIB_OBJECT_NONE;
			break;
		}
	} else {
		LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no scalar\n"));
		od->instance = MIB_OBJECT_NONE;
	}

	return;
}

static void yjlt300sys_ctdata_read_entry_get_value(struct obj_def *od, u16_t len, void *value)
{
	u32_t *u32_ptr = (u32_t *)value;
	u8_t *pch = (u8_t *)value;
	//struct sinkinfo_pt_ct_st;
	struct collect_ct_data_info_st data;
	u32_t ctdata[32]={0};
	u8_t id,i;
	//u8_t ctindex;

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));
	
	id = (u8_t)od->id_inst_ptr[0];
	
	switch (id) {
	case LT300SYS_CTDATA_PHASEA_READ_ID: 
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, od->id_inst_ptr[3], SI_MGC_GET_CTC_PA_INFO, ctdata, SINKINFO_PTC_PX_DATA_SIZE)){
			for(i=0; i<SINKINFO_PTC_PX_DATA_SIZE/sizeof(u32_t); i++)
				*(u32_ptr+i) = lwip_htonl(ctdata[i]);
			
			//rt_memcpy(u32_ptr, ctdata, SINKINFO_PTC_PX_DATA_SIZE);
		}
		else{
			
			printf_syn("CT data  phaseA PARAM read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, od->id_inst_ptr[3], SI_MGC_GET_CTC_PA_INFO, ctdata, SINKINFO_PTC_PX_DATA_SIZE)){
				printf_syn("CT data phaseA voltage read failed!\r\n");
			}
			else{
				for(i=0; i<SINKINFO_PTC_PX_DATA_SIZE/sizeof(u32_t); i++)
					*(u32_ptr+i) = lwip_htonl(ctdata[i]);
			
				//rt_memcpy(u32_ptr, ctdata, SINKINFO_PTC_PX_DATA_SIZE);
			}
		}		
//		get_sinkinfo_abc_param(od->id_inst_ptr[2]-1, SIC_GET_CT_CUR, uint_ptr, uint_ptr+1, uint_ptr+2); /* mark by David */
		break;	
	case LT300SYS_CTDATA_PHASEB_READ_ID:	  
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, od->id_inst_ptr[3], SI_MGC_GET_CTC_PB_INFO, ctdata, SINKINFO_PTC_PX_DATA_SIZE)){
			for(i=0; i<SINKINFO_PTC_PX_DATA_SIZE/sizeof(u32_t); i++)
				*(u32_ptr+i) = lwip_htonl(ctdata[i]);
			
			//rt_memcpy(u32_ptr, ctdata, SINKINFO_PTC_PX_DATA_SIZE);
		}
		else{
			printf_syn("CT data  phaseB PARAM read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, od->id_inst_ptr[3], SI_MGC_GET_CTC_PB_INFO, ctdata, SINKINFO_PTC_PX_DATA_SIZE)){
				printf_syn("CT data phaseB voltage read failed!\r\n");
			}
			else{
				for(i=0; i<SINKINFO_PTC_PX_DATA_SIZE/sizeof(u32_t); i++)
					*(u32_ptr+i) = lwip_htonl(ctdata[i]);
			
				//rt_memcpy(u32_ptr, ctdata, SINKINFO_PTC_PX_DATA_SIZE);
			}
		}	

		break;	
	case LT300SYS_CTDATA_PHASEC_READ_ID:	  
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, od->id_inst_ptr[3], SI_MGC_GET_CTC_PC_INFO, ctdata, SINKINFO_PTC_PX_DATA_SIZE)){
			for(i=0; i<SINKINFO_PTC_PX_DATA_SIZE/sizeof(u32_t); i++)
				*(u32_ptr+i) = lwip_htonl(ctdata[i]);
			
			//rt_memcpy(u32_ptr, ctdata, SINKINFO_PTC_PX_DATA_SIZE);
		}
		else{
			printf_syn("CT data  phaseC PARAM read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1, od->id_inst_ptr[3], SI_MGC_GET_CTC_PC_INFO, ctdata, SINKINFO_PTC_PX_DATA_SIZE)){
				printf_syn("CT data phaseC voltage read failed!\r\n");
			}
			else{
				for(i=0; i<SINKINFO_PTC_PX_DATA_SIZE/sizeof(u32_t); i++)
					*(u32_ptr+i) = lwip_htonl(ctdata[i]);
			
				//rt_memcpy(u32_ptr, ctdata, SINKINFO_PTC_PX_DATA_SIZE);
			}
		}			
		break;	

	case LT300SYS_CTDATA_LOAD_READ_ID:
		if(SIE_OK == get_sinkinfo_abc_param(od->id_inst_ptr[2]-1, od->id_inst_ptr[3], SIC_GET_CT_LOAD, ctdata, ctdata+1, ctdata+2)){
			
			*(u32_ptr) = lwip_htonl(ctdata[0]);
			*(u32_ptr+1) = lwip_htonl(ctdata[1]);  
			*(u32_ptr+2) = lwip_htonl(ctdata[2]);
			
			//rt_memcpy(u32_ptr, ctdata, sizeof(u32_t)*3);
		}
		else{
			//LWIP_DEBUGF(SNMP_MIB_DEBUG,("line_data_read_voltage: errer, please read again!\n"));
			printf_syn("PT data  phaseA voltage read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_abc_param(od->id_inst_ptr[2]-1, od->id_inst_ptr[3], SIC_GET_CT_LOAD, ctdata, ctdata+1, ctdata+2)){
 				printf_syn("PT data phaseA voltage read failed!\r\n");
			}else{
				*(u32_ptr) = lwip_htonl(ctdata[0]);
				*(u32_ptr+1) = lwip_htonl(ctdata[1]);  
				*(u32_ptr+2) = lwip_htonl(ctdata[2]);
			
				//rt_memcpy(u32_ptr, ctdata, sizeof(u32_t)*3);
			}
			
		}
		rt_memcpy(&data.CtData.CtActPowA, u32_ptr, sizeof(data.CtData.CtActPowA));
		rt_memcpy(&data.CtData.CtActPowB, u32_ptr+1, sizeof(data.CtData.CtActPowB));
		rt_memcpy(&data.CtData.CtActPowC, u32_ptr+2, sizeof(data.CtData.CtActPowC));
		break;

	case LT300SYS_CTDATA_TEMPERATURE_READ_ID:	  
		*u32_ptr = lwip_htons(INVLIDE_DATAS);
		break;

	case LT300SYS_CTDATA_READ_ALARM_ID: /* mark by David */
		*pch = data.ctcur_alarm;
		break;

	case LT300SYS_CTDATA_READ_ALARM_MASK_ID:
		*pch = data.ctalarm_mask;
		break;
		
	default:
		break;
	}
//	write_syscfgdata_tbl(SYSCFGDATA_TBL_CT_DATA, 0, &data);  //hongbin E
	return;
}

static u8_t yjlt300sys_ctdata_read_entry_set_test(struct obj_def *od, u16_t len, void *value)
{
	u8_t id, set_ok;

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	set_ok = 0;
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_CTDATA_PHASEA_READ_ID:
	case LT300SYS_CTDATA_LOAD_READ_ID:
	case LT300SYS_CTDATA_READ_ALARM_MASK_ID:	
#if 0
		if ((0==*sint_ptr) || (1==*sint_ptr))
			set_ok = 1;
#else
		set_ok = 1;
#endif
		break;

	default:
		break;
	}

	return set_ok;
}

static void yjlt300sys_ctdata_read_entry_set_value(struct obj_def *od, u16_t len, void *value)
{
//	u32_t *uint_ptr =(u32_t *)value;
//	struct collect_ct_data_info_st data;
	u8_t id;
    
	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

//	read_syscfgdata_tbl(SYSCFGDATA_TBL_CT_DATA, 0, &data);

	/* @todo @fixme: which kind of pointer is 'value'? s32_t or u8_t??? */
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_CTDATA_PHASEA_READ_ID:	
	case LT300SYS_CTDATA_LOAD_READ_ID:	
		break;
		
	case LT300SYS_CTDATA_READ_ALARM_MASK_ID:
//		data.ctalarm_mask = *uint_ptr;
		break;

	default:
		break;
	}

//	write_syscfgdata_tbl(SYSCFGDATA_TBL_CT_DATA, 0, &data);

	return;
}

/*
 * data analysis
 */
static void yjlt300sys_data_analysis_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
{
	/* return to object name, adding index depth (1) */
	ident_len += 2;
	ident     -= 2;

	if (ident_len == 3) {
		u8_t id;

		od->id_inst_len = ident_len;
		od->id_inst_ptr = ident;

		LWIP_ASSERT("invalid id", (ident[0] >= 0) && (ident[0] <= 0xff));

		id = (u8_t)ident[0];
		od->instance = MIB_OBJECT_TAB;
		switch (id) {
		case LT300SYS_PHASEA_DEV_ID:	
		case LT300SYS_PHASEB_DEV_ID:
		case LT300SYS_PHASEC_DEV_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 20;
			break;
		case LT300SYS_TOTAL_DEV_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 8;
			break;

		default:
			LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no such object\n"));
			od->instance = MIB_OBJECT_NONE;
			break;
		}
	} else {
		LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no scalar\n"));
		od->instance = MIB_OBJECT_NONE;
	}

	return;
}

static void yjlt300sys_data_analysis_entry_get_value(struct obj_def *od, u16_t len, void *value)
{
	u32_t *uint_ptr = (u32_t *)value;
	//struct data_analysis_info_st analysis;
	u8_t id;
	u32_t analysisdata[12];
	struct sinkinfo_em_dev_st em_ptr;
	rt_memset(analysisdata, 0, sizeof(analysisdata));

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

//	read_syscfgdata_tbl(SYSCFGDATA_TBL_DATA_ANALYSIS, 0, &analysis);
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_PHASEA_DEV_ID:
//		get_sinkinfo_other_param(od->id_inst_ptr[2], SIC_GET_PHASEA_INACCURACY, uint_ptr); /* mark by Daivd */
		/*rt_memcpy(&analysis.phaseA_dev.VolDev, uint_ptr, sizeof(analysis.phaseA_dev.VolDev));
		rt_memcpy(&analysis.phaseA_dev.CurDev, uint_ptr+=1, sizeof(analysis.phaseA_dev.CurDev));
		rt_memcpy(&analysis.phaseA_dev.ActPowDev, uint_ptr+=1, sizeof(analysis.phaseA_dev.ActPowTDev));
		rt_memcpy(&analysis.phaseA_dev.ReActPowDev, uint_ptr+=1, sizeof(analysis.phaseA_dev.ReActPowDev));
		rt_memcpy(&analysis.phaseA_dev.PowFacDev, uint_ptr+=1, sizeof(analysis.phaseA_dev.PowFacDev));*/
		break;	
	case LT300SYS_PHASEB_DEV_ID:
		/*get_sinkinfo_other_param(od->id_inst_ptr[2], SIC_GET_PHASEB_INACCURACY, uint_ptr);
		rt_memcpy(&analysis.phaseB_dev.VolDev, uint_ptr, sizeof(analysis.phaseB_dev.VolDev));
		rt_memcpy(&analysis.phaseB_dev.CurDev, uint_ptr+=1, sizeof(analysis.phaseB_dev.CurDev));
		rt_memcpy(&analysis.phaseB_dev.ActPowDev, uint_ptr+=1, sizeof(analysis.phaseB_dev.ActPowTDev));
		rt_memcpy(&analysis.phaseB_dev.ReActPowDev, uint_ptr+=1, sizeof(analysis.phaseB_dev.ReActPowDev));
		rt_memcpy(&analysis.phaseB_dev.PowFacDev, uint_ptr+=1, sizeof(analysis.phaseB_dev.PowFacDev));*/
		break;

	case LT300SYS_PHASEC_DEV_ID:
		/*get_sinkinfo_other_param(od->id_inst_ptr[2], SIC_GET_PHASEB_INACCURACY, uint_ptr);
		rt_memcpy(&analysis.phaseC_dev.VolDev, uint_ptr, sizeof(analysis.phaseC_dev.VolDev));
		rt_memcpy(&analysis.phaseC_dev.CurDev, uint_ptr+=1, sizeof(analysis.phaseC_dev.CurDev));
		rt_memcpy(&analysis.phaseC_dev.ActPowDev, uint_ptr+=1, sizeof(analysis.phaseC_dev.ActPowTDev));
		rt_memcpy(&analysis.phaseC_dev.ReActPowDev, uint_ptr+=1, sizeof(analysis.phaseC_dev.ReActPowDev));
		rt_memcpy(&analysis.phaseC_dev.PowFacDev, uint_ptr+=1, sizeof(analysis.phaseC_dev.PowFacDev));*/
		break;

	case LT300SYS_TOTAL_DEV_ID: /* mark by David */
#if 0
		if(SIE_OK == get_sinkinfo_other_param(od->id_inst_ptr[2], SIC_GET_ACT_INACCURACY, uint_ptr)){
			lwip_htonl(*uint_ptr);
		}
		else{
			printf_syn("real total active power inaccuracy read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_other_param(od->id_inst_ptr[2], SIC_GET_ACT_INACCURACY, uint_ptr)){
 				printf_syn("real total active power inaccuracy read failed!\r\n");
			}else{
				lwip_htonl(*uint_ptr);
			}
			
		}
		if(SIE_OK == get_sinkinfo_other_param(od->id_inst_ptr[2], SIC_GET_REACT_INACCURACY, uint_ptr+1)){
			lwip_htonl(*(uint_ptr+1));
		}
		else{
			printf_syn("real total reactive power inaccuracy read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_other_param(od->id_inst_ptr[2], SIC_GET_REACT_INACCURACY, uint_ptr+1)){
 				printf_syn("real total reactive power inaccuracy read failed!\r\n");
			}else{
				lwip_htonl(*(uint_ptr+1));
			}
			
		}
#endif
		if(SIE_OK == get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_DEV_INFO, &em_ptr, SINKINFO_EM_DEV_DATA_SIZE)){
			analysisdata[0] = lwip_htonl((u32_t)em_ptr.em_act_ee_inaccuracy);
			analysisdata[1] = lwip_htonl((u32_t)em_ptr.em_react_ee_inaccuracy);
			rt_memcpy(uint_ptr, analysisdata, sizeof(u32_t)*2);
		}
		else{
			printf_syn("em data  phaseC PARAM read failed!try read again!\r\n");
			if(SIE_OK != get_sinkinfo_use_by_mib(od->id_inst_ptr[2]-1,  0, SI_MGC_GET_EM_DEV_INFO, &em_ptr, SINKINFO_EM_DEV_DATA_SIZE)){
 				printf_syn("em data phaseC voltage read failed!\r\n");
			}
			else{
				analysisdata[0] = lwip_htonl((u32_t)em_ptr.em_act_ee_inaccuracy);
				analysisdata[1] = lwip_htonl((u32_t)em_ptr.em_react_ee_inaccuracy);
				rt_memcpy(uint_ptr, analysisdata, sizeof(u32_t)*2);
			}
		}
		break;
		
	default:
		break;
	}
//	write_syscfgdata_tbl(SYSCFGDATA_TBL_DATA_ANALYSIS, 0, &analysis);  //hongbin E
	return;
}

static u8_t yjlt300sys_data_analysis_entry_set_test(struct obj_def *od, u16_t len, void *value)
{
	u8_t id, set_ok;

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	set_ok = 0;
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_PHASEA_DEV_ID:
	case LT300SYS_PHASEB_DEV_ID:
	case LT300SYS_PHASEC_DEV_ID:	
	case LT300SYS_TOTAL_DEV_ID:	
#if 0
		if ((0==*sint_ptr) || (1==*sint_ptr))
			set_ok = 1;
#else
		set_ok = 1;
#endif
		break;

	default:
		break;
	}

	return set_ok;
}

static void yjlt300sys_data_analysis_entry_set_value(struct obj_def *od, u16_t len, void *value)
{
	//u32_t *uint_ptr = (u32_t *)value;
	//struct data_analysis_info_st analysis;
	u8_t id;
    
	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

//	read_syscfgdata_tbl(SYSCFGDATA_TBL_DATA_ANALYSIS, 0, &analysis);

	/* @todo @fixme: which kind of pointer is 'value'? s32_t or u8_t??? */
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_PHASEA_DEV_ID:	
	case LT300SYS_PHASEB_DEV_ID:	
	case LT300SYS_PHASEC_DEV_ID:	
	case LT300SYS_TOTAL_DEV_ID:
		break;

	default:
		break;
	}

//	write_syscfgdata_tbl(SYSCFGDATA_TBL_DATA_ANALYSIS, 0, &analysis);

	return;
}

/* 
 * deviation exceed configure
 */
static void yjlt300sys_dev_thd_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
{
	/* return to object name, adding index depth (1) */
	ident_len += 2;
	ident     -= 2;

	if (ident_len == 3) {
		u8_t id;

		od->id_inst_len = ident_len;
		od->id_inst_ptr = ident;

		LWIP_ASSERT("invalid id", (ident[0] >= 0) && (ident[0] <= 0xff));

		id = (u8_t)ident[0];
		od->instance = MIB_OBJECT_TAB;
		switch (id) {
		case LT300SYS_PHASEA_DEV_THD_ID:	
		case LT300SYS_PHASEB_DEV_THD_ID:
		case LT300SYS_PHASEC_DEV_THD_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 20;
			break;
		case LT300SYS_TOTAL_DEV_THD_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 24;
			break;

		default:
			LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no such object\n"));
			od->instance = MIB_OBJECT_NONE;
			break;
		}
	} else {
		LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no scalar\n"));
		od->instance = MIB_OBJECT_NONE;
	}

	return;
}

static void yjlt300sys_dev_thd_entry_get_value(struct obj_def *od, u16_t len, void *value)
{
	u32_t *uint_ptr = (u32_t *)value;
	struct data_analysis_info_st devthd;
	u8_t id;

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

//	read_syscfgdata_tbl(SYSCFGDATA_TBL_DATA_ANALYSIS, 0, &devthd);
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_PHASEA_DEV_THD_ID:
//		get_sinkinfo_devthd_param(SIC_SET_PHASEA_THD, uint_ptr);
		rt_memcpy(uint_ptr, &devthd.phaseA_datathd.VolDevThd, sizeof(devthd.phaseA_datathd.VolDevThd));
		rt_memcpy(uint_ptr+1, &devthd.phaseA_datathd.CurDevThd, sizeof(devthd.phaseA_datathd.CurDevThd));
		rt_memcpy(uint_ptr+2, &devthd.phaseA_datathd.ActPowDevThd, sizeof(devthd.phaseA_datathd.ActPowDevThd));
		rt_memcpy(uint_ptr+3, &devthd.phaseA_datathd.ReActPowDevThd, sizeof(devthd.phaseA_datathd.ReActPowDevThd));
		rt_memcpy(uint_ptr+4, &devthd.phaseA_datathd.PowFacDevThd, sizeof(devthd.phaseA_datathd.PowFacDevThd));
		break;	
	case LT300SYS_PHASEB_DEV_THD_ID:
//		get_sinkinfo_devthd_param(SIC_SET_PHASEB_THD, uint_ptr);
		rt_memcpy(uint_ptr, &devthd.phaseB_datathd.VolDevThd, sizeof(devthd.phaseB_datathd.VolDevThd));
		rt_memcpy(uint_ptr+1, &devthd.phaseB_datathd.CurDevThd, sizeof(devthd.phaseB_datathd.CurDevThd));
		rt_memcpy(uint_ptr+2, &devthd.phaseB_datathd.ActPowDevThd, sizeof(devthd.phaseB_datathd.ActPowDevThd));
		rt_memcpy(uint_ptr+3, &devthd.phaseB_datathd.ReActPowDevThd, sizeof(devthd.phaseB_datathd.ReActPowDevThd));
		rt_memcpy(uint_ptr+4, &devthd.phaseB_datathd.PowFacDevThd, sizeof(devthd.phaseB_datathd.PowFacDevThd));
		break;

	case LT300SYS_PHASEC_DEV_THD_ID:
//		get_sinkinfo_devthd_param(SIC_SET_PHASEC_THD, uint_ptr);
		rt_memcpy(uint_ptr, &devthd.phaseC_dev.VolDev, sizeof(devthd.phaseC_dev.VolDev));
		rt_memcpy(uint_ptr+1, &devthd.phaseC_dev.CurDev, sizeof(devthd.phaseC_dev.CurDev));
		rt_memcpy(uint_ptr+2, &devthd.phaseC_dev.ActPowDev, sizeof(devthd.phaseC_dev.ActPowTDev));
		rt_memcpy(uint_ptr+3, &devthd.phaseC_dev.ReActPowDev, sizeof(devthd.phaseC_dev.ReActPowDev));
		rt_memcpy(uint_ptr+4, &devthd.phaseC_dev.PowFacDev, sizeof(devthd.phaseC_dev.PowFacDev));
		break;

	case LT300SYS_TOTAL_DEV_THD_ID: /* mark by David */
//		get_sinkinfo_devthd_param(SIC_SET_TOTAL_THD, uint_ptr);
		rt_memcpy(uint_ptr, &devthd.total_datathd.ActTotalDevThd, sizeof(devthd.total_datathd.ActTotalDevThd));
		rt_memcpy(uint_ptr+1, &devthd.total_datathd.ReActTotalDevThd, sizeof(devthd.total_datathd.ReActTotalDevThd));
		rt_memcpy(uint_ptr+2, &devthd.total_datathd.ActPowTDevThd, sizeof(devthd.total_datathd.ActPowTDevThd));
		rt_memcpy(uint_ptr+3, &devthd.total_datathd.ReActPowTDevThd, sizeof(devthd.total_datathd.ReActPowTDevThd));
		rt_memcpy(uint_ptr+4, &devthd.total_datathd.PowFacTDevThd, sizeof(devthd.total_datathd.PowFacTDevThd));
		rt_memcpy(uint_ptr+5, &devthd.total_datathd.MonTotalDevThd, sizeof(devthd.total_datathd.MonTotalDevThd));
		break;
		
	default:
		break;
	}
//	write_syscfgdata_tbl(SYSCFGDATA_TBL_DATA_ANALYSIS, 0, &devthd);  //hongbin E
	return;
}

static u8_t yjlt300sys_dev_thd_entry_set_test(struct obj_def *od, u16_t len, void *value)
{
	u8_t id, set_ok;

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	set_ok = 0;
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_PHASEA_DEV_THD_ID:
	case LT300SYS_PHASEB_DEV_THD_ID:
	case LT300SYS_PHASEC_DEV_THD_ID:	
	case LT300SYS_TOTAL_DEV_THD_ID:	
#if 0
		if ((0==*sint_ptr) || (1==*sint_ptr))
			set_ok = 1;
#else
		set_ok = 1;
#endif
		break;

	default:
		break;
	}

	return set_ok;
}

static void yjlt300sys_dev_thd_entry_set_value(struct obj_def *od, u16_t len, void *value)
{
	u32_t *uint_ptr = (u32_t *)value;
	struct data_analysis_info_st devthd;
	u8_t id;
    
	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

//	read_syscfgdata_tbl(SYSCFGDATA_TBL_DATA_ANALYSIS, 0, &devthd);

	/* @todo @fixme: which kind of pointer is 'value'? s32_t or u8_t??? */
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_PHASEA_DEV_THD_ID:
//		set_sinkinfo_devthd_param(SIC_SET_PHASEA_THD, uint_ptr);
		rt_memcpy(&devthd.phaseA_datathd.VolDevThd, uint_ptr, sizeof(devthd.phaseA_datathd.VolDevThd));
		rt_memcpy(&devthd.phaseA_datathd.CurDevThd, uint_ptr+1, sizeof(devthd.phaseA_datathd.CurDevThd));
		rt_memcpy(&devthd.phaseA_datathd.ActPowDevThd, uint_ptr+2, sizeof(devthd.phaseA_datathd.ActPowDevThd));
		rt_memcpy(&devthd.phaseA_datathd.ReActPowDevThd, uint_ptr+3, sizeof(devthd.phaseA_datathd.ReActPowDevThd));
		rt_memcpy(&devthd.phaseA_datathd.PowFacDevThd, uint_ptr+4, sizeof(devthd.phaseA_datathd.PowFacDevThd));
		break;
	case LT300SYS_PHASEB_DEV_THD_ID:
//		set_sinkinfo_devthd_param(SIC_SET_PHASEB_THD, uint_ptr);
		rt_memcpy(&devthd.phaseB_datathd.VolDevThd, uint_ptr, sizeof(devthd.phaseB_datathd.VolDevThd));
		rt_memcpy(&devthd.phaseB_datathd.CurDevThd, uint_ptr+1, sizeof(devthd.phaseB_datathd.CurDevThd));
		rt_memcpy(&devthd.phaseB_datathd.ActPowDevThd, uint_ptr+2, sizeof(devthd.phaseB_datathd.ActPowDevThd));
		rt_memcpy(&devthd.phaseB_datathd.ReActPowDevThd, uint_ptr+3, sizeof(devthd.phaseB_datathd.ReActPowDevThd));
		rt_memcpy(&devthd.phaseB_datathd.PowFacDevThd, uint_ptr+4, sizeof(devthd.phaseB_datathd.PowFacDevThd));
		break;
	case LT300SYS_PHASEC_DEV_THD_ID:	
//		set_sinkinfo_devthd_param(SIC_SET_PHASEC_THD, uint_ptr);
		rt_memcpy(&devthd.phaseC_dev.VolDev, uint_ptr, sizeof(devthd.phaseC_dev.VolDev));
		rt_memcpy(&devthd.phaseC_dev.CurDev, uint_ptr+1, sizeof(devthd.phaseC_dev.CurDev));
		rt_memcpy(&devthd.phaseC_dev.ActPowDev, uint_ptr+2, sizeof(devthd.phaseC_dev.ActPowTDev));
		rt_memcpy(&devthd.phaseC_dev.ReActPowDev, uint_ptr+3, sizeof(devthd.phaseC_dev.ReActPowDev));
		rt_memcpy(&devthd.phaseC_dev.PowFacDev, uint_ptr+4, sizeof(devthd.phaseC_dev.PowFacDev));
		break;
	case LT300SYS_TOTAL_DEV_THD_ID:
//		set_sinkinfo_devthd_param(SIC_SET_TOTAL_THD, uint_ptr);
		rt_memcpy(&devthd.total_datathd.ActTotalDevThd, uint_ptr, sizeof(devthd.total_datathd.ActTotalDevThd));
		rt_memcpy(&devthd.total_datathd.ReActTotalDevThd, uint_ptr+1, sizeof(devthd.total_datathd.ReActTotalDevThd));
		rt_memcpy(&devthd.total_datathd.ActPowTDevThd, uint_ptr+2, sizeof(devthd.total_datathd.ActPowTDevThd));
		rt_memcpy(&devthd.total_datathd.ReActPowTDevThd, uint_ptr+3, sizeof(devthd.total_datathd.ReActPowTDevThd));
		rt_memcpy(&devthd.total_datathd.PowFacTDevThd, uint_ptr+4, sizeof(devthd.total_datathd.PowFacTDevThd));
		rt_memcpy(&devthd.total_datathd.MonTotalDevThd, uint_ptr+5, sizeof(devthd.total_datathd.MonTotalDevThd));
		break;

	default:
		break;
	}

//	write_syscfgdata_tbl(SYSCFGDATA_TBL_DATA_ANALYSIS, 0, &devthd);

	return;
}

/* 
 * threshold exceed alarm get
 */
static void yjlt300sys_thdexc_alarm_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
{
	/* return to object name, adding index depth (1) */
	ident_len += 2;
	ident     -= 2;

	if (ident_len == 3) {
		u8_t id;

		od->id_inst_len = ident_len;
		od->id_inst_ptr = ident;

		LWIP_ASSERT("invalid id", (ident[0] >= 0) && (ident[0] <= 0xff));

		id = (u8_t)ident[0];
		od->instance = MIB_OBJECT_TAB;
		switch (id) {
		case LT300SYS_PHASEA_THDEXC_ALARM_ID:	
		case LT300SYS_PHASEB_THDEXC_ALARM_ID:
		case LT300SYS_PHASEC_THDEXC_ALARM_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len    = 1;
			break;
		case LT300SYS_TOTAL_THDEXC_ALARM_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_GAUGE);
			od->v_len    = 1;
			break;

		default:
			LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no such object\n"));
			od->instance = MIB_OBJECT_NONE;
			break;
		}
	} else {
		LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no scalar\n"));
		od->instance = MIB_OBJECT_NONE;
	}

	return;
}

static void yjlt300sys_thdexc_alarm_entry_get_value(struct obj_def *od, u16_t len, void *value)
{
	//u32_t *uint_ptr = value;
	u8_t  *pch = (u8_t *)value;
	struct data_analysis_info_st excalarm;
	u8_t id;

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

//	read_syscfgdata_tbl(SYSCFGDATA_TBL_DATA_ANALYSIS, 0, &excalarm);
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_PHASEA_THDEXC_ALARM_ID:
//		get_sinkinfo_exc_alarm(SIC_GET_PHASEA_THD_EXC_ALARM, uint_ptr);
		ocstrncpy(pch, &(excalarm.thdexc_alarm[0]),sizeof(excalarm.thdexc_alarm[0]));
		break;	
	case LT300SYS_PHASEB_THDEXC_ALARM_ID:
		//get_sinkinfo_devthd_param(SIC_SET_PHASEB_THD, uint_ptr);
//		get_sinkinfo_exc_alarm(SIC_GET_PHASEB_THD_EXC_ALARM, uint_ptr);
		ocstrncpy(pch, &(excalarm.thdexc_alarm[1]),sizeof(excalarm.thdexc_alarm[1]));
		break;

	case LT300SYS_PHASEC_THDEXC_ALARM_ID:		
//		get_sinkinfo_exc_alarm(SIC_GET_PHASEC_THD_EXC_ALARM, uint_ptr);
		ocstrncpy(pch, &(excalarm.thdexc_alarm[2]),sizeof(excalarm.thdexc_alarm[2]));
		break;

	case LT300SYS_TOTAL_THDEXC_ALARM_ID: /* mark by David */
//		get_sinkinfo_devthd_param(SIC_GET_TOTAL_THD_EXC_ALARM, uint_ptr);
		ocstrncpy(pch, &(excalarm.thdexc_alarm[3]),sizeof(excalarm.thdexc_alarm[3]));		
		break;
		
	default:
		break;
	}
//	write_syscfgdata_tbl(SYSCFGDATA_TBL_DATA_ANALYSIS, 0, &excalarm);  //hongbin E
	return;
}

static u8_t yjlt300sys_thdexc_alarm_entry_set_test(struct obj_def *od, u16_t len, void *value)
{
	u8_t id, set_ok;

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	set_ok = 0;
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_PHASEA_THDEXC_ALARM_ID:
	case LT300SYS_PHASEB_THDEXC_ALARM_ID:
	case LT300SYS_PHASEC_THDEXC_ALARM_ID:	
	case LT300SYS_TOTAL_THDEXC_ALARM_ID:	
#if 0
		if ((0==*sint_ptr) || (1==*sint_ptr))
			set_ok = 1;
#else
		set_ok = 1;
#endif
		break;

	default:
		break;
	}

	return set_ok;
}

static void yjlt300sys_thdexc_alarm_entry_set_value(struct obj_def *od, u16_t len, void *value)
{
	//u32_t *uint_ptr = (u32_t *)value;
	//struct data_analysis_info_st devthd;
	u8_t id;
    
	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	//read_syscfgdata_tbl(SYSCFGDATA_TBL_DATA_ANALYSIS, 0, &devthd);

	/* @todo @fixme: which kind of pointer is 'value'? s32_t or u8_t??? */
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_PHASEA_THDEXC_ALARM_ID:	
	case LT300SYS_PHASEB_THDEXC_ALARM_ID:	
	case LT300SYS_PHASEC_THDEXC_ALARM_ID:		
	case LT300SYS_TOTAL_THDEXC_ALARM_ID:
		break;

	default:
		break;
	}

	//write_syscfgdata_tbl(SYSCFGDATA_TBL_DATA_ANALYSIS, 0, &devthd);

	return;
}

#if 0
/* 
 * trap node
 */
static void yjlt300_trap_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
{
	/* return to object name, adding index depth (1) */
	ident_len += 1;
	ident     -= 1;

	if (ident_len == 2) {
		u8_t id;

		od->id_inst_len = ident_len;
		od->id_inst_ptr = ident;

		LWIP_ASSERT("invalid id", (ident[0] >= 0) && (ident[0] <= 0xff));

		id = (u8_t)ident[0];
		od->instance = MIB_OBJECT_SCALAR;
		switch (id) {
		case LT300SYS_ETH_LINK_STATUS_CHANGE:	
		case LT300SYS_485_LINK_STATUS_CHANGE:
		case LT300SYS_DEV_TW_STATUS_CHANGE:
		case LT300SYS_PT_TW_STATUS_CHANGE:		
		case LT300SYS_CT_TW_STATUS_CHANGE:	
		case LT300SYS_TEMP_RH_OUTOF_GAUGE:
		case LT300SYS_LINE_DATA_READ_STATUS_CHANGE:
		case LT300SYS_METER_DATA_READ_STATUS_CHANGE:		
		case LT300SYS_PT_DATA_READ_STATUS_CHANGE:		
		case LT300SYS_CT_DATA_READ_STATUS_CHANGE:		
		case LT300SYS_PHASEA_THDEXC_ALARM_STATUS:	
		case LT300SYS_PHASEB_THDEXC_ALARM_STATUS:	
		case LT300SYS_PHASEC_THDEXC_ALARM_STATUS:
		case LT300SYS_TOTAL_THDEXC_ALARM_STATUS:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
			od->v_len    = 512;
			break;
		default:
			LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no such object\n"));
			od->instance = MIB_OBJECT_NONE;
			break;
		}
	} else {
		LWIP_DEBUGF(SNMP_MIB_DEBUG,("snmp_get_object_def: no scalar\n"));
		od->instance = MIB_OBJECT_NONE;
	}

	return;
}

static void yjlt300_trap_get_value(struct obj_def *od, u16_t len, void *value)
{
	//u32_t *uint_ptr = value;
	//struct data_analysis_info_st excalarm;
	u8_t id;

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_ETH_LINK_STATUS_CHANGE:	
	case LT300SYS_485_LINK_STATUS_CHANGE:
	case LT300SYS_DEV_TW_STATUS_CHANGE:
	case LT300SYS_PT_TW_STATUS_CHANGE:		
	case LT300SYS_CT_TW_STATUS_CHANGE:	
	case LT300SYS_TEMP_RH_OUTOF_GAUGE:
	case LT300SYS_LINE_DATA_READ_STATUS_CHANGE:
	case LT300SYS_METER_DATA_READ_STATUS_CHANGE:		
	case LT300SYS_PT_DATA_READ_STATUS_CHANGE:		
	case LT300SYS_CT_DATA_READ_STATUS_CHANGE:		
	case LT300SYS_PHASEA_THDEXC_ALARM_STATUS:	
	case LT300SYS_PHASEB_THDEXC_ALARM_STATUS:	
	case LT300SYS_PHASEC_THDEXC_ALARM_STATUS:
	case LT300SYS_TOTAL_THDEXC_ALARM_STATUS:	
		
	default:
		break;
	}
   
	return;
}

static u8_t yjlt300_trap_set_test(struct obj_def *od, u16_t len, void *value)
{
	u8_t id, set_ok;

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	set_ok = 0;
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_ETH_LINK_STATUS_CHANGE:	
	case LT300SYS_485_LINK_STATUS_CHANGE:
	case LT300SYS_DEV_TW_STATUS_CHANGE:
	case LT300SYS_PT_TW_STATUS_CHANGE:		
	case LT300SYS_CT_TW_STATUS_CHANGE:	
	case LT300SYS_TEMP_RH_OUTOF_GAUGE:
	case LT300SYS_LINE_DATA_READ_STATUS_CHANGE:
	case LT300SYS_METER_DATA_READ_STATUS_CHANGE:		
	case LT300SYS_PT_DATA_READ_STATUS_CHANGE:		
	case LT300SYS_CT_DATA_READ_STATUS_CHANGE:		
	case LT300SYS_PHASEA_THDEXC_ALARM_STATUS:	
	case LT300SYS_PHASEB_THDEXC_ALARM_STATUS:	
	case LT300SYS_PHASEC_THDEXC_ALARM_STATUS:
	case LT300SYS_TOTAL_THDEXC_ALARM_STATUS:	
#if 0
		if ((0==*sint_ptr) || (1==*sint_ptr))
			set_ok = 1;
#else
		set_ok = 1;
#endif
		break;

	default:
		break;
	}

	return set_ok;
}

static void yjlt300_trap_set_value(struct obj_def *od, u16_t len, void *value)
{
	//u32_t *uint_ptr = value;
	//struct data_analysis_info_st devthd;
	u8_t id;
    
	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	//read_syscfgdata_tbl(SYSCFGDATA_TBL_DATA_ANALYSIS, 0, &devthd);

	/* @todo @fixme: which kind of pointer is 'value'? s32_t or u8_t??? */
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case LT300SYS_ETH_LINK_STATUS_CHANGE:	
	case LT300SYS_485_LINK_STATUS_CHANGE:
	case LT300SYS_DEV_TW_STATUS_CHANGE:
	case LT300SYS_PT_TW_STATUS_CHANGE:		
	case LT300SYS_CT_TW_STATUS_CHANGE:	
	case LT300SYS_TEMP_RH_OUTOF_GAUGE:
	case LT300SYS_LINE_DATA_READ_STATUS_CHANGE:
	case LT300SYS_METER_DATA_READ_STATUS_CHANGE:		
	case LT300SYS_PT_DATA_READ_STATUS_CHANGE:		
	case LT300SYS_CT_DATA_READ_STATUS_CHANGE:		
	case LT300SYS_PHASEA_THDEXC_ALARM_STATUS:	
	case LT300SYS_PHASEB_THDEXC_ALARM_STATUS:	
	case LT300SYS_PHASEC_THDEXC_ALARM_STATUS:
	case LT300SYS_TOTAL_THDEXC_ALARM_STATUS:	
		break;

	default:
		break;
	}

	//write_syscfgdata_tbl(SYSCFGDATA_TBL_DATA_ANALYSIS, 0, &devthd);

	return;
}
#endif
/** enterprise ID for generic TRAPs, .1.3.6.1.4.1.40409 */
static struct snmp_obj_id yeejoin_id = {7,{1,3,6,1,4,1,YEEJOIN_ENTERPRISE_ID}};
#if 0
void snmp_get_yeejoinid_ptr(struct snmp_obj_id **oid)
{
	*oid = &yeejoin_id;
}
#endif

void snmp_yeejoin_trap(enum YEEJOIN_TRAP_ID_E trapid)
{
	/* !!NOTE: 这是不可重入函数, 需要加信号量做保护 */
	trap_msg.outvb.head  = NULL;
	trap_msg.outvb.tail  = NULL;
	trap_msg.outvb.count = 0;
	if(ERR_OK == snmp_send_trap(SNMP_GENTRAP_ENTERPRISESPC, &yeejoin_id, trapid))
		printf_syn("yeejoin trap send success\n");
}

void rs485_open_trap(void)
{
	 snmp_yeejoin_trap(YTI_485_LINK_STATUS_CHANGE);
		
}
FINSH_FUNCTION_EXPORT(rs485_open_trap, "rs485 trap send test");

void rs485_close_trap(void)
{
	snmp_yeejoin_trap(YTI_485_LINK_STATUS_CHANGE);
}


#endif /* LWIP_SNMP */
