#ifndef __PARSER_H__
#define __PARSER_H__ 

#include "tk_line.h"
#include "expr.h"

Expr_t parse_expr(Token**);

Expr_t parse_id(Token**); // mallocs expr, name

Expr_t parse_op_unary(char*, Expr_t, int line_number); 
Expr_t parse_op_binary(char* op, Expr_t left, Expr_t right, int line_number); 

Expr_t parse_set(Token**);
Expr_t parse_set_var(Token**); 
Expr_t parse_set_func(Token**); 
char*  get_name(Token**);

Expr_t parse_cond(Token **token_ptr, Token *pred, Token *cond_true, Token *cond_false); 

Expr_t parse_parens(Token**);
Expr_t parse_list(Token**);
Expr_t parse_sequence(Token**);

Expr_t parse_constant(Token**); 

uint get_cons_size(Cons_t *);
// Makes sure that everything nested between open and close is linked, even if
// in different lines.
// Returns the last token not equal to close.
Token *skip_nest(Token **, enum token_type open, enum token_type close);

void type_infer(enum result_type); // TODO

#endif
