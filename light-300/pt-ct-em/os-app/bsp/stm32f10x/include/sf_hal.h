/*
 * sf_hal.h
 *
 * 2013-09-13,  creat by David, zhaoshaowei@yeejoin.com
 */

#ifndef SF_HAL_H_
#define SF_HAL_H_

/*
 * spi flash cmd and param marco define
 * 
 */

#if 0 /* sst25命令 */
#define SST25_CMD_READ		0X03	/* read, 25MHz */
#define SST25_CMD_HS_READ	0X0b	/* high-speed read, 80MHz */
#define SST25_CMD_4KB_SE	0X20	/* 4KB sector-erase */
#define SST25_CMD_32KB_BE    0X52	/* 32KB block-erase */
#define SST25_CMD_64KB_BE    0Xd8	/* 64KB block-erase */
#define SST25_CMD_CE         0X60	/* chip-erase */
#define SST25_CMD_BYTE_P     0X02	/* byte-program */
#define SST25_CMD_AAI_W_P    0Xad	/* AAI-word-program */
#define SST25_CMD_RDSR       0X05	/* read status register */
#define SST25_CMD_EWSR       0X50	/* enable write status register */
#define SST25_CMD_WRSR       0X01	/* write status register */
#define SST25_CMD_WREN       0X06	/* write enable */
#define SST25_CMD_WRDI       0X04	/* write disable */
#define SST25_CMD_RDID       0X90	/* read ID */
#define SST25_CMD_JEDEC_ID   0X9f	/* JEDEC ID read */
#define SST25_CMD_EBSY       0X70	/* Enable SO to output RY/BY# status during AAI program-ming */
#define SST25_CMD_DBSY       0X80	/* Disable SO as RY/BY# status during AAI program-ming */

#define SST_ID		0xBF
#define SST_SPI_FLASH	0X25
#define SST_25VF016B	0X41

#define SST25_VF016B_SECTOR_SIZE	(4*1024)
#define SST25_VF016B_SECTOR_ADDR_MASK	(SST25_VF016B_SECTOR_SIZE-1)
#define SST25_VF016B_SIZE (2*1024*1024)
#endif

/* 以下命令以sst25的命令为模板进行修改 */
#if 0 /* sst25 */


#else /* winbond w25 */
#define SF_CMD_READ		0X03	/* read, 50MHz */
#define SF_CMD_HS_READ		0X0b	/* high-speed read, 100MHz */
#define SF_CMD_4KB_SE		0X20	/* 4KB sector-erase */
#define SF_CMD_32KB_BE          0X52	/* 32KB block-erase */
#define SF_CMD_64KB_BE          0Xd8	/* 64KB block-erase */
#define SF_CMD_CE               0X60	/* chip-erase, 0x60/0xc7 */
#define SF_CMD_RDSR             0X05	/* read status register */
#define SF_CMD_WRSR             0X01	/* write status register */
#define SF_CMD_WREN             0X06	/* write enable */
#define SF_CMD_WRDI             0X04	/* write disable */
#define SF_CMD_RDID             0X90	/* read ID */
#define SF_CMD_JEDEC_ID         0X9f	/* JEDEC ID read */

#define SF_CMD_WRITE_PAGE	0X02	/* page program */

#define SF_MANUFACTURE_ID	0xEF	/* 制造商ID, 0xEF--winbond */
#define SF_MEM_TYPE		0X30	/* 存储器类型, 0x30 -- w25x10bv/20bv/40bv */
#define SF_CAPACITY		0X13	/* 容量,  w25x10bv/20bv/40bv -- 0x11/0x12/0x13 -- 1/2/4 Mbit */

#define SF_SECTOR_SIZE		(4*1024)
#define SF_SECTOR_ADDR_MASK	(SF_SECTOR_SIZE-1)
#define SF_SIZE			(512 * 1024)	/* unit by byte */

#define SF_PROGRAM_PAGE_SIZE	(256)

#endif


#define WRITE_FLASH_BY_PAGE 1
#define WRITE_FLASH_BY_WORD 0

#define BUSY_BIT_IN_SR	(1<<0)
#define is_sf_busy(sr) ((sr) & BUSY_BIT_IN_SR)

#define DUMMY_DATA4SF (0x00)


extern int  sf_read_data(unsigned long addr, unsigned char *buf, unsigned long len, int has_hold_sem);
extern int  sf_write_data(unsigned long addr, const unsigned char *buf, unsigned long len);
extern void sf_read_jedec_id(unsigned char *buf, int len);
extern void sf_set_prote_level_to_none(void);
//extern int  sf_read_sr(void);

extern void flush_sfwb(int has_hold_sem);


#endif
