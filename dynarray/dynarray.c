/*! 
 *  \brief     Dynamic array
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdlib.h>
#include "dynarray.h"

struct dynarray
{
    void **data;
    size_t room;
    size_t size;
};

/* Compute next power of 2 for a given size */
static size_t next_pow2(size_t size)
{
    size--;
    size |= size >> 1;
    size |= size >> 2;
    size |= size >> 4;
    size |= size >> 8;
    size |= size >> 16;
    size |= size >> 32;
    size++;
    return size;
}

static size_t next_size(size_t size)
{
    return size == 0 ? 1 : next_pow2(size);
}

dynarray *dynarray_create(size_t room)
{
    dynarray *array = calloc(1, sizeof *array);
    
    if (array == NULL)
    {
        return NULL;
    }
    array->room = next_size(room);
    array->data = malloc(array->room * sizeof(void *));
    if (array->data == NULL)
    {
        free(array);
        return NULL;
    }
    return array;
}

void *dynarray_add(dynarray *array, void *data)
{
    if (data == NULL)
    {
        return NULL;
    }
    if (array->size == array->room)
    {
        size_t room = array->room * 2;
        void *temp = realloc(array->data, room * sizeof(void *));

        if (temp == NULL)
        {
            return NULL;
        }
        array->data = temp;
        array->room = room;
    }
    array->data[array->size++] = data;
    return data;
}

void *dynarray_get(dynarray *array, size_t index)
{
    return array->data[index];
}

size_t dynarray_size(dynarray *array)
{
    return array->size;
}

void dynarray_sort(dynarray *array, int (*comp)(const void *, const void *))
{
    qsort(array->data, array->size, sizeof(void *), comp);
}

/* Binary search */
void *dynarray_bsearch(const dynarray *array, const void *key, int (*comp)(const void *, const void *))
{
    void **data = bsearch(key, array->data, array->size, sizeof(void *), comp);
    
    if (data != NULL)
    {
        return *data;
    }
    return NULL;
}

/* Linear search */
void *dynarray_lsearch(const dynarray *array, const void *key, int (*comp)(const void *, const void *))
{
    for (size_t iter = 0; iter < array->size; iter++)
    {
        if (comp(&array->data[iter], key) == 0)
        {
            return array->data[iter];
        }
    }
    return NULL;
}

void *dynarray_resize(dynarray *array, size_t size, void (*func)(void *))
{
    if (size >= array->size) 
    {
        return array;
    }
    if (func != NULL)
    {
        for (size_t iter = size; iter < array->size; iter++)
        {
            func(array->data[iter]);
        }
    }
    if (size > array->room / 2)
    {
        array->size = size;
        return array;
    }

    size_t room = next_size(size);
    void *temp = realloc(array->data, room * sizeof(void *));

    if (temp == NULL)
    {
        return NULL;
    }
    array->data = temp;
    array->size = size;
    array->room = room;
    return array;
}

void dynarray_destroy(dynarray *array, void (*func)(void *data))
{
    if (func != NULL)
    {
        for (size_t iter = 0; iter < array->size; iter++)
        {
            func(array->data[iter]);
        }
    }
    free(array->data);
    free(array);
}

