#ifndef JSON_BUFFER_H
#define JSON_BUFFER_H

#include "json.h"

char *json_encode(const json *);
int json_write(const json *, FILE *);
int json_print(const json *);

#endif /* JSON_BUFFER_H */

