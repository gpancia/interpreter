#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tk_line.h"

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
    tk_lst.head.next = end;
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
}
