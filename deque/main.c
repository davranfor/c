#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "deque.h"

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

static void print(const deque *list)
{
    const void *iter = list;
    const struct data *data;

    while ((data = deque_next(list, &iter)))
    {
        printf("%d %s\n", data->key, data->value);
    }
}

static void delete(void *data)
{
    free(((struct data *)data)->value);
    free(data);
}

static deque *list;

static void clean(void)
{
    deque_destroy(list, delete);
}

int main(void)
{
    atexit(clean);
    srand((unsigned)time(NULL));

    list = deque_create();
    if (list == NULL)
    {
        perror("list_create");
        exit(EXIT_FAILURE);
    }

    int size = rand() % 10;
    struct data *data;

    for (int key = 0; key < size; key++)
    {
        data = calloc(1, sizeof *data);
        if (data == NULL)
        {
            perror("calloc");
            exit(EXIT_FAILURE);
        }
        if (key & 0x01)
        {
            if (deque_push_head(list, data) == NULL)
            {
                perror("deque_push_head");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            if (deque_push_tail(list, data) == NULL)
            {
                perror("deque_push_tail");
                exit(EXIT_FAILURE);
            }
        }
        data->key = key;
        data->value = keytostr(key);
    }
    printf("%zu elements:\n", deque_size(list));
    puts("Unsorted:");
    print(list);
    deque_sort(list, comp);
    puts("Sorted:");
    print(list);
    deque_reverse(list);
    puts("Reversed:");
    print(list);
    printf("Search item %d:\n", size / 2);
    data = deque_search(list, &(struct data){size / 2, NULL}, comp);
    if (data != NULL)
    {
        printf("Found %d %s\n", data->key, data->value);
    }
    printf("Delete item %zu:\n", deque_size(list) / 2);
    data = deque_delete(list, deque_size(list) / 2);
    if (data != NULL)
    {
        printf("%d %s\n", data->key, data->value);
        delete(data);
    }
    puts("Final:");
    print(list);
    return 0;
}

