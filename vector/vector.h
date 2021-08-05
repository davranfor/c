/*! 
 *  \brief     Vector
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef VECTOR_H
#define VECTOR_H

typedef struct vector vector;

vector *vector_create(size_t, void (*)(void *));
void *vector_resize(vector *, int);
void *vector_insert(vector *, size_t);
void *vector_delete(vector *, size_t);
void *vector_copy(vector *, const void *, size_t);
void *vector_concat(vector *, const void *, size_t);
void vector_sort(vector *, int (*)(const void *, const void *));
void *vector_bsearch(const vector *, const void *, int (*)(const void *, const void *));
void *vector_lsearch(const vector *, const void *, int (*)(const void *, const void *));
vector *vector_clear(vector *);
void vector_destroy(vector *);

#endif /* VECTOR_H */

