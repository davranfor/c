/*!
 *  \brief     json and json-schema library for C
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef JSON_STRUCT_H
#define JSON_STRUCT_H

#include "json.h"

struct json
{
    json *parent, *child, *next;
    char *name, *value;
    enum json_type type;
};

#endif /* JSON_STRUCT_H */

