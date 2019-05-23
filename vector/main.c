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

    data = vcreate(sizeof *data);
    if (data == NULL) {
        perror("vcreate");
        exit(EXIT_FAILURE);
    }
    n = (size_t)rand() % 10 + 1;
    printf("%zu items\n", n);
    for (i = 0; i < n; i++) {
        if (vadd(&data) == NULL) {
            perror("vadd");
            exit(EXIT_FAILURE);
        }
        data[i].key = rand() % 1000;
        data[i].value = strdup_printf("Item #%zu", i);
    }
    vsort(data, comp_stack);
    for (i = 0; i < vsize(data); i++) {
        printf("%03d %s\n", data[i].key, data[i].value);
    }
    vdestroy(data, free_stack);
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

    data = vcreate(sizeof *data);
    if (data == NULL) {
        perror("vcreate");
        exit(EXIT_FAILURE);
    }
    n = (size_t)rand() % 10 + 1;
    printf("%zu items\n", n);
    for (i = 0; i < n; i++) {
        if (vnew(&data, sizeof **data) == NULL) {
            perror("vnew");
            exit(EXIT_FAILURE);
        }
        data[i]->key = rand() % 1000;
        data[i]->value = strdup_printf("Item #%zu", i);
    }
    vsort(data, comp_heap);
    for (i = 0; i < vsize(data); i++) {
        printf("%03d %s\n", data[i]->key, data[i]->value);
    }
    vdestroy(data, free_heap);
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

    data = vcreate(sizeof *data);
    if (data == NULL) {
        perror("vcreate");
        exit(EXIT_FAILURE);
    }
    n = (size_t)rand() % 10 + 1;
    printf("%zu items\n", n);
    for (i = 0; i < n; i++) {
        r = rand();
        if (vcat(&data, &r) == NULL) {
            perror("vcat");
            exit(EXIT_FAILURE);
        }
    }
    vsort(data, comp_primitive);
    for (i = 0; i < vsize(data); i++) {
        printf("%d\n", data[i]);
    }
    printf("Searching %d\n", r);
    printf("%d\n", *(int *)vsearch(&r, data, comp_primitive));
    vdestroy(data, NULL);
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

