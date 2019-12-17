#ifndef AST_EVAL_H
#define AST_EVAL_H

/*
Para cuando quieras introducir strings:
struct ast_data;
*/

int push_number(double);
double pop_number(void);

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

#endif /* AST_EVAL_H */

