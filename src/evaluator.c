#include "evaluator.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "env.h"

Expr_t evaluate_expr(Expr_t expr) {
    Expr_t ret = NULL_EXPR;

    switch (expr.e_type) {
    case Add:
    case Sub:
    case Mul:
    case Div:
    case Concat:
        return evaluate_arith(expr);
        
    case Set:
        return evaluate_set(expr);
        
    case BExpr:
        return evaluate_bexpr(expr);
        
    case Conditional:
        return evaluate_cond(expr);
        
    case List:
        return evaluate_list(expr);
        
    case Sequence:
        return evaluate_sequence(expr);
        
    // TODO: implement these
    case ArgList:
    case Function:
    case FunctionDef:
        return NULL_EXPR;
        
    case Var:
        return evaluate_id(expr);
        
    case Constant:
        return expr;
        
    case Generic:
        return NULL_EXPR;
    }
    
    return ret;
}


Expr_t evaluate_arith(Expr_t expr) {
    Expr_t ret = NULL_EXPR, left, right;
    
    enum result_type r_type;

    // evaluate subexpressions so they are both constant
    if (expr.expr.arith->left.e_type != Constant) {
        left = evaluate_expr(expr.expr.arith->left);
    }
    else {
        left = expr.expr.arith->left;
    }
    if (expr.expr.arith->right.e_type != Constant) {
        right = evaluate_expr(expr.expr.arith->right);
    }
    else {
        right = expr.expr.arith->right;
    }

    if (left.e_type != Constant || right.e_type != Constant) {
        fprintf(stderr, "Error with following expression:\n");
        print_expr(expr);
        fprintf(stderr, "which evaluates to:\n");
        print_expr(left);
        print_expr(right);
        exit(1);
    }

    Expr_t lr_expr[2] = {left, right};
    char lr_str[2][MAX_STR_LEN];
    long long lr_int[2];
    double lr_double[2];
    
    // 
    if (expr.expr.arith->e_type == Concat || left.r_type == String_R || right.r_type == String_R) {
        r_type = String_R;
        for (int i = 0; i < 2; i++) {
            switch (lr_expr[i].r_type) {
            case String_R:
                strcpy(lr_str[i], lr_expr[i].expr.constant->str);
                break;
            case Int_R:
                sprintf(lr_str[i], "%lld", lr_expr[i].expr.constant->i);
                break;
            case Float_R:
                sprintf(lr_str[i], "%f", lr_expr[i].expr.constant->f);
                break;
            case Bool_R:
                strcpy(lr_str[i], (lr_expr[i].expr.constant->b) ? "true" : "false");
                break;
            default:
                strcpy(lr_str[i], "undef");
                break;
            }
        }
        
    }
    else if (left.r_type == Float_R || right.r_type == Float_R) {
        r_type = Float_R;
        for (int i = 0; i < 2; i++) {
            switch (lr_expr[i].r_type) {
            case String_R:
                lr_double[i] = INFINITY;
                break;
            case Int_R:
                lr_double[i] = (double) lr_expr[i].expr.constant->i;
                break;
            case Float_R:
                lr_double[i] = (double) lr_expr[i].expr.constant->f;
                break;
            case Bool_R:
                lr_double[i] = (double) (lr_expr[i].expr.constant->b) ? 1.0 : 0.0;
                break;
            default:
                lr_double[i] = (double) 0.0;
                break;
            }
        }
    }
    else {
        r_type = Int_R;
        for (int i = 0; i < 2; i++) {
            switch (lr_expr[i].r_type) {
            case String_R:
                lr_int[i] = (long long) INFINITY;
                break;
            case Int_R:
                lr_int[i] = (long long) lr_expr[i].expr.constant->i;
                break;
            case Float_R:
                lr_int[i] = (long long) lr_expr[i].expr.constant->f;
                break;
            case Bool_R:
                lr_int[i] = (long long) (lr_expr[i].expr.constant->b) ? 1 : 0;
                break;
            default:
                lr_int[i] = (long long) 0;
                break;
            }
        }
    }

    long long ret_int = 0;
    double ret_double = 0;
    char ret_str[MAX_STR_LEN] = "";
    
    switch (expr.expr.arith->e_type) {
    case Add:
        ret_int = lr_int[0] + lr_int[1];
        ret_double = lr_double[0] + lr_double[1];
        break;
    case Sub:
        ret_int = lr_int[0] - lr_int[1];
        ret_double = lr_double[0] - lr_double[1];
        break;
    case Mul:
        ret_int = lr_int[0] * lr_int[1];
        ret_double = lr_double[0] * lr_double[1];
        break;
    case Div:
        ret_int = lr_int[0] / lr_int[1];
        ret_double = lr_double[0] / lr_double[1];
        break;
    case Concat:
        sprintf(ret_str, "%s%s", lr_str[0], lr_str[1]);
        break;
    default:
        fprintf(stderr, "evaluate_arith: following expression of type %s should not be evaluated here\n",
                e_str[expr.expr.arith->e_type]);
        print_expr(expr);
        exit(1);
    }

    ret = create_expr(Constant, r_type);
    switch (r_type) {
    case Int_R:
        ret.expr.constant->i = ret_int;
        break;
    case Float_R:
        ret.expr.constant->f = ret_double;
        break;
    case String_R:
        ret.expr.constant->str = (char*) malloc(strlen(ret_str)+1);
        strcpy(ret.expr.constant->str, ret_str);
        break;
    default:
        fprintf(stderr, "evaluate_arith: invalid return type %s:\n", r_str[r_type]);
        print_expr(expr);
        exit(1);
    }
    
    return ret;
}

Expr_t evaluate_set(Expr_t expr) {
    Expr_t val = evaluate_expr(expr.expr.set->val);
    set_val(expr.expr.set->name, &val);
    return val;
}

Expr_t evaluate_id(Expr_t expr) {
    // print_env();
    Var_t *var = expr.expr.var;
    char *name = var->name;
    Expr_t *val = get_val(name);
    if (val == NULL) {
        fprintf(stderr, "evaluate_id: undeclared variable \"%s\"", name);
        exit(1);
    }
    return *val;
}

Expr_t evaluate_bexpr(Expr_t expr) {
    if (expr.e_type != BExpr) {
        fprintf(stderr, "evaluate_bexpr: invalid expression of type %s:\n", e_str[expr.e_type]);
        print_expr(expr);
        exit(1);
    }
    // Constant_t *ret_c = (Constant_t*) malloc(sizeof(Constant_t));
    // *ret_c = (Constant_t) {Constant, Bool_R, {.b=0}};
    // Expr_t ret = {Constant, Bool_R, {.constant=ret_c}};
    Expr_t ret = create_expr(Constant, Bool_R);
    Constant_t *ret_c = ret.expr.constant;
    BExpr_t *bexpr = expr.expr.bexpr;
    // char left_b, right_b;
    if (bexpr->b_type == Not) {
        char left_b = constant_to_bool(evaluate_expr(bexpr->left));
        ret_c->b = (long long) (left_b == 0);
    }
    else {
        Constant_Values *lr_vals[2] = {malloc(sizeof(Constant_Values)), malloc(sizeof(Constant_Values))};
        Expr_t lr_expr[2] = {evaluate_expr(bexpr->left), evaluate_expr(bexpr->right)};
        enum result_type r_type = consolidate_constant_pair(lr_expr, lr_vals);
        if (r_type == String_R) {
            lr_vals[0]->i = (long long) strlen(lr_vals[0]->str);
            lr_vals[1]->i = (long long) strlen(lr_vals[1]->str);
        }
        else if (r_type == Bool_R) {
            lr_vals[0]->i = (long long) lr_vals[0]->b;
            lr_vals[1]->i = (long long) lr_vals[1]->b;
        }
        else if (r_type == Float_R) {
            lr_vals[0]->i = * (long long*) &lr_vals[0]->f;
            lr_vals[1]->i = * (long long*) &lr_vals[1]->f;
        }

        char is_float = (char) (r_type == Float_R);
        
        switch (bexpr->b_type) {
        case Eql:
            // just check if bits/string are the same
            ret_c->b = (r_type == String_R) ?
                !strcmp(lr_vals[0]->str, lr_vals[1]->str) : lr_vals[0]->i == lr_vals[1]->i;
            break;
        case Nql:
            // !! so that it condenses result into either 1 or 0
            ret_c->b = (r_type == String_R) ?
                !!strcmp(lr_vals[0]->str, lr_vals[1]->str) : lr_vals[0]->i != lr_vals[1]->i;
            break;
        case Leq:
            ret_c->b = (is_float) ? lr_vals[0]->i <= lr_vals[1]->i : lr_vals[0]->f <= lr_vals[1]->f;
            break;
        case Geq:
            ret_c->b = (is_float) ? lr_vals[0]->i >= lr_vals[1]->i : lr_vals[0]->f >= lr_vals[1]->f;
            break;
        case Lsr:
            printf("%lld < %lld = ", lr_vals[0]->i, lr_vals[1]->i);
            ret_c->b = (is_float) ? lr_vals[0]->i < lr_vals[1]->i : lr_vals[0]->f < lr_vals[1]->f;
            printf("%lld\n", ret_c->b);
            break;
        case Grt:
            ret_c->b = (is_float) ? lr_vals[0]->i > lr_vals[1]->i : lr_vals[0]->f > lr_vals[1]->f;
            break;
        case And:
            // doesn't matter if float, just if 0 or not
            ret_c->b = lr_vals[0]->i && lr_vals[1]->i;
            break;
        case Or:
            // doesn't matter if float, just if 0 or not
            ret_c->b = lr_vals[0]->i || lr_vals[1]->i;
            break;
        case Not:
            // should never happen, just here for warning suppression
            break;
        }
        free(lr_vals[0]);
        free(lr_vals[1]);
    }

    return ret;
}

Expr_t evaluate_cond(Expr_t expr) {
    char p = constant_to_bool(evaluate_expr(expr.expr.cond->p));
    return evaluate_expr((p) ? expr.expr.cond->if_true : expr.expr.cond->if_false);
}

Expr_t evaluate_list(Expr_t expr) {
    Cons_t *curr = expr.expr.cons;
    while (curr != NULL) {
        Expr_t ret = evaluate_expr(curr->head);
        // free_expr(curr->head);
        curr->head = ret;
        curr = curr->tail;
    }
    return expr;
}

Expr_t evaluate_sequence(Expr_t expr) {
    Expr_t ret;
    Cons_t *curr = expr.expr.cons;
    while (curr != NULL) {
        ret = evaluate_expr(curr->head);
        curr = curr->tail;
    }
    return ret;    
}

char constant_to_bool(Expr_t expr) {
    if (expr.e_type != Constant) {
        fprintf(stderr, "constant_to_bool: invalid expression of type %s:\n", e_str[expr.e_type]);
        print_expr(expr);
        exit(1);
    }
    void *val = (void *) expr.expr.constant->i;
    switch (expr.expr.constant->r_type) {
    case Int_R:
    case Float_R:
    case Bool_R:
        return (char) (val != 0);
        // return (char) expr.expr.constant->b;
    case String_R:
        return (char) (strlen((char*) val) != 0);
    default:
        fprintf(stderr, "evaluate_arith: invalid return type %s:\n", r_str[expr.expr.constant->r_type]);
        print_expr(expr);
        exit(1);
    }
}
