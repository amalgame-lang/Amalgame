# Welcome to Amalgame 👋

This is a ready-to-use VS Code (code-server) environment for the
**Amalgame** programming language. Everything is wired up already:

| What | Status |
|------|--------|
| `amc` compiler on `PATH` | ✅ `amc --version` in a terminal |
| Syntax highlighting (`.am`) | ✅ |
| LSP (diagnostics, hover) | ✅ via `amc lsp` |
| Debugger (DAP) | ✅ via `amc dap` + gdb |
| Sample project | ✅ `MyFirstApp/` (left sidebar) |
| Full user guide | ✅ `📚 Amalgame Docs` folder (left sidebar) |

## Run it

Open **`MyFirstApp/src/main.am`**, then:

- **Run** — open a terminal (`` Ctrl+` ``) and:
  ```sh
  cd MyFirstApp && ./MyFirstApp
  ```
- **Debug** — press **F5**. The `amc: build (debug)` task rebuilds with
  `-g`, then `amc dap` launches it with `.am`-level breakpoints. Set a
  breakpoint in `src/main.am` and step through.

## Start your own

```sh
amc new myapp        # scaffold an F5-ready executable
cd myapp && ./build.sh
```

## Docs

The complete user guide ships offline in the **📚 Amalgame Docs** folder
(open any chapter and hit the Markdown preview button). Online:
<https://amalgame.me> and <https://docs.amalgame.me>.

---
*Set a `PASSWORD` env var on `docker run` before exposing this container
beyond `localhost`.*
