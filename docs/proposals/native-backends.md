# Native backends & cross-platform compilation

**Status:** draft, 2026-05-31. Long-term architecture proposal.
Goal: ship Amalgame binaries for **POSIX (Linux/macOS) and
Windows** from a single host, *and* lay the groundwork to
eventually emit native code without an external C compiler — by
factoring `CGen` behind a reusable internal IR with C as the
default backend and native backends as additive plug-ins.

## TL;DR for the impatient

1. **You do not need a new codegen to ship Windows/macOS
   binaries today.** amc transpiles to C; the same `.c` already
   compiles everywhere. The cheap win is swapping the *driver*
   (`gcc` → `zig cc`) to cross-compile from Linux, plus porting
   the **runtime** (Boehm GC, `pthread`) to Windows.
2. **A new codegen only pays off if the goal is to drop the C
   dependency** (self-contained amc) or hit targets C can't reach
   (WASM, bare-metal). That's a multi-quarter effort.
3. This doc proposes doing **both, in layers**: keep C as the
   default backend, but refactor the emitter behind an internal
   **AmIR** so that QBE / LLVM / WASM backends become additive,
   not rewrites.

## Why this doc

Today the entire compilation surface is:

```
.am  →  CGen  (src/generator/c_gen.am, ~5400 LOC)  →  .c  →  gcc/g++  →  native binary
```

`CGen` is the *only* generator. `main.am` then shells out to
`gcc -O2 … -lgc -lm -lz -ldl -lpthread -o <bin>`
([src/main.am](../../src/main.am) around the link step).

This is fine, and "compile directly for Windows" is partly a
**misframing**: C *is* the portable layer. The same emitted `.c`
builds on Linux, macOS and Windows; `main.am` even carries
`#ifdef _WIN32` / `#include <windows.h>` branches already. The
real blockers to a one-command multi-OS release are:

1. **The driver** assumes a host-native `gcc`. No cross-compile.
2. **The runtime libs** (`-lgc -lm -lz -ldl -lpthread`) must
   exist and behave on the target. `dl`/`pthread` are POSIX-only;
   Boehm GC needs its Windows build.
3. **The runtime headers** (`runtime/*.h`) use POSIX syscalls in
   a few hot spots (process, net, IO) with no Windows fallback.

None of those three is a codegen problem. So the proposal splits
into two independent tracks that share one architectural spine.

---

## Layer 0 — Architecture: factor `CGen` behind **AmIR**

The single most valuable long-term move, and the prerequisite
for *any* second backend, is to stop treating "lower the AST" and
"print C" as the same pass.

### Current coupling

`CGen` walks the typechecked AST and emits C strings inline
(`EmitStmt`, `EmitExpr`, `EmitCalleeStr`, `TypeToC`, …). All the
*semantic* lowering decisions — variadic arg collection into an
`AmalgameList*`, lambda env-struct capture, `this.Field` type
resolution, generic elem-type propagation, boxing into `void*`
inside lambda bodies — are entangled with C syntax. A second
backend would have to re-derive every one of those decisions.

### Proposed shape

Introduce a thin **AmIR** (Amalgame Intermediate Representation):
a backend-neutral, already-lowered tree. Not SSA, not a CFG — just
"the AST after every Amalgame-specific decision is resolved,
expressed in primitives every backend understands."

```
.am → AST → Typecheck → Lower (AmIR)  ┬→ CBackend   → .c   → cc/zig cc → bin
                                       ├→ QBEBackend → .ssa → qbe+as+ld → bin
                                       ├→ LLVMBackend→ .ll  → llc/clang → bin
                                       └→ WasmBackend → .wat/.wasm
```

AmIR node kinds (illustrative, ~25 total):

```
IrFunc{ name, params:[IrType], ret:IrType, body:[IrStmt], variadicFrom:int? }
IrStruct{ name, fields:[(name, IrType)] }
IrLet{ name, ty, init:IrExpr }
IrAssign / IrIf / IrWhile / IrReturn / IrExprStmt
IrCall{ callee, args, variadicCollectFrom:int? }   // arg collection already decided
IrFieldGet / IrFieldSet
IrBox{ inner, ty } / IrUnbox{ inner, ty }          // lambda void* boxing made explicit
IrClosure{ envStruct, captures:[name], fn }        // capture set already computed
IrLiteral{ kind, value }
IrType = I64 | F64 | Bool | Ptr(IrType) | Struct(name) | List(elem) | Map(k,v) | Void
```

Key property: **all the hard inference already done in
`InferTypeFromExpr` / the variadic-arity table / the lambda
capture walk happens once, in Lower, and is baked into AmIR.**
Each backend becomes a near-mechanical pretty-printer.

**Migration is incremental and risk-free:**

- Phase 0a: extract the current emitter's *decisions* into a
  `Lower` pass that produces AmIR, and rewrite `CGen` as
  `CBackend(AmIR) → C`. Net behaviour identical; the golden test
  is *byte-identical (or semantically identical) emitted C*.
- Validate with the existing `gen_test.am` + the full bundle
  suite. If the C is equivalent, the refactor is sound.
- Only *after* AmIR is the contract, add a second backend.

This is the "Architecture IR interne" the proposal centres on:
the interface contract is `interface Backend { emit(ir: AmIR,
target: Target): EmitResult }`, and `Target` carries
`{ os, arch, abi }`.

---

## Layer 1 — Toolchain / driver (the cheap multi-OS win)

This track ships **without any new codegen** and delivers the
literal request: `amc build --target x86_64-windows` produces a
`.exe` from Linux.

### Backend/toolchain comparison

| Option | Cross-compile? | Win PE? | New gen? | Size/dep | Verdict |
|---|---|---|---|---|---|
| **`zig cc`** as C driver | ✅ all triples | ✅ | ❌ | ~45 MB, single binary | **Recommended** for track 1 |
| **mingw-w64** cross GCC | Win only | ✅ | ❌ | apt package | Fallback / CI matrix |
| **Native `cc` per host** (today) | ❌ | via MSVC | ❌ | — | Status quo |
| **LLVM IR backend** | ✅ via `-target` | ✅ | ✅ | LLVM ~bigass | Track 2, heavy |
| **QBE backend** | partial (amd64/arm64/riscv) | ❌ (no PE) | ✅ | ~10k LOC C, tiny | Track 2, elegant but POSIX-only |
| **Direct ELF/PE/Mach-O** | ✅ | ✅ | ✅✅✅ | reinvent the world | Not worth it |

**Recommendation for track 1: adopt `zig cc` as an optional C
driver.** It is a drop-in `cc` that cross-compiles to
`{linux,macos,windows}×{x86_64,aarch64}` from any host, bundles
musl/MinGW headers, links statically, and needs nothing on the
target. The emitted C is unchanged — only the link command in
`main.am` (and the test runner + `new_cmd.am` scaffolder) gains a
`--target` → triple translation.

```
amc build app.am --target x86_64-windows-gnu   # → app.exe
amc build app.am --target aarch64-macos        # → app   (Mach-O)
amc build app.am                               # → host native (gcc, unchanged)
```

Driver selection: `AMC_CC` env / `--cc` flag, default `gcc`,
`zig cc` when `--target` is non-native or `--cc=zig` is passed.
`Target` from Layer 0 maps 1:1 to a zig triple.

mingw-w64 stays as the CI fallback and for users who already have
it, but `zig cc` is the recommended default because it's a single
vendored binary and covers macOS too.

---

## Layer 2 — Runtime portability (the *actual* hard part)

Cross-compiling the C is easy; making the **runtime** behave on
Windows is the real work. Inventory of what's POSIX-bound today:

| Concern | Today | Windows action |
|---|---|---|
| **GC** | `-lgc` (Boehm) | Boehm builds on Windows (`gc.dll` / static). Vendor a prebuilt `libgc` per target, or compile from source in CI. |
| **Threads** | `pthread` (`Amalgame_Process.h`) | Abstract behind `am_thread_*`; back with Win32 `CreateThread`/`SRWLOCK`. |
| **Dynamic linking** | `-ldl` (`dlopen`) | `LoadLibrary`/`GetProcAddress`. Only used if amc itself dlopen's — audit whether user code needs it at all. |
| **Compression** | `-lz` | zlib builds on Windows; vendor per target. |
| **Process/exec** | `fork`/`exec`/`Process.RunCapture` | `CreateProcess` + pipes. Biggest single porting item. |
| **Net** | `Amalgame_Net.h` BSD sockets | Winsock2 (`WSAStartup`, `closesocket`, `SOCKET` not `int`). |
| **Filesystem/paths** | `/`-separated, POSIX stat | `\`, drive letters, `_stat64`. `src/stdlib/path.am` already abstracts some. |

**Strategy:** introduce a `runtime/platform/` split — a single
`am_platform.h` with `#ifdef _WIN32` / `#else` defining a small
POSIX-or-Win32 shim (`am_thread_create`, `am_proc_spawn`,
`am_sock_t`, `am_path_sep`). The existing headers call the shim
instead of raw syscalls. This is ~6–10 functions, not a rewrite,
and it's where the bulk of the effort actually lands.

Boehm GC on Windows deserves its own spike: it works, but needs
the right build flags (`-DGC_THREADS`, `--enable-threads=win32`)
and the `GC_use_threads_discovery` / thread-registration dance.
Document a vendored-prebuilt path so users don't compile GC.

---

## Layer 3 — CI matrix & release artifacts

Once track 1 + the runtime shim land, releases become a matrix
build. Proposed GitHub Actions shape:

```yaml
strategy:
  matrix:
    target:
      - x86_64-linux-gnu
      - aarch64-linux-gnu
      - x86_64-macos
      - aarch64-macos
      - x86_64-windows-gnu
```

Two build modes worth supporting:

1. **Single-host cross-build** (fast, `zig cc`): one Linux
   runner emits every target. Good for nightly artifacts.
2. **Native-runner matrix** (canonical, slower): `ubuntu`,
   `macos`, `windows` runners each build+**test** natively. This
   is the one that gates releases, because cross-built binaries
   should still be smoke-tested on a real OS of that family.

Per-target release artifacts: `amc-vX.Y.Z-<triple>.tar.gz`
(`.zip` for Windows), each bundling the `amc` binary + `runtime/`
+ a vendored `libgc`/`libz` for that target. The
`reference_ci_ubuntu2404_gotchas` notes already apply to the
Linux leg.

Note: the test runner (`amc test`) shells out to `gcc` today and
will need the same `--target`/driver awareness; cross-built test
binaries can only *run* on the matching native runner, so the
single-host cross job builds-only and the native matrix runs.

---

## Track ordering & effort

| Phase | Scope | Needs Layer 0? | Rough effort |
|---|---|---|---|
| **P1** | `zig cc` driver + `--target` flag, host behaviour unchanged | no | small (driver plumbing in `main.am`/`new_cmd.am`/test runner) |
| **P2** | `runtime/platform/` shim, Windows runtime (threads/proc/net/GC) | no | **large** — the real cost |
| **P3** | CI matrix + per-target release artifacts | no | medium |
| **P4** | Extract `Lower → AmIR`, rewrite `CGen` as `CBackend(AmIR)` | — (defines it) | medium-large, zero user-visible change |
| **P5** | First native backend on AmIR (**QBE** for POSIX, or LLVM for full coverage) | yes | large |
| **P6** | WASM / freestanding backends | yes | exploratory |

**Recommended sequencing:** P1 → P2 → P3 gives the entire
"distribute for POSIX & Windows" payoff *with no codegen work*.
P4 is the architectural investment that can proceed in parallel
(it's a refactor gated only by golden-output tests). P5/P6 — the
actual "gen supplémentaire" — only start once P4's AmIR contract
is stable, and only if dropping the C dependency proves worth it.

The key insight to hold onto: **the multi-OS goal is a runtime +
driver problem (P1–P3), not a codegen problem. The new backend
(P4–P6) is a separate, optional, longer-horizon bet** justified by
self-containment, not by Windows support per se.

## Open questions

- Is dropping the C compiler dependency actually a goal, or is
  "works on Windows" the whole ask? If the latter, P5/P6 may never
  be worth starting — C stays the backend forever and that's fine.
- QBE (tiny, elegant, POSIX-only — no Windows PE) vs LLVM (heavy,
  covers everything incl. Windows + WASM). If a native backend
  ever happens, which trade-off? QBE can't be the *Windows* story,
  so a QBE backend would coexist with the C-via-`zig cc` Windows
  path rather than replace it.
- Does any user code path actually need `dlopen`/`-ldl`, or is it
  only amc-internal? If unused by emitted programs, drop it from
  the link line and shrink the Windows surface.
- Boehm GC vs a bump/arena allocator vs eventually an Amalgame-
  native GC: the runtime memory model is the deepest portability
  dependency and deserves its own proposal.
