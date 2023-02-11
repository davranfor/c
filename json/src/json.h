/*!
 *  \brief     JSON
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef JSON_H
#define JSON_H

typedef struct json json;
typedef int (*json_callback)(const json *, int, void *);

enum json_type
{
    JSON_UNDEFINED,
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_STRING,
    JSON_INTEGER,
    JSON_DOUBLE,
    JSON_BOOLEAN,
    JSON_NULL,
    JSON_TYPES,
};

#endif /* JSON_H */

