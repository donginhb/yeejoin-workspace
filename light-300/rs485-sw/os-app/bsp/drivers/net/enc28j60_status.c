
/*
 * Copyright (c) 2006-2008 by Roland Riegel <feedback@roland-riegel.de>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "enc28j60_io.h"
#include "enc28j60_status.h"

#include <rtdef.h>

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
 * Microchip ENC28J60 status implementation (license: GPLv2)
 *
 * \author Roland Riegel
 */

/**
 * Checks wether the link is up and has been continuously up since the last call.
 *
 * \returns \c TRUE if the network link is and has been up, \c FALSE otherwise.
 */
bool enc28j60_link_up()
{
	uint16_t phstat1 = enc28j60_read_phy(PHSTAT1);
	return phstat1 & (1 << PHSTAT1_LLSTAT);
}

/**
 * @}
 * @}
 * @}
 */

