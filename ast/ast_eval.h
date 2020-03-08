#ifndef AST_EVAL_H
#define AST_EVAL_H

ast_data ast_plus(ast_data, ast_data);
ast_data ast_minus(ast_data, ast_data);
ast_data ast_not(ast_data, ast_data);
ast_data ast_bit_not(ast_data, ast_data);
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
ast_data ast_identical(ast_data, ast_data);
ast_data ast_not_identical(ast_data, ast_data);
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

int ast_abs(unsigned);
int ast_ceil(unsigned);
int ast_cos(unsigned);
int ast_cosh(unsigned);
int ast_exp(unsigned);
int ast_floor(unsigned);
int ast_log(unsigned);
int ast_log10(unsigned);
int ast_pow(unsigned);
int ast_rand(unsigned);
int ast_round(unsigned);
int ast_sin(unsigned);
int ast_sinh(unsigned);
int ast_sqrt(unsigned);
int ast_tan(unsigned);
int ast_tanh(unsigned);
int ast_trunc(unsigned);

int ast_typeof(unsigned);
int ast_cond(unsigned);
int ast_print(unsigned);

#endif /* AST_EVAL_H */

