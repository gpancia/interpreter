#include "shell.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "error_handling.h"
#include "lexer.h"
#include "parser.h"
#include "evaluator.h"

#define INPUT_SIZE 500

#ifndef SETJMP
#define SETJMP setjmp(jmp_env)
#endif

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
        if (SETJMP) {
            fprintf(stderr, "lexing error\n");
            continue;
        }
        else {
            lex_string(input);
        }

        if (SETJMP) {
            fprintf(stderr, "parsing error\n");
            goto freeing_step;
        }
        else {
            token = NULL;
            expr = parse_expr(&token);
        }

        if (SETJMP) {
            fprintf(stderr, "evaluation error\n");
            goto freeing_step;
        }
        else {
            expr = evaluate_expr(expr, NULL);
        }
        print_expr(expr);
        printf("\n");
    freeing_step:
        tk_lst_free();  // no point in keeping old tokens around 
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
    char temp_str[INPUT_SIZE];
    strcpy(temp_str, str + beginning);
    strcpy(str, temp_str);
}
