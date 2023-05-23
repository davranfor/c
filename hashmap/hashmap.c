/*! 
 *  \brief     HashMap
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdlib.h>
#include "hashmap.h"

struct node
{
    void *data;
    struct node *next;
};

struct hashmap
{
    struct node *list;
    int (*comp)(const void *, const void *);
    unsigned long (*hash)(const void *);
    size_t room;
    size_t size;
    hashmap *next;
};

static const size_t primes[] =
{
    53,        97,         193,        389,
    769,       1543,       3079,       6151,
    12289,     24593,      49157,      98317, 
    196613,    393241,     786433,     1572869,
    3145739,   6291469,    12582917,   25165843, 
    50331653,  100663319,  201326611,  402653189,
    805306457, 1610612741, 3221225473, 4294967291
};

hashmap *hashmap_create(
    int (*comp)(const void *, const void *),
    unsigned long (*hash)(const void *),
    size_t size)
{
    enum {NPRIMES = sizeof primes / sizeof *primes};

    for (size_t iter = 0; iter < NPRIMES; iter++)
    {
        if (size < primes[iter])
        {
            size = primes[iter];
            break;
        }
    }

    hashmap *map = calloc(1, sizeof *map);

    if (map != NULL)
    {
        map->list = calloc(size, sizeof(struct node));
        if (map->list == NULL)
        {
            free(map);
            return NULL;
        }
        map->comp = comp;
        map->hash = hash;
        map->room = size;
    }
    return map;
}

static void reset(hashmap *map)
{
    hashmap *next = map->next;

    free(map->list);
    map->list = next->list;
    map->room = next->room;
    map->size = next->size;
    map->next = next->next;
    free(next);
}

static hashmap *rehash(hashmap *map, struct node *node)
{
    size_t size = map->size;

    while (node != NULL)
    {
        if (hashmap_insert(map->next, node->data) == NULL)
        {
            return NULL;
        }
    
        struct node *next = node->next;

        if (size == map->size--)
        {
            node->next = NULL;
            node->data = NULL;
        }
        else
        {
            free(node);
        }
        node = next;
    }
    if (map->size == 0)
    {
        reset(map);
        return map;
    }
    return map->next;
}

void *hashmap_insert(hashmap *map, void *data)
{
    unsigned long hash = map->hash(data);

    while (map != NULL)
    {
        struct node *node = map->list + hash % map->room;

        // We are not in the last table
        if (map->next != NULL)
        {
            if (node->data != NULL)
            {
                map = rehash(map, node);
            }
            else
            {
                map = map->next;
            }
            continue;
        }

        // We are in the last table
        while (node->data != NULL)
        {
            if (map->comp(node->data, data) == 0)
            {
                return node->data;
            }
            if (node->next == NULL)
            {
                node->next = calloc(1, sizeof *node);
                if (node->next == NULL)
                {
                    return NULL;
                }
            }
            node = node->next;
        }
        node->data = data;

        // If more than 75% occupied then create a new table
        if (++map->size > map->room - map->room / 4)
        {
            map->next = hashmap_create(map->comp, map->hash, map->room);
            if (map->next == NULL)
            {
                return NULL;
            }
        }
        return data;
    }
    return NULL;
}

void *hashmap_delete(hashmap *map, const void *data)
{
    unsigned long hash = map->hash(data);

    while (map != NULL)
    {
        struct node *node = map->list + hash % map->room;
        struct node *temp = NULL;
        
        if (node->data != NULL) do
        {
            if (map->comp(node->data, data) == 0)
            {
                void *result = node->data;

                if (temp == NULL)
                {
                    if (node->next == NULL)
                    {
                        node->data = NULL;
                    }
                    else
                    {
                        temp = node->next;
                        node->data = temp->data;
                        node->next = temp->next;
                        free(temp);
                    }
                }
                else
                {
                    temp->next = node->next;
                    free(node);
                }
                if ((--map->size == 0) && (map->next != NULL))
                {
                    reset(map);
                }
                return result;
            }
            temp = node;
            node = node->next;
        } while (node != NULL);

        // Not found in this table, try in the next one
        map = map->next;
    }
    return NULL;
}

void *hashmap_search(const hashmap *map, const void *data)
{
    unsigned long hash = map->hash(data);

    while (map != NULL)
    {
        const struct node *node = map->list + hash % map->room;

        if (node->data != NULL) do
        {
            if (map->comp(node->data, data) == 0)
            {
                return node->data;
            }
            node = node->next;
        } while (node != NULL);

        // Not found in this table, try in the next one
        map = map->next;
    }
    return NULL;
}

size_t hashmap_size(const hashmap *map)
{
    size_t size = 0;
    
    while (map != NULL)
    {
        size += map->size;
        map = map->next;
    }
    return size;
}

void *hashmap_copy(const hashmap *map, size_t *size)
{
    *size = hashmap_size(map);
    if (*size == 0)
    {
        return NULL;
    }

    void **data = malloc(*size * sizeof *data);

    if (data == NULL)
    {
        return NULL;
    }

    size_t count = 0;

    while (map != NULL)
    {
        for (size_t index = 0, elems = count; count - elems < map->size; index++)
        {
            const struct node *node = map->list + index;

            if (node->data != NULL) do
            {
                data[count++] = node->data;
                node = node->next;
            } while (node != NULL);
        }
        map = map->next;
    }
    return data;
}

void hashmap_destroy(hashmap *map, void (*func)(void *))
{
    void *temp;

    while (map != NULL)
    {
        for (size_t index = 0; map->size > 0; index++)
        {
            struct node *node = map->list + index;

            if (node->data != NULL) do
            {
                if (func != NULL)
                {
                    func(node->data);
                }
                temp = node;
                node = node->next;
                if (temp != map->list + index)
                {
                    free(temp);
                }
                map->size--;
            } while (node != NULL);
        }
        temp = map->next;
        free(map->list);
        free(map);
        map = temp;
    }
}

unsigned long hash_str(const unsigned char *key)
{
    unsigned long hash = 5381;
    unsigned char chr;

    while ((chr = *key++))
    {
        hash = ((hash << 5) + hash) + chr;
    }
    return hash;
}

unsigned long hash_ulong(unsigned long key)
{
    key = ((key >> 16) ^ key) * 0x45d9f3b;
    key = ((key >> 16) ^ key) * 0x45d9f3b;
    key = ((key >> 16) ^ key);
    return key;
}

unsigned long hash_ullong(unsigned long long key)
{
    key = (key ^ (key >> 30)) * 0xbf58476d1ce4e5b9;
    key = (key ^ (key >> 27)) * 0x94d049bb133111eb;
    key = (key ^ (key >> 31));
    return (unsigned long)key;
}

