/*
 * Amalgame Standard Library — Amalgame.BuildInfo (provenance macros)
 * Copyright (c) 2026 Bastien MOUGET
 * https://github.com/amalgame-lang/Amalgame
 *
 * Since v0.7.4 (project G) the BuildInfo.GitRev() / BuildDate()
 * accessors live in src/stdlib/amc_buildinfo.am and read these
 * macros directly via `@c { return AMC_GIT_REV; }` inline-C
 * blocks. The header is reduced to just the two `#ifndef`
 * guards so a build that doesn't pass `-DAMC_GIT_REV=...` /
 * `-DAMC_BUILD_DATE=...` still compiles — the AM helpers then
 * return "" and `amc --version` skips the banner line.
 */

#ifndef AMALGAME_BUILDINFO_H
#define AMALGAME_BUILDINFO_H

#ifndef AMC_GIT_REV
#define AMC_GIT_REV ""
#endif

#ifndef AMC_BUILD_DATE
#define AMC_BUILD_DATE ""
#endif

#endif /* AMALGAME_BUILDINFO_H */
