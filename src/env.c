#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "env.h"

void init_env()
{
    env_head.name = NULL;
    env_head.expr = NULL;
    env_head.next = NULL;
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
            // if (curr->expr->r_type != expr->r_type) {
            //     fprintf(stderr, "Unable to assign value of type %s to variable \"%s\" of type %s",
            //             r_str[curr->expr->r_type], name, r_str[expr->r_type]);
            //     exit(1);
            // }
            // else {
                
            // }
            curr->expr = expr;
            return 0;
        }
        else {
            curr = curr->next;
        }
    }
    Env *new_env = (Env*) malloc(sizeof(Env));
    new_env->name = (char*) malloc(strlen(name));
    strcpy(new_env->name, name);
    new_env->expr = expr;
    new_env->next = NULL;
    env_tail->next = new_env;
    env_tail = new_env;
    return 0;
}

void print_env() {
    Env *curr = env_head.next;
    printf("Environment:\n");
    while (curr != NULL) {
        printf("\t%s: ", curr->name);
        print_expr(*curr->expr);
        curr = curr->next;
    }
    printf("\n");
}
