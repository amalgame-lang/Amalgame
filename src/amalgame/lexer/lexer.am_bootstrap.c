#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "_runtime.h"

typedef struct _Amalgame_Compiler_Lexer Amalgame_Compiler_Lexer;

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
static void Amalgame_Compiler_Lexer_AddToken(Amalgame_Compiler_Lexer* self, Amalgame_Compiler_TokenType* t, code_string value);
static code_string Amalgame_Compiler_Lexer_Advance(Amalgame_Compiler_Lexer* self);
static void Amalgame_Compiler_Lexer_SkipWhitespace(Amalgame_Compiler_Lexer* self);
AmalgameList* Amalgame_Compiler_Lexer_Tokenize(Amalgame_Compiler_Lexer* self);
static void Amalgame_Compiler_Lexer_ReadString(Amalgame_Compiler_Lexer* self);
static void Amalgame_Compiler_Lexer_ReadNumber(Amalgame_Compiler_Lexer* self);
static void Amalgame_Compiler_Lexer_ReadIdentifier(Amalgame_Compiler_Lexer* self);
static Amalgame_Compiler_TokenType* Amalgame_Compiler_Lexer_LookupKeyword(Amalgame_Compiler_Lexer* self, code_string word);
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
    return code_string_equals(c, " ") || code_string_equals(c, "	") || code_string_equals(c, "");
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
    return Amalgame_Compiler_IsAlpha(c) || Amalgame_Compiler_IsDigit(c);
}

static code_string Amalgame_Compiler_Lexer_CharAt(Amalgame_Compiler_Lexer* self, i64 i) {
    (void)self;
    (void)i;
    if (i < 0 || i >= String_Length(self->Source)) {
        return "";
    }
    return String_Substring(self->Source, i, 1);
}

static void Amalgame_Compiler_Lexer_AddToken(Amalgame_Compiler_Lexer* self, Amalgame_Compiler_TokenType* t, code_string value) {
    (void)self;
    (void)t;
    (void)value;
    Amalgame_Compiler_Token* tok = Amalgame_Compiler_Token_new(t, value, self->Line, self->Column, self->Filename);
    AmalgameList_add(self->Tokens, (void*)(intptr_t)(tok));
}

static code_string Amalgame_Compiler_Lexer_Advance(Amalgame_Compiler_Lexer* self) {
    (void)self;
    void* ch = Amalgame_Compiler_CharAt(self->Pos);
    self->Pos = self->Pos + 1;
    self->Column = self->Column + 1;
    return ch;
}

static void Amalgame_Compiler_Lexer_SkipWhitespace(Amalgame_Compiler_Lexer* self) {
    (void)self;
    while (self->Pos < String_Length(self->Source)) {
        void* c = Amalgame_Compiler_CharAt(self->Pos);
        if (Amalgame_Compiler_IsSpace(c)) {
            Amalgame_Compiler_Advance();
        } else if (c == "/" && Amalgame_Compiler_CharAt(self->Pos + 1) == "/") {
            while (self->Pos < String_Length(self->Source) && Amalgame_Compiler_CharAt(self->Pos) != "
") {
                Amalgame_Compiler_Advance();
            }
        } else {
            break;
        }
    }
}

AmalgameList* Amalgame_Compiler_Lexer_Tokenize(Amalgame_Compiler_Lexer* self) {
    (void)self;
    while (self->Pos < String_Length(self->Source)) {
        Amalgame_Compiler_SkipWhitespace();
        if (self->Pos >= String_Length(self->Source)) {
            break;
        }
        void* ch = Amalgame_Compiler_CharAt(self->Pos);
        if (ch == "
") {
            Amalgame_Compiler_AddToken(Amalgame_Compiler_NEWLINE, "\n");
            self->Pos = self->Pos + 1;
            self->Line = self->Line + 1;
            self->Column = 1;
        } else if (ch == """) {
            Amalgame_Compiler_ReadString();
        } else if (Amalgame_Compiler_IsDigit(ch)) {
            Amalgame_Compiler_ReadNumber();
        } else if (Amalgame_Compiler_IsAlpha(ch)) {
            Amalgame_Compiler_ReadIdentifier();
        } else if (Amalgame_Compiler_IsSpace(ch)) {
            Amalgame_Compiler_Advance();
        } else {
            Amalgame_Compiler_ReadSymbol();
        }
    }
    Amalgame_Compiler_AddToken(Amalgame_Compiler_EOF, "");
    return self->Tokens;
}

static void Amalgame_Compiler_Lexer_ReadString(Amalgame_Compiler_Lexer* self) {
    (void)self;
    Amalgame_Compiler_Advance();
    code_string value = "";
    while (self->Pos < String_Length(self->Source) && Amalgame_Compiler_CharAt(self->Pos) != """) {
        void* c = Amalgame_Compiler_Advance();
        if (c == "\") {
            void* esc = Amalgame_Compiler_Advance();
            if (esc == "n") {
                value = value + "
";
            }
            if (esc == "t") {
                value = value + "	";
            }
            if (esc == """) {
                value = value + """;
            }
            if (esc == "\") {
                value = value + "\";
            }
        } else {
            value = value + c;
        }
    }
    if (self->Pos < String_Length(self->Source)) {
        Amalgame_Compiler_Advance();
    }
    Amalgame_Compiler_AddToken(Amalgame_Compiler_STRING, value);
}

static void Amalgame_Compiler_Lexer_ReadNumber(Amalgame_Compiler_Lexer* self) {
    (void)self;
    i64 startCol = self->Column;
    code_string value = "";
    code_bool isFloat = 0;
    while (self->Pos < String_Length(self->Source) && Amalgame_Compiler_IsDigit(Amalgame_Compiler_CharAt(self->Pos))) {
        value = value + Amalgame_Compiler_Advance();
    }
    if (self->Pos < String_Length(self->Source) && Amalgame_Compiler_CharAt(self->Pos) == "." && Amalgame_Compiler_IsDigit(Amalgame_Compiler_CharAt(self->Pos + 1))) {
        isFloat = 1;
        value = value + Amalgame_Compiler_Advance();
        while (self->Pos < String_Length(self->Source) && Amalgame_Compiler_IsDigit(Amalgame_Compiler_CharAt(self->Pos))) {
            value = value + Amalgame_Compiler_Advance();
        }
    }
    if (isFloat) {
        Amalgame_Compiler_Token* tok = Amalgame_Compiler_Token_new(Amalgame_Compiler_FLOAT, value, self->Line, startCol, self->Filename);
        AmalgameList_add(self->Tokens, (void*)(intptr_t)(tok));
    } else {
        Amalgame_Compiler_Token* tok = Amalgame_Compiler_Token_new(Amalgame_Compiler_INTEGER, value, self->Line, startCol, self->Filename);
        AmalgameList_add(self->Tokens, (void*)(intptr_t)(tok));
    }
}

static void Amalgame_Compiler_Lexer_ReadIdentifier(Amalgame_Compiler_Lexer* self) {
    (void)self;
    i64 startCol = self->Column;
    code_string value = "";
    while (self->Pos < String_Length(self->Source) && Amalgame_Compiler_IsAlphaNum(Amalgame_Compiler_CharAt(self->Pos))) {
        value = value + Amalgame_Compiler_Advance();
    }
    void* tt = Amalgame_Compiler_LookupKeyword(value);
    Amalgame_Compiler_Token* tok = Amalgame_Compiler_Token_new(tt, value, self->Line, startCol, self->Filename);
    AmalgameList_add(self->Tokens, (void*)(intptr_t)(tok));
}

static Amalgame_Compiler_TokenType* Amalgame_Compiler_Lexer_LookupKeyword(Amalgame_Compiler_Lexer* self, code_string word) {
    (void)self;
    (void)word;
    if (code_string_equals(word, "let")) {
        return Amalgame_Compiler_KW_LET;
    }
    if (code_string_equals(word, "var")) {
        return Amalgame_Compiler_KW_VAR;
    }
    if (code_string_equals(word, "if")) {
        return Amalgame_Compiler_KW_IF;
    }
    if (code_string_equals(word, "else")) {
        return Amalgame_Compiler_KW_ELSE;
    }
    if (code_string_equals(word, "for")) {
        return Amalgame_Compiler_KW_FOR;
    }
    if (code_string_equals(word, "while")) {
        return Amalgame_Compiler_KW_WHILE;
    }
    if (code_string_equals(word, "return")) {
        return Amalgame_Compiler_KW_RETURN;
    }
    if (code_string_equals(word, "class")) {
        return Amalgame_Compiler_KW_CLASS;
    }
    if (code_string_equals(word, "public")) {
        return Amalgame_Compiler_KW_PUBLIC;
    }
    if (code_string_equals(word, "private")) {
        return Amalgame_Compiler_KW_PRIVATE;
    }
    if (code_string_equals(word, "static")) {
        return Amalgame_Compiler_KW_STATIC;
    }
    if (code_string_equals(word, "new")) {
        return Amalgame_Compiler_KW_NEW;
    }
    if (code_string_equals(word, "this")) {
        return Amalgame_Compiler_KW_THIS;
    }
    if (code_string_equals(word, "null")) {
        return Amalgame_Compiler_KW_NULL;
    }
    if (code_string_equals(word, "true")) {
        return Amalgame_Compiler_KW_TRUE;
    }
    if (code_string_equals(word, "false")) {
        return Amalgame_Compiler_KW_FALSE;
    }
    if (code_string_equals(word, "import")) {
        return Amalgame_Compiler_KW_IMPORT;
    }
    if (code_string_equals(word, "namespace")) {
        return Amalgame_Compiler_KW_NAMESPACE;
    }
    if (code_string_equals(word, "enum")) {
        return Amalgame_Compiler_KW_ENUM;
    }
    if (code_string_equals(word, "match")) {
        return Amalgame_Compiler_KW_MATCH;
    }
    if (code_string_equals(word, "in")) {
        return Amalgame_Compiler_KW_IN;
    }
    if (code_string_equals(word, "is")) {
        return Amalgame_Compiler_KW_IS;
    }
    if (code_string_equals(word, "as")) {
        return Amalgame_Compiler_KW_AS;
    }
    if (code_string_equals(word, "void")) {
        return Amalgame_Compiler_KW_VOID;
    }
    if (code_string_equals(word, "int")) {
        return Amalgame_Compiler_KW_INT;
    }
    if (code_string_equals(word, "string")) {
        return Amalgame_Compiler_KW_STRING_TYPE;
    }
    if (code_string_equals(word, "bool")) {
        return Amalgame_Compiler_KW_BOOL_TYPE;
    }
    if (code_string_equals(word, "float")) {
        return Amalgame_Compiler_KW_FLOAT_TYPE;
    }
    if (code_string_equals(word, "try")) {
        return Amalgame_Compiler_KW_TRY;
    }
    if (code_string_equals(word, "catch")) {
        return Amalgame_Compiler_KW_CATCH;
    }
    if (code_string_equals(word, "throw")) {
        return Amalgame_Compiler_KW_THROW;
    }
    if (code_string_equals(word, "finally")) {
        return Amalgame_Compiler_KW_FINALLY;
    }
    if (code_string_equals(word, "interface")) {
        return Amalgame_Compiler_KW_INTERFACE;
    }
    if (code_string_equals(word, "record")) {
        return Amalgame_Compiler_KW_RECORD;
    }
    if (code_string_equals(word, "func")) {
        return Amalgame_Compiler_KW_FUNC;
    }
    return Amalgame_Compiler_IDENTIFIER;
}

static void Amalgame_Compiler_Lexer_ReadSymbol(Amalgame_Compiler_Lexer* self) {
    (void)self;
    i64 startCol = self->Column;
    void* c = Amalgame_Compiler_Advance();
    void* c2 = Amalgame_Compiler_CharAt(self->Pos);
    if (c == "+") {
        Amalgame_Compiler_AddToken(Amalgame_Compiler_OP_PLUS, "+");
    } else if (c == "%") {
        Amalgame_Compiler_AddToken(Amalgame_Compiler_OP_PERCENT, "%");
    } else if (c == "@") {
        Amalgame_Compiler_AddToken(Amalgame_Compiler_AT, "@");
    } else if (c == "(") {
        Amalgame_Compiler_AddToken(Amalgame_Compiler_LPAREN, "(");
    } else if (c == ")") {
        Amalgame_Compiler_AddToken(Amalgame_Compiler_RPAREN);
    } else if (c == "{") {
        Amalgame_Compiler_AddToken(Amalgame_Compiler_LBRACE, "{");
    } else if (c == "}") {
        Amalgame_Compiler_AddToken(Amalgame_Compiler_RBRACE, "}");
    } else if (c == "[") {
        Amalgame_Compiler_AddToken(Amalgame_Compiler_LBRACKET, "[");
    } else if (c == "]") {
        Amalgame_Compiler_AddToken(Amalgame_Compiler_RBRACKET, "]");
    } else if (c == ",") {
        Amalgame_Compiler_AddToken(Amalgame_Compiler_COMMA);
    } else if (c == ";") {
        Amalgame_Compiler_AddToken(Amalgame_Compiler_SEMICOLON, ";");
    } else if (c == "/") {
        Amalgame_Compiler_AddToken(Amalgame_Compiler_OP_SLASH, "/");
    } else if (c == -_unknown_) {
        if (c2 == ">") {
            Amalgame_Compiler_Advance();
            Amalgame_Compiler_AddToken(Amalgame_Compiler_OP_THIN_ARROW, "->");
        } else {
            Amalgame_Compiler_AddToken(Amalgame_Compiler_OP_MINUS, -_unknown_, _unknown_, _unknown_, _unknown_, _unknown_(c == "*"), _unknown_, Amalgame_Compiler_AddToken(Amalgame_Compiler_OP_STAR, "*"), _unknown_, _unknown_, _unknown_(c == "="), _unknown_, _unknown_(c2 == "="), _unknown_, Amalgame_Compiler_Advance(), Amalgame_Compiler_AddToken(Amalgame_Compiler_OP_EQEQ, "=="), _unknown_, _unknown_, _unknown_(c2 == ">"), _unknown_, Amalgame_Compiler_Advance(), Amalgame_Compiler_AddToken(Amalgame_Compiler_OP_ARROW, "=>"), _unknown_, _unknown_, _unknown_, Amalgame_Compiler_AddToken(Amalgame_Compiler_OP_EQ, "="), _unknown_, _unknown_, _unknown_, _unknown_(c == !_unknown_, _unknown_, _unknown_(c2 == "="), _unknown_, Amalgame_Compiler_Advance(), Amalgame_Compiler_AddToken(Amalgame_Compiler_OP_NEQ, "!="), _unknown_, _unknown_, _unknown_, Amalgame_Compiler_AddToken(Amalgame_Compiler_OP_NOT, !_unknown_, _unknown_, _unknown_, _unknown_, _unknown_(c == "<"), _unknown_, _unknown_(c2 == "="), _unknown_, Amalgame_Compiler_Advance(), Amalgame_Compiler_AddToken(Amalgame_Compiler_OP_LTE, "<="), _unknown_, _unknown_, _unknown_, Amalgame_Compiler_AddToken(Amalgame_Compiler_OP_LT, "<"), _unknown_, _unknown_, _unknown_, _unknown_(c == ">"), _unknown_, _unknown_(c2 == "="), _unknown_, Amalgame_Compiler_Advance(), Amalgame_Compiler_AddToken(Amalgame_Compiler_OP_GTE, ">="), _unknown_, _unknown_, _unknown_, Amalgame_Compiler_AddToken(Amalgame_Compiler_OP_GT, ">"), _unknown_, _unknown_, _unknown_, _unknown_(c == "&"), _unknown_, _unknown_(c2 == "&"), _unknown_, Amalgame_Compiler_Advance(), Amalgame_Compiler_AddToken(Amalgame_Compiler_OP_AND, "&&"), _unknown_, _unknown_, _unknown_, _unknown_(c == "|"), _unknown_, _unknown_(c2 == "|"), _unknown_, Amalgame_Compiler_Advance(), Amalgame_Compiler_AddToken(Amalgame_Compiler_OP_OR, "||"), _unknown_, _unknown_, _unknown_, _unknown_(c == "."), _unknown_, _unknown_(c2 == "."), _unknown_, Amalgame_Compiler_Advance(), Amalgame_Compiler_AddToken(Amalgame_Compiler_OP_DOTDOT, ".."), _unknown_, _unknown_, _unknown_, Amalgame_Compiler_AddToken(Amalgame_Compiler_OP_DOT, "."), _unknown_, _unknown_, _unknown_, _unknown_(c == ":"), _unknown_, _unknown_(c2 == ":"), _unknown_, Amalgame_Compiler_Advance(), Amalgame_Compiler_AddToken(Amalgame_Compiler_COLON, "::"), _unknown_, _unknown_, _unknown_, Amalgame_Compiler_AddToken(Amalgame_Compiler_COLON, ":"), _unknown_, _unknown_, _unknown_, _unknown_(c == "?"), _unknown_, _unknown_(c2 == "?"), _unknown_, Amalgame_Compiler_Advance(), Amalgame_Compiler_AddToken(Amalgame_Compiler_OP_COALESCE, "??"), _unknown_, _unknown_, _unknown_, _unknown_, Amalgame_Compiler_AddToken(Amalgame_Compiler_UNKNOWN, c), _unknown_, _unknown_, _unknown_)));
        }
    }
}

