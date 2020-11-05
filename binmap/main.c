#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "binmap.h"

static binmap *map;

static void clean(void)
{
    binmap_destroy(map);
}

int main(void)
{
    atexit(clean);
    srand((unsigned)time(NULL));

    size_t size = (size_t)(rand() % 12) + 1;

    map = binmap_create(size);
    if (map == NULL)
    {
        perror("binmap_create");
        exit(EXIT_FAILURE);
    }
    size *= 10;
    for (size_t iter = 0; iter < size; iter++)
    {
        int value = rand() % 2;

        if (binmap_set(map, iter, value) == 0)
        {
            perror("binmap_set");
            exit(EXIT_FAILURE);
        }
        printf("%d", value);
    }
    printf("\n");
    for (size_t iter = 0; iter < size; iter++)
    {
        int value = binmap_get(map, iter);

        printf("%d", value);
    }
    printf("\n");
    return 0;
}

