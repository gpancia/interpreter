#include <stdlib.h>
#include <stdio.h>
#include "flags.h"
#include "lexer.h"
#include "lexer_utils.h"
#include "parser.h"
#include "env.h"
#include "evaluator.h"

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

size_t e_size[] = {sizeof(Arith_t), sizeof(Arith_t), sizeof(Arith_t), sizeof(Arith_t), sizeof(Arith_t),
                   sizeof(Generic_t),
                   sizeof(Set_t),
                   sizeof(BExpr_t),
                   sizeof(Cond_t),
                   sizeof(Cons_t), sizeof(Cons_t), sizeof(Cons_t),
                   sizeof(Fun_t), 
                   sizeof(FunDef_t),
                   sizeof(Var_t),
                   sizeof(Constant_t)};

char FLAGS = 0;

int main(int argc, char *argv[])
{
    FLAGS = parse_flags(argc, argv);

    update_interrupts();

    init_ll_expr();
    
    print_all_tokens2(argc, argv);

    int num_expr = 0;
    Expr_t expr_arr[200];
    Token *token;
    while (tk_lst.head.next != NULL) {
        token = NULL;  // forces parse_expr to pop a new line off tk_lst
        expr_arr[num_expr] = parse_expr(&token);
        if (is_null_expr(expr_arr[num_expr])) { break; }
        print_expr(expr_arr[num_expr++]);
        printf("\n");
    }
    printf("\n --------------DONE PARSING-------------\n");
    Expr_t main_seq = expr_array_to_sequence(expr_arr, num_expr);
    // print_expr(main_seq);
    Expr_t ret_main = evaluate_expr(main_seq, NULL);
    print_expr(ret_main);
    printf("\n");
    // init_env(NULL, NULL);
    // for (int i = 0; i < num_expr; i++) {
        // print_expr(evaluate_expr(expr_arr[i]));
        // printf("\n");
    // }
    print_env(NULL);
    printf("\n");
    // free_env(NULL);
    free_all_expr();
    tk_lst_free();
    return 0;
}
