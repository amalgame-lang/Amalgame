// amc_bootstrap_main.c — entry point for amc_bootstrap (pure C, no Amalgame delegation)
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

typedef struct _Amalgame_Compiler_CGen Amalgame_Compiler_CGen;
Amalgame_Compiler_CGen* Amalgame_Compiler_CGen_new(void);
void  Amalgame_Compiler_CGen_BeginMulti(Amalgame_Compiler_CGen*, code_string);
void  Amalgame_Compiler_CGen_AddFilePass1(Amalgame_Compiler_CGen*, void*);
void  Amalgame_Compiler_CGen_EmitSeparator(Amalgame_Compiler_CGen*);
void  Amalgame_Compiler_CGen_AddFilePass2(Amalgame_Compiler_CGen*, void*);
AmalgameList* Amalgame_Compiler_CGen_GetLines(Amalgame_Compiler_CGen*);

typedef struct _Amalgame_Compiler_Lexer Amalgame_Compiler_Lexer;
Amalgame_Compiler_Lexer* Amalgame_Compiler_Lexer_new(code_string, code_string);
AmalgameList* Amalgame_Compiler_Lexer_Tokenize(Amalgame_Compiler_Lexer*);

typedef struct _Amalgame_Compiler_Parser Amalgame_Compiler_Parser;
Amalgame_Compiler_Parser* Amalgame_Compiler_Parser_new(AmalgameList*);
void* Amalgame_Compiler_Parser_Parse(Amalgame_Compiler_Parser*);

static code_string read_file(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) { fprintf(stderr, "Cannot open: %s\n", path); return ""; }
    fseek(f, 0, SEEK_END); long sz = ftell(f); rewind(f);
    char* buf = (char*)GC_MALLOC(sz + 1);
    fread(buf, 1, sz, f); buf[sz] = 0; fclose(f);
    return buf;
}

static code_string detect_namespace(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) return "App";
    char line[256] = {0};
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "namespace ", 10) == 0) {
            fclose(f);
            char* ns = line + 10;
            int len = strlen(ns);
            while (len > 0 && (ns[len-1]==10||ns[len-1]==13||ns[len-1]==32)) ns[--len]=0;
            char* r = (char*)GC_MALLOC(len+1);
            for (int i=0;i<len;i++) r[i]=(ns[i]==46)?95:ns[i];
            r[len]=0; return r;
        }
    }
    fclose(f); return "App";
}

int main(int argc, char** argv) {
    GC_INIT();
    if (argc < 2) { fprintf(stderr,"Usage: amc_bootstrap file.am ... -o out\n"); return 1; }

    AmalgameList* inputFiles = AmalgameList_new();
    const char* outputName = "a.out";
    for (int i=1; i<argc; i++) {
        if (strcmp(argv[i],"-o")==0 && i+1<argc) { outputName=argv[++i]; }
        else if (strlen(argv[i])>3 && strcmp(argv[i]+strlen(argv[i])-3,".am")==0)
            AmalgameList_add(inputFiles,(void*)argv[i]);
    }
    int n = (int)AmalgameList_count(inputFiles);
    if (n==0) { fprintf(stderr,"amc: no input .am files\n"); return 1; }

    printf("Compiling: %d file(s)\n", n);

    code_string ns = detect_namespace((const char*)AmalgameList_get(inputFiles,0));
    Amalgame_Compiler_CGen* gen = Amalgame_Compiler_CGen_new();
    Amalgame_Compiler_CGen_BeginMulti(gen, ns);

    AmalgameList* progs = AmalgameList_new();
    for (int i=0; i<n; i++) {
        const char* path = (const char*)AmalgameList_get(inputFiles,i);
        code_string src = read_file(path);
        Amalgame_Compiler_Lexer* lex = Amalgame_Compiler_Lexer_new(src,(code_string)path);
        AmalgameList* toks = Amalgame_Compiler_Lexer_Tokenize(lex);
        Amalgame_Compiler_Parser* par = Amalgame_Compiler_Parser_new(toks);
        void* prog = Amalgame_Compiler_Parser_Parse(par);
        AmalgameList_add(progs, prog);
        Amalgame_Compiler_CGen_AddFilePass1(gen, prog);
    }
    Amalgame_Compiler_CGen_EmitSeparator(gen);
    for (int j=0; j<n; j++)
        Amalgame_Compiler_CGen_AddFilePass2(gen, AmalgameList_get(progs,j));

    AmalgameList* lines = Amalgame_Compiler_CGen_GetLines(gen);
    int lc = (int)AmalgameList_count(lines);

    char outPath[512]; snprintf(outPath,sizeof(outPath),"%s.c",outputName);
    FILE* out = fopen(outPath,"w");
    if (!out) { fprintf(stderr,"Cannot write: %s\n",outPath); return 1; }
    for (int i=0; i<lc; i++) {
        const char* line = (const char*)AmalgameList_get(lines,i);
        if (line) fputs(line,out);
        fputc('\n',out);
    }
    fclose(out);

    // Detect and emit main()
    char mainFunc[256]; snprintf(mainFunc,sizeof(mainFunc),"%s_Program_Main",ns);
    FILE* check = fopen(outPath,"r"); int hasMain=0;
    if (check) { char lb[512];
        while(fgets(lb,sizeof(lb),check)) if(strstr(lb,mainFunc)){hasMain=1;break;}
        fclose(check); }
    if (hasMain) {
        FILE* out2=fopen(outPath,"a");
        if(out2){ fprintf(out2,"\nint main(int argc,char** argv){\n    GC_INIT();\n    %s((code_string*)argv);\n    return 0;\n}\n",mainFunc); fclose(out2); }
    }

    printf("Generated: %s (%d lines)\n", outPath, lc+(hasMain?5:0));
    return 0;
}
