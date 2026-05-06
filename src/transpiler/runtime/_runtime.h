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

/* Math constants — only if Amalgame_Math.h not included */
#ifndef AMALGAME_MATH_H
#define Math_PI    3.14159265358979323846
#define Math_Max(a,b) ((a)>(b)?(a):(b))
#define Math_Min(a,b) ((a)<(b)?(a):(b))
#endif

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

/* ── Collection helpers (lambda-compatible) ── */

typedef void* (*AmalgamePredicate)(void*);
typedef void  (*AmalgameAction)(void*);

static inline void AmalgameList_forEach(AmalgameList* l, AmalgameAction fn) {
    for (int i = 0; i < l->size; i++)
        fn(l->data[i]);
}

static inline AmalgameList* AmalgameList_where(AmalgameList* l, AmalgamePredicate fn) {
    AmalgameList* result = AmalgameList_new();
    for (int i = 0; i < l->size; i++)
        if (fn(l->data[i])) AmalgameList_add(result, l->data[i]);
    return result;
}

static inline AmalgameList* AmalgameList_select(AmalgameList* l, AmalgamePredicate fn) {
    AmalgameList* result = AmalgameList_new();
    for (int i = 0; i < l->size; i++)
        AmalgameList_add(result, fn(l->data[i]));
    return result;
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

#endif /* CODE_RUNTIME_H */
