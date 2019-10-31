/*! 
 *  \brief     Doubly linked list
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef LINKLIST_H
#define LINKLIST_H

typedef struct linklist linklist;

linklist *linklist_create(void);
void *linklist_push_head(linklist *, void *);
void *linklist_push_tail(linklist *, void *);
void *linklist_pop_head(linklist *);
void *linklist_pop_tail(linklist *);
void *linklist_insert(linklist *, size_t, void *);
void *linklist_delete(linklist *, size_t);
void *linklist_index(linklist *, size_t);
void *linklist_head(linklist *);
void *linklist_prev(linklist *, void **);
void *linklist_next(linklist *, void **);
void *linklist_tail(linklist *);
size_t linklist_size(const linklist *);
void linklist_sort(linklist *, int (*)(const void *, const void *));
void *linklist_search(const linklist *, const void *, int (*)(const void *, const void *));
void linklist_destroy(linklist *, void (*)(void *));

#endif /* LINKLIST_H */

