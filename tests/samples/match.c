/* ═══════════════════════════════════
 * Généré par CODE Transpiler v0.1.0
 * Source : ./tests/samples/match.am
 * NE PAS MODIFIER MANUELLEMENT
 * ═══════════════════════════════════
 */

#include "_runtime.h"

/* namespace Tests */

/* ── Forward Declarations ── */
typedef struct _Program Program;
static void Program_Main();



#line 3 "./tests/samples/match.am"
/* class Program */
struct _Program {
};

Program* Program_new() {
    Program* self = (Program*) code_alloc(sizeof(Program));
    return self;
}


#line 4 "./tests/samples/match.am"
static void Program_Main(int argc, char** argv) {

#line 5 "./tests/samples/match.am"
    i64 health = 75;

#line 7 "./tests/samples/match.am"
    /* match */
    {
        int _match_idx = 0;
        if (health == 100) {
            Console_WriteLine("Full health");
        }
        else if (health >= 75 && health <= 99) {
            Console_WriteLine("Slightly wounded");
        }
        else if (health >= 50 && health <= 74) {
            Console_WriteLine("Wounded");
        }
        else if (health >= 1 && health <= 49) {
            Console_WriteLine("Critical");
        }
        else if (health == 0) {
            Console_WriteLine("Dead");
        }
        else {
            Console_WriteLine("Unknown");
        }
    }

#line 16 "./tests/samples/match.am"
    i64 day = 3;

#line 17 "./tests/samples/match.am"
    /* match */
    {
        int _match_idx = 0;
        if (day == 1) {
            Console_WriteLine("Monday");
        }
        else if (day == 2) {
            Console_WriteLine("Tuesday");
        }
        else if (day == 3) {
            Console_WriteLine("Wednesday");
        }
        else if (day == 4) {
            Console_WriteLine("Thursday");
        }
        else if (day == 5) {
            Console_WriteLine("Friday");
        }
        else {
            Console_WriteLine("Weekend");
        }
    }
}



/* ── Point d entree C ── */
int main(int argc, char** argv) {
    code_runtime_init();
    Program_Main(argc, argv);
    return 0;
}
