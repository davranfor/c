#include <stdio.h>
#include <stdlib.h>
#include "garray.h"

int main(void)
{
    garray *array = garray_create(sizeof(int));

    if (array == NULL)
    {
        perror("garray_create");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < 64; i++)
    {
        int * const ptr = garray_grow(array);

        if (ptr == NULL)
        {
            perror("garray_grow");
            exit(EXIT_FAILURE);
        }
        *ptr = i;
    }
    for (size_t i = 0, n = garray_size(array); i < n; i++)
    {
        printf("%d\n", *(int *)garray_at(array, i));
    }
    garray_destroy(array);
    return 0;
}

