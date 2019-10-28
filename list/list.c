/*! 
 *  \brief     Doubly linked list
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdlib.h>
#include "list.h"

struct node
{
    void *data;
    struct node *prev;
    struct node *next;
};

struct linkedlist
{
    struct node *head;
    struct node *tail;
    size_t size;
};

linkedlist *list_create(void)
{
    return calloc(1, sizeof(struct linkedlist));
}

void *list_push_front(linkedlist *list, void *data)
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

void *list_push_back(linkedlist *list, void *data)
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

void *list_pop_front(linkedlist *list)
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
        if (list->head->next == NULL)
        {
            list->tail = list->head;
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

void *list_pop_back(linkedlist *list)
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
        if (list->tail->prev == NULL)
        {
            list->head = list->tail;
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

void *list_prev(linkedlist *list, void **iter)
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

void *list_next(linkedlist *list, void **iter)
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

void *list_head(linkedlist *list)
{
    return list->head;
}

void *list_tail(linkedlist *list)
{
    return list->tail;
}

size_t list_size(const linkedlist *list)
{
    return list->size;
}

static void swap(void **a, void **b)
{
    void *pa;

    pa = *a;
    *a = *b;
    *b = pa;
}

static struct node *partition(struct node *head, struct node *tail, int (*comp)(const void *, const void *))
{
    struct node *prev = head->prev;
    struct node *curr;

    for (curr = head; curr != tail; curr = curr->next)
    {
        if (comp(curr->data, tail->data) <= 0)
        {
            prev = (prev == NULL) ? head : prev->next;
            swap(&(prev->data), &(curr->data));
        }
    }
    prev = (prev == NULL) ? head : prev->next;
    swap(&(prev->data), &(tail->data));
    return prev;
}
 
static void sort(struct node *head, struct node *tail, int (*comp)(const void *, const void *))
{
    struct node *part;

    if ((tail != NULL) && (head != tail) && (head != tail->next))
    {
        part = partition(head, tail, comp);
        sort(head, part->prev, comp);
        sort(part->next, tail, comp);
    }
}

void list_sort(linkedlist *list, int (*comp)(const void *, const void *))
{
    sort(list->head, list->tail, comp);
}

void list_destroy(linkedlist *list, void (*func)(void *))
{
    struct node *node;

    while ((node = list_pop_front(list)))
    {
        if (func != NULL)
        {
            func(node);
        }
    }
    free(list);
}

