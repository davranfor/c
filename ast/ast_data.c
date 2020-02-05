#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "hashmap.h"
#include "ast_data.h"
#include "ast_eval.h"

///////////////////////////////////////////////////////////////////////////////

#define MAX_DATA 16384

struct stack
{
    ast_data data[MAX_DATA];
    int count;
};

static struct stack stack;

void wind_data(ast_data *data, int vars)
{
    if (vars == 0)
    {
        return;
    }
    if (stack.count + vars > MAX_DATA)
    {
        fprintf(stderr, "Stack overflow1\n");
        exit(EXIT_FAILURE);
    }
    memcpy(stack.data + stack.count,
           data,
           sizeof(*data) * (size_t)vars
    );
    stack.count += vars;
}

void unwind_data(ast_data *data, int vars)
{
    if (vars == 0)
    {
        return;
    }
    if (stack.count - vars < 0)
    {
        fprintf(stderr, "Stack underflow\n");
        exit(EXIT_FAILURE);
    }
    memcpy(data,
           stack.data + (stack.count - vars),
           sizeof(*data) * (size_t)vars
    );
    stack.count -= vars;
}

int push_data(ast_data data)
{
    if (stack.count == MAX_DATA)
    {
        fprintf(stderr, "Stack overflow\n");
        exit(EXIT_FAILURE);
    }
    stack.data[stack.count++] = data;
    return 1;
}

ast_data pop_data(void)
{
    if (stack.count == 0)
    {
        fprintf(stderr, "Stack underflow\n");
        exit(EXIT_FAILURE);
    }
    return stack.data[--stack.count];    
}

ast_data *peek_data(void)
{
    if (stack.count == 0)
    {
        fprintf(stderr, "Empty stack\n");
        exit(EXIT_FAILURE);
    }
    return &stack.data[stack.count - 1];    
}

ast_data *sync_data(int count)
{
    if ((count == 0) || (count > stack.count))
    {
        fprintf(stderr, "Stack underflow\n");
        exit(EXIT_FAILURE);
    }
    stack.count -= count - 1;
    return &stack.data[stack.count - 1];    
}

///////////////////////////////////////////////////////////////////////////////

static ast_data *new_data(ast_type type)
{
    ast_data *data;

    data = malloc(sizeof *data);
    if (data == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    data->type = type;
    data->flags = 0;
    return data;
}

///////////////////////////////////////////////////////////////////////////////

int is_sequence(int c)
{
    return (c == '\\') || (c == '/') || (c == '"') ||
           (c == 'b')  || (c == 'f') || (c == 'n') || (c == 'r') || (c == 't');
}

int get_sequence(int c)
{
    switch (c)
    {
        case 'b':
            return '\b';
        case 'f':
            return '\f';
        case 'n':
            return '\n';
        case 'r':
            return '\r';
        case 't':
            return '\t';
        // case '"':
        // case '\\':
        // case '/':
        default:
            return c;
    }
}

int valid_name(const char *str)
{
    // First character must be '_'  or [a ... Z]
    if ((*str != '_') && !isalpha((unsigned char)*str))
    {
        return 0;
    }
    str++;
    // The rest can also have digits
    while (*str != '\0')
    {
        if ((*str != '_') && !isalnum((unsigned char)*str))
        {
            return 0;
        }
        str++;
    }
    return 1;
}

///////////////////////////////////////////////////////////////////////////////

#define DEF_OPERATOR(o, ...) [o] =                      \
{                                                       \
    .type = TYPE_OPERATOR,                              \
    .flags = 0,                                         \
    .operator = &(const ast_operator){o, __VA_ARGS__}   \
}

static ast_data operators[] =
{
    DEF_OPERATOR(OPERATOR_PLUS,          1, 13, 'R', "+",   ast_plus),
    DEF_OPERATOR(OPERATOR_MINUS,         1, 13, 'R', "-",   ast_minus),
    DEF_OPERATOR(OPERATOR_NOT,           1, 13, 'R', "!",   ast_not),
    DEF_OPERATOR(OPERATOR_MUL,           2, 12, 'L', "*",   ast_mul),
    DEF_OPERATOR(OPERATOR_DIV,           2, 12, 'L', "/",   ast_div),
    DEF_OPERATOR(OPERATOR_REM,           2, 12, 'L', "%",   ast_rem),
    DEF_OPERATOR(OPERATOR_ADD,           2, 11, 'L', "+",   ast_add),
    DEF_OPERATOR(OPERATOR_SUB,           2, 11, 'L', "-",   ast_sub),
    DEF_OPERATOR(OPERATOR_BIT_LSHIFT,    2, 10, 'L', "<<",  ast_bit_lshift),
    DEF_OPERATOR(OPERATOR_BIT_RSHIFT,    2, 10, 'L', ">>",  ast_bit_rshift),
    DEF_OPERATOR(OPERATOR_LT,            2,  9, 'L', "<",   ast_lt),
    DEF_OPERATOR(OPERATOR_GT,            2,  9, 'L', ">",   ast_gt),
    DEF_OPERATOR(OPERATOR_LT_OR_EQ,      2,  9, 'L', "<=",  ast_lt_or_eq),
    DEF_OPERATOR(OPERATOR_GT_OR_EQ,      2,  9, 'L', ">=",  ast_gt_or_eq),
    DEF_OPERATOR(OPERATOR_IS_EQ,         2,  8, 'L', "==",  ast_is_eq),
    DEF_OPERATOR(OPERATOR_NOT_EQ,        2,  8, 'L', "!=",  ast_not_eq),
    DEF_OPERATOR(OPERATOR_BIT_AND,       2,  7, 'L', "&",   ast_bit_and),
    DEF_OPERATOR(OPERATOR_BIT_XOR,       2,  6, 'L', "^",   ast_bit_xor),
    DEF_OPERATOR(OPERATOR_BIT_OR,        2,  5, 'L', "|",   ast_bit_or),
    DEF_OPERATOR(OPERATOR_AND,           2,  4, 'L', "&&",  ast_and),
    DEF_OPERATOR(OPERATOR_XOR,           2,  3, 'L', "^^",  ast_xor),
    DEF_OPERATOR(OPERATOR_OR,            2,  2, 'L', "||",  ast_or),
    DEF_OPERATOR(OPERATOR_EQ,            2,  1, 'R', "=",   ast_eq),
    DEF_OPERATOR(OPERATOR_EQ_ADD,        2,  1, 'R', "+=",  ast_eq_add),
    DEF_OPERATOR(OPERATOR_EQ_SUB,        2,  1, 'R', "-=",  ast_eq_sub),
    DEF_OPERATOR(OPERATOR_EQ_MUL,        2,  1, 'R', "*=",  ast_eq_mul),
    DEF_OPERATOR(OPERATOR_EQ_DIV,        2,  1, 'R', "/=",  ast_eq_div),
    DEF_OPERATOR(OPERATOR_EQ_REM,        2,  1, 'R', "%=",  ast_eq_rem),
    DEF_OPERATOR(OPERATOR_EQ_BIT_AND,    2,  1, 'R', "&=",  ast_eq_bit_and),
    DEF_OPERATOR(OPERATOR_EQ_BIT_XOR,    2,  1, 'R', "^=",  ast_eq_bit_xor),
    DEF_OPERATOR(OPERATOR_EQ_BIT_OR,     2,  1, 'R', "|=",  ast_eq_bit_or),
    DEF_OPERATOR(OPERATOR_EQ_BIT_LSHIFT, 2,  1, 'R', "<<=", ast_eq_bit_lshift),
    DEF_OPERATOR(OPERATOR_EQ_BIT_RSHIFT, 2,  1, 'R', ">>=", ast_eq_bit_rshift),

    DEF_OPERATOR(OPERATOR_LPARENTHS,     0,  0, 'L', "(",   NULL),
    DEF_OPERATOR(OPERATOR_RPARENTHS,     0,  0, 'L', ")" ,  NULL),
    DEF_OPERATOR(OPERATOR_COMMA,         0,  0, 'L', ",",   NULL),
    DEF_OPERATOR(OPERATOR_SEMICOLON,     0,  0, 'L', ";",   NULL),
    DEF_OPERATOR(OPERATOR_EOF,           0,  0, 'L', "",    NULL),
};

int is_assignment(int operator)
{
    switch (operator)
    {
        case OPERATOR_EQ:
        case OPERATOR_EQ_ADD:
        case OPERATOR_EQ_SUB:
        case OPERATOR_EQ_MUL:
        case OPERATOR_EQ_DIV:
        case OPERATOR_EQ_REM:
        case OPERATOR_EQ_BIT_AND:
        case OPERATOR_EQ_BIT_XOR:
        case OPERATOR_EQ_BIT_OR:
        case OPERATOR_EQ_BIT_LSHIFT:
        case OPERATOR_EQ_BIT_RSHIFT:
            return 1;
        default:
            return 0;
    }
}

int is_operator(int operator)
{
    switch (operator)
    {
        case OPERATOR_NOT:
        case OPERATOR_MUL:
        case OPERATOR_DIV:
        case OPERATOR_REM:
        case OPERATOR_ADD:
        case OPERATOR_SUB:
        case OPERATOR_LT:
        case OPERATOR_GT:
        case OPERATOR_BIT_AND:
        case OPERATOR_BIT_XOR:
        case OPERATOR_BIT_OR:
        case OPERATOR_EQ:
        case OPERATOR_LPARENTHS:
        case OPERATOR_RPARENTHS:
        case OPERATOR_COMMA:
        case OPERATOR_SEMICOLON:
            return 1;
        default:
            return 0;
    }
}

static int get_operator(const char **operator)
{
    int result = 0;

    switch (**operator)
    {
        case OPERATOR_LPARENTHS:
        case OPERATOR_RPARENTHS:
        case OPERATOR_COMMA:
        case OPERATOR_SEMICOLON:
            result = **operator;
            (*operator)++;
            break;
        case OPERATOR_NOT:
        case OPERATOR_MUL:
        case OPERATOR_DIV:
        case OPERATOR_REM:
        case OPERATOR_ADD:
        case OPERATOR_SUB:
            result = **operator;
            (*operator)++;
            if (**operator == '=')
            {
                result ^= 'z';
                (*operator)++;
            }
            break;
        case OPERATOR_BIT_AND:
        case OPERATOR_BIT_XOR:
        case OPERATOR_BIT_OR:
        case OPERATOR_EQ:
            result = **operator;
            (*operator)++;
            if (**operator == '=')
            {
                result ^= 'z';
                (*operator)++;
            }
            else if (**operator == result)
            {
                result ^= '!';
                (*operator)++;
            }
            break;
        case OPERATOR_LT:
        case OPERATOR_GT:
            result = **operator;
            (*operator)++;
            if (**operator == '=')
            {
                result ^= 'z';
                (*operator)++;
            }
            else if (**operator == result)
            {
                result ^= '!';
                (*operator)++;
                if (**operator == '=')
                {
                    result ^= '=';
                    (*operator)++;
                }
            }
            break;
        default:
            break;
    }
    return result;
}

ast_data *map_operator(const char **operator)
{
    return &operators[get_operator(operator)];
}

int arguments(const ast_data *data)
{
    return data->operator->args;
}

int precedence(const ast_data *pa, const ast_data *pb)
{
    int a = pa->operator->precedence;
    int b = pb->operator->precedence;

    if (a == 0)
    {
        return 0;
    }
    if (a == b)
    {
        return pa->operator->associativity == 'L';
    }
    return a > b;
}

ast_data *unary(ast_data *data)
{
    switch (data->operator->key)
    {
        case OPERATOR_ADD:
            return &operators[OPERATOR_PLUS];
        case OPERATOR_SUB:
            return &operators[OPERATOR_MINUS];
    }
    return data;
}

///////////////////////////////////////////////////////////////////////////////

int is_iterator(const ast_data *data)
{
    switch (data->statement->key)
    {
        case STATEMENT_WHILE:
        case STATEMENT_FOR:
            return 1;
        default:
            return 0;
    }
}

#define DEF_STATEMENT(...)                              \
{                                                       \
    .type = TYPE_STATEMENT,                             \
    .flags = 0,                                         \
    .statement = &(const ast_statement){__VA_ARGS__}    \
}

static ast_data statements[] =
{
    DEF_STATEMENT(STATEMENT_NONE,     0, ""),
    DEF_STATEMENT(STATEMENT_DEF,      0, "def"),
    DEF_STATEMENT(STATEMENT_END,      0, "end"),
    DEF_STATEMENT(STATEMENT_IF,       1, "if"),
    DEF_STATEMENT(STATEMENT_ELIF,     1, "elif"),
    DEF_STATEMENT(STATEMENT_ELSE,     0, "else"),
    DEF_STATEMENT(STATEMENT_WHILE,    1, "while"),
    DEF_STATEMENT(STATEMENT_FOR,      3, "for"),
    DEF_STATEMENT(STATEMENT_CONTINUE, 0, "continue"),
    DEF_STATEMENT(STATEMENT_BREAK,    0, "break"),
    DEF_STATEMENT(STATEMENT_RETURN,   0, "return"),
};

static ast_data branches[] =
{
    DEF_STATEMENT(STATEMENT_IFEL,     0, "if"),
    DEF_STATEMENT(STATEMENT_THEN,     0, "then"),
};

static ast_function *data_def;

ast_data *map_statement(const char *name)
{
    const size_t count = sizeof statements / sizeof *statements;

    for (size_t iter = 1; iter < count; iter++)
    {
        if (strcmp(statements[iter].statement->name, name) == 0)
        {
            return &statements[iter];
        }
    }
    return NULL;
}

ast_data *map_branch(int branch)
{
    return &branches[branch];
}

///////////////////////////////////////////////////////////////////////////////

static ast_data *new_function(void)
{
    ast_data *data = new_data(TYPE_FUNCTION);
    ast_function *function;

    function = calloc(1, sizeof *function);
    if (function == NULL)
    {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    data->function = function;
    return data;
}

static int comp_function(const void *pa, const void *pb)
{
    const ast_data *a = pa;
    const ast_data *b = pb;

    return strcmp(a->function->name, b->function->name);
}

static unsigned long hash_function(const void *item)
{
    const ast_data *data = item;

    return hash_str((const unsigned char *)data->function->name);
}

static void free_function(void *data)
{
    free(((ast_data *)data)->function);
    free(data);
}

static hashmap *functions;
static ast_data *data_fnc;

static ast_data *map_function(const char *name)
{
    ast_data *data;

    data_fnc->function->name = name;
    data = hashmap_insert(functions, data_fnc);
    if (data == NULL)
    {
        perror("hashmap_insert");
        exit(EXIT_FAILURE);
    }
    if (data == data_fnc)
    {
        data_fnc = new_function();
    }
    if (data_def == NULL)
    {
        data_def = data->function;
    }
    return data;
}

static void map_functions(void)
{
    functions = hashmap_create(comp_function, hash_function, 1000);
    if (functions == NULL)
    {
        perror("hashmap_create");
        exit(EXIT_FAILURE);
    }
    data_fnc = new_function();
}

static void unmap_functions(void)
{
    hashmap_destroy(functions, free_function);
    free_function(data_fnc);
}

///////////////////////////////////////////////////////////////////////////////

static int comp_callable(const void *pa, const void *pb)
{
    const ast_data *a = pa;
    const ast_data *b = pb;

    return strcmp(a->callable->name, b->callable->name);
}

static unsigned long hash_callable(const void *item)
{
    const ast_data *data = item;

    return hash_str((const unsigned char *)data->callable->name);
}

static hashmap *callables;

ast_data *map_callable(const char *name)
{
    const ast_callable callable = {.name = name};
    ast_data temp = {.callable = &callable};
    ast_data *data;

    data = hashmap_search(callables, &temp);
    if (data == NULL)
    {
        return map_function(name);
    }
    return data;
}

#define DEF_CALLABLE(...)                           \
{                                                   \
    .type = TYPE_CALLABLE,                          \
    .flags = 0,                                     \
    .callable = &(const ast_callable){__VA_ARGS__}  \
}

static ast_data callable_list[] =
{
    DEF_CALLABLE("abs",    {1,  1}, ast_abs),
    DEF_CALLABLE("ceil",   {1,  1}, ast_ceil),
    DEF_CALLABLE("cos",    {1,  1}, ast_cos),
    DEF_CALLABLE("cosh",   {1,  1}, ast_cosh),
    DEF_CALLABLE("exp",    {1,  1}, ast_exp),
    DEF_CALLABLE("floor",  {1,  1}, ast_floor),
    DEF_CALLABLE("log",    {1,  1}, ast_log),
    DEF_CALLABLE("log10",  {1,  1}, ast_log10),
    DEF_CALLABLE("pow",    {2,  2}, ast_pow),
    DEF_CALLABLE("rand",   {0,  0}, ast_rand),
    DEF_CALLABLE("round",  {1,  1}, ast_round),
    DEF_CALLABLE("sin",    {1,  1}, ast_sin),
    DEF_CALLABLE("sinh",   {1,  1}, ast_sinh),
    DEF_CALLABLE("sqrt",   {1,  1}, ast_sqrt),
    DEF_CALLABLE("tan",    {1,  1}, ast_tan),
    DEF_CALLABLE("tanh",   {1,  1}, ast_tanh),
    DEF_CALLABLE("trunc",  {1,  1}, ast_trunc),

    DEF_CALLABLE("typeof", {1,  1}, ast_typeof),    
    DEF_CALLABLE("cond",   {3,  3}, ast_cond),
    DEF_CALLABLE("print",  {1, 64}, ast_print),
};

static void map_callables(void)
{
    const size_t count = sizeof callable_list / sizeof *callable_list;

    callables = hashmap_create(comp_callable, hash_callable, count * 4);
    if (callables == NULL)
    {
        perror("hashmap_create");
        exit(EXIT_FAILURE);
    }
    for (size_t iter = 0; iter < count; iter++)
    {
        if (hashmap_insert(callables, &callable_list[iter]) == NULL)
        {
            perror("hashmap_insert");
            exit(EXIT_FAILURE);
        }
    }
}

static void unmap_callables(void)
{
    hashmap_destroy(callables, NULL);
}

///////////////////////////////////////////////////////////////////////////////

static ast_data *new_variable(void)
{
    ast_data *data = new_data(TYPE_VARIABLE);
    ast_variable *variable;

    variable = malloc(sizeof *variable);
    if (variable == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    data->variable = variable;
    return data;
}

static int comp_variable(const void *pa, const void *pb)
{
    const ast_data *a = pa;
    const ast_data *b = pb;

    return (a->variable->function != b->variable->function) ||
           strcmp(a->variable->name, b->variable->name);
}

static unsigned long hash_variable(const void *item)
{
    const ast_data *data = item;

    return hash_str((const unsigned char *)data->variable->name) ^
           hash_str((const unsigned char *)data->variable->function->name);
}

static void free_variable(void *data)
{
    free(((ast_data *)data)->variable);
    free(data);
}

static hashmap *variables;
static ast_data *data_var;

ast_data *map_variable(const char *name)
{
    if (data_def == NULL)
    {
        return NULL;
    }
    data_var->variable->name = name;
    data_var->variable->function = data_def;

    ast_data *data;

    data = hashmap_insert(variables, data_var);
    if (data == NULL)
    {
        perror("hashmap_insert");
        exit(EXIT_FAILURE);
    }
    if (data == data_var)
    {
        data->variable->offset = data_def->vars++;
        data_var = new_variable();
    }
    return data;
}

static void map_variables(void)
{
    variables = hashmap_create(comp_variable, hash_variable, 1000);
    if (variables == NULL)
    {
        perror("hashmap_create");
        exit(EXIT_FAILURE);
    }
    data_var = new_variable();
}

static void unmap_variables(void)
{
    hashmap_destroy(variables, free_variable);
    free_variable(data_var);
}

void map_vars()
{
    if (data_def->vars == 0)
    {
        return;
    }
    data_def->data = calloc((size_t)data_def->vars, sizeof *data_def->data);
    if (data_def->data == NULL)
    {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    data_def = NULL;
}

///////////////////////////////////////////////////////////////////////////////

ast_data *map_boolean(const char *str)
{
    static ast_data booleans[] =
    {
        {.type = TYPE_BOOLEAN, .flags = 0, .number = 0},
        {.type = TYPE_BOOLEAN, .flags = 0, .number = 1},
    };

    if (strcmp(str, "true") == 0)
    {
        return &booleans[1];
    }
    else if (strcmp(str, "false") == 0)
    {
        return &booleans[0];
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

#define DEF_NUMBER(n)       \
{                           \
    .type = TYPE_NUMBER,    \
    .flags = 0,             \
    .number = n             \
}

static ast_data numbers[] =
{
    DEF_NUMBER(  0), DEF_NUMBER(  1), DEF_NUMBER(  2), DEF_NUMBER(  3),
    DEF_NUMBER(  4), DEF_NUMBER(  5), DEF_NUMBER(  6), DEF_NUMBER(  7),
    DEF_NUMBER(  8), DEF_NUMBER(  9), DEF_NUMBER( 10), DEF_NUMBER( 11),
    DEF_NUMBER( 12), DEF_NUMBER( 13), DEF_NUMBER( 14), DEF_NUMBER( 15),
    DEF_NUMBER( 16), DEF_NUMBER( 17), DEF_NUMBER( 18), DEF_NUMBER( 19),
    DEF_NUMBER( 20), DEF_NUMBER( 21), DEF_NUMBER( 22), DEF_NUMBER( 23),
    DEF_NUMBER( 24), DEF_NUMBER( 25), DEF_NUMBER( 26), DEF_NUMBER( 27),
    DEF_NUMBER( 28), DEF_NUMBER( 29), DEF_NUMBER( 30), DEF_NUMBER( 31),
    DEF_NUMBER( 32), DEF_NUMBER( 33), DEF_NUMBER( 34), DEF_NUMBER( 35),
    DEF_NUMBER( 36), DEF_NUMBER( 37), DEF_NUMBER( 38), DEF_NUMBER( 39),
    DEF_NUMBER( 40), DEF_NUMBER( 41), DEF_NUMBER( 42), DEF_NUMBER( 43),
    DEF_NUMBER( 44), DEF_NUMBER( 45), DEF_NUMBER( 46), DEF_NUMBER( 47),
    DEF_NUMBER( 48), DEF_NUMBER( 49), DEF_NUMBER( 50), DEF_NUMBER( 51),
    DEF_NUMBER( 52), DEF_NUMBER( 53), DEF_NUMBER( 54), DEF_NUMBER( 55),
    DEF_NUMBER( 56), DEF_NUMBER( 57), DEF_NUMBER( 58), DEF_NUMBER( 59),
    DEF_NUMBER( 60), DEF_NUMBER( 61), DEF_NUMBER( 62), DEF_NUMBER( 63),
    DEF_NUMBER( 64), DEF_NUMBER( 65), DEF_NUMBER( 66), DEF_NUMBER( 67),
    DEF_NUMBER( 68), DEF_NUMBER( 69), DEF_NUMBER( 70), DEF_NUMBER( 71),
    DEF_NUMBER( 72), DEF_NUMBER( 73), DEF_NUMBER( 74), DEF_NUMBER( 75),
    DEF_NUMBER( 76), DEF_NUMBER( 77), DEF_NUMBER( 78), DEF_NUMBER( 79),
    DEF_NUMBER( 80), DEF_NUMBER( 81), DEF_NUMBER( 82), DEF_NUMBER( 83),
    DEF_NUMBER( 84), DEF_NUMBER( 85), DEF_NUMBER( 86), DEF_NUMBER( 87),
    DEF_NUMBER( 88), DEF_NUMBER( 89), DEF_NUMBER( 90), DEF_NUMBER( 91),
    DEF_NUMBER( 92), DEF_NUMBER( 93), DEF_NUMBER( 94), DEF_NUMBER( 95),
    DEF_NUMBER( 96), DEF_NUMBER( 97), DEF_NUMBER( 98), DEF_NUMBER( 99),
    DEF_NUMBER(100), DEF_NUMBER(101), DEF_NUMBER(102), DEF_NUMBER(103),
    DEF_NUMBER(104), DEF_NUMBER(105), DEF_NUMBER(106), DEF_NUMBER(107),
    DEF_NUMBER(108), DEF_NUMBER(109), DEF_NUMBER(110), DEF_NUMBER(111),
    DEF_NUMBER(112), DEF_NUMBER(113), DEF_NUMBER(114), DEF_NUMBER(115),
    DEF_NUMBER(116), DEF_NUMBER(117), DEF_NUMBER(118), DEF_NUMBER(119),
    DEF_NUMBER(120), DEF_NUMBER(121), DEF_NUMBER(122), DEF_NUMBER(123),
    DEF_NUMBER(124), DEF_NUMBER(125), DEF_NUMBER(126), DEF_NUMBER(127),
    DEF_NUMBER(128), DEF_NUMBER(129), DEF_NUMBER(130), DEF_NUMBER(131),
    DEF_NUMBER(132), DEF_NUMBER(133), DEF_NUMBER(134), DEF_NUMBER(135),
    DEF_NUMBER(136), DEF_NUMBER(137), DEF_NUMBER(138), DEF_NUMBER(139),
    DEF_NUMBER(140), DEF_NUMBER(141), DEF_NUMBER(142), DEF_NUMBER(143),
    DEF_NUMBER(144), DEF_NUMBER(145), DEF_NUMBER(146), DEF_NUMBER(147),
    DEF_NUMBER(148), DEF_NUMBER(149), DEF_NUMBER(150), DEF_NUMBER(151),
    DEF_NUMBER(152), DEF_NUMBER(153), DEF_NUMBER(154), DEF_NUMBER(155),
    DEF_NUMBER(156), DEF_NUMBER(157), DEF_NUMBER(158), DEF_NUMBER(159),
    DEF_NUMBER(160), DEF_NUMBER(161), DEF_NUMBER(162), DEF_NUMBER(163),
    DEF_NUMBER(164), DEF_NUMBER(165), DEF_NUMBER(166), DEF_NUMBER(167),
    DEF_NUMBER(168), DEF_NUMBER(169), DEF_NUMBER(170), DEF_NUMBER(171),
    DEF_NUMBER(172), DEF_NUMBER(173), DEF_NUMBER(174), DEF_NUMBER(175),
    DEF_NUMBER(176), DEF_NUMBER(177), DEF_NUMBER(178), DEF_NUMBER(179),
    DEF_NUMBER(180), DEF_NUMBER(181), DEF_NUMBER(182), DEF_NUMBER(183),
    DEF_NUMBER(184), DEF_NUMBER(185), DEF_NUMBER(186), DEF_NUMBER(187),
    DEF_NUMBER(188), DEF_NUMBER(189), DEF_NUMBER(190), DEF_NUMBER(191),
    DEF_NUMBER(192), DEF_NUMBER(193), DEF_NUMBER(194), DEF_NUMBER(195),
    DEF_NUMBER(196), DEF_NUMBER(197), DEF_NUMBER(198), DEF_NUMBER(199),
    DEF_NUMBER(200), DEF_NUMBER(201), DEF_NUMBER(202), DEF_NUMBER(203),
    DEF_NUMBER(204), DEF_NUMBER(205), DEF_NUMBER(206), DEF_NUMBER(207),
    DEF_NUMBER(208), DEF_NUMBER(209), DEF_NUMBER(210), DEF_NUMBER(211),
    DEF_NUMBER(212), DEF_NUMBER(213), DEF_NUMBER(214), DEF_NUMBER(215),
    DEF_NUMBER(216), DEF_NUMBER(217), DEF_NUMBER(218), DEF_NUMBER(219),
    DEF_NUMBER(220), DEF_NUMBER(221), DEF_NUMBER(222), DEF_NUMBER(223),
    DEF_NUMBER(224), DEF_NUMBER(225), DEF_NUMBER(226), DEF_NUMBER(227),
    DEF_NUMBER(228), DEF_NUMBER(229), DEF_NUMBER(230), DEF_NUMBER(231),
    DEF_NUMBER(232), DEF_NUMBER(233), DEF_NUMBER(234), DEF_NUMBER(235),
    DEF_NUMBER(236), DEF_NUMBER(237), DEF_NUMBER(238), DEF_NUMBER(239),
    DEF_NUMBER(240), DEF_NUMBER(241), DEF_NUMBER(242), DEF_NUMBER(243),
    DEF_NUMBER(244), DEF_NUMBER(245), DEF_NUMBER(246), DEF_NUMBER(247),
    DEF_NUMBER(248), DEF_NUMBER(249), DEF_NUMBER(250), DEF_NUMBER(251),
    DEF_NUMBER(252), DEF_NUMBER(253), DEF_NUMBER(254), DEF_NUMBER(255),
};

ast_data *map_number(const char *str)
{
    ast_data *data = NULL;
    double number;
    char *ptr;

    number = strtod(str, &ptr);
    if (*ptr == '\0')
    {
        if ((number >= 0) && (number <= 255) && (number == (int)number))
        {
            data = &numbers[(int)number];
        }
        else
        {
            data = new_data(TYPE_NUMBER);
            data->number = number;
        }
    }
    return data;
}

///////////////////////////////////////////////////////////////////////////////

static int comp_string(const void *pa, const void *pb)
{
    const ast_data *a = pa;
    const ast_data *b = pb;

    return strcmp(a->string, b->string);
}

static unsigned long hash_string(const void *item)
{
    const ast_data *data = item;

    return hash_str((const unsigned char *)data->string);
}

static hashmap *strings;
static ast_data *data_str;

ast_data *map_string(const char *str)
{
    ast_data *data;

    data_str->string = str;
    data = hashmap_insert(strings, data_str);
    if (data == NULL)
    {
        perror("hashmap_insert");
        exit(EXIT_FAILURE);
    }
    if (data == data_str)
    {
        data_str = new_data(TYPE_STRING);
    }
    return data;
}

static void map_strings(void)
{
    strings = hashmap_create(comp_string, hash_string, 1000);
    if (strings == NULL)
    {
        perror("hashmap_create");
        exit(EXIT_FAILURE);
    }
    data_str = new_data(TYPE_STRING);
}

static void unmap_strings(void)
{
    hashmap_destroy(strings, free);
    free(data_str);
}

///////////////////////////////////////////////////////////////////////////////

ast_data *map_null(const char *str)
{
    static ast_data null = {.type = TYPE_NULL, .flags = 0, .number = 0};

    if (strcmp(str, "null") == 0)
    {
        return &null;
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

void map_data(void)
{
    map_functions();
    map_callables();
    map_variables();
    map_strings();
}

void unmap_data(void)
{
    unmap_functions();
    unmap_callables();
    unmap_variables();
    unmap_strings();
}

void free_data(ast_data *data)
{
    switch (data->type)
    {
        case TYPE_FUNCTION:
            if (data->function->data != NULL)
            {
                free(data->function->data);
                data->function->data = NULL;
            }
            break;
        case TYPE_NUMBER:
            if ((data < numbers) ||
                (data > numbers + 255))
            {
                free(data);
            }
            break;
        default:
            break;
    }
}

