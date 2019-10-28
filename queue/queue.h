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
void *queue_fetch(queue *, void **);
void *queue_head(queue *);
void *queue_tail(queue *);
size_t queue_size(const queue *);
void queue_destroy(queue *, void (*)(void *));

#endif /* QUEUE_H */

