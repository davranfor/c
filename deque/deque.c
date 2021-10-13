/*! 
 *  \brief     Double-ended queue
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdlib.h>
#include "deque.h"

struct node
{
    void *data;
    struct node *prev;
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
    return calloc(1, sizeof(deque));
}

void *deque_push_head(deque *list, void *data)
{
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

void *deque_push_tail(deque *list, void *data)
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

void *deque_pop_head(deque *list)
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

void *deque_pop_tail(deque *list)
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

static struct node *deque_node(const deque *list, size_t index)
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

void *deque_insert(deque *list, size_t index, void *data)
{
    if (index > list->size)
    {
        return NULL;
    }
    if (index == 0)
    {
        return deque_push_head(list, data);
    }
    if (index == list->size)
    {
        return deque_push_tail(list, data);
    }

    struct node *node = malloc(sizeof *node);

    if (node == NULL)
    {
        return NULL;
    }
        
    struct node *curr = deque_node(list, index);

    curr->prev->next = node;
    node->prev = curr->prev;
    curr->prev = node;
    node->next = curr;
    node->data = data;
    list->size++;
    return data;
}

void *deque_delete(deque *list, size_t index)
{
    if (index >= list->size)
    {
        return NULL;
    }
    if (index == 0)
    {
        return deque_pop_head(list);
    }
    if (index == list->size - 1)
    {
        return deque_pop_tail(list);
    }

    struct node *node = deque_node(list, index);

    node->prev->next = node->next;
    node->next->prev = node->prev;
    list->size--;

    void *data = node->data;

    free(node);
    return data;
}

void *deque_index(const deque *list, size_t index)
{
    if (index >= list->size)
    {
        return NULL;
    }
    return deque_node(list, index)->data;
}

void *deque_head(const deque *list)
{
    if (list->head != NULL)
    {
        return list->head->data;
    }
    return NULL;
}

void *deque_prev(const deque *list, const void **iter)
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
        const struct node *node = *iter;

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

void *deque_next(const deque *list, const void **iter)
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

void *deque_tail(const deque *list)
{
    if (list->tail != NULL)
    {
        return list->tail->data;
    }
    return NULL;
}

size_t deque_size(const deque *list)
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

static struct node *merge(struct node *head, struct node *tail,
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
        if (comp(head->data, prev->data) > 0)
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
            else if (comp(curr->next->data, prev->data) <= 0)
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

static struct node *sort(struct node *head,
    int (*comp)(const void *, const void *))
{
    if ((head == NULL) || (head->next == NULL))
    {
        return head;
    }

    struct node *tail = split(head);

    head = sort(head, comp);
    tail = sort(tail, comp);
    return merge(head, tail, comp);
}

/** 
 * Merge sort:
 * Since we need to update the tail, we use a singly linked list sort approach and
 * adjust the `prev` nodes and `list->tail` at the end
 */
void deque_sort(deque *list, int (*comp)(const void *, const void *))
{
    if (list->size > 1)
    {
        list->head = sort(list->head, comp);
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

void *deque_search(const deque *list, const void *data,
    int (*comp)(const void *, const void *))
{
    const struct node *iter;

    for (iter = list->head; iter != NULL; iter = iter->next)
    {
        if (comp(iter->data, data) == 0)
        {
            return iter->data;
        }
    }
    return NULL;
}

void deque_reverse(const deque *list)
{
    struct node *a = list->head;
    struct node *b = list->tail;
    size_t mid = list->size / 2;

    for (size_t iter = 0; iter < mid; iter++)
    {
        void *temp = a->data;

        a->data = b->data;
        b->data = temp;
        a = a->next;
        b = b->prev;
    }
}

void deque_clear(deque *list, void (*func)(void *))
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
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

void deque_destroy(deque *list, void (*func)(void *))
{
    if (list != NULL)
    {
        deque_clear(list, func);
        free(list);
    }
}

