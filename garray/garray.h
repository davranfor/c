/*! 
 *  \brief     Dynamic array with exponential growth
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef GARRAY_H
#define GARRAY_H

typedef struct garray garray;

garray *garray_create(size_t);
size_t garray_size(garray *);
void *garray_grow(garray *);
void *garray_at(garray *, size_t);
void garray_destroy(garray *);

#endif /* GARRAY_H */

