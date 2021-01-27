#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tk_line.h"
#include "lexer.h"
#include "flags.h"

char stk_lst[STK_LST_MAXLEN] = "";

// destructively pops the first line of tokens from tk_lst and returns the first token in the line
Token *pop_tk_line()
{
    Token *curr = tk_lst.head.next;
    Token *start = curr;
    if (curr == NULL)
	return NULL;
  
    while (curr->next != NULL && curr->next->tk != Newline) {
	curr = curr->next;
    }
    if (curr->next != NULL) {
	tk_lst.head.next = curr->next->next;
	curr->next->next = NULL;
    }
    if (FLAGS & DEBUG_FLAG) {
        update_stk_lst();
    }
    return start;
}

void push_tk_line(Token *token)
{
    if (token == NULL)
	return;
    Token *end = token;
    while (end->next != NULL)
	end = end->next;
    end->next = tk_lst.head.next;
    /* tk_lst.head.next = end;  // should be token? */
    tk_lst.head.next = token;  // should be token?
    if (FLAGS & DEBUG_FLAG) {
        update_stk_lst();
    }
}
	
void append_to_token(Token *token)
{
    if (token == NULL){
	token = pop_tk_line();
	return;
    }
    while (token->next != NULL)
	token = token->next;
    token->next = pop_tk_line();
    if (FLAGS & DEBUG_FLAG) {
        update_stk_lst();
    }
}

void update_stk_lst()
{
    stk_lst[0] = 0;
    /* stk_lst[1] = 0; */
    Token *curr = tk_lst.head.next;
    int len = 1;
    while (curr != NULL && len < STK_LST_MAXLEN - 1) {
        len = sprintf(stk_lst, "%s,%s", stk_lst, t_str[curr->tk]);
        curr = curr->next;
    }
    sprintf(stk_lst, "[%s]", stk_lst);
}
