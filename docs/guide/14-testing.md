# Testing

Amalgame's test story is deliberately framework-free. A test is a normal
program that prints `[PASS]` / `[FAIL]` / `[SKIP]` lines; `amc test`
discovers those programs, compiles and runs each, and tallies the tags.
There's no assertion library to learn, no annotations, no test runner
DSL — just `Console.WriteLine` and a naming convention.

---

## Write a test

A test file is named `*_test.am` and has a `Main()` that prints a tagged
line per check:

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

The three tags the runner counts, at the **start of a line**:

| Tag | Meaning |
|-----|---------|
| `[PASS]` | the check held |
| `[FAIL]` | the check did not hold — the run is failing |
| `[SKIP]` | intentionally not run (e.g. needs a service, known-flaky) |

```amalgame
Console.WriteLine("[PASS] sanity: hello")
Console.WriteLine("[FAIL] todo: not implemented yet")
Console.WriteLine("[SKIP] flaky: see issue #42")
```

That's the whole contract. Anything you can compute, you can assert by
branching and printing the right tag.

---

## Run the suite

```bash
amc test                  # discover *_test.am under . (recursively), run all
amc test ./tests          # restrict discovery to a directory
```

`amc test` compiles each discovered file, runs it, and aggregates the
tags into a summary:

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

A file that **crashes** (or emits no tags) is surfaced as a failure, so a
segfault or a forgotten body can't masquerade as "0 failures".

### Options

| Flag | Effect |
|------|--------|
| `--filter <pat>` | only run tests whose path contains `<pat>` |
| `--ci` | terse output — drop `PASS`/`SKIP` lines, keep `FAIL` + the tally |
| `--list` | print the discovered paths and exit (no compile, no run) |

```bash
amc test --filter parser        # just the parser tests
amc test --ci                   # CI-friendly: noise down, failures up
amc test --list                 # what would run?
```

---

## Organising tests

- **Naming is the discovery mechanism.** Only `*_test.am` files are
  picked up. A helper file without the suffix is ignored by the runner
  (good — put shared fixtures in plain `.am` files).
- **Group by directory.** `amc test ./tests/parser` runs one area;
  `--filter` slices across directories.
- **`fixtures/` is pruned.** Files under any nested `fixtures/` directory
  are *not* auto-run — that's where you keep sample inputs and
  not-meant-to-execute `.am` files. To run one explicitly, point at it:
  `amc test ./tests/fixtures/<name>/`.

> Larger suites in this project are organised as **bundles** — a single
> `*_test.am` whose `Main()` drives many cases (the compiler's own
> `core_test.am`, `stdlib_test.am`, etc.). A bundle can even shell out to
> `amc test` on a fixture directory and grep its summary line, which is
> how the runner tests itself. Start with one file per area; reach for a
> bundle when setup cost is worth amortising.

---

## A pattern for assertions

Since there's no assert library, a tiny local helper keeps test bodies
readable:

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

Keep the label descriptive — it's what you'll read in the `--ci` output
when something breaks. Note the helper is `public static` and called
qualified (`Program.check(...)`) — a static method invoked from `Main` is
reached through its class name.

---

## Gotchas

- **Tags must start the line** (after leading whitespace the runner
  trims). `Console.WriteLine("result: [PASS]")` is *not* counted —
  put the tag first.
- **No tags = failure.** If a test file runs but prints nothing the
  runner counts it as failing, not as "0 tests". Always emit at least one
  tag.
- **`[SKIP]` is a real outcome, not a pass.** Use it (with a reason) for
  checks that need an unavailable service — it keeps the suite honest
  without going red.
- **Exit code follows the tally** — any `[FAIL]` (or a crash) makes
  `amc test` exit non-zero, so it drops straight into CI as a gate.

For the CLI reference (every `amc test` flag) see
[03-cli-reference.md](03-cli-reference.md); for the build pipeline that
CI runs around the tests, see [06-build-and-tooling.md](06-build-and-tooling.md).
