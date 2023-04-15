#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json_struct.h"

/* return 0 if buffer_resize() fails */
#define CHECK(expr) do { if (!(expr)) return 0; } while(0)

typedef struct { char *text; size_t length, size; } json_buffer;

static char *buffer_resize(json_buffer *buffer, size_t size)
{
    char *text = realloc(buffer->text, size);

    if (text != NULL)
    {
        buffer->text = text;
        buffer->size = size;
    }
    return text;
}

static size_t buffer_next_size(size_t size)
{
    size--;
    size |= size >> 1;
    size |= size >> 2;
    size |= size >> 4;
    size |= size >> 8;
    size |= size >> 16;
    size++;
    return size;
}

static json_buffer *buffer_write_sized(json_buffer *buffer, const char *text,
    size_t length)
{
    size_t size = buffer->length + length + 1;

    if (size > buffer->size)
    {
        if (!buffer_resize(buffer, buffer_next_size(size)))
        {
            return NULL;
        }
    }
    memcpy(buffer->text + buffer->length, text, length + 1);
    buffer->length += length;
    return buffer;
}

static json_buffer *buffer_write(json_buffer *buffer, const char *text)
{
    return buffer_write_sized(buffer, text, strlen(text));
}

static int buffer_parse(json_buffer *buffer, const char *str)
{
    const char *ptr = str;

    while (*str != '\0')
    {
        char chr = '\0';

        switch (*str)
        {
            case '\\': chr = (str[1] != 'u') ? '\\': '\0'; break;
            case '"' : chr = '"'; break;
            case '\b': chr = 'b'; break;
            case '\f': chr = 'f'; break;
            case '\n': chr = 'n'; break;
            case '\r': chr = 'r'; break;
            case '\t': chr = 't'; break;
            default: break;
        }
        if (chr != '\0')
        {
            const char esc[] = {'\\', chr, '\0'};

            CHECK(buffer_write_sized(buffer, ptr, (size_t)(str - ptr)));
            CHECK(buffer_write_sized(buffer, esc, 2));
            ptr = ++str;
        }
        else
        {
            str++;
        }
    }
    CHECK(buffer_write_sized(buffer, ptr, (size_t)(str - ptr)));
    return 1;
}

static int buffer_quote(json_buffer *buffer, const char *text)
{
    CHECK(buffer_write(buffer, "\""));
    CHECK(buffer_parse(buffer, text));
    CHECK(buffer_write(buffer, "\""));
    return 1;
}

static int buffer_write_node(json_buffer *buffer, const json *node, int depth)
{
    for (int i = 0; i < depth; i++)
    {
        CHECK(buffer_write(buffer, "  "));
    }
    if (node->name != NULL)
    {
        CHECK(buffer_quote(buffer, node->name));
        CHECK(buffer_write(buffer, ": "));
    }
    switch (node->type)
    {
        case JSON_OBJECT:
            CHECK(buffer_write(buffer, "{"));
            break;
        case JSON_ARRAY:
            CHECK(buffer_write(buffer, "["));
            break;
        case JSON_STRING:
            CHECK(buffer_quote(buffer, node->value));
            break;
        default:
            CHECK(buffer_write(buffer, node->value));
            break;
    }
    if (node->child == NULL)
    {
        /* Prints an empty "object" or an empty "array" {} [] */
        switch (node->type)
        {
            case JSON_OBJECT:
                CHECK(buffer_write(buffer, "}"));
                break;
            case JSON_ARRAY:
                CHECK(buffer_write(buffer, "]"));
                break;
            default:
                break;
        }
        if ((depth > 0) && (node->next != NULL))
        {
            CHECK(buffer_write(buffer, ","));
        }
    }
    CHECK(buffer_write(buffer, "\n"));
    return 1;
}

static int buffer_write_next(json_buffer *buffer, const json *node, int depth)
{
    /* if "array" or "object" */
    if (node->child != NULL)
    {
        for (int i = 0; i < depth; i++)
        {
            CHECK(buffer_write(buffer, "  "));
        }
        switch (node->type)
        {
            case JSON_OBJECT:
                CHECK(buffer_write(buffer, "}"));
                break;
            case JSON_ARRAY:
                CHECK(buffer_write(buffer, "]"));
                break;
            default:
                break;
        }
        if ((depth > 0) && (node->next != NULL))
        {
           CHECK(buffer_write(buffer, ","));
        }
        CHECK(buffer_write(buffer, "\n"));
    }
    return 1;
}

static int buffer_encode(json_buffer *buffer, const json *node)
{
    int depth = 0;

    while (node != NULL)
    {
        loop:
        if (!buffer_write_node(buffer, node, depth))
        {
            return 0;
        }
        if (node->child != NULL)
        {
            node = node->child;
            depth++;
        }
        else if ((depth > 0) && (node->next != NULL))
        {
            node = node->next;
        }
        else
        {
            while (depth-- > 0)
            {
                node = node->parent;
                if (!buffer_write_next(buffer, node, depth))
                {
                    return 0;
                }
                if (node->next != NULL)
                {
                    node = node->next;
                    goto loop;
                }
            }
            break;
        }
    }
    return 1;
}

char *json_encode(const json *node)
{
    json_buffer buffer = {NULL, 0, 0};
    char *text = NULL;

    if (buffer_encode(&buffer, node))
    {
        text = buffer.text;
    }
    else
    {
        free(buffer.text);
    }
    return text;
}

char *json_encode_string(const json *node)
{
    if ((node == NULL) || (node->value == NULL))
    {
        return NULL;
    }

    json_buffer buffer = {NULL, 0, 0};
    char *text = NULL;

    if (buffer_parse(&buffer, node->value))
    {
        text = buffer.text;
    }
    else
    {
        free(buffer.text);
    }
    return text;
}

int json_write(const json *node, FILE *file)
{
    char *str = json_encode(node);

    if (str == NULL)
    {
        return 0;
    }

    int rc = fputs(str, file) != EOF;

    free(str);
    return rc;
}

int json_print(const json *node)
{
    return json_write(node, stdout);
}

static int buffer_write_path(json_buffer *buffer, const json *node)
{
    if (node->parent == NULL)
    {
        CHECK(buffer_write(buffer, "$"));
    }
    else
    {
        if (node->name != NULL)
        {
            CHECK(buffer_write(buffer, "[\""));
            CHECK(buffer_parse(buffer, node->name));
            CHECK(buffer_write(buffer, "\"]"));
        }
        if (node->parent->type == JSON_ARRAY)
        {
            char str[32];

            snprintf(str, sizeof str, "[%zu]", json_offset(node));
            CHECK(buffer_write(buffer, str));
        }
    }
    return 1;
}

static int buffer_path(json_buffer *buffer, const json *node)
{
    if (node == NULL)
    {
        return 1;
    }
    if (!buffer_path(buffer, node->parent))
    {
        return 0;
    }
    return buffer_write_path(buffer, node);
}

char *json_path(const json *node)
{
    json_buffer buffer = {NULL, 0, 0};
    char *text = NULL;

    if (buffer_path(&buffer, node))
    {
        text = buffer.text;
    }
    else
    {
        free(buffer.text);
    }
    return text;
}

