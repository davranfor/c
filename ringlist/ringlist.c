/*! 
 *  \brief     Circular list
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdlib.h>
#include "ringlist.h"

struct node
{
    void *data;
    struct node *next;
};

struct ringlist
{
    struct node *tail;
    size_t size;
};

ringlist *ringlist_create(void)
{
    return calloc(1, sizeof(struct ringlist));
}

void *ringlist_push(ringlist *list, void *data)
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
    if (list->tail == NULL)
    {
        node->next = node;
    }
    else
    {
        node->next = list->tail->next;
        list->tail->next = node;
    }
    list->tail = node;
    list->size++;
    return data;
}

void *ringlist_pop(ringlist *list)
{
    void *data = NULL;

    if (list->tail != NULL)
    {
        struct node *node = list->tail->next;

        list->tail->next = node->next;
        list->size--;
        if (list->size == 0)
        {
            list->tail = NULL;
        }
        data = node->data;
        free(node);
    }
    return data;
}

void *ringlist_fetch(const ringlist *list, const void **iter)
{
    void *data = NULL;

    if (*iter == list)
    {
        if (list->tail != NULL)
        {
            data = list->tail->next->data;
            *iter = list->tail->next;
        }
        else
        {
            *iter = NULL;
        }
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

void *ringlist_head(const ringlist *list)
{
    if (list->tail != NULL)
    {
        return list->tail->next->data;
    }
    return NULL;
}

void *ringlist_tail(const ringlist *list)
{
    if (list->tail != NULL)
    {
        return list->tail->data;
    }
    return NULL;
}

size_t ringlist_size(const ringlist *list)
{
    return list->size;
}

void ringlist_destroy(ringlist *list, void (*func)(void *))
{
    if (list->size > 0)
    {
        struct node *node = list->tail->next;

        list->tail->next = NULL;
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
    }
    free(list);
}

