/*! 
 *  \brief     SkipList
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdlib.h>
#include "skiplist.h"

#define SKIPLIST_MAX_LEVEL 32

struct node
{
    void *data;
    struct node *next[];
};

struct skiplist
{
    struct node *head;
    int (*comp)(const void *, const void *);
    int levels;
};

static int skiplist_level(void)
{
    unsigned long bits = (unsigned long)rand();
    unsigned long level = 0;

    while ((level < SKIPLIST_MAX_LEVEL - 1) && (bits & (1UL << level)))
    {
        level++;
    }
    return (int)level;
}

static struct node *skiplist_create_node(int levels)
{
    struct node *node;

    node = calloc(1, sizeof(*node) + sizeof(*node->next) * (size_t)levels);
    return node;
}

skiplist *skiplist_create(int (*comp)(const void *, const void *))
{
    skiplist *list = malloc(sizeof(*list));

    if (list != NULL)
    {
        list->head = skiplist_create_node(SKIPLIST_MAX_LEVEL);
        if (list->head == NULL)
        {
            free(list);
            return NULL;
        }
        for (int level = 0; level < SKIPLIST_MAX_LEVEL; level++)
        {
            list->head->next[level] = list->head;
        }
        list->comp = comp;
        list->levels = 0;
    }
    return list;
}

void *skiplist_insert(skiplist *list, void *data)
{
    struct node *nodes[SKIPLIST_MAX_LEVEL];
    struct node *node = list->head;
    int level;

    for (level = list->levels; level >= 0; level--)
    {
        while (node->next[level] != list->head)
        {
            int comp = list->comp(node->next[level]->data, data);

            if (comp > 0)
            {
                break;
            }
            if (comp < 0)
            {
                node = node->next[level];
            }
            else
            {
                return node->next[level]->data;
            }
        }
        nodes[level] = node;
    }
    level = skiplist_level();
    node = skiplist_create_node(level + 1);
    if (node == NULL)
    {
        return NULL;
    }
    while (list->levels < level)
    {
        nodes[++list->levels] = list->head;
    }
    while (level >= 0)
    {
        node->next[level] = nodes[level]->next[level];
        nodes[level]->next[level] = node;
        level--;
    }
    node->data = data;
    return data;
}

void *skiplist_delete(skiplist *list, const void *data)
{
    struct node *node = list->head;
    struct node *item = NULL;

    for (int level = list->levels; level >= 0; level--)
    {
        while (node->next[level] != list->head)
        {
            int comp = list->comp(node->next[level]->data, data);

            if (comp > 0)
            {
                break;
            }
            if (comp == 0)
            {
                if (item == NULL)
                {
                    item = node->next[0];
                }
                node->next[level] = item->next[level];
                break;
            }
            node = node->next[level];
        }
    }
    if (item != NULL)
    {
        while ((list->levels > 0) && (list->head->next[list->levels] == list->head))
        {
            list->levels--;
        }

        void *res = item->data;

        free(item);
        return res;
    }
    return NULL;
}

void *skiplist_search(const skiplist *list, const void *data)
{
    const struct node *node = list->head;

    for (int level = list->levels; level >= 0; level--)
    {
        while (node->next[level] != list->head)
        {
            int comp = list->comp(node->next[level]->data, data);

            if (comp > 0)
            {
                break;
            }
            if (comp < 0)
            {
                node = node->next[level];
            }
            else
            {
                return node->next[level]->data;
            }
        }
    }
    return NULL;
}

static struct node *fetch(const skiplist *list, const void *data,
    int (*comp)(const void *, const void *))
{
    struct node *node = list->head;

    for (int level = list->levels; level >= 0; level--)
    {
        while (node->next[level] != list->head)
        {
            if (comp(node->next[level]->data, data) >= 0)
            {
                break;
            }
            node = node->next[level];
        }
    }
    return node;
}

void *skiplist_fetch(const skiplist *list, const void **cursor, const void *data,
    int (*comp)(const void *, const void *))
{
    const struct node *node;

    if (*cursor == NULL)
    {
        if ((comp != NULL) && (data != NULL))
        {
            node = fetch(list, data, comp);
            comp = NULL;
        }
        else
        {
            node = list->head;
        }
    }
    else
    {
        node = *cursor;
    }
    *cursor = node = node->next[0];
    if ((comp != NULL) &&
        (data != NULL) &&
        (node->data != NULL) &&
        (comp(node->data, data) != 0))
    {
        return NULL;
    }
    return node->data;
}

void skiplist_destroy(skiplist *list, void (*func)(void *))
{
    if (list != NULL)
    {
        struct node *node;

        node = list->head->next[0];
        while (node != list->head)
        {
            struct node *temp;

            if (func != NULL)
            {
                func(node->data);
            }
            temp = node;
            node = node->next[0];
            free(temp);
        }
        free(list->head);
        free(list);
    }
}

