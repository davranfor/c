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
#include "json.h"

struct json
{
    char *name;
    char *value;
    json *left;
    json *right;
    json *parent;
    enum json_type type;
};

static const char *type_name[] =
{
    "Undefined",
    "Object",
    "Array",
    "String",
    "Integer",
    "Double",
    "Boolean",
    "Null"
};

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

enum json_type json_type(const json *node)
{
    if (node == NULL)
    {
        return JSON_UNDEFINED;
    }
    return node->type;
}

const char *json_type_name(const json *node)
{
    if (node == NULL)
    {
        return NULL;
    }
    return type_name[node->type];
}

const char *json_name(const json *node)
{
    if (node == NULL)
    {
        return NULL;
    }
    return node->name;
}

const char *json_string(const json *node)
{
    if (node == NULL)
    {
        return NULL;
    }
    return node->value;
}

long json_integer(const json *node)
{
    if ((node == NULL) || (node->value == NULL))
    {
        return 0;
    }
    return strtol(node->value, NULL, 10);
}

double json_double(const json *node)
{
    if ((node == NULL) || (node->value == NULL))
    {
        return 0.0;
    }
    return strtod(node->value, NULL);
}

double json_number(const json *node)
{
    if ((node == NULL) || (node->value == NULL))
    {
        return 0.0;
    }
    return strtod(node->value, NULL);
}

unsigned long json_real(const json *node)
{
    if ((node == NULL) || (node->value == NULL))
    {
        return 0;
    }
    return strtoul(node->value, NULL, 10);
}

int json_boolean(const json *node)
{
    if (node == NULL)
    {
        return 0;
    }
    if (node->type == JSON_BOOLEAN)
    {
        return node->value[0] == 't';
    }
    if (node->value != NULL)
    {
        return strtod(node->value, NULL) != 0.0;
    }
    return 0;
}

int json_is_any(const json *node)
{
    return node != NULL;
}

int json_is_object(const json *node)
{
    return (node != NULL)
        && (node->type == JSON_OBJECT);
}

int json_is_array(const json *node)
{
    return (node != NULL)
        && (node->type == JSON_ARRAY);
}

int json_is_string(const json *node)
{
    return (node != NULL)
        && (node->type == JSON_STRING);
}

int json_is_integer(const json *node)
{
    return (node != NULL)
        && (node->type == JSON_INTEGER);
}

int json_is_double(const json *node)
{
    return (node != NULL)
        && (node->type == JSON_DOUBLE);
}

int json_is_number(const json *node)
{
    return !(node == NULL)
        && ((node->type == JSON_INTEGER) ||
            (node->type == JSON_DOUBLE));
}

int json_is_real(const json *node)
{
    return (node != NULL)
        && (node->type == JSON_INTEGER)
        && (strtol(node->value, NULL, 10) >= 0);
}

int json_is_boolean(const json *node)
{
    return (node != NULL)
        && (node->type == JSON_BOOLEAN);
}

int json_is_true(const json *node)
{
    return (node != NULL)
        && (node->type == JSON_BOOLEAN)
        && (node->value[0] == 't');
}

int json_is_false(const json *node)
{
    return (node != NULL)
        && (node->type == JSON_BOOLEAN)
        && (node->value[0] == 'f');
}

int json_is_null(const json *node)
{
    return (node != NULL)
        && (node->type == JSON_NULL);
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

json *json_self(const json *node)
{
    /* Silence compiler due to const to non-const conversion */
    union {const json *constant; json *not_constant;} cast_to = {node};

    (void) cast_to.constant;
    return cast_to.not_constant;
}

json *json_root(const json *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    json *root = node->parent;

    if (root == NULL)
    {
        return json_self(node);
    }
    while (root->parent != NULL)
    {
        root = root->parent;
    }
    return root;
}

json *json_parent(const json *node)
{
    if (node == NULL)
    {
        return NULL;
    }
    return node->parent;
}

json *json_child(const json *node)
{
    if (node == NULL)
    {
        return NULL;
    }
    return node->left;
}

json *json_next(const json *node)
{
    if (node == NULL)
    {
        return NULL;
    }
    return node->right;
}

/* Locates a child node by name */
json *json_find(const json *root, const char *name)
{
    json *node;

    if ((root != NULL) && (node = root->left))
    {
        while (node != NULL)
        {
            if ((node->name != NULL) && (strcmp(node->name, name) == 0))
            {
                return node;
            }
            node = node->right;
        }
    }
    return NULL;
}

/* Locates the next sibling by name */
json *json_find_next(const json *root, const char *name)
{
    json *node;

    if ((root != NULL) && (node = root->right))
    {
        while (node != NULL)
        {
            if ((node->name != NULL) && (strcmp(node->name, name) == 0))
            {
                return node;
            }
            node = node->right;
        }
    }
    return NULL;
}

/*
 * Locates a child node by name given a name length
 * Useful to stop comparing when there is more text after the name
 */
json *json_match(const json *root, const char *name, size_t length)
{
    json *node;

    if ((root != NULL) && (node = root->left))
    {
        while (node != NULL)
        {
            if ((node->name != NULL) &&
                (strncmp(node->name, name, length) == 0) &&
                (node->name[length] == '\0'))
            {
                return node;
            }
            node = node->right;
        }
    }
    return NULL;
}

/* Locates a node by path */
json *json_node(const json *root, const char *path)
{
    json *node = NULL;

    if (path[0] == '/')
    {
        node = json_root(root);
        path += 1;
    }
    else
    {
        node = json_self(root);
    }
    while ((node != NULL) && (*path != '\0'))
    {
        const char *end = path + strcspn(path, "/");
        size_t length = (size_t)(end - path);

        /* Locate by #item */
        if ((node->type == JSON_ARRAY) && (strspn(path, "0123456789") == length))
        {
            node = json_item(node, strtoul(path, NULL, 10));
        }
        /* . Current node */
        else if ((length == 1) && (path[0] == '.'))
        {
            /* noop */
        }
        /* .. Parent node */
        else if ((length == 2) && (path[0] == '.') && (path[1] == '.'))
        {
            node = node->parent;
        }
        /* Locate by name */
        else
        {
            node = json_match(node, path, length);
        }
        /* Adjust pointer to path */
        path = (*end == '\0') ? end : end + 1;
    }
    return node;
}

/* Locates an item by offset */
json *json_item(const json *root, size_t item)
{
    json *node;

    if ((root != NULL) && (node = root->left))
    {
        size_t count = 0;

        while (node != NULL)
        {
            if (count++ == item)
            {
                return node;
            }
            node = node->right;
        }
    }
    return NULL;
}

size_t json_items(const json *node)
{
    size_t count = 0;

    if ((node != NULL) && (node = node->left))
    {
        while (node != NULL)
        {
            node = node->right;
            count++;
        }
    }
    return count;
}

static int equal(const json *a, const json *b, int depth)
{
    if (a->type != b->type)
    {
        return 0;
    }
    if ((a->left == NULL) ^ (b->left == NULL))
    {
        return 0;
    }
    if (depth > 0)
    {
        if ((a->right == NULL) ^ (b->right == NULL))
        {
            return 0;
        }
        if ((a->name == NULL) ^ (b->name == NULL))
        {
            return 0;
        }
        if ((a->name != NULL) && strcmp(a->name, b->name))
        {
            return 0;
        }
    }
    if ((a->value == NULL) ^ (b->value == NULL))
    {
        return 0;
    }
    if (a->value != NULL)
    {
        if (json_is_number(a))
        {
            return strtod(a->value, NULL) == strtod(b->value, NULL);
        }
        else
        {
            return strcmp(a->value, b->value) == 0;
        }
    }
    return 1;
}

/*
 * Compares two nodes
 * Returns 1 when nodes are equal, 0 otherwise
 */
int json_equal(const json *a, const json *b)
{
    if ((a == NULL) & (b == NULL))
    {
        return 1;
    }
    if ((a == NULL) ^ (b == NULL))
    {
        return 0;
    }

    int depth = 0;

    while (equal(a, b, depth))
    {
        if (a->left != NULL)
        {
            depth++;
            a = a->left;
            b = b->left;
        }
        else if ((depth > 0) && (a->right != NULL))
        {
            a = a->right;
            b = b->right;
        }
        else
        {
            while (depth > 0)
            {
                depth--;
                a = a->parent;
                b = b->parent;
                if (a->right != NULL)
                {
                    a = a->right;
                    b = b->right;
                    goto end;
                }
            }
            return 1;
        }
        end:;
    }
    return 0;
}

/*
 * Sends all nodes to a callback "func"
 * Exit when all nodes are read or when "func" returns a non 0 value
 */
int json_traverse(const json *root, json_callback func, void *data)
{
    const json *node = root;
    int result;

    if (node == NULL)
    {
        return 0;
    }
    while (1)
    {
        loop:
        if ((result = func(node, data)))
        {
            return result;
        }
        if (node->left != NULL)
        {
            node = node->left;
        }
        else if ((node != root) && (node->right != NULL))
        {
            node = node->right;
        }
        else
        {
            while (node->parent != root->parent)
            {
                node = node->parent;
                if (node->right != NULL)
                {
                    node = node->right;
                    goto loop;
                }
            }
            return 0;
        }
    }
    return 0;
}

#define JSON_BUFFER_DEFAULT_SIZE 16

typedef struct { char *text; size_t length, size; } json_buffer;

static json_buffer *buffer_new(void)
{
    json_buffer *buffer = calloc(1, sizeof *buffer);

    if (buffer != NULL)
    {
        buffer->size = JSON_BUFFER_DEFAULT_SIZE;
        buffer->text = malloc(buffer->size);
        if (buffer->text == NULL)
        {
            free(buffer);
            return NULL;
        }
        buffer->text[0] = '\0';
    }
    return buffer;
}

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

#define BUFFER_WRITE(text)                          \
    if (!buffer_write(buffer, text))                \
    {                                               \
        return 0;                                   \
    }

#define BUFFER_WRITE_LENGTH(text, length)           \
    if (!buffer_write_length(buffer, text, length)) \
    {                                               \
        return 0;                                   \
    }

#define BUFFER_QUOTE(text)                          \
    if (!buffer_write_length(buffer, "\"", 1))      \
    {                                               \
        return 0;                                   \
    }                                               \
    if (!buffer_encode(buffer, text))               \
    {                                               \
        return 0;                                   \
    }                                               \
    if (!buffer_write_length(buffer, "\"", 1))      \
    {                                               \
        return 0;                                   \
    }

static int buffer_encode(json_buffer *buffer, const char *text)
{
    const char *end = text;

    while (*text != '\0')
    {
        char escape = 0;

        switch (*text)
        {
            case '\\':
                escape = '\\';
                break;
            case '"':
                escape = '"';
                break;
            case '\b':
                escape = 'b';
                break;
            case '\f':
                escape = 'f';
                break;
            case '\n':
                escape = 'n';
                break;
            case '\r':
                escape = 'r';
                break;
            case '\t':
                escape = 't';
                break;
            default:
                text++;
                break;
        }
        if (escape != 0)
        {
            const char esc[] = {'\\', escape, '\0'};

            BUFFER_WRITE_LENGTH(end, (size_t)(text - end));
            BUFFER_WRITE_LENGTH(esc, 2);
            end = ++text;
        }
    }
    BUFFER_WRITE_LENGTH(end, (size_t)(text - end));
    return 1;
}

static int buffer_write_opening(json_buffer *buffer, const json *node, int depth)
{
    for (int i = 0; i < depth; i++)
    {
        BUFFER_WRITE("  ");
    }
    if (node->name != NULL)
    {
        BUFFER_QUOTE(node->name);
        BUFFER_WRITE(": ");
    }
    switch (node->type)
    {
        case JSON_OBJECT:
            BUFFER_WRITE("{");
            break;
        case JSON_ARRAY:
            BUFFER_WRITE("[");
            break;
        case JSON_STRING:
            BUFFER_QUOTE(node->value);
            break;
        default:
            BUFFER_WRITE(node->value);
            break;
    }
    if (node->left == NULL)
    {
        /* Prints an empty "object" or an empty "array" {} [] */
        switch (node->type)
        {
            case JSON_OBJECT:
                BUFFER_WRITE("}");
                break;
            case JSON_ARRAY:
                BUFFER_WRITE("]");
                break;
            default:
                break;
        }
        if ((depth > 0) && (node->right != NULL))
        {
            BUFFER_WRITE(",");
        }
    }
    BUFFER_WRITE("\n");
    return 1;
}

/* Prints the close group character for each change of level */
static int buffer_write_closing(json_buffer *buffer, const json *node, int depth)
{
    /* if "array" or "object" */
    if (node->left != NULL)
    {
        for (int i = 0; i < depth; i++)
        {
            BUFFER_WRITE("  ");
        }
        switch (node->type)
        {
            case JSON_OBJECT:
                BUFFER_WRITE("}");
                break;
            case JSON_ARRAY:
                BUFFER_WRITE("]");
                break;
            default:
                break;
        }
        if ((depth > 0) && (node->right != NULL))
        {
           BUFFER_WRITE(",");
        }
        BUFFER_WRITE("\n");
    }
    return 1;
}

static int json_set_buffer(const json *node, json_buffer *buffer)
{
    int depth = 0;

    if (node == NULL)
    {
        return 1;
    }
    while (1)
    {
        loop:
        if (!buffer_write_opening(buffer, node, depth))
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
            while (depth > 0)
            {
                node = node->parent;
                if (!buffer_write_closing(buffer, node, --depth))
                {
                    return 0;
                }
                if (node->right != NULL)
                {
                    node = node->right;
                    goto loop;
                }
            }
            return 1;
        }
    }
}

char *json_encode(const json *node)
{
    json_buffer *buffer = buffer_new();

    if (buffer == NULL)
    {
        return NULL;
    }

    char *text = NULL;

    if (json_set_buffer(node, buffer))
    {
        text = buffer->text;
    }
    else
    {
        free(buffer->text);
    }
    free(buffer);
    return text;
}

int json_write(const json *node, FILE *file)
{
    char *str = json_encode(node);

    if (str == NULL)
    {
        return 0;
    }

    int ok = fputs(str, file) != EOF;

    free(str);
    return ok;
}

int json_print(const json *node)
{
    return json_write(node, stdout);
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

