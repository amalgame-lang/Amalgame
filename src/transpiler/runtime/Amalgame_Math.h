/*
 * Amalgame Standard Library — Amalgame.Math
 * Copyright (c) 2026 Bastien MOUGET
 * https://github.com/BastienMOUGET/Amalgame
 *
 * Provides: Math constants and functions
 */

#ifndef AMALGAME_MATH_H
#define AMALGAME_MATH_H

#include "_runtime.h"
#include <math.h>

/* ─────────────────────────────────────────────
   Constants
   ───────────────────────────────────────────── */

#define Amalgame_Math_PI      3.14159265358979323846
#define Amalgame_Math_E       2.71828182845904523536
#define Amalgame_Math_TAU     6.28318530717958647692
#define Amalgame_Math_SQRT2   1.41421356237309504880
#define Amalgame_Math_LN2     0.69314718055994530942
#define Amalgame_Math_LN10    2.30258509299404568402
#define Amalgame_Math_INF     (1.0 / 0.0)

/* ─────────────────────────────────────────────
   Basic functions
   ───────────────────────────────────────────── */

static inline f64 Math_Abs(f64 x)            { return fabs(x); }
static inline f64 Math_Sqrt(f64 x)           { return sqrt(x); }
static inline f64 Math_Cbrt(f64 x)           { return cbrt(x); }
static inline f64 Math_Pow(f64 base, f64 exp){ return pow(base, exp); }
static inline f64 Math_Exp(f64 x)            { return exp(x); }
static inline f64 Math_Log(f64 x)            { return log(x); }
static inline f64 Math_Log2(f64 x)           { return log2(x); }
static inline f64 Math_Log10(f64 x)          { return log10(x); }

/* ─────────────────────────────────────────────
   Rounding
   ───────────────────────────────────────────── */

static inline f64 Math_Floor(f64 x)  { return floor(x); }
static inline f64 Math_Ceil(f64 x)   { return ceil(x); }
static inline f64 Math_Round(f64 x)  { return round(x); }
static inline f64 Math_Trunc(f64 x)  { return trunc(x); }

/* ─────────────────────────────────────────────
   Min / Max / Clamp
   ───────────────────────────────────────────── */

static inline f64 Math_MaxF(f64 a, f64 b)    { return a > b ? a : b; }
static inline f64 Math_MinF(f64 a, f64 b)    { return a < b ? a : b; }
static inline i64 Math_MaxI(i64 a, i64 b)    { return a > b ? a : b; }
static inline i64 Math_MinI(i64 a, i64 b)    { return a < b ? a : b; }

static inline f64 Math_ClampF(f64 v, f64 lo, f64 hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}
static inline i64 Math_ClampI(i64 v, i64 lo, i64 hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

/* ─────────────────────────────────────────────
   Sign
   ───────────────────────────────────────────── */

static inline i64 Math_Sign(f64 x) {
    if (x > 0) return  1;
    if (x < 0) return -1;
    return 0;
}

static inline f64 Math_CopySign(f64 mag, f64 sign) {
    return copysign(mag, sign);
}

/* ─────────────────────────────────────────────
   Trigonometry
   ───────────────────────────────────────────── */

static inline f64 Math_Sin(f64 x)   { return sin(x); }
static inline f64 Math_Cos(f64 x)   { return cos(x); }
static inline f64 Math_Tan(f64 x)   { return tan(x); }
static inline f64 Math_Asin(f64 x)  { return asin(x); }
static inline f64 Math_Acos(f64 x)  { return acos(x); }
static inline f64 Math_Atan(f64 x)  { return atan(x); }
static inline f64 Math_Atan2(f64 y, f64 x) { return atan2(y, x); }
static inline f64 Math_Sinh(f64 x)  { return sinh(x); }
static inline f64 Math_Cosh(f64 x)  { return cosh(x); }
static inline f64 Math_Tanh(f64 x)  { return tanh(x); }

/* Degrees ↔ Radians */
static inline f64 Math_ToRadians(f64 deg) {
    return deg * Amalgame_Math_PI / 180.0;
}
static inline f64 Math_ToDegrees(f64 rad) {
    return rad * 180.0 / Amalgame_Math_PI;
}

/* ─────────────────────────────────────────────
   Integer math
   ───────────────────────────────────────────── */

static inline i64 Math_AbsI(i64 x)           { return x < 0 ? -x : x; }
static inline i64 Math_PowI(i64 base, i64 e) {
    i64 r = 1;
    for (i64 i = 0; i < e; i++) r *= base;
    return r;
}

static inline i64 Math_Gcd(i64 a, i64 b) {
    while (b) { i64 t = b; b = a % b; a = t; }
    return a < 0 ? -a : a;
}

static inline i64 Math_Lcm(i64 a, i64 b) {
    i64 g = Math_Gcd(a, b);
    return g == 0 ? 0 : Math_AbsI(a / g * b);
}

static inline code_bool Math_IsPrime(i64 n) {
    if (n < 2) return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;
    for (i64 i = 3; i * i <= n; i += 2)
        if (n % i == 0) return false;
    return true;
}

/* ─────────────────────────────────────────────
   Floating-point checks
   ───────────────────────────────────────────── */

static inline code_bool Math_IsNaN(f64 x)   { return isnan(x); }
static inline code_bool Math_IsInf(f64 x)   { return isinf(x); }
static inline code_bool Math_IsFinite(f64 x){ return isfinite(x); }

/* Approximate equality */
static inline code_bool Math_ApproxEq(f64 a, f64 b, f64 eps) {
    f64 d = a - b;
    return (d < 0 ? -d : d) <= eps;
}

/* ─────────────────────────────────────────────
   Random (simple LCG — not cryptographic)
   ───────────────────────────────────────────── */

static i64 _amalgame_rng_state = 12345678901234LL;

static inline void Math_SeedRandom(i64 seed) {
    _amalgame_rng_state = seed;
}

static inline f64 Math_Random() {
    _amalgame_rng_state = _amalgame_rng_state * 6364136223846793005LL
                          + 1442695040888963407LL;
    return (f64)((u8)(_amalgame_rng_state >> 33)) / 255.0;
}

static inline i64 Math_RandomInt(i64 min, i64 max) {
    if (min >= max) return min;
    return min + (i64)(Math_Random() * (f64)(max - min));
}

#endif /* AMALGAME_MATH_H */
