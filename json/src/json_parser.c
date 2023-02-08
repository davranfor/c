/*! 
 *  \brief     JSON parser
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "json_struct.h"
#include "json_parser.h"

#define is_space(c) isspace((unsigned char)(c))
#define is_cntrl(c) iscntrl((unsigned char)(c))
#define is_digit(c) isdigit((unsigned char)(c))
#define is_xdigit(c) isxdigit((unsigned char)(c))

/* Check wether a character is an escape character */
static int is_escape(int c)
{
    return (c == '\\') || (c == '/') || (c == '"') ||
           (c == 'b')  || (c == 'f') || (c == 'n') || (c == 'r') || (c == 't');
}

/* Check wether a character is a control character */
static int is_control(int c)
{
    return (c == '\\') || is_cntrl(c);
}

/* Check wether a character is an UCN (Universal character name) "\uxxxx" */
static int is_ucn(const char *str)
{
    return (*str++ == 'u')
        && is_xdigit(*str++)
        && is_xdigit(*str++)
        && is_xdigit(*str++)
        && is_xdigit(*str);
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

    unsigned long codepoint = strtoul(hex, NULL, 16);

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

/* Check wether a character is a json token */
static int is_token(int c)
{
    return (c == '{') || (c == '}')
        || (c == '[') || (c == ']')
        || (c == ':') || (c == ',');
}

/* Returns the type of a token group character */
static enum json_type token_type(int token)
{
    switch (token)
    {
        case '{':
        case '}':
            return JSON_OBJECT;
        case '[':
        case ']':
            return JSON_ARRAY;
        default:
            return JSON_UNDEFINED;
    }
}

/* Check wether a string is a valid number */
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

/* Check wether a string already tested as a valid number is a double */
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

/* Returns a pointer to the next element or NULL on fail */
static const char *scan(const char **left, const char **right)
{
    int tokens = 0, quotes = 0;
    const char *str = *left;

    while (*str != '\0')
    {
        if (*str == '\\')
        {
            if (quotes == 1)
            {
                str++;
                if (is_escape(*str))
                {
                    str += 1;
                    continue;
                }
                if (is_ucn(str))
                {
                    str += 5;
                    continue;
                }
            }
            return NULL;
        }
        if (*str == '"')
        {
            /* Multiple strings are not allowed: <"abc" "def"> */
            if (quotes > 1)
            {
                return NULL;
            }
            /* Change state */
            quotes++;
        }
        /* Breaks on the first occurrence of a token outside a string */
        else if (quotes != 1)
        {
            if (is_token(*str))
            {
                break;
            }
        }
        /* Control characters must be escaped inside a string */
        else if (is_control(*str))
        {
            return NULL;
        }
        if (!is_space(*str))
        {
            /* Update left pointer */
            if (tokens == 0)
            {
                *left = str;
                tokens = 1;
            }
            /* Right pointer is always updated */
            *right = str;
        }
        str++;
    }
    /* Quotes are not closed */
    if (quotes == 1)
    {
        return NULL;
    }
    /* There are no contents in front of the token */
    if (tokens == 0)
    {
        *left = *right = str;
    }
    return str;
}

/* Allocates space for a 'name' or a 'value' escaping special characters */
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
                case '\\':
                case '/':
                case '"':
                    *ptr++ = *str;
                    break;
                case 'b':
                    *ptr++ = '\b';
                    break;
                case 'f':
                    *ptr++ = '\f';
                    break;
                case 'n':
                    *ptr++ = '\n';
                    break;
                case 'r':
                    *ptr++ = '\r';
                    break;
                case 't':
                    *ptr++ = '\t';
                    break;
                case 'u':
                    ptr += ucn_to_mb(str, ptr);
                    str += 4;
                    break;
                default:
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

static const char *parse(json *node, const char *left)
{
    const json *parent = node ? node->parent : NULL;
    const char *right;
    const char *token;

    #define ERROR left

    while (node != NULL)
    {
        if ((token = scan(&left, &right)) == NULL)
        {
            return ERROR;
        }
        switch (*token)
        {
            case '{':
            case '[':
                /* Object properties must have a name */
                if ((node->parent != NULL) &&
                    (node->parent->type == JSON_OBJECT) &&
                    (node->name == NULL))
                {
                    return ERROR;
                }
                /* Commas between groups are required, e.g.: [[] []] */
                if (node->type != JSON_UNDEFINED)
                {
                    return ERROR;
                }
                /* if there is text before the token, e.g.: {"abc":1{}} */
                if (left != token)
                {
                    return ERROR;
                }
                node->type = token_type(*token);
                /* Creates a left node (child) */
                node->left = create_node();
                if (node->left == NULL)
                {
                    return ERROR;
                }
                node->left->parent = node;
                node = node->left;
                break;
            case ':':
                /* Only object properties can have a name */
                if ((node->parent == NULL) ||
                    (node->parent->type != JSON_OBJECT) ||
                    (node->name != NULL))
                {
                    return ERROR;
                }
                if (set_name(node, left, right) == NULL)
                {
                    return ERROR;
                }
                break;
            case ',':
                /* Object properties must have a name */
                if ((node->parent == NULL) ||
                   ((node->parent->type == JSON_OBJECT) && (node->name == NULL)))
                {
                    return ERROR;
                }
                if (node->type == JSON_UNDEFINED)
                {
                    if (left == token)
                    {
                        return ERROR;
                    }
                    if (set_value(node, left, right) == NULL)
                    {
                        return ERROR;
                    }
                }
                else if (left != token)
                {
                    return ERROR;
                }
                /* Creates a right node (sibling) */
                node->right = create_node();
                if (node->right == NULL)
                {
                    return ERROR;
                }
                node->right->parent = node->parent;
                node = node->right;
                break;
            case ']':
            case '}':
                /* For every close there must be an open of the same type */
                if ((node->parent == NULL) ||
                    (node->parent->type != token_type(*token)))
                {
                    return ERROR;
                }
                if (node->type == JSON_UNDEFINED)
                {
                    if (left == token)
                    {
                        /* Can be an empty group {} or [] */
                        if (node->parent->left != node)
                        {
                            return ERROR;
                        }
                    }
                    else
                    {
                        /* Object properties must have a name */
                        if ((node->parent->type == JSON_OBJECT) &&
                            (node->name == NULL))
                        {
                            return ERROR;
                        }
                        if (set_value(node, left, right) == NULL)
                        {
                            return ERROR;
                        }
                    }
                }
                else if (left != token)
                {
                    return ERROR;
                }
                node = node->parent;
                /* Removes empty group nodes: {} or [] */
                if (node->left->type == JSON_UNDEFINED)
                {
                    free(node->left);
                    node->left = NULL;
                }
                break;
            /* We have reached the end */
            default:
                if (left != token)
                {
                    /*
                     * A json document can consist of a single value
                     * e.g.: "Text" or 123
                     */
                    if (node->type != JSON_UNDEFINED)
                    {
                        return ERROR;
                    }
                    if (set_value(node, left, right) == NULL)
                    {
                        return ERROR;
                    }
                }
                /* A json document can't be empty */
                if (node->type == JSON_UNDEFINED)
                {
                    return ERROR;
                }
                /* Bad closed document */
                if (node->parent != parent)
                {
                    return ERROR;
                }
                /* Correct document */
                return NULL;
        }
        /* Keep going ... */
        left = token + 1;
    }
    return ERROR;
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

/*
 * Returns:
 * - A reference to parent->left if parent is empty
 * - A reference to the last child->right otherwise
 * parent can not be NULL
 */
static json **get_link(json *parent)
{
    json *node;

    if ((node = parent->left))
    {
        while (node->right != NULL)
        {
            node = node->right;
        }
    }
    return node ? &node->right : &parent->left;
}

json *json_new(json *parent, const char *text)
{
    if (parent == NULL)
    {
        return json_parse(text, NULL);
    }
    if ((parent->type != JSON_OBJECT) && (parent->type != JSON_ARRAY))
    {
        return NULL;
    }

    json *node = create_node();

    if (node != NULL)
    {
        json **link = get_link(parent);

        *link = node;
        node->parent = parent;
        if (parse(node, text) != NULL)
        {
            json_free(node);
            *link = NULL;
            return NULL;
        }
    }
    return node;
}

json *json_parse(const char *text, json_error *error)
{
    if (error)
    {
        clear_error(error);
    }

    json *node = create_node();

    if (node != NULL)
    {
        const char *end = parse(node, text);

        if (end != NULL)
        {
            if (error)
            {
                set_error(text, end, error);
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
        if (error)
        {
            error->line = 0;
            error->column = 0;
        }
        return NULL;
    }

    json *node = json_parse(str, error);

    free(str);
    return node;
}

void json_raise_error(const json_error *error, const char *path)
{
    if (error)
    {
        if (error->line == 0)
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
    else
    {
        fprintf(stderr, "json: %s\n", path ? path : "Unhandled error");
    }
}

void json_free(json *node)
{
    json *parent = node ? node->parent : NULL;
    json *next;

    while (node != parent)
    {
        next = node->left;
        node->left = NULL;
        if (next == NULL)
        {
            if (node->right != NULL)
            {
                next = node->right;
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

