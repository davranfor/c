/*! 
 *  \brief     Double-ended queue
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef DEQUE_H
#define DEQUE_H

typedef struct deque deque;

deque *deque_create(void);
void *deque_push_head(deque *, void *);
void *deque_push_tail(deque *, void *);
void *deque_pop_head(deque *);
void *deque_pop_tail(deque *);
void *deque_insert(deque *, size_t, void *);
void *deque_delete(deque *, size_t);
void *deque_index(const deque *, size_t);
void *deque_head(const deque *);
void *deque_prev(const deque *, const void **);
void *deque_next(const deque *, const void **);
void *deque_tail(const deque *);
size_t deque_size(const deque *);
void deque_sort(deque *, int (*)(const void *, const void *));
void *deque_search(const deque *, const void *, int (*)(const void *, const void *));
void deque_reverse(const deque *);
void deque_clear(deque *, void (*)(void *));
void deque_destroy(deque *, void (*)(void *));

#endif /* DEQUE_H */

