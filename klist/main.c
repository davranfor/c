#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "klist.h"

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

static void print(const klist *list)
{
    const struct data *data = klist_head(list);

    while (data != NULL)
    {
        printf("%d %s\n", data->key, data->value);
        data = klist_next(data);
    }
}

static void delete(void *data)
{
    free(((struct data *)data)->value);
    klist_free(data);
}

static klist *list;

static void clean(void)
{
    klist_destroy(list, delete);
}

int main(void)
{
    atexit(clean);
    srand((unsigned)time(NULL));

    list = klist_create(sizeof(struct data));
    if (list == NULL)
    {
        perror("klist_create");
        exit(EXIT_FAILURE);
    }

    int size = rand() % 10;
    struct data *data;

    for (int key = 0; key < size; key++)
    {
        if (key & 0x01)
        {
            data = klist_push_head(list);
        }
        else
        {
            data = klist_push_tail(list);
        }
        if (data == NULL)
        {
            perror("klist_push");
            exit(EXIT_FAILURE);
        }
        data->key = key;
        data->value = keytostr(key);
    }
    printf("%zu elements:\n", klist_size(list));
    puts("Unsorted:");
    print(list);
    klist_sort(list, comp);
    puts("Sorted:");
    print(list);
    klist_reverse(list);
    puts("Reversed:");
    print(list);
    printf("Search item %d:\n", size / 2);
    data = klist_search(list, &(struct data){size / 2, NULL}, comp);
    if (data != NULL)
    {
        printf("Found %d %s\n", data->key, data->value);
    }
    while ((data = klist_pop_tail(list)))
    {
        printf("%d %s\n", data->key, data->value);
        delete(data);
    }
    return 0;
}

