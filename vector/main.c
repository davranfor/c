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

static void destroy(void *data)
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

int main(void)
{
    srand((unsigned)time(NULL));

    struct data *data = vector_create(sizeof *data);

    if (data == NULL)
    {
        perror("vector_create");
        exit(EXIT_FAILURE);
    }

    size_t size = (size_t)rand() % 10;
    struct data *item;

    for (size_t i = 0; i < size; i++)
    {
        item = vector_resize(&data);
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
    if (item == NULL)
    {
        perror("keytostr");
        exit(EXIT_FAILURE);
    }
    if (vector_copy(&data, item, 2) == NULL)
    {
        perror("vector_copy");
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
    if ((item = vector_search(item, data, comp)))
    {
        printf("%d %s\n", item->key, item->value);
    }
    else
    {
        puts("Not found");
    }
    if (vector_shrink(&data, destroy) == NULL)
    {
        perror("vector_shrink");
        exit(EXIT_FAILURE);
    }
    puts("Last element deleted");
    print(data);
    vector_destroy(data, destroy);
    return 0;
}

