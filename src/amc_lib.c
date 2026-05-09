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
#include "Amalgame_Process.h"

typedef enum _Amalgame_Compiler_TokenType Amalgame_Compiler_TokenType;
typedef struct _Amalgame_Compiler_Token Amalgame_Compiler_Token;
typedef enum _Amalgame_Compiler_NodeKind Amalgame_Compiler_NodeKind;
typedef struct _Amalgame_Compiler_AstNode Amalgame_Compiler_AstNode;
typedef struct _Amalgame_Compiler_Ast Amalgame_Compiler_Ast;
typedef struct _Amalgame_Compiler_Lexer Amalgame_Compiler_Lexer;
typedef struct _Amalgame_Compiler_Parser Amalgame_Compiler_Parser;
typedef struct _Amalgame_Compiler_Emitter Amalgame_Compiler_Emitter;
typedef struct _Amalgame_Compiler_CGen Amalgame_Compiler_CGen;
typedef struct _Amalgame_Compiler_Formatter Amalgame_Compiler_Formatter;
typedef struct _Amalgame_Compiler_Ansi Amalgame_Compiler_Ansi;
typedef struct _Amalgame_Compiler_SourceMap Amalgame_Compiler_SourceMap;
typedef struct _Amalgame_Compiler_SourceSnippet Amalgame_Compiler_SourceSnippet;
typedef enum _Amalgame_Compiler_DiagSeverity Amalgame_Compiler_DiagSeverity;
typedef struct _Amalgame_Compiler_Diagnostic Amalgame_Compiler_Diagnostic;
typedef struct _Amalgame_Compiler_DiagnosticFormatter Amalgame_Compiler_DiagnosticFormatter;
typedef enum _Amalgame_Compiler_SymKind Amalgame_Compiler_SymKind;
typedef struct _Amalgame_Compiler_Symbol Amalgame_Compiler_Symbol;
typedef struct _Amalgame_Compiler_SymbolTable Amalgame_Compiler_SymbolTable;
typedef struct _Amalgame_Compiler_Resolver Amalgame_Compiler_Resolver;
typedef struct _Amalgame_Compiler_MemberTable Amalgame_Compiler_MemberTable;
typedef struct _Amalgame_Compiler_ResolverError Amalgame_Compiler_ResolverError;
typedef struct _Amalgame_Compiler_FullResolver Amalgame_Compiler_FullResolver;
typedef struct _Amalgame_Compiler_TypeError Amalgame_Compiler_TypeError;
typedef struct _Amalgame_Compiler_TypeCheckResult Amalgame_Compiler_TypeCheckResult;
typedef struct _Amalgame_Compiler_TypeChecker Amalgame_Compiler_TypeChecker;
typedef struct _Amalgame_Compiler_LintWarning Amalgame_Compiler_LintWarning;
typedef struct _Amalgame_Compiler_Linter Amalgame_Compiler_Linter;
typedef struct _Amalgame_Compiler_LspServer Amalgame_Compiler_LspServer;
typedef struct _Amalgame_Compiler_MigrateResult Amalgame_Compiler_MigrateResult;
typedef struct _Amalgame_Compiler_MigrateCommand Amalgame_Compiler_MigrateCommand;
typedef struct _Amalgame_Compiler_GenerateCommand Amalgame_Compiler_GenerateCommand;
typedef struct _Amalgame_Compiler_ExplainCommand Amalgame_Compiler_ExplainCommand;
typedef struct _Amalgame_Compiler_NewCommand Amalgame_Compiler_NewCommand;
typedef struct _Amalgame_Compiler_AmalgameCompiler Amalgame_Compiler_AmalgameCompiler;
typedef struct _Amalgame_Compiler_Program Amalgame_Compiler_Program;

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
    Amalgame_Compiler_TokenType_KW_GUARD,
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
    Amalgame_Compiler_TokenType_OP_QDOT,
    Amalgame_Compiler_TokenType_OP_QMARK,
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
    Amalgame_Compiler_TokenType_COMMENT,
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
    Amalgame_Compiler_NodeKind_TRY_STMT,
    Amalgame_Compiler_NodeKind_THROW_STMT,
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
    Amalgame_Compiler_NodeKind_INDEX_EXPR,
    Amalgame_Compiler_NodeKind_LIST_COMP
};

struct _Amalgame_Compiler_AstNode {
    Amalgame_Compiler_NodeKind Kind;
    i64 Line;
    i64 Column;
    code_string Name;
    code_string Str;
    code_string Str2;
    code_string Str3;
    code_string Str4;
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
    self->Str3 = "";
    self->Str4 = "";
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
static void Amalgame_Compiler_Lexer_ReadLineComment(Amalgame_Compiler_Lexer* self);
AmalgameList* Amalgame_Compiler_Lexer_Tokenize(Amalgame_Compiler_Lexer* self);
static void Amalgame_Compiler_Lexer_ReadString(Amalgame_Compiler_Lexer* self);
static void Amalgame_Compiler_Lexer_ReadTripleQuoted(Amalgame_Compiler_Lexer* self);
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
        } else {
            break;
        }
    }
}

static void Amalgame_Compiler_Lexer_ReadLineComment(Amalgame_Compiler_Lexer* self) {
    (void)self;
    i64 __attribute__((unused)) startLine = self->Line;
    i64 __attribute__((unused)) startCol = self->Column;
    i64 __attribute__((unused)) start = self->Pos;
    while (self->Pos < String_Length(self->Source) && !code_string_equals(Amalgame_Compiler_Lexer_CharAt(self, self->Pos), "\n")) {
        Amalgame_Compiler_Lexer_Advance(self);
    }
    code_string __attribute__((unused)) raw = String_Substring(self->Source, start, self->Pos - start);
    Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Token_new(Amalgame_Compiler_TokenType_COMMENT, raw, startLine, startCol, self->Filename);
    AmalgameList_add(self->Tokens, (void*)(intptr_t)(tok));
}

AmalgameList* Amalgame_Compiler_Lexer_Tokenize(Amalgame_Compiler_Lexer* self) {
    (void)self;
    while (self->Pos < String_Length(self->Source)) {
        Amalgame_Compiler_Lexer_SkipWhitespace(self);
        if (self->Pos >= String_Length(self->Source)) {
            break;
        }
        code_string __attribute__((unused)) ch = Amalgame_Compiler_Lexer_CharAt(self, self->Pos);
        if (code_string_equals(ch, "/") && code_string_equals(Amalgame_Compiler_Lexer_CharAt(self, self->Pos + 1), "/")) {
            Amalgame_Compiler_Lexer_ReadLineComment(self);
        } else if (code_string_equals(ch, "\n")) {
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
    if (code_string_equals(Amalgame_Compiler_Lexer_CharAt(self, self->Pos), "\"") && code_string_equals(Amalgame_Compiler_Lexer_CharAt(self, self->Pos + 1), "\"")) {
        Amalgame_Compiler_Lexer_Advance(self);
        Amalgame_Compiler_Lexer_Advance(self);
        Amalgame_Compiler_Lexer_ReadTripleQuoted(self);
        return;
    }
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
            if (code_string_equals(esc, "u")) {
                code_string __attribute__((unused)) u1 = Amalgame_Compiler_Lexer_Advance(self);
                code_string __attribute__((unused)) u2 = Amalgame_Compiler_Lexer_Advance(self);
                code_string __attribute__((unused)) u3 = Amalgame_Compiler_Lexer_Advance(self);
                code_string __attribute__((unused)) u4 = Amalgame_Compiler_Lexer_Advance(self);
                i64 __attribute__((unused)) cp = Amalgame_Compiler_Lexer_HexNibble(self, u1) * 4096 + Amalgame_Compiler_Lexer_HexNibble(self, u2) * 256 + Amalgame_Compiler_Lexer_HexNibble(self, u3) * 16 + Amalgame_Compiler_Lexer_HexNibble(self, u4);
                value = code_string_concat(value, String_FromCodepoint(cp));
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

static void Amalgame_Compiler_Lexer_ReadTripleQuoted(Amalgame_Compiler_Lexer* self) {
    (void)self;
    code_string __attribute__((unused)) value = "";
    i64 __attribute__((unused)) len = String_Length(self->Source);
    while (self->Pos < len) {
        if (code_string_equals(Amalgame_Compiler_Lexer_CharAt(self, self->Pos), "\"") && code_string_equals(Amalgame_Compiler_Lexer_CharAt(self, self->Pos + 1), "\"") && code_string_equals(Amalgame_Compiler_Lexer_CharAt(self, self->Pos + 2), "\"")) {
            Amalgame_Compiler_Lexer_Advance(self);
            Amalgame_Compiler_Lexer_Advance(self);
            Amalgame_Compiler_Lexer_Advance(self);
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_STRING, value);
            return;
        }
        code_string __attribute__((unused)) c = Amalgame_Compiler_Lexer_Advance(self);
        if (code_string_equals(c, "\n")) {
            self->Line = self->Line + 1;
            self->Column = 1;
        }
        value = code_string_concat(value, c);
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
    if (code_string_equals(word, "guard")) {
        return Amalgame_Compiler_TokenType_KW_GUARD;
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
        } else if (code_string_equals(c2, ".")) {
            Amalgame_Compiler_Lexer_Advance(self);
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_QDOT, "?.");
        } else {
            Amalgame_Compiler_Lexer_AddToken(self, Amalgame_Compiler_TokenType_OP_QMARK, "?");
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
    code_string PendingDecorators;
    AmalgameList* Comments;
};

static void Amalgame_Compiler_Parser_ParseDecoratorList(Amalgame_Compiler_Parser* self);
static code_string Amalgame_Compiler_Parser_TakeDecorators(Amalgame_Compiler_Parser* self);
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
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseTry(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseThrow(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseVarDecl(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseReturn(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseIf(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseGuard(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseWhile(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseForIn(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseEnum(Amalgame_Compiler_Parser* self, code_bool isPublic);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseInterface(Amalgame_Compiler_Parser* self, code_bool isPublic);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseExpr(Amalgame_Compiler_Parser* self);
static code_bool Amalgame_Compiler_Parser_IsLambdaParenStart(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseLambdaSingle(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseLambdaMulti(Amalgame_Compiler_Parser* self);
static void Amalgame_Compiler_Parser_ParseLambdaBody(Amalgame_Compiler_Parser* self, Amalgame_Compiler_AstNode* lam);
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
static code_bool Amalgame_Compiler_Parser_LookaheadStartsWithDot(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseCallArgs(Amalgame_Compiler_Parser* self, Amalgame_Compiler_AstNode* callee);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParsePrimary(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseListComp(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseMatch(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseMatchPattern(Amalgame_Compiler_Parser* self);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseNew(Amalgame_Compiler_Parser* self);

Amalgame_Compiler_Parser* Amalgame_Compiler_Parser_new(AmalgameList* tokens) {
    Amalgame_Compiler_Parser* self = (Amalgame_Compiler_Parser*) GC_MALLOC(sizeof(Amalgame_Compiler_Parser));
    AmalgameList* __attribute__((unused)) filtered = AmalgameList_new();
    AmalgameList* __attribute__((unused)) comments = AmalgameList_new();
    i64 __attribute__((unused)) total = AmalgameList_count(tokens);
    i64 __attribute__((unused)) i = 0;
    while (i < total) {
        Amalgame_Compiler_Token* __attribute__((unused)) t = (void*)AmalgameList_get(tokens, i);
        if (t->Type == Amalgame_Compiler_TokenType_COMMENT) {
            AmalgameList_add(comments, (void*)(intptr_t)(t));
        } else {
            AmalgameList_add(filtered, (void*)(intptr_t)(t));
        }
        i = i + 1;
    }
    self->Tokens = filtered;
    self->TokenCount = AmalgameList_count(filtered);
    self->Pos = 0;
    self->Errors = AmalgameList_new();
    self->ParenDepth = 0;
    self->PendingDecorators = "";
    self->Comments = comments;
    return self;
}

static void Amalgame_Compiler_Parser_ParseDecoratorList(Amalgame_Compiler_Parser* self) {
    (void)self;
    while (Amalgame_Compiler_Parser_CheckValue(self, "@")) {
        Amalgame_Compiler_Parser_Advance(self);
        if (Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_IDENTIFIER)) {
            Amalgame_Compiler_Token* __attribute__((unused)) nameTok = Amalgame_Compiler_Parser_Advance(self);
            if (String_Length(self->PendingDecorators) > 0) {
                self->PendingDecorators = code_string_concat(self->PendingDecorators, ",");
            }
            self->PendingDecorators = code_string_concat(self->PendingDecorators, nameTok->Value);
        }
        Amalgame_Compiler_Parser_SkipNewlines(self);
    }
}

static code_string Amalgame_Compiler_Parser_TakeDecorators(Amalgame_Compiler_Parser* self) {
    (void)self;
    code_string __attribute__((unused)) d = self->PendingDecorators;
    self->PendingDecorators = "";
    return d;
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
        Amalgame_Compiler_Token* __attribute__((unused)) importTok = Amalgame_Compiler_Parser_Advance(self);
        code_string __attribute__((unused)) qname = Amalgame_Compiler_Parser_ParseQualifiedName(self);
        Amalgame_Compiler_AstNode* __attribute__((unused)) imp = Amalgame_Compiler_Ast_Ident(qname, importTok->Line, importTok->Column);
        AmalgameList_add(prog->Args, (void*)(intptr_t)(imp));
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
        code_string __attribute__((unused)) gparams = "";
        while (!Amalgame_Compiler_Parser_IsEnd(self) && !Amalgame_Compiler_Parser_CheckValue(self, ">")) {
            if (Amalgame_Compiler_Parser_CheckValue(self, ",")) {
                Amalgame_Compiler_Parser_Advance(self);
                continue;
            }
            if (Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_IDENTIFIER)) {
                Amalgame_Compiler_Token* __attribute__((unused)) pTok = Amalgame_Compiler_Parser_Advance(self);
                if (String_Length(gparams) > 0) {
                    gparams = code_string_concat(gparams, ",");
                }
                gparams = code_string_concat(gparams, pTok->Value);
            } else {
                Amalgame_Compiler_Parser_Advance(self);
            }
        }
        if (Amalgame_Compiler_Parser_CheckValue(self, ">")) {
            Amalgame_Compiler_Parser_Advance(self);
        }
        cls->Str3 = gparams;
    }
    if (Amalgame_Compiler_Parser_CheckKw(self, "extends")) {
        Amalgame_Compiler_Parser_Advance(self);
        cls->Str = Amalgame_Compiler_Parser_ParseQualifiedName(self);
    }
    if (Amalgame_Compiler_Parser_CheckKw(self, "implements")) {
        Amalgame_Compiler_Parser_Advance(self);
        code_string __attribute__((unused)) impl = "";
        while (!Amalgame_Compiler_Parser_IsEnd(self) && !Amalgame_Compiler_Parser_CheckValue(self, "{") && !Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_NEWLINE)) {
            if (Amalgame_Compiler_Parser_CheckValue(self, ",")) {
                Amalgame_Compiler_Parser_Advance(self);
                continue;
            }
            code_string __attribute__((unused)) iname = Amalgame_Compiler_Parser_ParseTypeName(self);
            if (String_Length(iname) > 0) {
                if (String_Length(impl) > 0) {
                    impl = code_string_concat(impl, ",");
                }
                impl = code_string_concat(impl, iname);
            }
        }
        cls->Str4 = impl;
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
    Amalgame_Compiler_Token* __attribute__((unused)) clsClose = Amalgame_Compiler_Parser_Current(self);
    cls->Str2 = String_FromInt(clsClose->Line);
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
    Amalgame_Compiler_Parser_ParseDecoratorList(self);
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
    method->Str2 = Amalgame_Compiler_Parser_TakeDecorators(self);
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
        i64 __attribute__((unused)) beforePos = self->Pos;
        Amalgame_Compiler_AstNode* __attribute__((unused)) p = Amalgame_Compiler_Parser_ParseParam(self);
        if (self->Pos == beforePos) {
            Amalgame_Compiler_Parser_Advance(self);
        }
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
        Amalgame_Compiler_Parser_Advance(self);
        return Amalgame_Compiler_Parser_Unknown(self);
    }
    code_string __attribute__((unused)) typeName = Amalgame_Compiler_Parser_ParseTypeName(self);
    if (Amalgame_Compiler_Parser_CheckValue(self, ":")) {
        Amalgame_Compiler_Parser_Advance(self);
        Amalgame_Compiler_Token* __attribute__((unused)) realTypeTok = Amalgame_Compiler_Parser_Current(self);
        code_string __attribute__((unused)) rv = realTypeTok->Value;
        code_bool __attribute__((unused)) rIsKeywordType = code_string_equals(rv, "int") || code_string_equals(rv, "string") || code_string_equals(rv, "bool") || code_string_equals(rv, "void") || code_string_equals(rv, "float");
        if (Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_IDENTIFIER) || rIsKeywordType) {
            code_string __attribute__((unused)) realType = Amalgame_Compiler_Parser_ParseTypeName(self);
            return Amalgame_Compiler_Ast_Param(typeName, realType, tok->Line, tok->Column);
        }
        return Amalgame_Compiler_Ast_Param(typeName, "?", tok->Line, tok->Column);
    }
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
    Amalgame_Compiler_Token* __attribute__((unused)) closeTok = Amalgame_Compiler_Parser_Current(self);
    block->Str2 = String_FromInt(closeTok->Line);
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
    if (code_string_equals(v, "guard")) {
        return Amalgame_Compiler_Parser_ParseGuard(self);
    }
    if (code_string_equals(v, "try")) {
        return Amalgame_Compiler_Parser_ParseTry(self);
    }
    if (code_string_equals(v, "throw")) {
        return Amalgame_Compiler_Parser_ParseThrow(self);
    }
    if (code_string_equals(v, "{")) {
        return Amalgame_Compiler_Parser_ParseBlock(self);
    }
    return Amalgame_Compiler_Parser_ParseExpr(self);
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseTry(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Advance(self);
    Amalgame_Compiler_Parser_SkipNewlines(self);
    Amalgame_Compiler_AstNode* __attribute__((unused)) tryBlock = Amalgame_Compiler_Parser_ParseBlock(self);
    Amalgame_Compiler_Parser_SkipNewlines(self);
    Amalgame_Compiler_AstNode* __attribute__((unused)) node = Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_TRY_STMT, tok->Line, tok->Column);
    node->Body = tryBlock;
    if (Amalgame_Compiler_Parser_CheckKw(self, "catch")) {
        Amalgame_Compiler_Parser_Advance(self);
        if (Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_IDENTIFIER)) {
            Amalgame_Compiler_Token* __attribute__((unused)) bind = Amalgame_Compiler_Parser_Advance(self);
            node->Name = bind->Value;
        }
        Amalgame_Compiler_Parser_SkipNewlines(self);
        node->Else = Amalgame_Compiler_Parser_ParseBlock(self);
        Amalgame_Compiler_Parser_SkipNewlines(self);
    }
    if (Amalgame_Compiler_Parser_CheckKw(self, "finally")) {
        Amalgame_Compiler_Parser_Advance(self);
        Amalgame_Compiler_Parser_SkipNewlines(self);
        node->Cond = Amalgame_Compiler_Parser_ParseBlock(self);
    }
    return node;
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseThrow(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Advance(self);
    Amalgame_Compiler_AstNode* __attribute__((unused)) n = Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_THROW_STMT, tok->Line, tok->Column);
    if (!Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_NEWLINE) && !Amalgame_Compiler_Parser_CheckValue(self, "}") && !Amalgame_Compiler_Parser_IsEnd(self)) {
        n->Left = Amalgame_Compiler_Parser_ParseExpr(self);
    }
    return n;
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
            code_string __attribute__((unused)) vname = (code_string)AmalgameList_get(names, i);
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

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseGuard(Amalgame_Compiler_Parser* self) {
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
    if (Amalgame_Compiler_Parser_CheckKw(self, "else")) {
        Amalgame_Compiler_Parser_Advance(self);
    }
    Amalgame_Compiler_Parser_SkipNewlines(self);
    Amalgame_Compiler_AstNode* __attribute__((unused)) body = Amalgame_Compiler_Parser_ParseBlock(self);
    Amalgame_Compiler_AstNode* __attribute__((unused)) neg = Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_UNARY, tok->Line, tok->Column);
    neg->Str = "!";
    neg->Left = cond;
    return Amalgame_Compiler_Ast_If(neg, body, tok->Line, tok->Column);
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
    if (Amalgame_Compiler_Parser_CheckValue(self, "<")) {
        Amalgame_Compiler_Parser_Advance(self);
        code_string __attribute__((unused)) gparams = "";
        while (!Amalgame_Compiler_Parser_IsEnd(self) && !Amalgame_Compiler_Parser_CheckValue(self, ">")) {
            if (Amalgame_Compiler_Parser_CheckValue(self, ",")) {
                Amalgame_Compiler_Parser_Advance(self);
                continue;
            }
            if (Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_IDENTIFIER)) {
                Amalgame_Compiler_Token* __attribute__((unused)) pTok = Amalgame_Compiler_Parser_Advance(self);
                if (String_Length(gparams) > 0) {
                    gparams = code_string_concat(gparams, ",");
                }
                gparams = code_string_concat(gparams, pTok->Value);
            } else {
                Amalgame_Compiler_Parser_Advance(self);
            }
        }
        if (Amalgame_Compiler_Parser_CheckValue(self, ">")) {
            Amalgame_Compiler_Parser_Advance(self);
        }
        iface->Str3 = gparams;
    }
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
            i64 __attribute__((unused)) beforePos = self->Pos;
            Amalgame_Compiler_AstNode* __attribute__((unused)) p = Amalgame_Compiler_Parser_ParseParam(self);
            if (self->Pos == beforePos) {
                Amalgame_Compiler_Parser_Advance(self);
            }
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
        return Amalgame_Compiler_Parser_ParseLambdaSingle(self);
    }
    if (Amalgame_Compiler_Parser_CheckValue(self, "(") && Amalgame_Compiler_Parser_IsLambdaParenStart(self)) {
        return Amalgame_Compiler_Parser_ParseLambdaMulti(self);
    }
    return Amalgame_Compiler_Parser_ParseAssign(self);
}

static code_bool Amalgame_Compiler_Parser_IsLambdaParenStart(Amalgame_Compiler_Parser* self) {
    (void)self;
    i64 __attribute__((unused)) i = 1;
    if (code_string_equals(Amalgame_Compiler_Parser_Peek(self, i)->Value, ")")) {
        return code_string_equals(Amalgame_Compiler_Parser_Peek(self, i + 1)->Value, "=>");
    }
    while (1) {
        if (Amalgame_Compiler_Parser_Peek(self, i)->Type != Amalgame_Compiler_TokenType_IDENTIFIER) {
            return 0;
        }
        i = i + 1;
        if (code_string_equals(Amalgame_Compiler_Parser_Peek(self, i)->Value, ")")) {
            return code_string_equals(Amalgame_Compiler_Parser_Peek(self, i + 1)->Value, "=>");
        }
        if (!code_string_equals(Amalgame_Compiler_Parser_Peek(self, i)->Value, ",")) {
            return 0;
        }
        i = i + 1;
    }
    return 0;
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseLambdaSingle(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_Token* __attribute__((unused)) paramTok = Amalgame_Compiler_Parser_Advance(self);
    Amalgame_Compiler_Parser_Advance(self);
    Amalgame_Compiler_AstNode* __attribute__((unused)) lam = Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_METHOD_DECL, paramTok->Line, paramTok->Column);
    lam->Name = "__lambda__";
    Amalgame_Compiler_AstNode* __attribute__((unused)) p = Amalgame_Compiler_Ast_Param(paramTok->Value, "?", paramTok->Line, paramTok->Column);
    AmalgameList_add(lam->Params, (void*)(intptr_t)(p));
    Amalgame_Compiler_Parser_ParseLambdaBody(self, lam);
    return lam;
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseLambdaMulti(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_Token* __attribute__((unused)) openTok = Amalgame_Compiler_Parser_Advance(self);
    Amalgame_Compiler_AstNode* __attribute__((unused)) lam = Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_METHOD_DECL, openTok->Line, openTok->Column);
    lam->Name = "__lambda__";
    while (!Amalgame_Compiler_Parser_IsEnd(self) && !Amalgame_Compiler_Parser_CheckValue(self, ")")) {
        if (Amalgame_Compiler_Parser_CheckValue(self, ",")) {
            Amalgame_Compiler_Parser_Advance(self);
            continue;
        }
        Amalgame_Compiler_Token* __attribute__((unused)) pTok = Amalgame_Compiler_Parser_ExpectIdent(self);
        Amalgame_Compiler_AstNode* __attribute__((unused)) p = Amalgame_Compiler_Ast_Param(pTok->Value, "?", pTok->Line, pTok->Column);
        AmalgameList_add(lam->Params, (void*)(intptr_t)(p));
    }
    Amalgame_Compiler_Parser_Expect(self, ")");
    Amalgame_Compiler_Parser_Advance(self);
    Amalgame_Compiler_Parser_ParseLambdaBody(self, lam);
    return lam;
}

static void Amalgame_Compiler_Parser_ParseLambdaBody(Amalgame_Compiler_Parser* self, Amalgame_Compiler_AstNode* lam) {
    (void)self;
    (void)lam;
    if (Amalgame_Compiler_Parser_CheckValue(self, "{")) {
        lam->Body = Amalgame_Compiler_Parser_ParseBlock(self);
    } else {
        lam->Left = Amalgame_Compiler_Parser_ParseExpr(self);
    }
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
        if (Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_NEWLINE) && Amalgame_Compiler_Parser_LookaheadStartsWithDot(self)) {
            Amalgame_Compiler_Parser_SkipNewlines(self);
        }
        if (Amalgame_Compiler_Parser_CheckValue(self, ".")) {
            Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Advance(self);
            Amalgame_Compiler_Token* __attribute__((unused)) memberTok = Amalgame_Compiler_Parser_ExpectIdent(self);
            expr = Amalgame_Compiler_Ast_Member(expr, memberTok->Value, tok->Line, tok->Column);
            if (Amalgame_Compiler_Parser_CheckValue(self, "(")) {
                expr = Amalgame_Compiler_Parser_ParseCallArgs(self, expr);
            }
        } else if (Amalgame_Compiler_Parser_CheckValue(self, "?.")) {
            Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Advance(self);
            Amalgame_Compiler_Token* __attribute__((unused)) memberTok = Amalgame_Compiler_Parser_ExpectIdent(self);
            Amalgame_Compiler_AstNode* __attribute__((unused)) m = Amalgame_Compiler_Ast_Member(expr, memberTok->Value, tok->Line, tok->Column);
            m->Flag = 1;
            expr = m;
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

static code_bool Amalgame_Compiler_Parser_LookaheadStartsWithDot(Amalgame_Compiler_Parser* self) {
    (void)self;
    i64 __attribute__((unused)) off = 0;
    i64 __attribute__((unused)) max = self->TokenCount;
    while (self->Pos + off < max) {
        Amalgame_Compiler_Token* __attribute__((unused)) t = (Amalgame_Compiler_Token*)AmalgameList_get(self->Tokens, self->Pos + off);
        if (t->Type == Amalgame_Compiler_TokenType_NEWLINE) {
            off = off + 1;
            continue;
        }
        if (code_string_equals(t->Value, ".") || code_string_equals(t->Value, "?.")) {
            return 1;
        }
        return 0;
    }
    return 0;
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
        code_string __attribute__((unused)) namedKey = "";
        if (Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_IDENTIFIER)) {
            Amalgame_Compiler_Token* __attribute__((unused)) peek = Amalgame_Compiler_Parser_Peek(self, 1);
            if (code_string_equals(peek->Value, ":")) {
                Amalgame_Compiler_Token* __attribute__((unused)) nameTok = Amalgame_Compiler_Parser_Advance(self);
                Amalgame_Compiler_Parser_Advance(self);
                namedKey = nameTok->Value;
            }
        }
        Amalgame_Compiler_AstNode* __attribute__((unused)) arg = Amalgame_Compiler_Parser_ParseExpr(self);
        if (String_Length(namedKey) > 0) {
            arg->Str2 = namedKey;
        }
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
    if (code_string_equals(v, "match")) {
        return Amalgame_Compiler_Parser_ParseMatch(self);
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
    if (Amalgame_Compiler_Parser_CheckValue(self, "[")) {
        return Amalgame_Compiler_Parser_ParseListComp(self);
    }
    Amalgame_Compiler_Parser_Advance(self);
    return Amalgame_Compiler_Parser_Unknown(self);
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_Parser_ParseListComp(Amalgame_Compiler_Parser* self) {
    (void)self;
    Amalgame_Compiler_Token* __attribute__((unused)) tok = Amalgame_Compiler_Parser_Advance(self);
    Amalgame_Compiler_AstNode* __attribute__((unused)) projection = Amalgame_Compiler_Parser_ParseExpr(self);
    Amalgame_Compiler_Parser_SkipNewlines(self);
    if (!Amalgame_Compiler_Parser_CheckKw(self, "for")) {
        AmalgameList_add(self->Errors, (void*)(intptr_t)(code_string_concat(code_string_concat(code_string_concat("Expected 'for' in list comprehension at ", String_FromInt(tok->Line)), ":"), String_FromInt(tok->Column))));
        while (!Amalgame_Compiler_Parser_IsEnd(self) && !Amalgame_Compiler_Parser_CheckValue(self, "]")) {
            Amalgame_Compiler_Parser_Advance(self);
        }
        if (Amalgame_Compiler_Parser_CheckValue(self, "]")) {
            Amalgame_Compiler_Parser_Advance(self);
        }
        return Amalgame_Compiler_Parser_Unknown(self);
    }
    Amalgame_Compiler_Parser_Advance(self);
    Amalgame_Compiler_Token* __attribute__((unused)) varTok = Amalgame_Compiler_Parser_ExpectIdent(self);
    if (!Amalgame_Compiler_Parser_CheckKw(self, "in")) {
        AmalgameList_add(self->Errors, (void*)(intptr_t)(code_string_concat(code_string_concat(code_string_concat("Expected 'in' in list comprehension at ", String_FromInt(varTok->Line)), ":"), String_FromInt(varTok->Column))));
    } else {
        Amalgame_Compiler_Parser_Advance(self);
    }
    Amalgame_Compiler_AstNode* __attribute__((unused)) iter = Amalgame_Compiler_Parser_ParseExpr(self);
    Amalgame_Compiler_AstNode* __attribute__((unused)) node = Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_LIST_COMP, tok->Line, tok->Column);
    node->Left = projection;
    node->Str = varTok->Value;
    node->Right = iter;
    Amalgame_Compiler_Parser_SkipNewlines(self);
    if (Amalgame_Compiler_Parser_CheckKw(self, "if")) {
        Amalgame_Compiler_Parser_Advance(self);
        node->Cond = Amalgame_Compiler_Parser_ParseExpr(self);
        Amalgame_Compiler_Parser_SkipNewlines(self);
    }
    Amalgame_Compiler_Parser_Expect(self, "]");
    return node;
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
        code_bool __attribute__((unused)) hasGuard = Amalgame_Compiler_Parser_CheckKw(self, "if");
        Amalgame_Compiler_AstNode* __attribute__((unused)) arm = Amalgame_Compiler_Ast_Binary(patNode, "=>", patNode, tok->Line, tok->Column);
        if (hasGuard) {
            Amalgame_Compiler_Parser_Advance(self);
            arm->Cond = Amalgame_Compiler_Parser_ParseExpr(self);
            Amalgame_Compiler_Parser_SkipNewlines(self);
        }
        if (Amalgame_Compiler_Parser_CheckValue(self, "=>")) {
            Amalgame_Compiler_Parser_Advance(self);
        }
        Amalgame_Compiler_Parser_SkipNewlines(self);
        Amalgame_Compiler_AstNode* __attribute__((unused)) armBody = Amalgame_Compiler_Parser_ParseStmt(self);
        if (Amalgame_Compiler_Parser_CheckValue(self, ",")) {
            Amalgame_Compiler_Parser_Advance(self);
        }
        arm->Right = armBody;
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
        if (Amalgame_Compiler_Parser_CheckValue(self, ".")) {
            Amalgame_Compiler_Parser_Advance(self);
            if (Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_IDENTIFIER)) {
                Amalgame_Compiler_Token* __attribute__((unused)) memberTok = Amalgame_Compiler_Parser_Advance(self);
                patIdent = Amalgame_Compiler_Ast_Member(patIdent, memberTok->Value, nameTok->Line, nameTok->Column);
            }
        }
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
    code_string __attribute__((unused)) generic = "";
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
            if (depth > 0) {
                generic = code_string_concat(generic, iv);
            }
            Amalgame_Compiler_Parser_Advance(self);
        }
    }
    Amalgame_Compiler_AstNode* __attribute__((unused)) node = Amalgame_Compiler_Ast_NewExpr(typeName, tok->Line, tok->Column);
    if (String_Length(generic) > 0) {
        node->Str2 = generic;
    }
    if (Amalgame_Compiler_Parser_CheckValue(self, "(")) {
        Amalgame_Compiler_Parser_Advance(self);
        Amalgame_Compiler_Parser_SkipNewlines(self);
        while (!Amalgame_Compiler_Parser_IsEnd(self) && !Amalgame_Compiler_Parser_CheckValue(self, ")")) {
            if (Amalgame_Compiler_Parser_CheckValue(self, ",")) {
                Amalgame_Compiler_Parser_Advance(self);
                Amalgame_Compiler_Parser_SkipNewlines(self);
                continue;
            }
            code_string __attribute__((unused)) namedKey = "";
            if (Amalgame_Compiler_Parser_CheckType(self, Amalgame_Compiler_TokenType_IDENTIFIER)) {
                Amalgame_Compiler_Token* __attribute__((unused)) peek = Amalgame_Compiler_Parser_Peek(self, 1);
                if (code_string_equals(peek->Value, ":")) {
                    Amalgame_Compiler_Token* __attribute__((unused)) nameTok = Amalgame_Compiler_Parser_Advance(self);
                    Amalgame_Compiler_Parser_Advance(self);
                    namedKey = nameTok->Value;
                }
            }
            Amalgame_Compiler_AstNode* __attribute__((unused)) arg = Amalgame_Compiler_Parser_ParseExpr(self);
            if (String_Length(namedKey) > 0) {
                arg->Str2 = namedKey;
            }
            AmalgameList_add(node->Args, (void*)(intptr_t)(arg));
            Amalgame_Compiler_Parser_SkipNewlines(self);
        }
        Amalgame_Compiler_Parser_Expect(self, ")");
    }
    return node;
}

struct _Amalgame_Compiler_Emitter {
    AmalgameList* Lines;
    i64 Indent;
    code_bool Streaming;
};

void Amalgame_Compiler_Emitter_SetStreaming(Amalgame_Compiler_Emitter* self, code_bool v);
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
    self->Streaming = 0;
    return self;
}

void Amalgame_Compiler_Emitter_SetStreaming(Amalgame_Compiler_Emitter* self, code_bool v) {
    (void)self;
    (void)v;
    self->Streaming = v;
}

void Amalgame_Compiler_Emitter_Emit(Amalgame_Compiler_Emitter* self, code_string text) {
    (void)self;
    (void)text;
    if (self->Streaming) {
        File_StreamLine(text);
    } else {
        AmalgameList_add(self->Lines, (void*)(intptr_t)(text));
    }
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
    if (self->Streaming) {
        File_StreamLine(line);
    } else {
        AmalgameList_add(self->Lines, (void*)(intptr_t)(line));
    }
}

void Amalgame_Compiler_Emitter_EmitBlank(Amalgame_Compiler_Emitter* self) {
    (void)self;
    if (self->Streaming) {
        File_StreamLine("");
    } else {
        AmalgameList_add(self->Lines, (void*)(intptr_t)(""));
    }
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
    AmalgameList* MethodRetRawNames;
    AmalgameList* MethodRetRawTypes;
    i64 LambdaCounter;
    code_bool InLambdaBody;
};

code_string Amalgame_Compiler_CGen_Generate(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* prog);
void Amalgame_Compiler_CGen_SetStreaming(Amalgame_Compiler_CGen* self, code_bool v);
void Amalgame_Compiler_CGen_BeginMulti(Amalgame_Compiler_CGen* self, code_string ns);
void Amalgame_Compiler_CGen_AddFilePass1(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* prog);
void Amalgame_Compiler_CGen_EmitSeparator(Amalgame_Compiler_CGen* self);
void Amalgame_Compiler_CGen_AddFilePass2(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* prog);
static void Amalgame_Compiler_CGen_EmitLambdaForwards(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* node);
static void Amalgame_Compiler_CGen_EmitLambdaBodies(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* node);
static void Amalgame_Compiler_CGen_EmitOneLambdaForward(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* lam);
static void Amalgame_Compiler_CGen_EmitOneLambdaBody(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* lam);
code_string Amalgame_Compiler_CGen_FinishMulti(Amalgame_Compiler_CGen* self);
AmalgameList* Amalgame_Compiler_CGen_GetLines(Amalgame_Compiler_CGen* self);
static code_string Amalgame_Compiler_CGen_SymName(Amalgame_Compiler_CGen* self, code_string name);
static void Amalgame_Compiler_CGen_LocalTypeSet(Amalgame_Compiler_CGen* self, code_string varName, code_string ctype);
static code_string Amalgame_Compiler_CGen_LocalTypeGet(Amalgame_Compiler_CGen* self, code_string varName);
static void Amalgame_Compiler_CGen_LocalTypeClear(Amalgame_Compiler_CGen* self);
static void Amalgame_Compiler_CGen_FieldTypeSet(Amalgame_Compiler_CGen* self, code_string className, code_string fieldName, code_string ctype);
static code_string Amalgame_Compiler_CGen_FieldTypeGet(Amalgame_Compiler_CGen* self, code_string className, code_string fieldName);
static void Amalgame_Compiler_CGen_TrackMapResultElem(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* stmt);
static void Amalgame_Compiler_CGen_TrackGenericLocal(Amalgame_Compiler_CGen* self, code_string varName, code_string rawType);
static void Amalgame_Compiler_CGen_ListElemSet(Amalgame_Compiler_CGen* self, code_string className, code_string fieldName, code_string elemCType);
static code_string Amalgame_Compiler_CGen_ListElemGet(Amalgame_Compiler_CGen* self, code_string className, code_string fieldName);
static void Amalgame_Compiler_CGen_MethodRetSet(Amalgame_Compiler_CGen* self, code_string className, code_string methodName, code_string ctype);
static void Amalgame_Compiler_CGen_MethodRetRawSet(Amalgame_Compiler_CGen* self, code_string className, code_string methodName, code_string rawType);
static code_string Amalgame_Compiler_CGen_MethodRetRawGet(Amalgame_Compiler_CGen* self, code_string className, code_string methodName);
static code_string Amalgame_Compiler_CGen_MethodRetGet(Amalgame_Compiler_CGen* self, code_string className, code_string methodName);
static code_string Amalgame_Compiler_CGen_InferTypeFromExpr(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* expr);
static code_string Amalgame_Compiler_CGen_EmitInterpolatedString(Amalgame_Compiler_CGen* self, code_string raw);
static code_string Amalgame_Compiler_CGen_WrapForInterp(Amalgame_Compiler_CGen* self, code_string cExpr, code_string cType);
static code_string Amalgame_Compiler_CGen_BuiltinCallReturnType(Amalgame_Compiler_CGen* self, code_string cName);
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
static code_string Amalgame_Compiler_CGen_EmitMatchExpr(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* n);
static code_string Amalgame_Compiler_CGen_EmitListComp(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* n);
static code_string Amalgame_Compiler_CGen_TryEmitListCall(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* callExpr);
static code_string Amalgame_Compiler_CGen_EmitClosureArg(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* arg);
static code_string Amalgame_Compiler_CGen_EmitLambdaAsClosure(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* lam);
static code_string Amalgame_Compiler_CGen_EmitLambdaCaptureCopy(Amalgame_Compiler_CGen* self, code_string envVar, Amalgame_Compiler_AstNode* cap);
static code_string Amalgame_Compiler_CGen_EmitCalleeStr(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* callee);
static code_bool Amalgame_Compiler_CGen_IsEnum(Amalgame_Compiler_CGen* self, code_string t);
static code_bool Amalgame_Compiler_CGen_IsCPointerType(Amalgame_Compiler_CGen* self, code_string ct);
static code_string Amalgame_Compiler_CGen_BoxAsVoid(Amalgame_Compiler_CGen* self, code_string expr);
static code_string Amalgame_Compiler_CGen_UnboxScalar(Amalgame_Compiler_CGen* self, code_string ctype, code_string expr);
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
    self->MethodRetRawNames = AmalgameList_new();
    self->MethodRetRawTypes = AmalgameList_new();
    self->LambdaCounter = 0;
    self->InLambdaBody = 0;
    Amalgame_Compiler_CGen_FieldTypeSet(self, "AmalgameHttpResponse", "Status", "i64");
    Amalgame_Compiler_CGen_FieldTypeSet(self, "AmalgameHttpResponse", "Body", "code_string");
    Amalgame_Compiler_CGen_FieldTypeSet(self, "AmalgameHttpResponse", "Error", "code_string");
    Amalgame_Compiler_CGen_FieldTypeSet(self, "AmalgameHttpResponse", "Ok", "code_bool");
    Amalgame_Compiler_CGen_FieldTypeSet(self, "AmalgameTcpConn", "RemoteIp", "code_string");
    Amalgame_Compiler_CGen_FieldTypeSet(self, "AmalgameTcpConn", "Fd", "i64");
    Amalgame_Compiler_CGen_FieldTypeSet(self, "AmalgameTcpServer", "Fd", "i64");
    Amalgame_Compiler_CGen_FieldTypeSet(self, "AmalgameTcpServer", "Port", "i64");
    Amalgame_Compiler_CGen_FieldTypeSet(self, "AmalgameProcessResult", "Exit", "i64");
    Amalgame_Compiler_CGen_FieldTypeSet(self, "AmalgameProcessResult", "Stdout", "code_string");
    Amalgame_Compiler_CGen_FieldTypeSet(self, "AmalgameProcessResult", "Stderr", "code_string");
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

void Amalgame_Compiler_CGen_SetStreaming(Amalgame_Compiler_CGen* self, code_bool v) {
    (void)self;
    (void)v;
    Amalgame_Compiler_Emitter_SetStreaming(self->Out, v);
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
    Amalgame_Compiler_CGen_EmitLambdaForwards(self, prog);
    i64 __attribute__((unused)) decls = AmalgameList_count(prog->Children);
    for (i64 j = 0; j < decls; j++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) decl = (Amalgame_Compiler_AstNode*)AmalgameList_get(prog->Children, j);
        Amalgame_Compiler_CGen_EmitDecl(self, decl);
    }
    Amalgame_Compiler_CGen_EmitLambdaBodies(self, prog);
}

static void Amalgame_Compiler_CGen_EmitLambdaForwards(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* node) {
    (void)self;
    (void)node;
    if (node == NULL) {
        return;
    }
    if (node->Kind == Amalgame_Compiler_NodeKind_METHOD_DECL && code_string_equals(node->Name, "__lambda__")) {
        Amalgame_Compiler_CGen_EmitOneLambdaForward(self, node);
    }
    if (node->Left != NULL) {
        Amalgame_Compiler_CGen_EmitLambdaForwards(self, node->Left);
    }
    if (node->Right != NULL) {
        Amalgame_Compiler_CGen_EmitLambdaForwards(self, node->Right);
    }
    if (node->Cond != NULL) {
        Amalgame_Compiler_CGen_EmitLambdaForwards(self, node->Cond);
    }
    if (node->Body != NULL) {
        Amalgame_Compiler_CGen_EmitLambdaForwards(self, node->Body);
    }
    if (node->Else != NULL) {
        Amalgame_Compiler_CGen_EmitLambdaForwards(self, node->Else);
    }
    i64 __attribute__((unused)) cn = AmalgameList_count(node->Children);
    for (i64 i = 0; i < cn; i++) {
        Amalgame_Compiler_CGen_EmitLambdaForwards(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(node->Children, i));
    }
    i64 __attribute__((unused)) pn = AmalgameList_count(node->Params);
    for (i64 j = 0; j < pn; j++) {
        Amalgame_Compiler_CGen_EmitLambdaForwards(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(node->Params, j));
    }
    i64 __attribute__((unused)) an = AmalgameList_count(node->Args);
    for (i64 k = 0; k < an; k++) {
        Amalgame_Compiler_CGen_EmitLambdaForwards(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(node->Args, k));
    }
}

static void Amalgame_Compiler_CGen_EmitLambdaBodies(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* node) {
    (void)self;
    (void)node;
    if (node == NULL) {
        return;
    }
    if (node->Kind == Amalgame_Compiler_NodeKind_METHOD_DECL && code_string_equals(node->Name, "__lambda__")) {
        Amalgame_Compiler_CGen_EmitOneLambdaBody(self, node);
    }
    if (node->Left != NULL) {
        Amalgame_Compiler_CGen_EmitLambdaBodies(self, node->Left);
    }
    if (node->Right != NULL) {
        Amalgame_Compiler_CGen_EmitLambdaBodies(self, node->Right);
    }
    if (node->Cond != NULL) {
        Amalgame_Compiler_CGen_EmitLambdaBodies(self, node->Cond);
    }
    if (node->Body != NULL) {
        Amalgame_Compiler_CGen_EmitLambdaBodies(self, node->Body);
    }
    if (node->Else != NULL) {
        Amalgame_Compiler_CGen_EmitLambdaBodies(self, node->Else);
    }
    i64 __attribute__((unused)) cn = AmalgameList_count(node->Children);
    for (i64 i = 0; i < cn; i++) {
        Amalgame_Compiler_CGen_EmitLambdaBodies(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(node->Children, i));
    }
    i64 __attribute__((unused)) pn = AmalgameList_count(node->Params);
    for (i64 j = 0; j < pn; j++) {
        Amalgame_Compiler_CGen_EmitLambdaBodies(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(node->Params, j));
    }
    i64 __attribute__((unused)) an = AmalgameList_count(node->Args);
    for (i64 k = 0; k < an; k++) {
        Amalgame_Compiler_CGen_EmitLambdaBodies(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(node->Args, k));
    }
}

static void Amalgame_Compiler_CGen_EmitOneLambdaForward(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* lam) {
    (void)self;
    (void)lam;
    i64 __attribute__((unused)) id = self->LambdaCounter;
    self->LambdaCounter = id + 1;
    code_string __attribute__((unused)) idStr = String_FromInt(id);
    lam->Str2 = idStr;
    code_string __attribute__((unused)) envName = code_string_concat("LamEnv_", idStr);
    code_string __attribute__((unused)) fnName = code_string_concat(code_string_concat("lam_", idStr), "_fn");
    i64 __attribute__((unused)) cn = AmalgameList_count(lam->Args);
    i64 __attribute__((unused)) pn = AmalgameList_count(lam->Params);
    Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat("typedef struct ", envName), " {"));
    Amalgame_Compiler_Emitter_Indent_(self->Out);
    if (cn == 0) {
        Amalgame_Compiler_Emitter_EmitLine(self->Out, "char _empty;");
    } else {
        for (i64 i = 0; i < cn; i++) {
            Amalgame_Compiler_AstNode* __attribute__((unused)) cap = (Amalgame_Compiler_AstNode*)AmalgameList_get(lam->Args, i);
            code_string __attribute__((unused)) ct = Amalgame_Compiler_CGen_TypeToC(self, cap->Str);
            if (String_Length(ct) == 0 || code_string_equals(ct, "void")) {
                Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat("void* _", cap->Name), ";"));
            } else {
                Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(ct, " _"), cap->Name), ";"));
            }
        }
    }
    Amalgame_Compiler_Emitter_Dedent(self->Out);
    Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat("} ", envName), ";"));
    code_string __attribute__((unused)) fwdLine = code_string_concat(code_string_concat("static void* ", fnName), "(void* __envRaw");
    for (i64 pi = 0; pi < pn; pi++) {
        fwdLine = code_string_concat(code_string_concat(fwdLine, ", void* __arg"), String_FromInt(pi));
    }
    fwdLine = code_string_concat(fwdLine, ");");
    Amalgame_Compiler_Emitter_EmitLine(self->Out, fwdLine);
}

static void Amalgame_Compiler_CGen_EmitOneLambdaBody(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* lam) {
    (void)self;
    (void)lam;
    code_string __attribute__((unused)) idStr = lam->Str2;
    code_string __attribute__((unused)) envName = code_string_concat("LamEnv_", idStr);
    code_string __attribute__((unused)) fnName = code_string_concat(code_string_concat("lam_", idStr), "_fn");
    i64 __attribute__((unused)) cn = AmalgameList_count(lam->Args);
    i64 __attribute__((unused)) pn = AmalgameList_count(lam->Params);
    code_string __attribute__((unused)) sigLine = code_string_concat(code_string_concat("static void* ", fnName), "(void* __envRaw");
    for (i64 pi = 0; pi < pn; pi++) {
        sigLine = code_string_concat(code_string_concat(sigLine, ", void* __arg"), String_FromInt(pi));
    }
    sigLine = code_string_concat(sigLine, ") {");
    Amalgame_Compiler_Emitter_EmitLine(self->Out, sigLine);
    Amalgame_Compiler_Emitter_Indent_(self->Out);
    Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(envName, "* __env = ("), envName), "*)__envRaw;"));
    Amalgame_Compiler_Emitter_EmitLine(self->Out, "(void)__env;");
    for (i64 pi = 0; pi < pn; pi++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) p = (Amalgame_Compiler_AstNode*)AmalgameList_get(lam->Params, pi);
        code_string __attribute__((unused)) pTypeRaw = p->Str;
        code_string __attribute__((unused)) pTypeC = "i64";
        if (String_Length(pTypeRaw) > 0 && !code_string_equals(pTypeRaw, "?")) {
            pTypeC = Amalgame_Compiler_CGen_TypeToC(self, pTypeRaw);
        }
        code_string __attribute__((unused)) argName = code_string_concat("__arg", String_FromInt(pi));
        if (Amalgame_Compiler_CGen_IsCPointerType(self, pTypeC)) {
            Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(pTypeC, " "), p->Name), " = ("), pTypeC), ")"), argName), ";"));
        } else {
            Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(pTypeC, " "), p->Name), " = "), Amalgame_Compiler_CGen_UnboxScalar(self, pTypeC, argName)), ";"));
        }
    }
    for (i64 i = 0; i < cn; i++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) cap = (Amalgame_Compiler_AstNode*)AmalgameList_get(lam->Args, i);
        code_string __attribute__((unused)) ct = Amalgame_Compiler_CGen_TypeToC(self, cap->Str);
        if (String_Length(ct) == 0 || code_string_equals(ct, "void")) {
            Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(code_string_concat("void* ", cap->Name), " = __env->_"), cap->Name), ";"));
        } else {
            Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(ct, " "), cap->Name), " = __env->_"), cap->Name), ";"));
        }
        Amalgame_Compiler_CGen_LocalTypeSet(self, cap->Name, ct);
    }
    for (i64 pi = 0; pi < pn; pi++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) p = (Amalgame_Compiler_AstNode*)AmalgameList_get(lam->Params, pi);
        code_string __attribute__((unused)) pTypeRaw = p->Str;
        code_string __attribute__((unused)) pTypeC = "i64";
        if (String_Length(pTypeRaw) > 0 && !code_string_equals(pTypeRaw, "?")) {
            pTypeC = Amalgame_Compiler_CGen_TypeToC(self, pTypeRaw);
        }
        Amalgame_Compiler_CGen_LocalTypeSet(self, p->Name, pTypeC);
    }
    if (lam->Body != NULL) {
        code_bool __attribute__((unused)) prevInLam = self->InLambdaBody;
        self->InLambdaBody = 1;
        Amalgame_Compiler_CGen_EmitBlock(self, lam->Body);
        self->InLambdaBody = prevInLam;
        Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat("return ", Amalgame_Compiler_CGen_BoxAsVoid(self, "0")), ";"));
    } else if (lam->Left != NULL) {
        code_string __attribute__((unused)) bodyStr = Amalgame_Compiler_CGen_EmitExprStr(self, lam->Left);
        Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat("return ", Amalgame_Compiler_CGen_BoxAsVoid(self, bodyStr)), ";"));
    } else {
        Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat("return ", Amalgame_Compiler_CGen_BoxAsVoid(self, "0")), ";"));
    }
    for (i64 pi = 0; pi < pn; pi++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) p = (Amalgame_Compiler_AstNode*)AmalgameList_get(lam->Params, pi);
        Amalgame_Compiler_CGen_LocalTypeSet(self, p->Name, "");
    }
    for (i64 i = 0; i < cn; i++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) cap = (Amalgame_Compiler_AstNode*)AmalgameList_get(lam->Args, i);
        Amalgame_Compiler_CGen_LocalTypeSet(self, cap->Name, "");
    }
    Amalgame_Compiler_Emitter_Dedent(self->Out);
    Amalgame_Compiler_Emitter_EmitLine(self->Out, "}");
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

static void Amalgame_Compiler_CGen_TrackMapResultElem(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* stmt) {
    (void)self;
    (void)stmt;
    Amalgame_Compiler_AstNode* __attribute__((unused)) call = stmt->Left;
    if (call == NULL) {
        return;
    }
    if (AmalgameList_count(call->Args) == 0) {
        return;
    }
    Amalgame_Compiler_AstNode* __attribute__((unused)) lam = (Amalgame_Compiler_AstNode*)AmalgameList_get(call->Args, 0);
    if (lam == NULL) {
        return;
    }
    if (lam->Kind != Amalgame_Compiler_NodeKind_METHOD_DECL) {
        return;
    }
    if (!code_string_equals(lam->Name, "__lambda__")) {
        return;
    }
    if (lam->Left == NULL) {
        return;
    }
    i64 __attribute__((unused)) pn = AmalgameList_count(lam->Params);
    for (i64 pi = 0; pi < pn; pi++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) p = (Amalgame_Compiler_AstNode*)AmalgameList_get(lam->Params, pi);
        code_string __attribute__((unused)) pTypeRaw = p->Str;
        code_string __attribute__((unused)) pTypeC = "i64";
        if (String_Length(pTypeRaw) > 0 && !code_string_equals(pTypeRaw, "?")) {
            pTypeC = Amalgame_Compiler_CGen_TypeToC(self, pTypeRaw);
        }
        Amalgame_Compiler_CGen_LocalTypeSet(self, p->Name, pTypeC);
    }
    code_string __attribute__((unused)) retC = Amalgame_Compiler_CGen_InferTypeFromExpr(self, lam->Left);
    for (i64 pi = 0; pi < pn; pi++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) p = (Amalgame_Compiler_AstNode*)AmalgameList_get(lam->Params, pi);
        Amalgame_Compiler_CGen_LocalTypeSet(self, p->Name, "");
    }
    if (String_Length(retC) > 0 && !code_string_equals(retC, "void*")) {
        Amalgame_Compiler_CGen_ListElemSet(self, "__local__", stmt->Name, retC);
    }
}

static void Amalgame_Compiler_CGen_TrackGenericLocal(Amalgame_Compiler_CGen* self, code_string varName, code_string rawType) {
    (void)self;
    (void)varName;
    (void)rawType;
    if (String_StartsWith(rawType, "List<") && String_EndsWith(rawType, ">")) {
        code_string __attribute__((unused)) inner = String_Substring(rawType, 5, String_Length(rawType) - 6);
        if (String_Length(inner) > 0) {
            Amalgame_Compiler_CGen_ListElemSet(self, "__local__", varName, Amalgame_Compiler_CGen_TypeToC(self, inner));
        }
        return;
    }
    if (String_StartsWith(rawType, "Map<") && String_EndsWith(rawType, ">")) {
        code_string __attribute__((unused)) inner = String_Substring(rawType, 4, String_Length(rawType) - 5);
        i64 __attribute__((unused)) comma = String_IndexOf(inner, ",");
        if (comma > 0) {
            code_string __attribute__((unused)) vRaw = String_Substring(inner, comma + 1, String_Length(inner) - comma - 1);
            code_string __attribute__((unused)) vTrim = String_Trim(vRaw);
            if (String_Length(vTrim) > 0) {
                Amalgame_Compiler_CGen_ListElemSet(self, "__local_map__", varName, Amalgame_Compiler_CGen_TypeToC(self, vTrim));
            }
        }
    }
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

static void Amalgame_Compiler_CGen_MethodRetRawSet(Amalgame_Compiler_CGen* self, code_string className, code_string methodName, code_string rawType) {
    (void)self;
    (void)className;
    (void)methodName;
    (void)rawType;
    code_string __attribute__((unused)) key = code_string_concat(code_string_concat(className, "::"), methodName);
    AmalgameList_add(self->MethodRetRawNames, (void*)(intptr_t)(key));
    AmalgameList_add(self->MethodRetRawTypes, (void*)(intptr_t)(rawType));
}

static code_string Amalgame_Compiler_CGen_MethodRetRawGet(Amalgame_Compiler_CGen* self, code_string className, code_string methodName) {
    (void)self;
    (void)className;
    (void)methodName;
    code_string __attribute__((unused)) key = code_string_concat(code_string_concat(className, "::"), methodName);
    i64 __attribute__((unused)) n = AmalgameList_count(self->MethodRetRawNames);
    code_string __attribute__((unused)) result = "";
    for (i64 i = 0; i < n; i++) {
        code_string __attribute__((unused)) k = (code_string)AmalgameList_get(self->MethodRetRawNames, i);
        if (code_string_equals(k, key)) {
            result = (code_string)AmalgameList_get(self->MethodRetRawTypes, i);
        }
    }
    return result;
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
    if (k == Amalgame_Compiler_NodeKind_LIST_COMP) {
        return "AmalgameList*";
    }
    if (k == Amalgame_Compiler_NodeKind_IF_STMT && code_string_equals(expr->Name, "__match__")) {
        if (AmalgameList_count(expr->Children) > 0) {
            Amalgame_Compiler_AstNode* __attribute__((unused)) firstArm = (Amalgame_Compiler_AstNode*)AmalgameList_get(expr->Children, 0);
            if (firstArm->Right != NULL) {
                return Amalgame_Compiler_CGen_InferTypeFromExpr(self, firstArm->Right);
            }
        }
        return "";
    }
    if (k == Amalgame_Compiler_NodeKind_IF_STMT) {
        if (expr->Body != NULL) {
            Amalgame_Compiler_AstNode* __attribute__((unused)) body = expr->Body;
            if (body->Kind == Amalgame_Compiler_NodeKind_BLOCK && AmalgameList_count(body->Children) > 0) {
                Amalgame_Compiler_AstNode* __attribute__((unused)) last = (Amalgame_Compiler_AstNode*)AmalgameList_get(body->Children, AmalgameList_count(body->Children) - 1);
                return Amalgame_Compiler_CGen_InferTypeFromExpr(self, last);
            }
            return Amalgame_Compiler_CGen_InferTypeFromExpr(self, body);
        }
        return "";
    }
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
        if (expr->Left != NULL && expr->Left->Kind == Amalgame_Compiler_NodeKind_MEMBER) {
            code_string __attribute__((unused)) mname = expr->Left->Name;
            if (code_string_equals(mname, "Filter") || code_string_equals(mname, "Map")) {
                return "AmalgameList*";
            }
            if (code_string_equals(mname, "Any") || code_string_equals(mname, "All")) {
                return "code_bool";
            }
            if (code_string_equals(mname, "CountIf")) {
                return "i64";
            }
        }
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
            if (code_string_equals(calleeStr, "String_Length") || code_string_equals(calleeStr, "String_IndexOf") || code_string_equals(calleeStr, "String_LastIndexOf") || code_string_equals(calleeStr, "String_ToInt")) {
                return "i64";
            }
            if (code_string_equals(calleeStr, "String_ToFloat")) {
                return "double";
            }
            if (code_string_equals(calleeStr, "String_ToBool")) {
                return "code_bool";
            }
            if (code_string_equals(calleeStr, "String_Contains") || code_string_equals(calleeStr, "String_StartsWith") || code_string_equals(calleeStr, "String_EndsWith") || code_string_equals(calleeStr, "String_IsEmpty")) {
                return "code_bool";
            }
            if (String_StartsWith(calleeStr, "String_")) {
                return "code_string";
            }
            if (code_string_equals(calleeStr, "Args_Count") || code_string_equals(calleeStr, "Exit_Get")) {
                return "i64";
            }
            if (code_string_equals(calleeStr, "Args_Get")) {
                return "code_string";
            }
            if (code_string_equals(calleeStr, "Exit_Set")) {
                return "void";
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
            if (code_string_equals(calleeStr, "File_WriteLines") || code_string_equals(calleeStr, "File_OpenWrite") || code_string_equals(calleeStr, "File_StreamLine") || code_string_equals(calleeStr, "File_CloseWrite")) {
                return "void";
            }
            if (code_string_equals(calleeStr, "File_WriteLines")) {
                return "void";
            }
            if (code_string_equals(calleeStr, "File_ReadAll") || code_string_equals(calleeStr, "File_ReadLine")) {
                return "code_string";
            }
            if (code_string_equals(calleeStr, "File_Size")) {
                return "i64";
            }
            if (code_string_equals(calleeStr, "Path_Combine") || code_string_equals(calleeStr, "Path_GetExtension") || code_string_equals(calleeStr, "Path_GetFilename") || code_string_equals(calleeStr, "Path_GetDirectory")) {
                return "code_string";
            }
            if (code_string_equals(calleeStr, "Http_Get") || code_string_equals(calleeStr, "Http_Post") || code_string_equals(calleeStr, "Http_GetWithHeaders") || code_string_equals(calleeStr, "Http_GetTimeout") || code_string_equals(calleeStr, "Http_PostJson") || code_string_equals(calleeStr, "Http_PostWithHeaders") || code_string_equals(calleeStr, "Http_Put") || code_string_equals(calleeStr, "Http_Delete") || code_string_equals(calleeStr, "Http_Patch")) {
                return "AmalgameHttpResponse*";
            }
            if (code_string_equals(calleeStr, "Env_Get")) {
                return "code_string";
            }
            if (code_string_equals(calleeStr, "Env_Has")) {
                return "code_bool";
            }
            if (code_string_equals(calleeStr, "Process_RunCapture")) {
                return "AmalgameProcessResult*";
            }
            if (code_string_equals(calleeStr, "Process_Run")) {
                return "i64";
            }
            if (code_string_equals(calleeStr, "Console_ReadLine") || code_string_equals(calleeStr, "Console_ReadBytes")) {
                return "code_string";
            }
            if (code_string_equals(calleeStr, "Console_Flush") || code_string_equals(calleeStr, "Console_Write")) {
                return "void";
            }
            if (code_string_equals(calleeStr, "String_Length") || code_string_equals(calleeStr, "String_IndexOf") || code_string_equals(calleeStr, "String_LastIndexOf") || code_string_equals(calleeStr, "String_ToInt")) {
                return "i64";
            }
            if (code_string_equals(calleeStr, "String_StartsWith") || code_string_equals(calleeStr, "String_EndsWith") || code_string_equals(calleeStr, "String_Contains") || code_string_equals(calleeStr, "String_IsEmpty")) {
                return "code_bool";
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
            if (code_string_equals(calleeStr, "Math_RandomInt") || code_string_equals(calleeStr, "Math_AbsI") || code_string_equals(calleeStr, "Math_PowI") || code_string_equals(calleeStr, "Math_Gcd") || code_string_equals(calleeStr, "Math_Lcm") || code_string_equals(calleeStr, "Math_Clamp") || code_string_equals(calleeStr, "Math_MaxI") || code_string_equals(calleeStr, "Math_MinI") || code_string_equals(calleeStr, "Math_ClampI")) {
                return "i64";
            }
            if (code_string_equals(calleeStr, "Math_SeedRandom")) {
                return "void";
            }
            if (expr->Left->Kind == Amalgame_Compiler_NodeKind_MEMBER) {
                code_string __attribute__((unused)) mname2 = expr->Left->Name;
                if (code_string_equals(mname2, "Count") || code_string_equals(mname2, "Size")) {
                    return "i64";
                }
                if (code_string_equals(mname2, "Clear")) {
                    return "void";
                }
                if (code_string_equals(mname2, "Reserve")) {
                    return "void";
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

static code_string Amalgame_Compiler_CGen_WrapForInterp(Amalgame_Compiler_CGen* self, code_string cExpr, code_string cType) {
    (void)self;
    (void)cExpr;
    (void)cType;
    if (code_string_equals(cType, "code_string")) {
        return code_string_concat(code_string_concat(code_string_concat(code_string_concat("(", cExpr), " ? "), cExpr), " : \"\")");
    }
    if (code_string_equals(cType, "i64")) {
        return code_string_concat(code_string_concat("String_FromInt(", cExpr), ")");
    }
    if (code_string_equals(cType, "double")) {
        return code_string_concat(code_string_concat("String_FromFloat(", cExpr), ")");
    }
    if (code_string_equals(cType, "code_bool")) {
        return code_string_concat(code_string_concat("((", cExpr), ") ? \"true\" : \"false\")");
    }
    return code_string_concat(code_string_concat("String_FromInt((i64)(", cExpr), "))");
}

static code_string Amalgame_Compiler_CGen_BuiltinCallReturnType(Amalgame_Compiler_CGen* self, code_string cName) {
    (void)self;
    (void)cName;
    if (code_string_equals(cName, "String_Length") || code_string_equals(cName, "String_IndexOf") || code_string_equals(cName, "String_LastIndexOf") || code_string_equals(cName, "String_ToInt")) {
        return "i64";
    }
    if (code_string_equals(cName, "String_ToFloat")) {
        return "double";
    }
    if (code_string_equals(cName, "String_IsEmpty") || code_string_equals(cName, "String_Contains") || code_string_equals(cName, "String_StartsWith") || code_string_equals(cName, "String_EndsWith") || code_string_equals(cName, "String_ToBool")) {
        return "code_bool";
    }
    if (String_StartsWith(cName, "String_")) {
        return "code_string";
    }
    if (code_string_equals(cName, "Math_Sqrt") || code_string_equals(cName, "Math_Abs") || code_string_equals(cName, "Math_Pow") || code_string_equals(cName, "Math_Floor") || code_string_equals(cName, "Math_Ceil") || code_string_equals(cName, "Math_Round") || code_string_equals(cName, "Math_Random")) {
        return "double";
    }
    if (code_string_equals(cName, "Math_AbsI") || code_string_equals(cName, "Math_PowI") || code_string_equals(cName, "Math_RandomInt") || code_string_equals(cName, "Math_Gcd") || code_string_equals(cName, "Math_MaxI") || code_string_equals(cName, "Math_MinI") || code_string_equals(cName, "Math_ClampI")) {
        return "i64";
    }
    if (code_string_equals(cName, "Math_IsPrime") || code_string_equals(cName, "Math_IsNaN") || code_string_equals(cName, "Math_IsFinite")) {
        return "code_bool";
    }
    if (code_string_equals(cName, "File_ReadAll")) {
        return "code_string";
    }
    if (code_string_equals(cName, "File_Exists") || code_string_equals(cName, "File_WriteAll") || code_string_equals(cName, "File_AppendAll") || code_string_equals(cName, "File_Delete")) {
        return "code_bool";
    }
    if (code_string_equals(cName, "File_Size") || code_string_equals(cName, "Args_Count") || code_string_equals(cName, "Exit_Get")) {
        return "i64";
    }
    if (code_string_equals(cName, "Args_Get")) {
        return "code_string";
    }
    return "";
}

static code_string Amalgame_Compiler_CGen_InterpExprToC(Amalgame_Compiler_CGen* self, code_string expr) {
    (void)self;
    (void)expr;
    i64 __attribute__((unused)) parenIdx = String_IndexOf(expr, "(");
    if (parenIdx > 0) {
        code_string __attribute__((unused)) callee = String_Substring(expr, 0, parenIdx);
        code_string __attribute__((unused)) rest = String_Substring(expr, parenIdx, String_Length(expr) - parenIdx);
        code_string __attribute__((unused)) cCallee = String_Replace(callee, ".", "_");
        code_string __attribute__((unused)) cCall = code_string_concat(cCallee, rest);
        code_string __attribute__((unused)) retT = Amalgame_Compiler_CGen_BuiltinCallReturnType(self, cCallee);
        if (String_Length(retT) > 0) {
            return Amalgame_Compiler_CGen_WrapForInterp(self, cCall, retT);
        }
        return code_string_concat(code_string_concat(code_string_concat(code_string_concat("(", cCall), " ? "), cCall), " : \"\")");
    }
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
    s = String_Replace(s, String_FromByte(13), "\\r");
    s = String_Replace(s, String_FromByte(27), "\\x1b");
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
    Amalgame_Compiler_Emitter_EmitLine(self->Out, "#include \"Amalgame_Process.h\"");
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
                Amalgame_Compiler_CGen_MethodRetRawSet(self, name, m->Name, m->Str);
                code_string __attribute__((unused)) attrs = "";
                code_string __attribute__((unused)) decos = m->Str2;
                if (String_Length(decos) > 0) {
                    if (String_Contains(code_string_concat(code_string_concat(",", decos), ","), ",deprecated,")) {
                        attrs = " __attribute__((deprecated))";
                    }
                }
                if (isPublic) {
                    Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(sig, attrs), ";"));
                } else {
                    Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat("static ", sig), attrs), ";"));
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
                } else {
                    for (i64 pi = 0; pi < pcount; pi++) {
                        Amalgame_Compiler_AstNode* __attribute__((unused)) p = (Amalgame_Compiler_AstNode*)AmalgameList_get(m->Params, pi);
                        code_bool __attribute__((unused)) hasField = 0;
                        for (i64 fi = 0; fi < members; fi++) {
                            Amalgame_Compiler_AstNode* __attribute__((unused)) f = (Amalgame_Compiler_AstNode*)AmalgameList_get(cls->Children, fi);
                            if (f->Kind == Amalgame_Compiler_NodeKind_VAR_DECL && code_string_equals(f->Name, p->Name)) {
                                hasField = 1;
                            }
                        }
                        if (hasField) {
                            Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(code_string_concat("self->", p->Name), " = "), p->Name), ";"));
                        }
                    }
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
    code_string __attribute__((unused)) decos = method->Str2;
    if (String_Length(decos) > 0) {
        if (String_Contains(code_string_concat(code_string_concat(",", decos), ","), ",inline,")) {
            prefix = code_string_concat(prefix, "inline ");
        }
    }
    if (String_StartsWith(method->Str, "(")) {
        self->CurrentRetType = Amalgame_Compiler_CGen_TupleStructName(self, method->Str);
        Amalgame_Compiler_CGen_MethodRetSet(self, className, method->Name, method->Str);
        Amalgame_Compiler_CGen_MethodRetRawSet(self, className, method->Name, method->Str);
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
        Amalgame_Compiler_CGen_TrackGenericLocal(self, p->Name, p->Str);
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
                code_string __attribute__((unused)) elsePrefix = (firstArm ? "" : "} else ");
                Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(elsePrefix, "{"));
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
                code_string __attribute__((unused)) elsePrefix = (firstArm ? "" : "} else ");
                code_bool __attribute__((unused)) isBinder = pk == Amalgame_Compiler_NodeKind_IDENTIFIER && !code_string_equals(pat->Name, "_");
                code_string __attribute__((unused)) cond = "";
                if (pk == Amalgame_Compiler_NodeKind_BINARY && code_string_equals(pat->Str, "..")) {
                    code_string __attribute__((unused)) lo = Amalgame_Compiler_CGen_EmitExprStr(self, pat->Left);
                    code_string __attribute__((unused)) hi = Amalgame_Compiler_CGen_EmitExprStr(self, pat->Right);
                    cond = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(subjectStr, " >= "), lo), " && "), subjectStr), " <= "), hi);
                } else if (pk == Amalgame_Compiler_NodeKind_LITERAL_STRING) {
                    code_string __attribute__((unused)) patStr = Amalgame_Compiler_CGen_EmitExprStr(self, pat);
                    cond = code_string_concat(code_string_concat(code_string_concat(code_string_concat("strcmp(", subjectStr), ", "), patStr), ") == 0");
                } else if (isBinder) {
                    cond = "1";
                } else {
                    code_string __attribute__((unused)) patStr = Amalgame_Compiler_CGen_EmitExprStr(self, pat);
                    cond = code_string_concat(code_string_concat(subjectStr, " == "), patStr);
                }
                if (arm->Cond != NULL) {
                    if (isBinder) {
                        Amalgame_Compiler_CGen_LocalTypeSet(self, pat->Name, "i64");
                    }
                    code_string __attribute__((unused)) guardStr = Amalgame_Compiler_CGen_EmitExprStr(self, arm->Cond);
                    if (isBinder) {
                        cond = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("(({ __auto_type ", pat->Name), " = "), subjectStr), "; "), guardStr), "; }))");
                    } else {
                        cond = code_string_concat(code_string_concat(code_string_concat(code_string_concat("(", cond), ") && ("), guardStr), ")");
                    }
                }
                Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(elsePrefix, "if ("), cond), ") {"));
                Amalgame_Compiler_Emitter_Indent_(self->Out);
                if (isBinder) {
                    Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(code_string_concat("__auto_type ", pat->Name), " = "), subjectStr), ";"));
                    Amalgame_Compiler_CGen_LocalTypeSet(self, pat->Name, "i64");
                }
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
    } else if (bk == Amalgame_Compiler_NodeKind_RETURN_STMT || bk == Amalgame_Compiler_NodeKind_BREAK_STMT || bk == Amalgame_Compiler_NodeKind_CONTINUE_STMT) {
        Amalgame_Compiler_CGen_EmitStmt(self, body);
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
    if (k == Amalgame_Compiler_NodeKind_TRY_STMT) {
        code_string __attribute__((unused)) suffix = String_FromInt(stmt->Line);
        Amalgame_Compiler_Emitter_EmitLine(self->Out, "{");
        Amalgame_Compiler_Emitter_Indent_(self->Out);
        Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat("jmp_buf _am_prev_env_", suffix), ";"));
        Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat("memcpy(&_am_prev_env_", suffix), ", &_am_ex.env, sizeof(jmp_buf));"));
        Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat("int _am_caught_", suffix), " = setjmp(_am_ex.env);"));
        Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat("if (_am_caught_", suffix), " == 0) {"));
        Amalgame_Compiler_Emitter_Indent_(self->Out);
        if (stmt->Body != NULL) {
            Amalgame_Compiler_CGen_EmitBlock(self, stmt->Body);
        }
        Amalgame_Compiler_Emitter_EmitLine(self->Out, "_am_ex.active = 0;");
        Amalgame_Compiler_Emitter_Dedent(self->Out);
        Amalgame_Compiler_Emitter_EmitLine(self->Out, "} else {");
        Amalgame_Compiler_Emitter_Indent_(self->Out);
        if (String_Length(stmt->Name) > 0) {
            Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat("void* __attribute__((unused)) ", stmt->Name), " = _am_ex.value;"));
            Amalgame_Compiler_CGen_LocalTypeSet(self, stmt->Name, "void*");
        }
        Amalgame_Compiler_Emitter_EmitLine(self->Out, "_am_ex.active = 0;");
        if (stmt->Else != NULL) {
            Amalgame_Compiler_CGen_EmitBlock(self, stmt->Else);
        }
        Amalgame_Compiler_Emitter_Dedent(self->Out);
        Amalgame_Compiler_Emitter_EmitLine(self->Out, "}");
        Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat("memcpy(&_am_ex.env, &_am_prev_env_", suffix), ", sizeof(jmp_buf));"));
        if (stmt->Cond != NULL) {
            Amalgame_Compiler_CGen_EmitBlock(self, stmt->Cond);
        }
        Amalgame_Compiler_Emitter_Dedent(self->Out);
        Amalgame_Compiler_Emitter_EmitLine(self->Out, "}");
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_THROW_STMT) {
        if (stmt->Left == NULL) {
            Amalgame_Compiler_Emitter_EmitLine(self->Out, "_am_throw(NULL, \"Error\", \"\");");
            return;
        }
        Amalgame_Compiler_AstNode* __attribute__((unused)) v = stmt->Left;
        if (v->Kind == Amalgame_Compiler_NodeKind_NEW_EXPR) {
            code_string __attribute__((unused)) typeName = v->Name;
            code_string __attribute__((unused)) argsStr = "";
            i64 __attribute__((unused)) ac = AmalgameList_count(v->Args);
            for (i64 ai = 0; ai < ac; ai++) {
                if (ai > 0) {
                    argsStr = code_string_concat(argsStr, ", ");
                }
                argsStr = code_string_concat(argsStr, Amalgame_Compiler_CGen_EmitExprStr(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(v->Args, ai)));
            }
            code_string __attribute__((unused)) msgStr = "\"\"";
            if (ac > 0) {
                Amalgame_Compiler_AstNode* __attribute__((unused)) firstArg = (Amalgame_Compiler_AstNode*)AmalgameList_get(v->Args, 0);
                if (firstArg->Kind == Amalgame_Compiler_NodeKind_LITERAL_STRING) {
                    msgStr = Amalgame_Compiler_CGen_EmitExprStr(self, firstArg);
                }
            }
            code_string __attribute__((unused)) symName = Amalgame_Compiler_CGen_SymName(self, typeName);
            Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("_am_throw((void*)(", symName), "_new("), argsStr), ")), \""), typeName), "\", "), msgStr), ");"));
            return;
        }
        code_string __attribute__((unused)) valStr = Amalgame_Compiler_CGen_EmitExprStr(self, v);
        Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat("_am_throw((void*)", valStr), ", \"Error\", \"\");"));
        return;
    }
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
            code_string __attribute__((unused)) id = lam->Str2;
            code_string __attribute__((unused)) envName = code_string_concat("LamEnv_", id);
            code_string __attribute__((unused)) fnName = code_string_concat(code_string_concat("lam_", id), "_fn");
            code_string __attribute__((unused)) envVar = code_string_concat("__env_", id);
            Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(envName, "* "), envVar), " = ("), envName), "*) code_alloc(sizeof("), envName), "));"));
            i64 __attribute__((unused)) cn = AmalgameList_count(lam->Args);
            for (i64 i = 0; i < cn; i++) {
                Amalgame_Compiler_AstNode* __attribute__((unused)) cap = (Amalgame_Compiler_AstNode*)AmalgameList_get(lam->Args, i);
                Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(envVar, "->_"), cap->Name), " = "), cap->Name), ";"));
            }
            Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("AmalgameClosure* ", stmt->Name), " = AmalgameClosure_new((void*)"), fnName), ", "), envVar), ");"));
            Amalgame_Compiler_CGen_LocalTypeSet(self, stmt->Name, "AmalgameClosure*");
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
        if (String_Length(stmt->Str) > 0) {
            Amalgame_Compiler_CGen_TrackGenericLocal(self, stmt->Name, stmt->Str);
        }
        if (stmt->Left != NULL && stmt->Left->Kind == Amalgame_Compiler_NodeKind_NEW_EXPR) {
            if (String_Length(stmt->Left->Str2) > 0) {
                code_string __attribute__((unused)) synth = code_string_concat(code_string_concat(code_string_concat(stmt->Left->Name, "<"), stmt->Left->Str2), ">");
                Amalgame_Compiler_CGen_TrackGenericLocal(self, stmt->Name, synth);
            }
        }
        if (stmt->Left != NULL && stmt->Left->Kind == Amalgame_Compiler_NodeKind_CALL) {
            Amalgame_Compiler_AstNode* __attribute__((unused)) callee = stmt->Left->Left;
            if (callee != NULL && callee->Kind == Amalgame_Compiler_NodeKind_MEMBER) {
                if (callee->Left != NULL && callee->Left->Kind == Amalgame_Compiler_NodeKind_IDENTIFIER) {
                    code_string __attribute__((unused)) retRaw = Amalgame_Compiler_CGen_MethodRetRawGet(self, callee->Left->Name, callee->Name);
                    if (String_Length(retRaw) > 0) {
                        Amalgame_Compiler_CGen_TrackGenericLocal(self, stmt->Name, retRaw);
                    }
                }
                code_string __attribute__((unused)) mname = callee->Name;
                if (code_string_equals(mname, "Filter")) {
                    if (callee->Left != NULL && callee->Left->Kind == Amalgame_Compiler_NodeKind_IDENTIFIER) {
                        code_string __attribute__((unused)) recvName = callee->Left->Name;
                        code_string __attribute__((unused)) recvElem = Amalgame_Compiler_CGen_ListElemGet(self, "__local__", recvName);
                        if (String_Length(recvElem) > 0) {
                            Amalgame_Compiler_CGen_ListElemSet(self, "__local__", stmt->Name, recvElem);
                        }
                    }
                } else if (code_string_equals(mname, "Map")) {
                    Amalgame_Compiler_CGen_TrackMapResultElem(self, stmt);
                }
            }
        }
        code_string __attribute__((unused)) rhs = Amalgame_Compiler_CGen_EmitExprStr(self, stmt->Left);
        code_string __attribute__((unused)) decl = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(t, " __attribute__((unused)) "), stmt->Name), " = "), rhs), ";");
        if (code_string_equals(t, "void*")) {
            if (String_StartsWith(rhs, "(") && !String_StartsWith(rhs, "(void*)") && !String_StartsWith(rhs, "({") && !String_Contains(rhs, "?")) {
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
            if (self->InLambdaBody) {
                Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat("return ", Amalgame_Compiler_CGen_BoxAsVoid(self, "0")), ";"));
            } else {
                Amalgame_Compiler_Emitter_EmitLine(self->Out, "return;");
            }
        } else {
            code_string __attribute__((unused)) retExpr = Amalgame_Compiler_CGen_EmitExprStr(self, stmt->Left);
            if (code_string_equals(retExpr, "_unknown_")) {
                Amalgame_Compiler_Emitter_EmitLine(self->Out, "return;");
            } else if (self->InLambdaBody) {
                Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat("return ", Amalgame_Compiler_CGen_BoxAsVoid(self, retExpr)), ";"));
            } else if (String_StartsWith(retExpr, "{") && String_Length(self->CurrentRetType) > 0) {
                Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(code_string_concat("return (", self->CurrentRetType), ")"), retExpr), ";"));
            } else {
                Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat("return ", retExpr), ";"));
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
        Amalgame_Compiler_Emitter_EmitLine(self->Out, "{");
        Amalgame_Compiler_Emitter_Indent_(self->Out);
        Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(code_string_concat("AmalgameList* __it_", vn), " = "), iter), ";"));
        Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(code_string_concat("i64 __len_", vn), " = AmalgameList_size(__it_"), vn), ");"));
        Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("for (i64 __idx_", vn), " = 0; __idx_"), vn), " < __len_"), vn), "; __idx_"), vn), "++) {"));
        Amalgame_Compiler_Emitter_Indent_(self->Out);
        Amalgame_Compiler_Emitter_EmitLine(self->Out, code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("void* ", vn), " = __it_"), vn), "->data[__idx_"), vn), "];"));
        Amalgame_Compiler_CGen_LocalTypeSet(self, vn, "void*");
        if (stmt->Body != NULL) {
            Amalgame_Compiler_CGen_EmitBlock(self, stmt->Body);
        }
        Amalgame_Compiler_Emitter_Dedent(self->Out);
        Amalgame_Compiler_Emitter_EmitLine(self->Out, "}");
        Amalgame_Compiler_Emitter_Dedent(self->Out);
        Amalgame_Compiler_Emitter_EmitLine(self->Out, "}");
        Amalgame_Compiler_CGen_LocalTypeSet(self, vn, "");
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
        if (code_string_equals(expr->Name, "__match__")) {
            return Amalgame_Compiler_CGen_EmitMatchExpr(self, expr);
        }
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
        code_string __attribute__((unused)) name = expr->Name;
        if (String_Length(self->CurrentClass) > 0) {
            code_string __attribute__((unused)) asLocal = Amalgame_Compiler_CGen_LocalTypeGet(self, name);
            if (String_Length(asLocal) == 0) {
                code_string __attribute__((unused)) asField = Amalgame_Compiler_CGen_FieldTypeGet(self, self->CurrentClass, name);
                if (String_Length(asField) > 0 && !code_string_equals(asField, "?")) {
                    return code_string_concat("self->", name);
                }
            }
        }
        return name;
    }
    if (k == Amalgame_Compiler_NodeKind_MEMBER) {
        if (expr->Flag) {
            code_string __attribute__((unused)) target = Amalgame_Compiler_CGen_EmitExprStr(self, expr->Left);
            return code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("(", target), " ? "), target), "->"), expr->Name), " : NULL)");
        }
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
        if (expr->Left != NULL) {
            Amalgame_Compiler_NodeKind __attribute__((unused)) lk = expr->Left->Kind;
            if (lk == Amalgame_Compiler_NodeKind_BINARY) {
                return code_string_concat(code_string_concat(code_string_concat(expr->Str, "("), operand), ")");
            }
        }
        return code_string_concat(expr->Str, operand);
    }
    if (k == Amalgame_Compiler_NodeKind_CALL) {
        if (expr->Left != NULL && expr->Left->Kind == Amalgame_Compiler_NodeKind_IDENTIFIER) {
            code_string __attribute__((unused)) calleeName = expr->Left->Name;
            code_string __attribute__((unused)) calleeType = Amalgame_Compiler_CGen_LocalTypeGet(self, calleeName);
            if (code_string_equals(calleeType, "AmalgameClosure*")) {
                i64 __attribute__((unused)) argc = AmalgameList_count(expr->Args);
                if (argc == 1) {
                    code_string __attribute__((unused)) arg0 = Amalgame_Compiler_CGen_EmitExprStr(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(expr->Args, 0));
                    return Amalgame_Compiler_CGen_UnboxScalar(self, "i64", code_string_concat(code_string_concat(code_string_concat(code_string_concat("AmalgameClosure_call1(", calleeName), ", "), Amalgame_Compiler_CGen_BoxAsVoid(self, arg0)), ")"));
                }
                if (argc == 2) {
                    code_string __attribute__((unused)) arg0 = Amalgame_Compiler_CGen_EmitExprStr(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(expr->Args, 0));
                    code_string __attribute__((unused)) arg1 = Amalgame_Compiler_CGen_EmitExprStr(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(expr->Args, 1));
                    return Amalgame_Compiler_CGen_UnboxScalar(self, "i64", code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("AmalgameClosure_call2(", calleeName), ", "), Amalgame_Compiler_CGen_BoxAsVoid(self, arg0)), ", "), Amalgame_Compiler_CGen_BoxAsVoid(self, arg1)), ")"));
                }
                if (argc == 3) {
                    code_string __attribute__((unused)) arg0 = Amalgame_Compiler_CGen_EmitExprStr(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(expr->Args, 0));
                    code_string __attribute__((unused)) arg1 = Amalgame_Compiler_CGen_EmitExprStr(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(expr->Args, 1));
                    code_string __attribute__((unused)) arg2 = Amalgame_Compiler_CGen_EmitExprStr(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(expr->Args, 2));
                    return Amalgame_Compiler_CGen_UnboxScalar(self, "i64", code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("AmalgameClosure_call3(", calleeName), ", "), Amalgame_Compiler_CGen_BoxAsVoid(self, arg0)), ", "), Amalgame_Compiler_CGen_BoxAsVoid(self, arg1)), ", "), Amalgame_Compiler_CGen_BoxAsVoid(self, arg2)), ")"));
                }
            }
        }
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
                    if (ll->Kind == Amalgame_Compiler_NodeKind_LITERAL_STRING) {
                        isSelfCall = 1;
                        selfExpr = Amalgame_Compiler_CGen_EmitExprStr(self, ll);
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
        if (expr->Left != NULL && expr->Left->Kind == Amalgame_Compiler_NodeKind_MEMBER && expr->Left->Flag) {
            code_string __attribute__((unused)) recv = Amalgame_Compiler_CGen_EmitExprStr(self, expr->Left->Left);
            return code_string_concat(code_string_concat(code_string_concat(code_string_concat("(", recv), " ? "), callStr), " : NULL)");
        }
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
    if (k == Amalgame_Compiler_NodeKind_LIST_COMP) {
        return Amalgame_Compiler_CGen_EmitListComp(self, expr);
    }
    return "/* unknown expr */";
}

static code_string Amalgame_Compiler_CGen_EmitMatchExpr(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* n) {
    (void)self;
    (void)n;
    Amalgame_Compiler_AstNode* __attribute__((unused)) subject = n->Left;
    if (subject == NULL) {
        return "/* match-expr: missing subject */";
    }
    code_string __attribute__((unused)) subjectStr = Amalgame_Compiler_CGen_EmitExprStr(self, subject);
    i64 __attribute__((unused)) armCount = AmalgameList_count(n->Children);
    if (armCount == 0) {
        return "/* match-expr: no arms */";
    }
    Amalgame_Compiler_AstNode* __attribute__((unused)) firstArm = (Amalgame_Compiler_AstNode*)AmalgameList_get(n->Children, 0);
    Amalgame_Compiler_AstNode* __attribute__((unused)) firstBody = firstArm->Right;
    code_string __attribute__((unused)) resultType = Amalgame_Compiler_CGen_InferTypeFromExpr(self, firstBody);
    if (String_Length(resultType) == 0) {
        resultType = "i64";
    }
    code_string __attribute__((unused)) s = code_string_concat(code_string_concat("({ ", resultType), " __mr; ");
    code_bool __attribute__((unused)) firstArmFlag = 1;
    for (i64 i = 0; i < armCount; i++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) arm = (Amalgame_Compiler_AstNode*)AmalgameList_get(n->Children, i);
        Amalgame_Compiler_AstNode* __attribute__((unused)) pat = arm->Left;
        Amalgame_Compiler_AstNode* __attribute__((unused)) body = arm->Right;
        if (pat == NULL || body == NULL) {
            continue;
        }
        Amalgame_Compiler_NodeKind __attribute__((unused)) pk = pat->Kind;
        code_string __attribute__((unused)) bodyStr = Amalgame_Compiler_CGen_EmitExprStr(self, body);
        if (pk == Amalgame_Compiler_NodeKind_IDENTIFIER && code_string_equals(pat->Name, "_")) {
            code_string __attribute__((unused)) prefix = (firstArmFlag ? "" : " else ");
            s = code_string_concat(code_string_concat(code_string_concat(code_string_concat(s, prefix), "{ __mr = ("), bodyStr), "); }");
            firstArmFlag = 0;
            continue;
        }
        code_string __attribute__((unused)) cond = "";
        if (pk == Amalgame_Compiler_NodeKind_BINARY && code_string_equals(pat->Str, "..")) {
            code_string __attribute__((unused)) lo = Amalgame_Compiler_CGen_EmitExprStr(self, pat->Left);
            code_string __attribute__((unused)) hi = Amalgame_Compiler_CGen_EmitExprStr(self, pat->Right);
            cond = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(subjectStr, " >= "), lo), " && "), subjectStr), " <= "), hi);
        } else if (pk == Amalgame_Compiler_NodeKind_LITERAL_STRING) {
            code_string __attribute__((unused)) patStr = Amalgame_Compiler_CGen_EmitExprStr(self, pat);
            cond = code_string_concat(code_string_concat(code_string_concat(code_string_concat("strcmp(", subjectStr), ", "), patStr), ") == 0");
        } else {
            code_string __attribute__((unused)) patStr = Amalgame_Compiler_CGen_EmitExprStr(self, pat);
            cond = code_string_concat(code_string_concat(subjectStr, " == "), patStr);
        }
        code_string __attribute__((unused)) prefix = (firstArmFlag ? "" : " else ");
        s = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(s, prefix), "if ("), cond), ") { __mr = ("), bodyStr), "); }");
        firstArmFlag = 0;
    }
    s = code_string_concat(s, " __mr; })");
    return s;
}

static code_string Amalgame_Compiler_CGen_EmitListComp(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* n) {
    (void)self;
    (void)n;
    code_string __attribute__((unused)) vn = n->Str;
    Amalgame_Compiler_AstNode* __attribute__((unused)) iter = n->Right;
    code_bool __attribute__((unused)) isRange = 0;
    if (iter != NULL && iter->Kind == Amalgame_Compiler_NodeKind_BINARY && code_string_equals(iter->Str, "..")) {
        isRange = 1;
    }
    code_string __attribute__((unused)) loopHeader = "";
    if (isRange) {
        code_string __attribute__((unused)) startStr = Amalgame_Compiler_CGen_EmitExprStr(self, iter->Left);
        code_string __attribute__((unused)) endStr = Amalgame_Compiler_CGen_EmitExprStr(self, iter->Right);
        loopHeader = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("for (i64 ", vn), " = "), startStr), "; "), vn), " < "), endStr), "; "), vn), "++) { ");
        Amalgame_Compiler_CGen_LocalTypeSet(self, vn, "i64");
    } else {
        code_string __attribute__((unused)) iterStr = Amalgame_Compiler_CGen_EmitExprStr(self, iter);
        loopHeader = code_string_concat(code_string_concat(code_string_concat(code_string_concat("AmalgameList* __it_", vn), " = "), iterStr), "; ");
        loopHeader = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(loopHeader, "i64 __n_"), vn), " = AmalgameList_size(__it_"), vn), "); ");
        loopHeader = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(loopHeader, "for (i64 __i_"), vn), " = 0; __i_"), vn), " < __n_"), vn), "; __i_"), vn), "++) { ");
        loopHeader = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(loopHeader, "void* "), vn), " = __it_"), vn), "->data[__i_"), vn), "]; ");
        Amalgame_Compiler_CGen_LocalTypeSet(self, vn, "void*");
    }
    code_string __attribute__((unused)) projStr = Amalgame_Compiler_CGen_EmitExprStr(self, n->Left);
    code_string __attribute__((unused)) guardStr = "";
    if (n->Cond != NULL) {
        guardStr = code_string_concat(code_string_concat("if (", Amalgame_Compiler_CGen_EmitExprStr(self, n->Cond)), ") ");
    }
    Amalgame_Compiler_CGen_LocalTypeSet(self, vn, "");
    code_string __attribute__((unused)) s = code_string_concat(code_string_concat("({ AmalgameList* __lc_", vn), " = AmalgameList_new(); ");
    s = code_string_concat(s, loopHeader);
    s = code_string_concat(s, guardStr);
    s = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(s, "AmalgameList_add(__lc_"), vn), ", "), Amalgame_Compiler_CGen_BoxAsVoid(self, projStr)), "); ");
    s = code_string_concat(code_string_concat(code_string_concat(s, "} __lc_"), vn), "; })");
    return s;
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
    if (code_string_equals(mname, "Set") || code_string_equals(mname, "Has") || code_string_equals(mname, "Size") || code_string_equals(mname, "Remove") || code_string_equals(mname, "Get")) {
        if (callee->Left != NULL && callee->Left->Kind == Amalgame_Compiler_NodeKind_IDENTIFIER) {
            code_string __attribute__((unused)) vname = callee->Left->Name;
            code_string __attribute__((unused)) vtype = Amalgame_Compiler_CGen_LocalTypeGet(self, vname);
            if (code_string_equals(vtype, "AmalgameMap*")) {
                AmalgameList* __attribute__((unused)) args = callExpr->Args;
                i64 __attribute__((unused)) ac = AmalgameList_count(args);
                if (code_string_equals(mname, "Set") && ac >= 2) {
                    code_string __attribute__((unused)) k2 = Amalgame_Compiler_CGen_EmitExprStr(self, (void*)AmalgameList_get(args, 0));
                    code_string __attribute__((unused)) v2 = Amalgame_Compiler_CGen_EmitExprStr(self, (void*)AmalgameList_get(args, 1));
                    return code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("AmalgameMap_set(", vname), ", "), k2), ", "), Amalgame_Compiler_CGen_BoxAsVoid(self, v2)), ")");
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
                if (code_string_equals(mname, "Get") && ac >= 1) {
                    code_string __attribute__((unused)) k2 = Amalgame_Compiler_CGen_EmitExprStr(self, (void*)AmalgameList_get(args, 0));
                    code_string __attribute__((unused)) castT = Amalgame_Compiler_CGen_ListElemGet(self, "__local_map__", vname);
                    if (String_Length(castT) == 0) {
                        castT = "void*";
                    }
                    return code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("(", castT), ")AmalgameMap_get("), vname), ", "), k2), ")");
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
    if (!code_string_equals(mname, "Add") && !code_string_equals(mname, "Count") && !code_string_equals(mname, "Get") && !code_string_equals(mname, "IsEmpty") && !code_string_equals(mname, "Remove") && !code_string_equals(mname, "RemoveAt") && !code_string_equals(mname, "Clear") && !code_string_equals(mname, "Reserve") && !code_string_equals(mname, "Filter") && !code_string_equals(mname, "Map") && !code_string_equals(mname, "Reduce") && !code_string_equals(mname, "ForEach") && !code_string_equals(mname, "Any") && !code_string_equals(mname, "All") && !code_string_equals(mname, "CountIf")) {
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
        if (lk == Amalgame_Compiler_NodeKind_CALL) {
            code_string __attribute__((unused)) innerStr = Amalgame_Compiler_CGen_TryEmitListCall(self, callee->Left);
            if (String_Length(innerStr) > 0) {
                listExpr = innerStr;
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
    if (code_string_equals(mname, "Clear")) {
        return code_string_concat(code_string_concat("AmalgameList_clear(", listExpr), ")");
    }
    if (code_string_equals(mname, "Reserve")) {
        code_string __attribute__((unused)) arg0 = Amalgame_Compiler_CGen_EmitExprStr(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(callExpr->Args, 0));
        return code_string_concat(code_string_concat(code_string_concat(code_string_concat("AmalgameList_reserve(", listExpr), ", "), arg0), ")");
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
        return code_string_concat(code_string_concat(code_string_concat(code_string_concat("AmalgameList_add(", listExpr), ", "), Amalgame_Compiler_CGen_BoxAsVoid(self, arg0)), ")");
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
                elemType = Amalgame_Compiler_CGen_ListElemGet(self, "__local__", vn5);
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
            return code_string_concat(code_string_concat(code_string_concat(code_string_concat("AmalgameList_remove(", listExpr), ", "), Amalgame_Compiler_CGen_BoxAsVoid(self, arg0)), ")");
        }
    }
    if (code_string_equals(mname, "RemoveAt")) {
        i64 __attribute__((unused)) argc = AmalgameList_count(callExpr->Args);
        if (argc > 0) {
            code_string __attribute__((unused)) arg0 = Amalgame_Compiler_CGen_EmitExprStr(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(callExpr->Args, 0));
            return code_string_concat(code_string_concat(code_string_concat(code_string_concat("AmalgameList_removeAt(", listExpr), ", "), arg0), ")");
        }
    }
    if (code_string_equals(mname, "Filter")) {
        i64 __attribute__((unused)) argc = AmalgameList_count(callExpr->Args);
        if (argc != 1) {
            return "";
        }
        code_string __attribute__((unused)) lamStr = Amalgame_Compiler_CGen_EmitClosureArg(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(callExpr->Args, 0));
        if (String_Length(lamStr) == 0) {
            return "";
        }
        return code_string_concat(code_string_concat(code_string_concat(code_string_concat("AmalgameList_filter(", listExpr), ", "), lamStr), ")");
    }
    if (code_string_equals(mname, "Map")) {
        i64 __attribute__((unused)) argc = AmalgameList_count(callExpr->Args);
        if (argc != 1) {
            return "";
        }
        code_string __attribute__((unused)) lamStr = Amalgame_Compiler_CGen_EmitClosureArg(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(callExpr->Args, 0));
        if (String_Length(lamStr) == 0) {
            return "";
        }
        return code_string_concat(code_string_concat(code_string_concat(code_string_concat("AmalgameList_map(", listExpr), ", "), lamStr), ")");
    }
    if (code_string_equals(mname, "ForEach")) {
        i64 __attribute__((unused)) argc = AmalgameList_count(callExpr->Args);
        if (argc != 1) {
            return "";
        }
        code_string __attribute__((unused)) lamStr = Amalgame_Compiler_CGen_EmitClosureArg(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(callExpr->Args, 0));
        if (String_Length(lamStr) == 0) {
            return "";
        }
        return code_string_concat(code_string_concat(code_string_concat(code_string_concat("AmalgameList_forEach(", listExpr), ", "), lamStr), ")");
    }
    if (code_string_equals(mname, "Reduce")) {
        i64 __attribute__((unused)) argc = AmalgameList_count(callExpr->Args);
        if (argc != 2) {
            return "";
        }
        code_string __attribute__((unused)) initStr = Amalgame_Compiler_CGen_EmitExprStr(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(callExpr->Args, 0));
        code_string __attribute__((unused)) lamStr = Amalgame_Compiler_CGen_EmitClosureArg(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(callExpr->Args, 1));
        if (String_Length(lamStr) == 0) {
            return "";
        }
        return Amalgame_Compiler_CGen_UnboxScalar(self, "i64", code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("AmalgameList_reduce(", listExpr), ", "), Amalgame_Compiler_CGen_BoxAsVoid(self, initStr)), ", "), lamStr), ")"));
    }
    if (code_string_equals(mname, "Any")) {
        i64 __attribute__((unused)) argc = AmalgameList_count(callExpr->Args);
        if (argc != 1) {
            return "";
        }
        code_string __attribute__((unused)) lamStr = Amalgame_Compiler_CGen_EmitClosureArg(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(callExpr->Args, 0));
        if (String_Length(lamStr) == 0) {
            return "";
        }
        return code_string_concat(code_string_concat(code_string_concat(code_string_concat("AmalgameList_any(", listExpr), ", "), lamStr), ")");
    }
    if (code_string_equals(mname, "All")) {
        i64 __attribute__((unused)) argc = AmalgameList_count(callExpr->Args);
        if (argc != 1) {
            return "";
        }
        code_string __attribute__((unused)) lamStr = Amalgame_Compiler_CGen_EmitClosureArg(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(callExpr->Args, 0));
        if (String_Length(lamStr) == 0) {
            return "";
        }
        return code_string_concat(code_string_concat(code_string_concat(code_string_concat("AmalgameList_all(", listExpr), ", "), lamStr), ")");
    }
    if (code_string_equals(mname, "CountIf")) {
        i64 __attribute__((unused)) argc = AmalgameList_count(callExpr->Args);
        if (argc != 1) {
            return "";
        }
        code_string __attribute__((unused)) lamStr = Amalgame_Compiler_CGen_EmitClosureArg(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(callExpr->Args, 0));
        if (String_Length(lamStr) == 0) {
            return "";
        }
        return code_string_concat(code_string_concat(code_string_concat(code_string_concat("AmalgameList_countIf(", listExpr), ", "), lamStr), ")");
    }
    return "";
}

static code_string Amalgame_Compiler_CGen_EmitClosureArg(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* arg) {
    (void)self;
    (void)arg;
    if (arg == NULL) {
        return "";
    }
    if (arg->Kind == Amalgame_Compiler_NodeKind_METHOD_DECL && code_string_equals(arg->Name, "__lambda__")) {
        return Amalgame_Compiler_CGen_EmitLambdaAsClosure(self, arg);
    }
    if (arg->Kind == Amalgame_Compiler_NodeKind_IDENTIFIER) {
        code_string __attribute__((unused)) t = Amalgame_Compiler_CGen_LocalTypeGet(self, arg->Name);
        if (code_string_equals(t, "AmalgameClosure*")) {
            return arg->Name;
        }
    }
    return "";
}

static code_string Amalgame_Compiler_CGen_EmitLambdaAsClosure(Amalgame_Compiler_CGen* self, Amalgame_Compiler_AstNode* lam) {
    (void)self;
    (void)lam;
    code_string __attribute__((unused)) id = lam->Str2;
    code_string __attribute__((unused)) envName = code_string_concat("LamEnv_", id);
    code_string __attribute__((unused)) fnName = code_string_concat(code_string_concat("lam_", id), "_fn");
    code_string __attribute__((unused)) envVar = code_string_concat("__env_", id);
    i64 __attribute__((unused)) cn = AmalgameList_count(lam->Args);
    code_string __attribute__((unused)) s = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("({ ", envName), "* "), envVar), " = ("), envName), "*)code_alloc(sizeof("), envName), ")); ");
    for (i64 i = 0; i < cn; i++) {
        s = code_string_concat(s, Amalgame_Compiler_CGen_EmitLambdaCaptureCopy(self, envVar, (Amalgame_Compiler_AstNode*)AmalgameList_get(lam->Args, i)));
    }
    s = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(s, "AmalgameClosure_new((void*)"), fnName), ", "), envVar), "); })");
    return s;
}

static code_string Amalgame_Compiler_CGen_EmitLambdaCaptureCopy(Amalgame_Compiler_CGen* self, code_string envVar, Amalgame_Compiler_AstNode* cap) {
    (void)self;
    (void)envVar;
    (void)cap;
    return code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(envVar, "->_"), cap->Name), " = "), cap->Name), "; ");
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
            if (lk == Amalgame_Compiler_NodeKind_LITERAL_STRING) {
                return code_string_concat("String_", mname);
            }
            if (lk == Amalgame_Compiler_NodeKind_IDENTIFIER) {
                code_string __attribute__((unused)) tname = callee->Left->Name;
                code_string __attribute__((unused)) firstChar = String_Substring(tname, 0, 1);
                code_bool __attribute__((unused)) isUpper = code_string_equals(firstChar, String_ToUpper(firstChar));
                if (isUpper) {
                    code_bool __attribute__((unused)) isStdlib = code_string_equals(tname, "Console") || code_string_equals(tname, "File") || code_string_equals(tname, "Math") || code_string_equals(tname, "String") || code_string_equals(tname, "List") || code_string_equals(tname, "Env") || code_string_equals(tname, "Process");
                    if (isStdlib) {
                        return code_string_concat(code_string_concat(tname, "_"), mname);
                    }
                    return code_string_concat(code_string_concat(Amalgame_Compiler_CGen_SymName(self, tname), "_"), mname);
                }
                code_string __attribute__((unused)) varType = Amalgame_Compiler_CGen_LocalTypeGet(self, tname);
                code_string __attribute__((unused)) bareType = String_Replace(varType, "*", "");
                if (String_Length(bareType) > 0) {
                    if (code_string_equals(bareType, "code_string")) {
                        return code_string_concat("String_", mname);
                    }
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

static code_bool Amalgame_Compiler_CGen_IsCPointerType(Amalgame_Compiler_CGen* self, code_string ct) {
    (void)self;
    (void)ct;
    if (code_string_equals(ct, "code_string")) {
        return 1;
    }
    if (String_EndsWith(ct, "*")) {
        return 1;
    }
    return 0;
}

static code_string Amalgame_Compiler_CGen_BoxAsVoid(Amalgame_Compiler_CGen* self, code_string expr) {
    (void)self;
    (void)expr;
    return code_string_concat(code_string_concat("(void*)(intptr_t)(", expr), ")");
}

static code_string Amalgame_Compiler_CGen_UnboxScalar(Amalgame_Compiler_CGen* self, code_string ctype, code_string expr) {
    (void)self;
    (void)ctype;
    (void)expr;
    return code_string_concat(code_string_concat(code_string_concat("(", ctype), ")(intptr_t)"), expr);
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
        return Amalgame_Compiler_CGen_TypeToC(self, inner);
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

struct _Amalgame_Compiler_Formatter {
    AmalgameList* Out;
    code_string Buf;
    i64 Indent;
    AmalgameList* Comments;
    i64 CommentPos;
    i64 CommentCnt;
    i64 LastLine;
};

code_string Amalgame_Compiler_Formatter_Format(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* prog);
static code_string Amalgame_Compiler_Formatter_IndentStr(Amalgame_Compiler_Formatter* self);
static void Amalgame_Compiler_Formatter_Begin(Amalgame_Compiler_Formatter* self);
static void Amalgame_Compiler_Formatter_Write(Amalgame_Compiler_Formatter* self, code_string s);
static void Amalgame_Compiler_Formatter_End(Amalgame_Compiler_Formatter* self);
static void Amalgame_Compiler_Formatter_Line(Amalgame_Compiler_Formatter* self, code_string s);
static void Amalgame_Compiler_Formatter_Close(Amalgame_Compiler_Formatter* self, code_string s);
static void Amalgame_Compiler_Formatter_Blank(Amalgame_Compiler_Formatter* self);
static void Amalgame_Compiler_Formatter_In_(Amalgame_Compiler_Formatter* self);
static void Amalgame_Compiler_Formatter_De_(Amalgame_Compiler_Formatter* self);
static void Amalgame_Compiler_Formatter_AttachToLastEmitted(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_Token* c);
static void Amalgame_Compiler_Formatter_DrainTrailingForLastLine(Amalgame_Compiler_Formatter* self);
static void Amalgame_Compiler_Formatter_Sync(Amalgame_Compiler_Formatter* self, i64 line);
static void Amalgame_Compiler_Formatter_FlushTrailingComments(Amalgame_Compiler_Formatter* self);
static void Amalgame_Compiler_Formatter_EmitProgram(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* prog);
static void Amalgame_Compiler_Formatter_EmitTopDecl(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n);
static void Amalgame_Compiler_Formatter_EmitClass(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n);
static void Amalgame_Compiler_Formatter_EmitClassMember(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* m, code_string className);
static void Amalgame_Compiler_Formatter_EmitMethod(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n, code_string className);
static code_string Amalgame_Compiler_Formatter_EmitParams(Amalgame_Compiler_Formatter* self, AmalgameList* params);
static void Amalgame_Compiler_Formatter_EmitField(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n);
static void Amalgame_Compiler_Formatter_EmitEnum(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n);
static i64 Amalgame_Compiler_Formatter_MaxLineInTree(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n);
static i64 Amalgame_Compiler_Formatter_BlockEndLine(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* body);
static void Amalgame_Compiler_Formatter_EmitBlockStmts(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* body);
static void Amalgame_Compiler_Formatter_EmitInline(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* body);
static void Amalgame_Compiler_Formatter_EmitStmt(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n);
static void Amalgame_Compiler_Formatter_EmitTry(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n);
static void Amalgame_Compiler_Formatter_EmitThrow(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n);
static void Amalgame_Compiler_Formatter_EmitBlock(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n);
static void Amalgame_Compiler_Formatter_EmitIf(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n);
static void Amalgame_Compiler_Formatter_EmitElseTail(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n);
static void Amalgame_Compiler_Formatter_EmitMatch(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n);
static code_string Amalgame_Compiler_Formatter_EmitPattern(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n);
static void Amalgame_Compiler_Formatter_EmitWhile(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n);
static void Amalgame_Compiler_Formatter_EmitForIn(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n);
static void Amalgame_Compiler_Formatter_EmitVarDecl(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n, code_bool topLevel);
static void Amalgame_Compiler_Formatter_EmitReturn(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n);
static code_string Amalgame_Compiler_Formatter_EmitExpr(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n);
static code_string Amalgame_Compiler_Formatter_EmitBinary(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n);
static code_string Amalgame_Compiler_Formatter_EmitUnary(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n);
static code_string Amalgame_Compiler_Formatter_EmitCall(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n);
static code_string Amalgame_Compiler_Formatter_EmitMember(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n);
static code_string Amalgame_Compiler_Formatter_EmitIndex(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n);
static code_string Amalgame_Compiler_Formatter_EmitMatchExpr(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n);
static code_string Amalgame_Compiler_Formatter_EmitIfExpr(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n);
static code_string Amalgame_Compiler_Formatter_EmitLambda(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n);
static code_string Amalgame_Compiler_Formatter_LambdaParamName(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* p);
static code_string Amalgame_Compiler_Formatter_EmitBlockAsExpr(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* body);
static code_string Amalgame_Compiler_Formatter_EmitNew(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n);
static code_string Amalgame_Compiler_Formatter_QuoteString(Amalgame_Compiler_Formatter* self, code_string content);
static code_string Amalgame_Compiler_Formatter_KindName(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_NodeKind k);

Amalgame_Compiler_Formatter* Amalgame_Compiler_Formatter_new(AmalgameList* comments) {
    Amalgame_Compiler_Formatter* self = (Amalgame_Compiler_Formatter*) GC_MALLOC(sizeof(Amalgame_Compiler_Formatter));
    self->Out = AmalgameList_new();
    self->Buf = "";
    self->Indent = 0;
    self->Comments = comments;
    self->CommentPos = 0;
    self->CommentCnt = AmalgameList_count(comments);
    self->LastLine = 0;
    return self;
}

code_string Amalgame_Compiler_Formatter_Format(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* prog) {
    (void)self;
    (void)prog;
    Amalgame_Compiler_Formatter_EmitProgram(self, prog);
    Amalgame_Compiler_Formatter_FlushTrailingComments(self);
    code_string __attribute__((unused)) result = "";
    i64 __attribute__((unused)) n = AmalgameList_count(self->Out);
    for (i64 i = 0; i < n; i++) {
        code_string __attribute__((unused)) line = (code_string)AmalgameList_get(self->Out, i);
        result = code_string_concat(code_string_concat(result, line), "\n");
    }
    return result;
}

static code_string Amalgame_Compiler_Formatter_IndentStr(Amalgame_Compiler_Formatter* self) {
    (void)self;
    code_string __attribute__((unused)) s = "";
    i64 __attribute__((unused)) i = 0;
    while (i < self->Indent) {
        s = code_string_concat(s, "    ");
        i = i + 1;
    }
    return s;
}

static void Amalgame_Compiler_Formatter_Begin(Amalgame_Compiler_Formatter* self) {
    (void)self;
    self->Buf = Amalgame_Compiler_Formatter_IndentStr(self);
}

static void Amalgame_Compiler_Formatter_Write(Amalgame_Compiler_Formatter* self, code_string s) {
    (void)self;
    (void)s;
    self->Buf = code_string_concat(self->Buf, s);
}

static void Amalgame_Compiler_Formatter_End(Amalgame_Compiler_Formatter* self) {
    (void)self;
    AmalgameList_add(self->Out, (void*)(intptr_t)(self->Buf));
    self->Buf = "";
}

static void Amalgame_Compiler_Formatter_Line(Amalgame_Compiler_Formatter* self, code_string s) {
    (void)self;
    (void)s;
    Amalgame_Compiler_Formatter_Begin(self);
    Amalgame_Compiler_Formatter_Write(self, s);
    Amalgame_Compiler_Formatter_End(self);
}

static void Amalgame_Compiler_Formatter_Close(Amalgame_Compiler_Formatter* self, code_string s) {
    (void)self;
    (void)s;
    Amalgame_Compiler_Formatter_Line(self, s);
}

static void Amalgame_Compiler_Formatter_Blank(Amalgame_Compiler_Formatter* self) {
    (void)self;
    AmalgameList_add(self->Out, (void*)(intptr_t)(""));
}

static void Amalgame_Compiler_Formatter_In_(Amalgame_Compiler_Formatter* self) {
    (void)self;
    self->Indent = self->Indent + 1;
}

static void Amalgame_Compiler_Formatter_De_(Amalgame_Compiler_Formatter* self) {
    (void)self;
    self->Indent = self->Indent - 1;
}

static void Amalgame_Compiler_Formatter_AttachToLastEmitted(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_Token* c) {
    (void)self;
    (void)c;
    i64 __attribute__((unused)) last = AmalgameList_count(self->Out) - 1;
    if (last < 0) {
        return;
    }
    code_string __attribute__((unused)) prev = (code_string)AmalgameList_get(self->Out, last);
    AmalgameList_removeAt(self->Out, last);
    AmalgameList_add(self->Out, (void*)(intptr_t)(code_string_concat(code_string_concat(prev, "  "), c->Value)));
}

static void Amalgame_Compiler_Formatter_DrainTrailingForLastLine(Amalgame_Compiler_Formatter* self) {
    (void)self;
    while (self->CommentPos < self->CommentCnt && self->LastLine > 0) {
        Amalgame_Compiler_Token* __attribute__((unused)) c = (Amalgame_Compiler_Token*)AmalgameList_get(self->Comments, self->CommentPos);
        if (c->Line != self->LastLine) {
            break;
        }
        Amalgame_Compiler_Formatter_AttachToLastEmitted(self, c);
        self->CommentPos = self->CommentPos + 1;
    }
}

static void Amalgame_Compiler_Formatter_Sync(Amalgame_Compiler_Formatter* self, i64 line) {
    (void)self;
    (void)line;
    Amalgame_Compiler_Formatter_DrainTrailingForLastLine(self);
    while (self->CommentPos < self->CommentCnt) {
        Amalgame_Compiler_Token* __attribute__((unused)) c = (Amalgame_Compiler_Token*)AmalgameList_get(self->Comments, self->CommentPos);
        if (c->Line >= line) {
            break;
        }
        if (self->LastLine > 0 && c->Line - self->LastLine >= 2) {
            Amalgame_Compiler_Formatter_Blank(self);
        }
        Amalgame_Compiler_Formatter_Line(self, c->Value);
        self->LastLine = c->Line;
        self->CommentPos = self->CommentPos + 1;
    }
    if (self->LastLine > 0 && line - self->LastLine >= 2) {
        Amalgame_Compiler_Formatter_Blank(self);
    }
    if (line > 0) {
        self->LastLine = line;
    }
}

static void Amalgame_Compiler_Formatter_FlushTrailingComments(Amalgame_Compiler_Formatter* self) {
    (void)self;
    Amalgame_Compiler_Formatter_DrainTrailingForLastLine(self);
    while (self->CommentPos < self->CommentCnt) {
        Amalgame_Compiler_Token* __attribute__((unused)) c = (Amalgame_Compiler_Token*)AmalgameList_get(self->Comments, self->CommentPos);
        if (self->LastLine > 0 && c->Line - self->LastLine >= 2) {
            Amalgame_Compiler_Formatter_Blank(self);
        }
        Amalgame_Compiler_Formatter_Line(self, c->Value);
        self->LastLine = c->Line;
        self->CommentPos = self->CommentPos + 1;
    }
}

static void Amalgame_Compiler_Formatter_EmitProgram(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* prog) {
    (void)self;
    (void)prog;
    if (String_Length(prog->Str) > 0) {
        Amalgame_Compiler_Formatter_Sync(self, 1);
        Amalgame_Compiler_Formatter_Line(self, code_string_concat("namespace ", prog->Str));
    }
    i64 __attribute__((unused)) importCount = AmalgameList_count(prog->Args);
    for (i64 ii = 0; ii < importCount; ii++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) imp = (Amalgame_Compiler_AstNode*)AmalgameList_get(prog->Args, ii);
        Amalgame_Compiler_Formatter_Sync(self, imp->Line);
        Amalgame_Compiler_Formatter_Line(self, code_string_concat("import ", imp->Name));
    }
    i64 __attribute__((unused)) n = AmalgameList_count(prog->Children);
    for (i64 i = 0; i < n; i++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) decl = (Amalgame_Compiler_AstNode*)AmalgameList_get(prog->Children, i);
        Amalgame_Compiler_Formatter_Sync(self, decl->Line);
        Amalgame_Compiler_Formatter_EmitTopDecl(self, decl);
    }
}

static void Amalgame_Compiler_Formatter_EmitTopDecl(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n) {
    (void)self;
    (void)n;
    Amalgame_Compiler_NodeKind __attribute__((unused)) k = n->Kind;
    if (k == Amalgame_Compiler_NodeKind_CLASS_DECL) {
        Amalgame_Compiler_Formatter_EmitClass(self, n);
    } else if (k == Amalgame_Compiler_NodeKind_METHOD_DECL) {
        Amalgame_Compiler_Formatter_EmitMethod(self, n, "");
    } else if (k == Amalgame_Compiler_NodeKind_ENUM_DECL) {
        Amalgame_Compiler_Formatter_EmitEnum(self, n);
    } else if (k == Amalgame_Compiler_NodeKind_VAR_DECL) {
        Amalgame_Compiler_Formatter_EmitVarDecl(self, n, 1);
    } else {
        Amalgame_Compiler_Formatter_Line(self, code_string_concat(code_string_concat("// TODO: top-level kind ", Amalgame_Compiler_Formatter_KindName(self, k)), ""));
    }
}

static void Amalgame_Compiler_Formatter_EmitClass(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n) {
    (void)self;
    (void)n;
    code_string __attribute__((unused)) head = "";
    if (n->Flag) {
        head = code_string_concat(head, "public ");
    }
    if (n->Flag2) {
        head = code_string_concat(code_string_concat(head, "interface "), n->Name);
    } else {
        head = code_string_concat(code_string_concat(head, "class "), n->Name);
        if (String_Length(n->Str) > 0) {
            head = code_string_concat(code_string_concat(head, " : "), n->Str);
        }
    }
    head = code_string_concat(head, " {");
    Amalgame_Compiler_Formatter_Line(self, head);
    Amalgame_Compiler_Formatter_In_(self);
    self->LastLine = n->Line;
    i64 __attribute__((unused)) count = AmalgameList_count(n->Children);
    code_string __attribute__((unused)) className = n->Name;
    for (i64 i = 0; i < count; i++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) m = (Amalgame_Compiler_AstNode*)AmalgameList_get(n->Children, i);
        Amalgame_Compiler_Formatter_Sync(self, m->Line);
        Amalgame_Compiler_Formatter_EmitClassMember(self, m, className);
    }
    if (String_Length(n->Str2) > 0) {
        self->LastLine = String_ToInt(n->Str2);
    } else if (count > 0) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) last = (Amalgame_Compiler_AstNode*)AmalgameList_get(n->Children, count - 1);
        self->LastLine = Amalgame_Compiler_Formatter_MaxLineInTree(self, last) + 1;
    }
    Amalgame_Compiler_Formatter_De_(self);
    Amalgame_Compiler_Formatter_Close(self, "}");
}

static void Amalgame_Compiler_Formatter_EmitClassMember(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* m, code_string className) {
    (void)self;
    (void)m;
    (void)className;
    Amalgame_Compiler_NodeKind __attribute__((unused)) k = m->Kind;
    if (k == Amalgame_Compiler_NodeKind_METHOD_DECL) {
        Amalgame_Compiler_Formatter_EmitMethod(self, m, className);
    } else if (k == Amalgame_Compiler_NodeKind_VAR_DECL) {
        Amalgame_Compiler_Formatter_EmitField(self, m);
    } else {
        Amalgame_Compiler_Formatter_Line(self, code_string_concat(code_string_concat("// TODO: member kind ", Amalgame_Compiler_Formatter_KindName(self, k)), ""));
    }
}

static void Amalgame_Compiler_Formatter_EmitMethod(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n, code_string className) {
    (void)self;
    (void)n;
    (void)className;
    if (String_Length(n->Str2) > 0) {
        code_string __attribute__((unused)) decos = n->Str2;
        i64 __attribute__((unused)) i = 0;
        i64 __attribute__((unused)) len = String_Length(decos);
        code_string __attribute__((unused)) cur = "";
        while (i < len) {
            code_string __attribute__((unused)) ch = String_Substring(decos, i, 1);
            if (code_string_equals(ch, ",")) {
                if (String_Length(cur) > 0) {
                    Amalgame_Compiler_Formatter_Line(self, code_string_concat("@", cur));
                }
                cur = "";
            } else {
                cur = code_string_concat(cur, ch);
            }
            i = i + 1;
        }
        if (String_Length(cur) > 0) {
            Amalgame_Compiler_Formatter_Line(self, code_string_concat("@", cur));
        }
    }
    code_string __attribute__((unused)) head = "";
    if (n->Flag) {
        head = code_string_concat(head, "public ");
    }
    if (n->Flag2) {
        head = code_string_concat(head, "static ");
    }
    code_bool __attribute__((unused)) isCtor = String_Length(className) > 0 && code_string_equals(n->Name, className);
    if (isCtor) {
        head = code_string_concat(code_string_concat(head, n->Name), "(");
    } else {
        head = code_string_concat(code_string_concat(code_string_concat(code_string_concat(head, n->Str), " "), n->Name), "(");
    }
    head = code_string_concat(head, Amalgame_Compiler_Formatter_EmitParams(self, n->Params));
    head = code_string_concat(head, ")");
    if (n->Body == NULL) {
        Amalgame_Compiler_Formatter_Line(self, head);
        return;
    }
    head = code_string_concat(head, " {");
    Amalgame_Compiler_Formatter_Line(self, head);
    Amalgame_Compiler_Formatter_In_(self);
    Amalgame_Compiler_AstNode* __attribute__((unused)) body = n->Body;
    Amalgame_Compiler_Formatter_EmitBlockStmts(self, body);
    Amalgame_Compiler_Formatter_De_(self);
    Amalgame_Compiler_Formatter_Close(self, "}");
}

static code_string Amalgame_Compiler_Formatter_EmitParams(Amalgame_Compiler_Formatter* self, AmalgameList* params) {
    (void)self;
    (void)params;
    code_string __attribute__((unused)) s = "";
    i64 __attribute__((unused)) count = AmalgameList_count(params);
    for (i64 i = 0; i < count; i++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) p = (Amalgame_Compiler_AstNode*)AmalgameList_get(params, i);
        if (i > 0) {
            s = code_string_concat(s, ", ");
        }
        s = code_string_concat(code_string_concat(code_string_concat(s, p->Str), " "), p->Name);
    }
    return s;
}

static void Amalgame_Compiler_Formatter_EmitField(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n) {
    (void)self;
    (void)n;
    code_string __attribute__((unused)) s = "";
    if (n->Flag) {
        s = code_string_concat(s, "public ");
    }
    s = code_string_concat(s, n->Name);
    if (String_Length(n->Str) > 0) {
        s = code_string_concat(code_string_concat(s, ": "), n->Str);
    }
    if (n->Left != NULL) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) init = n->Left;
        s = code_string_concat(code_string_concat(s, " = "), Amalgame_Compiler_Formatter_EmitExpr(self, init));
    }
    Amalgame_Compiler_Formatter_Line(self, s);
}

static void Amalgame_Compiler_Formatter_EmitEnum(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n) {
    (void)self;
    (void)n;
    code_string __attribute__((unused)) head = "";
    if (n->Flag) {
        head = code_string_concat(head, "public ");
    }
    head = code_string_concat(code_string_concat(code_string_concat(head, "enum "), n->Name), " {");
    Amalgame_Compiler_Formatter_Line(self, head);
    Amalgame_Compiler_Formatter_In_(self);
    self->LastLine = n->Line;
    i64 __attribute__((unused)) c = AmalgameList_count(n->Children);
    for (i64 i = 0; i < c; i++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) v = (Amalgame_Compiler_AstNode*)AmalgameList_get(n->Children, i);
        Amalgame_Compiler_Formatter_Sync(self, v->Line);
        if (String_Length(v->Str) > 0) {
            Amalgame_Compiler_Formatter_Line(self, code_string_concat(code_string_concat(code_string_concat(v->Name, "("), v->Str), ")"));
        } else {
            Amalgame_Compiler_Formatter_Line(self, v->Name);
        }
    }
    if (c > 0) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) last = (Amalgame_Compiler_AstNode*)AmalgameList_get(n->Children, c - 1);
        self->LastLine = last->Line + 1;
    }
    Amalgame_Compiler_Formatter_De_(self);
    Amalgame_Compiler_Formatter_Close(self, "}");
}

static i64 Amalgame_Compiler_Formatter_MaxLineInTree(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n) {
    (void)self;
    (void)n;
    if (n == NULL) {
        return 0;
    }
    i64 __attribute__((unused)) m = n->Line;
    i64 __attribute__((unused)) lm = Amalgame_Compiler_Formatter_MaxLineInTree(self, n->Left);
    if (lm > m) {
        m = lm;
    }
    i64 __attribute__((unused)) rm = Amalgame_Compiler_Formatter_MaxLineInTree(self, n->Right);
    if (rm > m) {
        m = rm;
    }
    i64 __attribute__((unused)) cm = Amalgame_Compiler_Formatter_MaxLineInTree(self, n->Cond);
    if (cm > m) {
        m = cm;
    }
    i64 __attribute__((unused)) bm = Amalgame_Compiler_Formatter_MaxLineInTree(self, n->Body);
    if (bm > m) {
        m = bm;
    }
    i64 __attribute__((unused)) em = Amalgame_Compiler_Formatter_MaxLineInTree(self, n->Else);
    if (em > m) {
        m = em;
    }
    i64 __attribute__((unused)) cc = AmalgameList_count(n->Children);
    for (i64 i = 0; i < cc; i++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) chi = (Amalgame_Compiler_AstNode*)AmalgameList_get(n->Children, i);
        i64 __attribute__((unused)) cmm = Amalgame_Compiler_Formatter_MaxLineInTree(self, chi);
        if (cmm > m) {
            m = cmm;
        }
    }
    i64 __attribute__((unused)) pc = AmalgameList_count(n->Params);
    for (i64 j = 0; j < pc; j++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) pj = (Amalgame_Compiler_AstNode*)AmalgameList_get(n->Params, j);
        i64 __attribute__((unused)) pmm = Amalgame_Compiler_Formatter_MaxLineInTree(self, pj);
        if (pmm > m) {
            m = pmm;
        }
    }
    i64 __attribute__((unused)) ac = AmalgameList_count(n->Args);
    for (i64 k = 0; k < ac; k++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) ak = (Amalgame_Compiler_AstNode*)AmalgameList_get(n->Args, k);
        i64 __attribute__((unused)) amm = Amalgame_Compiler_Formatter_MaxLineInTree(self, ak);
        if (amm > m) {
            m = amm;
        }
    }
    return m;
}

static i64 Amalgame_Compiler_Formatter_BlockEndLine(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* body) {
    (void)self;
    (void)body;
    if (String_Length(body->Str2) > 0) {
        return String_ToInt(body->Str2);
    }
    return Amalgame_Compiler_Formatter_MaxLineInTree(self, body) + 1;
}

static void Amalgame_Compiler_Formatter_EmitBlockStmts(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* body) {
    (void)self;
    (void)body;
    self->LastLine = body->Line;
    i64 __attribute__((unused)) count = AmalgameList_count(body->Children);
    for (i64 i = 0; i < count; i++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) s = (Amalgame_Compiler_AstNode*)AmalgameList_get(body->Children, i);
        Amalgame_Compiler_Formatter_Sync(self, s->Line);
        Amalgame_Compiler_Formatter_EmitStmt(self, s);
    }
    Amalgame_Compiler_Formatter_DrainTrailingForLastLine(self);
    self->LastLine = Amalgame_Compiler_Formatter_BlockEndLine(self, body);
}

static void Amalgame_Compiler_Formatter_EmitInline(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* body) {
    (void)self;
    (void)body;
    if (body->Kind == Amalgame_Compiler_NodeKind_BLOCK) {
        Amalgame_Compiler_Formatter_EmitBlockStmts(self, body);
    } else {
        self->LastLine = body->Line;
        Amalgame_Compiler_Formatter_EmitStmt(self, body);
        Amalgame_Compiler_Formatter_DrainTrailingForLastLine(self);
        self->LastLine = Amalgame_Compiler_Formatter_MaxLineInTree(self, body) + 1;
    }
}

static void Amalgame_Compiler_Formatter_EmitStmt(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n) {
    (void)self;
    (void)n;
    Amalgame_Compiler_NodeKind __attribute__((unused)) k = n->Kind;
    if (k == Amalgame_Compiler_NodeKind_IF_STMT) {
        if (code_string_equals(n->Name, "__match__")) {
            Amalgame_Compiler_Formatter_EmitMatch(self, n);
        } else {
            Amalgame_Compiler_Formatter_EmitIf(self, n);
        }
    } else if (k == Amalgame_Compiler_NodeKind_WHILE_STMT) {
        Amalgame_Compiler_Formatter_EmitWhile(self, n);
    } else if (k == Amalgame_Compiler_NodeKind_FOR_IN_STMT) {
        Amalgame_Compiler_Formatter_EmitForIn(self, n);
    } else if (k == Amalgame_Compiler_NodeKind_VAR_DECL) {
        Amalgame_Compiler_Formatter_EmitVarDecl(self, n, 0);
    } else if (k == Amalgame_Compiler_NodeKind_RETURN_STMT) {
        Amalgame_Compiler_Formatter_EmitReturn(self, n);
    } else if (k == Amalgame_Compiler_NodeKind_BREAK_STMT) {
        Amalgame_Compiler_Formatter_Line(self, "break");
    } else if (k == Amalgame_Compiler_NodeKind_CONTINUE_STMT) {
        Amalgame_Compiler_Formatter_Line(self, "continue");
    } else if (k == Amalgame_Compiler_NodeKind_TRY_STMT) {
        Amalgame_Compiler_Formatter_EmitTry(self, n);
    } else if (k == Amalgame_Compiler_NodeKind_THROW_STMT) {
        Amalgame_Compiler_Formatter_EmitThrow(self, n);
    } else if (k == Amalgame_Compiler_NodeKind_BLOCK) {
        Amalgame_Compiler_Formatter_EmitBlock(self, n);
    } else {
        Amalgame_Compiler_Formatter_Line(self, Amalgame_Compiler_Formatter_EmitExpr(self, n));
    }
}

static void Amalgame_Compiler_Formatter_EmitTry(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n) {
    (void)self;
    (void)n;
    Amalgame_Compiler_Formatter_Line(self, "try {");
    Amalgame_Compiler_Formatter_In_(self);
    if (n->Body != NULL) {
        Amalgame_Compiler_Formatter_EmitInline(self, n->Body);
    }
    Amalgame_Compiler_Formatter_De_(self);
    if (n->Else != NULL) {
        code_string __attribute__((unused)) head = "} catch";
        if (String_Length(n->Name) > 0) {
            head = code_string_concat(code_string_concat(head, " "), n->Name);
        }
        head = code_string_concat(head, " {");
        Amalgame_Compiler_Formatter_Line(self, head);
        Amalgame_Compiler_Formatter_In_(self);
        Amalgame_Compiler_Formatter_EmitInline(self, n->Else);
        Amalgame_Compiler_Formatter_De_(self);
    }
    if (n->Cond != NULL) {
        Amalgame_Compiler_Formatter_Line(self, "} finally {");
        Amalgame_Compiler_Formatter_In_(self);
        Amalgame_Compiler_Formatter_EmitInline(self, n->Cond);
        Amalgame_Compiler_Formatter_De_(self);
    }
    Amalgame_Compiler_Formatter_Close(self, "}");
}

static void Amalgame_Compiler_Formatter_EmitThrow(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n) {
    (void)self;
    (void)n;
    if (n->Left == NULL) {
        Amalgame_Compiler_Formatter_Line(self, "throw");
        return;
    }
    Amalgame_Compiler_Formatter_Line(self, code_string_concat("throw ", Amalgame_Compiler_Formatter_EmitExpr(self, n->Left)));
}

static void Amalgame_Compiler_Formatter_EmitBlock(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n) {
    (void)self;
    (void)n;
    Amalgame_Compiler_Formatter_Line(self, "{");
    Amalgame_Compiler_Formatter_In_(self);
    Amalgame_Compiler_Formatter_EmitBlockStmts(self, n);
    Amalgame_Compiler_Formatter_De_(self);
    Amalgame_Compiler_Formatter_Close(self, "}");
}

static void Amalgame_Compiler_Formatter_EmitIf(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n) {
    (void)self;
    (void)n;
    code_string __attribute__((unused)) condStr = Amalgame_Compiler_Formatter_EmitExpr(self, n->Cond);
    Amalgame_Compiler_Formatter_Line(self, code_string_concat(code_string_concat("if (", condStr), ") {"));
    Amalgame_Compiler_Formatter_In_(self);
    Amalgame_Compiler_Formatter_EmitInline(self, n->Body);
    Amalgame_Compiler_Formatter_De_(self);
    if (n->Else != NULL) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) elseN = n->Else;
        if (elseN->Kind == Amalgame_Compiler_NodeKind_IF_STMT && !code_string_equals(elseN->Name, "__match__")) {
            code_string __attribute__((unused)) inner = Amalgame_Compiler_Formatter_EmitExpr(self, elseN->Cond);
            Amalgame_Compiler_Formatter_Line(self, code_string_concat(code_string_concat("} else if (", inner), ") {"));
            Amalgame_Compiler_Formatter_In_(self);
            Amalgame_Compiler_Formatter_EmitInline(self, elseN->Body);
            Amalgame_Compiler_Formatter_De_(self);
            if (elseN->Else != NULL) {
                Amalgame_Compiler_Formatter_EmitElseTail(self, elseN->Else);
            } else {
                Amalgame_Compiler_Formatter_Close(self, "}");
            }
        } else {
            Amalgame_Compiler_Formatter_Line(self, "} else {");
            Amalgame_Compiler_Formatter_In_(self);
            Amalgame_Compiler_Formatter_EmitInline(self, elseN);
            Amalgame_Compiler_Formatter_De_(self);
            Amalgame_Compiler_Formatter_Close(self, "}");
        }
    } else {
        Amalgame_Compiler_Formatter_Close(self, "}");
    }
}

static void Amalgame_Compiler_Formatter_EmitElseTail(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n) {
    (void)self;
    (void)n;
    if (n->Kind == Amalgame_Compiler_NodeKind_IF_STMT && !code_string_equals(n->Name, "__match__")) {
        code_string __attribute__((unused)) inner = Amalgame_Compiler_Formatter_EmitExpr(self, n->Cond);
        Amalgame_Compiler_Formatter_Line(self, code_string_concat(code_string_concat("} else if (", inner), ") {"));
        Amalgame_Compiler_Formatter_In_(self);
        Amalgame_Compiler_Formatter_EmitInline(self, n->Body);
        Amalgame_Compiler_Formatter_De_(self);
        if (n->Else != NULL) {
            Amalgame_Compiler_Formatter_EmitElseTail(self, n->Else);
        } else {
            Amalgame_Compiler_Formatter_Close(self, "}");
        }
    } else {
        Amalgame_Compiler_Formatter_Line(self, "} else {");
        Amalgame_Compiler_Formatter_In_(self);
        Amalgame_Compiler_Formatter_EmitInline(self, n);
        Amalgame_Compiler_Formatter_De_(self);
        Amalgame_Compiler_Formatter_Close(self, "}");
    }
}

static void Amalgame_Compiler_Formatter_EmitMatch(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n) {
    (void)self;
    (void)n;
    code_string __attribute__((unused)) subj = Amalgame_Compiler_Formatter_EmitExpr(self, n->Left);
    Amalgame_Compiler_Formatter_Line(self, code_string_concat(code_string_concat("match ", subj), " {"));
    Amalgame_Compiler_Formatter_In_(self);
    i64 __attribute__((unused)) arms = AmalgameList_count(n->Children);
    for (i64 i = 0; i < arms; i++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) arm = (Amalgame_Compiler_AstNode*)AmalgameList_get(n->Children, i);
        code_string __attribute__((unused)) pat = Amalgame_Compiler_Formatter_EmitPattern(self, arm->Left);
        code_string __attribute__((unused)) head = pat;
        if (arm->Cond != NULL) {
            head = code_string_concat(code_string_concat(head, " if "), Amalgame_Compiler_Formatter_EmitExpr(self, arm->Cond));
        }
        head = code_string_concat(head, " => ");
        Amalgame_Compiler_AstNode* __attribute__((unused)) bodyN = arm->Right;
        if (bodyN->Kind == Amalgame_Compiler_NodeKind_BLOCK) {
            Amalgame_Compiler_Formatter_Line(self, code_string_concat(head, "{"));
            Amalgame_Compiler_Formatter_In_(self);
            Amalgame_Compiler_Formatter_EmitBlockStmts(self, bodyN);
            Amalgame_Compiler_Formatter_De_(self);
            Amalgame_Compiler_Formatter_Close(self, "}");
        } else {
            Amalgame_Compiler_Formatter_Line(self, code_string_concat(head, Amalgame_Compiler_Formatter_EmitExpr(self, bodyN)));
        }
    }
    Amalgame_Compiler_Formatter_De_(self);
    Amalgame_Compiler_Formatter_Close(self, "}");
}

static code_string Amalgame_Compiler_Formatter_EmitPattern(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n) {
    (void)self;
    (void)n;
    Amalgame_Compiler_NodeKind __attribute__((unused)) k = n->Kind;
    if (k == Amalgame_Compiler_NodeKind_IDENTIFIER) {
        return n->Name;
    }
    if (k == Amalgame_Compiler_NodeKind_LITERAL_INT) {
        return n->Str;
    }
    if (k == Amalgame_Compiler_NodeKind_BINARY && code_string_equals(n->Str, "..")) {
        return code_string_concat(code_string_concat(Amalgame_Compiler_Formatter_EmitExpr(self, n->Left), ".."), Amalgame_Compiler_Formatter_EmitExpr(self, n->Right));
    }
    if (k == Amalgame_Compiler_NodeKind_CALL) {
        code_string __attribute__((unused)) s = code_string_concat(n->Name, "(");
        i64 __attribute__((unused)) c = AmalgameList_count(n->Args);
        for (i64 i = 0; i < c; i++) {
            Amalgame_Compiler_AstNode* __attribute__((unused)) a = (Amalgame_Compiler_AstNode*)AmalgameList_get(n->Args, i);
            if (i > 0) {
                s = code_string_concat(s, ", ");
            }
            s = code_string_concat(s, a->Name);
        }
        return code_string_concat(s, ")");
    }
    return Amalgame_Compiler_Formatter_EmitExpr(self, n);
}

static void Amalgame_Compiler_Formatter_EmitWhile(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n) {
    (void)self;
    (void)n;
    code_string __attribute__((unused)) c = Amalgame_Compiler_Formatter_EmitExpr(self, n->Cond);
    Amalgame_Compiler_Formatter_Line(self, code_string_concat(code_string_concat("while (", c), ") {"));
    Amalgame_Compiler_Formatter_In_(self);
    Amalgame_Compiler_Formatter_EmitInline(self, n->Body);
    Amalgame_Compiler_Formatter_De_(self);
    Amalgame_Compiler_Formatter_Close(self, "}");
}

static void Amalgame_Compiler_Formatter_EmitForIn(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n) {
    (void)self;
    (void)n;
    code_string __attribute__((unused)) it = Amalgame_Compiler_Formatter_EmitExpr(self, n->Left);
    Amalgame_Compiler_Formatter_Line(self, code_string_concat(code_string_concat(code_string_concat(code_string_concat("for ", n->Name), " in "), it), " {"));
    Amalgame_Compiler_Formatter_In_(self);
    Amalgame_Compiler_Formatter_EmitInline(self, n->Body);
    Amalgame_Compiler_Formatter_De_(self);
    Amalgame_Compiler_Formatter_Close(self, "}");
}

static void Amalgame_Compiler_Formatter_EmitVarDecl(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n, code_bool topLevel) {
    (void)self;
    (void)n;
    (void)topLevel;
    code_string __attribute__((unused)) kw = "let";
    if (n->Flag) {
        kw = "var";
    }
    code_string __attribute__((unused)) s = code_string_concat(code_string_concat(kw, " "), n->Name);
    if (String_Length(n->Str) > 0) {
        s = code_string_concat(code_string_concat(s, ": "), n->Str);
    }
    if (n->Left != NULL) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) init = n->Left;
        s = code_string_concat(code_string_concat(s, " = "), Amalgame_Compiler_Formatter_EmitExpr(self, init));
    }
    Amalgame_Compiler_Formatter_Line(self, s);
}

static void Amalgame_Compiler_Formatter_EmitReturn(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n) {
    (void)self;
    (void)n;
    if (n->Left == NULL) {
        Amalgame_Compiler_Formatter_Line(self, "return");
        return;
    }
    Amalgame_Compiler_AstNode* __attribute__((unused)) v = n->Left;
    if (v->Kind == Amalgame_Compiler_NodeKind_IDENTIFIER && code_string_equals(v->Name, "_unknown_")) {
        Amalgame_Compiler_Formatter_Line(self, "return");
        return;
    }
    Amalgame_Compiler_Formatter_Line(self, code_string_concat("return ", Amalgame_Compiler_Formatter_EmitExpr(self, v)));
}

static code_string Amalgame_Compiler_Formatter_EmitExpr(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n) {
    (void)self;
    (void)n;
    if (n == NULL) {
        return "";
    }
    Amalgame_Compiler_NodeKind __attribute__((unused)) k = n->Kind;
    if (k == Amalgame_Compiler_NodeKind_IDENTIFIER) {
        return n->Name;
    }
    if (k == Amalgame_Compiler_NodeKind_LITERAL_INT) {
        return n->Str;
    }
    if (k == Amalgame_Compiler_NodeKind_LITERAL_FLOAT) {
        return n->Str;
    }
    if (k == Amalgame_Compiler_NodeKind_LITERAL_STRING) {
        return Amalgame_Compiler_Formatter_QuoteString(self, n->Str);
    }
    if (k == Amalgame_Compiler_NodeKind_LITERAL_BOOL) {
        if (n->Flag) {
            return "true";
        }
        return "false";
    }
    if (k == Amalgame_Compiler_NodeKind_LITERAL_NULL) {
        return "null";
    }
    if (k == Amalgame_Compiler_NodeKind_THIS_EXPR) {
        return "this";
    }
    if (k == Amalgame_Compiler_NodeKind_BINARY) {
        return Amalgame_Compiler_Formatter_EmitBinary(self, n);
    }
    if (k == Amalgame_Compiler_NodeKind_UNARY) {
        return Amalgame_Compiler_Formatter_EmitUnary(self, n);
    }
    if (k == Amalgame_Compiler_NodeKind_CALL) {
        return Amalgame_Compiler_Formatter_EmitCall(self, n);
    }
    if (k == Amalgame_Compiler_NodeKind_MEMBER) {
        return Amalgame_Compiler_Formatter_EmitMember(self, n);
    }
    if (k == Amalgame_Compiler_NodeKind_INDEX_EXPR) {
        return Amalgame_Compiler_Formatter_EmitIndex(self, n);
    }
    if (k == Amalgame_Compiler_NodeKind_NEW_EXPR) {
        return Amalgame_Compiler_Formatter_EmitNew(self, n);
    }
    if (k == Amalgame_Compiler_NodeKind_METHOD_DECL && code_string_equals(n->Name, "__lambda__")) {
        return Amalgame_Compiler_Formatter_EmitLambda(self, n);
    }
    if (k == Amalgame_Compiler_NodeKind_IF_STMT && !code_string_equals(n->Name, "__match__")) {
        return Amalgame_Compiler_Formatter_EmitIfExpr(self, n);
    }
    if (k == Amalgame_Compiler_NodeKind_IF_STMT && code_string_equals(n->Name, "__match__")) {
        return Amalgame_Compiler_Formatter_EmitMatchExpr(self, n);
    }
    if (k == Amalgame_Compiler_NodeKind_LIST_COMP) {
        code_string __attribute__((unused)) s = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("[", Amalgame_Compiler_Formatter_EmitExpr(self, n->Left)), " for "), n->Str), " in "), Amalgame_Compiler_Formatter_EmitExpr(self, n->Right));
        if (n->Cond != NULL) {
            s = code_string_concat(code_string_concat(s, " if "), Amalgame_Compiler_Formatter_EmitExpr(self, n->Cond));
        }
        return code_string_concat(s, "]");
    }
    return code_string_concat(code_string_concat("_TODO_", Amalgame_Compiler_Formatter_KindName(self, k)), "");
}

static code_string Amalgame_Compiler_Formatter_EmitBinary(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n) {
    (void)self;
    (void)n;
    code_string __attribute__((unused)) l = Amalgame_Compiler_Formatter_EmitExpr(self, n->Left);
    code_string __attribute__((unused)) r = Amalgame_Compiler_Formatter_EmitExpr(self, n->Right);
    if (code_string_equals(n->Str, "..")) {
        return code_string_concat(code_string_concat(l, ".."), r);
    }
    return code_string_concat(code_string_concat(code_string_concat(code_string_concat(l, " "), n->Str), " "), r);
}

static code_string Amalgame_Compiler_Formatter_EmitUnary(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n) {
    (void)self;
    (void)n;
    return code_string_concat(n->Str, Amalgame_Compiler_Formatter_EmitExpr(self, n->Left));
}

static code_string Amalgame_Compiler_Formatter_EmitCall(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n) {
    (void)self;
    (void)n;
    if (code_string_equals(n->Name, "__tuple_literal__")) {
        code_string __attribute__((unused)) t = "(";
        i64 __attribute__((unused)) c = AmalgameList_count(n->Args);
        for (i64 i = 0; i < c; i++) {
            if (i > 0) {
                t = code_string_concat(t, ", ");
            }
            Amalgame_Compiler_AstNode* __attribute__((unused)) a = (Amalgame_Compiler_AstNode*)AmalgameList_get(n->Args, i);
            t = code_string_concat(t, Amalgame_Compiler_Formatter_EmitExpr(self, a));
        }
        return code_string_concat(t, ")");
    }
    code_string __attribute__((unused)) callee = Amalgame_Compiler_Formatter_EmitExpr(self, n->Left);
    code_string __attribute__((unused)) s = code_string_concat(callee, "(");
    i64 __attribute__((unused)) c = AmalgameList_count(n->Args);
    for (i64 i = 0; i < c; i++) {
        if (i > 0) {
            s = code_string_concat(s, ", ");
        }
        Amalgame_Compiler_AstNode* __attribute__((unused)) a = (Amalgame_Compiler_AstNode*)AmalgameList_get(n->Args, i);
        if (String_Length(a->Str2) > 0) {
            s = code_string_concat(code_string_concat(code_string_concat(s, a->Str2), ": "), Amalgame_Compiler_Formatter_EmitExpr(self, a));
        } else {
            s = code_string_concat(s, Amalgame_Compiler_Formatter_EmitExpr(self, a));
        }
    }
    return code_string_concat(s, ")");
}

static code_string Amalgame_Compiler_Formatter_EmitMember(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n) {
    (void)self;
    (void)n;
    code_string __attribute__((unused)) base = Amalgame_Compiler_Formatter_EmitExpr(self, n->Left);
    code_string __attribute__((unused)) sep = (n->Flag ? "?." : ".");
    return code_string_concat(code_string_concat(base, sep), n->Name);
}

static code_string Amalgame_Compiler_Formatter_EmitIndex(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n) {
    (void)self;
    (void)n;
    return code_string_concat(code_string_concat(code_string_concat(Amalgame_Compiler_Formatter_EmitExpr(self, n->Left), "["), Amalgame_Compiler_Formatter_EmitExpr(self, n->Right)), "]");
}

static code_string Amalgame_Compiler_Formatter_EmitMatchExpr(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n) {
    (void)self;
    (void)n;
    code_string __attribute__((unused)) s = code_string_concat(code_string_concat("match ", Amalgame_Compiler_Formatter_EmitExpr(self, n->Left)), " { ");
    i64 __attribute__((unused)) arms = AmalgameList_count(n->Children);
    for (i64 i = 0; i < arms; i++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) arm = (Amalgame_Compiler_AstNode*)AmalgameList_get(n->Children, i);
        if (i > 0) {
            s = code_string_concat(s, ", ");
        }
        s = code_string_concat(s, Amalgame_Compiler_Formatter_EmitPattern(self, arm->Left));
        if (arm->Cond != NULL) {
            s = code_string_concat(code_string_concat(s, " if "), Amalgame_Compiler_Formatter_EmitExpr(self, arm->Cond));
        }
        s = code_string_concat(code_string_concat(s, " => "), Amalgame_Compiler_Formatter_EmitExpr(self, arm->Right));
    }
    return code_string_concat(s, " }");
}

static code_string Amalgame_Compiler_Formatter_EmitIfExpr(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n) {
    (void)self;
    (void)n;
    code_string __attribute__((unused)) cond = Amalgame_Compiler_Formatter_EmitExpr(self, n->Cond);
    code_string __attribute__((unused)) s = code_string_concat(code_string_concat(code_string_concat(code_string_concat("if (", cond), ") { "), Amalgame_Compiler_Formatter_EmitBlockAsExpr(self, n->Body)), " }");
    if (n->Else != NULL) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) elseN = n->Else;
        if (elseN->Kind == Amalgame_Compiler_NodeKind_IF_STMT && !code_string_equals(elseN->Name, "__match__")) {
            s = code_string_concat(code_string_concat(s, " else "), Amalgame_Compiler_Formatter_EmitIfExpr(self, elseN));
        } else {
            s = code_string_concat(code_string_concat(code_string_concat(s, " else { "), Amalgame_Compiler_Formatter_EmitBlockAsExpr(self, elseN)), " }");
        }
    }
    return s;
}

static code_string Amalgame_Compiler_Formatter_EmitLambda(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n) {
    (void)self;
    (void)n;
    i64 __attribute__((unused)) pn = AmalgameList_count(n->Params);
    code_string __attribute__((unused)) head = "";
    if (pn == 1) {
        head = Amalgame_Compiler_Formatter_LambdaParamName(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(n->Params, 0));
    } else {
        head = "(";
        for (i64 pi = 0; pi < pn; pi++) {
            if (pi > 0) {
                head = code_string_concat(head, ", ");
            }
            head = code_string_concat(head, Amalgame_Compiler_Formatter_LambdaParamName(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(n->Params, pi)));
        }
        head = code_string_concat(head, ")");
    }
    if (n->Body != NULL) {
        return code_string_concat(code_string_concat(code_string_concat(head, " => { "), Amalgame_Compiler_Formatter_EmitBlockAsExpr(self, n->Body)), " }");
    }
    if (n->Left != NULL) {
        return code_string_concat(code_string_concat(head, " => "), Amalgame_Compiler_Formatter_EmitExpr(self, n->Left));
    }
    return code_string_concat(head, " => 0");
}

static code_string Amalgame_Compiler_Formatter_LambdaParamName(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* p) {
    (void)self;
    (void)p;
    return p->Name;
}

static code_string Amalgame_Compiler_Formatter_EmitBlockAsExpr(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* body) {
    (void)self;
    (void)body;
    if (body->Kind != Amalgame_Compiler_NodeKind_BLOCK) {
        return Amalgame_Compiler_Formatter_EmitExpr(self, body);
    }
    i64 __attribute__((unused)) cnt = AmalgameList_count(body->Children);
    if (cnt == 0) {
        return "";
    }
    if (cnt == 1) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) s = (Amalgame_Compiler_AstNode*)AmalgameList_get(body->Children, 0);
        return Amalgame_Compiler_Formatter_EmitExpr(self, s);
    }
    code_string __attribute__((unused)) out = "";
    for (i64 i = 0; i < cnt; i++) {
        if (i > 0) {
            out = code_string_concat(out, "; ");
        }
        Amalgame_Compiler_AstNode* __attribute__((unused)) s = (Amalgame_Compiler_AstNode*)AmalgameList_get(body->Children, i);
        out = code_string_concat(out, Amalgame_Compiler_Formatter_EmitExpr(self, s));
    }
    return out;
}

static code_string Amalgame_Compiler_Formatter_EmitNew(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_AstNode* n) {
    (void)self;
    (void)n;
    code_string __attribute__((unused)) s = code_string_concat(code_string_concat("new ", n->Name), "(");
    i64 __attribute__((unused)) c = AmalgameList_count(n->Args);
    for (i64 i = 0; i < c; i++) {
        if (i > 0) {
            s = code_string_concat(s, ", ");
        }
        Amalgame_Compiler_AstNode* __attribute__((unused)) a = (Amalgame_Compiler_AstNode*)AmalgameList_get(n->Args, i);
        if (String_Length(a->Str2) > 0) {
            s = code_string_concat(code_string_concat(code_string_concat(s, a->Str2), ": "), Amalgame_Compiler_Formatter_EmitExpr(self, a));
        } else {
            s = code_string_concat(s, Amalgame_Compiler_Formatter_EmitExpr(self, a));
        }
    }
    return code_string_concat(s, ")");
}

static code_string Amalgame_Compiler_Formatter_QuoteString(Amalgame_Compiler_Formatter* self, code_string content) {
    (void)self;
    (void)content;
    i64 __attribute__((unused)) len = String_Length(content);
    code_string __attribute__((unused)) out = "\"";
    i64 __attribute__((unused)) i = 0;
    while (i < len) {
        code_string __attribute__((unused)) ch = String_Substring(content, i, 1);
        if (code_string_equals(ch, "\\")) {
            out = code_string_concat(out, "\\\\");
        } else if (code_string_equals(ch, "\"")) {
            out = code_string_concat(out, "\\\"");
        } else if (code_string_equals(ch, "\n")) {
            out = code_string_concat(out, "\\n");
        } else if (code_string_equals(ch, "\\r")) {
            out = code_string_concat(out, "\\r");
        } else if (code_string_equals(ch, "\t")) {
            out = code_string_concat(out, "\\t");
        } else {
            out = code_string_concat(out, ch);
        }
        i = i + 1;
    }
    return code_string_concat(out, "\"");
}

static code_string Amalgame_Compiler_Formatter_KindName(Amalgame_Compiler_Formatter* self, Amalgame_Compiler_NodeKind k) {
    (void)self;
    (void)k;
    if (k == Amalgame_Compiler_NodeKind_PROGRAM) {
        return "PROGRAM";
    }
    if (k == Amalgame_Compiler_NodeKind_CLASS_DECL) {
        return "CLASS_DECL";
    }
    if (k == Amalgame_Compiler_NodeKind_METHOD_DECL) {
        return "METHOD_DECL";
    }
    if (k == Amalgame_Compiler_NodeKind_FIELD_DECL) {
        return "FIELD_DECL";
    }
    if (k == Amalgame_Compiler_NodeKind_PARAM) {
        return "PARAM";
    }
    if (k == Amalgame_Compiler_NodeKind_ENUM_DECL) {
        return "ENUM_DECL";
    }
    if (k == Amalgame_Compiler_NodeKind_BLOCK) {
        return "BLOCK";
    }
    if (k == Amalgame_Compiler_NodeKind_VAR_DECL) {
        return "VAR_DECL";
    }
    if (k == Amalgame_Compiler_NodeKind_ASSIGN) {
        return "ASSIGN";
    }
    if (k == Amalgame_Compiler_NodeKind_RETURN_STMT) {
        return "RETURN_STMT";
    }
    if (k == Amalgame_Compiler_NodeKind_IF_STMT) {
        return "IF_STMT";
    }
    if (k == Amalgame_Compiler_NodeKind_WHILE_STMT) {
        return "WHILE_STMT";
    }
    if (k == Amalgame_Compiler_NodeKind_FOR_IN_STMT) {
        return "FOR_IN_STMT";
    }
    if (k == Amalgame_Compiler_NodeKind_BREAK_STMT) {
        return "BREAK_STMT";
    }
    if (k == Amalgame_Compiler_NodeKind_CONTINUE_STMT) {
        return "CONTINUE_STMT";
    }
    if (k == Amalgame_Compiler_NodeKind_TRY_STMT) {
        return "TRY_STMT";
    }
    if (k == Amalgame_Compiler_NodeKind_THROW_STMT) {
        return "THROW_STMT";
    }
    if (k == Amalgame_Compiler_NodeKind_BINARY) {
        return "BINARY";
    }
    if (k == Amalgame_Compiler_NodeKind_UNARY) {
        return "UNARY";
    }
    if (k == Amalgame_Compiler_NodeKind_CALL) {
        return "CALL";
    }
    if (k == Amalgame_Compiler_NodeKind_MEMBER) {
        return "MEMBER";
    }
    if (k == Amalgame_Compiler_NodeKind_IDENTIFIER) {
        return "IDENTIFIER";
    }
    if (k == Amalgame_Compiler_NodeKind_LITERAL_INT) {
        return "LITERAL_INT";
    }
    if (k == Amalgame_Compiler_NodeKind_LITERAL_FLOAT) {
        return "LITERAL_FLOAT";
    }
    if (k == Amalgame_Compiler_NodeKind_LITERAL_STRING) {
        return "LITERAL_STRING";
    }
    if (k == Amalgame_Compiler_NodeKind_LITERAL_BOOL) {
        return "LITERAL_BOOL";
    }
    if (k == Amalgame_Compiler_NodeKind_LITERAL_NULL) {
        return "LITERAL_NULL";
    }
    if (k == Amalgame_Compiler_NodeKind_NEW_EXPR) {
        return "NEW_EXPR";
    }
    if (k == Amalgame_Compiler_NodeKind_THIS_EXPR) {
        return "THIS_EXPR";
    }
    if (k == Amalgame_Compiler_NodeKind_INDEX_EXPR) {
        return "INDEX_EXPR";
    }
    if (k == Amalgame_Compiler_NodeKind_LIST_COMP) {
        return "LIST_COMP";
    }
    return "?";
}

struct _Amalgame_Compiler_Ansi {
};

code_string Amalgame_Compiler_Ansi_Reset();
code_string Amalgame_Compiler_Ansi_Bold();
code_string Amalgame_Compiler_Ansi_Dim();
code_string Amalgame_Compiler_Ansi_Red();
code_string Amalgame_Compiler_Ansi_Yellow();
code_string Amalgame_Compiler_Ansi_Cyan();
code_string Amalgame_Compiler_Ansi_Green();
code_string Amalgame_Compiler_Ansi_Blue();
code_string Amalgame_Compiler_Ansi_BoldRed();
code_string Amalgame_Compiler_Ansi_BoldYellow();
code_string Amalgame_Compiler_Ansi_BoldCyan();
code_string Amalgame_Compiler_Ansi_BoldGreen();
code_string Amalgame_Compiler_Ansi_BoldBlue();

Amalgame_Compiler_Ansi* Amalgame_Compiler_Ansi_new() {
    Amalgame_Compiler_Ansi* self = (Amalgame_Compiler_Ansi*) GC_MALLOC(sizeof(Amalgame_Compiler_Ansi));
    return self;
}

code_string Amalgame_Compiler_Ansi_Reset() {
    return "\x1b[0m";
}

code_string Amalgame_Compiler_Ansi_Bold() {
    return "\x1b[1m";
}

code_string Amalgame_Compiler_Ansi_Dim() {
    return "\x1b[2m";
}

code_string Amalgame_Compiler_Ansi_Red() {
    return "\x1b[31m";
}

code_string Amalgame_Compiler_Ansi_Yellow() {
    return "\x1b[33m";
}

code_string Amalgame_Compiler_Ansi_Cyan() {
    return "\x1b[36m";
}

code_string Amalgame_Compiler_Ansi_Green() {
    return "\x1b[32m";
}

code_string Amalgame_Compiler_Ansi_Blue() {
    return "\x1b[34m";
}

code_string Amalgame_Compiler_Ansi_BoldRed() {
    return "\x1b[1;31m";
}

code_string Amalgame_Compiler_Ansi_BoldYellow() {
    return "\x1b[1;33m";
}

code_string Amalgame_Compiler_Ansi_BoldCyan() {
    return "\x1b[1;36m";
}

code_string Amalgame_Compiler_Ansi_BoldGreen() {
    return "\x1b[1;32m";
}

code_string Amalgame_Compiler_Ansi_BoldBlue() {
    return "\x1b[1;34m";
}

struct _Amalgame_Compiler_SourceMap {
    AmalgameList* Paths;
    AmalgameList* Texts;
};

void Amalgame_Compiler_SourceMap_Add(Amalgame_Compiler_SourceMap* self, code_string path, code_string text);
code_string Amalgame_Compiler_SourceMap_GetLine(Amalgame_Compiler_SourceMap* self, code_string path, i64 line);
static code_string Amalgame_Compiler_SourceMap_NthLine(Amalgame_Compiler_SourceMap* self, code_string text, i64 line);

Amalgame_Compiler_SourceMap* Amalgame_Compiler_SourceMap_new() {
    Amalgame_Compiler_SourceMap* self = (Amalgame_Compiler_SourceMap*) GC_MALLOC(sizeof(Amalgame_Compiler_SourceMap));
    self->Paths = AmalgameList_new();
    self->Texts = AmalgameList_new();
    return self;
}

void Amalgame_Compiler_SourceMap_Add(Amalgame_Compiler_SourceMap* self, code_string path, code_string text) {
    (void)self;
    (void)path;
    (void)text;
    i64 __attribute__((unused)) count = AmalgameList_count(self->Paths);
    for (i64 i = 0; i < count; i++) {
        if (code_string_equals((code_string)AmalgameList_get(self->Paths, i), path)) {
            return;
        }
    }
    AmalgameList_add(self->Paths, (void*)(intptr_t)(path));
    AmalgameList_add(self->Texts, (void*)(intptr_t)(text));
}

code_string Amalgame_Compiler_SourceMap_GetLine(Amalgame_Compiler_SourceMap* self, code_string path, i64 line) {
    (void)self;
    (void)path;
    (void)line;
    i64 __attribute__((unused)) count = AmalgameList_count(self->Paths);
    for (i64 i = 0; i < count; i++) {
        if (code_string_equals((code_string)AmalgameList_get(self->Paths, i), path)) {
            return Amalgame_Compiler_SourceMap_NthLine(self, (code_string)AmalgameList_get(self->Texts, i), line);
        }
    }
    return "";
}

static code_string Amalgame_Compiler_SourceMap_NthLine(Amalgame_Compiler_SourceMap* self, code_string text, i64 line) {
    (void)self;
    (void)text;
    (void)line;
    i64 __attribute__((unused)) len = String_Length(text);
    i64 __attribute__((unused)) current = 1;
    i64 __attribute__((unused)) start = 0;
    i64 __attribute__((unused)) i = 0;
    while (i < len) {
        code_string __attribute__((unused)) ch = String_Substring(text, i, 1);
        if (code_string_equals(ch, "\n")) {
            if (current == line) {
                return String_Substring(text, start, i - start);
            }
            current = current + 1;
            start = i + 1;
        }
        i = i + 1;
    }
    if (current == line && start < len) {
        return String_Substring(text, start, len - start);
    }
    return "";
}

struct _Amalgame_Compiler_SourceSnippet {
};

code_string Amalgame_Compiler_SourceSnippet_Format(code_string lineText, i64 line, i64 col);
static code_string Amalgame_Compiler_SourceSnippet_Spaces(i64 n);

Amalgame_Compiler_SourceSnippet* Amalgame_Compiler_SourceSnippet_new() {
    Amalgame_Compiler_SourceSnippet* self = (Amalgame_Compiler_SourceSnippet*) GC_MALLOC(sizeof(Amalgame_Compiler_SourceSnippet));
    return self;
}

code_string Amalgame_Compiler_SourceSnippet_Format(code_string lineText, i64 line, i64 col) {
    (void)lineText;
    (void)line;
    (void)col;
    if (String_Length(lineText) == 0) {
        return "";
    }
    code_string __attribute__((unused)) lineNum = String_FromInt(line);
    code_string __attribute__((unused)) pad = Amalgame_Compiler_SourceSnippet_Spaces(String_Length(lineNum));
    code_string __attribute__((unused)) caret = "";
    i64 __attribute__((unused)) c = 0;
    while (c < col - 1) {
        caret = code_string_concat(caret, " ");
        c = c + 1;
    }
    return code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(pad, " |\n"), lineNum), " | "), lineText), "\n"), pad), " | "), caret), "^\n");
}

static code_string Amalgame_Compiler_SourceSnippet_Spaces(i64 n) {
    (void)n;
    code_string __attribute__((unused)) s = "";
    i64 __attribute__((unused)) i = 0;
    while (i < n) {
        s = code_string_concat(s, " ");
        i = i + 1;
    }
    return s;
}

enum _Amalgame_Compiler_DiagSeverity {
    Amalgame_Compiler_DiagSeverity_ERROR,
    Amalgame_Compiler_DiagSeverity_WARNING,
    Amalgame_Compiler_DiagSeverity_NOTE,
    Amalgame_Compiler_DiagSeverity_HINT
};

struct _Amalgame_Compiler_Diagnostic {
    Amalgame_Compiler_DiagSeverity Severity;
    code_string Kind;
    code_string Message;
    code_string Filename;
    i64 Line;
    i64 Column;
};


Amalgame_Compiler_Diagnostic* Amalgame_Compiler_Diagnostic_new(Amalgame_Compiler_DiagSeverity sev, code_string kind, code_string msg, code_string file, i64 line, i64 col) {
    Amalgame_Compiler_Diagnostic* self = (Amalgame_Compiler_Diagnostic*) GC_MALLOC(sizeof(Amalgame_Compiler_Diagnostic));
    self->Severity = sev;
    self->Kind = kind;
    self->Message = msg;
    self->Filename = file;
    self->Line = line;
    self->Column = col;
    return self;
}

struct _Amalgame_Compiler_DiagnosticFormatter {
    code_bool UseColor;
    code_bool Quiet;
    Amalgame_Compiler_SourceMap* Sources;
};

void Amalgame_Compiler_DiagnosticFormatter_EnableColor(Amalgame_Compiler_DiagnosticFormatter* self, code_bool v);
void Amalgame_Compiler_DiagnosticFormatter_SetQuiet(Amalgame_Compiler_DiagnosticFormatter* self, code_bool v);
void Amalgame_Compiler_DiagnosticFormatter_LoadSource(Amalgame_Compiler_DiagnosticFormatter* self, code_string path, code_string text);
static code_string Amalgame_Compiler_DiagnosticFormatter_SevLabel(Amalgame_Compiler_DiagnosticFormatter* self, Amalgame_Compiler_DiagSeverity sev);
code_string Amalgame_Compiler_DiagnosticFormatter_FormatDiag(Amalgame_Compiler_DiagnosticFormatter* self, Amalgame_Compiler_Diagnostic* d);
code_string Amalgame_Compiler_DiagnosticFormatter_FormatError(Amalgame_Compiler_DiagnosticFormatter* self, code_string kind, code_string message, code_string filename, i64 line, i64 col);
code_string Amalgame_Compiler_DiagnosticFormatter_FormatWarning(Amalgame_Compiler_DiagnosticFormatter* self, code_string kind, code_string message, code_string filename, i64 line, i64 col);
code_string Amalgame_Compiler_DiagnosticFormatter_FormatNote(Amalgame_Compiler_DiagnosticFormatter* self, code_string kind, code_string message, code_string filename, i64 line, i64 col);
void Amalgame_Compiler_DiagnosticFormatter_PrintPhaseOk(Amalgame_Compiler_DiagnosticFormatter* self, code_string phase);
void Amalgame_Compiler_DiagnosticFormatter_PrintPhaseError(Amalgame_Compiler_DiagnosticFormatter* self, code_string phase, code_string detail);
void Amalgame_Compiler_DiagnosticFormatter_PrintCompileOk(Amalgame_Compiler_DiagnosticFormatter* self, code_string phase);
void Amalgame_Compiler_DiagnosticFormatter_PrintCompileError(Amalgame_Compiler_DiagnosticFormatter* self, code_string phase, code_string detail);
code_string Amalgame_Compiler_DiagnosticFormatter_FormatSummary(Amalgame_Compiler_DiagnosticFormatter* self, i64 errorCount, i64 warningCount);
code_string Amalgame_Compiler_DiagnosticFormatter_FormatCompact(Amalgame_Compiler_DiagnosticFormatter* self, code_string severity, code_string kind, code_string message, code_string filename, i64 line, i64 col);

Amalgame_Compiler_DiagnosticFormatter* Amalgame_Compiler_DiagnosticFormatter_new() {
    Amalgame_Compiler_DiagnosticFormatter* self = (Amalgame_Compiler_DiagnosticFormatter*) GC_MALLOC(sizeof(Amalgame_Compiler_DiagnosticFormatter));
    self->UseColor = 0;
    self->Quiet = 0;
    self->Sources = Amalgame_Compiler_SourceMap_new();
    return self;
}

void Amalgame_Compiler_DiagnosticFormatter_EnableColor(Amalgame_Compiler_DiagnosticFormatter* self, code_bool v) {
    (void)self;
    (void)v;
    self->UseColor = v;
}

void Amalgame_Compiler_DiagnosticFormatter_SetQuiet(Amalgame_Compiler_DiagnosticFormatter* self, code_bool v) {
    (void)self;
    (void)v;
    self->Quiet = v;
}

void Amalgame_Compiler_DiagnosticFormatter_LoadSource(Amalgame_Compiler_DiagnosticFormatter* self, code_string path, code_string text) {
    (void)self;
    (void)path;
    (void)text;
    Amalgame_Compiler_SourceMap_Add(self->Sources, path, text);
}

static code_string Amalgame_Compiler_DiagnosticFormatter_SevLabel(Amalgame_Compiler_DiagnosticFormatter* self, Amalgame_Compiler_DiagSeverity sev) {
    (void)self;
    (void)sev;
    if (sev == Amalgame_Compiler_DiagSeverity_ERROR) {
        return "error";
    }
    if (sev == Amalgame_Compiler_DiagSeverity_WARNING) {
        return "warning";
    }
    if (sev == Amalgame_Compiler_DiagSeverity_NOTE) {
        return "note";
    }
    return "hint";
}

code_string Amalgame_Compiler_DiagnosticFormatter_FormatDiag(Amalgame_Compiler_DiagnosticFormatter* self, Amalgame_Compiler_Diagnostic* d) {
    (void)self;
    (void)d;
    code_string __attribute__((unused)) lineStr = String_FromInt(d->Line);
    code_string __attribute__((unused)) colStr = String_FromInt(d->Column);
    code_string __attribute__((unused)) label = Amalgame_Compiler_DiagnosticFormatter_SevLabel(self, d->Severity);
    code_string __attribute__((unused)) header = code_string_concat(code_string_concat(code_string_concat(code_string_concat(label, "["), d->Kind), "]: "), d->Message);
    code_string __attribute__((unused)) location = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("  --> ", d->Filename), ":"), lineStr), ":"), colStr);
    return code_string_concat(code_string_concat(code_string_concat(code_string_concat("\n", header), "\n"), location), "\n |\n");
}

code_string Amalgame_Compiler_DiagnosticFormatter_FormatError(Amalgame_Compiler_DiagnosticFormatter* self, code_string kind, code_string message, code_string filename, i64 line, i64 col) {
    (void)self;
    (void)kind;
    (void)message;
    (void)filename;
    (void)line;
    (void)col;
    Amalgame_Compiler_Diagnostic* __attribute__((unused)) d = Amalgame_Compiler_Diagnostic_new(Amalgame_Compiler_DiagSeverity_ERROR, kind, message, filename, line, col);
    return Amalgame_Compiler_DiagnosticFormatter_FormatDiag(self, d);
}

code_string Amalgame_Compiler_DiagnosticFormatter_FormatWarning(Amalgame_Compiler_DiagnosticFormatter* self, code_string kind, code_string message, code_string filename, i64 line, i64 col) {
    (void)self;
    (void)kind;
    (void)message;
    (void)filename;
    (void)line;
    (void)col;
    Amalgame_Compiler_Diagnostic* __attribute__((unused)) d = Amalgame_Compiler_Diagnostic_new(Amalgame_Compiler_DiagSeverity_WARNING, kind, message, filename, line, col);
    return Amalgame_Compiler_DiagnosticFormatter_FormatDiag(self, d);
}

code_string Amalgame_Compiler_DiagnosticFormatter_FormatNote(Amalgame_Compiler_DiagnosticFormatter* self, code_string kind, code_string message, code_string filename, i64 line, i64 col) {
    (void)self;
    (void)kind;
    (void)message;
    (void)filename;
    (void)line;
    (void)col;
    Amalgame_Compiler_Diagnostic* __attribute__((unused)) d = Amalgame_Compiler_Diagnostic_new(Amalgame_Compiler_DiagSeverity_NOTE, kind, message, filename, line, col);
    return Amalgame_Compiler_DiagnosticFormatter_FormatDiag(self, d);
}

void Amalgame_Compiler_DiagnosticFormatter_PrintPhaseOk(Amalgame_Compiler_DiagnosticFormatter* self, code_string phase) {
    (void)self;
    (void)phase;
    if (self->Quiet) {
        return;
    }
    Console_WriteLine(code_string_concat(phase, " OK"));
}

void Amalgame_Compiler_DiagnosticFormatter_PrintPhaseError(Amalgame_Compiler_DiagnosticFormatter* self, code_string phase, code_string detail) {
    (void)self;
    (void)phase;
    (void)detail;
    Console_WriteError(code_string_concat(phase, " ERROR"));
    if (String_Length(detail) > 0) {
        Console_WriteError(detail);
    }
}

void Amalgame_Compiler_DiagnosticFormatter_PrintCompileOk(Amalgame_Compiler_DiagnosticFormatter* self, code_string phase) {
    (void)self;
    (void)phase;
    Amalgame_Compiler_DiagnosticFormatter_PrintPhaseOk(self, phase);
}

void Amalgame_Compiler_DiagnosticFormatter_PrintCompileError(Amalgame_Compiler_DiagnosticFormatter* self, code_string phase, code_string detail) {
    (void)self;
    (void)phase;
    (void)detail;
    Amalgame_Compiler_DiagnosticFormatter_PrintPhaseError(self, phase, detail);
}

code_string Amalgame_Compiler_DiagnosticFormatter_FormatSummary(Amalgame_Compiler_DiagnosticFormatter* self, i64 errorCount, i64 warningCount) {
    (void)self;
    (void)errorCount;
    (void)warningCount;
    code_string __attribute__((unused)) result = " ";
    if (errorCount > 0) {
        code_string __attribute__((unused)) ec = String_FromInt(errorCount);
        code_string __attribute__((unused)) sfx = (errorCount > 1 ? "s" : " ");
        result = code_string_concat(code_string_concat(code_string_concat("aborting due to ", ec), " error"), sfx);
    }
    return result;
}

code_string Amalgame_Compiler_DiagnosticFormatter_FormatCompact(Amalgame_Compiler_DiagnosticFormatter* self, code_string severity, code_string kind, code_string message, code_string filename, i64 line, i64 col) {
    (void)self;
    (void)severity;
    (void)kind;
    (void)message;
    (void)filename;
    (void)line;
    (void)col;
    code_string __attribute__((unused)) loc = code_string_concat(code_string_concat(code_string_concat(code_string_concat(filename, ":"), String_FromInt(line)), ":"), String_FromInt(col));
    return code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(loc, ": "), severity), "["), kind), "]: "), message);
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
        code_string __attribute__((unused)) name = (code_string)AmalgameList_get(builtins, i);
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

struct _Amalgame_Compiler_ResolverError {
    code_string Message;
    code_string Filename;
    i64 Line;
    i64 Column;
};


Amalgame_Compiler_ResolverError* Amalgame_Compiler_ResolverError_new(code_string msg, code_string file, i64 line, i64 col) {
    Amalgame_Compiler_ResolverError* self = (Amalgame_Compiler_ResolverError*) GC_MALLOC(sizeof(Amalgame_Compiler_ResolverError));
    self->Message = msg;
    self->Filename = file;
    self->Line = line;
    self->Column = col;
    return self;
}

struct _Amalgame_Compiler_FullResolver {
    AmalgameList* GlobalNames;
    AmalgameList* GlobalTypes;
    AmalgameList* LocalNames;
    AmalgameList* LocalTypes;
    AmalgameList* LocalIsLets;
    AmalgameList* ScopeStarts;
    Amalgame_Compiler_MemberTable* Members;
    AmalgameList* Errors;
    AmalgameList* RawErrors;
    AmalgameList* Programs;
    code_string CurrentClass;
    code_string CurrentReturn;
    i64 LoopDepth;
    Amalgame_Compiler_AstNode* LambdaInProgress;
    i64 LambdaBoundary;
    code_string CurrentFile;
    Amalgame_Compiler_SourceMap* Sources;
};

static void Amalgame_Compiler_FullResolver_RegisterBuiltins(Amalgame_Compiler_FullResolver* self);
static void Amalgame_Compiler_FullResolver_PushScope(Amalgame_Compiler_FullResolver* self, code_string label);
static void Amalgame_Compiler_FullResolver_PopScope(Amalgame_Compiler_FullResolver* self);
static i64 Amalgame_Compiler_FullResolver_CurrentScopeStart(Amalgame_Compiler_FullResolver* self);
static void Amalgame_Compiler_FullResolver_DeclareGlobal(Amalgame_Compiler_FullResolver* self, code_string name, code_string typeName, code_bool isLet);
static void Amalgame_Compiler_FullResolver_DeclareLambdaParam(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* p);
static code_bool Amalgame_Compiler_FullResolver_DeclareCurrent(Amalgame_Compiler_FullResolver* self, code_string name, code_string typeName, code_bool isLet);
static code_bool Amalgame_Compiler_FullResolver_LookupInScopes(Amalgame_Compiler_FullResolver* self, code_string name);
static code_string Amalgame_Compiler_FullResolver_LookupType(Amalgame_Compiler_FullResolver* self, code_string name);
static code_bool Amalgame_Compiler_FullResolver_LookupIsLet(Amalgame_Compiler_FullResolver* self, code_string name);
static i64 Amalgame_Compiler_FullResolver_IndexOfLocal(Amalgame_Compiler_FullResolver* self, code_string name);
static void Amalgame_Compiler_FullResolver_AddCapture(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* lam, code_string name, code_string typeName, i64 line, i64 col);
static void Amalgame_Compiler_FullResolver_Error(Amalgame_Compiler_FullResolver* self, code_string msg, Amalgame_Compiler_AstNode* node);
static void Amalgame_Compiler_FullResolver_ErrorMsg(Amalgame_Compiler_FullResolver* self, code_string msg);
static void Amalgame_Compiler_FullResolver_EmitError(Amalgame_Compiler_FullResolver* self, code_string msg, i64 line, i64 col);
code_bool Amalgame_Compiler_FullResolver_HasErrors(Amalgame_Compiler_FullResolver* self);
code_string Amalgame_Compiler_FullResolver_GetErrors(Amalgame_Compiler_FullResolver* self);
i64 Amalgame_Compiler_FullResolver_ProgramCount(Amalgame_Compiler_FullResolver* self);
Amalgame_Compiler_AstNode* Amalgame_Compiler_FullResolver_ProgramAt(Amalgame_Compiler_FullResolver* self, i64 i);
i64 Amalgame_Compiler_FullResolver_GlobalCount(Amalgame_Compiler_FullResolver* self);
code_string Amalgame_Compiler_FullResolver_GlobalNameAt(Amalgame_Compiler_FullResolver* self, i64 i);
code_string Amalgame_Compiler_FullResolver_GlobalTypeAt(Amalgame_Compiler_FullResolver* self, i64 i);
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
static void Amalgame_Compiler_FullResolver_PatchLambdaParamTypes(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* call);
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
    self->GlobalNames = AmalgameList_new();
    self->GlobalTypes = AmalgameList_new();
    self->LocalNames = AmalgameList_new();
    self->LocalTypes = AmalgameList_new();
    self->LocalIsLets = AmalgameList_new();
    self->ScopeStarts = AmalgameList_new();
    self->Members = Amalgame_Compiler_MemberTable_new();
    self->Errors = AmalgameList_new();
    self->RawErrors = AmalgameList_new();
    self->Programs = AmalgameList_new();
    self->CurrentClass = "";
    self->CurrentReturn = "void";
    self->LoopDepth = 0;
    self->LambdaInProgress = NULL;
    self->LambdaBoundary = 0;
    self->CurrentFile = "";
    self->Sources = Amalgame_Compiler_SourceMap_new();
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
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "String", "type", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "List", "type", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Map", "type", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Set", "type", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Path", "type", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Env", "type", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Process", "type", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Process_Run", "int", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Process_RunCapture", "AmalgameProcessResult", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Console", "type", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Console_WriteLine", "void", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Console_WriteError", "void", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Console_Clear", "void", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Console_Write", "void", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Console_ReadLine", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Console_ReadBytes", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Console_Flush", "void", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "File", "type", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Path", "type", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "String_CharAt1", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "File_ReadAll", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "File_WriteAll", "void", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "File_AppendAll", "void", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "File_WriteLines", "void", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "File_OpenWrite", "void", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "File_StreamLine", "void", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "File_CloseWrite", "void", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "File_Exists", "bool", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "File_Delete", "bool", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "File_Size", "int", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Path_Combine", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Path_GetExtension", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Path_GetFilename", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Path_GetDirectory", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Env_Get", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Env_Has", "bool", 0);
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
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "String_LastIndexOf", "int", 0);
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
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "String_FromByte", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "String_FromCodepoint", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Args_Count", "int", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Args_Get", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Exit_Set", "void", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Exit_Get", "int", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "String_FromFloat", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "String_From", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "String_CharAt", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Http", "type", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Http_Get", "?", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Http_Post", "?", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Http_GetWithHeaders", "?", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Http_GetTimeout", "?", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Http_PostJson", "?", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Http_PostWithHeaders", "?", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Http_Put", "?", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Http_Delete", "?", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "Http_Patch", "?", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "TcpServer_Listen", "?", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "TcpServer_Accept", "?", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "TcpServer_Close", "void", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "TcpServer_IsListening", "bool", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "TcpClient_Connect", "?", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "TcpClient_Send", "void", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "TcpClient_Receive", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "TcpClient_Close", "void", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "TcpClient_IsConnected", "bool", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "TcpConn_Send", "void", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "TcpConn_Receive", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "TcpConn_Close", "void", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "TcpConn_IsConnected", "bool", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "UdpSocket_New", "?", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "UdpSocket_Bind", "bool", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "UdpSocket_Send", "bool", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "UdpSocket_Receive", "string", 0);
    Amalgame_Compiler_FullResolver_DeclareGlobal(self, "UdpSocket_Close", "void", 0);
}

static void Amalgame_Compiler_FullResolver_PushScope(Amalgame_Compiler_FullResolver* self, code_string label) {
    (void)self;
    (void)label;
    AmalgameList_add(self->ScopeStarts, (void*)(intptr_t)(AmalgameList_count(self->LocalNames)));
}

static void Amalgame_Compiler_FullResolver_PopScope(Amalgame_Compiler_FullResolver* self) {
    (void)self;
    i64 __attribute__((unused)) depth = AmalgameList_count(self->ScopeStarts);
    if (depth == 0) {
        return;
    }
    i64 __attribute__((unused)) mark = (i64)AmalgameList_get(self->ScopeStarts, depth - 1);
    AmalgameList_removeAt(self->ScopeStarts, depth - 1);
    while (AmalgameList_count(self->LocalNames) > mark) {
        i64 __attribute__((unused)) last = AmalgameList_count(self->LocalNames) - 1;
        AmalgameList_removeAt(self->LocalNames, last);
        AmalgameList_removeAt(self->LocalTypes, last);
        AmalgameList_removeAt(self->LocalIsLets, last);
    }
}

static i64 Amalgame_Compiler_FullResolver_CurrentScopeStart(Amalgame_Compiler_FullResolver* self) {
    (void)self;
    i64 __attribute__((unused)) depth = AmalgameList_count(self->ScopeStarts);
    if (depth == 0) {
        return 0;
    }
    return (i64)AmalgameList_get(self->ScopeStarts, depth - 1);
}

static void Amalgame_Compiler_FullResolver_DeclareGlobal(Amalgame_Compiler_FullResolver* self, code_string name, code_string typeName, code_bool isLet) {
    (void)self;
    (void)name;
    (void)typeName;
    (void)isLet;
    i64 __attribute__((unused)) count = AmalgameList_count(self->GlobalNames);
    for (i64 i = 0; i < count; i++) {
        if (code_string_equals((code_string)AmalgameList_get(self->GlobalNames, i), name)) {
            return;
        }
    }
    AmalgameList_add(self->GlobalNames, (void*)(intptr_t)(name));
    AmalgameList_add(self->GlobalTypes, (void*)(intptr_t)(typeName));
}

static void Amalgame_Compiler_FullResolver_DeclareLambdaParam(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* p) {
    (void)self;
    (void)p;
    code_string __attribute__((unused)) t = p->Str;
    if (String_Length(t) == 0) {
        t = "?";
    }
    Amalgame_Compiler_FullResolver_DeclareCurrent(self, p->Name, t, 1);
}

static code_bool Amalgame_Compiler_FullResolver_DeclareCurrent(Amalgame_Compiler_FullResolver* self, code_string name, code_string typeName, code_bool isLet) {
    (void)self;
    (void)name;
    (void)typeName;
    (void)isLet;
    i64 __attribute__((unused)) start = Amalgame_Compiler_FullResolver_CurrentScopeStart(self);
    i64 __attribute__((unused)) count = AmalgameList_count(self->LocalNames);
    for (i64 i = start; i < count; i++) {
        if (code_string_equals((code_string)AmalgameList_get(self->LocalNames, i), name)) {
            return 0;
        }
    }
    AmalgameList_add(self->LocalNames, (void*)(intptr_t)(name));
    AmalgameList_add(self->LocalTypes, (void*)(intptr_t)(typeName));
    AmalgameList_add(self->LocalIsLets, (void*)(intptr_t)(isLet));
    return 1;
}

static code_bool Amalgame_Compiler_FullResolver_LookupInScopes(Amalgame_Compiler_FullResolver* self, code_string name) {
    (void)self;
    (void)name;
    i64 __attribute__((unused)) i = AmalgameList_count(self->LocalNames) - 1;
    while (i >= 0) {
        if (code_string_equals((code_string)AmalgameList_get(self->LocalNames, i), name)) {
            return 1;
        }
        i = i - 1;
    }
    i64 __attribute__((unused)) gc = AmalgameList_count(self->GlobalNames);
    for (i64 j = 0; j < gc; j++) {
        if (code_string_equals((code_string)AmalgameList_get(self->GlobalNames, j), name)) {
            return 1;
        }
    }
    return 0;
}

static code_string Amalgame_Compiler_FullResolver_LookupType(Amalgame_Compiler_FullResolver* self, code_string name) {
    (void)self;
    (void)name;
    i64 __attribute__((unused)) i = AmalgameList_count(self->LocalNames) - 1;
    while (i >= 0) {
        if (code_string_equals((code_string)AmalgameList_get(self->LocalNames, i), name)) {
            return (code_string)AmalgameList_get(self->LocalTypes, i);
        }
        i = i - 1;
    }
    i64 __attribute__((unused)) gc = AmalgameList_count(self->GlobalNames);
    for (i64 j = 0; j < gc; j++) {
        if (code_string_equals((code_string)AmalgameList_get(self->GlobalNames, j), name)) {
            return (code_string)AmalgameList_get(self->GlobalTypes, j);
        }
    }
    return "?";
}

static code_bool Amalgame_Compiler_FullResolver_LookupIsLet(Amalgame_Compiler_FullResolver* self, code_string name) {
    (void)self;
    (void)name;
    i64 __attribute__((unused)) i = AmalgameList_count(self->LocalNames) - 1;
    while (i >= 0) {
        if (code_string_equals((code_string)AmalgameList_get(self->LocalNames, i), name)) {
            return (code_bool)AmalgameList_get(self->LocalIsLets, i);
        }
        i = i - 1;
    }
    return 0;
}

static i64 Amalgame_Compiler_FullResolver_IndexOfLocal(Amalgame_Compiler_FullResolver* self, code_string name) {
    (void)self;
    (void)name;
    i64 __attribute__((unused)) i = AmalgameList_count(self->LocalNames) - 1;
    while (i >= 0) {
        if (code_string_equals((code_string)AmalgameList_get(self->LocalNames, i), name)) {
            return i;
        }
        i = i - 1;
    }
    return -1;
}

static void Amalgame_Compiler_FullResolver_AddCapture(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* lam, code_string name, code_string typeName, i64 line, i64 col) {
    (void)self;
    (void)lam;
    (void)name;
    (void)typeName;
    (void)line;
    (void)col;
    i64 __attribute__((unused)) n = AmalgameList_count(lam->Args);
    for (i64 i = 0; i < n; i++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) arg = (Amalgame_Compiler_AstNode*)AmalgameList_get(lam->Args, i);
        if (code_string_equals(arg->Name, name)) {
            return;
        }
    }
    Amalgame_Compiler_AstNode* __attribute__((unused)) id = Amalgame_Compiler_AstNode_new(Amalgame_Compiler_NodeKind_IDENTIFIER, line, col);
    id->Name = name;
    id->Str = typeName;
    AmalgameList_add(lam->Args, (void*)(intptr_t)(id));
}

static void Amalgame_Compiler_FullResolver_Error(Amalgame_Compiler_FullResolver* self, code_string msg, Amalgame_Compiler_AstNode* node) {
    (void)self;
    (void)msg;
    (void)node;
    Amalgame_Compiler_FullResolver_EmitError(self, msg, node->Line, node->Column);
}

static void Amalgame_Compiler_FullResolver_ErrorMsg(Amalgame_Compiler_FullResolver* self, code_string msg) {
    (void)self;
    (void)msg;
    Amalgame_Compiler_FullResolver_EmitError(self, msg, 0, 0);
}

static void Amalgame_Compiler_FullResolver_EmitError(Amalgame_Compiler_FullResolver* self, code_string msg, i64 line, i64 col) {
    (void)self;
    (void)msg;
    (void)line;
    (void)col;
    code_string __attribute__((unused)) file = self->CurrentFile;
    code_string __attribute__((unused)) ln = String_FromInt(line);
    code_string __attribute__((unused)) cl = String_FromInt(col);
    code_string __attribute__((unused)) head = code_string_concat(code_string_concat("\nerror[resolver]: ", msg), "\n");
    if (String_Length(file) > 0 && line > 0) {
        head = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(head, "  --> "), file), ":"), ln), ":"), cl), "\n");
        void* __attribute__((unused)) snip = Amalgame_Compiler_SourceMap_GetLine(self->Sources, file, line);
        head = code_string_concat(head, Amalgame_Compiler_SourceSnippet_Format(snip, line, col));
    }
    AmalgameList_add(self->Errors, (void*)(intptr_t)(head));
    Amalgame_Compiler_ResolverError* __attribute__((unused)) raw = Amalgame_Compiler_ResolverError_new(msg, file, line, col);
    AmalgameList_add(self->RawErrors, (void*)(intptr_t)(raw));
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

i64 Amalgame_Compiler_FullResolver_ProgramCount(Amalgame_Compiler_FullResolver* self) {
    (void)self;
    return AmalgameList_count(self->Programs);
}

Amalgame_Compiler_AstNode* Amalgame_Compiler_FullResolver_ProgramAt(Amalgame_Compiler_FullResolver* self, i64 i) {
    (void)self;
    (void)i;
    return (Amalgame_Compiler_AstNode*)AmalgameList_get(self->Programs, i);
}

i64 Amalgame_Compiler_FullResolver_GlobalCount(Amalgame_Compiler_FullResolver* self) {
    (void)self;
    return AmalgameList_count(self->GlobalNames);
}

code_string Amalgame_Compiler_FullResolver_GlobalNameAt(Amalgame_Compiler_FullResolver* self, i64 i) {
    (void)self;
    (void)i;
    return (code_string)AmalgameList_get(self->GlobalNames, i);
}

code_string Amalgame_Compiler_FullResolver_GlobalTypeAt(Amalgame_Compiler_FullResolver* self, i64 i) {
    (void)self;
    (void)i;
    return (code_string)AmalgameList_get(self->GlobalTypes, i);
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
    self->CurrentFile = prog->Str2;
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
        code_string __attribute__((unused)) declType = stmt->Str;
        if (String_Length(declType) == 0 && stmt->Left != NULL) {
            declType = Amalgame_Compiler_FullResolver_InferExprType(self, stmt->Left);
        }
        Amalgame_Compiler_FullResolver_DeclareCurrent(self, stmt->Name, declType, isLet);
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
            Amalgame_Compiler_FullResolver_Error(self, "'break' outside loop", stmt);
        }
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_CONTINUE_STMT) {
        if (self->LoopDepth == 0) {
            Amalgame_Compiler_FullResolver_Error(self, "'continue' outside loop", stmt);
        }
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_TRY_STMT) {
        if (stmt->Body != NULL) {
            Amalgame_Compiler_FullResolver_ResolveBlock(self, stmt->Body);
        }
        if (stmt->Else != NULL) {
            Amalgame_Compiler_FullResolver_PushScope(self, "catch");
            if (String_Length(stmt->Name) > 0) {
                Amalgame_Compiler_FullResolver_DeclareCurrent(self, stmt->Name, "void*", 1);
            }
            Amalgame_Compiler_FullResolver_ResolveBlock(self, stmt->Else);
            Amalgame_Compiler_FullResolver_PopScope(self);
        }
        if (stmt->Cond != NULL) {
            Amalgame_Compiler_FullResolver_ResolveBlock(self, stmt->Cond);
        }
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_THROW_STMT) {
        if (stmt->Left != NULL) {
            Amalgame_Compiler_FullResolver_ResolveExpr(self, stmt->Left);
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
            Amalgame_Compiler_FullResolver_Error(self, code_string_concat(code_string_concat("Cannot assign to immutable binding '", varName), "'"), stmt->Left);
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

static void Amalgame_Compiler_FullResolver_PatchLambdaParamTypes(Amalgame_Compiler_FullResolver* self, Amalgame_Compiler_AstNode* call) {
    (void)self;
    (void)call;
    if (call->Left == NULL) {
        return;
    }
    if (call->Left->Kind != Amalgame_Compiler_NodeKind_MEMBER) {
        return;
    }
    code_string __attribute__((unused)) methodName = call->Left->Name;
    code_bool __attribute__((unused)) isSingleT = 0;
    if (code_string_equals(methodName, "Map")) {
        isSingleT = 1;
    } else if (code_string_equals(methodName, "Filter")) {
        isSingleT = 1;
    } else if (code_string_equals(methodName, "ForEach")) {
        isSingleT = 1;
    } else if (code_string_equals(methodName, "Any")) {
        isSingleT = 1;
    } else if (code_string_equals(methodName, "All")) {
        isSingleT = 1;
    } else if (code_string_equals(methodName, "CountIf")) {
        isSingleT = 1;
    }
    if (!isSingleT) {
        return;
    }
    if (call->Left->Left == NULL) {
        return;
    }
    code_string __attribute__((unused)) recvType = Amalgame_Compiler_FullResolver_InferExprType(self, call->Left->Left);
    code_string __attribute__((unused)) elem = Amalgame_Compiler_FullResolver_CollectionElemType(self, recvType);
    if (String_Length(elem) == 0) {
        return;
    }
    if (code_string_equals(elem, "?")) {
        return;
    }
    i64 __attribute__((unused)) argc = AmalgameList_count(call->Args);
    for (i64 i = 0; i < argc; i++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) arg = (Amalgame_Compiler_AstNode*)AmalgameList_get(call->Args, i);
        if (arg->Kind == Amalgame_Compiler_NodeKind_METHOD_DECL && code_string_equals(arg->Name, "__lambda__")) {
            i64 __attribute__((unused)) pn = AmalgameList_count(arg->Params);
            if (pn >= 1) {
                Amalgame_Compiler_AstNode* __attribute__((unused)) p = (Amalgame_Compiler_AstNode*)AmalgameList_get(arg->Params, 0);
                if (String_Length(p->Str) == 0 || code_string_equals(p->Str, "?")) {
                    p->Str = elem;
                }
            }
        }
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
        if (code_string_equals(expr->Name, "_unknown_")) {
            return;
        }
        if (code_string_equals(expr->Name, "_")) {
            return;
        }
        if (!Amalgame_Compiler_FullResolver_LookupInScopes(self, expr->Name)) {
            Amalgame_Compiler_FullResolver_Error(self, code_string_concat(code_string_concat("Unknown symbol '", expr->Name), "'"), expr);
            return;
        }
        if (self->LambdaInProgress != NULL) {
            i64 __attribute__((unused)) idx = Amalgame_Compiler_FullResolver_IndexOfLocal(self, expr->Name);
            if (idx != -1 && idx < self->LambdaBoundary) {
                code_string __attribute__((unused)) typeName = (code_string)AmalgameList_get(self->LocalTypes, idx);
                if (String_Length(typeName) == 0 || code_string_equals(typeName, "?")) {
                    typeName = "int";
                }
                Amalgame_Compiler_FullResolver_AddCapture(self, self->LambdaInProgress, expr->Name, typeName, expr->Line, expr->Column);
            }
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
        Amalgame_Compiler_FullResolver_PatchLambdaParamTypes(self, expr);
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
    if (k == Amalgame_Compiler_NodeKind_LIST_COMP) {
        if (expr->Right != NULL) {
            Amalgame_Compiler_FullResolver_ResolveExpr(self, expr->Right);
        }
        Amalgame_Compiler_FullResolver_PushScope(self, "list-comp");
        code_string __attribute__((unused)) elemType = "?";
        if (expr->Right != NULL) {
            code_string __attribute__((unused)) colType = Amalgame_Compiler_FullResolver_InferExprType(self, expr->Right);
            elemType = Amalgame_Compiler_FullResolver_CollectionElemType(self, colType);
        }
        Amalgame_Compiler_FullResolver_DeclareCurrent(self, expr->Str, elemType, 1);
        if (expr->Left != NULL) {
            Amalgame_Compiler_FullResolver_ResolveExpr(self, expr->Left);
        }
        if (expr->Cond != NULL) {
            Amalgame_Compiler_FullResolver_ResolveExpr(self, expr->Cond);
        }
        Amalgame_Compiler_FullResolver_PopScope(self);
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_ASSIGN) {
        Amalgame_Compiler_FullResolver_ResolveAssign(self, expr);
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_IF_STMT) {
        if (code_string_equals(expr->Name, "__match__")) {
            Amalgame_Compiler_FullResolver_ResolveMatch(self, expr);
            return;
        }
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
    if (k == Amalgame_Compiler_NodeKind_METHOD_DECL && code_string_equals(expr->Name, "__lambda__")) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) prevLam = self->LambdaInProgress;
        i64 __attribute__((unused)) prevBoundary = self->LambdaBoundary;
        Amalgame_Compiler_FullResolver_PushScope(self, "lambda");
        self->LambdaInProgress = expr;
        self->LambdaBoundary = AmalgameList_count(self->LocalNames);
        i64 __attribute__((unused)) pn = AmalgameList_count(expr->Params);
        for (i64 pi = 0; pi < pn; pi++) {
            Amalgame_Compiler_FullResolver_DeclareLambdaParam(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(expr->Params, pi));
        }
        if (expr->Left != NULL) {
            Amalgame_Compiler_FullResolver_ResolveExpr(self, expr->Left);
        }
        if (expr->Body != NULL) {
            Amalgame_Compiler_FullResolver_ResolveBlock(self, expr->Body);
        }
        Amalgame_Compiler_FullResolver_PopScope(self);
        self->LambdaInProgress = prevLam;
        self->LambdaBoundary = prevBoundary;
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
        if (String_Length(expr->Str2) > 0) {
            return code_string_concat(code_string_concat(code_string_concat(expr->Name, "<"), expr->Str2), ">");
        }
        return expr->Name;
    }
    if (k == Amalgame_Compiler_NodeKind_MEMBER) {
        if (expr->Left != NULL) {
            code_string __attribute__((unused)) targetType = Amalgame_Compiler_FullResolver_InferExprType(self, expr->Left);
            return Amalgame_Compiler_MemberTable_Get(self->Members, targetType, expr->Name);
        }
    }
    if (k == Amalgame_Compiler_NodeKind_CALL) {
        if (expr->Left != NULL && expr->Left->Kind == Amalgame_Compiler_NodeKind_MEMBER) {
            code_string __attribute__((unused)) mname = expr->Left->Name;
            if (code_string_equals(mname, "Filter") || code_string_equals(mname, "ForEach")) {
                if (expr->Left->Left != NULL) {
                    return Amalgame_Compiler_FullResolver_InferExprType(self, expr->Left->Left);
                }
            }
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
    i64 __attribute__((unused)) lc = AmalgameList_count(self->LocalNames);
    for (i64 i = 0; i < lc; i++) {
        if (code_string_equals((code_string)AmalgameList_get(self->LocalNames, i), name)) {
            AmalgameList_add(self->LocalTypes, (void*)(intptr_t)(typeName));
            return;
        }
    }
    i64 __attribute__((unused)) gc = AmalgameList_count(self->GlobalNames);
    for (i64 i = 0; i < gc; i++) {
        if (code_string_equals((code_string)AmalgameList_get(self->GlobalNames, i), name)) {
            AmalgameList_add(self->GlobalTypes, (void*)(intptr_t)(typeName));
            return;
        }
    }
}

struct _Amalgame_Compiler_TypeError {
    code_string Message;
    code_string Filename;
    i64 Line;
    i64 Column;
    code_string Snippet;
};

code_string Amalgame_Compiler_TypeError_ToString(Amalgame_Compiler_TypeError* self);

Amalgame_Compiler_TypeError* Amalgame_Compiler_TypeError_new(code_string msg, code_string file, i64 line, i64 col) {
    Amalgame_Compiler_TypeError* self = (Amalgame_Compiler_TypeError*) GC_MALLOC(sizeof(Amalgame_Compiler_TypeError));
    self->Message = msg;
    self->Filename = file;
    self->Line = line;
    self->Column = col;
    self->Snippet = "";
    return self;
}

code_string Amalgame_Compiler_TypeError_ToString(Amalgame_Compiler_TypeError* self) {
    (void)self;
    code_string __attribute__((unused)) ln = String_FromInt(self->Line);
    code_string __attribute__((unused)) col = String_FromInt(self->Column);
    code_string __attribute__((unused)) head = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("\nerror[typechecker]: ", self->Message), "\n  --> "), self->Filename), ":"), ln), ":"), col), "\n");
    code_string __attribute__((unused)) snip = Amalgame_Compiler_SourceSnippet_Format(self->Snippet, self->Line, self->Column);
    return code_string_concat(head, snip);
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
    AmalgameList* LocalNames;
    AmalgameList* LocalTypes;
    AmalgameList* ScopeStarts;
    AmalgameList* SubstParams;
    AmalgameList* SubstArgs;
    Amalgame_Compiler_SourceMap* Sources;
};

static void Amalgame_Compiler_TypeChecker_PushScope(Amalgame_Compiler_TypeChecker* self);
static void Amalgame_Compiler_TypeChecker_PopScope(Amalgame_Compiler_TypeChecker* self);
static void Amalgame_Compiler_TypeChecker_DeclareLocal(Amalgame_Compiler_TypeChecker* self, code_string name, code_string typeName);
static code_string Amalgame_Compiler_TypeChecker_LookupLocal(Amalgame_Compiler_TypeChecker* self, code_string name);
Amalgame_Compiler_TypeCheckResult* Amalgame_Compiler_TypeChecker_Check(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* program);
static code_string Amalgame_Compiler_TypeChecker_NodeKey(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* node);
static void Amalgame_Compiler_TypeChecker_SetType(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* node, code_string typeKey);
static code_string Amalgame_Compiler_TypeChecker_GetType(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* node);
code_string Amalgame_Compiler_TypeChecker_LookupNodeType(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* node);
static code_bool Amalgame_Compiler_TypeChecker_IsAssignable(Amalgame_Compiler_TypeChecker* self, code_string expected, code_string actual);
static code_string Amalgame_Compiler_TypeChecker_GenericBase(Amalgame_Compiler_TypeChecker* self, code_string t);
static code_bool Amalgame_Compiler_TypeChecker_IsNumericWiden(Amalgame_Compiler_TypeChecker* self, code_string to, code_string from);
static code_bool Amalgame_Compiler_TypeChecker_IsBool(Amalgame_Compiler_TypeChecker* self, code_string t);
static code_bool Amalgame_Compiler_TypeChecker_IsNumeric(Amalgame_Compiler_TypeChecker* self, code_string t);
static code_bool Amalgame_Compiler_TypeChecker_IsNullable(Amalgame_Compiler_TypeChecker* self, code_string t);
static code_string Amalgame_Compiler_TypeChecker_BinaryResultType(Amalgame_Compiler_TypeChecker* self, code_string op, code_string left, code_string right);
static code_string Amalgame_Compiler_TypeChecker_CollectionElementType(Amalgame_Compiler_TypeChecker* self, code_string typeKey);
static code_bool Amalgame_Compiler_TypeChecker_SymbolFound(Amalgame_Compiler_TypeChecker* self, code_string name);
static code_string Amalgame_Compiler_TypeChecker_SymbolTypeName(Amalgame_Compiler_TypeChecker* self, code_string name);
static void Amalgame_Compiler_TypeChecker_Error(Amalgame_Compiler_TypeChecker* self, code_string msg, Amalgame_Compiler_AstNode* node);
static void Amalgame_Compiler_TypeChecker_CheckBool(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* node, code_string context);
static code_string Amalgame_Compiler_TypeChecker_SymbolType(Amalgame_Compiler_TypeChecker* self, code_string name);
static code_string Amalgame_Compiler_TypeChecker_MemberTypeOf(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* classDecl, code_string memberName);
static void Amalgame_Compiler_TypeChecker_CheckProgram(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* prog);
static void Amalgame_Compiler_TypeChecker_CheckDecl(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* decl);
static void Amalgame_Compiler_TypeChecker_CheckClass(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* cls);
static void Amalgame_Compiler_TypeChecker_CheckImplementsContract(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* cls);
static void Amalgame_Compiler_TypeChecker_CheckOneInterface(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* cls, code_string iref);
static void Amalgame_Compiler_TypeChecker_CheckOneInterfaceMethod(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* cls, code_string ifaceLabel, Amalgame_Compiler_AstNode* imethod);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_TypeChecker_FindInterface(Amalgame_Compiler_TypeChecker* self, code_string name);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_TypeChecker_FindInterfaceInProgram(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* prog, code_string name);
static AmalgameList* Amalgame_Compiler_TypeChecker_SplitTopLevelCommas(Amalgame_Compiler_TypeChecker* self, code_string s);
static code_string Amalgame_Compiler_TypeChecker_SubstType(Amalgame_Compiler_TypeChecker* self, code_string t);
static code_string Amalgame_Compiler_TypeChecker_LookupParam(Amalgame_Compiler_TypeChecker* self, code_string t);
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
    self->LocalNames = AmalgameList_new();
    self->LocalTypes = AmalgameList_new();
    self->ScopeStarts = AmalgameList_new();
    self->SubstParams = AmalgameList_new();
    self->SubstArgs = AmalgameList_new();
    self->Sources = Amalgame_Compiler_SourceMap_new();
    return self;
}

static void Amalgame_Compiler_TypeChecker_PushScope(Amalgame_Compiler_TypeChecker* self) {
    (void)self;
    AmalgameList_add(self->ScopeStarts, (void*)(intptr_t)(AmalgameList_count(self->LocalNames)));
}

static void Amalgame_Compiler_TypeChecker_PopScope(Amalgame_Compiler_TypeChecker* self) {
    (void)self;
    i64 __attribute__((unused)) depth = AmalgameList_count(self->ScopeStarts);
    if (depth == 0) {
        return;
    }
    i64 __attribute__((unused)) mark = (i64)AmalgameList_get(self->ScopeStarts, depth - 1);
    AmalgameList_removeAt(self->ScopeStarts, depth - 1);
    while (AmalgameList_count(self->LocalNames) > mark) {
        i64 __attribute__((unused)) last = AmalgameList_count(self->LocalNames) - 1;
        AmalgameList_removeAt(self->LocalNames, last);
        AmalgameList_removeAt(self->LocalTypes, last);
    }
}

static void Amalgame_Compiler_TypeChecker_DeclareLocal(Amalgame_Compiler_TypeChecker* self, code_string name, code_string typeName) {
    (void)self;
    (void)name;
    (void)typeName;
    AmalgameList_add(self->LocalNames, (void*)(intptr_t)(name));
    AmalgameList_add(self->LocalTypes, (void*)(intptr_t)(typeName));
}

static code_string Amalgame_Compiler_TypeChecker_LookupLocal(Amalgame_Compiler_TypeChecker* self, code_string name) {
    (void)self;
    (void)name;
    i64 __attribute__((unused)) i = AmalgameList_count(self->LocalNames) - 1;
    while (i >= 0) {
        if (code_string_equals((code_string)AmalgameList_get(self->LocalNames, i), name)) {
            return (code_string)AmalgameList_get(self->LocalTypes, i);
        }
        i = i - 1;
    }
    return "";
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
    code_string __attribute__((unused)) knd = String_FromInt(node->Kind);
    return code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(knd, ":"), ln), ":"), col), ":"), node->Name), ":"), node->Str);
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

code_string Amalgame_Compiler_TypeChecker_LookupNodeType(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* node) {
    (void)self;
    (void)node;
    return Amalgame_Compiler_TypeChecker_GetType(self, node);
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
    if (code_string_equals(Amalgame_Compiler_TypeChecker_GenericBase(self, eBase), Amalgame_Compiler_TypeChecker_GenericBase(self, aBase))) {
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

static code_string Amalgame_Compiler_TypeChecker_GenericBase(Amalgame_Compiler_TypeChecker* self, code_string t) {
    (void)self;
    (void)t;
    i64 __attribute__((unused)) lt = String_IndexOf(t, "<");
    if (lt > 0) {
        return String_Substring(t, 0, lt);
    }
    return t;
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
    err->Snippet = Amalgame_Compiler_SourceMap_GetLine(self->Sources, file, line);
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
    if (String_Length(prog->Str2) > 0) {
        self->Filename = prog->Str2;
    }
    self->ExprTypeKeys = AmalgameList_new();
    self->ExprTypeVals = AmalgameList_new();
    self->LocalNames = AmalgameList_new();
    self->LocalTypes = AmalgameList_new();
    self->ScopeStarts = AmalgameList_new();
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
    if (!cls->Flag2 && String_Length(cls->Str4) > 0) {
        Amalgame_Compiler_TypeChecker_CheckImplementsContract(self, cls);
    }
    self->CurrentClass = prevClass;
}

static void Amalgame_Compiler_TypeChecker_CheckImplementsContract(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* cls) {
    (void)self;
    (void)cls;
    AmalgameList* __attribute__((unused)) entries = Amalgame_Compiler_TypeChecker_SplitTopLevelCommas(self, cls->Str4);
    i64 __attribute__((unused)) nEntries = AmalgameList_count(entries);
    for (i64 ie = 0; ie < nEntries; ie++) {
        void* __attribute__((unused)) iref = (void*)AmalgameList_get(entries, ie);
        Amalgame_Compiler_TypeChecker_CheckOneInterface(self, cls, iref);
    }
}

static void Amalgame_Compiler_TypeChecker_CheckOneInterface(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* cls, code_string iref) {
    (void)self;
    (void)cls;
    (void)iref;
    code_string __attribute__((unused)) ibase = iref;
    self->SubstParams = AmalgameList_new();
    self->SubstArgs = AmalgameList_new();
    i64 __attribute__((unused)) lt = String_IndexOf(iref, "<");
    if (lt > 0) {
        ibase = String_Substring(iref, 0, lt);
        code_string __attribute__((unused)) inner = String_Substring(iref, lt + 1, String_Length(iref) - lt - 2);
        AmalgameList* __attribute__((unused)) parts = Amalgame_Compiler_TypeChecker_SplitTopLevelCommas(self, inner);
        i64 __attribute__((unused)) np = AmalgameList_count(parts);
        for (i64 ia = 0; ia < np; ia++) {
            AmalgameList_add(self->SubstArgs, (void*)(intptr_t)((void*)AmalgameList_get(parts, ia)));
        }
    }
    Amalgame_Compiler_AstNode* __attribute__((unused)) iface = Amalgame_Compiler_TypeChecker_FindInterface(self, ibase);
    if (iface == NULL) {
        Amalgame_Compiler_TypeChecker_Error(self, code_string_concat(code_string_concat(code_string_concat(code_string_concat("class '", cls->Name), "' implements unknown interface '"), ibase), "'"), cls);
        return;
    }
    if (String_Length(iface->Str3) > 0) {
        AmalgameList* __attribute__((unused)) pp = Amalgame_Compiler_TypeChecker_SplitTopLevelCommas(self, iface->Str3);
        i64 __attribute__((unused)) pn = AmalgameList_count(pp);
        for (i64 ip = 0; ip < pn; ip++) {
            AmalgameList_add(self->SubstParams, (void*)(intptr_t)((void*)AmalgameList_get(pp, ip)));
        }
    }
    if (AmalgameList_count(self->SubstParams) != AmalgameList_count(self->SubstArgs)) {
        Amalgame_Compiler_TypeChecker_Error(self, code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("interface '", ibase), "' expects "), String_FromInt(AmalgameList_count(self->SubstParams))), " type argument(s), got "), String_FromInt(AmalgameList_count(self->SubstArgs))), cls);
        return;
    }
    i64 __attribute__((unused)) imethods = AmalgameList_count(iface->Children);
    for (i64 im = 0; im < imethods; im++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) imethod = (Amalgame_Compiler_AstNode*)AmalgameList_get(iface->Children, im);
        if (imethod->Kind == Amalgame_Compiler_NodeKind_METHOD_DECL) {
            Amalgame_Compiler_TypeChecker_CheckOneInterfaceMethod(self, cls, iref, imethod);
        }
    }
}

static void Amalgame_Compiler_TypeChecker_CheckOneInterfaceMethod(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* cls, code_string ifaceLabel, Amalgame_Compiler_AstNode* imethod) {
    (void)self;
    (void)cls;
    (void)ifaceLabel;
    (void)imethod;
    i64 __attribute__((unused)) cmems = AmalgameList_count(cls->Children);
    Amalgame_Compiler_AstNode* __attribute__((unused)) found = NULL;
    for (i64 ic = 0; ic < cmems; ic++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) m = (Amalgame_Compiler_AstNode*)AmalgameList_get(cls->Children, ic);
        if (m->Kind == Amalgame_Compiler_NodeKind_METHOD_DECL && code_string_equals(m->Name, imethod->Name)) {
            found = m;
        }
    }
    if (found == NULL) {
        Amalgame_Compiler_TypeChecker_Error(self, code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("class '", cls->Name), "' does not implement interface '"), ifaceLabel), "': missing method '"), imethod->Name), "'"), cls);
        return;
    }
    Amalgame_Compiler_AstNode* __attribute__((unused)) cmethod = found;
    code_string __attribute__((unused)) expectedRet = Amalgame_Compiler_TypeChecker_SubstType(self, imethod->Str);
    if (!code_string_equals(cmethod->Str, expectedRet)) {
        Amalgame_Compiler_TypeChecker_Error(self, code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("class '", cls->Name), "' method '"), imethod->Name), "': expected return type '"), expectedRet), "' (from interface '"), ifaceLabel), "'), got '"), cmethod->Str), "'"), cmethod);
        return;
    }
    i64 __attribute__((unused)) iN = AmalgameList_count(imethod->Params);
    i64 __attribute__((unused)) cN = AmalgameList_count(cmethod->Params);
    if (iN != cN) {
        Amalgame_Compiler_TypeChecker_Error(self, code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("class '", cls->Name), "' method '"), imethod->Name), "': expected "), String_FromInt(iN)), " param(s), got "), String_FromInt(cN)), cmethod);
        return;
    }
    for (i64 ip = 0; ip < iN; ip++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) ipar = (Amalgame_Compiler_AstNode*)AmalgameList_get(imethod->Params, ip);
        Amalgame_Compiler_AstNode* __attribute__((unused)) cpar = (Amalgame_Compiler_AstNode*)AmalgameList_get(cmethod->Params, ip);
        code_string __attribute__((unused)) expectedType = Amalgame_Compiler_TypeChecker_SubstType(self, ipar->Str);
        if (!code_string_equals(cpar->Str, expectedType)) {
            Amalgame_Compiler_TypeChecker_Error(self, code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("class '", cls->Name), "' method '"), imethod->Name), "' param '"), cpar->Name), "': expected type '"), expectedType), "' (from interface '"), ifaceLabel), "'), got '"), cpar->Str), "'"), cpar);
            return;
        }
    }
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_TypeChecker_FindInterface(Amalgame_Compiler_TypeChecker* self, code_string name) {
    (void)self;
    (void)name;
    i64 __attribute__((unused)) pn = Amalgame_Compiler_FullResolver_ProgramCount(self->Symbols);
    for (i64 ip = 0; ip < pn; ip++) {
        void* __attribute__((unused)) prog = Amalgame_Compiler_FullResolver_ProgramAt(self->Symbols, ip);
        Amalgame_Compiler_AstNode* __attribute__((unused)) hit = Amalgame_Compiler_TypeChecker_FindInterfaceInProgram(self, prog, name);
        if (hit != NULL) {
            return hit;
        }
    }
    return NULL;
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_TypeChecker_FindInterfaceInProgram(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* prog, code_string name) {
    (void)self;
    (void)prog;
    (void)name;
    i64 __attribute__((unused)) cn = AmalgameList_count(prog->Children);
    for (i64 ic = 0; ic < cn; ic++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) d = (Amalgame_Compiler_AstNode*)AmalgameList_get(prog->Children, ic);
        if (d->Kind == Amalgame_Compiler_NodeKind_CLASS_DECL && d->Flag2 && code_string_equals(d->Name, name)) {
            return d;
        }
    }
    return NULL;
}

static AmalgameList* Amalgame_Compiler_TypeChecker_SplitTopLevelCommas(Amalgame_Compiler_TypeChecker* self, code_string s) {
    (void)self;
    (void)s;
    AmalgameList* __attribute__((unused)) result = AmalgameList_new();
    i64 __attribute__((unused)) len = String_Length(s);
    if (len == 0) {
        return result;
    }
    i64 __attribute__((unused)) depth = 0;
    i64 __attribute__((unused)) start = 0;
    for (i64 i = 0; i < len; i++) {
        code_string __attribute__((unused)) c = String_CharAt1(s, i);
        if (code_string_equals(c, "<")) {
            depth = depth + 1;
        } else if (code_string_equals(c, ">")) {
            depth = depth - 1;
        } else if (code_string_equals(c, ",") && depth == 0) {
            code_string __attribute__((unused)) part = String_Trim(String_Substring(s, start, i - start));
            if (String_Length(part) > 0) {
                AmalgameList_add(result, (void*)(intptr_t)(part));
            }
            start = i + 1;
        }
    }
    code_string __attribute__((unused)) last = String_Trim(String_Substring(s, start, len - start));
    if (String_Length(last) > 0) {
        AmalgameList_add(result, (void*)(intptr_t)(last));
    }
    return result;
}

static code_string Amalgame_Compiler_TypeChecker_SubstType(Amalgame_Compiler_TypeChecker* self, code_string t) {
    (void)self;
    (void)t;
    if (String_Length(t) == 0) {
        return t;
    }
    code_bool __attribute__((unused)) nullable = 0;
    code_string __attribute__((unused)) core = t;
    if (String_EndsWith(t, "?")) {
        nullable = 1;
        core = String_Substring(t, 0, String_Length(t) - 1);
    }
    i64 __attribute__((unused)) lt = String_IndexOf(core, "<");
    if (lt < 0) {
        code_string __attribute__((unused)) resolved = Amalgame_Compiler_TypeChecker_LookupParam(self, core);
        if (nullable) {
            return code_string_concat(resolved, "?");
        }
        return resolved;
    }
    code_string __attribute__((unused)) base = String_Substring(core, 0, lt);
    code_string __attribute__((unused)) inner = String_Substring(core, lt + 1, String_Length(core) - lt - 2);
    AmalgameList* __attribute__((unused)) parts = Amalgame_Compiler_TypeChecker_SplitTopLevelCommas(self, inner);
    code_string __attribute__((unused)) newInner = "";
    i64 __attribute__((unused)) n = AmalgameList_count(parts);
    for (i64 i = 0; i < n; i++) {
        if (i > 0) {
            newInner = code_string_concat(newInner, ",");
        }
        newInner = code_string_concat(newInner, Amalgame_Compiler_TypeChecker_SubstType(self, (void*)AmalgameList_get(parts, i)));
    }
    code_string __attribute__((unused)) result = code_string_concat(code_string_concat(code_string_concat(base, "<"), newInner), ">");
    if (nullable) {
        return code_string_concat(result, "?");
    }
    return result;
}

static code_string Amalgame_Compiler_TypeChecker_LookupParam(Amalgame_Compiler_TypeChecker* self, code_string t) {
    (void)self;
    (void)t;
    i64 __attribute__((unused)) n = AmalgameList_count(self->SubstParams);
    for (i64 i = 0; i < n; i++) {
        if (code_string_equals((code_string)AmalgameList_get(self->SubstParams, i), t)) {
            return (code_string)AmalgameList_get(self->SubstArgs, i);
        }
    }
    return t;
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
    Amalgame_Compiler_TypeChecker_PushScope(self);
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
        Amalgame_Compiler_TypeChecker_DeclareLocal(self, p->Name, p->Str);
    }
    if (method->Body != NULL) {
        Amalgame_Compiler_TypeChecker_CheckBlock(self, method->Body);
    }
    Amalgame_Compiler_TypeChecker_PopScope(self);
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
    if (k == Amalgame_Compiler_NodeKind_TRY_STMT) {
        if (stmt->Body != NULL) {
            Amalgame_Compiler_TypeChecker_CheckBlock(self, stmt->Body);
        }
        if (stmt->Else != NULL) {
            Amalgame_Compiler_TypeChecker_PushScope(self);
            if (String_Length(stmt->Name) > 0) {
                Amalgame_Compiler_TypeChecker_DeclareLocal(self, stmt->Name, "void*");
            }
            Amalgame_Compiler_TypeChecker_CheckBlock(self, stmt->Else);
            Amalgame_Compiler_TypeChecker_PopScope(self);
        }
        if (stmt->Cond != NULL) {
            Amalgame_Compiler_TypeChecker_CheckBlock(self, stmt->Cond);
        }
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_THROW_STMT) {
        if (stmt->Left != NULL) {
            Amalgame_Compiler_TypeChecker_CheckExpr(self, stmt->Left);
        }
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
    if (code_string_equals(declaredType, "__tuple_destructure__")) {
        return;
    }
    code_string __attribute__((unused)) finalType = (String_Length(declaredType) > 0 ? declaredType : inferredType);
    if (String_Length(declaredType) > 0 && !code_string_equals(inferredType, "?")) {
        if (!Amalgame_Compiler_TypeChecker_IsAssignable(self, declaredType, inferredType)) {
            Amalgame_Compiler_TypeChecker_Error(self, code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("Cannot assign '", inferredType), "' to '"), stmt->Name), "' of type '"), declaredType), "'"), stmt);
        }
    }
    Amalgame_Compiler_TypeChecker_DeclareLocal(self, stmt->Name, finalType);
}

static void Amalgame_Compiler_TypeChecker_CheckReturn(Amalgame_Compiler_TypeChecker* self, Amalgame_Compiler_AstNode* stmt) {
    (void)self;
    (void)stmt;
    code_bool __attribute__((unused)) isBare = stmt->Left == NULL;
    if (!isBare && stmt->Left != NULL) {
        if (stmt->Left->Kind == Amalgame_Compiler_NodeKind_IDENTIFIER && code_string_equals(stmt->Left->Name, "_unknown_")) {
            isBare = 1;
        }
    }
    if (isBare) {
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
    Amalgame_Compiler_TypeChecker_PushScope(self);
    if (stmt->Left != NULL) {
        Amalgame_Compiler_TypeChecker_CheckExpr(self, stmt->Left);
        code_string __attribute__((unused)) colType = Amalgame_Compiler_TypeChecker_GetType(self, stmt->Left);
        code_string __attribute__((unused)) elemType = Amalgame_Compiler_TypeChecker_CollectionElementType(self, colType);
        Amalgame_Compiler_TypeChecker_DeclareLocal(self, stmt->Name, elemType);
    }
    if (stmt->Body != NULL) {
        Amalgame_Compiler_TypeChecker_CheckBlock(self, stmt->Body);
    }
    Amalgame_Compiler_TypeChecker_PopScope(self);
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
        code_string __attribute__((unused)) t = Amalgame_Compiler_TypeChecker_LookupLocal(self, expr->Name);
        if (String_Length(t) == 0) {
            t = Amalgame_Compiler_TypeChecker_SymbolTypeName(self, expr->Name);
        }
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
        code_string __attribute__((unused)) ifType = (!code_string_equals(thenType, "?") ? thenType : elseType);
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
    code_string __attribute__((unused)) lt = (expr->Left != NULL ? Amalgame_Compiler_TypeChecker_GetType(self, expr->Left) : "?");
    code_string __attribute__((unused)) rt = (expr->Right != NULL ? Amalgame_Compiler_TypeChecker_GetType(self, expr->Right) : "?");
    if (code_string_equals(op, "&&") || code_string_equals(op, "||")) {
        if (!Amalgame_Compiler_TypeChecker_IsBool(self, lt)) {
            Amalgame_Compiler_TypeChecker_Error(self, code_string_concat(code_string_concat(code_string_concat(code_string_concat("Left operand of '", op), "' must be bool, got '"), lt), "'"), expr->Left);
        }
        if (!Amalgame_Compiler_TypeChecker_IsBool(self, rt)) {
            Amalgame_Compiler_TypeChecker_Error(self, code_string_concat(code_string_concat(code_string_concat(code_string_concat("Right operand of '", op), "' must be bool, got '"), rt), "'"), expr->Right);
        }
    }
    if (code_string_equals(op, "-") || code_string_equals(op, "*") || code_string_equals(op, "/") || code_string_equals(op, "%") || code_string_equals(op, "^")) {
        if (!Amalgame_Compiler_TypeChecker_IsNumeric(self, lt) && !code_string_equals(lt, "?")) {
            Amalgame_Compiler_TypeChecker_Error(self, code_string_concat(code_string_concat(code_string_concat(code_string_concat("Operator '", op), "' requires numeric operands, got '"), lt), "'"), expr->Left);
        }
        if (!Amalgame_Compiler_TypeChecker_IsNumeric(self, rt) && !code_string_equals(rt, "?")) {
            Amalgame_Compiler_TypeChecker_Error(self, code_string_concat(code_string_concat(code_string_concat(code_string_concat("Operator '", op), "' requires numeric operands, got '"), rt), "'"), expr->Right);
        }
    }
    if (code_string_equals(op, "??")) {
        if (!Amalgame_Compiler_TypeChecker_IsNullable(self, lt) && !code_string_equals(lt, "?")) {
            code_string __attribute__((unused)) qqOp = "?";
            code_string __attribute__((unused)) qqOp2 = "?";
            Amalgame_Compiler_TypeChecker_Error(self, code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("Left operand of '", qqOp), qqOp2), "' must be nullable, got '"), lt), "'"), expr->Left);
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
    code_string __attribute__((unused)) ot = (expr->Left != NULL ? Amalgame_Compiler_TypeChecker_GetType(self, expr->Left) : "?");
    if (code_string_equals(op, "!")) {
        if (!Amalgame_Compiler_TypeChecker_IsBool(self, ot)) {
            Amalgame_Compiler_TypeChecker_Error(self, code_string_concat(code_string_concat("Operator '!' requires bool, got '", ot), "'"), expr->Left);
        }
        Amalgame_Compiler_TypeChecker_SetType(self, expr, "bool");
        return;
    }
    if (code_string_equals(op, "-")) {
        if (!Amalgame_Compiler_TypeChecker_IsNumeric(self, ot) && !code_string_equals(ot, "?")) {
            Amalgame_Compiler_TypeChecker_Error(self, code_string_concat(code_string_concat("Unary '-' requires numeric, got '", ot), "'"), expr->Left);
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
    code_string __attribute__((unused)) targetType = (expr->Left != NULL ? Amalgame_Compiler_TypeChecker_GetType(self, expr->Left) : "?");
    code_string __attribute__((unused)) baseType = targetType;
    if (String_EndsWith(baseType, "?")) {
        baseType = String_Substring(baseType, 0, String_Length(baseType) - 1);
    }
    if (String_EndsWith(baseType, "*")) {
        baseType = String_Substring(baseType, 0, String_Length(baseType) - 1);
    }
    code_string __attribute__((unused)) memberType = "?";
    if (String_Length(baseType) > 0 && !code_string_equals(baseType, "?")) {
        memberType = Amalgame_Compiler_FullResolver_GetMemberType(self->Symbols, baseType, expr->Name);
    }
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
    code_string __attribute__((unused)) targetType = (expr->Left != NULL ? Amalgame_Compiler_TypeChecker_GetType(self, expr->Left) : "?");
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

struct _Amalgame_Compiler_LintWarning {
    code_string Message;
    code_string Filename;
    i64 Line;
    i64 Column;
};

code_string Amalgame_Compiler_LintWarning_Format(Amalgame_Compiler_LintWarning* self);

Amalgame_Compiler_LintWarning* Amalgame_Compiler_LintWarning_new(code_string msg, code_string file, i64 line, i64 col) {
    Amalgame_Compiler_LintWarning* self = (Amalgame_Compiler_LintWarning*) GC_MALLOC(sizeof(Amalgame_Compiler_LintWarning));
    self->Message = msg;
    self->Filename = file;
    self->Line = line;
    self->Column = col;
    return self;
}

code_string Amalgame_Compiler_LintWarning_Format(Amalgame_Compiler_LintWarning* self) {
    (void)self;
    code_string __attribute__((unused)) ln = String_FromInt(self->Line);
    code_string __attribute__((unused)) col = String_FromInt(self->Column);
    return code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("warning[lint]: ", self->Message), "\n  --> "), self->Filename), ":"), ln), ":"), col), "\n");
}

struct _Amalgame_Compiler_Linter {
    AmalgameList* Warnings;
    code_string Filename;
    AmalgameList* LocalNames;
    AmalgameList* LocalLines;
    AmalgameList* LocalCols;
    AmalgameList* LocalIsParam;
    AmalgameList* LocalUseSnap;
    AmalgameList* ScopeStarts;
    AmalgameList* UsedNames;
};

void Amalgame_Compiler_Linter_Lint(Amalgame_Compiler_Linter* self, Amalgame_Compiler_AstNode* prog);
code_bool Amalgame_Compiler_Linter_HasWarnings(Amalgame_Compiler_Linter* self);
code_string Amalgame_Compiler_Linter_FormatWarnings(Amalgame_Compiler_Linter* self);
static void Amalgame_Compiler_Linter_LintDecl(Amalgame_Compiler_Linter* self, Amalgame_Compiler_AstNode* decl);
static void Amalgame_Compiler_Linter_LintMethod(Amalgame_Compiler_Linter* self, Amalgame_Compiler_AstNode* method);
static void Amalgame_Compiler_Linter_LintBlock(Amalgame_Compiler_Linter* self, Amalgame_Compiler_AstNode* block);
static code_bool Amalgame_Compiler_Linter_IsTerminator(Amalgame_Compiler_Linter* self, Amalgame_Compiler_AstNode* stmt);
static void Amalgame_Compiler_Linter_LintStmt(Amalgame_Compiler_Linter* self, Amalgame_Compiler_AstNode* stmt);
static void Amalgame_Compiler_Linter_LintBlockOrStmt(Amalgame_Compiler_Linter* self, Amalgame_Compiler_AstNode* n);
static void Amalgame_Compiler_Linter_LintExpr(Amalgame_Compiler_Linter* self, Amalgame_Compiler_AstNode* expr);
static void Amalgame_Compiler_Linter_PushScope(Amalgame_Compiler_Linter* self);
static void Amalgame_Compiler_Linter_PopScope(Amalgame_Compiler_Linter* self);
static void Amalgame_Compiler_Linter_DeclareLocal(Amalgame_Compiler_Linter* self, code_string name, i64 line, i64 col, code_bool isParam);
static code_bool Amalgame_Compiler_Linter_IsShadowing(Amalgame_Compiler_Linter* self, code_string name);
static void Amalgame_Compiler_Linter_MarkUsed(Amalgame_Compiler_Linter* self, code_string name);
static code_bool Amalgame_Compiler_Linter_UsedAfter(Amalgame_Compiler_Linter* self, code_string name, i64 fromIdx);
static void Amalgame_Compiler_Linter_Warn(Amalgame_Compiler_Linter* self, code_string msg, Amalgame_Compiler_AstNode* node);

Amalgame_Compiler_Linter* Amalgame_Compiler_Linter_new() {
    Amalgame_Compiler_Linter* self = (Amalgame_Compiler_Linter*) GC_MALLOC(sizeof(Amalgame_Compiler_Linter));
    self->Warnings = AmalgameList_new();
    self->Filename = "";
    self->LocalNames = AmalgameList_new();
    self->LocalLines = AmalgameList_new();
    self->LocalCols = AmalgameList_new();
    self->LocalIsParam = AmalgameList_new();
    self->LocalUseSnap = AmalgameList_new();
    self->ScopeStarts = AmalgameList_new();
    self->UsedNames = AmalgameList_new();
    return self;
}

void Amalgame_Compiler_Linter_Lint(Amalgame_Compiler_Linter* self, Amalgame_Compiler_AstNode* prog) {
    (void)self;
    (void)prog;
    if (String_Length(prog->Str2) > 0) {
        self->Filename = prog->Str2;
    }
    i64 __attribute__((unused)) n = AmalgameList_count(prog->Children);
    for (i64 i = 0; i < n; i++) {
        Amalgame_Compiler_Linter_LintDecl(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(prog->Children, i));
    }
}

code_bool Amalgame_Compiler_Linter_HasWarnings(Amalgame_Compiler_Linter* self) {
    (void)self;
    return AmalgameList_count(self->Warnings) > 0;
}

code_string Amalgame_Compiler_Linter_FormatWarnings(Amalgame_Compiler_Linter* self) {
    (void)self;
    code_string __attribute__((unused)) s = "";
    i64 __attribute__((unused)) n = AmalgameList_count(self->Warnings);
    for (i64 i = 0; i < n; i++) {
        Amalgame_Compiler_LintWarning* __attribute__((unused)) w = (Amalgame_Compiler_LintWarning*)AmalgameList_get(self->Warnings, i);
        s = code_string_concat(s, Amalgame_Compiler_LintWarning_Format(w));
    }
    return s;
}

static void Amalgame_Compiler_Linter_LintDecl(Amalgame_Compiler_Linter* self, Amalgame_Compiler_AstNode* decl) {
    (void)self;
    (void)decl;
    Amalgame_Compiler_NodeKind __attribute__((unused)) k = decl->Kind;
    if (k == Amalgame_Compiler_NodeKind_CLASS_DECL) {
        i64 __attribute__((unused)) members = AmalgameList_count(decl->Children);
        for (i64 i = 0; i < members; i++) {
            Amalgame_Compiler_AstNode* __attribute__((unused)) m = (Amalgame_Compiler_AstNode*)AmalgameList_get(decl->Children, i);
            if (m->Kind == Amalgame_Compiler_NodeKind_METHOD_DECL) {
                Amalgame_Compiler_Linter_LintMethod(self, m);
            }
        }
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_METHOD_DECL) {
        Amalgame_Compiler_Linter_LintMethod(self, decl);
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_ENUM_DECL) {
        i64 __attribute__((unused)) methods = AmalgameList_count(decl->Children);
        for (i64 i = 0; i < methods; i++) {
            Amalgame_Compiler_AstNode* __attribute__((unused)) m = (Amalgame_Compiler_AstNode*)AmalgameList_get(decl->Children, i);
            if (m->Kind == Amalgame_Compiler_NodeKind_METHOD_DECL) {
                Amalgame_Compiler_Linter_LintMethod(self, m);
            }
        }
    }
}

static void Amalgame_Compiler_Linter_LintMethod(Amalgame_Compiler_Linter* self, Amalgame_Compiler_AstNode* method) {
    (void)self;
    (void)method;
    self->UsedNames = AmalgameList_new();
    Amalgame_Compiler_Linter_PushScope(self);
    i64 __attribute__((unused)) pn = AmalgameList_count(method->Params);
    for (i64 i = 0; i < pn; i++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) p = (Amalgame_Compiler_AstNode*)AmalgameList_get(method->Params, i);
        Amalgame_Compiler_Linter_DeclareLocal(self, p->Name, p->Line, p->Column, 1);
    }
    if (method->Body != NULL) {
        Amalgame_Compiler_Linter_LintBlock(self, method->Body);
    }
    Amalgame_Compiler_Linter_PopScope(self);
}

static void Amalgame_Compiler_Linter_LintBlock(Amalgame_Compiler_Linter* self, Amalgame_Compiler_AstNode* block) {
    (void)self;
    (void)block;
    if (block == NULL) {
        return;
    }
    Amalgame_Compiler_Linter_PushScope(self);
    i64 __attribute__((unused)) n = AmalgameList_count(block->Children);
    code_bool __attribute__((unused)) dead = 0;
    for (i64 i = 0; i < n; i++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) stmt = (Amalgame_Compiler_AstNode*)AmalgameList_get(block->Children, i);
        if (dead) {
            Amalgame_Compiler_Linter_Warn(self, "unreachable code after `return` / `throw` / `break` / `continue`", stmt);
            Amalgame_Compiler_Linter_PopScope(self);
            return;
        }
        Amalgame_Compiler_Linter_LintStmt(self, stmt);
        if (Amalgame_Compiler_Linter_IsTerminator(self, stmt)) {
            dead = 1;
        }
    }
    Amalgame_Compiler_Linter_PopScope(self);
}

static code_bool Amalgame_Compiler_Linter_IsTerminator(Amalgame_Compiler_Linter* self, Amalgame_Compiler_AstNode* stmt) {
    (void)self;
    (void)stmt;
    Amalgame_Compiler_NodeKind __attribute__((unused)) k = stmt->Kind;
    if (k == Amalgame_Compiler_NodeKind_RETURN_STMT) {
        return 1;
    }
    if (k == Amalgame_Compiler_NodeKind_BREAK_STMT) {
        return 1;
    }
    if (k == Amalgame_Compiler_NodeKind_CONTINUE_STMT) {
        return 1;
    }
    if (k == Amalgame_Compiler_NodeKind_THROW_STMT) {
        return 1;
    }
    return 0;
}

static void Amalgame_Compiler_Linter_LintStmt(Amalgame_Compiler_Linter* self, Amalgame_Compiler_AstNode* stmt) {
    (void)self;
    (void)stmt;
    if (stmt == NULL) {
        return;
    }
    Amalgame_Compiler_NodeKind __attribute__((unused)) k = stmt->Kind;
    if (k == Amalgame_Compiler_NodeKind_VAR_DECL) {
        if (stmt->Left != NULL) {
            Amalgame_Compiler_Linter_LintExpr(self, stmt->Left);
        }
        Amalgame_Compiler_Linter_DeclareLocal(self, stmt->Name, stmt->Line, stmt->Column, 0);
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_ASSIGN) {
        if (stmt->Left != NULL) {
            Amalgame_Compiler_Linter_LintExpr(self, stmt->Left);
        }
        if (stmt->Right != NULL) {
            Amalgame_Compiler_Linter_LintExpr(self, stmt->Right);
        }
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_RETURN_STMT) {
        if (stmt->Left != NULL) {
            Amalgame_Compiler_Linter_LintExpr(self, stmt->Left);
        }
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_THROW_STMT) {
        if (stmt->Left != NULL) {
            Amalgame_Compiler_Linter_LintExpr(self, stmt->Left);
        }
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_IF_STMT) {
        if (code_string_equals(stmt->Name, "__match__")) {
            if (stmt->Left != NULL) {
                Amalgame_Compiler_Linter_LintExpr(self, stmt->Left);
            }
            i64 __attribute__((unused)) an = AmalgameList_count(stmt->Children);
            for (i64 ai = 0; ai < an; ai++) {
                Amalgame_Compiler_AstNode* __attribute__((unused)) arm = (Amalgame_Compiler_AstNode*)AmalgameList_get(stmt->Children, ai);
                if (arm->Body != NULL) {
                    Amalgame_Compiler_Linter_LintBlockOrStmt(self, arm->Body);
                }
            }
            return;
        }
        if (stmt->Cond != NULL) {
            Amalgame_Compiler_Linter_LintExpr(self, stmt->Cond);
        }
        if (stmt->Body != NULL) {
            Amalgame_Compiler_Linter_LintBlockOrStmt(self, stmt->Body);
        }
        if (stmt->Else != NULL) {
            Amalgame_Compiler_Linter_LintBlockOrStmt(self, stmt->Else);
        }
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_WHILE_STMT) {
        if (stmt->Cond != NULL) {
            Amalgame_Compiler_Linter_LintExpr(self, stmt->Cond);
        }
        if (stmt->Body != NULL) {
            Amalgame_Compiler_Linter_LintBlockOrStmt(self, stmt->Body);
        }
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_FOR_IN_STMT) {
        if (stmt->Left != NULL) {
            Amalgame_Compiler_Linter_LintExpr(self, stmt->Left);
        }
        Amalgame_Compiler_Linter_PushScope(self);
        Amalgame_Compiler_Linter_DeclareLocal(self, stmt->Name, stmt->Line, stmt->Column, 0);
        if (stmt->Body != NULL) {
            Amalgame_Compiler_Linter_LintBlockOrStmt(self, stmt->Body);
        }
        Amalgame_Compiler_Linter_PopScope(self);
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_TRY_STMT) {
        if (stmt->Body != NULL) {
            Amalgame_Compiler_Linter_LintBlockOrStmt(self, stmt->Body);
        }
        if (stmt->Else != NULL) {
            Amalgame_Compiler_Linter_LintBlockOrStmt(self, stmt->Else);
        }
        if (stmt->Cond != NULL) {
            Amalgame_Compiler_Linter_LintBlockOrStmt(self, stmt->Cond);
        }
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_BLOCK) {
        Amalgame_Compiler_Linter_LintBlock(self, stmt);
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_CALL || k == Amalgame_Compiler_NodeKind_MEMBER || k == Amalgame_Compiler_NodeKind_BINARY || k == Amalgame_Compiler_NodeKind_UNARY || k == Amalgame_Compiler_NodeKind_IDENTIFIER) {
        Amalgame_Compiler_Linter_LintExpr(self, stmt);
    }
}

static void Amalgame_Compiler_Linter_LintBlockOrStmt(Amalgame_Compiler_Linter* self, Amalgame_Compiler_AstNode* n) {
    (void)self;
    (void)n;
    if (n->Kind == Amalgame_Compiler_NodeKind_BLOCK) {
        Amalgame_Compiler_Linter_LintBlock(self, n);
    } else {
        Amalgame_Compiler_Linter_LintStmt(self, n);
    }
}

static void Amalgame_Compiler_Linter_LintExpr(Amalgame_Compiler_Linter* self, Amalgame_Compiler_AstNode* expr) {
    (void)self;
    (void)expr;
    if (expr == NULL) {
        return;
    }
    Amalgame_Compiler_NodeKind __attribute__((unused)) k = expr->Kind;
    if (k == Amalgame_Compiler_NodeKind_IDENTIFIER) {
        if (code_string_equals(expr->Name, "_unknown_")) {
            return;
        }
        if (code_string_equals(expr->Name, "_")) {
            return;
        }
        Amalgame_Compiler_Linter_MarkUsed(self, expr->Name);
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_BINARY) {
        if (expr->Left != NULL) {
            Amalgame_Compiler_Linter_LintExpr(self, expr->Left);
        }
        if (expr->Right != NULL) {
            Amalgame_Compiler_Linter_LintExpr(self, expr->Right);
        }
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_UNARY) {
        if (expr->Left != NULL) {
            Amalgame_Compiler_Linter_LintExpr(self, expr->Left);
        }
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_MEMBER) {
        if (expr->Left != NULL) {
            Amalgame_Compiler_Linter_LintExpr(self, expr->Left);
        }
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_CALL) {
        if (expr->Left != NULL) {
            Amalgame_Compiler_Linter_LintExpr(self, expr->Left);
        }
        i64 __attribute__((unused)) argc = AmalgameList_count(expr->Args);
        for (i64 i = 0; i < argc; i++) {
            Amalgame_Compiler_Linter_LintExpr(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(expr->Args, i));
        }
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_NEW_EXPR) {
        i64 __attribute__((unused)) argc = AmalgameList_count(expr->Args);
        for (i64 i = 0; i < argc; i++) {
            Amalgame_Compiler_Linter_LintExpr(self, (Amalgame_Compiler_AstNode*)AmalgameList_get(expr->Args, i));
        }
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_INDEX_EXPR) {
        if (expr->Left != NULL) {
            Amalgame_Compiler_Linter_LintExpr(self, expr->Left);
        }
        if (expr->Right != NULL) {
            Amalgame_Compiler_Linter_LintExpr(self, expr->Right);
        }
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_LIST_COMP) {
        if (expr->Right != NULL) {
            Amalgame_Compiler_Linter_LintExpr(self, expr->Right);
        }
        Amalgame_Compiler_Linter_PushScope(self);
        Amalgame_Compiler_Linter_DeclareLocal(self, expr->Str, expr->Line, expr->Column, 0);
        if (expr->Left != NULL) {
            Amalgame_Compiler_Linter_LintExpr(self, expr->Left);
        }
        if (expr->Cond != NULL) {
            Amalgame_Compiler_Linter_LintExpr(self, expr->Cond);
        }
        Amalgame_Compiler_Linter_PopScope(self);
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_ASSIGN) {
        if (expr->Left != NULL) {
            Amalgame_Compiler_Linter_LintExpr(self, expr->Left);
        }
        if (expr->Right != NULL) {
            Amalgame_Compiler_Linter_LintExpr(self, expr->Right);
        }
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_IF_STMT) {
        if (code_string_equals(expr->Name, "__match__")) {
            if (expr->Left != NULL) {
                Amalgame_Compiler_Linter_LintExpr(self, expr->Left);
            }
            i64 __attribute__((unused)) an = AmalgameList_count(expr->Children);
            for (i64 ai = 0; ai < an; ai++) {
                Amalgame_Compiler_AstNode* __attribute__((unused)) arm = (Amalgame_Compiler_AstNode*)AmalgameList_get(expr->Children, ai);
                if (arm->Body != NULL) {
                    Amalgame_Compiler_Linter_LintBlockOrStmt(self, arm->Body);
                }
            }
            return;
        }
        if (expr->Cond != NULL) {
            Amalgame_Compiler_Linter_LintExpr(self, expr->Cond);
        }
        if (expr->Body != NULL) {
            Amalgame_Compiler_Linter_LintBlockOrStmt(self, expr->Body);
        }
        if (expr->Else != NULL) {
            Amalgame_Compiler_Linter_LintBlockOrStmt(self, expr->Else);
        }
        return;
    }
    if (k == Amalgame_Compiler_NodeKind_METHOD_DECL && code_string_equals(expr->Name, "__lambda__")) {
        Amalgame_Compiler_Linter_PushScope(self);
        Amalgame_Compiler_Linter_DeclareLocal(self, expr->Str, expr->Line, expr->Column, 1);
        if (expr->Left != NULL) {
            Amalgame_Compiler_Linter_LintExpr(self, expr->Left);
        }
        Amalgame_Compiler_Linter_PopScope(self);
        return;
    }
}

static void Amalgame_Compiler_Linter_PushScope(Amalgame_Compiler_Linter* self) {
    (void)self;
    AmalgameList_add(self->ScopeStarts, (void*)(intptr_t)(AmalgameList_count(self->LocalNames)));
}

static void Amalgame_Compiler_Linter_PopScope(Amalgame_Compiler_Linter* self) {
    (void)self;
    i64 __attribute__((unused)) depth = AmalgameList_count(self->ScopeStarts);
    if (depth == 0) {
        return;
    }
    i64 __attribute__((unused)) mark = (i64)AmalgameList_get(self->ScopeStarts, depth - 1);
    AmalgameList_removeAt(self->ScopeStarts, depth - 1);
    while (AmalgameList_count(self->LocalNames) > mark) {
        i64 __attribute__((unused)) last = AmalgameList_count(self->LocalNames) - 1;
        code_string __attribute__((unused)) name = (code_string)AmalgameList_get(self->LocalNames, last);
        i64 __attribute__((unused)) line = (i64)AmalgameList_get(self->LocalLines, last);
        i64 __attribute__((unused)) col = (i64)AmalgameList_get(self->LocalCols, last);
        code_bool __attribute__((unused)) isParam = (code_bool)AmalgameList_get(self->LocalIsParam, last);
        i64 __attribute__((unused)) snap = (i64)AmalgameList_get(self->LocalUseSnap, last);
        if (!isParam && !String_StartsWith(name, "_")) {
            if (!Amalgame_Compiler_Linter_UsedAfter(self, name, snap)) {
                Amalgame_Compiler_LintWarning* __attribute__((unused)) w = Amalgame_Compiler_LintWarning_new(code_string_concat(code_string_concat("unused local '", name), "' (prefix with '_' to silence)"), self->Filename, line, col);
                AmalgameList_add(self->Warnings, (void*)(intptr_t)(w));
            }
        }
        AmalgameList_removeAt(self->LocalNames, last);
        AmalgameList_removeAt(self->LocalLines, last);
        AmalgameList_removeAt(self->LocalCols, last);
        AmalgameList_removeAt(self->LocalIsParam, last);
        AmalgameList_removeAt(self->LocalUseSnap, last);
    }
}

static void Amalgame_Compiler_Linter_DeclareLocal(Amalgame_Compiler_Linter* self, code_string name, i64 line, i64 col, code_bool isParam) {
    (void)self;
    (void)name;
    (void)line;
    (void)col;
    (void)isParam;
    if (Amalgame_Compiler_Linter_IsShadowing(self, name)) {
        Amalgame_Compiler_LintWarning* __attribute__((unused)) w = Amalgame_Compiler_LintWarning_new(code_string_concat(code_string_concat("'", name), "' shadows an enclosing binding"), self->Filename, line, col);
        AmalgameList_add(self->Warnings, (void*)(intptr_t)(w));
    }
    AmalgameList_add(self->LocalNames, (void*)(intptr_t)(name));
    AmalgameList_add(self->LocalLines, (void*)(intptr_t)(line));
    AmalgameList_add(self->LocalCols, (void*)(intptr_t)(col));
    AmalgameList_add(self->LocalIsParam, (void*)(intptr_t)(isParam));
    AmalgameList_add(self->LocalUseSnap, (void*)(intptr_t)(AmalgameList_count(self->UsedNames)));
}

static code_bool Amalgame_Compiler_Linter_IsShadowing(Amalgame_Compiler_Linter* self, code_string name) {
    (void)self;
    (void)name;
    i64 __attribute__((unused)) depth = AmalgameList_count(self->ScopeStarts);
    if (depth == 0) {
        return 0;
    }
    i64 __attribute__((unused)) curStart = (i64)AmalgameList_get(self->ScopeStarts, depth - 1);
    i64 __attribute__((unused)) i = curStart - 1;
    while (i >= 0) {
        if (code_string_equals((code_string)AmalgameList_get(self->LocalNames, i), name)) {
            return 1;
        }
        i = i - 1;
    }
    return 0;
}

static void Amalgame_Compiler_Linter_MarkUsed(Amalgame_Compiler_Linter* self, code_string name) {
    (void)self;
    (void)name;
    AmalgameList_add(self->UsedNames, (void*)(intptr_t)(name));
}

static code_bool Amalgame_Compiler_Linter_UsedAfter(Amalgame_Compiler_Linter* self, code_string name, i64 fromIdx) {
    (void)self;
    (void)name;
    (void)fromIdx;
    i64 __attribute__((unused)) n = AmalgameList_count(self->UsedNames);
    i64 __attribute__((unused)) i = fromIdx;
    while (i < n) {
        code_string __attribute__((unused)) u = (code_string)AmalgameList_get(self->UsedNames, i);
        if (code_string_equals(u, name)) {
            return 1;
        }
        i = i + 1;
    }
    return 0;
}

static void Amalgame_Compiler_Linter_Warn(Amalgame_Compiler_Linter* self, code_string msg, Amalgame_Compiler_AstNode* node) {
    (void)self;
    (void)msg;
    (void)node;
    i64 __attribute__((unused)) line = 0;
    i64 __attribute__((unused)) col = 0;
    if (node != NULL) {
        line = node->Line;
        col = node->Column;
    }
    Amalgame_Compiler_LintWarning* __attribute__((unused)) w = Amalgame_Compiler_LintWarning_new(msg, self->Filename, line, col);
    AmalgameList_add(self->Warnings, (void*)(intptr_t)(w));
}

struct _Amalgame_Compiler_LspServer {
    AmalgameList* DocUris;
    AmalgameList* Docs;
};

i64 Amalgame_Compiler_LspServer_Run(Amalgame_Compiler_LspServer* self);
static void Amalgame_Compiler_LspServer_UpsertDoc(Amalgame_Compiler_LspServer* self, code_string uri, code_string text);
static void Amalgame_Compiler_LspServer_RemoveDoc(Amalgame_Compiler_LspServer* self, code_string uri);
static code_string Amalgame_Compiler_LspServer_LookupDoc(Amalgame_Compiler_LspServer* self, code_string uri);
static code_string Amalgame_Compiler_LspServer_ReadMessage(Amalgame_Compiler_LspServer* self);
code_string Amalgame_Compiler_LspServer_Cr();
static void Amalgame_Compiler_LspServer_Send(Amalgame_Compiler_LspServer* self, code_string body);
static void Amalgame_Compiler_LspServer_SendInit(Amalgame_Compiler_LspServer* self, i64 id);
static void Amalgame_Compiler_LspServer_SendShutdown(Amalgame_Compiler_LspServer* self, i64 id);
static void Amalgame_Compiler_LspServer_PublishDiagnostics(Amalgame_Compiler_LspServer* self, code_string uri, code_string source);
static Amalgame_Compiler_FullResolver* Amalgame_Compiler_LspServer_BuildWorkspaceResolver(Amalgame_Compiler_LspServer* self, code_string path, Amalgame_Compiler_AstNode* prog);
static void Amalgame_Compiler_LspServer_LoadWorkspaceFiles(Amalgame_Compiler_LspServer* self, Amalgame_Compiler_FullResolver* resolver, code_string currentPath);
code_string Amalgame_Compiler_LspServer_FindWorkspaceRoot(code_string startPath);
code_string Amalgame_Compiler_LspServer_Dirname(code_string path);
static code_string Amalgame_Compiler_LspServer_Compile(Amalgame_Compiler_LspServer* self, code_string uri, code_string source);
static void Amalgame_Compiler_LspServer_HandleHover(Amalgame_Compiler_LspServer* self, i64 id, code_string uri, i64 line, i64 character);
static void Amalgame_Compiler_LspServer_SendNullResult(Amalgame_Compiler_LspServer* self, i64 id);
static void Amalgame_Compiler_LspServer_HandleCompletion(Amalgame_Compiler_LspServer* self, i64 id, code_string uri);
static void Amalgame_Compiler_LspServer_SendEmptyCompletion(Amalgame_Compiler_LspServer* self, i64 id);
code_string Amalgame_Compiler_LspServer_DiagnosticFromResolver(code_string source, Amalgame_Compiler_ResolverError* e);
code_string Amalgame_Compiler_LspServer_DiagnosticFromTc(code_string source, Amalgame_Compiler_TypeError* e);
static code_string Amalgame_Compiler_LspServer_DiagnosticBody(code_string source, i64 line, i64 col, code_string msg);
static i64 Amalgame_Compiler_LspServer_TokenEndCol(code_string source, i64 line, i64 col);
static code_bool Amalgame_Compiler_LspServer_IsWordChar(code_string ch);
code_string Amalgame_Compiler_LspServer_EscapeJsonStr(code_string s);
code_string Amalgame_Compiler_LspServer_UriToPath(code_string uri);
Amalgame_Compiler_AstNode* Amalgame_Compiler_LspServer_FindNodeAtPosition(Amalgame_Compiler_AstNode* root, i64 line, i64 col);
static Amalgame_Compiler_AstNode* Amalgame_Compiler_LspServer_FindNodeWalk(Amalgame_Compiler_AstNode* node, i64 line, i64 col, Amalgame_Compiler_AstNode* best);
static code_bool Amalgame_Compiler_LspServer_NodeCovers(Amalgame_Compiler_AstNode* node, i64 line, i64 col);
code_string Amalgame_Compiler_LspServer_JsonStr(code_string body, code_string key);
i64 Amalgame_Compiler_LspServer_JsonInt(code_string body, code_string key);

Amalgame_Compiler_LspServer* Amalgame_Compiler_LspServer_new() {
    Amalgame_Compiler_LspServer* self = (Amalgame_Compiler_LspServer*) GC_MALLOC(sizeof(Amalgame_Compiler_LspServer));
    self->DocUris = AmalgameList_new();
    self->Docs = AmalgameList_new();
    return self;
}

i64 Amalgame_Compiler_LspServer_Run(Amalgame_Compiler_LspServer* self) {
    (void)self;
    while (1) {
        code_string __attribute__((unused)) body = Amalgame_Compiler_LspServer_ReadMessage(self);
        if (String_Length(body) == 0) {
            return 0;
        }
        code_string __attribute__((unused)) method = Amalgame_Compiler_LspServer_JsonStr(body, "method");
        if (code_string_equals(method, "initialize")) {
            i64 __attribute__((unused)) id = Amalgame_Compiler_LspServer_JsonInt(body, "id");
            Amalgame_Compiler_LspServer_SendInit(self, id);
        } else if (code_string_equals(method, "shutdown")) {
            i64 __attribute__((unused)) id = Amalgame_Compiler_LspServer_JsonInt(body, "id");
            Amalgame_Compiler_LspServer_SendShutdown(self, id);
        } else if (code_string_equals(method, "exit")) {
            return 0;
        } else if (code_string_equals(method, "textDocument/didOpen")) {
            code_string __attribute__((unused)) uri = Amalgame_Compiler_LspServer_JsonStr(body, "uri");
            code_string __attribute__((unused)) txt = Amalgame_Compiler_LspServer_JsonStr(body, "text");
            Amalgame_Compiler_LspServer_UpsertDoc(self, uri, txt);
            Amalgame_Compiler_LspServer_PublishDiagnostics(self, uri, txt);
        } else if (code_string_equals(method, "textDocument/didChange")) {
            code_string __attribute__((unused)) uri = Amalgame_Compiler_LspServer_JsonStr(body, "uri");
            code_string __attribute__((unused)) txt = Amalgame_Compiler_LspServer_JsonStr(body, "text");
            Amalgame_Compiler_LspServer_UpsertDoc(self, uri, txt);
            Amalgame_Compiler_LspServer_PublishDiagnostics(self, uri, txt);
        } else if (code_string_equals(method, "textDocument/didClose")) {
            code_string __attribute__((unused)) uri = Amalgame_Compiler_LspServer_JsonStr(body, "uri");
            Amalgame_Compiler_LspServer_RemoveDoc(self, uri);
        } else if (code_string_equals(method, "textDocument/hover")) {
            i64 __attribute__((unused)) id = Amalgame_Compiler_LspServer_JsonInt(body, "id");
            code_string __attribute__((unused)) uri = Amalgame_Compiler_LspServer_JsonStr(body, "uri");
            i64 __attribute__((unused)) line = Amalgame_Compiler_LspServer_JsonInt(body, "line");
            i64 __attribute__((unused)) chr = Amalgame_Compiler_LspServer_JsonInt(body, "character");
            Amalgame_Compiler_LspServer_HandleHover(self, id, uri, line, chr);
        } else if (code_string_equals(method, "textDocument/completion")) {
            i64 __attribute__((unused)) id = Amalgame_Compiler_LspServer_JsonInt(body, "id");
            code_string __attribute__((unused)) uri = Amalgame_Compiler_LspServer_JsonStr(body, "uri");
            Amalgame_Compiler_LspServer_HandleCompletion(self, id, uri);
        }
    }
    return 0;
}

static void Amalgame_Compiler_LspServer_UpsertDoc(Amalgame_Compiler_LspServer* self, code_string uri, code_string text) {
    (void)self;
    (void)uri;
    (void)text;
    i64 __attribute__((unused)) n = AmalgameList_count(self->DocUris);
    for (i64 i = 0; i < n; i++) {
        code_string __attribute__((unused)) u = (code_string)AmalgameList_get(self->DocUris, i);
        if (code_string_equals(u, uri)) {
            AmalgameList_removeAt(self->DocUris, i);
            AmalgameList_removeAt(self->Docs, i);
            AmalgameList_add(self->DocUris, (void*)(intptr_t)(uri));
            AmalgameList_add(self->Docs, (void*)(intptr_t)(text));
            return;
        }
    }
    AmalgameList_add(self->DocUris, (void*)(intptr_t)(uri));
    AmalgameList_add(self->Docs, (void*)(intptr_t)(text));
}

static void Amalgame_Compiler_LspServer_RemoveDoc(Amalgame_Compiler_LspServer* self, code_string uri) {
    (void)self;
    (void)uri;
    i64 __attribute__((unused)) n = AmalgameList_count(self->DocUris);
    for (i64 i = 0; i < n; i++) {
        code_string __attribute__((unused)) u = (code_string)AmalgameList_get(self->DocUris, i);
        if (code_string_equals(u, uri)) {
            AmalgameList_removeAt(self->DocUris, i);
            AmalgameList_removeAt(self->Docs, i);
            return;
        }
    }
}

static code_string Amalgame_Compiler_LspServer_LookupDoc(Amalgame_Compiler_LspServer* self, code_string uri) {
    (void)self;
    (void)uri;
    i64 __attribute__((unused)) n = AmalgameList_count(self->DocUris);
    for (i64 i = 0; i < n; i++) {
        code_string __attribute__((unused)) u = (code_string)AmalgameList_get(self->DocUris, i);
        if (code_string_equals(u, uri)) {
            return (code_string)AmalgameList_get(self->Docs, i);
        }
    }
    return "";
}

static code_string Amalgame_Compiler_LspServer_ReadMessage(Amalgame_Compiler_LspServer* self) {
    (void)self;
    i64 __attribute__((unused)) contentLen = 0;
    code_bool __attribute__((unused)) sawAnyHeader = 0;
    while (1) {
        code_string __attribute__((unused)) raw = Console_ReadLine();
        code_string __attribute__((unused)) line = String_TrimEnd(raw);
        if (String_Length(line) == 0) {
            if (String_Length(raw) == 0 && !sawAnyHeader) {
                return "";
            }
            break;
        }
        sawAnyHeader = 1;
        if (String_StartsWith(line, "Content-Length:")) {
            i64 __attribute__((unused)) lineLen = String_Length(line);
            code_string __attribute__((unused)) rest = String_Substring(line, 15, lineLen - 15);
            contentLen = String_ToInt(String_Trim(rest));
        }
    }
    if (contentLen <= 0) {
        return "";
    }
    return Console_ReadBytes(contentLen);
}

code_string Amalgame_Compiler_LspServer_Cr() {
    return String_FromByte(13);
}

static void Amalgame_Compiler_LspServer_Send(Amalgame_Compiler_LspServer* self, code_string body) {
    (void)self;
    (void)body;
    i64 __attribute__((unused)) n = String_Length(body);
    code_string __attribute__((unused)) crlf = code_string_concat(Amalgame_Compiler_LspServer_Cr(), "\n");
    Console_Write(code_string_concat(code_string_concat(code_string_concat("Content-Length: ", String_FromInt(n)), crlf), crlf));
    Console_Write(body);
    Console_Flush();
}

static void Amalgame_Compiler_LspServer_SendInit(Amalgame_Compiler_LspServer* self, i64 id) {
    (void)self;
    (void)id;
    code_string __attribute__((unused)) body = code_string_concat(code_string_concat("{\"jsonrpc\":\"2.0\",\"id\":", String_FromInt(id)), ",\"result\":{\"capabilities\":{\"textDocumentSync\":1,\"hoverProvider\":true,\"completionProvider\":{\"triggerCharacters\":[\".\"]}}}}");
    Amalgame_Compiler_LspServer_Send(self, body);
}

static void Amalgame_Compiler_LspServer_SendShutdown(Amalgame_Compiler_LspServer* self, i64 id) {
    (void)self;
    (void)id;
    code_string __attribute__((unused)) body = code_string_concat(code_string_concat("{\"jsonrpc\":\"2.0\",\"id\":", String_FromInt(id)), ",\"result\":null}");
    Amalgame_Compiler_LspServer_Send(self, body);
}

static void Amalgame_Compiler_LspServer_PublishDiagnostics(Amalgame_Compiler_LspServer* self, code_string uri, code_string source) {
    (void)self;
    (void)uri;
    (void)source;
    code_string __attribute__((unused)) diags = Amalgame_Compiler_LspServer_Compile(self, uri, source);
    code_string __attribute__((unused)) body = code_string_concat(code_string_concat(code_string_concat(code_string_concat("{\"jsonrpc\":\"2.0\",\"method\":\"textDocument/publishDiagnostics\",\"params\":{\"uri\":\"", Amalgame_Compiler_LspServer_EscapeJsonStr(uri)), "\",\"diagnostics\":"), diags), "}}");
    Amalgame_Compiler_LspServer_Send(self, body);
}

static Amalgame_Compiler_FullResolver* Amalgame_Compiler_LspServer_BuildWorkspaceResolver(Amalgame_Compiler_LspServer* self, code_string path, Amalgame_Compiler_AstNode* prog) {
    (void)self;
    (void)path;
    (void)prog;
    Amalgame_Compiler_FullResolver* __attribute__((unused)) resolver = Amalgame_Compiler_FullResolver_new();
    AmalgameList_add(resolver->Programs, (void*)(intptr_t)(prog));
    Amalgame_Compiler_LspServer_LoadWorkspaceFiles(self, resolver, path);
    Amalgame_Compiler_FullResolver_ResolvePrograms(resolver);
    return resolver;
}

static void Amalgame_Compiler_LspServer_LoadWorkspaceFiles(Amalgame_Compiler_LspServer* self, Amalgame_Compiler_FullResolver* resolver, code_string currentPath) {
    (void)self;
    (void)resolver;
    (void)currentPath;
    code_string __attribute__((unused)) root = Amalgame_Compiler_LspServer_FindWorkspaceRoot(currentPath);
    if (String_Length(root) == 0) {
        return;
    }
    code_string __attribute__((unused)) findCmd = code_string_concat(code_string_concat("find ", root), " -name '*.am' -type f 2>/dev/null");
    AmalgameProcessResult* __attribute__((unused)) result = Process_RunCapture(findCmd);
    if (result->Exit != 0) {
        return;
    }
    AmalgameList* __attribute__((unused)) lines = String_Split(String_Trim(result->Stdout), "\n");
    i64 __attribute__((unused)) n = AmalgameList_count(lines);
    for (i64 i = 0; i < n; i++) {
        code_string __attribute__((unused)) path = String_Trim((code_string)AmalgameList_get(lines, i));
        if (String_Length(path) == 0) {
            continue;
        }
        if (code_string_equals(path, currentPath)) {
            continue;
        }
        if (!File_Exists(path)) {
            continue;
        }
        code_string __attribute__((unused)) src = File_ReadAll(path);
        Amalgame_Compiler_Lexer* __attribute__((unused)) lex = Amalgame_Compiler_Lexer_new(src, path);
        AmalgameList* __attribute__((unused)) toks = Amalgame_Compiler_Lexer_Tokenize(lex);
        Amalgame_Compiler_Parser* __attribute__((unused)) par = Amalgame_Compiler_Parser_new(toks);
        Amalgame_Compiler_AstNode* __attribute__((unused)) p = Amalgame_Compiler_Parser_Parse(par);
        p->Str2 = path;
        AmalgameList_add(resolver->Programs, (void*)(intptr_t)(p));
    }
}

code_string Amalgame_Compiler_LspServer_FindWorkspaceRoot(code_string startPath) {
    (void)startPath;
    code_string __attribute__((unused)) dir = Amalgame_Compiler_LspServer_Dirname(startPath);
    code_string __attribute__((unused)) initialDir = dir;
    for (i64 i = 0; i < 8; i++) {
        if (String_Length(dir) == 0) {
            return initialDir;
        }
        if (File_Exists(code_string_concat(dir, "/.git"))) {
            return dir;
        }
        if (File_Exists(code_string_concat(dir, "/build_amc.sh"))) {
            return dir;
        }
        if (File_Exists(code_string_concat(dir, "/package.json"))) {
            return dir;
        }
        code_string __attribute__((unused)) parent = Amalgame_Compiler_LspServer_Dirname(dir);
        if (code_string_equals(parent, dir)) {
            return initialDir;
        }
        dir = parent;
    }
    return initialDir;
}

code_string Amalgame_Compiler_LspServer_Dirname(code_string path) {
    (void)path;
    i64 __attribute__((unused)) last = String_LastIndexOf(path, "/");
    if (last <= 0) {
        return "";
    }
    return String_Substring(path, 0, last);
}

static code_string Amalgame_Compiler_LspServer_Compile(Amalgame_Compiler_LspServer* self, code_string uri, code_string source) {
    (void)self;
    (void)uri;
    (void)source;
    code_string __attribute__((unused)) path = Amalgame_Compiler_LspServer_UriToPath(uri);
    Amalgame_Compiler_Lexer* __attribute__((unused)) lex = Amalgame_Compiler_Lexer_new(source, path);
    AmalgameList* __attribute__((unused)) toks = Amalgame_Compiler_Lexer_Tokenize(lex);
    Amalgame_Compiler_Parser* __attribute__((unused)) par = Amalgame_Compiler_Parser_new(toks);
    Amalgame_Compiler_AstNode* __attribute__((unused)) prog = Amalgame_Compiler_Parser_Parse(par);
    prog->Str2 = path;
    Amalgame_Compiler_FullResolver* __attribute__((unused)) resolver = Amalgame_Compiler_LspServer_BuildWorkspaceResolver(self, path, prog);
    Amalgame_Compiler_TypeChecker* __attribute__((unused)) tc = Amalgame_Compiler_TypeChecker_new(resolver, path);
    Amalgame_Compiler_TypeChecker_Check(tc, prog);
    code_string __attribute__((unused)) arr = "[";
    code_bool __attribute__((unused)) first = 1;
    i64 __attribute__((unused)) rn = AmalgameList_count(resolver->RawErrors);
    for (i64 ri = 0; ri < rn; ri++) {
        Amalgame_Compiler_ResolverError* __attribute__((unused)) e = (Amalgame_Compiler_ResolverError*)AmalgameList_get(resolver->RawErrors, ri);
        if (!code_string_equals(e->Filename, path)) {
            continue;
        }
        if (!first) {
            arr = code_string_concat(arr, ",");
        }
        arr = code_string_concat(arr, Amalgame_Compiler_LspServer_DiagnosticFromResolver(source, e));
        first = 0;
    }
    i64 __attribute__((unused)) tn = AmalgameList_count(tc->Errors);
    for (i64 ti = 0; ti < tn; ti++) {
        Amalgame_Compiler_TypeError* __attribute__((unused)) te = (Amalgame_Compiler_TypeError*)AmalgameList_get(tc->Errors, ti);
        if (!code_string_equals(te->Filename, path)) {
            continue;
        }
        if (!first) {
            arr = code_string_concat(arr, ",");
        }
        arr = code_string_concat(arr, Amalgame_Compiler_LspServer_DiagnosticFromTc(source, te));
        first = 0;
    }
    arr = code_string_concat(arr, "]");
    return arr;
}

static void Amalgame_Compiler_LspServer_HandleHover(Amalgame_Compiler_LspServer* self, i64 id, code_string uri, i64 line, i64 character) {
    (void)self;
    (void)id;
    (void)uri;
    (void)line;
    (void)character;
    code_string __attribute__((unused)) source = Amalgame_Compiler_LspServer_LookupDoc(self, uri);
    if (String_Length(source) == 0) {
        Amalgame_Compiler_LspServer_SendNullResult(self, id);
        return;
    }
    code_string __attribute__((unused)) path = Amalgame_Compiler_LspServer_UriToPath(uri);
    Amalgame_Compiler_Lexer* __attribute__((unused)) lex = Amalgame_Compiler_Lexer_new(source, path);
    AmalgameList* __attribute__((unused)) toks = Amalgame_Compiler_Lexer_Tokenize(lex);
    Amalgame_Compiler_Parser* __attribute__((unused)) par = Amalgame_Compiler_Parser_new(toks);
    Amalgame_Compiler_AstNode* __attribute__((unused)) prog = Amalgame_Compiler_Parser_Parse(par);
    prog->Str2 = path;
    Amalgame_Compiler_FullResolver* __attribute__((unused)) resolver = Amalgame_Compiler_LspServer_BuildWorkspaceResolver(self, path, prog);
    Amalgame_Compiler_TypeChecker* __attribute__((unused)) tc = Amalgame_Compiler_TypeChecker_new(resolver, path);
    Amalgame_Compiler_TypeChecker_Check(tc, prog);
    i64 __attribute__((unused)) targetLine = line + 1;
    i64 __attribute__((unused)) targetCol = character + 1;
    Amalgame_Compiler_AstNode* __attribute__((unused)) node = Amalgame_Compiler_LspServer_FindNodeAtPosition(prog, targetLine, targetCol);
    if (node == NULL) {
        Amalgame_Compiler_LspServer_SendNullResult(self, id);
        return;
    }
    code_string __attribute__((unused)) typeStr = Amalgame_Compiler_TypeChecker_LookupNodeType(tc, node);
    if (code_string_equals(typeStr, "?") || String_Length(typeStr) == 0) {
        Amalgame_Compiler_LspServer_SendNullResult(self, id);
        return;
    }
    code_string __attribute__((unused)) content = code_string_concat(code_string_concat(code_string_concat(code_string_concat("**", node->Name), "**: `"), typeStr), "`");
    code_string __attribute__((unused)) body = code_string_concat(code_string_concat(code_string_concat(code_string_concat("{\"jsonrpc\":\"2.0\",\"id\":", String_FromInt(id)), ",\"result\":{\"contents\":{\"kind\":\"markdown\",\"value\":\""), Amalgame_Compiler_LspServer_EscapeJsonStr(content)), "\"}}}");
    Amalgame_Compiler_LspServer_Send(self, body);
}

static void Amalgame_Compiler_LspServer_SendNullResult(Amalgame_Compiler_LspServer* self, i64 id) {
    (void)self;
    (void)id;
    code_string __attribute__((unused)) body = code_string_concat(code_string_concat("{\"jsonrpc\":\"2.0\",\"id\":", String_FromInt(id)), ",\"result\":null}");
    Amalgame_Compiler_LspServer_Send(self, body);
}

static void Amalgame_Compiler_LspServer_HandleCompletion(Amalgame_Compiler_LspServer* self, i64 id, code_string uri) {
    (void)self;
    (void)id;
    (void)uri;
    code_string __attribute__((unused)) source = Amalgame_Compiler_LspServer_LookupDoc(self, uri);
    if (String_Length(source) == 0) {
        Amalgame_Compiler_LspServer_SendEmptyCompletion(self, id);
        return;
    }
    code_string __attribute__((unused)) path = Amalgame_Compiler_LspServer_UriToPath(uri);
    Amalgame_Compiler_Lexer* __attribute__((unused)) lex = Amalgame_Compiler_Lexer_new(source, path);
    AmalgameList* __attribute__((unused)) toks = Amalgame_Compiler_Lexer_Tokenize(lex);
    Amalgame_Compiler_Parser* __attribute__((unused)) par = Amalgame_Compiler_Parser_new(toks);
    Amalgame_Compiler_AstNode* __attribute__((unused)) prog = Amalgame_Compiler_Parser_Parse(par);
    prog->Str2 = path;
    Amalgame_Compiler_FullResolver* __attribute__((unused)) resolver = Amalgame_Compiler_LspServer_BuildWorkspaceResolver(self, path, prog);
    code_string __attribute__((unused)) items = "";
    code_bool __attribute__((unused)) first = 1;
    i64 __attribute__((unused)) gn = Amalgame_Compiler_FullResolver_GlobalCount(resolver);
    for (i64 gi = 0; gi < gn; gi++) {
        code_string __attribute__((unused)) name = Amalgame_Compiler_FullResolver_GlobalNameAt(resolver, gi);
        if (String_Length(name) == 0) {
            continue;
        }
        code_string __attribute__((unused)) typeS = Amalgame_Compiler_FullResolver_GlobalTypeAt(resolver, gi);
        i64 __attribute__((unused)) kind = 6;
        if (code_string_equals(typeS, "type")) {
            kind = 7;
        } else if (code_string_equals(typeS, "void")) {
            kind = 3;
        }
        if (!first) {
            items = code_string_concat(items, ",");
        }
        items = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(items, "{\"label\":\""), Amalgame_Compiler_LspServer_EscapeJsonStr(name)), "\",\"kind\":"), String_FromInt(kind)), ",\"detail\":\""), Amalgame_Compiler_LspServer_EscapeJsonStr(typeS)), "\"}");
        first = 0;
    }
    code_string __attribute__((unused)) body = code_string_concat(code_string_concat(code_string_concat(code_string_concat("{\"jsonrpc\":\"2.0\",\"id\":", String_FromInt(id)), ",\"result\":{\"isIncomplete\":false,\"items\":["), items), "]}}");
    Amalgame_Compiler_LspServer_Send(self, body);
}

static void Amalgame_Compiler_LspServer_SendEmptyCompletion(Amalgame_Compiler_LspServer* self, i64 id) {
    (void)self;
    (void)id;
    code_string __attribute__((unused)) body = code_string_concat(code_string_concat("{\"jsonrpc\":\"2.0\",\"id\":", String_FromInt(id)), ",\"result\":{\"isIncomplete\":false,\"items\":[]}}");
    Amalgame_Compiler_LspServer_Send(self, body);
}

code_string Amalgame_Compiler_LspServer_DiagnosticFromResolver(code_string source, Amalgame_Compiler_ResolverError* e) {
    (void)source;
    (void)e;
    return Amalgame_Compiler_LspServer_DiagnosticBody(source, e->Line, e->Column, e->Message);
}

code_string Amalgame_Compiler_LspServer_DiagnosticFromTc(code_string source, Amalgame_Compiler_TypeError* e) {
    (void)source;
    (void)e;
    return Amalgame_Compiler_LspServer_DiagnosticBody(source, e->Line, e->Column, e->Message);
}

static code_string Amalgame_Compiler_LspServer_DiagnosticBody(code_string source, i64 line, i64 col, code_string msg) {
    (void)source;
    (void)line;
    (void)col;
    (void)msg;
    i64 __attribute__((unused)) l = line - 1;
    i64 __attribute__((unused)) cStart = col - 1;
    if (l < 0) {
        l = 0;
    }
    if (cStart < 0) {
        cStart = 0;
    }
    i64 __attribute__((unused)) endCol = Amalgame_Compiler_LspServer_TokenEndCol(source, line, col);
    i64 __attribute__((unused)) cEnd = endCol - 1;
    if (cEnd <= cStart) {
        cEnd = cStart + 1;
    }
    code_string __attribute__((unused)) lStr = String_FromInt(l);
    code_string __attribute__((unused)) cStartStr = String_FromInt(cStart);
    code_string __attribute__((unused)) cEndStr = String_FromInt(cEnd);
    return code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("{\"severity\":1,\"range\":{\"start\":{\"line\":", lStr), ",\"character\":"), cStartStr), "},\"end\":{\"line\":"), lStr), ",\"character\":"), cEndStr), "}},\"message\":\""), Amalgame_Compiler_LspServer_EscapeJsonStr(msg)), "\"}");
}

static i64 Amalgame_Compiler_LspServer_TokenEndCol(code_string source, i64 line, i64 col) {
    (void)source;
    (void)line;
    (void)col;
    i64 __attribute__((unused)) n = String_Length(source);
    i64 __attribute__((unused)) off = 0;
    i64 __attribute__((unused)) ln = 1;
    while (ln < line && off < n) {
        code_string __attribute__((unused)) ch = String_CharAt1(source, off);
        if (code_string_equals(ch, "\n")) {
            ln = ln + 1;
        }
        off = off + 1;
    }
    i64 __attribute__((unused)) colIdx = 0;
    while (colIdx < col - 1 && off < n) {
        code_string __attribute__((unused)) ch = String_CharAt1(source, off);
        if (code_string_equals(ch, "\n")) {
            break;
        }
        off = off + 1;
        colIdx = colIdx + 1;
    }
    i64 __attribute__((unused)) endCol = col;
    while (off < n) {
        code_string __attribute__((unused)) ch = String_CharAt1(source, off);
        if (Amalgame_Compiler_LspServer_IsWordChar(ch)) {
            off = off + 1;
            endCol = endCol + 1;
        } else {
            break;
        }
    }
    return endCol;
}

static code_bool Amalgame_Compiler_LspServer_IsWordChar(code_string ch) {
    (void)ch;
    if (String_Length(ch) == 0) {
        return 0;
    }
    if (code_string_equals(ch, "_")) {
        return 1;
    }
    if (String_IndexOf("0123456789", ch) >= 0) {
        return 1;
    }
    if (String_IndexOf("abcdefghijklmnopqrstuvwxyz", ch) >= 0) {
        return 1;
    }
    if (String_IndexOf("ABCDEFGHIJKLMNOPQRSTUVWXYZ", ch) >= 0) {
        return 1;
    }
    return 0;
}

code_string Amalgame_Compiler_LspServer_EscapeJsonStr(code_string s) {
    (void)s;
    i64 __attribute__((unused)) n = String_Length(s);
    code_string __attribute__((unused)) cr = Amalgame_Compiler_LspServer_Cr();
    code_string __attribute__((unused)) r = "";
    for (i64 i = 0; i < n; i++) {
        code_string __attribute__((unused)) ch = String_CharAt1(s, i);
        if (code_string_equals(ch, "\"")) {
            r = code_string_concat(r, "\\\"");
        } else if (code_string_equals(ch, "\\")) {
            r = code_string_concat(r, "\\\\");
        } else if (code_string_equals(ch, "\n")) {
            r = code_string_concat(r, "\\n");
        } else if (code_string_equals(ch, cr)) {
            r = code_string_concat(r, "\\r");
        } else if (code_string_equals(ch, "\t")) {
            r = code_string_concat(r, "\\t");
        } else {
            r = code_string_concat(r, ch);
        }
    }
    return r;
}

code_string Amalgame_Compiler_LspServer_UriToPath(code_string uri) {
    (void)uri;
    if (String_StartsWith(uri, "file://")) {
        i64 __attribute__((unused)) n = String_Length(uri);
        return String_Substring(uri, 7, n - 7);
    }
    return uri;
}

Amalgame_Compiler_AstNode* Amalgame_Compiler_LspServer_FindNodeAtPosition(Amalgame_Compiler_AstNode* root, i64 line, i64 col) {
    (void)root;
    (void)line;
    (void)col;
    if (root == NULL) {
        return NULL;
    }
    Amalgame_Compiler_AstNode* __attribute__((unused)) best = NULL;
    best = Amalgame_Compiler_LspServer_FindNodeWalk(root, line, col, best);
    return best;
}

static Amalgame_Compiler_AstNode* Amalgame_Compiler_LspServer_FindNodeWalk(Amalgame_Compiler_AstNode* node, i64 line, i64 col, Amalgame_Compiler_AstNode* best) {
    (void)node;
    (void)line;
    (void)col;
    (void)best;
    Amalgame_Compiler_AstNode* __attribute__((unused)) current = best;
    if (Amalgame_Compiler_LspServer_NodeCovers(node, line, col)) {
        if (String_Length(node->Name) > 0) {
            current = node;
        } else if (current == NULL) {
            current = node;
        }
    }
    if (node->Left != NULL) {
        current = Amalgame_Compiler_LspServer_FindNodeWalk(node->Left, line, col, current);
    }
    if (node->Right != NULL) {
        current = Amalgame_Compiler_LspServer_FindNodeWalk(node->Right, line, col, current);
    }
    if (node->Cond != NULL) {
        current = Amalgame_Compiler_LspServer_FindNodeWalk(node->Cond, line, col, current);
    }
    if (node->Body != NULL) {
        current = Amalgame_Compiler_LspServer_FindNodeWalk(node->Body, line, col, current);
    }
    if (node->Else != NULL) {
        current = Amalgame_Compiler_LspServer_FindNodeWalk(node->Else, line, col, current);
    }
    i64 __attribute__((unused)) cn = AmalgameList_count(node->Children);
    for (i64 ci = 0; ci < cn; ci++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) c = (Amalgame_Compiler_AstNode*)AmalgameList_get(node->Children, ci);
        current = Amalgame_Compiler_LspServer_FindNodeWalk(c, line, col, current);
    }
    i64 __attribute__((unused)) pn = AmalgameList_count(node->Params);
    for (i64 pi = 0; pi < pn; pi++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) p = (Amalgame_Compiler_AstNode*)AmalgameList_get(node->Params, pi);
        current = Amalgame_Compiler_LspServer_FindNodeWalk(p, line, col, current);
    }
    i64 __attribute__((unused)) an = AmalgameList_count(node->Args);
    for (i64 ai = 0; ai < an; ai++) {
        Amalgame_Compiler_AstNode* __attribute__((unused)) a = (Amalgame_Compiler_AstNode*)AmalgameList_get(node->Args, ai);
        current = Amalgame_Compiler_LspServer_FindNodeWalk(a, line, col, current);
    }
    return current;
}

static code_bool Amalgame_Compiler_LspServer_NodeCovers(Amalgame_Compiler_AstNode* node, i64 line, i64 col) {
    (void)node;
    (void)line;
    (void)col;
    if (node->Line != line) {
        return 0;
    }
    i64 __attribute__((unused)) nameLen = String_Length(node->Name);
    if (nameLen == 0) {
        return 0;
    }
    i64 __attribute__((unused)) endCol = node->Column + nameLen;
    if (col < node->Column) {
        return 0;
    }
    if (col > endCol) {
        return 0;
    }
    return 1;
}

code_string Amalgame_Compiler_LspServer_JsonStr(code_string body, code_string key) {
    (void)body;
    (void)key;
    code_string __attribute__((unused)) needle = code_string_concat(code_string_concat("\"", key), "\"");
    i64 __attribute__((unused)) kIdx = String_IndexOf(body, needle);
    if (kIdx < 0) {
        return "";
    }
    i64 __attribute__((unused)) bn = String_Length(body);
    i64 __attribute__((unused)) needleLen = String_Length(needle);
    i64 __attribute__((unused)) i = kIdx + needleLen;
    while (i < bn) {
        code_string __attribute__((unused)) ch = String_CharAt1(body, i);
        if (code_string_equals(ch, "\"")) {
            i = i + 1;
            break;
        }
        i = i + 1;
    }
    code_string __attribute__((unused)) result = "";
    while (i < bn) {
        code_string __attribute__((unused)) ch = String_CharAt1(body, i);
        if (code_string_equals(ch, "\"")) {
            break;
        }
        if (code_string_equals(ch, "\\")) {
            if (i + 1 >= bn) {
                break;
            }
            code_string __attribute__((unused)) nx = String_CharAt1(body, i + 1);
            if (code_string_equals(nx, "n")) {
                result = code_string_concat(result, "\n");
            } else if (code_string_equals(nx, "r")) {
                result = code_string_concat(result, "\\r");
            } else if (code_string_equals(nx, "t")) {
                result = code_string_concat(result, "\t");
            } else if (code_string_equals(nx, "\"")) {
                result = code_string_concat(result, "\"");
            } else if (code_string_equals(nx, "\\")) {
                result = code_string_concat(result, "\\");
            } else if (code_string_equals(nx, "/")) {
                result = code_string_concat(result, "/");
            } else {
                result = code_string_concat(result, nx);
            }
            i = i + 2;
        } else {
            result = code_string_concat(result, ch);
            i = i + 1;
        }
    }
    return result;
}

i64 Amalgame_Compiler_LspServer_JsonInt(code_string body, code_string key) {
    (void)body;
    (void)key;
    code_string __attribute__((unused)) needle = code_string_concat(code_string_concat("\"", key), "\"");
    i64 __attribute__((unused)) kIdx = String_IndexOf(body, needle);
    if (kIdx < 0) {
        return 0;
    }
    i64 __attribute__((unused)) bn = String_Length(body);
    code_string __attribute__((unused)) dig = "0123456789";
    i64 __attribute__((unused)) needleLen = String_Length(needle);
    i64 __attribute__((unused)) i = kIdx + needleLen;
    while (i < bn) {
        code_string __attribute__((unused)) ch = String_CharAt1(body, i);
        if (code_string_equals(ch, "-")) {
            break;
        }
        if (String_IndexOf(dig, ch) >= 0) {
            break;
        }
        i = i + 1;
    }
    code_string __attribute__((unused)) numStr = "";
    while (i < bn) {
        code_string __attribute__((unused)) ch = String_CharAt1(body, i);
        if (code_string_equals(ch, "-") || String_IndexOf(dig, ch) >= 0) {
            numStr = code_string_concat(numStr, ch);
            i = i + 1;
        } else {
            break;
        }
    }
    return String_ToInt(numStr);
}

struct _Amalgame_Compiler_MigrateResult {
    code_bool Ok;
    code_string Content;
    code_string Error;
};


Amalgame_Compiler_MigrateResult* Amalgame_Compiler_MigrateResult_new() {
    Amalgame_Compiler_MigrateResult* self = (Amalgame_Compiler_MigrateResult*) GC_MALLOC(sizeof(Amalgame_Compiler_MigrateResult));
    self->Ok = 0;
    self->Content = "";
    self->Error = "";
    return self;
}

struct _Amalgame_Compiler_MigrateCommand {
};

void Amalgame_Compiler_MigrateCommand_PrintUsage();
i64 Amalgame_Compiler_MigrateCommand_Run(i64 argc);
static i64 Amalgame_Compiler_MigrateCommand_RunMigrateOne(code_string input, code_string output, code_string langHint, code_string provider, code_string model, code_bool force, code_bool dryRun, code_bool promptOnly, code_bool noCheck, i64 maxLines, code_bool noCache);
static i64 Amalgame_Compiler_MigrateCommand_RunMigrateDirectory(code_string dir, code_string langHint, code_string provider, code_string model, code_bool force, code_bool dryRun, code_bool promptOnly, code_bool noCheck, i64 maxLines, code_bool noCache);
static code_bool Amalgame_Compiler_MigrateCommand_IsDirectory(code_string path);
static code_string Amalgame_Compiler_MigrateCommand_DetectLanguage(code_string path);
static code_string Amalgame_Compiler_MigrateCommand_DefaultOutputPath(code_string input);
static i64 Amalgame_Compiler_MigrateCommand_CountLines(code_string s);
static code_string Amalgame_Compiler_MigrateCommand_BuildPrompt(code_string lang, code_string source);
static code_string Amalgame_Compiler_MigrateCommand_BuildSystemPrompt(code_string lang);
static code_string Amalgame_Compiler_MigrateCommand_BuildUserPrompt(code_string lang, code_string source);
code_string Amalgame_Compiler_MigrateCommand_LoadDocsHeader();
static Amalgame_Compiler_MigrateResult* Amalgame_Compiler_MigrateCommand_CallProvider(code_string provider, code_string model, code_string lang, code_string source);
Amalgame_Compiler_MigrateResult* Amalgame_Compiler_MigrateCommand_CallProviderRaw(code_string provider, code_string model, code_string systemPrompt, code_string userPrompt);
code_string Amalgame_Compiler_MigrateCommand_AutoSelectProvider();
code_string Amalgame_Compiler_MigrateCommand_EstimateCost(code_string provider, code_string model, code_string systemPrompt, code_string userPrompt);
static Amalgame_Compiler_MigrateResult* Amalgame_Compiler_MigrateCommand_CallClaudeApiRaw(code_string model, code_string systemPrompt, code_string userPrompt);
static code_string Amalgame_Compiler_MigrateCommand_JsonEscape(code_string s);
static code_string Amalgame_Compiler_MigrateCommand_JsonExtractText(code_string body);
static code_string Amalgame_Compiler_MigrateCommand_JsonExtract(code_string body, code_string key);
static Amalgame_Compiler_MigrateResult* Amalgame_Compiler_MigrateCommand_CallChatGptApi(code_string model, code_string systemPrompt, code_string userPrompt);
static Amalgame_Compiler_MigrateResult* Amalgame_Compiler_MigrateCommand_CallGeminiApi(code_string model, code_string systemPrompt, code_string userPrompt);
static Amalgame_Compiler_MigrateResult* Amalgame_Compiler_MigrateCommand_CallCustomScript(code_string systemPrompt, code_string userPrompt);
static Amalgame_Compiler_MigrateResult* Amalgame_Compiler_MigrateCommand_CallClaudeCli(code_string model, code_string prompt);
static code_bool Amalgame_Compiler_MigrateCommand_IsCommandAvailable(code_string cmd);
code_string Amalgame_Compiler_MigrateCommand_CacheHash(code_string source, code_string systemPrompt);
code_string Amalgame_Compiler_MigrateCommand_CachePath(code_string hash);
code_string Amalgame_Compiler_MigrateCommand_CacheLookup(code_string source, code_string systemPrompt);
void Amalgame_Compiler_MigrateCommand_CacheStore(code_string source, code_string systemPrompt, code_string content);
i64 Amalgame_Compiler_MigrateCommand_StreamClaudeCli(code_string model, code_string prompt);
static code_string Amalgame_Compiler_MigrateCommand_StripFences(code_string s);

Amalgame_Compiler_MigrateCommand* Amalgame_Compiler_MigrateCommand_new() {
    Amalgame_Compiler_MigrateCommand* self = (Amalgame_Compiler_MigrateCommand*) GC_MALLOC(sizeof(Amalgame_Compiler_MigrateCommand));
    return self;
}

void Amalgame_Compiler_MigrateCommand_PrintUsage() {
    Console_WriteError("Usage: amc migrate <file> [flags]");
    Console_WriteError("");
    Console_WriteError("Translates a source file in any supported language to Amalgame");
    Console_WriteError("via an LLM (v0: requires `claude` CLI on PATH).");
    Console_WriteError("");
    Console_WriteError("Supported source extensions:");
    Console_WriteError("  .ts .tsx .js .jsx .mjs .py .java .cs .go .rs");
    Console_WriteError("  .cpp .cc .cxx .hpp .h++ .c .h .kt .kts .swift .rb .php");
    Console_WriteError("");
    Console_WriteError("Flags:");
    Console_WriteError("  -o, --output <out>   Output path (default: <stem>.am next to source).");
    Console_WriteError("  --lang <name>        Override extension-based language detection.");
    Console_WriteError("  --provider <name>    LLM provider. Built-in: claude (CLI), claude-api");
    Console_WriteError("                       (Anthropic), chatgpt (OpenAI), gemini (Google),");
    Console_WriteError("                       custom (delegates to AMC_CUSTOM_PROVIDER_CMD).");
    Console_WriteError("                       Auto-selects API by env var: ANTHROPIC_API_KEY →");
    Console_WriteError("                       claude-api, OPENAI_API_KEY → chatgpt,");
    Console_WriteError("                       GEMINI_API_KEY → gemini, otherwise → claude (CLI).");
    Console_WriteError("  --model <id>         Pass a specific model id to the provider.");
    Console_WriteError("  --max-lines <n>      Refuse files larger than n lines (default: 2000).");
    Console_WriteError("  --no-check           Skip the post-migration `amc --check` validation.");
    Console_WriteError("  --no-cache           Skip the on-disk result cache (always re-call LLM).");
    Console_WriteError("  --force              Overwrite an existing .am at the output path.");
    Console_WriteError("  --dry-run            Print what would happen without invoking the LLM.");
    Console_WriteError("  --prompt-only        Dump the assembled prompt to stdout and exit.");
    Console_WriteError("  -h, --help           Print this help and exit.");
    Console_WriteError("");
    Console_WriteError("See docs/proposals/amc-migrate.md for design rationale.");
}

i64 Amalgame_Compiler_MigrateCommand_Run(i64 argc) {
    (void)argc;
    code_string __attribute__((unused)) input = "";
    code_string __attribute__((unused)) output = "";
    code_string __attribute__((unused)) langHint = "";
    code_string __attribute__((unused)) provider = "";
    code_bool __attribute__((unused)) providerSet = 0;
    code_string __attribute__((unused)) model = "";
    code_bool __attribute__((unused)) dryRun = 0;
    code_bool __attribute__((unused)) promptOnly = 0;
    code_bool __attribute__((unused)) noCheck = 0;
    code_bool __attribute__((unused)) force = 0;
    i64 __attribute__((unused)) maxLines = 2000;
    code_bool __attribute__((unused)) noCache = 0;
    i64 __attribute__((unused)) i = 2;
    while (i < argc) {
        code_string __attribute__((unused)) a = Args_Get(i);
        if (code_string_equals(a, "-h") || code_string_equals(a, "--help")) {
            Amalgame_Compiler_MigrateCommand_PrintUsage();
            return 0;
        } else if (code_string_equals(a, "-o") || code_string_equals(a, "--output")) {
            i = i + 1;
            if (i >= argc) {
                Console_WriteError(code_string_concat(code_string_concat("amc migrate: ", a), " requires a value"));
                return 1;
            }
            output = Args_Get(i);
        } else if (code_string_equals(a, "--dry-run")) {
            dryRun = 1;
        } else if (code_string_equals(a, "--prompt-only")) {
            promptOnly = 1;
        } else if (code_string_equals(a, "--no-check")) {
            noCheck = 1;
        } else if (code_string_equals(a, "--no-cache")) {
            noCache = 1;
        } else if (code_string_equals(a, "--force")) {
            force = 1;
        } else if (code_string_equals(a, "--lang")) {
            i = i + 1;
            if (i >= argc) {
                Console_WriteError("amc migrate: --lang requires a value");
                return 1;
            }
            langHint = Args_Get(i);
        } else if (code_string_equals(a, "--provider")) {
            i = i + 1;
            if (i >= argc) {
                Console_WriteError("amc migrate: --provider requires a value");
                return 1;
            }
            provider = Args_Get(i);
            providerSet = 1;
        } else if (code_string_equals(a, "--model")) {
            i = i + 1;
            if (i >= argc) {
                Console_WriteError("amc migrate: --model requires a value");
                return 1;
            }
            model = Args_Get(i);
        } else if (code_string_equals(a, "--max-lines")) {
            i = i + 1;
            if (i >= argc) {
                Console_WriteError("amc migrate: --max-lines requires a value");
                return 1;
            }
            maxLines = String_ToInt(Args_Get(i));
            if (maxLines <= 0) {
                Console_WriteError("amc migrate: --max-lines must be > 0");
                return 1;
            }
        } else if (String_StartsWith(a, "-")) {
            Console_WriteError(code_string_concat(code_string_concat("amc migrate: unknown option '", a), "'"));
            return 1;
        } else {
            if (String_Length(input) > 0) {
                Console_WriteError(code_string_concat(code_string_concat(code_string_concat(code_string_concat("amc migrate: too many positional arguments (got '", a), "', already had '"), input), "')"));
                return 1;
            }
            input = a;
        }
        i = i + 1;
    }
    if (String_Length(input) == 0) {
        Console_WriteError("amc migrate: no input file");
        Console_WriteError("usage: amc migrate <file> [--output <out>] [--lang <hint>] [--provider <name>] [--model <id>]");
        Console_WriteError("                         [--dry-run] [--prompt-only] [--no-check] [--force] [--max-lines <n>]");
        return 1;
    }
    if (!providerSet) {
        provider = Amalgame_Compiler_MigrateCommand_AutoSelectProvider();
    }
    if (!File_Exists(input)) {
        Console_WriteError(code_string_concat("amc migrate: path not found: ", input));
        return 1;
    }
    if (Amalgame_Compiler_MigrateCommand_IsDirectory(input)) {
        if (String_Length(output) > 0) {
            Console_WriteError("amc migrate: --output cannot be used with directory input (writes are in-place next to each source).");
            return 1;
        }
        return Amalgame_Compiler_MigrateCommand_RunMigrateDirectory(input, langHint, provider, model, force, dryRun, promptOnly, noCheck, maxLines, noCache);
    }
    return Amalgame_Compiler_MigrateCommand_RunMigrateOne(input, output, langHint, provider, model, force, dryRun, promptOnly, noCheck, maxLines, noCache);
}

static i64 Amalgame_Compiler_MigrateCommand_RunMigrateOne(code_string input, code_string output, code_string langHint, code_string provider, code_string model, code_bool force, code_bool dryRun, code_bool promptOnly, code_bool noCheck, i64 maxLines, code_bool noCache) {
    (void)input;
    (void)output;
    (void)langHint;
    (void)provider;
    (void)model;
    (void)force;
    (void)dryRun;
    (void)promptOnly;
    (void)noCheck;
    (void)maxLines;
    (void)noCache;
    code_string __attribute__((unused)) lang = langHint;
    if (String_Length(lang) == 0) {
        lang = Amalgame_Compiler_MigrateCommand_DetectLanguage(input);
    }
    if (String_Length(lang) == 0) {
        Console_WriteError(code_string_concat(code_string_concat("amc migrate: unrecognized source extension for '", input), "'"));
        Console_WriteError("Supported extensions: .ts .tsx .js .jsx .mjs .py .java .cs .go .rs .cpp .cc .cxx .hpp .h++ .c .h .kt .kts .swift .rb .php");
        Console_WriteError("Or pass --lang <hint> with a language name.");
        return 1;
    }
    code_string __attribute__((unused)) source = File_ReadAll(input);
    i64 __attribute__((unused)) lineCount = Amalgame_Compiler_MigrateCommand_CountLines(source);
    if (lineCount > maxLines) {
        Console_WriteError(code_string_concat(code_string_concat(code_string_concat(code_string_concat("amc migrate: source exceeds ", String_FromInt(maxLines)), " lines (got "), String_FromInt(lineCount)), ")"));
        Console_WriteError("Suggestion: split the file or override with --max-lines <n>.");
        return 1;
    }
    if (promptOnly) {
        Console_WriteLine(Amalgame_Compiler_MigrateCommand_BuildPrompt(lang, source));
        return 0;
    }
    code_string __attribute__((unused)) outPath = output;
    if (String_Length(outPath) == 0) {
        outPath = Amalgame_Compiler_MigrateCommand_DefaultOutputPath(input);
    }
    if (File_Exists(outPath) && !force) {
        Console_WriteError(code_string_concat("amc migrate: output exists: ", outPath));
        Console_WriteError("Pass --force to overwrite.");
        return 1;
    }
    if (dryRun) {
        code_string __attribute__((unused)) sysP = Amalgame_Compiler_MigrateCommand_BuildSystemPrompt(lang);
        code_string __attribute__((unused)) usrP = Amalgame_Compiler_MigrateCommand_BuildUserPrompt(lang, source);
        code_string __attribute__((unused)) est = Amalgame_Compiler_MigrateCommand_EstimateCost(provider, model, sysP, usrP);
        Console_WriteLine(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("[migrate] would migrate: ", input), " ("), lang), ", "), String_FromInt(lineCount)), " lines)"));
        Console_WriteLine(code_string_concat("[migrate] would write:   ", outPath));
        Console_WriteLine(code_string_concat("[migrate] provider:      ", provider));
        if (String_Length(model) > 0) {
            Console_WriteLine(code_string_concat("[migrate] model:         ", model));
        }
        Console_WriteLine(code_string_concat("[migrate] estimated cost: ", est));
        return 0;
    }
    code_string __attribute__((unused)) sysForCache = Amalgame_Compiler_MigrateCommand_BuildSystemPrompt(lang);
    if (!noCache) {
        code_string __attribute__((unused)) cached = Amalgame_Compiler_MigrateCommand_CacheLookup(source, sysForCache);
        if (String_Length(cached) > 0) {
            code_bool __attribute__((unused)) writeOkC = File_WriteAll(outPath, cached);
            if (!writeOkC) {
                Console_WriteError(code_string_concat("amc migrate: failed to write ", outPath));
                return 1;
            }
            Console_WriteLine(code_string_concat(code_string_concat("[migrate] wrote ", outPath), " (cache hit)"));
            if (!noCheck) {
                code_string __attribute__((unused)) amcPathC = Args_Get(0);
                code_string __attribute__((unused)) cmdC = code_string_concat(code_string_concat(amcPathC, " --check "), outPath);
                AmalgameProcessResult* __attribute__((unused)) checkC = Process_RunCapture(cmdC);
                if (checkC->Exit != 0) {
                    Console_WriteError("[migrate] check failed:");
                    Console_WriteError(checkC->Stdout);
                    return 1;
                }
                Console_WriteLine("[migrate] check passed");
            }
            return 0;
        }
    }
    Console_WriteError(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("[migrate] processing ", input), " ("), lang), ", "), String_FromInt(lineCount)), " lines, provider="), provider), ")..."));
    Amalgame_Compiler_MigrateResult* __attribute__((unused)) result = Amalgame_Compiler_MigrateCommand_CallProvider(provider, model, lang, source);
    if (!result->Ok) {
        Console_WriteError(code_string_concat("amc migrate: ", result->Error));
        return 1;
    }
    code_bool __attribute__((unused)) writeOk = File_WriteAll(outPath, result->Content);
    if (!writeOk) {
        Console_WriteError(code_string_concat("amc migrate: failed to write ", outPath));
        return 1;
    }
    Console_WriteLine(code_string_concat("[migrate] wrote ", outPath));
    if (!noCache) {
        Amalgame_Compiler_MigrateCommand_CacheStore(source, sysForCache, result->Content);
    }
    if (!noCheck) {
        code_string __attribute__((unused)) amcPath = Args_Get(0);
        code_string __attribute__((unused)) cmd = code_string_concat(code_string_concat(amcPath, " --check "), outPath);
        AmalgameProcessResult* __attribute__((unused)) check = Process_RunCapture(cmd);
        if (check->Exit != 0) {
            Console_WriteError("[migrate] check failed (typechecker errors in the migrated file):");
            Console_WriteError(check->Stdout);
            Console_WriteError("The .am file was still written so you can inspect / fix manually.");
            return 1;
        }
        Console_WriteLine("[migrate] check passed");
    }
    return 0;
}

static i64 Amalgame_Compiler_MigrateCommand_RunMigrateDirectory(code_string dir, code_string langHint, code_string provider, code_string model, code_bool force, code_bool dryRun, code_bool promptOnly, code_bool noCheck, i64 maxLines, code_bool noCache) {
    (void)dir;
    (void)langHint;
    (void)provider;
    (void)model;
    (void)force;
    (void)dryRun;
    (void)promptOnly;
    (void)noCheck;
    (void)maxLines;
    (void)noCache;
    Console_WriteError(code_string_concat(code_string_concat("[migrate] discovering source files in ", dir), "..."));
    code_string __attribute__((unused)) findCmd = code_string_concat(code_string_concat("find ", dir), " -type f 2>/dev/null");
    AmalgameProcessResult* __attribute__((unused)) result = Process_RunCapture(findCmd);
    if (result->Exit != 0) {
        Console_WriteError(code_string_concat("amc migrate: failed to enumerate ", dir));
        return 1;
    }
    AmalgameList* __attribute__((unused)) lines = String_Split(String_Trim(result->Stdout), "\n");
    i64 __attribute__((unused)) n = AmalgameList_count(lines);
    AmalgameList* __attribute__((unused)) candidates = AmalgameList_new();
    AmalgameList* __attribute__((unused)) langs = AmalgameList_new();
    for (i64 i = 0; i < n; i++) {
        code_string __attribute__((unused)) path = String_Trim((code_string)AmalgameList_get(lines, i));
        if (String_Length(path) == 0) {
            continue;
        }
        if (String_EndsWith(path, ".am")) {
            continue;
        }
        code_string __attribute__((unused)) lang = Amalgame_Compiler_MigrateCommand_DetectLanguage(path);
        if (String_Length(lang) == 0) {
            continue;
        }
        AmalgameList_add(candidates, (void*)(intptr_t)(path));
        AmalgameList_add(langs, (void*)(intptr_t)(lang));
    }
    i64 __attribute__((unused)) total = AmalgameList_count(candidates);
    if (total == 0) {
        Console_WriteError(code_string_concat("[migrate] no recognized source files found in ", dir));
        return 0;
    }
    Console_WriteError(code_string_concat(code_string_concat("[migrate] found ", String_FromInt(total)), " file(s) to migrate"));
    i64 __attribute__((unused)) ok = 0;
    i64 __attribute__((unused)) failed = 0;
    for (i64 j = 0; j < total; j++) {
        code_string __attribute__((unused)) path = (code_string)AmalgameList_get(candidates, j);
        code_string __attribute__((unused)) perFileLang = langHint;
        if (String_Length(perFileLang) == 0) {
            perFileLang = (code_string)AmalgameList_get(langs, j);
        }
        i64 __attribute__((unused)) r = Amalgame_Compiler_MigrateCommand_RunMigrateOne(path, "", perFileLang, provider, model, force, dryRun, promptOnly, noCheck, maxLines, noCache);
        if (r == 0) {
            ok = ok + 1;
        } else {
            failed = failed + 1;
        }
    }
    Console_WriteLine("");
    Console_WriteLine(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("[migrate] summary: ", String_FromInt(ok)), "/"), String_FromInt(total)), " succeeded, "), String_FromInt(failed)), " failed"));
    if (failed > 0) {
        return 1;
    }
    return 0;
}

static code_bool Amalgame_Compiler_MigrateCommand_IsDirectory(code_string path) {
    (void)path;
    code_string __attribute__((unused)) cmd = code_string_concat(code_string_concat("[ -d \"", path), "\" ] && echo y || echo n");
    AmalgameProcessResult* __attribute__((unused)) res = Process_RunCapture(cmd);
    code_string __attribute__((unused)) answer = String_Trim(res->Stdout);
    return code_string_equals(answer, "y");
}

static code_string Amalgame_Compiler_MigrateCommand_DetectLanguage(code_string path) {
    (void)path;
    if (String_EndsWith(path, ".ts")) {
        return "TypeScript";
    }
    if (String_EndsWith(path, ".tsx")) {
        return "TypeScript";
    }
    if (String_EndsWith(path, ".js")) {
        return "JavaScript";
    }
    if (String_EndsWith(path, ".jsx")) {
        return "JavaScript";
    }
    if (String_EndsWith(path, ".mjs")) {
        return "JavaScript";
    }
    if (String_EndsWith(path, ".py")) {
        return "Python";
    }
    if (String_EndsWith(path, ".java")) {
        return "Java";
    }
    if (String_EndsWith(path, ".cs")) {
        return "C#";
    }
    if (String_EndsWith(path, ".go")) {
        return "Go";
    }
    if (String_EndsWith(path, ".rs")) {
        return "Rust";
    }
    if (String_EndsWith(path, ".cpp")) {
        return "C++";
    }
    if (String_EndsWith(path, ".cc")) {
        return "C++";
    }
    if (String_EndsWith(path, ".cxx")) {
        return "C++";
    }
    if (String_EndsWith(path, ".hpp")) {
        return "C++";
    }
    if (String_EndsWith(path, ".h++")) {
        return "C++";
    }
    if (String_EndsWith(path, ".c")) {
        return "C";
    }
    if (String_EndsWith(path, ".h")) {
        return "C";
    }
    if (String_EndsWith(path, ".kt")) {
        return "Kotlin";
    }
    if (String_EndsWith(path, ".kts")) {
        return "Kotlin";
    }
    if (String_EndsWith(path, ".swift")) {
        return "Swift";
    }
    if (String_EndsWith(path, ".rb")) {
        return "Ruby";
    }
    if (String_EndsWith(path, ".php")) {
        return "PHP";
    }
    return "";
}

static code_string Amalgame_Compiler_MigrateCommand_DefaultOutputPath(code_string input) {
    (void)input;
    i64 __attribute__((unused)) lastDot = String_LastIndexOf(input, ".");
    i64 __attribute__((unused)) lastSlash = String_LastIndexOf(input, "/");
    if (lastDot <= lastSlash || lastDot <= 0) {
        return code_string_concat(input, ".am");
    }
    return code_string_concat(String_Substring(input, 0, lastDot), ".am");
}

static i64 Amalgame_Compiler_MigrateCommand_CountLines(code_string s) {
    (void)s;
    i64 __attribute__((unused)) len = String_Length(s);
    if (len == 0) {
        return 0;
    }
    i64 __attribute__((unused)) n = 1;
    for (i64 i = 0; i < len; i++) {
        code_string __attribute__((unused)) ch = String_CharAt1(s, i);
        if (code_string_equals(ch, "\n")) {
            n = n + 1;
        }
    }
    return n;
}

static code_string Amalgame_Compiler_MigrateCommand_BuildPrompt(code_string lang, code_string source) {
    (void)lang;
    (void)source;
    return code_string_concat(code_string_concat(Amalgame_Compiler_MigrateCommand_BuildSystemPrompt(lang), "\n\n"), Amalgame_Compiler_MigrateCommand_BuildUserPrompt(lang, source));
}

static code_string Amalgame_Compiler_MigrateCommand_BuildSystemPrompt(code_string lang) {
    (void)lang;
    code_string __attribute__((unused)) lb = "{";
    code_string __attribute__((unused)) rb = "}";
    code_string __attribute__((unused)) p = "";
    p = code_string_concat(code_string_concat(code_string_concat(p, "You are translating "), lang), " source code to Amalgame.\n");
    p = code_string_concat(p, "Amalgame is a self-hosted programming language that transpiles to C.\n");
    p = code_string_concat(p, "It uses class-based OOP with explicit visibility modifiers, generic\n");
    p = code_string_concat(p, "collections (List<T>, Map<K,V>, Set<T>), ML-style match expressions,\n");
    p = code_string_concat(p, "and exception-based error handling. Higher-order list operations like\n");
    p = code_string_concat(p, ".Map / .Filter / .Reduce / .Any / .All / .CountIf take lambdas.\n");
    p = code_string_concat(p, "\n");
    p = code_string_concat(p, "## Amalgame conventions\n");
    p = code_string_concat(p, "- Files start with `namespace <Name>` then declarations.\n");
    p = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(p, "- Classes: `public class Name "), lb), " public Field: int = 0; ... "), rb), "`.\n");
    p = code_string_concat(p, "- Data classes (record-like): `public data class User(string Name, int Age)`.\n");
    p = code_string_concat(p, "- Constructors: `let u = new User(\"Alice\", 30)`.\n");
    p = code_string_concat(p, "- Locals: `let x = 1` (immutable), `var y = 2` (mutable).\n");
    p = code_string_concat(p, "- Type annotations are optional but supported: `let n: int = 1`.\n");
    p = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(p, "- Lambdas: `(x, y) => x + y`, or block: `x => "), lb), " let d = x*2; return d+1 "), rb), "`.\n");
    p = code_string_concat(p, "- Higher-order list: `users.Map(u => u.Name)`, `xs.Filter(x => x > 0)`.\n");
    p = code_string_concat(p, "- Generics: `let xs = new List<int>()`, `let m = new Map<string,int>()`.\n");
    p = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(p, "- Match expression: `match x "), lb), " 0 => \"zero\", 1 => \"one\", _ => \"other\" "), rb), "`.\n");
    p = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(p, "- Try/catch: `try "), lb), " ... "), rb), " catch (e) "), lb), " ... "), rb), "`. Throw with `throw <expr>`.\n");
    p = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(p, "- Console output: `Console.WriteLine(\"x="), lb), "x"), rb), "\")` (string interpolation).\n");
    p = code_string_concat(p, "- File I/O: `File.ReadAll(path)`, `File.WriteAll(path, text)`.\n");
    p = code_string_concat(p, "- Process: `Process.RunCapture(cmd)` returns an exit + stdout.\n");
    p = code_string_concat(p, "- HTTP: `Http.Get(url)`, `Http.Post(url, body)` from the runtime.\n");
    p = code_string_concat(p, "- Comments: `//` line, `/* ... */` block.\n");
    p = code_string_concat(p, "- Null-safe: `obj?.Field`, `a ?? b`. Nullable type: `Foo?`.\n");
    p = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(p, "- Enum: `enum Direction "), lb), " North, South, East, West "), rb), "`.\n");
    p = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(p, "- Interface (method-only, no fields): `interface IDrawable "), lb), " void Draw() "), rb), "`.\n");
    p = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(p, "  Implement with `class Square implements IDrawable "), lb), " ... "), rb), "`.\n");
    p = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(p, "- For-in over a collection: `for x in xs "), lb), " ... "), rb), "`. Range: `for i in 0..n`.\n");
    p = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(p, "- Public entry point: `public class Program "), lb), " public static void Main(string[] args) "), lb), " ... "), rb), " "), rb), "`.\n");
    p = code_string_concat(p, "\n");
    p = code_string_concat(p, "## Idiomatic patterns\n");
    p = code_string_concat(p, "- Prefer immutable `let` over `var`. Loops and accumulators are exceptions.\n");
    p = code_string_concat(p, "- For functional pipelines, use the higher-order list methods, not for-in.\n");
    p = code_string_concat(p, "- For value types, use `data class`. For behavior, regular `class`.\n");
    p = code_string_concat(p, "- One namespace per project area; multiple files can share a namespace.\n");
    p = code_string_concat(p, "\n");
    p = code_string_concat(p, "## Known Amalgame limitations to work around\n");
    p = code_string_concat(p, "- String interpolation does NOT propagate inferred types into embedded\n");
    p = code_string_concat(p, "  calls. Workaround: stage in named locals before printing.\n");
    p = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(p, "    BAD:  Console.WriteLine(\"first: "), lb), "users.Map(u => u.Name).Get(0)"), rb), "\")\n");
    p = code_string_concat(p, "    GOOD: let names = users.Map(u => u.Name)\n");
    p = code_string_concat(p, "          let first: string = names.Get(0)\n");
    p = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(p, "          Console.WriteLine(\"first: "), lb), "first"), rb), "\")\n");
    p = code_string_concat(p, "- Lambda Reduce signatures still need init-arg type inference. If you\n");
    p = code_string_concat(p, "  hit issues with .Reduce, fall back to a for-in with `var acc`.\n");
    p = code_string_concat(p, "- ForEach captures by value: `var sum = 0; xs.ForEach(x => sum = sum + x)`\n");
    p = code_string_concat(p, "  does NOT accumulate. Use Reduce for accumulation.\n");
    code_string __attribute__((unused)) extras = Amalgame_Compiler_MigrateCommand_LoadDocsHeader();
    if (String_Length(extras) > 0) {
        p = code_string_concat(code_string_concat(p, "\n"), extras);
    }
    return p;
}

static code_string Amalgame_Compiler_MigrateCommand_BuildUserPrompt(code_string lang, code_string source) {
    (void)lang;
    (void)source;
    code_string __attribute__((unused)) p = "";
    p = code_string_concat(p, "## Source file to translate\n");
    p = code_string_concat(code_string_concat(code_string_concat(p, "```"), lang), "\n");
    p = code_string_concat(p, source);
    p = code_string_concat(p, "\n```\n");
    p = code_string_concat(p, "\n");
    p = code_string_concat(p, "## Output instructions\n");
    p = code_string_concat(p, "Reply with ONLY the Amalgame source code. No prose, no markdown\n");
    p = code_string_concat(p, "fences, no preamble like \"Here's the translation:\". Just `.am` content.\n");
    p = code_string_concat(p, "\n");
    p = code_string_concat(p, "If you encounter a construct that has no clean Amalgame equivalent,\n");
    p = code_string_concat(p, "insert a `// TODO[migrate]: <short reason>` comment instead of\n");
    p = code_string_concat(p, "best-effort guessing. Examples of such constructs:\n");
    p = code_string_concat(p, "  - Python decorators (other than @dataclass which maps to `data class`)\n");
    p = code_string_concat(p, "  - JS/TS Promises and async/await (no async runtime in Amalgame yet)\n");
    p = code_string_concat(p, "  - Java reflection / annotations\n");
    p = code_string_concat(p, "  - Rust lifetimes / ownership / borrowing\n");
    p = code_string_concat(p, "  - Go goroutines / channels\n");
    p = code_string_concat(p, "  - C++ templates with non-type parameters\n");
    p = code_string_concat(p, "  - C macros / preprocessor directives\n");
    p = code_string_concat(p, "Preserve the source's logical structure: same number of functions,\n");
    p = code_string_concat(p, "same class hierarchy, same public surface. The output should compile\n");
    p = code_string_concat(p, "with `amc --check` modulo the TODO[migrate] markers.\n");
    return p;
}

code_string Amalgame_Compiler_MigrateCommand_LoadDocsHeader() {
    AmalgameList* __attribute__((unused)) candidates = AmalgameList_new();
    code_string __attribute__((unused)) execPath = Args_Get(0);
    code_string __attribute__((unused)) execDir = Path_GetDirectory(execPath);
    if (String_Length(execDir) > 0) {
        AmalgameList_add(candidates, (void*)(intptr_t)(code_string_concat(execDir, "/../share/amalgame")));
        AmalgameList_add(candidates, (void*)(intptr_t)(execDir));
    }
    AmalgameList_add(candidates, (void*)(intptr_t)("."));
    i64 __attribute__((unused)) cn = AmalgameList_count(candidates);
    for (i64 i = 0; i < cn; i++) {
        code_string __attribute__((unused)) base = (code_string)AmalgameList_get(candidates, i);
        code_string __attribute__((unused)) grammarPath = code_string_concat(base, "/docs/language/grammar.ebnf");
        code_string __attribute__((unused)) tourPath = code_string_concat(base, "/docs/guide/02-language-tour.md");
        if (File_Exists(grammarPath) && File_Exists(tourPath)) {
            code_string __attribute__((unused)) g = File_ReadAll(grammarPath);
            code_string __attribute__((unused)) t = File_ReadAll(tourPath);
            code_string __attribute__((unused)) out = "";
            out = code_string_concat(out, "\n## Amalgame grammar (EBNF)\n\n");
            out = code_string_concat(out, g);
            out = code_string_concat(out, "\n\n## Amalgame language tour (excerpts)\n\n");
            out = code_string_concat(out, t);
            return out;
        }
    }
    return "";
}

static Amalgame_Compiler_MigrateResult* Amalgame_Compiler_MigrateCommand_CallProvider(code_string provider, code_string model, code_string lang, code_string source) {
    (void)provider;
    (void)model;
    (void)lang;
    (void)source;
    code_string __attribute__((unused)) sys = Amalgame_Compiler_MigrateCommand_BuildSystemPrompt(lang);
    code_string __attribute__((unused)) usr = Amalgame_Compiler_MigrateCommand_BuildUserPrompt(lang, source);
    return Amalgame_Compiler_MigrateCommand_CallProviderRaw(provider, model, sys, usr);
}

Amalgame_Compiler_MigrateResult* Amalgame_Compiler_MigrateCommand_CallProviderRaw(code_string provider, code_string model, code_string systemPrompt, code_string userPrompt) {
    (void)provider;
    (void)model;
    (void)systemPrompt;
    (void)userPrompt;
    if (code_string_equals(provider, "claude")) {
        return Amalgame_Compiler_MigrateCommand_CallClaudeCli(model, code_string_concat(code_string_concat(systemPrompt, "\n\n"), userPrompt));
    }
    if (code_string_equals(provider, "claude-api")) {
        return Amalgame_Compiler_MigrateCommand_CallClaudeApiRaw(model, systemPrompt, userPrompt);
    }
    if (code_string_equals(provider, "chatgpt")) {
        return Amalgame_Compiler_MigrateCommand_CallChatGptApi(model, systemPrompt, userPrompt);
    }
    if (code_string_equals(provider, "gemini")) {
        return Amalgame_Compiler_MigrateCommand_CallGeminiApi(model, systemPrompt, userPrompt);
    }
    if (code_string_equals(provider, "custom")) {
        return Amalgame_Compiler_MigrateCommand_CallCustomScript(systemPrompt, userPrompt);
    }
    Amalgame_Compiler_MigrateResult* __attribute__((unused)) res = Amalgame_Compiler_MigrateResult_new();
    res->Ok = 0;
    res->Error = code_string_concat(code_string_concat("provider '", provider), "' not supported (built-in: claude, claude-api, chatgpt, gemini, custom)");
    return res;
}

code_string Amalgame_Compiler_MigrateCommand_AutoSelectProvider() {
    if (Env_Has("ANTHROPIC_API_KEY")) {
        return "claude-api";
    }
    if (Env_Has("OPENAI_API_KEY")) {
        return "chatgpt";
    }
    if (Env_Has("GEMINI_API_KEY")) {
        return "gemini";
    }
    return "claude";
}

code_string Amalgame_Compiler_MigrateCommand_EstimateCost(code_string provider, code_string model, code_string systemPrompt, code_string userPrompt) {
    (void)provider;
    (void)model;
    (void)systemPrompt;
    (void)userPrompt;
    if (code_string_equals(provider, "claude")) {
        return "free (subscription via Claude Code CLI)";
    }
    if (code_string_equals(provider, "custom")) {
        return "free (user-managed local backend)";
    }
    i64 __attribute__((unused)) sysLen = String_Length(systemPrompt);
    i64 __attribute__((unused)) usrLen = String_Length(userPrompt);
    i64 __attribute__((unused)) totalChars = sysLen + usrLen;
    i64 __attribute__((unused)) inputToks = totalChars / 4;
    i64 __attribute__((unused)) outputToks = 1000;
    i64 __attribute__((unused)) inUsdPerM = 0;
    i64 __attribute__((unused)) outUsdPerM = 0;
    code_string __attribute__((unused)) resolvedModel = model;
    if (code_string_equals(provider, "claude-api")) {
        if (String_Length(resolvedModel) == 0) {
            resolvedModel = "claude-sonnet-4-6";
        }
        if (code_string_equals(resolvedModel, "claude-opus-4-7")) {
            inUsdPerM = 15000000;
            outUsdPerM = 75000000;
        } else if (code_string_equals(resolvedModel, "claude-haiku-4-5")) {
            inUsdPerM = 1000000;
            outUsdPerM = 5000000;
        } else {
            inUsdPerM = 3000000;
            outUsdPerM = 15000000;
        }
    } else if (code_string_equals(provider, "chatgpt")) {
        if (String_Length(resolvedModel) == 0) {
            resolvedModel = "gpt-4o-mini";
        }
        if (code_string_equals(resolvedModel, "gpt-4o")) {
            inUsdPerM = 2500000;
            outUsdPerM = 10000000;
        } else if (code_string_equals(resolvedModel, "gpt-4-turbo")) {
            inUsdPerM = 10000000;
            outUsdPerM = 30000000;
        } else {
            inUsdPerM = 150000;
            outUsdPerM = 600000;
        }
    } else if (code_string_equals(provider, "gemini")) {
        if (String_Length(resolvedModel) == 0) {
            resolvedModel = "gemini-1.5-flash";
        }
        if (code_string_equals(resolvedModel, "gemini-1.5-pro")) {
            inUsdPerM = 1250000;
            outUsdPerM = 5000000;
        } else {
            inUsdPerM = 75000;
            outUsdPerM = 300000;
        }
    }
    i64 __attribute__((unused)) inProd = inputToks * inUsdPerM;
    i64 __attribute__((unused)) inMicro = inProd / 1000000;
    i64 __attribute__((unused)) outProd = outputToks * outUsdPerM;
    i64 __attribute__((unused)) outMicro = outProd / 1000000;
    i64 __attribute__((unused)) totalMicro = inMicro + outMicro;
    i64 __attribute__((unused)) totalCents = totalMicro / 10000;
    i64 __attribute__((unused)) dollars = totalCents / 100;
    i64 __attribute__((unused)) cents = totalCents % 100;
    code_string __attribute__((unused)) centStr = String_FromInt(cents);
    if (cents < 10) {
        centStr = code_string_concat("0", centStr);
    }
    code_string __attribute__((unused)) inS = String_FromInt(inputToks);
    code_string __attribute__((unused)) outS = String_FromInt(outputToks);
    code_string __attribute__((unused)) dolS = String_FromInt(dollars);
    return code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("~", inS), " in + ~"), outS), " out -> ~$"), dolS), "."), centStr), " ("), resolvedModel), ")");
}

static Amalgame_Compiler_MigrateResult* Amalgame_Compiler_MigrateCommand_CallClaudeApiRaw(code_string model, code_string systemPrompt, code_string userPrompt) {
    (void)model;
    (void)systemPrompt;
    (void)userPrompt;
    Amalgame_Compiler_MigrateResult* __attribute__((unused)) res = Amalgame_Compiler_MigrateResult_new();
    code_string __attribute__((unused)) apiKey = Env_Get("ANTHROPIC_API_KEY");
    if (String_Length(apiKey) == 0) {
        res->Ok = 0;
        res->Error = "claude-api: ANTHROPIC_API_KEY not set. Export it or use --provider claude (CLI shell out).";
        return res;
    }
    code_string __attribute__((unused)) modelId = model;
    if (String_Length(modelId) == 0) {
        modelId = "claude-sonnet-4-6";
    }
    code_string __attribute__((unused)) body = "{";
    body = code_string_concat(code_string_concat(code_string_concat(body, "\"model\":\""), modelId), "\",");
    body = code_string_concat(body, "\"max_tokens\":8192,");
    body = code_string_concat(body, "\"system\":[{\"type\":\"text\",\"text\":\"");
    body = code_string_concat(body, Amalgame_Compiler_MigrateCommand_JsonEscape(systemPrompt));
    body = code_string_concat(body, "\",\"cache_control\":{\"type\":\"ephemeral\"}}],");
    body = code_string_concat(body, "\"messages\":[{\"role\":\"user\",\"content\":\"");
    body = code_string_concat(body, Amalgame_Compiler_MigrateCommand_JsonEscape(userPrompt));
    body = code_string_concat(body, "\"}]}");
    AmalgameMap* __attribute__((unused)) headers = AmalgameMap_new();
    AmalgameMap_set(headers, "x-api-key", (void*)(intptr_t)(apiKey));
    AmalgameMap_set(headers, "anthropic-version", (void*)(intptr_t)("2023-06-01"));
    AmalgameMap_set(headers, "Content-Type", (void*)(intptr_t)("application/json"));
    AmalgameHttpResponse* __attribute__((unused)) resp = Http_PostWithHeaders("https://api.anthropic.com/v1/messages", body, headers);
    i64 __attribute__((unused)) status = resp->Status;
    if (status != 200) {
        res->Ok = 0;
        res->Error = code_string_concat(code_string_concat(code_string_concat("claude-api: HTTP ", String_FromInt(status)), ". Response:\n"), resp->Body);
        return res;
    }
    code_string __attribute__((unused)) text = Amalgame_Compiler_MigrateCommand_JsonExtractText(resp->Body);
    if (String_Length(text) == 0) {
        res->Ok = 0;
        res->Error = code_string_concat("claude-api: empty or unparseable response. Raw body:\n", resp->Body);
        return res;
    }
    res->Ok = 1;
    res->Content = Amalgame_Compiler_MigrateCommand_StripFences(text);
    return res;
}

static code_string Amalgame_Compiler_MigrateCommand_JsonEscape(code_string s) {
    (void)s;
    code_string __attribute__((unused)) out = "";
    i64 __attribute__((unused)) n = String_Length(s);
    for (i64 i = 0; i < n; i++) {
        code_string __attribute__((unused)) c = String_CharAt1(s, i);
        if (code_string_equals(c, "\\")) {
            out = code_string_concat(out, "\\\\");
        } else if (code_string_equals(c, "\"")) {
            out = code_string_concat(out, "\\\"");
        } else if (code_string_equals(c, "\n")) {
            out = code_string_concat(out, "\\n");
        } else if (code_string_equals(c, "\\r")) {
            out = code_string_concat(out, "\\r");
        } else if (code_string_equals(c, "\t")) {
            out = code_string_concat(out, "\\t");
        } else {
            out = code_string_concat(out, c);
        }
    }
    return out;
}

static code_string Amalgame_Compiler_MigrateCommand_JsonExtractText(code_string body) {
    (void)body;
    return Amalgame_Compiler_MigrateCommand_JsonExtract(body, "\"text\":\"");
}

static code_string Amalgame_Compiler_MigrateCommand_JsonExtract(code_string body, code_string key) {
    (void)body;
    (void)key;
    i64 __attribute__((unused)) kIdx = String_IndexOf(body, key);
    if (kIdx < 0) {
        return "";
    }
    i64 __attribute__((unused)) kLen = String_Length(key);
    i64 __attribute__((unused)) start = kIdx + kLen;
    i64 __attribute__((unused)) n = String_Length(body);
    code_string __attribute__((unused)) out = "";
    i64 __attribute__((unused)) i = start;
    while (i < n) {
        code_string __attribute__((unused)) c = String_CharAt1(body, i);
        if (code_string_equals(c, "\\")) {
            if (i + 1 < n) {
                code_string __attribute__((unused)) nxt = String_CharAt1(body, i + 1);
                if (code_string_equals(nxt, "n")) {
                    out = code_string_concat(out, "\n");
                } else if (code_string_equals(nxt, "r")) {
                    out = code_string_concat(out, "\\r");
                } else if (code_string_equals(nxt, "t")) {
                    out = code_string_concat(out, "\t");
                } else if (code_string_equals(nxt, "\"")) {
                    out = code_string_concat(out, "\"");
                } else if (code_string_equals(nxt, "\\")) {
                    out = code_string_concat(out, "\\");
                } else {
                    out = code_string_concat(out, nxt);
                }
                i = i + 2;
                continue;
            }
        }
        if (code_string_equals(c, "\"")) {
            return out;
        }
        out = code_string_concat(out, c);
        i = i + 1;
    }
    return out;
}

static Amalgame_Compiler_MigrateResult* Amalgame_Compiler_MigrateCommand_CallChatGptApi(code_string model, code_string systemPrompt, code_string userPrompt) {
    (void)model;
    (void)systemPrompt;
    (void)userPrompt;
    Amalgame_Compiler_MigrateResult* __attribute__((unused)) res = Amalgame_Compiler_MigrateResult_new();
    code_string __attribute__((unused)) apiKey = Env_Get("OPENAI_API_KEY");
    if (String_Length(apiKey) == 0) {
        res->Ok = 0;
        res->Error = "chatgpt: OPENAI_API_KEY not set.";
        return res;
    }
    code_string __attribute__((unused)) modelId = model;
    if (String_Length(modelId) == 0) {
        modelId = "gpt-4o-mini";
    }
    code_string __attribute__((unused)) body = "{";
    body = code_string_concat(code_string_concat(code_string_concat(body, "\"model\":\""), modelId), "\",");
    body = code_string_concat(body, "\"messages\":[{\"role\":\"system\",\"content\":\"");
    body = code_string_concat(body, Amalgame_Compiler_MigrateCommand_JsonEscape(systemPrompt));
    body = code_string_concat(body, "\"},{\"role\":\"user\",\"content\":\"");
    body = code_string_concat(body, Amalgame_Compiler_MigrateCommand_JsonEscape(userPrompt));
    body = code_string_concat(body, "\"}]}");
    AmalgameMap* __attribute__((unused)) headers = AmalgameMap_new();
    AmalgameMap_set(headers, "Authorization", (void*)(intptr_t)(code_string_concat("Bearer ", apiKey)));
    AmalgameMap_set(headers, "Content-Type", (void*)(intptr_t)("application/json"));
    AmalgameHttpResponse* __attribute__((unused)) resp = Http_PostWithHeaders("https://api.openai.com/v1/chat/completions", body, headers);
    i64 __attribute__((unused)) status = resp->Status;
    if (status != 200) {
        res->Ok = 0;
        res->Error = code_string_concat(code_string_concat(code_string_concat("chatgpt: HTTP ", String_FromInt(status)), ". Response:\n"), resp->Body);
        return res;
    }
    code_string __attribute__((unused)) text = Amalgame_Compiler_MigrateCommand_JsonExtract(resp->Body, "\"content\":\"");
    if (String_Length(text) == 0) {
        res->Ok = 0;
        res->Error = code_string_concat("chatgpt: empty or unparseable response. Raw:\n", resp->Body);
        return res;
    }
    res->Ok = 1;
    res->Content = Amalgame_Compiler_MigrateCommand_StripFences(text);
    return res;
}

static Amalgame_Compiler_MigrateResult* Amalgame_Compiler_MigrateCommand_CallGeminiApi(code_string model, code_string systemPrompt, code_string userPrompt) {
    (void)model;
    (void)systemPrompt;
    (void)userPrompt;
    Amalgame_Compiler_MigrateResult* __attribute__((unused)) res = Amalgame_Compiler_MigrateResult_new();
    code_string __attribute__((unused)) apiKey = Env_Get("GEMINI_API_KEY");
    if (String_Length(apiKey) == 0) {
        res->Ok = 0;
        res->Error = "gemini: GEMINI_API_KEY not set.";
        return res;
    }
    code_string __attribute__((unused)) modelId = model;
    if (String_Length(modelId) == 0) {
        modelId = "gemini-1.5-flash";
    }
    code_string __attribute__((unused)) body = "{";
    body = code_string_concat(body, "\"systemInstruction\":{\"parts\":[{\"text\":\"");
    body = code_string_concat(body, Amalgame_Compiler_MigrateCommand_JsonEscape(systemPrompt));
    body = code_string_concat(body, "\"}]},");
    body = code_string_concat(body, "\"contents\":[{\"parts\":[{\"text\":\"");
    body = code_string_concat(body, Amalgame_Compiler_MigrateCommand_JsonEscape(userPrompt));
    body = code_string_concat(body, "\"}]}]}");
    AmalgameMap* __attribute__((unused)) headers = AmalgameMap_new();
    AmalgameMap_set(headers, "Content-Type", (void*)(intptr_t)("application/json"));
    code_string __attribute__((unused)) url = code_string_concat(code_string_concat(code_string_concat("https://generativelanguage.googleapis.com/v1beta/models/", modelId), ":generateContent?key="), apiKey);
    AmalgameHttpResponse* __attribute__((unused)) resp = Http_PostWithHeaders(url, body, headers);
    i64 __attribute__((unused)) status = resp->Status;
    if (status != 200) {
        res->Ok = 0;
        res->Error = code_string_concat(code_string_concat(code_string_concat("gemini: HTTP ", String_FromInt(status)), ". Response:\n"), resp->Body);
        return res;
    }
    code_string __attribute__((unused)) text = Amalgame_Compiler_MigrateCommand_JsonExtractText(resp->Body);
    if (String_Length(text) == 0) {
        res->Ok = 0;
        res->Error = code_string_concat("gemini: empty or unparseable response. Raw:\n", resp->Body);
        return res;
    }
    res->Ok = 1;
    res->Content = Amalgame_Compiler_MigrateCommand_StripFences(text);
    return res;
}

static Amalgame_Compiler_MigrateResult* Amalgame_Compiler_MigrateCommand_CallCustomScript(code_string systemPrompt, code_string userPrompt) {
    (void)systemPrompt;
    (void)userPrompt;
    Amalgame_Compiler_MigrateResult* __attribute__((unused)) res = Amalgame_Compiler_MigrateResult_new();
    code_string __attribute__((unused)) cmd = Env_Get("AMC_CUSTOM_PROVIDER_CMD");
    if (String_Length(cmd) == 0) {
        res->Ok = 0;
        res->Error = "custom: AMC_CUSTOM_PROVIDER_CMD not set. Point it at a script that reads prompt from stdin and writes the response to stdout.";
        return res;
    }
    code_string __attribute__((unused)) tmpPath = "/tmp/amc_custom_prompt.txt";
    code_string __attribute__((unused)) combined = code_string_concat(code_string_concat(systemPrompt, "\n\n"), userPrompt);
    code_bool __attribute__((unused)) writeOk = File_WriteAll(tmpPath, combined);
    if (!writeOk) {
        res->Ok = 0;
        res->Error = code_string_concat("custom: failed to write prompt temp file: ", tmpPath);
        return res;
    }
    code_string __attribute__((unused)) full = code_string_concat(code_string_concat(cmd, " < "), tmpPath);
    AmalgameProcessResult* __attribute__((unused)) rr = Process_RunCapture(full);
    if (rr->Exit != 0) {
        res->Ok = 0;
        res->Error = code_string_concat(code_string_concat(code_string_concat("custom: script exited ", String_FromInt(rr->Exit)), ". Output:\n"), rr->Stdout);
        return res;
    }
    res->Ok = 1;
    res->Content = Amalgame_Compiler_MigrateCommand_StripFences(rr->Stdout);
    return res;
}

static Amalgame_Compiler_MigrateResult* Amalgame_Compiler_MigrateCommand_CallClaudeCli(code_string model, code_string prompt) {
    (void)model;
    (void)prompt;
    Amalgame_Compiler_MigrateResult* __attribute__((unused)) res = Amalgame_Compiler_MigrateResult_new();
    if (!Amalgame_Compiler_MigrateCommand_IsCommandAvailable("claude")) {
        res->Ok = 0;
        res->Error = "claude CLI not found on PATH. Install Claude Code: https://docs.claude.com/claude-code\nOr pick another provider with --provider <name> (none ships in v0 yet).";
        return res;
    }
    code_string __attribute__((unused)) tmpPath = "/tmp/amc_migrate_prompt.txt";
    code_bool __attribute__((unused)) writeOk = File_WriteAll(tmpPath, prompt);
    if (!writeOk) {
        res->Ok = 0;
        res->Error = code_string_concat("failed to write prompt temp file: ", tmpPath);
        return res;
    }
    code_string __attribute__((unused)) cmd = "claude -p";
    if (String_Length(model) > 0) {
        cmd = code_string_concat(code_string_concat(cmd, " --model "), model);
    }
    cmd = code_string_concat(code_string_concat(cmd, " < "), tmpPath);
    AmalgameProcessResult* __attribute__((unused)) rr = Process_RunCapture(cmd);
    if (rr->Exit != 0) {
        res->Ok = 0;
        res->Error = code_string_concat(code_string_concat(code_string_concat("claude CLI failed (exit ", String_FromInt(rr->Exit)), "). Output:\n"), rr->Stdout);
        return res;
    }
    res->Ok = 1;
    res->Content = Amalgame_Compiler_MigrateCommand_StripFences(rr->Stdout);
    return res;
}

static code_bool Amalgame_Compiler_MigrateCommand_IsCommandAvailable(code_string cmd) {
    (void)cmd;
    code_string __attribute__((unused)) probe = code_string_concat(code_string_concat("command -v ", cmd), " >/dev/null 2>&1");
    AmalgameProcessResult* __attribute__((unused)) rr = Process_RunCapture(probe);
    return rr->Exit == 0;
}

code_string Amalgame_Compiler_MigrateCommand_CacheHash(code_string source, code_string systemPrompt) {
    (void)source;
    (void)systemPrompt;
    code_string __attribute__((unused)) tmpPath = "/tmp/amc_cache_input.txt";
    code_string __attribute__((unused)) combined = code_string_concat(code_string_concat(source, "\n---SYSTEM---\n"), systemPrompt);
    code_bool __attribute__((unused)) writeOk = File_WriteAll(tmpPath, combined);
    if (!writeOk) {
        return "";
    }
    AmalgameProcessResult* __attribute__((unused)) result = Process_RunCapture(code_string_concat(code_string_concat("sha256sum ", tmpPath), " 2>/dev/null"));
    if (result->Exit != 0) {
        return "";
    }
    code_string __attribute__((unused)) out = String_Trim(result->Stdout);
    if (String_Length(out) < 64) {
        return "";
    }
    return String_Substring(out, 0, 64);
}

code_string Amalgame_Compiler_MigrateCommand_CachePath(code_string hash) {
    (void)hash;
    code_string __attribute__((unused)) home = Env_Get("HOME");
    if (String_Length(home) == 0) {
        return "";
    }
    code_string __attribute__((unused)) dir = code_string_concat(home, "/.cache/amalgame/migrate");
    Process_RunCapture(code_string_concat("mkdir -p ", dir));
    return code_string_concat(code_string_concat(code_string_concat(dir, "/"), hash), ".am");
}

code_string Amalgame_Compiler_MigrateCommand_CacheLookup(code_string source, code_string systemPrompt) {
    (void)source;
    (void)systemPrompt;
    code_string __attribute__((unused)) hash = Amalgame_Compiler_MigrateCommand_CacheHash(source, systemPrompt);
    if (String_Length(hash) == 0) {
        return "";
    }
    code_string __attribute__((unused)) path = Amalgame_Compiler_MigrateCommand_CachePath(hash);
    if (String_Length(path) == 0) {
        return "";
    }
    if (!File_Exists(path)) {
        return "";
    }
    return File_ReadAll(path);
}

void Amalgame_Compiler_MigrateCommand_CacheStore(code_string source, code_string systemPrompt, code_string content) {
    (void)source;
    (void)systemPrompt;
    (void)content;
    code_string __attribute__((unused)) hash = Amalgame_Compiler_MigrateCommand_CacheHash(source, systemPrompt);
    if (String_Length(hash) == 0) {
        return;
    }
    code_string __attribute__((unused)) path = Amalgame_Compiler_MigrateCommand_CachePath(hash);
    if (String_Length(path) == 0) {
        return;
    }
    File_WriteAll(path, content);
}

i64 Amalgame_Compiler_MigrateCommand_StreamClaudeCli(code_string model, code_string prompt) {
    (void)model;
    (void)prompt;
    if (!Amalgame_Compiler_MigrateCommand_IsCommandAvailable("claude")) {
        Console_WriteError("claude CLI not found on PATH. Install Claude Code: https://docs.claude.com/claude-code");
        return 1;
    }
    code_string __attribute__((unused)) tmpPath = "/tmp/amc_stream_prompt.txt";
    code_bool __attribute__((unused)) writeOk = File_WriteAll(tmpPath, prompt);
    if (!writeOk) {
        Console_WriteError(code_string_concat("failed to write prompt temp file: ", tmpPath));
        return 1;
    }
    code_string __attribute__((unused)) cmd = "claude -p";
    if (String_Length(model) > 0) {
        cmd = code_string_concat(code_string_concat(cmd, " --model "), model);
    }
    cmd = code_string_concat(code_string_concat(cmd, " < "), tmpPath);
    return Process_Run(cmd);
}

static code_string Amalgame_Compiler_MigrateCommand_StripFences(code_string s) {
    (void)s;
    code_string __attribute__((unused)) trimmed = String_Trim(s);
    if (!String_StartsWith(trimmed, "```")) {
        return s;
    }
    i64 __attribute__((unused)) nl = String_IndexOf(trimmed, "\n");
    if (nl <= 0) {
        return s;
    }
    code_string __attribute__((unused)) afterFirst = String_Substring(trimmed, nl + 1, String_Length(trimmed) - nl - 1);
    code_string __attribute__((unused)) rtrimmed = String_Trim(afterFirst);
    if (!String_EndsWith(rtrimmed, "```")) {
        return s;
    }
    return String_Substring(rtrimmed, 0, String_Length(rtrimmed) - 3);
}

struct _Amalgame_Compiler_GenerateCommand {
};

void Amalgame_Compiler_GenerateCommand_PrintUsage();
i64 Amalgame_Compiler_GenerateCommand_Run(i64 argc);
static code_string Amalgame_Compiler_GenerateCommand_BuildSystemPrompt();
static code_string Amalgame_Compiler_GenerateCommand_BuildUserPrompt(code_string task);
static code_string Amalgame_Compiler_GenerateCommand_StripFences(code_string s);

Amalgame_Compiler_GenerateCommand* Amalgame_Compiler_GenerateCommand_new() {
    Amalgame_Compiler_GenerateCommand* self = (Amalgame_Compiler_GenerateCommand*) GC_MALLOC(sizeof(Amalgame_Compiler_GenerateCommand));
    return self;
}

void Amalgame_Compiler_GenerateCommand_PrintUsage() {
    Console_WriteError("Usage: amc generate \"<natural-language prompt>\" [flags]");
    Console_WriteError("");
    Console_WriteError("Generates an Amalgame program from a prompt via an LLM.");
    Console_WriteError("Default output is stdout; use -o to write to a file.");
    Console_WriteError("");
    Console_WriteError("Flags:");
    Console_WriteError("  -o, --output <out>   Write to <out> instead of stdout.");
    Console_WriteError("  --provider <name>    LLM provider. Built-in: claude (CLI), claude-api,");
    Console_WriteError("                       chatgpt, gemini, custom. Auto-selects API by env:");
    Console_WriteError("                       ANTHROPIC_API_KEY / OPENAI_API_KEY / GEMINI_API_KEY,");
    Console_WriteError("                       fallback claude (CLI).");
    Console_WriteError("  --model <id>         Pass a specific model id to the provider.");
    Console_WriteError("  --no-check           Skip the `amc --check` validation when -o is given.");
    Console_WriteError("  --force              Overwrite an existing file at the -o path.");
    Console_WriteError("  --dry-run            Print what would happen without invoking the LLM.");
    Console_WriteError("  --prompt-only        Dump the assembled prompt to stdout and exit.");
    Console_WriteError("  --stream             Stream the LLM response straight to stdout as it's");
    Console_WriteError("                       produced. Requires --provider claude (CLI), no -o.");
    Console_WriteError("  -h, --help           Print this help and exit.");
    Console_WriteError("");
    Console_WriteError("Examples:");
    Console_WriteError("  amc generate \"a HTTP server with /health and /version routes\"");
    Console_WriteError("  amc generate \"sieve of Eratosthenes up to N\" -o sieve.am");
}

i64 Amalgame_Compiler_GenerateCommand_Run(i64 argc) {
    (void)argc;
    code_string __attribute__((unused)) prompt = "";
    code_string __attribute__((unused)) output = "";
    code_string __attribute__((unused)) provider = "";
    code_bool __attribute__((unused)) providerSet = 0;
    code_string __attribute__((unused)) model = "";
    code_bool __attribute__((unused)) dryRun = 0;
    code_bool __attribute__((unused)) promptOnly = 0;
    code_bool __attribute__((unused)) noCheck = 0;
    code_bool __attribute__((unused)) force = 0;
    code_bool __attribute__((unused)) stream = 0;
    i64 __attribute__((unused)) i = 2;
    while (i < argc) {
        code_string __attribute__((unused)) a = Args_Get(i);
        if (code_string_equals(a, "-h") || code_string_equals(a, "--help")) {
            Amalgame_Compiler_GenerateCommand_PrintUsage();
            return 0;
        } else if (code_string_equals(a, "-o") || code_string_equals(a, "--output")) {
            i = i + 1;
            if (i >= argc) {
                Console_WriteError(code_string_concat(code_string_concat("amc generate: ", a), " requires a value"));
                return 1;
            }
            output = Args_Get(i);
        } else if (code_string_equals(a, "--dry-run")) {
            dryRun = 1;
        } else if (code_string_equals(a, "--prompt-only")) {
            promptOnly = 1;
        } else if (code_string_equals(a, "--no-check")) {
            noCheck = 1;
        } else if (code_string_equals(a, "--force")) {
            force = 1;
        } else if (code_string_equals(a, "--stream")) {
            stream = 1;
        } else if (code_string_equals(a, "--provider")) {
            i = i + 1;
            if (i >= argc) {
                Console_WriteError("amc generate: --provider requires a value");
                return 1;
            }
            provider = Args_Get(i);
            providerSet = 1;
        } else if (code_string_equals(a, "--model")) {
            i = i + 1;
            if (i >= argc) {
                Console_WriteError("amc generate: --model requires a value");
                return 1;
            }
            model = Args_Get(i);
        } else if (String_StartsWith(a, "-")) {
            Console_WriteError(code_string_concat(code_string_concat("amc generate: unknown option '", a), "'"));
            return 1;
        } else {
            if (String_Length(prompt) > 0) {
                Console_WriteError("amc generate: too many positional arguments. Quote your prompt: amc generate \"...\"");
                return 1;
            }
            prompt = a;
        }
        i = i + 1;
    }
    if (String_Length(prompt) == 0) {
        Console_WriteError("amc generate: no prompt given");
        Console_WriteError("usage: amc generate \"<natural-language prompt>\" [flags]");
        return 1;
    }
    if (!providerSet) {
        provider = Amalgame_Compiler_MigrateCommand_AutoSelectProvider();
    }
    code_string __attribute__((unused)) systemPrompt = Amalgame_Compiler_GenerateCommand_BuildSystemPrompt();
    code_string __attribute__((unused)) userPrompt = Amalgame_Compiler_GenerateCommand_BuildUserPrompt(prompt);
    if (promptOnly) {
        Console_WriteLine(systemPrompt);
        Console_WriteLine("");
        Console_WriteLine(userPrompt);
        return 0;
    }
    if (dryRun) {
        code_string __attribute__((unused)) est = Amalgame_Compiler_MigrateCommand_EstimateCost(provider, model, systemPrompt, userPrompt);
        Console_WriteLine(code_string_concat(code_string_concat("[generate] would generate from prompt (", String_FromInt(String_Length(prompt))), " chars)"));
        if (String_Length(output) > 0) {
            Console_WriteLine(code_string_concat("[generate] would write:   ", output));
        } else {
            Console_WriteLine("[generate] would write:   <stdout>");
        }
        Console_WriteLine(code_string_concat("[generate] provider:      ", provider));
        if (String_Length(model) > 0) {
            Console_WriteLine(code_string_concat("[generate] model:         ", model));
        }
        Console_WriteLine(code_string_concat("[generate] estimated cost: ", est));
        return 0;
    }
    if (String_Length(output) > 0 && File_Exists(output) && !force) {
        Console_WriteError(code_string_concat("amc generate: output exists: ", output));
        Console_WriteError("Pass --force to overwrite.");
        return 1;
    }
    if (stream) {
        if (!code_string_equals(provider, "claude")) {
            Console_WriteError("amc generate: --stream requires --provider claude (CLI). API streaming is a v3 follow-up.");
            return 1;
        }
        if (String_Length(output) > 0) {
            Console_WriteError("amc generate: --stream is incompatible with -o (no buffered text to write).");
            return 1;
        }
        Console_WriteError("[generate] streaming via claude CLI...");
        return Amalgame_Compiler_MigrateCommand_StreamClaudeCli(model, code_string_concat(code_string_concat(systemPrompt, "\n\n"), userPrompt));
    }
    Console_WriteError(code_string_concat(code_string_concat(code_string_concat(code_string_concat("[generate] generating from prompt (", String_FromInt(String_Length(prompt))), " chars, provider="), provider), ")..."));
    Amalgame_Compiler_MigrateResult* __attribute__((unused)) result = Amalgame_Compiler_MigrateCommand_CallProviderRaw(provider, model, systemPrompt, userPrompt);
    if (!result->Ok) {
        Console_WriteError(code_string_concat("amc generate: ", result->Error));
        return 1;
    }
    code_string __attribute__((unused)) content = Amalgame_Compiler_GenerateCommand_StripFences(result->Content);
    if (String_Length(output) == 0) {
        Console_WriteLine(content);
        return 0;
    }
    code_bool __attribute__((unused)) writeOk = File_WriteAll(output, content);
    if (!writeOk) {
        Console_WriteError(code_string_concat("amc generate: failed to write ", output));
        return 1;
    }
    Console_WriteLine(code_string_concat("[generate] wrote ", output));
    if (!noCheck) {
        code_string __attribute__((unused)) amcPath = Args_Get(0);
        code_string __attribute__((unused)) cmd = code_string_concat(code_string_concat(amcPath, " --check "), output);
        AmalgameProcessResult* __attribute__((unused)) check = Process_RunCapture(cmd);
        if (check->Exit != 0) {
            Console_WriteError("[generate] check failed (typechecker errors in the generated file):");
            Console_WriteError(check->Stdout);
            Console_WriteError("The .am file was still written so you can inspect / fix manually.");
            return 1;
        }
        Console_WriteLine("[generate] check passed");
    }
    return 0;
}

static code_string Amalgame_Compiler_GenerateCommand_BuildSystemPrompt() {
    code_string __attribute__((unused)) lb = "{";
    code_string __attribute__((unused)) rb = "}";
    code_string __attribute__((unused)) p = "";
    p = code_string_concat(p, "You are writing an Amalgame program from scratch.\n");
    p = code_string_concat(p, "Amalgame is a self-hosted programming language that transpiles to C.\n");
    p = code_string_concat(p, "It uses class-based OOP with explicit visibility modifiers, generic\n");
    p = code_string_concat(p, "collections (List<T>, Map<K,V>, Set<T>), ML-style match expressions,\n");
    p = code_string_concat(p, "exception-based error handling, and higher-order list operations\n");
    p = code_string_concat(p, "(.Map / .Filter / .Reduce / .Any / .All / .CountIf).\n");
    p = code_string_concat(p, "\n");
    p = code_string_concat(p, "## Amalgame conventions\n");
    p = code_string_concat(p, "- Files start with `namespace <Name>` then declarations.\n");
    p = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(p, "- Public entry point: `public class Program "), lb), " public static void Main(string[] args) "), lb), " ... "), rb), " "), rb), "`.\n");
    p = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(p, "- Classes: `public class Name "), lb), " public Field: int = 0 "), rb), "`.\n");
    p = code_string_concat(p, "- Data classes: `public data class User(string Name, int Age)`.\n");
    p = code_string_concat(p, "- Locals: `let x = 1` (immutable), `var y = 2` (mutable).\n");
    p = code_string_concat(p, "- Lambdas: `(x, y) => x + y` or block form.\n");
    p = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(p, "- Match: `match x "), lb), " 0 => \"zero\", _ => \"other\" "), rb), "`.\n");
    p = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(p, "- Console output: `Console.WriteLine(\"x="), lb), "x"), rb), "\")` (string interpolation).\n");
    p = code_string_concat(p, "\n");
    code_string __attribute__((unused)) extras = Amalgame_Compiler_MigrateCommand_LoadDocsHeader();
    if (String_Length(extras) > 0) {
        p = code_string_concat(p, extras);
    }
    return p;
}

static code_string Amalgame_Compiler_GenerateCommand_BuildUserPrompt(code_string task) {
    (void)task;
    code_string __attribute__((unused)) p = "";
    p = code_string_concat(p, "## Task\n");
    p = code_string_concat(code_string_concat(p, task), "\n");
    p = code_string_concat(p, "\n");
    p = code_string_concat(p, "## Output instructions\n");
    p = code_string_concat(p, "Reply with ONLY the Amalgame source code. No prose, no markdown\n");
    p = code_string_concat(p, "fences, no preamble like \"Here's the program:\". Just `.am` content.\n");
    p = code_string_concat(p, "The output should compile with `amc --check`.\n");
    return p;
}

static code_string Amalgame_Compiler_GenerateCommand_StripFences(code_string s) {
    (void)s;
    code_string __attribute__((unused)) trimmed = String_Trim(s);
    if (!String_StartsWith(trimmed, "```")) {
        return s;
    }
    i64 __attribute__((unused)) nl = String_IndexOf(trimmed, "\n");
    if (nl <= 0) {
        return s;
    }
    code_string __attribute__((unused)) afterFirst = String_Substring(trimmed, nl + 1, String_Length(trimmed) - nl - 1);
    code_string __attribute__((unused)) rtrimmed = String_Trim(afterFirst);
    if (!String_EndsWith(rtrimmed, "```")) {
        return s;
    }
    return String_Substring(rtrimmed, 0, String_Length(rtrimmed) - 3);
}

struct _Amalgame_Compiler_ExplainCommand {
};

void Amalgame_Compiler_ExplainCommand_PrintUsage();
i64 Amalgame_Compiler_ExplainCommand_Run(i64 argc);
static code_string Amalgame_Compiler_ExplainCommand_BuildSystemPrompt(code_string outLang);
static code_string Amalgame_Compiler_ExplainCommand_BuildUserPrompt(code_string path, code_string source);

Amalgame_Compiler_ExplainCommand* Amalgame_Compiler_ExplainCommand_new() {
    Amalgame_Compiler_ExplainCommand* self = (Amalgame_Compiler_ExplainCommand*) GC_MALLOC(sizeof(Amalgame_Compiler_ExplainCommand));
    return self;
}

void Amalgame_Compiler_ExplainCommand_PrintUsage() {
    Console_WriteError("Usage: amc explain <file.am> [flags]");
    Console_WriteError("");
    Console_WriteError("Reads an Amalgame source file and prints a natural-language");
    Console_WriteError("explanation of what it does. Default output is stdout.");
    Console_WriteError("");
    Console_WriteError("Flags:");
    Console_WriteError("  -o, --output <out>   Write to <out> instead of stdout.");
    Console_WriteError("  --lang <name>        Output language for the explanation.");
    Console_WriteError("                       Default: English. Try --lang French, etc.");
    Console_WriteError("  --provider <name>    LLM provider. Built-in: claude (CLI), claude-api,");
    Console_WriteError("                       chatgpt, gemini, custom. Auto-selects API by env:");
    Console_WriteError("                       ANTHROPIC_API_KEY / OPENAI_API_KEY / GEMINI_API_KEY,");
    Console_WriteError("                       fallback claude (CLI).");
    Console_WriteError("  --model <id>         Pass a specific model id to the provider.");
    Console_WriteError("  --force              Overwrite an existing file at the -o path.");
    Console_WriteError("  --dry-run            Print what would happen without invoking the LLM.");
    Console_WriteError("  --prompt-only        Dump the assembled prompt to stdout and exit.");
    Console_WriteError("  --stream             Stream the LLM response straight to stdout as it's");
    Console_WriteError("                       produced. Requires --provider claude (CLI), no -o.");
    Console_WriteError("  -h, --help           Print this help and exit.");
}

i64 Amalgame_Compiler_ExplainCommand_Run(i64 argc) {
    (void)argc;
    code_string __attribute__((unused)) input = "";
    code_string __attribute__((unused)) output = "";
    code_string __attribute__((unused)) outLang = "English";
    code_string __attribute__((unused)) provider = "";
    code_bool __attribute__((unused)) providerSet = 0;
    code_string __attribute__((unused)) model = "";
    code_bool __attribute__((unused)) dryRun = 0;
    code_bool __attribute__((unused)) promptOnly = 0;
    code_bool __attribute__((unused)) force = 0;
    code_bool __attribute__((unused)) stream = 0;
    i64 __attribute__((unused)) i = 2;
    while (i < argc) {
        code_string __attribute__((unused)) a = Args_Get(i);
        if (code_string_equals(a, "-h") || code_string_equals(a, "--help")) {
            Amalgame_Compiler_ExplainCommand_PrintUsage();
            return 0;
        } else if (code_string_equals(a, "-o") || code_string_equals(a, "--output")) {
            i = i + 1;
            if (i >= argc) {
                Console_WriteError(code_string_concat(code_string_concat("amc explain: ", a), " requires a value"));
                return 1;
            }
            output = Args_Get(i);
        } else if (code_string_equals(a, "--dry-run")) {
            dryRun = 1;
        } else if (code_string_equals(a, "--prompt-only")) {
            promptOnly = 1;
        } else if (code_string_equals(a, "--force")) {
            force = 1;
        } else if (code_string_equals(a, "--stream")) {
            stream = 1;
        } else if (code_string_equals(a, "--lang")) {
            i = i + 1;
            if (i >= argc) {
                Console_WriteError("amc explain: --lang requires a value");
                return 1;
            }
            outLang = Args_Get(i);
        } else if (code_string_equals(a, "--provider")) {
            i = i + 1;
            if (i >= argc) {
                Console_WriteError("amc explain: --provider requires a value");
                return 1;
            }
            provider = Args_Get(i);
            providerSet = 1;
        } else if (code_string_equals(a, "--model")) {
            i = i + 1;
            if (i >= argc) {
                Console_WriteError("amc explain: --model requires a value");
                return 1;
            }
            model = Args_Get(i);
        } else if (String_StartsWith(a, "-")) {
            Console_WriteError(code_string_concat(code_string_concat("amc explain: unknown option '", a), "'"));
            return 1;
        } else {
            if (String_Length(input) > 0) {
                Console_WriteError("amc explain: too many positional arguments");
                return 1;
            }
            input = a;
        }
        i = i + 1;
    }
    if (String_Length(input) == 0) {
        Console_WriteError("amc explain: no input file");
        Console_WriteError("usage: amc explain <file.am> [flags]");
        return 1;
    }
    if (!File_Exists(input)) {
        Console_WriteError(code_string_concat("amc explain: file not found: ", input));
        return 1;
    }
    if (!providerSet) {
        provider = Amalgame_Compiler_MigrateCommand_AutoSelectProvider();
    }
    code_string __attribute__((unused)) source = File_ReadAll(input);
    code_string __attribute__((unused)) systemPrompt = Amalgame_Compiler_ExplainCommand_BuildSystemPrompt(outLang);
    code_string __attribute__((unused)) userPrompt = Amalgame_Compiler_ExplainCommand_BuildUserPrompt(input, source);
    if (promptOnly) {
        Console_WriteLine(systemPrompt);
        Console_WriteLine("");
        Console_WriteLine(userPrompt);
        return 0;
    }
    if (dryRun) {
        code_string __attribute__((unused)) est = Amalgame_Compiler_MigrateCommand_EstimateCost(provider, model, systemPrompt, userPrompt);
        Console_WriteLine(code_string_concat(code_string_concat(code_string_concat(code_string_concat("[explain] would explain: ", input), " ("), String_FromInt(String_Length(source))), " chars)"));
        if (String_Length(output) > 0) {
            Console_WriteLine(code_string_concat("[explain] would write:   ", output));
        } else {
            Console_WriteLine("[explain] would write:   <stdout>");
        }
        Console_WriteLine(code_string_concat("[explain] provider:      ", provider));
        Console_WriteLine(code_string_concat("[explain] output lang:   ", outLang));
        Console_WriteLine(code_string_concat("[explain] estimated cost: ", est));
        return 0;
    }
    if (String_Length(output) > 0 && File_Exists(output) && !force) {
        Console_WriteError(code_string_concat("amc explain: output exists: ", output));
        Console_WriteError("Pass --force to overwrite.");
        return 1;
    }
    if (stream) {
        if (!code_string_equals(provider, "claude")) {
            Console_WriteError("amc explain: --stream requires --provider claude (CLI). API streaming is a v3 follow-up.");
            return 1;
        }
        if (String_Length(output) > 0) {
            Console_WriteError("amc explain: --stream is incompatible with -o (no buffered text to write).");
            return 1;
        }
        Console_WriteError("[explain] streaming via claude CLI...");
        return Amalgame_Compiler_MigrateCommand_StreamClaudeCli(model, code_string_concat(code_string_concat(systemPrompt, "\n\n"), userPrompt));
    }
    Console_WriteError(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("[explain] explaining ", input), " (provider="), provider), ", lang="), outLang), ")..."));
    Amalgame_Compiler_MigrateResult* __attribute__((unused)) result = Amalgame_Compiler_MigrateCommand_CallProviderRaw(provider, model, systemPrompt, userPrompt);
    if (!result->Ok) {
        Console_WriteError(code_string_concat("amc explain: ", result->Error));
        return 1;
    }
    if (String_Length(output) == 0) {
        Console_WriteLine(result->Content);
        return 0;
    }
    code_bool __attribute__((unused)) writeOk = File_WriteAll(output, result->Content);
    if (!writeOk) {
        Console_WriteError(code_string_concat("amc explain: failed to write ", output));
        return 1;
    }
    Console_WriteLine(code_string_concat("[explain] wrote ", output));
    return 0;
}

static code_string Amalgame_Compiler_ExplainCommand_BuildSystemPrompt(code_string outLang) {
    (void)outLang;
    code_string __attribute__((unused)) p = "";
    p = code_string_concat(p, "You are explaining Amalgame source code to a developer.\n");
    p = code_string_concat(p, "Amalgame is a self-hosted programming language that transpiles to C.\n");
    p = code_string_concat(p, "It uses class-based OOP, generic collections (List<T>, Map<K,V>, Set<T>),\n");
    p = code_string_concat(p, "ML-style match expressions, exception-based error handling, and\n");
    p = code_string_concat(p, "higher-order list operations (.Map / .Filter / .Reduce / .Any / .All).\n");
    p = code_string_concat(p, "\n");
    p = code_string_concat(code_string_concat(code_string_concat(p, "Write the explanation in "), outLang), ".\n");
    p = code_string_concat(p, "\n");
    code_string __attribute__((unused)) extras = Amalgame_Compiler_MigrateCommand_LoadDocsHeader();
    if (String_Length(extras) > 0) {
        p = code_string_concat(p, extras);
    }
    return p;
}

static code_string Amalgame_Compiler_ExplainCommand_BuildUserPrompt(code_string path, code_string source) {
    (void)path;
    (void)source;
    code_string __attribute__((unused)) p = "";
    p = code_string_concat(code_string_concat(code_string_concat(p, "## Amalgame source: "), path), "\n");
    p = code_string_concat(p, "```amalgame\n");
    p = code_string_concat(p, source);
    p = code_string_concat(p, "\n```\n");
    p = code_string_concat(p, "\n");
    p = code_string_concat(p, "## What to cover\n");
    p = code_string_concat(p, "Explain this code clearly and concisely:\n");
    p = code_string_concat(p, "  1. The overall purpose (what problem does it solve?).\n");
    p = code_string_concat(p, "  2. The main types and their roles.\n");
    p = code_string_concat(p, "  3. The control flow of the entry point.\n");
    p = code_string_concat(p, "  4. Anything Amalgame-specific worth highlighting (lambdas,\n");
    p = code_string_concat(p, "     match expressions, generics) and what they contribute.\n");
    p = code_string_concat(p, "  5. Edge cases or limitations the reader should know about.\n");
    p = code_string_concat(p, "\n");
    p = code_string_concat(p, "Use prose with short headings (Markdown). Quote short snippets\n");
    p = code_string_concat(p, "when illustrative. Don't restate the obvious — focus on the\n");
    p = code_string_concat(p, "non-trivial decisions in the code.\n");
    return p;
}

struct _Amalgame_Compiler_NewCommand {
};

void Amalgame_Compiler_NewCommand_PrintUsage();
i64 Amalgame_Compiler_NewCommand_Run(i64 argc);
static i64 Amalgame_Compiler_NewCommand_ScaffoldExe(code_string path, code_string base);
static i64 Amalgame_Compiler_NewCommand_ScaffoldLib(code_string path, code_string base);
static i64 Amalgame_Compiler_NewCommand_ScaffoldTest(code_string path, code_string base);
static code_string Amalgame_Compiler_NewCommand_MainAmExe(code_string name);
static code_string Amalgame_Compiler_NewCommand_TestAmExe(code_string name);
static code_string Amalgame_Compiler_NewCommand_BuildShExe(code_string name);
static code_string Amalgame_Compiler_NewCommand_ReadmeExe(code_string name);
static code_string Amalgame_Compiler_NewCommand_LibAm(code_string name);
static code_string Amalgame_Compiler_NewCommand_BuildShLib(code_string name);
static code_string Amalgame_Compiler_NewCommand_ReadmeLib(code_string name);
static code_string Amalgame_Compiler_NewCommand_TestAmTest(code_string name);
static code_string Amalgame_Compiler_NewCommand_ReadmeTest(code_string name);
static code_string Amalgame_Compiler_NewCommand_GitignoreCommon();
static code_bool Amalgame_Compiler_NewCommand_WriteFile(code_string path, code_string content);
static code_string Amalgame_Compiler_NewCommand_ShellEscape(code_string s);
static code_bool Amalgame_Compiler_NewCommand_IsSafeName(code_string s);
static code_string Amalgame_Compiler_NewCommand_Capitalize(code_string s);
static code_string Amalgame_Compiler_NewCommand_Basename(code_string p);

Amalgame_Compiler_NewCommand* Amalgame_Compiler_NewCommand_new() {
    Amalgame_Compiler_NewCommand* self = (Amalgame_Compiler_NewCommand*) GC_MALLOC(sizeof(Amalgame_Compiler_NewCommand));
    return self;
}

void Amalgame_Compiler_NewCommand_PrintUsage() {
    Console_WriteError("Usage: amc new <name> [--template <kind>] [--force]");
    Console_WriteError("");
    Console_WriteError("Scaffold a new Amalgame project named <name>.");
    Console_WriteError("");
    Console_WriteError("Templates:");
    Console_WriteError("  exe    Default. src/main.am with Program.Main + a passing test.");
    Console_WriteError("  lib    src/<name>.am with a public class skeleton, no main.");
    Console_WriteError("  test   tests/<name>_test.am only — bolt onto an existing project.");
    Console_WriteError("");
    Console_WriteError("Flags:");
    Console_WriteError("  --template <kind>    One of: exe (default), lib, test.");
    Console_WriteError("  --force              Overwrite if <name>/ already exists.");
    Console_WriteError("  -h, --help           Print this help and exit.");
}

i64 Amalgame_Compiler_NewCommand_Run(i64 argc) {
    (void)argc;
    code_string __attribute__((unused)) name = "";
    code_string __attribute__((unused)) template = "exe";
    code_bool __attribute__((unused)) force = 0;
    i64 __attribute__((unused)) i = 2;
    while (i < argc) {
        code_string __attribute__((unused)) a = Args_Get(i);
        if (code_string_equals(a, "-h") || code_string_equals(a, "--help")) {
            Amalgame_Compiler_NewCommand_PrintUsage();
            return 0;
        }
        if (code_string_equals(a, "--force")) {
            force = 1;
        } else if (code_string_equals(a, "--template")) {
            if (i + 1 >= argc) {
                Console_WriteError("amc new: --template needs a value");
                return 1;
            }
            i = i + 1;
            template = Args_Get(i);
        } else if (String_StartsWith(a, "-")) {
            Console_WriteError(code_string_concat(code_string_concat("amc new: unknown flag '", a), "'"));
            Amalgame_Compiler_NewCommand_PrintUsage();
            return 1;
        } else {
            if (String_Length(name) == 0) {
                name = a;
            } else {
                Console_WriteError(code_string_concat(code_string_concat("amc new: extra positional '", a), "'"));
                return 1;
            }
        }
        i = i + 1;
    }
    if (String_Length(name) == 0) {
        Console_WriteError("amc new: missing <name>");
        Amalgame_Compiler_NewCommand_PrintUsage();
        return 1;
    }
    if (!code_string_equals(template, "exe") && !code_string_equals(template, "lib") && !code_string_equals(template, "test")) {
        Console_WriteError(code_string_concat(code_string_concat("amc new: unknown template '", template), "' (try exe / lib / test)"));
        return 1;
    }
    code_string __attribute__((unused)) baseName = Amalgame_Compiler_NewCommand_Basename(name);
    if (!Amalgame_Compiler_NewCommand_IsSafeName(baseName)) {
        Console_WriteError(code_string_concat(code_string_concat("amc new: '", baseName), "' is not a safe project name (a-z, A-Z, 0-9, _ or -)"));
        return 1;
    }
    if (File_Exists(name) && !force) {
        Console_WriteError(code_string_concat(code_string_concat("amc new: '", name), "' already exists (pass --force to overwrite)"));
        return 1;
    }
    i64 __attribute__((unused)) mkExit = Process_Run(code_string_concat("mkdir -p ", Amalgame_Compiler_NewCommand_ShellEscape(name)));
    if (mkExit != 0) {
        Console_WriteError(code_string_concat(code_string_concat(code_string_concat(code_string_concat("amc new: failed to create directory '", name), "' (mkdir exited "), String_FromInt(mkExit)), ")"));
        return 1;
    }
    if (code_string_equals(template, "exe")) {
        return Amalgame_Compiler_NewCommand_ScaffoldExe(name, baseName);
    }
    if (code_string_equals(template, "lib")) {
        return Amalgame_Compiler_NewCommand_ScaffoldLib(name, baseName);
    }
    return Amalgame_Compiler_NewCommand_ScaffoldTest(name, baseName);
}

static i64 Amalgame_Compiler_NewCommand_ScaffoldExe(code_string path, code_string base) {
    (void)path;
    (void)base;
    i64 __attribute__((unused)) srcDir = Process_Run(code_string_concat("mkdir -p ", Amalgame_Compiler_NewCommand_ShellEscape(code_string_concat(path, "/src"))));
    i64 __attribute__((unused)) testDir = Process_Run(code_string_concat("mkdir -p ", Amalgame_Compiler_NewCommand_ShellEscape(code_string_concat(path, "/tests"))));
    if (srcDir != 0 || testDir != 0) {
        Console_WriteError("amc new: failed to create subdirectories");
        return 1;
    }
    code_string __attribute__((unused)) mainAm = Amalgame_Compiler_NewCommand_MainAmExe(base);
    code_string __attribute__((unused)) testAm = Amalgame_Compiler_NewCommand_TestAmExe(base);
    code_string __attribute__((unused)) buildSh = Amalgame_Compiler_NewCommand_BuildShExe(base);
    code_string __attribute__((unused)) readme = Amalgame_Compiler_NewCommand_ReadmeExe(base);
    code_string __attribute__((unused)) gitignore = Amalgame_Compiler_NewCommand_GitignoreCommon();
    if (!Amalgame_Compiler_NewCommand_WriteFile(code_string_concat(path, "/src/main.am"), mainAm)) {
        return 1;
    }
    if (!Amalgame_Compiler_NewCommand_WriteFile(code_string_concat(path, "/tests/hello_test.am"), testAm)) {
        return 1;
    }
    if (!Amalgame_Compiler_NewCommand_WriteFile(code_string_concat(path, "/build.sh"), buildSh)) {
        return 1;
    }
    if (!Amalgame_Compiler_NewCommand_WriteFile(code_string_concat(path, "/.gitignore"), gitignore)) {
        return 1;
    }
    if (!Amalgame_Compiler_NewCommand_WriteFile(code_string_concat(path, "/README.md"), readme)) {
        return 1;
    }
    Process_Run(code_string_concat("chmod +x ", Amalgame_Compiler_NewCommand_ShellEscape(code_string_concat(path, "/build.sh"))));
    Console_WriteLine(code_string_concat(code_string_concat("Scaffolded '", base), "' (exe template)."));
    Console_WriteLine(code_string_concat("  cd ", path));
    Console_WriteLine(code_string_concat("  ./build.sh && ./", base));
    return 0;
}

static i64 Amalgame_Compiler_NewCommand_ScaffoldLib(code_string path, code_string base) {
    (void)path;
    (void)base;
    i64 __attribute__((unused)) srcDir = Process_Run(code_string_concat("mkdir -p ", Amalgame_Compiler_NewCommand_ShellEscape(code_string_concat(path, "/src"))));
    if (srcDir != 0) {
        Console_WriteError("amc new: failed to create src/ subdirectory");
        return 1;
    }
    code_string __attribute__((unused)) libAm = Amalgame_Compiler_NewCommand_LibAm(base);
    code_string __attribute__((unused)) buildSh = Amalgame_Compiler_NewCommand_BuildShLib(base);
    code_string __attribute__((unused)) readme = Amalgame_Compiler_NewCommand_ReadmeLib(base);
    code_string __attribute__((unused)) gitignore = Amalgame_Compiler_NewCommand_GitignoreCommon();
    if (!Amalgame_Compiler_NewCommand_WriteFile(code_string_concat(code_string_concat(code_string_concat(path, "/src/"), base), ".am"), libAm)) {
        return 1;
    }
    if (!Amalgame_Compiler_NewCommand_WriteFile(code_string_concat(path, "/build.sh"), buildSh)) {
        return 1;
    }
    if (!Amalgame_Compiler_NewCommand_WriteFile(code_string_concat(path, "/.gitignore"), gitignore)) {
        return 1;
    }
    if (!Amalgame_Compiler_NewCommand_WriteFile(code_string_concat(path, "/README.md"), readme)) {
        return 1;
    }
    Process_Run(code_string_concat("chmod +x ", Amalgame_Compiler_NewCommand_ShellEscape(code_string_concat(path, "/build.sh"))));
    Console_WriteLine(code_string_concat(code_string_concat("Scaffolded '", base), "' (lib template)."));
    Console_WriteLine(code_string_concat("  cd ", path));
    Console_WriteLine(code_string_concat(code_string_concat("  ./build.sh   # builds ", base), ".o (library)"));
    return 0;
}

static i64 Amalgame_Compiler_NewCommand_ScaffoldTest(code_string path, code_string base) {
    (void)path;
    (void)base;
    i64 __attribute__((unused)) testDir = Process_Run(code_string_concat("mkdir -p ", Amalgame_Compiler_NewCommand_ShellEscape(code_string_concat(path, "/tests"))));
    if (testDir != 0) {
        Console_WriteError("amc new: failed to create tests/ subdirectory");
        return 1;
    }
    code_string __attribute__((unused)) testAm = Amalgame_Compiler_NewCommand_TestAmTest(base);
    code_string __attribute__((unused)) readme = Amalgame_Compiler_NewCommand_ReadmeTest(base);
    code_string __attribute__((unused)) gitignore = Amalgame_Compiler_NewCommand_GitignoreCommon();
    if (!Amalgame_Compiler_NewCommand_WriteFile(code_string_concat(code_string_concat(code_string_concat(path, "/tests/"), base), "_test.am"), testAm)) {
        return 1;
    }
    if (!Amalgame_Compiler_NewCommand_WriteFile(code_string_concat(path, "/.gitignore"), gitignore)) {
        return 1;
    }
    if (!Amalgame_Compiler_NewCommand_WriteFile(code_string_concat(path, "/README.md"), readme)) {
        return 1;
    }
    Console_WriteLine(code_string_concat(code_string_concat("Scaffolded '", base), "' (test template)."));
    Console_WriteLine(code_string_concat("  cd ", path));
    Console_WriteLine("  amc test tests/");
    return 0;
}

static code_string Amalgame_Compiler_NewCommand_MainAmExe(code_string name) {
    (void)name;
    code_string __attribute__((unused)) s = "";
    s = code_string_concat(code_string_concat(code_string_concat(s, "namespace "), name), "\n");
    s = code_string_concat(s, "\n");
    s = code_string_concat(s, "import Amalgame.Collections\n");
    s = code_string_concat(s, "import Amalgame.IO\n");
    s = code_string_concat(s, "import Amalgame.String\n");
    s = code_string_concat(s, "\n");
    s = code_string_concat(s, "public class Program {\n");
    s = code_string_concat(s, "    public static int Main(List<string> args) {\n");
    s = code_string_concat(code_string_concat(code_string_concat(s, "        Console.WriteLine(\"Hello from "), name), "!\")\n");
    s = code_string_concat(s, "        return 0\n");
    s = code_string_concat(s, "    }\n");
    s = code_string_concat(s, "}\n");
    return s;
}

static code_string Amalgame_Compiler_NewCommand_TestAmExe(code_string name) {
    (void)name;
    code_string __attribute__((unused)) s = "";
    s = code_string_concat(code_string_concat(code_string_concat(s, "// Smoke test for "), name), ". `amc test tests/` runs this file\n");
    s = code_string_concat(s, "// and counts the [PASS]/[FAIL] tags it prints.\n");
    s = code_string_concat(s, "\n");
    s = code_string_concat(s, "import Amalgame.IO\n");
    s = code_string_concat(s, "\n");
    s = code_string_concat(s, "class Program {\n");
    s = code_string_concat(s, "    public static void Main() {\n");
    s = code_string_concat(s, "        let n: int = 1 + 1\n");
    s = code_string_concat(s, "        if (n == 2) {\n");
    s = code_string_concat(s, "            Console.WriteLine(\"[PASS] sanity\")\n");
    s = code_string_concat(s, "        } else {\n");
    s = code_string_concat(s, "            Console.WriteLine(\"[FAIL] sanity\")\n");
    s = code_string_concat(s, "        }\n");
    s = code_string_concat(s, "    }\n");
    s = code_string_concat(s, "}\n");
    return s;
}

static code_string Amalgame_Compiler_NewCommand_BuildShExe(code_string name) {
    (void)name;
    code_string __attribute__((unused)) lb = "{";
    code_string __attribute__((unused)) rb = "}";
    code_string __attribute__((unused)) s = "";
    s = code_string_concat(s, "#!/bin/bash\n");
    s = code_string_concat(code_string_concat(code_string_concat(s, "# Build script for "), name), ". Compiles src/main.am to a native\n");
    s = code_string_concat(code_string_concat(code_string_concat(s, "# binary named `"), name), "` in the project root.\n");
    s = code_string_concat(s, "#\n");
    s = code_string_concat(s, "# amc emits the .c bundle; gcc links it against the Amalgame\n");
    s = code_string_concat(s, "# runtime headers. Set AMALGAME_HOME if your runtime/ lives\n");
    s = code_string_concat(s, "# outside the install default.\n");
    s = code_string_concat(s, "set -e\n");
    s = code_string_concat(s, "cd \"$(dirname \"$0\")\"\n");
    s = code_string_concat(s, "\n");
    s = code_string_concat(s, "# Locate runtime/ (env override, install dirs, then the\n");
    s = code_string_concat(s, "# Amalgame repo if amc is on PATH).\n");
    s = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(s, "RUNTIME=\"$"), lb), "AMALGAME_HOME:-"), rb), "\"\n");
    s = code_string_concat(s, "if [ -z \"$RUNTIME\" ] || [ ! -d \"$RUNTIME/runtime\" ]; then\n");
    s = code_string_concat(s, "  for d in /usr/local/share/amalgame /usr/share/amalgame; do\n");
    s = code_string_concat(s, "    [ -d \"$d/runtime\" ] && RUNTIME=\"$d\" && break\n");
    s = code_string_concat(s, "  done\n");
    s = code_string_concat(s, "fi\n");
    s = code_string_concat(s, "if [ -z \"$RUNTIME\" ] || [ ! -d \"$RUNTIME/runtime\" ]; then\n");
    s = code_string_concat(s, "  AMC_BIN=$(command -v amc 2>/dev/null || true)\n");
    s = code_string_concat(s, "  if [ -n \"$AMC_BIN\" ]; then\n");
    s = code_string_concat(s, "    cand=$(dirname \"$AMC_BIN\")\n");
    s = code_string_concat(s, "    [ -d \"$cand/runtime\" ] && RUNTIME=\"$cand\"\n");
    s = code_string_concat(s, "  fi\n");
    s = code_string_concat(s, "fi\n");
    s = code_string_concat(s, "if [ -z \"$RUNTIME\" ] || [ ! -d \"$RUNTIME/runtime\" ]; then\n");
    s = code_string_concat(s, "  echo \"build.sh: runtime/ not found. Set AMALGAME_HOME=<dir> (the dir\"\n");
    s = code_string_concat(s, "  echo \"          containing runtime/), or install Amalgame globally.\"\n");
    s = code_string_concat(s, "  exit 1\n");
    s = code_string_concat(s, "fi\n");
    s = code_string_concat(s, "\n");
    s = code_string_concat(code_string_concat(code_string_concat(s, "amc src/main.am -o "), name), "\n");
    s = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(s, "gcc -O2 -I\"$RUNTIME/runtime\" "), name), ".c -lgc -lm -lcurl -o "), name), "\n");
    s = code_string_concat(code_string_concat(code_string_concat(s, "echo \"built ./"), name), "\"\n");
    return s;
}

static code_string Amalgame_Compiler_NewCommand_ReadmeExe(code_string name) {
    (void)name;
    code_string __attribute__((unused)) s = "";
    s = code_string_concat(code_string_concat(code_string_concat(s, "# "), name), "\n");
    s = code_string_concat(s, "\n");
    s = code_string_concat(code_string_concat(code_string_concat(s, "Amalgame project scaffolded by `amc new "), name), "`.\n");
    s = code_string_concat(s, "\n");
    s = code_string_concat(s, "## Build & run\n");
    s = code_string_concat(s, "\n");
    s = code_string_concat(s, "```\n");
    s = code_string_concat(s, "./build.sh\n");
    s = code_string_concat(code_string_concat(code_string_concat(s, "./"), name), "\n");
    s = code_string_concat(s, "```\n");
    s = code_string_concat(s, "\n");
    s = code_string_concat(s, "## Test\n");
    s = code_string_concat(s, "\n");
    s = code_string_concat(s, "```\n");
    s = code_string_concat(s, "amc test tests/\n");
    s = code_string_concat(s, "```\n");
    return s;
}

static code_string Amalgame_Compiler_NewCommand_LibAm(code_string name) {
    (void)name;
    code_string __attribute__((unused)) s = "";
    s = code_string_concat(code_string_concat(code_string_concat(s, "namespace "), name), "\n");
    s = code_string_concat(s, "\n");
    s = code_string_concat(s, "import Amalgame.String\n");
    s = code_string_concat(s, "\n");
    s = code_string_concat(s, "// [Library]\n");
    s = code_string_concat(s, "// Marker comment is read by `amc --lib` to skip the missing-Main check.\n");
    s = code_string_concat(s, "// See https://github.com/BastienMOUGET/Amalgame for the reference.\n");
    s = code_string_concat(s, "\n");
    s = code_string_concat(code_string_concat(code_string_concat(s, "public class "), Amalgame_Compiler_NewCommand_Capitalize(name)), " {\n");
    s = code_string_concat(code_string_concat(code_string_concat(s, "    public "), Amalgame_Compiler_NewCommand_Capitalize(name)), "() {}\n");
    s = code_string_concat(s, "\n");
    s = code_string_concat(s, "    public string Greet(string who) {\n");
    s = code_string_concat(code_string_concat(code_string_concat(s, "        return \"Hello, \" + who + \", from "), name), "!\"\n");
    s = code_string_concat(s, "    }\n");
    s = code_string_concat(s, "}\n");
    return s;
}

static code_string Amalgame_Compiler_NewCommand_BuildShLib(code_string name) {
    (void)name;
    code_string __attribute__((unused)) s = "";
    s = code_string_concat(s, "#!/bin/bash\n");
    s = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(s, "# Build script for the "), name), " library. Emits "), name), ".c\n");
    s = code_string_concat(s, "# (a self-contained Amalgame translation) that callers compile\n");
    s = code_string_concat(s, "# alongside their own sources via gcc.\n");
    s = code_string_concat(s, "set -e\n");
    s = code_string_concat(s, "cd \"$(dirname \"$0\")\"\n");
    s = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(s, "amc --lib src/"), name), ".am -o "), name), "\n");
    s = code_string_concat(code_string_concat(code_string_concat(s, "echo \"built ./"), name), ".c\"\n");
    return s;
}

static code_string Amalgame_Compiler_NewCommand_ReadmeLib(code_string name) {
    (void)name;
    code_string __attribute__((unused)) s = "";
    s = code_string_concat(code_string_concat(code_string_concat(s, "# "), name), "\n");
    s = code_string_concat(s, "\n");
    s = code_string_concat(code_string_concat(code_string_concat(s, "Amalgame library scaffolded by `amc new "), name), " --template lib`.\n");
    s = code_string_concat(s, "\n");
    s = code_string_concat(s, "## Build\n");
    s = code_string_concat(s, "\n");
    s = code_string_concat(s, "```\n");
    s = code_string_concat(code_string_concat(code_string_concat(s, "./build.sh   # produces ./"), name), ".o\n");
    s = code_string_concat(s, "```\n");
    s = code_string_concat(s, "\n");
    s = code_string_concat(s, "## Use from a host binary\n");
    s = code_string_concat(s, "\n");
    s = code_string_concat(code_string_concat(code_string_concat(s, "Compile your host with `amc src/main.am src/"), name), ".am -o app`,\n");
    s = code_string_concat(s, "or pre-build the .o and link it manually with `gcc`.\n");
    return s;
}

static code_string Amalgame_Compiler_NewCommand_TestAmTest(code_string name) {
    (void)name;
    code_string __attribute__((unused)) s = "";
    s = code_string_concat(code_string_concat(code_string_concat(s, "// Test bundle for "), name), ". Run with `amc test tests/`.\n");
    s = code_string_concat(s, "\n");
    s = code_string_concat(s, "import Amalgame.IO\n");
    s = code_string_concat(s, "\n");
    s = code_string_concat(s, "class Program {\n");
    s = code_string_concat(s, "    public static void Main() {\n");
    s = code_string_concat(s, "        let n: int = 2 + 3\n");
    s = code_string_concat(s, "        if (n == 5) {\n");
    s = code_string_concat(code_string_concat(code_string_concat(s, "            Console.WriteLine(\"[PASS] "), name), ": baseline\")\n");
    s = code_string_concat(s, "        } else {\n");
    s = code_string_concat(code_string_concat(code_string_concat(s, "            Console.WriteLine(\"[FAIL] "), name), ": baseline\")\n");
    s = code_string_concat(s, "        }\n");
    s = code_string_concat(s, "    }\n");
    s = code_string_concat(s, "}\n");
    return s;
}

static code_string Amalgame_Compiler_NewCommand_ReadmeTest(code_string name) {
    (void)name;
    code_string __attribute__((unused)) s = "";
    s = code_string_concat(code_string_concat(code_string_concat(s, "# "), name), " — tests\n");
    s = code_string_concat(s, "\n");
    s = code_string_concat(code_string_concat(code_string_concat(s, "Test bundle scaffolded by `amc new "), name), " --template test`.\n");
    s = code_string_concat(s, "\n");
    s = code_string_concat(s, "## Run\n");
    s = code_string_concat(s, "\n");
    s = code_string_concat(s, "```\n");
    s = code_string_concat(s, "amc test tests/\n");
    s = code_string_concat(s, "```\n");
    return s;
}

static code_string Amalgame_Compiler_NewCommand_GitignoreCommon() {
    code_string __attribute__((unused)) s = "";
    s = code_string_concat(s, "# Build artifacts\n");
    s = code_string_concat(s, "*.o\n");
    s = code_string_concat(s, "*.c.bundle\n");
    s = code_string_concat(s, "a.out\n");
    s = code_string_concat(s, "\n");
    s = code_string_concat(s, "# Editor state\n");
    s = code_string_concat(s, ".vscode/\n");
    s = code_string_concat(s, ".idea/\n");
    s = code_string_concat(s, "*.swp\n");
    return s;
}

static code_bool Amalgame_Compiler_NewCommand_WriteFile(code_string path, code_string content) {
    (void)path;
    (void)content;
    code_bool __attribute__((unused)) ok = File_WriteAll(path, content);
    if (!ok) {
        Console_WriteError(code_string_concat(code_string_concat("amc new: failed to write '", path), "'"));
        return 0;
    }
    return 1;
}

static code_string Amalgame_Compiler_NewCommand_ShellEscape(code_string s) {
    (void)s;
    i64 __attribute__((unused)) n = String_Length(s);
    code_string __attribute__((unused)) out = "'";
    for (i64 i = 0; i < n; i++) {
        code_string __attribute__((unused)) c = String_CharAt1(s, i);
        if (code_string_equals(c, "'")) {
            out = code_string_concat(out, "'\\''");
        } else {
            out = code_string_concat(out, c);
        }
    }
    out = code_string_concat(out, "'");
    return out;
}

static code_bool Amalgame_Compiler_NewCommand_IsSafeName(code_string s) {
    (void)s;
    i64 __attribute__((unused)) n = String_Length(s);
    if (n == 0) {
        return 0;
    }
    code_string __attribute__((unused)) allowed = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-";
    for (i64 i = 0; i < n; i++) {
        code_string __attribute__((unused)) c = String_CharAt1(s, i);
        if (String_IndexOf(allowed, c) < 0) {
            return 0;
        }
    }
    if (String_StartsWith(s, "-")) {
        return 0;
    }
    return 1;
}

static code_string Amalgame_Compiler_NewCommand_Capitalize(code_string s) {
    (void)s;
    if (String_Length(s) == 0) {
        return s;
    }
    code_string __attribute__((unused)) head = String_Substring(s, 0, 1);
    code_string __attribute__((unused)) tail = String_Substring(s, 1, String_Length(s) - 1);
    return code_string_concat(String_ToUpper(head), tail);
}

static code_string Amalgame_Compiler_NewCommand_Basename(code_string p) {
    (void)p;
    i64 __attribute__((unused)) n = String_Length(p);
    if (n == 0) {
        return p;
    }
    i64 __attribute__((unused)) i = n - 1;
    while (i >= 0) {
        code_string __attribute__((unused)) c = String_Substring(p, i, 1);
        if (code_string_equals(c, "/")) {
            return String_Substring(p, i + 1, n - i - 1);
        }
        i = i - 1;
    }
    return p;
}

struct _Amalgame_Compiler_AmalgameCompiler {
    Amalgame_Compiler_DiagnosticFormatter* Diag;
    code_bool IsLib;
    code_bool CheckOnly;
    code_bool LintMode;
    code_bool Verbose;
    i64 ExitCode;
};

void Amalgame_Compiler_AmalgameCompiler_SetLib(Amalgame_Compiler_AmalgameCompiler* self, code_bool v);
void Amalgame_Compiler_AmalgameCompiler_SetCheckOnly(Amalgame_Compiler_AmalgameCompiler* self, code_bool v);
void Amalgame_Compiler_AmalgameCompiler_SetLintMode(Amalgame_Compiler_AmalgameCompiler* self, code_bool v);
void Amalgame_Compiler_AmalgameCompiler_SetVerbose(Amalgame_Compiler_AmalgameCompiler* self, code_bool v);
void Amalgame_Compiler_AmalgameCompiler_SetColor(Amalgame_Compiler_AmalgameCompiler* self, code_bool v);
i64 Amalgame_Compiler_AmalgameCompiler_GetExitCode(Amalgame_Compiler_AmalgameCompiler* self);
void Amalgame_Compiler_AmalgameCompiler_Run(Amalgame_Compiler_AmalgameCompiler* self, AmalgameList* inputFiles, code_string outputName);

Amalgame_Compiler_AmalgameCompiler* Amalgame_Compiler_AmalgameCompiler_new() {
    Amalgame_Compiler_AmalgameCompiler* self = (Amalgame_Compiler_AmalgameCompiler*) GC_MALLOC(sizeof(Amalgame_Compiler_AmalgameCompiler));
    self->Diag = Amalgame_Compiler_DiagnosticFormatter_new();
    self->IsLib = 0;
    self->CheckOnly = 0;
    self->LintMode = 0;
    self->Verbose = 0;
    self->ExitCode = 0;
    return self;
}

void Amalgame_Compiler_AmalgameCompiler_SetLib(Amalgame_Compiler_AmalgameCompiler* self, code_bool v) {
    (void)self;
    (void)v;
    self->IsLib = v;
}

void Amalgame_Compiler_AmalgameCompiler_SetCheckOnly(Amalgame_Compiler_AmalgameCompiler* self, code_bool v) {
    (void)self;
    (void)v;
    self->CheckOnly = v;
}

void Amalgame_Compiler_AmalgameCompiler_SetLintMode(Amalgame_Compiler_AmalgameCompiler* self, code_bool v) {
    (void)self;
    (void)v;
    self->LintMode = v;
}

void Amalgame_Compiler_AmalgameCompiler_SetVerbose(Amalgame_Compiler_AmalgameCompiler* self, code_bool v) {
    (void)self;
    (void)v;
    self->Verbose = v;
}

void Amalgame_Compiler_AmalgameCompiler_SetColor(Amalgame_Compiler_AmalgameCompiler* self, code_bool v) {
    (void)self;
    (void)v;
    Amalgame_Compiler_DiagnosticFormatter_EnableColor(self->Diag, v);
}

i64 Amalgame_Compiler_AmalgameCompiler_GetExitCode(Amalgame_Compiler_AmalgameCompiler* self) {
    (void)self;
    return self->ExitCode;
}

void Amalgame_Compiler_AmalgameCompiler_Run(Amalgame_Compiler_AmalgameCompiler* self, AmalgameList* inputFiles, code_string outputName) {
    (void)self;
    (void)inputFiles;
    (void)outputName;
    i64 __attribute__((unused)) inputCount = AmalgameList_count(inputFiles);
    if (inputCount == 0) {
        Console_WriteError("amc: no input .am files");
        self->ExitCode = 1;
        return;
    }
    if (self->Verbose) {
        Console_WriteLine(code_string_concat(code_string_concat("Compiling: ", String_FromInt(inputCount)), " file(s)"));
    }
    code_string __attribute__((unused)) firstPath = (code_string)AmalgameList_get(inputFiles, 0);
    code_string __attribute__((unused)) firstSrc = File_ReadAll(firstPath);
    code_string __attribute__((unused)) nsPrefix = "App";
    i64 __attribute__((unused)) nlIdx = String_IndexOf(firstSrc, "namespace ");
    if (nlIdx >= 0) {
        code_string __attribute__((unused)) nlRest = String_Substring(firstSrc, nlIdx + 10, 200);
        i64 __attribute__((unused)) nlEnd = String_IndexOf(nlRest, "\n");
        if (nlEnd > 0) {
            code_string __attribute__((unused)) rawNs = String_Substring(nlRest, 0, nlEnd);
            nsPrefix = String_Replace(String_Trim(rawNs), ".", "_");
        }
    }
    Amalgame_Compiler_CGen* __attribute__((unused)) gen = Amalgame_Compiler_CGen_new();
    Amalgame_Compiler_CGen_BeginMulti(gen, nsPrefix);
    AmalgameList* __attribute__((unused)) progs = AmalgameList_new();
    code_bool __attribute__((unused)) parseOk = 1;
    for (i64 i = 0; i < inputCount; i++) {
        code_string __attribute__((unused)) path = (code_string)AmalgameList_get(inputFiles, i);
        code_string __attribute__((unused)) src = File_ReadAll(path);
        Amalgame_Compiler_DiagnosticFormatter_LoadSource(self->Diag, path, src);
        Amalgame_Compiler_Lexer* __attribute__((unused)) lex = Amalgame_Compiler_Lexer_new(src, path);
        AmalgameList* __attribute__((unused)) toks = Amalgame_Compiler_Lexer_Tokenize(lex);
        Amalgame_Compiler_Parser* __attribute__((unused)) par = Amalgame_Compiler_Parser_new(toks);
        Amalgame_Compiler_AstNode* __attribute__((unused)) prog = Amalgame_Compiler_Parser_Parse(par);
        AmalgameList_add(progs, (void*)(intptr_t)(prog));
        Amalgame_Compiler_CGen_AddFilePass1(gen, prog);
    }
    Amalgame_Compiler_FullResolver* __attribute__((unused)) resolver = Amalgame_Compiler_FullResolver_new();
    resolver->Sources = self->Diag->Sources;
    i64 __attribute__((unused)) progCount = AmalgameList_count(progs);
    for (i64 ri = 0; ri < progCount; ri++) {
        AmalgameList_add(resolver->Programs, (void*)(intptr_t)((Amalgame_Compiler_AstNode*)AmalgameList_get(progs, ri)));
    }
    Amalgame_Compiler_FullResolver_ResolvePrograms(resolver);
    if (Amalgame_Compiler_FullResolver_HasErrors(resolver)) {
        Console_WriteError(Amalgame_Compiler_FullResolver_GetErrors(resolver));
        self->ExitCode = 1;
        if (!self->CheckOnly) {
        }
    }
    Amalgame_Compiler_TypeChecker* __attribute__((unused)) tc = Amalgame_Compiler_TypeChecker_new(resolver, firstPath);
    tc->Sources = self->Diag->Sources;
    for (i64 ti = 0; ti < progCount; ti++) {
        Amalgame_Compiler_TypeChecker_Check(tc, (Amalgame_Compiler_AstNode*)AmalgameList_get(progs, ti));
    }
    if (Amalgame_Compiler_TypeChecker_HasErrors(tc)) {
        Console_WriteError(Amalgame_Compiler_TypeChecker_FormatErrors(tc));
        self->ExitCode = 1;
    }
    if (self->LintMode) {
        Amalgame_Compiler_Linter* __attribute__((unused)) linter = Amalgame_Compiler_Linter_new();
        for (i64 li = 0; li < progCount; li++) {
            Amalgame_Compiler_Linter_Lint(linter, (Amalgame_Compiler_AstNode*)AmalgameList_get(progs, li));
        }
        if (Amalgame_Compiler_Linter_HasWarnings(linter)) {
            Console_WriteError(Amalgame_Compiler_Linter_FormatWarnings(linter));
        }
    }
    if (self->CheckOnly) {
        if (self->ExitCode == 0) {
            Amalgame_Compiler_DiagnosticFormatter_PrintPhaseOk(self->Diag, "Check");
        }
        return;
    }
    Amalgame_Compiler_CGen_EmitSeparator(gen);
    for (i64 j = 0; j < inputCount; j++) {
        Amalgame_Compiler_CGen_AddFilePass2(gen, (Amalgame_Compiler_AstNode*)AmalgameList_get(progs, j));
    }
    AmalgameList* __attribute__((unused)) lines = Amalgame_Compiler_CGen_GetLines(gen);
    i64 __attribute__((unused)) lineCount = AmalgameList_count(lines);
    code_string __attribute__((unused)) outC = code_string_concat(outputName, ".c");
    File_WriteAll(outC, "");
    for (i64 k = 0; k < lineCount; k++) {
        File_AppendAll(outC, code_string_concat((code_string)((code_string)AmalgameList_get(lines, k)), "\n"));
    }
    code_string __attribute__((unused)) mainFunc = code_string_concat(nsPrefix, "_Program_Main");
    code_string __attribute__((unused)) genSrc = File_ReadAll(outC);
    code_bool __attribute__((unused)) hasMain = String_Contains(genSrc, mainFunc);
    code_bool __attribute__((unused)) isLibrary = self->IsLib || !hasMain;
    if (!isLibrary) {
        File_AppendAll(outC, "\nint main(int argc, char** argv) {\n");
        File_AppendAll(outC, "    GC_INIT();\n");
        File_AppendAll(outC, "    code_runtime_init_args(argc, argv);\n");
        File_AppendAll(outC, code_string_concat(code_string_concat("    ", mainFunc), "((code_string*)argv);\n"));
        File_AppendAll(outC, "    return code_exit_code;\n");
        File_AppendAll(outC, "}\n");
    } else {
        File_AppendAll(outC, "\n/* Library — no entry point */\n");
    }
    code_string __attribute__((unused)) mode = (isLibrary ? "Library" : "Executable");
    Console_WriteLine(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("Generated: ", outC), " ("), String_FromInt(lineCount)), " lines) ["), mode), "]"));
    Amalgame_Compiler_DiagnosticFormatter_PrintCompileOk(self->Diag, "Build");
}

struct _Amalgame_Compiler_Program {
};

void Amalgame_Compiler_Program_PrintUsage();
i64 Amalgame_Compiler_Program_RunTest(i64 argc);
i64 Amalgame_Compiler_Program_RunFmt(i64 argc);
void Amalgame_Compiler_Program_Main(code_string* args);

Amalgame_Compiler_Program* Amalgame_Compiler_Program_new() {
    Amalgame_Compiler_Program* self = (Amalgame_Compiler_Program*) GC_MALLOC(sizeof(Amalgame_Compiler_Program));
    return self;
}

void Amalgame_Compiler_Program_PrintUsage() {
    Console_WriteError("Usage: amc [options] file1.am [file2.am ...] -o <output>");
    Console_WriteError("       amc fmt [-w] file.am [file.am ...]");
    Console_WriteError("");
    Console_WriteError("Options:");
    Console_WriteError("  -o <output>   Output file (default: a.out)");
    Console_WriteError("  --lib         Compile as library (no main() emitted)");
    Console_WriteError("  --check       Type-check only, no code generation");
    Console_WriteError("  --lint        Run static-analysis warnings (in addition to compile)");
    Console_WriteError("  --color       Force ANSI color output");
    Console_WriteError("  --no-color    Disable ANSI color output");
    Console_WriteError("  --quiet       Suppress progress messages");
    Console_WriteError("  --verbose     Print extra build info");
    Console_WriteError("  --version     Print version and exit");
    Console_WriteError("  --help        Print this help");
    Console_WriteError("");
    Console_WriteError("Subcommands:");
    Console_WriteError("  fmt           Format Amalgame source. Default: print to stdout.");
    Console_WriteError("                With -w, rewrite files in place.");
    Console_WriteError("  test [<dir>]  Discover *_test.am, compile + run each, aggregate");
    Console_WriteError("                [PASS]/[FAIL]/[SKIP] lines from their stdout.");
    Console_WriteError("  lsp           Run a minimal LSP server (stdio JSON-RPC).");
    Console_WriteError("                v1 publishes diagnostics on didOpen/didChange.");
    Console_WriteError("  migrate <f>   Migrate a source file or directory to Amalgame via LLM.");
    Console_WriteError("                Auto-uses claude-api when ANTHROPIC_API_KEY is set, else");
    Console_WriteError("                shells out to the local `claude` CLI. See `amc migrate --help`.");
    Console_WriteError("  generate <p>  Generate an Amalgame program from a natural-language prompt.");
    Console_WriteError("                Same provider auto-selection as migrate. See `amc generate --help`.");
    Console_WriteError("  explain <f>   Read an Amalgame file and emit a natural-language explanation.");
    Console_WriteError("                See `amc explain --help`.");
    Console_WriteError("  new <name>    Scaffold a new Amalgame project (exe / lib / test templates).");
    Console_WriteError("                See `amc new --help`.");
}

i64 Amalgame_Compiler_Program_RunTest(i64 argc) {
    (void)argc;
    code_string __attribute__((unused)) dir = ".";
    i64 __attribute__((unused)) i = 2;
    while (i < argc) {
        code_string __attribute__((unused)) a = Args_Get(i);
        if (String_StartsWith(a, "-")) {
            Console_WriteError(code_string_concat(code_string_concat("amc test: unknown option '", a), "'"));
            return 1;
        }
        dir = a;
        i = i + 1;
    }
    code_string __attribute__((unused)) findCmd = code_string_concat(code_string_concat("find ", dir), " -name '*_test.am' -type f");
    AmalgameProcessResult* __attribute__((unused)) discovered = Process_RunCapture(findCmd);
    if (discovered->Exit != 0) {
        Console_WriteError(code_string_concat("amc test: failed to enumerate tests in ", dir));
        Console_WriteError(discovered->Stdout);
        return 1;
    }
    AmalgameList* __attribute__((unused)) lines = String_Split(String_Trim(discovered->Stdout), "\n");
    i64 __attribute__((unused)) nLines = AmalgameList_count(lines);
    if (nLines == 0) {
        Console_WriteLine(code_string_concat("No *_test.am files found under ", dir));
        return 0;
    }
    if (nLines == 1) {
        code_string __attribute__((unused)) only = String_Trim((code_string)AmalgameList_get(lines, 0));
        if (String_Length(only) == 0) {
            Console_WriteLine(code_string_concat("No *_test.am files found under ", dir));
            return 0;
        }
    }
    code_string __attribute__((unused)) amcPath = Args_Get(0);
    i64 __attribute__((unused)) pass = 0;
    i64 __attribute__((unused)) fail = 0;
    i64 __attribute__((unused)) skip = 0;
    i64 __attribute__((unused)) compileFail = 0;
    for (i64 li = 0; li < nLines; li++) {
        code_string __attribute__((unused)) path = String_Trim((code_string)AmalgameList_get(lines, li));
        if (String_Length(path) == 0) {
            continue;
        }
        Console_WriteLine(code_string_concat("── ", path));
        code_string __attribute__((unused)) outBin = code_string_concat("/tmp/amc_test_", String_FromInt(li));
        code_string __attribute__((unused)) outC = code_string_concat(outBin, ".c");
        code_string __attribute__((unused)) amcCmd = code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat(amcPath, " "), path), " -o "), outBin), " --quiet");
        AmalgameProcessResult* __attribute__((unused)) cr = Process_RunCapture(amcCmd);
        if (cr->Exit != 0) {
            Console_WriteLine("  [COMPILE-FAIL]");
            Console_Write(cr->Stdout);
            compileFail = compileFail + 1;
            continue;
        }
        code_string __attribute__((unused)) gccCmd = code_string_concat(code_string_concat(code_string_concat(code_string_concat("gcc -O2 -Iruntime ", outC), " -lgc -lm -lcurl -o "), outBin), " 2>&1");
        AmalgameProcessResult* __attribute__((unused)) gcc = Process_RunCapture(gccCmd);
        if (gcc->Exit != 0) {
            Console_WriteLine("  [LINK-FAIL]");
            Console_Write(gcc->Stdout);
            compileFail = compileFail + 1;
            continue;
        }
        AmalgameProcessResult* __attribute__((unused)) rr = Process_RunCapture(outBin);
        AmalgameList* __attribute__((unused)) outLines = String_Split(rr->Stdout, "\n");
        i64 __attribute__((unused)) on = AmalgameList_count(outLines);
        for (i64 ln = 0; ln < on; ln++) {
            code_string __attribute__((unused)) lineStr = (code_string)AmalgameList_get(outLines, ln);
            if (String_StartsWith(lineStr, "[PASS]")) {
                Console_WriteLine(code_string_concat("  ", lineStr));
                pass = pass + 1;
            } else if (String_StartsWith(lineStr, "[FAIL]")) {
                Console_WriteLine(code_string_concat("  ", lineStr));
                fail = fail + 1;
            } else if (String_StartsWith(lineStr, "[SKIP]")) {
                Console_WriteLine(code_string_concat("  ", lineStr));
                skip = skip + 1;
            }
        }
        if (rr->Exit != 0 && pass == 0 && fail == 0 && skip == 0) {
            Console_WriteLine(code_string_concat("  [FAIL] <crash> exit=", String_FromInt(rr->Exit)));
            fail = fail + 1;
        }
    }
    Console_WriteLine("");
    Console_WriteLine("──────────────────────────────────");
    Console_WriteLine(code_string_concat(code_string_concat(code_string_concat(code_string_concat(code_string_concat("PASS: ", String_FromInt(pass)), "  FAIL: "), String_FromInt(fail)), "  SKIP: "), String_FromInt(skip)));
    if (compileFail > 0) {
        Console_WriteLine(code_string_concat("COMPILE-FAIL: ", String_FromInt(compileFail)));
        return 1;
    }
    if (fail > 0) {
        return 1;
    }
    return 0;
}

i64 Amalgame_Compiler_Program_RunFmt(i64 argc) {
    (void)argc;
    code_bool __attribute__((unused)) write = 0;
    AmalgameList* __attribute__((unused)) files = AmalgameList_new();
    i64 __attribute__((unused)) i = 2;
    while (i < argc) {
        code_string __attribute__((unused)) a = Args_Get(i);
        if (code_string_equals(a, "-w") || code_string_equals(a, "--write")) {
            write = 1;
        } else if (String_EndsWith(a, ".am")) {
            AmalgameList_add(files, (void*)(intptr_t)(a));
        } else {
            Console_WriteError(code_string_concat(code_string_concat("amc fmt: unknown argument '", a), "'"));
            return 1;
        }
        i = i + 1;
    }
    if (AmalgameList_count(files) == 0) {
        Console_WriteError("amc fmt: no input .am files");
        return 1;
    }
    i64 __attribute__((unused)) n = AmalgameList_count(files);
    for (i64 j = 0; j < n; j++) {
        code_string __attribute__((unused)) path = (code_string)AmalgameList_get(files, j);
        code_string __attribute__((unused)) src = File_ReadAll(path);
        Amalgame_Compiler_Lexer* __attribute__((unused)) lex = Amalgame_Compiler_Lexer_new(src, path);
        AmalgameList* __attribute__((unused)) toks = Amalgame_Compiler_Lexer_Tokenize(lex);
        Amalgame_Compiler_Parser* __attribute__((unused)) par = Amalgame_Compiler_Parser_new(toks);
        Amalgame_Compiler_AstNode* __attribute__((unused)) prog = Amalgame_Compiler_Parser_Parse(par);
        if (Amalgame_Compiler_Parser_HasErrors(par)) {
            Console_WriteError(code_string_concat("amc fmt: parse errors in ", path));
            Console_WriteError(Amalgame_Compiler_Parser_GetErrors(par));
            return 1;
        }
        Amalgame_Compiler_Formatter* __attribute__((unused)) fmt = Amalgame_Compiler_Formatter_new(par->Comments);
        code_string __attribute__((unused)) out = Amalgame_Compiler_Formatter_Format(fmt, prog);
        if (write) {
            File_WriteAll(path, out);
        } else {
            Console_Write(out);
        }
    }
    return 0;
}

void Amalgame_Compiler_Program_Main(code_string* args) {
    (void)args;
    i64 __attribute__((unused)) argc = Args_Count();
    if (argc < 2) {
        Amalgame_Compiler_Program_PrintUsage();
        Exit_Set(1);
        return;
    }
    if (code_string_equals(Args_Get(1), "fmt")) {
        Exit_Set(Amalgame_Compiler_Program_RunFmt(argc));
        return;
    }
    if (code_string_equals(Args_Get(1), "test")) {
        Exit_Set(Amalgame_Compiler_Program_RunTest(argc));
        return;
    }
    if (code_string_equals(Args_Get(1), "lsp")) {
        Amalgame_Compiler_LspServer* __attribute__((unused)) server = Amalgame_Compiler_LspServer_new();
        Exit_Set(Amalgame_Compiler_LspServer_Run(server));
        return;
    }
    if (code_string_equals(Args_Get(1), "migrate")) {
        Exit_Set(Amalgame_Compiler_MigrateCommand_Run(argc));
        return;
    }
    if (code_string_equals(Args_Get(1), "generate")) {
        Exit_Set(Amalgame_Compiler_GenerateCommand_Run(argc));
        return;
    }
    if (code_string_equals(Args_Get(1), "explain")) {
        Exit_Set(Amalgame_Compiler_ExplainCommand_Run(argc));
        return;
    }
    if (code_string_equals(Args_Get(1), "new")) {
        Exit_Set(Amalgame_Compiler_NewCommand_Run(argc));
        return;
    }
    AmalgameList* __attribute__((unused)) inputFiles = AmalgameList_new();
    code_string __attribute__((unused)) outputName = "a.out";
    code_bool __attribute__((unused)) isLib = 0;
    code_bool __attribute__((unused)) checkOnly = 0;
    code_bool __attribute__((unused)) lintMode = 0;
    code_bool __attribute__((unused)) useColor = 0;
    code_bool __attribute__((unused)) verbose = 1;
    i64 __attribute__((unused)) i = 1;
    while (i < argc) {
        code_string __attribute__((unused)) a = Args_Get(i);
        if (code_string_equals(a, "-o") && i + 1 < argc) {
            i = i + 1;
            outputName = Args_Get(i);
        } else if (code_string_equals(a, "--lib")) {
            isLib = 1;
        } else if (code_string_equals(a, "--check")) {
            checkOnly = 1;
        } else if (code_string_equals(a, "--lint")) {
            lintMode = 1;
        } else if (code_string_equals(a, "--color")) {
            useColor = 1;
        } else if (code_string_equals(a, "--no-color")) {
            useColor = 0;
        } else if (code_string_equals(a, "--quiet")) {
            verbose = 0;
        } else if (code_string_equals(a, "--verbose")) {
            verbose = 1;
        } else if (code_string_equals(a, "--version")) {
            Console_WriteLine("amc 0.4.1 (self-hosted Amalgame compiler)");
            Exit_Set(0);
            return;
        } else if (code_string_equals(a, "--help") || code_string_equals(a, "-h")) {
            Amalgame_Compiler_Program_PrintUsage();
            Exit_Set(0);
            return;
        } else if (String_EndsWith(a, ".am")) {
            AmalgameList_add(inputFiles, (void*)(intptr_t)(a));
        } else {
            Console_WriteError(code_string_concat(code_string_concat("amc: unknown option '", a), "'"));
            Amalgame_Compiler_Program_PrintUsage();
            Exit_Set(1);
            return;
        }
        i = i + 1;
    }
    if (AmalgameList_count(inputFiles) == 0) {
        Console_WriteError("amc: no input .am files");
        Exit_Set(1);
        return;
    }
    Amalgame_Compiler_AmalgameCompiler* __attribute__((unused)) compiler = Amalgame_Compiler_AmalgameCompiler_new();
    Amalgame_Compiler_AmalgameCompiler_SetLib(compiler, isLib);
    Amalgame_Compiler_AmalgameCompiler_SetCheckOnly(compiler, checkOnly);
    Amalgame_Compiler_AmalgameCompiler_SetLintMode(compiler, lintMode);
    Amalgame_Compiler_AmalgameCompiler_SetColor(compiler, useColor);
    Amalgame_Compiler_AmalgameCompiler_SetVerbose(compiler, verbose);
    Amalgame_Compiler_AmalgameCompiler_Run(compiler, inputFiles, outputName);
    Exit_Set(Amalgame_Compiler_AmalgameCompiler_GetExitCode(compiler));
}


int main(int argc, char** argv) {
    GC_INIT();
    code_runtime_init_args(argc, argv);
    Amalgame_Compiler_Program_Main((code_string*)argv);
    return code_exit_code;
}
