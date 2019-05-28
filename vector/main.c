#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "vector.h"

struct data {
    int key;
    char *value;
};

static int comp_stack(const void *pa, const void *pb)
{
    const struct data *a = pa;
    const struct data *b = pb;

    return a->key - b->key;
}

static void free_stack(void *ptr)
{
    struct data *data = ptr;

    free(data->value);
}

static void demo_stack(void)
{
    struct data *data;
    size_t n, i;

    data = vector_create(sizeof *data);
    if (data == NULL) {
        perror("vector_create");
        exit(EXIT_FAILURE);
    }
    n = (size_t)rand() % 10 + 1;
    printf("%zu items\n", n);
    for (i = 0; i < n; i++) {
        if (vector_add(&data) == NULL) {
            perror("vector_add");
            exit(EXIT_FAILURE);
        }
        data[i].key = rand() % 1000;
        data[i].value = strdup_printf("Item #%zu", i);
    }
    vector_sort(data, comp_stack);
    for (i = 0; i < vector_size(data); i++) {
        printf("%03d %s\n", data[i].key, data[i].value);
    }
    vector_destroy(data, free_stack);
}

static int comp_heap(const void *pa, const void *pb)
{
    const struct data * const *a = pa;
    const struct data * const *b = pb;

    return (*a)->key - (*b)->key;
}

static void free_heap(void *ptr)
{
    struct data **data = ptr;

    free((*data)->value);
    free((*data));
}

static void demo_heap(void)
{
    struct data **data;
    size_t n, i;

    data = vector_create(sizeof *data);
    if (data == NULL) {
        perror("vector_create");
        exit(EXIT_FAILURE);
    }
    n = (size_t)rand() % 10 + 1;
    printf("%zu items\n", n);
    for (i = 0; i < n; i++) {
        if (vector_new(&data, sizeof **data) == NULL) {
            perror("vector_new");
            exit(EXIT_FAILURE);
        }
        data[i]->key = rand() % 1000;
        data[i]->value = strdup_printf("Item #%zu", i);
    }
    vector_sort(data, comp_heap);
    for (i = 0; i < vector_size(data); i++) {
        printf("%03d %s\n", data[i]->key, data[i]->value);
    }
    vector_destroy(data, free_heap);
}

static int comp_primitive(const void *pa, const void *pb)
{
    const int *a = pa;
    const int *b = pb;

    return *a - *b;
}

static void demo_primitive(void)
{
    int *data, r;
    size_t n, i;

    data = vector_create(sizeof *data);
    if (data == NULL) {
        perror("vector_create");
        exit(EXIT_FAILURE);
    }
    n = (size_t)rand() % 10 + 1;
    printf("%zu items\n", n);
    for (i = 0; i < n; i++) {
        r = rand();
        if (vector_cat(&data, &r) == NULL) {
            perror("vector_cat");
            exit(EXIT_FAILURE);
        }
    }
    vector_sort(data, comp_primitive);
    for (i = 0; i < vector_size(data); i++) {
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

