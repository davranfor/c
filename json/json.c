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
    enum json_type type;
    char *name;
    char *value;
    json *left;
    json *right;
    json *parent;
};

static const char *type_name[] =
{
    "Undefined",
    "Object",
    "Array",
    "String",
    "Number",
    "Boolean",
    "Null"
};

#define is_space(c) isspace((unsigned char)c)

/* Check wether a character is an escape character */
static int is_escape(int c)
{
    return (c == '\\') || (c == '/') || (c == '"') ||
           (c == 'b')  || (c == 'f') || (c == 'n') || (c == 'r') || (c == 't');
}

/* Check wether a character is a control character */
static int is_control(int c)
{
    return (c == '\\') || iscntrl((unsigned char)c);
}

/* Returns the codepoint of an UCN (Universal character name) "\uxxxx" */
static unsigned ucn_code(const char *str, int *error)
{
    if (error != NULL)
    {
        *error = 0;
    }

    int code = 0;

    for (int len = 0; len < 4; len++)
    {
        int c = *(++str);

        switch (*str)
        {
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                code = code * 16 + c - '0';
                break;
            case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
                code = code * 16 + 10 + c - 'A';
                break;
            case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
                code = code * 16 + 10 + c - 'a';
                break;
            default:
                if (error != NULL)
                {
                    *error = 1;
                }
                return 0;
        }
    }
    return (unsigned)code;
}

/* Check wether a character is an UCN */
static int is_ucn(const char *str)
{
    if (*str == 'u')
    {
        int error;

        ucn_code(str, &error);
        return !error;
    }
    return 0;
}

/*
 * Converts UCN to multibyte
 * Returns the length of the multibyte in bytes
 */
static size_t ucn_to_mb(const char *str, char *buf)
{
    unsigned code = ucn_code(str, NULL);

    if (code <= 0x7f)
    {
        buf[0] = (char)code;
        return 1;
    }
    else if (code <= 0x7ff)
    {
        buf[0] = (char)(0xc0 | ((code >> 6) & 0x1f));
        buf[1] = (char)(0x80 | ((code & 0x3f)));
        return 2;
    }
    else if (code <= 0xffff)
    {
        buf[0] = (char)(0xe0 | ((code >> 12) & 0x0f));
        buf[1] = (char)(0x80 | ((code >> 6) & 0x3f));
        buf[2] = (char)(0x80 | ((code & 0x3f)));
        return 3;
    }
    else
    {
        buf[0] = (char)(0xf0 | ((code >> 18) & 0x07));
        buf[1] = (char)(0x80 | ((code >> 12) & 0x3f));
        buf[2] = (char)(0x80 | ((code >> 6) & 0x3f));
        buf[3] = (char)(0x80 | ((code & 0x3f)));
        return 4;
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

/* Returns a pointer to the next element or NULL on fail */
static const char *scan(const char **left, const char **right)
{
    const char *str = *left;

    *left = *right = NULL;

    int quotes = 0;

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
        else if (*str == '"')
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
        /* Control characters are not allowed outside a string */
        else if (is_control(*str))
        {
            return NULL;
        }
        if (!is_space(*str))
        {
            /* Update left pointer  */
            if (*left == NULL)
            {
                *left = str;
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
    if (*left == NULL)
    {
        *left = *right = str;
    }
    return str;
}

/* Allocates space for a 'name' or a 'value' escaping special characters */
static char *copy(const char *str, size_t len)
{
    char *buf = malloc(len + 1);

    if (buf == NULL)
    {
        return NULL;
    }

    const char *end = str + len;
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
    size_t len = (size_t)(right - left + 1);

    /* Must start and end with quotes */
    if ((*left != '"') || (*right != '"'))
    {
        return NULL;
    }
    node->name = copy(left, len);
    return node->name;
}

static char *set_value(json *node, const char *left, const char *right)
{
    size_t len = (size_t)(right - left + 1);

    if ((*left == '"') && (*right == '"'))
    {
        node->type = JSON_STRING;
    }
    else if ((len == 4) && (strncmp(left, "null", len) == 0))
    {
        node->type = JSON_NULL;
    }
    else if ((len == 4) && (strncmp(left, "true", len) == 0))
    {
        node->type = JSON_BOOLEAN;
    }
    else if ((len == 5) && (strncmp(left, "false", len) == 0))
    {
        node->type = JSON_BOOLEAN;
    }
    else
    {
        char *end;

        strtod(left, &end);
        if (end <= right)
        {
            return NULL;
        }
        node->type = JSON_NUMBER;
    }
    node->value = copy(left, len);
    return node->value;
}

static json *create_node(void)
{
    return calloc(1, sizeof(struct json));
}

static json *parse(json *node, const char *left, const char **error)
{
    #define ERROR (*error = left, NULL)

    const char *right;
    const char *token;

    while (node != NULL)
    {
        token = scan(&left, &right);
        if (token == NULL)
        {
            return ERROR;
        }
        switch (*token)
        {
            case '{':
            case '[':
                /*
                 * if there is text before the token
                 * e.g.: "abc"{ or 123[
                 */
                if (left != token)
                {
                    return ERROR;
                }
                /* Object properties must have a name */
                if ((node->parent != NULL) &&
                    (node->parent->type == JSON_OBJECT) &&
                    (node->name == NULL))
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
                if (node->parent != NULL)
                {
                    return ERROR;
                }
                /* Correct document */
                return node;
        }
        /* Keep going ... */
        left = token + 1;
    }
    return ERROR;
}

static void set_error(const char *str, const char *end, json_error *error)
{
    error->line = 1;
    error->column = 1;
    while ((str < end) && (*str != '\0'))
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

char *json_name(const json *node)
{
    if (node == NULL)
    {
        return NULL;
    }
    return node->name;
}

char *json_string(const json *node)
{
    if (node == NULL)
    {
        return NULL;
    }
    return node->value;
}

double json_number(const json *node)
{
    if ((node == NULL) || (node->value == NULL))
    {
        return 0.0;
    }
    return strtod(node->value, NULL);
}

long json_integer(const json *node)
{
    if ((node == NULL) || (node->value == NULL))
    {
        return 0;
    }
    return strtol(node->value, NULL, 10);
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
        return *node->value == 't';
    }
    if (node->type == JSON_NUMBER)
    {
        return json_number(node) != 0.0;
    }
    return 0;
}

int json_is_object(const json *node)
{
    if (node == NULL)
    {
        return 0;
    }
    return node->type == JSON_OBJECT;
}

int json_is_array(const json *node)
{
    if (node == NULL)
    {
        return 0;
    }
    return node->type == JSON_ARRAY;
}

int json_is_property(const json *node)
{
    if (node == NULL)
    {
        return 0;
    }
    return node->name != NULL;
}

int json_is_string(const json *node)
{
    if (node == NULL)
    {
        return 0;
    }
    return node->type == JSON_STRING;
}

int json_is_number(const json *node)
{
    if (node == NULL)
    {
        return 0;
    }
    return node->type == JSON_NUMBER;
}

int json_is_integer(const json *node)
{
    if (node == NULL)
    {
        return 0;
    }
    if (node->type != JSON_NUMBER)
    {
        return 0;
    }
    if (strchr(node->value, '.') != NULL)
    {
        return 0;
    }
    return 1;
}

int json_is_real(const json *node)
{
    if (node == NULL)
    {
        return 0;
    }
    if (node->type != JSON_NUMBER)
    {
        return 0;
    }
    if (strchr(node->value, '.') != NULL)
    {
        return 0;
    }
    if (strchr(node->value, '-') != NULL)
    {
        return 0;
    }
    return 1;
}

int json_is_boolean(const json *node)
{
    if (node == NULL)
    {
        return 0;
    }
    return node->type == JSON_BOOLEAN;
}

int json_is_null(const json *node)
{
    if (node == NULL)
    {
        return 0;
    }
    return node->type == JSON_NULL;
}

int json_streq(const json *node, const char *str)
{
    if (node == NULL)
    {
        return 0;
    }
    if (node->type != JSON_STRING)
    {
        return 0;
    }
    return !strcmp(node->value, str);
}

json *json_parse(const char *text, json_error *error)
{
    if (error)
    {
        error->file = 0;
        error->line = 0;
        error->column = 0;
    }

    json *node = create_node();

    if (node != NULL)
    {
        const char *end = text;

        if (parse(node, text, &end) == NULL)
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

json *json_parent(const json *node)
{
    if (node == NULL)
    {
        return NULL;
    }
    return node->parent;
}

json *json_next(const json *node)
{
    if (node == NULL)
    {
        return NULL;
    }
    return node->right;
}

json *json_child(const json *node)
{
    if (node == NULL)
    {
        return NULL;
    }
    return node->left;
}

/* Locates a child node by name */
json *json_node(const json *root, const char *name)
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

/* Sends all nodes to a callback */
void json_foreach(const json *node, void *data,
    void (*func)(const json *, void *))
{
    if (node != NULL)
    {
        func(node, data);
        json_foreach(node->left, data, func);
        json_foreach(node->right, data, func);
    }
}

/* Sends all childs to a callback */
void json_foreach_child(const json *node, void *data,
    void (*func)(const json *, void *))
{
    if ((node != NULL) && (node = node->left))
    {
        while (node != NULL)
        {
            func(node, data);
            node = node->right;
        }
    }
}

static void print_node_begin(const json *node, int level)
{
    while (level--)
    {
        printf("  ");
    }
    if (node->name != NULL)
    {
        printf("\"%s\": ", node->name);
    }
    switch (node->type)
    {
        case JSON_OBJECT:
            printf("{");
            break;
        case JSON_ARRAY:
            printf("[");
            break;
        case JSON_STRING:
            printf("\"%s\"", node->value);
            break;
        default:
            printf("%s", node->value);
            break;
    }
    if (node->left == NULL)
    {
        /* Prints an empty "object" or an empty "array" {} [] */
        switch (node->type)
        {
            case JSON_OBJECT:
                printf("}");
                break;
            case JSON_ARRAY:
                printf("]");
                break;
            default:
                break;
        }
        if (node->right != NULL)
        {
            printf(",");
        }
    }
    printf("\n");
}

/* Prints the close group character for each change of level */
static void print_node_end(const json *node, int level)
{
    /* if "array" or "object" */
    if (node->left != NULL)
    {
        while (level--)
        {
            printf("  ");
        }
        switch (node->type)
        {
            case JSON_OBJECT:
                printf("}");
                break;
            case JSON_ARRAY:
                printf("]");
                break;
            default:
                break;
        }
        if (node->right != NULL)
        {
            printf(",");
        }
        printf("\n");
    }
}

/*
 * Prints a tree recursively
 * "level" is incremented when a left branch is taken
 */
static void print(const json *node, int level)
{
    if (node != NULL)
    {
        print_node_begin(node, level);
        print(node->left, level + 1);
        print_node_end(node, level);
        print(node->right, level);
    }
}

void json_print(const json *node)
{
    print(node, 0);
}

void json_raise_error(const json_error *error, const char *path)
{
    if (error)
    {
        if (error->file)
        {
            fprintf(stderr, "json:\t%s\n\t%s\n",
                    path ? path : "", strerror(errno));
        }
        else
        {
            fprintf(stderr, "json:\t%s\n\tError at line %d, column %d\n",
                    path ? path : "", error->line, error->column);
        }
    }
    else
    {
        fprintf(stderr, "json: %s\n", path ? path : "Unhandled error");
    }
}

/*
 * Encodes a multibyte as UCN "\uxxxx"
 * Returns the length of the string in bytes (6)
 */
size_t json_ucn_encode(char *buf, const char *str)
{
    unsigned int code = 0;

    while (*str != 0)
    {
        unsigned char c = (unsigned char)*str;

        if (c <= 0x7f)
        {
            code = c;
        }
        else if (c <= 0xbf)
        {
            code = (code << 6) | (c & 0x3f);
        }
        else if (c <= 0xdf)
        {
            code = c & 0x1f;
        }
        else if (c <= 0xef)
        {
            code = c & 0x0f;
        }
        else
        {
            code = c & 0x07;
        }
        str++;
    }
    return (size_t)sprintf(buf, "\\u%04x", code);
}

/*
 * Quotes a string escaping special characters 
 * Returns the length of the string in bytes
 */
size_t json_string_encode(char *buf, const char *str)
{
    #define CONCAT(c) *(ptr++) = c
    #define ENCODE(c) CONCAT('\\'); CONCAT(c)

    char *ptr = buf;

    CONCAT('"');
    while (*str != '\0')
    {
        switch (*str)
        {
            case '\\':
                ENCODE('\\');
                break;
            case '/':
                ENCODE('/');
                break;
            case '"':
                ENCODE('"');
                break;
            case '\b':
                ENCODE('b');
                break;
            case '\f':
                ENCODE('f');
                break;
            case '\n':
                ENCODE('n');
                break;
            case '\r':
                ENCODE('r');
                break;
            case '\t':
                ENCODE('t');
                break;
            default:
                CONCAT(*str);
                break;
        }
        str++;
    }
    CONCAT('"');
    *ptr = '\0';
    return (size_t)(ptr - buf);
}

void json_free(json *node)
{
    json *next;

    while (node != NULL)
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

/* Free recursive version
void json_free(json *node)
{
    if (node != NULL)
    {
        json_free(node->left);
        json_free(node->right);
        free(node->name);
        free(node->value);
        free(node);
    }
}
*/

