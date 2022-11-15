/*!
 *  \brief     JSON parser
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef JSON_H
#define JSON_H

#include <stdio.h>

typedef struct json json;

typedef struct { int line, column; } json_error;
typedef int (*json_callback)(const json *, void *);

enum json_type
{
    JSON_UNDEFINED, /* Not assigned */
    JSON_OBJECT,    /* {Object} */
    JSON_ARRAY,     /* [Array] */
    JSON_STRING,    /* "String" */
    JSON_INTEGER,   /* Integer */
    JSON_DOUBLE,    /* Double */
    JSON_BOOLEAN,   /* true or false */
    JSON_NULL,      /* null */
    JSON_TYPES,     /* Number of types */
};

enum json_type json_type(const json *);
const char *json_type_name(const json *);
const char *json_name(const json *);
const char *json_string(const json *);
long json_integer(const json *);
double json_double(const json *);
double json_number(const json *);
unsigned long json_real(const json *);
int json_boolean(const json *);
int json_is_object(const json *);
int json_is_array(const json *);
int json_is_string(const json *);
int json_is_integer(const json *);
int json_is_double(const json *);
int json_is_number(const json *);
int json_is_real(const json *);
int json_is_boolean(const json *);
int json_is_null(const json *);
json *json_parse(const char *, json_error *);
json *json_self(const json *);
json *json_root(const json *);
json *json_parent(const json *);
json *json_next(const json *);
json *json_child(const json *);
json *json_find(const json *, const char *);
json *json_find_next(const json *, const char *);
json *json_node(const json *, const char *);
json *json_item(const json *, size_t);
size_t json_items(const json *);
int json_streq(const json *, const char *);
int json_equal(const json *, const json *);
int json_traverse(const json *, json_callback, void *);
void json_write(FILE *, const json *);
void json_print(const json *);
void json_raise_error(const json_error *error, const char *);
void json_free(json *);

#endif /* JSON_H */

