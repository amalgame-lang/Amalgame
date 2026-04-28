/* ═══════════════════════════════════
 * Généré par Amalgame Transpiler v0.3.0
 * Source : ./tests/samples/closures.am
 * NE PAS MODIFIER MANUELLEMENT
 * ═══════════════════════════════════
 */

#include "_runtime.h"

/* namespace Tests */

/* ── Forward Declarations ── */
typedef struct _Counter Counter;
typedef struct _Program Program;
static i64 Program_Apply();
static i64 Program_Clamp();
static void Program_Main();



#line 3 "./tests/samples/closures.am"
/* class Counter */
struct _Counter {
    i64 Value;
};


#line 6 "./tests/samples/closures.am"
Counter* Counter_new(i64 start) {
    Counter* self = (Counter*) code_alloc(sizeof(Counter));
{

#line 7 "./tests/samples/closures.am"
        self->Value = start;
    }    return self;
}


#line 10 "./tests/samples/closures.am"
void Counter_Increment(Counter* self) {

#line 11 "./tests/samples/closures.am"
    self->Value = self->Value + 1;
}


#line 14 "./tests/samples/closures.am"
void Counter_Add(Counter* self, i64 n) {

#line 15 "./tests/samples/closures.am"
    self->Value = self->Value + n;
}


#line 18 "./tests/samples/closures.am"
code_string Counter_Report(Counter* self) {

#line 19 "./tests/samples/closures.am"
    return code_string_format("Counter = %s", code_int_to_string(self->Value));
}



#line 23 "./tests/samples/closures.am"
/* class Program */
struct _Program {
};

Program* Program_new() {
    Program* self = (Program*) code_alloc(sizeof(Program));
    return self;
}


#line 24 "./tests/samples/closures.am"
static i64 Program_Apply(i64 x, i64 delta) {

#line 25 "./tests/samples/closures.am"
    return x + delta;
}


#line 28 "./tests/samples/closures.am"
static i64 Program_Clamp(i64 value, i64 min, i64 max) {

#line 29 "./tests/samples/closures.am"
    if (value < min) {

#line 30 "./tests/samples/closures.am"
        return min;
    }

#line 32 "./tests/samples/closures.am"
    if (value > max) {

#line 33 "./tests/samples/closures.am"
        return max;
    }

#line 35 "./tests/samples/closures.am"
    return value;
}


#line 38 "./tests/samples/closures.am"
static void Program_Main(int argc, char** argv) {

#line 39 "./tests/samples/closures.am"
    Counter* c = Counter_new(0);

#line 41 "./tests/samples/closures.am"
    Counter_Increment(c);

#line 42 "./tests/samples/closures.am"
    Counter_Increment(c);

#line 43 "./tests/samples/closures.am"
    Counter_Add(c, 8);

#line 44 "./tests/samples/closures.am"
    Console_WriteLine(Counter_Report(c));

#line 46 "./tests/samples/closures.am"
    i64 v1 = Program_Apply(5, 3);

#line 47 "./tests/samples/closures.am"
    i64 v2 = Program_Apply(v1, -2);

#line 48 "./tests/samples/closures.am"
    Console_WriteLine(code_string_format("Apply: %s to %s", code_int_to_string(v1), code_int_to_string(v2)));

#line 50 "./tests/samples/closures.am"
    i64 clamped = Program_Clamp(150, 0, 100);

#line 51 "./tests/samples/closures.am"
    Console_WriteLine(code_string_format("Clamp(150, 0, 100) = %s", code_int_to_string(clamped)));

#line 53 "./tests/samples/closures.am"
    i64 inner = Program_Clamp(200, 0, 100);

#line 54 "./tests/samples/closures.am"
    i64 outer = Program_Apply(10, 5);

#line 55 "./tests/samples/closures.am"
    i64 result = Program_Apply(inner, outer);

#line 56 "./tests/samples/closures.am"
    Console_WriteLine(code_string_format("Nested: %s", code_int_to_string(result)));
}



/* ── Point d entree C ── */
int main(int argc, char** argv) {
    code_runtime_init();
    Program_Main(argc, argv);
    return 0;
}
