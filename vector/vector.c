/*! 
 *  \brief     Vector
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include "vector.h"

struct vector {
    size_t room;
    size_t size;
    size_t szof;
    max_align_t data[];
};

#define CONST_VECTOR(v) ((const struct vector *)((const unsigned char *)v - offsetof(struct vector, data)))
#define VECTOR(v) ((struct vector *)((unsigned char *)v - offsetof(struct vector, data)))
#define VECTOR_ITEM(v, i) ((unsigned char *)v->data + v->szof * i)

void *vcreate(size_t szof)
{
    struct vector *vector;

    vector = malloc(sizeof(*vector) + szof);
    if (vector == NULL) {
        return NULL;
    }
    vector->room = 1;
    vector->size = 0;
    vector->szof = szof;
    return vector->data;
}

void vdestroy(void *data, void (*func)(void *))
{
    struct vector *vector = VECTOR(data);

    if (func != NULL) {
        for (size_t item = 0; item < vector->size; item++) {
            func(VECTOR_ITEM(vector, item));
        }
    }
    free(vector);
}

static void *vresize(struct vector *vector, size_t size)
{
    struct vector *new;

    new = realloc(vector, sizeof(*vector) + vector->szof * size);
    if (new != NULL) {
        new->room = size;
    }
    return new;
}

size_t vsize(const void *data)
{
    return CONST_VECTOR(data)->size;
}

static void *grow(void *data)
{
    struct vector *vector = VECTOR(data);

    if (vector->size >= vector->room) {
        vector = vresize(vector, vector->room * 2);
        if (vector == NULL) {
            return NULL;
        }
    }
    vector->size++;
    return vector->data;
}

void *vgrow(void *data)
{
    return *(void **)data = grow(*(void **)data);
}

static void *add(void *data, size_t size)
{
    struct vector *vector = VECTOR(data);
    void *item;

    if (vector->size >= vector->room) {
        vector = vresize(vector, vector->room * 2);
        if (vector == NULL) {
            return NULL;
        }
    }
    item = calloc(1, size);
    if (item == NULL) {
        return NULL;
    }
    memcpy(VECTOR_ITEM(vector, vector->size), &item, vector->szof);
    vector->size++;
    return vector->data;
}

void *vadd(void *data, size_t size)
{
    return *(void **)data = add(*(void **)data, size);
}

void vsort(void *base, int (*comp)(const void *, const void *))
{
    struct vector *vector = VECTOR(base);

    if (vector->size > 1) {
        qsort(vector->data, vector->size, vector->szof, comp);
    }
}

void *vsearch(const void *key, const void *base, int (*comp)(const void *, const void *))
{
    const struct vector *vector = CONST_VECTOR(base);

    return bsearch(key, vector->data, vector->size, vector->szof, comp);
}

static char *strdup_vprintf(const char *fmt, va_list args)
{
	va_list copy;
	size_t size;
	char *data;

	va_copy(copy, args);
	size = (size_t)vsnprintf(NULL, 0, fmt, args) + 1;
        data = malloc(size);
	if (data == NULL) {
            return NULL;
	}
	vsprintf(data, fmt, copy);
	return data;
}

char *strdup_printf(const char *fmt, ...)
{
	va_list args;
	char *data;

	va_start(args, fmt);
	data = strdup_vprintf(fmt, args);
	va_end(args);
	return data;
}
