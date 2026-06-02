# 7 · Fonctionnement interne du compilateur

Bienvenue, contributeur. Ce chapitre est la référence d'architecture pour
les personnes qui travaillent sur `amc` lui-même. Le compilateur est un
pipeline linéaire unique : source → lexer → parser → resolver → typechecker → cgen → C.

Si vous êtes ici parce que les tests ont cassé après votre modification, allez
directement aux recettes « Ajouter une fonctionnalité » vers la fin.

## Structure du pipeline

```
foo.am ─▶ Lexer ─▶ tokens ─▶ Parser ─▶ AST ─▶ Resolver ─▶ TypeChecker ─▶ CGen ─▶ foo.c
                                       │           │            │           │
                                       │           │            │           └─ src/generator/c_gen.am
                                       │           │            └────────────  src/typechecker.am
                                       │           └─────────────────────────  src/resolver/{symbol,resolver}.am
                                       └─────────────────────────────────────  src/parser/{ast,parser}.am
                                                                               src/lexer/{token,lexer}.am
```

`src/main.am` (`AmalgameCompiler.Run`) orchestre le pipeline. Chaque
phase est une classe indépendante dans son propre fichier.

## Lexer (`src/lexer/`)

- `token.am` définit `enum TokenType` (~130 variantes : mots-clés,
  ponctuation, opérateurs) et la classe `Token`.
- `lexer.am` parcourt la source octet par octet, reconnaissant les identifiants,
  les nombres, les chaînes (dont `\xHH`, `\uHHHH`, `"""`), les commentaires et la
  ponctuation. Les tokens transportent `(Type, Value, Line, Column, Filename)`.

Ajouter un nouveau token :

1. Ajouter la variante à `TokenType` dans `src/lexer/token.am`.
2. La reconnaître dans `lexer.am` — généralement dans le bloc de lecture de
   symboles (`else if (c == "@") { ... }`) ou dans la table des mots-clés
   (`if (word == "guard") { ... }`).

## Parser (`src/parser/`)

- `ast.am` définit `enum NodeKind` (CLASS_DECL, METHOD_DECL, IF_STMT,
  …) et la classe universelle `AstNode` avec les champs :
  - `Kind`, `Name`, `Str`, `Str2`, `Flag`, `Flag2`
  - `Left`, `Right`, `Cond`, `Body`, `Else` (références AstNode nullables)
  - `Children`, `Params`, `Args` (listes d'AstNode)
- `parser.am` est un parser par descente récursive avec une montée de
  précédence à la Pratt pour les expressions.

Chaque construction a une fonction de parsing dédiée (`ParseDecl`,
`ParseClass`, `ParseMethod`, `ParseStmt`, `ParseExpr`, `ParseUnary`,
`ParsePrimary`, `ParsePostfix`, `ParseCallArgs`, `ParseMatch`, …).

Ajouter un nouveau statement (`guard`, par exemple) :

1. Ajouter le token mot-clé dans le lexer.
2. Dans `ParseStmt`, dispatcher sur le mot-clé : `if (v == "guard") { return this.ParseGuard() }`.
3. Implémenter `ParseGuard()` — en construisant généralement un `IF_STMT`
   normal avec une condition transformée (pour que le reste du pipeline n'ait
   pas besoin de connaître la nouvelle construction).

## Resolver (`src/resolver/`)

Deux passes :

1. **CollectDecl** — enregistre chaque type de niveau supérieur (classe, enum) dans
   la portée globale afin que les références en avant fonctionnent ; construit la
   **MemberTable** qui associe `ClassName.MemberName → typeName`.
2. **ResolveDecl** — parcourt l'AST, ouvre/ferme les portées pour les
   méthodes/blocs/for-in/bras de match, enregistre les variables locales lors de
   leur déclaration, et signale `Unknown symbol 'x'` pour les identifiants non
   résolus.

La portée locale est un tableau plat de noms avec une pile d'indices de début
(`ScopeStarts`). `PushScope` enregistre le comptage courant, `PopScope`
tronque les entrées déclarées depuis lors.

Le resolver **possède également la SourceMap** — c'est elle qui alimente les
snippets de style rustc dans les messages d'erreur.

Ajouter un nouveau builtin (par exemple, un nouveau helper runtime `String_*`) :

1. Ajouter la déclaration C dans le bon header de `runtime/`.
2. Dans `src/resolver/resolver.am` — `RegisterBuiltins()` — le déclarer
   en tant que global avec son type de retour :
   ```amalgame
   this.DeclareGlobal("String_DamerauLevenshtein", "int", false)
   ```
3. (Facultatif, mais recommandé) Ajouter une entrée de type de retour à
   `BuiltinCallReturnType()` et `InferTypeFromExpr()` dans
   `src/generator/c_gen.am` afin que l'interpolation et l'inférence de type
   en aient connaissance.

## TypeChecker (`src/typechecker.am`)

- Maintient sa propre pile de portées (`LocalNames`, `LocalTypes`,
  `ScopeStarts`) — le resolver dépile ses portées après la résolution, de
  sorte que le typechecker ne peut pas les réutiliser.
- Pour les expressions, `CheckExpr` alimente une map `(clé-de-nœud → type)`
  via `SetType` / `GetType`.
- `CheckMemberExpr` recherche les types de `obj.Field` via
  `Symbols.GetMemberType(baseType, name)`.
- `CheckVarDecl` vérifie l'assignabilité lorsqu'une annotation de type est
  présente : `let n: int = "hello"` est une erreur de type.

Les erreurs transportent leur snippet source (chargé dans `Sources: SourceMap`
par `main.am`), rendu par `TypeError.ToString()`.

## CGen (`src/generator/c_gen.am`)

Le plus grand fichier unique (~2000 lignes). Deux passes :

- **Passe 1** (`AddFilePass1`) — émet les déclarations anticipées : typedefs
  pour les classes (`typedef struct _Foo Foo;`), déclarations anticipées des enums.
- **Passe 2** (`AddFilePass2`) — émet les corps de structs des classes, les
  déclarations anticipées de méthodes, les corps de méthodes. L'ordre des fichiers
  compte : les dépendants doivent venir APRÈS leurs dépendances dans la liste des
  sources (car le CGen de bootstrap émet déclaration + corps entrelacés par fichier).

L'émission des statements et des expressions est répartie en de nombreuses petites
fonctions (`EmitStmt`, `EmitBlock`, `EmitExprStr`, `EmitMatch`, …). Utilisez
`this.Out.EmitLine` / `Indent_` / `Dedent` pour gérer l'indentation.

L'`Emitter` a un flag `Streaming` — lorsqu'il est activé, `EmitLine` écrit
directement dans un fichier via `File.StreamLine` au lieu d'accumuler dans
une `List<string>`. Utilisé par `gen_test.am`'s `gen6` pour écrire
rapidement `amc_lib.c` de plusieurs Mo.

Ajouter une fonctionnalité dans CGen :

1. Décider de la forme AST — s'agit-il d'un nouveau NodeKind, ou d'un flag sur un
   existant (par exemple, `?.` réutilise MEMBER avec `Flag = true`) ?
2. Ajouter une branche dans `EmitStmt` / `EmitExprStr` pour la nouvelle forme.
3. Si la construction utilise des statements que le bloc parent ne verra pas
   (par exemple, une déclaration de liant devant être dans la portée pour un guard),
   recourir aux expressions composées GCC : `({ stmt; expr; })`.

## Formatter (`src/formatter/formatter.am`)

`amc fmt file.am` réémet le source depuis l'AST. Le formatter parcourt
le même arbre `AstNode` que le reste du compilateur, donc tout ce qui est
exprimable par le parser fait l'aller-retour. Les commentaires survivent parce que
le lexer les émet comme des tokens `COMMENT` (plutôt que de les ignorer comme des
espaces), et le parser les collecte dans `Parser.Comments` sans les placer dans
l'AST. `Formatter.Sync(line)` les réinjecte à leur ligne source d'origine.

Quelques éléments collaborent pour rendre l'aller-retour idempotent :

- `Block.Str2` et `CLASS_DECL.Str2` portent la ligne source du `}` fermant,
  afin que le formatter sache où un bloc se termine et où la chose suivante
  commence (utilisé pour préserver les lignes vides entre blocs).
- `EmitInline` décide si un corps est un statement unique ou un bloc, reflétant
  la flexibilité du parser.
- Les contextes d'expression qui n'ont pas encore de représentation source
  (voir le fallthrough de `EmitExpr`) émettent un placeholder `_TODO_<KIND>` pour
  que le résultat soit encore parsable ; l'idempotence est préservée au détriment
  du sens. C'est censé être temporaire et rare en pratique.

`tests/fmt/fmt_test.am` vérifie l'idempotence + l'équivalence sémantique
sur une petite fixture (`amc test ./tests/fmt/`) ; le balayage de régression
qui exécute `amc fmt` sur chaque source du compilateur doit rester vert.

## Linter (`src/linter.am`)

`amc --lint file.am` exécute une passe d'analyse statique au-dessus de
l'AST parsé et émet des avertissements non fatals. Le Linter ne partage aucun état
avec le typechecker — il parcourt l'AST de haut en bas et collecte des enregistrements
`LintWarning` dans `linter.Warnings`.

Couverture actuelle :

- **Code inaccessible** après `return` / `throw` / `break` /
  `continue`, y compris dans les corps imbriqués `if` / `while` / `for-in` /
  `try`. *(depuis v0.3.3)*
- **Variables locales inutilisées** — un `let` ou `var` dont le nom n'est jamais
  lu dans le reste de la portée. Préfixez le nom avec `_` pour le désactiver
  intentionnellement (reflète le traitement du joker `_` par le resolver).
  *(depuis lint-extensions PR)*
- **Noms masqués** — un liant `let` / `var` / `for-in` réutilise
  un nom visible dans une portée englobante, y compris les paramètres de méthode.
  *(depuis lint-extensions PR)*

Les vérifications d'inutilisation et de masquage s'appuient sur une petite pile de
portées interne au linter (tableaux parallèles `LocalNames` / `ScopeStarts`, même
forme que le resolver). L'utilisation est enregistrée dans une liste
`UsedNames` en ajout seul ; chaque variable locale stocke un instantané de
`UsedNames.Count()` à sa déclaration, pour que `PopScope` puisse répondre à
« ce nom est-il apparu dans `UsedNames` depuis ma déclaration ? » sans avoir
besoin de `List<T>.Set`. `UsedNames` est réinitialisée à une liste vierge à
l'entrée de chaque méthode, afin de garder les indices d'instantanés significatifs
par méthode et de borner la mémoire.

Les paramètres de méthode et de lambda sont suivis dans la portée pour participer
à la détection de masquage, mais ils ne sont jamais signalés comme inutilisés
(`_param` serait peu ergonomique).

Le squelette est conçu pour accueillir de nouvelles vérifications (patterns suspects,
fallthrough implicite dans un match, etc.) en étendant `LintStmt` /
`LintExpr` sans toucher au reste du pipeline. Les avertissements portent toujours le
nom de fichier par programme (renseigné depuis `prog.Str2`) afin que les
invocations multi-fichiers signalent les bons chemins.

Le flag CLI (`--lint`) est câblé dans `main.am`, après la passe typechecker
et avant la génération de code. Les avertissements ne font pas monter
`ExitCode` — `amc --lint -o foo file.am` produit quand même une sortie.

## Lanceur de tests (`amc test`)

`amc test [<dir>]` découvre les `*_test.am` sous `<dir>` (par défaut `.`),
compile + exécute chacun, et agrège les lignes `[PASS] <name>`,
`[FAIL] <name>: <msg>`, et `[SKIP] <name>` depuis la sortie standard de chaque
enfant. Le lanceur se trouve dans `Program.RunTest` dans `main.am` ; la
sous-commande est dispatchée depuis `Program.Main` à côté de la sous-commande `fmt`.

Pipeline par fichier de test :

0. **Pré-vol (une fois par exécution)** — résout `amcRuntime` depuis
   `$AMC_RUNTIME`, sinon `<dirname(amc)>/runtime`, sinon repli sur
   `./runtime`. Puis charge `PackageRegistry` (manifeste + lockfile)
   et, pour chaque package déclarant `[stdlib].sources`,
   `Program.PreCompilePackageSources` compile chaque `.c` une fois
   avec `gcc -O2 -I<amcRuntime> -w -c …` vers un
   `/tmp/amc-pkg-<class>-<leaf>.o` mis en cache. Les chemins `.o` sont injectés
   dans l'étape de linkage par test ci-dessous.
1. **Découverte** — appel shell à `find <dir> -name '*_test.am' -type f`
   via `Process.RunCapture`. Portable sur POSIX et Windows
   MSYS2 (le chemin Windows de la CI).
2. **Compilation vers C** — invoque le binaire `amc` en cours d'exécution sur le
   fichier (chemin depuis `Args_Get(0)`) avec `-o /tmp/amc_test_<idx>` et
   `--quiet`. Émet `<tmp>.c`.
3. **Compilation vers natif** — `gcc -O2 -Iruntime -I'<amcRuntime>'
   <tmp>.c <pkg-objs…> -lgc -lm -ldl -lpthread -o <tmp>`.
   Les fichiers `.o` de package pré-compilés de l'étape 0 sont injectés
   afin que les backends de vendoring (SQLite, DuckDB futur) s'éditent proprement
   sans intervention de l'utilisateur. `-ldl` / `-lpthread` sont inconditionnels —
   sans effet lorsqu'aucun package n'en a besoin, requis par SQLite.
4. **Exécution + analyse** — exécute le binaire de test via `Process.RunCapture`
   et scanne sa sortie standard à la recherche de lignes préfixées par des tags.
   Tout le reste est ignoré. Une sortie non nulle sans ligne de tag est signalée
   comme `[FAIL] <crash> exit=N` afin que les plantages silencieux soient
   quand même enregistrés.

Le lanceur sort avec un code non nul si un cas FAIL ou si un fichier échoue à
compiler ; sinon zéro. La convention reste délibérément sans framework pour v1 —
un module `Assert` plus riche + l'auto-découverte `test_<name>` est un v2
possible.

## Serveur LSP (`amc lsp`, `src/lsp.am`)

`amc lsp` exécute un serveur LSP 3.x minimal communiquant en JSON-RPC 2.0
via stdio avec le cadrage standard `Content-Length: N\r\n\r\n<N octets>`.
La v1 implémente :

- `initialize` / `shutdown` / `exit` — cycle de vie
- `textDocument/didOpen` / `didChange` / `didClose` — état du document,
  annoncé comme synchronisation Full (`textDocumentSync = 1`) de sorte que chaque
  `didChange` transporte le texte entier mis à jour
- `textDocument/publishDiagnostics` — poussé en retour sur chaque
  `did{Open,Change}`. Les diagnostics fusionnent les `RawErrors` du resolver et les
  `Errors` du typechecker, convertis depuis les `(line, column)` en base 1 vers
  la forme `Position` LSP en base 0 et soulignés comme un seul
  caractère à la colonne d'erreur.

Le survol, la complétion et le saut à la définition sont hors périmètre pour la v1
et arriveront dans un suivi.

La gestion JSON est **ad hoc** plutôt qu'un vrai parser : les helpers statiques
`JsonStr(body, key)` et `JsonInt(body, key)` trouvent `"<key>"`, sautent jusqu'à la
valeur et lisent jusqu'au terminateur approprié (gérant les échappements
antislash pour les chaînes). Les messages LSP n'ont pas de clés ambiguës à la
profondeur où on les extrait (`method`, `id`, `uri`, `text`), ce qui échange la
correction sur du JSON arbitraire contre ~50 lignes de code au lieu d'une union
taguée + un parser par descente récursive. Si le survol ou la complétion nécessitent
une extraction plus profonde, promouvoir le codec en un vrai module `stdlib/Json`.

Deux nouveaux helpers runtime ont été livrés pour gérer le cadrage :
`Console_ReadBytes(n)` lit exactement `n` octets depuis stdin (le corps LSP
après parsing de `Content-Length`), et `Console_Flush()` vide le buffer stdout
afin que le client ne soit pas bloqué en attente des réponses bufferisées.

Le resolver a acquis un `RawErrors: List<ResolverError>` parallèle
maintenu en synchronisation avec les `Errors: List<string>` formatées.
`ResolverError` est un petit miroir local de `TypeError` — ils dupliquent des
champs plutôt que de les partager, parce que resolver.am est
compilé avant typechecker.am dans le bundle et qu'on souhaite que le
graphe de dépendances entre fichiers sources reste unidirectionnel.

## Bootstrap par snapshot (`snapshot/`, `tools/save-snapshot.sh`)

`build_amc.sh` dispose d'une chaîne de bootstrap à 2 niveaux :

1. `./amc` — le compilateur auto-hébergé fraîchement construit.
2. `./snapshot/amc` — le dernier amc connu bon, capturé par
   `tools/save-snapshot.sh` après une exécution de tests verte. Le portable
   `snapshot/amc_lib.c` est commité ; le binaire est reconstruit par `gcc`
   sur chaque plateforme qui en a besoin.

Le niveau snapshot existe pour qu'on puisse introduire une nouvelle syntaxe sans
perdre le bootstrap. Si `./amc` casse en cours de développement, `build_amc.sh`
se rabat sur `./snapshot/amc`, qui comprend encore toute la syntaxe livrée au
moment du dernier `tools/save-snapshot.sh`. Depuis un clone vierge, recompiler
`snapshot/amc` depuis le `snapshot/amc_lib.c` tracké avec une seule invocation
`gcc` — voir `snapshot/INFO.md`.

Lorsqu'on introduit une nouvelle syntaxe, prendre un snapshot *avant* d'utiliser
la nouvelle construction dans `src/*.am`. Ainsi, si l'implémentation a une
régression, le snapshot fonctionne encore.

## main.am

Colle :

1. Parse les arguments CLI (`Args.Count`, `Args.Get`).
2. Lit chaque fichier d'entrée, exécute Lexer + Parser (Passe 1 de CGen).
3. Construit le `FullResolver`, alimente tous les programmes, exécute les deux passes.
4. Construit le `TypeChecker`, l'exécute sur le premier programme (le
   typechecking multi-fichiers est partiel aujourd'hui).
5. Exécute la Passe 2 de CGen sur chaque programme.
6. Émet le wrapper final `int main()` sauf si `--lib` ou si aucun
   `Program.Main` n'a été trouvé.

## gen_test.am

`src/generator/gen_test.am` est le « construire le build » — lorsqu'il est exécuté,
il parse chaque fichier `.am` du compilateur, pilote un seul `CGen`
sur tous, et écrit le résultat dans `src/amc_lib.c`. C'est l'artefact
d'auto-hébergement canonique.

Il s'exécute aussi en mode **streaming** (`SetStreaming(true)`), contournant
la liste de lignes en mémoire et écrivant directement vers `File_StreamLine`.

## Tests

- `tests/samples/*.am` — programmes d'entrée (compilés + exécutés par les bundles).
- `tests/<bundle>/*_test.am` — bundles de tests AM pilotés par `amc test` :
  `fmt/` (formatter), `amc_new/` (scaffolder), `stdlib_bundle/` (stdlib),
  `core_bundle/` (lang + LSP + DAP + outillage LLM).
- `tests/fixtures/` — fixtures e2e (caches PM, espace de travail LSP).
- `tests/core_bundle/fixtures/lsp_*.bin` — séquences JSON-RPC LSP précalculées
  (cadrage Content-Length) consommées par les cas LSP.
- `tests/samples/lib_e2e_consumer.c` — le consommateur C pour le test
  de bout en bout `--lib`.
- `tests/run_*.sh` — lanceurs bash legacy, conservés comme filet de sécurité
  pendant la migration vers les bundles (à supprimer après quelques releases stables).

Lorsqu'on ajoute une fonctionnalité :
1. Déposer un sample dans `tests/samples/montest.am` (une fonctionnalité, un observable).
2. Ajouter une assertion dans le bundle approprié, par exemple dans
   `core_bundle/core_test.am` :
   ```amalgame
   g_montest.Add(new CoreCase("ma feature", "sortie attendue"))
   Program.RunGroup("./tests/samples/montest.am", "montest", g_montest)
   ```
3. Vérifier : `./amc test ./tests/core_bundle/`.

Pour les tests d'outillage (LSP, lint, --check, --lib, multi-fichier, externe),
utiliser les helpers spécialisés déjà définis dans `core_test.am` :
`RunLspCheck`, `RunLintCheck`, `RunCheckFail`, `RunCCheck`,
`RunLibTest`, `RunMultiFile`, `RunExternalTest`, `RunCmdGrep`, etc.

## Pièges courants

- **Circularité du bootstrap** — lorsqu'on ajoute un helper runtime ou un
  builtin, le `amc` en cours d'exécution n'en a pas connaissance tant qu'il n'est
  pas reconstruit. `build_amc.sh` tolère une sortie non nulle de l'étape 1's `amc`
  pour que le pipeline puisse quand même produire un binaire fonctionnel à l'étape
  suivante.
- **Ordre des fichiers dans AMC_SOURCES** — voir Passe 2 de CGen ci-dessus. Si on
  voit `error: implicit declaration of function 'Foo_Bar'` suivi de
  `error: conflicting types`, inverser l'ordre des fichiers.
- **Les types génériques s'effacent en `void*` au niveau C** — le boxing des
  primitives utilise `(void*)(intptr_t)`. Depuis v0.3.3, le CGen
  suit le type d'élément de `List<T>` / `Map<K,V>` pour les variables locales,
  les paramètres, les valeurs de retour et les annotations explicites ; `xs.Get(i)`
  est abaissé avec le bon cast (`(int)AmalgameList_get(...)` etc.)
  afin que le résultat soit typé au site d'appel sans cast manuel.
  La représentation C sous-jacente n'a pas changé.
- **Les bras de match peuvent être des statements OU des expressions** —
  `1 => return "x"` et `1 => "x"` se parsent tous les deux. `let x = match y { ... }`
  fonctionne également depuis v0.3.0 — le codegen l'enveloppe dans une expression
  de statement composé GCC. Les patterns enum algébriques et les gardes de bras
  en position d'expression ne sont pas encore supportés, cependant.
- **Les imports sont informatifs** — la stdlib du resolver est globale.
  Ne pas s'appuyer sur `import` pour la visibilité. Depuis v0.3.2, `amc fmt`
  les préserve lors de l'aller-retour (le parser stocke chacun dans `prog.Args`).

## Où chercher

- `ROADMAP_COMPLET.md` — ce qui est prévu et ce qui est en cours.
- `CONTINUATION.md` — dump de contexte pour reprendre une session.
- Le git log des branches `feature/*` — chaque fonctionnalité livrée dispose
  d'un message de commit explicatif parcourant la modification.
