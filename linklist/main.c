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

    if (str != NULL)
    {
        memcpy(str, buf, len + 1);
    }
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

static void print(linklist *list)
{
    struct data *item;
    void *iter = list;

    while ((item = linklist_next(list, &iter)))
    {
        printf("%d %s\n", item->key, item->value);
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
    struct data *item;

    for (int key = 0; key < size; key++)
    {
        if (key & 0x01)
        {
            item = linklist_push_head(list, malloc(sizeof *item));
        }
        else
        {
            item = linklist_push_tail(list, malloc(sizeof *item));
        }
        if (item == NULL)
        {
            perror("linklist_push");
            exit(EXIT_FAILURE);
        }
        item->key = key;
        item->value = keytostr(key);
        if (item->value == NULL)
        {
            perror("keytostr");
            exit(EXIT_FAILURE);
        }
    }
    printf("%zu elements:\n", linklist_size(list));
    puts("Unsorted:");
    print(list);
    linklist_sort(list, comp);
    puts("Sorted:");
    print(list);
    puts("Search item 2:");
    item = linklist_search(list, &(struct data){2, NULL}, comp);
    if (item != NULL)
    {
        printf("%d %s\n", item->key, item->value);
    }
    printf("Delete item %zu:\n", linklist_size(list) / 2);
    item = linklist_delete(list, linklist_size(list) / 2);
    if (item != NULL)
    {
        printf("%d %s\n", item->key, item->value);
        delete(item);
    }
    puts("Final:");
    print(list);
    return 0;
}

