#ifndef SPIFLASH_H__
#define SPIFLASH_H__

#include "sf_hal.h"


#define SPI_FLASH_SECTOR_SIZE	RT_DFS_ELM_MAX_SECTOR_SIZE

#define SPI_FLASH_SIZE 		SF_SIZE
#define SPI_FLASH_BLOCK_SIZE	SF_SECTOR_SIZE

void rt_hw_spiflash_init();

#endif
