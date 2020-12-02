#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "linklist.h"

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

static int comp(const void *pa, const void *pb)
{
    const struct data *a = pa;
    const struct data *b = pb;

    return a->key < b->key ? -1 : a->key > b->key;
}

static void delete(void *data)
{
    free(((struct data *)data)->value);
    free(data);
}

static void print(const linklist *list)
{
    const void *iter = list;
    const struct data *data;

    while ((data = linklist_next(list, &iter)))
    {
        printf("%d %s\n", data->key, data->value);
    }
}

static linklist *list;

static void clean(void)
{
    linklist_destroy(list, delete);
}

int main(void)
{
    atexit(clean);
    srand((unsigned)time(NULL));

    list = linklist_create();
    if (list == NULL)
    {
        perror("list_create");
        exit(EXIT_FAILURE);
    }

    int size = rand() % 10;
    struct data *data;

    for (int key = 0; key < size; key++)
    {
        data = malloc(sizeof *data);
        if (data == NULL)
        {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        if (key & 0x01)
        {
            data = linklist_push_head(list, data);
        }
        else
        {
            data = linklist_push_tail(list, data);
        }
        if (data == NULL)
        {
            perror("linklist_push");
            exit(EXIT_FAILURE);
        }
        data->key = key;
        data->value = keytostr(key);
    }
    printf("%zu elements:\n", linklist_size(list));
    puts("Unsorted:");
    print(list);
    linklist_sort(list, comp);
    puts("Sorted:");
    print(list);
    linklist_reverse(list);
    puts("Reversed:");
    print(list);
    puts("Search item 2:");
    data = linklist_search(list, &(struct data){2, NULL}, comp);
    if (data != NULL)
    {
        printf("%d %s\n", data->key, data->value);
    }
    printf("Delete item %zu:\n", linklist_size(list) / 2);
    data = linklist_delete(list, linklist_size(list) / 2);
    if (data != NULL)
    {
        printf("%d %s\n", data->key, data->value);
        delete(data);
    }
    puts("Final:");
    print(list);
    return 0;
}

