/*!
 *  \brief     JSON parser
 *  \author    David Ranieri (davranfor)
 *  \copyright GNU Public License.
 */

#ifndef JSON_H
#define JSON_H

typedef struct json json;

enum json_type {
    JSON_EMPTY,     /* Nodo reservado sin usar */
    JSON_OBJECT,    /* {Propiedad} */
    JSON_ARRAY,     /* [Matriz] */
    JSON_STRING,    /* "Cadena" */
    JSON_NUMBER,    /* Número entero, con decimales o con notación científica */
    JSON_BOOLEAN,   /* true o false */
    JSON_NULL,      /* null */
    JSON_NONE,      /* Sin tipificar (puntero nulo) */
};

/*
 * Nota: Si una vez terminado el escaneo algún nodo queda marcado con el tipo
 *       JSON_EMPTY, nos encontramos ante un JSON mal formado.
 *       Los motivos pueden ser:
 *       ',' en un nodo a la izquierda de un nodo padre
 *       '}' en un nodo cuyo padre no es un JSON_OBJECT (sin emparejar con '{')
 *       ']' en un nodo cuyo padre no es un JSON_ARRAY  (sin emparejar con '[')
 */

size_t json_encode(char *, const char *);
json *json_create(const char *);
json *json_create_node(void);
enum json_type json_type(const json *);
char *json_name(const json *);
char *json_string(const json *);
double json_number(const json *);
long json_integer(const json *);
unsigned long json_real(const json *);
int json_boolean(const json *);
int json_is_empty(const json *);
int json_is_object(const json *);
int json_is_array(const json *);
int json_is_string(const json *);
int json_is_number(const json *);
int json_is_integer(const json *);
int json_is_real(const json *);
int json_is_boolean(const json *);
int json_is_null(const json *);
int json_streq(const json *, const char *);
json *json_parent(const json *);
json *json_node(const json *, const char *);
json *json_child(const json *, const char *);
json *json_next(const json *);
json *json_item(const json *, size_t);
size_t json_items(const json *);
void json_print(const json *);
void json_free(json *);

#endif /* JSON_H */

