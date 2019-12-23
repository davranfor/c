#ifndef AST_DATA_H
#define AST_DATA_H

typedef enum
{
    TYPE_NONE,
    TYPE_CALL,
    TYPE_OPERATOR,
    TYPE_COMPOUND,
    TYPE_STATEMENT,
    TYPE_FUNCTION,
    TYPE_VARIABLE,
    TYPE_NUMBER,
    TYPE_STRING,
} ast_type;

enum
{
    OPERATOR_EOF = 0,
    OPERATOR_PLUS = 1,
    OPERATOR_MINUS = 2,
    OPERATOR_EXP = '^',
    OPERATOR_MUL = '*',
    OPERATOR_DIV = '/',
    OPERATOR_REM = '%',
    OPERATOR_ADD = '+',
    OPERATOR_SUB = '-',
    OPERATOR_EQ = '=',
    OPERATOR_LEFT_PARENTHS = '(',
    OPERATOR_RIGHT_PARENTHS = ')',
    OPERATOR_COMMA = ',',
    OPERATOR_SEMICOLON = ';',
    OPERATOR_END = '@',
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
    int returns;
    int (*exec)(void);
} ast_call;

typedef struct ast_variable ast_variable;

typedef struct ast_data
{
    ast_type type;
    union
    {
        const ast_operator *operator;
        const ast_call *call;
        ast_variable *variable;
        double number;
        const char *string;
    };
} ast_data;

struct ast_variable
{
    const char *name;
    ast_data data;
};

ast_data *new_data(ast_type);

ast_data *map_operator(int);
int arguments(const ast_data *);
int precedence(const ast_data *, const ast_data *);
ast_data *unary(ast_data *);
int is_token(int);
int is_sequence(int);
int get_sequence(int);
int is_valid_name(const char *);

int is_statement(const char *);
ast_data *map_statement(const char *);
void map_statements(void);

ast_data *map_function(const char *);
void map_functions(void);
void unmap_functions(void);

ast_data *map_variable(const char *);
void map_variables(void);
void unmap_variables(void);

void print_help(void);

#endif /* AST_DATA_H */

