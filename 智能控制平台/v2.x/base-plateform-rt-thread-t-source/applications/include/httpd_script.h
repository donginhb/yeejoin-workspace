#ifndef _HTTPD_SCRIPT_H_
#define _HTTPD_SCRIPT_H_

#include "syscfgdata.h"

extern const char *deal_ipcfg_data(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
extern const char *deal_rs232cfg_data(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
extern const char *deal_login_data(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
extern const char *deal_lock_open(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
extern const char *deal_lock_close(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
extern const char *deal_ch_psword_data(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);

#if LWIP_HTTPD_SSI
extern u16_t SSI_proc(int iIndex, char *pcInsert, int iInsertLen);
#endif

#endif
