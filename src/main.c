#include <stdlib.h>
#include <stdio.h>
#include "lexer.h"
#include "lexer_utils.h"
#include "parser.h"

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
