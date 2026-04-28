/* ═══════════════════════════════════
 * Généré par Amalgame Transpiler v0.3.0
 * Source : ./tests/samples/variables.am
 * NE PAS MODIFIER MANUELLEMENT
 * ═══════════════════════════════════
 */

#include "_runtime.h"

/* namespace Tests */

/* ── Forward Declarations ── */
typedef struct _Program Program;
static void Program_Main();



#line 3 "./tests/samples/variables.am"
/* class Program */
struct _Program {
};

Program* Program_new() {
    Program* self = (Program*) code_alloc(sizeof(Program));
    return self;
}


#line 4 "./tests/samples/variables.am"
static void Program_Main(int argc, char** argv) {

#line 5 "./tests/samples/variables.am"
    i64 x = 42;

#line 6 "./tests/samples/variables.am"
    f32 y = 3.14;

#line 7 "./tests/samples/variables.am"
    code_string name = "CODE";

#line 8 "./tests/samples/variables.am"
    code_bool flag = true;

#line 10 "./tests/samples/variables.am"
    Console_WriteLine(code_string_format("int: %s", code_int_to_string(x)));

#line 11 "./tests/samples/variables.am"
    Console_WriteLine(code_string_format("float: %s", code_float_to_string(y)));

#line 12 "./tests/samples/variables.am"
    Console_WriteLine(code_string_format("string: %s", name));

#line 14 "./tests/samples/variables.am"
    i64 counter = 0;

#line 15 "./tests/samples/variables.am"
    counter = counter + 1;

#line 16 "./tests/samples/variables.am"
    Console_WriteLine(code_string_format("counter: %s", code_int_to_string(counter)));
}



/* ── Point d entree C ── */
int main(int argc, char** argv) {
    code_runtime_init();
    Program_Main(argc, argv);
    return 0;
}
