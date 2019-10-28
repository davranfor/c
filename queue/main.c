#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "queue.h"

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

static void delete(void *data)
{
    free(((struct data *)data)->value);
    free(data);
}

static void print(queue *list)
{
    struct data *item;
    void *iter = list;

    while ((item = queue_fetch(list, &iter)))
    {
        printf("%d %s\n", item->key, item->value);
    }
}

static queue *list;

static void clean(void)
{
    queue_destroy(list, delete);
}

int main(void)
{
    atexit(clean);
    srand((unsigned)time(NULL));

    list = queue_create();
    if (list == NULL)
    {
        perror("queue_create");
        exit(EXIT_FAILURE);
    }

    size_t size = (size_t)rand() % 10;
    struct data *item;

    for (size_t i = 0; i < size; i++)
    {
        item = queue_push(list, malloc(sizeof *item));
        if (item == NULL)
        {
            perror("queue_push");
            exit(EXIT_FAILURE);
        }
        item->key = (int)i;
        item->value = keytostr(item->key);
        if (item->value == NULL)
        {
            perror("keytostr");
            exit(EXIT_FAILURE);
        }
    }
    print(list);
    printf("%zu elements:\n", queue_size(list));
    while ((item = queue_pop(list)))
    {
        printf("%d %s\n", item->key, item->value);
        delete(item);
    }
    return 0;
}

