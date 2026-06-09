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

typedef enum _Tests_Direction Tests_Direction;
typedef enum _Tests_Season Tests_Season;
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

Tests_Program* Tests_Program_new();
code_string Tests_Program_DirectionName(Tests_Direction d);
code_bool Tests_Program_IsWarm(Tests_Season s);
void Tests_Program_Main(code_string* args);
enum _Tests_Direction {
    Tests_Direction_North,
    Tests_Direction_South,
    Tests_Direction_East,
    Tests_Direction_West
};

enum _Tests_Season {
    Tests_Season_Spring,
    Tests_Season_Summer,
    Tests_Season_Autumn,
    Tests_Season_Winter
};

struct _Tests_Program {
};

code_string Tests_Program_DirectionName(Tests_Direction d);
code_bool Tests_Program_IsWarm(Tests_Season s);
void Tests_Program_Main(code_string* args);

Tests_Program* Tests_Program_new() {
    Tests_Program* self = (Tests_Program*) GC_MALLOC(sizeof(Tests_Program));
    return self;
}

code_string Tests_Program_DirectionName(Tests_Direction d) {
    #line 13 "tests/samples/enums.am"
    { /* match d */
        if (d == Tests_Direction_North) {
            #line 14 "tests/samples/enums.am"
            return "North";
        } else if (d == Tests_Direction_South) {
            #line 15 "tests/samples/enums.am"
            return "South";
        } else if (d == Tests_Direction_East) {
            #line 16 "tests/samples/enums.am"
            return "East";
        } else if (d == Tests_Direction_West) {
            #line 17 "tests/samples/enums.am"
            return "West";
        } else {
            #line 18 "tests/samples/enums.am"
            return "Unknown";
        }
    }
    #line 20 "tests/samples/enums.am"
    return "Unknown";
}

code_bool Tests_Program_IsWarm(Tests_Season s) {
    #line 24 "tests/samples/enums.am"
    { /* match s */
        if (s == Tests_Season_Spring) {
            #line 25 "tests/samples/enums.am"
            return 1;
        } else if (s == Tests_Season_Summer) {
            #line 26 "tests/samples/enums.am"
            return 1;
        } else {
            #line 27 "tests/samples/enums.am"
            return 0;
        }
    }
    #line 29 "tests/samples/enums.am"
    return 0;
}

void Tests_Program_Main(code_string* args) {
    #line 33 "tests/samples/enums.am"
    Tests_Direction d = Tests_Direction_North;
    #line 34 "tests/samples/enums.am"
    code_string name = Tests_Program_DirectionName(d);
    #line 35 "tests/samples/enums.am"
    Console_WriteLine(code_string_concat(code_string_concat("", "Direction: "), (name ? name : "")));
    #line 37 "tests/samples/enums.am"
    Tests_Season s1 = Tests_Season_Summer;
    #line 38 "tests/samples/enums.am"
    Tests_Season s2 = Tests_Season_Winter;
    #line 39 "tests/samples/enums.am"
    code_bool w1 = Tests_Program_IsWarm(s1);
    #line 40 "tests/samples/enums.am"
    code_bool w2 = Tests_Program_IsWarm(s2);
    #line 41 "tests/samples/enums.am"
    Console_WriteLine(code_string_concat(code_string_concat("", "Summer warm: "), (w1 ? "true" : "false")));
    #line 42 "tests/samples/enums.am"
    Console_WriteLine(code_string_concat(code_string_concat("", "Winter warm: "), (w2 ? "true" : "false")));
    #line 45 "tests/samples/enums.am"
    code_bool isNorth = d == Tests_Direction_North;
    #line 46 "tests/samples/enums.am"
    Console_WriteLine(code_string_concat(code_string_concat("", "isNorth: "), (isNorth ? "true" : "false")));
    #line 49 "tests/samples/enums.am"
    code_string s = Tests_Program_DirectionName(Tests_Direction_East);
    #line 50 "tests/samples/enums.am"
    Console_WriteLine(code_string_concat(code_string_concat("", "East: "), (s ? s : "")));
}


int main(int argc, char** argv) {
    GC_INIT();
    code_runtime_init_args(argc, argv);
    Tests_Program_Main((code_string*)argv);
    return code_exit_code;
}
