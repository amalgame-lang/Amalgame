/* ═══════════════════════════════════
 * Généré par Amalgame Transpiler v0.3.0
 * Source : ./tests/samples/data_classes.am
 * NE PAS MODIFIER MANUELLEMENT
 * ═══════════════════════════════════
 */

#include "_runtime.h"

/* namespace Tests */

/* ── Forward Declarations ── */
typedef struct _Point Point;
typedef struct _Player Player;
typedef struct _Program Program;
static f32 Program_Distance();
static code_bool Program_IsAlive();
static void Program_PrintPlayer();
static void Program_Main();



#line 3 "./tests/samples/data_classes.am"
/* data class Point */
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



#line 5 "./tests/samples/data_classes.am"
/* data class Player */
struct _Player {
    code_string Name;
    i64 Health;
    i64 Level;
};

Player* Player_new(code_string Name, i64 Health, i64 Level) {
    Player* self = (Player*) code_alloc(sizeof(Player));
    self->Name = Name;
    self->Health = Health;
    self->Level = Level;
    return self;
}



#line 7 "./tests/samples/data_classes.am"
/* class Program */
struct _Program {
};

Program* Program_new() {
    Program* self = (Program*) code_alloc(sizeof(Program));
    return self;
}


#line 8 "./tests/samples/data_classes.am"
static f32 Program_Distance(Point* a, Point* b) {

#line 9 "./tests/samples/data_classes.am"
    f32 dx = a->X - b->X;

#line 10 "./tests/samples/data_classes.am"
    f32 dy = a->Y - b->Y;

#line 11 "./tests/samples/data_classes.am"
    return dx * dx + dy * dy;
}


#line 14 "./tests/samples/data_classes.am"
static code_bool Program_IsAlive(Player* p) {

#line 15 "./tests/samples/data_classes.am"
    return p->Health > 0;
}


#line 18 "./tests/samples/data_classes.am"
static void Program_PrintPlayer(Player* p) {

#line 19 "./tests/samples/data_classes.am"
    Console_WriteLine(code_string_format("Player: %s HP=%s Lvl=%s", p->Name, code_int_to_string(p->Health), code_int_to_string(p->Level)));
}


#line 22 "./tests/samples/data_classes.am"
static void Program_Main(int argc, char** argv) {

#line 23 "./tests/samples/data_classes.am"
    Point* origin = Point_new(0.0, 0.0);

#line 24 "./tests/samples/data_classes.am"
    Point* target = Point_new(3.0, 4.0);

#line 26 "./tests/samples/data_classes.am"
    f32 dist = Program_Distance(origin, target);

#line 27 "./tests/samples/data_classes.am"
    Console_WriteLine(code_string_format("Distance squared: %s", code_float_to_string(dist)));

#line 29 "./tests/samples/data_classes.am"
    Player* hero = Player_new("Arthus", 100, 42);

#line 30 "./tests/samples/data_classes.am"
    Player* fallen = Player_new("Ghost", 0, 10);

#line 32 "./tests/samples/data_classes.am"
    Program_PrintPlayer(hero);

#line 33 "./tests/samples/data_classes.am"
    Program_PrintPlayer(fallen);

#line 35 "./tests/samples/data_classes.am"
    code_bool alive = Program_IsAlive(hero);

#line 36 "./tests/samples/data_classes.am"
    Console_WriteLine(code_string_format("Arthus alive: %s", ((alive) ? "true" : "false")));
}



/* ── Point d entree C ── */
int main(int argc, char** argv) {
    code_runtime_init();
    Program_Main(argc, argv);
    return 0;
}
