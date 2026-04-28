#./compile.sh

# Transpiler + compiler + exécuter
./build/codec ./tests/samples/hello.code
./tests/samples/hello

# Debug : voir l AST
CODE_DEBUG=1 ./build/codec ./tests/samples/hello.code

# Voir le C généré
cat ./tests/samples/hello.c
