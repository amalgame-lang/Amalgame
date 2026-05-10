/*
 * Amalgame Standard Library — Amalgame.Service
 * Copyright (c) 2026 Bastien MOUGET
 * https://github.com/amalgame-lang/amalgame
 *
 * Long-running background process primitives. POSIX-flavoured —
 * the signal-handling path uses `signal()` (SIGTERM / SIGINT)
 * and `nanosleep()`. Windows path uses `SetConsoleCtrlHandler`
 * + `Sleep()` from the standard runtime. Both compile to the
 * same Amalgame surface: `Service.Install()`, `Service.ShouldStop()`,
 * `Service.Sleep(ms)`.
 *
 * The single-process singleton flag is a `sig_atomic_t` so the
 * signal handler can flip it without locking. No mutex around it
 * — signal handlers can only touch async-signal-safe state, and a
 * single flag is the simplest way to express "the main loop should
 * exit after the current iteration".
 */

#ifndef AMALGAME_SERVICE_H
#define AMALGAME_SERVICE_H

#include "_runtime.h"
#include <signal.h>

#ifdef _WIN32
#include <windows.h>
static volatile LONG Amalgame_Service_Stopping = 0;

static BOOL WINAPI Amalgame_Service_OnCtrl(DWORD ctrlType) {
    /* Catch CTRL_C, CTRL_BREAK, CTRL_CLOSE, CTRL_LOGOFF, CTRL_SHUTDOWN.
     * Returning TRUE tells Windows we handled it — the process is not
     * terminated; the main loop notices the flag on next ShouldStop. */
    (void) ctrlType;
    InterlockedExchange(&Amalgame_Service_Stopping, 1);
    return TRUE;
}

static inline void Service_Install(void) {
    SetConsoleCtrlHandler(Amalgame_Service_OnCtrl, TRUE);
}

static inline code_bool Service_ShouldStop(void) {
    return Amalgame_Service_Stopping ? 1 : 0;
}

static inline void Service_RequestStop(void) {
    InterlockedExchange(&Amalgame_Service_Stopping, 1);
}

static inline void Service_Sleep(i64 ms) {
    /* Slice the sleep into 100ms chunks so a SetConsoleCtrlHandler
     * call mid-sleep can interrupt within bounded latency. Windows
     * has no portable `nanosleep`-equivalent that's interruptible
     * by a Ctrl-handler callback, so polling is the cleanest path. */
    if (ms <= 0) return;
    i64 remaining = ms;
    while (remaining > 0) {
        if (Amalgame_Service_Stopping) return;
        DWORD chunk = remaining > 100 ? 100 : (DWORD) remaining;
        Sleep(chunk);
        remaining -= chunk;
    }
}

#else
#include <time.h>
static volatile sig_atomic_t Amalgame_Service_Stopping = 0;

static void Amalgame_Service_OnSignal(int sig) {
    (void) sig;
    Amalgame_Service_Stopping = 1;
}

static inline void Service_Install(void) {
    /* Best-effort: ignore errors. A failure here just means the
     * shutdown path falls back to whatever the default disposition
     * is (typically: SIGTERM kills the process abruptly). The
     * service still runs; it just won't clean-shutdown. */
    struct sigaction sa;
    sa.sa_handler = Amalgame_Service_OnSignal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;  /* not SA_RESTART — let blocking syscalls return EINTR */
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT,  &sa, NULL);
}

static inline code_bool Service_ShouldStop(void) {
    return Amalgame_Service_Stopping ? 1 : 0;
}

static inline void Service_RequestStop(void) {
    Amalgame_Service_Stopping = 1;
}

static inline void Service_Sleep(i64 ms) {
    /* Interruptible nanosleep: if the signal handler fires
     * mid-sleep, nanosleep returns -1 with EINTR and we exit
     * early. Caller's loop sees ShouldStop() == true on the
     * next iteration. */
    if (ms <= 0) return;
    if (Amalgame_Service_Stopping) return;
    struct timespec ts;
    ts.tv_sec  = (time_t) (ms / 1000);
    ts.tv_nsec = (long)   ((ms % 1000) * 1000000L);
    nanosleep(&ts, NULL);
}
#endif

#endif /* AMALGAME_SERVICE_H */
