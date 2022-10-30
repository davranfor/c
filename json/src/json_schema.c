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

#define SUBSCHEMA_BAD_ALLOC (1u << 0u)

typedef struct
{
    const json *iter, *node;
    size_t size, flags;
    int type;
} json_schema;

typedef struct
{
    json_schema schema;
    schema_callback callback;
    void *data;
} schema_validator;

enum {SCHEMA_ROOT, SCHEMA_OBJECT, SCHEMA_TUPLE, SCHEMA_ARRAY};

static void raise_warning(const json_schema *schema, const char *fmt, ...)
{
    const schema_validator *user = (const schema_validator *)schema;

    if (user->callback)
    {
        char message[1024];
        va_list args;

        va_start(args, fmt);
        vsnprintf(message, sizeof message, fmt, args);
        va_end(args);
        user->callback(schema->node, user->data, SCHEMA_WARNING, message);
    }
}

static void raise_error(const json_schema *schema, const char *fmt, ...)
{
    const schema_validator *user = (const schema_validator *)schema;

    if (user->callback)
    {
        char message[1024];
        va_list args;

        va_start(args, fmt);
        vsnprintf(message, sizeof message, fmt, args);
        va_end(args);
        user->callback(schema->node, user->data, SCHEMA_ERROR, message);
    }
}

static int childs_are(const json *node, enum json_type type)
{
    node = json_child(node);
    while (node != NULL)
    {
        if (json_type(node) != type)
        {
            return 0;
        }
        node = json_next(node);
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
    const json *head = node = json_child(node);

    while (node != NULL)
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
        node = json_next(node);
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

static int test_any(json_schema *schema, const json *node)
{
    (void)schema;
    (void)node;
    return 1;
}

static int test_is_array(json_schema *schema, const json *node)
{
    if (!json_is_array(node))
    {
        raise_error(schema, "Setting '%s'", json_name(node));
        return 0;
    }
    return 1;
}

static int test_is_string(json_schema *schema, const json *node)
{
    if (!json_is_string(node))
    {
        raise_error(schema, "Setting '%s'", json_name(node));
        return 0;
    }
    return 1;
}

static int test_is_boolean(json_schema *schema, const json *node)
{
    if (!json_is_boolean(node))
    {
        raise_error(schema, "Setting '%s'", json_name(node));
        return 0;
    }
    return 1;
}

static unsigned set_type(const char *type, unsigned value)
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

static unsigned get_type(const json *node)
{
    unsigned type = 0;

    if (json_is_string(node))
    {
        type = set_type(json_string(node), type);
    }
    else if (json_is_array(node) && unique_strings(node))
    {
        node = json_child(node);
        while (node != NULL)
        {
            if (!(type = set_type(json_string(node), type)))
            {
                break;
            }
            node = json_next(node);
        }
    }
    return type;
}

static int test_type(json_schema *schema, const json *node)
{
    unsigned mask = get_type(node);

    if (mask == 0)
    {
        raise_error(schema, "Setting 'type'");
        return 0;
    }
    if (schema->node)
    {
        unsigned type = json_type(schema->node);
        int valid;

        // 'integer' must validate if type is 'number'
        valid = (mask & (1u << type))
            || ((mask & (1u << JSON_DOUBLE)) && (type == JSON_INTEGER));

        if (!valid)
        {
            raise_error(schema, "Testing 'type'");
            return 0;
        }
    }
    return 1;
}

static int test_const(json_schema *schema, const json *node)
{
    if (schema->node && !json_equal(node, schema->node))
    {
        raise_error(schema, "Testing 'const'");
        return 0;
    }
    return 1;
}

static int test_enum(json_schema *schema, const json *node)
{
    if (!json_is_array(node))
    {
        raise_error(schema, "Setting 'enum'");
        return 0;
    }
    if (schema->node)
    {
        node = json_child(node);
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

static int test_required(json_schema *schema, const json *node)
{
    if (json_is_boolean(node))
    {
        if (json_is_boolean(node) && !schema->node)
        {
            const char *name = json_name(json_parent(node));

            raise_error(schema, "Testing 'required: %s'", name ? name : "/");
            return 0;
        }
    }
    else if (json_is_array(node) && unique_strings(node))
    {
        if (json_is_object(schema->node))
        {
            int valid = 1;

            node = json_child(node);
            while (node != NULL)
            {
                if (!json_find(schema->node, json_string(node)))
                {
                    raise_error(schema, "Testing 'required: %s'",
                                json_string(node));
                    valid = 0;
                }
                node = json_next(node);
            }
            return valid;
        }
    }
    else
    {
        raise_error(schema, "Setting 'required'");
        return 0;
    }
    return 1;
}

static int test_dependent_required(json_schema *schema, const json *node)
{
    if (!json_is_object(node))
    {
        raise_error(schema, "Setting 'dependentRequired'");
        return 0;
    }
    if (json_is_object(schema->node))
    {
        int valid = 1;

        node = json_child(node);
        while (node != NULL)
        {
            if (!(json_is_array(node) && unique_strings(node)))
            {
                raise_error(schema, "Setting 'dependentRequired'");
                return 0;
            }
            if (json_find(schema->node, json_name(node)))
            {
                const json *item = json_child(node);

                while (item != NULL)
                {
                    if (!json_find(schema->node, json_string(item)))
                    {
                        raise_error(schema, "'%s' is required if '%s' is set",
                                    json_string(item), json_name(node));
                        valid = 0;
                    }
                    item = json_next(item);
                }
            }
            node = json_next(node);
        }
        return valid;
    }
    return 1;
}

static int test_properties(json_schema *schema, const json *node)
{
    if (!(json_is_object(node) && unique_objects(node)))
    {
        raise_error(schema, "Setting 'properties'");
        return 0;
    }
    if (json_is_object(schema->node))
    {
        schema->iter = json_child(node);
        schema->type = SCHEMA_OBJECT;
    }
    return 1;
}

static int test_additional_properties(json_schema *schema, const json *node)
{
    if (!json_is_boolean(node))
    {
        raise_error(schema, "Setting 'additionalProperties'");
        return 0;
    }
    if (!json_boolean(node) && json_is_object(schema->node))
    {
        const json *properties = json_find(json_parent(node), "properties");

        if (properties)
        {
            node = json_child(schema->node);
            while (node != NULL)
            {
                if (!json_find(properties, json_name(node)))
                {
                    raise_error(schema, "'%s' was not expected",
                                json_name(node));
                    return 0;
                }
                node = json_next(node);
            }
        }
    }
    return 1;
}

static int test_min_properties(json_schema *schema, const json *node)
{
    if (!json_is_real(node))
    {
        raise_error(schema, "Setting 'minProperties'");
        return 0;
    }
    if (json_is_object(schema->node))
    {
        if (schema->size == 0)
        {
            schema->size = json_items(schema->node);
        }
        if (schema->size < json_real(node))
        {
            raise_error(schema, "Testing 'minProperties: %s'",
                        json_string(node));
            return 0;
        }
    }
    return 1;
}

static int test_max_properties(json_schema *schema, const json *node)
{
    if (!json_is_real(node))
    {
        raise_error(schema, "Setting 'maxProperties'");
        return 0;
    }
    if (json_is_object(schema->node))
    {
        if (schema->size == 0)
        {
            schema->size = json_items(schema->node);
        }
        if (schema->size > json_real(node))
        {
            raise_error(schema, "Testing 'maxProperties: %s'",
                        json_string(node));
            return 0;
        }
    }
    return 1;
}

static int test_items(json_schema *schema, const json *node)
{
    if (!json_is_object(node) &&
        !(json_is_array(node) && childs_are_objects(node)))
    {
        raise_error(schema, "Setting 'items'");
        return 0;
    }
    if (json_is_array(schema->node))
    {
        if (json_is_object(node))
        {
            schema->iter = node;
            schema->type = SCHEMA_ARRAY;
        }
        else
        {
            schema->iter = json_child(node);
            schema->type = SCHEMA_TUPLE;
        }
    }
    return 1;
}

static int test_additional_items(json_schema *schema, const json *node)
{
    if (!json_is_boolean(node))
    {
        raise_error(schema, "Setting 'additionalItems'");
        return 0;
    }
    if (!json_boolean(node) && json_is_array(schema->node))
    {
        const json *items = json_find(json_parent(node), "items");

        if (json_is_array(items))
        {
            size_t tuples = json_items(items);

            if (tuples > 0)
            {
                if (schema->size == 0)
                {
                    schema->size = json_items(schema->node);
                }
                if (schema->size > tuples)
                {
                    raise_error(schema, "Testing 'additionalItems'");
                    return 0;
                }
            }
        }
    }
    return 1;
}

static int test_min_items(json_schema *schema, const json *node)
{
    if (!json_is_real(node))
    {
        raise_error(schema, "Setting 'minItems'");
        return 0;
    }
    if (json_is_array(schema->node))
    {
        if (schema->size == 0)
        {
            schema->size = json_items(schema->node);
        }
        if (schema->size < json_real(node))
        {
            raise_error(schema, "Testing 'minItems: %s'", json_string(node));
            return 0;
        }
    }
    return 1;
}

static int test_max_items(json_schema *schema, const json *node)
{
    if (!json_is_real(node))
    {
        raise_error(schema, "Setting 'maxItems'");
        return 0;
    }
    if (json_is_array(schema->node))
    {
        if (schema->size == 0)
        {
            schema->size = json_items(schema->node);
        }
        if (schema->size > json_real(node))
        {
            raise_error(schema, "Testing 'maxItems: %s'", json_string(node));
            return 0;
        }
    }
    return 1;
}

static int test_unique_items(json_schema *schema, const json *node)
{
    if (!json_is_boolean(node))
    {
        raise_error(schema, "Setting 'uniqueItems'");
        return 0;
    }
    if (json_boolean(node) && json_is_array(schema->node))
    {
        const json *head = node = json_child(schema->node);

        while (node != NULL)
        {
            for (const json *item = head; item != node; item = json_next(item))
            {
                if (json_equal(node, item))
                {
                    raise_error(schema, "Testing 'uniqueItems'");
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

static int test_min_length(json_schema *schema, const json *node)
{
    if (!json_is_real(node))
    {
        raise_error(schema, "Setting 'minLength'");
        return 0;
    }
    if (json_is_string(schema->node))
    {
        if (json_real(node) > get_length(json_string(schema->node)))
        {
            raise_error(schema, "Testing 'minLength: %s'", json_string(node));
            return 0;
        }
    }
    return 1;
}

static int test_max_length(json_schema *schema, const json *node)
{
    if (!json_is_real(node))
    {
        raise_error(schema, "Setting 'maxLength'");
        return 0;
    }
    if (json_is_string(schema->node))
    {
        if (json_real(node) < get_length(json_string(schema->node)))
        {
            raise_error(schema, "Testing 'maxLength: %s'", json_string(node));
            return 0;
        }
    }
    return 1;
}

static int test_format(json_schema *schema, const json *node)
{
    if (!json_is_string(node))
    {
        raise_error(schema, "Setting 'format'");
        return 0;
    }
    if (json_is_string(schema->node))
    {
        const char *name = json_string(node);
        int valid;

        schema_format format =
            equal(name, "date") ? test_is_date :
            equal(name, "time") ? test_is_time :
            equal(name, "date-time") ? test_is_date_time :
            equal(name, "email") ? test_is_email :
            equal(name, "ipv4") ? test_is_ipv4 :
            equal(name, "ipv6") ? test_is_ipv6 :
            equal(name, "uuid") ? test_is_uuid : NULL;

        if (format)
        {
            valid = format(json_string(schema->node));
        }
        else
        {
            valid = test_regex(name, json_string(schema->node));
        }
        if (!valid)
        {
            raise_error(schema, "Testing 'format: %s'", name);
            return 0;
        }
    }
    return 1;
}

static int test_pattern(json_schema *schema, const json *node)
{
    if (!json_is_string(node))
    {
        raise_error(schema, "Setting 'pattern'");
        return 0;
    }
    if (json_is_string(schema->node))
    {
        const char *name = json_string(node);

        if (!test_regex(name, json_string(schema->node)))
        {
            raise_error(schema, "Testing 'pattern: %s'", name);
            return 0;
        }
    }
    return 1;
}

static int test_minimum(json_schema *schema, const json *node)
{
    if (!json_is_number(node))
    {
        raise_error(schema, "Setting 'minimum'");
        return 0;
    }
    if (json_is_number(schema->node))
    {
        const json *exclusive = json_find(json_parent(node), "exclusiveMinimum");
        int valid;

        if (json_is_boolean(exclusive) && json_boolean(exclusive))
        {
            valid = json_number(schema->node) > json_number(node);
        }
        else
        {
            valid = json_number(schema->node) >= json_number(node);
        }
        if (!valid)
        {
            raise_error(schema, "Testing 'minimum: %s'", json_string(node));
            return 0;
        }
    }
    return 1;
}

static int test_maximum(json_schema *schema, const json *node)
{
    if (!json_is_number(node))
    {
        raise_error(schema, "Setting 'maximum'");
        return 0;
    }
    if (json_is_number(schema->node))
    {
        const json *exclusive = json_find(json_parent(node), "exclusiveMaximum");
        int valid;

        if (json_is_boolean(exclusive) && json_boolean(exclusive))
        {
            valid = json_number(schema->node) < json_number(node);
        }
        else
        {
            valid = json_number(schema->node) <= json_number(node);
        }
        if (!valid)
        {
            raise_error(schema, "Testing 'maximum: %s'", json_string(node));
            return 0;
        }
    }
    return 1;
}

static int test_exclusive_minimum(json_schema *schema, const json *node)
{
    if (!json_is_boolean(node))
    {
        raise_error(schema, "Setting 'exclusiveMinimum'");
        return 0;
    }
    return 1;
}

static int test_exclusive_maximum(json_schema *schema, const json *node)
{
    if (!json_is_boolean(node))
    {
        raise_error(schema, "Setting 'exclusiveMaximum'");
        return 0;
    }
    return 1;
}

static int test_multiple_of(json_schema *schema, const json *node)
{
    if (!json_is_number(node))
    {
        raise_error(schema, "Setting 'multipleOf'");
        return 0;
    }
    if (json_is_number(schema->node))
    {
        if (fmod(json_number(schema->node), json_number(node)))
        {
            raise_error(schema, "Testing 'multipleOf: %s'", json_string(node));
            return 0;
        }
    }
    return 1;
}

typedef int (*schema_test)(json_schema *, const json *);

static schema_test get_test(const char *name)
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
        equal(name, "exclusiveMinimum") ? test_exclusive_minimum :
        equal(name, "exclusiveMaximum") ? test_exclusive_maximum :
        equal(name, "multipleOf") ? test_multiple_of :
        equal(name, "readOnly") ? test_is_boolean :
        equal(name, "writeOnly") ? test_is_boolean :
        equal(name, "deprecated") ? test_is_boolean :
        equal(name, "examples") ? test_is_array :
        equal(name, "default") ? test_any : NULL;
}

typedef struct subschema
{
    const json *iter, *root, *node;
    struct subschema *next;
    int type;
} json_subschema;

static void set_iter(const json_schema *schema, json_subschema *subschema)
{
    subschema->iter = schema->iter;
    subschema->type = schema->type;
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
            subschema->node = json_find(
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
                subschema->node = json_find(
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
        schema->flags |= SUBSCHEMA_BAD_ALLOC;
    }
    return new;
}

static json_subschema *next_subschema(json_schema *schema,
    json_subschema *subschema)
{
    if (schema->iter != NULL)
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
    json_subschema *subschema = NULL;
    int valid = 1;

    for (;;)
    {
        if (node == NULL)
        {
            if ((subschema = next_subschema(schema, subschema)))
            {
                printf("subschema: %s\n", json_name(subschema->iter));
                node = json_child(subschema->iter);
                memset(schema, 0, sizeof *schema);
                schema->node = subschema->node;
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
            schema_test func = get_test(name);

            if (func != NULL)
            {
                if (!func(schema, node))
                {
                    valid = 0;
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

int schema_validate(const json *node, const json *schema,
    schema_callback callback, void *data)
{
    schema_validator validator =
    {
        .schema.node = node,
        .callback = callback,
        .data = data
    };

    if (!json_is_object(schema))
    {
        raise_error(&validator.schema, "Invalid schema");
        return 0;
    }
    if ((schema = json_child(schema)))
    {
        return valid_schema(&validator.schema, schema);
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

