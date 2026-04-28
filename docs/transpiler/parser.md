# Parser CODE

> **Version** : 0.1.0
> **Fichier**  : `src/core/parser/parser.vala`
> **Statut**   : ✅ Complet et fonctionnel

---

## Rôle

Transforme une liste de tokens en AST.

[Tokens]  →  [ Parser ]  →  AST


---

## Technique : Recursive Descent

Chaque règle EBNF = une méthode Parse*().

```vala
// Exemple : ParseIf() correspond à la règle
// IfStmt = "if" "(" Expr ")" Block
//          { "else" "if" "(" Expr ")" Block }
//          [ "else" Block ]

private IfNode ParseIf() {
    var tok = Expect(KW_IF);
    Expect(LPAREN);
    var condition = ParseExpression();
    Expect(RPAREN);
    var thenBlock = ParseBlock();
    // ...
}
