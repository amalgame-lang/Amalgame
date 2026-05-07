#ifndef AMALGAME_CONSOLE_H
#define AMALGAME_CONSOLE_H
#include <stdio.h>
#include "_runtime.h"
static inline void Amalgame_Compiler_Console_WriteLine(code_string s) { puts(s ? s : ""); }
static inline void Amalgame_Compiler_Console_WriteError(code_string s) { fputs(s ? s : "", stderr); fputs("\n", stderr); }
#endif
