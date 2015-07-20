
/*
 * Copyright (c) 2006-2008 by Roland Riegel <feedback@roland-riegel.de>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "enc28j60_config.h"
#include "enc28j60_init.h"
#include "enc28j60_io.h"

#include <rtdef.h>
#include <board.h>

#define ENC28J60_INIT_DEBUG(x) //rt_kprintf x

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
 * \addtogroup net_driver_enc28j60 Microchip ENC28J60 driver
 *
 * @{
 */
/**
 * \file
 * Microchip ENC28J60 initialization (license: GPLv2)
 *
 * \author Roland Riegel
 */

/**
 * Reset and initialize the ENC28J60 and starts packet transmission/reception.
 *
 * \param[in] mac A pointer to a 6-byte buffer containing the MAC address.
 * \returns \c true on success, \c false on failure.
 */
bool enc28j60_init(const uint8_t* mac)
{
#if 0
	/* init i/o */
	enc28j60_io_init();
#endif
	/* reset device */
	enc28j60_reset();

	/* configure rx/tx buffers */
	enc28j60_write(ERXSTL, RX_START & 0xff);
	enc28j60_write(ERXSTH, RX_START >> 8);
	enc28j60_write(ERXNDL, RX_END & 0xff);
	enc28j60_write(ERXNDH, RX_END >> 8);

	enc28j60_write(ERXWRPTL, RX_START & 0xff);
	enc28j60_write(ERXWRPTH, RX_START >> 8);
	enc28j60_write(ERXRDPTL, RX_START & 0xff);
	enc28j60_write(ERXRDPTH, RX_START >> 8);

	/* configure frame filters */
	enc28j60_write(ERXFCON, (1 << ERXFCON_UCEN)  | /* accept unicast packets */
				   (1 << ERXFCON_CRCEN) | /* accept packets with valid CRC only */
				   (0 << ERXFCON_PMEN)  | /* no pattern matching */
				   (0 << ERXFCON_MPEN)  | /* ignore magic packets */
				   (0 << ERXFCON_HTEN)  | /* disable hash table filter */
				   (0 << ERXFCON_MCEN)  | /* ignore multicast packets */
				   (1 << ERXFCON_BCEN)  | /* accept broadcast packets */
				   (0 << ERXFCON_ANDOR)   /* packets must meet at least one criteria */
				  );

	/* configure MAC */
	enc28j60_clear_bits(MACON2, (1 << MACON2_MARST));
	enc28j60_write(MACON1, (0 << MACON1_LOOPBK) |
#if ENC28J60_FULL_DUPLEX
				   (1 << MACON1_TXPAUS) |
				   (1 << MACON1_RXPAUS) |
#else
				   (0 << MACON1_TXPAUS) |
				   (0 << MACON1_RXPAUS) |
#endif
				   (0 << MACON1_PASSALL) |
				   (1 << MACON1_MARXEN)
				  );
	enc28j60_write(MACON3, (1 << MACON3_PADCFG2) |
				   (1 << MACON3_PADCFG1) |
				   (1 << MACON3_PADCFG0) |
				   (1 << MACON3_TXCRCEN) |
				   (0 << MACON3_PHDRLEN) |
				   (0 << MACON3_HFRMEN) |
				   (1 << MACON3_FRMLNEN) |
#if ENC28J60_FULL_DUPLEX
				   (1 << MACON3_FULDPX)
#else
				   (0 << MACON3_FULDPX)
#endif
				  );
	enc28j60_write(MAMXFLL, 0xee);
	enc28j60_write(MAMXFLH, 0x05);
#if ENC28J60_FULL_DUPLEX
	enc28j60_write(MABBIPG, 0x15);
#else
	enc28j60_write(MABBIPG, 0x12);
#endif
	enc28j60_write(MAIPGL, 0x12);
#if !ENC28J60_FULL_DUPLEX
	enc28j60_write(MAIPGH, 0x0c);
#endif
	enc28j60_write(MAADR0, mac[5]);
	enc28j60_write(MAADR1, mac[4]);
	enc28j60_write(MAADR2, mac[3]);
	enc28j60_write(MAADR3, mac[2]);
	enc28j60_write(MAADR4, mac[1]);
	enc28j60_write(MAADR5, mac[0]);

	if(enc28j60_read(MAADR5)== mac[0]) {
		ENC28J60_INIT_DEBUG(("MAADR5 = 0x%x\n", enc28j60_read(MAADR5)));
		ENC28J60_INIT_DEBUG(("MAADR4 = 0x%x\n", enc28j60_read(MAADR4)));
		ENC28J60_INIT_DEBUG(("MAADR3 = 0x%x\n", enc28j60_read(MAADR3)));
		ENC28J60_INIT_DEBUG(("MAADR2 = 0x%x\n", enc28j60_read(MAADR2)));
		ENC28J60_INIT_DEBUG(("MAADR1 = 0x%x\n", enc28j60_read(MAADR1)));
		ENC28J60_INIT_DEBUG(("MAADR0 = 0x%x\n", enc28j60_read(MAADR0)));
	}

	/*
	 * configure PHY
	 */

	/* leda -- green led, ledb -- orange led */
#if 0
	enc28j60_write_phy(PHLCON, (1 << PHLCON_LACFG3) |
					   (1 << PHLCON_LACFG2) |
					   (0 << PHLCON_LACFG1) |
					   (1 << PHLCON_LACFG0) |
					   (0 << PHLCON_LBCFG3) |
					   (1 << PHLCON_LBCFG2) |
					   (0 << PHLCON_LBCFG1) |
					   (1 << PHLCON_LBCFG0) |
					   (0 << PHLCON_LFRQ1) |
					   (0 << PHLCON_LFRQ0) |
					   (1 << PHLCON_STRCH)
					  );
#else
	enc28j60_write_phy(PHLCON, (0 << PHLCON_LACFG3) |
					   (1 << PHLCON_LACFG2) |
					   (0 << PHLCON_LACFG1) |
					   (0 << PHLCON_LACFG0) |
					   (0 << PHLCON_LBCFG3) |
					   (1 << PHLCON_LBCFG2) |
					   (1 << PHLCON_LBCFG1) |
					   (1 << PHLCON_LBCFG0) |
					   (0 << PHLCON_LFRQ1) |
					   (0 << PHLCON_LFRQ0) |
					   (1 << PHLCON_STRCH)
					  );

#endif
	enc28j60_write_phy(PHCON1, (0 << PHCON1_PRST) |
					   (0 << PHCON1_PLOOPBK) |
					   (0 << PHCON1_PPWRSV) |
#if ENC28J60_FULL_DUPLEX
					   (1 << PHCON1_PDPXMD)
#else
					   (0 << PHCON1_PDPXMD)
#endif
					  );
	enc28j60_write_phy(PHCON2, (0 << PHCON2_FRCLNK) |
					   (0 << PHCON2_TXDIS) |
					   (0 << PHCON2_JABBER) |
					   (1 << PHCON2_HDLDIS)
					  );

	/* start receiving packets */
	enc28j60_set_bits(ECON2, (1 << ECON2_AUTOINC));
#if 0
	enc28j60_set_bits(ECON1, (1 << ECON1_RXEN));

	ENC28J60_INIT_DEBUG(("line:%d, EIE = 0x%x\n", __LINE__, enc28j60_read(EIE)));

	/* enable interrupt, David  */
	enc28j60_set_bits(EIE, (1 << EIE_INTIE) | (1<<EIE_PKTIE) | (1<<EIE_TXIE));

	ENC28J60_INIT_DEBUG(("line:%d, EIE = 0x%x\n", __LINE__, enc28j60_read(EIE)));
#endif
	return true;
}

/**
 * @}
 * @}
 * @}
 */

