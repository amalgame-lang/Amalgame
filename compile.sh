#!/bin/bash
set -e

echo "=== Amalgame Transpiler - Build ==="

# Meson setup si nécessaire
if [ ! -f build/build.ninja ]; then
    meson setup build
fi

# Compilation
ninja -C build

echo ""
echo "OK Build reussi !"
./build/amc --version
