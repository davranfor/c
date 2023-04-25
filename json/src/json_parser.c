/*!
 *  \brief     json and json-schema library for C
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "json_struct.h"

#define is_space(c) isspace((unsigned char)(c))
#define is_cntrl(c) iscntrl((unsigned char)(c))
#define is_digit(c) isdigit((unsigned char)(c))
#define is_xdigit(c) isxdigit((unsigned char)(c))

/* Check whether a character is a json token */
static int is_token(int c)
{
    return (c == '{') || (c == '}')
        || (c == '[') || (c == ']')
        || (c == ':') || (c == ',')
        || (c == '\0');
}

/* Returns the type of a token group character */
static enum json_type token_type(int token)
{
    switch (token)
    {
        default : return JSON_UNDEFINED;
        case '{':
        case '}': return JSON_OBJECT;
        case '[':
        case ']': return JSON_ARRAY;
    }
}

/* Check whether a string is a valid number */
static int is_number(const char *left, const char *right)
{
    char *end;

    strtod(left, &end);
    if (end <= right)
    {
        return 0;
    }
    /* Skip sign */
    if (left[0] == '-')
    {
        left++;
    }
    /* Do not allow padding 0s */
    if ((left[0] == '0') && is_digit(left[1]))
    {
        return 0;
    }
    /* Must start and end with a digit */ 
    if (!is_digit(*left) || !is_digit(*right))
    {
        return 0;
    }
    return 1;
}

/* Check whether a string already tested as a valid number is a double */
static int is_double(const char *left, const char *right)
{
    while (left <= right)
    {
        if ((*left == '.') || (*left == 'e') || (*left == 'E'))
        {
            return 1;
        }
        left++;
    }
    return 0;
}

/* Check whether a character is an escape character */
static int is_esc(const char *str)
{
    char c = *str;

    return (c == '\\') || (c == '/') || (c == '"') ||
           (c == 'b')  || (c == 'f') || (c == 'n') || (c == 'r') || (c == 't');
}

/* Check whether a character is an UCN (Universal character name) "\uxxxx" */
static int is_ucn(const char *str)
{
    return ((*str++) == 'u')
        && is_xdigit(*str++)
        && is_xdigit(*str++)
        && is_xdigit(*str++)
        && is_xdigit(*str);
}

/* Check whether a UCN converted to codepoint is valid */
static int valid_ucn(unsigned c)
{
    if ((c == '\b') || (c == '\f') || (c == '\n') || (c == '\r') || (c == '\t'))
    {
        return 1;
    }
    return !is_cntrl(c);
}

/*
 * Converts UCN to multibyte
 * Returns the length of the multibyte in bytes
 */
static int ucn_to_mb(const char *str, char *buf)
{
    char hex[5] = "";

    /* Copy UCN skipping the initial "u" */
    memcpy(hex, str + 1, 4);

    unsigned codepoint = (unsigned)strtoul(hex, NULL, 16);

    /* Copy "as is" if invalid */
    if (!valid_ucn(codepoint))
    {
        memcpy(buf, str - 1, 6);
        return 6;
    }
    /* Convert to multibyte and return the length */
    if (codepoint <= 0x7f)
    {
        buf[0] = (char)codepoint;
        return 1;
    }
    else if (codepoint <= 0x7ff)
    {
        buf[0] = (char)(0xc0 | ((codepoint >> 6) & 0x1f));
        buf[1] = (char)(0x80 | (codepoint & 0x3f));
        return 2;
    }
    else // if (codepoint <= 0xffff)
    {
        buf[0] = (char)(0xe0 | ((codepoint >> 12) & 0x0f));
        buf[1] = (char)(0x80 | ((codepoint >> 6) & 0x3f));
        buf[2] = (char)(0x80 | (codepoint & 0x3f));
        return 3;
    }
}

/* scan() helpers */

static const char *scan_quoted(const char *str)
{
    while (!is_cntrl(*str))
    {
        if (*str == '"')
        {
            break;
        }
        if (*str == '\\')
        {
            if (is_esc(str + 1))
            {
                str += 2;
                continue;
            }
            if (is_ucn(str + 1))
            {
                str += 6;
                continue;
            }
            break;
        }
        str++;
    }
    return str;
}

static const char *scan_unquoted(const char *str)
{
    while (!is_space(*str) && !is_token(*str))
    {
        if (*str == '"')
        {
            break;
        }
        str++;
    }
    return str;
}

/* Returns a pointer to the next element or NULL on fail */
static const char *scan(const char **left, const char **right)
{
    const char *str = *left;

    /* Skip leading spaces */
    while (is_space(*str))
    {
        str++;
    }
    /* Adjust pointers to token */
    *left = *right = str;
    /* Return on first token */
    if (is_token(*str))
    {
        return str;
    }
    /* Handle name or string scalar */
    if (*str == '"')
    {
        str = scan_quoted(str + 1);
        if (*str != '"')
        {
            goto fail;
        }
        *right = str++;
    }
    else // ... handle other scalars
    {
        str = scan_unquoted(str + 1);
        if (*str == '"')
        {
            goto fail;
        }
        *right = str - 1;
    }
    /* Skip trailing spaces */
    while (is_space(*str))
    {
        str++;
    }
    /* Unexpected character */
    if (!is_token(*str))
    {
        goto fail;
    }
    /* Valid */
    return str;
fail:
    /* Adjust pointers to error */
    *left = *right = str;
    return NULL;
}

/* Allocates space for a name or a scalar value escaping special characters */
static char *copy(const char *str, size_t length)
{
    char *buf = malloc(length + 1);

    if (buf == NULL)
    {
        return NULL;
    }

    const char *end = str + length;
    char *ptr = buf;

    while (str < end)
    {
        if (*str == '\\')
        {
            switch (*++str)
            {
                default : *ptr++ = *str; break;
                case 'b': *ptr++ = '\b'; break;
                case 'f': *ptr++ = '\f'; break;
                case 'n': *ptr++ = '\n'; break;
                case 'r': *ptr++ = '\r'; break;
                case 't': *ptr++ = '\t'; break;
                case 'u':
                    ptr += ucn_to_mb(str, ptr);
                    str += 4;
                    break;
            }
        }
        /* Skip quotes */
        else if (*str != '"')
        {
            *ptr++ = *str;
        }
        str++;
    }
    *ptr = '\0';
    return buf;
}

static char *set_name(json *node, const char *left, const char *right)
{
    size_t length = (size_t)(right - left + 1);

    /* Must start and end with quotes */
    if ((*left != '"') || (*right != '"'))
    {
        return NULL;
    }
    node->name = copy(left, length);
    return node->name;
}

static char *set_value(json *node, const char *left, const char *right)
{
    size_t length = (size_t)(right - left + 1);

    if ((*left == '"') && (*right == '"'))
    {
        node->type = JSON_STRING;
    }
    else if ((length == 4) && (strncmp(left, "null", length) == 0))
    {
        node->type = JSON_NULL;
    }
    else if ((length == 4) && (strncmp(left, "true", length) == 0))
    {
        node->type = JSON_BOOLEAN;
    }
    else if ((length == 5) && (strncmp(left, "false", length) == 0))
    {
        node->type = JSON_BOOLEAN;
    }
    else if (is_number(left, right))
    {
        node->type = is_double(left, right) ? JSON_DOUBLE : JSON_INTEGER;
    }
    else
    {
        return NULL;
    }
    node->value = copy(left, length);
    return node->value;
}

static json *create_node(void)
{
    return calloc(1, sizeof(struct json));
}

/* parse() helpers - node must exist */

static json *create_child(json *parent)
{
    json *child = calloc(1, sizeof(struct json));

    if (child != NULL)
    {
        child->parent = parent;
        parent->child = child;
    }
    return child;
}

static json *delete_child(json *parent)
{
    free(parent->child);
    parent->child = NULL;
    return parent;
}

static json *create_next(json *node)
{
    json *next = calloc(1, sizeof(struct json));

    if (next != NULL)
    {
        next->parent = node->parent;
        node->next = next;
    }
    return next;
}

/* Parse document - returns an error position or NULL on success */
static const char *parse(json *node, const char *left)
{
    const char *right = NULL;
    const char *token;

    while (node != NULL)
    {
        if ((token = scan(&left, &right)) == NULL)
        {
            return left;
        }
        switch (*token)
        {
            case '{':
            case '[':
                /* Commas between groups are required: [[] []] */
                if (node->type != JSON_UNDEFINED)
                {
                    return left;
                }
                /* Contents before groups are not allowed: 1[] */
                if (left != token)
                {
                    return left;
                }
                /* Object properties must have a name */
                if (json_is_object(node->parent) && (node->name == NULL))
                {
                    return token;
                }
                node->type = token_type(*token);
                node = create_child(node);
                break;
            case ':':
                if (left == token)
                {
                    return left;
                }
                /* Only object properties can have a name */
                if (!json_is_object(node->parent) || (node->name != NULL))
                {
                    return token;
                }
                if (set_name(node, left, right) == NULL)
                {
                    return left;
                }
                break;
            case ',':
                if (node->parent == NULL)
                {
                    return token;
                }
                if (node->type == JSON_UNDEFINED)
                {
                    if (left == token)
                    {
                        return left;
                    }
                    if (json_is_object(node->parent) && (node->name == NULL))
                    {
                        return left;
                    }
                    if (set_value(node, left, right) == NULL)
                    {
                        return left;
                    }
                }
                else
                {
                    if (left != token)
                    {
                        return left;
                    }
                }
                node = create_next(node);
                break;
            case ']':
            case '}':
                if (json_type(node->parent) != token_type(*token))
                {
                    return token;
                }
                if (node->type == JSON_UNDEFINED)
                {
                    if (left == token)
                    {
                        /* Remove empty group: {} or [] */
                        if ((node->parent->child == node) && (node->name == NULL))
                        {
                            node = delete_child(node->parent);
                            break;
                        }
                        return left;
                    }
                    if (json_is_object(node->parent) && (node->name == NULL))
                    {
                        return left;
                    }
                    if (set_value(node, left, right) == NULL)
                    {
                        return left;
                    }
                }
                else
                {
                    if (left != token)
                    {
                        return left;
                    }
                }
                node = node->parent;
                break;
            case '\0':
                /* Bad closed document */
                if (node->parent != NULL)
                {
                    return left;
                }
                if (node->type == JSON_UNDEFINED)
                {
                    if (left == token)
                    {
                        return left;
                    }
                    if (set_value(node, left, right) == NULL)
                    {
                        return left;
                    }
                }
                else
                {
                    if (left != token)
                    {
                        return left;
                    }
                }
                /* Correct document */
                return NULL;
        }
        /* Keep going ... */
        left = token + 1;
    }
    return left;
}

static void clear_error(json_error *error)
{
    error->line = error->column = 0;
}

static void set_error(const char *str, const char *end, json_error *error)
{
    error->line = error->column = 1;

    while (str < end)
    {
        if (*str == '\n')
        {
            error->line++;
            error->column = 1;
        }
        /* ASCII character or the first byte of a multibyte character */
        else if ((*str & 0xc0) != 0x80)
        {
            error->column++;
        }
        str++;
    }
}

json *json_parse(const char *str, json_error *error)
{
    if (error != NULL)
    {
        clear_error(error);
    }

    json *node = create_node();

    if (node != NULL)
    {
        const char *end = parse(node, str);

        if (end != NULL)
        {
            if (error != NULL)
            {
                set_error(str, end, error);
            }
            json_free(node);
            return NULL;
        }
    }
    return node;
}

static char *read_file(FILE *file, size_t size)
{
    char *str = malloc(size + 1);

    if (str != NULL)
    {
        if (fread(str, 1, size, file) == size)
        {
            str[size] = '\0';
        }
        else
        {
            free(str);
            str = NULL;
        }
    }
    return str;
}

static char *read_file_from(const char *path)
{
    FILE *file = fopen(path, "rb");

    if (file == NULL)
    {
        return NULL;
    }

    char *str = NULL;

    if (fseek(file, 0L, SEEK_END) != -1)
    {
        long size = ftell(file);

        if ((size != -1) && (fseek(file, 0L, SEEK_SET) != -1))
        {
            str = read_file(file, (size_t)size);
        }
    }
    fclose(file);
    return str;
}

json *json_parse_file(const char *path, json_error *error)
{
    char *str = read_file_from(path);

    if (str == NULL)
    {
        if (error != NULL)
        {
            clear_error(error);
        }
        return NULL;
    }

    json *node = json_parse(str, error);

    free(str);
    return node;
}

void json_print_error(const char *path, const json_error *error)
{
    if ((error == NULL) || (error->line == 0))
    {
        fprintf(stderr, "json:\t%s\n\t%s\n",
            path ? path : "", strerror(errno)
        );
    }
    else
    {
        fprintf(stderr, "json:\t%s\n\tError at line %d, column %d\n",
            path ? path : "", error->line, error->column
        );
    }
}

