/*! 
 *  \brief     Binary map
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef BINMAP_H
#define BINMAP_H

typedef struct binmap binmap;

binmap *binmap_create(size_t);
int binmap_set(binmap *, size_t, int);
int binmap_get(const binmap *, size_t);
void binmap_destroy(binmap *);

#endif /* BINMAP_H */

