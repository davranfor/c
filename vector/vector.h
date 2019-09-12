/*! 
 *  \brief     Vector
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef VECTOR_H
#define VECTOR_H

void *vector_create(size_t);
void *vector_resize(void *);
size_t vector_size(const void *);
void vector_sort(void *, int (*)(const void *, const void *));
void *vector_search(const void *, const void *, int (*)(const void *, const void *));
void vector_destroy(void *, void (*)(void *));

#endif /* VECTOR_H */

