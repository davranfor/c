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

static void ast_exit_failure(char *file, int line)
{ 
    fprintf(stderr, "Error building ast:\n\tFile: %s\n\tLine: %d\n", file, line);
    exit(EXIT_FAILURE);
}

#define AST_EXIT_FAILURE() ast_exit_failure(__FILE__, __LINE__)

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
            case TYPE_VARIABLE:
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

static char *strings;

static ast_data *parse(const char **text)
{
    /* Position to store strings in the file */
    static size_t pos;

    const char *start = NULL;
    const char *str = *text;
    int keyword = 1;
    int quoted = 0;

    while (*str != '\0')
    {
        if (*str == '"')
        {
            quoted ^= 1;
            keyword = 0;
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
            // Check escape sequences
            if (*str == '\\')
            {
                if (!is_sequence(*++str))
                {
                    fprintf(stderr, "Bad sequence\n");
                    AST_EXIT_FAILURE();
                }
                strings[pos++] = (char)get_sequence(*str);
            }
            else if (*str != '"')
            {
                strings[pos++] = *str;
            }
        }
        if (!isspace((unsigned char)*str))
        {
            if (start == NULL)
            {
                start = strings + pos;
            }
            if ((*str != '"') && (quoted == keyword))
            {
                fprintf(stderr, "An operand must be followed by an operator\n");
                AST_EXIT_FAILURE();
            }
            if (keyword == 1)
            {
                strings[pos++] = *str;
            }
        }
        str++;
    }

    if (quoted != 0)
    {
        fprintf(stderr, "Bad quotes\n");
        AST_EXIT_FAILURE();
    }

    ast_data *data = NULL;

    // An operator?
    if (start == NULL)
    {
        data = map_operator(*str);
        *text = str + 1;
        return data;
    }

    strings[pos++] = 0;

    // A string?
    if (!keyword)
    {
        data = new_data(TYPE_STRING);
        data->string = start;
    }
    else
    {
        // A number?
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
            if (!is_valid_name(start))
            {
                fprintf(stderr, "Invalid keyword name\n");
                AST_EXIT_FAILURE();
            }
            // A function?
            else if (*str == '(')
            {
                data = map_function(start);
                if (data == NULL)
                {
                    AST_EXIT_FAILURE();
                }
            }
            // A variable
            else
            {
                data = new_data(TYPE_VARIABLE);
                data->string = start;
            }
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
                    AST_EXIT_FAILURE();
                }
                if (expected == OPERAND)
                {
                    AST_EXIT_FAILURE();
                }
                expected = OPERAND;
                starting = false;
                break;
            case ';':
                if (parenths != 0)
                {
                    AST_EXIT_FAILURE();
                }
                if (expected == OPERAND)
                {
                    AST_EXIT_FAILURE();
                }
                expected = OPERAND;
                starting = true;
                break;
            case '\0':
                if (starting == false)
                {
                    AST_EXIT_FAILURE();
                }
                if (operators != NULL)
                {
                    AST_EXIT_FAILURE();
                }
                break;
            default:
                if (expected == OPERAND)
                {
                    data = unary(data);
                    if (arguments(data) != 1)
                    {
                        AST_EXIT_FAILURE();
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
            AST_EXIT_FAILURE();
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
                            AST_EXIT_FAILURE();
                        }

                        int args = arguments(root);

                        if (!move_data(root))
                        {
                            AST_EXIT_FAILURE();
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
                                AST_EXIT_FAILURE();
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
                            AST_EXIT_FAILURE();
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
                                AST_EXIT_FAILURE();
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
            case TYPE_VARIABLE:
                printf("%s\n", node->data->string);
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

