/*! 
 *  \brief     Doubly linked list
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef LIST_H
#define LIST_H

typedef struct linkedlist linkedlist;

linkedlist *list_create(void);
void *list_push_front(linkedlist *, void *);
void *list_push_back(linkedlist *, void *);
void *list_pop_front(linkedlist *);
void *list_pop_back(linkedlist *);
void *list_prev(linkedlist *, void **);
void *list_next(linkedlist *, void **);
void *list_head(linkedlist *);
void *list_tail(linkedlist *);
size_t list_size(const linkedlist *);
void list_sort(linkedlist *, int (*)(const void *, const void *));
void list_destroy(linkedlist *, void (*)(void *));

#endif /* LIST_H */

