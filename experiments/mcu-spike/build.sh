#!/usr/bin/env bash
# Phase 0 spike build — freestanding Cortex-M3 ELF, no host runtime.
set -euo pipefail
cd "$(dirname "$0")"

CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy
SIZE=arm-none-eabi-size
NM=arm-none-eabi-nm

CFLAGS=(
  -mcpu=cortex-m3 -mthumb
  -Os -g
  -ffreestanding            # __STDC_HOSTED__ = 0
  -ffunction-sections -fdata-sections
  -Wall -Wextra
)
LDFLAGS=(
  -nostartfiles             # we own the reset path (startup.c) — no newlib crt0
  -Wl,--gc-sections
  -T lm3s6965.ld
)

echo ">> compile + link"
"$CC" "${CFLAGS[@]}" "${LDFLAGS[@]}" startup.c blink.c -o blink.elf

echo ">> objcopy -> bin/hex"
"$OBJCOPY" -O binary blink.elf blink.bin
"$OBJCOPY" -O ihex   blink.elf blink.hex

echo ">> size"
"$SIZE" blink.elf

echo ">> host-coupling audit (must be EMPTY):"
if "$NM" blink.elf | grep -iE ' (GC_|_GC|printf|fopen|fwrite|__gxx|operator)' ; then
  echo "!! FAIL: host runtime symbol pulled in"; exit 1
else
  echo "   clean — no GC_*, no stdio"
fi
echo ">> undefined symbols (should only be link-resolved libgcc/newlib helpers, if any):"
"$NM" -u blink.elf || true

echo "OK: blink.elf built"
