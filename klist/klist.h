/*! 
 *  \brief     klist (Kernel list concept)
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef KLIST_H
#define KLIST_H

typedef struct klist klist;

klist *klist_create(size_t);
void *klist_push_head(klist *);
void *klist_push_tail(klist *);
void *klist_pop_head(klist *);
void *klist_pop_tail(klist *);
void *klist_head(const klist *);
void *klist_prev(const klist *, const void *);
void *klist_next(const klist *, const void *);
void *klist_tail(const klist *);
size_t klist_size(const klist *);
void klist_destroy(klist *, void (*)(void *));

#endif /* KLIST_H */

