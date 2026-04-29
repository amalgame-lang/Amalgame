/*
 * Amalgame Standard Library — Amalgame.IO
 * Copyright (c) 2026 Bastien MOUGET
 * https://github.com/BastienMOUGET/Amalgame
 *
 * Provides: Console, File, Path, Environment
 */

#ifndef AMALGAME_IO_H
#define AMALGAME_IO_H

#include "_runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

/* ─────────────────────────────────────────────
   Console
   ───────────────────────────────────────────── */

/* Already in _runtime.h:
   Console_WriteLine(code_string)
   Console_Write(code_string)
   Console_ReadLine() → code_string
*/

static inline void Console_WriteError(code_string s) {
    fprintf(stderr, "%s\n", s ? s : "");
}

static inline code_string Console_ReadPassword() {
    /* Simple implementation — no echo disable on all platforms */
    return Console_ReadLine();
}

static inline void Console_Clear() {
#ifdef _WIN32
    system("cls");
#else
    printf("\033[2J\033[H");
#endif
}

/* ─────────────────────────────────────────────
   File
   ───────────────────────────────────────────── */

static inline code_string File_ReadAll(code_string path) {
    FILE* f = fopen(path, "r");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char* buf = (char*) GC_MALLOC(size + 1);
    fread(buf, 1, size, f);
    buf[size] = '\0';
    fclose(f);
    return buf;
}

static inline code_bool File_WriteAll(code_string path,
                                       code_string content) {
    FILE* f = fopen(path, "w");
    if (!f) return false;
    fputs(content, f);
    fclose(f);
    return true;
}

static inline code_bool File_AppendAll(code_string path,
                                        code_string content) {
    FILE* f = fopen(path, "a");
    if (!f) return false;
    fputs(content, f);
    fclose(f);
    return true;
}

static inline code_bool File_Exists(code_string path) {
    struct stat st;
    return stat(path, &st) == 0;
}

static inline code_bool File_Delete(code_string path) {
    return remove(path) == 0;
}

static inline i64 File_Size(code_string path) {
    struct stat st;
    if (stat(path, &st) != 0) return -1;
    return (i64) st.st_size;
}

/* ─────────────────────────────────────────────
   Path
   ───────────────────────────────────────────── */

static inline code_string Path_Combine(code_string a,
                                        code_string b) {
    size_t la = strlen(a);
    size_t lb = strlen(b);
    char* r = (char*) GC_MALLOC(la + lb + 2);
    memcpy(r, a, la);
    if (la > 0 && a[la-1] != '/' && a[la-1] != '\\')
        r[la++] = '/';
    memcpy(r + la, b, lb + 1);
    return r;
}

static inline code_string Path_GetExtension(code_string path) {
    const char* dot = strrchr(path, '.');
    if (!dot || dot == path) return "";
    return code_strdup(dot);
}

static inline code_string Path_GetFilename(code_string path) {
    const char* slash = strrchr(path, '/');
    if (!slash) slash = strrchr(path, '\\');
    if (!slash) return code_strdup(path);
    return code_strdup(slash + 1);
}

static inline code_string Path_GetDirectory(code_string path) {
    const char* slash = strrchr(path, '/');
    if (!slash) slash = strrchr(path, '\\');
    if (!slash) return ".";
    size_t len = (size_t)(slash - path);
    char* r = (char*) GC_MALLOC(len + 1);
    memcpy(r, path, len);
    r[len] = '\0';
    return r;
}

/* ─────────────────────────────────────────────
   Environment
   ───────────────────────────────────────────── */

static inline code_string Environment_GetVar(code_string name) {
    const char* val = getenv(name);
    return val ? code_strdup(val) : NULL;
}

static inline code_string Environment_GetVarOr(code_string name,
                                                code_string fallback) {
    const char* val = getenv(name);
    return val ? code_strdup(val) : fallback;
}

static inline code_bool Environment_HasVar(code_string name) {
    return getenv(name) != NULL;
}

#endif /* AMALGAME_IO_H */
