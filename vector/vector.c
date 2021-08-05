/*! 
 *  \brief     Vector
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdlib.h>
#include <string.h>
#include "vector.h"

struct vector
{
    void * data;            // The contents of the array
    size_t size;            // Number of elements of the array
    size_t szof;            // sizeof each element of the array
    void (*fdel)(void *);   // Pointer to callback to delete function
};

#define VECTOR_ITEM(v, i) ((unsigned char *)((v)->data) + (v)->szof * (i))

vector *vector_create(size_t szof, void (*fdel)(void *))
{
    vector *vec = calloc(1, sizeof(*vec));

    if (vec != NULL)
    {
        vec->szof = szof;
        vec->fdel = fdel;
    }
    return vec;
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

static void *resize(vector *vec, size_t size)
{
    return realloc(vec->data, vec->szof * size);
}

static void *increment(vector *vec, size_t size)
{
    size_t room = next_size(vec->size);
    void *data;

    if (vec->size + size > room)
    {
        room = next_size(vec->size + size);
        data = resize(vec, room);
        if (data == NULL)
        {
            return NULL;
        }
        vec->data = data;
    }
    data = VECTOR_ITEM(vec, vec->size);
    vec->size += size;
    return data;
}

static void *decrement(vector *vec, size_t size)
{
    if (vec->size == 0)
    {
        return NULL;
    }

    size_t room = next_size(vec->size);

    if (size > vec->size)
    {
        size = vec->size;
    }
    if (vec->fdel != NULL)
    {
        while (size--)
        {
            vec->fdel(VECTOR_ITEM(vec, --vec->size));
        }
    }
    else
    {
        vec->size -= size;
    }
    if (vec->size == 0)
    {
        free(vec->data);
        vec->data = NULL;
        return NULL;
    }
    if (vec->size <= room / 2)
    {
        room = next_size(vec->size);

        void *data = resize(vec, room);

        /*
         * Since the API returns `NULL` when the vector is empty (0 items),
         * we don't return NULL in the very unlikely case that realloc fail
         * allocating less memory
         */
        if (data != NULL)
        {
            vec->data = data;
        }
    }
    return VECTOR_ITEM(vec, vec->size);
}

void *vector_resize(vector *vec, int size)
{
    if (size > 0)
    {
        return increment(vec, (size_t)+size);
    }
    if (size < 0)
    {
        return decrement(vec, (size_t)-size);
    }
    return NULL;
}

void *vector_insert(vector *vec, size_t index)
{
    if (index > vec->size)
    {
        return NULL;
    }
    if (index == vec->size)
    {
        return increment(vec, 1);
    }

    size_t room = next_size(vec->size);

    if (vec->size + 1 > room)
    {
        room = next_size(vec->size + 1);

        void *data = resize(vec, room);

        if (data == NULL)
        {
            return NULL;
        }
        vec->data = data;
    }
    memmove(
        VECTOR_ITEM(vec, index + 1),
        VECTOR_ITEM(vec, index),
        vec->szof * (vec->size++ - index)
    );
    return VECTOR_ITEM(vec, index);
}

void *vector_delete(vector *vec, size_t index)
{
    if (index >= vec->size)
    {
        return NULL;
    }
    if (index == vec->size - 1)
    {
        return decrement(vec, 1);
    }

    size_t room = next_size(vec->size--);

    if (vec->fdel != NULL)
    {
        vec->fdel(VECTOR_ITEM(vec, index));
    }
    memmove(
        VECTOR_ITEM(vec, index),
        VECTOR_ITEM(vec, index + 1),
        vec->szof * (vec->size - index)
    );
    if (vec->size <= room / 2)
    {
        room = next_size(vec->size);

        void *data = resize(vec, room);

        if (data != NULL)
        {
            vec->data = data;
        }
    }
    return VECTOR_ITEM(vec, index);
}

void *vector_copy(vector *vec, const void *source, size_t size)
{
    if (size == 0)
    {
        return NULL;
    }

    size_t diff = (size > vec->size) ? size - vec->size : 0;
    size_t room = next_size(vec->size);

    if (vec->fdel != NULL)
    {
        for (size_t item = 0; (item < size) && (item < vec->size); item++)
        {
            vec->fdel(VECTOR_ITEM(vec, item));
        }
    }
    if ((diff > 0) && (vec->size + diff > room))
    {
        room = next_size(vec->size + diff);

        void *data = resize(vec, room);

        if (data == NULL)
        {
            return NULL;
        }
        vec->data = data;
    }
    memcpy(vec->data, source, vec->szof * size);
    vec->size += diff;
    return vec->data;
}

void *vector_concat(vector *vec, const void *source, size_t size)
{
    if (size == 0)
    {
        return NULL;
    }

    size_t room = next_size(vec->size);
    void *data;

    if (vec->size + size > room)
    {
        room = next_size(vec->size + size);
        data = resize(vec, room);
        if (data == NULL)
        {
            return NULL;
        }
        vec->data = data;
    }
    data = memcpy(VECTOR_ITEM(vec, vec->size), source, vec->szof * size);
    vec->size += size;
    return data;
}

void vector_sort(vector *vec, int (*comp)(const void *, const void *))
{
    qsort(vec->data, vec->size, vec->szof, comp);
}

/* Binary search */
void *vector_bsearch(const vector *vec, const void *key, int (*comp)(const void *, const void *))
{
    return bsearch(key, vec->data, vec->size, vec->szof, comp);
}

/* Silence compiler casting non const to const with `(void *)const_var` */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"

/* Linear search */
void *vector_lsearch(const vector *vec, const void *key, int (*comp)(const void *, const void *))
{
    const unsigned char *item = vec->data;
    const unsigned char *last = item + vec->szof * vec->size;

    while (item < last)
    {
        if (comp(item, key) == 0)
        {
            return (void *)item;
        }
        item += vec->szof;
    }
    return NULL;
}

#pragma GCC diagnostic pop

vector *vector_clear(vector *vec)
{
    if (vec->fdel != NULL)
    {
        for (size_t item = 0; item < vec->size; item++)
        {
            vec->fdel(VECTOR_ITEM(vec, item));
        }
    }
    free(vec->data);
    vec->data = NULL;
    vec->size = 0;
    return vec;
}

void vector_destroy(vector *vec)
{
    if (vec != NULL)
    {
        free(vector_clear(vec));
    }
}

