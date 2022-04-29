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

#define REQUIRED                    (1u << 0u)
#define NOT_ADDITIONAL_PROPERTIES   (1u << 1u)
#define UNIQUE_ITEMS                (1u << 2u)
#define EXCLUSIVE_MINIMUM           (1u << 3u)
#define EXCLUSIVE_MAXIMUM           (1u << 4u)

typedef struct
{
    const json *node, *properties, *items;
    const json *required, *dependent_required;
    const char *min_properties, *max_properties;
    const char *min_items, *max_items;
    const char *min_length, *max_length;
    const char *minimum, *maximum, *multiple_of;
    const char *pattern;
    schema_format format;
    unsigned type, flags;
} schema;

struct properties
{
    const json *parent, *node;
    struct properties *next;
};

static int is_named_object(const json *node)
{
    return json_name(node) && json_is_object(node);
}

static int is_named_array(const json *node)
{
    return json_name(node) && json_is_array(node);
}

static int unique(const json *node,
    int (*test_func)(const json *), const char *(*comp_func)(const json *))
{
    const json *head = node = json_child(node);

    while (node != NULL)
    {
        if (!test_func(node))
        {
            return 0;
        }
        for (const json *item = head; item != node; item = json_next(item))
        {
            if (equal(comp_func(item), comp_func(node)))
            {
                return 0;
            }
        }
        node = json_next(node);
    }
    return 1;
}

static int unique_objects(const json *node)
{
    return unique(node, is_named_object, json_name);
}

static int unique_arrays(const json *node)
{
    return unique(node, is_named_array, json_name);
}

static int unique_strings(const json *node)
{
    return unique(node, json_is_string, json_string);
}

static int add_type(schema *data, const char *type)
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
            return 1;
        }
    }
    return 0;
}

static int set_type(const json *node, schema *data)
{
    data->type = 0;
    if (json_is_string(node))
    {
        return add_type(data, json_string(node));
    }
    if (json_is_array(node) && unique_strings(node))
    {
        node = json_child(node);
        while (node != NULL)
        {
            if (!add_type(data, json_string(node)))
            {
                return 0;
            }
            node = json_next(node);
        }
    }
    return data->type != 0;
}

static void set_flag(schema *data, unsigned flag, int value)
{
    if (value)
    {
        data->flags |= flag;
    }
    else
    {
        data->flags &= ~flag;
    }
}

static int set_required(const json *node, schema *data)
{
    if (json_is_boolean(node))
    {
        set_flag(data, REQUIRED, json_boolean(node));
        data->required = NULL;
        return 1;
    }
    if (json_is_array(node) && unique_strings(node))
    {
        set_flag(data, REQUIRED, 0);
        data->required = json_child(node);
        return 1;
    }
    return 0;
}

static int set_dependent_required(const json *node, schema *data)
{
    if (json_is_object(node) && unique_arrays(node))
    {
        const json *item = node = json_child(node);

        while (item != NULL)
        {
            if (!unique_strings(item))
            {
                return 0;
            }
            item = json_next(item);
        }
        data->dependent_required = node;
        return 1;
    }
    return 0;
}

static int set_properties(const json *node, schema *data)
{
    if (json_is_object(node) && unique_objects(node))
    {
        data->properties = json_child(node);
        return 1;
    }
    return 0;
}

static int set_additional_properties(const json *node, schema *data)
{
    if (json_is_boolean(node))
    {
        set_flag(data, NOT_ADDITIONAL_PROPERTIES, !json_boolean(node));
        return 1;
    }
    return 0;
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
    if (json_is_object(node))
    {
        data->items = json_child(node) ? node : NULL;
        return 1;
    }
    return 0;
}

static int set_unique_items(const json *node, schema *data)
{
    if (json_is_boolean(node))
    {
        set_flag(data, UNIQUE_ITEMS, json_boolean(node));
        return 1;
    }
    return 0;
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
    if (json_is_boolean(node))
    {
        set_flag(data, EXCLUSIVE_MINIMUM, json_boolean(node));
        return 1;
    }
    return 0;
}

static int set_exclusive_maximum(const json *node, schema *data)
{
    if (json_is_boolean(node))
    {
        set_flag(data, EXCLUSIVE_MAXIMUM, json_boolean(node));
        return 1;
    }
    return 0;
}

static int set_multiple_of(const json *node, schema *data)
{
    if (json_is_number(node) && (json_number(node) > 0.0))
    {
        data->multiple_of = json_string(node);
        return 1;
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

static int test_is_array(const json *node, schema *data)
{
    (void)data;
    return json_is_array(node);
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
    if (data->type)
    {
        /* Since "integer" is not a json type, an extra test is needed */
        int match = ((data->type & (1u << type)) ||
                    ((data->type & (1u << 0)) && json_is_integer(data->node)));

        if (!match)
        {
            fprintf(stderr, "Error testing 'type'\n");
            return 0;
        }
    }
    return 1;
}

static int test_required(schema *data)
{
    if (data->flags & REQUIRED)
    {
        if (data->node == NULL)
        {
            fprintf(stderr, "Error testing 'required'\n");
            return 0;
        }
    }
    else if (data->required)
    {
        const json *node = data->required;

        while (node != NULL)
        {
            if (!json_pair(data->node, json_string(node)))
            {
                fprintf(stderr, "'%s' is required\n", json_string(node));
                return 0;
            }
            node = json_next(node);
        }
    }
    if (data->dependent_required)
    {
        const json *node = data->dependent_required;;

        while (node != NULL)
        {
            if (json_pair(data->node, json_name(node)))
            {
                const json *item = json_child(node);

                while (item != NULL)
                {
                    if (!json_pair(data->node, json_string(item)))
                    {
                        fprintf(stderr, "'%s' is required when '%s' is set\n",
                                json_string(item), json_name(node));
                        return 0;
                    }
                    item = json_next(item);
                }
            }
            node = json_next(node);
        }
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
    if (data->properties && (data->flags & NOT_ADDITIONAL_PROPERTIES))
    {
        const json *node = json_child(data->node);

        while (node != NULL)
        {
            const json *property = data->properties;
            int found = 0;

            while (property != NULL)
            {
                if (equal(json_name(node), json_name(property)))
                {
                    found = 1;
                    break;
                }
                property = json_next(property);
            }
            if (!found)
            {
                fprintf(stderr, "'%s' was not expected\n", json_name(node));
                return 0;
            }
            node = json_next(node);
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

static int test_string(schema *data)
{
    if (data->min_length || data->max_length)
    {
        size_t min = data->min_length ? real(data->min_length) : 0;
        size_t max = data->max_length ? real(data->max_length) : 0;
        const char *str = json_string(data->node);
        size_t length = 0;

        while (*str != 0)
        {
            if ((*str & 0xc0) != 0x80)
            {
                length++;
            }
            str++;
        }
        if (data->min_length && (min > length))
        {
            fprintf(stderr, "Error testing 'minLength'\n");
            return 0;
        }
        if (data->max_length && (max < length))
        {
            fprintf(stderr, "Error testing 'maxLength'\n");
            return 0;
        }
    }
    if (data->format)
    {
        if (!data->format(json_string(data->node)))
        {
            fprintf(stderr, "Error testing 'format'\n");
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

static int test_data(schema *data)
{
    if (!test_required(data))
    {
        return 0;
    }
    if (data->node == NULL)
    {
        return 1;
    }

    unsigned type = json_type(data->node);

    if (!test_type(data, type))
    {
        return 0;
    }
    switch (type)
    {
        case JSON_OBJECT:
            return test_properties(data);
        case JSON_ARRAY:
            return test_items(data);
        case JSON_STRING:
            return test_string(data);
        case JSON_NUMBER:
            return test_number(data);
        default:
            return 1;
    }
}

typedef int (*schema_setter)(const json *, schema *);

static schema_setter get_setter(const char *name)
{
    return
        equal(name, "$id") ? test_is_string :
        equal(name, "$schema") ? test_is_string :
        equal(name, "title") ? test_is_string :
        equal(name, "description") ? test_is_string :
        equal(name, "type") ? set_type :
        equal(name, "required") ? set_required :
        equal(name, "dependentRequired") ? set_dependent_required :
        equal(name, "properties") ? set_properties :
        equal(name, "additionalProperties") ? set_additional_properties :
        equal(name, "minProperties") ? set_min_properties :
        equal(name, "maxProperties") ? set_max_properties :
        equal(name, "items") ? set_items :
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
        equal(name, "examples") ? test_is_array :
        equal(name, "default") ? test_true : NULL;
}

static struct properties *get_properties(struct properties *properties,
    schema *data)
{
    if (data->properties != NULL)
    {
        struct properties *next = calloc(1, sizeof *next);

        if (next == NULL)
        {
            return 0;
        }
        next->parent = data->node;
        next->node = data->properties;
        next->next = properties;
        properties = next;
    }
    else
    {
        while (properties != NULL)
        {
            properties->node = json_next(properties->node);
            if (properties->node == NULL)
            {
                struct properties *next = properties->next;

                free(properties);
                properties = next;
            }
            else
            {
                break;
            }
        }
    }
    return properties;
}

static void clean_properties(struct properties *properties)
{
    while (properties != NULL)
    {
        struct properties *next = properties->next;

        free(properties);
        properties = next;
    }
}

static int test_schema(const json *node, schema *data)
{
    struct properties *properties = NULL;
    const char *name;
    int valid = 1;

    for (;;)
    {
        if (node == NULL)
        {
            if (!test_data(data))
            {
                valid = 0;
                break;
            }
            properties = get_properties(properties, data);
            if (properties == NULL)
            {
                break;
            }
            memset(data, 0, sizeof *data);
            node = json_child(properties->node);
            name = json_name(properties->node);
            data->node = json_pair(properties->parent, name);
        }
        if ((name = json_name(node)))
        {
            schema_setter setter = get_setter(name);

            if (setter != NULL)
            {
                if (!setter(node, data))
                {
                    fprintf(stderr, "Error setting '%s'\n", name);
                    valid = 0;
                    break;
                }
            }
            else
            {
                fprintf(stderr, "Unknown instance: '%s'\n", name);
            }
        }
        node = json_next(node);
    }
    clean_properties(properties);
    return valid;
}

static int test(const json *node, schema *data)
{
    if (!json_is_object(node))
    {
        fprintf(stderr, "Invalid schema\n");
        return 0;
    }
    if ((node = json_child(node)))
    {
        return test_schema(node, data);
    }
    return 1;
}

int json_validate(const json *node, const json *rules)
{
    schema data = {.node = node};

    return test(rules, &data);
}

