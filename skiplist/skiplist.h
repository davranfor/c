/*! 
 *  \brief     SkipList
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef SKIPLIST_H
#define SKIPLIST_H

typedef struct skiplist skiplist;

skiplist *skiplist_create(int (*)(const void *, const void *));
void *skiplist_insert(skiplist *, void *);
void *skiplist_delete(skiplist *, const void *);
void *skiplist_search(const skiplist *, const void *);
void *skiplist_fetch(const skiplist *, const void **, const void *,
    int (*)(const void *, const void *));
void skiplist_destroy(skiplist *, void (*)(void *));

#endif /* SKIPLIST_H */

