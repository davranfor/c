/*! 
 *  \brief     Dynamic array with exponential growth
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdio.h>
#include <stdlib.h>
#include "garray.h" 

#define GARRAY_MAX_POINTERS 32

struct garray
{
    void *pointer[GARRAY_MAX_POINTERS];
    size_t szof;
    size_t size;
};

static inline unsigned ulog2(size_t n)
{
    unsigned rc = 0;

    while (n >>= 1)
    {
        rc++;
    }
    return rc;
}

garray *garray_create(size_t szof)
{
    garray *array = calloc(1, sizeof *array);

    if (array != NULL)
    {
        array->szof = szof;
    }
    return array;    
}

void *garray_grow(garray *array)
{
    unsigned i = ulog2(array->size + 1);
    unsigned n = 1u << i;
    size_t index = array->size + 1 - n;

    if ((index == 0) && !(array->pointer[i] = malloc(array->szof * n)))
    {
        return NULL;
    }
    array->size++;
    return (unsigned char *)array->pointer[i] + (array->szof * index);
}

void *garray_at(garray *array, size_t index)
{
    if (index >= array->size)
    {
        return NULL;
    }

    unsigned i = ulog2(index + 1);

    index = index + 1 - (1u << i);
    return (unsigned char *)array->pointer[i] + (array->szof * index);
}

size_t garray_size(garray *array)
{
    return array->size;    
}

void garray_destroy(garray *array)
{
    for (size_t i = 0; i < GARRAY_MAX_POINTERS; i++)
    {
        if (array->pointer[i] != NULL)
        {
            free(array->pointer[i]);
        }
        else
        {
            break;
        }
    }
    free(array);
}

