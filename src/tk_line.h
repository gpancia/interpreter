#ifndef __TK_LINE_H__
#define __TK_LINE_H__

#include "lexer.h"

Token *pop_tk_line();
void push_tk_line(Token*);
void append_to_token(Token*);


#endif
