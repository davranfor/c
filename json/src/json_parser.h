#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include "json.h"

typedef struct { int line, column; } json_error;

json *json_new(json *, const char *);
json *json_parse(const char *, json_error *);
json *json_parse_file(const char *, json_error *);
void json_raise_error(const char *, const json_error *);
void json_free(json *);

#endif /* JSON_PARSER_H */

