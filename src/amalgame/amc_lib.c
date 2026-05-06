#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "_runtime.h"
#include "Amalgame_String.h"
#include "Amalgame_Collections.h"
#include "Amalgame_IO.h"
#include "Amalgame_Math.h"
#include "Amalgame_Net.h"
#include "Amalgame_Console.h"

typedef enum _Amalgame_Compiler_TokenType Amalgame_Compiler_TokenType;
typedef struct _Amalgame_Compiler_Token Amalgame_Compiler_Token;
typedef enum _Amalgame_Compiler_NodeKind Amalgame_Compiler_NodeKind;
typedef struct _Amalgame_Compiler_AstNode Amalgame_Compiler_AstNode;
typedef struct _Amalgame_Compiler_Ast Amalgame_Compiler_Ast;
typedef struct _Amalgame_Compiler_Lexer Amalgame_Compiler_Lexer;
typedef struct _Amalgame_Compiler_Parser Amalgame_Compiler_Parser;
typedef struct _Amalgame_Compiler_Emitter Amalgame_Compiler_Emitter;
typedef struct _Amalgame_Compiler_CGen Amalgame_Compiler_CGen;
typedef enum _Amalgame_Compiler_SymKind Amalgame_Compiler_SymKind;
typedef struct _Amalgame_Compiler_Symbol Amalgame_Compiler_Symbol;
typedef struct _Amalgame_Compiler_SymbolTable Amalgame_Compiler_SymbolTable;
typedef struct _Amalgame_Compiler_Resolver Amalgame_Compiler_Resolver;
typedef struct _Amalgame_Compiler_MemberTable Amalgame_Compiler_MemberTable;
typedef struct _Amalgame_Compiler_Scope Amalgame_Compiler_Scope;
typedef struct _Amalgame_Compiler_FullResolver Amalgame_Compiler_FullResolver;
typedef struct _Amalgame_Compiler_DiagnosticFormatter Amalgame_Compiler_DiagnosticFormatter;
typedef struct _Amalgame_Compiler_TypeError Amalgame_Compiler_TypeError;
typedef struct _Amalgame_Compiler_TypeCheckResult Amalgame_Compiler_TypeCheckResult;
typedef struct _Amalgame_Compiler_TypeChecker Amalgame_Compiler_TypeChecker;
typedef struct _Amalgame_Compiler_AmalgameCompiler Amalgame_Compiler_AmalgameCompiler;
typedef struct _Amalgame_Compiler_AmcEntry Amalgame_Compiler_AmcEntry;

enum _Amalgame_Compiler_TokenType {
    Amalgame_Compiler_TokenType_INTEGER,
    Amalgame_Compiler_TokenType_FLOAT,
    Amalgame_Compiler_TokenType_STRING,
    Amalgame_Compiler_TokenType_BOOL,
    Amalgame_Compiler_TokenType_NULL,
    Amalgame_Compiler_TokenType_IDENTIFIER,
    Amalgame_Compiler_TokenType_KW_LET,
    Amalgame_Compiler_TokenType_KW_VAR,
    Amalgame_Compiler_TokenType_KW_IF,
    Amalgame_Compiler_TokenType_KW_ELSE,
    Amalgame_Compiler_TokenType_KW_FOR,
    Amalgame_Compiler_TokenType_KW_WHILE,
    Amalgame_Compiler_TokenType_KW_RETURN,
    Amalgame_Compiler_TokenType_KW_CLASS,
    Amalgame_Compiler_TokenType_KW_PUBLIC,
    Amalgame_Compiler_TokenType_KW_PRIVATE,
    Amalgame_Compiler_TokenType_KW_STATIC,
    Amalgame_Compiler_TokenType_KW_NEW,
    Amalgame_Compiler_TokenType_KW_THIS,
    Amalgame_Compiler_TokenType_KW_NULL,
    Amalgame_Compiler_TokenType_KW_TRUE,
    Amalgame_Compiler_TokenType_KW_FALSE,
    Amalgame_Compiler_TokenType_KW_IMPORT,
    Amalgame_Compiler_TokenType_KW_NAMESPACE,
    Amalgame_Compiler_TokenType_KW_ENUM,
    Amalgame_Compiler_TokenType_KW_MATCH,
    Amalgame_Compiler_TokenType_KW_IN,
    Amalgame_Compiler_TokenType_KW_IS,
    Amalgame_Compiler_TokenType_KW_AS,
    Amalgame_Compiler_TokenType_KW_VOID,
    Amalgame_Compiler_TokenType_KW_INT,
    Amalgame_Compiler_TokenType_KW_STRING_TYPE,
    Amalgame_Compiler_TokenType_KW_BOOL_TYPE,
    Amalgame_Compiler_TokenType_KW_FLOAT_TYPE,
    Amalgame_Compiler_TokenType_KW_TRY,
    Amalgame_Compiler_TokenType_KW_CATCH,
    Amalgame_Compiler_TokenType_KW_THROW,
    Amalgame_Compiler_TokenType_KW_FINALLY,
    Amalgame_Compiler_TokenType_KW_INTERFACE,
    Amalgame_Compiler_TokenType_KW_RECORD,
    Amalgame_Compiler_TokenType_KW_FUNC,
    Amalgame_Compiler_TokenType_OP_PLUS,
    Amalgame_Compiler_TokenType_OP_MINUS,
    Amalgame_Compiler_TokenType_OP_STAR,
    Amalgame_Compiler_TokenType_OP_SLASH,
    Amalgame_Compiler_TokenType_OP_PERCENT,
    Amalgame_Compiler_TokenType_OP_EQ,
    Amalgame_Compiler_TokenType_OP_EQEQ,
    Amalgame_Compiler_TokenType_OP_NEQ,
    Amalgame_Compiler_TokenType_OP_LT,
    Amalgame_Compiler_TokenType_OP_GT,
    Amalgame_Compiler_TokenType_OP_LTE,
    Amalgame_Compiler_TokenType_OP_GTE,
    Amalgame_Compiler_TokenType_OP_AND,
    Amalgame_Compiler_TokenType_OP_OR,
    Amalgame_Compiler_TokenType_OP_NOT,
    Amalgame_Compiler_TokenType_OP_ARROW,
    Amalgame_Compiler_TokenType_OP_THIN_ARROW,
    Amalgame_Compiler_TokenType_OP_DOT,
    Amalgame_Compiler_TokenType_OP_DOTDOT,
    Amalgame_Compiler_TokenType_OP_COALESCE,
    Amalgame_Compiler_TokenType_LPAREN,
    Amalgame_Compiler_TokenType_RPAREN,
    Amalgame_Compiler_TokenType_LBRACE,
    Amalgame_Compiler_TokenType_RBRACE,
    Amalgame_Compiler_TokenType_LBRACKET,
    Amalgame_Compiler_TokenType_RBRACKET,
    Amalgame_Compiler_TokenType_COMMA,
    Amalgame_Compiler_TokenType_COLON,
    Amalgame_Compiler_TokenType_SEMICOLON,
    Amalgame_Compiler_TokenType_AT,
    Amalgame_Compiler_TokenType_NEWLINE,
    Amalgame_Compiler_TokenType_EOF,
    Amalgame_Compiler_TokenType_UNKNOWN
};

struct _Amalgame_Compiler_Token {
    Amalgame_Compiler_TokenType Type;
    code_string Value;
    i64 Line;
    i64 Column;
    code_string Filename;
};

code_string Amalgame_Compiler_Token_ToString(Amalgame_Compiler_Token* self);
code_bool Amalgame_Compiler_Token_IsKeyword(Amalgame_Compiler_Token* self);

Amalgame_Compiler_Token* Amalgame_Compiler_Token_new(Amalgame_Compiler_TokenType t, code_string value, i64 line, i64 col, code_string file) {
    Amalgame_Compiler_Token* self = (Amalgame_Compiler_Token*) GC_MALLOC(sizeof(Amalgame_Compiler_Token));
    self->Type = t;
    self->Value = value;
    self->Line = line;
    self->Column = col;
    self->Filename = file;
    return self;
}

code_string Amalgame_Compiler_Token_ToString(Amalgame_Compiler_Token* self) {
    (void)self;
    return code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("", "Token("), String_FromInt((i64)self->Type)), ", '"), (self->Value ? self->Value : "")), "', "), String_FromInt(self->Line)), ":"), String_FromInt(self->Column)), ")");
}

code_bool Amalgame_Compiler_Token_IsKeyword(Amalgame_Compiler_Token* self) {
    (void)self;
    Amalgame_Compiler_TokenType __attribute__((unused)) v = self->Type;
    return v == Amalgame_Compiler_TokenType_KW_LET || v == Amalgame_Compiler_TokenType_KW_VAR || v == Amalgame_Compiler_TokenType_KW_IF || v == Amalgame_Compiler_TokenType_KW_ELSE || v == Amalgame_Compiler_TokenType_KW_FOR || v == Amalgame_Compiler_TokenType_KW_WHILE || v == Amalgame_Compiler_TokenType_KW_RETURN || v == Amalgame_Compiler_TokenType_KW_CLASS || v == Amalgame_Compiler_TokenType_KW_PUBLIC || v == Amalgame_Compiler_TokenType_KW_STATIC || v == Amalgame_Compiler_TokenType_KW_NEW || v == Amalgame_Compiler_TokenType_KW_ENUM || v == Amalgame_Compiler_TokenType_KW_MATCH || v == Amalgame_Compiler_TokenType_KW_IN;
}

enum _Amalgame_Compiler_NodeKind {
    Amalgame_Compiler_NodeKind_PROGRAM,
    Amalgame_Compiler_NodeKind_CLASS_DECL,
    Amalgame_Compiler_NodeKind_METHOD_DECL,
    Amalgame_Compiler_NodeKind_FIELD_DECL,
    Amalgame_Compiler_NodeKind_PARAM,
    Amalgame_Compiler_NodeKind_ENUM_DECL,
    Amalgame_Compiler_NodeKind_BLOCK,
    Amalgame_Compiler_NodeKind_VAR_DECL,
    Amalgame_Compiler_NodeKind_ASSIGN,
    Amalgame_Compiler_NodeKind_RETURN_STMT,
    Amalgame_Compiler_NodeKind_IF_STMT,
    Amalgame_Compiler_NodeKind_WHILE_STMT,
    Amalgame_Compiler_NodeKind_FOR_IN_STMT,
    Amalgame_Compiler_NodeKind_BREAK_STMT,
    Amalgame_Compiler_NodeKind_CONTINUE_STMT,
    Amalgame_Compiler_NodeKind_BINARY,
    Amalgame_Compiler_NodeKind_UNARY,
    Amalgame_Compiler_NodeKind_CALL,
    Amalgame_Compiler_NodeKind_MEMBER,
    Amalgame_Compiler_NodeKind_IDENTIFIER,
    Amalgame_Compiler_NodeKind_LITERAL_INT,
    Amalgame_Compiler_NodeKind_LITERAL_FLOAT,
    Amalgame_Compiler_NodeKind_LITERAL_STRING,
    Amalgame_Compiler_NodeKind_LITERAL_BOOL,
    Amalgame_Compiler_NodeKind_LITERAL_NULL,
    Amalgame_Compiler_NodeKind_NEW_EXPR,
    Amalgame_Compiler_NodeKind_THIS_EXPR,
    Amalgame_Compiler_NodeKind_INDEX_EXPR
};

struct _Amalgame_Compiler_AstNode {
    Amalgame_Compiler_NodeKind Kind;
    i64 Line;
    i64 Column;
    code_string Name;
    code_string Str;
    code_string Str2;
    code_bool Flag;
    code_bool Flag2;
    Amalgame_Compiler_AstNode* Left;
    Amalgame_Compiler_AstNode* Right;
    Amalgame_Compiler_AstNode* Cond;
    Amalgame_Compiler_AstNode* Body;
    Amalgame_Compiler_AstNode* Else;
    AmalgameList* Children;
    AmalgameList* Params;
    AmalgameList* Args;
};


Amalgame_Compiler_AstNode* Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind kind, i64 line, i64 col) {
    Amalgame_Compiler_AstNode* self = (Amalgame_Compiler_AstNode*) GC_MALLOC(sizeof(Amalgame_Compiler_AstNode));
    self->Kind = kind;
    self->Line = line;
    self->Column = col;
    self->Name = "";
    self->Str = "";
    self->Str2 = "";
    self->Flag = 0;
    self->Flag2 = 0;
    self->Left = NULL;
    self->Right = NULL;
    self->Cond = NULL;
    self->Body = NULL;
    self->Else = NULL;
    self->Children = AmalgameList_new();
    self->Params = AmalgameList_new();
    self->Args = AmalgameList_new();
    return self;
}

struct _Amalgame_Compiler_Ast {
};

Amalgame_Compiler_AstNode* Amalgame_Compiler_Ast_Program(i64 line, i64 col);
Amalgame_Compiler_AstNode* Amalgame_Compiler_Ast_Class(code_string name, i64 line, i64 col);
Amalgame_Compiler_AstNode* Amalgame_Compiler_Ast_Method(code_string name, code_string retType, i64 line, i64 col);
Amalgame_Compiler_AstNode* Amalgame_Compiler_Ast_Param(code_string name, code_string typeName, i64 line, i64 col);
Amalgame_Compiler_AstNode* Amalgame_Compiler_Ast_Block(i64 line, i64 col);
Amalgame_Compiler_AstNode* Amalgame_Compiler_Ast_VarDecl(code_string name, code_bool isMut, i64 line, i64 col);
Amalgame_Compiler_AstNode* Amalgame_Compiler_Ast_Return(Amalgame_Compiler_AstNode* value, i64 line, i64 col);
Amalgame_Compiler_AstNode* Amalgame_Compiler_Ast_If(Amalgame_Compiler_AstNode* cond, Amalgame_Compiler_AstNode* then, i64 line, i64 col);
Amalgame_Compiler_AstNode* Amalgame_Compiler_Ast_Binary(Amalgame_Compiler_AstNode* left, code_string op, Amalgame_Compiler_AstNode* right, i64 line, i64 col);
Amalgame_Compiler_AstNode* Amalgame_Compiler_Ast_Call(Amalgame_Compiler_AstNode* callee, i64 line, i64 col);
Amalgame_Compiler_AstNode* Amalgame_Compiler_Ast_Member(Amalgame_Compiler_AstNode* target, code_string member, i64 line, i64 col);
Amalgame_Compiler_AstNode* Amalgame_Compiler_Ast_Ident(code_string name, i64 line, i64 col);
Amalgame_Compiler_AstNode* Amalgame_Compiler_Ast_IntLit(code_string raw, i64 line, i64 col);
Amalgame_Compiler_AstNode* Amalgame_Compiler_Ast_StrLit(code_string raw, i64 line, i64 col);
Amalgame_Compiler_AstNode* Amalgame_Compiler_Ast_BoolLit(code_bool val, i64 line, i64 col);
Amalgame_Compiler_AstNode* Amalgame_Compiler_Ast_FloatLit(code_string raw, i64 line, i64 col);
Amalgame_Compiler_AstNode* Amalgame_Compiler_Ast_NewExpr(code_string typeName, i64 line, i64 col);
Amalgame_Compiler_AstNode* Amalgame_Compiler_Ast_This(i64 line, i64 col);

Amalgame_Compiler_Ast* Amalgame_Compiler_Ast_new() {
    Amalgame_Compiler_Ast* self = (Amalgame_Compiler_Ast*) GC_MALLOC(sizeof(Amalgame_Compiler_Ast));
    return self;
}

Amalgame_Compiler_AstNode* Amalgame_Compiler_Ast_Program(i64 line, i64 col) {
    (void)line;
    (void)col;
    return Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_PROGRAM, line, col);
}

Amalgame_Compiler_AstNode* Amalgame_Compiler_Ast_Class(code_string name, i64 line, i64 col) {
    (void)name;
    (void)line;
    (void)col;
    Amalgame_Compiler_AstNode* __attribute__((unused)) n = Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_CLASS_DECL, line, col);
    n->Name = name;
    return n;
}

Amalgame_Compiler_AstNode* Amalgame_Compiler_Ast_Method(code_string name, code_string retType, i64 line, i64 col) {
    (void)name;
    (void)retType;
    (void)line;
    (void)col;
    Amalgame_Compiler_AstNode* __attribute__((unused)) n = Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_METHOD_DECL, line, col);
    n->Name = name;
    n->Str = retType;
    return n;
}

Amalgame_Compiler_AstNode* Amalgame_Compiler_Ast_Param(code_string name, code_string typeName, i64 line, i64 col) {
    (void)name;
    (void)typeName;
    (void)line;
    (void)col;
    Amalgame_Compiler_AstNode* __attribute__((unused)) n = Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_PARAM, line, col);
    n->Name = name;
    n->Str = typeName;
    return n;
}

Amalgame_Compiler_AstNode* Amalgame_Compiler_Ast_Block(i64 line, i64 col) {
    (void)line;
    (void)col;
    return Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_BLOCK, line, col);
}

Amalgame_Compiler_AstNode* Amalgame_Compiler_Ast_VarDecl(code_string name, code_bool isMut, i64 line, i64 col) {
    (void)name;
    (void)isMut;
    (void)line;
    (void)col;
    Amalgame_Compiler_AstNode* __attribute__((unused)) n = Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_VAR_DECL, line, col);
    n->Name = name;
    n->Flag = isMut;
    return n;
}

Amalgame_Compiler_AstNode* Amalgame_Compiler_Ast_Return(Amalgame_Compiler_AstNode* value, i64 line, i64 col) {
    (void)value;
    (void)line;
    (void)col;
    Amalgame_Compiler_AstNode* __attribute__((unused)) n = Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_RETURN_STMT, line, col);
    n->Left = value;
    return n;
}

Amalgame_Compiler_AstNode* Amalgame_Compiler_Ast_If(Amalgame_Compiler_AstNode* cond, Amalgame_Compiler_AstNode* then, i64 line, i64 col) {
    (void)cond;
    (void)then;
    (void)line;
    (void)col;
    Amalgame_Compiler_AstNode* __attribute__((unused)) n = Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_IF_STMT, line, col);
    n->Cond = cond;
    n->Body = then;
    return n;
}

Amalgame_Compiler_AstNode* Amalgame_Compiler_Ast_Binary(Amalgame_Compiler_AstNode* left, code_string op, Amalgame_Compiler_AstNode* right, i64 line, i64 col) {
    (void)left;
    (void)op;
    (void)right;
    (void)line;
    (void)col;
    Amalgame_Compiler_AstNode* __attribute__((unused)) n = Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_BINARY, line, col);
    n->Left = left;
    n->Str = op;
    n->Right = right;
    return n;
}

Amalgame_Compiler_AstNode* Amalgame_Compiler_Ast_Call(Amalgame_Compiler_AstNode* callee, i64 line, i64 col) {
    (void)callee;
    (void)line;
    (void)col;
    Amalgame_Compiler_AstNode* __attribute__((unused)) n = Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_CALL, line, col);
    n->Left = callee;
    return n;
}

Amalgame_Compiler_AstNode* Amalgame_Compiler_Ast_Member(Amalgame_Compiler_AstNode* target, code_string member, i64 line, i64 col) {
    (void)target;
    (void)member;
    (void)line;
    (void)col;
    Amalgame_Compiler_AstNode* __attribute__((unused)) n = Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_MEMBER, line, col);
    n->Left = target;
    n->Name = member;
    return n;
}

Amalgame_Compiler_AstNode* Amalgame_Compiler_Ast_Ident(code_string name, i64 line, i64 col) {
    (void)name;
    (void)line;
    (void)col;
    Amalgame_Compiler_AstNode* __attribute__((unused)) n = Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_IDENTIFIER, line, col);
    n->Name = name;
    return n;
}

Amalgame_Compiler_AstNode* Amalgame_Compiler_Ast_IntLit(code_string raw, i64 line, i64 col) {
    (void)raw;
    (void)line;
    (void)col;
    Amalgame_Compiler_AstNode* __attribute__((unused)) n = Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_LITERAL_INT, line, col);
    n->Str = raw;
    return n;
}

Amalgame_Compiler_AstNode* Amalgame_Compiler_Ast_StrLit(code_string raw, i64 line, i64 col) {
    (void)raw;
    (void)line;
    (void)col;
    Amalgame_Compiler_AstNode* __attribute__((unused)) n = Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_LITERAL_STRING, line, col);
    n->Str = raw;
    return n;
}

Amalgame_Compiler_AstNode* Amalgame_Compiler_Ast_BoolLit(code_bool val, i64 line, i64 col) {
    (void)val;
    (void)line;
    (void)col;
    Amalgame_Compiler_AstNode* __attribute__((unused)) n = Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_LITERAL_BOOL, line, col);
    n->Flag = val;
    return n;
}

Amalgame_Compiler_AstNode* Amalgame_Compiler_Ast_FloatLit(code_string raw, i64 line, i64 col) {
    (void)raw;
    (void)line;
    (void)col;
    Amalgame_Compiler_AstNode* __attribute__((unused)) n = Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_LITERAL_FLOAT, line, col);
    n->Str = raw;
    return n;
}

Amalgame_Compiler_AstNode* Amalgame_Compiler_Ast_NewExpr(code_string typeName, i64 line, i64 col) {
    (void)typeName;
    (void)line;
    (void)col;
    Amalgame_Compiler_AstNode* __attribute__((unused)) n = Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_NEW_EXPR, line, col);
    n->Name = typeName;
    return n;
}

Amalgame_Compiler_AstNode* Amalgame_Compiler_Ast_This(i64 line, i64 col) {
    (void)line;
    (void)col;
    return Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_THIS_EXPR, line, col);
}

struct _Amalgame_Compiler_Lexer {
    code_string Source;
    i64 Pos;
    i64 Line;
    i64 Column;
    code_string Filename;
    AmalgameList* Tokens;
};

static code_bool Amalgame_Compiler_Lexer_IsSpace(Amalgame_Compiler_Lexer* self, code_string c);
static code_bool Amalgame_Compiler_Lexer_IsDigit(Amalgame_Compiler_Lexer* self, code_string c);
static code_bool Amalgame_Compiler_Lexer_IsAlpha(Amalgame_Compiler_Lexer* self, code_string c);
static code_bool Amalgame_Compiler_Lexer_IsAlphaNum(Amalgame_Compiler_Lexer* self, code_string c);
static code_string Amalgame_Compiler_Lexer_CharAt(Amalgame_Compiler_Lexer* self, i64 i);
static void Amalgame_Compiler_Lexer_AddToken(Amalgame_Compiler_Lexer* self, Amalgame_Compiler_TokenType t, code_string value);
static code_string Amalgame_Compiler_Lexer_Advance(Amalgame_Compiler_Lexer* self);
static void Amalgame_Compiler_Lexer_SkipWhitespace(Amalgame_Compiler_Lexer* self);
AmalgameList* Amalgame_Compiler_Lexer_Tokenize(Amalgame_Compiler_Lexer* self);
static void Amalgame_Compiler_Lexer_ReadString(Amalgame_Compiler_Lexer* self);
static void Amalgame_Compiler_Lexer_ReadNumber(Amalgame_Compiler_Lexer* self);
static void Amalgame_Compiler_Lexer_ReadIdentifier(Amalgame_Compiler_Lexer* self);
static Amalgame_Compiler_TokenType Amalgame_Compiler_Lexer_LookupKeyword(Amalgame_Compiler_Lexer* self, code_string word);
static void Amalgame_Compiler_Lexer_ReadSymbol(Amalgame_Compiler_Lexer* self);

Amalgame_Compiler_Lexer* Amalgame_Compiler_Lexer_new(code_string source, code_string filename) {
    Amalgame_Compiler_Lexer* self = (Amalgame_Compiler_Lexer*) GC_MALLOC(sizeof(Amalgame_Compiler_Lexer));
    self->Source = source;
    self->Filename = filename;
    self->Pos = 0;
    self->Line = 1;
    self->Column = 1;
    self->Tokens = AmalgameList_new();
    return self;
}

static code_bool Amalgame_Compiler_Lexer_IsSpace(Amalgame_Compiler_Lexer* self, code_string c) {
    (void)self;
    (void)c;
    return code_string_equals(c, " ") || code_string_equals(c, "\t") || code_string_equals(c, "");
}

static code_bool Amalgame_Compiler_Lexer_IsDigit(Amalgame_Compiler_Lexer* self, code_string c) {
    (void)self;
    (void)c;
    return String_Contains("0123456789", c);
}

static code_bool Amalgame_Compiler_Lexer_IsAlpha(Amalgame_Compiler_Lexer* self, code_string c) {
    (void)self;
    (void)c;
    return String_Contains("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_", c);
}

static code_bool Amalgame_Compiler_Lexer_IsAlphaNum(Amalgame_Compiler_Lexer* self, code_string c) {
    (void)self;
    (void)c;
    return Amalgame_Compiler_Lexer_IsAlpha(self, c) || Amalgame_Compiler_Lexer_IsDigit(self, c);
}

static code_string Amalgame_Compiler_Lexer_CharAt(Amalgame_Compiler_Lexer* self, i64 i) {
    (void)self;
    (void)i;
    if (i < 0 || i >= String_Length(self->Source)) {
        return "";
    }
    return String_Substring(self->Source, i, 1);
}

static void Amalgame_Compiler_Lexer_AddToken(Amalgame_Compiler_Lexer* self, Amalgame_Compiler_TokenType t, code_string value) {
    (void)self;
    (void)t;
    (void)value;
    Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Token_new(t, value, self->Line, self->Column, self->Filename);
    AmalgameList_add(self->Tokens, (void*)(intptr_t)(tok));
}

static code_string Amalgame_Compiler_Lexer_Advance(Amalgame_Compiler_Lexer* self) {
    (void)self;
    code_string __attribute__((unused)) ch = Amalgame_Compiler_Lexer_CharAt(self, self->Pos);
    self->Pos = self->Pos + 1;
    self->Column = self->Column + 1;
    return ch;
}

static void Amalgame_Compiler_Lexer_SkipWhitespace(Amalgame_Compiler_Lexer* self) {
    (void)self;
    while (self->Pos < String_Length(self->Source)) {
        code_string __attribute__((unused)) c = Amalgame_Compiler_Lexer_CharAt(self, self->Pos);
        if (Amalgame_Compiler_Lexer_IsSpace(self, c)) {
            Amalgame_Compiler_Lexer_Advance(self);
        } else if (code_string_equals(c, "/") && code_string_equals(Amalgame_Compiler_Lexer_CharAt(self, self->Pos + 1), "/")) {
            while (self->Pos < String_Length(self->Source) && !code_string_equals(Amalgame_Compiler_Lexer_CharAt(self, self->Pos), "\n")) {
                Amalgame_Compiler_Lexer_Advance(self);
            }
        } else {
            break;
        }
    }
}

AmalgameList* Amalgame_Compiler_Lexer_Tokenize(Amalgame_Compiler_Lexer* self) {
    (void)self;
    while (self->Pos < String_Length(self->Source)) {
        Amalgame_Compiler_Lexer_SkipWhitespace(self);
        if (self->Pos >= String_Length(self->Source)) {
            break;
        }
        code_string __attribute__((unused)) ch = Amalgame_Compiler_Lexer_CharAt(self, self->Pos);
        if (code_string_equals(ch, "\n")) {
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_NEWLINE, "\\n");
            self->Pos = self->Pos + 1;
            self->Line = self->Line + 1;
            self->Column = 1;
        } else if (code_string_equals(ch, "\"")) {
            Amalgame_Compiler_Lexer_ReadString(self);
        } else if (Amalgame_Compiler_Lexer_IsDigit(self, ch)) {
            Amalgame_Compiler_Lexer_ReadNumber(self);
        } else if (Amalgame_Compiler_Lexer_IsAlpha(self, ch)) {
            Amalgame_Compiler_Lexer_ReadIdentifier(self);
        } else if (Amalgame_Compiler_Lexer_IsSpace(self, ch)) {
            Amalgame_Compiler_Lexer_Advance(self);
        } else {
            Amalgame_Compiler_Lexer_ReadSymbol(self);
        }
    }
    Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_EOF, "");
    return self->Tokens;
}

static void Amalgame_Compiler_Lexer_ReadString(Amalgame_Compiler_Lexer* self) {
    (void)self;
    Amalgame_Compiler_Lexer_Advance(self);
    code_string __attribute__((unused)) value = "";
    while (self->Pos < String_Length(self->Source) && !code_string_equals(Amalgame_Compiler_Lexer_CharAt(self, self->Pos), "\"")) {
        code_string __attribute__((unused)) c = Amalgame_Compiler_Lexer_Advance(self);
        if (code_string_equals(c, "\\")) {
            code_string __attribute__((unused)) esc = Amalgame_Compiler_Lexer_Advance(self);
            if (code_string_equals(esc, "n")) {
                value = code_string_concat(value, "\n");
            }
            if (code_string_equals(esc, "t")) {
                value = code_string_concat(value, "\t");
            }
            if (code_string_equals(esc, "\"")) {
                value = code_string_concat(value, "\"");
            }
            if (code_string_equals(esc, "\\")) {
                value = code_string_concat(value, "\\");
            }
        } else {
            value = code_string_concat(value, c);
        }
    }
    if (self->Pos < String_Length(self->Source)) {
        Amalgame_Compiler_Lexer_Advance(self);
    }
    Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_STRING, value);
}

static void Amalgame_Compiler_Lexer_ReadNumber(Amalgame_Compiler_Lexer* self) {
    (void)self;
    i64 __attribute__((unused)) startCol = self->Column;
    code_string __attribute__((unused)) value = "";
    code_bool __attribute__((unused)) isFloat = 0;
    while (self->Pos < String_Length(self->Source) && Amalgame_Compiler_Lexer_IsDigit(self, Amalgame_Compiler_Lexer_CharAt(self, self->Pos))) {
        value = code_string_concat(value, Amalgame_Compiler_Lexer_Advance(self));
    }
    if (self->Pos < String_Length(self->Source) && code_string_equals(Amalgame_Compiler_Lexer_CharAt(self, self->Pos), ".") && Amalgame_Compiler_Lexer_IsDigit(self, Amalgame_Compiler_Lexer_CharAt(self, self->Pos + 1))) {
        isFloat = 1;
        value = code_string_concat(value, Amalgame_Compiler_Lexer_Advance(self));
        while (self->Pos < String_Length(self->Source) && Amalgame_Compiler_Lexer_IsDigit(self, Amalgame_Compiler_Lexer_CharAt(self, self->Pos))) {
            value = code_string_concat(value, Amalgame_Compiler_Lexer_Advance(self));
        }
    }
    if (isFloat) {
        Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Token_new(Amalgame_Compiler_TokenType_FLOAT, value, self->Line, startCol, self->Filename);
        AmalgameList_add(self->Tokens, (void*)(intptr_t)(tok));
    } else {
        Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Token_new(Amalgame_Compiler_TokenType_INTEGER, value, self->Line, startCol, self->Filename);
        AmalgameList_add(self->Tokens, (void*)(intptr_t)(tok));
    }
}

static void Amalgame_Compiler_Lexer_ReadIdentifier(Amalgame_Compiler_Lexer* self) {
    (void)self;
    i64 __attribute__((unused)) startCol = self->Column;
    code_string __attribute__((unused)) value = "";
    while (self->Pos < String_Length(self->Source) && Amalgame_Compiler_Lexer_IsAlphaNum(self, Amalgame_Compiler_Lexer_CharAt(self, self->Pos))) {
        value = code_string_concat(value, Amalgame_Compiler_Lexer_Advance(self));
    }
    Amalgame_Compiler_TokenType __attribute__((unused)) tt = Amalgame_Compiler_Lexer_LookupKeyword(self, value);
    Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Token_new(tt, value, self->Line, startCol, self->Filename);
    AmalgameList_add(self->Tokens, (void*)(intptr_t)(tok));
}

static Amalgame_Compiler_TokenType Amalgame_Compiler_Lexer_LookupKeyword(Amalgame_Compiler_Lexer* self, code_string word) {
    (void)self;
    (void)word;
    if (code_string_equals(word, "let")) {
        return Amalgame_Compiler_TokenType_KW_LET;
    }
    if (code_string_equals(word, "var")) {
        return Amalgame_Compiler_TokenType_KW_VAR;
    }
    if (code_string_equals(word, "if")) {
        return Amalgame_Compiler_TokenType_KW_IF;
    }
    if (code_string_equals(word, "else")) {
        return Amalgame_Compiler_TokenType_KW_ELSE;
    }
    if (code_string_equals(word, "for")) {
        return Amalgame_Compiler_TokenType_KW_FOR;
    }
    if (code_string_equals(word, "while")) {
        return Amalgame_Compiler_TokenType_KW_WHILE;
    }
    if (code_string_equals(word, "return")) {
        return Amalgame_Compiler_TokenType_KW_RETURN;
    }
    if (code_string_equals(word, "class")) {
        return Amalgame_Compiler_TokenType_KW_CLASS;
    }
    if (code_string_equals(word, "public")) {
        return Amalgame_Compiler_TokenType_KW_PUBLIC;
    }
    if (code_string_equals(word, "private")) {
        return Amalgame_Compiler_TokenType_KW_PRIVATE;
    }
    if (code_string_equals(word, "static")) {
        return Amalgame_Compiler_TokenType_KW_STATIC;
    }
    if (code_string_equals(word, "new")) {
        return Amalgame_Compiler_TokenType_KW_NEW;
    }
    if (code_string_equals(word, "this")) {
        return Amalgame_Compiler_TokenType_KW_THIS;
    }
    if (code_string_equals(word, "null")) {
        return Amalgame_Compiler_TokenType_KW_NULL;
    }
    if (code_string_equals(word, "true")) {
        return Amalgame_Compiler_TokenType_KW_TRUE;
    }
    if (code_string_equals(word, "false")) {
        return Amalgame_Compiler_TokenType_KW_FALSE;
    }
    if (code_string_equals(word, "import")) {
        return Amalgame_Compiler_TokenType_KW_IMPORT;
    }
    if (code_string_equals(word, "namespace")) {
        return Amalgame_Compiler_TokenType_KW_NAMESPACE;
    }
    if (code_string_equals(word, "enum")) {
        return Amalgame_Compiler_TokenType_KW_ENUM;
    }
    if (code_string_equals(word, "match")) {
        return Amalgame_Compiler_TokenType_KW_MATCH;
    }
    if (code_string_equals(word, "in")) {
        return Amalgame_Compiler_TokenType_KW_IN;
    }
    if (code_string_equals(word, "is")) {
        return Amalgame_Compiler_TokenType_KW_IS;
    }
    if (code_string_equals(word, "as")) {
        return Amalgame_Compiler_TokenType_KW_AS;
    }
    if (code_string_equals(word, "void")) {
        return Amalgame_Compiler_TokenType_KW_VOID;
    }
    if (code_string_equals(word, "int")) {
        return Amalgame_Compiler_TokenType_KW_INT;
    }
    if (code_string_equals(word, "string")) {
        return Amalgame_Compiler_TokenType_KW_STRING_TYPE;
    }
    if (code_string_equals(word, "bool")) {
        return Amalgame_Compiler_TokenType_KW_BOOL_TYPE;
    }
    if (code_string_equals(word, "float")) {
        return Amalgame_Compiler_TokenType_KW_FLOAT_TYPE;
    }
    if (code_string_equals(word, "try")) {
        return Amalgame_Compiler_TokenType_KW_TRY;
    }
    if (code_string_equals(word, "catch")) {
        return Amalgame_Compiler_TokenType_KW_CATCH;
    }
    if (code_string_equals(word, "throw")) {
        return Amalgame_Compiler_TokenType_KW_THROW;
    }
    if (code_string_equals(word, "finally")) {
        return Amalgame_Compiler_TokenType_KW_FINALLY;
    }
    if (code_string_equals(word, "interface")) {
        return Amalgame_Compiler_TokenType_KW_INTERFACE;
    }
    if (code_string_equals(word, "record")) {
        return Amalgame_Compiler_TokenType_KW_RECORD;
    }
    if (code_string_equals(word, "func")) {
        return Amalgame_Compiler_TokenType_KW_FUNC;
    }
    return Amalgame_Compiler_TokenType_IDENTIFIER;
}

static void Amalgame_Compiler_Lexer_ReadSymbol(Amalgame_Compiler_Lexer* self) {
    (void)self;
    i64 __attribute__((unused)) startCol = self->Column;
    code_string __attribute__((unused)) c = Amalgame_Compiler_Lexer_Advance(self);
    code_string __attribute__((unused)) c2 = Amalgame_Compiler_Lexer_CharAt(self, self->Pos);
    if (code_string_equals(c, "+")) {
        Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_PLUS, "+");
    } else if (code_string_equals(c, "%")) {
        Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_PERCENT, "%");
    } else if (code_string_equals(c, "@")) {
        Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_AT, "@");
    } else if (code_string_equals(c, "(")) {
        Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_LPAREN, "(");
    } else if (code_string_equals(c, ")")) {
        Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_RPAREN, ")");
    } else if (code_string_equals(c, "{")) {
        Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_LBRACE, "{");
    } else if (code_string_equals(c, "}")) {
        Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_RBRACE, "}");
    } else if (code_string_equals(c, "[")) {
        Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_LBRACKET, "[");
    } else if (code_string_equals(c, "]")) {
        Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_RBRACKET, "]");
    } else if (code_string_equals(c, ",")) {
        Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_COMMA, ",");
    } else if (code_string_equals(c, ";")) {
        Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_SEMICOLON, ";");
    } else if (code_string_equals(c, "/")) {
        Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_SLASH, "/");
    } else if (code_string_equals(c, "-")) {
        if (code_string_equals(c2, ">")) {
            Amalgame_Compiler_Lexer_Advance(self);
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_THIN_ARROW, "->");
        } else {
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_MINUS, "-");
        }
    } else if (code_string_equals(c, "*")) {
        Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_STAR, "*");
    } else if (code_string_equals(c, "=")) {
        if (code_string_equals(c2, "=")) {
            Amalgame_Compiler_Lexer_Advance(self);
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_EQEQ, "==");
        } else if (code_string_equals(c2, ">")) {
            Amalgame_Compiler_Lexer_Advance(self);
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_ARROW, "=>");
        } else {
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_EQ, "=");
        }
    } else if (code_string_equals(c, "!")) {
        if (code_string_equals(c2, "=")) {
            Amalgame_Compiler_Lexer_Advance(self);
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_NEQ, "!=");
        } else {
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_NOT, "!");
        }
    } else if (code_string_equals(c, "<")) {
        if (code_string_equals(c2, "=")) {
            Amalgame_Compiler_Lexer_Advance(self);
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_LTE, "<=");
        } else {
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_LT, "<");
        }
    } else if (code_string_equals(c, ">")) {
        if (code_string_equals(c2, "=")) {
            Amalgame_Compiler_Lexer_Advance(self);
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_GTE, ">=");
        } else {
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_GT, ">");
        }
    } else if (code_string_equals(c, "&")) {
        if (code_string_equals(c2, "&")) {
            Amalgame_Compiler_Lexer_Advance(self);
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_AND, "&&");
        }
    } else if (code_string_equals(c, "|")) {
        if (code_string_equals(c2, "|")) {
            Amalgame_Compiler_Lexer_Advance(self);
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_OR, "||");
        }
    } else if (code_string_equals(c, ".")) {
        if (code_string_equals(c2, ".")) {
            Amalgame_Compiler_Lexer_Advance(self);
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_DOTDOT, "..");
        } else {
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_DOT, ".");
        }
    } else if (code_string_equals(c, ":")) {
        if (code_string_equals(c2, ":")) {
            Amalgame_Compiler_Lexer_Advance(self);
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_COLON, "::");
        } else {
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_COLON, ":");
        }
    } else if (code_string_equals(c, "?")) {
        if (code_string_equals(c2, "?")) {
            Amalgame_Compiler_Lexer_Advance(self);
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_COALESCE, "??");
        }
    } else {
        Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_UNKNOWN, c);
    }
}

struct _Amalgame_Compiler_Parser {
    AmalgameList* Tokens;
    i64 Pos;
    AmalgameList* Errors;
    i64 ParenDepth;
};

Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_Parse(Amalgame_Compiler_Parser* self);
code_bool Amalgame_Compiler_Parser_HasErrors(Amalgame_Compiler_Parser* self);
code_string Amalgame_Compiler_Parser_GetErrors(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_Token* Amalgame_Compiler_Parser_Current(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_Token* Amalgame_Compiler_Parser_Peek(Amalgame_Compiler_Parser* self, i64 offset);
static Amalgame_Compiler_Token* Amalgame_Compiler_Parser_Advance(Amalgame_Compiler_Parser* self);
static code_bool Amalgame_Compiler_Parser_IsEnd(Amalgame_Compiler_Parser* self);
static code_bool Amalgame_Compiler_Parser_CheckType(Amalgame_Compiler_Parser* self, Amalgame_Compiler_TokenType t);
static code_bool Amalgame_Compiler_Parser_CheckKw(Amalgame_Compiler_Parser* self, code_string word);
static code_bool Amalgame_Compiler_Parser_CheckValue(Amalgame_Compiler_Parser* self, code_string v);
static void Amalgame_Compiler_Parser_SkipNewlines(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_Token* Amalgame_Compiler_Parser_Expect(Amalgame_Compiler_Parser* self, code_string value);
static Amalgame_Compiler_Token* Amalgame_Compiler_Parser_ExpectIdent(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_Unknown(Amalgame_Compiler_Parser* self);
static code_string Amalgame_Compiler_Parser_ParseQualifiedName(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseDecl(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseClass(Amalgame_Compiler_Parser* self, code_bool isPublic);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseDataClass(Amalgame_Compiler_Parser* self, code_bool isPublic);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseMember(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseField(Amalgame_Compiler_Parser* self, code_bool isPublic);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseMethod(Amalgame_Compiler_Parser* self, code_string retType, code_bool isPublic, code_bool isStatic, i64 line, i64 col);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseParam(Amalgame_Compiler_Parser* self);
static code_string Amalgame_Compiler_Parser_ParseTypeName(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseBlock(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseStmt(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseVarDecl(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseReturn(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseIf(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseWhile(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseForIn(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseEnum(Amalgame_Compiler_Parser* self, code_bool isPublic);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseInterface(Amalgame_Compiler_Parser* self, code_bool isPublic);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseExpr(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseAssign(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseOr(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseAnd(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseEquality(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseRelational(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseAdd(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseMul(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseUnary(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParsePostfix(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseCallArgs(Amalgame_Compiler_Parser* self, Amalgame_Compiler_AstNode* callee);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParsePrimary(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseMatch(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseMatchPattern(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseNew(Amalgame_Compiler_Parser* self);

Amalgame_Compiler_Parser* Amalgame_Compiler_Parser_new(AmalgameList* tokens) {
    Amalgame_Compiler_Parser* self = (Amalgame_Compiler_Parser*) GC_MALLOC(sizeof(Amalgame_Compiler_Parser));
    self->Tokens = tokens;
    self->Pos = 0;
    self->Errors = AmalgameList_new();
    self->ParenDepth = 0;
    return self;
}

Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_Parse(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_AstNode* __attribute__((unused)) prog = Amalgame_Compiler_Ast_Program(1, 1);
    Amalgame_Compiler_Parser_SkipNewlines(self);
    if (Amalgame_Compiler_Parser_CheckKw(self, "namespace")) {
        Amalgame_Compiler_Parser_Advance(self);
        prog->Str = Amalgame_Compiler_Parser_ParseQualifiedName(self);
        Amalgame_Compiler_Parser_SkipNewlines(self);
    }
    while (Amalgame_Compiler_Parser_CheckKw(self, "import")) {
        Amalgame_Compiler_Parser_Advance(self);
        Amalgame_Compiler_Parser_ParseQualifiedName(self);
        Amalgame_Compiler_Parser_SkipNewlines(self);
    }
    i64 __attribute__((unused)) parseLastPos = 0;
    while (!Amalgame_Compiler_Parser_IsEnd(self)) {
        Amalgame_Compiler_Parser_SkipNewlines(self);
        if (Amalgame_Compiler_Parser_IsEnd(self)) {
            break;
        }
        i64 __attribute__((unused)) parsePos = self->Pos;
        if (parsePos == parseLastPos && parseLastPos > 0) {
            Amalgame_Compiler_Parser_Advance(self);
            if (Amalgame_Compiler_Parser_IsEnd(self)) {
                break;
            }
        }
        parseLastPos = self->Pos;
        code_string __attribute__((unused)) curVal = Amalgame_Compiler_Parser_Current(self)->Value;
        Amalgame_Compiler_AstNode* __attribute__((unused)) decl = Amalgame_Compiler_Parser_ParseDecl(self);
        Amalgame_Compiler_NodeKind __attribute__((unused)) dk = decl->Kind;
        if (dk != Amalgame_Compiler_NodeKind_IDENTIFIER) {
            AmalgameList_add(prog->Children, (void*)(intptr_t)(decl));
        }
        Amalgame_Compiler_Parser_SkipNewlines(self);
    }
    return prog;
}

code_bool Amalgame_Compiler_Parser_HasErrors(Amalgame_Compiler_Parser* self) {
    (void)self;
    return AmalgameList_count(self->Errors) > 0;
}

code_string Amalgame_Compiler_Parser_GetErrors(Amalgame_Compiler_Parser* self) {
    (void)self;
    code_string __attribute__((unused)) result = "";
    i64 __attribute__((unused)) count = AmalgameList_count(self->Errors);
    for (i64 i = 0; i < count; i++) {
        code_string __attribute__((unused)) e = (code_string)AmalgameList_get(self->Errors, i);
        result = code_string_concat(code_string_concat(result, e), "\n");
    }
    return result;
}

static Amalgame_Compiler_Token* Amalgame_Compiler_Parser_Current(Amalgame_Compiler_Parser* self) {
    (void)self;
    i64 __attribute__((unused)) count = AmalgameList_count(self->Tokens);
    if (self->Pos >= count) {
        return (Amalgame_Compiler_Token*)AmalgameList_get(self->Tokens, count - 1);
    }
    return (Amalgame_Compiler_Token*)AmalgameList_get(self->Tokens, self->Pos);
}

static Amalgame_Compiler_Token* Amalgame_Compiler_Parser_Peek(Amalgame_Compiler_Parser* self, i64 offset) {
    (void)self;
    (void)offset;
    i64 __attribute__((unused)) i = self->Pos + offset;
    i64 __attribute__((unused)) count = AmalgameList_count(self->Tokens);
    if (i >= count) {
        return (Amalgame_Compiler_Token*)AmalgameList_get(self->Tokens, count - 1);
    }
    return (Amalgame_Compiler_Token*)AmalgameList_get(self->Tokens, i);
}

static Amalgame_Compiler_Token* Amalgame_Compiler_Parser_Advance(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Current(self);
    self->Pos = self->Pos + 1;
    return tok;
}

static code_bool Amalgame_Compiler_Parser_IsEnd(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Current(self);
    return tok->Type == Amalgame_Compiler_TokenType_EOF;
}

static code_bool Amalgame_Compiler_Parser_CheckType(Amalgame_Compiler_Parser* self, Amalgame_Compiler_TokenType t) {
    (void)self;
    (void)t;
    Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Current(self);
    return tok->Type == t;
}

static code_bool Amalgame_Compiler_Parser_CheckKw(Amalgame_Compiler_Parser* self, code_string word) {
    (void)self;
    (void)word;
    Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Current(self);
    return code_string_equals(tok->Value, word);
}

static code_bool Amalgame_Compiler_Parser_CheckValue(Amalgame_Compiler_Parser* self, code_string v) {
    (void)self;
    (void)v;
    Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Current(self);
    return code_string_equals(tok->Value, v);
}

static void Amalgame_Compiler_Parser_SkipNewlines(Amalgame_Compiler_Parser* self) {
    (void)self;
    while (!Amalgame_Compiler_Parser_IsEnd(self) && Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_NEWLINE)) {
        Amalgame_Compiler_Parser_Advance(self);
    }
}

static Amalgame_Compiler_Token* Amalgame_Compiler_Parser_Expect(Amalgame_Compiler_Parser* self, code_string value) {
    (void)self;
    (void)value;
    Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Current(self);
    if (code_string_equals(tok->Value, value)) {
        return Amalgame_Compiler_Parser_Advance(self);
    }
    i64 __attribute__((unused)) line = tok->Line;
    i64 __attribute__((unused)) col = tok->Column;
    code_string __attribute__((unused)) got = tok->Value;
    AmalgameList_add(self->Errors, (void*)(intptr_t)(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("Expected '", value), "' at "), String_FromInt(line)), ":"), String_FromInt(col)), " got '"), got), "'")));
    return tok;
}

static Amalgame_Compiler_Token* Amalgame_Compiler_Parser_ExpectIdent(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Current(self);
    if (Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_IDENTIFIER)) {
        return Amalgame_Compiler_Parser_Advance(self);
    }
    i64 __attribute__((unused)) line = tok->Line;
    code_string __attribute__((unused)) got = tok->Value;
    AmalgameList_add(self->Errors, (void*)(intptr_t)(code_string_concat(code_string_concat(code_string_concat(code_string_concat("Expected identifier at line ", String_FromInt(line)), " got '"), got), "'")));
    Amalgame_Compiler_Parser_Advance(self);
    return tok;
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_Unknown(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Current(self);
    return Amalgame_Compiler_Ast_Ident("_unknown_", tok->Line, tok->Column);
}

static code_string Amalgame_Compiler_Parser_ParseQualifiedName(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_ExpectIdent(self);
    code_string __attribute__((unused)) name = tok->Value;
    while (Amalgame_Compiler_Parser_CheckValue(self, ".")) {
        Amalgame_Compiler_Parser_Advance(self);
        Amalgame_Compiler_Token* __attribute__((unused)) next = Amalgame_Compiler_Parser_ExpectIdent(self);
        name = code_string_concat(code_string_concat(name, "."), next->Value);
    }
    return name;
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseDecl(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_Parser_SkipNewlines(self);
    if (Amalgame_Compiler_Parser_IsEnd(self)) {
        return Amalgame_Compiler_Parser_Unknown(self);
    }
    code_bool __attribute__((unused)) isPublic = 0;
    code_bool __attribute__((unused)) isStatic = 0;
    if (Amalgame_Compiler_Parser_CheckKw(self, "public")) {
        isPublic = 1;
        Amalgame_Compiler_Parser_Advance(self);
    }
    if (Amalgame_Compiler_Parser_CheckKw(self, "private")) {
        Amalgame_Compiler_Parser_Advance(self);
    }
    if (Amalgame_Compiler_Parser_CheckKw(self, "static")) {
        isStatic = 1;
        Amalgame_Compiler_Parser_Advance(self);
    }
    if (Amalgame_Compiler_Parser_CheckKw(self, "class")) {
        return Amalgame_Compiler_Parser_ParseClass(self, isPublic);
    }
    if (Amalgame_Compiler_Parser_CheckKw(self, "data")) {
        Amalgame_Compiler_Parser_Advance(self);
        if (Amalgame_Compiler_Parser_CheckKw(self, "class")) {
            return Amalgame_Compiler_Parser_ParseDataClass(self, isPublic);
        }
    }
    if (Amalgame_Compiler_Parser_CheckKw(self, "record")) {
        Amalgame_Compiler_Parser_Advance(self);
        return Amalgame_Compiler_Parser_ParseDataClass(self, isPublic);
    }
    if (Amalgame_Compiler_Parser_CheckKw(self, "enum")) {
        return Amalgame_Compiler_Parser_ParseEnum(self, isPublic);
    }
    if (Amalgame_Compiler_Parser_CheckKw(self, "interface")) {
        return Amalgame_Compiler_Parser_ParseInterface(self, isPublic);
    }
    Amalgame_Compiler_Parser_Advance(self);
    return Amalgame_Compiler_Parser_Unknown(self);
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseClass(Amalgame_Compiler_Parser* self, code_bool isPublic) {
    (void)self;
    (void)isPublic;
    Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Advance(self);
    Amalgame_Compiler_Token* __attribute__((unused)) nameTok = Amalgame_Compiler_Parser_ExpectIdent(self);
    Amalgame_Compiler_AstNode* __attribute__((unused)) cls = Amalgame_Compiler_Ast_Class(nameTok->Value, tok->Line, tok->Column);
    cls->Flag = isPublic;
    if (Amalgame_Compiler_Parser_CheckValue(self, "<")) {
        Amalgame_Compiler_Parser_Advance(self);
        while (!Amalgame_Compiler_Parser_IsEnd(self) && !Amalgame_Compiler_Parser_CheckValue(self, ">")) {
            Amalgame_Compiler_Parser_Advance(self);
        }
        if (Amalgame_Compiler_Parser_CheckValue(self, ">")) {
            Amalgame_Compiler_Parser_Advance(self);
        }
    }
    if (Amalgame_Compiler_Parser_CheckKw(self, "extends")) {
        Amalgame_Compiler_Parser_Advance(self);
        cls->Str = Amalgame_Compiler_Parser_ParseQualifiedName(self);
    }
    if (Amalgame_Compiler_Parser_CheckKw(self, "implements")) {
        Amalgame_Compiler_Parser_Advance(self);
        while (!Amalgame_Compiler_Parser_IsEnd(self) && !Amalgame_Compiler_Parser_CheckValue(self, "{") && !Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_NEWLINE)) {
            Amalgame_Compiler_Parser_Advance(self);
        }
    }
    Amalgame_Compiler_Parser_SkipNewlines(self);
    i64 __attribute__((unused)) classLastPos = 0;
    while (!Amalgame_Compiler_Parser_IsEnd(self) && !Amalgame_Compiler_Parser_CheckValue(self, "}")) {
        Amalgame_Compiler_Parser_SkipNewlines(self);
        if (Amalgame_Compiler_Parser_CheckValue(self, "}")) {
            break;
        }
        i64 __attribute__((unused)) classPos = self->Pos;
        if (classPos == classLastPos && classLastPos > 0) {
            Amalgame_Compiler_Parser_Advance(self);
            if (Amalgame_Compiler_Parser_CheckValue(self, "}")) {
                break;
            }
        }
        classLastPos = self->Pos;
        Amalgame_Compiler_AstNode* __attribute__((unused)) member = Amalgame_Compiler_Parser_ParseMember(self);
        Amalgame_Compiler_NodeKind __attribute__((unused)) mk = member->Kind;
        if (mk != Amalgame_Compiler_NodeKind_IDENTIFIER) {
            AmalgameList_add(cls->Children, (void*)(intptr_t)(member));
        }
        Amalgame_Compiler_Parser_SkipNewlines(self);
    }
    Amalgame_Compiler_Parser_Expect(self, "}");
    return cls;
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseDataClass(Amalgame_Compiler_Parser* self, code_bool isPublic) {
    (void)self;
    (void)isPublic;
    if (Amalgame_Compiler_Parser_CheckKw(self, "class")) {
        Amalgame_Compiler_Parser_Advance(self);
    }
    Amalgame_Compiler_Token* __attribute__((unused)) nameTok = Amalgame_Compiler_Parser_ExpectIdent(self);
    Amalgame_Compiler_AstNode* __attribute__((unused)) cls = Amalgame_Compiler_Ast_Class(nameTok->Value, nameTok->Line, nameTok->Column);
    cls->Flag = isPublic;
    if (Amalgame_Compiler_Parser_CheckValue(self, "(")) {
        Amalgame_Compiler_Parser_Advance(self);
        Amalgame_Compiler_AstNode* __attribute__((unused)) ctor = Amalgame_Compiler_Ast_Method(nameTok->Value, "void", nameTok->Line, nameTok->Column);
        ctor->Flag = 1;
        while (!Amalgame_Compiler_Parser_IsEnd(self) && !Amalgame_Compiler_Parser_CheckValue(self, ")")) {
            Amalgame_Compiler_Parser_SkipNewlines(self);
            if (Amalgame_Compiler_Parser_CheckValue(self, ")")) {
                break;
            }
            if (Amalgame_Compiler_Parser_CheckValue(self, ",")) {
                Amalgame_Compiler_Parser_Advance(self);
            }
            Amalgame_Compiler_Parser_SkipNewlines(self);
            if (Amalgame_Compiler_Parser_CheckValue(self, ")")) {
                break;
            }
            code_string __attribute__((unused)) ftype = Amalgame_Compiler_Parser_ParseTypeName(self);
            Amalgame_Compiler_Token* __attribute__((unused)) fname = Amalgame_Compiler_Parser_ExpectIdent(self);
            Amalgame_Compiler_AstNode* __attribute__((unused)) field = Amalgame_Compiler_Ast_VarDecl(fname->Value, 1, fname->Line, fname->Column);
            field->Str = ftype;
            field->Flag = 1;
            AmalgameList_add(cls->Children, (void*)(intptr_t)(field));
            Amalgame_Compiler_AstNode* __attribute__((unused)) param = Amalgame_Compiler_Ast_Param(fname->Value, ftype, fname->Line, fname->Column);
            AmalgameList_add(ctor->Params, (void*)(intptr_t)(param));
            if (Amalgame_Compiler_Parser_CheckValue(self, ",")) {
                Amalgame_Compiler_Parser_Advance(self);
            }
        }
        if (Amalgame_Compiler_Parser_CheckValue(self, ")")) {
            Amalgame_Compiler_Parser_Advance(self);
        }
        AmalgameList_add(cls->Children, (void*)(intptr_t)(ctor));
    }
    if (Amalgame_Compiler_Parser_CheckValue(self, "{")) {
        i64 __attribute__((unused)) depth2 = 0;
        while (!Amalgame_Compiler_Parser_IsEnd(self)) {
            if (Amalgame_Compiler_Parser_CheckValue(self, "{")) {
                depth2 = depth2 + 1;
            }
            if (Amalgame_Compiler_Parser_CheckValue(self, "}")) {
                depth2 = depth2 - 1;
                if (depth2 == 0) {
                    Amalgame_Compiler_Parser_Advance(self);
                    break;
                }
            }
            Amalgame_Compiler_Parser_Advance(self);
        }
    }
    return cls;
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseMember(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_Parser_SkipNewlines(self);
    if (Amalgame_Compiler_Parser_IsEnd(self) || Amalgame_Compiler_Parser_CheckValue(self, "}")) {
        return Amalgame_Compiler_Parser_Unknown(self);
    }
    code_bool __attribute__((unused)) isPublic = 0;
    code_bool __attribute__((unused)) isStatic = 0;
    if (Amalgame_Compiler_Parser_CheckKw(self, "public")) {
        isPublic = 1;
        Amalgame_Compiler_Parser_Advance(self);
    }
    if (Amalgame_Compiler_Parser_CheckKw(self, "private")) {
        Amalgame_Compiler_Parser_Advance(self);
    }
    if (Amalgame_Compiler_Parser_CheckKw(self, "static")) {
        isStatic = 1;
        Amalgame_Compiler_Parser_Advance(self);
    }
    Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Current(self);
    if (Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_IDENTIFIER)) {
        Amalgame_Compiler_Token* __attribute__((unused)) peek = Amalgame_Compiler_Parser_Peek(self, 1);
        if (code_string_equals(peek->Value, ":")) {
            return Amalgame_Compiler_Parser_ParseField(self, isPublic);
        }
        if (code_string_equals(peek->Value, "(")) {
            return Amalgame_Compiler_Parser_ParseMethod(self, "void", isPublic, isStatic, tok->Line, tok->Column);
        }
        code_string __attribute__((unused)) typeName = Amalgame_Compiler_Parser_ParseTypeName(self);
        Amalgame_Compiler_Token* __attribute__((unused)) nameTok = Amalgame_Compiler_Parser_Current(self);
        if (Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_IDENTIFIER)) {
            return Amalgame_Compiler_Parser_ParseMethod(self, typeName, isPublic, isStatic, tok->Line, tok->Column);
        }
    } else if (code_string_equals(tok->Value, "int") || code_string_equals(tok->Value, "string") || code_string_equals(tok->Value, "bool") || code_string_equals(tok->Value, "void") || code_string_equals(tok->Value, "float")) {
        code_string __attribute__((unused)) typeName = Amalgame_Compiler_Parser_ParseTypeName(self);
        if (Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_IDENTIFIER)) {
            return Amalgame_Compiler_Parser_ParseMethod(self, typeName, isPublic, isStatic, tok->Line, tok->Column);
        }
    } else if (code_string_equals(tok->Value, "(")) {
        Amalgame_Compiler_Parser_Advance(self);
        code_string __attribute__((unused)) tupleTypes = "";
        i64 __attribute__((unused)) tupleCount = 0;
        while (!Amalgame_Compiler_Parser_IsEnd(self) && !Amalgame_Compiler_Parser_CheckValue(self, ")")) {
            Amalgame_Compiler_Parser_SkipNewlines(self);
            if (Amalgame_Compiler_Parser_CheckValue(self, ",")) {
                Amalgame_Compiler_Parser_Advance(self);
                continue;
            }
            code_string __attribute__((unused)) tname = Amalgame_Compiler_Parser_ParseTypeName(self);
            if (tupleCount > 0) {
                tupleTypes = code_string_concat(tupleTypes, ",");
            }
            tupleTypes = code_string_concat(tupleTypes, tname);
            tupleCount = tupleCount + 1;
        }
        if (Amalgame_Compiler_Parser_CheckValue(self, ")")) {
            Amalgame_Compiler_Parser_Advance(self);
        }
        Amalgame_Compiler_Parser_SkipNewlines(self);
        if (Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_IDENTIFIER)) {
            code_string __attribute__((unused)) retType = code_string_concat(code_string_concat("(", tupleTypes), ")");
            Amalgame_Compiler_AstNode* __attribute__((unused)) method = Amalgame_Compiler_Parser_ParseMethod(self, retType, isPublic, isStatic, tok->Line, tok->Column);
            return method;
        }
        return Amalgame_Compiler_Parser_Unknown(self);
    }
    while (!Amalgame_Compiler_Parser_IsEnd(self) && !Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_NEWLINE) && !Amalgame_Compiler_Parser_CheckValue(self, "}")) {
        Amalgame_Compiler_Parser_Advance(self);
    }
    return Amalgame_Compiler_Parser_Unknown(self);
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseField(Amalgame_Compiler_Parser* self, code_bool isPublic) {
    (void)self;
    (void)isPublic;
    Amalgame_Compiler_Token* __attribute__((unused)) nameTok = Amalgame_Compiler_Parser_ExpectIdent(self);
    Amalgame_Compiler_Parser_Expect(self, ":");
    code_string __attribute__((unused)) typeName = Amalgame_Compiler_Parser_ParseTypeName(self);
    Amalgame_Compiler_AstNode* __attribute__((unused)) field = Amalgame_Compiler_Ast_VarDecl(nameTok->Value, 0, nameTok->Line, nameTok->Column);
    field->Str = typeName;
    field->Flag = isPublic;
    if (Amalgame_Compiler_Parser_CheckValue(self, "=")) {
        Amalgame_Compiler_Parser_Advance(self);
        field->Left = Amalgame_Compiler_Parser_ParseExpr(self);
    }
    return field;
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseMethod(Amalgame_Compiler_Parser* self, code_string retType, code_bool isPublic, code_bool isStatic, i64 line, i64 col) {
    (void)self;
    (void)retType;
    (void)isPublic;
    (void)isStatic;
    (void)line;
    (void)col;
    Amalgame_Compiler_Token* __attribute__((unused)) nameTok = Amalgame_Compiler_Parser_ExpectIdent(self);
    Amalgame_Compiler_AstNode* __attribute__((unused)) method = Amalgame_Compiler_Ast_Method(nameTok->Value, retType, line, col);
    method->Flag = isPublic;
    method->Flag2 = isStatic;
    code_string __attribute__((unused)) mname = nameTok->Value;
    if (Amalgame_Compiler_Parser_CheckValue(self, "<")) {
        Amalgame_Compiler_Parser_Advance(self);
        while (!Amalgame_Compiler_Parser_IsEnd(self) && !Amalgame_Compiler_Parser_CheckValue(self, ">")) {
            Amalgame_Compiler_Parser_Advance(self);
        }
        if (Amalgame_Compiler_Parser_CheckValue(self, ">")) {
            Amalgame_Compiler_Parser_Advance(self);
        }
    }
    Amalgame_Compiler_Parser_Expect(self, "(");
    while (!Amalgame_Compiler_Parser_IsEnd(self) && !Amalgame_Compiler_Parser_CheckValue(self, ")")) {
        if (Amalgame_Compiler_Parser_CheckValue(self, ",")) {
            Amalgame_Compiler_Parser_Advance(self);
            continue;
        }
        Amalgame_Compiler_AstNode* __attribute__((unused)) p = Amalgame_Compiler_Parser_ParseParam(self);
        Amalgame_Compiler_NodeKind __attribute__((unused)) pk = p->Kind;
        if (pk != Amalgame_Compiler_NodeKind_IDENTIFIER) {
            AmalgameList_add(method->Params, (void*)(intptr_t)(p));
        }
    }
    Amalgame_Compiler_Parser_Expect(self, ")");
    if (Amalgame_Compiler_Parser_CheckValue(self, "=>")) {
        Amalgame_Compiler_Parser_Advance(self);
        Amalgame_Compiler_AstNode* __attribute__((unused)) body = Amalgame_Compiler_Ast_Block(line, col);
        Amalgame_Compiler_AstNode* __attribute__((unused)) ret = Amalgame_Compiler_Ast_Return(Amalgame_Compiler_Parser_ParseExpr(self), line, col);
        AmalgameList_add(body->Children, (void*)(intptr_t)(ret));
        method->Body = body;
        return method;
    }
    Amalgame_Compiler_Parser_SkipNewlines(self);
    if (Amalgame_Compiler_Parser_CheckValue(self, "{")) {
        method->Body = Amalgame_Compiler_Parser_ParseBlock(self);
    }
    return method;
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseParam(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Current(self);
    code_string __attribute__((unused)) v = tok->Value;
    code_bool __attribute__((unused)) isKeywordType = code_string_equals(v, "int") || code_string_equals(v, "string") || code_string_equals(v, "bool") || code_string_equals(v, "void") || code_string_equals(v, "float");
    if (!Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_IDENTIFIER) && !isKeywordType) {
        return Amalgame_Compiler_Parser_Unknown(self);
    }
    code_string __attribute__((unused)) typeName = Amalgame_Compiler_Parser_ParseTypeName(self);
    if (!Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_IDENTIFIER)) {
        return Amalgame_Compiler_Ast_Param("_", typeName, 0, 0);
    }
    Amalgame_Compiler_Token* __attribute__((unused)) nameTok = Amalgame_Compiler_Parser_ExpectIdent(self);
    return Amalgame_Compiler_Ast_Param(nameTok->Value, typeName, nameTok->Line, nameTok->Column);
}

static code_string Amalgame_Compiler_Parser_ParseTypeName(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Current(self);
    code_string __attribute__((unused)) v = tok->Value;
    if (!Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_IDENTIFIER)) {
        if (code_string_equals(v, "void") || code_string_equals(v, "int") || code_string_equals(v, "string") || code_string_equals(v, "bool") || code_string_equals(v, "float")) {
            Amalgame_Compiler_Parser_Advance(self);
            code_string __attribute__((unused)) kwName = v;
            if (Amalgame_Compiler_Parser_CheckValue(self, "[")) {
                Amalgame_Compiler_Parser_Advance(self);
                Amalgame_Compiler_Parser_Expect(self, "]");
                kwName = code_string_concat(kwName, "[]");
            }
            return kwName;
        }
        return "void";
    }
    Amalgame_Compiler_Token* __attribute__((unused)) nameTok = Amalgame_Compiler_Parser_ExpectIdent(self);
    code_string __attribute__((unused)) name = nameTok->Value;
    if (Amalgame_Compiler_Parser_CheckValue(self, "<")) {
        Amalgame_Compiler_Parser_Advance(self);
        name = code_string_concat(name, "<");
        i64 __attribute__((unused)) depth = 1;
        while (!Amalgame_Compiler_Parser_IsEnd(self) && depth > 0) {
            Amalgame_Compiler_Token* __attribute__((unused)) inner = Amalgame_Compiler_Parser_Current(self);
            code_string __attribute__((unused)) iv = inner->Value;
            if (code_string_equals(iv, "<")) {
                depth = depth + 1;
            }
            if (code_string_equals(iv, ">")) {
                depth = depth - 1;
            }
            if (depth > 0) {
                name = code_string_concat(name, iv);
            }
            Amalgame_Compiler_Parser_Advance(self);
        }
        name = code_string_concat(name, ">");
    }
    if (Amalgame_Compiler_Parser_CheckValue(self, "[")) {
        Amalgame_Compiler_Parser_Advance(self);
        Amalgame_Compiler_Parser_Expect(self, "]");
        name = code_string_concat(name, "[]");
    }
    if (Amalgame_Compiler_Parser_CheckValue(self, "?")) {
        Amalgame_Compiler_Parser_Advance(self);
        name = code_string_concat(name, "?");
    }
    return name;
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseBlock(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Expect(self, "{");
    Amalgame_Compiler_AstNode* __attribute__((unused)) block = Amalgame_Compiler_Ast_Block(tok->Line, tok->Column);
    Amalgame_Compiler_Parser_SkipNewlines(self);
    i64 __attribute__((unused)) lastPos = 0;
    while (!Amalgame_Compiler_Parser_IsEnd(self) && !Amalgame_Compiler_Parser_CheckValue(self, "}")) {
        Amalgame_Compiler_Parser_SkipNewlines(self);
        if (Amalgame_Compiler_Parser_CheckValue(self, "}")) {
            break;
        }
        i64 __attribute__((unused)) curPos = self->Pos;
        if (curPos == lastPos && lastPos > 0) {
            Amalgame_Compiler_Parser_Advance(self);
            Amalgame_Compiler_Parser_SkipNewlines(self);
            if (Amalgame_Compiler_Parser_CheckValue(self, "}")) {
                break;
            }
            if (Amalgame_Compiler_Parser_IsEnd(self)) {
                break;
            }
        }
        lastPos = self->Pos;
        Amalgame_Compiler_AstNode* __attribute__((unused)) stmt = Amalgame_Compiler_Parser_ParseStmt(self);
        AmalgameList_add(block->Children, (void*)(intptr_t)(stmt));
        Amalgame_Compiler_Parser_SkipNewlines(self);
    }
    Amalgame_Compiler_Parser_Expect(self, "}");
    return block;
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseStmt(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_Parser_SkipNewlines(self);
    if (Amalgame_Compiler_Parser_IsEnd(self) || Amalgame_Compiler_Parser_CheckValue(self, "}")) {
        return Amalgame_Compiler_Parser_Unknown(self);
    }
    Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Current(self);
    code_string __attribute__((unused)) v = tok->Value;
    if (code_string_equals(v, "let") || code_string_equals(v, "var")) {
        return Amalgame_Compiler_Parser_ParseVarDecl(self);
    }
    if (code_string_equals(v, "return")) {
        return Amalgame_Compiler_Parser_ParseReturn(self);
    }
    if (code_string_equals(v, "if")) {
        return Amalgame_Compiler_Parser_ParseIf(self);
    }
    if (code_string_equals(v, "while")) {
        return Amalgame_Compiler_Parser_ParseWhile(self);
    }
    if (code_string_equals(v, "for")) {
        return Amalgame_Compiler_Parser_ParseForIn(self);
    }
    if (code_string_equals(v, "break")) {
        Amalgame_Compiler_Parser_Advance(self);
        Amalgame_Compiler_AstNode* __attribute__((unused)) n = Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_BREAK_STMT, tok->Line, tok->Column);
        return n;
    }
    if (code_string_equals(v, "continue")) {
        Amalgame_Compiler_Parser_Advance(self);
        Amalgame_Compiler_AstNode* __attribute__((unused)) n = Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_CONTINUE_STMT, tok->Line, tok->Column);
        return n;
    }
    if (code_string_equals(v, "match")) {
        return Amalgame_Compiler_Parser_ParseMatch(self);
    }
    if (code_string_equals(v, "{")) {
        return Amalgame_Compiler_Parser_ParseBlock(self);
    }
    return Amalgame_Compiler_Parser_ParseExpr(self);
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseVarDecl(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_Token* __attribute__((unused)) kwTok = Amalgame_Compiler_Parser_Advance(self);
    code_bool __attribute__((unused)) isMut = code_string_equals(kwTok->Value, "var");
    if (Amalgame_Compiler_Parser_CheckValue(self, "(")) {
        Amalgame_Compiler_Parser_Advance(self);
        AmalgameList* __attribute__((unused)) names = AmalgameList_new();
        while (!Amalgame_Compiler_Parser_IsEnd(self) && !Amalgame_Compiler_Parser_CheckValue(self, ")")) {
            Amalgame_Compiler_Parser_SkipNewlines(self);
            if (Amalgame_Compiler_Parser_CheckValue(self, ",")) {
                Amalgame_Compiler_Parser_Advance(self);
                continue;
            }
            if (Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_IDENTIFIER)) {
                Amalgame_Compiler_Token* __attribute__((unused)) n = Amalgame_Compiler_Parser_Advance(self);
                AmalgameList_add(names, (void*)(intptr_t)(n->Value));
            }
        }
        if (Amalgame_Compiler_Parser_CheckValue(self, ")")) {
            Amalgame_Compiler_Parser_Advance(self);
        }
        if (Amalgame_Compiler_Parser_CheckValue(self, "=")) {
            Amalgame_Compiler_Parser_Advance(self);
        }
        Amalgame_Compiler_AstNode* __attribute__((unused)) rhs = Amalgame_Compiler_Parser_ParseExpr(self);
        Amalgame_Compiler_AstNode* __attribute__((unused)) td = Amalgame_Compiler_Ast_VarDecl("__tuple__", isMut, kwTok->Line, kwTok->Column);
        td->Left = rhs;
        i64 __attribute__((unused)) nc = AmalgameList_count(names);
        for (i64 i = 0; i < nc; i++) {
            void* __attribute__((unused)) vname = (void*)AmalgameList_get(names, i);
            Amalgame_Compiler_AstNode* __attribute__((unused)) vnode = Amalgame_Compiler_Ast_Ident(vname, kwTok->Line, kwTok->Column);
            AmalgameList_add(td->Children, (void*)(intptr_t)(vnode));
        }
        td->Str = "__tuple_destructure__";
        return td;
    }
    Amalgame_Compiler_Token* __attribute__((unused)) nameTok = Amalgame_Compiler_Parser_ExpectIdent(self);
    Amalgame_Compiler_AstNode* __attribute__((unused)) decl = Amalgame_Compiler_Ast_VarDecl(nameTok->Value, isMut, nameTok->Line, nameTok->Column);
    if (Amalgame_Compiler_Parser_CheckValue(self, ":")) {
        Amalgame_Compiler_Parser_Advance(self);
        decl->Str = Amalgame_Compiler_Parser_ParseTypeName(self);
    }
    if (Amalgame_Compiler_Parser_CheckValue(self, "=")) {
        Amalgame_Compiler_Parser_Advance(self);
        decl->Left = Amalgame_Compiler_Parser_ParseExpr(self);
    }
    return decl;
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseReturn(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Advance(self);
    if (Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_NEWLINE) || Amalgame_Compiler_Parser_CheckValue(self, "}") || Amalgame_Compiler_Parser_IsEnd(self)) {
        return Amalgame_Compiler_Ast_Return(Amalgame_Compiler_Parser_Unknown(self), tok->Line, tok->Column);
    }
    return Amalgame_Compiler_Ast_Return(Amalgame_Compiler_Parser_ParseExpr(self), tok->Line, tok->Column);
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseIf(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Advance(self);
    code_bool __attribute__((unused)) hasParen = Amalgame_Compiler_Parser_CheckValue(self, "(");
    if (hasParen) {
        Amalgame_Compiler_Parser_Advance(self);
        self->ParenDepth = self->ParenDepth + 1;
    }
    Amalgame_Compiler_AstNode* __attribute__((unused)) cond = Amalgame_Compiler_Parser_ParseExpr(self);
    if (hasParen) {
        self->ParenDepth = self->ParenDepth - 1;
        Amalgame_Compiler_Parser_Expect(self, ")");
    }
    Amalgame_Compiler_Parser_SkipNewlines(self);
    Amalgame_Compiler_AstNode* __attribute__((unused)) then = Amalgame_Compiler_Parser_ParseBlock(self);
    Amalgame_Compiler_AstNode* __attribute__((unused)) node = Amalgame_Compiler_Ast_If(cond, then, tok->Line, tok->Column);
    Amalgame_Compiler_Parser_SkipNewlines(self);
    if (Amalgame_Compiler_Parser_CheckKw(self, "else")) {
        Amalgame_Compiler_Parser_Advance(self);
        Amalgame_Compiler_Parser_SkipNewlines(self);
        node->Flag = 1;
        if (Amalgame_Compiler_Parser_CheckKw(self, "if")) {
            node->Else = Amalgame_Compiler_Parser_ParseIf(self);
        } else {
            node->Else = Amalgame_Compiler_Parser_ParseBlock(self);
        }
    }
    return node;
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseWhile(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Advance(self);
    code_bool __attribute__((unused)) hasParen = Amalgame_Compiler_Parser_CheckValue(self, "(");
    if (hasParen) {
        Amalgame_Compiler_Parser_Advance(self);
        self->ParenDepth = self->ParenDepth + 1;
    }
    Amalgame_Compiler_AstNode* __attribute__((unused)) cond = Amalgame_Compiler_Parser_ParseExpr(self);
    if (hasParen) {
        self->ParenDepth = self->ParenDepth - 1;
        Amalgame_Compiler_Parser_Expect(self, ")");
    }
    Amalgame_Compiler_Parser_SkipNewlines(self);
    Amalgame_Compiler_AstNode* __attribute__((unused)) body = Amalgame_Compiler_Parser_ParseBlock(self);
    Amalgame_Compiler_AstNode* __attribute__((unused)) node = Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_WHILE_STMT, tok->Line, tok->Column);
    node->Cond = cond;
    node->Body = body;
    return node;
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseForIn(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Advance(self);
    Amalgame_Compiler_Token* __attribute__((unused)) varTok = Amalgame_Compiler_Parser_ExpectIdent(self);
    Amalgame_Compiler_Parser_Expect(self, "in");
    Amalgame_Compiler_AstNode* __attribute__((unused)) iter = Amalgame_Compiler_Parser_ParseExpr(self);
    Amalgame_Compiler_Parser_SkipNewlines(self);
    Amalgame_Compiler_AstNode* __attribute__((unused)) body = Amalgame_Compiler_Parser_ParseBlock(self);
    Amalgame_Compiler_AstNode* __attribute__((unused)) node = Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_FOR_IN_STMT, tok->Line, tok->Column);
    node->Name = varTok->Value;
    node->Left = iter;
    node->Body = body;
    return node;
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseEnum(Amalgame_Compiler_Parser* self, code_bool isPublic) {
    (void)self;
    (void)isPublic;
    Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Advance(self);
    Amalgame_Compiler_Token* __attribute__((unused)) nameTok = Amalgame_Compiler_Parser_ExpectIdent(self);
    Amalgame_Compiler_AstNode* __attribute__((unused)) en = Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_ENUM_DECL, tok->Line, tok->Column);
    en->Name = nameTok->Value;
    en->Flag = isPublic;
    Amalgame_Compiler_Parser_SkipNewlines(self);
    Amalgame_Compiler_Parser_Expect(self, "{");
    Amalgame_Compiler_Parser_SkipNewlines(self);
    while (!Amalgame_Compiler_Parser_IsEnd(self) && !Amalgame_Compiler_Parser_CheckValue(self, "}")) {
        if (Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_NEWLINE)) {
            Amalgame_Compiler_Parser_Advance(self);
            continue;
        }
        if (Amalgame_Compiler_Parser_CheckValue(self, ",")) {
            Amalgame_Compiler_Parser_Advance(self);
            continue;
        }
        if (!Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_IDENTIFIER)) {
            Amalgame_Compiler_Parser_Advance(self);
            continue;
        }
        Amalgame_Compiler_Token* __attribute__((unused)) memberTok = Amalgame_Compiler_Parser_ExpectIdent(self);
        Amalgame_Compiler_AstNode* __attribute__((unused)) member = Amalgame_Compiler_Ast_Ident(memberTok->Value, memberTok->Line, memberTok->Column);
        if (Amalgame_Compiler_Parser_CheckValue(self, "(")) {
            Amalgame_Compiler_Parser_Advance(self);
            code_string __attribute__((unused)) typeList = "";
            i64 __attribute__((unused)) typeCount = 0;
            while (!Amalgame_Compiler_Parser_IsEnd(self) && !Amalgame_Compiler_Parser_CheckValue(self, ")")) {
                if (Amalgame_Compiler_Parser_CheckValue(self, ",")) {
                    Amalgame_Compiler_Parser_Advance(self);
                    continue;
                }
                code_string __attribute__((unused)) tname = Amalgame_Compiler_Parser_ParseTypeName(self);
                if (typeCount > 0) {
                    typeList = code_string_concat(typeList, ",");
                }
                typeList = code_string_concat(typeList, tname);
                typeCount = typeCount + 1;
            }
            Amalgame_Compiler_Parser_Expect(self, ")");
            member->Str = typeList;
        }
        AmalgameList_add(en->Children, (void*)(intptr_t)(member));
        Amalgame_Compiler_Parser_SkipNewlines(self);
    }
    Amalgame_Compiler_Parser_Expect(self, "}");
    return en;
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseInterface(Amalgame_Compiler_Parser* self, code_bool isPublic) {
    (void)self;
    (void)isPublic;
    Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Advance(self);
    Amalgame_Compiler_Token* __attribute__((unused)) nameTok = Amalgame_Compiler_Parser_ExpectIdent(self);
    Amalgame_Compiler_AstNode* __attribute__((unused)) iface = Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_CLASS_DECL, tok->Line, tok->Column);
    iface->Name = nameTok->Value;
    iface->Flag = isPublic;
    iface->Flag2 = 1;
    Amalgame_Compiler_Parser_SkipNewlines(self);
    Amalgame_Compiler_Parser_Expect(self, "{");
    Amalgame_Compiler_Parser_SkipNewlines(self);
    while (!Amalgame_Compiler_Parser_IsEnd(self) && !Amalgame_Compiler_Parser_CheckValue(self, "}")) {
        Amalgame_Compiler_Parser_SkipNewlines(self);
        if (Amalgame_Compiler_Parser_CheckValue(self, "}")) {
            break;
        }
        code_string __attribute__((unused)) typeName = "void";
        code_string __attribute__((unused)) methodName = "_unknown_";
        i64 __attribute__((unused)) mline = Amalgame_Compiler_Parser_Current(self)->Line;
        i64 __attribute__((unused)) mcol = Amalgame_Compiler_Parser_Current(self)->Column;
        if (Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_IDENTIFIER)) {
            Amalgame_Compiler_Token* __attribute__((unused)) peek1 = Amalgame_Compiler_Parser_Peek(self, 1);
            if (code_string_equals(peek1->Value, "(")) {
                Amalgame_Compiler_Token* __attribute__((unused)) mnameTok2 = Amalgame_Compiler_Parser_ExpectIdent(self);
                methodName = mnameTok2->Value;
                mline = mnameTok2->Line;
                mcol = mnameTok2->Column;
            } else {
                typeName = Amalgame_Compiler_Parser_ParseTypeName(self);
                Amalgame_Compiler_Token* __attribute__((unused)) mnameTok3 = Amalgame_Compiler_Parser_ExpectIdent(self);
                methodName = mnameTok3->Value;
                mline = mnameTok3->Line;
                mcol = mnameTok3->Column;
            }
        }
        Amalgame_Compiler_AstNode* __attribute__((unused)) m = Amalgame_Compiler_Ast_Method(methodName, typeName, mline, mcol);
        Amalgame_Compiler_Parser_Expect(self, "(");
        while (!Amalgame_Compiler_Parser_IsEnd(self) && !Amalgame_Compiler_Parser_CheckValue(self, ")")) {
            if (Amalgame_Compiler_Parser_CheckValue(self, ",")) {
                Amalgame_Compiler_Parser_Advance(self);
            }
            if (Amalgame_Compiler_Parser_CheckValue(self, ")")) {
                break;
            }
            Amalgame_Compiler_AstNode* __attribute__((unused)) p = Amalgame_Compiler_Parser_ParseParam(self);
            AmalgameList_add(m->Params, (void*)(intptr_t)(p));
        }
        Amalgame_Compiler_Parser_Expect(self, ")");
        if (Amalgame_Compiler_Parser_CheckValue(self, "->")) {
            Amalgame_Compiler_Parser_Advance(self);
            code_string __attribute__((unused)) retType2 = Amalgame_Compiler_Parser_ParseTypeName(self);
            m->Str = retType2;
        }
        AmalgameList_add(iface->Children, (void*)(intptr_t)(m));
        Amalgame_Compiler_Parser_SkipNewlines(self);
    }
    Amalgame_Compiler_Parser_Expect(self, "}");
    return iface;
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseExpr(Amalgame_Compiler_Parser* self) {
    (void)self;
    if (Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_IDENTIFIER) && code_string_equals(Amalgame_Compiler_Parser_Peek(self, 1)->Value, "=>")) {
        Amalgame_Compiler_Token* __attribute__((unused)) paramTok = Amalgame_Compiler_Parser_Advance(self);
        Amalgame_Compiler_Parser_Advance(self);
        Amalgame_Compiler_AstNode* __attribute__((unused)) body = Amalgame_Compiler_Parser_ParseExpr(self);
        Amalgame_Compiler_AstNode* __attribute__((unused)) lam = Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_METHOD_DECL, paramTok->Line, paramTok->Column);
        lam->Name = "__lambda__";
        lam->Str = paramTok->Value;
        lam->Left = body;
        return lam;
    }
    return Amalgame_Compiler_Parser_ParseAssign(self);
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseAssign(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_AstNode* __attribute__((unused)) left = Amalgame_Compiler_Parser_ParseOr(self);
    if (Amalgame_Compiler_Parser_CheckValue(self, "=")) {
        Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Advance(self);
        Amalgame_Compiler_AstNode* __attribute__((unused)) right = Amalgame_Compiler_Parser_ParseAssign(self);
        return Amalgame_Compiler_Ast_Binary(left, "=", right, tok->Line, tok->Column);
    }
    return left;
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseOr(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_AstNode* __attribute__((unused)) left = Amalgame_Compiler_Parser_ParseAnd(self);
    while (Amalgame_Compiler_Parser_CheckValue(self, "||")) {
        Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Advance(self);
        Amalgame_Compiler_AstNode* __attribute__((unused)) right = Amalgame_Compiler_Parser_ParseAnd(self);
        left = Amalgame_Compiler_Ast_Binary(left, "||", right, tok->Line, tok->Column);
    }
    return left;
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseAnd(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_AstNode* __attribute__((unused)) left = Amalgame_Compiler_Parser_ParseEquality(self);
    while (Amalgame_Compiler_Parser_CheckValue(self, "&&")) {
        Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Advance(self);
        Amalgame_Compiler_AstNode* __attribute__((unused)) right = Amalgame_Compiler_Parser_ParseEquality(self);
        left = Amalgame_Compiler_Ast_Binary(left, "&&", right, tok->Line, tok->Column);
    }
    return left;
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseEquality(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_AstNode* __attribute__((unused)) left = Amalgame_Compiler_Parser_ParseRelational(self);
    while (Amalgame_Compiler_Parser_CheckValue(self, "==") || Amalgame_Compiler_Parser_CheckValue(self, "!=")) {
        Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Advance(self);
        Amalgame_Compiler_AstNode* __attribute__((unused)) right = Amalgame_Compiler_Parser_ParseRelational(self);
        left = Amalgame_Compiler_Ast_Binary(left, tok->Value, right, tok->Line, tok->Column);
    }
    return left;
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseRelational(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_AstNode* __attribute__((unused)) left = Amalgame_Compiler_Parser_ParseAdd(self);
    while (Amalgame_Compiler_Parser_CheckValue(self, "<") || Amalgame_Compiler_Parser_CheckValue(self, ">") || Amalgame_Compiler_Parser_CheckValue(self, "<=") || Amalgame_Compiler_Parser_CheckValue(self, ">=")) {
        Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Advance(self);
        Amalgame_Compiler_AstNode* __attribute__((unused)) right = Amalgame_Compiler_Parser_ParseAdd(self);
        left = Amalgame_Compiler_Ast_Binary(left, tok->Value, right, tok->Line, tok->Column);
    }
    return left;
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseAdd(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_AstNode* __attribute__((unused)) left = Amalgame_Compiler_Parser_ParseMul(self);
    while (Amalgame_Compiler_Parser_CheckValue(self, "+") || Amalgame_Compiler_Parser_CheckValue(self, "-")) {
        Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Advance(self);
        Amalgame_Compiler_AstNode* __attribute__((unused)) right = Amalgame_Compiler_Parser_ParseMul(self);
        left = Amalgame_Compiler_Ast_Binary(left, tok->Value, right, tok->Line, tok->Column);
    }
    return left;
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseMul(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_AstNode* __attribute__((unused)) left = Amalgame_Compiler_Parser_ParseUnary(self);
    while (Amalgame_Compiler_Parser_CheckValue(self, "*") || Amalgame_Compiler_Parser_CheckValue(self, "/") || Amalgame_Compiler_Parser_CheckValue(self, "%")) {
        Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Advance(self);
        Amalgame_Compiler_AstNode* __attribute__((unused)) right = Amalgame_Compiler_Parser_ParseUnary(self);
        left = Amalgame_Compiler_Ast_Binary(left, tok->Value, right, tok->Line, tok->Column);
    }
    return left;
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseUnary(Amalgame_Compiler_Parser* self) {
    (void)self;
    code_bool __attribute__((unused)) isUnaryNot = Amalgame_Compiler_Parser_CheckValue(self, "!") && !Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_STRING);
    code_bool __attribute__((unused)) isUnaryMinus = Amalgame_Compiler_Parser_CheckValue(self, "-") && !Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_STRING);
    if (isUnaryNot || isUnaryMinus) {
        Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Advance(self);
        Amalgame_Compiler_AstNode* __attribute__((unused)) operand = Amalgame_Compiler_Parser_ParseUnary(self);
        Amalgame_Compiler_AstNode* __attribute__((unused)) node = Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_UNARY, tok->Line, tok->Column);
        node->Str = tok->Value;
        node->Left = operand;
        return node;
    }
    return Amalgame_Compiler_Parser_ParsePostfix(self);
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParsePostfix(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_AstNode* __attribute__((unused)) expr = Amalgame_Compiler_Parser_ParsePrimary(self);
    code_bool __attribute__((unused)) running = 1;
    while (running) {
        if (Amalgame_Compiler_Parser_CheckValue(self, ".")) {
            Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Advance(self);
            Amalgame_Compiler_Token* __attribute__((unused)) memberTok = Amalgame_Compiler_Parser_ExpectIdent(self);
            expr = Amalgame_Compiler_Ast_Member(expr, memberTok->Value, tok->Line, tok->Column);
            if (Amalgame_Compiler_Parser_CheckValue(self, "(")) {
                expr = Amalgame_Compiler_Parser_ParseCallArgs(self, expr);
            }
        } else if (Amalgame_Compiler_Parser_CheckValue(self, "(")) {
            expr = Amalgame_Compiler_Parser_ParseCallArgs(self, expr);
        } else if (Amalgame_Compiler_Parser_CheckValue(self, "[")) {
            Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Advance(self);
            Amalgame_Compiler_AstNode* __attribute__((unused)) idx = Amalgame_Compiler_Parser_ParseExpr(self);
            Amalgame_Compiler_Parser_Expect(self, "]");
            Amalgame_Compiler_AstNode* __attribute__((unused)) node = Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_INDEX_EXPR, tok->Line, tok->Column);
            node->Left = expr;
            node->Right = idx;
            expr = node;
        } else {
            running = 0;
        }
    }
    return expr;
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseCallArgs(Amalgame_Compiler_Parser* self, Amalgame_Compiler_AstNode* callee) {
    (void)self;
    (void)callee;
    Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Expect(self, "(");
    Amalgame_Compiler_AstNode* __attribute__((unused)) call = Amalgame_Compiler_Ast_Call(callee, tok->Line, tok->Column);
    self->ParenDepth = self->ParenDepth + 1;
    i64 __attribute__((unused)) myDepth = self->ParenDepth;
    Amalgame_Compiler_Parser_SkipNewlines(self);
    i64 __attribute__((unused)) callLastPos = 0;
    while (!Amalgame_Compiler_Parser_IsEnd(self)) {
        i64 __attribute__((unused)) callPos = self->Pos;
        if (callPos == callLastPos && callLastPos > 0) {
            Amalgame_Compiler_Parser_Advance(self);
            if (Amalgame_Compiler_Parser_IsEnd(self)) {
                break;
            }
        }
        callLastPos = self->Pos;
        code_bool __attribute__((unused)) isRparen = Amalgame_Compiler_Parser_CheckValue(self, ")") && !Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_STRING);
        if (isRparen && self->ParenDepth == myDepth) {
            break;
        }
        code_bool __attribute__((unused)) isComma = Amalgame_Compiler_Parser_CheckValue(self, ",") && !Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_STRING);
        if (isComma) {
            Amalgame_Compiler_Parser_Advance(self);
            continue;
        }
        Amalgame_Compiler_Parser_SkipNewlines(self);
        code_bool __attribute__((unused)) isRparen2 = Amalgame_Compiler_Parser_CheckValue(self, ")") && !Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_STRING);
        if (isRparen2 && self->ParenDepth == myDepth) {
            break;
        }
        if (Amalgame_Compiler_Parser_IsEnd(self)) {
            break;
        }
        Amalgame_Compiler_AstNode* __attribute__((unused)) arg = Amalgame_Compiler_Parser_ParseExpr(self);
        AmalgameList_add(call->Args, (void*)(intptr_t)(arg));
        Amalgame_Compiler_Parser_SkipNewlines(self);
    }
    self->ParenDepth = self->ParenDepth - 1;
    Amalgame_Compiler_Parser_Expect(self, ")");
    return call;
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParsePrimary(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Current(self);
    code_string __attribute__((unused)) v = tok->Value;
    if (Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_INTEGER)) {
        Amalgame_Compiler_Parser_Advance(self);
        Amalgame_Compiler_AstNode* __attribute__((unused)) lit = Amalgame_Compiler_Ast_IntLit(v, tok->Line, tok->Column);
        if (Amalgame_Compiler_Parser_CheckValue(self, "..")) {
            Amalgame_Compiler_Parser_Advance(self);
            Amalgame_Compiler_AstNode* __attribute__((unused)) right = Amalgame_Compiler_Parser_ParsePrimary(self);
            return Amalgame_Compiler_Ast_Binary(lit, "..", right, tok->Line, tok->Column);
        }
        return lit;
    }
    if (Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_FLOAT)) {
        Amalgame_Compiler_Parser_Advance(self);
        return Amalgame_Compiler_Ast_FloatLit(v, tok->Line, tok->Column);
    }
    if (Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_STRING)) {
        Amalgame_Compiler_Parser_Advance(self);
        return Amalgame_Compiler_Ast_StrLit(v, tok->Line, tok->Column);
    }
    if (code_string_equals(v, "true")) {
        Amalgame_Compiler_Parser_Advance(self);
        return Amalgame_Compiler_Ast_BoolLit(1, tok->Line, tok->Column);
    }
    if (code_string_equals(v, "false")) {
        Amalgame_Compiler_Parser_Advance(self);
        return Amalgame_Compiler_Ast_BoolLit(0, tok->Line, tok->Column);
    }
    if (code_string_equals(v, "null")) {
        Amalgame_Compiler_Parser_Advance(self);
        return Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_LITERAL_NULL, tok->Line, tok->Column);
    }
    if (code_string_equals(v, "this")) {
        Amalgame_Compiler_Parser_Advance(self);
        return Amalgame_Compiler_Ast_This(tok->Line, tok->Column);
    }
    if (code_string_equals(v, "new")) {
        return Amalgame_Compiler_Parser_ParseNew(self);
    }
    if (code_string_equals(v, "if")) {
        return Amalgame_Compiler_Parser_ParseIf(self);
    }
    if (code_string_equals(v, "(")) {
        Amalgame_Compiler_Parser_Advance(self);
        self->ParenDepth = self->ParenDepth + 1;
        Amalgame_Compiler_AstNode* __attribute__((unused)) first2 = Amalgame_Compiler_Parser_ParseExpr(self);
        if (Amalgame_Compiler_Parser_CheckValue(self, ",")) {
            Amalgame_Compiler_AstNode* __attribute__((unused)) tupleNode = Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_CALL, tok->Line, tok->Column);
            tupleNode->Name = "__tuple_literal__";
            AmalgameList_add(tupleNode->Args, (void*)(intptr_t)(first2));
            while (Amalgame_Compiler_Parser_CheckValue(self, ",")) {
                Amalgame_Compiler_Parser_Advance(self);
                if (Amalgame_Compiler_Parser_CheckValue(self, ")")) {
                    break;
                }
                Amalgame_Compiler_AstNode* __attribute__((unused)) elem = Amalgame_Compiler_Parser_ParseExpr(self);
                AmalgameList_add(tupleNode->Args, (void*)(intptr_t)(elem));
            }
            self->ParenDepth = self->ParenDepth - 1;
            Amalgame_Compiler_Parser_Expect(self, ")");
            return tupleNode;
        }
        self->ParenDepth = self->ParenDepth - 1;
        Amalgame_Compiler_Parser_Expect(self, ")");
        return first2;
    }
    if (Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_IDENTIFIER)) {
        Amalgame_Compiler_Parser_Advance(self);
        Amalgame_Compiler_AstNode* __attribute__((unused)) ident = Amalgame_Compiler_Ast_Ident(v, tok->Line, tok->Column);
        if (Amalgame_Compiler_Parser_CheckValue(self, "..")) {
            Amalgame_Compiler_Parser_Advance(self);
            Amalgame_Compiler_AstNode* __attribute__((unused)) right = Amalgame_Compiler_Parser_ParsePrimary(self);
            return Amalgame_Compiler_Ast_Binary(ident, "..", right, tok->Line, tok->Column);
        }
        return ident;
    }
    Amalgame_Compiler_Parser_Advance(self);
    return Amalgame_Compiler_Parser_Unknown(self);
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseMatch(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Advance(self);
    Amalgame_Compiler_AstNode* __attribute__((unused)) subject = Amalgame_Compiler_Parser_ParseExpr(self);
    Amalgame_Compiler_Parser_SkipNewlines(self);
    Amalgame_Compiler_Parser_Expect(self, "{");
    Amalgame_Compiler_Parser_SkipNewlines(self);
    Amalgame_Compiler_AstNode* __attribute__((unused)) matchNode = Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_IF_STMT, tok->Line, tok->Column);
    matchNode->Left = subject;
    matchNode->Name = "__match__";
    while (!Amalgame_Compiler_Parser_IsEnd(self) && !Amalgame_Compiler_Parser_CheckValue(self, "}")) {
        Amalgame_Compiler_Parser_SkipNewlines(self);
        if (Amalgame_Compiler_Parser_CheckValue(self, "}")) {
            break;
        }
        if (Amalgame_Compiler_Parser_CheckValue(self, ",")) {
            Amalgame_Compiler_Parser_Advance(self);
            continue;
        }
        Amalgame_Compiler_AstNode* __attribute__((unused)) patNode = Amalgame_Compiler_Parser_ParseMatchPattern(self);
        Amalgame_Compiler_Parser_SkipNewlines(self);
        if (Amalgame_Compiler_Parser_CheckValue(self, "=>")) {
            Amalgame_Compiler_Parser_Advance(self);
        }
        Amalgame_Compiler_Parser_SkipNewlines(self);
        Amalgame_Compiler_AstNode* __attribute__((unused)) armBody = Amalgame_Compiler_Parser_ParseExpr(self);
        if (Amalgame_Compiler_Parser_CheckValue(self, "{")) {
            armBody = Amalgame_Compiler_Parser_ParseBlock(self);
        }
        if (Amalgame_Compiler_Parser_CheckValue(self, ",")) {
            Amalgame_Compiler_Parser_Advance(self);
        }
        Amalgame_Compiler_AstNode* __attribute__((unused)) arm = Amalgame_Compiler_Ast_Binary(patNode, "=>", armBody, tok->Line, tok->Column);
        AmalgameList_add(matchNode->Children, (void*)(intptr_t)(arm));
        Amalgame_Compiler_Parser_SkipNewlines(self);
    }
    Amalgame_Compiler_Parser_Expect(self, "}");
    return matchNode;
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseMatchPattern(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Current(self);
    if (code_string_equals(tok->Value, "_")) {
        Amalgame_Compiler_Parser_Advance(self);
        Amalgame_Compiler_AstNode* __attribute__((unused)) wc = Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_IDENTIFIER, tok->Line, tok->Column);
        wc->Name = "_";
        return wc;
    }
    if (Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_INTEGER)) {
        Amalgame_Compiler_Token* __attribute__((unused)) num = Amalgame_Compiler_Parser_Advance(self);
        Amalgame_Compiler_AstNode* __attribute__((unused)) lit = Amalgame_Compiler_Ast_IntLit(num->Value, num->Line, num->Column);
        if (Amalgame_Compiler_Parser_CheckValue(self, "..")) {
            Amalgame_Compiler_Parser_Advance(self);
            Amalgame_Compiler_AstNode* __attribute__((unused)) right = Amalgame_Compiler_Parser_ParsePrimary(self);
            return Amalgame_Compiler_Ast_Binary(lit, "..", right, num->Line, num->Column);
        }
        return lit;
    }
    if (Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_IDENTIFIER)) {
        Amalgame_Compiler_Token* __attribute__((unused)) nameTok = Amalgame_Compiler_Parser_Advance(self);
        Amalgame_Compiler_AstNode* __attribute__((unused)) patIdent = Amalgame_Compiler_Ast_Ident(nameTok->Value, nameTok->Line, nameTok->Column);
        if (Amalgame_Compiler_Parser_CheckValue(self, "(")) {
            Amalgame_Compiler_Parser_Advance(self);
            Amalgame_Compiler_AstNode* __attribute__((unused)) captures = Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_CALL, nameTok->Line, nameTok->Column);
            captures->Name = nameTok->Value;
            while (!Amalgame_Compiler_Parser_IsEnd(self) && !Amalgame_Compiler_Parser_CheckValue(self, ")")) {
                if (Amalgame_Compiler_Parser_CheckValue(self, ",")) {
                    Amalgame_Compiler_Parser_Advance(self);
                    continue;
                }
                if (Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_IDENTIFIER)) {
                    Amalgame_Compiler_Token* __attribute__((unused)) cap = Amalgame_Compiler_Parser_Advance(self);
                    Amalgame_Compiler_AstNode* __attribute__((unused)) capNode = Amalgame_Compiler_Ast_Ident(cap->Value, cap->Line, cap->Column);
                    AmalgameList_add(captures->Args, (void*)(intptr_t)(capNode));
                } else {
                    Amalgame_Compiler_Parser_Advance(self);
                }
            }
            Amalgame_Compiler_Parser_Expect(self, ")");
            return captures;
        }
        return patIdent;
    }
    Amalgame_Compiler_Parser_Advance(self);
    return Amalgame_Compiler_Parser_Unknown(self);
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseNew(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Advance(self);
    code_string __attribute__((unused)) typeName = Amalgame_Compiler_Parser_ParseQualifiedName(self);
    if (Amalgame_Compiler_Parser_CheckValue(self, "<")) {
        Amalgame_Compiler_Parser_Advance(self);
        i64 __attribute__((unused)) depth = 1;
        while (!Amalgame_Compiler_Parser_IsEnd(self) && depth > 0) {
            Amalgame_Compiler_Token* __attribute__((unused)) inner = Amalgame_Compiler_Parser_Current(self);
            code_string __attribute__((unused)) iv = inner->Value;
            if (code_string_equals(iv, "<")) {
                depth = depth + 1;
            }
            if (code_string_equals(iv, ">")) {
                depth = depth - 1;
            }
            Amalgame_Compiler_Parser_Advance(self);
        }
    }
    Amalgame_Compiler_AstNode* __attribute__((unused)) node = Amalgame_Compiler_Ast_NewExpr(typeName, tok->Line, tok->Column);
    if (Amalgame_Compiler_Parser_CheckValue(self, "(")) {
        Amalgame_Compiler_Parser_Advance(self);
        while (!Amalgame_Compiler_Parser_IsEnd(self) && !Amalgame_Compiler_Parser_CheckValue(self, ")")) {
            if (Amalgame_Compiler_Parser_CheckValue(self, ",")) {
                Amalgame_Compiler_Parser_Advance(self);
                continue;
            }
            Amalgame_Compiler_AstNode* __attribute__((unused)) arg = Amalgame_Compiler_Parser_ParseExpr(self);
            AmalgameList_add(node->Args, (void*)(intptr_t)(arg));
        }
        Amalgame_Compiler_Parser_Expect(self, ")");
    }
    return node;
}

struct _Amalgame_Compiler_Emitter {
    AmalgameList* Lines;
    i64 Indent;
};

void Amalgame_Compiler_Emitter_Emit(Amalgame_Compiler_Emitter* self, code_string text);
void Amalgame_Compiler_Emitter_EmitLine(Amalgame_Compiler_Emitter* self, code_string text);
void Amalgame_Compiler_Emitter_EmitBlank(Amalgame_Compiler_Emitter* self);
void Amalgame_Compiler_Emitter_Indent_(Amalgame_Compiler_Emitter* self);
void Amalgame_Compiler_Emitter_Dedent(Amalgame_Compiler_Emitter* self);
code_string Amalgame_Compiler_Emitter_GetOutput(Amalgame_Compiler_Emitter* self);

Amalgame_Compiler_Emitter* Amalgame_Compiler_Emitter_new() {
    Amalgame_Compiler_Emitter* self = (Amalgame_Compiler_Emitter*) GC_MALLOC(sizeof(Amalgame_Compiler_Emitter));
    self->Lines = AmalgameList_new();
    self->Indent = 0;
    return self;
}

void Amalgame_Compiler_Emitter_Emit(Amalgame_Compiler_Emitter* self, code_string text) {
    (void)self;
    (void)text;
    AmalgameList_add(self->Lines, (void*)(intptr_t)(text));
}

void Amalgame_Compiler_Emitter_EmitLine(Amalgame_Compiler_Emitter* self, code_string text) {
    (void)self;
    (void)text;
    code_string __attribute__((unused)) line = "";
    i64 __attribute__((unused)) i = self->Indent;
    for (i64 k = 0; k < i; k++) {
        line = code_string_concat(line, "    ");
    }
    line = code_string_concat(line, text);
    AmalgameList_add(self->Lines, (void*)(intptr_t)(line));
}

void Amalgame_Compiler_Emitter_EmitBlank(Amalgame_Compiler_Emitter* self) {
    (void)self;
    AmalgameList_add(self->Lines, (void*)(intptr_t)(""));
}

void Amalgame_Compiler_Emitter_Indent_(Amalgame_Compiler_Emitter* self) {
    (void)self;
    self->Indent = self->Indent + 1;
}

void Amalgame_Compiler_Emitter_Dedent(Amalgame_Compiler_Emitter* self) {
    (void)self;
    self->Indent = self->Indent - 1;
}

code_string Amalgame_Compiler_Emitter_GetOutput(Amalgame_Compiler_Emitter* self) {
    (void)self;
    code_string __attribute__((unused)) result = "";
    i64 __attribute__((unused)) count = AmalgameList_count(self->Lines);
    for (i64 i = 0; i < count; i++) {
        code_string __attribute__((unused)) line = (code_string)AmalgameList_get(self->Lines, i);
        result = code_string_concat(code_string_concat(result, line), "\n");
    }
    return result;
}

struct _Amalgame_Compiler_CGen {
    Amalgame_Compiler_Emitter* Out;
    code_string NsPrefix;
    AmalgameList* LocalTypeNames;
    AmalgameList* LocalTypeCType;
    code_string CurrentClass;
    code_string CurrentRetType;
    AmalgameList* FieldNames;
    AmalgameList* FieldCTypes;
    AmalgameList* ListElemNames;
    AmalgameList* ListElemCTypes;
    AmalgameList* EnumNames;
    AmalgameList* MethodRetNames;
    AmalgameList* MethodRetTypes;
};

code_string Amalgame_Compiler_CGen_Generate(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* prog);
void Amalgame_Compiler_CGen_BeginMulti(Amalgame_Compiler_CGen* self, code_string ns);
void Amalgame_Compiler_CGen_AddFilePass1(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* prog);
void Amalgame_Compiler_CGen_EmitSeparator(Amalgame_Compiler_CGen* self);
void Amalgame_Compiler_CGen_AddFilePass2(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* prog);
code_string Amalgame_Compiler_CGen_FinishMulti(Amalgame_Compiler_CGen* self);
AmalgameList* Amalgame_Compiler_CGen_GetLines(Amalgame_Compiler_CGen* self);
static code_string Amalgame_Compiler_CGen_SymName(Amalgame_Compiler_CGen* self, code_string name);
static void Amalgame_Compiler_CGen_LocalTypeSet(Amalgame_Compiler_CGen* self, code_string varName, code_string ctype);
static code_string Amalgame_Compiler_CGen_LocalTypeGet(Amalgame_Compiler_CGen* self, code_string varName);
static void Amalgame_Compiler_CGen_LocalTypeClear(Amalgame_Compiler_CGen* self);
static void Amalgame_Compiler_CGen_FieldTypeSet(Amalgame_Compiler_CGen* self, code_string className, code_string fieldName, code_string ctype);
static code_string Amalgame_Compiler_CGen_FieldTypeGet(Amalgame_Compiler_CGen* self, code_string className, code_string fieldName);
static void Amalgame_Compiler_CGen_ListElemSet(Amalgame_Compiler_CGen* self, code_string className, code_string fieldName, code_string elemCType);
static code_string Amalgame_Compiler_CGen_ListElemGet(Amalgame_Compiler_CGen* self, code_string className, code_string fieldName);
static void Amalgame_Compiler_CGen_MethodRetSet(Amalgame_Compiler_CGen* self, code_string className, code_string methodName, code_string ctype);
static code_string Amalgame_Compiler_CGen_MethodRetGet(Amalgame_Compiler_CGen* self, code_string className, code_string methodName);
static code_string Amalgame_Compiler_CGen_InferTypeFromExpr(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* expr);
static code_string Amalgame_Compiler_CGen_EmitInterpolatedString(Amalgame_Compiler_CGen* self, code_string raw);
static code_string Amalgame_Compiler_CGen_InterpExprToC(Amalgame_Compiler_CGen* self, code_string expr);
static code_string Amalgame_Compiler_CGen_EscapeStringForC(Amalgame_Compiler_CGen* self, code_string raw);
static void Amalgame_Compiler_CGen_EmitHeader(Amalgame_Compiler_CGen* self);
void Amalgame_Compiler_CGen_EmitNetHeader(Amalgame_Compiler_CGen* self);
static void Amalgame_Compiler_CGen_EmitForwardDecl(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* decl);
static void Amalgame_Compiler_CGen_EmitDecl(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* decl);
static void Amalgame_Compiler_CGen_EmitEnum(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* en);
static void Amalgame_Compiler_CGen_EmitClass(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* cls);
static code_string Amalgame_Compiler_CGen_TupleStructName(Amalgame_Compiler_CGen* self, code_string tupleType);
static void Amalgame_Compiler_CGen_EnsureTupleStruct(Amalgame_Compiler_CGen* self, code_string tupleType);
static code_string Amalgame_Compiler_CGen_MethodSig(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* method, code_string className);
static void Amalgame_Compiler_CGen_EmitMethod(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* method, code_string className);
static code_string Amalgame_Compiler_CGen_EmitIfBranch(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* block);
static void Amalgame_Compiler_CGen_EmitMatch(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* stmt);
static void Amalgame_Compiler_CGen_EmitMatchBody(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* body);
static void Amalgame_Compiler_CGen_EmitIf(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* stmt);
static void Amalgame_Compiler_CGen_EmitIfTail(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* stmt);
static void Amalgame_Compiler_CGen_EmitBlock(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* block);
static void Amalgame_Compiler_CGen_EmitStmt(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* stmt);
static code_string Amalgame_Compiler_CGen_EmitExprStr(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* expr);
static code_string Amalgame_Compiler_CGen_TryEmitListCall(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* callExpr);
static code_string Amalgame_Compiler_CGen_EmitCalleeStr(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* callee);
static code_bool Amalgame_Compiler_CGen_IsEnum(Amalgame_Compiler_CGen* self, code_string t);
static code_string Amalgame_Compiler_CGen_TypeToC(Amalgame_Compiler_CGen* self, code_string t);

Amalgame_Compiler_CGen* Amalgame_Compiler_CGen_new() {
    Amalgame_Compiler_CGen* self = (Amalgame_Compiler_CGen*) GC_MALLOC(sizeof(Amalgame_Compiler_CGen));
    self->Out = Amalgame_Compiler_Emitter_new();
    self->NsPrefix = "";
    self->LocalTypeNames = AmalgameList_new();
    self->LocalTypeCType = AmalgameList_new();
    self->CurrentClass = "";
    self->CurrentRetType = "";
    self->FieldNames = AmalgameList_new();
    self->FieldCTypes = AmalgameList_new();
    self->ListElemNames = AmalgameList_new();
    self->ListElemCTypes = AmalgameList_new();
    self->EnumNames = AmalgameList_new();
    self->MethodRetNames = AmalgameList_new();
    self->MethodRetTypes = AmalgameList_new();
    Amalgame_Compiler_CGen_FieldTypeSet(self, "AmalgameHttpResponse", "Status", "i64");
    Amalgame_Compiler_CGen_FieldTypeSet(self, "AmalgameHttpResponse", "Body", "code_string");
    Amalgame_Compiler_CGen_FieldTypeSet(self, "AmalgameHttpResponse", "Error", "code_string");
    Amalgame_Compiler_CGen_FieldTypeSet(self, "AmalgameHttpResponse", "Ok", "code_bool");
    Amalgame_Compiler_CGen_FieldTypeSet(self, "AmalgameTcpConn", "RemoteIp", "code_string");
    Amalgame_Compiler_CGen_FieldTypeSet(self, "AmalgameTcpConn", "Fd", "i64");
    Amalgame_Compiler_CGen_FieldTypeSet(self, "AmalgameTcpServer", "Fd", "i64");
    Amalgame_Compiler_CGen_FieldTypeSet(self, "AmalgameTcpServer", "Port", "i64");
    return self;
}

code_string Amalgame_Compiler_CGen_Generate(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* prog) {
    (void)self;
    (void)prog;
    code_string __attribute__((unused)) ns = prog->Str;
    if (String_Length(ns) > 0) {
        self->NsPrefix = String_Replace(ns, ".", "_");
    }
    Amalgame_Compiler_CGen_EmitHeader(self);
    i64 __attribute__((unused)) decls = AmalgameList_count(prog->Children);
    for (i64 i = 0; i < decls; i++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) decl = (Amalgame_Compiler_AstNode*)AmalgameList_get(prog->Children, i);
        Amalgame_Compiler_CGen_EmitForwardDecl(self, decl);
    }
    Amalgame_Compiler_Emitter_EmitBlank(self->Out);
    for (i64 j = 0; j < decls; j++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) decl = (Amalgame_Compiler_AstNode*)AmalgameList_get(prog->Children, j);
        Amalgame_Compiler_CGen_EmitDecl(self, decl);
    }
    return Amalgame_Compiler_Emitter_GetOutput(self->Out);
}

void Amalgame_Compiler_CGen_BeginMulti(Amalgame_Compiler_CGen* self, code_string ns) {
    (void)self;
    (void)ns;
    if (String_Length(ns) > 0) {
        self->NsPrefix = String_Replace(ns, ".", "_");
    }
    Amalgame_Compiler_CGen_EmitHeader(self);
}

void Amalgame_Compiler_CGen_AddFilePass1(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* prog) {
    (void)self;
    (void)prog;
    i64 __attribute__((unused)) decls = AmalgameList_count(prog->Children);
    for (i64 i = 0; i < decls; i++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) decl = (Amalgame_Compiler_AstNode*)AmalgameList_get(prog->Children, i);
        Amalgame_Compiler_CGen_EmitForwardDecl(self, decl);
    }
}

void Amalgame_Compiler_CGen_EmitSeparator(Amalgame_Compiler_CGen* self) {
    (void)self;
    Amalgame_Compiler_Emitter_EmitBlank(self->Out);
}

void Amalgame_Compiler_CGen_AddFilePass2(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* prog) {
    (void)self;
    (void)prog;
    i64 __attribute__((unused)) decls = AmalgameList_count(prog->Children);
    for (i64 j = 0; j < decls; j++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) decl = (Amalgame_Compiler_AstNode*)AmalgameList_get(prog->Children, j);
        Amalgame_Compiler_CGen_EmitDecl(self, decl);
    }
}

code_string Amalgame_Compiler_CGen_FinishMulti(Amalgame_Compiler_CGen* self) {
    (void)self;
    return Amalgame_Compiler_Emitter_GetOutput(self->Out);
}

AmalgameList* Amalgame_Compiler_CGen_GetLines(Amalgame_Compiler_CGen* self) {
    (void)self;
    return self->Out->Lines;
}

static code_string Amalgame_Compiler_CGen_SymName(Amalgame_Compiler_CGen* self, code_string name) {
    (void)self;
    (void)name;
    code_string __attribute__((unused)) ns = self->NsPrefix;
    if (String_Length(ns) > 0) {
        return code_string_concat(code_string_concat(ns, "_"), name);
    }
    return name;
}

static void Amalgame_Compiler_CGen_LocalTypeSet(Amalgame_Compiler_CGen* self, code_string varName, code_string ctype) {
    (void)self;
    (void)varName;
    (void)ctype;
    i64 __attribute__((unused)) n = AmalgameList_count(self->LocalTypeNames);
    for (i64 i = 0; i < n; i++) {
        code_string __attribute__((unused)) k = (code_string)AmalgameList_get(self->LocalTypeNames, i);
        if (code_string_equals(k, varName)) {
            AmalgameList_add(self->LocalTypeNames, (void*)(intptr_t)(varName));
            AmalgameList_add(self->LocalTypeCType, (void*)(intptr_t)(ctype));
            return;
        }
    }
    AmalgameList_add(self->LocalTypeNames, (void*)(intptr_t)(varName));
    AmalgameList_add(self->LocalTypeCType, (void*)(intptr_t)(ctype));
}

static code_string Amalgame_Compiler_CGen_LocalTypeGet(Amalgame_Compiler_CGen* self, code_string varName) {
    (void)self;
    (void)varName;
    i64 __attribute__((unused)) n = AmalgameList_count(self->LocalTypeNames);
    code_string __attribute__((unused)) result = "";
    for (i64 i = 0; i < n; i++) {
        code_string __attribute__((unused)) k = (code_string)AmalgameList_get(self->LocalTypeNames, i);
        if (code_string_equals(k, varName)) {
            result = (code_string)AmalgameList_get(self->LocalTypeCType, i);
        }
    }
    return result;
}

static void Amalgame_Compiler_CGen_LocalTypeClear(Amalgame_Compiler_CGen* self) {
    (void)self;
    self->LocalTypeNames = AmalgameList_new();
    self->LocalTypeCType = AmalgameList_new();
}

static void Amalgame_Compiler_CGen_FieldTypeSet(Amalgame_Compiler_CGen* self, code_string className, code_string fieldName, code_string ctype) {
    (void)self;
    (void)className;
    (void)fieldName;
    (void)ctype;
    code_string __attribute__((unused)) key = code_string_concat(code_string_concat(className, "."), fieldName);
    AmalgameList_add(self->FieldNames, (void*)(intptr_t)(key));
    AmalgameList_add(self->FieldCTypes, (void*)(intptr_t)(ctype));
}

static code_string Amalgame_Compiler_CGen_FieldTypeGet(Amalgame_Compiler_CGen* self, code_string className, code_string fieldName) {
    (void)self;
    (void)className;
    (void)fieldName;
    code_string __attribute__((unused)) key = code_string_concat(code_string_concat(className, "."), fieldName);
    i64 __attribute__((unused)) n = AmalgameList_count(self->FieldNames);
    code_string __attribute__((unused)) result = "";
    for (i64 i = 0; i < n; i++) {
        code_string __attribute__((unused)) k = (code_string)AmalgameList_get(self->FieldNames, i);
        if (code_string_equals(k, key)) {
            result = (code_string)AmalgameList_get(self->FieldCTypes, i);
        }
    }
    return result;
}

static void Amalgame_Compiler_CGen_ListElemSet(Amalgame_Compiler_CGen* self, code_string className, code_string fieldName, code_string elemCType) {
    (void)self;
    (void)className;
    (void)fieldName;
    (void)elemCType;
    code_string __attribute__((unused)) key = code_string_concat(code_string_concat(className, "."), fieldName);
    AmalgameList_add(self->ListElemNames, (void*)(intptr_t)(key));
    AmalgameList_add(self->ListElemCTypes, (void*)(intptr_t)(elemCType));
}

static code_string Amalgame_Compiler_CGen_ListElemGet(Amalgame_Compiler_CGen* self, code_string className, code_string fieldName) {
    (void)self;
    (void)className;
    (void)fieldName;
    code_string __attribute__((unused)) key = code_string_concat(code_string_concat(className, "."), fieldName);
    i64 __attribute__((unused)) n = AmalgameList_count(self->ListElemNames);
    code_string __attribute__((unused)) result = "";
    for (i64 i = 0; i < n; i++) {
        code_string __attribute__((unused)) k = (code_string)AmalgameList_get(self->ListElemNames, i);
        if (code_string_equals(k, key)) {
            result = (code_string)AmalgameList_get(self->ListElemCTypes, i);
        }
    }
    return result;
}

static void Amalgame_Compiler_CGen_MethodRetSet(Amalgame_Compiler_CGen* self, code_string className, code_string methodName, code_string ctype) {
    (void)self;
    (void)className;
    (void)methodName;
    (void)ctype;
    code_string __attribute__((unused)) key = code_string_concat(code_string_concat(className, "::"), methodName);
    AmalgameList_add(self->MethodRetNames, (void*)(intptr_t)(key));
    AmalgameList_add(self->MethodRetTypes, (void*)(intptr_t)(ctype));
}

static code_string Amalgame_Compiler_CGen_MethodRetGet(Amalgame_Compiler_CGen* self, code_string className, code_string methodName) {
    (void)self;
    (void)className;
    (void)methodName;
    code_string __attribute__((unused)) key = code_string_concat(code_string_concat(className, "::"), methodName);
    i64 __attribute__((unused)) n = AmalgameList_count(self->MethodRetNames);
    code_string __attribute__((unused)) result = "";
    for (i64 i = 0; i < n; i++) {
        code_string __attribute__((unused)) k = (code_string)AmalgameList_get(self->MethodRetNames, i);
        if (code_string_equals(k, key)) {
            result = (code_string)AmalgameList_get(self->MethodRetTypes, i);
        }
    }
    return result;
}

static code_string Amalgame_Compiler_CGen_InferTypeFromExpr(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* expr) {
    (void)self;
    (void)expr;
    if (expr == NULL) {
        return "";
    }
    Amalgame_Compiler_NodeKind __attribute__((unused)) k = expr->Kind;
    if (k == Amalgame_Compiler_NodeKind_LITERAL_INT) {
        return "i64";
    }
    if (k == Amalgame_Compiler_NodeKind_LITERAL_FLOAT) {
        return "double";
    }
    if (k == Amalgame_Compiler_NodeKind_LITERAL_STRING) {
        return "code_string";
    }
    if (k == Amalgame_Compiler_NodeKind_LITERAL_BOOL) {
        return "code_bool";
    }
    if (k == Amalgame_Compiler_NodeKind_BINARY) {
        code_string __attribute__((unused)) bop = expr->Str;
        code_bool __attribute__((unused)) isCmpOp = code_string_equals(bop, "&&") || code_string_equals(bop, "||") || code_string_equals(bop, "==") || code_string_equals(bop, "!=");
        code_bool __attribute__((unused)) isRelOp = code_string_equals(bop, "<") || code_string_equals(bop, ">") || code_string_equals(bop, "<=") || code_string_equals(bop, ">=");
        if (isCmpOp || isRelOp) {
            return "code_bool";
        }
        if (code_string_equals(bop, "+") || code_string_equals(bop, "-") || code_string_equals(bop, "*") || code_string_equals(bop, "/") || code_string_equals(bop, "%")) {
            code_string __attribute__((unused)) lt = Amalgame_Compiler_CGen_InferTypeFromExpr(self, expr->Left);
            code_string __attribute__((unused)) rt = Amalgame_Compiler_CGen_InferTypeFromExpr(self, expr->Right);
            if (code_string_equals(lt, "code_string") || code_string_equals(rt, "code_string")) {
                return "code_string";
            }
            if (expr->Left != NULL) {
                if (expr->Left->Kind == Amalgame_Compiler_NodeKind_LITERAL_STRING) {
                    return "code_string";
                }
                if (expr->Left->Kind == Amalgame_Compiler_NodeKind_LITERAL_INT) {
                    return "i64";
                }
            }
            if (expr->Right != NULL) {
                if (expr->Right->Kind == Amalgame_Compiler_NodeKind_LITERAL_STRING) {
                    return "code_string";
                }
                if (expr->Right->Kind == Amalgame_Compiler_NodeKind_LITERAL_INT) {
                    return "i64";
                }
            }
            if (code_string_equals(lt, "i64") || code_string_equals(rt, "i64")) {
                return "i64";
            }
            if (code_string_equals(lt, "double") || code_string_equals(rt, "double")) {
                return "double";
            }
        }
    }
    if (k == Amalgame_Compiler_NodeKind_UNARY) {
        if (code_string_equals(expr->Str, "!")) {
            return "code_bool";
        }
    }
    if (k == Amalgame_Compiler_NodeKind_IDENTIFIER) {
        return Amalgame_Compiler_CGen_LocalTypeGet(self, expr->Name);
    }
    if (k == Amalgame_Compiler_NodeKind_MEMBER) {
        if (expr->Left != NULL) {
            Amalgame_Compiler_NodeKind __attribute__((unused)) lk = expr->Left->Kind;
            if (lk == Amalgame_Compiler_NodeKind_THIS_EXPR) {
                return Amalgame_Compiler_CGen_FieldTypeGet(self, self->CurrentClass, expr->Name);
            }
            if (lk == Amalgame_Compiler_NodeKind_IDENTIFIER) {
                code_string __attribute__((unused)) lname = expr->Left->Name;
                code_string __attribute__((unused)) varCType = Amalgame_Compiler_CGen_LocalTypeGet(self, lname);
                code_string __attribute__((unused)) bare = String_Replace(varCType, "*", "");
                if (String_Length(bare) > 0) {
                    return Amalgame_Compiler_CGen_FieldTypeGet(self, bare, expr->Name);
                }
                code_string __attribute__((unused)) symT = Amalgame_Compiler_CGen_SymName(self, lname);
                if (Amalgame_Compiler_CGen_IsEnum(self, symT)) {
                    return symT;
                }
                if (Amalgame_Compiler_CGen_IsEnum(self, lname)) {
                    return Amalgame_Compiler_CGen_SymName(self, lname);
                }
            }
            if (lk == Amalgame_Compiler_NodeKind_CALL) {
                code_string __attribute__((unused)) callRetT = Amalgame_Compiler_CGen_InferTypeFromExpr(self, expr->Left);
                code_string __attribute__((unused)) bareCall = String_Replace(callRetT, "*", "");
                if (String_Length(bareCall) > 0) {
                    code_string __attribute__((unused)) ft = Amalgame_Compiler_CGen_FieldTypeGet(self, bareCall, expr->Name);
                    if (String_Length(ft) > 0) {
                        return ft;
                    }
                }
            }
            if (lk == Amalgame_Compiler_NodeKind_MEMBER) {
                if (expr->Left->Left != NULL) {
                    Amalgame_Compiler_NodeKind __attribute__((unused)) llk2 = expr->Left->Left->Kind;
                    if (llk2 == Amalgame_Compiler_NodeKind_THIS_EXPR) {
                        code_string __attribute__((unused)) fname2 = expr->Left->Name;
                        code_string __attribute__((unused)) parentT = Amalgame_Compiler_CGen_FieldTypeGet(self, self->CurrentClass, fname2);
                        code_string __attribute__((unused)) bareP = String_Replace(parentT, "*", "");
                        if (String_Length(bareP) > 0) {
                            code_string __attribute__((unused)) ft2 = Amalgame_Compiler_CGen_FieldTypeGet(self, bareP, expr->Name);
                            if (String_Length(ft2) > 0) {
                                return ft2;
                            }
                        }
                    }
                    if (llk2 == Amalgame_Compiler_NodeKind_IDENTIFIER) {
                        code_string __attribute__((unused)) vname3 = expr->Left->Left->Name;
                        code_string __attribute__((unused)) vtype3 = Amalgame_Compiler_CGen_LocalTypeGet(self, vname3);
                        code_string __attribute__((unused)) bareV = String_Replace(vtype3, "*", "");
                        if (String_Length(bareV) > 0) {
                            code_string __attribute__((unused)) fname3 = expr->Left->Name;
                            code_string __attribute__((unused)) parentT3 = Amalgame_Compiler_CGen_FieldTypeGet(self, bareV, fname3);
                            code_string __attribute__((unused)) bareP3 = String_Replace(parentT3, "*", "");
                            if (String_Length(bareP3) > 0) {
                                code_string __attribute__((unused)) ft3 = Amalgame_Compiler_CGen_FieldTypeGet(self, bareP3, expr->Name);
                                if (String_Length(ft3) > 0) {
                                    return ft3;
                                }
                            }
                        }
                    }
                    if (llk2 == Amalgame_Compiler_NodeKind_MEMBER) {
                        if (expr->Left->Left->Left != NULL) {
                            Amalgame_Compiler_AstNode* __attribute__((unused)) lll = expr->Left->Left->Left;
                            if (lll->Kind == Amalgame_Compiler_NodeKind_IDENTIFIER) {
                                code_string __attribute__((unused)) vn4 = lll->Name;
                                code_string __attribute__((unused)) vt4 = Amalgame_Compiler_CGen_LocalTypeGet(self, vn4);
                                code_string __attribute__((unused)) bare4 = String_Replace(vt4, "*", "");
                                if (String_Length(bare4) > 0) {
                                    code_string __attribute__((unused)) fn4 = expr->Left->Left->Name;
                                    code_string __attribute__((unused)) pt4 = Amalgame_Compiler_CGen_FieldTypeGet(self, bare4, fn4);
                                    code_string __attribute__((unused)) bp4 = String_Replace(pt4, "*", "");
                                    if (String_Length(bp4) > 0) {
                                        code_string __attribute__((unused)) fn5 = expr->Left->Name;
                                        code_string __attribute__((unused)) pt5 = Amalgame_Compiler_CGen_FieldTypeGet(self, bp4, fn5);
                                        code_string __attribute__((unused)) bp5 = String_Replace(pt5, "*", "");
                                        if (String_Length(bp5) > 0) {
                                            code_string __attribute__((unused)) ft5 = Amalgame_Compiler_CGen_FieldTypeGet(self, bp5, expr->Name);
                                            if (String_Length(ft5) > 0) {
                                                return ft5;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return "";
    }
    if (k == Amalgame_Compiler_NodeKind_NEW_EXPR) {
        code_string __attribute__((unused)) tname = expr->Name;
        if (String_StartsWith(tname, "List<") || code_string_equals(tname, "List")) {
            return "AmalgameList*";
        }
        if (String_StartsWith(tname, "Map<") || code_string_equals(tname, "Map")) {
            return "AmalgameMap*";
        }
        if (String_StartsWith(tname, "Set<") || code_string_equals(tname, "Set")) {
            return "AmalgameSet*";
        }
        if (Amalgame_Compiler_CGen_IsEnum(self, tname)) {
            return Amalgame_Compiler_CGen_SymName(self, tname);
        }
        return code_string_concat(Amalgame_Compiler_CGen_SymName(self, tname), "*");
    }
    if (k == Amalgame_Compiler_NodeKind_CALL) {
        if (expr->Left != NULL) {
            code_string __attribute__((unused)) calleeStr = Amalgame_Compiler_CGen_EmitCalleeStr(self, expr->Left);
            if (code_string_equals(calleeStr, "AmalgameMap_has") || code_string_equals(calleeStr, "AmalgameMap_remove") || code_string_equals(calleeStr, "AmalgameSet_contains") || code_string_equals(calleeStr, "AmalgameSet_remove") || code_string_equals(calleeStr, "AmalgameSet_add")) {
                return "code_bool";
            }
            if (code_string_equals(calleeStr, "AmalgameMap_size") || code_string_equals(calleeStr, "AmalgameSet_size")) {
                return "i64";
            }
            if (code_string_equals(calleeStr, "AmalgameList_isEmpty") || code_string_equals(calleeStr, "AmalgameList_remove")) {
                return "code_bool";
            }
            code_string __attribute__((unused)) algPrefix = "__alg__";
            i64 __attribute__((unused)) enCount2 = AmalgameList_count(self->EnumNames);
            for (i64 eni = 0; eni < enCount2; eni++) {
                code_string __attribute__((unused)) ename = (code_string)AmalgameList_get(self->EnumNames, eni);
                if (String_StartsWith(ename, algPrefix)) {
                    code_string __attribute__((unused)) enumType = String_Substring(ename, String_Length(algPrefix), String_Length(ename) - String_Length(algPrefix));
                    if (String_StartsWith(calleeStr, code_string_concat(enumType, "_"))) {
                        return enumType;
                    }
                }
            }
            if (expr->Left != NULL && expr->Left->Kind == Amalgame_Compiler_NodeKind_IDENTIFIER) {
                code_string __attribute__((unused)) calleeLocalType = Amalgame_Compiler_CGen_LocalTypeGet(self, expr->Left->Name);
                if (code_string_equals(calleeLocalType, "__macro__")) {
                    return "i64";
                }
            }
            if (code_string_equals(calleeStr, "String_Length") || code_string_equals(calleeStr, "String_IndexOf") || code_string_equals(calleeStr, "String_LastIndexOf")) {
                return "i64";
            }
            if (code_string_equals(calleeStr, "String_Contains") || code_string_equals(calleeStr, "String_StartsWith") || code_string_equals(calleeStr, "String_EndsWith") || code_string_equals(calleeStr, "String_IsEmpty")) {
                return "code_bool";
            }
            if (String_StartsWith(calleeStr, "String_")) {
                return "code_string";
            }
            if (String_EndsWith(calleeStr, "_CharAt")) {
                return "code_string";
            }
            if (code_string_equals(calleeStr, "code_string_concat")) {
                return "code_string";
            }
            if (code_string_equals(calleeStr, "AmalgameList_count")) {
                return "i64";
            }
            if (String_StartsWith(calleeStr, "AmalgameList_count")) {
                return "i64";
            }
            if (code_string_equals(calleeStr, "File_Exists") || code_string_equals(calleeStr, "File_WriteAll") || code_string_equals(calleeStr, "File_AppendAll") || code_string_equals(calleeStr, "File_Delete")) {
                return "code_bool";
            }
            if (code_string_equals(calleeStr, "File_ReadAll") || code_string_equals(calleeStr, "File_ReadLine")) {
                return "code_string";
            }
            if (code_string_equals(calleeStr, "Http_Get") || code_string_equals(calleeStr, "Http_Post") || code_string_equals(calleeStr, "Http_GetWithHeaders") || code_string_equals(calleeStr, "Http_GetTimeout") || code_string_equals(calleeStr, "Http_PostJson")) {
                return "AmalgameHttpResponse*";
            }
            if (code_string_equals(calleeStr, "TcpServer_Listen")) {
                return "AmalgameTcpServer*";
            }
            if (code_string_equals(calleeStr, "TcpServer_Accept")) {
                return "AmalgameTcpConn*";
            }
            if (code_string_equals(calleeStr, "TcpServer_IsListening") || code_string_equals(calleeStr, "TcpConn_Send") || code_string_equals(calleeStr, "TcpConn_Close") || code_string_equals(calleeStr, "TcpServer_Stop")) {
                return "code_bool";
            }
            if (code_string_equals(calleeStr, "TcpConn_Receive")) {
                return "code_string";
            }
            if (code_string_equals(calleeStr, "Math_Sqrt") || code_string_equals(calleeStr, "Math_Abs") || code_string_equals(calleeStr, "Math_Floor") || code_string_equals(calleeStr, "Math_Ceil") || code_string_equals(calleeStr, "Math_Round") || code_string_equals(calleeStr, "Math_Pow") || code_string_equals(calleeStr, "Math_Log")) {
                return "double";
            }
            if (code_string_equals(calleeStr, "Math_Max") || code_string_equals(calleeStr, "Math_Min")) {
                return "i64";
            }
            if (code_string_equals(calleeStr, "Math_IsPrime") || code_string_equals(calleeStr, "Math_IsNaN") || code_string_equals(calleeStr, "Math_IsInf") || code_string_equals(calleeStr, "Math_IsFinite") || code_string_equals(calleeStr, "Math_ApproxEq")) {
                return "code_bool";
            }
            if (code_string_equals(calleeStr, "Math_Random")) {
                return "double";
            }
            if (code_string_equals(calleeStr, "Math_RandomInt") || code_string_equals(calleeStr, "Math_AbsI") || code_string_equals(calleeStr, "Math_PowI") || code_string_equals(calleeStr, "Math_Gcd") || code_string_equals(calleeStr, "Math_Lcm") || code_string_equals(calleeStr, "Math_Clamp")) {
                return "i64";
            }
            if (expr->Left->Kind == Amalgame_Compiler_NodeKind_MEMBER) {
                code_string __attribute__((unused)) mname2 = expr->Left->Name;
                if (code_string_equals(mname2, "Count") || code_string_equals(mname2, "Size")) {
                    return "i64";
                }
                if (code_string_equals(mname2, "IsEmpty") || code_string_equals(mname2, "Has") || code_string_equals(mname2, "Contains") || code_string_equals(mname2, "Remove") || code_string_equals(mname2, "Add")) {
                    if (expr->Left->Left != NULL && expr->Left->Left->Kind == Amalgame_Compiler_NodeKind_IDENTIFIER) {
                        code_string __attribute__((unused)) objN = expr->Left->Left->Name;
                        code_string __attribute__((unused)) objT = Amalgame_Compiler_CGen_LocalTypeGet(self, objN);
                        if (code_string_equals(objT, "AmalgameList*") && code_string_equals(mname2, "IsEmpty")) {
                            return "code_bool";
                        }
                        if (code_string_equals(objT, "AmalgameList*") && code_string_equals(mname2, "Remove")) {
                            return "code_bool";
                        }
                        if (code_string_equals(objT, "AmalgameMap*") && code_string_equals(mname2, "Has") || code_string_equals(mname2, "Remove")) {
                            return "code_bool";
                        }
                        if (code_string_equals(objT, "AmalgameSet*") && code_string_equals(mname2, "Contains") || code_string_equals(mname2, "Remove") || code_string_equals(mname2, "Add")) {
                            return "code_bool";
                        }
                    }
                }
                if (code_string_equals(mname2, "Get")) {
                    if (expr->Left->Left != NULL) {
                        Amalgame_Compiler_AstNode* __attribute__((unused)) ll = expr->Left->Left;
                        if (ll->Kind == Amalgame_Compiler_NodeKind_THIS_EXPR) {
                        }
                        if (ll->Kind == Amalgame_Compiler_NodeKind_MEMBER) {
                            if (ll->Left != NULL) {
                                if (ll->Left->Kind == Amalgame_Compiler_NodeKind_THIS_EXPR) {
                                    code_string __attribute__((unused)) fn3 = ll->Name;
                                    code_string __attribute__((unused)) et = Amalgame_Compiler_CGen_ListElemGet(self, self->CurrentClass, fn3);
                                    if (String_Length(et) > 0) {
                                        return et;
                                    }
                                }
                            }
                        }
                        if (ll->Kind == Amalgame_Compiler_NodeKind_IDENTIFIER) {
                            code_string __attribute__((unused)) vn3 = ll->Name;
                            code_string __attribute__((unused)) vtype = Amalgame_Compiler_CGen_LocalTypeGet(self, vn3);
                            code_string __attribute__((unused)) bare3 = String_Replace(vtype, "*", "");
                        }
                    }
                    return "void*";
                }
                if (expr->Left->Left != NULL) {
                    Amalgame_Compiler_NodeKind __attribute__((unused)) llk2 = expr->Left->Left->Kind;
                    if (llk2 == Amalgame_Compiler_NodeKind_THIS_EXPR) {
                        code_string __attribute__((unused)) retT = Amalgame_Compiler_CGen_MethodRetGet(self, self->CurrentClass, mname2);
                        if (String_Length(retT) > 0) {
                            return retT;
                        }
                    }
                    if (llk2 == Amalgame_Compiler_NodeKind_IDENTIFIER) {
                        code_string __attribute__((unused)) cname2 = expr->Left->Left->Name;
                        code_string __attribute__((unused)) firstC = String_Substring(cname2, 0, 1);
                        code_bool __attribute__((unused)) isUpp = code_string_equals(firstC, String_ToUpper(firstC));
                        if (isUpp) {
                            code_string __attribute__((unused)) symCls = Amalgame_Compiler_CGen_SymName(self, cname2);
                            code_string __attribute__((unused)) retT2 = Amalgame_Compiler_CGen_MethodRetGet(self, symCls, mname2);
                            if (String_Length(retT2) > 0) {
                                return retT2;
                            }
                        }
                        if (!isUpp) {
                            code_string __attribute__((unused)) objType = Amalgame_Compiler_CGen_LocalTypeGet(self, cname2);
                            code_string __attribute__((unused)) bareCls = String_Replace(objType, "*", "");
                            if (String_Length(bareCls) > 0) {
                                code_string __attribute__((unused)) retT3 = Amalgame_Compiler_CGen_MethodRetGet(self, bareCls, mname2);
                                if (String_Length(retT3) > 0) {
                                    return retT3;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return "";
}

static code_string Amalgame_Compiler_CGen_EmitInterpolatedString(Amalgame_Compiler_CGen* self, code_string raw) {
    (void)self;
    (void)raw;
    i64 __attribute__((unused)) hasInterp = String_IndexOf(raw, "{");
    if (hasInterp < 0) {
        code_string __attribute__((unused)) escaped = Amalgame_Compiler_CGen_EscapeStringForC(self, raw);
        return code_string_concat(code_string_concat("\"", escaped), "\"");
    }
    i64 __attribute__((unused)) len = String_Length(raw);
    code_bool __attribute__((unused)) hasValid = 0;
    i64 __attribute__((unused)) ci = 0;
    while (ci < len) {
        code_string __attribute__((unused)) cch = String_Substring(raw, ci, 1);
        if (code_string_equals(cch, "{")) {
            code_string __attribute__((unused)) rest = String_Substring(raw, ci + 1, len - ci - 1);
            i64 __attribute__((unused)) close = String_IndexOf(rest, "}");
            if (close > 0) {
                code_string __attribute__((unused)) inner = String_Substring(raw, ci + 1, close);
                code_string __attribute__((unused)) fc = String_Substring(inner, 0, 1);
                code_bool __attribute__((unused)) isAlpha = String_Contains("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", fc);
                if (isAlpha || String_StartsWith(inner, "this.")) {
                    hasValid = 1;
                    ci = len;
                } else {
                    ci = ci + 1;
                }
            } else {
                ci = ci + 1;
            }
        } else {
            ci = ci + 1;
        }
    }
    if (!hasValid) {
        code_string __attribute__((unused)) escaped = Amalgame_Compiler_CGen_EscapeStringForC(self, raw);
        return code_string_concat(code_string_concat("\"", escaped), "\"");
    }
    code_string __attribute__((unused)) result = "\"\"";
    i64 __attribute__((unused)) i = 0;
    i64 __attribute__((unused)) segStart = 0;
    while (i < len) {
        code_string __attribute__((unused)) ch = String_Substring(raw, i, 1);
        if (code_string_equals(ch, "{")) {
            code_string __attribute__((unused)) rest2 = String_Substring(raw, i + 1, len - i - 1);
            i64 __attribute__((unused)) close2 = String_IndexOf(rest2, "}");
            if (close2 > 0) {
                code_string __attribute__((unused)) expr = String_Substring(raw, i + 1, close2);
                code_string __attribute__((unused)) fc2 = String_Substring(expr, 0, 1);
                code_bool __attribute__((unused)) isId = String_Contains("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", fc2);
                if (isId || String_StartsWith(expr, "this.")) {
                    if (i > segStart) {
                        code_string __attribute__((unused)) seg = String_Substring(raw, segStart, i - segStart);
                        code_string __attribute__((unused)) esc = Amalgame_Compiler_CGen_EscapeStringForC(self, seg);
                        result = code_string_concat(code_string_concat(code_string_concat(code_string_concat("code_string_concat(", result), ", \""), esc), "\")");
                    }
                    code_string __attribute__((unused)) cExpr = Amalgame_Compiler_CGen_InterpExprToC(self, expr);
                    result = code_string_concat(code_string_concat(code_string_concat(code_string_concat("code_string_concat(", result), ", "), cExpr), ")");
                    segStart = i + 1 + close2 + 1;
                    i = segStart;
                } else {
                    i = i + 1;
                }
            } else {
                i = i + 1;
            }
        } else {
            i = i + 1;
        }
    }
    if (segStart < len) {
        code_string __attribute__((unused)) seg = String_Substring(raw, segStart, len - segStart);
        code_string __attribute__((unused)) esc = Amalgame_Compiler_CGen_EscapeStringForC(self, seg);
        result = code_string_concat(code_string_concat(code_string_concat(code_string_concat("code_string_concat(", result), ", \""), esc), "\")");
    }
    return result;
}

static code_string Amalgame_Compiler_CGen_InterpExprToC(Amalgame_Compiler_CGen* self, code_string expr) {
    (void)self;
    (void)expr;
    if (String_StartsWith(expr, "this.")) {
        code_string __attribute__((unused)) field = String_Substring(expr, 5, String_Length(expr) - 5);
        code_string __attribute__((unused)) ft = Amalgame_Compiler_CGen_FieldTypeGet(self, self->CurrentClass, field);
        if (code_string_equals(ft, "i64")) {
            return code_string_concat(code_string_concat("String_FromInt(self->", field), ")");
        }
        if (code_string_equals(ft, "double")) {
            return code_string_concat(code_string_concat("String_FromFloat(self->", field), ")");
        }
        if (code_string_equals(ft, "code_bool")) {
            return code_string_concat(code_string_concat("((self->", field), ") ? \"true\" : \"false\")");
        }
        if (code_string_equals(ft, "code_string")) {
            return code_string_concat(code_string_concat(code_string_concat(code_string_concat("(self->", field), " ? self->"), field), " : \"\")");
        }
        if (String_Length(ft) > 0 && !String_Contains(ft, "*")) {
            return code_string_concat(code_string_concat("String_FromInt((i64)self->", field), ")");
        }
        return code_string_concat(code_string_concat(code_string_concat(code_string_concat("(self->", field), " ? self->"), field), " : \"\")");
    }
    i64 __attribute__((unused)) dotIdx = String_IndexOf(expr, ".");
    if (dotIdx > 0) {
        code_string __attribute__((unused)) objName = String_Substring(expr, 0, dotIdx);
        code_string __attribute__((unused)) field2 = String_Substring(expr, dotIdx + 1, String_Length(expr) - dotIdx - 1);
        code_string __attribute__((unused)) objType = Amalgame_Compiler_CGen_LocalTypeGet(self, objName);
        code_string __attribute__((unused)) bareT = String_Replace(objType, "*", "");
        if (String_Length(bareT) > 0) {
            code_string __attribute__((unused)) ft2 = Amalgame_Compiler_CGen_FieldTypeGet(self, bareT, field2);
            if (code_string_equals(ft2, "i64")) {
                return code_string_concat(code_string_concat(code_string_concat(code_string_concat("String_FromInt(", objName), "->"), field2), ")");
            }
            if (code_string_equals(ft2, "double")) {
                return code_string_concat(code_string_concat(code_string_concat(code_string_concat("String_FromFloat(", objName), "->"), field2), ")");
            }
            if (code_string_equals(ft2, "code_bool")) {
                return code_string_concat(code_string_concat(code_string_concat(code_string_concat("((", objName), "->"), field2), ") ? \"true\" : \"false\")");
            }
            if (code_string_equals(ft2, "code_string")) {
                return code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("(", objName), "->"), field2), " ? "), objName), "->"), field2), " : \"\")");
            }
            if (String_Length(ft2) > 0 && !String_Contains(ft2, "*")) {
                return code_string_concat(code_string_concat(code_string_concat(code_string_concat("String_FromInt((i64)", objName), "->"), field2), ")");
            }
            return code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("(", objName), "->"), field2), " ? "), objName), "->"), field2), " : \"\")");
        }
    }
    code_string __attribute__((unused)) varC = Amalgame_Compiler_CGen_LocalTypeGet(self, expr);
    if (code_string_equals(varC, "i64")) {
        return code_string_concat(code_string_concat("String_FromInt(", expr), ")");
    }
    if (code_string_equals(varC, "double")) {
        return code_string_concat(code_string_concat("String_FromFloat(", expr), ")");
    }
    if (code_string_equals(varC, "code_bool")) {
        return code_string_concat(code_string_concat("(", expr), " ? \"true\" : \"false\")");
    }
    if (String_Length(varC) > 0) {
        return code_string_concat(code_string_concat(code_string_concat(code_string_concat("(", expr), " ? "), expr), " : \"\")");
    }
    return code_string_concat(code_string_concat("String_FromInt(", expr), ")");
}

static code_string Amalgame_Compiler_CGen_EscapeStringForC(Amalgame_Compiler_CGen* self, code_string raw) {
    (void)self;
    (void)raw;
    code_string __attribute__((unused)) s = String_Replace(raw, "\\", "\\\\");
    s = String_Replace(s, "\"", "\\\"");
    s = String_Replace(s, "\n", "\\n");
    s = String_Replace(s, "\t", "\\t");
    s = String_Replace(s, "", "\\r");
    s = String_Replace(s, "1b", "\\x1b");
    return s;
}

static void Amalgame_Compiler_CGen_EmitHeader(Amalgame_Compiler_CGen* self) {
    (void)self;
    Amalgame_Compiler_Emitter_EmitLine(self->Out, "#include <stdio.h>");
    Amalgame_Compiler_Emitter_EmitLine(self->Out, "#include <stdlib.h>");
    Amalgame_Compiler_Emitter_EmitLine(self->Out, "#include <string.h>");
    Amalgame_Compiler_Emitter_EmitLine(self->Out, "#include \"_runtime.h\"");
    Amalgame_Compiler_Emitter_EmitLine(self->Out, "#include \"Amalgame_String.h\"");
    Amalgame_Compiler_Emitter_EmitLine(self->Out, "#include \"Amalgame_Collections.h\"");
    Amalgame_Compiler_Emitter_EmitLine(self->Out, "#include \"Amalgame_IO.h\"");
    Amalgame_Compiler_Emitter_EmitLine(self->Out, "#include \"Amalgame_Math.h\"");
    Amalgame_Compiler_Emitter_EmitLine(self->Out, "#include \"Amalgame_Net.h\"");
    Amalgame_Compiler_Emitter_EmitLine(self->Out, "#include \"Amalgame_Console.h\"");
    Amalgame_Compiler_Emitter_EmitBlank(self->Out);
}

void Amalgame_Compiler_CGen_EmitNetHeader(Amalgame_Compiler_CGen* self) {
    (void)self;
    Amalgame_Compiler_Emitter_EmitLine(self->Out, "#include \"Amalgame_Net.h\"");
}

static void Amalgame_Compiler_CGen_EmitForwardDecl(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* decl) {
    (void)self;
    (void)decl;
    Amalgame_Compiler_NodeKind __attribute__((unused)) k = decl->Kind;
    if (k == Amalgame_Compiler_NodeKind_ENUM_DECL) {
        code_string __attribute__((unused)) name = Amalgame_Compiler_CGen_SymName(self, decl->Name);
        code_bool __attribute__((unused)) isAlg = 0;
        i64 __attribute__((unused)) vc = AmalgameList_count(decl->Children);
        for (i64 vi = 0; vi < vc; vi++) {
            Amalgame_Compiler_AstNode* __attribute__((unused)) vm = (Amalgame_Compiler_AstNode*)AmalgameList_get(decl->Children, vi);
            if (String_Length(vm->Str) > 0) {
                isAlg = 1;
            }
        }
        if (isAlg) {
            AmalgameList_add(self->EnumNames, (void*)(intptr_t)(code_string_concat("__alg__", name)));
            Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(code_string_concat("typedef struct _", name), " "), name), "; /* alg-enum fwd */"));
        } else {
            AmalgameList_add(self->EnumNames, (void*)(intptr_t)(decl->Name));
            AmalgameList_add(self->EnumNames, (void*)(intptr_t)(name));
            Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(code_string_concat("typedef enum _", name), " "), name), ";"));
        }
    }
    if (k == Amalgame_Compiler_NodeKind_CLASS_DECL) {
        code_string __attribute__((unused)) name = Amalgame_Compiler_CGen_SymName(self, decl->Name);
        Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(code_string_concat("typedef struct _", name), " "), name), ";"));
    }
}

static void Amalgame_Compiler_CGen_EmitDecl(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* decl) {
    (void)self;
    (void)decl;
    Amalgame_Compiler_NodeKind __attribute__((unused)) k = decl->Kind;
    if (k == Amalgame_Compiler_NodeKind_ENUM_DECL) {
        Amalgame_Compiler_CGen_EmitEnum(self, decl);
    }
    if (k == Amalgame_Compiler_NodeKind_CLASS_DECL) {
        Amalgame_Compiler_CGen_EmitClass(self, decl);
    }
}

static void Amalgame_Compiler_CGen_EmitEnum(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* en) {
    (void)self;
    (void)en;
    code_string __attribute__((unused)) name = Amalgame_Compiler_CGen_SymName(self, en->Name);
    i64 __attribute__((unused)) count = AmalgameList_count(en->Children);
    code_bool __attribute__((unused)) isAlgebraic = 0;
    for (i64 i = 0; i < count; i++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) member = (Amalgame_Compiler_AstNode*)AmalgameList_get(en->Children, i);
        if (String_Length(member->Str) > 0) {
            isAlgebraic = 1;
        }
    }
    if (isAlgebraic) {
        Amalgame_Compiler_Emitter_EmitLine(self->Out, "typedef enum {");
        Amalgame_Compiler_Emitter_Indent_(self->Out);
        for (i64 i = 0; i < count; i++) {
            Amalgame_Compiler_AstNode* __attribute__((unused)) member = (Amalgame_Compiler_AstNode*)AmalgameList_get(en->Children, i);
            code_string __attribute__((unused)) mname = code_string_concat(code_string_concat(name, "_TAG_"), member->Name);
            if (i < count - 1) {
                Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(mname, ","));
            } else {
                Amalgame_Compiler_Emitter_EmitLine(self->Out, mname);
            }
        }
        Amalgame_Compiler_Emitter_Dedent(self->Out);
        Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat("} ", name), "_Tag;"));
        Amalgame_Compiler_Emitter_EmitBlank(self->Out);
        Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat("struct _", name), " {"));
        Amalgame_Compiler_Emitter_Indent_(self->Out);
        Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(name, "_Tag tag;"));
        Amalgame_Compiler_Emitter_EmitLine(self->Out, "union {");
        Amalgame_Compiler_Emitter_Indent_(self->Out);
        for (i64 i = 0; i < count; i++) {
            Amalgame_Compiler_AstNode* __attribute__((unused)) member = (Amalgame_Compiler_AstNode*)AmalgameList_get(en->Children, i);
            code_string __attribute__((unused)) payloads = member->Str;
            if (String_Length(payloads) > 0) {
                Amalgame_Compiler_Emitter_EmitLine(self->Out, "struct {");
                Amalgame_Compiler_Emitter_Indent_(self->Out);
                i64 __attribute__((unused)) pi = 0;
                code_string __attribute__((unused)) pcur = "";
                i64 __attribute__((unused)) pci = 0;
                i64 __attribute__((unused)) plen = String_Length(payloads);
                while (pci <= plen) {
                    code_string __attribute__((unused)) pch = "";
                    if (pci < plen) {
                        pch = String_Substring(payloads, pci, 1);
                    }
                    if (code_string_equals(pch, ",") || pci == plen) {
                        if (String_Length(pcur) > 0) {
                            Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(Amalgame_Compiler_CGen_TypeToC(self, pcur), " _"), String_FromInt(pi)), ";"));
                            pi = pi + 1;
                            pcur = "";
                        }
                    } else {
                        pcur = code_string_concat(pcur, pch);
                    }
                    pci = pci + 1;
                }
                Amalgame_Compiler_Emitter_Dedent(self->Out);
                Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat("} ", member->Name), ";"));
            } else {
                Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat("int ", member->Name), "_dummy;"));
            }
        }
        Amalgame_Compiler_Emitter_Dedent(self->Out);
        Amalgame_Compiler_Emitter_EmitLine(self->Out, "};");
        Amalgame_Compiler_Emitter_Dedent(self->Out);
        Amalgame_Compiler_Emitter_EmitLine(self->Out, "};");
        Amalgame_Compiler_Emitter_EmitBlank(self->Out);
        for (i64 i = 0; i < count; i++) {
            Amalgame_Compiler_AstNode* __attribute__((unused)) member = (Amalgame_Compiler_AstNode*)AmalgameList_get(en->Children, i);
            code_string __attribute__((unused)) payloads = member->Str;
            code_string __attribute__((unused)) params = "";
            code_string __attribute__((unused)) fields = "";
            if (String_Length(payloads) > 0) {
                i64 __attribute__((unused)) pi2 = 0;
                code_string __attribute__((unused)) pcur2 = "";
                i64 __attribute__((unused)) pci2 = 0;
                i64 __attribute__((unused)) plen2 = String_Length(payloads);
                while (pci2 <= plen2) {
                    code_string __attribute__((unused)) pch2 = "";
                    if (pci2 < plen2) {
                        pch2 = String_Substring(payloads, pci2, 1);
                    }
                    if (code_string_equals(pch2, ",") || pci2 == plen2) {
                        if (String_Length(pcur2) > 0) {
                            code_string __attribute__((unused)) ct = Amalgame_Compiler_CGen_TypeToC(self, pcur2);
                            code_string __attribute__((unused)) pname = code_string_concat("_", String_FromInt(pi2));
                            if (pi2 > 0) {
                                params = code_string_concat(params, ", ");
                                fields = code_string_concat(fields, ", ");
                            }
                            params = code_string_concat(code_string_concat(code_string_concat(params, ct), " "), pname);
                            fields = code_string_concat(fields, pname);
                            pi2 = pi2 + 1;
                            pcur2 = "";
                        }
                    } else {
                        pcur2 = code_string_concat(pcur2, pch2);
                    }
                    pci2 = pci2 + 1;
                }
            }
            code_string __attribute__((unused)) ctorSig = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("static inline ", name), " "), name), "_"), member->Name), "("), params), ")");
            Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(ctorSig, " {"));
            Amalgame_Compiler_Emitter_Indent_(self->Out);
            Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(name, " __v;"));
            Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(code_string_concat("__v.tag = ", name), "_TAG_"), member->Name), ";"));
            if (String_Length(fields) > 0) {
                i64 __attribute__((unused)) pi3 = 0;
                code_string __attribute__((unused)) pcur3 = "";
                i64 __attribute__((unused)) pci3 = 0;
                code_string __attribute__((unused)) payloads3 = member->Str;
                i64 __attribute__((unused)) plen3 = String_Length(payloads3);
                while (pci3 <= plen3) {
                    code_string __attribute__((unused)) pch3 = "";
                    if (pci3 < plen3) {
                        pch3 = String_Substring(payloads3, pci3, 1);
                    }
                    if (code_string_equals(pch3, ",") || pci3 == plen3) {
                        if (pi3 >= 0) {
                            code_string __attribute__((unused)) fidx = String_FromInt(pi3);
                            Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("__v.", member->Name), "._"), fidx), " = _"), fidx), ";"));
                            pi3 = pi3 + 1;
                        }
                        pcur3 = "";
                    } else {
                        pcur3 = code_string_concat(pcur3, pch3);
                    }
                    pci3 = pci3 + 1;
                }
            }
            Amalgame_Compiler_Emitter_EmitLine(self->Out, "return __v;");
            Amalgame_Compiler_Emitter_Dedent(self->Out);
            Amalgame_Compiler_Emitter_EmitLine(self->Out, "}");
            Amalgame_Compiler_Emitter_EmitBlank(self->Out);
        }
        AmalgameList_add(self->EnumNames, (void*)(intptr_t)(code_string_concat("__alg__", name)));
    } else {
        Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat("enum _", name), " {"));
        Amalgame_Compiler_Emitter_Indent_(self->Out);
        for (i64 i = 0; i < count; i++) {
            Amalgame_Compiler_AstNode* __attribute__((unused)) member = (Amalgame_Compiler_AstNode*)AmalgameList_get(en->Children, i);
            code_string __attribute__((unused)) mname = code_string_concat(code_string_concat(name, "_"), member->Name);
            if (i < count - 1) {
                Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(mname, ","));
            } else {
                Amalgame_Compiler_Emitter_EmitLine(self->Out, mname);
            }
        }
        Amalgame_Compiler_Emitter_Dedent(self->Out);
        Amalgame_Compiler_Emitter_EmitLine(self->Out, "};");
        Amalgame_Compiler_Emitter_EmitBlank(self->Out);
    }
}

static void Amalgame_Compiler_CGen_EmitClass(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* cls) {
    (void)self;
    (void)cls;
    code_string __attribute__((unused)) name = Amalgame_Compiler_CGen_SymName(self, cls->Name);
    i64 __attribute__((unused)) members = AmalgameList_count(cls->Children);
    self->CurrentClass = name;
    for (i64 i = 0; i < members; i++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) m = (Amalgame_Compiler_AstNode*)AmalgameList_get(cls->Children, i);
        Amalgame_Compiler_NodeKind __attribute__((unused)) mk = m->Kind;
        if (mk == Amalgame_Compiler_NodeKind_VAR_DECL) {
            code_string __attribute__((unused)) rawType = m->Str;
            code_string __attribute__((unused)) ftype = Amalgame_Compiler_CGen_TypeToC(self, rawType);
            Amalgame_Compiler_CGen_FieldTypeSet(self, name, m->Name, ftype);
            if (String_StartsWith(rawType, "List<")) {
                code_string __attribute__((unused)) inner = String_Substring(rawType, 5, String_Length(rawType) - 6);
                code_string __attribute__((unused)) elemC = Amalgame_Compiler_CGen_TypeToC(self, inner);
                Amalgame_Compiler_CGen_ListElemSet(self, name, m->Name, elemC);
            }
        }
    }
    Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat("struct _", name), " {"));
    Amalgame_Compiler_Emitter_Indent_(self->Out);
    code_string __attribute__((unused)) parentName = cls->Str;
    if (String_Length(parentName) > 0) {
        code_string __attribute__((unused)) parentC = Amalgame_Compiler_CGen_SymName(self, parentName);
        i64 __attribute__((unused)) pfc = AmalgameList_count(self->FieldNames);
        code_string __attribute__((unused)) pfPrefix = code_string_concat(parentC, ".");
        for (i64 pfi = 0; pfi < pfc; pfi++) {
            code_string __attribute__((unused)) pfkey = (code_string)AmalgameList_get(self->FieldNames, pfi);
            if (String_StartsWith(pfkey, pfPrefix)) {
                code_string __attribute__((unused)) pfname = String_Substring(pfkey, String_Length(pfPrefix), String_Length(pfkey) - String_Length(pfPrefix));
                code_string __attribute__((unused)) pftype = (code_string)AmalgameList_get(self->FieldCTypes, pfi);
                Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(pftype, " "), pfname), ";"));
                Amalgame_Compiler_CGen_FieldTypeSet(self, name, pfname, pftype);
            }
        }
    }
    for (i64 i = 0; i < members; i++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) m = (Amalgame_Compiler_AstNode*)AmalgameList_get(cls->Children, i);
        Amalgame_Compiler_NodeKind __attribute__((unused)) mk = m->Kind;
        if (mk == Amalgame_Compiler_NodeKind_VAR_DECL) {
            code_string __attribute__((unused)) ftype = Amalgame_Compiler_CGen_TypeToC(self, m->Str);
            Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(ftype, " "), m->Name), ";"));
        }
    }
    Amalgame_Compiler_Emitter_Dedent(self->Out);
    Amalgame_Compiler_Emitter_EmitLine(self->Out, "};");
    Amalgame_Compiler_Emitter_EmitBlank(self->Out);
    for (i64 i = 0; i < members; i++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) m = (Amalgame_Compiler_AstNode*)AmalgameList_get(cls->Children, i);
        Amalgame_Compiler_NodeKind __attribute__((unused)) mk = m->Kind;
        if (mk == Amalgame_Compiler_NodeKind_METHOD_DECL) {
            if (!code_string_equals(m->Name, cls->Name)) {
                code_string __attribute__((unused)) sig = Amalgame_Compiler_CGen_MethodSig(self, m, name);
                code_bool __attribute__((unused)) isPublic = m->Flag;
                code_string __attribute__((unused)) retC = Amalgame_Compiler_CGen_TypeToC(self, m->Str);
                Amalgame_Compiler_CGen_MethodRetSet(self, name, m->Name, retC);
                if (isPublic) {
                    Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(sig, ";"));
                } else {
                    Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat("static ", sig), ";"));
                }
            }
        }
    }
    Amalgame_Compiler_Emitter_EmitBlank(self->Out);
    code_bool __attribute__((unused)) ctorFound = 0;
    for (i64 i = 0; i < members; i++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) m = (Amalgame_Compiler_AstNode*)AmalgameList_get(cls->Children, i);
        Amalgame_Compiler_NodeKind __attribute__((unused)) mk = m->Kind;
        if (mk == Amalgame_Compiler_NodeKind_METHOD_DECL) {
            if (code_string_equals(m->Name, cls->Name)) {
                ctorFound = 1;
                code_string __attribute__((unused)) ctorSig = code_string_concat(code_string_concat(code_string_concat(name, "* "), name), "_new(");
                i64 __attribute__((unused)) pcount = AmalgameList_count(m->Params);
                code_bool __attribute__((unused)) first = 1;
                for (i64 pi = 0; pi < pcount; pi++) {
                    Amalgame_Compiler_AstNode* __attribute__((unused)) p = (Amalgame_Compiler_AstNode*)AmalgameList_get(m->Params, pi);
                    if (!first) {
                        ctorSig = code_string_concat(ctorSig, ", ");
                    }
                    ctorSig = code_string_concat(code_string_concat(code_string_concat(ctorSig, Amalgame_Compiler_CGen_TypeToC(self, p->Str)), " "), p->Name);
                    first = 0;
                }
                ctorSig = code_string_concat(ctorSig, ")");
                Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(ctorSig, " {"));
                Amalgame_Compiler_Emitter_Indent_(self->Out);
                Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(name, "* self = ("), name), "*) GC_MALLOC(sizeof("), name), "));"));
                Amalgame_Compiler_CGen_LocalTypeClear(self);
                Amalgame_Compiler_CGen_LocalTypeSet(self, "self", code_string_concat(name, "*"));
                for (i64 pi = 0; pi < pcount; pi++) {
                    Amalgame_Compiler_AstNode* __attribute__((unused)) p = (Amalgame_Compiler_AstNode*)AmalgameList_get(m->Params, pi);
                    Amalgame_Compiler_CGen_LocalTypeSet(self, p->Name, Amalgame_Compiler_CGen_TypeToC(self, p->Str));
                }
                if (m->Body != NULL) {
                    Amalgame_Compiler_CGen_EmitBlock(self, m->Body);
                }
                Amalgame_Compiler_CGen_LocalTypeClear(self);
                Amalgame_Compiler_Emitter_EmitLine(self->Out, "return self;");
                Amalgame_Compiler_Emitter_Dedent(self->Out);
                Amalgame_Compiler_Emitter_EmitLine(self->Out, "}");
                Amalgame_Compiler_Emitter_EmitBlank(self->Out);
            }
        }
    }
    if (!ctorFound) {
        Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(name, "* "), name), "_new() {"));
        Amalgame_Compiler_Emitter_Indent_(self->Out);
        Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(name, "* self = ("), name), "*) GC_MALLOC(sizeof("), name), "));"));
        Amalgame_Compiler_Emitter_EmitLine(self->Out, "return self;");
        Amalgame_Compiler_Emitter_Dedent(self->Out);
        Amalgame_Compiler_Emitter_EmitLine(self->Out, "}");
        Amalgame_Compiler_Emitter_EmitBlank(self->Out);
    }
    for (i64 j = 0; j < members; j++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) m2 = (Amalgame_Compiler_AstNode*)AmalgameList_get(cls->Children, j);
        Amalgame_Compiler_NodeKind __attribute__((unused)) mk2 = m2->Kind;
        if (mk2 == Amalgame_Compiler_NodeKind_METHOD_DECL) {
            if (!code_string_equals(m2->Name, cls->Name)) {
                Amalgame_Compiler_CGen_EmitMethod(self, m2, name);
            }
        }
    }
}

static code_string Amalgame_Compiler_CGen_TupleStructName(Amalgame_Compiler_CGen* self, code_string tupleType) {
    (void)self;
    (void)tupleType;
    code_string __attribute__((unused)) inner = String_Substring(tupleType, 1, String_Length(tupleType) - 2);
    code_string __attribute__((unused)) parts = String_Replace(inner, " ", "");
    code_string __attribute__((unused)) result = "";
    i64 __attribute__((unused)) count = 0;
    i64 __attribute__((unused)) i2 = 0;
    i64 __attribute__((unused)) len2 = String_Length(parts);
    code_string __attribute__((unused)) cur = "";
    while (i2 <= len2) {
        code_string __attribute__((unused)) ch = "";
        if (i2 < len2) {
            ch = String_Substring(parts, i2, 1);
        }
        if (code_string_equals(ch, ",") || i2 == len2) {
            if (String_Length(cur) > 0) {
                count = count + 1;
                result = code_string_concat(code_string_concat(result, "_"), Amalgame_Compiler_CGen_TypeToC(self, cur));
                cur = "";
            }
        } else {
            cur = code_string_concat(cur, ch);
        }
        i2 = i2 + 1;
    }
    return code_string_concat(code_string_concat("Tuple", String_FromInt(count)), result);
}

static void Amalgame_Compiler_CGen_EnsureTupleStruct(Amalgame_Compiler_CGen* self, code_string tupleType) {
    (void)self;
    (void)tupleType;
    code_string __attribute__((unused)) sname = Amalgame_Compiler_CGen_TupleStructName(self, tupleType);
    i64 __attribute__((unused)) n = AmalgameList_count(self->EnumNames);
    for (i64 i = 0; i < n; i++) {
        if (code_string_equals((code_string)AmalgameList_get(self->EnumNames, i), code_string_concat("__tuple__", sname))) {
            return;
        }
    }
    AmalgameList_add(self->EnumNames, (void*)(intptr_t)(code_string_concat("__tuple__", sname)));
    code_string __attribute__((unused)) inner = String_Substring(tupleType, 1, String_Length(tupleType) - 2);
    code_string __attribute__((unused)) parts = String_Replace(inner, " ", "");
    Amalgame_Compiler_Emitter_EmitLine(self->Out, "typedef struct {");
    i64 __attribute__((unused)) idx = 0;
    code_string __attribute__((unused)) cur = "";
    i64 __attribute__((unused)) i2 = 0;
    i64 __attribute__((unused)) len2 = String_Length(parts);
    while (i2 <= len2) {
        code_string __attribute__((unused)) ch = "";
        if (i2 < len2) {
            ch = String_Substring(parts, i2, 1);
        }
        if (code_string_equals(ch, ",") || i2 == len2) {
            if (String_Length(cur) > 0) {
                Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(code_string_concat("    ", Amalgame_Compiler_CGen_TypeToC(self, cur)), " _"), String_FromInt(idx)), ";"));
                idx = idx + 1;
                cur = "";
            }
        } else {
            cur = code_string_concat(cur, ch);
        }
        i2 = i2 + 1;
    }
    Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat("} ", sname), ";"));
}

static code_string Amalgame_Compiler_CGen_MethodSig(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* method, code_string className) {
    (void)self;
    (void)method;
    (void)className;
    code_string __attribute__((unused)) retTypeRaw = method->Str;
    code_string __attribute__((unused)) retType = "";
    if (String_StartsWith(retTypeRaw, "(")) {
        Amalgame_Compiler_CGen_EnsureTupleStruct(self, retTypeRaw);
        retType = Amalgame_Compiler_CGen_TupleStructName(self, retTypeRaw);
    } else {
        retType = Amalgame_Compiler_CGen_TypeToC(self, retTypeRaw);
    }
    code_bool __attribute__((unused)) isStatic = method->Flag2;
    code_string __attribute__((unused)) sig = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(retType, " "), className), "_"), method->Name), "(");
    code_bool __attribute__((unused)) first = 1;
    if (!isStatic) {
        sig = code_string_concat(code_string_concat(sig, className), "* self");
        first = 0;
    }
    i64 __attribute__((unused)) pcount = AmalgameList_count(method->Params);
    for (i64 i = 0; i < pcount; i++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) p = (Amalgame_Compiler_AstNode*)AmalgameList_get(method->Params, i);
        if (!first) {
            sig = code_string_concat(sig, ", ");
        }
        sig = code_string_concat(code_string_concat(code_string_concat(sig, Amalgame_Compiler_CGen_TypeToC(self, p->Str)), " "), p->Name);
        first = 0;
    }
    sig = code_string_concat(sig, ")");
    return sig;
}

static void Amalgame_Compiler_CGen_EmitMethod(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* method, code_string className) {
    (void)self;
    (void)method;
    (void)className;
    code_string __attribute__((unused)) sig = Amalgame_Compiler_CGen_MethodSig(self, method, className);
    code_bool __attribute__((unused)) isPublic = method->Flag;
    code_string __attribute__((unused)) prefix = "";
    if (!isPublic) {
        prefix = "static ";
    }
    if (String_StartsWith(method->Str, "(")) {
        self->CurrentRetType = Amalgame_Compiler_CGen_TupleStructName(self, method->Str);
        Amalgame_Compiler_CGen_MethodRetSet(self, className, method->Name, method->Str);
    } else {
        self->CurrentRetType = "";
    }
    Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(prefix, sig), " {"));
    Amalgame_Compiler_Emitter_Indent_(self->Out);
    Amalgame_Compiler_CGen_LocalTypeClear(self);
    code_bool __attribute__((unused)) isStatic = method->Flag2;
    if (!isStatic) {
        Amalgame_Compiler_CGen_LocalTypeSet(self, "self", code_string_concat(className, "*"));
        Amalgame_Compiler_Emitter_EmitLine(self->Out, "(void)self;");
    }
    i64 __attribute__((unused)) pcount = AmalgameList_count(method->Params);
    for (i64 i = 0; i < pcount; i++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) p = (Amalgame_Compiler_AstNode*)AmalgameList_get(method->Params, i);
        Amalgame_Compiler_CGen_LocalTypeSet(self, p->Name, Amalgame_Compiler_CGen_TypeToC(self, p->Str));
        Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat("(void)", p->Name), ";"));
    }
    if (method->Body != NULL) {
        Amalgame_Compiler_CGen_EmitBlock(self, method->Body);
    }
    Amalgame_Compiler_CGen_LocalTypeClear(self);
    Amalgame_Compiler_Emitter_Dedent(self->Out);
    Amalgame_Compiler_Emitter_EmitLine(self->Out, "}");
    Amalgame_Compiler_Emitter_EmitBlank(self->Out);
}

static code_string Amalgame_Compiler_CGen_EmitIfBranch(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* block) {
    (void)self;
    (void)block;
    if (block == NULL) {
        return "0";
    }
    if (block->Kind == Amalgame_Compiler_NodeKind_BLOCK) {
        AmalgameList* __attribute__((unused)) kids = block->Children;
        if (AmalgameList_count(kids) > 0) {
            return Amalgame_Compiler_CGen_EmitExprStr(self, (void*)AmalgameList_get(kids, 0));
        }
        return "0";
    }
    return Amalgame_Compiler_CGen_EmitExprStr(self, block);
}

static void Amalgame_Compiler_CGen_EmitMatch(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* stmt) {
    (void)self;
    (void)stmt;
    Amalgame_Compiler_AstNode* __attribute__((unused)) subject = stmt->Left;
    code_string __attribute__((unused)) subjectStr = Amalgame_Compiler_CGen_EmitExprStr(self, subject);
    code_string __attribute__((unused)) subjectType = "";
    if (subject != NULL && subject->Kind == Amalgame_Compiler_NodeKind_IDENTIFIER) {
        subjectType = Amalgame_Compiler_CGen_LocalTypeGet(self, subject->Name);
    }
    code_bool __attribute__((unused)) isAlg = 0;
    if (String_Length(subjectType) > 0) {
        code_string __attribute__((unused)) algKey2 = code_string_concat("__alg__", subjectType);
        i64 __attribute__((unused)) enCount3 = AmalgameList_count(self->EnumNames);
        for (i64 eni3 = 0; eni3 < enCount3; eni3++) {
            if (code_string_equals((code_string)AmalgameList_get(self->EnumNames, eni3), algKey2)) {
                isAlg = 1;
            }
        }
    }
    i64 __attribute__((unused)) armCount = AmalgameList_count(stmt->Children);
    if (isAlg) {
        Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat("switch (", subjectStr), ".tag) {"));
    } else {
        Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat("{ /* match ", subjectStr), " */"));
    }
    Amalgame_Compiler_Emitter_Indent_(self->Out);
    code_bool __attribute__((unused)) firstArm = 1;
    code_bool __attribute__((unused)) needsClose = 0;
    for (i64 i = 0; i < armCount; i++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) arm = (Amalgame_Compiler_AstNode*)AmalgameList_get(stmt->Children, i);
        Amalgame_Compiler_AstNode* __attribute__((unused)) pat = arm->Left;
        Amalgame_Compiler_AstNode* __attribute__((unused)) body = arm->Right;
        if (pat == NULL) {
            continue;
        }
        Amalgame_Compiler_NodeKind __attribute__((unused)) pk = pat->Kind;
        if (pk == Amalgame_Compiler_NodeKind_IDENTIFIER && code_string_equals(pat->Name, "_")) {
            if (isAlg) {
                Amalgame_Compiler_Emitter_EmitLine(self->Out, "default: {");
                Amalgame_Compiler_Emitter_Indent_(self->Out);
                Amalgame_Compiler_CGen_EmitMatchBody(self, body);
                Amalgame_Compiler_Emitter_EmitLine(self->Out, "break;");
                Amalgame_Compiler_Emitter_Dedent(self->Out);
                Amalgame_Compiler_Emitter_EmitLine(self->Out, "}");
                needsClose = 0;
            } else {
                void* __attribute__((unused)) elsePrefix = (firstArm ? "" : "} else ");
                Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat((code_string)(elsePrefix), "{"));
                Amalgame_Compiler_Emitter_Indent_(self->Out);
                Amalgame_Compiler_CGen_EmitMatchBody(self, body);
                Amalgame_Compiler_Emitter_Dedent(self->Out);
                Amalgame_Compiler_Emitter_EmitLine(self->Out, "}");
                needsClose = 0;
            }
            firstArm = 0;
        } else {
            if (isAlg && pk == Amalgame_Compiler_NodeKind_CALL) {
                code_string __attribute__((unused)) variantName = pat->Name;
                code_string __attribute__((unused)) tagConst = code_string_concat(code_string_concat(subjectType, "_TAG_"), variantName);
                Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat("case ", tagConst), ": {"));
                Amalgame_Compiler_Emitter_Indent_(self->Out);
                i64 __attribute__((unused)) capCount = AmalgameList_count(pat->Args);
                for (i64 ci = 0; ci < capCount; ci++) {
                    Amalgame_Compiler_AstNode* __attribute__((unused)) capNode = (Amalgame_Compiler_AstNode*)AmalgameList_get(pat->Args, ci);
                    code_string __attribute__((unused)) capName = capNode->Name;
                    code_string __attribute__((unused)) fidx = String_FromInt(ci);
                    Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("__auto_type ", capName), " = "), subjectStr), "."), variantName), "._"), fidx), ";"));
                    Amalgame_Compiler_CGen_LocalTypeSet(self, capName, "i64");
                }
                Amalgame_Compiler_CGen_EmitMatchBody(self, body);
                Amalgame_Compiler_Emitter_EmitLine(self->Out, "break;");
                Amalgame_Compiler_Emitter_Dedent(self->Out);
                Amalgame_Compiler_Emitter_EmitLine(self->Out, "}");
                needsClose = 0;
            } else {
                void* __attribute__((unused)) elsePrefix = (firstArm ? "" : "} else ");
                if (pk == Amalgame_Compiler_NodeKind_BINARY && code_string_equals(pat->Str, "..")) {
                    code_string __attribute__((unused)) lo = Amalgame_Compiler_CGen_EmitExprStr(self, pat->Left);
                    code_string __attribute__((unused)) hi = Amalgame_Compiler_CGen_EmitExprStr(self, pat->Right);
                    Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat((code_string)(elsePrefix), "if ("), subjectStr), " >= "), lo), " && "), subjectStr), " <= "), hi), ") {"));
                } else if (pk == Amalgame_Compiler_NodeKind_LITERAL_STRING) {
                    code_string __attribute__((unused)) patStr = Amalgame_Compiler_CGen_EmitExprStr(self, pat);
                    Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat((code_string)(elsePrefix), "if (strcmp("), subjectStr), ", "), patStr), ") == 0) {"));
                } else {
                    code_string __attribute__((unused)) patStr = Amalgame_Compiler_CGen_EmitExprStr(self, pat);
                    Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat((code_string)(elsePrefix), "if ("), subjectStr), " == "), patStr), ") {"));
                }
                Amalgame_Compiler_Emitter_Indent_(self->Out);
                Amalgame_Compiler_CGen_EmitMatchBody(self, body);
                Amalgame_Compiler_Emitter_Dedent(self->Out);
                needsClose = 1;
            }
            firstArm = 0;
        }
    }
    if (needsClose) {
        Amalgame_Compiler_Emitter_EmitLine(self->Out, "}");
    }
    Amalgame_Compiler_Emitter_Dedent(self->Out);
    Amalgame_Compiler_Emitter_EmitLine(self->Out, "}");
}

static void Amalgame_Compiler_CGen_EmitMatchBody(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* body) {
    (void)self;
    (void)body;
    if (body == NULL) {
        return;
    }
    Amalgame_Compiler_NodeKind __attribute__((unused)) bk = body->Kind;
    if (bk == Amalgame_Compiler_NodeKind_BLOCK) {
        i64 __attribute__((unused)) stmts = AmalgameList_count(body->Children);
        for (i64 si = 0; si < stmts; si++) {
            Amalgame_Compiler_CGen_EmitStmt(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(body->Children, si));
        }
    } else {
        code_string __attribute__((unused)) exprStr = Amalgame_Compiler_CGen_EmitExprStr(self, body);
        if (String_Length(exprStr) > 0 && !code_string_equals(exprStr, "_unknown_")) {
            if (bk == Amalgame_Compiler_NodeKind_CALL || bk == Amalgame_Compiler_NodeKind_IDENTIFIER) {
                Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(exprStr, ";"));
            } else {
                Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat("return ", exprStr), ";"));
            }
        }
    }
}

static void Amalgame_Compiler_CGen_EmitIf(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* stmt) {
    (void)self;
    (void)stmt;
    Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat("if (", Amalgame_Compiler_CGen_EmitExprStr(self, stmt->Cond)), ") {"));
    Amalgame_Compiler_Emitter_Indent_(self->Out);
    if (stmt->Body != NULL) {
        Amalgame_Compiler_CGen_EmitBlock(self, stmt->Body);
    }
    Amalgame_Compiler_Emitter_Dedent(self->Out);
    if (stmt->Else != NULL) {
        Amalgame_Compiler_NodeKind __attribute__((unused)) elseKind = stmt->Else->Kind;
        if (elseKind == Amalgame_Compiler_NodeKind_IF_STMT) {
            code_string __attribute__((unused)) innerCond = Amalgame_Compiler_CGen_EmitExprStr(self, stmt->Else->Cond);
            Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat("} else if (", innerCond), ") {"));
            Amalgame_Compiler_Emitter_Indent_(self->Out);
            if (stmt->Else->Body != NULL) {
                Amalgame_Compiler_CGen_EmitBlock(self, stmt->Else->Body);
            }
            Amalgame_Compiler_Emitter_Dedent(self->Out);
            Amalgame_Compiler_CGen_EmitIfTail(self, stmt->Else);
        } else {
            Amalgame_Compiler_Emitter_EmitLine(self->Out, "} else {");
            Amalgame_Compiler_Emitter_Indent_(self->Out);
            Amalgame_Compiler_CGen_EmitBlock(self, stmt->Else);
            Amalgame_Compiler_Emitter_Dedent(self->Out);
            Amalgame_Compiler_Emitter_EmitLine(self->Out, "}");
        }
    } else {
        Amalgame_Compiler_Emitter_EmitLine(self->Out, "}");
    }
}

static void Amalgame_Compiler_CGen_EmitIfTail(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* stmt) {
    (void)self;
    (void)stmt;
    if (stmt->Else == NULL) {
        Amalgame_Compiler_Emitter_EmitLine(self->Out, "}");
        return;
    }
    Amalgame_Compiler_NodeKind __attribute__((unused)) elseKind = stmt->Else->Kind;
    if (elseKind == Amalgame_Compiler_NodeKind_IF_STMT) {
        code_string __attribute__((unused)) innerCond = Amalgame_Compiler_CGen_EmitExprStr(self, stmt->Else->Cond);
        Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat("} else if (", innerCond), ") {"));
        Amalgame_Compiler_Emitter_Indent_(self->Out);
        if (stmt->Else->Body != NULL) {
            Amalgame_Compiler_CGen_EmitBlock(self, stmt->Else->Body);
        }
        Amalgame_Compiler_Emitter_Dedent(self->Out);
        Amalgame_Compiler_CGen_EmitIfTail(self, stmt->Else);
    } else {
        Amalgame_Compiler_Emitter_EmitLine(self->Out, "} else {");
        Amalgame_Compiler_Emitter_Indent_(self->Out);
        Amalgame_Compiler_CGen_EmitBlock(self, stmt->Else);
        Amalgame_Compiler_Emitter_Dedent(self->Out);
        Amalgame_Compiler_Emitter_EmitLine(self->Out, "}");
    }
}

static void Amalgame_Compiler_CGen_EmitBlock(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* block) {
    (void)self;
    (void)block;
    i64 __attribute__((unused)) count = AmalgameList_count(block->Children);
    for (i64 i = 0; i < count; i++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) stmt = (Amalgame_Compiler_AstNode*)AmalgameList_get(block->Children, i);
        Amalgame_Compiler_CGen_EmitStmt(self, stmt);
    }
}

static void Amalgame_Compiler_CGen_EmitStmt(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* stmt) {
    (void)self;
    (void)stmt;
    Amalgame_Compiler_NodeKind __attribute__((unused)) k = stmt->Kind;
    if (k == Amalgame_Compiler_NodeKind_VAR_DECL) {
        if (code_string_equals(stmt->Str, "__tuple_destructure__")) {
            code_string __attribute__((unused)) rhs = Amalgame_Compiler_CGen_EmitExprStr(self, stmt->Left);
            code_string __attribute__((unused)) tmpName = code_string_concat("__tup_", String_FromInt(stmt->Line));
            Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(code_string_concat("__auto_type ", tmpName), " = "), rhs), ";"));
            code_string __attribute__((unused)) tupleRetType = "";
            if (stmt->Left != NULL && stmt->Left->Kind == Amalgame_Compiler_NodeKind_CALL) {
                code_string __attribute__((unused)) calleeStr2 = Amalgame_Compiler_CGen_EmitCalleeStr(self, stmt->Left->Left);
                i64 __attribute__((unused)) lastUs = String_LastIndexOf(calleeStr2, "_");
                if (lastUs > 0) {
                    code_string __attribute__((unused)) cls2 = String_Substring(calleeStr2, 0, lastUs);
                    code_string __attribute__((unused)) mth2 = String_Substring(calleeStr2, lastUs + 1, String_Length(calleeStr2) - lastUs - 1);
                    tupleRetType = Amalgame_Compiler_CGen_MethodRetGet(self, cls2, mth2);
                }
            }
            i64 __attribute__((unused)) nc = AmalgameList_count(stmt->Children);
            for (i64 i = 0; i < nc; i++) {
                Amalgame_Compiler_AstNode* __attribute__((unused)) vnode = (Amalgame_Compiler_AstNode*)AmalgameList_get(stmt->Children, i);
                code_string __attribute__((unused)) vname = vnode->Name;
                code_string __attribute__((unused)) idxStr = String_FromInt(i);
                Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("__auto_type __attribute__((unused)) ", vname), " = "), tmpName), "._"), idxStr), ";"));
                code_string __attribute__((unused)) elemType = "void*";
                if (String_Length(tupleRetType) > 2 && String_StartsWith(tupleRetType, "(")) {
                    code_string __attribute__((unused)) inner2 = String_Substring(tupleRetType, 1, String_Length(tupleRetType) - 2);
                    i64 __attribute__((unused)) ei = 0;
                    code_string __attribute__((unused)) cur2 = "";
                    i64 __attribute__((unused)) ci2 = 0;
                    i64 __attribute__((unused)) ilen = String_Length(inner2);
                    while (ci2 <= ilen) {
                        code_string __attribute__((unused)) ch2 = "";
                        if (ci2 < ilen) {
                            ch2 = String_Substring(inner2, ci2, 1);
                        }
                        if (code_string_equals(ch2, ",") || ci2 == ilen) {
                            if (ei == i && String_Length(cur2) > 0) {
                                elemType = Amalgame_Compiler_CGen_TypeToC(self, cur2);
                            }
                            ei = ei + 1;
                            cur2 = "";
                        } else {
                            cur2 = code_string_concat(cur2, ch2);
                        }
                        ci2 = ci2 + 1;
                    }
                }
                Amalgame_Compiler_CGen_LocalTypeSet(self, vname, elemType);
            }
            return;
        }
        if (stmt->Left != NULL && stmt->Left->Kind == Amalgame_Compiler_NodeKind_METHOD_DECL && code_string_equals(stmt->Left->Name, "__lambda__")) {
            Amalgame_Compiler_AstNode* __attribute__((unused)) lam = stmt->Left;
            code_string __attribute__((unused)) paramName = lam->Str;
            code_string __attribute__((unused)) bodyStr = Amalgame_Compiler_CGen_EmitExprStr(self, lam->Left);
            code_string __attribute__((unused)) fnName = code_string_concat(code_string_concat(self->CurrentClass, "_lam_"), stmt->Name);
            Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("/* lambda ", stmt->Name), " = "), paramName), " => "), bodyStr), " */"));
            Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("#define ", stmt->Name), "("), paramName), ") ("), bodyStr), ")"));
            Amalgame_Compiler_CGen_LocalTypeSet(self, stmt->Name, "__macro__");
            return;
        }
        code_string __attribute__((unused)) t = Amalgame_Compiler_CGen_TypeToC(self, stmt->Str);
        if (String_Length(t) == 0 || code_string_equals(t, "void")) {
            t = Amalgame_Compiler_CGen_InferTypeFromExpr(self, stmt->Left);
        }
        if (String_Length(t) == 0) {
            t = "void*";
        }
        Amalgame_Compiler_CGen_LocalTypeSet(self, stmt->Name, t);
        code_string __attribute__((unused)) rhs = Amalgame_Compiler_CGen_EmitExprStr(self, stmt->Left);
        code_string __attribute__((unused)) decl = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(t, " __attribute__((unused)) "), stmt->Name), " = "), rhs), ";");
        if (code_string_equals(t, "void*")) {
            if (String_StartsWith(rhs, "(") && !String_StartsWith(rhs, "(void*)") && !String_Contains(rhs, "?")) {
                i64 __attribute__((unused)) closeP = String_IndexOf(rhs, ")");
                if (closeP > 1) {
                    code_string __attribute__((unused)) castT = String_Substring(rhs, 1, closeP - 1);
                    if (String_Length(castT) > 0) {
                        t = castT;
                        Amalgame_Compiler_CGen_LocalTypeSet(self, stmt->Name, t);
                        decl = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(t, " __attribute__((unused)) "), stmt->Name), " = "), rhs), ";");
                    }
                }
            }
        }
        Amalgame_Compiler_Emitter_EmitLine(self->Out, decl);
    }
    if (k == Amalgame_Compiler_NodeKind_RETURN_STMT) {
        if (stmt->Left == NULL) {
            Amalgame_Compiler_Emitter_EmitLine(self->Out, "return;");
        } else {
            code_string __attribute__((unused)) retExpr = Amalgame_Compiler_CGen_EmitExprStr(self, stmt->Left);
            if (code_string_equals(retExpr, "_unknown_")) {
                Amalgame_Compiler_Emitter_EmitLine(self->Out, "return;");
            } else {
                if (String_StartsWith(retExpr, "{") && String_Length(self->CurrentRetType) > 0) {
                    Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(code_string_concat("return (", self->CurrentRetType), ")"), retExpr), ";"));
                } else {
                    Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat("return ", retExpr), ";"));
                }
            }
        }
    }
    if (k == Amalgame_Compiler_NodeKind_IF_STMT) {
        if (code_string_equals(stmt->Name, "__match__")) {
            Amalgame_Compiler_CGen_EmitMatch(self, stmt);
            return;
        }
        Amalgame_Compiler_CGen_EmitIf(self, stmt);
    }
    if (k == Amalgame_Compiler_NodeKind_WHILE_STMT) {
        Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat("while (", Amalgame_Compiler_CGen_EmitExprStr(self, stmt->Cond)), ") {"));
        Amalgame_Compiler_Emitter_Indent_(self->Out);
        if (stmt->Body != NULL) {
            Amalgame_Compiler_CGen_EmitBlock(self, stmt->Body);
        }
        Amalgame_Compiler_Emitter_Dedent(self->Out);
        Amalgame_Compiler_Emitter_EmitLine(self->Out, "}");
    }
    if (k == Amalgame_Compiler_NodeKind_FOR_IN_STMT) {
        code_string __attribute__((unused)) vn = stmt->Name;
        if (stmt->Left != NULL) {
            Amalgame_Compiler_NodeKind __attribute__((unused)) iterKind = stmt->Left->Kind;
            if (iterKind == Amalgame_Compiler_NodeKind_BINARY) {
                code_string __attribute__((unused)) iterOp = stmt->Left->Str;
                if (code_string_equals(iterOp, "..")) {
                    code_string __attribute__((unused)) startExpr = Amalgame_Compiler_CGen_EmitExprStr(self, stmt->Left->Left);
                    code_string __attribute__((unused)) endExpr = Amalgame_Compiler_CGen_EmitExprStr(self, stmt->Left->Right);
                    Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("for (i64 ", vn), " = "), startExpr), "; "), vn), " < "), endExpr), "; "), vn), "++) {"));
                    Amalgame_Compiler_CGen_LocalTypeSet(self, vn, "i64");
                    Amalgame_Compiler_Emitter_Indent_(self->Out);
                    if (stmt->Body != NULL) {
                        Amalgame_Compiler_CGen_EmitBlock(self, stmt->Body);
                    }
                    Amalgame_Compiler_Emitter_Dedent(self->Out);
                    Amalgame_Compiler_Emitter_EmitLine(self->Out, "}");
                    Amalgame_Compiler_CGen_LocalTypeSet(self, vn, "");
                    return;
                }
            }
        }
        code_string __attribute__((unused)) iter = Amalgame_Compiler_CGen_EmitExprStr(self, stmt->Left);
        Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(code_string_concat("/* for ", vn), " in "), iter), " — collection iteration not yet supported */"));
    }
    if (k == Amalgame_Compiler_NodeKind_BREAK_STMT) {
        Amalgame_Compiler_Emitter_EmitLine(self->Out, "break;");
    }
    if (k == Amalgame_Compiler_NodeKind_CONTINUE_STMT) {
        Amalgame_Compiler_Emitter_EmitLine(self->Out, "continue;");
    }
    if (k == Amalgame_Compiler_NodeKind_BINARY) {
        Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(Amalgame_Compiler_CGen_EmitExprStr(self, stmt), ";"));
    }
    if (k == Amalgame_Compiler_NodeKind_CALL) {
        Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(Amalgame_Compiler_CGen_EmitExprStr(self, stmt), ";"));
    }
    if (k == Amalgame_Compiler_NodeKind_MEMBER) {
        Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(Amalgame_Compiler_CGen_EmitExprStr(self, stmt), ";"));
    }
}

static code_string Amalgame_Compiler_CGen_EmitExprStr(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* expr) {
    (void)self;
    (void)expr;
    if (expr == NULL) {
        return "NULL";
    }
    Amalgame_Compiler_NodeKind __attribute__((unused)) k = expr->Kind;
    if (k == Amalgame_Compiler_NodeKind_METHOD_DECL && code_string_equals(expr->Name, "__lambda__")) {
        code_string __attribute__((unused)) paramName = expr->Str;
        code_string __attribute__((unused)) bodyStr = Amalgame_Compiler_CGen_EmitExprStr(self, expr->Left);
        return code_string_concat(code_string_concat(code_string_concat(code_string_concat("__lambda_", paramName), "_"), bodyStr), "__");
    }
    if (k == Amalgame_Compiler_NodeKind_CALL && code_string_equals(expr->Name, "__tuple_literal__")) {
        i64 __attribute__((unused)) argc = AmalgameList_count(expr->Args);
        code_string __attribute__((unused)) result = "{";
        for (i64 i = 0; i < argc; i++) {
            if (i > 0) {
                result = code_string_concat(result, ", ");
            }
            result = code_string_concat(result, Amalgame_Compiler_CGen_EmitExprStr(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(expr->Args, i)));
        }
        result = code_string_concat(result, "}");
        return result;
    }
    if (k == Amalgame_Compiler_NodeKind_IF_STMT) {
        code_string __attribute__((unused)) condStr = Amalgame_Compiler_CGen_EmitExprStr(self, expr->Cond);
        code_string __attribute__((unused)) thenStr = Amalgame_Compiler_CGen_EmitIfBranch(self, expr->Body);
        code_string __attribute__((unused)) elseStr = Amalgame_Compiler_CGen_EmitIfBranch(self, expr->Else);
        return code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("(", condStr), " ? "), thenStr), " : "), elseStr), ")");
    }
    if (k == Amalgame_Compiler_NodeKind_LITERAL_INT) {
        return expr->Str;
    }
    if (k == Amalgame_Compiler_NodeKind_LITERAL_FLOAT) {
        return expr->Str;
    }
    if (k == Amalgame_Compiler_NodeKind_LITERAL_STRING) {
        code_string __attribute__((unused)) raw = expr->Str;
        return Amalgame_Compiler_CGen_EmitInterpolatedString(self, raw);
    }
    if (k == Amalgame_Compiler_NodeKind_LITERAL_BOOL) {
        if (expr->Flag) {
            return "1";
        }
        return "0";
    }
    if (k == Amalgame_Compiler_NodeKind_LITERAL_NULL) {
        return "NULL";
    }
    if (k == Amalgame_Compiler_NodeKind_THIS_EXPR) {
        return "self";
    }
    if (k == Amalgame_Compiler_NodeKind_IDENTIFIER) {
        return expr->Name;
    }
    if (k == Amalgame_Compiler_NodeKind_MEMBER) {
        if (expr->Left != NULL) {
            Amalgame_Compiler_NodeKind __attribute__((unused)) lk = expr->Left->Kind;
            if (lk == Amalgame_Compiler_NodeKind_THIS_EXPR) {
                return code_string_concat("self->", expr->Name);
            }
            if (lk == Amalgame_Compiler_NodeKind_IDENTIFIER) {
                code_string __attribute__((unused)) tname = expr->Left->Name;
                code_string __attribute__((unused)) firstChar = String_Substring(tname, 0, 1);
                code_bool __attribute__((unused)) isUpper = code_string_equals(firstChar, String_ToUpper(firstChar));
                if (isUpper) {
                    return code_string_concat(code_string_concat(Amalgame_Compiler_CGen_SymName(self, tname), "_"), expr->Name);
                }
                return code_string_concat(code_string_concat(tname, "->"), expr->Name);
            }
        }
        code_string __attribute__((unused)) target = Amalgame_Compiler_CGen_EmitExprStr(self, expr->Left);
        return code_string_concat(code_string_concat(target, "->"), expr->Name);
    }
    if (k == Amalgame_Compiler_NodeKind_BINARY) {
        code_string __attribute__((unused)) left = Amalgame_Compiler_CGen_EmitExprStr(self, expr->Left);
        code_string __attribute__((unused)) op = expr->Str;
        code_string __attribute__((unused)) right = Amalgame_Compiler_CGen_EmitExprStr(self, expr->Right);
        if (code_string_equals(op, "==") || code_string_equals(op, "!=")) {
            code_string __attribute__((unused)) ltype = Amalgame_Compiler_CGen_InferTypeFromExpr(self, expr->Left);
            code_bool __attribute__((unused)) useStrEq = 0;
            if (code_string_equals(ltype, "code_string")) {
                useStrEq = 1;
            }
            if (String_Length(ltype) == 0) {
                if (expr->Left != NULL) {
                    Amalgame_Compiler_NodeKind __attribute__((unused)) lk2 = expr->Left->Kind;
                    if (lk2 == Amalgame_Compiler_NodeKind_LITERAL_STRING) {
                        useStrEq = 1;
                    }
                    if (lk2 == Amalgame_Compiler_NodeKind_CALL) {
                        if (expr->Left->Left != NULL) {
                            code_string __attribute__((unused)) calleeStr = Amalgame_Compiler_CGen_EmitCalleeStr(self, expr->Left->Left);
                            if (String_StartsWith(calleeStr, "String_") || String_EndsWith(calleeStr, "_CharAt")) {
                                useStrEq = 1;
                            }
                        }
                    }
                    if (lk2 == Amalgame_Compiler_NodeKind_IDENTIFIER) {
                        code_string __attribute__((unused)) lt2 = Amalgame_Compiler_CGen_LocalTypeGet(self, expr->Left->Name);
                        if (code_string_equals(lt2, "code_string")) {
                            useStrEq = 1;
                        }
                    }
                    if (lk2 == Amalgame_Compiler_NodeKind_MEMBER) {
                        code_string __attribute__((unused)) ft = Amalgame_Compiler_CGen_InferTypeFromExpr(self, expr->Left);
                        if (code_string_equals(ft, "code_string")) {
                            useStrEq = 1;
                        }
                    }
                }
            }
            if (!useStrEq) {
                if (expr->Right != NULL) {
                    if (expr->Right->Kind == Amalgame_Compiler_NodeKind_LITERAL_STRING) {
                        useStrEq = 1;
                    }
                }
            }
            if (useStrEq) {
                if (code_string_equals(op, "==")) {
                    return code_string_concat(code_string_concat(code_string_concat(code_string_concat("code_string_equals(", left), ", "), right), ")");
                }
                return code_string_concat(code_string_concat(code_string_concat(code_string_concat("!code_string_equals(", left), ", "), right), ")");
            }
            return code_string_concat(code_string_concat(code_string_concat(code_string_concat(left, " "), op), " "), right);
        }
        if (code_string_equals(op, "+")) {
            code_string __attribute__((unused)) ltype2 = Amalgame_Compiler_CGen_InferTypeFromExpr(self, expr->Left);
            code_bool __attribute__((unused)) useConcat = 0;
            if (code_string_equals(ltype2, "code_string")) {
                useConcat = 1;
            }
            if (!useConcat) {
                if (expr->Left != NULL) {
                    Amalgame_Compiler_NodeKind __attribute__((unused)) lk3 = expr->Left->Kind;
                    if (lk3 == Amalgame_Compiler_NodeKind_LITERAL_STRING) {
                        useConcat = 1;
                    }
                    if (lk3 == Amalgame_Compiler_NodeKind_IDENTIFIER) {
                        code_string __attribute__((unused)) lt3 = Amalgame_Compiler_CGen_LocalTypeGet(self, expr->Left->Name);
                        if (code_string_equals(lt3, "code_string")) {
                            useConcat = 1;
                        }
                    }
                    if (lk3 == Amalgame_Compiler_NodeKind_CALL) {
                        code_string __attribute__((unused)) ltype3 = Amalgame_Compiler_CGen_InferTypeFromExpr(self, expr->Left);
                        if (code_string_equals(ltype3, "code_string")) {
                            useConcat = 1;
                        }
                        if (expr->Left->Left != NULL) {
                            code_string __attribute__((unused)) calleeL = Amalgame_Compiler_CGen_EmitCalleeStr(self, expr->Left->Left);
                            if (String_StartsWith(calleeL, "String_") || code_string_equals(calleeL, "code_string_concat")) {
                                useConcat = 1;
                            }
                        }
                    }
                }
            }
            if (!useConcat) {
                if (expr->Right != NULL) {
                    Amalgame_Compiler_NodeKind __attribute__((unused)) rk3 = expr->Right->Kind;
                    if (rk3 == Amalgame_Compiler_NodeKind_LITERAL_STRING) {
                        useConcat = 1;
                    }
                    if (rk3 == Amalgame_Compiler_NodeKind_CALL) {
                        if (expr->Right->Left != NULL) {
                            code_string __attribute__((unused)) calleeR = Amalgame_Compiler_CGen_EmitCalleeStr(self, expr->Right->Left);
                            if (String_StartsWith(calleeR, "String_") || code_string_equals(calleeR, "code_string_concat")) {
                                useConcat = 1;
                            }
                        }
                    }
                }
            }
            if (useConcat) {
                code_string __attribute__((unused)) lcast = left;
                code_string __attribute__((unused)) rcast = right;
                code_string __attribute__((unused)) ltype4 = Amalgame_Compiler_CGen_InferTypeFromExpr(self, expr->Left);
                code_string __attribute__((unused)) rtype4 = Amalgame_Compiler_CGen_InferTypeFromExpr(self, expr->Right);
                if (code_string_equals(ltype4, "void*")) {
                    lcast = code_string_concat(code_string_concat("(code_string)(", left), ")");
                }
                if (code_string_equals(rtype4, "void*")) {
                    rcast = code_string_concat(code_string_concat("(code_string)(", right), ")");
                }
                return code_string_concat(code_string_concat(code_string_concat(code_string_concat("code_string_concat(", lcast), ", "), rcast), ")");
            }
        }
        return code_string_concat(code_string_concat(code_string_concat(code_string_concat(left, " "), op), " "), right);
    }
    if (k == Amalgame_Compiler_NodeKind_UNARY) {
        code_string __attribute__((unused)) operand = Amalgame_Compiler_CGen_EmitExprStr(self, expr->Left);
        return code_string_concat(expr->Str, operand);
    }
    if (k == Amalgame_Compiler_NodeKind_CALL) {
        code_string __attribute__((unused)) listCall = Amalgame_Compiler_CGen_TryEmitListCall(self, expr);
        if (String_Length(listCall) > 0) {
            return listCall;
        }
        code_bool __attribute__((unused)) isSelfCall = 0;
        code_string __attribute__((unused)) selfExpr = "self";
        if (expr->Left != NULL) {
            if (expr->Left->Kind == Amalgame_Compiler_NodeKind_MEMBER) {
                if (expr->Left->Left != NULL) {
                    Amalgame_Compiler_AstNode* __attribute__((unused)) ll = expr->Left->Left;
                    if (ll->Kind == Amalgame_Compiler_NodeKind_THIS_EXPR) {
                        isSelfCall = 1;
                        selfExpr = "self";
                    }
                    if (ll->Kind == Amalgame_Compiler_NodeKind_MEMBER) {
                        if (ll->Left != NULL) {
                            if (ll->Left->Kind == Amalgame_Compiler_NodeKind_THIS_EXPR) {
                                isSelfCall = 1;
                                selfExpr = code_string_concat("self->", ll->Name);
                            }
                        }
                    }
                    if (ll->Kind == Amalgame_Compiler_NodeKind_IDENTIFIER) {
                        code_string __attribute__((unused)) vt = Amalgame_Compiler_CGen_LocalTypeGet(self, ll->Name);
                        code_string __attribute__((unused)) bare = String_Replace(vt, "*", "");
                        if (String_Length(bare) > 0 && !Amalgame_Compiler_CGen_IsEnum(self, bare)) {
                            isSelfCall = 1;
                            selfExpr = ll->Name;
                        }
                    }
                }
            }
        }
        code_string __attribute__((unused)) callee = Amalgame_Compiler_CGen_EmitCalleeStr(self, expr->Left);
        code_string __attribute__((unused)) callStr = code_string_concat(callee, "(");
        code_bool __attribute__((unused)) first = 1;
        if (isSelfCall) {
            callStr = code_string_concat(callStr, selfExpr);
            first = 0;
        }
        i64 __attribute__((unused)) argc = AmalgameList_count(expr->Args);
        for (i64 i = 0; i < argc; i++) {
            Amalgame_Compiler_AstNode* __attribute__((unused)) arg = (Amalgame_Compiler_AstNode*)AmalgameList_get(expr->Args, i);
            if (!first) {
                callStr = code_string_concat(callStr, ", ");
            }
            callStr = code_string_concat(callStr, Amalgame_Compiler_CGen_EmitExprStr(self, arg));
            first = 0;
        }
        callStr = code_string_concat(callStr, ")");
        return callStr;
    }
    if (k == Amalgame_Compiler_NodeKind_NEW_EXPR) {
        code_string __attribute__((unused)) tname = expr->Name;
        code_string __attribute__((unused)) newCall = "";
        if (String_StartsWith(tname, "List<") || code_string_equals(tname, "List")) {
            newCall = "AmalgameList_new()";
        } else {
            if (String_StartsWith(tname, "Map<") || code_string_equals(tname, "Map")) {
                newCall = "AmalgameMap_new()";
            } else {
                if (String_StartsWith(tname, "Set<") || code_string_equals(tname, "Set")) {
                    newCall = "AmalgameSet_new()";
                } else {
                    newCall = code_string_concat(Amalgame_Compiler_CGen_SymName(self, tname), "_new(");
                    i64 __attribute__((unused)) argc = AmalgameList_count(expr->Args);
                    for (i64 i = 0; i < argc; i++) {
                        Amalgame_Compiler_AstNode* __attribute__((unused)) arg = (Amalgame_Compiler_AstNode*)AmalgameList_get(expr->Args, i);
                        if (i > 0) {
                            newCall = code_string_concat(newCall, ", ");
                        }
                        newCall = code_string_concat(newCall, Amalgame_Compiler_CGen_EmitExprStr(self, arg));
                    }
                    newCall = code_string_concat(newCall, ")");
                }
            }
        }
        return newCall;
    }
    if (k == Amalgame_Compiler_NodeKind_INDEX_EXPR) {
        code_string __attribute__((unused)) base = Amalgame_Compiler_CGen_EmitExprStr(self, expr->Left);
        code_string __attribute__((unused)) idx = Amalgame_Compiler_CGen_EmitExprStr(self, expr->Right);
        return code_string_concat(code_string_concat(code_string_concat(base, "["), idx), "]");
    }
    return "/* unknown expr */";
}

static code_string Amalgame_Compiler_CGen_TryEmitListCall(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* callExpr) {
    (void)self;
    (void)callExpr;
    if (callExpr->Left == NULL) {
        return "";
    }
    if (callExpr->Left->Kind != Amalgame_Compiler_NodeKind_MEMBER) {
        return "";
    }
    Amalgame_Compiler_AstNode* __attribute__((unused)) callee = callExpr->Left;
    code_string __attribute__((unused)) mname = callee->Name;
    if (code_string_equals(mname, "Set") || code_string_equals(mname, "Has") || code_string_equals(mname, "Size") || code_string_equals(mname, "Remove")) {
        if (callee->Left != NULL && callee->Left->Kind == Amalgame_Compiler_NodeKind_IDENTIFIER) {
            code_string __attribute__((unused)) vname = callee->Left->Name;
            code_string __attribute__((unused)) vtype = Amalgame_Compiler_CGen_LocalTypeGet(self, vname);
            if (code_string_equals(vtype, "AmalgameMap*")) {
                AmalgameList* __attribute__((unused)) args = callExpr->Args;
                i64 __attribute__((unused)) ac = AmalgameList_count(args);
                if (code_string_equals(mname, "Set") && ac >= 2) {
                    code_string __attribute__((unused)) k2 = Amalgame_Compiler_CGen_EmitExprStr(self, (void*)AmalgameList_get(args, 0));
                    code_string __attribute__((unused)) v2 = Amalgame_Compiler_CGen_EmitExprStr(self, (void*)AmalgameList_get(args, 1));
                    return code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("AmalgameMap_set(", vname), ", "), k2), ", (void*)(intptr_t)("), v2), "))");
                }
                if (code_string_equals(mname, "Has") && ac >= 1) {
                    code_string __attribute__((unused)) k2 = Amalgame_Compiler_CGen_EmitExprStr(self, (void*)AmalgameList_get(args, 0));
                    return code_string_concat(code_string_concat(code_string_concat(code_string_concat("AmalgameMap_has(", vname), ", "), k2), ")");
                }
                if (code_string_equals(mname, "Size")) {
                    return code_string_concat(code_string_concat("AmalgameMap_size(", vname), ")");
                }
                if (code_string_equals(mname, "Remove") && ac >= 1) {
                    code_string __attribute__((unused)) k2 = Amalgame_Compiler_CGen_EmitExprStr(self, (void*)AmalgameList_get(args, 0));
                    return code_string_concat(code_string_concat(code_string_concat(code_string_concat("AmalgameMap_remove(", vname), ", "), k2), ")");
                }
            }
        }
    }
    if (code_string_equals(mname, "Contains") || code_string_equals(mname, "Size")) {
        if (callee->Left != NULL && callee->Left->Kind == Amalgame_Compiler_NodeKind_IDENTIFIER) {
            code_string __attribute__((unused)) vname = callee->Left->Name;
            code_string __attribute__((unused)) vtype = Amalgame_Compiler_CGen_LocalTypeGet(self, vname);
            if (code_string_equals(vtype, "AmalgameSet*")) {
                AmalgameList* __attribute__((unused)) args = callExpr->Args;
                i64 __attribute__((unused)) ac = AmalgameList_count(args);
                if (code_string_equals(mname, "Contains") && ac >= 1) {
                    code_string __attribute__((unused)) v2 = Amalgame_Compiler_CGen_EmitExprStr(self, (void*)AmalgameList_get(args, 0));
                    return code_string_concat(code_string_concat(code_string_concat(code_string_concat("AmalgameSet_contains(", vname), ", "), v2), ")");
                }
                if (code_string_equals(mname, "Size")) {
                    return code_string_concat(code_string_concat("AmalgameSet_size(", vname), ")");
                }
            }
        }
    }
    if (code_string_equals(mname, "Add") || code_string_equals(mname, "Remove")) {
        if (callee->Left != NULL && callee->Left->Kind == Amalgame_Compiler_NodeKind_IDENTIFIER) {
            code_string __attribute__((unused)) vname = callee->Left->Name;
            code_string __attribute__((unused)) vtype = Amalgame_Compiler_CGen_LocalTypeGet(self, vname);
            if (code_string_equals(vtype, "AmalgameSet*")) {
                AmalgameList* __attribute__((unused)) args = callExpr->Args;
                i64 __attribute__((unused)) ac = AmalgameList_count(args);
                if (ac >= 1) {
                    code_string __attribute__((unused)) v2 = Amalgame_Compiler_CGen_EmitExprStr(self, (void*)AmalgameList_get(args, 0));
                    if (code_string_equals(mname, "Add")) {
                        return code_string_concat(code_string_concat(code_string_concat(code_string_concat("AmalgameSet_add(", vname), ", "), v2), ")");
                    }
                    return code_string_concat(code_string_concat(code_string_concat(code_string_concat("AmalgameSet_remove(", vname), ", "), v2), ")");
                }
            }
        }
    }
    if (!code_string_equals(mname, "Add") && !code_string_equals(mname, "Count") && !code_string_equals(mname, "Get") && !code_string_equals(mname, "IsEmpty") && !code_string_equals(mname, "Remove")) {
        return "";
    }
    code_string __attribute__((unused)) listExpr = "";
    code_string __attribute__((unused)) listCType = "";
    if (callee->Left != NULL) {
        Amalgame_Compiler_NodeKind __attribute__((unused)) lk = callee->Left->Kind;
        if (lk == Amalgame_Compiler_NodeKind_IDENTIFIER) {
            code_string __attribute__((unused)) vname = callee->Left->Name;
            code_string __attribute__((unused)) vtype = Amalgame_Compiler_CGen_LocalTypeGet(self, vname);
            if (code_string_equals(vtype, "AmalgameList*")) {
                listExpr = vname;
                listCType = "AmalgameList*";
            }
        }
        if (lk == Amalgame_Compiler_NodeKind_MEMBER) {
            if (callee->Left->Left != NULL) {
                Amalgame_Compiler_NodeKind __attribute__((unused)) llk = callee->Left->Left->Kind;
                if (llk == Amalgame_Compiler_NodeKind_THIS_EXPR) {
                    code_string __attribute__((unused)) fname = callee->Left->Name;
                    code_string __attribute__((unused)) ftype = Amalgame_Compiler_CGen_FieldTypeGet(self, self->CurrentClass, fname);
                    if (code_string_equals(ftype, "AmalgameList*")) {
                        listExpr = code_string_concat("self->", fname);
                        listCType = "AmalgameList*";
                    }
                }
                if (llk == Amalgame_Compiler_NodeKind_IDENTIFIER) {
                    code_string __attribute__((unused)) vname2 = callee->Left->Left->Name;
                    code_string __attribute__((unused)) vtype2 = Amalgame_Compiler_CGen_LocalTypeGet(self, vname2);
                    code_string __attribute__((unused)) fname2 = callee->Left->Name;
                    code_string __attribute__((unused)) vbare = String_Replace(vtype2, "*", "");
                    code_string __attribute__((unused)) ftype2 = Amalgame_Compiler_CGen_FieldTypeGet(self, vbare, fname2);
                    if (code_string_equals(ftype2, "AmalgameList*")) {
                        listExpr = code_string_concat(code_string_concat(vname2, "->"), fname2);
                        listCType = "AmalgameList*";
                    }
                }
            }
        }
    }
    if (String_Length(listExpr) == 0) {
        return "";
    }
    if (code_string_equals(mname, "Count")) {
        return code_string_concat(code_string_concat("AmalgameList_count(", listExpr), ")");
    }
    if (code_string_equals(mname, "Add")) {
        i64 __attribute__((unused)) argc = AmalgameList_count(callExpr->Args);
        if (argc == 0) {
            return code_string_concat(code_string_concat("AmalgameList_add(", listExpr), ", NULL)");
        }
        code_string __attribute__((unused)) arg0 = Amalgame_Compiler_CGen_EmitExprStr(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(callExpr->Args, 0));
        return code_string_concat(code_string_concat(code_string_concat(code_string_concat("AmalgameList_add(", listExpr), ", (void*)(intptr_t)("), arg0), "))");
    }
    if (code_string_equals(mname, "Get")) {
        i64 __attribute__((unused)) argc = AmalgameList_count(callExpr->Args);
        code_string __attribute__((unused)) idx0 = "0";
        if (argc > 0) {
            idx0 = Amalgame_Compiler_CGen_EmitExprStr(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(callExpr->Args, 0));
        }
        code_string __attribute__((unused)) elemType = "";
        if (callee->Left != NULL) {
            Amalgame_Compiler_NodeKind __attribute__((unused)) lkG = callee->Left->Kind;
            if (lkG == Amalgame_Compiler_NodeKind_THIS_EXPR) {
            }
            if (lkG == Amalgame_Compiler_NodeKind_MEMBER) {
                if (callee->Left->Left != NULL) {
                    Amalgame_Compiler_AstNode* __attribute__((unused)) lll = callee->Left->Left;
                    if (lll->Kind == Amalgame_Compiler_NodeKind_THIS_EXPR) {
                        code_string __attribute__((unused)) fn4 = callee->Left->Name;
                        elemType = Amalgame_Compiler_CGen_ListElemGet(self, self->CurrentClass, fn4);
                    }
                    if (lll->Kind == Amalgame_Compiler_NodeKind_IDENTIFIER) {
                        code_string __attribute__((unused)) vn4 = lll->Name;
                        code_string __attribute__((unused)) vt4 = Amalgame_Compiler_CGen_LocalTypeGet(self, vn4);
                        code_string __attribute__((unused)) bare4 = String_Replace(vt4, "*", "");
                        code_string __attribute__((unused)) fn4b = callee->Left->Name;
                        elemType = Amalgame_Compiler_CGen_ListElemGet(self, bare4, fn4b);
                    }
                }
            }
            if (lkG == Amalgame_Compiler_NodeKind_IDENTIFIER) {
                code_string __attribute__((unused)) vn5 = callee->Left->Name;
                code_string __attribute__((unused)) _ = vn5;
            }
        }
        if (String_Length(elemType) > 0) {
            return code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("(", elemType), ")AmalgameList_get("), listExpr), ", "), idx0), ")");
        }
        return code_string_concat(code_string_concat(code_string_concat(code_string_concat("(void*)AmalgameList_get(", listExpr), ", "), idx0), ")");
    }
    if (code_string_equals(mname, "IsEmpty")) {
        return code_string_concat(code_string_concat("AmalgameList_isEmpty(", listExpr), ")");
    }
    if (code_string_equals(mname, "Remove")) {
        i64 __attribute__((unused)) argc = AmalgameList_count(callExpr->Args);
        if (argc > 0) {
            code_string __attribute__((unused)) arg0 = Amalgame_Compiler_CGen_EmitExprStr(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(callExpr->Args, 0));
            return code_string_concat(code_string_concat(code_string_concat(code_string_concat("AmalgameList_remove(", listExpr), ", (void*)(intptr_t)("), arg0), "))");
        }
    }
    return "";
}

static code_string Amalgame_Compiler_CGen_EmitCalleeStr(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* callee) {
    (void)self;
    (void)callee;
    if (callee == NULL) {
        return "NULL";
    }
    Amalgame_Compiler_NodeKind __attribute__((unused)) k = callee->Kind;
    if (k == Amalgame_Compiler_NodeKind_IDENTIFIER) {
        return callee->Name;
    }
    if (k == Amalgame_Compiler_NodeKind_MEMBER) {
        code_string __attribute__((unused)) mname = callee->Name;
        if (callee->Left != NULL) {
            Amalgame_Compiler_NodeKind __attribute__((unused)) lk = callee->Left->Kind;
            if (lk == Amalgame_Compiler_NodeKind_THIS_EXPR) {
                return code_string_concat(code_string_concat(self->CurrentClass, "_"), mname);
            }
            if (lk == Amalgame_Compiler_NodeKind_IDENTIFIER) {
                code_string __attribute__((unused)) tname = callee->Left->Name;
                code_string __attribute__((unused)) firstChar = String_Substring(tname, 0, 1);
                code_bool __attribute__((unused)) isUpper = code_string_equals(firstChar, String_ToUpper(firstChar));
                if (isUpper) {
                    code_bool __attribute__((unused)) isStdlib = code_string_equals(tname, "Console") || code_string_equals(tname, "File") || code_string_equals(tname, "Math") || code_string_equals(tname, "String") || code_string_equals(tname, "List") || code_string_equals(tname, "Env");
                    if (isStdlib) {
                        return code_string_concat(code_string_concat(tname, "_"), mname);
                    }
                    return code_string_concat(code_string_concat(Amalgame_Compiler_CGen_SymName(self, tname), "_"), mname);
                }
                code_string __attribute__((unused)) varType = Amalgame_Compiler_CGen_LocalTypeGet(self, tname);
                code_string __attribute__((unused)) bareType = String_Replace(varType, "*", "");
                if (String_Length(bareType) > 0) {
                    return code_string_concat(code_string_concat(bareType, "_"), mname);
                }
                return code_string_concat(code_string_concat(tname, "_"), mname);
            }
            if (lk == Amalgame_Compiler_NodeKind_MEMBER) {
                if (callee->Left->Left != NULL) {
                    Amalgame_Compiler_NodeKind __attribute__((unused)) llk = callee->Left->Left->Kind;
                    if (llk == Amalgame_Compiler_NodeKind_THIS_EXPR) {
                        code_string __attribute__((unused)) fname = callee->Left->Name;
                        code_string __attribute__((unused)) ftype = Amalgame_Compiler_CGen_FieldTypeGet(self, self->CurrentClass, fname);
                        code_string __attribute__((unused)) bare = String_Replace(ftype, "*", "");
                        if (String_Length(bare) > 0) {
                            return code_string_concat(code_string_concat(bare, "_"), mname);
                        }
                    }
                }
            }
        }
        code_string __attribute__((unused)) target = Amalgame_Compiler_CGen_EmitExprStr(self, callee->Left);
        return code_string_concat(code_string_concat(target, "_"), mname);
    }
    return Amalgame_Compiler_CGen_EmitExprStr(self, callee);
}

static code_bool Amalgame_Compiler_CGen_IsEnum(Amalgame_Compiler_CGen* self, code_string t) {
    (void)self;
    (void)t;
    i64 __attribute__((unused)) n = AmalgameList_count(self->EnumNames);
    for (i64 i = 0; i < n; i++) {
        code_string __attribute__((unused)) e = (code_string)AmalgameList_get(self->EnumNames, i);
        if (code_string_equals(e, t)) {
            return 1;
        }
    }
    return 0;
}

static code_string Amalgame_Compiler_CGen_TypeToC(Amalgame_Compiler_CGen* self, code_string t) {
    (void)self;
    (void)t;
    if (code_string_equals(t, "int")) {
        return "i64";
    }
    if (code_string_equals(t, "float")) {
        return "double";
    }
    if (code_string_equals(t, "bool")) {
        return "code_bool";
    }
    if (code_string_equals(t, "string")) {
        return "code_string";
    }
    if (code_string_equals(t, "void")) {
        return "void";
    }
    if (code_string_equals(t, "")) {
        return "void";
    }
    if (String_EndsWith(t, "[]")) {
        code_string __attribute__((unused)) inner = String_Substring(t, 0, String_Length(t) - 2);
        return code_string_concat(Amalgame_Compiler_CGen_TypeToC(self, inner), "*");
    }
    if (String_EndsWith(t, "?")) {
        code_string __attribute__((unused)) inner = String_Substring(t, 0, String_Length(t) - 1);
        return code_string_concat(Amalgame_Compiler_CGen_TypeToC(self, inner), "*");
    }
    if (String_StartsWith(t, "List<")) {
        return "AmalgameList*";
    }
    if (String_StartsWith(t, "Map<")) {
        return "AmalgameMap*";
    }
    if (String_StartsWith(t, "Set<")) {
        return "AmalgameSet*";
    }
    if (code_string_equals(t, "List")) {
        return "AmalgameList*";
    }
    if (code_string_equals(t, "Map")) {
        return "AmalgameMap*";
    }
    if (code_string_equals(t, "Set")) {
        return "AmalgameSet*";
    }
    if (Amalgame_Compiler_CGen_IsEnum(self, t)) {
        return Amalgame_Compiler_CGen_SymName(self, t);
    }
    code_string __attribute__((unused)) sym = Amalgame_Compiler_CGen_SymName(self, t);
    if (String_Length(t) == 1) {
        code_string __attribute__((unused)) fc = String_Substring(t, 0, 1);
        code_bool __attribute__((unused)) isUp = code_string_equals(fc, String_ToUpper(fc));
        if (isUp) {
            return "void*";
        }
    }
    if (Amalgame_Compiler_CGen_IsEnum(self, sym)) {
        return sym;
    }
    code_string __attribute__((unused)) algKey = code_string_concat("__alg__", sym);
    i64 __attribute__((unused)) en2 = AmalgameList_count(self->EnumNames);
    for (i64 ei2 = 0; ei2 < en2; ei2++) {
        if (code_string_equals((code_string)AmalgameList_get(self->EnumNames, ei2), algKey)) {
            return sym;
        }
    }
    return code_string_concat(sym, "*");
}

enum _Amalgame_Compiler_SymKind {
    Amalgame_Compiler_SymKind_CLASS,
    Amalgame_Compiler_SymKind_ENUM,
    Amalgame_Compiler_SymKind_METHOD,
    Amalgame_Compiler_SymKind_FIELD,
    Amalgame_Compiler_SymKind_PARAM,
    Amalgame_Compiler_SymKind_LOCAL,
    Amalgame_Compiler_SymKind_BUILTIN
};

struct _Amalgame_Compiler_Symbol {
    code_string Name;
    Amalgame_Compiler_SymKind Kind;
    code_string TypeName;
    code_bool IsPublic;
    code_bool IsStatic;
};


Amalgame_Compiler_Symbol* Amalgame_Compiler_Symbol_new(code_string name, Amalgame_Compiler_SymKind kind) {
    Amalgame_Compiler_Symbol* self = (Amalgame_Compiler_Symbol*) GC_MALLOC(sizeof(Amalgame_Compiler_Symbol));
    self->Name = name;
    self->Kind = kind;
    self->TypeName = "";
    self->IsPublic = 0;
    self->IsStatic = 0;
    return self;
}

struct _Amalgame_Compiler_SymbolTable {
    AmalgameList* Symbols;
    Amalgame_Compiler_SymbolTable* Parent;
};

void Amalgame_Compiler_SymbolTable_Declare(Amalgame_Compiler_SymbolTable* self, Amalgame_Compiler_Symbol* sym);
code_bool Amalgame_Compiler_SymbolTable_Has(Amalgame_Compiler_SymbolTable* self, code_string name);
Amalgame_Compiler_Symbol* Amalgame_Compiler_SymbolTable_Lookup(Amalgame_Compiler_SymbolTable* self, code_string name);
code_bool Amalgame_Compiler_SymbolTable_HasSymbol(Amalgame_Compiler_SymbolTable* self, code_string name);
code_string Amalgame_Compiler_SymbolTable_GetTypeName(Amalgame_Compiler_SymbolTable* self, code_string name);
void Amalgame_Compiler_SymbolTable_SetTypeName(Amalgame_Compiler_SymbolTable* self, code_string name, code_string typeName);

Amalgame_Compiler_SymbolTable* Amalgame_Compiler_SymbolTable_new() {
    Amalgame_Compiler_SymbolTable* self = (Amalgame_Compiler_SymbolTable*) GC_MALLOC(sizeof(Amalgame_Compiler_SymbolTable));
    self->Symbols = AmalgameList_new();
    return self;
}

void Amalgame_Compiler_SymbolTable_Declare(Amalgame_Compiler_SymbolTable* self, Amalgame_Compiler_Symbol* sym) {
    (void)self;
    (void)sym;
    AmalgameList_add(self->Symbols, (void*)(intptr_t)(sym));
}

code_bool Amalgame_Compiler_SymbolTable_Has(Amalgame_Compiler_SymbolTable* self, code_string name) {
    (void)self;
    (void)name;
    i64 __attribute__((unused)) count = AmalgameList_count(self->Symbols);
    for (i64 i = 0; i < count; i++) {
        Amalgame_Compiler_Symbol* __attribute__((unused)) sym = (Amalgame_Compiler_Symbol*)AmalgameList_get(self->Symbols, i);
        if (code_string_equals(sym->Name, name)) {
            return 1;
        }
    }
    return 0;
}

Amalgame_Compiler_Symbol* Amalgame_Compiler_SymbolTable_Lookup(Amalgame_Compiler_SymbolTable* self, code_string name) {
    (void)self;
    (void)name;
    i64 __attribute__((unused)) count = AmalgameList_count(self->Symbols);
    for (i64 i = 0; i < count; i++) {
        Amalgame_Compiler_Symbol* __attribute__((unused)) sym = (Amalgame_Compiler_Symbol*)AmalgameList_get(self->Symbols, i);
        if (code_string_equals(sym->Name, name)) {
            return sym;
        }
    }
    return Amalgame_Compiler_Symbol_new("_not_found_", Amalgame_Compiler_SymKind_BUILTIN);
}

code_bool Amalgame_Compiler_SymbolTable_HasSymbol(Amalgame_Compiler_SymbolTable* self, code_string name) {
    (void)self;
    (void)name;
    i64 __attribute__((unused)) count = AmalgameList_count(self->Symbols);
    for (i64 i = 0; i < count; i++) {
        Amalgame_Compiler_Symbol* __attribute__((unused)) sym = (Amalgame_Compiler_Symbol*)AmalgameList_get(self->Symbols, i);
        if (code_string_equals(sym->Name, name)) {
            return 1;
        }
    }
    return 0;
}

code_string Amalgame_Compiler_SymbolTable_GetTypeName(Amalgame_Compiler_SymbolTable* self, code_string name) {
    (void)self;
    (void)name;
    i64 __attribute__((unused)) count = AmalgameList_count(self->Symbols);
    for (i64 i = 0; i < count; i++) {
        Amalgame_Compiler_Symbol* __attribute__((unused)) sym = (Amalgame_Compiler_Symbol*)AmalgameList_get(self->Symbols, i);
        if (code_string_equals(sym->Name, name)) {
            return sym->TypeName;
        }
    }
    return "?";
}

void Amalgame_Compiler_SymbolTable_SetTypeName(Amalgame_Compiler_SymbolTable* self, code_string name, code_string typeName) {
    (void)self;
    (void)name;
    (void)typeName;
    i64 __attribute__((unused)) count = AmalgameList_count(self->Symbols);
    for (i64 i = 0; i < count; i++) {
        Amalgame_Compiler_Symbol* __attribute__((unused)) sym = (Amalgame_Compiler_Symbol*)AmalgameList_get(self->Symbols, i);
        if (code_string_equals(sym->Name, name)) {
            if (String_Length(sym->TypeName) == 0) {
                sym->TypeName = typeName;
            }
            return;
        }
    }
}

struct _Amalgame_Compiler_Resolver {
    Amalgame_Compiler_SymbolTable* Global;
    AmalgameList* Errors;
    Amalgame_Compiler_SymbolTable* Current;
    AmalgameList* Programs;
};

static void Amalgame_Compiler_Resolver_RegisterBuiltins(Amalgame_Compiler_Resolver* self);
void Amalgame_Compiler_Resolver_Resolve(Amalgame_Compiler_Resolver* self, Amalgame_Compiler_AstNode* prog);
void Amalgame_Compiler_Resolver_ResolvePrograms(Amalgame_Compiler_Resolver* self);
code_bool Amalgame_Compiler_Resolver_HasErrors(Amalgame_Compiler_Resolver* self);
code_string Amalgame_Compiler_Resolver_GetErrors(Amalgame_Compiler_Resolver* self);
static void Amalgame_Compiler_Resolver_CollectDecl(Amalgame_Compiler_Resolver* self, Amalgame_Compiler_AstNode* decl);
static void Amalgame_Compiler_Resolver_ResolveDecl(Amalgame_Compiler_Resolver* self, Amalgame_Compiler_AstNode* decl);
static void Amalgame_Compiler_Resolver_ResolveClass(Amalgame_Compiler_Resolver* self, Amalgame_Compiler_AstNode* cls);
static void Amalgame_Compiler_Resolver_ResolveMethod(Amalgame_Compiler_Resolver* self, Amalgame_Compiler_AstNode* method, Amalgame_Compiler_SymbolTable* classScope);
static void Amalgame_Compiler_Resolver_ResolveBlock(Amalgame_Compiler_Resolver* self, Amalgame_Compiler_AstNode* block);
static void Amalgame_Compiler_Resolver_ResolveStmt(Amalgame_Compiler_Resolver* self, Amalgame_Compiler_AstNode* stmt);
static void Amalgame_Compiler_Resolver_ResolveExpr(Amalgame_Compiler_Resolver* self, Amalgame_Compiler_AstNode* expr);
static code_bool Amalgame_Compiler_Resolver_LookupInScopes(Amalgame_Compiler_Resolver* self, code_string name);

Amalgame_Compiler_Resolver* Amalgame_Compiler_Resolver_new() {
    Amalgame_Compiler_Resolver* self = (Amalgame_Compiler_Resolver*) GC_MALLOC(sizeof(Amalgame_Compiler_Resolver));
    self->Global = Amalgame_Compiler_SymbolTable_new();
    self->Errors = AmalgameList_new();
    self->Current = self->Global;
    self->Programs = AmalgameList_new();
    Amalgame_Compiler_Resolver_RegisterBuiltins(self);
    return self;
}

static void Amalgame_Compiler_Resolver_RegisterBuiltins(Amalgame_Compiler_Resolver* self) {
    (void)self;
    AmalgameList* __attribute__((unused)) builtins = AmalgameList_new();
    AmalgameList_add(builtins, (void*)(intptr_t)("Console"));
    AmalgameList_add(builtins, (void*)(intptr_t)("File"));
    AmalgameList_add(builtins, (void*)(intptr_t)("Path"));
    AmalgameList_add(builtins, (void*)(intptr_t)("Math"));
    AmalgameList_add(builtins, (void*)(intptr_t)("String"));
    AmalgameList_add(builtins, (void*)(intptr_t)("Http"));
    AmalgameList_add(builtins, (void*)(intptr_t)("int"));
    AmalgameList_add(builtins, (void*)(intptr_t)("string"));
    AmalgameList_add(builtins, (void*)(intptr_t)("bool"));
    AmalgameList_add(builtins, (void*)(intptr_t)("float"));
    AmalgameList_add(builtins, (void*)(intptr_t)("void"));
    AmalgameList_add(builtins, (void*)(intptr_t)("String_Length"));
    AmalgameList_add(builtins, (void*)(intptr_t)("String_Contains"));
    AmalgameList_add(builtins, (void*)(intptr_t)("String_StartsWith"));
    AmalgameList_add(builtins, (void*)(intptr_t)("String_EndsWith"));
    AmalgameList_add(builtins, (void*)(intptr_t)("String_Substring"));
    AmalgameList_add(builtins, (void*)(intptr_t)("String_ToUpper"));
    AmalgameList_add(builtins, (void*)(intptr_t)("String_ToLower"));
    AmalgameList_add(builtins, (void*)(intptr_t)("String_Trim"));
    AmalgameList_add(builtins, (void*)(intptr_t)("String_Replace"));
    AmalgameList_add(builtins, (void*)(intptr_t)("String_IndexOf"));
    AmalgameList_add(builtins, (void*)(intptr_t)("String_FromInt"));
    AmalgameList_add(builtins, (void*)(intptr_t)("String_ToInt"));
    AmalgameList_add(builtins, (void*)(intptr_t)("String_IsEmpty"));
    AmalgameList_add(builtins, (void*)(intptr_t)("String_From"));
    AmalgameList_add(builtins, (void*)(intptr_t)("File_ReadAll"));
    AmalgameList_add(builtins, (void*)(intptr_t)("File_WriteAll"));
    AmalgameList_add(builtins, (void*)(intptr_t)("File_Exists"));
    AmalgameList_add(builtins, (void*)(intptr_t)("File_Delete"));
    AmalgameList_add(builtins, (void*)(intptr_t)("File_Size"));
    AmalgameList_add(builtins, (void*)(intptr_t)("Math_Sqrt"));
    AmalgameList_add(builtins, (void*)(intptr_t)("Math_Abs"));
    AmalgameList_add(builtins, (void*)(intptr_t)("Math_Max"));
    AmalgameList_add(builtins, (void*)(intptr_t)("Math_Min"));
    AmalgameList_add(builtins, (void*)(intptr_t)("null"));
    AmalgameList_add(builtins, (void*)(intptr_t)("true"));
    AmalgameList_add(builtins, (void*)(intptr_t)("false"));
    AmalgameList_add(builtins, (void*)(intptr_t)("this"));
    AmalgameList_add(builtins, (void*)(intptr_t)("args"));
    i64 __attribute__((unused)) count = AmalgameList_count(builtins);
    for (i64 i = 0; i < count; i++) {
        void* __attribute__((unused)) name = (void*)AmalgameList_get(builtins, i);
        Amalgame_Compiler_Symbol* __attribute__((unused)) sym = Amalgame_Compiler_Symbol_new(name, Amalgame_Compiler_SymKind_BUILTIN);
        Amalgame_Compiler_SymbolTable_Declare(self->Global, sym);
    }
}

void Amalgame_Compiler_Resolver_Resolve(Amalgame_Compiler_Resolver* self, Amalgame_Compiler_AstNode* prog) {
    (void)self;
    (void)prog;
    i64 __attribute__((unused)) decls = AmalgameList_count(prog->Children);
    for (i64 i = 0; i < decls; i++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) decl = (Amalgame_Compiler_AstNode*)AmalgameList_get(prog->Children, i);
        Amalgame_Compiler_Resolver_CollectDecl(self, decl);
    }
    for (i64 j = 0; j < decls; j++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) decl = (Amalgame_Compiler_AstNode*)AmalgameList_get(prog->Children, j);
        Amalgame_Compiler_Resolver_ResolveDecl(self, decl);
    }
}

void Amalgame_Compiler_Resolver_ResolvePrograms(Amalgame_Compiler_Resolver* self) {
    (void)self;
    i64 __attribute__((unused)) progCount = AmalgameList_count(self->Programs);
    for (i64 p = 0; p < progCount; p++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) prog = (Amalgame_Compiler_AstNode*)AmalgameList_get(self->Programs, p);
        i64 __attribute__((unused)) decls = AmalgameList_count(prog->Children);
        for (i64 i = 0; i < decls; i++) {
            Amalgame_Compiler_AstNode* __attribute__((unused)) decl = (Amalgame_Compiler_AstNode*)AmalgameList_get(prog->Children, i);
            Amalgame_Compiler_Resolver_CollectDecl(self, decl);
        }
    }
    for (i64 p2 = 0; p2 < progCount; p2++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) prog2 = (Amalgame_Compiler_AstNode*)AmalgameList_get(self->Programs, p2);
        i64 __attribute__((unused)) decls2 = AmalgameList_count(prog2->Children);
        for (i64 j = 0; j < decls2; j++) {
            Amalgame_Compiler_AstNode* __attribute__((unused)) decl = (Amalgame_Compiler_AstNode*)AmalgameList_get(prog2->Children, j);
            Amalgame_Compiler_Resolver_ResolveDecl(self, decl);
        }
    }
}

code_bool Amalgame_Compiler_Resolver_HasErrors(Amalgame_Compiler_Resolver* self) {
    (void)self;
    return AmalgameList_count(self->Errors) > 0;
}

code_string Amalgame_Compiler_Resolver_GetErrors(Amalgame_Compiler_Resolver* self) {
    (void)self;
    code_string __attribute__((unused)) result = "";
    i64 __attribute__((unused)) count = AmalgameList_count(self->Errors);
    for (i64 i = 0; i < count; i++) {
        code_string __attribute__((unused)) e = (code_string)AmalgameList_get(self->Errors, i);
        result = code_string_concat(code_string_concat(result, e), "\n");
    }
    return result;
}

static void Amalgame_Compiler_Resolver_CollectDecl(Amalgame_Compiler_Resolver* self, Amalgame_Compiler_AstNode* decl) {
    (void)self;
    (void)decl;
    Amalgame_Compiler_NodeKind __attribute__((unused)) k = decl->Kind;
    if (k == Amalgame_Compiler_NodeKind_CLASS_DECL) {
        Amalgame_Compiler_Symbol* __attribute__((unused)) sym = Amalgame_Compiler_Symbol_new(decl->Name, Amalgame_Compiler_SymKind_CLASS);
        sym->IsPublic = decl->Flag;
        Amalgame_Compiler_SymbolTable_Declare(self->Global, sym);
    }
    if (k == Amalgame_Compiler_NodeKind_ENUM_DECL) {
        Amalgame_Compiler_Symbol* __attribute__((unused)) esym = Amalgame_Compiler_Symbol_new(decl->Name, Amalgame_Compiler_SymKind_ENUM);
        esym->IsPublic = decl->Flag;
        Amalgame_Compiler_SymbolTable_Declare(self->Global, esym);
    }
}

static void Amalgame_Compiler_Resolver_ResolveDecl(Amalgame_Compiler_Resolver* self, Amalgame_Compiler_AstNode* decl) {
    (void)self;
    (void)decl;
    Amalgame_Compiler_NodeKind __attribute__((unused)) k = decl->Kind;
    if (k == Amalgame_Compiler_NodeKind_CLASS_DECL) {
        Amalgame_Compiler_Resolver_ResolveClass(self, decl);
    }
}

static void Amalgame_Compiler_Resolver_ResolveClass(Amalgame_Compiler_Resolver* self, Amalgame_Compiler_AstNode* cls) {
    (void)self;
    (void)cls;
    Amalgame_Compiler_SymbolTable* __attribute__((unused)) classScope = Amalgame_Compiler_SymbolTable_new();
    classScope->Parent = self->Global;
    Amalgame_Compiler_SymbolTable* __attribute__((unused)) prev = self->Current;
    self->Current = classScope;
    i64 __attribute__((unused)) members = AmalgameList_count(cls->Children);
    for (i64 i = 0; i < members; i++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) m = (Amalgame_Compiler_AstNode*)AmalgameList_get(cls->Children, i);
        Amalgame_Compiler_NodeKind __attribute__((unused)) mk = m->Kind;
        if (mk == Amalgame_Compiler_NodeKind_VAR_DECL) {
            Amalgame_Compiler_Symbol* __attribute__((unused)) fsym = Amalgame_Compiler_Symbol_new(m->Name, Amalgame_Compiler_SymKind_FIELD);
            fsym->TypeName = m->Str;
            fsym->IsPublic = m->Flag;
            Amalgame_Compiler_SymbolTable_Declare(classScope, fsym);
        }
        if (mk == Amalgame_Compiler_NodeKind_METHOD_DECL) {
            Amalgame_Compiler_Symbol* __attribute__((unused)) msym = Amalgame_Compiler_Symbol_new(m->Name, Amalgame_Compiler_SymKind_METHOD);
            msym->TypeName = m->Str;
            msym->IsPublic = m->Flag;
            msym->IsStatic = m->Flag2;
            Amalgame_Compiler_SymbolTable_Declare(classScope, msym);
        }
    }
    for (i64 j = 0; j < members; j++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) m = (Amalgame_Compiler_AstNode*)AmalgameList_get(cls->Children, j);
        Amalgame_Compiler_NodeKind __attribute__((unused)) mk = m->Kind;
        if (mk == Amalgame_Compiler_NodeKind_METHOD_DECL) {
            Amalgame_Compiler_Resolver_ResolveMethod(self, m, classScope);
        }
    }
    self->Current = prev;
}

static void Amalgame_Compiler_Resolver_ResolveMethod(Amalgame_Compiler_Resolver* self, Amalgame_Compiler_AstNode* method, Amalgame_Compiler_SymbolTable* classScope) {
    (void)self;
    (void)method;
    (void)classScope;
    Amalgame_Compiler_SymbolTable* __attribute__((unused)) methodScope = Amalgame_Compiler_SymbolTable_new();
    methodScope->Parent = classScope;
    Amalgame_Compiler_SymbolTable* __attribute__((unused)) prev = self->Current;
    self->Current = methodScope;
    i64 __attribute__((unused)) params = AmalgameList_count(method->Params);
    for (i64 i = 0; i < params; i++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) p = (Amalgame_Compiler_AstNode*)AmalgameList_get(method->Params, i);
        Amalgame_Compiler_Symbol* __attribute__((unused)) psym = Amalgame_Compiler_Symbol_new(p->Name, Amalgame_Compiler_SymKind_PARAM);
        psym->TypeName = p->Str;
        Amalgame_Compiler_SymbolTable_Declare(methodScope, psym);
    }
    if (method->Body != NULL) {
        Amalgame_Compiler_Resolver_ResolveBlock(self, method->Body);
    }
    self->Current = prev;
}

static void Amalgame_Compiler_Resolver_ResolveBlock(Amalgame_Compiler_Resolver* self, Amalgame_Compiler_AstNode* block) {
    (void)self;
    (void)block;
    i64 __attribute__((unused)) stmts = AmalgameList_count(block->Children);
    for (i64 i = 0; i < stmts; i++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) stmt = (Amalgame_Compiler_AstNode*)AmalgameList_get(block->Children, i);
        Amalgame_Compiler_Resolver_ResolveStmt(self, stmt);
    }
}

static void Amalgame_Compiler_Resolver_ResolveStmt(Amalgame_Compiler_Resolver* self, Amalgame_Compiler_AstNode* stmt) {
    (void)self;
    (void)stmt;
    Amalgame_Compiler_NodeKind __attribute__((unused)) k = stmt->Kind;
    if (k == Amalgame_Compiler_NodeKind_VAR_DECL) {
        if (stmt->Left != NULL) {
            Amalgame_Compiler_Resolver_ResolveExpr(self, stmt->Left);
        }
        Amalgame_Compiler_Symbol* __attribute__((unused)) vsym = Amalgame_Compiler_Symbol_new(stmt->Name, Amalgame_Compiler_SymKind_LOCAL);
        vsym->TypeName = stmt->Str;
        Amalgame_Compiler_SymbolTable_Declare(self->Current, vsym);
    }
    if (k == Amalgame_Compiler_NodeKind_RETURN_STMT) {
        if (stmt->Left != NULL) {
            Amalgame_Compiler_Resolver_ResolveExpr(self, stmt->Left);
        }
    }
    if (k == Amalgame_Compiler_NodeKind_IF_STMT) {
        if (stmt->Cond != NULL) {
            Amalgame_Compiler_Resolver_ResolveExpr(self, stmt->Cond);
        }
        if (stmt->Body != NULL) {
            Amalgame_Compiler_Resolver_ResolveBlock(self, stmt->Body);
        }
        if (stmt->Else != NULL) {
            Amalgame_Compiler_Resolver_ResolveBlock(self, stmt->Else);
        }
    }
    if (k == Amalgame_Compiler_NodeKind_WHILE_STMT) {
        if (stmt->Cond != NULL) {
            Amalgame_Compiler_Resolver_ResolveExpr(self, stmt->Cond);
        }
        if (stmt->Body != NULL) {
            Amalgame_Compiler_Resolver_ResolveBlock(self, stmt->Body);
        }
    }
    if (k == Amalgame_Compiler_NodeKind_BINARY) {
        Amalgame_Compiler_Resolver_ResolveExpr(self, stmt);
    }
    if (k == Amalgame_Compiler_NodeKind_CALL) {
        Amalgame_Compiler_Resolver_ResolveExpr(self, stmt);
    }
    if (k == Amalgame_Compiler_NodeKind_MEMBER) {
        Amalgame_Compiler_Resolver_ResolveExpr(self, stmt);
    }
}

static void Amalgame_Compiler_Resolver_ResolveExpr(Amalgame_Compiler_Resolver* self, Amalgame_Compiler_AstNode* expr) {
    (void)self;
    (void)expr;
    if (expr == NULL) {
        return;
    }
    Amalgame_Compiler_NodeKind __attribute__((unused)) k = expr->Kind;
    if (k == Amalgame_Compiler_NodeKind_IDENTIFIER) {
        code_string __attribute__((unused)) name = expr->Name;
        if (!Amalgame_Compiler_Resolver_LookupInScopes(self, name)) {
            i64 __attribute__((unused)) line = expr->Line;
            code_string __attribute__((unused)) ln = String_FromInt(line);
            AmalgameList_add(self->Errors, (void*)(intptr_t)(code_string_concat(code_string_concat(code_string_concat("Unknown symbol '", name), "' at line "), ln)));
        }
    }
    if (k == Amalgame_Compiler_NodeKind_BINARY) {
        if (expr->Left != NULL) {
            Amalgame_Compiler_Resolver_ResolveExpr(self, expr->Left);
        }
        if (expr->Right != NULL) {
            Amalgame_Compiler_Resolver_ResolveExpr(self, expr->Right);
        }
    }
    if (k == Amalgame_Compiler_NodeKind_CALL) {
        if (expr->Left != NULL) {
            Amalgame_Compiler_Resolver_ResolveExpr(self, expr->Left);
        }
        i64 __attribute__((unused)) argc = AmalgameList_count(expr->Args);
        for (i64 i = 0; i < argc; i++) {
            Amalgame_Compiler_Resolver_ResolveExpr(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(expr->Args, i));
        }
    }
    if (k == Amalgame_Compiler_NodeKind_MEMBER) {
        if (expr->Left != NULL) {
            Amalgame_Compiler_Resolver_ResolveExpr(self, expr->Left);
        }
    }
    if (k == Amalgame_Compiler_NodeKind_NEW_EXPR) {
        i64 __attribute__((unused)) argc2 = AmalgameList_count(expr->Args);
        for (i64 i = 0; i < argc2; i++) {
            Amalgame_Compiler_Resolver_ResolveExpr(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(expr->Args, i));
        }
    }
    if (k == Amalgame_Compiler_NodeKind_UNARY) {
        if (expr->Left != NULL) {
            Amalgame_Compiler_Resolver_ResolveExpr(self, expr->Left);
        }
    }
    if (k == Amalgame_Compiler_NodeKind_INDEX_EXPR) {
        if (expr->Left != NULL) {
            Amalgame_Compiler_Resolver_ResolveExpr(self, expr->Left);
        }
        if (expr->Right != NULL) {
            Amalgame_Compiler_Resolver_ResolveExpr(self, expr->Right);
        }
    }
}

static code_bool Amalgame_Compiler_Resolver_LookupInScopes(Amalgame_Compiler_Resolver* self, code_string name) {
    (void)self;
    (void)name;
    Amalgame_Compiler_SymbolTable* __attribute__((unused)) scope = self->Current;
    code_bool __attribute__((unused)) found = 0;
    i64 __attribute__((unused)) safety = 0;
    while (scope != NULL && !found) {
        safety = safety + 1;
        if (safety > 100) {
            break;
        }
        if (Amalgame_Compiler_SymbolTable_Has(scope, name)) {
            found = 1;
        }
        if (!found) {
            scope = scope->Parent;
        }
    }
    return found;
}

struct _Amalgame_Compiler_MemberTable {
    AmalgameList* Keys;
    AmalgameList* Values;
};

void Amalgame_Compiler_MemberTable_Set(Amalgame_Compiler_MemberTable* self, code_string className, code_string memberName, code_string typeName);
code_string Amalgame_Compiler_MemberTable_Get(Amalgame_Compiler_MemberTable* self, code_string className, code_string memberName);
code_bool Amalgame_Compiler_MemberTable_Has(Amalgame_Compiler_MemberTable* self, code_string className, code_string memberName);

Amalgame_Compiler_MemberTable* Amalgame_Compiler_MemberTable_new() {
    Amalgame_Compiler_MemberTable* self = (Amalgame_Compiler_MemberTable*) GC_MALLOC(sizeof(Amalgame_Compiler_MemberTable));
    self->Keys = AmalgameList_new();
    self->Values = AmalgameList_new();
    return self;
}

void Amalgame_Compiler_MemberTable_Set(Amalgame_Compiler_MemberTable* self, code_string className, code_string memberName, code_string typeName) {
    (void)self;
    (void)className;
    (void)memberName;
    (void)typeName;
    code_string __attribute__((unused)) key = code_string_concat(code_string_concat(className, "."), memberName);
    i64 __attribute__((unused)) count = AmalgameList_count(self->Keys);
    for (i64 i = 0; i < count; i++) {
        if (code_string_equals((code_string)AmalgameList_get(self->Keys, i), key)) {
            AmalgameList_add(self->Values, (void*)(intptr_t)(typeName));
            return;
        }
    }
    AmalgameList_add(self->Keys, (void*)(intptr_t)(key));
    AmalgameList_add(self->Values, (void*)(intptr_t)(typeName));
}

code_string Amalgame_Compiler_MemberTable_Get(Amalgame_Compiler_MemberTable* self, code_string className, code_string memberName) {
    (void)self;
    (void)className;
    (void)memberName;
    code_string __attribute__((unused)) key = code_string_concat(code_string_concat(className, "."), memberName);
    i64 __attribute__((unused)) count = AmalgameList_count(self->Keys);
    code_string __attribute__((unused)) result = "?";
    for (i64 i = 0; i < count; i++) {
        if (code_string_equals((code_string)AmalgameList_get(self->Keys, i), key)) {
            result = (code_string)AmalgameList_get(self->Values, i);
        }
    }
    return result;
}

code_bool Amalgame_Compiler_MemberTable_Has(Amalgame_Compiler_MemberTable* self, code_string className, code_string memberName) {
    (void)self;
    (void)className;
    (void)memberName;
    code_string __attribute__((unused)) key = code_string_concat(code_string_concat(className, "."), memberName);
    i64 __attribute__((unused)) count = AmalgameList_count(self->Keys);
    for (i64 i = 0; i < count; i++) {
        if (code_string_equals((code_string)AmalgameList_get(self->Keys, i), key)) {
            return 1;
        }
    }
    return 0;
}

struct _Amalgame_Compiler_Scope {
    AmalgameList* Names;
    AmalgameList* Types;
    AmalgameList* IsLets;
    code_string Label;
    Amalgame_Compiler_Scope* Parent;
};

code_bool Amalgame_Compiler_Scope_Declare(Amalgame_Compiler_Scope* self, code_string name, code_string typeName, code_bool isLet);
code_bool Amalgame_Compiler_Scope_Has(Amalgame_Compiler_Scope* self, code_string name);
code_string Amalgame_Compiler_Scope_GetType(Amalgame_Compiler_Scope* self, code_string name);
code_bool Amalgame_Compiler_Scope_IsLet(Amalgame_Compiler_Scope* self, code_string name);

Amalgame_Compiler_Scope* Amalgame_Compiler_Scope_new(code_string label) {
    Amalgame_Compiler_Scope* self = (Amalgame_Compiler_Scope*) GC_MALLOC(sizeof(Amalgame_Compiler_Scope));
    self->Names = AmalgameList_new();
    self->Types = AmalgameList_new();
    self->IsLets = AmalgameList_new();
    self->Label = label;
    self->Parent = NULL;
    return self;
}

code_bool Amalgame_Compiler_Scope_Declare(Amalgame_Compiler_Scope* self, code_string name, code_string typeName, code_bool isLet) {
    (void)self;
    (void)name;
    (void)typeName;
    (void)isLet;
    i64 __attribute__((unused)) count = AmalgameList_count(self->Names);
    for (i64 i = 0; i < count; i++) {
        if (code_string_equals((code_string)AmalgameList_get(self->Names, i), name)) {
            return 0;
        }
    }
    AmalgameList_add(self->Names, (void*)(intptr_t)(name));
    AmalgameList_add(self->Types, (void*)(intptr_t)(typeName));
    AmalgameList_add(self->IsLets, (void*)(intptr_t)(isLet));
    return 1;
}

code_bool Amalgame_Compiler_Scope_Has(Amalgame_Compiler_Scope* self, code_string name) {
    (void)self;
    (void)name;
    i64 __attribute__((unused)) count = AmalgameList_count(self->Names);
    for (i64 i = 0; i < count; i++) {
        if (code_string_equals((code_string)AmalgameList_get(self->Names, i), name)) {
            return 1;
        }
    }
    return 0;
}

code_string Amalgame_Compiler_Scope_GetType(Amalgame_Compiler_Scope* self, code_string name) {
    (void)self;
    (void)name;
    i64 __attribute__((unused)) count = AmalgameList_count(self->Names);
    for (i64 i = 0; i < count; i++) {
        if (code_string_equals((code_string)AmalgameList_get(self->Names, i), name)) {
            return (code_string)AmalgameList_get(self->Types, i);
        }
    }
    return "?";
}

code_bool Amalgame_Compiler_Scope_IsLet(Amalgame_Compiler_Scope* self, code_string name) {
    (void)self;
    (void)name;
    i64 __attribute__((unused)) count = AmalgameList_count(self->Names);
    for (i64 i = 0; i < count; i++) {
        if (code_string_equals((code_string)AmalgameList_get(self->Names, i), name)) {
            return (code_bool)AmalgameList_get(self->IsLets, i);
        }
    }
    return 0;
}

struct _Amalgame_Compiler_FullResolver {
    Amalgame_Compiler_Scope* Global;
    Amalgame_Compiler_Scope* Current;
    Amalgame_Compiler_MemberTable* Members;
    AmalgameList* Errors;
    AmalgameList* Programs;
    code_string CurrentClass;
    code_string CurrentReturn;
    i64 LoopDepth;
};

static void Amalgame_Compiler_FullResolver_RegisterBuiltins(Amalgame_Compiler_FullResolver* self);
static void Amalgame_Compiler_FullResolver_PushScope(Amalgame_Compiler_FullResolver* self, code_string label);
static void Amalgame_Compiler_FullResolver_PopScope(Amalgame_Compiler_FullResolver* self);
static void Amalgame_Compiler_FullResolver_DeclareGlobal(Amalgame_Compiler_FullResolver* self, code_string name, code_string typeName, code_bool isLet);
static code_bool Amalgame_Compiler_FullResolver_DeclareCurrent(Amalgame_Compiler_FullResolver* self, code_string name, code_string typeName, code_bool isLet);
static code_bool Amalgame_Compiler_FullResolver_LookupInScopes(Amalgame_Compiler_FullResolver* self, code_string name);
static code_string Amalgame_Compiler_FullResolver_LookupType(Amalgame_Compiler_FullResolver* self, code_string name);
static code_bool Amalgame_Compiler_FullResolver_LookupIsLet(Amalgame_Compiler_FullResolver* self, code_string name);
static void Amalgame_Compiler_FullResolver_Error(Amalgame_Compiler_FullResolver* self, code_string msg, Amalgame_Compiler_AstNode* node);
static void Amalgame_Compiler_FullResolver_ErrorMsg(Amalgame_Compiler_FullResolver* self, code_string msg);
code_bool Amalgame_Compiler_FullResolver_HasErrors(Amalgame_Compiler_FullResolver* self);
code_string Amalgame_Compiler_FullResolver_GetErrors(Amalgame_Compiler_FullResolver* self);
void Amalgame_Compiler_FullResolver_ResolvePrograms(Amalgame_Compiler_FullResolver* self);
static void Amalgame_Compiler_FullResolver_CollectProgram(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* prog);
static void Amalgame_Compiler_FullResolver_CollectDecl(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* decl);
static void Amalgame_Compiler_FullResolver_CollectClassMembers(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* cls);
static void Amalgame_Compiler_FullResolver_CollectClassMember(Amalgame_Compiler_FullResolver* self, code_string className, Amalgame_Compiler_AstNode* m);
static void Amalgame_Compiler_FullResolver_CollectEnumMembers(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* en);
static void Amalgame_Compiler_FullResolver_CollectEnumMember(Amalgame_Compiler_FullResolver* self, code_string enumName, Amalgame_Compiler_AstNode* m);
static void Amalgame_Compiler_FullResolver_ResolveEnumChild(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* m);
static void Amalgame_Compiler_FullResolver_ResolveProgram(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* prog);
static void Amalgame_Compiler_FullResolver_ResolveDecl(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* decl);
static void Amalgame_Compiler_FullResolver_ResolveClass(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* cls);
static void Amalgame_Compiler_FullResolver_PreRegisterMember(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* m);
static void Amalgame_Compiler_FullResolver_ResolveMember(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* m);
static void Amalgame_Compiler_FullResolver_ResolveMethod(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* method);
static void Amalgame_Compiler_FullResolver_ResolveBlock(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* block);
static void Amalgame_Compiler_FullResolver_ResolveStmt(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* stmt);
static void Amalgame_Compiler_FullResolver_ResolveIf(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* stmt);
static void Amalgame_Compiler_FullResolver_ResolveElseBranch(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* branch);
static void Amalgame_Compiler_FullResolver_ResolveForIn(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* stmt);
static void Amalgame_Compiler_FullResolver_ResolveAssign(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* stmt);
static void Amalgame_Compiler_FullResolver_ResolveMatch(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* stmt);
static void Amalgame_Compiler_FullResolver_ResolveMatchArm(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* arm);
static void Amalgame_Compiler_FullResolver_ResolveArmBody(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* body);
static void Amalgame_Compiler_FullResolver_ResolveExpr(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* expr);
static code_string Amalgame_Compiler_FullResolver_InferExprType(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* expr);
static code_string Amalgame_Compiler_FullResolver_CollectionElemType(Amalgame_Compiler_FullResolver* self, code_string typeKey);
code_string Amalgame_Compiler_FullResolver_GetMemberType(Amalgame_Compiler_FullResolver* self, code_string className, code_string memberName);
code_bool Amalgame_Compiler_FullResolver_HasMember(Amalgame_Compiler_FullResolver* self, code_string className, code_string memberName);
code_string Amalgame_Compiler_FullResolver_GetVarType(Amalgame_Compiler_FullResolver* self, code_string name);
code_bool Amalgame_Compiler_FullResolver_HasSymbol(Amalgame_Compiler_FullResolver* self, code_string name);
code_string Amalgame_Compiler_FullResolver_GetTypeName(Amalgame_Compiler_FullResolver* self, code_string name);
void Amalgame_Compiler_FullResolver_SetTypeName(Amalgame_Compiler_FullResolver* self, code_string name, code_string typeName);

Amalgame_Compiler_FullResolver* Amalgame_Compiler_FullResolver_new() {
    Amalgame_Compiler_FullResolver* self = (Amalgame_Compiler_FullResolver*) GC_MALLOC(sizeof(Amalgame_Compiler_FullResolver));
    self->Global = Amalgame_Compiler_Scope_new("global");
    self->Current = self->Global;
    self->Members = Amalgame_Compiler_MemberTable_new();
    self->Errors = AmalgameList_new();
    self->Programs = AmalgameList_new();
    self->CurrentClass = "";
    self->CurrentReturn = "void";
    self->LoopDepth = 0;
    Amalgame_Compiler_FullResolver_RegisterBuiltins(self);
    return self;
}

static void Amalgame_Compiler_FullResolver_RegisterBuiltins(Amalgame_Compiler_FullResolver* self) {
    (void)self;
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "int", "type", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "float", "type", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "double", "type", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "bool", "type", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "string", "type", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "void", "type", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "null", "null", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "true", "bool", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "false", "bool", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "this", "?", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "args", "string[]", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Console", "type", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Console_WriteLine", "void", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Console_WriteError", "void", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Console_Clear", "void", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "File", "type", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Path", "type", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "File_ReadAll", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "File_WriteAll", "void", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "File_AppendAll", "void", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "File_Exists", "bool", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "File_Delete", "bool", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "File_Size", "int", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Path_Combine", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Path_GetExtension", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Path_GetFilename", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Path_GetDirectory", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Math", "type", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Math_Sqrt", "float", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Math_Abs", "float", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Math_AbsI", "int", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Math_PowI", "int", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Math_Pow", "float", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Math_Floor", "float", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Math_Ceil", "float", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Math_Round", "float", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Math_MaxI", "int", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Math_MinI", "int", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Math_MaxF", "float", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Math_MinF", "float", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Math_ClampI", "int", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Math_Gcd", "int", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Math_IsPrime", "bool", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Math_IsFinite", "bool", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Math_IsNaN", "bool", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Math_SeedRandom", "void", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Math_Random", "float", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Math_RandomInt", "int", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "String_Length", "int", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "String_IsEmpty", "bool", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "String_Contains", "bool", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "String_StartsWith", "bool", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "String_EndsWith", "bool", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "String_IndexOf", "int", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "String_Substring", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "String_ToUpper", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "String_ToLower", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "String_Trim", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "String_TrimStart", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "String_TrimEnd", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "String_Replace", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "String_Split", "List<string>", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "String_Join", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "String_Repeat", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "String_ToInt", "int", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "String_ToFloat", "float", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "String_FromInt", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "String_FromFloat", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "String_From", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "String_CharAt", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Http", "type", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Http_Get", "?", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Http_Post", "?", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Http_GetWithHeaders", "?", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "TcpServer_Listen", "?", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "TcpServer_Accept", "?", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "TcpServer_Close", "void", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "TcpClient_Connect", "?", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "TcpClient_Send", "void", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "TcpClient_Receive", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "TcpClient_Close", "void", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "TcpConn_Send", "void", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "TcpConn_Receive", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "TcpConn_Close", "void", 0);
}

static void Amalgame_Compiler_FullResolver_PushScope(Amalgame_Compiler_FullResolver* self, code_string label) {
    (void)self;
    (void)label;
    Amalgame_Compiler_Scope* __attribute__((unused)) s = Amalgame_Compiler_Scope_new(label);
    s->Parent = self->Current;
    self->Current = s;
}

static void Amalgame_Compiler_FullResolver_PopScope(Amalgame_Compiler_FullResolver* self) {
    (void)self;
    if (self->Current->Parent != NULL) {
        self->Current = self->Current->Parent;
    }
}

static void Amalgame_Compiler_FullResolver_DeclareGlobal(Amalgame_Compiler_FullResolver* self, code_string name, code_string typeName, code_bool isLet) {
    (void)self;
    (void)name;
    (void)typeName;
    (void)isLet;
    Amalgame_Compiler_Scope_Declare(self->Global, name, typeName, isLet);
}

static code_bool Amalgame_Compiler_FullResolver_DeclareCurrent(Amalgame_Compiler_FullResolver* self, code_string name, code_string typeName, code_bool isLet) {
    (void)self;
    (void)name;
    (void)typeName;
    (void)isLet;
    return Amalgame_Compiler_Scope_Declare(self->Current, name, typeName, isLet);
}

static code_bool Amalgame_Compiler_FullResolver_LookupInScopes(Amalgame_Compiler_FullResolver* self, code_string name) {
    (void)self;
    (void)name;
    Amalgame_Compiler_Scope* __attribute__((unused)) scope = self->Current;
    i64 __attribute__((unused)) safety = 0;
    while (scope != NULL) {
        safety = safety + 1;
        if (safety > 200) {
            break;
        }
        if (Amalgame_Compiler_Scope_Has(scope, name)) {
            return 1;
        }
        scope = scope->Parent;
    }
    return 0;
}

static code_string Amalgame_Compiler_FullResolver_LookupType(Amalgame_Compiler_FullResolver* self, code_string name) {
    (void)self;
    (void)name;
    Amalgame_Compiler_Scope* __attribute__((unused)) scope = self->Current;
    i64 __attribute__((unused)) safety = 0;
    while (scope != NULL) {
        safety = safety + 1;
        if (safety > 200) {
            break;
        }
        if (Amalgame_Compiler_Scope_Has(scope, name)) {
            return Amalgame_Compiler_Scope_GetType(scope, name);
        }
        scope = scope->Parent;
    }
    return "?";
}

static code_bool Amalgame_Compiler_FullResolver_LookupIsLet(Amalgame_Compiler_FullResolver* self, code_string name) {
    (void)self;
    (void)name;
    Amalgame_Compiler_Scope* __attribute__((unused)) scope = self->Current;
    i64 __attribute__((unused)) safety = 0;
    while (scope != NULL) {
        safety = safety + 1;
        if (safety > 200) {
            break;
        }
        if (Amalgame_Compiler_Scope_Has(scope, name)) {
            return Amalgame_Compiler_Scope_IsLet(scope, name);
        }
        scope = scope->Parent;
    }
    return 0;
}

static void Amalgame_Compiler_FullResolver_Error(Amalgame_Compiler_FullResolver* self, code_string msg, Amalgame_Compiler_AstNode* node) {
    (void)self;
    (void)msg;
    (void)node;
    code_string __attribute__((unused)) ln = String_FromInt(node->Line);
    code_string __attribute__((unused)) col = String_FromInt(node->Column);
    code_string __attribute__((unused)) err = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("\nerror[resolver]: ", msg), "\n  --> "), node->Name), ":"), ln), ":"), col), "\n |\n");
    AmalgameList_add(self->Errors, (void*)(intptr_t)(err));
}

static void Amalgame_Compiler_FullResolver_ErrorMsg(Amalgame_Compiler_FullResolver* self, code_string msg) {
    (void)self;
    (void)msg;
    AmalgameList_add(self->Errors, (void*)(intptr_t)(code_string_concat(code_string_concat("error[resolver]: ", msg), "\n")));
}

code_bool Amalgame_Compiler_FullResolver_HasErrors(Amalgame_Compiler_FullResolver* self) {
    (void)self;
    return AmalgameList_count(self->Errors) > 0;
}

code_string Amalgame_Compiler_FullResolver_GetErrors(Amalgame_Compiler_FullResolver* self) {
    (void)self;
    code_string __attribute__((unused)) result = "";
    i64 __attribute__((unused)) count = AmalgameList_count(self->Errors);
    for (i64 i = 0; i < count; i++) {
        result = code_string_concat(result, (code_string)AmalgameList_get(self->Errors, i));
    }
    return result;
}

void Amalgame_Compiler_FullResolver_ResolvePrograms(Amalgame_Compiler_FullResolver* self) {
    (void)self;
    i64 __attribute__((unused)) progCount = AmalgameList_count(self->Programs);
    for (i64 p = 0; p < progCount; p++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) prog = (Amalgame_Compiler_AstNode*)AmalgameList_get(self->Programs, p);
        Amalgame_Compiler_FullResolver_CollectProgram(self, prog);
    }
    for (i64 p2 = 0; p2 < progCount; p2++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) prog2 = (Amalgame_Compiler_AstNode*)AmalgameList_get(self->Programs, p2);
        Amalgame_Compiler_FullResolver_ResolveProgram(self, prog2);
    }
}

static void Amalgame_Compiler_FullResolver_CollectProgram(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* prog) {
    (void)self;
    (void)prog;
    i64 __attribute__((unused)) count = AmalgameList_count(prog->Children);
    for (i64 i = 0; i < count; i++) {
        Amalgame_Compiler_FullResolver_CollectDecl(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(prog->Children, i));
    }
}

static void Amalgame_Compiler_FullResolver_CollectDecl(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* decl) {
    (void)self;
    (void)decl;
    Amalgame_Compiler_NodeKind __attribute__((unused)) k = decl->Kind;
    if (k == Amalgame_Compiler_NodeKind_CLASS_DECL) {
        Amalgame_Compiler_FullResolver_DeclareGlobal(self, decl->Name, decl->Name, 0);
        Amalgame_Compiler_FullResolver_CollectClassMembers(self, decl);
    }
    if (k == Amalgame_Compiler_NodeKind_ENUM_DECL) {
        Amalgame_Compiler_FullResolver_DeclareGlobal(self, decl->Name, decl->Name, 0);
        Amalgame_Compiler_FullResolver_CollectEnumMembers(self, decl);
    }
}

static void Amalgame_Compiler_FullResolver_CollectClassMembers(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* cls) {
    (void)self;
    (void)cls;
    code_string __attribute__((unused)) name = cls->Name;
    AmalgameList* __attribute__((unused)) kids = cls->Children;
    i64 __attribute__((unused)) members = AmalgameList_count(kids);
    for (i64 i = 0; i < members; i++) {
        Amalgame_Compiler_FullResolver_CollectClassMember(self, name, (void*)AmalgameList_get(kids, i));
    }
}

static void Amalgame_Compiler_FullResolver_CollectClassMember(Amalgame_Compiler_FullResolver* self, code_string className, Amalgame_Compiler_AstNode* m) {
    (void)self;
    (void)className;
    (void)m;
    Amalgame_Compiler_NodeKind __attribute__((unused)) mk = m->Kind;
    if (mk == Amalgame_Compiler_NodeKind_VAR_DECL) {
        Amalgame_Compiler_MemberTable_Set(self->Members, className, m->Name, m->Str);
    }
    if (mk == Amalgame_Compiler_NodeKind_METHOD_DECL) {
        Amalgame_Compiler_MemberTable_Set(self->Members, className, m->Name, m->Str);
    }
}

static void Amalgame_Compiler_FullResolver_CollectEnumMembers(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* en) {
    (void)self;
    (void)en;
    code_string __attribute__((unused)) name = en->Name;
    AmalgameList* __attribute__((unused)) kids = en->Children;
    i64 __attribute__((unused)) count = AmalgameList_count(kids);
    for (i64 i = 0; i < count; i++) {
        Amalgame_Compiler_FullResolver_CollectEnumMember(self, name, (void*)AmalgameList_get(kids, i));
    }
}

static void Amalgame_Compiler_FullResolver_CollectEnumMember(Amalgame_Compiler_FullResolver* self, code_string enumName, Amalgame_Compiler_AstNode* m) {
    (void)self;
    (void)enumName;
    (void)m;
    Amalgame_Compiler_MemberTable_Set(self->Members, enumName, m->Name, enumName);
    code_string __attribute__((unused)) qualName = code_string_concat(code_string_concat(enumName, "_"), m->Name);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, qualName, enumName, 0);
}

static void Amalgame_Compiler_FullResolver_ResolveEnumChild(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* m) {
    (void)self;
    (void)m;
    if (m->Kind == Amalgame_Compiler_NodeKind_METHOD_DECL) {
        Amalgame_Compiler_FullResolver_ResolveMethod(self, m);
    }
}

static void Amalgame_Compiler_FullResolver_ResolveProgram(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* prog) {
    (void)self;
    (void)prog;
    i64 __attribute__((unused)) count = AmalgameList_count(prog->Children);
    for (i64 i = 0; i < count; i++) {
        Amalgame_Compiler_FullResolver_ResolveDecl(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(prog->Children, i));
    }
}

static void Amalgame_Compiler_FullResolver_ResolveDecl(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* decl) {
    (void)self;
    (void)decl;
    Amalgame_Compiler_NodeKind __attribute__((unused)) k = decl->Kind;
    if (k == Amalgame_Compiler_NodeKind_CLASS_DECL) {
        Amalgame_Compiler_FullResolver_ResolveClass(self, decl);
    }
    if (k == Amalgame_Compiler_NodeKind_ENUM_DECL) {
        AmalgameList* __attribute__((unused)) enumKids = decl->Children;
        i64 __attribute__((unused)) methods = AmalgameList_count(enumKids);
        for (i64 i = 0; i < methods; i++) {
            Amalgame_Compiler_FullResolver_ResolveEnumChild(self, (void*)AmalgameList_get(enumKids, i));
        }
    }
}

static void Amalgame_Compiler_FullResolver_ResolveClass(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* cls) {
    (void)self;
    (void)cls;
    code_string __attribute__((unused)) prevClass = self->CurrentClass;
    self->CurrentClass = cls->Name;
    Amalgame_Compiler_FullResolver_PushScope(self, code_string_concat("class:", cls->Name));
    Amalgame_Compiler_FullResolver_DeclareCurrent(self, "this", cls->Name, 0);
    AmalgameList* __attribute__((unused)) classKids = cls->Children;
    i64 __attribute__((unused)) members = AmalgameList_count(classKids);
    for (i64 i = 0; i < members; i++) {
        Amalgame_Compiler_FullResolver_PreRegisterMember(self, (void*)AmalgameList_get(classKids, i));
    }
    for (i64 j = 0; j < members; j++) {
        Amalgame_Compiler_FullResolver_ResolveMember(self, (void*)AmalgameList_get(classKids, j));
    }
    Amalgame_Compiler_FullResolver_PopScope(self);
    self->CurrentClass = prevClass;
}

static void Amalgame_Compiler_FullResolver_PreRegisterMember(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* m) {
    (void)self;
    (void)m;
    Amalgame_Compiler_NodeKind __attribute__((unused)) mk = m->Kind;
    if (mk == Amalgame_Compiler_NodeKind_VAR_DECL) {
        Amalgame_Compiler_FullResolver_DeclareCurrent(self, m->Name, m->Str, 0);
    }
    if (mk == Amalgame_Compiler_NodeKind_METHOD_DECL) {
        Amalgame_Compiler_FullResolver_DeclareCurrent(self, m->Name, m->Str, 0);
    }
}

static void Amalgame_Compiler_FullResolver_ResolveMember(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* m) {
    (void)self;
    (void)m;
    Amalgame_Compiler_NodeKind __attribute__((unused)) mk = m->Kind;
    if (mk == Amalgame_Compiler_NodeKind_METHOD_DECL) {
        Amalgame_Compiler_FullResolver_ResolveMethod(self, m);
    }
    if (mk == Amalgame_Compiler_NodeKind_VAR_DECL) {
        if (m->Left != NULL) {
            Amalgame_Compiler_FullResolver_ResolveExpr(self, m->Left);
        }
    }
}

static void Amalgame_Compiler_FullResolver_ResolveMethod(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* method) {
    (void)self;
    (void)method;
    code_string __attribute__((unused)) prevReturn = self->CurrentReturn;
    self->CurrentReturn = method->Str;
    Amalgame_Compiler_FullResolver_PushScope(self, code_string_concat("method:", method->Name));
    i64 __attribute__((unused)) params = AmalgameList_count(method->Params);
    for (i64 i = 0; i < params; i++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) p = (Amalgame_Compiler_AstNode*)AmalgameList_get(method->Params, i);
        Amalgame_Compiler_FullResolver_DeclareCurrent(self, p->Name, p->Str, 1);
    }
    if (method->Body != NULL) {
        Amalgame_Compiler_FullResolver_ResolveBlock(self, method->Body);
    }
    Amalgame_Compiler_FullResolver_PopScope(self);
    self->CurrentReturn = prevReturn;
}

static void Amalgame_Compiler_FullResolver_ResolveBlock(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* block) {
    (void)self;
    (void)block;
    Amalgame_Compiler_FullResolver_PushScope(self, "block");
    i64 __attribute__((unused)) stmts = AmalgameList_count(block->Children);
    for (i64 i = 0; i < stmts; i++) {
        Amalgame_Compiler_FullResolver_ResolveStmt(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(block->Children, i));
    }
    Amalgame_Compiler_FullResolver_PopScope(self);
}

static void Amalgame_Compiler_FullResolver_ResolveStmt(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* stmt) {
    (void)self;
    (void)stmt;
    if (stmt == NULL) {
        return;
    }
    Amalgame_Compiler_NodeKind __attribute__((unused)) k = stmt->Kind;
    if (k == Amalgame_Compiler_NodeKind_VAR_DECL) {
        if (stmt->Left != NULL) {
            Amalgame_Compiler_FullResolver_ResolveExpr(self, stmt->Left);
        }
        code_bool __attribute__((unused)) isLet = stmt->Flag == 0;
        Amalgame_Compiler_FullResolver_DeclareCurrent(self, stmt->Name, stmt->Str, isLet);
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_RETURN_STMT) {
        if (stmt->Left != NULL) {
            Amalgame_Compiler_FullResolver_ResolveExpr(self, stmt->Left);
        }
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_IF_STMT) {
        Amalgame_Compiler_FullResolver_ResolveIf(self, stmt);
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_WHILE_STMT) {
        if (stmt->Cond != NULL) {
            Amalgame_Compiler_FullResolver_ResolveExpr(self, stmt->Cond);
        }
        self->LoopDepth = self->LoopDepth + 1;
        if (stmt->Body != NULL) {
            Amalgame_Compiler_FullResolver_ResolveBlock(self, stmt->Body);
        }
        self->LoopDepth = self->LoopDepth - 1;
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_FOR_IN_STMT) {
        Amalgame_Compiler_FullResolver_ResolveForIn(self, stmt);
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_BREAK_STMT) {
        if (self->LoopDepth == 0) {
            Amalgame_Compiler_FullResolver_ErrorMsg(self, "'break' outside loop");
        }
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_CONTINUE_STMT) {
        if (self->LoopDepth == 0) {
            Amalgame_Compiler_FullResolver_ErrorMsg(self, "'continue' outside loop");
        }
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_ASSIGN) {
        Amalgame_Compiler_FullResolver_ResolveAssign(self, stmt);
        return;
    }
    if (code_string_equals(stmt->Name, "__match__")) {
        Amalgame_Compiler_FullResolver_ResolveMatch(self, stmt);
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_BLOCK) {
        Amalgame_Compiler_FullResolver_ResolveBlock(self, stmt);
        return;
    }
    Amalgame_Compiler_FullResolver_ResolveExpr(self, stmt);
}

static void Amalgame_Compiler_FullResolver_ResolveIf(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* stmt) {
    (void)self;
    (void)stmt;
    if (stmt->Cond != NULL) {
        Amalgame_Compiler_FullResolver_ResolveExpr(self, stmt->Cond);
    }
    if (stmt->Body != NULL) {
        Amalgame_Compiler_FullResolver_ResolveBlock(self, stmt->Body);
    }
    if (stmt->Else != NULL) {
        Amalgame_Compiler_FullResolver_ResolveElseBranch(self, stmt->Else);
    }
}

static void Amalgame_Compiler_FullResolver_ResolveElseBranch(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* branch) {
    (void)self;
    (void)branch;
    if (branch == NULL) {
        return;
    }
    Amalgame_Compiler_NodeKind __attribute__((unused)) bk = branch->Kind;
    if (bk == Amalgame_Compiler_NodeKind_IF_STMT) {
        Amalgame_Compiler_FullResolver_ResolveIf(self, branch);
    } else {
        Amalgame_Compiler_FullResolver_ResolveBlock(self, branch);
    }
}

static void Amalgame_Compiler_FullResolver_ResolveForIn(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* stmt) {
    (void)self;
    (void)stmt;
    if (stmt->Left != NULL) {
        Amalgame_Compiler_FullResolver_ResolveExpr(self, stmt->Left);
    }
    Amalgame_Compiler_FullResolver_PushScope(self, "for-in");
    code_string __attribute__((unused)) elemType = "?";
    if (stmt->Left != NULL) {
        code_string __attribute__((unused)) colType = Amalgame_Compiler_FullResolver_InferExprType(self, stmt->Left);
        elemType = Amalgame_Compiler_FullResolver_CollectionElemType(self, colType);
    }
    Amalgame_Compiler_FullResolver_DeclareCurrent(self, stmt->Name, elemType, 1);
    self->LoopDepth = self->LoopDepth + 1;
    if (stmt->Body != NULL) {
        Amalgame_Compiler_FullResolver_ResolveBlock(self, stmt->Body);
    }
    self->LoopDepth = self->LoopDepth - 1;
    Amalgame_Compiler_FullResolver_PopScope(self);
}

static void Amalgame_Compiler_FullResolver_ResolveAssign(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* stmt) {
    (void)self;
    (void)stmt;
    if (stmt->Left != NULL) {
        Amalgame_Compiler_FullResolver_ResolveExpr(self, stmt->Left);
    }
    if (stmt->Right != NULL) {
        Amalgame_Compiler_FullResolver_ResolveExpr(self, stmt->Right);
    }
    if (stmt->Left != NULL && stmt->Left->Kind == Amalgame_Compiler_NodeKind_IDENTIFIER) {
        code_string __attribute__((unused)) varName = stmt->Left->Name;
        if (Amalgame_Compiler_FullResolver_LookupIsLet(self, varName)) {
            Amalgame_Compiler_FullResolver_ErrorMsg(self, code_string_concat(code_string_concat("Cannot assign to immutable binding '", varName), "'"));
        }
    }
}

static void Amalgame_Compiler_FullResolver_ResolveMatch(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* stmt) {
    (void)self;
    (void)stmt;
    if (stmt->Left != NULL) {
        Amalgame_Compiler_FullResolver_ResolveExpr(self, stmt->Left);
    }
    i64 __attribute__((unused)) armCount = AmalgameList_count(stmt->Children);
    for (i64 i = 0; i < armCount; i++) {
        Amalgame_Compiler_FullResolver_ResolveMatchArm(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(stmt->Children, i));
    }
}

static void Amalgame_Compiler_FullResolver_ResolveMatchArm(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* arm) {
    (void)self;
    (void)arm;
    if (arm == NULL) {
        return;
    }
    Amalgame_Compiler_FullResolver_PushScope(self, "match-arm");
    if (arm->Left != NULL) {
        Amalgame_Compiler_FullResolver_ResolveExpr(self, arm->Left);
    }
    Amalgame_Compiler_AstNode* __attribute__((unused)) armBody = arm->Right;
    if (armBody != NULL) {
        Amalgame_Compiler_FullResolver_ResolveArmBody(self, armBody);
    }
    Amalgame_Compiler_FullResolver_PopScope(self);
}

static void Amalgame_Compiler_FullResolver_ResolveArmBody(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* body) {
    (void)self;
    (void)body;
    Amalgame_Compiler_NodeKind __attribute__((unused)) bk = body->Kind;
    if (bk == Amalgame_Compiler_NodeKind_BLOCK) {
        AmalgameList* __attribute__((unused)) kids = body->Children;
        i64 __attribute__((unused)) count = AmalgameList_count(kids);
        for (i64 i = 0; i < count; i++) {
            Amalgame_Compiler_FullResolver_ResolveStmt(self, (void*)AmalgameList_get(kids, i));
        }
    } else {
        Amalgame_Compiler_FullResolver_ResolveStmt(self, body);
    }
}

static void Amalgame_Compiler_FullResolver_ResolveExpr(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* expr) {
    (void)self;
    (void)expr;
    if (expr == NULL) {
        return;
    }
    Amalgame_Compiler_NodeKind __attribute__((unused)) k = expr->Kind;
    if (k == Amalgame_Compiler_NodeKind_IDENTIFIER) {
        if (!Amalgame_Compiler_FullResolver_LookupInScopes(self, expr->Name)) {
            Amalgame_Compiler_FullResolver_ErrorMsg(self, code_string_concat(code_string_concat("Unknown symbol '", expr->Name), "'"));
        }
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_BINARY) {
        if (expr->Left != NULL) {
            Amalgame_Compiler_FullResolver_ResolveExpr(self, expr->Left);
        }
        if (expr->Right != NULL) {
            Amalgame_Compiler_FullResolver_ResolveExpr(self, expr->Right);
        }
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_UNARY) {
        if (expr->Left != NULL) {
            Amalgame_Compiler_FullResolver_ResolveExpr(self, expr->Left);
        }
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_MEMBER) {
        if (expr->Left != NULL) {
            Amalgame_Compiler_FullResolver_ResolveExpr(self, expr->Left);
        }
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_CALL) {
        if (expr->Left != NULL) {
            Amalgame_Compiler_FullResolver_ResolveExpr(self, expr->Left);
        }
        i64 __attribute__((unused)) argc = AmalgameList_count(expr->Args);
        for (i64 i = 0; i < argc; i++) {
            Amalgame_Compiler_FullResolver_ResolveExpr(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(expr->Args, i));
        }
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_NEW_EXPR) {
        i64 __attribute__((unused)) argc = AmalgameList_count(expr->Args);
        for (i64 i = 0; i < argc; i++) {
            Amalgame_Compiler_FullResolver_ResolveExpr(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(expr->Args, i));
        }
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_INDEX_EXPR) {
        if (expr->Left != NULL) {
            Amalgame_Compiler_FullResolver_ResolveExpr(self, expr->Left);
        }
        if (expr->Right != NULL) {
            Amalgame_Compiler_FullResolver_ResolveExpr(self, expr->Right);
        }
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_ASSIGN) {
        Amalgame_Compiler_FullResolver_ResolveAssign(self, expr);
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_IF_STMT) {
        if (expr->Cond != NULL) {
            Amalgame_Compiler_FullResolver_ResolveExpr(self, expr->Cond);
        }
        if (expr->Body != NULL) {
            Amalgame_Compiler_FullResolver_ResolveBlock(self, expr->Body);
        }
        if (expr->Else != NULL) {
            Amalgame_Compiler_FullResolver_ResolveElseBranch(self, expr->Else);
        }
        return;
    }
}

static code_string Amalgame_Compiler_FullResolver_InferExprType(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* expr) {
    (void)self;
    (void)expr;
    if (expr == NULL) {
        return "?";
    }
    Amalgame_Compiler_NodeKind __attribute__((unused)) k = expr->Kind;
    if (k == Amalgame_Compiler_NodeKind_IDENTIFIER) {
        return Amalgame_Compiler_FullResolver_LookupType(self, expr->Name);
    }
    if (k == Amalgame_Compiler_NodeKind_LITERAL_INT) {
        return "int";
    }
    if (k == Amalgame_Compiler_NodeKind_LITERAL_FLOAT) {
        return "float";
    }
    if (k == Amalgame_Compiler_NodeKind_LITERAL_STRING) {
        return "string";
    }
    if (k == Amalgame_Compiler_NodeKind_LITERAL_BOOL) {
        return "bool";
    }
    if (k == Amalgame_Compiler_NodeKind_BINARY && code_string_equals(expr->Str, "..")) {
        return "range";
    }
    if (k == Amalgame_Compiler_NodeKind_NEW_EXPR) {
        return expr->Name;
    }
    if (k == Amalgame_Compiler_NodeKind_MEMBER) {
        if (expr->Left != NULL) {
            code_string __attribute__((unused)) targetType = Amalgame_Compiler_FullResolver_InferExprType(self, expr->Left);
            return Amalgame_Compiler_MemberTable_Get(self->Members, targetType, expr->Name);
        }
    }
    return "?";
}

static code_string Amalgame_Compiler_FullResolver_CollectionElemType(Amalgame_Compiler_FullResolver* self, code_string typeKey) {
    (void)self;
    (void)typeKey;
    if (code_string_equals(typeKey, "range") || code_string_equals(typeKey, "int")) {
        return "int";
    }
    if (String_StartsWith(typeKey, "List<") && String_EndsWith(typeKey, ">")) {
        return String_Substring(typeKey, 5, String_Length(typeKey) - 6);
    }
    if (String_StartsWith(typeKey, "Set<") && String_EndsWith(typeKey, ">")) {
        return String_Substring(typeKey, 4, String_Length(typeKey) - 5);
    }
    if (String_EndsWith(typeKey, "[]")) {
        return String_Substring(typeKey, 0, String_Length(typeKey) - 2);
    }
    return "?";
}

code_string Amalgame_Compiler_FullResolver_GetMemberType(Amalgame_Compiler_FullResolver* self, code_string className, code_string memberName) {
    (void)self;
    (void)className;
    (void)memberName;
    return Amalgame_Compiler_MemberTable_Get(self->Members, className, memberName);
}

code_bool Amalgame_Compiler_FullResolver_HasMember(Amalgame_Compiler_FullResolver* self, code_string className, code_string memberName) {
    (void)self;
    (void)className;
    (void)memberName;
    return Amalgame_Compiler_MemberTable_Has(self->Members, className, memberName);
}

code_string Amalgame_Compiler_FullResolver_GetVarType(Amalgame_Compiler_FullResolver* self, code_string name) {
    (void)self;
    (void)name;
    return Amalgame_Compiler_FullResolver_LookupType(self, name);
}

code_bool Amalgame_Compiler_FullResolver_HasSymbol(Amalgame_Compiler_FullResolver* self, code_string name) {
    (void)self;
    (void)name;
    return Amalgame_Compiler_FullResolver_LookupInScopes(self, name);
}

code_string Amalgame_Compiler_FullResolver_GetTypeName(Amalgame_Compiler_FullResolver* self, code_string name) {
    (void)self;
    (void)name;
    return Amalgame_Compiler_FullResolver_LookupType(self, name);
}

void Amalgame_Compiler_FullResolver_SetTypeName(Amalgame_Compiler_FullResolver* self, code_string name, code_string typeName) {
    (void)self;
    (void)name;
    (void)typeName;
    Amalgame_Compiler_Scope* __attribute__((unused)) scope = self->Current;
    i64 __attribute__((unused)) safety = 0;
    while (scope != NULL) {
        safety = safety + 1;
        if (safety > 200) {
            break;
        }
        if (Amalgame_Compiler_Scope_Has(scope, name)) {
            Amalgame_Compiler_Scope_Declare(scope, name, typeName, Amalgame_Compiler_Scope_IsLet(scope, name));
            return;
        }
        scope = scope->Parent;
    }
}

struct _Amalgame_Compiler_DiagnosticFormatter {
    code_bool UseColor;
};

void Amalgame_Compiler_DiagnosticFormatter_EnableColor(Amalgame_Compiler_DiagnosticFormatter* self, code_bool v);
static code_string Amalgame_Compiler_DiagnosticFormatter_Colored(Amalgame_Compiler_DiagnosticFormatter* self, code_string code, code_string s);
code_string Amalgame_Compiler_DiagnosticFormatter_FormatError(Amalgame_Compiler_DiagnosticFormatter* self, code_string kind, code_string message, code_string filename, i64 line, i64 col);
code_string Amalgame_Compiler_DiagnosticFormatter_FormatWarning(Amalgame_Compiler_DiagnosticFormatter* self, code_string kind, code_string message, code_string filename, i64 line, i64 col);
void Amalgame_Compiler_DiagnosticFormatter_PrintCompileOk(Amalgame_Compiler_DiagnosticFormatter* self, code_string phase);
void Amalgame_Compiler_DiagnosticFormatter_PrintCompileError(Amalgame_Compiler_DiagnosticFormatter* self, code_string phase, code_string detail);

Amalgame_Compiler_DiagnosticFormatter* Amalgame_Compiler_DiagnosticFormatter_new() {
    Amalgame_Compiler_DiagnosticFormatter* self = (Amalgame_Compiler_DiagnosticFormatter*) GC_MALLOC(sizeof(Amalgame_Compiler_DiagnosticFormatter));
    self->UseColor = 0;
    return self;
}

void Amalgame_Compiler_DiagnosticFormatter_EnableColor(Amalgame_Compiler_DiagnosticFormatter* self, code_bool v) {
    (void)self;
    (void)v;
    self->UseColor = v;
}

static code_string Amalgame_Compiler_DiagnosticFormatter_Colored(Amalgame_Compiler_DiagnosticFormatter* self, code_string code, code_string s) {
    (void)self;
    (void)code;
    (void)s;
    if (!self->UseColor) {
        return s;
    }
    return code_string_concat(code_string_concat(code_string_concat(code_string_concat("1b[", code), "m"), s), "1b[0m");
}

code_string Amalgame_Compiler_DiagnosticFormatter_FormatError(Amalgame_Compiler_DiagnosticFormatter* self, code_string kind, code_string message, code_string filename, i64 line, i64 col) {
    (void)self;
    (void)kind;
    (void)message;
    (void)filename;
    (void)line;
    (void)col;
    code_string __attribute__((unused)) lineStr = String_FromInt(line);
    code_string __attribute__((unused)) colStr = String_FromInt(col);
    code_string __attribute__((unused)) result = code_string_concat(code_string_concat(code_string_concat(code_string_concat("error[", kind), "]: "), message), "\n");
    result = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(result, "  --> "), filename), ":"), lineStr), ":"), colStr), "\n");
    result = code_string_concat(result, " |\n");
    return result;
}

code_string Amalgame_Compiler_DiagnosticFormatter_FormatWarning(Amalgame_Compiler_DiagnosticFormatter* self, code_string kind, code_string message, code_string filename, i64 line, i64 col) {
    (void)self;
    (void)kind;
    (void)message;
    (void)filename;
    (void)line;
    (void)col;
    code_string __attribute__((unused)) lineStr = String_FromInt(line);
    code_string __attribute__((unused)) colStr = String_FromInt(col);
    code_string __attribute__((unused)) result = code_string_concat(code_string_concat(code_string_concat(code_string_concat("warning[", kind), "]: "), message), "\n");
    result = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(result, "  --> "), filename), ":"), lineStr), ":"), colStr), "\n");
    result = code_string_concat(result, " |\n");
    return result;
}

void Amalgame_Compiler_DiagnosticFormatter_PrintCompileOk(Amalgame_Compiler_DiagnosticFormatter* self, code_string phase) {
    (void)self;
    (void)phase;
    Console_WriteLine(code_string_concat(phase, " OK"));
}

void Amalgame_Compiler_DiagnosticFormatter_PrintCompileError(Amalgame_Compiler_DiagnosticFormatter* self, code_string phase, code_string detail) {
    (void)self;
    (void)phase;
    (void)detail;
    Console_WriteError(code_string_concat(phase, " ERROR"));
    Console_WriteError(detail);
}

struct _Amalgame_Compiler_TypeError {
    code_string Message;
    code_string Filename;
    i64 Line;
    i64 Column;
};

code_string Amalgame_Compiler_TypeError_ToString(Amalgame_Compiler_TypeError* self);

Amalgame_Compiler_TypeError* Amalgame_Compiler_TypeError_new(code_string msg, code_string file, i64 line, i64 col) {
    Amalgame_Compiler_TypeError* self = (Amalgame_Compiler_TypeError*) GC_MALLOC(sizeof(Amalgame_Compiler_TypeError));
    self->Message = msg;
    self->Filename = file;
    self->Line = line;
    self->Column = col;
    return self;
}

code_string Amalgame_Compiler_TypeError_ToString(Amalgame_Compiler_TypeError* self) {
    (void)self;
    code_string __attribute__((unused)) ln = String_FromInt(self->Line);
    code_string __attribute__((unused)) col = String_FromInt(self->Column);
    return code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("\n┌── [typechecker] ", self->Filename), ":"), ln), ":"), col), "\n│\n│  "), self->Message), "\n│\n└──\n");
}

struct _Amalgame_Compiler_TypeCheckResult {
    code_bool Success;
    AmalgameList* Errors;
};


Amalgame_Compiler_TypeCheckResult* Amalgame_Compiler_TypeCheckResult_new() {
    Amalgame_Compiler_TypeCheckResult* self = (Amalgame_Compiler_TypeCheckResult*) GC_MALLOC(sizeof(Amalgame_Compiler_TypeCheckResult));
    self->Success = 1;
    self->Errors = AmalgameList_new();
    return self;
}

struct _Amalgame_Compiler_TypeChecker {
    AmalgameList* ExprTypeKeys;
    AmalgameList* ExprTypeVals;
    code_string CurrentReturn;
    code_string CurrentClass;
    AmalgameList* Errors;
    code_string Filename;
    Amalgame_Compiler_FullResolver* Symbols;
};

Amalgame_Compiler_TypeCheckResult* Amalgame_Compiler_TypeChecker_Check(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* program);
static code_string Amalgame_Compiler_TypeChecker_NodeKey(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* node);
static void Amalgame_Compiler_TypeChecker_SetType(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* node, code_string typeKey);
static code_string Amalgame_Compiler_TypeChecker_GetType(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* node);
static code_bool Amalgame_Compiler_TypeChecker_IsAssignable(Amalgame_Compiler_TypeChecker* self, code_string expected, code_string actual);
static code_bool Amalgame_Compiler_TypeChecker_IsNumericWiden(Amalgame_Compiler_TypeChecker* self, code_string to, code_string from);
static code_bool Amalgame_Compiler_TypeChecker_IsBool(Amalgame_Compiler_TypeChecker* self, code_string t);
static code_bool Amalgame_Compiler_TypeChecker_IsNumeric(Amalgame_Compiler_TypeChecker* self, code_string t);
static code_bool Amalgame_Compiler_TypeChecker_IsNullable(Amalgame_Compiler_TypeChecker* self, code_string t);
static code_string Amalgame_Compiler_TypeChecker_BinaryResultType(Amalgame_Compiler_TypeChecker* self, code_string op, code_string left, code_string right);
static code_string Amalgame_Compiler_TypeChecker_CollectionElementType(Amalgame_Compiler_TypeChecker* self, code_string typeKey);
static code_bool Amalgame_Compiler_TypeChecker_SymbolFound(Amalgame_Compiler_TypeChecker* self, code_string name);
static code_string Amalgame_Compiler_TypeChecker_SymbolTypeName(Amalgame_Compiler_TypeChecker* self, code_string name);
static void Amalgame_Compiler_TypeChecker_SymbolSetType(Amalgame_Compiler_TypeChecker* self, code_string name, code_string typeName);
static void Amalgame_Compiler_TypeChecker_Error(Amalgame_Compiler_TypeChecker* self, code_string msg, Amalgame_Compiler_AstNode* node);
static void Amalgame_Compiler_TypeChecker_CheckBool(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* node, code_string context);
static code_string Amalgame_Compiler_TypeChecker_SymbolType(Amalgame_Compiler_TypeChecker* self, code_string name);
static code_string Amalgame_Compiler_TypeChecker_MemberTypeOf(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* classDecl, code_string memberName);
static void Amalgame_Compiler_TypeChecker_CheckProgram(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* prog);
static void Amalgame_Compiler_TypeChecker_CheckDecl(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* decl);
static void Amalgame_Compiler_TypeChecker_CheckClass(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* cls);
static void Amalgame_Compiler_TypeChecker_CheckFieldDecl(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* field);
static void Amalgame_Compiler_TypeChecker_CheckMethod(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* method);
static void Amalgame_Compiler_TypeChecker_CheckBlock(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* block);
static void Amalgame_Compiler_TypeChecker_CheckStmt(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* stmt);
static void Amalgame_Compiler_TypeChecker_CheckVarDecl(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* stmt);
static void Amalgame_Compiler_TypeChecker_CheckReturn(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* stmt);
static void Amalgame_Compiler_TypeChecker_CheckIf(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* stmt);
static void Amalgame_Compiler_TypeChecker_CheckElseBranch(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* branch);
static void Amalgame_Compiler_TypeChecker_CheckForIn(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* stmt);
static void Amalgame_Compiler_TypeChecker_CheckAssign(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* stmt);
static void Amalgame_Compiler_TypeChecker_CheckMatch(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* stmt);
static void Amalgame_Compiler_TypeChecker_CheckMatchArm(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* arm);
static void Amalgame_Compiler_TypeChecker_CheckExpr(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* expr);
static code_string Amalgame_Compiler_TypeChecker_CheckIfBranch(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* block);
static void Amalgame_Compiler_TypeChecker_CheckBinaryExpr(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* expr);
static void Amalgame_Compiler_TypeChecker_CheckUnaryExpr(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* expr);
static void Amalgame_Compiler_TypeChecker_CheckMemberExpr(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* expr);
static void Amalgame_Compiler_TypeChecker_CheckCallExpr(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* expr);
static void Amalgame_Compiler_TypeChecker_CheckNewExpr(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* expr);
static void Amalgame_Compiler_TypeChecker_CheckIndexExpr(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* expr);
code_bool Amalgame_Compiler_TypeChecker_HasErrors(Amalgame_Compiler_TypeChecker* self);
code_string Amalgame_Compiler_TypeChecker_FormatErrors(Amalgame_Compiler_TypeChecker* self);

Amalgame_Compiler_TypeChecker* Amalgame_Compiler_TypeChecker_new(Amalgame_Compiler_FullResolver* symbols, code_string filename) {
    Amalgame_Compiler_TypeChecker* self = (Amalgame_Compiler_TypeChecker*) GC_MALLOC(sizeof(Amalgame_Compiler_TypeChecker));
    self->ExprTypeKeys = AmalgameList_new();
    self->ExprTypeVals = AmalgameList_new();
    self->CurrentReturn = "void";
    self->CurrentClass = "";
    self->Errors = AmalgameList_new();
    self->Filename = filename;
    self->Symbols = symbols;
    return self;
}

Amalgame_Compiler_TypeCheckResult* Amalgame_Compiler_TypeChecker_Check(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* program) {
    (void)self;
    (void)program;
    Amalgame_Compiler_TypeChecker_CheckProgram(self, program);
    Amalgame_Compiler_TypeCheckResult* __attribute__((unused)) result = Amalgame_Compiler_TypeCheckResult_new();
    i64 __attribute__((unused)) ec = AmalgameList_count(self->Errors);
    result->Success = ec == 0;
    for (i64 i = 0; i < ec; i++) {
        AmalgameList_add(result->Errors, (void*)(intptr_t)((Amalgame_Compiler_TypeError*)AmalgameList_get(self->Errors, i)));
    }
    return result;
}

static code_string Amalgame_Compiler_TypeChecker_NodeKey(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* node) {
    (void)self;
    (void)node;
    code_string __attribute__((unused)) ln = String_FromInt(node->Line);
    code_string __attribute__((unused)) col = String_FromInt(node->Column);
    return code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(ln, ":"), col), ":"), node->Name), ":"), node->Str);
}

static void Amalgame_Compiler_TypeChecker_SetType(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* node, code_string typeKey) {
    (void)self;
    (void)node;
    (void)typeKey;
    code_string __attribute__((unused)) key = Amalgame_Compiler_TypeChecker_NodeKey(self, node);
    i64 __attribute__((unused)) count = AmalgameList_count(self->ExprTypeKeys);
    for (i64 i = 0; i < count; i++) {
        if (code_string_equals((code_string)AmalgameList_get(self->ExprTypeKeys, i), key)) {
            AmalgameList_add(self->ExprTypeVals, (void*)(intptr_t)(typeKey));
            return;
        }
    }
    AmalgameList_add(self->ExprTypeKeys, (void*)(intptr_t)(key));
    AmalgameList_add(self->ExprTypeVals, (void*)(intptr_t)(typeKey));
}

static code_string Amalgame_Compiler_TypeChecker_GetType(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* node) {
    (void)self;
    (void)node;
    code_string __attribute__((unused)) key = Amalgame_Compiler_TypeChecker_NodeKey(self, node);
    i64 __attribute__((unused)) count = AmalgameList_count(self->ExprTypeKeys);
    code_string __attribute__((unused)) result = "?";
    for (i64 i = 0; i < count; i++) {
        if (code_string_equals((code_string)AmalgameList_get(self->ExprTypeKeys, i), key)) {
            result = (code_string)AmalgameList_get(self->ExprTypeVals, i);
        }
    }
    return result;
}

static code_bool Amalgame_Compiler_TypeChecker_IsAssignable(Amalgame_Compiler_TypeChecker* self, code_string expected, code_string actual) {
    (void)self;
    (void)expected;
    (void)actual;
    if (code_string_equals(expected, "?") || code_string_equals(actual, "?")) {
        return 1;
    }
    if (code_string_equals(expected, actual)) {
        return 1;
    }
    if (code_string_equals(expected, "object")) {
        return 1;
    }
    code_string __attribute__((unused)) eBase = expected;
    code_string __attribute__((unused)) aBase = actual;
    if (String_EndsWith(expected, "?")) {
        eBase = String_Substring(expected, 0, String_Length(expected) - 1);
    }
    if (String_EndsWith(actual, "?")) {
        aBase = String_Substring(actual, 0, String_Length(actual) - 1);
    }
    if (code_string_equals(eBase, aBase)) {
        return 1;
    }
    if (code_string_equals(actual, "null") && String_EndsWith(expected, "?")) {
        return 1;
    }
    if (Amalgame_Compiler_TypeChecker_IsNumericWiden(self, eBase, aBase)) {
        return 1;
    }
    return 0;
}

static code_bool Amalgame_Compiler_TypeChecker_IsNumericWiden(Amalgame_Compiler_TypeChecker* self, code_string to, code_string from) {
    (void)self;
    (void)to;
    (void)from;
    if (code_string_equals(to, "double")) {
        if (code_string_equals(from, "float") || code_string_equals(from, "int") || code_string_equals(from, "i64") || code_string_equals(from, "i32") || code_string_equals(from, "f32")) {
            return 1;
        }
    }
    if (code_string_equals(to, "float")) {
        if (code_string_equals(from, "int") || code_string_equals(from, "i32") || code_string_equals(from, "f32")) {
            return 1;
        }
    }
    return 0;
}

static code_bool Amalgame_Compiler_TypeChecker_IsBool(Amalgame_Compiler_TypeChecker* self, code_string t) {
    (void)self;
    (void)t;
    return code_string_equals(t, "bool") || code_string_equals(t, "?");
}

static code_bool Amalgame_Compiler_TypeChecker_IsNumeric(Amalgame_Compiler_TypeChecker* self, code_string t) {
    (void)self;
    (void)t;
    if (code_string_equals(t, "int")) {
        return 1;
    }
    if (code_string_equals(t, "float")) {
        return 1;
    }
    if (code_string_equals(t, "double")) {
        return 1;
    }
    if (code_string_equals(t, "i8")) {
        return 1;
    }
    if (code_string_equals(t, "i16")) {
        return 1;
    }
    if (code_string_equals(t, "i32")) {
        return 1;
    }
    if (code_string_equals(t, "i64")) {
        return 1;
    }
    if (code_string_equals(t, "u8")) {
        return 1;
    }
    if (code_string_equals(t, "u16")) {
        return 1;
    }
    if (code_string_equals(t, "u32")) {
        return 1;
    }
    if (code_string_equals(t, "u64")) {
        return 1;
    }
    if (code_string_equals(t, "f32")) {
        return 1;
    }
    if (code_string_equals(t, "f64")) {
        return 1;
    }
    return 0;
}

static code_bool Amalgame_Compiler_TypeChecker_IsNullable(Amalgame_Compiler_TypeChecker* self, code_string t) {
    (void)self;
    (void)t;
    return String_EndsWith(t, "?");
}

static code_string Amalgame_Compiler_TypeChecker_BinaryResultType(Amalgame_Compiler_TypeChecker* self, code_string op, code_string left, code_string right) {
    (void)self;
    (void)op;
    (void)left;
    (void)right;
    if (code_string_equals(op, "==") || code_string_equals(op, "!=") || code_string_equals(op, "<") || code_string_equals(op, ">") || code_string_equals(op, "<=") || code_string_equals(op, ">=")) {
        return "bool";
    }
    if (code_string_equals(op, "&&") || code_string_equals(op, "||")) {
        return "bool";
    }
    if (code_string_equals(op, "??")) {
        if (String_EndsWith(left, "?")) {
            return String_Substring(left, 0, String_Length(left) - 1);
        }
        return left;
    }
    if (code_string_equals(op, "..") || code_string_equals(op, "...")) {
        return "range";
    }
    if (code_string_equals(op, "|>")) {
        return right;
    }
    if (code_string_equals(op, "+") || code_string_equals(op, "-") || code_string_equals(op, "*") || code_string_equals(op, "/") || code_string_equals(op, "%") || code_string_equals(op, "^")) {
        if (code_string_equals(left, "double") || code_string_equals(right, "double")) {
            return "double";
        }
        if (code_string_equals(left, "float") || code_string_equals(right, "float")) {
            return "float";
        }
        if (code_string_equals(left, "string") || code_string_equals(right, "string")) {
            return "string";
        }
        return left;
    }
    return "?";
}

static code_string Amalgame_Compiler_TypeChecker_CollectionElementType(Amalgame_Compiler_TypeChecker* self, code_string typeKey) {
    (void)self;
    (void)typeKey;
    if (code_string_equals(typeKey, "?") || code_string_equals(typeKey, "")) {
        return "?";
    }
    if (String_StartsWith(typeKey, "List<") && String_EndsWith(typeKey, ">")) {
        code_string __attribute__((unused)) inner = String_Substring(typeKey, 5, String_Length(typeKey) - 6);
        return inner;
    }
    if (String_StartsWith(typeKey, "Set<") && String_EndsWith(typeKey, ">")) {
        code_string __attribute__((unused)) inner = String_Substring(typeKey, 4, String_Length(typeKey) - 5);
        return inner;
    }
    if (String_StartsWith(typeKey, "Map<") && String_EndsWith(typeKey, ">")) {
        code_string __attribute__((unused)) inner = String_Substring(typeKey, 4, String_Length(typeKey) - 5);
        i64 __attribute__((unused)) comma = String_IndexOf(inner, ",");
        if (comma >= 0) {
            code_string __attribute__((unused)) vt = String_Substring(inner, comma + 1, String_Length(inner) - comma - 1);
            return String_Trim(vt);
        }
    }
    if (String_EndsWith(typeKey, "[]")) {
        return String_Substring(typeKey, 0, String_Length(typeKey) - 2);
    }
    return "?";
}

static code_bool Amalgame_Compiler_TypeChecker_SymbolFound(Amalgame_Compiler_TypeChecker* self, code_string name) {
    (void)self;
    (void)name;
    return Amalgame_Compiler_FullResolver_HasSymbol(self->Symbols, name);
}

static code_string Amalgame_Compiler_TypeChecker_SymbolTypeName(Amalgame_Compiler_TypeChecker* self, code_string name) {
    (void)self;
    (void)name;
    return Amalgame_Compiler_FullResolver_GetTypeName(self->Symbols, name);
}

static void Amalgame_Compiler_TypeChecker_SymbolSetType(Amalgame_Compiler_TypeChecker* self, code_string name, code_string typeName) {
    (void)self;
    (void)name;
    (void)typeName;
    Amalgame_Compiler_FullResolver_SetTypeName(self->Symbols, name, typeName);
}

static void Amalgame_Compiler_TypeChecker_Error(Amalgame_Compiler_TypeChecker* self, code_string msg, Amalgame_Compiler_AstNode* node) {
    (void)self;
    (void)msg;
    (void)node;
    code_string __attribute__((unused)) file = self->Filename;
    i64 __attribute__((unused)) line = 0;
    i64 __attribute__((unused)) col = 0;
    if (node != NULL) {
        line = node->Line;
        col = node->Column;
    }
    Amalgame_Compiler_TypeError* __attribute__((unused)) err = Amalgame_Compiler_TypeError_new(msg, file, line, col);
    AmalgameList_add(self->Errors, (void*)(intptr_t)(err));
}

static void Amalgame_Compiler_TypeChecker_CheckBool(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* node, code_string context) {
    (void)self;
    (void)node;
    (void)context;
    code_string __attribute__((unused)) t = Amalgame_Compiler_TypeChecker_GetType(self, node);
    if (!Amalgame_Compiler_TypeChecker_IsBool(self, t)) {
        Amalgame_Compiler_TypeChecker_Error(self, code_string_concat(code_string_concat(code_string_concat(code_string_concat("'", context), "' must be bool, got '"), t), "'"), node);
    }
}

static code_string Amalgame_Compiler_TypeChecker_SymbolType(Amalgame_Compiler_TypeChecker* self, code_string name) {
    (void)self;
    (void)name;
    return Amalgame_Compiler_TypeChecker_SymbolTypeName(self, name);
}

static code_string Amalgame_Compiler_TypeChecker_MemberTypeOf(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* classDecl, code_string memberName) {
    (void)self;
    (void)classDecl;
    (void)memberName;
    if (classDecl == NULL) {
        return "?";
    }
    Amalgame_Compiler_NodeKind __attribute__((unused)) k = classDecl->Kind;
    if (k == Amalgame_Compiler_NodeKind_ENUM_DECL) {
        i64 __attribute__((unused)) members = AmalgameList_count(classDecl->Children);
        for (i64 i = 0; i < members; i++) {
            Amalgame_Compiler_AstNode* __attribute__((unused)) m = (Amalgame_Compiler_AstNode*)AmalgameList_get(classDecl->Children, i);
            if (code_string_equals(m->Name, memberName)) {
                return classDecl->Name;
            }
        }
        return classDecl->Name;
    }
    if (k == Amalgame_Compiler_NodeKind_CLASS_DECL) {
        i64 __attribute__((unused)) members = AmalgameList_count(classDecl->Children);
        for (i64 i = 0; i < members; i++) {
            Amalgame_Compiler_AstNode* __attribute__((unused)) m = (Amalgame_Compiler_AstNode*)AmalgameList_get(classDecl->Children, i);
            Amalgame_Compiler_NodeKind __attribute__((unused)) mk = m->Kind;
            if (code_string_equals(m->Name, memberName)) {
                if (mk == Amalgame_Compiler_NodeKind_VAR_DECL) {
                    return m->Str;
                }
                if (mk == Amalgame_Compiler_NodeKind_METHOD_DECL) {
                    return m->Str;
                }
            }
        }
    }
    return "?";
}

static void Amalgame_Compiler_TypeChecker_CheckProgram(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* prog) {
    (void)self;
    (void)prog;
    i64 __attribute__((unused)) count = AmalgameList_count(prog->Children);
    for (i64 i = 0; i < count; i++) {
        Amalgame_Compiler_TypeChecker_CheckDecl(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(prog->Children, i));
    }
}

static void Amalgame_Compiler_TypeChecker_CheckDecl(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* decl) {
    (void)self;
    (void)decl;
    Amalgame_Compiler_NodeKind __attribute__((unused)) k = decl->Kind;
    if (k == Amalgame_Compiler_NodeKind_CLASS_DECL) {
        Amalgame_Compiler_TypeChecker_CheckClass(self, decl);
    }
    if (k == Amalgame_Compiler_NodeKind_ENUM_DECL) {
        i64 __attribute__((unused)) methods = AmalgameList_count(decl->Children);
        for (i64 i = 0; i < methods; i++) {
            Amalgame_Compiler_AstNode* __attribute__((unused)) m = (Amalgame_Compiler_AstNode*)AmalgameList_get(decl->Children, i);
            if (m->Kind == Amalgame_Compiler_NodeKind_METHOD_DECL) {
                Amalgame_Compiler_TypeChecker_CheckMethod(self, m);
            }
        }
    }
}

static void Amalgame_Compiler_TypeChecker_CheckClass(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* cls) {
    (void)self;
    (void)cls;
    code_string __attribute__((unused)) prevClass = self->CurrentClass;
    self->CurrentClass = cls->Name;
    i64 __attribute__((unused)) members = AmalgameList_count(cls->Children);
    for (i64 i = 0; i < members; i++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) m = (Amalgame_Compiler_AstNode*)AmalgameList_get(cls->Children, i);
        Amalgame_Compiler_NodeKind __attribute__((unused)) mk = m->Kind;
        if (mk == Amalgame_Compiler_NodeKind_VAR_DECL) {
            Amalgame_Compiler_TypeChecker_CheckFieldDecl(self, m);
        }
        if (mk == Amalgame_Compiler_NodeKind_METHOD_DECL) {
            Amalgame_Compiler_TypeChecker_CheckMethod(self, m);
        }
    }
    self->CurrentClass = prevClass;
}

static void Amalgame_Compiler_TypeChecker_CheckFieldDecl(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* field) {
    (void)self;
    (void)field;
    code_string __attribute__((unused)) fieldType = field->Str;
    if (field->Left != NULL) {
        Amalgame_Compiler_TypeChecker_CheckExpr(self, field->Left);
        code_string __attribute__((unused)) initType = Amalgame_Compiler_TypeChecker_GetType(self, field->Left);
        if (!Amalgame_Compiler_TypeChecker_IsAssignable(self, fieldType, initType)) {
            Amalgame_Compiler_TypeChecker_Error(self, code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("Field '", field->Name), "' declared as '"), fieldType), "' but initialised with '"), initType), "'"), field->Left);
        }
    }
}

static void Amalgame_Compiler_TypeChecker_CheckMethod(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* method) {
    (void)self;
    (void)method;
    code_string __attribute__((unused)) retType = method->Str;
    code_string __attribute__((unused)) prevReturn = self->CurrentReturn;
    self->CurrentReturn = retType;
    i64 __attribute__((unused)) params = AmalgameList_count(method->Params);
    for (i64 i = 0; i < params; i++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) p = (Amalgame_Compiler_AstNode*)AmalgameList_get(method->Params, i);
        if (p->Left != NULL) {
            Amalgame_Compiler_TypeChecker_CheckExpr(self, p->Left);
            code_string __attribute__((unused)) defType = Amalgame_Compiler_TypeChecker_GetType(self, p->Left);
            if (!Amalgame_Compiler_TypeChecker_IsAssignable(self, p->Str, defType)) {
                Amalgame_Compiler_TypeChecker_Error(self, code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("Parameter '", p->Name), "' is '"), p->Str), "' but default is '"), defType), "'"), p->Left);
            }
        }
    }
    if (method->Body != NULL) {
        Amalgame_Compiler_TypeChecker_CheckBlock(self, method->Body);
    }
    self->CurrentReturn = prevReturn;
}

static void Amalgame_Compiler_TypeChecker_CheckBlock(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* block) {
    (void)self;
    (void)block;
    i64 __attribute__((unused)) stmts = AmalgameList_count(block->Children);
    for (i64 i = 0; i < stmts; i++) {
        Amalgame_Compiler_TypeChecker_CheckStmt(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(block->Children, i));
    }
}

static void Amalgame_Compiler_TypeChecker_CheckStmt(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* stmt) {
    (void)self;
    (void)stmt;
    Amalgame_Compiler_NodeKind __attribute__((unused)) k = stmt->Kind;
    if (k == Amalgame_Compiler_NodeKind_VAR_DECL) {
        Amalgame_Compiler_TypeChecker_CheckVarDecl(self, stmt);
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_RETURN_STMT) {
        Amalgame_Compiler_TypeChecker_CheckReturn(self, stmt);
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_IF_STMT) {
        Amalgame_Compiler_TypeChecker_CheckIf(self, stmt);
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_WHILE_STMT) {
        if (stmt->Cond != NULL) {
            Amalgame_Compiler_TypeChecker_CheckExpr(self, stmt->Cond);
            Amalgame_Compiler_TypeChecker_CheckBool(self, stmt->Cond, "while condition");
        }
        if (stmt->Body != NULL) {
            Amalgame_Compiler_TypeChecker_CheckBlock(self, stmt->Body);
        }
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_FOR_IN_STMT) {
        Amalgame_Compiler_TypeChecker_CheckForIn(self, stmt);
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_BREAK_STMT || k == Amalgame_Compiler_NodeKind_CONTINUE_STMT) {
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_ASSIGN) {
        Amalgame_Compiler_TypeChecker_CheckAssign(self, stmt);
        return;
    }
    if (code_string_equals(stmt->Name, "__match__")) {
        Amalgame_Compiler_TypeChecker_CheckMatch(self, stmt);
        return;
    }
    Amalgame_Compiler_TypeChecker_CheckExpr(self, stmt);
}

static void Amalgame_Compiler_TypeChecker_CheckVarDecl(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* stmt) {
    (void)self;
    (void)stmt;
    code_string __attribute__((unused)) declaredType = stmt->Str;
    code_string __attribute__((unused)) inferredType = "?";
    if (stmt->Left != NULL) {
        Amalgame_Compiler_TypeChecker_CheckExpr(self, stmt->Left);
        inferredType = Amalgame_Compiler_TypeChecker_GetType(self, stmt->Left);
    }
    void* __attribute__((unused)) finalType = (String_Length(declaredType) > 0 ? declaredType : inferredType);
    if (String_Length(declaredType) > 0 && !code_string_equals(inferredType, "?")) {
        if (!Amalgame_Compiler_TypeChecker_IsAssignable(self, declaredType, inferredType)) {
            Amalgame_Compiler_TypeChecker_Error(self, code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("Cannot assign '", inferredType), "' to '"), stmt->Name), "' of type '"), declaredType), "'"), stmt);
        }
    }
    Amalgame_Compiler_TypeChecker_SymbolSetType(self, stmt->Name, finalType);
}

static void Amalgame_Compiler_TypeChecker_CheckReturn(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* stmt) {
    (void)self;
    (void)stmt;
    if (stmt->Left == NULL) {
        if (!code_string_equals(self->CurrentReturn, "void") && !code_string_equals(self->CurrentReturn, "?")) {
            Amalgame_Compiler_TypeChecker_Error(self, code_string_concat(code_string_concat("Empty return in method expecting '", self->CurrentReturn), "'"), stmt);
        }
        return;
    }
    Amalgame_Compiler_TypeChecker_CheckExpr(self, stmt->Left);
    code_string __attribute__((unused)) valType = Amalgame_Compiler_TypeChecker_GetType(self, stmt->Left);
    if (code_string_equals(self->CurrentReturn, "void")) {
        Amalgame_Compiler_TypeChecker_Error(self, "Cannot return a value from a void method", stmt);
    } else if (!Amalgame_Compiler_TypeChecker_IsAssignable(self, self->CurrentReturn, valType)) {
        Amalgame_Compiler_TypeChecker_Error(self, code_string_concat(code_string_concat(code_string_concat(code_string_concat("Return type mismatch: expected '", self->CurrentReturn), "', got '"), valType), "'"), stmt);
    }
}

static void Amalgame_Compiler_TypeChecker_CheckIf(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* stmt) {
    (void)self;
    (void)stmt;
    if (stmt->Cond != NULL) {
        Amalgame_Compiler_TypeChecker_CheckExpr(self, stmt->Cond);
        Amalgame_Compiler_TypeChecker_CheckBool(self, stmt->Cond, "if condition");
    }
    if (stmt->Body != NULL) {
        Amalgame_Compiler_TypeChecker_CheckBlock(self, stmt->Body);
    }
    if (stmt->Else != NULL) {
        Amalgame_Compiler_TypeChecker_CheckElseBranch(self, stmt->Else);
    }
}

static void Amalgame_Compiler_TypeChecker_CheckElseBranch(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* branch) {
    (void)self;
    (void)branch;
    if (branch == NULL) {
        return;
    }
    Amalgame_Compiler_NodeKind __attribute__((unused)) bk = branch->Kind;
    if (bk == Amalgame_Compiler_NodeKind_IF_STMT) {
        Amalgame_Compiler_TypeChecker_CheckIf(self, branch);
    } else {
        Amalgame_Compiler_TypeChecker_CheckBlock(self, branch);
    }
}

static void Amalgame_Compiler_TypeChecker_CheckForIn(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* stmt) {
    (void)self;
    (void)stmt;
    if (stmt->Left != NULL) {
        Amalgame_Compiler_TypeChecker_CheckExpr(self, stmt->Left);
        code_string __attribute__((unused)) colType = Amalgame_Compiler_TypeChecker_GetType(self, stmt->Left);
        code_string __attribute__((unused)) elemType = Amalgame_Compiler_TypeChecker_CollectionElementType(self, colType);
        Amalgame_Compiler_TypeChecker_SymbolSetType(self, stmt->Name, elemType);
    }
    if (stmt->Body != NULL) {
        Amalgame_Compiler_TypeChecker_CheckBlock(self, stmt->Body);
    }
}

static void Amalgame_Compiler_TypeChecker_CheckAssign(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* stmt) {
    (void)self;
    (void)stmt;
    if (stmt->Left != NULL) {
        Amalgame_Compiler_TypeChecker_CheckExpr(self, stmt->Left);
    }
    if (stmt->Right != NULL) {
        Amalgame_Compiler_TypeChecker_CheckExpr(self, stmt->Right);
    }
    if (stmt->Left != NULL && stmt->Right != NULL) {
        code_string __attribute__((unused)) targetType = Amalgame_Compiler_TypeChecker_GetType(self, stmt->Left);
        code_string __attribute__((unused)) valueType = Amalgame_Compiler_TypeChecker_GetType(self, stmt->Right);
        if (!Amalgame_Compiler_TypeChecker_IsAssignable(self, targetType, valueType)) {
            Amalgame_Compiler_TypeChecker_Error(self, code_string_concat(code_string_concat(code_string_concat(code_string_concat("Cannot assign '", valueType), "' to '"), targetType), "'"), stmt);
        }
    }
}

static void Amalgame_Compiler_TypeChecker_CheckMatch(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* stmt) {
    (void)self;
    (void)stmt;
    if (stmt->Left != NULL) {
        Amalgame_Compiler_TypeChecker_CheckExpr(self, stmt->Left);
    }
    i64 __attribute__((unused)) armCount = AmalgameList_count(stmt->Children);
    for (i64 i = 0; i < armCount; i++) {
        Amalgame_Compiler_TypeChecker_CheckMatchArm(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(stmt->Children, i));
    }
}

static void Amalgame_Compiler_TypeChecker_CheckMatchArm(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* arm) {
    (void)self;
    (void)arm;
    if (arm == NULL) {
        return;
    }
    Amalgame_Compiler_AstNode* __attribute__((unused)) body = arm->Right;
    if (body == NULL) {
        return;
    }
    Amalgame_Compiler_NodeKind __attribute__((unused)) bk = body->Kind;
    if (bk == Amalgame_Compiler_NodeKind_BLOCK) {
        Amalgame_Compiler_TypeChecker_CheckBlock(self, body);
    } else {
        Amalgame_Compiler_TypeChecker_CheckStmt(self, body);
    }
}

static void Amalgame_Compiler_TypeChecker_CheckExpr(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* expr) {
    (void)self;
    (void)expr;
    if (expr == NULL) {
        return;
    }
    Amalgame_Compiler_NodeKind __attribute__((unused)) k = expr->Kind;
    if (k == Amalgame_Compiler_NodeKind_LITERAL_INT) {
        Amalgame_Compiler_TypeChecker_SetType(self, expr, "int");
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_LITERAL_FLOAT) {
        Amalgame_Compiler_TypeChecker_SetType(self, expr, "float");
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_LITERAL_STRING) {
        Amalgame_Compiler_TypeChecker_SetType(self, expr, "string");
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_LITERAL_BOOL) {
        Amalgame_Compiler_TypeChecker_SetType(self, expr, "bool");
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_LITERAL_NULL) {
        Amalgame_Compiler_TypeChecker_SetType(self, expr, "null");
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_THIS_EXPR) {
        Amalgame_Compiler_TypeChecker_SetType(self, expr, self->CurrentClass);
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_IDENTIFIER) {
        code_string __attribute__((unused)) t = Amalgame_Compiler_TypeChecker_SymbolTypeName(self, expr->Name);
        Amalgame_Compiler_TypeChecker_SetType(self, expr, t);
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_BINARY) {
        Amalgame_Compiler_TypeChecker_CheckBinaryExpr(self, expr);
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_UNARY) {
        Amalgame_Compiler_TypeChecker_CheckUnaryExpr(self, expr);
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_MEMBER) {
        Amalgame_Compiler_TypeChecker_CheckMemberExpr(self, expr);
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_CALL) {
        Amalgame_Compiler_TypeChecker_CheckCallExpr(self, expr);
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_NEW_EXPR) {
        Amalgame_Compiler_TypeChecker_CheckNewExpr(self, expr);
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_INDEX_EXPR) {
        Amalgame_Compiler_TypeChecker_CheckIndexExpr(self, expr);
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_ASSIGN) {
        Amalgame_Compiler_TypeChecker_CheckAssign(self, expr);
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_IF_STMT) {
        if (expr->Cond != NULL) {
            Amalgame_Compiler_TypeChecker_CheckExpr(self, expr->Cond);
            Amalgame_Compiler_TypeChecker_CheckBool(self, expr->Cond, "if condition");
        }
        code_string __attribute__((unused)) thenType = Amalgame_Compiler_TypeChecker_CheckIfBranch(self, expr->Body);
        code_string __attribute__((unused)) elseType = Amalgame_Compiler_TypeChecker_CheckIfBranch(self, expr->Else);
        void* __attribute__((unused)) ifType = (!code_string_equals(thenType, "?") ? thenType : elseType);
        Amalgame_Compiler_TypeChecker_SetType(self, expr, ifType);
        return;
    }
    Amalgame_Compiler_TypeChecker_SetType(self, expr, "?");
}

static code_string Amalgame_Compiler_TypeChecker_CheckIfBranch(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* block) {
    (void)self;
    (void)block;
    if (block == NULL) {
        return "?";
    }
    if (block->Kind == Amalgame_Compiler_NodeKind_BLOCK) {
        AmalgameList* __attribute__((unused)) kids = block->Children;
        if (AmalgameList_count(kids) > 0) {
            void* __attribute__((unused)) first = (void*)AmalgameList_get(kids, 0);
            Amalgame_Compiler_TypeChecker_CheckExpr(self, first);
            return Amalgame_Compiler_TypeChecker_GetType(self, first);
        }
        return "?";
    }
    Amalgame_Compiler_TypeChecker_CheckExpr(self, block);
    return Amalgame_Compiler_TypeChecker_GetType(self, block);
}

static void Amalgame_Compiler_TypeChecker_CheckBinaryExpr(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* expr) {
    (void)self;
    (void)expr;
    code_string __attribute__((unused)) op = expr->Str;
    if (expr->Left != NULL) {
        Amalgame_Compiler_TypeChecker_CheckExpr(self, expr->Left);
    }
    if (expr->Right != NULL) {
        Amalgame_Compiler_TypeChecker_CheckExpr(self, expr->Right);
    }
    void* __attribute__((unused)) lt = (expr->Left != NULL ? Amalgame_Compiler_TypeChecker_GetType(self, expr->Left) : "?");
    void* __attribute__((unused)) rt = (expr->Right != NULL ? Amalgame_Compiler_TypeChecker_GetType(self, expr->Right) : "?");
    if (code_string_equals(op, "&&") || code_string_equals(op, "||")) {
        if (!Amalgame_Compiler_TypeChecker_IsBool(self, lt)) {
            Amalgame_Compiler_TypeChecker_Error(self, code_string_concat(code_string_concat(code_string_concat(code_string_concat("Left operand of '", op), "' must be bool, got '"), (code_string)(lt)), "'"), expr->Left);
        }
        if (!Amalgame_Compiler_TypeChecker_IsBool(self, rt)) {
            Amalgame_Compiler_TypeChecker_Error(self, code_string_concat(code_string_concat(code_string_concat(code_string_concat("Right operand of '", op), "' must be bool, got '"), (code_string)(rt)), "'"), expr->Right);
        }
    }
    if (code_string_equals(op, "-") || code_string_equals(op, "*") || code_string_equals(op, "/") || code_string_equals(op, "%") || code_string_equals(op, "^")) {
        if (!Amalgame_Compiler_TypeChecker_IsNumeric(self, lt) && !code_string_equals(lt, "?")) {
            Amalgame_Compiler_TypeChecker_Error(self, code_string_concat(code_string_concat(code_string_concat(code_string_concat("Operator '", op), "' requires numeric operands, got '"), (code_string)(lt)), "'"), expr->Left);
        }
        if (!Amalgame_Compiler_TypeChecker_IsNumeric(self, rt) && !code_string_equals(rt, "?")) {
            Amalgame_Compiler_TypeChecker_Error(self, code_string_concat(code_string_concat(code_string_concat(code_string_concat("Operator '", op), "' requires numeric operands, got '"), (code_string)(rt)), "'"), expr->Right);
        }
    }
    if (code_string_equals(op, "??")) {
        if (!Amalgame_Compiler_TypeChecker_IsNullable(self, lt) && !code_string_equals(lt, "?")) {
            code_string __attribute__((unused)) qqOp = "?";
            code_string __attribute__((unused)) qqOp2 = "?";
            Amalgame_Compiler_TypeChecker_Error(self, code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("Left operand of '", qqOp), qqOp2), "' must be nullable, got '"), (code_string)(lt)), "'"), expr->Left);
        }
    }
    code_string __attribute__((unused)) result = Amalgame_Compiler_TypeChecker_BinaryResultType(self, op, lt, rt);
    Amalgame_Compiler_TypeChecker_SetType(self, expr, result);
}

static void Amalgame_Compiler_TypeChecker_CheckUnaryExpr(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* expr) {
    (void)self;
    (void)expr;
    code_string __attribute__((unused)) op = expr->Str;
    if (expr->Left != NULL) {
        Amalgame_Compiler_TypeChecker_CheckExpr(self, expr->Left);
    }
    void* __attribute__((unused)) ot = (expr->Left != NULL ? Amalgame_Compiler_TypeChecker_GetType(self, expr->Left) : "?");
    if (code_string_equals(op, "!")) {
        if (!Amalgame_Compiler_TypeChecker_IsBool(self, ot)) {
            Amalgame_Compiler_TypeChecker_Error(self, code_string_concat(code_string_concat("Operator '!' requires bool, got '", (code_string)(ot)), "'"), expr->Left);
        }
        Amalgame_Compiler_TypeChecker_SetType(self, expr, "bool");
        return;
    }
    if (code_string_equals(op, "-")) {
        if (!Amalgame_Compiler_TypeChecker_IsNumeric(self, ot) && !code_string_equals(ot, "?")) {
            Amalgame_Compiler_TypeChecker_Error(self, code_string_concat(code_string_concat("Unary '-' requires numeric, got '", (code_string)(ot)), "'"), expr->Left);
        }
        Amalgame_Compiler_TypeChecker_SetType(self, expr, ot);
        return;
    }
    Amalgame_Compiler_TypeChecker_SetType(self, expr, ot);
}

static void Amalgame_Compiler_TypeChecker_CheckMemberExpr(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* expr) {
    (void)self;
    (void)expr;
    if (expr->Left != NULL) {
        Amalgame_Compiler_TypeChecker_CheckExpr(self, expr->Left);
    }
    void* __attribute__((unused)) targetType = (expr->Left != NULL ? Amalgame_Compiler_TypeChecker_GetType(self, expr->Left) : "?");
    void* __attribute__((unused)) baseType = targetType;
    if (String_EndsWith(targetType, "?")) {
        baseType = String_Substring(targetType, 0, String_Length(targetType) - 1);
    }
    code_string __attribute__((unused)) memberType = "?";
    Amalgame_Compiler_TypeChecker_SetType(self, expr, memberType);
}

static void Amalgame_Compiler_TypeChecker_CheckCallExpr(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* expr) {
    (void)self;
    (void)expr;
    if (expr->Left != NULL) {
        Amalgame_Compiler_TypeChecker_CheckExpr(self, expr->Left);
    }
    i64 __attribute__((unused)) argc = AmalgameList_count(expr->Args);
    for (i64 i = 0; i < argc; i++) {
        Amalgame_Compiler_TypeChecker_CheckExpr(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(expr->Args, i));
    }
    code_string __attribute__((unused)) calleeType = "?";
    Amalgame_Compiler_AstNode* __attribute__((unused)) callee = expr->Left;
    if (callee != NULL) {
        Amalgame_Compiler_NodeKind __attribute__((unused)) calleeKind = callee->Kind;
        if (calleeKind == Amalgame_Compiler_NodeKind_IDENTIFIER) {
            calleeType = Amalgame_Compiler_TypeChecker_SymbolTypeName(self, callee->Name);
        } else if (calleeKind == Amalgame_Compiler_NodeKind_MEMBER) {
            calleeType = Amalgame_Compiler_TypeChecker_GetType(self, callee);
        }
    }
    Amalgame_Compiler_TypeChecker_SetType(self, expr, calleeType);
}

static void Amalgame_Compiler_TypeChecker_CheckNewExpr(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* expr) {
    (void)self;
    (void)expr;
    i64 __attribute__((unused)) argc = AmalgameList_count(expr->Args);
    for (i64 i = 0; i < argc; i++) {
        Amalgame_Compiler_TypeChecker_CheckExpr(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(expr->Args, i));
    }
    Amalgame_Compiler_TypeChecker_SetType(self, expr, expr->Name);
}

static void Amalgame_Compiler_TypeChecker_CheckIndexExpr(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* expr) {
    (void)self;
    (void)expr;
    if (expr->Left != NULL) {
        Amalgame_Compiler_TypeChecker_CheckExpr(self, expr->Left);
    }
    if (expr->Right != NULL) {
        Amalgame_Compiler_TypeChecker_CheckExpr(self, expr->Right);
    }
    void* __attribute__((unused)) targetType = (expr->Left != NULL ? Amalgame_Compiler_TypeChecker_GetType(self, expr->Left) : "?");
    code_string __attribute__((unused)) elemType = Amalgame_Compiler_TypeChecker_CollectionElementType(self, targetType);
    Amalgame_Compiler_TypeChecker_SetType(self, expr, elemType);
}

code_bool Amalgame_Compiler_TypeChecker_HasErrors(Amalgame_Compiler_TypeChecker* self) {
    (void)self;
    return AmalgameList_count(self->Errors) > 0;
}

code_string Amalgame_Compiler_TypeChecker_FormatErrors(Amalgame_Compiler_TypeChecker* self) {
    (void)self;
    code_string __attribute__((unused)) result = "";
    i64 __attribute__((unused)) count = AmalgameList_count(self->Errors);
    for (i64 i = 0; i < count; i++) {
        Amalgame_Compiler_TypeError* __attribute__((unused)) e = (Amalgame_Compiler_TypeError*)AmalgameList_get(self->Errors, i);
        result = code_string_concat(result, Amalgame_Compiler_TypeError_ToString(e));
    }
    return result;
}

struct _Amalgame_Compiler_AmalgameCompiler {
    Amalgame_Compiler_DiagnosticFormatter* Diag;
};

void Amalgame_Compiler_AmalgameCompiler_Run(Amalgame_Compiler_AmalgameCompiler* self, AmalgameList* inputFiles, code_string outputName);

Amalgame_Compiler_AmalgameCompiler* Amalgame_Compiler_AmalgameCompiler_new() {
    Amalgame_Compiler_AmalgameCompiler* self = (Amalgame_Compiler_AmalgameCompiler*) GC_MALLOC(sizeof(Amalgame_Compiler_AmalgameCompiler));
    self->Diag = Amalgame_Compiler_DiagnosticFormatter_new();
    return self;
}

void Amalgame_Compiler_AmalgameCompiler_Run(Amalgame_Compiler_AmalgameCompiler* self, AmalgameList* inputFiles, code_string outputName) {
    (void)self;
    (void)inputFiles;
    (void)outputName;
    i64 __attribute__((unused)) inputCount = AmalgameList_count(inputFiles);
    if (inputCount == 0) {
        Console_WriteError("amc: no input .am files");
        return;
    }
    Console_WriteLine(code_string_concat(code_string_concat("Compiling: ", String_FromInt(inputCount)), " file(s)"));
    void* __attribute__((unused)) firstPath = (void*)AmalgameList_get(inputFiles, 0);
    code_string __attribute__((unused)) firstSrc = File_ReadAll(firstPath);
    code_string __attribute__((unused)) nsPrefix = "App";
    i64 __attribute__((unused)) nlIdx = String_IndexOf(firstSrc, "namespace ");
    if (nlIdx >= 0) {
        code_string __attribute__((unused)) nlRest = String_Substring(firstSrc, nlIdx + 10, 200);
        i64 __attribute__((unused)) nlEnd = String_IndexOf(nlRest, "\n");
        if (nlEnd > 0) {
            code_string __attribute__((unused)) rawNs = String_Substring(nlRest, 0, nlEnd);
            nsPrefix = String_Replace(rawNs, ".", "_");
        }
    }
    Amalgame_Compiler_CGen* __attribute__((unused)) gen = Amalgame_Compiler_CGen_new();
    Amalgame_Compiler_CGen_BeginMulti(gen, nsPrefix);
    AmalgameList* __attribute__((unused)) progs = AmalgameList_new();
    for (i64 i = 0; i < inputCount; i++) {
        void* __attribute__((unused)) path = (void*)AmalgameList_get(inputFiles, i);
        code_string __attribute__((unused)) src = File_ReadAll(path);
        Amalgame_Compiler_Lexer* __attribute__((unused)) lex = Amalgame_Compiler_Lexer_new(src, path);
        AmalgameList* __attribute__((unused)) toks = Amalgame_Compiler_Lexer_Tokenize(lex);
        Amalgame_Compiler_Parser* __attribute__((unused)) par = Amalgame_Compiler_Parser_new(toks);
        Amalgame_Compiler_AstNode* __attribute__((unused)) prog = Amalgame_Compiler_Parser_Parse(par);
        AmalgameList_add(progs, (void*)(intptr_t)(prog));
        Amalgame_Compiler_CGen_AddFilePass1(gen, prog);
    }
    Amalgame_Compiler_FullResolver* __attribute__((unused)) resolver = Amalgame_Compiler_FullResolver_new();
    i64 __attribute__((unused)) rprogCount = AmalgameList_count(progs);
    for (i64 ri = 0; ri < rprogCount; ri++) {
        AmalgameList_add(resolver->Programs, (void*)(intptr_t)((void*)AmalgameList_get(progs, ri)));
    }
    Amalgame_Compiler_FullResolver_ResolvePrograms(resolver);
    if (Amalgame_Compiler_FullResolver_HasErrors(resolver)) {
        Console_WriteError(Amalgame_Compiler_FullResolver_GetErrors(resolver));
    }
    Amalgame_Compiler_TypeChecker* __attribute__((unused)) tc = Amalgame_Compiler_TypeChecker_new(resolver, firstPath);
    void* __attribute__((unused)) tcProg = (void*)AmalgameList_get(progs, 0);
    Amalgame_Compiler_TypeChecker_Check(tc, tcProg);
    if (Amalgame_Compiler_TypeChecker_HasErrors(tc)) {
        Console_WriteError(Amalgame_Compiler_TypeChecker_FormatErrors(tc));
    }
    Amalgame_Compiler_CGen_EmitSeparator(gen);
    for (i64 j = 0; j < inputCount; j++) {
        void* __attribute__((unused)) prog2 = (void*)AmalgameList_get(progs, j);
        Amalgame_Compiler_CGen_AddFilePass2(gen, prog2);
    }
    AmalgameList* __attribute__((unused)) lines = Amalgame_Compiler_CGen_GetLines(gen);
    i64 __attribute__((unused)) lineCount = AmalgameList_count(lines);
    code_string __attribute__((unused)) outC = code_string_concat(outputName, ".c");
    File_WriteAll(outC, "");
    for (i64 k = 0; k < lineCount; k++) {
        void* __attribute__((unused)) line = (void*)AmalgameList_get(lines, k);
        File_AppendAll(outC, code_string_concat((code_string)(line), "\n"));
    }
    code_string __attribute__((unused)) mainFunc = code_string_concat(nsPrefix, "_Program_Main");
    code_string __attribute__((unused)) genSrc = File_ReadAll(outC);
    if (String_Contains(genSrc, mainFunc)) {
        File_AppendAll(outC, "\nint main(int argc, char** argv) {\n");
        File_AppendAll(outC, "    GC_INIT();\n");
        File_AppendAll(outC, code_string_concat(code_string_concat("    ", mainFunc), "((code_string*)argv);\n"));
        File_AppendAll(outC, "    return 0;\n");
        File_AppendAll(outC, "}\n");
    }
    Console_WriteLine(code_string_concat(code_string_concat(code_string_concat(code_string_concat("Generated: ", outC), " ("), String_FromInt(lineCount)), " lines)"));
    Amalgame_Compiler_DiagnosticFormatter_PrintCompileOk(self->Diag, "Build");
}

struct _Amalgame_Compiler_AmcEntry {
};

void Amalgame_Compiler_AmcEntry_AmcStart();

Amalgame_Compiler_AmcEntry* Amalgame_Compiler_AmcEntry_new() {
    Amalgame_Compiler_AmcEntry* self = (Amalgame_Compiler_AmcEntry*) GC_MALLOC(sizeof(Amalgame_Compiler_AmcEntry));
    return self;
}

void Amalgame_Compiler_AmcEntry_AmcStart() {
    Console_WriteError("Use amc_bootstrap binary directly");
}

