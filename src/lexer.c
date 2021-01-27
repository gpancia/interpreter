#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <regex.h>
#include "lexer.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_INTERRUPTS 100

InterruptList interrupt_list;
TokenList tk_lst = {};
int file_line_num = 0;
char *file_name;
#define PERR_FL(msg) fprintf(stderr, "%s:%s:%d: error: %s", __func__, file_name, file_line_num, msg);

// for determining type of token and whether or not to store its string (storing '\n' is pointless, for example)
tk_match_t tk_match[] = {{Newline, 0, 1},{OParens, 0, 1},{CParens, 0, 1},
                         {OBrace, 0, 1},{CBrace, 0, 1}, {Comma, 0, 1},
                         {Oper, 1, 14},
                         {Str, 1, 1}, {Bool, 1, 2}, {Cond, 0, 1}
};
char *tk_pattern[][14] = {{"\n"},{"("},{")"},
                          {"{"},{"}"},{","},
                          {"+","-","*","/","=",">",">=",
                           "<","<=","==","!","&&","||", "!="},
                          {"\""},{"true","false"},{"if"}};
int num_tk_types = sizeof(tk_match)/sizeof(tk_match_t);
int num_interrupts;

int lex(char *file_path)
{
    file_name = basename(file_path);
    FILE *fd = fopen(file_path, "r");
    if (fd == NULL){
        fprintf(stderr, "Unable to open file \"%s\"\n", file_path);
        fclose(fd);
        return 0;
    }
    build_interrupts();
    char token[1000] = "\0";
    char c;
    for (int i = 0; i < 999; i++){
        c = fgetc(fd);
    skip_getc:;
        if (c == EOF){
            if (*token && *token != '\n'){
                token[i] = '\0';
                tk_add(token);
            }
            fclose(fd);
            return 1;
        }
        else if (c == '#'){
            printf("COMMENT: ");
            while (c != '\n' && c != '\0' && c != EOF){
                printf("%c",c);
                c = fgetc(fd);
            }
            printf("\n");
            i = -1;
        }
	  
        else if (c == ' ' || c == '\t'){
            if (*token){
                token[i] = '\0';
                tk_add(token);
            }
            token[0] = '\0';
            i = -1;
        }		
        else if (is_interrupt(c)){
            if (*token){
                token[i] = '\0';
                tk_add(token);
            }
            i = 0;
            if (c == '\n'){
                file_line_num++;
                if (tk_lst.tail->tk == Newline) {// ignore repeated newlines
                    i = -1;
                    continue;
                }
            }
            if (c == '"'){
                token[i++] = c;
                c = fgetc(fd);
                int internal_line_num_counter = 0;
                char escaped = 0;
                while (c != '"' || escaped){
                    if (c == '\n')
                        internal_line_num_counter++;
                    if (c == EOF){
                        fprintf(stderr,"%s:%d: unmatched \"\n",file_name,file_line_num);
                        return 0;
                    }
                    else if (c == '\\' && !escaped){
                        escaped = 1;
                    }
                    else {
                        escaped = 0;
                        token[i++] = c;
                        token[i+1] = 0;
                    }
                    c = fgetc(fd);
                }
                tk_add(token);
                i = 0;
                token[0] = '\0';
                file_line_num += internal_line_num_counter;
            }
            else if (c == '<' || c == '>' || c == '=' || c == '='){
                char nc = fgetc(fd);
                if (nc == '='){
                    token[0] = c;
                    token[1] = nc;
                    token[2] = '\0';
                    tk_add(token);
                    token[0] = '\0';
                    i = -1;
                }
                else {
                    token[0] = c;
                    token[1] = '\0';
                    tk_add(token);
                    token[0] = '\0';
                    c = nc;
                    i = 0;
                    goto skip_getc;
                }
            }
            else if (c == '&' || c == '|'){
                char nc = fgetc(fd);
                if (nc == c){
                    token[0] = c;
                    token[1] = nc;
                    token[2] = '\0';
                    tk_add(token);
                    token[0] = '\0';
                    i = -1;
                }
                else {
                    token[0] = c;
                    token[1] = '\0';
                    tk_add(token);
                    token[0] = '\0';
                    i = 0;
                    c = nc;
                    goto skip_getc;
                }
            }
            else {
                token[0] = c;
                token[1] = '\0';
                tk_add(token);
                token[0] = '\0';
                i = -1;
            }
        }
        else {
            token[i] = c;
        }
    }
    fclose(fd);
    return 1;
}



void tk_lst_init()
{
    tk_lst.head.tk = Null;
    tk_lst.head.val = NULL;
    tk_lst.head.next = NULL;
    tk_lst.head.prev = NULL;
    tk_lst.head.line_num = -1;
    tk_lst.tail = &(tk_lst.head);
}

void tk_lst_free()
{
    if (tk_lst.tail == NULL)
        return;
    else {
        Token *curr = tk_lst.tail;
        while (curr->line_num >= 0 && curr->prev != NULL){
            Token *temp = curr;
            curr = curr->prev;
            tk_free(temp);
        }
    }
}

void tk_free(Token *token)
{
    if (token == NULL)
        return;
    if (token->val != NULL)
        free(token->val);
    free(token);
}

int tk_add(char *wrd)
{
    char alloc_val = 0;
    enum token_type tk;
    if (wrd == NULL || !strlen(wrd)){
        fprintf(stderr, "%s:%d: warning: attempting to add empty token to token list\n", file_name, file_line_num);
        return 0;
    }

    // check if char
    if (*wrd == '\''){
        int len = strlen(wrd);
        if (len > 3){
            fprintf(stderr, "%s:%d: invalid character %s\n",file_name,file_line_num,wrd);
            return 0;
        }
        else if (wrd[len-1] != '\''){
            fprintf(stderr, "%s:%d: unmatched '\n",file_name,file_line_num);
            return 0;
        }
        else {
            tk = Char;
            char chr[1];
            chr[0] = wrd[1];
            wrd = chr;
            alloc_val = 1;
            goto assign;
        }
    }

    // check if string
    if (*wrd == '"'){
        tk = Str;
        alloc_val = 1;
        wrd = wrd + 1;
        goto assign;
    }
  
    // check if int or float
    regex_t regex_num;
    if (!regcomp(&regex_num, "^[0-9.][0-9.]*$", 0)){
        int reti = regexec(&regex_num, wrd, 0, NULL, 0);
        regfree(&regex_num);
        if (reti != REG_NOMATCH){
            if (strchr(wrd,'.'))
                tk = Float;
            else
                tk = Int;
            alloc_val = 1;
            goto assign;
        }
    }  
    int match_size;
    for (int i = 0; i < num_tk_types; i++){
        match_size = tk_match[i].num_patterns;
        for (int j = 0; j < match_size; j++){
            if (seq(wrd,tk_pattern[i][j])){
                tk = tk_match[i].tk;
                alloc_val = tk_match[i].has_val;
                wrd += (*wrd == '"') ? 1 : 0;
                goto assign;
            }
        }
    }
    tk = Id;
    alloc_val = 1;
 assign: ;
    Token *new_tk = malloc(sizeof(Token));
    new_tk->tk = tk;
    if (alloc_val){
    
        new_tk->val = malloc(sizeof(char)*(strlen(wrd)+1));
        strcpy(new_tk->val,wrd);
    }
    else {
        new_tk->val = NULL;
    }
    new_tk->next = NULL;
    new_tk->line_num = file_line_num;
    new_tk->prev = tk_lst.tail;
    tk_lst.tail->next = new_tk;
    tk_lst.tail = new_tk;
    return 1;
} 

//[number of interrupt types]{[token_type enum][interrupt's first char]...}
void build_interrupts()
{
    int fd = open("data/lexer_token_interrupts.dat", O_RDONLY);
    struct stat sb;
    if (fd == -1 || fstat(fd, &sb) == -1)
        PERR_FL("invalid file/file size");
  
    char *file = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    interrupt_list.size = *file;
    int offset = sizeof(int); // skip size
    interrupt_list.interrupt = malloc(2*sizeof(int)*interrupt_list.size); // 2*sizeof(int) instead of sizeof(char)+sizeof(int) to compensate for padding
    for (int i = 0; i < interrupt_list.size; i++){
        interrupt_list.interrupt[i].tk_t = (int)file[offset]; // read enum token_type
        offset += sizeof(int); 
        interrupt_list.interrupt[i].c =  file[offset++]; // read interrupt char
    }
}

int is_interrupt(char c)
{
    for (int i = 0; i < interrupt_list.size; i++){
        if (interrupt_list.interrupt[i].c == c)
            return 1;
    }
    return 0;
}

