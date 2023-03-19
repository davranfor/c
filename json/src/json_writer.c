#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json_struct.h"

/* Macros checking if buffers allocations succeeds */
#define BUFFER_WRITE(buffer, text)                  \
    if (!buffer_write(buffer, text))                \
    {                                               \
        return 0;                                   \
    }
#define BUFFER_WRITE_LENGTH(buffer, text, length)   \
    if (!buffer_write_length(buffer, text, length)) \
    {                                               \
        return 0;                                   \
    }
#define BUFFER_QUOTE(buffer, text)                  \
    if (!buffer_write_length(buffer, "\"", 1))      \
    {                                               \
        return 0;                                   \
    }                                               \
    if (!buffer_write_string(buffer, text))         \
    {                                               \
        return 0;                                   \
    }                                               \
    if (!buffer_write_length(buffer, "\"", 1))      \
    {                                               \
        return 0;                                   \
    }

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

static json_buffer *buffer_write_length(json_buffer *buffer,
    const char *text, size_t length)
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
    return buffer_write_length(buffer, text, strlen(text));
}

static int buffer_write_string(json_buffer *buffer, const char *str)
{
    const char *ptr = str;

    while (*str != '\0')
    {
        char chr = '\0';

        switch (*str)
        {
            case '\\': chr = (str[1] != 'u') ? '\\': '\0'; break;
            case '"' : chr = '"' ; break;
            case '\b': chr = 'b' ; break;
            case '\f': chr = 'f' ; break;
            case '\n': chr = 'n' ; break;
            case '\r': chr = 'r' ; break;
            case '\t': chr = 't' ; break;
            default: break;
        }
        if (chr != '\0')
        {
            const char esc[] = {'\\', chr, '\0'};

            BUFFER_WRITE_LENGTH(buffer, ptr, (size_t)(str - ptr));
            BUFFER_WRITE_LENGTH(buffer, esc, 2);
            ptr = ++str;
        }
        else
        {
            str++;
        }
    }
    BUFFER_WRITE_LENGTH(buffer, ptr, (size_t)(str - ptr));
    return 1;
}

static int buffer_print_node(json_buffer *buffer, const json *node, int depth)
{
    for (int i = 0; i < depth; i++)
    {
        BUFFER_WRITE(buffer, "  ");
    }
    if (node->name != NULL)
    {
        BUFFER_QUOTE(buffer, node->name);
        BUFFER_WRITE(buffer, ": ");
    }
    switch (node->type)
    {
        case JSON_OBJECT:
            BUFFER_WRITE(buffer, "{");
            break;
        case JSON_ARRAY:
            BUFFER_WRITE(buffer, "[");
            break;
        case JSON_STRING:
            BUFFER_QUOTE(buffer, node->value);
            break;
        default:
            BUFFER_WRITE(buffer, node->value);
            break;
    }
    if (node->left == NULL)
    {
        /* Prints an empty "object" or an empty "array" {} [] */
        switch (node->type)
        {
            case JSON_OBJECT:
                BUFFER_WRITE(buffer, "}");
                break;
            case JSON_ARRAY:
                BUFFER_WRITE(buffer, "]");
                break;
            default:
                break;
        }
        if ((depth > 0) && (node->right != NULL))
        {
            BUFFER_WRITE(buffer, ",");
        }
    }
    BUFFER_WRITE(buffer, "\n");
    return 1;
}

static int buffer_next_node(json_buffer *buffer, const json *node, int depth)
{
    /* if "array" or "object" */
    if (node->left != NULL)
    {
        for (int i = 0; i < depth; i++)
        {
            BUFFER_WRITE(buffer, "  ");
        }
        switch (node->type)
        {
            case JSON_OBJECT:
                BUFFER_WRITE(buffer, "}");
                break;
            case JSON_ARRAY:
                BUFFER_WRITE(buffer, "]");
                break;
            default:
                break;
        }
        if ((depth > 0) && (node->right != NULL))
        {
           BUFFER_WRITE(buffer, ",");
        }
        BUFFER_WRITE(buffer, "\n");
    }
    return 1;
}

static int buffer_encode(json_buffer *buffer, const json *node)
{
    int depth = 0;

    while (node != NULL)
    {
        loop:
        if (!buffer_print_node(buffer, node, depth))
        {
            return 0;
        }
        if (node->left != NULL)
        {
            node = node->left;
            depth++;
        }
        else if ((depth > 0) && (node->right != NULL))
        {
            node = node->right;
        }
        else
        {
            while (depth-- > 0)
            {
                node = node->parent;
                if (!buffer_next_node(buffer, node, depth))
                {
                    return 0;
                }
                if (node->right != NULL)
                {
                    node = node->right;
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

