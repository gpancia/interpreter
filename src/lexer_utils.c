#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "lexer_utils.h"
#include "lexer.h"
#include "config.h"
#include "flags.h"
int curr_arg = 1;

//[number of interrupts]{[token_type enum][interrupt]...}
void update_interrupts() 
{
    char filename[200];
    sprintf(filename, "%s/data/lexer_token_interrupts.dat", ROOT_DIR);
    FILE *fd = fopen(filename, "w");
    int ntk = num_tk_types - 2;
    int nint = 0;
    for (int i = 0; i < ntk; i++)
	nint += tk_match[i].num_patterns;
    fwrite(&nint, sizeof(int), 1, fd);
    for (int i = 0; i < ntk; i++){
	for (int j = 0; j < tk_match[i].num_patterns; j++){
            fwrite(&(tk_match[i].tk), sizeof(int), 1, fd);
            fwrite(&(*tk_pattern[i][j]), sizeof(char), 1, fd);
	}
    }
    fclose(fd);
}

void print_all_tokens2(int argc, char *argv[])
{
    char fp[200];
    if (curr_arg >= argc-1){
	printf("Using default file, \"test.txt\"\n");
	sprintf(fp, "%s/test/test.txt", ROOT_DIR);
    }
    else {
	strcpy(fp, argv[++curr_arg]);
    }
    tk_lst_init();
    lex_file(fp);
    if (FLAGS & VERBOSE_FLAG) {
        Token *curr = tk_lst.head.next;
        while (curr != NULL){
            printf("[%s", t_str[curr->tk]);
            if (curr->val == NULL)
                printf("]\n");
            else
                printf(" : %s]\n", curr->val);
            curr = curr->next;
        }
    }
    printf("--------DONE LEXING--------\n\n");
  
}

void print_all_tokens(int argc, char *argv[])
{
    char *fp;
    if (curr_arg >= argc-1){
	printf("Using default file, \"test.txt\"\n");
	fp = "test/test.txt";
    }
    else
	fp = argv[++curr_arg];
    tk_lst_init();
    lex(fp);
    Token *curr = tk_lst.head.next;
    while (curr != NULL){
	printf("[%s", t_str[curr->tk]);
	if (curr->val == NULL)
            printf("]\n");
	else
            printf(" : %s]\n", curr->val);
	curr = curr->next;
    }
    tk_lst_free();
    free(interrupt_list.interrupt);
}

void test_interrupts()
{
    update_interrupts();
    build_interrupts();
    enum token_type curr_tk = -1;
    for (int i = 0; i < interrupt_list.size; i++){
	if (curr_tk != interrupt_list.interrupt[i].tk_t){
            curr_tk = interrupt_list.interrupt[i].tk_t;
            printf("\n%s:\n", t_str[curr_tk]);
	}
	if (interrupt_list.interrupt[i].c == '\n')
            printf("\t\\n");
	else
            printf("\t%c", interrupt_list.interrupt[i].c);
    }
    printf("\n");
    free(interrupt_list.interrupt);
}
