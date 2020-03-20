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
  int line_num; // for debugging, if this isn't stored here you can't reference line number when parsing/evaluating
}Token;

// for easy freeing
typedef struct {
  Token head;  // should be a Null token
  Token *tail;
}TokenList;

typedef struct {
  int size;
  struct {
	char c;
	enum token_type tk_t;
  }*interrupt;
}InterruptList;

extern TokenList tk_lst;
extern InterruptList interrupt_list;

void tk_lst_init(); // initializes global var tk_lst 
void tk_lst_free(); // frees all malloc'd fields in tk_lst
void tk_free(Token*); // frees a malloc'd token and its string, if any
int tk_add(char *wrd); // adds a token to tk_lst with the appropriate type enum
int lex(char *file_path); // reads a file and turns it into a list of tokens using tk_add()
void build_interrupts(); // reads a file with characters that interrupt token names, enabling separation of, for example, a*b into tokens 'a', '*' and 'b'.
							   // if file does not exist, calls lexer_utils and builds it.

int is_interrupt(char c);

typedef struct tk_match_t{
  // token type enum corresponding to chars in matches
  enum token_type tk; 
  // whether or not the token's value is needed
  // (i.e.: storing '\n' is pointless malloc'ing and free'ing)
  char has_val;
  int num_patterns; // necessary for looping through pattern array properly
}tk_match_t;

extern tk_match_t tk_match[];
extern char* tk_pattern[][14];
extern int num_tk_types;

#endif
