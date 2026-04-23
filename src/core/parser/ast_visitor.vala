// ═══════════════════════════════════════════════════════
//  ast_visitor.vala  -  Visitor pattern pour l'AST
//
//  Permet de parcourir l'AST sans modifier les nœuds.
//  Utilisé par :
//  → Le générateur C    (produit du code C)
//  → Le type checker    (vérifie les types)
//  → Le resolver        (résout les noms)
//  → Le LSP             (autocomplétion, hover)
//  → Le pretty printer  (debug)
// ═══════════════════════════════════════════════════════

namespace CodeTranspiler.Ast {

    /**
     * Interface visiteur de base.
     * Implémentée par chaque composant qui parcourt l'AST.
     */
    public abstract class AstVisitor : Object {

        // ── Programme ──────────────────────────────────
        public abstract void VisitProgram       (ProgramNode   n);
        public abstract void VisitNamespace     (NamespaceNode n);
        public abstract void VisitImport        (ImportNode    n);

        // ── Déclarations ───────────────────────────────
        public abstract void VisitDecorator     (DecoratorNode       n);
        public abstract void VisitClassDecl     (ClassDeclNode       n);
        public abstract void VisitInterfaceDecl (InterfaceDeclNode   n);
        public abstract void VisitEnumDecl      (EnumDeclNode        n);
        public abstract void VisitEnumMember    (EnumMemberNode      n);
        public abstract void VisitRecordDecl    (RecordDeclNode      n);
        public abstract void VisitDataClassDecl (DataClassDeclNode   n);
        public abstract void VisitRecordParam   (RecordParamNode     n);
        public abstract void VisitFieldDecl     (FieldDeclNode       n);
        public abstract void VisitPropertyDecl  (PropertyDeclNode    n);
        public abstract void VisitMethodDecl    (MethodDeclNode      n);
        public abstract void VisitConstructorDecl(ConstructorDeclNode n);
        public abstract void VisitParam         (ParamNode          n);

        // ── Instructions ───────────────────────────────
        public abstract void VisitBlock         (BlockNode       n);
        public abstract void VisitVarDecl       (VarDeclNode     n);
        public abstract void VisitIf            (IfNode          n);
        public abstract void VisitElseIf        (ElseIfNode      n);
        public abstract void VisitMatch         (MatchNode       n);
        public abstract void VisitMatchArm      (MatchArmNode    n);
        public abstract void VisitMatchPattern  (MatchPatternNode n);
        public abstract void VisitWhile         (WhileNode       n);
        public abstract void VisitFor           (ForNode         n);
        public abstract void VisitForeach       (ForeachNode     n);
        public abstract void VisitReturn        (ReturnNode      n);
        public abstract void VisitGuard         (GuardNode       n);
        public abstract void VisitBreak         (BreakNode       n);
        public abstract void VisitContinue      (ContinueNode    n);
        public abstract void VisitTryCatch      (TryCatchNode    n);
        public abstract void VisitGoStmt        (GoStmtNode      n);

        // ── Expressions ────────────────────────────────
        public abstract void VisitBinaryExpr    (BinaryExprNode  n);
        public abstract void VisitUnaryExpr     (UnaryExprNode   n);
        public abstract void VisitMemberAccess  (MemberAccessNode n);
        public abstract void VisitCallExpr      (CallExprNode    n);
        public abstract void VisitIndexExpr     (IndexExprNode   n);
        public abstract void VisitAssignExpr    (AssignExprNode  n);
        public abstract void VisitNewExpr       (NewExprNode     n);
        public abstract void VisitLambdaExpr    (LambdaExprNode  n);
        public abstract void VisitAwaitExpr     (AwaitExprNode   n);
        public abstract void VisitWithExpr      (WithExprNode    n);
        public abstract void VisitListLiteral   (ListLiteralNode n);
        public abstract void VisitMapLiteral    (MapLiteralNode  n);
        public abstract void VisitMapEntry      (MapEntryNode    n);
        public abstract void VisitIdentifier    (IdentifierNode  n);
        public abstract void VisitThis          (ThisNode        n);
        public abstract void VisitNull          (NullNode        n);
        public abstract void VisitLiteral       (LiteralNode     n);

        // ── Types ──────────────────────────────────────
        public abstract void VisitSimpleType    (SimpleTypeNode  n);
        public abstract void VisitGenericType   (GenericTypeNode n);
        public abstract void VisitFuncType      (FuncTypeNode    n);
        public abstract void VisitTupleType     (TupleTypeNode   n);
    }


    /**
     * Visiteur de base avec implémentations vides.
     * Hériter de cette classe pour n'override
     * que les nœuds qui nous intéressent.
     */
    public abstract class BaseAstVisitor : AstVisitor {

        public override void VisitProgram        (ProgramNode    n) {}
        public override void VisitNamespace      (NamespaceNode  n) {}
        public override void VisitImport         (ImportNode     n) {}
        public override void VisitDecorator      (DecoratorNode  n) {}
        public override void VisitClassDecl      (ClassDeclNode  n) {}
        public override void VisitInterfaceDecl  (InterfaceDeclNode n) {}
        public override void VisitEnumDecl       (EnumDeclNode   n) {}
        public override void VisitEnumMember     (EnumMemberNode n) {}
        public override void VisitRecordDecl     (RecordDeclNode n) {}
        public override void VisitDataClassDecl  (DataClassDeclNode n) {}
        public override void VisitRecordParam    (RecordParamNode n) {}
        public override void VisitFieldDecl      (FieldDeclNode  n) {}
        public override void VisitPropertyDecl   (PropertyDeclNode n) {}
        public override void VisitMethodDecl     (MethodDeclNode n) {}
        public override void VisitConstructorDecl(ConstructorDeclNode n) {}
        public override void VisitParam          (ParamNode      n) {}
        public override void VisitBlock          (BlockNode      n) {}
        public override void VisitVarDecl        (VarDeclNode    n) {}
        public override void VisitIf             (IfNode         n) {}
        public override void VisitElseIf         (ElseIfNode     n) {}
        public override void VisitMatch          (MatchNode      n) {}
        public override void VisitMatchArm       (MatchArmNode   n) {}
        public override void VisitMatchPattern   (MatchPatternNode n) {}
        public override void VisitWhile          (WhileNode      n) {}
        public override void VisitFor            (ForNode        n) {}
        public override void VisitForeach        (ForeachNode    n) {}
        public override void VisitReturn         (ReturnNode     n) {}
        public override void VisitGuard          (GuardNode      n) {}
        public override void VisitBreak          (BreakNode      n) {}
        public override void VisitContinue       (ContinueNode   n) {}
        public override void VisitTryCatch       (TryCatchNode   n) {}
        public override void VisitGoStmt         (GoStmtNode     n) {}
        public override void VisitBinaryExpr     (BinaryExprNode n) {}
        public override void VisitUnaryExpr      (UnaryExprNode  n) {}
        public override void VisitMemberAccess   (MemberAccessNode n) {}
        public override void VisitCallExpr       (CallExprNode   n) {}
        public override void VisitIndexExpr      (IndexExprNode  n) {}
        public override void VisitAssignExpr     (AssignExprNode n) {}
        public override void VisitNewExpr        (NewExprNode    n) {}
        public override void VisitLambdaExpr     (LambdaExprNode n) {}
        public override void VisitAwaitExpr      (AwaitExprNode  n) {}
        public override void VisitWithExpr       (WithExprNode   n) {}
        public override void VisitListLiteral    (ListLiteralNode n) {}
        public override void VisitMapLiteral     (MapLiteralNode n) {}
        public override void VisitMapEntry       (MapEntryNode   n) {}
        public override void VisitIdentifier     (IdentifierNode n) {}
        public override void VisitThis           (ThisNode       n) {}
        public override void VisitNull           (NullNode       n) {}
        public override void VisitLiteral        (LiteralNode    n) {}
        public override void VisitSimpleType     (SimpleTypeNode n) {}
        public override void VisitGenericType    (GenericTypeNode n) {}
        public override void VisitFuncType       (FuncTypeNode   n) {}
        public override void VisitTupleType      (TupleTypeNode  n) {}
    }
}
