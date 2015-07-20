/*
 * spiflash.c
 *
 * spi-flash Driver
 *
 * 2012-12-26,  creat by David, zhaoshaowei@yeejoin.com
 */

#include <rtthread.h>
#include <dfs_fs.h>
#include <integer.h>
#include <diskio.h>

#include "spiflash.h"


static struct rt_device spiflash_device;
static struct dfs_partition part;


/* RT-Thread Device Driver Interface */
static rt_err_t rt_spiflash_init(rt_device_t dev)
{
	return RT_EOK;
}

static rt_err_t rt_spiflash_open(rt_device_t dev, rt_uint16_t oflag)
{
	return RT_EOK;
}

static rt_err_t rt_spiflash_close(rt_device_t dev)
{
	return RT_EOK;
}

static rt_size_t rt_spiflash_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
	// rt_kprintf("write: 0x%x, size %d\n", pos, size);

	sst25_read_data((part.offset + pos)*RT_DFS_ELM_MAX_SECTOR_SIZE, buffer, size*RT_DFS_ELM_MAX_SECTOR_SIZE, 0);

	return size;
}

static rt_size_t rt_spiflash_write (rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
	// rt_kprintf("write: 0x%x, size %d\n", pos, size);

	sst25_write_data((part.offset + pos)*RT_DFS_ELM_MAX_SECTOR_SIZE, buffer, size*RT_DFS_ELM_MAX_SECTOR_SIZE);

	return size;
}

static rt_err_t rt_spiflash_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
	struct rt_device_blk_geometry *geometry;
	int ret = RT_EOK;

	RT_ASSERT(dev != RT_NULL);
#if 1
	switch (cmd) {
	case RT_DEVICE_CTRL_BLK_GETGEOME:
		geometry = (struct rt_device_blk_geometry *)args;
		if (geometry == RT_NULL) return -RT_ERROR;

		geometry->bytes_per_sector 	= SPI_FLASH_SECTOR_SIZE;
		geometry->block_size 		= SPI_FLASH_BLOCK_SIZE;
		geometry->sector_count 		= SPI_FLASH_SIZE / SPI_FLASH_SECTOR_SIZE;
		break;

	case RT_DEVICE_CTRL_BLK_SYNC:
		flush_sfwb(0); /* mark by David */
		break;

	default:
		printf_syn("spi-flash-ctrl cmd(%d) invalid\r\n", cmd);
		ret = -RT_ERROR;
		break;
	}
#else
	switch (cmd) {
	case GET_SECTOR_COUNT:
		*(DWORD *)args = SPI_FLASH_SIZE / SPI_FLASH_SECTOR_SIZE;
		break;

	case GET_SECTOR_SIZE:
		*(WORD *)args = SPI_FLASH_SECTOR_SIZE;
		break;

	case GET_BLOCK_SIZE:
		*(DWORD *)args = SPI_FLASH_BLOCK_SIZE;
		break;

	default:
		printf_syn("spi-flash-ctrl cmd(%d) invalid\r\n", cmd);
		break;
	}
#endif
	return ret;
}

void rt_hw_spiflash_init()
{
	unsigned char buf[4];
	rt_uint8_t *sector;
	rt_uint8_t status;

	sst25_read_jedec_id(buf, sizeof(buf));
	if (0xbf!=buf[0] || 0x25!=buf[1] || 0x41!=buf[2]) {
		rt_kprintf("there is no sst25vf016b. ID:0x%x,0x%x,0x%x\n", buf[0], buf[1], buf[2]);
		//return;
	}

	/* register spi-flash device */
	spiflash_device.type  = RT_Device_Class_Block;
	spiflash_device.init 	= rt_spiflash_init;
	spiflash_device.open 	= rt_spiflash_open;
	spiflash_device.close   = rt_spiflash_close;
	spiflash_device.read 	= rt_spiflash_read;
	spiflash_device.write   = rt_spiflash_write;
	spiflash_device.control = rt_spiflash_control;
	spiflash_device.user_data = RT_NULL; /* no private */

	/* get the first sector to read partition table */
	sector = (rt_uint8_t*) rt_malloc (512);
	if (sector == RT_NULL) {
		rt_kprintf("allocate partition sector buffer failed\n");
		return;
	}

	sst25_read_data(0, sector, 512, 0);
	/* get the first partition */
	status = dfs_filesystem_get_partition(&part, sector, 0);
	if (status != RT_EOK) {
		/* there is no partition table */
		part.offset = 0;
		part.size   = 0;
	}

	rt_free(sector);

	rt_device_register(&spiflash_device, "sf0",
			RT_DEVICE_FLAG_RDWR | \
			RT_DEVICE_FLAG_REMOVABLE | \
			RT_DEVICE_FLAG_STANDALONE);

	return;
}
