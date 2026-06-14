#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "_runtime.h"
#include "Amalgame_String.h"
#include "Amalgame_Collections.h"
#include "Amalgame_IO.h"
#include "Amalgame_Net.h"
#include "Amalgame_Console.h"
#include "Amalgame_Process.h"

typedef struct _Tests_Point Tests_Point;
typedef struct _Tests_Player Tests_Player;
typedef struct _Tests_Program Tests_Program;
typedef enum _Amalgame_Formats_Json_JsonKind {
    Amalgame_Formats_Json_JsonKind_Null,
    Amalgame_Formats_Json_JsonKind_Bool,
    Amalgame_Formats_Json_JsonKind_Int,
    Amalgame_Formats_Json_JsonKind_Float,
    Amalgame_Formats_Json_JsonKind_String,
    Amalgame_Formats_Json_JsonKind_Array,
    Amalgame_Formats_Json_JsonKind_Object
} Amalgame_Formats_Json_JsonKind; /* external */
typedef struct _Amalgame_Formats_Json_JsonValue Amalgame_Formats_Json_JsonValue; /* external */
typedef struct _Amalgame_Formats_Json_JsonError Amalgame_Formats_Json_JsonError; /* external */
typedef struct _Amalgame_Formats_Json_JsonResult Amalgame_Formats_Json_JsonResult; /* external */
typedef struct _Amalgame_Formats_Json_JsonParser Amalgame_Formats_Json_JsonParser; /* external */
typedef struct _Amalgame_Formats_Json_Json Amalgame_Formats_Json_Json; /* external */
struct _Amalgame_Formats_Json_JsonValue {
    Amalgame_Formats_Json_JsonKind Kind;
    code_bool B;
    i64 I;
    double F;
    code_string S;
    AmalgameList* Items;
    AmalgameList* ObjKeys;
    AmalgameList* ObjVals;
};
struct _Amalgame_Formats_Json_JsonError {
    code_string Message;
    i64 Line;
    i64 Column;
};
struct _Amalgame_Formats_Json_JsonResult {
    code_bool Ok;
    Amalgame_Formats_Json_JsonValue* Value;
    Amalgame_Formats_Json_JsonError* Error;
};
struct _Amalgame_Formats_Json_JsonParser {
    code_string Source;
    i64 Pos;
    i64 Line;
    i64 Column;
    code_bool Failed;
    code_string ErrMsg;
    i64 ErrLine;
    i64 ErrCol;
};
struct _Amalgame_Formats_Json_Json {
};
Amalgame_Formats_Json_JsonValue* Amalgame_Formats_Json_JsonValue_new();
code_bool Amalgame_Formats_Json_JsonValue_IsNull(Amalgame_Formats_Json_JsonValue* self);
code_bool Amalgame_Formats_Json_JsonValue_IsBool(Amalgame_Formats_Json_JsonValue* self);
code_bool Amalgame_Formats_Json_JsonValue_IsInt(Amalgame_Formats_Json_JsonValue* self);
code_bool Amalgame_Formats_Json_JsonValue_IsFloat(Amalgame_Formats_Json_JsonValue* self);
code_bool Amalgame_Formats_Json_JsonValue_IsNumber(Amalgame_Formats_Json_JsonValue* self);
code_bool Amalgame_Formats_Json_JsonValue_IsString(Amalgame_Formats_Json_JsonValue* self);
code_bool Amalgame_Formats_Json_JsonValue_IsArray(Amalgame_Formats_Json_JsonValue* self);
code_bool Amalgame_Formats_Json_JsonValue_IsObject(Amalgame_Formats_Json_JsonValue* self);
code_bool Amalgame_Formats_Json_JsonValue_AsBool(Amalgame_Formats_Json_JsonValue* self);
i64 Amalgame_Formats_Json_JsonValue_AsInt(Amalgame_Formats_Json_JsonValue* self);
double Amalgame_Formats_Json_JsonValue_AsFloat(Amalgame_Formats_Json_JsonValue* self);
code_string Amalgame_Formats_Json_JsonValue_AsString(Amalgame_Formats_Json_JsonValue* self);
AmalgameList* Amalgame_Formats_Json_JsonValue_AsArray(Amalgame_Formats_Json_JsonValue* self);
Amalgame_Formats_Json_JsonValue* Amalgame_Formats_Json_JsonValue_Get(Amalgame_Formats_Json_JsonValue* self, code_string key);
code_bool Amalgame_Formats_Json_JsonValue_Has(Amalgame_Formats_Json_JsonValue* self, code_string key);
AmalgameList* Amalgame_Formats_Json_JsonValue_Keys(Amalgame_Formats_Json_JsonValue* self);
Amalgame_Formats_Json_JsonValue* Amalgame_Formats_Json_JsonValue_At(Amalgame_Formats_Json_JsonValue* self, i64 i);
i64 Amalgame_Formats_Json_JsonValue_Length(Amalgame_Formats_Json_JsonValue* self);
void Amalgame_Formats_Json_JsonValue_SetNull(Amalgame_Formats_Json_JsonValue* self);
void Amalgame_Formats_Json_JsonValue_SetBool(Amalgame_Formats_Json_JsonValue* self, code_bool b);
void Amalgame_Formats_Json_JsonValue_SetInt(Amalgame_Formats_Json_JsonValue* self, i64 n);
void Amalgame_Formats_Json_JsonValue_SetFloat(Amalgame_Formats_Json_JsonValue* self, double f);
void Amalgame_Formats_Json_JsonValue_SetString(Amalgame_Formats_Json_JsonValue* self, code_string s);
void Amalgame_Formats_Json_JsonValue_SetArray(Amalgame_Formats_Json_JsonValue* self, AmalgameList* xs);
void Amalgame_Formats_Json_JsonValue_SetObject(Amalgame_Formats_Json_JsonValue* self, AmalgameList* keys, AmalgameList* vals);
void Amalgame_Formats_Json_JsonValue_AppendItem(Amalgame_Formats_Json_JsonValue* self, Amalgame_Formats_Json_JsonValue* v);
void Amalgame_Formats_Json_JsonValue_AppendEntry(Amalgame_Formats_Json_JsonValue* self, code_string key, Amalgame_Formats_Json_JsonValue* v);
Amalgame_Formats_Json_JsonError* Amalgame_Formats_Json_JsonError_new(code_string msg, i64 line, i64 col);
Amalgame_Formats_Json_JsonResult* Amalgame_Formats_Json_JsonResult_new();
Amalgame_Formats_Json_JsonParser* Amalgame_Formats_Json_JsonParser_new(code_string source);
code_bool Amalgame_Formats_Json_JsonParser_HasFailed(Amalgame_Formats_Json_JsonParser* self);
code_string Amalgame_Formats_Json_JsonParser_ErrorMsg(Amalgame_Formats_Json_JsonParser* self);
i64 Amalgame_Formats_Json_JsonParser_ErrorLine(Amalgame_Formats_Json_JsonParser* self);
i64 Amalgame_Formats_Json_JsonParser_ErrorCol(Amalgame_Formats_Json_JsonParser* self);
void Amalgame_Formats_Json_JsonParser_Fail(Amalgame_Formats_Json_JsonParser* self, code_string msg);
code_bool Amalgame_Formats_Json_JsonParser_AtEnd(Amalgame_Formats_Json_JsonParser* self);
code_string Amalgame_Formats_Json_JsonParser_Peek(Amalgame_Formats_Json_JsonParser* self);
code_string Amalgame_Formats_Json_JsonParser_PeekAt(Amalgame_Formats_Json_JsonParser* self, i64 offset);
code_string Amalgame_Formats_Json_JsonParser_Advance(Amalgame_Formats_Json_JsonParser* self);
void Amalgame_Formats_Json_JsonParser_SkipWs(Amalgame_Formats_Json_JsonParser* self);
code_bool Amalgame_Formats_Json_JsonParser_MatchLit(Amalgame_Formats_Json_JsonParser* self, code_string lit);
Amalgame_Formats_Json_JsonValue* Amalgame_Formats_Json_JsonParser_ParseTopLevel(Amalgame_Formats_Json_JsonParser* self);
Amalgame_Formats_Json_JsonValue* Amalgame_Formats_Json_JsonParser_ParseValue(Amalgame_Formats_Json_JsonParser* self);
code_bool Amalgame_Formats_Json_JsonParser_IsDigit(code_string c);
Amalgame_Formats_Json_JsonValue* Amalgame_Formats_Json_JsonParser_ParseObject(Amalgame_Formats_Json_JsonParser* self);
Amalgame_Formats_Json_JsonValue* Amalgame_Formats_Json_JsonParser_ParseArray(Amalgame_Formats_Json_JsonParser* self);
Amalgame_Formats_Json_JsonValue* Amalgame_Formats_Json_JsonParser_ParseString(Amalgame_Formats_Json_JsonParser* self);
i64 Amalgame_Formats_Json_JsonParser_ParseHex4(Amalgame_Formats_Json_JsonParser* self);
i64 Amalgame_Formats_Json_JsonParser_HexDigit(code_string c);
code_string Amalgame_Formats_Json_JsonParser_Bs();
code_string Amalgame_Formats_Json_JsonParser_Ff();
Amalgame_Formats_Json_JsonValue* Amalgame_Formats_Json_JsonParser_ParseNumber(Amalgame_Formats_Json_JsonParser* self);
Amalgame_Formats_Json_JsonValue* Amalgame_Formats_Json_JsonParser_ParseBool(Amalgame_Formats_Json_JsonParser* self);
Amalgame_Formats_Json_JsonValue* Amalgame_Formats_Json_JsonParser_ParseNull(Amalgame_Formats_Json_JsonParser* self);
Amalgame_Formats_Json_Json* Amalgame_Formats_Json_Json_new();
Amalgame_Formats_Json_JsonResult* Amalgame_Formats_Json_Json_Parse(code_string source);
code_string Amalgame_Formats_Json_Json_Encode(Amalgame_Formats_Json_JsonValue* v);
code_string Amalgame_Formats_Json_Json_EncodeArray(Amalgame_Formats_Json_JsonValue* v);
code_string Amalgame_Formats_Json_Json_EncodeObject(Amalgame_Formats_Json_JsonValue* v);
code_string Amalgame_Formats_Json_Json_EscapeString(code_string s);
Amalgame_Formats_Json_JsonValue* Amalgame_Formats_Json_Json_NullValue();
Amalgame_Formats_Json_JsonValue* Amalgame_Formats_Json_Json_OfBool(code_bool b);
Amalgame_Formats_Json_JsonValue* Amalgame_Formats_Json_Json_OfInt(i64 n);
Amalgame_Formats_Json_JsonValue* Amalgame_Formats_Json_Json_OfFloat(double f);
Amalgame_Formats_Json_JsonValue* Amalgame_Formats_Json_Json_OfString(code_string s);
Amalgame_Formats_Json_JsonValue* Amalgame_Formats_Json_Json_OfArray(AmalgameList* xs);

Tests_Point* Tests_Point_new(double X, double Y);
Tests_Player* Tests_Player_new(code_string Name, i64 Health, i64 Level);
Tests_Program* Tests_Program_new();
double Tests_Program_Distance(Tests_Point* a, Tests_Point* b);
code_bool Tests_Program_IsAlive(Tests_Player* p);
void Tests_Program_PrintPlayer(Tests_Player* p);
void Tests_Program_Main(code_string* args);
struct _Tests_Point {
    double X;
    double Y;
};


Tests_Point* Tests_Point_new(double X, double Y) {
    Tests_Point* self = (Tests_Point*) GC_MALLOC(sizeof(Tests_Point));
    self->X = X;
    self->Y = Y;
    return self;
}

struct _Tests_Player {
    code_string Name;
    i64 Health;
    i64 Level;
};


Tests_Player* Tests_Player_new(code_string Name, i64 Health, i64 Level) {
    Tests_Player* self = (Tests_Player*) GC_MALLOC(sizeof(Tests_Player));
    self->Name = Name;
    self->Health = Health;
    self->Level = Level;
    return self;
}

struct _Tests_Program {
};

double Tests_Program_Distance(Tests_Point* a, Tests_Point* b);
code_bool Tests_Program_IsAlive(Tests_Player* p);
void Tests_Program_PrintPlayer(Tests_Player* p);
void Tests_Program_Main(code_string* args);

Tests_Program* Tests_Program_new() {
    Tests_Program* self = (Tests_Program*) GC_MALLOC(sizeof(Tests_Program));
    return self;
}

double Tests_Program_Distance(Tests_Point* a, Tests_Point* b) {
    #line 9 "tests/samples/data_classes.am"
    double dx = a->X - b->X;
    #line 10 "tests/samples/data_classes.am"
    double dy = a->Y - b->Y;
    #line 11 "tests/samples/data_classes.am"
    return (dx * dx) + (dy * dy);
}

code_bool Tests_Program_IsAlive(Tests_Player* p) {
    #line 15 "tests/samples/data_classes.am"
    return p->Health > 0LL;
}

void Tests_Program_PrintPlayer(Tests_Player* p) {
    #line 19 "tests/samples/data_classes.am"
    Console_WriteLine(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("", "Player: "), (p->Name ? p->Name : "")), " HP="), String_FromInt(p->Health)), " Lvl="), String_FromInt(p->Level)));
}

void Tests_Program_Main(code_string* args) {
    #line 23 "tests/samples/data_classes.am"
    Tests_Point* origin = Tests_Point_new(0.0, 0.0);
    #line 24 "tests/samples/data_classes.am"
    Tests_Point* target = Tests_Point_new(3.0, 4.0);
    #line 26 "tests/samples/data_classes.am"
    double dist = Tests_Program_Distance(origin, target);
    #line 27 "tests/samples/data_classes.am"
    Console_WriteLine(code_string_concat(code_string_concat("", "Distance squared: "), String_FromFloat(dist)));
    #line 29 "tests/samples/data_classes.am"
    Tests_Player* hero = Tests_Player_new("Arthus", 100LL, 42LL);
    #line 30 "tests/samples/data_classes.am"
    Tests_Player* fallen = Tests_Player_new("Ghost", 0LL, 10LL);
    #line 32 "tests/samples/data_classes.am"
    Tests_Program_PrintPlayer(hero);
    #line 33 "tests/samples/data_classes.am"
    Tests_Program_PrintPlayer(fallen);
    #line 35 "tests/samples/data_classes.am"
    code_bool alive = Tests_Program_IsAlive(hero);
    #line 36 "tests/samples/data_classes.am"
    Console_WriteLine(code_string_concat(code_string_concat("", "Arthus alive: "), (alive ? "true" : "false")));
}


int main(int argc, char** argv) {
    GC_INIT();
    code_runtime_init_args(argc, argv);
    Tests_Program_Main((code_string*)argv);
    return code_exit_code;
}
