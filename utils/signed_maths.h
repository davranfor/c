#ifndef SIGNED_MATHS_H
#define SIGNED_MATHS_H

#include <stdint.h>
#include <limits.h>

#define SIGNED_MATHS                                        \
    X(char, signed char, SCHAR_MIN, SCHAR_MAX, UCHAR_MAX)   \
    X(short, short, SHRT_MIN, SHRT_MAX, USHRT_MAX)          \
    X(int, int, INT_MIN, INT_MAX, UINT_MAX)                 \
    X(long, long, LONG_MIN, LONG_MAX, ULONG_MAX)            \
    X(llong, long long, LLONG_MIN, LLONG_MAX, ULLONG_MAX)   \
    X(int8, int8_t, INT8_MIN, INT8_MAX, UINT8_MAX)          \
    X(int16, int16_t, INT16_MIN, INT16_MAX, UINT16_MAX)     \
    X(int32, int32_t, INT32_MIN, INT32_MAX, UINT32_MAX)     \
    X(int64, int64_t, INT64_MIN, INT64_MAX, UINT64_MAX)

/* Prototypes */
#define X(title, type, min, max, umax)          \
type safe_##title##_add(long long, long long);  \
type safe_##title##_sub(long long, long long);  \
type safe_##title##_mul(long long, long long);  \
type safe_##title##_div(long long, long long);  \
                                                \
type range_##title##_add(long long, long long); \
type range_##title##_sub(long long, long long); \
type range_##title##_mul(long long, long long); \
type range_##title##_div(long long, long long); \
                                                \
type wrap_##title##_add(long long, long long);  \
type wrap_##title##_sub(long long, long long);  \
type wrap_##title##_mul(long long, long long);  \
type wrap_##title##_div(long long, long long);
SIGNED_MATHS
#undef X

#endif /* SIGNED_MATHS_H */

