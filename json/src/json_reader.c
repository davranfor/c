/*!
 *  \brief     json and json-schema library for C
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "json_struct.h"

static const char *type_name[] =
{
    "Undefined",
    "Object",
    "Array",
    "String",
    "Integer",
    "Double",
    "Boolean",
    "Null"
};

static int (*func_array[])(const json *) =
{
    json_is_any,
    json_is_iterable,
    json_is_scalar,
    json_is_object,
    json_is_array,
    json_is_string,
    json_is_integer,
    json_is_real,
    json_is_double,
    json_is_number,
    json_is_boolean,
    json_is_null,
};

enum {func_size = sizeof func_array / sizeof *func_array};

static int is(const json *node, int (*func)(const json *))
{
    while (func(node) != 0)
    {
        node = node->next;
    }
    return node == NULL;
}

static int is_unique(const json *node, int (*func)(const json *))
{
    const json *head = node;

    while (func(node) != 0)
    {
        for (const json *item = head; item != node; item = item->next)
        {
            if (json_equal(node, item))
            {
                return 0;
            }
        }
        node = node->next;
    }
    return node == NULL;
}

int json_is(const json *node, enum json_query query)
{
    if (node == NULL)
    {
        return 0;
    }

    int (*func)(const json *) = func_array[query % func_size];

    if ((query >= objectOfItems) && (query <= objectOfNulls))
    {
        if (node->type != JSON_OBJECT)
        {
            return 0;
        }
        return (node = node->child) ? is(node, func) : 0;
    }
    if ((query >= arrayOfItems) && (query <= arrayOfNulls))
    {
        if (node->type != JSON_ARRAY)
        {
            return 0;
        }
        return (node = node->child) ? is(node, func) : 0;
    }
    if ((query >= objectOfOptionalItems) && (query <= objectOfOptionalNulls))
    {
        if (node->type != JSON_OBJECT)
        {
            return 0;
        }
        return (node = node->child) ? is(node, func) : 1;
    }
    if ((query >= arrayOfOptionalItems) && (query <= arrayOfOptionalNulls))
    {
        if (node->type != JSON_ARRAY)
        {
            return 0;
        }
        return (node = node->child) ? is(node, func) : 1;
    }
    if ((query >= objectOfUniqueItems) && (query <= objectOfUniqueNulls))
    {
        if (node->type != JSON_OBJECT)
        {
            return 0;
        }
        return (node = node->child) ? is_unique(node, func) : 1;
    }
    if ((query >= arrayOfUniqueItems) && (query <= arrayOfUniqueNulls))
    {
        if (node->type != JSON_ARRAY)
        {
            return 0;
        }
        return (node = node->child) ? is_unique(node, func) : 1;
    }
    return 0;
}

enum json_type json_type(const json *node)
{
    if (node == NULL)
    {
        return JSON_UNDEFINED;
    }
    return node->type;
}

const char *json_type_name(const json *node)
{
    if (node == NULL)
    {
        return NULL;
    }
    return type_name[node->type];
}

const char *json_name(const json *node)
{
    if (node == NULL)
    {
        return NULL;
    }
    return node->name;
}

const char *json_string(const json *node)
{
    if (node == NULL)
    {
        return NULL;
    }
    return node->value;
}

long long json_integer(const json *node)
{
    if ((node == NULL) || (node->value == NULL))
    {
        return 0;
    }
    return strtoll(node->value, NULL, 10);
}

unsigned long long json_real(const json *node)
{
    if ((node == NULL) || (node->value == NULL))
    {
        return 0;
    }
    return strtoull(node->value, NULL, 10);
}

double json_double(const json *node)
{
    if ((node == NULL) || (node->value == NULL))
    {
        return 0.0;
    }
    return strtod(node->value, NULL);
}

double json_number(const json *node)
{
    if ((node == NULL) || (node->value == NULL))
    {
        return 0.0;
    }
    return strtod(node->value, NULL);
}

int json_boolean(const json *node)
{
    if (node == NULL)
    {
        return 0;
    }
    if (node->type == JSON_BOOLEAN)
    {
        return node->value[0] == 't';
    }
    if (node->value != NULL)
    {
        return strtod(node->value, NULL) != 0.0;
    }
    return 0;
}

int json_is_any(const json *node)
{
    return node != NULL;
}

int json_is_iterable(const json *node)
{
    return (node != NULL)
        && (node->value == NULL);
}

int json_is_scalar(const json *node)
{
    return (node != NULL)
        && (node->value != NULL);
}

int json_is_object(const json *node)
{
    return (node != NULL)
        && (node->type == JSON_OBJECT);
}

int json_is_array(const json *node)
{
    return (node != NULL)
        && (node->type == JSON_ARRAY);
}

int json_is_string(const json *node)
{
    return (node != NULL)
        && (node->type == JSON_STRING);
}

int json_is_integer(const json *node)
{
    return (node != NULL)
        && (node->type == JSON_INTEGER);
}

int json_is_real(const json *node)
{
    return (node != NULL)
        && (node->type == JSON_INTEGER)
        && (strtod(node->value, NULL) >= 0);
}

int json_is_double(const json *node)
{
    return (node != NULL)
        && (node->type == JSON_DOUBLE);
}

int json_is_number(const json *node)
{
    return (node != NULL)
       && ((node->type == JSON_INTEGER)
       ||  (node->type == JSON_DOUBLE));
}

int json_is_boolean(const json *node)
{
    return (node != NULL)
        && (node->type == JSON_BOOLEAN);
}

int json_is_true(const json *node)
{
    return (node != NULL)
        && (node->type == JSON_BOOLEAN)
        && (node->value[0] == 't');
}

int json_is_false(const json *node)
{
    return (node != NULL)
        && (node->type == JSON_BOOLEAN)
        && (node->value[0] == 'f');
}

int json_is_null(const json *node)
{
    return (node != NULL)
        && (node->type == JSON_NULL);
}

/**
 * Silence compiler.
 * Useful to return a non constant 'json *node' when a function
 * gets 'const json *node' as argument and returns the same node.
 * For example calling 'x = json_pointer(node, "");'
 * returns the same node that was passed.
 */
json *json_self(const json *node)
{
    uintptr_t cast = (uintptr_t)(const void *)node;

    return (void *)cast;
}

json *json_root(const json *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    json *root = node->parent;

    if (root == NULL)
    {
        return json_self(node);
    }
    while (root->parent != NULL)
    {
        root = root->parent;
    }
    return root;
}

json *json_parent(const json *node)
{
    if (node == NULL)
    {
        return NULL;
    }
    return node->parent;
}

json *json_child(const json *node)
{
    if (node == NULL)
    {
        return NULL;
    }
    return node->child;
}

json *json_next(const json *node)
{
    if (node == NULL)
    {
        return NULL;
    }
    return node->next;
}

/* Locates a child by index */
json *json_at(const json *root, size_t index)
{
    if (root == NULL)
    {
        return NULL;
    }
    for (json *node = root->child; node != NULL; node = node->next)
    {
        if (index-- == 0)
        {
            return node;
        }
    }
    return NULL;
}

/* Locates a child by name */
json *json_find(const json *root, const char *name)
{
    if ((root == NULL) || (root->type != JSON_OBJECT) || (name == NULL))
    {
        return NULL;
    }
    for (json *node = root->child; node != NULL; node = node->next)
    {
        if (strcmp(node->name, name) == 0)
        {
            return node;
        }
    }
    return NULL;
}

/* Locates the next sibling by name */
json *json_find_next(const json *root, const char *name)
{
    if ((root == NULL) || (root->name == NULL) || (name == NULL))
    {
        return NULL;
    }
    for (json *node = root->next; node != NULL; node = node->next)
    {
        if (strcmp(node->name, name) == 0)
        {
            return node;
        }
    }
    return NULL;
}

/* Number of childs of an iterable */
size_t json_size(const json *node)
{
    if (node == NULL)
    {
        return 0;
    }

    size_t size = 0;

    for (node = node->child; node != NULL; node = node->next)
    {
        size++;
    }
    return size;
}

/* Position of the node into an interable */
size_t json_offset(const json *node)
{
    size_t offset = 0;

    if ((node != NULL) && (node->parent != NULL))
    {
        const json *iter = node->parent->child;

        while (iter != node)
        {
            iter = iter->next;
            offset++;
        }
    }
    return offset;
}

/* Number of edges from the root node to the passed node */
int json_depth(const json *node)
{
    int depth = 0;

    if (node != NULL)
    {
        while (node->parent != NULL)
        {
            node = node->parent;
            depth++;
        }
    }
    return depth;
}

/* json_equal helper */
static int equal(const json *a, const json *b, int depth)
{
    if (a->type != b->type)
    {
        return 0;
    }
    if ((a->child == NULL) ^ (b->child == NULL))
    {
        return 0;
    }
    if (depth > 0)
    {
        if ((a->next == NULL) ^ (b->next == NULL))
        {
            return 0;
        }
        if ((a->name == NULL) ^ (b->name == NULL))
        {
            return 0;
        }
        if ((a->name != NULL) && strcmp(a->name, b->name))
        {
            return 0;
        }
    }
    if ((a->value == NULL) ^ (b->value == NULL))
    {
        return 0;
    }
    if (a->value != NULL)
    {
        if (json_is_number(a))
        {
            return strtod(a->value, NULL) == strtod(b->value, NULL);
        }
        else
        {
            return strcmp(a->value, b->value) == 0;
        }
    }
    return 1;
}

/*
 * Compares two nodes
 * Returns 1 when nodes are equal, 0 otherwise
 */
int json_equal(const json *a, const json *b)
{
    if ((a == NULL) & (b == NULL))
    {
        return 1;
    }
    if ((a == NULL) ^ (b == NULL))
    {
        return 0;
    }

    int depth = 0;

    while (equal(a, b, depth))
    {
        if (a->child != NULL)
        {
            a = a->child;
            b = b->child;
            depth++;
        }
        else if ((depth > 0) && (a->next != NULL))
        {
            a = a->next;
            b = b->next;
        }
        else
        {
            while (depth-- > 0)
            {
                a = a->parent;
                b = b->parent;
                if (a->next != NULL)
                {
                    a = a->next;
                    b = b->next;
                    break;
                }
            }
            if (depth <= 0)
            {
                return 1;
            }
        }
    }
    return 0;
}

/*
 * Sends all nodes to a callback func providing depth and user-data
 * Exit when all nodes are read or func returns a non 0 value
 */
int json_traverse(const json *node, json_callback func, void *data)
{
    int depth = 0;

    while (node != NULL)
    {
        int result;

        if ((result = func(node, depth, data)))
        {
            return result;
        }
        if (node->child != NULL)
        {
            node = node->child;
            depth++;
        }
        else if ((depth > 0) && (node->next != NULL))
        {
            node = node->next;
        }
        else
        {
            while (depth-- > 0)
            {
                node = node->parent;
                if (node->next != NULL)
                {
                    node = node->next;
                    break;
                }
            }
            if (depth <= 0)
            {
                break;
            }
        }
    }
    return 0;
}

