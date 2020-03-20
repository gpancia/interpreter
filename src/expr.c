#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "expr.h"

int is_null_expr(Expr_t e)
{
  return (e.e_type == Generic && e.r_type == Undef_R && e.expr.generic == NULL);
}


void print_expr(Expr_t expr)
{
  printf("{%s, %s, ", e_str[expr.e_type], r_str[1 + expr.r_type]);
  
  switch (expr.e_type) {
    case Add:
    case Sub:
    case Mul:
    case Div:
    case Concat:
      print_expr(expr.expr.arith->left);
      printf(", ");
      print_expr(expr.expr.arith->right);
      break;
    case Set:
      printf("%s, ", expr.expr.set->name);
      print_expr(expr.expr.set->val);
      break;
    case BExpr:
      printf("%s, ", b_str[expr.expr.bexpr->b_type]);
      print_expr(expr.expr.bexpr->left);
      if (expr.expr.bexpr->b_type != Not) {
        printf(", ");
        print_expr(expr.expr.bexpr->right);
      }
      break;
    case Conditional:
      printf("if: ");
      print_expr(expr.expr.cond->p);
      printf(", then: ");
      print_expr(expr.expr.cond->if_true);
      printf(", else: ");
      print_expr(expr.expr.cond->if_false);
      break;
    case List:
    case ArgList:
    case Sequence: {
      Cons_t *curr = expr.expr.cons;
      if (is_null_expr(curr->head)) {
        printf("EMPTY");
        break;
      }
      while (curr != NULL) {
        print_expr(curr->head);
        if (curr->tail != NULL) {
          printf(", ");
          curr = curr->tail;
        }
        else {break;}
      }
      break;
    }
    case Var:
      printf("%s", expr.expr.var->name);
      break;
    case Constant:
      switch (expr.r_type) {
        case Int_R:
          printf("%ld", expr.expr.constant->i);
          break;
        case Float_R:
          printf("%f", expr.expr.constant->f);
          break;
        case String_R:
          printf("\"%s\"", expr.expr.constant->str);
          break;
        case Bool_R:
          printf("%s", ((expr.expr.constant->i == 0) ? "False" : "True"));
          break;
        default:
          printf("ERROR: UNDEF");
          break;
      }
      break;
    case Function:
    case FunctionDef:
      printf("TO BE IMPLEMENTED");
      break;
    case Generic:
    default:
      break;
  }
  printf("}");
  fflush(stdout);
  return;
}

// Constant wrappers
Expr_t wrap_int(int i)
{
  Expr_t ret = {Constant, Int_R, {malloc(sizeof(Constant_t))}};
  *(ret.expr.constant) = (Constant_t){Constant, Int_R, {i}};
  return ret;
}
Expr_t wrap_flt(double f)
{
  Expr_t ret = {Constant, Float_R, {malloc(sizeof(Constant_t))}};
  *(ret.expr.constant) = (Constant_t){Constant, Float_R, {f}};
  return ret;
}
Expr_t wrap_str(char *str)
{
  Expr_t ret = {Constant, String_R, {malloc(sizeof(Constant_t))}};
  *(ret.expr.constant) = (Constant_t){Constant, String_R, {(size_t)malloc(sizeof(char)*(1+strlen(str)))}};
  sprintf(ret.expr.constant->str, "%s", str);
  return ret;
}

// Currently deprecated
Expr_t wrap_bool(char b)
{
  Expr_t ret = {Constant, Bool_R, {malloc(sizeof(Constant_t))}};
  *(ret.expr.constant) = (Constant_t){Constant, Bool_R, {b}};
  return ret;
}

// Free'ing funcs
void free_expr(Expr_t expr)
{
  if (expr.expr.generic == NULL)
	return;
  free_expr_u(expr.expr, expr.e_type);
}

void free_expr_u(Expr_u expr, enum expr_type e_type)
{
  if (expr.generic == NULL || e_type == Generic)
	return;

  else if (e_type == Set){
	free_expr(expr.set->val);
	if (expr.set->name != NULL)
	  free(expr.set->name);
	free(expr.set);
  }

  else if (e_type == BExpr){
	free_expr(expr.bexpr->left);
	free_expr(expr.bexpr->right);
	free(expr.bexpr);
  }

  else if (e_type == Conditional){
	free_expr(expr.cond->p);
	free_expr(expr.cond->if_true);
	free_expr(expr.cond->if_false);
	free(expr.cond);
  }

  else if (e_type == List || e_type == Sequence || e_type == ArgList){
	free_expr(expr.cons->head);
	Expr_u ex_u;
	ex_u.cons = expr.cons->tail;
	free_expr_u(ex_u, e_type);
	free(expr.cons);
  }

  else if (e_type < 6){
	free_expr(expr.arith->left);
	free_expr(expr.arith->right);
	free(expr.arith);
  }

  else if (e_type == FunctionDef){
	if (expr.func_def->name != NULL)
	  free(expr.func_def->name);
	Expr_u ex_u;
	ex_u.cons = expr.func_def->args;
	free_expr_u(ex_u, expr.func_def->args->e_type);
	free_expr(expr.func_def->app);
	free(expr.func_def);
  }

  else if (e_type == Function){
	if (expr.func->name != NULL)
	  free(expr.func->name);
	Expr_u ex_u;
	ex_u.cons = expr.func->args;
	free_expr_u(ex_u, expr.func->args->e_type);
	free(expr.func);
  }

  else if (e_type == Var){
	if (expr.var->name != NULL)
	  free(expr.var->name);
	free(expr.var);
  }

  else if (e_type == Constant){
	if (expr.constant->r_type == String_R){
	  if (expr.constant->str != NULL)
		free(expr.constant->str);
	}
	free(expr.constant);
  }
}
