/*
 *******************************************************************************
 * base_ds.c
 * base data structure
 *
 *******************************************************************************
 */

#include <rtthread.h>

#include "base_ds.h"
#include "misc_lib.h"



#define  bds_memalloc    rt_malloc
#define  bds_memfree     rt_free


/*
 * ring buffer
 * 
 * read_index == write_index: buffer is empty
 * read_index - write_incex == -1: buffer is full
 *******************************************************************************
 */

#define MAX_INDEX_OF_RB(rb)  (rb->size - 1)
#define BUFFER_SIZE_MIN      (64)

int rb_rw_sem_cnt;

#if 0
static int _rb_get_used_bytes_num(struct rb* rb);
#else
#define _rb_get_used_bytes_num rb_get_used_bytes_num
#endif
/*
 * 缓冲区最小为64字节
 */
int rb_init(struct rb *rb, int size)
{
	char name[12];
    int alloc_size;
    
	if (RT_NULL == rb)
	    return RT_ERROR;

    alloc_size = size > BUFFER_SIZE_MIN ? size : BUFFER_SIZE_MIN;
    rb->buf = bds_memalloc(alloc_size);
    if (RT_NULL == rb->buf)
        return RT_ENOMEM;

    get_ipc_id_name(name, "rbrw", &rb_rw_sem_cnt);
    if (NULL == (rb->rw_sem = rt_sem_create(name, 1, RT_IPC_FLAG_FIFO))) {
        bds_memfree(rb->buf);
        return RT_ENOMEM;
    }

	rb->r_ind = 0;
	rb->w_ind = 0;
	rb->size  = alloc_size;

	return RT_EOK;
}

/*
 * length -- buffer size
 * return value -- 读到的字节个数
 */
int rb_read(struct rb *rb, unsigned char *ptr, int len)
{
	int cnt, ret, index;
	unsigned char *buf;

	if (RT_NULL == rb || NULL==ptr)
	    return 0;

    if (rb->r_ind == rb->w_ind)
        return 0;

    //rt_sem_take(rb->rw_sem, RT_WAITING_FOREVER);
    buf   = rb->buf;
    index = rb->r_ind;

    if (rb->r_ind > rb->w_ind) {
        /* 需要处理read index回绕 */
        cnt = rb->size - rb->r_ind;
        cnt = MIN(cnt, len);
        ret = cnt;
        while (cnt-- > 0)
            *ptr++ = buf[index++];

        index = 0;
        if (len > ret) {
            cnt = MIN(rb->w_ind, len - ret);
            ret += cnt;
            while (cnt-- > 0)
                *ptr++ = buf[index++];
        }
    } else {
        cnt = rb->w_ind - rb->r_ind;
        cnt = MIN(cnt, len);
        ret = cnt;
        while (cnt-- > 0)
            *ptr++ = buf[index++];
    }

    rb->r_ind = index;
    //rt_sem_release(rb->rw_sem);

    return ret;
}

/*
 * len -- 希望写入的字节个数
 * return value -- 实际写入的字节个数
 *
 * NOTE: 缓冲区满时, 不覆盖旧数据 !!!
 */
int rb_write(struct rb* rb, const unsigned char *ptr, int len)
{
	int  cnt, ret, index;
	unsigned char *buf;
       
	if (RT_NULL == rb || NULL==ptr)
	    return 0;
    //rt_sem_take(rb->rw_sem, RT_WAITING_FOREVER);

    buf   = rb->buf;
    index = rb->w_ind;

    if (rb->r_ind > rb->w_ind) {
        cnt = rb->r_ind - rb->w_ind - 1;
        cnt = MIN(cnt, len);
        ret = cnt;
        while (cnt-- >0)
            buf[index++] = *ptr++;
    } else {
        /* w_ind >= r_ind, 需要处理write index回绕 */
        cnt = MAX_INDEX_OF_RB(rb) - rb->w_ind;
        cnt = MIN(cnt, len);
        ret = cnt;
        while (cnt-- > 0)
            buf[index++] = *ptr++;

        if (ret<len && 0!=rb->r_ind) {
            buf[index] = *ptr++;
            ret++;
            index = 0;

            cnt = rb->r_ind - 1;
            cnt = MIN(cnt, len-ret);
            ret += cnt;
            while (cnt-- > 0)
                buf[index++] = *ptr++;
        }
    }

    rb->w_ind = index;
    //rt_sem_release(rb->rw_sem);

    return ret;
}

int rb_get_used_bytes_num(struct rb* rb)
{
    int ret;
    
	if (RT_NULL == rb)
	    return 0;
    //rt_sem_take(rb->rw_sem, RT_WAITING_FOREVER);

    if (rb->r_ind == rb->w_ind)
        ret = 0;
    else if (rb->r_ind < rb->w_ind)
        ret = rb->w_ind - rb->r_ind;
    else
        ret = rb->w_ind + rb->size - rb->r_ind;

    //rt_sem_release(rb->rw_sem);
    return ret;
}
#if 0
static int _rb_get_used_bytes_num(struct rb* rb)
{
    int ret;
    

    if (rb->r_ind == rb->w_ind)
        ret = 0;
    else if (rb->r_ind < rb->w_ind)
        ret = rb->w_ind - rb->r_ind;
    else
        ret = rb->w_ind + rb->size - rb->r_ind;

    return ret;
}
#endif
int rb_get_unused_bytes_num(struct rb* rb)
{
	if (RT_NULL == rb)
	    return 0;

    return (rb->size - rb_get_used_bytes_num(rb) - 1);
}

void rb_destroy(struct rb* rb)
{
	if (RT_NULL == rb)
	    return;

    bds_memfree(rb->buf);
    rt_sem_delete(rb->rw_sem);

    rb->buf    = RT_NULL;
    rb->rw_sem = RT_NULL;

    return;
}

#if 0
/*
 * return value:
 *     0 -- succ
 *     1 -- fail
 */
int rb_last_write_byte_get(struct rb* rb, unsigned char *byte)
{
    int last_ind, ret;

    if (rb_get_used_bytes_num(rb)) {
        last_ind = 0==rb->w_ind ? (rb->size - 1) : (rb->w_ind - 1);
        *byte = rb->buf[last_ind];
        ret = 0;
    } else
        ret = 1;

    return ret;    
}


/*
 * return value:
 *     0 -- succ
 *     1 -- fail
 */
int rb_next2last_w_byte_get(struct rb* rb, unsigned char *byte)
{
    int next2last_ind, ret;

    if (rb_get_used_bytes_num(rb) >= 2) {
        next2last_ind = 1>=(rb)->w_ind ? ((rb)->w_ind + (rb)->size - 2) : ((rb)->w_ind-2);
        *byte = rb->buf[next2last_ind];
        ret = 0;
    } else
        ret = 1;

    return ret;    
}


/*
 * return value:
 *     0 -- succ
 *     1 -- fail
 */
int rb_last_write_byte_drop(struct rb* rb)
{
    int ret;

    rt_sem_take(rb->w_sem, RT_WAITING_FOREVER);

    if (rb_get_used_bytes_num(rb)) {
        
        if (0==rb->w_ind)
            rb->w_ind = rb->size - 1;
        else
            rb->w_ind -= 1;


        ret = 0;
    } else {
        ret = 1;
    }
    
    rt_sem_release(rb->w_sem);

    return ret;
}
#endif
int rb_first_read_byte_pry(struct rb* rb, unsigned char *byte)
{
    int ret;

	if (RT_NULL == rb || NULL==byte)
	    return 1;

    //rt_sem_take(rb->rw_sem, RT_WAITING_FOREVER);

    if (_rb_get_used_bytes_num(rb)) {
        *byte = rb->buf[rb->r_ind];
        ret = 0;
    } else
        ret = 1;

    //rt_sem_release(rb->rw_sem);
    return ret;    
}

int rb_first_read_byte_drop(struct rb* rb)
{
    int ret;

	if (RT_NULL == rb)
	    return 1;
    //rt_sem_take(rb->rw_sem, RT_WAITING_FOREVER);

    if (_rb_get_used_bytes_num(rb)) {
        if (rb->size - 1 == rb->r_ind)
            rb->r_ind = 0;
        else
            rb->r_ind += 1;

        ret = 0;
    } else {
        ret = 1;
    }
    
    //rt_sem_release(rb->rw_sem);

    return ret;
}


void rb_cleanup(struct rb* rb)
{

	if (RT_NULL == rb)
	    return;

    rb->r_ind = 0;
    rb->w_ind = 0;

    return;
}


