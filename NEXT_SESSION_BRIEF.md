# Brief pour la prochaine session Pollen

À coller en début de prochaine session (ou juste pointer Claude vers
ce fichier). Récap court et plan d'action.

## Où on en est (fin 2e sitting 2026-05-30)

**Runtime Pollen v3** : déjà shippé sur main d'`amalgame-pollen`
(PR #2 mergée commit `4905047`). CI verte. Couvre Phases 1-4
partial.

**Phase 5 (UI pollen-manager)** : **gros bond aujourd'hui — 6 commits
pushés sur master de `pollen-manager`** :

| # | Commit | Slice |
|---|--------|-------|
| 1 | c72d925 | Sidebar Bus/Callable/Dead + search + fold |
| 2 | 5730e57 | Flowchart dagre + clusters compound for/while |
| 3 | 37a3dab | Polish ↗ + labels boundary + case pill |
| 4 | bd3a659 | Cross-entry hints sidebar + dashed goto |
| 5 | 4d5cd3e | Add/delete root step + save pulse keyframe |
| 6 | fc75ddb | Inline step editor root + nested (slice 4b) |

dagre vendoré dans `public/vendor/dagre.min.js` (276KB) et chargé
on-demand via `<script>` dynamique pour éviter rebuild AM.

## Lecture obligatoire avant de plonger

1. **`session_2026_05_30_pollen_v3_runtime.md`** dans la memory — bien
   à jour, contient la liste des 6 commits + limites résiduelles
2. `docs/proposals/pollen-v3.md` dans `amalgame-pollen` — spec
3. `project_pollen_manager_dag_refactor.md` dans la memory

## Ce qui RESTE pour `amalgame-pollen v0.2.0`

### Phase 5 — slices restants pour terminer le manager v3

**Slice 4c (petit, ~150 LOC)** :
- ↑↓ buttons sur chaque root step pour reorder (déjà X pour delete)
- ✕ delete sur les nested steps (pas juste root)
- Logique : navigateur dans `entry.do` avec un path `[idx, idx, …]`
  pour résoudre la position

**Slice 4d (moyen, ~300 LOC)** :
- Edit entry metadata : `on` (bus topic), `params`, `returns`
- Add new entry (button au-dessus de la sidebar)
- Rename entry (avec mise à jour des goto targets qui pointent vers)
- Delete entry

**Slice 4e (petit, ~100 LOC)** :
- Toolbar add step **par niveau** (pas juste root)
- Boutons `+ call / + set / …` à la fin de chaque for body, while
  body, case do

### Phase 6 — drop v2 + tag v0.2.0

Bloquée par Phase 5 complète. Quand les 4c/4d/4e seront là :
- Drop le dispatcher v2 d'amalgame-pollen
- Drop les `examples/workflow-v2-*.json`
- Update README + CHANGELOG
- Bump `version = "0.2.0"`
- Tag + PR packages-index
- Bumper consumers (`amalgame-live`, `pollen`)

## Plan d'attaque suggéré pour reprise

Continuer slice par slice — c'est ce qui marche :

1. **Slice 4c** d'abord (1 session, petit) : reorder + delete nested
2. **Slice 4e** ensuite (1 session, petit) : add per-level
3. **Slice 4d** (1 session) : entry-level edits
4. **Phase 6** (1 session) : drop v2 + tag

## Limites résiduelles connues (à mentionner si Bastien teste)

- L'edit du `if` n'expose que les `when` des cases, pas le `do`
  (mais cliquer sur les nested steps de chaque case do fonctionne
  — c'est juste un editor par step en cascade).
- Edge labels dagre (`▶ each` / `↪ after`) tombent parfois sur des
  steps voisins. Workaround actuel = 70%/30% du polyline.
  Vrai fix = computer crossing point avec la cluster bbox.
- Save toolbar à droite : le bouton Save pulse maintenant en
  keyframe 1.4s — visible across the screen. Click le pour
  persister.

## Commands utiles

```bash
# Lancer le manager local avec une fixture v3
mkdir -p /tmp/pollen-manager-demo
cp ~/Développement/amalgame-pollen/examples/workflow-v3-feature-demo.json \
   /tmp/pollen-manager-demo/workflow.json
cd ~/Développement/pollen-manager
WORKFLOW_PATH=/tmp/pollen-manager-demo/workflow.json \
SHARED_DIR=/tmp/pollen-manager-demo \
./server
# ouvrir http://localhost:3000

# amalgame-pollen tests
cd ~/Développement/amalgame-pollen
./tests/run_tests.sh ~/.local/bin/amc           # C-side
./tests/build-v3-validator-smoke.sh             # validator
./tests/build-cel-lite-smoke.sh                  # CEL-lite
./tests/build-v3-loader-smoke.sh                 # loader
./tests/build-v3-dispatch-smoke.sh               # dispatcher
```

## Si je veux juste un alpha tag rapide

Toujours valide : runtime v3 ready sur `main`, manager en bonne
voie. Pour publier un `v0.2.0-alpha` :
1. Bump `version = "0.2.0-alpha"` dans `amalgame-pollen/amalgame.toml`
2. `git tag v0.2.0-alpha` + push
3. PR sur `amalgame-lang/packages-index` ajoutant le tag
4. Bumper la dépendance dans consumers qui veulent tester
