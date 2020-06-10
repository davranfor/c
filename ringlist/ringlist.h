/*! 
 *  \brief     Circular list
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef RINGLIST_H
#define RINGLIST_H

typedef struct ringlist ringlist;

ringlist *ringlist_create(void);
void *ringlist_push(ringlist *, void *);
void *ringlist_pop(ringlist *);
void *ringlist_fetch(const ringlist *, const void **);
void *ringlist_head(const ringlist *);
void *ringlist_tail(const ringlist *);
size_t ringlist_size(const ringlist *);
void ringlist_destroy(ringlist *, void (*)(void *));

#endif /* RINGLIST_H */

