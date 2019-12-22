#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "hashmap.h"
#include "ast_eval.h"
#include "ast_data.h"

///////////////////////////////////////////////////////////////////////////////

ast_data *new_data(ast_type type)
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

///////////////////////////////////////////////////////////////////////////////

static ast_data operators[] =
{
    [ OPERATOR_END            ] = { .type = TYPE_OPERATOR, .operator = &(const ast_operator){ OPERATOR_END,            0, 0, 'L', ""        } },
    [ OPERATOR_PLUS           ] = { .type = TYPE_OPERATOR, .operator = &(const ast_operator){ OPERATOR_PLUS,           1, 5, 'R', "+ Unary" } },
    [ OPERATOR_MINUS          ] = { .type = TYPE_OPERATOR, .operator = &(const ast_operator){ OPERATOR_MINUS,          1, 5, 'R', "- Unary" } },
    [ OPERATOR_EXP            ] = { .type = TYPE_OPERATOR, .operator = &(const ast_operator){ OPERATOR_EXP,            2, 4, 'R', "^"       } },
    [ OPERATOR_MUL            ] = { .type = TYPE_OPERATOR, .operator = &(const ast_operator){ OPERATOR_MUL,            2, 3, 'L', "*"       } },
    [ OPERATOR_DIV            ] = { .type = TYPE_OPERATOR, .operator = &(const ast_operator){ OPERATOR_DIV,            2, 3, 'L', "/"       } },
    [ OPERATOR_REM            ] = { .type = TYPE_OPERATOR, .operator = &(const ast_operator){ OPERATOR_REM,            2, 3, 'L', "%"       } },
    [ OPERATOR_ADD            ] = { .type = TYPE_OPERATOR, .operator = &(const ast_operator){ OPERATOR_ADD,            2, 2, 'L', "+"       } },
    [ OPERATOR_SUB            ] = { .type = TYPE_OPERATOR, .operator = &(const ast_operator){ OPERATOR_SUB,            2, 2, 'L', "-"       } },
    [ OPERATOR_EQ             ] = { .type = TYPE_OPERATOR, .operator = &(const ast_operator){ OPERATOR_EQ,             2, 1, 'R', "="       } },
    [ OPERATOR_LEFT_PARENTHS  ] = { .type = TYPE_OPERATOR, .operator = &(const ast_operator){ OPERATOR_LEFT_PARENTHS,  0, 0, 'L', "("       } },
    [ OPERATOR_RIGHT_PARENTHS ] = { .type = TYPE_OPERATOR, .operator = &(const ast_operator){ OPERATOR_RIGHT_PARENTHS, 0, 0, 'L', ")"       } },
    [ OPERATOR_COMMA          ] = { .type = TYPE_OPERATOR, .operator = &(const ast_operator){ OPERATOR_COMMA,          0, 0, 'L', ","       } },
    [ OPERATOR_SEMICOLON      ] = { .type = TYPE_OPERATOR, .operator = &(const ast_operator){ OPERATOR_SEMICOLON,      0, 0, 'L', ";"       } },
};

ast_data *map_operator(int value)
{
    return &operators[value];
}

int arguments(const ast_data *data)
{
    return data->operator->arguments;
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

int is_token(int c)
{
    return
        (c == OPERATOR_EXP) ||
        (c == OPERATOR_MUL) ||
        (c == OPERATOR_DIV) ||
        (c == OPERATOR_REM) ||
        (c == OPERATOR_ADD) ||
        (c == OPERATOR_SUB) ||
        (c == OPERATOR_EQ) ||
        (c == OPERATOR_LEFT_PARENTHS) ||
        (c == OPERATOR_RIGHT_PARENTHS) ||
        (c == OPERATOR_COMMA) ||
        (c == OPERATOR_SEMICOLON);
}

///////////////////////////////////////////////////////////////////////////////

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

int is_valid_name(const char *str)
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

static const ast_call statements[] =
{
    { "if",      1, 1, NULL },
    { "elif",    1, 1, NULL },
    { "while",   1, 1, NULL },
    { "for",     1, 1, NULL },
    { "foreach", 1, 1, NULL },
};

static const ast_call functions[] =
{
    { "abs",     1, 1, ast_abs     },
    { "ceil",    1, 1, ast_ceil    },
    { "cos",     1, 1, ast_cos     },
    { "cosh",    1, 1, ast_cosh    },
    { "exp",     1, 1, ast_exp     },
    { "floor",   1, 1, ast_floor   },
    { "log",     1, 1, ast_log     },
    { "log10",   1, 1, ast_log10   },
    { "pow",     2, 1, ast_pow     },
    { "rand",    0, 1, ast_rand    },
    { "round",   1, 1, ast_round   },
    { "sin",     1, 1, ast_sin     },
    { "sinh",    1, 1, ast_sinh    },
    { "sqr",     1, 1, ast_sqr     },
    { "tan",     1, 1, ast_tan     },
    { "tanh",    1, 1, ast_tanh    },
    { "trunc",   1, 1, ast_trunc   },

    { "print",   1, 1, ast_print   },
};

static ast_data calls
[
    2 * (
        (sizeof statements / sizeof *statements)
        +
        (sizeof functions / sizeof *functions)
    )
];

static hashmap *map;

ast_data *map_call(const char *name)
{
    const ast_call call = {.name = name};
    ast_data data = {.call = &call};

    return hashmap_search(map, &data);
}

static int comp_call(const void *pa, const void *pb)
{
    const ast_data *a = pa;
    const ast_data *b = pb;

    return strcmp(a->call->name, b->call->name);
}

static unsigned long hash_call(const void *item)
{
    const ast_data *data = item;

    return hash_string((const unsigned char *)data->call->name);
}

void map_calls(void)
{
    size_t n = sizeof calls / sizeof *calls;

    map = hashmap_create(comp_call, hash_call, n * 2);
    if (map == NULL)
    {
        perror("hashmap_create");
        exit(EXIT_FAILURE);
    }

    size_t count;
    size_t iter;

    count = sizeof statements / sizeof *statements;
    for (iter = 0, n = 0; iter < count; iter++, n += 2)
    {
        calls[n + 0].type = TYPE_CALL;
        calls[n + 0].call = &statements[iter];
        calls[n + 1].type = TYPE_STATEMENT;
        calls[n + 1].call = &statements[iter];
        if (hashmap_insert(map, &calls[n]) == NULL)
        {
            perror("hashmap_insert");
            exit(EXIT_FAILURE);
        }
    }
    count = sizeof functions / sizeof *functions;
    for (iter = 0; iter < count; iter++, n += 2)
    {
        calls[n + 0].type = TYPE_CALL;
        calls[n + 0].call = &functions[iter];
        calls[n + 1].type = TYPE_FUNCTION;
        calls[n + 1].call = &functions[iter];
        if (hashmap_insert(map, &calls[n]) == NULL)
        {
            perror("hashmap_insert");
            exit(EXIT_FAILURE);
        }
    }
}

void unmap_calls(void)
{
    hashmap_destroy(map, NULL);
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

void print_help(void)
{
    size_t count;

    count = sizeof operators / sizeof *operators;
    puts("Operators:");
    for (size_t iter = 1; iter < count; iter++)
    {
        if (operators[iter].type == 0)
        {
            continue;
        }
        printf("   %-10s\tPrecedence: %d\n",
            operators[iter].operator->text,
            operators[iter].operator->precedence
        );
    }
    count = sizeof functions / sizeof *functions;
    puts("Functions:");
    for (size_t iter = 0; iter < count; iter++)
    {
        printf("   %-10s\tArguments: %d\n",
            functions[iter].name,
            functions[iter].arguments
        );
    }
    puts("Options:");
    puts("   --help\tDisplay this information");
    puts("   --tree\tDisplay an abstract syntax tree");
}

