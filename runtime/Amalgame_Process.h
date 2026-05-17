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

/* ============================================================
 *  v3: streaming Spawn — persistent handle, line-by-line I/O
 *      POSIX only for now (fork + pipe + waitpid + select).
 *      Windows path (CreateProcess + Named Pipes) lands in v3.1.
 * ============================================================ */

#ifndef _WIN32

typedef struct AmalgameProcessHandle {
    pid_t pid;
    int   stdin_fd;     /* parent's write end, -1 if not piped */
    int   stdout_fd;    /* parent's read  end, -1 if not piped */
    int   stderr_fd;    /* parent's read  end, -1 if not piped */
    int   alive;        /* 1 until waitpid reaps */
    int   exit_code;    /* -1 until reaped */
    /* Per-stream line accumulator. ReadLine pulls bytes via
     * non-blocking read() and stops at '\n'; the leftover
     * bytes (no newline yet) live here until the next call
     * appends more and finds the terminator. */
    char* out_buf;
    size_t out_len;
    size_t out_cap;
    char* err_buf;
    size_t err_len;
    size_t err_cap;
} AmalgameProcessHandle;

/* Spawn a child process via /bin/sh -c. captureStreams flags:
 *   bit 0 (1) — pipe stdin  (parent can WriteLine)
 *   bit 1 (2) — pipe stdout (parent can ReadLine)
 *   bit 2 (4) — pipe stderr (parent can ReadErrLine)
 * Pass 7 to pipe all three. Pass 0 for an inherit-parent-stdio
 * child (useful for background scripts that don't need IPC).
 *
 * Returns the handle even on partial failure; check pid > 0
 * (parent saw fork OK) and alive == 1 before using stream
 * methods. Internal pipes are O_NONBLOCK on the parent side
 * so ReadLine/WriteLine never block past their timeout.
 */
static inline AmalgameProcessHandle* Process_Spawn(code_string cmd, i64 captureStreams) {
    AmalgameProcessHandle* h =
        (AmalgameProcessHandle*) code_alloc(sizeof(AmalgameProcessHandle));
    h->pid       = -1;
    h->stdin_fd  = -1;
    h->stdout_fd = -1;
    h->stderr_fd = -1;
    h->alive     = 0;
    h->exit_code = -1;
    h->out_buf   = NULL;
    h->out_len   = 0;
    h->out_cap   = 0;
    h->err_buf   = NULL;
    h->err_len   = 0;
    h->err_cap   = 0;
    if (!cmd) return h;

    int want_in  = (captureStreams & 1) ? 1 : 0;
    int want_out = (captureStreams & 2) ? 1 : 0;
    int want_err = (captureStreams & 4) ? 1 : 0;

    int in_pipe[2]  = {-1, -1};
    int out_pipe[2] = {-1, -1};
    int err_pipe[2] = {-1, -1};
    if (want_in  && pipe(in_pipe)  < 0) return h;
    if (want_out && pipe(out_pipe) < 0) {
        if (want_in) { close(in_pipe[0]); close(in_pipe[1]); }
        return h;
    }
    if (want_err && pipe(err_pipe) < 0) {
        if (want_in)  { close(in_pipe[0]);  close(in_pipe[1]);  }
        if (want_out) { close(out_pipe[0]); close(out_pipe[1]); }
        return h;
    }

    pid_t pid = fork();
    if (pid < 0) {
        if (want_in)  { close(in_pipe[0]);  close(in_pipe[1]);  }
        if (want_out) { close(out_pipe[0]); close(out_pipe[1]); }
        if (want_err) { close(err_pipe[0]); close(err_pipe[1]); }
        return h;
    }

    if (pid == 0) {
        /* ── child ── */
        if (want_in)  {
            dup2(in_pipe[0],  STDIN_FILENO);
            close(in_pipe[0]); close(in_pipe[1]);
        }
        if (want_out) {
            dup2(out_pipe[1], STDOUT_FILENO);
            close(out_pipe[0]); close(out_pipe[1]);
        }
        if (want_err) {
            dup2(err_pipe[1], STDERR_FILENO);
            close(err_pipe[0]); close(err_pipe[1]);
        }
        execl("/bin/sh", "sh", "-c", (const char*) cmd, (char*) NULL);
        _exit(127);
    }

    /* ── parent ── */
    h->pid   = pid;
    h->alive = 1;
    if (want_in) {
        close(in_pipe[0]);
        fcntl(in_pipe[1], F_SETFL, fcntl(in_pipe[1], F_GETFL, 0) | O_NONBLOCK);
        h->stdin_fd = in_pipe[1];
    }
    if (want_out) {
        close(out_pipe[1]);
        fcntl(out_pipe[0], F_SETFL, fcntl(out_pipe[0], F_GETFL, 0) | O_NONBLOCK);
        h->stdout_fd = out_pipe[0];
    }
    if (want_err) {
        close(err_pipe[1]);
        fcntl(err_pipe[0], F_SETFL, fcntl(err_pipe[0], F_GETFL, 0) | O_NONBLOCK);
        h->stderr_fd = err_pipe[0];
    }
    return h;
}

/* Non-blocking probe of the child status. Returns 1 if alive,
 * 0 if reaped (exit_code populated). Side-effect: refreshes
 * h->alive and h->exit_code on transition. */
static inline code_bool Process_IsAlive(AmalgameProcessHandle* h) {
    if (!h || h->pid < 0) return 0;
    if (!h->alive) return 0;
    int status = 0;
    pid_t w = waitpid(h->pid, &status, WNOHANG);
    if (w == h->pid) {
        h->alive = 0;
        h->exit_code = (int) _am_decode_status(status);
        return 0;
    }
    return 1;
}

static inline i64 Process_ExitCode(AmalgameProcessHandle* h) {
    if (!h) return -1;
    (void) Process_IsAlive(h);   /* refresh state */
    return (i64) h->exit_code;
}

/* Block up to timeout_ms (≤ 0 = forever) for the child to exit.
 * Returns true if reaped within the budget. */
static inline code_bool Process_Wait(AmalgameProcessHandle* h, i64 timeout_ms) {
    if (!h || h->pid < 0) return 0;
    if (!h->alive) return 1;
    if (timeout_ms <= 0) {
        int status = 0;
        if (waitpid(h->pid, &status, 0) == h->pid) {
            h->alive = 0;
            h->exit_code = (int) _am_decode_status(status);
            return 1;
        }
        return 0;
    }
    struct timeval start, now;
    gettimeofday(&start, NULL);
    while (1) {
        if (!Process_IsAlive(h)) return 1;
        gettimeofday(&now, NULL);
        i64 elapsed = (i64)(now.tv_sec - start.tv_sec) * 1000
                    + (i64)(now.tv_usec - start.tv_usec) / 1000;
        if (elapsed >= timeout_ms) return 0;
        struct timespec ts = { 0, 10 * 1000000L };  /* 10ms poll */
        nanosleep(&ts, NULL);
    }
}

/* Send SIGTERM (graceful). */
static inline void Process_Kill(AmalgameProcessHandle* h) {
    if (h && h->pid > 0 && h->alive) kill(h->pid, SIGTERM);
}
/* Send SIGKILL (hard). */
static inline void Process_KillForce(AmalgameProcessHandle* h) {
    if (h && h->pid > 0 && h->alive) kill(h->pid, SIGKILL);
}

/* Write a UTF-8 string to the child's stdin, appending a '\n'
 * if the string doesn't already end with one. Returns true on
 * full write. Non-blocking — fails if the kernel pipe is full
 * (caller can retry). */
static inline code_bool Process_WriteLine(AmalgameProcessHandle* h, code_string s) {
    if (!h || h->stdin_fd < 0 || !s) return 0;
    size_t n = strlen(s);
    int need_nl = (n == 0 || s[n - 1] != '\n') ? 1 : 0;
    size_t total = n + (need_nl ? 1 : 0);
    char* buf = (char*) GC_MALLOC(total);
    if (n > 0) memcpy(buf, s, n);
    if (need_nl) buf[n] = '\n';
    ssize_t k = write(h->stdin_fd, buf, total);
    return (k == (ssize_t) total) ? 1 : 0;
}

/* Internal: read from fd into the buffer, expanding as needed.
 * Stops on a '\n' or when read returns EAGAIN. Returns the
 * index of the newline (≥0) or -1 if no newline yet. */
static inline ssize_t _am_proc_pump(int fd, char** buf, size_t* len, size_t* cap, i64 timeout_ms) {
    if (fd < 0) return -1;
    /* Check the existing buffer first. */
    if (*buf && *len > 0) {
        for (size_t i = 0; i < *len; i++) {
            if ((*buf)[i] == '\n') return (ssize_t) i;
        }
    }
    /* Use select for the timeout. */
    struct timeval tv;
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);
    if (timeout_ms > 0) {
        tv.tv_sec  = (long) (timeout_ms / 1000);
        tv.tv_usec = (long) ((timeout_ms % 1000) * 1000);
    } else {
        tv.tv_sec = 0; tv.tv_usec = 0;
    }
    int sr = select(fd + 1, &rfds, NULL, NULL, &tv);
    if (sr <= 0) return -1;   /* timeout or error */

    char chunk[4096];
    ssize_t k = read(fd, chunk, sizeof(chunk));
    if (k <= 0) return -2;    /* EOF or error */
    /* Grow buffer if needed. */
    size_t need = *len + (size_t) k + 1;
    if (need > *cap) {
        size_t nc = *cap ? *cap : 256;
        while (nc < need) nc *= 2;
        char* nb = (char*) GC_MALLOC(nc);
        if (*len > 0) memcpy(nb, *buf, *len);
        *buf = nb;
        *cap = nc;
    }
    memcpy(*buf + *len, chunk, (size_t) k);
    *len += (size_t) k;
    (*buf)[*len] = '\0';
    for (size_t i = 0; i < *len; i++) {
        if ((*buf)[i] == '\n') return (ssize_t) i;
    }
    return -1;
}

/* Internal: extract a line at index `nl` (newline position),
 * shift the remainder back to the front, return the line as
 * a fresh GC-allocated string (newline stripped). */
static inline code_string _am_proc_extract(char** buf, size_t* len, ssize_t nl) {
    char* p = (char*) code_alloc((size_t) nl + 1);
    if (nl > 0) memcpy(p, *buf, (size_t) nl);
    p[nl] = '\0';
    /* Shift the rest down. */
    size_t after = (size_t) nl + 1;
    if (after < *len) {
        memmove(*buf, *buf + after, *len - after);
        *len -= after;
    } else {
        *len = 0;
    }
    return p;
}

/* Read one '\n'-terminated line from the child's stdout. The
 * newline is stripped. Returns "" on timeout, NULL sentinel
 * (encoded as the literal string "__EOF__") on EOF. */
static inline code_string Process_ReadLine(AmalgameProcessHandle* h, i64 timeout_ms) {
    if (!h || h->stdout_fd < 0) return (code_string) "";
    ssize_t nl = _am_proc_pump(h->stdout_fd, &h->out_buf, &h->out_len, &h->out_cap, timeout_ms);
    if (nl == -2) {
        /* EOF — flush any remaining buffer as a final partial line. */
        if (h->out_len > 0) {
            return _am_proc_extract(&h->out_buf, &h->out_len, (ssize_t) h->out_len);
        }
        return (code_string) "__EOF__";
    }
    if (nl < 0) return (code_string) "";    /* timeout */
    return _am_proc_extract(&h->out_buf, &h->out_len, nl);
}

/* Same shape for stderr. */
static inline code_string Process_ReadErrLine(AmalgameProcessHandle* h, i64 timeout_ms) {
    if (!h || h->stderr_fd < 0) return (code_string) "";
    ssize_t nl = _am_proc_pump(h->stderr_fd, &h->err_buf, &h->err_len, &h->err_cap, timeout_ms);
    if (nl == -2) {
        if (h->err_len > 0) {
            return _am_proc_extract(&h->err_buf, &h->err_len, (ssize_t) h->err_len);
        }
        return (code_string) "__EOF__";
    }
    if (nl < 0) return (code_string) "";
    return _am_proc_extract(&h->err_buf, &h->err_len, nl);
}

#else  /* _WIN32 — streaming Spawn not yet implemented */

typedef struct AmalgameProcessHandle {
    int unused;
} AmalgameProcessHandle;

static inline AmalgameProcessHandle* Process_Spawn(code_string cmd, i64 captureStreams) {
    (void) cmd; (void) captureStreams;
    return NULL;  /* TODO v3.1: CreateProcess + Named Pipes */
}
static inline code_bool   Process_IsAlive(AmalgameProcessHandle* h)        { (void) h; return 0; }
static inline i64         Process_ExitCode(AmalgameProcessHandle* h)       { (void) h; return -1; }
static inline code_bool   Process_Wait(AmalgameProcessHandle* h, i64 t)    { (void) h; (void) t; return 0; }
static inline void        Process_Kill(AmalgameProcessHandle* h)            { (void) h; }
static inline void        Process_KillForce(AmalgameProcessHandle* h)       { (void) h; }
static inline code_bool   Process_WriteLine(AmalgameProcessHandle* h, code_string s)     { (void) h; (void) s; return 0; }
static inline code_string Process_ReadLine(AmalgameProcessHandle* h, i64 t)              { (void) h; (void) t; return (code_string) ""; }
static inline code_string Process_ReadErrLine(AmalgameProcessHandle* h, i64 t)           { (void) h; (void) t; return (code_string) ""; }

#endif /* _WIN32 */

#endif /* AMALGAME_PROCESS_H */
