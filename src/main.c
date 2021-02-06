#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "flags.h"
#include "lexer.h"
#include "lexer_utils.h"
#include "parser.h"
#include "env.h"
#include "evaluator.h"
#include "error_handling.h"
#include "shell.h"

#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

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
                   sizeof(Func_t), 
                   sizeof(FuncDef_t),
                   sizeof(Var_t),
                   sizeof(Constant_t)};

char FLAGS = 0;

jmp_buf main_jmp_env;

int main(int argc, char *argv[])
{
    char fp[200];
    char use_shell = 1;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-f") && argc > i+1) {
            sprintf(fp, "%s/%s", ROOT_DIR, argv[i+1]);
            use_shell = 0;
        }
        else if (!strcmp(argv[i], "-t") || !strcmp(argv[i], "-F")) {
            sprintf(fp, "%s/test/test.txt", ROOT_DIR);
            use_shell = 0;
        }
    }
    
    FLAGS = parse_flags(argc, argv);
    
    update_interrupts();
    build_interrupts();
    
    init_ll_expr();
    init_env(NULL, NULL);

    if (use_shell) {
        shell();
    }
    else {
        tk_lst_init();
        if (setjmp(main_jmp_env)) {
            fprintf(stderr, "failed to parse tokens\n");
            goto freeing_step;
        }
        else {
            lex_file(fp);
            if (FLAGS & VERBOSE_FLAG) {
                Token *curr = tk_lst.head.next;
                while (curr != NULL){
                    printf("[%s", t_str[curr->tk]);
                    if (curr->val == NULL)
                        printf("]\n");
                    else
                        printf(" : %s]\n", curr->val);
                    curr = curr->next;
                }
            }
            printf("--------DONE LEXING--------\n\n");
        }

        int num_expr = 0;
        Expr_t expr_arr[200];
        Token *token;
        while (tk_lst.head.next != NULL) {
            if (setjmp(main_jmp_env)) {
                fprintf(stderr, "failed to parse expression\n");
            }
            else {
                token = NULL;  // forces parse_expr to pop a new line off tk_lst
                expr_arr[num_expr] = parse_expr(&token);
                if (is_null_expr(expr_arr[num_expr])) { break; }
                print_expr(expr_arr[num_expr++]);
                printf("\n");
            }
        }
        printf("\n --------------DONE PARSING-------------\n");

#define EVAL_EXPRS true
        if (EVAL_EXPRS) {
            Expr_t main_seq = expr_array_to_sequence(expr_arr, num_expr);
            // print_expr(main_seq);
            if (setjmp(main_jmp_env)) {
                fprintf(stderr, "failed to evaluate expression\n");
                goto freeing_step;
            }
            else {
                Expr_t ret_main = evaluate_expr(main_seq, NULL);
                print_expr(ret_main);
                printf("\n");
                print_env(NULL);
                printf("\n");
            }
        }
        tk_lst_free();
    }
 freeing_step:
    free_env(NULL);
    free_all_expr();
    return 0;
}
