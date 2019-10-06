#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "vector.h"

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
}

static void print(struct data *data)
{
    size_t size = vector_size(data);

    for (size_t i = 0; i < size; i++)
    {
        printf("%d %s\n", data[i].key, data[i].value);
    }
}

static struct data *data;

static void clean(void)
{
    vector_destroy(data);
}

int main(void)
{
    atexit(clean);
    srand((unsigned)time(NULL));

    data = vector_create(sizeof *data, delete);
    if (data == NULL)
    {
        perror("vector_create");
        exit(EXIT_FAILURE);
    }

    size_t size = (size_t)rand() % 10;
    struct data *item;

    for (size_t i = 0; i < size; i++)
    {
        item = vector_resize(&data, +1);
        if (item == NULL)
        {
            perror("vector_resize");
            exit(EXIT_FAILURE);
        }
        item->key = rand() % 10;
        item->value = keytostr(item->key);
        if (item->value == NULL)
        {
            perror("keytostr");
            exit(EXIT_FAILURE);
        }
    }
    item = (struct data[]){{10, keytostr(10)}, {11, keytostr(11)}};
    if ((item[0].value == NULL) || (item[1].value == NULL))
    {
        perror("keytostr");
        exit(EXIT_FAILURE);
    }
    if (vector_concat(&data, item, 2) == NULL)
    {
        perror("vector_concat");
        exit(EXIT_FAILURE);
    }
    printf("Inserted: %zu elements\n", vector_size(data));
    puts("Unsorted:");
    print(data);
    vector_sort(data, comp);
    puts("Sorted:");
    print(data);
    item = &((struct data){.key = 5});
    printf("Searching %d\n", item->key);
    if ((item = vector_bsearch(item, data, comp)))
    {
        printf("%d %s\n", item->key, item->value);
    }
    else
    {
        puts("Not found");
    }
    puts("Deleting last element");
    if (vector_resize(&data, -1) == NULL)
    {
        perror("vector_resize");
        exit(EXIT_FAILURE);
    }
    print(data);
    return 0;
}

