#ifndef __LEXER_H__
#define __LEXER_H__

#define seq(s1,s2) !strcmp(s1,s2)

extern char *token_type_string[];

// Constants:  <5
enum token_type {Int, Float, Str,
                 Bool, Char,
                 Cond, Id, Oper,
                 OParens, CParens,
                 OBrace, CBrace,
                 OBracket, CBracket, Comma,
                 Newline, Null};

extern char *t_str[];

extern char *file_name;


typedef struct Token{
    enum token_type tk;
    char *val;
    struct Token *next;
    struct Token *prev;
    int line_num;  // for debugging, if this isn't stored here you can't reference line number when parsing/evaluating
} Token;

// output token list, easily manipulatable for parsing but can lose pointers leading to leaks if by itself
typedef struct {
    Token head;  // should be a Null token
    Token *tail;
} TokenList;
extern TokenList tk_lst;

// Linked list of pointers that is independent from the alterations to the values it points to
typedef struct LL_Token{
    struct LL_Token *next;
    Token *this;
} LL_Token;

LL_Token ll_tk_head;  
LL_Token *ll_tk_tail;

typedef struct {
    int size;
    struct {
	char c;
	enum token_type tk_t;
    } *interrupt;
} InterruptList;

extern InterruptList interrupt_list;

void tk_lst_init();  // initializes global vars tk_lst and full_tk_lst
void tk_lst_free();  // frees all malloc'd fields in full_tk_lst; tk_lst is ready to use again immediately
void tk_free(Token*);  // frees a malloc'd token and its string, if any
int tk_add(char *wrd);  // adds a token to tk_lst with the appropriate type enum and its pointer to full_tk_lst
int lex_string(char *str);  // passes a string stream to lex
int lex_file(char *file_path);  // passes a file stream to lex
int lex(void *stream);  // reads a stream and turns it into a list of tokens using tk_add()
void build_interrupts();  // reads a file with characters that interrupt token names, enabling separation of, for example, a*b into tokens 'a', '*' and 'b'.
// if file does not exist, calls lexer_utils and builds it.

int is_interrupt(char c);

typedef struct tk_match_t{
    // token type enum corresponding to chars in matches
    enum token_type tk; 
    // whether or not the token's value is needed
    // (i.e.: storing '\n' is pointless malloc'ing and free'ing)
    char has_val;
    int num_patterns;  // necessary for looping through pattern array properly
} tk_match_t;

extern tk_match_t tk_match[];
extern char* tk_pattern[][14];
extern int num_tk_types;

#endif
