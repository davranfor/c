/*!
 *  \brief     JSON
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef JSON_H
#define JSON_H

typedef struct json json;
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

#endif /* JSON_H */

