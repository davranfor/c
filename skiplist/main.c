#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "skiplist.h"

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

static int comp_key(const void *a, const void *b)
{
    const struct data *pa = a;
    const struct data *pb = b;

    return pa->key < pb->key ? -1 : pa->key > pb->key;
}

static int comp_range(const void *a, const void *b)
{
    const struct data *data = a;
    const int *range = b;

    return data->key < range[0] ? -1 : data->key > range[1];
}

static void walk(skiplist *list)
{
    const void *cursor = NULL;
    const struct data *data;
    int count = 0;
 
    printf("First 5 records:\n");
    while ((data = skiplist_fetch(list, &cursor, NULL, NULL)))
    {
        printf("%d %s fetched\n", data->key, data->value);
        if (++count == 5) {
            return;
        }
    }
}

static void filter(skiplist *list, int min, int max)
{
    const void *cursor = NULL;
    const struct data *data;

    printf("Filtering records from %d to %d\n", min, max);
    while ((data = skiplist_fetch(list, &cursor, (int []){min, max}, comp_range)))
    {
        printf("%d %s fetched\n", data->key, data->value);
    }
}

static void delete(void *data)
{
    free(((struct data *)data)->value);
    free(data);
}

static skiplist *list;

static void clean(void)
{
    skiplist_destroy(list, delete);
}

int main(void)
{
    #define NELEMS 1000000

    atexit(clean);
    srand((unsigned)time(NULL));

    list = skiplist_create(comp_key);
    if (list == NULL)
    {
        perror("skiplist_create");
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
        item = skiplist_insert(list, data);
        if (item == NULL)
        {
            perror("skiplist_insert");
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
    item = skiplist_search(list, data);
    if (item == NULL)
    {
        printf("%d not found\n", data->key);
    }
    else
    {
        printf("%d %s found\n", item->key, item->value);
    }

    // Delete records
    for (data->key = 0; data->key < 10; data->key++)
    {
        item = skiplist_delete(list, data);
        if (item != NULL)
        {
            printf("%d %s deleted\n", item->key, item->value);
            delete(item);
        }
    }

    walk(list);
    filter(list, NELEMS - 6, NELEMS - 1);

    free(data);
    return 0;
}

