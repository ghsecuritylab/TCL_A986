

#ifndef __IDR_H__
#define __IDR_H__

#include <linux/types.h>
#include <linux/bitops.h>
#include <linux/init.h>
#include <linux/rcupdate.h>

#if BITS_PER_LONG == 32
# define IDR_BITS 5
# define IDR_FULL 0xfffffffful
# define TOP_LEVEL_FULL (IDR_FULL >> 30)
#elif BITS_PER_LONG == 64
# define IDR_BITS 6
# define IDR_FULL 0xfffffffffffffffful
# define TOP_LEVEL_FULL (IDR_FULL >> 62)
#else
# error "BITS_PER_LONG is not 32 or 64"
#endif

#define IDR_SIZE (1 << IDR_BITS)
#define IDR_MASK ((1 << IDR_BITS)-1)

#define MAX_ID_SHIFT (sizeof(int)*8 - 1)
#define MAX_ID_BIT (1U << MAX_ID_SHIFT)
#define MAX_ID_MASK (MAX_ID_BIT - 1)

/* Leave the possibility of an incomplete final layer */
#define MAX_LEVEL (MAX_ID_SHIFT + IDR_BITS - 1) / IDR_BITS

/* Number of id_layer structs to leave in free list */
#define IDR_FREE_MAX MAX_LEVEL + MAX_LEVEL

struct idr_layer {
	unsigned long		 bitmap; /* A zero bit means "space here" */
	struct idr_layer	*ary[1<<IDR_BITS];
	int			 count;	 /* When zero, we can release it */
	int			 layer;	 /* distance from leaf */
	struct rcu_head		 rcu_head;
};

struct idr {
	struct idr_layer *top;
	struct idr_layer *id_free;
	int		  layers; /* only valid without concurrent changes */
	int		  id_free_cnt;
	spinlock_t	  lock;
};

#define IDR_INIT(name)						\
{								\
	.top		= NULL,					\
	.id_free	= NULL,					\
	.layers 	= 0,					\
	.id_free_cnt	= 0,					\
	.lock		= __SPIN_LOCK_UNLOCKED(name.lock),	\
}
#define DEFINE_IDR(name)	struct idr name = IDR_INIT(name)

/* Actions to be taken after a call to _idr_sub_alloc */
#define IDR_NEED_TO_GROW -2
#define IDR_NOMORE_SPACE -3

#define _idr_rc_to_errno(rc) ((rc) == -1 ? -EAGAIN : -ENOSPC)



void *idr_find(struct idr *idp, int id);
int idr_pre_get(struct idr *idp, gfp_t gfp_mask);
int idr_get_new(struct idr *idp, void *ptr, int *id);
int idr_get_new_above(struct idr *idp, void *ptr, int starting_id, int *id);
int idr_for_each(struct idr *idp,
		 int (*fn)(int id, void *p, void *data), void *data);
void *idr_get_next(struct idr *idp, int *nextid);
void *idr_replace(struct idr *idp, void *ptr, int id);
void idr_remove(struct idr *idp, int id);
void idr_remove_all(struct idr *idp);
void idr_destroy(struct idr *idp);
void idr_init(struct idr *idp);


#define IDA_CHUNK_SIZE		128	/* 128 bytes per chunk */
#define IDA_BITMAP_LONGS	(128 / sizeof(long) - 1)
#define IDA_BITMAP_BITS		(IDA_BITMAP_LONGS * sizeof(long) * 8)

struct ida_bitmap {
	long			nr_busy;
	unsigned long		bitmap[IDA_BITMAP_LONGS];
};

struct ida {
	struct idr		idr;
	struct ida_bitmap	*free_bitmap;
};

#define IDA_INIT(name)		{ .idr = IDR_INIT(name), .free_bitmap = NULL, }
#define DEFINE_IDA(name)	struct ida name = IDA_INIT(name)

int ida_pre_get(struct ida *ida, gfp_t gfp_mask);
int ida_get_new_above(struct ida *ida, int starting_id, int *p_id);
int ida_get_new(struct ida *ida, int *p_id);
void ida_remove(struct ida *ida, int id);
void ida_destroy(struct ida *ida);
void ida_init(struct ida *ida);

void __init idr_init_cache(void);

#endif /* __IDR_H__ */
