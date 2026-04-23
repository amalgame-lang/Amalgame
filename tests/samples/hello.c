/* ═══════════════════════════════════
 * Généré par CODE Transpiler v0.1.0
 * Source : tests/samples/hello.code
 * NE PAS MODIFIER MANUELLEMENT
 * ═══════════════════════════════════
 */

#include "_runtime.h"

/* namespace MyApp */
/* import Code.IO */

/* ── Forward Declarations ── */
typedef struct _Program Program;
static void Program_Main();



#line 5 "tests/samples/hello.code"
/* class Program */
struct _Program {
};

Program* Program_new() {
    Program* self = (Program*) code_alloc(sizeof(Program));
    return self;
}


#line 6 "tests/samples/hello.code"
static void Program_Main(int argc, char** argv) {

#line 7 "tests/samples/hello.code"
    code_string name = "World";

#line 8 "tests/samples/hello.code"
    Console_WriteLine(code_string_format("Hello %s !", name));
}



/* ── Point d entree C ── */
int main(int argc, char** argv) {
    code_runtime_init();
    Program_Main(argc, argv);
    return 0;
}
