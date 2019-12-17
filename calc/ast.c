/*! 
 *  \brief     Calculator - Abstract syntax tree
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

/*
 * Abstract Syntax Tree
 * https://en.wikipedia.org/wiki/Shunting-yard_algorithm
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "ast_eval.h"
#include "ast_data.h"
#include "ast.h"

static void *ast_failure(char *file, int line)
{ 
    fprintf(stderr, "Error building ast\nFile: %s\nLine: %d\n", file, line);
    return NULL;
}

#define AST_FAILURE ast_failure(__FILE__, __LINE__)

static ast_data *push(ast_node **root, ast_data *data)
{
    ast_node *node;

    node = malloc(sizeof *node);
    if (node == NULL)
    {
        AST_FAILURE;
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
        if (root->data->type == TYPE_NUMBER)
        {
            free(root->data);
        }
        free(root);
    }
}

static enum ast_type node_type(const ast_node *node)
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

static ast_node *operators;
static ast_node *operands;

enum {OPERATOR, OPERAND};
static int expected = OPERAND;

static int move_operator(int args)
{
    int count = 0;

    while (count < args)
    {
        if (!move(&operators->left, &operands))
        {
            return 0;
        }
        count++;
    }
    move(&operands, &operators);
    return count;
}

static int move_operands(int operator)
{
    int count = 0;

    while (node_operator(operands) != operator)
    {
        if (!move(&operators->left, &operands))
        {
            return -1;
        }
        count++;
    }
    return count;
}

static int move_parenths(void)
{
    int count = move_operands('(');

    if (count == -1)
    {
        return 0;
    }
    pop(&operands);
    switch (node_type(operands))
    {
        case CLASSIFY_FUNCTION:
            if (operands->data->function->arguments != count)
            {
                fprintf(
                    stderr,
                    "\"%s\" was expecting %d argument(s), got %d\n",
                    operands->data->function->name,
                    operands->data->function->arguments,
                    count
                );
                return 0;
            }
            operands->left = operators->left;
            operands->data += 1;
            if (count == 0)
            {
                expected = OPERATOR;
            }
            break;
        default:
            if (count == 1)
            {
                move(&operands, &operators->left);
            }
            else
            {
                return 0;
            }
            break;
    }
    pop(&operators);
    return 1;
}

static int move_data(const ast_data *data)
{
    int operator = data->operator->value;

    switch (operator)
    {
        case '(':
            return move_parenths();
        default:
            return move_operator(arguments(data));
    }
}

static int is_token(int c)
{
    return
        (c == OPERATOR_END) ||
        (c == OPERATOR_EXP) ||
        (c == OPERATOR_MUL) ||
        (c == OPERATOR_DIV) ||
        (c == OPERATOR_REM) ||
        (c == OPERATOR_ADD) ||
        (c == OPERATOR_SUB) ||
        (c == OPERATOR_LEFT_PARENTHS) ||
        (c == OPERATOR_RIGHT_PARENTHS) ||
        (c == OPERATOR_COMMA) ||
        (c == OPERATOR_LINE_BREAK);
}

#define MAX_LENGTH 64

static size_t trim(char *str, const char *left, const char *right)
{
    while (right > left)
    {
        if (!isspace((unsigned char)*right))
        {
            break;
        }
        right--;
    }

    size_t len = (size_t)(right - left) + 1;

    if (len >= MAX_LENGTH)
    {
        fprintf(stderr, "Keyword MaxLength = %d\n", MAX_LENGTH - 1);
        return 0;
    }
    memcpy(str, left, len);
    str[len] = '\0';
    return len;
}

static ast_data *parse(const char **text)
{
    const char *str = *text;

    *text = NULL;
    while (*str != '\0')
    {
        if (is_token(*str))
        {
            break;
        }
        /* First character which is not a space nor a token */
        if ((*text == NULL) && !isspace((unsigned char)*str))
        {
            *text = str;
        }
        str++;
    }

    ast_data *data;

    // An operator?
    if (*text == NULL)
    {
        data = map_operator(*str);
        *text = str + 1;
        return data;
    }

    char buf[MAX_LENGTH];
    size_t len;

    len = trim(buf, *text, str - 1);
    if (len == 0)
    {
        return AST_FAILURE;
    }

    // A number?
    double number;
    char *ptr;

    number = strtod(buf, &ptr);
    if (*ptr == '\0')
    {
        data = malloc(sizeof *data);
        if (data == NULL)
        {
            return AST_FAILURE;
        }
        data->type = TYPE_NUMBER;
        data->number = number;
    }
    // A function?
    else
    {
        if (*str != '(')
        {
            return AST_FAILURE;
        }
        data = map_function(buf);
        if (data == NULL)
        {
            return AST_FAILURE;
        }
    }
    *text = str;
    return data;
}

static ast_data *classify(ast_data *data)
{
    static int parenths = 0;

    if (data->type == TYPE_OPERATOR)
    {
        switch (data->operator->value)
        {
            case '(':
                parenths++;
                break;
            case ')':
                parenths--;
                break;
            case ',':
                if (parenths == 0)
                {
                    return AST_FAILURE;
                }
                if (expected == OPERAND)
                {
                    return AST_FAILURE;
                }
                expected = OPERAND;
                break;
            default:
                if (expected == OPERAND)
                {
                    data = unary(data);
                    if (arguments(data) != 1)
                    {
                        return AST_FAILURE;
                    }
                }
                else
                {
                    if (arguments(data) != 1)
                    {
                        expected = OPERAND;
                    }
                }
                break;
            case '\n':
                if (parenths != 0)
                {
                    return AST_FAILURE;
                }
                if (expected == OPERAND)
                {
                    return AST_FAILURE;
                }
                expected = OPERAND;
                break;
            case '\0':
                if (operators != NULL)
                {
                    return AST_FAILURE;
                }
                break;
        }
    }
    else
    {
        if (expected == OPERATOR)
        {
            return AST_FAILURE;
        }
        switch (data->type)
        {
            case CLASSIFY_FUNCTION:
                break;
            default:
                expected = OPERATOR;
                break;
        }
    }
    return data;
}

static ast_node *script;

static ast_node *build(const char *text)
{
    for (;;)
    {
        ast_data *root, *data;

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
                            return AST_FAILURE;
                        }

                        int args = arguments(root);

                        if (!move_data(root))
                        {
                            return AST_FAILURE;
                        }
                        if (args == 0)
                        {
                            break;
                        }
                    }
                    break;
                case ',':
                    while ((root = peek(operators)))
                    {
                        if (arguments(root) != 0)
                        {
                            if (!move_data(root))
                            {
                                return AST_FAILURE;
                            }
                            continue;
                        }
                        break;
                    }
                    break;
                default:
                    while ((root = peek(operators)))
                    {
                        if (precedence(root, data))
                        {
                            if (!move_data(root))
                            {
                                return AST_FAILURE;
                            }
                            continue;
                        }
                        break;
                    }
                    push(&operators, data);
                    break;
                case '\n':
                    while ((root = peek(operators)))
                    {
                        if (!move_data(root))
                        {
                            return AST_FAILURE;
                        }
                    }
                    break;
                case '\0':
                    while (move(&script, &operands));
                    return script;
            }
        }
        else
        {
            push(&operands, data);
        }
    }
    return NULL;
}

static void print(const ast_node *node, int level)
{
    int iter;

    if (node != NULL)
    {
        for (iter = 0; iter < level; iter++)
        {
            putchar('\t');
        }
        switch (node->data->type)
        {
            case TYPE_OPERATOR:
                printf("%s\n", node->data->operator->text);
                break;
            case TYPE_FUNCTION:
                printf("%s\n", node->data->function->name);
                break;
            case TYPE_NUMBER:
                printf("%g\n", node->data->number);
                break;
            default:
                printf("Undefined type %d\n", node->data->type);
                break;
        }
        print(node->left, level + 1);
        print(node->right, level);
    }
}

static double eval(const ast_node *node)
{
    if (node->data->type == TYPE_NUMBER)
    {
        return node->data->number;
    }
    else
    if (node->data->type == TYPE_OPERATOR)
    {
        double a = 0;
        double b = 0;

        if (node->left != NULL)
        {
            a = eval(node->left);
            if (node->left->right != NULL)
            {
                b = eval(node->left->right);
            }
        }
        switch (node->data->operator->value)
        {
            case OPERATOR_PLUS:
                return +a;
            case OPERATOR_MINUS:
                return -a;
            case OPERATOR_EXP:
                return pow(a, b);
            case OPERATOR_MUL:
                return a * b;
            case OPERATOR_DIV:
                return a / b;
            case OPERATOR_REM:
                return (int)a % (int)b;
            case OPERATOR_ADD:
                return a + b;
            case OPERATOR_SUB:
                return a - b;
            default:
                return 0;
        }
    }
    else
    if (node->data->type == TYPE_FUNCTION)
    {
        switch (node->data->function->arguments)
        {
            case 0:
                break;
            case 1:
                push_number(eval(node->left));
                break;
            case 2:
                push_number(eval(node->left));
                push_number(eval(node->left->right));
                break;
            default:
                return 0;
        }
        if (node->data->function->exec() == 0)
        {
            return 0;
        }
        return pop_number();
    }
    else
    {
        return 0;
    }
}

int ast_create(void)
{
    return map_functions();
}

ast_node *ast_build(const char *expression)
{
    return build(expression);
}

void ast_print(const ast_node *node)
{
    print(node, 0);
}

void ast_eval(const ast_node *node)
{
    printf("%g\n", eval(node));
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
}

