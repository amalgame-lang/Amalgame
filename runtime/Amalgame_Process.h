/*
 * Amalgame Runtime — Process module
 *
 * v1 surface (kept stable):
 *   Process_Run(cmd)        -> i64                      // exit code, stdio flows to parent
 *   Process_RunCapture(cmd) -> AmalgameProcessResult*   // exit + merged stdout/stderr via 2>&1
 *
 * v2 surface (added 2026-05-16):
 *   Process_RunTimeout(cmd, timeout_ms) -> i64
 *       Like Run, but SIGKILL'd (POSIX) or TerminateProcess'd (Win)
 *       after timeout_ms. Returns 124 on timeout (matches GNU
 *       `timeout(1)`). timeout_ms <= 0 = no timeout (= Run).
 *
 *   Process_RunCaptureBoth(cmd) -> AmalgameProcessResult*
 *       Real pipe pair — Stdout and Stderr split. Uses fork+execvp
 *       on POSIX, CreateProcess on Windows. No shell timeout.
 *
 *   Process_RunCaptureBothTimeout(cmd, timeout_ms) -> AmalgameProcessResult*
 *       Same split, with timeout. Exit = 124 on timeout.
 *
 * v1 helpers use popen / _popen for portability; v2 uses fork+exec /
 * CreateProcess so it can split streams and kill on timeout.
 */

#ifndef AMALGAME_PROCESS_H
#define AMALGAME_PROCESS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "_runtime.h"

#ifdef _WIN32
  #include <io.h>
  #include <windows.h>
  #define _AM_POPEN  _popen
  #define _AM_PCLOSE _pclose
#else
  #include <sys/wait.h>
  #include <sys/types.h>
  #include <sys/time.h>
  #include <time.h>          /* nanosleep */
  #include <unistd.h>
  #include <fcntl.h>
  #include <signal.h>
  #include <poll.h>
  #include <errno.h>
  #define _AM_POPEN  popen
  #define _AM_PCLOSE pclose
#endif

/* Exit code returned when a timeout kills the child. Matches GNU
 * `timeout(1)` convention so shell pipelines can detect it. */
#ifndef AMALGAME_PROCESS_TIMEOUT_EXIT
#  define AMALGAME_PROCESS_TIMEOUT_EXIT 124
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

/* ============================================================
 *  v2: timeout + real pipe split (fork+exec on POSIX,
 *      CreateProcess on Windows)
 * ============================================================ */

/* Append `n` bytes to a GC growable buffer. Returns the new buffer
 * (possibly reallocated); updates *cap and *len in place. */
static inline char* _am_buf_append(char* buf, size_t* cap, size_t* len,
                                    const char* src, size_t n) {
    if (*len + n + 1 > *cap) {
        size_t nc = (*cap) * 2;
        if (nc < *len + n + 1) nc = *len + n + 1;
        char* nb = (char*) GC_MALLOC(nc);
        if (*len > 0) memcpy(nb, buf, *len);
        buf = nb;
        *cap = nc;
    }
    if (n > 0) memcpy(buf + *len, src, n);
    *len += n;
    buf[*len] = '\0';
    return buf;
}

#ifndef _WIN32

/* POSIX: fork + execvp + 2 pipes for stdout/stderr.
 * timeout_ms <= 0  ⇒ blocks indefinitely.
 * timeout_ms > 0   ⇒ SIGKILL the child past the deadline; result.Exit = 124.
 * capture_streams = 0 ⇒ child inherits parent's stdio (Process_Run semantics).
 *                   1 ⇒ pipe both streams and capture into r->Stdout / Stderr.
 *
 * Returns a fully-populated result; on internal error r->Exit = -1.
 */
static inline AmalgameProcessResult* _am_proc_runex(code_string cmd,
                                                      i64 timeout_ms,
                                                      int capture_streams) {
    AmalgameProcessResult* r = (AmalgameProcessResult*) code_alloc(sizeof(AmalgameProcessResult));
    r->Exit   = -1;
    r->Stdout = code_strdup("");
    r->Stderr = code_strdup("");
    if (!cmd) return r;

    int out_pipe[2] = {-1, -1};
    int err_pipe[2] = {-1, -1};
    if (capture_streams) {
        if (pipe(out_pipe) < 0) return r;
        if (pipe(err_pipe) < 0) {
            close(out_pipe[0]); close(out_pipe[1]);
            return r;
        }
    }

    pid_t pid = fork();
    if (pid < 0) {
        if (capture_streams) {
            close(out_pipe[0]); close(out_pipe[1]);
            close(err_pipe[0]); close(err_pipe[1]);
        }
        return r;
    }

    if (pid == 0) {
        /* ── child ── */
        if (capture_streams) {
            close(out_pipe[0]); close(err_pipe[0]);
            dup2(out_pipe[1], STDOUT_FILENO);
            dup2(err_pipe[1], STDERR_FILENO);
            close(out_pipe[1]); close(err_pipe[1]);
        }
        execl("/bin/sh", "sh", "-c", (const char*) cmd, (char*) NULL);
        _exit(127);  /* exec failed */
    }

    /* ── parent ── */
    size_t out_cap = 4096, out_len = 0;
    size_t err_cap = 4096, err_len = 0;
    char*  out_buf = capture_streams ? (char*) GC_MALLOC(out_cap) : NULL;
    char*  err_buf = capture_streams ? (char*) GC_MALLOC(err_cap) : NULL;
    if (out_buf) out_buf[0] = '\0';
    if (err_buf) err_buf[0] = '\0';

    if (capture_streams) {
        close(out_pipe[1]);
        close(err_pipe[1]);
        /* Non-blocking read so poll() drives the loop cleanly. */
        fcntl(out_pipe[0], F_SETFL, fcntl(out_pipe[0], F_GETFL, 0) | O_NONBLOCK);
        fcntl(err_pipe[0], F_SETFL, fcntl(err_pipe[0], F_GETFL, 0) | O_NONBLOCK);
    }

    /* Compute absolute deadline if a timeout is set. */
    struct timeval start, now;
    if (timeout_ms > 0) gettimeofday(&start, NULL);

    int   timed_out  = 0;
    int   out_open   = capture_streams;
    int   err_open   = capture_streams;
    char  chunk[4096];

    /* Drain pipes until both EOF + child exits, or timeout. */
    while (1) {
        /* Compute remaining budget for this poll/waitpid round. */
        int poll_ms = -1;  /* infinite */
        if (timeout_ms > 0) {
            gettimeofday(&now, NULL);
            i64 elapsed = (i64)(now.tv_sec - start.tv_sec) * 1000
                        + (i64)(now.tv_usec - start.tv_usec) / 1000;
            i64 left = timeout_ms - elapsed;
            if (left <= 0) {
                timed_out = 1;
                break;
            }
            poll_ms = (int) (left > 1000 ? 1000 : left);
        }

        if (capture_streams && (out_open || err_open)) {
            struct pollfd pfd[2];
            int nfds = 0;
            if (out_open) { pfd[nfds].fd = out_pipe[0]; pfd[nfds].events = POLLIN; nfds++; }
            if (err_open) { pfd[nfds].fd = err_pipe[0]; pfd[nfds].events = POLLIN; nfds++; }
            int pr = poll(pfd, nfds, poll_ms == -1 ? 100 : poll_ms);
            if (pr > 0) {
                int idx = 0;
                if (out_open) {
                    if (pfd[idx].revents & (POLLIN | POLLHUP)) {
                        ssize_t k = read(out_pipe[0], chunk, sizeof(chunk));
                        if (k > 0) out_buf = _am_buf_append(out_buf, &out_cap, &out_len, chunk, (size_t) k);
                        else if (k == 0) out_open = 0;
                        else if (errno != EAGAIN && errno != EINTR) out_open = 0;
                    }
                    idx++;
                }
                if (err_open) {
                    if (pfd[idx].revents & (POLLIN | POLLHUP)) {
                        ssize_t k = read(err_pipe[0], chunk, sizeof(chunk));
                        if (k > 0) err_buf = _am_buf_append(err_buf, &err_cap, &err_len, chunk, (size_t) k);
                        else if (k == 0) err_open = 0;
                        else if (errno != EAGAIN && errno != EINTR) err_open = 0;
                    }
                }
            }
        } else if (timeout_ms > 0) {
            /* No capture: just sleep up to the remaining budget. */
            struct timespec ts = { 0, (poll_ms > 0 ? poll_ms : 100) * 1000000L };
            nanosleep(&ts, NULL);
        }

        /* Non-blocking child status probe. */
        int   status = 0;
        pid_t w = waitpid(pid, &status, WNOHANG);
        if (w == pid) {
            /* Drain any remaining buffered bytes before returning. */
            if (capture_streams) {
                while (out_open) {
                    ssize_t k = read(out_pipe[0], chunk, sizeof(chunk));
                    if (k > 0) out_buf = _am_buf_append(out_buf, &out_cap, &out_len, chunk, (size_t) k);
                    else break;
                }
                while (err_open) {
                    ssize_t k = read(err_pipe[0], chunk, sizeof(chunk));
                    if (k > 0) err_buf = _am_buf_append(err_buf, &err_cap, &err_len, chunk, (size_t) k);
                    else break;
                }
            }
            r->Exit = _am_decode_status(status);
            if (capture_streams) {
                r->Stdout = out_buf ? out_buf : code_strdup("");
                r->Stderr = err_buf ? err_buf : code_strdup("");
                close(out_pipe[0]); close(err_pipe[0]);
            }
            return r;
        }

        /* If we're not capturing and not timing out, the only way out
         * is for waitpid above to reap the child. Keep looping. */
        if (!capture_streams && timeout_ms <= 0) {
            /* Tiny sleep to avoid a busy spin. */
            struct timespec ts = { 0, 10 * 1000000L };
            nanosleep(&ts, NULL);
        }
    }

    /* Timeout path — SIGKILL the child, reap it, mark exit=124. */
    if (timed_out) {
        kill(pid, SIGKILL);
        int status = 0;
        waitpid(pid, &status, 0);
        if (capture_streams) {
            /* Drain whatever's already in the pipes. */
            while (1) {
                ssize_t k = read(out_pipe[0], chunk, sizeof(chunk));
                if (k <= 0) break;
                out_buf = _am_buf_append(out_buf, &out_cap, &out_len, chunk, (size_t) k);
            }
            while (1) {
                ssize_t k = read(err_pipe[0], chunk, sizeof(chunk));
                if (k <= 0) break;
                err_buf = _am_buf_append(err_buf, &err_cap, &err_len, chunk, (size_t) k);
            }
            close(out_pipe[0]); close(err_pipe[0]);
            r->Stdout = out_buf ? out_buf : code_strdup("");
            r->Stderr = err_buf ? err_buf : code_strdup("");
        }
        r->Exit = AMALGAME_PROCESS_TIMEOUT_EXIT;
        return r;
    }

    return r;
}

#else  /* _WIN32 */

/* Windows: CreateProcess + optional pipes for stdout/stderr.
 * Same semantics as the POSIX path. */
static inline AmalgameProcessResult* _am_proc_runex(code_string cmd,
                                                      i64 timeout_ms,
                                                      int capture_streams) {
    AmalgameProcessResult* r = (AmalgameProcessResult*) code_alloc(sizeof(AmalgameProcessResult));
    r->Exit   = -1;
    r->Stdout = code_strdup("");
    r->Stderr = code_strdup("");
    if (!cmd) return r;

    SECURITY_ATTRIBUTES sa;
    sa.nLength              = sizeof(sa);
    sa.bInheritHandle       = TRUE;
    sa.lpSecurityDescriptor = NULL;

    HANDLE out_rd = NULL, out_wr = NULL;
    HANDLE err_rd = NULL, err_wr = NULL;
    if (capture_streams) {
        if (!CreatePipe(&out_rd, &out_wr, &sa, 0)) return r;
        SetHandleInformation(out_rd, HANDLE_FLAG_INHERIT, 0);
        if (!CreatePipe(&err_rd, &err_wr, &sa, 0)) {
            CloseHandle(out_rd); CloseHandle(out_wr);
            return r;
        }
        SetHandleInformation(err_rd, HANDLE_FLAG_INHERIT, 0);
    }

    /* Wrap with cmd.exe /c so the user can pass shell-style commands. */
    size_t lc = strlen(cmd);
    char*  wrapped = (char*) GC_MALLOC(lc + 16);
    snprintf(wrapped, lc + 16, "cmd.exe /c %s", cmd);

    STARTUPINFOA si = {0};
    si.cb = sizeof(si);
    if (capture_streams) {
        si.dwFlags    = STARTF_USESTDHANDLES;
        si.hStdOutput = out_wr;
        si.hStdError  = err_wr;
        si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);
    }
    PROCESS_INFORMATION pi = {0};
    BOOL ok = CreateProcessA(NULL, wrapped, NULL, NULL,
                              capture_streams ? TRUE : FALSE,
                              0, NULL, NULL, &si, &pi);
    if (!ok) {
        if (capture_streams) {
            CloseHandle(out_rd); CloseHandle(out_wr);
            CloseHandle(err_rd); CloseHandle(err_wr);
        }
        return r;
    }
    if (capture_streams) {
        CloseHandle(out_wr);
        CloseHandle(err_wr);
    }

    size_t out_cap = 4096, out_len = 0;
    size_t err_cap = 4096, err_len = 0;
    char*  out_buf = capture_streams ? (char*) GC_MALLOC(out_cap) : NULL;
    char*  err_buf = capture_streams ? (char*) GC_MALLOC(err_cap) : NULL;
    if (out_buf) out_buf[0] = '\0';
    if (err_buf) err_buf[0] = '\0';
    char chunk[4096];

    /* Read available bytes from a pipe handle into the buffer.
     * PeekNamedPipe avoids blocking when the child is still alive
     * but quiet. Returns 0 on EOF, 1 on bytes read or quiet, -1 on
     * error. */
    #define _AM_DRAIN(handle, buf, cap, len) do {                  \
        DWORD avail = 0;                                            \
        if (!PeekNamedPipe((handle), NULL, 0, NULL, &avail, NULL))  \
            break;                                                  \
        if (avail == 0) break;                                      \
        DWORD got = 0;                                              \
        if (avail > sizeof(chunk)) avail = sizeof(chunk);           \
        if (!ReadFile((handle), chunk, avail, &got, NULL) || got == 0) \
            break;                                                  \
        buf = _am_buf_append(buf, &cap, &len, chunk, (size_t) got); \
    } while (0)

    DWORD wait_total = (timeout_ms > 0) ? (DWORD) timeout_ms : INFINITE;
    DWORD wait_start = GetTickCount();
    DWORD wait_left  = wait_total;
    int   timed_out  = 0;
    while (1) {
        DWORD step = (wait_left == INFINITE || wait_left > 50) ? 50 : wait_left;
        DWORD wr = WaitForSingleObject(pi.hProcess, step);
        if (capture_streams) {
            _AM_DRAIN(out_rd, out_buf, out_cap, out_len);
            _AM_DRAIN(err_rd, err_buf, err_cap, err_len);
        }
        if (wr == WAIT_OBJECT_0) break;
        if (wait_total != INFINITE) {
            DWORD elapsed = GetTickCount() - wait_start;
            if (elapsed >= wait_total) { timed_out = 1; break; }
            wait_left = wait_total - elapsed;
        }
    }

    if (timed_out) {
        TerminateProcess(pi.hProcess, AMALGAME_PROCESS_TIMEOUT_EXIT);
        WaitForSingleObject(pi.hProcess, INFINITE);
        if (capture_streams) {
            _AM_DRAIN(out_rd, out_buf, out_cap, out_len);
            _AM_DRAIN(err_rd, err_buf, err_cap, err_len);
        }
        r->Exit = AMALGAME_PROCESS_TIMEOUT_EXIT;
    } else {
        if (capture_streams) {
            _AM_DRAIN(out_rd, out_buf, out_cap, out_len);
            _AM_DRAIN(err_rd, err_buf, err_cap, err_len);
        }
        DWORD code = 0;
        GetExitCodeProcess(pi.hProcess, &code);
        r->Exit = (i64) code;
    }
    if (capture_streams) {
        r->Stdout = out_buf ? out_buf : code_strdup("");
        r->Stderr = err_buf ? err_buf : code_strdup("");
        CloseHandle(out_rd); CloseHandle(err_rd);
    }
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    #undef _AM_DRAIN
    return r;
}

#endif /* _WIN32 */

/* Public v2 entry points. Thin wrappers over _am_proc_runex. */

static inline i64 Process_RunTimeout(code_string cmd, i64 timeout_ms) {
    AmalgameProcessResult* r = _am_proc_runex(cmd, timeout_ms, 0);
    return r->Exit;
}

static inline AmalgameProcessResult* Process_RunCaptureBoth(code_string cmd) {
    return _am_proc_runex(cmd, 0, 1);
}

static inline AmalgameProcessResult* Process_RunCaptureBothTimeout(
        code_string cmd, i64 timeout_ms) {
    return _am_proc_runex(cmd, timeout_ms, 1);
}

#endif /* AMALGAME_PROCESS_H */
