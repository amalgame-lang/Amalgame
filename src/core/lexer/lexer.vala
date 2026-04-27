// CODE Language - Lexer
// Copyright (c) 2024 NeitsabTeguom
// Licensed under Apache 2.0

namespace CodeTranspiler.Lexer {

    public class Lexer : Object {

        private string   _source;
        private int      _pos;
        private int      _line;
        private int      _column;
        private string   _filename;

        private static Gee.HashMap<string,
            CodeTranspiler.Lexer.TokenType> _keywords;

        static construct {
            _keywords = new Gee.HashMap<string,
                CodeTranspiler.Lexer.TokenType>();

            _keywords["int"]        = CodeTranspiler.Lexer.TokenType.KW_INT;
            _keywords["float"]      = CodeTranspiler.Lexer.TokenType.KW_FLOAT;
            _keywords["double"]     = CodeTranspiler.Lexer.TokenType.KW_DOUBLE;
            _keywords["string"]     = CodeTranspiler.Lexer.TokenType.KW_STRING;
            _keywords["bool"]       = CodeTranspiler.Lexer.TokenType.KW_BOOL;
            _keywords["void"]       = CodeTranspiler.Lexer.TokenType.KW_VOID;
            _keywords["var"]        = CodeTranspiler.Lexer.TokenType.KW_VAR;
            _keywords["let"]        = CodeTranspiler.Lexer.TokenType.KW_LET;
            _keywords["class"]      = CodeTranspiler.Lexer.TokenType.KW_CLASS;
            _keywords["interface"]  = CodeTranspiler.Lexer.TokenType.KW_INTERFACE;
            _keywords["struct"]     = CodeTranspiler.Lexer.TokenType.KW_STRUCT;
            _keywords["enum"]       = CodeTranspiler.Lexer.TokenType.KW_ENUM;
            _keywords["record"]     = CodeTranspiler.Lexer.TokenType.KW_RECORD;
            _keywords["data"]       = CodeTranspiler.Lexer.TokenType.KW_DATA;
            _keywords["trait"]      = CodeTranspiler.Lexer.TokenType.KW_TRAIT;
            _keywords["extends"]    = CodeTranspiler.Lexer.TokenType.KW_EXTENDS;
            _keywords["implements"] = CodeTranspiler.Lexer.TokenType.KW_IMPLEMENTS;
            _keywords["new"]        = CodeTranspiler.Lexer.TokenType.KW_NEW;
            _keywords["this"]       = CodeTranspiler.Lexer.TokenType.KW_THIS;
            _keywords["static"]     = CodeTranspiler.Lexer.TokenType.KW_STATIC;
            _keywords["abstract"]   = CodeTranspiler.Lexer.TokenType.KW_ABSTRACT;
            _keywords["override"]   = CodeTranspiler.Lexer.TokenType.KW_OVERRIDE;
            _keywords["public"]     = CodeTranspiler.Lexer.TokenType.KW_PUBLIC;
            _keywords["private"]    = CodeTranspiler.Lexer.TokenType.KW_PRIVATE;
            _keywords["protected"]  = CodeTranspiler.Lexer.TokenType.KW_PROTECTED;
            _keywords["internal"]   = CodeTranspiler.Lexer.TokenType.KW_INTERNAL;
            _keywords["if"]         = CodeTranspiler.Lexer.TokenType.KW_IF;
            _keywords["else"]       = CodeTranspiler.Lexer.TokenType.KW_ELSE;
            _keywords["match"]      = CodeTranspiler.Lexer.TokenType.KW_MATCH;
            _keywords["return"]     = CodeTranspiler.Lexer.TokenType.KW_RETURN;
            _keywords["while"]      = CodeTranspiler.Lexer.TokenType.KW_WHILE;
            _keywords["for"]        = CodeTranspiler.Lexer.TokenType.KW_FOR;
            _keywords["foreach"]    = CodeTranspiler.Lexer.TokenType.KW_FOREACH;
            _keywords["in"]         = CodeTranspiler.Lexer.TokenType.KW_IN;
            _keywords["break"]      = CodeTranspiler.Lexer.TokenType.KW_BREAK;
            _keywords["continue"]   = CodeTranspiler.Lexer.TokenType.KW_CONTINUE;
            _keywords["guard"]      = CodeTranspiler.Lexer.TokenType.KW_GUARD;
            _keywords["pure"]       = CodeTranspiler.Lexer.TokenType.KW_PURE;
            _keywords["async"]      = CodeTranspiler.Lexer.TokenType.KW_ASYNC;
            _keywords["await"]      = CodeTranspiler.Lexer.TokenType.KW_AWAIT;
            _keywords["go"]         = CodeTranspiler.Lexer.TokenType.KW_GO;
            _keywords["try"]        = CodeTranspiler.Lexer.TokenType.KW_TRY;
            _keywords["catch"]      = CodeTranspiler.Lexer.TokenType.KW_CATCH;
            _keywords["throw"]      = CodeTranspiler.Lexer.TokenType.KW_THROW;
            _keywords["namespace"]  = CodeTranspiler.Lexer.TokenType.KW_NAMESPACE;
            _keywords["import"]     = CodeTranspiler.Lexer.TokenType.KW_IMPORT;
            _keywords["weak"]       = CodeTranspiler.Lexer.TokenType.KW_WEAK;
            _keywords["true"]       = CodeTranspiler.Lexer.TokenType.BOOL_TRUE;
            _keywords["false"]      = CodeTranspiler.Lexer.TokenType.BOOL_FALSE;
            _keywords["null"]       = CodeTranspiler.Lexer.TokenType.NULL;
            _keywords["sealed"]     = CodeTranspiler.Lexer.TokenType.KW_SEALED;
        }

        public Lexer(string source,
                     string filename = "<unknown>") {
            _source   = source;
            _pos      = 0;
            _line     = 1;
            _column   = 1;
            _filename = filename;
        }

        // ── Helpers caracteres ─────────────────────────
        private bool IsLetter(char c) {
            return (c >= 'a' && c <= 'z') ||
                   (c >= 'A' && c <= 'Z') ||
                   c == '_';
        }

        private bool IsDigit(char c) {
            return c >= '0' && c <= '9';
        }

        private bool IsAlphaNum(char c) {
            return IsLetter(c) || IsDigit(c);
        }

        // ── Navigation ─────────────────────────────────
        private char Cur() {
            if (_pos >= _source.length) return '\0';
            return _source[_pos];
        }

        private char Pk(int offset = 1) {
            int i = _pos + offset;
            if (i >= _source.length) return '\0';
            return _source[i];
        }

        private char Adv() {
            char c = Cur();
            if (_pos < _source.length) {
                _pos++;
                _column++;
            }
            return c;
        }

        private bool End() {
            return _pos >= _source.length;
        }

        private Token Tok(CodeTranspiler.Lexer.TokenType t,
                          string v, int l, int c) {
            return new Token(t, v, l, c, _filename);
        }

        // ── Skip espaces et commentaires ───────────────
        private void Skip() {
            while (!End()) {
                char c = Cur();
                if (c == ' ' || c == '\t' || c == '\r') {
                    Adv();
                } else if (c == '/' && Pk() == '/') {
                    while (!End() && Cur() != '\n') Adv();
                } else if (c == '/' && Pk() == '*') {
                    Adv(); Adv();
                    while (!End()) {
                        if (Cur() == '*' && Pk() == '/') {
                            Adv(); Adv(); break;
                        }
                        if (Cur() == '\n') {
                            _line++; _column = 1;
                        }
                        Adv();
                    }
                } else {
                    break;
                }
            }
        }

        // ── Point d entree ─────────────────────────────
        public Gee.ArrayList<Token> Tokenize() {
            var tokens = new Gee.ArrayList<Token>();
            while (!End()) {
                Skip();
                if (End()) break;
                var t = NextTok();
                if (t != null) tokens.add(t);
            }
            tokens.add(Tok(
                CodeTranspiler.Lexer.TokenType.EOF,
                "", _line, _column));
            return tokens;
        }

        private Token? NextTok() {
            int  l = _line;
            int  c = _column;
            char ch = Cur();

            // Newline
            if (ch == '\n') {
                Adv();
                _line++;
                _column = 1;
                return Tok(CodeTranspiler.Lexer.TokenType.NEWLINE,
                           "\n", l, c);
            }

            // Identifiant ou mot-cle
            // DOIT etre avant les nombres !
            if (IsLetter(ch)) {
                return ReadIdent(l, c);
            }

            // Nombre
            if (IsDigit(ch)) {
                return ReadNum(l, c);
            }

            // String
            if (ch == '"') {
                // Verifier triple quote
                if (Pk(1) == '"' && Pk(2) == '"') {
                    return ReadTriple(l, c);
                }
                return ReadStr(l, c);
            }

            // Decorateur
            if (ch == '@') {
                Adv();
                return Tok(CodeTranspiler.Lexer.TokenType.AT,
                           "@", l, c);
            }

            // Operateurs et delimiteurs
            return ReadOp(l, c);
        }

        // ── Identifiant ou mot-cle ─────────────────────
        private Token ReadIdent(int l, int c) {
            var sb = new StringBuilder();
            while (!End() && IsAlphaNum(Cur())) {
                sb.append_c(Adv());
            }
            string word = sb.str;

            // Chercher dans les mots-cles
            if (_keywords.has_key(word)) {
                return Tok(_keywords[word], word, l, c);
            }

            return Tok(CodeTranspiler.Lexer.TokenType.IDENTIFIER,
                       word, l, c);
        }

        // ── Nombre ─────────────────────────────────────
        private Token ReadNum(int l, int c) {
            var  sb      = new StringBuilder();
            bool isFloat = false;

            while (!End() && (IsDigit(Cur()) || Cur() == '_')) {
                if (Cur() != '_') sb.append_c(Cur());
                Adv();
            }

            if (!End() && Cur() == '.' && IsDigit(Pk())) {
                isFloat = true;
                sb.append_c('.');
                Adv();
                while (!End() && IsDigit(Cur())) {
                    sb.append_c(Cur());
                    Adv();
                }
            }

            if (!End() && (Cur() == 'e' || Cur() == 'E')) {
                isFloat = true;
                sb.append_c(Adv());
                if (!End() && (Cur() == '+' || Cur() == '-')) {
                    sb.append_c(Adv());
                }
                while (!End() && IsDigit(Cur())) {
                    sb.append_c(Adv());
                }
            }

            var type = isFloat
                ? CodeTranspiler.Lexer.TokenType.FLOAT
                : CodeTranspiler.Lexer.TokenType.INTEGER;
            return Tok(type, sb.str, l, c);
        }

        // ── String simple ──────────────────────────────
        private Token ReadStr(int l, int c) {
            var sb = new StringBuilder();
            Adv(); // consume "
            while (!End() && Cur() != '"') {
                if (Cur() == '\\') {
                    Adv();
                    switch (Cur()) {
                        case 'n':  sb.append_c('\n'); break;
                        case 't':  sb.append_c('\t'); break;
                        case '"':  sb.append_c('"');  break;
                        case '\\': sb.append_c('\\'); break;
                        default:
                            sb.append_c('\\');
                            sb.append_c(Cur());
                            break;
                    }
                } else {
                    sb.append_c(Cur());
                }
                Adv();
            }
            Adv(); // consume "
            return Tok(CodeTranspiler.Lexer.TokenType.STRING,
                       sb.str, l, c);
        }

        // ── String triple ──────────────────────────────
        private Token ReadTriple(int l, int c) {
            var sb = new StringBuilder();
            Adv(); Adv(); Adv(); // consume """
            while (!End()) {
                if (Cur() == '"' && Pk() == '"' && Pk(2) == '"') {
                    Adv(); Adv(); Adv();
                    break;
                }
                if (Cur() == '\n') { _line++; _column = 0; }
                sb.append_c(Adv());
            }
            return Tok(CodeTranspiler.Lexer.TokenType.STRING,
                       sb.str, l, c);
        }

        // ── Operateurs ─────────────────────────────────
        private Token ReadOp(int l, int c) {
            char ch = Adv();
            switch (ch) {
                case '{': return Tok(CodeTranspiler.Lexer.TokenType.LBRACE,    "{", l, c);
                case '}': return Tok(CodeTranspiler.Lexer.TokenType.RBRACE,    "}", l, c);
                case '(': return Tok(CodeTranspiler.Lexer.TokenType.LPAREN,    "(", l, c);
                case ')': return Tok(CodeTranspiler.Lexer.TokenType.RPAREN,    ")", l, c);
                case '[': return Tok(CodeTranspiler.Lexer.TokenType.LBRACKET,  "[", l, c);
                case ']': return Tok(CodeTranspiler.Lexer.TokenType.RBRACKET,  "]", l, c);
                case ';': return Tok(CodeTranspiler.Lexer.TokenType.SEMICOLON, ";", l, c);
                case ',': return Tok(CodeTranspiler.Lexer.TokenType.COMMA,     ",", l, c);
                case '~': return Tok(CodeTranspiler.Lexer.TokenType.UNKNOWN,   "~", l, c);
                case ':':
                    return Tok(CodeTranspiler.Lexer.TokenType.COLON, ":", l, c);
                case '.':
                    if (!End() && Cur() == '.') {
                        Adv();
                        if (!End() && Cur() == '.') {
                            Adv();
                            return Tok(CodeTranspiler.Lexer.TokenType.OP_SPREAD, "...", l, c);
                        }
                        return Tok(CodeTranspiler.Lexer.TokenType.OP_RANGE, "..", l, c);
                    }
                    return Tok(CodeTranspiler.Lexer.TokenType.DOT, ".", l, c);
                case '+': return Tok(CodeTranspiler.Lexer.TokenType.OP_PLUS,    "+", l, c);
                case '*': return Tok(CodeTranspiler.Lexer.TokenType.OP_STAR,    "*", l, c);
                case '%': return Tok(CodeTranspiler.Lexer.TokenType.OP_PERCENT, "%", l, c);
                case '^': return Tok(CodeTranspiler.Lexer.TokenType.OP_POWER,   "^", l, c);
                case '-':
                    if (!End() && Cur() == '>') {
                        Adv();
                        return Tok(CodeTranspiler.Lexer.TokenType.OP_THIN_ARROW, "->", l, c);
                    }
                    return Tok(CodeTranspiler.Lexer.TokenType.OP_MINUS, "-", l, c);
                case '/':
                    return Tok(CodeTranspiler.Lexer.TokenType.OP_SLASH, "/", l, c);
                case '=':
                    if (!End() && Cur() == '=') {
                        Adv();
                        return Tok(CodeTranspiler.Lexer.TokenType.OP_EQEQ, "==", l, c);
                    }
                    if (!End() && Cur() == '>') {
                        Adv();
                        return Tok(CodeTranspiler.Lexer.TokenType.OP_ARROW, "=>", l, c);
                    }
                    return Tok(CodeTranspiler.Lexer.TokenType.OP_EQ, "=", l, c);
                case '!':
                    if (!End() && Cur() == '=') {
                        Adv();
                        return Tok(CodeTranspiler.Lexer.TokenType.OP_NEQ, "!=", l, c);
                    }
                    return Tok(CodeTranspiler.Lexer.TokenType.OP_NOT, "!", l, c);
                case '<':
                    if (!End() && Cur() == '=') {
                        Adv();
                        return Tok(CodeTranspiler.Lexer.TokenType.OP_LTE, "<=", l, c);
                    }
                    return Tok(CodeTranspiler.Lexer.TokenType.OP_LT, "<", l, c);
                case '>':
                    if (!End() && Cur() == '=') {
                        Adv();
                        return Tok(CodeTranspiler.Lexer.TokenType.OP_GTE, ">=", l, c);
                    }
                    if (!End() && Cur() == '>') {
                        Adv();
                        return Tok(CodeTranspiler.Lexer.TokenType.OP_COMPOSE, ">>", l, c);
                    }
                    return Tok(CodeTranspiler.Lexer.TokenType.OP_GT, ">", l, c);
                case '&':
                    if (!End() && Cur() == '&') {
                        Adv();
                        return Tok(CodeTranspiler.Lexer.TokenType.OP_AND, "&&", l, c);
                    }
                    return Tok(CodeTranspiler.Lexer.TokenType.UNKNOWN, "&", l, c);
                case '|':
                    if (!End() && Cur() == '|') {
                        Adv();
                        return Tok(CodeTranspiler.Lexer.TokenType.OP_OR, "||", l, c);
                    }
                    if (!End() && Cur() == '>') {
                        Adv();
                        return Tok(CodeTranspiler.Lexer.TokenType.OP_PIPE, "|>", l, c);
                    }
                    return Tok(CodeTranspiler.Lexer.TokenType.UNKNOWN, "|", l, c);
                case '?':
                    if (!End() && Cur() == '?') {
                        Adv();
                        return Tok(CodeTranspiler.Lexer.TokenType.OP_NULLCOAL, "??", l, c);
                    }
                    if (!End() && Cur() == '.') {
                        Adv();
                        return Tok(CodeTranspiler.Lexer.TokenType.OP_NULLSAFE, "?.", l, c);
                    }
                    return Tok(CodeTranspiler.Lexer.TokenType.QUESTION, "?", l, c);
                default:
                    return Tok(CodeTranspiler.Lexer.TokenType.UNKNOWN,
                               ch.to_string(), l, c);
            }
        }

        // Propriete publique pour compat
        public bool IsEnd() { return End(); }
    }
}
