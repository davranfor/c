#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ast_data.h"
#include "ast_eval.h"

///////////////////////////////////////////////////////////////////////////////

#define EVAL_VAR(v) (v.variable->function->vars[v.variable->offset])

///////////////////////////////////////////////////////////////////////////////

static void conv_number(ast_data *data)
{
    if (data->type == TYPE_STRING)
    {
        data->number = strtod(data->string, NULL); 
    }
    data->type = TYPE_NUMBER;
}

static void conv_boolean(ast_data *data)
{
    if (data->type == TYPE_STRING)
    {
        data->number = strtod(data->string, NULL); 
    }
    data->type = TYPE_BOOLEAN;
}

static void cast_number(ast_data *data)
{
    if (data->type == TYPE_STRING)
    {
        data->number = strtod(data->string, NULL); 
    }
}

///////////////////////////////////////////////////////////////////////////////
// Operators
///////////////////////////////////////////////////////////////////////////////

ast_data ast_plus(ast_data a, ast_data b)
{
    (void)b;
    conv_number(&a);
    return a;
}

ast_data ast_minus(ast_data a, ast_data b)
{
    (void)b;
    conv_number(&a);
    a.number = -a.number;
    return a;
}

ast_data ast_not(ast_data a, ast_data b)
{
    (void)b;
    conv_boolean(&a);
    a.number = a.number == 0;
    return a;
}

ast_data ast_mul(ast_data a, ast_data b)
{
    conv_number(&a);
    cast_number(&b);
    a.number = a.number * b.number;
    return a;
}

ast_data ast_div(ast_data a, ast_data b)
{
    conv_number(&a);
    cast_number(&b);
    a.number = a.number / b.number;
    return a;
}

ast_data ast_rem(ast_data a, ast_data b)
{
    conv_number(&a);
    cast_number(&b);
    a.number = (double)((long)a.number % (long)b.number);
    return a;
}

ast_data ast_add(ast_data a, ast_data b)
{
    conv_number(&a);
    cast_number(&b);
    a.number = a.number + b.number;
    return a;
}

ast_data ast_sub(ast_data a, ast_data b)
{
    conv_number(&a);
    cast_number(&b);
    a.number = a.number - b.number;
    return a;
}

ast_data ast_bit_lshift(ast_data a, ast_data b)
{
    conv_number(&a);
    cast_number(&b);
    a.number = (double)((unsigned long)a.number << (unsigned long)b.number);
    return a;
}

ast_data ast_bit_rshift(ast_data a, ast_data b)
{
    conv_number(&a);
    cast_number(&b);
    a.number = (double)((unsigned long)a.number >> (unsigned long)b.number);
    return a;
}

ast_data ast_lt(ast_data a, ast_data b)
{
    conv_boolean(&a);
    cast_number(&b);
    a.number = a.number < b.number;
    return a;
}

ast_data ast_gt(ast_data a, ast_data b)
{
    conv_boolean(&a);
    cast_number(&b);
    a.number = a.number > b.number;
    return a;
}

ast_data ast_lt_or_eq(ast_data a, ast_data b)
{
    conv_boolean(&a);
    cast_number(&b);
    a.number = a.number <= b.number;
    return a;
}

ast_data ast_gt_or_eq(ast_data a, ast_data b)
{
    conv_boolean(&a);
    cast_number(&b);
    a.number = a.number >= b.number;
    return a;
}

ast_data ast_is_eq(ast_data a, ast_data b)
{
    conv_boolean(&a);
    cast_number(&b);
    a.number = a.number == b.number;
    return a;
}

ast_data ast_not_eq(ast_data a, ast_data b)
{
    conv_boolean(&a);
    cast_number(&b);
    a.number = a.number != b.number;
    return a;
}

ast_data ast_bit_and(ast_data a, ast_data b)
{
    conv_number(&a);
    cast_number(&b);
    a.number = (double)((unsigned long)a.number & (unsigned long)b.number);
    return a;
}

ast_data ast_bit_xor(ast_data a, ast_data b)
{
    conv_number(&a);
    cast_number(&b);
    a.number = (double)((unsigned long)a.number ^ (unsigned long)b.number);
    return a;
}

ast_data ast_bit_or(ast_data a, ast_data b)
{
    conv_number(&a);
    cast_number(&b);
    a.number = (double)((unsigned long)a.number | (unsigned long)b.number);
    return a;
}

ast_data ast_and(ast_data a, ast_data b)
{
    conv_boolean(&a);
    cast_number(&b);
    a.number = a.number && b.number;
    return a;
}

ast_data ast_xor(ast_data a, ast_data b)
{
    conv_boolean(&a);
    cast_number(&b);
    a.number = (a.number || b.number) && !(a.number && b.number);
    return a;
}

ast_data ast_or(ast_data a, ast_data b)
{
    conv_boolean(&a);
    cast_number(&b);
    a.number = a.number || b.number;
    return a;
}

ast_data ast_eq(ast_data a, ast_data b)
{
    EVAL_VAR(a) = b;
    return b;
}

ast_data ast_eq_add(ast_data a, ast_data b)
{
    conv_number(&EVAL_VAR(a));
    cast_number(&b);
    EVAL_VAR(a).number += b.number;
    return EVAL_VAR(a);
}

ast_data ast_eq_sub(ast_data a, ast_data b)
{
    conv_number(&EVAL_VAR(a));
    cast_number(&b);
    EVAL_VAR(a).number -= b.number;
    return EVAL_VAR(a);
}

ast_data ast_eq_mul(ast_data a, ast_data b)
{
    conv_number(&EVAL_VAR(a));
    cast_number(&b);
    EVAL_VAR(a).number *= b.number;
    return EVAL_VAR(a);
}

ast_data ast_eq_div(ast_data a, ast_data b)
{
    conv_number(&EVAL_VAR(a));
    cast_number(&b);
    EVAL_VAR(a).number /= b.number;
    return EVAL_VAR(a);
}

ast_data ast_eq_rem(ast_data a, ast_data b)
{
    conv_number(&EVAL_VAR(a));
    cast_number(&b);
    EVAL_VAR(a).number = (double)((long)EVAL_VAR(a).number % (long)b.number);
    return EVAL_VAR(a);
}

ast_data ast_eq_bit_and(ast_data a, ast_data b)
{
    conv_number(&EVAL_VAR(a));
    cast_number(&b);
    EVAL_VAR(a).number = (double)((unsigned long)EVAL_VAR(a).number & (unsigned long)b.number);
    return EVAL_VAR(a);
}

ast_data ast_eq_bit_xor(ast_data a, ast_data b)
{
    conv_number(&EVAL_VAR(a));
    cast_number(&b);
    EVAL_VAR(a).number = (double)((unsigned long)EVAL_VAR(a).number ^ (unsigned long)b.number);
    return EVAL_VAR(a);
}

ast_data ast_eq_bit_or(ast_data a, ast_data b)
{
    conv_number(&EVAL_VAR(a));
    cast_number(&b);
    EVAL_VAR(a).number = (double)((unsigned long)EVAL_VAR(a).number | (unsigned long)b.number);
    return EVAL_VAR(a);
}

ast_data ast_eq_bit_lshift(ast_data a, ast_data b)
{
    conv_number(&EVAL_VAR(a));
    cast_number(&b);
    EVAL_VAR(a).number = (double)((unsigned long)EVAL_VAR(a).number << (unsigned long)b.number);
    return EVAL_VAR(a);
}

ast_data ast_eq_bit_rshift(ast_data a, ast_data b)
{
    conv_number(&EVAL_VAR(a));
    cast_number(&b);
    EVAL_VAR(a).number = (double)((unsigned long)EVAL_VAR(a).number >> (unsigned long)b.number);
    return EVAL_VAR(a);
}

///////////////////////////////////////////////////////////////////////////////
// Maths
///////////////////////////////////////////////////////////////////////////////

int ast_abs(int args)
{
    (void)args;

    ast_data *data = peek_data();

    conv_number(data);
    data->number = fabs(data->number);
    return 1;
}

int ast_ceil(int args)
{
    (void)args;

    ast_data *data = peek_data();

    conv_number(data);
    data->number = ceil(data->number);
    return 1;
}

int ast_cos(int args)
{
    (void)args;

    ast_data *data = peek_data();

    conv_number(data);
    data->number = cos(data->number);
    return 1;
}

int ast_cosh(int args)
{
    (void)args;

    ast_data *data = peek_data();

    conv_number(data);
    data->number = cosh(data->number);
    return 1;
}

int ast_exp(int args)
{
    (void)args;

    ast_data *data = peek_data();

    conv_number(data);
    data->number = exp(data->number);
    return 1;
}

int ast_floor(int args)
{
    (void)args;

    ast_data *data = peek_data();

    conv_number(data);
    data->number = floor(data->number);
    return 1;
}

int ast_log(int args)
{
    (void)args;

    ast_data *data = peek_data();

    conv_number(data);
    data->number = log(data->number);
    return 1;
}

int ast_log10(int args)
{
    (void)args;

    ast_data *data = peek_data();

    conv_number(data);
    data->number = log10(data->number);
    return 1;
}

int ast_pow(int args)
{
    (void)args;

    ast_data b = pop_data();
    ast_data *a = peek_data();

    conv_number(a);
    cast_number(&b);
    a->number = pow(a->number, b.number);
    return 1;
}

int ast_rand(int args)
{
    (void)args;

    ast_data data;

    data.type = TYPE_NUMBER;
    data.number = rand();
    return push_data(data);
}

int ast_round(int args)
{
    (void)args;

    ast_data *data = peek_data();

    conv_number(data);
    data->number = round(data->number);
    return 1;
}

int ast_sin(int args)
{
    (void)args;

    ast_data *data = peek_data();

    conv_number(data);
    data->number = sin(data->number);
    return 1;
}

int ast_sinh(int args)
{
    (void)args;

    ast_data *data = peek_data();

    conv_number(data);
    data->number = sinh(data->number);
    return 1;
}

int ast_sqrt(int args)
{
    (void)args;

    ast_data *data = peek_data();

    conv_number(data);
    data->number = sqrt(data->number);
    return 1;
}

int ast_tan(int args)
{
    (void)args;

    ast_data *data = peek_data();

    conv_number(data);
    data->number = tan(data->number);
    return 1;
}

int ast_tanh(int args)
{
    (void)args;

    ast_data *data = peek_data();

    conv_number(data);
    data->number = tanh(data->number);
    return 1;
}

int ast_trunc(int args)
{
    (void)args;

    ast_data *data = peek_data();

    conv_number(data);
    data->number = trunc(data->number);
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
    data->type = TYPE_NUMBER;
    data->number = result;
    return 1;
}

