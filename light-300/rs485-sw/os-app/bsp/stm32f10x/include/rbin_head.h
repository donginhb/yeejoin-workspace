#ifndef RBIN_HEAD_H_
#define RBIN_HEAD_H_

#define RBIN_FILE_MAGIC	0xa2c45d7b
#define REMOTE_UPDATE_COMPLETE_MAGIC 	0xe1

#define SPI_FLASH_ADDR_OF_RBIN_HEAD	(0)
#define SPI_FLASH_ADDR_OF_OSAPP		(SST25_VF016B_SECTOR_SIZE)

/* 不包含字符串结束标志 */
#define RBIN_FILE_NAME_MAX_LEN		63

enum rbin_data_status_e {
	RDS_INVAILD		= 0,
	RDS_BIN2RBIN		= 1,
	RDS_SPIF_HAD_CHECK	= 2,
	RDS_ONCHIP_HAD_CHECK	= 3,
};
/*
 * typedef long time_t
 * time_t
 * It is almost universally expected to be an integral value representing the
 * number of seconds elapsed since 00:00 hours, Jan 1, 1970 UTC. This is due to
 * historical reasons, since it corresponds to a unix timestamp, but is widely
 * implemented in C libraries across all platforms.
 *
 * 以网络序存储
 */
struct rbin_head_st {
	uint8_t  rh_md5[16];	/* bin file md5 digest */
	uint32_t rh_sw_ver;	/* 记录spi flash中os&app的版本号 */
	uint8_t  rbin_data_status;
	uint8_t  rh_update_succ;	/*  */
	uint8_t  rh_pad[2];

	char	 rh_rbin_fn[RBIN_FILE_NAME_MAX_LEN+1]; /* rbin file name, the space that not use pad with zero */
	uint32_t rh_rbin_size;	/* rbin file size in bytes */
	long	 rh_mtime;		/* Modified time */
	long	 rh_ctime;		/* Creation time([Linux] time of last status change) */

	uint32_t rh_magic;	/* 0xa2c45d7b */
};

#ifndef SUCC
#define SUCC 0
#endif

#ifndef FAIL
#define FAIL 1
#endif

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define EXT_MEM_USE_SST25  0
#define EXT_MEM_USE_SDCARD 1

#if EXT_MEM_USE_SDCARD
extern struct dfs_partition bootpart_info;
extern int sdcard_read_data4bootpart(unsigned long addr, unsigned char *buf, unsigned long len);
extern int sdcard_write_data_by_sector4bootpart(unsigned long addr, unsigned char *buf, unsigned long len);

#define SDCARD_SECTOR_SIZE	512

#define SDCARD_ADDR_OF_RBIN_HEAD	(bootpart_info.offset * SDCARD_SECTOR_SIZE)
#define SDCARD_ADDR_OF_OSAPP		((bootpart_info.offset + 1) * SDCARD_SECTOR_SIZE)
#endif


#if EXT_MEM_USE_SST25
#include <sst25.h>
#define EXT_MEM_SECTOR_SIZE		SST25_VF016B_SECTOR_SIZE

#define EXT_MEM_ADDR_OF_RBIN_HEAD	SPI_FLASH_ADDR_OF_RBIN_HEAD
#define EXT_MEM_ADDR_OF_OSAPP		SPI_FLASH_ADDR_OF_OSAPP

#define ext_mem_read_data		sst25_read_data
#define ext_mem_write_data_by_sector	sst25_write_data_by_sector
#endif

#if EXT_MEM_USE_SDCARD
#define EXT_MEM_SECTOR_SIZE		(2*SDCARD_SECTOR_SIZE)

#define EXT_MEM_ADDR_OF_RBIN_HEAD	SDCARD_ADDR_OF_RBIN_HEAD
#define EXT_MEM_ADDR_OF_OSAPP		SDCARD_ADDR_OF_OSAPP

#define ext_mem_read_data		sdcard_read_data4bootpart
#define ext_mem_write_data_by_sector	sdcard_write_data_by_sector4bootpart
#endif


#endif
