# Installer les dépendances (Linux)
sudo apt install valac libglib2.0-dev libgee-0.8-dev meson ninja-build

# Compiler
meson setup build
cd build && ninja

# Tester avec debug
CODE_DEBUG=1 ./codec ../tests/samples/hello.code

# Sortie attendue :
# ⚙️  Compiling: hello.code → hello.c
# ✅ Lexer: 47 tokens
#    [KW_NAMESPACE] 'namespace' @ hello.code:1:1
#    [IDENTIFIER] 'MyApp' @ hello.code:1:11
#    [NEWLINE] '\n' @ hello.code:1:16
#    [KW_IMPORT] 'import' @ hello.code:3:1
#    ...
