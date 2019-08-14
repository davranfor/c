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

/*
 * - Nodo
 * - Un puntero al nodo padre (parent) permite construir el árbol de forma no recursiva
 *   y además permite escalar hacia arriba para saber que nodo lo contiene
 */
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

/*
 * Convierte una cadena en una cadena json entrecomillada escapando carácteres especiales
 * Devuelve el tamaño de la cadena final en bytes
 */
size_t json_encode(char *dst, const char *src)
{
#define JSON_CONCAT(c) *(ptr++) = c
#define JSON_ENCODE(c) JSON_CONCAT('\\'); JSON_CONCAT(c)

    char *ptr = dst;

    JSON_CONCAT('"');
    while (*src != '\0')
    {
        switch (*src)
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
                JSON_CONCAT(*src);
                break;
        }
        src++;
    }
    JSON_CONCAT('"');
    *ptr = '\0';
    return (size_t)(ptr - dst);
}

/* Devuelve un puntero al próximo elemento (entidad) o NULL si es una cadena mal formada */
static const char *json_scan(const char **text)
{
    const char *str = *text;

    *text = NULL;

    /* Centinela para saber si estamos dentro de un string */
    int quoted = 0;

    /* Mientras haya texto que parsear */
    while (*str != '\0')
    {
        /* Si es un código de escape */
        if (*str == '\\')
        {
            /*
             * Si aparece fuera de un string es una cadena mal formada
             * Si no es un código de escape válido es una cadena mal formada
             */
            if ((quoted == 0) || !json_isescape(*(++str)))
            {
                return NULL;
            }
        } else
        /* Si es una comilla cambiamos el estado */
        if (*str == '"')
        {
            quoted ^= 1;
        }
        /* Si estamos fuera de un string y el caracter es una entidad */
        if ((quoted == 0) && json_istoken(*str))
        {
            break;
        }
        /* Si es el primer caracter que no es un espacio */
        if ((*text == NULL) && !isspace(*str))
        {
            *text = str;
        }
        str++;
    }
    /* Si no se han cerrado comillas */
    if (quoted != 0)
    {
        return NULL;
    }
    /* Si no hay contenido delante del token */
    if (*text == NULL)
    {
        *text = str;
    }
    return str;
}

/* Devuelve un nuevo nombre o valor alojado en memoria fresca escapando caracteres especiales */
static char *json_copy(const char *str, size_t size)
{
    char *new = malloc(size + 1);

    if (new == NULL)
    {
        return NULL;
    }

    size_t n = 0;

    for (size_t i = 0; i < size; i++)
    {
        /* Si el carácter es un escape '\' */
        if (str[i] == '\\')
        {
            switch (str[++i])
            {
                /* Si es un código de escape válido lo transforma */
                case '\\':
                case '/':
                case '"':
                    new[n++] = str[i];
                    break;
                case 'b':
                    new[n++] = '\b';
                    break;
                case 'f':
                    new[n++] = '\f';
                    break;
                case 'n':
                    new[n++] = '\n';
                    break;
                case 'r':
                    new[n++] = '\r';
                    break;
                case 't':
                    new[n++] = '\t';
                    break;
                /* Si no, lo salta */
                default:
                    break;
            }
        }
        /* Si es una comilla '"', la salta */
        else if (str[i] != '"')
        {
            new[n++] = str[i];
        }
    }
    new[n] = '\0';
    return new;
}

/* Elimina espacios y entidades y devuelve el tamaño del string */
static size_t json_trim(const char **left, const char **right)
{
    /* A la izquierda */
    while (*left < *right)
    {
        if (!isspace(**left))
        {
            break;
        }
        (*left)++;
    }
    /* A la derecha */
    while (*right > *left)
    {
        if (!(isspace(**right) || json_istoken(**right)))
        {
            break;
        }
        (*right)--;
    }
    return (size_t)(*right - *left + 1);
}

static char *json_set_name(json *node, const char *left, const char *right)
{
    size_t size = json_trim(&left, &right);

    /* Si no empieza y acaba con comillas */
    if ((*left != '"') || (*right != '"'))
    {
        return NULL;
    }
    /* Asigna el nombre */
    node->name = json_copy(left, size);
    return node->name;
}

static char *json_set_value(json *node, const char *left, const char *right)
{
    size_t size = json_trim(&left, &right);

    /* Asigna el tipo */
    if ((*left == '"') && (*right == '"'))
    {
        node->type = JSON_STRING;
    }
    else if ((size == 4) && (strncmp(left, "null", size) == 0))
    {
        node->type = JSON_NULL;
    }
    else if ((size == 4) && (strncmp(left, "true", size) == 0))
    {
        node->type = JSON_BOOLEAN;
    }
    else if ((size == 5) && (strncmp(left, "false", size) == 0))
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
    node->value = json_copy(left, size);
    return node->value;
}

/* Recorre el texto y rellena los nodos */
static json *json_parse(json *node, const char *text)
{
    const char *token;

    while (node != NULL)
    {
        token = json_scan(&text);
        if (token == NULL)
        {
            return NULL;
        }
        switch (*token)
        {
            /* Crea un nodo a la izquierda (hijo) */
            case '{':
            case '[':
                /* No puede haber nada entre los dos puntos (:) o la coma (,) y el token */
                if (text != token)
                {
                    return NULL;
                }
                /* Los elementos de un objeto deben tener nombre */
                if ((node->parent != NULL) && (node->parent->type == JSON_OBJECT) && (node->name == NULL))
                {
                    return NULL;
                }
                node->type = json_token(*token);
                node->left = json_create_node();
                if (node->left == NULL)
                {
                    return NULL;
                }
                node->left->parent = node;
                node = node->left;
                break;
            /* Nombre del nodo */
            case ':':
                /* Solo los elementos de un objeto pueden tener nombre */
                if ((node->parent == NULL) || (node->parent->type != JSON_OBJECT) || (node->name != NULL))
                {
                    return NULL;
                }
                if (json_set_name(node, text, token) == NULL)
                {
                    return NULL;
                }
                break;
            /* Crea un nodo a la derecha (hermano) */
            case ',':
                /* Los elementos de un objeto deben tener nombre */
                if ((node->parent == NULL) || ((node->parent->type == JSON_OBJECT) && (node->name == NULL)))
                {
                    return NULL;
                }
                if (node->type == JSON_EMPTY)
                {
                    if (text == token)
                    {
                        return NULL;
                    }
                    if (json_set_value(node, text, token) == NULL)
                    {
                        return NULL;
                    }
                }
                else if (text != token)
                {
                    return NULL;
                }
                node->right = json_create_node();
                if (node->right == NULL)
                {
                    return NULL;
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
                    return NULL;
                }
                if (node->type == JSON_EMPTY)
                {
                    if (text == token)
                    {
                        /* Puede ser un grupo vacío: {} o [] */
                        if (node->parent->left != node)
                        {
                            return NULL;
                        }
                    }
                    else
                    {
                        /* Los elementos de un objeto deben tener nombre */
                        if ((node->parent->type == JSON_OBJECT) && (node->name == NULL))
                        {
                            return NULL;
                        }
                        if (json_set_value(node, text, token) == NULL)
                        {
                            return NULL;
                        }
                    }
                }
                else if (text != token)
                {
                    return NULL;
                }
                node = node->parent;
                break;
            /* Si hemos llegado al final */
            default:
                if (text != token)
                {
                    /* Puede consistir en un solo elemento, p.ej: "Texto" ó 123 */ 
                    if (node->type != JSON_EMPTY)
                    {
                        return NULL;
                    }
                    if (json_set_value(node, text, text + strlen(text) - 1) == NULL)
                    {
                        return NULL;
                    }
                }
                /* No puede estar vacío */
                if (node->type == JSON_EMPTY)
                {
                    return NULL;
                }
                /* Si no está bien cerrado */
                if (node->parent != NULL)
                {
                    return NULL;
                }
                /* El documento es correcto */
                return node;
        }
        /* Seguimos avanzando */
        text = token + 1;
    }
    return NULL;
}

/* Crea el nodo root y lo pasa al parseador junto con un puntero al texto */
json *json_create(const char *text)
{
    json *node = json_create_node();

    if (node != NULL)
    {
        if (json_parse(node, text) == NULL)
        {
            json_free(node);
            return NULL;
        }
    }
    return node;
}

/* Devuelve un nodo nuevo */
json *json_create_node(void)
{
    return calloc(1, sizeof(struct json));
}

/* Devuelve el tipo de un nodo */
enum json_type json_type(const json *node)
{
    if (node != NULL)
    {
        return node->type;
    }
    else
    {
        return JSON_NONE;
    }
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
    if ((node == NULL) || (node->value == NULL))
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
    if ((node == NULL) || (node->value == NULL))
    {
        return 0;
    }
    if (node->type != JSON_STRING)
    {
        return 0;
    }
    return !strcmp(node->value, str);
}

/* Devuelve el nodo padre */
json *json_parent(const json *node)
{
    if (node != NULL)
    {
        return node->parent;
    }
    return NULL;
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

/* Localiza un nodo por clave recorriendo los nodos derechos del nodo pasado y devuelve el hijo*/
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
    if (node != NULL)
    {
        return node->right;
    }
    return NULL;
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
