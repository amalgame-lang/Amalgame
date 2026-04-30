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
            Emit(" * Generated by Amalgame Transpiler v0.6.0\n");
            Emit(" * Source    : %s\n".printf(_sourceFile));
            Emit(" * Mode      : %s\n".printf(mode));
            if (_nsPrefix != "")
                Emit(" * Namespace : %s\n".printf(_nsPrefix.replace("_", ".")));
            Emit(" * DO NOT EDIT MANUALLY\n");
            Emit(" * ═══════════════════════════════════════\n");
            Emit(" */\n\n");
            Emit("#include \"_runtime.h\"\n\n");
        }

        /**
         * Emit typedef structs for all tuple types used as return values.
         * Called after EmitHeader, before EmitForwardDecls.
         */
        private void EmitTupleStructs(ProgramNode n) {
            var seen = new Gee.HashSet<string>();
            foreach (var decl in n.Declarations) {
                if (!(decl is ClassDeclNode)) continue;
                var cls = (ClassDeclNode) decl;
                foreach (var m in cls.Members) {
                    if (!(m is MethodDeclNode)) continue;
                    var md = (MethodDeclNode) m;
                    if (!(md.ReturnType is TupleTypeNode)) continue;
                    var tt     = (TupleTypeNode) md.ReturnType;
                    string sname = _TupleStructNameFromTypes(tt.ElementTypes);
                    if (seen.contains(sname)) continue;
                    seen.add(sname);
                    // typedef struct { T0 _0; T1 _1; ... } _Tuple_T0_T1;
                    Emit("typedef struct { ");
                    for (int i = 0; i < tt.ElementTypes.size; i++) {
                        Emit("%s _%d; ".printf(TypeToC(tt.ElementTypes[i]), i));
                    }
                    Emit("} %s;\n".printf(sname));
                }
            }
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

            // Tuple struct typedefs (before forward decls that may use them)
            EmitTupleStructs(n);

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

        // ── Collection method dispatch helpers ──────────

        private bool _EmitListMethod(string obj, string method,
                                      Gee.ArrayList<AstNode> args) {
            switch (method) {
                case "Add":      case "add":
                    Emit("AmalgameList_add(%s, (void*)(intptr_t)(".printf(obj));
                    if (args.size > 0) args[0].Accept(this);
                    Emit("))");
                    return true;
                case "Get":      case "get":
                    Emit("AmalgameList_get(%s, ".printf(obj));
                    if (args.size > 0) args[0].Accept(this);
                    Emit(")");
                    return true;
                case "Count":    case "count":   case "Size":   case "size":
                    Emit("AmalgameList_size(%s)".printf(obj));
                    return true;
                case "IsEmpty":  case "isEmpty":
                    Emit("AmalgameList_isEmpty(%s)".printf(obj));
                    return true;
                case "Clear":    case "clear":
                    Emit("AmalgameList_clear(%s)".printf(obj));
                    return true;
                case "Remove":   case "remove":
                    Emit("AmalgameList_remove(%s, (void*)(intptr_t)(".printf(obj));
                    if (args.size > 0) args[0].Accept(this);
                    Emit("))");
                    return true;
                case "RemoveAt": case "removeAt":
                    Emit("AmalgameList_removeAt(%s, ".printf(obj));
                    if (args.size > 0) args[0].Accept(this);
                    Emit(")");
                    return true;
                case "Contains": case "contains":
                    Emit("AmalgameList_contains(%s, (void*)(intptr_t)(".printf(obj));
                    if (args.size > 0) args[0].Accept(this);
                    Emit("))");
                    return true;
                case "First":    case "first":
                    Emit("AmalgameList_first(%s)".printf(obj));
                    return true;
                case "Last":     case "last":
                    Emit("AmalgameList_last(%s)".printf(obj));
                    return true;
                case "Reverse":  case "reverse":
                    Emit("AmalgameList_reverse(%s)".printf(obj));
                    return true;
                case "Copy":     case "copy":
                    Emit("AmalgameList_copy(%s)".printf(obj));
                    return true;
                case "IndexOf":  case "indexOf":
                    Emit("AmalgameList_indexOf(%s, (void*)(intptr_t)(".printf(obj));
                    if (args.size > 0) args[0].Accept(this);
                    Emit("))");
                    return true;
                default:
                    return false;
            }
        }

        private bool _EmitMapMethod(string obj, string method,
                                     Gee.ArrayList<AstNode> args) {
            switch (method) {
                case "Set":    case "set":    case "Put":  case "put":
                case "Add":    case "add":
                    Emit("AmalgameMap_set(%s, ".printf(obj));
                    if (args.size > 0) args[0].Accept(this);
                    Emit(", (void*)(intptr_t)(");
                    if (args.size > 1) args[1].Accept(this);
                    Emit("))");
                    return true;
                case "Get":    case "get":
                    Emit("AmalgameMap_get(%s, ".printf(obj));
                    if (args.size > 0) args[0].Accept(this);
                    Emit(")");
                    return true;
                case "Has":    case "has":
                case "Contains": case "contains":
                case "ContainsKey": case "containsKey":
                    Emit("AmalgameMap_has(%s, ".printf(obj));
                    if (args.size > 0) args[0].Accept(this);
                    Emit(")");
                    return true;
                case "Remove": case "remove":
                    Emit("AmalgameMap_remove(%s, ".printf(obj));
                    if (args.size > 0) args[0].Accept(this);
                    Emit(")");
                    return true;
                case "Size":   case "size":
                case "Count":  case "count":
                    Emit("AmalgameMap_size(%s)".printf(obj));
                    return true;
                case "IsEmpty": case "isEmpty":
                    Emit("AmalgameMap_isEmpty(%s)".printf(obj));
                    return true;
                case "Keys":   case "keys":
                    Emit("AmalgameMap_keys(%s)".printf(obj));
                    return true;
                case "Values": case "values":
                    Emit("AmalgameMap_values(%s)".printf(obj));
                    return true;
                default:
                    return false;
            }
        }

        private bool _EmitSetMethod(string obj, string method,
                                     Gee.ArrayList<AstNode> args) {
            switch (method) {
                case "Add":      case "add":
                    Emit("AmalgameSet_add(%s, ".printf(obj));
                    if (args.size > 0) args[0].Accept(this);
                    Emit(")");
                    return true;
                case "Contains": case "contains":
                    Emit("AmalgameSet_contains(%s, ".printf(obj));
                    if (args.size > 0) args[0].Accept(this);
                    Emit(")");
                    return true;
                case "Remove":   case "remove":
                    Emit("AmalgameSet_remove(%s, ".printf(obj));
                    if (args.size > 0) args[0].Accept(this);
                    Emit(")");
                    return true;
                case "Size":     case "size":
                case "Count":    case "count":
                    Emit("AmalgameSet_size(%s)".printf(obj));
                    return true;
                case "IsEmpty":  case "isEmpty":
                    Emit("AmalgameSet_isEmpty(%s)".printf(obj));
                    return true;
                case "ToList":   case "toList":
                    Emit("AmalgameSet_toList(%s)".printf(obj));
                    return true;
                default:
                    return false;
            }
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
            // Map Amalgame stdlib modules to their header files
            string? header = _ResolveImport(n.Name);
            if (header != null) {
                Emit("#include \"%s\"\n".printf(header));
            } else {
                // Unknown import — emit as comment for now
                Emit("/* import %s */\n".printf(n.Name));
            }
        }

        /**
         * Maps an Amalgame import name to its C header file.
         * Returns null for unknown/user imports.
         */
        private string? _ResolveImport(string name) {
            // Strip alias: "import Amalgame.IO as IO" → "Amalgame.IO"
            string mod = name;
            if (mod.contains(" as "))
                mod = mod.substring(0, mod.index_of(" as ")).strip();

            switch (mod) {
                case "Amalgame.IO":
                case "Amalgame.IO.Console":
                case "Amalgame.IO.File":
                    return "Amalgame_IO.h";

                case "Amalgame.Math":
                    return "Amalgame_Math.h";

                case "Amalgame.String":
                case "Amalgame.Strings":
                    return "Amalgame_String.h";

                case "Amalgame.Net":
                case "Amalgame.Net.Http":
                case "Amalgame.Net.Tcp":
                case "Amalgame.Net.Udp":
                    return "Amalgame_Net.h";

                case "Amalgame.Collections":
                case "Amalgame.Collections.List":
                case "Amalgame.Collections.Map":
                case "Amalgame.Collections.Set":
                    return "Amalgame_Collections.h";

                default:
                    return null;
            }
        }

        /**
         * Génère les forward declarations pour les structs
         * et les méthodes statiques
         */
        private void EmitForwardDecls(ProgramNode n) {
            Emit("/* ── Forward Declarations ── */\n");

            // Pass 1a: emit interface fat-pointer typedefs
            foreach (var decl in n.Declarations) {
                if (!(decl is InterfaceDeclNode)) continue;
                var iface = (InterfaceDeclNode) decl;
                string sym = _SymName(iface.Name);
                // Forward declare the vtable and fat-pointer structs
                Emit("typedef struct _%s_vtable %s_vtable;\n"
                     .printf(sym, sym));
                Emit("typedef struct _%s %s;\n".printf(sym, sym));
            }

            // Pass 1b: emit all enum typedefs
            foreach (var decl in n.Declarations) {
                if (!(decl is EnumDeclNode)) continue;
                var e   = (EnumDeclNode) decl;
                string sym = _SymName(e.Name);

                bool isSimple = true;
                foreach (var m in e.Members)
                    if (m.AssocTypes.size > 0) { isSimple = false; break; }

                if (isSimple) {
                    Emit("typedef enum { ");
                    for (int i = 0; i < e.Members.size; i++) {
                        if (i > 0) Emit(", ");
                        Emit("%s_%s".printf(sym, e.Members[i].Name));
                    }
                    Emit(" } %s;\n".printf(sym));
                } else {
                    // Rich enum — emit tag enum only for forward decl
                    Emit("typedef enum { ");
                    for (int i = 0; i < e.Members.size; i++) {
                        if (i > 0) Emit(", ");
                        Emit("%s_%s_TAG".printf(sym, e.Members[i].Name));
                    }
                    Emit(" } %s_Tag;\n".printf(sym));
                    // Full struct emitted in VisitEnumDecl
                }
            }

            // Pass 2: class/record/data-class typedef + static method fwd decls
            foreach (var decl in n.Declarations) {
                if (decl is ClassDeclNode) {
                    var c   = (ClassDeclNode) decl;
                    string sym = _SymName(c.Name);
                    Emit("typedef struct _%s %s;\n".printf(sym, sym));
                    foreach (var m in c.Members) {
                        if (!(m is MethodDeclNode)) continue;
                        var md = (MethodDeclNode) m;
                        if (!md.IsStatic) continue;
                        if (md.Name == c.Name) continue;
                        string ret = md.ReturnType != null
                            ? TypeToC(md.ReturnType) : "void";
                        string mname = _MethodName(c.Name, md.Name);
                        var sb = new StringBuilder();
                        for (int i = 0; i < md.Params.size; i++) {
                            if (i > 0) sb.append(", ");
                            var p = md.Params[i];
                            string pt = TypeToC(p.ParamType);
                            if (pt == "code_string*" && p.Name == "args")
                                sb.append("int, char**");
                            else
                                sb.append(pt);
                        }
                        string paramStr = sb.len > 0 ? sb.str : "void";
                        Emit("static %s %s(%s);\n"
                             .printf(ret, mname, paramStr));
                    }
                } else if (decl is EnumDeclNode) {
                    // Already emitted in pass 1 — skip
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

            // Interface vtables + as_Interface() converters
            foreach (var iface in n.Interfaces)
                EmitInterfaceImpl(n, iface);

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
        //  Interfaces — vtable dispatch
        // ═══════════════════════════════════════════════

        public override void VisitInterfaceDecl(InterfaceDeclNode n) {
            EmitLine(n.Line);
            string sym = _SymName(n.Name);

            Emit("/* interface %s */\n".printf(n.Name));

            // 1. vtable struct — one function pointer per method
            Emit("typedef struct _%s_vtable {\n".printf(sym));
            _indent++;
            foreach (var member in n.Members) {
                if (!(member is MethodDeclNode)) continue;
                var md = (MethodDeclNode) member;
                string ret = md.ReturnType != null
                    ? TypeToC(md.ReturnType) : "void";
                // Build param list (self + declared params)
                var sb = new StringBuilder("void*");
                foreach (var p in md.Params) {
                    sb.append(", ");
                    sb.append(TypeToC(p.ParamType));
                }
                EmitI("%s (*%s)(%s);\n".printf(ret, md.Name, sb.str));
            }
            _indent--;
            Emit("} %s_vtable;\n\n".printf(sym));

            // 2. interface "fat pointer" — vtable + self
            Emit("typedef struct _%s {\n".printf(sym));
            _indent++;
            EmitI("%s_vtable* vtable;\n".printf(sym));
            EmitI("void*           self;\n");
            _indent--;
            Emit("} %s;\n\n".printf(sym));
        }

        /**
         * Emit vtable initializer and as_Interface() function
         * for a class that implements an interface.
         *
         * class Player implements IDamageable →
         *   static IDamageable_vtable Tests_Player_IDamageable_vtable = { ... };
         *   static IDamageable Tests_Player_as_IDamageable(Tests_Player* self) { ... }
         */
        private void EmitInterfaceImpl(ClassDeclNode cls,
                                        TypeNode      ifaceType) {
            string classSym = _SymName(cls.Name);
            string ifaceName = TypeName(ifaceType);
            string ifaceSym  = _SymName(ifaceName);

            // Find the interface declaration to get its method list
            InterfaceDeclNode? iface = null;
            if (_program != null) {
                foreach (var decl in _program.Declarations) {
                    if (decl is InterfaceDeclNode) {
                        var id = (InterfaceDeclNode) decl;
                        if (id.Name == ifaceName) { iface = id; break; }
                    }
                }
            }
            if (iface == null) return;

            // vtable instance
            Emit("static %s_vtable %s_%s_vtable = {\n"
                 .printf(ifaceSym, classSym, ifaceName));
            _indent++;
            foreach (var member in iface.Members) {
                if (!(member is MethodDeclNode)) continue;
                var md = (MethodDeclNode) member;
                string implFn = _MethodName(cls.Name, md.Name);
                EmitI(".%s = (void*)%s,\n".printf(md.Name, implFn));
            }
            _indent--;
            Emit("};\n\n");

            // as_Interface() converter function
            Emit("static inline %s %s_as_%s(%s* self) {\n"
                 .printf(ifaceSym, classSym, ifaceName, classSym));
            _indent++;
            EmitI("%s _r;\n".printf(ifaceSym));
            EmitI("_r.vtable = &%s_%s_vtable;\n"
                  .printf(classSym, ifaceName));
            EmitI("_r.self   = (void*) self;\n");
            EmitI("return _r;\n");
            _indent--;
            Emit("}\n\n");
        }


        // ═══════════════════════════════════════════════
        //  Enums
        // ═══════════════════════════════════════════════

        public override void VisitEnumDecl(EnumDeclNode n) {
            EmitLine(n.Line);
            string sym = _SymName(n.Name);

            bool isSimple = true;
            foreach (var m in n.Members)
                if (m.AssocTypes.size > 0) { isSimple = false; break; }

            if (isSimple) {
                // Simple enum typedef already emitted in EmitForwardDecls.
                // Just emit a comment for readability.
                Emit("/* enum %s — typedef emitted above */\n\n"
                     .printf(n.Name));
            } else {
                // Rich enum (associated types) → tagged union
                Emit("/* enum %s (tagged union) */\n".printf(n.Name));
                // Tag enum
                Emit("typedef enum {\n");
                _indent++;
                for (int i = 0; i < n.Members.size; i++) {
                    var member = n.Members[i];
                    EmitI("%s_%s_TAG".printf(sym, member.Name));
                    if (i < n.Members.size - 1) Emit(",");
                    Emit("\n");
                }
                _indent--;
                Emit("} %s_Tag;\n\n".printf(sym));

                // Data union
                Emit("typedef struct {\n");
                _indent++;
                EmitI("%s_Tag tag;\n".printf(sym));
                EmitI("union {\n");
                _indent++;
                foreach (var member in n.Members) {
                    if (member.AssocTypes.size == 0) continue;
                    EmitI("struct {\n");
                    _indent++;
                    for (int i = 0; i < member.AssocTypes.size; i++) {
                        EmitI("%s _v%d;\n".printf(
                            TypeToC(member.AssocTypes[i]), i));
                    }
                    _indent--;
                    EmitI("} %s;\n".printf(member.Name.down()));
                }
                _indent--;
                EmitI("} data;\n");
                _indent--;
                Emit("} %s;\n\n".printf(sym));

                // Constructor functions for each variant
                foreach (var member in n.Members) {
                    string ctorName = "%s_%s".printf(sym, member.Name);
                    if (member.AssocTypes.size == 0) {
                        // Constant variant
                        Emit("static inline %s %s() {\n".printf(sym, ctorName));
                        _indent++;
                        EmitI("%s _r; _r.tag = %s_TAG; return _r;\n"
                              .printf(sym, ctorName));
                        _indent--;
                        Emit("}\n");
                    } else {
                        // Variant with data
                        Emit("static inline %s %s(".printf(sym, ctorName));
                        for (int i = 0; i < member.AssocTypes.size; i++) {
                            if (i > 0) Emit(", ");
                            Emit("%s _v%d".printf(
                                TypeToC(member.AssocTypes[i]), i));
                        }
                        Emit(") {\n");
                        _indent++;
                        EmitI("%s _r;\n".printf(sym));
                        EmitI("_r.tag = %s_TAG;\n".printf(ctorName));
                        for (int i = 0; i < member.AssocTypes.size; i++) {
                            EmitI("_r.data.%s._v%d = _v%d;\n"
                                  .printf(member.Name.down(), i, i));
                        }
                        EmitI("return _r;\n");
                        _indent--;
                        Emit("}\n");
                    }
                }
                Emit("\n");
            }

            // Enum methods (if any)
            string savedClass = _className;
            _className = n.Name;
            _inClass   = true;
            foreach (var method in n.Methods)
                EmitMethod(n.Name, method);
            _className = savedClass;
            _inClass   = false;
        }

        public override void VisitEnumMember(EnumMemberNode n) {
            // Members are emitted inside VisitEnumDecl
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
            // If expression: emit as nested ternaries
            // if cond { a } else if cond2 { b } else { c }
            // → ((cond) ? (a) : (cond2) ? (b) : (c))
            if (n.IsExpr) {
                Emit("((");
                n.Condition.Accept(this);
                Emit(") ? (");
                _EmitBlockExpr(n.ThenBlock);
                Emit(")");

                foreach (var ei in n.ElseIfs) {
                    Emit(" : (");
                    ei.Condition.Accept(this);
                    Emit(") ? (");
                    _EmitBlockExpr(ei.Block);
                    Emit(")");
                }

                if (n.ElseBlock != null) {
                    Emit(" : (");
                    _EmitBlockExpr(n.ElseBlock);
                    Emit(")");
                } else {
                    Emit(" : 0");
                }
                Emit(")");
                return;
            }

            // If statement
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

        /**
         * Emit the "value" of a block used as expression.
         * Takes the last statement's expression as the value.
         * { return "big" } → "big"
         * { "big" }        → "big"
         */
        private void _EmitBlockExpr(BlockNode block) {
            if (block.Statements.size == 0) {
                Emit("0");
                return;
            }
            var last = block.Statements[block.Statements.size - 1];
            if (last is ReturnNode) {
                var ret = (ReturnNode) last;
                if (ret.Value != null)
                    ret.Value.Accept(this);
                else
                    Emit("0");
            } else {
                last.Accept(this);
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
                // Don't add ; if body already ends with one (return/break/continue/block)
                if (!(arm.Body is BlockNode) &&
                    !(arm.Body is ReturnNode) &&
                    !(arm.Body is BreakNode)  &&
                    !(arm.Body is ContinueNode))
                    Emit(";");
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

                case MatchPatternKind.ENUM_VARIANT:
                    // Role.Tank => subject == Tests_Role_Tank
                    if (p.BindName != null) {
                        string variant = p.BindName;
                        if (variant.contains(".")) {
                            string[] parts = variant.split(".");
                            string enumName   = parts[0];
                            string memberName = parts[parts.length - 1];
                            // Check if this is a rich enum (tagged union)
                            bool isRich = _IsRichEnum(enumName);
                            if (isRich) {
                                subject.Accept(this);
                                Emit(".tag == %s_%s_TAG".printf(
                                    _SymName(enumName), memberName));
                            } else {
                                // Simple enum: direct comparison
                                subject.Accept(this);
                                Emit(" == %s_%s".printf(
                                    _SymName(enumName), memberName));
                            }
                        } else {
                            subject.Accept(this);
                            Emit(" == %s".printf(_SymName(variant)));
                        }
                    } else if (p.Value != null) {
                        subject.Accept(this);
                        Emit(" == ");
                        p.Value.Accept(this);
                    }
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
            // Detect if collection is a range: 0..10 or start..end
            bool isRange = false;
            if (n.Collection is BinaryExprNode) {
                var bin = (BinaryExprNode) n.Collection;
                if (bin.Operator == "..") isRange = true;
            }

            if (isRange) {
                // for i in 0..10 → for (i64 i = start; i < end; i++)
                var bin = (BinaryExprNode) n.Collection;
                string idx = n.VarName;
                EmitI("for (i64 %s = ".printf(idx));
                bin.Left.Accept(this);
                Emit("; %s < ".printf(idx));
                bin.Right.Accept(this);
                Emit("; %s++) {\n".printf(idx));
                _indent++;
                // Register var type
                _localCTypes[idx] = "i64";
                foreach (var stmt in n.Body.Statements) {
                    EmitI("");
                    stmt.Accept(this);
                    if (!(stmt is ReturnNode || stmt is BreakNode ||
                          stmt is ContinueNode || stmt is BlockNode))
                        Emit(";");
                    Emit("\n");
                }
                _localCTypes.unset(idx);
                _indent--;
                EmitI("}");
                return;
            }

            // List/collection iteration
            // Infer the item type from collection type
            string collType = InferCType(n.Collection);
            string itemType = "void*";
            if (collType == "code_string")
                itemType = "char";  // char-by-char iteration

            // Generate: { AmalgameList* _lst = ...; for (int _i = ...) }
            EmitI("{\n");
            _indent++;

            if (itemType == "char") {
                // String character iteration
                EmitI("code_string _str = (code_string)(");
                n.Collection.Accept(this);
                Emit(");\n");
                EmitI("i64 _slen = (i64) strlen(_str);\n");
                if (n.IndexVar != null) {
                    EmitI("for (i64 %s = 0; %s < _slen; %s++) {\n"
                          .printf(n.IndexVar, n.IndexVar, n.IndexVar));
                } else {
                    EmitI("for (i64 _i = 0; _i < _slen; _i++) {\n");
                }
                _indent++;
                EmitI("char %s = _str[%s];\n"
                      .printf(n.VarName, n.IndexVar ?? "_i"));
                _localCTypes[n.VarName] = "char";
            } else {
                // AmalgameList iteration
                EmitI("AmalgameList* _lst = (AmalgameList*)(");
                n.Collection.Accept(this);
                Emit(");\n");
                if (n.IndexVar != null) {
                    _localCTypes[n.IndexVar] = "i64";
                    EmitI("for (i64 %s = 0; %s < _lst->size; %s++) {\n"
                          .printf(n.IndexVar, n.IndexVar, n.IndexVar));
                } else {
                    EmitI("for (int _i = 0; _i < _lst->size; _i++) {\n");
                }
                _indent++;
                EmitI("void* %s = AmalgameList_get(_lst, %s);\n"
                      .printf(n.VarName, n.IndexVar ?? "_i"));
                _localCTypes[n.VarName] = "void*";
            }

            // Body
            foreach (var stmt in n.Body.Statements) {
                EmitI("");
                stmt.Accept(this);
                if (!(stmt is ReturnNode || stmt is BreakNode ||
                      stmt is ContinueNode || stmt is BlockNode))
                    Emit(";");
                Emit("\n");
            }

            _localCTypes.unset(n.VarName);
            if (n.IndexVar != null) _localCTypes.unset(n.IndexVar);
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
            // try { ... } catch e { ... } finally { ... }
            // Generated C:
            //   { jmp_buf _prev; memcpy(&_prev, &_am_ex.env, sizeof(jmp_buf));
            //     int _caught = setjmp(_am_ex.env);
            //     if (_caught == 0) { /* try */ ... _am_ex.active = 0; }
            //     else { void* e = _am_ex.value; _am_ex.active = 0; /* catch */ }
            //     memcpy(&_am_ex.env, &_prev, sizeof(jmp_buf));
            //     /* finally */ }

            EmitI("{\n");
            _indent++;
            EmitI("jmp_buf _am_prev_env;\n");
            EmitI("memcpy(&_am_prev_env, &_am_ex.env, sizeof(jmp_buf));\n");
            EmitI("int _am_caught = setjmp(_am_ex.env);\n");
            EmitI("if (_am_caught == 0) {\n");
            _indent++;
            // try block
            foreach (var stmt in n.TryBlock.Statements) {
                EmitI("");
                stmt.Accept(this);
                if (!(stmt is ReturnNode || stmt is BreakNode ||
                      stmt is ContinueNode || stmt is BlockNode ||
                      stmt is IfNode || stmt is TryCatchNode))
                    Emit(";");
                Emit("\n");
            }
            EmitI("_am_ex.active = 0;\n");
            _indent--;
            EmitI("} else {\n");
            _indent++;
            // catch block — declare error variable
            EmitI("void* %s = _am_ex.value;\n".printf(n.ErrorName));
            EmitI("_am_ex.active = 0;\n");
            _localCTypes[n.ErrorName] = "void*";
            foreach (var stmt in n.CatchBlock.Statements) {
                EmitI("");
                stmt.Accept(this);
                if (!(stmt is ReturnNode || stmt is BreakNode ||
                      stmt is ContinueNode || stmt is BlockNode ||
                      stmt is IfNode || stmt is TryCatchNode))
                    Emit(";");
                Emit("\n");
            }
            _localCTypes.unset(n.ErrorName);
            _indent--;
            EmitI("}\n");
            // Restore previous exception environment
            EmitI("memcpy(&_am_ex.env, &_am_prev_env, sizeof(jmp_buf));\n");
            // finally block
            if (n.FinallyBlock != null) {
                EmitI("/* finally */\n");
                foreach (var stmt in n.FinallyBlock.Statements) {
                    EmitI("");
                    stmt.Accept(this);
                    if (!(stmt is ReturnNode || stmt is BreakNode ||
                          stmt is ContinueNode || stmt is BlockNode ||
                          stmt is IfNode || stmt is TryCatchNode))
                        Emit(";");
                    Emit("\n");
                }
            }
            _indent--;
            EmitI("}");
        }

        public override void VisitThrow(ThrowNode n) {
            // throw new DivisionError("msg") →
            //   _am_throw(_SymName_DivisionError_new(args), "DivisionError", args[0])
            if (n.Value is NewExprNode) {
                var ne = (NewExprNode) n.Value;
                string typeName = TypeName(ne.ObjectType);
                Emit("_am_throw((void*)(%s_new(".printf(_SymName(typeName)));
                for (int i = 0; i < ne.Arguments.size; i++) {
                    if (i > 0) Emit(", ");
                    ne.Arguments[i].Accept(this);
                }
                Emit(")), \"%s\", ".printf(typeName));
                // message = first string arg if available
                if (ne.Arguments.size > 0)
                    ne.Arguments[0].Accept(this);
                else
                    Emit("\"\"");
                Emit(");");
            } else if (n.Value != null) {
                Emit("_am_throw((void*)(");
                n.Value.Accept(this);
                Emit("), \"Error\", \"\");");
            } else {
                Emit("_am_throw(NULL, \"Error\", \"\");");
            }
        }

        public override void VisitTupleType(TupleTypeNode n) {
            // handled via TypeToC
        }

        public override void VisitTupleExpr(TupleExprNode n) {
            string tname = _TupleStructName(n.Elements);
            Emit("(%s){".printf(tname));
            for (int i = 0; i < n.Elements.size; i++) {
                if (i > 0) Emit(", ");
                n.Elements[i].Accept(this);
            }
            Emit("}");
        }

        public override void VisitTupleDestructure(TupleDestructureNode n) {
            string tmpVar = "_am_tuple_%d".printf(_tupleCounter++);
            string ttype  = _TupleTypeFromExpr(n.Value);
            EmitI("%s %s = ".printf(ttype, tmpVar));
            n.Value.Accept(this);
            Emit(";\n");
            for (int i = 0; i < n.Names.size; i++) {
                string elemType = _TupleElemType(ttype, i);
                EmitI("%s %s = %s._%d;\n"
                      .printf(elemType, n.Names[i], tmpVar, i));
                _localCTypes[n.Names[i]] = elemType;
            }
        }

        private int _tupleCounter = 0;

        private string _TupleStructName(Gee.ArrayList<AstNode> elems) {
            var sb = new StringBuilder("_Tuple");
            foreach (var e in elems) {
                sb.append("_");
                sb.append(_TupleTypeShort(InferCType(e)));
            }
            return sb.str;
        }

        private string _TupleStructNameFromTypes(Gee.ArrayList<TypeNode> types) {
            var sb = new StringBuilder("_Tuple");
            foreach (var t in types) {
                sb.append("_");
                sb.append(_TupleTypeShort(TypeToC(t)));
            }
            return sb.str;
        }

        private string _TupleTypeShort(string ct) {
            switch (ct) {
                case "i64":         return "i64";
                case "f64":         return "f64";
                case "f32":         return "f32";
                case "code_bool":   return "bool";
                case "code_string": return "str";
                default:
                    return _StripNsPrefix(ct.replace("*","").replace(" ",""));
            }
        }

        private string _TupleTypeFromExpr(AstNode expr) {
            if (expr is TupleExprNode)
                return _TupleStructName(((TupleExprNode)expr).Elements);
            if (expr is CallExprNode) {
                var call = (CallExprNode) expr;
                if (call.Callee is MemberAccessNode) {
                    var ma = (MemberAccessNode) call.Callee;
                    if (ma.Target is IdentifierNode) {
                        string t = _LookupTupleReturnType(
                            ((IdentifierNode)ma.Target).Name, ma.MemberName);
                        if (t != "") return t;
                    }
                }
            }
            return "_Tuple_ptr_ptr";
        }

        private string _LookupTupleReturnType(string className,
                                                string methodName) {
            if (_program == null) return "";
            string bare = _StripNsPrefix(className);
            foreach (var decl in _program.Declarations) {
                if (!(decl is ClassDeclNode)) continue;
                var cls = (ClassDeclNode) decl;
                if (cls.Name != bare) continue;
                foreach (var m in cls.Members) {
                    if (!(m is MethodDeclNode)) continue;
                    var md = (MethodDeclNode) m;
                    if (md.Name != methodName) continue;
                    if (md.ReturnType is TupleTypeNode) {
                        var tt = (TupleTypeNode) md.ReturnType;
                        return _TupleStructNameFromTypes(tt.ElementTypes);
                    }
                }
            }
            return "";
        }

        private string _TupleElemType(string tname, int idx) {
            string body = tname.substring("_Tuple".length);
            string[] parts = body.split("_");
            var types = new Gee.ArrayList<string>();
            foreach (var p in parts)
                if (p.length > 0) types.add(p);
            if (idx >= types.size) return "void*";
            switch (types[idx]) {
                case "i64":  return "i64";
                case "f64":  return "f64";
                case "f32":  return "f32";
                case "bool": return "code_bool";
                case "str":  return "code_string";
                default:     return "void*";
            }
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

            // Enum member access: Role.Tank → Tests_Role_Tank
            // Detect when target is an enum type name
            if (n.Target is IdentifierNode) {
                var id = (IdentifierNode) n.Target;
                if (_IsEnumType(id.Name)) {
                    Emit("%s_%s".printf(_SymName(id.Name), n.MemberName));
                    return;
                }
            }

            n.Target.Accept(this);

            // Check if this member is inherited (needs _base. prefix)
            if (_program != null && _inClass) {
                string parentField = _FindInheritedMemberPrefix(
                    _className, n.MemberName);
                if (parentField != "") {
                    Emit("->_base.");
                    Emit(n.MemberName);
                    return;
                }
            }
            Emit("->");
            Emit(n.MemberName);
        }

        /**
         * Returns true if 'name' refers to an enum declared in the program.
         */
        private bool _IsEnumType(string name) {
            if (_program == null) return false;
            foreach (var decl in _program.Declarations)
                if (decl is EnumDeclNode &&
                    ((EnumDeclNode) decl).Name == name)
                    return true;
            return false;
        }

        private bool _IsInterfaceType(string name) {
            if (_program == null) return false;
            string bare = _StripNsPrefix(name);
            foreach (var decl in _program.Declarations)
                if (decl is InterfaceDeclNode &&
                    ((InterfaceDeclNode) decl).Name == bare)
                    return true;
            return false;
        }

        /**
         * Returns the expected C type of the i-th parameter of a
         * static method. Used to detect interface auto-conversion.
         */
        private string _LookupParamType(string className,
                                         string methodName,
                                         int    paramIndex) {
            if (_program == null) return "";
            string bare = _StripNsPrefix(className);
            foreach (var decl in _program.Declarations) {
                if (!(decl is ClassDeclNode)) continue;
                var cls = (ClassDeclNode) decl;
                if (cls.Name != bare) continue;
                foreach (var m in cls.Members) {
                    if (!(m is MethodDeclNode)) continue;
                    var md = (MethodDeclNode) m;
                    if (md.Name != methodName) continue;
                    if (paramIndex < md.Params.size)
                        return TypeName(md.Params[paramIndex].ParamType);
                }
            }
            return "";
        }

        private bool _IsRichEnum(string name) {
            if (_program == null) return false;
            foreach (var decl in _program.Declarations) {
                if (!(decl is EnumDeclNode)) continue;
                var e = (EnumDeclNode) decl;
                if (e.Name != name) continue;
                foreach (var m in e.Members)
                    if (m.AssocTypes.size > 0) return true;
            }
            return false;
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
            // ── Special dispatch ──────────────────────────
            if (n.Callee is MemberAccessNode) {
                var ma = (MemberAccessNode) n.Callee;

                if (ma.Target is IdentifierNode) {
                    var id = (IdentifierNode) ma.Target;

                    // Console.WriteLine / Write / ReadLine
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

                    // ── List<T> method dispatch ────────────
                    // Detect: variable whose C type is AmalgameList*
                    if (_localCTypes.has_key(id.Name) &&
                        _localCTypes[id.Name] == "AmalgameList*") {
                        if (_EmitListMethod(id.Name, ma.MemberName,
                                            n.Arguments)) return;
                    }

                    // ── Map<K,V> method dispatch ───────────
                    if (_localCTypes.has_key(id.Name) &&
                        _localCTypes[id.Name] == "AmalgameMap*") {
                        if (_EmitMapMethod(id.Name, ma.MemberName,
                                           n.Arguments)) return;
                    }

                    // ── Set<T> method dispatch ─────────────
                    if (_localCTypes.has_key(id.Name) &&
                        _localCTypes[id.Name] == "AmalgameSet*") {
                        if (_EmitSetMethod(id.Name, ma.MemberName,
                                           n.Arguments)) return;
                    }
                }

                // Generic method call
                EmitMethodCall(ma, n.Arguments, n.NamedArgs);
                return;
            }

            // Simple function call
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
                        else
                            className = ct; // interface fat pointer (no *)
                    }
                    string bareClass = _StripNsPrefix(className);

                    // Check if this is an interface type — use vtable dispatch
                    if (_IsInterfaceType(bareClass)) {
                        // obj.Method(args) → obj.vtable->Method(obj.self, args)
                        Emit("%s.vtable->%s(%s.self"
                             .printf(id.Name, ma.MemberName, id.Name));
                        for (int i = 0; i < args.size; i++) {
                            Emit(", ");
                            args[i].Accept(this);
                        }
                        Emit(")");
                        return;
                    }

                    funcName = _MethodName(bareClass, ma.MemberName);
                }

            } else if (ma.Target is MemberAccessNode) {
                // Chained: this.Home.Format() → Tests_Address_Format(self->Home)
                var innerMa = (MemberAccessNode) ma.Target;
                // Resolve the type of the inner member
                string innerClassName = "";
                if (innerMa.Target is ThisNode) {
                    // this.Field → look up Field type in current class
                    string ft = _LookupFieldCType(_className, innerMa.MemberName);
                    innerClassName = _StripNsPrefix(ft);
                } else if (innerMa.Target is IdentifierNode) {
                    var innerId = (IdentifierNode) innerMa.Target;
                    if (_localCTypes.has_key(innerId.Name)) {
                        innerClassName = _StripNsPrefix(_localCTypes[innerId.Name]);
                    }
                }
                funcName = _MethodName(innerClassName, ma.MemberName);
                Emit("%s(".printf(funcName));
                // Emit the object expression as self arg
                ma.Target.Accept(this);
                for (int i = 0; i < args.size; i++) {
                    Emit(", ");
                    args[i].Accept(this);
                }
                Emit(")");
                return;

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
                // Auto-convert concrete types to interface fat pointers if needed
                string targetClass = "";
                if (ma.Target is IdentifierNode)
                    targetClass = ((IdentifierNode) ma.Target).Name;

                for (int i = 0; i < args.size; i++) {
                    if (i > 0) Emit(", ");
                    // Check if arg needs interface conversion
                    string argType = _StripNsPrefix(InferCType(args[i]));
                    string expectedType = _LookupParamType(
                        targetClass, ma.MemberName, i);
                    if (expectedType != "" && _IsInterfaceType(expectedType)
                        && !_IsInterfaceType(argType)) {
                        // Auto-wrap: Circle* → Tests_Circle_as_IDescribable(c)
                        Emit("%s_as_%s(".printf(
                            _SymName(argType), expectedType));
                        args[i].Accept(this);
                        Emit(")");
                    } else {
                        args[i].Accept(this);
                    }
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
            Emit("AmalgameList_get((AmalgameList*)(");
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

            // Stdlib collection constructors
            switch (typeName) {
                case "List":
                    Emit("AmalgameList_new()");
                    return;
                case "Map":
                    Emit("AmalgameMap_new()");
                    return;
                case "Set":
                    Emit("AmalgameSet_new()");
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
                Emit("AmalgameList_new() /* comprehension */");
                return;
            }

            // [1, 2, 3] → AmalgameList inline
            Emit("({\n");
            _indent++;
            EmitI("AmalgameList* _tmp = AmalgameList_new();\n");
            foreach (var elem in n.Elements) {
                EmitI("AmalgameList_add(_tmp, (void*)(intptr_t)(");
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
            if (t is TupleTypeNode) {
                var tt = (TupleTypeNode) t;
                return _TupleStructNameFromTypes(tt.ElementTypes);
            }
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
                case "List":      return "AmalgameList*";
                case "Map":       return "AmalgameMap*";
                case "Set":       return "AmalgameSet*";
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
                    // Interface type → fat pointer (value type, no *)
                    if (_IsInterfaceType(name))
                        return _SymName(name);
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
            // If expression: infer type from then-branch last statement
            if (expr is IfNode) {
                var ifn = (IfNode) expr;
                if (ifn.IsExpr && ifn.ThenBlock.Statements.size > 0) {
                    var last = ifn.ThenBlock.Statements[
                        ifn.ThenBlock.Statements.size - 1];
                    if (last is ReturnNode && ((ReturnNode)last).Value != null)
                        return InferCType(((ReturnNode)last).Value);
                    return InferCType(last);
                }
                return "void*";
            }

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

                // Known Net/stdlib struct fields — fast path
                switch (ma.MemberName) {
                    case "Ok": case "Connected": case "Listening":
                    case "Bound":
                        return "code_bool";
                    case "Status": case "RemotePort": case "BoundPort":
                    case "Port":
                        return "i64";
                    case "Body": case "Error": case "RemoteHost":
                    case "RemoteIp":
                        return "code_string";
                }

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
                } else if (call.Callee is IdentifierNode) {
                    // Direct stdlib call: File_WriteAll(...), Math_Sqrt(...), etc.
                    var id = (IdentifierNode) call.Callee;
                    string stdlibType = _StdlibReturnType(id.Name);
                    if (stdlibType != "void*") return stdlibType;
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

            // Try user-defined class first
            string userResult = _LookupMethodInClass(
                _StripNsPrefix(className), memberName);
            if (userResult != "void*") return userResult;

            // Fall back to stdlib return type table
            return _StdlibReturnType(memberName);
        }

        private string _StdlibReturnType(string funcName) {
                // code_bool
            switch (funcName) {
                case "File_Exists": case "File_WriteAll":
                case "File_AppendAll": case "File_Delete":
                case "Environment_HasVar":
                case "String_IsEmpty": case "String_IsWhitespace":
                case "String_Contains": case "String_StartsWith":
                case "String_EndsWith": case "String_ToBool":
                case "String_IsDigit": case "String_IsAlpha":
                case "String_IsAlnum":
                case "Math_IsPrime": case "Math_IsNaN":
                case "Math_IsInf": case "Math_IsFinite":
                case "Math_ApproxEq":
                // Collections C-level names
                case "AmalgameList_isEmpty": case "AmalgameList_contains":
                case "AmalgameList_containsStr": case "AmalgameList_remove":
                case "AmalgameList_any": case "AmalgameList_all":
                case "AmalgameMap_has": case "AmalgameMap_remove":
                case "AmalgameMap_isEmpty":
                case "AmalgameSet_add": case "AmalgameSet_contains":
                case "AmalgameSet_remove": case "AmalgameSet_isEmpty":
                // Net — bool returns
                case "TcpClient_Send": case "TcpClient_SendBytes":
                case "TcpClient_IsConnected": case "TcpClient_Close":
                case "TcpServer_Close": case "TcpServer_IsListening":
                case "TcpConn_Send": case "TcpConn_Close":
                case "TcpConn_IsConnected":
                case "UdpSocket_Bind": case "UdpSocket_Send":
                case "UdpSocket_Close":
                // Collections + Net — Amalgame-level bool method names
                case "IsEmpty":    case "isEmpty":
                case "Contains":   case "contains":
                case "Has":        case "has":
                case "ContainsKey": case "containsKey":
                case "Any":        case "any":
                case "All":        case "all":
                case "Remove":     case "remove":
                case "Send":       case "send":
                case "Bind":       case "bind":
                case "IsConnected": case "isConnected":
                case "IsListening": case "isListening":
                    return "code_bool";

                // i64
                case "File_Size":
                case "String_Length": case "String_IndexOf":
                case "String_LastIndexOf": case "String_ToInt":
                case "Math_MaxI": case "Math_MinI": case "Math_ClampI":
                case "Math_AbsI": case "Math_PowI":
                case "Math_Gcd": case "Math_Lcm":
                case "Math_Sign": case "Math_RandomInt":
                // Collections C-level
                case "AmalgameList_size": case "AmalgameList_count":
                case "AmalgameList_indexOf": case "AmalgameList_countIf":
                case "AmalgameMap_size": case "AmalgameMap_count":
                case "AmalgameSet_size": case "AmalgameSet_count":
                // Collections Amalgame-level
                case "Count": case "count":
                case "Size": case "size":
                case "IndexOf": case "indexOf":
                case "CountIf": case "countIf":
                    return "i64";

                // f64
                case "Math_Abs": case "Math_Sqrt": case "Math_Cbrt":
                case "Math_Pow": case "Math_Exp":
                case "Math_Log": case "Math_Log2": case "Math_Log10":
                case "Math_Floor": case "Math_Ceil":
                case "Math_Round": case "Math_Trunc":
                case "Math_MaxF": case "Math_MinF": case "Math_ClampF":
                case "Math_CopySign":
                case "Math_Sin": case "Math_Cos": case "Math_Tan":
                case "Math_Asin": case "Math_Acos": case "Math_Atan":
                case "Math_Atan2": case "Math_Sinh": case "Math_Cosh":
                case "Math_Tanh": case "Math_ToRadians":
                case "Math_ToDegrees": case "Math_Random":
                case "String_ToFloat":
                    return "f64";

                // code_string
                case "File_ReadAll":
                case "Path_Combine": case "Path_GetExtension":
                case "Path_GetFilename": case "Path_GetDirectory":
                case "Environment_GetVar": case "Environment_GetVarOr":
                case "String_Substring": case "String_From":
                case "String_Until": case "String_ToUpper":
                case "String_ToLower": case "String_TrimStart":
                case "String_TrimEnd": case "String_Trim":
                case "String_Replace": case "String_Join":
                case "String_Repeat": case "String_PadLeft":
                case "String_PadRight": case "String_FromInt":
                case "String_FromFloat": case "String_FromBool":
                // Net — string returns
                case "TcpClient_Receive": case "UdpSocket_Receive":
                    return "code_string";

                // Net — pointer returns (HttpResponse*, TcpClient*, UdpSocket*)
                case "Http_Get": case "Http_GetWithHeaders":
                case "Http_GetTimeout": case "Http_Post":
                case "Http_PostJson": case "Http_PostWithHeaders":
                case "Http_Put": case "Http_Delete": case "Http_Patch":
                    return "AmalgameHttpResponse*";

                case "TcpClient_Connect":
                    return "AmalgameTcpClient*";

                case "TcpServer_Listen":
                    return "AmalgameTcpServer*";

                case "TcpServer_Accept":
                    return "AmalgameTcpConn*";

                case "UdpSocket_New":
                    return "AmalgameUdpSocket*";

                default:
                    return "void*";
            }
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
            // Normalize multiline string:
            // Strip leading newline and trailing whitespace+newline
            string normalized = raw;
            if (raw.contains("\n")) {
                // Strip leading newline if string starts with one
                if (normalized.has_prefix("\n"))
                    normalized = normalized.substring(1);
                // Strip trailing newline + optional whitespace
                if (normalized.has_suffix("\n"))
                    normalized = normalized.substring(0, normalized.length - 1);
                // Find minimum indentation to strip (dedent)
                string[] lines = normalized.split("\n");
                int minIndent = int.MAX;
                foreach (var line in lines) {
                    if (line.strip().length == 0) continue;
                    int spaces = 0;
                    for (int k = 0; k < line.length; k++) {
                        if (line[k] == ' ')  spaces++;
                        else if (line[k] == '\t') spaces += 4;
                        else break;
                    }
                    if (spaces < minIndent) minIndent = spaces;
                }
                if (minIndent == int.MAX) minIndent = 0;
                if (minIndent > 0) {
                    var dedented = new StringBuilder();
                    for (int li = 0; li < lines.length; li++) {
                        string line = lines[li];
                        int stripped = 0;
                        int pos = 0;
                        while (pos < line.length && stripped < minIndent) {
                            if (line[pos] == ' ')  { stripped++;      pos++; }
                            else if (line[pos] == '\t') { stripped += 4; pos++; }
                            else break;
                        }
                        if (li > 0) dedented.append("\n");
                        dedented.append(line.substring(pos));
                    }
                    normalized = dedented.str;
                }
            }

            // No interpolation — emit plain string
            if (!normalized.contains("{")) {
                var escaped = new StringBuilder();
                for (int k = 0; k < normalized.length; k++) {
                    char ch = normalized[k];
                    if      (ch == '"')  escaped.append("\\\"");
                    else if (ch == '\\') escaped.append("\\\\");
                    else if (ch == '\n') escaped.append("\\n");
                    else if (ch == '\t') escaped.append("\\t");
                    else                 escaped.append_c(ch);
                }
                Emit("\"%s\"".printf(escaped.str));
                return;
            }

            // Parse segments with interpolation
            var fmt  = new StringBuilder();
            var args = new Gee.ArrayList<string>();
            int i    = 0;

            while (i < normalized.length) {
                char c = normalized[i];

                if (c == '{' && i + 1 < normalized.length
                    && normalized[i+1] != '{') {
                    int start = i + 1;
                    int end   = normalized.index_of("}", start);
                    if (end < 0) {
                        fmt.append_c(c);
                        i++;
                        continue;
                    }
                    string expr = normalized[start:end];
                    fmt.append("%s");
                    args.add(expr);
                    i = end + 1;
                } else {
                    if      (c == '"')  fmt.append("\\\"");
                    else if (c == '\\') fmt.append("\\\\");
                    else if (c == '\n') fmt.append("\\n");
                    else if (c == '\t') fmt.append("\\t");
                    else                fmt.append_c(c);
                    i++;
                }
            }

            if (args.size == 0) {
                Emit("\"%s\"".printf(fmt.str));
            } else {
                Emit("code_string_format(\"%s\"".printf(fmt.str));
                foreach (var arg in args) {
                    string cArg   = _InterpolArgToC(arg);
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
