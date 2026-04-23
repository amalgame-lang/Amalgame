// ─────────────────────────────────────────────────────
//  CODE Programming Language
//  Copyright (c) 2024 NeitsabTeguom
//  Licensed under MIT OR Apache 2.0
// ─────────────────────────────────────────────────────
//  c_generator.vala  -  Générateur de code C depuis AST
//
//  Parcourt l'AST avec le Visitor pattern
//  et produit du code C valide.
// ─────────────────────────────────────────────────────

namespace CodeTranspiler.Generator {

    using CodeTranspiler.Ast;
    using CodeTranspiler.Lexer;


    // ═══════════════════════════════════════════════════
    //  Résultat de la génération
    // ═══════════════════════════════════════════════════

    public class GeneratorResult : Object {
        public bool   Success  { get; set; }
        public string CCode    { get; set; }
        public string Errors   { get; set; }

        public GeneratorResult() {
            CCode  = "";
            Errors = "";
        }
    }


    // ═══════════════════════════════════════════════════
    //  Le Générateur C
    // ═══════════════════════════════════════════════════

    /**
     * Génère du code C depuis un AST CODE.
     *
     * Usage :
     *   var gen    = new CGenerator("myfile.code");
     *   var result = gen.Generate(ast);
     *   if (result.Success) {
     *       FileUtils.set_contents("out.c", result.CCode);
     *   }
     */
    public class CGenerator : BaseAstVisitor {

        // ── État interne ───────────────────────────────
        private StringBuilder _out;         // code C produit
        private int           _indent;      // niveau d'indentation
        private string        _sourceFile;  // fichier .code source
        private int           _sourceLine;  // ligne courante source
        private bool          _inClass;     // dans une classe ?
        private string        _className;   // classe courante
        private bool          _hasErrors;
        private StringBuilder _errors;


        public CGenerator(string sourceFile) {
            _sourceFile = sourceFile;
            _out        = new StringBuilder();
            _errors     = new StringBuilder();
            _indent     = 0;
            _inClass    = false;
            _className  = "";
            _hasErrors  = false;
            _sourceLine = 0;
        }


        // ═══════════════════════════════════════════════
        //  Point d'entrée
        // ═══════════════════════════════════════════════

        public GeneratorResult Generate(ProgramNode ast) {
            var result = new GeneratorResult();

            // En-tête du fichier C généré
            EmitHeader();

            // Visiter l'AST
            ast.Accept(this);

            // Pied de page
            EmitFooter();

            result.CCode   = _out.str;
            result.Success = !_hasErrors;
            result.Errors  = _errors.str;

            return result;
        }


        // ═══════════════════════════════════════════════
        //  En-tête et pied de page
        // ═══════════════════════════════════════════════

        private void EmitHeader() {
            Emit("/* ═══════════════════════════════════\n");
            Emit(" * Généré par CODE Transpiler v0.1.0\n");
            Emit(" * Source : %s\n".printf(_sourceFile));
            Emit(" * NE PAS MODIFIER MANUELLEMENT\n");
            Emit(" * ═══════════════════════════════════\n");
            Emit(" */\n\n");
            Emit("#include \"_runtime.h\"\n\n");
        }

        private void EmitFooter() {
            Emit("\n/* ── Point d entree C ── */\n");
            Emit("int main(int argc, char** argv) {\n");
            Emit("    code_runtime_init();\n");
            Emit("    Program_Main(argc, argv);\n");
            Emit("    return 0;\n");
            Emit("}\n");
        }


        // ═══════════════════════════════════════════════
        //  Programme
        // ═══════════════════════════════════════════════

        public override void VisitProgram(ProgramNode n) {
            // Namespace → commentaire
            n.Namespace.Accept(this);

            // Imports → commentaires
            foreach (var imp in n.Imports) {
                imp.Accept(this);
            }

            Emit("\n");

            // Forward declarations
            EmitForwardDecls(n);

            Emit("\n");

            // Déclarations
            foreach (var decl in n.Declarations) {
                decl.Accept(this);
                Emit("\n");
            }
        }

        public override void VisitNamespace(NamespaceNode n) {
            Emit("/* namespace %s */\n".printf(n.Name));
        }

        public override void VisitImport(ImportNode n) {
            Emit("/* import %s */\n".printf(n.Name));
        }

        /**
         * Génère les forward declarations pour les structs
         * et les méthodes statiques
         */
        private void EmitForwardDecls(ProgramNode n) {
            Emit("/* ── Forward Declarations ── */\n");
            foreach (var decl in n.Declarations) {
                if (decl is ClassDeclNode) {
                    var c = (ClassDeclNode) decl;
                    Emit("typedef struct _%s %s;\n"
                         .printf(c.Name, c.Name));
                    // Forward decl des méthodes statiques
                    foreach (var m in c.Members) {
                        if (m is MethodDeclNode) {
                            var md = (MethodDeclNode) m;
                            if (md.IsStatic) {
                                string ret = md.ReturnType != null
                                    ? TypeToC(md.ReturnType)
                                    : "void";
                                Emit("static %s %s_%s();
"
                                     .printf(ret, c.Name,
                                              md.Name));
                            }
                        }
                    }
                } else if (decl is RecordDeclNode) {
                    var r = (RecordDeclNode) decl;
                    Emit("typedef struct _%s %s;\n"
                         .printf(r.Name, r.Name));
                } else if (decl is DataClassDeclNode) {
                    var d = (DataClassDeclNode) decl;
                    Emit("typedef struct _%s %s;\n"
                         .printf(d.Name, d.Name));
                }
            }
            Emit("\n");
        }


        // ═══════════════════════════════════════════════
        //  Classes
        // ═══════════════════════════════════════════════

        public override void VisitClassDecl(ClassDeclNode n) {
            EmitLine(n.Line);
            _inClass   = true;
            _className = n.Name;

            // Struct C pour la classe
            Emit("/* class %s */\n".printf(n.Name));
            Emit("struct _%s {\n".printf(n.Name));
            _indent++;

            // Champs et propriétés
            foreach (var member in n.Members) {
                if (member is FieldDeclNode) {
                    member.Accept(this);
                } else if (member is PropertyDeclNode) {
                    var prop = (PropertyDeclNode) member;
                    EmitI("%s %s;\n".printf(
                        TypeToC(prop.PropType),
                        prop.Name
                    ));
                }
            }

            _indent--;
            Emit("};\n\n");

            // Constructeur par défaut
            EmitDefaultConstructor(n);

            // Méthodes
            foreach (var member in n.Members) {
                if (member is MethodDeclNode) {
                    var method = (MethodDeclNode) member;
                    EmitMethod(n.Name, method);
                } else if (member is ConstructorDeclNode) {
                    EmitConstructor(n.Name,
                        (ConstructorDeclNode) member);
                }
            }

            _inClass   = false;
            _className = "";
        }

        private void EmitDefaultConstructor(ClassDeclNode n) {
            Emit("%s* %s_new() {\n".printf(n.Name, n.Name));
            _indent++;
            EmitI("%s* self = (%s*) code_alloc(sizeof(%s));\n"
                  .printf(n.Name, n.Name, n.Name));

            // Initialiser les champs avec valeurs par défaut
            foreach (var member in n.Members) {
                if (member is FieldDeclNode) {
                    var f = (FieldDeclNode) member;
                    if (f.Initial != null) {
                        EmitI("self->%s = ".printf(f.Name));
                        f.Initial.Accept(this);
                        Emit(";\n");
                    }
                }
            }

            EmitI("return self;\n");
            _indent--;
            Emit("}\n\n");
        }

        private void EmitConstructor(string className,
                                      ConstructorDeclNode n) {
            // Prototype
            Emit("%s* %s_create(".printf(className, className));
            EmitParamList(n.Params);
            Emit(") {\n");
            _indent++;

            EmitI("%s* self = (%s*) code_alloc(sizeof(%s));\n"
                  .printf(className, className, className));

            // Corps
            foreach (var stmt in n.Body.Statements) {
                EmitI("");
                stmt.Accept(this);
                Emit("\n");
            }

            EmitI("return self;\n");
            _indent--;
            Emit("}\n\n");
        }

        private void EmitMethod(string className,
                                 MethodDeclNode n) {
            EmitLine(n.Line);

            // Type de retour
            string retType = n.ReturnType != null
                ? TypeToC(n.ReturnType) : "void";

            // Prototype
            if (n.IsStatic) {
                Emit("static %s %s_%s(".printf(
                    retType, className, n.Name));
                EmitParamList(n.Params);
                Emit(") ");
            } else {
                Emit("%s %s_%s(%s* self".printf(
                    retType, className, n.Name, className));
                if (n.Params.size > 0) {
                    Emit(", ");
                    EmitParamList(n.Params);
                }
                Emit(") ");
            }

            // Corps
            if (n.Body != null) {
                if (n.Body is BlockNode) {
                    n.Body.Accept(this);
                } else {
                    Emit("{\n");
                    _indent++;
                    EmitI("return ");
                    n.Body.Accept(this);
                    Emit(";\n");
                    _indent--;
                    Emit("}");
                }
            } else {
                Emit("{}");
            }

            Emit("\n\n");
        }

        private void EmitParamList(
            Gee.ArrayList<ParamNode> parms) {

            for (int i = 0; i < parms.size; i++) {
                if (i > 0) Emit(", ");
                var p    = parms[i];
                var cType = TypeToC(p.ParamType);

                // string[] args → int argc, char** argv
                if (cType == "code_string*" &&
                    p.Name == "args") {
                    Emit("int argc, char** argv");
                } else {
                    Emit("%s %s".printf(cType, p.Name));
                }
            }
        }


        // ═══════════════════════════════════════════════
        //  Records et Data Classes
        // ═══════════════════════════════════════════════

        public override void VisitRecordDecl(RecordDeclNode n) {
            EmitLine(n.Line);
            Emit("/* record %s */\n".printf(n.Name));
            Emit("struct _%s {\n".printf(n.Name));
            _indent++;

            foreach (var p in n.Params) {
                EmitI("%s %s;\n".printf(
                    TypeToC(p.ParamType), p.Name));
            }

            _indent--;
            Emit("};\n\n");

            // Constructeur
            Emit("%s* %s_new(".printf(n.Name, n.Name));
            for (int i = 0; i < n.Params.size; i++) {
                if (i > 0) Emit(", ");
                var p = n.Params[i];
                Emit("%s %s".printf(TypeToC(p.ParamType), p.Name));
            }
            Emit(") {\n");
            _indent++;
            EmitI("%s* self = (%s*) code_alloc(sizeof(%s));\n"
                  .printf(n.Name, n.Name, n.Name));
            foreach (var p in n.Params) {
                EmitI("self->%s = %s;\n".printf(p.Name, p.Name));
            }
            EmitI("return self;\n");
            _indent--;
            Emit("}\n\n");
        }

        public override void VisitDataClassDecl(
            DataClassDeclNode n) {

            EmitLine(n.Line);
            Emit("/* data class %s */\n".printf(n.Name));
            Emit("struct _%s {\n".printf(n.Name));
            _indent++;

            foreach (var p in n.Params) {
                EmitI("%s %s;\n".printf(
                    TypeToC(p.ParamType), p.Name));
            }

            _indent--;
            Emit("};\n\n");

            // Constructeur
            Emit("%s* %s_new(".printf(n.Name, n.Name));
            for (int i = 0; i < n.Params.size; i++) {
                if (i > 0) Emit(", ");
                var p = n.Params[i];
                Emit("%s %s".printf(TypeToC(p.ParamType), p.Name));
            }
            Emit(") {\n");
            _indent++;
            EmitI("%s* self = (%s*) code_alloc(sizeof(%s));\n"
                  .printf(n.Name, n.Name, n.Name));
            foreach (var p in n.Params) {
                EmitI("self->%s = %s;\n".printf(p.Name, p.Name));
            }
            EmitI("return self;\n");
            _indent--;
            Emit("}\n\n");
        }


        // ═══════════════════════════════════════════════
        //  Champs et Propriétés
        // ═══════════════════════════════════════════════

        public override void VisitFieldDecl(FieldDeclNode n) {
            EmitI("%s %s".printf(
                TypeToC(n.FieldType), n.Name));
            if (n.Initial != null) {
                Emit(" = ");
                n.Initial.Accept(this);
            }
            Emit(";\n");
        }


        // ═══════════════════════════════════════════════
        //  Instructions
        // ═══════════════════════════════════════════════

        public override void VisitBlock(BlockNode n) {
            Emit("{\n");
            _indent++;
            foreach (var stmt in n.Statements) {
                EmitLine(stmt.Line);
                EmitI("");
                stmt.Accept(this);
                // Ajouter ; si expression
                if (stmt is BinaryExprNode   ||
                    stmt is CallExprNode      ||
                    stmt is AssignExprNode    ||
                    stmt is MemberAccessNode  ||
                    stmt is IdentifierNode    ||
                    stmt is AwaitExprNode) {
                    Emit(";");
                }
                Emit("\n");
            }
            _indent--;
            EmitI("}");
        }

        public override void VisitVarDecl(VarDeclNode n) {
            string cType = "void*";
            if (n.VarType != null) {
                cType = TypeToC(n.VarType);
            } else if (n.Initial != null) {
                cType = InferCType(n.Initial);
            }

            Emit("%s %s".printf(cType, n.Name));
            if (n.Initial != null) {
                Emit(" = ");
                n.Initial.Accept(this);
            }
            Emit(";");
        }

        public override void VisitIf(IfNode n) {
            Emit("if (");
            n.Condition.Accept(this);
            Emit(") ");
            n.ThenBlock.Accept(this);

            foreach (var elseif in n.ElseIfs) {
                Emit(" else if (");
                elseif.Condition.Accept(this);
                Emit(") ");
                elseif.Block.Accept(this);
            }

            if (n.ElseBlock != null) {
                Emit(" else ");
                n.ElseBlock.Accept(this);
            }
        }

        public override void VisitMatch(MatchNode n) {
            // Match → switch/if-else chain en C
            Emit("/* match */\n");
            EmitI("{\n");
            _indent++;
            EmitI("int _match_idx = 0;\n");

            bool first = true;
            foreach (var arm in n.Arms) {
                if (arm.Pattern.Kind ==
                    MatchPatternKind.WILDCARD) {
                    // _ → else final
                    if (!first) EmitI("else ");
                    Emit("{\n");
                } else {
                    if (first) EmitI("if (");
                    else EmitI("else if (");

                    EmitMatchCondition(n.Subject, arm.Pattern);
                    Emit(") {\n");
                }

                _indent++;
                EmitI("");
                arm.Body.Accept(this);
                if (!(arm.Body is BlockNode)) Emit(";");
                Emit("\n");
                _indent--;
                EmitI("}\n");
                first = false;
            }

            _indent--;
            EmitI("}");
        }

        private void EmitMatchCondition(AstNode subject,
                                         MatchPatternNode p) {
            switch (p.Kind) {
                case MatchPatternKind.LITERAL:
                    if (p.Value is LiteralNode) {
                        var lit = (LiteralNode) p.Value;
                        if (lit.Kind == LiteralKind.STRING) {
                            Emit("code_string_equals(");
                            subject.Accept(this);
                            Emit(", \"%s\")".printf(lit.Raw));
                        } else {
                            subject.Accept(this);
                            Emit(" == ");
                            p.Value.Accept(this);
                        }
                    }
                    break;

                case MatchPatternKind.RANGE:
                    subject.Accept(this);
                    Emit(" >= ");
                    p.Value.Accept(this);
                    Emit(" && ");
                    subject.Accept(this);
                    Emit(" <= ");
                    p.RangeEnd.Accept(this);
                    break;

                default:
                    subject.Accept(this);
                    Emit(" == ");
                    if (p.BindName != null) {
                        Emit(p.BindName);
                    }
                    break;
            }
        }

        public override void VisitWhile(WhileNode n) {
            Emit("while (");
            n.Condition.Accept(this);
            Emit(") ");
            n.Body.Accept(this);
        }

        public override void VisitFor(ForNode n) {
            Emit("for (");
            // Init sans le ; final (déjà dans VarDecl)
            string cType = "void*";
            if (n.Init.VarType != null) {
                cType = TypeToC(n.Init.VarType);
            }
            Emit("%s %s = ".printf(cType, n.Init.Name));
            if (n.Init.Initial != null) {
                n.Init.Initial.Accept(this);
            }
            Emit("; ");
            n.Condition.Accept(this);
            Emit("; ");
            n.Step.Accept(this);
            Emit(") ");
            n.Body.Accept(this);
        }

        public override void VisitForeach(ForeachNode n) {
            // foreach → for avec index en C
            Emit("/* foreach %s in ... */\n".printf(n.VarName));
            EmitI("{\n");
            _indent++;
            EmitI("CodeList* _list = (CodeList*)(");
            n.Collection.Accept(this);
            Emit(");\n");
            EmitI("for (int _i = 0; _i < _list->size; _i++) {\n");
            _indent++;
            EmitI("void* %s = CodeList_get(_list, _i);\n"
                  .printf(n.VarName));
            foreach (var stmt in n.Body.Statements) {
                EmitI("");
                stmt.Accept(this);
                Emit(";\n");
            }
            _indent--;
            EmitI("}\n");
            _indent--;
            EmitI("}");
        }

        public override void VisitReturn(ReturnNode n) {
            Emit("return");
            if (n.Value != null) {
                Emit(" ");
                n.Value.Accept(this);
            }
            Emit(";");
        }

        public override void VisitGuard(GuardNode n) {
            Emit("if (!(");
            n.Condition.Accept(this);
            Emit(")) ");
            n.ElseBlock.Accept(this);
        }

        public override void VisitBreak(BreakNode n) {
            Emit("break;");
        }

        public override void VisitContinue(ContinueNode n) {
            Emit("continue;");
        }

        public override void VisitTryCatch(TryCatchNode n) {
            // Pas d'exceptions en C → simuler avec setjmp
            // Pour l'instant : juste le bloc try
            Emit("/* try */ ");
            n.TryBlock.Accept(this);
            Emit(" /* catch(%s %s) skipped for now */"
                 .printf(n.ErrorType, n.ErrorName));
        }

        public override void VisitGoStmt(GoStmtNode n) {
            // Goroutines → threads POSIX (simplifié)
            Emit("/* go */ ");
            n.Expression.Accept(this);
            Emit(";");
        }


        // ═══════════════════════════════════════════════
        //  Expressions
        // ═══════════════════════════════════════════════

        public override void VisitBinaryExpr(BinaryExprNode n) {
            // Pipeline |> : right(left)
            if (n.Operator == "|>") {
                n.Right.Accept(this);
                Emit("(");
                n.Left.Accept(this);
                Emit(")");
                return;
            }

            n.Left.Accept(this);
            Emit(" %s ".printf(OperatorToC(n.Operator)));
            n.Right.Accept(this);
        }

        public override void VisitUnaryExpr(UnaryExprNode n) {
            if (n.IsPrefix) {
                Emit(n.Operator);
                n.Operand.Accept(this);
            } else {
                n.Operand.Accept(this);
                Emit(n.Operator);
            }
        }

        public override void VisitMemberAccess(
            MemberAccessNode n) {

            n.Target.Accept(this);

            // Accès struct C : -> si pointeur, . si valeur
            Emit("->");
            Emit(n.MemberName);
        }

        public override void VisitCallExpr(CallExprNode n) {
            // Cas spéciaux : Console.WriteLine, etc.
            if (n.Callee is MemberAccessNode) {
                var ma = (MemberAccessNode) n.Callee;

                // Console.WriteLine("...")
                if (ma.Target is IdentifierNode) {
                    var id = (IdentifierNode) ma.Target;

                    if (id.Name == "Console") {
                        if (ma.MemberName == "WriteLine") {
                            Emit("Console_WriteLine(");
                            EmitStringArg(n);
                            Emit(")");
                            return;
                        }
                        if (ma.MemberName == "Write") {
                            Emit("Console_Write(");
                            EmitStringArg(n);
                            Emit(")");
                            return;
                        }
                        if (ma.MemberName == "ReadLine") {
                            Emit("Console_ReadLine()");
                            return;
                        }
                    }

                    if (id.Name == "Log") {
                        Emit("Console_WriteLine(");
                        EmitStringArg(n);
                        Emit(")");
                        return;
                    }
                }

                // Appel méthode : obj.Method(args)
                // → ClassName_Method(obj, args)
                EmitMethodCall(ma, n.Arguments, n.NamedArgs);
                return;
            }

            // Appel fonction simple
            n.Callee.Accept(this);
            Emit("(");
            for (int i = 0; i < n.Arguments.size; i++) {
                if (i > 0) Emit(", ");
                n.Arguments[i].Accept(this);
            }
            Emit(")");
        }

        private void EmitStringArg(CallExprNode n) {
            if (n.Arguments.size > 0) {
                var arg = n.Arguments[0];
                if (arg is LiteralNode) {
                    var lit = (LiteralNode) arg;
                    // Transformer l'interpolation
                    // "Hello {name} !" → code_string_format(...)
                    EmitInterpolatedString(lit.Raw);
                } else {
                    arg.Accept(this);
                }
            } else {
                Emit("\"\"");
            }
        }

        private void EmitMethodCall(
            MemberAccessNode              ma,
            Gee.ArrayList<AstNode>        args,
            Gee.HashMap<string, AstNode>  namedArgs) {

            // Déterminer le nom de la fonction C
            // obj.Method(args) → TypeName_Method(obj, args)
            string funcName = "";

            if (ma.Target is IdentifierNode) {
                var id = (IdentifierNode) ma.Target;
                funcName = "%s_%s".printf(id.Name, ma.MemberName);
            } else if (ma.Target is ThisNode) {
                funcName = "%s_%s".printf(
                    _className, ma.MemberName);
            } else {
                // Expression complexe
                ma.Target.Accept(this);
                Emit("->%s".printf(ma.MemberName));
                Emit("(");
                ma.Target.Accept(this);
                for (int i = 0; i < args.size; i++) {
                    Emit(", ");
                    args[i].Accept(this);
                }
                Emit(")");
                return;
            }

            Emit("%s(".printf(funcName));

            // this → self comme premier argument
            if (ma.Target is ThisNode) {
                Emit("self");
            } else {
                ma.Target.Accept(this);
            }

            for (int i = 0; i < args.size; i++) {
                Emit(", ");
                args[i].Accept(this);
            }

            // Arguments nommés
            foreach (var kv in namedArgs.entries) {
                Emit(", ");
                kv.value.Accept(this);
            }

            Emit(")");
        }

        public override void VisitIndexExpr(IndexExprNode n) {
            Emit("CodeList_get((CodeList*)(");
            n.Target.Accept(this);
            Emit("), ");
            n.Index.Accept(this);
            Emit(")");
        }

        public override void VisitAssignExpr(AssignExprNode n) {
            n.Target.Accept(this);
            Emit(" %s ".printf(n.Operator));
            n.Value.Accept(this);
        }

        public override void VisitNewExpr(NewExprNode n) {
            string typeName = TypeName(n.ObjectType);

            // new List<T>() → CodeList_new()
            if (typeName == "List") {
                Emit("CodeList_new()");
                return;
            }

            // new MyClass(args) → MyClass_new(args)
            // ou MyClass_create(args)
            if (n.Arguments.size == 0 &&
                n.NamedArgs.size == 0) {
                Emit("%s_new()".printf(typeName));
            } else {
                Emit("%s_create(".printf(typeName));
                for (int i = 0; i < n.Arguments.size; i++) {
                    if (i > 0) Emit(", ");
                    n.Arguments[i].Accept(this);
                }
                foreach (var kv in n.NamedArgs.entries) {
                    Emit(", ");
                    kv.value.Accept(this);
                }
                Emit(")");
            }
        }

        public override void VisitLambdaExpr(LambdaExprNode n) {
            // Lambdas → pointeurs de fonction en C
            // Simplifié pour l'instant
            Emit("/* lambda */");
        }

        public override void VisitAwaitExpr(AwaitExprNode n) {
            // Await → appel direct (async simplifié)
            n.Expression.Accept(this);
        }

        public override void VisitWithExpr(WithExprNode n) {
            // with { X = 5 } → copie + modification
            Emit("/* with */ ");
            n.Source.Accept(this);
        }

        public override void VisitListLiteral(
            ListLiteralNode n) {

            if (n.IsComprehension) {
                Emit("CodeList_new() /* comprehension */");
                return;
            }

            // [1, 2, 3] → CodeList inline
            Emit("({\n");
            _indent++;
            EmitI("CodeList* _tmp = CodeList_new();\n");
            foreach (var elem in n.Elements) {
                EmitI("CodeList_add(_tmp, (void*)(intptr_t)(");
                elem.Accept(this);
                Emit("));\n");
            }
            EmitI("_tmp;\n");
            _indent--;
            EmitI("})");
        }

        public override void VisitIdentifier(IdentifierNode n) {
            // Traduire certains identifiants CODE → C
            switch (n.Name) {
                case "true":  Emit("true");  break;
                case "false": Emit("false"); break;
                case "null":  Emit("NULL");  break;
                default:      Emit(n.Name);  break;
            }
        }

        public override void VisitThis(ThisNode n) {
            Emit("self");
        }

        public override void VisitNull(NullNode n) {
            Emit("NULL");
        }

        public override void VisitLiteral(LiteralNode n) {
            switch (n.Kind) {
                case LiteralKind.INTEGER:
                    Emit(n.Raw);
                    break;

                case LiteralKind.FLOAT:
                    Emit(n.Raw);
                    break;

                case LiteralKind.BOOL:
                    Emit(n.BoolValue ? "true" : "false");
                    break;

                case LiteralKind.STRING:
                    EmitInterpolatedString(n.Raw);
                    break;

                default:
                    Emit("\"%s\"".printf(n.Raw));
                    break;
            }
        }


        // ═══════════════════════════════════════════════
        //  Types
        // ═══════════════════════════════════════════════

        public override void VisitSimpleType(SimpleTypeNode n) {
            Emit(TypeNameToC(n.Name, n.IsNullable));
        }

        public override void VisitGenericType(
            GenericTypeNode n) {
            Emit(TypeNameToC(n.Name, n.IsNullable));
        }


        // ═══════════════════════════════════════════════
        //  Helpers
        // ═══════════════════════════════════════════════

        /**
         * Convertit un TypeNode en type C.
         */
        private string TypeToC(TypeNode? t) {
            if (t == null) return "void";
            if (t is SimpleTypeNode) {
                var s = (SimpleTypeNode) t;
                return TypeNameToC(s.Name, s.IsNullable);
            }
            if (t is GenericTypeNode) {
                var g = (GenericTypeNode) t;
                return TypeNameToC(g.Name, g.IsNullable);
            }
            return "void*";
        }

        private string TypeNameToC(string name,
                                    bool nullable) {
            switch (name) {
                case "int":    return "i64";
                case "float":  return "f32";
                case "double": return "f64";
                case "string": return "code_string";
                case "bool":   return "code_bool";
                case "void":   return "void";
                case "byte":   return "u8";
                case "char":   return "char";
                case "List":      return "CodeList*";
                case "var":       return "void*";
                case "string[]":  return "code_string*";
                case "int[]":     return "i64*";
                case "float[]":   return "f64*";
                case "bool[]":    return "code_bool*";
                default:
                    // Tableau generique
                    if (name.has_suffix("[]")) {
                        string baseName = name[0:name.length-2];
                        return TypeNameToC(baseName, false) + "*";
                    }
                    // Type utilisateur → pointeur struct
                    return "%s*".printf(name);
            }
        }

        private string TypeName(TypeNode? t) {
            if (t == null) return "void";
            if (t is SimpleTypeNode) {
                return ((SimpleTypeNode) t).Name;
            }
            if (t is GenericTypeNode) {
                return ((GenericTypeNode) t).Name;
            }
            return "unknown";
        }

        /**
         * Convertit un opérateur CODE en opérateur C.
         */
        private string OperatorToC(string op) {
            switch (op) {
                case "&&":  return "&&";
                case "||":  return "||";
                case "==":  return "==";
                case "!=":  return "!=";
                case "<=":  return "<=";
                case ">=":  return ">=";
                case "<":   return "<";
                case ">":   return ">";
                case "+":   return "+";
                case "-":   return "-";
                case "*":   return "*";
                case "/":   return "/";
                case "%":   return "%";
                case "^":   return "/* ^ not direct in C */";
                case "..":  return "/* range */";
                case "??":  return "/* ?? */";
                default:    return op;
            }
        }

        /**
         * Infère le type C d'une expression.
         */
        private string InferCType(AstNode expr) {
            if (expr is LiteralNode) {
                var lit = (LiteralNode) expr;
                switch (lit.Kind) {
                    case LiteralKind.INTEGER: return "i64";
                    case LiteralKind.FLOAT:   return "f64";
                    case LiteralKind.STRING:  return "code_string";
                    case LiteralKind.BOOL:    return "code_bool";
                    default: return "void*";
                }
            }
            if (expr is NewExprNode) {
                var n = (NewExprNode) expr;
                return TypeToC(n.ObjectType);
            }
            return "void*";
        }

        /**
         * Gère l'interpolation de strings.
         * "Hello {name} !" → code_string_format("Hello %s !", name)
         */
        private void EmitInterpolatedString(string raw) {
            // Chercher des {expr} dans la string
            if (!raw.contains("{")) {
                Emit("\"%s\"".printf(
                    raw.replace("\\", "\\\\")
                       .replace("\"", "\\\"")));
                return;
            }

            // Parser les segments
            var fmt  = new StringBuilder();
            var args = new Gee.ArrayList<string>();
            int i    = 0;

            while (i < raw.length) {
                char c = raw[i];

                if (c == '{' && i + 1 < raw.length
                    && raw[i+1] != '{') {
                    // Début d'interpolation
                    int start = i + 1;
                    int end   = raw.index_of("}", start);
                    if (end < 0) {
                        fmt.append_c(c);
                        i++;
                        continue;
                    }

                    string expr = raw[start:end];
                    fmt.append("%s");
                    args.add(expr);
                    i = end + 1;
                } else {
                    if (c == '"')       fmt.append("\\\"");
                    else if (c == '\\') fmt.append("\\\\");
                    else                fmt.append_c(c);
                    i++;
                }
            }

            if (args.size == 0) {
                Emit("\"%s\"".printf(fmt.str));
            } else {
                Emit("code_string_format(\"%s\"".printf(fmt.str));
                foreach (var arg in args) {
                    Emit(", %s".printf(arg));
                }
                Emit(")");
            }
        }

        /**
         * Émet une directive #line pour le debug.
         */
        private void EmitLine(int line) {
            if (line > 0 && line != _sourceLine) {
                Emit("\n#line %d \"%s\"\n"
                     .printf(line, _sourceFile));
                _sourceLine = line;
            }
        }

        /**
         * Émet du texte dans le buffer de sortie.
         */
        private void Emit(string text) {
            _out.append(text);
        }

        /**
         * Émet du texte indenté.
         */
        private void EmitI(string text) {
            for (int i = 0; i < _indent; i++) {
                _out.append("    ");
            }
            _out.append(text);
        }
    }
}
