# Lexer CODE

> **Version** : 0.1.0
> **Fichiers** : `src/core/lexer/token.vala`,
>                `src/core/lexer/lexer.vala`
> **Statut**   : ✅ Complet et fonctionnel

---

## Rôle

Transforme le code source brut en liste de tokens.

"Hello World !"  →  [KW_NAMESPACE][IDENTIFIER][NEWLINE]...


---

## Correction Importante : IsLetter/IsDigit

Vala 0.48 sur Debian : `c.isalpha()` causait un bug
où les identifiants étaient tokenisés comme INTEGER.

**Solution** : helpers explicites

```vala
private bool IsLetter(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

private bool IsDigit(char c) {
    return c >= '0' && c <= '9';
}