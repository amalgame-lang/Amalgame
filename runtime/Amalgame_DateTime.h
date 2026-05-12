/*
 * Amalgame Standard Library — Amalgame.DateTime
 * Copyright (c) 2026 Bastien MOUGET
 * https://github.com/amalgame-lang/Amalgame
 *
 * Provides: wall + monotonic clocks, ISO 8601 format/parse.
 *
 * The pure-Amalgame side (src/stdlib/datetime.am) holds an
 * `Instant` as i64 nanoseconds since 1970-01-01T00:00:00Z and a
 * `Duration` as i64 nanoseconds. 64 bits cover ~292 years either
 * side of the epoch — wide enough for everything from the early
 * 18th century through to the year ~2262.
 *
 * v1 is UTC-only. Accepting `+HH:MM` offsets, normalizing to
 * UTC, and exposing local time / timezones are tracked in the
 * roadmap. Same goes for leap seconds (we emit and accept 60 in
 * the seconds field so that real-world ISO timestamps round-trip,
 * but treat the second internally as if it were 59 — strict
 * leap-second support requires a UTC↔TAI table we don't ship).
 */

#ifndef AMALGAME_DATETIME_H
#define AMALGAME_DATETIME_H

#include "_runtime.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
  #include <windows.h>
#endif

/* Sentinel returned by DateTime_ParseIsoNanos on malformed input.
 * INT64_MIN sits well outside the ±292-year window an Instant
 * can represent, so callers can check `nanos == INT64_MIN`
 * (or use DateTime_IsParseError below) without worrying about
 * collisions with real-but-extreme dates. */
#define AMALGAME_DATETIME_PARSE_ERROR INT64_MIN

/* ─────────────────────────────────────────────
   Wall clock — current UTC time in ns since epoch
   ─────────────────────────────────────────────
*/
static inline i64 DateTime_NowNanos(void) {
#ifdef _WIN32
    /* GetSystemTimeAsFileTime: 100-ns ticks since 1601-01-01 UTC.
     * Convert to ns since 1970-01-01 by subtracting the difference
     * (11644473600 seconds = 116444736000000000 100-ns ticks). */
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER u;
    u.LowPart  = ft.dwLowDateTime;
    u.HighPart = ft.dwHighDateTime;
    int64_t ticks = (int64_t) u.QuadPart - 116444736000000000LL;
    return (i64)(ticks * 100LL);
#else
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) != 0) return 0;
    return (i64)((int64_t) ts.tv_sec * 1000000000LL + (int64_t) ts.tv_nsec);
#endif
}

/* ─────────────────────────────────────────────
   Monotonic clock — for measuring elapsed time
   ─────────────────────────────────────────────
   Not anchored to wall time — value only meaningful relative to
   another Monotonic reading from the same process. Immune to
   wall-clock adjustments (NTP, manual changes), unlike NowNanos.
*/
static inline i64 DateTime_NowMonotonicNanos(void) {
#ifdef _WIN32
    LARGE_INTEGER counter, freq;
    QueryPerformanceCounter(&counter);
    QueryPerformanceFrequency(&freq);
    if (freq.QuadPart == 0) return (i64) GetTickCount64() * 1000000LL;
    return (i64)((counter.QuadPart * 1000000000LL) / freq.QuadPart);
#else
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) return 0;
    return (i64)((int64_t) ts.tv_sec * 1000000000LL + (int64_t) ts.tv_nsec);
#endif
}

/* ─────────────────────────────────────────────
   ISO 8601 / RFC 3339 formatting
   ─────────────────────────────────────────────
   Output: YYYY-MM-DDTHH:MM:SS.NNNNNNNNNZ
   Fixed 9-digit fractional seconds (always padded with zeros) so
   timestamps sort lexically. Append `Z` to mark UTC explicitly.
*/
static inline code_string DateTime_FormatIso(i64 nanos) {
    int64_t secs = (int64_t)(nanos / 1000000000LL);
    int64_t frac = (int64_t)(nanos % 1000000000LL);
    if (frac < 0) {
        secs -= 1;
        frac += 1000000000LL;
    }
    time_t t = (time_t) secs;
    struct tm tm_;
#ifdef _WIN32
    gmtime_s(&tm_, &t);
#else
    if (gmtime_r(&t, &tm_) == NULL) {
        /* gmtime_r returns NULL only on out-of-range — fall back
         * to a placeholder rather than dereferencing garbage. */
        return code_strdup("1970-01-01T00:00:00.000000000Z");
    }
#endif
    char* buf = (char*) GC_MALLOC(48);
    snprintf(buf, 48,
        "%04d-%02d-%02dT%02d:%02d:%02d.%09lldZ",
        tm_.tm_year + 1900, tm_.tm_mon + 1, tm_.tm_mday,
        tm_.tm_hour, tm_.tm_min, tm_.tm_sec, (long long) frac);
    return buf;
}

/* ─────────────────────────────────────────────
   UTC field breakdown
   ─────────────────────────────────────────────
   Each helper decomposes a nanos-since-epoch instant via
   gmtime_r / gmtime_s and returns one calendar field. Cheap —
   gmtime is a few dozen instructions — but if a caller needs
   the full breakdown it pays 6× the cost over a single
   `struct tm` extraction. Easy to consolidate later if
   benchmarks complain.
*/

static inline int _amc_dt_breakdown(i64 nanos, struct tm* out) {
    int64_t secs = (int64_t)(nanos / 1000000000LL);
    time_t t = (time_t) secs;
#ifdef _WIN32
    return gmtime_s(out, &t) == 0 ? 1 : 0;
#else
    return gmtime_r(&t, out) != NULL ? 1 : 0;
#endif
}

static inline i64 DateTime_Year(i64 nanos) {
    struct tm tm_;
    if (!_amc_dt_breakdown(nanos, &tm_)) { return 1970; }
    return (i64)(tm_.tm_year + 1900);
}
static inline i64 DateTime_Month(i64 nanos) {
    struct tm tm_;
    if (!_amc_dt_breakdown(nanos, &tm_)) { return 1; }
    return (i64)(tm_.tm_mon + 1);
}
static inline i64 DateTime_Day(i64 nanos) {
    struct tm tm_;
    if (!_amc_dt_breakdown(nanos, &tm_)) { return 1; }
    return (i64) tm_.tm_mday;
}
static inline i64 DateTime_Hour(i64 nanos) {
    struct tm tm_;
    if (!_amc_dt_breakdown(nanos, &tm_)) { return 0; }
    return (i64) tm_.tm_hour;
}
static inline i64 DateTime_Minute(i64 nanos) {
    struct tm tm_;
    if (!_amc_dt_breakdown(nanos, &tm_)) { return 0; }
    return (i64) tm_.tm_min;
}
static inline i64 DateTime_Second(i64 nanos) {
    struct tm tm_;
    if (!_amc_dt_breakdown(nanos, &tm_)) { return 0; }
    return (i64) tm_.tm_sec;
}

/* ─────────────────────────────────────────────
   ISO 8601 / RFC 3339 parsing (UTC subset)
   ─────────────────────────────────────────────
   Accepts: YYYY-MM-DDTHH:MM:SS[.frac][Z]
   - Lower- and uppercase `T` / `Z` both fine.
   - 0–9 fractional digits; we truncate beyond 9 (nanosecond
     precision is the cap).
   - Trailing `Z` is required for v1; explicit offsets like
     `+02:00` return AMALGAME_DATETIME_PARSE_ERROR.
   - Returns AMALGAME_DATETIME_PARSE_ERROR on any malformation.
*/
static inline int _amc_dt_isdigit(char c) {
    return (c >= '0' && c <= '9') ? 1 : 0;
}

static inline i64 DateTime_ParseIsoNanos(code_string s) {
    if (!s) return AMALGAME_DATETIME_PARSE_ERROR;
    int year = 0, month = 0, day = 0, hour = 0, min = 0, sec = 0;
    /* Be strict about the fixed-width prefix: `YYYY-MM-DDTHH:MM:SS`
     * is 19 chars. Anything shorter is malformed. */
    size_t len = strlen(s);
    if (len < 19) return AMALGAME_DATETIME_PARSE_ERROR;

    /* Walk the prefix character-by-character so we reject things
     * sscanf might accept (e.g. `2026-1-1T...`). */
    for (size_t i = 0; i < 19; i++) {
        char c = s[i];
        if (i == 4 || i == 7) {
            if (c != '-') return AMALGAME_DATETIME_PARSE_ERROR;
        } else if (i == 10) {
            if (c != 'T' && c != 't') return AMALGAME_DATETIME_PARSE_ERROR;
        } else if (i == 13 || i == 16) {
            if (c != ':') return AMALGAME_DATETIME_PARSE_ERROR;
        } else {
            if (!_amc_dt_isdigit(c)) return AMALGAME_DATETIME_PARSE_ERROR;
        }
    }

    int n = sscanf(s, "%4d-%2d-%2dT%2d:%2d:%2d",
                   &year, &month, &day, &hour, &min, &sec);
    if (n != 6) {
        /* Try lowercase t (sscanf format strings are case-sensitive
         * for literal chars in some libcs). */
        n = sscanf(s, "%4d-%2d-%2dt%2d:%2d:%2d",
                   &year, &month, &day, &hour, &min, &sec);
        if (n != 6) return AMALGAME_DATETIME_PARSE_ERROR;
    }

    /* Field bounds (we'll let mktime validate Feb 29 etc.). */
    if (month  < 1 || month  > 12) return AMALGAME_DATETIME_PARSE_ERROR;
    if (day    < 1 || day    > 31) return AMALGAME_DATETIME_PARSE_ERROR;
    if (hour   < 0 || hour   > 23) return AMALGAME_DATETIME_PARSE_ERROR;
    if (min    < 0 || min    > 59) return AMALGAME_DATETIME_PARSE_ERROR;
    /* sec=60 is permitted by ISO 8601 for leap seconds; we clamp
     * it back to 59 below when packing into time_t. */
    if (sec    < 0 || sec    > 60) return AMALGAME_DATETIME_PARSE_ERROR;

    const char* p = s + 19;
    int64_t fracNs = 0;
    if (*p == '.') {
        p++;
        const char* fStart = p;
        while (*p && _amc_dt_isdigit(*p)) p++;
        int flen = (int)(p - fStart);
        if (flen == 0) return AMALGAME_DATETIME_PARSE_ERROR;
        /* Truncate to 9 digits and pad with zeros. */
        char fbuf[10];
        int copy = flen < 9 ? flen : 9;
        memcpy(fbuf, fStart, (size_t) copy);
        for (int i = copy; i < 9; i++) fbuf[i] = '0';
        fbuf[9] = '\0';
        fracNs = atoll(fbuf);
    }

    if (*p != 'Z' && *p != 'z') return AMALGAME_DATETIME_PARSE_ERROR;
    p++;
    if (*p != '\0') return AMALGAME_DATETIME_PARSE_ERROR;

    /* Pack into time_t via timegm (POSIX) / _mkgmtime (Windows).
     * Both interpret struct tm fields as UTC, no DST, no locale. */
    struct tm tm_;
    memset(&tm_, 0, sizeof(tm_));
    tm_.tm_year = year - 1900;
    tm_.tm_mon  = month - 1;
    tm_.tm_mday = day;
    tm_.tm_hour = hour;
    tm_.tm_min  = min;
    tm_.tm_sec  = (sec == 60) ? 59 : sec;

    time_t t;
#ifdef _WIN32
    t = _mkgmtime(&tm_);
#else
    t = timegm(&tm_);
#endif
    if (t == (time_t) -1) return AMALGAME_DATETIME_PARSE_ERROR;

    return (i64)((int64_t) t * 1000000000LL + fracNs);
}

/* Convenience predicate so Amalgame callers don't need to bake
 * INT64_MIN into their source. */
static inline code_bool DateTime_IsParseError(i64 nanos) {
    return nanos == AMALGAME_DATETIME_PARSE_ERROR;
}

#endif /* AMALGAME_DATETIME_H */
