#ifndef _TCA8418_H_
#define _TCA8418_H_

#include <sys_app_cfg.h>

/*
 * tca8418addr_read/write + tca8418_reg_addr + data
 * 
 * the default value in all tca8418 registers is 0.
 */
#define TCA8418_I2C_ADDRCMD_READ  (0X69) 
#define TCA8418_I2C_ADDRCMD_WRITE (0X68)

/*
 * interupt pin connect to stm32 pin88-PD7
 */
#if 0==USE_KEY_CTR_CHIP
/* matrix keyscan */
enum key_value {
 	KEYV_R0C0 = 0xee,
 	KEYV_R0C1 = 0xed,
 	KEYV_R0C2 = 0xeb,
 	KEYV_R0C3 = 0xe7,
 	KEYV_R1C0 = 0xde,
 	KEYV_R1C1 = 0xdd,
 	KEYV_R1C2 = 0xdb,
 	KEYV_R1C3 = 0xd7,
 	KEYV_R2C0 = 0xbe,
 	KEYV_R2C1 = 0xbd,
 	KEYV_R2C2 = 0xbb,
 	KEYV_R2C3 = 0xb7,
 	KEYV_R3C0 = 0x7e,
 	KEYV_R3C1 = 0x7d,
 	KEYV_R3C2 = 0x7b,
 	KEYV_R3C3 = 0x77,
};
#elif 1==USE_KEY_CTR_RA8875
/* ra8875 */
enum key_value {
	KEYV_R0C0 = 0x00,
	KEYV_R0C1 = 0x01,
	KEYV_R0C2 = 0x02,
	KEYV_R0C3 = 0x03,
	KEYV_R0C4 = 0x04,
	KEYV_R1C0 = 0x10,
	KEYV_R1C1 = 0x11,
	KEYV_R1C2 = 0x12,
	KEYV_R1C3 = 0x13,
	KEYV_R1C4 = 0x14,
	KEYV_R2C0 = 0x20,
	KEYV_R2C1 = 0x21,
	KEYV_R2C2 = 0x22,
	KEYV_R2C3 = 0x23,
	KEYV_R2C4 = 0x24,
	KEYV_R3C0 = 0x30,
	KEYV_R3C1 = 0x31,
	KEYV_R3C2 = 0x32,
	KEYV_R3C3 = 0x33,
	KEYV_R3C4 = 0x34,
};
#elif 1==USE_KEY_CTR_TCA8418
/* tca8418 */
enum key_value {
	KEYV_R0C0 = 1 ,
	KEYV_R0C1 = 2 ,
	KEYV_R0C2 = 3 ,
	KEYV_R0C3 = 4 ,
	KEYV_R0C4 = 5 ,
	KEYV_R0C5 = 6 ,
	KEYV_R0C6 = 7 ,
	KEYV_R0C7 = 8 ,
	KEYV_R0C8 = 9 ,
	KEYV_R0C9 = 10,
	KEYV_R1C0 = 11,
	KEYV_R1C1 = 12,
	KEYV_R1C2 = 13,
	KEYV_R1C3 = 14,
	KEYV_R1C4 = 15,
	KEYV_R1C5 = 16,
	KEYV_R1C6 = 17,
	KEYV_R1C7 = 18,
	KEYV_R1C8 = 19,
	KEYV_R1C9 = 20,
	KEYV_R2C0 = 21,
	KEYV_R2C1 = 22,
	KEYV_R2C2 = 23,
	KEYV_R2C3 = 24,
	KEYV_R2C4 = 25,
	KEYV_R2C5 = 26,
	KEYV_R2C6 = 27,
	KEYV_R2C7 = 28,
	KEYV_R2C8 = 29,
	KEYV_R2C9 = 30,
	KEYV_R3C0 = 31,
	KEYV_R3C1 = 32,
	KEYV_R3C2 = 33,
	KEYV_R3C3 = 34,
	KEYV_R3C4 = 35,
	KEYV_R3C5 = 36,
	KEYV_R3C6 = 37,
	KEYV_R3C7 = 38,
	KEYV_R3C8 = 39,
	KEYV_R3C9 = 40,
	KEYV_R4C0 = 41,
	KEYV_R4C1 = 42,
	KEYV_R4C2 = 43,
	KEYV_R4C3 = 44,
	KEYV_R4C4 = 45,
	KEYV_R4C5 = 46,
	KEYV_R4C6 = 47,
	KEYV_R4C7 = 48,
	KEYV_R4C8 = 49,
	KEYV_R4C9 = 50,
	KEYV_R5C0 = 51,
	KEYV_R5C1 = 52,
	KEYV_R5C2 = 53,
	KEYV_R5C3 = 54,
	KEYV_R5C4 = 55,
	KEYV_R5C5 = 56,
	KEYV_R5C6 = 57,
	KEYV_R5C7 = 58,
	KEYV_R5C8 = 59,
	KEYV_R5C9 = 60,
	KEYV_R6C0 = 61,
	KEYV_R6C1 = 62,
	KEYV_R6C2 = 63,
	KEYV_R6C3 = 64,
	KEYV_R6C4 = 65,
	KEYV_R6C5 = 66,
	KEYV_R6C6 = 67,
	KEYV_R6C7 = 68,
	KEYV_R6C8 = 69,
	KEYV_R6C9 = 70,
	KEYV_R7C0 = 71,
	KEYV_R7C1 = 72,
	KEYV_R7C2 = 73,
	KEYV_R7C3 = 74,
	KEYV_R7C4 = 75,
	KEYV_R7C5 = 76,
	KEYV_R7C6 = 77,
	KEYV_R7C7 = 78,
	KEYV_R7C8 = 79,
	KEYV_R7C9 = 80,
};

#endif


#define REG_CFG        		 0x01
#define REG_INT_STAT   		     0x02
#define REG_KEY_LCK_EC 		     0x03
#define REG_KEY_EVENT_A              0x04
#define REG_KEY_EVENT_B              0x05
#define REG_KEY_EVENT_C              0x06
#define REG_KEY_EVENT_D              0x07
#define REG_KEY_EVENT_E              0x08
#define REG_KEY_EVENT_F              0x09
#define REG_KEY_EVENT_G              0x0A
#define REG_KEY_EVENT_H              0x0B
#define REG_KEY_EVENT_I              0x0C
#define REG_KEY_EVENT_J              0x0D
#define REG_KP_LCK_TIMER             0x0E
#define REG_Unlock1                  0x0F
#define REG_Unlock2                  0x10
#define REG_GPIO_INT_STAT1           0x11
#define REG_GPIO_INT_STAT2           0x12
#define REG_GPIO_INT_STAT3           0x13
#define REG_GPIO_DAT_STAT1           0x14
#define REG_GPIO_DAT_STAT2           0x15
#define REG_GPIO_DAT_STAT3           0x16
#define REG_GPIO_DAT_OUT1            0x17
#define REG_GPIO_DAT_OUT2            0x18
#define REG_GPIO_DAT_OUT3            0x19
#define REG_GPIO_INT_EN1             0x1A
#define REG_GPIO_INT_EN2             0x1B
#define REG_GPIO_INT_EN3             0x1C
#define REG_KP_GPIO1                 0x1D
#define REG_KP_GPIO2                 0x1E
#define REG_KP_GPIO3                 0x1F
#define REG_GPI_EM1 		 0x20
#define REG_GPI_EM2	             0x21
#define REG_GPI_EM3	             0x22
#define REG_GPIO_DIR1                0x23
#define REG_GPIO_DIR2                0x24
#define REG_GPIO_DIR3                0x25
#define REG_GPIO_INT_LVL1            0x26
#define REG_GPIO_INT_LVL2            0x27
#define REG_GPIO_INT_LVL3            0x28
#define REG_DEBOUNCE_DIS1            0x29
#define REG_DEBOUNCE_DIS2            0x2A
#define REG_DEBOUNCE_DIS3            0x2B
#define REG_GPIO_PULL1               0x2C
#define REG_GPIO_PULL2               0x2D
#define REG_GPIO_PULL3               0x2E

extern void tca8418_init(void);
extern int tca8418_read_reg(int reg_addr, char *reg_data);
extern int tca8418_write_reg(int reg_addr, char byte);



#endif
