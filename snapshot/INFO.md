# Amalgame compiler snapshot

Captured: 2026-05-17T18:51:26+02:00
Git rev:  a5db58252ade3e38aeb75ad0dd7de30e0737c9b6
Branch:   feature/bugs-6-7-8-jsonvalue-stringconcat-nestedgenerics
Tests:    458 passed

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
