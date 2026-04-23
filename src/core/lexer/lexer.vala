// ─────────────────────────────────────────────────────
//  CODE Programming Language
//  Copyright (c) 2024 NeitsabTeguom
//  Licensed under Apache 2.0
// ─────────────────────────────────────────────────────
// ═══════════════════════════════════════════════════════
//  lexer.vala  -  Lexer du langage CODE
// ═══════════════════════════════════════════════════════

namespace CodeTranspiler.Lexer {

    public class Lexer : Object {

        private string   _source;
        private int      _pos;
        private int      _line;
        private int      _column;
        private string   _filename;

        // Table des mots clés
        private static HashTable<string, TokenType> _keywords;

        // Initialisation statique des mots clés
        static construct {
            _keywords = new HashTable<string, TokenType>(str_hash, str_equal);

            // Types
            _keywords.insert("int",        TokenType.KW_INT);
            _keywords.insert("float",      TokenType.KW_FLOAT);
            _keywords.insert("double",     TokenType.KW_DOUBLE);
            _keywords.insert("string",     TokenType.KW_STRING);
            _keywords.insert("bool",       TokenType.KW_BOOL);
            _keywords.insert("void",       TokenType.KW_VOID);
            _keywords.insert("var",        TokenType.KW_VAR);
            _keywords.insert("let",        TokenType.KW_LET);

            // OO
            _keywords.insert("class",      TokenType.KW_CLASS);
            _keywords.insert("interface",  TokenType.KW_INTERFACE);
            _keywords.insert("struct",     TokenType.KW_STRUCT);
            _keywords.insert("enum",       TokenType.KW_ENUM);
            _keywords.insert("record",     TokenType.KW_RECORD);
            _keywords.insert("data",       TokenType.KW_DATA);
            _keywords.insert("trait",      TokenType.KW_TRAIT);
            _keywords.insert("extends",    TokenType.KW_EXTENDS);
            _keywords.insert("implements", TokenType.KW_IMPLEMENTS);
            _keywords.insert("new",        TokenType.KW_NEW);
            _keywords.insert("this",       TokenType.KW_THIS);
            _keywords.insert("static",     TokenType.KW_STATIC);
            _keywords.insert("abstract",   TokenType.KW_ABSTRACT);
            _keywords.insert("override",   TokenType.KW_OVERRIDE);

            // Accès
            _keywords.insert("public",     TokenType.KW_PUBLIC);
            _keywords.insert("private",    TokenType.KW_PRIVATE);
            _keywords.insert("protected",  TokenType.KW_PROTECTED);
            _keywords.insert("internal",   TokenType.KW_INTERNAL);

            // Contrôle
            _keywords.insert("if",         TokenType.KW_IF);
            _keywords.insert("else",       TokenType.KW_ELSE);
            _keywords.insert("match",      TokenType.KW_MATCH);
            _keywords.insert("return",     TokenType.KW_RETURN);
            _keywords.insert("while",      TokenType.KW_WHILE);
            _keywords.insert("for",        TokenType.KW_FOR);
            _keywords.insert("foreach",    TokenType.KW_FOREACH);
            _keywords.insert("in",         TokenType.KW_IN);
            _keywords.insert("break",      TokenType.KW_BREAK);
            _keywords.insert("continue",   TokenType.KW_CONTINUE);
            _keywords.insert("guard",      TokenType.KW_GUARD);

            // Fonctionnel
            _keywords.insert("pure",       TokenType.KW_PURE);
            _keywords.insert("async",      TokenType.KW_ASYNC);
            _keywords.insert("await",      TokenType.KW_AWAIT);
            _keywords.insert("go",         TokenType.KW_GO);

            // Erreurs
            _keywords.insert("try",        TokenType.KW_TRY);
            _keywords.insert("catch",      TokenType.KW_CATCH);
            _keywords.insert("throw",      TokenType.KW_THROW);

            // Namespace
            _keywords.insert("namespace",  TokenType.KW_NAMESPACE);
            _keywords.insert("import",     TokenType.KW_IMPORT);

            // Littéraux
            _keywords.insert("true",       TokenType.BOOL_TRUE);
            _keywords.insert("false",      TokenType.BOOL_FALSE);
            _keywords.insert("null",       TokenType.NULL);

            // Mémoire
            _keywords.insert("weak",       TokenType.KW_WEAK);
        }


        public Lexer(string source, string filename = "<unknown>") {
            _source   = source;
            _pos      = 0;
            _line     = 1;
            _column   = 1;
            _filename = filename;
        }


        // ─────────────────────────────────────────────
        //  Point d'entrée : tokenize tout le source
        // ─────────────────────────────────────────────
        public Gee.ArrayList<Token> Tokenize() {
            var tokens = new Gee.ArrayList<Token>();

            while (!IsEnd()) {
                SkipWhitespaceAndComments();
                if (IsEnd()) break;

                var token = NextToken();
                if (token != null) {
                    tokens.add(token);
                }
            }

            tokens.add(MakeToken(TokenType.EOF, ""));
            return tokens;
        }


        // ─────────────────────────────────────────────
        //  Lecture du prochain token
        // ─────────────────────────────────────────────
        private Token? NextToken() {
            int    startLine   = _line;
            int    startColumn = _column;
            char   c           = Current();

            // ── Nombres ────────────────────────────
            if (c.isdigit()) {
                return ReadNumber(startLine, startColumn);
            }

            // ── Identifiants & mots clés ───────────
            if (c.isalpha() || c == '_') {
                return ReadIdentifierOrKeyword(startLine, startColumn);
            }

            // ── Strings ────────────────────────────
            if (c == '"') {
                return ReadString(startLine, startColumn);
            }

            // ── Strings multi-lignes """ ────────────
            if (c == '"' && Peek(1) == '"' && Peek(2) == '"') {
                return ReadMultilineString(startLine, startColumn);
            }

            // ── Décorateurs @ ──────────────────────
            if (c == '@') {
                Advance();
                return new Token(TokenType.AT, "@",
                                 startLine, startColumn, _filename);
            }

            // ── Opérateurs & délimiteurs ───────────
            return ReadOperatorOrDelimiter(startLine, startColumn);
        }


        // ─────────────────────────────────────────────
        //  Lecture d'un nombre
        // ─────────────────────────────────────────────
        private Token ReadNumber(int line, int col) {
            var sb       = new StringBuilder();
            bool isFloat = false;

            while (!IsEnd() && (Current().isdigit() || Current() == '_')) {
                if (Current() != '_') sb.append_c(Current());
                Advance();
            }

            // Partie décimale
            if (!IsEnd() && Current() == '.' && Peek(1).isdigit()) {
                isFloat = true;
                sb.append_c('.');
                Advance();
                while (!IsEnd() && Current().isdigit()) {
                    sb.append_c(Current());
                    Advance();
                }
            }

            // Exposant (1e10, 3.14e-2)
            if (!IsEnd() && (Current() == 'e' || Current() == 'E')) {
                isFloat = true;
                sb.append_c(Current());
                Advance();
                if (!IsEnd() && (Current() == '+' || Current() == '-')) {
                    sb.append_c(Current());
                    Advance();
                }
                while (!IsEnd() && Current().isdigit()) {
                    sb.append_c(Current());
                    Advance();
                }
            }

            var type = isFloat ? TokenType.FLOAT : TokenType.INTEGER;
            return new Token(type, sb.str, line, col, _filename);
        }


        // ─────────────────────────────────────────────
        //  Lecture d'un identifiant ou mot clé
        // ─────────────────────────────────────────────
        private Token ReadIdentifierOrKeyword(int line, int col) {
            var sb = new StringBuilder();

            while (!IsEnd() && (Current().isalnum() || Current() == '_')) {
                sb.append_c(Current());
                Advance();
            }

            string word = sb.str;

            // Vérifie si c'est un mot clé
            TokenType? kwType = _keywords.lookup(word);
            var type = (kwType != null) ? kwType : TokenType.IDENTIFIER;

            return new Token(type, word, line, col, _filename);
        }


        // ─────────────────────────────────────────────
        //  Lecture d'une string avec interpolation
        // ─────────────────────────────────────────────
        private Token ReadString(int line, int col) {
            var sb = new StringBuilder();
            Advance(); // consume "

            while (!IsEnd() && Current() != '"') {
                if (Current() == '\\') {
                    Advance();
                    switch (Current()) {
                        case 'n':  sb.append_c('\n'); break;
                        case 't':  sb.append_c('\t'); break;
                        case '"':  sb.append_c('"');  break;
                        case '\\': sb.append_c('\\'); break;
                        default:
                            sb.append_c('\\');
                            sb.append_c(Current());
                            break;
                    }
                } else {
                    sb.append_c(Current());
                }
                Advance();
            }

            Advance(); // consume "
            return new Token(TokenType.STRING, sb.str, line, col, _filename);
        }


        // ─────────────────────────────────────────────
        //  Lecture d'une string multi-lignes """
        // ─────────────────────────────────────────────
        private Token ReadMultilineString(int line, int col) {
            var sb = new StringBuilder();
            Advance(); Advance(); Advance(); // consume """

            while (!IsEnd()) {
                if (Current() == '"' && Peek(1) == '"' && Peek(2) == '"') {
                    Advance(); Advance(); Advance(); // consume """
                    break;
                }
                sb.append_c(Current());
                if (Current() == '\n') { _line++; _column = 0; }
                Advance();
            }

            return new Token(TokenType.STRING, sb.str, line, col, _filename);
        }


        // ─────────────────────────────────────────────
        //  Lecture des opérateurs et délimiteurs
        // ─────────────────────────────────────────────
        private Token ReadOperatorOrDelimiter(int line, int col) {
            char c = Current();
            Advance();

            switch (c) {
                case '{': return new Token(TokenType.LBRACE,    "{", line, col, _filename);
                case '}': return new Token(TokenType.RBRACE,    "}", line, col, _filename);
                case '(': return new Token(TokenType.LPAREN,    "(", line, col, _filename);
                case ')': return new Token(TokenType.RPAREN,    ")", line, col, _filename);
                case '[': return new Token(TokenType.LBRACKET,  "[", line, col, _filename);
                case ']': return new Token(TokenType.RBRACKET,  "]", line, col, _filename);
                case ';': return new Token(TokenType.SEMICOLON, ";", line, col, _filename);
                case ':': return new Token(TokenType.COLON,     ":", line, col, _filename);
                case ',': return new Token(TokenType.COMMA,     ",", line, col, _filename);
                case '@': return new Token(TokenType.AT,        "@", line, col, _filename);

                case '.':
                    if (!IsEnd() && Current() == '.') {
                        Advance();
                        if (!IsEnd() && Current() == '.') {
                            Advance();
                            return new Token(TokenType.OP_SPREAD, "...", line, col, _filename);
                        }
                        return new Token(TokenType.OP_RANGE, "..", line, col, _filename);
                    }
                    return new Token(TokenType.DOT, ".", line, col, _filename);

                case '+': return new Token(TokenType.OP_PLUS,   "+", line, col, _filename);
                case '-':
                    if (!IsEnd() && Current() == '>') {
                        Advance();
                        return new Token(TokenType.OP_THIN_ARROW, "->", line, col, _filename);
                    }
                    return new Token(TokenType.OP_MINUS, "-", line, col, _filename);

                case '*': return new Token(TokenType.OP_STAR,    "*", line, col, _filename);
                case '%': return new Token(TokenType.OP_PERCENT, "%", line, col, _filename);
                case '^': return new Token(TokenType.OP_POWER,   "^", line, col, _filename);

                case '/':
                    return new Token(TokenType.OP_SLASH, "/", line, col, _filename);

                case '=':
                    if (!IsEnd() && Current() == '=') {
                        Advance();
                        return new Token(TokenType.OP_EQEQ, "==", line, col, _filename);
                    }
                    if (!IsEnd() && Current() == '>') {
                        Advance();
                        return new Token(TokenType.OP_ARROW, "=>", line, col, _filename);
                    }
                    return new Token(TokenType.OP_EQ, "=", line, col, _filename);

                case '!':
                    if (!IsEnd() && Current() == '=') {
                        Advance();
                        return new Token(TokenType.OP_NEQ, "!=", line, col, _filename);
                    }
                    return new Token(TokenType.OP_NOT, "!", line, col, _filename);

                case '<':
                    if (!IsEnd() && Current() == '=') {
                        Advance();
                        return new Token(TokenType.OP_LTE, "<=", line, col, _filename);
                    }
                    return new Token(TokenType.OP_LT, "<", line, col, _filename);

                case '>':
                    if (!IsEnd() && Current() == '=') {
                        Advance();
                        return new Token(TokenType.OP_GTE, ">=", line, col, _filename);
                    }
                    if (!IsEnd() && Current() == '>') {
                        Advance();
                        return new Token(TokenType.OP_COMPOSE, ">>", line, col, _filename);
                    }
                    return new Token(TokenType.OP_GT, ">", line, col, _filename);

                case '&':
                    if (!IsEnd() && Current() == '&') {
                        Advance();
                        return new Token(TokenType.OP_AND, "&&", line, col, _filename);
                    }
                    break;

                case '|':
                    if (!IsEnd() && Current() == '|') {
                        Advance();
                        return new Token(TokenType.OP_OR, "||", line, col, _filename);
                    }
                    if (!IsEnd() && Current() == '>') {
                        Advance();
                        return new Token(TokenType.OP_PIPE, "|>", line, col, _filename);
                    }
                    break;

                case '?':
                    if (!IsEnd() && Current() == '?') {
                        Advance();
                        return new Token(TokenType.OP_NULLCOAL, "??", line, col, _filename);
                    }
                    if (!IsEnd() && Current() == '.') {
                        Advance();
                        return new Token(TokenType.OP_NULLSAFE, "?.", line, col, _filename);
                    }
                    return new Token(TokenType.QUESTION, "?", line, col, _filename);

                case '\n':
                    _line++;
                    _column = 1;
                    return new Token(TokenType.NEWLINE, "\\n", line, col, _filename);

                default: break;
            }

            return new Token(TokenType.UNKNOWN,
                             c.to_string(), line, col, _filename);
        }


        // ─────────────────────────────────────────────
        //  Skip espaces et commentaires
        // ─────────────────────────────────────────────
        private void SkipWhitespaceAndComments() {
            while (!IsEnd()) {
                char c = Current();

                // Espaces
                if (c == ' ' || c == '\t' || c == '\r') {
                    Advance();
                    continue;
                }

                // Commentaire ligne //
                if (c == '/' && Peek(1) == '/') {
                    while (!IsEnd() && Current() != '\n') Advance();
                    continue;
                }

                // Commentaire bloc /* ... */
                if (c == '/' && Peek(1) == '*') {
                    Advance(); Advance();
                    while (!IsEnd()) {
                        if (Current() == '*' && Peek(1) == '/') {
                            Advance(); Advance();
                            break;
                        }
                        if (Current() == '\n') { _line++; _column = 1; }
                        Advance();
                    }
                    continue;
                }

                break;
            }
        }


        // ─────────────────────────────────────────────
        //  Helpers navigation
        // ─────────────────────────────────────────────
        private char Current() {
            if (IsEnd()) return '\0';
            return _source[_pos];
        }

        private char Peek(int offset = 1) {
            int idx = _pos + offset;
            if (idx >= _source.length) return '\0';
            return _source[idx];
        }

        private void Advance() {
            if (!IsEnd()) {
                _pos++;
                _column++;
            }
        }

        private bool IsEnd() {
            return _pos >= _source.length;
        }

        private Token MakeToken(TokenType type, string value) {
            return new Token(type, value, _line, _column, _filename);
        }
    }
}
