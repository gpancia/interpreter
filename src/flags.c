#include <string.h>
#include "flags.h"

const char* flag_chars = "dvNNNNNN";

char default_flags() {
    return (char) DEBUG_FLAG;
}

char parse_flags(int argc, char *argv[]) {
    if (argc == 1) {
        return default_flags();
    }
    char flag = 0;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            int pos = 1;
            while (argv[i][pos] == '-') {
                pos++;
            }
            int ff = 1;
            for (int fi = 0; fi < 8; fi++) {
                if (flag_chars[fi] == argv[i][pos]) {
                    flag |= ff;
                    break;
                }
                else {
                    ff *= 2;
                }
            }
        }
    }
    return flag;
}

/* int is_flag(char *arg, Flag flag) { */
/*     /\* if (!strcmp(arg,flag.l_name) || !strcmp(arg, flag.s_name)) { *\/ */
/*     /\*     return 1; *\/ */
/*     /\* } *\/ */
/*     if (arg[0] == '-') { */
/*         int pos = 1; */
/*         while (arg[pos] == '-') { */
/*             pos++; */
/*         } */
/*         if (arg[pos] == ' ') { */
/*             return 0; */
/*         } */
/*         else { */
/*             return (arg[pos] == flag.s_name); */
/*         } */
/*     } */
/*     else { */
/*         return 0; */
/*     } */
/* } */

/* void init_flag_list() { */
/*     const char *s_names = "dNNNNNNN"; */
/* } */
