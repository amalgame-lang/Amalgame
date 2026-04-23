// ═══════════════════════════════════════════════════════
//  ast_printer.vala  -  Affichage de l'AST pour debug
// ═══════════════════════════════════════════════════════

namespace CodeTranspiler.Ast {

    /**
     * Affiche l'AST sous forme d'arbre textuel.
     *
     * Exemple de sortie :
     *   Program
     *   ├── Namespace: GuildApp
     *   ├── Import: Code.IO
     *   └── ClassDecl: Player [public]
     *       ├── FieldDecl: Name : string [public]
     *       ├── FieldDecl: Health : int = 100 [public]
     *       └── MethodDecl: TakeDamage [public]
     *           ├── Param: amount : int
     *           └── Block
     *               └── AssignExpr: -=
     *                   ├── MemberAccess: this.Health
     *                   └── Identifier: amount
     */
    public class AstPrinter : BaseAstVisitor {

        private StringBuilder _sb;
        private string        _indent;
        private bool          _isLast;

        public AstPrinter() {
            _sb     = new StringBuilder();
            _indent = "";
            _isLast = true;
        }

        public string Print(AstNode node) {
            _sb.truncate(0);
            node.Accept(this);
            return _sb.str;
        }

        private void Write(string text) {
            _sb.append(_indent);
            _sb.append(_isLast ? "└── " : "├── ");
            _sb.append(text);
            _sb.append("\n");
        }

        private void WithChild(bool isLast, AstNode child) {
            string prevIndent = _indent;
            bool   prevLast   = _isLast;

            _indent  = _indent + (prevLast ? "    " : "│   ");
            _isLast  = isLast;

            child.Accept(this);

            _indent = prevIndent;
            _isLast = prevLast;
        }

        private void WithChildren(Gee.ArrayList<AstNode> children) {
            for (int i = 0; i < children.size; i++) {
                WithChild(i == children.size - 1, children[i]);
            }
        }

        public override void VisitProgram(ProgramNode n) {
            _sb.append("Program\n");
            _isLast = false;
            n.Namespace.Accept(this);

            for (int i = 0; i < n.Imports.size; i++) {
                _isLast = (i == n.Imports.size - 1)
                           && n.Declarations.size == 0;
                n.Imports[i].Accept(this);
            }

            for (int i = 0; i < n.Declarations.size; i++) {
                _isLast = (i == n.Declarations.size - 1);
                n.Declarations[i].Accept(this);
            }
        }

        public override void VisitNamespace(NamespaceNode n) {
            Write("Namespace: %s".printf(n.Name));
        }

        public override void VisitImport(ImportNode n) {
            string alias = n.Alias != null
                           ? " as %s".printf(n.Alias) : "";
            Write("Import: %s%s".printf(n.Name, alias));
        }

        public override void VisitClassDecl(ClassDeclNode n) {
            string mods = n.IsAbstract ? " [abstract]" : "";
            Write("ClassDecl: %s [%s]%s"
                  .printf(n.Name,
                           AccessStr(n.Access),
                           mods));
            WithChildren(n.Members);
        }

        public override void VisitFieldDecl(FieldDeclNode n) {
            Write("FieldDecl: %s : %s [%s]"
                  .printf(n.Name,
                           TypeStr(n.FieldType),
                           AccessStr(n.Access)));
        }

        public override void VisitMethodDecl(MethodDeclNode n) {
            string mods = "";
            if (n.IsStatic)   mods += " static";
            if (n.IsAsync)    mods += " async";
            if (n.IsPure)     mods += " pure";
            if (n.IsOverride) mods += " override";

            Write("MethodDecl: %s [%s]%s"
                  .printf(n.Name, AccessStr(n.Access), mods));

            // Paramètres
            foreach (var p in n.Params) {
                bool last = (p == n.Params.last())
                            && n.Body == null;
                WithChild(last, p);
            }

            // Corps
            if (n.Body != null) {
                bool prevLast = _isLast;
                _isLast = true;
                n.Body.Accept(this);
                _isLast = prevLast;
            }
        }

        public override void VisitParam(ParamNode n) {
            string def = n.Default != null ? " = ..." : "";
            Write("Param: %s : %s%s"
                  .printf(n.Name, TypeStr(n.ParamType), def));
        }

        public override void VisitBlock(BlockNode n) {
            Write("Block");
            WithChildren(n.Statements);
        }

        public override void VisitVarDecl(VarDeclNode n) {
            string kw   = n.IsLet ? "let" : "var";
            string type = n.VarType != null
                          ? " : " + TypeStr(n.VarType) : "";
            string mem  = n.MemoryHint != null
                          ? " @" + n.MemoryHint : "";
            Write("VarDecl: %s %s%s%s"
                  .printf(kw, n.Name, type, mem));
        }

        public override void VisitIf(IfNode n) {
            Write("If");
            WithChild(false, n.Condition);
            WithChild(n.ElseIfs.size == 0 && n.ElseBlock == null,
                      n.ThenBlock);
        }

        public override void VisitMatch(MatchNode n) {
            Write("Match");
            WithChild(false, n.Subject);
            for (int i = 0; i < n.Arms.size; i++) {
                WithChild(i == n.Arms.size - 1, n.Arms[i]);
            }
        }

        public override void VisitMatchArm(MatchArmNode n) {
            Write("MatchArm");
            WithChild(false, n.Pattern);
            WithChild(true,  n.Body);
        }

        public override void VisitBinaryExpr(BinaryExprNode n) {
            Write("BinaryExpr: %s".printf(n.Operator));
            WithChild(false, n.Left);
            WithChild(true,  n.Right);
        }

        public override void VisitMemberAccess(MemberAccessNode n) {
            string safe = n.IsNullSafe ? "?." : ".";
            Write("MemberAccess: %s%s".printf(safe, n.MemberName));
            WithChild(true, n.Target);
        }

        public override void VisitCallExpr(CallExprNode n) {
            Write("CallExpr (%d args)".printf(n.Arguments.size));
            WithChild(n.Arguments.size == 0, n.Callee);
            for (int i = 0; i < n.Arguments.size; i++) {
                WithChild(i == n.Arguments.size - 1,
                          n.Arguments[i]);
            }
        }

        public override void VisitIdentifier(IdentifierNode n) {
            Write("Identifier: %s".printf(n.Name));
        }

        public override void VisitLiteral(LiteralNode n) {
            Write("Literal: %s (%s)"
                  .printf(n.Raw, n.Kind.to_string()));
        }

        public override void VisitReturn(ReturnNode n) {
            Write("Return");
            if (n.Value != null) WithChild(true, n.Value);
        }

        public override void VisitNewExpr(NewExprNode n) {
            Write("NewExpr: %s".printf(TypeStr(n.ObjectType)));
            for (int i = 0; i < n.Arguments.size; i++) {
                WithChild(i == n.Arguments.size - 1,
                          n.Arguments[i]);
            }
        }

        // ── Helpers ────────────────────────────────────
        private string AccessStr(AccessModifier a) {
            switch (a) {
                case AccessModifier.PUBLIC:    return "public";
                case AccessModifier.PRIVATE:   return "private";
                case AccessModifier.PROTECTED: return "protected";
                case AccessModifier.INTERNAL:  return "internal";
                default:                       return "internal";
            }
        }

        private string TypeStr(TypeNode? t) {
            if (t == null) return "?";
            if (t is SimpleTypeNode) {
                var s = (SimpleTypeNode) t;
                return s.IsNullable ? s.Name + "?" : s.Name;
            }
            if (t is GenericTypeNode) {
                var g = (GenericTypeNode) t;
                return "%s<...>".printf(g.Name);
            }
            return "TypeNode";
        }
    }
}
