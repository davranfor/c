/*!
 *  \brief     JSON parser
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef JSON_FILTER_H
#define JSON_FILTER_H

#include "json.h"

/**
 * Do not change the order nor delete any element of the enum
 * without checking then json_is() implementation
 */
enum json_query
{
    objectOfItems,
    objectOfObjects,
    objectOfArrays,
    objectOfStrings,
    objectOfIntegers,
    objectOfDoubles,
    objectOfNumbers,
    objectOfReals,
    objectOfBooleans,
    objectOfNulls,

    arrayOfItems,
    arrayOfObjects,
    arrayOfArrays,
    arrayOfStrings,
    arrayOfIntegers,
    arrayOfDoubles,
    arrayOfNumbers,
    arrayOfReals,
    arrayOfBooleans,
    arrayOfNulls,

    objectOfOptionalItems,
    objectOfOptionalObjects,
    objectOfOptionalArrays,
    objectOfOptionalStrings,
    objectOfOptionalIntegers,
    objectOfOptionalDoubles,
    objectOfOptionalNumbers,
    objectOfOptionalReals,
    objectOfOptionalBooleans,
    objectOfOptionalNulls,

    arrayOfOptionalItems,
    arrayOfOptionalObjects,
    arraytOfOptionalArrays,
    arrayOfOptionalStrings,
    arrayOfOptionalIntegers,
    arrayOfOptionalDoubles,
    arrayOfOptionalNumbers,
    arrayOfOptionalReals,
    arrayOfOptionalBooleans,
    arrayOfOptionalNulls,

    objectOfUniqueItems,
    objectOfUniqueObjects,
    objectOfUniqueArrays,
    objectOfUniqueStrings,
    objectOfUniqueIntegers,
    objectOfUniqueDoubles,
    objectOfUniqueNumbers,
    objectOfUniqueReals,
    objectOfUniqueBooleans,
    objectOfUniqueNulls,

    arrayOfUniqueItems,
    arrayOfUniqueObjects,
    arrayOfUniqueArrays,
    arrayOfUniqueStrings,
    arrayOfUniqueIntegers,
    arrayOfUniqueDoubles,
    arrayOfUniqueNumbers,
    arrayOfUniqueReals,
    arrayOfUniqueBooleans,
    arrayOfUniqueNulls,
};

int json_is(const json *, enum json_query);

enum json_type json_type(const json *);
const char *json_type_name(const json *);
const char *json_name(const json *);
const char *json_string(const json *);
long json_integer(const json *);
double json_double(const json *);
double json_number(const json *);
unsigned long json_real(const json *);
int json_boolean(const json *);
int json_is_any(const json *);
int json_is_object(const json *);
int json_is_array(const json *);
int json_is_string(const json *);
int json_is_integer(const json *);
int json_is_double(const json *);
int json_is_number(const json *);
int json_is_real(const json *);
int json_is_boolean(const json *);
int json_is_true(const json *);
int json_is_false(const json *);
int json_is_null(const json *);
json *json_self(const json *);
json *json_root(const json *);
json *json_parent(const json *);
json *json_child(const json *);
json *json_next(const json *);
json *json_find(const json *, const char *);
json *json_find_next(const json *, const char *);
json *json_match(const json *, const char *, size_t);
json *json_node(const json *, const char *);
json *json_item(const json *, size_t);
size_t json_items(const json *);
size_t json_depth(const json *);
int json_equal(const json *, const json *);
int json_traverse(const json *, json_callback, void *);

#endif /* JSON_FILTER_H */

