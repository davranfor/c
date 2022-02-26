/*!
 *  \brief     JSON Schema validator
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef JSON_SCHEMA_H
#define JSON_SCHEMA_H

#include "json.h"

int json_schema_validate(const json *, const json *);

#endif /* JSON_SCHEMA_H */

