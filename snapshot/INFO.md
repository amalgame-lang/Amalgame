# Amalgame compiler snapshot

Captured: 2026-05-08T01:04:07+02:00
Git rev:  e6b42f47ab289c84d1b9e5c42d6306eb32e02a4b
Branch:   feature/release-v0.3.1
Tests:    124 passed

This snapshot is the canonical recovery binary when ./amc is broken
mid-development. `build_amc.sh` falls back to `snapshot/amc` before
`./build/amc` (Vala) so a known-good Amalgame compiler is always one
rung closer than the original bootstrap.

To recompile the snapshot binary on this platform:

    gcc -O2 -Iruntime snapshot/amc_lib.c -lgc -lm -lcurl -o snapshot/amc

To restore an older snapshot (after a bad commit):

    git checkout <good-rev> -- snapshot/amc_lib.c
    gcc -O2 -Iruntime snapshot/amc_lib.c -lgc -lm -lcurl -o snapshot/amc
