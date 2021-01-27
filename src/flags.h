#ifndef __FLAGS_H__
#define __FLAGS_H__

#define DEBUG_FLAG 1

extern char FLAGS;
extern const char* flag_chars;

/* typedef struct { */
/*     /\* char *l_name; *\/ */
/*     char s_name; */
/*     char flag; */
/* } Flag; */

/* Flag flag_list[8]; */

char default_flags();
char parse_flags(int argc, char *argv[]);
/* int is_flag(char *arg, Flag flag); */
/* void init_flag_list(); */


#endif
