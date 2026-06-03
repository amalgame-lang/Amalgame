# syntax=docker/dockerfile:1
#
# ═══════════════════════════════════════════════════════════════════════
#  Amalgame IDE — batteries-included container
#
#  `docker run -p 8080:8080 ghcr.io/amalgame-lang/amalgame-ide` boots a
#  browser VS Code (code-server) that opens with:
#    • the Amalgame extension pre-installed (syntax, LSP, DAP)
#    • a ready-to-run sample project (MyFirstApp) — F5 just works
#    • the full user guide pinned in the sidebar
#    • amc + gcc + gdb on PATH so build / run / debug all work offline
#
#  Build (from the repo root, NOT from docker/):
#    docker build -f docker/Dockerfile -t amalgame-ide .
# ═══════════════════════════════════════════════════════════════════════

# ─────────────────────────────────────────────────────────────────────
# Stage 1 — bootstrap amc from the tracked snapshot (same path as ci.yml)
#           and package the VS Code extension into a .vsix.
# ─────────────────────────────────────────────────────────────────────
FROM debian:bookworm-slim AS builder

# Build deps mirror ci.yml's Linux job + Node for `vsce package`.
RUN apt-get update && apt-get install -y --no-install-recommends \
        gcc libc6-dev libgc-dev libcurl4-openssl-dev zlib1g-dev \
        ca-certificates nodejs npm \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . .

# Compile snapshot/amc from the tracked C, then self-host build_amc.sh.
# build_amc.sh runs tools/build-stdlib.sh as its last step, so
# lib/libamalgame.a is on disk afterwards.
RUN gcc -O2 -Iruntime snapshot/amc_lib.c -lgc -lm -lcurl -o snapshot/amc \
    && ./build_amc.sh \
    && ./amc --version

# Package the extension. `vsce package` bundles non-dev deps
# (vscode-languageclient), so the .vsix is self-contained.
RUN cd editors/vscode \
    && npm install --no-audit --no-fund --loglevel=error \
    && npx --yes @vscode/vsce package --out /tmp/amalgame.vsix

# Stage the XDG-style install tree under /opt/amalgame — this mirrors the
# layout release.yml ships and that amc's ResolveRuntimeDir probes for at
# <bin>/../share/amalgame/{runtime,lib,stdlib,docs}.
RUN set -eux; PREFIX=/opt/amalgame; \
    mkdir -p "$PREFIX/bin" \
             "$PREFIX/share/amalgame/runtime" \
             "$PREFIX/share/amalgame/lib" \
             "$PREFIX/share/amalgame/stdlib" \
             "$PREFIX/share/amalgame/docs/language" \
             "$PREFIX/share/amalgame/docs/guide" \
             "$PREFIX/share/amalgame/editors/vscode"; \
    cp amc                          "$PREFIX/bin/"; \
    cp runtime/*.h                  "$PREFIX/share/amalgame/runtime/"; \
    cp lib/libamalgame.a            "$PREFIX/share/amalgame/lib/"; \
    cp src/stdlib/json.am src/stdlib/toml.am src/stdlib/msgpack.am \
       src/stdlib/path.am src/stdlib/amc_buildinfo.am \
                                    "$PREFIX/share/amalgame/stdlib/"; \
    cp docs/language/grammar.ebnf   "$PREFIX/share/amalgame/docs/language/"; \
    cp -r docs/guide/.              "$PREFIX/share/amalgame/docs/guide/"; \
    cp /tmp/amalgame.vsix           "$PREFIX/share/amalgame/editors/vscode/"

# ─────────────────────────────────────────────────────────────────────
# Stage 2 — runtime image: code-server + amc + the pre-wired workspace.
# ─────────────────────────────────────────────────────────────────────
FROM debian:bookworm-slim

LABEL org.opencontainers.image.title="Amalgame IDE"
LABEL org.opencontainers.image.description="Browser VS Code with the Amalgame compiler, extension (syntax/LSP/DAP), a sample project and the docs — ready to use."
LABEL org.opencontainers.image.source="https://github.com/amalgame-lang/Amalgame"
LABEL org.opencontainers.image.licenses="MIT"

# Runtime deps:
#   gcc + libc6-dev + libgc/curl/zlib  → compile the C amc emits
#   gdb                                → `amc dap` debug backend
#   git                                → `amc package` fetches
#   curl/ca-certificates               → code-server install + TLS
#   locales                            → quiet code-server's locale warning
RUN apt-get update && apt-get install -y --no-install-recommends \
        gcc libc6-dev libgc-dev libcurl4-openssl-dev zlib1g-dev \
        gdb git curl ca-certificates locales tini \
    && sed -i 's/# en_US.UTF-8/en_US.UTF-8/' /etc/locale.gen \
    && locale-gen \
    && rm -rf /var/lib/apt/lists/*

ENV LANG=en_US.UTF-8 LC_ALL=en_US.UTF-8

# code-server (open-source VS Code in the browser). Installs the latest
# release for the image's architecture; the .vsix below is a local file
# so no marketplace / network is needed at runtime.
RUN curl -fsSL https://code-server.dev/install.sh | sh \
    && rm -rf /root/.cache

# Drop amc + runtime + stdlib + docs + vsix into /usr/local. Now
# /usr/local/bin/amc resolves /usr/local/share/amalgame/... with no env.
COPY --from=builder /opt/amalgame /usr/local

# Non-root user — code-server, extensions and the workspace all live here.
RUN useradd --create-home --shell /bin/bash --uid 1000 coder
WORKDIR /home/coder

# Workspace wiring shipped with the image.
COPY docker/amalgame.code-workspace /home/coder/amalgame.code-workspace
COPY docker/welcome.md              /home/coder/welcome.md
COPY docker/settings.json           /home/coder/.local/share/code-server/User/settings.json
COPY docker/entrypoint.sh           /usr/local/bin/amalgame-ide-entrypoint
RUN chmod +x /usr/local/bin/amalgame-ide-entrypoint \
    && chown -R coder:coder /home/coder

USER coder
ENV HOME=/home/coder

# 1) install the bundled extension into code-server
# 2) scaffold the F5-ready sample (vscode wiring is default-on)
# 3) pre-build it (release + debug) so Run and Debug work on first click
RUN code-server --install-extension \
        /usr/local/share/amalgame/editors/vscode/amalgame.vsix \
    && amc new MyFirstApp \
    && cd MyFirstApp \
    && ./build.sh \
    && ./build.sh -g

EXPOSE 8080
ENV PORT=8080 \
    AMALGAME_WORKSPACE=/home/coder/amalgame.code-workspace

# tini reaps the gcc/gdb children code-server spawns during build/debug.
ENTRYPOINT ["/usr/bin/tini", "--", "/usr/local/bin/amalgame-ide-entrypoint"]
