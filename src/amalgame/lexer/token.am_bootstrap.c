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

