#ifndef AST_HEAP_H
#define AST_HEAP_H

#define MAX_HEAP 256

ast_data *push(ast_node **, ast_data *);
ast_data *pop(ast_node **);
ast_data *move(ast_node **, ast_node **);
ast_data *peek(const ast_node *);
void clear(ast_node *);

void push_branch(const ast_node *);
const ast_node *pop_branch(void);
const ast_node *peek_branch(void);
int current_branch(void);
int iterating(void);

void push_call(ast_node *);
ast_node *pop_call(void);
ast_node *peek_call(void);

void push_jump(const ast_node *);
const ast_node *pop_jump(void);
const ast_node *peek_jump(void);

#endif /* AST_HEAP_H */

