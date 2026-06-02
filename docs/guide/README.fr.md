# Amalgame — Guide de l'utilisateur

Une référence complète pour les utilisateurs et les contributeurs. Si vous
voulez simplement écrire de l'Amalgame, commencez au chapitre 1 et
continuez. Si vous êtes là pour bidouiller le compilateur lui-même, sautez
au chapitre 7.

> 🇬🇧 🇫🇷 Le guide est entièrement bilingue : chaque chapitre existe en
> anglais (`NN-….md`) et en français (`NN-….fr.md`). Sur
> [docs.amalgame.me](https://docs.amalgame.me) la langue est choisie
> automatiquement, avec une bascule sur chaque page.

## Table des matières

1. [Premiers pas](01-getting-started.md) — installation, hello world, le
   rythme compiler-puis-lier.
2. [Tour du langage](02-language-tour.md) — chaque fonctionnalité avec des
   extraits exécutables : types, classes, génériques, pattern matching,
   décorateurs…
3. [Référence CLI](03-cli-reference.md) — chaque option d'`amc`, codes de
   sortie, workflows typiques, astuces de débogage.
4. [Bibliothèque standard](04-stdlib.md) — `Console`, `File`, `Path`,
   `IO.FileWatcher`, `Math`, `Math.Vec` (Vec3/Vec4/Mat4), `String`,
   `List`/`Map`/`Set`, `Http`, `Tcp`, `Net.WebSocket`, `Json`,
   `Formats.Yaml`, `Formats.MsgPack`, `Random`, `Encoding`,
   `DateTime`, `Crypto`, `Regex`, `Compress`, `Logging`, `Service`,
   `Database.SQLite`, `Database.DuckDB`.
5. [Runtime & interop C](05-runtime-and-interop.md) — modèle mémoire,
   bdwgc, correspondance des types vers le C, appeler du C depuis Amalgame,
   blocs C en ligne (`@c { … }`) depuis la v0.7.4.
6. [Build & outillage](06-build-and-tooling.md) — `build_amc.sh`,
   le bootstrap par snapshot, la CI, les releases, et le débogueur
   `amc dap` (v0.8.0+).
7. [Internes du compilateur](07-internals.md) — comment `amc` est
   structuré, ajouter un builtin, ajouter une forme syntaxique, ajouter
   une règle de CGen.
8. [Commandes LLM](08-llm-commands.md) — `amc migrate`, `amc generate`,
   `amc explain` : fournisseurs, variables d'env, cache, choix de conception.
9. [`amalgame-ui-web`](09-ui-web/README.md) — binding GUI desktop
   par-dessus la webview de l'OS (WebView2 / WKWebView / WebKitGTK). Cinq
   sous-chapitres : premiers pas, catalogue de widgets, événements +
   mises à jour partielles du DOM, mise en page + thèmes, échappatoires.
10. [Cookbook](10-cookbook.md) — extraits idiomatiques pour les motifs dont
    vous aurez besoin tous les jours : chaînes, collections, fichiers +
    JSON, parsing d'arguments CLI, processus, réseau, pattern matching,
    threads, primitives C en ligne.
11. [Framework web (Mosaic)](11-web-mosaic.md) — le framework serveur
    `amalgame-web` : routage, `WebContext`/`HttpResponse`, middleware
    (en-têtes de sécurité, CORS, CSRF, limitation de débit), sessions,
    fichiers statiques, HTTP/1.1 + HTTPS. (Distinct du GUI desktop du
    chapitre 9.)
12. [Utiliser des packages](12-packages.md) — le workflow des packages
    externes : trouver, ajouter, utiliser, mettre à jour, lockfile + builds
    CI reproductibles, et dépannage. (La référence est au chapitre 3.)
13. [Async & concurrence](13-async.md) — le sucre `async fn`/`await`
    (v0.8.70) et le runtime `amalgame-async` : fibers, channels,
    l'ordonnanceur coopératif, l'I/O non bloquante.
14. [Tests](14-testing.md) — la convention sans framework `*_test.am` /
    `[PASS]`/`[FAIL]`/`[SKIP]`, lancer `amc test`, et comment organiser
    les suites.
15. [Outillage éditeur & débogage](15-debugging.md) — `amc lsp` (les
    fonctionnalités du serveur de langage + la configuration de l'éditeur)
    et `amc dap` (breakpoints/pas-à-pas dans le source `.am`, le pont, `-g`).
16. [Héberger avec Mosaic](16-hosting.md) — mettre un serveur Mosaic en
    production : plusieurs sites sur un hôte (dispatch par `Host`),
    certificats Let's Encrypt + SNI, tourner sous systemd, et un
    renouvellement de cert qui marche vraiment.

## Guides pratiques

Des pas-à-pas orientés tâches, à côté des chapitres de référence
ci-dessus :

- [Raspberry Pi](howto/raspberry-pi/README.md) — de la carte SD vierge
  jusqu'au pilotage de vrai matériel (GPIO, boutons, capteurs I²C,
  barre de LEDs) avec le package `amalgame-hardware-gpio`.

Le [README](../../README.md) du dépôt a le pitch et les captures d'écran.
La [ROADMAP](../../ROADMAP_COMPLET.md) suit ce qui est prévu ensuite.
