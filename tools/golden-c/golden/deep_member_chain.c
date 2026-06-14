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

typedef struct _Demo_Holder Demo_Holder;
typedef struct _Demo_Wrapper Demo_Wrapper;
typedef struct _Demo_Router Demo_Router;
typedef struct _Demo_WebApp Demo_WebApp;
typedef struct _Demo_Store Demo_Store;
typedef struct _Demo_Hub Demo_Hub;
typedef struct _Demo_Leaf Demo_Leaf;
typedef struct _Demo_Mid Demo_Mid;
typedef struct _Demo_Root Demo_Root;
typedef struct _Demo_Program Demo_Program;
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

Demo_Holder* Demo_Holder_new();
Demo_Wrapper* Demo_Wrapper_new();
Demo_Router* Demo_Router_new();
Demo_WebApp* Demo_WebApp_new();
Demo_Store* Demo_Store_new();
Demo_Hub* Demo_Hub_new();
Demo_Leaf* Demo_Leaf_new();
Demo_Mid* Demo_Mid_new();
Demo_Root* Demo_Root_new();
Demo_Program* Demo_Program_new();
code_string Demo_Leaf_Hit(Demo_Leaf* self);
i64 Demo_Leaf_Plus(Demo_Leaf* self, i64 a, i64 b);
code_string Demo_Root_ChainThis(Demo_Root* self);
i64 Demo_Root_ChainThisArgs(Demo_Root* self);
void Demo_Program_Main();
struct _Demo_Holder {
    AmalgameList* Vals;
};


Demo_Holder* Demo_Holder_new() {
    Demo_Holder* self = (Demo_Holder*) GC_MALLOC(sizeof(Demo_Holder));
    #line 16 "tests/samples/deep_member_chain.am"
    self->Vals = ({ AmalgameList* __ll = AmalgameList_new(); AmalgameList_add(__ll, (void*)(intptr_t)(1LL)); AmalgameList_add(__ll, (void*)(intptr_t)(2LL)); AmalgameList_add(__ll, (void*)(intptr_t)(3LL)); __ll; });
    return self;
}

struct _Demo_Wrapper {
    Demo_Holder* Inner;
};


Demo_Wrapper* Demo_Wrapper_new() {
    Demo_Wrapper* self = (Demo_Wrapper*) GC_MALLOC(sizeof(Demo_Wrapper));
    #line 21 "tests/samples/deep_member_chain.am"
    self->Inner = Demo_Holder_new();
    return self;
}

struct _Demo_Router {
    AmalgameList* Routes;
};


Demo_Router* Demo_Router_new() {
    Demo_Router* self = (Demo_Router*) GC_MALLOC(sizeof(Demo_Router));
    #line 29 "tests/samples/deep_member_chain.am"
    self->Routes = ({ AmalgameList* __ll = AmalgameList_new(); AmalgameList_add(__ll, (void*)(intptr_t)(1LL)); AmalgameList_add(__ll, (void*)(intptr_t)(2LL)); AmalgameList_add(__ll, (void*)(intptr_t)(3LL)); AmalgameList_add(__ll, (void*)(intptr_t)(4LL)); __ll; });
    return self;
}

struct _Demo_WebApp {
    Demo_Router* Routes;
};


Demo_WebApp* Demo_WebApp_new() {
    Demo_WebApp* self = (Demo_WebApp*) GC_MALLOC(sizeof(Demo_WebApp));
    #line 33 "tests/samples/deep_member_chain.am"
    self->Routes = Demo_Router_new();
    return self;
}

struct _Demo_Store {
    AmalgameMap* Items;
};


Demo_Store* Demo_Store_new() {
    Demo_Store* self = (Demo_Store*) GC_MALLOC(sizeof(Demo_Store));
    #line 40 "tests/samples/deep_member_chain.am"
    self->Items = AmalgameMap_new();
    #line 41 "tests/samples/deep_member_chain.am"
    AmalgameMap_set(self->Items, "a", (void*)(intptr_t)(10LL));
    return self;
}

struct _Demo_Hub {
    Demo_Store* Backend;
};


Demo_Hub* Demo_Hub_new() {
    Demo_Hub* self = (Demo_Hub*) GC_MALLOC(sizeof(Demo_Hub));
    #line 46 "tests/samples/deep_member_chain.am"
    self->Backend = Demo_Store_new();
    return self;
}

struct _Demo_Leaf {
};

code_string Demo_Leaf_Hit(Demo_Leaf* self);
i64 Demo_Leaf_Plus(Demo_Leaf* self, i64 a, i64 b);

Demo_Leaf* Demo_Leaf_new() {
    Demo_Leaf* self = (Demo_Leaf*) GC_MALLOC(sizeof(Demo_Leaf));
    return self;
}

code_string Demo_Leaf_Hit(Demo_Leaf* self) {
    #line 58 "tests/samples/deep_member_chain.am"
    return "hit";
}

i64 Demo_Leaf_Plus(Demo_Leaf* self, i64 a, i64 b) {
    #line 59 "tests/samples/deep_member_chain.am"
    return a + b;
}

struct _Demo_Mid {
    Demo_Leaf* L;
};


Demo_Mid* Demo_Mid_new() {
    Demo_Mid* self = (Demo_Mid*) GC_MALLOC(sizeof(Demo_Mid));
    #line 63 "tests/samples/deep_member_chain.am"
    self->L = Demo_Leaf_new();
    return self;
}

struct _Demo_Root {
    Demo_Mid* M;
};

code_string Demo_Root_ChainThis(Demo_Root* self);
i64 Demo_Root_ChainThisArgs(Demo_Root* self);

Demo_Root* Demo_Root_new() {
    Demo_Root* self = (Demo_Root*) GC_MALLOC(sizeof(Demo_Root));
    #line 67 "tests/samples/deep_member_chain.am"
    self->M = Demo_Mid_new();
    return self;
}

code_string Demo_Root_ChainThis(Demo_Root* self) {
    #line 69 "tests/samples/deep_member_chain.am"
    return Demo_Leaf_Hit(self->M->L);
}

i64 Demo_Root_ChainThisArgs(Demo_Root* self) {
    #line 71 "tests/samples/deep_member_chain.am"
    return Demo_Leaf_Plus(self->M->L, 2LL, 3LL);
}

struct _Demo_Program {
};

void Demo_Program_Main();

Demo_Program* Demo_Program_new() {
    Demo_Program* self = (Demo_Program*) GC_MALLOC(sizeof(Demo_Program));
    return self;
}

void Demo_Program_Main() {
    #line 77 "tests/samples/deep_member_chain.am"
    Demo_Wrapper* w = Demo_Wrapper_new();
    #line 78 "tests/samples/deep_member_chain.am"
    if (AmalgameList_count(w->Inner->Vals) == 3LL) {
        #line 79 "tests/samples/deep_member_chain.am"
        Console_WriteLine("[PASS] deep list distinct names");
    } else {
        #line 81 "tests/samples/deep_member_chain.am"
        Console_WriteLine("[FAIL] deep list distinct names");
    }
    #line 86 "tests/samples/deep_member_chain.am"
    Demo_WebApp* app = Demo_WebApp_new();
    #line 87 "tests/samples/deep_member_chain.am"
    if (AmalgameList_count(app->Routes->Routes) == 4LL) {
        #line 88 "tests/samples/deep_member_chain.am"
        Console_WriteLine("[PASS] deep list identical names");
    } else {
        #line 90 "tests/samples/deep_member_chain.am"
        Console_WriteLine("[FAIL] deep list identical names");
    }
    #line 95 "tests/samples/deep_member_chain.am"
    Demo_Hub* h = Demo_Hub_new();
    #line 96 "tests/samples/deep_member_chain.am"
    if (AmalgameMap_has(h->Backend->Items, "a") && ((void*)AmalgameMap_get(h->Backend->Items, "a") == 10LL)) {
        #line 97 "tests/samples/deep_member_chain.am"
        Console_WriteLine("[PASS] deep map chain");
    } else {
        #line 99 "tests/samples/deep_member_chain.am"
        Console_WriteLine("[FAIL] deep map chain");
    }
    #line 103 "tests/samples/deep_member_chain.am"
    Demo_Root* r = Demo_Root_new();
    #line 104 "tests/samples/deep_member_chain.am"
    if (code_string_equals(Demo_Root_ChainThis(r), "hit")) {
        #line 105 "tests/samples/deep_member_chain.am"
        Console_WriteLine("[PASS] deep chain this.A.B.method()");
    } else {
        #line 107 "tests/samples/deep_member_chain.am"
        Console_WriteLine("[FAIL] deep chain this.A.B.method()");
    }
    #line 109 "tests/samples/deep_member_chain.am"
    if (Demo_Root_ChainThisArgs(r) == 5LL) {
        #line 110 "tests/samples/deep_member_chain.am"
        Console_WriteLine("[PASS] deep chain this.A.B.method(args)");
    } else {
        #line 112 "tests/samples/deep_member_chain.am"
        Console_WriteLine("[FAIL] deep chain this.A.B.method(args)");
    }
    #line 116 "tests/samples/deep_member_chain.am"
    Demo_Root* rt = Demo_Root_new();
    #line 117 "tests/samples/deep_member_chain.am"
    if (code_string_equals(Demo_Leaf_Hit(rt->M->L), "hit")) {
        #line 118 "tests/samples/deep_member_chain.am"
        Console_WriteLine("[PASS] deep chain ident.A.B.method()");
    } else {
        #line 120 "tests/samples/deep_member_chain.am"
        Console_WriteLine("[FAIL] deep chain ident.A.B.method()");
    }
}


int main(int argc, char** argv) {
    GC_INIT();
    code_runtime_init_args(argc, argv);
    Demo_Program_Main();
    return code_exit_code;
}
