/*
 * Amalgame Standard Library — Amalgame.Crypto
 * Copyright (c) 2026 Bastien MOUGET
 * https://github.com/amalgame-lang/Amalgame
 *
 * Provides: SHA-256 (FIPS 180-4) and HMAC-SHA-256 (RFC 2104).
 *
 * Pure C, no external dependencies. The SHA-256 core is a straight
 * implementation of FIPS 180-4 §6.2; HMAC follows RFC 2104. Inputs
 * are AmalgameList<int> (each entry masked to 8 bits) or NUL-
 * terminated UTF-8 strings.
 *
 * The Amalgame side (src/stdlib/crypto.am) is just a thin facade —
 * Sha256.Bytes / Sha256.Hex / Sha256.OfString and the matching
 * Hmac.Sha256* methods all call straight through to these helpers.
 */

#ifndef AMALGAME_CRYPTO_H
#define AMALGAME_CRYPTO_H

#include "_runtime.h"
#include <stdint.h>
#include <string.h>

/* ─────────────────────────────────────────────
   SHA-256 core (FIPS 180-4 §6.2)
   ─────────────────────────────────────────────
   - Block size: 64 bytes (512 bits)
   - Output:     32 bytes (256 bits)
   - State:      8 × 32-bit words (H0..H7)
*/

typedef struct {
    uint32_t H[8];           /* running hash state */
    uint64_t bits;           /* total message length in bits */
    uint8_t  buf[64];        /* partial block buffer */
    size_t   buf_len;        /* bytes currently in buf */
} Crypto_Sha256Ctx;

static const uint32_t Crypto_Sha256_K[64] = {
    0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u,
    0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
    0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u,
    0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
    0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu,
    0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
    0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u,
    0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
    0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u,
    0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
    0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u,
    0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
    0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u,
    0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
    0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u,
    0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u
};

static inline uint32_t Crypto_RotR32(uint32_t x, unsigned n) {
    return (x >> n) | (x << ((32u - n) & 31u));
}

static inline void Crypto_Sha256_Init(Crypto_Sha256Ctx* c) {
    c->H[0] = 0x6a09e667u; c->H[1] = 0xbb67ae85u;
    c->H[2] = 0x3c6ef372u; c->H[3] = 0xa54ff53au;
    c->H[4] = 0x510e527fu; c->H[5] = 0x9b05688cu;
    c->H[6] = 0x1f83d9abu; c->H[7] = 0x5be0cd19u;
    c->bits = 0;
    c->buf_len = 0;
}

/* Process one 64-byte block, in place against c->H. */
static inline void Crypto_Sha256_Block(Crypto_Sha256Ctx* c,
                                       const uint8_t* p) {
    uint32_t W[64];
    for (int i = 0; i < 16; i++) {
        W[i] = ((uint32_t)p[4*i+0] << 24) |
               ((uint32_t)p[4*i+1] << 16) |
               ((uint32_t)p[4*i+2] <<  8) |
               ((uint32_t)p[4*i+3]      );
    }
    for (int i = 16; i < 64; i++) {
        uint32_t s0 = Crypto_RotR32(W[i-15], 7) ^
                      Crypto_RotR32(W[i-15], 18) ^ (W[i-15] >> 3);
        uint32_t s1 = Crypto_RotR32(W[i-2], 17) ^
                      Crypto_RotR32(W[i-2], 19) ^ (W[i-2] >> 10);
        W[i] = W[i-16] + s0 + W[i-7] + s1;
    }
    uint32_t a = c->H[0], b = c->H[1], cc = c->H[2], d = c->H[3];
    uint32_t e = c->H[4], f = c->H[5], g  = c->H[6], h = c->H[7];
    for (int i = 0; i < 64; i++) {
        uint32_t S1 = Crypto_RotR32(e, 6) ^ Crypto_RotR32(e, 11)
                                          ^ Crypto_RotR32(e, 25);
        uint32_t ch = (e & f) ^ ((~e) & g);
        uint32_t t1 = h + S1 + ch + Crypto_Sha256_K[i] + W[i];
        uint32_t S0 = Crypto_RotR32(a, 2) ^ Crypto_RotR32(a, 13)
                                          ^ Crypto_RotR32(a, 22);
        uint32_t mj = (a & b) ^ (a & cc) ^ (b & cc);
        uint32_t t2 = S0 + mj;
        h = g; g = f; f = e; e = d + t1;
        d = cc; cc = b; b = a; a = t1 + t2;
    }
    c->H[0] += a; c->H[1] += b; c->H[2] += cc; c->H[3] += d;
    c->H[4] += e; c->H[5] += f; c->H[6] += g;  c->H[7] += h;
}

static inline void Crypto_Sha256_Update(Crypto_Sha256Ctx* c,
                                        const uint8_t* data,
                                        size_t len) {
    c->bits += (uint64_t) len * 8u;
    /* Drain the partial-block buffer first if there's anything in it. */
    if (c->buf_len > 0) {
        size_t fill = 64 - c->buf_len;
        if (fill > len) fill = len;
        memcpy(c->buf + c->buf_len, data, fill);
        c->buf_len += fill;
        data += fill;
        len  -= fill;
        if (c->buf_len == 64) {
            Crypto_Sha256_Block(c, c->buf);
            c->buf_len = 0;
        }
    }
    while (len >= 64) {
        Crypto_Sha256_Block(c, data);
        data += 64;
        len  -= 64;
    }
    if (len > 0) {
        memcpy(c->buf, data, len);
        c->buf_len = len;
    }
}

/* Finalize: pad with 0x80, zero-fill, append 64-bit length, write
 * the 32-byte big-endian digest into out. */
static inline void Crypto_Sha256_Final(Crypto_Sha256Ctx* c,
                                       uint8_t out[32]) {
    uint8_t pad[64];
    pad[0] = 0x80u;
    memset(pad + 1, 0, 63);
    /* The padded message length must be ≡ 56 (mod 64), so we always
     * emit at least one 0x80 byte and possibly fill into a second
     * block before the 8-byte length suffix. */
    size_t pad_len = (c->buf_len < 56) ? (56 - c->buf_len)
                                       : (120 - c->buf_len);
    Crypto_Sha256_Update(c, pad, pad_len);
    /* Subtract the padding bits we just added — `bits` should be the
     * original message length when we encode it. */
    c->bits -= (uint64_t) pad_len * 8u;
    uint8_t lenbe[8];
    uint64_t b = c->bits;
    for (int i = 0; i < 8; i++) lenbe[7 - i] = (uint8_t)(b >> (8 * i));
    Crypto_Sha256_Update(c, lenbe, 8);
    /* Buf must now be empty (we landed on a 64-byte boundary). */
    for (int i = 0; i < 8; i++) {
        uint32_t w = c->H[i];
        out[4*i+0] = (uint8_t)(w >> 24);
        out[4*i+1] = (uint8_t)(w >> 16);
        out[4*i+2] = (uint8_t)(w >>  8);
        out[4*i+3] = (uint8_t)(w      );
    }
}

/* One-shot convenience — copy raw bytes into 32-byte digest. */
static inline void Crypto_Sha256_Raw(const uint8_t* data, size_t len,
                                     uint8_t out[32]) {
    Crypto_Sha256Ctx c;
    Crypto_Sha256_Init(&c);
    Crypto_Sha256_Update(&c, data, len);
    Crypto_Sha256_Final(&c, out);
}

/* ─────────────────────────────────────────────
   HMAC-SHA-256 (RFC 2104)
   ─────────────────────────────────────────────
   - block size = 64 (matches SHA-256 input block)
   - if key > 64 bytes, replace with SHA-256(key)
   - inner = SHA-256( (key^ipad) || msg )
   - outer = SHA-256( (key^opad) || inner )
*/
static inline void Crypto_HmacSha256_Raw(const uint8_t* key, size_t klen,
                                         const uint8_t* msg, size_t mlen,
                                         uint8_t out[32]) {
    uint8_t k[64];
    if (klen > 64) {
        Crypto_Sha256_Raw(key, klen, k);
        memset(k + 32, 0, 32);
    } else {
        memcpy(k, key, klen);
        if (klen < 64) memset(k + klen, 0, 64 - klen);
    }
    uint8_t ki[64], ko[64];
    for (int i = 0; i < 64; i++) {
        ki[i] = k[i] ^ 0x36u;
        ko[i] = k[i] ^ 0x5cu;
    }
    uint8_t inner[32];
    Crypto_Sha256Ctx c;
    Crypto_Sha256_Init(&c);
    Crypto_Sha256_Update(&c, ki, 64);
    Crypto_Sha256_Update(&c, msg, mlen);
    Crypto_Sha256_Final(&c, inner);

    Crypto_Sha256_Init(&c);
    Crypto_Sha256_Update(&c, ko, 64);
    Crypto_Sha256_Update(&c, inner, 32);
    Crypto_Sha256_Final(&c, out);
}

/* ─────────────────────────────────────────────
   AmalgameList ↔ raw-bytes glue + hex
   ─────────────────────────────────────────────
*/

/* Copy AmalgameList<int> entries into a freshly-allocated byte
 * buffer, masked to 8 bits. Caller owns nothing — buffer is GC. */
static inline uint8_t* Crypto_ListToBytes(AmalgameList* l, size_t* out_len) {
    int n = AmalgameList_count(l);
    if (n < 0) n = 0;
    *out_len = (size_t) n;
    uint8_t* buf = (uint8_t*) GC_MALLOC_ATOMIC((size_t)(n > 0 ? n : 1));
    for (int i = 0; i < n; i++) {
        unsigned int b = (unsigned int)(intptr_t) AmalgameList_get(l, i);
        buf[i] = (uint8_t)(b & 0xFFu);
    }
    return buf;
}

/* 32-byte digest → AmalgameList<int> with each entry in [0, 255]. */
static inline AmalgameList* Crypto_BytesToList(const uint8_t* data,
                                                size_t len) {
    AmalgameList* out = AmalgameList_newWithCapacity((int) len);
    for (size_t i = 0; i < len; i++) {
        AmalgameList_add(out, (void*)(intptr_t)(unsigned int) data[i]);
    }
    return out;
}

/* Lowercase hex of `len` bytes. Returns a freshly-allocated NUL-
 * terminated string of length 2*len. */
static inline code_string Crypto_BytesToHex(const uint8_t* data, size_t len) {
    static const char hexd[] = "0123456789abcdef";
    char* buf = (char*) GC_MALLOC(2 * len + 1);
    for (size_t i = 0; i < len; i++) {
        buf[2*i+0] = hexd[(data[i] >> 4) & 0x0Fu];
        buf[2*i+1] = hexd[ data[i]       & 0x0Fu];
    }
    buf[2*len] = 0;
    return buf;
}

/* ─────────────────────────────────────────────
   Public API — these are the symbols Amalgame's
   resolver registers as global builtins.
   ─────────────────────────────────────────────
*/

static inline AmalgameList* Crypto_Sha256(AmalgameList* bytes) {
    size_t len; uint8_t* buf = Crypto_ListToBytes(bytes, &len);
    uint8_t out[32];
    Crypto_Sha256_Raw(buf, len, out);
    return Crypto_BytesToList(out, 32);
}

static inline code_string Crypto_Sha256Hex(AmalgameList* bytes) {
    size_t len; uint8_t* buf = Crypto_ListToBytes(bytes, &len);
    uint8_t out[32];
    Crypto_Sha256_Raw(buf, len, out);
    return Crypto_BytesToHex(out, 32);
}

static inline code_string Crypto_Sha256OfString(code_string s) {
    if (!s) s = "";
    uint8_t out[32];
    Crypto_Sha256_Raw((const uint8_t*) s, strlen(s), out);
    return Crypto_BytesToHex(out, 32);
}

static inline AmalgameList* Crypto_HmacSha256(AmalgameList* key,
                                              AmalgameList* msg) {
    size_t klen; uint8_t* kbuf = Crypto_ListToBytes(key, &klen);
    size_t mlen; uint8_t* mbuf = Crypto_ListToBytes(msg, &mlen);
    uint8_t out[32];
    Crypto_HmacSha256_Raw(kbuf, klen, mbuf, mlen, out);
    return Crypto_BytesToList(out, 32);
}

static inline code_string Crypto_HmacSha256Hex(AmalgameList* key,
                                               AmalgameList* msg) {
    size_t klen; uint8_t* kbuf = Crypto_ListToBytes(key, &klen);
    size_t mlen; uint8_t* mbuf = Crypto_ListToBytes(msg, &mlen);
    uint8_t out[32];
    Crypto_HmacSha256_Raw(kbuf, klen, mbuf, mlen, out);
    return Crypto_BytesToHex(out, 32);
}

static inline code_string Crypto_HmacSha256OfStrings(code_string key,
                                                     code_string msg) {
    if (!key) key = "";
    if (!msg) msg = "";
    uint8_t out[32];
    Crypto_HmacSha256_Raw((const uint8_t*) key, strlen(key),
                           (const uint8_t*) msg, strlen(msg), out);
    return Crypto_BytesToHex(out, 32);
}

#endif /* AMALGAME_CRYPTO_H */
