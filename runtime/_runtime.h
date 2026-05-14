/*
 * CODE Language Runtime - v0.1.0
 * Copyright (c) 2026 Bastien MOUGET
 */

#ifndef CODE_RUNTIME_H
#define CODE_RUNTIME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <math.h>
#include <gc.h>

/* Types de base */
typedef int64_t  i64;
typedef int32_t  i32;
typedef double   f64;
typedef float    f32;
typedef uint8_t  u8;
typedef char*    code_string;
typedef bool     code_bool;

/* Allocation GC */
#define code_alloc(size)  GC_MALLOC(size)
#define code_strdup(s)    (s ? (char*)memcpy(GC_MALLOC(strlen(s)+1), (s), strlen(s)+1) : NULL)

/* String helpers */
static inline code_string code_string_format(
    const char* fmt, ...) {
    va_list a1, a2;
    va_start(a1, fmt);
    va_copy(a2, a1);
    int len = vsnprintf(NULL, 0, fmt, a1);
    va_end(a1);
    char* buf = (char*) GC_MALLOC(len + 1);
    vsnprintf(buf, len + 1, fmt, a2);
    va_end(a2);
    return buf;
}

static inline code_string code_string_concat(
    code_string a, code_string b) {
    if (!a) a = "";
    if (!b) b = "";
    size_t la = strlen(a), lb = strlen(b);
    char*  r  = (char*) GC_MALLOC(la + lb + 1);
    memcpy(r, a, la);
    memcpy(r + la, b, lb + 1);
    return r;
}

static inline bool code_string_equals(
    code_string a, code_string b) {
    if (!a) a = "";
    if (!b) b = "";
    return strcmp(a, b) == 0;
}

static inline code_string code_int_to_string(i64 n) {
    char* buf = (char*) GC_MALLOC(32);
    snprintf(buf, 32, "%ld", n);
    return buf;
}

static inline code_string code_float_to_string(f64 n) {
    char* buf = (char*) GC_MALLOC(64);
    snprintf(buf, 64, "%g", n);
    return buf;
}

/* Console */
static inline void Console_WriteLine(code_string s) {
    printf("%s\n", s ? s : "");
}

static inline void Console_Write(code_string s) {
    printf("%s", s ? s : "");
}

static inline code_string Console_ReadLine() {
    char* buf = (char*) GC_MALLOC(4096);
    if (fgets(buf, 4096, stdin) == NULL) return "";
    size_t len = strlen(buf);
    if (len > 0 && buf[len-1] == '\n') buf[len-1] = 0;
    return buf;
}

/* Read exactly `n` bytes from stdin, returning a freshly-allocated
 * NUL-terminated string. Used by `amc lsp` to read the body of a
 * JSON-RPC message after parsing its `Content-Length` header.
 * Returns "" on EOF or n < 0.
 *
 * NOTE: this assumes the bytes are valid UTF-8 (or at least don't
 * contain embedded NULs the caller cares about). LSP messages are
 * always UTF-8 JSON, so that's fine in practice.
 */
static inline code_string Console_ReadBytes(i64 n) {
    if (n < 0) return "";
    if (n == 0) return code_strdup("");
    char* buf = (char*) GC_MALLOC((size_t)n + 1);
    size_t got = 0;
    while (got < (size_t)n) {
        size_t r = fread(buf + got, 1, (size_t)n - got, stdin);
        if (r == 0) break;
        got += r;
    }
    buf[got] = 0;
    return buf;
}

/* Flush stdout. LSP server must flush after each response so the
 * client doesn't block forever waiting for a buffered reply. */
static inline void Console_Flush(void) {
    fflush(stdout);
}

/* Math constants used to live here as fallback macros for users
 * who didn't #include "Amalgame_Math.h". With Math.h migrated to
 * src/stdlib/math.am (post-v0.7.5), the macros are gone — callers
 * use Math.PI() / Math.MaxF() etc. as qualified method calls. */

/* List generique */
typedef struct {
    void** data;
    int    size;
    int    capacity;
} AmalgameList;

static inline AmalgameList* AmalgameList_new() {
    AmalgameList* l = (AmalgameList*) GC_MALLOC(sizeof(AmalgameList));
    l->capacity = 64;
    l->size     = 0;
    l->data     = (void**) GC_MALLOC(sizeof(void*) * 64);
    return l;
}

static inline AmalgameList* AmalgameList_newWithCapacity(int cap) {
    AmalgameList* l = (AmalgameList*) GC_MALLOC(sizeof(AmalgameList));
    if (cap < 8) cap = 8;
    l->capacity = cap;
    l->size     = 0;
    l->data     = (void**) GC_MALLOC(sizeof(void*) * cap);
    return l;
}

static inline void AmalgameList_reserve(AmalgameList* l, int cap) {
    if (cap <= l->capacity) return;
    void** nd = (void**) GC_MALLOC(sizeof(void*) * cap);
    memcpy(nd, l->data, sizeof(void*) * l->size);
    l->data     = nd;
    l->capacity = cap;
}

static inline void AmalgameList_add(AmalgameList* l, void* item) {
    if (l->size >= l->capacity) {
        l->capacity *= 2;
        void** nd = (void**) GC_MALLOC(
            sizeof(void*) * l->capacity);
        memcpy(nd, l->data, sizeof(void*) * l->size);
        l->data = nd;
    }
    l->data[l->size++] = item;
}

static inline void* AmalgameList_get(AmalgameList* l, int i) {
    if (i < 0 || i >= l->size) return NULL;
    return l->data[i];
}

static inline int AmalgameList_count(AmalgameList* l) {
    return l->size;
}

/* ── Fast single-char string lookup (no GC allocation) ── */
/* Pre-allocated table of all 256 single-char strings */
static char* __amc_char_table[256] = {0};

static inline void __amc_init_char_table() {
    if (__amc_char_table[(unsigned char)'a']) return;
    for (int i = 0; i < 256; i++) {
        char* s = (char*) GC_MALLOC_ATOMIC(2);
        s[0] = (char)i;
        s[1] = 0;
        __amc_char_table[i] = s;
    }
}

static inline code_string String_CharAt1(code_string s, int i) {
    if (!s || i < 0) return "";
    int len = (int)strlen(s);
    if (i >= len) return "";
    __amc_init_char_table();
    return __amc_char_table[(unsigned char)s[i]];
}

/* ── Streaming file output (for fast CGen output) ── */
static FILE* __amc_stream_file = NULL;

static inline void File_OpenWrite(const char* path) {
    if (__amc_stream_file) { fclose(__amc_stream_file); }
    __amc_stream_file = fopen(path, "w");
}

static inline void File_StreamLine(const char* line) {
    if (!__amc_stream_file) return;
    if (line) { fputs(line, __amc_stream_file); }
    fputc('\n', __amc_stream_file);
}

static inline void File_CloseWrite() {
    if (__amc_stream_file) {
        fclose(__amc_stream_file);
        __amc_stream_file = NULL;
    }
}

/* ── File write helpers ── */
static inline void File_WriteLines(const char* path, AmalgameList* lines) {
    FILE* f = fopen(path, "w");
    if (!f) return;
    int count = (int)AmalgameList_count(lines);
    for (int i = 0; i < count; i++) {
        const char* line = (const char*)AmalgameList_get(lines, i);
        if (line) { fputs(line, f); }
        fputc('\n', f);
    }
    fclose(f);
}

/* ── Capturing closures (forward — full helpers below) ──
   The collection higher-order helpers a few lines down dispatch
   through AmalgameClosure_callN, so the closure type and the
   `_call1` / `_call2` wrappers must be in scope here. The full
   commentary block lives further down with the rest of the
   closure runtime. */

typedef struct AmalgameClosure {
    void* fn;
    void* env;
} AmalgameClosure;

typedef void* (*AmalgameClosure1Fn)(void*, void*);
typedef void* (*AmalgameClosure2Fn)(void*, void*, void*);
typedef void* (*AmalgameClosure3Fn)(void*, void*, void*, void*);

static inline AmalgameClosure* AmalgameClosure_new(void* fn, void* env) {
    AmalgameClosure* c = (AmalgameClosure*) code_alloc(sizeof(AmalgameClosure));
    c->fn  = fn;
    c->env = env;
    return c;
}

static inline void* AmalgameClosure_call1(AmalgameClosure* c, void* arg) {
    return ((AmalgameClosure1Fn)c->fn)(c->env, arg);
}

static inline void* AmalgameClosure_call2(AmalgameClosure* c, void* a, void* b) {
    return ((AmalgameClosure2Fn)c->fn)(c->env, a, b);
}

static inline void* AmalgameClosure_call3(AmalgameClosure* c, void* a, void* b, void* d) {
    return ((AmalgameClosure3Fn)c->fn)(c->env, a, b, d);
}

/* ── Collection helpers (lambda-compatible) ── */

/* Closure-aware higher-order helpers (since v0.3.6).
   These all dispatch through AmalgameClosure_callN so the captured
   environment of a lambda is threaded through. Items / accumulator /
   result are all `void*` boxed; the caller's CGen unbox via
   `(i64)(intptr_t)…` at the call site. */

static inline void AmalgameList_forEach(AmalgameList* l, AmalgameClosure* fn) {
    for (int i = 0; i < l->size; i++)
        AmalgameClosure_call1(fn, l->data[i]);
}

static inline AmalgameList* AmalgameList_filter(AmalgameList* l, AmalgameClosure* fn) {
    AmalgameList* result = AmalgameList_new();
    for (int i = 0; i < l->size; i++)
        if ((intptr_t)AmalgameClosure_call1(fn, l->data[i]))
            AmalgameList_add(result, l->data[i]);
    return result;
}

static inline AmalgameList* AmalgameList_map(AmalgameList* l, AmalgameClosure* fn) {
    AmalgameList* result = AmalgameList_new();
    for (int i = 0; i < l->size; i++)
        AmalgameList_add(result, AmalgameClosure_call1(fn, l->data[i]));
    return result;
}

/* Two-arg reducer: acc, item → new acc. Initial accumulator is
   passed boxed (void*); the caller unboxes the return at the call
   site. */
static inline void* AmalgameList_reduce(AmalgameList* l, void* init, AmalgameClosure* fn) {
    void* acc = init;
    for (int i = 0; i < l->size; i++)
        acc = AmalgameClosure_call2(fn, acc, l->data[i]);
    return acc;
}

static inline void* AmalgameList_first(AmalgameList* l) {
    return l->size > 0 ? l->data[0] : NULL;
}

static inline void* AmalgameList_last(AmalgameList* l) {
    return l->size > 0 ? l->data[l->size - 1] : NULL;
}

/* Result et Option */
typedef struct {
    bool        is_ok;
    void*       value;
    code_string error;
} AmalgameResult;

static inline AmalgameResult Result_Ok(void* v) {
    return (AmalgameResult){true, v, NULL};
}

static inline AmalgameResult Result_Error(code_string e) {
    return (AmalgameResult){false, NULL, e};
}

typedef struct {
    bool  has_value;
    void* value;
} AmalgameOption;

static inline AmalgameOption Option_Some(void* v) {
    return (AmalgameOption){true, v};
}

static inline AmalgameOption Option_None() {
    return (AmalgameOption){false, NULL};
}

/* Init runtime */
static inline void code_runtime_init() {
    GC_INIT();
}

/* Process argv/argc + exit code — set by the generated `int main()` entry,
 * read from Amalgame via the Args_* / Exit_* builtins below. */
static int    code_argc      = 0;
static char** code_argv      = NULL;
static int    code_exit_code = 0;

static inline void code_runtime_init_args(int argc, char** argv) {
    code_argc = argc;
    code_argv = argv;
}

/* Build the AmalgameList<string> the generated `int main()` passes to
 * `Program.Main(List<string> args)` user code. argv[0] is the program
 * name (already reachable via Args.Get(0)); skip it so `args` matches
 * the .NET/Kotlin convention where `args[0]` is the first user arg. */
static inline AmalgameList* code_runtime_args_list(void) {
    int n = code_argc > 0 ? code_argc - 1 : 0;
    AmalgameList* l = AmalgameList_newWithCapacity(n > 0 ? n : 8);
    for (int i = 1; i < code_argc; i++) {
        AmalgameList_add(l, (void*) code_argv[i]);
    }
    return l;
}

/* Args (zero-indexed; index 0 is the program name like in C). */
static inline i64 Args_Count(void) {
    return (i64)code_argc;
}

static inline code_string Args_Get(i64 i) {
    if (i < 0 || i >= (i64)code_argc) return "";
    return (code_string)code_argv[i];
}

/* Exit code — Amalgame programs call Exit_Set(n) to set the process
 * exit status; the generated int main() returns it. */
static inline void Exit_Set(i64 n) {
    code_exit_code = (int)n;
}

static inline i64 Exit_Get(void) {
    return (i64)code_exit_code;
}


/* ================================================================
   Exception support — setjmp/longjmp based
   ================================================================ */
#include <setjmp.h>

typedef struct _AmalgameException {
    jmp_buf     env;
    void*       value;     /* thrown object */
    code_string type;      /* type name as string */
    code_string message;   /* error message if available */
    int         active;    /* 1 if exception is in flight */
} AmalgameException;

/* Thread-local exception state */
static AmalgameException _am_ex;  /* zero-initialized by default (static storage) */

/* Throw: save value and longjmp */
static inline void _am_throw(void* val, code_string type,
                               code_string msg) {
    _am_ex.value   = val;
    _am_ex.type    = type  ? type  : "Error";
    _am_ex.message = msg   ? msg   : "";
    _am_ex.active  = 1;
    longjmp(_am_ex.env, 1);
}

/* Get message from thrown object — tries .Message field */
#define _AM_EX_MSG() (_am_ex.message)

/* ================================================================
   Capturing closures
   ================================================================
   A lambda value is a pair (fn, env): a function pointer and a
   heap-allocated env struct holding captured locals. The CGen emits
   one struct + one top-level function per lambda, then allocates
   the env at the creation site and bundles them in an AmalgameClosure.

   v2 supports arities 1, 2, 3 — covers single-param, predicate-style
   ((acc,x) => …, (a,b) => …), and reducer/sorter-style lambdas. arg
   and result are all void* (boxed); the call site casts back to the
   right type. The struct, typedefs, and `_callN` wrappers are
   declared up at the top of the file so the collection higher-order
   helpers (`AmalgameList_filter` & co.) can use them.
*/

#endif /* CODE_RUNTIME_H */
