/*! 
 *  \brief     JSON Schema validator
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "json.h"
#include "json_format.h"
#include "json_schema.h"

#define number(str) strtod(str, NULL)
#define real(str) strtoul(str, NULL, 10)
#define equal(a, b) (strcmp(a, b) == 0)

#define ADDITIONAL_PROPERTIES   (1u << 0u)
#define UNIQUE_ITEMS            (1u << 1u)
#define EXCLUSIVE_MINIMUM       (1u << 2u)
#define EXCLUSIVE_MAXIMUM       (1u << 3u)

typedef struct
{
    const json *node, *path;
    const json *properties, *items;
    const json *required, *dependent_required;
    const char *min_properties, *max_properties;
    const char *min_items, *max_items;
    const char *min_length, *max_length;
    const char *minimum, *maximum, *multiple_of;
    const char *pattern;
    schema_format format;
    unsigned type, flags;
} schema;

static int set_flag(const json *node, schema *data, unsigned flag)
{
    if (json_is_boolean(node))
    {
        if (json_boolean(node))
        {
            data->flags |= flag;
        }
        else
        {
            data->flags &= ~flag;
        }
        return 1;
    }
    return 0;
}

static int unique_strings(const json *node)
{
    const json *first = node;

    while (node != NULL)
    {
        if (json_is_property(node) || !json_is_string(node))
        {
            return 0;
        }
        for (const json *item = first; item != node; item = json_next(item))
        {
            if (equal(json_string(item), json_string(node)))
            {
                return 0;
            }
        }
        node = json_next(node);
    }
    return 1;
}

static void add_type(schema *data, const char *type)
{
    static const char *types[] =
    {
        "integer", "object", "array", "string", "number", "boolean", "null"
    };
    size_t size = sizeof(types) / sizeof(char *);

    for (size_t item = 0; item < size; item++)
    {
        if (equal(type, types[item]))
        {
            data->type |= 1u << item;
            return;
        }
    }
}

static int set_type(const json *node, schema *data)
{
    data->type = 0;
    if (json_is_string(node))
    {
        add_type(data, json_string(node));
    }
    else if ((json_is_array(node)) && (node = json_child(node)))
    {
        if (unique_strings(node))
        {
            while (node != NULL)
            {
                add_type(data, json_string(node));
                node = json_next(node);
            }
        }
    }
    return data->type != 0;
}

static int set_required(const json *node, schema *data)
{
    if (json_is_array(node) && unique_strings(json_child(node)))
    {
        data->required = node;
        return 1;
    }
    return 0;
}

static int set_dependent_required(const json *node, schema *data)
{
    if (json_type(node) == JSON_ARRAY)
    {
        if (unique_strings(json_child(node)))
        {
            data->dependent_required = node;
            return 1;
        }
    }
    return 0;
}

static int set_properties(const json *node, schema *data)
{
    data->properties = json_child(node) ? node : NULL;
    return 1;
}

static int set_additional_properties(const json *node, schema *data)
{
    return set_flag(node, data, ADDITIONAL_PROPERTIES);
}

static int set_min_properties(const json *node, schema *data)
{
    if (json_is_real(node))
    {
        data->min_properties = json_string(node);
        return 1;
    }
    return 0;
}

static int set_max_properties(const json *node, schema *data)
{
    if (json_is_real(node))
    {
        data->max_properties = json_string(node);
        return 1;
    }
    return 0;
}

static int set_items(const json *node, schema *data)
{
    data->items = json_child(node) ? node : NULL;
    return 1;
}

static int set_unique_items(const json *node, schema *data)
{
    return set_flag(node, data, UNIQUE_ITEMS);
}

static int set_min_items(const json *node, schema *data)
{
    if (json_is_real(node))
    {
        data->min_items = json_string(node);
        return 1;
    }
    return 0;
}

static int set_max_items(const json *node, schema *data)
{
    if (json_is_real(node))
    {
        data->max_items = json_string(node);
        return 1;
    }
    return 0;
}

static int set_min_length(const json *node, schema *data)
{
    if (json_is_real(node))
    {
        data->min_length = json_string(node);
        return 1;
    }
    return 0;
}

static int set_max_length(const json *node, schema *data)
{
    if (json_is_real(node))
    {
        data->max_length = json_string(node);
        return 1;
    }
    return 0;
}

static int set_minimum(const json *node, schema *data)
{
    if (json_is_number(node))
    {
        data->minimum = json_string(node);
        return 1;
    }
    return 0;
}

static int set_maximum(const json *node, schema *data)
{
    if (json_is_number(node))
    {
        data->maximum = json_string(node);
        return 1;
    }
    return 0;
}

static int set_exclusive_minimum(const json *node, schema *data)
{
    return set_flag(node, data, EXCLUSIVE_MINIMUM);
}

static int set_exclusive_maximum(const json *node, schema *data)
{
    return set_flag(node, data, EXCLUSIVE_MAXIMUM);
}

static int set_multiple_of(const json *node, schema *data)
{
    if (json_is_number(node))
    {
        if (json_number(node) > 0.0)
        {
            data->multiple_of = json_string(node);
            return 1;
        }
    }
    return 0;
}

static int set_format(const json *node, schema *data)
{
    if (json_is_string(node))
    {
        const char *format = json_string(node);

        data->format =
            equal(format, "date") ? test_is_date :
            equal(format, "time") ? test_is_time :
            equal(format, "date-time") ? test_is_date_time :
            equal(format, "email") ? test_is_email :
            equal(format, "ipv4") ? test_is_ipv4 :
            equal(format, "ipv6") ? test_is_ipv6 :
            equal(format, "uuid") ? test_is_uuid : NULL;

        if (data->format == NULL)
        {
            data->pattern = format;
        }
        return 1;
    }
    return 0;
}

static int set_pattern(const json *node, schema *data)
{
    if (json_is_string(node))
    {
        data->pattern = json_string(node);
        return 1;
    }
    return 0;
}

static int test_true(const json *node, schema *data)
{
    (void)node;
    (void)data;
    return 1;
}

static int test_is_string(const json *node, schema *data)
{
    (void)data;
    return json_is_string(node);
}

static int test_is_boolean(const json *node, schema *data)
{
    (void)data;
    return json_is_boolean(node);
}

static int test_type(schema *data, unsigned type)
{
    int valid = (data->type & (1u << type)) != 0;

    if (valid || ((data->type & 1u) && json_is_integer(data->node)))
    {
        return 1;
    }
    fprintf(stderr, "Error testing 'type'\n");
    return 0;
}

static int test_required(schema *data)
{
    if (!data->properties)
    {
        return 0;
    }

    const json *item = json_child(data->required);

    while (item != NULL)
    {
        if (!json_pair(data->node, json_string(item)))
        {
            fprintf(stderr, "Field '%s' is 'required'\n", json_string(item));
            return 0;
        }
        item = json_next(item);
    }
    return 1;
}

static int test_properties(schema *data)
{
    if (data->min_properties || data->max_properties)
    {
        unsigned long properties = json_items(data->node);

        if (data->min_properties && (properties < real(data->min_properties)))
        {
            fprintf(stderr, "Error testing 'minProperties'\n");
            return 0;
        }
        if (data->max_properties && (properties > real(data->max_properties)))
        {
            fprintf(stderr, "Error testing 'maxProperties'\n");
            return 0;
        }
    }
    return 1;
}

static int test_items(schema *data)
{
    if (data->min_items || data->max_items)
    {
        unsigned long items = json_items(data->node);

        if (data->min_items && (items < real(data->min_items)))
        {
            fprintf(stderr, "Error testing 'minItems'\n");
            return 0;
        }
        if (data->max_items && (items > real(data->max_items)))
        {
            fprintf(stderr, "Error testing 'maxItems'\n");
            return 0;
        }
    }
    return 1;
}

static int test_minimum(schema *data, double value)
{
    if (data->flags & EXCLUSIVE_MINIMUM)
    {
        return value > number(data->minimum);
    }
    else
    {
        return value >= number(data->minimum);
    }
}

static int test_maximum(schema *data, double value)
{
    if (data->flags & EXCLUSIVE_MAXIMUM)
    {
        return value < number(data->maximum);
    }
    else
    {
        return value <= number(data->maximum);
    }
}

static int test_number(schema *data)
{
    if (data->minimum || data->maximum || data->multiple_of)
    {
        double value = json_number(data->node);

        if (data->minimum && !test_minimum(data, value))
        {
            fprintf(stderr, "Error testing 'minimum'\n");
            return 0;
        }
        if (data->maximum && !test_maximum(data, value))
        {
            fprintf(stderr, "Error testing 'maximum'\n");
            return 0;
        }
        if (data->multiple_of && fmod(value, number(data->multiple_of)))
        {
            fprintf(stderr, "Error testing 'multipleOf'\n");
            return 0;
        }
    }
    return 1;
}

static int test_format(schema *data)
{
    int valid = data->format(json_string(data->node));

    if (!valid)
    {
        fprintf(stderr, "Error testing 'format'\n");
    }
    return valid;
}

static int test_data(schema *data)
{
    unsigned type = json_type(data->node);

    if (data->type && !test_type(data, type))
    {
        return 0;
    }
    if (data->required && !test_required(data))
    {
        return 0;
    }
    switch (type)
    {
        case JSON_OBJECT:
            if (!test_properties(data))
            {
                return 0;
            }
            break;
        case JSON_ARRAY:
            if (!test_items(data))
            {
                return 0;
            }
            break;
        case JSON_NUMBER:
            if (!test_number(data))
            {
                return 0;
            }
            break;
        default:
            break;
    }
    switch (type)
    {
        case JSON_OBJECT:
        case JSON_ARRAY:
            break;
        default:
            if (data->format && !test_format(data))
            {
                return 0;
            }
            break;
    }
    return 1;
}

typedef int (*schema_setter)(const json *, schema *);

static schema_setter get_setter(const json *node, const char *name)
{
    if (json_string(node))
    {
        return
            equal(name, "$id") ? test_is_string :
            equal(name, "$schema") ? test_is_string :
            equal(name, "title") ? test_is_string :
            equal(name, "description") ? test_is_string :
            equal(name, "type") ? set_type :
            equal(name, "additionalProperties") ? set_additional_properties :
            equal(name, "minProperties") ? set_min_properties :
            equal(name, "maxProperties") ? set_max_properties :
            equal(name, "uniqueItems") ? set_unique_items :
            equal(name, "minItems") ? set_min_items :
            equal(name, "maxItems") ? set_max_items :
            equal(name, "minLength") ? set_min_length :
            equal(name, "maxLength") ? set_max_length :
            equal(name, "minimum") ? set_minimum :
            equal(name, "maximum") ? set_maximum :
            equal(name, "exclusiveMinimum") ? set_exclusive_minimum :
            equal(name, "exclusiveMaximum") ? set_exclusive_maximum :
            equal(name, "multipleOf") ? set_multiple_of :
            equal(name, "format") ? set_format :
            equal(name, "pattern") ? set_pattern :
            equal(name, "readOnly") ? test_is_boolean :
            equal(name, "writeOnly") ? test_is_boolean :
            equal(name, "deprecated") ? test_is_boolean :
            equal(name, "default") ? test_true : NULL;
    }
    else if (json_is_object(node))
    {
        return
            equal(name, "properties") ? set_properties :
            equal(name, "items") ? set_items : NULL;
    }
    else if (json_is_array(node))
    {
        return
            equal(name, "type") ? set_type :
            equal(name, "required") ? set_required :
            equal(name, "dependentRequired") ? set_dependent_required :
            equal(name, "examples") ? test_true : NULL;
    }
    return NULL;
}

static int test_schema(const json *node, schema *data)
{
    for (;;)
    {
        if (node == NULL)
        {
            test_data(data);
            break;
        }

        const char *name = json_name(node);

        /* if property */
        if (name != NULL)
        {
            json_print(node);

            schema_setter setter = get_setter(node, name);

            if (setter != NULL)
            {
                if (!setter(node, data))
                {
                    fprintf(stderr, "Error setting '%s'\n", name);
                    return 0;
                }
            }
            else
            {
                fprintf(stderr, "Unknown instance: '%s'\n", name);
            }
        }
        node = json_next(node);
    }
    return 1;
}

static int test(schema *data)
{
    if (!json_is_object(data->path))
    {
        fprintf(stderr, "Invalid schema\n");
        return 0;
    }
    if (data->node == NULL)
    {
        fprintf(stderr, "Nothing to validate\n");
        return 0;
    }

    const json *node = json_child(data->path);

    if (node != NULL)
    {
        return test_schema(node, data);
    }
    return 1;
}

int json_schema_validate(const json *node, const json *path)
{
    schema data = {.node = node, .path = path};

    return test(&data);
}

