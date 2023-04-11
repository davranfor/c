/*!
 *  \brief     JSON
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef JSON_H
#define JSON_H

typedef struct json json;
typedef struct {int line, column;} json_error;
typedef int (*json_callback)(const json *, int, void *);

enum json_type
{
    JSON_UNDEFINED,
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_STRING,
    JSON_INTEGER,
    JSON_DOUBLE,
    JSON_BOOLEAN,
    JSON_NULL,
    JSON_TYPES,
};

enum json_query
{
    objectOfItems,
    objectOfIterables,
    objectOfScalars,
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
    arrayOfIterables,
    arrayOfScalars,
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
    objectOfOptionalIterables,
    objectOfOptionalScalars,
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
    arrayOfOptionalIterables,
    arrayOfOptionalScalars,
    arrayOfOptionalObjects,
    arrayOfOptionalArrays,
    arrayOfOptionalStrings,
    arrayOfOptionalIntegers,
    arrayOfOptionalDoubles,
    arrayOfOptionalNumbers,
    arrayOfOptionalReals,
    arrayOfOptionalBooleans,
    arrayOfOptionalNulls,

    objectOfUniqueItems,
    objectOfUniqueIterables,
    objectOfUniqueScalars,
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
    arrayOfUniqueIterables,
    arrayOfUniqueScalars,
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

// ============================================================================
// Builder
// ============================================================================
json *json_new_object(const char *);
json *json_new_array(const char *);
json *json_new_string(const char *, const char *);
json *json_new_integer(const char *, long long);
json *json_new_real(const char *, unsigned long long);
json *json_new_double(const char *, double, int);
json *json_new_boolean(const char *, int);
json *json_new_null(const char *);
json *json_push_front(json *, json *);
json *json_push_back(json *, json *);
json *json_push_fast(json *, json *, json *);
json *json_append_to(json *, json *);
void json_free(json *);
// ============================================================================
// Parser
// ============================================================================
json *json_parse(const char *, json_error *);
json *json_parse_file(const char *, json_error *);
void json_print_error(const char *, const json_error *);
// ============================================================================
// Reader
// ============================================================================
enum json_type json_type(const json *);
const char *json_type_name(const json *);
const char *json_name(const json *);
const char *json_string(const json *);
long long json_integer(const json *);
unsigned long long json_real(const json *);
double json_double(const json *);
double json_number(const json *);
int json_boolean(const json *);
int json_is(const json *, enum json_query);
int json_is_any(const json *);
int json_is_iterable(const json *);
int json_is_scalar(const json *);
int json_is_object(const json *);
int json_is_array(const json *);
int json_is_string(const json *);
int json_is_integer(const json *);
int json_is_real(const json *);
int json_is_double(const json *);
int json_is_number(const json *);
int json_is_boolean(const json *);
int json_is_true(const json *);
int json_is_false(const json *);
int json_is_null(const json *);
json *json_self(const json *);
json *json_root(const json *);
json *json_parent(const json *);
json *json_child(const json *);
json *json_next(const json *);
json *json_last(const json *);
json *json_find(const json *, const char *);
json *json_find_next(const json *, const char *);
json *json_match(const json *, const char *, size_t);
json *json_node(const json *, const char *);
json *json_item(const json *, size_t);
size_t json_items(const json *);
size_t json_offset(const json *);
int json_depth(const json *);
int json_equal(const json *, const json *);
int json_traverse(const json *, json_callback, void *);
// ============================================================================
// Writer
// ============================================================================
char *json_encode(const json *);
int json_write(const json *, FILE *);
int json_print(const json *);
char *json_path(const json *);

#endif /* JSON_H */

