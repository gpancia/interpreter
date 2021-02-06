#ifndef __ERROR_HANDLING_H__
#define __ERROR_HANDLING_H__

#include <setjmp.h>

#ifndef THROW_ERROR
#define THROW_ERROR longjmp(jmp_env, 1);
#endif

extern jmp_buf jmp_env;

#endif
