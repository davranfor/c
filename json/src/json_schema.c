/*! 
 *  \brief     JSON Schema validator
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "json.h"
#include "json_format.h"
#include "json_schema.h"

#define equal(a, b) (strcmp(a, b) == 0)

typedef struct
{
    json_callback callback;
    void *data;
} json_schema;

enum
{
    SCHEMA_INVALID, SCHEMA_VALID, SCHEMA_ERROR,
    SCHEMA_OBJECT, SCHEMA_ARRAY, SCHEMA_TUPLE
};

static void schema_callback(const json_schema *schema,
    const json *node, const json *iter, const char *title)
{
    if (schema->callback)
    {
        fprintf(stderr, "%s:\n", title);
        schema->callback(iter, schema->data);
        json_write(stderr, node);
    }
}

static void raise_invalid(const json_schema *schema,
    const json *node, const json *iter)
{
    schema_callback(schema, node, iter, "Invalid");
}

static void raise_warning(const json_schema *schema,
    const json *node, const json *iter)
{
    schema_callback(schema, node, iter, "Warning");
}

static void raise_error(const json_schema *schema,
    const json *node, const json *iter)
{
    schema_callback(schema, node, iter, "Error");
}

static int childs_are(const json *node, enum json_type type)
{
    for (node = json_child(node); node != NULL; node = json_next(node))
    {
        if (json_type(node) != type)
        {
            return 0;
        }
    }
    return 1;
}

static int childs_are_objects(const json *node)
{
    return childs_are(node, JSON_OBJECT);
}

static int unique(const json *node, enum json_type type,
    const char *(*comp)(const json *))
{
    const json *head = json_child(node);

    for (node = head; node != NULL; node = json_next(node))
    {
        if (json_type(node) != type)
        {
            return 0;
        }
        for (const json *item = head; item != node; item = json_next(item))
        {
            if (equal(comp(node), comp(item)))
            {
                return 0;
            }
        }
    }
    return 1;
}

static int unique_objects(const json *node)
{
    return unique(node, JSON_OBJECT, json_name);
}

static int unique_strings(const json *node)
{
    return unique(node, JSON_STRING, json_string);
}

static int test_error(const json *node, const json *iter)
{
    (void)node;
    (void)iter;
    return SCHEMA_ERROR;
}

static int test_valid(const json *node, const json *iter)
{
    (void)node;
    (void)iter;
    return SCHEMA_VALID;
}

static int test_is_array(const json *node, const json *iter)
{
    (void)iter;
    return json_is_array(node) ? SCHEMA_VALID : SCHEMA_ERROR;
}

static int test_is_string(const json *node, const json *iter)
{
    (void)iter;
    return json_is_string(node) ? SCHEMA_VALID : SCHEMA_ERROR;
}

static int test_is_boolean(const json *node, const json *iter)
{
    (void)iter;
    return json_is_boolean(node) ? SCHEMA_VALID : SCHEMA_ERROR;
}

static unsigned add_type(const char *type, unsigned value)
{
    static const char *types[] =
    {
        "object", "array", "string", "integer", "number", "boolean", "null"
    };
    size_t size = sizeof(types) / sizeof(char *);

    for (size_t item = 0; item < size; item++)
    {
        if (equal(type, types[item]))
        {
            return value | (1u << (item + 1));
        }
    }
    return 0;
}

static int test_type(const json *node, const json *iter)
{
    unsigned mask = 0;

    if (json_is_string(node))
    {
        mask = add_type(json_string(node), mask);
    }
    else if (json_is_array(node) && unique_strings(node))
    {
        for (node = json_child(node); node != NULL; node = json_next(node))
        {
            if ((mask = add_type(json_string(node), mask)) == 0)
            {
                break;
            }
        }
    }
    if (mask == 0)
    {
        return SCHEMA_ERROR;
    }
    if (iter != NULL)
    {
        unsigned type = json_type(iter);

        // 'integer' must validate if type is 'number'
        return (mask & (1u << type))
           || ((mask & (1u << JSON_DOUBLE)) && (type == JSON_INTEGER));
    }
    return 1;
}

static int test_const(const json *node, const json *iter)
{
    if ((iter != NULL) && !json_equal(node, iter))
    {
        return 0;
    }
    return 1;
}

static int test_enum(const json *node, const json *iter)
{
    if (!json_is_array(node))
    {
        return SCHEMA_ERROR;
    }
    if (iter != NULL)
    {
        for (node = json_child(node); node != NULL; node = json_next(node))
        {
            if (json_equal(node, iter))
            {
                return 1;
            }
        }
        return 0;
    }
    return 1;
}

static int test_required(const json *node, const json *iter)
{
    if (json_is_array(node) && unique_strings(node))
    {
        if (json_is_object(iter))
        {
            for (node = json_child(node); node != NULL; node = json_next(node))
            {
                if (!json_find(iter, json_string(node)))
                {
                    return 0;
                }
            }
        }
        return 1;
    }
    return SCHEMA_ERROR;
}

static int test_dependent_required(const json *node, const json *iter)
{
    if (!json_is_object(node))
    {
        return SCHEMA_ERROR;
    }
    if (json_is_object(iter))
    {
        for (node = json_child(node); node != NULL; node = json_next(node))
        {
            if (!(json_is_array(node) && unique_strings(node)))
            {
                return SCHEMA_ERROR;
            }
            if (json_find(iter, json_name(node)))
            {
                const json *item = json_child(node);

                while (item != NULL)
                {
                    if (!json_find(iter, json_string(item)))
                    {
                        return 0;
                    }
                    item = json_next(item);
                }
            }
        }
    }
    return 1;
}

static int test_properties(const json *node, const json *iter)
{
    if (!(json_is_object(node) && unique_objects(node)))
    {
        return SCHEMA_ERROR;
    }
    if (json_is_object(iter))
    {
        return SCHEMA_OBJECT;
    }
    return 1;
}

static int test_additional_properties(const json *node, const json *iter)
{
    if (!json_is_boolean(node))
    {
        return SCHEMA_ERROR;
    }
    if (!json_boolean(node) && json_is_object(iter))
    {
        const json *properties = json_find(json_parent(node), "properties");

        if (properties != NULL)
        {
            for (iter = json_child(iter); iter != NULL; iter = json_next(iter))
            {
                if (!json_find(properties, json_name(iter)))
                {
                    return 0;
                }
            }
        }
    }
    return 1;
}

static int test_min_properties(const json *node, const json *iter)
{
    if (!json_is_real(node))
    {
        return SCHEMA_ERROR;
    }
    if (json_is_object(iter))
    {
        return json_items(iter) >= json_real(node);
    }
    return 1;
}

static int test_max_properties(const json *node, const json *iter)
{
    if (!json_is_real(node))
    {
        return SCHEMA_ERROR;
    }
    if (json_is_object(iter))
    {
        return json_items(iter) <= json_real(node);
    }
    return 1;
}

static int test_items(const json *node, const json *iter)
{
    if (!json_is_object(node) &&
        !(json_is_array(node) && childs_are_objects(node)))
    {
        return SCHEMA_ERROR;
    }
    if (json_is_array(iter))
    {
        if (json_is_object(node))
        {
            return SCHEMA_ARRAY;
        }
        else
        {
            return SCHEMA_TUPLE;
        }
    }
    return 1;
}

static int test_additional_items(const json *node, const json *iter)
{
    if (!json_is_boolean(node))
    {
        return SCHEMA_ERROR;
    }
    if (!json_boolean(node) && json_is_array(iter))
    {
        const json *items = json_find(json_parent(node), "items");

        if (json_is_array(items))
        {
            size_t tuples = json_items(items);

            if (tuples > 0)
            {
                return json_items(iter) <= tuples;
            }
        }
    }
    return 1;
}

static int test_min_items(const json *node, const json *iter)
{
    if (!json_is_real(node))
    {
        return SCHEMA_ERROR;
    }
    if (json_is_array(iter))
    {
        return json_items(iter) >= json_real(node);
    }
    return 1;
}

static int test_max_items(const json *node, const json *iter)
{
    if (!json_is_real(node))
    {
        return SCHEMA_ERROR;
    }
    if (json_is_array(iter))
    {
        return json_items(iter) <= json_real(node);
    }
    return 1;
}

static int test_unique_items(const json *node, const json *iter)
{
    if (!json_is_boolean(node))
    {
        return SCHEMA_ERROR;
    }
    if (json_boolean(node) && json_is_array(iter))
    {
        const json *head = json_child(iter);

        for (iter = head; iter != NULL; iter = json_next(iter))
        {
            for (const json *item = head; item != iter; item = json_next(item))
            {
                if (json_equal(iter, item))
                {
                    return 0;
                }
            }
        }
    }
    return 1;
}

static size_t get_length(const char *str)
{
    size_t length = 0;

    while (*str != 0)
    {
        if ((*str & 0xc0) != 0x80)
        {
            length++;
        }
        str++;
    }
    return length;
}

static int test_min_length(const json *node, const json *iter)
{
    if (!json_is_real(node))
    {
        return SCHEMA_ERROR;
    }
    if (json_is_string(iter))
    {
        return get_length(json_string(iter)) >= json_real(node);
    }
    return 1;
}

static int test_max_length(const json *node, const json *iter)
{
    if (!json_is_real(node))
    {
        return SCHEMA_ERROR;
    }
    if (json_is_string(iter))
    {
        return get_length(json_string(iter)) <= json_real(node);
    }
    return 1;
}

static int test_format(const json *node, const json *iter)
{
    if (!json_is_string(node))
    {
        return SCHEMA_ERROR;
    }
    if (json_is_string(iter))
    {
        const char *name = json_string(node);

        int (*format)(const char *) =
            equal(name, "date") ? test_is_date :
            equal(name, "time") ? test_is_time :
            equal(name, "date-time") ? test_is_date_time :
            equal(name, "email") ? test_is_email :
            equal(name, "ipv4") ? test_is_ipv4 :
            equal(name, "ipv6") ? test_is_ipv6 :
            equal(name, "uuid") ? test_is_uuid : NULL;

        if (format != NULL)
        {
            return format(json_string(iter));
        }
        else
        {
            return test_regex(name, json_string(iter));
        }
    }
    return 1;
}

static int test_pattern(const json *node, const json *iter)
{
    if (!json_is_string(node))
    {
        return SCHEMA_ERROR;
    }
    if (json_is_string(iter))
    {
        return test_regex(json_string(node), json_string(iter));
    }
    return 1;
}

static int test_minimum(const json *node, const json *iter)
{
    if (!json_is_number(node))
    {
        return SCHEMA_ERROR;
    }
    if (json_is_number(iter))
    {
        const json *exclusive = json_find(json_parent(node), "exclusiveMinimum");

        if (json_is_boolean(exclusive) && json_boolean(exclusive))
        {
            return json_number(iter) > json_number(node);
        }
        else
        {
            return json_number(iter) >= json_number(node);
        }
    }
    return 1;
}

static int test_maximum(const json *node, const json *iter)
{
    if (!json_is_number(node))
    {
        return SCHEMA_ERROR;
    }
    if (json_is_number(iter))
    {
        const json *exclusive = json_find(json_parent(node), "exclusiveMaximum");

        if (json_is_boolean(exclusive) && json_boolean(exclusive))
        {
            return json_number(iter) < json_number(node);
        }
        else
        {
            return json_number(iter) <= json_number(node);
        }
    }
    return 1;
}

static int test_multiple_of(const json *node, const json *iter)
{
    if (!json_is_number(node))
    {
        return SCHEMA_ERROR;
    }
    if (json_is_number(iter))
    {
        return fmod(json_number(iter), json_number(node)) == 0;
    }
    return 1;
}

typedef int (*tester)(const json *, const json *);

static tester get_test_by_name(const char *name)
{
    return
        equal(name, "$schema") ? test_is_string :
        equal(name, "$id") ? test_is_string :
        equal(name, "title") ? test_is_string :
        equal(name, "description") ? test_is_string :
        equal(name, "type") ? test_type :
        equal(name, "const") ? test_const :
        equal(name, "enum") ? test_enum :
        equal(name, "required") ? test_required :
        equal(name, "dependentRequired") ? test_dependent_required :
        equal(name, "properties") ? test_properties :
        equal(name, "additionalProperties") ? test_additional_properties :
        equal(name, "minProperties") ? test_min_properties :
        equal(name, "maxProperties") ? test_max_properties :
        equal(name, "items") ? test_items :
        equal(name, "additionalItems") ? test_additional_items :
        equal(name, "minItems") ? test_min_items :
        equal(name, "maxItems") ? test_max_items :
        equal(name, "uniqueItems") ? test_unique_items :
        equal(name, "minLength") ? test_min_length :
        equal(name, "maxLength") ? test_max_length :
        equal(name, "format") ? test_format :
        equal(name, "pattern") ? test_pattern :
        equal(name, "minimum") ? test_minimum :
        equal(name, "maximum") ? test_maximum :
        equal(name, "exclusiveMinimum") ? test_is_boolean :
        equal(name, "exclusiveMaximum") ? test_is_boolean :
        equal(name, "multipleOf") ? test_multiple_of :
        equal(name, "readOnly") ? test_is_boolean :
        equal(name, "writeOnly") ? test_is_boolean :
        equal(name, "deprecated") ? test_is_boolean :
        equal(name, "examples") ? test_is_array :
        equal(name, "default") ? test_valid : NULL;
}

static tester get_test(const json *node)
{
    const char *name = json_name(node);

    if (name == NULL)
    {
        return test_error;
    }
    return get_test_by_name(name);
}

static int valid_schema(json_schema *schema,
    const json *node, const json *iter)
{
    int valid = 1;

    while (node != NULL)
    {
        tester test = get_test(node);

        if (test != NULL)
        {
            switch (test(node, iter))
            {
                case SCHEMA_OBJECT:
                {
                    const json *next = json_child(node);

                    while (next != NULL)
                    {
                        const json *item = json_find(iter, json_name(next));

                        valid &= valid_schema(schema, json_child(next), item);
                        next = json_next(next);
                    }
                }
                break;
                case SCHEMA_ARRAY:
                {
                    const json *next = json_child(node);
                    const json *item = json_child(iter);

                    while (item != NULL)
                    {
                        valid &= valid_schema(schema, next, item);
                        item = json_next(item);
                    }
                }
                break;
                case SCHEMA_TUPLE:
                {
                    const json *next = json_child(node);
                    const json *item = json_child(iter);

                    while (next != NULL)
                    {
                        valid &= valid_schema(schema, json_child(next), item);
                        next = json_next(next);
                        item = json_next(item);
                    }
                }
                break;
                case SCHEMA_INVALID:
                {
                    raise_invalid(schema, node, iter);
                    valid = 0;
                }
                break;
                case SCHEMA_ERROR:
                {
                    raise_error(schema, node, iter);
                    valid = 0;
                }
                break;
            }
        }
        else
        {
            raise_warning(schema, node, iter);
        }
        node = json_next(node);
    }
    return valid;
}

int json_validate(const json *iter, const json *node,
    json_callback callback, void *data)
{
    json_schema schema =
    {
        .callback = callback,
        .data = data
    };

    if (!json_is_object(node))
    {
        raise_error(&schema, node, iter);
        return 0;
    }
    if ((node = json_child(node)))
    {
        return valid_schema(&schema, node, iter);
    }
    return 1;
}

int json_schema_print(const json *node, void *data)
{
    (void)data;
    json_write(stderr, node);
    return 1;
}

