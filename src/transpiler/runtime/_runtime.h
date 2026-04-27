/*
 * CODE Language Runtime - v0.1.0
 * Copyright (c) 2024 NeitsabTeguom
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
#define code_strdup(s)    GC_STRDUP(s)

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
    size_t la = strlen(a), lb = strlen(b);
    char*  r  = (char*) GC_MALLOC(la + lb + 1);
    memcpy(r, a, la);
    memcpy(r + la, b, lb + 1);
    return r;
}

static inline bool code_string_equals(
    code_string a, code_string b) {
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

/* Math */
#define Math_PI    3.14159265358979323846
#define Math_Abs   fabs
#define Math_Sqrt  sqrt
#define Math_Pow   pow
#define Math_Max(a,b) ((a)>(b)?(a):(b))
#define Math_Min(a,b) ((a)<(b)?(a):(b))

/* List generique */
typedef struct {
    void** data;
    int    size;
    int    capacity;
} CodeList;

static inline CodeList* CodeList_new() {
    CodeList* l = (CodeList*) GC_MALLOC(sizeof(CodeList));
    l->capacity = 8;
    l->size     = 0;
    l->data     = (void**) GC_MALLOC(sizeof(void*) * 8);
    return l;
}

static inline void CodeList_add(CodeList* l, void* item) {
    if (l->size >= l->capacity) {
        l->capacity *= 2;
        void** nd = (void**) GC_MALLOC(
            sizeof(void*) * l->capacity);
        memcpy(nd, l->data, sizeof(void*) * l->size);
        l->data = nd;
    }
    l->data[l->size++] = item;
}

static inline void* CodeList_get(CodeList* l, int i) {
    if (i < 0 || i >= l->size) return NULL;
    return l->data[i];
}

static inline int CodeList_count(CodeList* l) {
    return l->size;
}

/* Result et Option */
typedef struct {
    bool        is_ok;
    void*       value;
    code_string error;
} CodeResult;

static inline CodeResult Result_Ok(void* v) {
    return (CodeResult){true, v, NULL};
}

static inline CodeResult Result_Error(code_string e) {
    return (CodeResult){false, NULL, e};
}

typedef struct {
    bool  has_value;
    void* value;
} CodeOption;

static inline CodeOption Option_Some(void* v) {
    return (CodeOption){true, v};
}

static inline CodeOption Option_None() {
    return (CodeOption){false, NULL};
}

/* Init runtime */
static inline void code_runtime_init() {
    GC_INIT();
}

#endif /* CODE_RUNTIME_H */
