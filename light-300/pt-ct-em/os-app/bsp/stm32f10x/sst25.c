/*
 * sst25.c
 *
 * 2012-10-24,  creat by David, zhaoshaowei@yeejoin.com
 */
#include <stm32f10x.h>
#include <board.h>
#include <sst25.h>

#define DUMMY_DATA4SST25 (0x00)

#define sst25_select()		gpio_bits_reset(SST25_CS_PORT, SST25_CS_PIN)
#define sst25_deselect()	gpio_bits_set(SST25_CS_PORT, SST25_CS_PIN)

#define sst25_spi_send_byte(writedat) 	spix_send_byte(SPI1, writedat)
#define sst25_spi_rec_byte()  		spix_rec_byte(SPI1, DUMMY_DATA4SST25)
#define sst25_spi_send_data(data, len) 	spix_send_data(SPI1, data, len)
#define sst25_spi_rec_data(buf, len)   	spix_rec_data(SPI1, buf, len, DUMMY_DATA4SST25)

#define SST25_DEBUG(x) /* printf_syn x */

static void sst25_wait_nobusy(void);


/* spi1 clock is 36MHz */
int sst25_read_data(unsigned long addr, unsigned char *buf, unsigned long len, int has_hold_sem)
{
	unsigned char cmd[8];

	if (NULL==buf)
		return -1;

	//flush_sfwb(has_hold_sem); /* 临时简单处理 */

	cmd[0] = SST25_CMD_HS_READ;
	cmd[1] = (addr>>16) & 0xff;
	cmd[2] = (addr>>8)  & 0xff;
	cmd[3] = (addr)     & 0xff;
	cmd[4] = 0;
	sst25_select();
	sst25_spi_send_data(cmd, 5);
	sst25_spi_rec_data(buf, len);
	sst25_deselect();

	return 0;
}

int sst25_write_data_by_sector(unsigned long addr, const unsigned char *buf, unsigned long len)
{
	unsigned char cmd[8];
	unsigned long i;
	unsigned long cnt;

	if (0!=(addr & SST25_VF016B_SECTOR_ADDR_MASK) || len > 4*1024)
		return 1;

	if (0==len || NULL==buf)
		return 2;

	sst25_select();
	sst25_spi_send_byte(SST25_CMD_WREN);
	sst25_deselect();

	cmd[0] = SST25_CMD_4KB_SE;
	cmd[1] = (addr>>16) & 0xff;
	cmd[2] = (addr>>8)  & 0xff;
	cmd[3] = (addr)     & 0xff;
	sst25_select();
	sst25_spi_send_data(cmd, 4);
	sst25_deselect();

	sst25_wait_nobusy(); /* 扇区擦除已完成 */

	SST25_DEBUG(("%s() line:%d, len:%d!\n", __FUNCTION__, __LINE__, len));

	sst25_select();
	sst25_spi_send_byte(SST25_CMD_WREN);
	sst25_deselect();

	cmd[0] = SST25_CMD_AAI_W_P;
	cmd[1] = (addr>>16) & 0xff;
	cmd[2] = (addr>>8)  & 0xff;
	cmd[3] = (addr)     & 0xff;
	cmd[4] = buf[0];
	if (len <= 2) {
		if (1 == len)
			cmd[5] = 0xff;
		else
			cmd[5] = buf[1];
		sst25_select();
		sst25_spi_send_data(cmd, 6);
		sst25_deselect();
	} else {
		cmd[5] = buf[1];
		sst25_select();
		sst25_spi_send_data(cmd, 6);
		sst25_deselect();

		SST25_DEBUG(("%s() line:%d, data:%d, %d!\n", __FUNCTION__, __LINE__, cmd[4], cmd[5]));

		cnt = len & (~1);
		for (i=2; i<cnt; i+=2) {
			cmd[1] = buf[i];
			cmd[2] = buf[i+1];
			SST25_DEBUG(("%s() line:%d, data:%d, %d!\n", __FUNCTION__, __LINE__, cmd[1], cmd[2]));

			sst25_wait_nobusy();
			sst25_select();
			sst25_spi_send_data(cmd, 3);
			sst25_deselect();
		}

		if (len & 0x01) {
			cmd[1] = buf[i];
			cmd[2] = 0xff;
			SST25_DEBUG(("%s() line:%d, data:%d, %d!\n", __FUNCTION__, __LINE__, cmd[1], cmd[2]));

			sst25_wait_nobusy();
			sst25_select();
			sst25_spi_send_data(cmd, 3);
			sst25_deselect();
		}
	}

	sst25_wait_nobusy();
	sst25_select();
	sst25_spi_send_byte(SST25_CMD_WRDI);
	sst25_deselect();

	return 0;
}

void sst25_read_jedec_id(unsigned char *buf, int len)
{
	if (NULL==buf || len < 3)
		return;

	sst25_select();
	sst25_spi_send_byte(SST25_CMD_JEDEC_ID);
	sst25_spi_rec_data(buf, 3);
	sst25_deselect();

	return;
}

void sst25_set_prote_level_to_none(void)
{
	unsigned char buf[4];

	sst25_select();
	sst25_spi_send_byte(SST25_CMD_EWSR);
	sst25_deselect();

	buf[0] = SST25_CMD_WRSR;
	buf[1] = 0x00;
	sst25_select();
	sst25_spi_send_data(buf, 2);
	sst25_deselect();

	return;
}

int sst25_read_sr(void)
{
	char ret;

	sst25_select();
	sst25_spi_send_byte(SST25_CMD_RDSR);
	ret = sst25_spi_rec_byte();
	sst25_deselect();

	return ret;
}

static void sst25_wait_nobusy(void)
{
	char ret;

	do {
		ret = sst25_read_sr();
	} while (ret & 0x01);

	SST25_DEBUG(("%s() line:%d, SR:0x%x!\n", __FUNCTION__, __LINE__, ret));
	return;
}

#if 1
enum spi_flash_wb_state_e {
	SFWBS_EMPTY  = 0,
	SFWBS_UPDATE = 1,
	SFWBS_DIRTY  = 2,
};

struct spi_flash_write_buf_t {
	volatile unsigned long addr;
	volatile int state; /* 0-empty, 1-update, 2-dirty */
	unsigned char buf[SST25_VF016B_SECTOR_SIZE];
};

static struct spi_flash_write_buf_t sst_buf;

struct rt_semaphore spiflash_sem;
/*
volatile int sst25_w_cnt, sst25_w_bytes;
volatile unsigned long sst25_w_addr;
*/
int sst25_write_data(unsigned long addr, const unsigned char *buf, unsigned long len)
{
	unsigned long h_sec_addr, l_sec_addr, offset;
	unsigned long llen, cnt;

	if (0==len)
		return 0;

	if (NULL == buf)
		return -2;
	/*++sst25_w_cnt;
	//sst25_w_bytes += len;
	//sst25_w_addr = addr;
	*/
	llen     = len;
	rt_sem_take(&spiflash_sem, RT_WAITING_FOREVER);

	l_sec_addr = addr & (~SST25_VF016B_SECTOR_ADDR_MASK);
	h_sec_addr = (addr + SST25_VF016B_SECTOR_ADDR_MASK) & (~SST25_VF016B_SECTOR_ADDR_MASK);
	if (SFWBS_EMPTY == sst_buf.state) {
		sst25_read_data(l_sec_addr, sst_buf.buf, sizeof(sst_buf.buf), 1);
		sst_buf.state = SFWBS_UPDATE;
		sst_buf.addr  = l_sec_addr;
	}

	/* 起始的不对齐数据 */
	offset = addr & SST25_VF016B_SECTOR_ADDR_MASK;
	if (0 != offset) {
		if (l_sec_addr != sst_buf.addr) {
			if (SFWBS_DIRTY == sst_buf.state)
				sst25_write_data_by_sector(sst_buf.addr, sst_buf.buf, sizeof(sst_buf.buf));

			sst25_read_data(l_sec_addr, sst_buf.buf, sizeof(sst_buf.buf), 1);
			sst_buf.addr  = l_sec_addr;
			sst_buf.state = SFWBS_UPDATE;
		}

		cnt = MIN(len, h_sec_addr - addr);
		rt_memcpy(sst_buf.buf+offset, buf, cnt);
		if (offset+cnt >= SST25_VF016B_SECTOR_SIZE) {
			sst25_write_data_by_sector(sst_buf.addr, sst_buf.buf, sizeof(sst_buf.buf));
			sst_buf.state = SFWBS_UPDATE;
		} else {
			sst_buf.state = SFWBS_DIRTY;
		}

		len -= cnt;
		buf += cnt;
	}

	if (0 == len)
		goto w_over;

	/* 对齐的整扇区数据 */
	while (len >= SST25_VF016B_SECTOR_SIZE) {
		sst25_write_data_by_sector(h_sec_addr, buf, SST25_VF016B_SECTOR_SIZE);
		h_sec_addr += SST25_VF016B_SECTOR_SIZE;
		buf		 += SST25_VF016B_SECTOR_SIZE;
		len		 -= SST25_VF016B_SECTOR_SIZE;
	}

	if (0 == len)
		goto w_over;

	/* 不足一个扇区的对齐数据 */
	if (h_sec_addr != sst_buf.addr) {
		if (SFWBS_DIRTY == sst_buf.state)
			sst25_write_data_by_sector(sst_buf.addr, sst_buf.buf, sizeof(sst_buf.buf));

		sst25_read_data(h_sec_addr, sst_buf.buf, sizeof(sst_buf.buf), 1);
		sst_buf.addr = h_sec_addr;
	}

	rt_memcpy(sst_buf.buf, buf, len);
	sst_buf.state = SFWBS_DIRTY;

w_over:

	rt_sem_release(&spiflash_sem);

	return llen;
}

void flush_sfwb(int has_hold_sem)
{
	if (0 == has_hold_sem)
		rt_sem_take(&spiflash_sem, RT_WAITING_FOREVER);

	if (SFWBS_DIRTY == sst_buf.state) {
		sst25_write_data_by_sector(sst_buf.addr, sst_buf.buf, sizeof(sst_buf.buf));
		sst_buf.state = SFWBS_UPDATE;
	}

	if (0 == has_hold_sem)
		rt_sem_release(&spiflash_sem);

	return;

}
#endif
