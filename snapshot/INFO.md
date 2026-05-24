# Amalgame compiler snapshot

Captured: 2026-05-25T00:43:25+02:00
Git rev:  5e1f3c4bf7c25c69b4619778160804e32851c379
Branch:   feat/udp-receivefrom-and-ephemeral-fix
Tests:    591 passed

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
