#include <rtthread.h>
#include <dfs_posix.h>
#include <lwip/sockets.h>

#include <finsh.h>
#include <rbin_head.h>
#include "md5.h"


#define TFTP_PORT	69
/* opcode */
#define TFTP_RRQ		1 	/* read request */
#define TFTP_WRQ		2	/* write request */
#define TFTP_DATA		3	/* data */
#define TFTP_ACK		4	/* ACK */
#define TFTP_ERROR		5	/* error */

struct write_data2spiflash_st {
	char *sector_buf;
	int spi_flash_addr;
	int sector_buf_ind;
};


#define TFTP_DATA_BLK (512)
#define TFTP_BUF_LEN  (TFTP_DATA_BLK+4)

#define RBIN_HEAD_LEN (sizeof(struct rbin_head_st))

static int cp_data2ext_mem(int tftp_cur_blk, char *tftp_buf, int len, struct write_data2spiflash_st *pwdsf);
static int send_updata_req2tftpd(const char *host, const char *filename, int *psock_fd, char *tftp_buf);
static int check_ext_mem_app(const char *buf, const int len, int total_bytes);

/*
 *   opcode  operation
 *     1     Read request (RRQ)
 *     2     Write request (WRQ)
 *     3     Data (DATA)
 *     4     Acknowledgment (ACK)
 *     5     Error (ERROR)
 *
 * RRQ/WRQ packet
 * 2 bytes     string    1 byte     string   1 byte
 * ------------------------------------------------
 * | Opcode |  Filename  |   0  |    Mode    |   0  |
 * ------------------------------------------------
 *
 * DATA packet
 *   2 bytes   2 bytes      n bytes
 * ----------------------------------
 * | Opcode  |  Block #    |  Data  |
 * ----------------------------------
 *
 * ACK packet
 *  2 bytes     2 bytes
 *  ---------------------
 * | Opcode |   Block #  |
 *  ---------------------
 *
 * ERROR packet
 * 2 bytes     2 bytes      string    1 byte
 * -----------------------------------------
 * | Opcode |  ErrorCode |   ErrMsg   |   0  |
 * -----------------------------------------
 */

/*
 *
 */
void update_by_tftp(const char *host, const char *filename)
{
	int sock_fd;
	struct sockaddr_in from_addr;
	socklen_t fromlen;

	uint16_t *p, tftp_cur_blk, opcode;
	int len, total_bytes;
	struct write_data2spiflash_st wdsf;
	char *tftp_buf;

	if (TFTP_DATA_BLK < RBIN_HEAD_LEN) {
		printf_syn("TFTP_DATA_BLK < RBIN_HEAD_LEN\n");
		return;
	}

	if (NULL==host || NULL==filename) {
		printf_syn("%s() param error\n", __FUNCTION__);
		return;
	}

#if EXT_MEM_USE_SDCARD
	if (0 == bootpart_info.offset) { /* mark by David */
		printf_syn("[%s]sd have not boot partition\n", __FUNCTION__);
		return;
	}
#endif

	wdsf.sector_buf = rt_malloc(EXT_MEM_SECTOR_SIZE);
	if (NULL == wdsf.sector_buf) {
		printf_syn("allocate sector_buf fail\n");
		return;
	}

	tftp_buf = rt_malloc(TFTP_BUF_LEN);
	if (NULL == tftp_buf) {
		printf_syn("allocate tftp_buf fail\n");
		rt_free(wdsf.sector_buf);
		return;
	}

	if (SUCC != send_updata_req2tftpd(host, filename, &sock_fd, tftp_buf)) {
		rt_free(wdsf.sector_buf);
		rt_free(tftp_buf);
		return;
	}

	tftp_cur_blk   = 1;
	total_bytes    = 0;
	wdsf.sector_buf_ind = 0;
	wdsf.spi_flash_addr = 0;
	do {
		len = lwip_recvfrom(sock_fd, tftp_buf, TFTP_BUF_LEN, 0, (struct sockaddr *)&from_addr, &fromlen);
		if (len >= 4) {
			p      = (rt_uint16_t *) &tftp_buf[0];
			opcode = lwip_ntohs(*p);

			p = (rt_uint16_t *) &tftp_buf[2];
			if (TFTP_DATA == opcode) {
				if (*p == lwip_htons(tftp_cur_blk)) {
					/* ...... */ /* mark by David */
					if (SUCC != cp_data2ext_mem(tftp_cur_blk, &tftp_buf[4], len-4, &wdsf)) {
						printf_syn("error. total %d bytes(blk:%d)\n", total_bytes, tftp_cur_blk);
						goto ret_entry;
					}

					/* make ACK */
					tftp_buf[0] = 0;
					tftp_buf[1] = TFTP_ACK; /* opcode */
					/* *p = lwip_htons(tftp_cur_blk); */
					/* send ACK */
					lwip_sendto(sock_fd, tftp_buf, 4, 0, (struct sockaddr *)&from_addr, fromlen);

					if (len - 4 < TFTP_DATA_BLK) {
						total_bytes += len - 4;
						printf_syn("done. total %d bytes\n", total_bytes);
						if (SUCC == check_ext_mem_app(wdsf.sector_buf, EXT_MEM_SECTOR_SIZE, total_bytes)) {
							printf_syn("Reboot to complete the upgrade\n");
						}
						break;
					}

					++tftp_cur_blk;
					total_bytes += 512;
				} else {
					/* 已经处理的数据块, 不再处理 */
				}
			} else if (TFTP_ERROR == opcode) {
				tftp_buf[TFTP_BUF_LEN-1] = '\0';
				printf_syn("tftp error(%d):%s\n", lwip_ntohs(*p),  &tftp_buf[4]);
				break;
			} else {
				printf_syn("tftp unusual!(tftp opcode:%d)\n", opcode);
				break;
			}
		} else if ( 0 == len) {
			printf_syn("the peer has performed an orderly shutdown\n");
			break;
		} else {
			printf_syn("socket that tftp have an error(len:%d)\n", len);
			break;
		}
	} while (1);

ret_entry:
	lwip_close(sock_fd);
	rt_free(wdsf.sector_buf);
	rt_free(tftp_buf);

	return;
}
FINSH_FUNCTION_EXPORT(update_by_tftp, host-filename);



static int send_updata_req2tftpd(const char *host, const char *filename, int *psock_fd, char *tftp_buf)
{
	struct sockaddr_in tftp_addr;
	int sock_fd;
	int sock_opt;
	int len;
	socklen_t tolen;

	/* connect to tftp server */
	inet_aton(host, (struct in_addr*)&(tftp_addr.sin_addr));
	tftp_addr.sin_family = AF_INET;
	tftp_addr.sin_port   = htons(TFTP_PORT);

	sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock_fd < 0) {
		printf_syn("can't create a socket\n");
		return FAIL;
	}

	/* set socket option */
	sock_opt = 5000; /* 5 seconds */
	lwip_setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &sock_opt, sizeof(sock_opt));

	/*
	 *  2 bytes   string    1 byte   string    1 byte
	 * ------------------------------------------------
	 * | Opcode | Filename |   0   |   Mode   |   0   |
	 * ------------------------------------------------
	 */
	tftp_buf[0] = 0;		/* opcode */
	tftp_buf[1] = TFTP_RRQ; 	/* RRQ */
	len = rt_sprintf((char *)&tftp_buf[2], "%s", filename) + 2;
	tftp_buf[len++] = 0;
	len += rt_sprintf((char*)&tftp_buf[len], "%s", "octet");
	tftp_buf[len++] = 0;
	tolen = sizeof(struct sockaddr_in);
	/* send request */
	lwip_sendto(sock_fd, tftp_buf, len, 0, (struct sockaddr *)&tftp_addr, tolen);

	*psock_fd = sock_fd;
	return SUCC;
}





/*
 * 将接收到的数据转储到缓冲区, 缓冲区满的时候, 将数据写入 spi flash
 *
 * len -- tftp_buf data length
 */
static int cp_data2ext_mem(int tftp_cur_blk, char *tftp_buf, int len, struct write_data2spiflash_st *pwdsf)
{
	int cnt, temp;
	int ret = SUCC;
	struct rbin_head_st *p_rbin_h;

	if (1 != tftp_cur_blk) {
		/* only contain rbin data */
		temp = EXT_MEM_SECTOR_SIZE - pwdsf->sector_buf_ind;
		cnt  = len > temp ? temp : len;
		rt_memcpy(pwdsf->sector_buf+pwdsf->sector_buf_ind, tftp_buf, cnt);
		pwdsf->sector_buf_ind += cnt;

		if (pwdsf->sector_buf_ind >= EXT_MEM_SECTOR_SIZE || (0!=len && len<TFTP_DATA_BLK)) {
			ext_mem_write_data_by_sector(pwdsf->spi_flash_addr, (uint8_t *)pwdsf->sector_buf,
										 pwdsf->sector_buf_ind);
			pwdsf->spi_flash_addr += EXT_MEM_SECTOR_SIZE;
			pwdsf->sector_buf_ind = 0;
		}

		temp = len - cnt;
		if (temp > 0) {
			rt_memcpy(pwdsf->sector_buf, tftp_buf+cnt, temp);
			pwdsf->sector_buf_ind += temp;
		}
	} else if (1 == tftp_cur_blk) {
		cnt = len - RBIN_HEAD_LEN;
		if (cnt < 0) {
			ret = FAIL;
			printf_syn("first tftp blk too small\n");
			goto ret_entry;
		}

		p_rbin_h = (struct rbin_head_st *)tftp_buf;
		if (RBIN_FILE_MAGIC != lwip_ntohl(p_rbin_h->rh_magic)) {
			ret = FAIL;
			printf_syn("rbin file magic number(%#x) is invalid\nrbin head size %d\n",
					   lwip_ntohl(p_rbin_h->rh_magic), RBIN_HEAD_LEN);
			goto ret_entry;
		}

		/* contain rbin-head info */
		ext_mem_write_data_by_sector(EXT_MEM_ADDR_OF_RBIN_HEAD, (uint8_t *)tftp_buf, RBIN_HEAD_LEN);
		rt_memcpy(pwdsf->sector_buf, tftp_buf+RBIN_HEAD_LEN, cnt);

		pwdsf->sector_buf_ind = cnt;
		pwdsf->spi_flash_addr = EXT_MEM_ADDR_OF_OSAPP;
	}

ret_entry:
	return ret;
}


static int check_ext_mem_app(const char *buf, const int len, int total_bytes)
{
	struct rbin_head_st *p_rbin_h;
	uint8_t  rh_md5[16];	/* bin file md5 digest */

	int data_len, ret = SUCC;
	uint32_t spi_flash_addr;
	MD5_CTX md5_test_ctx;
	unsigned char digest[16];

	if (NULL==buf || len < EXT_MEM_SECTOR_SIZE) {
		printf_syn("%s() param error\n", __FUNCTION__);
		return FAIL;
	}

	ext_mem_read_data(EXT_MEM_ADDR_OF_RBIN_HEAD, (uint8_t *)buf, sizeof(*p_rbin_h));
	p_rbin_h = (struct rbin_head_st *)buf;

	if (RBIN_FILE_MAGIC != lwip_ntohl(p_rbin_h->rh_magic)) {
		printf_syn("magic number is invalid\n");
		return FAIL;
	}

	if (total_bytes != lwip_ntohl(p_rbin_h->rh_rbin_size)) {
		printf_syn("bin size is not match\n");
		return FAIL;
	}

	printf_syn("\nMD5(read from spi flash):");
	for (data_len=0; data_len<sizeof(rh_md5); ++data_len) {
		rh_md5[data_len] = p_rbin_h->rh_md5[data_len];
		printf_syn("%02x", rh_md5[data_len]);
	}


	spi_flash_addr  = EXT_MEM_ADDR_OF_OSAPP;
	data_len	= total_bytes - sizeof(struct rbin_head_st); /* head与os-app在spi flash中分开存储 */
	MD5Init(&md5_test_ctx);
	printf_syn("\nstart check program of spi flash(bin-file:%dBytes)...\n", data_len);
	for (; data_len>=len; data_len-=len) {
		ext_mem_read_data(spi_flash_addr, (uint8_t *)buf, len);
		MD5Update(&md5_test_ctx, (uint8_t *)buf, len);
		spi_flash_addr += len;
	}
	if (0!=data_len && len>data_len) {
		ext_mem_read_data(spi_flash_addr, (uint8_t *)buf, data_len);
		MD5Update(&md5_test_ctx, (uint8_t *)buf, data_len);
	}
	MD5Final(digest, &md5_test_ctx);


	printf_syn("MD5(calculate):");
	for (data_len=0; data_len<sizeof(digest); ++data_len) {
		printf_syn("%02x", digest[data_len]);
	}

	for (data_len=0; data_len<sizeof(digest); ++data_len) {
		if (digest[data_len] != rh_md5[data_len])
			break;
	}

	printf_syn("\nend check program of spi flash...");

	if (data_len < sizeof(digest)) {
		printf_syn("spi flash data MD5 check fail\n");
		ret = FAIL;
	} else {
		printf_syn("spi flash data MD5 check succ\n");
	}

	ext_mem_read_data(EXT_MEM_ADDR_OF_RBIN_HEAD, (uint8_t *)buf, sizeof(*p_rbin_h));
	p_rbin_h = (struct rbin_head_st *)buf;
	p_rbin_h->rbin_data_status = RDS_SPIF_HAD_CHECK;
	ext_mem_write_data_by_sector(EXT_MEM_ADDR_OF_RBIN_HEAD, (uint8_t *)buf, sizeof(*p_rbin_h));

	return ret;
}

