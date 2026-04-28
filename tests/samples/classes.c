/* ═══════════════════════════════════
 * Généré par Amalgame Transpiler v0.3.0
 * Source : ./tests/samples/classes.am
 * NE PAS MODIFIER MANUELLEMENT
 * ═══════════════════════════════════
 */

#include "_runtime.h"

/* namespace Tests */

/* ── Forward Declarations ── */
typedef struct _Animal Animal;
typedef struct _Program Program;
static void Program_Main();



#line 3 "./tests/samples/classes.am"
/* class Animal */
struct _Animal {
    code_string Name;
    i64 Age;
};


#line 7 "./tests/samples/classes.am"
Animal* Animal_new(code_string name, i64 age) {
    Animal* self = (Animal*) code_alloc(sizeof(Animal));
{

#line 8 "./tests/samples/classes.am"
        self->Name = name;

#line 9 "./tests/samples/classes.am"
        self->Age = age;
    }    return self;
}


#line 12 "./tests/samples/classes.am"
void Animal_Speak(Animal* self) {

#line 13 "./tests/samples/classes.am"
    Console_WriteLine(code_string_format("%s says hello!", self->Name));
}


#line 16 "./tests/samples/classes.am"
code_string Animal_Describe(Animal* self) {

#line 17 "./tests/samples/classes.am"
    return code_string_format("%s is %s years old", self->Name, code_int_to_string(self->Age));
}



#line 21 "./tests/samples/classes.am"
/* class Program */
struct _Program {
};

Program* Program_new() {
    Program* self = (Program*) code_alloc(sizeof(Program));
    return self;
}


#line 22 "./tests/samples/classes.am"
static void Program_Main(int argc, char** argv) {

#line 23 "./tests/samples/classes.am"
    Animal* cat = Animal_new("Cat", 3);

#line 24 "./tests/samples/classes.am"
    Animal* dog = Animal_new("Dog", 5);

#line 26 "./tests/samples/classes.am"
    Animal_Speak(cat);

#line 27 "./tests/samples/classes.am"
    Animal_Speak(dog);

#line 29 "./tests/samples/classes.am"
    code_string desc = Animal_Describe(cat);

#line 30 "./tests/samples/classes.am"
    Console_WriteLine(desc);
}



/* ── Point d entree C ── */
int main(int argc, char** argv) {
    code_runtime_init();
    Program_Main(argc, argv);
    return 0;
}
