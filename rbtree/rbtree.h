/*! 
 *  \brief     Red-Black tree
 *  \author    David Ranieri <davranfor@gmail.com>
 */

#ifndef RBTREE_H
#define RBTREE_H

typedef struct rbtree rbtree;

rbtree *rbtree_create(int (*)(const void *, const void *));
void *rbtree_insert(rbtree *, void *);
void *rbtree_delete(rbtree *, const void *);
void *rbtree_search(const rbtree *, const void *);
void *rbtree_walk(const rbtree *, void *(*)(void *, void *), void *);
void rbtree_destroy(rbtree *, void (*)(void *));

#endif /* RBTREE_H */

