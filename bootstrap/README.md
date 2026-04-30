# Amalgame Bootstrap

Ce dossier contient le compilateur `amc` pré-compilé (binaire Vala stable).

## Pourquoi ce binaire existe

Quand `amc` sera écrit en Amalgame, il sera compilé par lui-même.
Si une modification casse `amc.am`, ce binaire permet de revenir à un état stable.

## Utilisation

```bash
# Sauvegarder le binaire actuel comme stable
amc bootstrap save

# Restaurer en cas de problème
amc bootstrap restore

# Valider que le compilateur se compile lui-même correctement
amc bootstrap validate
```

## Philosophie

```
amc-stable (Vala)  →  compile amc.am  →  amc-stage1
amc-stage1         →  compile amc.am  →  amc-stage2
diff(stage1, stage2) == 0  →  bootstrap validé ✅
```

Le binaire `amc-stable` est versionné via **Git LFS**.
Ne jamais le supprimer.

## Plateformes

- `amc-linux-x86_64`   — Linux x86-64 (Debian/Ubuntu)
- `amc-macos-arm64`    — macOS Apple Silicon
- `amc-macos-x86_64`   — macOS Intel
- `amc-windows-x86_64.exe` — Windows (MSYS2/MinGW)
