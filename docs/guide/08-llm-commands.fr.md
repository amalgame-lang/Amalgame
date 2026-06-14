# 8 · Commandes pilotées par LLM

Trois commandes vous permettent de collaborer avec un LLM sur du code Amalgame :

| Commande | Direction |
|---|---|
| **`amc migrate <file\|dir>`** | autre langage → Amalgame |
| **`amc generate "<prompt>"`** | langage naturel → Amalgame |
| **`amc explain <file.am>`** | Amalgame → langage naturel |

Les trois partagent la même pile de providers, le même en-tête de prompt
sur disque, le même mécanisme d'estimation de coût `--dry-run`. Elles
sont arrivées ensemble avec la version v0.4.0.

Si vous ne les avez jamais utilisées : parcourez les sections **Démarrage
rapide** et **Sélection du provider**, puis revenez quand vous avez besoin
des détails. La justification complète du design se trouve dans
[`docs/proposals/amc-migrate.md`](https://github.com/amalgame-lang/Amalgame/blob/main/docs/proposals/amc-migrate.md).

## Démarrage rapide

```bash
# Traduire un fichier
amc migrate src/user_service.ts

# Traduire un répertoire entier (écrit <nom>.am à côté de chaque source)
amc migrate src/

# Générer un nouveau programme depuis un prompt
amc generate "an HTTP server with /health and /version routes" -o api.am

# Expliquer un fichier .am en anglais courant
amc explain src/lsp.am

# Idem, mais en français
amc explain src/lsp.am --lang French
```

La sortie de `migrate` et `generate` est automatiquement validée avec
`amc --check`. En cas d'échec, le fichier est quand même écrit afin que
vous puissiez l'inspecter et le corriger manuellement, mais le code de
retour est non nul.

## Sélection du provider

Cinq providers sont livrés d'emblée :

| Provider | Backend | Sélectionné automatiquement quand |
|---|---|---|
| `claude` | délègue au CLI local `claude` (Claude Code) | (repli par défaut) |
| `claude-api` | `POST api.anthropic.com/v1/messages` | `ANTHROPIC_API_KEY` est défini |
| `chatgpt` | `POST api.openai.com/v1/chat/completions` | `OPENAI_API_KEY` est défini |
| `gemini` | `POST generativelanguage.googleapis.com/v1beta/...` | `GEMINI_API_KEY` est défini |
| `custom` | délègue à `$AMC_CUSTOM_PROVIDER_CMD` (stdin = prompt, stdout = réponse) | jamais (opt-in uniquement) |

**Priorité de sélection automatique**, quand `--provider` n'est pas passé :

1. `ANTHROPIC_API_KEY` → `claude-api`
2. `OPENAI_API_KEY` → `chatgpt`
3. `GEMINI_API_KEY` → `gemini`
4. repli → `claude` (CLI)

Forcer explicitement :

```bash
amc migrate file.cs --provider chatgpt --model gpt-4o
amc generate "..." --provider gemini --model gemini-1.5-pro
amc explain file.am --provider custom
```

Pour `custom`, pointez vers n'importe quel script qui lit un prompt sur
stdin et écrit la réponse sur stdout :

```bash
export AMC_CUSTOM_PROVIDER_CMD="ollama run codellama"
amc migrate file.go --provider custom
```

## Rapport de coût

Deux formes, selon que l'appel a déjà eu lieu ou non.

**Estimation pré-appel** (`--dry-run`) — basée sur une heuristique de
4 caractères par token et des tables de prix de modèles intégrées :

```
$ amc migrate src/api.ts --dry-run
[migrate] would migrate: src/api.ts (TypeScript, 240 lines)
[migrate] would write:   src/api.am
[migrate] provider:      claude-api
[migrate] estimated cost: ~6862 in + ~1000 out -> ~$0.05 (claude-sonnet-4-6)
```

**Coût réel** (depuis la v0.4.3) — affiché après une migration réussie,
extrait de l'objet `usage` de la réponse API. Pas de `~`, pas
d'heuristique — tokens facturés exacts :

```
$ amc migrate src/api.ts
[migrate] processing src/api.ts (TypeScript, 240 lines, provider=claude-api)...
[migrate] wrote src/api.am
[migrate] cost: 6431 in + 982 out = $0.04 (claude-sonnet-4-6)
[migrate] check passed
```

Chemins d'extraction par provider :
- `claude-api` → `usage.input_tokens` / `usage.output_tokens`
- `chatgpt` → `usage.prompt_tokens` / `usage.completion_tokens`
- `gemini` → `usageMetadata.promptTokenCount` / `usageMetadata.candidatesTokenCount`

Les appels par CLI (`claude`, `custom`) ne rapportent rien, car la
facturation est sur votre abonnement ou modèle local. Le résultat a
`InputTokens = -1`, que le formateur traite comme « ne pas afficher la
ligne de coût ».

## Cache des résultats (`amc migrate` uniquement)

Relancer `amc migrate` sur une source inchangée avec le même prompt
système saute entièrement l'appel LLM. Le cache réside dans
`~/.cache/amalgame/migrate/<sha256>.am` et est indexé sur
`SHA-256(source + prompt système)`. Donc :

| Changement | Effet |
|---|---|
| Source modifiée | nouveau hash → rappel LLM |
| `amc` mis à jour avec de nouvelles conventions | nouveau hash → rappel |
| `grammar.ebnf` mis à jour | nouveau hash → rappel |
| Rien n'a changé | cache hit → réutilisation du résultat |

`--no-cache` contourne à la fois la lecture et l'écriture du cache.

## Docs du prompt sur disque

Le prompt système des trois commandes embarque la référence grammaticale
d'Amalgame (`docs/language/grammar.ebnf`) et le tour du langage
(`docs/guide/02-language-tour.md`) afin que le LLM dispose d'une syntaxe
faisant autorité. L'ordre de recherche :

1. `<exec-dir>/../share/amalgame/docs/...` (layout d'installation
   Linux/macOS — peuplé par `install.sh` et les archives de release)
2. `<exec-dir>/docs/...` (Windows / installation portable)
3. `./docs/...` (checkout source / développement)

Si aucun de ces chemins n'est accessible, les commandes dégradent
gracieusement vers un bloc de conventions embarqué — toujours
fonctionnel, mais le modèle perd l'accès à la grammaire canonique.
Exécutez `amc migrate --prompt-only` sur n'importe quel fichier pour
afficher le prompt système+utilisateur assemblé et inspecter ce que le
LLM reçoit vraiment.

## Sortie en streaming (`generate` et `explain`)

Pour les générations longues, `--stream` fait affluer la réponse
directement sur stdout au fur et à mesure de sa production. Contraintes
(refusées en amont avec des erreurs explicites) :

- Nécessite `--provider claude` (CLI). Le streaming API via SSE est prévu
  pour la v3.
- Incompatible avec `-o` (pas de texte bufferisé à écrire dans un
  fichier).

```bash
amc generate "implement quicksort with pivot tuning" --stream
```

## Matrice des options

| Option | migrate | generate | explain |
|---|:---:|:---:|:---:|
| `<positionnel>` | fichier ou répertoire | prompt | fichier |
| `-o / --output` | ✓ (fichier unique) | ✓ | ✓ |
| `--lang` | ✓ (indication de langage source) | — | ✓ (langue de sortie) |
| `--provider` | ✓ | ✓ | ✓ |
| `--model` | ✓ | ✓ | ✓ |
| `--dry-run` | ✓ | ✓ | ✓ |
| `--prompt-only` | ✓ | ✓ | ✓ |
| `--stream` | — | ✓ | ✓ |
| `--no-check` | ✓ | ✓ (avec `-o`) | — (pas de validation de sortie) |
| `--no-cache` | ✓ | — (pas de cache) | — |
| `--max-lines` | ✓ (défaut 2000) | — | — |
| `--force` | ✓ | ✓ | ✓ |

## Limitations connues

- Le streaming API est bufferisé, pas via Server-Sent Events. Seul le
  CLI local `claude` streame visiblement avec `--stream`.
- Le LLM produit parfois du code qui compile mais ne correspond pas à
  l'intention de la source — relisez toujours le fichier migré. Le
  `amc --check` automatique ne détecte que les erreurs de typage, pas
  les dérives sémantiques.
- Les fichiers sources dépassant `--max-lines` (défaut 2000) sont refusés
  avec une suggestion de découpage. Forcez à vos risques et périls — les
  grands prompts dépassent la fenêtre de cache de prompt d'Anthropic et
  produisent une sortie notablement moins bonne.
- Les signatures lambda de `Reduce` nécessitent encore l'inférence du
  type de l'argument initial — le LLM génère parfois du code qui se
  heurte à cela. Le contournement est documenté dans le prompt, mais si
  vous le rencontrez, repliez-vous sur un `for-in` avec `var acc`.

## Dépannage

| Symptôme | Correction probable |
|---|---|
| `claude CLI not found on PATH` | Installez Claude Code, ou utilisez `--provider claude-api` (nécessite `ANTHROPIC_API_KEY`). |
| `ANTHROPIC_API_KEY not set` | `export ANTHROPIC_API_KEY=sk-ant-...`. Même forme pour OPENAI / GEMINI. |
| `provider 'X' not supported` | Faute de frappe, ou tentative de streaming v3 sur un provider API. Vérifiez `amc migrate --help`. |
| HTTP 401 / 403 depuis l'API | Clé incorrecte ou expirée. Vérifiez avec `curl -H "x-api-key: $ANTHROPIC_API_KEY" https://api.anthropic.com/v1/models`. |
| HTTP 429 | Débit limité. Attendez, ou passez à un `--model` moins sollicité. |
| `check failed (typechecker errors in the migrated file)` | Inspectez le `.am` (il a quand même été écrit). Cause fréquente : une limitation d'Amalgame que le LLM n'a pas esquivée — voir « Limitations connues » ci-dessus. |
| La migration est lente | Le modèle par défaut du CLI `claude` est Opus. Passez `--model claude-sonnet-4-6` pour un gain de vitesse d'environ 3×. |

## Où regarder dans le code

- [`src/migrate.am`](https://github.com/amalgame-lang/Amalgame/blob/main/src/migrate.am) — `MigrateCommand`. Héberge
  la table de dispatch des providers, les helpers JSON, le cache,
  l'estimateur de coût.
- [`src/generate.am`](https://github.com/amalgame-lang/Amalgame/blob/main/src/generate.am) — `GenerateCommand`.
  Réutilise `MigrateCommand.CallProviderRaw` + `LoadDocsHeader` +
  `AutoSelectProvider`.
- [`src/explain.am`](https://github.com/amalgame-lang/Amalgame/blob/main/src/explain.am) — `ExplainCommand`. Même
  pattern de réutilisation.
- [`docs/proposals/amc-migrate.md`](https://github.com/amalgame-lang/Amalgame/blob/main/docs/proposals/amc-migrate.md) —
  document de design complet avec la roadmap v0/v1/v2.
