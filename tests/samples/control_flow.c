/* ═══════════════════════════════════
 * Généré par Amalgame Transpiler v0.3.0
 * Source : ./tests/samples/control_flow.am
 * NE PAS MODIFIER MANUELLEMENT
 * ═══════════════════════════════════
 */

#include "_runtime.h"

/* namespace Tests */

/* ── Forward Declarations ── */
typedef struct _Program Program;
static void Program_Main();



#line 3 "./tests/samples/control_flow.am"
/* class Program */
struct _Program {
};

Program* Program_new() {
    Program* self = (Program*) code_alloc(sizeof(Program));
    return self;
}


#line 4 "./tests/samples/control_flow.am"
static void Program_Main(int argc, char** argv) {

#line 6 "./tests/samples/control_flow.am"
    i64 score = 85;

#line 7 "./tests/samples/control_flow.am"
    if (score >= 90) {

#line 8 "./tests/samples/control_flow.am"
        Console_WriteLine("Grade: A");
    } else if (score >= 80) {

#line 10 "./tests/samples/control_flow.am"
        Console_WriteLine("Grade: B");
    } else if (score >= 70) {

#line 12 "./tests/samples/control_flow.am"
        Console_WriteLine("Grade: C");
    } else {

#line 14 "./tests/samples/control_flow.am"
        Console_WriteLine("Grade: F");
    }

#line 18 "./tests/samples/control_flow.am"
    i64 i = 0;

#line 19 "./tests/samples/control_flow.am"
    while (i < 3) {

#line 20 "./tests/samples/control_flow.am"
        Console_WriteLine(code_string_format("while: %s", code_int_to_string(i)));

#line 21 "./tests/samples/control_flow.am"
        i = i + 1;
    }

#line 25 "./tests/samples/control_flow.am"
    for (i64 j = 0; j < 3; j = j + 1) {

#line 26 "./tests/samples/control_flow.am"
        Console_WriteLine(code_string_format("for: %s", code_int_to_string(j)));
    }
}



/* ── Point d entree C ── */
int main(int argc, char** argv) {
    code_runtime_init();
    Program_Main(argc, argv);
    return 0;
}
