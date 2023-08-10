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
    struct node **list;
    unsigned long (*hash)(const void *);
    int (*comp)(const void *, const void *);
    hashmap *next;
    size_t room;
    size_t size;
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
    unsigned long (*hash)(const void *),
    int (*comp)(const void *, const void *),
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
        map->list = calloc(size, sizeof *map->list);
        if (map->list == NULL)
        {
            free(map);
            return NULL;
        }
        map->hash = hash;
        map->comp = comp;
        map->room = size;
    }
    return map;
}

static struct node *insert(struct node *next, void *data)
{
    struct node *node = malloc(sizeof *node);

    if (node != NULL)
    {
        node->next = next;
        node->data = data;
    }
    return node;
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

static void move(hashmap *map, struct node *node)
{
    struct node **head = map->list + map->hash(node->data) % map->room;

    node->next = *head;
    *head = node;
    map->size++;
}

static hashmap *rehash(hashmap *map, unsigned long hash)
{
    while (map->next != NULL)
    {
        struct node **head = map->list + hash % map->room;
        struct node *node = *head;

        *head = NULL;
        while (node != NULL)
        {
            struct node *next = node->next;

            move(map->next, node);
            map->size--;
            node = next;
        }
        if (map->size == 0)
        {
            reset(map);
        }
        else
        {
            map = map->next;
        }
    }
    return map;
}

void *hashmap_insert(hashmap *map, void *data)
{
    if (map != NULL)
    {
        unsigned long hash = map->hash(data);

        map = rehash(map, hash);

        struct node **head = map->list + hash % map->room;
        struct node *node = *head;

        while (node != NULL)
        {
            if (map->comp(node->data, data) == 0)
            {
                return node->data;
            }
            node = node->next;
        }
        *head = insert(*head, data);
        if (*head == NULL)
        {
            return NULL;
        }
        // If more than 75% occupied then create a new table
        if (++map->size > map->room - map->room / 4)
        {
            map->next = hashmap_create(map->hash, map->comp, map->room);
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
    if (map != NULL)
    {
        unsigned long hash = map->hash(data);

        map = rehash(map, hash);

        struct node **head = map->list + hash % map->room;
        struct node *node = *head, *prev = NULL;

        while (node != NULL)
        {
            if (map->comp(node->data, data) == 0)
            {
                void *temp = node->data;

                if (prev != NULL)
                {
                    prev->next = node->next;
                }
                else
                {
                    *head = node->next;
                }
                free(node);
                map->size--;
                return temp;
            }
            prev = node;
            node = node->next;
        }
    }
    return NULL;
}

void *hashmap_search(const hashmap *map, const void *data)
{
    unsigned long hash = map->hash(data);

    while (map != NULL)
    {
        const struct node *node = map->list[hash % map->room];

        while (node != NULL)
        {
            if (map->comp(node->data, data) == 0)
            {
                return node->data;
            }
            node = node->next;
        }
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

void hashmap_destroy(hashmap *map, void (*function)(void *))
{
    while (map != NULL)
    {
        for (size_t index = 0; map->size > 0; index++)
        {
            struct node *node = map->list[index];

            while (node != NULL)
            {
                struct node *next = node->next;

                if (function != NULL)
                {
                    function(node->data);
                }
                free(node);
                node = next;
                map->size--;
            }
        }

        hashmap *next = map->next;

        free(map->list);
        free(map);
        map = next;
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

