/*
 * Stub runtime header for the fake-pkg fixture. Backs the manifest
 * at tests/fixtures/pm/cache/example.com/fake/fake-pkg/v0.1.0_deadbeef/
 * amalgame.toml — declares the four functions the PackageRegistry
 * e2e test exercises (Init, Tick, IsOk, Close).
 *
 * Not a real binding to anything — just enough symbols for gcc to
 * link a user binary compiled via the manifest-driven dispatch.
 */

#ifndef FAKE_PKG_H
#define FAKE_PKG_H

#include "_runtime.h"

typedef struct AmalgameFakePkg {
    i64 ticks;
} AmalgameFakePkg;

static inline AmalgameFakePkg* Amalgame_Fake_FakePkg_Init(void) {
    AmalgameFakePkg* p = (AmalgameFakePkg*) GC_MALLOC(sizeof(AmalgameFakePkg));
    p->ticks = 0;
    return p;
}

static inline i64 Amalgame_Fake_FakePkg_Tick(AmalgameFakePkg* p) {
    if (!p) return 0;
    p->ticks++;
    return p->ticks;
}

static inline code_bool Amalgame_Fake_FakePkg_IsOk(AmalgameFakePkg* p) {
    return p ? 1 : 0;
}

static inline void Amalgame_Fake_FakePkg_Close(AmalgameFakePkg* p) {
    (void) p;  /* GC-managed, nothing to free */
}

#endif /* FAKE_PKG_H */
