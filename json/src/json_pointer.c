#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json_struct.h"

/*
Evaluation of each reference token begins by decoding any escaped character sequence.
This is performed by first transforming any occurrence of the sequence '~1' to '/',
and then transforming any occurrence of the sequence '~0' to '~'.
By performing the substitutions in this order, an implementation avoids the error of
turning '~01' first into '~1' and then into '/', which would be incorrect
(the string '~01' correctly becomes '~1' after transformation).
*/
static int compare(const char *a, const char *b)
{
    for (;;)
    {
        const char *p = a;

        while (*a != '\0')
        {
            if (*a != *b)
            {
                break;
            }
            a++;
            b++;
        }
        if (*a == '\0')
        {
            if ((*b == '/') || (*b == '\0'))
            {
                return 1;
            }
        }
        else if ((a[0] == '/') && (b[0] == '~') && (b[1] == '1'))
        {
            a += 1;
            b += 2;
            continue;
        }
        else if ((a > p) && (a[-1] == '~') && (b[-1] == '~') && (b[0] == '0'))
        {
            b += 1;
            continue;
        }
        return 0;
    }
}

static json *equal(const json *root, const char *name)
{
    if ((root == NULL) || (root->type != JSON_OBJECT))
    {
        return NULL;
    }
    for (json *node = root->child; node != NULL; node = node->next)
    {
        if (compare(node->name, name))
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
        size_t length = (size_t)(end - path);

        /* Locate by #item */
        if (node->type == JSON_ARRAY)
        {
            if (strspn(path, "0123456789") == length)
            {
                node = json_item(node, strtoul(path, NULL, 10));
            }
            else
            {
                return NULL;
            }
        }
        /* Locate by name */
        else
        {
            node = equal(node, path);
        }
        /* Adjust pointer to path */
        path = (*end == '\0') ? end : end + 1;
    }
    return node;
}

