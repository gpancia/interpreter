#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tk_line.h"
#include "parser.h"

#define PERR_FL(str) fprintf(stderr, "%s:%d, %s:%d: error: %s\n", file_name, (*token_ptr)->line_num, __func__,__LINE__, str);exit(1)
#define PERR(str) fprintf(stderr, "%s:%s: error: %s\n", file_name, __func__, str);exit(1)
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
  enum token_type tk;
  enum ordering {divmul, addsub, bexpr, set, null=-1}; // ordering is lowest to highest in AST, e.g.: + is higher than *
  enum ordering highest_op = null;
  while (token != NULL){
    tk = token->tk;
    if (tk < 5 || tk == Id) { // skip constants and binds for now
      NXT_TK;
    }
    else if (tk == Cond){ // skip ifs for now
      if (token->next == NULL){
        token->next = pop_tk_line();
      }
      NXT_TK;
      // skip_nest checks for anything wrong with the layout of the if statement
      skip_nest(&token, OParens, CParens); // skip over predicate
      skip_nest(&token, OBrace, CBrace); // skip over if_true
      skip_nest(&token, OBrace, CBrace); // skip over if_false
    }
    else if (tk == Oper){
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
    else if (tk == OBrace) {
      skip_nest(&token, OBrace, CBrace);
    }
    else if (tk == OParens) {
      skip_nest(&token, OParens, CParens);
    }
    else if (tk == CParens || tk == CBrace) {
      PERR_FL("mismatched closing brace or paren");
    }
    else if (tk == Comma){
      PERR_FL("unexpected comma");
    }
    else if (tk == Newline){
      break;
    }
    else {
      char err[1000];
      sprintf(err, "unexpected token_type %d, debug this pls", tk);
      PERR_FL(err);
    }
  }
  if (highest_op < 0){
    tk = left->tk;
    if (tk == Newline) {
      ret = NULL_EXPR;
    }
    else if (tk < 5) {
      ret = parse_constant(&left);
    }
    else if (tk == Cond) {
      ret = parse_cond(&left);
    }
    else if (tk == Id) {
      ret = parse_id(&left);
    }
    else if (tk == OParens) {
      ret = parse_parens(&left);
    }
    else if (tk == OBrace) {
      ret = parse_sequence(&left);
    }
    else {
      char err[100];
      sprintf(err, "debug this, unexpected token %d", tk); // shouldn't happen
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
      if (seq(op,"+") || seq(op, "-") || seq(op, "!"))
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
        left->next = NULL;
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
  Expr_u expr;
  Token *token = *token_ptr;
  char *name = token->val;
  NXT_TK;
  if (token != NULL && token->tk == OParens){
    Fun_t fun = {Function, Undef_R, malloc(sizeof(char*)), parse_parens(&token).expr.cons};
    strcpy(fun.name, name);
    expr.func = malloc(sizeof(Fun_t));
    *(expr.func) = fun;
  }
  else {
    Var_t var = {Var, Undef_R, malloc(sizeof(char)*(1+strlen(name)))};
    strcpy(var.name, name);
    expr.func = malloc(sizeof(Var_t));
    *(expr.var) = var;
  }
  return (Expr_t){expr.generic->e_type, expr.generic->r_type, expr};
}


Expr_t parse_op_unary(char *op, Expr_t e, int line_number)
{
  Expr_t ret = NULL_EXPR;
  if (seq(op, "+")){
    fprintf(stderr, "%s:%d: error: invalid use of unary '+'",
            file_name, line_number);
    exit(1);
  }
  else if (seq(op, "-")){
    if (e.r_type > 1){
      fprintf(stderr, "%s:%d: error: cannot apply value of type %d to unary '-'",
              file_name, line_number, e.r_type);
      exit(1);
    }
    ret = (Expr_t){Sub, e.r_type, {malloc(sizeof(Arith_t))}};
    *(ret.expr.arith) = (Arith_t){Sub, e.r_type, ZERO_CONSTANT, e};
  }
  else if (seq(op, "!")){
    if (e.r_type == String_R){
      fprintf(stderr, "%s:%d: error: cannot apply value of type %d to unary '!'",
              file_name, line_number, e.r_type);
      exit(1);
    }
    ret = (Expr_t){BExpr, Bool_R, {malloc(sizeof(BExpr_t))}};
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
      ret = (Expr_t){Concat, String_R, {malloc(sizeof(Arith_t))}};
      *(ret.expr.arith) = (Arith_t){Concat, String_R, left, right};
    }
    else if (left.r_type == Float_R || right.r_type == Float_R){
      ret = (Expr_t){Add, Float_R, {malloc(sizeof(Arith_t))}};
      *(ret.expr.arith) = (Arith_t){Add, Float_R, left, right};
    }
    else if (left.r_type == Int_R || right.r_type == Int_R){
      ret = (Expr_t){Add, Int_R, {malloc(sizeof(Arith_t))}};
      *(ret.expr.arith) = (Arith_t){Add, Int_R, left, right};
    }
    else if (left.r_type == Bool_R || right.r_type == Bool_R){
      ret = (Expr_t){BExpr, Bool_R, {malloc(sizeof(BExpr_t))}};
      *(ret.expr.bexpr) = (BExpr_t){BExpr, Or, left, right};
    }
    else {
      ret = (Expr_t){Add, Undef_R, {malloc(sizeof(Arith_t))}};
      *(ret.expr.arith) = (Arith_t){Add, Undef_R, left, right};
    }
  }
  else if (seq(op, "-")){
    if (left.r_type > 1 || right.r_type > 1){
      fprintf(stderr, "%s:%d: error: unexpected type\n", file_name, line_number);
      exit(1);
    }
    else if (left.r_type == Float_R || right.r_type == Float_R){
      ret = (Expr_t){Sub, Float_R, {malloc(sizeof(Arith_t))}};
      *(ret.expr.arith) = (Arith_t){Sub, Float_R, left, right};
    }
    else if (left.r_type == Int_R || right.r_type == Int_R){
      ret = (Expr_t){Sub, Int_R, {malloc(sizeof(Arith_t))}};
      *(ret.expr.arith) = (Arith_t){Sub, Int_R, left, right};
    }
    else {
      ret = (Expr_t){Sub, Undef_R, {malloc(sizeof(Arith_t))}};
      *(ret.expr.arith) = (Arith_t){Sub, Undef_R, left, right};
    }
  }
  else if (seq(op, "*")){
    if (left.r_type == String_R || right.r_type == String_R){
      fprintf(stderr, "%s:%d: error: unexpected type\n", file_name, line_number);
      exit(1);
    }
    else if (left.r_type == Float_R || right.r_type == Float_R){
      ret = (Expr_t){Mul, Float_R, {malloc(sizeof(Arith_t))}};
      *(ret.expr.arith) = (Arith_t){Mul, Float_R, left, right};
    }
    else if (left.r_type == Int_R || right.r_type == Int_R){
      ret = (Expr_t){Mul, Int_R, {malloc(sizeof(Arith_t))}};
      *(ret.expr.arith) = (Arith_t){Mul, Int_R, left, right};
    }
    else if (left.r_type == Bool_R || right.r_type == Bool_R){
      ret = (Expr_t){BExpr, Bool_R, {malloc(sizeof(BExpr_t))}};
      *(ret.expr.bexpr) = (BExpr_t){BExpr, And, left, right};
    }
    else {
      ret = (Expr_t){Mul, Undef_R, {malloc(sizeof(Arith_t))}};
      *(ret.expr.arith) = (Arith_t){Mul, Undef_R, left, right};
    }
  }
  else if (seq(op, "/")){
    if (left.r_type > 1 || right.r_type > 1){
      fprintf(stderr, "%s:%d: error: unexpected type\n", file_name, line_number);
      exit(1);
    }
    else if (left.r_type == Float_R || right.r_type == Float_R){
      ret = (Expr_t){Div, Float_R, {malloc(sizeof(Arith_t))}};
      *(ret.expr.arith) = (Arith_t){Div, Float_R, left, right};
    }
    else if (left.r_type == Int_R || right.r_type == Int_R){
      ret = (Expr_t){Div, Int_R, {malloc(sizeof(Arith_t))}};
      *(ret.expr.arith) = (Arith_t){Div, Int_R, left, right};
    }
    else {
      ret = (Expr_t){Div, Undef_R, {malloc(sizeof(Arith_t))}};
      *(ret.expr.arith) = (Arith_t){Div, Undef_R, left, right};
    }
  }
  // Shouldn't ever happen
  /* else if (seq(op, "=")){ */
  /*   if (left.e_type != Id){ */
  /*     fprintf(stderr, "%s:%d: error: cannot use expression of type %d as variable name\n", file_name, line_number, left.e_type); */
  /*     exit(1); */
  /*   } */
  /*   // check if id is function dec (has parens) */
  /* } */
  else {
    char found = 0;
    char *bops[Or+1] = {"==","!=",">=","<=",">","<","!","&&","||"};
    for (int i = 0; i <= Or && !found; i++){
      if (seq(op, bops[i])){
        ret = (Expr_t){BExpr, Bool_R, {malloc(sizeof(BExpr_t))}};
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

// check if set is for var or func (parens before '=')
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
  Set_t set = {Set, val.r_type, val, malloc(sizeof(char)*(1+strlen(name)))};
  strcpy(set.name, name);
  Expr_t ret = {Set, set.r_type, {malloc(sizeof(Set_t))}};
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
  Cons_t *args = parse_expr(&token).expr.cons;
  if (token->tk != Oper || !seq(token->val, "=")){
    PERR_FL("unexpected token");
  }
  NXT_TK;
  Expr_t app = parse_expr(&token);
  if (args->r_type != String_R){
    PERR_FL("Argument list when defining function must be composed of names only");
  }
  FunDef_t fun = {FunctionDef, app.r_type, malloc(sizeof(char)*(1+strlen(name))), args, app};
  strcpy(fun.name, name);
  Expr_t expr = {FunctionDef, fun.r_type, {malloc(sizeof(FunDef_t))}};
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


Expr_t parse_cond(Token **token_ptr)
{
  Token *token = *token_ptr;
  if (token == NULL){
    PERR_FL("expected token after conditional");
  }
  if (token->next == NULL){
    PERR("expected token after conditional");
  }
  NXT_TK;
  if (token->tk != OParens){
    PERR_FL("error: expected boolean expression (surrounded by parens)");
  }
  
  Expr_t predicate = parse_expr(&token);
  Expr_t if_true = parse_expr(&token);
  Expr_t if_false = parse_expr(&token);
  if (if_false.r_type != if_true.r_type){
    PERR_FL("error: type mismatch between cases");
  }
  Cond_t cond = {Conditional, if_false.r_type, predicate, if_true, if_false};
  Expr_t ret;
  ret.e_type = Conditional;
  ret.r_type = cond.r_type;
  ret.expr.cond = malloc(sizeof(Cond_t));
  *(ret.expr.cond) = cond;
  *token_ptr = token;
  return ret;
}

Expr_t parse_parens(Token **token_ptr)
{
  Token *token = *token_ptr;
  if (token == NULL){
    PERR("trailing '('");
  }
  else if (token->tk != OParens){
    PERR_FL("expected '('");
  }
  else if (token->next == NULL){
    PERR_FL("trailing '('");
  }
  NXT_TK;

  Expr_u lst;
  lst.cons = malloc(sizeof(Cons_t));
  lst.cons->e_type = List;
  Expr_t lst_expr = {List, Undef_R, lst};
  Cons_t *curr = lst.cons;
  
  // Since every time a new element is found new memory is malloc'd, to avoid
  // having an empty head at the beginning of the list, this if/else statement
  // is necessary before the while loop
  if (token->tk == CParens) {
    curr->head = NULL_EXPR;
  }
  else {
    curr->head = parse_expr(&token);
    NXT_TK;
  }
  while (token != NULL){
    if (token->tk == CParens){
      curr->tail = NULL;
      lst_expr.r_type = curr->head.r_type;
      NXT_TK;
      *token_ptr = token;
      return lst_expr;
    }
    else {
      curr->tail = malloc(sizeof(Cons_t));
      curr = curr->tail;
      curr->head = parse_expr(&token);
      NXT_TK;
    }
  }
  PERR_FL("missing ')'");
}

Expr_t parse_sequence(Token **token_ptr)
{
  Token *token = *token_ptr;
  if (token == NULL){
    PERR("trailing '{'");
  }
  else if (token->tk != OBrace){
    PERR_FL("expected '{'");
  }
  else if (token->next == NULL){
    PERR_FL("trailing '{'");
  }
  NXT_TK;

  
  Expr_u seq;
  seq.cons = malloc(sizeof(Cons_t));
  seq.cons->e_type = Sequence;
  Expr_t seq_expr = {Sequence, Undef_R, seq};
  Cons_t *curr = seq.cons;
  
  // Since every time a new element is found new memory is malloc'd, to avoid
  // having an empty head at the beginning of the sequence, this if/else statement
  // is necessary before the while loop
  if (token->tk == CBrace) {
    curr->head = NULL_EXPR;
  }
  else {
    curr->head = parse_expr(&token);
    NXT_TK;
  }
  while (token != NULL){
    if (token->tk == Newline) {  // ignore newlines within code block
      NXT_TK;
    }
    else if (token->tk == CBrace){
      curr->tail = NULL;
      seq_expr.r_type = curr->head.r_type;
      NXT_TK;
      if (token->tk == Newline) {
        NXT_TK;
      }
      *token_ptr = token;
      return seq_expr;
    }
    else {
      curr->tail = malloc(sizeof(Cons_t));
      curr = curr->tail;
      curr->head = parse_expr(&token);
      NXT_TK;
    }
  }
  PERR_FL("missing '}'");
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
        if (token->next != NULL) {
          push_tk_line(token->next);
        }
        NXT_TK;
        *token_ptr = token;
        return token;
      }
      else if (nested < 0){
        PERR_FL("invalid nesting, missing opening paren or brace");
        exit(1);
      }
    }
    if (token->next == NULL) {
      token->next = pop_tk_line();
    }
    NXT_TK;
  }
  PERR_FL("no closing parens or braces");
}
