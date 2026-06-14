#!/usr/bin/env bash
# Build examples/mcu/hello.am for Cortex-M3 (QEMU lm3s6965evb) in one step.
# `amc build --target=` does the transpile + cross-compile + link + objcopy.
set -euo pipefail
cd "$(dirname "$0")"
AMC="$(cd ../.. && pwd)/amc"

"$AMC" build --target=cortex-m3 hello.am -o hello --verbose
echo "OK: hello (ELF) + hello.bin + hello.hex"
