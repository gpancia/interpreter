#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "env.h"

#define ENV_LOCAL_OR_GLOBAL(e) ((e == NULL) ? &global_env : e)

void init_env(Env *env, Env *parent)
{
    env = ENV_LOCAL_OR_GLOBAL(env);
    if (env != &global_env && parent == NULL) {
        parent = &global_env;
    }
    env->head = (Env_Item) {NULL, NULL, NULL, NULL, NULL};
    env->tail = &env->head;
    env->parent = parent;
    env->child = NULL;
    env->next = env;
    env->prev = env;
    if (parent != NULL) {
        Env *sibling = parent->child;
        if (sibling != NULL) {
            env->next = sibling->next;
            env->prev = sibling;
            sibling->next->prev = env;
            sibling->next = env;
        }
        parent->child = env;
    }
}

Env_Item * find_env_item(Env **env_ptr, char*name) {
    Env *env = *env_ptr;
    Env_Item *curr = env->head.next;
    while (curr != NULL) {
        if (curr->expr && curr->name && !strcmp(name, curr->name)) {
            return curr;
        }
        else {
            curr = curr->next;
        }
    }
    // recurse through parent scopes until name is found
    if (env->parent != NULL) {
        *env_ptr = env->parent;
        return find_env_item(env_ptr, name);
    }
    return NULL;  // found nothing
}

Expr_t * get_val(Env *env, char *name) {
    env = ENV_LOCAL_OR_GLOBAL(env);
    Env_Item *ei = find_env_item(&env, name);
    return (ei == NULL) ? NULL : ei->expr;
}

int set_val(Env *env, char *name, Expr_t *expr) {
    env = ENV_LOCAL_OR_GLOBAL(env);

    // search for existing variables with the same name
    Env_Item *ei = find_env_item(&env, name);
    if (ei != NULL) {
        // if they exist, delete them and replace them in their original scope
        free_env_item(ei);
    }
    // if they do not, variable will be added to original scope
    
    Env_Item *new_env = (Env_Item *) malloc(sizeof(Env_Item));
    *new_env = (Env_Item) {env->tail, NULL,
                           (char *) malloc(strlen(name)+1),
                           (Expr_t *) malloc(sizeof(Expr_t)),
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

void free_env_item(Env_Item *ei) {
    if (ei->ll_loc != NULL) {
        cut_ll_expr(ei->ll_loc);
    }
    if (ei->name != NULL) {
        free(ei->name);
    }
    if (ei->expr != NULL) {
        free(ei->expr);
    }
    if (ei->prev != NULL) {
        ei->prev->next = ei->next;
    }
    if (ei->next != NULL) {
        ei->next->prev = ei->prev;
    }
    free(ei);
}

void free_env(Env *env) {
    env = ENV_LOCAL_OR_GLOBAL(env);
    if (env->child != NULL) {
        Env *og_env = env->child;
        Env *curr_env = og_env;
        do {
            curr_env = curr_env->next;
            free_env(curr_env);
        } while (curr_env != og_env);
        env->child = NULL;
    }
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
