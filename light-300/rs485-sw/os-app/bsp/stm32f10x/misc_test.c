/*
 ******************************************************************************
 * misc_test.c
 *
 *  Created on: 2014-3-16
 *      Author: David, zhaoshaowei@yeejoin.com
 *
 * COPYRIGHT (C) 2014, YEEJOIN (Beijing) Technology Development Co., Ltd.
 ******************************************************************************
 */

#include <rtdef.h>
#include <board.h>
#include <rtthread.h>
#include <finsh.h>
#include <syscfgdata.h>


#if RT_USING_SERIAL_FLASH
#include <spiflash.h>
#include <sf_hal.h>
#endif


extern void sst25_set_prote_level_to_none(void);



#if 0!=TEST_PSRAM_MODE /* test PSRAM */
enum test_psram_cmde {
	TPC_START		= 0,

	WRITE_PSRAM		= 1,
	READ_PSRAM		= 2,
	AUTO_CHECK_WHOLE_PSRAM	= 3,

	WRITE_DHW_PSRAM		= 4,
	WRITE_SEQ_PSRAM		= 5,
	TPC_END,
};

static void do_test_psram(const int cmd, const int unit,
			  const unsigned long addr, const unsigned long value,
			  const unsigned int cnt, const int addr_delta, const int val_delta);
/*
 * cmd		-- command
 * unit		-- 每次操作的字节数
 */
void test_psram(const int cmd, int unit,
		unsigned long addr, unsigned long value, unsigned int cnt,
		int addr_delta, int val_delta)
{
	if (cnt <= 0)
		cnt = 1;

	if (addr<(unsigned long)STM32_EXT_SRAM_START_ADDR || addr>(unsigned long)(STM32_EXT_SRAM_START_ADDR+STM32_EXT_SRAM_MAX_LEN-4))
		addr = (unsigned long)STM32_EXT_SRAM_START_ADDR;

	if (1!=unit && 2!=unit && 4!=unit)
		unit = 4;

	do_test_psram(cmd, unit, addr, value, cnt, addr_delta, val_delta);

	return;
}
FINSH_FUNCTION_EXPORT(test_psram, cmd-unit-addr-value-cnt-addr_delta-val_delta);

static void do_test_psram(const int cmd, const int unit,
			  const unsigned long addr, const unsigned long value,
			  const unsigned int cnt, const int addr_delta, const int val_delta)
{
	rt_uint32_t i;
	rt_uint16_t i16;
	rt_uint8_t  i8;

	volatile rt_uint32_t *pw;
	volatile rt_uint16_t *ph;
	volatile rt_uint8_t  *pc;
	int err_cnt;
	unsigned long val;


	pw = (rt_uint32_t *)addr;
	ph = (rt_uint16_t *)addr;
	pc = (rt_uint8_t  *)addr;
	val = value;

	printf_syn("[do_test_psram]cmd:%d, unit:%d, addr:0x%x, val:0x%x, cnt:%d, addr_delta:0x%x, val_delta:0x%x\n",
		cmd, unit, addr, value, cnt, addr_delta, val_delta);

	switch (cmd) {
	case WRITE_PSRAM:
		if (1 == unit) {
			for (i=0; i<cnt; ++i) {
 				*pc =  val;
				pc  += addr_delta;
				val += val_delta;
			}
			printf_syn("write psram by %d byte(s) over! last addr:0x%x, last value:0x%x, should val:0x%x\n",
					unit, (pc - addr_delta), *(pc - addr_delta), val-val_delta);
 		} else if (2 == unit) {
			for (i=0; i<cnt; ++i) {
 				*ph =  val;
				ph  += addr_delta;
				val += val_delta;
			}
			printf_syn("write psram by %d byte(s) over! last addr:0x%x, last value:0x%x, should val:0x%x\n",
					unit, (ph - addr_delta), *(ph - addr_delta), val-val_delta);
 		} else if (4 == unit) {
			for (i=0; i<cnt; ++i) {
			printf_syn("i:%d, pc:0x%x, val:0x%x\n", i, pc, val);
				*pw =  val;
				pw  += addr_delta;
				val += val_delta;
			}
			printf_syn("write psram by %d byte(s) over! last addr:0x%x, last value:0x%x, should val:0x%x\n",
					unit, (pw - addr_delta), *(pw - addr_delta), val-val_delta);
		} else {
			printf_syn("write unit error(%d)!\n", unit);
		}
		break;

	case READ_PSRAM:
		printf_syn("read psram by %d byte(s) start......\n", unit);
		if (1 ==  unit) {
			for (i=0; i<cnt; ++i) {
				printf_syn("addr:0x%x, value:0x%x, should val:0x%x\n", pc, *pc, val);
				pc  += addr_delta;
				val += val_delta;
			}
		} else if (2 ==  unit) {
			for (i=0; i<cnt; ++i) {
				printf_syn("addr:0x%x, value:0x%x, should val:0x%x\n", ph, *ph, val);
				ph  += addr_delta;
				val += val_delta;
			}
		} else if (4 ==  unit) {
			for (i=0; i<cnt; ++i) {
				printf_syn("addr:0x%x, value:0x%x, should val:0x%x\n", pw, *pw, val);
				pw  += addr_delta;
				val += val_delta;
			}
		} else {
			printf_syn("read unit error(%d)!\n", unit);
		}

		printf_syn("read psram by %d byte(s) end.\n", unit);
		break;

	case AUTO_CHECK_WHOLE_PSRAM:
		if (4 == unit) {
			pw = (rt_uint32_t *)STM32_EXT_SRAM_START_ADDR;
			for (i=0; i<(STM32_EXT_SRAM_MAX_LEN/4); ++i) {
				*pw = i;
				++pw;
			}
			printf_syn("[word]write psram over! last addr:0x%x, last value:0x%x, i:0x%x\n", (pw-1), *(pw-1), i);

			err_cnt = 0;
			pw = (rt_uint32_t *)STM32_EXT_SRAM_START_ADDR;
			for (i=0; i<(STM32_EXT_SRAM_MAX_LEN/4); ++i) {
				if (*pw != i) {
					if (err_cnt < cnt) {
						printf_syn("addr:0x%x, value:0x%x, i:0x%x\n", pw, *pw, i);
						++err_cnt;
					} else {
						printf_syn("had %d errors\n", err_cnt);
						break;
					}
				}
				++pw;
			}
		} else if (2 == unit) {
			i16 = 0;
			ph = (rt_uint16_t *)STM32_EXT_SRAM_START_ADDR;
			for (i=0; i<(STM32_EXT_SRAM_MAX_LEN/2); ++i) {
				*ph = i16++;
				++ph;
			}
			printf_syn("[half-word]write psram over! last addr:0x%x, last value:0x%x, i:0x%x\n", (ph-1), *(ph-1), i);

			err_cnt = 0;
			i16 = 0;
			ph = (rt_uint16_t *)STM32_EXT_SRAM_START_ADDR;
			for (i=0; i<(STM32_EXT_SRAM_MAX_LEN/2); ++i) {
				if (*ph != i16++) {
					if (err_cnt < cnt) {
						printf_syn("addr:0x%x, value:0x%x, i:0x%x, i16:0x%x\n", ph, *ph, i, i16);
						++err_cnt;
					} else {
						printf_syn("had %d errors\n", err_cnt);
						break;
					}
				}
				++ph;
			}
		} else {
			i8 = 0;
			pc = (rt_uint8_t *)STM32_EXT_SRAM_START_ADDR;
			for (i=0; i<(STM32_EXT_SRAM_MAX_LEN); ++i) {
				*pc = i8++;
				++pc;
			}
			printf_syn("[byte]write psram over! last addr:0x%x, last value:0x%x, i:0x%x\n", (pc-1), *(pc-1), i);

			err_cnt = 0;
			i8 = 0;
			pc = (rt_uint8_t *)STM32_EXT_SRAM_START_ADDR;
			for (i=0; i<(STM32_EXT_SRAM_MAX_LEN); ++i) {
				if (*pc != i8++) {
					if (err_cnt < cnt) {
						printf_syn("addr:0x%x, value:0x%x, i:0x%x, i8:0x%x\n", pc, *pc, i, i8);
						++err_cnt;
					} else {
						printf_syn("had %d errors\n", err_cnt);
						break;
					}
				}
				++pc;
			}
		}
		break;

	case WRITE_DHW_PSRAM:
		if (2 == unit) {
			*ph++ = val;
			*ph   = val;
			printf_syn("write psram by double half word! last value:0x%x\n", *(ph));
 		} else {
			printf_syn("write psram by double half word! param error\n");
 		}
		break;

	case WRITE_SEQ_PSRAM:
		if (1 == unit) {
			for (i=0; i<cnt; ++i) {
 				*pc++ =  i;
			}
 		} else if (2 == unit) {
			for (i=0; i<cnt; ++i) {
 				*ph++ =  i;
			}
 		} else if (4 == unit) {
			for (i=0; i<cnt; ++i) {
				*pw++ =  i;
			}
		} else {
			printf_syn("write unit error(%d)!\n", unit);
		}
		printf_syn("write psram by %d byte(s) over! cnt:%d\n", unit, cnt);

		break;

	default:
		printf_syn("error cmd :%d\n", cmd);
		break;

	}

	return;
}

/*
 * len   -- bytes
 * width -- bit
 */
void mem_tdata_bus(volatile void *addr, unsigned long len, const int width)
{
	volatile rt_uint32_t *pw, *pw1;
	volatile rt_uint16_t *ph, *ph1;
	volatile rt_uint8_t  *pc, *pc1;
	int need_walk_01 = 0;
	int i;

	pw = addr;
	ph = addr;
	pc = addr;

	pw1 = pw + len/4 - 1;
	ph1 = ph + len/2 - 1;
	pc1 = pc + len/1 - 1;

	switch (width) {
	case 8:
		*pc  = 0x55;
		*pc1 = 0xaa;
		if (0x55!=*pc || 0xaa!=*pc1) {
			printf_syn("*pc(0x55):0x%x, *pc1(0xaa):0x%x\n", *pc, *pc1);
			need_walk_01 = 1;
		}

		*pc  = 0xaa;
		*pc1 = 0x55;
		if (0xaa!=*pc || 0x55!=*pc1) {
			printf_syn("*pc(0xaa):0x%x, *pc1(0x55):0x%x\n", *pc, *pc1);
			need_walk_01 = 1;
		}

		*pc  = 0x00;
		*pc1 = 0xff;
		if (0x00!=*pc || 0xff!=*pc1) {
			printf_syn("*pc(0x00):0x%x, *pc1(0xff):0x%x\n", *pc, *pc1);
			need_walk_01 = 1;
		}
		break;

	case 16:
		*ph  = 0x5555;
		*ph1 = 0xaaaa;
		if (0x5555!=*ph || 0xaaaa!=*ph1) {
			printf_syn("*pc(0x5555):0x%x, *pc1(0xaaaa):0x%x\n", *ph, *ph1);
			need_walk_01 = 1;
		}

		*ph  = 0xaaaa;
		*ph1 = 0x5555;
		if (0xaaaa!=*ph || 0x5555!=*ph1) {
			printf_syn("*pc(0xaaaa):0x%x, *pc1(0x5555):0x%x\n", *ph, *ph1);
			need_walk_01 = 1;
		}

		*ph  = 0x0000;
		*ph1 = 0xffff;
		if (0x0000!=*ph || 0xffff!=*ph1) {
			printf_syn("*pc(0x0000):0x%x, *pc1(0xffff):0x%x\n", *ph, *ph1);
			need_walk_01 = 1;
		}
		break;

	case 32:
		*pw  = 0x55555555;
		*pw1 = 0xaaaaaaaa;
		if (0x55555555!=*pw || 0xaaaaaaaa!=*pw1) {
			printf_syn("*pc(0x55555555):0x%x, *pc1(0xaaaaaaaa):0x%x\n", *pw, *pw1);
			need_walk_01 = 1;
		}

		*pw  = 0xaaaaaaaa;
		*pw1 = 0x55555555;
		if (0xaaaaaaaa!=*pw || 0x55555555!=*pw1) {
			printf_syn("*pc(0xaaaaaaaa):0x%x, *pc1(0x55555555):0x%x\n", *pw, *pw1);
			need_walk_01 = 1;
		}

		*pw  = 0x00000000;
		*pw1 = 0xffffffff;
		if (0x00000000!=*pw || 0xffffffff!=*pw1) {
			printf_syn("*pc(0x00000000):0x%x, *pc1(0xffffffff):0x%x\n", *pw, *pw1);
			need_walk_01 = 1;
		}
		break;

	default:
		printf_syn("error width:%d\n", width);
		break;
	}

	if (0 != need_walk_01) {
		need_walk_01 = 0;

		switch (width) {
		case 8:
			i = 0;
			while (i<8-1) {
				*pc  = 1 << i++;
				*pc1 = 1 << i++;
				if ((1<<(i-2))!=*pc || (1<<(i-1))!=*pc1) {
					printf_syn("[walk 1]*pc(0x%x):0x%x, *pc1(0x%x):0x%x\n",
						(1<<(i-2)), *pc, (1<<(i-1)), *pc1);
					need_walk_01 = 1;
				}
			}

			i = 0;
			while (i<8-1) {
				*pc  = ~(1 << i++);
				*pc1 = ~(1 << i++);
				if (~(1<<(i-2))!=*pc || ~(1<<(i-1))!=*pc1) {
					printf_syn("[walk 0]*pc(0x%x):0x%x, *pc1(0x%x):0x%x\n",
						~(1<<(i-2)), *pc, ~(1<<(i-1)), *pc1);
					need_walk_01 = 1;
				}
			}
			break;

		case 16:
			i = 0;
			while (i<16-1) {
				*ph  = 1 << i++;
				*ph1 = 1 << i++;
				if ((1<<(i-2))!=*ph || (1<<(i-1))!=*ph1) {
					printf_syn("[walk 1]*ph(0x%x):0x%x, *ph1(0x%x):0x%x\n",
						(1<<(i-2)), *ph, (1<<(i-1)), *ph1);
					need_walk_01 = 1;
				}
			}

			i = 0;
			while (i<16-1) {
				*ph  = ~(1 << i++);
				*ph1 = ~(1 << i++);
				if (~(1<<(i-2))!=*ph || ~(1<<(i-1))!=*ph1) {
					printf_syn("[walk 0]*ph(0x%x):0x%x, *ph1(0x%x):0x%x\n",
						~(1<<(i-2)), *ph, ~(1<<(i-1)), *ph1);
					need_walk_01 = 1;
				}
			}
			break;

		case 32:
			i = 0;
			while (i<32-1) {
				*pw  = 1 << i++;
				*pw1 = 1 << i++;
				if ((1<<(i-2))!=*pw || (1<<(i-1))!=*pw1) {
					printf_syn("[walk 1]*ph(0x%x):0x%x, *ph1(0x%x):0x%x\n",
						(1<<(i-2)), *pw, (1<<(i-1)), *pw1);
					need_walk_01 = 1;
				}
			}

			i = 0;
			while (i<32-1) {
				*pw  = ~(1 << i++);
				*pw1 = ~(1 << i++);
				if (~(1<<(i-2))!=*pw || ~(1<<(i-1))!=*pw1) {
					printf_syn("[walk 0]*ph(0x%x):0x%x, *ph1(0x%x):0x%x\n",
						~(1<<(i-2)), *pw, ~(1<<(i-1)), *pw1);
					need_walk_01 = 1;
				}
			}
			break;

		default:
			printf_syn("error width:%d\n", width);
			break;
		}

	}

	if (0 == need_walk_01)
		printf_syn("data bus no error!(width:%d)\n", width);
	else
		printf_syn("data bus has error!(width:%d)\n", width);


	return;
}
FINSH_FUNCTION_EXPORT(mem_tdata_bus, addr-len-width);

void mem_taddr_bus(volatile void*addr, const unsigned long len, unsigned char initval, unsigned char val)
{
	volatile rt_uint8_t  *pc;
	int i, j;
	unsigned long mask;
	unsigned long offset, offset1;

	pc   = addr;
	mask = len - 1;

walk_1:
	i      = 0;
	offset = 0;
	do {
		*(pc+offset) = initval;
		offset = mask & (1<<i++);
	} while (0 != offset);

	i      = 0;
	offset = 0;
	do {
		*(pc+offset) = val;
		for (j=i; 0!=(offset1=mask & (1<<j)); ++j) {
			if (initval != *(pc+offset1)) {
				printf_syn("[walk 1 error,index:%d]addr:0x%x should 0x%x, but is 0x%x, [when addr:0x%x, val:0x%x]\n",
					i, (pc+offset1), initval, *(pc+offset1), (pc+offset), *(pc+offset));
				*(pc+offset1) = initval;
				//goto walk_0;
			}
		}
		offset = mask & (1<<i++);
	} while (0 != offset);

walk_0:
	i      = 0;
	offset = mask & (~0);
	do {
		*(pc+offset) = initval;
		offset = mask & ~(1<<i++);
	} while (0 != (mask & ~offset));

	i      = 0;
	offset = mask & (~0);
	do {
		*(pc+offset) = val;

		j = i;
		offset1 = mask & ~(1<<j);
		while (0 != (mask & ~offset1)) {
			if (initval != *(pc+offset1)) {
				printf_syn("[walk 0 error, index:%d]addr:0x%x should 0x%x, but is 0x%x, [when addr:0x%x, val:0x%x]\n",
					i, (pc+offset1), initval, *(pc+offset1), (pc+offset), *(pc+offset));
				*(pc+offset1) = initval;
				//goto ret_entry;
			}
			offset1 = mask & ~(1<<j++);
		}

		offset = mask & ~(1<<i++);
	} while (0 != (mask & ~offset));

ret_entry:
	printf_syn("%s() over!\n", __FUNCTION__);

	return;
}
FINSH_FUNCTION_EXPORT(mem_taddr_bus, addr-len-initval-val);
#endif
  

#if 0
#include <sf_hal.h>

enum sst25_cmd {
	SST25_READ_ID = 1,
	SST25_READ_DATA = 2,
	SST25_WRITE_DATA = 3,
	SST25_READ_SR    = 4,
	SST25_SET_PROTE  = 5,
};

//extern void sf_erase_chip(void);

void sf_test(int cmd, unsigned long addr, int len, int init_data)
{
	unsigned char buf[16];
	int i;

	if (len > sizeof(buf))
		len = sizeof(buf);

	printf_syn("cmd:%d, addr:0x%x, len:%d, init_data:%d\n", cmd, addr, len, init_data);

	switch (cmd) {
#if 0
	case 0:
		sf_erase_chip();
		break;
#endif
	case SST25_READ_ID:
		sf_read_jedec_id(buf, sizeof(buf));
		printf_syn("0x%x, 0x%x, 0x%x\n", buf[0], buf[1], buf[2]);
		break;

	case SST25_READ_DATA:
		sf_read_data(addr, buf, sizeof(buf), 0);
		for (i=0; i<len; i+=4)
			printf_syn("[i:%d]0x%x, 0x%x, 0x%x, 0x%x\n", i, buf[i], buf[i+1], buf[i+2], buf[i+3]);

		break;

	case SST25_WRITE_DATA:
#if 1
		for (i=0; i<len; ++i)
			buf[i] = init_data + i;
		sf_write_data(addr, buf, len);
		break;
#else
	{
		rt_device_t sf_dev;

		sf_dev = rt_device_find("sf0");
		if (NULL == sf_dev) {
			printf_syn("find sf0 failed\n");
			break;
		}

		for (i=0; i<len; ++i)
			buf[i] = init_data + i;

		if (sf_dev->write != NULL)
			sf_dev->write(sf_dev, addr, buf, len);
		else
			printf_syn("sf write is null\n");
		break;
	}
#endif
#if 0
	case SST25_READ_SR:
		printf_syn("sst25 SR:0x%x!\n", sf_read_sr());
		break;
#endif
	case SST25_SET_PROTE:
		sf_set_prote_level_to_none();
		break;

	default:
		printf_syn("%s() cmd(%d) err!\n", __FUNCTION__, cmd);
		break;
	}

	printf_syn("%s() over!\n", __FUNCTION__);

	return;
}
FINSH_FUNCTION_EXPORT(sf_test, cmd-addr-len-initdata);
#endif




#if 0
#include <sf_hal.h>

enum sst25_cmd {
	SST25_READ_ID = 1,
	SST25_READ_DATA = 2,
	SST25_WRITE_DATA = 3,
	SST25_READ_SR    = 4,
	SST25_SET_PROTE  = 5,
};

//extern void sf_erase_chip(void);

void sf_test(int cmd, unsigned long addr, int len, int init_data)
{
	unsigned char buf[16];
	int i;

	if (len > sizeof(buf))
		len = sizeof(buf);

	printf_syn("cmd:%d, addr:0x%x, len:%d, init_data:%d\n", cmd, addr, len, init_data);

	switch (cmd) {
#if 0
	case 0:
		sf_erase_chip();
		break;
#endif
	case SST25_READ_ID:
		sf_read_jedec_id(buf, sizeof(buf));
		printf_syn("0x%x, 0x%x, 0x%x\n", buf[0], buf[1], buf[2]);
		break;

	case SST25_READ_DATA:
		sf_read_data(addr, buf, sizeof(buf), 0);
		for (i=0; i<len; i+=4)
			printf_syn("[i:%d]0x%x, 0x%x, 0x%x, 0x%x\n", i, buf[i], buf[i+1], buf[i+2], buf[i+3]);

		break;

	case SST25_WRITE_DATA:
#if 1
		for (i=0; i<len; ++i)
			buf[i] = init_data + i;
		sf_write_data(addr, buf, len);
		break;
#else
	{
		rt_device_t sf_dev;

		sf_dev = rt_device_find("sf0");
		if (NULL == sf_dev) {
			printf_syn("find sf0 failed\n");
			break;
		}

		for (i=0; i<len; ++i)
			buf[i] = init_data + i;

		if (sf_dev->write != NULL)
			sf_dev->write(sf_dev, addr, buf, len);
		else
			printf_syn("sf write is null\n");
		break;
	}
#endif
#if 0
	case SST25_READ_SR:
		printf_syn("sst25 SR:0x%x!\n", sf_read_sr());
		break;
#endif
	case SST25_SET_PROTE:
		sf_set_prote_level_to_none();
		break;

	default:
		printf_syn("%s() cmd(%d) err!\n", __FUNCTION__, cmd);
		break;
	}

	printf_syn("%s() over!\n", __FUNCTION__);

	return;
}
FINSH_FUNCTION_EXPORT(sf_test, cmd-addr-len-initdata);
#endif


#if 1
void temp_test(int cmd, unsigned data)
{
	switch (cmd) {
	case 1:
		break;

	case 2:
		break;

	case 3:
		break;

	case 4:
		break;

	case 5:      
		break;

	default:
		break;
	}
}

FINSH_FUNCTION_EXPORT(temp_test, temp_test cmd-data);
#endif




#if 0
void set_usart_baud_for_645(int uart_no, int baud)
{
	USART_InitTypeDef USART_InitStructure;
	USART_ClockInitTypeDef USART_ClockInitStructure;
	USART_TypeDef* USARTx;

	switch (uart_no) {
	case 1:
		USARTx = USART1;
		break;

	case 2:
		USARTx = USART2;
		break;

	case 3:
		USARTx = USART3;
		break;

	case 4:
		USARTx = UART4;
		break;

	case 5:
		USARTx = UART5;
		break;

	default:
		printf_syn("%s(), line:%d, uart_no invalid:%d\n", __FUNCTION__, __LINE__, uart_no);
		return;
	}

	if (1200!=baud && 2400!=baud && 4800!=baud && 9600!=baud) {
		printf_syn("%s(), line:%d, baud invalid:%d\n", __FUNCTION__, __LINE__, baud);
		return;
	}

	/* Enable USART2 Rx request */
	USART_ITConfig(USARTx, USART_IT_RXNE, DISABLE);
	USART_DeInit(USARTx);

	USART_InitStructure.USART_BaudRate = baud;

	USART_InitStructure.USART_WordLength = USART_WordLength_9b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_Even;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_ClockInitStructure.USART_Clock = USART_Clock_Disable;
	USART_ClockInitStructure.USART_CPOL = USART_CPOL_Low;
	USART_ClockInitStructure.USART_CPHA = USART_CPHA_2Edge;
	USART_ClockInitStructure.USART_LastBit = USART_LastBit_Disable;
	USART_Init(USARTx, &USART_InitStructure);
	USART_ClockInit(USARTx, &USART_ClockInitStructure);

	/* Enable USARTx Rx request */
	USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE);

	printf_syn("had set uart-%d's baud to %d\n", uart_no, baud);
	return;
}
#endif

