/*! 
 *  \brief     knode (Kernel list concept applied to a XOR linked list)
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdlib.h>
#include <stdalign.h>
#include "knode.h"

struct knode
{
    void *head;
    void *tail;
    size_t szof;
    size_t size;
};

typedef uintptr_t addr;

knode *knode_create(size_t size)
{
    knode *list = calloc(1, sizeof *list);

    if (list != NULL)
    {
        size_t align = alignof(addr);

        // Round size up to nearest multiple of alignof(uintptr_t)
        list->szof = (size + (align - 1)) / align * align;
    }
    return list;
}

#define knode_const_addr(list, data) ((const uintptr_t *)((uintptr_t)data + list->szof))
#define knode_addr(list, data) ((uintptr_t *)((uintptr_t)data + list->szof))

void *knode_push_head(knode *list)
{
    void *data = calloc(1, list->szof + sizeof(addr));

    if (data == NULL)
    {
        return NULL;
    }
    if (list->head != NULL)
    {
        *knode_addr(list, list->head) ^= (addr)data;
        *knode_addr(list, data) = (addr)list->head;
    }
    else
    {
        list->tail = data;
    }
    list->head = data;
    list->size++;
    return data;
}

void *knode_push_tail(knode *list)
{
    void *data = calloc(1, list->szof + sizeof(addr));

    if (data == NULL)
    {
        return NULL;
    }
    if (list->tail != NULL)
    {
        *knode_addr(list, list->tail) ^= (addr)data;
        *knode_addr(list, data) = (addr)list->tail;
    }
    else
    {
        list->head = data;
    }
    list->tail = data;
    list->size++;
    return data;
}

void *knode_pop_head(knode *list)
{
    void *data = list->head;

    if (data == NULL)
    {
        return NULL;
    }

    addr *head = knode_addr(list, data);
    void *next = (void *)*head;

    if (next != NULL)
    {
        *knode_addr(list, next) ^= (addr)data;
        *head = 0;
    }
    else
    {
        list->tail = NULL;
    }
    list->head = next;
    list->size--;
    return data;
}

void *knode_pop_tail(knode *list)
{
    void *data = list->tail;

    if (data == NULL)
    {
        return NULL;
    }

    addr *tail = knode_addr(list, data);
    void *prev = (void *)*tail;

    if (prev != NULL)
    {
        *knode_addr(list, prev) ^= (addr)data;
        *tail = 0;
    }
    else
    {
        list->head = NULL;
    }
    list->tail = prev;
    list->size--;
    return data;
}

void *knode_head(const knode *list)
{
    return list->head;
}

void *knode_tail(const knode *list)
{
    return list->tail;
}

void *knode_fetch(const knode *list, kiter *iter, const void *data)
{
    if (*iter == KNODE_HEAD)
    {
        *iter = 0;
        return list->head;
    }
    if (*iter == KNODE_TAIL)
    {
        *iter = 0;
        return list->tail;
    }
    
    addr next = *knode_const_addr(list, data) ^ *iter;

    *iter = (kiter)data;
    return (void *)next;
}

void *knode_index(const knode *list, size_t index)
{
    if (index >= list->size)
    {
        return NULL;
    }

    void *node, *prev = NULL;

    if (list->size - index > index)
    {
        node = list->head;
    }
    else
    {
        index = list->size - index - 1;
        node = list->tail;
    }
    for (size_t iter = 0; iter < index; iter++)
    {
        void *temp = node;

        node = (void *)(*knode_addr(list, node) ^ (addr)prev);
        prev = temp;
    }
    return node;
}

size_t knode_size(const knode *list)
{
    return list->size;
}

void knode_clear(knode *list, void (*func)(void *))
{
    void *node = list->head;

    while (node != NULL)
    {
        addr *head = knode_addr(list, node);
        void *next = (void *)*head;

        if (next != NULL)
        {
            *knode_addr(list, next) ^= (addr)node;
            *head = 0;
        }
        if (func != NULL)
        {
            func(node);
        }
        node = next;
    }
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

void knode_destroy(knode *list, void (*func)(void *))
{
    if (list != NULL)
    {
        knode_clear(list, func);
        free(list);
    }
}

