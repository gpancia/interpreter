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

Expr_t parse_cond(Token**); 

Expr_t parse_parens(Token**); 
Expr_t parse_sequence(Token**);

Expr_t parse_constant(Token**); 

Token *skip_nest(Token **, enum token_type open, enum token_type close);

void type_infer(enum result_type); // TODO

#endif
