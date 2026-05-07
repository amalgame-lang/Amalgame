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
    Amalgame_Compiler_TokenType_OP_AMP,
    Amalgame_Compiler_TokenType_OP_PIPE,
    Amalgame_Compiler_TokenType_OP_CARET,
    Amalgame_Compiler_TokenType_OP_TILDE,
    Amalgame_Compiler_TokenType_OP_SHL,
    Amalgame_Compiler_TokenType_OP_SHR,
    Amalgame_Compiler_TokenType_OP_PLUS_EQ,
    Amalgame_Compiler_TokenType_OP_MINUS_EQ,
    Amalgame_Compiler_TokenType_OP_STAR_EQ,
    Amalgame_Compiler_TokenType_OP_SLASH_EQ,
    Amalgame_Compiler_TokenType_OP_PERCENT_EQ,
    Amalgame_Compiler_TokenType_OP_AMP_EQ,
    Amalgame_Compiler_TokenType_OP_PIPE_EQ,
    Amalgame_Compiler_TokenType_OP_CARET_EQ,
    Amalgame_Compiler_TokenType_OP_SHL_EQ,
    Amalgame_Compiler_TokenType_OP_SHR_EQ,
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
static i64 Amalgame_Compiler_Lexer_HexNibble(Amalgame_Compiler_Lexer* self, code_string ch);
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
    if (String_Length(c) == 0) {
        return 0;
    }
    return code_string_equals(c, " ") || code_string_equals(c, "\t") || code_string_equals(c, "\\r");
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
            if (code_string_equals(esc, "r")) {
                value = code_string_concat(value, "\\r");
            }
            if (code_string_equals(esc, "\"")) {
                value = code_string_concat(value, "\"");
            }
            if (code_string_equals(esc, "\\")) {
                value = code_string_concat(value, "\\");
            }
            if (code_string_equals(esc, "x")) {
                code_string __attribute__((unused)) h1 = Amalgame_Compiler_Lexer_Advance(self);
                code_string __attribute__((unused)) h2 = Amalgame_Compiler_Lexer_Advance(self);
                i64 __attribute__((unused)) byte = Amalgame_Compiler_Lexer_HexNibble(self, h1) * 16 + Amalgame_Compiler_Lexer_HexNibble(self, h2);
                value = code_string_concat(value, String_FromByte(byte));
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

static i64 Amalgame_Compiler_Lexer_HexNibble(Amalgame_Compiler_Lexer* self, code_string ch) {
    (void)self;
    (void)ch;
    if (code_string_equals(ch, "0")) {
        return 0;
    }
    if (code_string_equals(ch, "1")) {
        return 1;
    }
    if (code_string_equals(ch, "2")) {
        return 2;
    }
    if (code_string_equals(ch, "3")) {
        return 3;
    }
    if (code_string_equals(ch, "4")) {
        return 4;
    }
    if (code_string_equals(ch, "5")) {
        return 5;
    }
    if (code_string_equals(ch, "6")) {
        return 6;
    }
    if (code_string_equals(ch, "7")) {
        return 7;
    }
    if (code_string_equals(ch, "8")) {
        return 8;
    }
    if (code_string_equals(ch, "9")) {
        return 9;
    }
    if (code_string_equals(ch, "a") || code_string_equals(ch, "A")) {
        return 10;
    }
    if (code_string_equals(ch, "b") || code_string_equals(ch, "B")) {
        return 11;
    }
    if (code_string_equals(ch, "c") || code_string_equals(ch, "C")) {
        return 12;
    }
    if (code_string_equals(ch, "d") || code_string_equals(ch, "D")) {
        return 13;
    }
    if (code_string_equals(ch, "e") || code_string_equals(ch, "E")) {
        return 14;
    }
    if (code_string_equals(ch, "f") || code_string_equals(ch, "F")) {
        return 15;
    }
    return 0;
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
        if (code_string_equals(c2, "=")) {
            Amalgame_Compiler_Lexer_Advance(self);
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_PLUS_EQ, "+=");
        } else {
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_PLUS, "+");
        }
    } else if (code_string_equals(c, "%")) {
        if (code_string_equals(c2, "=")) {
            Amalgame_Compiler_Lexer_Advance(self);
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_PERCENT_EQ, "%=");
        } else {
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_PERCENT, "%");
        }
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
        if (code_string_equals(c2, "=")) {
            Amalgame_Compiler_Lexer_Advance(self);
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_SLASH_EQ, "/=");
        } else {
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_SLASH, "/");
        }
    } else if (code_string_equals(c, "-")) {
        if (code_string_equals(c2, ">")) {
            Amalgame_Compiler_Lexer_Advance(self);
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_THIN_ARROW, "->");
        } else if (code_string_equals(c2, "=")) {
            Amalgame_Compiler_Lexer_Advance(self);
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_MINUS_EQ, "-=");
        } else {
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_MINUS, "-");
        }
    } else if (code_string_equals(c, "*")) {
        if (code_string_equals(c2, "=")) {
            Amalgame_Compiler_Lexer_Advance(self);
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_STAR_EQ, "*=");
        } else {
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_STAR, "*");
        }
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
        } else if (code_string_equals(c2, "<")) {
            Amalgame_Compiler_Lexer_Advance(self);
            code_string __attribute__((unused)) c3 = Amalgame_Compiler_Lexer_CharAt(self, self->Pos);
            if (code_string_equals(c3, "=")) {
                Amalgame_Compiler_Lexer_Advance(self);
                Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_SHL_EQ, "<<=");
            } else {
                Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_SHL, "<<");
            }
        } else {
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_LT, "<");
        }
    } else if (code_string_equals(c, ">")) {
        if (code_string_equals(c2, "=")) {
            Amalgame_Compiler_Lexer_Advance(self);
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_GTE, ">=");
        } else if (code_string_equals(c2, ">")) {
            Amalgame_Compiler_Lexer_Advance(self);
            code_string __attribute__((unused)) c3 = Amalgame_Compiler_Lexer_CharAt(self, self->Pos);
            if (code_string_equals(c3, "=")) {
                Amalgame_Compiler_Lexer_Advance(self);
                Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_SHR_EQ, ">>=");
            } else {
                Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_SHR, ">>");
            }
        } else {
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_GT, ">");
        }
    } else if (code_string_equals(c, "&")) {
        if (code_string_equals(c2, "&")) {
            Amalgame_Compiler_Lexer_Advance(self);
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_AND, "&&");
        } else if (code_string_equals(c2, "=")) {
            Amalgame_Compiler_Lexer_Advance(self);
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_AMP_EQ, "&=");
        } else {
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_AMP, "&");
        }
    } else if (code_string_equals(c, "|")) {
        if (code_string_equals(c2, "|")) {
            Amalgame_Compiler_Lexer_Advance(self);
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_OR, "||");
        } else if (code_string_equals(c2, "=")) {
            Amalgame_Compiler_Lexer_Advance(self);
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_PIPE_EQ, "|=");
        } else {
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_PIPE, "|");
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
    } else if (code_string_equals(c, "~")) {
        Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_TILDE, "~");
    } else if (code_string_equals(c, "^")) {
        if (code_string_equals(c2, "=")) {
            Amalgame_Compiler_Lexer_Advance(self);
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_CARET_EQ, "^=");
        } else {
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_CARET, "^");
        }
    } else {
        Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_UNKNOWN, c);
    }
}

struct _Amalgame_Compiler_Parser {
    AmalgameList* Tokens;
    i64 TokenCount;
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
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseBitwiseOr(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseBitwiseXor(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseBitwiseAnd(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseAnd(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseEquality(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseRelational(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseShift(Amalgame_Compiler_Parser* self);
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
    self->TokenCount = AmalgameList_count(tokens);
    self->Pos = 0;
    self->Errors = AmalgameList_new();
    self->ParenDepth = 0;
    return self;
}

Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_Parse(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_AstNode* __attribute__((unused)) prog = Amalgame_Compiler_Ast_Program(1, 1);
    if (AmalgameList_count(self->Tokens) > 0) {
        Amalgame_Compiler_Token* __attribute__((unused)) firstTok = (Amalgame_Compiler_Token*)AmalgameList_get(self->Tokens, 0);
        prog->Str2 = firstTok->Filename;
    }
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
    i64 __attribute__((unused)) count = self->TokenCount;
    if (self->Pos >= count) {
        return (Amalgame_Compiler_Token*)AmalgameList_get(self->Tokens, count - 1);
    }
    return (Amalgame_Compiler_Token*)AmalgameList_get(self->Tokens, self->Pos);
}

static Amalgame_Compiler_Token* Amalgame_Compiler_Parser_Peek(Amalgame_Compiler_Parser* self, i64 offset) {
    (void)self;
    (void)offset;
    i64 __attribute__((unused)) i = self->Pos + offset;
    i64 __attribute__((unused)) count = self->TokenCount;
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
    if (self->Pos >= self->TokenCount) {
        return 1;
    }
    Amalgame_Compiler_Token* __attribute__((unused)) tok = (Amalgame_Compiler_Token*)AmalgameList_get(self->Tokens, self->Pos);
    return tok->Type == Amalgame_Compiler_TokenType_EOF;
}

static code_bool Amalgame_Compiler_Parser_CheckType(Amalgame_Compiler_Parser* self, Amalgame_Compiler_TokenType t) {
    (void)self;
    (void)t;
    if (self->Pos >= self->TokenCount) {
        return 0;
    }
    Amalgame_Compiler_Token* __attribute__((unused)) tok = (Amalgame_Compiler_Token*)AmalgameList_get(self->Tokens, self->Pos);
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
    if (self->Pos >= self->TokenCount) {
        return 0;
    }
    Amalgame_Compiler_Token* __attribute__((unused)) tok = (Amalgame_Compiler_Token*)AmalgameList_get(self->Tokens, self->Pos);
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
    Amalgame_Compiler_Parser_SkipNewlines(self);
    while (!Amalgame_Compiler_Parser_IsEnd(self) && !Amalgame_Compiler_Parser_CheckValue(self, ")")) {
        if (Amalgame_Compiler_Parser_CheckValue(self, ",")) {
            Amalgame_Compiler_Parser_Advance(self);
            Amalgame_Compiler_Parser_SkipNewlines(self);
            continue;
        }
        Amalgame_Compiler_AstNode* __attribute__((unused)) p = Amalgame_Compiler_Parser_ParseParam(self);
        Amalgame_Compiler_NodeKind __attribute__((unused)) pk = p->Kind;
        if (pk != Amalgame_Compiler_NodeKind_IDENTIFIER) {
            AmalgameList_add(method->Params, (void*)(intptr_t)(p));
        }
        Amalgame_Compiler_Parser_SkipNewlines(self);
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
    code_bool __attribute__((unused)) hadProgress = 1;
    while (!Amalgame_Compiler_Parser_IsEnd(self) && !Amalgame_Compiler_Parser_CheckValue(self, "}")) {
        Amalgame_Compiler_Parser_SkipNewlines(self);
        if (Amalgame_Compiler_Parser_CheckValue(self, "}")) {
            break;
        }
        i64 __attribute__((unused)) curPos = self->Pos;
        if (!hadProgress) {
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
        hadProgress = 0;
        Amalgame_Compiler_AstNode* __attribute__((unused)) stmt = Amalgame_Compiler_Parser_ParseStmt(self);
        AmalgameList_add(block->Children, (void*)(intptr_t)(stmt));
        hadProgress = 1;
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
    code_bool __attribute__((unused)) isAssign = Amalgame_Compiler_Parser_CheckValue(self, "=") || Amalgame_Compiler_Parser_CheckValue(self, "+=") || Amalgame_Compiler_Parser_CheckValue(self, "-=") || Amalgame_Compiler_Parser_CheckValue(self, "*=") || Amalgame_Compiler_Parser_CheckValue(self, "/=") || Amalgame_Compiler_Parser_CheckValue(self, "%=");
    code_bool __attribute__((unused)) isBitAssign = Amalgame_Compiler_Parser_CheckValue(self, "&=") || Amalgame_Compiler_Parser_CheckValue(self, "|=") || Amalgame_Compiler_Parser_CheckValue(self, "^=") || Amalgame_Compiler_Parser_CheckValue(self, "<<=") || Amalgame_Compiler_Parser_CheckValue(self, ">>=");
    if (isAssign || isBitAssign) {
        Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Advance(self);
        Amalgame_Compiler_AstNode* __attribute__((unused)) right = Amalgame_Compiler_Parser_ParseAssign(self);
        return Amalgame_Compiler_Ast_Binary(left, tok->Value, right, tok->Line, tok->Column);
    }
    return left;
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseOr(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_AstNode* __attribute__((unused)) left = Amalgame_Compiler_Parser_ParseBitwiseOr(self);
    while (Amalgame_Compiler_Parser_CheckValue(self, "||")) {
        Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Advance(self);
        Amalgame_Compiler_AstNode* __attribute__((unused)) right = Amalgame_Compiler_Parser_ParseBitwiseOr(self);
        left = Amalgame_Compiler_Ast_Binary(left, "||", right, tok->Line, tok->Column);
    }
    return left;
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseBitwiseOr(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_AstNode* __attribute__((unused)) left = Amalgame_Compiler_Parser_ParseBitwiseXor(self);
    while (Amalgame_Compiler_Parser_CheckValue(self, "|") && !Amalgame_Compiler_Parser_CheckValue(self, "||")) {
        Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Advance(self);
        Amalgame_Compiler_AstNode* __attribute__((unused)) right = Amalgame_Compiler_Parser_ParseBitwiseXor(self);
        left = Amalgame_Compiler_Ast_Binary(left, "|", right, tok->Line, tok->Column);
    }
    return left;
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseBitwiseXor(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_AstNode* __attribute__((unused)) left = Amalgame_Compiler_Parser_ParseBitwiseAnd(self);
    while (Amalgame_Compiler_Parser_CheckValue(self, "^")) {
        Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Advance(self);
        Amalgame_Compiler_AstNode* __attribute__((unused)) right = Amalgame_Compiler_Parser_ParseBitwiseAnd(self);
        left = Amalgame_Compiler_Ast_Binary(left, "^", right, tok->Line, tok->Column);
    }
    return left;
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseBitwiseAnd(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_AstNode* __attribute__((unused)) left = Amalgame_Compiler_Parser_ParseAnd(self);
    while (Amalgame_Compiler_Parser_CheckValue(self, "&") && !Amalgame_Compiler_Parser_CheckValue(self, "&&")) {
        Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Advance(self);
        Amalgame_Compiler_AstNode* __attribute__((unused)) right = Amalgame_Compiler_Parser_ParseAnd(self);
        left = Amalgame_Compiler_Ast_Binary(left, "&", right, tok->Line, tok->Column);
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
        Amalgame_Compiler_AstNode* __attribute__((unused)) right = Amalgame_Compiler_Parser_ParseShift(self);
        left = Amalgame_Compiler_Ast_Binary(left, tok->Value, right, tok->Line, tok->Column);
    }
    return left;
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseShift(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_AstNode* __attribute__((unused)) left = Amalgame_Compiler_Parser_ParseAdd(self);
    while (Amalgame_Compiler_Parser_CheckValue(self, "<<") || Amalgame_Compiler_Parser_CheckValue(self, ">>")) {
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
    code_bool __attribute__((unused)) isUnaryTilde = Amalgame_Compiler_Parser_CheckValue(self, "~") && !Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_STRING);
    if (isUnaryNot || isUnaryMinus || isUnaryTilde) {
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

