/*! 
 *  \brief     Queue
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef QUEUE_H
#define QUEUE_H

typedef struct queue queue;

queue *queue_create(void);
void *queue_push(queue *, void *);
void *queue_pop(queue *);
void *queue_fetch(const queue *, const void **);
void *queue_head(const queue *);
void *queue_tail(const queue *);
size_t queue_size(const queue *);
void queue_destroy(queue *, void (*)(void *));

#endif /* QUEUE_H */

