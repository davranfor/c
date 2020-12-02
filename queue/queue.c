/*! 
 *  \brief     Queue
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdlib.h>
#include "queue.h"

struct node
{
    void *data;
    struct node *next;
};

struct queue
{
    struct node *head;
    struct node *tail;
    size_t size;
};

queue *queue_create(void)
{
    return calloc(1, sizeof(struct queue));
}

void *queue_push(queue *list, void *data)
{
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

void *queue_pop(queue *list)
{
    void *data = NULL;

    if (list->head != NULL)
    {
        struct node *node = list->head;

        data = node->data;
        list->head = node->next;
        if (--list->size <= 1)
        {
            list->tail = list->head;
        }
        free(node);
    }
    return data;
}

void *queue_fetch(const queue *list, const void **iter)
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
        const struct node *node = *iter;

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

void *queue_head(const queue *list)
{
    if (list->head != NULL)
    {
        return list->head->data;
    }
    return NULL;
}

void *queue_tail(const queue *list)
{
    if (list->tail != NULL)
    {
        return list->tail->data;
    }
    return NULL;
}

size_t queue_size(const queue *list)
{
    return list->size;
}

void queue_destroy(queue *list, void (*func)(void *))
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

