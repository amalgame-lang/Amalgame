# Amalgame compiler snapshot

Captured: 2026-05-07T23:54:45+02:00
Git rev:  345f0c85645a2ca9e497a71fc041739e6ddf7b55
Branch:   feature/release-v0.3.0
Tests:    136 passed

This snapshot is the canonical recovery binary when ./amc is broken
mid-development. `build_amc.sh` falls back to `snapshot/amc` before
`./build/amc` (Vala) so a known-good Amalgame compiler is always one
rung closer than the original bootstrap.

To recompile the snapshot binary on this platform:

    gcc -O2 -Iruntime snapshot/amc_lib.c -lgc -lm -lcurl -o snapshot/amc

To restore an older snapshot (after a bad commit):

    git checkout <good-rev> -- snapshot/amc_lib.c
    gcc -O2 -Iruntime snapshot/amc_lib.c -lgc -lm -lcurl -o snapshot/amc
