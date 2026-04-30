/*
 * Amalgame Standard Library — Amalgame.String
 * Copyright (c) 2026 Bastien MOUGET
 * https://github.com/BastienMOUGET/Amalgame
 *
 * Provides: String manipulation functions
 */

#ifndef AMALGAME_STRING_H
#define AMALGAME_STRING_H

#include "_runtime.h"
#include <ctype.h>

/* ─────────────────────────────────────────────
   Length & info
   ───────────────────────────────────────────── */

static inline i64 String_Length(code_string s) {
    return s ? (i64) strlen(s) : 0;
}

static inline code_bool String_IsEmpty(code_string s) {
    return !s || s[0] == '\0';
}

static inline code_bool String_IsWhitespace(code_string s) {
    if (!s) return true;
    for (const char* p = s; *p; p++)
        if (!isspace((unsigned char)*p)) return false;
    return true;
}

/* ─────────────────────────────────────────────
   Search
   ───────────────────────────────────────────── */

static inline code_bool String_Contains(code_string s,
                                         code_string sub) {
    return s && sub && strstr(s, sub) != NULL;
}

static inline code_bool String_StartsWith(code_string s,
                                           code_string prefix) {
    if (!s || !prefix) return false;
    return strncmp(s, prefix, strlen(prefix)) == 0;
}

static inline code_bool String_EndsWith(code_string s,
                                         code_string suffix) {
    if (!s || !suffix) return false;
    size_t ls = strlen(s), lp = strlen(suffix);
    if (lp > ls) return false;
    return strcmp(s + ls - lp, suffix) == 0;
}

static inline i64 String_IndexOf(code_string s, code_string sub) {
    if (!s || !sub) return -1;
    const char* p = strstr(s, sub);
    return p ? (i64)(p - s) : -1;
}

static inline i64 String_LastIndexOf(code_string s,
                                      code_string sub) {
    if (!s || !sub) return -1;
    size_t ls = strlen(s), lsub = strlen(sub);
    if (lsub > ls) return -1;
    for (i64 i = (i64)(ls - lsub); i >= 0; i--)
        if (strncmp(s + i, sub, lsub) == 0) return i;
    return -1;
}

/* ─────────────────────────────────────────────
   Substrings
   ───────────────────────────────────────────── */

static inline code_string String_Substring(code_string s,
                                            i64 start,
                                            i64 len) {
    if (!s) return "";
    i64 slen = (i64) strlen(s);
    if (start < 0) start = 0;
    if (start >= slen) return "";
    if (len < 0 || start + len > slen) len = slen - start;
    char* r = (char*) GC_MALLOC(len + 1);
    memcpy(r, s + start, len);
    r[len] = '\0';
    return r;
}

static inline code_string String_From(code_string s, i64 start) {
    return String_Substring(s, start, -1);
}

static inline code_string String_Until(code_string s, i64 end) {
    return String_Substring(s, 0, end);
}

/* ─────────────────────────────────────────────
   Case conversion
   ───────────────────────────────────────────── */

static inline code_string String_ToUpper(code_string s) {
    if (!s) return "";
    size_t len = strlen(s);
    char* r = (char*) GC_MALLOC(len + 1);
    for (size_t i = 0; i <= len; i++)
        r[i] = (char) toupper((unsigned char) s[i]);
    return r;
}

static inline code_string String_ToLower(code_string s) {
    if (!s) return "";
    size_t len = strlen(s);
    char* r = (char*) GC_MALLOC(len + 1);
    for (size_t i = 0; i <= len; i++)
        r[i] = (char) tolower((unsigned char) s[i]);
    return r;
}

/* ─────────────────────────────────────────────
   Trim
   ───────────────────────────────────────────── */

static inline code_string String_TrimStart(code_string s) {
    if (!s) return "";
    while (*s && isspace((unsigned char)*s)) s++;
    return code_strdup(s);
}

static inline code_string String_TrimEnd(code_string s) {
    if (!s) return "";
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char) s[len-1])) len--;
    char* r = (char*) GC_MALLOC(len + 1);
    memcpy(r, s, len);
    r[len] = '\0';
    return r;
}

static inline code_string String_Trim(code_string s) {
    return String_TrimEnd(String_TrimStart(s));
}

/* ─────────────────────────────────────────────
   Replace
   ───────────────────────────────────────────── */

static inline code_string String_Replace(code_string s,
                                          code_string from,
                                          code_string to) {
    if (!s || !from || !to) return s;
    size_t lf = strlen(from), lt = strlen(to);
    if (lf == 0) return code_strdup(s);

    /* Count occurrences */
    int count = 0;
    const char* p = s;
    while ((p = strstr(p, from)) != NULL) { count++; p += lf; }

    size_t ls = strlen(s);
    size_t rlen = ls + (size_t)count * (lt - lf) + 1;
    char* r = (char*) GC_MALLOC(rlen);
    char* w = r;
    p = s;

    const char* q;
    while ((q = strstr(p, from)) != NULL) {
        size_t chunk = (size_t)(q - p);
        memcpy(w, p, chunk); w += chunk;
        memcpy(w, to, lt);   w += lt;
        p = q + lf;
    }
    strcpy(w, p);
    return r;
}

/* ─────────────────────────────────────────────
   Split & Join
   ───────────────────────────────────────────── */

static inline AmalgameList* String_Split(code_string s,
                                      code_string sep) {
    AmalgameList* list = AmalgameList_new();
    if (!s || !sep || sep[0] == '\0') {
        AmalgameList_add(list, (void*) code_strdup(s ? s : ""));
        return list;
    }
    size_t lsep = strlen(sep);
    const char* p = s;
    const char* q;
    while ((q = strstr(p, sep)) != NULL) {
        size_t len = (size_t)(q - p);
        char* part = (char*) GC_MALLOC(len + 1);
        memcpy(part, p, len);
        part[len] = '\0';
        AmalgameList_add(list, (void*) part);
        p = q + lsep;
    }
    AmalgameList_add(list, (void*) code_strdup(p));
    return list;
}

static inline code_string String_Join(AmalgameList* parts,
                                       code_string sep) {
    if (!parts || parts->size == 0) return "";
    size_t lsep = sep ? strlen(sep) : 0;
    size_t total = 0;
    for (int i = 0; i < parts->size; i++)
        total += strlen((code_string) parts->data[i]);
    total += lsep * (size_t)(parts->size > 0 ? parts->size - 1 : 0);

    char* r = (char*) GC_MALLOC(total + 1);
    char* w = r;
    for (int i = 0; i < parts->size; i++) {
        if (i > 0 && sep) {
            memcpy(w, sep, lsep); w += lsep;
        }
        code_string part = (code_string) parts->data[i];
        size_t len = strlen(part);
        memcpy(w, part, len); w += len;
    }
    *w = '\0';
    return r;
}

/* ─────────────────────────────────────────────
   Repeat & Pad
   ───────────────────────────────────────────── */

static inline code_string String_Repeat(code_string s, i64 n) {
    if (!s || n <= 0) return "";
    size_t ls = strlen(s);
    char* r = (char*) GC_MALLOC(ls * (size_t) n + 1);
    for (i64 i = 0; i < n; i++) memcpy(r + i * ls, s, ls);
    r[ls * (size_t) n] = '\0';
    return r;
}

static inline code_string String_PadLeft(code_string s,
                                          i64 width,
                                          char pad) {
    if (!s) s = "";
    i64 ls = (i64) strlen(s);
    if (ls >= width) return code_strdup(s);
    i64 extra = width - ls;
    char* r = (char*) GC_MALLOC((size_t) width + 1);
    memset(r, pad, (size_t) extra);
    memcpy(r + extra, s, (size_t) ls + 1);
    return r;
}

static inline code_string String_PadRight(code_string s,
                                           i64 width,
                                           char pad) {
    if (!s) s = "";
    i64 ls = (i64) strlen(s);
    if (ls >= width) return code_strdup(s);
    char* r = (char*) GC_MALLOC((size_t) width + 1);
    memcpy(r, s, (size_t) ls);
    memset(r + ls, pad, (size_t)(width - ls));
    r[width] = '\0';
    return r;
}

/* ─────────────────────────────────────────────
   Conversion
   ───────────────────────────────────────────── */

static inline i64 String_ToInt(code_string s) {
    return s ? (i64) strtoll(s, NULL, 10) : 0;
}

static inline f64 String_ToFloat(code_string s) {
    return s ? strtod(s, NULL) : 0.0;
}

static inline code_bool String_ToBool(code_string s) {
    return s && (strcmp(s, "true") == 0 || strcmp(s, "1") == 0);
}

static inline code_string String_FromInt(i64 n) {
    return code_int_to_string(n);
}

static inline code_string String_FromFloat(f64 n) {
    return code_float_to_string(n);
}

static inline code_string String_FromBool(code_bool b) {
    return b ? "true" : "false";
}

/* ─────────────────────────────────────────────
   Char access
   ───────────────────────────────────────────── */

static inline char String_CharAt(code_string s, i64 i) {
    if (!s) return '\0';
    i64 len = (i64) strlen(s);
    if (i < 0 || i >= len) return '\0';
    return s[i];
}

static inline code_bool String_IsDigit(char c) {
    return isdigit((unsigned char) c);
}

static inline code_bool String_IsAlpha(char c) {
    return isalpha((unsigned char) c);
}

static inline code_bool String_IsAlnum(char c) {
    return isalnum((unsigned char) c);
}

#endif /* AMALGAME_STRING_H */
