#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "hashmap.h"
#include "ast_data.h"
#include "ast_eval.h"

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
    return data;
}

int is_sequence(int c)
{
    return (c == '\\') || (c == '/') || (c == '"') ||
           (c == 'b')  || (c == 'f') || (c == 'n') || (c == 'r') || (c == 't');
}

int get_sequence(int sequence)
{
    switch (sequence)
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
            return sequence;
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

ast_type call_type(const ast_data *data)
{
    if ((data + 1)->type == TYPE_FUNCTION)
    {
        return TYPE_FUNCTION;
    }
    else
    {
        return TYPE_STATEMENT;
    }
}

///////////////////////////////////////////////////////////////////////////////

#define DEF_OPERATOR(o, ...)                                 \
    [o] = {                                                  \
        .type = TYPE_OPERATOR,                               \
        .operator = &(const ast_operator){o, __VA_ARGS__}    \
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
    switch (data->operator->value)
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
    switch (data->statement->value)
    {
        case STATEMENT_WHILE:
        case STATEMENT_UNTIL:
        case STATEMENT_FOR:
        case STATEMENT_FOREACH:
            return 1;
        default:
            return 0;
    }
}

static const ast_statement statements[] =
{
    { "",         0, 0  },
    { "if",       1, 1  },
    { "elif",     1, 2  },
    { "else",     0, 3  },
    { "while",    1, 4  },
    { "until",    1, 5  },
    { "for",      3, 6  },
    { "foreach",  1, 7  },
    { "continue", 0, 8  },
    { "break",    0, 9  },
    { "end",      0, 10 },
};

static const ast_statement branches[] =
{
    { "then",     0, 11 },
    { "if",       1, 12 }, // IFEL
};

ast_data *map_branch(int branch)
{
    static ast_data list[] =
    {
        {.type = TYPE_STATEMENT, .statement = &branches[0]},
        {.type = TYPE_STATEMENT, .statement = &branches[1]},
    };

    return &list[branch];
}

static ast_data statement_list[(sizeof statements / sizeof *statements) * 3];

ast_data *map_statement(const char *name)
{
    size_t count = sizeof statements / sizeof *statements;

    for (size_t iter = 0; iter < count; iter++)
    {
        if (strcmp(statements[iter].name, name) == 0)
        {
            return &statement_list[iter * 3];
        }
    }
    return NULL;
}

void map_statements(void)
{
    size_t count = sizeof statements / sizeof *statements;

    for (size_t iter = 0, items = 0; iter < count; iter += 1, items += 3)
    {
        statement_list[items + 0].type = TYPE_CALL;
        statement_list[items + 0].statement = &statements[iter];
        statement_list[items + 1].type = TYPE_COMPOUND;
        statement_list[items + 1].statement = &statements[iter];
        statement_list[items + 2].type = TYPE_STATEMENT;
        statement_list[items + 2].statement = &statements[iter];
    }
}

///////////////////////////////////////////////////////////////////////////////

static hashmap *functions;

ast_data *map_function(const char *name)
{
    const ast_function function = {.name = name};
    ast_data data = {.function = &function};

    return hashmap_search(functions, &data);
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

    return hash_string((const unsigned char *)data->function->name);
}

void map_functions(void)
{
    static const ast_function list[] =
    {
        { "abs",     {1,  1}, ast_abs     },
        { "ceil",    {1,  1}, ast_ceil    },
        { "cos",     {1,  1}, ast_cos     },
        { "cosh",    {1,  1}, ast_cosh    },
        { "exp",     {1,  1}, ast_exp     },
        { "floor",   {1,  1}, ast_floor   },
        { "log",     {1,  1}, ast_log     },
        { "log10",   {1,  1}, ast_log10   },
        { "pow",     {2,  2}, ast_pow     },
        { "rand",    {0,  0}, ast_rand    },
        { "round",   {1,  1}, ast_round   },
        { "sin",     {1,  1}, ast_sin     },
        { "sinh",    {1,  1}, ast_sinh    },
        { "sqr",     {1,  1}, ast_sqr     },
        { "tan",     {1,  1}, ast_tan     },
        { "tanh",    {1,  1}, ast_tanh    },
        { "trunc",   {1,  1}, ast_trunc   },

        { "print",   {1, 64}, ast_print   },
        { "println", {1, 64}, ast_println },
    };
    static ast_data function_list[(sizeof list / sizeof *list) * 2];
    size_t count = sizeof list / sizeof *list;

    functions = hashmap_create(comp_function, hash_function, count * 4);
    if (functions == NULL)
    {
        perror("hashmap_create");
        exit(EXIT_FAILURE);
    }
    for (size_t iter = 0, items = 0; iter < count; iter += 1, items += 2)
    {
        function_list[items + 0].type = TYPE_CALL;
        function_list[items + 0].function = &list[iter];
        function_list[items + 1].type = TYPE_FUNCTION;
        function_list[items + 1].function = &list[iter];
        if (hashmap_insert(functions, &function_list[items]) == NULL)
        {
            perror("hashmap_insert");
            exit(EXIT_FAILURE);
        }
    }
}

void unmap_functions(void)
{
    hashmap_destroy(functions, NULL);
}

///////////////////////////////////////////////////////////////////////////////

static hashmap *variables;
static ast_data *data_var;

static ast_data *new_variable(void)
{
    ast_data *data;

    data = new_data(TYPE_VARIABLE);

    ast_variable *variable;

    variable = malloc(sizeof *variable);
    if (variable == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    variable->data.type = TYPE_NUMBER;
    variable->data.number = 0;
    data->variable = variable;
    return data;
}

ast_data *map_variable(const char *name)
{
    ast_data *data;

    data_var->variable->name = name;
    data = hashmap_insert(variables, data_var);
    if (data == NULL)
    {
        perror("hashmap_insert");
        exit(EXIT_FAILURE);
    }
    if (data == data_var)
    {
        data_var = new_variable();
    }
    return data;
}

static int comp_variable(const void *pa, const void *pb)
{
    const ast_data *a = pa;
    const ast_data *b = pb;

    return strcmp(a->variable->name, b->variable->name);
}

static unsigned long hash_variable(const void *item)
{
    const ast_data *data = item;

    return hash_string((const unsigned char *)data->variable->name);
}

static void free_variable(void *data)
{
    free(((ast_data *)data)->variable);
    free(data);
}

void map_variables(void)
{
    variables = hashmap_create(comp_variable, hash_variable, 1000);
    if (variables == NULL)
    {
        perror("hashmap_create");
        exit(EXIT_FAILURE);
    }
    data_var = new_variable();
}

void unmap_variables(void)
{
    hashmap_destroy(variables, free_variable);
    free_variable(data_var);
}

///////////////////////////////////////////////////////////////////////////////

ast_data *map_number(const char *str)
{
    ast_data *data = NULL;
    double number;
    char *ptr;

    number = strtod(str, &ptr);
    if (*ptr == '\0')
    {
        data = new_data(TYPE_NUMBER);
        data->number = number;
    }
    return data;
}

ast_data *map_string(const char *str)
{
    ast_data *data = new_data(TYPE_STRING);

    data->string = str;
    return data;
}

ast_data *map_boolean(const char *str)
{
    static ast_data list[] =
    {
        { TYPE_BOOLEAN, .number = 1 },
        { TYPE_BOOLEAN, .number = 0 },
    };

    if (strcmp(str, "true") == 0)
    {
        return &list[0];
    }
    else if (strcmp(str, "false") == 0)
    {
        return &list[1];
    }
    return NULL;
}

