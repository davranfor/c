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
    return calloc(1, sizeof(dynarray));
}

void *dynarray_push(dynarray *array, void *data)
{
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
    if (index > array->size)
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
    memmove((array->data + index + 1),
            (array->data + index),
            (array->size - index) * sizeof(void *));
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

    memmove((array->data + index),
            (array->data + index + 1),
            (array->size - index - 1) * sizeof(void *));
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
void *dynarray_set(const dynarray *array, size_t index, void *data)
{
    if (index >= array->size)
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

static void swap(void *a[], void *b[])
{
    void *tm;

    tm = *a;
    *a = *b;
    *b = tm;
}

static int partition(void *array[], int head, int tail, int (*comp)(const void *, const void *))
{
    int part = head - 1;

    for (int iter = head; iter <= tail - 1; iter++)
    {
        if (comp(array[iter], array[tail]) <= 0)
        {
            swap(&array[++part], &array[iter]);
        }
    }
    swap(&array[part + 1], &array[tail]);
    return part + 1;
}

static void sort(void *array[], int head, int tail, int (*comp)(const void *, const void *))
{
    if (head < tail)
    {
        int part = partition(array, head, tail, comp);

        sort(array, head, part - 1, comp);
        sort(array, part + 1, tail, comp);
    }
}

void dynarray_sort(dynarray *array, int (*comp)(const void *, const void *))
{
    sort(array->data, 0, (int)array->size - 1, comp);
}

/* Binary search */
void *dynarray_bsearch(const dynarray *array, const void *key, int (*comp)(const void *, const void *))
{
    int head = 0;
    int tail = (int)array->size - 1;

    while (head <= tail)
    {
        int mid = (head + tail) / 2;
        int cmp = comp(key, array->data[mid]);

        if (cmp < 0)
        {
            tail = mid - 1;
        }
        else if (cmp > 0)
        {
            head = mid + 1;
        }
        else
        {
            return array->data[mid];
        }
    }
    return NULL;
}

/* Linear search */
void *dynarray_lsearch(const dynarray *array, const void *key, int (*comp)(const void *, const void *))
{
    for (size_t iter = 0; iter < array->size; iter++)
    {
        if (comp(key, array->data[iter]) == 0)
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

void dynarray_clear(dynarray *array, void (*func)(void *))
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

void dynarray_destroy(dynarray *array, void (*func)(void *))
{
    if (array != NULL)
    {
        dynarray_clear(array, func);
        free(array);
    }
}

