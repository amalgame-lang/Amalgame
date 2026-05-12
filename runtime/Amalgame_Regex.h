/*
 * Amalgame Standard Library — Amalgame.Regex
 * Copyright (c) 2026 Bastien MOUGET
 * https://github.com/amalgame-lang/regex
 *
 * Provides: Regex.Test / Regex.Match / Regex.MatchAll / Regex.Replace
 *
 * Thin binding over POSIX `regex.h` (regcomp + regexec) — available
 * on every POSIX platform and MinGW; no third-party PCRE dependency.
 *
 * Syntax: POSIX extended (ERE). The bits that matter day-to-day:
 *   .      any single char
 *   *      0 or more
 *   +      1 or more
 *   ?      0 or 1
 *   ^ $    line anchors
 *   [...]  character class (e.g. [a-z], [^abc])
 *   ( )    capture group
 *   |      alternation
 *   {n,m}  repetition
 *
 * Out of scope (PCRE territory): \d / \w / \s shorthand, look-arounds,
 * non-greedy modifiers, named captures, Unicode property classes.
 * For those, the user can shell out via Process.Run("grep -P …") or
 * bind libpcre2 in a future package.
 */

#ifndef AMALGAME_REGEX_H
#define AMALGAME_REGEX_H

#include "_runtime.h"
#include <regex.h>
#include <string.h>
#include <stdlib.h>

/* Container for the result of a single match: the matched text plus
 * its `Start` / `End` byte offsets in the original subject. Captures
 * (if any) live in parallel arrays; index 0 is the full match,
 * 1..N are the parenthesised groups in order. */
typedef struct AmalgameRegexMatch {
    code_string  Text;
    i64          Start;
    i64          End;
    /* Captures */
    code_string* GroupTexts;   /* NULL when GroupCount == 0 */
    i64*         GroupStarts;
    i64*         GroupEnds;
    i64          GroupCount;
} AmalgameRegexMatch;

/* POSIX regex.h supports up to RE_NREGS captures; cap conservatively
 * at 16 to keep the per-match allocation predictable. Plenty for the
 * config-extraction and template-tokenisation 80% use cases. */
#define AMALGAME_REGEX_MAX_GROUPS 16

static inline code_string Regex__Slice(code_string s, i64 start, i64 end) {
    if (start < 0 || end < start) { return code_strdup(""); }
    i64 len = end - start;
    char* out = (char*) GC_MALLOC((size_t)(len + 1));
    memcpy(out, s + start, (size_t) len);
    out[len] = '\0';
    return out;
}

/* Try to match `pattern` against `subject`. Returns NULL when the
 * pattern doesn't compile (bad syntax) or doesn't match. On success
 * the caller owns the GC-rooted AmalgameRegexMatch*. */
static inline AmalgameRegexMatch* Regex_Match(code_string pattern, code_string subject) {
    if (!pattern || !subject) { return NULL; }
    regex_t re;
    if (regcomp(&re, pattern, REG_EXTENDED) != 0) { return NULL; }
    regmatch_t marr[AMALGAME_REGEX_MAX_GROUPS];
    if (regexec(&re, subject, AMALGAME_REGEX_MAX_GROUPS, marr, 0) != 0) {
        regfree(&re);
        return NULL;
    }
    AmalgameRegexMatch* m = (AmalgameRegexMatch*) GC_MALLOC(sizeof(AmalgameRegexMatch));
    m->Start = (i64) marr[0].rm_so;
    m->End   = (i64) marr[0].rm_eo;
    m->Text  = Regex__Slice(subject, m->Start, m->End);
    /* Count valid capture groups (skip the [0] full-match and any
     * trailing unused slots — POSIX uses -1 for "no match"). */
    i64 count = 0;
    for (i64 i = 1; i < AMALGAME_REGEX_MAX_GROUPS; i++) {
        if (marr[i].rm_so >= 0) { count++; } else { break; }
    }
    m->GroupCount = count;
    if (count > 0) {
        m->GroupTexts  = (code_string*) GC_MALLOC((size_t)count * sizeof(code_string));
        m->GroupStarts = (i64*)         GC_MALLOC((size_t)count * sizeof(i64));
        m->GroupEnds   = (i64*)         GC_MALLOC((size_t)count * sizeof(i64));
        for (i64 i = 0; i < count; i++) {
            m->GroupStarts[i] = (i64) marr[i + 1].rm_so;
            m->GroupEnds[i]   = (i64) marr[i + 1].rm_eo;
            m->GroupTexts[i]  = Regex__Slice(subject, m->GroupStarts[i], m->GroupEnds[i]);
        }
    } else {
        m->GroupTexts  = NULL;
        m->GroupStarts = NULL;
        m->GroupEnds   = NULL;
    }
    regfree(&re);
    return m;
}

/* True iff the pattern matches anywhere in subject. Cheaper than
 * `Match` (doesn't allocate the result struct). */
static inline code_bool Regex_Test(code_string pattern, code_string subject) {
    if (!pattern || !subject) { return 0; }
    regex_t re;
    if (regcomp(&re, pattern, REG_EXTENDED | REG_NOSUB) != 0) { return 0; }
    int rc = regexec(&re, subject, 0, NULL, 0);
    regfree(&re);
    return rc == 0 ? 1 : 0;
}

/* Replace the FIRST occurrence of `pattern` in `subject` with
 * `replacement`. Captures aren't expanded — `\1` stays literal. For
 * "all occurrences" use Regex_ReplaceAll. */
static inline code_string Regex_Replace(code_string pattern,
                                         code_string subject,
                                         code_string replacement) {
    if (!pattern || !subject || !replacement) { return code_strdup(subject ? subject : ""); }
    regex_t re;
    if (regcomp(&re, pattern, REG_EXTENDED) != 0) {
        return code_strdup(subject);
    }
    regmatch_t mm;
    if (regexec(&re, subject, 1, &mm, 0) != 0) {
        regfree(&re);
        return code_strdup(subject);
    }
    size_t ls = strlen(subject), lr = strlen(replacement);
    size_t span = (size_t)(mm.rm_eo - mm.rm_so);
    size_t out_len = ls - span + lr;
    char* out = (char*) GC_MALLOC(out_len + 1);
    memcpy(out, subject, (size_t) mm.rm_so);
    memcpy(out + mm.rm_so, replacement, lr);
    memcpy(out + mm.rm_so + lr, subject + mm.rm_eo, ls - (size_t) mm.rm_eo);
    out[out_len] = '\0';
    regfree(&re);
    return out;
}

/* Replace every non-overlapping occurrence. Each match's start
 * advances past the replaced span so we don't loop on a zero-length
 * match (e.g. pattern `.*` against any input). */
static inline code_string Regex_ReplaceAll(code_string pattern,
                                            code_string subject,
                                            code_string replacement) {
    if (!pattern || !subject || !replacement) { return code_strdup(subject ? subject : ""); }
    regex_t re;
    if (regcomp(&re, pattern, REG_EXTENDED) != 0) {
        return code_strdup(subject);
    }
    size_t lr = strlen(replacement);
    size_t cap = 64;
    char* out = (char*) GC_MALLOC(cap);
    size_t out_len = 0;
    const char* cursor = subject;
    while (*cursor) {
        regmatch_t mm;
        if (regexec(&re, cursor, 1, &mm, 0) != 0) { break; }
        size_t prefix = (size_t) mm.rm_so;
        size_t need = out_len + prefix + lr + 1;
        if (need > cap) {
            while (cap < need) { cap *= 2; }
            char* bigger = (char*) GC_MALLOC(cap);
            memcpy(bigger, out, out_len);
            out = bigger;
        }
        memcpy(out + out_len, cursor, prefix);
        out_len += prefix;
        memcpy(out + out_len, replacement, lr);
        out_len += lr;
        /* Step past the match. Guard against zero-length matches
         * (e.g. `^` on each iteration) by advancing at least one
         * byte so we don't spin forever. */
        size_t step = (size_t)(mm.rm_eo - mm.rm_so);
        if (step == 0) {
            if (cursor[mm.rm_eo] == '\0') { break; }
            /* Copy the byte we'd otherwise skip past. */
            need = out_len + 2;
            if (need > cap) {
                while (cap < need) { cap *= 2; }
                char* bigger = (char*) GC_MALLOC(cap);
                memcpy(bigger, out, out_len);
                out = bigger;
            }
            out[out_len++] = cursor[mm.rm_eo];
            cursor += mm.rm_eo + 1;
        } else {
            cursor += mm.rm_eo;
        }
    }
    size_t tail = strlen(cursor);
    size_t need = out_len + tail + 1;
    if (need > cap) {
        cap = need;
        char* bigger = (char*) GC_MALLOC(cap);
        memcpy(bigger, out, out_len);
        out = bigger;
    }
    memcpy(out + out_len, cursor, tail);
    out_len += tail;
    out[out_len] = '\0';
    regfree(&re);
    return out;
}

/* Accessors for AmalgameRegexMatch — emitted as fields and pulled
 * via Match_GetText etc. for the Amalgame side, paralleling how
 * Http_GetBody / Http_GetStatus wrap AmalgameHttpResponse. */
static inline code_string Match_GetText(AmalgameRegexMatch* m)  { return m->Text; }
static inline i64         Match_GetStart(AmalgameRegexMatch* m) { return m->Start; }
static inline i64         Match_GetEnd(AmalgameRegexMatch* m)   { return m->End; }
static inline i64         Match_GroupCount(AmalgameRegexMatch* m) { return m->GroupCount; }

/* Group accessor: 0-indexed over the capture groups (group 0 in
 * regex parlance is the overall match — for that use GetText).
 * Returns "" for out-of-range indices so callers can probe by
 * count without bounds-checking each lookup. */
static inline code_string Match_GroupText(AmalgameRegexMatch* m, i64 idx) {
    if (idx < 0 || idx >= m->GroupCount) { return code_strdup(""); }
    return m->GroupTexts[idx];
}
static inline i64 Match_GroupStart(AmalgameRegexMatch* m, i64 idx) {
    if (idx < 0 || idx >= m->GroupCount) { return -1; }
    return m->GroupStarts[idx];
}
static inline i64 Match_GroupEnd(AmalgameRegexMatch* m, i64 idx) {
    if (idx < 0 || idx >= m->GroupCount) { return -1; }
    return m->GroupEnds[idx];
}

#endif /* AMALGAME_REGEX_H */
