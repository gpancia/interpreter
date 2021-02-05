#ifndef __EXPR_H__
#define __EXPR_H__

#include <stdlib.h>

#define NULL_EXPR (Expr_t){Generic, Undef_R, {NULL}}
#define MAX_STR_LEN 500

enum expr_type {Add = 0, Sub = 1, Mul = 2, Div = 3, Concat = 4,
                Generic,
                Set,
                BExpr,
                Conditional,
                List, Sequence, ArgList, 
                Function,
                FunctionDef,
                Var,
                Constant};

enum result_type {Int_R = 0, Float_R = 1, String_R = 2, Bool_R = 3, Undef_R = -1};
// `Or` must stay last, with no explicit numbering,
// otherwise logic in `parse_op_binary` will break:
enum bexpr_type {Eql, Nql, Geq, Leq, Grt, Lsr, Not, And, Or};

// enum name arrays for easy printing:
extern char *e_str[];
extern char *r_str[];
extern char *b_str[];

// expr memory size array
extern size_t e_size[];

typedef union Expr_u{
    void *ptr;
    struct Expr_t *expr; 
    struct Generic_t *generic; // Generic (should only be used for NULL_EXPR)
    struct Set_t *set; // Set
    struct BExpr_t *bexpr; // BExpr
    struct Cond_t *cond; // Conditional
    struct Cons_t *cons; // List, Sequence, ArgList
    struct Arith_t *arith; // Add, Sub, Mul, Div
    struct Func_t *func; // Function
    struct FuncDef_t *func_def; // FunctionDef
    struct Var_t *var; // Var
    struct Constant_t *constant; // Constant
}Expr_u;

typedef struct Expr_t{
    enum expr_type e_type;
    enum result_type r_type;
    Expr_u expr;
}Expr_t;

typedef struct Generic_t{
    enum expr_type e_type;
    enum result_type r_type;
    void *ptr;
}Generic_t;

typedef struct Set_t{
    enum expr_type e_type;
    enum result_type r_type;
    char *name;
    Expr_t val;
}Set_t;

typedef struct BExpr_t{
    enum expr_type e_type;
    enum bexpr_type b_type;
    Expr_t left;
    Expr_t right;
}BExpr_t;

typedef struct Cond_t{
    enum expr_type e_type;
    enum result_type r_type;
    Expr_t p;
    Expr_t if_true;
    Expr_t if_false;
}Cond_t;

typedef struct Cons_t{
    enum expr_type e_type;
    enum result_type r_type;
    Expr_t head;
    struct Cons_t *tail;
    uint size;
}Cons_t;

typedef struct Arith_t{
    enum expr_type e_type;
    enum result_type r_type;
    Expr_t left;
    Expr_t right;
}Arith_t;

typedef struct Func_t{
    enum expr_type e_type;
    enum result_type r_type;
    char *name;
    Cons_t *args;
}Func_t;

typedef struct FuncDef_t{
    enum expr_type e_type;
    enum result_type r_type;
    char *name;
    Cons_t *args;
    Expr_t app;
}FuncDef_t;

typedef struct Var_t{
    enum expr_type e_type;
    enum result_type r_type;
    char *name;
}Var_t;

typedef struct Constant_t{
    enum expr_type e_type;
    enum result_type r_type;
    union {
        long long i;
        double f;
        char *str;
        long long b;
    };
}Constant_t;


int is_null_expr(Expr_t);

void print_expr(Expr_t);

// Wrappers
Expr_t wrap_expr_u(Expr_u);
Expr_t wrap_cons(Cons_t*);
Expr_t wrap_int(long long);
Expr_t wrap_flt(double);
Expr_t wrap_str(char*);
Expr_t wrap_bool(char);

typedef struct Constant_Values {
    long long i;
    double f;
    char str[MAX_STR_LEN];
    long long b;
}Constant_Values;

// consolidates the types of the constant pair in priority order String -> Float -> Int -> Bool
enum result_type consolidate_constant_pair(Expr_t lr_expr[2], Constant_Values *lr_vals[2]);

// Linked list of all created Expr_u pointers for freeing purposes
typedef struct LL_Expr {
    struct LL_Expr *prev, *next;
    void *this;
    struct LL_Expr **loc;  // used in env
}LL_Expr;

LL_Expr *ll_expr_head;
LL_Expr *ll_expr_tail;

void init_ll_expr();

// *loc is the pointer to the LL_Expr struct which is set if loc != NULL
Expr_t create_expr(enum expr_type, enum result_type, LL_Expr **loc);
void add_ptr_to_ll(void *ptr, LL_Expr **loc);
Expr_t copy_expr(Expr_t *expr, LL_Expr **loc);

int cut_expr(void *ptr);  // return 1 if expr not found in list
void cut_ll_expr(LL_Expr *loc);  // cut_expr if LL_Expr address is known
void free_all_expr();
void free_ll_expr(LL_Expr *ll_expr);
void free_expr(Expr_t);
void free_expr_u(Expr_u);

#define ZERO_CONSTANT wrap_int(0)

#endif
