#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "signed_maths.h"

#define SIGNED_MATHS_ABORT(a, b, error)                                 \
    fprintf(stderr, "Aborted: %s(%" PRIdMAX ", %" PRIdMAX ") -> %s\n",  \
            __func__, a, b, error);                                     \
    exit(EXIT_FAILURE)

#define X(title, type, min, max, umax)                              \
type safe_##title##_add(intmax_t a, intmax_t b)                     \
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
type safe_##title##_sub(intmax_t a, intmax_t b)                     \
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
type safe_##title##_mul(intmax_t a, intmax_t b)                     \
{                                                                   \
    if ((a == 0) || (b == 0))                                       \
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
type safe_##title##_div(intmax_t a, intmax_t b)                     \
{                                                                   \
    if (a == 0)                                                     \
    {                                                               \
        return 0;                                                   \
    }                                                               \
    if (b == 0)                                                     \
    {                                                               \
        SIGNED_MATHS_ABORT(a, b, "division by 0");                  \
    }                                                               \
    if (((a == -1) && (b == min)) || ((b == -1) && (a == min)))     \
    {                                                               \
        SIGNED_MATHS_ABORT(a, b, "overflow");                       \
    }                                                               \
    if (min > a / b)                                                \
    {                                                               \
        SIGNED_MATHS_ABORT(a, b, "underflow");                      \
    }                                                               \
    if (max < a / b)                                                \
    {                                                               \
        SIGNED_MATHS_ABORT(a, b, "overflow");                       \
    }                                                               \
    return (type)(a / b);                                           \
}                                                                   \
                                                                    \
type range_##title##_add(intmax_t a, intmax_t b)                    \
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
type range_##title##_sub(intmax_t a, intmax_t b)                    \
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
type range_##title##_mul(intmax_t a, intmax_t b)                    \
{                                                                   \
    if ((a == 0) || (b == 0))                                       \
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
type range_##title##_div(intmax_t a, intmax_t b)                    \
{                                                                   \
    if (a == 0)                                                     \
    {                                                               \
        return 0;                                                   \
    }                                                               \
    if (b == 0)                                                     \
    {                                                               \
        SIGNED_MATHS_ABORT(a, b, "division by 0");                  \
    }                                                               \
    if (((a == -1) && (b == min)) || ((b == -1) && (a == min)))     \
    {                                                               \
        return max;                                                 \
    }                                                               \
    if (min > a / b)                                                \
    {                                                               \
        return min;                                                 \
    }                                                               \
    if (max < a / b)                                                \
    {                                                               \
        return max;                                                 \
    }                                                               \
    return (type)(a / b);                                           \
}                                                                   \
                                                                    \
type wrap_##title##_add(intmax_t a, intmax_t b)                     \
{                                                                   \
    if (((a < 0) && (b < min - a)) || ((a > 0) && (b > max - a)))   \
    {                                                               \
        return (type)(((uintmax_t)a + (uintmax_t)b) & umax);        \
    }                                                               \
    return (type)(a + b);                                           \
}                                                                   \
type wrap_##title##_sub(intmax_t a, intmax_t b)                     \
{                                                                   \
    if (((a < 0) && (b > max + a)) || ((a > 0) && (b < min + a)))   \
    {                                                               \
        return (type)(((uintmax_t)a - (uintmax_t)b) & umax);        \
    }                                                               \
    return (type)(a - b);                                           \
}                                                                   \
type wrap_##title##_mul(intmax_t a, intmax_t b)                     \
{                                                                   \
    if ((a == 0) || (b == 0))                                       \
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
type wrap_##title##_div(intmax_t a, intmax_t b)                     \
{                                                                   \
    if (a == 0)                                                     \
    {                                                               \
        return 0;                                                   \
    }                                                               \
    if (b == 0)                                                     \
    {                                                               \
        SIGNED_MATHS_ABORT(a, b, "division by 0");                  \
    }                                                               \
    if (((a == -1) && (b == min)) || ((b == -1) && (a == min)))     \
    {                                                               \
        return min;                                                 \
    }                                                               \
    if ((min > a / b) || (max < a / b))                             \
    {                                                               \
        return (type)(((uintmax_t)a / (uintmax_t)b) & umax);        \
    }                                                               \
    return (type)(a / b);                                           \
}
SIGNED_MATHS
#undef X


