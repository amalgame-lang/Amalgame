#!/bin/bash
# Nettoyage des artefacts de debug à la racine du projet
# Usage: ./CLEANUP.sh (depuis la racine du repo)

REPO="$(cd "$(dirname "$0")" && pwd)"
cd "$REPO"

echo "=== Nettoyage des artefacts ==="

# Binaires et .c de test à la racine
rm -f debug_eq debug_eq.c
rm -f file_lex_test file_lex_test.c
rm -f gen_test gen_test.c
rm -f lexer_file_test lexer_file_test.c
rm -f lexer_simple lexer_simple.c
rm -f parser_real parser_real.c
rm -f parser_test parser_test.c
rm -f parser_test2 parser_test2.c
rm -f resolver_test resolver_test.c
rm -f test_ast test_ast.c test_ast2 test_ast2.c test_ast3 test_ast3.c
rm -f test_contains2 test_contains2.c
rm -f token_bootstrap.c token_bootstrap.o
rm -f lexer_test lexer_test.c
rm -f file_lex2 file_lex2.c

# .o orphelins à la racine
rm -f *.o

# Fichiers debug dans parser/
rm -f src/amalgame/parser/debug_eq.am
rm -f src/amalgame/parser/test_input.txt

echo "=== Nettoyage terminé ==="
echo ""
echo "Vérification git :"
git status --short
