#ifndef __ERROR_HANDLING_H__
#define __ERROR_HANDLING_H__

#include <setjmp.h>

#ifndef THROW_MAIN
#define THROW_MAIN longjmp(main_jmp_env, 1);
#endif

extern jmp_buf main_jmp_env;


#endif
