#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json_struct.h"

static int compare(const char *name, const char *path, const char *end)
{
    for (; path < end; name++, path++)
    {
        /* Any occurrence of '~' in 'name' must match with '~0' in 'path' */
        if (*name == '~')
        {
            if ((path[0] != '~') || (path[1] != '0'))
            {
                return 0;
            }
            path++;
        }
        /* Any occurrence of '/' in 'name' must match with '~1' in 'path' */
        else if (*name == '/')
        {
            if ((path[0] != '~') || (path[1] != '1'))
            {
                return 0;
            }
            path++;
        }
        /* Doesn't match */
        else if (*name != *path)
        {
            return 0;
        }
    }
    return (*name == '\0') && (path == end);
}

static json *equal(const json *root, const char *path, const char *end)
{
    if ((root == NULL) || (root->type != JSON_OBJECT))
    {
        return NULL;
    }
    for (json *node = root->child; node != NULL; node = node->next)
    {
        if (compare(node->name, path, end))
        {
            return node;
        }
    }
    return NULL;
}

/* Locates a node by path */
json *json_pointer(const json *root, const char *path)
{
    json *node = NULL;

    if (path[0] == '/')
    {
        node = json_root(root);
        path += 1;
    }
    else
    {
        node = json_self(root);
    }
    while ((node != NULL) && (*path != '\0'))
    {
        const char *end = path + strcspn(path, "/");

        /* Locate by #item */
        if (node->type == JSON_ARRAY)
        {
            if (path + strspn(path, "0123456789") != end)
            {
                return NULL;
            }
            node = json_item(node, strtoul(path, NULL, 10));
        }
        /* Locate by name */
        else
        {
            node = equal(node, path, end);
        }
        /* Adjust pointer to path */
        path = (*end == '\0') ? end : end + 1;
    }
    return node;
}

