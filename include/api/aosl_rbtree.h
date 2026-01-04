/***************************************************************************
 * Module:	AOSL Red-Black tree header file
 *
 * Copyright © 2025 Agora
 * This file is part of AOSL, an open source project.
 * Licensed under the Apache License, Version 2.0, with certain conditions.
 * Refer to the "LICENSE" file in the root directory for more information.
 ***************************************************************************/

#ifndef	__AOSL_RBTREE_H__
#define	__AOSL_RBTREE_H__

#include <api/aosl_types.h>
#include <api/aosl_defs.h>


#ifdef __cplusplus
extern "C" {
#endif


struct aosl_rb_node {
	uintptr_t rb_parent_color;
	struct aosl_rb_node *rb_right;
	struct aosl_rb_node *rb_left;
};

typedef int (*aosl_rb_node_cmp_t) (struct aosl_rb_node *rb_node, struct aosl_rb_node *node, va_list args);

struct aosl_rb_root {
	struct aosl_rb_node *rb_node;
	aosl_rb_node_cmp_t rb_cmp;
	uintptr_t count;
};


#define	aosl_rb_entry(ptr, type, member) container_of(ptr, type, member)

extern __aosl_api__ void aosl_rb_root_init (struct aosl_rb_root *root, aosl_rb_node_cmp_t cmp);


extern __aosl_api__ struct aosl_rb_node **aosl_vfind_rb_links (struct aosl_rb_root *root, struct aosl_rb_node **rb_parent,
	struct aosl_rb_node **pprev, struct aosl_rb_node **pnext, struct aosl_rb_node *node, va_list args);

extern __aosl_api__ struct aosl_rb_node **aosl_find_rb_links (struct aosl_rb_root *root, struct aosl_rb_node **rb_parent,
		struct aosl_rb_node **pprev, struct aosl_rb_node **pnext, struct aosl_rb_node *node, ...);

extern __aosl_api__ struct aosl_rb_node *aosl_vfind_rb_node (struct aosl_rb_root *root, struct aosl_rb_node *node, va_list args);
extern __aosl_api__ struct aosl_rb_node *aosl_find_rb_node (struct aosl_rb_root *root, struct aosl_rb_node *node, ...);

extern __aosl_api__ void aosl_rb_insert_node (struct aosl_rb_root *root, struct aosl_rb_node *node, ...);
extern __aosl_api__ struct aosl_rb_node *aosl_rb_remove (struct aosl_rb_root *root, struct aosl_rb_node *node, ...);

extern __aosl_api__ void aosl_rb_erase (struct aosl_rb_root *root, struct aosl_rb_node *node);

typedef int (*aosl_rb_walk_func_t) (struct aosl_rb_node *node, void *arg);

extern __aosl_api__ void aosl_rb_traverse_dlr (struct aosl_rb_root *root, aosl_rb_walk_func_t func, void *arg);
extern __aosl_api__ void aosl_rb_traverse_ldr (struct aosl_rb_root *root, aosl_rb_walk_func_t func, void *arg);
extern __aosl_api__ void aosl_rb_traverse_lrd (struct aosl_rb_root *root, aosl_rb_walk_func_t func, void *arg);

/* Find logical next and previous nodes in a tree */
extern __aosl_api__ struct aosl_rb_node *aosl_rb_next (struct aosl_rb_node *);
extern __aosl_api__ struct aosl_rb_node *aosl_rb_prev (struct aosl_rb_node *);
extern __aosl_api__ struct aosl_rb_node *aosl_rb_first (struct aosl_rb_root *);
extern __aosl_api__ struct aosl_rb_node *aosl_rb_last (struct aosl_rb_root *);

// clear rb tree
#define aosl_rb_clear(root, type, member, type_free)                           \
do {                                                                           \
	struct aosl_rb_node *node;                                                   \
	while ((node = (root)->rb_node) != NULL) {                                   \
		type *pos = aosl_rb_entry(node, type, member);                             \
		aosl_rb_erase(root, node);                                                 \
		type_free(pos);                                                            \
	}                                                                            \
} while(0)


#ifdef __cplusplus
}
#endif

#endif /* __AOSL_RBTREE_H__ */
