# Grammaire Formelle de CODE

> **Version** : 0.1.0
> **Format**  : EBNF (Extended Backus-Naur Form)
> **Fichier** : `docs/language/grammar.ebnf`
> **Statut**  : ✅ Version initiale

---

## Points Clés

| Fonctionnalité        | Règle EBNF          |
|-----------------------|---------------------|
| Classes               | `ClassDecl`         |
| Interfaces            | `InterfaceDecl`     |
| Enums riches          | `EnumDecl`          |
| Records / Data class  | `RecordDecl`        |
| Traits                | `TraitDecl`         |
| Pattern matching      | `MatchStmt`         |
| Guard clauses         | `GuardStmt`         |
| Pipeline `\|>`        | `PipelineExpr`      |
| Null safety `?.` `??` | `NullSafeAccess`    |
| Décorateurs `@`       | `Decorator`         |
| Async / Await         | `AwaitExpr`         |
| Génériques            | `GenericParams`     |
| Lambdas               | `LambdaExpr`        |
| Interpolation string  | `Interpolation`     |
| Compréhensions liste  | `ListLiteral`       |
