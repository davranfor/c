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
    [ OPERATOR_PLUS           ] = { .type = TYPE_OPERATOR, .operator = &(const ast_operator){ OPERATOR_PLUS,           1, 4, 'R', "+ Unary" } },
    [ OPERATOR_MINUS          ] = { .type = TYPE_OPERATOR, .operator = &(const ast_operator){ OPERATOR_MINUS,          1, 4, 'R', "- Unary" } },
    [ OPERATOR_EXP            ] = { .type = TYPE_OPERATOR, .operator = &(const ast_operator){ OPERATOR_EXP,            2, 3, 'R', "^"       } },
    [ OPERATOR_MUL            ] = { .type = TYPE_OPERATOR, .operator = &(const ast_operator){ OPERATOR_MUL,            2, 2, 'L', "*"       } },
    [ OPERATOR_DIV            ] = { .type = TYPE_OPERATOR, .operator = &(const ast_operator){ OPERATOR_DIV,            2, 2, 'L', "/"       } },
    [ OPERATOR_REM            ] = { .type = TYPE_OPERATOR, .operator = &(const ast_operator){ OPERATOR_REM,            2, 2, 'L', "%"       } },
    [ OPERATOR_ADD            ] = { .type = TYPE_OPERATOR, .operator = &(const ast_operator){ OPERATOR_ADD,            2, 1, 'L', "+"       } },
    [ OPERATOR_SUB            ] = { .type = TYPE_OPERATOR, .operator = &(const ast_operator){ OPERATOR_SUB,            2, 1, 'L', "-"       } },
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
    if (isdigit((unsigned char)*str))
    {
        return 0;
    }
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

static const ast_function list[] =
{
    { "abs",   1, ast_abs   },
    { "ceil",  1, ast_ceil  },
    { "cos",   1, ast_cos   },
    { "cosh",  1, ast_cosh  },
    { "exp",   1, ast_exp   },
    { "floor", 1, ast_floor },
    { "log",   1, ast_log   },
    { "log10", 1, ast_log10 },
    { "pow",   2, ast_pow   },
    { "rand",  0, ast_rand  },
    { "round", 1, ast_round },
    { "sin",   1, ast_sin   },
    { "sinh",  1, ast_sinh  },
    { "sqr",   1, ast_sqr   },
    { "tan",   1, ast_tan   },
    { "tanh",  1, ast_tanh  },
    { "trunc", 1, ast_trunc },
};

static ast_data functions[sizeof list / sizeof *list * 2];
static hashmap *map;

ast_data *map_function(const char *name)
{
    const ast_function function = {.name = name};
    ast_data data = {.function = &function};
    ast_data *item;

    item = hashmap_search(map, &data);
    if (item == NULL)
    {
        fprintf(stderr, "function \"%s\" not found\n", name);
    }
    return item;
}

static int comp(const void *pa, const void *pb)
{
    const ast_data *a = pa;
    const ast_data *b = pb;

    return strcmp(a->function->name, b->function->name);
}

static unsigned long hash(const void *item)
{
    const ast_data *data = item;

    return hash_string((const unsigned char *)data->function->name);
}

int map_functions(void)
{
    size_t count = sizeof functions / sizeof *functions;

    map = hashmap_create(comp, hash, count * 2);
    if (map == NULL)
    {
        return 0;
    }

    for (size_t iter = 0; iter < count; iter += 2)
    {
        functions[iter + 0].type = CLASSIFY_FUNCTION;
        functions[iter + 0].function = &list[iter / 2];
        functions[iter + 1].type = TYPE_FUNCTION;
        functions[iter + 1].function = &list[iter / 2];
        if (hashmap_insert(map, &functions[iter]) == NULL)
        {
            hashmap_destroy(map, NULL);
            return 0;
        }
    }
    return 1;
}

void unmap_functions(void)
{
    hashmap_destroy(map, NULL);
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
    count = sizeof list / sizeof *list;
    puts("Functions:");
    for (size_t iter = 0; iter < count; iter++)
    {
        printf("   %-10s\tArguments: %d\n",
            list[iter].name,
            list[iter].arguments
        );
    }
    puts("Options:");
    puts("   --help\tDisplay this information");
    puts("   --tree\tDisplay an abstract syntax tree");
}

