#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "knode.h"

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

static void delete(void *data)
{
    free(((struct data *)data)->value);
    free(data);
}

static void print(const knode *list)
{
    const void *iter = list;
    const struct data *data;

    while ((data = knode_fetch(list, &iter, data)))
    {
        printf("%d %s\n", data->key, data->value);
    }
}

static knode *list;

static void clean(void)
{
    knode_destroy(list, delete);
}

int main(void)
{
    atexit(clean);
    srand((unsigned)time(NULL));

    list = knode_create(sizeof(struct data));
    if (list == NULL)
    {
        perror("knode_create");
        exit(EXIT_FAILURE);
    }

    int size = rand() % 10;
    struct data *data;

    for (int key = 0; key < size; key++)
    {
        if (key & 0x01)
        {
            data = knode_push_head(list);
        }
        else
        {
            data = knode_push_tail(list);
        }
        if (data == NULL)
        {
            perror("knode_push");
            exit(EXIT_FAILURE);
        }
        data->key = key;
        data->value = keytostr(key);
    }
    printf("%zu elements:\n", knode_size(list));
    print(list);
    printf("Select index %d:\n", size / 2);
    data = knode_index(list, (size_t)(size / 2));
    if (data != NULL)
    {
        printf("Found %d %s\n", data->key, data->value);
    }
    puts("Deleting from tail ...");
    while ((data = knode_pop_tail(list)))
    {
        printf("%d %s\n", data->key, data->value);
        delete(data);
    }
    return 0;
}

