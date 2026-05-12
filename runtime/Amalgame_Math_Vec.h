/*
 * Amalgame Standard Library — Amalgame.Math.Vec
 * Copyright (c) 2026 Bastien MOUGET
 * https://github.com/amalgame-lang/Amalgame
 *
 * Provides: Vec3, Vec4, Mat4 — 3D math primitives for game /
 *           graphics use cases. Scalar implementation; no SIMD.
 *           All structures are heap-allocated via GC_MALLOC so
 *           Amalgame classes wrap them as pointer types
 *           (`Vec3*`, `Mat4*`).
 *
 * Conventions:
 *   - Matrices are 4×4 column-major (OpenGL convention).
 *   - Vec4.W is the homogeneous coordinate (1 = position, 0 = direction).
 *   - Rotations are in radians.
 */

#ifndef AMALGAME_MATH_VEC_H
#define AMALGAME_MATH_VEC_H

#include <math.h>
#include "_runtime.h"

/* ─── Vec3 ──────────────────────────────────────── */

typedef struct AmalgameVec3 {
    double X;
    double Y;
    double Z;
} AmalgameVec3;

static inline AmalgameVec3* Vec3_new(double x, double y, double z) {
    AmalgameVec3* v = (AmalgameVec3*)GC_MALLOC(sizeof(AmalgameVec3));
    v->X = x; v->Y = y; v->Z = z;
    return v;
}

static inline double Vec3_GetX(AmalgameVec3* v) { return v->X; }
static inline double Vec3_GetY(AmalgameVec3* v) { return v->Y; }
static inline double Vec3_GetZ(AmalgameVec3* v) { return v->Z; }

static inline AmalgameVec3* Vec3_Add(AmalgameVec3* a, AmalgameVec3* b) {
    return Vec3_new(a->X + b->X, a->Y + b->Y, a->Z + b->Z);
}
static inline AmalgameVec3* Vec3_Sub(AmalgameVec3* a, AmalgameVec3* b) {
    return Vec3_new(a->X - b->X, a->Y - b->Y, a->Z - b->Z);
}
static inline AmalgameVec3* Vec3_Scale(AmalgameVec3* a, double k) {
    return Vec3_new(a->X * k, a->Y * k, a->Z * k);
}
static inline double Vec3_Dot(AmalgameVec3* a, AmalgameVec3* b) {
    return a->X * b->X + a->Y * b->Y + a->Z * b->Z;
}
static inline AmalgameVec3* Vec3_Cross(AmalgameVec3* a, AmalgameVec3* b) {
    return Vec3_new(a->Y * b->Z - a->Z * b->Y,
                    a->Z * b->X - a->X * b->Z,
                    a->X * b->Y - a->Y * b->X);
}
static inline double Vec3_Length(AmalgameVec3* a) {
    return sqrt(a->X * a->X + a->Y * a->Y + a->Z * a->Z);
}
static inline AmalgameVec3* Vec3_Normalize(AmalgameVec3* a) {
    double L = Vec3_Length(a);
    if (L == 0.0) { return Vec3_new(0.0, 0.0, 0.0); }
    return Vec3_new(a->X / L, a->Y / L, a->Z / L);
}
static inline code_bool Vec3_Equals(AmalgameVec3* a, AmalgameVec3* b) {
    return (a->X == b->X) && (a->Y == b->Y) && (a->Z == b->Z);
}

/* ─── Vec4 ──────────────────────────────────────── */

typedef struct AmalgameVec4 {
    double X;
    double Y;
    double Z;
    double W;
} AmalgameVec4;

static inline AmalgameVec4* Vec4_new(double x, double y, double z, double w) {
    AmalgameVec4* v = (AmalgameVec4*)GC_MALLOC(sizeof(AmalgameVec4));
    v->X = x; v->Y = y; v->Z = z; v->W = w;
    return v;
}
static inline double Vec4_GetX(AmalgameVec4* v) { return v->X; }
static inline double Vec4_GetY(AmalgameVec4* v) { return v->Y; }
static inline double Vec4_GetZ(AmalgameVec4* v) { return v->Z; }
static inline double Vec4_GetW(AmalgameVec4* v) { return v->W; }

static inline AmalgameVec4* Vec4_Add(AmalgameVec4* a, AmalgameVec4* b) {
    return Vec4_new(a->X + b->X, a->Y + b->Y, a->Z + b->Z, a->W + b->W);
}
static inline AmalgameVec4* Vec4_Sub(AmalgameVec4* a, AmalgameVec4* b) {
    return Vec4_new(a->X - b->X, a->Y - b->Y, a->Z - b->Z, a->W - b->W);
}
static inline AmalgameVec4* Vec4_Scale(AmalgameVec4* a, double k) {
    return Vec4_new(a->X * k, a->Y * k, a->Z * k, a->W * k);
}
static inline double Vec4_Dot(AmalgameVec4* a, AmalgameVec4* b) {
    return a->X * b->X + a->Y * b->Y + a->Z * b->Z + a->W * b->W;
}

/* ─── Mat4 ──────────────────────────────────────── */
/*
 * Column-major 4×4 matrix. M[col][row]: M[0] is the first column
 * (Xx, Xy, Xz, 0 for an affine transform). This matches OpenGL's
 * fixed-pipeline convention so users can blit straight into a
 * `glUniformMatrix4fv` without transposing.
 */

typedef struct AmalgameMat4 {
    double M[16]; /* column-major: M[col*4 + row] */
} AmalgameMat4;

static inline AmalgameMat4* Mat4_new(void) {
    AmalgameMat4* m = (AmalgameMat4*)GC_MALLOC(sizeof(AmalgameMat4));
    for (int i = 0; i < 16; i++) { m->M[i] = 0.0; }
    return m;
}
static inline AmalgameMat4* Mat4_Identity(void) {
    AmalgameMat4* m = Mat4_new();
    m->M[0] = 1.0; m->M[5] = 1.0; m->M[10] = 1.0; m->M[15] = 1.0;
    return m;
}
static inline double Mat4_Get(AmalgameMat4* m, i64 col, i64 row) {
    return m->M[col * 4 + row];
}
static inline void Mat4_Set(AmalgameMat4* m, i64 col, i64 row, double v) {
    m->M[col * 4 + row] = v;
}
static inline AmalgameMat4* Mat4_Multiply(AmalgameMat4* a, AmalgameMat4* b) {
    AmalgameMat4* r = Mat4_new();
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            double sum = 0.0;
            for (int k = 0; k < 4; k++) {
                sum += a->M[k * 4 + row] * b->M[col * 4 + k];
            }
            r->M[col * 4 + row] = sum;
        }
    }
    return r;
}
static inline AmalgameMat4* Mat4_Translate(double tx, double ty, double tz) {
    AmalgameMat4* m = Mat4_Identity();
    m->M[12] = tx; m->M[13] = ty; m->M[14] = tz;
    return m;
}
static inline AmalgameMat4* Mat4_Scale(double sx, double sy, double sz) {
    AmalgameMat4* m = Mat4_new();
    m->M[0] = sx; m->M[5] = sy; m->M[10] = sz; m->M[15] = 1.0;
    return m;
}
static inline AmalgameMat4* Mat4_RotateX(double angleRad) {
    AmalgameMat4* m = Mat4_Identity();
    double c = cos(angleRad), s = sin(angleRad);
    m->M[5] = c;  m->M[6] = s;
    m->M[9] = -s; m->M[10] = c;
    return m;
}
static inline AmalgameMat4* Mat4_RotateY(double angleRad) {
    AmalgameMat4* m = Mat4_Identity();
    double c = cos(angleRad), s = sin(angleRad);
    m->M[0] = c;  m->M[2] = -s;
    m->M[8] = s;  m->M[10] = c;
    return m;
}
static inline AmalgameMat4* Mat4_RotateZ(double angleRad) {
    AmalgameMat4* m = Mat4_Identity();
    double c = cos(angleRad), s = sin(angleRad);
    m->M[0] = c;  m->M[1] = s;
    m->M[4] = -s; m->M[5] = c;
    return m;
}

/* Multiply a Vec4 by a Mat4 (column-major): v' = M * v. */
static inline AmalgameVec4* Mat4_TransformVec4(AmalgameMat4* m, AmalgameVec4* v) {
    double rx = m->M[0]  * v->X + m->M[4]  * v->Y + m->M[8]  * v->Z + m->M[12] * v->W;
    double ry = m->M[1]  * v->X + m->M[5]  * v->Y + m->M[9]  * v->Z + m->M[13] * v->W;
    double rz = m->M[2]  * v->X + m->M[6]  * v->Y + m->M[10] * v->Z + m->M[14] * v->W;
    double rw = m->M[3]  * v->X + m->M[7]  * v->Y + m->M[11] * v->Z + m->M[15] * v->W;
    return Vec4_new(rx, ry, rz, rw);
}

#endif /* AMALGAME_MATH_VEC_H */
