/*! 
 *  \brief     Abstract syntax tree
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef AST_H
#define AST_H

#include "ast_node.h"

void ast_create(void);
ast_node *ast_build(char *);
void ast_explain(const ast_node *);
void ast_eval(const ast_node *);
void ast_help(void);
void ast_destroy(void);

#endif /* AST_H */

