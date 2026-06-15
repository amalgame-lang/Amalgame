# Changelog

All notable changes to Amalgame are recorded here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) loosely.

For releases prior to v0.3.2, see the git log and `ROADMAP_COMPLET.md`.

---

## [v0.8.90] — 2026-06-15

### Changed
- Télémétrie : le ping d'usage détecte désormais le client HTTP
  disponible avant d'appeler — `curl` en priorité, repli sur `wget`,
  et si aucun des deux n'est installé le ping est sauté en silence.
  amc compile toujours normalement (best-effort, non bloquant, sans
  libcurl). Opt-out inchangé : `AMALGAME_NO_TELEMETRY=1` / `DO_NOT_TRACK=1`.

## [v0.8.89] — 2026-06-15

### Added
- MCU bare-metal target — build/flash/debug Amalgame sur Cortex-M.
- Runtime MCU : `List<T>` adossé à une arena (embedded).
- Télémétrie d'usage anonyme : amc envoie au build/run un ping
  {version, os, arch} à tel.amalgame.me. Aucun identifiant, aucune IP
  stockée. Opt-out : `AMALGAME_NO_TELEMETRY=1` ou `DO_NOT_TRACK=1`.

### Changed
- `install.sh` télécharge les assets via get.amalgame.me (miroir
  souverain, compté) avec repli GitHub.
- Docs : auto-déploiement zéro-Node, sync ROADMAP, liens corrigés.

---

## [v0.8.88] — 2026-06-08

Two Windows-native build fixes uncovered porting FF-TAROT.

### Fixed

- **Bundled stdlib `math` facades were never shipped.** The release
  staging copied `json/toml/msgpack/path/amc_buildinfo` into
  `share/amalgame/stdlib/` but omitted `math.am` and `math_vec.am`,
  even though the bundled-stdlib auto-`--external` catalog
  (`Amalgame.Math|math.am`) lists them. Result: user code doing
  `import Amalgame.Math; Math.Floor(x)` resolved `Math` against the
  caller's own namespace (`App_Math_Floor`) instead of
  `Amalgame_Math_Math_Floor`, and the link failed with an undefined
  reference. Local dev builds masked it because `build_amc.sh` installs
  the full `src/stdlib/` tree. Now all four staging blocks ship math.

- **Transitive deps could pull an older version over a cached newer
  one.** When a parent package's `[dependencies]` lists a dep by short
  form (`amalgame-tls = ">=0.3.1"`), `amc package add` auto-resolved and
  installed it via the packages-index — even when a newer, already-added
  version satisfied the constraint. A stale index then dragged in an
  older facade alongside the newer one (two versions of the same runtime
  header; on Windows the older, un-ported one broke the build).
  Transitive resolution now reuses an already-cached version of the dep
  instead of re-resolving.

---

## [v0.8.75] — 2026-06-03

Tooling + distribution release. The compiler is functionally unchanged
since v0.8.74; this cuts a versioned release for the new batteries-
included Docker IDE image and the Mosaic documentation work.

### Added

- **Docker IDE image** (`ghcr.io/amalgame-lang/amalgame-ide`). One
  command — `docker run -p 8080:8080 …` — boots a browser VS Code
  (code-server) that opens straight into a working Amalgame setup: the
  extension pre-installed (syntax highlighting, LSP via `amc lsp`, DAP
  via `amc dap`), an F5-ready `MyFirstApp` sample (pre-built release +
  debug), the full user guide pinned in the sidebar, and `amc` + gcc +
  gdb on `PATH` so build/run/debug all work offline. Built and published
  by `.github/workflows/docker.yml` as a multi-arch manifest
  (linux/amd64 + linux/arm64, the arm64 leg native on `ubuntu-24.04-arm`,
  no QEMU). PRs touching `docker/**` build and runtime-smoke the image
  (entrypoint + code-server health) before merge.

### Docs

- Mosaic web-framework documentation: a dedicated guide sub-section
  (`mosaic serve`, deploy models, `[[proxy]]` schema), a full FR
  translation of the Configuration reference, and the `mosaic service`
  command (install a site as a systemd/launchd/SCM service).

## [v0.8.69] — 2026-05-31

TypeChecker performance — the expression-type memo was a linearly
scanned parallel-list pair (O(n²)); a hashed map makes it O(1). This is
the third and final O(n²) in the `amc --check` pipeline: a full check of
`c_gen.am` (5k LoC) now runs in **~0.2 s**, down from ~118 s at the
start of the v0.8.67–v0.8.69 perf run.

### Performance

- **TypeChecker expression-type memo was O(n²).** `SetType`/`GetType`
  stored each expression's inferred type in a pair of parallel lists
  (`ExprTypeKeys`/`ExprTypeVals`) keyed by `NodeKey`, and both did a
  **linear scan over every previously-recorded entry** on each call
  (Set even appended duplicates). With one memo entry per typed
  expression, that is O(typed-exprs) per access → O(n²) over a file —
  the same anti-pattern the resolver's `MemberTable`/`GlobalNames`
  shed in v0.6.x. Replaced the two lists with a single
  `Map<string, string>` (`ExprTypeMap`); `Map.Set` overwrites in place,
  preserving the original "last write wins" semantics, and both
  accessors are now O(1).

  Measured (typecheck phase, `amc --check`):

  | File          | typecheck before | typecheck after |
  |---------------|------------------|-----------------|
  | `c_gen.am`    | ~1.12 s          | **28 ms**       |
  | `parser.am`   | ~379 ms          | **14 ms**       |
  | `token.am`    | ~33 ms           | **6 ms**        |

- **Cumulative result of the perf run (v0.8.67 + v0.8.68 + v0.8.69).**
  `amc --check src/generator/c_gen.am` end-to-end:

  | Phase     | session start | now      |
  |-----------|---------------|----------|
  | parse     | ~12.8 s       | ~0.13 s  |
  | resolve   | ~103 s        | ~5 ms    |
  | typecheck | ~1.7 s        | ~35 ms   |
  | **wall**  | **~118 s**    | **~0.2 s** |

  All three were O(n²): diagnostics snippet formatting (v0.8.67), the
  lexer's per-char strlen (v0.8.68), and the typecheck memo scan
  (v0.8.69). The big-file LSP responsiveness this unlocks (hover /
  on-edit checking) was the original motivation. Full suite green:
  core 368 + stdlib 212 (+5 skip) + fmt 12 = 592 PASS / 0 FAIL.

---

## [v0.8.68] — 2026-05-31

Lexer performance — the parse phase was O(n²) in source size; large
files now lex in milliseconds. With v0.8.67 (resolve) this takes a full
`amc --check` of `c_gen.am` (5k LoC) from ~118 s to ~1.3 s.

### Performance

- **Lexer was O(n²) in file size.** `code_string` is a bare `char*`, so
  `String_Length` is an `strlen` (O(n)) and `String_Substring` runs an
  internal `strlen` for bounds. The lexer called `String_Length(
  this.Source)` in every per-character loop condition, and `CharAt`
  fetched each character via `String_Substring(src, i, 1)` — so every
  one of the ~n characters paid an O(n) strlen, making tokenisation
  O(n²). Two fixes:
  - The lexer caches `strlen(Source)` once into `SourceLen` (the source
    is immutable after construction) and uses it for all bound checks.
  - New runtime primitive `String_CharAtUnchecked(s, i)` reads `s[i]` in
    O(1) (via the pre-allocated 1-char table, no strlen); `CharAt` uses
    it after its own `i < SourceLen` bound check, so it stays safe.

  Measured (parse phase, `amc --check`):

  | File          | parse before | parse after |
  |---------------|--------------|-------------|
  | `c_gen.am`    | ~10.8 s      | **0.10 s**  |
  | `parser.am`   | ~1.0 s       | **0.04 s**  |
  | `token.am`    | ~95 ms       | **17 ms**   |

  Combined with the v0.8.67 diagnostics fix, `amc --check c_gen.am` now
  spends 100 ms parse + 3 ms resolve + ~1.15 s typecheck (≈1.3 s total,
  was ~118 s). Full suite green: core 368 + stdlib 212 (+5 skip) +
  fmt 12 = 592 PASS / 0 FAIL.
- `String_CharAtUnchecked` trusts the caller for bounds (it skips the
  strlen), so it is intentionally scoped to the lexer's checked `CharAt`.
  The general-purpose `String_CharAt1` (used by the typechecker, TOML
  parser, etc.) is unchanged; those call sites can adopt the cached-
  length pattern in a follow-up if they show up in a profile.

---

## [v0.8.67] — 2026-05-31

Resolver/diagnostics performance — error-heavy `amc --check` (and LSP
mid-edit) goes from tens of seconds to milliseconds.

### Performance

- **`SourceMap.GetLine` was O(filesize) per call → O(errors × filesize)
  overall.** Every diagnostic renders a source snippet, which fetched
  the offending line via `NthLine` — a character-by-character walk of
  the whole file text that allocated a fresh 1-char string per
  character. With one `GetLine` per diagnostic, an error-heavy resolve
  (a file `--check`ed in isolation without its imports, or a file with
  genuine errors open in the LSP) degraded quadratically. `GetLine` now
  splits each file into lines once (lazily, on first use) and caches the
  result, so it is O(1) amortised. Measured on the compiler's own
  sources (`amc --check`, isolation → many unknown-symbol errors):

  | File          | resolve before | resolve after |
  |---------------|-----------------|---------------|
  | `c_gen.am`    | ~103–168 s      | **3 ms**      |
  | `parser.am`   | ~15 s           | **4 ms**      |
  | `lexer.am`    | ~27 s           | **3 ms**      |

  The resolver algorithm itself was already O(1)-lookup throughout; the
  cost was entirely in the diagnostic snippet formatter. Error output
  (the rustc-style caret snippet) is byte-for-byte unchanged; the
  TypeChecker shares the same `SourceMap` and benefits identically. Full
  suite green: core 368 + stdlib 212 (+5 skip) + fmt 12 = 592 PASS /
  0 FAIL.
- Not addressed here: the parse phase is independently slow on very
  large files (~10 s on `c_gen.am`); that has a separate root cause and
  is tracked for a follow-up.

---

## [v0.8.66] — 2026-05-31

Deep member-chain dispatch — `obj.Field1.Field2.CollectionMethod()` now
lowers correctly instead of emitting an undefined symbol.

### Fixed

- **Deep member chain `obj.F1.F2.CollectionMethod()`.** A 2+-level field
  chain ending in a List/Map/Set method collapsed the last field name
  into the method name: `app.Routes.Routes.Count()` emitted
  `app->Routes->Routes_Count()` (undefined → link/compile failure)
  instead of `AmalgameList_count(app->Routes->Routes)`.
  `TryEmitListCall` only resolved single-hop receivers (`this.F` /
  `ident.F`); a chain whose `callee.Left.Left` is itself a `MEMBER` fell
  through to the broken generic dispatch. The fix resolves such
  receivers generically via `InferTypeFromExpr` (the chain's C type) and
  `EmitExprStr` (the `a->b->c` accessor), covering List (Case 5) and
  Map/Set (rxExpr/rxType). The earlier "identical-names collapse"
  diagnosis (`.Routes.Routes`) was a misdiagnosis — distinct names
  (`w.Inner.Vals`) failed identically; the cause was chain *depth*, not
  name equality.
- Covered by `tests/samples/deep_member_chain.am` (3 cases: list with
  distinct field names, list with identical field names, deep Map
  chain). Full suite green: core 368 + stdlib 212 (+5 skip) + fmt 12 =
  592 PASS / 0 FAIL.
- Known limitation (orthogonal, out of scope): a single-uppercase-letter
  class name (`class B`) still lowers to `void*` via the
  generic-type-parameter heuristic in `TypeToC`.

---

## [v0.8.65] — 2026-05-31

Variadic constructors across a package boundary — the residual that
closes the variadic saga (v0.8.61 params/call-sites → v0.8.62 external
methods → v0.8.63 local constructors → **v0.8.65 external constructors**).

### Fixed

- **Variadic constructor cross-package registration.** A class shipped
  by an `--external` facade with a variadic constructor
  (`public VBag(...nums: int)`) was not registered in the
  `MethodVariadic` table on the consumer side: `ExternalMethodSig`
  guarded the registration behind `if (!isCtor)`, so consumer call sites
  (`new VBag(1, ...xs, 2)`) emitted each argument positionally instead of
  gathering the variadic tail into one `AmalgameList*` — producing
  broken C against the facade's single-`AmalgameList*` ctor parameter.
  `ExternalMethodSig` now registers the variadic ctor under
  `<mangledClass>_new` (the exact key the `NEW_EXPR` call site already
  looks up via `ctorPrefix + "_new"`), mirroring the in-bundle ctor
  registration in `EmitConstructorForwards`. Local-class variadic ctors
  (v0.8.63) were already covered; this extends them across the package
  boundary, the external-facade analog of v0.8.62.
- Covered by `tests/samples/variadic_ctor_external/` (facade + consumer,
  4 cases via the `RunExternalPair` core-bundle helper: inline, mixed
  inline/spread/inline, zero args, fixed param before the variadic
  tail). Full suite green: core 365 + stdlib 212 (+5 skip) + fmt 12 =
  589 PASS / 0 FAIL.

---

## [v0.8.64] — 2026-05-31

Primitive `.ToString()` — `n.ToString()` on an `int` / `float` / `bool`
now works instead of failing at link time.

### Fixed

- **`<primitive>.ToString()` lowering.** Calling `.ToString()` on a
  primitive receiver previously emitted a non-existent `i64_ToString(n)`
  (resp. `double_`/`code_bool_`) — code that passed `amc --check` but
  failed at the link step with `undefined reference to 'i64_ToString'`.
  `EmitCalleeStr` now special-cases `ToString` on a primitive receiver
  and returns the matching runtime converter, so `n.ToString()` lowers
  to `String_FromInt(n)` (the receiver is passed as the converter's
  first argument). Covers `int → String_FromInt`,
  `float → String_FromFloat`, `bool → String_FromBool`, for both a typed
  variable receiver (`s.ToString()`) and a chained-call receiver
  (`f(x).ToString()`). The idiomatic `String_FromInt(n)` still works.
- Covered by `tests/samples/primitive_tostring.am` (6 cases: int/float/
  bool × variable/chain). Full suite green: combined 629 PASS / 0 FAIL.

## [v0.8.63] — 2026-05-31

Variadic **constructors** — extends v0.8.61/62's variadic params to the
`new X(...)` path, the remaining gap in the variadic story.

### Added

- **Variadic constructor parameters.** A constructor's trailing
  parameter may now be variadic — `public Bag(...nums: int)` — exactly
  like a method's. The param lowers to a single `AmalgameList*` in both
  the `Name_new` forward declaration (`EmitConstructorForwards`) and the
  definition, mirroring `MethodSig`. The constructor is registered in the
  `MethodVariadic` table under `Name_new` so call sites gather correctly.
- **Variadic `new X(...)` call sites.** The `NEW_EXPR` codegen now
  consults the `MethodVariadic` table and reuses `EmitVariadicTail`:
  `new Bag(1, 2, 3)` gathers the trailing args into a fresh list,
  `new Bag(...xs)` splices an existing list, `new Bag(1, ...xs, 2)` mixes
  the two, and `new Bag()` yields an empty (non-NULL) list. Fixed params
  may precede the variadic one (`new Tagged("hi", 7, 8, 9)`).
- **Parser: spread in `new` argument lists.** `ParseNew` now recognises a
  leading `...` argument (`TokenType.OP_SPREAD`) and wraps it in the same
  `UNARY op="..."` shape as `ParseCallArgs`, so the cgen collector reuses
  the splice loop. Previously `new X(...xs)` emitted `_unknown_`.
- Covered by `tests/samples/variadic_ctor.am` (6 cases: zero, inline,
  spread, mixed, fixed + variadic, fixed + zero). The formatter
  round-trips the `...` marker on constructors for free (constructors are
  methods). Full suite green: core 355, stdlib 212 (+5 skip), fmt 12.

### Note

Variadic constructors are covered for **locally-defined classes**; a
variadic ctor exposed across a package boundary (the external-facade
analog of v0.8.62) is the residual follow-up.

## [v0.8.62] — 2026-05-31

Variadic parameters across a package boundary — follow-up to v0.8.61's
residual scope.

### Added

- **Variadic params on external-package facade methods.**
  `ExternalMethodSig` now lowers a variadic facade param to a single
  `AmalgameList*` (matching the in-bundle `MethodSig` lowering) and
  registers the `MethodVariadic` table under the package-mangled
  callee name (`Ns_Class_Method`). A consumer calling
  `Pkg.Foo(1, ...xs, 2)` therefore builds the gathered `AmalgameList*`
  at the call site, which matches the facade body compiled separately
  via `MethodSig`. Previously only locally-defined classes were
  covered.
- New core-bundle test helper `RunExternalPair`: compiles a facade and
  its consumer in two separate amc invocations (consumer with
  `--external`), links both `.c` files, and greps the output — so an
  ad-hoc facade can be tested without precompiling it into
  `libamalgame.a`. Covered by `tests/samples/variadic_external/`
  (3 cases: inline, spread, fixed + mixed).

## [v0.8.61] — 2026-05-31

Variadic parameters + variadic call sites — closes the last "still
pending" item on the spread-operator backlog (the list-literal spread
MVP shipped v0.8.36).

### Added

- **Variadic parameters** — a method's trailing parameter may be
  variadic: `Sum(...nums: int)`. Callers pass zero or more `int` args
  in that position; inside the body `nums` is a `List<int>`, so all
  the usual list machinery (`for n in nums`, `nums.Get(i)`,
  `nums.Count()`, `.Map`/`.Filter`, …) works unchanged. Lowers to a
  single `AmalgameList*` C parameter — no `va_list`. Fixed params may
  precede the variadic one (`SumFrom(base: int, ...nums: int)`).
- **Variadic call sites** — `f(a, b, c)` gathers the trailing args
  into a fresh list for the callee's variadic param, and `f(...xs)`
  splices an existing `List` into that position (same AST shape as a
  list-literal spread item). The two compose freely:
  `Sum(1, ...xs, 2)`. Zero trailing args produce an empty (non-NULL)
  list.

### How it lowers

- Parser: `ParseParam` consumes a leading `OP_SPREAD` and flags the
  PARAM node; `ParseCallArgs` wraps a `...expr` argument in the
  established `UNARY op="..."` node.
- CGen: a new `MethodVariadic*` table records each variadic method's
  mangled C name → fixed-param count (populated in the forward-decl
  pass). At a CALL site the fixed args emit positionally, then
  `EmitVariadicTail` gathers the rest into an `AmalgameList*` via the
  same boxed-add / spread-splice loop the list-literal emitter uses.
  `MethodSig`/`EmitMethod` emit the variadic param as `AmalgameList*`
  and seed elem-type tracking (`List<T>`) so in-body access casts
  correctly.
- Formatter: variadic params re-emit with the `...` marker
  (`...int nums`); round-trip is idempotent.

Covered by `tests/samples/variadic_params.am` (8 cases: zero args,
inline args, fixed+variadic, fixed+zero, spread, mixed inline+spread,
fixed+spread, variadic strings), wired into the core bundle.

## [v0.8.60] — 2026-05-30

Compiler internals refactor + three parser fixes.

### Refactored

- All seven `NodeKind`/keyword dispatchers in the self-hosted compiler
  moved from `if (k == NodeKind.X) … else if …` chains to `match`:
  `ResolveStmt`, `ResolveExpr` (resolver); `EmitStmt`, `EmitExprStr`,
  `InferTypeFromExprUncached` (cgen); `ParseStmt`, `ParseDecl` (parser).
  Behaviour-preserving — compound branches became guarded arms; arms that
  don't `return` fall through to the post-`match` default exactly as the
  if-chains did. The roadmap's "blocked on algebraic-enum patterns /
  match-arm guards" was a misdiagnosis: `NodeKind` is a simple enum.

### Added

- `match` now accepts **string-literal patterns** — `match v { "if" =>
  …, "while" => … }` lowers via `strcmp`. This unblocked converting the
  two keyword-string parser dispatchers above.

### Fixed

- Parser: a bare `;` statement separator no longer synthesises a spurious
  `_unknown_` node (`a(); b()`, `let x = 5;`, `;`-separated blocks). It is
  now treated as an optional statement terminator, equivalent to a
  newline. `amc fmt` round-trips `;` cleanly again.
- Parser: TS-style return-type signatures `Name(p): T { … }` no longer
  silently drop the entire method body (and no longer report the return
  type as `void`). `ParseMethod` consumes the trailing `: <type>`.

### Note

- The CHANGELOG was dormant between **v0.8.29 and v0.8.59**; those
  releases (async/HTTP stack, package ecosystem, DAP bridge, native TLS,
  resolver perf, `amc doc`, …) are catalogued in `ROADMAP_COMPLET.md` and
  the git history. This entry resumes the log.

## [v0.8.28] — 2026-05-17

Follow-up to v0.8.27 — AutoBUS Phase 4 (`subscription.am::FindByTopic`)
surfaced a silent miscompilation on `==` between a class-field-rooted
`List<string>.Get(i)` and a string variable.

### Fixed — Bug 10 (cgen): `obj.Field.Get(i) == s` emitted pointer compare

`InferTypeFromExpr`'s `.Get(...)` branch knew how to resolve the
element type for two shapes:

* `this.Field.Get(i)` — looked up via `ListElemGet(CurrentClass, Field)`
* `localList.Get(i)`  — looked up via `ListElemGet("__local__", localList)`

…but not the chained MEMBER receiver `obj.Field.Get(i)` where
`obj` is a local var of class type `C` and `C.Field: List<string>`.
That shape fell through to `void*`, and the `==`/`!=` handler
therefore couldn't see that the result was `code_string` — so it
lowered to raw C `==` (pointer comparison) instead of
`code_string_equals(...)`.

Worse, the failure was silent: under gcc's default string-literal
coalescing, two compile-time `"beta"` literals share an address and
pointer compare happens to match — masking the bug for typical
in-source repros. The break only surfaces when one operand is built
at runtime (concat, read, decode, …), which is exactly what
AutoBUS's topic-name lookup does on JSON-decoded subscriptions.

**Fix**: extend the `Get` branch of `InferTypeFromExpr` to
recognise `ll.Left.Kind == NodeKind.IDENTIFIER` on the
`expr.Left.Left` MEMBER receiver — resolve `obj`'s C type via
`LocalTypeGet`, strip `*`, and look up the element type with
`ListElemGet(objType, Field)`. The `==`/`!=` resolver now sees
`code_string` for the LHS and lowers to `code_string_equals`.

The same path covers `!=` and the LHS-on-the-right variant
(`localVar == obj.Field.Get(i)` → the existing RHS-literal check
already handled compile-time literals, and the new LHS resolution
catches the rest).

Regression test `tests/samples/bug10_list_string_eq.am` covers
four shapes: `obj.Field.Get(i) == localVar`, `localVar ==
obj.Field.Get(i)`, the `!=` mirror, and the previously-working
`localList.Get(i) == concatVar` form (to catch a future regression
on the wider inference path). All four match the runtime against
a `"be" + "ta"` concat so the test fails loudly if the cgen ever
slips back to pointer compare.

**Downstream**: AutoBUS's `subscription.am::FindByTopic` workaround
— a typed local between `topics.Get(j)` and the `==` — can be
reverted once AutoBUS adopts amc v0.8.28.

---

## [v0.8.27] — 2026-05-17

Follow-up to v0.8.26 — the AutoBUS downstream session discovered
that the Bug 6 fix unblocked method dispatch on bundled-stdlib
classes but left field reads broken.

### Fixed — Bug 9 (cgen): external struct field access hit "incomplete typedef"

The v0.8.26 auto-attach registered external classes via
`RegisterExternalProg`, which only emitted a forward typedef
`typedef struct _X X;` per class. Field-level reads on those
classes — the documented `Json.Parse` entry point pattern:

```amalgame
let r = Json.Parse(body)
if (r.Ok) {
    let root: JsonValue = r.Value
    …
}
```

…hit `error: invalid use of incomplete typedef 'Amalgame_Formats_Json_JsonResult'`
at the gcc step. The fields live in libamalgame.a's compiled
image, but the consumer's `.c` had no way to know the layout.

**Fix**: `RegisterExternalProg` now emits the full struct body
for every external CLASS_DECL (mirroring `EmitClass`'s layout)
and the full enum body for every external simple ENUM_DECL
(`typedef enum _M { … } M;`). Fields lower through `TypeToC`
exactly like in-bundle classes, so sibling references resolve
to the bundled namespace mangling instead of the consumer's
`NsPrefix`.

New `ExternalEnums` / `ExternalEnumNsArr` parallel registries
back the enum lookup — checked by `TypeToC` right after
`ExternalClassMangled` so types like `JsonValue.Kind: JsonKind`
(a sibling enum reference inside the struct body) resolve to
the bundled mangled name as a value type, not a pointer.

The struct body emission registers each field via
`FieldTypeSet(mangled, fieldName, ctype)` so chained
`r.Field.Method()` dispatch through the cgen's `FieldTypeGet`
lookup gets a non-empty C type — keeps the chained-call paths
honest for external classes too.

Algebraic external enums (variants with payloads) still get
forward-only typedefs — they'd need the tagged-union machinery
`EmitEnum` runs in-bundle. The current bundled-stdlib catalog
(Json / Toml / MsgPack / Path / Compiler) uses simple enums
only, so this is fine.

### Tests

492/492 PASS (was 489 + 3 new):
- `bug9_external_struct_fields.am` — 3 cases covering
  `JsonResult.Ok` (bool field), `r.Value` (sibling-class
  pointer), `r2.Error.Message` (chained field across two
  external classes — JsonResult + JsonError), `JsonError.Line`
  (int field at non-zero offset).

The AutoBUS `@c {…}` shim in `Common/state/topic.am::LoadAll`
can now be replaced with the clean form:

```amalgame
let r = Json.Parse(content)
if (!r.Ok) { continue }
let root: JsonValue = r.Value
let t: Topic = Topic.FromJson(root)
```

---

## [v0.8.26] — 2026-05-17

Three downstream-reported bugs surfaced while building a JSON-schema
processing app on top of v0.8.25. Each one had a documented
workaround (rename the field, extract the length to a local, drop the
outer `List` annotation) — this release closes the underlying root
causes so the workarounds can come out.

### Fixed — Bug 6 (cgen): bundled-stdlib field types mangle to user NsPrefix

User code with its own namespace would declare a field like
`Definition: JsonValue` and watch the cgen emit
`MyApp_JsonValue* Definition;` (and `MyApp_Json_NullValue()` for the
construction call) instead of the bundled-stdlib's
`Amalgame_Formats_Json_JsonValue*` / `Amalgame_Formats_Json_Json_NullValue()`.
The struct typedef was never emitted under the wrong prefix, so gcc
bailed with `unknown type name 'MyApp_JsonValue'`; the documented
workaround was to switch the field to `string` and re-parse JSON on
each access.

**Root cause**: `amc build app.am` runs amc with only the user's
entry file as input. The cgen has no idea that `JsonValue` is a
bundled-stdlib class, so its `SymName("JsonValue")` falls through to
the user's `NsPrefix` (`MyApp_JsonValue`). Passing
`--external src/stdlib/json.am` already fixed it, but `amc build`
doesn't expose `--external` and the bare `amc -o` workflow doesn't
either for end users.

**Fix**: at compile start, amc scans the input files for
`import Amalgame.Formats.Json` / `.Toml` / `.MsgPack` / `Amalgame.Path`
/ `Amalgame.Compiler`. For each namespace the user imports, the
corresponding `.am` facade is auto-attached as `--external`,
delegating the registration to the existing `RegisterExternalProg`
path. `TypeToC` / `InferTypeFromExpr` / `EmitCalleeStr` then dispatch
to the bundled namespace automatically.

The facades live at `<bin>/../share/amalgame/stdlib/` for installed
amc (release.yml + install.sh + amalgame.iss now ship them) and at
`<bin>/src/stdlib/` for source-tree dev builds. `$AMC_STDLIB_SRC`
overrides for unusual layouts. Auto-attach skips:
- Namespaces declared by the user's own input files (so amc
  compiling its own `src/stdlib/json.am` doesn't double-register).
- Facades already passed via explicit `--external` or as primary
  inputs (de-dup by filename leaf so relative-vs-absolute path
  drift between caller and probe doesn't double-emit).

Both the canonical 4-part namespace AND the legacy 3-part shorthand
are accepted by the auto-attach scanner — `import Amalgame.Json`
resolves to the same `json.am` facade as `import Amalgame.Formats.Json`.
This keeps the Autobus downstream session (and `src/stdlib/msgpack.am`'s
own `import Amalgame.Json` doc-comment) working without forcing a
rename. The aliases apply to Json / Toml / MsgPack only.

### Fixed — Bug 7 (cgen): `+` with a `String_*`-callee operand misclassified as concat

`return pos + String_Length(needle)` lowered to
`return code_string_concat(pos, String_Length(needle));` because the
cgen's BINARY-`+` branch matched the RHS callee by name prefix —
`String_StartsWith(callee, "String_")` ⇒ assumed string-returning ⇒
dispatch to `code_string_concat`. But `String_Length`, `String_IndexOf`,
`String_LastIndexOf`, `String_ToInt` all return `i64`, so the
classification was wrong and the C compiled into pointer-arithmetic
on an integer (the workaround was to extract the call to a local).

**Fix**: BINARY-`+` now consults `InferTypeFromExpr` on both LHS and
RHS, which already has precise return-type knowledge for the
`String_*` family (see the explicit return-type table around c_gen.am
line 966). The name-pattern check is removed; the literal
`code_string_concat` callee stays in the trigger list because nested
concatenations do return `code_string`.

### Fixed — Bug 8 (parser): `List<List<int>>` rejected by ParseTypeName

The lexer fuses two consecutive `>` characters into a single
`OP_SHR` token (`>>`), so the parser's generic-bracket loop only
recognised single-character `>` for depth bookkeeping. For
`let xs: List<List<int>> = ...`, the loop never saw a closing `>`
and consumed the rest of the file looking for one, producing
`Expected '}' got ''` at EOF. Workaround was to drop the outer
annotation and rely on implicit `void*` cast on `xs.Get(i)`.

**Fix**: `ParseTypeName`, `ParseNew` and the lambda-param lookahead
all detect the `>>` token explicitly and treat it as two `>` for
depth bookkeeping. The inner closer(s) get appended to the type
string; the outermost one stays unappended (the post-loop
`name + ">"` re-emits it). Triple-nested (`>>>` = `>>` + `>`) and
quad-nested (`>>>>` = `>>` + `>>`) generics parse cleanly too.

### Tests

251/251 PASS (was 240 + 11 new):
- 5 cases in `bug7_int_plus_string_helper.am` cover the originally
  broken form plus the LHS mirror, plus three sanity cases (string +
  string, string + `String_FromInt`, `String_Length` + `String_Length`)
  that must NOT regress to plain `+`.
- 4 cases in `bug8_nested_generics.am` cover double-, triple-,
  quad-nested generics and `Map<K, List<V>>` (comma + nested).
- 2 cases in `bundled_stdlib_field_type.am` (a new `run_external_test`
  invocation with no `--external` flag) exercise the auto-attach
  path end-to-end: amc emits the right C, gcc links against
  `lib/libamalgame.a`, and the resulting binary reads/writes the
  JsonValue field via the bundled struct typedef.

---

## [v0.8.25] — 2026-05-17

Two follow-up fixes for issues surfaced by the Autobus downstream
project after v0.8.24 shipped.

### Fixed — Bug 5 mirror: `..` becomes a real binary operator

v0.8.24 fixed `for i in 0..xs.Count()` by changing the RHS of `..`
to parse via `ParsePostfix`, but the mirror form `xs.Count()..N`
still failed at parse time. `..` was only recognised inside
`ParsePrimary`'s integer + identifier branches, so a call
expression on the LHS exited `ParsePostfix` cleanly and left the
`..` dangling — the for-stmt then reported `Expected '{'` where
the range RHS should have been.

**Fix**: pull `..` out of `ParsePrimary` into a proper
binary-operator layer `ParseRange`, between `ParseAssign` and
`ParseOr` in the precedence ladder. Same shape as Rust's range
expression — just above logical_or, below comparison/additive.
Both LHS and RHS go through `ParseOr`, so any combination of
atoms, calls, member chains, arithmetic, etc. is fair game on
either side.

```
// Before: xs.Count()..N → parse error "Expected '{'"
// After:  xs.Count()..N → BINARY("..", CALL(MEMBER(xs, Count)), N)
```

Non-associative: `a..b..c` rejects the second `..` cleanly by
leaving it for the caller.

**Cases now accepted**:
- `0..xs.Count()` (already worked since v0.8.24, regression check)
- `xs.Count()..n` (mirror — the v0.8.25 fix)
- `lo.Count()..xs.Count()` (both sides are calls)
- `0..n+1` (arithmetic on RHS)
- `(a+b)..(c*d)` (full sub-expressions on both sides)

### Fixed — Bug 6: LSP `FindWorkspaceRoot` accepts `amalgame.toml`

Reported on the Autobus project (no `.git` at the project root,
manifest-driven layout). Opening `tests/wire/byteio_test.am` — a
file in a sub-directory of a project that only had an
`amalgame.toml` manifest at the root — surfaced 10+
`Unknown symbol 'ByteIO'` diagnostics in VS Code, even though the
build was green.

**Root cause**: `FindWorkspaceRoot` recognised `.git`,
`build_amc.sh`, and `package.json` as workspace markers but not
`amalgame.toml`. Autobus has no `.git` and no upstream marker
between `tests/wire/` and `/home`, so the walk fell back to the
file's own directory. The sibling scan only saw the 2 files in
`tests/wire/`, missing the actual class definition in
`Common/wire/byteio.am`.

**Fix**: add `amalgame.toml` to the marker list. Any Amalgame
project or package now anchors the workspace root at its
manifest, even without a `.git` directory.

### Tests

240/240 PASS (was 234 + 6 new):
- 2 new `for_range_method_call.am` cases: `xs.Count()..n` and
  `lo.Count()..xs.Count()` (mirror + both-sides-calls)
- 1 new LSP regression: `tests/fixtures/lsp-workspace/` (a
  manifest at root + `lib/byteio.am` + `tests/byteio_test.am`
  referencing it cross-dir) with a new `run_lsp_absent` helper
  asserting no `Unknown symbol 'ByteIO'` diagnostic is published.

Real-world verification on Autobus: pre-fix LSP on
`tests/wire/byteio_test.am` produced 10 `Unknown symbol 'ByteIO'`
diagnostics; post-fix produces zero.

## [v0.8.24] — 2026-05-17

Single-bug point release. Bug 5, reported mid-flight on the
v0.8.23 release by the same downstream project that surfaced
Bugs 1-4.

### Fixed — Bug 5: `for i in 0..xs.Count()` drops the method call

```amalgame
for i in 0..xs.Count() {       // ← was rejected by gcc
    sum = sum + xs.Get(i)
}
```

cgen emitted bogus C:

```c
AmalgameList* __it_i = 0 .. xs_Count();
i64 __len_i = AmalgameList_size(__it_i);
for (i64 __idx_i = 0; __idx_i < __len_i; __idx_i++) { ... }
```

gcc then rejected with `expected identifier before '.'`.

**Root cause** — parser, not cgen. `..` is parsed inline inside
`ParsePrimary`'s integer + identifier branches, with the RHS
pulled via `ParsePrimary()`. That consumes just the atom (`xs`)
and stops. The `BINARY("..", 0, xs)` was then handed up to
`ParsePostfix`, which saw the trailing `.Count()` and wrapped the
whole range in `CALL(MEMBER(range, Count))`. cgen's range
detector at `EmitStmt:FOR_IN_STMT` checks
`stmt.Left.Kind == BINARY && stmt.Left.Str == ".."` — with the
range wrapped in a CALL the check fails and emission falls into
the collection-foreach branch.

**Fix**: change the RHS parser from `ParsePrimary()` to
`ParsePostfix()` in both range-producing branches of
`ParsePrimary` (integer LHS and identifier LHS). `ParsePostfix`
descends into `ParsePrimary` for the atom then attaches the
`.method` / `(args)` / `[index]` suffix chain, so `xs.Count()`
is fully consumed before the BINARY is built.

```
// Before: 0..xs.Count() → BINARY("..", 0, xs) + dangling .Count()
// After:  0..xs.Count() → BINARY("..", 0, CALL(MEMBER(xs, Count)))
```

cgen then emits the counted loop cleanly:

```c
for (i64 i = 0; i < AmalgameList_count(xs); i++) { ... }
```

### Tests

3-case regression in `tests/samples/for_range_method_call.am`:
- `0..xs.Count()` (the original repro, asserts `sum=60`)
- `0..m.Size()` on a `Map<K,V>` (same parse shape, asserts iter count)
- `ident..xs.Count()` (mirror branch via identifier LHS)

237/237 PASS (was 234 + 3 new).

### Known follow-up — not in this release

The mirror form `xs.Count()..N` (call-on-LHS, atom-on-RHS) still
doesn't parse cleanly. `ParsePostfix` consumes `xs.Count()` and
leaves `..N` dangling because `..` isn't recognised at any
binary-op precedence level outside the two `ParsePrimary`
branches. Fix would mean moving `..` to a proper binary-op layer
(probably between `ParseAssign` and `ParseOr`). Tracked for a
follow-up release.

## [v0.8.23] — 2026-05-17

Four cgen + runtime bugs surfaced while compiling an external
Amalgame project against the v0.8.22 runtime. All four had local
workarounds documented in user code; this release removes the
need for any of them.

### Fixed — Bug 1: cgen multi-file forward decls

`amc a.am b.am` where `a.am` calls a method defined in `b.am`
tripped gcc's `-Wimplicit-function-declaration` followed by a
hard `conflicting-types` error. Pass 1 only emitted struct
typedefs across files; method + constructor forwards landed
inside Pass 2 file-by-file, so when `a.am`'s body was emitted,
`b.am`'s prototypes hadn't been written yet. Workaround was
reordering args (`amc b.am a.am`).

**Fix**: split `AddFilePass2` into `AddFilePass2Forwards` +
`AddFilePass2Bodies`. `main.am` and `gen_test.am` now run the
forward sweep across every file before any body. New
`EmitMethodForwards` covers non-constructor methods.

### Fixed — Bug 2: `this.field.Set / .Get / .Has / .Add` dispatch

The cgen specialized Map / Set / List methods only when the
receiver was a bare `IDENTIFIER` (`m.Set(k, v)`). When the
receiver was a class field — `this.entries.Set(k, v)` or
`obj.cache.Add(x)` — the dispatch fell through to the generic
method path which emits PascalCase (`AmalgameMap_Set`, undef).
Workaround was an intermediate local.

**Fix**: factor receiver resolution to handle three kinds —
`IDENTIFIER`, `MEMBER-of-THIS`, `MEMBER-of-IDENTIFIER` — and
reuse the same `rxExpr` / `rxType` for the Map and Set blocks.
Also track `Map<K,V>` *field* element types (already done for
`List<T>` fields) so `this.entries.Get(k)` casts to `V` instead
of `(void*)`.

### Fixed — Bug 3: `AmalgameMap_remove` poisons collision chains

Open-addressing linear probe + `_remove` marking the slot
`AMMAP_EMPTY` → colliding keys inserted *before* the remove
became unreachable. `Has(k)` returned false even though `Set(k,
…)` was called. Has-then-Get patterns then segfaulted.

**Fix**: 3-state slot (`EMPTY` / `OCCUPIED` / `TOMBSTONE`).
`_remove` marks `TOMBSTONE`; `_get` / `_has` skip tombstones;
`_set` reuses the first tombstone on the probe path. Load
factor counts size + tombstones so insert/remove churn grows
before probes degenerate. `_ammap_grow` rebuilds OCCUPIED-only.

### Fixed — Bug 4: `Map.Keys / Values` leak tombstoned entries

Discovered while reverting Bug 3 to confirm the regression
fired: even with the tombstone fix in `_set` / `_get` / `_has`
/ `_remove`, `_keys` and `_values` still emitted the dead
entries because they tested `if (e->used)` — and `TOMBSTONE`
(=2) is truthy in C.

**Fix**: switch both loops to `if (e->used == AMMAP_OCCUPIED)`.
Same 3-state constant the rest of the file already uses.

### Tests

234/234 PASS (was 232 + 9 new regression cases):

- `map_tombstone.am` — insert 50 keys, remove 25, assert all
  remaining findable, sum = 925, `Keys().Count() == 25`,
  `Values().Count() == 25`.
- `field_map_dispatch.am` — Map / Set / List fields on a class,
  full `Set` / `Has` / `Get` / `Remove` / `Add` / `Size` /
  `Count` surface.
- `multi_file_bug1/{main,helper}.am` + new `run_multi_test`
  harness with `-Werror=implicit-function-declaration` so the
  Bug 1 regression fails hard if it reappears.

## [v0.8.22] — 2026-05-17

`Process` v3 — streaming `Spawn` with a persistent handle.

### Added: `Process.Spawn(cmd, captureStreams) → AmalgameProcessHandle*`

Unlike `Process.Run` / `RunCapture` (synchronous, block until
done) and v2's `RunCaptureBoth*` (synchronous, capture full
buffers), `Spawn` gives you a **persistent handle** to the
child process that the parent can drive line-by-line in
parallel:

```kotlin
let h = Process.Spawn("tail -f /var/log/app.log", 2)  // bit 1 = pipe stdout
while (Process.IsAlive(h)) {
    let line: string = Process.ReadLine(h, 1000)      // 1s timeout
    if (line == "__EOF__") { break }
    if (String_Length(line) > 0) {
        Console.WriteLine("LOG: " + line)
    }
}
Process.Wait(h, 0)   // 0 = wait forever
```

`captureStreams` is a bit-mask:
- bit 0 (1) → pipe stdin (`WriteLine`)
- bit 1 (2) → pipe stdout (`ReadLine`)
- bit 2 (4) → pipe stderr (`ReadErrLine`)
- pass `7` to pipe all three; pass `0` to let the child inherit
  the parent's stdio (background scripts that don't need IPC)

### Full v3 surface

| Method | Returns | Notes |
|---|---|---|
| `Process.Spawn(cmd, captureStreams)` | `AmalgameProcessHandle*` | Fork+exec via `/bin/sh -c`, non-blocking |
| `Process.IsAlive(h)` | `bool` | Non-blocking probe (`waitpid WNOHANG`) |
| `Process.ExitCode(h)` | `int` | `-1` until reaped |
| `Process.Wait(h, timeout_ms)` | `bool` | `timeout ≤ 0` blocks forever |
| `Process.Kill(h)` | `void` | SIGTERM (graceful) |
| `Process.KillForce(h)` | `void` | SIGKILL (hard) |
| `Process.WriteLine(h, text)` | `bool` | Appends `\n` if missing; non-blocking |
| `Process.ReadLine(h, timeout_ms)` | `string` | `""` on timeout, `"__EOF__"` on stream close |
| `Process.ReadErrLine(h, timeout_ms)` | `string` | Same shape for stderr |

### Implementation

POSIX-only initially (`fork` + `pipe(2)` × {0..3} + `select(2)` +
`waitpid` + `kill`). Parent-side pipes are `O_NONBLOCK` so
`ReadLine` / `WriteLine` never block past their timeout.
Per-stream line accumulator handles partial reads (a `read()`
that returns half a line stays buffered until the next call
completes the newline).

Windows path (`CreateProcess` + Named Pipes for stdin/stdout/stderr
+ `WaitForMultipleObjects`) lands in v3.1 — the current Windows
fallbacks return empty handles / sentinels so user code compiles
cross-platform but only the POSIX flow is functional.

### Out of scope (v3.x)

- Windows native CreateProcess + Named Pipes implementation
- Binary read (current `ReadLine` is text/newline-oriented; for
  binary IPC bytes use Channel via `amalgame-threading` as the
  inter-thread fanout instead)
- Async I/O via callbacks (composable today with Threading's
  `ThreadSpawn` + the streaming `ReadLine` loop)
- Process tree management (parent → many children + supervisor)

### Tested

- 5 new test cases in `tests/run_tests.sh` against
  `tests/samples/process_v3.am` (spawn alive / 3 stream lines /
  EOF detection / exit code 0)
- Full suite: **461/461 PASS** (+5 vs v0.8.21 → 223+191+12+35)
- Snapshot updated

## [v0.8.21] — 2026-05-17

Parser quality-of-life — multi-line `&&` / `||` continuation.

### Fixed: multi-line `&&` / `||` continuation in parser

`ParseAnd` and `ParseOr` ended their loops on the first NEWLINE
token, so:

```kotlin
if (cond1
        && cond2
        && cond3) {
    ...
}
```

silently dropped every condition past the first `&&` (parser
emitted a `_unknown_` placeholder for the right operand). Same
for `||`. The fix mirrors v0.8.14's `+ - * / %` continuation:
each operator's parse loop now peeks past NEWLINEs and continues
when the next non-blank token is the matching operator. `expr &&
\n   next` (operator at EOL) was already accepted thanks to the
existing `SkipNewlines` after operator consumption — only the
"newline-then-operator" shape needed the fix.

Surfaced during the v0.2 + Kafka + RabbitMQ test-fixture work this
week: every `if (a\n   && b)` chain had to be folded into
`let aOk: bool = ...; let bOk: bool = ...; if (aOk && bOk)`
intermediates. Those workarounds can now be removed at leisure.

### Added: regression sample `tests/samples/multiline_logical.am`

Four shapes exercised: newline-before-`&&`, trailing-`&&`,
newline-before-`||`, mixed `&&`+`||` across lines. Wired into
`tests/run_tests.sh` as 4 new test cases (total 218 PASS).

## [v0.8.20] — 2026-05-16

LSP UX fix.

### Fixed: `Unknown symbol '<ClassName>'` when developing in a package repo

When developing INSIDE an amalgame-lang package's own repo (e.g.
editing `tests/stdlib_pg.am` in `amalgame-database-postgresql/`),
the LSP flagged every call to the package's own class
(`PostgreSQL.Open(...)`, `Image.Load(...)`, `NATS.Open(...)`, …)
as `Unknown symbol`. The resolver only saw the workspace's `.am`
files; the package's manifest, which is where the class is
exposed via `[stdlib].class`, was never read by `amc lsp`.

Fix: `BuildWorkspaceResolver` now calls a new
`LspServer.DetectLocalPackageClass(root)` helper that:
- Probes `<workspace_root>/amalgame.toml` for an `[stdlib]` table
- Pulls the `class = "Foo"` value with a small line-based scan
  (no TOML-parser dep — keeps the LSP startup lightweight)
- `resolver.DeclareGlobal(name, "type", false)` so the class is
  visible as a type-global before `ResolvePrograms()` runs

The fix is namespace-aware (only triggers inside `[stdlib]` /
`[stdlib.functions]`) so a stray `class = "Foo"` anywhere else
in the manifest is ignored. Block-comment lines aren't parsed
(rare in TOML).

Validated end-to-end via a synthetic LSP session: PG test file
no longer emits `Unknown symbol 'PostgreSQL'`; injecting a real
unknown symbol (`TotallyNotASymbol`) still surfaces correctly.

### Tested

- 452/452 PASS local (214 core + 191 stdlib + 12 fmt + 35 amc-new)
- Snapshot regenerated

## [v0.8.19] — 2026-05-16

Bugfix release — two compiler issues surfaced by the
`amalgame-database-postgresql` CI end-to-end run.

### Fixed: lexer — `namespace` keyword inside `//` comments leaks into the prefix

`main.am` pre-extracts the namespace via `String_IndexOf(firstSrc, "namespace ")`
on the raw source to feed `nsPrefix` into the cgen + the
self-package skip loop. Comments weren't being skipped, so a
line like `// schema namespace foo bar` placed before the real
`namespace App` declaration matched first and `foo bar` leaked
into every generated C identifier (`typedef struct _foo bar_Program …`).

Fix: replace the raw `IndexOf` with a line-by-line scan that
trims and skips `//` lines before the keyword check. Block
comments `/* … */` aren't skipped here yet (rare in practice; can
be added if it ever bites). Same patch adds the missing
`: List<string>` annotation on the `String_Split` result so the
new code itself doesn't trigger the second bug below.

### Fixed: cgen — opaque-`void*` receiver lowered `.Get()` to `void_Get(…)`

When a cross-package call like `PostgreSQL.QueryAll` returned
`List<List<string>>`, the inner-cell type erased to `void*`
because nested generics aren't propagated through the type
registry. A follow-up `row0.Get(j)` then hit the MEMBER-call
dispatch which built the symbol as `<bareType>_<mname>` →
`void_Get(row0, j)` → linker undefined-reference.

Fix: intercept `bareType == "void"` and dispatch known list
verbs (`Get` / `Count` / `Size` / `Add` / `Remove` / `Clear` /
`IndexOf` / `Contains`) through the `AmalgameList_<method>`
runtime with the same camelCase-first-letter trick the
chained-CALL branch uses. Doesn't solve the deeper
nested-generics-in-type-registry problem, but unblocks the
common `List<List<X>>` → row → cell pattern without forcing
explicit annotations at every intermediate `let`.

### Tested

- 452/452 PASS (214 core + 191 stdlib + 12 fmt + 35 amc-new)
- Snapshot regenerated
- `amalgame-database-postgresql` test fixture compiles cleanly
  without the previously-needed comment-rewording + List<string>
  workarounds; CI green end-to-end against `postgres:16-alpine`
  service container

## [v0.8.18] — 2026-05-16

Stdlib feature release: `Process` v2 surface.

### Added: split stderr/stdout + timeout-aware Process variants

Three new entry points on the `Process` runtime module sit alongside
the v1 `Run` / `RunCapture` (unchanged):

| Function | Behaviour |
|---|---|
| `Process.RunCaptureBoth(cmd)` | Real stderr/stdout split via `fork` + `pipe(2)`×2 on POSIX, `CreateProcess` + `CreatePipe`×2 on Windows |
| `Process.RunCaptureBothTimeout(cmd, ms)` | Same split + `SIGKILL` / `TerminateProcess` after `ms` |
| `Process.RunTimeout(cmd, ms)` | Like `Run` + timeout, no capture |

The timeout sentinel exit code is **124** (matches GNU `timeout(1)`).

v1 used `popen` with shell `2>&1` to merge streams into a single
`Stdout`, so `result.Stderr` was always `""`. v2 wires real pipes
so the two streams arrive separately. The `AmalgameProcessResult`
shape is unchanged — the `Stderr` field just becomes useful for
callers that opt into the v2 functions.

POSIX path drives a `poll(2)` loop with non-blocking reads and
`waitpid(WNOHANG)` per round; Windows path drives a
`WaitForSingleObject(50ms)` loop with `PeekNamedPipe`-gated
`ReadFile`. Shared scaffolding: `_am_buf_append` for growable GC
buffers, `AMALGAME_PROCESS_TIMEOUT_EXIT` macro for the sentinel.

Resolver declares the three new globals; cgen knows their return
types (`AmalgameProcessResult*` for the two capture variants,
`i64` for `RunTimeout`). No changes to v1 callers — fully backward
compatible.

Sample at `tests/samples/process_v2.am` exercises split stderr,
timeout kill, sub-budget completion, and `RunTimeout` without
capture.

**Out of scope**: `Process.Spawn(cmd)` → handle persistant with
`ReadLine` / `IsAlive` / `Kill` (streaming async model) is tracked
as Process v3 backlog. Pose des questions de concurrence
vis-à-vis bdwgc; déféré tant qu'un consumer réel n'en a pas besoin.

## [v0.8.17] — 2026-05-16

Single-fix patch release.

### Fixed: macOS CI build (clang inline-function-in-function-body)

The cross-platform release pipeline failed on `Build (macOS arm64)`
with 20 errors of `function definition is not allowed here` from
`<libkern/OSByteOrder.h>`. Root cause: `src/main.am` included
`<mach-o/dyld.h>` inside a function-scope `@c { … }` block for
`ResolveSelfPath` — that header transitively pulls in
`OSByteOrder.h` which declares inline functions at top level,
which clang refuses to (re-)declare inside a function body.
gcc/glibc accepts the same pattern because Linux uses macros
rather than file-scope inlines.

Fix: hoist the `#include <mach-o/dyld.h>` + `#include <stdlib.h>`
into the existing file-scope `@c { … }` block at the top of
main.am, gated by `__APPLE__`. The local `@c` body in
`ResolveSelfPath` keeps a comment pointer.

Same fix shape applies to any future macOS-specific header that
defines inline functions — keep system header includes at file
scope.

tests/run_all_tests.sh: 451/451 green.

## [v0.8.16] — 2026-05-16

Single-fix patch release.

### Fixed: chained method calls on `new X(...)` receivers

After v0.8.15 cleared the type-inference cascade for long fluent
chains, a related pattern was still broken at the codegen level:

```kotlin
let a = new Widget("foo").Class("bar")
let b = new Widget("baz").Class("a").Style("red")
let o = new Other().Set(new Widget("zzz"))
```

emitted invalid C — the method name was mashed into the
constructor call as a suffix
(`App_Widget_new("foo")_Class("bar")`) because neither
`EmitCalleeStr` nor `InferTypeFromExpr` had a case for a
NEW_EXPR (or CALL-on-NEW_EXPR) receiver of a method call.

Three additions in `src/generator/c_gen.am`:

1. `EmitCalleeStr` — new branches for `lk == NEW_EXPR` and
   `lk == CALL`. The NEW_EXPR branch resolves the method
   symbol via the same dispatch as a static `ClassName.Method()`
   call (external mangled / core stdlib / package mangled /
   `SymName` fallback). The CALL branch infers the inner
   receiver's return type and emits `<bare>_<mname>`.
2. `InferTypeFromExpr` — new branch for `llk2 == NEW_EXPR`.
   Mirrors the existing CALL branch so chains like
   `new X().M1().M2()` know the type after `M1()`.
3. Call-emission site — receiver detection adds the
   `ll.Kind == NEW_EXPR` case so the new'd instance is
   forwarded as the first arg, mirroring `varName.Method()`.

Documented workarounds in `amalgame-ui-web` test spikes (e.g.
the `BuildLabeledStack()` static helpers that split
`new LabeledInput(...).Render()` across `let` intermediates)
can be retired starting with this release.

## [v0.8.15] — 2026-05-16

Single-fix patch release.

### Fixed: `InferTypeFromExpr` chain-length hang

amc hung indefinitely on long fluent chains — ui-web v0.0.5+
apps past ~24 `.AddChild(...)` calls in a single expression
never finished compiling. `InferTypeFromExpr` recurses on
`expr.Left` for every CALL/MEMBER, which on a chain like

```
a.AddChild(x).AddChild(y).AddChild(z)...AddChild(zzz)
```

re-infers the entire prefix at every step — N calls produce
O(N²) re-visits, plus per-visit work made it superquadratic in
practice.

Fix: cache the inference by `AstNode` identity (C pointer cast
to `i64`) using parallel `List<int>` / `List<string>` on the
`CGen` instance. `InferTypeFromExpr` checks the cache first,
falls through to the new `InferTypeFromExprUncached` on a miss,
stores the result. The cache lives one AST and is wiped per
file by the per-file fresh `CGen`.

Stress test (40-link chain): hangs → 26 ms to .c.

Documented workarounds in the v0.0.5+ ui-web `docs/guide/04-layout-and-theme.md`
("Fluent chain limits") can be retired once consumers pick up
v0.8.15.

## [v0.8.14] — 2026-05-15

Toolchain release bundling four orthogonal fixes/features that
together unblock `amalgame-ui-web` v0.0.5.

### Added: list literals — `[a, b, c]`

The bracket-expression parser now accepts a comma-separated form
in addition to the existing list-comprehension form. Backward
compatible — `[proj for x in iter]` keeps its meaning; what's
new is `[]`, `[e1, e2, e3]`, trailing commas, and multi-line
literals. Codegen emits the same compound-statement expression
as `new List<T>().Add(...)` chains, so any `AmalgameList*` slot
can be initialized inline.

```kotlin
let names: List<string> = ["alpha", "beta", "gamma"]
let nums:  List<int>    = [1, 2, 3, 4, 5]
let empty: List<int>    = []
```

Touches `parser/ast.am` (new `NodeKind.LIST_LITERAL`),
`parser/parser.am`, `resolver/resolver.am`, `linter.am`,
`formatter/formatter.am`, `generator/c_gen.am`. Doc updated in
`docs/guide/02-language-tour.md`.

### Fixed: `.Size()` short-circuit in c_gen

`InferTypeFromExpr` hardcoded `.Count()` and `.Size()` to return
`i64` — correct for `AmalgameList*` / `AmalgameMap*` / etc.,
wrong for user classes with their own `Size()` returning
something else. Symptom: chaining `.Size()` between a static
factory call and a follow-up method produced an undefined
`i64_<method>` C call (`Element.AbsoluteContainer().Size(0, 100).AddChild(x)`
lowered to `i64_AddChild(...)`). Fix: gate the `.Size()`
shortcut behind a receiver-type check — only short-circuit when
the receiver resolves to a collection. `.Count()` keeps the
unconditional shortcut.

### Fixed: multi-line `+ / - / * / / / %` continuation

`ParseAdd` and `ParseMul` ended their loops on the first
NEWLINE token, so:

```kotlin
var s = "alpha"
    + "beta"
    + "gamma"
```

silently parsed as `s = "alpha"` and dropped the continuation.
The EOL variant (`"a" +\n "b"`) ended up with `_unknown_` as
the right operand. Fix: a `LookaheadAfterNewlinesIs(s1, s2, s3)`
helper peeks past every NEWLINE; the binary parsers accept the
operator across a newline iff the next real token matches the
operator set. Statement-terminator newlines still break the
loop.

### Changed: `amc new --template ui-web-form` scaffold

The scaffolder emits an `Element.OnResult("out")` call instead
of the verbose in-page click-listener bridge. Generated
`amalgame.toml` pins `ui-web @v0.0.5` to match the released
package.

### Added: `docs/guide/09-ui-web/` chapter

Mirror of the developer guide that ships with the
`amalgame-ui-web` package — five sub-chapters (getting started,
widget catalogue, events + partial DOM, layout + theming,
extending) under the existing user-guide TOC.

## [v0.8.13] — 2026-05-15

Tooling release: a new `--template ui-web-form` scaffolder for the
freshly-released [`amalgame-ui-web`](https://github.com/amalgame-lang/amalgame-ui-web)
package, plus three small fixes from the post-v0.8.12 backlog.

### Added: `amc new --template ui-web-form`

Scaffolds a single-window webview GUI project on top of
amalgame-ui-web v0.0.4:

- `src/main.am` — `Window` + `Page` + small form (Input + Textarea
  + Submit) + handler with a tiny in-page click bridge that wires
  the handler's return value into a `<pre id=out>` panel.
- `amalgame.toml` — pins `ui-web @v0.0.4`.
- `build.sh` — locates the cached ui-web clone, compiles
  `webview.cc` (C++ TU, one-shot) + `Amalgame_UI_Web.c` + facade
  + user main, links against the OS-native engine:
  - Linux/BSD : `pkg-config --libs webkit2gtk-4.1` (auto-detected,
    falls back to 4.0)
  - macOS    : `-framework Cocoa -framework WebKit`
  - Windows  : `-lWebView2Loader.dll.lib -lOle32 -lShlwapi`
  Resolves `libamalgame.a` from either an XDG install layout
  (`share/amalgame/lib`) or a dev checkout (`$AMC_DIR/lib`).
- `README.md` — install prereqs per distro + theming pointers
  (`Page.SetTheme`, `--amc-*` CSS variables).

The existing `--template forms` (ui-sdl + ui-forms) row in the help
text gains a **sunset 2026-05-15** marker; use `ui-web-form` for
new projects.

### Fixed: `amc package add` tolerates `-dev` pre-release suffix

The strict manifest-vs-tag validator rejected installs whenever a
package was tagged without bumping `version = "X.Y.Z-dev"` to the
release-bare form first — which is the convention several official
packages (including the freshly-tagged ui-web v0.0.4) actually
use. The check now strips a `-dev` suffix before comparing.
Versions that don't match even after stripping (e.g. `0.0.5-dev`
at tag `v0.0.4`) still hard-fail.

### Fixed: macOS canonical executable path

`Program.ResolveSelfPath` now resolves the canonical executable
path on macOS via `_NSGetExecutablePath` + `realpath()`, matching
the existing Linux `/proc/self/exe` and Windows
`GetModuleFileNameA` branches. Homebrew's `/usr/local/bin/amc`
symlink resolves to the versioned cellar, so `amc build`'s
sibling-runtime discovery works on macOS through PATH too. Closes
the last edge case tracked in `CONTINUATION.md` since v0.8.1.

### Fixed: `gdb --dap` fallback in `amc dap`

After exhausting the `lldb-dap` candidate chain, `DetectBackend`
now probes `gdb` (plus a couple of fallback paths
`/usr/bin/gdb` / `/opt/homebrew/bin/gdb`). gdb gained `--dap` in
v14 (2023-11); all modern distros and MSYS2 ship a recent enough
gdb. The path is returned with a `gdb:` sentinel that
`ExecBackend` strips and uses to inject `--dap` as `argv[1]`. The
old-gdb case fails clearly at exec time — the DAP client
surfaces it as a launch error rather than a silent timeout. Linux
+ Windows-MSYS2 users without LLVM now get a working debugger out
of the box.

---

## [v0.8.12] — 2026-05-15

Compiler release unlocking fluent chain APIs for external packages.
Closes the last of the high-impact cgen bugs from the ui-forms
backlog (#2 in ROADMAP_COMPLET) — only #6 (let scope flattened)
remains open.

### Fixed: cross-package chained method calls

`ExtType.Static().Method()` where `ExtType` lives in an `--external`
facade produced invalid `Type_Static()_Method(...)` C tokens. The
underlying mismatch: `RegisterExternalProg` emitted forward decls
but never called `MethodRetSet` for external methods, so the
return-type registry stayed empty for them. `InferTypeFromExpr`'s
`ClassName.Method()` static path also fell back to the consumer's
`SymName` (e.g. `App_Page`) instead of the external mangling
(`Amalgame_UI_Web_Page`) — so even when MethodRet had entries,
the lookup keys didn't match.

Two-part fix in `c_gen.am`:

- `RegisterExternalProg`/`ExternalMethodSig` now register every
  external method's return type via `MethodRetSet` +
  `MethodRetRawSet`, mirroring the in-bundle registration that
  `EmitForwardDecl` does for first-party classes.
- `InferTypeFromExpr`'s static-call IDENTIFIER path tries
  `ExternalClassMangled` before `SymName`, so `Page.New()` on an
  external `Page` resolves to `Amalgame_UI_Web_Page*` and propagates
  through subsequent chained calls.

### Fixed: inline lambda as argument outside list-method dispatch

`EmitExprStr` for `__lambda__` arguments returned a
`__lambda_<param>_<body>__` placeholder string. Higher-order list
methods (Map/Filter/etc.) intercepted earlier via `EmitClosureArg`
and recovered, but every other call site (webview `Bind`, builder
`OnClick`, etc.) got literal garbage tokens.

Replaced with a direct `EmitLambdaAsClosure` call so every inline
lambda emits a real `AmalgameClosure_new((void*)lam_N_fn, env)`
compound statement expression.

### amalgame-ui-web v0.0.3 unlocks fluent builder

Together the two fixes above are what made the new
[`amalgame-ui-web`](https://github.com/amalgame-lang/amalgame-ui-web)
v0.0.3 fluent builder API representable:

```amalgame
let page = Page.New()
    .SetTitle("My App")
    .SetBody(
        Element.Stack()
            .AddChild(Element.Label("Hello"))
            .AddChild(Element.Button("Save")
                .OnClick((req: string) => Json.EncodeString("ok")))
    )
page.ApplyTo(win)
```

### Patch surface

- `src/generator/c_gen.am` — `ExternalMethodSig` registers
  MethodRet entries; InferTypeFromExpr static-call path consults
  ExternalClassMangled; EmitExprStr inline lambda emits real
  closure value.
- `ROADMAP_COMPLET.md` — bug #2 marked fixed.
- `src/package_registry.am` — version bump.
- `README.md` — version bump.

Test suite stays green: 46/46 PASS on Linux/macOS/Windows CI.

### Cgen backlog status

5 of the 6 ui-forms cgen bugs are now closed (parens lost, return
null typecheck, brace-literal interpolation, typed-param lambdas
return type + inline lambda as arg, cross-package chained calls).
The last one — `let` scope flattened to function level — hasn't
been retriggered by ui-web v0.0.x usage; it'll get re-prioritised
when a real consumer hits it again.

---

## [v0.8.11] — 2026-05-15

Compiler release unlocking C-trampoline-style callbacks (used by
the brand-new `amalgame-ui-web` package's `Window.Bind`, and a
prerequisite for any future stdlib callback API). Two coordinated
changes shipped:

### Fixed: cgen mis-parses `{` `}` `:` inside string literals

`EmitInterpolatedString` accepted any `{x...}` content starting
with a letter (or `this.`) as an interpolation slot. CSS embedded
in `.am` string literals — e.g. `"body{font-family:system-ui}"` —
silently triggered interpolation parsing and produced malformed C
(ternary cascades, `String_FromInt(font-family:...)`).

Introduces `IsValidInterpExpr` which requires the brace content to
match `ident(.ident)*` with an optional trailing `(args)` for
method calls. Anything containing `:`, `;`, `-`, ` `, etc. falls
back to a literal `{` / `}`. Discovered while bringing up the
`amalgame-ui-web` spike (inline HTML with `<style>body{...}</style>`
blocks).

PR #452.

### Added: typed-param lambdas + return-type-aware closure calls

Three coordinated parser/cgen changes that make `(req: string) =>
"saved: " + req` style handlers representable. Before this release,
multi-param lambdas accepted only bare identifiers and CGen always
unboxed the closure result as `i64`, which made string-shaped
handlers (needed by webview-style C trampolines) impossible.

- `ParseLambdaMulti` consumes optional `:type` annotations per
  param. `IsLambdaParenStart` looks past type tokens (incl.
  `<...>`, `[]`, `?` suffixes) so the lookahead still distinguishes
  lambdas from parenthesised expressions.
- At lambda assignment, `InferTypeFromExpr` stashes the return type
  via a new `__closure_ret__` map. At `AmalgameClosure_call1/2/3`
  sites, pointer-typed returns (`code_string`, `*`) emit a direct
  cast; scalars stay on the `UnboxScalar` path; missing info falls
  back to the legacy `i64` default so block-bodied lambdas and
  pre-existing code are unaffected.
- Inline lambdas passed as call arguments now emit a real
  `AmalgameClosure_new(...)` instead of a `__lambda_..._placeholder__`
  token. Higher-order list methods (Map/Filter/etc.) still
  intercept earlier via `EmitClosureArg`, so this only affects
  "regular" call sites such as webview `Bind`.

Also: new AM type `Closure` → `AmalgameClosure*` so facade methods
can declare `handler: Closure` without per-package typedef shims.

PR #453.

### New external package: amalgame-ui-web

[`amalgame-ui-web`](https://github.com/amalgame-lang/amalgame-ui-web)
v0.0.2-dev — Webview-based GUI binding. Wraps the MIT-licensed
[`webview/webview`](https://github.com/webview/webview) library and
renders HTML/CSS/JS in the OS-native engine (WebView2 / WKWebView /
WebKitGTK). v0.0.2 surface: `Window` (new, IsValid, SetTitle,
SetSize, Navigate, SetHtml, Init, Eval, Run, Terminate, Destroy,
Bind, Unbind) + bidirectional JS↔AM IPC via typed-lambda handlers.

The two cgen/lambda changes in this release are what made the
package's `Bind` API viable. Replaces the sunset `amalgame-ui-forms`
(SDL retained-mode) and `amalgame-ui-tk` (Tcl/Tk) toolkits as the
recommended GUI path for productive apps — see the design proposal
at `docs/proposals/amalgame-ui-web.md`.

### Patch surface

- `src/parser/parser.am` — `ParseLambdaMulti` typed params,
  `IsLambdaParenStart` typed-param lookahead.
- `src/generator/c_gen.am` — `IsValidInterpExpr` strict
  interpolation check, `__closure_ret__` tracking, return-type-
  aware closure call sites, inline lambdas emit closures directly,
  `Closure` → `AmalgameClosure*` typedef.
- `src/package_registry.am` — version bump.
- `README.md` — version bump.
- `tests/samples/typed_lambda.am` — new sample exercising
  `(string)→string`, `(int)→int`, `(string,int)→string`.

Test suite stays green: 46/46 PASS on Linux/macOS/Windows CI.

The 4 remaining cgen bugs from the ui-forms backlog (#1 forward-
decl ordering, #2 chained calls cross-pkg, #3 field=type shadowing,
#6 let scope flattened) are still open — none reproduce intra-
package on the current amc, awaiting a cross-package consumer.

---

## [v0.8.10] — 2026-05-14

Bug-fix release closing 2 of the 6 cgen/typechecker bugs that
surfaced while shipping amalgame-ui-forms v0.0.x (tracked in
ROADMAP_COMPLET.md under 'Cgen/typechecker bugs surfaced by
amalgame-ui-forms'), plus the install.sh SDL2 auto-install
landed earlier on develop.

### Fixed: parens lost on mixed `* + /`

The cgen's `EmitExprStr` BINARY branch now wraps any sub-
BINARY operand in parens. `(h - 2*pad - gap*(n-1)) / n` used
to lower as `h - 2*pad - gap*(n-1) / n` and evaluate left-
associatively against C precedence, producing the wrong int.
Belt-and-braces over-parenthesising; matches what pretty-
printers like clang-format do.

This was the parens-lost bug listed in CONTINUATION.md
"Persistent todos" since the DateTime migration and worked-
around in five places across ui-forms + amc-bundled stdlib.

### Fixed: `return null` rejected by the typechecker

`IsAssignable` now accepts `null` for any target type that
isn't a primitive value (int/i64/i32/i16/i8/u*/float/double/
f32/f64/bool/char/void). The new `IsPrimitiveValue` helper
enumerates the rejection set; everything else (classes,
strings, List/Map/Set, custom enums via boxed pointers) takes
null as the legitimate NULL sentinel — `return null` from a
`public Widget FindIt()` method now type-checks straight
through without the `@c { return NULL; }` workaround.

### Added: install.sh installs SDL2 + SDL2_ttf by default

Previously, the GUI workflow (`amc new --template forms`)
required a manual extra step to install SDL2 dev headers after
install.sh finished. Now apt / dnf / pacman / zypper / brew /
pkg install libsdl2-dev + libsdl2-ttf-dev alongside libgc +
libcurl + zlib — adds ~5 MB to the install footprint, near-
zero cost for non-GUI users.

Skip with `AMC_NO_GUI=1 ./install.sh` for minimal installs.
Post-success hint trimmed to a one-liner so the output stays
tidy.

### Patch surface

- `src/generator/c_gen.am` — EmitExprStr BINARY branch wraps
  sub-BINARY operands.
- `src/typechecker.am` — IsAssignable null path + new
  IsPrimitiveValue helper.
- `install/install.sh` — SDL2 in the apt/dnf/pacman/zypper/
  brew/pkg install lists; AMC_NO_GUI=1 opt-out.

Test suite stays green: 421/421 PASS.

The 4 remaining cgen bugs (#1 forward-decl ordering, #2
chained calls cross-pkg, #3 field=type shadowing, #6 let
scope flattened) didn't reproduce in intra-package smoke
tests on the current amc — they may already be collateral-
fixed by #4 + #5, or they need a cross-package setup the
ui-forms tests exercised originally. Will investigate when
the next package consumer hits one.

---

## [v0.8.9] — 2026-05-14

Two post-toolkit fixes to round off v0.8.8:

### Fixed: `amc new --template forms` points at the published tags

v0.8.8's scaffolder hardcoded `tag = "v0.0.6-dev"` for both
`amalgame-ui-sdl` and `amalgame-ui-forms` — those tags never
existed on GitHub, so `amc package add` bailed on the clone
step. Now ships v0.1.0 / v0.1.1 (the first stable tags pushed
right after v0.8.8 cut). Scaffolded projects build end-to-end
from the `./build.sh` line.

### Added: SDL2 install hints in `install.sh` + Homebrew formula

- `install.sh` post-success now prints an OS-specific SDL2
  install snippet (apt / dnf / pacman / zypper / brew) right
  after the exe/lib hint, so users discover the GUI workflow
  without reading the README.
- Homebrew formula gains `sdl2` + `sdl2_ttf` as runtime deps;
  ~5 MB combined and the install is otherwise a no-op for
  non-GUI users.
- Windows installer (`amalgame.iss`) stays unchanged — SDL2
  on Windows means MSYS2 or vcpkg, which is too heavy to
  bundle. The forms template README documents the manual
  setup for that platform.

---

## [v0.8.8] — 2026-05-14

Toolkit-day release. Five features lining up the
[amalgame-ui-sdl](https://github.com/amalgame-lang/amalgame-ui-sdl)
+ [amalgame-ui-forms](https://github.com/amalgame-lang/amalgame-ui-forms)
v0.1.0 ecosystem release:

### Added: `amc new --template forms`

Scaffold a ready-to-build GUI project on top of ui-sdl + ui-forms:

```sh
amc new my-app --template forms
cd my-app
sudo apt install libsdl2-dev libsdl2-ttf-dev libgc-dev
amc package add ui-sdl ui-forms
./build.sh && ./my-app
```

The scaffolded `src/main.am` opens a 320×240 window with a Label
+ Button + StackVertical layout via `Application.Run`. Files
also include `amalgame.toml` declaring the package deps, a
`build.sh` that pre-compiles the facade archives and links
against SDL2 via pkg-config, plus a README with the apt / brew /
pacman install snippets.

### Added: `amc package add` accepts multiple specs

`amc package add ui-sdl ui-forms` used to bail with 'unexpected
extra argument: ui-forms'. Now collects positional specs into a
list and installs them sequentially. First failure stops the
run so the user sees the broken spec; multi-spec runs print a
'── Installing X (N/M) ──' header per package.

### Fixed: `PrecompileFacade` resolves runtime/ via the XDG layout

Pre-v0.8.8 the facade pre-compile probed only `<bin>/runtime`
for amc's runtime/_runtime.h — the legacy source-tree path.
Installed via `install.sh` (XDG layout: `<bin>/../share/
amalgame/runtime/`), facade pre-compile failed with
`_runtime.h: No such file`. Now probes the XDG path first,
falling back to the legacy path. Same fix applied to
`PrecompilePackage`.

### Added: `PrecompileFacade` splices `[stdlib].cflags` into gcc

Packages that need extra `-I` flags (typically SDL or other
system libraries via pkg-config) can now declare them in the
manifest:

```toml
[stdlib]
class   = "Window"
header  = "runtime/Amalgame_UI.h"
facade  = "facade.am"
libs    = ["SDL2", "SDL2_ttf"]
cflags  = "$(pkg-config --cflags sdl2 SDL2_ttf)"
```

The string is spliced into the gcc invocation verbatim; popen
runs it through `/bin/sh -c`, so command substitution works
natively. Cross-platform when `pkg-config` is installed (Linux
+ macOS + MSYS2).

### Added: `amc --help` mentions the `forms` template

The top-level `amc --help` line for `new <name>` used to list
only `exe / lib / test / service`; `forms` is now in there too
so users discover it without having to run `amc new --help`
separately.

### Patch surface

- `src/main.am` — top-level help line
- `src/new_cmd.am` — ScaffoldForms + Main/Manifest/BuildSh/
  Readme generators, --template forms accepted
- `src/add_cmd.am` — Run() parses multi-spec, RunOne extracts
  the per-package install. PrecompileFacade + PrecompilePackage
  probe XDG runtime path. PrecompileFacade splices cflags.

421/421 tests pass.

---

## [v0.8.7] — 2026-05-14

**Cross-package facade deps in `amc --lib`** — unblocks packages
whose `facade.am` imports another external package's facade
(e.g. `amalgame-ui-forms` reaching for `Color` / `OSTheme` from
`amalgame-ui-sdl`).

### Fixed: `amc --lib` now resolves cross-package types

Pre-v0.8.7 the `--lib` path skipped the whole "auto-add every
loaded package's `[stdlib].facade` as `--external`" loop. The
work-around was justified for a single-package facade (compiling
ui-sdl's `facade.am` with itself listed as external would force
forward decls and break the precompile), but it left consumers
without forward decls for **other** packages — the resolver
bailed on `Unknown symbol 'OSTheme'` and the cgen mangled
`Amalgame_UI_SDL_Color*` parameters as `Amalgame_UI_Forms_Color*`.

The fix narrows the skip to the self-package only, identified by
matching the registered namespace (`lp.Ns`) against the one
detected from the input file. Other packages still load as
`--external`, so cross-package types resolve cleanly to forward
decls.

`amc --lib facade.am` now works end-to-end for facades with deps
on other external packages. `amc package add` consumes the fix
implicitly — no manifest changes required.

### Patch surface

- `src/main.am` — single branch reworked (`if (!this.IsLib)` →
  per-package self-check via `lpPrefix == nsPrefix`).
- Test suite stays green: 451/451 PASS.

---

## [v0.8.6] — 2026-05-14

**Real fix** for the Windows sample-scaffold bug we *thought* we
fixed in v0.8.5.

### Fixed: `amc new` no longer shells to `mkdir -p` (PR #425)

v0.8.5 patched `amalgame.iss` to call `amc.exe` directly (bypassing
`cmd.exe /c` wrapping) — that closed one half of the symptom. But
the directories still landed as `'MyFirstApp'` (with literal single
quotes) and `-p`, because the **real** bug was inside `amc new`:

- `Process.Run("mkdir -p " + ShellEscape(name))` — cmd.exe's
  built-in `mkdir` has no `-p` flag, so cmd interpreted `-p` as a
  literal directory name to create alongside the actual one.
- `ShellEscape(name)` wraps the project name in single quotes for
  POSIX `sh`; cmd doesn't strip single quotes → the folder name
  ended up as `'MyFirstApp'` (with literal quotes).

Fix in two parts:

1. **New `File.Mkdir(path)` runtime helper** (`Amalgame_IO.h`) —
   cross-platform recursive directory creation. POSIX
   `mkdir(0755)` / Windows `_mkdir`. Walks the path, ignores
   `EEXIST`, handles drive letters + leading slashes. Idempotent.
2. **Six call sites in `src/new_cmd.am`** switched from
   `Process.Run("mkdir -p " + ShellEscape(...))` to
   `File.Mkdir(...)`. Linux scaffold output is byte-identical
   pre/post fix.

amc 0.8.6 is otherwise byte-identical to 0.8.5 on Linux / macOS;
only the Windows `amc new` scaffold behaviour changed.

---

## [v0.8.5] — 2026-05-14

**Hotfix** for v0.8.4 Windows sample scaffold.

### Fixed: scaffold created `'MyFirstApp'` + `-p` instead of `MyFirstApp` (PR #421)

The Windows installer's sample-scaffold post-install step invoked
`amc.exe` through `cmd.exe /c cd /d "..." && "..." new MyFirstApp
--vscode`. With the bundled `amc.exe` living under
`C:\Program Files\Amalgame\bin\` (path contains a space → must be
quoted), cmd's documented but quirky `/c` parsing of multiple
internal double-quote pairs re-tokenised the call and amc ended
up receiving its name argument wrapped in literal single quotes.
Result: `%USERPROFILE%\Amalgame\samples\` ended up with a
`'MyFirstApp'` directory and a stray `-p` one, while a usable
`MyFirstApp\` was never created.

Fix: drop the `cmd.exe` wrap entirely. Inno Setup's `Exec()`
takes a `WorkingDir` as its third parameter, so the Pascal code
now calls `amc.exe` directly with `cwd = ParentDir`. No shell,
no quoting surface, no mangled args.

amc 0.8.5 is otherwise byte-identical to 0.8.4 (same compiler,
same stdlib, same DLLs). Linux / macOS tarballs unchanged —
this hotfix only touches `install/windows/amalgame.iss`.

---

## [v0.8.4] — 2026-05-14

**Hotfix** for v0.8.3 Windows installer.

### Fixed: setup.exe crashed at sample-scaffold step (PR #418)

When a user ran the v0.8.3 setup on a fresh Windows install, the
final task — running `amc.exe new MyFirstApp --vscode` to scaffold
the sample project — exploded with:

> The code execution cannot proceed because libngtcp2_crypto_ossl-0.dll
> was not found. Reinstalling the program may fix this problem.

Root cause: `release.yml` shipped a hardcoded list of MinGW DLLs
the binary "should" link against. MSYS2 silently renamed the
ngtcp2-crypto package from `libngtcp2_crypto_quictls.dll` to
`libngtcp2_crypto_ossl-0.dll` (libcurl switched from QuicTLS to
plain OpenSSL for HTTP/3). The hardcoded list still referenced
the old name → the new DLL never landed in the tarball → amc.exe
launched but immediately failed to resolve its imports.

Fixed by replacing the static list with a recursive `ldd` walker
in the Windows build job: start from `amc.exe`, follow every
`/mingw64/bin/` dependency, recurse into each freshly-copied DLL,
converge at fixed-point. Future MSYS2 renames can't silently
break us again.

amc 0.8.4 is otherwise byte-identical to 0.8.3 (same compiler,
same stdlib, same VS Code extension). If you already grabbed the
Linux / macOS tarballs from v0.8.3, no need to re-download —
this hotfix only changes which Windows DLLs ship.

---

## [v0.8.3] — 2026-05-14

The **"peacock"** release. Cosmetic but long-overdue: Amalgame
finally has a logo. A peacock wheel of 7 polychrome plumes
converging on a central eye-as-`A` — metaphor for the synthesis
of best features from many languages.

### Wired everywhere (PR #415)

- `assets/logo.svg` — source-of-truth SVG, 400×300 viewBox
- `README.md` — banner above the title (240 px wide)
- `install/windows/assets/amalgame.ico` — multi-resolution
  16/32/48/64/128/256 ICO so Windows Add/Remove Programs, the
  setup wizard window, and the taskbar all show the right glyph;
  `amalgame.iss` `SetupIconFile` re-enabled (had been commented
  out in v0.8.2 because the asset wasn't committed)
- `editors/vscode/icon.png` — 256×256 PNG referenced from
  `editors/vscode/package.json`'s `icon` field; `vsce package`
  picks it up automatically so the bundled `.vsix` and the
  future Marketplace listing both carry it

No runtime / compiler changes — pure branding. amc 0.8.3 reports
the bumped version but behaves identically to 0.8.2.

---

## [v0.8.2] — 2026-05-14

The **"batteries-included onboarding"** release. A fresh developer
running `install.sh` (POSIX) or the `.exe` (Windows) now lands on a
ready-to-code setup: gcc on PATH, runtime libs in place, VS Code
extension installed, Neovim / Helix LSP wired, and a sample
project at `~/Amalgame/samples/MyFirstApp` (or
`%USERPROFILE%\Amalgame\samples\MyFirstApp`) they can open with
VS Code and F5-debug immediately. Plus a long-standing data-layer
bug on `Main(List<string> args)` that made `args` unusable.

### Fixed: Windows `gcc-bundle` opt-in restored (regression PR #398)

The XDG-layout cleanup in v0.8.1 dropped the `gcc-bundle/` block
from `install/windows/amalgame.iss`. Without it, a Windows user
who installed via the `.exe` ended up with a working `amc.exe`
but a broken `amc build` — no `gcc.exe` on PATH unless they had
MSYS2 separately.

Restored as a compile-time opt-in via `#if FileExists("gcc-bundle\\bin\\gcc.exe")`:
when the maintainer drops a MinGW-w64 toolchain under
`install/windows/gcc-bundle/` before running `iscc`, the resulting
`.exe` ships gcc + gdb at `{app}\gcc` and prepends `{app}\gcc\bin`
to user PATH. `PUBLISHING.md` documents the staging step
(`winlibs.com` archive).

Bonus: the wizard auto-deselects the bundled gcc task when
`where gcc` succeeds on the host, so users who already have
MSYS2 / MinGW / Cygwin don't get a duplicate ~200 MB toolchain.
They can re-check the box to force an isolated install.

### New: VS Code extension auto-installed by both installers

`release.yml` now stages the latest `editors/vscode/amalgame-*.vsix`
(`sort -V`) under `share/amalgame/editors/vscode/`. Both
installers pick it up:

- **POSIX** — `install.sh` detects every variant on PATH
  (`code`, `code-insiders`, `codium`, `code-oss`) and runs
  `--install-extension … --force` against each. Idempotent.
- **Windows** — `amalgame.iss` `samplescaffold` task + a new
  `vscode_ext` checkbox triggers the same `cmd /c code
  --install-extension` from the Pascal `[Code]` section.

Opt out of the editor wiring entirely with `AMC_NO_EDITORS=1`.

### New: Neovim + Helix LSP auto-wiring (POSIX)

`install.sh` drops a self-contained module at
`~/.config/nvim/lua/amalgame_lsp.lua` (with `lspconfig.amalgame`
+ `vim.filetype.add({ extension = { am = "amalgame" }})`) and
prints the one-line `require('amalgame_lsp')` the user adds to
`init.lua`. Doesn't touch `init.lua` itself — too many competing
setups to safely edit.

For Helix, appends a `[[language]]` + `[language-server.amalgame-lsp]`
block to `~/.config/helix/languages.toml`, idempotent via grep
guard.

### New: sample project scaffold at `~/Amalgame/samples/MyFirstApp`

After deps + editor wiring, both installers run `amc new
MyFirstApp --vscode` so the user has somewhere to F5 into without
first having to learn `amc new`. The Windows `.iss` adds it as a
wizard task (default checked, opt-out friendly), and the `FinishedLabel`
points the user at the path + the GitHub online docs URL.

The POSIX installer's final summary now includes `code
$SAMPLE_DIR` as the first suggested command and bumps the docs
link to the full guide (`tree/main/docs/guide`, not just the
README).

Opt out with `AMC_NO_SAMPLE=1`.

### Fixed: macOS xcode-select hard-blocks the install

Previously the missing Xcode Command Line Tools surfaced as a
silent warn during `install.sh`, then a confusing build failure
later at the user's first `amc build`. Now the script short-
circuits with a three-step actionable message ("run
`xcode-select --install`, accept Apple's GUI, re-run me") if
`xcode-select -p` fails. The Apple GUI dialog itself remains
unavoidable — it's a system policy, not something a script can
bypass.

### Fixed: `Main(List<string> args)` saw garbage instead of args

The C entry point emitted by `src/main.am` was casting `argv`
(a `char**`) to `code_string*` and passing it to user code
declared as `Main(AmalgameList* args)`. The pointer happened to
be reachable, but reading `args.size` / `args.data` dereferenced
raw stack memory: in the VS Code debug pane `args` showed up as a
naked hex address (no struct fields), and any access from
Amalgame returned garbage or crashed.

Fix (PR #402):

- New `code_runtime_args_list()` helper in `runtime/_runtime.h`
  builds a proper `List<string>` from `argv[1..]`. Program name
  is left on `Args.Get(0)` for callers who want it — matches
  the .NET / Kotlin convention.
- The cgen detects the declared signature by grepping the
  emitted forward decl: `Main(AmalgameList*` → fixed path;
  `Main(code_string*` → legacy `(code_string*)argv` cast kept
  for amc's own Main + the older test samples.

Inline pretty-printing (`args = ["first", "second"]` directly in
the debug pane) still requires the DAP message-rewriting bridge
tracked as "Approche A" in `ROADMAP_COMPLET.md` — but the data
layer is now correct and the variables pane can navigate the
struct.

---

## [v0.8.1] — 2026-05-14

The **"polish the debugger"** release. Four PRs land together to
make the v0.8.0 DAP work flawlessly from a fresh install — no
`AMC_RUNTIME` hack, no missing breakpoints, no surprise rebuild
gotchas, no broken `amc test` after PATH installs.

### New: `amc new --vscode` opt-in flag (PR #395)

`amc new <name> --vscode` writes `.vscode/launch.json` (two
configurations: POSIX + Windows `.exe`, picked via F5 dropdown)
and `.vscode/settings.json` alongside the regular template
scaffold. Opt-in so projects edited in Neovim / Helix / IntelliJ
stay clean. Top-level `amc --help` advertises the flag on the
`new` line for discoverability.

### Improved: `.vscode/tasks.json` with `preLaunchTask` (PR #397)

The scaffold now also drops `.vscode/tasks.json` with two tasks
("amc: build (debug)" and "amc: build (release)"), and both
`launch.json` configurations carry `preLaunchTask: "amc: build
(debug)"`. F5 rebuilds with `-g` automatically — fixes the
classic "F5 runs but no breakpoint stops" pitfall when the
binary on disk is a release build.

The generated `README.md` (exe + service templates) gains a
"Debug" section covering `./build.sh -g`, an `lldb` session,
and the VS Code F5 flow.

### Fixed: `amc build` / `amc test` via $PATH install (PR #396 + #397)

`amc build` and `amc test` derived `runtime/` from
`dirname(argv[0])`. When amc is launched through `$PATH` —
which is the standard install case — `argv[0]` is the bare name
`amc`, dirname collapses, and gcc never gets `-I'<runtime>'` →
`fatal error: _runtime.h: fichier ou dossier de ce type`.

New helper `Program.ResolveSelfPath()` reads `/proc/self/exe` on
Linux and `GetModuleFileNameA` on Windows for the kernel-resolved
canonical executable path. macOS still uses `argv[0]` (Homebrew
ships an absolute symlink so the legacy lookup already works
there); `_NSGetExecutablePath` is queued for v0.8.2.

The scaffolds also stop reimplementing the runtime-discovery
logic. `BuildShExe`, `BuildShService` and `BuildPs1Service` are
now two-line wrappers around `amc build`, forwarding `"$@"` so
`./build.sh -g` propagates straight to the debug build.

### Fixed: XDG-style install layout, cross-OS (PR #398)

`install/install.sh` used to copy `_runtime.h` alone (missing
every `Amalgame_*.h`), skip `libamalgame.a` entirely, and force
`AMC_RUNTIME` into the user's shell rc as a hack to compensate.
The Windows Inno Setup script (`install/windows/amalgame.iss`)
was worse: pinned to `0.3.0`, broken paths (`build-windows/`,
`src/transpiler/`), no `libamalgame.a`, registry-writing
`AMC_RUNTIME`.

All three install paths now share one tree shape:

```
<prefix>/
├── bin/
│   └── amc(.exe) [+ MinGW DLLs on Windows]
└── share/amalgame/
    ├── runtime/      _runtime.h + Amalgame_*.h
    ├── lib/          libamalgame.a
    └── docs/         grammar + language tour (for amc migrate / explain / generate)
```

`<prefix>` is `/usr/local` (Linux system), `~/.local` (Linux/macOS
user — auto-fallback when /usr/local isn't writable), `brew
--prefix` (macOS Homebrew, auto-detected), or `C:\Program
Files\Amalgame` (Windows installer).

amc gains two helpers — `Program.ResolveRuntimeDir(amcPath)` and
`Program.ResolveLibAmalgameA(amcPath)` — that probe a stable
chain:

1. `$AMC_RUNTIME` / `$AMC_LIB` env override
2. `<bin>/../share/amalgame/{runtime, lib}` ← XDG install
3. `<bin>/{runtime, lib}` ← legacy + dev source tree

No env var is required for the standard layout. The Inno Setup
registry write for `AMC_RUNTIME` is gone.

`release.yml` stages the matching XDG tree, so installers do a
flat `cp -r dist/$NAME/* $PREFIX/` with no per-file rewiring.

### Build state

- `./amc --version` → `amc 0.8.1 (commit <sha>, built <ts>)`
- `./build_amc.sh` ≈ 3 s end-to-end
- `./tests/run_all_tests.sh` → 451 / 451 PASS (214 core + 191
  stdlib + 12 fmt + 34 amc-new)
- snapshot/amc_lib.c regenerated; snapshot/amc rebuilds in one
  gcc step from the tracked .c

### Setup notes

LLVM 18+ for `amc dap` (unchanged from v0.8.0):

```bash
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh && sudo ./llvm.sh 18
sudo apt install -y lldb-18
```

macOS: `xcode-select --install`. Windows: pending the
`gdb --dap` fallback queued for v0.8.2.

---

## [v0.8.0] — 2026-05-13

The **"debug adapter"** release. Three changes ship together to
make `.am` source-level debugging Just Work in any DAP-capable
editor (VS Code, Neovim, Helix, …).

### New: `amc dap` — DAP proxy over stdio

`amc dap` is a thin Debug Adapter Protocol proxy. It detects a
DAP-native backend on the host (`lldb-dap` from LLVM 18+ today,
`gdb --dap` from gdb 14+ planned for v0.8.1) and `execvp()`s
into it. After the exec, stdin/stdout — already wired by the
DAP client to its JSON-RPC pipes — flow directly to the backend
with no in-amc copy. No message parsing, no path rewriting at
this layer; everything resolves natively via DWARF + the new
`#line` directives below.

Detection order (first hit wins):

1. `lldb-dap` (unsuffixed, if a generic `lldb` distro package
   installed it)
2. `lldb-dap-{20,19,18}` (Debian/Ubuntu LLVM versioned aliases)
3. `/usr/lib/llvm-{20,19,18}/bin/lldb-dap` (full apt.llvm.org path)

Backend args (`--port`, `--comm-file`, …) pass through: `amc dap
--port 12345` is equivalent to `lldb-dap --port 12345`. If no
backend is found, prints an install hint per OS and exits 127.

**v0.9.0+ trajectory (Approche A, see ROADMAP_COMPLET.md):**
swap `execvp` for a fork + pipe + `poll()` loop and start
rewriting messages on the way through — pretty-print
`AmalgameList*`/`AmalgameMap*`, filter Amalgame runtime
frames, decode closures. The proxy stays as a fallback
(e.g. `amc dap --raw`).

### New: `amc build --debug` / `amc run --debug` (alias `-g`)

When set, the build pipeline swaps `-O2` for `-O0 -g` on the
gcc/g++ invocation (both the single-stage and C++ two-stage
paths). DWARF debug info is now embedded so `lldb` /
`lldb-dap` can map breakpoints, inspect locals, and walk the
stack. Watch builds (`amc watch`) keep `-O2` by default —
no overhead penalty for the non-debug hot loop.

### New: `#line` directives in cgen output

For every statement whose source line differs from the
previous one, the cgen now emits a `#line N "foo.am"`
directive before the lowered C. gcc + clang both honour these
and embed the `.am` filename + line number in DWARF
(`DW_AT_decl_file` / `DW_AT_decl_line`). Result: debuggers see
the user's `.am` sources directly — `breakpoint set --file
foo.am --line 5` binds to the right address with no source
map files, no path translation in the proxy.

This also improves gcc error messages on debug builds: `-Werror`
warnings reference `foo.am:line` instead of `foo.c:line`.

### Setup

LLVM 18+ install on Debian/Ubuntu (the same recipe used to
develop this release):

```bash
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 18
sudo apt install -y lldb-18
# lldb-dap-18 ends up in /usr/bin and /usr/lib/llvm-18/bin
```

macOS: `xcode-select --install` (lldb-dap ships with Xcode
Command Line Tools 14+). Windows: pending v0.8.1 `gdb --dap`
support, install gdb 14+ via MSYS2.

### VS Code `launch.json`

```jsonc
{
  "type": "amc",
  "request": "launch",
  "name": "Debug current .am",
  "program": "${workspaceFolder}/${fileBasenameNoExtension}",
  "args": []
}
```

Pair with a custom adapter type in `package.json`:

```jsonc
"debuggers": [{
  "type": "amc",
  "label": "Amalgame",
  "program": "amc",
  "args": ["dap"]
}]
```

This will be folded into `editors/vscode/` in a follow-up; for
v0.8.0, copying the snippets into a fresh `.vscode/launch.json`
is enough to attach.

---

## [v0.7.10] — 2026-05-13

The **"LSP signature help"** release. Two tooltip-side UX
improvements for the editor integration land.

### New: `textDocument/signatureHelp`

The LSP server now publishes the `signatureHelpProvider`
capability with trigger characters `(` and `,`. When the user
is typing inside an active call (`foo.bar(|x, y)`), the editor
asks the server for the active method's signature; we return:

- `signatures[0].label` = `name(p1: t1, p2: t2): ret`
- `signatures[0].parameters[]` = per-param `[start, end]` offsets
  so the client can bold the active parameter
- `activeParameter` = index based on how many `,` the user has
  already typed (one per completed arg)

v1 finds the called method by name across the workspace via the
new `FindMethodDeclByName` helper — first match wins. Overloading
is not expressed in Amalgame, so a workspace-order scan is
unambiguous for well-formed code.

### Improved: `textDocument/hover`

When the hovered identifier resolves to a method (declaration
site, MEMBER receiver, or IDENTIFIER call-site), the tooltip
now shows the **full signature** — `name(p1: t1, …): ret` —
instead of just the inferred type. Falls through to the old
type-only display for non-method nodes.

### Internals

- New static helpers on `LspServer`:
  - `FormatMethodSignatureMarkdown(METHOD_DECL)` — shared
    formatter used by hover + signatureHelp.
  - `FindMethodDeclByName(prog, name)` — workspace-wide
    method lookup.
  - `FindCallAtPosition(root, line, col)` — deepest CALL
    whose line/col precedes the cursor.
  - `CallCalleeName(call)` — extract the function name from a
    CALL's `Left` (IDENTIFIER or MEMBER).

### Deferred

- Per-call-site dispatch resolution (resolve the exact
  overload the typechecker would pick) — not needed today
  since Amalgame doesn't have method overloading, but the hook
  is the same place to wire in once overloading lands.
- Doc-comment extraction in tooltips — when the parser starts
  attaching `///` doc comments to METHOD_DECL nodes, surface
  them as the body of the markdown response.

---

## [v0.7.9] — 2026-05-13

The **"build / run / watch"** release. Three first-class compile
verbs land, factoring the gcc-link logic that `amc test` has
been carrying internally since v0.5.

### New subcommands

- **`amc build [-o <out>] [-v] <entry.am>`** — runs amc + gcc
  in one step. Output defaults to the entry's stem
  (`hello.am` → `./hello`). Links `lib/libamalgame.a` when
  shipped alongside amc, plus every installed package's facade
  archive / vendored `.o` / `[stdlib].libs`.
- **`amc run [-o <out>] [-v] <entry.am> [-- args…]`** — build
  then exec. Args after the `--` sentinel forward to the user
  binary's argv.
- **`amc watch [-o <out>] [--run] [-v] <entry.am>`** — build
  now, then poll the entry's mtime every 500 ms and rebuild on
  change. With `--run`, re-exec after each rebuild. mtime
  polling is a small vendored `@c { }` block calling `stat()` /
  `_stat64` — no FileWatcher package dependency.

### Internals

- New `Program.BuildOneBinary` static helper factors the
  amc + gcc invocation that `RunTest` was carrying inline.
  Returns the exit code; same gcc flags either way (runtime
  `-I`, two-stage C++ link when any package ships `.cpp`,
  package `.o` / facade archives / `-l<lib>`).
- New `Program.BuildEntry` orchestrates the full pipeline:
  `EnsureInstalled`, `PreCompilePackageSources`,
  `CollectFacadeArchives`, `CollectLibs`, then `BuildOneBinary`.

### Backwards compatibility

The bare-args form `amc foo.am -o foo` keeps its v0.7.x
behaviour: emit `foo.c`, no gcc step. Users who want to splice
their own gcc command (CI cross-compile, custom flags) can rely
on it staying stable.

### Migration note

`amc watch` watches only the entry file explicitly named on the
command line in v1. Transitive imports come post-D when the
`Amalgame.IO.FileWatcher` package gains an event-based mode.

---

## [v0.7.8] — 2026-05-13

The **"bundled-runtime trim"** release. Three runtime-header
modules move out of amc's bundled tree into stand-alone external
packages — same pattern as the v0.7.7 framework split, but for
C-header bindings rather than `.am` facades.

### Three new external packages (all v0.1.0)

- [`amalgame-regex`](https://github.com/amalgame-lang/amalgame-regex)
  — POSIX extended-regex binding (Test / Match / MatchAll /
  Replace). Available on every POSIX + MSYS2 host.
- [`amalgame-compress`](https://github.com/amalgame-lang/amalgame-compress)
  — zlib gzip + raw-deflate codec (Gzip / Gunzip / Deflate /
  Inflate over `List<int>` buffers).
- [`amalgame-net-websocket`](https://github.com/amalgame-lang/amalgame-net-websocket)
  — RFC 6455 WebSocket client (text frames, plain TCP, in-process
  SHA-1 + Base64 — no OpenSSL dep).

### Bundled `runtime/Amalgame_*.h` shrinks

Before v0.7.8: 9 runtime headers shipped with amc. After: 6
headers (the bootstrap surface).

```
runtime/
├── _runtime.h           (GC, exception, code_string — irréductible)
├── Amalgame_String.h    (compiler hot path)
├── Amalgame_Collections.h
├── Amalgame_Console.h
├── Amalgame_IO.h        (+ Path_*)
├── Amalgame_Net.h       (HTTP for migrate.am AI providers)
└── Amalgame_Process.h   (gcc invocation, env)
```

The three removed headers ship in their respective external
packages. amc's cgen `isCoreStdlib` mapping for `Regex.X` /
`Compress.X` / `WebSocket.X` stays in place — call sites still
lower to flat `Regex_X` etc., and the package's runtime header
contributes the matching symbol via `PkgHeaders` → `#include`.

### Migration

User code calling `Regex.X` / `Compress.X` / `WebSocket.X` must
add the matching package once:

```bash
amc package add regex compress net-websocket
```

The package is required from v0.7.8 onwards. On v0.7.7 it's a
no-op (header already bundled).

### Known gotcha

The cgen has a latent **facade-package ABI bug** when a facade
`.am` calls its own static methods via `ClassName.X()` syntax.
The bug fires when the package's own class is in
`amalgame.lock` at compile time — `PkgClassMangledPrefix`
collides with the file-scope `SymName` path, producing a
function-decl with the wrong mangling and a runtime segfault.
No currently-released facade hits the pattern; tracking for
v0.8.0 as the prerequisite to extract msgpack.

---

## [v0.7.7] — 2026-05-13

The **"framework split"** release. Five user-facing stdlib modules
move from the bundled `lib/libamalgame.a` into stand-alone external
packages on `amalgame-lang/`, leaving amc itself slimmer
(`libamalgame.a` 215 KB → 91 KB) and exposing the same modules as
versioned-and-installable Cargo-style deps.

### Five new external packages (all v0.1.0)

- [`amalgame-datetime`](https://github.com/amalgame-lang/amalgame-datetime)
  — Instant / Duration / Stopwatch + ISO 8601 round-trip.
- [`amalgame-logging`](https://github.com/amalgame-lang/amalgame-logging)
  — 4-level leveled stderr + optional file sink.
- [`amalgame-service`](https://github.com/amalgame-lang/amalgame-service)
  — SIGTERM/SIGINT handler + ShouldStop polling + interruptible Sleep.
- [`amalgame-io-filewatcher`](https://github.com/amalgame-lang/amalgame-io-filewatcher)
  — Single-file mtime polling.
- [`amalgame-yaml`](https://github.com/amalgame-lang/amalgame-yaml)
  — YAML 1.2 subset reader (block mappings, sequences, scalars).

(The five packages from v0.7.6 — `amalgame-{math,math-vec,encoding,
crypto,random}` — were already external; v0.7.7 just removes the
now-redundant `src/stdlib/<mod>.am` source from amc.)

### Compiler

- **cgen**: `RegisterExternalProg` split into a 2-pass walk —
  pass 1 registers every external class in the file, pass 2 emits
  the method forward declarations. Fixes cross-class signatures
  (e.g. `InstantResult.GetValue()` returning `Instant`) that
  previously fell back to the consumer's namespace prefix because
  the second class wasn't yet registered when the first class's
  methods were processed.
- **`amc new --template service`**: scaffold now drops an
  `amalgame.toml` declaring the two external deps so
  `amc package add logging service` Just Works on a fresh project.

### Bundled stdlib after the split

`lib/libamalgame.a` now only contains the bootstrap deps:
`json`, `toml`, `path`, `amc_buildinfo`, plus `msgpack` (deferred
to v0.8.0 — its public API references `JsonValue` and waits for
the bundled-stdlib forward-decl resolution work in the cgen).

### Other

- `MonoTimer` vendored into `src/main.am` for `--verbose` phase
  timing (Stopwatch is no longer bundled).
- `tests/run_stdlib_tests.sh` trimmed to the bootstrap modules
  (random / encoding / crypto / datetime / logging / service /
  filewatcher / yaml / math / math-vec tests live in their
  packages' repos and run on package CI).

### Migration notes

User code importing `Amalgame.DateTime`, `Amalgame.Logging`,
`Amalgame.Service`, `Amalgame.IO.FileWatcher`, or
`Amalgame.Formats.Yaml` must add the matching package once:

```bash
amc package add datetime logging service io-filewatcher yaml
```

`amalgame.toml` gets a `[dependencies]` entry per package; the
lock + cache are populated by `amc package add` at install time.

---

## [v0.7.6] — 2026-05-13

The **"stdlib purity arc + per-package facade pipeline"** release.
Bundles seven PRs accumulated since v0.7.5:

- `#371` Logging migrated runtime → AM (first post-G migration).
- `#372` DateTime migrated to pure AM (Hinnant civil-from-days,
  ISO 8601 format/parse). Two `@c {}` for clock syscalls only.
- `#373` `@c { … }` extended to **file scope** for state globals
  + libc includes. Drops `Amalgame_Logging.h` + `Amalgame_DateTime.h`.
- `#374` FileWatcher / Service / Random / Crypto migrated;
  their runtime headers deleted.
- `#375` BuildInfo 100 % pure AM via build-time literal
  substitution (`build_amc.sh` Step 0 + `amc_buildinfo.am.in`).
  `runtime/Amalgame_BuildInfo.h` deleted.
- `#377` **Per-package facade pipeline** — manifests can declare
  `[stdlib].facade = "facade.am"`. `amc package add` precompiles
  the facade through `amc --lib` + `gcc -c` + `ar rcs` into
  `<pkg-cache>/build/<platform>/libamalgame-pkg-<class>.a`.
  User builds auto-pass `--external <facade>` and auto-link the
  archive (same mechanism as `lib/libamalgame.a` for the
  integrated stdlib, but per-external-package).
- `#378` Math.h + Math_Vec.h migrated to pure AM, runtime
  headers deleted. `Math.X(...)` and `Vec3.X / Vec4.X / Mat4.X`
  now lower via namespace mangling (no more `isCoreStdlib`
  short-circuit). The v0.5-era LCG primitives
  (`Math_Random` / `Math_SeedRandom`) are gone — `Amalgame.Random`
  (PCG-32 + crypto entropy) replaces them.

### Net result

- `runtime/Amalgame_*.h` count: **18 → 9** files (eight removed
  this arc: Logging, DateTime, FileWatch, Service, Random,
  Crypto, BuildInfo, Math, Math_Vec). The remaining 9 are all
  justified by perf (foundation `_runtime.h`, String/Collections)
  or vendor bindings (Net libcurl, Compress zlib, Regex POSIX,
  Console/IO/Process/WebSocket thin syscall wrappers).
- Tests: **613 → 621 PASS** / 0 FAIL / 0 SKIP.
- Five new external packages live (`amalgame-encoding`,
  `amalgame-crypto`, `amalgame-random`, `amalgame-math`,
  `amalgame-math-vec`) all opt in to the facade pipeline. They
  ship pure-AM facades for the same modules amc still bundles —
  v0.7.6 keeps both paths working; v0.8.0+ will remove the
  bundled copies once the packages stabilise on the curated index.

### Breaking changes

- **`Math_Sqrt(x)` / `Math_AbsI(x)` etc. flat-name calls removed.**
  Callers use the qualified `Math.Sqrt(x)` / `Math.AbsI(x)` form
  exposed by `src/stdlib/math.am`.
- **`Math_Random` / `Math_SeedRandom` / `Math_RandomInt`** —
  the runtime LCG primitives — are gone. Use
  `import Amalgame.Random` + `new Random(seed)` / `Random.SystemBytes(n)`.
- `Vec3` / `Vec4` / `Mat4` symbol naming: the C-level mangled
  name changes from `Vec3_X` (runtime header form) to
  `Amalgame_Math_Vec_Vec3_X` (namespace form). User AM code is
  unaffected; only handwritten C consumers of the runtime
  symbols need updating.

---

## [v0.7.5] — 2026-05-13

The **"project F — libamalgame.a pre-compile"** patch release.
Ships the second half of the v0.7.4 decision: every user-facing
stdlib module (`src/stdlib/*.am`) is now pre-compiled into a
single `lib/libamalgame.a` archive shipped alongside the `amc`
binary. User programs that import these modules pass them via
`--external <mod.am>` and link against the archive — the
generated `.c` shrinks by up to 10×, the amc step is ~30%
faster, and the namespace mangling stays consistent across
modules.

### --external `<file.am>` (project F POC, expanded)

PR #360 introduced the flag in a 2-module POC (random,
encoding). This release expands it to cover all 11 standalone +
cross-dependent user-facing modules: **random, encoding, crypto,
datetime, logging, path, service, json, toml, yaml, msgpack**.

Files passed via `--external` are parsed for resolver +
typechecker visibility (so call sites still type-check), but
the cgen emits forward declarations only — class definitions
land at link time from `lib/libamalgame.a`. The cgen tracks each
file's own namespace prefix in a new `ExternalClasses` /
`ExternalClassNsArr` table so call sites lower through the
file's mangling (`Amalgame_Random_Random_new`), not the bundle's
global `NsPrefix` (which used to produce `App_Random_new` and
only worked by accident when the bundled .am files happened to
share a root namespace).

### Cgen — InferTypeFromExpr fix for cross-stdlib deps

`let v = new JsonValue()` inside `msgpack.am` (compiled with
`--external src/stdlib/json.am`) now infers
`Amalgame_Formats_Json_JsonValue*` instead of the namespace-
collided `Amalgame_Formats_MsgPack_JsonValue*`. Was the last
hold-out preventing msgpack from round-tripping through the lib
path.

### tools/build-stdlib.sh — 11 modules, 197 KB archive

Pre-compiles each user-facing standalone stdlib module:

```
amc --lib --quiet -o $BUILD/<mod>  src/stdlib/<mod>.am
gcc -O2 -Iruntime -c $BUILD/<mod>.c -o $BUILD/<mod>.o
```

then `ar rcs lib/libamalgame.a $BUILD/*.o`. Cross-stdlib modules
(today: just **msgpack** which references Json's `JsonValue`)
get `--external src/stdlib/<dep>.am` threaded through so the
cgen routes inter-module references correctly. Reads `$CPPFLAGS`
for the gcc step so macOS release.yml can splice in homebrew
prefix includes.

Resulting archive is **197 KB** on Linux x86_64. Math.Vec stays
out — its real impl lives in `runtime/Amalgame_Math_Vec.h` (the
AM file is a facade stub for the resolver, same shape as
path/logging/service in v0.7.4 before BuildInfo migrated).

### build_amc.sh — Step 4 auto-builds libamalgame.a

After the amc binary is up, `build_amc.sh` invokes
`tools/build-stdlib.sh` as Step 4 so a normal `./build_amc.sh`
run also produces `lib/libamalgame.a`. From a clean clone the
expected output ends with:

```
=== Step 4: Build lib/libamalgame.a ===
✓ lib/libamalgame.a (197346 bytes)
```

### release.yml — bundle libamalgame.a per OS

Each platform job copies `lib/libamalgame.a` into the staged
release archive's `lib/` directory (alongside `runtime/`). macOS
and Windows jobs gained a **"Pre-compile user-facing stdlib"**
step since neither runs `build_amc.sh` end-to-end — they each
call `tools/build-stdlib.sh` directly after the amc binary is
built. Result: every Linux / macOS / Windows release artefact
now ships with the pre-compiled stdlib lib.

### Bench (msgpack + json from user code)

| Path                                | user .c size       | amc time       |
|-------------------------------------|--------------------|----------------|
| Legacy (bundle .am into user.c)     | 1337 lines         | 147 ms         |
| `libamalgame.a` (--external + link) | 145 lines (-90 %)  | 99 ms (-32 %)  |

Gcc step is similarly faster — it digests an order of magnitude
less code. Bonus correctness: the legacy path bundled cross-
module symbols under the *root file's* `NsPrefix`, which only
happened to work because the existing tests pass .am files in a
specific order. The new path uses each module's own namespace,
so cross-module refs stay consistent.

### Tests

- New `run_external_test` helper in `tests/run_tests.sh`
  (`--external` + `lib/libamalgame.a` link, asserts substring).
- New fixture `tests/samples/external_libamalgame.am` exercises
  random + encoding + json + msgpack through the lib path
  end-to-end.
- 3 new e2e tests: `libamalgame.a: random` /
  `libamalgame.a: encoding` / `libamalgame.a: msgpack`.
- Existing 610 tests unchanged — they keep the legacy "bundle .am
  as input" path so both compilation modes stay covered.
- Suite: 610 → **613 PASS** (216 core + 351 stdlib + 12 fmt + 34
  amc-new), 0 FAIL, 0 SKIP.

### Roadmap

`ROADMAP_COMPLET.md` banner marks project F as shipped.

### Migration notes

The legacy "pass `src/stdlib/<mod>.am` as an input" path still
works — `--external` is opt-in. To opt into the lib path:

```
amc -o myapp myapp.am --external src/stdlib/random.am
gcc -Iruntime myapp.c lib/libamalgame.a -lgc -lm -lcurl -lz -o myapp
```

Released artefacts ship `lib/libamalgame.a` in the same
directory layout as the binary, so an installed
`<install_prefix>/amc` has its lib at
`<install_prefix>/lib/libamalgame.a`.

External packages (`amc package add sqlite`, …) keep their own
precompile-on-install cache from v0.5.4 — that pathway is
unchanged.

## [v0.7.4] — 2026-05-12

The **"project G — inline-C blocks"** patch release. Adds a balisé
`@c { … }` block to Amalgame so methods can drop into C without a
runtime-header detour, plus file-scope `@c_include "<h.h>"` and
`@c_link "name"` directives. First POC migration:
`runtime/Amalgame_BuildInfo.h` → `src/stdlib/amc_buildinfo.am`. 8
new tests on top of v0.7.3 (602 → **610 PASS**). docs/guide chapters
4 + 5 caught up with everything shipped since v0.7.0.

### Inline-C blocks — `@c { … }`

```
public class CTools {
    public static int CLen(string s) {
        @c {
            return (int) strlen(s);
        }
    }
}
```

- **Lexer** scans the body verbatim, tracking nested-brace depth
  while skipping `}` inside C string literals (`"…"`), char literals
  (`'…'`), and `//` / `/* … */` comments — so braces in those
  contexts never close the block early.
- **Parser** dispatches the new `INLINE_C` token by type at the
  top of `ParseStmt` and produces a leaf `NodeKind.INLINE_C` node
  holding the body in `.Str`.
- **Resolver / typechecker / linter** treat the block as opaque
  by design — no symbols to resolve, no types to check.
- **CGen** splices the body inside a compound statement
  (`{ /* inline-C */ … }`) so any `int x = …;` declarations stay
  scoped to the block. The body's plain C `return …;` returns from
  the enclosing Amalgame method (the original proposal's
  `@out = expr;` was dropped — `return` is cleaner).
- **Formatter** re-emits `@c { … }` line-by-line so `amc fmt`
  round-trips inline-C blocks.

### File-scope directives — `@c_include` / `@c_link`

```
namespace App

@c_include "<ctype.h>"
@c_link "m"

public class CaseTools {
    public static int ToUpperByte(int b) {
        @c {
            return (int) toupper((int) b);
        }
    }
}
```

- **`@c_include "<header.h>"`** — emits `#include <header.h>` at
  the top of the generated `.c` (angle form when the argument
  starts with `<`, quoted form otherwise).
- **`@c_link "name"`** — surfaces as a `/* link: -lname */`
  comment in the emitted `.c` for the MVP. Threading the libs
  into `amc test`'s internal gcc step is a follow-up.

Both directives parse at file scope between class / enum
declarations; the parser peeks `@` + IDENT before falling through
to `ParseDecl` so the leading `@` doesn't get swallowed by the
decorator path.

### BuildInfo POC migration

```
// src/stdlib/amc_buildinfo.am
namespace Amalgame.Compiler

public class BuildInfo {
    public static string GitRev() {
        @c { return AMC_GIT_REV; }
    }

    public static string BuildDate() {
        @c { return AMC_BUILD_DATE; }
    }
}
```

`runtime/Amalgame_BuildInfo.h` shrank from 38 to 28 lines — only
the two `#ifndef AMC_GIT_REV` / `AMC_BUILD_DATE` guards remain.
`BuildInfo` removed from the cgen's `isCoreStdlib` list so the
call site lowers through the regular namespace-mangled dispatch
and matches the AM-emitted `Amalgame_Compiler_BuildInfo_GitRev`
symbol. `amc --version` still prints `commit … built …` as
before, now driven by AM code.

### CI — Windows MSYS2 unblocked

POSIX `<regex.h>` (since v0.7.1) and zlib link (since v0.7.2) were
never wired into the Windows MSYS2 CI job. Pre-existing legacy bug
fixed by adding `mingw-w64-x86_64-libsystre` + `mingw-w64-x86_64-zlib`
to the MSYS2 install and `-lz -lsystre -ltre` to both gcc invocations
(snapshot bootstrap + smoke sample compile). Windows CI green again.

### Roadmap

New section in `ROADMAP_COMPLET.md` — **"Runtime → AM migrations
(project G follow-up, optional)"** — listing 6 candidates that
*could* migrate post-v0.7.6 once project F (`libamalgame.a`
pre-compile) lands and makes the user-side ergonomics painless:
`DateTime`, `Logging`, `Crypto`, `Random`, `Service`, `FileWatch`.
Headers that stay in C are explicitly enumerated too (the
foundation, perf-critical primitives, vendor bindings).

The original "Inline-C injection blocks" design item under "Open
design questions" is now marked **SHIPPED in v0.7.4** with commit
refs.

### docs/guide catch-up

Chapter 4 (stdlib) gained eight new sections covering everything
shipped between v0.7.0 and v0.7.4 that the chapter hadn't been
updated to cover yet: `IO.FileWatcher`, `Math.Vec` (Vec3/Vec4/Mat4),
`Net.WebSocket`, `Formats.Yaml`, `Formats.MsgPack`, `Regex`,
`Compress`, plus a new `### UTC breakdown` subsection inside
DateTime documenting `Year()`/`Month()`/`Day()`/`Hour()`/`Minute()`/
`Second()` (v0.7.1).

Chapter 5 (runtime & C interop) gained a new option 3 under
"Calling C from Amalgame" walking through the inline-C surface,
the unsafe-like sandbox warning, and the BuildInfo POC as the
canonical migration example.

### Tests

- 4 new `inline-C: …` fixtures in `tests/samples/inline_c.am`
  (return value, multi-stmt with local, brace torture, void
  side-effect).
- 4 new `inline-C dir: …` fixtures in
  `tests/samples/inline_c_directives.am` (`@c_include "<ctype.h>"`
  + `@c_link "m"` driving `toupper` / `isalpha`).
- Suite goes from 602 → **610 PASS** (213 core + 351 stdlib + 12
  fmt + 34 amc-new), 0 FAIL, 0 SKIP.

### Migration notes

Existing callers of `BuildInfo.GitRev()` / `BuildInfo.BuildDate()`
(both internal to `amc --version` today) are unaffected — the call
site is unchanged, only the emitted symbol moves from
`BuildInfo_GitRev` to `Amalgame_Compiler_BuildInfo_GitRev`. Third-
party callers don't exist for this API.

The `runtime/Amalgame_BuildInfo.h` header still ships so the macro
guards (`#ifndef AMC_GIT_REV` / `AMC_BUILD_DATE`) remain in scope
for the inline-C bodies. Anyone vendoring that header in their own
project should keep it.

## [v0.7.3] — 2026-05-12

The **"WebSocket + final `.h` runtime"** patch release. 4 new
tests on top of v0.7.2 (598 → **602 PASS**). Marks the freeze
on new `runtime/Amalgame_*.h` headers — every stdlib module
from v0.7.6 onwards will land as a `.am` file via project G
(inline-C `@c { ... }` blocks).

### `Amalgame.Net.WebSocket` — RFC 6455 client

```
let ws = WebSocket.Connect("echo.websocket.org", 80, "/")
if (ws != null && ws.IsConnected()) {
    let _ = ws.SendText("hello")
    let reply: string = ws.ReceiveText()
    Console.WriteLine(reply)         // "hello"
    ws.Close()
}
```

Surface:
- **`WebSocket.Connect(host, port, path) → WebSocket?`** — TCP
  open + HTTP upgrade handshake (GET + Sec-WebSocket-Key/Accept
  with SHA-1 + Base64 verify). Returns `null` on any failure
  (refused TCP / DNS / HTTP non-101 / Sec-WebSocket-Accept
  mismatch).
- **`ws.SendText(s) → bool`** — client → server text frame
  (opcode 0x1, FIN set, masked per RFC).
- **`ws.ReceiveText() → string?`** — blocks for one server → client
  frame; transparently replies to Ping (0x9) with Pong (0xa) and
  reads the next non-control frame; returns `null` on Close
  (0x8) or read error; returns `""` for binary / continuation
  frames so callers can opt to ignore them.
- **`ws.Close()`** — sends a Close frame and shuts the TCP fd.
- **`ws.IsConnected()` / `ws.GetHost()` / `ws.GetPort()`** —
  trivial accessors.
- **`WebSocket.AcceptKey(clientKey) → string`** — derives the
  Sec-WebSocket-Accept value. Exposed so test fixtures (and
  third-party WS-aware code) can verify the handshake without
  reaching into the runtime.

Implementation lives in `runtime/Amalgame_WebSocket.h`,
self-contained: SHA-1 (FIPS 180-4) and Base64 (RFC 4648) are
inlined there since `Amalgame.Crypto` only ships SHA-256.
Frame parser handles both masked + unmasked server → client
frames defensively; payload size capped at 16 MiB so a buggy /
malicious server can't OOM the client.

Out of scope (next iterations):
- **`wss://` TLS** — needs an OpenSSL binding or a TcpTls
  layer; gets its own release.
- **Binary opcodes (0x2)** — same machinery, returns
  `List<int>` instead of `string`. Wait for a real consumer.
- **Continuation frames** — multi-fragment messages aren't
  reassembled in v1 (each frame is a complete message). Add
  when a real protocol needs them.
- **per-message-deflate negotiation** — handshake-level
  optimisation; pairs with `Amalgame.Compress` once a
  `Sec-WebSocket-Extensions: permessage-deflate` consumer
  shows up.
- **HTTP subprotocols** (`Sec-WebSocket-Protocol`) — easy add
  to `Connect()`'s signature when needed.

### 4 stdlib tests

- **RFC 6455 §1.3 canonical Sec-WebSocket-Accept** —
  client key `"dGhlIHNhbXBsZSBub25jZQ=="` must produce
  `"s3pPLMBiTxaQ9kYGzzhZRbK+xOo="`. Exercises the SHA-1 + Base64
  pair end-to-end without a live server.
- **Empty client key edge case** — AcceptKey("") returns a
  28-char base64 string (not a crash).
- **`WebSocket.Connect` to refused port** — returns `null`
  (well-defined "no server" path).
- **`WebSocket.Connect` to bogus DNS** — returns `null` on
  `getaddrinfo` failure.

Live-server integration testing via Python `websockets` is
opt-in; the runner skips it silently when the module isn't
installed. A dedicated WS-server harness lands in a follow-up
once a real consumer (LSP-over-WS?) needs end-to-end coverage.

### Roadmap reorientation

The freeze on new `runtime/Amalgame_*.h` headers takes effect
this release. The two follow-ups are:

- **v0.7.4 = project G — inline-C blocks (`@c { ... }`)** —
  major language feature. Once shipped, every stdlib module
  can be written as a `.am` file with C-level glue inlined,
  collapsing the `runtime/header.h + stdlib/facade.am` dance.
- **v0.7.5 = project F — `libamalgame.a` pre-compile** — the
  user-facing stdlib `.am` modules pre-compiled into a static
  library shipped with the amc binary. Today every `amc -o
  foo foo.am` re-parses ~10 `.am` stdlib modules; F drops
  that overhead.

Both projects are detailed in the "Open design questions"
section.

---

## [v0.7.2] — 2026-05-12

The **"stdlib expansion 3"** patch release. 23 new tests on top
of v0.7.1 (575 → **598 PASS**). Plus a cgen fix that drops a
paper-cut on chained collection-method calls.

### `Amalgame.Compress` — zlib gzip + raw-deflate codec

```
let body: List<int> = File.ReadBytes("payload.bin")
let gz: List<int>   = Compress.Gzip(body)
File.WriteBytes("payload.bin.gz", gz)

let back: List<int> = Compress.Gunzip(gz)
// back == body (byte-exact round-trip)
```

Four entry points, paired:

- **`Compress.Gzip` / `Gunzip`** — RFC 1952 wrapper. Same
  bytes `gzip -c` would produce, correct file-format magic,
  suitable for HTTP `Content-Encoding: gzip` or `.gz` files.
- **`Compress.Deflate` / `Inflate`** — RFC 1951 raw stream,
  no header, smaller, suitable for embedded protocols
  (WebSocket per-message-deflate, custom binary RPCs).
- **`Compress.GzipString` / `GunzipString`** — UTF-8 string
  convenience pair.

Input + output as `List<int>` byte buffers (one entry per
byte, 0..255). Same shape as `Amalgame.Crypto.Sha256` /
`Amalgame.Random.SystemBytes` — pipe-friendly.

Build adds `-lz` to every gcc invocation that already links
against `libgc` / `libcurl` (`build_amc.sh`, the user-compile
path in `main.am`, every test runner). 9 stdlib tests cover
empty input, gzip magic bytes, large-input shrink ratio,
round-trip exactness, and raw-deflate symmetry.

### `Amalgame.Formats.MsgPack` — binary codec via JsonValue

```
let bytes: List<int> = MsgPack.EncodeJson(jv)   // any JsonValue
let jv2: JsonValue   = MsgPack.DecodeJson(bytes)
```

MessagePack 1.0 subset, pure-Amalgame (no runtime header) on
top of `JsonValue`. Coverage:

| Marker            | Bytes        | Type                  |
|-------------------|--------------|-----------------------|
| `0xc0`            | nil          | `JsonValue.IsNull()`  |
| `0xc2` / `0xc3`   | bool         | `IsBool()` / `AsBool()` |
| `0xxxxxxx`        | positive fixint (0..127) | `IsInt()` |
| `111xxxxx`        | negative fixint (-32..-1) | |
| `0xd0` / `0xd1` / `0xd2` | int 8 / 16 / 32 | |
| `101xxxxx` + bytes | fixstr (0..31)  | `IsString()` |
| `0xd9` / `0xda` + len + bytes | str 8 / 16 | |
| `1001xxxx` + items | fixarray (0..15) | `IsArray()` |
| `0xdc` + 2-byte len | array 16 | |
| `1000xxxx` + pairs | fixmap (0..15) | `IsObject()` |
| `0xde` + 2-byte len | map 16 | |

Out of scope (v2): int64, float 32/64, bin, ext, timestamps —
each easy to add (encoder gets a branch, decoder gets a prefix
match), but not blocking for typical config / RPC payloads
where the fix-* small forms win.

14 stdlib tests cover encode + decode round-trips for every
type above, with edge-case nesting (fixarray of fixints,
fixmap of mixed values, single-key map, empty containers).

Round-trips through `JsonValue` mean any code that builds JSON
trees can switch to MsgPack with a one-line `Json.Stringify(jv)`
→ `MsgPack.EncodeJson(jv)` rename.

### CGen fix: chained `<call>.Count()` on collections

`EmitCalleeStr` used to emit `<bareReceiverType>_<methodName>`
verbatim for chained method calls. When the inner call returned
`AmalgameList*` / `Map*` / `Set*` (e.g. `jv.AsArray().Count()`),
the PascalCase `Count` collided with the C runtime's camelCase
`AmalgameList_count`, leaking an `-Wimplicit-function-declaration`
warning and an undefined symbol at link.

Fix downcases the first letter of `mname` when `bareR` is one
of the three collection structs. User classes still keep
PascalCase methods (`ArgParser_Flag` / `MsgPackCursor_Read`
etc. unchanged). Roadmap "Compiler — polish" item recoché.

### Deferred to v0.7.3

`Amalgame.Net.WebSocket` (RFC 6455 client) — ~1 day of focused
work for the handshake + SHA-1 Sec-WebSocket-Accept verify +
frame masking + ping/pong + a live-server test harness. The
test infrastructure is its own piece of work; better to land
it in a dedicated release. Tracked in the roadmap.

---

## [v0.7.1] — 2026-05-12

The **"stdlib expansion 2"** patch release — second installment
of the v1.0-bound stdlib fill-in. 43 new tests on top of v0.7.0
(532 → **575 PASS**).

### `Amalgame.Formats.Yaml` — YAML 1.2 subset reader

CI / app config use case:

```
let cfg = Yaml.Parse(File.ReadAll("app.yml"))
let port: int = cfg.Get("server").Get("port").AsInt()
let envs = cfg.Get("environments")  // YamlValue (Array)
let firstEnv = envs.At(0).Get("name").AsString()
```

Coverage:
- Block mappings (nested arbitrarily deep).
- Block sequences of scalars and of maps.
- Typed scalars: bool (`true` / `false`), int, float, plain
  string, single- and double-quoted strings (with `\n` / `\t` /
  `\\` / `\"` escapes in double quotes; `''` → `'` in single).
- Comments (`#` to end of line, with the leading-space rule
  for inline comments).
- Blank-line tolerance.
- Missing-key returns a Null-kind `YamlValue` so chains like
  `cfg.Get("a").Get("b").AsString()` stay readable.

Out of scope (raise an error or fall back to plain string —
not the goal of a config-reader subset): anchors / aliases,
multi-doc `---`, flow style `[1,2]` / `{a: b}`, multiline
scalars (folded `>`, literal `|`), tags (`!!str`).

`YamlValue` tree mirrors `TomlValue` shape so callers can
switch formats with a one-line `Toml.Parse` ↔ `Yaml.Parse`
rename.

### `Amalgame.DateTime` — UTC breakdown accessors

`Instant` gains six new methods:

```
let i = Instant.Now()
Console.WriteLine(String_FromInt(i.Year())  + "-"
                + String_FromInt(i.Month()) + "-"
                + String_FromInt(i.Day())   + " "
                + String_FromInt(i.Hour()))
```

`Year()` / `Month()` (1–12) / `Day()` (1–31) / `Hour()` (0–23) /
`Minute()` (0–59) / `Second()` (0–60 with leap-second tick) —
all UTC, via `gmtime_r` / `gmtime_s`.

Named timezones (`LocalTime`, `In(zone)`, `strftime`-ish
formatter, `+HH:MM` offset parsing) stay deferred — needs a
tzdata shipping strategy (bundle IANA vs. OS delegate via
POSIX `TZ` env / `/usr/share/zoneinfo` / Windows
`GetDynamicTimeZoneInformation`). The UTC half covers the
typical server-side / logging use case.

### `Amalgame.Regex` — POSIX extended-regex binding

```
Regex.Test("[0-9]+", "abc")                    // false
let m = Regex.Match("([a-z]+) ([a-z]+)", "foo bar")
m.GroupText(0)                                  // "foo"
m.GroupText(1)                                  // "bar"
Regex.ReplaceAll("[0-9]+", "a1b2c3", "X")       // "aXbXcX"
```

Surface:
- `Regex.Test(pat, subj) → bool` — cheap predicate; skips the
  match-result allocation.
- `Regex.Match(pat, subj) → Match*` (or `null` on no-match) —
  with `GetText` / `GetStart` / `GetEnd` / `GroupCount` /
  `GroupText(i)` / `GroupStart(i)` / `GroupEnd(i)` for the
  parenthesised groups (up to 16, indices 0..N-1).
- `Regex.Replace(pat, subj, repl)` — first occurrence; `\1`
  capture-expansion stays literal in v1.
- `Regex.ReplaceAll(pat, subj, repl)` — every non-overlapping
  occurrence, zero-length-match safe (steps past the match by
  at least one byte to avoid spinning on patterns like `^` or
  `.*`).

Syntax is POSIX extended (ERE) — the standard suspects work:
`. * + ? ^ $ [...] ( ) | {n,m}`. PCRE-only features (`\d` /
`\w` / `\s` shorthand, look-arounds, non-greedy modifiers,
named captures, Unicode property classes) are out of scope —
bind libpcre2 in a future package when a real consumer needs
them. `regex.h` is in libc everywhere we ship (POSIX, MinGW)
so no third-party dependency.

### Wiring recipe

Three places to register a new builtin runtime-backed type
follows from v0.7.0 — see commits `7360d4c` (v0.6.4 BuildInfo)
and `56b03c4` (v0.7.0 Vec/FileWatcher) for the pattern.

---

## [v0.7.0] — 2026-05-12

The **"stdlib expansion 1"** minor release — first installment of
the v1.0-bound stdlib fill-in. 23 new tests on top of v0.6.4
(509 → **532 PASS**).

### `Amalgame.Math.Vec` — Vec3 / Vec4 / Mat4

Scalar 3D math primitives for game / graphics / linear-algebra
use cases. Runtime in `runtime/Amalgame_Math_Vec.h`; facade in
`src/stdlib/math_vec.am`. The cgen short-circuits method calls
to the runtime helpers directly so user code reads at the right
level of abstraction without paying the namespace-dispatch cost.

```
let a = new Vec3(1.0, 2.0, 3.0)
let b = new Vec3(4.0, 5.0, 6.0)
let c = a.Cross(b)                // (-3, 6, -3)
let m = Mat4.RotateZ(Math.Pi() / 4.0)
let p = m.TransformVec4(new Vec4(1.0, 0.0, 0.0, 1.0))
```

Coverage:
- **Vec3** — ctor + GetX/Y/Z + Add/Sub/Scale/Dot/Cross/Length/
  Normalize/Equals.
- **Vec4** — ctor + GetX/Y/Z/W + Add/Sub/Scale/Dot.
- **Mat4** — Identity/Translate/Scale/RotateX/Y/Z/Multiply/
  TransformVec4/Get/Set. Column-major (OpenGL convention) so
  `glUniformMatrix4fv` works without transposition.

All operations heap-allocate via GC_MALLOC so chained calls
don't alias their inputs (functional style — `a.Add(b)` returns
a fresh `Vec3*`, leaving `a` and `b` untouched).

16 stdlib tests cover every method, including a verified
RotateZ(π/2) on (1, 0, 0, 1) → (0, 1, 0, 1).

Complex numbers and BigInt are deferred — different scope (no
GMP dep wanted); revisit when a real consumer needs them.

### `Amalgame.IO.FileWatcher` — single-file mtime polling

```
let w = new FileWatcher("/etc/foo.toml")
while (!Service.ShouldStop()) {
    if (w.Changed()) { reload() }
    // sleep loop
}
```

Cross-platform via `stat(2)` / `_stat64`. API:
- `new FileWatcher(path)` — snapshot the initial mtime.
- `w.Exists()` — true iff the file is currently present.
- `w.Changed()` — true the first call after the mtime advances
  (or the file appears / disappears); false on subsequent calls
  until the next change. After a true return, the watcher's
  internal snapshot is updated to the new mtime so the polling
  loop self-resets without extra bookkeeping.
- `w.GetPath()` — the path passed at construction.

Covers the "reload a config file" / "rebuild on source change"
80% use case. Directory-level recursive watches via `inotify` /
`FSEvents` / `ReadDirectoryChangesW` are deferred — add when a
real consumer needs them; the polling MVP works on every
filesystem amc compiles for.

7 stdlib tests use the delete / re-create flip for
deterministic results (mtime-advance granularity varies across
filesystems).

### Wiring

Three places to register a new builtin runtime-backed type in
the bootstrap pipeline (`Vec3`/`Vec4`/`Mat4`/`FileWatcher` all
follow the same recipe — see commit `56b03c4` for the diff):

1. `src/resolver/symbol.am` — add to the `builtins` list.
2. `src/resolver/resolver.am` — `DeclareGlobal("Type", "type", false)`
   plus one `DeclareGlobal` per runtime helper.
3. `src/generator/c_gen.am` — entries in `TypeToC`,
   `InferTypeFromExpr` (NEW_EXPR + CALL paths), `NEW_EXPR`
   emitter (new-keyword short-circuit), `isCoreStdlib`
   (static-call dispatch), `bareType`-strip path (instance-call
   dispatch), and an `#include` line in the auto-emitted prelude.

---

## [v0.6.4] — 2026-05-12

The **"cgen cleanup + profile"** patch release.

### Snapshot C output shrunk (-9%)

`amc_lib.c` goes from 22 860 → 21 361 lines (-1 499 lines) and
1.17 MB → 1.06 MB (-9%). Two source changes:

- cgen no longer emits a `__attribute__((unused))` marker on
  every `VAR_DECL` (3 342 occurrences gone).
- cgen no longer emits `(void)self;` / `(void)<param>;`
  boilerplate at the top of every method body (~2 000 lines
  gone).

The gcc invocations (`build_amc.sh` + `main.am` user-compile
flow) now pass `-Wno-unused-variable -Wno-unused-parameter
-Wno-unused-but-set-variable` instead. `amc --lint` is the
canonical "is this variable actually used" gate — it already
flagged unused locals, shadowing, the v0.6.3 catch-binder, and
var-never-reassigned. The trade is: gcc warnings hidden at
build, linter handles correctness; in exchange, every PR review
diff on the checked-in snapshot is meaningfully smaller, and
`compile + link` is a hair faster.

### Compile-time phase profiling

`amc --verbose foo.am` now prints per-phase timings on stderr:

```
  parse:     154us
  resolve:   199us
  typecheck: 9us
  cgen:      411us
Compiling: 1 file(s)
Generated: foo.c (30 lines) [Library]
Build OK
```

Powered by a single `Stopwatch` (`Amalgame.DateTime`) reset at
each phase boundary. Reveals the hot spot without external
profiling.

`./build_amc.sh` total wall time is now ~2 s end-to-end (was ~5 s
when the AST-cache roadmap entry was originally written), so the
proposed pickled-AST cache is no longer high-priority — would
risk a serializer-bug class for marginal gain. Revisit if a
single-file compile ever exceeds ~50 ms.

### Internal: Box/Unbox helpers consolidated

Audit confirmed `BoxAsVoid(expr)` and `UnboxScalar(ctype, expr)`
already serve every site that previously open-coded the
`(void*)(intptr_t)X` dance. Roadmap entry recoché [x] — no code
change.

---

## [v0.6.3] — 2026-05-12

The **"internal refactor"** patch release. Three items from the
"Compiler — internal refactoring & optimization" backlog.

### Reduce void* erasure on local containers

cgen's element/value-type inference for `xs.Get(i)` on a local
`List<T>` / `Map<K,V>` now consults the `__local__` /
`__local_map__` registry that `TrackGenericLocal` already
populates at the declaration site. Drops the
`let n: string = xs.Get(0)` workaround at most call sites:

```
// Before
let n = names.Get(0)   // names: List<string>
// → void* n = (void*)AmalgameList_get(names, 0);

// After
let n = names.Get(0)
// → code_string n = (code_string)AmalgameList_get(names, 0);
```

Class-method return types were already propagating since v0.5;
the gap was specifically on the generic-collection accessors.

### Linter coverage

`amc --lint` learns two new diagnostics:

- **catch-binder unused** — `try { ... } catch e { ... }` where
  the binder isn't read warns `unused local 'e' (prefix with
  '_' to silence)`. Same opt-out as other unused locals.
- **`var`-declared-but-never-reassigned** — flags `var x = 0;
  print(x)` where `x` is read but never reassigned, suggesting
  `let` would communicate the immutability better. Picks up
  every `=` / `+=` / `-=` / `*=` / `/=` / `%=` / `&=` / `|=` /
  `^=` / `<<=` / `>>=` operator as a reassignment.

Still TBD (each needs typecheck integration or a non-trivial
walk): suspicious match (missing default + non-exhaustive
enum), implicit fallthrough, dead `import`.

### `ArgParser` framework

`src/argparser.am` ships a fluent registration class for the
subcommand arg loops that were duplicated across `migrate.am`,
`generate.am`, `explain.am`, `main.am::RunFmt`/`RunTest`, and
`add_cmd.am`:

```
let ap = new ArgParser()
ap.Flag("-w").Flag("--write").Flag("-h").Flag("--help")
  .Option("--from")
  .Parse(argc, 2)

if (ap.HelpRequested()) { ... }
if (String_Length(ap.GetUnknown()) > 0) {
    Console.WriteError("amc fmt: unknown argument '"
        + ap.GetUnknown() + "'")
}
let write = ap.HasFlag("-w") || ap.HasFlag("--write")
let files = ap.GetPositionals()
```

`RunFmt` migrated as the inaugural caller. Other subcommands
can move to the framework incrementally without coordinating a
big-bang rewrite. Lacks short-flag clustering (`-vh`),
`--key=value` form, and `--` end-of-flags marker — add when a
real caller needs them.

---

## [v0.6.2] — 2026-05-12

The **"compiler audit + parser bugfix"** patch release.

### Roadmap audit

Six items in the "Compiler — internal refactoring & optimization"
section had been silently resolved by intervening compiler work
(NodeKey fix, cgen chained-call handling, constructor forward-
decl pass) but were never re-checked. Audit pass verified each
via local repros and marked them `[x]`:

- `>> N` inside a `let` no longer drops the shift operator.
- Constructor forward-declarations now precede call sites.
- `while (cur != null)` traverses a linked list without GC drama.
- Typechecker return-type for enum members in IF bodies is
  correct (NodeKey fix from null-safety closed the leak).
- Chained `obj.Field.M()` and `obj.M().M()` both lower right.
- `Snapshot size` item (b) — `.gitattributes merge=ours` —
  shipped already; split the remaining "shrink C output" half
  into its own item.

### Parser: diagnose TS-style free functions

`public List<int> Helper(int n) { ... }` at file scope used to
parse without complaint; the cgen then emitted the call site as
`Helper(...)` against no definition, leaking a
`-Wimplicit-function-declaration` warning into the user binary.
The parser already caught the `fn name(...)` form; this release
extends the same lookahead-and-diagnose pattern to TS/C-style
return-type-then-name signatures:

```
$ amc --check helper.am
Top-level functions aren't supported (got 'Helper' at 2:8).
Wrap it inside a class as `public static`.
```

`ParseDecl` scans the next ~16 tokens for an `IDENT (` pair
before any `{ / ; / }` punctuator and emits the error, then
skips past the body. False positives only on top-level bare
calls (already invalid Amalgame).

---

## [v0.6.1] — 2026-05-12

The **"package-manager polish + ship-to-prod"** release. Eight QoL
items landed on top of v0.6.0, plus a `--version` rework that
embeds build provenance.

### New `amc package` verbs

- **`amc package info <name>`** — single-package detail view:
  description, url, tier, licence, category, maintainer, every
  indexed version with compat marker, and whether the package is
  currently installed in this project (matched by URL slug
  against `amalgame.lock`).
- **`amc package outdated`** — cross-references the lockfile
  against the index and lists installed deps with a newer
  compatible tag, ready to feed into `amc package update`. Silent
  on up-to-date deps (grep-friendly in CI); unindexed deps go to
  stderr. Exit code stays 0 in non-error cases.
- **`amc package notice`** — aggregates each installed package's
  `[package].license / authors / description` from its cached
  `amalgame.toml` into a NOTICE-style listing on stdout, ready to
  redirect into `NOTICE_DEPS.md` for downstream commercial
  redistribution / Apache-2.0 NOTICE propagation.
- **`amc package check [--frozen]`** — verifies `amalgame.lock`
  matches the installed cache. Bare form is informational
  (always exits 0); `--frozen` exits 1 on mismatch — drop-in for
  CI fail-fast lanes that catch a PR bumping the lockfile
  without re-installing.

### Flags on existing verbs

- **`amc package search --no-versions`** — skip the per-package
  versions block for faster browsing of long search results.
- **`amc package versions --json`** — machine-readable output
  (jq / CI compat probes); schema is stable.

### Compiler / cgen

- **`Path.{Combine,Sep,IsAbsolute,Normalize}`** now lower to
  their flat runtime symbol without requiring
  `import Amalgame.Path` — the four method names match the
  runtime 1:1 so it's safe. `Directory` / `Filename` /
  `Extension` / `Stem` still need the import (they wrap
  `Path_Get*` and would mis-mangle without the namespace
  dispatch). Removes ceremony from `amc`'s own code.

### `amc --version` enriched

Bakes the git short-SHA + UTC build timestamp at link time via
`-DAMC_GIT_REV=...` / `-DAMC_BUILD_DATE=...` (`build_amc.sh`
wires both; fall back to "" cleanly when the defines are
absent). Banner now surfaces author / licence / website /
repository / issues URLs:

```
amc 0.6.1 (commit abc12345, built 2026-05-12T01:23:45Z)
Self-hosted Amalgame compiler.
Copyright (c) 2026 Bastien Mouget. License: Apache-2.0.
Website:    https://amalgame.me
Repository: https://github.com/amalgame-lang/Amalgame
Issues:     https://github.com/amalgame-lang/Amalgame/issues
```

New `runtime/Amalgame_BuildInfo.h` exposes the defines via
inline `BuildInfo_GitRev` / `BuildInfo_BuildDate` helpers;
`BuildInfo` joins the cgen's core-stdlib dispatch list so
`BuildInfo.GitRev()` lowers without ceremony.

### Help-text audit

`amc package add --help` now documents the `<shortname>`
auto-resolve form (v0.6.0), the `--no-precompile` flag (v0.5.4),
and the full semver operator set on `required-amalgame`. The
top-level `amc --help` verb list adds the new verbs (`versions`,
`info`, `outdated`, `notice`, `check`). Fixes a stray
`amc update` typo in `update`'s missing-arg error path. Drops
the reference to the never-shipped `amc package gc` from
`remove --help`. `PrintPackageUsage` surfaces the `[@<tag>]`
safety suffix on `remove`.

### Documentation

- `ROADMAP_COMPLET.md` — per-release breakdown of the package-
  manager entry (v0.5.3 → v0.6.1) + `Path.*` dispatch note +
  new entry for unifying the ~1.9k lines of bash test runners
  under `amc test`.
- `docs/proposals/amalgame-package-manager.md` — documents the
  packages-index schema v2 (`[[version]]` array), back-compat
  matrix, ordering convention, and 30-min TTL.

### External repos

- `amalgame-database-nosql-redis` + `amalgame-messaging-mqtt` CI
  matrix bumped to `[v0.6.0, v0.5.0]` — catches regressions on
  the latest amc while keeping the baseline coverage.
- `amalgame-database-sqlite` v0.2.1 orphan tag deleted (it was a
  red CI history entry with no release attached).

---

## [v0.6.0] — 2026-05-12

The **"auto-resolve + semver constraints"** release. Two related
package-manager improvements that mature the install surface
toward what Cargo / npm users expect.

### Auto-resolve `amc package add <pkg>` (no tag)

```
$ amc package add duckdb
Auto-resolved 'duckdb' → 'duckdb@v0.1.1' (latest compatible with amc 0.6.0)
Resolving shortname 'duckdb' via amalgame-lang/packages-index...
  → github.com/amalgame-lang/amalgame-database-duckdb (official)
…
Added duckdb v0.1.1
```

Drop the `@<tag>` suffix on **indexed** shortnames — amc fetches
the index, walks the package's `[[version]]` entries newest-last,
and picks the first one whose `required-amalgame` is satisfied by
the running amc. Full git URLs still need an explicit `@<tag>`
(the index is the SoT for "what's known to amc"; unindexed repos
opt out by design).

### Semver constraints in `required-amalgame`

`PackageRegistry.VersionSatisfies` learned more operators:

| Operator | Example         | Semantics                          |
| -------- | --------------- | ---------------------------------- |
| `>=`     | `>=0.5.0`       | At least (existing)                |
| `>`      | `>0.5.5`        | Strictly greater                   |
| `<=`     | `<=0.5.5`       | At most                            |
| `<`      | `<0.5.5`        | Strictly less                      |
| `=`      | `=0.5.5`        | Exact match                        |
| `^`      | `^0.5.3`        | Caret (locks major, or minor for 0.x) |
| `~`      | `~1.2.3`        | Tilde (locks major.minor)          |
| bare     | `0.5.0`         | Treated as `>=` (back-compat)      |

Caret is the npm/Cargo flavour: `^1.2.3` ↔ `>=1.2.3, <2.0.0` for
`major > 0`; `^0.Y.Z` ↔ `>=0.Y.Z, <0.(Y+1).0` because 0.x minor
bumps may break; `^0.0.Z` ↔ `=0.0.Z`. Tilde always locks `major.minor`.

19 new stdlib tests cover every operator + edge cases (caret 1.x
/ 0.x / 0.0.x branches, tilde minor reject, bare-as-ge fallback).

---

## [v0.5.6] — 2026-05-12

The **"index TTL + QoL"** patch release.

### Index cache TTL

`amc package search` / `versions` / `add` previously cached the
packages-index **forever** at `~/.amalgame/cache/packages-index.toml`.
Bumping a tag in a package repo + waiting for the auto-PR to merge
on the index didn't surface until the user manually ran
`--refresh` or `amc package cache clear`. Annoying.

v0.5.6 adds a **30-min TTL** via `date -r <file> +%s` (POSIX +
MSYS2 + Cygwin). Older than that → automatic re-fetch on the
next `search` / `versions` / `add`. Network failure during
refresh falls back to serving the stale cache with a warning
(better than a hard error when offline).

### Network failure resilience

When the index can't be fetched (offline, 5xx, etc.) but a
cached file exists, `FetchIndex()` now returns the stale cache
with a `warning: index fetch failed, serving stale cache`
message on stderr. Previously: hard error. Tradeoff: slightly
stale data over no-go.

### External packages — test runner fixes

`amalgame-database-nosql-redis` and `amalgame-messaging-mqtt`
had `tests/run_tests.sh` still using the v0.5.0-era `amc add`
syntax (removed in v0.5.1 by PR #303). Their CI was silently
SKIPping every case for months. Now both runners use the
AMALGAME_PACKAGES_DIR + symlink trick, matching what SQLite
and DuckDB do.

---

## [v0.5.5] — 2026-05-11

The **"search-with-versions"** release. `amc package search` and a
new `amc package versions <pkg>` verb show every indexed version
of a package with **compat status** against the running amc — no
more guessing which tag to pass to `amc package add`.

### Index schema bump (packages-index v2)

[`amalgame-lang/packages-index`](https://github.com/amalgame-lang/packages-index)
gains a flat `[[version]]` array — one entry per (shortname, tag)
pair, linked to `[[package]]` by the `package` field:

```toml
[[version]]
package           = "duckdb"
tag               = "v0.1.1"
required-amalgame = ">=0.5.4"
```

amc reads these and shows compat per tag during `search` / `versions`
without cloning any repo. Back-compat: amc v0.5.4 and older ignore
the new array; the index keeps the existing `[[package]]` schema.

DuckDB is now registered (was missing in the v0.5.3 / v0.5.4 index).

### New: `amc package search` with versions inline

```
$ amc package search duckdb
✓ duckdb — DuckDB binding — vendored C++ amalgamation (MIT)…
    github.com/amalgame-lang/amalgame-database-duckdb (official, Apache-2.0)
    versions:
      v0.1.1 ✓ (needs amc >=0.5.4)  ← latest compatible
      v0.1.0 ✓ (needs amc >=0.5.3)
```

Each tag shows `✓` / `✗` against the running amc. The newest
compatible one gets a `← latest compatible` marker so the user
knows which `add` invocation to type.

### New: `amc package versions <name>`

```
$ amc package versions sqlite
sqlite — github.com/amalgame-lang/amalgame-database-sqlite
    versions:
      v0.2.0 ✓ (needs amc >=0.5.0)  ← latest compatible
```

Shortcut for `search <name>` filtered to one package. Cheap
discovery: "what versions of X work with my amc?"

### `--refresh` for index cache

`amc package search --refresh` (and `versions --refresh`) drop
the cached `~/.amalgame/cache/packages-index.toml` before re-
fetching. Useful when a new version was just published.

### `amc package list` shows pinned version

```
$ amc package list
1 package(s) installed:

  DuckDB @ v0.1.1 — amalgame-database-duckdb
    namespace: Amalgame.Database.DuckDB
    header:    /home/.../runtime/Amalgame_Database_DuckDB.h
```

The tag now lives on `LoadedPackage.Tag`, populated by
`PackageRegistry.LoadFrom` from the lockfile's `[[package]].tag`.

### `amc package remove` accepts `@<tag>` safety suffix

```
$ amc package remove duckdb@v0.1.1   # safe: verify-then-strip
$ amc package remove duckdb          # bare: strip whatever is pinned
```

When the `@<tag>` suffix is present, amc refuses to remove unless
the installed tag matches. Protects against typo'ing the wrong
version after an `amc package update`.

---

## [v0.5.4] — 2026-05-11

The **"precompile-on-install + cross-platform home"** release. Two
groups of work that share a single new helper, hence one PR.

### Package manager — precompile-on-install

Heavy C/C++ packages (DuckDB, future Postgres static builds) can
now declare:

```toml
[stdlib]
precompile = true
```

When that flag is set, `amc package add foo@vX.Y.Z` compiles each
`[stdlib].sources` entry **at install time** into a persistent
cache at:

```
~/.amalgame/packages/<host>/<owner>/<repo>/<tag>_<sha>/
└─ build/<platform>/<class>-<basename>.o
```

`amc test` and `amc build` look there first (before the v0.5.2
`/tmp/amc-pkg-*.o` lazy cache), so the 5-minute DuckDB compile is
paid **once** at install instead of every fresh `amc test`. Survives
reboots. Wiped by `amc package remove`.

Opt-out: `amc package add foo@vX.Y.Z --no-precompile` to skip even
if the manifest says yes (useful in CI batch installs where you
want install separated from build).

### Calibration — auto-learning ETA

Each precompile writes a sample to `~/.amalgame/calibration.toml`:

```toml
[[sample]]
lang      = "cxx"
size_kb   = 25500
elapsed_s = 167
pkg_ver   = "amalgame-database-duckdb@v0.1.0"
```

Future `amc package add` reads these samples and shows an ETA
derived from the weighted average (lang-bucketed) on this specific
machine. No hardcoded "5 min" lie in manifests — the estimate
self-corrects to your CPU over a few installs. First-time users
get an honest "no ETA yet" message plus elapsed time after.

### Cross-platform `$HOME` resolution

`PackageRegistry.AmalgameHome()` resolves the user state dir in
order:

1. `$AMALGAME_HOME` (explicit override)
2. `$HOME` (POSIX, MSYS2, Cygwin, WSL — always set)
3. `$USERPROFILE` (Windows native cmd.exe / PowerShell)

Fixes a pre-existing bug where amc fell back to `/tmp` on Windows
natively (cmd.exe / PowerShell don't set `$HOME`). MSYS2-only CI
wasn't exercising this. All path concatenation now goes through
`Path_Combine` which handles both `/` and `\` separators.

### Other

- New `PackageRegistry.PlatformTag()` returns lowercased `os-arch`
  (`linux-x86_64`, `macos-arm64`, `windows-x86_64`) so multi-OS
  cache layouts don't collide on a shared `$HOME`.
- 10 new stdlib tests cover precompile / PkgDir / platform tag /
  Calibration round-trip (240 PASS total, up from 231).

---

## [v0.5.3] — 2026-05-11

The **"C++-bearing packages"** release. Lays the wiring needed
to ship vendored C++ amalgamations (DuckDB landing first) without
forcing the user to know anything about g++ or libstdc++.

### Package manager — manifest schema growth

Four new optional keys land on `[stdlib]`:

```toml
[stdlib]
sources  = ["foo.cpp"]                  # extension drives g++ vs gcc
cflags   = "-DFOO=1"                    # extra flags for .c sources
cxxflags = "-O3 -DNDEBUG -std=c++17"    # extra flags for .cpp/.cc/.cxx
libs     = ["stdc++", "m"]              # bare names → -l<name> at link
```

Plus one on `[package]`:

```toml
[package]
schema-version = 1   # rejects installs when amc supports < value
```

`PackageRegistry.LoadedPackage` carries the new fields; `amc package
add` refuses to install a package declaring a higher
`schema-version` than the running amc understands (parallel to the
existing `required-amalgame` gate).

### `amc test` — C/C++ dispatch

`Program.PreCompilePackageSources` now switches between `gcc` and
`g++` based on file extension (`.cpp` / `.cc` / `.cxx` → g++). When
any installed package contributes a C++ source, `RunTest` switches
the final-link compiler from `gcc` to `g++` so libstdc++ + C++
static-init order resolve correctly (a plain `gcc -lstdc++` doesn't
guarantee the latter). Per-package `cflags` / `cxxflags` get spliced
into the per-source pre-compile; `libs` get spliced as `-l<name>` at
the final link.

### Standard library

- `Amalgame.Database.DuckDB` — DuckDB binding, shipped as a new
  external package `amalgame-lang/amalgame-database-duckdb` (vendored
  C++ amalgamation, MIT). Embedded analytical (OLAP) database. See
  the [stdlib chapter](docs/guide/04-stdlib.md#databaseduckdb--embedded-analytics-via-opt-in-package).

### Gotchas retired

- **`amc test` doesn't link C++ packages** — gone in v0.5.3 (this
  release). The dispatch + per-package flags + libs make the path
  symmetric to v0.5.2's C-only auto-link.

---

## [v0.5.2] — 2026-05-11

The **"package CLI polish + amc test linking"** release. Two
batches of work since v0.5.0 land under one tag (v0.5.1 was never
cut as a standalone tag — its content is rolled into v0.5.2):

### CLI grouping — `amc package <action>` (PR #303)

The flat verbs `amc add` / `amc remove` / `amc search` / `amc list` /
`amc update` / `amc cache` are now grouped under a single
subcommand, dotnet-style, with `amc pkg` as a short alias:

```
amc package add github.com/amalgame-lang/amalgame-database-sqlite@v0.2.0
amc package list
amc package search redis
amc pkg cache gc
```

**Breaking** vs v0.5.0: the flat forms are removed. Update any
scripts or CI that called `amc add` to `amc package add`.

### `amc test` is package-aware

- **Auto-install missing deps** (PR #302) — before discovery, the
  runner reads `amalgame.lock`, compares against the local cache,
  and `git clone`s any missing tag into `~/.amalgame/packages/`.
  No more "I cloned a fresh checkout and the tests fail until I
  manually run `amc package add`".
- **Auto-link vendored sources** (PR #304) — packages declaring
  `[stdlib].sources` in their manifest get their `.c` files
  pre-compiled to cached `/tmp/amc-pkg-*.o` objects, then spliced
  into every test binary's `gcc` invocation alongside `-ldl
  -lpthread`. SQLite tests now work out-of-the-box; future
  vendoring backends (DuckDB etc.) get this for free.

### Docs refresh (PR #305)

- `03-cli-reference.md` documents the new `amc package <action>`
  table.
- `04-stdlib.md` SQLite section updated to the new install command
  and explains the automatic linking.
- `07-internals.md` `amc test` runner now documents the pre-compile
  step + `amcRuntime` resolution + `-ldl -lpthread` link flags.
- `ROADMAP_COMPLET.md` ticks the package-manager item off the
  planned list.

### Behind the scenes

- `LoadedPackage` carries a `Sources: List<string>` field populated
  from `[stdlib].sources` in each package's manifest.
- New helper `Program.PreCompilePackageSources(registry,
  amcRuntime)` walks the registry once per `amc test` run.
- `RunTest` resolves `amcRuntime` once up front (env →
  `dirname(amc)` → `./runtime`) instead of per-test-file.

---

## [v0.5.0] — 2026-05-11

The **"package manager + ecosystem launch"** release. v0.5 is the
biggest architectural shift since v0.3 self-hosting: the
monolithic compiler bundle is split into a **lean core** plus an
**opt-in package ecosystem**, with a real package-manager CLI,
manifest format, lock file, and 3 inaugural external packages
shipped alongside this release.

### Headline: `amc add`

```
amc add github.com/amalgame-lang/amalgame-database-sqlite@v0.2.0
```

…clones an external package, validates its manifest, writes the
project's `amalgame.toml` + `amalgame.lock`, and from that point
on `import Amalgame.Database.SQLite` Just Works in user code.

### 3 inaugural external packages

The previously bundled optional backends are now self-hosted in
their own repos with their own CI, their own release cadence,
their own tests:

| Package | Replaces (pre-v0.5) | License |
|---|---|---|
| [`amalgame-lang/amalgame-database-sqlite@v0.2.0`](https://github.com/amalgame-lang/amalgame-database-sqlite) | bundled SQLite in v0.4.15 | Apache-2.0 over public-domain SQLite |
| [`amalgame-lang/amalgame-database-nosql-redis@v0.2.0`](https://github.com/amalgame-lang/amalgame-database-nosql-redis) | bundled Redis in v0.4.17 | Apache-2.0 |
| [`amalgame-lang/amalgame-messaging-mqtt@v0.2.0`](https://github.com/amalgame-lang/amalgame-messaging-mqtt) | bundled MQTT in v0.4.17 | Apache-2.0 |

User-facing Amalgame source is **unchanged** — same
`SQLite.Open(":memory:")` / `Redis.Set(r, k, v)` /
`MQTT.Publish(m, t, p)`. Only difference: the dep is declared
explicitly in `amalgame.toml` instead of being bundled.

### Added — package manager pipeline

- **`Amalgame.Formats.Toml`** (PR #283) — TOML 1.0 subset parser
  + serializer. ~480 LoC pure Amalgame. Supports tables, nested
  tables, inline tables, arrays, basic + literal strings,
  integers, booleans, `#` comments, and `[[foo]]` array-of-
  tables (PR #285, needed for the lock-file format).
- **`Amalgame.Formats.*` umbrella** — `Amalgame.Json` migrated
  to `Amalgame.Formats.Json`. Future XML / YAML / CSV / INI /
  MessagePack / CBOR fit naturally. Breaking for v0.4.x users
  but trivial find-replace; no compat shim.
- **`amc add <git-url>@<tag>`** (PR #284) — package installer.
  Cache layout `~/.amalgame/packages/<host>/<owner>/<repo>/<tag>_<sha>/`.
  Idempotent (re-running is a no-op). Validates manifest at
  install: `[package].name` matches URL slug, `version` matches
  tag, `license` declared, `required-amalgame` constraint is
  satisfied by the running compiler.
- **`PackageRegistry`** (PR #286) — compiler-side loader reads
  `amalgame.lock` + each cached manifest, exposes a typed
  registry. Single source of truth for the resolver + cgen.
- **Resolver wiring** (PR #287) — declares each package's class
  + functions as globals so user code resolves cleanly without
  any per-package hardcoding in `src/resolver/resolver.am`.
- **CGen wiring** (PR #288) — isStdlib short-circuit consults
  the registry; return-type table is manifest-driven; prelude
  `#include` lines are emitted per installed package.
- **End-to-end pipeline test** (PR #289) — fixture exercises
  Toml → PackageRegistry → Resolver → CGen on a fake package
  without any network call.
- **Namespace-mangled C symbols** (PR #295) — package class
  methods lower to the namespace-prefixed C name
  (`Redis.Open` → `Amalgame_Database_NoSQL_Redis_Open`) so two
  packages exposing the same short class can't collide at link
  time. Core stdlib (Console / File / Math / …) keeps its flat
  `Class_Method` symbols — single-author, no risk.
- **`required-amalgame` compat gate** (PR #291) — package
  manifests declare `required-amalgame = ">=X.Y.Z"`. amc
  refuses to install if the running compiler version doesn't
  satisfy it (errors at install instead of mysteriously at
  gcc-link time).

### Removed — optional backends moved to packages

- `runtime/Amalgame_Database_SQLite.h` + the 9MB vendored
  amalgamation (`runtime/Amalgame_Database/sqlite/sqlite3.{c,h}`)
  → `amalgame-lang/amalgame-database-sqlite`
- `runtime/Amalgame_Database_Redis.h`
  → `amalgame-lang/amalgame-database-nosql-redis`
- `runtime/Amalgame_Messaging_MQTT.h`
  → `amalgame-lang/amalgame-messaging-mqtt`
- All hardcoded isStdlib / return-type / DeclareGlobal entries
  for the three classes — replaced by manifest-driven dispatch.
- The corresponding test fixtures and runner blocks — moved to
  each package's own `tests/run_tests.sh`.

### Changed — architectural

- **Main repo binary size** drops from ~470KB to ~370KB —
  9MB of vendored SQLite no longer ships with amc.
- **Main repo CI** tests only the compiler + core stdlib + the
  package-manager pipeline. No external server dependency.
  Clone-and-test runs green out of the box; no Redis, mosquitto,
  or SQLite-dev install required.
- **Each package** owns its own tests, CI workflow, release
  cadence. Bug fixes ship from the package repo without a
  compiler release.

### Tests / infra

- Main repo suite: **205 core / 219 stdlib / 12 fmt / 34 amc-new
  = 470 PASS / 0 FAIL / 0 SKIP** (was 438/24 in v0.4.17 — the
  SKIPs were Redis + MQTT cases waiting for a server, now those
  live in the package repos).
- 14 PRs landed (#282–#295). Develop accumulation per the v0.5
  batching policy — no intermediate patch releases since v0.4.17.

### Migration from v0.4.x

User code at v0.4.17 today:

```amalgame
import Amalgame.Database.SQLite
let db = SQLite.Open(":memory:")
```

…keeps working at v0.5.0 — but the dep must now be declared:

```bash
amc add github.com/amalgame-lang/amalgame-database-sqlite@v0.2.0
```

Same for Redis + MQTT. Json users update one import:
`import Amalgame.Json` → `import Amalgame.Formats.Json`. No
compat shim — pre-1.0 language, breaking renames are documented.

### Design doc

Full design captured in [`docs/proposals/amalgame-package-manager.md`](docs/proposals/amalgame-package-manager.md).

### Roadmap — v0.5.x + v0.6

- **v0.5.x** patch follow-ups (no breaking):
  - Transitive dep resolution + cycle/conflict detection
  - `amalgame-lang/packages-index` shortname lookup
    (`amc add redis@v0.2.0`)
  - `amc remove` / `update` / `list` / `search`
  - Path deps (`{ path = "../foo" }`)
  - Semver range constraints (`^X.Y.Z`, `~X.Y.Z`)
- **v0.6**: multi-version coexistence (Cargo-style dual-link
  enabled by the v0.5 mangling).

---

## [v0.4.17] — 2026-05-11

The "Redis client + LSP folding" release. Closes the last
explicitly-requested LSP slice (foldingRange, slice 5) and ships
the second `Amalgame.Database.*` brick after SQLite — this time
a pure-protocol client with no vendored lib, opening the NoSQL
branch of the namespace. Also a small fix to the release-flow
tooling so it stops emitting the `Co-Authored-By: Claude` trailer
that contradicts the project's authorship policy. Suite grows to
**438 PASS / 0 FAIL / 14 SKIP** (the 14 SKIP are Redis cases that
need a running server; the runner gates on a TCP probe so they
SKIP cleanly when no server is up).

### Added

- **`Amalgame.Database.NoSQL.Redis`** (PR #277) — pure-protocol
  RESP2 client over raw TCP, no vendored client lib. The
  runtime header (`runtime/Amalgame_Database_Redis.h`, ~340 LoC)
  reuses the cross-platform socket layer from `Amalgame_Net.h`
  so it works against Redis / KeyDB / Dragonfly / Valkey
  unchanged. Surface: `Redis.Open(host, port)`, `Close`,
  `IsOpen`, `LastError`, `Ping`, `Set`, `Get`, `Del`, `Exists`,
  `Incr`, `Decr`, `Expire`. Namespace nested under
  `Amalgame.Database.NoSQL.<Engine>` so siblings (Mongo,
  Cassandra, DynamoDB) can land alongside. AUTH / SELECT,
  pipelining, pub/sub, MULTI/EXEC transactions, SCAN, TLS,
  binary values with embedded NULs deferred to v2.
- **`amc lsp` foldingRange** (PR #276, slice 5) — token-driven
  brace-pair matching for class / method / block bodies; plus
  runs of `//` comments (`kind:"comment"`) and consecutive
  `import …` statements (`kind:"imports"`). Filters one- and
  two-line blocks so the gutter stays uncluttered. Capability
  advertised as `foldingRangeProvider: true`; VS Code and other
  LSP clients wire the chevron rendering automatically.

### Changed

- **`tools/release.sh`** (PR #275) — drop the hardcoded
  `Co-Authored-By: Claude` trailer from the release-commit
  message it generates. Aligns the script with the project's
  authorship policy stated in `NOTICE.md` (AI is a tool, not
  a co-author at law). The manual v0.4.16 release flow already
  followed this; the patch makes the automated path do the
  same so the next `./tools/release.sh` invocation stays
  compliant without manual edits.

### Roadmap

- **v0.5 — stdlib package manager + DB-backend extraction.**
  Once 3–4 optional backends are in tree (Redis is #2; DuckDB,
  Postgres, MQTT are the natural next), the monolithic-stdlib
  model starts hurting and a real package manager pays off.
  Design space: manifest format (`package.am` /
  `amalgame.toml`), `amc add <repo-url>` CLI that git-clones
  into `~/.amalgame/packages/`, resolver search path,
  cgen-plugin manifest replacing today's hardcoded isStdlib
  list, linker flags + vendored .c amalgamations declared per
  package. Inaugural packages: `amalgame-lang/amalgame-sqlite`
  and `amalgame-lang/amalgame-redis`, both extracted from the
  current monolithic stdlib as proof the design holds against
  two distinct shapes (vendored amalgamation vs pure protocol).

### Tests / infra

- Suite grows to **438 PASS / 0 FAIL / 14 SKIP** (was 434).
  +4 LSP cases for the new foldingRange handler, +14 Redis
  cases gated on a TCP reachability probe.
- Snapshot refreshed.

---

## [v0.4.16] — 2026-05-11

A small roll-up release that lands the post-v0.4.15 develop
accumulation: a roadmap expansion across the database + messaging
families, a stale-doc refresh, and one dead-script removal. No
code changes — the test suite stays at **434 PASS / 0 FAIL / 0
SKIP** and the snapshot is refreshed unchanged.

### Changed

- **`ROADMAP_COMPLET.md`** (PRs #270, #271) — fleshes out two
  stdlib families that previously sat as one-liners:
  - `Amalgame.Database.<Engine>` siblings now spell out Oracle
    (Instant Client dynamic-link, proprietary download) and SQL
    Server (MS ODBC default + FreeTDS fallback) alongside the
    already-tracked DuckDB / Postgres / MySQL. The NoSQL branch
    adds MongoDB (libmongoc + libbson), Redis (pure-Amalgame
    RESP3 client ~300 LoC), DynamoDB / Cosmos DB / Firestore
    (HTTP over Net.Http + Json), Cassandra / ScyllaDB (CQL
    binary protocol).
  - New `Amalgame.Messaging.<Broker>` family — pure-Amalgame
    MQTT (~300 LoC) + NATS Core (~250 LoC), plus dynamic-link
    backends for Kafka (librdkafka) and RabbitMQ (librabbitmq
    AMQP). Sits parallel to the database namespace so brokers
    and storage engines stay separately namespaced.
- **`CONTINUATION.md`** (PR #272) — refreshed for v0.4.15. The
  file had drifted to v0.4.4 state (Path / Logging / Service /
  Database.SQLite were all missing from the stdlib block); now
  reflects the current 434-test self-hosted state, the four
  closed LSP slices, the `amc new` service template, and the
  authorship / contribution policy bullets.

### Removed

- **`CLEANUP.sh`** (PR #269) — referenced bootstrap artefacts
  (`build/`, `stage1/`, `stage2/`) that haven't existed since
  the self-hosting transition. Dead code; deleting it is a net
  reduction in "wait, what does this do?" surface area.

### Tests / infra

- No code changes; snapshot refreshed via `tools/save-snapshot.sh`
  for consistency with the bumped `--version` string.
- Test suite: **434 PASS / 0 FAIL / 0 SKIP** (unchanged).

---

## [v0.4.15] — 2026-05-11

The "embedded database lands" release. Fourth stdlib brick after
Path / Logging / Service, plus a cluster of governance + ergonomics
upgrades — NOTICE.md, CONTRIBUTING.md, an auto-close workflow for
external PRs, and the docs/guide refresh.

### Added

- **`Amalgame.Database.SQLite`** (PR #266) — embedded SQL via the
  vendored SQLite amalgamation
  (`runtime/Amalgame_Database/sqlite/sqlite3.c` + `sqlite3.h`,
  public-domain dedication). No `libsqlite3-dev` package needed
  on any OS; the amalgamation compiles directly into the user
  binary. Surface: `SQLite.Open(path)`, `Close`, `IsOpen`,
  `Exec(sql)`, `QueryAll(sql)` → `List<List<string>>`,
  `LastInsertId`, `Changes`, `LastError`. Namespace nests under
  `Amalgame.Database.<Engine>` so sibling backends (DuckDB,
  Postgres, MySQL) can land alongside without conflicts.
- **`.github/workflows/auto-close-external-prs.yml`** (PR #265)
  — auto-closes PRs opened from forks with a polite pointer to
  `CONTRIBUTING.md`. Detects external-vs-internal via the
  head-repo / base-repo mismatch, so `release/*` / `feat/*` /
  `docs/*` branches pushed by the maintainer skip the hook.
- **NOTICE.md** (PR #264, expanded here) — clarifies authorship
  (sole author = Bastien Mouget; the AI used during development
  is a tool, not a co-author at law), a "You are free to use
  Amalgame" plain-language Apache-2.0 grant summary, and a full
  third-party-licence audit (bdwgc / libcurl / SQLite / NSSM /
  host compilers) with compatibility notes for each.
- **CONTRIBUTING.md** (PR #264) — external contributions paused
  while the project is solo-developed; Issues stay open for bug
  reports; forks allowed per Apache-2.0.
- **docs/guide/04-stdlib.md** — Database.SQLite section with
  worked example + linking instructions + v1 limitations
  (no parameter binding yet, text-only columns, no explicit
  transactions — all tracked for v2).

### Changed

- **`tests/run_stdlib_tests.sh`** precompiles the SQLite
  amalgamation once at startup (~10s) and links the resulting
  `.o` into every stdlib test binary. Per-test overhead stays
  flat; the .o is dead-code-eliminated for non-Database tests.

### Roadmap

- `Amalgame.Database.<Engine>` siblings — DuckDB (vendored
  amalgamation), Postgres + MySQL (dynamic-link to system libs).
- `Amalgame.Database.SQLite` v2 — parameter binding via `?`
  placeholders, typed column accessors (`row.AsInt(0)`), prepared-
  statement reuse, transactions.
- `Amalgame.Service` v2 — native Windows SCM dispatcher, drop the
  NSSM dependency on Windows.

### Tests / infra

- Suite grows to **434 PASS / 0 FAIL / 0 SKIP** (was 421). 13 new
  cases cover open, DDL, DML, last-insert-id, changes counter,
  query+columns, aggregate, error path, delete, close. End-to-end
  on an in-memory database.
- Repo size: `+10MB` for the vendored SQLite amalgamation.
  `linguist-vendored=true` in `.gitattributes` keeps the lines
  out of GitHub's language stats.
- Snapshot refreshed.

---

## [v0.4.14] — 2026-05-10

The "service scaffolder lands" release. Path + Logging + Service
stdlib bricks (v0.4.11–v0.4.13) all came together to unblock
`amc new --template service` — one command generates a full daemon
project with cross-platform install paths.

### Added

- **`amc new --template service`** (PR #261) — fourth `amc new`
  template. Generates:
    - `src/main.am` with the canonical `Service.Install()` /
      `while !ShouldStop()` / `Log.Info` loop pattern.
    - **Linux side**: `<name>.service` systemd unit + `install.sh`
      that runs `systemctl daemon-reload`, `enable`, `restart`.
      Logs route to journald; `journalctl -fu <name>` to tail.
    - **Windows side**: `install.ps1` registers the binary as a
      Windows service via [NSSM](https://nssm.cc) (auto-downloaded
      if missing). NSSM transparently wraps any console binary as
      a proper Windows service: SCM dispatch, status reporting,
      auto-restart, log rotation. Operator interface is identical
      to a native service (`sc start` / `Get-Service` / Event
      Viewer all work).
    - `build.sh` (POSIX `gcc`) + `build.ps1` (MinGW `gcc`). Both
      auto-locate `runtime/` via `AMALGAME_HOME` env var, system
      install dirs, or the `amc` binary's neighbour directory.
    - `README.md` with the full run / install / operate matrix
      for every supported OS, plus a rationale section on why
      NSSM and how the v2 native-SCM path will eventually drop
      the wrapper.

  Native Windows Service mode (`StartServiceCtrlDispatcher` +
  `RegisterServiceCtrlHandler` + `SetServiceStatus`) is tracked
  as v2 — requires a worker-thread split and an amc-side wrap of
  generated `main()`. NSSM is the v1 pragmatic answer; existing
  NSSM installs will keep working through the v2 transition since
  NSSM-managed services run the same binary either way.

### Changed

- **Logging runtime helpers renamed `Logging_*` → `Log_*`** —
  matches the class facade name so `Log.Info(...)` in user code
  lowers to `Log_Info(...)` via the cgen's `isStdlib` short-
  circuit, same convention as `Console_*` / `File_*` / `Math_*`.
  The `src/stdlib/logging.am` facade is now optional sugar; user
  code reaches the same runtime helpers directly when amc auto-
  includes `Amalgame_Logging.h`. The service template generates
  code that uses this direct path — no facade import needed.
- **`Log` and `Service` added to the cgen's stdlib short-circuit
  list** plus `DeclareGlobal("Log", "type", ...)` /
  `DeclareGlobal("Service", "type", ...)` in the resolver, so
  the short-syntax usage (`Log.Info(...)` / `Service.Sleep(...)`)
  resolves cleanly without importing the facade source.

### Roadmap

- `Amalgame.Service` v2 — native SCM dispatcher (drop NSSM
  dependency on Windows).
- `amc new --template service` v2 — macOS launchd plist +
  `launchctl bootstrap` wrapper.
- `Amalgame.Database` — SQLite via libsqlite3 (next stdlib brick).

### Tests / infra

- Suite grows to **421 PASS / 0 FAIL / 0 SKIP** (was 407). 12
  new `amc new` cases cover the service template (file presence,
  generated `main.am` references the right APIs, systemd unit
  has the right `ExecStart`, `install.ps1` references NSSM).
- End-to-end roundtrip verified locally — `amc new`, `./build.sh`,
  run the binary, observe `INFO myd starting` / `INFO tick 0`,
  SIGTERM, observe `INFO myd shutting down cleanly`.
- Snapshot refreshed.

---

## [v0.4.13] — 2026-05-10

The "Service stdlib lands" release. Third stdlib brick after Path
(v0.4.11) and Logging (v0.4.12); last pre-requisite before the
`amc new --template service` scaffolder.

### Added

- **`Amalgame.Service`** (PR #256) — long-running process
  primitives wrapping POSIX `signal()` + `nanosleep()` (Linux/
  macOS) and `SetConsoleCtrlHandler` + `Sleep()` (Windows) behind
  the same Amalgame surface. Callers don't branch on platform.

  Surface: `Service.Install()`, `Service.ShouldStop()`,
  `Service.RequestStop()`, `Service.Sleep(ms)`. The typical loop:

      Service.Install()
      while (!Service.ShouldStop()) {
          Log.Info("heartbeat")
          Service.Sleep(5000)
      }

  Shutdown flag is `sig_atomic_t` on POSIX, `LONG` + `Interlocked-
  Exchange` on Windows — async-signal-safe and atomic respectively,
  no mutex needed for v1 single-process scope. POSIX `Service.
  Sleep` uses `nanosleep` (interruptible via EINTR); Windows
  slices into 100ms chunks (no portable equivalent for console
  signals).

### Roadmap

- `amc new <name> --template service` — now unblocked. Scaffolds
  a project using `Amalgame.Service` + `Amalgame.Logging` with
  a `<name>.service` systemd unit + `install.sh` for Linux v1.
- `--template forms` (cross-platform GUI) — SDL2 binding work
  tracked separately.

### Tests / infra

- Suite grows to **407 PASS / 0 FAIL / 0 SKIP** (was 403). 4 new
  cases drive the programmatic shutdown path (`RequestStop` flips
  the same flag the OS would). Direct SIGTERM delivery to the
  test runner would kill the suite, so the test exercises the
  post-flag state instead.
- Snapshot refreshed.

---

## [v0.4.12] — 2026-05-10

The "Logging stdlib lands" release. v0.4.11 added `Amalgame.Path`;
v0.4.12 adds the second stdlib brick on the way to `amc new
--template service`: leveled logging with a stderr + optional
file sink.

### Added

- **`Amalgame.Logging`** (PR #253) — `public class Log` with four
  levels (Debug / Info / Warn / Error), UTC ISO 8601 timestamps,
  fixed-width labels, optional file sink. Configuration is
  process-wide singleton state held in the runtime (`Logging_*`
  helpers), same pattern as `Exit_Set` / `Exit_Get`.

  Level names are case-insensitive on the first letter — "DEBUG",
  "Debug", "debug" all map. Unknown names default to "info"
  silently; logging itself should never crash a process. File
  sink reopens on each emit — slower than holding a stream open
  but robust against external log rotation, which is the typical
  production setup.

  Single-process, thread-unsafe v1 — fine for CLIs and single-
  threaded servers. Mutex is the v2 ask once a real multi-threaded
  user lands.

### Roadmap

- `Amalgame.Database` — minimum SQLite via libsqlite3.
- `amc new <name> --template service` and `--template forms` —
  cross-platform service / GUI scaffolders. Now unblocked on the
  Path + Logging side.

### Tests / infra

- Suite grows to **403 PASS / 0 FAIL / 0 SKIP** (was 393). 10 new
  cases cover level round-trips, case-insensitive parsing,
  unknown-level fallback, file sink round-trip, and emit + level
  filter via the file sink.
- Snapshot refreshed.

---

## [v0.4.11] — 2026-05-10

The "Path stdlib lands" release. v0.4.10 closed LSP slice 4;
v0.4.11 pivots to the stdlib backlog and ships the first of
several gaps: cross-platform path manipulation as a proper
`namespace Amalgame.Path` facade.

### Added

- **`Amalgame.Path`** (PR #250) — `public class Path` exposing
  Combine, Extension, Filename, Directory, Stem, IsAbsolute,
  Normalize, and Sep. Wraps the existing C runtime `Path_*`
  helpers and adds four new operations (`Stem`, `IsAbsolute`,
  `Normalize`, `Sep`) for the gaps the runtime didn't cover.
  Normalize follows Go's `filepath.Clean` semantics — pure
  string operation, doesn't touch the filesystem, so it works
  on non-existent paths. Output canonical separator is always
  `/` — Windows accepts forward slashes everywhere, and a
  deterministic canonical form is what callers expect.

### Roadmap

- `Amalgame.Logging` — structured logging with levels, JSON +
  text formatters, console + file sinks. Tracked alongside Path
  as a pre-requisite for `amc new --template service`.
- `Amalgame.Database` — minimum SQLite via libsqlite3.
- `amc new <name> --template service` and `--template forms` —
  cross-platform service / GUI scaffolders.

### Tests / infra

- Suite grows to **393 PASS / 0 FAIL / 0 SKIP** (was 372).
  21 new cases under `tests/run_stdlib_tests.sh` exercise every
  Path method + the documented edge cases.
- Snapshot refreshed.

---

## [v0.4.10] — 2026-05-10

The "LSP slice 4 closes" release. v0.4.9 finished slice 3 (rename
+ call hierarchy); v0.4.10 ships the next two features that ride on
the typechecker's per-node inference: inferred-type inlay hints and
the lightbulb / Quick Fix UI.

### Added

- **`textDocument/inlayHint`** (PR #246) — walks the open file's
  AST + runs the typechecker, emits one `InlayHint` per `VAR_DECL`
  that has an RHS but no annotation. Hint pinned right after the
  variable name, kind=1 (Type), paddingLeft=true so VS Code renders
  it as a greyed-out `name : type` decoration. Skips annotated
  decls, tuple destructuring, and decls whose inferred type is `?`.
- **`textDocument/codeAction`** (PR #247) — first quick fix lands:
  *Add type annotation*. When the cursor (or selection start) is on
  a `let x = …` whose annotation is empty and the typechecker can
  infer a concrete type, the server emits a `CodeAction` whose
  `WorkspaceEdit` inserts `: <type>` right after the variable name.
  Reuses the same inference path inlayHint already walks. The
  framework — dispatch + WorkspaceEdit emission — also unblocks
  future quick fixes (typechecker-driven annotation correction,
  unused-local removal, top-level-fn wrap-in-class) without
  another round-trip through the parser.

Capabilities advertised:
  `inlayHintProvider: true`
  `codeActionProvider: true`

### Roadmap

- Slice 5 LSP — tighter `selectionRange` (parser nameStart hook),
  more code actions (driven by linter / typechecker diags).
- `amc new <name> --template service` and `--template forms` —
  cross-platform service / GUI scaffolders.

### Tests / infra

- Suite stays at **372 PASS / 0 FAIL / 0 SKIP**. Snapshot refreshed
  twice across the cycle (one per LSP-touching PR).
- `tests/run_tests.sh` lsp-init expected string updated for
  `inlayHintProvider:true` and `codeActionProvider:true`.

---

## [v0.4.9] — 2026-05-10

The "LSP slice 3 closes" release. v0.4.8 made navigation fast and
broad; v0.4.9 fills in the IDE features that ride on top of the
already-walked AST: rename's pre-flight check (so F2's UI stops
silently failing on punctuation) and the Call Hierarchy panel
(Shift+Alt+H).

### Added

- **`textDocument/prepareRename`** (PR #243) — F2's pre-flight
  check. Returns `{ range, placeholder }` when the cursor is on a
  renameable token (IDENTIFIER / MEMBER / CLASS_DECL / ENUM_DECL /
  METHOD_DECL / VAR_DECL / PARAM with a non-empty Name), null
  otherwise. VS Code now shows the proper "you cannot rename this
  element" message on punctuation/whitespace instead of silently
  noop-ing.
- **`textDocument/prepareCallHierarchy`** + **`callHierarchy/incomingCalls`**
  + **`callHierarchy/outgoingCalls`** (PR #243) — exposes the
  caller/callee graph for the method enclosing the cursor. Shift+Alt+H
  in VS Code lights up; the outgoing pass walks the method body
  collecting CALL nodes grouped by callee Name; the incoming pass
  walks every program for CALL nodes whose callee matches `item.name`
  and groups call sites by their enclosing METHOD_DECL. Items
  roundtrip name/uri/line/character via the spec's `data` field so
  follow-up requests re-locate the method without another search.

### Roadmap

- Slice 4 LSP work: tighter `selectionRange` (parser nameStart
  hook), inlay hints (parameter names, inferred types), code
  actions (organize imports, "extract to method").
- `amc new <name> --template service` and `--template forms` —
  cross-platform service / GUI scaffolders.

### Tests / infra

- Suite stays at **372 PASS / 0 FAIL / 0 SKIP**. Snapshot refreshed
  for the new cgen output.
- `tests/run_tests.sh` lsp-init expected string updated for
  `renameProvider:{prepareProvider:true}` and `callHierarchyProvider:true`.

---

## [v0.4.8] — 2026-05-10

The "LSP gets fast + comprehensive" release. v0.4.7 made Cmd+Click
work; this release makes it instant, expands navigation to the
features users hit constantly (Find All References, Outline,
project-wide symbol search, parameter / local jumps), and patches
the VS Code extension wart that broke the natural `~/`-style
`serverPath` value.

### Added

- **`amc lsp` workspace resolver caching** (PR #232) — every
  hover / completion / definition / references request used to
  re-parse every `.am` in the workspace from scratch. On a 30-
  file repo that's 2.7s per Cmd+Click; VS Code shows the spinner.
  Now sibling files are parsed once on first use and reused
  across requests; the open file's freshly-parsed AST overrides
  the cached entry per request so editor-fresh text always wins.
  Probe before / after on three sequential definition calls:

      def #1: 0.061s    def #2: 0.037s    def #3: 0.040s

  ~45–70× speedup. Cache is keyed on the resolved workspace
  root; navigating to a different project rebuilds it once.
- **`textDocument/documentSymbol`** (PR #235) — Outline panel,
  breadcrumbs, and Ctrl+Shift+O fuzzy-find now light up for
  every `.am` file. Top-level: classes (kind 5), enums (kind 10).
  Class children: methods (kind 6), fields (kind 8). Enum
  variants surface as kind 22 (EnumMember).
- **`textDocument/typeDefinition`** + **`workspace/symbol`**
  (PR #236) — Ctrl+T project-wide symbol search across every
  cached `.am` file (case-insensitive substring filter), and
  Cmd+K F12 "Go to Type Definition" advertised. Type definition
  is identical to definition for now; diverges once the
  typechecker exposes per-node inferred types.
- **`textDocument/references`** (PR #237) — Find All References
  / Shift+F12 walks every program, recursively descends through
  every AstNode child slot (`Left/Right/Cond/Body/Else/Children/
  Params/Args`), collects every IDENTIFIER / MEMBER whose Name
  matches the symbol under the cursor. Probe shape on `NodeKind`
  in `parser/ast.am`: 433 hits across 9 files in 220ms.
- **`textDocument/definition` resolves params + local var-decls**
  (PR #238) — first pass searches the enclosing method's Params
  + recursively-walked Body for VAR_DECLs matching the lookup
  name, then falls through to the workspace top-level walk. Bug
  report: clicking on `model` at an assignment site in
  `migrate.am` returned null because `model` is a local. Now
  jumps to the `var model: string = ""` declaration.

### Fixed

- **VS Code extension: `amalgame.serverPath` trim + tilde-expand**
  (PR #234) — `child_process.spawn` doesn't run through a shell,
  so neither incidental whitespace nor `~/` were processed. A
  setting like `' ~/.local/bin/amc'` (leading space, tilde) failed
  with ENOENT and the LSP never started. New `resolveServerPath`
  helper trims, replaces a leading `~` with `os.homedir()`, falls
  through to `'amc'` if the resulting string is empty. Bumped
  extension version to 0.2.1.

### Roadmap

- Slice 3 LSP work staying on the roadmap: tighter
  `selectionRange` (parser nameStart hook), member-def in
  definition (jump to a method body when clicking on
  `obj.method()`), rename, call hierarchy, inlay hints, code
  actions. Each is sized to land as a small follow-up PR.
- `amc new <name> --template service` and `--template forms` —
  cross-platform service / GUI scaffolders. Roadmap entries
  expanded in PR #232 with the SDL2 / systemd / launchd / SCM
  outline.

### Tests / infra

- Suite stays at **372 PASS / 0 FAIL / 0 SKIP**. Snapshot
  refreshed multiple times across the cycle (each LSP-touching
  PR + the cgen patches needed a re-bootstrap so cold-start
  clones could parse the new sources).
- Run scripts updated to track every new LSP capability advertised
  in the initialize reply (`documentSymbolProvider`,
  `workspaceSymbolProvider`, `referencesProvider`,
  `typeDefinitionProvider`).

---

## [v0.4.7] — 2026-05-10

The "LSP gets useful" release. The IDE experience moves from
"diagnostics + hover + completion, with ~150 spurious typechecker
errors per workspace" to "diagnostics-clean + Cmd+Click navigates
to definition". Plus a one-line resolver fix that turned out to
be the root cause of the long-parked typechecker false-positives.

### Added

- **`textDocument/definition` + `textDocument/declaration`**
  (PR #227) — Cmd+Click jumps to the declaration site of any
  top-level symbol (class, enum, interface) reachable through
  the workspace resolver. Slice 1 of `amc lsp` v2 navigation
  per the roadmap. Slice 2 (member methods, fields, references)
  lands as a follow-up.
- **`Amalgame.Crypto` cleanup-only follow-up tooling**
  — `xs.Set(idx, value)` wired into the cgen as a side effect
  of investigating the resolver bug below. The runtime
  `AmalgameList_set` was already there; the cgen surface is
  new (PR #228).

### Fixed

- **Resolver: `MemberTable.Set` was `Values.Add`-ing on duplicate
  key** (PR #228) — single-line semantic bug that desynced the
  parallel `Keys` / `Values` arrays. Every subsequent lookup
  returned a value belonging to *another* (member, class) pair.
  The build path never tripped on it (each MemberTable instance
  saw each member once); workspace LSP did, because the open
  file is parsed once from didOpen text and *also* picked up by
  the workspace `find` scan, re-collecting its classes. LSP probe
  before / after across the 9 main compiler files: ~150 spurious
  typechecker diagnostics → **0**. Resolves the "37-case spurious
  typechecker on lexer.am via the LSP" item parked in
  ROADMAP_COMPLET since v0.4.4.
- **LSP: `UriToPath` percent-decodes `%XX` escapes** (PR #227) —
  VS Code sends URIs with percent-encoded UTF-8 for any non-ASCII
  byte in the path (`Développement` → `D%C3%A9veloppement`). The
  previous decoder kept the literal bytes, the workspace scan
  walked an inexistent path, the resolver fell back to
  single-file mode, and any cross-file type reference became an
  "Unknown symbol" diagnostic. Self-contained ~50 LoC pure-
  Amalgame decoder + hex helper. On `c_gen.am` alone: ~130
  spurious "Unknown symbol 'NodeKind'" → 0.
- **LSP: definition replies now percent-encode the response URI**
  (PR #229) — the inverse of the request-side fix. VS Code's
  DocumentURI parser silently rejects replies whose `uri` carries
  raw UTF-8 bytes; symptom was Cmd+Click "spinning forever" even
  though the LSP did respond. New `PercentEncodePath` walks the
  reply path byte-by-byte, leaves the URI-unreserved set
  (`A-Za-z0-9-._~/`) literal, percent-encodes everything else.
- **Cleanup: chain-mash typed-local workarounds removed**
  (PR #225) — now that PR #221 (constructor forward-decl) and
  PR #222 (`obj.Field.Method()` / `obj.Method().Method()`
  chain-mash) are in, the workarounds in `lsp.am`, `migrate.am`,
  and `stdlib/datetime.am::InstantResult` are mechanically
  unnecessary. Same PR uncovered + fixed two more cgen type-
  inference gaps (`Get`-handler chain receiver,
  `MethodRetGet` chain receiver) that the workarounds had been
  masking.

### Tooling

- **`tools/release.sh` patched in four spots** (PR #220) after
  walking through it manually for v0.4.5: README sed catches
  both bump patterns, back-merge from main runs before the
  develop → main PR, `--admin --merge` fallback for repos where
  auto-merge isn't available, CHANGELOG-edit prompt prefers
  `$EDITOR` and falls through cleanly when there's no tty.
- **`release-pdf.yml` lmodern fix + `workflow_dispatch`**
  (PR #219) — Ubuntu 24.04's `texlive-fonts-recommended` doesn't
  pull `lmodern`, which pandoc's default LaTeX template needs.
  v0.4.5 missed its PDF; v0.4.6+ have the asset attached on the
  first run. Manual trigger added so older tags can be reprocessed.

### Roadmap

- **`amc lsp` v2 navigation** (PR #226) — full inventory of the
  next-tier LSP features (declaration, type definition,
  references, hover preview, find-all-references, document
  symbols, workspace symbol, call hierarchy, rename, inlay
  hints, code actions). Slice 1 (definition) lands here; the
  rest is sized to drop in as small PRs.
- **`amc lsp` performance — workspace resolver caching**
  (PR #229) — every request currently re-parses the entire
  workspace (~2.5–3s on a 30-file repo). Caching with
  invalidation on didChange is the next perf win.
- **VS Code extension robustness on `serverPath`** (PR #229) —
  ~5 LoC fix in `extension.js`: trim whitespace + expand
  leading `~/` before `child_process.spawn` so the natural
  shell-style values just work.

### Tests / infra

- Suite stays at **372 PASS / 0 FAIL / 0 SKIP**. Snapshot refreshed
  several times across the cycle (each cgen-touching PR + the
  resolver bug fix needed a re-bootstrap so cold-start clones
  could parse the new sources).

---

## [v0.4.6] — 2026-05-10

The "compiler quirk-cleanup" release. Three patches to long-known
parser/cgen sharp edges, plus an end-to-end fix to `tools/release.sh`
informed by walking through it manually for v0.4.5. No new user-
facing features; this is purely making the compiler emit cleaner C
and the release flow less twitchy.

### Fixed

- **Parser: `expr >> N` and `expr << N` no longer drop the shift**
  (PR #215). `ParseRelational` was correctly calling `ParseShift` on
  its RHS but went straight to `ParseAdd` on the LHS, so any
  expression starting with a shift got truncated. One-line fix
  restores the intended `equality > relational > shift > add > mul`
  precedence ladder. Cleaned up the `r / 256` workaround in
  `random.am`. `encoding.am` keeps `/` / `*` because the cgen
  doesn't yet preserve precedence parens around mixed `+` / shift
  expressions — separate roadmap item.
- **Parser: top-level `fn` now errors clearly** (PR #221). Before:
  `public fn foo()` outside a class parsed silently as Unknown,
  the body was swallowed, the call site compiled fine, the linker
  failed (or worse, called the C `double` keyword by accident).
  Now: `amc` exits 1 with `Top-level functions aren't supported
  (got 'fn foo' at L:C). Wrap it inside a class as 'public static'.`
  Drive-by: the regular compile path now actually checks
  `par.HasErrors()` after each parse — only `amc fmt` was doing
  it before.
- **CGen: constructor `_new(...)` forward-decls emitted before any
  class body** (PR #221). Before: a constructor whose body called
  `new B(...)` for a class declared later (or even earlier in the
  same file but emitted after) hit `gcc: implicit declaration` +
  `conflicting types` for `App_B_new`. Fix walks every class decl
  at pass-2 entry and emits its `_new` signature before any body.
  +316 forward declarations across the bundled compiler.
- **CGen: chain-mash on `obj.Field.Method()` and
  `obj.Method().Method()`** (PR #222). Two related bugs in
  `EmitCalleeStr` + the call-site self-injection block both caused
  the cgen to mash a member or call expression into the method
  name instead of resolving it through the typed receiver:

      Before                   →  After
      o.Field.Get()            →  Inner_Get(o->Field)
                                 (was: o->Field_Get())
      o.GetInner().Doubled()   →  Inner_Doubled(Outer_GetInner(o))
                                 (was: Outer_GetInner(o)_Doubled())

  The typed-local workarounds (`let mid: T = obj.Field; mid.Method()`)
  in `lsp.am`, `migrate.am`, `stdlib/json.am`, `stdlib/datetime.am`
  are now mechanically unnecessary; cleanup deferred to a separate
  PR to keep the diff focused.

### Tooling

- **`tools/release.sh` patched in four spots** (PR #220) after
  walking through it manually for v0.4.5:
    1. `sed` for README now also bumps the `Current version: **vX.Y.Z**.`
       overview line, not just the `# amc X.Y.Z` comment.
    2. New "Back-merge main into develop" step before the
       develop → main PR — prevents the conflict cascade when main
       carries a merge commit develop never absorbed (the standard
       outcome of any previous develop → main PR with conflict
       resolution).
    3. `--admin --merge` fallback retried on each poll tick if
       `--auto --merge` rejects (this repo's main branch doesn't
       allow auto-merge with merge-commit strategy).
    4. CHANGELOG-edit prompt uses `$EDITOR` if set, falls back to
       the read-from-tty pattern, finally warns + continues if
       neither path works (was hanging non-interactive runs).

### Tests / infra

- Suite stays at **372 PASS / 0 FAIL / 0 SKIP** — cleanup release,
  no new test surface added. Snapshot refreshed three times during
  the cycle (after each cgen-touching PR) so cold-start clones
  bootstrap from a parser+cgen consistent with the new sources.

---

## [v0.4.5] — 2026-05-10

The "infrastructure cleanup + Crypto" release. Drops the Vala
bootstrap entirely now that the snapshot is the sole bootstrap
entry point, adds `Amalgame.Crypto` (SHA-256 + HMAC), fixes a
silent shift-drop bug in the parser, and wires a release-time
PDF guide build. 14 commits across 6 PRs.

### Added

- **`Amalgame.Crypto`** (PR #213) — SHA-256 (FIPS 180-4) and
  HMAC-SHA-256 (RFC 2104) primitives in two static facades:
  `Sha256` for the bare hash, `Hmac` for keyed authentication.
  Pure-C runtime header `runtime/Amalgame_Crypto.h` (~150 lines,
  no external crypto dep). API:

      Sha256.Bytes(data: List<int>)    -> List<int>   # raw 32 bytes
      Sha256.Hex(data: List<int>)      -> string      # hex
      Sha256.OfString(s: string)       -> string      # UTF-8 → hex

      Hmac.Sha256(key, msg: List<int>)        -> List<int>
      Hmac.Sha256Hex(key, msg: List<int>)     -> string
      Hmac.Sha256OfStrings(key, msg: string)  -> string

  Tested against FIPS 180-4 SHA-256 vectors ('abc', empty,
  100×'a' multi-block) and RFC 4231 HMAC-SHA-256 cases 1–2.
  Documented caveat: constant-time MAC comparison is the
  caller's responsibility.

- **`release-pdf` GitHub Actions workflow** (PR #211) — on every
  `v*` tag, concatenates `docs/guide/0*.md` (chapters 01..08) and
  builds `amalgame-guide-vX.Y.Z.pdf` via pandoc + xelatex,
  attached to the GitHub Release. The amalgame.me /releases page
  picks it up automatically.

### Changed

- **README + guide code blocks** (PR #216) — 61 ` ```amalgame `
  fences swapped to ` ```kotlin ` as a syntax-highlight fallback.
  GitHub Linguist doesn't recognise `amalgame` yet (roadmap entry
  added), and Kotlin shares enough keywords (`let` / `var` /
  `class` / `null` / `for x in xs`) to highlight ~80% of the
  syntax usefully.

### Fixed

- **Parser: `expr >> N` and `expr << N` no longer drop the shift**
  (PR #215). `ParseRelational` was correctly calling `ParseShift`
  on its RHS but went straight to `ParseAdd` on the LHS, so any
  expression starting with a shift got truncated. One-line fix
  restores the intended `equality > relational > shift > add >
  mul` precedence ladder. Cleaned up the `r / 256` → `r >> 8`
  workaround in `random.am`. `encoding.am` keeps `/` / `*` for
  now since its slices appear inside mixed `+` expressions where
  the cgen-precedence-parens issue would change C evaluation
  order — separate roadmap item.

### Removed

- **Vala bootstrap** (PR #210) — `archive/vala-bootstrap/`
  (Vala compiler sources, meson.build, ~13 500 LoC), `compile.sh`
  (Vala build wrapper), `install/release.sh` (meson-based release
  builder, superseded by `release.yml`), top-level `install.sh`
  (stale stub referencing `codec --version` and the pre-rename
  repo URL), `bootstrap/` directory (unused 2 MB Vala binary +
  README describing commands that don't exist), and 6
  `docs/transpiler/*.md` v0.1.0 design docs (described the
  deleted impl). The bootstrap chain is now 2-rung — `./amc →
  ./snapshot/amc` — and a clean clone cold-starts in one `gcc`
  invocation. The pre-removal state is preserved at the
  `vala-bootstrap-final` git tag if revival is ever needed.
  Net diff: 30 files removed, ~13 569 lines deleted.

### Roadmap

- **Stdlib gaps — second tier** (PR #214) — new ROADMAP_COMPLET
  section inventorying modules a complete stdlib usually has
  and Amalgame doesn't yet: Audio (miniaudio binding sketch),
  Database (SQLite), Path manipulation, Logging, WebSocket,
  Filesystem watcher, advanced Math (Vec/Mat/BigInt), other
  serialization formats (TOML/YAML/MessagePack).
- **Submit Amalgame to GitHub Linguist** (PR #216) — long-term
  fix for the markdown highlight fallback; we already have the
  TextMate grammar in `editors/vscode/syntaxes/`.

### Tests / infra

- Suite is now **372 PASS / 0 FAIL / 0 SKIP** (+9 from
  `Amalgame.Crypto`).
- `snapshot/amc_lib.c` refreshed twice during the cycle: once
  after Crypto wiring (new resolver globals), once after the
  parser fix (so cold-start clones don't hit the old shift-drop
  on the new `random.am`).

---

## [v0.4.4] — 2026-05-10

The "stdlib trio" release. Three new modules — `Amalgame.Random`,
`Amalgame.Encoding`, `Amalgame.DateTime` — close the most-cited
gaps in everyday Amalgame code. Plus a release script that
automates the version bump → tag flow so the v0.4.0-style
"forgot to bump src/main.am" mistake can't happen again.

### Added

- **`Amalgame.Random`** (PR #200) — instance-based PCG XSH-RR
  64/32 PRNG. `new Random(seed)` for reproducibility,
  `Random.FromSystem()` for crypto-grade entropy via the OS pool
  (`getentropy` on POSIX, `BCryptGenRandom` on Windows).
  Methods: `NextUInt32`, `NextInt`, `IntRange(min, max)`,
  `Float`, `Bool`, `Bytes(n)`. Static `Random.SystemBytes(n)`
  for one-off salt / nonce generation. Replaces the legacy
  `Math.Random` 8-bit-output LCG for new code; the old API
  stays for compatibility.
- **`Amalgame.Encoding`** (PR #201) — three small static
  facades: `Base64` (RFC 4648 + URL-safe variant), `Hex`
  (lower / upper, case-insensitive decode), `Url`
  (percent-encoding per RFC 3986, plus an `EncodeComponent`
  matching JS's `encodeURIComponent`). 100% pure Amalgame, no
  runtime header. Bytes flow as `List<int>` ∈ [0, 255], so
  encoding `Random.Bytes(n)` output works without a cast layer.
- **`Amalgame.DateTime`** (PR #202) — `Instant` (i64 ns since
  1970-01-01 UTC, range ±292 years), `Duration` (signed ns
  interval), `Stopwatch` (monotonic-clock convenience for
  benchmarks). RFC 3339 strict parse + format (UTC only —
  `+HH:MM` offsets rejected; local time / timezones tracked
  as a v2 follow-up).
- **`tools/release.sh`** (PR #203) — one-shot release script
  that bumps the version everywhere, builds + tests + saves a
  snapshot, opens the release → develop and develop → main PRs
  with `gh pr merge --auto --merge`, polls until each lands,
  then tags from main and pushes. `--dry-run` and `--no-tag`
  flags for inspection. Closes the manual-checklist gap that
  caused v0.4.0's missed bump.
- CI Windows build links `-lbcrypt` for `Random.FromSystem()`'s
  `BCryptGenRandom` call. Same flag added to the Windows release
  artifact build.

### Changed

- `runtime/` gains `Amalgame_Random.h` + `Amalgame_DateTime.h`
  next to the existing headers. The PCG step (`Random_PcgOutput`
  / `Random_PcgAdvance`) keeps the multiply unsigned because
  signed overflow is UB on Amalgame's `int` = i64.
- `docs/guide/04-stdlib.md` gains Random, Encoding, and DateTime
  sections — each with the full method table and the sharp-edge
  caveats (modulo bias for IntRange, RFC-strict `+` handling for
  Url.Decode, ±292-year window for Instant).

### Roadmap (parser/cgen findings, not fixed in this release)

Surfaced while writing the trio; workarounds applied inline,
real fixes tracked in `ROADMAP_COMPLET.md`:

- Parser drops `expr >> N` inside a `let` initializer (lowers
  as `i64 x = expr;`). Workaround: division by 2^N.
- Top-level `public fn(...)` outside a class parses but cgen
  never emits a definition. Workaround: hang the helper on a
  class as `public static`.
- Constructors that instantiate a later-declared class trigger
  an implicit-decl conflict because `_new` forward-decls aren't
  emitted before pass2 bodies. Workaround: pass the dependency
  in as a constructor parameter (used by `InstantResult`).
- 25 stale `BastienMOUGET/Amalgame` URLs scattered across
  runtime/ headers, install/, and archive/ — to be swept after
  this release. Tracked under "URL sweep" in Distribution.

### Tests / infra

- Suite is now **363 PASS / 0 FAIL / 0 SKIP** (was 307 at
  v0.4.3). +56 assertions split as +14 Random / +21 Encoding
  / +21 DateTime.
- Snapshot refreshed; `snapshot/amc_lib.c` is ~16 500 lines
  reflecting the three new modules + the resolver builtins
  for the new runtime helpers.

---

## [v0.4.3] — 2026-05-09

The "Json migration completes" release. Phase 2 final + phase 3
of the Amalgame.Json plan (the compiler + LLM commands now go
through the proper parser, not substring tricks), plus
`amc migrate v3` real cost reporting on top.

### Changed

- **`src/lsp.am`** request dispatcher: switched from the ad-hoc
  `JsonStr` / `JsonInt` substring extractors to a single
  `Json.Parse(body)` walked via structured `Get(...).At(...)`
  chains. Each LSP message is now parsed once with a real JSON
  parser; the helpers are gone.
- **`src/migrate.am`** response parsing: each provider now reads
  the actual JSON shape of its response instead of the first
  `"text":"..."` substring trick:
    - Anthropic → `root.content[0].text`
    - OpenAI → `root.choices[0].message.content`
    - Gemini → `root.candidates[0].content.parts[0].text`
  Each path returns an attributed error if the shape doesn't
  match, instead of a silent empty string.
- **`amc migrate` real cost line** (v3): after a successful
  migration, the real per-call cost is printed alongside the
  pre-flight `--dry-run` estimate. No `~` prefix, no heuristic —
  exact tokens billed, pulled from each provider's `usage`
  object:

      [migrate] wrote src/api.am
      [migrate] cost: 6431 in + 982 out = $0.04 (claude-sonnet-4-6)

  CLI shell-out (`claude`, `custom`) still reports nothing since
  the bill is on the user's subscription / local backend.

### Removed

- `lsp.am::EscapeJsonStr` / `JsonStr` / `JsonInt`
- `migrate.am::JsonEscape` / `JsonExtract` / `JsonExtractText`

Six functions, ~150 LoC out. The compiler ships with one JSON
parser (`Amalgame.Json`) instead of two-and-a-half overlapping
substring matchers.

### Fixed

- The earlier-noted "Json.Parse hangs on 16 KB bodies" turned
  out to be a benchmark artefact — bash `${#body}` counts
  characters, not bytes, so the LSP probe shipped a frame that
  under-counted UTF-8 multibyte content by the byte/char delta.
  Real client traffic uses byte-accurate counts and the parser
  handles a typical 16 KB didOpen body in ~37 ms.

### Documentation

- `docs/guide/08-llm-commands.md` — "Cost estimation" renamed to
  "Cost reporting", documents both the pre-flight `--dry-run`
  estimate and the post-flight real cost.
- `docs/proposals/amc-migrate.md` — v3 cost reporting moved from
  "deferred" to "partial". API streaming via SSE remains the
  only deferred v3 item.
- `ROADMAP_COMPLET.md` — `Promote ad-hoc JSON to a real
  Amalgame.Json module` marked resolved.

---

## [v0.4.2] — 2026-05-09

The "stdlib + DX" release. Adds a real JSON module, a project
scaffolder, and IDE-friendly member completion. Suite goes from
**263/263** to **307/307**.

### Added

- **`Amalgame.Json`** — first-class JSON parser, encoder, and
  accessor surface. Strict RFC 8259 with full escape support
  (incl. `\uXXXX` + surrogate pairs). Class-with-tag `JsonValue`
  with `Is*` / `As*` / `Get` / `At` / `Length` / `Keys`. Static
  `Json.Parse` / `Encode` / `EscapeString` / `Of*` factories.
  Lives in the new `src/stdlib/` directory. Replaces 4 ad-hoc
  substring extractors in the compiler in a future PR (phase 2);
  this release ships the library standalone. Direct prep for
  `amc migrate v3` cost reporting via nested `usage.input_tokens`.
  [PR #182, #183]

- **`amc new <name>`** — project scaffolder à la `cargo new`.
  Three templates: `exe` (default — `src/main.am` + a passing
  test), `lib` (public class skeleton), `test` (test bundle
  only). Each generates a working `build.sh` that locates the
  Amalgame runtime via `$AMALGAME_HOME` / install dirs / `which
  amc`, plus README, `.gitignore`. Path-aware: `amc new
  path/to/foo` works (basename is the namespace stem). Refusal
  on existing dirs unless `--force`. [PR #184]

- **LSP member completion** — `obj.<cursor>` now narrows to the
  methods/fields of the receiver type. Two-step receiver
  resolution: global-symbol lookup (covers `Json.<cursor>`,
  user-class names) then a local-decl text scan covering
  `let x = new T(...)` / `let x: T = ...`. Falls back to the
  global list for `this.<cursor>`, chained calls, builtin C
  modules. [PR #185]

- **Editor integration on install** — roadmap item added (auto-
  wire VS Code `.vsix` / Neovim lspconfig snippet / Helix
  `languages.toml` entry on first install). [PR #181]

### Documentation

- `docs/proposals/amalgame-json.md` — full design doc for the
  JSON module: motivation, type design, API surface, 3-phase
  migration plan, testing strategy, decisions captured in review.
  [PR #182]

- `ROADMAP_COMPLET.md` — LSP enum-lookup item marked resolved
  (verified empirically: 0 "Unknown symbol" diagnostics on
  `lexer.am` via `amc lsp`). New typechecker item parked with
  investigation findings on the spurious "Return type mismatch"
  cases. [PR #181, #186]

### Infrastructure

- Repo transferred from `BastienMOUGET/Amalgame` to
  `amalgame-lang/Amalgame`. All code, docs, install scripts, and
  publication recipes updated to the new URL. GitHub redirects
  the old URLs for ~1 year; this lands canonical references
  ahead of that window. [PR #187]

- `tests/run_stdlib_tests.sh` extended with a 5th-arg
  `extra_inputs` so per-module tests can pull in a stdlib source
  file alongside the sample (used by the Json suite). [PR #183]

- `tests/run_amc_new_tests.sh` added and wired into
  `run_all_tests.sh`. 20 assertions: file/dir presence per
  template, end-to-end compile + run for `exe`, error paths.
  [PR #184]

### Known issues (unchanged from v0.4.1)

- Typechecker reports spurious "Return type mismatch" on enum-
  member returns in some `lexer.am` IF-body shapes (37 cases via
  the LSP). Doesn't reproduce on minimal cross-file repros.
  Investigation findings captured in `ROADMAP_COMPLET.md`; next
  step documented (instrument `CheckReturn`).

---

## [v0.4.1] — 2026-05-09

Patch release. The v0.4.0 binary still printed `amc 0.3.6` from
`amc --version` because `src/main.am`'s hardcoded version string
wasn't bumped before the tag. This release fixes the string and
adds a CONTINUATION.md pitfall (#12) so the next contributor
catches it pre-tag.

No other changes — same code as v0.4.0 modulo the version literal
and the snapshot refresh. CONTINUATION.md adds the pitfall.

---

## [v0.4.0] — 2026-05-09

The "LLM-driven workflows" release. Three new subcommands shipped in
one big push: **`amc migrate`** (translate other languages → Amalgame),
**`amc generate`** (write a new `.am` from a natural-language prompt),
**`amc explain`** (read a `.am`, write prose). Plus a batch of
compiler / parser / CGen fixes that the LLM-generated code surfaced
along the way. Suite goes from **187/187** to **263/263**.

### New subcommands

- **`amc migrate <file|dir>`** *(PRs #149, #158, #161, #162, #156, #166, #167, #169, #170)*
  - Auto-detects 21 source languages by extension (TS, JS, Python,
    Java, C#, Go, Rust, C, C++, Kotlin, Swift, Ruby, PHP, …).
  - Provider abstraction with five built-ins:
      - `claude` (Claude Code CLI shell out — default fallback)
      - `claude-api` (Anthropic HTTP via `Http_PostWithHeaders`)
      - `chatgpt` (OpenAI HTTP)
      - `gemini` (Google AI Studio HTTP)
      - `custom` (delegates to `AMC_CUSTOM_PROVIDER_CMD`, reads stdin
        / writes stdout — wraps any local LLM CLI)
  - Provider auto-selection from env: `ANTHROPIC_API_KEY` →
    `claude-api`, `OPENAI_API_KEY` → `chatgpt`, `GEMINI_API_KEY` →
    `gemini`, fallback `claude` (CLI).
  - Directory recursion. Result cache by SHA-256(source + system
    prompt) at `~/.cache/amalgame/migrate/`. Per-file `amc --check`
    validation (suppressible). Source-size cap (default 2000 lines).
  - Cost estimation in `--dry-run` per provider + model.
- **`amc generate "<prompt>"`** *(PR #164)* — same provider stack,
  output to stdout by default. `--stream` flows the response live
  via the claude CLI.
- **`amc explain <file.am>`** *(PR #165)* — reverse direction.
  `--lang <name>` controls the explanation language.

### Compiler — language

- **Lambda v2.5 — non-int signatures** *(PR #142)*. Higher-order
  list operations like `xs.Map(x => x.Name)` over a `List<Class>`
  now work end-to-end. The TypeChecker patches the lambda's `PARAM`
  type from the higher-order callsite (so `x` resolves as `Class`
  inside the body), and the CGen splits `EmitOneLambda` into
  forward + body emission so `lam_N_fn` can dereference fields on
  user struct types.
- **Multi-line method chains** *(PR #154)*. `xs\n    .Filter(...)\n    .Map(...)`
  parses cleanly. `ParsePostfix` peeks past `NEWLINE` when the next
  non-NL token is `.` or `?.`. Statement boundaries (NEWLINE not
  followed by a dot) stay intact.
- **TS-style param syntax** *(PR #152)*. `Foo(id: int)` is accepted
  as an alias for `Foo(int id)`. Same fix also resolves a
  pre-existing infinite-loop on the malformed input. The parser
  param-list while-loops gain a "did the cursor advance?" safety
  belt.
- **CGen auto-qualify implicit field access** *(PR #155)*. Within a
  class method, `Id = id` lowers to `self->Id = id` when `Id` isn't
  a local but is a field of the current class. Lets users from
  C# / TS / Kotlin / Swift skip the explicit `this.` qualifier.

### Stdlib

- **`Env.Get(name)` / `Env.Has(name)`** *(PR #160)*. Wraps
  `Environment_GetVar` / `_HasVar`. Required by the API providers
  to read API keys from the environment.
- **`Http_PostWithHeaders` / `Put` / `Delete` / `Patch` return
  type tracked** *(PR #160)*. `resp.Status` / `.Body` / `.Ok`
  type-check correctly after these calls (previously only
  `Http_Get` / `Post` / `GetWithHeaders` / `GetTimeout` /
  `PostJson` were registered).

### Tooling

- **LSP — workspace-aware diagnostics** *(PR #146)*. `amc lsp`
  now scans every `.am` file under the detected workspace root
  (markers: `.git`, `build_amc.sh`, `package.json`) and feeds them
  to the resolver. Eliminates ~44 false `Unknown symbol` diagnostics
  per file on `src/typechecker.am` (NodeKind, SourceSnippet, …).
  Hover / completion benefit too.
- **`amc migrate --help` / `-h`** *(PR #156)* — the discoverability
  flag actually prints help instead of erroring.

### Distribution

- **Release artifacts now bundle `docs/language/grammar.ebnf` +
  `docs/guide/02-language-tour.md`** under `share/amalgame/docs/...`
  so `amc migrate / generate / explain` find them at
  `<exec-dir>/../share/amalgame/docs/...` post-install. Without
  these, the LLM commands degrade gracefully to an inline
  conventions block.
- `install.sh` copies them to `$PREFIX/share/amalgame/docs/`.
- `build_amc.sh` now links `gen_test` against `-lcurl` (the bundled
  `migrate.am` pulls in curl symbols transitively).

### Documentation

- New chapter **`docs/guide/08-llm-commands.md`** — user-facing
  reference for migrate / generate / explain.
- New design doc **`docs/proposals/amc-migrate.md`** — full v0/v1/v2
  rationale, kept up-to-date with what shipped.
- CONTINUATION.md and ROADMAP_COMPLET.md refreshed.

### Refactor & contributor DX

- **`merge=ours` + `linguist-generated` for generated artefacts**
  *(PR #173)*. `.gitattributes` declares `merge=ours` for
  `snapshot/amc_lib.c`, `snapshot/INFO.md`, and `src/amc_lib.c`
  — the canonical resolution on conflict is "rebuild from
  sources", not three-way merge. Eliminates the manual `--theirs`
  → rebuild → re-snapshot → amend dance that ate ~6 PRs in the
  v0.4 cycle. One-time post-clone setup:
  `git config merge.ours.driver true`. Documented as pitfall #12
  in CONTINUATION.md.
- **CGen: `BoxAsVoid` / `UnboxScalar` helpers extracted** *(PR #174)*.
  The `(void*)(intptr_t)X` boxing dance and symmetric unbox were
  duplicated across ~17 sites; consolidated into two helpers so
  intent reads at a glance. Behavior unchanged — emitted C is
  byte-identical pre / post.
- **Parser-loop audit** *(PR #175)*. Verified that every `while`
  loop calling a sub-parser already has a `lastPos` watchdog
  (added preventively in PR #152). No new hazard found.

### Cleanup

- `use.sh` (pre-self-hosted doc snippet) removed.
- `.github/workflow/` typo directory removed (was duplicate of
  `.github/workflows/`, ignored by GitHub anyway).

---

## [v0.3.6] — 2026-05-08

The "lambdas in the wild" release. Higher-order `List<T>`
methods now take a lambda directly: `xs.Filter / .Map / .Reduce
/ .ForEach / .Any / .All / .CountIf`. Suite goes from 180/180
to **187/187** under `./amc`.

### Stdlib

- **Higher-order List methods** — Filter, Map, Reduce (with a
  `(acc, x) → acc` reducer dispatched through `Closure_call2`),
  ForEach, Any, All, CountIf. Each takes an `AmalgameClosure*`
  so the captured environment travels with the lambda.
  Implemented as static-inlines in `runtime/_runtime.h` and
  `runtime/Amalgame_Collections.h`. The closure struct +
  `_callN` wrappers are hoisted to the top of `_runtime.h` so
  the helpers a few lines down can use them; the
  `AmalgamePredicate` / `AmalgameAction` typedefs from v0.3.4
  are gone (no caller left). *(PR #133)*

### Compiler

- **CGen — closure-as-argument lowering** — `TryEmitListCall`
  recognises the seven new method names and lowers each via a
  new `EmitClosureArg` helper. The lambda is passed inline as a
  GCC compound-statement-expression that allocates the env,
  copies captured locals in, and yields a fresh
  `AmalgameClosure_new(...)`. `EmitLambdaCaptureCopy` is a tiny
  helper around the per-capture line so the self-host codegen
  sees `cap.Name` through a typed parameter (chained
  `.Get(i).Name` would erase to `void*` at the call boundary).
- **CGen — type inference for higher-order returns** —
  `InferTypeFromExpr` knows that a CALL whose member name is
  Filter / Map returns `AmalgameList*`, Any / All return
  `code_bool`, CountIf returns `i64`. So
  `let big = xs.Filter(...)` typechecks without an explicit
  annotation, and `EmitVarDecl` propagates the receiver's
  list-element type to the result so `big.Get(0)` lowers with
  the right cast. *(PR #133)*

### Documentation

- `docs/guide/02-language-tour.md` — Lambdas section gains a
  higher-order `Filter / Map / Reduce` example and a rewritten
  limitations note around the i64 boundary.
- `docs/guide/04-stdlib.md` — List section gains the table of
  higher-order methods.
- `docs/guide/05-runtime-and-interop.md` — new "Closures and
  higher-order calls" section with the AmalgameClosure struct,
  the three `_callN` wrappers, and the GCC compound-statement-
  expression idiom the CGen emits.
- `ROADMAP_COMPLET.md` — Lambda v2.5 split into a `[x]` partial
  (this release) + a `[ ]` non-int signatures pending. *(PR #134)*

### Known limitations (deferred to v2.5 final)

- Lambda arguments and results are still `(i64) → i64` at the
  C level. `xs.Map(x => x.Name)` over a `List<Class>` doesn't
  yet work — needs a TypeChecker layer that infers the lambda
  signature from the formal param at the call site, plus CGen
  to emit non-int `lam_N_fn` signatures.
- String interpolation `"x: {coll.Count()}"` doesn't yet
  propagate the inferred `AmalgameList*` to the embedded call.
  Workaround: stage in named locals before printing.
- ForEach mutating an enclosing `var` doesn't accumulate
  (closures capture by value). Reduce is the right tool.

## [v0.3.5] — 2026-05-08

The "developer experience" release. Three coordinated pieces:
**generic interfaces** with static contract verification,
**LSP hover + global completion** on top of the v0.3.4 diagnostics,
and **lambda v2** (multi-param + block body) on top of v0.3.4
single-param closures. Suite goes from **170/170 to 180/180**
under `./amc`.

### Language

- **Generic interfaces** — `interface IComparable<T> { Compare(T) -> int }`
  now type-parameterise. Classes declared `class Box implements
  IComparable<int>` go through a new `CheckImplementsContract`
  pass in the TypeChecker that walks every implemented interface,
  substitutes its generic params by the concrete args (recursing
  into nested generics like `List<T>` → `List<int>`, preserving
  trailing `?` nullability), and asserts every interface method
  exists on the class with the matching signature. Diagnostics
  point at the offending method/param with expected vs got
  types. Two new fields on `AstNode` (`Str3` for generic params
  CSV, `Str4` for the implements list CSV). No vtable / dynamic
  dispatch yet — duck-typed dispatch as before. *(PR #120)*

- **Lambda v2: multi-param + block body** — `(x, y) => x + y`
  and `x => { let d = x * 2; return d + 1 }` now parse and
  compile. Multi-param routes through new `AmalgameClosure_call2`
  / `_call3` runtime helpers; block bodies emit through `EmitBlock`
  with an `InLambdaBody` flag that boxes `RETURN_STMT` values as
  `void*`. The lambda AST node migrates from `Str=name` to
  `Params: List<PARAM>` to support N parameters. v1 closures
  (capturing single-param expression-bodied) keep their existing
  semantics. Lambdas in argument position with non-int signatures
  (e.g. `xs.Filter(x => x > 0)`) need a lambda-typing layer in
  the TypeChecker — that lands in v2.5. *(PR #129)*

### Tooling

- **`amc lsp` hover + global completion** — the LSP server
  shipped in v0.3.4 (diagnostics-only) now responds to
  `textDocument/hover` and `textDocument/completion` too. Hover
  walks the AST via a new `FindNodeAtPosition` helper to find
  the deepest named node covering the cursor, then returns its
  inferred type as Markdown via the new `TypeChecker.LookupNodeType`.
  Completion lists every global the resolver knows about
  (builtins + user classes / enums / functions), mapped to LSP
  `CompletionItemKind` (Class / Function / Variable). The VS
  Code client picks up the new capabilities automatically via
  the initialize handshake — no client changes. Member completion
  (`obj.<cursor>` narrowed to the receiver's type) and
  goto-definition / signature help remain follow-ups. *(PR #126)*

### Documentation

- **Stdlib expansion roadmap** — `ROADMAP_COMPLET.md` gains a
  "Core stdlib expansion" backlog entry listing eight modules
  everyday Amalgame code currently has to fake or shell out for:
  `DateTime`, `Json`, `Regex`, `Random`, `Encoding`, `Compress`,
  `Crypto`, `Threading`. Plus a new "Stdlib delivery model"
  open design question that lays out five alternatives to the
  current header-only-inline approach. *(PR #121)*

### Internal

- **Typed accessors on FullResolver** — `ProgramCount` /
  `ProgramAt` (used by the contract verifier), then `GlobalCount`
  / `GlobalNameAt` / `GlobalTypeAt` (used by the completion
  provider). Both batches work around the same self-host quirk
  where chained member access through a `List<T>` field emits
  `Programs_Count(...)` instead of `Programs->Count(...)`.
- **`-Wint-conversion` hotfix** — `let pn = ...ProgramCount()`
  in `FindInterface` lost its int type to `void*` erasure across
  the method call boundary. Pinned via explicit `let pn: int`,
  matching the precedent in `src/lsp.am` for `String_Length`
  results. Local builds had ignored the warning; CI on Linux
  and macOS treats it as an error. *(PR #124)*

## [v0.3.4] — 2026-05-08

The "tooling" release. Five new pieces ship together: capturing
closures in the language, two new lint checks, a `Process` stdlib
module, an `amc test` runner, and a minimal LSP server (`amc lsp`)
paired with a VS Code client. Suite goes from **150/150 to
170/170** under `./amc`.

### Language

- **Capturing closures** — `let f = x => x + n` now captures `n`
  (and any other enclosing local) by value at creation time, not
  by textual substitution. Replaces the v0.3.x macro path
  (`#define f(x) (n+x)`) which couldn't carry an environment.
  Pieces: a new `AmalgameClosure { fn, env }` runtime type with
  `Closure_new` / `Closure_call1` helpers; capture analysis in
  the resolver (free identifiers below `LambdaBoundary`); a
  per-lambda `LamEnv_N` struct + `lam_N_fn` top-level function
  emitted by a Phase A walk of CGen Pass 2; identifier-callable
  closures dispatched via `AmalgameClosure_call1` with arg/result
  boxing through `intptr_t`. v1 is single-param, expression-bodied,
  arity-1 `(i64) -> i64`. Multi-param, block bodies, lambdas in
  argument position, and non-int signatures are tracked for v2.
  *(PR #101)*

### Tooling

- **`amc --lint` extensions** — two new categories on top of the
  v0.3.3 unreachable-code check:
  - **Unused locals** (`let x = …` never read; prefix `_` to silence)
  - **Shadowed names** (a `let` reuses a name visible in an
    enclosing scope, including method params)

  The linter keeps its own scope stack with append-only
  `UsedNames` + per-local snapshot, sidestepping the missing
  `List<T>.Set` op. Method/lambda params participate in shadow
  detection but are never warned-on as unused. *(PR #104)*

- **`Process` stdlib module** — `Process.Run(cmd) -> int` and
  `Process.RunCapture(cmd) -> AmalgameProcessResult { Exit,
  Stdout, Stderr }`. Cross-platform via `popen`/`_popen`; exit
  codes decoded with `WEXITSTATUS` / `WTERMSIG` (signals
  surface as `128 + signum`, shell convention). Stderr is
  currently merged into Stdout via shell `2>&1`; v2 will split
  them with a real pipe pair. *(PR #106)*

- **`amc test [<dir>]`** — discovers `*_test.am` files under
  `<dir>` (default `.`), compiles + runs each via `Process`, and
  aggregates `[PASS] <name>` / `[FAIL] <name>: <msg>` /
  `[SKIP] <name>` lines from each child's stdout. Crash-with-no-tags
  shows up as `[FAIL] <crash> exit=N`. Tests stay framework-free
  (just normal `Main`s that print tag lines). Exits non-zero on
  any FAIL or compile error. *(PR #109)*

- **`amc lsp` server + VS Code client** — minimal LSP 3.x server
  speaking JSON-RPC 2.0 over stdio. v1 implements `initialize` /
  `shutdown` / `exit`, `textDocument/didOpen` / `didChange` /
  `didClose` (Full sync), and `textDocument/publishDiagnostics`
  (push on every did{Open,Change}). Diagnostics merge resolver
  and typechecker errors and underline the whole token at the
  error column. JSON handling is ad-hoc — `JsonStr` / `JsonInt`
  helpers find `"<key>"` and read the value, no real parser.
  Hover and completion are out of scope for this release.

  Editor side: `editors/vscode/` (now version 0.2.0) gains an
  `extension.js` that spawns `amc lsp` via `vscode-languageclient`.
  Configurable via `amalgame.serverPath` (default `amc`) and
  `amalgame.enableLsp` settings. `npm install` is a one-time
  prerequisite. *(PR #110)*

### Compiler

- **Lexer-bootstrap CR escape fix** — the bootstrapping amc binary
  lexed `"\r"` as the two chars `\` + `r` instead of a single CR
  (0x0d) byte, which made `EscapeStringForC`'s `String_Replace(s,
  "\r", "\\r")` a no-op and propagated through `lsp.am`'s framing,
  causing the LSP wire to emit literal `\r\n` (5c 72 0a) instead
  of CR LF (0d 0a). VS Code's LanguageClient never matched the
  framing and stayed in Starting forever. Fixed by routing CR
  through `String_FromByte(13)`, which is just an int→byte builtin
  and has no escape-table dependency; same idea for `\x1b` (ESC).
  *(PR #110)*

- **CGen return-type table extended** — `Console_{Read,Flush,Write}`,
  `Process_{Run,RunCapture}`, and the `String_*` int/bool returns
  (`Length`, `IndexOf`, `LastIndexOf`, `ToInt`, `StartsWith`,
  `EndsWith`, `Contains`, `IsEmpty`) gain explicit entries. Fixes
  a pre-existing latent bug: an unannotated
  `let n = String_IndexOf(...)` defaulted to `code_string`, breaking
  arithmetic on the result. *(PR #110)*

- **Resolver structured errors** — new
  `RawErrors: List<ResolverError>` kept in lock-step with the
  formatted `Errors: List<string>`. Exposes `(file, line, col, msg)`
  for downstream tools (the new LSP, future IDE features). The
  formatted-string CLI path is unchanged. *(PR #110)*

### Runtime

- **`Console_ReadBytes(n)`** — reads exactly `n` bytes from stdin.
  Used by the LSP framing reader after parsing `Content-Length`.
- **`Console_Flush()`** — drains stdout. Required by the LSP
  server so the client doesn't block on buffered replies.
- **`AmalgameClosure { fn, env }`** + `AmalgameClosure_new` /
  `AmalgameClosure_call1` — backbone of the new closure feature.
- **`AmalgameProcessResult { Exit, Stdout, Stderr }`** + the
  `Process_Run` / `Process_RunCapture` helpers.

### Tests / infra

- Suite is now **170/170** (108 core + 50 stdlib + 12 fmt) under
  `./amc` — +20 vs v0.3.3. New samples: `tests/samples/closures_capture.am`,
  `lint_unused_shadow.am`, `process_api.am`,
  `test_runner/{arith,mixed}_test.am`. New helpers in
  `run_tests.sh`: `run_amc_test_check`, `run_lsp_check`.

### Docs

- `docs/guide/07-internals.md` gains sections on the **test runner**
  (`amc test`) and the **LSP server** (`amc lsp`, `src/lsp.am`).
  The Linter section is rewritten to cover the new unused / shadow
  checks.
- `docs/language/grammar.{ebnf,md}` — full rewrite. The previous
  files dated from v0.1.0 (back when the language was called *CODE*)
  and described constructs the parser never implemented while
  missing everything added since (try/catch, match-as-expression,
  closures, list comp, null-safety, decorators, generics, …). The
  new EBNF mirrors `src/parser/parser.am` exactly.
- README adds bullets for `--lint`, `amc test`, and `amc lsp`.

---

## [v0.3.3] — 2026-05-08

Quality-of-life follow-up to v0.3.2. Cleans up cosmetic typechecker
noise during multi-file compilation, ships a first `amc --lint`
flag (basic static analysis), and stops the workflow from leaking
a stale v0.3.0 artefact into every release. Suite goes from
149/149 to **150/150**.

### Compiler

- **Multi-file false-positive cleanup** — `build_amc.sh` step 1
  invokes `amc` on ten compiler sources at once, and the
  typechecker was emitting ~93 spurious errors on that path
  (build still went through, but the noise drowned out anything
  real). Three small fixes in `CheckProgram` / `CheckReturn`:
  the per-program filename is now read from `prog.Str2` rather
  than the constructor argument; `ExprType*`, `LocalNames`/
  `Types` and `ScopeStarts` reset between programs; and bare
  `return` (parsed as `Return(_unknown_, …)`) is recognised as
  such instead of being treated as a value-return.
  `build_amc.sh` is now silent.

### Tooling

- **`amc --lint`** — new flag that runs a static-analysis pass on
  top of the parsed AST and emits non-fatal warnings. MVP catches
  unreachable code after `return` / `throw` / `break` / `continue`,
  including inside nested `if` / `while` / `for-in` / `try` bodies.
  The skeleton (`src/linter.am`) is set up to grow more checks
  (unused locals, shadowed names, …) without touching anywhere
  else.

### CI

- **Drop stale v0.3.0 artefact, stop tracking `dist/`** — a
  `dist/amc-0.3.0-linux-x86_64.tar.gz` had been committed during
  v0.3.0 and was being uploaded on top of every later release by
  the workflow's `dist/*.tar.gz` glob. Removed, and `dist/` is now
  in `.gitignore` so future stagings don't sneak back in.

### Tests / infra

- Suite is now **150/150** (88 core + 50 stdlib + 12 fmt) under
  `./amc`. New samples: `tests/samples/lint_test.am`. New helper:
  `run_lint_check` in `run_tests.sh`.

---

## [v0.3.2] — 2026-05-08

Closes every open compiler bug tracked in v0.3.1's `SKIP_SELFHOST`
list, restores `try` / `catch` / `throw` / `finally` end-to-end in
the self-hosted parser, and ships the first round of generic type
inference for `List<T>` and `Map<K,V>`. The full test suite is
**149/149 green with zero SKIPs**.

### Language

- **`try` / `catch` / `throw` / `finally`** restored end-to-end in the
  self-hosted compiler (the Vala bootstrap had it; the rewrite hadn't
  grown a `ParseTry` yet). Lowers via `setjmp`/`longjmp` using the
  existing `_am_throw` runtime helpers.
- **`Type.Variant` patterns in `match`** — `Direction.North => …` is
  now recognised as a pattern. `ParseMatchPattern` reads an optional
  `.IDENT` suffix.
- **Generic type inference (locals + params + returns)**:
  `let xs = new List<int>()`, `xs: List<int>`, `xs = MakeList()`,
  and `Foo(List<int> xs)` all carry the elem type, so `xs.Get(i)`
  lowers to `(int)AmalgameList_get(xs, i)` without a manual cast.
  Same for `Map<K,V>.Get(k)` (which wasn't even dispatched before).
- **`obj.Method()` instance syntax for strings** — `s.Length()`,
  `"foo".Trim()`, `s.Replace(a, b)`, … route through `String_*`.

### Compiler

- **Multi-file type checking** — TypeChecker now walks every parsed
  program (was hard-coded to `programs[0]`).
- **Null-safety** — three independent bugs fixed: NodeKey collision
  in the typechecker (Kind is now part of the key), bare `?` was
  dropped by the lexer (added `OP_QMARK`), and `TypeToC("T?")` no
  longer doubles up the C pointer.

### Formatter

- **Same-line comments** preserved on round-trip: `let x = 1  // foo`
  no longer sees its comment bumped to the next line.
- **`import` directives** preserved: the parser used to discard them.
- `try` / `catch` / `throw` round-trip lossless.

### Tests / infra

- Suite is now **149/149** (87 core + 50 stdlib + 12 fmt) under
  `./amc`. **`SKIP_SELFHOST` is empty.**
- `run_multifile_test` shell helper fixed (was passing `-o foo.c`,
  amc dutifully expanded it to `foo.c.c` and never produced an
  executable).
- `tests/run_all_tests.sh` completes end-to-end for the first time
  (its `set -e` no longer trips on a half-failing suite).

[v0.8.75]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.8.75
[v0.5.0]:  https://github.com/amalgame-lang/Amalgame/releases/tag/v0.5.0
[v0.5.2]:  https://github.com/amalgame-lang/Amalgame/releases/tag/v0.5.2
[v0.5.3]:  https://github.com/amalgame-lang/Amalgame/releases/tag/v0.5.3
[v0.5.4]:  https://github.com/amalgame-lang/Amalgame/releases/tag/v0.5.4
[v0.5.5]:  https://github.com/amalgame-lang/Amalgame/releases/tag/v0.5.5
[v0.5.6]:  https://github.com/amalgame-lang/Amalgame/releases/tag/v0.5.6
[v0.6.0]:  https://github.com/amalgame-lang/Amalgame/releases/tag/v0.6.0
[v0.6.1]:  https://github.com/amalgame-lang/Amalgame/releases/tag/v0.6.1
[v0.6.2]:  https://github.com/amalgame-lang/Amalgame/releases/tag/v0.6.2
[v0.6.3]:  https://github.com/amalgame-lang/Amalgame/releases/tag/v0.6.3
[v0.6.4]:  https://github.com/amalgame-lang/Amalgame/releases/tag/v0.6.4
[v0.7.0]:  https://github.com/amalgame-lang/Amalgame/releases/tag/v0.7.0
[v0.7.1]:  https://github.com/amalgame-lang/Amalgame/releases/tag/v0.7.1
[v0.7.2]:  https://github.com/amalgame-lang/Amalgame/releases/tag/v0.7.2
[v0.7.3]:  https://github.com/amalgame-lang/Amalgame/releases/tag/v0.7.3
[v0.4.17]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.17
[v0.4.16]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.16
[v0.4.15]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.15
[v0.4.14]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.14
[v0.4.13]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.13
[v0.4.12]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.12
[v0.4.11]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.11
[v0.4.10]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.10
[v0.4.9]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.9
[v0.4.8]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.8
[v0.4.7]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.7
[v0.4.6]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.6
[v0.4.5]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.5
[v0.4.4]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.4
[v0.4.3]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.3
[v0.4.2]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.2
[v0.4.1]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.1
[v0.4.0]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.4.0
[v0.3.6]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.3.6
[v0.3.5]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.3.5
[v0.3.4]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.3.4
[v0.3.3]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.3.3
[v0.3.2]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.3.2
[v0.8.25]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.8.25
[v0.8.24]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.8.24
[v0.8.23]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.8.23
[v0.8.22]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.8.22
[v0.8.21]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.8.21
[v0.8.20]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.8.20
[v0.8.19]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.8.19
[v0.8.18]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.8.18
[v0.8.17]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.8.17
[v0.8.16]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.8.16
[v0.8.15]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.8.15
[v0.8.14]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.8.14
[v0.8.89]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.8.89
[v0.8.90]: https://github.com/amalgame-lang/Amalgame/releases/tag/v0.8.90
