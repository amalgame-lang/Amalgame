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

## Statut du Projet

```
✅ Spécification du langage
✅ Grammaire EBNF
✅ Lexer
✅ AST (nœuds complets)
🔜 Parser
🔜 Resolver / Type Checker
🔜 Générateur C
🔜 Librairie Standard
🔜 LSP / DAP
```

---

## Roadmap

Voir [docs/changelog/roadmap.md](docs/changelog/roadmap.md)

---

## Licence

Apache  License - voir [LICENSE](LICENSE)

---

## Auteur

**NeitsabTeguom**
GitHub : [@NeitsabTeguom](https://github.com/NeitsabTeguom)
