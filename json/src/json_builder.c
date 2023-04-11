#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "json_struct.h"

static int must_escape(char c)
{
    return iscntrl((unsigned char)c)
        ? (c != '\b') && (c != '\f') && (c != '\n') && (c != '\r') && (c != '\t')
        : 0;
}

static size_t copy_length(const char *str)
{
    size_t length = 0;

    while (*str != '\0')
    {
        if (must_escape(*str))
        {
            length += 6;
        }
        else
        {
            length += 1;
        }
        str++;
    }
    return length;
}

static char *copy(const char *str)
{
    size_t length = copy_length(str);
    char *ptr, *res;

    ptr = res = malloc(length + 1);
    if (ptr == NULL)
    {
        return NULL;
    }
    while (*str != '\0')
    {
        if (must_escape(*str))
        {
            snprintf(ptr, 7, "\\u%04x", *str);
            ptr += 6;
        }
        else
        {
            *ptr++ = *str;
        }
        str++;
    }
    *ptr = '\0';
    return res;
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

/*
 * Returns:
 * - A reference to parent->child if parent is empty
 * - A reference to the last child->next otherwise
 * parent can not be NULL
 */
static json **link(json *parent)
{
    json *node = parent->child;

    if (node != NULL)
    {
        while (node->next != NULL)
        {
            node = node->next;
        }
    }
    return node ? &node->next : &parent->child;
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
    *link(parent) = child;
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
