/*! 
 *  \brief     knode (Kernel list concept applied to a XOR linked list)
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef KNODE_H
#define KNODE_H

#include <stdint.h>

#define KNODE_HEAD 0x01
#define KNODE_TAIL 0x02

typedef struct knode knode;
typedef uintptr_t kiter;

knode *knode_create(size_t);
void *knode_push_head(knode *);
void *knode_push_tail(knode *);
void *knode_pop_head(knode *);
void *knode_pop_tail(knode *);
void *knode_head(const knode *);
void *knode_tail(const knode *);
void *knode_fetch(const knode *, kiter *, const void *);
void *knode_index(const knode *, size_t);
size_t knode_size(const knode *);
void knode_clear(knode *, void (*)(void *));
void knode_destroy(knode *, void (*)(void *));

#endif /* KNODE_H */

