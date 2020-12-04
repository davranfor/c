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

static knode *split(knode *head)
{
    knode *fast = head;
    knode *slow = head;

    while ((fast->next != NULL) && (fast->next->next != NULL))
    {
        fast = fast->next->next;
        slow = slow->next;
    }

    knode *temp = slow->next;

    slow->next = NULL;
    return temp;
}

static knode *merge(klist *list, knode *first, knode *second, int (*comp)(const void *, const void *))
{
    if (first == NULL)
    {
        return second;
    }
    if (second == NULL)
    {
        return first;
    }
    if (comp(klist_data(list, first), klist_data(list, second)) < 0)
    {
        first->next = merge(list, first->next, second, comp);
        return first;
    }
    else
    {
        second->next = merge(list, first, second->next, comp);
        return second;
    }
}

static knode *sort(klist *list, knode *head, int (*comp)(const void *, const void *))
{
    if ((head == NULL) || (head->next == NULL))
    {
        return head;
    }

    knode *second = split(head);

    head = sort(list, head, comp);
    second = sort(list, second, comp);
    return merge(list, head, second, comp);
}

/** 
 * Merge sort:
 * Since we need to update the tail, we use a singly linked list sort approach and
 * adjust the `prev` nodes and `list->tail` at the end
 */
void klist_sort(klist *list, int (*comp)(const void *, const void *))
{
    if (list->size > 1)
    {
        list->head = sort(list, list->head, comp);
        list->head->prev = NULL;

        knode *node = list->head;

        while (node->next != NULL)
        {
            node->next->prev = node;
            node = node->next;
        }
        list->tail = node;
    }
}

void *klist_search(const klist *list, const void *data, int (*comp)(const void *, const void *))
{
    knode *iter;

    for (iter = list->head; iter != NULL; iter = iter->next)
    {
        if (comp(klist_data(list, iter), data) == 0)
        {
            return klist_data(list, iter);
        }
    }
    return NULL;
}

void klist_reverse(klist *list)
{
    knode *a = list->head;
    knode *b = list->tail;

    list->head = b;
    list->tail = a;

    size_t mid = (list->size / 2) + (list->size % 2);

    for (size_t iter = 0; iter < mid; iter++)
    {
        knode *a_prev = a->prev;
        knode *a_next = a->next;
        knode *b_prev = b->prev;
        knode *b_next = b->next;

        knode **pa = &a;
        knode **pb = &b;
        knode *tmp;

        tmp = *pa;
        *pa = *pb;
        *pb = tmp;

        a->prev = b_next;
        a->next = b_prev;
        b->prev = a_next;
        b->next = a_prev;

        a = a_next;
        b = b_prev;
    }
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

