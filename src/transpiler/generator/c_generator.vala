// ─────────────────────────────────────────────────────
//  CODE Programming Language
//  Copyright (c) 2026 Bastien MOUGET
//  Licensed under Apache 2.0
//  https://github.com/BastienMOUGET/Amalgame
// ─────────────────────────────────────────────────────

namespace CodeTranspiler.Generator {

    using CodeTranspiler.Ast;
    using CodeTranspiler.Lexer;


    // ═══════════════════════════════════════════════════
    //  Résultat de la génération
    // ═══════════════════════════════════════════════════

    public class GeneratorResult : Object {
        public bool   Success   { get; set; }
        public string CCode     { get; set; }
        public string Errors    { get; set; }
        public bool   IsLibrary { get; set; default = false; }

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
        private string        _sourceFile;  // fichier source
        private int           _sourceLine;  // ligne courante source
        private bool          _inClass;     // dans une classe ?
        private string        _className;   // classe courante
        private bool          _hasErrors;
        private StringBuilder _errors;
        // Namespace prefix: "MyApp" → all symbols become "MyApp_ClassName"
        private string        _nsPrefix;
        // Library mode: no main() emitted
        private bool          _isLibrary;
        // Symbol table from the Resolver/TypeChecker
        private CodeTranspiler.Analyzer.SymbolTable? _symbolTable;
        // Local C type map: varName → cType, built during generation
        private Gee.HashMap<string, string> _localCTypes;
        // Program AST reference for method return type lookup
        private ProgramNode? _program;
        // Lambda support: counter + preamble buffer
        private int           _lambdaCounter;
        private StringBuilder _lambdaPreamble;
        private bool          _emitToLambda;


        public CGenerator(string sourceFile,
                          CodeTranspiler.Analyzer.SymbolTable? symbolTable = null,
                          bool forceLib = false) {
            _sourceFile     = sourceFile;
            _symbolTable    = symbolTable;
            _out            = new StringBuilder();
            _errors         = new StringBuilder();
            _lambdaPreamble = new StringBuilder();
            _localCTypes    = new Gee.HashMap<string, string>();
            _indent         = 0;
            _inClass        = false;
            _className      = "";
            _hasErrors      = false;
            _program        = null;
            _sourceLine     = 0;
            _lambdaCounter  = 0;
            _emitToLambda   = false;
            _nsPrefix       = "";
            _isLibrary      = forceLib;
        }


        // ═══════════════════════════════════════════════
        //  Point d'entrée
        // ═══════════════════════════════════════════════

        public GeneratorResult Generate(ProgramNode ast) {
            var result = new GeneratorResult();
            _program   = ast;

            // ── Extract namespace prefix ───────────────
            // "MyApp"         → prefix = "MyApp"
            // "MyApp.Models"  → prefix = "MyApp_Models"
            // ""              → no prefix
            if (ast.Namespace != null) {
                _nsPrefix = ast.Namespace.Name.replace(".", "_");
            }

            // ── Detect library mode ────────────────────
            // A file is a library if:
            //   1. --lib flag was passed (forceLib=true), OR
            //   2. No class named "Program" with a static "Main" method
            if (!_isLibrary) {
                _isLibrary = !_HasEntryPoint(ast);
            }

            // ── Emit ──────────────────────────────────
            EmitHeader();
            ast.Accept(this);

            // Lambda functions collected during generation
            if (_lambdaPreamble.len > 0) {
                Emit("\n/* ── Lambda functions ── */\n");
                Emit(_lambdaPreamble.str);
            }

            // Entry point only for executables
            if (!_isLibrary) {
                EmitFooter(ast);
            } else {
                Emit("\n/* Library — no entry point */\n");
            }

            result.CCode      = _out.str;
            result.Success    = !_hasErrors;
            result.Errors     = _errors.str;
            result.IsLibrary  = _isLibrary;

            return result;
        }

        /**
         * Returns true if the program has a class "Program"
         * with a static method "Main".
         */
        private bool _HasEntryPoint(ProgramNode ast) {
            foreach (var decl in ast.Declarations) {
                if (!(decl is ClassDeclNode)) continue;
                var cls = (ClassDeclNode) decl;
                if (cls.Name != "Program") continue;
                foreach (var m in cls.Members) {
                    if (m is MethodDeclNode) {
                        var md = (MethodDeclNode) m;
                        if (md.Name == "Main" && md.IsStatic)
                            return true;
                    }
                }
            }
            return false;
        }


        // ═══════════════════════════════════════════════
        //  En-tête et pied de page
        // ═══════════════════════════════════════════════

        private void EmitHeader() {
            string mode = _isLibrary ? "Library" : "Executable";
            Emit("/* ═══════════════════════════════════════\n");
            Emit(" * Generated by Amalgame Transpiler v0.3.0\n");
            Emit(" * Source    : %s\n".printf(_sourceFile));
            Emit(" * Mode      : %s\n".printf(mode));
            if (_nsPrefix != "")
                Emit(" * Namespace : %s\n".printf(_nsPrefix.replace("_", ".")));
            Emit(" * DO NOT EDIT MANUALLY\n");
            Emit(" * ═══════════════════════════════════════\n");
            Emit(" */\n\n");
            Emit("#include \"_runtime.h\"\n\n");
        }

        private void EmitFooter(ProgramNode ast) {
            // Find the actual entry class name (may be namespaced)
            string entryClass  = _SymName("Program");
            string entryMethod = "%s_Main".printf(entryClass);

            Emit("\n/* ── Entry point ── */\n");
            Emit("int main(int argc, char** argv) {\n");
            Emit("    code_runtime_init();\n");
            Emit("    %s(argc, argv);\n".printf(entryMethod));
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
            // Namespace is handled via _nsPrefix for symbol naming.
            // Emit a comment for readability.
            Emit("/* namespace %s */\n".printf(n.Name));
        }

        /**
         * Returns the full C symbol name for a type/class.
         * With namespace "MyApp":
         *   "Player" → "MyApp_Player"
         *   "Program" → "MyApp_Program"
         * Without namespace:
         *   "Player" → "Player"
         */
        private string _SymName(string name) {
            if (_nsPrefix == "") return name;
            return "%s_%s".printf(_nsPrefix, name);
        }

        /**
         * Returns the full C function name for a class method.
         *   class="Player", method="TakeDamage" → "MyApp_Player_TakeDamage"
         */
        private string _MethodName(string className, string methodName) {
            return "%s_%s".printf(_SymName(className), methodName);
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
                    var c   = (ClassDeclNode) decl;
                    string sym = _SymName(c.Name);
                    Emit("typedef struct _%s %s;\n".printf(sym, sym));
                    // Forward decl for static methods
                    foreach (var m in c.Members) {
                        if (m is MethodDeclNode) {
                            var md = (MethodDeclNode) m;
                            if (md.IsStatic && md.Name != c.Name) {
                                string ret = md.ReturnType != null
                                    ? TypeToC(md.ReturnType) : "void";
                                Emit("static %s %s();\n"
                                     .printf(ret, _MethodName(c.Name, md.Name)));
                            }
                        }
                    }
                } else if (decl is RecordDeclNode) {
                    var r   = (RecordDeclNode) decl;
                    string sym = _SymName(r.Name);
                    Emit("typedef struct _%s %s;\n".printf(sym, sym));
                } else if (decl is DataClassDeclNode) {
                    var d   = (DataClassDeclNode) decl;
                    string sym = _SymName(d.Name);
                    Emit("typedef struct _%s %s;\n".printf(sym, sym));
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

            string sym = _SymName(n.Name);

            // C struct
            Emit("/* class %s */\n".printf(n.Name));
            Emit("struct _%s {\n".printf(sym));
            _indent++;

            // Inheritance: embed parent struct as first field
            if (n.BaseClass != null) {
                string parentName = _SymName(TypeName(n.BaseClass));
                EmitI("struct _%s _base; /* extends %s */\n"
                      .printf(parentName, parentName));
            }

            // Fields and properties
            foreach (var member in n.Members) {
                if (member is FieldDeclNode) {
                    member.Accept(this);
                } else if (member is PropertyDeclNode) {
                    var prop = (PropertyDeclNode) member;
                    EmitI("%s %s;\n".printf(
                        TypeToC(prop.PropType), prop.Name));
                }
            }

            _indent--;
            Emit("};\n\n");

            // Default constructor — only if no explicit constructor exists
            bool hasExplicitCtor = false;
            foreach (var member in n.Members) {
                if (member is MethodDeclNode) {
                    var md = (MethodDeclNode) member;
                    if (md.Name == n.Name) { hasExplicitCtor = true; break; }
                } else if (member is ConstructorDeclNode) {
                    hasExplicitCtor = true; break;
                }
            }
            if (!hasExplicitCtor)
                EmitDefaultConstructor(n);

            // Methods
            foreach (var member in n.Members) {
                if (member is MethodDeclNode) {
                    EmitMethod(n.Name, (MethodDeclNode) member);
                } else if (member is ConstructorDeclNode) {
                    EmitConstructor(n.Name, (ConstructorDeclNode) member);
                }
            }

            _inClass   = false;
            _className = "";
        }

        private void EmitDefaultConstructor(ClassDeclNode n) {
            string sym = _SymName(n.Name);
            Emit("%s* %s_new() {\n".printf(sym, sym));
            _indent++;
            EmitI("%s* self = (%s*) code_alloc(sizeof(%s));\n"
                  .printf(sym, sym, sym));

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
            string sym = _SymName(className);
            Emit("%s* %s_new(".printf(sym, sym));
            EmitParamList(n.Params);
            Emit(") {\n");
            _indent++;
            EmitI("%s* self = (%s*) code_alloc(sizeof(%s));\n"
                  .printf(sym, sym, sym));
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

            string sym = _SymName(className);

            // A MethodDeclNode whose name matches the class name
            // is actually a constructor (parser limitation).
            if (n.Name == className) {
                Emit("%s* %s_new(".printf(sym, sym));
                EmitParamList(n.Params);
                Emit(") {\n");
                _indent++;
                EmitI("%s* self = (%s*) code_alloc(sizeof(%s));\n"
                      .printf(sym, sym, sym));
                if (n.Body != null) n.Body.Accept(this);
                EmitI("return self;\n");
                _indent--;
                Emit("}\n\n");
                return;
            }

            // Return type
            string retType = n.ReturnType != null
                ? TypeToC(n.ReturnType) : "void";

            // Prototype
            if (n.IsStatic) {
                Emit("static %s %s(".printf(
                    retType, _MethodName(className, n.Name)));
                EmitParamList(n.Params);
                Emit(") ");
            } else {
                Emit("%s %s(%s* self".printf(
                    retType, _MethodName(className, n.Name), sym));
                if (n.Params.size > 0) {
                    Emit(", ");
                    EmitParamList(n.Params);
                }
                Emit(") ");
            }

            // Body
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
                var p     = parms[i];
                var cType = TypeToC(p.ParamType);

                // Register param type for InferCType lookups
                _localCTypes[p.Name] = cType;

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
            string sym = _SymName(n.Name);
            Emit("/* record %s */\n".printf(n.Name));
            Emit("struct _%s {\n".printf(sym));
            _indent++;
            foreach (var p in n.Params)
                EmitI("%s %s;\n".printf(TypeToC(p.ParamType), p.Name));
            _indent--;
            Emit("};\n\n");

            Emit("%s* %s_new(".printf(sym, sym));
            for (int i = 0; i < n.Params.size; i++) {
                if (i > 0) Emit(", ");
                var p = n.Params[i];
                Emit("%s %s".printf(TypeToC(p.ParamType), p.Name));
            }
            Emit(") {\n");
            _indent++;
            EmitI("%s* self = (%s*) code_alloc(sizeof(%s));\n"
                  .printf(sym, sym, sym));
            foreach (var p in n.Params)
                EmitI("self->%s = %s;\n".printf(p.Name, p.Name));
            EmitI("return self;\n");
            _indent--;
            Emit("}\n\n");
        }

        public override void VisitDataClassDecl(DataClassDeclNode n) {
            EmitLine(n.Line);
            string sym = _SymName(n.Name);
            Emit("/* data class %s */\n".printf(n.Name));
            Emit("struct _%s {\n".printf(sym));
            _indent++;
            foreach (var p in n.Params)
                EmitI("%s %s;\n".printf(TypeToC(p.ParamType), p.Name));
            _indent--;
            Emit("};\n\n");

            Emit("%s* %s_new(".printf(sym, sym));
            for (int i = 0; i < n.Params.size; i++) {
                if (i > 0) Emit(", ");
                var p = n.Params[i];
                Emit("%s %s".printf(TypeToC(p.ParamType), p.Name));
            }
            Emit(") {\n");
            _indent++;
            EmitI("%s* self = (%s*) code_alloc(sizeof(%s));\n"
                  .printf(sym, sym, sym));
            foreach (var p in n.Params)
                EmitI("self->%s = %s;\n".printf(p.Name, p.Name));
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

            // Register for interpolation type lookup
            _localCTypes[n.Name] = cType;

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
            string cType = "i64";  // default for loop counter
            if (n.Init.VarType != null) {
                cType = TypeToC(n.Init.VarType);
            } else if (n.Init.Initial != null) {
                cType = InferCType(n.Init.Initial);
            }
            // Register for interpolation
            _localCTypes[n.Init.Name] = cType;
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

            // String concatenation: string + string → code_string_concat()
            if (n.Operator == "+") {
                string lt = InferCType(n.Left);
                string rt = InferCType(n.Right);
                if (lt == "code_string" || rt == "code_string") {
                    Emit("code_string_concat(");
                    // Wrap non-string left side
                    if (lt != "code_string") {
                        Emit("code_int_to_string((i64)(");
                        n.Left.Accept(this);
                        Emit("))");
                    } else {
                        n.Left.Accept(this);
                    }
                    Emit(", ");
                    // Wrap non-string right side
                    if (rt != "code_string") {
                        Emit("code_int_to_string((i64)(");
                        n.Right.Accept(this);
                        Emit("))");
                    } else {
                        n.Right.Accept(this);
                    }
                    Emit(")");
                    return;
                }
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

            // Check if this member belongs to the current class
            // or is inherited from a parent (needs _base. prefix)
            string memberAccess = "->";
            if (_program != null && _inClass) {
                string parentField = _FindInheritedMemberPrefix(
                    _className, n.MemberName);
                if (parentField != "") {
                    Emit("->_base.");
                    Emit(n.MemberName);
                    return;
                }
            }
            Emit(memberAccess);
            Emit(n.MemberName);
        }

        /**
         * Returns "_base" if memberName is declared in a parent class,
         * "" if it belongs to className itself or is unknown.
         */
        private string _FindInheritedMemberPrefix(string className,
                                                   string memberName) {
            if (_program == null) return "";
            string bare = _StripNsPrefix(className);
            foreach (var decl in _program.Declarations) {
                if (!(decl is ClassDeclNode)) continue;
                var cls = (ClassDeclNode) decl;
                if (cls.Name != bare) continue;
                foreach (var m in cls.Members) {
                    if (m is FieldDeclNode &&
                        ((FieldDeclNode) m).Name == memberName) return "";
                    if (m is PropertyDeclNode &&
                        ((PropertyDeclNode) m).Name == memberName) return "";
                    if (m is MethodDeclNode &&
                        ((MethodDeclNode) m).Name == memberName) return "";
                }
                if (cls.BaseClass != null) {
                    string parentName = _StripNsPrefix(TypeName(cls.BaseClass));
                    if (_ClassHasMember(parentName, memberName))
                        return "_base";
                }
                break;
            }
            return "";
        }

        private bool _ClassHasMember(string className, string memberName) {
            if (_program == null) return false;
            string bare = _StripNsPrefix(className);
            foreach (var decl in _program.Declarations) {
                if (!(decl is ClassDeclNode)) continue;
                var cls = (ClassDeclNode) decl;
                if (cls.Name != bare) continue;
                foreach (var m in cls.Members) {
                    if (m is FieldDeclNode &&
                        ((FieldDeclNode) m).Name == memberName) return true;
                    if (m is PropertyDeclNode &&
                        ((PropertyDeclNode) m).Name == memberName) return true;
                    if (m is MethodDeclNode &&
                        ((MethodDeclNode) m).Name == memberName) return true;
                }
                if (cls.BaseClass != null)
                    return _ClassHasMember(
                        _StripNsPrefix(TypeName(cls.BaseClass)), memberName);
            }
            return false;
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

            // Determine the C function name and call style.
            string funcName   = "";
            bool   isStaticCall = false;

            if (ma.Target is ThisNode) {
                funcName = _MethodName(_className, ma.MemberName);

            } else if (ma.Target is IdentifierNode) {
                var id = (IdentifierNode) ma.Target;

                if (id.Name.length > 0 && id.Name[0].isupper()) {
                    // Uppercase → class/type name → static call
                    // Apply namespace prefix to the class name
                    funcName    = _MethodName(id.Name, ma.MemberName);
                    isStaticCall = true;
                } else {
                    // Lowercase → instance variable
                    // Resolve the class name from _localCTypes
                    string className = id.Name;
                    if (_localCTypes.has_key(id.Name)) {
                        string ct = _localCTypes[id.Name];
                        if (ct.has_suffix("*"))
                            className = ct.substring(0, ct.length - 1);
                    }
                    // className may already be prefixed (e.g. "MyApp_Animal")
                    // strip prefix to get bare class name, then re-apply
                    string bareClass = className;
                    if (_nsPrefix != "" && className.has_prefix(_nsPrefix + "_"))
                        bareClass = className.substring(_nsPrefix.length + 1);
                    funcName = _MethodName(bareClass, ma.MemberName);
                }

            } else {
                // Complex expression — emit directly
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

            if (ma.Target is ThisNode) {
                // this.Method(args) → ClassName_Method(self, args)
                Emit("self");
                for (int i = 0; i < args.size; i++) {
                    Emit(", ");
                    args[i].Accept(this);
                }
            } else if (isStaticCall) {
                // ClassName.Method(args) → ClassName_Method(args)
                for (int i = 0; i < args.size; i++) {
                    if (i > 0) Emit(", ");
                    args[i].Accept(this);
                }
                foreach (var kv in namedArgs.entries) {
                    if (args.size > 0) Emit(", ");
                    kv.value.Accept(this);
                }
            } else {
                // obj.Method(args) → ClassName_Method(obj, args)
                ma.Target.Accept(this);
                for (int i = 0; i < args.size; i++) {
                    Emit(", ");
                    args[i].Accept(this);
                }
                foreach (var kv in namedArgs.entries) {
                    Emit(", ");
                    kv.value.Accept(this);
                }
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

            // Apply namespace prefix
            string sym = _SymName(typeName);

            Emit("%s_new(".printf(sym));
            for (int i = 0; i < n.Arguments.size; i++) {
                if (i > 0) Emit(", ");
                n.Arguments[i].Accept(this);
            }
            foreach (var kv in n.NamedArgs.entries) {
                if (n.Arguments.size > 0) Emit(", ");
                kv.value.Accept(this);
            }
            Emit(")");
        }

        public override void VisitLambdaExpr(LambdaExprNode n) {
            // Simple single-expression lambda: p => expr
            // Emitted inline as a GCC nested function (GNU extension)
            // or as a cast to a function pointer stub.
            //
            // Strategy: emit an immediately-called wrapper that
            // captures by value via a helper macro, or for simple
            // cases just emit the expression directly (for use in
            // ForEach-style calls where the body is a statement).
            if (n.Params.size == 0) {
                // () => expr
                if (n.Body != null) n.Body.Accept(this);
                return;
            }

            // p => expr  (single param, expression body)
            // Used inline: emit as a named lambda function reference.
            _lambdaCounter++;
            string lambdaName = "_lambda_%d".printf(_lambdaCounter);

            // Build parameter list
            var paramStr = new StringBuilder();
            foreach (var p in n.Params) {
                if (paramStr.len > 0) paramStr.append(", ");
                paramStr.append("void* %s".printf(p.Name));
            }

            // Register param names as void* for body generation
            foreach (var p in n.Params)
                _localCTypes[p.Name] = "void*";

            // Write lambda function directly into preamble buffer
            // by temporarily redirecting Emit() calls.
            // We use a flag to route Emit() to _lambdaPreamble.
            _emitToLambda = true;

            _lambdaPreamble.append("static void* %s(%s) {\n"
                .printf(lambdaName, paramStr.str));
            _indent++;
            _lambdaPreamble.append("    return (void*)(intptr_t)(");
            if (n.Body != null) n.Body.Accept(this);
            _lambdaPreamble.append(");\n");
            _indent--;
            _lambdaPreamble.append("}\n");

            _emitToLambda = false;

            // Emit reference to the lambda function
            Emit(lambdaName);
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
                case "i8":     return "i8";
                case "i16":    return "i16";
                case "i32":    return "i32";
                case "i64":    return "i64";
                case "u8":     return "u8";
                case "u16":    return "u16";
                case "u32":    return "u32";
                case "u64":    return "u64";
                case "f32":    return "f32";
                case "f64":    return "f64";
                case "List":      return "CodeList*";
                case "var":       return "void*";
                case "string[]":  return "code_string*";
                case "int[]":     return "i64*";
                case "float[]":   return "f32*";
                case "double[]":  return "f64*";
                case "bool[]":    return "code_bool*";
                default:
                    // Generic array
                    if (name.has_suffix("[]")) {
                        string baseName = name[0:name.length-2];
                        return TypeNameToC(baseName, false) + "*";
                    }
                    // User-defined type → apply namespace prefix
                    return "%s*".printf(_SymName(name));
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
                    case LiteralKind.FLOAT:   return "f32";
                    case LiteralKind.STRING:  return "code_string";
                    case LiteralKind.INTERPOLATED_STRING: return "code_string";
                    case LiteralKind.BOOL:    return "code_bool";
                    default: return "void*";
                }
            }
            if (expr is NewExprNode) {
                var n = (NewExprNode) expr;
                return TypeToC(n.ObjectType);
            }
            // Member access: look up the field type in the class
            if (expr is MemberAccessNode) {
                var ma = (MemberAccessNode) expr;
                string objType = InferCType(ma.Target);
                // Strip pointer to get class name: "Animal*" → "Animal"
                string className = objType.has_suffix("*")
                    ? objType.substring(0, objType.length - 1) : objType;
                string ft = _LookupFieldCType(className, ma.MemberName);
                if (ft != "") return ft;
                // Could be a method call result
                return _LookupMethodReturnType(ma.Target, ma.MemberName);
            }
            // Call expression: look up the method return type
            if (expr is CallExprNode) {
                var call = (CallExprNode) expr;
                if (call.Callee is MemberAccessNode) {
                    var ma = (MemberAccessNode) call.Callee;
                    string methodType = _LookupMethodReturnType(
                        ma.Target, ma.MemberName);
                    if (methodType != "void*") return methodType;
                }
            }
            // Binary expression: infer from operands with numeric widening
            if (expr is BinaryExprNode) {
                var bin = (BinaryExprNode) expr;
                string lt = InferCType(bin.Left);
                string rt = InferCType(bin.Right);
                // Comparison → bool
                if (bin.Operator == "==" || bin.Operator == "!=" ||
                    bin.Operator == "<"  || bin.Operator == ">"  ||
                    bin.Operator == "<=" || bin.Operator == ">=")
                    return "code_bool";
                // String concat: use code_string
                if (lt == "code_string" || rt == "code_string")
                    return "code_string";
                // Numeric widening: f64 > f32 > i64
                if (lt == "f64" || rt == "f64") return "f64";
                if (lt == "f32" || rt == "f32") return "f32";
                if (lt == "i64" || rt == "i64") return "i64";
                if (lt != "void*") return lt;
                return rt;
            }
            // Identifier: look up in local C types map
            if (expr is IdentifierNode) {
                var id = (IdentifierNode) expr;
                if (_localCTypes.has_key(id.Name))
                    return _localCTypes[id.Name];
            }
            // This node
            if (expr is ThisNode)
                return "%s*".printf(_className);
            return "void*";
        }

        /**
         * Look up the C return type of a method call.
         * Walks the known class declarations to find the method.
         */
        private string _LookupMethodReturnType(AstNode target,
                                                string memberName) {
            string className = "";
            if (target is IdentifierNode) {
                var id = (IdentifierNode) target;
                className = id.Name;
                if (_localCTypes.has_key(id.Name)) {
                    string ct = _localCTypes[id.Name];
                    if (ct.has_suffix("*"))
                        className = ct.substring(0, ct.length - 1);
                }
            } else if (target is ThisNode) {
                className = _className;
            } else {
                return "void*";
            }

            if (_program == null || className == "") return "void*";
            return _LookupMethodInClass(_StripNsPrefix(className), memberName);
        }

        private string _LookupMethodInClass(string className,
                                             string memberName) {
            if (_program == null) return "void*";
            string bare = _StripNsPrefix(className);
            foreach (var decl in _program.Declarations) {
                if (decl is ClassDeclNode) {
                    var cls = (ClassDeclNode) decl;
                    if (cls.Name != bare) continue;
                    foreach (var m in cls.Members) {
                        if (m is MethodDeclNode) {
                            var md = (MethodDeclNode) m;
                            if (md.Name == memberName && md.ReturnType != null)
                                return TypeToC(md.ReturnType);
                        }
                    }
                    if (cls.BaseClass != null) {
                        string parentName = _StripNsPrefix(TypeName(cls.BaseClass));
                        string parentResult = _LookupMethodInClass(parentName, memberName);
                        if (parentResult != "void*") return parentResult;
                    }
                }
            }
            return "void*";
        }

        /**
         * Strip namespace prefix and pointer suffix from a C type name.
         * "Tests_Animal*" → "Animal"
         * "Tests_Animal"  → "Animal"
         * "Animal*"       → "Animal"
         * "Animal"        → "Animal"
         */
        private string _StripNsPrefix(string name) {
            string s = name;
            if (s.has_suffix("*"))
                s = s.substring(0, s.length - 1);
            if (_nsPrefix != "" && s.has_prefix(_nsPrefix + "_"))
                s = s.substring(_nsPrefix.length + 1);
            return s;
        }

        /**
         * Gère l'interpolation de strings.
         * "Hello {name} !"   → code_string_format("Hello %s !", name)
         * "val: {x}"         → code_string_format("val: %s", code_int_to_string(x))
         * "pt: {p.X}"        → code_string_format("pt: %s", code_float_to_string(p->X))
         */
        private void EmitInterpolatedString(string raw) {
            // No interpolation — emit plain string
            if (!raw.contains("{")) {
                Emit("\"%s\"".printf(
                    raw.replace("\\", "\\\\")
                       .replace("\"", "\\\"")));
                return;
            }

            // Parse segments
            var fmt  = new StringBuilder();
            var args = new Gee.ArrayList<string>();
            int i    = 0;

            while (i < raw.length) {
                char c = raw[i];

                if (c == '{' && i + 1 < raw.length
                    && raw[i+1] != '{') {
                    // Start of interpolation
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
                    // Convert member access: p.X → p->X
                    string cArg = _InterpolArgToC(arg);
                    // Wrap non-string types with conversion helpers
                    string wrapped = _WrapInterpolArg(cArg, arg);
                    Emit(", %s".printf(wrapped));
                }
                Emit(")");
            }
        }

        /**
         * Convert a CODE expression string inside interpolation
         * to its C equivalent.
         *   "p.X"         → "p->X"
         *   "this.Name"   → "self->Name"
         *   "counter"     → "counter"
         */
        private string _InterpolArgToC(string expr) {
            // Handle member access chains: a.b.c → a->b->c
            if (expr.contains(".")) {
                string[] parts = expr.split(".");
                var sb = new StringBuilder();
                for (int j = 0; j < parts.length; j++) {
                    string part = parts[j].strip();
                    if (part == "this") part = "self";
                    if (j == 0) {
                        sb.append(part);
                    } else if (j == 1) {
                        // Check if this member is inherited (needs _base.)
                        string objName = parts[0].strip();
                        string memberName = part;
                        string currentClass = _className;
                        // If accessing via 'this'/'self', use current class
                        if (objName != "this" && objName != "self" &&
                            _localCTypes.has_key(objName)) {
                            string ct = _localCTypes[objName];
                            if (ct.has_suffix("*"))
                                currentClass = ct.substring(0, ct.length - 1);
                        }
                        string prefix = _FindInheritedMemberPrefix(
                            currentClass, memberName);
                        if (prefix != "")
                            sb.append("->_base.%s".printf(memberName));
                        else
                            sb.append("->%s".printf(memberName));
                    } else {
                        sb.append("->%s".printf(part));
                    }
                }
                return sb.str;
            }
            if (expr.strip() == "this") return "self";
            return expr.strip();
        }

        /**
         * Wrap a C interpolation argument with a type conversion
         * helper so it produces a code_string for %s.
         *
         * Heuristic: if the source expression contains a known
         * numeric variable name or looks like an int/float literal,
         * wrap with code_int_to_string / code_float_to_string.
         * Otherwise pass through (assumed already code_string).
         *
         * The TypeChecker's symbol table would give us exact types;
         * for now we use the InferCType heuristic on the raw expr.
         */
        private string _WrapInterpolArg(string cArg, string srcExpr) {
            // 1. Look up in local C type map (most reliable)
            string simpleKey = srcExpr.strip();
            if (_localCTypes.has_key(simpleKey)) {
                string ct = _localCTypes[simpleKey];
                if (ct == "i64" || ct == "i32" || ct == "i16" ||
                    ct == "i8"  || ct == "u64" || ct == "u32" ||
                    ct == "u16" || ct == "u8"  || ct == "byte")
                    return "code_int_to_string(%s)".printf(cArg);
                if (ct == "f64" || ct == "f32" || ct == "double" ||
                    ct == "float")
                    return "code_float_to_string(%s)".printf(cArg);
                if (ct == "code_bool")
                    return "((%s) ? \"true\" : \"false\")".printf(cArg);
                if (ct == "code_string")
                    return cArg;
            }

            // 2. Look up in symbol table (global symbols)
            if (_symbolTable != null) {
                var sym = _symbolTable.Lookup(simpleKey);
                if (sym != null && sym.TypeKey != "" && sym.TypeKey != "?") {
                    string tk = sym.TypeKey;
                    if (tk == "int" || tk == "i64" || tk == "i32" ||
                        tk == "i16" || tk == "i8"  || tk == "byte")
                        return "code_int_to_string(%s)".printf(cArg);
                    if (tk == "float" || tk == "double" ||
                        tk == "f32"   || tk == "f64")
                        return "code_float_to_string(%s)".printf(cArg);
                    if (tk == "bool")
                        return "((%s) ? \"true\" : \"false\")".printf(cArg);
                    if (tk == "string")
                        return cArg;
                }
            }

            // 3. Member access — resolve field type from class declaration
            // e.g. "this.Name" → srcExpr has "." → look up in class members
            if (srcExpr.contains(".")) {
                string[] parts = srcExpr.strip().split(".");
                // parts[0] is the object (this / variable name)
                // parts[1] is the field name
                if (parts.length >= 2) {
                    string objName   = parts[0].strip();
                    string fieldName = parts[parts.length - 1].strip();

                    // Determine the class name
                    string className = "";
                    if (objName == "this" || objName == "self") {
                        className = _className;
                    } else if (_localCTypes.has_key(objName)) {
                        string ct = _localCTypes[objName];
                        if (ct.has_suffix("*"))
                            className = ct.substring(0, ct.length - 1);
                    }

                    // Look up the field type in the class declaration
                    string fieldCType = _LookupFieldCType(className, fieldName);
                    if (fieldCType == "code_string") return cArg;
                    if (fieldCType == "i64" || fieldCType == "i32")
                        return "code_int_to_string(%s)".printf(cArg);
                    if (fieldCType == "f64" || fieldCType == "f32")
                        return "code_float_to_string(%s)".printf(cArg);
                    if (fieldCType == "code_bool")
                        return "((%s) ? \"true\" : \"false\")".printf(cArg);
                    if (fieldCType != "")
                        return cArg; // unknown but non-empty → pass through
                }
                // Fallback for unresolved member access
                return "code_int_to_string((i64)(%s))".printf(cArg);
            }

            // 4. Unknown — pass through (assume code_string)
            return cArg;
        }

        /**
         * Look up the C type of a field in a known class.
         */
        private string _LookupFieldCType(string className, string fieldName) {
            if (_program == null || className == "") return "";
            string bare = _StripNsPrefix(className);
            foreach (var decl in _program.Declarations) {
                if (decl is ClassDeclNode) {
                    var cls = (ClassDeclNode) decl;
                    if (cls.Name != bare) continue;
                    foreach (var m in cls.Members) {
                        if (m is FieldDeclNode) {
                            var f = (FieldDeclNode) m;
                            if (f.Name == fieldName) return TypeToC(f.FieldType);
                        } else if (m is PropertyDeclNode) {
                            var p = (PropertyDeclNode) m;
                            if (p.Name == fieldName) return TypeToC(p.PropType);
                        }
                    }
                    if (cls.BaseClass != null) {
                        string parentName = _StripNsPrefix(TypeName(cls.BaseClass));
                        string parentResult = _LookupFieldCType(parentName, fieldName);
                        if (parentResult != "") return parentResult;
                    }
                }
                else if (decl is DataClassDeclNode) {
                    var dc = (DataClassDeclNode) decl;
                    if (dc.Name != bare) continue;
                    foreach (var rp in dc.Params)
                        if (rp.Name == fieldName) return TypeToC(rp.ParamType);
                    foreach (var m in dc.Members) {
                        if (m is FieldDeclNode) {
                            var f = (FieldDeclNode) m;
                            if (f.Name == fieldName) return TypeToC(f.FieldType);
                        }
                    }
                }
                else if (decl is RecordDeclNode) {
                    var rec = (RecordDeclNode) decl;
                    if (rec.Name != bare) continue;
                    foreach (var rp in rec.Params)
                        if (rp.Name == fieldName) return TypeToC(rp.ParamType);
                }
            }
            return "";
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
            if (_emitToLambda)
                _lambdaPreamble.append(text);
            else
                _out.append(text);
        }

        /**
         * Émet du texte indenté.
         */
        private void EmitI(string text) {
            unowned StringBuilder buf = _emitToLambda ? _lambdaPreamble : _out;
            for (int i = 0; i < _indent; i++)
                buf.append("    ");
            buf.append(text);
        }
    }
}
