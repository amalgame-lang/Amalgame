// ─────────────────────────────────────────────────────
//  CODE Programming Language
//  Copyright (c) 2024 NeitsabTeguom
//  Licensed under Apache 2.0
// ─────────────────────────────────────────────────────
//  parser.vala  -  Parser du langage CODE
//
//  Transforme une liste de tokens en AST.
//  Technique : Recursive Descent Parser
//
//  Chaque méthode Parse*() correspond à
//  une règle de la grammaire EBNF.
// ─────────────────────────────────────────────────────

namespace CodeTranspiler.Parser {

    using CodeTranspiler.Lexer;
    using CodeTranspiler.Ast;

    // Alias pour éviter conflit avec GLib.TokenType
    // Utiliser LexerCodeTranspiler.Lexer.TokenType partout dans ce fichier



    // ═══════════════════════════════════════════════════
    //  Erreur de parsing
    // ═══════════════════════════════════════════════════

    public class ParseError : Object {
        public string Message  { get; set; }
        public int    Line     { get; set; }
        public int    Column   { get; set; }
        public string Filename { get; set; }

        public ParseError(string msg, Token token) {
            Message  = msg;
            Line     = token.Line;
            Column   = token.Column;
            Filename = token.Filename;
        }

        public string ToString() {
            return "\n┌── Erreur dans %s:%d:%d\n│\n│  %s\n│\n└──\n"
                   .printf(Filename, Line, Column, Message);
        }
    }


    // ═══════════════════════════════════════════════════
    //  Le Parser
    // ═══════════════════════════════════════════════════

    /**
     * Parser récursif descendant pour le langage CODE.
     *
     * Usage :
     *   var parser = new Parser(tokens, filename);
     *   var result = parser.Parse();
     *   if (result.Success) {
     *       var ast = result.Program;
     *   } else {
     *       foreach (var err in result.Errors) {
     *           stderr.printf(err.ToString());
     *       }
     *   }
     */
    public class Parser : Object {

        // ── État interne ───────────────────────────────
        private Gee.ArrayList<Token>      _tokens;
        private int                       _pos;
        private string                    _filename;
        private Gee.ArrayList<ParseError> _errors;


        // ── Résultat du parsing ────────────────────────
        public class ParseResult : Object {
            public bool                       Success { get; set; }
            public ProgramNode?               Program { get; set; }
            public Gee.ArrayList<ParseError>  Errors  { get; set; }

            public ParseResult() {
                Errors = new Gee.ArrayList<ParseError>();
            }
        }


        public Parser(Gee.ArrayList<Token> tokens,
                      string               filename = "<unknown>") {
            _tokens   = tokens;
            _pos      = 0;
            _filename = filename;
            _errors   = new Gee.ArrayList<ParseError>();
        }


        // ═══════════════════════════════════════════════
        //  Point d'entrée
        // ═══════════════════════════════════════════════

        /**
         * Parse le programme complet.
         * Retourne un ParseResult avec l'AST ou les erreurs.
         */
        public ParseResult Parse() {
            var result = new ParseResult();

            result.Program = ParseProgram();
            result.Success = _errors.size == 0;

            result.Errors = _errors;
            return result;
        }


        // ═══════════════════════════════════════════════
        //  Programme
        //  Program = NamespaceDecl { ImportDecl }
        //            { TopLevelDecl }
        // ═══════════════════════════════════════════════

        private ProgramNode ParseProgram() {
            var node = new ProgramNode();

            SkipNewlines();

            // namespace MyApp
            if (Check(CodeTranspiler.Lexer.TokenType.KW_NAMESPACE)) {
                node.Namespace = ParseNamespace();
            } else {
                // Namespace par défaut si absent
                var ns   = new NamespaceNode("Default");
                ns.Line  = 1;
                ns.Column= 1;
                node.Namespace = ns;
            }

            SkipNewlines();

            // import Code.IO
            // import Code.Net
            while (Check(CodeTranspiler.Lexer.TokenType.KW_IMPORT)) {
                node.Imports.add(ParseImport());
                SkipNewlines();
            }

            // Déclarations top-level
            // (classes, interfaces, enums, fonctions...)
            while (!IsEnd()) {
                SkipNewlines();
                if (IsEnd()) break;

                var decl = ParseTopLevelDecl();
                if (decl != null) {
                    node.Declarations.add(decl);
                }
                SkipNewlines();
            }

            return node;
        }


        // ═══════════════════════════════════════════════
        //  Namespace
        //  NamespaceDecl = "namespace" QualifiedName
        // ═══════════════════════════════════════════════

        private NamespaceNode ParseNamespace() {
            var tok  = Expect(CodeTranspiler.Lexer.TokenType.KW_NAMESPACE);
            var name = ParseQualifiedName();
            var node = new NamespaceNode(name);
            node.SetPosition(tok);
            SkipNewlines();
            return node;
        }


        // ═══════════════════════════════════════════════
        //  Import
        //  ImportDecl = "import" QualifiedName
        // ═══════════════════════════════════════════════

        private ImportNode ParseImport() {
            var tok  = Expect(CodeTranspiler.Lexer.TokenType.KW_IMPORT);
            var name = ParseQualifiedName();
            var node = new ImportNode(name);
            node.SetPosition(tok);
            SkipNewlines();
            return node;
        }


        // ═══════════════════════════════════════════════
        //  Déclaration Top-Level
        //  TopLevelDecl = { Decorator }
        //                 ( ClassDecl | InterfaceDecl
        //                 | EnumDecl  | RecordDecl
        //                 | FunctionDecl )
        // ═══════════════════════════════════════════════

        private AstNode? ParseTopLevelDecl() {

            // Collecter les décorateurs @memory, @pure...
            var decorators = new Gee.ArrayList<DecoratorNode>();
            while (Check(CodeTranspiler.Lexer.TokenType.AT)) {
                decorators.add(ParseDecorator());
                SkipNewlines();
            }

            // Modificateur d'accès optionnel
            var access = ParseAccessModifier();

            // Dispatcher selon le mot clé
            if (Check(CodeTranspiler.Lexer.TokenType.KW_CLASS)) {
                var node = ParseClassDecl(access);
                foreach (var d in decorators) {
                    node.Decorators.add(d);
                }
                return node;
            }

            if (Check(CodeTranspiler.Lexer.TokenType.KW_INTERFACE)) {
                var node = ParseInterfaceDecl(access);
                foreach (var d in decorators) {
                    node.Decorators.add(d);
                }
                return node;
            }

            if (Check(CodeTranspiler.Lexer.TokenType.KW_ENUM)) {
                var node = ParseEnumDecl(access);
                foreach (var d in decorators) {
                    node.Decorators.add(d);
                }
                return node;
            }

            if (Check(CodeTranspiler.Lexer.TokenType.KW_RECORD)) {
                var node = ParseRecordDecl(access);
                foreach (var d in decorators) {
                    node.Decorators.add(d);
                }
                return node;
            }

            if (Check(CodeTranspiler.Lexer.TokenType.KW_DATA)) {
                var node = ParseDataClassDecl(access);
                foreach (var d in decorators) {
                    node.Decorators.add(d);
                }
                return node;
            }

            // Fonction top-level
            if (CheckMethodStart()) {
                var node = ParseMethodDecl(access);
                foreach (var d in decorators) {
                    node.Decorators.add(d);
                }
                return node;
            }

            // Token inattendu
            var tok = Current();
            AddError("Déclaration inattendue : '%s'"
                     .printf(tok.Value), tok);
            Advance(); // éviter boucle infinie
            return null;
        }


        // ═══════════════════════════════════════════════
        //  Décorateur
        //  Decorator = "@" IDENTIFIER
        //              [ "(" DecoratorArgList ")" ]
        // ═══════════════════════════════════════════════

        private DecoratorNode ParseDecorator() {
            var at   = Expect(CodeTranspiler.Lexer.TokenType.AT);
            var name = Expect(CodeTranspiler.Lexer.TokenType.IDENTIFIER);
            var node = new DecoratorNode(name.Value);
            node.SetPosition(at);

            // Arguments optionnels : @memory(arc, size=4MB)
            if (Check(CodeTranspiler.Lexer.TokenType.LPAREN)) {
                Advance(); // consume (
                while (!Check(CodeTranspiler.Lexer.TokenType.RPAREN) && !IsEnd()) {
                    node.Args.add(ParseExpression());
                    if (Check(CodeTranspiler.Lexer.TokenType.COMMA)) Advance();
                }
                Expect(CodeTranspiler.Lexer.TokenType.RPAREN);
            }

            SkipNewlines();
            return node;
        }


        // ═══════════════════════════════════════════════
        //  Classe
        //  ClassDecl = "class" IDENTIFIER
        //              [ GenericParams ]
        //              [ "extends" TypeRef ]
        //              [ "implements" TypeRef,... ]
        //              "{" { ClassMember } "}"
        // ═══════════════════════════════════════════════

        private ClassDeclNode ParseClassDecl(AccessModifier access) {
            var tok  = Expect(CodeTranspiler.Lexer.TokenType.KW_CLASS);
            var name = Expect(CodeTranspiler.Lexer.TokenType.IDENTIFIER);
            var node = new ClassDeclNode(name.Value);
            node.SetPosition(tok);
            node.Access = access;

            // Modificateurs
            node.IsAbstract = ConsumeIf(CodeTranspiler.Lexer.TokenType.KW_ABSTRACT);
            node.IsSealed   = ConsumeIf(CodeTranspiler.Lexer.TokenType.KW_SEALED);

            // Génériques : <T, U>
            if (Check(CodeTranspiler.Lexer.TokenType.OP_LT)) {
                node.Generics = ParseGenericParams();
            }

            // extends BaseClass
            if (Check(CodeTranspiler.Lexer.TokenType.KW_EXTENDS)) {
                Advance();
                node.BaseClass = ParseTypeRef();
            }

            // implements IFace1, IFace2
            if (Check(CodeTranspiler.Lexer.TokenType.KW_IMPLEMENTS)) {
                Advance();
                node.Interfaces.add(ParseTypeRef());
                while (Check(CodeTranspiler.Lexer.TokenType.COMMA)) {
                    Advance();
                    node.Interfaces.add(ParseTypeRef());
                }
            }

            // Corps { ... }
            Expect(CodeTranspiler.Lexer.TokenType.LBRACE);
            SkipNewlines();

            while (!Check(CodeTranspiler.Lexer.TokenType.RBRACE) && !IsEnd()) {
                var member = ParseClassMember();
                if (member != null) {
                    node.Members.add(member);
                }
                SkipNewlines();
            }

            Expect(CodeTranspiler.Lexer.TokenType.RBRACE);
            return node;
        }


        // ═══════════════════════════════════════════════
        //  Membre de classe
        // ═══════════════════════════════════════════════

        private AstNode? ParseClassMember() {

            // Décorateurs
            var decorators = new Gee.ArrayList<DecoratorNode>();
            while (Check(CodeTranspiler.Lexer.TokenType.AT)) {
                decorators.add(ParseDecorator());
                SkipNewlines();
            }

            // Modificateur d'accès
            var access = ParseAccessModifier();

            // Modificateurs supplémentaires
            bool isStatic   = ConsumeIf(CodeTranspiler.Lexer.TokenType.KW_STATIC);
            bool isAbstract = ConsumeIf(CodeTranspiler.Lexer.TokenType.KW_ABSTRACT);
            bool isOverride = ConsumeIf(CodeTranspiler.Lexer.TokenType.KW_OVERRIDE);
            bool isAsync    = ConsumeIf(CodeTranspiler.Lexer.TokenType.KW_ASYNC);
            bool isPure     = ConsumeIf(CodeTranspiler.Lexer.TokenType.KW_PURE);
            bool isWeak     = ConsumeIf(CodeTranspiler.Lexer.TokenType.KW_WEAK);

            // Constructeur : même nom que la classe
            // → détecté au niveau class, simplifié ici

            // Propriété ou champ : IDENTIFIER ":" TypeRef
            // Méthode           : IDENTIFIER "(" ...
            if (Check(CodeTranspiler.Lexer.TokenType.IDENTIFIER)) {
                
                // Méthode : nom suivi de ( ou <
                if (PeekType(1) == CodeTranspiler.Lexer.TokenType.LPAREN ||
                    PeekType(1) == CodeTranspiler.Lexer.TokenType.OP_LT) {
                    var method = ParseMethodDecl(access);
                    method.IsStatic   = isStatic;
                    method.IsAbstract = isAbstract;
                    method.IsOverride = isOverride;
                    method.IsAsync    = isAsync;
                    method.IsPure     = isPure;
                    foreach (var d in decorators) {
                        method.Decorators.add(d);
                    }
                    return method;
                }

                // Propriété : nom suivi de ":" TypeRef "{"
                if (PeekType(1) == CodeTranspiler.Lexer.TokenType.COLON) {
                    // Regarder plus loin pour { get; set; }
                    // vs champ simple
                    var field = ParseFieldOrProperty(
                        access, isStatic, isWeak, decorators
                    );
                    return field;
                }
            }

            // Mots clés de méthode
            if (CheckMethodStart()) {
                var method = ParseMethodDecl(access);
                method.IsStatic   = isStatic;
                method.IsAbstract = isAbstract;
                method.IsOverride = isOverride;
                method.IsAsync    = isAsync;
                method.IsPure     = isPure;
                foreach (var d in decorators) {
                    method.Decorators.add(d);
                }
                return method;
            }

            // Token inattendu dans la classe
            var tok = Current();
            if (!Check(CodeTranspiler.Lexer.TokenType.RBRACE)) {
                AddError("Membre de classe inattendu : '%s'"
                         .printf(tok.Value), tok);
                Advance();
            }
            return null;
        }


        // ═══════════════════════════════════════════════
        //  Champ ou Propriété
        // ═══════════════════════════════════════════════

        private AstNode ParseFieldOrProperty(
            AccessModifier             access,
            bool                       isStatic,
            bool                       isWeak,
            Gee.ArrayList<DecoratorNode> decorators) {

            var nameTok = Expect(CodeTranspiler.Lexer.TokenType.IDENTIFIER);
            Expect(CodeTranspiler.Lexer.TokenType.COLON);
            var type = ParseTypeRef();

            // Propriété : { get; set; }
            //           : { get => expr }
            if (Check(CodeTranspiler.Lexer.TokenType.LBRACE)) {
                var prop = new PropertyDeclNode(nameTok.Value, type);
                prop.SetPosition(nameTok);
                prop.Access   = access;
                prop.IsStatic = isStatic;
                foreach (var d in decorators) {
                    prop.Decorators.add(d);
                }

                Advance(); // consume {
                SkipNewlines();

                // get
                if (Check(CodeTranspiler.Lexer.TokenType.IDENTIFIER) &&
                    Current().Value == "get") {
                    Advance();
                    if (Check(CodeTranspiler.Lexer.TokenType.OP_ARROW)) {
                        Advance();
                        prop.Getter = ParseExpression();
                    } else if (Check(CodeTranspiler.Lexer.TokenType.LBRACE)) {
                        prop.Getter = ParseBlock();
                    }
                    ConsumeIf(CodeTranspiler.Lexer.TokenType.SEMICOLON);
                    SkipNewlines();
                }

                // set
                if (Check(CodeTranspiler.Lexer.TokenType.IDENTIFIER) &&
                    Current().Value == "set") {
                    Advance();
                    if (Check(CodeTranspiler.Lexer.TokenType.OP_ARROW)) {
                        Advance();
                        prop.Setter = ParseExpression();
                    } else if (Check(CodeTranspiler.Lexer.TokenType.LBRACE)) {
                        prop.Setter = ParseBlock();
                    }
                    ConsumeIf(CodeTranspiler.Lexer.TokenType.SEMICOLON);
                    SkipNewlines();
                }

                Expect(CodeTranspiler.Lexer.TokenType.RBRACE);
                return prop;
            }

            // Champ simple avec valeur initiale optionnelle
            var field = new FieldDeclNode(nameTok.Value, type);
            field.SetPosition(nameTok);
            field.Access   = access;
            field.IsStatic = isStatic;
            field.IsWeak   = isWeak;
            foreach (var d in decorators) {
                field.Decorators.add(d);
            }

            // = valeur initiale
            if (Check(CodeTranspiler.Lexer.TokenType.OP_EQ)) {
                Advance();
                field.Initial = ParseExpression();
            }

            SkipNewlines();
            return field;
        }


        // ═══════════════════════════════════════════════
        //  Méthode / Fonction
        //  MethodDecl = [ modifiers ] IDENTIFIER
        //               [ GenericParams ]
        //               "(" [ ParamList ] ")"
        //               [ "->" TypeRef ]
        //               ( Block | "=>" Expression )
        // ═══════════════════════════════════════════════

        private MethodDeclNode ParseMethodDecl(
            AccessModifier access) {

            // Modificateurs inline (async, pure, static...)
            bool isAsync  = ConsumeIf(CodeTranspiler.Lexer.TokenType.KW_ASYNC);
            bool isPure   = ConsumeIf(CodeTranspiler.Lexer.TokenType.KW_PURE);
            bool isStatic = ConsumeIf(CodeTranspiler.Lexer.TokenType.KW_STATIC);

            var nameTok = Expect(CodeTranspiler.Lexer.TokenType.IDENTIFIER);
            var node    = new MethodDeclNode(nameTok.Value);
            node.SetPosition(nameTok);
            node.Access   = access;
            node.IsAsync  = isAsync;
            node.IsPure   = isPure;
            node.IsStatic = isStatic;

            // Génériques <T>
            if (Check(CodeTranspiler.Lexer.TokenType.OP_LT)) {
                node.Generics = ParseGenericParams();
            }

            // Paramètres ( ... )
            Expect(CodeTranspiler.Lexer.TokenType.LPAREN);
            if (!Check(CodeTranspiler.Lexer.TokenType.RPAREN)) {
                ParseParamList(node.Params);
            }
            Expect(CodeTranspiler.Lexer.TokenType.RPAREN);

            // Type de retour -> TypeRef
            if (Check(CodeTranspiler.Lexer.TokenType.OP_THIN_ARROW)) {
                Advance();
                node.ReturnType = ParseTypeRef();
            }

            // Corps : bloc { } ou expression => expr
            if (Check(CodeTranspiler.Lexer.TokenType.OP_ARROW)) {
                Advance();
                node.Body = ParseExpression();
                SkipNewlines();
            } else if (Check(CodeTranspiler.Lexer.TokenType.LBRACE)) {
                node.Body = ParseBlock();
            } else {
                // Méthode abstraite : pas de corps
                SkipNewlines();
            }

            return node;
        }


        // ═══════════════════════════════════════════════
        //  Paramètres
        // ═══════════════════════════════════════════════

        private void ParseParamList(
            Gee.ArrayList<ParamNode> dest) {

            dest.add(ParseParam());
            while (Check(CodeTranspiler.Lexer.TokenType.COMMA)) {
                Advance();
                SkipNewlines();
                dest.add(ParseParam());
            }
        }

        private ParamNode ParseParam() {
            bool isWeak = ConsumeIf(CodeTranspiler.Lexer.TokenType.KW_WEAK);
            var  name   = Expect(CodeTranspiler.Lexer.TokenType.IDENTIFIER);
            Expect(CodeTranspiler.Lexer.TokenType.COLON);
            var  type   = ParseTypeRef();

            AstNode? defaultVal = null;
            if (Check(CodeTranspiler.Lexer.TokenType.OP_EQ)) {
                Advance();
                defaultVal = ParseExpression();
            }

            var node = new ParamNode(name.Value, type, defaultVal);
            node.SetPosition(name);
            node.IsWeak = isWeak;
            return node;
        }


        // ═══════════════════════════════════════════════
        //  Interface
        // ═══════════════════════════════════════════════

        private InterfaceDeclNode ParseInterfaceDecl(
            AccessModifier access) {

            var tok  = Expect(CodeTranspiler.Lexer.TokenType.KW_INTERFACE);
            var name = Expect(CodeTranspiler.Lexer.TokenType.IDENTIFIER);
            var node = new InterfaceDeclNode(name.Value);
            node.SetPosition(tok);
            node.Access = access;

            if (Check(CodeTranspiler.Lexer.TokenType.OP_LT)) {
                node.Generics = ParseGenericParams();
            }

            if (Check(CodeTranspiler.Lexer.TokenType.KW_EXTENDS)) {
                Advance();
                node.BaseTypes.add(ParseTypeRef());
                while (Check(CodeTranspiler.Lexer.TokenType.COMMA)) {
                    Advance();
                    node.BaseTypes.add(ParseTypeRef());
                }
            }

            Expect(CodeTranspiler.Lexer.TokenType.LBRACE);
            SkipNewlines();

            while (!Check(CodeTranspiler.Lexer.TokenType.RBRACE) && !IsEnd()) {
                bool isAsync = ConsumeIf(CodeTranspiler.Lexer.TokenType.KW_ASYNC);
                bool isPure  = ConsumeIf(CodeTranspiler.Lexer.TokenType.KW_PURE);
                var  method  = ParseMethodDecl(
                                   AccessModifier.PUBLIC);
                method.IsAsync = isAsync;
                method.IsPure  = isPure;
                node.Members.add(method);
                SkipNewlines();
            }

            Expect(CodeTranspiler.Lexer.TokenType.RBRACE);
            return node;
        }


        // ═══════════════════════════════════════════════
        //  Enum
        // ═══════════════════════════════════════════════

        private EnumDeclNode ParseEnumDecl(
            AccessModifier access) {

            var tok  = Expect(CodeTranspiler.Lexer.TokenType.KW_ENUM);
            var name = Expect(CodeTranspiler.Lexer.TokenType.IDENTIFIER);
            var node = new EnumDeclNode(name.Value);
            node.SetPosition(tok);
            node.Access = access;

            Expect(CodeTranspiler.Lexer.TokenType.LBRACE);
            SkipNewlines();

            // Membres de l'enum
            while (!Check(CodeTranspiler.Lexer.TokenType.RBRACE) && !IsEnd()) {

                // Méthode dans l'enum
                if (Check(CodeTranspiler.Lexer.TokenType.KW_PUBLIC)  ||
                    Check(CodeTranspiler.Lexer.TokenType.KW_PRIVATE) ||
                    CheckMethodStart()) {
                    var access2 = ParseAccessModifier();
                    node.Methods.add(ParseMethodDecl(access2));
                    SkipNewlines();
                    continue;
                }

                // Membre enum
                if (Check(CodeTranspiler.Lexer.TokenType.IDENTIFIER)) {
                    var memberTok = Advance();
                    var member    = new EnumMemberNode(
                                       memberTok.Value);
                    member.SetPosition(memberTok);

                    // Valeurs associées : Tank(int, string)
                    if (Check(CodeTranspiler.Lexer.TokenType.LPAREN)) {
                        Advance();
                        while (!Check(CodeTranspiler.Lexer.TokenType.RPAREN)
                               && !IsEnd()) {
                            member.AssocTypes.add(ParseTypeRef());
                            if (Check(CodeTranspiler.Lexer.TokenType.COMMA)) Advance();
                        }
                        Expect(CodeTranspiler.Lexer.TokenType.RPAREN);
                    }

                    node.Members.add(member);
                    ConsumeIf(CodeTranspiler.Lexer.TokenType.COMMA);
                    SkipNewlines();
                } else {
                    break;
                }
            }

            Expect(CodeTranspiler.Lexer.TokenType.RBRACE);
            return node;
        }


        // ═══════════════════════════════════════════════
        //  Record
        //  record Point(float X, float Y)
        // ═══════════════════════════════════════════════

        private RecordDeclNode ParseRecordDecl(
            AccessModifier access) {

            var tok  = Expect(CodeTranspiler.Lexer.TokenType.KW_RECORD);
            var name = Expect(CodeTranspiler.Lexer.TokenType.IDENTIFIER);
            var node = new RecordDeclNode(name.Value);
            node.SetPosition(tok);
            node.Access = access;

            Expect(CodeTranspiler.Lexer.TokenType.LPAREN);
            if (!Check(CodeTranspiler.Lexer.TokenType.RPAREN)) {
                ParseRecordParams(node.Params);
            }
            Expect(CodeTranspiler.Lexer.TokenType.RPAREN);

            // Corps optionnel { méthodes }
            if (Check(CodeTranspiler.Lexer.TokenType.LBRACE)) {
                Advance();
                SkipNewlines();
                while (!Check(CodeTranspiler.Lexer.TokenType.RBRACE) && !IsEnd()) {
                    var acc = ParseAccessModifier();
                    node.Methods.add(ParseMethodDecl(acc));
                    SkipNewlines();
                }
                Expect(CodeTranspiler.Lexer.TokenType.RBRACE);
            }

            return node;
        }


        // ═══════════════════════════════════════════════
        //  Data Class
        //  data class Player(string Name, int Health = 100)
        // ═══════════════════════════════════════════════

        private DataClassDeclNode ParseDataClassDecl(
            AccessModifier access) {

            var tok = Expect(CodeTranspiler.Lexer.TokenType.KW_DATA);
            Expect(CodeTranspiler.Lexer.TokenType.KW_CLASS);
            var name = Expect(CodeTranspiler.Lexer.TokenType.IDENTIFIER);
            var node = new DataClassDeclNode(name.Value);
            node.SetPosition(tok);
            node.Access = access;

            Expect(CodeTranspiler.Lexer.TokenType.LPAREN);
            if (!Check(CodeTranspiler.Lexer.TokenType.RPAREN)) {
                ParseRecordParams(node.Params);
            }
            Expect(CodeTranspiler.Lexer.TokenType.RPAREN);

            // Corps optionnel
            if (Check(CodeTranspiler.Lexer.TokenType.LBRACE)) {
                Advance();
                SkipNewlines();
                while (!Check(CodeTranspiler.Lexer.TokenType.RBRACE) && !IsEnd()) {
                    var member = ParseClassMember();
                    if (member != null) {
                        node.Members.add(member);
                    }
                    SkipNewlines();
                }
                Expect(CodeTranspiler.Lexer.TokenType.RBRACE);
            }

            return node;
        }

        private void ParseRecordParams(
            Gee.ArrayList<RecordParamNode> dest) {

            dest.add(ParseRecordParam());
            while (Check(CodeTranspiler.Lexer.TokenType.COMMA)) {
                Advance();
                SkipNewlines();
                dest.add(ParseRecordParam());
            }
        }

        private RecordParamNode ParseRecordParam() {
            var type = ParseTypeRef();
            var name = Expect(CodeTranspiler.Lexer.TokenType.IDENTIFIER);

            AstNode? defaultVal = null;
            if (Check(CodeTranspiler.Lexer.TokenType.OP_EQ)) {
                Advance();
                defaultVal = ParseExpression();
            }

            var node = new RecordParamNode(type, name.Value,
                                           defaultVal);
            node.SetPosition(name);
            return node;
        }


        // ═══════════════════════════════════════════════
        //  INSTRUCTIONS (Statements)
        // ═══════════════════════════════════════════════

        private BlockNode ParseBlock() {
            var tok  = Expect(CodeTranspiler.Lexer.TokenType.LBRACE);
            var node = new BlockNode();
            node.SetPosition(tok);
            SkipNewlines();

            while (!Check(CodeTranspiler.Lexer.TokenType.RBRACE) && !IsEnd()) {
                var stmt = ParseStatement();
                if (stmt != null) {
                    node.Statements.add(stmt);
                }
                SkipNewlines();
            }

            Expect(CodeTranspiler.Lexer.TokenType.RBRACE);
            return node;
        }

        private AstNode? ParseStatement() {

            SkipNewlines();

            // let / var
            if (Check(CodeTranspiler.Lexer.TokenType.KW_LET) ||
                Check(CodeTranspiler.Lexer.TokenType.KW_VAR)) {
                return ParseVarDecl();
            }

            // if
            if (Check(CodeTranspiler.Lexer.TokenType.KW_IF)) {
                return ParseIf();
            }

            // match
            if (Check(CodeTranspiler.Lexer.TokenType.KW_MATCH)) {
                return ParseMatch();
            }

            // while
            if (Check(CodeTranspiler.Lexer.TokenType.KW_WHILE)) {
                return ParseWhile();
            }

            // for
            if (Check(CodeTranspiler.Lexer.TokenType.KW_FOR)) {
                return ParseFor();
            }

            // foreach
            if (Check(CodeTranspiler.Lexer.TokenType.KW_FOREACH)) {
                return ParseForeach();
            }

            // return
            if (Check(CodeTranspiler.Lexer.TokenType.KW_RETURN)) {
                return ParseReturn();
            }

            // guard
            if (Check(CodeTranspiler.Lexer.TokenType.KW_GUARD)) {
                return ParseGuard();
            }

            // break
            if (Check(CodeTranspiler.Lexer.TokenType.KW_BREAK)) {
                var tok  = Advance();
                var node = new BreakNode();
                node.SetPosition(tok);
                SkipNewlines();
                return node;
            }

            // continue
            if (Check(CodeTranspiler.Lexer.TokenType.KW_CONTINUE)) {
                var tok  = Advance();
                var node = new ContinueNode();
                node.SetPosition(tok);
                SkipNewlines();
                return node;
            }

            // try/catch
            if (Check(CodeTranspiler.Lexer.TokenType.KW_TRY)) {
                return ParseTryCatch();
            }

            // go (goroutine)
            if (Check(CodeTranspiler.Lexer.TokenType.KW_GO)) {
                return ParseGoStmt();
            }

            // bloc { }
            if (Check(CodeTranspiler.Lexer.TokenType.LBRACE)) {
                return ParseBlock();
            }

            // Expression (appel, affectation...)
            var expr = ParseExpression();
            SkipNewlines();
            return expr;
        }


        // ── VarDecl ────────────────────────────────────
        private VarDeclNode ParseVarDecl() {
            bool isLet = Check(CodeTranspiler.Lexer.TokenType.KW_LET);
            var  tok   = Advance(); // consume let/var

            // Hint mémoire : let@arena
            string? memHint = null;
            if (Check(CodeTranspiler.Lexer.TokenType.AT)) {
                Advance();
                memHint = Expect(CodeTranspiler.Lexer.TokenType.IDENTIFIER).Value;
            }

            var name = Expect(CodeTranspiler.Lexer.TokenType.IDENTIFIER);
            var node = new VarDeclNode(isLet, name.Value);
            node.SetPosition(tok);
            node.MemoryHint = memHint;

            // Type optionnel : let x: int
            if (Check(CodeTranspiler.Lexer.TokenType.COLON)) {
                Advance();
                node.VarType = ParseTypeRef();
            }

            // Valeur initiale : let x = 5
            if (Check(CodeTranspiler.Lexer.TokenType.OP_EQ)) {
                Advance();
                node.Initial = ParseExpression();
            }

            SkipNewlines();
            return node;
        }


        // ── If ─────────────────────────────────────────
        private IfNode ParseIf() {
            var tok = Expect(CodeTranspiler.Lexer.TokenType.KW_IF);

            Expect(CodeTranspiler.Lexer.TokenType.LPAREN);
            var condition = ParseExpression();
            Expect(CodeTranspiler.Lexer.TokenType.RPAREN);

            var thenBlock = ParseBlock();
            var node      = new IfNode(condition, thenBlock);
            node.SetPosition(tok);

            SkipNewlines();

            // else if / else
            while (Check(CodeTranspiler.Lexer.TokenType.KW_ELSE)) {
                Advance();
                SkipNewlines();

                if (Check(CodeTranspiler.Lexer.TokenType.KW_IF)) {
                    Advance();
                    Expect(CodeTranspiler.Lexer.TokenType.LPAREN);
                    var cond2  = ParseExpression();
                    Expect(CodeTranspiler.Lexer.TokenType.RPAREN);
                    var block2 = ParseBlock();
                    var elseIf = new ElseIfNode(cond2, block2);
                    node.ElseIfs.add(elseIf);
                    SkipNewlines();
                } else {
                    node.ElseBlock = ParseBlock();
                    break;
                }
            }

            return node;
        }


        // ── Match ──────────────────────────────────────
        private MatchNode ParseMatch() {
            var tok     = Expect(CodeTranspiler.Lexer.TokenType.KW_MATCH);
            var subject = ParseExpression();
            var node    = new MatchNode(subject);
            node.SetPosition(tok);

            Expect(CodeTranspiler.Lexer.TokenType.LBRACE);
            SkipNewlines();

            while (!Check(CodeTranspiler.Lexer.TokenType.RBRACE) && !IsEnd()) {
                var arm = ParseMatchArm();
                node.Arms.add(arm);
                SkipNewlines();
            }

            Expect(CodeTranspiler.Lexer.TokenType.RBRACE);
            return node;
        }

        private MatchArmNode ParseMatchArm() {
            var pattern = ParseMatchPattern();

            Expect(CodeTranspiler.Lexer.TokenType.OP_ARROW);

            AstNode body;
            if (Check(CodeTranspiler.Lexer.TokenType.LBRACE)) {
                body = ParseBlock();
            } else {
                body = ParseExpression();
            }

            ConsumeIf(CodeTranspiler.Lexer.TokenType.COMMA);
            SkipNewlines();

            var node = new MatchArmNode(pattern, body);
            return node;
        }

        private MatchPatternNode ParseMatchPattern() {

            // _ wildcard
            if (Check(CodeTranspiler.Lexer.TokenType.IDENTIFIER) &&
                Current().Value == "_") {
                var tok  = Advance();
                var node = new MatchPatternNode(
                               MatchPatternKind.WILDCARD);
                node.SetPosition(tok);
                return node;
            }

            // null
            if (Check(CodeTranspiler.Lexer.TokenType.NULL)) {
                var tok  = Advance();
                var node = new MatchPatternNode(
                               MatchPatternKind.LITERAL);
                var lit  = new NullNode();
                lit.SetPosition(tok);
                node.Value = lit;
                node.SetPosition(tok);
                return node;
            }

            // Littéral ou range
            if (Check(CodeTranspiler.Lexer.TokenType.INTEGER) ||
                Check(CodeTranspiler.Lexer.TokenType.FLOAT)   ||
                Check(CodeTranspiler.Lexer.TokenType.STRING)  ||
                Check(CodeTranspiler.Lexer.TokenType.BOOL_TRUE)||
                Check(CodeTranspiler.Lexer.TokenType.BOOL_FALSE)) {

                var litNode = ParseLiteral();
                var node    = new MatchPatternNode(
                                  MatchPatternKind.LITERAL);
                node.Value  = litNode;
                node.SetPosition(litNode.SourceToken);

                // Range : 1..10
                if (Check(CodeTranspiler.Lexer.TokenType.OP_RANGE)) {
                    Advance();
                    node.Kind     = MatchPatternKind.RANGE;
                    node.RangeEnd = ParseLiteral();
                }

                // Guard : n if condition
                if (Check(CodeTranspiler.Lexer.TokenType.KW_IF)) {
                    Advance();
                    node.Guard = ParseExpression();
                }

                return node;
            }

            // Identifier : Type pattern ou enum variant
            if (Check(CodeTranspiler.Lexer.TokenType.IDENTIFIER)) {
                var nameTok = Advance();
                var node    = new MatchPatternNode(
                                  MatchPatternKind.ENUM_VARIANT);
                node.BindName = nameTok.Value;
                node.SetPosition(nameTok);

                // Destructuring : Player(name, health)
                if (Check(CodeTranspiler.Lexer.TokenType.LPAREN)) {
                    Advance();
                    node.Kind = MatchPatternKind.DESTRUCTURE;
                    while (!Check(CodeTranspiler.Lexer.TokenType.RPAREN) && !IsEnd()) {
                        node.SubPatterns.add(ParseMatchPattern());
                        if (Check(CodeTranspiler.Lexer.TokenType.COMMA)) Advance();
                    }
                    Expect(CodeTranspiler.Lexer.TokenType.RPAREN);
                }

                // Guard
                if (Check(CodeTranspiler.Lexer.TokenType.KW_IF)) {
                    Advance();
                    node.Guard = ParseExpression();
                }

                return node;
            }

            // Pattern inconnu
            var errTok = Current();
            AddError("Pattern invalide : '%s'"
                     .printf(errTok.Value), errTok);
            var errNode = new MatchPatternNode(
                              MatchPatternKind.WILDCARD);
            errNode.SetPosition(errTok);
            Advance();
            return errNode;
        }


        // ── While ──────────────────────────────────────
        private WhileNode ParseWhile() {
            var tok = Expect(CodeTranspiler.Lexer.TokenType.KW_WHILE);
            Expect(CodeTranspiler.Lexer.TokenType.LPAREN);
            var condition = ParseExpression();
            Expect(CodeTranspiler.Lexer.TokenType.RPAREN);
            var body = ParseBlock();
            var node = new WhileNode(condition, body);
            node.SetPosition(tok);
            return node;
        }


        // ── For ────────────────────────────────────────
        private ForNode ParseFor() {
            var tok = Expect(CodeTranspiler.Lexer.TokenType.KW_FOR);
            Expect(CodeTranspiler.Lexer.TokenType.LPAREN);
            var init = ParseVarDecl();
            Expect(CodeTranspiler.Lexer.TokenType.SEMICOLON);
            var condition = ParseExpression();
            Expect(CodeTranspiler.Lexer.TokenType.SEMICOLON);
            var step = ParseExpression();
            Expect(CodeTranspiler.Lexer.TokenType.RPAREN);
            var body = ParseBlock();
            var node = new ForNode(init, condition, step, body);
            node.SetPosition(tok);
            return node;
        }


        // ── Foreach ────────────────────────────────────
        private ForeachNode ParseForeach() {
            var tok = Expect(CodeTranspiler.Lexer.TokenType.KW_FOREACH);
            Expect(CodeTranspiler.Lexer.TokenType.LPAREN);

            bool isLet = Check(CodeTranspiler.Lexer.TokenType.KW_LET) ||
                         Check(CodeTranspiler.Lexer.TokenType.KW_VAR);
            if (isLet) Advance();

            var varName    = Expect(CodeTranspiler.Lexer.TokenType.IDENTIFIER);
            Expect(CodeTranspiler.Lexer.TokenType.KW_IN);
            var collection = ParseExpression();
            Expect(CodeTranspiler.Lexer.TokenType.RPAREN);
            var body       = ParseBlock();

            var node = new ForeachNode(isLet, varName.Value,
                                       collection, body);
            node.SetPosition(tok);
            return node;
        }


        // ── Return ─────────────────────────────────────
        private ReturnNode ParseReturn() {
            var tok  = Expect(CodeTranspiler.Lexer.TokenType.KW_RETURN);
            var node = new ReturnNode();
            node.SetPosition(tok);

            // Valeur optionnelle
            if (!Check(CodeTranspiler.Lexer.TokenType.NEWLINE) &&
                !Check(CodeTranspiler.Lexer.TokenType.RBRACE)  &&
                !Check(CodeTranspiler.Lexer.TokenType.EOF)     &&
                !IsEnd()) {
                node.Value = ParseExpression();
            }

            SkipNewlines();
            return node;
        }


        // ── Guard ──────────────────────────────────────
        private GuardNode ParseGuard() {
            var tok       = Expect(CodeTranspiler.Lexer.TokenType.KW_GUARD);
            var condition = ParseExpression();
            Expect(CodeTranspiler.Lexer.TokenType.KW_ELSE);
            var elseBlock = ParseBlock();
            var node      = new GuardNode(condition, elseBlock);
            node.SetPosition(tok);
            SkipNewlines();
            return node;
        }


        // ── Try/Catch ──────────────────────────────────
        private TryCatchNode ParseTryCatch() {
            var tok      = Expect(CodeTranspiler.Lexer.TokenType.KW_TRY);
            var tryBlock = ParseBlock();

            Expect(CodeTranspiler.Lexer.TokenType.KW_CATCH);
            Expect(CodeTranspiler.Lexer.TokenType.LPAREN);
            var errType = Expect(CodeTranspiler.Lexer.TokenType.IDENTIFIER).Value;
            var errName = Expect(CodeTranspiler.Lexer.TokenType.IDENTIFIER).Value;
            Expect(CodeTranspiler.Lexer.TokenType.RPAREN);

            var catchBlock = ParseBlock();
            var node = new TryCatchNode(tryBlock, errType,
                                        errName, catchBlock);
            node.SetPosition(tok);
            return node;
        }


        // ── Go (goroutine) ─────────────────────────────
        private GoStmtNode ParseGoStmt() {
            var tok  = Expect(CodeTranspiler.Lexer.TokenType.KW_GO);
            var expr = ParseExpression();
            var node = new GoStmtNode(expr);
            node.SetPosition(tok);
            SkipNewlines();
            return node;
        }


        // ═══════════════════════════════════════════════
        //  EXPRESSIONS
        //  Précédence (du plus faible au plus fort) :
        //  1. Affectation    = += -= *= /=
        //  2. Pipeline       |>
        //  3. Null coalesc.  ??
        //  4. Or             ||
        //  5. And            &&
        //  6. Égalité        == !=
        //  7. Comparaison    < > <= >= is
        //  8. Range          ..
        //  9. Addition       + -
        // 10. Multiplication * / %
        // 11. Unaire         ! -
        // 12. Puissance      ^
        // 13. Postfixe       . ?. [] () with
        // 14. Primaire       lit, id, new, lambda...
        // ═══════════════════════════════════════════════

        private AstNode ParseExpression() {
            return ParseAssignment();
        }

        // ── Affectation ────────────────────────────────
        private AstNode ParseAssignment() {
            var left = ParsePipeline();

            if (Check(CodeTranspiler.Lexer.TokenType.OP_EQ)      ||
                CheckOp("+=") || CheckOp("-=") ||
                CheckOp("*=") || CheckOp("/=")) {
                var op    = Advance().Value;
                var right = ParseAssignment();
                var node  = new AssignExprNode(left, op, right);
                node.SetPosition(left.SourceToken);
                return node;
            }

            return left;
        }

        // ── Pipeline |> ────────────────────────────────
        private AstNode ParsePipeline() {
            var left = ParseNullCoal();

            while (Check(CodeTranspiler.Lexer.TokenType.OP_PIPE)) {
                var op    = Advance();
                var right = ParseNullCoal();
                var node  = new BinaryExprNode(left, "|>", right);
                node.SetPosition(op);
                left = node;
            }

            return left;
        }

        // ── Null Coalescence ?? ────────────────────────
        private AstNode ParseNullCoal() {
            var left = ParseOr();

            while (Check(CodeTranspiler.Lexer.TokenType.OP_NULLCOAL)) {
                var op    = Advance();
                var right = ParseOr();
                var node  = new BinaryExprNode(left, "??", right);
                node.SetPosition(op);
                left = node;
            }

            return left;
        }

        // ── Or || ──────────────────────────────────────
        private AstNode ParseOr() {
            var left = ParseAnd();

            while (Check(CodeTranspiler.Lexer.TokenType.OP_OR)) {
                var op    = Advance();
                var right = ParseAnd();
                var node  = new BinaryExprNode(left, "||", right);
                node.SetPosition(op);
                left = node;
            }

            return left;
        }

        // ── And && ─────────────────────────────────────
        private AstNode ParseAnd() {
            var left = ParseEquality();

            while (Check(CodeTranspiler.Lexer.TokenType.OP_AND)) {
                var op    = Advance();
                var right = ParseEquality();
                var node  = new BinaryExprNode(left, "&&", right);
                node.SetPosition(op);
                left = node;
            }

            return left;
        }

        // ── Égalité == != ──────────────────────────────
        private AstNode ParseEquality() {
            var left = ParseRelational();

            while (Check(CodeTranspiler.Lexer.TokenType.OP_EQEQ) ||
                   Check(CodeTranspiler.Lexer.TokenType.OP_NEQ)) {
                var op    = Advance();
                var right = ParseRelational();
                var node  = new BinaryExprNode(
                                left, op.Value, right);
                node.SetPosition(op);
                left = node;
            }

            return left;
        }

        // ── Comparaison < > <= >= is ───────────────────
        private AstNode ParseRelational() {
            var left = ParseRange();

            while (Check(CodeTranspiler.Lexer.TokenType.OP_LT)  ||
                   Check(CodeTranspiler.Lexer.TokenType.OP_GT)  ||
                   Check(CodeTranspiler.Lexer.TokenType.OP_LTE) ||
                   Check(CodeTranspiler.Lexer.TokenType.OP_GTE) ||
                   CheckKw("is")) {
                var op    = Advance();
                var right = ParseRange();
                var node  = new BinaryExprNode(
                                left, op.Value, right);
                node.SetPosition(op);
                left = node;
            }

            return left;
        }

        // ── Range .. ───────────────────────────────────
        private AstNode ParseRange() {
            var left = ParseAddition();

            if (Check(CodeTranspiler.Lexer.TokenType.OP_RANGE)) {
                var op    = Advance();
                var right = ParseAddition();
                var node  = new BinaryExprNode(left, "..", right);
                node.SetPosition(op);
                return node;
            }

            return left;
        }

        // ── Addition + - ───────────────────────────────
        private AstNode ParseAddition() {
            var left = ParseMultiplication();

            while (Check(CodeTranspiler.Lexer.TokenType.OP_PLUS) ||
                   Check(CodeTranspiler.Lexer.TokenType.OP_MINUS)) {
                var op    = Advance();
                var right = ParseMultiplication();
                var node  = new BinaryExprNode(
                                left, op.Value, right);
                node.SetPosition(op);
                left = node;
            }

            return left;
        }

        // ── Multiplication * / % ───────────────────────
        private AstNode ParseMultiplication() {
            var left = ParseUnary();

            while (Check(CodeTranspiler.Lexer.TokenType.OP_STAR)    ||
                   Check(CodeTranspiler.Lexer.TokenType.OP_SLASH)   ||
                   Check(CodeTranspiler.Lexer.TokenType.OP_PERCENT)) {
                var op    = Advance();
                var right = ParseUnary();
                var node  = new BinaryExprNode(
                                left, op.Value, right);
                node.SetPosition(op);
                left = node;
            }

            return left;
        }

        // ── Unaire ! - ─────────────────────────────────
        private AstNode ParseUnary() {
            if (Check(CodeTranspiler.Lexer.TokenType.OP_NOT) ||
                Check(CodeTranspiler.Lexer.TokenType.OP_MINUS)) {
                var op      = Advance();
                var operand = ParseUnary();
                var node    = new UnaryExprNode(
                                  op.Value, operand, true);
                node.SetPosition(op);
                return node;
            }

            return ParsePower();
        }

        // ── Puissance ^ ────────────────────────────────
        private AstNode ParsePower() {
            var left = ParsePostfix();

            if (Check(CodeTranspiler.Lexer.TokenType.OP_POWER)) {
                var op    = Advance();
                var right = ParseUnary(); // droite-associatif
                var node  = new BinaryExprNode(left, "^", right);
                node.SetPosition(op);
                return node;
            }

            return left;
        }

        // ── Postfixe . ?. [] () with ───────────────────
        private AstNode ParsePostfix() {
            var expr = ParsePrimary();

            while (true) {

                // Accès membre : .Name
                if (Check(CodeTranspiler.Lexer.TokenType.DOT)) {
                    Advance();
                    var member = Expect(CodeTranspiler.Lexer.TokenType.IDENTIFIER);
                    var node   = new MemberAccessNode(
                                     expr, member.Value, false);
                    node.SetPosition(member);
                    expr = node;
                    continue;
                }

                // Accès null-safe : ?.Name
                if (Check(CodeTranspiler.Lexer.TokenType.OP_NULLSAFE)) {
                    Advance();
                    var member = Expect(CodeTranspiler.Lexer.TokenType.IDENTIFIER);
                    var node   = new MemberAccessNode(
                                     expr, member.Value, true);
                    node.SetPosition(member);
                    expr = node;
                    continue;
                }

                // Appel : (args)
                if (Check(CodeTranspiler.Lexer.TokenType.LPAREN)) {
                    var tok  = Advance();
                    var call = new CallExprNode(expr);
                    call.SetPosition(tok);

                    if (!Check(CodeTranspiler.Lexer.TokenType.RPAREN)) {
                        ParseArgList(call);
                    }
                    Expect(CodeTranspiler.Lexer.TokenType.RPAREN);
                    expr = call;
                    continue;
                }

                // Index : [expr]
                if (Check(CodeTranspiler.Lexer.TokenType.LBRACKET)) {
                    var tok   = Advance();
                    var index = ParseExpression();
                    Expect(CodeTranspiler.Lexer.TokenType.RBRACKET);
                    var node  = new IndexExprNode(expr, index);
                    node.SetPosition(tok);
                    expr = node;
                    continue;
                }

                // With : expr with { X = 5 }
                if (CheckKw("with")) {
                    var tok  = Advance();
                    var with = new WithExprNode(expr);
                    with.SetPosition(tok);
                    Expect(CodeTranspiler.Lexer.TokenType.LBRACE);
                    while (!Check(CodeTranspiler.Lexer.TokenType.RBRACE) && !IsEnd()) {
                        var field = Expect(CodeTranspiler.Lexer.TokenType.IDENTIFIER);
                        Expect(CodeTranspiler.Lexer.TokenType.OP_EQ);
                        var val = ParseExpression();
                        with.Changes[field.Value] = val;
                        ConsumeIf(CodeTranspiler.Lexer.TokenType.COMMA);
                        SkipNewlines();
                    }
                    Expect(CodeTranspiler.Lexer.TokenType.RBRACE);
                    expr = with;
                    continue;
                }

                break;
            }

            return expr;
        }

        private void ParseArgList(CallExprNode call) {
            // Premier argument
            ParseArg(call);
            while (Check(CodeTranspiler.Lexer.TokenType.COMMA)) {
                Advance();
                SkipNewlines();
                ParseArg(call);
            }
        }

        private void ParseArg(CallExprNode call) {
            // Argument nommé : name: value
            if (Check(CodeTranspiler.Lexer.TokenType.IDENTIFIER) &&
                PeekType(1) == CodeTranspiler.Lexer.TokenType.COLON) {
                var name = Advance().Value;
                Advance(); // consume :
                var val  = ParseExpression();
                call.NamedArgs[name] = val;
            } else {
                call.Arguments.add(ParseExpression());
            }
        }


        // ── Primaire ───────────────────────────────────
        private AstNode ParsePrimary() {

            // Littéraux
            if (Check(CodeTranspiler.Lexer.TokenType.INTEGER) ||
                Check(CodeTranspiler.Lexer.TokenType.FLOAT)   ||
                Check(CodeTranspiler.Lexer.TokenType.STRING)  ||
                Check(CodeTranspiler.Lexer.TokenType.BOOL_TRUE)||
                Check(CodeTranspiler.Lexer.TokenType.BOOL_FALSE)) {
                return ParseLiteral();
            }

            // null
            if (Check(CodeTranspiler.Lexer.TokenType.NULL)) {
                var tok  = Advance();
                var node = new NullNode();
                node.SetPosition(tok);
                return node;
            }

            // this
            if (Check(CodeTranspiler.Lexer.TokenType.KW_THIS)) {
                var tok  = Advance();
                var node = new ThisNode();
                node.SetPosition(tok);
                return node;
            }

            // new MyClass(...)
            if (Check(CodeTranspiler.Lexer.TokenType.KW_NEW)) {
                return ParseNewExpr();
            }

            // await expr
            if (Check(CodeTranspiler.Lexer.TokenType.KW_AWAIT)) {
                var tok  = Advance();
                var expr = ParseExpression();
                var node = new AwaitExprNode(expr);
                node.SetPosition(tok);
                return node;
            }

            // match expr { }
            if (Check(CodeTranspiler.Lexer.TokenType.KW_MATCH)) {
                return ParseMatch();
            }

            // Lambda : (x) => expr  ou  x => expr
            if (IsLambdaStart()) {
                return ParseLambda();
            }

            // Liste : [1, 2, 3]
            if (Check(CodeTranspiler.Lexer.TokenType.LBRACKET)) {
                return ParseListLiteral();
            }

            // Groupe : (expr)
            if (Check(CodeTranspiler.Lexer.TokenType.LPAREN)) {
                Advance();
                var expr = ParseExpression();
                Expect(CodeTranspiler.Lexer.TokenType.RPAREN);
                return expr;
            }

            // Identifiant
            if (Check(CodeTranspiler.Lexer.TokenType.IDENTIFIER)) {
                var tok  = Advance();
                var node = new IdentifierNode(tok.Value);
                node.SetPosition(tok);
                return node;
            }

            // Erreur
            var errTok = Current();
            AddError("Expression inattendue : '%s'"
                     .printf(errTok.Value), errTok);
            var errNode = new IdentifierNode("__error__");
            errNode.SetPosition(errTok);
            if (!IsEnd()) Advance();
            return errNode;
        }


        // ── Littéral ───────────────────────────────────
        private LiteralNode ParseLiteral() {
            var tok  = Advance();
            LiteralKind kind;

            switch (tok.Type) {
                case CodeTranspiler.Lexer.TokenType.INTEGER:
                    kind = LiteralKind.INTEGER;
                    break;
                case CodeTranspiler.Lexer.TokenType.FLOAT:
                    kind = LiteralKind.FLOAT;
                    break;
                case CodeTranspiler.Lexer.TokenType.STRING:
                    kind = LiteralKind.STRING;
                    break;
                case CodeTranspiler.Lexer.TokenType.BOOL_TRUE:
                case CodeTranspiler.Lexer.TokenType.BOOL_FALSE:
                    kind = LiteralKind.BOOL;
                    break;
                default:
                    kind = LiteralKind.STRING;
                    break;
            }

            var node = new LiteralNode(kind, tok.Value);
            node.SetPosition(tok);

            // Valeurs typées
            if (kind == LiteralKind.INTEGER) {
                node.IntValue = int64.parse(tok.Value);
            } else if (kind == LiteralKind.FLOAT) {
                node.FloatValue = double.parse(tok.Value);
            } else if (kind == LiteralKind.BOOL) {
                node.BoolValue = tok.Type == CodeTranspiler.Lexer.TokenType.BOOL_TRUE;
            } else if (kind == LiteralKind.STRING) {
                node.StringValue = tok.Value;
            }

            return node;
        }


        // ── New ────────────────────────────────────────
        private NewExprNode ParseNewExpr() {
            var tok  = Expect(CodeTranspiler.Lexer.TokenType.KW_NEW);
            var type = ParseTypeRef();
            var node = new NewExprNode(type);
            node.SetPosition(tok);

            Expect(CodeTranspiler.Lexer.TokenType.LPAREN);
            if (!Check(CodeTranspiler.Lexer.TokenType.RPAREN)) {
                ParseNewArgList(node);
            }
            Expect(CodeTranspiler.Lexer.TokenType.RPAREN);

            return node;
        }

        private void ParseNewArgList(NewExprNode node) {
            ParseNewArg(node);
            while (Check(CodeTranspiler.Lexer.TokenType.COMMA)) {
                Advance();
                SkipNewlines();
                ParseNewArg(node);
            }
        }

        private void ParseNewArg(NewExprNode node) {
            if (Check(CodeTranspiler.Lexer.TokenType.IDENTIFIER) &&
                PeekType(1) == CodeTranspiler.Lexer.TokenType.COLON) {
                var name = Advance().Value;
                Advance();
                var val  = ParseExpression();
                node.NamedArgs[name] = val;
            } else {
                node.Arguments.add(ParseExpression());
            }
        }


        // ── Lambda ─────────────────────────────────────
        private LambdaExprNode ParseLambda() {
            var node = new LambdaExprNode();

            if (Check(CodeTranspiler.Lexer.TokenType.LPAREN)) {
                // (x, y) => expr
                var tok = Advance();
                node.SetPosition(tok);
                if (!Check(CodeTranspiler.Lexer.TokenType.RPAREN)) {
                    ParseParamList(node.Params);
                }
                Expect(CodeTranspiler.Lexer.TokenType.RPAREN);
            } else {
                // x => expr  (param implicitement typé)
                var nameTok = Expect(CodeTranspiler.Lexer.TokenType.IDENTIFIER);
                node.SetPosition(nameTok);
                var type    = new SimpleTypeNode("var");
                var param   = new ParamNode(nameTok.Value, type);
                param.SetPosition(nameTok);
                node.Params.add(param);
            }

            Expect(CodeTranspiler.Lexer.TokenType.OP_ARROW);

            if (Check(CodeTranspiler.Lexer.TokenType.LBRACE)) {
                node.Body = ParseBlock();
            } else {
                node.Body = ParseExpression();
            }

            return node;
        }


        // ── Liste ──────────────────────────────────────
        private ListLiteralNode ParseListLiteral() {
            var tok  = Expect(CodeTranspiler.Lexer.TokenType.LBRACKET);
            var node = new ListLiteralNode();
            node.SetPosition(tok);

            if (!Check(CodeTranspiler.Lexer.TokenType.RBRACKET)) {
                var first = ParseExpression();

                // Compréhension : [x * x for x in list]
                if (CheckKw("for")) {
                    Advance();
                    var varName = Expect(CodeTranspiler.Lexer.TokenType.IDENTIFIER);
                    Expect(CodeTranspiler.Lexer.TokenType.KW_IN);
                    var source  = ParseExpression();

                    node.IsComprehension = true;
                    node.CompExpr        = first;
                    node.CompVarName     = varName.Value;
                    node.CompSource      = source;

                    if (Check(CodeTranspiler.Lexer.TokenType.KW_IF)) {
                        Advance();
                        node.CompFilter = ParseExpression();
                    }
                } else {
                    // Liste normale
                    node.Elements.add(first);
                    while (Check(CodeTranspiler.Lexer.TokenType.COMMA)) {
                        Advance();
                        SkipNewlines();
                        if (Check(CodeTranspiler.Lexer.TokenType.RBRACKET)) break;
                        node.Elements.add(ParseExpression());
                    }
                }
            }

            Expect(CodeTranspiler.Lexer.TokenType.RBRACKET);
            return node;
        }


        // ═══════════════════════════════════════════════
        //  TYPES
        // ═══════════════════════════════════════════════

        private TypeNode ParseTypeRef() {

            // Func<int, int, bool>
            if (CheckKw("Func")) {
                return ParseFuncType();
            }

            // Type de base ou générique
            var name = ParseQualifiedName();

            // Générique : List<Player>
            if (Check(CodeTranspiler.Lexer.TokenType.OP_LT)) {
                Advance();
                var gen = new GenericTypeNode(name);
                gen.TypeArgs.add(ParseTypeRef());
                while (Check(CodeTranspiler.Lexer.TokenType.COMMA)) {
                    Advance();
                    gen.TypeArgs.add(ParseTypeRef());
                }
                Expect(CodeTranspiler.Lexer.TokenType.OP_GT);

                // Nullable : List<Player>?
                gen.IsNullable = ConsumeIf(CodeTranspiler.Lexer.TokenType.QUESTION);
                return gen;
            }

            // Simple : int, string, Player
            var simple     = new SimpleTypeNode(name);
            simple.IsNullable = ConsumeIf(CodeTranspiler.Lexer.TokenType.QUESTION);
            return simple;
        }

        private FuncTypeNode ParseFuncType() {
            Advance(); // consume "Func"
            Expect(CodeTranspiler.Lexer.TokenType.OP_LT);

            var types = new Gee.ArrayList<TypeNode>();
            types.add(ParseTypeRef());
            while (Check(CodeTranspiler.Lexer.TokenType.COMMA)) {
                Advance();
                types.add(ParseTypeRef());
            }

            Expect(CodeTranspiler.Lexer.TokenType.OP_GT);

            // Dernier type = type de retour
            var returnType = types.remove_at(types.size - 1);
            var node       = new FuncTypeNode(returnType);
            foreach (var t in types) {
                node.ParamTypes.add(t);
            }

            return node;
        }

        private Gee.ArrayList<GenericParam> ParseGenericParams() {
            var result = new Gee.ArrayList<GenericParam>();
            Expect(CodeTranspiler.Lexer.TokenType.OP_LT);

            var param = new GenericParam(
                Expect(CodeTranspiler.Lexer.TokenType.IDENTIFIER).Value);
            if (Check(CodeTranspiler.Lexer.TokenType.COLON)) {
                Advance();
                param.Constraints.add(ParseTypeRef());
            }
            result.add(param);

            while (Check(CodeTranspiler.Lexer.TokenType.COMMA)) {
                Advance();
                var p2 = new GenericParam(
                    Expect(CodeTranspiler.Lexer.TokenType.IDENTIFIER).Value);
                if (Check(CodeTranspiler.Lexer.TokenType.COLON)) {
                    Advance();
                    p2.Constraints.add(ParseTypeRef());
                }
                result.add(p2);
            }

            Expect(CodeTranspiler.Lexer.TokenType.OP_GT);
            return result;
        }


        // ═══════════════════════════════════════════════
        //  HELPERS
        // ═══════════════════════════════════════════════

        // ── Navigation ─────────────────────────────────

        private Token Current() {
            if (_pos >= _tokens.size) {
                return _tokens[_tokens.size - 1]; // EOF
            }
            return _tokens[_pos];
        }

        private Token Peek(int offset = 1) {
            int idx = _pos + offset;
            if (idx >= _tokens.size) {
                return _tokens[_tokens.size - 1];
            }
            return _tokens[idx];
        }

        private CodeTranspiler.Lexer.TokenType PeekType(int offset = 1) {
            return Peek(offset).Type;
        }

        private Token Advance() {
            var tok = Current();
            if (_pos < _tokens.size - 1) _pos++;
            return tok;
        }

        private bool Check(CodeTranspiler.Lexer.TokenType type) {
            return Current().Type == type;
        }

        private bool CheckOp(string op) {
            return Current().Type == CodeTranspiler.Lexer.TokenType.OP_EQ &&
                   Current().Value == op;
        }

        private bool CheckKw(string kw) {
            return Current().Type == CodeTranspiler.Lexer.TokenType.IDENTIFIER &&
                   Current().Value == kw;
        }

        private Token Expect(CodeTranspiler.Lexer.TokenType type) {
            if (!Check(type)) {
                AddError(
                    "Attendu '%s', trouvé '%s'"
                    .printf(type.to_string(),
                             Current().Value),
                    Current()
                );
            }
            return Advance();
        }

        private bool ConsumeIf(CodeTranspiler.Lexer.TokenType type) {
            if (Check(type)) { Advance(); return true; }
            return false;
        }

        private bool IsEnd() {
            return Current().Type == CodeTranspiler.Lexer.TokenType.EOF;
        }

        private void SkipNewlines() {
            while (Check(CodeTranspiler.Lexer.TokenType.NEWLINE) && !IsEnd()) {
                Advance();
            }
        }

        private void AddError(string msg, Token tok) {
            _errors.add(new ParseError(msg, tok));
        }

        // ── Détection de contexte ──────────────────────

        private bool CheckMethodStart() {
            return Check(CodeTranspiler.Lexer.TokenType.IDENTIFIER) ||
                   Check(CodeTranspiler.Lexer.TokenType.KW_ASYNC)   ||
                   Check(CodeTranspiler.Lexer.TokenType.KW_PURE)    ||
                   Check(CodeTranspiler.Lexer.TokenType.KW_STATIC);
        }

        private bool IsLambdaStart() {
            // x => expr
            if (Check(CodeTranspiler.Lexer.TokenType.IDENTIFIER) &&
                PeekType(1) == CodeTranspiler.Lexer.TokenType.OP_ARROW) {
                return true;
            }

            // (x) => expr  ou  () => expr
            if (Check(CodeTranspiler.Lexer.TokenType.LPAREN)) {
                int i = 1;
                int depth = 1;
                while (i < _tokens.size - _pos && depth > 0) {
                    var t = Peek(i);
                    if (t.Type == CodeTranspiler.Lexer.TokenType.LPAREN) depth++;
                    if (t.Type == CodeTranspiler.Lexer.TokenType.RPAREN) depth--;
                    i++;
                }
                // Après le ) fermant, on doit avoir =>
                if (Peek(i).Type == CodeTranspiler.Lexer.TokenType.OP_ARROW) {
                    return true;
                }
            }

            return false;
        }

        // ── Nom qualifié ───────────────────────────────

        private string ParseQualifiedName() {
            var sb = new StringBuilder();
            sb.append(Expect(CodeTranspiler.Lexer.TokenType.IDENTIFIER).Value);
            while (Check(CodeTranspiler.Lexer.TokenType.DOT) &&
                   PeekType(1) == CodeTranspiler.Lexer.TokenType.IDENTIFIER) {
                Advance(); // consume .
                sb.append(".");
                sb.append(Advance().Value);
            }
            return sb.str;
        }

        // ── Modificateur d'accès ───────────────────────

        private AccessModifier ParseAccessModifier() {
            if (ConsumeIf(CodeTranspiler.Lexer.TokenType.KW_PUBLIC))    {
                return AccessModifier.PUBLIC;
            }
            if (ConsumeIf(CodeTranspiler.Lexer.TokenType.KW_PRIVATE))   {
                return AccessModifier.PRIVATE;
            }
            if (ConsumeIf(CodeTranspiler.Lexer.TokenType.KW_PROTECTED)) {
                return AccessModifier.PROTECTED;
            }
            if (ConsumeIf(CodeTranspiler.Lexer.TokenType.KW_INTERNAL))  {
                return AccessModifier.INTERNAL;
            }
            return AccessModifier.INTERNAL; // défaut
        }
    }
}
