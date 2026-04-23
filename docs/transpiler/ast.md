# AST - Abstract Syntax Tree

> **Version** : 0.1.0
> **Fichiers** :
>   - `src/core/parser/ast.vala`
>   - `src/core/parser/ast_visitor.vala`
>   - `src/core/parser/ast_printer.vala`
> **Statut**   : ✅ Implémenté

---

## Hiérarchie des Nœuds

```
AstNode (abstract)
│
├── Programme
│   ├── ProgramNode
│   ├── NamespaceNode
│   └── ImportNode
│
├── Déclarations
│   ├── DecoratorNode
│   ├── ClassDeclNode
│   ├── InterfaceDeclNode
│   ├── EnumDeclNode / EnumMemberNode
│   ├── RecordDeclNode
│   ├── DataClassDeclNode
│   ├── FieldDeclNode
│   ├── PropertyDeclNode
│   ├── MethodDeclNode
│   ├── ConstructorDeclNode
│   └── ParamNode
│
├── Instructions
│   ├── BlockNode
│   ├── VarDeclNode
│   ├── IfNode / ElseIfNode
│   ├── MatchNode / MatchArmNode / MatchPatternNode
│   ├── WhileNode
│   ├── ForNode / ForeachNode
│   ├── ReturnNode
│   ├── GuardNode
│   ├── BreakNode / ContinueNode
│   ├── TryCatchNode
│   └── GoStmtNode
│
├── Expressions
│   ├── BinaryExprNode
│   ├── UnaryExprNode
│   ├── MemberAccessNode
│   ├── CallExprNode
│   ├── IndexExprNode
│   ├── AssignExprNode
│   ├── NewExprNode
│   ├── LambdaExprNode
│   ├── AwaitExprNode
│   ├── WithExprNode
│   ├── ListLiteralNode
│   ├── MapLiteralNode
│   ├── IdentifierNode
│   ├── ThisNode
│   ├── NullNode
│   └── LiteralNode
│
└── Types
    ├── SimpleTypeNode
    ├── GenericTypeNode
    ├── FuncTypeNode
    └── TupleTypeNode
```

---

## Visitor Pattern

Utiliser `BaseAstVisitor` et n'override
que les nœuds nécessaires :

```vala
public class MyVisitor : BaseAstVisitor {

    public override void VisitClassDecl(ClassDeclNode n) {
        stdout.printf("Found class: %s\n", n.Name);
        // Visiter les membres
        foreach (var m in n.Members) {
            m.Accept(this);
        }
    }
}
```

---

## Debug

```vala
var printer = new AstPrinter();
stdout.printf(printer.Print(ast));
```
