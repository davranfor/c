/*! 
 *  \brief     Vector
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef VECTOR_H
#define VECTOR_H

void *vcreate(size_t);
void vdestroy(void *, void (*)(void *));
size_t vsize(const void *);
void *vgrow(void *);
void *vadd(void *, size_t);
void vsort(void *, int (*)(const void *, const void *));
void *vsearch(const void *, const void *, int (*)(const void *, const void *));
char *strdup_printf(const char *, ...);

#endif /* VECTOR_H */

