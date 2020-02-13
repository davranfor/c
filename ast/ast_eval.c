#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ast_data.h"
#include "ast_eval.h"

///////////////////////////////////////////////////////////////////////////////

#define VAR(v) ((*(ast_data * const *)(v.address))[v.key])

///////////////////////////////////////////////////////////////////////////////
// Operators
///////////////////////////////////////////////////////////////////////////////

ast_data ast_plus(ast_data a, ast_data b)
{
    (void)b;
    if (a.type == TYPE_STRING)
    {
        a.number = strtod(a.string, NULL);
    }
    a.type = TYPE_NUMBER;
    return a;
}

ast_data ast_minus(ast_data a, ast_data b)
{
    (void)b;
    if (a.type == TYPE_STRING)
    {
        a.number = strtod(a.string, NULL);
    }
    a.number = -a.number;
    a.type = TYPE_NUMBER;
    return a;
}

ast_data ast_not(ast_data a, ast_data b)
{
    (void)b;
    if (a.type == TYPE_STRING)
    {
        a.number = strtod(a.string, NULL);
    }
    a.number = a.number == 0;
    a.type = TYPE_BOOLEAN;
    return a;
}

ast_data ast_bit_not(ast_data a, ast_data b)
{
    (void)b;
    if (a.type == TYPE_STRING)
    {
        a.number = strtod(a.string, NULL);
    }
    a.number = (double)(~(long)a.number);
    a.type = TYPE_NUMBER;
    return a;
}

ast_data ast_mul(ast_data a, ast_data b)
{
    if (a.type == TYPE_STRING)
    {
        a.number = strtod(a.string, NULL);
    }
    if (b.type == TYPE_STRING)
    {
        b.number = strtod(b.string, NULL);
    }
    a.number = a.number * b.number;
    a.type = TYPE_NUMBER;
    return a;
}

ast_data ast_div(ast_data a, ast_data b)
{
    if (a.type == TYPE_STRING)
    {
        a.number = strtod(a.string, NULL);
    }
    if (b.type == TYPE_STRING)
    {
        b.number = strtod(b.string, NULL);
    }
    a.number = a.number / b.number;
    a.type = TYPE_NUMBER;
    return a;
}

ast_data ast_rem(ast_data a, ast_data b)
{
    if (a.type == TYPE_STRING)
    {
        a.number = strtod(a.string, NULL);
    }
    if (b.type == TYPE_STRING)
    {
        b.number = strtod(b.string, NULL);
    }
    a.number = (double)((long)a.number % (long)b.number);
    a.type = TYPE_NUMBER;
    return a;
}

ast_data ast_add(ast_data a, ast_data b)
{
    if (a.type == TYPE_STRING)
    {
        a.number = strtod(a.string, NULL);
    }
    if (b.type == TYPE_STRING)
    {
        b.number = strtod(b.string, NULL);
    }
    a.number = a.number + b.number;
    a.type = TYPE_NUMBER;
    return a;
}

ast_data ast_sub(ast_data a, ast_data b)
{
    if (a.type == TYPE_STRING)
    {
        a.number = strtod(a.string, NULL);
    }
    if (b.type == TYPE_STRING)
    {
        b.number = strtod(b.string, NULL);
    }
    a.number = a.number - b.number;
    a.type = TYPE_NUMBER;
    return a;
}

ast_data ast_bit_lshift(ast_data a, ast_data b)
{
    if (a.type == TYPE_STRING)
    {
        a.number = strtod(a.string, NULL);
    }
    if (b.type == TYPE_STRING)
    {
        b.number = strtod(b.string, NULL);
    }
    a.number = (double)((long)a.number << (unsigned long)b.number);
    a.type = TYPE_NUMBER;
    return a;
}

ast_data ast_bit_rshift(ast_data a, ast_data b)
{
    if (a.type == TYPE_STRING)
    {
        a.number = strtod(a.string, NULL);
    }
    if (b.type == TYPE_STRING)
    {
        b.number = strtod(b.string, NULL);
    }
    a.number = (double)((long)a.number >> (unsigned long)b.number);
    a.type = TYPE_NUMBER;
    return a;
}

ast_data ast_lt(ast_data a, ast_data b)
{
    if (a.type == b.type)
    {
        if (a.type == TYPE_STRING)
        {
            a.number = strcmp(a.string, b.string) < 0;
        }
        else
        {
            a.number = a.number < b.number;
        }
    }
    else
    {
        if (a.type == TYPE_STRING)
        {
            a.number = strtod(a.string, NULL);
        }
        if (b.type == TYPE_STRING)
        {
            b.number = strtod(b.string, NULL);
        }
        a.number = a.number < b.number;
    }
    a.type = TYPE_BOOLEAN;
    return a;
}

ast_data ast_gt(ast_data a, ast_data b)
{
    if (a.type == b.type)
    {
        if (a.type == TYPE_STRING)
        {
            a.number = strcmp(a.string, b.string) > 0;
        }
        else
        {
            a.number = a.number > b.number;
        }
    }
    else
    {
        if (a.type == TYPE_STRING)
        {
            a.number = strtod(a.string, NULL);
        }
        if (b.type == TYPE_STRING)
        {
            b.number = strtod(b.string, NULL);
        }
        a.number = a.number > b.number;
    }
    a.type = TYPE_BOOLEAN;
    return a;
}

ast_data ast_lt_or_eq(ast_data a, ast_data b)
{
    if (a.type == b.type)
    {
        if (a.type == TYPE_STRING)
        {
            a.number = strcmp(a.string, b.string) <= 0;
        }
        else
        {
            a.number = a.number <= b.number;
        }
    }
    else
    {
        if (a.type == TYPE_STRING)
        {
            a.number = strtod(a.string, NULL);
        }
        if (b.type == TYPE_STRING)
        {
            b.number = strtod(b.string, NULL);
        }
        a.number = a.number <= b.number;
    }
    a.type = TYPE_BOOLEAN;
    return a;
}

ast_data ast_gt_or_eq(ast_data a, ast_data b)
{
    if (a.type == b.type)
    {
        if (a.type == TYPE_STRING)
        {
            a.number = strcmp(a.string, b.string) >= 0;
        }
        else
        {
            a.number = a.number >= b.number;
        }
    }
    else
    {
        if (a.type == TYPE_STRING)
        {
            a.number = strtod(a.string, NULL);
        }
        if (b.type == TYPE_STRING)
        {
            b.number = strtod(b.string, NULL);
        }
        a.number = a.number >= b.number;
    }
    a.type = TYPE_BOOLEAN;
    return a;
}

ast_data ast_is_eq(ast_data a, ast_data b)
{
    if (a.type == b.type)
    {
        if (a.type == TYPE_STRING)
        {
            a.number = a.string == b.string;
        }
        else
        {
            a.number = a.number == b.number;
        }
    }
    else
    {
        if (a.type == TYPE_STRING)
        {
            a.number = strtod(a.string, NULL);
        }
        if (b.type == TYPE_STRING)
        {
            b.number = strtod(b.string, NULL);
        }
        a.number = a.number == b.number;
    }
    a.type = TYPE_BOOLEAN;
    return a;
}

ast_data ast_not_eq(ast_data a, ast_data b)
{
    if (a.type == b.type)
    {
        if (a.type == TYPE_STRING)
        {
            a.number = a.string != b.string;
        }
        else
        {
            a.number = a.number != b.number;
        }
    }
    else
    {
        if (a.type == TYPE_STRING)
        {
            a.number = strtod(a.string, NULL);
        }
        if (b.type == TYPE_STRING)
        {
            b.number = strtod(b.string, NULL);
        }
        a.number = a.number != b.number;
    }
    a.type = TYPE_BOOLEAN;
    return a;
}

ast_data ast_identical(ast_data a, ast_data b)
{
    if (a.type != b.type)
    {
        a.number = 0;
    }
    else
    {
        if (a.type == TYPE_STRING)
        {
            a.number = a.string == b.string;
        }
        else
        {
            a.number = a.number == b.number;
        }
    }
    a.type = TYPE_BOOLEAN;
    return a;
}

ast_data ast_not_identical(ast_data a, ast_data b)
{
    if (a.type != b.type)
    {
        a.number = 1;
    }
    else
    {
        if (a.type == TYPE_STRING)
        {
            a.number = a.string != b.string;
        }
        else
        {
            a.number = a.number != b.number;
        }
    }
    a.type = TYPE_BOOLEAN;
    return a;
}

ast_data ast_bit_and(ast_data a, ast_data b)
{
    if (a.type == TYPE_STRING)
    {
        a.number = strtod(a.string, NULL);
    }
    if (b.type == TYPE_STRING)
    {
        b.number = strtod(b.string, NULL);
    }
    a.number = (double)((long)a.number & (long)b.number);
    a.type = TYPE_NUMBER;
    return a;
}

ast_data ast_bit_xor(ast_data a, ast_data b)
{
    if (a.type == TYPE_STRING)
    {
        a.number = strtod(a.string, NULL);
    }
    if (b.type == TYPE_STRING)
    {
        b.number = strtod(b.string, NULL);
    }
    a.number = (double)((long)a.number ^ (long)b.number);
    a.type = TYPE_NUMBER;
    return a;
}

ast_data ast_bit_or(ast_data a, ast_data b)
{
    if (a.type == TYPE_STRING)
    {
        a.number = strtod(a.string, NULL);
    }
    if (b.type == TYPE_STRING)
    {
        b.number = strtod(b.string, NULL);
    }
    a.number = (double)((long)a.number | (long)b.number);
    a.type = TYPE_NUMBER;
    return a;
}

ast_data ast_and(ast_data a, ast_data b)
{
    if (a.type == TYPE_STRING)
    {
        a.number = strtod(a.string, NULL);
    }
    if (b.type == TYPE_STRING)
    {
        b.number = strtod(b.string, NULL);
    }
    a.number = a.number && b.number;
    a.type = TYPE_NUMBER;
    return a;
}

ast_data ast_xor(ast_data a, ast_data b)
{
    if (a.type == TYPE_STRING)
    {
        a.number = strtod(a.string, NULL);
    }
    if (b.type == TYPE_STRING)
    {
        b.number = strtod(b.string, NULL);
    }
    a.number = (a.number || b.number) && !(a.number && b.number);
    a.type = TYPE_NUMBER;
    return a;
}

ast_data ast_or(ast_data a, ast_data b)
{
    if (a.type == TYPE_STRING)
    {
        a.number = strtod(a.string, NULL);
    }
    if (b.type == TYPE_STRING)
    {
        b.number = strtod(b.string, NULL);
    }
    a.number = a.number || b.number;
    a.type = TYPE_NUMBER;
    return a;
}

ast_data ast_eq(ast_data a, ast_data b)
{
    VAR(a) = b;
    return b;
}

ast_data ast_eq_add(ast_data a, ast_data b)
{
    if (VAR(a).type == TYPE_STRING)
    {
        VAR(a).number = strtod(VAR(a).string, NULL);
    }
    if (b.type == TYPE_STRING)
    {
        b.number = strtod(b.string, NULL);
    }
    VAR(a).number += b.number;
    VAR(a).type = TYPE_NUMBER;
    return VAR(a);
}

ast_data ast_eq_sub(ast_data a, ast_data b)
{
    if (VAR(a).type == TYPE_STRING)
    {
        VAR(a).number = strtod(VAR(a).string, NULL);
    }
    if (b.type == TYPE_STRING)
    {
        b.number = strtod(b.string, NULL);
    }
    VAR(a).number -= b.number;
    VAR(a).type = TYPE_NUMBER;
    return VAR(a);
}

ast_data ast_eq_mul(ast_data a, ast_data b)
{
    if (VAR(a).type == TYPE_STRING)
    {
        VAR(a).number = strtod(VAR(a).string, NULL);
    }
    if (b.type == TYPE_STRING)
    {
        b.number = strtod(b.string, NULL);
    }
    VAR(a).number *= b.number;
    VAR(a).type = TYPE_NUMBER;
    return VAR(a);
}

ast_data ast_eq_div(ast_data a, ast_data b)
{
    if (VAR(a).type == TYPE_STRING)
    {
        VAR(a).number = strtod(VAR(a).string, NULL);
    }
    if (b.type == TYPE_STRING)
    {
        b.number = strtod(b.string, NULL);
    }
    VAR(a).number /= b.number;
    VAR(a).type = TYPE_NUMBER;
    return VAR(a);
}

ast_data ast_eq_rem(ast_data a, ast_data b)
{
    if (VAR(a).type == TYPE_STRING)
    {
        VAR(a).number = strtod(VAR(a).string, NULL);
    }
    if (b.type == TYPE_STRING)
    {
        b.number = strtod(b.string, NULL);
    }
    VAR(a).number = (double)((long)VAR(a).number % (long)b.number);
    VAR(a).type = TYPE_NUMBER;
    return VAR(a);
}

ast_data ast_eq_bit_and(ast_data a, ast_data b)
{
    if (VAR(a).type == TYPE_STRING)
    {
        VAR(a).number = strtod(VAR(a).string, NULL);
    }
    if (b.type == TYPE_STRING)
    {
        b.number = strtod(b.string, NULL);
    }
    VAR(a).number = (double)((long)VAR(a).number & (long)b.number);
    VAR(a).type = TYPE_NUMBER;
    return VAR(a);
}

ast_data ast_eq_bit_xor(ast_data a, ast_data b)
{
    if (VAR(a).type == TYPE_STRING)
    {
        VAR(a).number = strtod(VAR(a).string, NULL);
    }
    if (b.type == TYPE_STRING)
    {
        b.number = strtod(b.string, NULL);
    }
    VAR(a).number = (double)((long)VAR(a).number ^ (long)b.number);
    VAR(a).type = TYPE_NUMBER;
    return VAR(a);
}

ast_data ast_eq_bit_or(ast_data a, ast_data b)
{
    if (VAR(a).type == TYPE_STRING)
    {
        VAR(a).number = strtod(VAR(a).string, NULL);
    }
    if (b.type == TYPE_STRING)
    {
        b.number = strtod(b.string, NULL);
    }
    VAR(a).number = (double)((long)VAR(a).number | (long)b.number);
    VAR(a).type = TYPE_NUMBER;
    return VAR(a);
}

ast_data ast_eq_bit_lshift(ast_data a, ast_data b)
{
    if (VAR(a).type == TYPE_STRING)
    {
        VAR(a).number = strtod(VAR(a).string, NULL);
    }
    if (b.type == TYPE_STRING)
    {
        b.number = strtod(b.string, NULL);
    }
    VAR(a).number = (double)((long)VAR(a).number << (unsigned long)b.number);
    VAR(a).type = TYPE_NUMBER;
    return VAR(a);
}

ast_data ast_eq_bit_rshift(ast_data a, ast_data b)
{
    if (VAR(a).type == TYPE_STRING)
    {
        VAR(a).number = strtod(VAR(a).string, NULL);
    }
    if (b.type == TYPE_STRING)
    {
        b.number = strtod(b.string, NULL);
    }
    VAR(a).number = (double)((long)VAR(a).number >> (unsigned long)b.number);
    VAR(a).type = TYPE_NUMBER;
    return VAR(a);
}

///////////////////////////////////////////////////////////////////////////////
// Maths
///////////////////////////////////////////////////////////////////////////////

int ast_abs(int args)
{
    (void)args;

    ast_data *data = peek_data();

    if (data->type == TYPE_STRING)
    {
        data->number = strtod(data->string, NULL); 
    }
    data->number = fabs(data->number);
    data->type = TYPE_NUMBER;
    return 1;
}

int ast_ceil(int args)
{
    (void)args;

    ast_data *data = peek_data();

    if (data->type == TYPE_STRING)
    {
        data->number = strtod(data->string, NULL); 
    }
    data->number = ceil(data->number);
    data->type = TYPE_NUMBER;
    return 1;
}

int ast_cos(int args)
{
    (void)args;

    ast_data *data = peek_data();

    if (data->type == TYPE_STRING)
    {
        data->number = strtod(data->string, NULL); 
    }
    data->number = cos(data->number);
    data->type = TYPE_NUMBER;
    return 1;
}

int ast_cosh(int args)
{
    (void)args;

    ast_data *data = peek_data();

    if (data->type == TYPE_STRING)
    {
        data->number = strtod(data->string, NULL); 
    }
    data->number = cosh(data->number);
    data->type = TYPE_NUMBER;
    return 1;
}

int ast_exp(int args)
{
    (void)args;

    ast_data *data = peek_data();

    if (data->type == TYPE_STRING)
    {
        data->number = strtod(data->string, NULL); 
    }
    data->number = exp(data->number);
    data->type = TYPE_NUMBER;
    return 1;
}

int ast_floor(int args)
{
    (void)args;

    ast_data *data = peek_data();

    if (data->type == TYPE_STRING)
    {
        data->number = strtod(data->string, NULL); 
    }
    data->number = floor(data->number);
    data->type = TYPE_NUMBER;
    return 1;
}

int ast_log(int args)
{
    (void)args;

    ast_data *data = peek_data();

    if (data->type == TYPE_STRING)
    {
        data->number = strtod(data->string, NULL); 
    }
    data->number = log(data->number);
    data->type = TYPE_NUMBER;
    return 1;
}

int ast_log10(int args)
{
    (void)args;

    ast_data *data = peek_data();

    if (data->type == TYPE_STRING)
    {
        data->number = strtod(data->string, NULL); 
    }
    data->number = log10(data->number);
    data->type = TYPE_NUMBER;
    return 1;
}

int ast_pow(int args)
{
    (void)args;

    ast_data *a = sync_data(2);
    ast_data *b = a + 1;

    if (a->type == TYPE_STRING)
    {
        a->number = strtod(a->string, NULL); 
    }
    if (b->type == TYPE_STRING)
    {
        b->number = strtod(b->string, NULL); 
    }
    a->number = pow(a->number, b->number);
    a->type = TYPE_NUMBER;
    return 1;
}

int ast_rand(int args)
{
    (void)args;

    ast_data data;

    data.number = rand();
    data.type = TYPE_NUMBER;
    return push_data(data);
}

int ast_round(int args)
{
    (void)args;

    ast_data *data = peek_data();

    if (data->type == TYPE_STRING)
    {
        data->number = strtod(data->string, NULL); 
    }
    data->number = round(data->number);
    data->type = TYPE_NUMBER;
    return 1;
}

int ast_sin(int args)
{
    (void)args;

    ast_data *data = peek_data();

    if (data->type == TYPE_STRING)
    {
        data->number = strtod(data->string, NULL); 
    }
    data->number = sin(data->number);
    data->type = TYPE_NUMBER;
    return 1;
}

int ast_sinh(int args)
{
    (void)args;

    ast_data *data = peek_data();

    if (data->type == TYPE_STRING)
    {
        data->number = strtod(data->string, NULL); 
    }
    data->number = sinh(data->number);
    data->type = TYPE_NUMBER;
    return 1;
}

int ast_sqrt(int args)
{
    (void)args;

    ast_data *data = peek_data();

    if (data->type == TYPE_STRING)
    {
        data->number = strtod(data->string, NULL); 
    }
    data->number = sqrt(data->number);
    data->type = TYPE_NUMBER;
    return 1;
}

int ast_tan(int args)
{
    (void)args;

    ast_data *data = peek_data();

    if (data->type == TYPE_STRING)
    {
        data->number = strtod(data->string, NULL); 
    }
    data->number = tan(data->number);
    data->type = TYPE_NUMBER;
    return 1;
}

int ast_tanh(int args)
{
    (void)args;

    ast_data *data = peek_data();

    if (data->type == TYPE_STRING)
    {
        data->number = strtod(data->string, NULL); 
    }
    data->number = tanh(data->number);
    data->type = TYPE_NUMBER;
    return 1;
}

int ast_trunc(int args)
{
    (void)args;

    ast_data *data = peek_data();

    if (data->type == TYPE_STRING)
    {
        data->number = strtod(data->string, NULL); 
    }
    data->number = trunc(data->number);
    data->type = TYPE_NUMBER;
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
// Misc
///////////////////////////////////////////////////////////////////////////////

int ast_typeof(int args)
{
    (void)args;

    ast_data *data = peek_data();

    data->number = data->type;
    data->type = TYPE_NUMBER;
    return 1;
}

int ast_cond(int args)
{
    ast_data *data = sync_data(args);
    int offset = 1;

    switch (data->type)
    {
        case TYPE_STRING:
            offset += data->string[0] == '\0';
            break;
        default:
            offset += data->number == 0;
            break;
    }
    memcpy(data, data + offset, sizeof *data);
    return 1;
}

int ast_print(int args)
{
    ast_data *data = sync_data(args);
    int result = 0;

    for (int iter = 0; iter < args; iter++)
    {
        switch (data[iter].type)
        {
            case TYPE_BOOLEAN:
                result += printf("%s", data[iter].number ? "true" : "false");
                break;
            case TYPE_NUMBER:
                result += printf("%g", data[iter].number);
                break;
            case TYPE_STRING:
                result += printf("%s", data[iter].string);
                break;
            case TYPE_NULL:
                result += printf("null");
                break;
            default:
                printf("Can not print this value\n");
                return 0;
        }
    }
    result += printf("\n");
    data->number = result;
    data->type = TYPE_NUMBER;
    return 1;
}

