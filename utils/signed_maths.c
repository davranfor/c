#include <stdio.h>
#include <stdlib.h>
#include "signed_maths.h"

/* Implementation */
#define X(title, type, min, max, umax)                                      \
type safe_##title##_add(long long a, long long b)                           \
{                                                                           \
    if ((a > 0) && (b > max - a))                                           \
    {                                                                       \
        fprintf(stderr, "%s(%lld, %lld) overflows\n", __func__, a, b);      \
        exit(EXIT_FAILURE);                                                 \
    }                                                                       \
    if ((a < 0) && (b < min - a))                                           \
    {                                                                       \
        fprintf(stderr, "%s(%lld, %lld) underflows\n", __func__, a, b);     \
        exit(EXIT_FAILURE);                                                 \
    }                                                                       \
    return (type)(a + b);                                                   \
}                                                                           \
type safe_##title##_sub(long long a, long long b)                           \
{                                                                           \
    if ((a > 0) && (b < min + a))                                           \
    {                                                                       \
        fprintf(stderr, "%s(%lld, %lld) overflows\n", __func__, a, b);      \
        exit(EXIT_FAILURE);                                                 \
    }                                                                       \
    if ((a < 0) && (b > max + a))                                           \
    {                                                                       \
        fprintf(stderr, "%s(%lld, %lld) underflows\n", __func__, a, b);     \
        exit(EXIT_FAILURE);                                                 \
    }                                                                       \
    return (type)(a - b);                                                   \
}                                                                           \
type safe_##title##_mul(long long a, long long b)                           \
{                                                                           \
    if ((a == 0) || (b == 0))                                               \
    {                                                                       \
        return 0;                                                           \
    }                                                                       \
    if (((a == -1) && (b == min)) || ((b == -1) && (a == min)))             \
    {                                                                       \
        fprintf(stderr, "%s(%lld, %lld) overflows\n", __func__, a, b);      \
        exit(EXIT_FAILURE);                                                 \
    }                                                                       \
    if (a > max / b)                                                        \
    {                                                                       \
        fprintf(stderr, "%s(%lld, %lld) overflows\n", __func__, a, b);      \
        exit(EXIT_FAILURE);                                                 \
    }                                                                       \
    if (a < min / b)                                                        \
    {                                                                       \
        fprintf(stderr, "%s(%lld, %lld) underflows\n", __func__, a, b);     \
        exit(EXIT_FAILURE);                                                 \
    }                                                                       \
    return (type)(a * b);                                                   \
}                                                                           \
type safe_##title##_div(long long a, long long b)                           \
{                                                                           \
    if (a == 0)                                                             \
    {                                                                       \
        return 0;                                                           \
    }                                                                       \
    if (b == 0)                                                             \
    {                                                                       \
        fprintf(stderr, "%s(%lld, %lld) division by 0\n", __func__, a, b);  \
        exit(EXIT_FAILURE);                                                 \
    }                                                                       \
    if (((a == -1) && (b == min)) || ((b == -1) && (a == min)))             \
    {                                                                       \
        fprintf(stderr, "%s(%lld, %lld) overflows\n", __func__, a, b);      \
        exit(EXIT_FAILURE);                                                 \
    }                                                                       \
    if (max < a / b)                                                        \
    {                                                                       \
        fprintf(stderr, "%s(%lld, %lld) overflows\n", __func__, a, b);      \
        exit(EXIT_FAILURE);                                                 \
    }                                                                       \
    if (min > a / b)                                                        \
    {                                                                       \
        fprintf(stderr, "%s(%lld, %lld) underflows\n", __func__, a, b);     \
        exit(EXIT_FAILURE);                                                 \
    }                                                                       \
    return (type)(a / b);                                                   \
}                                                                           \
                                                                            \
type range_##title##_add(long long a, long long b)                          \
{                                                                           \
    if ((a > 0) && (b > max - a))                                           \
    {                                                                       \
        return max;                                                         \
    }                                                                       \
    if ((a < 0) && (b < min - a))                                           \
    {                                                                       \
        return min;                                                         \
    }                                                                       \
    return (type)(a + b);                                                   \
}                                                                           \
type range_##title##_sub(long long a, long long b)                          \
{                                                                           \
    if ((a > 0) && (b < min + a))                                           \
    {                                                                       \
        return max;                                                         \
    }                                                                       \
    if ((a < 0) && (b > max + a))                                           \
    {                                                                       \
        return min;                                                         \
    }                                                                       \
    return (type)(a - b);                                                   \
}                                                                           \
type range_##title##_mul(long long a, long long b)                          \
{                                                                           \
    if ((a == 0) || (b == 0))                                               \
    {                                                                       \
        return 0;                                                           \
    }                                                                       \
    if (((a == -1) && (b == min)) || ((b == -1) && (a == min)))             \
    {                                                                       \
        return max;                                                         \
    }                                                                       \
    if (a > max / b)                                                        \
    {                                                                       \
        return max;                                                         \
    }                                                                       \
    if (a < min / b)                                                        \
    {                                                                       \
        return min;                                                         \
    }                                                                       \
    return (type)(a * b);                                                   \
}                                                                           \
type range_##title##_div(long long a, long long b)                          \
{                                                                           \
    if (a == 0)                                                             \
    {                                                                       \
        return 0;                                                           \
    }                                                                       \
    if (b == 0)                                                             \
    {                                                                       \
        fprintf(stderr, "%s(%lld, %lld) division by 0\n", __func__, a, b);  \
        exit(EXIT_FAILURE);                                                 \
    }                                                                       \
    if (((a == -1) && (b == min)) || ((b == -1) && (a == min)))             \
    {                                                                       \
        return max;                                                         \
    }                                                                       \
    if (max < a / b)                                                        \
    {                                                                       \
        return max;                                                         \
    }                                                                       \
    if (min > a / b)                                                        \
    {                                                                       \
        return min;                                                         \
    }                                                                       \
    return (type)(a / b);                                                   \
}                                                                           \
                                                                            \
type wrap_##title##_add(long long a, long long  b)                          \
{                                                                           \
    if (((a > 0) && (b > max - a)) || ((a < 0) && (b < min - a)))           \
    {                                                                       \
        unsigned long long x = (unsigned long long)a;                       \
        unsigned long long y = (unsigned long long)b;                       \
                                                                            \
        return (type)((x + y) & umax);                                      \
    }                                                                       \
    return (type)(a + b);                                                   \
}                                                                           \
type wrap_##title##_sub(long long a, long long  b)                          \
{                                                                           \
    if (((a > 0) && (b < min + a)) || ((a < 0) && (b > max + a)))           \
    {                                                                       \
        unsigned long long x = (unsigned long long)a;                       \
        unsigned long long y = (unsigned long long)b;                       \
                                                                            \
        return (type)((x - y) & umax);                                      \
    }                                                                       \
    return (type)(a - b);                                                   \
}                                                                           \
type wrap_##title##_mul(long long a, long long  b)                          \
{                                                                           \
    if ((a == 0) || (b == 0))                                               \
    {                                                                       \
        return 0;                                                           \
    }                                                                       \
    if (((a == -1) && (b == min)) || ((b == -1) && (a == min)))             \
    {                                                                       \
        return min;                                                         \
    }                                                                       \
    if ((a > max / b) || (a < min / b))                                     \
    {                                                                       \
        unsigned long long x = (unsigned long long)a;                       \
        unsigned long long y = (unsigned long long)b;                       \
                                                                            \
        return (type)((x * y) & umax);                                      \
    }                                                                       \
    return (type)(a * b);                                                   \
}                                                                           \
type wrap_##title##_div(long long a, long long  b)                          \
{                                                                           \
    if (a == 0)                                                             \
    {                                                                       \
        return 0;                                                           \
    }                                                                       \
    if (b == 0)                                                             \
    {                                                                       \
        fprintf(stderr, "%s(%lld, %lld) division by 0\n", __func__, a, b);  \
        exit(EXIT_FAILURE);                                                 \
    }                                                                       \
    if (((a == -1) && (b == min)) || ((b == -1) && (a == min)))             \
    {                                                                       \
        return min;                                                         \
    }                                                                       \
    if ((max < a / b) || (min > a / b))                                     \
    {                                                                       \
        unsigned long long x = (unsigned long long)a;                       \
        unsigned long long y = (unsigned long long)b;                       \
                                                                            \
        return (type)((x / y) & umax);                                      \
    }                                                                       \
    return (type)(a / b);                                                   \
}
SIGNED_MATHS
#undef X


