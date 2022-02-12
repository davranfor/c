/*! 
 *  \brief     JSON parser
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
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

static const char *json_type_name[] =
{
    "Empty",
    "Object",
    "Array",
    "String",
    "Number",
    "Boolean",
    "Null"
};

#define json_isspace(c) isspace((unsigned char)c)

/* Devuelve true or false dependiendo de si el carácter a examinar es una entidad de JSON */
static int json_istoken(int c)
{
    return (c == '{') || (c == '[') || (c == ':') || (c == ',') || (c == ']') || (c == '}');
}

/* Devuelve true or false dependiendo de si el carácter a examinar es un codigo de escape */
static int json_isescape(int c)
{
    return (c == '\\') || (c == '/') || (c == '"') ||
           (c == 'b')  || (c == 'f') || (c == 'n') || (c == 'r') || (c == 't');
}

/* Devuelve true or false dependiendo de si el carácter a examinar es "\" o de control */
static int json_iscontrol(int c)
{
    return (c == '\\') || iscntrl((unsigned char)c);
}

/* Devuelve true or false dependiendo de si el carácter a examinar es un UCN "\uxxxx" */
static int json_isunicode(const char *str)
{
    return (*str++ == 'u')
        && isxdigit((unsigned char)*str++)
        && isxdigit((unsigned char)*str++)
        && isxdigit((unsigned char)*str++)
        && isxdigit((unsigned char)*str);
}

/* Convierte entidad en tipo */
static enum json_type json_token(int token)
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
            return JSON_EMPTY;
    }
}

/* Rellena cadena a partir de UCN, devuelve el número de bytes */
static size_t json_unicode(const char *str, char *buf)
{
    unsigned code;

    if (sscanf(str, "u%04x", &code) == 1)
    {
        int len = wctomb(buf, (wchar_t)code);

        if (len != -1)
        {
            return (size_t)len;
        }
    }
    return 0;
}

/* Devuelve un puntero al próximo elemento (entidad) o NULL si es una cadena mal formada */
static const char *json_scan(const char **left, const char **right)
{
    const char *str = *left;

    *left = *right = NULL;

    int quotes = 0;

    while (*str != '\0')
    {
        /* Si es un código de escape */
        if (*str == '\\')
        {
            /* Si estamos dentro de una cadena */
            if (quotes == 1)
            {
                str++;
                /* Si es un código de escape válido "\?" */
                if (json_isescape(*str))
                {
                    str += 1;
                    continue;
                }
                /* Si es un UCN (Universal Character Name) "\uxxxx" */
                if (json_isunicode(str))
                {
                    str += 5;
                    continue;
                }
            }
            return NULL;
        }
        /* Si es una comilla */
        else if (*str == '"')
        {
            /* Si hay más de una cadena, p.ej <"abc" "def"> */
            if (quotes > 1)
            {
                return NULL;
            }
            /* Cambiamos el estado */
            quotes++;
        }
        /* Si estamos fuera de una cadena */
        else if (quotes != 1)
        {
            /* Si es una entidad JSON */
            if (json_istoken(*str))
            {
                break;
            }
        }
        /* Si estamos dentro de una cadena y es un carácter de control */
        else if (json_iscontrol(*str))
        {
            return NULL;
        }
        /* Si no es un espacio */
        if (!json_isspace(*str))
        {
            /* Si es el principio del token */
            if (*left == NULL)
            {
                *left = str;
            }
            /* El final del token siempre está actualizado */
            *right = str;
        }
        str++;
    }
    /* Si no se han cerrado comillas */
    if (quotes == 1)
    {
        return NULL;
    }
    /* Si no hay contenido delante del token */
    if (*left == NULL)
    {
        *left = *right = str;
    }
    return str;
}

/* Devuelve un nuevo nombre o valor alojado en memoria fresca escapando caracteres especiales */
static char *json_copy(const char *str, size_t len)
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
        /* Si el carácter es un escape '\' */
        if (*str == '\\')
        {
            switch (*++str)
            {
                /* Si es un código de escape válido lo transforma */
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
                    ptr += json_unicode(str, ptr);
                    str += 4;
                    break;
                /* No debería llegar aquí */
                default:
                    break;
            }
        }
        /* Si es una comilla '"', la salta */
        else if (*str != '"')
        {
            *ptr++ = *str;
        }
        str++;
    }
    *ptr = '\0';
    return buf;
}

static char *json_set_name(json *node, const char *left, const char *right)
{
    size_t len = (size_t)(right - left + 1);

    /* Si no empieza y acaba con comillas */
    if ((*left != '"') || (*right != '"'))
    {
        return NULL;
    }
    /* Asigna el nombre */
    node->name = json_copy(left, len);
    return node->name;
}

static char *json_set_value(json *node, const char *left, const char *right)
{
    size_t len = (size_t)(right - left + 1);

    /* Asigna el tipo */
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
    /* Asigna el valor */
    node->value = json_copy(left, len);
    return node->value;
}

/* Devuelve un nodo nuevo */
static json *json_create(void)
{
    return calloc(1, sizeof(struct json));
}

/* Recorre el texto y rellena los nodos */
static json *json_build(json *node, const char *left, const char **error)
{
    #define JSON_ERROR (*error = left, NULL)

    const char *right;
    const char *token;

    while (node != NULL)
    {
        token = json_scan(&left, &right);
        if (token == NULL)
        {
            return JSON_ERROR;
        }
        switch (*token)
        {
            /* Crea un nodo a la izquierda (hijo) */
            case '{':
            case '[':
                /* No puede haber nada entre los dos puntos (:) o la coma (,) y el token */
                if (left != token)
                {
                    return JSON_ERROR;
                }
                /* Los elementos de un objeto deben tener nombre */
                if ((node->parent != NULL) && (node->parent->type == JSON_OBJECT) && (node->name == NULL))
                {
                    return JSON_ERROR;
                }
                node->type = json_token(*token);
                node->left = json_create();
                if (node->left == NULL)
                {
                    return JSON_ERROR;
                }
                node->left->parent = node;
                node = node->left;
                break;
            /* Nombre del nodo */
            case ':':
                /* Solo los elementos de un objeto pueden tener nombre */
                if ((node->parent == NULL) || (node->parent->type != JSON_OBJECT) || (node->name != NULL))
                {
                    return JSON_ERROR;
                }
                if (json_set_name(node, left, right) == NULL)
                {
                    return JSON_ERROR;
                }
                break;
            /* Crea un nodo a la derecha (hermano) */
            case ',':
                /* Los elementos de un objeto deben tener nombre */
                if ((node->parent == NULL) || ((node->parent->type == JSON_OBJECT) && (node->name == NULL)))
                {
                    return JSON_ERROR;
                }
                if (node->type == JSON_EMPTY)
                {
                    if (left == token)
                    {
                        return JSON_ERROR;
                    }
                    if (json_set_value(node, left, right) == NULL)
                    {
                        return JSON_ERROR;
                    }
                }
                else if (left != token)
                {
                    return JSON_ERROR;
                }
                node->right = json_create();
                if (node->right == NULL)
                {
                    return JSON_ERROR;
                }
                node->right->parent = node->parent;
                node = node->right;
                break;
            /*
             * Si el nodo está sin utilizar es un valor, si no es el fin de una matriz
             * y toca emparejarlo con el padre
             */
            case ']':
            case '}':
                /* Por cada cierre debe haber una apertura del mismo tipo */
                if ((node->parent == NULL) || (node->parent->type != json_token(*token)))
                {
                    return JSON_ERROR;
                }
                if (node->type == JSON_EMPTY)
                {
                    if (left == token)
                    {
                        /* Puede ser un grupo vacío: {} o [] */
                        if (node->parent->left != node)
                        {
                            return JSON_ERROR;
                        }
                    }
                    else
                    {
                        /* Los elementos de un objeto deben tener nombre */
                        if ((node->parent->type == JSON_OBJECT) && (node->name == NULL))
                        {
                            return JSON_ERROR;
                        }
                        if (json_set_value(node, left, right) == NULL)
                        {
                            return JSON_ERROR;
                        }
                    }
                }
                else if (left != token)
                {
                    return JSON_ERROR;
                }
                node = node->parent;
                break;
            /* Si hemos llegado al final */
            default:
                if (left != token)
                {
                    /* Puede consistir en un solo elemento, p.ej: "Texto" ó 123 */ 
                    if (node->type != JSON_EMPTY)
                    {
                        return JSON_ERROR;
                    }
                    if (json_set_value(node, left, right) == NULL)
                    {
                        return JSON_ERROR;
                    }
                }
                /* No puede estar vacío */
                if (node->type == JSON_EMPTY)
                {
                    return JSON_ERROR;
                }
                /* Si no está bien cerrado */
                if (node->parent != NULL)
                {
                    return JSON_ERROR;
                }
                /* El documento es correcto */
                return node;
        }
        /* Seguimos avanzando */
        left = token + 1;
    }
    return JSON_ERROR;
}

static void json_set_error(const char *str, const char *end, json_error *error)
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
        // If ASCII character or first byte of a multi-byte character
        if ((*str & 0xc0) != 0x80)
        {
            error->column++;
        }
        str++;
    }
}

/* Devuelve el tipo de un nodo */
enum json_type json_type(const json *node)
{
    if (node == NULL)
    {
        return JSON_NONE;
    }
    return node->type;
}

/* Devuelve el label de un nodo */
char *json_name(const json *node)
{
    if (node == NULL)
    {
        return NULL;
    }
    return node->name;
}

/* Conversiones a diferentes tipos */

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

int json_is_empty(const json *node)
{
    if (node == NULL)
    {
        return 0;
    }
    return node->type == JSON_EMPTY;
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

/* Devuelve 0 o 1 dependiendo de si la cadena pasada coincide con el valor del nodo */
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

/*
 * Crea el nodo root y lo pasa al parseador junto con un puntero al texto
 * Asigna la línea y columna de error si se ha pasado un puntero a error
 */
json *json_parse(const char *text, json_error *error)
{
    if (error)
    {
        error->line = 0;
        error->column = 0;
    }

    json *node = json_create();

    if (node != NULL)
    {
        const char *end = text;

        if (json_build(node, text, &end) == NULL)
        {
            if (error)
            {
                json_set_error(text, end, error);
            }
            json_free(node);
            return NULL;
        }
    }
    return node;
}

/* Devuelve el nodo padre */
json *json_parent(const json *node)
{
    if (node == NULL)
    {
        return NULL;
    }
    return node->parent;
}

/* Localiza un nodo por clave recorriendo los nodos derechos del nodo pasado */
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

/* Localiza un nodo por clave recorriendo los nodos derechos del nodo pasado y devuelve el hijo */
json *json_child(const json *node, const char *name)
{
    if ((node != NULL) && (node = node->left))
    {
        while (node != NULL)
        {
            if ((node->name != NULL) && (strcmp(node->name, name) == 0))
            {
                return node->left;
            }
            node = node->right;
        }
    }
    return NULL;
}

/* Devuelve el nodo hermano */
json *json_next(const json *node)
{
    if (node == NULL)
    {
        return NULL;
    }
    return node->right;
}

/* Localiza un nodo por offset recorriendo los nodos derechos del nodo pasado */
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

/* Cuenta el número de nodos derechos del nodo pasado */
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

/* Manda cada nodo al callback "func" */
void json_foreach(const json *node, void *data, void (*func)(const json *, void *))
{
    if (node != NULL)
    {
        func(node, data);
        json_foreach(node->left, data, func);
        json_foreach(node->right, data, func);
    }
}

/*
 * Imprime el árbol de forma recursiva
 * Los nodos izquierdos son los que marcan el cambio de nivel
 */ 
static void print(const json *node, int level)
{
    if (node != NULL)
    {
        /* Imprime una tabulación por cada cambio de nivel */
        for (int i = 0; i < level; i++)
        {
            putchar('\t');
        }
        /* Imprime el nodo */
        printf("(%s) <%s> = <%s>\n", json_type_name[node->type], node->name, node->value);
        /* Por cada nodo izquierdo visita los nodos derechos */
        print(node->left, level + 1);
        print(node->right, level);
    }
}

void json_print(const json *node)
{
    print(node, 0);
}

/* Libera toda la memoria reservada para el árbol */
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

/* Libera toda la memoria reservada para el árbol de forma recursiva
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

/*
 * Convierte una cadena en una cadena json entrecomillada escapando carácteres especiales
 * Devuelve el tamaño de la cadena final en bytes
 */
size_t json_encode(char *buf, const char *str)
{
    #define JSON_CONCAT(c) *(ptr++) = c
    #define JSON_ENCODE(c) JSON_CONCAT('\\'); JSON_CONCAT(c)

    char *ptr = buf;

    JSON_CONCAT('"');
    while (*str != '\0')
    {
        switch (*str)
        {
            case '\\':
                JSON_ENCODE('\\');
                break;
            case '/':
                JSON_ENCODE('/');
                break;
            case '"':
                JSON_ENCODE('"');
                break;
            case '\b':
                JSON_ENCODE('b');
                break;
            case '\f':
                JSON_ENCODE('f');
                break;
            case '\n':
                JSON_ENCODE('n');
                break;
            case '\r':
                JSON_ENCODE('r');
                break;
            case '\t':
                JSON_ENCODE('t');
                break;
            default:
                JSON_CONCAT(*str);
                break;
        }
        str++;
    }
    JSON_CONCAT('"');
    *ptr = '\0';
    return (size_t)(ptr - buf);
}

