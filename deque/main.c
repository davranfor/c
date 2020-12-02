#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "deque.h"

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

static void print(const deque *list)
{
    const void *iter = list;
    const struct data *data;

    while ((data = deque_fetch(list, &iter)))
    {
        printf("%d %s\n", data->key, data->value);
    }
}

static deque *list;

static void clean(void)
{
    deque_destroy(list, delete);
}

int main(void)
{
    atexit(clean);
    srand((unsigned)time(NULL));

    list = deque_create();
    if (list == NULL)
    {
        perror("deque_create");
        exit(EXIT_FAILURE);
    }

    int size = rand() % 10;
    struct data *data;

    for (int key = 0; key < size; key++)
    {
        data = malloc(sizeof *data);
        if (data == NULL)
        {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        if (key & 0x01)
        {
            if (deque_push_head(list, data) == NULL)
            {
                perror("deque_push_head");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            if (deque_push_tail(list, data) == NULL)
            {
                perror("deque_push_tail");
                exit(EXIT_FAILURE);
            }
        }
        data->key = key;
        data->value = keytostr(key);
    }
    print(list);
    printf("%zu elements:\n", deque_size(list));
    while ((data = deque_pop(list)))
    {
        printf("%d %s\n", data->key, data->value);
        delete(data);
    }
    return 0;
}

