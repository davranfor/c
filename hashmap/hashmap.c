/*! 
 *  \brief     HashMap
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdlib.h>
#include "hashmap.h"

struct node {
    void *data;
    struct node *next;
};

struct hashmap {
    struct node *list;
    int (*comp)(const void *, const void *);
    size_t (*hash)(const void *);
    size_t room;
    size_t size;
};

static const size_t primes[] = {
    53ul,        97ul,         193ul,        389ul,
    769ul,       1543ul,       3079ul,       6151ul,
    12289ul,     24593ul,      49157ul,      98317ul, 
    196613ul,    393241ul,     786433ul,     1572869ul,
    3145739ul,   6291469ul,    12582917ul,   25165843ul, 
    50331653ul,  100663319ul,  201326611ul,  402653189ul,
    805306457ul, 1610612741ul, 3221225473ul, 4294967291ul
};

hashmap *hashmap_create(
    int (*comp)(const void *, const void *),
    size_t (*hash)(const void *),
    size_t size)
{
    hashmap *map;
    size_t iter, nprimes;

    nprimes = sizeof primes / sizeof *primes;
    for (iter = 0; iter < nprimes; iter++) {
        if (size < primes[iter]) {
            size = primes[iter];
            break;
        }
    }
    map = malloc(sizeof *map);
    if (map != NULL) {
        map->list = calloc(size + 1, sizeof(struct node));
        if (map->list == NULL) {
            return NULL;
        }
        map->comp = comp;
        map->hash = hash;
        map->room = size;
        map->size = 0;
    }
    return map;
}

static void hashmap_move(hashmap *head, hashmap *tail)
{
    free(head->list);
    head->list = tail->list;
    head->room = tail->room;
    head->size = tail->size;
    free(tail);
}

static size_t hashmap_rehash(hashmap *map, struct node *node)
{
    struct node *temp;
    size_t size = 0;

    while (node != NULL) {
        if (hashmap_insert(map, node->data) == NULL) {
            return 0;
        }
        temp = node->next;
        if (size > 0) {
            free(node);
        }
        node = temp;
        size++;
    }
    return size;
}

void *hashmap_insert(hashmap *map, void *data)
{
    struct node *node;
    struct node *tail;
    size_t size = 0;
    size_t hash;

    hash = map->hash(data) % map->room;
    tail = map->list + map->room;
    node = map->list + hash;
    if (tail->data != NULL) {
        if (node->data != NULL) {
            size = hashmap_rehash(tail->data, node);
            if (size == 0) {
                return NULL;
            }
            map->size -= size;
            node->next = NULL;
            node->data = NULL;
        }
        data = hashmap_insert(tail->data, data);
        if (map->size == 0) {
            hashmap_move(map, tail->data);
        }
        return data;
    }
    while (node->data != NULL) {
        if (map->comp(node->data, data) == 0) {
            return node->data;
        }
        if (node->next == NULL) {
            node->next = calloc(1, sizeof *node);
            if (node->next == NULL) {
                return NULL;
            }
        }
        node = node->next;
    }
    node->data = data;
    map->size++;
    if (map->size > map->room - map->room / 4) {
        tail->data = hashmap_create(map->comp, map->hash, map->room);
        if (tail->data == NULL) {
            return NULL;
        }
    }
    return data;
}

void *hashmap_delete(hashmap *map, const void *data)
{
    struct node *node, *temp = NULL;
    size_t hash;
    void *res;
    
    hash = map->hash(data) % map->room;
    node = map->list + hash;
    if (node->data != NULL)
    do {
        if (map->comp(node->data, data) == 0) {
            res = node->data;
            if (temp == NULL) {
                if (node->next == NULL) {
                    node->data = NULL;
                } else {
                    temp = node->next;
                    node->data = temp->data;
                    node->next = temp->next;
                    free(temp);
                }
            } else {
                temp->next = node->next;
                free(node);
            }
            map->size--;
            return res;
        }
        temp = node;
        node = node->next;
    } while (node != NULL);
    node = map->list + map->room;
    if (node->data != NULL) {
        return hashmap_delete(node->data, data);
    }
    return NULL;
}

void *hashmap_search(hashmap *map, const void *data)
{   
    struct node *node;
    size_t hash;
    
    hash = map->hash(data) % map->room;
    node = map->list + hash;
    if (node->data != NULL)
    do {
        if (map->comp(node->data, data) == 0) {
            return node->data;
        }
        node = node->next;
    } while (node != NULL);
    node = map->list + map->room;
    if (node->data != NULL) {
        return hashmap_search(node->data, data);
    }
    return NULL;
}

void hashmap_destroy(hashmap *map, void (*func)(void *))
{   
    struct node *node, *temp;
    size_t iter;

    if (map->size > 0) {
        for (iter = 0; iter < map->room; iter++) {
            node = map->list + iter;
            if (node->data != NULL) {
                if (func != NULL) {
                    func(node->data);
                }
            }
            if (node->next != NULL) {
                node = node->next;
                while (node != NULL) {
                    if (func != NULL) {
                        func(node->data);
                    }
                    temp = node;
                    node = node->next;
                    free(temp);
                }
            }
        }
    }
    node = map->list + map->room;
    if (node->data != NULL) {
        hashmap_destroy(node->data, func);
    }
    free(map->list);
    free(map);
}

size_t hashmap_hash_str(unsigned char *key)
{
    size_t hash = 5381;
    size_t chr;

    while ((chr = *key++)) {
        hash = ((hash << 5) + hash) + chr;
    }
    return hash;
}

size_t hashmap_hash_uint(unsigned int key)
{
    key = ((key >> 16) ^ key) * 0x45d9f3b;
    key = ((key >> 16) ^ key) * 0x45d9f3b;
    key = ((key >> 16) ^ key);
    return key;
}

size_t hashmap_hash_ulong(unsigned long int key)
{
    key = (key ^ (key >> 30)) * 0xbf58476d1ce4e5b9;
    key = (key ^ (key >> 27)) * 0x94d049bb133111eb;
    key = (key ^ (key >> 31));
    return key;
}
