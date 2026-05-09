/*
 * Amalgame Standard Library — Amalgame.Random
 * Copyright (c) 2026 Bastien MOUGET
 * https://github.com/amalgame-lang/Amalgame
 *
 * Provides: PCG XSH-RR 64/32 step + nanosecond timer + OS entropy.
 *
 * The Amalgame side (src/stdlib/random.am) holds the PRNG state
 * itself and threads it through the two stateless step helpers
 * below. We do the multiply + shifts in C because PCG mixes via
 * deliberate uint64 wrap-around, which is well-defined for unsigned
 * arithmetic but UB for signed — and Amalgame's `int` is i64 signed.
 *
 * The OS entropy source (Random_SystemBytes) and high-resolution
 * timer (Random_TimeSeedNanos) live here too, since both reach for
 * platform-specific syscalls.
 */

#ifndef AMALGAME_RANDOM_H
#define AMALGAME_RANDOM_H

#include "_runtime.h"
#include <stdint.h>
#include <string.h>

#ifdef _WIN32
  #include <windows.h>
  #include <bcrypt.h>
  /* Link with -lbcrypt on Windows (MSYS2/MinGW); see ci.yml. */
#else
  #include <time.h>
  #include <unistd.h>
  #include <fcntl.h>
  #if defined(__APPLE__) || defined(__linux__) || defined(__FreeBSD__) \
      || defined(__OpenBSD__) || defined(__NetBSD__)
    #include <sys/random.h>
  #endif
#endif

/* ─────────────────────────────────────────────
   PCG XSH-RR 64/32 step — stateless
   ─────────────────────────────────────────────
   Reference: https://www.pcg-random.org/
   - state advances by   state = state * MUL + inc
   - output is a permutation of (state >> 18) ^ state
     XOR-shifted by 27, then right-rotated by (state >> 59)
   `inc` must be odd. Amalgame's constructor enforces that.
*/
static inline i64 Random_PcgOutput(i64 state) {
    uint64_t old = (uint64_t) state;
    uint32_t xs  = (uint32_t)(((old >> 18u) ^ old) >> 27u);
    uint32_t rot = (uint32_t)(old >> 59u);
    uint32_t out = (xs >> rot) | (xs << ((32u - rot) & 31u));
    return (i64) out;
}

static inline i64 Random_PcgAdvance(i64 state, i64 inc) {
    uint64_t s = (uint64_t) state;
    uint64_t i = (uint64_t) inc;
    return (i64)(s * 6364136223846793005ULL + i);
}

/* Combine two 32-bit halves (each in [0, 2^32-1] returned by
 * Random_PcgOutput) into a full-range i64. Done in C because the
 * obvious `(hi << 32) | lo` is UB on signed i64 when bit 31 of hi
 * is set. */
static inline i64 Random_CombineHiLo(i64 hi, i64 lo) {
    return (i64)(((uint64_t)(uint32_t) hi << 32u) |
                  (uint64_t)(uint32_t) lo);
}

/* Build the (always-odd) PCG `inc` field from a user seed.
 * Equivalent to `(seed << 1) | 1` but safe when the input has
 * bit 62 or 63 set. */
static inline i64 Random_PrepInc(i64 seed) {
    return (i64)(((uint64_t) seed << 1u) | 1u);
}

/* Read 8 consecutive bytes from `bytes` starting at `offset`,
 * fold them big-endian into an i64. Used by FromSystem to build a
 * full-range seed from 8 bytes of OS entropy. Out-of-range or
 * non-byte values are masked to 8 bits. */
static inline i64 Random_BytesToI64(AmalgameList* bytes, i64 offset) {
    uint64_t r = 0;
    int sz = AmalgameList_count(bytes);
    for (i64 i = 0; i < 8; i++) {
        int idx = (int)(offset + i);
        unsigned int b = 0;
        if (idx >= 0 && idx < sz) {
            b = (unsigned int)(intptr_t) AmalgameList_get(bytes, idx);
        }
        r = (r << 8u) | (uint64_t)(b & 0xFFu);
    }
    return (i64) r;
}

/* ─────────────────────────────────────────────
   Nanosecond timer — for opportunistic seeding only
   ─────────────────────────────────────────────
   Not crypto-grade. Use Random_SystemBytes (below) when
   the seed must be unguessable.
*/
static inline i64 Random_TimeSeedNanos(void) {
#ifdef _WIN32
    LARGE_INTEGER counter, freq;
    QueryPerformanceCounter(&counter);
    QueryPerformanceFrequency(&freq);
    if (freq.QuadPart == 0) return (i64) GetTickCount64();
    /* Convert ticks → ns. Computed in the order
       (counter * 1e9) / freq to keep precision. */
    return (i64)((counter.QuadPart * 1000000000LL) / freq.QuadPart);
#else
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) != 0) return 0;
    return (i64)((int64_t) ts.tv_sec * 1000000000LL + (int64_t) ts.tv_nsec);
#endif
}

/* ─────────────────────────────────────────────
   OS entropy source — crypto-grade
   ─────────────────────────────────────────────
   Returns a freshly-allocated AmalgameList<int> of length n,
   each entry in [0, 255].
   - POSIX: prefer getentropy() (max 256 bytes per call); fall
     back to reading /dev/urandom if getentropy fails.
   - Windows: BCryptGenRandom with the system-preferred RNG.

   On hard failure (rare: chrooted no-/dev/urandom POSIX, or no
   crypto provider on Windows), zeroes the buffer rather than
   silently returning a partial fill. Callers that need to
   detect failure should use a fallback strategy at a higher
   level — this primitive optimises for "always returns
   something".
*/
static inline AmalgameList* Random_SystemBytes(i64 n) {
    int cap = (int)(n > 0 ? n : 1);
    AmalgameList* out = AmalgameList_newWithCapacity(cap);
    if (n <= 0) return out;

    unsigned char* buf = (unsigned char*) GC_MALLOC_ATOMIC((size_t) n);
    int ok = 0;

#ifdef _WIN32
    NTSTATUS st = BCryptGenRandom(
        NULL, buf, (ULONG) n, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (st == 0) ok = 1;
#else
    size_t got = 0;
    while (got < (size_t) n) {
        size_t chunk = (size_t) n - got;
        if (chunk > 256) chunk = 256;
        if (getentropy(buf + got, chunk) == 0) {
            got += chunk;
        } else {
            break;
        }
    }
    if (got == (size_t) n) ok = 1;

    if (!ok) {
        int fd = open("/dev/urandom", O_RDONLY);
        if (fd >= 0) {
            size_t got2 = 0;
            while (got2 < (size_t) n) {
                ssize_t r = read(fd, buf + got2, (size_t) n - got2);
                if (r <= 0) break;
                got2 += (size_t) r;
            }
            close(fd);
            if (got2 == (size_t) n) ok = 1;
        }
    }
#endif

    if (!ok) memset(buf, 0, (size_t) n);

    for (i64 i = 0; i < n; i++) {
        AmalgameList_add(out, (void*)(intptr_t)(unsigned int) buf[i]);
    }
    return out;
}

#endif /* AMALGAME_RANDOM_H */
