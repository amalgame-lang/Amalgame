// amc_bootstrap/amc — C glue: parses CLI args and calls AmalgameCompiler.Run()
// This file is NOT linked with amc_am.c — amc_am.c has its own main()
// Instead, main.am calls amc_run() from this file via extern declaration.

// This file provides the C entry point that:
// 1. Parses argc/argv
// 2. Builds a List<string> of .am files
// 3. Calls Amalgame_Compiler_AmalgameCompiler_Run()

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "_runtime.h"
#include "Amalgame_String.h"
#include "Amalgame_Collections.h"
#include "Amalgame_IO.h"
#include "Amalgame_Net.h"
#include "Amalgame_Math.h"
#include "Amalgame_Console.h"

typedef struct _Amalgame_Compiler_AmalgameCompiler Amalgame_Compiler_AmalgameCompiler;

Amalgame_Compiler_AmalgameCompiler* Amalgame_Compiler_AmalgameCompiler_new(void);
void Amalgame_Compiler_AmalgameCompiler_Run(
    Amalgame_Compiler_AmalgameCompiler* self,
    AmalgameList* inputFiles,
    code_string outputName);
void Amalgame_Compiler_Program_Main(code_string* args);

int main(int argc, char** argv) {
    GC_INIT();

    if (argc < 2) {
        fprintf(stderr, "Usage: amc file1.am [file2.am ...] -o output\n");
        return 1;
    }

    AmalgameList* inputFiles = AmalgameList_new();
    const char* outputName = "a.out";
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 && i+1 < argc) { outputName = argv[++i]; }
        else if (strlen(argv[i]) > 3 && strcmp(argv[i]+strlen(argv[i])-3, ".am") == 0)
            AmalgameList_add(inputFiles, (void*)argv[i]);
    }

    int n = (int)AmalgameList_count(inputFiles);
    if (n == 0) { fprintf(stderr, "amc: no input .am files\n"); return 1; }

    Amalgame_Compiler_AmalgameCompiler* compiler = Amalgame_Compiler_AmalgameCompiler_new();
    Amalgame_Compiler_AmalgameCompiler_Run(compiler, inputFiles, (code_string)outputName);
    return 0;
}
