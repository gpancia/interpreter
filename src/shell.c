#include "shell.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "error_handling.h"
#include "lexer.h"
#include "parser.h"
#include "evaluator.h"

#define INPUT_SIZE 500

void shell() {
    char input[INPUT_SIZE];
    Token *token;
    Expr_t expr;
    while (1) {
        printf("$ ");
        fgets(input, INPUT_SIZE, stdin);
        trim_str(input);
        if (!strcmp(input, "quit") || !strcmp(input, "q")) {
            break;
        }
        tk_lst_init();
        lex_string(input);
        token = NULL;
        expr = parse_expr(&token);
        expr = evaluate_expr(expr, NULL);
        print_expr(expr);
        printf("\n");
        tk_lst_free();
    }
}

void trim_str(char *str) {
    int len = strlen(str);
    for (int i = len - 1; i >= 0; i--) {
        switch (str[i]) {
        case '\n':
        case ' ':
        case '\t':
            str[i] = 0;
            break;
        default:
            i = -1;
            break;
        }
    }
    int beginning = 0;
    for (int i = 0; i < len; i++) {
        switch (str[i]) {
        case '\n':
        case ' ':
        case '\t':
            break;
        default:
            beginning = i;
            i = len;
            break;
        }
    }
    sprintf(str, "%s", &str[beginning]);
}
