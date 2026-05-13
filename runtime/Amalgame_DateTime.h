/*
 * Amalgame Standard Library — Amalgame.DateTime (runtime shim)
 * Copyright (c) 2026 Bastien MOUGET
 * https://github.com/amalgame-lang/Amalgame
 *
 * Since the v0.7.x pure-AM migration, the entire DateTime
 * implementation lives in `src/stdlib/datetime.am` — Howard
 * Hinnant's `civil_from_days` / `days_from_civil` calendar
 * conversion, ISO 8601 format / parse, field breakdown, the
 * sentinel value, all in pure Amalgame.
 *
 * The only bits that stay in C are the two clock primitives
 * (`clock_gettime` / `GetSystemTimeAsFileTime` /
 * `QueryPerformanceCounter`) — they're invoked from two
 * `@c { … }` blocks in `datetime.am`. This header just supplies
 * the headers those blocks reach for.
 */

#ifndef AMALGAME_DATETIME_H
#define AMALGAME_DATETIME_H

#include "_runtime.h"
#include <stdint.h>
#include <time.h>

#ifdef _WIN32
  #include <windows.h>
#endif

#endif /* AMALGAME_DATETIME_H */
