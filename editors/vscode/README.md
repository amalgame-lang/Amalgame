# Amalgame VS Code extension

Syntax highlighting and basic language support for the
[Amalgame programming language](https://github.com/BastienMOUGET/Amalgame).

## Features

- Syntax highlighting for `.am` files
- Bracket matching, auto-closing pairs
- Line/block comment toggling
- Indentation rules

## Install (development)

```bash
# From the repo root
cd editors/vscode
code --install-extension .
```

Or, for a quick local test, copy the directory to
`~/.vscode/extensions/amalgame-0.1.0/` and restart VS Code.

## Layout

- `package.json` — extension manifest, registers the `amalgame` language
- `language-configuration.json` — comments, brackets, indentation rules
- `syntaxes/amalgame.tmLanguage.json` — TextMate grammar (keywords,
  literals, decorators, operators, type names, etc.)

## Roadmap

The current scope is *highlighting only*. Future enhancements (LSP
client, completion, hover, go-to-def) are tracked in the main project's
`ROADMAP_COMPLET.md` under "🟢 PRIORITÉS ÉCOSYSTÈME".
