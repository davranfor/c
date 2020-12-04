/*! 
 *  \brief     klist (Kernel list concept)
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdlib.h>
#include <stdalign.h>
#include "klist.h"

typedef struct knode knode;

struct knode
{
    knode *next;
    knode *prev;
};

struct klist
{
    knode *head;
    knode *tail;
    size_t szof;
    size_t size;
};

klist *klist_create(size_t size)
{
    klist *list = calloc(1, sizeof *list);

    if (list != NULL)
    {
        size_t align = alignof(knode);

        // Rounding size up to nearest multiple of alignof(knode)
        list->szof = (size + (align - 1)) / align * align;
    }
    return list;
}

#define klist_const_node(list, data) ((const knode *)((const char *)data + list->szof))
#define klist_node(list, data) ((knode *)((char *)data + list->szof))
#define klist_data(list, node) ((void  *)((char *)node - list->szof))

void *klist_push_head(klist *list)
{
    void *data = calloc(1, list->szof + sizeof(knode));

    if (data == NULL)
    {
        return NULL;
    }

    knode *node = klist_node(list, data);

    if (list->head != NULL)
    {
        list->head->prev = node;
        node->next = list->head;
    }
    list->head = node;
    if (list->tail == NULL)
    {
        list->tail = node;
    }
    list->size++;
    return data;
}

void *klist_push_tail(klist *list)
{
    void *data = calloc(1, list->szof + sizeof(knode));

    if (data == NULL)
    {
        return NULL;
    }

    knode *node = klist_node(list, data);

    if (list->tail != NULL)
    {
        list->tail->next = node;
        node->prev = list->tail;
    }
    list->tail = node;
    if (list->head == NULL)
    {
        list->head = node;
    }
    list->size++;
    return data;
}

void *klist_pop_head(klist *list)
{
    if (list->head == NULL)
    {
        return NULL;
    }

    void *data = klist_data(list, list->head);

    list->head = list->head->next;
    if (list->head != NULL)
    {
        list->head->prev = NULL;
    }
    else
    {
        list->tail = NULL;
    }
    list->size--;
    return data;
}

void *klist_pop_tail(klist *list)
{
    if (list->tail == NULL)
    {
        return NULL;
    }

    void *data = klist_data(list, list->tail);

    list->tail = list->tail->prev;
    if (list->tail != NULL)
    {
        list->tail->next = NULL;
    }
    else
    {
        list->head = NULL;
    }
    list->size--;
    return data;
}

void *klist_head(const klist *list)
{
    if (list->head != NULL)
    {
        return klist_data(list, list->head);
    }
    return NULL;
}

void *klist_prev(const klist *list, const void *data)
{
    if (data != NULL)
    {
        const knode *node = klist_const_node(list, data);

        if (node->prev != NULL)
        {
            return klist_data(list, node->prev);
        }
    }
    return NULL;
}

void *klist_next(const klist *list, const void *data)
{
    if (data != NULL)
    {
        const knode *node = klist_const_node(list, data);

        if (node->next != NULL)
        {
            return klist_data(list, node->next);
        }
    }
    return NULL;
}

void *klist_tail(const klist *list)
{
    if (list->tail != NULL)
    {
        return klist_data(list, list->tail);
    }
    return NULL;
}

size_t klist_size(const klist *list)
{
    return list->size;
}

void klist_destroy(klist *list, void (*func)(void *))
{
    if (func != NULL)
    {
        knode *node = list->head;

        while (node != NULL)
        {
            knode *next = node->next;

            func(klist_data(list, node));
            node = next;
        }
    }
    free(list);
}

