
/*
 * Copyright (c) 2006-2008 by Roland Riegel <feedback@roland-riegel.de>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef ENC28J60_CONFIG_H
#define ENC28J60_CONFIG_H

/**
 * \addtogroup net
 *
 * @{
 */
/**
 * \addtogroup net_driver
 *
 * @{
 */
/**
 * \addtogroup net_driver_enc28j60
 *
 * @{
 */
/**
 * \file
 * Microchip ENC28J60 I/O configuration header (license: GPLv2)
 *
 * \author Roland Riegel
 */
/* i/o 已集中配置, 这里只是复位 */
#define enc28j60_config_io()	enc28j60_reset_on()

#define enc28j60_select()	gpio_bits_reset(ENC28J60_CS_PORT, ENC28J60_CS_PIN)
#define enc28j60_deselect()	gpio_bits_set(ENC28J60_CS_PORT, ENC28J60_CS_PIN)

#define enc28j60_reset_on()	gpio_bits_reset(ENC28J60_RST_PORT, ENC28J60_RST_PIN)
#define enc28j60_reset_off()	gpio_bits_set(ENC28J60_RST_PORT, ENC28J60_RST_PIN)

#define ENC28J60_FULL_DUPLEX 1

/**
 * @}
 * @}
 * @}
 */

#endif

