/*!
 *  \brief     JSON Schema validator
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef JSON_SCHEMA_H
#define JSON_SCHEMA_H

#include "json.h"

enum {SCHEMA_WARNING, SCHEMA_ERROR};

typedef void (*schema_callback)(const json *, void *, int, const char *);

int schema_validate(const json *, const json *, schema_callback, void *);
void schema_default_callback(const json *, void *, int, const char *);

#endif /* JSON_SCHEMA_H */

