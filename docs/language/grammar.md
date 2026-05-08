# Grammaire formelle d'Amalgame

> **Version** : 0.3.6
> **Format**  : EBNF (Extended Backus-Naur Form)
> **Fichier** : [`docs/language/grammar.ebnf`](grammar.ebnf)
> **Statut**  : ✅ Synchronisée avec `src/parser/parser.am`

La grammaire EBNF du fichier `grammar.ebnf` reflète exactement
le parser self-hosted ; toute construction qu'il accepte y est
décrite, et toute production absente n'est pas reconnue par
le compilateur actuel.

---

## Sommaire des constructions

| Catégorie         | Construction                            | Règle EBNF              |
|-------------------|-----------------------------------------|-------------------------|
| **Programme**     | namespace, imports, top-level decls     | `Program`               |
| **Types**         | `class`, `data class` / `record`, `enum` algébrique, `interface` | `ClassDecl`, `DataClassDecl`, `EnumDecl`, `InterfaceDecl` |
| **Annotations**   | `T?`, `T[]`, `List<T>`, tuples `(T, U)` | `Type`                  |
| **Variables**     | `let` / `var`, destructuration tuple    | `VarDecl`               |
| **Contrôle**      | `if/else`, `while`, `for x in iter`, `guard … else`, `break`, `continue` | `IfStmt`, `WhileStmt`, `ForInStmt`, `GuardStmt` |
| **Pattern matching** | `match`, gardes `if cond`, variantes algébriques `Some(x)`, ranges `1..5`, wildcard `_` | `MatchStmt`, `Pattern` |
| **Exceptions**    | `try` / `catch` / `finally`, `throw`    | `TryStmt`, `ThrowStmt`  |
| **Expressions**   | Pratt-style precedence climbing (15 niveaux) | `Expression`       |
| **Lambdas**       | single ou multi-param, corps expression ou bloc (`x => expr`, `(x,y) => x+y`, `x => { … }`) | `Lambda`, `LambdaBody`, `GenericParams` |
| **Closures**      | capture par valeur des locaux englobants (v0.3.4 single-param, v0.3.5 multi-param + bloc) | (sémantique, pas grammaticale) |
| **Generic interfaces** | `interface IComparable<T>` + check `implements I<args>` (v0.3.5) | `InterfaceDecl`, `GenericParams` |
| **Member access** | `.`, null-safe `?.`, indexation `[i]`, appel `()` | `PostfixOp`     |
| **Strings**       | `"..."` avec escapes `\n \t \r \xHH \uHHHH`, `"""..."""` raw, interpolation `"hi {x}"` | `StringLit`, `StringInterp` |
| **Décorateurs**   | `@inline`, `@deprecated` (forme drapeau) | `Decorator`            |
| **Spécialités**   | List comprehension `[x*2 for x in xs if x>0]`, pipeline `\|>`, range `..` | `ListComp`, `Range` |
| **Génériques**    | classes/méthodes/`new`, inférence v0.3.3 | `GenericParams`, `GenericArgs` |

## Différences avec la grammaire v0.1.0

La version v0.1.0 décrivait un langage plus large (encore appelé
*CODE*) qui visait `async`/`await`, properties, traits, foreach
C-style, etc. La v0.3.5 capture **le sous-ensemble réellement
implémenté** par le parser self-hosted, plus les ajouts arrivés
entre-temps :

**Présent dans v0.3.5, absent de v0.1.0 :**
- `try` / `catch` / `throw` / `finally`
- `guard cond else { … }`
- `match` comme expression (pas seulement statement)
- Lambdas multi-param et à corps de bloc (capturantes depuis v0.3.4 single-param ; multi-param + bloc depuis v0.3.5)
- List comprehensions `[expr for x in iter if cond]`
- Null-safe `obj?.field` et coalescence `a ?? b`
- `for x in collection` (boucle d'itération)
- Destructuration tuple `let (a, b) = expr`
- Arguments nommés `f(name: value)`
- `data class` / `record` à constructeur primaire
- Méthodes à corps d'expression `=> expr`
- Escapes `\xHH` et `\uHHHH` dans les strings
- Strings triple-quoted `"""..."""`
- Variantes algébriques avec payloads `Some(int)`
- Décorateurs `@inline`
- Génériques `<T>` sur classes / méthodes / `new`
- Compound assigns complets (`+=` à `>>=`)
- Bitwise (`& | ^ ~ << >>`)

**Présent dans v0.1.0, absent de v0.3.5** (réservé pour plus tard) :
- Properties avec `get` / `set` accessors
- Constructeurs explicites (`init(...) { ... }`)
- `async` / `await` / coroutines
- `with { … }` expression
- Traits au-delà des interfaces
- Modificateurs `protected`, `internal`, `abstract`, `override`, `weak`, `pure`
- Function types nommés `(int) -> string` (les lambdas
  en argument avec signatures non-int sont prévues pour v2.5)
- Spread `...args`
- `foreach` C-style
- Mot-clé `func` pour fonctions top-level (token réservé, parser ne l'utilise pas)

Voir [`ROADMAP_COMPLET.md`](../../ROADMAP_COMPLET.md) pour le suivi
de ces évolutions.
