#ifndef JSON_QUERY_H
#define JSON_QUERY_H

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

#endif /* JSON_QUERY_H */

