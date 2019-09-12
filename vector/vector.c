/*! 
 *  \brief     Vector
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "vector.h"

#define CONST_VECTOR(v) ((const struct vector *)((const unsigned char *)(v) - offsetof(struct vector, data)))
#define VECTOR(v) ((struct vector *)((unsigned char *)(v) - offsetof(struct vector, data)))
#define VECTOR_ITEM(v, i) ((unsigned char *)(v)->data + (v)->szof * (i))

struct vector
{
    size_t room;
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
    vector->room = 1;
    vector->size = 0;
    vector->szof = szof;
    return vector->data;
}

void *vector_resize(void *data)
{
    struct vector *vector = VECTOR(data);
    struct vector *new = vector;

    if (vector->size >= vector->room)
    {
        vector->room *= 2;
        new = realloc(vector, sizeof(*vector) + vector->szof * vector->room);
        if (new == NULL)
        {
            return NULL;
        }
    }
    new->size++;
    return new->data;
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

