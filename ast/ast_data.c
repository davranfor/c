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

ast_data *peep_data(int count)
{
    if ((count == 0) || (count > stack.count))
    {
        fprintf(stderr, "Stack underflow\n");
        exit(EXIT_FAILURE);
    }
    stack.count -= count;
    return &stack.data[stack.count];    
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

#define DEF_OPERATOR(o, ...)                                \
    [o] = {                                                 \
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
        case STATEMENT_UNTIL:
        case STATEMENT_FOR:
        case STATEMENT_FOREACH:
            return 1;
        default:
            return 0;
    }
}

#define DEF_STATEMENT(...)                                  \
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
    DEF_STATEMENT(STATEMENT_UNTIL,    1, "until"),
    DEF_STATEMENT(STATEMENT_FOR,      3, "for"),
    DEF_STATEMENT(STATEMENT_FOREACH,  2, "foreach"),
    DEF_STATEMENT(STATEMENT_CONTINUE, 0, "continue"),
    DEF_STATEMENT(STATEMENT_BREAK,    0, "break"),
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

    function = malloc(sizeof *function);
    if (function == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    function->node = NULL;
    function->vars = NULL;
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

#define DEF_CALLABLE(...)                               \
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
static int nvars;

ast_data *map_variable(const char *name)
{
    if (data_def == NULL)
    {
        return NULL;
    }

    ast_data *data;

    data_var->variable->name = name;
    data_var->variable->function = data_def;
    data = hashmap_insert(variables, data_var);
    if (data == NULL)
    {
        perror("hashmap_insert");
        exit(EXIT_FAILURE);
    }
    if (data == data_var)
    {
        data->variable->offset = nvars++;
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
    if (nvars == 0)
    {
        return;
    }
    data_def->vars = calloc((size_t)nvars, sizeof *data_def->vars);
    if (data_def->vars == NULL)
    {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    data_def = NULL;
    nvars = 0;
}

///////////////////////////////////////////////////////////////////////////////

ast_data *map_boolean(const char *str)
{
    static ast_data booleans[] =
    {
        {TYPE_BOOLEAN, 0, .number = 0},
        {TYPE_BOOLEAN, 0, .number = 1},
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

static ast_data numbers[] =
{
    {TYPE_NUMBER, 0, .number =    0}, {TYPE_NUMBER, 0, .number =    1},
    {TYPE_NUMBER, 0, .number =    2}, {TYPE_NUMBER, 0, .number =    3},
    {TYPE_NUMBER, 0, .number =    4}, {TYPE_NUMBER, 0, .number =    5},
    {TYPE_NUMBER, 0, .number =    6}, {TYPE_NUMBER, 0, .number =    7},
    {TYPE_NUMBER, 0, .number =    8}, {TYPE_NUMBER, 0, .number =    9},
    {TYPE_NUMBER, 0, .number =   10}, {TYPE_NUMBER, 0, .number =   11},
    {TYPE_NUMBER, 0, .number =   12}, {TYPE_NUMBER, 0, .number =   13},
    {TYPE_NUMBER, 0, .number =   14}, {TYPE_NUMBER, 0, .number =   15},
    {TYPE_NUMBER, 0, .number =   16}, {TYPE_NUMBER, 0, .number =   17},
    {TYPE_NUMBER, 0, .number =   18}, {TYPE_NUMBER, 0, .number =   19},
    {TYPE_NUMBER, 0, .number =   20}, {TYPE_NUMBER, 0, .number =   21},
    {TYPE_NUMBER, 0, .number =   22}, {TYPE_NUMBER, 0, .number =   23},
    {TYPE_NUMBER, 0, .number =   24}, {TYPE_NUMBER, 0, .number =   25},
    {TYPE_NUMBER, 0, .number =   26}, {TYPE_NUMBER, 0, .number =   27},
    {TYPE_NUMBER, 0, .number =   28}, {TYPE_NUMBER, 0, .number =   29},
    {TYPE_NUMBER, 0, .number =   30}, {TYPE_NUMBER, 0, .number =   31},
    {TYPE_NUMBER, 0, .number =   32}, {TYPE_NUMBER, 0, .number =   33},
    {TYPE_NUMBER, 0, .number =   34}, {TYPE_NUMBER, 0, .number =   35},
    {TYPE_NUMBER, 0, .number =   36}, {TYPE_NUMBER, 0, .number =   37},
    {TYPE_NUMBER, 0, .number =   38}, {TYPE_NUMBER, 0, .number =   39},
    {TYPE_NUMBER, 0, .number =   40}, {TYPE_NUMBER, 0, .number =   41},
    {TYPE_NUMBER, 0, .number =   42}, {TYPE_NUMBER, 0, .number =   43},
    {TYPE_NUMBER, 0, .number =   44}, {TYPE_NUMBER, 0, .number =   45},
    {TYPE_NUMBER, 0, .number =   46}, {TYPE_NUMBER, 0, .number =   47},
    {TYPE_NUMBER, 0, .number =   48}, {TYPE_NUMBER, 0, .number =   49},
    {TYPE_NUMBER, 0, .number =   50}, {TYPE_NUMBER, 0, .number =   51},
    {TYPE_NUMBER, 0, .number =   52}, {TYPE_NUMBER, 0, .number =   53},
    {TYPE_NUMBER, 0, .number =   54}, {TYPE_NUMBER, 0, .number =   55},
    {TYPE_NUMBER, 0, .number =   56}, {TYPE_NUMBER, 0, .number =   57},
    {TYPE_NUMBER, 0, .number =   58}, {TYPE_NUMBER, 0, .number =   59},
    {TYPE_NUMBER, 0, .number =   60}, {TYPE_NUMBER, 0, .number =   61},
    {TYPE_NUMBER, 0, .number =   62}, {TYPE_NUMBER, 0, .number =   63},
    {TYPE_NUMBER, 0, .number =   64}, {TYPE_NUMBER, 0, .number =   65},
    {TYPE_NUMBER, 0, .number =   66}, {TYPE_NUMBER, 0, .number =   67},
    {TYPE_NUMBER, 0, .number =   68}, {TYPE_NUMBER, 0, .number =   69},
    {TYPE_NUMBER, 0, .number =   70}, {TYPE_NUMBER, 0, .number =   71},
    {TYPE_NUMBER, 0, .number =   72}, {TYPE_NUMBER, 0, .number =   73},
    {TYPE_NUMBER, 0, .number =   74}, {TYPE_NUMBER, 0, .number =   75},
    {TYPE_NUMBER, 0, .number =   76}, {TYPE_NUMBER, 0, .number =   77},
    {TYPE_NUMBER, 0, .number =   78}, {TYPE_NUMBER, 0, .number =   79},
    {TYPE_NUMBER, 0, .number =   80}, {TYPE_NUMBER, 0, .number =   81},
    {TYPE_NUMBER, 0, .number =   82}, {TYPE_NUMBER, 0, .number =   83},
    {TYPE_NUMBER, 0, .number =   84}, {TYPE_NUMBER, 0, .number =   85},
    {TYPE_NUMBER, 0, .number =   86}, {TYPE_NUMBER, 0, .number =   87},
    {TYPE_NUMBER, 0, .number =   88}, {TYPE_NUMBER, 0, .number =   89},
    {TYPE_NUMBER, 0, .number =   90}, {TYPE_NUMBER, 0, .number =   91},
    {TYPE_NUMBER, 0, .number =   92}, {TYPE_NUMBER, 0, .number =   93},
    {TYPE_NUMBER, 0, .number =   94}, {TYPE_NUMBER, 0, .number =   95},
    {TYPE_NUMBER, 0, .number =   96}, {TYPE_NUMBER, 0, .number =   97},
    {TYPE_NUMBER, 0, .number =   98}, {TYPE_NUMBER, 0, .number =   99},
    {TYPE_NUMBER, 0, .number =  100}, {TYPE_NUMBER, 0, .number =  101},
    {TYPE_NUMBER, 0, .number =  102}, {TYPE_NUMBER, 0, .number =  103},
    {TYPE_NUMBER, 0, .number =  104}, {TYPE_NUMBER, 0, .number =  105},
    {TYPE_NUMBER, 0, .number =  106}, {TYPE_NUMBER, 0, .number =  107},
    {TYPE_NUMBER, 0, .number =  108}, {TYPE_NUMBER, 0, .number =  109},
    {TYPE_NUMBER, 0, .number =  110}, {TYPE_NUMBER, 0, .number =  111},
    {TYPE_NUMBER, 0, .number =  112}, {TYPE_NUMBER, 0, .number =  113},
    {TYPE_NUMBER, 0, .number =  114}, {TYPE_NUMBER, 0, .number =  115},
    {TYPE_NUMBER, 0, .number =  116}, {TYPE_NUMBER, 0, .number =  117},
    {TYPE_NUMBER, 0, .number =  118}, {TYPE_NUMBER, 0, .number =  119},
    {TYPE_NUMBER, 0, .number =  120}, {TYPE_NUMBER, 0, .number =  121},
    {TYPE_NUMBER, 0, .number =  122}, {TYPE_NUMBER, 0, .number =  123},
    {TYPE_NUMBER, 0, .number =  124}, {TYPE_NUMBER, 0, .number =  125},
    {TYPE_NUMBER, 0, .number =  126}, {TYPE_NUMBER, 0, .number =  127},
    {TYPE_NUMBER, 0, .number =  128}, {TYPE_NUMBER, 0, .number =  129},
    {TYPE_NUMBER, 0, .number =  130}, {TYPE_NUMBER, 0, .number =  131},
    {TYPE_NUMBER, 0, .number =  132}, {TYPE_NUMBER, 0, .number =  133},
    {TYPE_NUMBER, 0, .number =  134}, {TYPE_NUMBER, 0, .number =  135},
    {TYPE_NUMBER, 0, .number =  136}, {TYPE_NUMBER, 0, .number =  137},
    {TYPE_NUMBER, 0, .number =  138}, {TYPE_NUMBER, 0, .number =  139},
    {TYPE_NUMBER, 0, .number =  140}, {TYPE_NUMBER, 0, .number =  141},
    {TYPE_NUMBER, 0, .number =  142}, {TYPE_NUMBER, 0, .number =  143},
    {TYPE_NUMBER, 0, .number =  144}, {TYPE_NUMBER, 0, .number =  145},
    {TYPE_NUMBER, 0, .number =  146}, {TYPE_NUMBER, 0, .number =  147},
    {TYPE_NUMBER, 0, .number =  148}, {TYPE_NUMBER, 0, .number =  149},
    {TYPE_NUMBER, 0, .number =  150}, {TYPE_NUMBER, 0, .number =  151},
    {TYPE_NUMBER, 0, .number =  152}, {TYPE_NUMBER, 0, .number =  153},
    {TYPE_NUMBER, 0, .number =  154}, {TYPE_NUMBER, 0, .number =  155},
    {TYPE_NUMBER, 0, .number =  156}, {TYPE_NUMBER, 0, .number =  157},
    {TYPE_NUMBER, 0, .number =  158}, {TYPE_NUMBER, 0, .number =  159},
    {TYPE_NUMBER, 0, .number =  160}, {TYPE_NUMBER, 0, .number =  161},
    {TYPE_NUMBER, 0, .number =  162}, {TYPE_NUMBER, 0, .number =  163},
    {TYPE_NUMBER, 0, .number =  164}, {TYPE_NUMBER, 0, .number =  165},
    {TYPE_NUMBER, 0, .number =  166}, {TYPE_NUMBER, 0, .number =  167},
    {TYPE_NUMBER, 0, .number =  168}, {TYPE_NUMBER, 0, .number =  169},
    {TYPE_NUMBER, 0, .number =  170}, {TYPE_NUMBER, 0, .number =  171},
    {TYPE_NUMBER, 0, .number =  172}, {TYPE_NUMBER, 0, .number =  173},
    {TYPE_NUMBER, 0, .number =  174}, {TYPE_NUMBER, 0, .number =  175},
    {TYPE_NUMBER, 0, .number =  176}, {TYPE_NUMBER, 0, .number =  177},
    {TYPE_NUMBER, 0, .number =  178}, {TYPE_NUMBER, 0, .number =  179},
    {TYPE_NUMBER, 0, .number =  180}, {TYPE_NUMBER, 0, .number =  181},
    {TYPE_NUMBER, 0, .number =  182}, {TYPE_NUMBER, 0, .number =  183},
    {TYPE_NUMBER, 0, .number =  184}, {TYPE_NUMBER, 0, .number =  185},
    {TYPE_NUMBER, 0, .number =  186}, {TYPE_NUMBER, 0, .number =  187},
    {TYPE_NUMBER, 0, .number =  188}, {TYPE_NUMBER, 0, .number =  189},
    {TYPE_NUMBER, 0, .number =  190}, {TYPE_NUMBER, 0, .number =  191},
    {TYPE_NUMBER, 0, .number =  192}, {TYPE_NUMBER, 0, .number =  193},
    {TYPE_NUMBER, 0, .number =  194}, {TYPE_NUMBER, 0, .number =  195},
    {TYPE_NUMBER, 0, .number =  196}, {TYPE_NUMBER, 0, .number =  197},
    {TYPE_NUMBER, 0, .number =  198}, {TYPE_NUMBER, 0, .number =  199},
    {TYPE_NUMBER, 0, .number =  200}, {TYPE_NUMBER, 0, .number =  201},
    {TYPE_NUMBER, 0, .number =  202}, {TYPE_NUMBER, 0, .number =  203},
    {TYPE_NUMBER, 0, .number =  204}, {TYPE_NUMBER, 0, .number =  205},
    {TYPE_NUMBER, 0, .number =  206}, {TYPE_NUMBER, 0, .number =  207},
    {TYPE_NUMBER, 0, .number =  208}, {TYPE_NUMBER, 0, .number =  209},
    {TYPE_NUMBER, 0, .number =  210}, {TYPE_NUMBER, 0, .number =  211},
    {TYPE_NUMBER, 0, .number =  212}, {TYPE_NUMBER, 0, .number =  213},
    {TYPE_NUMBER, 0, .number =  214}, {TYPE_NUMBER, 0, .number =  215},
    {TYPE_NUMBER, 0, .number =  216}, {TYPE_NUMBER, 0, .number =  217},
    {TYPE_NUMBER, 0, .number =  218}, {TYPE_NUMBER, 0, .number =  219},
    {TYPE_NUMBER, 0, .number =  220}, {TYPE_NUMBER, 0, .number =  221},
    {TYPE_NUMBER, 0, .number =  222}, {TYPE_NUMBER, 0, .number =  223},
    {TYPE_NUMBER, 0, .number =  224}, {TYPE_NUMBER, 0, .number =  225},
    {TYPE_NUMBER, 0, .number =  226}, {TYPE_NUMBER, 0, .number =  227},
    {TYPE_NUMBER, 0, .number =  228}, {TYPE_NUMBER, 0, .number =  229},
    {TYPE_NUMBER, 0, .number =  230}, {TYPE_NUMBER, 0, .number =  231},
    {TYPE_NUMBER, 0, .number =  232}, {TYPE_NUMBER, 0, .number =  233},
    {TYPE_NUMBER, 0, .number =  234}, {TYPE_NUMBER, 0, .number =  235},
    {TYPE_NUMBER, 0, .number =  236}, {TYPE_NUMBER, 0, .number =  237},
    {TYPE_NUMBER, 0, .number =  238}, {TYPE_NUMBER, 0, .number =  239},
    {TYPE_NUMBER, 0, .number =  240}, {TYPE_NUMBER, 0, .number =  241},
    {TYPE_NUMBER, 0, .number =  242}, {TYPE_NUMBER, 0, .number =  243},
    {TYPE_NUMBER, 0, .number =  244}, {TYPE_NUMBER, 0, .number =  245},
    {TYPE_NUMBER, 0, .number =  246}, {TYPE_NUMBER, 0, .number =  247},
    {TYPE_NUMBER, 0, .number =  248}, {TYPE_NUMBER, 0, .number =  249},
    {TYPE_NUMBER, 0, .number =  250}, {TYPE_NUMBER, 0, .number =  251},
    {TYPE_NUMBER, 0, .number =  252}, {TYPE_NUMBER, 0, .number =  253},
    {TYPE_NUMBER, 0, .number =  254}, {TYPE_NUMBER, 0, .number =  255},
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
    static ast_data null = {TYPE_NULL, 0, .number = 0};

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
            if (data->function->vars != NULL)
            {
                free(data->function->vars);
                data->function->vars = NULL;
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

