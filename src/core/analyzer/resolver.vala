// ─────────────────────────────────────────────────────
//  CODE Programming Language
//  Copyright (c) 2026 Bastien MOUGET
//  Licensed under Apache 2.0
//  https://github.com/NeitsabTeguom/CODE
// ─────────────────────────────────────────────────────

// ═══════════════════════════════════════════════════════
//  resolver.vala  -  Name resolution pass for CODE
//
//  The Resolver runs after the Parser and before the
//  TypeChecker. It performs two sequential passes over
//  the AST:
//
//  Pass 1 — CollectTopLevel
//    Walk only the top-level declarations and register
//    every type name (class, interface, enum, record,
//    data class) into the global scope.  This ensures
//    forward references work: class A can reference
//    class B even when B is declared after A.
//
//  Pass 2 — Resolve
//    Walk the entire AST. Open / close scopes around
//    every declaration and block. For each identifier
//    or type reference, verify that the name is visible
//    in the current scope. Emit a ResolveError when it
//    is not, with a "did you mean?" suggestion when a
//    close match exists.
//
//  Usage:
//    var resolver = new Resolver(filename);
//    var result   = resolver.Resolve(ast);
//    if (!result.Success) {
//        foreach (var err in result.Errors)
//            stderr.printf(err.ToString());
//    }
// ═══════════════════════════════════════════════════════

namespace CodeTranspiler.Analyzer {

    using CodeTranspiler.Ast;
    using CodeTranspiler.Lexer;


    // ═══════════════════════════════════════════════════
    //  Resolve error
    // ═══════════════════════════════════════════════════

    /**
     * A single name-resolution error.
     *
     * Rendered with a box-drawing frame consistent with
     * the ParseError format already used in the project.
     */
    public class ResolveError : Object {

        public string Message  { get; set; }
        public int    Line     { get; set; }
        public int    Column   { get; set; }
        public string Filename { get; set; }

        public ResolveError(string message,
                            string filename,
                            int    line,
                            int    column) {
            Message  = message;
            Filename = filename;
            Line     = line;
            Column   = column;
        }

        /** Build a ResolveError from an AstNode position. */
        public ResolveError.from_node(string message, AstNode node) {
            Message  = message;
            Filename = node.Filename;
            Line     = node.Line;
            Column   = node.Column;
        }

        public string ToString() {
            return "\n┌── [resolver] %s:%d:%d\n│\n│  %s\n│\n└──\n"
                   .printf(Filename, Line, Column, Message);
        }
    }


    // ═══════════════════════════════════════════════════
    //  Resolve result
    // ═══════════════════════════════════════════════════

    /**
     * Returned by Resolver.Resolve().
     *
     * On success, the same ProgramNode is returned with
     * resolver-annotated symbols attached.
     * On failure, Errors contains every problem found
     * (the resolver is error-tolerant and keeps going).
     */
    public class ResolveResult : Object {

        public bool                          Success { get; set; }
        public ProgramNode?                  Program { get; set; }
        public Gee.ArrayList<ResolveError>   Errors  { get; set; }
        public SymbolTable                   Symbols { get; set; }

        public ResolveResult() {
            Errors = new Gee.ArrayList<ResolveError>();
        }
    }


    // ═══════════════════════════════════════════════════
    //  Resolver
    // ═══════════════════════════════════════════════════

    /**
     * Two-pass name resolver.
     *
     * Extends BaseAstVisitor so only the relevant Visit*
     * methods need to be overridden.
     */
    public class Resolver : BaseAstVisitor {

        // ── Internal state ─────────────────────────────

        private string                    _filename;
        private SymbolTable               _table;
        private Gee.ArrayList<ResolveError> _errors;

        /**
         * Name of the class currently being visited.
         * Used to populate the "owner" label in scope names
         * and to resolve implicit `this` references.
         */
        private string? _currentClass;

        /**
         * Return type of the method currently being visited.
         * Stored as a raw string key for now; the TypeChecker
         * will validate it later.
         */
        private string? _currentReturnType;

        /**
         * Whether we are inside a loop body.
         * Enables validation of break / continue placement.
         */
        private int _loopDepth;

        /**
         * Whether we are inside an async method.
         * Enables validation of await expressions.
         */
        private bool _inAsync;


        // ── Constructor ────────────────────────────────

        public Resolver(string filename = "<unknown>") {
            _filename          = filename;
            _table             = new SymbolTable();
            _errors            = new Gee.ArrayList<ResolveError>();
            _currentClass      = null;
            _currentReturnType = null;
            _loopDepth         = 0;
            _inAsync           = false;
        }


        // ═══════════════════════════════════════════════
        //  Public entry point
        // ═══════════════════════════════════════════════

        /**
         * Run both resolution passes on the given program AST.
         */
        public ResolveResult Resolve(ProgramNode program) {
            // Pass 1: collect all top-level type names
            _CollectTopLevel(program);

            // Pass 2: full resolution walk
            program.Accept(this);

            var result     = new ResolveResult();
            result.Program = program;
            result.Symbols = _table;
            result.Errors  = _errors;
            result.Success = (_errors.size == 0);
            return result;
        }


        // ═══════════════════════════════════════════════
        //  Pass 1 — Collect top-level declarations
        // ═══════════════════════════════════════════════

        /**
         * Register every type declared at the top level of the
         * program into the global scope so that forward
         * references resolve correctly in Pass 2.
         */
        private void _CollectTopLevel(ProgramNode program) {
            foreach (var decl in program.Declarations) {

                if (decl is ClassDeclNode) {
                    var n   = (ClassDeclNode) decl;
                    var sym = new Symbol(n.Name, SymbolKind.SYM_CLASS, n);
                    _DeclareGlobalOrError(sym);

                } else if (decl is InterfaceDeclNode) {
                    var n   = (InterfaceDeclNode) decl;
                    var sym = new Symbol(n.Name, SymbolKind.SYM_INTERFACE, n);
                    _DeclareGlobalOrError(sym);

                } else if (decl is EnumDeclNode) {
                    var n   = (EnumDeclNode) decl;
                    var sym = new Symbol(n.Name, SymbolKind.SYM_ENUM, n);
                    _DeclareGlobalOrError(sym);

                } else if (decl is RecordDeclNode) {
                    var n   = (RecordDeclNode) decl;
                    var sym = new Symbol(n.Name, SymbolKind.SYM_RECORD, n);
                    _DeclareGlobalOrError(sym);

                } else if (decl is DataClassDeclNode) {
                    var n   = (DataClassDeclNode) decl;
                    var sym = new Symbol(n.Name, SymbolKind.SYM_DATA_CLASS, n);
                    _DeclareGlobalOrError(sym);
                }
            }
        }


        // ═══════════════════════════════════════════════
        //  Pass 2 — Visitor overrides
        // ═══════════════════════════════════════════════

        // ── Program ────────────────────────────────────

        public override void VisitProgram(ProgramNode n) {
            // Register namespace alias in global scope
            if (n.Namespace != null)
                n.Namespace.Accept(this);

            // Register import aliases
            foreach (var imp in n.Imports)
                imp.Accept(this);

            // Visit all top-level declarations
            foreach (var decl in n.Declarations)
                decl.Accept(this);
        }

        public override void VisitNamespace(NamespaceNode n) {
            var sym = new Symbol(n.Name, SymbolKind.SYM_NAMESPACE, n);
            // Namespaces may be declared in multiple files; ignore dups
            _table.DeclareGlobal(sym);
        }

        public override void VisitImport(ImportNode n) {
            // Register the alias (or last segment) in global scope
            string alias = n.Alias ?? _LastSegment(n.Name);
            var sym      = new Symbol(alias, SymbolKind.SYM_IMPORT, n);
            if (!_table.DeclareGlobal(sym)) {
                // Duplicate import — warn but continue
                _Warn("Duplicate import '%s'".printf(n.Name), n);
            }
        }


        // ── Class ──────────────────────────────────────

        public override void VisitClassDecl(ClassDeclNode n) {
            // Validate base class exists (if any)
            if (n.BaseClass != null)
                _CheckTypeExists(n.BaseClass);

            // Validate interfaces exist
            foreach (var iface in n.Interfaces)
                _CheckTypeExists(iface);

            string prev = _currentClass;
            _currentClass = n.Name;
            _table.PushScope("class:%s".printf(n.Name));

            // Register 'this' in the class scope
            var thisSym      = new Symbol("this", SymbolKind.SYM_LOCAL_VAR, n);
            thisSym.TypeKey  = n.Name;
            _table.Declare(thisSym);

            // Pass 1 within class: collect all members first
            // so methods can reference fields declared below them
            _CollectClassMembers(n);

            // Pass 2 within class: fully visit each member
            foreach (var member in n.Members)
                member.Accept(this);

            _table.PopScope();
            _currentClass = prev;
        }

        /**
         * Pre-register all fields, properties and methods of a
         * class before visiting their bodies.
         */
        private void _CollectClassMembers(ClassDeclNode n) {
            foreach (var member in n.Members) {

                if (member is FieldDeclNode) {
                    var f   = (FieldDeclNode) member;
                    var sym = new Symbol(f.Name, SymbolKind.SYM_FIELD, f);
                    sym.IsStatic = f.IsStatic;
                    _table.Declare(sym);

                } else if (member is PropertyDeclNode) {
                    var p   = (PropertyDeclNode) member;
                    var sym = new Symbol(p.Name, SymbolKind.SYM_PROP, p);
                    sym.IsStatic = p.IsStatic;
                    _table.Declare(sym);

                } else if (member is MethodDeclNode) {
                    var m   = (MethodDeclNode) member;
                    var sym = new Symbol(m.Name, SymbolKind.SYM_METHOD, m);
                    sym.IsStatic = m.IsStatic;
                    _table.Declare(sym);

                } else if (member is ConstructorDeclNode) {
                    var c   = (ConstructorDeclNode) member;
                    var sym = new Symbol(
                        _currentClass ?? "constructor",
                        SymbolKind.SYM_CONSTRUCTOR, c);
                    _table.Declare(sym);
                }
            }
        }


        // ── Interface ──────────────────────────────────

        public override void VisitInterfaceDecl(InterfaceDeclNode n) {
            foreach (var bt in n.BaseTypes)
                _CheckTypeExists(bt);

            _table.PushScope("interface:%s".printf(n.Name));
            foreach (var member in n.Members)
                member.Accept(this);
            _table.PopScope();
        }


        // ── Enum ───────────────────────────────────────

        public override void VisitEnumDecl(EnumDeclNode n) {
            _table.PushScope("enum:%s".printf(n.Name));

            foreach (var member in n.Members) {
                var sym = new Symbol(
                    member.Name, SymbolKind.SYM_ENUM_MEMBER, member);
                sym.TypeKey = n.Name;
                if (!_table.Declare(sym))
                    _Error("Duplicate enum member '%s'".printf(member.Name),
                           member);
            }

            foreach (var method in n.Methods)
                method.Accept(this);

            _table.PopScope();
        }


        // ── Record ─────────────────────────────────────

        public override void VisitRecordDecl(RecordDeclNode n) {
            _table.PushScope("record:%s".printf(n.Name));

            foreach (var param in n.Params) {
                _CheckTypeExists(param.ParamType);
                var sym = new Symbol(param.Name, SymbolKind.SYM_FIELD, param);
                sym.TypeKey = _TypeKey(param.ParamType);
                sym.IsLet   = true;  // record fields are immutable
                if (!_table.Declare(sym))
                    _Error("Duplicate record field '%s'".printf(param.Name),
                           param);
            }

            foreach (var method in n.Methods)
                method.Accept(this);

            _table.PopScope();
        }


        // ── Data class ─────────────────────────────────

        public override void VisitDataClassDecl(DataClassDeclNode n) {
            string prev   = _currentClass;
            _currentClass = n.Name;
            _table.PushScope("data:%s".printf(n.Name));

            foreach (var param in n.Params) {
                _CheckTypeExists(param.ParamType);
                var sym = new Symbol(param.Name, SymbolKind.SYM_FIELD, param);
                sym.TypeKey = _TypeKey(param.ParamType);
                if (!_table.Declare(sym))
                    _Error("Duplicate data class field '%s'"
                           .printf(param.Name), param);
            }

            foreach (var member in n.Members)
                member.Accept(this);

            _table.PopScope();
            _currentClass = prev;
        }


        // ── Method ─────────────────────────────────────

        public override void VisitMethodDecl(MethodDeclNode n) {
            // Validate return type
            if (n.ReturnType != null)
                _CheckTypeExists(n.ReturnType);

            string? prevReturn = _currentReturnType;
            bool    prevAsync  = _inAsync;

            _currentReturnType = (n.ReturnType != null)
                                 ? _TypeKey(n.ReturnType) : "void";
            _inAsync           = n.IsAsync;

            _table.PushScope("method:%s".printf(n.Name));

            // Register generic type params as local type aliases
            foreach (var gp in n.Generics) {
                var sym = new Symbol(gp.Name, SymbolKind.SYM_CLASS, null);
                sym.Location = "<generic-param>";
                _table.Declare(sym);
            }

            // Register parameters
            foreach (var param in n.Params)
                param.Accept(this);

            // Visit body
            if (n.Body != null)
                n.Body.Accept(this);

            _table.PopScope();

            _currentReturnType = prevReturn;
            _inAsync           = prevAsync;
        }


        // ── Constructor ────────────────────────────────

        public override void VisitConstructorDecl(ConstructorDeclNode n) {
            _table.PushScope("constructor:%s"
                             .printf(_currentClass ?? "?"));

            foreach (var param in n.Params)
                param.Accept(this);

            n.Body.Accept(this);

            _table.PopScope();
        }


        // ── Parameter ──────────────────────────────────

        public override void VisitParam(ParamNode n) {
            _CheckTypeExists(n.ParamType);

            var sym     = new Symbol(n.Name, SymbolKind.SYM_PARAMETER, n);
            sym.TypeKey = _TypeKey(n.ParamType);
            sym.IsLet   = true;  // parameters are immutable by default

            if (!_table.Declare(sym))
                _Error("Duplicate parameter '%s'".printf(n.Name), n);

            // Visit default value expression
            if (n.Default != null)
                n.Default.Accept(this);
        }


        // ── Field / Property (already declared in _CollectClassMembers)

        public override void VisitFieldDecl(FieldDeclNode n) {
            _CheckTypeExists(n.FieldType);
            if (n.Initial != null)
                n.Initial.Accept(this);
        }

        public override void VisitPropertyDecl(PropertyDeclNode n) {
            _CheckTypeExists(n.PropType);
            if (n.Getter != null) n.Getter.Accept(this);
            if (n.Setter != null) n.Setter.Accept(this);
            if (n.Initial != null) n.Initial.Accept(this);
        }


        // ── Statements ─────────────────────────────────

        public override void VisitBlock(BlockNode n) {
            _table.PushScope("block");
            foreach (var stmt in n.Statements)
                stmt.Accept(this);
            _table.PopScope();
        }

        public override void VisitVarDecl(VarDeclNode n) {
            // Visit initialiser first (right-hand side must not see
            // the variable being declared — prevents `let x = x`)
            if (n.Initial != null)
                n.Initial.Accept(this);

            if (n.VarType != null)
                _CheckTypeExists(n.VarType);

            var sym     = new Symbol(n.Name, SymbolKind.SYM_LOCAL_VAR, n);
            sym.IsLet   = n.IsLet;
            sym.TypeKey = (n.VarType != null) ? _TypeKey(n.VarType) : "";

            if (!_table.Declare(sym))
                _Error("Variable '%s' already declared in this scope"
                       .printf(n.Name), n);
        }

        public override void VisitIf(IfNode n) {
            n.Condition.Accept(this);
            n.ThenBlock.Accept(this);
            foreach (var ei in n.ElseIfs)
                ei.Accept(this);
            if (n.ElseBlock != null)
                n.ElseBlock.Accept(this);
        }

        public override void VisitElseIf(ElseIfNode n) {
            n.Condition.Accept(this);
            n.Block.Accept(this);
        }

        public override void VisitMatch(MatchNode n) {
            n.Subject.Accept(this);
            foreach (var arm in n.Arms)
                arm.Accept(this);
        }

        public override void VisitMatchArm(MatchArmNode n) {
            _table.PushScope("match-arm");
            n.Pattern.Accept(this);
            n.Body.Accept(this);
            _table.PopScope();
        }

        public override void VisitMatchPattern(MatchPatternNode n) {
            switch (n.Kind) {
                case MatchPatternKind.LITERAL:
                    if (n.Value != null) n.Value.Accept(this);
                    break;

                case MatchPatternKind.RANGE:
                    if (n.Value    != null) n.Value.Accept(this);
                    if (n.RangeEnd != null) n.RangeEnd.Accept(this);
                    break;

                case MatchPatternKind.TYPE:
                    if (n.PatternType != null)
                        _CheckTypeExists(n.PatternType);
                    // Bind the capture variable into the arm scope
                    if (n.BindName != null) {
                        var sym     = new Symbol(n.BindName,
                                                 SymbolKind.SYM_LOCAL_VAR, n);
                        sym.TypeKey = (n.PatternType != null)
                                      ? _TypeKey(n.PatternType) : "";
                        _table.Declare(sym);
                    }
                    break;

                case MatchPatternKind.DESTRUCTURE:
                    if (n.PatternType != null)
                        _CheckTypeExists(n.PatternType);
                    foreach (var sub in n.SubPatterns)
                        sub.Accept(this);
                    break;

                case MatchPatternKind.GUARD:
                    if (n.Value != null) n.Value.Accept(this);
                    if (n.Guard != null) n.Guard.Accept(this);
                    break;

                case MatchPatternKind.ENUM_VARIANT:
                    if (n.Value != null) n.Value.Accept(this);
                    break;

                default:
                    // WILDCARD — nothing to resolve
                    break;
            }
        }

        public override void VisitWhile(WhileNode n) {
            n.Condition.Accept(this);
            _loopDepth++;
            n.Body.Accept(this);
            _loopDepth--;
        }

        public override void VisitFor(ForNode n) {
            _table.PushScope("for");
            n.Init.Accept(this);
            n.Condition.Accept(this);
            n.Step.Accept(this);
            _loopDepth++;
            n.Body.Accept(this);
            _loopDepth--;
            _table.PopScope();
        }

        public override void VisitForeach(ForeachNode n) {
            n.Collection.Accept(this);

            _table.PushScope("foreach");
            var sym   = new Symbol(n.VarName, SymbolKind.SYM_LOCAL_VAR, n);
            sym.IsLet = n.IsLet;
            _table.Declare(sym);

            _loopDepth++;
            n.Body.Accept(this);
            _loopDepth--;
            _table.PopScope();
        }

        public override void VisitReturn(ReturnNode n) {
            if (n.Value != null)
                n.Value.Accept(this);
        }

        public override void VisitGuard(GuardNode n) {
            n.Condition.Accept(this);
            n.ElseBlock.Accept(this);
        }

        public override void VisitBreak(BreakNode n) {
            if (_loopDepth == 0)
                _Error("'break' used outside of a loop", n);
        }

        public override void VisitContinue(ContinueNode n) {
            if (_loopDepth == 0)
                _Error("'continue' used outside of a loop", n);
        }

        public override void VisitTryCatch(TryCatchNode n) {
            n.TryBlock.Accept(this);

            _table.PushScope("catch");
            // Register the caught error variable
            var sym     = new Symbol(n.ErrorName, SymbolKind.SYM_LOCAL_VAR, n);
            sym.TypeKey = n.ErrorType;
            _table.Declare(sym);
            n.CatchBlock.Accept(this);
            _table.PopScope();
        }

        public override void VisitGoStmt(GoStmtNode n) {
            n.Expression.Accept(this);
        }


        // ── Expressions ────────────────────────────────

        public override void VisitBinaryExpr(BinaryExprNode n) {
            n.Left.Accept(this);
            n.Right.Accept(this);
        }

        public override void VisitUnaryExpr(UnaryExprNode n) {
            n.Operand.Accept(this);
        }

        public override void VisitMemberAccess(MemberAccessNode n) {
            // Only resolve the target; the member itself is
            // validated by the TypeChecker once types are known.
            n.Target.Accept(this);
        }

        public override void VisitCallExpr(CallExprNode n) {
            n.Callee.Accept(this);
            foreach (var arg in n.Arguments)
                arg.Accept(this);
            foreach (var kv in n.NamedArgs.entries)
                kv.value.Accept(this);
            foreach (var targ in n.Generics)
                _CheckTypeExists(targ);
        }

        public override void VisitIndexExpr(IndexExprNode n) {
            n.Target.Accept(this);
            n.Index.Accept(this);
        }

        public override void VisitAssignExpr(AssignExprNode n) {
            n.Target.Accept(this);
            n.Value.Accept(this);

            // Immutability check: refuse assignment to a 'let' variable
            if (n.Target is IdentifierNode) {
                var id  = (IdentifierNode) n.Target;
                var sym = _table.Lookup(id.Name);
                if (sym != null && sym.IsLet && n.Operator == "=")
                    _Error("Cannot assign to immutable binding '%s'"
                           .printf(id.Name), n);
            }
        }

        public override void VisitNewExpr(NewExprNode n) {
            _CheckTypeExists(n.ObjectType);
            foreach (var arg in n.Arguments)
                arg.Accept(this);
            foreach (var kv in n.NamedArgs.entries)
                kv.value.Accept(this);
        }

        public override void VisitLambdaExpr(LambdaExprNode n) {
            _table.PushScope("lambda");
            foreach (var param in n.Params)
                param.Accept(this);
            n.Body.Accept(this);
            _table.PopScope();
        }

        public override void VisitAwaitExpr(AwaitExprNode n) {
            if (!_inAsync)
                _Error("'await' used outside of an 'async' method", n);
            n.Expression.Accept(this);
        }

        public override void VisitWithExpr(WithExprNode n) {
            n.Source.Accept(this);
            foreach (var kv in n.Changes.entries)
                kv.value.Accept(this);
        }

        public override void VisitListLiteral(ListLiteralNode n) {
            if (n.IsComprehension) {
                _table.PushScope("list-comprehension");
                // Declare the comprehension variable
                if (n.CompVarName != null) {
                    var sym = new Symbol(n.CompVarName,
                                         SymbolKind.SYM_LOCAL_VAR, n);
                    _table.Declare(sym);
                }
                if (n.CompSource != null) n.CompSource.Accept(this);
                if (n.CompFilter != null) n.CompFilter.Accept(this);
                if (n.CompExpr   != null) n.CompExpr.Accept(this);
                _table.PopScope();
            } else {
                foreach (var el in n.Elements)
                    el.Accept(this);
            }
        }

        public override void VisitMapLiteral(MapLiteralNode n) {
            foreach (var entry in n.Entries)
                entry.Accept(this);
        }

        public override void VisitMapEntry(MapEntryNode n) {
            n.Key.Accept(this);
            n.Value.Accept(this);
        }

        public override void VisitIdentifier(IdentifierNode n) {
            var sym = _table.Lookup(n.Name);
            if (sym != null) return;  // resolved — all good

            // Unknown symbol: build a helpful error message
            string? suggestion = _table.DidYouMean(n.Name);
            string  msg;

            if (suggestion != null)
                msg = "Unknown symbol '%s' — did you mean '%s'?"
                      .printf(n.Name, suggestion);
            else
                msg = "Unknown symbol '%s'".printf(n.Name);

            _Error(msg, n);
        }

        public override void VisitThis(ThisNode n) {
            if (_currentClass == null)
                _Error("'this' used outside of a class", n);
        }

        public override void VisitNull(NullNode n) {
            // 'null' is always valid — no check needed
        }

        public override void VisitLiteral(LiteralNode n) {
            // Visit interpolated string segments
            if (n.Kind == LiteralKind.INTERPOLATED_STRING &&
                n.Segments != null) {
                foreach (var seg in n.Segments)
                    seg.Accept(this);
            }
        }

        // ── Types (checked via _CheckTypeExists, not Visit) ────────

        public override void VisitSimpleType  (SimpleTypeNode  n) {}
        public override void VisitGenericType (GenericTypeNode n) {}
        public override void VisitFuncType    (FuncTypeNode    n) {}
        public override void VisitTupleType   (TupleTypeNode   n) {}


        // ═══════════════════════════════════════════════
        //  Private helpers
        // ═══════════════════════════════════════════════

        /**
         * Validate that a TypeNode refers to a known type.
         * Reports a ResolveError if not found.
         */
        private void _CheckTypeExists(TypeNode type) {
            if (type is SimpleTypeNode) {
                var st   = (SimpleTypeNode) type;
                var name = st.Name;

                // Strip trailing '?' before lookup
                if (name.has_suffix("?"))
                    name = name.substring(0, name.length - 1);

                // Strip trailing '[]' — array of a known type is valid
                if (name.has_suffix("[]"))
                    name = name.substring(0, name.length - 2);

                if (_table.Lookup(name) == null) {
                    string? sug = _table.DidYouMean(name);
                    string  msg = (sug != null)
                        ? "Unknown type '%s' — did you mean '%s'?"
                          .printf(name, sug)
                        : "Unknown type '%s'".printf(name);
                    _Error(msg, type);
                }

            } else if (type is GenericTypeNode) {
                var gt = (GenericTypeNode) type;
                if (_table.Lookup(gt.Name) == null) {
                    string? sug = _table.DidYouMean(gt.Name);
                    string  msg = (sug != null)
                        ? "Unknown type '%s' — did you mean '%s'?"
                          .printf(gt.Name, sug)
                        : "Unknown type '%s'".printf(gt.Name);
                    _Error(msg, type);
                }
                // Recurse into type arguments
                foreach (var targ in gt.TypeArgs)
                    _CheckTypeExists(targ);

            } else if (type is FuncTypeNode) {
                var ft = (FuncTypeNode) type;
                foreach (var pt in ft.ParamTypes)
                    _CheckTypeExists(pt);
                _CheckTypeExists(ft.ReturnType);

            } else if (type is TupleTypeNode) {
                var tt = (TupleTypeNode) type;
                foreach (var et in tt.ElementTypes)
                    _CheckTypeExists(et);
            }
        }

        /**
         * Declare a symbol in the global scope.
         * Reports an error on duplicate (e.g. two classes
         * with the same name in the same namespace).
         */
        private void _DeclareGlobalOrError(Symbol sym) {
            if (!_table.DeclareGlobal(sym)) {
                _Error("Duplicate top-level declaration '%s'"
                       .printf(sym.Name), sym.DeclNode);
            }
        }

        /**
         * Produce a string type key from a TypeNode.
         * Used to populate Symbol.TypeKey before the
         * TypeChecker runs its full inference pass.
         */
        private string _TypeKey(TypeNode type) {
            if (type is SimpleTypeNode)
                return ((SimpleTypeNode) type).Name;

            if (type is GenericTypeNode) {
                var gt  = (GenericTypeNode) type;
                var sb  = new StringBuilder(gt.Name);
                sb.append("<");
                bool first = true;
                foreach (var ta in gt.TypeArgs) {
                    if (!first) sb.append(", ");
                    sb.append(_TypeKey(ta));
                    first = false;
                }
                sb.append(">");
                return sb.str;
            }

            if (type is TupleTypeNode) {
                var tt = (TupleTypeNode) type;
                var sb = new StringBuilder("(");
                bool first = true;
                foreach (var et in tt.ElementTypes) {
                    if (!first) sb.append(", ");
                    sb.append(_TypeKey(et));
                    first = false;
                }
                sb.append(")");
                return sb.str;
            }

            if (type is FuncTypeNode) {
                var ft = (FuncTypeNode) type;
                return "Func<…,%s>".printf(_TypeKey(ft.ReturnType));
            }

            return "?";
        }

        /**
         * Returns the last dot-separated segment of a qualified name.
         *   "Code.IO"     → "IO"
         *   "MyApp"       → "MyApp"
         */
        private string _LastSegment(string name) {
            int dot = name.last_index_of(".");
            return (dot >= 0) ? name.substring(dot + 1) : name;
        }

        /**
         * Emit a resolve error from an AstNode position.
         */
        private void _Error(string message, AstNode? node) {
            ResolveError err;
            if (node != null)
                err = new ResolveError.from_node(message, node);
            else
                err = new ResolveError(message, _filename, 0, 0);
            _errors.add(err);
        }

        /**
         * Emit a non-fatal warning (currently stored as an error
         * with a [warning] prefix so the caller can filter).
         * A proper Warning class can be added in a later pass.
         */
        private void _Warn(string message, AstNode node) {
            _Error("[warning] " + message, node);
        }
    }
}
