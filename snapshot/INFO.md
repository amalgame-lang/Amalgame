# Amalgame compiler snapshot

Captured: 2026-05-09T08:50:48+02:00
Git rev:  a86f2de73b28202f24a9f9ffa504840a143bfe13
Branch:   feature/migrate-v1-claude-api
Tests:    227 passed

This snapshot is the canonical recovery binary when ./amc is broken
mid-development. `build_amc.sh` falls back to `snapshot/amc` before
`./build/amc` (Vala) so a known-good Amalgame compiler is always one
rung closer than the original bootstrap.

To recompile the snapshot binary on this platform:

    gcc -O2 -Iruntime snapshot/amc_lib.c -lgc -lm -lcurl -o snapshot/amc

To restore an older snapshot (after a bad commit):

    git checkout <good-rev> -- snapshot/amc_lib.c
    gcc -O2 -Iruntime snapshot/amc_lib.c -lgc -lm -lcurl -o snapshot/amc
