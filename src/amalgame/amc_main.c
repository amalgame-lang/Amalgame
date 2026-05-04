// amc_bootstrap — hand-written C entry point
// Fix: cast away const for Lexer_new

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "_runtime.h"
#include "Amalgame_String.h"
#include "Amalgame_Collections.h"
#include "Amalgame_IO.h"

void Amalgame_Compiler_Console_WriteLine(code_string s) { printf("%s\n", s ? s : ""); }
void Amalgame_Compiler_Console_WriteError(code_string s) { fprintf(stderr, "%s\n", s ? s : ""); }

typedef struct _Amalgame_Compiler_CGen    Amalgame_Compiler_CGen;
typedef struct _Amalgame_Compiler_Lexer   Amalgame_Compiler_Lexer;
typedef struct _Amalgame_Compiler_Parser  Amalgame_Compiler_Parser;
typedef struct _Amalgame_Compiler_AstNode Amalgame_Compiler_AstNode;

Amalgame_Compiler_CGen*    Amalgame_Compiler_CGen_new(void);
void                       Amalgame_Compiler_CGen_BeginMulti(Amalgame_Compiler_CGen*, code_string);
void                       Amalgame_Compiler_CGen_AddFilePass1(Amalgame_Compiler_CGen*, Amalgame_Compiler_AstNode*);
void                       Amalgame_Compiler_CGen_EmitSeparator(Amalgame_Compiler_CGen*);
void                       Amalgame_Compiler_CGen_AddFilePass2(Amalgame_Compiler_CGen*, Amalgame_Compiler_AstNode*);
AmalgameList*              Amalgame_Compiler_CGen_GetLines(Amalgame_Compiler_CGen*);
Amalgame_Compiler_Lexer*   Amalgame_Compiler_Lexer_new(code_string, code_string);
AmalgameList*              Amalgame_Compiler_Lexer_Tokenize(Amalgame_Compiler_Lexer*);
Amalgame_Compiler_Parser*  Amalgame_Compiler_Parser_new(AmalgameList*);
Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_Parse(Amalgame_Compiler_Parser*);

static Amalgame_Compiler_AstNode* parse_file(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) { fprintf(stderr, "Cannot open: %s\n", path); return NULL; }
    fseek(f, 0, SEEK_END); long sz = ftell(f); rewind(f);
    char* src = (char*)GC_MALLOC(sz + 1);
    fread(src, 1, sz, f); src[sz] = 0; fclose(f);
    Amalgame_Compiler_Lexer*   lex  = Amalgame_Compiler_Lexer_new(src, (char*)path);
    AmalgameList*              toks = Amalgame_Compiler_Lexer_Tokenize(lex);
    Amalgame_Compiler_Parser*  par  = Amalgame_Compiler_Parser_new(toks);
    return Amalgame_Compiler_Parser_Parse(par);
}

int main(int argc, char** argv) {
    GC_INIT();
    if (argc < 2) { fprintf(stderr, "Usage: amc_bootstrap file.am ... -o output\n"); return 1; }

    AmalgameList* inputFiles = AmalgameList_new();
    const char* outputName = "a.out";
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 && i+1 < argc) { outputName = argv[++i]; }
        else if (strlen(argv[i]) > 3 && strcmp(argv[i]+strlen(argv[i])-3, ".am") == 0)
            AmalgameList_add(inputFiles, argv[i]);
    }
    int n = (int)AmalgameList_count(inputFiles);
    if (n == 0) { fprintf(stderr, "amc: no input .am files\n"); return 1; }
    printf("Compiling: %d file(s)\n", n);

    Amalgame_Compiler_CGen* gen = Amalgame_Compiler_CGen_new();
    Amalgame_Compiler_CGen_BeginMulti(gen, "Amalgame.Compiler");
    for (int i = 0; i < n; i++) {
        Amalgame_Compiler_AstNode* p = parse_file((char*)AmalgameList_get(inputFiles,i));
        if (!p) return 1;
        Amalgame_Compiler_CGen_AddFilePass1(gen, p);
    }
    Amalgame_Compiler_CGen_EmitSeparator(gen);
    for (int i = 0; i < n; i++) {
        Amalgame_Compiler_AstNode* p = parse_file((char*)AmalgameList_get(inputFiles,i));
        if (!p) return 1;
        Amalgame_Compiler_CGen_AddFilePass2(gen, p);
    }

    char outPath[512];
    snprintf(outPath, sizeof(outPath), "%s.c", outputName);
    FILE* out = fopen(outPath, "w");
    if (!out) { fprintf(stderr, "Cannot write: %s\n", outPath); return 1; }
    AmalgameList* lines = Amalgame_Compiler_CGen_GetLines(gen);
    int lc = (int)AmalgameList_count(lines);
    for (int i = 0; i < lc; i++) {
        const char* line = (const char*)AmalgameList_get(lines, i);
        if (line) fputs(line, out);
        fputc('\n', out);
    }
    fclose(out);
    printf("Generated: %s (%d lines)\n", outPath, lc);
    return 0;
}
