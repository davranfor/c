#ifndef JSON_STRUCT_H
#define JSON_STRUCT_H

#include "json.h"

struct json
{
    char *name, *value;
    json *left, *right, *parent;
    enum json_type type;
};

#endif /* JSON_STRUCT_H */

