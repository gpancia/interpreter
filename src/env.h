#ifndef __ENV_H__
#define __ENV_H__

#include "expr.h"

typedef struct Env_Item {
    char *name;
    Expr_t *expr;
    LL_Expr *ll_loc;
    struct Env_Item *next;
} Env_Item;

typedef struct Env {
    Env_Item head;
    Env_Item *tail;
} Env;

Env global_env;

void init_env(Env *);

// returns NULL if not found
Expr_t* get_val(Env *, char *);

// returns 1 if error when setting val
int set_val(Env *, char *, Expr_t *);

void print_env(Env *);

void free_env(Env *);

#endif
