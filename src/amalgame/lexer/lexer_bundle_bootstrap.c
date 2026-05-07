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
typedef struct _Amalgame_Compiler_Lexer Amalgame_Compiler_Lexer;

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

