# examples/mcu — Amalgame on bare metal

The first program compiled **by amc itself** (not hand-written C) for a
microcontroller. See `docs/proposals/amc-embedded.md`.

```bash
sudo apt-get install -y gcc-arm-none-eabi qemu-system-arm   # one-time
./build.sh        # amc build --target=cortex-m3  (transpile+cross-compile+link+objcopy)
./run.sh          # → "Hello bare metal" via semihosting
```

Or directly, one command:

```bash
amc build --target=cortex-m3 hello.am -o hello   # → hello (ELF) + hello.bin + hello.hex
```

## What slice 2 wired

- `amc --target=cortex-m3 hello.am` emits **freestanding C**: prelude is just
  `#include "_runtime.h"` (the embedded runtime), entry is `amc_main()` (no
  `GC_INIT`, no argv), `new`/ctors route through `code_alloc` (the arena), and
  the Json/Toml stdlib auto-attach is off. **Zero `GC_*`, zero `<stdio.h>`,
  zero `malloc`.**
- Unknown targets are rejected: `amc --target=banana …` → error listing
  `cortex-m3, cortex-m4, cortex-m7`.
- Hosted (POSIX/Windows) is untouched — guarded byte-for-byte by
  `tools/golden-c/golden-c.sh`.

## Not yet (later slices)

- `setup`/`loop`/`region`/`persist` soft keywords (slice 3) — until then the
  program is a plain `Main()`.
- Real silicon board package `amalgame-mcu-nucleo-f767zi` (slice 6). Today
  only `cortex-m3` resolves a bundled board (QEMU lm3s6965); `cortex-m4/m7`
  report that they need a board package for startup + linker script.
