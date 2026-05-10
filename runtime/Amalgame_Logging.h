/*
 * Amalgame Standard Library — Amalgame.Logging
 * Copyright (c) 2026 Bastien MOUGET
 * https://github.com/amalgame-lang/Amalgame
 *
 * Provides: leveled logging (Debug/Info/Warn/Error) to stderr +
 * optional rotating file sink. Single-process, thread-unsafe v1 —
 * fine for CLIs and single-threaded servers, needs a mutex before
 * the first real multi-threaded sink user lands.
 */

#ifndef AMALGAME_LOGGING_H
#define AMALGAME_LOGGING_H

#include "_runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* File-scope state. Inline static so the symbols don't conflict
 * when this header is pulled into multiple translation units. The
 * single Amalgame compilation unit doesn't actually need that —
 * everything lands in one .c file — but the inline-static
 * convention matches sibling stdlib headers (Amalgame_Random,
 * Amalgame_DateTime, etc.). */
static int   Amalgame_Logging_MinLevel = 1;     /* default Info */
static char* Amalgame_Logging_FilePath = NULL;  /* NULL = disabled */

/* Map a 4-char level name to its 0..3 integer code. Case-
 * insensitive on the first char so callers can write "DEBUG",
 * "debug", or "Debug" interchangeably. Unknown names default
 * to Info (1) — silently, since logging itself should never
 * crash a process. */
static inline int Amalgame_Logging_NameToLevel(code_string name) {
    if (!name || !name[0]) return 1;
    char c = name[0];
    if (c == 'd' || c == 'D') return 0;
    if (c == 'i' || c == 'I') return 1;
    if (c == 'w' || c == 'W') return 2;
    if (c == 'e' || c == 'E') return 3;
    return 1;
}

static inline void Logging_SetMinLevel(code_string name) {
    Amalgame_Logging_MinLevel = Amalgame_Logging_NameToLevel(name);
}

static inline code_string Logging_GetMinLevel(void) {
    int l = Amalgame_Logging_MinLevel;
    if (l == 0) return "debug";
    if (l == 1) return "info";
    if (l == 2) return "warn";
    return "error";
}

static inline void Logging_SetFile(code_string path) {
    if (Amalgame_Logging_FilePath) {
        free(Amalgame_Logging_FilePath);
        Amalgame_Logging_FilePath = NULL;
    }
    if (path && path[0]) {
        size_t n = strlen(path);
        Amalgame_Logging_FilePath = (char*) malloc(n + 1);
        if (Amalgame_Logging_FilePath) memcpy(Amalgame_Logging_FilePath, path, n + 1);
    }
}

static inline code_string Logging_GetFile(void) {
    return Amalgame_Logging_FilePath ? Amalgame_Logging_FilePath : "";
}

/* Internal: emit one formatted line to stderr + optional file
 * sink. Caller passes the integer level (0..3) and a label string
 * fitting the slot ("DEBUG", "INFO ", "WARN ", "ERROR" — the
 * 5-char fixed-width labels keep the timestamp columns aligned). */
static inline void Amalgame_Logging_Emit(int level, code_string label, code_string msg) {
    if (level < Amalgame_Logging_MinLevel) return;
    /* UTC ISO 8601 timestamp with second resolution. Logging is
     * not a perf-critical path; gettimeofday() millisecond
     * precision is a slice-2 ask if it ever matters. */
    time_t t = time(NULL);
    struct tm tm;
#ifdef _WIN32
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif
    char ts[32];
    strftime(ts, sizeof(ts), "%Y-%m-%dT%H:%M:%SZ", &tm);
    fprintf(stderr, "%s %s %s\n", ts, label ? label : "?",
            msg ? msg : "");
    if (Amalgame_Logging_FilePath) {
        FILE* f = fopen(Amalgame_Logging_FilePath, "a");
        if (f) {
            fprintf(f, "%s %s %s\n", ts, label ? label : "?",
                    msg ? msg : "");
            fclose(f);
        }
    }
}

static inline void Logging_Debug(code_string msg) {
    Amalgame_Logging_Emit(0, "DEBUG", msg);
}
static inline void Logging_Info(code_string msg) {
    Amalgame_Logging_Emit(1, "INFO ", msg);
}
static inline void Logging_Warn(code_string msg) {
    Amalgame_Logging_Emit(2, "WARN ", msg);
}
static inline void Logging_Error(code_string msg) {
    Amalgame_Logging_Emit(3, "ERROR", msg);
}

#endif /* AMALGAME_LOGGING_H */
