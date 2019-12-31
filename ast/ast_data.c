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
    return (data + 1)->type;
}

///////////////////////////////////////////////////////////////////////////////

static ast_data operators[] =
{
    [ OPERATOR_EOF            ] = { .type = TYPE_OPERATOR, .operator = &(const ast_operator){ OPERATOR_EOF,            0, 0, 'L', ""        } },
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
    [ OPERATOR_END            ] = { .type = TYPE_OPERATOR, .operator = &(const ast_operator){ OPERATOR_END,            0, 0, 'L', "@"       } },
};

int is_operator(int c)
{
    switch (c)
    {
        case OPERATOR_EXP:
        case OPERATOR_MUL:
        case OPERATOR_DIV:
        case OPERATOR_REM:
        case OPERATOR_ADD:
        case OPERATOR_SUB:
        case OPERATOR_EQ:
        case OPERATOR_LEFT_PARENTHS:
        case OPERATOR_RIGHT_PARENTHS:
        case OPERATOR_COMMA:
        case OPERATOR_SEMICOLON:
        case OPERATOR_END:
            return 1;
        default:
            return 0;

    }
}

ast_data *map_operator(int value)
{
    return &operators[value];
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

static const ast_call statements[] =
{
    { "if",      1, 0, NULL },
    { "elif",    1, 0, NULL },
    { "else",    0, 0, NULL },
    { "while",   1, 0, NULL },
    { "for",     1, 0, NULL },
    { "foreach", 1, 0, NULL },
    { "break",   0, 0, NULL },
    { "end",     0, 0, NULL },
};

int is_statement(const char *name)
{
    size_t count = sizeof statements / sizeof *statements;

    for (size_t iter = 0; iter < count; iter++)
    {
        if (strcmp(statements[iter].name, name) == 0)
        {
            return 1;
        }
    }
    return 0;
}

static ast_data statement_calls[(sizeof statements / sizeof *statements) * 3];

ast_data *map_statement(const char *name)
{
    size_t count = sizeof statements / sizeof *statements;

    for (size_t iter = 0; iter < count; iter++)
    {
        if (strcmp(statements[iter].name, name) == 0)
        {
            return &statement_calls[iter * 3];
        }
    }
    return NULL;
}

void map_statements(void)
{
    size_t count = sizeof statements / sizeof *statements;

    for (size_t iter = 0, rows = 0; iter < count; iter += 1, rows += 3)
    {
        statement_calls[rows + 0].type = TYPE_CALL;
        statement_calls[rows + 0].call = &statements[iter];
        statement_calls[rows + 1].type = TYPE_COMPOUND;
        statement_calls[rows + 1].call = &statements[iter];
        statement_calls[rows + 2].type = TYPE_STATEMENT;
        statement_calls[rows + 2].call = &statements[iter];
    }
}

///////////////////////////////////////////////////////////////////////////////

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

static hashmap *map;

ast_data *map_function(const char *name)
{
    const ast_call call = {.name = name};
    ast_data data = {.call = &call};

    return hashmap_search(map, &data);
}

static int comp_function(const void *pa, const void *pb)
{
    const ast_data *a = pa;
    const ast_data *b = pb;

    return strcmp(a->call->name, b->call->name);
}

static unsigned long hash_function(const void *item)
{
    const ast_data *data = item;

    return hash_string((const unsigned char *)data->call->name);
}

static ast_data function_calls[(sizeof functions / sizeof *functions) * 2];

void map_functions(void)
{
    size_t count = sizeof functions / sizeof *functions;

    map = hashmap_create(comp_function, hash_function, count * 4);
    if (map == NULL)
    {
        perror("hashmap_create");
        exit(EXIT_FAILURE);
    }
    for (size_t iter = 0, rows = 0; iter < count; iter += 1, rows += 2)
    {
        function_calls[rows + 0].type = TYPE_CALL;
        function_calls[rows + 0].call = &functions[iter];
        function_calls[rows + 1].type = TYPE_FUNCTION;
        function_calls[rows + 1].call = &functions[iter];
        if (hashmap_insert(map, &function_calls[rows]) == NULL)
        {
            perror("hashmap_insert");
            exit(EXIT_FAILURE);
        }
    }
}

void unmap_functions(void)
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
            functions[iter].args
        );
    }
    puts("Options:");
    puts("   --help\tDisplay this information");
    puts("   --tree\tDisplay an abstract syntax tree");
}

