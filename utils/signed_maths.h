#ifndef SIGNED_MATHS_H
#define SIGNED_MATHS_H

#include <stdint.h>
#include <limits.h>

#define SIGNED_MATHS                                                \
    X(char, signed char, unsigned char, SCHAR_MIN, SCHAR_MAX)       \
    X(short, short, unsigned short, SHRT_MIN, SHRT_MAX)             \
    X(int, int, unsigned int, INT_MIN, INT_MAX )                    \
    X(long, long, unsigned long, LONG_MIN, LONG_MAX)                \
    X(llong, long long, unsigned long long, LLONG_MIN, LLONG_MAX)   \
    X(int8, int8_t, uint8_t, INT8_MIN, INT8_MAX)                    \
    X(int16, int16_t, uint16_t, INT16_MIN, INT16_MAX)               \
    X(int32, int32_t, uint32_t, INT32_MIN, INT32_MAX)               \
    X(int64, int64_t, uint64_t, INT64_MIN, INT64_MAX)

/* Prototypes */
#define X(name, type, min, max, umax)       \
type name##_safe_add(intmax_t, intmax_t);   \
type name##_safe_sub(intmax_t, intmax_t);   \
type name##_safe_mul(intmax_t, intmax_t);   \
type name##_safe_div(intmax_t, intmax_t);   \
type name##_safe_mod(intmax_t, intmax_t);   \
                                            \
type name##_range_add(intmax_t, intmax_t);  \
type name##_range_sub(intmax_t, intmax_t);  \
type name##_range_mul(intmax_t, intmax_t);  \
type name##_range_div(intmax_t, intmax_t);  \
type name##_range_mod(intmax_t, intmax_t);  \
                                            \
type name##_wrap_add(intmax_t, intmax_t);   \
type name##_wrap_sub(intmax_t, intmax_t);   \
type name##_wrap_mul(intmax_t, intmax_t);   \
type name##_wrap_div(intmax_t, intmax_t);   \
type name##_wrap_mod(intmax_t, intmax_t);
SIGNED_MATHS
#undef X

#endif /* SIGNED_MATHS_H */

