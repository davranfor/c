#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "hashmap.h"

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

static int comp_key(const void *pa, const void *pb)
{
    const struct data *a = pa;
    const struct data *b = pb;

    return a->key < b->key ? -1 : a->key > b->key;
}

static int comp_pkey(const void *pa, const void *pb)
{
    const struct data *a = *(const struct data * const *)pa;
    const struct data *b = *(const struct data * const *)pb;

    return a->key < b->key ? -1 : a->key > b->key;
}

static unsigned long hash_key(const void *item)
{
    const struct data *data = item;

    return hash_ulong((unsigned long)data->key);
}

/* The compare by value version
static int comp_value(const void *pa, const void *pb)
{
    const struct data *a = pa;
    const struct data *b = pb;

    return strcmp(a->value, b->value);
}

static unsigned long hash_value(const void *item)
{
    const struct data *data = item;

    return hash_str((const unsigned char *)data->value);
}
*/

static void delete(void *data)
{
    free(((struct data *)data)->value);
    free(data);
}

static hashmap *map;

static void copy(void)
{
    struct data **data;
    size_t size;

    puts("\nWalking on a copy ...");
    data = hashmap_copy(map, &size);
    if (data == NULL)
    {
        if (size > 0)
        {
            perror("hashmap_copy");
            exit(EXIT_FAILURE);
        }
        return;
    }
    qsort(data, size, sizeof *data, comp_pkey);
    for (size_t i = 0; i < size; i++)
    {
        printf("%d %s\n", data[i]->key, data[i]->value);
    }
    free(data);
}

static void clean(void)
{
    puts("\nDestroying ...");
    hashmap_destroy(map, delete);
}

int main(void)
{
    #define NELEMS 1000000

    atexit(clean);
    srand((unsigned)time(NULL));

    map = hashmap_create(comp_key, hash_key, NELEMS);
    if (map == NULL)
    {
        perror("hashmap_create");
        exit(EXIT_FAILURE);
    }

    struct data *data = calloc(1, sizeof *data);

    if (data == NULL)
    {
        perror("calloc");
        exit(EXIT_FAILURE);
    }

    struct data *item;

    // Insert records
    for (int iter = 0; iter < NELEMS; iter++)
    {
        data->key = rand() % NELEMS;
        item = hashmap_insert(map, data);
        if (item == NULL)
        {
            perror("hashmap_insert");
            exit(EXIT_FAILURE);
        }
        if (data == item)
        {
            data->value = keytostr(data->key);
            data = calloc(1, sizeof *data);
            if (data == NULL)
            {
                perror("calloc");
                exit(EXIT_FAILURE);
            }
        }
    }

    // Search records
    data->key = NELEMS / 2;
    item = hashmap_search(map, data);
    if (item == NULL)
    {
        printf("%d not found\n", data->key);
    }
    else
    {
        printf("%d %s found\n", item->key, item->value);
    }

    // Delete records
    for (data->key = 0; data->key < 10; data->key++)
    {
        item = hashmap_delete(map, data);
        if (item != NULL)
        {
            printf("%d %s deleted\n", item->key, item->value);
            delete(item);
        }
    }

    (void)copy;
    //copy();

    free(data);
    return 0;
}

