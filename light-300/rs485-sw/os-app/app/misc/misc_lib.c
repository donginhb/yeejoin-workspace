/*
 * misc_lib.c
 *
 * 2013-03-03,  creat by David, zhaoshaowei@yeejoin.com
 */
#include <rtdef.h>
#include <rtthread.h>

static void update_ipc_id(int *ipc_cnt);


void get_ipc_id_name(char *id_name, char *prefix, int *ipc_cnt)
{
	char *p, *q, tch;
	int temp;

	update_ipc_id(ipc_cnt);
	rt_strncpy(id_name, prefix, 8); /* rtos中mb的name字段长度为8 */

	p = q = id_name + 4;

	temp = *ipc_cnt;
	do {
		*p++ = '0' + temp%10;
		temp /= 10;
	} while(0 != temp);

	if (p - q < 8)
		*p-- = '\0';
	else
		--p;

	while (p > q) {
		tch = *q;
		*q++ = *p;
		*p-- = tch;
	}

	return;
}

int i2str(char *str, int n)
{
	char *pch;
	char ch;

	if (NULL==str)
		return FAIL;

	if (0 == n) {
		*str++ = '0';
		*str   = '\0';
		return SUCC;
	}

	if (n < 0) {
		*str++ = '-';
		n = -n;
	}

	pch = str;
	while (0 != n) {
		*pch++ = '0' + n%10;
		n /= 10;
	}

	*pch-- = '\0';
	while (str < pch) {
		ch = *pch;
		*pch-- = *str;
		*str++ = ch;
	}

	return SUCC;
}

int ui2str(char *str, unsigned int n)
{
	char *pch;
	char ch;

	if (NULL==str)
		return FAIL;

	if (0 == n) {
		*str++ = '0';
		*str   = '\0';
		return SUCC;
	}

	pch = str;
	while (0 != n) {
		*pch++ = '0' + n%10;
		n /= 10;
	}

	*pch-- = '\0';
	while (str < pch) {
		ch = *pch;
		*pch-- = *str;
		*str++ = ch;
	}

	return SUCC;
}

int str_reverse(char *str)
{
	char *pch;
	char ch;

	if (NULL == str) {
		return FAIL;
	}

	pch = str;
	while ('\0' != *pch++)
		;

	pch -= 2;
	while (str < pch) {
		ch = *str;
		*str++ = *pch;
		*pch-- = ch;
	}

	return SUCC;
}

int byte_reverse(char *str, int n)
{
	char *pch;
	char ch;

	if (NULL == str || n<=0) {
		return FAIL;
	}

	pch = str + n - 1;
	while (str < pch) {
		ch = *str;
		*str++ = *pch;
		*pch-- = ch;
	}

	return SUCC;
}


/* 使用一个id后, 调用该函数 */
static void update_ipc_id(int *ipc_cnt)
{
	if (*ipc_cnt < 9999)
		++*ipc_cnt;
	else
		*ipc_cnt = 0;
}

/*
 * Mar  1 2013 --> 20130301
 * 17:25:35
 */
const char *const months_str[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};


int convert_date_format(char *date, char *str)
{
	int i;
	char datestr_1[12];

	if (NULL==date || NULL==str)
		return RT_ERROR;

	/* Aug 22 2012 */
	rt_strncpy(datestr_1, date, sizeof(datestr_1));
	datestr_1[3] = '\0';
	if (' '==datestr_1[4])
		datestr_1[4] = '0';
	datestr_1[6] = '\0';

	for (i=0; i<12; ++i)
		if (0==rt_strncmp(months_str[i], datestr_1, 3))
			break;

	rt_sprintf(str, "%4s%02d%2s", &datestr_1[7], i+1, &datestr_1[4]);

	return RT_EOK;
}

#if !(!defined (RT_USING_NEWLIB) && defined (RT_USING_MINILIBC))
char *strcat(char *dest, const char *src)
{
	char *pch;

	if (NULL==dest || NULL==src)
		return NULL;

	pch = dest;
	while ('\0' != *pch)
		++pch;

	while ('\0' != *src)
		*pch++ = *src++;

	*pch = '\0';

	return dest;
}
#endif

/*
 * 将3字节有符号数转换为4字节有符号数, 都以网络序存储
 * len4/len3 表示缓冲区字节个数
 **/
int conv_3bsinged_to_4bsinged(void *singed4b, int len4, void *singed3b, int len3)
{
	char *pchs, *pchd;
	int i, cnt;

	cnt = len3 / 3;
	if (len4 < (len3+cnt)) {
		printf_syn("func:%s, buf len error(%d, %d)\n", __FUNCTION__, len3, len4);
		return FAIL;
	}

	pchs = singed3b;
	pchd = singed4b;
	for (i=0; i<cnt; ++i) {
		if (*pchs & (1<<23))
			*pchd++ = 0xff;
		else
			*pchd++ = 0;

		*pchd++ = *pchs++;
		*pchd++ = *pchs++;
		*pchd++ = *pchs++;
	}

	return SUCC;
}

/*
 * 将4字节bcd码转换为整数
 *
 * */
unsigned long conv_4byte_bcd_to_long(unsigned long bcd)
{
	unsigned int  i, a;
	unsigned long ret;

	ret = 0;
	for (i=0; i<8; ++i) {
		a   =  (bcd >> (7-i)*4) & 0xf;
		ret *= 10;
		ret += a;
	}

	return ret;
}

int convert_str2bcd(unsigned char *str, int slen, unsigned char *bcd, int blen)
{
	unsigned char t;
	int i;
	unsigned char *pch;

	if (NULL==bcd || NULL==str) {
		printf_syn("func:%s() param is NULL pointer\n", __FUNCTION__);
		return FAIL;
	}

	pch = str;
	i   = slen;
	while ('\0'!=*pch && 0!=i)
		++pch, --i;

	slen = pch - str;
	t = slen >> 1;

	if (blen < (slen&0x1 ? t+1 : t)) {
		printf_syn("func:%s() bcd's space is too small\n", __FUNCTION__);
		return FAIL;
	}

	pch = bcd;
	for (i=0; i<slen && '\0'!=str[i]; i+=2) {
		t = str[i];
		t -= '0';
		*pch = t << 4;
		if (0 != str[i+1]) {
			t = str[i+1];
			t -= '0';
			*pch++ |= t;
		}
	}


	return SUCC;
}


int convert_bcd2str(char *bcd, int blen, char *str, int slen)
{
	unsigned char ch;
	int i;

	if (NULL==bcd || NULL==str) {
		printf_syn("func:%s() param is NULL pointer\n", __FUNCTION__);
		return FAIL;
	}

	if ((blen<<1) >= slen) {
		printf_syn("func:%s() str's space is too small\n", __FUNCTION__);
		return FAIL;
	}


	for (i=0; i<blen; ++i) {
		ch = *bcd++;
		*str++ = (ch>>4) + '0';
		*str++ = (ch & 0xf) + '0';
	}

	*str = '\0';

	return SUCC;
}


int del_multi_zero_in_str(char *str, int len)
{
	char *ps, *pd;
	int cnt;

	if (NULL == str) {
		printf_syn("func:%s() param is NULL pointer\n", __FUNCTION__);
		return FAIL;
	}

	ps = str;
	cnt = len;
	while ('0' == *ps && 0!=cnt--)
		++ps;

	if (ps != str) {
		/* 有前导0 */
		pd = str;
		cnt = len -(ps-str);
		while ('\0' != *ps && 0!=cnt--)
			*pd++ = *ps++;

		*pd = '\0';
	}

	return SUCC;
}

int reset_timer(rt_timer_t timer)
{
	if (NULL == timer) {
		printf_syn("%s() param is NULL\n", __func__);
		return FAIL;
	}

	if(is_bit_set(timer->parent.flag, RT_TIMER_FLAG_ACTIVATED)) {
		rt_timer_stop(timer);
		rt_timer_start(timer);
	}

	return SUCC;
}


void print_buf_in_hex(char *prompt, char *buf, int len)
{
	int i;

	if (prompt==NULL || NULL==buf)
		return;

	printf_syn("%s:\n", prompt);

	len = len<255 ? len : 255;

	for (i=0; i<len; ++i)
		printf_syn("0x%02x ", *buf++);

	printf_syn("\n");

	return;
}

#if 0!=USE_HEX_DEV_SN
#define signal_hex2char(h) ((h)<=9 ? '0'+(h) : 'A'+((h)-10))
int convert_hex2str(unsigned char *hex, int hlen, unsigned char *str, int slen)
{
	int i;
	unsigned t, t1;

	if (NULL==hex || NULL==str) {
		printf_syn("func:%s() param is NULL pointer\n", __FUNCTION__);
		return FAIL;
	}

	if (slen < (hlen<<1)) {
		printf_syn("func:%s() str's space is too small\n", __FUNCTION__);
		return FAIL;
	}

	for (i=0; i<hlen; ++i) {
		t = *hex++;

		t1 = (t>>4) & 0xf;
		*str++ = signal_hex2char(t1);

		t1 = t & 0xf;
		*str++ = signal_hex2char(t1);
	}

	return SUCC;

}

#define signal_char2hex(c) (((c)<='9' && (c)>='0') ? (c)-'0' : (c)-'A'+10)
#define convert_upper_case(ch)	((ch)>='a' && (ch)<='z' ? (ch)-('a'-'A') : (ch))
int convert_str2hex(unsigned char *str, int slen, unsigned char *hex, int hlen)
{
	unsigned t;
	int i;

	if (NULL==hex || NULL==str) {
		printf_syn("func:%s() param is NULL pointer\n", __FUNCTION__);
		return FAIL;
	}

	t = slen >> 1;
	if (hlen < (slen&0x1 ? t+1 : t)) {
		printf_syn("func:%s() hex's space is too small\n", __FUNCTION__);
		return FAIL;
	}

	for (i=0; i<slen; ++i) {
		t = *str++;
		t = convert_upper_case(t);
		t = signal_char2hex(t);
		*hex = t << 4;

		t = *str++;
		t = convert_upper_case(t);
		*hex++ |= signal_char2hex(t);
	}

	return SUCC;
}
#endif



