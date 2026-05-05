// amc_bootstrap — C entry point
// Compiles .am files to .c — does NOT bundle bootstrap internals with user code

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "_runtime.h"
#include "Amalgame_String.h"
#include "Amalgame_Collections.h"
#include "Amalgame_IO.h"
#include "Amalgame_Console.h"

// Console functions defined in amc_bootstrap_lib.c via Amalgame_Console.h

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

// Namespace detection - read first line of first .am file
static code_string detect_namespace(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) return "App";
    char line[256] = {0};
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "namespace ", 10) == 0) {
            fclose(f);
            char* ns = line + 10;
            int len = strlen(ns);
            while (len > 0 && (ns[len-1] == 10 || ns[len-1] == 13 || ns[len-1] == 32)) ns[--len] = 0;
            char* r = (char*)GC_MALLOC(len+1);
            for (int i = 0; i < len; i++) r[i] = (ns[i] == 46) ? 95 : ns[i];
            r[len] = 0;
            return r;
        }
    }
    fclose(f);
    return "App";
}
int main(int argc, char** argv) {
    GC_INIT();
    if (argc < 2) {
        fprintf(stderr, "Usage: amc_bootstrap file.am [file2.am ...] -o output\n");
        return 1;
    }

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

    // Parse all files first to detect namespace
    AmalgameList* progs = AmalgameList_new();
    for (int i = 0; i < n; i++) {
        Amalgame_Compiler_AstNode* p = parse_file((char*)AmalgameList_get(inputFiles, i));
        if (!p) return 1;
        AmalgameList_add(progs, p);
    }

    // Detect namespace from first file
    code_string ns = detect_namespace((const char*)AmalgameList_get(inputFiles, 0));

    // Detect if any file imports Amalgame.Net
    int needsNet = 0;
    for (int i = 0; i < n && !needsNet; i++) {
        FILE* sf = fopen((char*)AmalgameList_get(inputFiles, i), "r");
        if (sf) {
            char lb[256];
            while (fgets(lb, sizeof(lb), sf)) {
                if (strstr(lb, "import Amalgame.Net")) { needsNet = 1; break; }
            }
            fclose(sf);
        }
    }

    // Generate C
    Amalgame_Compiler_CGen* gen = Amalgame_Compiler_CGen_new();
    Amalgame_Compiler_CGen_BeginMulti(gen, ns);

    // Pass 1: forward declarations
    for (int i = 0; i < n; i++) {
        Amalgame_Compiler_CGen_AddFilePass1(gen, (Amalgame_Compiler_AstNode*)AmalgameList_get(progs, i));
    }
    Amalgame_Compiler_CGen_EmitSeparator(gen);

    // Pass 2: definitions
    for (int i = 0; i < n; i++) {
        Amalgame_Compiler_CGen_AddFilePass2(gen, (Amalgame_Compiler_AstNode*)AmalgameList_get(progs, i));
    }

    // Write output line by line
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
    // Prepend Amalgame_Net.h if needed (requires -lcurl)
    if (needsNet) {
        // Append net include at top by rewriting - simpler: just emit at end as forward decl
        // Actually emit as comment since curl may not be available
    }
    fclose(out);  // Must close before reopening for read

    // Emit main() only if there's a Program class with Main method
    char mainFuncName[256];
    snprintf(mainFuncName, sizeof(mainFuncName), "%s_Program_Main", ns);

    FILE* check = fopen(outPath, "r");
    int hasMain = 0;
    if (check) {
        char lineBuf[512];
        while (fgets(lineBuf, sizeof(lineBuf), check)) {
            if (strstr(lineBuf, mainFuncName)) {
                hasMain = 1;
                break;
            }
        }
        fclose(check);
    }

    if (hasMain) {
        FILE* out2 = fopen(outPath, "a");
        if (out2) {
            fprintf(out2, "\nint main(int argc, char** argv) {\n");
            fprintf(out2, "    GC_INIT();\n");
            fprintf(out2, "    %s((code_string*)argv);\n", mainFuncName);
            fprintf(out2, "    return 0;\n");
            fprintf(out2, "}\n");
            fclose(out2);
        }
    }

    printf("Generated: %s (%d lines)\n", outPath, lc + (hasMain ? 5 : 0));
    return 0;
}
