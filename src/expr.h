#ifndef __EXPR_H__
#define __EXPR_H__

#define NULL_EXPR (Expr_t){Generic, Undef_R, {NULL}}
#define ZERO_CONSTANT (Expr_t){Constant, Int_R, {NULL}}

enum expr_type {Add = 0, Sub = 1, Mul = 2, Div = 3,
				Concat = 4,
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

typedef union Expr_u{
  struct Expr_t *expr; 
  struct Generic_t *generic; // Generic (should only be used for NULL_EXPR)
  struct Set_t *set; // Set
  struct BExpr_t *bexpr; // BExpr
  struct Cond_t *cond; // Conditional
  struct Cons_t *cons; // List, Sequence, ArgList
  struct Arith_t *arith; // Add, Sub, Mul, Div
  struct Fun_t *func; // Function
  struct FunDef_t *func_def; // FunctionDef
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
  unsigned long left;
  unsigned long right;
}Generic_t;

typedef struct Set_t{
  enum expr_type e_type;
  enum result_type r_type;
  Expr_t val;
  char *name;
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
}Cons_t;

typedef struct Arith_t{
  enum expr_type e_type;
  enum result_type r_type;
  Expr_t left;
  Expr_t right;
}Arith_t;

typedef struct Fun_t{
  enum expr_type e_type;
  enum result_type r_type;
  char *name;
  Cons_t *args;
}Fun_t;

typedef struct FunDef_t{
  enum expr_type e_type;
  enum result_type r_type;
  char *name;
  Cons_t *args;
  Expr_t app;
}FunDef_t;

typedef struct Var_t{
  enum expr_type e_type;
  enum result_type r_type;
  char *name;
}Var_t;

typedef struct Constant_t{
  enum expr_type e_type;
  enum result_type r_type;
  union {
    long int i;
    double f;
    char *str;
    char b;
  };
}Constant_t;

int is_null_expr(Expr_t);

void print_expr(Expr_t);

Expr_t wrap_int(int);
Expr_t wrap_flt(double);
Expr_t wrap_str(char*);
Expr_t wrap_bool(char);

void free_expr(Expr_t);
void free_expr_u(Expr_u, enum expr_type);

#endif
