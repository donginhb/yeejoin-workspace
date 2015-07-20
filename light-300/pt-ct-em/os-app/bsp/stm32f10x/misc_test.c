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
#include <ade7880_hw.h>
#include <ade7880_api.h>

#if EM_ALL_TYPE_BASE
#include <ammeter.h>
#include <sink_info.h>
#endif

#if RT_USING_SERIAL_FLASH
#include <spiflash.h>
#include <sf_hal.h>
#endif


#include <frame_em.h>

extern void sst25_set_prote_level_to_none(void);
extern void rt_val_7880(void);
extern volatile s32 XFVAR_HSCD_BUFFER[3];
extern volatile u32 AI_HSCD_BUFFER[40];
extern volatile u32 AV_HSCD_BUFFER[40];
extern volatile u32 BI_HSCD_BUFFER[40];
extern volatile u32 BV_HSCD_BUFFER[40];
extern volatile u32 CI_HSCD_BUFFER[40];  
extern volatile u32 CV_HSCD_BUFFER[40];


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


#if EM_MULTI_BASE
void temp_test(int cmd, unsigned data)
{
	switch (cmd) {
	case 1:
		set_decoder_3to8_data(data);
		break;

	case 2:
		set_decoder_4to16_data(data);
		break;

	case 3:
		if (data)
			set_port_pin(ade7880_range_switch_gpio, ade7880_range_switch_pin);
		else
			clr_port_pin(ade7880_range_switch_gpio, ade7880_range_switch_pin);
		break;

	case 4:
		led_blink(led_port_sys_fault, led_pin_sys_fault);
		printf_syn("double_mbus_indication:%d%d%d%d %d%d%d%d %d%d%d%d\n",
				!!pin_in_is_set(double_mbus_indication_1_gpio, double_mbus_indication_1_pin),
				!!pin_in_is_set(double_mbus_indication_2_gpio, double_mbus_indication_2_pin),
				!!pin_in_is_set(double_mbus_indication_3_gpio, double_mbus_indication_3_pin),
				!!pin_in_is_set(double_mbus_indication_4_gpio, double_mbus_indication_4_pin),
				!!pin_in_is_set(double_mbus_indication_5_gpio, double_mbus_indication_5_pin),
				!!pin_in_is_set(double_mbus_indication_6_gpio, double_mbus_indication_6_pin),
				!!pin_in_is_set(double_mbus_indication_7_gpio, double_mbus_indication_7_pin),
				!!pin_in_is_set(double_mbus_indication_8_gpio, double_mbus_indication_8_pin),
				!!pin_in_is_set(double_mbus_indication_9_gpio, double_mbus_indication_9_pin),
				!!pin_in_is_set(double_mbus_indication_10_gpio, double_mbus_indication_10_pin),
				!!pin_in_is_set(double_mbus_indication_11_gpio, double_mbus_indication_11_pin),
				!!pin_in_is_set(double_mbus_indication_12_gpio, double_mbus_indication_12_pin) );
		break;

	case 5:      
		switch_current_channels(data);
		break;

	case 6:    
		//get_one_em_all_sinkinfo(data);
		break;

	case 7:
		//switch_to_cur_em(data);
		break;

	default:
		break;
	}
}

FINSH_FUNCTION_EXPORT(temp_test, temp_test cmd-data);
#endif

#if EM_ALL_TYPE_BASE
#include <rs485.h>


extern struct px_sample_data_st px_sample_data[ELECTRIC_METER_NUMBER_MAX];
extern struct rt_semaphore sinkinfo_sem;

extern volatile int connect33_data;


static void print_emc_px_independence_info(char *title, struct sinkinfo_emc_px_independence_st *info)
{
	if (NULL == info)
		return;

	if (NULL != title)
		printf_syn("%s", title);

	printf_syn("v:%+8d, i:%+8d, freq:%+4d, phase:%+8d, activp:%+8d, "
			"reactivep:%+8d, apparent:%+8d, factor:%+8d, v-dist:%+4d, i-dist:%+4d\n",
			info->vx, info->ix, info->hzx, info->phx, info->apx,
			info->rapx, info->appx, info->pfx, info->vdx, info->cdx);

	return;

}

static void print_em_px_independence_info(char *title, struct sinkinfo_em_px_independence_st *info)
{
	if (NULL == info)
		return;

	if (NULL != title)
		printf_syn("%s", title);

	printf_syn("v:%8x, i:%8x, activp:%8x, reactivep:%8x, apparent:%8x, factor:%8x, v-dist:%4x, i-dist:%4x\n",
			info->vx, info->ix, info->apx, info->rapx, info->appx, info->pfx, info->vdx, info->cdx);

	return;
}

void print_pt_ct_st_info(char *title, struct sinkinfo_pt_ct_st *info)
{
	if (NULL == info)
		return;

	if (NULL != title)
		printf_syn("%s", title);

	printf_syn("v:%+10d, i:%+10d, activp:%+10d, apparent:%+10d, factor:%+10d\n",
			info->vx, info->ix, info->apx, info->appx, info->pfx);

	return;
}


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

extern void print_sink_data(void);
#define DEBUG_CAREFUL 1

static void print_harmonic_parameter_st(struct harmonic_parameter_st *p)
{
	printf_syn("vrms:%10d, irms:%10d, watt:%10d, var:%10d, va:%10d, pf:%10d, vhd:%10d, ihd:%10d\n",
			p->vrms, p->irms, p->watt, p->var,
			p->va,   p->pf,   p->vhd,  p->ihd);

	return;
}

static void print_sinkinfo_harmonic(int param)
{
	printf_syn("si_harmonic_pa num-1:\n");
	print_harmonic_parameter_st(&sinkinfo_all_em[param].si_harmonic_pa[1]);

	printf_syn("si_harmonic_pb num-1:\n");
	print_harmonic_parameter_st(&sinkinfo_all_em[param].si_harmonic_pb[1]);

	printf_syn("si_harmonic_pc num-1:\n");
	print_harmonic_parameter_st(&sinkinfo_all_em[param].si_harmonic_pc[1]);

	printf_syn("si_harmonic_pa num-2:\n");
	print_harmonic_parameter_st(&sinkinfo_all_em[param].si_harmonic_pa[2]);

	printf_syn("si_harmonic_pb num-2:\n");
	print_harmonic_parameter_st(&sinkinfo_all_em[param].si_harmonic_pb[2]);

	printf_syn("si_harmonic_pc num-2:\n");
	print_harmonic_parameter_st(&sinkinfo_all_em[param].si_harmonic_pc[2]);

	return;
}

static void print_sinkinfo_vi_sample(int param)
{
#if DEBUG_CAREFUL
	int i;
	int cnt;

	printf_syn("\nA v-sampling:\n");
	cnt = 0;
	for (i=0; i<15; ++i) {
		printf_syn("0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x\n",
				sinkinfo_all_em[param].px_vi_sample.pa_vi_sample[0][cnt],
				sinkinfo_all_em[param].px_vi_sample.pa_vi_sample[0][cnt+1],
				sinkinfo_all_em[param].px_vi_sample.pa_vi_sample[0][cnt+2],
				sinkinfo_all_em[param].px_vi_sample.pa_vi_sample[0][cnt+3],
				sinkinfo_all_em[param].px_vi_sample.pa_vi_sample[0][cnt+4],
				sinkinfo_all_em[param].px_vi_sample.pa_vi_sample[0][cnt+5],
				sinkinfo_all_em[param].px_vi_sample.pa_vi_sample[0][cnt+6],
				sinkinfo_all_em[param].px_vi_sample.pa_vi_sample[0][cnt+7]
				);
		cnt += 8;
	}

	printf_syn("\nA i-sampling:\n");
	cnt = 0;
	for (i=0; i<15; ++i) {
		printf_syn("0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x\n",
				sinkinfo_all_em[param].px_vi_sample.pa_vi_sample[1][cnt],
				sinkinfo_all_em[param].px_vi_sample.pa_vi_sample[1][cnt+1],
				sinkinfo_all_em[param].px_vi_sample.pa_vi_sample[1][cnt+2],
				sinkinfo_all_em[param].px_vi_sample.pa_vi_sample[1][cnt+3],
				sinkinfo_all_em[param].px_vi_sample.pa_vi_sample[1][cnt+4],
				sinkinfo_all_em[param].px_vi_sample.pa_vi_sample[1][cnt+5],
				sinkinfo_all_em[param].px_vi_sample.pa_vi_sample[1][cnt+6],
				sinkinfo_all_em[param].px_vi_sample.pa_vi_sample[1][cnt+7]
				);
		cnt += 8;
	}

	printf_syn("\nB v-sampling :\n");
	cnt = 0;
	for (i=0; i<15; ++i) {
		printf_syn("0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x\n",
				sinkinfo_all_em[param].px_vi_sample.pb_vi_sample[0][cnt],
				sinkinfo_all_em[param].px_vi_sample.pb_vi_sample[0][cnt+1],
				sinkinfo_all_em[param].px_vi_sample.pb_vi_sample[0][cnt+2],
				sinkinfo_all_em[param].px_vi_sample.pb_vi_sample[0][cnt+3],
				sinkinfo_all_em[param].px_vi_sample.pb_vi_sample[0][cnt+4],
				sinkinfo_all_em[param].px_vi_sample.pb_vi_sample[0][cnt+5],
				sinkinfo_all_em[param].px_vi_sample.pb_vi_sample[0][cnt+6],
				sinkinfo_all_em[param].px_vi_sample.pb_vi_sample[0][cnt+7]
				);

		cnt += 8;
	}

	printf_syn("\nB i-sampling :\n");
	cnt = 0;
	for (i=0; i<15; ++i) {
		printf_syn("0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x\n",
				sinkinfo_all_em[param].px_vi_sample.pb_vi_sample[1][cnt],
				sinkinfo_all_em[param].px_vi_sample.pb_vi_sample[1][cnt+1],
				sinkinfo_all_em[param].px_vi_sample.pb_vi_sample[1][cnt+2],
				sinkinfo_all_em[param].px_vi_sample.pb_vi_sample[1][cnt+3],
				sinkinfo_all_em[param].px_vi_sample.pb_vi_sample[1][cnt+4],
				sinkinfo_all_em[param].px_vi_sample.pb_vi_sample[1][cnt+5],
				sinkinfo_all_em[param].px_vi_sample.pb_vi_sample[1][cnt+6],
				sinkinfo_all_em[param].px_vi_sample.pb_vi_sample[1][cnt+7]
				);

		cnt += 8;
	}

	printf_syn("\nC v-sampling :\n");
	cnt = 0;
	for (i=0; i<15; ++i) {
		printf_syn("0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x\n",
				sinkinfo_all_em[param].px_vi_sample.pc_vi_sample[0][cnt],
				sinkinfo_all_em[param].px_vi_sample.pc_vi_sample[0][cnt+1],
				sinkinfo_all_em[param].px_vi_sample.pc_vi_sample[0][cnt+2],
				sinkinfo_all_em[param].px_vi_sample.pc_vi_sample[0][cnt+3],
				sinkinfo_all_em[param].px_vi_sample.pc_vi_sample[0][cnt+4],
				sinkinfo_all_em[param].px_vi_sample.pc_vi_sample[0][cnt+5],
				sinkinfo_all_em[param].px_vi_sample.pc_vi_sample[0][cnt+6],
				sinkinfo_all_em[param].px_vi_sample.pc_vi_sample[0][cnt+7]
				);

		cnt += 8;
	}

	printf_syn("\nC i-sampling :\n");
	cnt = 0;
	for (i=0; i<15; ++i) {
		printf_syn("0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x\n",
				sinkinfo_all_em[param].px_vi_sample.pc_vi_sample[1][cnt],
				sinkinfo_all_em[param].px_vi_sample.pc_vi_sample[1][cnt+1],
				sinkinfo_all_em[param].px_vi_sample.pc_vi_sample[1][cnt+2],
				sinkinfo_all_em[param].px_vi_sample.pc_vi_sample[1][cnt+3],
				sinkinfo_all_em[param].px_vi_sample.pc_vi_sample[1][cnt+4],
				sinkinfo_all_em[param].px_vi_sample.pc_vi_sample[1][cnt+5],
				sinkinfo_all_em[param].px_vi_sample.pc_vi_sample[1][cnt+6],
				sinkinfo_all_em[param].px_vi_sample.pc_vi_sample[1][cnt+7]
				);

		cnt += 8;
	}
#else
		printf_syn("av-sample:%d, %d, %d, %d, %d\nai-sample:%d, %d, %d, %d, %d\n",
				sinkinfo_all_em[param].px_vi_sample.pa_vi_sample[0][0],  sinkinfo_all_em[param].px_vi_sample.pa_vi_sample[0][8],
				sinkinfo_all_em[param].px_vi_sample.pa_vi_sample[0][16], sinkinfo_all_em[param].px_vi_sample.pa_vi_sample[0][24],
				sinkinfo_all_em[param].px_vi_sample.pa_vi_sample[0][32], sinkinfo_all_em[param].px_vi_sample.pa_vi_sample[1][0],
				sinkinfo_all_em[param].px_vi_sample.pa_vi_sample[1][8],  sinkinfo_all_em[param].px_vi_sample.pa_vi_sample[1][16],
				sinkinfo_all_em[param].px_vi_sample.pa_vi_sample[1][24], sinkinfo_all_em[param].px_vi_sample.pa_vi_sample[1][32]
				);
#endif
}

static void print_sinkinfo_other(int param)
{
#if 0
		printf_syn("voltage:0x%x, 0x%x, 0x%x\ncurrent:0x%x, 0x%x, 0x%x\n"
				"freq::0x%x, 0x%x, 0x%x\ntphase:0x%x, 0x%x, 0x%x\n"
				"activep::0x%x, 0x%x, 0x%x\nreactive:0x%x, 0x%x, 0x%x\n"
				"apparent:0x%x, 0x%x, 0x%x\nfactor:0x%x, 0x%x, 0x%x\n"
				"vol-dist:0x%x, 0x%x, 0x%x\ncurrent-dist:0x%x, 0x%x, 0x%x\n",
				sinkinfo_all_em[param].si_emc_ind_pa.vx,   sinkinfo_all_em[param].si_emc_ind_pb.vx,   sinkinfo_all_em[param].si_emc_ind_pc.vx,
				sinkinfo_all_em[param].si_emc_ind_pa.ix,   sinkinfo_all_em[param].si_emc_ind_pb.ix,   sinkinfo_all_em[param].si_emc_ind_pc.ix,
				sinkinfo_all_em[param].si_emc_ind_pa.hzx,  sinkinfo_all_em[param].si_emc_ind_pb.hzx,  sinkinfo_all_em[param].si_emc_ind_pc.hzx,
				sinkinfo_all_em[param].si_emc_ind_pa.phx,  sinkinfo_all_em[param].si_emc_ind_pb.phx,  sinkinfo_all_em[param].si_emc_ind_pc.phx,
				sinkinfo_all_em[param].si_emc_ind_pa.apx,  sinkinfo_all_em[param].si_emc_ind_pb.apx,  sinkinfo_all_em[param].si_emc_ind_pc.apx,
				sinkinfo_all_em[param].si_emc_ind_pa.rapx, sinkinfo_all_em[param].si_emc_ind_pb.rapx, sinkinfo_all_em[param].si_emc_ind_pc.rapx,
				sinkinfo_all_em[param].si_emc_ind_pa.appx, sinkinfo_all_em[param].si_emc_ind_pb.appx, sinkinfo_all_em[param].si_emc_ind_pc.appx,
				sinkinfo_all_em[param].si_emc_ind_pa.pfx,  sinkinfo_all_em[param].si_emc_ind_pb.pfx,  sinkinfo_all_em[param].si_emc_ind_pc.pfx,
				sinkinfo_all_em[param].si_emc_ind_pa.vdx,  sinkinfo_all_em[param].si_emc_ind_pb.vdx,  sinkinfo_all_em[param].si_emc_ind_pc.vdx,
				sinkinfo_all_em[param].si_emc_ind_pa.cdx,  sinkinfo_all_em[param].si_emc_ind_pb.cdx,  sinkinfo_all_em[param].si_emc_ind_pc.cdx);
		printf_syn("v-sample:0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\ni-sample:0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n",
				sinkinfo_all_em[param].px_vi_sample.pa_vi_sample[0][0], sinkinfo_all_em[param].px_vi_sample.pa_vi_sample[0][1],
				sinkinfo_all_em[param].px_vi_sample.pb_vi_sample[0][0], sinkinfo_all_em[param].px_vi_sample.pb_vi_sample[0][1],
				sinkinfo_all_em[param].px_vi_sample.pc_vi_sample[0][0], sinkinfo_all_em[param].px_vi_sample.pc_vi_sample[0][1],
				sinkinfo_all_em[param].px_vi_sample.pa_vi_sample[1][0], sinkinfo_all_em[param].px_vi_sample.pa_vi_sample[1][1],
				sinkinfo_all_em[param].px_vi_sample.pb_vi_sample[1][0], sinkinfo_all_em[param].px_vi_sample.pb_vi_sample[1][1],
				sinkinfo_all_em[param].px_vi_sample.pc_vi_sample[1][0], sinkinfo_all_em[param].px_vi_sample.pc_vi_sample[1][1]
				);
#else
		print_emc_px_independence_info("emc pa info:", &sinkinfo_all_em[param].si_emc_ind_pa);
		print_emc_px_independence_info("emc pb info:", &sinkinfo_all_em[param].si_emc_ind_pb);
		print_emc_px_independence_info("emc pc info:", &sinkinfo_all_em[param].si_emc_ind_pc);

		printf_syn("em_wire_con_mode:%d\nemc act(react) ee:%d, %d; month_ee:%d\n", register_em_info.em_wire_con_mode[param],
				sinkinfo_all_em[param].emc_dev_info.dev_act_electric_energy,
				sinkinfo_all_em[param].emc_dev_info.dev_react_electric_energy,
				sinkinfo_all_em[param].emc_dev_info.dev_month_electric_energy);

		print_em_px_independence_info("em pa info:", &sinkinfo_all_em[param].si_em_ind_pa);
		print_em_px_independence_info("em pb info:", &sinkinfo_all_em[param].si_em_ind_pb);
		print_em_px_independence_info("em pc info:", &sinkinfo_all_em[param].si_em_ind_pc);
		printf_syn("em act(react) ee inaccuracy:%d, %d\n" "em-temper:%d, em-v-clock:%d, em-v-read-em:%d\n"
				"em act(react) ee inaccuracy:%d, %d\n",
				sinkinfo_all_em[param].em_dev_info.em_act_total_energy,
				sinkinfo_all_em[param].em_dev_info.em_react_total_energy,

				sinkinfo_all_em[param].em_dev_info.em_temper, sinkinfo_all_em[param].em_dev_info.em_v_clock,
				sinkinfo_all_em[param].em_dev_info.em_v_read_em,

				sinkinfo_all_em[param].em_dev_info.em_act_ee_inaccuracy,
				sinkinfo_all_em[param].em_dev_info.em_react_ee_inaccuracy
				);

		print_pt_ct_st_info("pt pa info:", &sinkinfo_all_em[param].pt_info.pt_pa);
		print_pt_ct_st_info("pt pb info:", &sinkinfo_all_em[param].pt_info.pt_pb);
		print_pt_ct_st_info("pt pc info:", &sinkinfo_all_em[param].pt_info.pt_pc);

		print_pt_ct_st_info("ct pa info:", &sinkinfo_all_em[param].ct_info.ct_pa);
		print_pt_ct_st_info("ct pb info:", &sinkinfo_all_em[param].ct_info.ct_pb);
		print_pt_ct_st_info("ct pc info:", &sinkinfo_all_em[param].ct_info.ct_pc);
#endif
}

static void print_5bytes_hex_format(unsigned char *pch, int is_need_lf)
{
	if (is_need_lf)
		printf_syn("%02x%02x%02x%02x%02x\n", *pch, *(pch+1), *(pch+2), *(pch+3), *(pch+4));
	else
		printf_syn("%02x%02x%02x%02x%02x", *pch, *(pch+1), *(pch+2), *(pch+3), *(pch+4));
	return;
}

static void print_sinkinfo_em_data_max_demand(int param)
{
	struct sinkinfo_em_max_demand_st *p = &sinkinfo_all_em[param].si_em_max_demand;

	/* 正向有功 */
	printf_syn("act_max_demand_total:%08x, time:", p->act_max_demand_total);
	print_5bytes_hex_format(p->act_max_demand_time_total, 1);

	printf_syn("act_max_demand_rate1:%08x, time:", p->act_max_demand_rate1);
	print_5bytes_hex_format(p->act_max_demand_time_rate1, 1);
	printf_syn("act_max_demand_rate2:%08x, time:", p->act_max_demand_rate2);
	print_5bytes_hex_format(p->act_max_demand_time_rate2, 1);
	printf_syn("act_max_demand_rate3:%08x, time:", p->act_max_demand_rate3);
	print_5bytes_hex_format(p->act_max_demand_time_rate3, 1);
	printf_syn("act_max_demand_rate4:%08x, time:", p->act_max_demand_rate4);
	print_5bytes_hex_format(p->act_max_demand_time_rate4, 1);

	/* 反向有功 */
	printf_syn("reverse act_max_demand_total:%08x, time:", p->react_max_demand_total);
	print_5bytes_hex_format(p->react_max_demand_time_total, 1);

	printf_syn("reverse act_max_demand_rate1:%08x, time:", p->react_max_demand_rate1);
	print_5bytes_hex_format(p->react_max_demand_time_rate1, 1);
	printf_syn("reverse act_max_demand_rate2:%08x, time:", p->react_max_demand_rate2);
	print_5bytes_hex_format(p->react_max_demand_time_rate2, 1);
	printf_syn("reverse act_max_demand_rate3:%08x, time:", p->react_max_demand_rate3);
	print_5bytes_hex_format(p->react_max_demand_time_rate3, 1);
	printf_syn("reverse act_max_demand_rate4:%08x, time:", p->react_max_demand_rate4);
	print_5bytes_hex_format(p->react_max_demand_time_rate4, 1);

	return;
}

static void print_sinkinfo_em_data_timing_freeze(int param)
{
	int i;
	struct sinkinfo_em_timing_freeze_st *p = sinkinfo_all_em[param].si_em_time_freeze;

	for (i=0; i<ELECTRIC_METER_TIMING_FREEZE_MAX; ++i) {
		printf_syn("----------------%2dth timing freeze data start----------------\n", i+1);

		printf_syn("act_max_demand:%08x, tiem:", p->act_max_demand);
		print_5bytes_hex_format(p->act_max_demand_time, 1);

		printf_syn("reverse_act_max_demand:%08x, tiem:", p->reverse_act_max_demand);
		print_5bytes_hex_format(p->reverse_act_max_demand_time, 1);

		printf_syn("--------freeze time:");
		print_5bytes_hex_format(p->freeze_time, 0);
		printf_syn("--------\n");

		printf_syn("[forward direction]act_elec_energy:%08x, "
			"total act-power:%08x, A-act-power:%08x, B-act-power:%08x, C-act-power:%08x\n",
				p->act_elec_energy,
				p->apxT, p->apxA, p->apxB, p->apxC);

		printf_syn("[reverse direction]act_elec_energy:%08x, "
			"total act-power:%08x, A-act-power:%08x, B-act-power:%08x, C-act-power:%08x\n",
				p->reverse_act_elec_energy,
				p->rapxT, p->rapxA, p->rapxB, p->rapxC);

		printf_syn("----------------%2dth timing freeze data end----------------\n", i+1);

		++p;
	}

	return;
}


extern rt_err_t test_forzen_now(void);

void sinkinfo_test(int cmd, int param)
{
	rt_err_t ret;
	int cnt;

	printf_syn("test enter the function(%d, %d)\n", cmd, param);

	switch (cmd) {
	case 1:
		if (NULL == sinkinfo_all_em || !((param<NUM_OF_COLLECT_EM_MAX) && (param>=0))) {
			printf_syn("func:%s(), line:%d, pointer:0x%x, param:%d(%d)\n",
					sinkinfo_all_em, param, NUM_OF_COLLECT_EM_MAX);
			break;
		}

		ret = rt_sem_take(&sinkinfo_sem, RT_WAITING_FOREVER);
		if (RT_EOK != ret) {
			printf_syn("take sinkinfo_sem fail(%d)\n", ret);
			break;
		}

		print_sinkinfo_harmonic(param);
		print_sinkinfo_vi_sample(param);
		print_sinkinfo_other(param);

		rt_sem_release(&sinkinfo_sem);
		break;

	case 11:
		if (NULL == sinkinfo_all_em || !((param<NUM_OF_COLLECT_EM_MAX) && (param>=0))) {
			printf_syn("func:%s(), line:%d, pointer:0x%x, param:%d(%d)\n",
					sinkinfo_all_em, param, NUM_OF_COLLECT_EM_MAX);
			break;
		}

		ret = rt_sem_take(&sinkinfo_sem, RT_WAITING_FOREVER);
		if (RT_EOK != ret) {
			printf_syn("take sinkinfo_sem fail(%d)\n", ret);
			break;
		}

		print_sinkinfo_harmonic(param);

		rt_sem_release(&sinkinfo_sem);
		break;

	case 12:
		if (NULL == sinkinfo_all_em || !((param<NUM_OF_COLLECT_EM_MAX) && (param>=0))) {
			printf_syn("func:%s(), line:%d, pointer:0x%x, param:%d(%d)\n",
					sinkinfo_all_em, param, NUM_OF_COLLECT_EM_MAX);
			break;
		}

		ret = rt_sem_take(&sinkinfo_sem, RT_WAITING_FOREVER);
		if (RT_EOK != ret) {
			printf_syn("take sinkinfo_sem fail(%d)\n", ret);
			break;
		}

		print_sinkinfo_vi_sample(param);

		rt_sem_release(&sinkinfo_sem);
		break;

	case 13:
		if (NULL == sinkinfo_all_em || !((param<NUM_OF_COLLECT_EM_MAX) && (param>=0))) {
			printf_syn("func:%s(), line:%d, pointer:0x%x, param:%d(%d)\n",
					sinkinfo_all_em, param, NUM_OF_COLLECT_EM_MAX);
			break;
		}

		ret = rt_sem_take(&sinkinfo_sem, RT_WAITING_FOREVER);
		if (RT_EOK != ret) {
			printf_syn("take sinkinfo_sem fail(%d)\n", ret);
			break;
		}

		print_sinkinfo_other(param);

		rt_sem_release(&sinkinfo_sem);
		break;

	case 14:
		if (NULL == sinkinfo_all_em || !((param<NUM_OF_COLLECT_EM_MAX) && (param>=0))) {
			printf_syn("func:%s(), line:%d, pointer:0x%x, param:%d(%d)\n",
					sinkinfo_all_em, param, NUM_OF_COLLECT_EM_MAX);
			break;
		}

		ret = rt_sem_take(&sinkinfo_sem, RT_WAITING_FOREVER);
		if (RT_EOK != ret) {
			printf_syn("take sinkinfo_sem fail(%d)\n", ret);
			break;
		}

		print_sinkinfo_em_data_max_demand(param);

		rt_sem_release(&sinkinfo_sem);
		break;

	case 15:
		if (NULL == sinkinfo_all_em || !((param<NUM_OF_COLLECT_EM_MAX) && (param>=0))) {
			printf_syn("func:%s(), line:%d, pointer:0x%x, param:%d(%d)\n",
					sinkinfo_all_em, param, NUM_OF_COLLECT_EM_MAX);
			break;
		}

		ret = rt_sem_take(&sinkinfo_sem, RT_WAITING_FOREVER);
		if (RT_EOK != ret) {
			printf_syn("take sinkinfo_sem fail(%d)\n", ret);
			break;
		}

		print_sinkinfo_em_data_timing_freeze(param);

		rt_sem_release(&sinkinfo_sem);
		break;

	case 2:
	{
		if (1 == param) {
			static char cmd_485_seq[] = {0xfe, 0xfe, 0xfe, 0xfe, 0x68, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x68, 0x01, 0x02, 0x65, 0xf3, 0x27, 0x16};
			int rx_cnt = 0, err_cnt = 0;
			rt_err_t err;
			char rx_ch;

			send_data_by_485(UART_485_1_DEV_PTR, cmd_485_seq, sizeof(cmd_485_seq));
			printf_syn("485-1 485 cmd had send over\n");

			while (rx_cnt<23 && err_cnt<2) {
				if (RT_EOK == (err=rt_sem_take(&uart485_1_rx_byte_sem, (get_ticks_of_ms(1500))))) {
					while (1 == recv_data_by_485(UART_485_1_DEV_PTR, &rx_ch, 1)) {
						++rx_cnt;
						printf_syn("485-1 recv ind[%3d]:0x%2x\n", rx_cnt, rx_ch);
					}
				} else {
					printf_syn("485-1 recv error(%d)\n", err);
					++err_cnt;
				}
			}
		} else if (2 == param) {
			printf_syn("will invoke em_data_timing_freeze now func....\n");
			test_forzen_now();
			printf_syn("em_data_timing_freeze func run over.\n");
		} else {
			printf_syn("error cmd (%d, %d)\n", cmd, param);
		}
	}
		break;

	case 3:
		print_sink_data();
		break;

	case 4:
		if (param) {
			si_set_is_had_finish_wl_netcfg_flag(TRUE);
		} else {
			si_set_is_had_finish_wl_netcfg_flag(FALSE);
		}
		break;

	case 5:
 
		printf_syn("\nav sampling :\n");	  		
		for(cnt=0 ;cnt<40; cnt++){
			printf_syn("%d  ",AV_HSCD_BUFFER[cnt]);
		}

		printf_syn("\nbv sampling :\n");			
		for(cnt=0 ;cnt<40; cnt++){
			printf_syn("%d  ",BV_HSCD_BUFFER[cnt]);
		}

		printf_syn("\ncv sampling :\n");			
		for(cnt=0 ;cnt<40; cnt++){
			printf_syn("%d  ",CV_HSCD_BUFFER[cnt]);
		}
			  
		printf_syn("\nai sampling :\n");			
		for(cnt=0 ;cnt<40; cnt++){
			printf_syn("%d  ",AI_HSCD_BUFFER[cnt]);
		}
  
		printf_syn("\nbi sampling :\n");			
		for(cnt=0 ;cnt<40; cnt++){
			printf_syn("%d  ",BI_HSCD_BUFFER[cnt]);
		}
  
		printf_syn("\nci sampling :\n");			
		for(cnt=0 ;cnt<40; cnt++){
			printf_syn("%d  ",CI_HSCD_BUFFER[cnt]);
		}
		
		printf_syn("\nxFVAR :\n");			
		for(cnt=0 ;cnt<3; cnt++){  
			printf_syn("0x%x  ",XFVAR_HSCD_BUFFER[cnt]);  
		}   
		printf_syn("\n\n");
		break;

	case 6:
	{
		/* 0x99是广播地址, 0xaa是通配地址 */
		#if 1
		static rt_uint8_t broadcast_addr_645[] = {0x99, 0x99, 0x99, 0x99, 0x99, 0x99};
		#else
		static rt_uint8_t broadcast_addr_645[] = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
		#endif

		static rt_uint8_t data[256];
#if 0
		rt_uint8_t em_sn_645_97[] = {0x08, 0x38, 0x90, 0x06, 0x00, 0x00};
		rt_uint8_t em_sn_645_07[] = {0x06, 0x54, 0x12, 0x01, 0x00, 0x00};
#else
		rt_uint8_t em_sn_645_97[] = {0x00, 0x00, 0x06, 0x90, 0x38, 0x08};
		rt_uint8_t em_sn_645_07[] = {0x00, 0x00, 0x01, 0x12, 0x54, 0x06};
#endif
		rt_uint32_t data_len = 0;
		rt_uint32_t data_flag = 0;
		rt_uint32_t move_len = 0;
		enum frame_error_e result = FRAME_E_ERROR;

		switch (param) {
		case 1:
			printf_syn("++++%02X%02X%02X%02X%02X%02X\n", get_dev_sn_hex_list(em_sn_645_97));
			printf_syn("will read em sn by 645-97...\n");
			if (output645_97power_data_flag(AC_GET_AMMETR_ADDR, &data_flag, &move_len) != RT_EOK) {
					printf_syn("ERROR: func:%s, line(%d)\n", __FUNCTION__, __LINE__);
					break;
			}

			result = get_data_from_645_97(em_sn_645_97, data_flag, data, &data_len, RS485_PORT_USED_BY_645);
			break;

		case 2:
			printf_syn("++++%02X%02X%02X%02X%02X%02X\n", get_dev_sn_hex_list(broadcast_addr_645));
			printf_syn("will read em sn by 645-97...\n");
			if (output645_97power_data_flag(AC_GET_AMMETR_ADDR, &data_flag, &move_len) != RT_EOK) {
					printf_syn("ERROR: func:%s, line(%d)\n", __FUNCTION__, __LINE__);
					break;
			}

			result = get_data_from_645_97(broadcast_addr_645, data_flag, data, &data_len, RS485_PORT_USED_BY_645);
			break;

		case 3:
			printf_syn("++++%02X%02X%02X%02X%02X%02X\n", get_dev_sn_hex_list(em_sn_645_07));
			printf_syn("will read em sn by 645-07...\n");
			if (output645_07power_data_flag(AC_GET_AMMETR_ADDR, &data_flag) != RT_EOK) {
				printf_syn("ERROR: func:%s, line(%d)\n", __FUNCTION__, __LINE__);
				break;
			}

			result = get_data_from_645_07(em_sn_645_07, data_flag, data, &data_len, RS485_PORT_USED_BY_645);
			break;

		case 4:
			printf_syn("++++%02X%02X%02X%02X%02X%02X\n", get_dev_sn_hex_list(broadcast_addr_645));
			printf_syn("will read em sn by 645-07...\n");
			if (output645_07power_data_flag(AC_GET_AMMETR_ADDR, &data_flag) != RT_EOK) {
				printf_syn("ERROR: func:%s, line(%d)\n", __FUNCTION__, __LINE__);
				break;
			}

			result = get_data_from_645_07(broadcast_addr_645, data_flag, data, &data_len, RS485_PORT_USED_BY_645);
			break;

		case 5:
			printf_syn("++++%02X%02X%02X%02X%02X%02X\n", get_dev_sn_hex_list(em_sn_645_07));
			printf_syn("will read em sn by 645-97...\n");
			if (output645_97power_data_flag(AC_GET_AMMETR_ADDR, &data_flag, &move_len) != RT_EOK) {
					printf_syn("ERROR: func:%s, line(%d)\n", __FUNCTION__, __LINE__);
					break;
			}

			result = get_data_from_645_97(em_sn_645_07, data_flag, data, &data_len, RS485_PORT_USED_BY_645);
			break;

		default:
			break;
		}

		if (result != FRAME_E_OK) {
			printf_syn("ERROR: func:%s, line(%d)\n", __FUNCTION__, __LINE__);
		} else {
			printf_syn("return data len:%d, em sn:%02x-%02x-%02x-%02x-%02x-%02x\n", data_len,
					data[0], data[1], data[2], data[3], data[4], data[5]);
		}

	}
		break;

	case 7:
	{
//		set_usart_baud_for_645(2, param);
#if 0 /* 用于测试主em之间分发ptc/ctc数据, add by David */
		extern enum tcp_transport_err_e
			em_distribute_data_by_em_distrib_table(
					struct msfd_report_collection_data_sn_st *data);

		int i;
		struct msfd_report_collection_data_sn_st data;

		rt_strncpy((char *)(data.sn), "1403-01F4321", DEV_SN_MODE_LEN);
		for (i=0; i<3; ++i) {
			data.coll_data.pt_ct_ap_p[i]	= 1+i;
			data.coll_data.pt_ct_app_p[i]	= 2+i;
			data.coll_data.pt_ct_i[i]	= 3+i;
			data.coll_data.pt_ct_v[i]	= 4+i;
		}

		data.last_update_time		= 0x5a;

		em_distribute_data_by_em_distrib_table(&data);

		printf_syn("%s(), line:%d, had distribute data to em\n", __FUNCTION__, __LINE__);
#endif
	}
		break;

	case 8:
	{
		int reg1, reg2, reg3;
		extern void test_ade7880_read_reg(int *reg1, int *reg2, int *reg3);

		test_ade7880_read_reg(&reg1, &reg2, &reg3);
		printf_syn("0x%x, 0x%x, 0x%x\n", reg1, reg2, reg3);
	}
		break;

#if 0 /* 仅用于mib自测 */
	case 9:
		if (NULL == sinkinfo_all_em || !((param<NUM_OF_COLLECT_EM_MAX) && (param>=0))) {
			printf_syn("func:%s(), line:%d, pointer:0x%x, param:%d(%d)\n",
					sinkinfo_all_em, param, NUM_OF_COLLECT_EM_MAX);
			break;
		}
		ret = rt_sem_take(&sinkinfo_sem, RT_WAITING_FOREVER);
		if (RT_EOK != ret) {
			printf_syn("take sinkinfo_sem fail(%d)\n", ret);
			break;
		}

		u8_t time[6]={14,5,28,9,19,30};
		
		sinkinfo_all_em[param].si_emc_ind_pa.vx = 0x12345678;
		sinkinfo_all_em[param].si_emc_ind_pa.ix = 0x12345678;
		sinkinfo_all_em[param].si_emc_ind_pa.hzx= 0x12345678;
		sinkinfo_all_em[param].si_emc_ind_pa.phx = 0x12345678;
		sinkinfo_all_em[param].si_emc_ind_pa.viphx = 0x12345678;
		sinkinfo_all_em[param].si_emc_ind_pa.apx = 0x12345678;
		sinkinfo_all_em[param].si_emc_ind_pa.rapx = 0x12345678;
		sinkinfo_all_em[param].si_emc_ind_pa.appx = 0x12345678;
		sinkinfo_all_em[param].si_emc_ind_pa.pfx = 0x12345678;
		sinkinfo_all_em[param].si_emc_ind_pa.vdx = 0x12345678;
		sinkinfo_all_em[param].si_emc_ind_pa.cdx = 0x12345678;

		sinkinfo_all_em[param].emc_dev_info.dev_act_electric_energy = 0x12345678;
		sinkinfo_all_em[param].emc_dev_info.dev_react_electric_energy = 0x12345678;
		sinkinfo_all_em[param].emc_dev_info.dev_month_electric_energy = 0x12345678;

		sinkinfo_all_em[param].si_em_ind_pa.vx = 0x12345678;
		sinkinfo_all_em[param].si_em_ind_pa.ix = 0x12345678;
		sinkinfo_all_em[param].si_em_ind_pa.hzx= 0x12345678;
		sinkinfo_all_em[param].si_em_ind_pa.phx = 0x12345678;
		sinkinfo_all_em[param].si_em_ind_pa.viphx = 0x12345678;
		sinkinfo_all_em[param].si_em_ind_pa.apx = 0x12345678;
		sinkinfo_all_em[param].si_em_ind_pa.rapx = 0x12345678;
		sinkinfo_all_em[param].si_em_ind_pa.appx = 0x12345678;
		sinkinfo_all_em[param].si_em_ind_pa.pfx = 0x12345678;
		sinkinfo_all_em[param].si_em_ind_pa.vdx = 0x12345678;
		sinkinfo_all_em[param].si_em_ind_pa.cdx = 0x12345678;

		sinkinfo_all_em[param].em_dev_info.em_act_total_energy = 0x12345678;
		sinkinfo_all_em[param].em_dev_info.em_react_total_energy = 0x12345678;
		sinkinfo_all_em[param].em_dev_info.em_act_ee_inaccuracy = 0x1234;
		sinkinfo_all_em[param].em_dev_info.em_react_ee_inaccuracy = 0x1234;

		//sinkinfo_all_em[param].si_em_ind_pa.vx = lwip_htonl(0x12345678);
		//sinkinfo_all_em[param].si_em_ind_pa.ix = lwip_htonl(0x12345678);
		
		sinkinfo_all_em[param].si_em_max_demand.act_max_demand_total = 0x12345678;
		rt_memcpy(sinkinfo_all_em[param].si_em_max_demand.act_max_demand_time_total,time, sizeof(sinkinfo_all_em[param].si_em_max_demand.act_max_demand_time_total)-3);
		sinkinfo_all_em[param].si_em_max_demand.act_max_demand_rate1= 0x12345678;
		rt_memcpy(sinkinfo_all_em[param].si_em_max_demand.act_max_demand_time_rate1,time, sizeof(sinkinfo_all_em[param].si_em_max_demand.act_max_demand_time_rate1)-3);
		sinkinfo_all_em[param].si_em_max_demand.act_max_demand_rate2= 0x12345678;
		rt_memcpy(sinkinfo_all_em[param].si_em_max_demand.act_max_demand_time_rate2,time, sizeof(sinkinfo_all_em[param].si_em_max_demand.act_max_demand_time_rate2)-3);
		sinkinfo_all_em[param].si_em_max_demand.act_max_demand_rate3 = 0x12345678;
		rt_memcpy(sinkinfo_all_em[param].si_em_max_demand.act_max_demand_time_rate3,time, sizeof(sinkinfo_all_em[param].si_em_max_demand.act_max_demand_time_rate3)-3);
		sinkinfo_all_em[param].si_em_max_demand.act_max_demand_rate4 = 0x12345678;
		rt_memcpy(sinkinfo_all_em[param].si_em_max_demand.act_max_demand_time_rate4,time, sizeof(sinkinfo_all_em[param].si_em_max_demand.act_max_demand_time_rate4)-3);
		sinkinfo_all_em[param].si_em_max_demand.react_max_demand_total = 0x12345678;
		rt_memcpy(sinkinfo_all_em[param].si_em_max_demand.react_max_demand_time_total,time, sizeof(sinkinfo_all_em[param].si_em_max_demand.react_max_demand_time_total)-3);
		sinkinfo_all_em[param].si_em_max_demand.react_max_demand_rate1= 0x12345678;
		rt_memcpy(sinkinfo_all_em[param].si_em_max_demand.react_max_demand_time_rate1,time, sizeof(sinkinfo_all_em[param].si_em_max_demand.react_max_demand_time_rate1)-3);
		sinkinfo_all_em[param].si_em_max_demand.react_max_demand_rate2= 0x12345678;
		rt_memcpy(sinkinfo_all_em[param].si_em_max_demand.react_max_demand_time_rate2,time, sizeof(sinkinfo_all_em[param].si_em_max_demand.react_max_demand_time_rate2)-3);
		sinkinfo_all_em[param].si_em_max_demand.react_max_demand_rate3 = 0x12345678;
		rt_memcpy(sinkinfo_all_em[param].si_em_max_demand.react_max_demand_time_rate3,time, sizeof(sinkinfo_all_em[param].si_em_max_demand.react_max_demand_time_rate3)-3);
		sinkinfo_all_em[param].si_em_max_demand.react_max_demand_rate4 = 0x12345678;
		rt_memcpy(sinkinfo_all_em[param].si_em_max_demand.react_max_demand_time_rate4,time, sizeof(sinkinfo_all_em[param].si_em_max_demand.react_max_demand_time_rate4)-3);

		rt_memcpy(sinkinfo_all_em[param].si_em_time_freeze[0].freeze_time,time, sizeof(sinkinfo_all_em[param].si_em_time_freeze[0].freeze_time)-3);
		sinkinfo_all_em[param].si_em_time_freeze[0].act_elec_energy = 0x12345678;
		sinkinfo_all_em[param].si_em_time_freeze[0].reverse_act_elec_energy = 0x12345678;
		sinkinfo_all_em[param].si_em_time_freeze[0].apxT = 0x12345678;
		sinkinfo_all_em[param].si_em_time_freeze[0].apxA = 0x12345678;
		sinkinfo_all_em[param].si_em_time_freeze[0].apxB = 0x12345678;
		sinkinfo_all_em[param].si_em_time_freeze[0].apxC = 0x12345678;
		sinkinfo_all_em[param].si_em_time_freeze[0].rapxT = 0x12345678;
		sinkinfo_all_em[param].si_em_time_freeze[0].rapxA = 0x12345678;
		sinkinfo_all_em[param].si_em_time_freeze[0].rapxB = 0x12345678;
		sinkinfo_all_em[param].si_em_time_freeze[0].rapxC = 0x12345678;
		sinkinfo_all_em[param].si_em_time_freeze[0].act_max_demand = 0x12345678;
		rt_memcpy(sinkinfo_all_em[param].si_em_time_freeze[0].act_max_demand_time,time, sizeof(sinkinfo_all_em[param].si_em_time_freeze[0].act_max_demand_time)-3);
		sinkinfo_all_em[param].si_em_time_freeze[0].reverse_act_max_demand= 0x12345678;
		rt_memcpy(sinkinfo_all_em[param].si_em_time_freeze[0].reverse_act_max_demand_time,time, sizeof(sinkinfo_all_em[param].si_em_time_freeze[0].reverse_act_max_demand_time)-3);
		
		rt_sem_release(&sinkinfo_sem);
		break;
#endif

	default:
		printf_syn("cmd(%d) error", cmd);
		break;
	}

	return;
}

FINSH_FUNCTION_EXPORT(sinkinfo_test, cmd);



#endif

