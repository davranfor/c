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

static dynarray *array;

static void clean(void)
{
    puts("\nDestroying ...");
    dynarray_destroy(array, delete);
}

int main(void)
{
    enum {N = 100};

    atexit(clean);
    srand((unsigned)time(NULL));

    array = dynarray_create();
    if (array == NULL)
    {
        perror("dynarray_create");
        exit(EXIT_FAILURE);
    }

    struct data *data;

    for (int iter = 0; iter < N; iter++)
    {
        data = calloc(1, sizeof *data);
        if (data == NULL)
        {
            perror("calloc");
            exit(EXIT_FAILURE);
        }
        if (iter % 2)
        {
            if (dynarray_push(array, data) == NULL)
            {
                perror("dynarray_push");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            if (dynarray_insert(array, 0, data) == NULL)
            {
                perror("dynarray_insert");
                exit(EXIT_FAILURE);
            }
        }
        data->key = iter;
        data->value = keytostr(data->key);
    }
    dynarray_sort(array, comp);
    data = &(struct data){.key = N / 2};
    data = dynarray_bsearch(array, data, comp);
    if (data != NULL)
    {
        printf("Found: %d %s\n", data->key, data->value);
    }
    data = dynarray_delete(array, N / 2);
    if (data != NULL)
    {
        printf("Deleting: %d %s\n", data->key, data->value);
        delete(data);
    }
    data = dynarray_pop(array);
    if (data != NULL)
    {
        printf("Deleting: %d %s\n", data->key, data->value);
        delete(data);
    }
    if (dynarray_refresh(array) == NULL)
    {
        perror("dynarray_refresh");
        exit(EXIT_FAILURE);
    }
    dynarray_reverse(array);

    size_t size = dynarray_size(array);

    for (size_t iter = 0; iter < size; iter++)
    {
        data = dynarray_get(array, iter);
        printf("%d %s\n", data->key, data->value);
    }
    return 0;
}

