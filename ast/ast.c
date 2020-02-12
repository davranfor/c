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

static ast_node *operators;
static ast_node *operands;

enum {OPERATOR, OPERAND};
static int expected = OPERAND;

enum
{
    ACCESSING   = 0x01,
    ACCESSED    = 0x02,
    MASK_ACCESS = ACCESSING | ACCESSED,
    APPENDING   = 0x04,
    ASSIGNING   = 0x08,
    MASK_PARAMS = APPENDING | ASSIGNING,
    DEFINING    = 0x10,
    DEFINED     = 0x20,
    EVALUATING  = 0x40,
    /* List of hex flags
    0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080,
    0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000, 0x8000, 
    */
};
static int status = 0;

static bool starting = true;

///////////////////////////////////////////////////////////////////////////////

static size_t lines;

static void ast_die(const char *file, int line, const char *format, ...)
{
    if (status == EVALUATING)
    {
        fprintf(stderr, "Error evaluating script:\n\t");
    }
    else
    {
        fprintf(stderr, "Error on line %zu:\n\t", lines + 1);
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
            if ((*str == '.') &&
            (
                isdigit((unsigned char)str[+1]) ||
                isdigit((unsigned char)str[-1]))
            )
            {
                // Skip decimal separator in order to not
                // process the "access member" operator
            }
            // Stop scanning on operator
            else if (is_operator(*str))
            {
                break;
            }
            // Skip comments
            else if (*str == '#')
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
    int flags = 0;

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
                    if (!(status & (DEFINING | DEFINED)))
                    {
                        die("Missing or invalid 'def'");
                    }
                    if ((*str == '.') || (status & MASK_ACCESS))
                    {
                        if (!(status & MASK_ACCESS))
                        {
                            data = map_object(start);
                            flags = ACCESSING;
                        }
                        else
                        {
                            data = map_member(operands->data->function, start);
                            flags = ACCESSED;
                        }
                    }
                    else
                    {
                        data = map_variable(start);
                    }
                    if (data == NULL)
                    {
                        die("Mapping '%s'", start);
                    }
                }
                else if (data->statement->args > 0)
                {
                    die("Missing '(' before condition");
                }
            }
        }
    }
    status &= ~MASK_ACCESS;
    status |= flags;
    *text = str;
    return data;
}

///////////////////////////////////////////////////////////////////////////////

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

static int move_operands(ast_node *node)
{
    int args = 0;

    while (operands != node)
    {
        if (!move(&node->left, &operands))
        {
            die("Expected operand");
        }
        args++;
    }
    return args;
}

static ast_data eval_expr(const ast_node *);

static int move_params(ast_node *node)
{
    int args = 0;

    def_args();
    while (operands != node)
    {
        if (operands == NULL)
        {
            die("Expected operand");
        }
        if (operands->left != NULL)
        {
            eval_expr(operands);
            clear(operands->left);
        }
        pop(&operands);
        args++;
    }
    return args;
}

static void move_arguments(void)
{
    int args;

    if (status & DEFINING)
    {
        args = move_params(pop_call());
    }
    else
    {
        args = move_operands(pop_call());
    }
    switch (operands->data->type)
    {
        case TYPE_OPERATOR:
            if (args != 1)
            {
                die("One argument was expected");
            }
            move(&operands->right, &operands->left);
            pop(&operands);
            break;
        case TYPE_STATEMENT:
            if (args != operands->data->statement->args)
            {
                die("'%s' was expecting %d argument(s), got %d",
                    operands->data->statement->name,
                    operands->data->statement->args,
                    args
                );
            }
            expected = OPERAND;
            starting = true;
            break;
        case TYPE_CALLABLE:
            if ((args < operands->data->callable->args.min) ||
                (args > operands->data->callable->args.max))
            {
                die("%s() was expecting (min: %d, max: %d) argument(s), got %d",
                    operands->data->callable->name,
                    operands->data->callable->args.min,
                    operands->data->callable->args.max,
                    args
                );
            }
            if (args == 0)
            {
                expected = OPERATOR;
            }
            break;
        case TYPE_FUNCTION:
            if (status & DEFINING)
            {
                operands->data->function->node = operands;
                status &= ~DEFINING;
                status |= DEFINED;
                expected = OPERAND;
                starting = true;
            }
            else if (args == 0)
            {
                expected = OPERATOR;
            }
            break;
        default:
            break;
    }
    pop(&operators);
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

static void move_branch(const ast_node *node)
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
            move_branch(statement);
        }
        else if (operands->data->statement->key == STATEMENT_DEF)
        {
            def_vars();
            status = 0;
        }
    }
    else
    {
        if (operands->data->statement->key == STATEMENT_ELSE)
        {
            die("An 'else' block can not be followed by another statement");
        }
    }
}

static void move_return(void)
{
    const ast_node *branch = peek_branch();

    if ((branch != NULL) && (branch->data->statement->key == STATEMENT_RETURN))
    {
        if (operands != branch)
        {
            ast_node *node = operands->right;

            move(&node->left, &operands);
        }
        pop_branch();
    }
}

///////////////////////////////////////////////////////////////////////////////

static void define(const ast_data *data)
{
    if (!(status & DEFINING))
    {
        if ((data->type == TYPE_STATEMENT) &&
            (data->statement->key == STATEMENT_DEF))
        {
            status |= DEFINING;
            return;
        }
        if ((data->type == TYPE_OPERATOR) &&
            (data->operator->key == OPERATOR_EOF))
        {
            return;
        }
        die("Only 'def's can be defined at global scope");
    }

    ast_node *call = peek_call();

    if (call == NULL)
    {
        if (data->type == TYPE_CALLABLE)
        {
            die("def %s() shadows a standard function",
                data->callable->name
            );
        }
        if (data->type == TYPE_FUNCTION)
        {
            if (data->function->node != NULL)
            {
                die("def %s() was already defined",
                    data->function->name
                );
            }
        }
        else
        {
            die("A function name was expected");
        }
    }
    else
    {
        ast_function *function = call->data->function;

        if (status & ASSIGNING)
        {
            if (function->args.min == function->args.max)
            {
                function->args.min--;
            }
            if ((data->type == TYPE_BOOLEAN) ||
                (data->type == TYPE_NUMBER) ||
                (data->type == TYPE_STRING) ||
                (data->type == TYPE_NULL))
            {
                return;
            }
        }
        else if (data->type == TYPE_VARIABLE)
        {
            if (function->args.min == function->args.max)
            {
                function->args.min++;
            }
            if (function->vars != ++function->args.max)
            {
                die("Duplicated arg name");
            }
            return;
        }
        else if (data->type == TYPE_OPERATOR)
        {
            if ((data->operator->key == ',') ||
                (data->operator->key == '=') ||
                (data->operator->key == ')'))
            {
                return;
            }
        }
        die("Wrong arg section");
    }
}

///////////////////////////////////////////////////////////////////////////////

static ast_data *classify(const char **text)
{
    ast_data *data = parse(text);
    int flags = 0;

    if (data == NULL)
    {
        return NULL;
    }
    if (!(status & DEFINED))
    {
        define(data);
    }
    switch (status & MASK_ACCESS)
    {
        case ACCESSING:
            if (data->type != TYPE_FUNCTION)
            {
                die("A function was expected");
            }
            break;
        case ACCESSED:
            if (data->type != TYPE_VARIABLE)
            {
                die("A variable was expected");
            }
            break;
        default:
            break;
    }
    if (data->type == TYPE_OPERATOR)
    {
        switch (data->operator->key)
        {
            case '(':
                starting = false;
                break;
            case ')':
                if (peek_call() == NULL)
                {
                    die("')' without '('");
                }
                if (status & APPENDING)
                {
                    die("Expected operand");
                }
                starting = false;
                break;
            case '.':
                if (expected == OPERAND)
                {
                    die("Expected operand");
                }
                flags = MASK_ACCESS;
                expected = OPERAND;
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
                flags = APPENDING;
                expected = OPERAND;
                starting = false;
                break;
            case ';':
                if (peek_call() != NULL)
                {
                    die("'(' without ')'");
                }
                if ((expected == OPERAND) &&
                    (starting == false))
                {
                    die("Expected operand");
                }
                expected = OPERAND;
                starting = true;
                break;
            case '=':
                flags = ASSIGNING;
                // fall through
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
                switch (data->statement->key)
                {
                    case STATEMENT_DEF:
                        if (status & DEFINED)
                        {
                            die("Nested 'def's are not allowed");
                        }
                        break;
                    case STATEMENT_ELIF:
                    case STATEMENT_ELSE:
                        if (statement_key() != STATEMENT_IF)
                        {
                            die("'else' without 'if'");
                        }
                        break;
                    case STATEMENT_CONTINUE:
                    case STATEMENT_BREAK:
                        if (!iterating())
                        {
                            die("'continue' and 'break'"
                                " can't be used outside a loop");
                        }
                        if (**text != ';')
                        {
                            die("';' was expected");
                        }
                        break;
                    case STATEMENT_END:
                        if (peek_statement() == NULL)
                        {
                            die("'end' was not expected");
                        }
                        break;
                    default:
                        break;
                }
                if (starting == false)
                {
                    die("Unexpected statement");
                }
                if (data->statement->key == STATEMENT_RETURN)
                {
                    starting = **text == ';';
                }
                else
                {
                    starting = data->statement->args == 0;
                }
                break;
            case TYPE_FUNCTION:
                if (status & ACCESSING)
                {
                    expected = OPERATOR;
                }
                starting = false;
                break;
            case TYPE_CALLABLE:
                starting = false;
                break;
            default:
                expected = OPERATOR;
                starting = false;
                break;
        }
    }
    status &= ~MASK_PARAMS;
    status |= flags;
    return data;
}

///////////////////////////////////////////////////////////////////////////////

static int build(const char *text)
{
    for (;;)
    {
        ast_data *data = classify(&text);

        if (data == NULL)
        {
            return 0;
        }
        if (data->type == TYPE_OPERATOR)
        {
            ast_data *root;

            switch (data->operator->key)
            {
                case '(':
                    push(&operands, data);
                    push(&operators, data);
                    push_call(operands);
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
                case '.':
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
                    move_return();
                    break;
                case '\0':
                    status = EVALUATING;
                    return 1;
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
            switch (data->statement->key)
            {
                case STATEMENT_DEF:
                    push(&operands, data);
                    push_statement(operands);
                    break;
                case STATEMENT_IF:
                case STATEMENT_WHILE:
                case STATEMENT_FOR:
                    push(&operands, data);
                    push(&operators, map_operator(&text));
                    push_statement(operands);
                    push_call(operands);
                    break;
                case STATEMENT_ELIF:
                    move_block(false);
                    push(&operands, data);
                    push(&operators, map_operator(&text));
                    push_branch(operands);
                    push_call(operands);
                    break;
                case STATEMENT_ELSE:
                    move_block(false);
                    push(&operands, data);
                    push_branch(operands);
                    break;
                case STATEMENT_END:
                    move_block(true);
                    break;
                case STATEMENT_RETURN:
                    push(&operands, data);
                    push_branch(operands);
                    break;
                default:
                    push(&operands, data);
                    break;
            }   
        }
        else if (data->type == TYPE_FUNCTION)
        {
            push(&operands, data);
            if (!(status & ACCESSING))
            {
                push(&operators, map_operator(&text));
                push_call(operands);
            }
        }
        else if (data->type == TYPE_CALLABLE)
        {
            push(&operands, data);
            push(&operators, map_operator(&text));
            push_call(operands);
        }
        else if (data->type == TYPE_VARIABLE)
        {
            if (status & ACCESSED)
            {
                pop(&operands);
            }
            push(&operands, data);
        }
        else
        {
            push(&operands, data);
        }
    }
    return 0;
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

#define GET_VAR(x)                                                          \
    if (x.type == TYPE_VARIABLE)                                            \
    {                                                                       \
        x = (*(ast_data * const *)(x.variable->base))[x.variable->offset];  \
    }

static ast_data eval_tree(const ast_node *);

static ast_data eval_function(const ast_function *function, int args)
{
    if ((args < function->args.min) || (args > function->args.max))
    {
        die("%s() was expecting (min: %d, max: %d) argument(s), got %d",
            function->name,
            function->args.min,
            function->args.max,
            args
        );
    }
    unwind_data(function->data, args);

    ast_data data = eval_tree(function->node->right);

    GET_VAR(data);
    unwind_data(function->data, function->vars);
    return data;
}

static ast_data eval_expr(const ast_node *node)
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
                a = eval_expr(node->left);
                if (is_assignment(node->data->operator->key))
                {
                    if (a.type != TYPE_VARIABLE)
                    {
                        die("lvalue required as left operand of assignment");
                    }                    
                }
                else
                {
                    GET_VAR(a);
                }
                if (node->left->right != NULL)
                {
                    b = eval_expr(node->left->right);
                    GET_VAR(b);
                }
            }
            return node->data->operator->eval(a, b);
        case TYPE_FUNCTION:
            wind_data(node->data->function->data, node->data->function->vars);
            next = node->left;
            while (next != NULL)
            {
                a = eval_expr(next);
                GET_VAR(a);
                push_data(a);
                next = next->right;
                args++;
            }
            if (node->data->function->node == NULL)
            {
                die("%s() is not defined", node->data->function->name);
            }
            return eval_function(node->data->function, args);
        case TYPE_CALLABLE:
            next = node->left;
            while (next != NULL)
            {
                a = eval_expr(next);
                GET_VAR(a);
                push_data(a);
                next = next->right;
                args++;
            }
            if (node->data->callable->eval(args) == 0)
            {
                die("%s() returned error", node->data->callable->name);
            }
            return pop_data();
        default:
            die("Bad Expression");
            exit(EXIT_FAILURE);
    }
}

static int eval_cond(const ast_node *node)
{
    ast_data data = eval_expr(node);

    GET_VAR(data);
    if (data.type == TYPE_STRING)
    {
        return data.string[0] != '\0';
    }
    else
    {
        return data.number != 0;
    }
}

static ast_data eval_tree(const ast_node *node)
{
    while (node != NULL)
    {
        switch (node->data->type)
        {
            case TYPE_STATEMENT:
                switch (node->data->statement->key)
                {
                    case STATEMENT_FOR:
                        if (node != peek_jump())
                        {
                            push_jump(node);
                            eval_expr(node->left);
                            node = node->left->right;
                        }
                        else
                        {
                            node = node->left->right;
                            eval_expr(node->right);
                        }
                        if (eval_cond(node) == 0)
                        {
                            node = pop_jump()->right;
                        }
                        else
                        {
                            node = node->right->right;
                        }
                        break;
                    case STATEMENT_WHILE:
                        if (node != peek_jump())
                        {
                            push_jump(node);
                        }
                        if (eval_cond(node->left) == 0)
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
                        if (eval_cond(node->left) == 0)
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
                        if (eval_cond(node) == 0)
                        {
                            node = node->right->right;
                        }
                        else
                        {
                            node = node->right->left;
                        }
                        break;
                    case STATEMENT_ELIF:
                        if (eval_cond(node->left) == 0)
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
                    case STATEMENT_RETURN:
                        while (pop_jump());
                        if (node->left == NULL)
                        {
                            return (ast_data){0};
                        }
                        return eval_expr(node->left);
                    default:
                        break;
                }
                break;
            default:
                eval_expr(node);
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
    return (ast_data){0};
}

///////////////////////////////////////////////////////////////////////////////

void ast_create(void)
{
    map_data();
}

int ast_build(char *script)
{
    strings = script;
    return build(script);
}

void ast_explain(void)
{
    ast_node *node = NULL;

    while (move(&node, &operands));
    explain(node, 0);
    operands = node;
}

void ast_eval(void)
{
    const ast_data *data = map_callable("main");

    if ((data == NULL) || (data->function->node == NULL))
    {
        die("main() is not defined");
    }
    wind_data(data->function->data, data->function->vars);
    eval_function(data->function, 0);
}

void ast_help(void)
{
    puts("Options:");
    puts("   --help\tDisplay this information");
    puts("   --tree\tDisplay an abstract syntax tree");
}

void ast_destroy(void)
{
    clear(operators);
    operators = NULL;
    clear(operands);
    operands = NULL;
    unmap_data();
}

