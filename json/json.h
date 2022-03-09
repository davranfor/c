/*!
 *  \brief     JSON parser
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef JSON_H
#define JSON_H

typedef struct json json;

typedef struct
{
    int file, line, column;
} json_error;

enum json_type
{
    JSON_UNDEFINED, /* Sin asignar */
    JSON_OBJECT,    /* {Objeto} */
    JSON_ARRAY,     /* [Matriz] */
    JSON_STRING,    /* "Cadena" */
    JSON_NUMBER,    /* Número entero o con decimales */
    JSON_BOOLEAN,   /* true o false */
    JSON_NULL,      /* null */
    JSON_TYPES,     /* Número de tipos */
};

enum json_type json_type(const json *);
const char *json_type_name(const json *);
const char *json_name(const json *);
const char *json_string(const json *);
double json_number(const json *);
long json_integer(const json *);
unsigned long json_real(const json *);
int json_boolean(const json *);
int json_is_object(const json *);
int json_is_array(const json *);
int json_is_property(const json *);
int json_is_string(const json *);
int json_is_number(const json *);
int json_is_integer(const json *);
int json_is_real(const json *);
int json_is_boolean(const json *);
int json_is_null(const json *);
int json_streq(const json *, const char *);
json *json_parse(const char *, json_error *);
json *json_root(const json *);
json *json_parent(const json *);
json *json_next(const json *);
json *json_child(const json *);
json *json_pair(const json *, const char *);
json *json_node(const json *, const char *);
json *json_item(const json *, size_t);
size_t json_items(const json *);
void json_foreach(const json *, void *, void (*)(const json *, void *));
void json_print(const json *);
void json_raise_error(const json_error *error, const char *);
size_t json_ucn_write(char *, unsigned int);
size_t json_string_encode(char *, const char *);
void json_free(json *);

#endif /* JSON_H */

