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
void *hashmap_search(const hashmap *, const void *);
size_t hashmap_size(const hashmap *);
void *hashmap_copy(const hashmap *, size_t *);
void hashmap_destroy(hashmap *, void (*)(void *));
unsigned long hash_string(unsigned char *);
unsigned long hash_ulong(unsigned long);
unsigned long hash_ullong(unsigned long long);

#endif /* HASHMAP_H */

