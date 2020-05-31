/*! 
 *  \brief     Doubly linked list
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdlib.h>
#include "linklist.h"

struct node
{
    void *data;
    struct node *prev;
    struct node *next;
};

struct linklist
{
    struct node *head;
    struct node *tail;
    size_t size;
};

linklist *linklist_create(void)
{
    return calloc(1, sizeof(struct linklist));
}

void *linklist_push_head(linklist *list, void *data)
{
    if (data == NULL)
    {
        return NULL;
    }

    struct node *node = malloc(sizeof *node);

    if (node == NULL)
    {
        return NULL;
    }
    node->data = data;
    node->prev = NULL;
    if (list->head == NULL)
    {
        list->tail = node;
        node->next = NULL;
    }
    else
    {
        list->head->prev = node;
        node->next = list->head;
    }
    list->head = node;
    list->size++;
    return data;
}

void *linklist_push_tail(linklist *list, void *data)
{
    if (data == NULL)
    {
        return NULL;
    }

    struct node *node = malloc(sizeof *node);

    if (node == NULL)
    {
        return NULL;
    }
    node->data = data;
    node->next = NULL;
    if (list->head == NULL)
    {
        list->head = node;
        node->prev = NULL;
    }
    else
    {
        list->tail->next = node;
        node->prev = list->tail;
    }
    list->tail = node;
    list->size++;
    return data;
}

void *linklist_pop_head(linklist *list)
{
    void *data = NULL;

    if (list->head != NULL)
    {
        struct node *node = list->head;

        data = node->data;
        list->head = node->next;
        if (list->head == NULL)
        {
            list->tail = NULL;
        }
        else
        {
            list->head->prev = NULL;
        }
        list->size--;
        free(node);
    }
    return data;
}

void *linklist_pop_tail(linklist *list)
{
    void *data = NULL;

    if (list->tail != NULL)
    {
        struct node *node = list->tail;

        data = node->data;
        list->tail = node->prev;
        if (list->tail == NULL)
        {
            list->head = NULL;
        }
        else
        {
            list->tail->next = NULL;
        }
        list->size--;
        free(node);
    }
    return data;
}

static struct node *linklist_node(linklist *list, size_t index)
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

void *linklist_insert(linklist *list, size_t index, void *data)
{
    if (index > list->size)
    {
        return NULL;
    }
    if (index == 0)
    {
        return linklist_push_head(list, data);
    }
    if (index == list->size)
    {
        return linklist_push_tail(list, data);
    }
    if (data == NULL)
    {
        return NULL;
    }

    struct node *node = malloc(sizeof *node);

    if (node == NULL)
    {
        return NULL;
    }
        
    struct node *curr = linklist_node(list, index);

    node->prev = curr->prev;
    curr->prev = node;
    node->next = curr;
    node->data = data;
    list->size++;
    return data;
}

void *linklist_delete(linklist *list, size_t index)
{
    if (index >= list->size)
    {
        return NULL;
    }
    if (index == 0)
    {
        return linklist_pop_head(list);
    }
    if (index == list->size - 1)
    {
        return linklist_pop_tail(list);
    }

    struct node *node = linklist_node(list, index);

    node->prev->next = node->next;
    node->next->prev = node->prev;
    list->size--;

    void *data = node->data;

    free(node);
    return data;
}

void *linklist_index(linklist *list, size_t index)
{
    if (index >= list->size)
    {
        return NULL;
    }
    return linklist_node(list, index)->data;
}

void *linklist_head(linklist *list)
{
    return list->head;
}

void *linklist_prev(linklist *list, void **iter)
{
    void *data = NULL;

    if (*iter == list)
    {
        if (list->tail != NULL)
        {
            data = list->tail->data;
        }
        *iter = list->tail;
    }
    else
    {
        struct node *node = *iter;

        if (node == NULL)
        {
            return NULL;
        }
        if (node->prev != NULL)
        {
            data = node->prev->data;
        }
        *iter = node->prev;
    }
    return data;
}

void *linklist_next(linklist *list, void **iter)
{
    void *data = NULL;

    if (*iter == list)
    {
        if (list->head != NULL)
        {
            data = list->head->data;
        }
        *iter = list->head;
    }
    else
    {
        struct node *node = *iter;

        if (node == NULL)
        {
            return NULL;
        }
        if (node->next != NULL)
        {
            data = node->next->data;
        }
        *iter = node->next;
    }
    return data;
}

void *linklist_tail(linklist *list)
{
    return list->tail;
}

size_t linklist_size(const linklist *list)
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

static struct node *merge(struct node *first, struct node *second, int (*comp)(const void *, const void *))
{
    if (first == NULL)
    {
        return second;
    }
    if (second == NULL)
    {
        return first;
    }
    if (comp(first->data, second->data) < 0)
    {
        first->next = merge(first->next, second, comp);
        return first;
    }
    else
    {
        second->next = merge(first, second->next, comp);
        return second;
    }
}

static struct node *sort(struct node *head, int (*comp)(const void *, const void *))
{
    if ((head == NULL) || (head->next == NULL))
    {
        return head;
    }

    struct node *second = split(head);

    head = sort(head, comp);
    second = sort(second, comp);
    return merge(head, second, comp);
}

/** 
 * Merge sort:
 * Since we need to update the tail, we use a singly linked list sort approach and
 * adjust the `prev` nodes and `list->tail` at the end
 */
void linklist_sort(struct linklist *list, int (*comp)(const void *, const void *))
{
    if (list->size > 1)
    {
        list->head = sort(list->head, comp);
        list->head->prev = NULL;

        struct node *prev = list->head;
        struct node *node = prev->next;
        
        while (node != NULL)
        {
            node->prev = prev;
            prev = node;
            node = node->next;
        }
        list->tail = prev;
    }
}

/* Silence compiler casting non const to const with `(void *)const_var` */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"

void *linklist_search(const linklist *list, const void *data, int (*comp)(const void *, const void *))
{
    struct node *iter;

    for (iter = list->head; iter != NULL; iter = iter->next)
    {
        if (comp(iter->data, data) == 0)
        {
            return (void *)iter->data;
        }
    }
    return NULL;
}

#pragma GCC diagnostic pop

void linklist_destroy(linklist *list, void (*func)(void *))
{
    struct node *node = list->head;

    while (node != NULL)
    {
        if (func != NULL)
        {
            func(node->data);
        }

        struct node *temp = node->next;

        free(node);
        node = temp;
    }
    free(list);
}

