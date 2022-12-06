#include "json.h"
#include "json_query.h"

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
    while (func(node))
    {
        node = json_next(node);
    }
    return node == NULL;
}

static int is_unique(const json *node, int (*func)(const json *))
{
    const json *head = node;

    while (func(node))
    {
        for (const json *item = head; item != node; item = json_next(item))
        {
            if (json_equal(node, item))
            {
                return 0;
            }
        }
        node = json_next(node);
    }
    return node == NULL;
}

int json_is(const json *node, enum json_query query)
{
    int (*func)(const json *) = func_array[query % func_size];

    if ((query >= objectOfItems) && (query <= objectOfNulls))
    {
        if (json_type(node) != JSON_OBJECT)
        {
            return 0;
        }
        return (node = json_child(node)) ? is(node, func) : 0;
    }
    if ((query >= arrayOfItems) && (query <= arrayOfNulls))
    {
        if (json_type(node) != JSON_ARRAY)
        {
            return 0;
        }
        return (node = json_child(node)) ? is(node, func) : 0;
    }
    if ((query >= objectOfOptionalItems) && (query <= objectOfOptionalNulls))
    {
        if (json_type(node) != JSON_OBJECT)
        {
            return 0;
        }
        return (node = json_child(node)) ? is(node, func) : 1;
    }
    if ((query >= arrayOfOptionalItems) && (query <= arrayOfOptionalNulls))
    {
        if (json_type(node) != JSON_ARRAY)
        {
            return 0;
        }
        return (node = json_child(node)) ? is(node, func) : 1;
    }
    if ((query >= objectOfUniqueItems) && (query <= objectOfUniqueNulls))
    {
        if (json_type(node) != JSON_OBJECT)
        {
            return 0;
        }
        return (node = json_child(node)) ? is_unique(node, func) : 1;
    }
    if ((query >= arrayOfUniqueItems) && (query <= arrayOfUniqueNulls))
    {
        if (json_type(node) != JSON_ARRAY)
        {
            return 0;
        }
        return (node = json_child(node)) ? is_unique(node, func) : 1;
    }
    return 0;
}

