/*
 * Amalgame Runtime — Process module
 *
 * Provides:
 *   Process_Run(cmd)        -> i64                      // exit code, output flows to parent stdio
 *   Process_RunCapture(cmd) -> AmalgameProcessResult*   // exit + captured stdout (stderr merged in)
 *
 * Cross-platform: uses popen/pclose on POSIX, _popen/_pclose on Windows.
 * Both are widely available; we don't rely on fork/exec or CreateProcess
 * for v1 to keep the dependency surface tiny.
 *
 * Stderr is currently merged into Stdout via shell redirection (`2>&1`)
 * so a single FILE* read covers both streams. A future v2 may split
 * them with a proper pipe pair.
 */

#ifndef AMALGAME_PROCESS_H
#define AMALGAME_PROCESS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "_runtime.h"

#ifdef _WIN32
  #include <io.h>
  #define _AM_POPEN  _popen
  #define _AM_PCLOSE _pclose
#else
  #include <sys/wait.h>
  #define _AM_POPEN  popen
  #define _AM_PCLOSE pclose
#endif

/* Result of Process_RunCapture. Stderr is currently always "" — its
   bytes are merged into Stdout via `2>&1`. Kept as a separate field
   so callers don't break when v2 splits them.
*/
typedef struct AmalgameProcessResult {
    i64         Exit;
    code_string Stdout;
    code_string Stderr;
} AmalgameProcessResult;

/* Decode a `system()` / pclose() return value into a human exit code:
 * - On POSIX, encode normal exits as the WEXITSTATUS, and signal
 *   terminations as 128 + signum (matches shell convention).
 * - On Windows the value is already the raw exit code from the child.
 */
static inline i64 _am_decode_status(int rc) {
    if (rc == -1) return -1;
#ifdef _WIN32
    return (i64) rc;
#else
    if (WIFEXITED(rc))   return (i64) WEXITSTATUS(rc);
    if (WIFSIGNALED(rc)) return (i64)(128 + WTERMSIG(rc));
    return (i64) rc;
#endif
}

/* Run a shell command, streaming its stdio to the parent's. */
static inline i64 Process_Run(code_string cmd) {
    if (!cmd) return -1;
    int rc = system(cmd);
    return _am_decode_status(rc);
}

/* Run a shell command and capture its merged stdout+stderr. */
static inline AmalgameProcessResult* Process_RunCapture(code_string cmd) {
    AmalgameProcessResult* r = (AmalgameProcessResult*) code_alloc(sizeof(AmalgameProcessResult));
    r->Stderr = code_strdup("");
    if (!cmd) {
        r->Exit   = -1;
        r->Stdout = code_strdup("");
        return r;
    }
    /* Wrap with `2>&1` so a single read picks up both streams.
       Allocates a small buffer with room for "(...) 2>&1\0". */
    size_t lc = strlen(cmd);
    size_t lw = lc + 16;
    char* wrapped = (char*) GC_MALLOC(lw);
    snprintf(wrapped, lw, "(%s) 2>&1", cmd);

    FILE* fp = _AM_POPEN(wrapped, "r");
    if (!fp) {
        r->Exit   = -1;
        r->Stdout = code_strdup("");
        return r;
    }

    /* Growable buffer for the child's output. */
    size_t cap = 4096;
    size_t len = 0;
    char*  buf = (char*) GC_MALLOC(cap);
    char chunk[4096];
    size_t n;
    while ((n = fread(chunk, 1, sizeof(chunk), fp)) > 0) {
        if (len + n + 1 > cap) {
            size_t nc = cap * 2;
            if (nc < len + n + 1) nc = len + n + 1;
            char* nb = (char*) GC_MALLOC(nc);
            memcpy(nb, buf, len);
            buf = nb;
            cap = nc;
        }
        memcpy(buf + len, chunk, n);
        len += n;
    }
    buf[len] = '\0';

    int rc = _AM_PCLOSE(fp);
    r->Stdout = buf;
    r->Exit   = _am_decode_status(rc);
    return r;
}

#endif /* AMALGAME_PROCESS_H */
