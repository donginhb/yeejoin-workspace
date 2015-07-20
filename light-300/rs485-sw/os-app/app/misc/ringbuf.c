/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         Ring buffer library implementation
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include <ringbuf.h>
#include <rtthread.h>

/*---------------------------------------------------------------------------*/
void
ringbuf_init_sys_init(struct ringbuf *r, char *name, uint8_t *dataptr, uint8_t size)
{
	rt_mutex_init(&r->mu, name, RT_IPC_FLAG_PRIO);
	ringbuf_init(r, dataptr, size);
}

void ringbuf_reset(struct ringbuf *r)
{
	char str[12];
	char *pch;
	int i;

	pch = r->mu.parent.parent.name;
	for (i=0; i<sizeof(str); ++i) {
		str[i] = *pch++;
		if ('\0' == str[i])
			break;
	}

	rt_mutex_detach(&r->mu);
	rt_mutex_init(&r->mu, str, RT_IPC_FLAG_PRIO);
	ringbuf_clr(r);
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void
ringbuf_init(struct ringbuf *r, uint8_t *dataptr, uint8_t size)
{
	r->data = dataptr;
	r->mask = size - 1;
	r->put_ptr = 0;
	r->get_ptr = 0;
}
/*---------------------------------------------------------------------------*/
int
ringbuf_put(struct ringbuf *r, uint8_t c)
{
	rt_err_t err;
	int ret;

	/* Check if buffer is full. If it is full, return 0 to indicate that
	the element was not inserted into the buffer.

	XXX: there is a potential risk for a race condition here, because
	the ->get_ptr field may be written concurrently by the
	ringbuf_get() function. To avoid this, access to ->get_ptr must
	be atomic. We use an uint8_t type, which makes access atomic on
	most platforms, but C does not guarantee this.
	*/
	err = rt_mutex_take(&r->mu, RT_WAITING_FOREVER);
	if (RT_EOK == err) {
		if(((r->put_ptr - r->get_ptr) & r->mask) == r->mask) {
			ret = 0;
		} else {
			r->data[r->put_ptr] = c;
			r->put_ptr = (r->put_ptr + 1) & r->mask;
			ret = 1;
		}
	} else {
		printf_syn("%s() get mutex fail(%d)\n", __func__, err);
		return 0;
	}

	rt_mutex_release(&r->mu);
	return ret;
}
/*---------------------------------------------------------------------------*/
int
ringbuf_get(struct ringbuf *r)
{
	uint8_t c;
	rt_err_t err;
	int ret;

	/* Check if there are bytes in the buffer. If so, we return the
	first one and increase the pointer. If there are no bytes left, we
	return -1.

	XXX: there is a potential risk for a race condition here, because
	the ->put_ptr field may be written concurrently by the
	ringbuf_put() function. To avoid this, access to ->get_ptr must
	be atomic. We use an uint8_t type, which makes access atomic on
	most platforms, but C does not guarantee this.
	*/

	err = rt_mutex_take(&r->mu, RT_WAITING_FOREVER);
	if (RT_EOK == err) {
		if(((r->put_ptr - r->get_ptr) & r->mask) > 0) {
			c = r->data[r->get_ptr];
			r->get_ptr = (r->get_ptr + 1) & r->mask;
			ret = c;
		} else {
			ret = -1;
		}
	} else {
		printf_syn("%s() get mutex fail(%d)\n", __func__, err);
		return -1;
	}

	rt_mutex_release(&r->mu);
	return ret;

}
/*---------------------------------------------------------------------------*/
int
ringbuf_size(struct ringbuf *r)
{
	return r->mask + 1;
}
/*---------------------------------------------------------------------------*/
int
ringbuf_elements(struct ringbuf *r)
{
	rt_err_t err;
	int ret;

	err = rt_mutex_take(&r->mu, RT_WAITING_FOREVER);
	if (RT_EOK == err) {
		ret = (r->put_ptr - r->get_ptr) & r->mask;
	} else {
		printf_syn("%s() get mutex fail(%d)\n", __func__, err);
		return 0;
	}

	rt_mutex_release(&r->mu);
	return ret;
}
/*---------------------------------------------------------------------------*/

void ringbuf_clr(struct ringbuf *r)
{
	rt_err_t err;

	err = rt_mutex_take(&r->mu, RT_WAITING_FOREVER);
	if (RT_EOK == err) {
		r->put_ptr = 0;
		r->get_ptr = 0;
	} else {
		printf_syn("%s() get mutex fail(%d)\n", __func__, err);
		return;
	}

	rt_mutex_release(&r->mu);
	return;
}

/*---------------------------------------------------------------------------*/

int ringbuf_get_slice(struct ringbuf *r, uint8_t offset, char *buf, int len)
{
	uint8_t get_ptr;
	int i, ret;
	rt_err_t err;

	if (NULL==buf || len>(ringbuf_elements(r)-offset)) {
		return 0;
	}

	err = rt_mutex_take(&r->mu, RT_WAITING_FOREVER);
	if (RT_EOK == err) {
		get_ptr = (r->get_ptr + offset) & r->mask;;

		for (i=0; i<len; ++i) {
			*buf++ = r->data[get_ptr];
			get_ptr = (get_ptr + 1) & r->mask;
		}

		ret = len;
	} else {
		printf_syn("%s() get mutex fail(%d)\n", __func__, err);
		return 0;
	}

	rt_mutex_release(&r->mu);
	return ret;

}
