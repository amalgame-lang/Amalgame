# Amalgame VS Code extension

Syntax highlighting **and** an LSP client for the
[Amalgame programming language](https://github.com/amalgame-lang/Amalgame).

## Features

- Syntax highlighting for `.am` files
- Bracket matching, auto-closing pairs, indentation rules
- **LSP client** that connects to `amc lsp` for live diagnostics
  (resolver + typechecker errors as you type)

## Install (development)

The LSP client uses `vscode-languageclient`, so a one-time
`npm install` in this directory is required before VS Code can
load the extension.

```bash
cd editors/vscode
npm install                 # pulls vscode-languageclient
code --install-extension .  # or symlink (see below)
```

For a quick local test without packaging, symlink the directory
into VS Code's extensions folder and reload the window
(Cmd-Shift-P → *Developer: Reload Window*):

```bash
ln -s "$(pwd)" ~/.vscode/extensions/amalgame-0.2.0
```

The extension activates when you open a `.am` file. It expects
`amc` on `PATH` (configurable).

## Configuration

| Setting                | Default | What it does                                                                  |
|------------------------|---------|-------------------------------------------------------------------------------|
| `amalgame.serverPath`  | `amc`   | Path to the amc binary used as the LSP server. Use `${workspaceFolder}/amc` to point at a local build. |
| `amalgame.enableLsp`   | `true`  | Enable the LSP client. Set to `false` to fall back to syntax-only highlighting. |

## Trying it out

1. Build amc in the repo root: `./build_amc.sh`
2. Either add the repo root to `PATH`, or set
   `"amalgame.serverPath": "/abs/path/to/amc"` in your VS Code settings.
3. Open any `.am` file. You should see red squigglies on lines with
   resolver / typechecker errors. The output channel
   *Amalgame LSP* in the panel shows raw JSON-RPC traffic if you
   need to debug the connection.

## Layout

- `package.json` — extension manifest (language, grammar, LSP config)
- `extension.js` — LSP client (spawns `amc lsp`, pipes stdio)
- `language-configuration.json` — comments, brackets, indentation rules
- `syntaxes/amalgame.tmLanguage.json` — TextMate grammar

## Roadmap

The current scope is **diagnostics-only**. The server doesn't yet
implement hover, completion, or goto-definition; tracked in the main
project's `ROADMAP_COMPLET.md`.
