/*!
 *  \brief     json and json-schema library for C
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json_struct.h"

static int compare(const char *name, const char *path, const char *end)
{
    for (; path < end; name++, path++)
    {
        // '~' in name must match with '~0' in path
        if (*name == '~')
        {
            if ((path[0] != '~') || (path[1] != '0'))
            {
                return 0;
            }
            path++;
        }
        // '/' in name must match with '~1' in path
        else if (*name == '/')
        {
            if ((path[0] != '~') || (path[1] != '1'))
            {
                return 0;
            }
            path++;
        }
        // Doesn't match
        else if (*name != *path)
        {
            return 0;
        }
    }
    return (*name == '\0');
}

static json *get_by_name(const json *root, const char *path, const char *end)
{
    for (json *node = root->child; node != NULL; node = node->next)
    {
        if (compare(node->name, path, end))
        {
            return node;
        }
    }
    return NULL;
}

static json *get_by_item(const json *root, const char *path, const char *end)
{
    if (path + strspn(path, "0123456789") != end)
    {
        return NULL;
    }

    unsigned long item = strtoul(path, NULL, 10);

    for (json *node = root->child; node != NULL; node = node->next)
    {
        if (item-- == 0)
        {
            return node;
        }
    }
    return NULL;
}

static const char *next(const char *path, const char *delim)
{
    return path + strcspn(path, delim);
}

/* Locates a node by path */
json *json_pointer(const json *root, const char *path)
{
    json *node = (*path == '/') ? path++, json_root(root) : json_self(root);

    while ((node != NULL) && (*path != '\0'))
    {
        const char *end = next(path, "/");

        node = (node->type == JSON_OBJECT)
            ? get_by_name(node, path, end)
            : get_by_item(node, path, end);
        path = *end ? end + 1 : end;
    }
    return node;
}

