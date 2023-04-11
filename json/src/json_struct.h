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

