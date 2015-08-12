/*
 * sst25.c
 *
 * 2012-10-24,  creat by David, zhaoshaowei@yeejoin.com
 */

#ifndef SST25_H_

#define SST25_H_

#define SST25_CMD_READ			0X03	/* read, 25MHz */
#define SST25_CMD_HS_READ		0X0b	/* high-speed read, 80MHz */
#define SST25_CMD_4KB_SE		0X20	/* 4KB sector-erase */
#define SST25_CMD_32KB_BE               0X52	/* 32KB block-erase */
#define SST25_CMD_64KB_BE               0Xd8	/* 64KB block-erase */
#define SST25_CMD_CE                    0X60	/* chip-erase */
#define SST25_CMD_BYTE_P                0X02	/* byte-program */
#define SST25_CMD_AAI_W_P               0Xad	/* AAI-word-program */
#define SST25_CMD_RDSR                  0X05	/* read status register */
#define SST25_CMD_EWSR                  0X50	/* enable write status register */
#define SST25_CMD_WRSR                  0X01	/* write status register */
#define SST25_CMD_WREN                  0X06	/* write enable */
#define SST25_CMD_WRDI                  0X04	/* write disable */
#define SST25_CMD_RDID                  0X90	/* read ID */
#define SST25_CMD_JEDEC_ID              0X9f	/* JEDEC ID read */
#define SST25_CMD_EBSY                  0X70	/* Enable SO to output RY/BY# status during AAI program-ming */
#define SST25_CMD_DBSY                  0X80	/* Disable SO as RY/BY# status during AAI program-ming */


#define SST_ID		0xBF
#define SST_SPI_FLASH	0X25
#define SST_25VF016B	0X41

#define SST25_VF016B_SECTOR_SIZE	(4*1024)
#define SST25_VF016B_SECTOR_ADDR_MASK	(SST25_VF016B_SECTOR_SIZE-1)
#define SST25_VF016B_SIZE (2*1024*1024)

extern int sst25_read_data(unsigned long addr, unsigned char *buf, unsigned long len, int has_hold_sem);
extern int sst25_write_data_by_sector(unsigned long addr, const unsigned char *buf, unsigned long len);
extern int sst25_write_data(unsigned long addr, const unsigned char *buf, unsigned long len);
extern void sst25_read_jedec_id(unsigned char *buf, int len);
extern void sst25_set_prote_level_to_none(void);
extern int sst25_read_sr(void);

extern void flush_sfwb(int has_hold_sem);


#endif
