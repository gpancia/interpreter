#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "env.h"

#define ENV_LOCAL_OR_GLOBAL(e) ((e == NULL) ? &global_env : e)

void init_env(Env *env)
{
    env = ENV_LOCAL_OR_GLOBAL(env);
    env->head = (Env_Item) {NULL, NULL, NULL, NULL};
    env->tail = &env->head;
}

Expr_t* get_val(Env *env, char *name)
{
    env = ENV_LOCAL_OR_GLOBAL(env);
    Env_Item *curr = env->head.next;
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

int set_val(Env *env, char *name, Expr_t *expr)
{
    env = ENV_LOCAL_OR_GLOBAL(env);
    Env_Item *curr = env->head.next;
    while (curr != NULL) {
        if (curr->name && !strcmp(curr->name, name)) {
            curr->expr = expr;
            return 0;
        }
        else {
            curr = curr->next;
        }
    }
    Env_Item *new_env = (Env_Item *) malloc(sizeof(Env_Item));
    *new_env = (Env_Item) {(char *) malloc(strlen(name)+1),
                      (Expr_t *) malloc(sizeof(Expr_t)),
                      NULL,
                      NULL};
    strcpy(new_env->name, name);
    *new_env->expr = copy_expr(expr, &new_env->ll_loc);
    env->tail->next = new_env;
    env->tail = new_env;
    return 0;
}

void print_env(Env *env) {
    env = ENV_LOCAL_OR_GLOBAL(env);
    Env_Item *curr = env->head.next;
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

void free_env(Env *env) {
    env = ENV_LOCAL_OR_GLOBAL(env);
    Env_Item *curr = env->head.next;
    Env_Item *prev;
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
