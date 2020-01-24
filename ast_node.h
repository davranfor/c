#ifndef AST_NODE_H
#define AST_NODE_H

typedef struct ast_node
{
    struct ast_node *left;
    struct ast_node *right;
    struct ast_data *data;
} ast_node;

#endif /* AST_NODE_H */

