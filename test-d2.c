#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "_runtime.h"
#include "Amalgame_String.h"
#include "Amalgame_Collections.h"
#include "Amalgame_IO.h"
#include "Amalgame_Net.h"
#include "Amalgame_Console.h"
#include "Amalgame_Process.h"
#include "/home/neitsab/.amalgame/packages/github.com/amalgame-lang/amalgame-math/v0.1.0_d8fdcca4/facade-stub.h"
#include "/home/neitsab/.amalgame/packages/github.com/amalgame-lang/amalgame-math-vec/v0.1.0_319511eb/facade-stub.h"

typedef struct _Amalgame_Math Amalgame_Math; /* pkg-class */
typedef struct _Amalgame_Math_Vec_Vec3 Amalgame_Math_Vec_Vec3; /* pkg-class */
/* inline-C top-level */

    static i64 _opaque_consume(i64 x) {
        return x + 10;
    }

typedef struct _Demo_Bridge Demo_Bridge;
typedef struct _Demo_Program Demo_Program;
typedef struct _Amalgame_Math_Math Amalgame_Math_Math; /* external */
typedef struct _Amalgame_Math_Vec_Vec3 Amalgame_Math_Vec_Vec3; /* external */
typedef struct _Amalgame_Math_Vec_Vec4 Amalgame_Math_Vec_Vec4; /* external */
typedef struct _Amalgame_Math_Vec_Mat4 Amalgame_Math_Vec_Mat4; /* external */
struct _Amalgame_Math_Math {
};
double Amalgame_Math_Math_PI();
double Amalgame_Math_Math_E();
double Amalgame_Math_Math_TAU();
double Amalgame_Math_Math_SQRT2();
double Amalgame_Math_Math_LN2();
double Amalgame_Math_Math_LN10();
double Amalgame_Math_Math_INF();
double Amalgame_Math_Math_Abs(double x);
double Amalgame_Math_Math_Sqrt(double x);
double Amalgame_Math_Math_Cbrt(double x);
double Amalgame_Math_Math_Pow(double base, double exp);
double Amalgame_Math_Math_Exp(double x);
double Amalgame_Math_Math_Log(double x);
double Amalgame_Math_Math_Log2(double x);
double Amalgame_Math_Math_Log10(double x);
double Amalgame_Math_Math_Floor(double x);
double Amalgame_Math_Math_Ceil(double x);
double Amalgame_Math_Math_Round(double x);
double Amalgame_Math_Math_Trunc(double x);
double Amalgame_Math_Math_MaxF(double a, double b);
double Amalgame_Math_Math_MinF(double a, double b);
i64 Amalgame_Math_Math_MaxI(i64 a, i64 b);
i64 Amalgame_Math_Math_MinI(i64 a, i64 b);
double Amalgame_Math_Math_ClampF(double v, double lo, double hi);
i64 Amalgame_Math_Math_ClampI(i64 v, i64 lo, i64 hi);
i64 Amalgame_Math_Math_Sign(double x);
double Amalgame_Math_Math_CopySign(double mag, double sign);
double Amalgame_Math_Math_Sin(double x);
double Amalgame_Math_Math_Cos(double x);
double Amalgame_Math_Math_Tan(double x);
double Amalgame_Math_Math_Asin(double x);
double Amalgame_Math_Math_Acos(double x);
double Amalgame_Math_Math_Atan(double x);
double Amalgame_Math_Math_Atan2(double y, double x);
double Amalgame_Math_Math_Sinh(double x);
double Amalgame_Math_Math_Cosh(double x);
double Amalgame_Math_Math_Tanh(double x);
double Amalgame_Math_Math_ToRadians(double deg);
double Amalgame_Math_Math_ToDegrees(double rad);
i64 Amalgame_Math_Math_AbsI(i64 x);
i64 Amalgame_Math_Math_PowI(i64 base, i64 e);
i64 Amalgame_Math_Math_Gcd(i64 a, i64 b);
i64 Amalgame_Math_Math_Lcm(i64 a, i64 b);
code_bool Amalgame_Math_Math_IsPrime(i64 n);
code_bool Amalgame_Math_Math_IsNaN(double x);
code_bool Amalgame_Math_Math_IsInf(double x);
code_bool Amalgame_Math_Math_IsFinite(double x);
code_bool Amalgame_Math_Math_ApproxEq(double a, double b, double eps);
struct _Amalgame_Math_Vec_Vec3 {
    double X;
    double Y;
    double Z;
};
struct _Amalgame_Math_Vec_Vec4 {
    double X;
    double Y;
    double Z;
    double W;
};
struct _Amalgame_Math_Vec_Mat4 {
    double M00;
    double M01;
    double M02;
    double M03;
    double M10;
    double M11;
    double M12;
    double M13;
    double M20;
    double M21;
    double M22;
    double M23;
    double M30;
    double M31;
    double M32;
    double M33;
};
Amalgame_Math_Vec_Vec3* Amalgame_Math_Vec_Vec3_new(double x, double y, double z);
double Amalgame_Math_Vec_Vec3_GetX(Amalgame_Math_Vec_Vec3* self);
double Amalgame_Math_Vec_Vec3_GetY(Amalgame_Math_Vec_Vec3* self);
double Amalgame_Math_Vec_Vec3_GetZ(Amalgame_Math_Vec_Vec3* self);
Amalgame_Math_Vec_Vec3* Amalgame_Math_Vec_Vec3_Add(Amalgame_Math_Vec_Vec3* self, Amalgame_Math_Vec_Vec3* other);
Amalgame_Math_Vec_Vec3* Amalgame_Math_Vec_Vec3_Sub(Amalgame_Math_Vec_Vec3* self, Amalgame_Math_Vec_Vec3* other);
Amalgame_Math_Vec_Vec3* Amalgame_Math_Vec_Vec3_Scale(Amalgame_Math_Vec_Vec3* self, double k);
double Amalgame_Math_Vec_Vec3_Dot(Amalgame_Math_Vec_Vec3* self, Amalgame_Math_Vec_Vec3* other);
Amalgame_Math_Vec_Vec3* Amalgame_Math_Vec_Vec3_Cross(Amalgame_Math_Vec_Vec3* self, Amalgame_Math_Vec_Vec3* other);
double Amalgame_Math_Vec_Vec3_Length(Amalgame_Math_Vec_Vec3* self);
Amalgame_Math_Vec_Vec3* Amalgame_Math_Vec_Vec3_Normalize(Amalgame_Math_Vec_Vec3* self);
code_bool Amalgame_Math_Vec_Vec3_Equals(Amalgame_Math_Vec_Vec3* self, Amalgame_Math_Vec_Vec3* other);
Amalgame_Math_Vec_Vec4* Amalgame_Math_Vec_Vec4_new(double x, double y, double z, double w);
double Amalgame_Math_Vec_Vec4_GetX(Amalgame_Math_Vec_Vec4* self);
double Amalgame_Math_Vec_Vec4_GetY(Amalgame_Math_Vec_Vec4* self);
double Amalgame_Math_Vec_Vec4_GetZ(Amalgame_Math_Vec_Vec4* self);
double Amalgame_Math_Vec_Vec4_GetW(Amalgame_Math_Vec_Vec4* self);
Amalgame_Math_Vec_Vec4* Amalgame_Math_Vec_Vec4_Add(Amalgame_Math_Vec_Vec4* self, Amalgame_Math_Vec_Vec4* other);
Amalgame_Math_Vec_Vec4* Amalgame_Math_Vec_Vec4_Sub(Amalgame_Math_Vec_Vec4* self, Amalgame_Math_Vec_Vec4* other);
Amalgame_Math_Vec_Vec4* Amalgame_Math_Vec_Vec4_Scale(Amalgame_Math_Vec_Vec4* self, double k);
double Amalgame_Math_Vec_Vec4_Dot(Amalgame_Math_Vec_Vec4* self, Amalgame_Math_Vec_Vec4* other);
Amalgame_Math_Vec_Mat4* Amalgame_Math_Vec_Mat4_new();
Amalgame_Math_Vec_Mat4* Amalgame_Math_Vec_Mat4_Identity();
Amalgame_Math_Vec_Mat4* Amalgame_Math_Vec_Mat4_Translate(double tx, double ty, double tz);
Amalgame_Math_Vec_Mat4* Amalgame_Math_Vec_Mat4_Scale(double sx, double sy, double sz);
Amalgame_Math_Vec_Mat4* Amalgame_Math_Vec_Mat4_RotateX(double angleRad);
Amalgame_Math_Vec_Mat4* Amalgame_Math_Vec_Mat4_RotateY(double angleRad);
Amalgame_Math_Vec_Mat4* Amalgame_Math_Vec_Mat4_RotateZ(double angleRad);
double Amalgame_Math_Vec_Mat4_Get(Amalgame_Math_Vec_Mat4* self, i64 col, i64 row);
void Amalgame_Math_Vec_Mat4_Set(Amalgame_Math_Vec_Mat4* self, i64 col, i64 row, double v);
Amalgame_Math_Vec_Mat4* Amalgame_Math_Vec_Mat4_Multiply(Amalgame_Math_Vec_Mat4* self, Amalgame_Math_Vec_Mat4* other);
Amalgame_Math_Vec_Vec4* Amalgame_Math_Vec_Mat4_TransformVec4(Amalgame_Math_Vec_Mat4* self, Amalgame_Math_Vec_Vec4* v);

Demo_Bridge* Demo_Bridge_new();
Demo_Program* Demo_Program_new();
i64 Demo_Bridge_Consume(i64 handle);
i64 Demo_Program_Main(code_string* args);
typedef struct LamEnv_0 {
    char _empty;
} LamEnv_0;
static void* lam_0_fn(void* __envRaw, void* __arg0);
struct _Demo_Bridge {
};

i64 Demo_Bridge_Consume(i64 handle);

Demo_Bridge* Demo_Bridge_new() {
    Demo_Bridge* self = (Demo_Bridge*) GC_MALLOC(sizeof(Demo_Bridge));
    return self;
}

i64 Demo_Bridge_Consume(i64 handle) {
    #line 11 "/tmp/test-d2.am"
    i64 r = 0;
    #line 12 "/tmp/test-d2.am"
    { /* inline-C */
         r = _opaque_consume(handle); 
    }
    #line 13 "/tmp/test-d2.am"
    return r;
}

struct _Demo_Program {
};

i64 Demo_Program_Main(code_string* args);

Demo_Program* Demo_Program_new() {
    Demo_Program* self = (Demo_Program*) GC_MALLOC(sizeof(Demo_Program));
    return self;
}

i64 Demo_Program_Main(code_string* args) {
    #line 21 "/tmp/test-d2.am"
    LamEnv_0* __env_0 = (LamEnv_0*) code_alloc(sizeof(LamEnv_0));
    AmalgameClosure* cb = AmalgameClosure_new((void*)lam_0_fn, __env_0);
    #line 26 "/tmp/test-d2.am"
    i64 r = (i64)(intptr_t)AmalgameClosure_call1(cb, (void*)(intptr_t)(32));
    #line 27 "/tmp/test-d2.am"
    Console_WriteLine(code_string_concat("got=", String_FromInt(r)));
    #line 28 "/tmp/test-d2.am"
    return 0;
}

static void* lam_0_fn(void* __envRaw, void* __arg0) {
    LamEnv_0* __env = (LamEnv_0*)__envRaw;
    i64 conn = (i64)(intptr_t)__arg0;
    #line 24 "/tmp/test-d2.am"
    return (void*)(intptr_t)(Demo_Bridge_Consume(conn));
    return (void*)(intptr_t)(0);
}

int main(int argc, char** argv) {
    GC_INIT();
    code_runtime_init_args(argc, argv);
    Demo_Program_Main((code_string*)argv);
    return code_exit_code;
}
