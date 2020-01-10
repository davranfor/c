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
    TYPE_BOOLEAN,
    TYPE_NUMBER,
    TYPE_STRING,
} ast_type;

enum
{
    OPERATOR_EOF           = 0x0,
    OPERATOR_PLUS          = 0x1,
    OPERATOR_MINUS         = 0x2,
    OPERATOR_NOT           = '!',
    OPERATOR_MUL           = '*',
    OPERATOR_DIV           = '/',
    OPERATOR_REM           = '%',
    OPERATOR_ADD           = '+',
    OPERATOR_SUB           = '-',
    OPERATOR_BIT_LSHIFT    = '<' ^ '!',
    OPERATOR_BIT_RSHIFT    = '>' ^ '!',
    OPERATOR_LT            = '<',
    OPERATOR_GT            = '>',
    OPERATOR_LT_OR_EQ      = '<' ^ 'z',
    OPERATOR_GT_OR_EQ      = '>' ^ 'z',
    OPERATOR_IS_EQ         = '=' ^ 'z',
    OPERATOR_NOT_EQ        = '!' ^ 'z',
    OPERATOR_BIT_AND       = '&',
    OPERATOR_BIT_XOR       = '^',
    OPERATOR_BIT_OR        = '|',
    OPERATOR_AND           = '&' ^ '!',
    OPERATOR_XOR           = '^' ^ '!',
    OPERATOR_OR            = '|' ^ '!',
    OPERATOR_EQ            = '=',
    OPERATOR_EQ_ADD        = '+' ^ 'z',
    OPERATOR_EQ_SUB        = '-' ^ 'z',
    OPERATOR_EQ_MUL        = '*' ^ 'z',
    OPERATOR_EQ_DIV        = '/' ^ 'z',
    OPERATOR_EQ_REM        = '%' ^ 'z',
    OPERATOR_EQ_BIT_AND    = '&' ^ 'z',
    OPERATOR_EQ_BIT_XOR    = '^' ^ 'z',
    OPERATOR_EQ_BIT_OR     = '|' ^ 'z',
    OPERATOR_EQ_BIT_LSHIFT = '=' ^ ('<' ^ '!'),
    OPERATOR_EQ_BIT_RSHIFT = '=' ^ ('>' ^ '!'),
    OPERATOR_LPARENTHS     = '(',
    OPERATOR_RPARENTHS     = ')',
    OPERATOR_COMMA         = ',',
    OPERATOR_SEMICOLON     = ';',
};

enum
{
    STATEMENT_NONE,
    STATEMENT_IF,
    STATEMENT_ELIF,
    STATEMENT_ELSE,
    STATEMENT_WHILE,
    STATEMENT_UNTIL,
    STATEMENT_FOR,
    STATEMENT_FOREACH,
    STATEMENT_CONTINUE,
    STATEMENT_BREAK,
    STATEMENT_END,
    STATEMENT_THEN,
    STATEMENT_IFEL,
};

typedef struct
{
    const char *name;
    int args;
    int value;
} ast_statement;

typedef struct
{
    const char *name;
    struct
    {
        int min, max;
    } args;
    int (*eval)(int);
} ast_function;

typedef struct ast_operator ast_operator;
typedef struct ast_variable ast_variable;

typedef struct ast_data
{
    ast_type type;
    union
    {
        const ast_operator *operator;
        const ast_statement *statement;
        const ast_function *function;
        ast_variable *variable;
        double number;
        const char *string;
    };
} ast_data;

struct ast_operator
{
    int value;
    int args;
    int precedence;
    int associativity;
    const char *text;
    ast_data (*eval)(ast_data, ast_data);
};

struct ast_variable
{
    const char *name;
    ast_data data;
};

int is_sequence(int);
int get_sequence(int);
int valid_name(const char *);
ast_type call_type(const ast_data *);

int is_assignment(int);
int is_operator(int);
ast_data *map_operator(const char **);
int arguments(const ast_data *);
int precedence(const ast_data *, const ast_data *);
ast_data *unary(ast_data *);

int is_iterator(const ast_data *);
ast_data *map_branch(int);
ast_data *map_statement(const char *);
void map_statements(void);

ast_data *map_function(const char *);
void map_functions(void);
void unmap_functions(void);

ast_data *map_variable(const char *);
void map_variables(void);
void unmap_variables(void);

ast_data *map_number(const char *);
ast_data *map_string(const char *);
ast_data *map_boolean(const char *);

#endif /* AST_DATA_H */

