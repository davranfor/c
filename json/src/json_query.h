#ifndef JSON_QUERY_H
#define JSON_QUERY_H

/**
 * Do not change the order nor delete any element of the enum
 * without checking then json_is() implementation
 */
enum json_query
{
    objectOfValues,
    objectOfObjects,
    objectOfArrays,
    objectOfStrings,
    objectOfIntegers,
    objectOfDoubles,
    objectOfNumbers,
    objectOfReals,
    objectOfBooleans,
    objectOfNulls,

    arrayOfValues,
    arrayOfObjects,
    arrayOfArrays,
    arrayOfStrings,
    arrayOfIntegers,
    arrayOfDoubles,
    arrayOfNumbers,
    arrayOfReals,
    arrayOfBooleans,
    arrayOfNulls,

    objectOfOptionalValues,
    objectOfOptionalObjects,
    objectOfOptionalArrays,
    objectOfOptionalStrings,
    objectOfOptionalIntegers,
    objectOfOptionalDoubles,
    objectOfOptionalNumbers,
    objectOfOptionalReals,
    objectOfOptionalBooleans,
    objectOfOptionalNulls,

    arrayOfOptionalValues,
    arrayOfOptionalObjects,
    arraytOfOptionalArrays,
    arrayOfOptionalStrings,
    arrayOfOptionalIntegers,
    arrayOfOptionalDoubles,
    arrayOfOptionalNumbers,
    arrayOfOptionalReals,
    arrayOfOptionalBooleans,
    arrayOfOptionalNulls,

    objectOfUniqueValues,
    objectOfUniqueObjects,
    objectOfUniqueArrays,
    objectOfUniqueStrings,
    objectOfUniqueIntegers,
    objectOfUniqueDoubles,
    objectOfUniqueNumbers,
    objectOfUniqueReals,
    objectOfUniqueBooleans,
    objectOfUniqueNulls,

    arrayOfUniqueValues,
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

#endif /* JSON_QUERY_H */

