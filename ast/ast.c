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
#include "ast.h"
#include "ast_data.h"
#include "ast_eval.h"
#include "ast_heap.h"

///////////////////////////////////////////////////////////////////////////////

static ast_node *script;

static size_t lines;

static void ast_die(char *file, int line, const char *format, ...)
{
    if (script == NULL)
    {
        fprintf(stderr, "Error on line %zu:\n\t", lines + 1);
    }
    else
    {
        fprintf(stderr, "Error evaluating script:\n\t");
    }

    va_list args;

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\nDebug info:\n\tFile: %s\n\tLine: %d\n", file, line);
    exit(EXIT_FAILURE);
}

#define die(...) ast_die(__FILE__, __LINE__, __VA_ARGS__)

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
                while ((*str != '\0') && (*str != '\n'))
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
        data = map_operator(&str);
    }
    else
    {
        strings[pos++] = '\0';
        if (quotes != 0)
        {
            data = map_string(start);
        }
        else if ((data = map_number(start)))
        {
            // Decimal, hexadecimal, float
        }
        else if ((data = map_boolean(start)))
        {
            // true or false
        }
        else if ((data = map_null(start)))
        {
            // null
        }
        else
        {
            if (!valid_name(start))
            {
                die("\"%s\" is not a valid name", start);
            }
            data = map_statement(start);
            if (*str == '(')
            {
                if (data == NULL)
                {
                    data = map_callable(start);
                }
            }
            else
            {
                if (data == NULL)
                {
                    data = map_variable(start);
                }
                else if (data->statement->args > 0)
                {
                    die("'(' was expected");
                }
            }
        }
    }
    *text = str;
    return data;
}

///////////////////////////////////////////////////////////////////////////////

enum {OPERATOR, OPERAND};

static ast_node *operators;
static ast_node *operands;

static int expected = OPERAND;
static bool starting = true;

///////////////////////////////////////////////////////////////////////////////

static void move_operator(void)
{
    int args = arguments(operators->data);
    struct call *call = peek_call();

    if (call != NULL)
    {
        call->args += 1 - args;
    }
    while (args--)
    {
        if (!move(&operators->left, &operands))
        {
            die("Expected operand");
        }
    }
    move(&operands, &operators);
}

static void move_operands(int args)
{
    while (args--)
    {
        if (!move(&operators->left, &operands))
        {
            die("Expected operand");
        }
    }
}

static void move_arguments(void)
{
    struct call *call = pop_call();

    if (call == NULL)
    {
        die("')' without '('");
    }
    move_operands(call->args);
    switch (call->type)
    {
        case TYPE_OPERATOR:
            if (call->args == 1)
            {
                move(&operands, &operators->left);
            }
            else
            {
                die("One argument was expected");
            }
            break;
        case TYPE_STATEMENT:
            if (call->args != operands->data->statement->args)
            {
                die("\"%s\" was expecting %d argument(s), got %d",
                    operands->data->statement->name,
                    operands->data->statement->args,
                    call->args
                );
            }
            operands->left = operators->left;
            expected = OPERAND;
            starting = true;
            break;
        case TYPE_CALLABLE:
            if ((call->args < operands->data->callable->args.min) ||
                (call->args > operands->data->callable->args.max))
            {
                die("\"%s\" was expecting %d to %d argument(s), got %d",
                    operands->data->callable->name,
                    operands->data->callable->args.min,
                    operands->data->callable->args.max,
                    call->args
                );
            }
            // FALLTHROUGH
        case TYPE_FUNCTION:
            operands->left = operators->left;
            if (call->args == 0)
            {
                expected = OPERATOR;
            }
            break;
        default:
            break;
    }
    pop(&operators);
    call = peek_call();
    if (call != NULL)
    {
        call->args++;
    }
}

static void move_expressions(ast_node *node)
{
    const ast_node *statement = peek_statement();
    const ast_node *branch = peek_branch();

    while ((operands != statement) && (operands != branch))
    {
        if (!move(&node->left, &operands))
        {
            die("Expected operand");
        }
    }
}

static void move_branches(const ast_node *node)
{
    push(&operators, map_branch(1));
    operators->left = node->left->right;
    node->left->right = NULL;
    while (operands != node)
    {
        move(&node->left->right, &operands);
    }
    move(&operands->left->right, &operators);
    operands->data = map_branch(0);
    pop_branch();
}

static void move_block(bool end)
{
    ast_node node = {0};

    move_expressions(&node);
    if (operands == NULL)
    {
        die("'end' was not expected");
    }
    switch (operands->data->statement->args)
    {
        case 0:
            operands->left = node.left;
            break;
        case 1:
            operands->left->right = node.left;
            break;
        case 2:
            operands->left->right->right = node.left;
            break;
        case 3:
            operands->left->right->right->right = node.left;
            break;
        default:
            break;
    }
    if (end)
    {
        const ast_node *statement = pop_statement();

        if (operands != statement)
        {
            move_branches(statement);
        }
    }
    else
    {
        if (operands->data->statement->value == STATEMENT_ELSE)
        {
            die("An 'else' block can not be followed by another statement");
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

static ast_data *classify(const char **text)
{
    ast_data *data;

    data = parse(text);
    if (data == NULL)
    {
        return NULL;
    }
    if (data->type == TYPE_OPERATOR)
    {
        switch (data->operator->value)
        {
            case '(':
            case ')':
                starting = false;
                break;
            case ',':
                if (peek_call() == NULL)
                {
                    die("',' was not expected");
                }
                if (expected == OPERAND)
                {
                    die("Expected operand");
                }
                expected = OPERAND;
                starting = false;
                break;
            case ';':
                if (peek_call() != NULL)
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
                if (peek_statement() != NULL)
                {
                    die("'end' was expected");
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
            case TYPE_STATEMENT:
                switch (data->statement->value)
                {
                    case STATEMENT_ELIF:
                    case STATEMENT_ELSE:
                        if (peek_statement_type() != STATEMENT_IF)
                        {
                            die("'else' without 'if'");
                        }
                        break;
                    case STATEMENT_CONTINUE:
                    case STATEMENT_BREAK:
                        if (**text != ';')
                        {
                            die("';' was expected");
                        }
                        if (peek_statement_iterators() == 0)
                        {
                            die("'continue' and 'break'"
                                " can't be used outside a loop");
                        }
                        break;
                    default:
                        break;
                }
                if (starting == false)
                {
                    die("Unexpected statement");
                }
                starting = data->statement->args == 0;
                break;
            case TYPE_FUNCTION:
            case TYPE_CALLABLE:
                starting = false;
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

static ast_node *build(const char *text)
{
    for (;;)
    {
        ast_data *data;

        data = classify(&text);
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
                    push_call(TYPE_OPERATOR);
                    break;
                case ')':
                    while ((root = peek(operators)))
                    {
                        if (arguments(root) == 0)
                        {
                            move_arguments();
                            break;
                        }
                        move_operator();
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
        else if (data->type == TYPE_STATEMENT)
        {
            switch (data->statement->value)
            {
                case STATEMENT_IF:
                case STATEMENT_WHILE:
                case STATEMENT_UNTIL:
                case STATEMENT_FOR:
                case STATEMENT_FOREACH:
                    push(&operands, data);
                    push_statement(operands);
                    push(&operators, map_operator(&text));
                    push_call(TYPE_STATEMENT);
                    break;
                case STATEMENT_ELIF:
                    move_block(false);
                    push(&operands, data);
                    push_branch(operands);
                    push(&operators, map_operator(&text));
                    push_call(TYPE_STATEMENT);
                    break;
                case STATEMENT_ELSE:
                    move_block(false);
                    push(&operands, data);
                    push_branch(operands);
                    break;
                case STATEMENT_END:
                    move_block(true);
                    break;
                default:
                    push(&operands, data);
                    break;
            }   
        }
        else if (data->type == TYPE_FUNCTION)
        {
            push(&operands, data);
            push(&operators, map_operator(&text));
            push_call(TYPE_FUNCTION);
        }
        else if (data->type == TYPE_CALLABLE)
        {
            push(&operands, data);
            push(&operators, map_operator(&text));
            push_call(TYPE_CALLABLE);
        }
        else
        {
            push(&operands, data);
            if (peek_call() != NULL)
            {
                peek_call()->args++;
            }
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
            case TYPE_CALLABLE:
                printf("%s()\n", node->data->callable->name);
                break;
            case TYPE_VARIABLE:
                printf("%s\n", node->data->variable->name);
                break;
            case TYPE_BOOLEAN:
                printf("%s\n", node->data->number ? "true" : "false");
                break;
            case TYPE_NUMBER:
                printf("%g\n", node->data->number);
                break;
            case TYPE_STRING:
                printf("\"%s\"\n", node->data->string);
                break;
            case TYPE_NULL:
                printf("null\n");
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
    ast_node *next = NULL;
    ast_data a = {0};
    ast_data b = {0};
    int args = 0;

    switch (node->data->type)
    {
        case TYPE_VARIABLE:
        case TYPE_BOOLEAN:
        case TYPE_NUMBER:
        case TYPE_STRING:
        case TYPE_NULL:
            return *node->data;
        case TYPE_OPERATOR:
            if (node->left != NULL)
            {
                a = eval(node->left);
                if (is_assignment(node->data->operator->value))
                {
                    if (a.type != TYPE_VARIABLE)
                    {
                        die("lvalue required as left operand of assignment");
                    }                    
                }
                else
                {
                    AST_TRANSFORM_VAR(a);
                }
                if (node->left->right != NULL)
                {
                    b = eval(node->left->right);
                    AST_TRANSFORM_VAR(b);
                }
            }
            return node->data->operator->eval(a, b);
        case TYPE_CALLABLE:
            next = node->left;
            while (next != NULL)
            {
                a = eval(next);
                AST_TRANSFORM_VAR(a);
                push_data(a);
                next = next->right;
                args++;
            }
            if (node->data->callable->eval(args) == 0)
            {
                die("\"%s\" returned error", node->data->callable->name);
            }
            return pop_data();
        default:
            die("Bad Expression");
            exit(EXIT_FAILURE);
    }
}

///////////////////////////////////////////////////////////////////////////////

static int test(const ast_node *node)
{
    ast_data data = eval(node);

    if (data.type == TYPE_VARIABLE)
    {
        data = data.variable->data;
    }
    if (data.type == TYPE_STRING)
    {
        return data.string[0] == '\0';
    }
    else
    {
        return data.number != 0;
    }
}

///////////////////////////////////////////////////////////////////////////////

void ast_create(void)
{
    map_data();
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
        switch (node->data->type)
        {
            case TYPE_STATEMENT:
                switch (node->data->statement->value)
                {
                    case STATEMENT_FOR:
                        if (node != peek_jump())
                        {
                            push_jump(node);
                            eval(node->left);
                            node = node->left->right;
                        }
                        else
                        {
                            node = node->left->right;
                            eval(node->right);
                        }
                        if (test(node) == 0)
                        {
                            node = pop_jump()->right;
                        }
                        else
                        {
                            node = node->right->right;
                        }
                        break;
                    case STATEMENT_WHILE:
                    case STATEMENT_UNTIL:
                    case STATEMENT_FOREACH:
                        if (node != peek_jump())
                        {
                            push_jump(node);
                        }
                        if (test(node->left) == 0)
                        {
                            node = pop_jump()->right;
                        }
                        else
                        {
                            node = node->left->right;
                        }
                        break;
                    case STATEMENT_IF:
                        push_jump(node);
                        if (test(node->left) == 0)
                        {
                            node = pop_jump()->right;
                        }
                        else
                        {
                            node = node->left->right;
                        }
                        break;
                    case STATEMENT_IFEL:
                        push_jump(node);
                        node = node->left;
                        if (test(node) == false)
                        {
                            node = node->right->right;
                        }
                        else
                        {
                            node = node->right->left;
                        }
                        break;
                    case STATEMENT_ELIF:
                        if (test(node->left) == false)
                        {
                            node = node->right;
                        }
                        else
                        {
                            node = node->left->right;
                        }
                        break;
                    case STATEMENT_ELSE:
                        node = node->left;
                        break;
                    case STATEMENT_CONTINUE:
                        while ((node = peek_jump()))
                        {
                            if (is_iterator(node->data))
                            {
                                node = NULL;
                                break;
                            }
                            pop_jump();
                        }
                        break;
                    case STATEMENT_BREAK:
                        while ((node = pop_jump()))
                        {
                            if (is_iterator(node->data))
                            {
                                node = node->right;
                                break;
                            }
                        }
                        break;
                    default:
                        break;
                }
                break;
            default:
                eval(node);
                node = node->right;
                break;
        }
        if (node == NULL)
        {
            while ((node = peek_jump()))
            {
                if (is_iterator(node->data))
                {
                    break;
                }
                if ((node = pop_jump()->right))
                {
                    break;
                }
            }
        }
    }
}

void ast_help(void)
{
    puts("Options:");
    puts("   --help\tDisplay this information");
    puts("   --tree\tDisplay an abstract syntax tree");
}

void ast_destroy(void)
{
    clear(script);
    script = NULL;
    clear(operators);
    operators = NULL;
    clear(operands);
    operands = NULL;
    unmap_data();
}

