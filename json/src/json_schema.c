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
    json_callback callback;
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

#define equal(a, b) (strcmp(a, b) == 0)

static void schema_callback(const json_schema *schema,
    const json *node, const json *iter, const char *title)
{
    if (schema->callback)
    {
        fprintf(stderr, "%s:\n", title);
        schema->callback(iter, schema->depth, schema->data);
        json_write(node, stderr);
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

static void raise_error(json_schema *schema,
    const json *node, const json *iter)
{
    schema_callback(schema, node, iter, "Error");
    longjmp(schema->error, 1);
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

static int test_is_object(const json *node, const json *iter)
{
    (void)iter;
    return json_is_object(node) ? SCHEMA_VALID : SCHEMA_ERROR;
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

static int test_ref(const json *node, const json *iter)
{
    (void)iter;
    return json_is_string(node) ? SCHEMA_REF : SCHEMA_ERROR;
}

static int test_not(const json *node, const json *iter)
{
    (void)iter;
    return json_is_object(node) ? SCHEMA_NOT : SCHEMA_ERROR;
}

static int test_all_of(const json *node, const json *iter)
{
    (void)iter;
    return json_is(node, arrayOfOptionalObjects)
        ? SCHEMA_ALL_OF
        : SCHEMA_ERROR;
}

static int test_any_of(const json *node, const json *iter)
{
    (void)iter;
    return json_is(node, arrayOfOptionalObjects)
        ? SCHEMA_ANY_OF
        : SCHEMA_ERROR;
}

static int test_one_of(const json *node, const json *iter)
{
    (void)iter;
    return json_is(node, arrayOfOptionalObjects)
        ? SCHEMA_ONE_OF
        : SCHEMA_ERROR;
}

static int test_if(const json *node, const json *iter)
{
    (void)iter;
    return json_is_object(node) ? SCHEMA_IF : SCHEMA_ERROR;
}

static int test_then(const json *node, const json *iter)
{
    (void)iter;
    return json_is_object(node) ? SCHEMA_THEN : SCHEMA_ERROR;
}

static int test_else(const json *node, const json *iter)
{
    (void)iter;
    return json_is_object(node) ? SCHEMA_ELSE : SCHEMA_ERROR;
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
        if ((mask = add_type(json_string(node), mask)) == 0)
        {
            return SCHEMA_ERROR;
        }
    }
    else if (json_is(node, arrayOfOptionalStrings))
    {
        for (node = json_child(node); node != NULL; node = json_next(node))
        {
            if ((mask = add_type(json_string(node), mask)) == 0)
            {
                return SCHEMA_ERROR;
            }
        }
    }
    else
    {
        return SCHEMA_ERROR;
    }
    if (iter != NULL)
    {
        unsigned type = json_type(iter);

        /* 'integer' must validate if type is 'number' */
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

static int find_required(const json *node, const json *iter)
{
    for (node = json_child(node); node != NULL; node = json_next(node))
    {
        if (!json_find(iter, json_string(node)))
        {
            return 0;
        }
    }
    return 1;
}

static int test_required(const json *node, const json *iter)
{
    if (json_is(node, arrayOfOptionalStrings))
    {
        if (json_is_object(iter))
        {
            return find_required(node, iter);
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

    int valid = 1;

    for (node = json_child(node); node != NULL; node = json_next(node))
    {
        if (!json_is(node, arrayOfOptionalStrings))
        {
            return SCHEMA_ERROR;
        }
        if (valid && json_is_object(iter))
        {
            if (json_find(iter, json_name(node)) && !find_required(node, iter))
            {
                valid = 0;
            }
        }
    }
    return valid;
}

static int test_dependent_schemas(const json *node, const json *iter)
{
    (void)iter;
    return json_is(node, objectOfOptionalObjects)
        ? SCHEMA_DEPENDENT_SCHEMAS
        : SCHEMA_ERROR;
}

static int test_properties(const json *node, const json *iter)
{
    (void)iter;
    return json_is(node, objectOfOptionalObjects)
        ? SCHEMA_PROPERTIES
        : SCHEMA_ERROR;
}

static int test_pattern_properties(const json *node, const json *iter)
{
    (void)iter;
    return json_is(node, objectOfOptionalObjects)
        ? SCHEMA_PATTERN_PROPERTIES
        : SCHEMA_ERROR;
}

static int test_additional_properties(const json *node, const json *iter)
{
    if (json_is_object(node))
    {
        return SCHEMA_ADDITIONAL_PROPERTIES;
    }
    if (!json_is_boolean(node))
    {
        return SCHEMA_ERROR;
    }
    if (json_is_false(node) && json_is_object(iter))
    {
        const json *properties = json_find(json_parent(node), "properties");

        if (json_is(properties, objectOfOptionalObjects))
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
    if (json_is_boolean(node))
    {
        return json_is_array(iter)
            ? (json_boolean(node) == json_is_any(json_child(iter)))
            : SCHEMA_VALID;
    }
    if (json_is_object(node))
    {
        return SCHEMA_ITEMS;
    }
    if (json_is(node, arrayOfOptionalObjects))
    {
        return SCHEMA_TUPLES;
    }
    return SCHEMA_ERROR;
}

static int test_additional_items(const json *node, const json *iter)
{
    if (json_is_object(node))
    {
        return SCHEMA_ADDITIONAL_ITEMS;
    }
    if (!json_is_boolean(node))
    {
        return SCHEMA_ERROR;
    }
    if (json_is_false(node) && json_is_array(iter))
    {
        const json *items = json_find(json_parent(node), "items");

        if (json_is(items, arrayOfOptionalObjects))
        {
            return json_items(iter) <= json_items(items);
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
    if (json_is_true(node) && json_is_array(iter))
    {
        return json_is(iter, arrayOfUniqueItems);
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
            equal(name, "hostname") ? test_is_hostname :
            equal(name, "email") ? test_is_email :
            equal(name, "ipv4") ? test_is_ipv4 :
            equal(name, "ipv6") ? test_is_ipv6 :
            equal(name, "uuid") ? test_is_uuid :
            equal(name, "url") ? test_is_url : NULL;

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
        if (json_is_true(json_find(json_parent(node), "exclusiveMinimum")))
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
        if (json_is_true(json_find(json_parent(node), "exclusiveMaximum")))
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
    if (json_number(node) <= 0)
    {
        return SCHEMA_ERROR;
    }
    if (json_is_number(iter))
    {
        return fmod(json_number(iter), json_number(node)) == 0;
    }
    return 1;
}

static const json *handle_ref(json_schema *schema,
    const json *node, const json *iter)
{
    const char *ref = json_string(node);
    
    if (ref[0] != '#')
    {
        raise_error(schema, node, iter);
    }

    const json *next = ref[1] ? json_node(node, ref + 1) : schema->root;

    if (!json_is_object(next))
    {
        raise_error(schema, node, iter);
    }
    if (iter == NULL)
    {
        if (schema->skip == node)
        {
            schema->skip = NULL;
            return NULL;
        }
        if (schema->skip == NULL)
        {
            schema->skip = node;
        }
    }
    else
    {
        schema->skip = NULL;
    }
    return next;
}

static int handle_cond(const json **node, int cond)
{
    const json *next = json_next(*node);
    const char *name;

    if (json_is_object(next) && (name = json_name(next)))
    {
        if (equal(name, "then"))
        {
            *node = next;
            return cond;
        }
        if (equal(name, "else"))
        {
            *node = next;
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

static tester get_test(const json *node)
{
    const char *name = json_name(node);

    if (name == NULL)
    {
        return test_error;
    }
    return get_test_by_name(name);
}

static int validate(json_schema *schema,
    const json *node, const json *iter, int flag)
{
    int valid = 1;

    if (schema->depth++ > SCHEMA_MAX_DEPTH)
    {
        raise_error(schema, node, iter);
    }
    while (node != NULL)
    {
        const tester test = get_test(node);

        if (test != NULL)
        {
            switch (test(node, iter))
            {
                case SCHEMA_DEPENDENT_SCHEMAS:
                {
                    const json *next = json_child(node);

                    while (next != NULL)
                    {
                        if (json_find(iter, json_name(next)))
                        {
                            valid &= validate(schema, json_child(next), iter, flag);
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
                    const json *item = json_is_object(iter) ? iter : NULL;
                    const json *next = json_child(node);

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
                        item = json_find(iter, json_name(next));
                        do valid &= validate(schema, json_child(next), item, flag);
                        while ((item = json_find_next(item, json_name(next))));
                        next = json_next(next);
                    }
                }
                break;
                case SCHEMA_PATTERN_PROPERTIES:
                {
                    const json *head = json_is_object(iter) ? json_child(iter) : NULL;
                    const json *next = json_child(node);

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
                    const json *properties = json_find(json_parent(node), "properties");
                    const json *item = json_is_object(iter) ? json_child(iter) : NULL;
                    const json *next = json_child(node);
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
                    const json *item = json_is_array(iter) ? json_child(iter) : NULL;
                    const json *next = json_child(node);

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
                    const json *next = json_child(node);
                    const json *item = NULL;

                    if (json_is_array(iter))
                    {
                        const json *items = json_find(json_parent(node), "items");

                        if (json_is(items, arrayOfOptionalObjects))
                        {
                            item = json_item(iter, json_items(items));
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
                    const json *item = json_is_array(iter) ? json_child(iter) : NULL;
                    const json *next = json_child(node);

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
                    const json *next = handle_ref(schema, node, iter);

                    if (next != NULL)
                    {
                        valid &= validate(schema, json_child(next), iter, flag);
                    }
                }
                break;
                case SCHEMA_NOT:
                {
                    int old_valid = valid;

                    valid = !validate(schema, json_child(node), iter, 1);
                    if (flag == 0)
                    {
                        if (valid)
                        {
                            valid = old_valid;
                        }
                        else
                        {
                            raise_invalid(schema, node, iter);
                        }
                    }
                }
                break;
                case SCHEMA_ALL_OF:
                case SCHEMA_ANY_OF:
                case SCHEMA_ONE_OF:
                {
                    const json *next = json_child(node);
                    int old_valid = valid;
                    int count = 0;

                    valid = 1;
                    while (next != NULL)
                    {
                        if (count++ == 0)
                        {
                            valid = validate(schema, json_child(next), iter, 1);
                        }
                        else if (test == test_all_of)
                        {
                            valid &= validate(schema, json_child(next), iter, 1);
                        }
                        else if (test == test_any_of)
                        {
                            valid |= validate(schema, json_child(next), iter, 1);
                        }
                        else if (test == test_one_of)
                        {
                            valid ^= validate(schema, json_child(next), iter, 1);
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
                            raise_invalid(schema, node, iter);
                        }
                    }
                }
                break;
                case SCHEMA_IF:
                {
                    int cond_valid = validate(schema, json_child(node), iter, 1);
                    int cond;

                    while ((cond = handle_cond(&node, cond_valid)) != -1)
                    {
                        if (cond == 1)
                        {
                            valid &= validate(schema, json_child(node), iter, flag);
                        }
                        else
                        {
                            validate(schema, json_child(node), NULL, 1);
                        }
                    }
                }
                break;
                case SCHEMA_THEN:
                case SCHEMA_ELSE:
                {
                    validate(schema, json_child(node), NULL, 1);
                }
                break;
                case SCHEMA_INVALID:
                {
                    if (flag == 0)
                    {
                        raise_invalid(schema, node, iter);
                    }
                    valid = 0;
                }
                break;
                case SCHEMA_ERROR:
                {
                    raise_error(schema, node, iter);
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
    schema->depth--;
    return valid;
}

int json_validate(const json *iter, const json *root,
    json_callback callback, void *data)
{
    json_schema schema =
    {
        .root = root,
        .callback = callback,
        .data = data
    };

    if (setjmp(schema.error))
    {
        return 0;
    }
    if (json_is_object(root))
    {
        return validate(&schema, json_child(root), iter, 0);
    }
    raise_error(&schema, root, iter);
    return 0;
}

int json_schema_default_callback(const json *node, int depth, void *data)
{
    (void)data;
    (void)depth;
    json_write(node, stderr);
    return 1;
}

