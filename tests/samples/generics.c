/* ═══════════════════════════════════
 * Généré par Amalgame Transpiler v0.3.0
 * Source : ./tests/samples/generics.am
 * NE PAS MODIFIER MANUELLEMENT
 * ═══════════════════════════════════
 */

#include "_runtime.h"

/* namespace Tests */

/* ── Forward Declarations ── */
typedef struct _Stack Stack;
typedef struct _Pair Pair;
typedef struct _Program Program;
static i64 Program_Max();
static code_string Program_Repeat();
static void Program_Main();



#line 3 "./tests/samples/generics.am"
/* class Stack */
struct _Stack {
    i64* Items;
    i64 Size;
};


#line 7 "./tests/samples/generics.am"
Stack* Stack_new() {
    Stack* self = (Stack*) code_alloc(sizeof(Stack));
{

#line 8 "./tests/samples/generics.am"
        self->Size = 0;
    }    return self;
}


#line 11 "./tests/samples/generics.am"
void Stack_Push(Stack* self, i64 value) {

#line 12 "./tests/samples/generics.am"
    self->Size = self->Size + 1;

#line 13 "./tests/samples/generics.am"
    Console_WriteLine(code_string_format("Pushed: %s", code_int_to_string(value)));
}


#line 16 "./tests/samples/generics.am"
i64 Stack_Size(Stack* self) {

#line 17 "./tests/samples/generics.am"
    return self->Size;
}



#line 21 "./tests/samples/generics.am"
/* class Pair */
struct _Pair {
    code_string First;
    i64 Second;
};


#line 25 "./tests/samples/generics.am"
Pair* Pair_new(code_string first, i64 second) {
    Pair* self = (Pair*) code_alloc(sizeof(Pair));
{

#line 26 "./tests/samples/generics.am"
        self->First = first;

#line 27 "./tests/samples/generics.am"
        self->Second = second;
    }    return self;
}


#line 30 "./tests/samples/generics.am"
code_string Pair_ToString(Pair* self) {

#line 31 "./tests/samples/generics.am"
    return code_string_format("(%s, %s)", self->First, code_int_to_string(self->Second));
}



#line 35 "./tests/samples/generics.am"
/* class Program */
struct _Program {
};

Program* Program_new() {
    Program* self = (Program*) code_alloc(sizeof(Program));
    return self;
}


#line 36 "./tests/samples/generics.am"
static i64 Program_Max(i64 a, i64 b) {

#line 37 "./tests/samples/generics.am"
    if (a > b) {

#line 38 "./tests/samples/generics.am"
        return a;
    }

#line 40 "./tests/samples/generics.am"
    return b;
}


#line 43 "./tests/samples/generics.am"
static code_string Program_Repeat(code_string s, i64 n) {

#line 44 "./tests/samples/generics.am"
    code_string result = "";

#line 45 "./tests/samples/generics.am"
    i64 i = 0;

#line 46 "./tests/samples/generics.am"
    while (i < n) {

#line 47 "./tests/samples/generics.am"
        result = code_string_concat(result, s);

#line 48 "./tests/samples/generics.am"
        i = i + 1;
    }

#line 50 "./tests/samples/generics.am"
    return result;
}


#line 53 "./tests/samples/generics.am"
static void Program_Main(int argc, char** argv) {

#line 54 "./tests/samples/generics.am"
    Pair* p1 = Pair_new("hello", 42);

#line 55 "./tests/samples/generics.am"
    Pair* p2 = Pair_new("world", 99);

#line 57 "./tests/samples/generics.am"
    Console_WriteLine(Pair_ToString(p1));

#line 58 "./tests/samples/generics.am"
    Console_WriteLine(Pair_ToString(p2));

#line 60 "./tests/samples/generics.am"
    i64 bigger = Program_Max(p1->Second, p2->Second);

#line 61 "./tests/samples/generics.am"
    Console_WriteLine(code_string_format("Max: %s", code_int_to_string(bigger)));

#line 63 "./tests/samples/generics.am"
    code_string rep = Program_Repeat("ab", 3);

#line 64 "./tests/samples/generics.am"
    Console_WriteLine(code_string_format("Repeat: %s", rep));
}



/* ── Point d entree C ── */
int main(int argc, char** argv) {
    code_runtime_init();
    Program_Main(argc, argv);
    return 0;
}
