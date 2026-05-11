/*
 * Amalgame Standard Library — Amalgame.BuildInfo
 * Copyright (c) 2026 Bastien MOUGET
 * https://github.com/amalgame-lang/Amalgame
 *
 * Provides: BuildInfo.GitRev(), BuildInfo.BuildDate()
 *
 * Both values are baked in at compile time via `-DAMC_GIT_REV=...`
 * and `-DAMC_BUILD_DATE=...` (build_amc.sh wires this up). When the
 * defines are absent (e.g. someone gcc's the C snapshot directly
 * outside the build script), the helpers return the empty string —
 * callers must treat empty as "no build info baked in" and skip the
 * line rather than emit a half-rendered banner.
 */

#ifndef AMALGAME_BUILDINFO_H
#define AMALGAME_BUILDINFO_H

#include "_runtime.h"

#ifndef AMC_GIT_REV
#define AMC_GIT_REV ""
#endif

#ifndef AMC_BUILD_DATE
#define AMC_BUILD_DATE ""
#endif

static inline code_string BuildInfo_GitRev(void) {
    return AMC_GIT_REV;
}

static inline code_string BuildInfo_BuildDate(void) {
    return AMC_BUILD_DATE;
}

#endif /* AMALGAME_BUILDINFO_H */
