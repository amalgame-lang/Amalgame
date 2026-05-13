/*
 * Amalgame Standard Library — Amalgame.Logging (state declarations)
 * Copyright (c) 2026 Bastien MOUGET
 * https://github.com/amalgame-lang/Amalgame
 *
 * Since the post-v0.7.5 migration, the Logging implementation lives
 * in `src/stdlib/logging.am` as a class with `@c { … }` bodies that
 * read / write the two file-scope state globals declared below.
 * The header keeps the state declarations (so the @c blocks can
 * share them across translation units) plus the libc includes the
 * @c blocks reach for (`<time.h>` for `gmtime_r` / `strftime`,
 * `<stdio.h>` / `<stdlib.h>` / `<string.h>` from `_runtime.h`).
 *
 * Process-wide singleton state; thread-unsafe v1 — fine for CLIs
 * and single-threaded servers, needs a mutex before the first real
 * multi-threaded sink user lands.
 */

#ifndef AMALGAME_LOGGING_H
#define AMALGAME_LOGGING_H

#include "_runtime.h"
#include <time.h>

/* File-scope state. Inline static so the symbols don't conflict
 * when this header is pulled into multiple translation units. The
 * single Amalgame compilation unit doesn't actually need that —
 * everything lands in one .c file — but the inline-static
 * convention matches sibling stdlib headers (Amalgame_Random,
 * Amalgame_DateTime, etc.). */
static int   Amalgame_Logging_MinLevel = 1;     /* default Info */
static char* Amalgame_Logging_FilePath = NULL;  /* NULL = disabled */

#endif /* AMALGAME_LOGGING_H */
