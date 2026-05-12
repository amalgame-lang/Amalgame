/*
 * Amalgame Standard Library — Amalgame.Compress
 * Copyright (c) 2026 Bastien MOUGET
 * https://github.com/amalgame-lang/Amalgame
 *
 * Provides: Compress.Gzip / Compress.Gunzip / Compress.Deflate /
 *           Compress.Inflate — gzip + raw-deflate codec backed by
 *           zlib.
 *
 * Input + output are byte buffers represented as `List<int>`
 * (each entry holds a byte value 0..255). The same convention as
 * `Amalgame.Crypto.Sha256` and `Amalgame.Random.SystemBytes`, so
 * callers can pipe a `File.ReadBytes(...)` straight through
 * `Compress.Gzip` without an intermediate format.
 *
 * gzip vs. deflate:
 *   - `Gzip` / `Gunzip` produce / consume the standard gzip
 *     wrapper (RFC 1952) — same bytes `gzip -c` would write,
 *     correct file-format magic, suitable for HTTP
 *     `Content-Encoding: gzip` or `.gz` files.
 *   - `Deflate` / `Inflate` produce / consume raw-deflate
 *     (RFC 1951) — no header, smaller, suitable for embedded
 *     protocols (WebSocket per-message-deflate, custom binary
 *     RPCs).
 *
 * No error reporting in v1 beyond "returns empty list on
 * failure". Callers that need to distinguish "truncated input"
 * from "bad checksum" should fall back to `system("gunzip")`
 * for now; structured errors land with the Result<T,E> proposal.
 */

#ifndef AMALGAME_COMPRESS_H
#define AMALGAME_COMPRESS_H

#include "_runtime.h"
#include "Amalgame_Collections.h"
#include <zlib.h>
#include <string.h>

/* ─── byte-list ⇄ uint8_t* helpers ────────────────── */

/* Convert a List<int> of byte values into a freshly-GC-allocated
 * `uint8_t*` buffer. `*outLen` gets the byte count. Returns NULL
 * for an empty list (callers must check before reading). */
static inline unsigned char* Compress__ListToBytes(AmalgameList* xs, size_t* outLen) {
    i64 n = AmalgameList_count(xs);
    *outLen = (size_t) n;
    if (n == 0) { return NULL; }
    unsigned char* buf = (unsigned char*) GC_MALLOC((size_t) n);
    for (i64 i = 0; i < n; i++) {
        void* v = AmalgameList_get(xs, i);
        buf[i] = (unsigned char)((intptr_t) v & 0xff);
    }
    return buf;
}

/* Wrap a `uint8_t*` buffer in a freshly-allocated List<int>. */
static inline AmalgameList* Compress__BytesToList(const unsigned char* buf, size_t n) {
    AmalgameList* xs = AmalgameList_new();
    for (size_t i = 0; i < n; i++) {
        i64 b = (i64) buf[i];
        AmalgameList_add(xs, (void*)(intptr_t) b);
    }
    return xs;
}

/* Core deflate-side helper: zlib's `deflate` with the windowBits
 * parameter chosen by the caller — 15 for raw deflate (RFC 1951),
 * 31 (15 + 16) for the gzip wrapper (RFC 1952).
 *
 * Returns a List<int> with the compressed bytes; empty list on
 * any zlib error (the input was probably already compressed or
 * the stream was malformed). */
static inline AmalgameList* Compress__deflate_to_list(AmalgameList* xs, int windowBits) {
    size_t inLen;
    unsigned char* in = Compress__ListToBytes(xs, &inLen);

    z_stream zs;
    memset(&zs, 0, sizeof(zs));
    /* Z_DEFAULT_COMPRESSION = -1 in zlib; the level constants 1..9
     * trade speed for ratio. Stick with default; users that need
     * tuning can fall back to shelling out. */
    if (deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, windowBits,
                     8, Z_DEFAULT_STRATEGY) != Z_OK) {
        return AmalgameList_new();
    }
    zs.next_in  = in;
    zs.avail_in = (uInt) inLen;

    /* Grow the output buffer in 4 KiB chunks. zlib's worst case
     * is roughly input + a header — 4 KiB covers most config-file
     * compresses in one pass. */
    size_t outCap = inLen + 64;
    if (outCap < 4096) { outCap = 4096; }
    unsigned char* out = (unsigned char*) GC_MALLOC(outCap);
    size_t outLen = 0;

    int rc;
    do {
        if (outLen == outCap) {
            size_t newCap = outCap * 2;
            unsigned char* bigger = (unsigned char*) GC_MALLOC(newCap);
            memcpy(bigger, out, outLen);
            out = bigger;
            outCap = newCap;
        }
        zs.next_out  = out + outLen;
        zs.avail_out = (uInt)(outCap - outLen);
        rc = deflate(&zs, Z_FINISH);
        outLen = outCap - zs.avail_out;
        if (rc == Z_STREAM_ERROR) {
            deflateEnd(&zs);
            return AmalgameList_new();
        }
    } while (rc != Z_STREAM_END);

    deflateEnd(&zs);
    return Compress__BytesToList(out, outLen);
}

static inline AmalgameList* Compress__inflate_from_list(AmalgameList* xs, int windowBits) {
    size_t inLen;
    unsigned char* in = Compress__ListToBytes(xs, &inLen);
    if (inLen == 0) { return AmalgameList_new(); }

    z_stream zs;
    memset(&zs, 0, sizeof(zs));
    if (inflateInit2(&zs, windowBits) != Z_OK) {
        return AmalgameList_new();
    }
    zs.next_in  = in;
    zs.avail_in = (uInt) inLen;

    size_t outCap = inLen * 4;
    if (outCap < 4096) { outCap = 4096; }
    unsigned char* out = (unsigned char*) GC_MALLOC(outCap);
    size_t outLen = 0;

    int rc;
    do {
        if (outLen == outCap) {
            size_t newCap = outCap * 2;
            unsigned char* bigger = (unsigned char*) GC_MALLOC(newCap);
            memcpy(bigger, out, outLen);
            out = bigger;
            outCap = newCap;
        }
        zs.next_out  = out + outLen;
        zs.avail_out = (uInt)(outCap - outLen);
        rc = inflate(&zs, Z_NO_FLUSH);
        outLen = outCap - zs.avail_out;
        if (rc < 0) {
            inflateEnd(&zs);
            return AmalgameList_new();
        }
    } while (rc != Z_STREAM_END);

    inflateEnd(&zs);
    return Compress__BytesToList(out, outLen);
}

/* Public surface — four entry points, paired:
 *   - Gzip/Gunzip use windowBits = 15 + 16 (gzip wrapper).
 *   - Deflate/Inflate use windowBits = -15 (raw deflate). */
static inline AmalgameList* Compress_Gzip(AmalgameList* xs)    { return Compress__deflate_to_list(xs, 31); }
static inline AmalgameList* Compress_Gunzip(AmalgameList* xs)  { return Compress__inflate_from_list(xs, 31); }
static inline AmalgameList* Compress_Deflate(AmalgameList* xs) { return Compress__deflate_to_list(xs, -15); }
static inline AmalgameList* Compress_Inflate(AmalgameList* xs) { return Compress__inflate_from_list(xs, -15); }

/* String convenience — converts the UTF-8 bytes of `s`, runs
 * through Gzip / Gunzip, returns the result as bytes (Gzip) or
 * decodes back to string (Gunzip). The decode form assumes the
 * decompressed payload is valid UTF-8; for arbitrary binary use
 * the byte-list pair instead. */
static inline AmalgameList* Compress_GzipString(code_string s) {
    if (!s) { return AmalgameList_new(); }
    AmalgameList* xs = AmalgameList_new();
    for (const char* p = s; *p; p++) {
        AmalgameList_add(xs, (void*)(intptr_t)((unsigned char)*p));
    }
    return Compress_Gzip(xs);
}

static inline code_string Compress_GunzipString(AmalgameList* xs) {
    AmalgameList* bytes = Compress_Gunzip(xs);
    i64 n = AmalgameList_count(bytes);
    char* s = (char*) GC_MALLOC((size_t)(n + 1));
    for (i64 i = 0; i < n; i++) {
        void* v = AmalgameList_get(bytes, i);
        s[i] = (char)((intptr_t) v & 0xff);
    }
    s[n] = '\0';
    return s;
}

#endif /* AMALGAME_COMPRESS_H */
