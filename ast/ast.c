/*! 
 *  \brief     Abstract syntax tree
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
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
    fprintf(stderr, "Error building ast:\n\tFile: %s\n\tLine: %d\n", file, line);
    return NULL;
}

#define AST_FAILURE ast_failure(__FILE__, __LINE__)

///////////////////////////////////////////////////////////////////////////////

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

        enum ast_type type = root->data->type;

        if ((type == TYPE_NUMBER) || (type == TYPE_STRING))
        {
            free(root->data);
        }
        free(root);
    }
}

///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////

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
                fprintf(stderr,
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

///////////////////////////////////////////////////////////////////////////////

static int is_token(int c)
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

static int is_escape(int c)
{
    return (c == '\\') || (c == '/') || (c == '"') ||
           (c == 'b')  || (c == 'f') || (c == 'n') || (c == 'r') || (c == 't');
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

static char *strings;

static ast_data *parse(const char **text)
{
    /* Position to store strings in the file */
    static size_t pos;

    const char *start = NULL;
    const char *str = *text;
    int quoted = 0;
    int is_str = 0;

    while (*str != '\0')
    {
        if (*str == '"')
        {
            quoted ^= 1;
            is_str = 1;
        }
        if (quoted == 0)
        {
            // Stop scanning on token
            if (is_token(*str))
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
        else
        {
            if (*str != '"')
            {
                strings[pos++] = *str;
            }
            // Check escape sequences
// Cuidado aquí que no lee la segunda secuéncia
            if ((*str == '\\') && !is_escape(*(++str)))
            {
                fprintf(stderr, "Bad sequence\n");
                return AST_FAILURE;
            }
        }
        if (!isspace((unsigned char)*str))
        {
            if (start == NULL)
            {
                if (!is_str)
                {
                    start = str;
                }
                else
                {
                    start = strings + pos;
                }
            }
            if ((*str != '"') && (quoted != is_str))
            {
                fprintf(stderr, "An operand must be followed with an operator\n");
                return AST_FAILURE;
            }
        }
        str++;
    }

    if (quoted != 0)
    {
        fprintf(stderr, "Bad quotes\n");
        return AST_FAILURE;
    }

    ast_data *data = NULL;

    // An operator?
    if (start == NULL)
    {
        data = map_operator(*str);
        *text = str + 1;
        return data;
    }

    // A string?
    if (is_str)
    {
        data = malloc(sizeof *data);
        if (data == NULL)
        {
            return AST_FAILURE;
        }
        data->type = TYPE_STRING;
        data->string = start;
        strings[pos++] = 0;
        *text = str;
        return data;
    }

    char buf[MAX_LENGTH];
    size_t len;

    len = trim(buf, start, str - 1);
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

///////////////////////////////////////////////////////////////////////////////

static ast_data *classify(ast_data *data)
{
    static bool starting = true;

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
                    return AST_FAILURE;
                }
                if (expected == OPERAND)
                {
                    return AST_FAILURE;
                }
                expected = OPERAND;
                starting = false;
                break;
            case ';':
                if (parenths != 0)
                {
                    return AST_FAILURE;
                }
                if (expected == OPERAND)
                {
                    return AST_FAILURE;
                }
                expected = OPERAND;
                starting = true;
                break;
            case '\0':
                if (starting == false)
                {
                    return AST_FAILURE;
                }
                if (operators != NULL)
                {
                    return AST_FAILURE;
                }
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
                starting = false;
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
        starting = false;
    }
    return data;
}

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
                case ';':
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
            case TYPE_FUNCTION:
                printf("%s\n", node->data->function->name);
                break;
            case TYPE_NUMBER:
                printf("%g\n", node->data->number);
                break;
            case TYPE_STRING:
                printf("%s\n", node->data->string);
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

ast_node *ast_build(char *text)
{
    strings = text;
    return build(text);
}

void ast_explain(const ast_node *node)
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

