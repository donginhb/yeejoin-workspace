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

