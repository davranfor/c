#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "signed_maths.h"

#define SIGNED_MATHS_ABORT(a, b, error)                                 \
    fprintf(stderr, "Aborted: %s(%" PRIdMAX ", %" PRIdMAX ") -> %s\n",  \
            __func__, a, b, error);                                     \
    exit(EXIT_FAILURE)

#define X(name, type, min, max, umax)                               \
type name##_safe_add(intmax_t a, intmax_t b)                        \
{                                                                   \
    if ((a < 0) && (b < min - a))                                   \
    {                                                               \
        SIGNED_MATHS_ABORT(a, b, "underflow");                      \
    }                                                               \
    if ((a > 0) && (b > max - a))                                   \
    {                                                               \
        SIGNED_MATHS_ABORT(a, b, "overflow");                       \
    }                                                               \
    return (type)(a + b);                                           \
}                                                                   \
type name##_safe_sub(intmax_t a, intmax_t b)                        \
{                                                                   \
    if ((a < 0) && (b > max + a))                                   \
    {                                                               \
        SIGNED_MATHS_ABORT(a, b, "underflow");                      \
    }                                                               \
    if ((a > 0) && (b < min + a))                                   \
    {                                                               \
        SIGNED_MATHS_ABORT(a, b, "overflow");                       \
    }                                                               \
    return (type)(a - b);                                           \
}                                                                   \
type name##_safe_mul(intmax_t a, intmax_t b)                        \
{                                                                   \
    if (b == 0)                                                     \
    {                                                               \
        return 0;                                                   \
    }                                                               \
    if (((a == -1) && (b == min)) || ((b == -1) && (a == min)))     \
    {                                                               \
        SIGNED_MATHS_ABORT(a, b, "overflow");                       \
    }                                                               \
    if (a < min / b)                                                \
    {                                                               \
        SIGNED_MATHS_ABORT(a, b, "underflow");                      \
    }                                                               \
    if (a > max / b)                                                \
    {                                                               \
        SIGNED_MATHS_ABORT(a, b, "overflow");                       \
    }                                                               \
    return (type)(a * b);                                           \
}                                                                   \
type name##_safe_div(intmax_t a, intmax_t b)                        \
{                                                                   \
    if (b == 0)                                                     \
    {                                                               \
        SIGNED_MATHS_ABORT(a, b, "division by 0");                  \
    }                                                               \
    if (((a == -1) && (b == min)) || ((b == -1) && (a == min)))     \
    {                                                               \
        SIGNED_MATHS_ABORT(a, b, "overflow");                       \
    }                                                               \
                                                                    \
    intmax_t result = a / b;                                        \
                                                                    \
    if (min > result)                                               \
    {                                                               \
        SIGNED_MATHS_ABORT(a, b, "underflow");                      \
    }                                                               \
    if (max < result)                                               \
    {                                                               \
        SIGNED_MATHS_ABORT(a, b, "overflow");                       \
    }                                                               \
    return (type)result;                                            \
}                                                                   \
type name##_safe_mod(intmax_t a, intmax_t b)                        \
{                                                                   \
    if (b == 0)                                                     \
    {                                                               \
        SIGNED_MATHS_ABORT(a, b, "division by 0");                  \
    }                                                               \
    if (b == -1)                                                    \
    {                                                               \
        return 0;                                                   \
    }                                                               \
                                                                    \
    intmax_t result = a % b;                                        \
                                                                    \
    if (min > result)                                               \
    {                                                               \
        SIGNED_MATHS_ABORT(a, b, "underflow");                      \
    }                                                               \
    if (max < result)                                               \
    {                                                               \
        SIGNED_MATHS_ABORT(a, b, "overflow");                       \
    }                                                               \
    return (type)result;                                            \
}                                                                   \
                                                                    \
type name##_range_add(intmax_t a, intmax_t b)                       \
{                                                                   \
    if ((a < 0) && (b < min - a))                                   \
    {                                                               \
        return min;                                                 \
    }                                                               \
    if ((a > 0) && (b > max - a))                                   \
    {                                                               \
        return max;                                                 \
    }                                                               \
    return (type)(a + b);                                           \
}                                                                   \
type name##_range_sub(intmax_t a, intmax_t b)                       \
{                                                                   \
    if ((a < 0) && (b > max + a))                                   \
    {                                                               \
        return min;                                                 \
    }                                                               \
    if ((a > 0) && (b < min + a))                                   \
    {                                                               \
        return max;                                                 \
    }                                                               \
    return (type)(a - b);                                           \
}                                                                   \
type name##_range_mul(intmax_t a, intmax_t b)                       \
{                                                                   \
    if (b == 0)                                                     \
    {                                                               \
        return 0;                                                   \
    }                                                               \
    if (((a == -1) && (b == min)) || ((b == -1) && (a == min)))     \
    {                                                               \
        return max;                                                 \
    }                                                               \
    if (a < min / b)                                                \
    {                                                               \
        return min;                                                 \
    }                                                               \
    if (a > max / b)                                                \
    {                                                               \
        return max;                                                 \
    }                                                               \
    return (type)(a * b);                                           \
}                                                                   \
type name##_range_div(intmax_t a, intmax_t b)                       \
{                                                                   \
    if (b == 0)                                                     \
    {                                                               \
        SIGNED_MATHS_ABORT(a, b, "division by 0");                  \
    }                                                               \
    if (((a == -1) && (b == min)) || ((b == -1) && (a == min)))     \
    {                                                               \
        return max;                                                 \
    }                                                               \
                                                                    \
    intmax_t result = a / b;                                        \
                                                                    \
    if (min > result)                                               \
    {                                                               \
        return min;                                                 \
    }                                                               \
    if (max < result)                                               \
    {                                                               \
        return max;                                                 \
    }                                                               \
    return (type)result;                                            \
}                                                                   \
type name##_range_mod(intmax_t a, intmax_t b)                       \
{                                                                   \
    if (b == 0)                                                     \
    {                                                               \
        SIGNED_MATHS_ABORT(a, b, "division by 0");                  \
    }                                                               \
    if (b == -1)                                                    \
    {                                                               \
        return 0;                                                   \
    }                                                               \
                                                                    \
    intmax_t result = a % b;                                        \
                                                                    \
    if (min > result)                                               \
    {                                                               \
        return min;                                                 \
    }                                                               \
    if (max < result)                                               \
    {                                                               \
        return max;                                                 \
    }                                                               \
    return (type)result;                                            \
}                                                                   \
                                                                    \
type name##_wrap_add(intmax_t a, intmax_t b)                        \
{                                                                   \
    if (((a < 0) && (b < min - a)) || ((a > 0) && (b > max - a)))   \
    {                                                               \
        return (type)(((uintmax_t)a + (uintmax_t)b) & umax);        \
    }                                                               \
    return (type)(a + b);                                           \
}                                                                   \
type name##_wrap_sub(intmax_t a, intmax_t b)                        \
{                                                                   \
    if (((a < 0) && (b > max + a)) || ((a > 0) && (b < min + a)))   \
    {                                                               \
        return (type)(((uintmax_t)a - (uintmax_t)b) & umax);        \
    }                                                               \
    return (type)(a - b);                                           \
}                                                                   \
type name##_wrap_mul(intmax_t a, intmax_t b)                        \
{                                                                   \
    if (b == 0)                                                     \
    {                                                               \
        return 0;                                                   \
    }                                                               \
    if (((a == -1) && (b == min)) || ((b == -1) && (a == min)))     \
    {                                                               \
        return min;                                                 \
    }                                                               \
    if ((a < min / b) || (a > max / b))                             \
    {                                                               \
        return (type)(((uintmax_t)a * (uintmax_t)b) & umax);        \
    }                                                               \
    return (type)(a * b);                                           \
}                                                                   \
type name##_wrap_div(intmax_t a, intmax_t b)                        \
{                                                                   \
    if (b == 0)                                                     \
    {                                                               \
        SIGNED_MATHS_ABORT(a, b, "division by 0");                  \
    }                                                               \
    if (((a == -1) && (b == min)) || ((b == -1) && (a == min)))     \
    {                                                               \
        return min;                                                 \
    }                                                               \
                                                                    \
    intmax_t result = a / b;                                        \
                                                                    \
    if ((min > result) || (max < result))                           \
    {                                                               \
        return (type)((uintmax_t)result & umax);                    \
    }                                                               \
    return (type)result;                                            \
}                                                                   \
type name##_wrap_mod(intmax_t a, intmax_t b)                        \
{                                                                   \
    if (b == 0)                                                     \
    {                                                               \
        SIGNED_MATHS_ABORT(a, b, "division by 0");                  \
    }                                                               \
    if (b == -1)                                                    \
    {                                                               \
        return 0;                                                   \
    }                                                               \
                                                                    \
    intmax_t result = a % b;                                        \
                                                                    \
    if ((min > result) || (max < result))                           \
    {                                                               \
        return (type)((uintmax_t)result & umax);                    \
    }                                                               \
    return (type)result;                                            \
}
SIGNED_MATHS
#undef X


