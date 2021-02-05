#ifndef __EVALUATOR_H__
#define __EVALUATOR_H__

#include "expr.h"
#include "env.h"

Expr_t evaluate_expr(Expr_t expr, Env *env);

Expr_t evaluate_arith(Expr_t expr, Env *env);
Expr_t evaluate_set(Expr_t expr, Env *env);
Expr_t evaluate_id(Expr_t expr, Env *env);
Expr_t evaluate_bexpr(Expr_t expr, Env *env);
Expr_t evaluate_cond(Expr_t expr, Env *env);
Expr_t evaluate_sequence(Expr_t expr, Env *env);
Expr_t evaluate_list(Expr_t expr, Env *env);

char constant_to_bool(Expr_t expr);

Expr_t expr_array_to_sequence(Expr_t *arr, int size);

#endif
