/*! 
 *  \brief     Dynamic array
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef DYNARRAY_H
#define DYNARRAY_H

typedef struct dynarray dynarray;

dynarray *dynarray_create(size_t);
void *dynarray_add(dynarray *, void *);
void *dynarray_get(dynarray *, size_t);
size_t dynarray_size(dynarray *);
void dynarray_sort(dynarray *, int (*)(const void *, const void *));
void *dynarray_bsearch(const dynarray *, const void *, int (*)(const void *, const void *));
void *dynarray_lsearch(const dynarray *, const void *, int (*)(const void *, const void *));
void *dynarray_resize(dynarray *, size_t, void (*)(void *));
void dynarray_destroy(dynarray *, void (*)(void *));

#endif /* DYNARRAY_H */

