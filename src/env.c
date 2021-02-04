#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "env.h"

void init_env()
{
    env_head = (Env) {NULL, NULL, NULL, NULL};
    env_tail = &env_head;
}

Expr_t* get_val(char *name)
{
    Env *curr = env_head.next;
    while (curr != NULL) {
        if (curr->expr && curr->name && !strcmp(name, curr->name)) {
            return curr->expr;
        }
        else {
            curr = curr->next;
        }
    }
    return NULL;  // found nothing
}

int set_val(char *name, Expr_t *expr)
{
    Env *curr = env_head.next;
    while (curr != NULL) {
        if (curr->name && !strcmp(curr->name, name)) {
            curr->expr = expr;
            return 0;
        }
        else {
            curr = curr->next;
        }
    }
    Env *new_env = (Env *) malloc(sizeof(Env));
    *new_env = (Env) {(char *) malloc(strlen(name)+1),
                      (Expr_t *) malloc(sizeof(Expr_t)),
                      NULL,
                      NULL};
    strcpy(new_env->name, name);
    *new_env->expr = copy_expr(expr, &new_env->ll_loc);
    env_tail->next = new_env;
    env_tail = new_env;
    return 0;
}

void print_env() {
    Env *curr = env_head.next;
    printf("Environment:\n");
    while (curr != NULL) {
        printf("\t%s: ", curr->name);
        if (curr->expr != NULL) {
            print_expr(*curr->expr);
            printf("\n");
        }
        curr = curr->next;
    }
    printf("\n");
}

void free_env() {
    Env *curr = env_head.next;
    Env *prev;
    while (curr != NULL) {
        if (curr->ll_loc != NULL) {
            cut_ll_expr(curr->ll_loc);
        }
        if (curr->name != NULL) {
            free(curr->name);
        }
        if (curr->expr != NULL) {
            free(curr->expr);
        }
        prev = curr;
        curr = curr->next;
        free(prev);
    }
}
