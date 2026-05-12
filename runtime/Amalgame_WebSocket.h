/*
 * Amalgame Standard Library — Amalgame.Net.WebSocket
 * Copyright (c) 2026 Bastien MOUGET
 * https://github.com/amalgame-lang/Amalgame
 *
 * Provides: WebSocket client (RFC 6455), text frames only.
 *
 * MVP scope:
 *   - ws://host:port/path  (plain TCP — wss:// TLS deferred)
 *   - Client → Server framing with mask (REQUIRED by RFC)
 *   - Server → Client framing without mask (parser handles both)
 *   - Text frames (opcode 0x1)
 *   - Close handshake (opcode 0x8)
 *   - Auto-pong reply to Ping (opcode 0x9)
 *
 * Out of scope (deferred to v0.7.4+):
 *   - wss:// TLS via TcpTls or OpenSSL binding
 *   - Binary frames (opcode 0x2)
 *   - Continuation frames (multi-fragment messages)
 *   - per-message-deflate negotiation
 *   - HTTP subprotocols (Sec-WebSocket-Protocol)
 *
 * Test strategy: most call paths are exercised via in-process
 * unit tests on the helper layer (SHA-1 + Base64 + framing). A
 * live-server integration test is optional — skipped silently if
 * no `python3` + `websockets` package is on the host.
 */

#ifndef AMALGAME_WEBSOCKET_H
#define AMALGAME_WEBSOCKET_H

#include "_runtime.h"
#include "Amalgame_Net.h"
#include <string.h>
#include <stdint.h>

/* ─── SHA-1 (FIPS 180-4) — minimal implementation ──── */
/* Public-domain reference; the only WS-specific use is computing
 * the Sec-WebSocket-Accept value from the client key + the magic
 * GUID. Inline here so the WebSocket header is self-contained
 * (Amalgame.Crypto.Sha256 is SHA-256, different algorithm). */

typedef struct {
    uint32_t state[5];
    uint32_t count[2];
    uint8_t  buffer[64];
} _amws_sha1_ctx;

static inline uint32_t _amws_sha1_rotl(uint32_t v, int b) {
    return (v << b) | (v >> (32 - b));
}

static inline void _amws_sha1_init(_amws_sha1_ctx* c) {
    c->state[0] = 0x67452301; c->state[1] = 0xefcdab89; c->state[2] = 0x98badcfe;
    c->state[3] = 0x10325476; c->state[4] = 0xc3d2e1f0;
    c->count[0] = 0; c->count[1] = 0;
}

static inline void _amws_sha1_compress(uint32_t state[5], const uint8_t b[64]) {
    uint32_t w[80];
    int i;
    for (i = 0; i < 16; i++) {
        w[i] = ((uint32_t)b[i*4] << 24) | ((uint32_t)b[i*4+1] << 16)
             | ((uint32_t)b[i*4+2] << 8) | ((uint32_t)b[i*4+3]);
    }
    for (i = 16; i < 80; i++) {
        w[i] = _amws_sha1_rotl(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1);
    }
    uint32_t a = state[0], bv = state[1], c = state[2], d = state[3], e = state[4];
    for (i = 0; i < 80; i++) {
        uint32_t f, k;
        if (i < 20)      { f = (bv & c) | ((~bv) & d);          k = 0x5a827999; }
        else if (i < 40) { f = bv ^ c ^ d;                       k = 0x6ed9eba1; }
        else if (i < 60) { f = (bv & c) | (bv & d) | (c & d);    k = 0x8f1bbcdc; }
        else             { f = bv ^ c ^ d;                       k = 0xca62c1d6; }
        uint32_t t = _amws_sha1_rotl(a, 5) + f + e + k + w[i];
        e = d; d = c; c = _amws_sha1_rotl(bv, 30); bv = a; a = t;
    }
    state[0] += a; state[1] += bv; state[2] += c; state[3] += d; state[4] += e;
}

static inline void _amws_sha1_update(_amws_sha1_ctx* ctx, const uint8_t* data, size_t len) {
    size_t i = (ctx->count[0] >> 3) & 63;
    if ((ctx->count[0] += (uint32_t)(len << 3)) < (uint32_t)(len << 3)) { ctx->count[1]++; }
    ctx->count[1] += (uint32_t)(len >> 29);
    size_t j = 64 - i;
    size_t k = 0;
    if (len >= j) {
        memcpy(&ctx->buffer[i], data, j);
        _amws_sha1_compress(ctx->state, ctx->buffer);
        for (k = j; k + 63 < len; k += 64) { _amws_sha1_compress(ctx->state, &data[k]); }
        i = 0;
    }
    memcpy(&ctx->buffer[i], &data[k], len - k);
}

static inline void _amws_sha1_final(_amws_sha1_ctx* ctx, uint8_t digest[20]) {
    uint8_t finalcount[8];
    int i;
    for (i = 0; i < 8; i++) {
        finalcount[i] = (uint8_t)((ctx->count[(i >= 4 ? 0 : 1)] >> ((3 - (i & 3)) * 8)) & 255);
    }
    uint8_t c = 0x80;
    _amws_sha1_update(ctx, &c, 1);
    while ((ctx->count[0] & 504) != 448) {
        c = 0;
        _amws_sha1_update(ctx, &c, 1);
    }
    _amws_sha1_update(ctx, finalcount, 8);
    for (i = 0; i < 20; i++) {
        digest[i] = (uint8_t)((ctx->state[i >> 2] >> ((3 - (i & 3)) * 8)) & 255);
    }
}

/* ─── Base64 (RFC 4648, no padding control) ────────── */

static const char _amws_b64_chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static inline code_string _amws_base64_encode(const uint8_t* data, size_t len) {
    size_t out_len = 4 * ((len + 2) / 3);
    char* out = (char*) GC_MALLOC(out_len + 1);
    size_t i, j;
    for (i = 0, j = 0; i < len;) {
        uint32_t a = i < len ? data[i++] : 0;
        uint32_t b = i < len ? data[i++] : 0;
        uint32_t c = i < len ? data[i++] : 0;
        uint32_t tri = (a << 16) + (b << 8) + c;
        out[j++] = _amws_b64_chars[(tri >> 18) & 63];
        out[j++] = _amws_b64_chars[(tri >> 12) & 63];
        out[j++] = _amws_b64_chars[(tri >>  6) & 63];
        out[j++] = _amws_b64_chars[ tri        & 63];
    }
    /* Pad with `=` to drop the trailing zero-bytes the loop produced. */
    size_t pad = (3 - (len % 3)) % 3;
    for (i = 0; i < pad; i++) { out[out_len - 1 - i] = '='; }
    out[out_len] = '\0';
    return out;
}

/* Public: SHA-1 → 20 raw bytes, then base64 — what WS computes for
 * Sec-WebSocket-Accept. Exposed so users can verify the handshake
 * value independently if needed (and tests have a hook). */
static inline code_string WebSocket_AcceptKey(code_string clientKey) {
    if (!clientKey) { return code_strdup(""); }
    static const char GUID[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    size_t cklen = strlen(clientKey);
    size_t glen  = strlen(GUID);
    size_t total = cklen + glen;
    uint8_t* concat = (uint8_t*) GC_MALLOC(total);
    memcpy(concat, clientKey, cklen);
    memcpy(concat + cklen, GUID, glen);
    _amws_sha1_ctx ctx;
    _amws_sha1_init(&ctx);
    _amws_sha1_update(&ctx, concat, total);
    uint8_t digest[20];
    _amws_sha1_final(&ctx, digest);
    return _amws_base64_encode(digest, 20);
}

/* ─── WebSocket client struct + ops ────────────────── */

typedef struct AmalgameWebSocket {
    int       _fd;           /* TCP socket fd; -1 when closed */
    code_bool Connected;
    code_string RemoteHost;
    i64         RemotePort;
    code_string Path;
} AmalgameWebSocket;

/* Internal: blocking send-all. Returns 1 on success, 0 on error. */
static inline int _amws_send_all(int fd, const uint8_t* buf, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = send(fd, (const char*)(buf + sent), (int)(len - sent), 0);
        if (n <= 0) { return 0; }
        sent += (size_t) n;
    }
    return 1;
}

/* Internal: blocking recv-all. Reads exactly `len` bytes or fails. */
static inline int _amws_recv_all(int fd, uint8_t* buf, size_t len) {
    size_t got = 0;
    while (got < len) {
        ssize_t n = recv(fd, (char*)(buf + got), (int)(len - got), 0);
        if (n <= 0) { return 0; }
        got += (size_t) n;
    }
    return 1;
}

/* Internal: read one HTTP-style line ending in CRLF. Caps at 4 KiB
 * (enough for any standards-compliant header line). Returns the line
 * without trailing CRLF, or NULL on EOF / error. */
static inline code_string _amws_recv_line(int fd) {
    char* buf = (char*) GC_MALLOC(4096);
    size_t pos = 0;
    while (pos + 1 < 4096) {
        char c;
        ssize_t n = recv(fd, &c, 1, 0);
        if (n <= 0) { return NULL; }
        buf[pos++] = c;
        if (pos >= 2 && buf[pos-2] == '\r' && buf[pos-1] == '\n') {
            buf[pos - 2] = '\0';
            return buf;
        }
    }
    return NULL;
}

/* Internal: generate the 16 random bytes for the client key, base64-
 * encode them. Uses libc rand() seeded once with time(NULL); the
 * key value isn't cryptographically sensitive (it's just nonce
 * material the server echoes back). */
static inline code_string _amws_make_key(void) {
    static int seeded = 0;
    if (!seeded) { srand((unsigned)time(NULL)); seeded = 1; }
    uint8_t k[16];
    int i;
    for (i = 0; i < 16; i++) { k[i] = (uint8_t)(rand() & 0xff); }
    return _amws_base64_encode(k, 16);
}

/* Connect: TCP + HTTP upgrade + accept verification. Returns NULL on
 * any failure (bad host, refused, HTTP non-101, missing /
 * mismatched Sec-WebSocket-Accept). On success returns a connected
 * AmalgameWebSocket* ready for SendText / ReceiveText. */
static inline AmalgameWebSocket* WebSocket_Connect(
        code_string host, i64 port, code_string path) {
    if (!host) { return NULL; }
    AmalgameWebSocket* ws = (AmalgameWebSocket*) GC_MALLOC(sizeof(AmalgameWebSocket));
    ws->RemoteHost = host;
    ws->RemotePort = port;
    ws->Path       = path ? path : "/";
    ws->Connected  = 0;
    ws->_fd        = -1;

    AmalgameTcpClient* tc = TcpClient_Connect(host, port);
    if (!tc || !tc->Connected) { return NULL; }
    ws->_fd = tc->_fd;

    /* Send HTTP upgrade request. */
    code_string key = _amws_make_key();
    char req[1024];
    int rlen = snprintf(req, sizeof(req),
        "GET %s HTTP/1.1\r\n"
        "Host: %s:%lld\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Key: %s\r\n"
        "Sec-WebSocket-Version: 13\r\n"
        "\r\n",
        ws->Path, host, (long long)port, key);
    if (rlen <= 0 || !_amws_send_all(ws->_fd, (const uint8_t*)req, (size_t)rlen)) {
        return NULL;
    }

    /* Read status line. */
    code_string status = _amws_recv_line(ws->_fd);
    if (!status || strstr(status, " 101 ") == NULL) {
        return NULL;
    }
    /* Read headers until empty line. Verify Sec-WebSocket-Accept. */
    code_string expected = WebSocket_AcceptKey(key);
    code_bool acceptOk = 0;
    while (1) {
        code_string line = _amws_recv_line(ws->_fd);
        if (!line) { break; }
        if (strlen(line) == 0) { break; }
        /* Case-insensitive header-name match for `Sec-WebSocket-Accept`. */
        const char* colon = strchr(line, ':');
        if (!colon) { continue; }
        size_t nameLen = (size_t)(colon - line);
        if (nameLen == 20 && strncasecmp(line, "Sec-WebSocket-Accept", 20) == 0) {
            const char* v = colon + 1;
            while (*v == ' ' || *v == '\t') { v++; }
            if (strcmp(v, expected) == 0) { acceptOk = 1; }
        }
    }
    if (!acceptOk) { return NULL; }
    ws->Connected = 1;
    return ws;
}

/* Build a masked client → server frame for `text` (opcode 0x1, FIN
 * set). Returns the wire bytes ready for send(). */
static inline int _amws_send_text_frame(int fd, code_string text) {
    size_t plen = text ? strlen(text) : 0;
    uint8_t hdr[14];
    size_t hi = 0;
    hdr[hi++] = 0x81;                              /* FIN + text opcode */
    if (plen < 126) {
        hdr[hi++] = (uint8_t)(0x80 | plen);
    } else if (plen <= 0xffff) {
        hdr[hi++] = (uint8_t)(0x80 | 126);
        hdr[hi++] = (uint8_t)((plen >> 8) & 0xff);
        hdr[hi++] = (uint8_t)(plen & 0xff);
    } else {
        hdr[hi++] = (uint8_t)(0x80 | 127);
        uint64_t p = plen;
        int i;
        for (i = 7; i >= 0; i--) { hdr[hi++] = (uint8_t)((p >> (i * 8)) & 0xff); }
    }
    uint8_t mask[4];
    int i;
    for (i = 0; i < 4; i++) { mask[i] = (uint8_t)(rand() & 0xff); hdr[hi++] = mask[i]; }
    if (!_amws_send_all(fd, hdr, hi)) { return 0; }
    if (plen == 0) { return 1; }
    uint8_t* masked = (uint8_t*) GC_MALLOC(plen);
    for (i = 0; i < (int)plen; i++) { masked[i] = (uint8_t)(text[i] ^ mask[i & 3]); }
    return _amws_send_all(fd, masked, plen);
}

static inline code_bool WebSocket_SendText(AmalgameWebSocket* ws, code_string text) {
    if (!ws || !ws->Connected || ws->_fd < 0) { return 0; }
    return _amws_send_text_frame(ws->_fd, text) ? 1 : 0;
}

/* Receive one server → client text frame. Returns the payload as a
 * C string (NUL-terminated). On Close (0x8) or read error: marks
 * the socket disconnected and returns NULL. On Ping (0x9): replies
 * with Pong and recurses to read the next frame. Continuation /
 * binary frames are reported as "" so callers can opt to ignore. */
static inline code_string WebSocket_ReceiveText(AmalgameWebSocket* ws) {
    if (!ws || !ws->Connected || ws->_fd < 0) { return NULL; }
    uint8_t hdr2[2];
    if (!_amws_recv_all(ws->_fd, hdr2, 2)) { ws->Connected = 0; return NULL; }
    /* int fin = (hdr2[0] >> 7) & 1; — unused in v1; one-frame-per-message. */
    int opcode = hdr2[0] & 0x0f;
    int masked = (hdr2[1] >> 7) & 1;
    uint64_t plen = hdr2[1] & 0x7f;
    if (plen == 126) {
        uint8_t ext[2];
        if (!_amws_recv_all(ws->_fd, ext, 2)) { ws->Connected = 0; return NULL; }
        plen = ((uint64_t)ext[0] << 8) | ext[1];
    } else if (plen == 127) {
        uint8_t ext[8];
        if (!_amws_recv_all(ws->_fd, ext, 8)) { ws->Connected = 0; return NULL; }
        plen = 0;
        int i;
        for (i = 0; i < 8; i++) { plen = (plen << 8) | ext[i]; }
    }
    uint8_t mask[4] = {0, 0, 0, 0};
    if (masked) {
        if (!_amws_recv_all(ws->_fd, mask, 4)) { ws->Connected = 0; return NULL; }
    }
    /* Cap at 16 MiB so a malicious / buggy server can't OOM us. */
    if (plen > 16 * 1024 * 1024) { ws->Connected = 0; return NULL; }
    uint8_t* payload = NULL;
    if (plen > 0) {
        payload = (uint8_t*) GC_MALLOC((size_t)plen + 1);
        if (!_amws_recv_all(ws->_fd, payload, (size_t)plen)) {
            ws->Connected = 0; return NULL;
        }
        if (masked) {
            uint64_t i;
            for (i = 0; i < plen; i++) { payload[i] ^= mask[i & 3]; }
        }
        payload[plen] = '\0';
    } else {
        payload = (uint8_t*) GC_MALLOC(1);
        payload[0] = '\0';
    }
    if (opcode == 0x1) {                            /* text */
        return (code_string) payload;
    }
    if (opcode == 0x8) {                            /* close */
        ws->Connected = 0;
        return NULL;
    }
    if (opcode == 0x9) {                            /* ping → pong */
        uint8_t pong[14]; size_t hi = 0;
        pong[hi++] = 0x8a;                          /* FIN + pong opcode */
        if (plen < 126) {
            pong[hi++] = (uint8_t)(0x80 | plen);
        } else {
            pong[hi++] = (uint8_t)(0x80 | 126);
            pong[hi++] = (uint8_t)((plen >> 8) & 0xff);
            pong[hi++] = (uint8_t)(plen & 0xff);
        }
        uint8_t mk[4];
        int i;
        for (i = 0; i < 4; i++) { mk[i] = (uint8_t)(rand() & 0xff); pong[hi++] = mk[i]; }
        if (_amws_send_all(ws->_fd, pong, hi)) {
            if (plen > 0) {
                uint8_t* mp = (uint8_t*) GC_MALLOC((size_t)plen);
                uint64_t i2;
                for (i2 = 0; i2 < plen; i2++) { mp[i2] = (uint8_t)(payload[i2] ^ mk[i2 & 3]); }
                _amws_send_all(ws->_fd, mp, (size_t)plen);
            }
        }
        /* Recurse for the next non-control frame. */
        return WebSocket_ReceiveText(ws);
    }
    /* Other opcodes (continuation 0x0, binary 0x2, pong 0xa) — v1
     * skips silently with an empty string so the caller doesn't
     * have to special-case. */
    return code_strdup("");
}

static inline void WebSocket_Close(AmalgameWebSocket* ws) {
    if (!ws || ws->_fd < 0) { return; }
    /* Send Close frame (opcode 0x8) with empty payload + mask. */
    uint8_t frame[6] = { 0x88, 0x80, 0, 0, 0, 0 };
    _amws_send_all(ws->_fd, frame, 6);
    _amnet_close_socket(ws->_fd);
    ws->_fd = -1;
    ws->Connected = 0;
}

static inline code_bool WebSocket_IsConnected(AmalgameWebSocket* ws) {
    return ws && ws->Connected ? 1 : 0;
}

static inline code_string WebSocket_GetHost(AmalgameWebSocket* ws) {
    return ws ? ws->RemoteHost : (code_string)"";
}

static inline i64 WebSocket_GetPort(AmalgameWebSocket* ws) {
    return ws ? ws->RemotePort : 0;
}

#endif /* AMALGAME_WEBSOCKET_H */
