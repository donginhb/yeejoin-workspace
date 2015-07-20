/*
 * EZMacPro_cfg.h
 *
 *  Created on: 2013-11-12
 *      Author: David, zhaoshaowei@yeejoin.com
 *
 * description:
 * 	该文件用于定义EZMacProc协议栈的配置宏
 */

#ifndef EZMACPRO_CFG_H_
#define EZMACPRO_CFG_H_

#include <app-hw-resource.h>

/* 编译器类型 */
#define _ARM_GCC_

/* 如果使用C51语法, 请定义该宏为1 */
#define C51_SYNTAX_	0



/*
 * ---------------- 以下内容来自于"EZMAC AND EZHOP USER’S GUIDE" ----------------
 *
 * */


/*
 * 2.2.2.1. Supported Chip Revision
 * Radio chip revision
 * */
#define B1_ONLY


/*
 * 2.2.2.2. Frequency Band Selection
 * All major sub-GHz ISM bands (434, 868, and 915 MHz) are supported by EZMac and selected via the appropriate
 * definition in the Project/Tool Chain Integration/Compiler menu of the Silicon Laboratories IDE:
 *
 * FREQUENCY_BAND_434
 * FREQUENCY_BAND_868
 * FREQUENCY_BAND_915
 * FREQUENCY_BAND_868
 *  */
#define FREQUENCY_BAND_434


/*
 * 2.2.2.3. Radio Operation Selection
 * The source code can be compiled for transmitter, receiver, or transceiver operation by enabling the appropriate
 * definition:
 * TRANSCEIVER_OPERATION
 * TRANSMITTER_ONLY_OPERATION
 * RECEIVER_ONLY_OPERATION
 * */
#define TRANSCEIVER_OPERATION


/*
 * 2.2.2.4. Channel Number Selection
 * These definitions define the operation mode of the stack: EZMac or EZHop mode.
 * If the FOUR_CHANNEL_IS_USED definition is enabled, the stack operates as the EZMac module and supports up
 * to four channels. If the MORE_CHANNEL_IS_USED definition is enabled, the stack operates as EZHop and
 * supports up to 50 channels.
 * FOUR_CHANNEL_IS_USED
 * MORE_CHANNEL_IS_USED
 * */
#define FOUR_CHANNEL_IS_USED


/*
 * 2.2.2.5. Antenna Diversity
 * If the ANTENNA_DIVERSITY_ENABLED definition is enabled, the antenna diversity feature is compiled into the
 * source code.
 * Note: Antenna diversity is supported only if EZMac mode is selected.
 */

/*
 * 2.2.2.6. Packet Format Selection
 * Two definitions determine the packet format:
 * STANDARD_PACKET_FORMAT
 * EXTENDED_PACKET_FORMAT
 *
 * For a detailed description, see "3.3. Packet Format" on page 17. The automatic acknowledgement feature can be
 * used only in EXTENDED_PACKET_FORMAT. Further information about the automatic acknowledgment can be
 * found in "3.6. Automatic Acknowledgement" on page 22.
 *
 * */
#define EXTENDED_PACKET_FORMAT


/*
 * 2.2.2.7. Packet Forwarding
 * If the PACKET_FORWARDING_SUPPORTED definition is enabled, the packet forwarding feature can be used.
 * Packet forwarding can be used only with extended packet format.
 * */
//#define PACKET_FORWARDING_SUPPORTED





#endif /* EZMACPRO_CFG_H_ */
