# Utiliser les packages

La bibliothèque standard d'Amalgame est délibérément réduite. La plupart
des fonctionnalités — HTTP, TLS, crypto, datetime, Redis, le framework web
Mosaic — sont livrées sous forme de **packages externes** dans
`github.com/amalgame-lang/amalgame-*`, intégrés projet par projet. Ce
chapitre décrit le flux de travail de bout en bout : trouver un package,
l'ajouter, l'utiliser, le maintenir à jour et résoudre les problèmes.

> Pour la **référence** exhaustive des commandes et du manifeste (chaque
> option, chaque champ d'`amalgame.toml`, les opérateurs semver), voir
> [03-cli-reference.fr.md](03-cli-reference.fr.md). Ce chapitre en est le
> compagnon orienté tâches.

Les deux fichiers qui suivent vos dépendances :

- **`amalgame.toml`** — ce que vous *avez demandé* (package + tag). Vous
  l'éditez (directement ou via `amc package add/remove`) ; versionnez-le.
- **`amalgame.lock`** — ce qui a été *résolu* (révisions Git exactes).
  Généré automatiquement ; versionnez-le aussi, afin que collaborateurs et
  CI construisent le même code.

---

## Trouver un package

```bash
amc package search redis          # recherche par sous-chaîne dans l'index
amc package info redis            # description, url, licence, versions
amc package versions redis        # chaque tag indexé + compatibilité avec votre amc
```

`search` marque chaque tag `✓`/`✗` par rapport à votre `amc` en cours
d'exécution et signale le tag `← latest compatible`. L'index est mis en
cache ~30 min ; ajoutez `--refresh` pour forcer une nouvelle récupération.

Vous ne savez pas quel package fournit un type que vous avez vu ?
`suggest` fait correspondre une classe ou un namespace à un package :

```bash
amc package suggest WebApp        # → amalgame-web
amc package suggest Amalgame.Net.Http
```

---

## Ajouter un package

```bash
amc package add redis             # dernier tag compatible avec votre amc
amc package add redis@v0.2.0      # épingler un tag exact
amc package add web datetime      # plusieurs à la fois
```

Un simple `amc package add <name>` (nom court indexé, sans `@tag`) résout
automatiquement le dernier tag compatible avec votre compilateur installé
et affiche son choix :

```
Auto-resolved 'redis' → 'redis@v0.2.0' (latest compatible with amc 0.8.71)
```

`add` écrit la dépendance dans `amalgame.toml`, enregistre la révision
résolue dans `amalgame.lock` et (sauf `--no-precompile`) compile le
package au moment de l'installation, de sorte que la première
construction de votre code soit rapide.

Votre `amalgame.toml` gagne une entrée comme :

```toml
[dependencies.redis]
git = "github.com/amalgame-lang/amalgame-database-nosql-redis"
tag = "v0.2.0"
```

et `amalgame.lock` épingle le commit exact :

```toml
[[package]]
name = "redis"
git  = "github.com/amalgame-lang/amalgame-database-nosql-redis"
tag  = "v0.2.0"
rev  = "…"
```

> **Les dépendances transitives sont résolues automatiquement**
> (amc ≥ 0.8.39). Lorsque vous ajoutez `web`, ses propres dépendances
> (`net-http`, `tls`, `crypto`, …) sont également intégrées — vous n'avez
> pas à les lister vous-même.

---

## L'utiliser dans le code

Un package déclare le namespace auquel il contribue (son
`[stdlib].namespace`). Vous faites un `import` de ce namespace et appelez
ses types — `amc build` attache automatiquement la façade du package
correspondante :

```amalgame
import Amalgame.Collections
import Amalgame.Net.Http
import Amalgame.Web

public class Program {
    public static void Main(string[] args) {
        let app = new WebApp()
        app.Get("/", ctx => HttpResponse.New().Text("hi"))
        app.Serve(8080)
    }
}
```

```bash
amc build main.am        # résout les deps depuis amalgame.toml + .lock, les lie
```

La ligne `import` nomme le *namespace* du package, pas son slug — par
exemple, le package `web` expose `Amalgame.Web`, et `net-http` expose
`Amalgame.Net.Http`. `amc package info <name>` affiche le namespace ; le
README du package aussi.

---

## Maintenir les dépendances à jour

```bash
amc package outdated              # deps avec un tag compatible plus récent
amc package update redis@v0.3.0   # mettre à jour un tag épinglé (met à jour toml + lock)
```

`update` prend un `name@tag` explicite — il n'existe pas de « tout mettre
à jour » aveugle, à dessein : vous choisissez chaque montée de version et
le lockfile l'enregistre. Après une mise à jour, reconstruisez et lancez
vos tests.

Retirez une dépendance dont vous n'avez plus besoin :

```bash
amc package remove redis          # la retire de toml + lock
amc package remove redis@v0.2.0   # @tag est un contrôle de sécurité — refuse si
                                  # le tag installé diffère
```

---

## Builds reproductibles (CI)

Versionnez **les deux** fichiers `amalgame.toml` et `amalgame.lock`. Sur
un checkout neuf, vérifiez que le cache correspond au lock avant de
construire :

```bash
amc package check --frozen        # sort avec le code 1 si cache ≠ lock — fait échouer le job CI
```

`--frozen` est la forme adaptée à la CI : elle ne modifie jamais rien,
elle affirme simplement que ce qui est installé correspond à ce que le
lockfile épingle. Sans `--frozen`, `check` signale les écarts sans
échouer.

Auditez les licences et la paternité de tout ce que vous intégrez :

```bash
amc package notice                # licence + paternité agrégées des deps
```

---

## Résolution des problèmes

**`amc build` signale `Unknown symbol 'WebApp'` (ou similaire) pour un
type que vous savez présent dans un package.** Le package n'est pas
attaché. Vérifiez que :
1. il figure dans `amalgame.toml` (`amc package list` affiche les deps
   installées) ;
2. votre source fait un `import` du **namespace** du package (par exemple
   `import Amalgame.Web`), et non de son slug.
   amc n'attache une façade que lorsque le namespace correspondant est
   importé.

> Le **LSP** d'amc (VS Code / Neovim) peut afficher de *faux*
> avertissements `Unknown symbol` pour les types de packages, même quand
> `amc build` réussit — l'éditeur ne charge pas toujours le lockfile.
> Fiez-vous au build ; le diagnostic est la limite connue, pas votre
> code.

**Pas de `@tag` et la mauvaise version est résolue.** La résolution
automatique ne fonctionne que pour les **noms courts indexés**. Pour un
package non indexé, ou pour être explicite, épinglez le tag :
`amc package add <name>@<tag>` (ou une URL git complète).

**Un package nécessite un amc plus récent que le vôtre.** `search` /
`versions` marquent les tags incompatibles `✗`. La contrainte
`[package].required-amalgame` du package (par exemple `">=0.8.58"`) est
vérifiée au moment de la résolution — mettez amc à jour, ou épinglez un
tag compatible plus ancien.

**Les contraintes de version dans les `[dependencies]` propres à un
package** utilisent des opérateurs de style npm (`>=`, `^`, `~`, `=`,
nu = `>=`) ; voir la
[référence CLI](03-cli-reference.fr.md#référence-du-manifeste-amalgametoml).

**Index ou cache périmé.** `amc package search --refresh` récupère à
nouveau l'index ; `amc package cache clear` le supprime (`--all` efface
aussi les packages téléchargés).

---

## Publier votre propre package

Si vous *créez* un package plutôt que d'en consommer un, la forme
d'`amalgame.toml` (les blocs `[package]` + `[stdlib]` :
`class`/`classes`, `namespace`, `header`, `facade`, `sources`, `libs`)
est documentée dans la
[section manifeste de la référence CLI](03-cli-reference.fr.md#référence-du-manifeste-amalgametoml).
La convention pour figurer dans l'index se trouve dans le dépôt
`amalgame-lang/packages-index`.
