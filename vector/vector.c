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

struct vector
{
    size_t size;
    size_t szof;
    max_align_t data[];
};

void *vector_create(size_t szof)
{
    struct vector *vector = malloc(sizeof(*vector) + szof);

    if (vector == NULL)
    {
        return NULL;
    }
    vector->size = 0;
    vector->szof = szof;
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

void *vector_resize(void *data)
{
    struct vector *vector = VECTOR(*(void **)data);
    size_t size = next_size(vector->size);

    if ((vector->size > 0) && (vector->size == size))
    {
        vector = realloc(vector, sizeof(*vector) + vector->szof * size * 2);
        if (vector == NULL)
        {
            return NULL;
        }
        *(void **)data = vector->data;
    }
    return VECTOR_ITEM(vector, vector->size++);
}

void *vector_shrink(void *data, void (*func)(void *))
{
    struct vector *vector = VECTOR(*(void **)data);

    if (vector->size == 0)
    {
        return vector->data;
    }
    vector->size--;
    if (func != NULL)
    {
        func(VECTOR_ITEM(vector, vector->size));
    }

    size_t size = next_size(vector->size);

    if ((vector->size > 0) && (vector->size == size))
    {
        vector = realloc(vector, sizeof(*vector) + vector->szof * size);
        if (vector == NULL)
        {
            return NULL;
        }
        *(void **)data = vector->data;
    }
    return VECTOR_ITEM(vector, vector->size - 1);
}

void *vector_concat(void *target, const void *source, size_t elms)
{
    struct vector *vector = VECTOR(*(void **)target);

    if (elms == 0)
    {
        return vector->data;
    }

    size_t size = next_size(vector->size);

    if (vector->size + elms >= size)
    {
        size = next_size(vector->size + elms);
        vector = realloc(vector, sizeof(*vector) + vector->szof * size);
        if (vector == NULL)
        {
            return NULL;
        }
        *(void **)target = vector->data;
    }
    target = memcpy(VECTOR_ITEM(vector, vector->size), source, vector->szof * elms);
    vector->size += elms;
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

void vector_destroy(void *data, void (*func)(void *))
{
    struct vector *vector = VECTOR(data);

    if (func != NULL)
    {
        for (size_t item = 0; item < vector->size; item++)
        {
            func(VECTOR_ITEM(vector, item));
        }
    }
    free(vector);
}

