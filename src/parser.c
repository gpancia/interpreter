#include "parser.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tk_line.h"
#include "lexer.h"
#include "error_handling.h"

#define PERR_FL(str) fprintf(stderr, "%s:%d, %s:%d: error: %s\n", file_name, (*token_ptr)->line_num, __func__,__LINE__, str);THROW_MAIN
#define PERR(str) fprintf(stderr, "%s:%s: error: %s\n", file_name, __func__, str);THROW_MAIN
#define PWAR_FL(str) fprintf(stderr, "%s:%s:%d: warning: %s\n", file_name, __func__, token->line_num, str)
#define PWAR(str) fprintf(stderr, "%s:%s: warning: %s\n", file_name, __func__, str)

#define NXT_TK token = token->next

Expr_t parse_expr(Token **token_ptr)
{
    Expr_t ret = NULL_EXPR;
    Token *token = *token_ptr;
    // start with a specific token if provided, otherwise pop line off tk_lst
    if (token == NULL) {
        token = pop_tk_line();
    }
    
    // find top-most operator in AST and its operand(s). If none, parse expression normally
    Token *left = token, *highest_op_tk, *right;

    // for conditionals: 
    Token *pred = NULL;
    Token *cond_tf[2] = {NULL, NULL};
    enum ordering {divmul, addsub, bexpr, set, null=-1}; // ordering is lowest to highest in AST, e.g.: + is higher than *
    enum ordering highest_op = null;
    
    while (token != NULL){
        if (token->tk < 5 || token->tk == Id) { // skip constants and binds for now
            NXT_TK;
        }
        else if (token->tk == Newline){
            break;
        }
        else if (token->tk == Cond){
            if (token->next == NULL){
                token->next = pop_tk_line();
            }
            NXT_TK;
            pred = token;
            // skip_nest checks for anything wrong with the layout of the if statement
            skip_nest(&token, OParens, CParens); // skip over predicate
            NXT_TK;
            // skip over if_true and if_false
            for (int i = 0; i < 2; i++) {
                if (token->tk != OBrace) {
                    if (token->tk == Newline) {
                        NXT_TK;
                        if (token == NULL) {
                            token = pop_tk_line();
                        }
                    }
                    else {
                        fprintf(stderr, "Unexpected token %s\n", t_str[token->tk]);
                        PERR_FL("Bad if-statement format");
                    }
                }
                cond_tf[i] = token;
                skip_nest(&token, OBrace, CBrace);
                NXT_TK;
            }
        }
        else if (token->tk == Oper){
            char *val = token->val;
            enum ordering op;
            if (seq(val, "+") || seq(val, "-"))
                op = addsub;
            else if (seq(val, "*") || seq(val, "/"))
                op = divmul;
            else if (seq(val, "="))
                op = set;
            else
                op = bexpr;
            if (op > highest_op){
                highest_op = op;
                highest_op_tk = token;
                right = token->next;
            }
            NXT_TK;
        }
        else if (token->tk == OBrace) {
            skip_nest(&token, OBrace, CBrace);
            NXT_TK;
        }
        else if (token->tk == OParens) {
            skip_nest(&token, OParens, CParens);
            NXT_TK;
        }
        else if (token->tk == OBracket) {
            skip_nest(&token, OBracket, CBracket);
            NXT_TK;
        }
        else if (token->tk == CParens || token->tk == CBrace || token->tk == CBracket) {
            PERR_FL("mismatched closing brace or paren");
        }
        else if (token->tk == Comma){
            PERR_FL("unexpected comma");
        }
        else {
            char err[1000];
            sprintf(err, "unexpected token_type %d, debug this pls", token->tk);
            PERR_FL(err);
        }
    }
    if (highest_op < 0){
        if (left->tk == Newline) {
            ret = NULL_EXPR;
        }
        else if (left->tk < 5) {
            ret = parse_constant(&left);
        }
        else if (left->tk == Cond) {
            ret = parse_cond(&left, pred, cond_tf[0], cond_tf[1]);
        }
        else if (left->tk == Id) {
            ret = parse_id(&left);
        }
        else if (left->tk == OParens) {
            ret = parse_parens(&left);
        }
        else if (left->tk == OBrace) {
            ret = parse_sequence(&left);
        }
        else if (left->tk == OBracket) {
            ret = parse_list(&left);
        }
        else {
            char err[100];
            sprintf(err, "debug this, unexpected token %d", left->tk); // shouldn't happen
            *token_ptr = token;
            PERR_FL(err);
        }
    }
    else {
        if (highest_op_tk == NULL){
            *token_ptr = token;
            PERR_FL("debug this, highest_op_tk shouldn't be null here");
        }
        char *op = highest_op_tk->val;
        if (highest_op_tk == left || right == NULL){
            if (seq(op, "-") || seq(op, "!"))
                ret = parse_op_unary(op, parse_expr(&(highest_op_tk->next)), highest_op_tk->line_num);
            else {
                *token_ptr = token;
                PERR_FL("expected expression");
            }
        }
        else {
            if (seq(op, "!")){
                *token_ptr = token;
                PERR_FL("expected token before '!'");
            }
            else if (seq(op, "="))
                ret = parse_set(&left);
            else {
                highest_op_tk->prev->next = NULL;
                ret = parse_op_binary(op, parse_expr(&left), parse_expr(&right), highest_op_tk->line_num);
            }
        }
    }
    if (token != NULL) {
        NXT_TK;
    }
    *token_ptr = token;
    return ret;
}


// TODO: disable undeclared vars
Expr_t parse_id(Token** token_ptr)
{
    Expr_t ret;
    Token *token = *token_ptr;
    char *name = token->val;
    NXT_TK;
    if (token != NULL && token->tk == OParens){
        ret = create_expr(Function, Undef_R, NULL);
        Expr_t args = parse_arglist(&token);
        Func_t fun = {Function, Undef_R, malloc(sizeof(char)*(1+strlen(name))), args.expr.cons};
        strcpy(fun.name, name);
        *(ret.expr.func) = fun;
    }
    else {
        Expr_t index = NULL_EXPR;
        if (token!= NULL && token->tk == OBracket) {
            Token *start = token->next;
            Token *end = skip_nest(&token, OBracket, CBracket)->prev;
            end->next = NULL;
            index = parse_expr(&start);
            end->next = token;
        }
        ret = create_expr(Var, Undef_R, NULL);
        Var_t var = {Var, Undef_R, malloc(sizeof(char)*(1+strlen(name))), index};
        strcpy(var.name, name);
        *(ret.expr.var) = var;
    }
    return ret;
}


Expr_t parse_op_unary(char *op, Expr_t e, int line_number)
{
    Expr_t ret = NULL_EXPR;
    if (seq(op, "+")){
        fprintf(stderr, "%s:%d: error: invalid use of unary '+'",
                file_name, line_number);
        THROW_MAIN;
    }
    else if (seq(op, "-")){
        if (e.r_type > 1){
            fprintf(stderr, "%s:%d: error: cannot apply value of type %d to unary '-'",
                    file_name, line_number, e.r_type);
            THROW_MAIN;
        }
        ret = create_expr(Sub, e.r_type, NULL);
        *(ret.expr.arith) = (Arith_t){Sub, e.r_type, ZERO_CONSTANT, e};
    }
    else if (seq(op, "!")){
        if (e.r_type == String_R){
            fprintf(stderr, "%s:%d: error: cannot apply value of type %d to unary '!'",
                    file_name, line_number, e.r_type);
            THROW_MAIN;
        }
        ret = create_expr(BExpr, Bool_R, NULL);
        BExpr_t not_expr = {BExpr, Not, e, NULL_EXPR};
        *(ret.expr.bexpr) = not_expr;
    }
    return ret;
}

Expr_t parse_op_binary(char* op, Expr_t left, Expr_t right, int line_number)
{
    Expr_t ret = NULL_EXPR;
    if (seq(op, "+")){
        if (left.r_type == String_R || right.r_type == String_R){
            ret = create_expr(Concat, String_R, NULL);
            *(ret.expr.arith) = (Arith_t){Concat, String_R, left, right};
        }
        else if (left.r_type == Float_R || right.r_type == Float_R){
            ret = create_expr(Add, Float_R, NULL);
            *(ret.expr.arith) = (Arith_t){Add, Float_R, left, right};
        }
        else if (left.r_type == Int_R || right.r_type == Int_R){
            ret = create_expr(Add, Int_R, NULL);
            *(ret.expr.arith) = (Arith_t){Add, Int_R, left, right};
        }
        else if (left.r_type == Bool_R || right.r_type == Bool_R){
            ret = create_expr(BExpr, Bool_R, NULL);
            *(ret.expr.bexpr) = (BExpr_t){BExpr, Or, left, right};
        }
        else {
            ret = create_expr(Add, Undef_R, NULL);
            *(ret.expr.arith) = (Arith_t){Add, Undef_R, left, right};
        }
    }
    else if (seq(op, "-")){
        if (left.r_type > 1 || right.r_type > 1){
            fprintf(stderr, "%s:%d: error: unexpected type\n", file_name, line_number);
            THROW_MAIN;
        }
        else if (left.r_type == Float_R || right.r_type == Float_R){
            ret = create_expr(Sub, Float_R, NULL);
            *(ret.expr.arith) = (Arith_t){Sub, Float_R, left, right};
        }
        else if (left.r_type == Int_R || right.r_type == Int_R){
            ret = create_expr(Sub, Int_R, NULL);
            *(ret.expr.arith) = (Arith_t){Sub, Int_R, left, right};
        }
        else {
            ret = create_expr(Sub, Undef_R, NULL);
            *(ret.expr.arith) = (Arith_t){Sub, Undef_R, left, right};
        }
    }
    else if (seq(op, "*")){
        if (left.r_type == String_R || right.r_type == String_R){
            fprintf(stderr, "%s:%d: error: unexpected type\n", file_name, line_number);
            THROW_MAIN;
        }
        else if (left.r_type == Float_R || right.r_type == Float_R){
            ret = create_expr(Mul, Float_R, NULL);
            *(ret.expr.arith) = (Arith_t){Mul, Float_R, left, right};
        }
        else if (left.r_type == Int_R || right.r_type == Int_R){
            ret = create_expr(Mul, Int_R, NULL);
            *(ret.expr.arith) = (Arith_t){Mul, Int_R, left, right};
        }
        else if (left.r_type == Bool_R || right.r_type == Bool_R){
            ret = create_expr(BExpr, Bool_R, NULL);
            *(ret.expr.bexpr) = (BExpr_t){BExpr, And, left, right};
        }
        else {
            ret = create_expr(Mul, Undef_R, NULL);
            *(ret.expr.arith) = (Arith_t){Mul, Undef_R, left, right};
        }
    }
    else if (seq(op, "/")){
        if (left.r_type > 1 || right.r_type > 1){
            fprintf(stderr, "%s:%d: error: unexpected type\n", file_name, line_number);
            THROW_MAIN;
        }
        else if (left.r_type == Float_R || right.r_type == Float_R){
            ret = create_expr(Div, Float_R, NULL);
            *(ret.expr.arith) = (Arith_t){Div, Float_R, left, right};
        }
        else if (left.r_type == Int_R || right.r_type == Int_R){
            ret = create_expr(Div, Int_R, NULL);
            *(ret.expr.arith) = (Arith_t){Div, Int_R, left, right};
        }
        else {
            ret = create_expr(Div, Undef_R, NULL);
            *(ret.expr.arith) = (Arith_t){Div, Undef_R, left, right};
        }
    }
    else {
        char found = 0;
        char *bops[Or+1] = {"==","!=",">=","<=",">","<","!","&&","||"};
        for (int i = 0; i <= Or && !found; i++){
            if (seq(op, bops[i])){
                ret = create_expr(BExpr, Bool_R, NULL);
                *(ret.expr.bexpr) = (BExpr_t){BExpr, i, left, right};
                found = 1;
                break;
            }
        }
        if (!found) {
            fprintf(stderr, "%s:%d: error: unidentified operator '%s'\n",
                    file_name, line_number, op);
        }
    }
    return ret;
}

// check if set is for var or func (square brackets before '=')
Expr_t parse_set(Token **token_ptr)
{
    if ((*token_ptr)->next->tk == OParens) {
        return parse_set_func(token_ptr);
    }
    else if ((*token_ptr)->next->tk == Oper){
        if (seq((*token_ptr)->next->val, "=")) {
            return parse_set_var(token_ptr);
        }
    }
    PERR_FL("unexpected token");
}    
  

Expr_t parse_set_var(Token **token_ptr)
{
    Token *token = *token_ptr;
    char *name = get_name(&token);
    if (token->tk != Oper || !seq(token->val, "=")){
        PERR_FL("unexpected token");
    }
    NXT_TK;
    Expr_t val = parse_expr(&token);
    Set_t set = {Set, val.r_type, malloc(sizeof(char)*(1+strlen(name))), val};
    strcpy(set.name, name);
    Expr_t ret = create_expr(Set, set.r_type, NULL);
    *(ret.expr.set) = set;
    *token_ptr = token;
    return ret;
}

Expr_t parse_set_func(Token **token_ptr)
{
    Token *token = *token_ptr;
    if (token == NULL){
        PERR("Null token");
    }
    char *name = get_name(&token);
    Expr_t args = parse_arglist(&token);
    
    if (token->tk != Oper || !seq(token->val, "=")){
        PERR_FL("unexpected token");
    }
    NXT_TK;
    if (token->tk == Newline) {
        while (token != NULL && token->tk == Newline) {
            token = token->next;
        }
    }
    Expr_t app = parse_expr(&token);
    // if (args->r_type != String_R){
    //     PERR_FL("Argument list when defining function must be composed of names only");
    // }
    FuncDef_t fun = {FunctionDef, app.r_type, malloc(sizeof(char)*(1+strlen(name))), args.expr.cons, app};
    strcpy(fun.name, name);
    Expr_t expr = create_expr(FunctionDef, fun.r_type, NULL);
    *(expr.expr.func_def) = fun;
    *token_ptr = token;
    return expr;
}

// returns token->val and advances token pointer
char* get_name(Token **token_ptr)
{
    Token *token = *token_ptr;
    if (token == NULL){
        PERR("null token");
    }
    else if (token->tk != Id){
        PERR_FL("Expected ID token");
    }
    *token_ptr = token->next;
    return token->val;
}


Expr_t parse_cond(Token **token_ptr, Token *pred, Token *cond_true, Token *cond_false)
{
    Token *token = pred;
    skip_nest(&token, OParens, CParens);
    token->next = NULL;
    Expr_t predicate = parse_parens(&pred);
    
    token = cond_true;
    // skip_nest(&token, OBrace, CBrace);
    // token->next = NULL;
    Expr_t if_true = parse_sequence(&cond_true);
    token = cond_false;
    // skip_nest(&token, OBrace, CBrace);
    // token->next = NULL;
    Expr_t if_false = parse_sequence(&cond_false);
    // if (if_false.r_type != if_true.r_type){
    //     PERR_FL("error: type mismatch between cases");
    // }
    *token_ptr = token->next;
    Cond_t cond = {Conditional, if_false.r_type, predicate, if_true, if_false};
    Expr_t ret = create_expr(Conditional, cond.r_type, NULL);
    *(ret.expr.cond) = cond;
    return ret;
}

Expr_t parse_parens(Token **token_ptr)
{
    Token *token = *token_ptr;
    Token *start = token;
    if (token->tk == OParens && token->next != NULL) {
        start = token->next;
        
    }
    Token *end = skip_nest(&token, OParens, CParens);
    end = end->prev;
    // free(end->next);
    // free(start->prev);
    start->prev = NULL;
    end->next = NULL;
    end->next = NULL;
    return parse_expr(&start);
}

Expr_t parse_cons(Token **token_ptr, enum expr_type e_type) 
{
    Token *token = *token_ptr;
    enum token_type open, close, separator;
    char c_open, c_close;
    if (e_type == List) {
        open = OBracket;
        close = CBracket;
        separator = Comma;
        c_open = '[';
        c_close= ']';
    }
    else if (e_type == ArgList) {
        open = OParens;
        close = CParens;
        separator = Comma;
        c_open = '(';
        c_close= ')';
    }
    else if (e_type == Sequence) {
        open = OBrace;
        close = CBrace;
        separator = Newline;
        c_open = '{';
        c_close= '}';
    }
    else {
        fprintf(stderr, "%s: ", e_str[e_type]);
        PERR_FL("Bad e_type");
    }
    
    
    if (token == NULL){
        char err_str[100];
        sprintf(err_str, "%s: trailing '%c'", e_str[e_type], c_open);
        PERR(err_str);
    }
    else if (token->tk != open){
        char err_str[100];
        sprintf(err_str, "%s: expected '%c'", e_str[e_type], c_open);
        PERR_FL(err_str);
    }
    else if (token->next == NULL){
        char err_str[100];
        sprintf(err_str, "%s: trailing '%c'", e_str[e_type], c_open);
        PERR_FL(err_str);
    }

    // if empty list/sequence
    if (token->next != NULL && token->next->tk == close) {
        Expr_t ret = create_expr(e_type, Undef_R, NULL);
        *ret.expr.cons = (Cons_t) {e_type, Undef_R, NULL_EXPR, NULL, 0};
        *token_ptr = token->next->next;
        return ret;
    }
    
    Token *start = token;
    Token *end = skip_nest(&token, open, close);
    end = end->next;
    token->next = NULL;
    token = start;
    NXT_TK;
    Expr_u cons = {.cons = malloc(sizeof(Cons_t))};
    Expr_t cons_expr = {e_type, Undef_R, cons};
    Cons_t *curr = cons.cons;
    curr->e_type = e_type;
    curr->r_type = Undef_R;

    Token *element_end;
    Token *temp;
    while (token != NULL){
        element_end = token;
        while (element_end != NULL) {
            if (element_end->tk == open) {
                skip_nest(&element_end, open, close);
                element_end = element_end->next;
            }
            else if (element_end->tk == separator) {
                if (element_end->prev->tk != separator && element_end->prev->tk != open) {
                    temp = element_end;
                    element_end->prev->next = NULL;
                    curr->tail = malloc(sizeof(Cons_t));
                    curr = curr->tail;
                    curr->head = parse_expr(&token);
                    curr->e_type = e_type;
                    curr->r_type = curr->head.r_type;
                    element_end->prev->next = temp;
                    token = temp->next;
                    element_end = token;
                }
                else {
                    token = element_end->next;
                    break;
                }
                curr->tail = NULL;
            }
            else if (element_end->tk == close) {
                if (element_end->prev->tk != separator && element_end->prev->tk != open) {
                    temp = element_end;
                    element_end->prev->next = NULL;
                    curr->tail = malloc(sizeof(Cons_t));
                    curr = curr->tail;
                    curr->head = parse_expr(&token);
                    curr->e_type = e_type;
                    curr->r_type = curr->head.r_type;
                    element_end->prev->next = temp;
                    element_end->next = end;
                    *token_ptr = element_end->next;
                }
                curr->tail = NULL;
                cons_expr.r_type = curr->head.r_type;
                cons_expr.expr.cons = cons.cons->tail;
                add_ptr_to_ll(cons_expr.expr.ptr, NULL);
                free(cons.cons);
                // cons_expr.expr = cons_2;
                get_cons_size(cons_expr.expr.cons);
                return cons_expr;
            }
            else {
                element_end = element_end->next;
            }
        }
    }
    PERR_FL("missing ']'");
}

Expr_t parse_list(Token **token_ptr) {
    return parse_cons(token_ptr, List);
}

Expr_t parse_sequence(Token **token_ptr) {
    return parse_cons(token_ptr, Sequence);
}

Expr_t parse_arglist(Token **token_ptr) {
    return parse_cons(token_ptr, ArgList);
}


Expr_t parse_constant(Token **token_ptr)
{
    Expr_t ret = NULL_EXPR;
    Token *token = *token_ptr;
    if (token == NULL){
        PERR("Null token");
    }
    else if (token->tk > 4){
        PERR_FL("Not a constant");
    }

    if (token->tk == Char || token->tk == Str) {
        ret = wrap_str(token->val);
    }
    else if (token->tk == Int) {
        ret = wrap_int(atoi(token->val));
    }
    else if (token->tk == Float) {
        ret = wrap_flt(atof(token->val));
    }
    else if (token->tk == Bool) {
        // Bools are to be treated the same as ints for almost everything
        if (seq(token->val, "false")) {
            ret = wrap_int(0);
        }
        else {
            ret = wrap_int(1);
        }
        ret.r_type = Bool_R;
    }
    else {
        PERR_FL("something went wrong with token type (parse_constant)");
    }
    return ret;
}

uint get_cons_size(Cons_t *cons) {
    uint size = 0;
    Cons_t *curr = cons;
    while (curr != NULL) {
        size++;
        curr = curr->tail;
    }
    curr = cons;
    uint size_0 = size;
    while (curr != NULL) {
        curr->size = size--;
        curr = curr->tail;
    }
    return size_0;
}


Token *skip_nest(Token **token_ptr, enum token_type open, enum token_type close)
{
    Token *token = *token_ptr;
    if (token == NULL){
        PERR("NULL pointer");
    }
    if (token->tk != open){
        PERR_FL("token type and open don't match");
    }
    NXT_TK;
    int nested = 1;
    enum token_type tk;
    while (token != NULL){
        tk = token->tk;
        if (tk == open) {
            nested++;
        }
        else if (tk == close){
            if (--nested == 0){
                *token_ptr = token;
                return token;
            }
            else if (nested < 0){
                PERR_FL("invalid nesting, missing opening paren or brace");
                THROW_MAIN;
            }
        }
        if (token->next == NULL) {
            token->next = pop_tk_line();
        }
        NXT_TK;
    }
    
    char close_char;
    switch (close) {
    case CParens:
        close_char = ')';
        break;
    case CBracket:
        close_char = ']';
        break;
    case CBrace:
        close_char = '}';
        break;
    default:  // just to appease the compiler, should never happen
        close_char = '?';
        break;
    }
    char str[100];
    sprintf(str, "expected closing '%c'", close_char);
    PERR_FL(str);
}
