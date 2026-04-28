# Transpiler + compiler + exécuter
./build/codec mon_programme.code
./mon_programme

# Debug : voir l AST
CODE_DEBUG=1 ./build/codec mon_programme.code

# Voir le C généré
cat mon_programme.c
