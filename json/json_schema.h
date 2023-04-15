/*!
 *  \brief     JSON Schema validator
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef JSON_SCHEMA_H
#define JSON_SCHEMA_H

#include "json.h"

enum {JSON_SCHEMA_WARNING, JSON_SCHEMA_INVALID, JSON_SCHEMA_ERROR};

typedef int (*json_schema_callback)(const json *, const json*, int, void *);

int json_validate(const json *, const json *, json_schema_callback, void *);

#endif /* JSON_SCHEMA_H */

