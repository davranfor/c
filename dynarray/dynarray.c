/*! 
 *  \brief     Dynamic array
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdlib.h>
#include <string.h>
#include "dynarray.h"

struct dynarray
{
    void **data;
    size_t size;
};

static size_t next_size(size_t size)
{
    // size is a power of two?
    if ((size & (size - 1)) == 0)
    {
        return size == 0 ? 1 : size << 1;
    }
    return 0;
}

dynarray *dynarray_create(void)
{
    dynarray *array = calloc(1, sizeof *array);

    return array;
}

void *dynarray_push(dynarray *array, void *data)
{
    if (data == NULL)
    {
        return NULL;
    }

    size_t size = next_size(array->size);

    if (size != 0)
    {
        void *temp = realloc(array->data, size * sizeof(void *));

        if (temp == NULL)
        {
            return NULL;
        }
        array->data = temp;
    }
    array->data[array->size++] = data;
    return data;
}

void *dynarray_pop(dynarray *array)
{
    if (array->size == 0)
    {
        return NULL;
    }
    return array->data[--array->size];
}

void *dynarray_insert(dynarray *array, size_t index, void *data)
{
    if ((data == NULL) || (index > array->size))
    {
        return NULL;
    }

    size_t size = next_size(array->size);

    if (size != 0)
    {
        void *temp = realloc(array->data, size * sizeof(void *));

        if (temp == NULL)
        {
            return NULL;
        }
        array->data = temp;
    }
    memmove(
        (array->data + index + 1),
        (array->data + index),
        (array->size - index) * sizeof(void *)
    );
    array->data[index] = data;
    array->size++;
    return data;
}

void *dynarray_delete(dynarray *array, size_t index)
{
    if (index >= array->size) 
    {
        return NULL;
    }

    void *data = array->data[index];

    memmove(
        (array->data + index),
        (array->data + index + 1),
        (array->size - index - 1) * sizeof(void *)
    );
    array->size--;
    return data;
}

/**
 * Realloc to the optimal size after deleting items
 * Do not use it on each pop / delete operation
 */
void *dynarray_refresh(dynarray *array)
{
    if (array->size == 0)
    {
        dynarray_clear(array, NULL);
        return array;
    }

    size_t size = next_size(array->size);

    if (size != 0)
    {
        void *temp = realloc(array->data, size * sizeof(void *));

        if (temp == NULL)
        {
            return NULL;
        }
        array->data = temp;
    }
    return array;
}

/**
 * Replace an item (the index must exist)
 * Returns the old item
 */
void *dynarray_set(dynarray *array, size_t index, void *data)
{
    if ((data == NULL) || (index >= array->size))
    {
        return NULL;
    }

    void *temp = array->data[index];

    array->data[index] = data;
    return temp;
}

void *dynarray_get(const dynarray *array, size_t index)
{
    if (index < array->size)
    {
        return array->data[index];
    }
    return NULL;
}

size_t dynarray_size(const dynarray *array)
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

void dynarray_reverse(const dynarray *array)
{
    if (array->size > 1)
    {
        for (size_t a = 0, b = array->size - 1; a < b; a++, b--)
        {
            struct node *temp = array->data[a];

            array->data[a] = array->data[b];
            array->data[b] = temp;
        }
    }
}

void dynarray_clear(dynarray *array, void (*func)(void *data))
{
    if (func != NULL)
    {
        for (size_t iter = 0; iter < array->size; iter++)
        {
            func(array->data[iter]);
        }
    }
    free(array->data);
    array->data = NULL;
    array->size = 0;
}

void dynarray_destroy(dynarray *array, void (*func)(void *data))
{
    dynarray_clear(array, func);
    free(array);
}

