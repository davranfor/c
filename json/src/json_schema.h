/*!
 *  \brief     JSON Schema validator
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef JSON_SCHEMA_H
#define JSON_SCHEMA_H

#include "json.h"

int json_validate(const json *, const json *, json_callback, void *);
int json_schema_print(const json *, void *);

#endif /* JSON_SCHEMA_H */

