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
    const char *min_length, *max_length, *pattern;
    const char *minimum, *maximum, *multiple_of;
    schema_format format;
    unsigned type, flags;
} json_schema;

typedef struct json_subschema
{
    const json *root, *node;
    struct json_subschema *next;
} json_subschema;

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

static int add_type(json_schema *schema, const char *type)
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
            schema->type |= 1u << item;
            return 1;
        }
    }
    return 0;
}

static int set_type(json_schema *schema, const json *node)
{
    schema->type = 0;
    if (json_is_string(node))
    {
        return add_type(schema, json_string(node));
    }
    if (json_is_array(node) && unique_strings(node))
    {
        node = json_child(node);
        while (node != NULL)
        {
            if (!add_type(schema, json_string(node)))
            {
                return 0;
            }
            node = json_next(node);
        }
    }
    return schema->type != 0;
}

static void set_flag(json_schema *schema, unsigned flag, int value)
{
    if (value)
    {
        schema->flags |= flag;
    }
    else
    {
        schema->flags &= ~flag;
    }
}

static int set_required(json_schema *schema, const json *node)
{
    if (json_is_boolean(node))
    {
        set_flag(schema, REQUIRED, json_boolean(node));
        schema->required = NULL;
        return 1;
    }
    if (json_is_array(node) && unique_strings(node))
    {
        set_flag(schema, REQUIRED, 0);
        schema->required = json_child(node);
        return 1;
    }
    return 0;
}

static int set_dependent_required(json_schema *schema, const json *node)
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
        schema->dependent_required = node;
        return 1;
    }
    return 0;
}

static int set_properties(json_schema *schema, const json *node)
{
    if (json_is_object(node) && unique_objects(node))
    {
        schema->properties = json_child(node);
        return 1;
    }
    return 0;
}

static int set_additional_properties(json_schema *schema, const json *node)
{
    if (json_is_boolean(node))
    {
        set_flag(schema, NOT_ADDITIONAL_PROPERTIES, !json_boolean(node));
        return 1;
    }
    return 0;
}

static int set_min_properties(json_schema *schema, const json *node)
{
    if (json_is_real(node))
    {
        schema->min_properties = json_string(node);
        return 1;
    }
    return 0;
}

static int set_max_properties(json_schema *schema, const json *node)
{
    if (json_is_real(node))
    {
        schema->max_properties = json_string(node);
        return 1;
    }
    return 0;
}

static int set_items(json_schema *schema, const json *node)
{
    if (json_is_object(node))
    {
        schema->items = json_child(node);
        return 1;
    }
    return 0;
}

static int set_unique_items(json_schema *schema, const json *node)
{
    if (json_is_boolean(node))
    {
        set_flag(schema, UNIQUE_ITEMS, json_boolean(node));
        return 1;
    }
    return 0;
}

static int set_min_items(json_schema *schema, const json *node)
{
    if (json_is_real(node))
    {
        schema->min_items = json_string(node);
        return 1;
    }
    return 0;
}

static int set_max_items(json_schema *schema, const json *node)
{
    if (json_is_real(node))
    {
        schema->max_items = json_string(node);
        return 1;
    }
    return 0;
}

static int set_min_length(json_schema *schema, const json *node)
{
    if (json_is_real(node))
    {
        schema->min_length = json_string(node);
        return 1;
    }
    return 0;
}

static int set_max_length(json_schema *schema, const json *node)
{
    if (json_is_real(node))
    {
        schema->max_length = json_string(node);
        return 1;
    }
    return 0;
}

static int set_minimum(json_schema *schema, const json *node)
{
    if (json_is_number(node))
    {
        schema->minimum = json_string(node);
        return 1;
    }
    return 0;
}

static int set_maximum(json_schema *schema, const json *node)
{
    if (json_is_number(node))
    {
        schema->maximum = json_string(node);
        return 1;
    }
    return 0;
}

static int set_exclusive_minimum(json_schema *schema, const json *node)
{
    if (json_is_boolean(node))
    {
        set_flag(schema, EXCLUSIVE_MINIMUM, json_boolean(node));
        return 1;
    }
    return 0;
}

static int set_exclusive_maximum(json_schema *schema, const json *node)
{
    if (json_is_boolean(node))
    {
        set_flag(schema, EXCLUSIVE_MAXIMUM, json_boolean(node));
        return 1;
    }
    return 0;
}

static int set_multiple_of(json_schema *schema, const json *node)
{
    if (json_is_number(node) && (json_number(node) > 0.0))
    {
        schema->multiple_of = json_string(node);
        return 1;
    }
    return 0;
}

static int set_format(json_schema *schema, const json *node)
{
    if (json_is_string(node))
    {
        const char *format = json_string(node);

        schema->format =
            equal(format, "date") ? test_is_date :
            equal(format, "time") ? test_is_time :
            equal(format, "date-time") ? test_is_date_time :
            equal(format, "email") ? test_is_email :
            equal(format, "ipv4") ? test_is_ipv4 :
            equal(format, "ipv6") ? test_is_ipv6 :
            equal(format, "uuid") ? test_is_uuid : NULL;

        if (schema->format == NULL)
        {
            schema->pattern = format;
        }
        return 1;
    }
    return 0;
}

static int set_pattern(json_schema *schema, const json *node)
{
    if (json_is_string(node))
    {
        schema->pattern = json_string(node);
        return 1;
    }
    return 0;
}

static int test_true(json_schema *schema, const json *node)
{
    (void)schema;
    (void)node;
    return 1;
}

static int test_is_array(json_schema *schema, const json *node)
{
    (void)schema;
    return json_is_array(node);
}

static int test_is_string(json_schema *schema, const json *node)
{
    (void)schema;
    return json_is_string(node);
}

static int test_is_boolean(json_schema *schema, const json *node)
{
    (void)schema;
    return json_is_boolean(node);
}

static int test_type(const json_schema *schema, unsigned type)
{
    if (schema->type)
    {
        /* Since "integer" is not a json type, an extra test is needed */
        int match = ((schema->type & (1u << type)) ||
                    ((schema->type & (1u << 0)) && json_is_integer(schema->node)));

        if (!match)
        {
            fprintf(stderr, "Error testing 'type'\n");
            return 0;
        }
    }
    return 1;
}

static int test_required(const json_schema *schema)
{
    if (schema->flags & REQUIRED)
    {
        if (schema->node == NULL)
        {
            fprintf(stderr, "Error testing 'required'\n");
            return 0;
        }
    }
    else if (schema->required)
    {
        const json *node = schema->required;

        while (node != NULL)
        {
            if (!json_pair(schema->node, json_string(node)))
            {
                fprintf(stderr, "'%s' is required\n", json_string(node));
                return 0;
            }
            node = json_next(node);
        }
    }
    if (schema->dependent_required)
    {
        const json *node = schema->dependent_required;;

        while (node != NULL)
        {
            if (json_pair(schema->node, json_name(node)))
            {
                const json *item = json_child(node);

                while (item != NULL)
                {
                    if (!json_pair(schema->node, json_string(item)))
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

static int test_properties(const json_schema *schema)
{
    if (schema->min_properties || schema->max_properties)
    {
        size_t properties = json_items(schema->node);

        if (schema->min_properties && (properties < real(schema->min_properties)))
        {
            fprintf(stderr, "Error testing 'minProperties'\n");
            return 0;
        }
        if (schema->max_properties && (properties > real(schema->max_properties)))
        {
            fprintf(stderr, "Error testing 'maxProperties'\n");
            return 0;
        }
    }
    if (schema->properties && (schema->flags & NOT_ADDITIONAL_PROPERTIES))
    {
        const json *node = json_child(schema->node);

        while (node != NULL)
        {
            const json *property = schema->properties;
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

static int test_items(const json_schema *schema)
{
    if (schema->min_items || schema->max_items)
    {
        size_t items = json_items(schema->node);

        if (schema->min_items && (items < real(schema->min_items)))
        {
            fprintf(stderr, "Error testing 'minItems'\n");
            return 0;
        }
        if (schema->max_items && (items > real(schema->max_items)))
        {
            fprintf(stderr, "Error testing 'maxItems'\n");
            return 0;
        }
    }
    if (schema->flags & UNIQUE_ITEMS)
    {
        const json *head, *node;

        head = node = json_child(schema->node);
        while (node != NULL)
        {
            for (const json *item = head; item != node; item = json_next(item))
            {
                if (json_equal(item, node))
                {
                    fprintf(stderr, "Error testing 'uniqueItems'\n");
                    return 0;
                }
            }
            node = json_next(node);
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

static int test_string(const json_schema *schema)
{
    if (schema->min_length || schema->max_length)
    {
        size_t min = schema->min_length ? real(schema->min_length) : 0;
        size_t max = schema->max_length ? real(schema->max_length) : 0;
        size_t length = get_length(json_string(schema->node));

        if (schema->min_length && (min > length))
        {
            fprintf(stderr, "Error testing 'minLength'\n");
            return 0;
        }
        if (schema->max_length && (max < length))
        {
            fprintf(stderr, "Error testing 'maxLength'\n");
            return 0;
        }
    }
    if (schema->format)
    {
        if (!schema->format(json_string(schema->node)))
        {
            fprintf(stderr, "Error testing 'format'\n");
            return 0;
        }
    }
    return 1;
}

static int test_minimum(const json_schema *schema, double value)
{
    if (schema->flags & EXCLUSIVE_MINIMUM)
    {
        return value > number(schema->minimum);
    }
    else
    {
        return value >= number(schema->minimum);
    }
}

static int test_maximum(const json_schema *schema, double value)
{
    if (schema->flags & EXCLUSIVE_MAXIMUM)
    {
        return value < number(schema->maximum);
    }
    else
    {
        return value <= number(schema->maximum);
    }
}

static int test_number(const json_schema *schema)
{
    if (schema->minimum || schema->maximum || schema->multiple_of)
    {
        double value = json_number(schema->node);

        if (schema->minimum && !test_minimum(schema, value))
        {
            fprintf(stderr, "Error testing 'minimum'\n");
            return 0;
        }
        if (schema->maximum && !test_maximum(schema, value))
        {
            fprintf(stderr, "Error testing 'maximum'\n");
            return 0;
        }
        if (schema->multiple_of && fmod(value, number(schema->multiple_of)))
        {
            fprintf(stderr, "Error testing 'multipleOf'\n");
            return 0;
        }
    }
    return 1;
}

static int test_schema(const json_schema *schema)
{
    if (!test_required(schema))
    {
        return 0;
    }
    if (schema->node == NULL)
    {
        return 1;
    }

    unsigned type = json_type(schema->node);

    if (!test_type(schema, type))
    {
        return 0;
    }
    switch (type)
    {
        case JSON_OBJECT:
            return test_properties(schema);
        case JSON_ARRAY:
            return test_items(schema);
        case JSON_STRING:
            return test_string(schema);
        case JSON_NUMBER:
            return test_number(schema);
        default:
            return 1;
    }
}

typedef int (*schema_setter)(json_schema *, const json *);

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

static json_subschema *get_subschema(const json_schema *schema,
    json_subschema *subschema)
{
    if (schema->properties != NULL)
    {
        json_subschema *new = malloc(sizeof *new);

        if (new == NULL)
        {
            return NULL;
        }
        new->root = schema->node;
        new->node = schema->properties;
        new->next = subschema;
        subschema = new;
    }
    else
    {
        while (subschema != NULL)
        {
            subschema->node = json_next(subschema->node);
            if (subschema->node == NULL)
            {
                json_subschema *next = subschema->next;

                free(subschema);
                subschema = next;
            }
            else
            {
                break;
            }
        }
    }
    return subschema;
}

static void clean_subschema(json_subschema *subschema)
{
    while (subschema != NULL)
    {
        json_subschema *next = subschema->next;

        free(subschema);
        subschema = next;
    }
}

static int valid_schema(json_schema *schema, const json *node)
{
    json_subschema *subschema = NULL;
    int valid = 1;

    for (;;)
    {
        if (node == NULL)
        {
            if (!test_schema(schema))
            {
                valid = 0;
                break;
            }
            subschema = get_subschema(schema, subschema);
            if (subschema == NULL)
            {
                break;
            }
            memset(schema, 0, sizeof *schema);
            schema->node = json_pair(subschema->root, json_name(subschema->node));
            node = json_child(subschema->node);
        }

        const char *name = json_name(node);

        if (name != NULL)
        {
            schema_setter setter = get_setter(name);

            if (setter != NULL)
            {
                if (!setter(schema, node))
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
    clean_subschema(subschema);
    return valid;
}

static int validate(json_schema *schema, const json *node)
{
    if (!json_is_object(node))
    {
        fprintf(stderr, "Invalid schema\n");
        return 0;
    }
    if ((node = json_child(node)))
    {
        return valid_schema(schema, node);
    }
    return 1;
}

int json_validate(const json *node, const json *rules)
{
    json_schema schema = {.node = node};

    return validate(&schema, rules);
}

