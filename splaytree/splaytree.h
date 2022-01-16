/*! 
 *  \brief     SplayTree
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef SPLAYTREE_H
#define SPLAYTREE_H

typedef struct splaytree splaytree;

splaytree *splaytree_create(int (*)(const void *, const void *));
void *splaytree_insert(splaytree *, void *);
void *splaytree_delete(splaytree *, const void *);
void *splaytree_search(splaytree *, const void *);
void *splaytree_walk(const splaytree *, void *(*)(void *, void *), void *);
void splaytree_destroy(splaytree *, void (*)(void *));

#endif /* SPLAYTREE_H */

