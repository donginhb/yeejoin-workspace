
/*
 * Copyright (c) 2006-2008 by Roland Riegel <feedback@roland-riegel.de>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "enc28j60_config.h"
#include "enc28j60_io.h"

#include <rtdef.h>
#include <board.h>

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
 * Microchip ENC28J60 I/O implementation (license: GPLv2)
 *
 * \author Roland Riegel
 */

#define spi_send_byte(data)	spix_send_byte(SPI2, data)
#define spi_rec_byte()		spix_rec_byte(SPI2)

#define spi_send_data(data, len)	spix_send_data(SPI2, data, len)
#define spi_rec_data(buf, len)		spix_rec_data(SPI2, buf, len)

/* read control register */
/* read buffer memory */
#define enc28j60_rcr(address)	spi_send_byte(0x00 | (address))
#define enc28j60_rbm() 		spi_send_byte(0x3a)

/* write control register */
/* write buffer memory */
#define enc28j60_wcr(address)	spi_send_byte(0x40 | (address))
#define enc28j60_wbm(address)	spi_send_byte(0x7a)

/* bit filed set */
/* bit filed clr */
#define enc28j60_bfs(address)	spi_send_byte(0x80 | (address))
#define enc28j60_bfc(address)	spi_send_byte(0xa0 | (address))

/* soft reset */
#define enc28j60_sc() 		spi_send_byte(0xff)


static void enc28j60_bank(uint8_t num);

#define MS_CNT 10301
#define US_CNT 11

void _delay_ms(u32 nMs)
{
	u32 i;
	
	for(; nMs !=0; nMs--)
	{
		//i = 10301;
		i = MS_CNT;
		while(i--);
	}
}

void _delay_us(u32 nMs)
{
	u32 i;
	
	for(; nMs !=0; nMs--)
	{
		//i = 11;
		i = US_CNT;
		while(i--);
	}
}


/**
 * \internal
 * Initializes the SPI interface to the ENC28J60 chips.
 */
void enc28j60_io_init()
{
    /* configure pins */
    enc28j60_config_io();
}

/**
 * \internal
 * Forces a reset to the ENC28J60.
 *
 * After the reset a reinitialization is necessary.
 */
void enc28j60_reset()
{
    enc28j60_reset_on();
    _delay_ms(10);
    enc28j60_reset_off();
    _delay_ms(10);
}

/**
 * \internal
 * Reads the value of a hardware register.
 *
 * \param[in] address The address of the register to read.
 * \returns The register value.
 */
uint8_t enc28j60_read(uint8_t address)
{
    uint8_t result;

    /* switch to register bank */
    enc28j60_bank((address & MASK_BANK) >> SHIFT_BANK);
    
    /* read from address */
    enc28j60_select();
    enc28j60_rcr((address & MASK_ADDR) >> SHIFT_ADDR);
    if(address & MASK_DBRD)
        spi_rec_byte();
    result = spi_rec_byte();
    enc28j60_deselect();

    return result;
}

/**
 * \internal
 * Writes the value of a hardware register.
 *
 * \param[in] address The address of the register to write.
 * \param[in] value The value to write into the register.
 */
void enc28j60_write(uint8_t address, uint8_t value)
{
    /* switch to register bank */
    enc28j60_bank((address & MASK_BANK) >> SHIFT_BANK);
    
    /* write to address */
    enc28j60_select();
    enc28j60_wcr((address & MASK_ADDR) >> SHIFT_ADDR);
    spi_send_byte(value);
    enc28j60_deselect();
}

/**
 * \internal
 * Clears bits in a hardware register.
 *
 * Performs a NAND operation on the current register value
 * and the given bitmask.
 *
 * \param[in] address The address of the register to alter.
 * \param[in] bits A bitmask specifiying the bits to clear.
 */
void enc28j60_clear_bits(uint8_t address, uint8_t bits)
{
    /* switch to register bank */
    enc28j60_bank((address & MASK_BANK) >> SHIFT_BANK);
    
    /* write to address */
    enc28j60_select();
    enc28j60_bfc((address & MASK_ADDR) >> SHIFT_ADDR);
    spi_send_byte(bits);
    enc28j60_deselect();
}

/**
 * \internal
 * Sets bits in a hardware register.
 *
 * Performs an OR operation on the current register value
 * and the given bitmask.
 *
 * \param[in] address The address of the register to alter.
 * \param[in] bits A bitmask specifiying the bits to set.
 */
void enc28j60_set_bits(uint8_t address, uint8_t bits)
{
    /* switch to register bank */
    enc28j60_bank((address & MASK_BANK) >> SHIFT_BANK);
    
    /* write to address */
    enc28j60_select();
    enc28j60_bfs((address & MASK_ADDR) >> SHIFT_ADDR);
    spi_send_byte(bits);
    enc28j60_deselect();
}

/**
 * \internal
 * Reads the value of a hardware PHY register.
 *
 * \param[in] address The address of the PHY register to read.
 * \returns The register value.
 */
uint16_t enc28j60_read_phy(uint8_t address)
{
    enc28j60_write(MIREGADR, address);
    enc28j60_set_bits(MICMD, (1 << MICMD_MIIRD));

    _delay_us(10);

    while(enc28j60_read(MISTAT) & (1 << MISTAT_BUSY));
    
    enc28j60_clear_bits(MICMD, (1 << MICMD_MIIRD));

    return ((uint16_t) enc28j60_read(MIRDH)) << 8 |
           ((uint16_t) enc28j60_read(MIRDL));
}

/**
 * \internal
 * Writes the value to a hardware PHY register.
 *
 * \param[in] address The address of the PHY register to write.
 * \param[in] value The value to write into the register.
 */
void enc28j60_write_phy(uint8_t address, uint16_t value)
{
    enc28j60_write(MIREGADR, address);
    enc28j60_write(MIWRL, value & 0xff);
    enc28j60_write(MIWRH, value >> 8);
    
    _delay_us(10);

    while(enc28j60_read(MISTAT) & (1 << MISTAT_BUSY));
}

/**
 * \internal
 * Reads a byte from the RAM buffer at the current position.
 *
 * \returns The byte read from the current RAM position.
 */
uint8_t enc28j60_read_buffer_byte()
{
    uint8_t b;

    enc28j60_select();
    enc28j60_rbm();

    b = spi_rec_byte();
    
    enc28j60_deselect();

    return b;
}

/**
 * \internal
 * Writes a byte to the RAM buffer at the current position.
 *
 * \param[in] b The data byte to write.
 */
void enc28j60_write_buffer_byte(uint8_t b)
{
    enc28j60_select();
    enc28j60_wbm();

    spi_send_byte(b);
    
    enc28j60_deselect();
}

/**
 * \internal
 * Reads multiple bytes from the RAM buffer.
 *
 * \param[out] buffer A pointer to the buffer which receives the data.
 * \param[in] buffer_len The buffer length and number of bytes to read.
 */
void enc28j60_read_buffer(uint8_t* buffer, uint16_t buffer_len)
{
    enc28j60_select();
    enc28j60_rbm();

    spi_rec_data(buffer, buffer_len);
    
    enc28j60_deselect();
}

/**
 * \internal
 * Writes multiple bytes to the RAM buffer.
 *
 * \param[in] buffer A pointer to the buffer containing the data to write.
 * \param[in] buffer_len The number of bytes to write.
 */
void enc28j60_write_buffer(uint8_t* buffer, uint16_t buffer_len)
{
    enc28j60_select();
    enc28j60_wbm();

    spi_send_data(buffer, buffer_len);
    
    enc28j60_deselect();
}

/**
 * Switches the hardware register bank.
 *
 * \param[in] num The index of the register bank to switch to.
 */
void enc28j60_bank(uint8_t num)
{
    static uint8_t bank = 0xff;

    if(num == bank)
        return;
    
    /* clear bank bits */
    enc28j60_select();
    enc28j60_bfc(ECON1);
    spi_send_byte(0x03);
    enc28j60_deselect();

    /* set bank bits */
    enc28j60_select();
    enc28j60_bfs(ECON1);
    spi_send_byte(num);
    enc28j60_deselect();

    bank = num;
}


#if 1
#include <board.h>

struct rt_semaphore enc28j60_spi_free;

/*
 * isenable: DISABLE, ENABLE
 */
void enc28j60_nvic_cfg(FunctionalState isenable)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	/* ‘ –ÌENC28J60÷–∂œ */
	NVIC_InitStructure.NVIC_IRQChannel = ENC28J60_INT_NUM;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = isenable;
	NVIC_Init(&NVIC_InitStructure);

	return;
}

#endif

#if 1 //def RT_USING_FINSH
/*
 * Debug routine to dump useful register contents
 */
/*
 * DS80349C.errata #8: For silicon revisions, B1 and B4, use a 2.7K, 1% external resistor between the RBIAS pin and
 * ground. The value shown in the data sheet(2.32K) is correct for revisions B5 and B7.
 */
static void enc28j60(void)
{
	rt_err_t result;

	result = rt_sem_take(&enc28j60_spi_free, get_ticks_of_ms(2000));
	if (result != RT_EOK) {
		printf_syn("%s() get enc28j60_spi_free fail!\n", __FUNCTION__);
		return;
	}

	rt_kprintf("-- enc28j60 registers:\n");
	rt_kprintf("HwRevID(B1-0x02, B4-0x04, B5-0x05, B7-0x06): 0x%02x\n", enc28j60_read(EREVID));
	rt_kprintf("Cntrl: ECON1 ECON2 ESTAT  EIR  EIE\n");
	rt_kprintf("       0x%02x  0x%02x  0x%02x  0x%02x  0x%02x\n",enc28j60_read(ECON1),
			enc28j60_read(ECON2), enc28j60_read(ESTAT), enc28j60_read(EIR), enc28j60_read(EIE));
	rt_kprintf("MAC  : MACON1 MACON3 MACON4\n");
	rt_kprintf("       0x%02x   0x%02x   0x%02x\n", enc28j60_read(MACON1), enc28j60_read(MACON3),
			enc28j60_read(MACON4));
	rt_kprintf("Rx   : ERXST  ERXND  ERXWRPT ERXRDPT ERXFCON EPKTCNT MAMXFL\n");
	rt_kprintf("       0x%04x 0x%04x 0x%04x  0x%04x  ", (enc28j60_read(ERXSTH) << 8) | enc28j60_read(ERXSTL),
		(enc28j60_read(ERXNDH) << 8) | enc28j60_read(ERXNDL),
		(enc28j60_read(ERXWRPTH) << 8) | enc28j60_read(ERXWRPTL),
		(enc28j60_read(ERXRDPTH) << 8) | enc28j60_read(ERXRDPTL));
	rt_kprintf("0x%02x    0x%02x    0x%04x\n", enc28j60_read(ERXFCON), enc28j60_read(EPKTCNT),
		(enc28j60_read(MAMXFLH) << 8) | enc28j60_read(MAMXFLL));

	rt_kprintf("Tx   : ETXST  ETXND  MACLCON1 MACLCON2 MAPHSUP\n");
	rt_kprintf("       0x%04x 0x%04x 0x%02x     0x%02x     0x%02x\n",
		(enc28j60_read(ETXSTH) << 8) | enc28j60_read(ETXSTL),
		(enc28j60_read(ETXNDH) << 8) | enc28j60_read(ETXNDL),
		enc28j60_read(MACLCON1), enc28j60_read(MACLCON2), enc28j60_read(MAPHSUP));

	rt_sem_release(&enc28j60_spi_free);
	return;
}
#include <finsh.h>
FINSH_FUNCTION_EXPORT(enc28j60, dump enc28j60 registers);
#endif



/**
 * @}
 * @}
 * @}
 */

