#include <stdio.h>
#include <stdlib.h>
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
    json_is_object,
    json_is_array,
    json_is_string,
    json_is_integer,
    json_is_double,
    json_is_number,
    json_is_real,
    json_is_boolean,
    json_is_null,
};

enum {func_size = sizeof func_array / sizeof *func_array};

static int is(const json *node, int (*func)(const json *))
{
    while (func(node) != 0)
    {
        node = node->right;
    }
    return node == NULL;
}

static int is_unique(const json *node, int (*func)(const json *))
{
    const json *head = node;

    while (func(node) != 0)
    {
        for (const json *item = head; item != node; item = item->right)
        {
            if (json_equal(node, item))
            {
                return 0;
            }
        }
        node = node->right;
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
        return (node = node->left) ? is(node, func) : 0;
    }
    if ((query >= arrayOfItems) && (query <= arrayOfNulls))
    {
        if (node->type != JSON_ARRAY)
        {
            return 0;
        }
        return (node = node->left) ? is(node, func) : 0;
    }
    if ((query >= objectOfOptionalItems) && (query <= objectOfOptionalNulls))
    {
        if (node->type != JSON_OBJECT)
        {
            return 0;
        }
        return (node = node->left) ? is(node, func) : 1;
    }
    if ((query >= arrayOfOptionalItems) && (query <= arrayOfOptionalNulls))
    {
        if (node->type != JSON_ARRAY)
        {
            return 0;
        }
        return (node = node->left) ? is(node, func) : 1;
    }
    if ((query >= objectOfUniqueItems) && (query <= objectOfUniqueNulls))
    {
        if (node->type != JSON_OBJECT)
        {
            return 0;
        }
        return (node = node->left) ? is_unique(node, func) : 1;
    }
    if ((query >= arrayOfUniqueItems) && (query <= arrayOfUniqueNulls))
    {
        if (node->type != JSON_ARRAY)
        {
            return 0;
        }
        return (node = node->left) ? is_unique(node, func) : 1;
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

long json_integer(const json *node)
{
    if ((node == NULL) || (node->value == NULL))
    {
        return 0;
    }
    return strtol(node->value, NULL, 10);
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

unsigned long json_real(const json *node)
{
    if ((node == NULL) || (node->value == NULL))
    {
        return 0;
    }
    return strtoul(node->value, NULL, 10);
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

int json_is_double(const json *node)
{
    return (node != NULL)
        && (node->type == JSON_DOUBLE);
}

int json_is_number(const json *node)
{
    return !(node == NULL)
        && ((node->type == JSON_INTEGER) ||
            (node->type == JSON_DOUBLE));
}

int json_is_real(const json *node)
{
    return (node != NULL)
        && (node->type == JSON_INTEGER)
        && (strtol(node->value, NULL, 10) >= 0);
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

json *json_self(const json *node)
{
    /* Silence compiler due to const to non-const conversion */
    union {const json *constant; json *not_constant;} cast_to = {node};

    (void) cast_to.constant;
    return cast_to.not_constant;
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
    return node->left;
}

json *json_next(const json *node)
{
    if (node == NULL)
    {
        return NULL;
    }
    return node->right;
}

/* Locates a child node by name */
json *json_find(const json *root, const char *name)
{
    json *node;

    if ((root != NULL) && (node = root->left))
    {
        while (node != NULL)
        {
            if ((node->name != NULL) && (strcmp(node->name, name) == 0))
            {
                return node;
            }
            node = node->right;
        }
    }
    return NULL;
}

/* Locates the next sibling by name */
json *json_find_next(const json *root, const char *name)
{
    json *node;

    if ((root != NULL) && (node = root->right))
    {
        while (node != NULL)
        {
            if ((node->name != NULL) && (strcmp(node->name, name) == 0))
            {
                return node;
            }
            node = node->right;
        }
    }
    return NULL;
}

/*
 * Locates a child node by name given a name length
 * Useful to stop comparing when there is more text after the name
 */
json *json_match(const json *root, const char *name, size_t length)
{
    json *node;

    if ((root != NULL) && (node = root->left))
    {
        while (node != NULL)
        {
            if ((node->name != NULL) &&
                (strncmp(node->name, name, length) == 0) &&
                (node->name[length] == '\0'))
            {
                return node;
            }
            node = node->right;
        }
    }
    return NULL;
}

/* Locates a node by path */
json *json_node(const json *root, const char *path)
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
        if ((node->type == JSON_ARRAY) && (strspn(path, "0123456789") == length))
        {
            node = json_item(node, strtoul(path, NULL, 10));
        }
        /* . Current node */
        else if ((length == 1) && (path[0] == '.'))
        {
            /* noop */
        }
        /* .. Parent node */
        else if ((length == 2) && (path[0] == '.') && (path[1] == '.'))
        {
            node = node->parent;
        }
        /* Locate by name */
        else
        {
            node = json_match(node, path, length);
        }
        /* Adjust pointer to path */
        path = (*end == '\0') ? end : end + 1;
    }
    return node;
}

/* Locates an item by offset */
json *json_item(const json *root, size_t item)
{
    json *node;

    if ((root != NULL) && (node = root->left))
    {
        size_t count = 0;

        while (node != NULL)
        {
            if (count++ == item)
            {
                return node;
            }
            node = node->right;
        }
    }
    return NULL;
}

size_t json_items(const json *node)
{
    size_t count = 0;

    if ((node != NULL) && (node = node->left))
    {
        while (node != NULL)
        {
            node = node->right;
            count++;
        }
    }
    return count;
}

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

static int equal(const json *a, const json *b, int depth)
{
    if (a->type != b->type)
    {
        return 0;
    }
    if ((a->left == NULL) ^ (b->left == NULL))
    {
        return 0;
    }
    if (depth > 0)
    {
        if ((a->right == NULL) ^ (b->right == NULL))
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
        if (a->left != NULL)
        {
            a = a->left;
            b = b->left;
            depth++;
        }
        else if ((depth > 0) && (a->right != NULL))
        {
            a = a->right;
            b = b->right;
        }
        else
        {
            while (depth-- > 0)
            {
                a = a->parent;
                b = b->parent;
                if (a->right != NULL)
                {
                    a = a->right;
                    b = b->right;
                    goto end;
                }
            }
            return 1;
        }
        end:;
    }
    return 0;
}

/*
 * Sends all nodes to a callback "func"
 * Exit when all nodes are read or when "func" returns a non 0 value
 */
int json_traverse(const json *node, json_callback func, void *data)
{
    int depth = 0;
    int result;

    while (node != NULL)
    {
        loop:
        if ((result = func(node, depth, data)))
        {
            return result;
        }
        if (node->left != NULL)
        {
            node = node->left;
            depth++;
        }
        else if ((depth > 0) && (node->right != NULL))
        {
            node = node->right;
        }
        else
        {
            while (depth-- > 0)
            {
                node = node->parent;
                if (node->right != NULL)
                {
                    node = node->right;
                    goto loop;
                }
            }
            break;
        }
    }
    return 0;
}

