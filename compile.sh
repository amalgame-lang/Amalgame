#!/bin/bash
set -e

echo "=== Amalgame Transpiler - Build ==="

# Nettoyage
rm -rf build

# Meson setup
meson setup build

# Compilation
ninja -C build

echo ""
echo "OK Build reussi !"
./build/amc --version
