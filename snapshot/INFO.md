# Amalgame compiler snapshot

Captured: 2026-05-21T22:12:37+02:00
Git rev:  6e800e6fa231aa2e36b0df2300d2531c91036129
Branch:   feature/interp-capture-in-lambda
Tests:    (skipped) passed

This snapshot is the canonical bootstrap binary. `build_amc.sh` uses
`./amc` if present, otherwise falls back to `./snapshot/amc`. From a
clean clone, recompile `snapshot/amc` from the tracked
`snapshot/amc_lib.c` with `gcc` (one command, no other compiler
needed).

To recompile the snapshot binary on this platform:

    gcc -O2 -Iruntime snapshot/amc_lib.c -lgc -lm -o snapshot/amc

To restore an older snapshot (after a bad commit):

    git checkout <good-rev> -- snapshot/amc_lib.c
    gcc -O2 -Iruntime snapshot/amc_lib.c -lgc -lm -o snapshot/amc
