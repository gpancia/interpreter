#include "expr.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "flags.h"

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
    Expr_t ret = create_expr(Constant, Int_R, NULL);
    *(ret.expr.constant) = (Constant_t){Constant, Int_R, {.i=i}};
    return ret;
}
Expr_t wrap_flt(double f)
{
    Expr_t ret = create_expr(Constant, Float_R, NULL);
    *(ret.expr.constant) = (Constant_t){Constant, Float_R, {.f=f}};
    // *(ret.expr.constant) = (Constant_t){Constant, Float_R, {*((long long*)&f)}};  // evil bit trick
    return ret;
}
Expr_t wrap_str(char *str)
{
    Expr_t ret = create_expr(Constant, String_R, NULL);
    *(ret.expr.constant) = (Constant_t){Constant, String_R, {.str=(char*)malloc(sizeof(char)*(1+strlen(str)))}};
    sprintf(ret.expr.constant->str, "%s", str);
    return ret;
}

Expr_t wrap_bool(char b)
{
    Expr_t ret = create_expr(Constant, Bool_R, NULL);
    *(ret.expr.constant) = (Constant_t){Constant, Bool_R, {.b = b}};
    return ret;
}

enum result_type consolidate_constant_pair(Expr_t lr_expr[2], Constant_Values *lr_vals[2]) {
    enum result_type r_type;
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



////// Expression creation/deletion

void init_ll_expr() {
    ll_expr_head = (LL_Expr *) malloc(sizeof(LL_Expr));
    *ll_expr_head = (LL_Expr) {NULL, NULL, NULL, NULL};
    ll_expr_tail = ll_expr_head;
}

Expr_t create_expr(enum expr_type e_type, enum result_type r_type, LL_Expr **loc) {
    Expr_t ret = {e_type, r_type, {malloc(e_size[e_type])}};
    ret.expr.generic->e_type = e_type;
    ret.expr.generic->r_type = r_type;
    add_ptr_to_ll((void *) ret.expr.generic, loc);
    return ret;
}

void add_ptr_to_ll(void *ptr, LL_Expr **loc) {
    LL_Expr *new_ptr = (LL_Expr *) malloc(sizeof(LL_Expr));
    *new_ptr = (LL_Expr) {ll_expr_tail, NULL, ptr, loc};
    ll_expr_tail->next = new_ptr;
    ll_expr_tail = new_ptr;

    if (loc != NULL) {
        *loc = new_ptr;
    }
}

Expr_t copy_expr(Expr_t *expr, LL_Expr **loc) {
    if (expr->expr.ptr == NULL) {
        return *expr;
    }
    
    Expr_t new_expr = create_expr(expr->expr.generic->e_type, expr->expr.generic->r_type, loc);
    switch (expr->e_type) {
    case Constant:
        new_expr.expr.constant->e_type = expr->expr.constant->e_type;
        new_expr.expr.constant->r_type = expr->expr.constant->r_type;
        switch (expr->r_type) {
        case String_R:
            new_expr.expr.constant->str = (char *) malloc(strlen(expr->expr.constant->str)+1);
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
        new_expr.expr.cons->e_type = expr->expr.cons->e_type;
        new_expr.expr.cons->r_type = expr->expr.cons->r_type;
        Cons_t *cons_read = expr->expr.cons, *cons_write = new_expr.expr.cons;
        if (FLAGS & VERBOSE_FLAG) {
            printf("Copying %s: %p -> %p\n", e_str[expr->e_type], cons_read, cons_write);
        }
        while (cons_read != NULL) {
            cons_write->head = copy_expr(&cons_read->head, NULL);  // don't propagate loc
            cons_write->e_type = expr->e_type;
            cons_write->r_type = cons_write->head.r_type;
            if (cons_read->tail != NULL) {
                cons_write->tail = (Cons_t*) malloc(sizeof(Cons_t));
                *cons_write->tail = *cons_read->tail;
                cons_write->tail->tail = NULL;
                cons_write = cons_write->tail;
            }
            else {
                cons_write->tail = NULL;
            }
            cons_read = cons_read->tail;
        }
    default:
        new_expr.expr = expr->expr;
        break;
    }
    return new_expr;
}


// Free'ing funcs
int cut_expr(void *ptr) {
    LL_Expr *curr = ll_expr_head->next;
    while (curr != NULL) {
        if (curr->this == ptr) {
            cut_ll_expr(curr);
            return 0;
        }
        else {
            curr = curr->next;
        }
    }
    fprintf(stderr, "expr not found in linked list\n");
    return 1;
}

void cut_ll_expr(LL_Expr *loc) {
    if (loc != NULL) {
        if (loc->prev != NULL) {
            loc->prev->next = loc->next;
        }
        if (loc->next != NULL) {
            loc->next->prev = loc->prev;
        }
        if (loc == ll_expr_tail) {
            ll_expr_tail = ll_expr_tail->prev;
        }
        free_ll_expr(loc);
    }
}

void free_all_expr() {
    LL_Expr *prev, *curr = ll_expr_head->next;
    while (curr != NULL) {
        prev = curr;
        curr = curr->next;
        free_ll_expr(prev);
    }
    free(ll_expr_head);
    if (FLAGS & DEBUG_FLAG) {
        printf("done freeing\n");
    }
}

void free_ll_expr(LL_Expr *ll_expr) {
    if (ll_expr != NULL) {
        if (ll_expr->this != NULL) {
            free_expr_u((Expr_u) {.ptr = ll_expr->this});
        }
        if (ll_expr->loc != NULL) {
            *ll_expr->loc = NULL;
        }
        free(ll_expr);
    }
}

void free_expr(Expr_t expr)
{
    free_expr_u(expr.expr);
}

// free_all_expr should handle all subexpressions here,
// so recursion would double free things
void free_expr_u(Expr_u expr)
{
    if (expr.ptr == NULL)
	return;

    enum expr_type e_type = expr.generic->e_type;
    switch (e_type) {
    case (Set):
    case (Function):
    case (FunctionDef):
    case (Var):
        // All of the above expr types have strings
        // in the same location in their respective structs
        if (expr.set->name != NULL) {
            free(expr.set->name);
        }
        break;
    case (Constant):
        if (expr.constant->r_type == String_R && expr.constant->str != NULL){
            free(expr.constant->str);
	}
        break;
    case (List):
    case (Sequence):
    case (ArgList):
        // freeing the head is uneccessary, as it will be done
        // in free_all_expr
	free_expr_u((Expr_u) {.cons = expr.cons->tail});
        break;
        
    // not using default here in case I'm missing something
    case (Add):
    case (Sub):
    case (Div):
    case (Mul):
    case (Concat):
    case (BExpr):
    case (Conditional):
        break;
    default:
        fprintf(stderr, "unable to free expression of type %s\n", e_str[expr.generic->e_type]);
    }
    free(expr.generic);
}
