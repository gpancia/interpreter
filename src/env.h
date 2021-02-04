#ifndef __ENV_H__
#define __ENV_H__

#include "expr.h"

typedef struct Env_Item {
    struct Env_Item *prev, *next;
    char *name;
    Expr_t *expr;
    LL_Expr *ll_loc;
} Env_Item;


// recursive tree of circular doubly-linked lists
typedef struct Env {
    Env_Item head;
    Env_Item *tail;
    struct Env *parent;  // NULL if global
    struct Env *prev, *next;  // circular
    struct Env *child;  // last to be added
} Env;

Env global_env;

void init_env(Env *env, Env *parent);

// finds Env_Item from current Env and all parent Envs, NULL if not found;
// sets *env_ptr to environment in which name was found
Env_Item *find_env_item(Env **env_ptr, char *name);

// returns NULL if not found
Expr_t* get_val(Env *, char *);

// returns 1 if error when setting val
int set_val(Env *, char *, Expr_t *);

void print_env(Env *);

void free_env_item(Env_Item *);
void free_env(Env *);  // recursively frees env and every child as to avoid orphans

#endif
