/*
 * Amalgame Standard Library — Amalgame.IO.FileWatcher
 * Copyright (c) 2026 Bastien MOUGET
 * https://github.com/amalgame-lang/Amalgame
 *
 * Provides: FileWatcher — single-file mtime polling.
 *
 * Cross-platform via stat()/_stat. Watches one file at a time;
 * `Changed()` returns true the first time it's called after the
 * file's mtime advances (or the file disappears / reappears).
 * Same call returns false until the next change — caller drives
 * the polling loop:
 *
 *   let w = new FileWatcher("/etc/foo.toml")
 *   while (true) {
 *       if (w.Changed()) { reload() }
 *       Process_Sleep(500)
 *   }
 *
 * For directory-level recursive watches, inotify / FSEvents /
 * ReadDirectoryChangesW are needed — out of scope for v1; the
 * polling MVP covers the "reload a config file" / "rebuild on
 * source change" cases that motivate 80% of users.
 */

#ifndef AMALGAME_FILEWATCH_H
#define AMALGAME_FILEWATCH_H

#include "_runtime.h"
#include <sys/stat.h>
#include <stdint.h>

#ifdef _WIN32
#include <sys/types.h>
#define amalgame_stat _stat64
#define amalgame_stat_struct struct __stat64
#define amalgame_mtime(s) ((int64_t)(s).st_mtime)
#else
#define amalgame_stat stat
#define amalgame_stat_struct struct stat
#define amalgame_mtime(s) ((int64_t)(s).st_mtime)
#endif

typedef struct AmalgameFileWatcher {
    code_string path;
    int64_t     lastMtime;   /* seconds since epoch, or -1 if absent */
} AmalgameFileWatcher;

/* Internal: return current mtime, or -1 if the file is absent /
 * unreadable. Doesn't follow symlinks differently from stat(2). */
static inline int64_t FileWatcher__Mtime(code_string path) {
    if (!path) { return -1; }
    amalgame_stat_struct st;
    if (amalgame_stat(path, &st) != 0) { return -1; }
    return amalgame_mtime(st);
}

static inline AmalgameFileWatcher* FileWatcher_new(code_string path) {
    AmalgameFileWatcher* w = (AmalgameFileWatcher*)GC_MALLOC(sizeof(AmalgameFileWatcher));
    w->path = path;
    w->lastMtime = FileWatcher__Mtime(path);
    return w;
}

/* Returns the path being watched (the same `path` passed at
 * construction). Useful for `Console.WriteLine("watching " +
 * w.GetPath())` in callers that hand FileWatcher instances around. */
static inline code_string FileWatcher_GetPath(AmalgameFileWatcher* w) {
    return w->path;
}

/* Returns true iff the file's mtime has advanced (or the file
 * appeared / disappeared) since the previous call. After a true
 * return, the watcher's internal snapshot is updated to the new
 * mtime so the next call is comparing against the latest state. */
static inline code_bool FileWatcher_Changed(AmalgameFileWatcher* w) {
    int64_t cur = FileWatcher__Mtime(w->path);
    if (cur != w->lastMtime) {
        w->lastMtime = cur;
        return 1;
    }
    return 0;
}

/* True iff the file currently exists (stat succeeded). Cheap —
 * a single stat() call. Distinct from `Changed()` so callers can
 * tell "file is missing right now" from "file changed".  */
static inline code_bool FileWatcher_Exists(AmalgameFileWatcher* w) {
    return (FileWatcher__Mtime(w->path) >= 0) ? 1 : 0;
}

#endif /* AMALGAME_FILEWATCH_H */
