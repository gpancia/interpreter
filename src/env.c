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
    Env *curr = &env_head;
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
    Env *curr = &env_head;
    while (curr->next) {
        if (curr->name && !strcmp(curr->name, name)) {
            if (curr->expr->r_type != expr->r_type) {
                fprintf(stderr, "Unable to assign value of type %s to variable \"%s\" of type %s",
                        r_str[curr->expr->r_type], name, r_str[expr->r_type]);
                return 1;
            }
            else {
            }
            curr = curr->next;
        }
    }
    return 0;
}
