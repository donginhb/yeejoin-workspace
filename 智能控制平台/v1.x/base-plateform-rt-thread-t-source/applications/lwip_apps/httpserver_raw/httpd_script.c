/*
 *该文件是httpd的脚本文件
 */
 
#include <string.h>

#include "httpd.h"
#include "httpd_script.h"

#include "lwip/debug.h"
#include "lwipusrlib.h"

#include <board.h>
#include <stm32f10x.h>
#include <sys_cfg_api.h>
#include <syscfgdata.h>


#if LWIP_HTTPD_CGI /* add by david */


#ifndef HTTPD_DEBUG
#define HTTPD_DEBUG         LWIP_DBG_OFF
#endif

#define DATA_SET_SUCC		"/setsucc.html"
#define LOCK_OPEN_SUCC   	"/lock_off.html" /*add by flyingbug*/
#define LOCK_CLOSE_SUCC 	"/lock_on.html" /*add by flyingbug*/

struct string_int_pairs {
    char *name;
    long value;
};

/* 名字凑足3个字节, 可以用unsigned long int型整数对比 */
struct string_int_pairs ipcfg_param_name[] = {
    {"im0", ~0}, /* ipmode0 */
    {"a03", ~0}, /* ipaddr03 */
    {"a02", ~0}, /* ipaddr02 */
    {"a01", ~0}, /* ipaddr01 */
    {"a00", ~0}, /* ipaddr00 */
    {"m03", ~0}, /* netmask03 */
    {"m02", ~0}, /* netmask02 */
    {"m01", ~0}, /* netmask01 */
    {"m00", ~0}, /* netmask00 */
    {"g03", ~0}, /* gw03 */
    {"g02", ~0}, /* gw02 */
    {"g01", ~0}, /* gw01 */
    {"g00", ~0}  /* gw00 */

};
#define NUM_OF_IPCFG_PARAM_NAEM  (sizeof(ipcfg_param_name)/sizeof(struct string_int_pairs))
#define IPCFG_INTERFACE0_BASE_INDEX      0
#define NUM_OF_IPCFG_PARAM_PER_INTERFACE 13



/* 名字凑足3个字节, 可以用unsigned long int型整数对比 */
struct string_int_pairs rs232cfg_param_name[] = {
    {"br0", ~0}, /* baudrate0 */
    {"db0", ~0}, /* databits0 */
    {"pb0", ~0}, /* paritybit0 */
    {"sb0", ~0}, /* stopbit0 */

    {"br1", ~0}, /* baudrate1 */
    {"db1", ~0}, /* databits1 */
    {"pb1", ~0}, /* paritybit1 */
    {"sb1", ~0}  /* stopbit1 */
};
#define NUM_OF_RS232CFG_PARAM_NAEM (sizeof(rs232cfg_param_name)/sizeof(struct string_int_pairs))
#define RS232CFG_INTERFACE0_BASE_INDEX      0
#define RS232CFG_INTERFACE1_BASE_INDEX      4
#define NUM_OF_RS232CFG_PARAM_PER_INTERFACE 4


static int init_rs232cfg_param_name(struct string_int_pairs *rs232cfg_param);
static int init_ipcfg_param_name(struct string_int_pairs *ipcfg_param);
static int rs232cfg_param_to_rs232cfg(struct string_int_pairs *rs232cfg_param, struct rs232_param *rs232cfg, int intno);
static int ipcfg_param_to_ipcfg(struct string_int_pairs *ipcfg_param, struct ip_param *ipcfg, int intno);
static int save_login_data(struct login_param *logindata);
static int save_rs232cfg_data(struct rs232_param *rs232cfg, int intno);
static int save_ipcfg_data(struct ip_param *ipcfg, int intno);



/* 返回0表示成功, 否则, 表示失败 */
int str2l(char *str, int len, long *value)
{
    long tmp;
    int i;
    #define IS_NUMERIC_CHAR(c) (c>='0' && c<='9')

    if (NULL==str || NULL==value)
        return -1;

    tmp = 0;
    for (i=0; i<len; ++i) {
        if (IS_NUMERIC_CHAR(*str)) {
            tmp *= 10;
            tmp += *str - '0';
            ++str;
        } else {
            return -1;
        }
    }
    
    *value = tmp;
    return 0;
}

#if 0
/* 返回值为字符串长度 */
int strlen(char *str)
{
    int i = 0;

    if (NULL==str)
        return 0;

    while (*str++)
        ++i;

    return i;    
}
#endif

const char *deal_ipcfg_data(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    int i, j;
    unsigned long temp1, temp2;
    struct ip_param ipcfg;

    init_ipcfg_param_name(ipcfg_param_name);
    
    LWIP_DEBUGF(HTTPD_DEBUG, ("func:%s, iIndex:%d, iNumParams:%d \n", __FUNCTION__, iIndex, iNumParams));
    for (i=0; i<iNumParams; ++i) {
        httpd_print(("[%d] param:%s, value:%s \n", i, pcParam[i], pcValue[i]));
    }

    for (i=0; i<iNumParams; ++i) {
        temp1 = *((unsigned long*)pcParam[i]);
        for (j=0; j<NUM_OF_IPCFG_PARAM_NAEM; ++j) {
            temp2 = *((unsigned long*)(ipcfg_param_name[j].name));
            if (temp1 == temp2) {
                str2l(pcValue[i], strlen(pcValue[i]), &(ipcfg_param_name[j].value));
                break;
            }
        }
    }

    for (i=0; i<iNumParams; ++i) {
        httpd_print(("name:%s, value:%lu \n", ipcfg_param_name[i].name, ipcfg_param_name[i].value));
    }

    if (~0 != ipcfg_param_name[0].value) {
        i = 0;
    } else {
        i = 1;
    }

    if (0 == ipcfg_param_to_ipcfg(ipcfg_param_name, &ipcfg, i)) {
        save_ipcfg_data(&ipcfg, i);
    } else {
        httpd_print(("ipcfg_param_to_ipcfg fail!\n"));
    }
    
    return DATA_SET_SUCC;
}

const char *deal_rs232cfg_data(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    int i, j;
    unsigned long temp1, temp2;
    struct rs232_param rs232cfg;
    
    init_rs232cfg_param_name(rs232cfg_param_name);
    
    LWIP_DEBUGF(HTTPD_DEBUG, ("func:%s, iIndex:%d, iNumParams:%d \n", __FUNCTION__, iIndex, iNumParams));
    for (i=0; i<iNumParams; ++i) {
        httpd_print(("[%d] param:%s, value:%s \n", i, pcParam[i], pcValue[i]));
    }

    for (i=0; i<iNumParams; ++i) {
        temp1 = *((unsigned long*)pcParam[i]);
        for (j=0; j<NUM_OF_RS232CFG_PARAM_NAEM; ++j) {
            temp2 = *((unsigned long*)(rs232cfg_param_name[j].name));
            if (temp1 == temp2) {
                str2l(pcValue[i], strlen(pcValue[i]), &(rs232cfg_param_name[j].value));
                break;
            }
        }
    }

    for (i=0; i<NUM_OF_RS232CFG_PARAM_NAEM; ++i) {
        httpd_print(("name:%s, value:%lu \n", rs232cfg_param_name[i].name, rs232cfg_param_name[i].value));
    }

    if (~0 != rs232cfg_param_name[0].value) {
        i = 0;
    } else {
        i = 1;
    }

    rs232cfg_param_to_rs232cfg(rs232cfg_param_name, &rs232cfg, i);
    if (0 == save_rs232cfg_data(&rs232cfg, i)) {
        do_set_rs232cfg(&rs232cfg, i);
    }

   return DATA_SET_SUCC;
}

#define USR_NAME_PW_MATCH       "/index.html"
#define USR_NAME_PW_NOT_MATCH   "/loginerr.html"
#define USR_PW_CHANGE           "/login.html"
#define USR_PW_CHANGE_O_ERR	"/old_pwd_err.html"
#define USR_PW_CHANGE_N_ERR	"/new_pwd_err.html"
const char *deal_login_data(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    int i;
    struct usr_pw_pair tmp;

    httpd_print( ("func:%s, iIndex:%d, iNumParams:%d \n", __FUNCTION__, iIndex, iNumParams));

    tmp.usr[0] = '\0';
    tmp.pw[0]  = '\0';
    for (i=0; i<iNumParams; ++i) {
        httpd_print(("[%d] param:%s, value:%s \n", i, pcParam[i], pcValue[i]));

        if (0 == strcmp(pcParam[i], "username"))
            strcpy(tmp.usr, pcValue[i]);
        else if (0 == strcmp(pcParam[i], "pw"))
            strcpy(tmp.pw, pcValue[i]);
    }

    if (is_usr_pw_matching(tmp.usr, tmp.pw))
        return USR_NAME_PW_MATCH;
    else
        return USR_NAME_PW_NOT_MATCH;
}

/*
 * creat by flyingbug
 * modify by David 2012-04-09 15:41:16
 *
 * pw -- old password
 * newpw   -- new pw
 * newpwag -- 
 *
 */
const char *deal_ch_psword_data(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    int i;
    struct login_param logindata;
    char pw[PW_LEN_MAX+1];
    char newpw[PW_LEN_MAX+1];
    char newpwag[PW_LEN_MAX+1];

    httpd_print( ("func:%s, iIndex:%d, iNumParams:%d \n", __FUNCTION__, iIndex, iNumParams));

    pw[0]  = '\0';
    newpw[0]  = '\0';
    newpwag[0]  = '\0';
    for (i=0; i<iNumParams; ++i) {
        httpd_print(("[%d] param:%s, value:%s \n", i, pcParam[i], pcValue[i]));

        if (0 == rt_strcmp(pcParam[i], "pw")) {
            rt_strncpy(pw, pcValue[i], PW_LEN_MAX);
            if (!is_usr_pw_matching("admin", pw))
            	return USR_PW_CHANGE_O_ERR;
            else
            	continue;
        } else if (0 == rt_strcmp(pcParam[i], "newpw")) {
            	rt_strncpy(newpw, pcValue[i], PW_LEN_MAX);
 		continue;
	} else if (0 == rt_strcmp(pcParam[i], "newpwag")) {
                rt_strncpy(newpwag, pcValue[i], PW_LEN_MAX);
		continue;
 	} else {
	        httpd_print(("errr param name.\n"));
		return USR_PW_CHANGE_N_ERR;
 	}
    }

    if (newpw[0]!='\0' && newpwag[0]!='\0' && (0 == rt_strcmp(newpw, newpwag))) {
    	rt_strncpy(logindata.login.usr, "admin", USR_NAME_LEN_MAX);
    	rt_strncpy(logindata.login.pw, newpw, PW_LEN_MAX);
    } else {
    	return USR_PW_CHANGE_N_ERR;
    }

    write_syscfgdata_tbl(SYSCFGDATA_TBL_LOGINDATA, 0, &logindata);

    return DATA_SET_SUCC;
}

/*
 * creat by flyingbug
 *
 * modify by David 2012-04-09 17:09:04
 */
const char *deal_lock_open(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
	httpd_print( ("func:%s, iIndex:%d, iNumParams:%d \n", __FUNCTION__, iIndex, iNumParams));
	elock_open(dummy);

	return LOCK_OPEN_SUCC;
}

/*
 * creat by flyingbug
 * 
 * modify by David 2012-04-09 17:09:04
 */
const char *deal_lock_close(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
	httpd_print( ("func:%s, iIndex:%d, iNumParams:%d \n", __FUNCTION__, iIndex, iNumParams));
	elock_close(dummy);

	return LOCK_CLOSE_SUCC;
}


static int save_ipcfg_data(struct ip_param *ipcfg, int intno)
{
    return write_syscfgdata_tbl(SYSCFGDATA_TBL_IPCFG, intno, ipcfg);
}

static int save_rs232cfg_data(struct rs232_param *rs232cfg, int intno)
{
    return write_syscfgdata_tbl(SYSCFGDATA_TBL_RS232CFG, intno, rs232cfg);
}

static int save_login_data(struct login_param *logindata)
{
    return 0;
}

static int ipcfg_param_to_ipcfg(struct string_int_pairs *ipcfg_param, struct ip_param *ipcfg, int intno)
{
    int i;
    if (NULL==ipcfg_param || NULL==ipcfg)
        return -1;

    if (0==intno)
        i = 0;
    else
        return -1;

    ipcfg->ipmode       = ipcfg_param[i].value;
    ipcfg->ipaddr.addr  = ipcfg_param[i+1].value<<24 | ipcfg_param[i+2].value<<16
                            | ipcfg_param[i+3].value<<8 | ipcfg_param[i+4].value;
    ipcfg->netmask.addr = ipcfg_param[i+5].value<<24 | ipcfg_param[i+6].value<<16
                            | ipcfg_param[i+7].value<<8 | ipcfg_param[i+8].value;
    ipcfg->gw.addr      = ipcfg_param[i+9].value<<24 | ipcfg_param[i+10].value<<16
                            | ipcfg_param[i+11].value<<8 | ipcfg_param[i+12].value;

httpd_print(("func:%s, line:%d, ipmode:%d, ip:%#x, mask:%#x, gw:%#x \n", __FUNCTION__, __LINE__, ipcfg->ipmode,
        ipcfg->ipaddr.addr, ipcfg->netmask.addr, ipcfg->gw.addr));

    return 0;
}

static int rs232cfg_param_to_rs232cfg(struct string_int_pairs *rs232cfg_param, struct rs232_param *rs232cfg, int intno)
{
    int i;
    
    if (NULL==rs232cfg_param || NULL==rs232cfg)
        return -1;

    if (0 == intno)
        i = 0;
    else if (1 == intno)
        i = RS232CFG_INTERFACE1_BASE_INDEX;
    else
        return -1;

    rs232cfg->baudrate  = rs232cfg_param[i].value;
    rs232cfg->databits  = rs232cfg_param[i+1].value;
    rs232cfg->paritybit = rs232cfg_param[i+2].value;
    rs232cfg->stopbits  = rs232cfg_param[i+3].value;

    return 0;
}

static int init_ipcfg_param_name(struct string_int_pairs *ipcfg_param)
{
    int i;
    
    if (NULL==ipcfg_param)
        return -1;

    for (i=0; i<NUM_OF_IPCFG_PARAM_NAEM; ++i) {
        ipcfg_param[i].value = ~0;
    }

    return 0;
}

static int init_rs232cfg_param_name(struct string_int_pairs *rs232cfg_param)
{
    int i;

    if (NULL==rs232cfg_param)
        return -1;

    for (i=0; i<NUM_OF_RS232CFG_PARAM_NAEM; ++i) {
        rs232cfg_param[i].value = ~0;
    }

    return 0;
}



#endif /* #if LWIP_HTTPD_CGI */




#if LWIP_HTTPD_SSI
/* 返回值指向字符串结束标志 -- '\0' */
char *uint2str(unsigned int n, char *str)
{
    char *p, *ret;
    int tmp;
    
    if (0 == n) {
        *str++ = '0';
        *str   = '\0';
        return str;
    }

    p = str;
    while (n != 0) {
        *p++ = n % 10 + '0';
        n   /= 10;
    }
    *p = '\0';

    ret = p--;

    while (str < p) {
        tmp  = *str;
        *str = *p;
        *p   = tmp;
        ++str;
        --p;
    }

    return ret;
}

void ip2str(u32_t ip, char *str)
{
    u32_t tmp;
    int i;
    char *pch = str;

    for (i=0; i<4; ++i) {
        tmp = (ip & (0xff000000>>(i<<3))) >> (24 - (i<<3));
        pch = uint2str(tmp, pch);
        *pch++ = '.';
    }
    *--pch = '\0';

    //return strlen(str);
    return;
}


/* hexadecimal to alpha */
#define HEX2ALPHA(x) ((x)<10 ? (x)+'0' : (x)-10+'A')

/*
 * mac地址占6个字节
 * mac[0]:mac[1]:mac[2]:mac[3]:mac[4]:mac[5]
 * 返回字符串长度
 */
int mac2str(char *mac, char *str)
{
    int i;

    for (i=0; i<6; ++i) {
        *str++ = HEX2ALPHA((*mac & 0xf0)>>4);
        *str++ = HEX2ALPHA(*mac & 0x0f);
        *str++ = ':';
        ++mac;
    }
    *--str = '\0';
    return (2*6 + 5);
}


/* the tag form <!--#name--> */
u16_t SSI_proc(int iIndex, char *pcInsert, int iInsertLen)
{
    char mac[8];
    struct ip_param ipcfg;
    struct rs232_param rs232cfg;

httpd_print(("func:%s, line:%d, index:%d, len:%d \n", __FUNCTION__, __LINE__, iIndex, iInsertLen));

    pcInsert[iInsertLen-1] = '\0';
    if (iIndex>=0 && iIndex<=NETMASK0) {
        if (0 != read_syscfgdata_tbl(SYSCFGDATA_TBL_IPCFG, 0, &ipcfg)) 
            return 0;
    } else if (iIndex>=BAUDRATE0 && iIndex<=STOPBIT0) {
        if (0 != read_syscfgdata_tbl(SYSCFGDATA_TBL_RS232CFG, 0, &rs232cfg))
            return 0;
    } else if (iIndex>=BAUDRATE1 && iIndex<=STOPBIT1) {
        if (0 != read_syscfgdata_tbl(SYSCFGDATA_TBL_RS232CFG, 1, &rs232cfg))
            return 0;
    }
if (iIndex>=BAUDRATE0 && iIndex<=STOPBIT1) {
    httpd_print(("baudrate:%u, databits:%u, paritybit:%u, stopbits:%u \n", 
            rs232cfg.baudrate, rs232cfg.databits, rs232cfg.paritybit, rs232cfg.stopbits));
}
    switch (iIndex) {
    case IPMODE0:
        if (IPADDR_USE_STATIC == ipcfg.ipmode) {
            strcpy(pcInsert, "静态");
        } else if (IPADDR_USE_DHCP == ipcfg.ipmode) {
            strcpy(pcInsert, "动态");
        } else {
            *pcInsert = '\0';
        }
        break;

    case IPADDR0:
        ip2str(ipcfg.ipaddr.addr, pcInsert);
        break;

    case GATEWAY0:
        ip2str(ipcfg.gw.addr, pcInsert);
        break;

    case NETMASK0:
        ip2str(ipcfg.netmask.addr, pcInsert);
        break;

    case MAC0:
        mac[0] = MACOCT0;
        mac[1] = MACOCT1;
        mac[2] = MACOCT2;
        mac[3] = MACOCT3;
        mac[4] = MACOCT4;
        mac[5] = MACOCT5;
        mac2str(mac, pcInsert);
        break;

    case BAUDRATE0:
    case BAUDRATE1:
        uint2str(rs232cfg.baudrate, pcInsert);
        break;

    case DATABITS0:
    case DATABITS1:
        uint2str(rs232cfg.databits, pcInsert);
        break;

    case PARITYBIT0:
    case PARITYBIT1:
        if (UART_PAR_NONE == rs232cfg.paritybit) {
            strcpy(pcInsert, "无");
        } else if (UART_PAR_EVEN == rs232cfg.paritybit) {
            strcpy(pcInsert, "偶");
        } else if (UART_PAR_ODD == rs232cfg.paritybit) {
            strcpy(pcInsert, "奇");
        } else {
            *pcInsert = '\0';
        }
        break;

    case STOPBIT0:
    case STOPBIT1:
        uint2str(rs232cfg.stopbits, pcInsert);
        break;

    default:
        strcpy(pcInsert, "error ssi tag!");
        break;
    }
httpd_print(("func:%s, line:%d, pcInsert:%s \n", __FUNCTION__, __LINE__, pcInsert));
    return strlen(pcInsert);;
}

#endif /* LWIP_HTTPD_SSI */


