#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ast_eval.h"

#define MAX_FRAME 2048

static double frame[MAX_FRAME];
static int counter;

int push_number(double number)
{
    if (counter == MAX_FRAME)
    {
        fprintf(stderr, "Stack overflow\n");
        exit(EXIT_FAILURE);
    }
    frame[counter++] = number;
    return 1;
}

double pop_number(void)
{
    if (counter == 0)
    {
        fprintf(stderr, "Stack underflow\n");
        exit(EXIT_FAILURE);
    }
    return frame[--counter];    
}

int ast_abs(void)
{
    double number = pop_number();

    return push_number(fabs(number));
}

int ast_ceil(void)
{
    double number = pop_number();

    return push_number(ceil(number));
}

int ast_cos(void)
{
    double number = pop_number();

    return push_number(cos(number));
}

int ast_cosh(void)
{
    double number = pop_number();

    return push_number(cosh(number));
}

int ast_exp(void)
{
    double number = pop_number();

    return push_number(exp(number));
}

int ast_floor(void)
{
    double number = pop_number();

    return push_number(floor(number));
}

int ast_log(void)
{
    double number = pop_number();

    return push_number(log(number));
}

int ast_log10(void)
{
    double number = pop_number();

    return push_number(log10(number));
}

int ast_pow(void)
{
    double b = pop_number();
    double a = pop_number();

    return push_number(pow(a, b));
}

int ast_rand(void)
{
    return push_number(rand());
}

int ast_round(void)
{
    double number = pop_number();

    return push_number(round(number));
}

int ast_sin(void)
{
    double number = pop_number();

    return push_number(sin(number));
}

int ast_sinh(void)
{
    double number = pop_number();

    return push_number(sinh(number));
}

int ast_sqr(void)
{
    double number = pop_number();

    return push_number(sqrt(number));
}

int ast_tan(void)
{
    double number = pop_number();

    return push_number(tan(number));
}

int ast_tanh(void)
{
    double number = pop_number();

    return push_number(tanh(number));
}

int ast_trunc(void)
{
    double number = pop_number();

    return push_number(trunc(number));
}

