#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "json_struct.h"

static int valid_char(char c)
{
    if (!iscntrl((unsigned char)c))
    {
        return 1;
    }
    return (c == '\b') || (c == '\f') || (c == '\n') || (c == '\r') || (c == '\t');
}

static size_t copy_size(const char *str)
{
    const char *ptr = str;

    while (*str != '\0')
    {
        if (!valid_char(*str))
        {
            return 0;
        }
        str++;
    }
    return (size_t)(str - ptr) + 1;
}

static char *copy(const char *str)
{
    size_t size = copy_size(str);

    if (size == 0)
    {
        return NULL;
    }

    char *ptr = malloc(size);

    if (ptr != NULL)
    {
        memcpy(ptr, str, size);
    }
    return ptr;
}

static json *new_type(enum json_type type, const char *key, char *value)
{
    char *name = NULL;

    if (key != NULL)
    {
        name = copy(key);
        if (name == NULL)
        {
            free(value);
            return NULL;
        }
    }

    json *node = calloc(1, sizeof *node);

    if (node != NULL)
    {
        node->type = type;
        node->name = name;
        node->value = value;
    }
    else
    {
        free(name);
        free(value);
    }
    return node;
}

json *json_new_object(const char *name)
{
    return new_type(JSON_OBJECT, name, NULL);
}

json *json_new_array(const char *name)
{
    return new_type(JSON_ARRAY, name, NULL);
}

json *json_new_string(const char *name, const char *value)
{
    char *str = copy(value);

    if (str == NULL)
    {
        return NULL;
    }
    return new_type(JSON_STRING, name, str);
}

json *json_new_integer(const char *name, long long value)
{
    size_t size = 1 + (size_t)snprintf(NULL, 0, "%lld", value);
    char *str = malloc(size);

    if (str == NULL)
    {
        return NULL;
    }
    snprintf(str, size, "%lld", value);
    return new_type(JSON_INTEGER, name, str);
}

json *json_new_real(const char *name, unsigned long long value)
{
    size_t size = 1 + (size_t)snprintf(NULL, 0, "%llu", value);
    char *str = malloc(size);

    if (str == NULL)
    {
        return NULL;
    }
    snprintf(str, size, "%llu", value);
    return new_type(JSON_INTEGER, name, str);
}

json *json_new_double(const char *name, double value, int decimals)
{
    size_t size = 1 + (size_t)snprintf(NULL, 0, "%.*f", decimals, value);
    char *str = malloc(size);

    if (str == NULL)
    {
        return NULL;
    }
    snprintf(str, size, "%.*f", decimals, value);
    return new_type(decimals == 0 ? JSON_INTEGER : JSON_DOUBLE, name, str);
}

json *json_new_boolean(const char *name, int value)
{
    char *str = value ? copy("true") : copy("false");

    if (str == NULL)
    {
        return NULL;
    }
    return new_type(JSON_BOOLEAN, name, str);
}

json *json_new_null(const char *name)
{
    char *str = copy("null");

    if (str == NULL)
    {
        return NULL;
    }
    return new_type(JSON_NULL, name, str);
}

json *json_push_front(json *parent, json *child)
{
    if (!json_is_iterable(parent) || (child == NULL))
    {
        return NULL;
    }
    if ((parent->type == JSON_OBJECT) ^ (child->name != NULL))
    {
        return NULL;
    }
    child->next = parent->child;
    child->parent = parent;
    parent->child = child;
    return child;
}

json *json_push_back(json *parent, json *child)
{
    if (!json_is_iterable(parent) || (child == NULL))
    {
        return NULL;
    }
    if ((parent->type == JSON_OBJECT) ^ (child->name != NULL))
    {
        return NULL;
    }
    child->parent = parent;
    if (parent->child == NULL)
    {
        parent->child = child;
    }
    else
    {
        json_last(parent)->next = child;
    }
    return child;
}

json *json_push_fast(json *parent, json *node, json *next)
{
    if (node == NULL)
    {
        return json_push_back(parent, next);
    }
    else
    {
        return json_append_to(node, next);
    }
}

json *json_append_to(json *node, json *next)
{
    json *parent = json_parent(node);

    if ((parent == NULL) || (next == NULL))
    {
        return NULL;
    }
    if ((parent->type == JSON_OBJECT) ^ (next->name != NULL))
    {
        return NULL;
    }
    next->parent = parent;
    next->next = node->next;
    node->next = next;
    return next;
}

void json_free(json *node)
{
    json *parent = node ? node->parent : NULL;
    json *next;

    while (node != parent)
    {
        next = node->child;
        node->child = NULL;
        if (next == NULL)
        {
            if (node->next != NULL)
            {
                next = node->next;
            }
            else
            {
                next = node->parent;
            }
            free(node->name);
            free(node->value);
            free(node);
        }
        node = next;
    }
}

