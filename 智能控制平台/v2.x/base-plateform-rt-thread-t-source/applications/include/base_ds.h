/*
 * base_ds.h
 * base data structure
 *
 */
#ifndef _BASE_DS_H_
#define _BASE_DS_H_

#include <rtdef.h>

/*
 * ring buffer
 * 
 * read_index == write_index: buffer is empty
 * read_index - write_incex == -1: buffer is full
 *
 * rb_get_used_bytes_num()需要r_ind, w_ind保持不变, r_sem/w_sem --> rw_sem
 */
struct rb
{
	int  r_ind; /* read index, first vaild byte */
	int  w_ind; /* write index, fisrt empty byte */
	rt_sem_t rw_sem;
	unsigned char *buf;  /* buffer start address */
	int  size;  /* buffer size of bytes */
};

extern int rb_init(struct rb *rb, int size);
extern int rb_read(struct rb *rb, unsigned char *ptr, int len);
extern int rb_write(struct rb* rb, const unsigned char *ptr, int len);
extern int rb_get_used_bytes_num(struct rb* rb);
extern int rb_get_unused_bytes_num(struct rb* rb);
extern void rb_destroy(struct rb* rb);

extern int rb_last_write_byte_get(struct rb* rb, unsigned char *byte);
extern int rb_next2last_w_byte_get(struct rb* rb, unsigned char *byte);
extern int rb_last_write_byte_drop(struct rb* rb);
extern int rb_first_read_byte_pry(struct rb* rb, unsigned char *byte);
extern int rb_first_read_byte_drop(struct rb* rb);

#endif

