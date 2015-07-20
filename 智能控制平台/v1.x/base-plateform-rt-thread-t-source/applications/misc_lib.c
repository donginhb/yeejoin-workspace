/*
 *
 */
#include <rtdef.h>

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
	}while(0 != temp);

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
