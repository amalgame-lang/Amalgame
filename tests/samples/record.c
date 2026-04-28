/* ═══════════════════════════════════
 * Généré par Amalgame Transpiler v0.3.0
 * Source : ./tests/samples/record.am
 * NE PAS MODIFIER MANUELLEMENT
 * ═══════════════════════════════════
 */

#include "_runtime.h"

/* namespace Tests */

/* ── Forward Declarations ── */
typedef struct _Point Point;
typedef struct _Color Color;
typedef struct _Program Program;
static void Program_Main();



#line 3 "./tests/samples/record.am"
/* record Point */
struct _Point {
    f32 X;
    f32 Y;
};

Point* Point_new(f32 X, f32 Y) {
    Point* self = (Point*) code_alloc(sizeof(Point));
    self->X = X;
    self->Y = Y;
    return self;
}



#line 5 "./tests/samples/record.am"
/* record Color */
struct _Color {
    i64 R;
    i64 G;
    i64 B;
};

Color* Color_new(i64 R, i64 G, i64 B) {
    Color* self = (Color*) code_alloc(sizeof(Color));
    self->R = R;
    self->G = G;
    self->B = B;
    return self;
}



#line 7 "./tests/samples/record.am"
/* class Program */
struct _Program {
};

Program* Program_new() {
    Program* self = (Program*) code_alloc(sizeof(Program));
    return self;
}


#line 8 "./tests/samples/record.am"
static void Program_Main(int argc, char** argv) {

#line 9 "./tests/samples/record.am"
    Point* p = Point_new(3.0, 4.0);

#line 10 "./tests/samples/record.am"
    Console_WriteLine(code_string_format("Point: (%s, %s)", code_float_to_string(p->X), code_float_to_string(p->Y)));

#line 12 "./tests/samples/record.am"
    Color* red = Color_new(255, 0, 0);

#line 13 "./tests/samples/record.am"
    Console_WriteLine(code_string_format("Color: (%s, %s, %s)", code_int_to_string(red->R), code_int_to_string(red->G), code_int_to_string(red->B)));
}



/* ── Point d entree C ── */
int main(int argc, char** argv) {
    code_runtime_init();
    Program_Main(argc, argv);
    return 0;
}
