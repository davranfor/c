/*! 
 *  \brief     klist (Kernel list concept)
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdalign.h>
#include "klist.h"

struct node
{
    struct node *prev;
    struct node *next;
};

struct klist
{
    struct node *head;
    struct node *tail;
    size_t szof;
    size_t size;
};

klist *klist_create(size_t size)
{
    klist *list = calloc(1, sizeof *list);

    if (list != NULL)
    {
        size_t align = alignof(struct node);

        // Round size up to nearest multiple of alignof(struct node)
        list->szof = (size + (align - 1)) / align * align;
    }
    return list;
}

#define klist_const_node(list, data) ((const void *)((uintptr_t)data + list->szof))
#define klist_node(list, data) ((void *)((uintptr_t)data + list->szof))
#define klist_data(list, node) ((void *)((uintptr_t)node - list->szof))

void *klist_push_head(klist *list)
{
    void *data = calloc(1, list->szof + sizeof(struct node));

    if (data == NULL)
    {
        return NULL;
    }

    struct node *node = klist_node(list, data);

    if (list->head != NULL)
    {
        list->head->prev = node;
        node->next = list->head;
    }
    else
    {
        list->tail = node;
    }
    list->head = node;
    list->size++;
    return data;
}

void *klist_push_tail(klist *list)
{
    void *data = calloc(1, list->szof + sizeof(struct node));

    if (data == NULL)
    {
        return NULL;
    }

    struct node *node = klist_node(list, data);

    if (list->tail != NULL)
    {
        list->tail->next = node;
        node->prev = list->tail;
    }
    else
    {
        list->head = node;
    }
    list->tail = node;
    list->size++;
    return data;
}

void *klist_pop_head(klist *list)
{
    struct node *node = list->head;

    if (node == NULL)
    {
        return NULL;
    }

    void *data = klist_data(list, node);

    list->head = node->next;
    if (node->next != NULL)
    {
        node->next->prev = NULL;
        node->next = NULL;
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
    struct node *node = list->tail;

    if (node == NULL)
    {
        return NULL;
    }

    void *data = klist_data(list, node);

    list->tail = node->prev;
    if (node->prev != NULL)
    {
        node->prev->next = NULL;
        node->prev = NULL;
    }
    else
    {
        list->head = NULL;
    }
    list->size--;
    return data;
}

static struct node *klist_get(const klist *list, size_t index)
{
    struct node *node;

    if (list->size - index > index)
    {
        node = list->head;
        for (size_t iter = 0; iter < index; iter++)
        {
            node = node->next;
        }
    }
    else
    {
        node = list->tail;
        for (size_t iter = list->size - 1; iter > index; iter--)
        {
            node = node->prev;
        }
    }
    return node;
}

void *klist_insert(klist *list, size_t index)
{
    if (index == 0)
    {
        return klist_push_head(list);
    }
    if (index >= list->size)
    {
        return klist_push_tail(list);
    }

    void *data = calloc(1, list->szof + sizeof(struct node));

    if (data == NULL)
    {
        return NULL;
    }

    struct node *node = klist_node(list, data); 
    struct node *curr = klist_get(list, index);

    curr->prev->next = node;
    node->prev = curr->prev;
    curr->prev = node;
    node->next = curr;
    list->size++;
    return data;
}

void *klist_delete(klist *list, size_t index)
{
    if (index >= list->size)
    {
        return NULL;
    }
    if (index == 0)
    {
        return klist_pop_head(list);
    }
    if (index == list->size - 1)
    {
        return klist_pop_tail(list);
    }

    struct node *node = klist_get(list, index);
    void *data = klist_data(list, node);

    node->prev->next = node->next;
    node->next->prev = node->prev;
    list->size--;
    return data;
}

void *klist_index(const klist *list, size_t index)
{
    if (index >= list->size)
    {
        return NULL;
    }
    return klist_data(list, klist_get(list, index));
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
        const struct node *node = klist_const_node(list, data);

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
        const struct node *node = klist_const_node(list, data);

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

static struct node *split(struct node *head)
{
    struct node *fast = head;
    struct node *slow = head;

    while ((fast->next != NULL) && (fast->next->next != NULL))
    {
        fast = fast->next->next;
        slow = slow->next;
    }

    struct node *temp = slow->next;

    slow->next = NULL;
    return temp;
}

/* Recursive version causing stack overflow on large datatsets
static struct node *merge(const klist *list, struct node *head, struct node *tail,
                          int (*comp)(const void *, const void *))
{
    if (head == NULL)
    {
        return tail;
    }
    if (tail == NULL)
    {
        return head;
    }
    if (comp(klist_data(list, head), klist_data(list, tail)) <= 0)
    {
        head->next = merge(list, head->next, tail, comp);
        return head;
    }
    else
    {
        tail->next = merge(list, head, tail->next, comp);
        return tail;
    }
}
*/

/* Iterative version */
static struct node *merge(const klist *list, struct node *head, struct node *tail,
                          int (*comp)(const void *, const void *))
{
    if (head == NULL)
    {
        return tail;
    }
    if (tail == NULL)
    {
        return head;
    }

    struct node *curr = head;

    while (tail != NULL)
    {
        struct node *prev = tail;

        tail = tail->next;
        prev->next = NULL;
        if (comp(klist_data(list, head), klist_data(list, prev)) > 0)
        {
            prev->next = head;
            head = prev;
            curr = head;
            continue;
        }
        while (1)
        {
            if (curr->next == NULL)
            {
                curr->next = prev;
                curr = curr->next;
                break;
            }
            else if (comp(klist_data(list, curr->next), klist_data(list, prev)) <= 0)
            {
                curr = curr->next;
                continue;
            }
            else
            {
                struct node *temp;

                temp = curr->next;
                curr->next = prev;
                prev->next = temp;
                break;
            }
        }
    }
    return head;
}

static struct node *sort(klist *list, struct node *head,
                         int (*comp)(const void *, const void *))
{
    if ((head == NULL) || (head->next == NULL))
    {
        return head;
    }

    struct node *tail = split(head);

    head = sort(list, head, comp);
    tail = sort(list, tail, comp);
    return merge(list, head, tail, comp);
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

        struct node *node = list->head;

        while (node->next != NULL)
        {
            node->next->prev = node;
            node = node->next;
        }
        list->tail = node;
    }
}

void *klist_search(const klist *list, const void *data,
                   int (*comp)(const void *, const void *))
{
    struct node *iter;

    for (iter = list->head; iter != NULL; iter = iter->next)
    {
        void *curr = klist_data(list, iter);

        if (comp(curr, data) == 0)
        {
            return curr;
        }
    }
    return NULL;
}

void klist_reverse(klist *list)
{
    struct node *head = list->head;
    struct node *tail = list->tail;

    list->head = tail;
    list->tail = head;
    while (head != NULL)
    {
        struct node *temp;

        temp = head->prev;
        head->prev = head->next;
        head->next = temp;            
        head = head->prev;
    }
}

void klist_clear(klist *list, void (*func)(void *))
{
    struct node *node = list->head;

    while (node != NULL)
    {
        struct node *next = node->next;

        node->prev = NULL;
        node->next = NULL;
        if (func != NULL)
        {
            func(klist_data(list, node));
        }
        node = next;
    }
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

void klist_destroy(klist *list, void (*func)(void *))
{
    klist_clear(list, func);
    free(list);
}

