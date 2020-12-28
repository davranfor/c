/*! 
 *  \brief     knode (Kernel list concept applied to a XOR linked list)
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef KNODE_H
#define KNODE_H

typedef struct knode knode;

knode *knode_create(size_t);
void *knode_push_head(knode *);
void *knode_push_tail(knode *);
void *knode_pop_head(knode *);
void *knode_pop_tail(knode *);
void *knode_head(const knode *);
void *knode_tail(const knode *);
void *knode_fetch(const knode *, const void **, const void *);
size_t knode_size(const knode *);
void knode_clear(knode *, void (*)(void *));
void knode_destroy(knode *, void (*)(void *));

#endif /* KNODE_H */

