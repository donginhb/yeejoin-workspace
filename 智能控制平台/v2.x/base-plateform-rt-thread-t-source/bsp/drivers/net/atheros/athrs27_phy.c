/*
 * Copyright (c) 2008, Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <rtdef.h>
#include <stm32_eth.h>
//#include "athrs_phy.h"
//#include "athrs_mac.h"
#include "athrs27_phy.h"
#if 0 //David
/* PHY selections and access functions */

typedef enum {
    PHY_SRCPORT_INFO, 
    PHY_PORTINFO_SIZE,
} PHY_CAP_TYPE;

typedef enum {
    PHY_SRCPORT_NONE,
    PHY_SRCPORT_VLANTAG, 
    PHY_SRCPORT_TRAILER,
} PHY_SRCPORT_TYPE;

#ifdef CONFIG_AR7240_S27_VLAN_IGMP
#define PYTHON_OK   0
#define PYTHON_ERR  1
#define PYTHON_FULL 2
#endif

#define DRV_LOG(DBG_SW, X0, X1, X2, X3, X4, X5, X6)
#define DRV_MSG(x,a,b,c,d,e,f)
#define DRV_PRINT(DBG_SW,X)

#define ATHR_LAN_PORT_VLAN          1
#define ATHR_WAN_PORT_VLAN          2
#define ENET_UNIT_LAN 1
#define ENET_UNIT_WAN 0

#define TRUE    1
#define FALSE   0
#endif
#define ATHR_PHY0_ADDR   0x0
#define ATHR_PHY1_ADDR   0x1
#define ATHR_PHY2_ADDR   0x2
#define ATHR_PHY3_ADDR   0x3
#define ATHR_PHY4_ADDR   0x4

#define ATHR_PHY_MAX     0x5

#define printk printf_syn

#ifdef S27_PHY_DEBUG
#define DPRINTF_PHY(_fmt,...) do {         \
                printk(MODULE_NAME":"_fmt, __VA_ARGS__);      \
} while (0)
#else
#define DPRINTF_PHY(_fmt,...) 
#endif

#ifdef ATHR_GMAC_DEBUG
#define DPRINTF(_fmt,...) do {         \
            printk("GMAC:"_fmt, __VA_ARGS__);      \
} while (0)
#else
#define DPRINTF(_fmt,...)
#endif


#if 0 //David

extern int phy_in_reset;
extern int athr_gmac_check_link(athr_gmac_t *mac,int phyUnit);

/*
 * Track per-PHY port information.
 */
typedef struct {
    BOOL   isEnetPort;       /* normal enet port */
    BOOL   isPhyAlive;       /* last known state of link */
    int    ethUnit;          /* MAC associated with this phy port */
    uint32_t phyBase;
    uint32_t phyAddr;          /* PHY registers associated with this phy port */
    uint32_t VLANTableSetting; /* Value to be written to VLAN table */
} athrPhyInfo_t;

static athr_gmac_t *athr_macs[2];
/*
 * Per-PHY information, indexed by PHY unit number.
 */
static athrPhyInfo_t athrPhyInfo[] = {

    {TRUE,   /* port 1 -- LAN port 1 */
     FALSE,
#ifdef CONFIG_ATHR_PHY_SWAP
     ENET_UNIT_WAN,
#else
     ENET_UNIT_LAN,
#endif
     1,
     ATHR_PHY0_ADDR,
     ATHR_LAN_PORT_VLAN
    },

    {TRUE,   /* port 2 -- LAN port 2 */
     FALSE,
     ENET_UNIT_LAN,
     1,
     ATHR_PHY1_ADDR, 
     ATHR_LAN_PORT_VLAN
    },

    {TRUE,   /* port 3 -- LAN port 3 */
     FALSE,
     ENET_UNIT_LAN,
     1,
     ATHR_PHY2_ADDR, 
     ATHR_LAN_PORT_VLAN
    },

    {TRUE,   /* port 4 --  LAN port 4 */
     FALSE,
     ENET_UNIT_LAN,     
     1,
     ATHR_PHY3_ADDR, 
     ATHR_LAN_PORT_VLAN   /* Send to all ports */
    },
    
    {TRUE,  /* port 5 -- WAN Port 5 */
     FALSE,
#ifdef CONFIG_ATHR_PHY_SWAP
     ENET_UNIT_LAN,
#else
     ENET_UNIT_WAN,
#endif
     1,
     ATHR_PHY4_ADDR, 
     ATHR_LAN_PORT_VLAN    /* Send to all ports */
    },

    {FALSE,   /* port 0 -- cpu port 0 */
     TRUE,
     ENET_UNIT_LAN,
     1,
     0x00,
     ATHR_LAN_PORT_VLAN
    },

};
#endif
static uint8_t athr27_init_flag = 0;


#define ATHR_GMAC_ADDR_SHIFT                 8

#define phy_reg_read    ETH_ReadPHYRegister
#define phy_reg_write   ETH_WritePHYRegister

static unsigned int s27_rd_phy(int ethUnit,unsigned int phy_addr, unsigned int reg_addr);
static void s27_wr_phy(int ethUnit,unsigned int phy_addr, unsigned int reg_addr, unsigned int write_data);


#if 0 /* David */
int athrs27_reg_init(void)
{
    uint32_t rd_data;

    /* if using header for register configuration, we have to     */
    /* configure s27 register after frame transmission is enabled */
    if (athr27_init_flag)
        return 0;
    
    athrs27_reg_rmw(0x8,(1<<28));  /* Set WAN port is connected to GE0 */

#if defined(S27_FORCE_100M)
    athrs27_force_100M(ATHR_PHY4_ADDR,1);
#elif  defined(S27_FORCE_10M)
    athrs27_force_10M(ATHR_PHY4_ADDR,1);
#else
    s27_wr_phy(1,ATHR_PHY4_ADDR,ATHR_PHY_CONTROL,0x9000);

    /* Class A setting for 100M */
    s27_wr_phy(1,ATHR_PHY4_ADDR, 29, 5);
    s27_wr_phy(1,ATHR_PHY4_ADDR, 30, (s27_rd_phy(1,ATHR_PHY4_ADDR, 30)&((~2)&0xffff)));
#endif

    /* Disable WAN mac inside S27 */
    athrs27_reg_write(PORT_STATUS_REGISTER5,0x0);
#if 0
    /* Enable WAN mac inside S27 */
    if (mac_has_flag(mac,ETH_SWONLY_MODE)) 
        athrs27_reg_write(PORT_STATUS_REGISTER5,0x200);
#endif
    /* Enable MDIO Access if PHY is Powered-down */
    s27_wr_phy(1,ATHR_PHY4_ADDR,ATHR_DEBUG_PORT_ADDRESS,0x3);
    rd_data = s27_rd_phy(1,ATHR_PHY4_ADDR,ATHR_DEBUG_PORT_DATA);
    s27_wr_phy(1,ATHR_PHY4_ADDR,ATHR_DEBUG_PORT_ADDRESS,0x3);
    s27_wr_phy(1,ATHR_PHY4_ADDR,ATHR_DEBUG_PORT_DATA,(rd_data & 0xfffffeff) );

#if 0
    if (mac_has_flag(mac,ATHR_S27_HEADER))
        athrs27_reg_write(PORT_CONTROL_REGISTER0, 0x4804);
    else
#endif
        athrs27_reg_write(PORT_CONTROL_REGISTER0, 0x4004);

    athr27_init_flag = 1;

    DPRINTF(MODULE_NAME":OPERATIONAL_MODE_REG0:%x\n",athrs27_reg_read(OPERATIONAL_MODE_REG0));
    DPRINTF(MODULE_NAME":REG 0x4-->:%x\n",athrs27_reg_read(0x4));
    DPRINTF(MODULE_NAME":REG 0x2c-->:%x\n",athrs27_reg_read(0x2c));
    DPRINTF(MODULE_NAME":REG 0x8-->:%x\n",athrs27_reg_read(0x8));

    return 0;
}

unsigned int athrs27_reg_read(unsigned int s27_addr)
{
    unsigned int addr_temp;
    unsigned int s27_rd_csr_low, s27_rd_csr_high, s27_rd_csr;
    unsigned int data;
    unsigned int phy_address, reg_address;

    addr_temp = s27_addr >>2;
    data = addr_temp >> 7;

    phy_address = 0x1f;
    reg_address = 0x10;

    phy_reg_write(phy_address, reg_address, data);

    phy_address = (0x17 & ((addr_temp >> 4) | 0x10));
    reg_address = ((addr_temp << 1) & 0x1e);
    s27_rd_csr_low = (uint32_t) phy_reg_read(phy_address, reg_address);

    reg_address = reg_address | 0x1;
    s27_rd_csr_high = (uint32_t) phy_reg_read(phy_address, reg_address);
    s27_rd_csr = (s27_rd_csr_high << 16) | s27_rd_csr_low ;

    return(s27_rd_csr);
}

void athrs27_reg_write(unsigned int s27_addr, unsigned int s27_write_data)
{
    unsigned int addr_temp;
    unsigned int data;
    unsigned int phy_address, reg_address;

    addr_temp = (s27_addr ) >>2;
    data = addr_temp >> 7;

    phy_address = 0x1f;
    reg_address = 0x10;

    phy_reg_write(phy_address, reg_address, data);

    phy_address = (0x17 & ((addr_temp >> 4) | 0x10));

    reg_address = (((addr_temp << 1) & 0x1e) | 0x1);
    data = (s27_write_data >> 16) & 0xffff;
    phy_reg_write(phy_address, reg_address, data);

    reg_address = ((addr_temp << 1) & 0x1e);
    data = s27_write_data  & 0xffff;
    phy_reg_write(phy_address, reg_address, data);
}

void athrs27_reg_rmw(unsigned int s27_addr, unsigned int s27_write_data)
{
    int val = athrs27_reg_read(s27_addr);
    athrs27_reg_write(s27_addr,(val | s27_write_data));
}


static unsigned int s27_rd_phy(int ethUnit,unsigned int phy_addr, unsigned int reg_addr)
{

     unsigned int rddata, i = 100;

     if(phy_addr >= ATHR_PHY_MAX) {
         DPRINTF("%s:Error invalid Phy Address:0x%x\n",__func__,phy_addr);
         return -1 ;
     }


    /* MDIO_CMD is set for read */

    rddata = athrs27_reg_read(0x98);
    rddata = (rddata & 0x0) | (reg_addr<<16) 
              | (phy_addr<<21) | (1<<27) 
              | (1<<30) | (1<<31) ;

    athrs27_reg_write(0x98, rddata);

    rddata = athrs27_reg_read(0x98);
    rddata = rddata & (1<<31);

    /* Check MDIO_BUSY status */
    while(rddata && --i){
        rddata = athrs27_reg_read(0x98);
        rddata = rddata & (1<<31);
    }

    if(i <= 0)
      printk("ERROR:%s failed:phy:%d reg:%X rd_data:%X\n",
                __func__,phy_addr,reg_addr,rddata);
    /* Read the data from phy */

    rddata = athrs27_reg_read(0x98);
    rddata = rddata & 0xffff;
    return(rddata);
}

static void s27_wr_phy(int ethUnit,unsigned int phy_addr, unsigned int reg_addr, unsigned int write_data)
{
    unsigned int rddata,i = 100;

    if(phy_addr >= ATHR_PHY_MAX) {
        DPRINTF("%s:Error invalid Phy Address:0x%x\n",__func__,phy_addr);
        return ;
    }

    /* MDIO_CMD is set for read */


    rddata = athrs27_reg_read(0x98);
    rddata = (rddata & 0x0) | (write_data & 0xffff)
               | (reg_addr<<16) | (phy_addr<<21)
               | (0<<27) | (1<<30) | (1<<31) ;


    athrs27_reg_write(0x98, rddata);


    rddata = athrs27_reg_read(0x98);
    rddata = rddata & (1<<31);

    /* Check MDIO_BUSY status */
    while(rddata && --i){
        rddata = athrs27_reg_read(0x98);
        rddata = rddata & (1<<31);
    }
    if(i <= 0)
      printk("ERROR:%s failed:phy:%d reg%X\n",__func__,phy_addr,reg_addr);

}

#else /* willy */
#if 0
void phy_reg_write(unsigned int phy_address, unsigned char reg_address, unsigned short data)
{
	//CPU平台的MDIO write 函数
}

unsigned short phy_reg_read(unsigned int phy_address, unsigned char reg_address)
{
	//CPU平台的MDIO read 函数
}
#endif

/* switch internal registers,  reg_addr[18:9] */
#define SIREG_REG_H_PHY_ADDR (0X1F)
#define SIREG_REG_H_REG_ADDR (0X10)


/*
 * write switch internal register, datasheet page28
 *
 * 2012-03-31 18:28:51, update by David
 */
void athrs27_reg_write(unsigned int s27_addr, unsigned int s27_write_data)
{
	unsigned int addr_temp;
	unsigned int data;
	unsigned int phy_address, reg_address;

	addr_temp = s27_addr  >> 2; /* low 2 bits is not using */
	data 	  = addr_temp >> 7; /* 移除了地址的低9bits */

	phy_address = SIREG_REG_H_PHY_ADDR; /* 与ds不一致 */
	reg_address = SIREG_REG_H_REG_ADDR; /* 与ds不一致 */
	phy_reg_write(phy_address, reg_address, data); /* send reg_a[18:9] */

	phy_address = 0x10 | ((addr_temp >> 4) & 0x07);
	reg_address = ((addr_temp << 1) & 0x1e) | 0x1;
	data 	    = (s27_write_data >> 16) & 0xffff;
	phy_reg_write(phy_address, reg_address, data); /* send reg_a[7:1] and data[31:16] */

	reg_address = (addr_temp << 1) & 0x1e;
	data 	    = s27_write_data  & 0xffff;
	phy_reg_write(phy_address, reg_address, data); /* send reg_a[7:1] and data[15:0] */

	return;
}

 
/*
 * read switch internal register, datasheet page28
 *
 * 2012-04-01 10:05:28, update by David
 */
unsigned int athrs27_reg_read(unsigned int s27_addr)
{
	unsigned int addr_temp;
	unsigned int s27_rd_csr_low, s27_rd_csr_high, s27_rd_csr;
	unsigned int data;
	unsigned int phy_address, reg_address;

	addr_temp = s27_addr  >> 2; /* low 2 bits is not using */
	data 	  = addr_temp >> 7; /* 移除了地址的低9bits */

	phy_address = SIREG_REG_H_PHY_ADDR;
	reg_address = SIREG_REG_H_REG_ADDR;
	phy_reg_write(phy_address, reg_address, data); /* send reg_a[18:9] */

	phy_address 	= 0x17 & ((addr_temp >> 4) | 0x10);
	reg_address 	= (addr_temp << 1) & 0x1e;
	/* send reg_a[7:1](phy addr) and read data[15:0] */
	s27_rd_csr_low  = phy_reg_read(phy_address, reg_address);

	reg_address 	= reg_address | 0x1;
	/* send reg_a[7:1](phy addr) and read data[31:16] */
	s27_rd_csr_high = phy_reg_read(phy_address, reg_address);

	s27_rd_csr 	= (s27_rd_csr_high << 16) | s27_rd_csr_low ;
	return(s27_rd_csr);
}

 

void ar8228_driver_init()
{
	//......

	/*
	 * #define S27_FLD_MASK_REG  0x002c
	 * ds -- page60
	 */
	athrs27_reg_write(0x2c,  0xfe7f007f);

	/*
	 * #define ATHR_LATCH_LINK_PASS  0x0004 
	 * #define PHY_LINK_CHANGE_REG 	 0x4
	 ** #define OPERATIONAL_MODE_REG0 0x4
	 ** bit17 -- mac0_rmii_en
	 */
	athrs27_reg_write(0x04,  0x80020000);

	/*
	 * #define ATHR_CTRL_SPEED_FULL_DUPLEX     0x0100
	 * #define ATHR_LINK_100BASETX_FULL_DUPLEX 0x0100
	 * #define ATHR_ADVERTISE_100FULL          0x0100
	 * #define ATHR_ADVERTISE_1000HALF	   0x0100
	 * #define ATHER_SERDES_BEACON             0x0100
	 * #define PORT_STATUS_REGISTER0           0x0100 
	 */
	//athrs27_reg_write(0x100, 0x7d);

	//......

	return;
}

#endif


