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
Expr_t evaluate_sequence_local(Expr_t expr, Env *env);  // creates local scope and calls evaluate_sequence
Expr_t evaluate_sequence(Expr_t expr, Env *env);
Expr_t evaluate_list(Expr_t expr, Env *env);

Expr_t evaluate_funcdef(Expr_t expr, Env *env);
Expr_t evaluate_func(Expr_t expr, Env *env);

// returns pointer to function responsible for evaluating keyword,
// or NULL if not a special function (e.g.: print)
void * special_func(char *name);

// special function "print"
Expr_t evaluate_print(Expr_t expr, Env *env);

char constant_to_bool(Expr_t expr);


Expr_t expr_array_to_sequence(Expr_t *arr, int size);

#endif
