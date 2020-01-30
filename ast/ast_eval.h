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
ast_data ast_bit_lshift(ast_data, ast_data);
ast_data ast_bit_rshift(ast_data, ast_data);
ast_data ast_lt(ast_data, ast_data);
ast_data ast_gt(ast_data, ast_data);
ast_data ast_lt_or_eq(ast_data, ast_data);
ast_data ast_gt_or_eq(ast_data, ast_data);
ast_data ast_is_eq(ast_data, ast_data);
ast_data ast_not_eq(ast_data, ast_data);
ast_data ast_bit_and(ast_data, ast_data);
ast_data ast_bit_xor(ast_data, ast_data);
ast_data ast_bit_or(ast_data, ast_data);
ast_data ast_and(ast_data, ast_data);
ast_data ast_xor(ast_data, ast_data);
ast_data ast_or(ast_data, ast_data);
ast_data ast_eq(ast_data, ast_data);
ast_data ast_eq_add(ast_data, ast_data);
ast_data ast_eq_sub(ast_data, ast_data);
ast_data ast_eq_mul(ast_data, ast_data);
ast_data ast_eq_div(ast_data, ast_data);
ast_data ast_eq_rem(ast_data, ast_data);
ast_data ast_eq_bit_and(ast_data, ast_data);
ast_data ast_eq_bit_xor(ast_data, ast_data);
ast_data ast_eq_bit_or(ast_data, ast_data);
ast_data ast_eq_bit_lshift(ast_data, ast_data);
ast_data ast_eq_bit_rshift(ast_data, ast_data);

int push_data(ast_data);
ast_data pop_data(void);

int ast_abs(int);
int ast_ceil(int);
int ast_cos(int);
int ast_cosh(int);
int ast_exp(int);
int ast_floor(int);
int ast_log(int);
int ast_log10(int);
int ast_pow(int);
int ast_rand(int);
int ast_round(int);
int ast_sin(int);
int ast_sinh(int);
int ast_sqrt(int);
int ast_tan(int);
int ast_tanh(int);
int ast_trunc(int);

int ast_typeof(int);
int ast_cond(int);
int ast_print(int);

#endif /* AST_EVAL_H */

