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

static int comp_stack(const void *pa, const void *pb)
{
    const struct data *a = pa;
    const struct data *b = pb;

    return a->key - b->key;
}

static void destroy_stack(void *ptr)
{
    struct data *data = ptr;

    free(data->value);
}

static void demo_stack(void)
{
    struct data *data = vector_create(sizeof *data);

    if (data == NULL)
    {
        perror("vector_create");
        exit(EXIT_FAILURE);
    }

    size_t size = (size_t)rand() % 10 + 1;

    printf("%zu items\n", size);
    for (size_t i = 0; i < size; i++)
    {
        if (vector_add(&data) == NULL)
        {
            perror("vector_add");
            exit(EXIT_FAILURE);
        }
        data[i].key = rand() % 1000;
        data[i].value = keytostr(data[i].key);
    }
    vector_sort(data, comp_stack);
    size = vector_size(data);
    for (size_t i = 0; i < size; i++)
    {
        printf("%03d %s\n", data[i].key, data[i].value);
    }
    vector_destroy(data, destroy_stack);
}

static int comp_heap(const void *pa, const void *pb)
{
    const struct data * const *a = pa;
    const struct data * const *b = pb;

    return (*a)->key - (*b)->key;
}

static void destroy_heap(void *ptr)
{
    struct data **data = ptr;

    free((*data)->value);
    free((*data));
}

static void demo_heap(void)
{
    struct data **data = vector_create(sizeof *data);

    if (data == NULL)
    {
        perror("vector_create");
        exit(EXIT_FAILURE);
    }

    size_t size = (size_t)rand() % 10 + 1;

    printf("%zu items\n", size);
    for (size_t i = 0; i < size; i++)
    {
        if (vector_new(&data, sizeof **data) == NULL)
        {
            perror("vector_new");
            exit(EXIT_FAILURE);
        }
        data[i]->key = rand() % 1000;
        data[i]->value = keytostr(data[i]->key);
    }
    vector_sort(data, comp_heap);
    size = vector_size(data);
    for (size_t i = 0; i < size; i++)
    {
        printf("%03d %s\n", data[i]->key, data[i]->value);
    }
    vector_destroy(data, destroy_heap);
}

static int comp_primitive(const void *pa, const void *pb)
{
    const int *a = pa;
    const int *b = pb;

    return *a - *b;
}

static void demo_primitive(void)
{
    int *data = vector_create(sizeof *data);

    if (data == NULL)
    {
        perror("vector_create");
        exit(EXIT_FAILURE);
    }

    size_t size = (size_t)rand() % 10 + 1;
    int r = 0;

    printf("%zu items\n", size);
    for (size_t i = 0; i < size; i++)
    {
        r = rand();
        if (vector_cat(&data, &r) == NULL)
        {
            perror("vector_cat");
            exit(EXIT_FAILURE);
        }
    }
    vector_sort(data, comp_primitive);
    size = vector_size(data);
    for (size_t i = 0; i < size; i++)
    {
        printf("%d\n", data[i]);
    }
    printf("Searching %d\n", r);
    printf("%d\n", *(int *)vector_search(&r, data, comp_primitive));
    vector_destroy(data, NULL);
}

int main(void)
{
    srand((unsigned)time(NULL));
    puts("Sample on stack:");
    demo_stack();
    puts("Sample on heap:");
    demo_heap();
    puts("Sample on primitive:");
    demo_primitive();
    return 0;
}

