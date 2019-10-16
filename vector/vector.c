/*! 
 *  \brief     Vector
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdlib.h>
#include <string.h>
#include "vector.h"

#define VECTOR_ITEM(v, i) ((unsigned char *)(v)->data + (v)->szof * (i))

typedef void (*cb_del)(void *); // Callback to delete function

struct vector
{
    unsigned char *data;    // The contents of the array
    size_t size;            // Number of elements of the array
    size_t szof;            // sizeof each element of the array
    cb_del fdel;            // Pointer to callback to delete function

};

void *vector_create(size_t szof, cb_del fdel)
{
    struct vector *vector = calloc(1, sizeof(*vector));

    if (vector == NULL)
    {
        return NULL;
    }
    vector->szof = szof;
    vector->fdel = fdel;
    return vector;
}

/* Round up to the next power of 2 */
static size_t next_size(size_t size)
{
    size--;
    size |= size >> 1;
    size |= size >> 2;
    size |= size >> 4;
    size |= size >> 8;
    size |= size >> 16;
    if (sizeof(size) >= 8)
    {
        size |= size >> 32;
    }
    size++;
    return size;
}

static void *resize(struct vector *vector, size_t size)
{
    return realloc(vector->data, vector->szof * size);
}

static void *increment(void *data, size_t size)
{
    struct vector *vector = data;
    size_t room = next_size(vector->size);

    if (vector->size + size >= room)
    {
        room = next_size(vector->size + size);
        data = resize(vector, room);
        if (data == NULL)
        {
            return NULL;
        }
        vector->data = data;
    }
    data = VECTOR_ITEM(vector, vector->size);
    vector->size += size;
    return data;
}

static void *decrement(void *data, size_t size)
{
    struct vector *vector = data;

    if (vector->size == 0)
    {
        return data;
    }

    size_t room = next_size(vector->size);

    if (size > vector->size)
    {
        size = vector->size;
    }
    if (vector->fdel != NULL)
    {
        while (size--)
        {
            vector->fdel(VECTOR_ITEM(vector, --vector->size));
        }
    }
    else
    {
        vector->size -= size;
    }
    if (vector->size <= room / 2)
    {
        room = (vector->size == 0) ? 1 : next_size(vector->size);
        data = resize(vector, room);
        if (data == NULL)
        {
            return NULL;
        }
        vector->data = data;
    }
    return VECTOR_ITEM(vector, vector->size);
}

void *vector_resize(void *data, int size)
{
    if (size > 0)
    {
        return increment(data, (size_t)+size);
    }
    if (size < 0)
    {
        return decrement(data, (size_t)-size);
    }
    return NULL;
}

void *vector_copy(void *data, const void *source, size_t size)
{
    if (size == 0)
    {
        return NULL;
    }

    struct vector *vector = data;
    size_t diff = (size > vector->size) ? size - vector->size : 0;
    size_t room = next_size(vector->size);

    if (vector->fdel != NULL)
    {
        for (size_t item = 0; (item < size) && (item < vector->size); item++)
        {
            vector->fdel(VECTOR_ITEM(vector, item));
        }
    }
    if ((diff > 0) && (vector->size + diff >= room))
    {
        room = next_size(vector->size + diff);
        data = resize(vector, room);
        if (data == NULL)
        {
            return NULL;
        }
        vector->data = data;
    }
    memcpy(vector->data, source, vector->szof * size);
    vector->size += diff;
    return vector->data;
}

void *vector_concat(void *data, const void *source, size_t size)
{
    if (size == 0)
    {
        return NULL;
    }

    struct vector *vector = data;
    size_t room = next_size(vector->size);

    if (vector->size + size >= room)
    {
        room = next_size(vector->size + size);
        data = resize(vector, room);
        if (data == NULL)
        {
            return NULL;
        }
        vector->data = data;
    }
    data = memcpy(VECTOR_ITEM(vector, vector->size), source, vector->szof * size);
    vector->size += size;
    return data;
}

size_t vector_size(const void *data)
{
    const struct vector *vector = data;

    return vector->size;
}

size_t vector_sizeof(const void *data)
{
    const struct vector *vector = data;

    return vector->szof * vector->size;
}

void vector_sort(void *data, int (*comp)(const void *, const void *))
{
    struct vector *vector = data;

    qsort(vector->data, vector->size, vector->szof, comp);
}

/* Binary search */
void *vector_bsearch(const void *key, void *data, int (*comp)(const void *, const void *))
{
    struct vector *vector = data;

    return bsearch(key, vector->data, vector->size, vector->szof, comp);
}

/* Linear search */
void *vector_lsearch(const void *key, void *data, int (*comp)(const void *, const void *))
{
    struct vector *vector = data;
    unsigned char *item = vector->data;
    unsigned char *last = item + vector->szof * vector->size;

    while (item < last)
    {
        if (comp(item, key) == 0)
        {
            return item;
        }
        item += vector->szof;
    }
    return NULL;
}

void *vector_min(void *data, int (*comp)(const void *, const void *))
{
    struct vector *vector = data;

    if (vector->size == 0)
    {
        return NULL;
    }

    unsigned char *min = vector->data;
    unsigned char *item = min + vector->szof;
    unsigned char *last = min + vector->szof * vector->size;

    while (item < last)
    {
        if (comp(item, min) < 0)
        {
            min = item;
        }
        item += vector->szof;
    }
    return min;
}

void *vector_max(void *data, int (*comp)(const void *, const void *))
{
    struct vector *vector = data;

    if (vector->size == 0)
    {
        return NULL;
    }

    unsigned char *max = vector->data;
    unsigned char *item = max + vector->szof;
    unsigned char *last = max + vector->szof * vector->size;

    while (item < last)
    {
        if (comp(item, max) > 0)
        {
            max = item;
        }
        item += vector->szof;
    }
    return max;
}

void *vector_clear(void *data)
{
    struct vector *vector = data;
 
    if (vector->fdel != NULL)
    {
        for (size_t item = 0; item < vector->size; item++)
        {
            vector->fdel(VECTOR_ITEM(vector, item));
        }
    }
    free(vector->data);
    vector->size = 0;
    vector->data = NULL;
    return vector;
}

void vector_destroy(void *data)
{
    free(vector_clear(data));
}

