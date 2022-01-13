#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "rbtree.h"

struct data
{
    int key;
    char *value;
};

static char *keytostr(int key)
{
    char buf[32];
    size_t len;

    len = (size_t)snprintf(buf, sizeof buf, "(%d)", key);

    char *str = malloc(len + 1);

    if (str == NULL)
    {
        perror("keytostr");
        exit(EXIT_FAILURE);
    }
    memcpy(str, buf, len + 1);
    return str;
}

static int compare(const void *a, const void *b)
{
    const struct data *pa = a;
    const struct data *pb = b;

    return pa->key < pb->key ? -1 : pa->key > pb->key;
}

static void *print(void *item, void *cookie)
{
    struct data *data = item;
    int max = *(int *)cookie;

    if (data->key < max)
    {
        printf("%d %s fetched\n", data->key, data->value);
        return NULL;
    }
	return data;
}

static void delete(void *data)
{
    free(((struct data *)data)->value);
    free(data);
}

static rbtree *tree;

static void clean(void)
{
    rbtree_destroy(tree, delete);
}

int main(void)
{
    #define NELEMS 1000000

    atexit(clean);
    srand((unsigned)time(NULL));

    tree = rbtree_create(compare);
    if (tree == NULL)
    {
        perror("rbtree_create");
        exit(EXIT_FAILURE);
    }

    struct data *data = calloc(1, sizeof *data);

    if (data == NULL)
    {
        perror("calloc");
        exit(EXIT_FAILURE);
    }

    struct data *item;

    // Insert records
    for (int iter = 0; iter < NELEMS; iter++)
    {
        data->key = rand() % NELEMS;
        item = rbtree_insert(tree, data);
        if (item == NULL)
        {
            perror("rbtree_insert");
            exit(EXIT_FAILURE);
        }
        if (data == item)
        {
            data->value = keytostr(data->key);
            data = calloc(1, sizeof *data);
            if (data == NULL)
            {
                perror("calloc");
                exit(EXIT_FAILURE);
            }
        }
    }

    // Search records
    data->key = NELEMS / 2;
    item = rbtree_search(tree, data);
    if (item == NULL)
    {
        printf("%d not found\n", data->key);
    }
    else
    {
        printf("%d %s found\n", item->key, item->value);
    }

    // Print first 10 records
    rbtree_walk(tree, print, (int []){10});

    // Delete first 10 records
    for (data->key = 0; data->key < 10; data->key++)
    {
        item = rbtree_delete(tree, data);
        if (item != NULL)
        {
            printf("%d %s deleted\n", item->key, item->value);
            delete(item);
        }
    }
    free(data);
    return 0;
}

