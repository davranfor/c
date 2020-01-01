/*! 
 *  \brief     Abstract syntax tree
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "ast_eval.h"
#include "ast_data.h"
#include "ast.h"

///////////////////////////////////////////////////////////////////////////////

static size_t lines;

static void ast_die(char *file, int line, const char *format, ...)
{
    fprintf(stderr, "Error on line %zu: ", lines + 1);

    va_list args;

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    fprintf(stderr, "\nDebug info:\n\tFile: %s\n\tLine: %d\n", file, line);
    exit(EXIT_FAILURE);
}

#define die(...) ast_die(__FILE__, __LINE__, __VA_ARGS__)

///////////////////////////////////////////////////////////////////////////////

static ast_data *push(ast_node **root, ast_data *data)
{
    ast_node *node;

    node = malloc(sizeof *node);
    if (node == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    node->right = *root;
    node->left = NULL;
    node->data = data;
    *root = node;
    return data;
}

static ast_data *pop(ast_node **root)
{
    ast_data *data = NULL;
    ast_node *node;

    if ((node = *root) != NULL)
    {
        *root = node->right;
        data = node->data;
        free(node);
    }
    return data;
}

static ast_data *move(ast_node **target, ast_node **source)
{
    ast_node *node;

    if ((node = *source) == NULL)
    {
        return NULL;
    }
    *source = node->right;
    node->right = *target;
    *target = node;
    return node->data;
}

static ast_data *peek(const ast_node *root)
{
    if (root == NULL)
    {
        return NULL;
    }
    return root->data;
}

static void clear(ast_node *root)
{
    if (root != NULL)
    {
        clear(root->left);
        clear(root->right);
        switch (root->data->type)
        {
            case TYPE_NUMBER:
            case TYPE_STRING:
                free(root->data);
                break;
            default:
                break;
        }
        free(root);
    }
}

///////////////////////////////////////////////////////////////////////////////

static ast_type node_type(const ast_node *node)
{
    if (node == NULL)
    {
        return TYPE_NONE;
    }
    return node->data->type;
}

static int node_operator(const ast_node *node)
{
    if (node_type(node) != TYPE_OPERATOR)
    {
        return 0;
    }
    return node->data->operator->value;
}

///////////////////////////////////////////////////////////////////////////////

enum {OPERATOR, OPERAND};

static ast_node *operators;
static ast_node *operands;

static int expected = OPERAND;
static bool starting = true;

static void move_operator(void)
{
    int args = arguments(operators->data);

    while (args--)
    {
        if (!move(&operators->left, &operands))
        {
            die("Expected operand");
        }
    }
    move(&operands, &operators);
}

static int move_until_operator(int operator)
{
    int args = 0;

    while (node_operator(operands) != operator)
    {
        if (!move(&operators->left, &operands))
        {
            die("Expected operand");
        }
        args++;
    }
    return args;
}

static int move_until_type(ast_type type)
{
    int count = 0;

    while (node_type(operands) != type)
    {
        if (!move(&operators->left, &operands))
        {
            die("Expected operand");
        }
        count++;
    }
    return count;
}

static void move_operands(void)
{
    int args = move_until_operator('(');
    ast_call call;

    pop(&operands);
    switch (node_type(operands))
    {
        case TYPE_CALL:
            if (call_type(operands->data) == TYPE_FUNCTION)
            {
                call.name = operands->data->function->name;
                call.args = operands->data->function->args;
                if (args == 0)
                {
                    expected = OPERATOR;
                }
            }
            else
            {
                call.name = operands->data->statement->name;
                call.args = operands->data->statement->args;
                expected = OPERAND;
                starting = true;
            }
            if (call.args != args)
            {
                die("\"%s\" was expecting %d argument(s), got %d",
                    call.name,
                    call.args,
                    args
                );
            }
            operands->left = operators->left;
            operands->data += 1;
            break;
        default:
            if (args == 1)
            {
                move(&operands, &operators->left);
            }
            else
            {
                die("One argument was expected");
            }
            break;
    }
    pop(&operators);
}

static void move_expressions(void)
{
    move_until_type(TYPE_COMPOUND);
    if (node_type(operands) != TYPE_COMPOUND)
    {
        die("'end' was not expected");
    }
    if (operands->data->statement->args == 0)
    {
        operands->left = operators->left;
    }
    else
    {
        operands->left->right = operators->left;
    }
    operands->data += 1;
    starting = true;
    pop(&operators);
}

///////////////////////////////////////////////////////////////////////////////

static char *strings;

static ast_data *parse(const char **text)
{
    static size_t pos;

    const char *start = NULL;
    const char *str;
    int tokens = 0;
    int quotes = 0;

    for (str = *text; *str != '\0'; str++)
    {
        if (!(quotes & 1)) // Outside quotes
        {
            // Stop scanning on operator
            if (is_operator(*str))
            {
                break;
            }
            // Skip comments
            if (*str == '#')
            {
                while ((*str != 0) && (*str != '\n'))
                {
                    str++;
                }
            }
        }
        if (!isspace((unsigned char)*str))
        {
            if (start == NULL)
            {
                start = strings + pos;
            }
            if (*str == '"')
            {
                if (tokens != 0)
                {
                    break;
                }
                quotes++;
            }
            else
            {
                if (*str == '\\')
                {
                    if (!is_sequence(*++str))
                    {
                        die("Invalid sequence: '\\%c'", *str);
                    }
                    strings[pos++] = (char)get_sequence(*str);
                }
                else
                {
                    if ((tokens == 0) && !(quotes & 1))
                    {
                        tokens = 1;
                    }
                    else if (tokens > 1)
                    {
                        break;
                    }
                    strings[pos++] = *str;
                }
            }
            if (tokens && quotes)
            {
                die("Too many operands");
            }
        }
        else
        {
            if (quotes & 1)
            {
                strings[pos++] = *str;
            }
            else if (tokens == 1)
            {
                tokens++;
            }
            if (*str == '\n')
            {
                lines++;
            }
        }
    }
    if (quotes & 1)
    {
        die("Unpaired quotes");
    }

    ast_data *data = NULL;

    if (start == NULL)
    {
        data = map_operator(*str);
        *text = str + 1;
        return data;
    }
    strings[pos++] = 0;
    if (quotes != 0)
    {
        data = new_data(TYPE_STRING);
        data->string = start;
    }
    else
    {
        double number;
        char *ptr;

        number = strtod(start, &ptr);
        if (*ptr == '\0')
        {
            data = new_data(TYPE_NUMBER);
            data->number = number;
        }
        else
        {
            if (!valid_name(start))
            {
                die("\"%s\" is not a valid name", start);
            }

            ast_data *statement = map_statement(start);

            if (*str == '(')
            {
                if (statement != NULL)
                {
                    data = statement;
                }
                else
                {
                    data = map_function(start);
                    if (data == NULL)
                    {
                        die("\"%s\" was not found", start);
                    }
                }
            }
            else
            {
                if (statement != NULL)
                {
                    if (statement->statement->args > 0)
                    {
                        die("'(' was expected");
                    }
                    data = statement;
                }
                else
                {
                    data = map_variable(start);
                }
            }
        }
    }
    *text = str;
    return data;
}

///////////////////////////////////////////////////////////////////////////////

static ast_data *classify(ast_data *data)
{
    if (data->type == TYPE_OPERATOR)
    {
        static int parenths = 0;

        switch (data->operator->value)
        {
            case '(':
                starting = false;
                parenths++;
                break;
            case ')':
                starting = false;
                parenths--;
                break;
            case ',':
                if (parenths == 0)
                {
                    die("Comma used outside function");
                }
                if (expected == OPERAND)
                {
                    die("Expected operand");
                }
                expected = OPERAND;
                starting = false;
                break;
            case ';':
                if (parenths != 0)
                {
                    die("'(' without ')'");
                }
                if (expected == OPERAND)
                {
                    if (starting == false)
                    {
                        die("Expected operand");
                    }
                }
                expected = OPERAND;
                starting = true;
                break;
            case '\0':
                if (starting == false)
                {
                    die("Expected operand");
                }
                if (operators != NULL)
                {
                    die("Expected operand");
                }
                break;
            default:
                if (expected == OPERAND)
                {
                    data = unary(data);
                    if (arguments(data) != 1)
                    {
                        die("Expected operand");
                    }
                }
                else
                {
                    if (arguments(data) != 1)
                    {
                        expected = OPERAND;
                    }
                }
                starting = false;
                break;
        }
    }
    else
    {
        if (expected == OPERATOR)
        {
            die("Expected operator");
        }
        switch (data->type)
        {
            case TYPE_CALL:
                switch (call_type(data))
                {
                    case TYPE_FUNCTION:
                        starting = false;
                        break;
                    // case TYPE_STATEMENT:
                    default:
                        if (starting == false)
                        {
                            die("Unexpected statement");
                        }
                        starting = data->statement->args == 0;
                        break;
                }
                break;
            default:
                expected = OPERATOR;
                starting = false;
                break;
        }
    }
    return data;
}

///////////////////////////////////////////////////////////////////////////////

static ast_node *script;

static ast_node *build(const char *text)
{
    for (;;)
    {
        ast_data *data;

        data = parse(&text);
        if (data == NULL)
        {
            return NULL;
        }
        data = classify(data);
        if (data == NULL)
        {
            return NULL;
        }
        if (data->type == TYPE_OPERATOR)
        {
            ast_data *root;

            switch (data->operator->value)
            {
                case '(':
                    push(&operators, data);
                    push(&operands, data);
                    break;
                case ')':
                    for (;;)
                    {
                        root = peek(operators);
                        if (root == NULL)
                        {
                            die("')' without '('");
                        }
                        if (arguments(root) == 0)
                        {
                            move_operands();
                            break;
                        }
                        else
                        {
                            move_operator();
                        }
                    }
                    break;
                case ',':
                    while ((root = peek(operators)))
                    {
                        if (arguments(root) != 0)
                        {
                            move_operator();
                        }
                        else
                        {
                            break;
                        }
                    }
                    break;
                case ';':
                    while ((root = peek(operators)))
                    {
                        move_operator();
                    }
                    break;
                case '\0':
                    while (move(&script, &operands));
                    return script;
                default:
                    while ((root = peek(operators)))
                    {
                        if (precedence(root, data))
                        {
                            move_operator();
                        }
                        else
                        {
                            break;
                        }
                    }
                    push(&operators, data);
                    break;
            }
        }
        else if (data->type == TYPE_CALL)
        {
            if (call_type(data) == TYPE_STATEMENT)
            {
                switch (data->statement->value)
                {
                    case STATEMENT_ELIF:
                        push(&operators, map_operator(0));
                        move_expressions();
                        push(&operands, data);
                        break;
                    case STATEMENT_ELSE:
                        push(&operators, map_operator(0));
                        move_expressions();
                        push(&operands, data + 1);
                        break;
                    case STATEMENT_CONTINUE:
                    case STATEMENT_BREAK:
                        if (*text != ';')
                        {
                            die("';' was expected");
                        }
                        push(&operands, data + 2);
                        break;
                    case STATEMENT_END:
                        push(&operators, map_operator(0));
                        move_expressions();
                        break;
                    default:
                        push(&operands, data);
                        break;
                }
            }
            else // TYPE_FUNCTION
            {
                push(&operands, data);
            }            
        }
        else // TYPE_VARIABLE or TYPE_NUMBER or TYPE_STRING
        {
            push(&operands, data);
        }
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

static void explain(const ast_node *node, int level)
{
    if (node != NULL)
    {
        for (int iter = 0; iter < level; iter++)
        {
            putchar('\t');
        }
        switch (node->data->type)
        {
            case TYPE_OPERATOR:
                printf("%s\n", node->data->operator->text);
                break;
            case TYPE_STATEMENT:
                printf("%s\n", node->data->statement->name);
                break;
            case TYPE_FUNCTION:
                printf("%s()\n", node->data->function->name);
                break;
            case TYPE_VARIABLE:
                printf("%s\n", node->data->variable->name);
                break;
            case TYPE_NUMBER:
                printf("%g\n", node->data->number);
                break;
            case TYPE_STRING:
                printf("\"%s\"\n", node->data->string);
                break;
            default:
                printf("Undefined type %d\n", node->data->type);
                break;
        }
        explain(node->left, level + 1);
        explain(node->right, level);
    }
}

///////////////////////////////////////////////////////////////////////////////

#define AST_TRANSFORM_VAR(x)     \
    if (x.type == TYPE_VARIABLE) \
    {                            \
        x = x.variable->data;    \
    }

static ast_data eval(const ast_node *node)
{
    ast_data data = {.type = TYPE_NUMBER};
    ast_data a = {0};
    ast_data b = {0};
    int operator = 0;

    switch (node->data->type)
    {
        case TYPE_VARIABLE:
        case TYPE_NUMBER:
        case TYPE_STRING:
            return *node->data;
        case TYPE_OPERATOR:
            operator = node->data->operator->value;
            if (node->left != NULL)
            {
                a = eval(node->left);
                if (operator != OPERATOR_EQ)
                {
                    AST_TRANSFORM_VAR(a);
                }
                if (node->left->right != NULL)
                {
                    b = eval(node->left->right);
                    AST_TRANSFORM_VAR(b);
                }
            }
            switch (operator)
            {
                case OPERATOR_PLUS:
                    data.number = +a.number;
                    return data;
                case OPERATOR_MINUS:
                    data.number = -a.number;
                    return data;
                case OPERATOR_EXP:
                    data.number = pow(a.number, b.number);
                    return data;
                case OPERATOR_MUL:
                    data.number = a.number * b.number;
                    return data;
                case OPERATOR_DIV:
                    data.number = a.number / b.number;
                    return data;
                case OPERATOR_REM:
                    data.number = (int)a.number % (int)b.number;
                    return data;
                case OPERATOR_ADD:
                    data.number = a.number + b.number;
                    return data;
                case OPERATOR_SUB:
                    data.number = a.number - b.number;
                    return data;
                case OPERATOR_EQ:
                    a.variable->data = b;
                    return b;
                default:
                    return data;
            }
        case TYPE_FUNCTION:
            switch (node->data->function->args)
            {
                case 0:
                    break;
                case 1:
                    a = eval(node->left);
                    AST_TRANSFORM_VAR(a);
                    push_data(a);
                    break;
                case 2:
                    a = eval(node->left);
                    AST_TRANSFORM_VAR(a);
                    push_data(a);
                    b = eval(node->left->right);
                    AST_TRANSFORM_VAR(b);
                    push_data(b);
                    break;
                default:
                    die("Bad Expression");
            }
            if (node->data->function->exec() == 0)
            {
                die("Function \"%s\" returned error", node->data->function->name);
            }
            return pop_data();
        default:
            die("Bad Expression");
            exit(EXIT_FAILURE);
    }
}

///////////////////////////////////////////////////////////////////////////////

void ast_create(void)
{
    map_statements();
    map_functions();
    map_variables();
}

ast_node *ast_build(char *text)
{
    strings = text;
    return build(text);
}

void ast_explain(const ast_node *node)
{
    explain(node, 0);
}

void ast_eval(const ast_node *node)
{
    while (node != NULL)
    {
        eval(node);
        node = node->right;
    }
}

void ast_help(void)
{
    print_help();
}

void ast_clean(void)
{
    clear(script);
    clear(operators);
    clear(operands);
    script = NULL;
    operators = NULL;
    operands = NULL;
}

void ast_destroy(void)
{
    ast_clean();
    unmap_functions();
    unmap_variables();
}

