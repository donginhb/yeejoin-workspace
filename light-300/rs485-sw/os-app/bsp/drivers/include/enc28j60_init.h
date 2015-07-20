
/*
 * Copyright (c) 2006-2008 by Roland Riegel <feedback@roland-riegel.de>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef ENC28J60_INIT_H
#define ENC28J60_INIT_H

#include <rtdef.h>

/**
 * \addtogroup net
 *
 * @{
 */
/**
 * \addtogroup net_driver Hardware drivers
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
 * Microchip ENC28J60 initialization header (license: GPLv2)
 *
 * \author Roland Riegel
 */

bool enc28j60_init(const uint8_t* mac);

/**
 * @}
 * @}
 * @}
 */

#endif

