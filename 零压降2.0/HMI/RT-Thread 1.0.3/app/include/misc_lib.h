/*
 * misc_lib.c
 *
 * 2013-03-03,  creat by David, zhaoshaowei@yeejoin.com
 */
#ifndef _MISC_LIB_H_
#define _MISC_LIB_H_

extern void get_ipc_id_name(char *id_name, char *prefix, int *ipc_cnt);

extern int i2str(char *str, int n);
extern int convert_date_format(char *date, char *str);
#if !(!defined (RT_USING_NEWLIB) && defined (RT_USING_MINILIBC))
extern char *strcat(char *dest, const char *src);
#endif

#ifndef sub_abs
#define sub_abs(x, y) ((x)>(y) ? (x)-(y) : (y)-(x))
#endif

#endif
