// ═══════════════════════════════════════════════════════
//  token.vala  -  Définition des tokens du langage CODE
// ═══════════════════════════════════════════════════════

namespace CodeTranspiler.Lexer {

    public enum TokenType {

        // ── Littéraux ──────────────────────────────────
        INTEGER,        // 42
        FLOAT,          // 3.14
        STRING,         // "hello"
        BOOL_TRUE,      // true
        BOOL_FALSE,     // false
        NULL,           // null

        // ── Identifiants ───────────────────────────────
        IDENTIFIER,     // myVar, MyClass, ...

        // ── Mots clés ──────────────────────────────────
        // Types
        KW_INT,         // int
        KW_FLOAT,       // float
        KW_DOUBLE,      // double
        KW_STRING,      // string
        KW_BOOL,        // bool
        KW_VOID,        // void
        KW_VAR,         // var
        KW_LET,         // let

        // OO
        KW_CLASS,       // class
        KW_INTERFACE,   // interface
        KW_STRUCT,      // struct
        KW_ENUM,        // enum
        KW_RECORD,      // record
        KW_DATA,        // data
        KW_TRAIT,       // trait
        KW_IMPL,        // impl
        KW_EXTENDS,     // extends
        KW_IMPLEMENTS,  // implements
        KW_NEW,         // new
        KW_THIS,        // this
        KW_STATIC,      // static
        KW_ABSTRACT,    // abstract
        KW_SEALED,      // sealed
        KW_OVERRIDE,    // override

        // Accès
        KW_PUBLIC,      // public
        KW_PRIVATE,     // private
        KW_PROTECTED,   // protected
        KW_INTERNAL,    // internal

        // Contrôle
        KW_IF,          // if
        KW_ELSE,        // else
        KW_MATCH,       // match
        KW_RETURN,      // return
        KW_WHILE,       // while
        KW_FOR,         // for
        KW_FOREACH,     // foreach
        KW_IN,          // in
        KW_BREAK,       // break
        KW_CONTINUE,    // continue
        KW_GUARD,       // guard

        // Fonctionnel
        KW_PURE,        // pure
        KW_ASYNC,       // async
        KW_AWAIT,       // await
        KW_GO,          // go
        KW_SELECT,      // select
        KW_CASE,        // case

        // Gestion erreurs
        KW_TRY,         // try
        KW_CATCH,       // catch
        KW_THROW,       // throw

        // Namespace
        KW_NAMESPACE,   // namespace
        KW_IMPORT,      // import

        // Mémoire
        KW_WEAK,        // weak

        // ── Opérateurs ─────────────────────────────────
        OP_PLUS,        // +
        OP_MINUS,       // -
        OP_STAR,        // *
        OP_SLASH,       // /
        OP_PERCENT,     // %
        OP_POWER,       // ^
        OP_EQ,          // =
        OP_EQEQ,        // ==
        OP_NEQ,         // !=
        OP_LT,          // <
        OP_GT,          // >
        OP_LTE,         // <=
        OP_GTE,         // >=
        OP_AND,         // &&
        OP_OR,          // ||
        OP_NOT,         // !
        OP_ARROW,       // =>
        OP_THIN_ARROW,  // ->
        OP_PIPE,        // |>
        OP_COMPOSE,     // >>
        OP_NULLCOAL,    // ??
        OP_NULLSAFE,    // ?.
        OP_RANGE,       // ..
        OP_SPREAD,      // ...
        OP_WITH,        // with

        // ── Délimiteurs ────────────────────────────────
        LBRACE,         // {
        RBRACE,         // }
        LPAREN,         // (
        RPAREN,         // )
        LBRACKET,       // [
        RBRACKET,       // ]
        SEMICOLON,      // ;  (optionnel)
        COLON,          // :
        COMMA,          // ,
        DOT,            // .
        AT,             // @  (décorateurs)
        QUESTION,       // ?  (nullable)

        // ── Spéciaux ───────────────────────────────────
        NEWLINE,
        EOF,
        UNKNOWN
    }


    // ─────────────────────────────────────────────────
    //  Structure Token
    // ─────────────────────────────────────────────────
    public class Token : Object {

        public TokenType Type     { get; construct; }
        public string    Value    { get; construct; }
        public int       Line     { get; construct; }
        public int       Column   { get; construct; }
        public string    Filename { get; construct; }

        public Token(TokenType type,
                     string    value,
                     int       line,
                     int       column,
                     string    filename = "<unknown>") {
            Object(
                Type:     type,
                Value:    value,
                Line:     line,
                Column:   column,
                Filename: filename
            );
        }

        public string ToString() {
            return "[%s] '%s' @ %s:%d:%d".printf(
                Type.to_string(),
                Value,
                Filename,
                Line,
                Column
            );
        }

        public bool IsKeyword() {
            return Type >= TokenType.KW_INT &&
                   Type <= TokenType.KW_WEAK;
        }

        public bool IsLiteral() {
            return Type == TokenType.INTEGER  ||
                   Type == TokenType.FLOAT    ||
                   Type == TokenType.STRING   ||
                   Type == TokenType.BOOL_TRUE ||
                   Type == TokenType.BOOL_FALSE ||
                   Type == TokenType.NULL;
        }

        public bool IsOperator() {
            return Type >= TokenType.OP_PLUS &&
                   Type <= TokenType.OP_WITH;
        }
    }
}
