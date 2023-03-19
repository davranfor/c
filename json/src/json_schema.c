/*! 
 *  \brief     JSON Schema validator
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <math.h>
#include "json.h"
#include "json_format.h"
#include "json_schema.h"

typedef struct
{
    const json *root;
    // Recursion
    const json *skip;
    int depth;
    // User data
    json_schema_callback callback;
    void *data;
    // Exception
    jmp_buf error;
} json_schema;

enum
{
    SCHEMA_INVALID, SCHEMA_VALID, SCHEMA_ERROR,
    SCHEMA_DEPENDENT_SCHEMAS,
    SCHEMA_PROPERTIES, SCHEMA_PATTERN_PROPERTIES, SCHEMA_ADDITIONAL_PROPERTIES,
    SCHEMA_ITEMS, SCHEMA_ADDITIONAL_ITEMS, SCHEMA_TUPLES,
    SCHEMA_REF,
    SCHEMA_NOT,
    SCHEMA_ALL_OF, SCHEMA_ANY_OF, SCHEMA_ONE_OF,
    SCHEMA_IF, SCHEMA_THEN, SCHEMA_ELSE
};

#define SCHEMA_MAX_DEPTH 1024

#define equal(a, b) (strcmp((a), (b)) == 0)

static void schema_callback(const json_schema *schema,
    const json *rule, const json *node, const char *title)
{
    if (schema->callback)
    {
        fprintf(stderr, "%s:\n", title);
        schema->callback(node, rule, schema->depth, schema->data);
    }
}

static void raise_invalid(const json_schema *schema,
    const json *rule, const json *node)
{
    schema_callback(schema, rule, node, "Invalid");
}

static void raise_warning(const json_schema *schema,
    const json *rule, const json *node)
{
    schema_callback(schema, rule, node, "Warning");
}

static void raise_error(json_schema *schema,
    const json *rule, const json *node)
{
    schema_callback(schema, rule, node, "Error");
    longjmp(schema->error, 1);
}

static int test_error(const json *rule, const json *node)
{
    (void)rule;
    (void)node;
    return SCHEMA_ERROR;
}

static int test_valid(const json *rule, const json *node)
{
    (void)rule;
    (void)node;
    return SCHEMA_VALID;
}

static int test_is_object(const json *rule, const json *node)
{
    (void)node;
    return json_is_object(rule) ? SCHEMA_VALID : SCHEMA_ERROR;
}

static int test_is_array(const json *rule, const json *node)
{
    (void)node;
    return json_is_array(rule) ? SCHEMA_VALID : SCHEMA_ERROR;
}

static int test_is_string(const json *rule, const json *node)
{
    (void)node;
    return json_is_string(rule) ? SCHEMA_VALID : SCHEMA_ERROR;
}

static int test_is_boolean(const json *rule, const json *node)
{
    (void)node;
    return json_is_boolean(rule) ? SCHEMA_VALID : SCHEMA_ERROR;
}

static int test_ref(const json *rule, const json *node)
{
    (void)node;
    return json_is_string(rule) ? SCHEMA_REF : SCHEMA_ERROR;
}

static int test_not(const json *rule, const json *node)
{
    (void)node;
    return json_is_object(rule) ? SCHEMA_NOT : SCHEMA_ERROR;
}

static int test_all_of(const json *rule, const json *node)
{
    (void)node;
    return json_is(rule, arrayOfOptionalObjects)
        ? SCHEMA_ALL_OF
        : SCHEMA_ERROR;
}

static int test_any_of(const json *rule, const json *node)
{
    (void)node;
    return json_is(rule, arrayOfOptionalObjects)
        ? SCHEMA_ANY_OF
        : SCHEMA_ERROR;
}

static int test_one_of(const json *rule, const json *node)
{
    (void)node;
    return json_is(rule, arrayOfOptionalObjects)
        ? SCHEMA_ONE_OF
        : SCHEMA_ERROR;
}

static int test_if(const json *rule, const json *node)
{
    (void)node;
    return json_is_object(rule) ? SCHEMA_IF : SCHEMA_ERROR;
}

static int test_then(const json *rule, const json *node)
{
    (void)node;
    return json_is_object(rule) ? SCHEMA_THEN : SCHEMA_ERROR;
}

static int test_else(const json *rule, const json *node)
{
    (void)node;
    return json_is_object(rule) ? SCHEMA_ELSE : SCHEMA_ERROR;
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

static int test_type(const json *rule, const json *node)
{
    unsigned mask = 0;

    if (json_is_string(rule))
    {
        if ((mask = add_type(json_string(rule), mask)) == 0)
        {
            return SCHEMA_ERROR;
        }
    }
    else if (json_is(rule, arrayOfOptionalStrings))
    {
        for (rule = json_child(rule); rule != NULL; rule = json_next(rule))
        {
            if ((mask = add_type(json_string(rule), mask)) == 0)
            {
                return SCHEMA_ERROR;
            }
        }
    }
    else
    {
        return SCHEMA_ERROR;
    }
    if (node != NULL)
    {
        unsigned type = json_type(node);

        /* 'integer' must validate if type is 'number' */
        return (mask & (1u << type))
           || ((mask & (1u << JSON_DOUBLE)) && (type == JSON_INTEGER));
    }
    return 1;
}

static int test_const(const json *rule, const json *node)
{
    if ((node != NULL) && !json_equal(rule, node))
    {
        return 0;
    }
    return 1;
}

static int test_enum(const json *rule, const json *node)
{
    if (!json_is_array(rule))
    {
        return SCHEMA_ERROR;
    }
    if (node != NULL)
    {
        for (rule = json_child(rule); rule != NULL; rule = json_next(rule))
        {
            if (json_equal(rule, node))
            {
                return 1;
            }
        }
        return 0;
    }
    return 1;
}

static int find_required(const json *rule, const json *node)
{
    for (rule = json_child(rule); rule != NULL; rule = json_next(rule))
    {
        if (!json_find(node, json_string(rule)))
        {
            return 0;
        }
    }
    return 1;
}

static int test_required(const json *rule, const json *node)
{
    if (json_is(rule, arrayOfOptionalStrings))
    {
        if (json_is_object(node))
        {
            return find_required(rule, node);
        }
        return 1;
    }
    return SCHEMA_ERROR;
}

static int test_dependent_required(const json *rule, const json *node)
{
    if (!json_is_object(rule))
    {
        return SCHEMA_ERROR;
    }

    int valid = 1;

    for (rule = json_child(rule); rule != NULL; rule = json_next(rule))
    {
        if (!json_is(rule, arrayOfOptionalStrings))
        {
            return SCHEMA_ERROR;
        }
        if (valid && json_is_object(node))
        {
            if (json_find(node, json_name(rule)) && !find_required(rule, node))
            {
                valid = 0;
            }
        }
    }
    return valid;
}

static int test_dependent_schemas(const json *rule, const json *node)
{
    (void)node;
    return json_is(rule, objectOfOptionalObjects)
        ? SCHEMA_DEPENDENT_SCHEMAS
        : SCHEMA_ERROR;
}

static int test_properties(const json *rule, const json *node)
{
    (void)node;
    return json_is(rule, objectOfOptionalObjects)
        ? SCHEMA_PROPERTIES
        : SCHEMA_ERROR;
}

static int test_pattern_properties(const json *rule, const json *node)
{
    (void)node;
    return json_is(rule, objectOfOptionalObjects)
        ? SCHEMA_PATTERN_PROPERTIES
        : SCHEMA_ERROR;
}

static int test_additional_properties(const json *rule, const json *node)
{
    if (json_is_object(rule))
    {
        return SCHEMA_ADDITIONAL_PROPERTIES;
    }
    if (!json_is_boolean(rule))
    {
        return SCHEMA_ERROR;
    }
    if (json_is_false(rule) && json_is_object(node))
    {
        const json *properties = json_find(json_parent(rule), "properties");

        if (json_is(properties, objectOfOptionalObjects))
        {
            for (node = json_child(node); node != NULL; node = json_next(node))
            {
                if (!json_find(properties, json_name(node)))
                {
                    return 0;
                }
            }
        }
    }
    return 1;
}

static int test_min_properties(const json *rule, const json *node)
{
    if (!json_is_real(rule))
    {
        return SCHEMA_ERROR;
    }
    if (json_is_object(node))
    {
        return json_items(node) >= json_real(rule);
    }
    return 1;
}

static int test_max_properties(const json *rule, const json *node)
{
    if (!json_is_real(rule))
    {
        return SCHEMA_ERROR;
    }
    if (json_is_object(node))
    {
        return json_items(node) <= json_real(rule);
    }
    return 1;
}

static int test_items(const json *rule, const json *node)
{
    if (json_is_boolean(rule))
    {
        return json_is_array(node)
            ? (json_boolean(rule) == json_is_any(json_child(node)))
            : SCHEMA_VALID;
    }
    if (json_is_object(rule))
    {
        return SCHEMA_ITEMS;
    }
    if (json_is(rule, arrayOfOptionalObjects))
    {
        return SCHEMA_TUPLES;
    }
    return SCHEMA_ERROR;
}

static int test_additional_items(const json *rule, const json *node)
{
    if (json_is_object(rule))
    {
        return SCHEMA_ADDITIONAL_ITEMS;
    }
    if (!json_is_boolean(rule))
    {
        return SCHEMA_ERROR;
    }
    if (json_is_false(rule) && json_is_array(node))
    {
        const json *items = json_find(json_parent(rule), "items");

        if (json_is(items, arrayOfOptionalObjects))
        {
            return json_items(node) <= json_items(items);
        }
    }
    return 1;
}

static int test_min_items(const json *rule, const json *node)
{
    if (!json_is_real(rule))
    {
        return SCHEMA_ERROR;
    }
    if (json_is_array(node))
    {
        return json_items(node) >= json_real(rule);
    }
    return 1;
}

static int test_max_items(const json *rule, const json *node)
{
    if (!json_is_real(rule))
    {
        return SCHEMA_ERROR;
    }
    if (json_is_array(node))
    {
        return json_items(node) <= json_real(rule);
    }
    return 1;
}

static int test_unique_items(const json *rule, const json *node)
{
    if (!json_is_boolean(rule))
    {
        return SCHEMA_ERROR;
    }
    if (json_is_true(rule) && json_is_array(node))
    {
        return json_is(node, arrayOfUniqueItems);
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

static int test_min_length(const json *rule, const json *node)
{
    if (!json_is_real(rule))
    {
        return SCHEMA_ERROR;
    }
    if (json_is_string(node))
    {
        return get_length(json_string(node)) >= json_real(rule);
    }
    return 1;
}

static int test_max_length(const json *rule, const json *node)
{
    if (!json_is_real(rule))
    {
        return SCHEMA_ERROR;
    }
    if (json_is_string(node))
    {
        return get_length(json_string(node)) <= json_real(rule);
    }
    return 1;
}

static int test_format(const json *rule, const json *node)
{
    if (!json_is_string(rule))
    {
        return SCHEMA_ERROR;
    }
    if (json_is_string(node))
    {
        const char *name = json_string(rule);

        int (*format)(const char *) =
            equal(name, "date") ? test_is_date :
            equal(name, "time") ? test_is_time :
            equal(name, "date-time") ? test_is_date_time :
            equal(name, "hostname") ? test_is_hostname :
            equal(name, "email") ? test_is_email :
            equal(name, "ipv4") ? test_is_ipv4 :
            equal(name, "ipv6") ? test_is_ipv6 :
            equal(name, "uuid") ? test_is_uuid :
            equal(name, "url") ? test_is_url : NULL;

        if (format != NULL)
        {
            return format(json_string(node));
        }
        else
        {
            return test_regex(name, json_string(node));
        }
    }
    return 1;
}

static int test_pattern(const json *rule, const json *node)
{
    if (!json_is_string(rule))
    {
        return SCHEMA_ERROR;
    }
    if (json_is_string(node))
    {
        return test_regex(json_string(rule), json_string(node));
    }
    return 1;
}

static int test_minimum(const json *rule, const json *node)
{
    if (!json_is_number(rule))
    {
        return SCHEMA_ERROR;
    }
    if (json_is_number(node))
    {
        if (json_is_true(json_find(json_parent(rule), "exclusiveMinimum")))
        {
            return json_number(node) > json_number(rule);
        }
        else
        {
            return json_number(node) >= json_number(rule);
        }
    }
    return 1;
}

static int test_maximum(const json *rule, const json *node)
{
    if (!json_is_number(rule))
    {
        return SCHEMA_ERROR;
    }
    if (json_is_number(node))
    {
        if (json_is_true(json_find(json_parent(rule), "exclusiveMaximum")))
        {
            return json_number(node) < json_number(rule);
        }
        else
        {
            return json_number(node) <= json_number(rule);
        }
    }
    return 1;
}

static int test_multiple_of(const json *rule, const json *node)
{
    if (json_number(rule) <= 0)
    {
        return SCHEMA_ERROR;
    }
    if (json_is_number(node))
    {
        return fmod(json_number(node), json_number(rule)) == 0;
    }
    return 1;
}

static const json *handle_ref(json_schema *schema,
    const json *rule, const json *node)
{
    const char *ref = json_string(rule);
    
    if (ref[0] != '#')
    {
        raise_error(schema, rule, node);
    }

    const json *next = ref[1] ? json_node(rule, ref + 1) : schema->root;

    if (!json_is_object(next))
    {
        raise_error(schema, rule, node);
    }
    if (node == NULL)
    {
        if (schema->skip == rule)
        {
            schema->skip = NULL;
            return NULL;
        }
        if (schema->skip == NULL)
        {
            schema->skip = rule;
        }
    }
    else
    {
        schema->skip = NULL;
    }
    return next;
}

static int handle_cond(const json **rule, int cond)
{
    const json *next = json_next(*rule);
    const char *name;

    if (json_is_object(next) && (name = json_name(next)))
    {
        if (equal(name, "then"))
        {
            *rule = next;
            return cond;
        }
        if (equal(name, "else"))
        {
            *rule = next;
            return !cond;
        }
    }
    return -1;
}

typedef int (*tester)(const json *, const json *);

static tester get_test_by_name(const char *name)
{
    return
        equal(name, "$schema") ? test_is_string :
        equal(name, "$id") ? test_is_string :
        equal(name, "$defs") ? test_is_object :
        equal(name, "$ref") ? test_ref :
        equal(name, "title") ? test_is_string :
        equal(name, "description") ? test_is_string :
        equal(name, "not") ? test_not :
        equal(name, "allOf") ? test_all_of :
        equal(name, "anyOf") ? test_any_of :
        equal(name, "oneOf") ? test_one_of :
        equal(name, "if") ? test_if :
        equal(name, "then") ? test_then :
        equal(name, "else") ? test_else :
        equal(name, "type") ? test_type :
        equal(name, "const") ? test_const :
        equal(name, "enum") ? test_enum :
        equal(name, "required") ? test_required :
        equal(name, "dependentRequired") ? test_dependent_required :
        equal(name, "dependentSchemas") ? test_dependent_schemas :
        equal(name, "properties") ? test_properties :
        equal(name, "patternProperties") ? test_pattern_properties :
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

static tester get_test(const json *rule)
{
    const char *name = json_name(rule);

    if (name == NULL)
    {
        return test_error;
    }
    return get_test_by_name(name);
}

static int validate(json_schema *schema,
    const json *rule, const json *node, int flag)
{
    int valid = 1;

    if (schema->depth++ > SCHEMA_MAX_DEPTH)
    {
        raise_error(schema, rule, node);
    }
    while (rule != NULL)
    {
        const tester test = get_test(rule);

        if (test != NULL)
        {
            switch (test(rule, node))
            {
                case SCHEMA_DEPENDENT_SCHEMAS:
                {
                    const json *next = json_child(rule);

                    while (next != NULL)
                    {
                        if (json_find(node, json_name(next)))
                        {
                            valid &= validate(schema, json_child(next), node, flag);
                        }
                        else
                        {
                            validate(schema, json_child(next), NULL, 1);
                        }
                        next = json_next(next);
                    }
                }
                break;
                case SCHEMA_PROPERTIES:
                {
                    const json *item = json_is_object(node) ? node : NULL;
                    const json *next = json_child(rule);

                    if (item == NULL)
                    {
                        while (next != NULL)
                        {
                            validate(schema, json_child(next), item, 1);
                            next = json_next(next);
                        }
                    }
                    else while (next != NULL)
                    {
                        item = json_find(node, json_name(next));
                        do valid &= validate(schema, json_child(next), item, flag);
                        while ((item = json_find_next(item, json_name(next))));
                        next = json_next(next);
                    }
                }
                break;
                case SCHEMA_PATTERN_PROPERTIES:
                {
                    const json *head = json_is_object(node) ? json_child(node) : NULL;
                    const json *next = json_child(rule);

                    while (next != NULL)
                    {
                        const char *regex = json_name(next);
                        const json *item = head;
                        int count = 0;

                        while (item != NULL)
                        {
                            if (test_regex(regex, json_name(item)))
                            {
                                valid &= validate(schema, json_child(next), item, flag);
                                count++;
                            }
                            item = json_next(item);
                        }
                        if (count == 0)
                        {
                            validate(schema, json_child(next), NULL, 1);
                        }
                        next = json_next(next);
                    }
                }
                break;
                case SCHEMA_ADDITIONAL_PROPERTIES:
                {
                    const json *properties = json_find(json_parent(rule), "properties");
                    const json *item = json_is_object(node) ? json_child(node) : NULL;
                    const json *next = json_child(rule);
                    int count = 0;

                    if (json_is(properties, objectOfOptionalObjects))
                    {
                        while (item != NULL)
                        {
                            if (!json_find(properties, json_name(item)))
                            {
                                valid &= validate(schema, next, item, flag);
                                count++;
                            }
                            item = json_next(item);
                        }
                    }
                    if (count == 0)
                    {
                        validate(schema, next, NULL, 1);
                    }
                }
                break;
                case SCHEMA_ITEMS:
                {
                    const json *item = json_is_array(node) ? json_child(node) : NULL;
                    const json *next = json_child(rule);

                    if (item == NULL)
                    {
                        validate(schema, next, item, 1);
                    }
                    else while (item != NULL)
                    {
                        valid &= validate(schema, next, item, flag);
                        item = json_next(item);
                    }
                }
                break;
                case SCHEMA_ADDITIONAL_ITEMS:
                {
                    const json *next = json_child(rule);
                    const json *item = NULL;

                    if (json_is_array(node))
                    {
                        const json *items = json_find(json_parent(rule), "items");

                        if (json_is(items, arrayOfOptionalObjects))
                        {
                            item = json_item(node, json_items(items));
                        }
                    }
                    if (item == NULL)
                    {
                        validate(schema, next, item, 1);
                    }
                    else while (item != NULL)
                    {
                        valid &= validate(schema, next, item, flag);
                        item = json_next(item);
                    }
                }
                break;
                case SCHEMA_TUPLES:
                {
                    const json *item = json_is_array(node) ? json_child(node) : NULL;
                    const json *next = json_child(rule);

                    if (item == NULL)
                    {
                        while (next != NULL)
                        {
                            validate(schema, json_child(next), item, 1);
                            next = json_next(next);
                        }
                    }
                    else while (next != NULL)
                    {
                        valid &= validate(schema, json_child(next), item, flag);
                        next = json_next(next);
                        item = json_next(item);
                    }
                }
                break;
                case SCHEMA_REF:
                {
                    const json *next = handle_ref(schema, rule, node);

                    if (next != NULL)
                    {
                        valid &= validate(schema, json_child(next), node, flag);
                    }
                }
                break;
                case SCHEMA_NOT:
                {
                    int old_valid = valid;

                    valid = !validate(schema, json_child(rule), node, 1);
                    if (flag == 0)
                    {
                        if (valid)
                        {
                            valid = old_valid;
                        }
                        else
                        {
                            raise_invalid(schema, rule, node);
                        }
                    }
                }
                break;
                case SCHEMA_ALL_OF:
                case SCHEMA_ANY_OF:
                case SCHEMA_ONE_OF:
                {
                    const json *next = json_child(rule);
                    int old_valid = valid;
                    int count = 0;

                    valid = 1;
                    while (next != NULL)
                    {
                        if (count++ == 0)
                        {
                            valid = validate(schema, json_child(next), node, 1);
                        }
                        else if (test == test_all_of)
                        {
                            valid &= validate(schema, json_child(next), node, 1);
                        }
                        else if (test == test_any_of)
                        {
                            valid |= validate(schema, json_child(next), node, 1);
                        }
                        else if (test == test_one_of)
                        {
                            valid ^= validate(schema, json_child(next), node, 1);
                        }
                        next = json_next(next);
                    }
                    if (flag == 0)
                    {
                        if (valid)
                        {
                            valid = old_valid;
                        }
                        else
                        {
                            raise_invalid(schema, rule, node);
                        }
                    }
                }
                break;
                case SCHEMA_IF:
                {
                    int cond_valid = validate(schema, json_child(rule), node, 1);
                    int cond;

                    while ((cond = handle_cond(&rule, cond_valid)) != -1)
                    {
                        if (cond == 1)
                        {
                            valid &= validate(schema, json_child(rule), node, flag);
                        }
                        else
                        {
                            validate(schema, json_child(rule), NULL, 1);
                        }
                    }
                }
                break;
                case SCHEMA_THEN:
                case SCHEMA_ELSE:
                {
                    validate(schema, json_child(rule), NULL, 1);
                }
                break;
                case SCHEMA_INVALID:
                {
                    if (flag == 0)
                    {
                        raise_invalid(schema, rule, node);
                    }
                    valid = 0;
                }
                break;
                case SCHEMA_ERROR:
                {
                    raise_error(schema, rule, node);
                }
                break;
            }
        }
        else
        {
            raise_warning(schema, rule, node);
        }
        rule = json_next(rule);
    }
    schema->depth--;
    return valid;
}

int json_validate(const json *node, const json *rule,
    json_schema_callback callback, void *data)
{
    json_schema schema =
    {
        .root = rule,
        .callback = callback,
        .data = data
    };

    if (setjmp(schema.error))
    {
        return 0;
    }
    if (json_is_object(rule))
    {
        return validate(&schema, json_child(rule), node, 0);
    }
    raise_error(&schema, rule, node);
    return 0;
}

