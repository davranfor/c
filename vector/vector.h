/*! 
 *  \brief     Vector
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef VECTOR_H
#define VECTOR_H

typedef struct
{
    void * data;            // The contents of the array
    size_t size;            // Number of elements of the array
    size_t szof;            // sizeof each element of the array
    void (*fdel)(void *);   // Pointer to callback to delete function
} vector;

vector *vector_create(size_t, void (*)(void *));
void *vector_resize(vector *, int);
void *vector_copy(vector *, const void *, size_t);
void *vector_concat(vector *, const void *, size_t);
void vector_sort(vector *, int (*)(const void *, const void *));
void *vector_bsearch(const vector *, const void *, int (*)(const void *, const void *));
void *vector_lsearch(const vector *, const void *, int (*)(const void *, const void *));
void *vector_min(const vector *, int (*)(const void *, const void *));
void *vector_max(const vector *, int (*)(const void *, const void *));
vector *vector_clear(vector *);
void vector_destroy(vector *);

#endif /* VECTOR_H */

