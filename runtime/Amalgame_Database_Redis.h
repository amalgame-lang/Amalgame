/*
 * Amalgame Standard Library — Amalgame.Database.NoSQL.Redis
 * Copyright (c) 2026 Bastien MOUGET
 * https://github.com/amalgame-lang/Amalgame
 *
 * Redis client speaking the RESP (Redis Serialization Protocol) v2
 * directly over a BSD-socket TCP connection. No external client lib —
 * the protocol fits in ~250 lines of C and the wire format is identical
 * between Redis, KeyDB, Dragonfly, and Valkey, so this binding works
 * against all of them.
 *
 * Surface (v1):
 *   Open / Close / IsOpen / LastError      — lifecycle
 *   Ping                                   — health probe
 *   Set / Get / Del / Exists               — key/value
 *   Incr / Decr                            — atomic counters
 *   Expire                                 — TTL
 *
 * Out of scope for v1 (tracked for v2):
 *   AUTH / SELECT db, pipelining, pub/sub, MULTI/EXEC transactions,
 *   SCAN / KEYS array replies, binary-safe values with embedded NULs,
 *   TLS, connection pooling, auto-reconnect.
 *
 * Reuses the cross-platform socket layer from Amalgame_Net.h
 * (`_amnet_init_once`, `_amnet_close_socket`, the winsock2 vs BSD
 * conditional). User binaries linking against this header don't need
 * libcurl unless they also pull in `Http_*` — the curl includes in
 * Amalgame_Net.h are gated on `__has_include`.
 *
 * Threading: an `AmalgameRedis*` handle is single-owner. Concurrent
 * calls on the same handle from different threads are undefined; if
 * you need shared access, wrap the handle in a mutex on the caller
 * side. Concurrent calls on different handles are safe.
 */

#ifndef AMALGAME_DATABASE_REDIS_H
#define AMALGAME_DATABASE_REDIS_H

#include "_runtime.h"
#include "Amalgame_Collections.h"
#include "Amalgame_Net.h"   /* cross-platform sockets + _amnet_init_once */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct AmalgameRedis {
    int         fd;          /* socket fd; -1 = not connected */
    char*       last_error;  /* GC-strdup'd error message, or NULL */
} AmalgameRedis;

/* GC-dup an error message into a stable buffer. */
static inline code_string _amredis_err_dup(const char* msg) {
    if (!msg) return NULL;
    size_t n = strlen(msg);
    char* p = (char*) code_alloc(n + 1);
    memcpy(p, msg, n + 1);
    return p;
}

/* ── Lifecycle ──────────────────────────────────────── */

/* Open a TCP connection to redis://host:port. Returns a non-NULL
 * handle even on failure — call Redis.IsOpen() to check, or
 * Redis.LastError() for the message. */
static inline AmalgameRedis* Redis_Open(code_string host, i64 port) {
    _amnet_init_once();
    AmalgameRedis* r = (AmalgameRedis*) code_alloc(sizeof(AmalgameRedis));
    r->fd         = -1;
    r->last_error = NULL;

    if (!host || !*host) {
        r->last_error = _amredis_err_dup("host is empty");
        return r;
    }

    char portStr[16];
    snprintf(portStr, sizeof(portStr), "%lld", (long long) port);

    struct addrinfo hints = {0};
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo* res = NULL;
    if (getaddrinfo(host, portStr, &hints, &res) != 0) {
        r->last_error = _amredis_err_dup("getaddrinfo failed");
        return r;
    }

    int fd = (int) socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd < 0) {
        r->last_error = _amredis_err_dup("socket() failed");
        freeaddrinfo(res);
        return r;
    }
    if (connect(fd, res->ai_addr, (int) res->ai_addrlen) != 0) {
        r->last_error = _amredis_err_dup("connect() failed");
        _amnet_close_socket(fd);
        freeaddrinfo(res);
        return r;
    }
    freeaddrinfo(res);
    r->fd = fd;
    return r;
}

/* Close the socket. Idempotent. The wrapper struct itself is
 * GC-managed; we don't free it here. */
static inline void Redis_Close(AmalgameRedis* r) {
    if (r && r->fd >= 0) {
        _amnet_close_socket(r->fd);
        r->fd = -1;
    }
}

static inline code_bool Redis_IsOpen(AmalgameRedis* r) {
    return (r && r->fd >= 0) ? 1 : 0;
}

static inline code_string Redis_LastError(AmalgameRedis* r) {
    if (!r) return "";
    return r->last_error ? r->last_error : "";
}

/* ── Wire format (RESP2) ────────────────────────────── */

/* send() can short-write; this loops until every byte is on the
 * wire or the socket errors. Returns 0 on success, -1 on error. */
static inline int _amredis_send_all(int fd, const char* buf, size_t n) {
    size_t off = 0;
    while (off < n) {
        ssize_t k = send(fd, buf + off, n - off, 0);
        if (k <= 0) return -1;
        off += (size_t) k;
    }
    return 0;
}

/* Encode a command as a RESP array of bulk strings.
 *   SET foo bar → *3\r\n$3\r\nSET\r\n$3\r\nfoo\r\n$3\r\nbar\r\n
 *
 * Sized two-pass: first compute the total length, allocate exactly
 * that much, then fill in. Lets us send the whole command in one
 * send() and avoids fragmentation across multiple syscalls. */
static inline char* _amredis_build_cmd(int argc, const char* const* args, size_t* out_n) {
    size_t total = 0;
    char tmp[24];

    int hlen = snprintf(tmp, sizeof(tmp), "*%d\r\n", argc);
    total += (size_t) hlen;
    for (int i = 0; i < argc; i++) {
        const char* a = args[i] ? args[i] : "";
        size_t alen   = strlen(a);
        int llen      = snprintf(tmp, sizeof(tmp), "$%zu\r\n", alen);
        total += (size_t) llen + alen + 2; /* + trailing CRLF */
    }

    char* buf = (char*) code_alloc(total + 1);
    size_t pos = 0;
    int wrote = snprintf(buf + pos, total + 1 - pos, "*%d\r\n", argc);
    pos += (size_t) wrote;
    for (int i = 0; i < argc; i++) {
        const char* a = args[i] ? args[i] : "";
        size_t alen   = strlen(a);
        wrote = snprintf(buf + pos, total + 1 - pos, "$%zu\r\n", alen);
        pos += (size_t) wrote;
        memcpy(buf + pos, a, alen);
        pos += alen;
        buf[pos++] = '\r';
        buf[pos++] = '\n';
    }
    buf[pos] = '\0';
    *out_n = pos;
    return buf;
}

/* Read one RESP line (everything up to \r\n) into a GC buffer.
 * Returns NULL on EOF / socket error. Caller does NOT include the
 * leading prefix byte ('+', '-', ':', '$', '*') — that's read
 * separately by _amredis_read_reply. */
static inline char* _amredis_read_line(int fd) {
    size_t cap = 64;
    size_t n   = 0;
    char*  buf = (char*) code_alloc(cap);
    char   ch;
    while (1) {
        ssize_t k = recv(fd, &ch, 1, 0);
        if (k <= 0) return NULL;
        if (ch == '\r') {
            /* RESP frames end with CRLF; consume the LF and bail. */
            recv(fd, &ch, 1, 0);
            buf[n] = '\0';
            return buf;
        }
        if (n + 1 >= cap) {
            cap *= 2;
            char* nb = (char*) code_alloc(cap);
            memcpy(nb, buf, n);
            buf = nb;
        }
        buf[n++] = ch;
    }
}

/* Read exactly N bytes followed by \r\n. Used for bulk-string
 * payloads after we've parsed the length header. */
static inline char* _amredis_read_bulk(int fd, size_t n) {
    char* buf = (char*) code_alloc(n + 1);
    size_t off = 0;
    while (off < n) {
        ssize_t k = recv(fd, buf + off, n - off, 0);
        if (k <= 0) return NULL;
        off += (size_t) k;
    }
    buf[n] = '\0';
    /* Discard trailing CRLF — Redis always sends it. */
    char cr;
    recv(fd, &cr, 1, 0);
    recv(fd, &cr, 1, 0);
    return buf;
}

/* Parsed RESP reply. `kind` carries the prefix byte; on protocol /
 * socket error we report 'x' and the caller treats that as a
 * connection-level failure (distinct from a Redis-level '-' error). */
typedef struct {
    char        kind;     /* '+', '-', ':', '$', or 'x' on error */
    code_string str_val;  /* +, -, $ → the payload as a GC string */
    i64         int_val;  /* : → integer */
    code_bool   is_nil;   /* $ with length -1 (nil bulk string) */
} _AmRedisReply;

static inline _AmRedisReply _amredis_read_reply(int fd) {
    _AmRedisReply rep;
    rep.kind    = 'x';
    rep.str_val = "";
    rep.int_val = 0;
    rep.is_nil  = 0;

    char prefix;
    ssize_t k = recv(fd, &prefix, 1, 0);
    if (k <= 0) return rep;

    char* line = _amredis_read_line(fd);
    if (!line) return rep;

    rep.kind = prefix;
    if (prefix == '+' || prefix == '-') {
        rep.str_val = line;
    } else if (prefix == ':') {
        rep.int_val = (i64) atoll(line);
    } else if (prefix == '$') {
        long long len = atoll(line);
        if (len < 0) {
            rep.is_nil  = 1;
            rep.str_val = "";
        } else {
            char* data = _amredis_read_bulk(fd, (size_t) len);
            if (!data) { rep.kind = 'x'; rep.str_val = ""; }
            else       { rep.str_val = data; }
        }
    } else if (prefix == '*') {
        /* Array replies (KEYS, MGET, …) aren't surfaced in v1.
         * Mark as unsupported; the caller will report it via
         * Redis.LastError(). */
        rep.kind    = 'x';
        rep.str_val = "array replies not supported in v1";
    } else {
        rep.kind    = 'x';
        rep.str_val = "unexpected RESP prefix";
    }
    return rep;
}

/* Send command, read one reply, return true iff the reply is a
 * +SimpleString. Used by SET / EXPIRE / PING etc. — commands whose
 * success is indicated by '+OK' or '+PONG'. */
static inline code_bool _amredis_exec_simple(AmalgameRedis* r, int argc, const char* const* args) {
    if (!r || r->fd < 0) return 0;
    size_t cmd_n = 0;
    char*  cmd   = _amredis_build_cmd(argc, args, &cmd_n);
    if (_amredis_send_all(r->fd, cmd, cmd_n) < 0) {
        r->last_error = _amredis_err_dup("send() failed");
        return 0;
    }
    _AmRedisReply rep = _amredis_read_reply(r->fd);
    if (rep.kind == '+') return 1;
    if (rep.kind == ':') return rep.int_val != 0 ? 1 : 0;
    r->last_error = _amredis_err_dup(
        rep.str_val && *rep.str_val ? rep.str_val : "redis error");
    return 0;
}

/* ── Commands ───────────────────────────────────────── */

/* PING → +PONG. Cheap connection liveness probe. */
static inline code_bool Redis_Ping(AmalgameRedis* r) {
    const char* args[1] = { "PING" };
    return _amredis_exec_simple(r, 1, args);
}

/* SET key value → +OK. Overwrites any existing value, ignores TTL.
 * Use Redis.Expire(key, seconds) afterwards to apply a TTL. */
static inline code_bool Redis_Set(AmalgameRedis* r, code_string key, code_string value) {
    const char* args[3];
    args[0] = "SET";
    args[1] = key   ? key   : "";
    args[2] = value ? value : "";
    return _amredis_exec_simple(r, 3, args);
}

/* GET key → $<len>\r\n<value>\r\n (or $-1\r\n for missing key).
 * Returns the value, or "" both when the key is missing and on
 * error. Use Redis.Exists(key) to disambiguate, or check
 * Redis.LastError() on the latter. */
static inline code_string Redis_Get(AmalgameRedis* r, code_string key) {
    if (!r || r->fd < 0) return "";
    const char* args[2];
    args[0] = "GET";
    args[1] = key ? key : "";
    size_t cmd_n = 0;
    char*  cmd   = _amredis_build_cmd(2, args, &cmd_n);
    if (_amredis_send_all(r->fd, cmd, cmd_n) < 0) {
        r->last_error = _amredis_err_dup("send() failed");
        return "";
    }
    _AmRedisReply rep = _amredis_read_reply(r->fd);
    if (rep.kind == '$') return rep.is_nil ? "" : rep.str_val;
    if (rep.kind == '-' || rep.kind == 'x') {
        r->last_error = _amredis_err_dup(
            rep.str_val && *rep.str_val ? rep.str_val : "redis error");
    }
    return "";
}

/* DEL key → :<count>. Number of keys actually removed (0 or 1 for
 * a single-key DEL). */
static inline i64 Redis_Del(AmalgameRedis* r, code_string key) {
    if (!r || r->fd < 0) return 0;
    const char* args[2];
    args[0] = "DEL";
    args[1] = key ? key : "";
    size_t cmd_n = 0;
    char*  cmd   = _amredis_build_cmd(2, args, &cmd_n);
    if (_amredis_send_all(r->fd, cmd, cmd_n) < 0) {
        r->last_error = _amredis_err_dup("send() failed");
        return 0;
    }
    _AmRedisReply rep = _amredis_read_reply(r->fd);
    if (rep.kind == ':') return rep.int_val;
    if (rep.kind == '-' || rep.kind == 'x') {
        r->last_error = _amredis_err_dup(
            rep.str_val && *rep.str_val ? rep.str_val : "redis error");
    }
    return 0;
}

/* EXISTS key → :1 if present, :0 otherwise. */
static inline code_bool Redis_Exists(AmalgameRedis* r, code_string key) {
    if (!r || r->fd < 0) return 0;
    const char* args[2];
    args[0] = "EXISTS";
    args[1] = key ? key : "";
    size_t cmd_n = 0;
    char*  cmd   = _amredis_build_cmd(2, args, &cmd_n);
    if (_amredis_send_all(r->fd, cmd, cmd_n) < 0) {
        r->last_error = _amredis_err_dup("send() failed");
        return 0;
    }
    _AmRedisReply rep = _amredis_read_reply(r->fd);
    if (rep.kind == ':') return rep.int_val > 0 ? 1 : 0;
    if (rep.kind == '-' || rep.kind == 'x') {
        r->last_error = _amredis_err_dup(
            rep.str_val && *rep.str_val ? rep.str_val : "redis error");
    }
    return 0;
}

/* INCR key → :<new value>. Creates the key with value 1 if it
 * didn't exist; errors against a non-integer value. */
static inline i64 Redis_Incr(AmalgameRedis* r, code_string key) {
    if (!r || r->fd < 0) return 0;
    const char* args[2];
    args[0] = "INCR";
    args[1] = key ? key : "";
    size_t cmd_n = 0;
    char*  cmd   = _amredis_build_cmd(2, args, &cmd_n);
    if (_amredis_send_all(r->fd, cmd, cmd_n) < 0) {
        r->last_error = _amredis_err_dup("send() failed");
        return 0;
    }
    _AmRedisReply rep = _amredis_read_reply(r->fd);
    if (rep.kind == ':') return rep.int_val;
    if (rep.kind == '-' || rep.kind == 'x') {
        r->last_error = _amredis_err_dup(
            rep.str_val && *rep.str_val ? rep.str_val : "redis error");
    }
    return 0;
}

/* DECR key → :<new value>. Mirror of Redis_Incr. */
static inline i64 Redis_Decr(AmalgameRedis* r, code_string key) {
    if (!r || r->fd < 0) return 0;
    const char* args[2];
    args[0] = "DECR";
    args[1] = key ? key : "";
    size_t cmd_n = 0;
    char*  cmd   = _amredis_build_cmd(2, args, &cmd_n);
    if (_amredis_send_all(r->fd, cmd, cmd_n) < 0) {
        r->last_error = _amredis_err_dup("send() failed");
        return 0;
    }
    _AmRedisReply rep = _amredis_read_reply(r->fd);
    if (rep.kind == ':') return rep.int_val;
    if (rep.kind == '-' || rep.kind == 'x') {
        r->last_error = _amredis_err_dup(
            rep.str_val && *rep.str_val ? rep.str_val : "redis error");
    }
    return 0;
}

/* EXPIRE key seconds → :1 on success (TTL set), :0 if the key
 * doesn't exist. Use PEXPIRE for millisecond resolution (v2). */
static inline code_bool Redis_Expire(AmalgameRedis* r, code_string key, i64 seconds) {
    char secStr[32];
    snprintf(secStr, sizeof(secStr), "%lld", (long long) seconds);
    const char* args[3];
    args[0] = "EXPIRE";
    args[1] = key ? key : "";
    args[2] = secStr;
    return _amredis_exec_simple(r, 3, args);
}

#endif /* AMALGAME_DATABASE_REDIS_H */
