/*! 
 *  \brief     Vector
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "vector.h"

#define CONST_VECTOR(v) ((const struct vector *)((const unsigned char *)(v) - offsetof(struct vector, data)))
#define VECTOR(v) ((struct vector *)((unsigned char *)(v) - offsetof(struct vector, data)))
#define VECTOR_ITEM(v, i) ((unsigned char *)(v)->data + (v)->szof * (i))

typedef void (*fncdel)(void *);

struct vector
{
    size_t size;
    size_t szof;
    fncdel fdel;
    max_align_t data[];
};

void *vector_create(size_t szof, fncdel fdel)
{
    struct vector *vector = malloc(sizeof(*vector) + szof);

    if (vector == NULL)
    {
        return NULL;
    }
    vector->size = 0;
    vector->szof = szof;
    vector->fdel = fdel;
    return vector->data;
}

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

static void *inc(void *data, size_t size)
{
    struct vector *vector = VECTOR(*(void **)data);
    size_t room = next_size(vector->size);

    if (vector->size + size >= room)
    {
        room = next_size(vector->size + size);
        vector = realloc(vector, sizeof(*vector) + vector->szof * room);
        if (vector == NULL)
        {
            return NULL;
        }
        *(void **)data = vector->data;
    }
    data = VECTOR_ITEM(vector, vector->size);
    vector->size += size;
    return data;
}

static void *dec(void *data, size_t size)
{
    struct vector *vector = VECTOR(*(void **)data);

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
        vector = realloc(vector, sizeof(*vector) + vector->szof * room);
        if (vector == NULL)
        {
            return NULL;
        }
        *(void **)data = vector->data;
    }
    return VECTOR_ITEM(vector, vector->size - 1);
}

void *vector_resize(void *data, int size)
{
    if (size > 0)
    {
        return inc(data, (size_t)+size);
    }
    if (size < 0)
    {
        return dec(data, (size_t)-size);
    }
    return NULL;
}

void *vector_concat(void *target, const void *source, size_t size)
{
    struct vector *vector = VECTOR(*(void **)target);

    if (size == 0)
    {
        return NULL;
    }

    size_t room = next_size(vector->size);

    if (vector->size + size >= room)
    {
        room = next_size(vector->size + size);
        vector = realloc(vector, sizeof(*vector) + vector->szof * room);
        if (vector == NULL)
        {
            return NULL;
        }
        *(void **)target = vector->data;
    }
    target = memcpy(VECTOR_ITEM(vector, vector->size), source, vector->szof * size);
    vector->size += size;
    return target;
}

size_t vector_size(const void *data)
{
    return CONST_VECTOR(data)->size;
}

void vector_sort(void *base, int (*comp)(const void *, const void *))
{
    struct vector *vector = VECTOR(base);

    qsort(vector->data, vector->size, vector->szof, comp);
}

void *vector_search(const void *key, const void *base, int (*comp)(const void *, const void *))
{
    const struct vector *vector = CONST_VECTOR(base);

    return bsearch(key, vector->data, vector->size, vector->szof, comp);
}

void *vector_clear(void *data)
{
    struct vector *vector = VECTOR(data);
    size_t szof = vector->szof;
    fncdel fdel = vector->fdel;

    vector_destroy(data);
    return vector_create(szof, fdel);
}

void vector_destroy(void *data)
{
    struct vector *vector = VECTOR(data);

    if (vector->fdel != NULL)
    {
        for (size_t item = 0; item < vector->size; item++)
        {
            vector->fdel(VECTOR_ITEM(vector, item));
        }
    }
    free(vector);
}

