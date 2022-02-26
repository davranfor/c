#ifndef JSON_UTILS_H
#define JSON_UTILS_H

char *json_read_file(const char *);
json *json_parse_file(const char *, json_error *);

#endif /* JSON_UTILS_H */

