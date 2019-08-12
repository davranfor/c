#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "hashmap.h"

#define NELEMS 1000000

struct data {
    int key;
    char *value;
};

static int comp_key(const void *pa, const void *pb)
{
    const struct data *a = pa;
    const struct data *b = pb;

    return a->key - b->key;
}

static unsigned long hash_key(const void *item)
{
    const struct data *data = item;

    return hashmap_hash_ulong((unsigned long)data->key);
}

/* The compare value version
static int comp_value(const void *pa, const void *pb)
{
    const struct data *a = pa;
    const struct data *b = pb;

    return strcmp(a->value, b->value);
}

static unsigned long hash_value(const void *item)
{
    const struct data *data = item;

    return hashmap_hash_string((unsigned char *)data->value);
}
*/

static char *keytostr(int key)
{
    char buf[32];
    size_t len;
    char *str;

    len = (size_t)snprintf(buf, sizeof buf, "(%d)", key);
    str = malloc(len + 1);
    if (str == NULL) {
        return NULL;
    }
    memcpy(str, buf, len + 1);
    return str;
}

static void destroy(void *data)
{
    free(((struct data *)data)->value);
    free(data);
}

static hashmap *map;

static void clean(void)
{
    hashmap_destroy(map, destroy);
}

int main(void)
{
    struct data *data;
    struct data *item;

    atexit(clean);
    srand((unsigned)time(NULL));
    map = hashmap_create(comp_key, hash_key, NELEMS);
    if (map == NULL) {
        perror("hashmap_create");
        exit(EXIT_FAILURE);
    }
    data = malloc(sizeof *data);
    if (data == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    for (int iter = 0; iter < NELEMS; iter++) {
        data->key = rand() % NELEMS;
        item = hashmap_insert(map, data);
        if (item == NULL) {
            perror("hashmap_insert");
            exit(EXIT_FAILURE);
        }
        if (data == item) {
            data->value = keytostr(data->key);
            if (data->value == NULL) {
                perror("keytostr");
                exit(EXIT_FAILURE);
            }
            data = malloc(sizeof *data);
            if (data == NULL) {
                perror("malloc");
                exit(EXIT_FAILURE);
            }
        }
    }
    data->key = NELEMS / 2;
    item = hashmap_search(map, data);
    if (item == NULL) {
        printf("%d not found\n", data->key);
    } else {
        printf("%d %s found\n", item->key, item->value);
    }
    for (int key = 0; key < 10; key++) {
        data->key = key;
        item = hashmap_delete(map, data);
        if (item != NULL) {
            printf("%d %s deleted\n", item->key, item->value);
            free(item->value);
            free(item);
        }
    }
    free(data);
    return 0;
}

