# Amalgame compiler snapshot

Captured: 2026-05-17T17:46:21+02:00
Git rev:  c026f520ac31ef5917f1c13a7757d7bd42e8d60a
Branch:   release/v0.8.25
Tests:    (skipped) passed

This snapshot is the canonical bootstrap binary. `build_amc.sh` uses
`./amc` if present, otherwise falls back to `./snapshot/amc`. From a
clean clone, recompile `snapshot/amc` from the tracked
`snapshot/amc_lib.c` with `gcc` (one command, no other compiler
needed).

To recompile the snapshot binary on this platform:

    gcc -O2 -Iruntime snapshot/amc_lib.c -lgc -lm -lcurl -o snapshot/amc

To restore an older snapshot (after a bad commit):

    git checkout <good-rev> -- snapshot/amc_lib.c
    gcc -O2 -Iruntime snapshot/amc_lib.c -lgc -lm -lcurl -o snapshot/amc
