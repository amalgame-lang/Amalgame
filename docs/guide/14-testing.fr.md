# Tests

L'approche d'Amalgame en matière de tests est délibérément sans framework.
Un test est un programme normal qui affiche des lignes `[PASS]` / `[FAIL]` /
`[SKIP]` ; `amc test` découvre ces programmes, compile et exécute chacun
d'eux, puis additionne les tags. Il n'y a aucune bibliothèque d'assertions à
apprendre, aucune annotation, aucun DSL de test runner — juste
`Console.WriteLine` et une convention de nommage.

---

## Écrire un test

Un fichier de test est nommé `*_test.am` et possède un `Main()` qui affiche
une ligne taguée par vérification :

```amalgame
// tests/arith_test.am
class Program {
    public static void Main() {
        let a = 2 + 3
        if (a == 5) {
            Console.WriteLine("[PASS] add: 2 + 3 == 5")
        } else {
            Console.WriteLine("[FAIL] add: expected 5, got " + String_FromInt(a))
        }

        let b = 7 * 6
        if (b == 42) {
            Console.WriteLine("[PASS] mul: 7 * 6 == 42")
        } else {
            Console.WriteLine("[FAIL] mul: expected 42, got " + String_FromInt(b))
        }
    }
}
```

Les trois tags que le runner comptabilise, en **début de ligne** :

| Tag | Signification |
|-----|---------|
| `[PASS]` | la vérification est satisfaite |
| `[FAIL]` | la vérification n'est pas satisfaite — l'exécution échoue |
| `[SKIP]` | non exécuté intentionnellement (p. ex. nécessite un service, connu pour être instable) |

```amalgame
Console.WriteLine("[PASS] sanity: hello")
Console.WriteLine("[FAIL] todo: not implemented yet")
Console.WriteLine("[SKIP] flaky: see issue #42")
```

C'est tout le contrat. Tout ce que vous pouvez calculer, vous pouvez l'asserter
en branchant et en affichant le bon tag.

---

## Lancer la suite

```bash
amc test                  # discover *_test.am under . (recursively), run all
amc test ./tests          # restrict discovery to a directory
```

`amc test` compile chaque fichier découvert, l'exécute et agrège les tags dans
un récapitulatif :

```
── tests/arith_test.am
  [PASS] add: 2 + 3 == 5
  [PASS] mul: 7 * 6 == 42
── tests/mixed_test.am
  [PASS] sanity: hello
  [FAIL] todo: not implemented yet
  [SKIP] flaky: see issue #42

──────────────────────────────────
PASS: 3  FAIL: 1  SKIP: 1
```

Un fichier qui **plante** (ou qui n'émet aucun tag) est remonté comme un échec,
de sorte qu'un segfault ou un corps oublié ne peut pas se faire passer pour
« 0 failures ».

### Options

| Flag | Effet |
|------|--------|
| `--filter <pat>` | n'exécute que les tests dont le chemin contient `<pat>` |
| `--ci` | sortie laconique — supprime les lignes `PASS`/`SKIP`, conserve `FAIL` + le total |
| `--list` | affiche les chemins découverts et quitte (pas de compilation, pas d'exécution) |

```bash
amc test --filter parser        # just the parser tests
amc test --ci                   # CI-friendly: noise down, failures up
amc test --list                 # what would run?
```

---

## Organiser les tests

- **Le nommage est le mécanisme de découverte.** Seuls les fichiers
  `*_test.am` sont pris en compte. Un fichier helper sans le suffixe est ignoré
  par le runner (tant mieux — placez les fixtures partagées dans de simples
  fichiers `.am`).
- **Groupez par répertoire.** `amc test ./tests/parser` exécute un seul
  domaine ; `--filter` découpe transversalement à travers les répertoires.
- **`fixtures/` est élagué.** Les fichiers situés sous n'importe quel
  répertoire `fixtures/` imbriqué ne sont *pas* exécutés automatiquement —
  c'est là que vous conservez les entrées d'exemple et les fichiers `.am` non
  destinés à être exécutés. Pour en exécuter un explicitement, pointez dessus :
  `amc test ./tests/fixtures/<name>/`.

> Les suites plus volumineuses de ce projet sont organisées en **bundles** — un
> unique `*_test.am` dont le `Main()` pilote de nombreux cas (les propres
> `core_test.am`, `stdlib_test.am`, etc. du compilateur). Un bundle peut même
> invoquer `amc test` sur un répertoire de fixtures et grepper sa ligne de
> récapitulatif, ce qui est la manière dont le runner se teste lui-même.
> Commencez par un fichier par domaine ; recourez à un bundle lorsque le coût
> de mise en place vaut la peine d'être amorti.

---

## Un pattern pour les assertions

Comme il n'y a pas de bibliothèque d'assertions, un petit helper local garde
les corps de tests lisibles :

```amalgame
class Program {
    public static void check(bool cond, string label) {
        if (cond) {
            Console.WriteLine("[PASS] " + label)
        } else {
            Console.WriteLine("[FAIL] " + label)
        }
    }

    public static void Main() {
        Program.check(2 + 3 == 5,  "add")
        Program.check("ab" + "c" == "abc", "concat")
    }
}
```

Gardez le label descriptif — c'est ce que vous lirez dans la sortie `--ci`
lorsque quelque chose casse. Notez que le helper est `public static` et appelé
de façon qualifiée (`Program.check(...)`) — une méthode statique invoquée
depuis `Main` est atteinte via le nom de sa classe.

---

## Pièges

- **Les tags doivent commencer la ligne** (après les espaces de début que le
  runner rogne). `Console.WriteLine("result: [PASS]")` n'est *pas* compté —
  placez le tag en premier.
- **Pas de tags = échec.** Si un fichier de test s'exécute mais n'affiche rien,
  le runner le comptabilise comme un échec, et non comme « 0 tests ». Émettez
  toujours au moins un tag.
- **`[SKIP]` est un vrai résultat, pas un succès.** Utilisez-le (avec une
  raison) pour les vérifications qui nécessitent un service indisponible — cela
  garde la suite honnête sans la faire passer au rouge.
- **Le code de sortie suit le total** — tout `[FAIL]` (ou un plantage) fait
  sortir `amc test` avec un code non nul, de sorte qu'il s'intègre directement
  dans la CI comme garde-fou.

Pour la référence de la CLI (chaque flag de `amc test`), voir
[03-cli-reference.fr.md](03-cli-reference.fr.md) ; pour le pipeline de build
que la CI exécute autour des tests, voir
[06-build-and-tooling.fr.md](06-build-and-tooling.fr.md).
