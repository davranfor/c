#include <stdio.h>
#include <stdlib.h>
#include "safe_maths.h"

/* Implementation */
#define X(title, type, min, max)                                            \
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
    return (type)(a / b);                                                   \
}
SAFE_SIGNED_MATHS
#undef X

