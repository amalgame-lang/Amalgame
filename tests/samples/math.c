/* ═══════════════════════════════════
 * Généré par Amalgame Transpiler v0.3.0
 * Source : ./tests/samples/math.am
 * NE PAS MODIFIER MANUELLEMENT
 * ═══════════════════════════════════
 */

#include "_runtime.h"

/* namespace Tests */

/* ── Forward Declarations ── */
typedef struct _MathHelper MathHelper;
static i64 MathHelper_Add();
static i64 MathHelper_Max();
static i64 MathHelper_Factorial();
typedef struct _Program Program;
static void Program_Main();



#line 3 "./tests/samples/math.am"
/* class MathHelper */
struct _MathHelper {
};

MathHelper* MathHelper_new() {
    MathHelper* self = (MathHelper*) code_alloc(sizeof(MathHelper));
    return self;
}


#line 4 "./tests/samples/math.am"
static i64 MathHelper_Add(i64 a, i64 b) {

#line 5 "./tests/samples/math.am"
    return a + b;
}


#line 8 "./tests/samples/math.am"
static i64 MathHelper_Max(i64 a, i64 b) {

#line 9 "./tests/samples/math.am"
    if (a > b) {

#line 10 "./tests/samples/math.am"
        return a;
    } else {

#line 12 "./tests/samples/math.am"
        return b;
    }
}


#line 16 "./tests/samples/math.am"
static i64 MathHelper_Factorial(i64 n) {

#line 17 "./tests/samples/math.am"
    if (n <= 1) {

#line 18 "./tests/samples/math.am"
        return 1;
    }

#line 20 "./tests/samples/math.am"
    return n * MathHelper_Factorial(n - 1);
}



#line 24 "./tests/samples/math.am"
/* class Program */
struct _Program {
};

Program* Program_new() {
    Program* self = (Program*) code_alloc(sizeof(Program));
    return self;
}


#line 25 "./tests/samples/math.am"
static void Program_Main(int argc, char** argv) {

#line 26 "./tests/samples/math.am"
    i64 sum = MathHelper_Add(21, 21);

#line 27 "./tests/samples/math.am"
    Console_WriteLine(code_string_format("21 + 21 = %s", code_int_to_string(sum)));

#line 29 "./tests/samples/math.am"
    i64 biggest = MathHelper_Max(42, 17);

#line 30 "./tests/samples/math.am"
    Console_WriteLine(code_string_format("max(42, 17) = %s", code_int_to_string(biggest)));

#line 32 "./tests/samples/math.am"
    i64 fact = MathHelper_Factorial(5);

#line 33 "./tests/samples/math.am"
    Console_WriteLine(code_string_format("5! = %s", code_int_to_string(fact)));
}



/* ── Point d entree C ── */
int main(int argc, char** argv) {
    code_runtime_init();
    Program_Main(argc, argv);
    return 0;
}
