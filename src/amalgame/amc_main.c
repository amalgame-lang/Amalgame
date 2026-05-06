// amc — C entry point: parses CLI args and calls AmalgameCompiler.Run()

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>   // isatty()
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
void Amalgame_Compiler_AmalgameCompiler_SetLib(
    Amalgame_Compiler_AmalgameCompiler* self, int v);
void Amalgame_Compiler_AmalgameCompiler_SetCheckOnly(
    Amalgame_Compiler_AmalgameCompiler* self, int v);
void Amalgame_Compiler_AmalgameCompiler_SetVerbose(
    Amalgame_Compiler_AmalgameCompiler* self, int v);
void Amalgame_Compiler_AmalgameCompiler_SetColor(
    Amalgame_Compiler_AmalgameCompiler* self, int v);
long Amalgame_Compiler_AmalgameCompiler_GetExitCode(
    Amalgame_Compiler_AmalgameCompiler* self);
void Amalgame_Compiler_Program_Main(code_string* args);

static void print_usage(void) {
    fprintf(stderr,
        "Usage: amc [options] file1.am [file2.am ...] -o <output>\n"
        "\n"
        "Options:\n"
        "  -o <output>   Output file (default: a.out)\n"
        "  --lib         Compile as library (no main() emitted)\n"
        "  --check       Type-check only, no code generation\n"
        "  --color       Force ANSI color output\n"
        "  --no-color    Disable ANSI color output\n"
        "  --quiet       Suppress progress messages\n"
        "  --verbose     Print extra build info\n"
        "  --version     Print version and exit\n"
        "  --help        Print this help\n"
    );
}

int main(int argc, char** argv) {
    GC_INIT();

    if (argc < 2) { print_usage(); return 1; }

    AmalgameList* inputFiles = AmalgameList_new();
    const char*   outputName = "a.out";
    int           isLib      = 0;
    int           checkOnly  = 0;
    int           useColor   = isatty(STDERR_FILENO);  // auto-detect TTY
    int           verbose    = 1;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 && i+1 < argc) {
            outputName = argv[++i];
        } else if (strcmp(argv[i], "--lib") == 0) {
            isLib = 1;
        } else if (strcmp(argv[i], "--check") == 0) {
            checkOnly = 1;
        } else if (strcmp(argv[i], "--color") == 0) {
            useColor = 1;
        } else if (strcmp(argv[i], "--no-color") == 0) {
            useColor = 0;
        } else if (strcmp(argv[i], "--quiet") == 0) {
            verbose = 0;
        } else if (strcmp(argv[i], "--verbose") == 0) {
            verbose = 1;
        } else if (strcmp(argv[i], "--version") == 0) {
            printf("amc 0.1.0 (self-hosted Amalgame compiler)\n");
            return 0;
        } else if (strcmp(argv[i], "--help") == 0 ||
                   strcmp(argv[i], "-h") == 0) {
            print_usage();
            return 0;
        } else if (strlen(argv[i]) > 3 &&
                   strcmp(argv[i] + strlen(argv[i]) - 3, ".am") == 0) {
            AmalgameList_add(inputFiles, (void*)argv[i]);
        } else {
            fprintf(stderr, "amc: unknown option '%s'\n", argv[i]);
            print_usage();
            return 1;
        }
    }

    if ((int)AmalgameList_count(inputFiles) == 0) {
        fprintf(stderr, "amc: no input .am files\n");
        return 1;
    }

    Amalgame_Compiler_AmalgameCompiler* compiler =
        Amalgame_Compiler_AmalgameCompiler_new();

    Amalgame_Compiler_AmalgameCompiler_SetLib(compiler, isLib);
    Amalgame_Compiler_AmalgameCompiler_SetCheckOnly(compiler, checkOnly);
    Amalgame_Compiler_AmalgameCompiler_SetColor(compiler, useColor);
    Amalgame_Compiler_AmalgameCompiler_SetVerbose(compiler, verbose);
    Amalgame_Compiler_AmalgameCompiler_Run(compiler, inputFiles,
                                           (code_string)outputName);

    return (int)Amalgame_Compiler_AmalgameCompiler_GetExitCode(compiler);
}
