/*! 
 *  \brief     SkipList
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef LIST_H
#define LIST_H

typedef struct skiplist skiplist;

skiplist *skiplist_create(int (*)(const void *, const void *));
void *skiplist_insert(skiplist *, void *);
void *skiplist_delete(skiplist *, const void *);
void *skiplist_search(skiplist *, const void *);
void *skiplist_fetch(skiplist *, void **, const void *,
    int (*)(const void *, const void *));
void skiplist_destroy(skiplist *, void (*)(void *));

#endif /* LIST_H */

