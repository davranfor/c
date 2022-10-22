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

#define number(str) strtod(str, NULL)
#define real(str) strtoul(str, NULL, 10)
#define equal(a, b) (strcmp(a, b) == 0)

#define REQUIRED                    (1u << 0u)
#define NOT_ADDITIONAL_PROPERTIES   (1u << 1u)
#define NOT_ADDITIONAL_ITEMS        (1u << 2u)
#define UNIQUE_ITEMS                (1u << 3u)
#define EXCLUSIVE_MINIMUM           (1u << 4u)
#define EXCLUSIVE_MAXIMUM           (1u << 5u)

#define SUBSCHEMA_BAD_ALLOC         (1u << 6u)
#define IS_SUBSCHEMA                (1u << 7u)

typedef struct
{
    const json *node, *properties, *items;
    const json *required, *dependent_required;
    const json *const_value, *enum_values;
    const char *min_properties, *max_properties;
    const char *min_items, *max_items;
    const char *min_length, *max_length, *pattern;
    const char *minimum, *maximum, *multiple_of;
    unsigned type, tuples, flags;
    schema_format format;
    schema_callback callback;
    void *data;
} json_schema;

static void raise_warning(const json_schema *schema, const char *fmt, ...)
{
    if (schema->callback == NULL)
    {
        return;
    }

    char message[1024];
    va_list args;

    va_start(args, fmt);
    vsnprintf(message, sizeof message, fmt, args);
    va_end(args);
    schema->callback(schema->node, schema->data, SCHEMA_WARNING, message);
}

static void raise_error(const json_schema *schema, const char *fmt, ...)
{
    if (schema->callback == NULL)
    {
        return;
    }

    char message[1024];
    va_list args;

    va_start(args, fmt);
    vsnprintf(message, sizeof message, fmt, args);
    va_end(args);
    schema->callback(schema->node, schema->data, SCHEMA_ERROR, message);
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
            if (equal(comp_func(node), comp_func(item)))
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
    return unique(node, json_is_named_object, json_name);
}

static int unique_arrays(const json *node)
{
    return unique(node, json_is_named_array, json_name);
}

static int unique_strings(const json *node)
{
    return unique(node, json_is_string, json_string);
}

static int add_type(json_schema *schema, const char *type)
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
            schema->type |= 1u << (item + 1);
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

static int set_const(json_schema *schema, const json *node)
{
    schema->const_value = node;
    return 1;
}

static int set_enum(json_schema *schema, const json *node)
{
    if (json_is_array(node))
    {
        schema->enum_values = json_child(node);
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
        schema->items = json_child(node) ? node : NULL;
        return 1;
    }
    if (json_is_array(node))
    {
        const json *item = node = json_child(node);

        while (item != NULL)
        {
            if (!json_is_unnamed_object(item))
            {
                return 0;
            }
            item = json_next(item);
            schema->tuples++;
        }
        schema->items = node;
        return 1;
    }
    return 0;
}

static int set_additional_items(json_schema *schema, const json *node)
{
    if (json_is_boolean(node))
    {
        set_flag(schema, NOT_ADDITIONAL_ITEMS, !json_boolean(node));
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

static int set_unique_items(json_schema *schema, const json *node)
{
    if (json_is_boolean(node))
    {
        set_flag(schema, UNIQUE_ITEMS, json_boolean(node));
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

static int test_any(json_schema *schema, const json *node)
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
    int match = 1;

    if (schema->type)
    {
        /* 'integer' must validate if type is 'number' */
        match = (schema->type & (1u << type))
            || ((schema->type & (1u << JSON_DOUBLE)) && (type == JSON_INTEGER));

        if (!match)
        {
            raise_error(schema, "Testing 'type'");
        }
    }
    return match;
}

static int test_required(const json_schema *schema)
{
    if (schema->flags & REQUIRED)
    {
        if (schema->node == NULL)
        {
            raise_error(schema, "Testing 'required'");
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
                raise_error(schema, "'%s' is required", json_string(node));
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
                        raise_error(schema, "'%s' is required if '%s' is set",
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

static int test_const(const json_schema *schema)
{
    if (schema->const_value)
    {
        if (json_equal_value(schema->const_value, schema->node))
        {
            return 1;
        } 
        if (json_equal(schema->const_value, schema->node))
        {
            return 1;
        }
        raise_error(schema, "Testing 'const'");
        return 0;
    }
    return 1;
}

static int test_enum(const json_schema *schema)
{
    if (schema->enum_values)
    {
        const json *node = schema->enum_values;

        while (node != NULL)
        {
            if (json_equal(node, schema->node))
            {
                return 1;
            }
            node = json_next(node);
        }
        raise_error(schema, "Testing 'enum'");
        return 0;
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
            raise_error(schema, "Testing 'minProperties: %s'", schema->min_properties);
            return 0;
        }
        if (schema->max_properties && (properties > real(schema->max_properties)))
        {
            raise_error(schema, "Testing 'maxProperties: %s'", schema->max_properties);
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
                raise_error(schema, "'%s' was not expected", json_name(node));
                return 0;
            }
            node = json_next(node);
        }
    }
    return 1;
}

static int test_items(const json_schema *schema)
{
    if ((schema->min_items || schema->max_items)
    || ((schema->flags & NOT_ADDITIONAL_ITEMS) && (schema->tuples > 0)))
    {
        size_t items = json_items(schema->node);

        if (schema->min_items && (items < real(schema->min_items)))
        {
            raise_error(schema, "Testing 'minItems: %s'", schema->min_items);
            return 0;
        }
        if (schema->max_items && (items > real(schema->max_items)))
        {
            raise_error(schema, "Testing 'maxItems: %s'", schema->max_items);
            return 0;
        }
        if ((schema->flags & NOT_ADDITIONAL_ITEMS) &&
            (schema->tuples > 0) && (schema->tuples < items))
        {
            raise_error(schema, "Testing 'additionalItems: false'");
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
                if (json_equal(node, item))
                {
                    raise_error(schema, "Testing 'uniqueItems: true'");
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
            raise_error(schema, "Testing 'minLength: %s'", schema->min_length);
            return 0;
        }
        if (schema->max_length && (max < length))
        {
            raise_error(schema, "Testing 'maxLength: %s'", schema->max_length);
            return 0;
        }
    }
    if (schema->format)
    {
        if (!schema->format(json_string(schema->node)))
        {
            raise_error(schema, "Testing 'format: %s'", schema->format);
            return 0;
        }
    }
    if (schema->pattern)
    {
        if (!test_pattern(schema->pattern, json_string(schema->node)))
        {
            raise_error(schema, "Testing 'pattern: %s'", schema->pattern);
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
            raise_error(schema, "Testing 'minimum: %s'", schema->minimum);
            return 0;
        }
        if (schema->maximum && !test_maximum(schema, value))
        {
            raise_error(schema, "Testing 'maximum: %s'", schema->maximum);
            return 0;
        }
        if (schema->multiple_of && fmod(value, number(schema->multiple_of)))
        {
            raise_error(schema, "Testing 'multipleOf: %s'", schema->multiple_of);
            return 0;
        }
    }
    return 1;
}

static int test_schema(json_schema *schema)
{
    if (!test_required(schema))
    {
        return 0;
    }
    if (schema->node == NULL)
    {
        return 1;
    }
    if (!test_const(schema))
    {
        return 0;
    }
    if (!test_enum(schema))
    {
        return 0;
    }

    unsigned type = json_type(schema->node);

    if (!test_type(schema, type))
    {
        return 0;
    }
    switch (type)
    {
        case JSON_OBJECT:
            set_flag(schema, IS_SUBSCHEMA, 1);
            return test_properties(schema);
        case JSON_ARRAY:
            set_flag(schema, IS_SUBSCHEMA, 1);
            return test_items(schema);
        case JSON_STRING:
            return test_string(schema);
        case JSON_INTEGER:
        case JSON_DOUBLE:
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
        equal(name, "const") ? set_const :
        equal(name, "enum") ? set_enum :
        equal(name, "properties") ? set_properties :
        equal(name, "additionalProperties") ? set_additional_properties :
        equal(name, "minProperties") ? set_min_properties :
        equal(name, "maxProperties") ? set_max_properties :
        equal(name, "items") ? set_items :
        equal(name, "additionalItems") ? set_additional_items :
        equal(name, "minItems") ? set_min_items :
        equal(name, "maxItems") ? set_max_items :
        equal(name, "uniqueItems") ? set_unique_items :
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
        equal(name, "default") ? test_any : NULL;
}

enum {SCHEMA_OBJECT, SCHEMA_TUPLE, SCHEMA_ARRAY};

typedef struct subschema
{
    const json *iter, *root, *node;
    struct subschema *next;
    int type;
} json_subschema;

static void set_iter(const json_schema *schema, json_subschema *subschema)
{
    if (schema->properties && schema->items)
    {
        subschema->type = json_is_object(schema->node)
                        ? SCHEMA_OBJECT : SCHEMA_ARRAY;
    }
    else
    {
        subschema->type = schema->properties
                        ? SCHEMA_OBJECT : SCHEMA_ARRAY;
    }
    if (subschema->type == SCHEMA_OBJECT)
    {
        subschema->iter = schema->properties;
    }
    else
    {
        subschema->iter = schema->items;
        if (schema->tuples > 0)
        {
            subschema->type = SCHEMA_TUPLE;
        }
    }
}

static void set_root(const json_schema *schema, json_subschema *subschema)
{
    subschema->root = schema->node;
}

static void set_node(const json_schema *schema, json_subschema *subschema)
{
    switch (subschema->type)
    {
        case SCHEMA_OBJECT:
            subschema->node = json_pair(
                schema->node, json_name(subschema->iter)
            );
            break;
        case SCHEMA_TUPLE:
        case SCHEMA_ARRAY:
            subschema->node = json_child(schema->node);
            break;
    }
}

static int next_node(json_subschema *subschema)
{
    switch (subschema->type)
    {
        case SCHEMA_OBJECT:
            if ((subschema->iter = json_next(subschema->iter)))
            {
                subschema->node = json_pair(
                    subschema->root, json_name(subschema->iter)
                );
                return 1;
            }
            break;
        case SCHEMA_TUPLE:
            if ((subschema->iter = json_next(subschema->iter)))
            {
                subschema->node = json_next(subschema->node);
                return 1;
            }
            break;
        case SCHEMA_ARRAY:
            if ((subschema->node = json_next(subschema->node)))
            {
                return 1;
            }
            break;
    }
    return 0;
}

static json_subschema *new_subschema(json_schema *schema,
    json_subschema *subschema)
{
    json_subschema *new = malloc(sizeof *new);

    if (new != NULL)
    {
        set_iter(schema, new);
        set_root(schema, new);
        set_node(schema, new);
        new->next = subschema;
    }
    else
    {
        set_flag(schema, SUBSCHEMA_BAD_ALLOC, 1);
    }
    return new;
}

static json_subschema *next_subschema(json_schema *schema,
    json_subschema *subschema)
{
    if ((schema->flags & IS_SUBSCHEMA) &&
        (schema->properties || schema->items))
    {
        return new_subschema(schema, subschema);
    }
    while (subschema != NULL)
    {
        if (next_node(subschema))
        {
            break;
        }

        json_subschema *next = subschema->next;

        free(subschema);
        subschema = next;
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
    schema_callback callback = schema->callback;
    void *data = schema->data;

    json_subschema *subschema = NULL;
    int valid = 1;

    for (;;)
    {
        if (node == NULL)
        {
            if (!test_schema(schema))
            {
                valid = 0;
            }
            if ((subschema = next_subschema(schema, subschema)))
            {
                //printf("subschema: %s\n", json_name(subschema->iter));
                node = json_child(subschema->iter);
                memset(schema, 0, sizeof *schema);
                schema->node = subschema->node;
                schema->callback = callback;
                schema->data = data;
            }
            else
            {
                if (schema->flags & SUBSCHEMA_BAD_ALLOC)
                {
                    raise_error(schema, "Out of memory");
                    valid = 0;
                }
                break;
            }
        }

        const char *name = json_name(node);

        if (name != NULL)
        {
            schema_setter setter = get_setter(name);

            if (setter != NULL)
            {
                if (!setter(schema, node))
                {
                    raise_error(schema, "Setting '%s'", name);
                    valid = 0;
                    break;
                }
            }
            else
            {
                raise_warning(schema, "Unknown instance '%s'", name);
            }
        }
        node = json_next(node);
    }
    clean_subschema(subschema);
    return valid;
}

int schema_validate(const json *node, const json *rules,
    schema_callback callback, void *data)
{
    json_schema schema =
    {
        .node = node,
        .callback = callback,
        .data = data
    };

    if (!json_is_object(rules))
    {
        raise_error(&schema, "Invalid schema");
        return 0;
    }
    if ((rules = json_child(rules)))
    {
        return valid_schema(&schema, rules);
    }
    return 1;
}

void schema_default_callback(const json *node, void *data, int type,
    const char *message)
{
    (void)data;
    fprintf(stderr, "%s: %s\n", type == SCHEMA_ERROR ? "Error" : "Warning", message);
    json_write(stderr, node);
}

