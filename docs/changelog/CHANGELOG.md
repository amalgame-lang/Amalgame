# Changelog CODE

Format : [Keep a Changelog](https://keepachangelog.com)
Versions : [Semantic Versioning](https://semver.org)

---

## [Unreleased]

### 🔜 En cours
- Parser (tokens → AST)
- Resolver (noms et scopes)
- Type Checker
- Générateur C

---

## [0.1.0] - 2024-01-XX  ← mettre la vraie date

### ✅ Ajouté

#### Spécification
- Vision et philosophie du langage CODE
- Grammaire EBNF complète
- Système de types
- Gestion mémoire (GC + décorateurs)

#### Transpileur (Vala)
- `src/core/lexer/token.vala`   : Définition des tokens
- `src/core/lexer/lexer.vala`   : Lexer complet
- `src/core/parser/ast.vala`    : Nœuds AST complets
- `src/core/parser/ast_visitor.vala` : Visitor pattern
- `src/core/parser/ast_printer.vala` : Debug printer

#### Éditeurs
- `editors/vscode/syntaxes/code.tmLanguage.json`
- `editors/vscode/snippets/code.json`

#### Documentation
- `docs/language/grammar.ebnf`
- `docs/transpiler/lexer.md`
- `docs/transpiler/tokens.md`
- `docs/transpiler/ast.md`
