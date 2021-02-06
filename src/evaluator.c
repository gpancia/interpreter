#include "evaluator.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "error_handling.h"

// #include "env.h"

Expr_t evaluate_expr(Expr_t expr, Env *env) {
    Expr_t ret = NULL_EXPR;

    switch (expr.e_type) {
    case Add:
    case Sub:
    case Mul:
    case Div:
    case Concat:
        return evaluate_arith(expr, env);
        
    case Set:
        return evaluate_set(expr, env);
        
    case BExpr:
        return evaluate_bexpr(expr, env);
        
    case Conditional:
        return evaluate_cond(expr, env);
        
    case List:
        return evaluate_list(expr, env);
        
    case Sequence:
        return evaluate_sequence_local(expr, env);
        
    case ArgList:
        // I don't think this should ever come up here
        fprintf(stderr, "evaluator error: ArgList shouldn't be here\n");
        THROW_ERROR;
    case Function:
        return evaluate_func(expr, env);
    case FunctionDef:
        return evaluate_funcdef(expr, env);
        
    case Var:
        return evaluate_id(expr, env);
        
    case Constant:
        return expr;
        
    case Generic:
        return NULL_EXPR;
    }
    
    return ret;
}


Expr_t evaluate_arith(Expr_t expr, Env *env) {
    Expr_t ret = NULL_EXPR, left, right;
    
    enum result_type r_type;

    // evaluate subexpressions so they are both constant
    left = evaluate_expr(expr.expr.arith->left, env);
    right = evaluate_expr(expr.expr.arith->right, env);

    if (left.e_type != Constant || right.e_type != Constant) {
        fprintf(stderr, "Error with following expression:\n");
        print_expr(expr);
        fprintf(stderr, "which evaluates to:\n");
        print_expr(left);
        fprintf(stderr, ", ");
        print_expr(right);
        fprintf(stderr, "\n");
        THROW_ERROR;
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
        THROW_ERROR;
    }
    
    ret = create_expr(Constant, r_type, NULL);
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
        THROW_ERROR;
    }
    
    return ret;
}

Expr_t evaluate_set(Expr_t expr, Env *env) {
    void *func_ptr = special_func(expr.expr.set->name);
    if (func_ptr != NULL) {
        fprintf(stderr, "cannot redefine \"%s\"\n", expr.expr.set->name);
        THROW_ERROR;
    }
    Expr_t val = evaluate_expr(expr.expr.set->val, env);
    set_val(env, expr.expr.set->name, &val);
    return val;
}

Expr_t evaluate_id(Expr_t expr, Env *env) {
    // print_env();
    Var_t *var = expr.expr.var;
    char *name = var->name;
    void *func_ptr = special_func(name);
    if (func_ptr != NULL) {
        return NULL_EXPR;
    }
    Expr_t *val = get_val(env, name);
    
    if (val == NULL) {
        fprintf(stderr, "evaluate_id: undeclared variable \"%s\"\n", name);
        THROW_ERROR;
    }

    if (!is_null_expr(var->index)) {
        // Can only access strings or lists
        if (val->e_type != List && !(val->e_type == Constant && val->r_type == String_R)) {
            if (val->e_type == Constant) {
                fprintf(stderr, "invalid access of %s type variable '%s'\n", r_str[val->r_type], name);
            }
            else {
                fprintf(stderr, "invalid access of %s type variable '%s'\n", e_str[val->e_type], name);
            }
            THROW_ERROR;
        }

        Expr_t index_expr = evaluate_expr(var->index, env);

        // index must be int constant
        if (index_expr.e_type != Constant || index_expr.r_type != Int_R) {
            fprintf(stderr, "invalid index type %s/%s for variable %s\n", e_str[index_expr.e_type], r_str[index_expr.r_type], name);
            THROW_ERROR;
        }

        int index_signed = index_expr.expr.constant->i;
        uint size = (val->e_type == List) ? val->expr.cons->size : strlen(val->expr.constant->str);
        
        if (index_signed < 0) {  // if index is negative, count from end of list
            index_signed += size;
        }
        uint index = *(uint *) &index_signed;
        if (size < index) {
            fprintf(stderr, "index too large for variable '%s' of len %u\n", name, size);
            THROW_ERROR;
        }

        if (val->e_type == List) {
            Cons_t *cons = val->expr.cons;
            for (uint i = 0; i < index; i++) {
                cons = cons->tail;
            }
            return cons->head;
        }
        else {
            return wrap_char(val->expr.constant->str[index]);
        }
        
    }
    
    return *val;
}

Expr_t evaluate_bexpr(Expr_t expr, Env *env) {
    if (expr.e_type != BExpr) {
        fprintf(stderr, "evaluate_bexpr: invalid expression of type %s:\n", e_str[expr.e_type]);
        print_expr(expr);
        THROW_ERROR;
    }
    // Constant_t *ret_c = (Constant_t*) malloc(sizeof(Constant_t));
    // *ret_c = (Constant_t) {Constant, Bool_R, {.b=0}};
    // Expr_t ret = {Constant, Bool_R, {.constant=ret_c}};
    Expr_t ret = create_expr(Constant, Bool_R, NULL);
    Constant_t *ret_c = ret.expr.constant;
    BExpr_t *bexpr = expr.expr.bexpr;
    // char left_b, right_b;
    if (bexpr->b_type == Not) {
        char left_b = constant_to_bool(evaluate_expr(bexpr->left, env));
        ret_c->b = (long long) (left_b == 0);
    }
    else {
        Constant_Values *lr_vals[2] = {malloc(sizeof(Constant_Values)), malloc(sizeof(Constant_Values))};
        Expr_t lr_expr[2] = {evaluate_expr(bexpr->left, env), evaluate_expr(bexpr->right, env)};
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

Expr_t evaluate_cond(Expr_t expr, Env *env) {
    char p = constant_to_bool(evaluate_expr(expr.expr.cond->p, env));
    return evaluate_expr((p) ? expr.expr.cond->if_true : expr.expr.cond->if_false, env);
}

Expr_t evaluate_list(Expr_t expr, Env *env) {
    Cons_t *curr = expr.expr.cons;
    while (curr != NULL) {
        Expr_t ret = evaluate_expr(curr->head, env);
        curr->head = ret;
        curr = curr->tail;
    }
    return expr;
}

Expr_t evaluate_sequence_local(Expr_t expr, Env *env) {

    Env *local_env;
    if (env == NULL) {
        local_env = &global_env;
    }
    else {
        local_env = (Env *) malloc(sizeof(Env));
    }
    init_env(local_env, env);
    
    Expr_t ret = evaluate_sequence(expr, local_env);
    
    if (local_env != &global_env) {
        free_env(local_env);
        free(local_env);
    }
    return ret;
}

Expr_t evaluate_sequence(Expr_t expr, Env *env) {
    env = (env == NULL) ? &global_env : env;
    Expr_t ret;
    Cons_t *curr = expr.expr.cons;
    while (curr != NULL) {
        ret = evaluate_expr(curr->head, env);
        curr = curr->tail;
    }
    return ret;    
}


Expr_t evaluate_funcdef(Expr_t expr, Env *env) {
    FuncDef_t *func_def = expr.expr.func_def;
    void *func_ptr = special_func(func_def->name);
    if (func_ptr != NULL) {
        fprintf(stderr, "cannot redefine \"%s\"\n", func_def->name);
        THROW_ERROR;
    }
    // // check if all args are valid (i.e.: empty variable names) (is this necessary? no)
    // if (!is_null_expr(expr.expr.func_def->args)) {
    //     Cons_t *curr = func_def->args.expr.cons;
    //     uint size = curr->size;
    //     for (uint i = 0; i < size; i++) {
    //         if (curr->head.e_type != Var || get_val(env, curr->head.expr.var->name) != NULL) {
    //             fprintf(stderr,
    //                     "evaluator error: argument %s of function %s is already defined as a variable elsewhere\n",
    //                     curr->head.expr.var->name, func_def->name);
    //             THROW_ERROR;
    //         }
    //     }
    // }

    set_val(env, func_def->name, &expr);
    return NULL_EXPR;  // hopefully this doesn't trigger any errors
}

Expr_t evaluate_func(Expr_t expr, Env *env) {
    Expr_t *func_def_expr = get_val(env, expr.expr.func->name);
    Expr_t (*func_ptr)(Expr_t, Env *) = special_func(expr.expr.func->name);
    if (func_ptr != NULL) {
        return func_ptr(expr, env);
    }
    else if (func_def_expr == NULL) {
        fprintf(stderr, "error: function %s not defined\n", expr.expr.func->name);
        THROW_ERROR;
    }
    
    FuncDef_t *func_def = func_def_expr->expr.func_def;
    Func_t *func = expr.expr.func;
    Cons_t *arg_names = func_def->args;
    Cons_t *arg_vals = func->args;

    // check for argument number consistency
    if (arg_names->size != arg_vals->size) {
        fprintf(stderr, "error:%s: passed %u arguments, expected %u\n",
                func->name, arg_vals->size, arg_names->size);
        THROW_ERROR;
    }

    // create local environment with arguments
    Env *local_env = malloc(sizeof(Env));
    init_env(local_env, env);
    // for (uint i = 0; i < arg_names->size; i++) {
    while (arg_names != NULL && arg_vals != NULL) {
        // arguments set here will have priority over variables set in parent scopes
        Expr_t a_val = evaluate_expr(arg_vals->head, env);
        add_val(local_env, arg_names->head.expr.var->name, &a_val);
        arg_names = arg_names->tail;
        arg_vals = arg_vals->tail;
    }
    if ((arg_names == NULL && arg_vals != NULL) || (arg_vals == NULL && arg_names != NULL)) {
        fprintf(stderr, "evaluator error: mismatched arg_names and arg_vals");
    }
    
    Expr_t ret = evaluate_sequence(func_def->app, local_env);

    free_env(local_env);
    free(local_env);
    
    return ret;    
}



Expr_t evaluate_print(Expr_t expr, Env *env) {
    Cons_t *args = expr.expr.func->args;
    Expr_t val;
    uint num_args = args->size;
    if (num_args == 0) {
        val = NULL_EXPR;
    }
    else if (num_args == 1){
        val = evaluate_expr(args->head, env);
    }
    else {
        printf("(");
        for (uint i = 0; i < num_args-1; i++) {
            print_expr_ret(evaluate_expr(args->head, env));
            printf(", ");
            args = args->tail;
        }
        print_expr_ret(evaluate_expr(args->head, env));
        printf(")\n");
        return NULL_EXPR;
    }
    
    print_expr_ret(val);  // only reached if num_args < 2
    printf("\n");
    return NULL_EXPR;
}


void * special_func(char *name) {
    void *func_ptr = NULL;
    if (!strcmp(name, "print")) {
        func_ptr = &evaluate_print;
    }

    return func_ptr;
}


char constant_to_bool(Expr_t expr) {
    if (expr.e_type != Constant) {
        fprintf(stderr, "constant_to_bool: invalid expression of type %s:\n", e_str[expr.e_type]);
        print_expr(expr);
        THROW_ERROR;
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
        THROW_ERROR;
    }
}

Expr_t expr_array_to_sequence(Expr_t *arr, int size) {
    enum result_type r_type = arr[size-1].r_type;
    Expr_t ret = create_expr(Sequence, r_type, NULL);
    Cons_t *curr = ret.expr.cons;
    for (int i = 0; i < size-1; i++) {
        *curr = (Cons_t) {Sequence, r_type, arr[i], malloc(sizeof(Cons_t)), size-i};
        curr = curr->tail;
    }
    *curr = (Cons_t) {Sequence, r_type, arr[size-1], NULL, 1};
    return ret;
}
