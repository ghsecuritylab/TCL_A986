
#ifndef _LINUX_EXT3_EXTENTS
#define _LINUX_EXT3_EXTENTS

#define AGRESSIVE_TEST_

#define CHECK_BINSEARCH_

//#define EXT_DEBUG
#ifdef EXT_DEBUG
#define ext_debug(tree,fmt,a...) 			\
do {							\
	if (test_opt((tree)->inode->i_sb, EXTDEBUG))	\
		printk(fmt, ##a);			\
} while (0);
#else
#define ext_debug(tree,fmt,a...)
#endif

#define EXT_STATS_


#define EXT3_ALLOC_NEEDED	3	/* block bitmap + group desc. + sb */


struct ext3_extent {
	__u32	ee_block;	/* first logical block extent covers */
	__u16	ee_len;		/* number of blocks covered by extent */
	__u16	ee_start_hi;	/* high 16 bits of physical block */
	__u32	ee_start;	/* low 32 bigs of physical block */
};

struct ext3_extent_idx {
	__u32	ei_block;	/* index covers logical blocks from 'block' */
	__u32	ei_leaf;	/* pointer to the physical block of the next *
				 * level. leaf or next index could bet here */
	__u16	ei_leaf_hi;	/* high 16 bits of physical block */
	__u16	ei_unused;
};

struct ext3_extent_header {
	__u16	eh_magic;	/* probably will support different formats */
	__u16	eh_entries;	/* number of valid entries */
	__u16	eh_max;		/* capacity of store in entries */
	__u16	eh_depth;	/* has tree real underlaying blocks? */
	__u32	eh_generation;	/* generation of the tree */
};

#define EXT3_EXT_MAGIC		0xf30a

struct ext3_ext_path {
	__u32				p_block;
	__u16				p_depth;
	struct ext3_extent		*p_ext;
	struct ext3_extent_idx		*p_idx;
	struct ext3_extent_header	*p_hdr;
	struct buffer_head		*p_bh;
};


#define EXT_CONTINUE	0
#define EXT_BREAK	1
#define EXT_REPEAT	2


#define EXT_MAX_BLOCK	0xffffffff
#define EXT_CACHE_MARK	0xffff


#define EXT_FIRST_EXTENT(__hdr__) \
	((struct ext3_extent *) (((char *) (__hdr__)) +		\
				 sizeof(struct ext3_extent_header)))
#define EXT_FIRST_INDEX(__hdr__) \
	((struct ext3_extent_idx *) (((char *) (__hdr__)) +	\
				     sizeof(struct ext3_extent_header)))
#define EXT_HAS_FREE_INDEX(__path__) \
	((__path__)->p_hdr->eh_entries < (__path__)->p_hdr->eh_max)
#define EXT_LAST_EXTENT(__hdr__) \
	(EXT_FIRST_EXTENT((__hdr__)) + (__hdr__)->eh_entries - 1)
#define EXT_LAST_INDEX(__hdr__) \
	(EXT_FIRST_INDEX((__hdr__)) + (__hdr__)->eh_entries - 1)
#define EXT_MAX_EXTENT(__hdr__) \
	(EXT_FIRST_EXTENT((__hdr__)) + (__hdr__)->eh_max - 1)
#define EXT_MAX_INDEX(__hdr__) \
	(EXT_FIRST_INDEX((__hdr__)) + (__hdr__)->eh_max - 1)

#define EXT_ROOT_HDR(tree) \
	((struct ext3_extent_header *) (tree)->root)
#define EXT_BLOCK_HDR(bh) \
	((struct ext3_extent_header *) (bh)->b_data)
#define EXT_DEPTH(_t_)	\
	(((struct ext3_extent_header *)((_t_)->root))->eh_depth)
#define EXT_GENERATION(_t_)	\
	(((struct ext3_extent_header *)((_t_)->root))->eh_generation)


#define EXT_ASSERT(__x__) if (!(__x__)) BUG();


struct ext3_extent_buf {
	unsigned long start;
	int buflen;
	void *buffer;
	void *cur;
	int err;
};

struct ext3_extent_tree_stats {
	int depth;
	int extents_num;
	int leaf_num;
};

#ifdef __KERNEL__
struct ext3_extents_helpers;
struct ext3_extents_tree {
	struct inode *inode;	/* inode which tree belongs to */
	void *root;		/* ptr to data top of tree resides at */
	void *buffer;		/* will be passed as arg to ^^ routines	*/
	int buffer_len;
	void *private;
	struct ext3_extent *cex;/* last found extent */
	struct ext3_extents_helpers *ops;
};

struct ext3_extents_helpers {
	int (*get_write_access)(handle_t *h, void *buffer);
	int (*mark_buffer_dirty)(handle_t *h, void *buffer);
	int (*mergable)(struct ext3_extent *ex1, struct ext3_extent *ex2);
	int (*remove_extent_credits)(struct ext3_extents_tree *,
					struct ext3_extent *, unsigned long,
					unsigned long);
	int (*remove_extent)(struct ext3_extents_tree *,
				struct ext3_extent *, unsigned long,
				unsigned long);
	int (*new_block)(handle_t *, struct ext3_extents_tree *,
				struct ext3_ext_path *, struct ext3_extent *,
				int *);
};

typedef int (*ext_prepare_callback)(struct ext3_extents_tree *,
					struct ext3_ext_path *,
					struct ext3_extent *, int);
void ext3_init_tree_desc(struct ext3_extents_tree *, struct inode *);
extern int ext3_extent_tree_init(handle_t *, struct ext3_extents_tree *);
extern int ext3_ext_calc_credits_for_insert(struct ext3_extents_tree *, struct ext3_ext_path *);
extern int ext3_ext_insert_extent(handle_t *, struct ext3_extents_tree *, struct ext3_ext_path *, struct ext3_extent *);
extern int ext3_ext_walk_space(struct ext3_extents_tree *, unsigned long, unsigned long, ext_prepare_callback);
extern int ext3_ext_remove_space(struct ext3_extents_tree *, unsigned long, unsigned long);
extern struct ext3_ext_path * ext3_ext_find_extent(struct ext3_extents_tree *, int, struct ext3_ext_path *);

static inline void
ext3_ext_invalidate_cache(struct ext3_extents_tree *tree)
{
	if (tree->cex)
		tree->cex->ee_len = 0;
}
#endif /* __KERNEL__ */


#endif /* _LINUX_EXT3_EXTENTS */

