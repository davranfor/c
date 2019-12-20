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

static void move_operator(int args)
{
    while (args--)
    {
        if (!move(&operators->left, &operands))
        {
            die("Expected operand");
        }
    }
    move(&operands, &operators);
}

static int move_operands(int operator)
{
    int count = 0;

    while (node_operator(operands) != operator)
    {
        if (!move(&operators->left, &operands))
        {
            die("Unpaired parenthesis");
        }
        count++;
    }
    return count;
}

static void move_parenths(void)
{
    int count = move_operands('(');

    pop(&operands);
    switch (node_type(operands))
    {
        case CLASSIFY_FUNCTION:
            if (operands->data->function->arguments != count)
            {
                die("\"%s\" was expecting %d argument(s), got %d",
                    operands->data->function->name,
                    operands->data->function->arguments,
                    count
                );
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
                die("Comma can not be used outside function");
            }
            break;
    }
    pop(&operators);
}

static void move_data(const ast_data *data)
{
    int operator = data->operator->value;

    switch (operator)
    {
        case '(':
            move_parenths();
            break;
        default:
            move_operator(arguments(data));
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////

static char *strings;

static ast_data *parse(const char **text)
{
    static size_t pos;

    const char *start = NULL;
    const char *str = *text;
    int quotes = 0;
    int words = 0;

    for (str = *text; *str != '\0'; str++)
    {
        if (!(quotes & 1)) // Outside quotes
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
        if (!isspace((unsigned char)*str))
        {
            if (start == NULL)
            {
                start = strings + pos;
            }
            if (*str == '"')
            {
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
                    strings[pos++] = *str;
                }
                if ((words == 0) && !(quotes & 1))
                {
                    words = 1;
                }
            }
            if ((words > 1) || (quotes && words))
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
            else if (words == 1)
            {
                words++;
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

    // An operator?
    if (start == NULL)
    {
        data = map_operator(*str);
        *text = str + 1;
        return data;
    }

    strings[pos++] = 0;

    // A string?
    if (quotes != 0)
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
                die("\"%s\" is not a valid keyword name", start);
            }
            // A function?
            else if (*str == '(')
            {
                data = map_function(start);
                if (data == NULL)
                {
                    die("Function \"%s\" was not found", start);
                }
            }
            // A variable
            else
            {
                data = map_variable(start);
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
                    die("Expected operand");
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

                        int args = arguments(root);

                        move_data(root);
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
                            move_data(root);
                            continue;
                        }
                        break;
                    }
                    break;
                case ';':
                    while ((root = peek(operators)))
                    {
                        move_data(root);
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
                            move_data(root);
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
            case TYPE_FUNCTION:
                printf("%s()\n", node->data->function->name);
                break;
            case TYPE_VARIABLE:
                printf("%s\n", node->data->string);
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

static ast_data eval(const ast_node *node)
{
    ast_data data = {.type = TYPE_NUMBER};
    ast_data a = {0};
    ast_data b = {0};

    switch (node->data->type)
    {
        case TYPE_VARIABLE:
        case TYPE_NUMBER:
        case TYPE_STRING:
            return *node->data;
        case TYPE_OPERATOR:

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
                default:
                    return data;
            }
        case TYPE_FUNCTION:
            switch (node->data->function->arguments)
            {
                case 0:
                    break;
                case 1:
                    push_data(eval(node->left));
                    break;
                case 2:
                    push_data(eval(node->left));
                    push_data(eval(node->left->right));
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

