/*! 
 *  \brief     SkipList
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdlib.h>
#include "skiplist.h"

#define SKIPLIST_MAX_LEVEL 32

struct node {
    void *data;
    struct node *next[];
};

struct skiplist {
    struct node *head;
    int (*comp)(const void *, const void *);
    int levels;
};

static int skiplist_level(void)
{
    unsigned long bits = (unsigned long)rand();
    unsigned long level = 0;

    while ((level < SKIPLIST_MAX_LEVEL - 1) && (bits & (1UL << level))) {
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
    skiplist *list;
    int level;

    list = malloc(sizeof(*list));
    if (list != NULL) {
        list->head = skiplist_create_node(SKIPLIST_MAX_LEVEL);
        if (list->head == NULL) {
            return NULL;
        }
        for (level = 0; level < SKIPLIST_MAX_LEVEL; level++) {
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
    int level, comp;

    for (level = list->levels; level >= 0; level--) {
        while (node->next[level] != list->head) {
            comp = list->comp(node->next[level]->data, data);
            if (comp > 0) {
                break;   
            }
            if (comp == 0) {
                return node->next[level]->data;
            }
            node = node->next[level];
        }
        nodes[level] = node;
    }
    level = skiplist_level();
    node = skiplist_create_node(level + 1);
    if (node == NULL) {
        return NULL;
    }
    while (list->levels < level) {
        nodes[++list->levels] = list->head;
    }
    while (level >= 0) {
        node->next[level] = nodes[level]->next[level];
        nodes[level]->next[level] = node;
        level--;
    }
    node->data = data;
    return data;
}

void *skiplist_delete(skiplist *list, const void *data)
{
    struct node *nodes[SKIPLIST_MAX_LEVEL];
    struct node *node = list->head;
    int level;
    void *res;

    for (level = list->levels; level >= 0; level--) {
        while ((node->next[level] != list->head)
            && (list->comp(node->next[level]->data, data) < 0)) {
            node = node->next[level];
        }
        nodes[level] = node;
    }
    node = node->next[0];
    if ((node == list->head) || (list->comp(node->data, data) != 0)) {
        return NULL;
    }
    for (level = 0; level <= list->levels; level++) {
        if (nodes[level]->next[level] != node) {
            break;
        }
        nodes[level]->next[level] = node->next[level];
    }
    while ((list->levels > 0) && (list->head->next[list->levels] == list->head)) {
        list->levels--;
    }
    res = node->data;
    free(node);
    return res;
}

void *skiplist_search(skiplist *list, const void *data)
{
    struct node *node = list->head;
    int level, comp;

    for (level = list->levels; level >= 0; level--) {
        while (node->next[level] != list->head) {
            comp = list->comp(node->next[level]->data, data);
            if (comp > 0) {
                break;   
            }
            if (comp == 0) {
                return node->next[level]->data;
            }
            node = node->next[level];
        }
    }
    return NULL;
}

void *skiplist_fetch(skiplist *list, struct cursor *cursor)
{
    struct node *node;
    int level;

    if (cursor->node == NULL) {
        node = list->head;
        if ((cursor->comp != NULL) && (cursor->data != NULL)) {
            for (level = list->levels; level >= 0; level--) {
                while ((node->next[level] != list->head)
                    && (cursor->comp(node->next[level]->data, cursor->data) < 0)) {
                    node = node->next[level];
                }
            }
        }
    } else {
        node = cursor->node;
    }
    node = cursor->node = node->next[0];
    if ((cursor->comp != NULL) &&
        (cursor->data != NULL) && (node->data != NULL) &&
        (cursor->comp(node->data, cursor->data) != 0)) {
        return NULL;
    }
    return node->data;
}

void skiplist_destroy(skiplist *list, void (*func)(void *))
{
    struct node *node;
    struct node *next;

    node = list->head->next[0];
    while (node != list->head) {
        if (func != NULL) {
            func(node->data);
        }
        next = node->next[0];
        free(node);
        node = next;
    }
    free(list->head);
    free(list);
}
