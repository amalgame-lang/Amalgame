/*
 * Amalgame Standard Library — Amalgame.IO
 * Copyright (c) 2026 Bastien MOUGET
 * https://github.com/amalgame-lang/Amalgame
 *
 * Provides: Console, File, Path, Environment
 */

#ifndef AMALGAME_IO_H
#define AMALGAME_IO_H

#include "_runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#ifdef _WIN32
  #include <direct.h>  /* _mkdir */
#endif

/* ─────────────────────────────────────────────
   Console
   ───────────────────────────────────────────── */

/* Already in _runtime.h:
   Console_WriteLine(code_string)
   Console_Write(code_string)
   Console_ReadLine() → code_string
*/

static inline void Console_WriteError(code_string s) {
    fprintf(stderr, "%s\n", s ? s : "");
}

static inline code_string Console_ReadPassword() {
    /* Simple implementation — no echo disable on all platforms */
    return Console_ReadLine();
}

static inline void Console_Clear() {
#ifdef _WIN32
    system("cls");
#else
    printf("\033[2J\033[H");
#endif
}

/* ─────────────────────────────────────────────
   File
   ───────────────────────────────────────────── */

static inline code_string File_ReadAll(code_string path) {
    FILE* f = fopen(path, "r");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char* buf = (char*) GC_MALLOC(size + 1);
    size_t got = fread(buf, 1, (size_t) size, f);
    buf[got] = '\0';
    fclose(f);
    return buf;
}

static inline code_bool File_WriteAll(code_string path,
                                       code_string content) {
    // Pin content against GC collection during file write
    if (!content) content = "";
    GC_add_roots((void*)content, (void*)(content + strlen(content) + 1));
    FILE* f = fopen(path, "w");
    if (!f) {
        GC_remove_roots((void*)content, (void*)(content + strlen(content) + 1));
        return false;
    }
    size_t len = strlen(content);
    fwrite(content, 1, len, f);
    fclose(f);
    GC_remove_roots((void*)content, (void*)(content + strlen(content) + 1));
    return true;
}

static inline code_bool File_AppendAll(code_string path,
                                        code_string content) {
    FILE* f = fopen(path, "a");
    if (!f) return false;
    fputs(content, f);
    fclose(f);
    return true;
}

static inline code_bool File_Exists(code_string path) {
    struct stat st;
    return stat(path, &st) == 0;
}

static inline code_bool File_Delete(code_string path) {
    return remove(path) == 0;
}

static inline i64 File_Size(code_string path) {
    struct stat st;
    if (stat(path, &st) != 0) return -1;
    return (i64) st.st_size;
}

/* Last-modification time as a Unix epoch (seconds since 1970-01-01 UTC).
 * Returns -1 if the path doesn't exist or stat() fails. Used by HTTP
 * static-file serving (Last-Modified header, ETag = "size-mtime"). */
static inline i64 File_Mtime(code_string path) {
    struct stat st;
    if (stat(path, &st) != 0) return -1;
    return (i64) st.st_mtime;
}

/* True iff `path` exists AND points to a regular file (not a directory,
 * symlink-to-dir, device, fifo, etc.). Distinct from File_Exists, which
 * accepts any inode type. The static-file middleware uses this to
 * reject GET /assets/ (a directory) with a 403 rather than passing the
 * dir path to File_ReadAll (where fopen() silently succeeds on some
 * platforms and yields garbage). */
static inline code_bool File_IsFile(code_string path) {
    struct stat st;
    if (stat(path, &st) != 0) return false;
    return S_ISREG(st.st_mode) ? true : false;
}

/* True iff `path` exists AND points to a directory. Symmetric companion
 * to File_IsFile for callers that need to branch on inode type. */
static inline code_bool File_IsDir(code_string path) {
    struct stat st;
    if (stat(path, &st) != 0) return false;
    return S_ISDIR(st.st_mode) ? true : false;
}

/* Cross-platform recursive mkdir — equivalent to POSIX `mkdir -p`.
 * On Windows we shell out to cmd.exe via Process_Run elsewhere, but
 * `mkdir -p name` runs `mkdir -p` literally there (cmd's mkdir has
 * no -p), creating folders called `-p` and `'name'`. So callers that
 * need cross-platform behaviour must use this helper instead.
 * Walks the path, mkdir-ing each prefix segment. Idempotent: if a
 * segment already exists, EEXIST is ignored. */
static inline code_bool File_Mkdir(code_string path) {
    if (!path || !*path) return false;
    size_t len = strlen(path);
    char* buf = (char*) GC_MALLOC(len + 1);
    memcpy(buf, path, len + 1);

    /* Skip the leading non-data prefix so we don't try to mkdir("")
     * or mkdir("C:"). Relative paths start their first segment at
     * index 0 (we still skip i=0 in the walk by starting at 1).  */
    size_t start = 1;
#ifdef _WIN32
    if (len >= 2 && buf[1] == ':') {
        start = (len >= 3 && (buf[2] == '\\' || buf[2] == '/')) ? 3 : 2;
    } else if (len >= 1 && (buf[0] == '/' || buf[0] == '\\')) {
        start = 1;
    }
#else
    if (buf[0] == '/') start = 1;
#endif

    for (size_t i = start; i <= len; i++) {
        char c = (i < len) ? buf[i] : '\0';
        if (i == len || c == '/' || c == '\\') {
            buf[i] = '\0';
            int rc;
#ifdef _WIN32
            rc = _mkdir(buf);
#else
            rc = mkdir(buf, 0755);
#endif
            buf[i] = c;
            if (rc != 0 && errno != EEXIST) return false;
        }
    }
    return true;
}

/* ─────────────────────────────────────────────
   Path
   ───────────────────────────────────────────── */

static inline code_string Path_Combine(code_string a,
                                        code_string b) {
    size_t la = strlen(a);
    size_t lb = strlen(b);
    char* r = (char*) GC_MALLOC(la + lb + 2);
    memcpy(r, a, la);
    if (la > 0 && a[la-1] != '/' && a[la-1] != '\\')
        r[la++] = '/';
    memcpy(r + la, b, lb + 1);
    return r;
}

static inline code_string Path_GetExtension(code_string path) {
    const char* dot = strrchr(path, '.');
    if (!dot || dot == path) return "";
    return code_strdup(dot);
}

static inline code_string Path_GetFilename(code_string path) {
    const char* slash = strrchr(path, '/');
    if (!slash) slash = strrchr(path, '\\');
    if (!slash) return code_strdup(path);
    return code_strdup(slash + 1);
}

static inline code_string Path_GetDirectory(code_string path) {
    const char* slash = strrchr(path, '/');
    if (!slash) slash = strrchr(path, '\\');
    if (!slash) return ".";
    size_t len = (size_t)(slash - path);
    char* r = (char*) GC_MALLOC(len + 1);
    memcpy(r, path, len);
    r[len] = '\0';
    return r;
}

/* Filename minus its extension, mirroring Python's `Path.stem` and
 * Rust's `Path::file_stem`. Uses the same separator handling as
 * Path_GetFilename — last `/` or `\` wins — then drops everything
 * from the rightmost `.` onward. Matches the convention that
 * "report.tar.gz" stems to "report.tar" (only the LAST extension
 * is stripped). A leading-dot dotfile ("./.bashrc") stems to
 * itself, since the rightmost dot IS the leading dot. */
static inline code_string Path_GetStem(code_string path) {
    const char* slash = strrchr(path, '/');
    if (!slash) slash = strrchr(path, '\\');
    const char* base = slash ? slash + 1 : path;
    const char* dot  = strrchr(base, '.');
    if (!dot || dot == base) return code_strdup(base);
    size_t len = (size_t)(dot - base);
    char* r = (char*) GC_MALLOC(len + 1);
    memcpy(r, base, len);
    r[len] = '\0';
    return r;
}

/* True iff path starts with `/` (POSIX) or `<drive>:` (Windows
 * absolute), i.e. the path resolves without consulting any cwd.
 * Backslash-only Windows roots like `\\\\server\\share` and `\\?\\…`
 * UNCs also count as absolute. Mirrors Python's `os.path.isabs`. */
static inline code_bool Path_IsAbsolute(code_string path) {
    if (!path || !path[0]) return 0;
    if (path[0] == '/' || path[0] == '\\') return 1;
    /* Windows drive letter: a-zA-Z then ':'. */
    if (((path[0] >= 'A' && path[0] <= 'Z') ||
         (path[0] >= 'a' && path[0] <= 'z')) &&
        path[1] == ':') return 1;
    return 0;
}

/* Native path separator: "/" on POSIX, "\\" on Windows. The runtime
 * accepts both throughout (Path_Combine et al normalize on insert),
 * but callers writing platform-native code paths still need it
 * for shelling out, registry keys, etc. */
static inline code_string Path_Sep(void) {
#ifdef _WIN32
    return "\\";
#else
    return "/";
#endif
}

/* Lexical normalisation: collapse runs of `/` and `\\`, drop `.`
 * components, resolve `..` against earlier components. Pure string
 * operation — does NOT touch the filesystem (so `..` past a
 * symlink may resolve incorrectly relative to the real path; use
 * realpath(3) when you need that). Trailing separator is removed
 * unless the path is exactly "/" or a Windows root.
 *
 * Mirrors Go's filepath.Clean semantics — chosen over libc
 * realpath(3) precisely because it works on non-existent paths,
 * which is what most Path API callers want for path manipulation. */
static inline code_string Path_Normalize(code_string path) {
    if (!path) return code_strdup("");
    size_t n = strlen(path);
    if (n == 0) return code_strdup(".");
    /* Detect whether path is absolute and/or starts with a drive.
     * POSIX: leading '/'. Windows drive: `[A-Za-z]:`. */
    int absolute = (path[0] == '/' || path[0] == '\\');
    int drive    = 0;
    size_t start = 0;
    if (((path[0] >= 'A' && path[0] <= 'Z') ||
         (path[0] >= 'a' && path[0] <= 'z')) &&
        n >= 2 && path[1] == ':') {
        drive = 1;
        start = 2;
        if (n > 2 && (path[2] == '/' || path[2] == '\\')) {
            absolute = 1;
            start = 3;
        }
    } else if (absolute) {
        start = 1;
    }
    /* Tokenise on '/' or '\\'. Stack of component pointers + lengths
     * into the original string; walk back on `..`. */
    const char* parts[256];
    size_t      lens [256];
    int top = 0;
    size_t i = start;
    while (i < n) {
        while (i < n && (path[i] == '/' || path[i] == '\\')) i++;
        if (i >= n) break;
        size_t j = i;
        while (j < n && path[j] != '/' && path[j] != '\\') j++;
        size_t len = j - i;
        if (len == 1 && path[i] == '.') {
            /* skip "." */
        } else if (len == 2 && path[i] == '.' && path[i+1] == '.') {
            if (top > 0 && !(lens[top-1] == 2 && parts[top-1][0] == '.' && parts[top-1][1] == '.')) {
                top--;
            } else if (!absolute) {
                if (top < 256) {
                    parts[top] = path + i;
                    lens [top] = len;
                    top++;
                }
            }
            /* absolute + ".." past root: drop silently */
        } else {
            if (top < 256) {
                parts[top] = path + i;
                lens [top] = len;
                top++;
            }
        }
        i = j;
    }
    /* Compute output length. */
    size_t out_len = start;
    for (int k = 0; k < top; k++) {
        if (k > 0) out_len += 1;
        out_len += lens[k];
    }
    if (out_len == 0) return code_strdup(".");
    char* r = (char*) GC_MALLOC(out_len + 1);
    size_t w = 0;
    /* Always emit '/' for consistency — Windows accepts forward
     * slashes everywhere, and a deterministic canonical form is
     * what callers expect from Normalize. */
    if (drive) {
        r[w++] = path[0];
        r[w++] = ':';
        if (absolute) r[w++] = '/';
    } else if (absolute) {
        r[w++] = '/';
    }
    for (int k = 0; k < top; k++) {
        if (k > 0) r[w++] = '/';
        memcpy(r + w, parts[k], lens[k]);
        w += lens[k];
    }
    r[w] = '\0';
    return r;
}

/* ─────────────────────────────────────────────
   Environment
   ───────────────────────────────────────────── */

static inline code_string Environment_GetVar(code_string name) {
    const char* val = getenv(name);
    return val ? code_strdup(val) : NULL;
}

static inline code_string Environment_GetVarOr(code_string name,
                                                code_string fallback) {
    const char* val = getenv(name);
    return val ? code_strdup(val) : fallback;
}

static inline code_bool Environment_HasVar(code_string name) {
    return getenv(name) != NULL;
}

/* Short aliases exposed to Amalgame as `Env.Get(...)` / `Env.Has(...)`.
   The resolver declares the prefixed names; CGen maps `Env.Get` to
   `Env_Get` via the isStdlib branch. */
static inline code_string Env_Get(code_string name) {
    return Environment_GetVar(name);
}
static inline code_bool Env_Has(code_string name) {
    return Environment_HasVar(name);
}

#endif /* AMALGAME_IO_H */
