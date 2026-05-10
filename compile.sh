#!/bin/bash
set -e

echo "=== Amalgame Transpiler - Build ==="

# Meson setup si nécessaire — le meson.build vit dans archive/vala-bootstrap/
# (rang 3 du fallback chain build_amc.sh, gardé pour cold-start).
if [ ! -f build/build.ninja ]; then
    meson setup build archive/vala-bootstrap
fi

# Compilation
ninja -C build

echo ""
echo "OK Build reussi !"
./build/amc --version
