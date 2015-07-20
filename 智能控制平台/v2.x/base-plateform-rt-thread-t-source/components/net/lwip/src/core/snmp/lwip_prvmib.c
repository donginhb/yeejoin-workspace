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

#include <lwip/private_mib.h>

#if LWIP_SNMP

#include <lwip/snmp_msg.h>
#include <lwip/snmp_asn1.h>

#include <board.h>
#include <am2301.h>
#include <lwip/snmp_msg.h>
#include <syscfgdata.h>

#include <lwip/opt.h>

#define PRIVMIB_DEBUG(x) //printf_syn x

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

static void yj_m3_if_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od);
static void yj_m3_if_get_value(struct obj_def *od, u16_t len, void *value);
static u8_t yj_m3_if_set_test(struct obj_def *od, u16_t len, void *value);
static void yj_m3_if_set_value(struct obj_def *od, u16_t len, void *value);

static void yjm3sys_netcfg_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od);
static void yjm3sys_netcfg_entry_get_value(struct obj_def *od, u16_t len, void *value);
static u8_t yjm3sys_netcfg_entry_set_test(struct obj_def *od, u16_t len, void *value);
static void yjm3sys_netcfg_entry_set_value(struct obj_def *od, u16_t len, void *value);

static void yjm3sys_mcfg_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od);
static void yjm3sys_mcfg_entry_get_value(struct obj_def *od, u16_t len, void *value);
static u8_t yjm3sys_mcfg_entry_set_test(struct obj_def *od, u16_t len, void *value);
static void yjm3sys_mcfg_entry_set_value(struct obj_def *od, u16_t len, void *value);

static void yjm3sys_trapcfg_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od);
static void yjm3sys_trapcfg_entry_get_value(struct obj_def *od, u16_t len, void *value);
static u8_t yjm3sys_trapcfg_entry_set_test(struct obj_def *od, u16_t len, void *value);
static void yjm3sys_trapcfg_entry_set_value(struct obj_def *od, u16_t len, void *value);

static void yjm3sys_envpcfg_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od);
static void yjm3sys_envpcfg_entry_get_value(struct obj_def *od, u16_t len, void *value);
static u8_t yjm3sys_envpcfg_entry_set_test(struct obj_def *od, u16_t len, void *value);
static void yjm3sys_envpcfg_entry_set_value(struct obj_def *od, u16_t len, void *value);

static void yjm3sys_eponcfg_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od);
static void yjm3sys_eponcfg_entry_get_value(struct obj_def *od, u16_t len, void *value);
static u8_t yjm3sys_eponcfg_entry_set_test(struct obj_def *od, u16_t len, void *value);
static void yjm3sys_eponcfg_entry_set_value(struct obj_def *od, u16_t len, void *value);


/* lwip .1.3.6.1.4.1.26381 */


/* yjm3sys_mcfg_entry .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.1.1.1.1 */
/*
 * M3SYS_MCFG_DEV_INFO
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
enum yjm3sys_mcfg_entry_leaf_id_e {
	M3SYS_MCFG_DEV_SN_ID	= 1,	/* The SN number of the device. */
	M3SYS_MCFG_SYS_DESCR_ID,	/* the Descr of yeeJOINSC device System */
	M3SYS_MCFG_DEV_INFO_ID,		/*  */
	M3SYS_MCFG_ALARM_STATUS_ID,
	M3SYS_MCFG_NE_ID_ID,		/* NetId */
	M3SYS_MCFG_SYS_DEFAULT_ID,
	M3SYS_MCFG_SYS_REBOOT_ID,
};
static const mib_scalar_node yjm3sys_mcfg_entry_scalar = {
	&yjm3sys_mcfg_entry_get_object_def,
	&yjm3sys_mcfg_entry_get_value,
	&yjm3sys_mcfg_entry_set_test,
	&yjm3sys_mcfg_entry_set_value,
	MIB_NODE_SC,
	0
};
static const s32_t yjm3sys_mcfg_entry_ids[7] = {1, 2, 3, 4, 5, 6, 7};
static struct mib_node* const yjm3sys_mcfg_entry_nodes[7] = {
	(struct mib_node*)&yjm3sys_mcfg_entry_scalar, (struct mib_node*)&yjm3sys_mcfg_entry_scalar,
	(struct mib_node*)&yjm3sys_mcfg_entry_scalar, (struct mib_node*)&yjm3sys_mcfg_entry_scalar, 
	(struct mib_node*)&yjm3sys_mcfg_entry_scalar, (struct mib_node*)&yjm3sys_mcfg_entry_scalar, 
	(struct mib_node*)&yjm3sys_mcfg_entry_scalar,
};
static const struct mib_array_node yjm3sys_mcfg_entry = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	7,
	yjm3sys_mcfg_entry_ids,
	yjm3sys_mcfg_entry_nodes
};
/** yjm3sys_mcfg_tbl(system manage table)   .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.1.1.1 */
static const s32_t yjm3sys_mcfg_tbl_ids[1] = {1};
static struct mib_node* const yjm3sys_mcfg_tbl_nodes[1] = { 
	(struct mib_node* const)&yjm3sys_mcfg_entry,
};
static const struct mib_array_node yjm3sys_mcfg_tbl = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	1,
	yjm3sys_mcfg_tbl_ids,
	yjm3sys_mcfg_tbl_nodes
};


/* yjm3sys_netcfg_entry .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.1.1.2.1 */
enum yjm3sys_netcfg_entry_leaf_id_e {
	M3SYS_NETCFG_LOCAL_IP_ID = 1,
	M3SYS_NETCFG_LOCAL_SUBNET_ID,
	M3SYS_NETCFG_LOCAL_GW_ID,
	M3SYS_NETCFG_LOCAL_MAC_ID,
	M3SYS_NETCFG_READ_COMMUNITY_ID,
	M3SYS_NETCFG_WRITE_COMMUNITY_ID,
};
static const mib_scalar_node yjm3sys_netcfg_entry_scalar = {
	&yjm3sys_netcfg_entry_get_object_def,
	&yjm3sys_netcfg_entry_get_value,
	&yjm3sys_netcfg_entry_set_test,
	&yjm3sys_netcfg_entry_set_value,
	MIB_NODE_SC,
	0
};
static const s32_t yjm3sys_netcfg_entry_ids[6] = {1, 2, 3, 4, 5, 6};
static struct mib_node* const yjm3sys_netcfg_entry_nodes[6] = {
	(struct mib_node*)&yjm3sys_netcfg_entry_scalar, (struct mib_node*)&yjm3sys_netcfg_entry_scalar,
	(struct mib_node*)&yjm3sys_netcfg_entry_scalar, (struct mib_node*)&yjm3sys_netcfg_entry_scalar, 
	(struct mib_node*)&yjm3sys_netcfg_entry_scalar, (struct mib_node*)&yjm3sys_netcfg_entry_scalar, 
};
static const struct mib_array_node yjm3sys_netcfg_entry = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	6,
	yjm3sys_netcfg_entry_ids,
	yjm3sys_netcfg_entry_nodes
};
/** yjm3sys_netcfg_tbl   .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.1.1.2 */
static const s32_t yjm3sys_netcfg_tbl_ids[1] = {1};
static struct mib_node* const yjm3sys_netcfg_tbl_nodes[1] = { 
	(struct mib_node* const)&yjm3sys_netcfg_entry,
};
static const struct mib_array_node yjm3sys_netcfg_tbl = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	1,
	yjm3sys_netcfg_tbl_ids,
	yjm3sys_netcfg_tbl_nodes
};

/* yjm3sys_trapcfg_entry .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.1.1.3.1 */
/*
 * trap类型
 * 0 coldStart
 * 1 warmStart
 * 2 linkDown
 * 3 linUp
 * 4 authenticationFailure
 * 5 egpNeighborLoss
 * 6 enterpriseSpecific
 * yjm3sys_trapcfg只提供对6类型的配置
 */
enum yjm3sys_trapcfg_entry_leaf_id_e {
	M3SYS_TRAPCFG_TRAP_TYPE_ID = 1,	/* 0-6 */
	M3SYS_TRAPCFG_ADDR_ID,
	M3SYS_TRAPCFG_SNMP_VER_ID,	/* 0:V1 ; 1:V2C */
	M3SYS_TRAPCFG_TARGET_PORT_ID,	/* default:162 */
	M3SYS_TRAPCFG_COMMUNITY_ID,	/* default:public */
	M3SYS_TRAPCFG_VALID_ID,		/* 1:valid ; 0 : invalid */
};
static const mib_scalar_node yjm3sys_trapcfg_entry_scalar = {
	&yjm3sys_trapcfg_entry_get_object_def,
	&yjm3sys_trapcfg_entry_get_value,
	&yjm3sys_trapcfg_entry_set_test,
	&yjm3sys_trapcfg_entry_set_value,
	MIB_NODE_SC,
	0
};
static const s32_t yjm3sys_trapcfg_entry_ids[6] = {1, 2, 3, 4, 5, 6};
static struct mib_node* const yjm3sys_trapcfg_entry_nodes[6] = {
	(struct mib_node*)&yjm3sys_trapcfg_entry_scalar, (struct mib_node*)&yjm3sys_trapcfg_entry_scalar,
	(struct mib_node*)&yjm3sys_trapcfg_entry_scalar, (struct mib_node*)&yjm3sys_trapcfg_entry_scalar, 
	(struct mib_node*)&yjm3sys_trapcfg_entry_scalar, (struct mib_node*)&yjm3sys_trapcfg_entry_scalar, 
};
static const struct mib_array_node yjm3sys_trapcfg_entry = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	6,
	yjm3sys_trapcfg_entry_ids,
	yjm3sys_trapcfg_entry_nodes
};
/** yjm3sys_trapcfg_tbl   .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.1.1.3 */
static const s32_t yjm3sys_trapcfg_tbl_ids[1] = {1};
static struct mib_node* const yjm3sys_trapcfg_tbl_nodes[1] = { 
	(struct mib_node* const)&yjm3sys_trapcfg_entry,
};
static const struct mib_array_node yjm3sys_trapcfg_tbl = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	1,
	yjm3sys_trapcfg_tbl_ids,
	yjm3sys_trapcfg_tbl_nodes
};

/**
 * yj_m3_sys .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.1.1
 */
static const s32_t yj_m3_sys_ids[3] = {1, 2, 3};
static struct mib_node* const yj_m3_sys_nodes[3] = { 
	(struct mib_node* const)&yjm3sys_mcfg_tbl,
	(struct mib_node* const)&yjm3sys_netcfg_tbl,
	(struct mib_node* const)&yjm3sys_trapcfg_tbl,
};
static const struct mib_array_node yj_m3_sys = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	3,
	yj_m3_sys_ids,
	yj_m3_sys_nodes
};



/* yj_m3_if_leaf  .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.1.2.x */
enum yj_m3_if_leaf_id_e {
	SCB_ELOCK_CTRL_ID	= 1,	/* 1: closed, 0: opened */
	SCB_ELOCK_DELAY_AUTO_LOCK_ID,   /* 1~30min (s), 如果不在这个范围内, 就禁止自动关锁 */
	SCB_DOOR_STATE_ID,		/* 1: closed, 0: opened */
};
static const mib_scalar_node yj_m3_if_leaf_scalar = {
	&yj_m3_if_get_object_def,
	&yj_m3_if_get_value,
	&yj_m3_if_set_test,
	&yj_m3_if_set_value,
	MIB_NODE_SC,
	0
};

/* yjm3sys_envpcfg_entry(environment parameter cfg) .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.1.2.3.1 */
enum yjm3sys_envpcfg_entry_leaf_id_e {
	M3SYS_ENVPCFG_TMP_RH_VALUE_ID = 1,	/* get temperature and relative humidity,
						   byte0-1: Temperature, byte2-3: relative Humidity */
	M3SYS_ENVPCFG_TMP_LIMEN_ID,		/* set temperature limen, 下限, 上限 */
	M3SYS_ENVPCFG_RH_LIMEN_ID,		/* set relative humidity limen */
	M3SYS_ENVPCFG_TMP_RH_ALARM_ID,		/* temperature or relative humidity out of gauge alarm */
	M3SYS_ENVPCFG_TMP_RH_ALARM_MASK_ID,	/*  */
};
static const mib_scalar_node yjm3sys_envpcfg_entry_scalar = {
	&yjm3sys_envpcfg_entry_get_object_def,
	&yjm3sys_envpcfg_entry_get_value,
	&yjm3sys_envpcfg_entry_set_test,
	&yjm3sys_envpcfg_entry_set_value,
	MIB_NODE_SC,
	0
};
static const s32_t yjm3sys_envpcfg_entry_ids[5] = {1, 2, 3, 4, 5};
static struct mib_node* const yjm3sys_envpcfg_entry_nodes[5] = {
	(struct mib_node*)&yjm3sys_envpcfg_entry_scalar, (struct mib_node*)&yjm3sys_envpcfg_entry_scalar,
	(struct mib_node*)&yjm3sys_envpcfg_entry_scalar, (struct mib_node*)&yjm3sys_envpcfg_entry_scalar, 
	(struct mib_node*)&yjm3sys_envpcfg_entry_scalar,
};
static const struct mib_array_node yjm3sys_envpcfg_entry = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	5,
	yjm3sys_envpcfg_entry_ids,
	yjm3sys_envpcfg_entry_nodes
};
/** yjm3sys_envpcfg_tbl   .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.1.2.3 */
static const s32_t yjm3sys_envpcfg_tbl_ids[1] = {1};
static struct mib_node* const yjm3sys_envpcfg_tbl_nodes[1] = { 
	(struct mib_node* const)&yjm3sys_envpcfg_entry,
};
static const struct mib_array_node yjm3sys_envpcfg_tbl = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	1,
	yjm3sys_envpcfg_tbl_ids,
	yjm3sys_envpcfg_tbl_nodes
};

/* yjm3sys_eponcfg_entry(epon-line status detection cfg) .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.1.2.4.1 */
enum yjm3sys_eponcfg_entry_leaf_id_e {
	M3SYS_EPONCFG_EPON_INDEX_ID = 1,/* Now Only 1 */
	M3SYS_EPONCFG_DETEC_DEV_EN_ID,	/* 1:Enable to Inquire the Epon device 0:Disable to Inquire the Epon device */
	M3SYS_EPONCFG_ONU_IP_ID,	/* the Epon ONU device IP address */
	M3SYS_EPONCFG_OLT_IP_ID,	/* the Epon OLT device IP address */
	M3SYS_EPONCFG_EPON_ALARM_ID,	/* bit1:SC <--> ONU link alarm 0 : Normal , 1 : Unlink 
					   bit2:SC <--> OLT link alarm 0 : Normal , 1 : Unlink  */
	M3SYS_EPONCFG_ALARM_MASK_ID,	/* 1: Mask ; 0 : Unmask */
};
static const mib_scalar_node yjm3sys_eponcfg_entry_scalar = {
	&yjm3sys_eponcfg_entry_get_object_def,
	&yjm3sys_eponcfg_entry_get_value,
	&yjm3sys_eponcfg_entry_set_test,
	&yjm3sys_eponcfg_entry_set_value,
	MIB_NODE_SC,
	0
};
static const s32_t yjm3sys_eponcfg_entry_ids[6] = {1, 2, 3, 4, 5, 6};
static struct mib_node* const yjm3sys_eponcfg_entry_nodes[6] = {
	(struct mib_node*)&yjm3sys_eponcfg_entry_scalar, (struct mib_node*)&yjm3sys_eponcfg_entry_scalar,
	(struct mib_node*)&yjm3sys_eponcfg_entry_scalar, (struct mib_node*)&yjm3sys_eponcfg_entry_scalar, 
	(struct mib_node*)&yjm3sys_eponcfg_entry_scalar, (struct mib_node*)&yjm3sys_eponcfg_entry_scalar,
};
static const struct mib_array_node yjm3sys_eponcfg_entry = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	6,
	yjm3sys_eponcfg_entry_ids,
	yjm3sys_eponcfg_entry_nodes
};
/** yjm3sys_envpcfg_tbl   .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.1.2.4 */
static const s32_t yjm3sys_eponcfg_tbl_ids[1] = {1};
static struct mib_node* const yjm3sys_eponcfg_tbl_nodes[1] = { 
	(struct mib_node* const)&yjm3sys_eponcfg_entry,
};
static const struct mib_array_node yjm3sys_eponcfg_tbl = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	1,
	yjm3sys_eponcfg_tbl_ids,
	yjm3sys_eponcfg_tbl_nodes
};



/*
 * yj_m3_if  .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.1.2
 */
static const s32_t yj_m3_if_ids[5] = {1, 2, 3, 4, 5};
static struct mib_node* const yj_m3_if_nodes[5] = {
	(struct mib_node*)&yj_m3_if_leaf_scalar, (struct mib_node*)&yj_m3_if_leaf_scalar,
	(struct mib_node*)&yj_m3_if_leaf_scalar,
	(struct mib_node*)&yjm3sys_envpcfg_tbl, (struct mib_node*)&yjm3sys_eponcfg_tbl, 
};
static const struct mib_array_node yj_m3_if = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	5,
	yj_m3_if_ids,
	yj_m3_if_nodes
};


/*
 * yj_m3_trap .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.1.3
 *
 * .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.1.3.1 -- elock status change: When elock status changed,send the trap.
 * .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.1.3.2 -- door status change
 * .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.1.3.3 -- etnernet link status change
 * .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.1.3.4 -- temp and rh alarm
 * .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.1.3.5 -- 
 */



/*
 * yeejoin-scb .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2.1
 */
static const s32_t yj_scb_ids[2] = { 1, 2};
static struct mib_node* const yj_scb_nodes[2] = {
	(struct mib_node* const)&yj_m3_sys,
	(struct mib_node* const)&yj_m3_if,
	//(struct mib_node* const)&yj_m3_trap,
};
static const struct mib_array_node yj_scb = {
	&noleafs_get_object_def,
	&noleafs_get_value,
	&noleafs_set_test,
	&noleafs_set_value,
	MIB_NODE_AR,
	2,
	yj_scb_ids,
	yj_scb_nodes
};

/* devices .1.3.6.1.4.1.YEEJOIN_ENTERPRISE_ID.2 */
static const s32_t devices_ids[1] = {1};
static struct mib_node* const devices_nodes[1] = { (struct mib_node* const)&yj_scb };
static const struct mib_array_node devices = {
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
static const s32_t yeejoin_ids[1] = {2};
static struct mib_node* const yeejoin_nodes[1] = { (struct mib_node* const)&devices };
static const struct mib_array_node yeejoin = {
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
static const s32_t enterprises_ids[1] = {YEEJOIN_ENTERPRISE_ID};
static struct mib_node* const enterprises_nodes[1] = { (struct mib_node* const)&yeejoin };
static const struct mib_array_node enterprises = {
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
static const s32_t private_ids[1] = { 1 };
static struct mib_node* const private_nodes[1] = { (struct mib_node* const)&enterprises };
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

/**
 * Initialises this private MIB before use.
 */
void lwip_privmib_init(void)
{
	struct snmp_community_st snmp_commu;
	struct nms_if_info_st nms_if;

	printf_syn("SNMP private MIB start.\n");

	read_syscfgdata_tbl(SYSCFGDATA_TBL_SNMP_COMMU, 0, &snmp_commu);
	rt_strncpy(snmp_publiccommunity, snmp_commu.get_commu, SNMP_COMMUNITY_LEN_MAX);

	read_syscfgdata_tbl(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);

	nms_if.trap_ip = lwip_htonl(nms_if.trap_ip);
	snmp_trap_dst_ip_set(0, (ip_addr_t *)&nms_if.trap_ip);
	snmp_trap_dst_enable(0, 1);

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
yj_m3_if_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
{
	/* return to object name, adding index depth (1) */
	ident_len += 1;
	ident     -= 1;

	if (ident_len == 2) {
		u8_t id;

		od->id_inst_len = ident_len;
		od->id_inst_ptr = ident;

		LWIP_ASSERT("invalid id", (ident[0] >= 0) && (ident[0] <= 0xff));

		od->instance = MIB_OBJECT_SCALAR;
		od->asn_type = (SNMP_ASN1_GAUGE);

		id = (u8_t)ident[0];
		switch (id) {
		case SCB_ELOCK_CTRL_ID: /* elock ctrl */
		case SCB_ELOCK_DELAY_AUTO_LOCK_ID:
			od->access   = MIB_OBJECT_READ_WRITE;
			od->v_len = sizeof(u32_t);
			break;

		case SCB_DOOR_STATE_ID: /* door status */
			od->access   = MIB_OBJECT_READ_ONLY;
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

static void yj_m3_if_get_value(struct obj_def *od, u16_t len, void *value)
{
	u32_t *uint_ptr = (u32_t*)value;
	u8_t id;

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));


	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case SCB_ELOCK_CTRL_ID: /* elock ctrl */
		*uint_ptr = !is_elock_open(ELOCK_PORT, ELOCK_PIN);
		break;

	case SCB_ELOCK_DELAY_AUTO_LOCK_ID:
		{
		struct m3_sys_info_st sys_info;
		read_syscfgdata_tbl(SYSCFGDATA_TBL_SYS_INFO, 0, &sys_info);
		*((u32_t*)value) = sys_info.delay_auto_elock_time;
		}
		break;

	case SCB_DOOR_STATE_ID: /* door status */
		*uint_ptr = !is_door_open(ELOCK_SWITCH_PORT, ELOCK_SWITCH_PIN);
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
static u8_t yj_m3_if_set_test(struct obj_def *od, u16_t len, void *value)
{
	u8_t id, set_ok;

	LWIP_UNUSED_ARG(len);
	set_ok = 0;
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	id = (u8_t)od->id_inst_ptr[0];
	if (SCB_ELOCK_CTRL_ID==id || SCB_ELOCK_DELAY_AUTO_LOCK_ID==id) {
		set_ok = 1;
	}

	return set_ok;
}

static void yj_m3_if_set_value(struct obj_def *od, u16_t len, void *value)
{
	u8_t id;

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	id = (u8_t)od->id_inst_ptr[0];
	if (SCB_ELOCK_CTRL_ID == id) {
		/* @todo @fixme: which kind of pointer is 'value'? s32_t or u8_t??? */
		s32_t *ptr = (s32_t*)value;

		if (1 == *ptr)
			elock_close(dummy);
		else
			elock_open(dummy);

	} else if (SCB_ELOCK_DELAY_AUTO_LOCK_ID==id) {
		struct m3_sys_info_st sys_info;
		read_syscfgdata_tbl(SYSCFGDATA_TBL_SYS_INFO, 0, &sys_info);
		sys_info.delay_auto_elock_time = *((u32_t*)value);
		write_syscfgdata_tbl(SYSCFGDATA_TBL_SYS_INFO, 0, &sys_info);
	}

	return;
}



static void yjm3sys_netcfg_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
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
		case M3SYS_NETCFG_LOCAL_IP_ID           :
		case M3SYS_NETCFG_LOCAL_SUBNET_ID       :
		case M3SYS_NETCFG_LOCAL_GW_ID           :
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_IPADDR);
			od->v_len    = 4;
			break;

		case M3SYS_NETCFG_LOCAL_MAC_ID          :
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_OPAQUE);
			od->v_len    = 12; /* string */
			break;

		case M3SYS_NETCFG_READ_COMMUNITY_ID     :
		case M3SYS_NETCFG_WRITE_COMMUNITY_ID    :
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_OPAQUE);
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

static void yjm3sys_netcfg_entry_get_value(struct obj_def *od, u16_t len, void *value)
{
	u32_t *uint_ptr = value;
	char  *pch      = value;
	struct ip_param ipcfg;
	u8_t id;

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	read_syscfgdata_tbl(SYSCFGDATA_TBL_IPCFG, 0, &ipcfg); /* 可以节省程序存储空间 */

	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case M3SYS_NETCFG_LOCAL_IP_ID           :
		*uint_ptr = lwip_htonl(ipcfg.ipaddr.addr);
		break;

	case M3SYS_NETCFG_LOCAL_SUBNET_ID       :
		*uint_ptr = lwip_htonl(ipcfg.netmask.addr);
		break;

	case M3SYS_NETCFG_LOCAL_GW_ID           :
		*uint_ptr = lwip_htonl(ipcfg.gw.addr);
		break;

	case M3SYS_NETCFG_LOCAL_MAC_ID          :
		rt_sprintf(pch,    "%02X", MACOCT0);
		rt_sprintf(pch+2,  "%02X", MACOCT1);
		rt_sprintf(pch+4,  "%02X", MACOCT2);
		rt_sprintf(pch+6,  "%02X", MACOCT3);
		rt_sprintf(pch+8,  "%02X", MACOCT4);
		rt_sprintf(pch+10, "%02X", MACOCT5);
		break;

	/* NOTE: 没有区分get, set, trap的community */
	case M3SYS_NETCFG_READ_COMMUNITY_ID     :
	case M3SYS_NETCFG_WRITE_COMMUNITY_ID    :
		rt_strncpy(value, snmp_publiccommunity, SNMP_COMMUNITY_LEN_MAX);
		break;

	default: /*  */
		break;

	}

	return;
}

static u8_t yjm3sys_netcfg_entry_set_test(struct obj_def *od, u16_t len, void *value)
{
	u8_t id, set_ok;
	//u8_t *pch = value;

	LWIP_UNUSED_ARG(len);
	set_ok = 0;
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));


	id = (u8_t)od->id_inst_ptr[0];
	PRIVMIB_DEBUG(("line:%d, id:%d, ip:%d.%d.%d.%d\n", __LINE__, id, *pch, *(pch+1), *(pch+2), *(pch+3)));
	switch (id) {
	case M3SYS_NETCFG_LOCAL_IP_ID           :
	case M3SYS_NETCFG_LOCAL_SUBNET_ID       :
	case M3SYS_NETCFG_LOCAL_GW_ID           :
	case M3SYS_NETCFG_READ_COMMUNITY_ID     :
	case M3SYS_NETCFG_WRITE_COMMUNITY_ID    :
		set_ok = 1;
		break;

	default:
		break;
	}

	return set_ok;
}

static void yjm3sys_netcfg_entry_set_value(struct obj_def *od, u16_t len, void *value)
{
	u8_t id;
	//u8_t *pch = value;
	struct ip_param ipcfg;

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	read_syscfgdata_tbl(SYSCFGDATA_TBL_IPCFG, 0, &ipcfg); /* 可以节省程序存储空间 */

	/* @todo @fixme: which kind of pointer is 'value'? s32_t or u8_t??? */
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case M3SYS_NETCFG_LOCAL_IP_ID           :
		ipcfg.ipaddr.addr =  lwip_ntohl(*((u32_t *)value));
		break;

	case M3SYS_NETCFG_LOCAL_SUBNET_ID       :
		ipcfg.netmask.addr =  lwip_ntohl(*((u32_t *)value));
		break;

	case M3SYS_NETCFG_LOCAL_GW_ID           :
		ipcfg.gw.addr =  lwip_ntohl(*((u32_t *)value));
		break;

	/* NOTE: 没有区分get, set, trap的community */
	case M3SYS_NETCFG_READ_COMMUNITY_ID    :
	case M3SYS_NETCFG_WRITE_COMMUNITY_ID    :
		{
		struct snmp_community_st snmp_commu;
		read_syscfgdata_tbl(SYSCFGDATA_TBL_SNMP_COMMU, 0, &snmp_commu);
		rt_strncpy(snmp_commu.get_commu, value, SNMP_COMMUNITY_LEN_MAX);
		write_syscfgdata_tbl(SYSCFGDATA_TBL_SNMP_COMMU, 0, &snmp_commu);
		}
		break;

	default:
		break;
	}

	if (M3SYS_NETCFG_LOCAL_IP_ID==id || M3SYS_NETCFG_LOCAL_SUBNET_ID==id || M3SYS_NETCFG_LOCAL_GW_ID==id) {
		write_syscfgdata_tbl(SYSCFGDATA_TBL_IPCFG, 0, &ipcfg);
	}
	
	PRIVMIB_DEBUG(("line:%d, id:%d, ip:%d.%d.%d.%d\n", __LINE__, id, *pch, *(pch+1), *(pch+2), *(pch+3)));

	return;
}


#define M3_SYS_DESCR_INFO_MAX_LEN 24
#define M3_SYS_DEV_INFO_MAX_LEN   17
/*
 * system manage
 */
static void yjm3sys_mcfg_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
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
		case M3SYS_MCFG_DEV_SN_ID	:
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_OPAQUE);
			od->v_len    = DEV_SN_LEN_MAX+1;
			break;

		case M3SYS_MCFG_SYS_DESCR_ID    :
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_OPAQUE);
			od->v_len    = M3_SYS_DESCR_INFO_MAX_LEN;
			break;

		case M3SYS_MCFG_DEV_INFO_ID     :
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_OPAQUE);
			od->v_len    = M3_SYS_DEV_INFO_MAX_LEN;
			break;

		case M3SYS_MCFG_ALARM_STATUS_ID :
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = (SNMP_ASN1_GAUGE);
			od->v_len    = 4;
			break;

		case M3SYS_MCFG_NE_ID_ID        :
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = SNMP_ASN1_GAUGE;
			od->v_len    = 4;
			break;

		case M3SYS_MCFG_SYS_DEFAULT_ID  :
		case M3SYS_MCFG_SYS_REBOOT_ID   :
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_WRITE_ONLY;
			od->asn_type = SNMP_ASN1_GAUGE;
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

static void yjm3sys_mcfg_entry_get_value(struct obj_def *od, u16_t len, void *value)
{
	u32_t *uint_ptr = value;
	u8_t  *pch      = value;
	struct m3_sys_info_st sys_info;
	u8_t id;

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	read_syscfgdata_tbl(SYSCFGDATA_TBL_SYS_INFO, 0, &sys_info);

	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case M3SYS_MCFG_DEV_SN_ID:	/* The SN number of the device. */
		{
		struct m3_sys_info_st m3_sys_info;
		read_syscfgdata_tbl(SYSCFGDATA_TBL_SYS_INFO, 0, &m3_sys_info);
		rt_strncpy(value, m3_sys_info.dev_sn, DEV_SN_LEN_MAX+1);
		}
		break;

	case M3SYS_MCFG_SYS_DESCR_ID:
		rt_strncpy(value, M3_SYS_TYPE_DESCR, rt_strlen(M3_SYS_TYPE_DESCR));
		break;

	case M3SYS_MCFG_DEV_INFO_ID     :
		/* M3_SYS_DEV_INFO_MAX_LEN */
		*pch++ = 1;				/* Byte 0 : system's type */
		*pch++ = 1;				/* Byte 1 : number of port */
		*pch++ = (sys_info.sw_ver>>16)&0xff;	/* Byte 2 : Soft's Version (major) */
		*pch++ = (sys_info.sw_ver>>8)&0xff;	/* Byte 3 : Soft's Version (minor) */
		*pch++ = (sys_info.sw_ver)&0xff;	/* Byte 4 : Soft's Version (revise) */
		*pch++ = M3_HW_VERSION;			/* Byte 5 : Hardware's Version(major) */
		*pch++ = M3_HW_SUBVERSION;		/* Byte 6 : Hardware's Version(minor)  */
		*pch++ = M3_HW_REVISION;		/* Byte 7 : Hardware's Version(revise) */
		*pch++ = 1;				/* Byte 8 : FE Cooper port number */
		*pch++ = 0;				/* Byte 9 : FE Fiber port number  */
		*pch++ = 0;				/* Byte 10: GE Cooper port number */
		*pch++ = 0;				/* Byte 11: GE Fiber port number */
		*pch++ = 0;				/* Byte 12 : E1 port number */
		*pch++ = 1; 				/* Byte 13 : Serial port number */
		*pch++ = 0;				/* Byte 14 : USB-A port number */
		*pch++ = 0;				/* Byte 15 : USB OTG port number */
		*pch++ = 0;				/* Byte 16 : MMC port number */
		break;

	case M3SYS_MCFG_ALARM_STATUS_ID :	/* mark by David */
		*uint_ptr = 1;
		break;

	case M3SYS_MCFG_NE_ID_ID        :
		*uint_ptr = sys_info.neid;
		break;

	default:
		break;
	}

	return;
}

static u8_t yjm3sys_mcfg_entry_set_test(struct obj_def *od, u16_t len, void *value)
{
	u8_t id, set_ok;
	//u8_t *pch = value;

	LWIP_UNUSED_ARG(len);
	set_ok = 0;
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	id = (u8_t)od->id_inst_ptr[0];
	PRIVMIB_DEBUG(("line:%d, id:%d, ip:%d.%d.%d.%d\n", __LINE__, id, *pch, *(pch+1), *(pch+2), *(pch+3)));
	switch (id) {
	case M3SYS_MCFG_NE_ID_ID        :
	case M3SYS_MCFG_SYS_DEFAULT_ID  :
	case M3SYS_MCFG_SYS_REBOOT_ID   :
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

static void yjm3sys_mcfg_entry_set_value(struct obj_def *od, u16_t len, void *value)
{
	u8_t id;
	//u8_t *pch = value;

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	/* @todo @fixme: which kind of pointer is 'value'? s32_t or u8_t??? */
	id = (u8_t)od->id_inst_ptr[0];
	PRIVMIB_DEBUG(("line:%d, id:%d, ip:%d.%d.%d.%d\n", __LINE__, id, *pch, *(pch+1), *(pch+2), *(pch+3)));

	switch (id) {
	case M3SYS_MCFG_NE_ID_ID        :
		{
		struct m3_sys_info_st sys_info;
		read_syscfgdata_tbl(SYSCFGDATA_TBL_SYS_INFO, 0, &sys_info);
		sys_info.neid = (*((u32_t *)value));
		write_syscfgdata_tbl(SYSCFGDATA_TBL_SYS_INFO, 0, &sys_info);
		}
		break;

	case M3SYS_MCFG_SYS_DEFAULT_ID  :
		if (1==(*((u32_t *)value)))
			restore_default_syscfgdata();
		break;

	case M3SYS_MCFG_SYS_REBOOT_ID   :
		if (1==(*((u32_t *)value)))
			reset_whole_system();
		break;

	default:
		break;
	}

	return;
}


/*
 * trap cfg
 */
static void yjm3sys_trapcfg_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
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
		case M3SYS_TRAPCFG_TRAP_TYPE_ID		:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = SNMP_ASN1_GAUGE;
			od->v_len    = 4;
			break;

		case M3SYS_TRAPCFG_ADDR_ID              :
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_IPADDR);
			od->v_len    = 4;
			break;

		case M3SYS_TRAPCFG_SNMP_VER_ID          :
		case M3SYS_TRAPCFG_TARGET_PORT_ID       :
		case M3SYS_TRAPCFG_VALID_ID             :
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = SNMP_ASN1_GAUGE;
			od->v_len    = 4;
			break;

		case M3SYS_TRAPCFG_COMMUNITY_ID         :
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_OPAQUE);
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

static void yjm3sys_trapcfg_entry_get_value(struct obj_def *od, u16_t len, void *value)
{
	u32_t *uint_ptr = value;
	u8_t id;
	struct nms_if_info_st nms_if;

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	read_syscfgdata_tbl(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case M3SYS_TRAPCFG_TRAP_TYPE_ID		:
		*uint_ptr = 6;
		break;

	case M3SYS_TRAPCFG_ADDR_ID              :
		*uint_ptr = lwip_htonl(nms_if.trap_ip);
		break;

	case M3SYS_TRAPCFG_SNMP_VER_ID          :
		*uint_ptr = 0;
		break;

	case M3SYS_TRAPCFG_TARGET_PORT_ID       :
		*uint_ptr = nms_if.trap_port;
		break;

	case M3SYS_TRAPCFG_COMMUNITY_ID         :
		{
		struct snmp_community_st snmp_commu;
		read_syscfgdata_tbl(SYSCFGDATA_TBL_SNMP_COMMU, 0, &snmp_commu);
		rt_strncpy(value, snmp_commu.get_commu, SNMP_COMMUNITY_LEN_MAX);
		}
		break;

	case M3SYS_TRAPCFG_VALID_ID             : /* mark by David */
		*uint_ptr = nms_if.trap_enable_bits;
		break;

	default:
		break;
	}

	return;
}

static u8_t yjm3sys_trapcfg_entry_set_test(struct obj_def *od, u16_t len, void *value)
{
	u8_t id, set_ok;

	LWIP_UNUSED_ARG(len);
	set_ok = 0;
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case M3SYS_TRAPCFG_ADDR_ID              :
	case M3SYS_TRAPCFG_TARGET_PORT_ID       :
	case M3SYS_TRAPCFG_COMMUNITY_ID         :
	case M3SYS_TRAPCFG_VALID_ID             :
		#if 0
		if ((0==*sint_ptr) || (1==*sint_ptr))
			set_ok = 1;
		#else
		set_ok = 1;
		#endif
		break;
	#if 0
	/* M3只支持v1版本 */
	case M3SYS_TRAPCFG_SNMP_VER_ID          :
		break;
	#endif

	default:
		break;
	}


	return set_ok;
}

static void yjm3sys_trapcfg_entry_set_value(struct obj_def *od, u16_t len, void *value)
{
	u8_t id;
	struct nms_if_info_st nms_if;

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	read_syscfgdata_tbl(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);

	/* @todo @fixme: which kind of pointer is 'value'? s32_t or u8_t??? */
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case M3SYS_TRAPCFG_ADDR_ID              :
		nms_if.trap_ip = lwip_ntohl(*((u32_t *)value));
		break;

	case M3SYS_TRAPCFG_TARGET_PORT_ID       :
		nms_if.trap_port = (*((u32_t *)value));
		break;

	case M3SYS_TRAPCFG_COMMUNITY_ID         :
		{
		struct snmp_community_st snmp_commu;
		read_syscfgdata_tbl(SYSCFGDATA_TBL_SNMP_COMMU, 0, &snmp_commu);
		rt_strncpy(snmp_commu.get_commu, value, SNMP_COMMUNITY_LEN_MAX);
		write_syscfgdata_tbl(SYSCFGDATA_TBL_SNMP_COMMU, 0, &snmp_commu);
		}
		break;

	case M3SYS_TRAPCFG_VALID_ID             :
		nms_if.trap_enable_bits = (*((u32_t *)value)); /* mark by David */
		break;

	default:
		break;
	}

	if (M3SYS_TRAPCFG_ADDR_ID==id || M3SYS_TRAPCFG_TARGET_PORT_ID==id || M3SYS_TRAPCFG_VALID_ID==id)
		write_syscfgdata_tbl(SYSCFGDATA_TBL_NMS_IF, 0, &nms_if);
	
	return;
}

/*
 * env parameter cfg
 */
static void yjm3sys_envpcfg_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
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
		case M3SYS_ENVPCFG_TMP_RH_VALUE_ID	:
		case M3SYS_ENVPCFG_TMP_RH_ALARM_ID      :
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = SNMP_ASN1_GAUGE;
			od->v_len    = 4;
			break;

		case M3SYS_ENVPCFG_TMP_LIMEN_ID         :
		case M3SYS_ENVPCFG_RH_LIMEN_ID          :
		case M3SYS_ENVPCFG_TMP_RH_ALARM_MASK_ID :
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = SNMP_ASN1_GAUGE;
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

static void yjm3sys_envpcfg_entry_get_value(struct obj_def *od, u16_t len, void *value)
{
	u32_t *uint_ptr = value;
	struct temp_rh_limen t_rh;
	u8_t id;

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	read_temp_rh_limen(&t_rh);
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case M3SYS_ENVPCFG_TMP_RH_VALUE_ID	:
		{
		u32_t temp1, temp2;
		temp1 = 0;
		if (tmp_from_am2301 & (1<<15))
			temp1 = -(tmp_from_am2301 & ~(1<<15));
		else
			temp1 = tmp_from_am2301;

		temp2 = 0;
		if (rh_from_am2301 & (1<<15))
			temp2 = -(rh_from_am2301 & ~(1<<15));
		else
			temp2 = rh_from_am2301;

		*uint_ptr = ((temp2 << 16)) | (temp1);
		}
		break;

	case M3SYS_ENVPCFG_TMP_LIMEN_ID         :
		*uint_ptr = ((t_rh.temp_h)<<16) | (t_rh.temp_l);
		break;

	case M3SYS_ENVPCFG_RH_LIMEN_ID          :
		*uint_ptr = ((t_rh.rh_h)<<16) | (t_rh.rh_l);
		break;

	case M3SYS_ENVPCFG_TMP_RH_ALARM_ID      :
		*uint_ptr = ((is_bit_set(warning_info, RH_OVERRUN))<<1) | (is_bit_set(warning_info, TMP_OVERRUN));
		break;

	case M3SYS_ENVPCFG_TMP_RH_ALARM_MASK_ID : /* mark by David */
		*uint_ptr = (t_rh.warn_mask);
		break;

	default:
		break;
	}

	return;
}

static u8_t yjm3sys_envpcfg_entry_set_test(struct obj_def *od, u16_t len, void *value)
{
	u8_t id, set_ok;

	LWIP_UNUSED_ARG(len);
	set_ok = 0;
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case M3SYS_ENVPCFG_TMP_LIMEN_ID         :
	case M3SYS_ENVPCFG_RH_LIMEN_ID          :
	case M3SYS_ENVPCFG_TMP_RH_ALARM_MASK_ID :
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

static void yjm3sys_envpcfg_entry_set_value(struct obj_def *od, u16_t len, void *value)
{
	u8_t *pch = value;
	struct temp_rh_limen t_rh;
	u8_t id;


	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	read_temp_rh_limen(&t_rh);

	/* @todo @fixme: which kind of pointer is 'value'? s32_t or u8_t??? */
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case M3SYS_ENVPCFG_TMP_LIMEN_ID         :
		t_rh.temp_l = *(rt_int16_t*)pch;
		t_rh.temp_h = *(rt_int16_t*)(pch+2);
		break;

	case M3SYS_ENVPCFG_RH_LIMEN_ID          :
		t_rh.rh_l = *(rt_int16_t*)pch;
		t_rh.rh_h = *(rt_int16_t*)(pch+2);
		break;

	case M3SYS_ENVPCFG_TMP_RH_ALARM_MASK_ID :
		t_rh.warn_mask = *(rt_uint32_t*)pch;
		break;

	default:
		break;
	}

	save_temp_rh_limen(&t_rh);
	
	return;
}

/*
 * epon line status detection cfg
 */
static void yjm3sys_eponcfg_entry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
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
		case M3SYS_EPONCFG_EPON_INDEX_ID:
		case M3SYS_EPONCFG_EPON_ALARM_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_ONLY;
			od->asn_type = SNMP_ASN1_GAUGE;
			od->v_len    = 4;
			break;

		case M3SYS_EPONCFG_DETEC_DEV_EN_ID:
		case M3SYS_EPONCFG_ALARM_MASK_ID:
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = SNMP_ASN1_GAUGE;
			od->v_len    = 4;
			break;

		case M3SYS_EPONCFG_ONU_IP_ID    :
		case M3SYS_EPONCFG_OLT_IP_ID    :
			//od->instance = MIB_OBJECT_SCALAR;
			od->access   = MIB_OBJECT_READ_WRITE;
			od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_IPADDR);
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

static void yjm3sys_eponcfg_entry_get_value(struct obj_def *od, u16_t len, void *value)
{
	u32_t *uint_ptr = value;
	struct epon_device_st eponp;
	u8_t id;

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	read_syscfgdata_tbl(SYSCFGDATA_TBL_EPON_DEV, 0, &eponp);
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case M3SYS_EPONCFG_EPON_INDEX_ID:
		*uint_ptr = (1);
		break;

	case M3SYS_EPONCFG_DETEC_DEV_EN_ID:
		*uint_ptr = is_bit_set(eponp.epon_bitv, EPON_DEV_DETECT_EN_BIT);
		break;

	case M3SYS_EPONCFG_ONU_IP_ID    :
		*uint_ptr = lwip_htonl(eponp.onu_ip);
		break;

	case M3SYS_EPONCFG_OLT_IP_ID    :
		*uint_ptr = lwip_htonl(eponp.olt_ip);
		break;

	case M3SYS_EPONCFG_EPON_ALARM_ID: /* mark by David */
		*uint_ptr = (0);
		break;

	case M3SYS_EPONCFG_ALARM_MASK_ID:
		*uint_ptr = is_bit_set(eponp.epon_bitv, EPON_DEV_ALARM_MASK_BIT);
		break;

	default:
		break;
	}

	return;
}

static u8_t yjm3sys_eponcfg_entry_set_test(struct obj_def *od, u16_t len, void *value)
{
	u8_t id, set_ok;

	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	set_ok = 0;
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case M3SYS_EPONCFG_DETEC_DEV_EN_ID:
	case M3SYS_EPONCFG_ONU_IP_ID    :
	case M3SYS_EPONCFG_OLT_IP_ID    :
	case M3SYS_EPONCFG_ALARM_MASK_ID:
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

static void yjm3sys_eponcfg_entry_set_value(struct obj_def *od, u16_t len, void *value)
{
	struct epon_device_st eponp;
	u8_t id;


	LWIP_UNUSED_ARG(len);
	LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));

	read_syscfgdata_tbl(SYSCFGDATA_TBL_EPON_DEV, 0, &eponp);

	/* @todo @fixme: which kind of pointer is 'value'? s32_t or u8_t??? */
	id = (u8_t)od->id_inst_ptr[0];
	switch (id) {
	case M3SYS_EPONCFG_DETEC_DEV_EN_ID:
		if (1 == *((rt_uint32_t*)value))
			set_bit(eponp.epon_bitv, EPON_DEV_DETECT_EN_BIT);
		else
			clr_bit(eponp.epon_bitv, EPON_DEV_DETECT_EN_BIT);
		break;

	case M3SYS_EPONCFG_ONU_IP_ID    :
		eponp.onu_ip = lwip_ntohl(*((u32_t *)value));
		break;

	case M3SYS_EPONCFG_OLT_IP_ID    :
		eponp.olt_ip = lwip_ntohl(*((u32_t *)value));
		break;

	case M3SYS_EPONCFG_ALARM_MASK_ID:
		if (0 != *((rt_uint32_t*)value))
			set_bit(eponp.epon_bitv, EPON_DEV_ALARM_MASK_BIT);
		else
			clr_bit(eponp.epon_bitv, EPON_DEV_ALARM_MASK_BIT);
		break;

	default:
		break;
	}

	write_syscfgdata_tbl(SYSCFGDATA_TBL_EPON_DEV, 0, &eponp);

	return;
}

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
	snmp_send_trap(SNMP_GENTRAP_ENTERPRISESPC, &yeejoin_id, trapid);
}

void elock_open_trap(void)
{
	set_port_pin(ELOCK_PORT, ELOCK_PIN);
	snmp_yeejoin_trap(YTI_ELOCK_STATUS_CHANGE);
}

void elock_close_trap(void)
{
	clr_port_pin(ELOCK_PORT, ELOCK_PIN);
	snmp_yeejoin_trap(YTI_ELOCK_STATUS_CHANGE);
}


#endif /* LWIP_SNMP */
