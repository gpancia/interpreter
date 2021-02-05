#ifndef __ENV_H__
#define __ENV_H__

#include "expr.h"

typedef struct Env_Item {
    struct Env_Item *prev, *next;
    char *name;
    Expr_t *expr;
    LL_Expr *ll_loc;
} Env_Item;


// Tree of circular doubly-linked lists
typedef struct Env {
    Env_Item head;
    Env_Item *tail;
    struct Env *parent;  // NULL if global
    struct Env *prev, *next;  // circular
    struct Env *child;  // last to be added
} Env;

Env global_env;

void init_env(Env *env, Env *parent);

// Finds Env_Item from current Env and all parent Envs, NULL if not found;
// Sets *env_ptr to environment in which name was found.
// Priority is given to local envs.
Env_Item *find_env_item(Env **env_ptr, char *name);

// returns NULL if not found
Expr_t* get_val(Env *, char *);

// Either adds or updates a value in local env or parent envs, with priority
// to local envs.
int set_val(Env *, char *, Expr_t *);

// Like set_env but only adds values to local env, never updating them.
// Used to set values for function arguments.
// Should only be used in newly created envs, otherwise there is a risk that
// duplicate values are inserted into the same env.
int add_val(Env *, char *, Expr_t *);  


void print_env(Env *);

void free_env_item(Env_Item *);
void free_env(Env *);  // recursively frees env and every child as to avoid orphans

#endif
