/*! 
 *  \brief     Binary map
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "binmap.h"

struct binmap
{
    uint8_t *data;
    size_t size;
};

/* Next power of two */
static size_t get_size(size_t size)
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

binmap *binmap_create(size_t size)
{
    binmap *map = calloc(1, sizeof *map);

    if (map != NULL)
    {
        // The minimum size is 8 (bits)
        size = size < 8 ? 8 : get_size(size);
        map->data = calloc(size / 8, 1);
        if (map->data == NULL)
        {
            free(map);
            return NULL;
        }
        map->size = size;
    }
    return map;
}

static binmap *resize(binmap *map, size_t size)
{
    size_t old_bytes = map->size / 8;
    size_t new_bytes = size / 8;
    uint8_t *temp;

    temp = realloc(map->data, new_bytes);
    if (temp != NULL)
    {
        map->data = temp;
        map->size = size;
        memset(map->data + old_bytes, 0, new_bytes - old_bytes);
        return map;
    }
    return NULL;
}

/**
 * Set the boolean value at index
 * Returns 1 on success or 0 if it fails (resizing)
 */
int binmap_set(binmap *map, size_t index, int value)
{
    if (index >= map->size)
    {
        // If false is passed then there is no need to resize
        if (value == 0)
        {
            return 1;
        }
        // Resize to the next power of two
        if (resize(map, get_size(index + 1)) == NULL)
        {
            return 0;
        }
    }

    uint8_t *byte = map->data + (index / 8);

    if (value != 0)
    {
        *byte |= (uint8_t)(1u << (index % 8));
    }
    else
    {
        *byte &= (uint8_t)~(1u << (index % 8));
    }
    return 1;
}

/**
 * Get the boolean value at index
 * Returns the boolean value of index or 0 if index doesn't exist
 */
int binmap_get(const binmap *map, size_t index)
{
    if (index >= map->size)
    {
        return 0;
    }

    uint8_t byte = map->data[index / 8];

    return (byte & (1u << (index % 8))) ? 1 : 0;
}

void binmap_destroy(binmap *map)
{
    if (map != NULL)
    {
        free(map->data);
        free(map);
    }
}

