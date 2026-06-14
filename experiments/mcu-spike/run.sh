#!/usr/bin/env bash
# Run the spike under QEMU with semihosting on the serial/debug console.
# Needs: qemu-system-arm  (sudo apt-get install qemu-system-arm)
set -euo pipefail
cd "$(dirname "$0")"

if ! command -v qemu-system-arm >/dev/null 2>&1; then
  echo "qemu-system-arm not found. Install it with:"
  echo "    sudo apt-get install -y qemu-system-arm"
  exit 127
fi

[ -f blink.elf ] || ./build.sh

exec qemu-system-arm \
  -M lm3s6965evb \
  -nographic \
  -semihosting-config enable=on,target=native \
  -kernel blink.elf
