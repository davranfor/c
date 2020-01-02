#ifndef AST_EVAL_H
#define AST_EVAL_H

ast_data ast_plus(ast_data, ast_data);
ast_data ast_minus(ast_data, ast_data);
ast_data ast_not(ast_data, ast_data);
ast_data ast_xor(ast_data, ast_data);
ast_data ast_mul(ast_data, ast_data);
ast_data ast_div(ast_data, ast_data);
ast_data ast_rem(ast_data, ast_data);
ast_data ast_add(ast_data, ast_data);
ast_data ast_sub(ast_data, ast_data);
ast_data ast_lt(ast_data, ast_data);
ast_data ast_gt(ast_data, ast_data);
ast_data ast_lt_or_eq(ast_data, ast_data);
ast_data ast_gt_or_eq(ast_data, ast_data);
ast_data ast_is_eq(ast_data, ast_data);
ast_data ast_not_eq(ast_data, ast_data);
ast_data ast_eq(ast_data, ast_data);

int push_data(ast_data);
ast_data pop_data(void);

int ast_abs(void);
int ast_ceil(void);
int ast_cos(void);
int ast_cosh(void);
int ast_exp(void);
int ast_floor(void);
int ast_log(void);
int ast_log10(void);
int ast_pow(void);
int ast_rand(void);
int ast_round(void);
int ast_sin(void);
int ast_sinh(void);
int ast_sqr(void);
int ast_tan(void);
int ast_tanh(void);
int ast_trunc(void);

int ast_print(void);

#endif /* AST_EVAL_H */

