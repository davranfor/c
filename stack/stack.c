/*! 
 *  \brief     Stack
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdlib.h>
#include "stack.h"

struct node
{
    void *data;
    struct node *next;
};

struct stack
{
    struct node *head;
    size_t size;
};

stack *stack_create(void)
{
    return calloc(1, sizeof(struct stack));
}

void *stack_push(stack *list, void *data)
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
    node->next = list->head;
    list->head = node;
    list->size++;
    return data;
}

void *stack_pop(stack *list)
{
    void *data = NULL;

    if (list->head != NULL)
    {
        struct node *node = list->head;

        data = node->data;
        list->head = node->next;
        list->size--;
        free(node);
    }
    return data;
}

void *stack_fetch(stack *list, void **iter)
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

void *stack_head(stack *list)
{
    return list->head;
}

size_t stack_size(const stack *list)
{
    return list->size;
}

void stack_destroy(stack *list, void (*func)(void *))
{
    struct node *node = list->head;

    while (node != NULL)
    {
        if (func != NULL)
        {
            func(node->data);
        }

        struct node *next = node;

        next = node->next;
        free(node);
        node = next;
    }
    free(list);
}

