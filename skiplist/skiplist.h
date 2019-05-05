/*! 
 *  \brief     SkipList
 *  \author    David Ranieri (davranfor)
 *  \copyright GNU Public License.
 */

#ifndef LIST_H
#define LIST_H

struct cursor {
    void *node;
    int (*comp)(const void *, const void *);
    const void *data;
};

typedef struct skiplist skiplist;

skiplist *skiplist_create(int (*)(const void *, const void *));
void *skiplist_insert(skiplist *, void *);
void *skiplist_delete(skiplist *, const void *);
void *skiplist_search(skiplist *, const void *);
void *skiplist_fetch(skiplist *, struct cursor *);
void skiplist_destroy(skiplist *, void (*)(void *));

#endif /* LIST_H */

