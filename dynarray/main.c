#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "dynarray.h"

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

static int comp_key(const void *pa, const void *pb)
{
    const struct data * const *a = pa;
    const struct data * const *b = pb;

    return (*a)->key < (*b)->key ? -1 : (*a)->key > (*b)->key;
}

static void destroy(void *data)
{
    free(((struct data *)data)->value);
    free(data);
}

static dynarray *array;

static void clean(void)
{
    puts("\nDestroying ...");
    dynarray_destroy(array, destroy);
}

int main(void)
{
    #define NELEMS 1000000

    atexit(clean);
    srand((unsigned)time(NULL));

    array = dynarray_create(0);
    if (array == NULL)
    {
        perror("dynarray_create");
        exit(EXIT_FAILURE);
    }

    struct data *data;

    for (int iter = 0; iter < NELEMS; iter++)
    {
        data = dynarray_add(array, malloc(sizeof *data));
        if (data == NULL)
        {
            perror("dynarray_add");
            exit(EXIT_FAILURE);
        }
        data->key = rand() % NELEMS;
        data->value = keytostr(data->key);
        if (data->value == NULL)
        {
            perror("keytostr");
            exit(EXIT_FAILURE);
        }
    }
    dynarray_sort(array, comp_key);
    data = &(struct data){.key = NELEMS / 2};
    data = dynarray_bsearch(array, &data, comp_key);
    if (data != NULL)
    {
        printf("Found: %d %s\n", data->key, data->value);
    }
    if (dynarray_resize(array, 10, destroy) == NULL)
    {
        perror("dynarray_resize");
        exit(EXIT_FAILURE);
    }

    size_t size = dynarray_size(array);

    for (size_t iter = 0; iter < size; iter++)
    {
        data = dynarray_get(array, iter);
        printf("%d %s\n", data->key, data->value);
    }
    return 0;
}

