/* ═══════════════════════════════════
 * Généré par Amalgame Transpiler v0.3.0
 * Source : ./tests/samples/inheritance.am
 * NE PAS MODIFIER MANUELLEMENT
 * ═══════════════════════════════════
 */

#include "_runtime.h"

/* namespace Tests */

/* ── Forward Declarations ── */
typedef struct _Shape Shape;
typedef struct _Circle Circle;
typedef struct _Rectangle Rectangle;
typedef struct _Program Program;
static void Program_Main();



#line 3 "./tests/samples/inheritance.am"
/* class Shape */
struct _Shape {
    code_string Name;
};


#line 6 "./tests/samples/inheritance.am"
Shape* Shape_new(code_string name) {
    Shape* self = (Shape*) code_alloc(sizeof(Shape));
{

#line 7 "./tests/samples/inheritance.am"
        self->Name = name;
    }    return self;
}


#line 10 "./tests/samples/inheritance.am"
code_string Shape_Describe(Shape* self) {

#line 11 "./tests/samples/inheritance.am"
    return code_string_format("Shape: %s", self->Name);
}



#line 15 "./tests/samples/inheritance.am"
/* class Circle */
struct _Circle {
    struct _Shape _base; /* extends Shape */
    f32 Radius;
};


#line 18 "./tests/samples/inheritance.am"
Circle* Circle_new(code_string name, f32 radius) {
    Circle* self = (Circle*) code_alloc(sizeof(Circle));
{

#line 19 "./tests/samples/inheritance.am"
        self->_base.Name = name;

#line 20 "./tests/samples/inheritance.am"
        self->Radius = radius;
    }    return self;
}


#line 23 "./tests/samples/inheritance.am"
f32 Circle_Area(Circle* self) {

#line 24 "./tests/samples/inheritance.am"
    return 3.14159 * self->Radius * self->Radius;
}


#line 27 "./tests/samples/inheritance.am"
code_string Circle_Describe(Circle* self) {

#line 28 "./tests/samples/inheritance.am"
    return code_string_format("Circle '%s' r=%s", self->_base.Name, code_float_to_string(self->Radius));
}



#line 32 "./tests/samples/inheritance.am"
/* class Rectangle */
struct _Rectangle {
    struct _Shape _base; /* extends Shape */
    f32 Width;
    f32 Height;
};


#line 36 "./tests/samples/inheritance.am"
Rectangle* Rectangle_new(code_string name, f32 width, f32 height) {
    Rectangle* self = (Rectangle*) code_alloc(sizeof(Rectangle));
{

#line 37 "./tests/samples/inheritance.am"
        self->_base.Name = name;

#line 38 "./tests/samples/inheritance.am"
        self->Width = width;

#line 39 "./tests/samples/inheritance.am"
        self->Height = height;
    }    return self;
}


#line 42 "./tests/samples/inheritance.am"
f32 Rectangle_Area(Rectangle* self) {

#line 43 "./tests/samples/inheritance.am"
    return self->Width * self->Height;
}


#line 46 "./tests/samples/inheritance.am"
code_string Rectangle_Describe(Rectangle* self) {

#line 47 "./tests/samples/inheritance.am"
    return code_string_format("Rectangle '%s' %sx%s", self->_base.Name, code_float_to_string(self->Width), code_float_to_string(self->Height));
}



#line 51 "./tests/samples/inheritance.am"
/* class Program */
struct _Program {
};

Program* Program_new() {
    Program* self = (Program*) code_alloc(sizeof(Program));
    return self;
}


#line 52 "./tests/samples/inheritance.am"
static void Program_Main(int argc, char** argv) {

#line 53 "./tests/samples/inheritance.am"
    Circle* c = Circle_new("Sun", 5.0);

#line 54 "./tests/samples/inheritance.am"
    Rectangle* r = Rectangle_new("Wall", 4.0, 6.0);

#line 56 "./tests/samples/inheritance.am"
    Console_WriteLine(Circle_Describe(c));

#line 57 "./tests/samples/inheritance.am"
    Console_WriteLine(Rectangle_Describe(r));

#line 59 "./tests/samples/inheritance.am"
    f32 ca = Circle_Area(c);

#line 60 "./tests/samples/inheritance.am"
    f32 ra = Rectangle_Area(r);

#line 62 "./tests/samples/inheritance.am"
    Console_WriteLine(code_string_format("Circle area: %s", code_float_to_string(ca)));

#line 63 "./tests/samples/inheritance.am"
    Console_WriteLine(code_string_format("Rectangle area: %s", code_float_to_string(ra)));
}



/* ── Point d entree C ── */
int main(int argc, char** argv) {
    code_runtime_init();
    Program_Main(argc, argv);
    return 0;
}
