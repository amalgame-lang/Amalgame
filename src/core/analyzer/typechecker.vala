// ─────────────────────────────────────────────────────
//  Amalgame Programming Language
//  Copyright (c) 2026 Bastien MOUGET
//  Licensed under Apache 2.0
//  https://github.com/BastienMOUGET/Amalgame
// ─────────────────────────────────────────────────────

// ═══════════════════════════════════════════════════════
//  typechecker.vala  -  Type inference & validation pass
//
//  Runs after the Resolver. Expects a fully name-resolved
//  AST and the SymbolTable produced by the Resolver.
//
//  Responsibilities:
//  1. Infer types for every expression node (stored in
//     ExprTypes, a parallel map node → TypeKey string).
//  2. Annotate every Symbol with its resolved TypeKey.
//  3. Validate type compatibility at every use site:
//       - assignments and compound assignments
//       - call arguments vs parameter types
//       - return expressions vs declared return type
//       - binary / unary operator operand types
//       - null safety (nullable vs non-nullable)
//       - if / while / guard conditions must be bool
//  4. Validate member accesses (player.Name → Player has
//     a field/property called Name).
//  5. Validate method calls (argument count & types).
//
//  What the TypeChecker does NOT do (deferred):
//  - Full generic type unification
//  - Variance checking (covariance / contravariance)
//  - Flow-sensitive narrowing (smart casts)
//  These are left for a future Hindley-Milner pass.
//
//  Usage:
//    var tc     = new TypeChecker(symbolTable, filename);
//    var result = tc.Check(ast);
//    if (!result.Success)
//        foreach (var e in result.Errors)
//            stderr.printf(e.ToString());
// ═══════════════════════════════════════════════════════

namespace CodeTranspiler.Analyzer {

    using CodeTranspiler.Ast;
    using CodeTranspiler.Lexer;
    // ═══════════════════════════════════════════════════
    //  Type error
    // ═══════════════════════════════════════════════════

    /**
     * A single type error with source location.
     * Uses the same box-drawing format as ResolveError.
     */
    public class TypeError : Object {

        public string Message  { get; set; }
        public int    Line     { get; set; }
        public int    Column   { get; set; }
        public string Filename { get; set; }

        public TypeError(string message,
                         string filename,
                         int    line,
                         int    column) {
            Message  = message;
            Filename = filename;
            Line     = line;
            Column   = column;
        }

        public TypeError.from_node(string message, AstNode node) {
            Message  = message;
            Filename = node.Filename;
            Line     = node.Line;
            Column   = node.Column;
        }

        public string ToString() {
            return "\n┌── [typechecker] %s:%d:%d\n│\n│  %s\n│\n└──\n"
                   .printf(Filename, Line, Column, Message);
        }
    }
    // ═══════════════════════════════════════════════════
    //  Type check result
    // ═══════════════════════════════════════════════════

    public class TypeCheckResult : Object {
        public bool                        Success { get; set; }
        public ProgramNode?                Program { get; set; }
        public Gee.ArrayList<TypeError>    Errors  { get; set; }
        public SymbolTable                 Symbols { get; set; }

        public TypeCheckResult() {
            Errors = new Gee.ArrayList<TypeError>();
        }
    }
    // ═══════════════════════════════════════════════════
    //  TypeChecker
    // ═══════════════════════════════════════════════════

    public class TypeChecker : BaseAstVisitor {

        // ── Internal state ─────────────────────────────

        private SymbolTable                        _table;
        private string                             _filename;
        private Gee.ArrayList<TypeError>           _errors;

        /**
         * Maps every expression AstNode (by object identity via
         * its Location string + pointer approximation) to its
         * inferred TypeKey.  Used by parent nodes to retrieve
         * the type of a child expression after visiting it.
         *
         * Key   : AstNode reference (stored as pointer string)
         * Value : TypeKey string, e.g. "int", "Player", "List<int>"
         */
        private Gee.HashMap<AstNode, string>       _exprTypes;

        /** Return type expected in the current method. */
        private string  _currentReturnType;

        /** Name of the class currently being checked. */
        private string? _currentClass;

        /** Whether we are inside an async method. */
        private bool    _inAsync;
        // ── Constructor ────────────────────────────────

        public TypeChecker(SymbolTable table,
                           string      filename = "<unknown>") {
            _table             = table;
            _filename          = filename;
            _errors            = new Gee.ArrayList<TypeError>();
            _exprTypes         = new Gee.HashMap<AstNode, string>();
            _currentReturnType = "void";
            _currentClass      = null;
            _inAsync           = false;
        }
        // ═══════════════════════════════════════════════
        //  Public entry point
        // ═══════════════════════════════════════════════

        public TypeCheckResult Check(ProgramNode program) {
            program.Accept(this);

            var result     = new TypeCheckResult();
            result.Program = program;
            result.Symbols = _table;
            result.Errors  = _errors;
            result.Success = (_errors.size == 0);
            return result;
        }
        // ═══════════════════════════════════════════════
        //  Helpers — type inference storage
        // ═══════════════════════════════════════════════

        /** Store the inferred type of an expression node. */
        private void _SetType(AstNode node, string typeKey) {
            _exprTypes[node] = typeKey;
        }

        /**
         * Retrieve the inferred type of an expression node.
         * Returns "?" when the type could not be determined.
         */
        private string _GetType(AstNode node) {
            return _exprTypes.has_key(node) ? _exprTypes[node] : "?";
        }
        // ═══════════════════════════════════════════════
        //  Helpers — type compatibility
        // ═══════════════════════════════════════════════

        /**
         * Returns true when `actual` is assignable to `expected`.
         *
         * Rules (simplified — full generics unification is deferred):
         *   • "?" (unknown) is compatible with anything (avoids
         *     cascading errors when a previous check already failed).
         *   • exact match always wins.
         *   • numeric widening: int → float → double.
         *   • null is assignable to any nullable type (ends with "?").
         *   • any type is assignable to "object" (top type).
         *   • a type is assignable to itself with/without "?".
         */
        private bool _IsAssignable(string expected, string actual) {
            if (expected == "?" || actual == "?") return true;
            if (expected == actual)               return true;
            if (expected == "object")             return true;

            // Strip trailing "?" for base-type comparison
            string eBase = expected.has_suffix("?")
                           ? expected.substring(0, expected.length - 1)
                           : expected;
            string aBase = actual.has_suffix("?")
                           ? actual.substring(0, actual.length - 1)
                           : actual;

            if (eBase == aBase) return true;

            // null → any nullable
            if (actual == "null" && expected.has_suffix("?")) return true;

            // Numeric widening
            if (_IsNumericWiden(eBase, aBase)) return true;

            return false;
        }

        private bool _IsNumericWiden(string to, string from) {
            // int → float, int → double, float → double
            // i8/i16/i32/i64 family widening
            if (to == "double" &&
                (from == "float" || from == "int" ||
                 from == "i8"  || from == "i16" ||
                 from == "i32" || from == "i64" ||
                 from == "f32")) return true;

            if (to == "float" &&
                (from == "int" ||
                 from == "i8"  || from == "i16" ||
                 from == "i32" || from == "f32")) return true;

            return false;
        }

        private bool _IsBool(string t) {
            return t == "bool" || t == "?";
        }

        private bool _IsNumeric(string t) {
            string[] nums = {
                "int","float","double",
                "i8","i16","i32","i64",
                "u8","u16","u32","u64",
                "f32","f64","byte","char"
            };
            foreach (var n in nums)
                if (t == n) return true;
            return false;
        }

        private bool _IsNullable(string t) {
            return t.has_suffix("?");
        }

        /**
         * Resolve the result type of a binary operator given
         * the types of its left and right operands.
         */
        private string _BinaryResultType(string op,
                                         string left,
                                         string right) {
            switch (op) {
                // Comparison → always bool
                case "==": case "!=":
                case "<":  case ">":
                case "<=": case ">=":
                    return "bool";

                // Logical → bool
                case "&&": case "||":
                    return "bool";

                // Null coalescing → base type of left (strip "?")
                case "??":
                    return left.has_suffix("?")
                           ? left.substring(0, left.length - 1)
                           : left;

                // Range → special "range" pseudo-type
                case "..": case "...":
                    return "range";

                // Pipeline |> → type of right (the function result)
                case "|>":
                    return right;

                // Composition >> → func type (simplified)
                case ">>":
                    return right;

                // Arithmetic — widen to the "larger" numeric type
                case "+": case "-": case "*":
                case "/": case "%": case "^":
                    if (left == "double" || right == "double")
                        return "double";
                    if (left == "float" || right == "float")
                        return "float";
                    // String concatenation via +
                    if (left == "string" || right == "string")
                        return "string";
                    return left;  // fallback

                default:
                    return "?";
            }
        }

        // ═══════════════════════════════════════════════
        //  Visitor — Program & top-level
        // ═══════════════════════════════════════════════

        public override void VisitProgram(ProgramNode n) {
            foreach (var decl in n.Declarations)
                decl.Accept(this);
        }

        public override void VisitNamespace(NamespaceNode n) {}
        public override void VisitImport(ImportNode n) {}
        public override void VisitDecorator(DecoratorNode n) {}
        // ═══════════════════════════════════════════════
        //  Visitor — Declarations
        // ═══════════════════════════════════════════════

        public override void VisitClassDecl(ClassDeclNode n) {
            string? prev  = _currentClass;
            _currentClass = n.Name;
            foreach (var m in n.Members) m.Accept(this);
            
            _currentClass = prev;
        }

        public override void VisitInterfaceDecl(InterfaceDeclNode n) {
            foreach (var m in n.Members) m.Accept(this);
            
        }

        public override void VisitEnumDecl(EnumDeclNode n) {
            foreach (var m in n.Methods) m.Accept(this);
            
        }

        public override void VisitEnumMember(EnumMemberNode n) {}

        public override void VisitRecordDecl(RecordDeclNode n) {
            foreach (var m in n.Methods) m.Accept(this);
            
        }

        public override void VisitRecordParam(RecordParamNode n) {}

        public override void VisitDataClassDecl(DataClassDeclNode n) {
            string? prev  = _currentClass;
            _currentClass = n.Name;
            foreach (var m in n.Members) m.Accept(this);
            
            _currentClass = prev;
        }

        public override void VisitFieldDecl(FieldDeclNode n) {
            string fieldType = _TypeKey(n.FieldType);

            // Update symbol TypeKey if not yet set
            var sym = _table.Lookup(n.Name);
            if (sym != null && sym.TypeKey == "")
                sym.TypeKey = fieldType;

            if (n.Initial != null) {
                n.Initial.Accept(this);
                string initType = _GetType(n.Initial);
                if (!_IsAssignable(fieldType, initType))
                    _Error("Field '%s' declared as '%s' but initialised with '%s'".printf(n.Name, fieldType, initType),
                           n.Initial);
            }
        }

        public override void VisitPropertyDecl(PropertyDeclNode n) {
            string propType = _TypeKey(n.PropType);

            var sym = _table.Lookup(n.Name);
            if (sym != null && sym.TypeKey == "")
                sym.TypeKey = propType;

            if (n.Initial != null) {
                n.Initial.Accept(this);
                string initType = _GetType(n.Initial);
                if (!_IsAssignable(propType, initType))
                    _Error("Property '%s' declared as '%s' but initialised with '%s'"
                           .printf(n.Name, propType, initType),
                           n.Initial);
            }
            if (n.Getter != null) n.Getter.Accept(this);
            if (n.Setter != null) n.Setter.Accept(this);
        }

        public override void VisitMethodDecl(MethodDeclNode n) {
            // Always read return type directly from the AST node —
            // don't rely on Symbol.TypeKey which may not be set yet
            // when the class scope is not active.
            string retType = (n.ReturnType != null)
                             ? _TypeKey(n.ReturnType) : "void";

            // Wrap async return type in Task<T> if not already
            if (n.IsAsync && !retType.has_prefix("Task"))
                retType = "Task<%s>".printf(retType);

            // Annotate the method symbol if found
            var sym = _table.Lookup(n.Name);
            if (sym != null && sym.TypeKey == "")
                sym.TypeKey = retType;

            string prevReturn      = _currentReturnType;
            bool   prevAsync       = _inAsync;
            _currentReturnType     = retType;
            _inAsync               = n.IsAsync;

            foreach (var p in n.Params) p.Accept(this);

            if (n.Body != null) {
                n.Body.Accept(this);

                // Expression body (=> expr) — check return type
                if (!(n.Body is BlockNode)) {
                    string bodyType = _GetType(n.Body);
                    string baseRet  = retType.has_prefix("Task<")
                        ? retType.substring(5, retType.length - 6)
                        : retType;
                    if (baseRet != "void" &&
                        !_IsAssignable(baseRet, bodyType))
                        _Error("Method '%s' returns '%s' but body has type '%s'"
                               .printf(n.Name, retType, bodyType),
                               n.Body);
                }
            }

            _currentReturnType = prevReturn;
            _inAsync           = prevAsync;
        }

        public override void VisitConstructorDecl(ConstructorDeclNode n) {
            string prev        = _currentReturnType;
            _currentReturnType = "void";

            foreach (var p in n.Params) p.Accept(this);
            n.Body.Accept(this);
            

            _currentReturnType = prev;
        }

        public override void VisitParam(ParamNode n) {
            string typeKey = _TypeKey(n.ParamType);
            var    sym     = _table.Lookup(n.Name);
            if (sym != null && sym.TypeKey == "")
                sym.TypeKey = typeKey;

            if (n.Default != null) {
                n.Default.Accept(this);
                string defType = _GetType(n.Default);
                if (!_IsAssignable(typeKey, defType))
                    _Error("Parameter '%s' is '%s' but default is '%s'"
                           .printf(n.Name, typeKey, defType),
                           n.Default);
            }
        }
        // ═══════════════════════════════════════════════
        //  Visitor — Statements
        // ═══════════════════════════════════════════════

        public override void VisitBlock(BlockNode n) {
            
            foreach (var s in n.Statements) s.Accept(this);
            
        }

        public override void VisitVarDecl(VarDeclNode n) {
            string? declaredType = (n.VarType != null)
                                   ? _TypeKey(n.VarType) : null;
            string  inferredType = "?";

            if (n.Initial != null) {
                n.Initial.Accept(this);
                inferredType = _GetType(n.Initial);
            }

            // Determine the final type
            string finalType;
            if (declaredType != null) {
                // Check compatibility if both sides are known
                if (inferredType != "?" &&
                    !_IsAssignable(declaredType, inferredType))
                    _Error("Cannot assign '%s' to variable '%s' of type '%s'"
                           .printf(inferredType, n.Name, declaredType),
                           n);
                finalType = declaredType;
            } else {
                finalType = inferredType; // inferred
            }

            // Annotate the symbol
            var sym = _table.Lookup(n.Name);
            if (sym != null)
                sym.TypeKey = finalType;
        }

        public override void VisitIf(IfNode n) {
            n.Condition.Accept(this);
            _CheckBool(n.Condition, "if condition");
            n.ThenBlock.Accept(this);
            foreach (var ei in n.ElseIfs) ei.Accept(this);
            if (n.ElseBlock != null) n.ElseBlock.Accept(this);
        }

        public override void VisitElseIf(ElseIfNode n) {
            n.Condition.Accept(this);
            _CheckBool(n.Condition, "else if condition");
            n.Block.Accept(this);
        }

        public override void VisitWhile(WhileNode n) {
            n.Condition.Accept(this);
            _CheckBool(n.Condition, "while condition");
            n.Body.Accept(this);
        }

        public override void VisitFor(ForNode n) {
            
            n.Init.Accept(this);
            n.Condition.Accept(this);
            _CheckBool(n.Condition, "for condition");
            n.Step.Accept(this);
            n.Body.Accept(this);
            
        }

        public override void VisitForeach(ForeachNode n) {
            n.Collection.Accept(this);
            // Infer element type from collection type
            string colType  = _GetType(n.Collection);
            string elemType = _CollectionElementType(colType);
            var sym = _table.Lookup(n.VarName);
            if (sym != null && sym.TypeKey == "")
                sym.TypeKey = elemType;
            n.Body.Accept(this);
            
        }

        public override void VisitReturn(ReturnNode n) {
            if (n.Value == null) {
                // void return — check that method expects void
                string baseRet = _BaseReturnType();
                if (baseRet != "void" && _currentReturnType != "?")
                    _Error("Empty return in method expecting '%s'"
                           .printf(_currentReturnType), n);
                return;
            }

            n.Value.Accept(this);
            string valType = _GetType(n.Value);
            string baseRet2 = _BaseReturnType();

            if (baseRet2 == "void") {
                _Error("Cannot return a value from a void method", n);
            } else if (!_IsAssignable(baseRet2, valType)) {
                _Error("Return type mismatch: expected '%s', got '%s'"
                       .printf(_currentReturnType, valType), n);
            }
        }

        public override void VisitGuard(GuardNode n) {
            n.Condition.Accept(this);
            _CheckBool(n.Condition, "guard condition");
            n.ElseBlock.Accept(this);
        }

        public override void VisitBreak(BreakNode n) {}
        public override void VisitContinue(ContinueNode n) {}

        public override void VisitTryCatch(TryCatchNode n) {
            n.TryBlock.Accept(this);
            
            var sym = _table.Lookup(n.ErrorName);
            if (sym != null) sym.TypeKey = n.ErrorType;
            n.CatchBlock.Accept(this);
            
        }

        public override void VisitMatch(MatchNode n) {
            n.Subject.Accept(this);
            string subjectType = _GetType(n.Subject);
            foreach (var arm in n.Arms) {
                _VisitMatchArmTyped(arm, subjectType);
            }
        }

        private void _VisitMatchArmTyped(MatchArmNode arm,
                                          string subjectType) {
            
            arm.Pattern.Accept(this);

            // Bind pattern variable with subject type when TYPE pattern
            if (arm.Pattern.Kind == MatchPatternKind.TYPE &&
                arm.Pattern.BindName != null) {
                var sym = _table.Lookup(arm.Pattern.BindName);
                if (sym != null && sym.TypeKey == "") {
                    sym.TypeKey = (arm.Pattern.PatternType != null)
                        ? _TypeKey(arm.Pattern.PatternType)
                        : subjectType;
                }
            }

            arm.Body.Accept(this);
            
        }

        public override void VisitMatchArm(MatchArmNode n) {
            // Called only when not going through _VisitMatchArmTyped
            
            n.Pattern.Accept(this);
            n.Body.Accept(this);
            
        }

        public override void VisitMatchPattern(MatchPatternNode n) {
            if (n.Value    != null) n.Value.Accept(this);
            if (n.RangeEnd != null) n.RangeEnd.Accept(this);
            if (n.Guard    != null) {
                n.Guard.Accept(this);
                _CheckBool(n.Guard, "match guard");
            }
            foreach (var sub in n.SubPatterns) sub.Accept(this);
        }

        public override void VisitGoStmt(GoStmtNode n) {
            n.Expression.Accept(this);
        }
        // ═══════════════════════════════════════════════
        //  Visitor — Expressions
        // ═══════════════════════════════════════════════

        public override void VisitBinaryExpr(BinaryExprNode n) {
            n.Left.Accept(this);
            n.Right.Accept(this);

            string lt = _GetType(n.Left);
            string rt = _GetType(n.Right);

            // Operator-specific checks
            switch (n.Operator) {
                case "&&": case "||":
                    if (!_IsBool(lt))
                        _Error("Left operand of '%s' must be bool, got '%s'".printf(n.Operator, lt), n.Left);
                    if (!_IsBool(rt))
                        _Error("Right operand of '%s' must be bool, got '%s'".printf(n.Operator, rt), n.Right);
                    break;

                case "+":
                    // Allow string + anything (concatenation) or numeric
                    if (lt != "string" && rt != "string" &&
                        !_IsNumeric(lt) && lt != "?")
                        _Error("Operator '+' not applicable to '%s'"
                               .printf(lt), n.Left);
                    break;

                case "-": case "*": case "/":
                case "%": case "^":
                    if (!_IsNumeric(lt) && lt != "?")
                        _Error("Operator '%s' requires numeric operands, got '%s'".printf(n.Operator, lt), n.Left);
                    if (!_IsNumeric(rt) && rt != "?")
                        _Error("Operator '%s' requires numeric operands, got '%s'".printf(n.Operator, rt), n.Right);
                    break;

                case "??":
                    // Left must be nullable
                    if (!_IsNullable(lt) && lt != "?")
                        _Error("Left operand of '??' must be nullable, got '%s'".printf(lt), n.Left);
                    break;
            }

            string resultType = _BinaryResultType(n.Operator, lt, rt);
            _SetType(n, resultType);
        }

        public override void VisitUnaryExpr(UnaryExprNode n) {
            n.Operand.Accept(this);
            string ot = _GetType(n.Operand);

            switch (n.Operator) {
                case "!":
                    if (!_IsBool(ot))
                        _Error("Operator '!' requires bool, got '%s'"
                               .printf(ot), n.Operand);
                    _SetType(n, "bool");
                    break;
                case "-":
                    if (!_IsNumeric(ot) && ot != "?")
                        _Error("Unary '-' requires numeric, got '%s'"
                               .printf(ot), n.Operand);
                    _SetType(n, ot);
                    break;
                default:
                    _SetType(n, ot);
                    break;
            }
        }

        public override void VisitMemberAccess(MemberAccessNode n) {
            n.Target.Accept(this);
            string targetType = _GetType(n.Target);

            // Strip nullable marker for member lookup
            string baseType = targetType.has_suffix("?")
                ? targetType.substring(0, targetType.length - 1)
                : targetType;

            // Null-safe access on non-nullable type is suspicious
            if (n.IsNullSafe && !_IsNullable(targetType) &&
                targetType != "?" && targetType != "null")
                _Warn("Null-safe '?.' on non-nullable type '%s'"
                      .printf(targetType), n);

            // Look up the member in the class symbol table
            string memberType = "?";
            if (baseType != "?" && baseType != "null") {
                var classSym = _table.LookupGlobal(baseType);
                if (classSym != null && classSym.DeclNode != null) {
                    memberType = _ResolveMemberType(
                        classSym.DeclNode, n.MemberName, n);
                } else if (classSym == null) {
                    // Unknown type — already caught by Resolver
                }
            }

            _SetType(n, memberType);
        }

        public override void VisitCallExpr(CallExprNode n) {
            n.Callee.Accept(this);
            foreach (var arg in n.Arguments) arg.Accept(this);
            foreach (var kv in n.NamedArgs.entries) kv.value.Accept(this);

            // Determine the return type of the callee
            string calleeType = _GetType(n.Callee);

            // If callee is a method/function identifier, look up symbol
            if (n.Callee is IdentifierNode) {
                var id  = (IdentifierNode) n.Callee;
                var sym = _table.Lookup(id.Name);
                if (sym != null) {
                    _CheckCallArity(sym, n);
                    calleeType = sym.TypeKey;
                }
            } else if (n.Callee is MemberAccessNode) {
                // type is already set by VisitMemberAccess
                calleeType = _GetType(n.Callee);
            }

            // The result type of a call is the return type of the callee
            _SetType(n, calleeType != "" ? calleeType : "?");
        }

        public override void VisitIndexExpr(IndexExprNode n) {
            n.Target.Accept(this);
            n.Index.Accept(this);

            string targetType = _GetType(n.Target);
            string elemType   = _CollectionElementType(targetType);
            _SetType(n, elemType);
        }

        public override void VisitAssignExpr(AssignExprNode n) {
            n.Target.Accept(this);
            n.Value.Accept(this);

            string targetType = _GetType(n.Target);
            string valueType  = _GetType(n.Value);

            if (!_IsAssignable(targetType, valueType))
                _Error("Cannot assign '%s' to '%s'"
                       .printf(valueType, targetType), n);

            _SetType(n, targetType);
        }

        public override void VisitNewExpr(NewExprNode n) {
            string typeKey = _TypeKey(n.ObjectType);
            foreach (var arg in n.Arguments) arg.Accept(this);
            foreach (var kv in n.NamedArgs.entries) kv.value.Accept(this);

            // Validate constructor arity if the class is known
            var sym = _table.LookupGlobal(
                typeKey.has_suffix("?")
                ? typeKey.substring(0, typeKey.length - 1)
                : typeKey);

            if (sym != null && sym.DeclNode != null)
                _CheckConstructorArity(sym.DeclNode, n);

            _SetType(n, typeKey);
        }

        public override void VisitLambdaExpr(LambdaExprNode n) {
            
            foreach (var p in n.Params) p.Accept(this);
            n.Body.Accept(this);
            string bodyType = _GetType(n.Body);
            

            // Build a rough Func<…,ReturnType> key
            var sb = new StringBuilder("Func<");
            foreach (var p in n.Params) {
                sb.append(_TypeKey(p.ParamType));
                sb.append(", ");
            }
            sb.append(bodyType);
            sb.append(">");
            _SetType(n, sb.str);
        }

        public override void VisitAwaitExpr(AwaitExprNode n) {
            n.Expression.Accept(this);
            string innerType = _GetType(n.Expression);
            // Unwrap Task<T> → T
            string resultType = _UnwrapTask(innerType);
            _SetType(n, resultType);
        }

        public override void VisitWithExpr(WithExprNode n) {
            n.Source.Accept(this);
            string srcType = _GetType(n.Source);

            foreach (var kv in n.Changes.entries) {
                kv.value.Accept(this);
                // Validate field exists and type matches
                if (srcType != "?") {
                    var classSym = _table.LookupGlobal(srcType);
                    if (classSym != null && classSym.DeclNode != null) {
                        string memberType = _ResolveMemberType(
                            classSym.DeclNode, kv.key, n);
                        string valType = _GetType(kv.value);
                        if (!_IsAssignable(memberType, valType))
                            _Error("'with' field '%s': cannot assign '%s' to '%s'"
                                   .printf(kv.key, valType, memberType),
                                   kv.value);
                    }
                }
            }

            _SetType(n, srcType);
        }

        public override void VisitListLiteral(ListLiteralNode n) {
            if (n.IsComprehension) {
                
                if (n.CompSource != null) n.CompSource.Accept(this);
                if (n.CompFilter != null) {
                    n.CompFilter.Accept(this);
                    _CheckBool(n.CompFilter, "list comprehension filter");
                }
                if (n.CompExpr != null)   n.CompExpr.Accept(this);
                string elemType = (n.CompExpr != null)
                                  ? _GetType(n.CompExpr) : "?";
                
                _SetType(n, "List<%s>".printf(elemType));
            } else {
                string elemType = "?";
                foreach (var el in n.Elements) {
                    el.Accept(this);
                    if (elemType == "?") elemType = _GetType(el);
                }
                _SetType(n, "List<%s>".printf(elemType));
            }
        }

        public override void VisitMapLiteral(MapLiteralNode n) {
            string kt = "?";
            string vt = "?";
            foreach (var entry in n.Entries) {
                entry.Accept(this);
                if (kt == "?") kt = _GetType(entry.Key);
                if (vt == "?") vt = _GetType(entry.Value);
            }
            _SetType(n, "Map<%s, %s>".printf(kt, vt));
        }

        public override void VisitMapEntry(MapEntryNode n) {
            n.Key.Accept(this);
            n.Value.Accept(this);
        }

        public override void VisitIdentifier(IdentifierNode n) {
            var sym = _table.Lookup(n.Name);
            _SetType(n, sym != null ? sym.TypeKey : "?");
        }

        public override void VisitThis(ThisNode n) {
            _SetType(n, _currentClass ?? "?");
        }

        public override void VisitNull(NullNode n) {
            _SetType(n, "null");
        }

        public override void VisitLiteral(LiteralNode n) {
            switch (n.Kind) {
                case LiteralKind.INTEGER:
                    _SetType(n, "int");
                    break;
                case LiteralKind.FLOAT:
                    _SetType(n, "float");
                    break;
                case LiteralKind.STRING:
                    _SetType(n, "string");
                    break;
                case LiteralKind.BOOL:
                    _SetType(n, "bool");
                    break;
                case LiteralKind.INTERPOLATED_STRING:
                    if (n.Segments != null)
                        foreach (var seg in n.Segments)
                            seg.Accept(this);
                    _SetType(n, "string");
                    break;
            }
        }
        // ═══════════════════════════════════════════════
        //  Visitor — Types (no-ops at this pass)
        // ═══════════════════════════════════════════════

        public override void VisitSimpleType  (SimpleTypeNode  n) {}
        public override void VisitGenericType (GenericTypeNode n) {}
        public override void VisitFuncType    (FuncTypeNode    n) {}
        public override void VisitTupleType   (TupleTypeNode   n) {}
        // ═══════════════════════════════════════════════
        //  Private helpers — type resolution
        // ═══════════════════════════════════════════════

        /**
         * Convert a TypeNode to a string TypeKey.
         */
        private string _TypeKey(TypeNode type) {
            if (type is SimpleTypeNode) {
                var st = (SimpleTypeNode) type;
                return st.IsNullable ? st.Name + "?" : st.Name;
            }
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
                if (gt.IsNullable) sb.append("?");
                return sb.str;
            }
            if (type is TupleTypeNode) {
                var tt  = (TupleTypeNode) type;
                var sb  = new StringBuilder("(");
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
                var sb = new StringBuilder("Func<");
                foreach (var pt in ft.ParamTypes) {
                    sb.append(_TypeKey(pt));
                    sb.append(", ");
                }
                sb.append(_TypeKey(ft.ReturnType));
                sb.append(">");
                return sb.str;
            }
            return "?";
        }

        /**
         * Extract the element type from a collection TypeKey.
         *   "List<Player>"  → "Player"
         *   "Map<string,int>" → "int"   (value type)
         *   "int[]"         → "int"
         *   "?"             → "?"
         */
        private string _CollectionElementType(string typeKey) {
            if (typeKey == "?" || typeKey == "") return "?";

            // List<T> → T
            if (typeKey.has_prefix("List<") && typeKey.has_suffix(">"))
                return typeKey.substring(5, typeKey.length - 6);

            // Set<T> → T
            if (typeKey.has_prefix("Set<") && typeKey.has_suffix(">"))
                return typeKey.substring(4, typeKey.length - 5);

            // Map<K,V> → V (value type)
            if (typeKey.has_prefix("Map<") && typeKey.has_suffix(">")) {
                string inner = typeKey.substring(4, typeKey.length - 5);
                int comma    = inner.index_of(",");
                if (comma >= 0)
                    return inner.substring(comma + 1).strip();
            }

            // T[] → T
            if (typeKey.has_suffix("[]"))
                return typeKey.substring(0, typeKey.length - 2);

            return "?";
        }

        /**
         * Unwrap a Task<T> TypeKey to T.
         *   "Task<string>" → "string"
         *   "Task"         → "void"
         *   "string"       → "string"  (not a Task)
         */
        private string _UnwrapTask(string typeKey) {
            if (typeKey == "Task") return "void";
            if (typeKey.has_prefix("Task<") && typeKey.has_suffix(">"))
                return typeKey.substring(5, typeKey.length - 6);
            return typeKey;
        }

        /**
         * The base return type, unwrapping Task<T> for async methods.
         */
        private string _BaseReturnType() {
            if (_inAsync) return _UnwrapTask(_currentReturnType);
            return _currentReturnType;
        }

        /**
         * Resolve the type of member `memberName` within the class
         * represented by `classDecl`.
         *
         * Walks the class members list looking for a FieldDecl,
         * PropertyDecl, or MethodDecl with that name.
         * Returns "?" when not found (and emits an error).
         */
        private string _ResolveMemberType(AstNode  classDecl,
                                          string   memberName,
                                          AstNode  refNode) {
            Gee.ArrayList<AstNode>? members = null;
            string className = "?";
            TypeNode? baseClass = null;

            // Enum member access: Direction.North → type is "Direction"
            if (classDecl is EnumDeclNode) {
                var en = (EnumDeclNode) classDecl;
                foreach (var m in en.Members)
                    if (m.Name == memberName)
                        return en.Name;   // type of Direction.North is "Direction"
                foreach (var m in en.Methods)
                    if (m.Name == memberName)
                        return (m.ReturnType != null)
                               ? _TypeKey(m.ReturnType) : "void";
                _Error("Enum '%s' has no member '%s'"
                       .printf(en.Name, memberName), refNode);
                return "?";
            }

            if (classDecl is ClassDeclNode) {
                var cls = (ClassDeclNode) classDecl;
                members   = cls.Members;
                className = cls.Name;
                baseClass = cls.BaseClass;
            } else if (classDecl is DataClassDeclNode) {
                members   = ((DataClassDeclNode) classDecl).Members;
                className = ((DataClassDeclNode) classDecl).Name;
            } else if (classDecl is RecordDeclNode) {
                var rec = (RecordDeclNode) classDecl;
                className = rec.Name;
                foreach (var rp in rec.Params)
                    if (rp.Name == memberName)
                        return _TypeKey(rp.ParamType);
                foreach (var m in rec.Methods)
                    if (m.Name == memberName)
                        return (m.ReturnType != null)
                               ? _TypeKey(m.ReturnType) : "void";
                _Error("'%s' has no member '%s'"
                       .printf(className, memberName), refNode);
                return "?";
            }

            if (members == null) return "?";

            foreach (var m in members) {
                if (m is FieldDeclNode) {
                    var f = (FieldDeclNode) m;
                    if (f.Name == memberName) return _TypeKey(f.FieldType);
                } else if (m is PropertyDeclNode) {
                    var p = (PropertyDeclNode) m;
                    if (p.Name == memberName) return _TypeKey(p.PropType);
                } else if (m is MethodDeclNode) {
                    var md = (MethodDeclNode) m;
                    if (md.Name == memberName)
                        return (md.ReturnType != null)
                               ? _TypeKey(md.ReturnType) : "void";
                } else if (m is ConstructorDeclNode) {
                    if (memberName == className) return className;
                }
            }

            // Also check data class params
            if (classDecl is DataClassDeclNode) {
                var dc = (DataClassDeclNode) classDecl;
                foreach (var rp in dc.Params)
                    if (rp.Name == memberName)
                        return _TypeKey(rp.ParamType);
            }

            // Walk parent class chain for inherited members
            if (baseClass != null) {
                string parentName = _TypeKey(baseClass);
                var parentSym = _table.LookupGlobal(parentName);
                if (parentSym != null && parentSym.DeclNode != null)
                    return _ResolveMemberType(parentSym.DeclNode,
                                              memberName, refNode);
            }

            _Error("'%s' has no member '%s'"
                   .printf(className, memberName), refNode);
            return "?";
        }

        /**
         * Validate that the number of arguments in a call matches
         * the number of parameters declared in the symbol's DeclNode.
         */
        private void _CheckCallArity(Symbol sym, CallExprNode call) {
            if (sym.DeclNode == null) return;

            int declared = 0;
            int defaults = 0;

            if (sym.DeclNode is MethodDeclNode) {
                var m = (MethodDeclNode) sym.DeclNode;
                declared = m.Params.size;
                foreach (var p in m.Params)
                    if (p.Default != null) defaults++;
            } else {
                return; // not a method — skip arity check
            }

            int provided = call.Arguments.size + call.NamedArgs.size;
            int minArgs  = declared - defaults;

            if (provided < minArgs || provided > declared)
                _Error("Method '%s' expects %d-%d arguments, got %d"
                       .printf(sym.Name, minArgs, declared, provided),
                       call);
        }

        /**
         * Validate constructor call argument count.
         */
        private void _CheckConstructorArity(AstNode classDecl,
                                             NewExprNode call) {
            ConstructorDeclNode? ctor = null;

            if (classDecl is ClassDeclNode) {
                foreach (var m in ((ClassDeclNode) classDecl).Members)
                    if (m is ConstructorDeclNode) {
                        ctor = (ConstructorDeclNode) m;
                        break;
                    }
            }

            if (ctor == null) return;  // no explicit constructor — OK

            int declared = ctor.Params.size;
            int defaults = 0;
            foreach (var p in ctor.Params)
                if (p.Default != null) defaults++;

            int provided = call.Arguments.size + call.NamedArgs.size;
            int minArgs  = declared - defaults;

            if (provided < minArgs || provided > declared)
                _Error("Constructor expects %d-%d arguments, got %d"
                       .printf(minArgs, declared, provided), call);
        }

        /**
         * Emit an error if the given node's type is not bool.
         * `context` is a human-readable label used in the message.
         */
        private void _CheckBool(AstNode node, string context) {
            string t = _GetType(node);
            if (!_IsBool(t))
                _Error("'%s' must be bool, got '%s'"
                       .printf(context, t), node);
        }

        private void _Error(string message, AstNode? node) {
            TypeError err;
            if (node != null)
                err = new TypeError.from_node(message, node);
            else
                err = new TypeError(message, _filename, 0, 0);
            _errors.add(err);
        }

        private void _Warn(string message, AstNode node) {
            _Error("[warning] " + message, node);
        }
    }
}
