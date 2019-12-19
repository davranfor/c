#ifndef AST_DATA_H
#define AST_DATA_H

typedef enum
{
    TYPE_NONE,
    TYPE_OPERATOR,
    TYPE_FUNCTION,
    TYPE_VARIABLE,
    TYPE_NUMBER,
    TYPE_STRING,

    CLASSIFY_FUNCTION,
} ast_type;

enum
{
    OPERATOR_END = 0,
    OPERATOR_PLUS = 1,
    OPERATOR_MINUS = 2,
    OPERATOR_EXP = '^',
    OPERATOR_MUL = '*',
    OPERATOR_DIV = '/',
    OPERATOR_REM = '%',
    OPERATOR_ADD = '+',
    OPERATOR_SUB = '-',
    OPERATOR_LEFT_PARENTHS = '(',
    OPERATOR_RIGHT_PARENTHS = ')',
    OPERATOR_COMMA = ',',
    OPERATOR_SEMICOLON = ';',
};

typedef struct 
{
    int value;
    int arguments;
    int precedence;
    int associativity;
    const char *text;
} ast_operator;

typedef struct
{
    const char *name;
    int arguments;
    int (*exec)(void);
} ast_function;

typedef struct ast_data
{
    ast_type type;
    union
    {
        double number;
        const char *string;
        const ast_operator *operator;
        const ast_function *function;
    };
} ast_data;

ast_data *new_data(ast_type);

ast_data *map_operator(int);
int arguments(const ast_data *);
int precedence(const ast_data *, const ast_data *);
ast_data *unary(ast_data *);
int is_token(int);
int is_sequence(int);
int get_sequence(int);
int is_valid_name(const char *);

ast_data *map_function(const char *);
int map_functions(void);
void unmap_functions(void);

void print_help(void);

#endif /* AST_DATA_H */

