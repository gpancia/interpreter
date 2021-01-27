#ifndef __TK_LINE_H__
#define __TK_LINE_H__

#include "lexer.h"

#define STK_LST_MAXLEN 1000

extern char stk_lst[STK_LST_MAXLEN];

Token *pop_tk_line();
void push_tk_line(Token*);
void append_to_token(Token*);
void update_stk_lst();

#endif
