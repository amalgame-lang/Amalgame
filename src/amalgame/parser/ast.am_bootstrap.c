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

typedef enum _Amalgame_Compiler_NodeKind Amalgame_Compiler_NodeKind;
typedef struct _Amalgame_Compiler_AstNode Amalgame_Compiler_AstNode;
typedef struct _Amalgame_Compiler_Ast Amalgame_Compiler_Ast;

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

