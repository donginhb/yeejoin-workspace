/*
 * spi4ezmacpro.c
 *
 *  Created on: 2013-11-28
 *      Author: David, zhaoshaowei@yeejoin.com
 *
 *  该文件中的函数, 主要参考'EZMacPRO_v3.0.1r/bsp/spi.c'
 */

#include <rtdef.h>

#include <spi4ezmacpro.h>
#include <stm32_cpu_comm.h>
#include <board.h>
#include <si4432_v2.h>
#include <si4432_hardif.h>

#define SI4432_SPI_DUMMY_DATA	0X00
#define si4432_select()		gpio_bits_reset(SI4432_CS_PORT, SI4432_CS_PIN)
#define si4432_deselect()	gpio_bits_set(SI4432_CS_PORT, SI4432_CS_PIN)

#if ADE7880_SI4432_SHARE_SPI
int si4432_get_spi(void)
{
	rt_sem_take(&spi_sem_share_by_ade7880, RT_WAITING_FOREVER);
	init_si4432_spi_pin();
	cfg_si4432_spi_param();

	return 0;

}

int si4432_release_spi(void)
{
	SPI_Cmd(SI4432_SPIX, DISABLE);
	rt_sem_release(&spi_sem_share_by_ade7880);

	return 0;
}
#endif

/*
 * ***********************************************************************************************
 * spi Functions for EZMacPro.c module
 * ***********************************************************************************************
 */

/*
 *  Notes:
 ＊
 ＊ The spi functions in this module are for use in the main thread. The EZMacPro API calls should
 ＊ only be used in the main thread. The SPI is used by the main thread, as well as the external
 ＊ interrupt INT0 thread, and the T0 interrupt. Since all SPI tranfers are multiple bytes. It is
 ＊ important that MAC interrupts are disabled when using the SPI from the main thread.
 ＊
 ＊ These SPI functions may be interrupted by other interrupts, so the double buffered transfers
 ＊ are managed with this in mind.
 ＊
 ＊ The double buffered transfer maximizes the data throughput and eliminates any software
 ＊ delay between bytes. The clock is continuous for the two byte transfer. Instead of using the
 ＊ SPIF flag for each byte, the TXBMT is used to keep the transmit buffer full, then the SPIBSY
 ＊ bit is used to determine when all bits have been transfered. The SPIF flag should not be
 ＊ polled in double buffered transfers.
 */

/*------------------------------------------------------------------------------------------------
 * Function Name: macSpiWriteReg()
 * 	Write a register of the radio.
 * Return Values: None
 * Parameters	 :	U8 reg - register address from the si4432.h file.
 *    			U8 value - value to write to register
 * Notes:
 *    MAC interrupts are preserved and restored.
 * -----------------------------------------------------------------------------------------------
 */
void macSpiWriteReg (U8 reg, U8 value)
{
	base_t flag;

	flag = disable_si4432_mac_int();
	send_byte_to_si4432(reg, value);
	restore_si4432_mac_int(flag);

	return;
}

/*------------------------------------------------------------------------------------------------
 * Function Name: macSpiReadReg()
 *		Read a register from the radio.
 *
 * Return Value : U8 value - value returned from the si4432 register
 * Parameters   : U8 reg - register address from the si4432.h file.
 *
 * Notes:
 *     MAC interrupts are preserved and restored.
 *-----------------------------------------------------------------------------------------------
 */
U8 macSpiReadReg (U8 reg)
{
	U8 value;
	base_t flag;

	flag = disable_si4432_mac_int();
	value = read_byte_from_si4432(reg);
	restore_si4432_mac_int(flag);

	return value;
}

/*------------------------------------------------------------------------------------------------
 * Function Name: macSpiWriteFIFO()
 * 	Write the FIFO of the radio.
 *
 * Return Value : None
 * Parameters   :	n - the length of trasnmitted bytes
 * 		buffer - the transmitted bytes
 *
 * Notes:
 *     MAC interrupts are preserved and restored.
 * -----------------------------------------------------------------------------------------------
 */
void macSpiWriteFIFO (U8 n, VARIABLE_SEGMENT_POINTER(buffer, U8, BUFFER_MSPACE))
{
	base_t flag;

	flag = disable_si4432_mac_int();
	send_data_to_si4432_fifo(n, buffer);
	restore_si4432_mac_int(flag);

	return;
}



/*
 * ***********************************************************************************************
 * spi Functions for externalInt.c module
 *
 * 从函数实现来看, 用于externalInt.c的spi函数与用于EZMacPro.c的spi函数的区别是，少了对 EX0 & ET0的中断屏蔽处理
 * ***********************************************************************************************
 */

/*
 * Notes:
 *
 * The spi functions in this module are for use in the externalInt thread. The SPI is used by the
 * main thread, as well as the external interrupt INT0 thread, and the T0 interrupt. The SPI
 * functions are duplicated for the externalInt module. If the same SPI functions were used, the
 * linker would generate a multiple call to segment warning. The linker would not be able to
 * overlay the threads separately, local data may be corrupted be a reentrant function call,
 * the SPI transfer may be corrupted.
 *
 * These SPI functions may be interrupted by a high priority interrupt, so the double buffered
 * transfers are managed with this in mind.
 *
 * The double buffered transfer maximizes the data throughput and eliminates any software
 * delay between bytes. The clock is continuous for the two byte transfer. Instead of using the
 * SPIF flag for each byte, the TXBMT is used to keep the transmit buffer full, then the SPIBSY
 * bit is used to determine when all bits have been transferred. The SPIF flag should not be
 * polled in double buffered transfers.
 *
 */

/*------------------------------------------------------------------------------------------------
 * Function Name
 *    extIntSpiWriteReg()
 *
 * Return Value   : None
 * Parameters :
 *    U8 reg - register address from the si4432.h file.
 *    U8 value - value to write to register
 *
 * Notes:
 *    Write uses a Double buffered transfer.
 * -----------------------------------------------------------------------------------------------
 */
#if !USE_MACRO_FUN_IN_SPI4EZMACPRO
void extIntSpiWriteReg (U8 macReg, U8 value)
{
	send_byte_to_si4432(macReg, value);
	return;
}
#endif

/*------------------------------------------------------------------------------------------------
 * Function Name
 *    extIntSpiReadReg()
 *
 * Return Value : U8 value - value returned from the si4432 register
 * Parameters   : U8 reg - register address from the si4432.h file.
 *
 * Notes:
 *    Read uses a Double buffered transfer.
 *-----------------------------------------------------------------------------------------------
 */
#if !USE_MACRO_FUN_IN_SPI4EZMACPRO
U8 extIntSpiReadReg (U8 macReg)
{
	return read_byte_from_si4432(macReg);
}
#endif


/*------------------------------------------------------------------------------------------------
 * Function Name
 *    extIntSpiWriteFIFO()
 *
 * Return Value : None
 * Parameters   :
 *    U8 n - the number of bytes to be written
 *    *buffer - pointer to address of write buffer
 *
 * Notes:
 *    Write FIFO uses Double buffered transfers.
 *    The WriteFIFO function is only included if packet forwarding is defined.
 *    Packet forwarding requires writing the forward packet back to the TX buffer.
 *-----------------------------------------------------------------------------------------------
 */
#ifdef PACKET_FORWARDING_SUPPORTED
#if !USE_MACRO_FUN_IN_SPI4EZMACPRO
void extIntSpiWriteFIFO (U8 n, VARIABLE_SEGMENT_POINTER(buffer, U8, BUFFER_MSPACE))
{
	send_data_to_si4432_fifo(n, buffer);
	return;
}
#endif
#endif


/*------------------------------------------------------------------------------------------------
 * Function Name
 *    extIntSpiReadFIFO()
 *
 * Return Value : None
 * Parameters   :
 *
 * Notes:
 *    This function does not use double buffered data transfers to prevent data loss.
 *    This function may be interrupted by another process and should not loose data
 *    or hang on the SPIF flag.
 *
 *    This function is not included for the Transmitter only configuration.
 *-----------------------------------------------------------------------------------------------
 */
#ifndef TRANSMITTER_ONLY_OPERATION
#if !USE_MACRO_FUN_IN_SPI4EZMACPRO
void extIntSpiReadFIFO (U8 n, VARIABLE_SEGMENT_POINTER(buffer, U8, BUFFER_MSPACE))
{
	read_data_from_si4432_fifo(n, buffer);
	return;
}
#endif
#endif // TRANSMITTER_ONLY_OPERATION not defined




/*
 * ***********************************************************************************************
 * spi Functions for timerInt.c module
 *
 * 从函数实现来看, 用于externalInt.c的spi函数与用于EZMacPro.c的spi函数的区别是，少了对 EX0 & ET0的中断屏蔽处理
 * ***********************************************************************************************
 */

/*
 * Notes:
 *
 * The spi functions in this module are for use in the timerInt thread. The SPI is used by the
 * main thread, as well as the external interrupt INT0 thread, and the T0 interrupt. The SPI
 * functions are duplicated for the timerInt module. If the same SPI functions were used, the
 * linker would generate a multiple call to segment warning. The linker would not be able to
 * overlay the threads separately, local data may be corrupted be a reentrant function call,
 * the SPI transfer may be corrupted.
 *
 * These SPI functions may be interrupted by a high priority interrupt, so the double buffered
 * transfers are managed with this in mind.
 *
 * The double buffered transfer maximizes the data throughput and eliminates any software
 * delay between bytes. The clock is continuous for the two byte transfer. Instead of using the
 * SPIF flag for each byte, the TXBMT is used to keep the transmit buffer full, then the SPIBSY
 * bit is used to determine when all bits have been transferred. The SPIF flag should not be
 * polled in double buffered transfers.
 */

/*------------------------------------------------------------------------------------------------
 * Function Name
 *    timerIntSpiWriteReg()
 *
 * Return Value   : None
 * Parameters :
 *    U8 reg - register address from the si4432.h file.
 *    U8 value - value to write to register
 *
 * Notes:
 *    Write uses a Double buffered transfer.
 *    This function is not included in the Transmitter only configuration.
 *-----------------------------------------------------------------------------------------------
 */

#if !USE_MACRO_FUN_IN_SPI4EZMACPRO
void timerIntSpiWriteReg (U8 macReg, U8 value)
{
	send_byte_to_si4432(macReg, value);
	return;
}
#endif



/*------------------------------------------------------------------------------------------------
 * Function Name
 *    timerIntSpiReadReg()
 *
 * Return Value : U8 value - value returned from the si4432 register
 * Parameters   : U8 reg - register address from the si4432.h file.
 *
 * Notes:
 *    Read uses a Double buffered transfer.
 *    This function is not included for the Transmitter only configuration.
 *-----------------------------------------------------------------------------------------------
 */
#if !USE_MACRO_FUN_IN_SPI4EZMACPRO
U8 timerIntSpiReadReg (U8 macReg)
{
	return read_byte_from_si4432(macReg);
}
#endif






/*
 * common spi function for si4432
 *
 * */
void send_byte_to_si4432 (U8 reg, U8 value)
{
#if ADE7880_SI4432_SHARE_SPI
	si4432_get_spi();
#endif
	si4432_select();
	spix_send_byte(SI4432_SPIX, reg | 0x80);	// write reg address
	spix_send_byte(SI4432_SPIX, value);
	si4432_deselect();
#if ADE7880_SI4432_SHARE_SPI
	si4432_release_spi();
#endif

	return;
}




U8 read_byte_from_si4432(U8 reg)
{
	U8 value;

#if ADE7880_SI4432_SHARE_SPI
	si4432_get_spi();
#endif

	si4432_select();
	spix_send_byte(SI4432_SPIX, reg);	// write reg address
	value = spix_rec_byte(SI4432_SPIX, SI4432_SPI_DUMMY_DATA);
	si4432_deselect();
#if ADE7880_SI4432_SHARE_SPI
	si4432_release_spi();
#endif

	return value;
}



void send_data_to_si4432_fifo(U8 n, VARIABLE_SEGMENT_POINTER(buffer, U8, BUFFER_MSPACE))
{
#if ADE7880_SI4432_SHARE_SPI
	si4432_get_spi();
#endif
	si4432_select();
	spix_send_byte(SI4432_SPIX, 0x80 | SI4432_FIFO_ACCESS);	// write reg address
	spix_send_data(SI4432_SPIX, buffer, n);
	si4432_deselect();
#if ADE7880_SI4432_SHARE_SPI
	si4432_release_spi();
#endif

	return;
}

void read_data_from_si4432_fifo(U8 n, VARIABLE_SEGMENT_POINTER(buffer, U8, BUFFER_MSPACE))
{
#if ADE7880_SI4432_SHARE_SPI
	si4432_get_spi();
#endif
	si4432_select();
	spix_send_byte(SI4432_SPIX, SI4432_FIFO_ACCESS);	// write reg address
	spix_rec_data(SI4432_SPIX, buffer, n, SI4432_SPI_DUMMY_DATA);
	si4432_deselect();
#if ADE7880_SI4432_SHARE_SPI
	si4432_release_spi();
#endif

	return;
}
