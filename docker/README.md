# Amalgame IDE — Docker image

A batteries-included container: open a browser, get a full VS Code with
the Amalgame toolchain already wired up. No local install, no extension
hunting, no LSP/DAP setup — `docker run` and start coding.

## What's inside

- **code-server** — open-source VS Code, served over HTTP
- **Amalgame VS Code extension** — syntax highlighting, LSP client
  (`amc lsp`), DAP debugger (`amc dap`)
- **`amc` compiler** + runtime + bundled stdlib, on `PATH`
- **gcc + gdb** — so build, run and debug all work offline
- **`MyFirstApp/`** — an F5-ready sample project, open by default
- **📚 Amalgame Docs** — the full user guide, pinned in the sidebar

## Quick start

```sh
docker run --rm -p 8080:8080 ghcr.io/amalgame-lang/amalgame-ide
```

Open <http://localhost:8080>. The window lands on the sample project with
the docs alongside. Press **F5** to build (debug) and step through
`MyFirstApp/app/main.am`.

### Persist your work

Mount a host directory over the projects area:

```sh
docker run --rm -p 8080:8080 -v "$PWD/work:/home/coder/work" \
  ghcr.io/amalgame-lang/amalgame-ide
```

### Password / remote exposure

Auth is **disabled** by default (convenient on localhost). Set `PASSWORD`
before exposing the container beyond your machine:

```sh
docker run -p 8080:8080 -e PASSWORD='choose-a-strong-one' \
  ghcr.io/amalgame-lang/amalgame-ide
```

| Env var | Default | Meaning |
|---------|---------|---------|
| `PORT` | `8080` | bind port inside the container |
| `PASSWORD` | *(unset)* | when set, code-server requires this password |
| `AMALGAME_WORKSPACE` | `…/amalgame.code-workspace` | folder/workspace to open |

## Build it yourself

From the **repo root** (the build context is the whole repo — amc is
bootstrapped from `snapshot/amc_lib.c` exactly like CI):

```sh
docker build -f docker/Dockerfile -t amalgame-ide .
docker run --rm -p 8080:8080 amalgame-ide
```

## How it's published

`.github/workflows/docker.yml` builds `linux/amd64` (on `ubuntu-latest`)
and `linux/arm64` (natively on `ubuntu-24.04-arm`), pushes each by digest
to `ghcr.io/amalgame-lang/amalgame-ide`, then merges them into one
multi-arch manifest. Runs on version tags (`v*`) and on manual dispatch.
