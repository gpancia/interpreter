#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
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
            printf("%lld", expr.expr.constant->i);
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
Expr_t wrap_int(long long i)
{
    Expr_t ret = {Constant, Int_R, {malloc(sizeof(Constant_t))}};
    *(ret.expr.constant) = (Constant_t){Constant, Int_R, {.i=i}};
    return ret;
}
Expr_t wrap_flt(double f)
{
    Expr_t ret = {Constant, Float_R, {malloc(sizeof(Constant_t))}};
    *(ret.expr.constant) = (Constant_t){Constant, Float_R, {.f=f}};
    // *(ret.expr.constant) = (Constant_t){Constant, Float_R, {*((long long*)&f)}};  // evil bit trick
    return ret;
}
Expr_t wrap_str(char *str)
{
    Expr_t ret = {Constant, String_R, {malloc(sizeof(Constant_t))}};
    *(ret.expr.constant) = (Constant_t){Constant, String_R, {.str=(char*)malloc(sizeof(char)*(1+strlen(str)))}};
    sprintf(ret.expr.constant->str, "%s", str);
    return ret;
}

Expr_t wrap_bool(char b)
{
    Expr_t ret = {Constant, Bool_R, {malloc(sizeof(Constant_t))}};
    *(ret.expr.constant) = (Constant_t){Constant, Bool_R, {.b = b}};
    return ret;
}

enum result_type consolidate_constant_pair(Expr_t lr_expr[2], Constant_Values *lr_vals[2]) {
    enum result_type r_type;
    lr_vals[0] = (Constant_Values*) malloc(sizeof(Constant_Values));
    lr_vals[1] = (Constant_Values*) malloc(sizeof(Constant_Values));
    *lr_vals[0] = (Constant_Values){0, 0.0, "", 0};
    *lr_vals[1] = (Constant_Values){0, 0.0, "", 0};
    for (int i = 0; i < 2; i++) {
        if (lr_expr[i].e_type != Constant) {
            fprintf(stderr, "unwrap_consolidate_constant_pair: invalid expressions of type %s:\n", e_str[lr_expr[i].e_type]);
            print_expr(lr_expr[i]);
            exit(1);
        }
        if (lr_expr[0].r_type == lr_expr[1].r_type) {
            r_type = lr_expr[0].r_type;
            lr_vals[i]->i = lr_expr[i].expr.constant->i;
            lr_vals[i]->f = lr_expr[i].expr.constant->f;
            strcpy(lr_vals[i]->str, (r_type == String_R) ? lr_expr[i].expr.constant->str : "");
            lr_vals[i]->b = lr_expr[i].expr.constant->b;
        }
        else if (lr_expr[0].r_type == String_R || lr_expr[1].r_type == String_R) {
            r_type = String_R;
            switch (lr_expr[i].r_type) {
            case String_R:
                strcpy(lr_vals[i]->str, lr_expr[i].expr.constant->str);
                break;
            case Float_R:
                sprintf(lr_vals[i]->str, "%f", lr_expr[i].expr.constant->f);
                break;
            case Int_R:
                sprintf(lr_vals[i]->str, "%lld", lr_expr[i].expr.constant->i);
                break;
            case Bool_R:
                strcpy(lr_vals[i]->str, (lr_expr[i].expr.constant->b) ? "true" : "false");
                break;
            default:
                strcpy(lr_vals[i]->str, "undef");
                break;
            }
        }
        else if (lr_expr[0].r_type == Float_R || lr_expr[1].r_type == Float_R) {
            r_type = Float_R;
            switch (lr_expr[i].r_type) {
            case Float_R:
                lr_vals[i]->f = (double) lr_expr[i].expr.constant->f;
                break;
            case Int_R:
                lr_vals[i]->f = (double) lr_expr[i].expr.constant->i;
                break;
            case Bool_R:
                lr_vals[i]->f = (double) (lr_expr[i].expr.constant->b) ? 1.0 : 0.0;
                break;
            default:
                lr_vals[i]->f = (double) 0.0;
                break;
            }
        }
        else if (lr_expr[0].r_type == Int_R || lr_expr[1].r_type == Int_R) {
            r_type =  Bool_R;
            for (int i = 0; i < 2; i++) {
                switch (lr_expr[i].r_type) {
                case Int_R:
                    lr_vals[i]->i = (long long) lr_expr[i].expr.constant->i;
                    break;
                case Bool_R:
                    lr_vals[i]->i = (long long) (lr_expr[i].expr.constant->b) ? 1 : 0;
                    break;
                default:
                    lr_vals[i]->i = (long long) 0;
                    break;
                }
            }
        }
        else {
            if (lr_expr[i].r_type == Bool_R) {
                lr_vals[i]->b = (long long) (lr_expr[i].expr.constant->b) ? 1 : 0;
            }
            else {
                lr_vals[i]->b = (long long) 0;
            }
        }
    }
    return r_type;
}

Expr_t copy_expr(Expr_t *expr) {
    Expr_t new_expr;// = (Expr_t*) malloc(sizeof(Expr_t));
    new_expr.e_type = expr->e_type;
    new_expr.r_type = expr->r_type;
    switch (expr->e_type) {
    case Constant:
        new_expr.expr.constant = (Constant_t*) malloc(sizeof(Constant_t));
        new_expr.expr.constant->e_type = expr->expr.constant->e_type;
        new_expr.expr.constant->r_type = expr->expr.constant->r_type;
        switch (expr->r_type) {
        case String_R:
            new_expr.expr.constant->str = (char*) malloc(strlen(expr->expr.constant->str));
            strcpy(new_expr.expr.constant->str, expr->expr.constant->str);
            break;
        case Float_R:
            new_expr.expr.constant->f = expr->expr.constant->f;
            break;
        case Bool_R:
            new_expr.expr.constant->b = expr->expr.constant->b;
            break;
        case Int_R:
        default:
            new_expr.expr.constant->i = expr->expr.constant->i;
            break;
        }
        break;
    case Sequence:
    case List:
    case ArgList:
        new_expr.expr.cons = (Cons_t*) malloc(sizeof(Cons_t));
        new_expr.expr.cons->e_type = expr->expr.cons->e_type;
        new_expr.expr.cons->r_type = expr->expr.cons->r_type;
        Cons_t *cons_read = expr->expr.cons, *cons_write = new_expr.expr.cons;
        while (cons_read != NULL) {
            cons_write->head = copy_expr(&cons_read->head);
            if (cons_read->tail != NULL) {
                cons_write->tail = (Cons_t*) malloc(sizeof(Cons_t));
            }
            cons_read = cons_read->tail;
            cons_write = cons_write->tail;
        }
    default:
        new_expr.expr = expr->expr;
        break;
    }
    return new_expr;
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
