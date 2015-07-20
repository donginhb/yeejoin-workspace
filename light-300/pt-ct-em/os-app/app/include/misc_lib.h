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


#define set_bit(bit_vector, mask)     ((bit_vector) |=  (mask))
#define clr_bit(bit_vector, mask)     ((bit_vector) &= ~(mask))
#define reverse_bit(bit_vector, mask) ((bit_vector) ^=  (mask))
#define is_bit_set(bit_vector, mask)  (!!((bit_vector) & (mask)))
#define is_bit_clr(bit_vector, mask)  (!((bit_vector) &  (mask)))

#ifndef sub_abs
#define sub_abs(x, y) ((x)>(y) ? (x)-(y) : (y)-(x))
#endif

#ifndef MIN
#define MIN(a, b) ((a)>(b) ? (b) : (a))
#endif

#ifndef MAX
#define MAX(a, b) ((a)>(b) ? (a) : (b))
#endif


extern int conv_3bsinged_to_4bsinged(void *singed4b, int len4, void *singed3b, int len3);
extern unsigned long conv_4byte_bcd_to_long(unsigned long bcd);
extern int convert_str2bcd(unsigned char *str, int slen, unsigned char *bcd, int blen);
extern int del_multi_zero_in_str(char *str, int len);

extern int convert_hex2str(unsigned char *hex, int hlen, unsigned char *str, int slen);
extern int convert_str2hex(unsigned char *str, int slen, unsigned char *hex, int hlen);

extern void print_buf_in_hex(char *prompt, char *buf, int len);

#endif
