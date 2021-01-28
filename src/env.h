#ifndef __ENV_H__
#define __ENV_H__

#include "expr.h"

typedef struct Env {
    char *name;
    Expr_t *expr;
    struct Env *next;
} Env;

Env env_head;
Env *env_tail;

void init_env();

// returns NULL if not found
Expr_t* get_val(char*);

// returns 1 if error when setting val
int set_val(char*, Expr_t*);

#endif
