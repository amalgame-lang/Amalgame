#!/usr/bin/env bash
# Run examples/mcu/hello (ELF) under QEMU (semihosting console).
set -euo pipefail
cd "$(dirname "$0")"
[ -f hello ] || ./build.sh
exec qemu-system-arm -M lm3s6965evb -nographic \
  -semihosting-config enable=on,target=native \
  -kernel hello
