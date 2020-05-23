/*! 
 *  \brief     deque (Double-ended queue)
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdlib.h>
#include "deque.h"

struct node
{
    void *data;
    struct node *next;
};

struct deque
{
    struct node *head;
    struct node *tail;
    size_t size;
};

deque *deque_create(void)
{
    return calloc(1, sizeof(struct deque));
}

void *deque_push_head(deque *list, void *data)
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
    if (list->head == NULL)
    {
        list->tail = node;
        node->next = NULL;
    }
    else
    {
        node->next = list->head;
    }
    list->head = node;
    list->size++;
    return data;
}

void *deque_push_tail(deque *list, void *data)
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
    }
    else
    {
        list->tail->next = node;
    }
    list->tail = node;
    list->size++;
    return data;
}

void *deque_pop(deque *list)
{
    void *data = NULL;

    if (list->head != NULL)
    {
        struct node *node = list->head;

        data = node->data;
        list->head = node->next;
        if ((list->head == NULL) || (list->head->next == NULL))
        {
            list->tail = list->head;
        }
        list->size--;
        free(node);
    }
    return data;
}

void *deque_fetch(deque *list, void **iter)
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

void *deque_head(deque *list)
{
    return list->head;
}

void *deque_tail(deque *list)
{
    return list->tail;
}

size_t deque_size(const deque *list)
{
    return list->size;
}

void deque_destroy(deque *list, void (*func)(void *))
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

