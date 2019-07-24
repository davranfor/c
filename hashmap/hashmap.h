/*! 
 *  \brief     HashMap
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef HASHMAP_H
#define HASHMAP_H

typedef struct hashmap hashmap;

hashmap *hashmap_create(
    int (*)(const void *, const void *),
    unsigned long (*)(const void *),
    size_t
);
void *hashmap_insert(hashmap *, void *);
void *hashmap_delete(hashmap *, const void *);
void *hashmap_search(hashmap *, const void *);
void hashmap_destroy(hashmap *, void (*)(void *));
unsigned long hashmap_hash_str(unsigned char *);
unsigned long hashmap_hash_uint(unsigned int);
unsigned long hashmap_hash_ulong(unsigned long);

#endif /* HASHMAP_H */

