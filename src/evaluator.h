#ifndef __EVALUATOR_H__
#define __EVALUATOR_H__

#include "expr.h"

Expr_t evaluate_expr(Expr_t expr);

Expr_t evaluate_arith(Expr_t expr);
Expr_t evaluate_set(Expr_t expr);
Expr_t evaluate_id(Expr_t expr);
Expr_t evaluate_bexpr(Expr_t expr);
Expr_t evaluate_cond(Expr_t expr);
Expr_t evaluate_sequence(Expr_t expr);
Expr_t evaluate_list(Expr_t expr);

char constant_to_bool(Expr_t expr);

#endif
