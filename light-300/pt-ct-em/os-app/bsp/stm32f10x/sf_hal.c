/*
 * sf_hal.c
 *
 * 2013-09-13,  creat by David, zhaoshaowei@yeejoin.com
 */
#include <stm32f10x.h>
#include <board.h>
#include <sf_hal.h>

/*
 * 与cpu相关
 */
#define SF_SPI	SF_SPIX
#define sf_select()	gpio_bits_reset(SF_CS_PORT, SF_CS_PIN)
#define sf_deselect()	gpio_bits_set(SF_CS_PORT, SF_CS_PIN)

#define sf_spi_send_byte(writedat) 	spix_send_byte(SF_SPI, writedat)
#define sf_spi_rec_byte()  		spix_rec_byte(SF_SPI, DUMMY_DATA4SF)
#define sf_spi_send_data(data, len) 	spix_send_data(SF_SPI, data, len)
#define sf_spi_rec_data(buf, len)   	spix_rec_data(SF_SPI, buf, len, DUMMY_DATA4SF)

#define SF_DEBUG(x) //printf_syn x

enum spi_flash_wb_state_e {
	SFWBS_EMPTY  = 0,
	SFWBS_UPDATE = 1,
	SFWBS_DIRTY  = 2,
};

struct spi_flash_write_buf_t {
	volatile unsigned long addr;
	volatile int state; /* 0-empty, 1-update, 2-dirty */
	unsigned char buf[SF_SECTOR_SIZE];
};

static struct spi_flash_write_buf_t sf_buf;

struct rt_semaphore spiflash_sem;


static void sf_wait_nobusy(void);
static int sf_read_sr(void);
static int  sf_write_data_by_sector(unsigned long addr, const unsigned char *buf, unsigned long len);

#if ADE7880_SPIFLASH_SHARE_SPI
extern void spiflash_spi_cfg(void);

int spi_flash_get_spi(void)
{
	rt_sem_take(&spi_sem_share_by_ade7880, RT_WAITING_FOREVER);
	spiflash_spi_cfg();

	return 0;
}

int spi_flash_release_spi(void)
{
	SPI_Cmd(ADE7880_SPIX, DISABLE);
	rt_sem_release(&spi_sem_share_by_ade7880);

	return 0;
}
#endif


#if 1
int sf_write_data(unsigned long addr, const unsigned char *buf, unsigned long len)
{
	unsigned long h_sec_addr, l_sec_addr, offset;
	unsigned long llen, cnt;

	if (0==len)
		return 0;

	if (NULL == buf)
		return -2;

	llen     = len;
	rt_sem_take(&spiflash_sem, RT_WAITING_FOREVER);
	spi_flash_get_spi();

	l_sec_addr = addr & (~SF_SECTOR_ADDR_MASK);
	h_sec_addr = (addr + SF_SECTOR_ADDR_MASK) & (~SF_SECTOR_ADDR_MASK);
	if (SFWBS_EMPTY == sf_buf.state) {
		sf_read_data(l_sec_addr, sf_buf.buf, sizeof(sf_buf.buf), 1);
		sf_buf.state = SFWBS_UPDATE;
		sf_buf.addr  = l_sec_addr;
	}

	/* 起始的不对齐数据 */
	offset = addr & SF_SECTOR_ADDR_MASK;
	if (0 != offset) {
		if (l_sec_addr != sf_buf.addr) {
			if (SFWBS_DIRTY == sf_buf.state)
				sf_write_data_by_sector(sf_buf.addr, sf_buf.buf, sizeof(sf_buf.buf));

			sf_read_data(l_sec_addr, sf_buf.buf, sizeof(sf_buf.buf), 1);
			sf_buf.addr  = l_sec_addr;
			sf_buf.state = SFWBS_UPDATE;
		}

		cnt = MIN(len, h_sec_addr - addr);
		rt_memcpy(sf_buf.buf+offset, buf, cnt);
		if (offset+cnt >= SF_SECTOR_SIZE) {
			sf_write_data_by_sector(sf_buf.addr, sf_buf.buf, sizeof(sf_buf.buf));
			sf_buf.state = SFWBS_UPDATE;
		} else {
			sf_buf.state = SFWBS_DIRTY;
		}

		len -= cnt;
		buf += cnt;
	}

	if (0 == len)
		goto w_over;

	/* 对齐的整扇区数据 */
	while (len >= SF_SECTOR_SIZE) {
		sf_write_data_by_sector(h_sec_addr, buf, SF_SECTOR_SIZE);
		h_sec_addr += SF_SECTOR_SIZE;
		buf		 += SF_SECTOR_SIZE;
		len		 -= SF_SECTOR_SIZE;
	}

	if (0 == len)
		goto w_over;

	/* 不足一个扇区的对齐数据 */
	if (h_sec_addr != sf_buf.addr) {
		if (SFWBS_DIRTY == sf_buf.state)
			sf_write_data_by_sector(sf_buf.addr, sf_buf.buf, sizeof(sf_buf.buf));

		sf_read_data(h_sec_addr, sf_buf.buf, sizeof(sf_buf.buf), 1);
		sf_buf.addr = h_sec_addr;
	}

	rt_memcpy(sf_buf.buf, buf, len);
	sf_buf.state = SFWBS_DIRTY;

w_over:
	spi_flash_release_spi();
	rt_sem_release(&spiflash_sem);

	return llen;
}

void flush_sfwb(int has_hold_sem)
{
	if (0 == has_hold_sem) {
		rt_sem_take(&spiflash_sem, RT_WAITING_FOREVER);
		spi_flash_get_spi();
	}

	if (SFWBS_DIRTY == sf_buf.state) {
		sf_write_data_by_sector(sf_buf.addr, sf_buf.buf, sizeof(sf_buf.buf));
		sf_buf.state = SFWBS_UPDATE;
	}

	if (0 == has_hold_sem) {
		spi_flash_release_spi();
		rt_sem_release(&spiflash_sem);
	}

	return;

}
#endif


/*
 * spi1 clock is 36MHz
 * spi3 clock is 18MHz
 */
int sf_read_data(unsigned long addr, unsigned char *buf, unsigned long len, int has_hold_sem)
{
	unsigned char cmd[8];
	unsigned long h_addr, l_addr;

	if (NULL==buf)
		return -1;

	if (0 == has_hold_sem) {
		rt_sem_take(&spiflash_sem, RT_WAITING_FOREVER);
		spi_flash_get_spi();
	}

	l_addr = addr & (~SF_SECTOR_ADDR_MASK);
	h_addr = (addr+len-1) & (~SF_SECTOR_ADDR_MASK);
	if (SFWBS_DIRTY==sf_buf.state && ((l_addr==sf_buf.addr) || (h_addr==sf_buf.addr) || (sf_buf.addr>l_addr && sf_buf.addr<h_addr)))
		flush_sfwb(1); /* 临时简单处理 */

	cmd[0] = SF_CMD_HS_READ;
	cmd[1] = (addr>>16) & 0xff;
	cmd[2] = (addr>>8)  & 0xff;
	cmd[3] = (addr)     & 0xff;
	cmd[4] = 0;

	SF_DEBUG(("%s() line:%d, addr:0x%x!\n", __FUNCTION__, __LINE__, addr));

	sf_select();
	sf_spi_send_data(cmd, 5);
	sf_spi_rec_data(buf, len);
	sf_deselect();

	if (0 == has_hold_sem) {
		spi_flash_release_spi();
		rt_sem_release(&spiflash_sem);
	}

	return 0;
}


void sf_read_jedec_id(unsigned char *buf, int len)
{
	if (NULL==buf || len < 3)
		return;

	sf_select();
	sf_spi_send_byte(SF_CMD_JEDEC_ID);
	sf_spi_rec_data(buf, 3);
	sf_deselect();

	return;
}

void sf_set_prote_level_to_none(void)
{
	unsigned char buf[4];

#ifdef SF_CMD_EWSR
	sf_select();
	sf_spi_send_byte(SF_CMD_EWSR);
	sf_deselect();
#endif
	buf[0] = SF_CMD_WRSR;
	buf[1] = 0x00;
	sf_select();
	sf_spi_send_data(buf, 2);
	sf_deselect();

	return;
}

void sf_erase_chip(void)
{
	sf_wait_nobusy();

	sf_select();
	sf_spi_send_byte(SF_CMD_WREN);
	sf_deselect();

	sf_wait_nobusy();

	sf_select();
	sf_spi_send_byte(SF_CMD_CE);
	sf_deselect();

	return;
}

static int sf_read_sr(void)
{
	char ret;

	sf_select();
	sf_spi_send_byte(SF_CMD_RDSR);
	ret = sf_spi_rec_byte();
	sf_deselect();

	return ret;
}

/*
 * NOTE:
 * 1. 只有获取信号量后才能调用该函数
 */
static void sf_wait_nobusy(void)
{
	char ret;

	do {
		ret = sf_read_sr();
	} while (is_sf_busy(ret));

	SF_DEBUG(("%s() line:%d, SR:0x%x!\n", __FUNCTION__, __LINE__, ret));
	return;
}

/*
 * NOTE:
 * 1. 扇区大小为偶数
 * 2. 扇区大小要能够被页大小整除
 * 3. 只有获取信号量后才能调用该函数
 */
static int sf_write_data_by_sector(unsigned long addr, const unsigned char *buf, unsigned long len)
{
	unsigned char cmd[8];
	unsigned long i;
	unsigned long cnt;

	if (0 != (addr & SF_SECTOR_ADDR_MASK))
		return 1;

	if (SF_SECTOR_SIZE!=len || NULL==buf)
		return 2;

	sf_select();
	sf_spi_send_byte(SF_CMD_WREN);
	sf_deselect();

	cmd[0] = SF_CMD_4KB_SE;
	cmd[1] = (addr>>16) & 0xff;
	cmd[2] = (addr>>8)  & 0xff;
	cmd[3] = (addr)     & 0xff;
	sf_select();
	sf_spi_send_data(cmd, 4);
	sf_deselect();

	sf_wait_nobusy(); /* 扇区擦除已完成 */

	SF_DEBUG(("%s() line:%d, addr:0x%x, len:%d!\n", __FUNCTION__, __LINE__, addr, len));
	SF_DEBUG(("%s() line:%d, data:%d, %d, %d, %d!\n", __FUNCTION__, __LINE__, buf[2048], buf[2049], buf[2050], buf[2051]));

	sf_select();
	sf_spi_send_byte(SF_CMD_WREN);
	sf_deselect();

#if 1==WRITE_FLASH_BY_PAGE
	cmd[0] = SF_CMD_WRITE_PAGE;
	cnt    = SF_SECTOR_SIZE / SF_PROGRAM_PAGE_SIZE;
	SF_DEBUG(("%s() line:%d, cnt:%d\n", __FUNCTION__, __LINE__, cnt));
	for (i=0; i<cnt; ++i) {
		cmd[1] = (addr>>16) & 0xff;
		cmd[2] = (addr>>8)  & 0xff;
		cmd[3] = (addr)     & 0xff;

		SF_DEBUG(("%s() line:%d, addr:0x%x, buf:0x%x\n", __FUNCTION__, __LINE__, addr, buf));

		sf_select();
		sf_spi_send_data(cmd, 4);
		sf_spi_send_data(buf, SF_PROGRAM_PAGE_SIZE);
		sf_deselect();

		buf  += SF_PROGRAM_PAGE_SIZE;
		addr += SF_PROGRAM_PAGE_SIZE;
		sf_wait_nobusy();

		sf_select();
		sf_spi_send_byte(SF_CMD_WREN);
		sf_deselect();

	}
#elif 1==WRITE_FLASH_BY_WORD
	cmd[0] = SF_CMD_AAI_W_P;
	cmd[1] = (addr>>16) & 0xff;
	cmd[2] = (addr>>8)  & 0xff;
	cmd[3] = (addr)     & 0xff;
	cmd[4] = buf[0];
	cmd[5] = buf[1];
	sf_select();
	sf_spi_send_data(cmd, 6);
	sf_deselect();

	SF_DEBUG(("%s() line:%d, data:%d, %d!\n", __FUNCTION__, __LINE__, cmd[4], cmd[5]));

	for (i=2; i<len; i+=2) {
		cmd[1] = buf[i];
		cmd[2] = buf[i+1];
		SF_DEBUG(("%s() line:%d, data:%d, %d!\n", __FUNCTION__, __LINE__, cmd[1], cmd[2]));

		sf_wait_nobusy();
		sf_select();
		sf_spi_send_data(cmd, 3);
		sf_deselect();
	}
#endif

	sf_wait_nobusy();
	sf_select();
	sf_spi_send_byte(SF_CMD_WRDI);
	sf_deselect();

	return 0;
}
