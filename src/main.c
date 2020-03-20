#include <stdlib.h>
#include <stdio.h>
#include "lexer.h"
#include "lexer_utils.h"
#include "parser.h"

char *t_str[] = {"Int", "Float", "Str",
                 "Bool", "Char",
                 "Cond", "Id", "Oper",
                 "OParens", "CParens",
                 "OBrace", "CBrace",
                 "OBracket", "CBracket", "Comma",
                 "Newline", "Null"};
char *e_str[] = {"Add", "Sub", "Mul", "Div", "Concat","Generic", "Set",
                 "BExpr", "Conditional", "List", "Sequence", "ArgList",
                 "Function", "FunctionDef", "Var", "Constant"};

char *r_str[] = {"Undef_R", "Int_R", "Float_R", "String_R", "Bool_R"};

char *b_str[] = {"Eql", "Nql", "Geq", "Leq", "Grt", "Lsr", "Not", "And", "Or"};

int main(int argc, char *argv[])
{
  update_interrupts();
  print_all_tokens2(argc, argv);
  Expr_t expr;
  Token *token;
  while (tk_lst.head.next != NULL) {
    token = NULL;  // forces parse_expr to pop a new line off tk_lst
    expr = parse_expr(&token);
    if (is_null_expr(expr)) { break; }
    print_expr(expr);
    printf("\n");
  }
  return 0;
}
