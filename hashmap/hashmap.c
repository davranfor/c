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
};

static const size_t primes[] =
{
    53ul,        97ul,         193ul,        389ul,
    769ul,       1543ul,       3079ul,       6151ul,
    12289ul,     24593ul,      49157ul,      98317ul, 
    196613ul,    393241ul,     786433ul,     1572869ul,
    3145739ul,   6291469ul,    12582917ul,   25165843ul, 
    50331653ul,  100663319ul,  201326611ul,  402653189ul,
    805306457ul, 1610612741ul, 3221225473ul, 4294967291ul
};

#define HASHMAP_NPRIMES (sizeof primes / sizeof *primes)

hashmap *hashmap_create(
    int (*comp)(const void *, const void *),
    unsigned long (*hash)(const void *),
    size_t size)
{
    for (size_t iter = 0; iter < HASHMAP_NPRIMES; iter++)
    {
        if (size < primes[iter])
        {
            size = primes[iter];
            break;
        }
    }

    hashmap *map = malloc(sizeof *map);

    if (map != NULL)
    {
        map->list = calloc(size + 1, sizeof(struct node));
        if (map->list == NULL)
        {
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

    while (node != NULL)
    {
        if (hashmap_insert(map, node->data) == NULL)
        {
            return 0;
        }
        temp = node;
        node = node->next;
        if (size++ > 0)
        {
            free(temp);
        }
    }
    return size;
}

void *hashmap_insert(hashmap *map, void *data)
{
    struct node *node;
    struct node *tail;
    size_t hash;

    hash = map->hash(data) % map->room;
    tail = map->list + map->room;
    node = map->list + hash;

    // We are not in the last table
    if (tail->data != NULL)
    {
        if (node->data != NULL)
        {
            size_t size = hashmap_rehash(tail->data, node);

            if (size == 0)
            {
                return NULL;
            }
            node->next = NULL;
            node->data = NULL;
            map->size -= size;
            if (map->size == 0)
            {
                hashmap_move(map, tail->data);
                return hashmap_insert(map, data);
            }
        }
        return hashmap_insert(tail->data, data);
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
        tail->data = hashmap_create(map->comp, map->hash, map->room);
        if (tail->data == NULL)
        {
            return NULL;
        }
    }
    return data;
}

void *hashmap_delete(hashmap *map, const void *data)
{
    struct node *node, *temp = NULL;
    struct node *tail;
    size_t hash;
    
    hash = map->hash(data) % map->room;
    tail = map->list + map->room;
    node = map->list + hash;
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
            if ((--map->size == 0) && (tail->data != NULL))
            {
                hashmap_move(map, tail->data);
            }
            return result;
        }
        temp = node;
        node = node->next;
    } while (node != NULL);

    // Not found in this table, try in the next one
    if (tail->data == NULL)
    {
        return NULL;
    }
    return hashmap_delete(tail->data, data);
}

void *hashmap_search(hashmap *map, const void *data)
{   
    struct node *node;
    size_t hash;
    
    hash = map->hash(data) % map->room;
    node = map->list + hash;
    if (node->data != NULL) do
    {
        if (map->comp(node->data, data) == 0)
        {
            return node->data;
        }
        node = node->next;
    } while (node != NULL);

    // Not found in this table, try in the next one
    node = map->list + map->room;
    if (node->data == NULL)
    {
        return NULL;
    }
    return hashmap_search(node->data, data);
}

void hashmap_destroy(hashmap *map, void (*func)(void *))
{   
    struct node *node;
    size_t index = 0;

    while (map->size > 0)
    {
        node = map->list + index;
        if (node->data != NULL) do
        {
            struct node *temp;

            if (func != NULL)
            {
                func(node->data);
            }
            map->size--;
            temp = node;
            node = node->next;
            // The pointer in the table can not be freed
            if (temp != map->list + index)
            {
                free(temp);
            }
        } while (node != NULL);
        index++;
    }

    void *next = (struct node *)(map->list + map->room)->data;

    free(map->list);
    free(map);

    // If there are more tables then destroy them also
    if (next != NULL)
    {
        hashmap_destroy(next, func);
    }
}

unsigned long hashmap_hash_string(unsigned char *key)
{
    unsigned long hash = 5381;
    unsigned int chr;

    while ((chr = *key++))
    {
        hash = ((hash << 5) + hash) + chr;
    }
    return hash;
}

unsigned long hashmap_hash_ulong(unsigned long key)
{
    key = ((key >> 16) ^ key) * 0x45d9f3b;
    key = ((key >> 16) ^ key) * 0x45d9f3b;
    key = ((key >> 16) ^ key);
    return key;
}

unsigned long hashmap_hash_ullong(unsigned long long int key)
{
    key = (key ^ (key >> 30)) * 0xbf58476d1ce4e5b9;
    key = (key ^ (key >> 27)) * 0x94d049bb133111eb;
    key = (key ^ (key >> 31));
    return (unsigned long)key;
}

