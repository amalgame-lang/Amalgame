# Changelog CODE

Format : [Keep a Changelog](https://keepachangelog.com)
Versions : [Semantic Versioning](https://semver.org)

---

## [0.1.0] - 2024-01-XX

### 🎉 MILESTONE : Hello World fonctionne !

Premier programme CODE transpilé et exécuté avec succès.

### ✅ Ajouté

#### Lexer (src/core/lexer/)
- `token.vala` : définition complète des TokenType
  - Littéraux : INTEGER, FLOAT, STRING, BOOL, NULL
  - Mots-clés : class, interface, enum, let, var...
  - Opérateurs : +, -, *, /, |>, ??, ?., .., ...
  - Délimiteurs : {, }, (, ), [, ], @, ...
- `lexer.vala` : lexer complet
  - Détection correcte IDENTIFIER vs INTEGER
  - Helpers IsLetter(), IsDigit(), IsAlphaNum()
  - Strings simples et multi-lignes
  - Commentaires // et /* */
  - Suivi ligne/colonne

#### AST (src/core/parser/)
- `ast.vala` : tous les nœuds AST
  - Programme : ProgramNode, NamespaceNode, ImportNode
  - Déclarations : ClassDeclNode, MethodDeclNode,
    FieldDeclNode, PropertyDeclNode, EnumDeclNode,
    RecordDeclNode, DataClassDeclNode, ParamNode
  - Instructions : BlockNode, VarDeclNode, IfNode,
    MatchNode, WhileNode, ForNode, ForeachNode,
    ReturnNode, GuardNode, TryCatchNode, GoStmtNode
  - Expressions : BinaryExprNode, UnaryExprNode,
    CallExprNode, MemberAccessNode, NewExprNode,
    LambdaExprNode, LiteralNode, IdentifierNode...
  - Types : SimpleTypeNode, GenericTypeNode,
    FuncTypeNode, TupleTypeNode
- `ast_visitor.vala` : visitor pattern complet
- `ast_printer.vala` : debug printer (arbre textuel)

#### Parser (src/core/parser/parser.vala)
- Recursive descent parser complet
- Style Java/C# : type nom (pas Kotlin nom: type)
- Gestion des tableaux : string[]
- Mots-clés acceptés comme noms de types
- Décorateurs @memory, @pure, @realtime...
- Pattern matching complet
- Expressions avec précédence correcte
- Messages d erreur clairs avec position

#### Générateur C (src/transpiler/generator/)
- `c_generator.vala` : génération C depuis AST
  - Classes → structs C + constructeurs
  - Méthodes → fonctions C
  - Variables → déclarations C typées
  - String interpolation → code_string_format()
  - Console.WriteLine → printf natif
  - Directives #line pour debug GDB
  - Forward declarations automatiques

#### Runtime (src/transpiler/runtime/_runtime.h)
- Boehm GC intégré
- Types : i64, f64, f32, code_string, code_bool
- Console : Console_WriteLine, Console_Write
- String : format, concat, equals, contains
- Math : PI, Abs, Sqrt, Pow, Max, Min
- Collections : CodeList (liste générique)
- Result<T> et Option<T>

#### Build System
- `meson.build` : compatible Meson 0.56 + Vala 0.48
- `compile.sh` : script de build automatisé
- CI GitHub Actions : build automatique

#### Tests
- `tests/samples/hello.code` : Hello World ✅

#### Documentation
- `docs/language/grammar.ebnf` : grammaire formelle
- `docs/transpiler/lexer.md`
- `docs/transpiler/tokens.md`
- `docs/transpiler/ast.md`
- `README.md` : guide complet

### 🔧 Corrections
- Fix : isalpha()/isdigit() → IsLetter()/IsDigit()
  (bug : identifiants tokenisés comme INTEGER)
- Fix : syntaxe paramètres Java/C# (type nom)
- Fix : tableaux string[] → code_string*
- Fix : forward declarations méthodes statiques
- Fix : Vala 0.48 : => remplacé par {} pour override
- Fix : TokenType ambigu GLib vs CodeTranspiler
- Fix : ParseArgList dupliqué → ParseNewArgList
- Fix : base mot réservé → baseName
- Fix : Posix.system → GLib.Process.spawn_sync

### 🔜 À venir (v0.2.0)
- Resolver : résolution des noms et scopes
- TypeChecker : vérification et inférence des types
- Librairie Standard : Code.IO, Code.Net...
- LSP Server : autocomplétion, erreurs temps réel
- DAP Server : debug avec breakpoints visuels

---

## [Unreleased]

### 🔜 En cours
- Tests unitaires du Lexer et Parser
- Exemple Guild App complet
- Resolver / TypeChecker