#ifndef SAFE_MATHS_H
#define SAFE_MATHS_H

#include <stdint.h>
#include <limits.h>

#define SAFE_SIGNED_MATHS                       \
    X(char, signed char, SCHAR_MIN, SCHAR_MAX)  \
    X(short, short, SHRT_MIN, SHRT_MAX)         \
    X(int, int, INT_MIN, INT_MAX)               \
    X(long, long, LONG_MIN, LONG_MAX)           \
    X(llong, long long, LLONG_MIN, LLONG_MAX)   \
    X(int8, int8_t, INT8_MIN, INT8_MAX)         \
    X(int16, int16_t, INT16_MIN, INT16_MAX)     \
    X(int32, int32_t, INT32_MIN, INT32_MAX)     \
    X(int64, int64_t, INT64_MIN, INT64_MAX)

/* Prototypes */
#define X(title, type, min, max)                    \
type safe_##title##_add(long long a, long long b);  \
type safe_##title##_sub(long long a, long long b);  \
type safe_##title##_mul(long long a, long long b);  \
type safe_##title##_div(long long a, long long b);
SAFE_SIGNED_MATHS
#undef X

#endif /* SAFE_MATHS_H */

