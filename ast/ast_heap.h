#ifndef AST_HEAP_H
#define AST_HEAP_H

#define MAX_HEAP 256

ast_data *push(ast_node **, ast_data *);
ast_data *pop(ast_node **);
ast_data *move(ast_node **, ast_node **);
ast_data *peek(const ast_node *);
void clear(ast_node *);

void push_statement(const ast_node *);
const ast_node *pop_statement(void);
const ast_node *peek_statement(void);
int statement_type(void);
int defs(void);
int defined(void);
int defining(const ast_node *);
int iterators(void);

void push_branch(const ast_node *);
const ast_node *pop_branch(void);
const ast_node *peek_branch(void);

struct calls
{
    struct call
    {
        ast_type type;
        int args;
    } call[MAX_HEAP];
    int count;
};
void push_call(ast_type);
struct call *pop_call(void);
struct call *peek_call(void);

void push_jump(const ast_node *);
const ast_node *pop_jump(void);
const ast_node *peek_jump(void);

#endif /* AST_HEAP_H */

