# CODE Programming Language 🚀

> Un langage moderne, expressif et portable.
> Transpile vers C → compilable partout via GCC.

---

## Vision

CODE prend le meilleur de chaque langage :

| Fonctionnalité        | Inspiré de        |
|-----------------------|-------------------|
| Inférence de types    | Kotlin / Swift    |
| Null safety           | Kotlin / Swift    |
| Pattern matching      | Rust / Haskell    |
| Result / Option       | Rust              |
| Data classes          | Kotlin            |
| Extension methods     | Kotlin / C#       |
| Pipeline `\|>`        | F# / Elixir       |
| Async / Await         | C# / JS           |
| GC par défaut         | Go / C#           |
| Décorateurs mémoire   | Nim / D           |

---

## Exemple

```java
namespace MyApp

import Code.IO

public class Program {

    public static void Main(string[] args) {

        let players = new List<Player>([
            new Player("Arthus", level: 42),
            new Player("Merlin", level: 38),
            new Player("Robyn",  level: 15)
        ])

        let veterans = players
            |> Where(p  => p.Level >= 35)
            |> OrderBy(p => p.Level)
            |> ToList()

        veterans |> ForEach(p =>
            Console.WriteLine("⚔️  {p.Name} (Lvl {p.Level})")
        )
    }
}
```

Sortie :
```
⚔️  Merlin (Lvl 38)
⚔️  Arthus (Lvl 42)
```

---

## Architecture

```
.code source
    │
    ▼
[ Lexer ]  →  Tokens
    │
    ▼
[ Parser ]  →  AST
    │
    ▼
[ Resolver + TypeChecker ]  →  AST annoté
    │
    ▼
[ Générateur C ]  →  .c
    │
    ▼
[ GCC ]  →  Exécutable natif
```

---

## Outils

| Outil       | Description                    |
|-------------|--------------------------------|
| `codec`     | Transpileur principal          |
| `codec-lsp` | Serveur LSP (éditeurs)         |
| `codec-dap` | Debug Adapter (breakpoints)    |

---

## Éditeurs Supportés

Via LSP + DAP :
- ✅ VSCode
- ✅ NeoVim
- ✅ Emacs
- ✅ Vim
- ✅ Sublime Text
- ✅ IntelliJ

---

## Compilation

```bash
# Dépendances
sudo apt install valac libglib2.0-dev libgee-0.8-dev \
                 meson ninja-build libgc-dev

# Compiler
meson setup build
cd build && ninja

# Tester
./codec --version
```

---

## Project Status

```
✅ Language specification
✅ EBNF grammar
✅ Lexer
✅ AST (complete nodes)
✅ Parser
✅ Resolver (name resolution & scope checking)
🔜 Type Checker
✅ C Generator
🔜 Standard Library
🔜 LSP / DAP
```

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

---

## Roadmap

Voir [docs/changelog/roadmap.md](docs/changelog/roadmap.md)

---

## Licence

Licensed under Apache 2.0
Copyright (c) 2026 Bastien MOUGET
[LICENSE](LICENSE)

---

## Auteur

**NeitsabTeguom**
GitHub : [@NeitsabTeguom](https://github.com/NeitsabTeguom)
